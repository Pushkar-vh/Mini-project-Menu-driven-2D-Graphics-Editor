/*
 * ============================================================================
 *  Mini Project: Menu-Driven 2D Graphics Editor
 * ============================================================================
 *  A console-based 2D graphics editor that uses a character array as canvas.
 *  Canvas is filled with '_' (underscore) and objects are drawn with '*'.
 *  Supports drawing, deleting, and modifying circles, rectangles, lines,
 *  and triangles.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* ── Canvas dimensions ───────────────────────────────────────────────────── */
#define CANVAS_ROWS 40
#define CANVAS_COLS 80
#define BLANK_CHAR  '_'
#define DRAW_CHAR   '*'

/* ── Maximum number of objects stored ─────────────────────────────────────── */
#define MAX_OBJECTS 50

/* ── Object type enumeration ─────────────────────────────────────────────── */
typedef enum {
    OBJ_NONE,
    OBJ_CIRCLE,
    OBJ_RECTANGLE,
    OBJ_LINE,
    OBJ_TRIANGLE
} ObjectType;

/* ── Object data structure ───────────────────────────────────────────────── */
typedef struct {
    ObjectType type;
    /* Circle:    params[0]=cx, params[1]=cy, params[2]=radius              */
    /* Rectangle: params[0]=top-left x, params[1]=top-left y,               */
    /*            params[2]=width, params[3]=height                         */
    /* Line:      params[0]=x1, params[1]=y1, params[2]=x2, params[3]=y2   */
    /* Triangle:  params[0]=x1, params[1]=y1, params[2]=x2, params[3]=y2,  */
    /*            params[4]=x3, params[5]=y3                                */
    int params[6];
} GraphObject;

/* ── Global state ────────────────────────────────────────────────────────── */
char canvas[CANVAS_ROWS][CANVAS_COLS];
GraphObject objects[MAX_OBJECTS];
int objectCount = 0;

/* ══════════════════════════════════════════════════════════════════════════ */
/*                          CANVAS OPERATIONS                               */
/* ══════════════════════════════════════════════════════════════════════════ */

/* Clear the canvas – fill every cell with BLANK_CHAR */
void clearCanvas(void) {
    int i, j;
    for (i = 0; i < CANVAS_ROWS; i++)
        for (j = 0; j < CANVAS_COLS; j++)
            canvas[i][j] = BLANK_CHAR;
}

/* Display the canvas with a border */
void displayCanvas(void) {
    int i, j;

    /* Top border */
    printf("\n  +");
    for (j = 0; j < CANVAS_COLS; j++) printf("-");
    printf("+\n");

    /* Canvas rows with side borders */
    for (i = 0; i < CANVAS_ROWS; i++) {
        printf("%2d|", i);
        for (j = 0; j < CANVAS_COLS; j++)
            printf("%c", canvas[i][j]);
        printf("|\n");
    }

    /* Bottom border */
    printf("  +");
    for (j = 0; j < CANVAS_COLS; j++) printf("-");
    printf("+\n");

    /* Column number hints (tens and units) */
    printf("   ");
    for (j = 0; j < CANVAS_COLS; j++) printf("%d", (j / 10) % 10);
    printf("\n   ");
    for (j = 0; j < CANVAS_COLS; j++) printf("%d", j % 10);
    printf("\n");
}

/* Set a pixel on the canvas if within bounds */
void setPixel(int row, int col) {
    if (row >= 0 && row < CANVAS_ROWS && col >= 0 && col < CANVAS_COLS)
        canvas[row][col] = DRAW_CHAR;
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                          DRAWING PRIMITIVES                              */
/* ══════════════════════════════════════════════════════════════════════════ */

/* ── Draw a circle using the Midpoint Circle Algorithm ───────────────────── */
void drawCircle(int cx, int cy, int radius) {
    int x = 0, y = radius;
    int d = 1 - radius;

    while (x <= y) {
        /* Plot all 8 symmetric octant points */
        setPixel(cy + y, cx + x);
        setPixel(cy + y, cx - x);
        setPixel(cy - y, cx + x);
        setPixel(cy - y, cx - x);
        setPixel(cy + x, cx + y);
        setPixel(cy + x, cx - y);
        setPixel(cy - x, cx + y);
        setPixel(cy - x, cx - y);

        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

/* ── Draw a rectangle (outline) ──────────────────────────────────────────── */
void drawRectangle(int topX, int topY, int width, int height) {
    int i;
    /* Top and bottom horizontal edges */
    for (i = topX; i <= topX + width; i++) {
        setPixel(topY, i);
        setPixel(topY + height, i);
    }
    /* Left and right vertical edges */
    for (i = topY; i <= topY + height; i++) {
        setPixel(i, topX);
        setPixel(i, topX + width);
    }
}

/* ── Draw a line using Bresenham's Line Algorithm ────────────────────────── */
void drawLine(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int e2;

    while (1) {
        setPixel(y1, x1);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

/* ── Draw a triangle (three connected lines) ─────────────────────────────── */
void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x3, y3);
    drawLine(x3, y3, x1, y1);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                     REDRAW ALL OBJECTS ON CANVAS                         */
/* ══════════════════════════════════════════════════════════════════════════ */

/* Wipe canvas and re-render every stored object */
void redrawAll(void) {
    int i;
    clearCanvas();
    for (i = 0; i < objectCount; i++) {
        GraphObject *o = &objects[i];
        switch (o->type) {
            case OBJ_CIRCLE:
                drawCircle(o->params[0], o->params[1], o->params[2]);
                break;
            case OBJ_RECTANGLE:
                drawRectangle(o->params[0], o->params[1],
                              o->params[2], o->params[3]);
                break;
            case OBJ_LINE:
                drawLine(o->params[0], o->params[1],
                         o->params[2], o->params[3]);
                break;
            case OBJ_TRIANGLE:
                drawTriangle(o->params[0], o->params[1],
                             o->params[2], o->params[3],
                             o->params[4], o->params[5]);
                break;
            default:
                break;
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                          HELPER UTILITIES                                */
/* ══════════════════════════════════════════════════════════════════════════ */

/* Print the name of an object type */
const char* objectTypeName(ObjectType t) {
    switch (t) {
        case OBJ_CIRCLE:    return "Circle";
        case OBJ_RECTANGLE: return "Rectangle";
        case OBJ_LINE:      return "Line";
        case OBJ_TRIANGLE:  return "Triangle";
        default:            return "Unknown";
    }
}

/* Print details of a single object */
void printObjectInfo(int index) {
    GraphObject *o = &objects[index];
    printf("  [%d] %s  |  ", index + 1, objectTypeName(o->type));
    switch (o->type) {
        case OBJ_CIRCLE:
            printf("Center(%d,%d)  Radius=%d",
                   o->params[0], o->params[1], o->params[2]);
            break;
        case OBJ_RECTANGLE:
            printf("TopLeft(%d,%d)  W=%d  H=%d",
                   o->params[0], o->params[1],
                   o->params[2], o->params[3]);
            break;
        case OBJ_LINE:
            printf("(%d,%d) -> (%d,%d)",
                   o->params[0], o->params[1],
                   o->params[2], o->params[3]);
            break;
        case OBJ_TRIANGLE:
            printf("(%d,%d) (%d,%d) (%d,%d)",
                   o->params[0], o->params[1],
                   o->params[2], o->params[3],
                   o->params[4], o->params[5]);
            break;
        default:
            break;
    }
    printf("\n");
}

/* List all objects currently stored */
void listObjects(void) {
    int i;
    if (objectCount == 0) {
        printf("\n  >> No objects on canvas.\n");
        return;
    }
    printf("\n  %-5s %-12s %s\n", "ID", "Type", "Parameters");
    printf("  ------------------------------------------------\n");
    for (i = 0; i < objectCount; i++)
        printObjectInfo(i);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                           ADD OBJECT                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

void addObject(void) {
    int choice;
    GraphObject obj;
    memset(&obj, 0, sizeof(obj));

    if (objectCount >= MAX_OBJECTS) {
        printf("\n  >> Maximum object limit (%d) reached!\n", MAX_OBJECTS);
        return;
    }

    printf("\n  +-------------------------------+\n");
    printf("  |      Add Shape to Canvas      |\n");
    printf("  +-------------------------------+\n");
    printf("  |  1. Circle                    |\n");
    printf("  |  2. Rectangle                 |\n");
    printf("  |  3. Line                      |\n");
    printf("  |  4. Triangle                  |\n");
    printf("  |  5. Cancel                    |\n");
    printf("  +-------------------------------+\n");
    printf("  Enter choice: ");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            obj.type = OBJ_CIRCLE;
            printf("  Enter center X (col 0-%d): ", CANVAS_COLS - 1);
            scanf("%d", &obj.params[0]);
            printf("  Enter center Y (row 0-%d): ", CANVAS_ROWS - 1);
            scanf("%d", &obj.params[1]);
            printf("  Enter radius: ");
            scanf("%d", &obj.params[2]);
            if (obj.params[2] <= 0) {
                printf("\n  >> Radius must be positive!\n");
                return;
            }
            break;
        case 2:
            obj.type = OBJ_RECTANGLE;
            printf("  Enter top-left X (col 0-%d): ", CANVAS_COLS - 1);
            scanf("%d", &obj.params[0]);
            printf("  Enter top-left Y (row 0-%d): ", CANVAS_ROWS - 1);
            scanf("%d", &obj.params[1]);
            printf("  Enter width: ");
            scanf("%d", &obj.params[2]);
            printf("  Enter height: ");
            scanf("%d", &obj.params[3]);
            if (obj.params[2] <= 0 || obj.params[3] <= 0) {
                printf("\n  >> Width and height must be positive!\n");
                return;
            }
            break;
        case 3:
            obj.type = OBJ_LINE;
            printf("  Enter start X (col): ");
            scanf("%d", &obj.params[0]);
            printf("  Enter start Y (row): ");
            scanf("%d", &obj.params[1]);
            printf("  Enter end X (col): ");
            scanf("%d", &obj.params[2]);
            printf("  Enter end Y (row): ");
            scanf("%d", &obj.params[3]);
            break;
        case 4:
            obj.type = OBJ_TRIANGLE;
            printf("  Enter vertex 1 X (col): ");
            scanf("%d", &obj.params[0]);
            printf("  Enter vertex 1 Y (row): ");
            scanf("%d", &obj.params[1]);
            printf("  Enter vertex 2 X (col): ");
            scanf("%d", &obj.params[2]);
            printf("  Enter vertex 2 Y (row): ");
            scanf("%d", &obj.params[3]);
            printf("  Enter vertex 3 X (col): ");
            scanf("%d", &obj.params[4]);
            printf("  Enter vertex 3 Y (row): ");
            scanf("%d", &obj.params[5]);
            break;
        case 5:
            printf("\n  >> Cancelled.\n");
            return;
        default:
            printf("\n  >> Invalid choice!\n");
            return;
    }

    objects[objectCount++] = obj;
    redrawAll();
    printf("\n  >> %s added successfully! (Object #%d)\n",
           objectTypeName(obj.type), objectCount);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                          DELETE OBJECT                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void deleteObject(void) {
    int id, i;

    if (objectCount == 0) {
        printf("\n  >> No objects to delete.\n");
        return;
    }

    printf("\n  Current objects on canvas:\n");
    listObjects();

    printf("\n  Enter object ID to delete (1-%d): ", objectCount);
    scanf("%d", &id);

    if (id < 1 || id > objectCount) {
        printf("\n  >> Invalid object ID!\n");
        return;
    }

    printf("  >> Deleting %s (Object #%d)...\n",
           objectTypeName(objects[id - 1].type), id);

    /* Shift remaining objects left to fill the gap */
    for (i = id - 1; i < objectCount - 1; i++)
        objects[i] = objects[i + 1];

    objectCount--;
    redrawAll();
    printf("  >> Object deleted. %d object(s) remaining.\n", objectCount);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                          MODIFY OBJECT                                   */
/* ══════════════════════════════════════════════════════════════════════════ */

void modifyObject(void) {
    int id;
    GraphObject *o;

    if (objectCount == 0) {
        printf("\n  >> No objects to modify.\n");
        return;
    }

    printf("\n  Current objects on canvas:\n");
    listObjects();

    printf("\n  Enter object ID to modify (1-%d): ", objectCount);
    scanf("%d", &id);

    if (id < 1 || id > objectCount) {
        printf("\n  >> Invalid object ID!\n");
        return;
    }

    o = &objects[id - 1];
    printf("\n  >> Modifying %s (Object #%d)\n",
           objectTypeName(o->type), id);

    switch (o->type) {
        case OBJ_CIRCLE:
            printf("  Current: Center(%d,%d) Radius=%d\n",
                   o->params[0], o->params[1], o->params[2]);
            printf("  Enter new center X (col): ");
            scanf("%d", &o->params[0]);
            printf("  Enter new center Y (row): ");
            scanf("%d", &o->params[1]);
            printf("  Enter new radius: ");
            scanf("%d", &o->params[2]);
            if (o->params[2] <= 0) {
                printf("\n  >> Radius must be positive! Reverting.\n");
                return;
            }
            break;
        case OBJ_RECTANGLE:
            printf("  Current: TopLeft(%d,%d) W=%d H=%d\n",
                   o->params[0], o->params[1],
                   o->params[2], o->params[3]);
            printf("  Enter new top-left X (col): ");
            scanf("%d", &o->params[0]);
            printf("  Enter new top-left Y (row): ");
            scanf("%d", &o->params[1]);
            printf("  Enter new width: ");
            scanf("%d", &o->params[2]);
            printf("  Enter new height: ");
            scanf("%d", &o->params[3]);
            if (o->params[2] <= 0 || o->params[3] <= 0) {
                printf("\n  >> Dimensions must be positive! Reverting.\n");
                return;
            }
            break;
        case OBJ_LINE:
            printf("  Current: (%d,%d) -> (%d,%d)\n",
                   o->params[0], o->params[1],
                   o->params[2], o->params[3]);
            printf("  Enter new start X (col): ");
            scanf("%d", &o->params[0]);
            printf("  Enter new start Y (row): ");
            scanf("%d", &o->params[1]);
            printf("  Enter new end X (col): ");
            scanf("%d", &o->params[2]);
            printf("  Enter new end Y (row): ");
            scanf("%d", &o->params[3]);
            break;
        case OBJ_TRIANGLE:
            printf("  Current: (%d,%d) (%d,%d) (%d,%d)\n",
                   o->params[0], o->params[1],
                   o->params[2], o->params[3],
                   o->params[4], o->params[5]);
            printf("  Enter new vertex 1 X (col): ");
            scanf("%d", &o->params[0]);
            printf("  Enter new vertex 1 Y (row): ");
            scanf("%d", &o->params[1]);
            printf("  Enter new vertex 2 X (col): ");
            scanf("%d", &o->params[2]);
            printf("  Enter new vertex 2 Y (row): ");
            scanf("%d", &o->params[3]);
            printf("  Enter new vertex 3 X (col): ");
            scanf("%d", &o->params[4]);
            printf("  Enter new vertex 3 Y (row): ");
            scanf("%d", &o->params[5]);
            break;
        default:
            break;
    }

    redrawAll();
    printf("\n  >> Object #%d modified successfully!\n", id);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                          TRANSLATE OBJECT                                */
/* ══════════════════════════════════════════════════════════════════════════ */

void translateObject(void) {
    int id, dx, dy;
    GraphObject *o;

    if (objectCount == 0) {
        printf("\n  >> No objects to translate.\n");
        return;
    }

    printf("\n  Current objects on canvas:\n");
    listObjects();

    printf("\n  Enter object ID to translate (1-%d): ", objectCount);
    scanf("%d", &id);

    if (id < 1 || id > objectCount) {
        printf("\n  >> Invalid object ID!\n");
        return;
    }

    printf("  Enter X offset (cols, +right/-left): ");
    scanf("%d", &dx);
    printf("  Enter Y offset (rows, +down/-up):    ");
    scanf("%d", &dy);

    o = &objects[id - 1];
    switch (o->type) {
        case OBJ_CIRCLE:
            o->params[0] += dx;
            o->params[1] += dy;
            break;
        case OBJ_RECTANGLE:
            o->params[0] += dx;
            o->params[1] += dy;
            break;
        case OBJ_LINE:
            o->params[0] += dx;  o->params[1] += dy;
            o->params[2] += dx;  o->params[3] += dy;
            break;
        case OBJ_TRIANGLE:
            o->params[0] += dx;  o->params[1] += dy;
            o->params[2] += dx;  o->params[3] += dy;
            o->params[4] += dx;  o->params[5] += dy;
            break;
        default:
            break;
    }

    redrawAll();
    printf("\n  >> Object #%d translated by (%d, %d).\n", id, dx, dy);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                          SCALE OBJECT                                    */
/* ══════════════════════════════════════════════════════════════════════════ */

void scaleObject(void) {
    int id;
    float factor;
    GraphObject *o;

    if (objectCount == 0) {
        printf("\n  >> No objects to scale.\n");
        return;
    }

    printf("\n  Current objects on canvas:\n");
    listObjects();

    printf("\n  Enter object ID to scale (1-%d): ", objectCount);
    scanf("%d", &id);

    if (id < 1 || id > objectCount) {
        printf("\n  >> Invalid object ID!\n");
        return;
    }

    printf("  Enter scale factor (e.g. 1.5 or 0.5): ");
    scanf("%f", &factor);

    if (factor <= 0) {
        printf("\n  >> Scale factor must be positive!\n");
        return;
    }

    o = &objects[id - 1];
    switch (o->type) {
        case OBJ_CIRCLE:
            o->params[2] = (int)(o->params[2] * factor);
            if (o->params[2] < 1) o->params[2] = 1;
            break;
        case OBJ_RECTANGLE:
            o->params[2] = (int)(o->params[2] * factor);
            o->params[3] = (int)(o->params[3] * factor);
            if (o->params[2] < 1) o->params[2] = 1;
            if (o->params[3] < 1) o->params[3] = 1;
            break;
        case OBJ_LINE: {
            int mx = (o->params[0] + o->params[2]) / 2;
            int my = (o->params[1] + o->params[3]) / 2;
            o->params[0] = mx + (int)((o->params[0] - mx) * factor);
            o->params[1] = my + (int)((o->params[1] - my) * factor);
            o->params[2] = mx + (int)((o->params[2] - mx) * factor);
            o->params[3] = my + (int)((o->params[3] - my) * factor);
            break;
        }
        case OBJ_TRIANGLE: {
            int cx = (o->params[0] + o->params[2] + o->params[4]) / 3;
            int cy = (o->params[1] + o->params[3] + o->params[5]) / 3;
            o->params[0] = cx + (int)((o->params[0] - cx) * factor);
            o->params[1] = cy + (int)((o->params[1] - cy) * factor);
            o->params[2] = cx + (int)((o->params[2] - cx) * factor);
            o->params[3] = cy + (int)((o->params[3] - cy) * factor);
            o->params[4] = cx + (int)((o->params[4] - cx) * factor);
            o->params[5] = cy + (int)((o->params[5] - cy) * factor);
            break;
        }
        default:
            break;
    }

    redrawAll();
    printf("\n  >> Object #%d scaled by factor %.2f.\n", id, factor);
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                          CLEAR ALL OBJECTS                               */
/* ══════════════════════════════════════════════════════════════════════════ */

void clearAll(void) {
    char confirm;
    printf("\n  >> Are you sure you want to clear ALL objects? (y/n): ");
    scanf(" %c", &confirm);
    if (confirm == 'y' || confirm == 'Y') {
        objectCount = 0;
        clearCanvas();
        printf("  >> Canvas cleared!\n");
    } else {
        printf("  >> Cancelled.\n");
    }
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                            MAIN MENU                                     */
/* ══════════════════════════════════════════════════════════════════════════ */

void printMenu(void) {
    printf("\n");
    printf("  +------------------------------------------+\n");
    printf("  |       2D GRAPHICS EDITOR - MAIN MENU     |\n");
    printf("  +------------------------------------------+\n");
    printf("  |  1.  Display Canvas                      |\n");
    printf("  |  2.  Add Object                          |\n");
    printf("  |  3.  Delete Object                       |\n");
    printf("  |  4.  Modify Object                       |\n");
    printf("  |  5.  Translate (Move) Object              |\n");
    printf("  |  6.  Scale Object                        |\n");
    printf("  |  7.  List All Objects                    |\n");
    printf("  |  8.  Clear Canvas                        |\n");
    printf("  |  9.  Exit                                |\n");
    printf("  +------------------------------------------+\n");
    printf("  Enter your choice: ");
}

/* ══════════════════════════════════════════════════════════════════════════ */
/*                              MAIN                                        */
/* ══════════════════════════════════════════════════════════════════════════ */

int main(void) {
    int choice;

    clearCanvas();

    printf("\n");
    printf("  +------------------------------------------------------+\n");
    printf("  |                                                      |\n");
    printf("  |    Welcome to the 2D Graphics Editor!                |\n");
    printf("  |                                                      |\n");
    printf("  |    Canvas Size : %d rows x %d cols                  |\n",
           CANVAS_ROWS, CANVAS_COLS);
    printf("  |    Blank Char  : '%c'                                |\n",
           BLANK_CHAR);
    printf("  |    Draw Char   : '%c'                                |\n",
           DRAW_CHAR);
    printf("  |                                                      |\n");
    printf("  +------------------------------------------------------+\n");

    do {
        printMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                displayCanvas();
                break;
            case 2:
                addObject();
                break;
            case 3:
                deleteObject();
                break;
            case 4:
                modifyObject();
                break;
            case 5:
                translateObject();
                break;
            case 6:
                scaleObject();
                break;
            case 7:
                listObjects();
                break;
            case 8:
                clearAll();
                break;
            case 9:
                printf("\n  >> Exiting Graphics Editor. Goodbye!\n\n");
                break;
            default:
                printf("\n  >> Invalid choice! Please try again.\n");
        }
    } while (choice != 9);

    return 0;
}

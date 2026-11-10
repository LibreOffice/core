# Draw and Impress in vector mode - a first overview

In vector mode the client draws the slide from the objects the engine sends, not from
picture tiles. The selection, its handles and what you do with the mouse and the keys are
worked out in the client, and the engine is told the result. This page lists what you can do,
for someone using it the first time.

The slide stands on a lawn, a workspace around it. Objects can lie on the lawn, partly or
wholly outside the slide, and are moved and selected there like any other.


## 0. Switching vector mode on

Vector mode is for Draw and Impress documents only.

### In the desktop application

Start it with the environment variable CODA_VECTOR set to anything, for example:

    CODA_VECTOR=1 ./qt/coda-qt --edit presentation.odp

| Part of the command | What it does |
|---|---|
| CODA_VECTOR=1 | Opens Draw and Impress documents in vector mode. Any value that is not empty does. |
| ACTIVATE_ADVANCED_DIAGRAM_FEATURES=1 | Also needed to work on diagrams one object at a time; without it a diagram is a plain group. Put it in front the same way. |
| --edit | Opens the document ready to edit. Without it the document opens read-only, and no handles are shown. |
| --debug | Writes trace output to the terminal, for finding faults; not needed to use it. |

A full line for diagrams:

    ACTIVATE_ADVANCED_DIAGRAM_FEATURES=1 CODA_VECTOR=1 ./qt/coda-qt --edit presentation.odp

### In the browser, with a Collabora Online server

Add the parameter vector=true to the address of the page that loads the document. It is the
same parameter the desktop application sets for you.

### At runtime: the debug panel

Ctrl + Shift + Alt + D opens and closes the debug panel. Under "Display":

| Switch | What it does |
|---|---|
| Vector | Switches vector mode on and off for the open document, at once. It starts at what the document was opened with. Switching ends the selection. |
| Vector Overlays | Draws the outline of every object and what the hit test finds under the pointer, for checking. Only with Vector on. |


## 1. Moving around in the document

### Changing slides

| Mouse or keys | What it does |
|---|---|
| Click a slide in the slide panel | Shows that slide. |
| Page Up / Page Down | The slide before / after. |
| Home / End (Pos1 / Ende) | The first / the last slide. |

The keys change slides as in the office, while nothing is selected and no text is edited. With
an object selected, Home and End with Ctrl select the first and the last object instead (see
below).

### Scrolling and zooming

| What | Mouse | Keys |
|---|---|---|
| Scroll | Mouse wheel; Shift + wheel across; the scroll bars | Arrow keys while nothing is selected |
| Zoom | Ctrl + wheel | The zoom control |
| Scroll while dragging | Drag past the edge of the view: the further out, the faster | - |
| Call off a scroll bar drag | Escape while the bar is held puts the view back | - |

Scrolling by arrow key, with nothing selected:

| Modifier | Step |
|---|---|
| none | a twentieth of the view |
| Shift | a quarter of the view |
| Ctrl | half of the view |
| Alt | one pixel |

The zoom you chose and the place you look at are kept when the window is resized.

The lawn grows to take in every object of the slide, so an object a file holds far outside the
slide can still be scrolled to.

### Selecting objects

| Mouse | What it does |
|---|---|
| Click an object | Selects it alone. Inside a group you have not entered, the click selects the group. |
| Click empty space | Selects nothing. Inside a group it leaves the group (see section 3). |
| Shift + click or Ctrl + click | Adds the object to the selection, or takes it out when it is selected. Only objects of the same level go together; an object of another level replaces the selection. |
| Alt + click | Goes through all objects under the mouse, one per click, from the front to the back. A group comes before the objects it holds, so the clicks go into groups. |
| Alt + Shift + click | The same, backwards. |
| Double click | On an object of a group: enters the group one level and selects that object. On a plain object: edits its text. |
| Drag on empty space | Draws a dashed frame and selects every object that lies wholly inside it, at the level you stand at (inside an entered group, its objects). With Shift each object in the frame changes its state: a selected one leaves the selection, the others join it. Escape while dragging drops the frame. Form controls out of design mode are left out. |
| Right click | Opens the context menu for the object under the mouse. |

| Keys | What they do |
|---|---|
| Tab / Shift + Tab | Selects the next / the previous object of the level, going round at the end. With nothing selected it starts at the first / the last. |
| Ctrl + Home / Ctrl + End | Selects the first / the last object of the level. |
| Escape | On a page: selects nothing. In a group: selects the group, which leaves it. |
| Enter or F3 | On one selected group: enters it and selects its first object. On another object: starts editing its text. |
| Ctrl + F3 | Leaves the group you stand in and selects it. |
| F2 | Starts editing the text of the selected object, as in the office. |
| Ctrl + A | Selects all objects of the slide, as in the office. |

An object selected by a key is scrolled into view.

While text is edited, every key goes to the text. Escape ends the text edit, and a click
outside the object does too.

The pointer says what a click would do: a text pointer over text, the hand where something can
be dragged, the plain arrow elsewhere.

A click a little beside a thin line still takes the object: 8 screen pixels, whatever the zoom.
The distance can be set in the client's preferences as vectorHitTolerance, in screen pixels.


## 2. Changing objects

### The handles

| Handle | Where | What a drag does |
|---|---|---|
| Eight framing handles | Corners and side middles, a little outside the frame | Scale. A side handle dragged past the opposite side mirrors the object. |
| Turn handle | Above the middle of the upper side | Turns the selection about its middle; in its shearing mode it shears it instead, see "Turning and shearing" below. Shown only where the selection can be turned, not for a table, for example. |
| Yellow handles | On custom shapes | Change the shape (an arrow's head, a corner's size); each runs only where the shape lets it. |
| Corner radius | On a rectangle | Rounds the corners. |
| Line ends | At both ends of a straight line | Move that end; see "Lines" below. |
| Path points | On curves and polygons, while Edit Points is on | Move a point or one of its two weights (curve handles); the curve keeps how it runs through the point. A click opens the menu of the point, see "Editing points" below. |
| Connector ends | At both ends of a connector | Tie the end to the object or glue point it is dropped on, or leave it free. |
| Connector lines | On the lines between the ends | Move that line, across itself only. |
| Glue points | On the object, while Glue Points is on | Move a point someone added. A click opens the menu of the point, see "Editing points" below. The points the shape makes itself and the four at the middles of the sides stay where they are. |

Pale handles cannot be used: the object does not allow that change (it is protected, or it is
a node of a diagram).

### With the mouse

| Action | How | Modifiers |
|---|---|---|
| Move | Drag the object; it stops at the edge of the lawn, and a move back towards the slide is never stopped | Alt turns the snap over: with the grid on it is off while Alt is held, with the grid off Alt snaps to it. Without the grid the drag lines up with the edges and middles of other objects. Ctrl held when you let go: a copy goes to the new place and the object stays. |
| Scale | Drag a framing handle | A shape: Shift keeps the ratio. A picture or media: keeps the ratio, Shift lets it go. A group or several objects with a turned object among them: keeps the ratio, Shift lets it go. An object that cannot be sheared always keeps it. Ctrl held when you let go: a scaled copy, the object stays. |
| Turn | Drag the turn handle | Snaps to angles counted from the page: Ctrl 45 degrees, Shift 15, Alt 5, no modifier free. The "0" mark beyond the handle stands it upright. |
| Shear | Drag the turn handle in its shearing mode | The same snaps, counted from no slant. The "0" mark takes the slant away. |
| Mirror | Drag a side handle past the opposite side | - |
| Call off | Escape while dragging | Puts everything back; the selection stays. |

A modifier pressed or let go while the mouse stands still takes effect at once. The copy with
Ctrl follows the desktop application's option "Copy when moving" (Impress or Draw, General);
with it off, Ctrl does not copy. A turn and a shear keep Ctrl for their snap and do not copy.

### With the keyboard

| Keys | What they do |
|---|---|
| Arrow keys | Move the selection. It stops at the edge of the lawn. |
| Ctrl + Tab or Alt + Tab | Puts the keyboard on the first handle, then goes to the next one; with Shift to the one before. The handle in use blinks. Many systems take Alt + Tab for switching windows; then use Ctrl + Tab. |
| Ctrl + Home / Ctrl + End | With the keyboard on a handle: the first / the last handle. |
| Arrow keys, keyboard on a handle | Move that handle as a drag would: scale, turn, shape. |
| Escape | Takes the keyboard off the handle. |
| 0 | Takes every slant away, where some object is slanted; once none is, every turn, so each object stands upright again, where some object is turned or some line is not level; otherwise it is an ordinary key. So a turned and slanted object loses its slant with the first 0 and its turn with the second. A line is laid level the shorter way. |

The 0 key works on each object by its own angle, around its own middle, also on objects
selected together and on the objects inside a selected group, at any depth. All of it is one
undo step. The result appears when the engine answers.

Steps for moving an object or a handle by key:

| Modifier | Step |
|---|---|
| none | 1 mm |
| Shift | 10 mm |
| Ctrl | 25 mm |
| Alt | one pixel |

On a very large page the millimetre steps grow by ten for each tenfold of an A4 page.

On the turn handle, Left and Up turn against the clock, Right and Down with it: Ctrl to the
next 45 degrees, Shift 15, Alt 5, no modifier 1. In its shearing mode the same keys slant. The
0 key brings the angle of the handle's mode back to nought, the turn in turning mode and the
slant in shearing mode, and once that is nought the other one. Space or Return switch the mode.

A key step goes exactly where it says: no snap and no kept ratio. What the keys change is sent
to the engine when you let the keys go, so a run of steps is one undo step. A key held down
goes on step by step on every handle, also on the ones the engine works out (a corner radius,
the angles of a circle piece, a callout's tail, the points of a custom shape, connectors, glue
points): the handle stands where the keys took it until the engine has answered.

A callout keeps the end of its tail where it is when you move or scale it, as the office does:
the box goes along and the tail is laid out anew to the end, in the ghost already. Dragging the
tail end shows the box and the new tail thin. Scaling a callout or a text box past its other
side lays it out upright on the other side, since neither can be mirrored.

The settings of the tail are in Position and Size on a callout, on its Callout tab: the style,
the spacing, the side it leaves the box at and the length. A connector has its own dialog in the
context menu, "Connector...", for its type, the skew of its lines and the spacing at its ends.

### Turning and shearing

The handle above the selection has two modes: turning, shown as a circle, and shearing, shown
as a rectangle slanted by thirty degrees. A new selection starts in the turning mode. Shearing
is offered only where every selected object can be sheared; a picture cannot.

Switching the mode:

| How | What happens |
|---|---|
| Click the handle | Switches to the other mode. |
| Rest the mouse on the handle for two seconds | The other mode's symbol comes up beside it on a bar, to the right of the circle or to the left of the slanted rectangle. Move along the bar onto it to switch. Leaving the bar puts it away; a press on the handle does too. |
| Space or Return, keyboard on the handle | Switches to the other mode. |

A shear works as the office's slant does, about the middle of the selection: the upper side
keeps its direction and length, and the other axis turns by the angle the mouse goes round the
middle, keeping its length. Past ninety degrees the selection turns over; the shear itself
stays within 89 degrees either way, as an object holds no more.

While turning or shearing:

- A mark labelled "0" (with a degree sign) stands beyond the handle where there is no turn or
  no slant. With the mouse on it the handle snaps there, so one movement stands the selection
  upright or takes the slant away.
- The angle reached is written below and to the right of the mouse.
- While the handle is shown, its angle is written below and to the right of it: the turn in
  the turning mode, the slant in the shearing mode. With the keyboard on the handle the arrow
  keys and the 0 key work as in "With the keyboard" above. A run of key steps keeps
  counting its angle until something else changes the objects. Several objects or a group
  that all have the same turn and slant show theirs and start from it.

A single object turns and shears from the angle and slant it has, and so do several objects or
a group whose objects all have the same turn and slant. Otherwise several objects or a group
start from nought each time, on their upright frame, and get one change all alike.

The two angle readings can be switched off in the client's preferences: vectorAngleAtMouse for
the one beside the mouse, vectorAngleAtHandle for the one beside the handle. How long the mouse
rests before the bar comes up is vectorOtherModeDelay, in milliseconds.

### Other keys

These work as in the office, through the engine:

| Keys | What they do |
|---|---|
| Delete or Backspace | Deletes what is selected. The selection goes to what is left: inside a group it stays in the group. |
| Ctrl + Z / Ctrl + Y | Undo / redo. |
| Ctrl + C / Ctrl + X / Ctrl + V | Copy / cut / paste. A node of a diagram is copied as a plain shape. |

### Cropping a picture

Select one picture and press Crop in the toolbar. Crop marks appear on the picture's edge.

- Drag a mark to crop. Shift keeps the ratio of the window.
- A mark snaps to the graphic's edge when it comes close; Alt lets it go closer.
- The handle in the middle moves the window over the picture.
- Crop marks also move by key, like other handles.
- Press Crop again to end.
- While cropping, the eight handles and the turn handle are hidden, so only the crop marks act.

### Editing points

The Shape tab of the notebookbar has two buttons for the points of one selected object. Only one
kind of points is shown at a time: showing the one hides the other.

| Button | What it does |
|---|---|
| Edit Points | Shown in place of Convert to Polygon when one path is selected. Shows the points and weights of the path, and hides them again. Its arrow opens: Add Point, Close Path (closes an open path or opens a closed one), Simplify Curves, Simplify All, and the conversions To Curve, To Contour and To Polygon. |
| Convert to Polygon | Shown for a shape that is not a path. Turns it into a path whose points can be edited: curved edges stay curves, straight ones become lines. Its arrow opens To Curve, To Contour and To Polygon. |
| Glue Points | Shows the glue points of the selected object in place of the eight handles and the turn handle, and hides them again. Its arrow opens Show and Add New. |

With Add Point or Add New the pointer turns into a cross over the object, and the next press adds
the point there; Escape ends it without one.

A click on a point opens its menu:

| Point | Menu |
|---|---|
| Path point | Delete; To Curve or To Line, for the piece after the point; Corner Point, Smooth Transition, Symmetric Transition; Split Path. |
| Glue point | The ways a connector may leave it: Left, Right, Up, Down; Relative, where the point keeps its share of the size of the object, not its distance; Horizontal Alignment and Vertical Alignment, the sides its distance is measured from, only without Relative; Delete. A point the shape makes from its own geometry shows the entries disabled. |

Delete with the keyboard on a point deletes that point, and the object stays. Each change is
one undo step.

### Dimension lines

A dimension line (a measurement) has no frame of eight. It is shaped by its own six handles:

- The start and the end handle sit on the two measured points and move freely. Moving one
  changes the length, and the value shown beside the mouse changes with it.
- The bottom handles sit where the help lines begin, behind the start and end handles and a
  little larger, so they can still be taken hold of: a press inside the small handle takes the
  end point, a press on the rim around it the help line. They move across the measured line
  only.
- The top handles sit at the far ends of the help lines and move the dimension line nearer or
  further, across the measured line only. Taken across the measured line, they move it to the
  other side.

With Shift held while dragging, the handles behave as in the office: the start and end handle
keep the direction of the measured line, the two bottom handles move together, and the top
handles stay where they are. With Ctrl held, a handle snaps to whole units of what the dimension
line shows, for example whole centimetres; with Alt, to tenths of that unit. The ends snap
the measured length, the other handles the length they change.

While a handle is dragged or moved by keys, the dimension line is drawn thin as it will be: the
dimension line, the help lines and the box of the text, laid out as the drawing does, with the
text moving beside the line where it no longer fits between the help lines. The other handles
follow at once.

The handle above, 45 pixels beyond the middle of the dimension line, turns it about the middle
of the measured line. A dimension line is turned only, not slanted.

Moving, turning and the arrow keys keep the measured line as it is, whichever way it runs.

### Lines

A straight line, one of two points, has no frame of eight either, whichever way it runs. It is
shaped by its two ends, which move freely; while one is dragged or moved by keys the line is
drawn thin from the other end to it.

- With Shift held, the end keeps the direction of the line, as in the office.
- With Ctrl held, the length snaps to whole centimetres, with Alt to millimetres, also together
  with Shift. On a very large page the steps grow by ten for each tenfold of an A4 page. The
  steps can be set in the client's preferences: vectorLengthSnapControl and
  vectorLengthSnapAlt, in millimetres.

The handle that turns a line stands beside its middle, at the usual distance, on the left of
the way from its first point to its second, so it shows which way the line was drawn. The
angle of a line is the one of that way: 0 degrees for a line drawn level from left to right,
180 for one drawn from right to left. The "0" mark of a turn always stands above the middle,
and the snaps count from the level. A line is turned only, not slanted.

A line or a dimension line without a length, both points on one spot, as a file may bring it,
shows its two ends on that spot and a pale handle above it, which does nothing. A press takes
the end on top; drag it out to give the object a length. On a dimension line without a length
the help line handles are pale as well, so the press reaches an end.

### The toolbar and the menus

Commands such as Group, Align, Distribute, Arrange (To Front, To Back), Position and Size,
Name, Description, Flip, To Curve, Area and Line are enabled from what is selected, in the
client. To Front is off when the selection is in front already, Flip is off for a text frame,
and so on.


## 3. Groups and diagrams

### Groups

A group is selected as one object. To work on what it holds you enter it.

| Enter a group | Leave a group |
|---|---|
| Double click an object of it | Escape (selects the group) |
| Enter or F3 with the group selected | Ctrl + F3 |
| Alt + click through the objects under the mouse | Click empty space |
| The Enter button beside its band | The Leave button beside its band |

- A rounded band goes round each group the selection stands in, one per level, and round each
  selected group. It shows how deep you are. The band always stands upright, also round a
  turned or slanted group.
- Inside a group, Tab, Ctrl + Home and Ctrl + End travel the objects of that group.
- Moving, scaling and turning a group changes all the objects it holds.
- While you stand in a group, everything else on the slide is drawn pale, mixed half way to
  white as in the desktop application. The same transparency is used for an object in front of
  a chart being edited (see Charts). An object in front of the group that covers part of it
  is also drawn transparent, so what you edit shows through. The colour, how pale, and how
  transparent are settings: vectorGhostColor (white), vectorGhostAmount (0.5) and
  vectorGhostTransparency (0.8).

### Charts

A double click or Return on a chart edits it in place, as in the desktop application. A turned
chart is shown upright while edited, and an object in front of it is drawn transparent.

Inside, the parts of the chart are selected as the chart itself selects them: a click takes the
part under it (a series first, a single data point with the next click), Tab and Shift+Tab go
through the parts of one level in the chart's own order, Return or F3 goes a level down - also
from data labels to a single label - and Escape a level up; Escape at the top leaves the chart
and keeps it selected. Return on a title starts its text edit.

A part the chart lets you move or resize (title, legend, diagram, a data label, a trend line's
equation, a shape added to the chart) has the eight handles: drag them, or move by arrow key
with the same steps as on the slide (Shift and Ctrl larger, Alt one pixel). A part that can be
neither moved nor resized (a series, its data labels, an axis, a grid) shows a dashed frame and
the chart's own marks, small squares on its data points or labels; a click on a mark takes the
point or label under it. A pie or donut segment is dragged out of its pie only along its own way.
Whatever you move stays inside the chart. Each change is one undo step, as in the chart.

A click outside on another object leaves the chart and selects that object, and can drag it at
once; a click outside on nothing leaves the chart and keeps it selected.

### Formulas

A double click or Return on a formula edits it in place, in the drawn formula itself: click
where the caret is to go, move it with the arrow keys, type and delete, and undo or redo what you
typed with Ctrl+Z and Ctrl+Y. The formula keeps the size it has on the slide while edited. The
sidebar shows the Elements deck to insert from. Escape or a click outside leaves it.

### Ungroup

Ungroup (toolbar, menu or the button) takes apart, each by one level:

- every selected group and diagram, other objects selected or not;
- with none of them selected, the group you stand in;
- on a node of a diagram, the diagram.

### Diagrams

A diagram is a set of shapes laid out from data. It is never selected on its own, but through
its objects.

- Click a diagram: its first object is selected, the shape that spans the whole diagram. It
  stands for the diagram: moving, scaling or turning it moves, scales or turns the whole
  diagram.
- Click a node: that node is selected. Nodes show pale handles, since the layout places them.
  Their text, name, description, area and line can be changed.
- Double click a node: edits its text.
- Tab / Shift + Tab: the next / previous object of the diagram; past the last you leave it.
- Escape: from a node to the whole diagram, then out of it.
- Shift + click or Ctrl + click adds the whole diagram to a selection only from outside it.
- Copy on a node copies that node as a plain shape.
- Deleting a node lays the diagram out again; names and descriptions of the other nodes stay.

### The buttons beside a band

A column of buttons stands beside the band of the outermost group or diagram that holds the
selection, at any depth. When several groups or diagrams are selected side by side, each gets
its own column.

| Button | For | What it does |
|---|---|---|
| Diagram | a diagram; a group holding one | Opens the diagram dialog. |
| Enter | a group | Enters the group. |
| Leave | a group | Leaves the group you stand in. |
| Ungroup | a group or a diagram | Takes it apart. |

- When the group or diagram itself is selected, its buttons act on it alone: it is selected on
  its own and entered, opened or taken apart.
- When the selection is inside it, the buttons do what the menu commands do.
- A pale button cannot be pressed now. Resting the pointer on a button shows its name.
- On a low band, buttons that do not fit are left out from the bottom up.
- The column hides while you drag, and comes back where the drag ends.

In the desktop application F3 and Ctrl + F3 do not arrive, so the buttons and the double click
are the ways into and out of a group there.


## 4. Connectors

How glue points are shown, as in the office: a glue point is a small circle. Every way a
connector may leave the object there puts a nose on the circle, a quarter of it replaced by two
lines out to a tip; a point with no way named is a plain circle, and a connector may leave it
in any direction. Small marks beside the circle show which side of the object the point's place
is measured from. The four points an object has at the middles of its sides are drawn in a
color of their own, since they are the object's defaults and not points someone added.

- Drag an end onto an object: it ties to the nearest glue point, or to the best side; the run
  is shown while dragging. Dropped elsewhere, the end stays free.
- A connector tied to an object follows it when the object moves.
- The handles on its lines move one line each, across that line only; the run of the drag is
  the run you get.
- While an end is dragged onto an object, every place it could tie to is marked.


## 5. What you see while you work, and why it is quick

### During a drag

- The objects themselves move with the mouse, drawn as they will be, not as an outline; a group
  or a diagram moves with all it holds.
- The band round a group or a diagram and the buttons beside it hide while the drag runs and
  come back where the drag left the group.
- Lines and dots show where the drag lines up with the edges and middles of other objects.
- Connectors tied to what is dragged are laid out anew at every step and follow along.
- While an end of a connector is dragged, the glue points of the object it is aimed at pop up,
  every place the end could tie to, and the one it would take is marked.
- While a line of a connector is dragged, the run it will get is shown.
- While a path point or a weight is dragged, the path is shown as it will be, with thin lines
  to the weights.
- While a picture is cropped, the part of the graphic outside the window is shown faded.
- Dragging past the edge of the view scrolls it: the further out the mouse is, the faster,
  starting gently and growing up to a fast scroll. The drag goes on under the mouse while the
  view moves, so you can drag an object to a place that was out of view.
- A side handle dragged past the opposite side mirrors the object, and the handle stays under
  the mouse; an object is never made thinner than a hair, so it does not vanish on the way.

### Other things you see

- The pointer over a handle says what it does: an arrow in the direction a framing handle
  scales, a hand over a handle that is dragged, the pointing finger over one that is only
  clicked, such as the select squares of a table, and a "no" sign where the object cannot be
  resized.
- The handles stand a little outside the frame and face outwards, also on a turned or mirrored
  object, so they do not hide path points or small objects.
- The handle the keyboard is on blinks larger, so you see which one the arrow keys move.
- The view follows what the keyboard does: a selection made by a key, an object moved by key
  and a handle moved by key are kept in view, at every step of a key held down.
- A dashed outline marks the object under the pointer. It follows the object's own shape, a
  circle round, a curve along its course, without the letters of its text.
- The toolbar and the menus change their states at once with the selection, since the client
  works them out itself.
- Changes made by other people editing the same document arrive as they happen. When they
  delete an object you have selected, your selection goes to what is left, staying in the
  group you stood in.

### No waiting for the engine

Everything shown during a drag is worked out in the client, from the objects it already holds.
No message goes to the engine and back while the mouse moves, so the drag follows the mouse
without delay however far away the engine is.

When the drag ends, the engine is told the result in one message: the new mapping of each
object, the points that moved, or the new crop window. The client does not wait for the answer.
It lays the change onto its own copy of the objects at once, so nothing jumps back. When the
engine answers, its version takes the place of the client's. Where the two agree, which is the
normal case, nothing visible happens. Where they do not, the engine's version is taken, since
the engine says what the document is.

Text is the one thing the client does not lay out itself. Until the engine answers, a run of
text keeps its glyphs and only moves and turns with the object; then the engine's new layout
replaces it.

### Holding a key down

Holding an arrow key repeats the step quickly, and every step is shown at once. What the keys
change is collected and sent to the engine only when the keys are let go, as the last state of
each object. So a long run of steps is one message and one undo step, and the engine is never
flooded with steps it would have to work through one by one. Separate presses stay separate
undo steps. Only a few changes are on their way to the engine at a time, so a slow engine is not
given a queue to catch up on.

This holds for every handle: the frame, the turn handle, the crop, and also the handles the
engine shapes itself - a corner radius, a circle angle, a custom shape point, a callout tail, a
connector end or middle, a glue point, a point or a weight of a path. Those stay where the keys
took them until the engine answers, with the object shown thin as it will be.

Holding Ctrl + Z or Ctrl + Y works the same way: the first undo goes out at once, and the
repeats go out as one undo of that many steps when you let the key go.

### On a slow connection

Selecting does not wait for the engine either. A click, a key like Tab, or a click on another
object inside the frame of the selected one shows the new selection with its handles at once,
and the engine's late reports of earlier selections do not take it back. Clicks on different
objects in quick succession are single clicks, not a double click.

An undo or a redo is the one change the client cannot predict, since only the engine knows its
undo history. While one is on its way, the arrow keys and the 0 key wait, and a drag does not
start. The keys pressed meanwhile are carried out, in order, once the engine has answered, from
where the undo left the objects. If the answer takes longer than a second, a short notice below
the selection says so ("Waiting for undo..."), and fades away. On a connection of usual speed
the answer comes long before that, and the notice is never seen. The second and how long the
notice is seen can be set (chapter 7).

To try this out, the debug panel has "Delay messages", which holds back every message in both
directions by the round trip you give it.

### Like predictive text

This works like predictive text, or like a remote terminal that echoes your typing before the
far end answers: the client predicts what the engine will make of a change, shows the
prediction at once, and checks it against the answer when that arrives. The prediction is
almost always right, because the client does the same geometry the engine does; where it is
not, the answer simply replaces it.


## 6. Object kinds

What each kind of object looks like in vector mode and what you can do with it. The details are
in the sections above.

| Kind | Shown as | What you can do | Different from the office |
|------|----------|-----------------|---------------------------|
| Rectangle, ellipse | As in the office | Eight handles, turn, shear, text; a rectangle's corner radius handle | - |
| Circle section, arc, segment | As in the office | As a rectangle, plus the start and end angle handles | - |
| Line | As in the office | Its two ends, the turn handle beside its middle; 0 lays it level (see Lines) | - |
| Polyline, polygon, curve | As in the office | Points and weights with Edit Points, moved by drag and key, and edited by their menu (see Editing points) | - |
| Freehand line | As in the office | As a curve | Not checked yet |
| Text box, title, outline | As in the office | As a rectangle; double click, typing, F2 or Return edits the text | - |
| Callout | As in the office | As a rectangle, plus the tail end; the tail is laid out live | - |
| Custom shape, Fontwork | As in the office | As a rectangle, plus the yellow handles on their rails | No live shape while a yellow handle is dragged |
| Dimension line | As in the office | Its ends, help lines and the line itself (see Dimension lines) | - |
| Connector | As in the office | Ends and middle handles, tied to glue points (see section 4) | - |
| Group, diagram | As in the office, with a band | Enter, leave, the band's buttons (see section 3) | - |
| Picture | As in the office | As a rectangle, plus the crop window | Shear by key past 90 degrees mirrors once more |
| Chart | As in the office | Edit in place, its parts (see Charts) | - |
| Formula | As in the office | Edit in place in the drawn formula (see Formulas) | - |
| Table | As in the office | Eight handles; the table markers (see Tables below) | Light lines between cells come and go with the zoom, in the office too |
| Form control | Drawn in Online's look (see below) | In design mode select, move, scale, undo, as any object; out of it they work | Not every kind works yet out of design mode |
| 3D scene | As one picture | Select, move, scale | Nothing inside can be selected yet; no 3D turn |
| Video, audio | Filled black | Select, move, scale | The desktop application on Qt 6.10 may crash when a video is selected |

### Tables

A selected table shows, besides its eight handles, the markers of the table, outside the eight so
that nothing lies on top of another. They are handles like the others: they light up under the
mouse, Ctrl + Tab walks over them, and the one the keyboard is on blinks. A table has no turn
handle, as it cannot be turned.

| Marker | Where | What it does |
|---|---|---|
| Anchor | At the upper left corner | Drag it to move the table; the arrow keys on it move the table too. |
| Select square | Above each column, beside each row | A click selects the cells of that column or row; a click on the one selected alone takes the selection away again. With Shift a column or row next to the selection joins it, one at its edge leaves it. Delete deletes that column or row, or all the selected ones it is part of; deleting the last one deletes the table, as elsewhere in Online. |
| Border marker | Above each border between columns and at the right end, beside each border between rows and at the bottom | Drag it to move that border: a line across the table shows where it goes, and it stays a little away from the borders beside it. The arrow keys on it move the border a step. The upper and the left border move with the table and have no marker. |
| Insert bar | At the right and below the table | A click adds a column or a row after the one the text cursor stands in. |

The arrow keys on a select square or an insert bar move the table, as on the anchor. Return or
Space on a marker does what a click does. A click on a marker counts only where the mouse is let
go on the same marker, as with a button.

The edge of the table, a narrow band just inside its outline, takes hold of the table to move it;
the markers go along with it at once. Inside the cells:

| What | What happens |
|---|---|
| Click into a cell | Starts its text edit. Tab goes to the next cell, Shift + Tab to the one before; Tab in the last cell adds a new row, as in the office. |
| Drag from one cell into others | Selects the cells from the cell it started in to the one under the mouse. A drag within one cell selects its text. |
| Shift + click on another cell | Selects the cells from the edited cell, or from where the last selection of cells started, to the clicked one. In the edited cell itself Shift + click selects text. |
| Shift + arrow keys in a cell | Selects cells, as in the office. |
| Escape | Leaves the text edit; a second Escape leaves the table. |

### Form controls

Buttons, check boxes, option buttons, labels, group boxes, text, number, currency, date, time and
pattern fields, file selection, combo and list boxes, image controls, scroll bars, spin buttons,
table controls and navigation bars are drawn in Online's own look, sharp at any zoom, showing
their text, their value and their state as the document holds them. A document opens them in
the mode its file asks for, a new one in design mode. In design mode a click selects a control
like any object, and it is moved, scaled and undone as one. They cannot be turned or sheared, as
in the office.

The properties of a control are set in the office's own dialog, in Online's look:

| How | What happens |
|---|---|
| Double-click a control | Opens its Control Properties. |
| Right-click a control | The menu offers Control Properties and Form Properties, the second for the form the control belongs to. |

Control Properties has two tabs. General holds the label, the font, the colours, the alignment,
whether the control is enabled, the entries of a list and the rest; Events binds macros to what
happens with the control. A change is taken when Return is pressed or the field is left, and the
control on the slide shows it at once; it can be undone like any other change. A label or list
of several lines is edited in the small editor behind the arrow beside it. The dialog stays open
while another control is selected, and then shows that one. A URL is typed; there is no button
to pick a file.

Design mode can be switched on and off (the place and the icon are not final, the icon is the
office's own and does not match Online's look yet). The switch shows only in a document that
holds form controls and can be edited, with tiles as well as in vector mode:

| How | What happens |
|---|---|
| Insert tab, Design Mode (notebookbar) | Switches design mode; the button shows whether it is on. |
| Tools, Design Mode (compact menu) | The same, with a check mark. |
| Console: app.map.isFormControlDesignMode() | Says whether design mode is on. |
| Console: app.map.setFormControlDesignMode(true or false) | Switches it on or off. |

With design mode off, the controls cannot be selected, as in the office, and a click on one does
not reach what lies behind it. In the vector view the edited slide then shows them as working
controls in Online's look, over the page; they follow scrolling and zoom. What is done with one
goes to the office, which operates its own control, so the form's events and macros run as there.
Working so far: push buttons and image buttons (pressed), check boxes and option buttons, and the
text, formatted, numeric, currency, date, time and pattern fields: click into one and type; Return
or leaving the field sends the text, which the field reads in its own format, and Escape takes
back what was typed. A list box selects the entry clicked; where it takes several, Ctrl+click
adds an entry or takes it away. A list that drops down opens the browser's own list on a click.
A combo box is typed into like a field, and its arrow opens its entries to choose from. A scroll
bar goes a step with its arrows, a page with a click on its track, and follows its thumb when it
is dragged. A spin button adds or takes a step with its arrows; a macro on its adjusting event
does not run yet. A file selection takes a typed path; its Browse button is off, as there is no
file picker for it. A field with spin buttons steps its value with them, or with the up and down
arrow keys while the cursor is in it; what was typed counts first. The arrow of a date field opens
a calendar of the month of its date, with arrows to the month before and after; a click on a day
sets it. The table control and the navigation bar work on the records of a database and are shown
only. The slide previews, the presentation and the other users
in design mode show the controls drawn.

## 7. Settings

Some values of what you see and of how the mouse and the keys work can be set. They are
client preferences, kept as the other settings of Online are: in the browser's local storage,
with the user's settings on the server, and in the desktop application's own settings.

### Setting a value

There is no dialog for them yet. Open the browser's developer console and write:

    window.prefs.set('vectorKeyStep', 2)

The value counts at once, without a reload, and is kept for the next time. To go back to the
default:

    window.prefs.remove('vectorKeyStep')

The desktop application has no console of its own. Start it with
QTWEBENGINE_REMOTE_DEBUGGING=9222 in front, as CODA_VECTOR above, and open
http://localhost:9222 in a Chromium-based browser; its console runs in the page.

A number that is not valid for a setting (nought or below where it must be above nought, a
share outside 0 to 1, a colour that is not #rrggbb) is ignored, and the default counts.
The settings count only in vector mode; with tiles everything stays as it was.

### Moving, scrolling and snapping

A distance in millimetres grows with the page: up to ten times the size of an A4 page it is
as given, from ten times on it is ten times that, from a hundred times on a hundred times
that, and so on. Alt with an arrow key always moves or scrolls by one pixel, and is not a
setting.

| Name | Default | What it is |
|---|---|---|
| vectorKeyStep | 1 | How far an arrow key moves the selection or a handle, in millimetres. |
| vectorKeyStepShift | 10 | The same with Shift, in millimetres. |
| vectorKeyStepControl | 25 | The same with Ctrl, in millimetres. |
| vectorScrollStep | 0.05 | How far an arrow key scrolls the view when nothing is selected, as a share of what is in view. |
| vectorScrollStepShift | 0.25 | The same with Shift. |
| vectorScrollStepControl | 0.5 | The same with Ctrl. |
| vectorLengthSnapControl | 10 | The step the length of a line snaps to while Ctrl is held and one of its ends is dragged, in millimetres. |
| vectorLengthSnapAlt | 1 | The same with Alt, in millimetres. |
| vectorSnapDistance | 10 | How near an edge or the middle of a dragged object comes to one of another object before the drag lines up with it, in screen pixels. 0 never lines up. |

### Turning and shearing

The angles are counted from the page, so a turn snaps to 0, 45, 90 degrees and so on, not to
steps from where the object stood.

| Name | Default | What it is |
|---|---|---|
| vectorTurnSnapControl | 45 | The step a turn or a shear snaps to with Ctrl, in degrees. |
| vectorTurnSnapShift | 15 | The same with Shift, in degrees. |
| vectorTurnSnapAlt | 5 | The same with Alt, in degrees. With no key held the mouse turns freely. |
| vectorKeyTurnStep | 1 | The step an arrow key turns or shears by on the handle above the selection, with no key held, in degrees. With Ctrl, Shift or Alt the key goes to the next step of the three above. |
| vectorOtherModeDelay | 2000 | How long the mouse rests on the handle above the selection before the other mode's symbol comes up beside it, in milliseconds. 0 shows it at once. |
| vectorAngleAtMouse | true | Shows the angle beside the mouse while turning or shearing. |
| vectorAngleAtHandle | true | Shows the turn or the slant of the selection beside the handle above it, while nothing is dragged. |

### Selecting

| Name | Default | What it is |
|---|---|---|
| vectorHitTolerance | 8 | How far beside a line or an edge a click still hits it, in screen pixels. It is the same at every zoom. |
| vectorHandleSize | 12 | How wide a handle is, in screen pixels, 4 or more. The other kinds of handle (the points of a path, the tie points) and the band round a group keep their size in relation to it. |

### Waiting for the engine

| Name | Default | What it is |
|---|---|---|
| vectorHoldBackToldAfter | 1000 | How long an undo or a redo has been on its way before a key or a drag held back for it is told by a notice below the selection, in milliseconds. A key held back for less time is held back without a word. |
| vectorHoldBackSeen | 700 | How long that notice is fully seen, in milliseconds. |
| vectorHoldBackFading | 400 | How long it then takes to fade away, in milliseconds, 1 or more. |
| vectorCaretRestDelay | 300 | How long the caret moved in a text rests before the engine is told where it is, in milliseconds. The engine then shows it to the toolbar and the other views. |

### Editing inside a group

While a group or a diagram is entered, the objects outside it are drawn pale, as the office
draws them.

| Name | Default | What it is |
|---|---|---|
| vectorGhostColor | #ffffff | The colour the objects outside are mixed towards, as #rrggbb. |
| vectorGhostAmount | 0.5 | How far they are mixed towards that colour, from 0 (not at all) to 1 (all of it). |
| vectorGhostTransparency | 0.8 | How transparent the objects outside are where they lie in front of the group, from 0 (not at all) to 1 (not seen). |


## 8. Not there yet

What is still missing, or only partly there:

- 3D objects and 3D charts: they are shown as one picture. Selecting single objects inside a
  scene, and turning a scene in 3D as the office does, are planned.
- Thin lines: a hairline running across or down can land between two pixels and look pale or
  vanish at some zooms. Placing such lines on whole pixels, as the office does, is planned, and
  drawing the cells of a table without the light seams between them.
- Form controls: the picture of an image button or image control is not shown yet, and a simple
  look as a setting is planned.

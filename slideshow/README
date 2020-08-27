The Impress slideshow engine

== 3D transitions ==

The 3D transitions are slideshow transition engine using OpenGL and
are located in slideshow/source/engine/OGLTrans/. They were initially
written by GSOC student Shane.M.Mathews. Radek has later polished the
code a bit, added few new 3D transitions, added infrastructure for
vertex and fragment shaders. Wrote few transitions with fragment shader
too.

== Physics Animation Effects ==

Physics animation effects are simulated by external 2d physics engine
library Box2D. They don't directly call Box2D functions but instead
use the wrapper in:
* slideshow/source/inc/box2dtools.hxx
* slideshow/source/engine/box2dtools.cxx

The wrapper has two corresponding classes to manage the Box2D world
and Box2D bodies.

When a physics animation starts, a Box2DWorld is initiated and
populated with every shape that is part of the foreground (which are
shapes that do not belong to the master slide and not a background
shape).

After creation until the end of the slide (not the whole slideshow)
the Box2D World isn't destroyed and reused. But the bodies that
represent the shapes in the slide get destroyed when there's a point
in time that there's no physics animation in progress. And recreated
when another physics animation starts.

If there are multiple physics animations in parallel only one of them
takes the role of stepping through the simulation.

If there are other animation effects that go in parallel which change
the shape position, rotation, or visibility - they also report the
change to Box2D World. These updates are collected in a queue in
Box2DWorld and processed before stepping through the simulation.
To achieve convincing results these updates are performed by setting
the box2d body's linear velocity or angular velocity instead of
setting directly it's position or rotation.

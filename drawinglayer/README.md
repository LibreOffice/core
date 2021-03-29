# Drawing API

Drawing API that can specify what to draw via a kind of display list.

Example of the DrawingLayer use is eg. in `svx/source/xoutdev/xtabhtch.cxx:121`.
A stripped down version with extended comments:

     // Create a hatch primitive (here a rectangle that will be filled with
     // the appropriate hatching, but has no border).
     // This will not draw it yet; it's so far only constructed to add it to a
     // display list later.
     const drawinglayer::primitive2d::Primitive2DReference aHatchPrimitive(
         new drawinglayer::primitive2d::PolyPolygonHatchPrimitive2D(...));

     // Create a rectangle around the hatch, to give it a border.
     const drawinglayer::primitive2d::Primitive2DReference aBlackRectanglePrimitive(
         new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(...));

     // Here we want to render to a virtual device (to later obtain the bitmap
     // out of that), so prepare it.
     VirtualDevice aVirtualDevice;

     // Create processor and draw primitives, to get it ready for rendering.
     std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(
         drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(...));

     if (pProcessor2D)
     {
         // Fill-in the display list.
         drawinglayer::primitive2d::Primitive2DSequence aSequence(2);

         aSequence[0] = aHatchPrimitive;
         aSequence[1] = aBlackRectanglePrimitive;

         // Render it to the virtual device.
         pProcessor2D->process(aSequence);
         pProcessor2D.reset();
     }

     // Obtain the bitmap that was rendered from the virtual device, to re-use
     // it in the widget.
     aRetval = aVirtualDevice.GetBitmap(Point(0, 0), aVirtualDevice.GetOutputSizePixel());

## DrawingLayer Glossary

Primitives - classes that represent what should be drawn.  It holds the data
what to draw, but does not contain any kind of the rendering.  Some of the
primitives are 'Basic primitives', that are primitives that cannot be
decomposed.  The rest of the primitives can be decomposed to the basic
primitives.

Decomposition - a way how to break down the more complicated primitives into
the basic primitives, and represent them via them; this logically makes the
plain `Primitive2DSequence` display list a hierarchy.
Eg. `PolygonMarkerPrimitive2D` can be decomposed to 2 hairlines
`PolyPolygonHairlinePrimitive2D`'s, each with different color.

Processor - a class that goes through the hierarchy of the Primitives, and
renders it some way.  Various processors, like `VclPixelProcessor2D` (renders to
the screen), `VclMetafileProcessor2D` (renders to the VCL metafile, eg. for
printing), etc.

## How to Implement a New Primitive ("Something New to Draw")

* Create an ancestor of `BasePrimitive2D`
  (or of its ancestor if it fits the purpose better)

  * Assign it an ID [in `drawinglayer_primitivetypes2d.hxx`]

  * Implement its decomposition
    [`virtual Primitive2DSequence create2DDecomposition(...)`]

* Extend the (various) processor(s)
  If you need more than relying on just the decomposition

## Where is DrawingLayer Used

* `SdrObject`(s) (rectangles, Circles, predefined shapes etc.)

* Selections

* Various smaller cases to 'just draw something'

  * Draw to a virtual device, and use the resulting bitmap (like the example
    above)

* Custom widgets (like the Header / Footer indicator button)

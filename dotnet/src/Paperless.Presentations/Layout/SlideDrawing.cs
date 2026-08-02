using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.Presentations.Layout;

/// <summary>
/// Turns a laid-out slide into drawing commands.
/// </summary>
/// <remarks>
/// <para>
/// Background, then each shape's fill, its outline and its text, in the order the file states
/// its shapes. That order is z-order in both PresentationML and ODF — a shape tree is written
/// back to front — so nothing here sorts anything.
/// </para>
/// <para>
/// Fill before line before text, per shape rather than per slide. A stroke is centred on its own
/// path, so half of it overlaps the fill it belongs to; drawing every fill first and then every
/// outline would put a shape's outline on top of the shape in front of it.
/// </para>
/// </remarks>
public static class SlideDrawing
{
    /// <summary>Draws a slide into a sink.</summary>
    /// <param name="slide">The slide to draw.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public static void Draw(LaidOutSlide slide, IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(slide);
        ArgumentNullException.ThrowIfNull(sink);

        sink.BeginPage(slide.Size);
        try
        {
            if (slide.Background is { } background)
            {
                sink.FillPath(
                    GraphicsPath.Rectangle(new DocRect(DocPoint.Origin, slide.Size)), background);
            }

            foreach (PlacedShape shape in slide.Shapes) DrawShape(shape, sink);
        }
        finally
        {
            // Always closed, even when a sink throws part way through: a page left open makes the
            // next one nest inside it, turning one bad slide into a broken document.
            sink.EndPage();
        }
    }

    private static void DrawShape(PlacedShape shape, IDrawingSink sink)
    {
        if (shape.Fill is { } fill) sink.FillPath(shape.Outline, fill);
        if (shape.Picture is { } picture) DrawPicture(shape, picture, sink);
        if (shape.Line is { } line) sink.StrokePath(shape.Outline, line);

        if (shape.Text is not { Runs.Count: > 0 } text) return;

        // An upright shape's runs are already in slide coordinates, so the common case costs no
        // state-stack traffic at all — and, more usefully, its pens land in a backend's output as
        // plain numbers that compare directly against a reference renderer's.
        if (text.IsUpright)
        {
            foreach (PlacedGlyphRun run in text.Runs) DrawRun(run, sink);
            return;
        }

        sink.Save();
        try
        {
            sink.Transform(text.Transform);
            foreach (PlacedGlyphRun run in text.Runs) DrawRun(run, sink);
        }
        finally
        {
            sink.Restore();
        }
    }

    /// <summary>
    /// Draws one run and the rules under and through it.
    /// </summary>
    /// <remarks>
    /// The rules go on after the glyphs and in the run's own colour, which is what every renderer
    /// here does with a decoration: they are filled rectangles rather than strokes so that a
    /// half-point rule stays a half-point rule instead of becoming the device's thinnest line.
    /// </remarks>
    private static void DrawRun(in PlacedGlyphRun run, IDrawingSink sink)
    {
        sink.DrawGlyphRun(run.Run, Paint.Solid(run.Colour));

        if (run.Rules is not { Count: > 0 } rules) return;

        foreach (DocRect rule in rules)
        {
            sink.FillPath(
                new GraphicsPath()
                    .MoveTo(new DocPoint(rule.X, rule.Y))
                    .LineTo(new DocPoint(rule.X + rule.Width, rule.Y))
                    .LineTo(new DocPoint(rule.X + rule.Width, rule.Y + rule.Height))
                    .LineTo(new DocPoint(rule.X, rule.Y + rule.Height))
                    .Close(),
                Paint.Solid(run.Colour));
        }
    }

    /// <summary>
    /// Draws a picture inside the shape that holds it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Always clipped to the outline, for two reasons that both matter. A crop is expressed by
    /// drawing the whole picture into a rectangle larger than the frame, so without a clip a
    /// cropped picture spills across the slide; and a picture frame need not be rectangular —
    /// PowerPoint will put a photograph inside any preset geometry — so the outline is what
    /// bounds it rather than the destination rectangle.
    /// </para>
    /// <para>
    /// Between the fill and the line: the fill is behind a picture with transparency, and the
    /// frame's own border is drawn over the picture's edge, which is what puts a hairline
    /// exactly on the boundary rather than half under it.
    /// </para>
    /// </remarks>
    private static void DrawPicture(PlacedShape shape, PlacedPicture picture, IDrawingSink sink)
    {
        if (picture.Destination.IsEmpty) return;

        sink.Save();
        try
        {
            sink.ClipPath(shape.Outline);

            // The vector wins where a shape has both, which is the DrawingML `svgBlip` case and
            // nothing else; an empty decode falls back to the raster the file put there for it.
            //
            // `VectorImage.Draw` stretches the picture's own *frame* onto the destination and not
            // the extent of its ink, which is the same rule `DrawImage` follows for a raster and
            // is why a crop is expressed here as a larger destination plus a clip rather than as
            // anything the picture knows about.
            if (picture.Vector is { } vector && !vector.Value.IsEmpty)
            {
                vector.Value.Draw(sink, picture.Destination);
            }
            else if (picture.Image is { } image)
            {
                sink.DrawImage(image, picture.Destination, picture.Opacity);
            }
        }
        finally
        {
            sink.Restore();
        }
    }
}

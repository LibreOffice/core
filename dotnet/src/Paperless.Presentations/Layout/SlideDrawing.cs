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
        if (shape.Line is { } line) sink.StrokePath(shape.Outline, line);

        if (shape.Text is not { Runs.Count: > 0 } text) return;

        // An upright shape's runs are already in slide coordinates, so the common case costs no
        // state-stack traffic at all — and, more usefully, its pens land in a backend's output as
        // plain numbers that compare directly against a reference renderer's.
        if (text.IsUpright)
        {
            foreach (PlacedGlyphRun run in text.Runs) sink.DrawGlyphRun(run.Run, Paint.Solid(run.Colour));
            return;
        }

        sink.Save();
        try
        {
            sink.Transform(text.Transform);
            foreach (PlacedGlyphRun run in text.Runs) sink.DrawGlyphRun(run.Run, Paint.Solid(run.Colour));
        }
        finally
        {
            sink.Restore();
        }
    }
}

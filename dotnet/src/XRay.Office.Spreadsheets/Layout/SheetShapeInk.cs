using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// Paints a worksheet shape's interior and outline into the rectangle its anchor gave it.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A worksheet shape's fill and outline were read in no format at all, and no gate column
/// can see one.</strong> A fill adds no glyphs and no pages, so a page-count-and-text harness
/// scores a sheet of blank stars exactly as it scores a sheet of coloured ones — the
/// <c>w:pgBorders</c> shape again. Censused whole-corpus over the <c>xlsx</c> family,
/// <b>644 <c>xdr:sp</c> in 49 documents</b>: 421 state an <c>a:solidFill</c> of their own, 205 an
/// <c>a:ln/a:solidFill</c>, and 583 an <c>xdr:style</c> naming the theme's format matrix.
/// </para>
/// <para>
/// <strong>The outline is the preset's, not the anchor's box.</strong> Those 644 shapes name 28
/// distinct presets and only 206 of them are <c>rect</c>, so painting the box fills a rectangle
/// where the file states a star, a heart or a cloud. <see cref="CustomShapeGeometry"/> resolves
/// all of them, and its two outlines are used rather than one: a subpath states whether it is
/// filled and whether it is stroked, so every connector — one open subpath declaring
/// <c>fill="none"</c> — would otherwise be painted as a solid blob, and the shading faces of
/// <c>cube</c>, <c>can</c> and <c>bevel</c> would be ruled into a wireframe.
/// </para>
/// <para>
/// <strong>It is painted under the shape's own text and over the cells.</strong> Calc prints the
/// back drawing layer, the cell backgrounds, the strings, the grid and only then
/// <c>PrintDrawingLayer(SC_LAYER_FRONT)</c> (<c>sc/source/ui/view/printfun.cxx</c>:1651-1703), so a
/// filled shape covers the cells it sits on; <see cref="SheetPageGraphics"/> already draws after
/// the strings for that reason, and the text goes on last inside this shape's own box.
/// </para>
/// </remarks>
internal static class SheetShapeInk
{
    /// <summary>
    /// The width an outline is stroked at when the file states a hairline.
    /// </summary>
    /// <remarks>
    /// A comment caption's border is <c>svg:stroke-width="0in"</c> in LibreOffice's own export,
    /// which is a hairline rather than an absent line — the same convention the page furniture
    /// already draws its rules at.
    /// </remarks>
    private static readonly Length HairlineWidth = Length.FromPoints(0.1);

    /// <summary>Paints one shape's fill and outline.</summary>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="box">Where the shape lands on the page, already scaled.</param>
    /// <param name="fill">The flat interior colour, or null.</param>
    /// <param name="gradient">The interior ramp, or null; placed against <paramref name="box"/>.</param>
    /// <param name="texture">The interior tile, or null; also placed against <paramref name="box"/>.</param>
    /// <param name="stroke">The outline colour, or null.</param>
    /// <param name="width">How wide to stroke it; zero for a hairline.</param>
    /// <param name="preset">The preset whose outline to paint through, or null for the box.</param>
    /// <param name="adjustments">The preset's stated adjustment handles, or null for its defaults.</param>
    /// <param name="flipHorizontal">Mirror the outline across the box's vertical centre line.</param>
    /// <param name="flipVertical">Mirror it across the horizontal one.</param>
    /// <param name="scale">The print zoom, applied to the stroke width.</param>
    public static void Draw(
        IDrawingSink sink,
        DocRect box,
        Colour? fill,
        GradientDescription? gradient,
        SheetShapeTexture? texture,
        Colour? stroke,
        Length width,
        string? preset,
        IReadOnlyDictionary<string, double>? adjustments,
        bool flipHorizontal,
        bool flipVertical,
        double scale)
    {
        ArgumentNullException.ThrowIfNull(sink);

        if (fill is null && gradient is null && texture is null && stroke is null) return;
        if (box.Width <= Length.Zero || box.Height <= Length.Zero) return;

        (GraphicsPath filled, GraphicsPath stroked) =
            Outlines(box, preset, adjustments, flipHorizontal, flipVertical);

        if (gradient is { } ramp) sink.FillPath(filled, ramp.Paint(box));
        else if (texture is { } tiled) sink.FillPath(filled, Tiled(tiled, box, scale));
        else if (fill is { } colour) sink.FillPath(filled, Paint.Solid(colour));

        if (stroke is not { } line) return;

        Length pen = width > Length.Zero
            ? Length.FromEmu((long)Math.Round(width.Emu * scale))
            : Length.Zero;

        sink.StrokePath(
            stroked, new Stroke(Paint.Solid(line), pen > Length.Zero ? pen : HairlineWidth));
    }

    /// <summary>The paint a <see cref="SheetShapeTexture"/> describes, placed on its box.</summary>
    /// <remarks>
    /// The tile grid is anchored on the shape's own origin rather than on the page's, which is
    /// what a fill measured in the object's coordinates means and what the reference draws: two
    /// identical shapes at different offsets show the pattern in the same place relative to their
    /// own edges.
    /// </remarks>
    private static BitmapPaint Tiled(SheetShapeTexture texture, DocRect box, double scale)
    {
        bool stretch = texture.Tile.Width <= Length.Zero || texture.Tile.Height <= Length.Zero;

        DocSize tile = new(
            Length.FromEmu((long)Math.Round(texture.Tile.Width.Emu * scale)),
            Length.FromEmu((long)Math.Round(texture.Tile.Height.Emu * scale)));

        return new BitmapPaint(
            texture.Image,
            stretch ? box.Size : tile,
            box.Origin,
            stretch)
        {
            Opacity = texture.Opacity,
        };
    }

    /// <summary>
    /// The path to fill and the path to stroke, in page coordinates.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The box arrives already scaled by the print zoom and a preset's geometry is linear in its
    /// size, so evaluating it against the scaled box gives the scaled outline with no second
    /// conversion — the same argument <see cref="SheetShapePainter"/> makes for the text
    /// rectangle.
    /// </para>
    /// <para>
    /// A preset the catalogue does not know, and a shape that states none, both fall back to the
    /// rectangle, which is what LibreOffice falls back to as well and what every BIFF shape gets:
    /// the Escher path states a shape type rather than a DrawingML preset name.
    /// </para>
    /// </remarks>
    private static (GraphicsPath Fill, GraphicsPath Stroke) Outlines(
        DocRect box,
        string? preset,
        IReadOnlyDictionary<string, double>? adjustments,
        bool flipHorizontal,
        bool flipVertical)
    {
        if (preset is not { Length: > 0 } named) return Box(box);

        if (CustomShapeGeometry.Preset(named, new DocSize(box.Width, box.Height), adjustments)
            is not { } geometry)
        {
            return Box(box);
        }

        return (Placed(geometry.FillOutline, box, flipHorizontal, flipVertical),
                Placed(geometry.StrokeOutline, box, flipHorizontal, flipVertical));

        static (GraphicsPath, GraphicsPath) Box(DocRect area)
        {
            GraphicsPath rectangle = GraphicsPath.Rectangle(area);
            return (rectangle, rectangle);
        }
    }

    /// <summary>
    /// A path in the shape's own coordinates, mirrored if the shape says so and moved to where
    /// the anchor put it.
    /// </summary>
    /// <remarks>
    /// The mirror is inside the box rather than around the origin, so it composes with the
    /// translation in one pass and leaves the anchor's rectangle exactly where it was — which is
    /// what <c>a:xfrm/@flipH</c> means: the rectangle does not move, its contents are reflected in
    /// it.
    /// </remarks>
    private static GraphicsPath Placed(
        GraphicsPath path, DocRect area, bool flipHorizontal, bool flipVertical)
    {
        GraphicsPath placed = new();
        foreach (PathCommand command in path.Commands)
        {
            switch (command.Verb)
            {
                case PathVerb.MoveTo: placed.MoveTo(Shift(command.Point)); break;
                case PathVerb.LineTo: placed.LineTo(Shift(command.Point)); break;
                case PathVerb.CubicTo:
                    placed.CubicTo(
                        Shift(command.Control1), Shift(command.Control2), Shift(command.Point));
                    break;
                case PathVerb.Close: placed.Close(); break;
                default: break;
            }
        }

        return placed;

        DocPoint Shift(DocPoint point) => new(
            flipHorizontal ? area.X + area.Width - point.X : area.X + point.X,
            flipVertical ? area.Y + area.Height - point.Y : area.Y + point.Y);
    }
}

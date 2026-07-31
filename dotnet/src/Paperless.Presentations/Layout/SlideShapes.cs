using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.Presentations.Layout;

/// <summary>
/// A slide after layout: its size, its background, and its shapes in paint order.
/// </summary>
/// <remarks>
/// <para>
/// The presentation family's equivalent of a word processor's laid-out page, and much smaller,
/// because almost nothing on a slide flows. A word-processing page is the result of deciding
/// <em>where</em> content goes; a slide is told, so the work is resolving what a shape's
/// placement and formatting are rather than choosing them. What remains genuinely laid out is
/// the text inside each shape, which is why <see cref="PlacedShape.Text"/> holds glyph runs and
/// everything else holds geometry.
/// </para>
/// <para>
/// Shapes are held in the order the file states them, which is paint order: PresentationML and
/// ODF both write a shape tree in z-order, back to front.
/// </para>
/// </remarks>
public sealed record LaidOutSlide
{
    /// <summary>The zero-based position in the deck.</summary>
    public required int Index { get; init; }

    /// <summary>The slide's size, which every slide in a deck shares.</summary>
    public required DocSize Size { get; init; }

    /// <summary>The slide's name, or null when the file states none.</summary>
    public string? Name { get; init; }

    /// <summary>True when the slide is skipped during a show.</summary>
    /// <remarks>
    /// Carried rather than acted on. A hidden slide is still a page of the document — it is
    /// what <c>--convert-to pdf</c> leaves out and what an editor shows greyed — so the
    /// decision belongs to whoever asked for the layout.
    /// </remarks>
    public bool IsHidden { get; init; }

    /// <summary>
    /// The background fill.
    /// </summary>
    /// <remarks>
    /// White rather than null when nothing in the slide, its layout or its master states one,
    /// because that is what a slide is: LibreOffice paints the page colour as a full-sheet
    /// rectangle on every slide of every deck, and a caller rasterising one that painted nothing
    /// would get transparency where a viewer shows paper. Null means no background at all, which
    /// is what an explicit <c>a:noFill</c> asks for.
    /// </remarks>
    public Paint? Background { get; init; } = Paint.Solid(Colour.White);

    /// <summary>The shapes, back to front.</summary>
    public IReadOnlyList<PlacedShape> Shapes { get; init; } = [];
}

/// <summary>
/// One shape, placed: its outline in slide coordinates, how it is painted, and its text.
/// </summary>
/// <remarks>
/// <para>
/// The outline is already in slide coordinates — flipped, rotated and mapped out of whatever
/// groups contained it. Keeping the transform separate and letting a backend apply it would
/// mean every consumer had to compose the same matrix chain, and a comparison against a
/// reference renderer's PDF would be comparing our coordinates in some other space against its
/// coordinates on the page.
/// </para>
/// <para>
/// The text is the exception, and <see cref="PlacedText.Transform"/> says why: glyph positions
/// inside a rotated shape cannot be baked into slide coordinates, because a glyph run carries
/// an origin and advances rather than a matrix. So an upright shape's text is in slide
/// coordinates and a rotated one's is in the shape's own, with the matrix that takes it there.
/// </para>
/// </remarks>
public sealed record PlacedShape
{
    /// <summary>The shape's author-visible name, for diagnostics and tests.</summary>
    public string? Name { get; init; }

    /// <summary>The outline, in slide coordinates.</summary>
    public required GraphicsPath Outline { get; init; }

    /// <summary>
    /// The rectangle the shape occupies before any rotation, in slide coordinates.
    /// </summary>
    /// <remarks>
    /// The <c>a:off</c>/<c>a:ext</c> pair mapped through the groups above it, which is what
    /// text layout is measured against and what a placement comparison asserts on. Not the
    /// outline's bounding box: a rotated rectangle's bounding box is larger than the rectangle.
    /// </remarks>
    public DocRect Bounds { get; init; }

    /// <summary>How the shape is filled, or null when it is not.</summary>
    public Paint? Fill { get; init; }

    /// <summary>The pen its outline is drawn with, or null when it has none.</summary>
    public Stroke? Line { get; init; }

    /// <summary>The text inside it, or null when it holds none.</summary>
    public PlacedText? Text { get; init; }
}

/// <summary>
/// A shape's text after layout: glyph runs, and the transform that puts them on the slide.
/// </summary>
/// <param name="Runs">The runs, in paint order.</param>
/// <param name="Transform">
/// The matrix taking the runs' coordinates to the slide's. The identity for an upright shape,
/// in which case the runs are already in slide coordinates and a backend needs no state stack.
/// </param>
public sealed record PlacedText(
    IReadOnlyList<PlacedGlyphRun> Runs,
    AffineTransform Transform)
{
    /// <summary>True when the runs are already in slide coordinates.</summary>
    public bool IsUpright => Transform.IsIdentity;
}

/// <summary>One glyph run and the colour it is drawn in.</summary>
/// <param name="Run">The run.</param>
/// <param name="Colour">Its colour.</param>
public readonly record struct PlacedGlyphRun(GlyphRun Run, Colour Colour);

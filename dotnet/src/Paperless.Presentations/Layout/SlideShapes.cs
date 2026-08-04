using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector;

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

    /// <summary>
    /// The marker at the start of its outline, when the outline is a line and carries one.
    /// </summary>
    /// <remarks>
    /// Carried rather than resolved into geometry here because resolving it produces *more
    /// shapes* — an arrowhead is a filled polygon beside the shaft, not a property of the stroke
    /// — and a reader that wants the model rather than the drawing should be able to see which
    /// end of which line the file put a marker on.
    /// </remarks>
    public SlideLineEnd HeadEnd { get; init; }

    /// <summary>The marker at the end of its outline.</summary>
    public SlideLineEnd TailEnd { get; init; }

    /// <summary>The picture it draws, or null when it is not a picture frame.</summary>
    /// <remarks>
    /// Separate from <see cref="Fill"/>, and the difference is not cosmetic. A
    /// <see cref="BitmapPaint"/> covers whatever path it is given and repeats to do it; a
    /// picture is placed once, in a rectangle of its own that a crop may push outside the
    /// shape. Both can appear on one shape — a <c>p:pic</c> may carry a solid fill behind a
    /// transparent PNG — so they are drawn in that order rather than made to exclude each
    /// other.
    /// </remarks>
    public PlacedPicture? Picture { get; init; }

    /// <summary>The text inside it, or null when it holds none.</summary>
    public PlacedText? Text { get; init; }

    /// <summary>The drop shadow it casts, or null when it casts none.</summary>
    public SlideShadow? Shadow { get; init; }
}

/// <summary>
/// A shape's drop shadow: where the copy goes, what colour it is, and how far through it you
/// can see.
/// </summary>
/// <remarks>
/// <para>
/// A shadow is not a property of a shape's outline — it is the <em>whole shape drawn again</em>,
/// offset, with every colour replaced by one. That is literally what LibreOffice does:
/// <c>createEmbeddedShadowPrimitive</c> (<c>svx/source/sdr/primitive2d/sdrdecompositiontools.cxx:860</c>)
/// wraps the shape's entire decomposition — fill, outline, picture and text alike — in a
/// <c>ShadowPrimitive2D</c>, which embeds it in a <c>BColorModifier_replace</c> and a translation
/// (<c>drawinglayer/source/primitive2d/shadowprimitive2d.cxx:76-89</c>), and puts the result
/// <em>behind</em> the shape rather than beside it.
/// </para>
/// <para>
/// The transparency is carried apart from the colour rather than as its alpha, because that is
/// what makes overlapping parts of one shadow composite once. LibreOffice wraps the whole shadow
/// in a single <c>UnifiedTransparencePrimitive2D</c>, so a shape whose outline overlaps its own
/// fill casts one 38%-black shadow and not a darker seam where the two meet.
/// </para>
/// </remarks>
/// <param name="OffsetX">How far right the copy sits; negative is left.</param>
/// <param name="OffsetY">How far down the copy sits; negative is up.</param>
/// <param name="Colour">What every mark in the copy is drawn in.</param>
/// <param name="Opacity">How opaque the whole copy is, from 0 to 1.</param>
/// <param name="Blur">
/// The blur radius the file states, or zero.
/// </param>
/// <remarks>
/// <para>
/// <see cref="Blur"/> is carried and only partly acted on, and the reason is worth stating
/// because it decides what reaches the page. LibreOffice rasterises a blurred shadow: with a
/// non-zero radius <c>ShadowPrimitive2D</c> renders its children to a bitmap and softens that
/// (<c>shadowprimitive2d.cxx:91-140</c>), so the reference PDF holds a greyscale image with a
/// soft mask and <strong>no text</strong> — verified on <c>passiv.pptx</c>, whose every page
/// carries a 918 × 272 gray JPEG plus smask and whose words are not duplicated. With a zero
/// radius it stays vector and the shadow's text is real, extractable text — which is how
/// <c>pres_ioc_phuket.ppt</c> comes to draw "National" fourteen times, seven pairs 6.01 pt apart
/// in both axes.
/// </para>
/// <para>
/// So a blurred shadow is drawn here as a hard-edged one <em>without its text</em>, and an
/// unblurred one with it. Suppressing the text under blur is not a nicety: drawing it would add
/// words the reference does not have to every deck with a themed shadow, which is the largest
/// avoidable regression this feature could cause.
/// </para>
/// </remarks>
public readonly record struct SlideShadow(
    Length OffsetX,
    Length OffsetY,
    Colour Colour,
    double Opacity = 1.0,
    Length Blur = default)
{
    /// <summary>True when the shadow would put no mark on the page.</summary>
    public bool IsInvisible => Opacity <= 0 || Colour.A == 0;

    /// <summary>True when the shadow's copy carries the shape's text, as an unblurred one does.</summary>
    public bool CarriesText => Blur <= Length.Zero;
}

/// <summary>
/// A picture placed on a slide: the bytes, where the whole of it goes, and how opaque it is.
/// </summary>
/// <param name="Image">
/// The picture, normally still encoded — a reader hands the file's own bytes on and whichever
/// backend wants pixels decodes them, so that extraction never pays for a codec. Null when the
/// picture is a vector and there is no raster fallback beside it.
/// </param>
/// <param name="Destination">
/// Where the <em>undisturbed</em> picture goes. A cropped picture's rectangle is larger than
/// the shape and is clipped to it, which is what a crop becomes once it reaches a drawing model
/// that has clipping and no crop.
/// </param>
/// <param name="Opacity">A uniform opacity multiplier from 0 to 1.</param>
public sealed record PlacedPicture(RasterImage? Image, DocRect Destination, double Opacity = 1.0)
{
    /// <summary>
    /// The picture as a display list — an SVG, a WMF, an EMF or an EMF+ — or null when it is a
    /// raster.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Decoded on first use rather than while the slide is read, which is what keeps a deck whose
    /// master carries a metafile logo from paying the font stack's start-up cost on a caller that
    /// only wanted the words. Measured: the first decode in a process is 1044 ms for a WMF with
    /// text and 0.2 ms once warm.
    /// </para>
    /// <para>
    /// <see cref="Image"/> may be set beside it, and then means the raster fallback of a
    /// DrawingML <c>svgBlip</c> — what PowerPoint shows to a consumer that cannot read SVG.
    /// </para>
    /// </remarks>
    public Lazy<VectorImage>? Vector { get; init; }

    /// <summary>
    /// True when the picture's bytes are carried inside the document's own markup or records
    /// rather than as a package entry of their own.
    /// </summary>
    /// <remarks>
    /// A `.ppt`'s Escher blip and a flat ODF's <c>office:binary-data</c> are inline; a
    /// <c>ppt/media/…</c> relationship target and an ODP's <c>Pictures/…</c> entry are not. It is
    /// a fact about storage and would be an odd thing to draw differently — see
    /// <c>SlideDrawing.FillReachesThePage</c>, which is the one place that reads it, for the
    /// measurement that made it necessary and for the competing explanation it could not rule out.
    /// </remarks>
    public bool IsInline { get; init; }
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
/// <param name="Rules">
/// The underline and strikethrough rectangles the run's own face asks for, in the same
/// coordinates as the run, or empty when it carries neither.
/// </param>
/// <remarks>
/// The rules travel with the run rather than being drawn by a backend from the font, because a
/// decoration's offset and thickness come from the face's <c>post</c> and <c>OS/2</c> tables and
/// the face is resolved here — a backend holds a <see cref="FontReference"/> and cannot read
/// them. They are filled rectangles rather than strokes so their thickness is exact, which is
/// what the spreadsheet layer does with the same metrics.
/// </remarks>
public readonly record struct PlacedGlyphRun(
    GlyphRun Run, Colour Colour, IReadOnlyList<DocRect>? Rules = null);

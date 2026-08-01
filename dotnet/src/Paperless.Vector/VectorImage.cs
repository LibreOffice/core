using Paperless.Core.Diagnostics;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Vector;

/// <summary>
/// A decoded vector image: a replayable display list, the coordinate space it is drawn in,
/// and the physical size it asks for.
/// </summary>
/// <remarks>
/// <para>
/// <b>Three pieces of geometry, and keeping them apart is the whole job.</b>
/// <see cref="Content"/> is drawn in the coordinates <see cref="ViewBox"/> describes;
/// <see cref="IntrinsicSize"/> is how big the image says it wants to be; and the destination
/// rectangle passed to <see cref="Draw(IDrawingSink, DocRect)"/> is how big the
/// <em>document</em> says the frame is.
/// Conflating any two of them is the classic wrongly-scaled-or-mirrored metafile bug, and the
/// metafile formats will arrive here with exactly the same three quantities: an EMF's frame
/// rectangle is a view box, its reference-device fields give an intrinsic size, and the
/// <c>a:ext</c> on the shape holding it gives the destination.
/// </para>
/// <para>
/// <b>The document's extent wins.</b> <see cref="Draw(IDrawingSink, DocRect)"/> stretches <see cref="ViewBox"/> onto
/// the destination without preserving aspect ratio, and clips to it. That is what LibreOffice
/// does: a <c>Graphic</c> is mapped onto the <c>SdrObject</c>'s logic rectangle whatever its
/// own preferred size was. <see cref="IntrinsicSize"/> is for the caller that has no extent to
/// impose — a bare <c>.svg</c> opened on its own, or a frame whose size the format leaves to
/// the picture.
/// </para>
/// <para>
/// Note that <em>within</em> the image, aspect ratio has already been honoured: an SVG's
/// <c>preserveAspectRatio</c> maps its <c>viewBox</c> into its own viewport during decoding,
/// so the letterboxing SVG asks for is baked into <see cref="Content"/> and the outer stretch
/// is a separate, later question. Doing it in one step instead would apply the wrong rule to
/// one of the two mappings.
/// </para>
/// </remarks>
public sealed record VectorImage
{
    /// <summary>An image with nothing in it, at no size.</summary>
    public static VectorImage Empty { get; } = new()
    {
        Content = new DisplayList(),
        ViewBox = DocRect.Empty,
        IntrinsicSize = DocSize.Empty,
    };

    /// <summary>The recorded drawing commands.</summary>
    public required DisplayList Content { get; init; }

    /// <summary>
    /// The rectangle of <see cref="Content"/>'s coordinate space that fills a destination.
    /// </summary>
    /// <remarks>
    /// In EMUs, like every other length in Paperless, even though the source format states it
    /// in its own units — SVG user units, an EMF's logical units. Converting once here, at the
    /// boundary, is what keeps the rest of the pipeline from having to know which format the
    /// picture came from.
    /// </remarks>
    public required DocRect ViewBox { get; init; }

    /// <summary>
    /// The physical size the image asks to be drawn at when nothing imposes one.
    /// </summary>
    /// <remarks>
    /// For SVG this is <c>width</c> and <c>height</c> resolved against the CSS 96-per-inch
    /// pixel, falling back to the <c>viewBox</c>. It usually equals <see cref="ViewBox"/>'s
    /// size and is kept separate because for the metafile formats it will not: an EMF states
    /// its physical extent in the header independently of its logical coordinate space.
    /// </remarks>
    public required DocSize IntrinsicSize { get; init; }

    /// <summary>What went wrong, or was skipped, while decoding.</summary>
    public IReadOnlyList<Diagnostic> Diagnostics { get; init; } = [];

    /// <summary>
    /// True when a limit in <see cref="VectorLimits"/> stopped the decode early, so what is
    /// here is a prefix of the picture rather than the whole of it.
    /// </summary>
    public bool IsTruncated { get; init; }

    /// <summary>True when there is nothing to draw.</summary>
    public bool IsEmpty => Content.Count == 0 || ViewBox.IsEmpty;

    /// <summary>
    /// Draws the image into a destination rectangle, mapping <see cref="ViewBox"/> onto it.
    /// </summary>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="destination">Where the image goes, in document coordinates.</param>
    public void Draw(IDrawingSink sink, DocRect destination)
    {
        ArgumentNullException.ThrowIfNull(sink);

        if (IsEmpty || destination.IsEmpty) return;

        double sx = (double)destination.Width.Emu / ViewBox.Width.Emu;
        double sy = (double)destination.Height.Emu / ViewBox.Height.Emu;

        sink.Save();

        // Clipped to the frame because the source formats say so and because the alternative
        // is a picture that paints over its neighbours: SVG clips to its outermost viewport
        // (svgio/source/svgreader/svgsvgnode.cxx:632) and an EMF is bounded by its frame.
        sink.ClipPath(GraphicsPath.Rectangle(destination));

        sink.Transform(new AffineTransform(
            sx,
            0,
            0,
            sy,
            destination.X.Emu - (ViewBox.X.Emu * sx),
            destination.Y.Emu - (ViewBox.Y.Emu * sy)));

        Content.Replay(sink);

        sink.Restore();
    }

    /// <summary>
    /// Draws the image at its intrinsic size, with its top-left corner at a point.
    /// </summary>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="origin">Where the image's top-left corner goes.</param>
    public void Draw(IDrawingSink sink, DocPoint origin)
        => Draw(sink, new DocRect(origin, IntrinsicSize));

    /// <summary>
    /// The size an image takes when a container states one dimension and leaves the other to
    /// the picture.
    /// </summary>
    /// <remarks>
    /// Both DrawingML and ODF can do this, and the answer has to come from the image's own
    /// aspect ratio rather than from a default, or a logo comes out square.
    /// </remarks>
    /// <param name="width">The width to fit, or null to take it from the image.</param>
    /// <param name="height">The height to fit, or null to take it from the image.</param>
    public DocSize SizeFor(Length? width, Length? height)
    {
        if (width is { } w && height is { } h) return new DocSize(w, h);
        if (IntrinsicSize.IsEmpty) return new DocSize(width ?? Length.Zero, height ?? Length.Zero);

        double ratio = (double)IntrinsicSize.Width.Emu / IntrinsicSize.Height.Emu;

        if (width is { } onlyWidth) return new DocSize(onlyWidth, onlyWidth / ratio);
        if (height is { } onlyHeight) return new DocSize(onlyHeight * ratio, onlyHeight);

        return IntrinsicSize;
    }
}

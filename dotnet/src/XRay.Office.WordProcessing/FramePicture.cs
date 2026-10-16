using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Vector;

namespace XRay.Office.WordProcessing;

/// <summary>
/// What a frame draws: encoded raster bytes, a vector picture, both, or neither.
/// </summary>
/// <remarks>
/// <para>
/// One value rather than two properties threaded side by side, because all four front ends carry a
/// picture through two or three layers before it reaches a <see cref="Layout.PageFrame"/> and a pair of
/// nullable fields would let one layer set one of them and drop the other. The pair travels together or
/// not at all.
/// </para>
/// <para>
/// <strong>Both at once is the <c>svgBlip</c> case and nothing else.</strong> A DrawingML
/// <c>a:blip</c> since Office 2016 may name an SVG in an extension <em>and</em> a raster in
/// <c>r:embed</c>, the raster being what a consumer that cannot read SVG shows. Keeping both lets the
/// drawing code fall back when a decode comes back empty, which is what
/// <c>BlipReference.Choice.FallbackRelationshipId</c> exists for; every other format states one or the
/// other.
/// </para>
/// <para>
/// <strong>Why the vector is a <see cref="Lazy{T}"/> and the raster is not.</strong> A
/// <see cref="RasterImage"/> is the file's own bytes and costs nothing to carry; decoding a metafile
/// costs the font stack. Measured on this tree: the first <c>VectorImages.Decode</c> in a process takes
/// <strong>1044 ms</strong> for a WMF with one text run, 381 ms for an EMF+ and 67 ms for a text-free
/// EMF, against 0.08–0.21 ms once warm — nearly all of it resolving and loading faces through
/// <c>XRay.Office.Text</c>. RTF and DOC read their pictures while parsing the document, which is the
/// <em>extraction</em> path, so decoding there would put a second of font work on a caller that only
/// wanted the words. Deferring it to the first draw keeps extraction free and still decodes once, since
/// the readers cache a picture per part and a <see cref="Lazy{T}"/> caches its value.
/// </para>
/// </remarks>
/// <param name="Raster">The picture as the file stored it, when it is one a raster backend can draw.</param>
/// <param name="Vector">
/// The picture as a display list, decoded on first use, when the bytes are a format
/// <c>XRay.Office.Vector</c> reads.
/// </param>
public readonly record struct FramePicture(RasterImage? Raster, Lazy<VectorImage>? Vector)
{
    /// <summary>
    /// How much of the picture each edge throws away, or <see cref="PictureCropFractions.None"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Travels with the bytes for the reason the pair itself does: a crop that arrived separately
    /// could be dropped by a layer that carried the picture. It cannot be applied here, because a
    /// crop is a rectangle grown around the frame and a frame has no rectangle until
    /// <c>FrameLayout</c> has placed it — <c>PageDrawing</c> does the arithmetic and takes the
    /// clip that goes with it.
    /// </para>
    /// <para>
    /// Set from Escher properties 256–259 on the <c>.doc</c> path. DrawingML's <c>a:srcRect</c> and
    /// ODF's <c>fo:clip</c> say the same thing on the other three front ends and are not read yet,
    /// so this is <see cref="PictureCropFractions.None"/> there.
    /// </para>
    /// </remarks>
    public PictureCropFractions Crop { get; init; }

    /// <summary>A frame with no picture in it.</summary>
    public static FramePicture None => default;

    /// <summary>True when there is nothing to draw.</summary>
    public bool IsEmpty => Raster is null && Vector is null;

    /// <summary>A picture that is only a raster.</summary>
    /// <param name="raster">The bytes, or null for nothing.</param>
    public static FramePicture Of(RasterImage? raster) => new(raster, null);

    /// <summary>
    /// A picture that is a vector, decoded when something draws it.
    /// </summary>
    /// <param name="bytes">The encoded picture, which <c>VectorImages</c> sniffs when it is asked to.</param>
    public static FramePicture OfVector(ReadOnlyMemory<byte> bytes)
        => new(null, new Lazy<VectorImage>(() => VectorImages.Decode(bytes)));
}

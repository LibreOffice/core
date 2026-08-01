using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.Vector;

/// <summary>
/// Decodes an embedded vector image into Paperless drawing commands.
/// </summary>
/// <remarks>
/// <para>
/// Not an optional extra. Office documents embed WMF, EMF and EMF+ constantly: pasted
/// clip art, Visio and chart snapshots, equation images, and the fallback rendering
/// that accompanies SmartArt and OLE objects. A renderer that skips them leaves large
/// blank areas on visually important slides and pages.
/// </para>
/// <para>
/// <b>What a decoder owes its caller.</b> A <see cref="VectorImage"/>: a display list, the
/// coordinate rectangle that list is drawn in, and the physical size the image asks for.
/// Not pixels — rasterising here would throw away the resolution independence that is the
/// entire reason the document stored a vector. Not a live replay either: the picture is
/// recorded so it can be drawn many times, counted against
/// <see cref="VectorLimits"/>, and cached.
/// </para>
/// <para>
/// <b>What a decoder owes itself.</b> Every implementation must honour
/// <see cref="VectorLimits"/> and must resolve no external reference — no URL, no file
/// path. Both obligations are on this seam rather than on each format because the formats
/// arrive from the same untrusted place and would otherwise each have to remember. See
/// <see cref="VectorLimits"/> for the fan-out attack that motivates the harder half.
/// </para>
/// <para>
/// EMF+ is the highest-risk area in the whole project: roughly fifty record types with
/// no existing C# implementation to build on. See
/// <c>dotnet/research/06-rendering.md</c> section D. What a metafile implementer has to
/// supply beyond a record parser is listed in <c>src/Paperless.Vector/TODO.md</c>.
/// </para>
/// </remarks>
public interface IVectorImageDecoder
{
    /// <summary>The media types this decoder handles.</summary>
    IReadOnlyCollection<string> SupportedMediaTypes { get; }

    /// <summary>True when the decoder recognises the data's signature.</summary>
    /// <remarks>
    /// By content, never by the media type the document declared: office files mislabel
    /// embedded graphics as routinely as they mislabel themselves, and a WMF stored under a
    /// <c>.emf</c> part name is common enough to be unremarkable.
    /// </remarks>
    bool CanDecode(ReadOnlySpan<byte> data);

    /// <summary>
    /// Decodes the image into a replayable display list.
    /// </summary>
    /// <param name="data">The encoded image.</param>
    /// <param name="limits">
    /// Caps on the work this decode may cause; null uses <see cref="VectorLimits.Default"/>.
    /// </param>
    /// <returns>
    /// The decoded image. Never null and never throws for malformed input: a picture that
    /// cannot be read is a document to draw <em>without</em> that picture, so the failure
    /// comes back as <see cref="VectorImage.Empty"/> plus a diagnostic.
    /// </returns>
    VectorImage Decode(ReadOnlyMemory<byte> data, VectorLimits? limits = null);

    /// <summary>
    /// Decodes the image and draws it into a rectangle, in one step.
    /// </summary>
    /// <remarks>
    /// The convenience form, for a caller drawing a picture once. A caller drawing the same
    /// picture on many pages should <see cref="Decode(ReadOnlyMemory{byte}, VectorLimits)"/> once and keep the
    /// <see cref="VectorImage"/>.
    /// </remarks>
    /// <param name="data">The encoded image.</param>
    /// <param name="destination">
    /// The rectangle to fit the image into. The decoder maps the image's own
    /// coordinate space onto this.
    /// </param>
    /// <param name="sink">Receives the drawing commands.</param>
    void Decode(ReadOnlyMemory<byte> data, DocRect destination, IDrawingSink sink)
        => Decode(data).Draw(sink, destination);
}

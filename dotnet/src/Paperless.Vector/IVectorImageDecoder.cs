using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.Vector;

/// <summary>
/// Decodes an embedded vector image into Paperless drawing commands.
/// </summary>
/// <remarks>
/// Not an optional extra. Office documents embed WMF, EMF and EMF+ constantly: pasted
/// clip art, Visio and chart snapshots, equation images, and the fallback rendering
/// that accompanies SmartArt and OLE objects. A renderer that skips them leaves large
/// blank areas on visually important slides and pages.
/// <para>
/// EMF+ is the highest-risk area in the whole project: roughly fifty record types with
/// no existing C# implementation to build on. See
/// <c>dotnet/research/06-rendering.md</c> section D.
/// </para>
/// </remarks>
public interface IVectorImageDecoder
{
    /// <summary>The media types this decoder handles.</summary>
    IReadOnlyCollection<string> SupportedMediaTypes { get; }

    /// <summary>True when the decoder recognises the data's signature.</summary>
    bool CanDecode(ReadOnlySpan<byte> data);

    /// <summary>
    /// Decodes the image, replaying it into a drawing sink.
    /// </summary>
    /// <param name="data">The encoded image.</param>
    /// <param name="destination">
    /// The rectangle to fit the image into. The decoder maps the image's own
    /// coordinate space onto this.
    /// </param>
    /// <param name="sink">Receives the drawing commands.</param>
    void Decode(ReadOnlyMemory<byte> data, DocRect destination, IDrawingSink sink);
}

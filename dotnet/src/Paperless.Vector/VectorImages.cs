using Paperless.Vector.Svg;

namespace Paperless.Vector;

/// <summary>
/// Finds the decoder for a picture's bytes.
/// </summary>
/// <remarks>
/// <para>
/// The one call a reader needs. It sniffs rather than trusting the media type the document
/// declared, for the same reason <c>FormatCatalogue</c> sniffs whole documents: office files
/// mislabel their parts constantly, and a WMF stored under a <c>.emf</c> part name is
/// unremarkable. The declared type is used only to break a tie.
/// </para>
/// <para>
/// SVG is the only entry today. WMF, EMF and EMF+ register here as they arrive, and nothing
/// that calls this has to change when they do — which is the point of the seam.
/// </para>
/// </remarks>
public static class VectorImages
{
    private static readonly IVectorImageDecoder[] Registered = [new SvgImageDecoder()];

    /// <summary>Every decoder Paperless has.</summary>
    public static IReadOnlyList<IVectorImageDecoder> Decoders => Registered;

    /// <summary>The decoder that recognises these bytes, or null.</summary>
    /// <param name="data">The encoded picture.</param>
    public static IVectorImageDecoder? For(ReadOnlySpan<byte> data)
    {
        foreach (IVectorImageDecoder decoder in Registered)
        {
            if (decoder.CanDecode(data)) return decoder;
        }

        return null;
    }

    /// <summary>
    /// True when a media type names a vector picture Paperless can decode.
    /// </summary>
    /// <remarks>
    /// For the reader that has a part's declared type but not yet its bytes, and wants to
    /// know whether fetching them is worth it. A false answer is not a promise: the bytes
    /// still decide.
    /// </remarks>
    /// <param name="mediaType">The declared media type, or null.</param>
    public static bool IsVectorMediaType(string? mediaType)
    {
        if (mediaType is not { Length: > 0 }) return false;

        // Parameters after a semicolon — "image/svg+xml; charset=utf-8" — are legal and do
        // turn up in [Content_Types].xml.
        int semicolon = mediaType.IndexOf(';', StringComparison.Ordinal);
        string bare = (semicolon < 0 ? mediaType : mediaType[..semicolon]).Trim();

        foreach (IVectorImageDecoder decoder in Registered)
        {
            foreach (string supported in decoder.SupportedMediaTypes)
            {
                if (string.Equals(bare, supported, StringComparison.OrdinalIgnoreCase)) return true;
            }
        }

        return false;
    }

    /// <summary>
    /// Decodes a picture with whichever decoder recognises it.
    /// </summary>
    /// <param name="data">The encoded picture.</param>
    /// <param name="limits">Caps on the work; null uses <see cref="VectorLimits.Default"/>.</param>
    /// <returns><see cref="VectorImage.Empty"/> when nothing recognises the bytes.</returns>
    public static VectorImage Decode(ReadOnlyMemory<byte> data, VectorLimits? limits = null)
        => For(data.Span)?.Decode(data, limits) ?? VectorImage.Empty;
}

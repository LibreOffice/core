using Paperless.Core.Diagnostics;
using Paperless.Core.Geometry;
using Paperless.Vector.Metafiles;

namespace Paperless.Vector.Wmf;

/// <summary>
/// Decodes Windows Metafiles into the Paperless display list.
/// </summary>
/// <remarks>
/// <para>
/// The oldest and smallest of the three metafile formats, and the one that exercises the shared
/// device context in <c>Paperless.Vector.Metafiles</c> on the simplest of them. A WMF is a
/// stateful command stream of 16-bit records: the mapping, the selected pen and brush, the clip
/// and the current position are all device state that earlier records set and later records
/// depend on. None of that is the drawing seam's — see
/// <see cref="MetafileDeviceContext"/> — and none of it is specific to WMF.
/// </para>
/// <para>
/// WMFs turn up embedded far more often than they turn up on their own: an OLE object's
/// presentation, a pasted chart or piece of clip art, and the fallback rendering DOC and PPT
/// store alongside a native shape. So the realistic input is a picture inside a document, and
/// the picture is as attacker-supplied as the document is.
/// </para>
/// </remarks>
public sealed class WmfImageDecoder : IVectorImageDecoder
{
    private readonly MetafileTextEngine _text = new();

    /// <summary>The media types a WMF arrives under.</summary>
    /// <remarks>
    /// <c>image/x-wmf</c> is what OOXML's content types use and <c>image/wmf</c> what ODF
    /// writes; both spellings appear in real packages, and the bytes are sniffed regardless.
    /// </remarks>
    public IReadOnlyCollection<string> SupportedMediaTypes { get; } =
        ["image/x-wmf", "image/wmf", "application/x-msmetafile", "image/x-msmetafile"];

    /// <inheritdoc/>
    public bool CanDecode(ReadOnlySpan<byte> data) => WmfReader.Looks(data);

    /// <inheritdoc/>
    public VectorImage Decode(ReadOnlyMemory<byte> data, VectorLimits? limits = null)
    {
        VectorLimits caps = limits ?? VectorLimits.Default;
        List<Diagnostic> diagnostics = [];

        if (data.Length > caps.MaxBytes)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning,
                "PL6002",
                $"A WMF of {data.Length} bytes exceeded the {caps.MaxBytes}-byte limit."));
            return VectorImage.Empty with { Diagnostics = diagnostics };
        }

        WmfReader reader = new(data.ToArray(), caps, diagnostics, _text);

        try
        {
            if (!reader.Read()) return VectorImage.Empty with { Diagnostics = diagnostics };
        }
        catch (Exception exception) when (exception is not OutOfMemoryException and not StackOverflowException)
        {
            // The reader is written not to throw — every read is bounds-checked and every
            // malformed record is skipped — but the contract on this seam is that a picture which
            // cannot be read is a document to draw *without* that picture, and that has to hold
            // even for the bug we have not found yet.
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL6035", $"A WMF could not be read: {exception.Message}"));
            return VectorImage.Empty with { Diagnostics = diagnostics };
        }

        if (reader.Extent.IsEmpty) return VectorImage.Empty with { Diagnostics = diagnostics };

        return new VectorImage
        {
            Content = reader.Content,
            ViewBox = new DocRect(DocPoint.Origin, reader.Extent),
            IntrinsicSize = reader.Extent,
            Diagnostics = diagnostics,
            IsTruncated = reader.IsTruncated,
        };
    }
}

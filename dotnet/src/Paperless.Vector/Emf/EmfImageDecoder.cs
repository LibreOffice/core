using Paperless.Core.Diagnostics;
using Paperless.Vector.Metafiles;

namespace Paperless.Vector.Emf;

/// <summary>
/// Decodes enhanced metafiles into the Paperless display list.
/// </summary>
/// <remarks>
/// <para>
/// The format Windows has actually recorded in since 1993, and the one an office document is
/// most likely to be carrying: a pasted chart, a Visio drawing, an equation, and the fallback
/// rendering that accompanies SmartArt and every OLE object. It is also the format a WMF turns
/// out to be hiding — a 16 700-byte WMF that LibreOffice writes carries 12 964 bytes of a
/// complete EMF inside two escape records.
/// </para>
/// <para>
/// Everything stateful is shared with the WMF reader through
/// <c>Paperless.Vector.Metafiles</c>. What arrives here is the header question, and it is the
/// one the seam's <c>ViewBox</c>/<c>IntrinsicSize</c> split exists for: see
/// <see cref="EmfReader.ViewBox"/>.
/// </para>
/// </remarks>
public sealed class EmfImageDecoder : IVectorImageDecoder
{
    private readonly MetafileTextEngine _text = new();

    /// <summary>The media types an EMF arrives under.</summary>
    /// <remarks>
    /// <c>image/x-emf</c> is what OOXML's content types use and <c>image/emf</c> what ODF
    /// writes; both spellings appear in real packages, and the bytes are sniffed regardless.
    /// </remarks>
    public IReadOnlyCollection<string> SupportedMediaTypes { get; } =
        ["image/x-emf", "image/emf", "application/x-msmetafile", "image/x-msmetafile"];

    /// <inheritdoc/>
    public bool CanDecode(ReadOnlySpan<byte> data) => EmfReader.Looks(data);

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
                $"An EMF of {data.Length} bytes exceeded the {caps.MaxBytes}-byte limit."));
            return VectorImage.Empty with { Diagnostics = diagnostics };
        }

        EmfReader reader = new(data.ToArray(), caps, diagnostics, _text);

        try
        {
            if (!reader.Read())
            {
                return VectorImage.Empty with { Diagnostics = diagnostics, IsTruncated = reader.IsTruncated };
            }
        }
        catch (Exception exception) when (exception is not OutOfMemoryException and not StackOverflowException)
        {
            // The reader is written not to throw — every read is bounds-checked and every
            // malformed record is skipped — but the contract on this seam is that a picture which
            // cannot be read is a document to draw *without* that picture, and that has to hold
            // even for the bug we have not found yet.
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL6035", $"An EMF could not be read: {exception.Message}"));
            return VectorImage.Empty with { Diagnostics = diagnostics };
        }

        if (reader.ViewBox.IsEmpty)
        {
            return VectorImage.Empty with { Diagnostics = diagnostics, IsTruncated = reader.IsTruncated };
        }

        return new VectorImage
        {
            Content = reader.Content,
            ViewBox = reader.ViewBox,
            IntrinsicSize = reader.Extent,
            Diagnostics = diagnostics,
            IsTruncated = reader.IsTruncated,
        };
    }
}

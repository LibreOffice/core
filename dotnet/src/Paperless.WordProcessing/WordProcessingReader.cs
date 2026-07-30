using Paperless.Containers;
using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Formats;
using Paperless.WordProcessing.OpenDocument;
using Paperless.WordProcessing.Ooxml;
using Paperless.WordProcessing.Rtf;
using Paperless.WordProcessing.Ww8;

namespace Paperless.WordProcessing;

/// <summary>
/// Reads word-processing documents: DOCX and its variants, legacy DOC, RTF, and ODT.
/// The Paperless counterpart of LibreOffice Writer.
/// </summary>
/// <remarks>
/// <para>
/// All four front ends converge on one document model and one layout engine. That is a
/// deliberate copy of how LibreOffice is arranged — its DOCX, DOC and RTF importers all
/// build the same <c>SwDoc</c> through the same API and differ only in parsing — and it
/// is what keeps the expensive half of the work, layout, written once.
/// </para>
/// <para>
/// See <c>dotnet/research/02-writer.md</c> for the model and layout architecture this
/// is derived from.
/// </para>
/// </remarks>
public sealed class WordProcessingReader : IDocumentReader
{
    /// <inheritdoc/>
    public IReadOnlyCollection<DocumentFormat> SupportedFormats { get; } =
    [
        DocumentFormat.Docx, DocumentFormat.Docm, DocumentFormat.Dotx, DocumentFormat.Dotm,
        DocumentFormat.Doc, DocumentFormat.Dot,
        DocumentFormat.Rtf, DocumentFormat.WordXml2003,
        DocumentFormat.Odt, DocumentFormat.Ott, DocumentFormat.Fodt,
        DocumentFormat.Sxw, DocumentFormat.Stw,
    ];

    /// <inheritdoc/>
    public IDocument Read(DocumentSource source)
    {
        ArgumentNullException.ThrowIfNull(source);
        DocumentFormat format = SourceIdentification.Resolve(source);

        return format switch
        {
            DocumentFormat.Odt or DocumentFormat.Ott or DocumentFormat.Fodt
                => new OdtReader().Read(source, format),

            DocumentFormat.Docx or DocumentFormat.Docm or DocumentFormat.Dotx or DocumentFormat.Dotm
                => DocxReader.Read(source, format),

            DocumentFormat.Rtf => RtfReader.Read(source, format),

            DocumentFormat.Doc or DocumentFormat.Dot => DocReader.Read(source, format),

            // Named in SupportedFormats but not implemented yet. Distinguishing "we will
            // support this" from "this is not a word-processing document" matters to callers
            // deciding whether to retry with another reader.
            DocumentFormat.WordXml2003 or DocumentFormat.Sxw or DocumentFormat.Stw
                => throw new UnsupportedFormatException(
                    format, $"Reading {format} is not implemented yet."),

            _ => throw new UnsupportedFormatException(
                format, $"{format} is not a word-processing format."),
        };
    }
}

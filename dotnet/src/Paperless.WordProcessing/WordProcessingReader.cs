using Paperless.Core.Documents;
using Paperless.Core.Formats;

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
    public IDocument Read(DocumentSource source) => throw new NotImplementedException();
}

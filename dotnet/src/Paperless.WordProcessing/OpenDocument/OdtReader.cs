using System.Xml.Linq;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Core.Documents;
using Paperless.OpenDocument;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// Reads ODF text documents: <c>odt</c>, the <c>ott</c> template, and the flat <c>fodt</c>.
/// </summary>
/// <remarks>
/// The body of a text document is one flow, so it becomes a single
/// <see cref="SectionKind.Body"/> section. Headers, footers, footnotes and comments are not
/// part of that flow and become their own sections after it — LibreOffice's own text export
/// drops all four, so extracting them is a deliberate improvement rather than a difference to
/// reconcile.
/// </remarks>
public sealed class OdtReader : OdfReader
{
    /// <summary>
    /// Reads a text document, giving it the page geometry a Writer document has.
    /// </summary>
    /// <remarks>
    /// A separate entry point from the inherited <see cref="OdfReader.Read"/> rather than an override,
    /// because the base returns the family-neutral document all three readers share and this adds
    /// something only a text document has.
    /// </remarks>
    public OdtWordDocument ReadText(DocumentSource source, DocumentFormat format)
    {
        OdfDocument inner = Read(source, format);
        return new OdtWordDocument(inner, _marks?.Marks ?? Model.WritingMarks.Empty);
    }

    /// <inheritdoc/>
    /// <remarks>
    /// Word processing is the only family with anywhere to put a bookmark's range, so this is the
    /// only reader that asks for the marks at all.
    /// </remarks>
    protected override IOdfMarkSink? CreateMarkSink() => _marks = new OdtMarkSink();

    private OdtMarkSink? _marks;

    /// <inheritdoc/>
    protected override DocumentFamily Family => DocumentFamily.WordProcessing;

    /// <inheritdoc/>
    protected override bool IncludesMasterPageContent => true;

    /// <inheritdoc/>
    protected override void ReadBody(XElement body, OdfContentReader reader, ContentDocument content)
    {
        ArgumentNullException.ThrowIfNull(body);
        ArgumentNullException.ThrowIfNull(reader);
        ArgumentNullException.ThrowIfNull(content);

        ContentSection section = new() { Kind = SectionKind.Body, Index = 0 };
        reader.ReadBlocks(body, section);
        content.Children.Add(section);

        // Notes and comments are appended immediately after the body they belong to rather
        // than left to the very end, so their position stays meaningful.
        foreach (ContentNode node in reader.TakeHoisted()) content.Children.Add(node);
    }
}

using System.Xml.Linq;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
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

using System.Xml.Linq;
using Paperless.Containers.Ooxml;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Ooxml;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// Reads OOXML word-processing documents: <c>docx</c>, <c>docm</c>, and the <c>dotx</c> and
/// <c>dotm</c> templates.
/// </summary>
/// <remarks>
/// <para>
/// The macro-enabled variants are read as data like any other. A macro's presence is reported
/// through <c>FormatInfo.CanCarryMacros</c>; nothing is ever executed.
/// </para>
/// <para>
/// The body is one flow and becomes one <see cref="SectionKind.Body"/> section. Headers,
/// footers, footnotes, endnotes, comments and text-box flows are separate flows and become
/// their own sections — LibreOffice's own text export drops all of them, so extracting them is a
/// deliberate improvement rather than a difference to reconcile.
/// </para>
/// </remarks>
public static class DocxReader
{
    private const string HeaderRelationship =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/header";
    private const string FooterRelationship =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer";

    /// <summary>Reads a document, leaving the source's stream for the caller to dispose.</summary>
    /// <param name="source">The document to read.</param>
    /// <param name="format">The identified format, recorded on the result.</param>
    public static OoxmlWordDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        DocxFile file = DocxFile.Open(source.Stream, leaveOpen: true);
        try
        {
            List<Diagnostic> diagnostics = [.. file.Diagnostics];
            List<WritingSection> sections = [];
            ContentDocument content = new()
            {
                Metadata = OoxmlMetadata.Read(
                    (OpcPackage)file.Package, DocumentFamily.WordProcessing),
            };

            XElement? body = file.Body;
            if (body is null)
            {
                diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Error, "PL2130",
                    "The document has no w:body, so it has no content to extract."));
            }
            else
            {
                DocxContentReader reader = new(file, diagnostics);

                ContentSection section = new() { Kind = SectionKind.Body, Index = 0 };
                reader.ReadBlocks(body, section);
                content.Children.Add(section);

                // Notes and comments land immediately after the body they belong to rather than
                // at the very end, so their position stays meaningful.
                foreach (ContentNode node in reader.TakeHoisted()) content.Children.Add(node);

                ReadHeadersAndFooters(file, body, reader, content);
                sections = ReadSections(file, body);
            }

            return new OoxmlWordDocument(format, file, content, diagnostics, sections);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }

    /// <summary>
    /// Reads every section's page geometry, in document order.
    /// </summary>
    /// <remarks>
    /// A DOCX states a section's properties at its <em>end</em>: each <c>w:sectPr</c> inside a
    /// paragraph's properties closes the section that paragraph finishes, and the one directly under
    /// <c>w:body</c> closes the last. So the enumeration is already in document order, and a document
    /// with no <c>w:sectPr</c> at all still has one section — of default geometry, not of none.
    /// </remarks>
    private static List<WritingSection> ReadSections(DocxFile file, XElement body)
    {
        List<WritingSection> sections =
        [
            .. DocxContentReader.SectionProperties(body)
                .Select(properties => DocxPageGeometry.Read(properties, file.Settings)),
        ];

        if (sections.Count == 0) sections.Add(DocxPageGeometry.Read(null, file.Settings));
        return sections;
    }

    /// <summary>
    /// Reads the header and footer parts the document's sections name.
    /// </summary>
    /// <remarks>
    /// Unlike ODF, a DOCX names its headers by relationship from each <c>w:sectPr</c>, so only
    /// the parts a section actually references are read — there is no risk of picking up an
    /// unused master. A part referenced by several sections is read once.
    /// </remarks>
    private static void ReadHeadersAndFooters(
        DocxFile file,
        XElement body,
        DocxContentReader reader,
        ContentDocument content)
    {
        HashSet<string> alreadyRead = new(StringComparer.Ordinal);
        int index = 0;

        foreach (XElement sectionProperties in DocxContentReader.SectionProperties(body))
        {
            foreach (XElement reference in sectionProperties.Elements())
            {
                bool isHeader = Word.Is(reference, "headerReference");
                if (!isHeader && !Word.Is(reference, "footerReference")) continue;

                string? relationshipId = Word.RelationshipId(reference);
                if (relationshipId is null) continue;

                if (file.Relationship(relationshipId) is { } relationship
                    && !alreadyRead.Add(relationship.Target))
                    continue;

                XElement? part = file.LoadHeaderOrFooter(relationshipId);
                if (part is null) continue;

                // Each flow numbers its own lists: a page number in a footer does not continue
                // the body's count.
                file.Numbering.ResetCounters();

                ContentSection section = new()
                {
                    Kind = isHeader ? SectionKind.Header : SectionKind.Footer,
                    Index = index,
                    // w:type distinguishes the default, first-page and even-page variants, and
                    // which one this is is the only thing that tells them apart in the output.
                    Name = Word.Attribute(reference, "type") ?? "default",
                };
                reader.ReadBlocks(part, section);

                if (section.Children.Count == 0) continue;

                content.Children.Add(section);
                foreach (ContentNode node in reader.TakeHoisted()) content.Children.Add(node);
                index++;
            }
        }

        // Restored so that a caller reading the numbering afterwards sees the body's state
        // rather than a header's.
        file.Numbering.ResetCounters();
    }
}

/// <summary>An OOXML word-processing document that has been read.</summary>
public sealed class OoxmlWordDocument : IWordProcessingDocument, IPaginatedDocument
{
    private readonly DocxFile _file;

    internal OoxmlWordDocument(
        DocumentFormat format,
        DocxFile file,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics,
        IReadOnlyList<WritingSection> sections)
    {
        Format = format;
        _file = file;
        Content = content;
        Diagnostics = diagnostics;
        Sections = sections.Count > 0 ? sections : [new WritingSection()];
    }

    /// <inheritdoc/>
    public DocumentFormat Format { get; }

    /// <inheritdoc/>
    public DocumentFamily Family => DocumentFamily.WordProcessing;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => Content.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics { get; }

    /// <inheritdoc/>
    public IReadOnlyList<WritingSection> Sections { get; }

    /// <summary>
    /// The underlying package: its styles, numbering, settings and remaining parts.
    /// </summary>
    /// <remarks>
    /// Kept reachable because rendering will need the formatting the content tree deliberately
    /// discards, and because a caller wanting an embedded image's bytes has to reach the
    /// package. Valid until this document is disposed.
    /// </remarks>
    public DocxFile File => _file;

    /// <summary>
    /// Lays the document out into pages.
    /// </summary>
    /// <remarks>
    /// One section's geometry, the first, because the paginator takes one — a document that changes page
    /// setup part way through has its page break honoured and the geometry after it not. Carrying a
    /// section change needs each paragraph to know which section it is in, which is the gap recorded in
    /// this library's TODO.
    /// </remarks>
    public IPageSequence Layout(LayoutOptions? options = null)
    {
        XElement? body = _file.Body;
        if (body is null) return new WordProcessingPages([]);

        DocxLayoutSource source = new(_file.Styles, _file.Settings);
        List<PageParagraph> paragraphs = source.Read(body);

        // Read from the document rather than assumed per format. LibreOffice's PARA_SPACE_MAX means the
        // two spacings *add*; when it is off the larger wins, which is Word's behaviour. Its OOXML
        // exporter writes w:doNotUseHTMLParagraphAutoSpacing exactly when the flag is on
        // (docxexport.cxx), so the element's absence is what makes a DOCX collapse — and a document
        // carrying it adds, like an ODF one.
        bool collapses = !Word.IsOn(
            Word.Child(Word.Child(_file.Settings, "compat"), "doNotUseHTMLParagraphAutoSpacing"));

        PaginationOptions pagination = PaginationOptions.Word with
        {
            CollapsesSpacing = collapses,
            MaxPages = options?.MaxPages is > 0
                ? options.MaxPages
                : PaginationOptions.Word.MaxPages,
        };

        return new WordProcessingPages(
            new Paginator(pagination).Paginate(paragraphs, Sections[0]), paragraphs);
    }

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}

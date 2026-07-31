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
        List<PageBlock> blocks = source.Read(body);

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
            new Paginator(pagination).Paginate(blocks, Paginated(source, body)),
            blocks);
    }

    /// <summary>
    /// The document's sections, each paired with the furniture its own <c>w:sectPr</c> names.
    /// </summary>
    /// <remarks>
    /// Per section rather than the first section's throughout, because a document that changes its page
    /// setup halfway usually changes its headers with it — a landscape appendix with its own running head is
    /// the ordinary case. The enumeration order matches <see cref="Sections"/>, which is what
    /// <see cref="PageBlock.SectionIndex"/> indexes.
    /// </remarks>
    private List<PaginatedSection> Paginated(DocxLayoutSource source, XElement body)
    {
        List<XElement> properties = [.. DocxContentReader.SectionProperties(body)];
        List<PaginatedSection> sections = new(Sections.Count);

        for (int i = 0; i < Sections.Count; i++)
        {
            sections.Add(new PaginatedSection(
                Sections[i],
                i < properties.Count ? Furniture(source, properties[i]) : null));
        }

        return sections;
    }

    /// <summary>
    /// One section's headers and footers, laid out from the parts its <c>w:sectPr</c> names.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Read through the same walk the body uses, because a header's paragraphs are paragraphs: they
    /// resolve their styles the same way, measure the same way, and a second walk would be a second place
    /// for the run and tab handling to be got right. A <c>w:hdr</c> root holds block-level children just
    /// as <c>w:body</c> does, so the walk needs no special case.
    /// </para>
    /// <para>
    /// A DOCX names its furniture by relationship and labels each with a <c>w:type</c>, so — unlike ODF
    /// — an unreferenced part is never picked up. The <c>w:type</c> is the only thing distinguishing the
    /// three variants; a reference without one is the default.
    /// </para>
    /// </remarks>
    private PageFurnitureSet? Furniture(DocxLayoutSource source, XElement sectionProperties)
    {
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>> headers = [];
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>> footers = [];

        foreach (XElement reference in sectionProperties.Elements())
        {
            bool isHeader = Word.Is(reference, "headerReference");
            if (!isHeader && !Word.Is(reference, "footerReference")) continue;

            if (SlotOf(Word.Attribute(reference, "type")) is not { } slot) continue;
            if (_file.LoadHeaderOrFooter(Word.RelationshipId(reference)) is not { } part) continue;

            List<PageParagraph> paragraphs = source.ReadFlow(part);
            if (paragraphs.Count == 0) continue;

            (isHeader ? headers : footers)[slot] = paragraphs;
        }

        PageFurnitureSet set = new(headers, footers);
        return set.IsEmpty ? null : set;
    }

    /// <summary>
    /// The slot a <c>w:type</c> names, or null for a value this reader does not know.
    /// </summary>
    /// <remarks>
    /// Null rather than falling back to the default slot, because a producer writing an unknown type
    /// means something by it, and quietly filing it as the default would put that content on every page
    /// of the document.
    /// </remarks>
    private static PageFurnitureSlot? SlotOf(string? type) => type switch
    {
        null or "" or "default" => PageFurnitureSlot.Default,
        "first" => PageFurnitureSlot.First,
        "even" => PageFurnitureSlot.Even,
        _ => null,
    };

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}

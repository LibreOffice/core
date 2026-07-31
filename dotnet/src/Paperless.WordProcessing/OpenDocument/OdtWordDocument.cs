using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using System.Xml.Linq;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// An ODF text document, with the page geometry a Writer document has and a spreadsheet does not.
/// </summary>
/// <remarks>
/// <para>
/// A wrapper rather than a subclass, because <see cref="OdfDocument"/> serves all three families and
/// lives in <c>Paperless.OpenDocument</c> — below this library in the dependency order, so it cannot
/// know what a Writer section is. Wrapping keeps the layering intact and costs one delegation per
/// member.
/// </para>
/// <para>
/// The sections are read here rather than during the content walk because ODF states them nowhere near
/// the content: a paragraph reaches its page setup through its paragraph style's master page, so the
/// answer comes from the style tables and not from the body.
/// </para>
/// </remarks>
public sealed class OdtWordDocument : IWordProcessingDocument, IPaginatedDocument
{
    private readonly OdfDocument _inner;

    internal OdtWordDocument(OdfDocument inner)
    {
        ArgumentNullException.ThrowIfNull(inner);
        _inner = inner;
        Sections = ReadSections(inner.File.Styles);
    }

    /// <inheritdoc/>
    public DocumentFormat Format => _inner.Format;

    /// <inheritdoc/>
    public DocumentFamily Family => _inner.Family;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => _inner.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content => _inner.Content;

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics => _inner.Diagnostics;

    /// <inheritdoc/>
    public IReadOnlyList<WritingSection> Sections { get; }

    /// <summary>The underlying ODF file: its styles, master pages and remaining parts.</summary>
    public OdfFile File => _inner.File;

    /// <summary>
    /// Lays the document out into pages.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A second walk over the content, separate from the one that produced <see cref="Content"/>:
    /// extraction discards the font sizes, indents and spacing layout needs, so re-deriving them from
    /// the tree is not possible and making extraction carry them would charge every caller for a feature
    /// most never use.
    /// </para>
    /// <para>
    /// One section's geometry, the first, because ODF has no section list — a paragraph reaches its page
    /// description through its style's master page, and following that needs the page-break chain, which
    /// is what this produces. So a document whose masters differ mid-way is laid out wholly on its first
    /// master's geometry, and the page break at the change is honoured while the geometry after it is
    /// not.
    /// </para>
    /// </remarks>
    public IPageSequence Layout(LayoutOptions? options = null)
    {
        XElement? body = _inner.File.Body;
        if (body is null) return new WordProcessingPages([]);

        List<OdfMasterPage> masters = OrderedMasters(_inner.File.Styles);
        OdtLayoutSource source = new(
            _inner.File.Styles,
            masterPages: masters
                .Select((master, index) => (master.Name, index))
                .Where(pair => pair.Name is not null)
                .ToDictionary(pair => pair.Name!, pair => pair.index, StringComparer.Ordinal),
            stylesRoot: _inner.File.StylesRoot);

        List<PageBlock> blocks = source.Read(body);

        PaginationOptions pagination = PaginationOptions.Default with
        {
            MaxPages = options?.MaxPages is > 0 ? options.MaxPages : PaginationOptions.Default.MaxPages,
        };

        return new WordProcessingPages(
            new Paginator(pagination).Paginate(
                blocks,
                [.. masters.Select((master, index) => new PaginatedSection(
                    index < Sections.Count ? Sections[index] : Sections[^1],
                    Furniture(source, master)))]),
            blocks);
    }

    /// <summary>
    /// The headers and footers of one master page.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Read through the same walk the body uses, because a header's paragraphs are paragraphs: they resolve
    /// their styles the same way and measure the same way, and a second walk would be a second place for
    /// the run and tab handling to be got right.
    /// </para>
    /// <para>
    /// ODF's slots are its own: <c>style:header</c> is the default, <c>style:header-left</c> the even-page
    /// one — ODF says <em>left</em> where the other formats say even — and <c>style:header-first</c> the
    /// first page's. A missing left header means the pages share one rather than that left pages have none,
    /// which is why its absence leaves the slot empty and lets the default apply.
    /// </para>
    /// </remarks>
    private static PageFurnitureSet? Furniture(OdtLayoutSource source, OdfMasterPage? master)
    {
        if (master is null) return null;

        Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>> headers = [];
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>> footers = [];

        Add(headers, PageFurnitureSlot.Default, master.Header, source);
        Add(headers, PageFurnitureSlot.Even, master.LeftHeader, source);
        Add(headers, PageFurnitureSlot.First, master.FirstHeader, source);
        Add(footers, PageFurnitureSlot.Default, master.Footer, source);
        Add(footers, PageFurnitureSlot.Even, master.LeftFooter, source);
        Add(footers, PageFurnitureSlot.First, master.FirstFooter, source);

        PageFurnitureSet set = new(headers, footers);
        return set.IsEmpty ? null : set;
    }

    private static void Add(
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>> slots,
        PageFurnitureSlot slot,
        XElement? element,
        OdtLayoutSource source)
    {
        if (element is null) return;

        List<PageParagraph> paragraphs = source.ReadFlow(element);
        if (paragraphs.Count > 0) slots[slot] = paragraphs;
    }

    /// <inheritdoc/>
    public void Dispose() => _inner.Dispose();

    /// <summary>
    /// One section per master page the document defines.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Not the same thing as one section per page break, which is what the other three formats give —
    /// ODF has no section list, only masters and the styles that reach them. Deciding which master
    /// applies where needs the page-break chain, and that needs layout. So this reports the geometries
    /// the document defines, with the <c>Standard</c> master first because that is what a paragraph
    /// naming no master gets.
    /// </para>
    /// <para>
    /// A document with no masters at all still gets one section of default geometry, which matches what
    /// LibreOffice does with such a file rather than leaving a caller with nothing to lay out on.
    /// </para>
    /// </remarks>
    private static List<WritingSection> ReadSections(OdfStyles styles)
    {
        List<WritingSection> sections =
            [.. OrderedMasters(styles).Select(master => OdfPageGeometry.Read(styles, master))];

        if (sections.Count == 0) sections.Add(OdfPageGeometry.Read(styles, master: null));
        return sections;
    }

    /// <summary>
    /// The document's master pages in the order the sections are numbered.
    /// </summary>
    /// <remarks>
    /// <c>Standard</c> first, because that is the master a paragraph naming none is laid on and so the one
    /// section zero has to be; the rest by name, so the numbering is stable across reads of the same file.
    /// The <em>order</em> is what matters here rather than the sequence — ODF has no document order for
    /// masters, since a master is reached from a style rather than from a position.
    /// </remarks>
    private static List<OdfMasterPage> OrderedMasters(OdfStyles styles)
        => [.. styles.MasterPages.Values
            .OrderBy(master => master.Name == StandardMasterName ? 0 : 1)
            .ThenBy(master => master.Name, StringComparer.Ordinal)];

    /// <summary>
    /// The master page a paragraph naming none is laid on.
    /// </summary>
    /// <remarks>
    /// Not localised in the file: ODF stores the internal name and keeps the translated one in
    /// <c>style:display-name</c>, so matching on this is safe in every language.
    /// </remarks>
    private const string StandardMasterName = "Standard";
}

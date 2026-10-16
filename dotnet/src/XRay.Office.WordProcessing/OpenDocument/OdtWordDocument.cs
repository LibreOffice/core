using XRay.Office.Core.Diagnostics;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Formats;
using XRay.Office.Core.Units;
using System.Xml.Linq;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.OpenDocument;

/// <summary>
/// An ODF text document, with the page geometry a Writer document has and a spreadsheet does not.
/// </summary>
/// <remarks>
/// <para>
/// A wrapper rather than a subclass, because <see cref="OdfDocument"/> serves all three families and
/// lives in <c>XRay.Office.OpenDocument</c> — below this library in the dependency order, so it cannot
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

    /// <summary>
    /// What laying the document out found, which reading it could not have.
    /// </summary>
    /// <remarks>
    /// A second list rather than an addition to the reader's, because the two are produced at different
    /// times: <see cref="Diagnostics"/> is answerable the moment the document is open, and a picture that
    /// will not draw is only discovered when something asks for the pages. So the list starts empty, and a
    /// caller that has run <see cref="Layout"/> sees more than one that has not — which is the honest
    /// answer, since before layout nothing had looked.
    /// </remarks>
    private readonly List<Diagnostic> _laidOut = [];

    internal OdtWordDocument(OdfDocument inner, WritingMarks marks)
    {
        ArgumentNullException.ThrowIfNull(inner);
        _inner = inner;
        Marks = marks;
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
    public IReadOnlyList<Diagnostic> Diagnostics
        => _laidOut.Count == 0 ? _inner.Diagnostics : [.. _inner.Diagnostics, .. _laidOut];

    /// <inheritdoc/>
    public IReadOnlyList<WritingSection> Sections { get; }

    /// <inheritdoc/>
    public WritingMarks Marks { get; }

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
        => XRay.Office.Core.Threading.OfficeOperation.Layout(options, () => LayoutCore(options));

    private WordProcessingPages LayoutCore(LayoutOptions? options)
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
            stylesRoot: _inner.File.StylesRoot,
            pictures: new OdfPictures(_inner.File, _laidOut),
            settings: _inner.File.Settings,
            sectionMargins: [.. Sections.Select(section => section.Page.Margins.Left)],
            sectionCount: masters.Count);

        List<PageBlock> blocks = source.Read(body);

        PaginationOptions pagination = PaginationOptions.Default with
        {
            // Adding is the ODF default and what the preset already says, but a document converted from
            // a Word file carries AddParaTableSpacing=false and collapses instead — see
            // OdtLayoutSource.AddsParagraphSpacing.
            CollapsesSpacing = !OdtLayoutSource.AddsParagraphSpacing(_inner.File.Settings),
            // Keeping it is the ODF default too, and the preset had the opposite — see
            // OdtLayoutSource.KeepsParagraphSpacingAtPages, which measures what an absent item means.
            KeepsSpacingAtTopOfPage = OdtLayoutSource.KeepsParagraphSpacingAtPages(_inner.File.Settings),
            // The deadline a split floating table is cut at — the body's print bottom, or the page's
            // when the document carries Word's pre-2013 rule. See
            // OdtLayoutSource.FliesMayOverlapTheBottomMargin.
            FliesMayOverlapTheBottomMargin =
                OdtLayoutSource.FliesMayOverlapTheBottomMargin(_inner.File.Settings),
            MaxPages = options?.MaxPages is > 0 ? options.MaxPages : PaginationOptions.Default.MaxPages,
        };

        Paginator paginator = new(pagination);

        List<PaginatedSection> paginated = [];

        for (int index = 0; index < masters.Count; index++)
        {
            WritingSection stated = index < Sections.Count ? Sections[index] : Sections[^1];
            (PageFurnitureSet? furniture, Length header, Length footer) =
                Furnished(source, masters[index], stated.Page.TextWidth);

            paginated.Add(new PaginatedSection(
                GrownTo(stated, _inner.File.Styles, masters[index], header, footer), furniture));
        }

        // The sections a text:section asked for, derived from the master each one sits on so that they
        // keep its paper, its running heads and the header height measured above. They are numbered after
        // the masters because the walk allocated them that way, and every one of them breaks continuously:
        // a Writer text section starts where the flow reaches it.
        foreach (OdtColumnSection derived in source.ColumnSections)
        {
            PaginatedSection master = paginated[Math.Clamp(derived.Master, 0, paginated.Count - 1)];
            paginated.Add(new PaginatedSection(Derived(master.Section, derived.Geometry), master.Furniture));
        }

        return new WordProcessingPages(
            paginator.Paginate(blocks, paginated), paginator.Blocks ?? blocks);
    }

    /// <summary>
    /// The section re-read with its dynamic header and footer grown to the height their content needs.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="OdfPageGeometry"/> can only read what the file <em>states</em>, and for a dynamic
    /// height — <c>fo:min-height</c> rather than <c>svg:height</c> — the file states a floor.
    /// LibreOffice sizes the frame to its content and floors it at that minimum:
    /// <c>SwHeadFootFrame::FormatPrt</c> takes <c>nHeight = lcl_CalcContentHeight(*this)</c> whenever
    /// <c>!HasFixSize()</c> and raises it to <c>nMinHeight</c> only if it falls short
    /// (<c>sw/source/core/layout/hffrm.cxx</c>:114-145). So the rule
    /// <c>total = max(min-height, content + (dynamic-spacing ? 0 : gap))</c> — which
    /// <see cref="OdfPageGeometry"/> already states and whose first half
    /// <c>OdfHeaderDynamicSpacingTests</c> pins — needs the content, and the content can be measured
    /// here, where the walk that reads the header's blocks has already run.
    /// </para>
    /// <para>
    /// It is not a refinement. LibreOffice's exporter writes <c>fo:min-height="0.0398in"</c> — two and
    /// a half points — for a running head of any height, so a nine-paragraph header was being given
    /// three points of room and everything below it sat a hundred points too high on every page.
    /// <b>Thirty of the converted corpus's failing <c>.odt</c> declare a dynamic header or footer whose
    /// content plainly outruns its stated minimum</b>, and closing it moved seven of them onto the
    /// reference and none off it.
    /// </para>
    /// <para>
    /// Two limits of the model, both deliberate. The height is one number per <em>section</em> where
    /// Writer sizes each page's header to its own content, so a document whose first-page header
    /// differs in height from its default one is laid out on the default one's — the slot the
    /// overwhelming majority of its pages use. And the growth is refused outright if it would leave
    /// the body less than a line of room, which is a guard against a malformed file rather than a
    /// rule: Writer has no such cap and simply lets the body shrink.
    /// </para>
    /// </remarks>
    /// <summary>
    /// A master's section wearing what a <c>text:section</c> does to it, reached by a continuous break.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The indents are added to the master's margins rather than replacing them, because that is how the
    /// file states them — LibreOffice's importers put the <em>difference</em> between the section's own
    /// margins and the page style's on the section format, and its ODF export writes that difference out;
    /// see <see cref="OdfSectionGeometry"/> for the two citations.
    /// </para>
    /// <para>
    /// Everything else is the master's: the paper, the vertical margins, the running heads and the page
    /// numbering all belong to the sheet the section sits on, and a section frame changes none of them.
    /// A null geometry is the section that closes a <c>text:section</c>, which is the master's own
    /// geometry with a continuous break so that leaving the columns does not start a page.
    /// </para>
    /// </remarks>
    /// <param name="master">The section of the master page the text section sits on.</param>
    /// <param name="geometry">What the <c>text:section</c> states, or null to restore the master's.</param>
    private static WritingSection Derived(WritingSection master, OdfSectionGeometry? geometry)
    {
        PageMargins margins = master.Page.Margins with
        {
            Left = master.Page.Margins.Left + (geometry?.IndentLeft ?? Length.Zero),
            Right = master.Page.Margins.Right + (geometry?.IndentRight ?? Length.Zero),
        };

        return master with
        {
            Break = SectionBreak.Continuous,
            IsTextSection = true,
            RestartPageNumberAt = null,
            BalancesColumns = geometry?.BalancesColumns ?? false,
            Page = master.Page with
            {
                Margins = margins,
                Columns = geometry?.Columns ?? 1,
                ColumnGap = geometry?.ColumnGap ?? Length.Zero,
                ColumnRuler = geometry?.RulerFor(
                    master.Page.Size.Width - margins.Left - margins.Right - master.Page.Gutter),
            },
        };
    }

    /// <param name="stated">The section as the file states it, used when the growth is refused.</param>
    /// <param name="styles">The document's styles, to re-read the geometry through.</param>
    /// <param name="master">The master page the section is on.</param>
    /// <param name="header">The height the header's content needs, or zero when it has none.</param>
    /// <param name="footer">The same for the footer.</param>
    private static WritingSection GrownTo(
        WritingSection stated,
        OdfStyles styles,
        OdfMasterPage? master,
        Length header,
        Length footer)
    {
        if (header <= Length.Zero && footer <= Length.Zero) return stated;

        WritingSection grown = OdfPageGeometry.Read(styles, master, header, footer);
        PageGeometry page = grown.Page;

        // One line of body text, so a header measured absurdly tall cannot leave a page with nothing to
        // paginate into and no way to terminate.
        Length room = page.Size.Height - page.Margins.Top - page.Margins.Bottom;

        return room < Length.FromPoints(12) ? stated : grown;
    }

    /// <summary>
    /// One master page's furniture, and how tall its header's and footer's content actually are.
    /// </summary>
    /// <remarks>
    /// The heights are the <see cref="PageFurnitureSlot.Default"/> slot's where it exists and the
    /// tallest slot's otherwise, for the reason <see cref="GrownTo"/> gives: one number has to serve
    /// every page of the section, and the default slot is the one most of them use.
    /// </remarks>
    /// <param name="source">The walk that reads the blocks.</param>
    /// <param name="master">The master page, or null.</param>
    /// <param name="width">The width the furniture is laid out at, which is the body's text width.</param>
    private static (PageFurnitureSet? Set, Length Header, Length Footer) Furnished(
        OdtLayoutSource source, OdfMasterPage? master, Length width)
    {
        PageFurnitureSet? set = Furniture(source, master);
        if (set is null || master is null) return (set, Length.Zero, Length.Zero);

        Length header = Extent(source, master.Header, master.LeftHeader, master.FirstHeader, width);
        Length footer = Extent(source, master.Footer, master.LeftFooter, master.FirstFooter, width);

        return (set, header, footer);
    }

    /// <summary>The height one furniture flow's content needs, laid out at a width.</summary>
    /// <param name="source">The walk that reads the blocks.</param>
    /// <param name="preferred">The default slot's element, whose height wins when it exists.</param>
    /// <param name="even">The even-page slot's element.</param>
    /// <param name="first">The first-page slot's element.</param>
    /// <param name="width">The width to lay out at.</param>
    private static Length Extent(
        OdtLayoutSource source, XElement? preferred, XElement? even, XElement? first, Length width)
    {
        if (width <= Length.Zero) return Length.Zero;
        if (preferred is not null) return HeightOf(source, preferred, width);

        return Length.Max(
            even is null ? Length.Zero : HeightOf(source, even, width),
            first is null ? Length.Zero : HeightOf(source, first, width));
    }

    /// <summary>One flow's laid-out height.</summary>
    private static Length HeightOf(OdtLayoutSource source, XElement element, Length width)
    {
        List<PageBlock> blocks = source.ReadFlow(element);
        return blocks.Count == 0 ? Length.Zero : FlowLayouter.HeightOf(blocks, width);
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

        Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> headers = [];
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> footers = [];

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
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> slots,
        PageFurnitureSlot slot,
        XElement? element,
        OdtLayoutSource source)
    {
        if (element is null) return;

        List<PageBlock> blocks = source.ReadFlow(element);
        if (blocks.Count > 0) slots[slot] = blocks;
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

using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// A section's headers and footers, ready to be laid out into a page's furniture areas.
/// </summary>
/// <remarks>
/// <para>
/// The layout counterpart of <see cref="WritingSection.Headers"/>, holding paragraphs where the section
/// holds content. The two are kept apart because a header is read once and laid out once, while the
/// section's own copy is what an extraction caller reads — and because the layout engine has no business
/// knowing about the document model's bodies.
/// </para>
/// <para>
/// Laid out per <em>slot</em> and cached, not per page. Most pages of a document share one header, and
/// shaping its text again for each would be the largest single cost in paginating a long document for an
/// answer that cannot change.
/// </para>
/// <para>
/// The exception is a running head carrying a page number, which is a different running head on every
/// page: it is resolved by <see cref="PageFields"/> before it is laid out, and cached against the number
/// as well as the slot. Resolving it afterwards is not open to us — the number is text of a different
/// width from the cached one, so it has to take part in the line breaking rather than be painted over it.
/// </para>
/// </remarks>
public sealed class PageFurnitureSet
{
    private readonly Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> _headers;
    private readonly Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> _footers;
    private readonly Dictionary<(PageFurnitureSlot Slot, int PageNumber), PlacedFlow?> _laidOutHeaders = [];
    private readonly Dictionary<(PageFurnitureSlot Slot, int PageNumber), PlacedFlow?> _laidOutFooters = [];

    /// <summary>Creates a set from the blocks each slot holds.</summary>
    /// <param name="headers">The headers, by slot; a slot with no entry has no header.</param>
    /// <param name="footers">The footers, by slot.</param>
    public PageFurnitureSet(
        IReadOnlyDictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>? headers = null,
        IReadOnlyDictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>? footers = null)
    {
        _headers = headers is null ? [] : new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>(headers);
        _footers = footers is null ? [] : new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>(footers);
    }

    /// <summary>True when the set holds nothing, so a page needs no furniture at all.</summary>
    public bool IsEmpty => _headers.Count == 0 && _footers.Count == 0;

    /// <summary>True when any of this set's furniture carries a <c>NUMPAGES</c> field.</summary>
    /// <remarks>
    /// Asked by the paginator to decide whether the document needs the second pass at all — a page count
    /// costs one extra fill of the whole document, and almost no document carries one.
    /// </remarks>
    public bool CarriesPageCount
    {
        get
        {
            foreach (IReadOnlyList<PageBlock> blocks in _headers.Values)
            {
                if (PageFields.CarriesPageCount(blocks)) return true;
            }

            foreach (IReadOnlyList<PageBlock> blocks in _footers.Values)
            {
                if (PageFields.CarriesPageCount(blocks)) return true;
            }

            return false;
        }
    }

    /// <summary>
    /// How many pages the document turned out to have, or nought while that is not yet known.
    /// </summary>
    /// <remarks>
    /// Set between the two passes of a layout that has to resolve a <c>NUMPAGES</c> field. Assigning it
    /// discards the laid-out cache, because every flow in it was laid out against the old answer — and a
    /// stale cache here is exactly the defect this whole file exists to undo, one step further along.
    /// </remarks>
    public int TotalPages
    {
        get => _totalPages;
        set
        {
            if (_totalPages == value) return;

            _totalPages = value;
            _laidOutHeaders.Clear();
            _laidOutFooters.Clear();
        }
    }

    private int _totalPages;

    /// <summary>The header a page takes, laid out, or null when it has none.</summary>
    /// <param name="section">The section, for its slot rules.</param>
    /// <param name="geometry">The page's geometry, for the header's area.</param>
    /// <param name="pageNumber">The page's printed number.</param>
    /// <param name="isFirstPageOfSection">True for the section's own first page.</param>
    /// <param name="collapsesSpacing">
    /// Whether the paragraphs of the running head collapse their spacing against one another rather than
    /// adding it — see <see cref="FlowLayouter.LayOut"/>. A header is a frame like any other and Writer
    /// measures the gap above its paragraphs with the same method it uses in the body.
    /// </param>
    /// <param name="addsCellLineSpacing">
    /// Whether a table in the running head grows its cells by their last paragraph's proportional line
    /// spacing — see <see cref="PaginationOptions.AddsCellLineSpacing"/>. A header laid out as a table is
    /// the ordinary way a Word document puts a logo beside a title, so this reaches many of them.
    /// </param>
    public PlacedFlow? Header(
        WritingSection section,
        PageGeometry geometry,
        int pageNumber,
        bool isFirstPageOfSection,
        bool collapsesSpacing = false,
        bool addsCellLineSpacing = false)
        => Resolve(
            _headers, _laidOutHeaders, section, geometry.HeaderArea, pageNumber, isFirstPageOfSection,
            offsetFromTop: Length.Zero, collapsesSpacing, addsCellLineSpacing);

    /// <summary>
    /// The footer a page takes, laid out, or null when it has none.
    /// </summary>
    /// <remarks>
    /// Placed by <see cref="PageGeometry.FooterOffset"/>: an offset puts its first line that far below the
    /// area's top and no offset bottom-aligns it. Both rules are real — see that property for which format
    /// uses which.
    /// </remarks>
    /// <param name="section">The section, for its slot rules.</param>
    /// <param name="geometry">The page's geometry, for the footer's area.</param>
    /// <param name="pageNumber">The page's printed number.</param>
    /// <param name="isFirstPageOfSection">True for the section's own first page.</param>
    /// <param name="collapsesSpacing">As <see cref="Header"/>'s.</param>
    /// <param name="addsCellLineSpacing">As <see cref="Header"/>'s.</param>
    /// <remarks>
    /// A footer is suppressed on a title page exactly as a header is. This used to pass
    /// <c>mayBeSuppressed: false</c>, on the grounds that the corpus evidence for footers contradicted
    /// itself — and it did, but the resolution was to measure rather than to apply the half that looked
    /// established. The probe in <see cref="ChosenSlot"/>'s remarks shows the reference drawing no
    /// page-one footer in three of its four cases, including the two where a first-page *header* is named
    /// and where the old rule copied the default footer onto the title page.
    /// </remarks>
    public PlacedFlow? Footer(
        WritingSection section,
        PageGeometry geometry,
        int pageNumber,
        bool isFirstPageOfSection,
        bool collapsesSpacing = false,
        bool addsCellLineSpacing = false)
        => Resolve(
            _footers, _laidOutFooters, section, geometry.FooterArea, pageNumber, isFirstPageOfSection,
            offsetFromTop: geometry.FooterOffset, collapsesSpacing, addsCellLineSpacing);


    private PlacedFlow? Resolve(
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> slots,
        Dictionary<(PageFurnitureSlot Slot, int PageNumber), PlacedFlow?> cache,
        WritingSection section,
        DocRect area,
        int pageNumber,
        bool isFirstPageOfSection,
        Length? offsetFromTop,
        bool collapsesSpacing,
        bool addsCellLineSpacing)
    {
        PageFurnitureSlot? chosen = ChosenSlot(
            slots, pageNumber, isFirstPageOfSection,
            section.HasDifferentFirstPage, section.HasDifferentEvenPages);

        if (chosen is not { } slot) return null;
        if (!slots.TryGetValue(slot, out IReadOnlyList<PageBlock>? blocks)) return null;

        // A running head carrying a page number is a different running head on every page, so the cache
        // has to be keyed on the number as well as on the slot. Everything else is the same on every page
        // and is keyed on the slot alone — which is the case that matters, because re-shaping one header
        // per page is the largest single cost in paginating a long document.
        bool varies = PageFields.CarriesPageNumber(blocks);
        var key = (slot, varies ? pageNumber : 0);
        if (cache.TryGetValue(key, out PlacedFlow? cached)) return cached;

        // A page *count* does not vary from page to page, so it does not reach the cache key — but it does
        // have to be substituted once the total is known, which is why the resolve is asked for whenever
        // either kind is present rather than only when the head varies.
        bool resolves = varies || (_totalPages > 0 && PageFields.CarriesPageCount(blocks));

        PlacedFlow? placed = FlowLayouter.LayOut(
            resolves
                ? PageFields.Resolve(blocks, pageNumber, section.PageNumberFormat, _totalPages)
                : blocks,
            area, offsetFromTop,
            collapsesSpacing: collapsesSpacing,
            addsCellLineSpacing: addsCellLineSpacing,

            // A running head or foot is the one flow where a positioned table is a frame its neighbours do
            // not move out of the way of — see `FlowLayouter.LayOut`, which carries the measurement.
            floatsPositionedTables: true);
        cache[key] = placed;
        return placed;
    }

    /// <summary>
    /// Which slot a page takes, as a slot rather than as its contents.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same rules <see cref="PageFurnitureSlots"/> states, asked in terms of the slot so that the
    /// answer can be cached against it. Asking for the contents and caching against those would key the
    /// cache on a list that two slots could share.
    /// </para>
    /// <para>
    /// Null means <em>no furniture at all</em>, which is not the same as "fall back to the default one".
    /// **A title page carries only what the section named for a first page, and nothing is copied onto it
    /// from the ordinary slot.** Measured directly rather than inferred, with a four-document probe that
    /// varies only which first-page parts a <c>w:titlePg</c> section names:
    /// </para>
    /// <list type="table">
    /// <item><description>names neither → LibreOffice's page one carries <em>nothing</em>;</description></item>
    /// <item><description>names a first <em>header</em> only → that header, and <em>no footer</em>;</description></item>
    /// <item><description>names a first <em>footer</em> only → that footer, and <em>no header</em>;</description></item>
    /// <item><description>names both → both.</description></item>
    /// </list>
    /// <para>
    /// This replaces a rule derived from three corpus documents that said naming any first-page part makes
    /// a first-page style onto which the *other* kind is copied from the ordinary one — attributed to
    /// writerfilter's <c>copyHeaderFooter</c> (<c>writerfilter/dmapper/PropertyMap.cxx:1117-1125</c>). The
    /// probe contradicts it in three of its four cases, and the corpus evidence behind it was already
    /// self-contradictory: two documents of *identical* shape (<c>final-technical-report-template.docx</c>
    /// and <c>Agile_Arc_SysDes.docx</c>, both <c>w:titlePg</c> with default parts and nothing named for a
    /// first page) were recorded as differing, one with a page-one footer and one without. Two documents
    /// that differ in their output while agreeing in every input are a sign that the input being compared
    /// is not the one that decides — not a fact to build a rule on. It was found from
    /// <c>AC-150-5370-10G-updated-201604.docx</c>, whose first section names a first-page header and no
    /// first-page footer, and whose reference page one carries no page number where ours drew <c>i</c>.
    /// </para>
    /// <para>
    /// Inheritance across sections is settled before this is asked — <c>DocxReader.Paginated</c> carries
    /// each slot forward per ECMA-376 §17.10.1 — so a First slot missing here is one no earlier section
    /// supplied either.
    /// </para>
    /// </remarks>
    private static PageFurnitureSlot? ChosenSlot(
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> slots,
        int pageNumber,
        bool isFirstPageOfSection,
        bool hasDifferentFirstPage,
        bool hasDifferentEvenPages)
    {
        if (isFirstPageOfSection && hasDifferentFirstPage)
        {
            // Only what the section named for a first page. Nothing is copied from the ordinary slot —
            // see the probe table in the remarks above.
            return slots.ContainsKey(PageFurnitureSlot.First) ? PageFurnitureSlot.First : null;
        }

        if (hasDifferentEvenPages
            && pageNumber % 2 == 0
            && slots.ContainsKey(PageFurnitureSlot.Even))
        {
            return PageFurnitureSlot.Even;
        }

        return PageFurnitureSlot.Default;
    }
}

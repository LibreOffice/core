using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.Layout;

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
/// answer that cannot change. What does change per page — a page number in a header — is a field, and
/// resolving fields is a later pass than this.
/// </para>
/// </remarks>
public sealed class PageFurnitureSet
{
    private readonly Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> _headers;
    private readonly Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> _footers;
    private readonly Dictionary<PageFurnitureSlot, PlacedFlow?> _laidOutHeaders = [];
    private readonly Dictionary<PageFurnitureSlot, PlacedFlow?> _laidOutFooters = [];

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
    public PlacedFlow? Header(
        WritingSection section,
        PageGeometry geometry,
        int pageNumber,
        bool isFirstPageOfSection,
        bool collapsesSpacing = false)
        => Resolve(
            _headers, _laidOutHeaders, section, geometry.HeaderArea, pageNumber, isFirstPageOfSection,
            offsetFromTop: Length.Zero, collapsesSpacing);

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
    public PlacedFlow? Footer(
        WritingSection section,
        PageGeometry geometry,
        int pageNumber,
        bool isFirstPageOfSection,
        bool collapsesSpacing = false)
        => Resolve(
            _footers, _laidOutFooters, section, geometry.FooterArea, pageNumber, isFirstPageOfSection,
            offsetFromTop: geometry.FooterOffset, collapsesSpacing);

    private static PlacedFlow? Resolve(
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> slots,
        Dictionary<PageFurnitureSlot, PlacedFlow?> cache,
        WritingSection section,
        DocRect area,
        int pageNumber,
        bool isFirstPageOfSection,
        Length? offsetFromTop,
        bool collapsesSpacing)
    {
        PageFurnitureSlot? chosen = ChosenSlot(
            slots, pageNumber, isFirstPageOfSection,
            section.HasDifferentFirstPage, section.HasDifferentEvenPages);

        if (chosen is not { } slot) return null;
        if (!slots.TryGetValue(slot, out IReadOnlyList<PageBlock>? blocks)) return null;
        if (cache.TryGetValue(slot, out PlacedFlow? cached)) return cached;

        PlacedFlow? placed = FlowLayouter.LayOut(
            blocks, area, offsetFromTop, collapsesSpacing: collapsesSpacing);
        cache[slot] = placed;
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
    /// Null means <em>no furniture</em>, which is not the same as "fall back to the default one". A
    /// section that says its first page is different and supplies nothing for it draws nothing there:
    /// "different" is the whole of what the flag asserts, and Writer keeps a separate first-page page
    /// descriptor whose empty header is empty rather than inherited. Falling through to the default slot
    /// puts the running head on a title page, which is both visible and a line's worth of room the page
    /// should have had. Measured on <c>final-technical-report-template.docx</c>, whose first section names
    /// a default header and a <c>w:titlePg</c> and no <c>w:type="first"</c> reference at all: LibreOffice's
    /// page one carries no running head and ours carried the default one.
    /// </para>
    /// <para>
    /// Inheritance is already settled before this is asked — <c>DocxReader.Paginated</c> carries each slot
    /// across sections per ECMA-376 §17.10.1 — so a First slot missing here is one no earlier section
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

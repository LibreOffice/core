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
    private readonly Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>> _headers;
    private readonly Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>> _footers;
    private readonly Dictionary<PageFurnitureSlot, PlacedFlow?> _laidOutHeaders = [];
    private readonly Dictionary<PageFurnitureSlot, PlacedFlow?> _laidOutFooters = [];

    /// <summary>Creates a set from the paragraphs each slot holds.</summary>
    /// <param name="headers">The headers, by slot; a slot with no entry has no header.</param>
    /// <param name="footers">The footers, by slot.</param>
    public PageFurnitureSet(
        IReadOnlyDictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>>? headers = null,
        IReadOnlyDictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>>? footers = null)
    {
        _headers = headers is null ? [] : new Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>>(headers);
        _footers = footers is null ? [] : new Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>>(footers);
    }

    /// <summary>True when the set holds nothing, so a page needs no furniture at all.</summary>
    public bool IsEmpty => _headers.Count == 0 && _footers.Count == 0;

    /// <summary>The header a page takes, laid out, or null when it has none.</summary>
    /// <param name="section">The section, for its slot rules.</param>
    /// <param name="geometry">The page's geometry, for the header's area.</param>
    /// <param name="pageNumber">The page's printed number.</param>
    /// <param name="isFirstPageOfSection">True for the section's own first page.</param>
    public PlacedFlow? Header(
        WritingSection section, PageGeometry geometry, int pageNumber, bool isFirstPageOfSection)
        => Resolve(
            _headers, _laidOutHeaders, section, geometry.HeaderArea, pageNumber, isFirstPageOfSection,
            offsetFromTop: Length.Zero);

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
    public PlacedFlow? Footer(
        WritingSection section, PageGeometry geometry, int pageNumber, bool isFirstPageOfSection)
        => Resolve(
            _footers, _laidOutFooters, section, geometry.FooterArea, pageNumber, isFirstPageOfSection,
            offsetFromTop: geometry.FooterOffset);

    private static PlacedFlow? Resolve(
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>> slots,
        Dictionary<PageFurnitureSlot, PlacedFlow?> cache,
        WritingSection section,
        DocRect area,
        int pageNumber,
        bool isFirstPageOfSection,
        Length? offsetFromTop)
    {
        PageFurnitureSlot slot = ChosenSlot(
            slots, pageNumber, isFirstPageOfSection,
            section.HasDifferentFirstPage, section.HasDifferentEvenPages);

        if (!slots.TryGetValue(slot, out IReadOnlyList<PageParagraph>? paragraphs)) return null;
        if (cache.TryGetValue(slot, out PlacedFlow? cached)) return cached;

        PlacedFlow? placed = FlowLayouter.LayOut(paragraphs, area, offsetFromTop);
        cache[slot] = placed;
        return placed;
    }

    /// <summary>
    /// Which slot a page takes, as a slot rather than as its contents.
    /// </summary>
    /// <remarks>
    /// The same rules <see cref="PageFurnitureSlots"/> states, asked in terms of the slot so that the
    /// answer can be cached against it. Asking for the contents and caching against those would key the
    /// cache on a list that two slots could share.
    /// </remarks>
    private static PageFurnitureSlot ChosenSlot(
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageParagraph>> slots,
        int pageNumber,
        bool isFirstPageOfSection,
        bool hasDifferentFirstPage,
        bool hasDifferentEvenPages)
    {
        if (isFirstPageOfSection
            && hasDifferentFirstPage
            && slots.ContainsKey(PageFurnitureSlot.First))
        {
            return PageFurnitureSlot.First;
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

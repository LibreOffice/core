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
    /// <remarks>
    /// The title-page suppression <see cref="Header"/> applies is deliberately <em>not</em> applied here.
    /// It is measured for headers in both directions and the footer evidence contradicts itself: the
    /// reference's first page of <c>final-technical-report-template.docx</c> has no footer, and its first
    /// page of <c>Agile_Arc_SysDes.docx</c> — the same shape of section, a default header and footer with
    /// <c>w:titlePg</c> and nothing named for a first page — has one. Suppressing both cost that document
    /// eight words and bought nothing, so the half that is established is the half that is applied.
    /// </remarks>
    public PlacedFlow? Footer(
        WritingSection section,
        PageGeometry geometry,
        int pageNumber,
        bool isFirstPageOfSection,
        bool collapsesSpacing = false)
        => Resolve(
            _footers, _laidOutFooters, section, geometry.FooterArea, pageNumber, isFirstPageOfSection,
            offsetFromTop: geometry.FooterOffset, collapsesSpacing, mayBeSuppressed: false);

    /// <summary>
    /// True when something — a header or a footer — was named for a first page.
    /// </summary>
    /// <remarks>
    /// Asked of both maps together on purpose, and it is the whole of what distinguishes the two
    /// behaviours below: a section that names <em>any</em> first-page part gets a first-page page style,
    /// and the part it did not name is copied onto that style from the section's ordinary one. A section
    /// that names none gets no such style, and its first page carries no furniture at all.
    /// </remarks>
    private bool HasFirstPageFurniture
        => _headers.ContainsKey(PageFurnitureSlot.First) || _footers.ContainsKey(PageFurnitureSlot.First);

    private PlacedFlow? Resolve(
        Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> slots,
        Dictionary<PageFurnitureSlot, PlacedFlow?> cache,
        WritingSection section,
        DocRect area,
        int pageNumber,
        bool isFirstPageOfSection,
        Length? offsetFromTop,
        bool collapsesSpacing,
        bool mayBeSuppressed = true)
    {
        PageFurnitureSlot? chosen = ChosenSlot(
            slots, pageNumber, isFirstPageOfSection,
            section.HasDifferentFirstPage, section.HasDifferentEvenPages,
            hasFirstPageFurniture: HasFirstPageFurniture || !mayBeSuppressed);

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
    /// Null means <em>no furniture at all</em>, which is not the same as "fall back to the default one",
    /// and which of the two a title page gets is decided by a pair of corpus documents that differ in
    /// exactly one thing. Both state <c>w:titlePg</c> and neither names a first-page <em>header</em>:
    /// </para>
    /// <list type="bullet">
    /// <item><description>
    /// <c>batch-007/final-technical-report-template.docx</c> names a default header, a default footer and
    /// nothing for a first page. LibreOffice's page one carries <em>neither</em> a running head nor a
    /// footer.
    /// </description></item>
    /// <item><description>
    /// <c>batch-016/JEMIT_Template.docx</c> names even and default headers, even and default footers, and
    /// a <c>w:footerReference w:type="first"</c>. LibreOffice's page one carries the <em>default</em>
    /// header — the one it never named for a first page — along with that first-page footer.
    /// </description></item>
    /// </list>
    /// <para>
    /// So the switch is whether the section named a first page part at all, of either kind: naming one
    /// makes a first-page style and the other kind is copied onto it, naming none means the first page has
    /// no furniture. That is the shape of writerfilter's <c>copyHeaderFooter</c>
    /// (<c>writerfilter/dmapper/PropertyMap.cxx:1117-1125</c>), which links a section's page style to the
    /// previous one's for the parts it did not set, beside the branch at <c>:594-598</c> that forces
    /// <c>HeaderIsOn</c> false for a section that pushed no applicable header. Which of the two wins was
    /// left unnamed by an earlier round; the pair above settles it by measurement.
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
        bool hasDifferentEvenPages,
        bool hasFirstPageFurniture)
    {
        if (isFirstPageOfSection && hasDifferentFirstPage)
        {
            if (slots.ContainsKey(PageFurnitureSlot.First)) return PageFurnitureSlot.First;
            if (!hasFirstPageFurniture) return null;

            // The section named a first-page part of the other kind, so it has a first-page style and this
            // kind is copied onto it from the ordinary one.
            return PageFurnitureSlot.Default;
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

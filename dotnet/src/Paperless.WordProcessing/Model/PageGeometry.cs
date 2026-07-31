using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.WordProcessing.Model;

/// <summary>
/// The four page margins.
/// </summary>
/// <remarks>
/// Separate from <see cref="PageGeometry"/> so a header or footer can be given the page's horizontal
/// margins without inheriting its vertical ones, which are measured differently in every format.
/// </remarks>
/// <param name="Left">The left margin.</param>
/// <param name="Right">The right margin.</param>
/// <param name="Top">The top margin.</param>
/// <param name="Bottom">The bottom margin.</param>
public readonly record struct PageMargins(Length Left, Length Right, Length Top, Length Bottom)
{
    /// <summary>The 2 cm margins a blank Writer document starts with.</summary>
    public static PageMargins Default { get; } = Uniform(Length.FromMm100(2000));

    /// <summary>The same margin on all four sides.</summary>
    public static PageMargins Uniform(Length all) => new(all, all, all, all);

    /// <summary>How much width the left and right margins take together.</summary>
    public Length Horizontal => Left + Right;

    /// <summary>How much height the top and bottom margins take together.</summary>
    public Length Vertical => Top + Bottom;
}

/// <summary>
/// Which of a section's three header or footer slots is meant.
/// </summary>
/// <remarks>
/// All four formats have the same three, and all four spell them differently: DOCX writes
/// <c>w:type</c> of <c>default</c>, <c>first</c> and <c>even</c>; RTF has <c>\header</c>,
/// <c>\headerf</c> and <c>\headerl</c>/<c>\headerr</c>; DOC gives each section six consecutive
/// stories; ODF splits a master page's header into <c>style:header</c> and
/// <c>style:header-left</c>. One enumeration for all of them is what lets layout ask the question once.
/// </remarks>
public enum PageFurnitureSlot
{
    /// <summary>Used by any page no other slot claims.</summary>
    Default,

    /// <summary>Used by the section's first page, when the section asks for a different one.</summary>
    First,

    /// <summary>Used by even-numbered pages, when the section asks for mirrored pages.</summary>
    Even,
}

/// <summary>
/// A page's physical geometry: how big it is, and where the text sits on it.
/// </summary>
/// <remarks>
/// <para>
/// Every format states this in twips or in hundredths of a millimetre, and every format states it
/// slightly differently — so the conversion happens in the reader and this holds the resolved answer
/// in EMUs. That is the point of a single unit: <see cref="TextWidth"/> is an exact integer whether the
/// document said 9639 twips or 17000 hundredths of a millimetre.
/// </para>
/// <para>
/// <see cref="TextWidth"/> is the number line breaking is decided against, so it is worth being
/// precise about what is <em>not</em> in it. The gutter is, because it is extra binding margin added to
/// the inside edge. The header and footer are not: in Word's model they live inside the top and bottom
/// margins and grow into the text area only when they are taller than the margin allows, which is a
/// layout decision rather than a property of the page.
/// </para>
/// </remarks>
public sealed record PageGeometry
{
    /// <summary>
    /// A4, in hundredths of a millimetre — the unit ODF states it in, so it is exact there.
    /// </summary>
    /// <remarks>
    /// Spelled out as constants rather than taken from <see cref="Default"/>, because a property
    /// initialiser that reads a static of its own type runs <em>during</em> that static's construction
    /// and gets a half-built object. The result is a null-reference exception the first time anything
    /// touches the type, thrown from the initialiser rather than from the caller — so the shape here is
    /// load-bearing rather than stylistic.
    /// </remarks>
    private const long A4WidthMm100 = 21000;
    private const long A4HeightMm100 = 29700;

    /// <summary>A4 portrait with 2 cm margins, which is what a blank Writer document is.</summary>
    public static PageGeometry Default { get; } = new();

    /// <summary>US Letter portrait with one-inch margins, Word's default in an American locale.</summary>
    public static PageGeometry Letter { get; } = new()
    {
        Size = new DocSize(Length.FromTwips(12240), Length.FromTwips(15840)),
        Margins = PageMargins.Uniform(Length.FromTwips(1440)),
    };

    /// <summary>The paper size, as the document states it rather than as a named size.</summary>
    public DocSize Size { get; init; } =
        new(Length.FromMm100(A4WidthMm100), Length.FromMm100(A4HeightMm100));

    /// <summary>
    /// The margins around the <em>body</em> text area.
    /// </summary>
    /// <remarks>
    /// The body's, which is not what every format calls its top margin. Word's <c>w:top</c> is the
    /// distance from the page edge to the first line of body text, with the header living above it;
    /// ODF's <c>fo:margin-top</c> is the distance to the top of the <em>header</em>, and the header
    /// and its spacing then push the body further down. Storing the body's is what pagination needs
    /// — it is the number that decides how much text fits — so the ODF reader adds the header's
    /// extent and the Word readers take the value as given.
    /// </remarks>
    public PageMargins Margins { get; init; } = PageMargins.Default;

    /// <summary>
    /// Extra binding margin, added to the inside edge.
    /// </summary>
    /// <remarks>
    /// Inside rather than left, because a document with mirrored margins puts it on the right of a
    /// left-hand page. It narrows the text area either way, which is why it is part of
    /// <see cref="TextWidth"/>.
    /// </remarks>
    public Length Gutter { get; init; }

    /// <summary>
    /// The distance from the top of the page to the top of the header.
    /// </summary>
    /// <remarks>
    /// Measured from the page edge, not from the margin — which is how DOCX's <c>w:header</c> and
    /// RTF's <c>\headery</c> state it. ODF states a header's height and its distance from the body
    /// instead, so the reader converts.
    /// </remarks>
    public Length HeaderDistance { get; init; }

    /// <summary>The distance from the bottom of the page to the bottom of the footer.</summary>
    public Length FooterDistance { get; init; }

    /// <summary>
    /// How much of the top margin the header occupies, its spacing from the body included.
    /// </summary>
    /// <remarks>
    /// <para>
    /// So <c>HeaderDistance + HeaderHeight</c> is <see cref="PageMargins.Top"/> for a page whose
    /// header fits inside its margin. Kept alongside the margin rather than derived from it because
    /// each format states one of the two and implies the other, and which one it states decides what
    /// a reader can be exact about.
    /// </para>
    /// <para>
    /// The honest caveat: ODF declares a header's height, while LibreOffice lays the header out and
    /// uses the result. The two differ whenever the header's content does not fill its declared
    /// height, and closing that gap needs the header laid out before the page it sits on — which is
    /// the case LibreOffice's own exporter calls "totally nonoptimum, but the best we can do"
    /// (<c>sw/source/filter/ww8/writerwordglue.cxx</c>, <c>CalcHdFtDist</c>).
    /// </para>
    /// </remarks>
    public Length HeaderHeight { get; init; }

    /// <summary>How much of the bottom margin the footer occupies, its spacing included.</summary>
    public Length FooterHeight { get; init; }

    /// <summary>How many columns the text area is divided into; one for ordinary text.</summary>
    public int Columns { get; init; } = 1;

    /// <summary>The gap between columns, when there is more than one.</summary>
    public Length ColumnGap { get; init; }

    /// <summary>True when the page is wider than it is tall, as the document declares it.</summary>
    /// <remarks>
    /// Taken from the document's own orientation flag rather than derived by comparing the two
    /// dimensions, because the two can disagree: a producer may write a landscape flag with portrait
    /// dimensions, and which one wins differs by format. The reader decides; this records the answer.
    /// </remarks>
    public bool IsLandscape { get; init; }

    /// <summary>
    /// True when the margins swap on facing pages, so the gutter stays on the binding edge.
    /// </summary>
    public bool HasMirroredMargins { get; init; }

    /// <summary>The width a line of body text has to fit in.</summary>
    public Length TextWidth
    {
        get
        {
            Length width = Size.Width - Margins.Horizontal - Gutter;
            return width > Length.Zero ? width : Length.Zero;
        }
    }

    /// <summary>The height available to body text before the page is full.</summary>
    public Length TextHeight
    {
        get
        {
            Length height = Size.Height - Margins.Vertical;
            return height > Length.Zero ? height : Length.Zero;
        }
    }

    /// <summary>The width of one column, with the gaps between them taken out.</summary>
    public Length ColumnWidth
    {
        get
        {
            if (Columns <= 1) return TextWidth;

            Length available = TextWidth - (ColumnGap * (Columns - 1));
            return available > Length.Zero ? available / Columns : Length.Zero;
        }
    }

    /// <summary>The text area's rectangle on the page.</summary>
    public DocRect TextArea =>
        new(Margins.Left + Gutter, Margins.Top, TextWidth, TextHeight);
}

/// <summary>
/// One section of a document: a page description, and the furniture that goes round it.
/// </summary>
/// <remarks>
/// <para>
/// A section is where a document changes page geometry — a landscape page in the middle of a report, a
/// two-column stretch, a fresh set of headers. Every format has the concept and each attaches it
/// somewhere different: DOCX hangs a <c>w:sectPr</c> off the last paragraph of the section it ends,
/// DOC keeps a table of section descriptors indexed by character position, RTF resets with
/// <c>\sectd</c> and ends with <c>\sect</c>, and ODF applies a master page through a paragraph style.
/// The readers converge on this.
/// </para>
/// <para>
/// The furniture is referenced by flow rather than held inline, because a header belongs to a page and
/// not to a position in the text — and because two sections routinely share one.
/// </para>
/// </remarks>
public sealed record WritingSection
{
    /// <summary>The section's page geometry.</summary>
    public PageGeometry Page { get; init; } = PageGeometry.Default;

    /// <summary>The section's headers, by slot.</summary>
    public IReadOnlyDictionary<PageFurnitureSlot, WritingBody> Headers { get; init; } =
        new Dictionary<PageFurnitureSlot, WritingBody>();

    /// <summary>The section's footers, by slot.</summary>
    public IReadOnlyDictionary<PageFurnitureSlot, WritingBody> Footers { get; init; } =
        new Dictionary<PageFurnitureSlot, WritingBody>();

    /// <summary>
    /// The page number the section restarts at, or null when numbering continues.
    /// </summary>
    /// <remarks>
    /// Nullable rather than zero-means-continue: a document can legitimately restart at zero, and a
    /// title page numbered 0 so that the following page is 1 is a real thing people do.
    /// </remarks>
    public int? RestartPageNumberAt { get; init; }

    /// <summary>True when the section's first page uses the <c>First</c> furniture slot.</summary>
    public bool HasDifferentFirstPage { get; init; }

    /// <summary>True when the section distinguishes even from odd pages.</summary>
    public bool HasDifferentEvenPages { get; init; }

    /// <summary>
    /// The furniture for a page, or null when the section has none for it.
    /// </summary>
    /// <remarks>
    /// The slot rules in one place, because all four formats share them and none states them: the
    /// first page takes the first-page slot only if the section asked for one, an even page takes the
    /// even slot only if it asked for that, and anything else falls back to the default slot. Falling
    /// back is the part that is easy to miss — a section with only a default header still has a header
    /// on its first page.
    /// </remarks>
    public WritingBody? HeaderFor(int pageNumber, bool isFirstPageOfSection)
        => Furniture(Headers, pageNumber, isFirstPageOfSection);

    /// <summary>The footer for a page, by the same rules as <see cref="HeaderFor"/>.</summary>
    public WritingBody? FooterFor(int pageNumber, bool isFirstPageOfSection)
        => Furniture(Footers, pageNumber, isFirstPageOfSection);

    private WritingBody? Furniture(
        IReadOnlyDictionary<PageFurnitureSlot, WritingBody> slots,
        int pageNumber,
        bool isFirstPageOfSection)
    {
        if (isFirstPageOfSection
            && HasDifferentFirstPage
            && slots.TryGetValue(PageFurnitureSlot.First, out WritingBody? first))
        {
            return first;
        }

        if (HasDifferentEvenPages
            && pageNumber % 2 == 0
            && slots.TryGetValue(PageFurnitureSlot.Even, out WritingBody? even))
        {
            return even;
        }

        return slots.GetValueOrDefault(PageFurnitureSlot.Default);
    }
}

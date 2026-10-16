using XRay.Office.Core.Graphics;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;

namespace XRay.Office.WordProcessing.Model;

/// <summary>Which pages of a section a page border is drawn on.</summary>
public enum PageBorderDisplay
{
    /// <summary>Every page of the section — <c>w:display="allPages"</c>, and the default.</summary>
    AllPages,

    /// <summary>Its first page only — <c>w:display="firstPage"</c>.</summary>
    FirstPage,

    /// <summary>Every page but its first — <c>w:display="notFirstPage"</c>.</summary>
    NotFirstPage,
}

/// <summary>One side of a page border.</summary>
/// <param name="Width">The line's width; zero for a side that draws nothing.</param>
/// <param name="Colour">The line's colour.</param>
/// <param name="Space">
/// How far the line stands off the edge it is measured from — the paper's edge or the text's,
/// depending on <see cref="PageBorders.OffsetFromText"/>. Word states this in whole points.
/// </param>
public readonly record struct PageBorderSide(Length Width, Colour Colour, Length Space)
{
    /// <summary>True when the side draws a line at all.</summary>
    public bool Draws => Width > Length.Zero;
}

/// <summary>
/// A border drawn round the page rather than round a paragraph — Word's <c>w:pgBorders</c>.
/// </summary>
/// <remarks>
/// <para>
/// It is page furniture and not content: with <see cref="OffsetFromText"/> false the rectangle is
/// measured from the paper's edge and does not touch the text area at all, which is why it can be
/// carried on the geometry and drawn without anything in the layout knowing about it.
/// </para>
/// <para>
/// <strong>The shadow shrinks the rectangle rather than growing the page.</strong> Measured off
/// 26.2.4.2's own PDF of <c>Case-Study-Heathrow-Airport.docx</c> — A4, <c>w:sz="36"</c> (4.5 pt),
/// <c>w:space="15"</c>, <c>w:shadow="1"</c> — the four strokes are 4.5 pt wide at
/// <c>#396533</c> with their centrelines at 17.25 from the left and top, 573.60 from the left
/// (21.70 from the right) and 21.69 from the bottom: the right and bottom edges come in by the
/// shadow's own width, and the shadow is two black rectangles offset by it,
/// <c>19.4 15.039 560.85 4.45 re f*</c> and <c>575.8 19.439 4.45 803.05 re f*</c>.
/// </para>
/// </remarks>
public sealed record PageBorders
{
    /// <summary>The top side.</summary>
    public PageBorderSide Top { get; init; }

    /// <summary>The left side.</summary>
    public PageBorderSide Left { get; init; }

    /// <summary>The bottom side.</summary>
    public PageBorderSide Bottom { get; init; }

    /// <summary>The right side.</summary>
    public PageBorderSide Right { get; init; }

    /// <summary>
    /// True when the spacing is measured from the text rather than from the paper's edge —
    /// <c>w:offsetFrom="text"</c>.
    /// </summary>
    public bool OffsetFromText { get; init; }

    /// <summary>
    /// How wide a shadow the border casts down and to the right, or zero for none.
    /// </summary>
    /// <remarks>
    /// A length rather than a flag because the three readers state its width differently and only one
    /// of them derives it from the border. <strong>Word puts the shadow on every side and takes its
    /// width from the <em>right</em> side alone</strong> — <c>ApplyBorderToPageStyles</c> reads
    /// <c>m_bBorderShadows[BORDER_RIGHT]</c> and <c>getShadowFromBorder</c> takes that side's
    /// <c>LineWidth</c> (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:715-734), and WW8's
    /// <c>SwWW8ImplReader::SetShadow</c> reads <c>pbrc[WW8_RIGHT].fShadow()</c> with a floor of sixteen
    /// twips (<c>ww8par6.cxx</c>:1548-1562). ODF states the offset outright, in
    /// <c>style:shadow</c>.
    /// </remarks>
    public Length Shadow { get; init; }

    /// <summary>Which pages of the section carry it.</summary>
    public PageBorderDisplay Display { get; init; }

    /// <summary>True when at least one side draws a line.</summary>
    public bool Draws => Top.Draws || Left.Draws || Bottom.Draws || Right.Draws;

    /// <summary>Whether a page of the section carries the border.</summary>
    /// <param name="isFirstOfSection">True for the section's first page.</param>
    public bool AppearsOn(bool isFirstOfSection) => Display switch
    {
        PageBorderDisplay.FirstPage => isFirstOfSection,
        PageBorderDisplay.NotFirstPage => !isFirstOfSection,
        _ => true,
    };

    /// <summary>
    /// Where the border's rectangle sits on a page of this size with this text area.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The border never moves the text</strong>, whichever edge it is measured from. That is
    /// not obvious and it is what <c>editeng::BorderDistanceFromWord</c>
    /// (<c>editeng/source/items/frmitems.cxx</c>:4143-4174) does: Writer keeps a page's margin as the
    /// distance to the <em>border box</em> and a separate distance from the box to the text, and both
    /// of Word's two spellings are converted so that margin + width + distance is the margin Word
    /// stated. Confirmed by rendering: the first line of a bordered fixture and of the same fixture
    /// with no border sit at the same point on 26.2.4.2, in both <c>offsetFrom</c> modes.
    /// </para>
    /// <para>
    /// So a side that draws has its box edge <see cref="PageBorderSide.Space"/> from the paper's edge
    /// when the section says <c>offsetFrom="page"</c>, and <c>margin − space − width</c> from it when
    /// the section says <c>offsetFrom="text"</c>. <strong>A side that draws nothing keeps the text
    /// area's own edge</strong> — measured on a fixture bordered left and right only, where 26.2.4.2
    /// runs the two verticals between the top and bottom margins rather than the length of the paper.
    /// </para>
    /// <para>
    /// The two clamps are that function's, and they fire when a border would land outside the paper
    /// or inside the text: a <c>page</c> border whose space and width together exceed the margin
    /// keeps the margin, and a <c>text</c> border whose margin is too small for it sits on the
    /// paper's edge.
    /// </para>
    /// </remarks>
    /// <param name="page">The sheet's size.</param>
    /// <param name="textArea">Where body text goes, which the border does not disturb.</param>
    public PlacedPageBorder? Place(DocSize page, DocRect textArea)
    {
        if (!Draws) return null;

        Length left = Inset(Left, textArea.X);
        Length top = Inset(Top, textArea.Y);
        Length right = Inset(Right, page.Width - textArea.Right);
        Length bottom = Inset(Bottom, page.Height - textArea.Bottom);

        DocRect outer = new(
            left, top,
            page.Width - left - right,
            page.Height - top - bottom);

        if (Shadow > Length.Zero)
        {
            outer = new DocRect(
                outer.X,
                outer.Y,
                Length.Max(Length.Zero, outer.Width - Shadow),
                Length.Max(Length.Zero, outer.Height - Shadow));
        }

        if (outer.Width <= Length.Zero || outer.Height <= Length.Zero) return null;

        return new PlacedPageBorder
        {
            Outer = outer,
            Top = Top,
            Left = Left,
            Bottom = Bottom,
            Right = Right,
            Shadow = Shadow,
        };
    }

    /// <summary>How far one side's box edge stands in from the paper's matching edge.</summary>
    private Length Inset(PageBorderSide side, Length textInset)
    {
        if (!side.Draws) return textInset;

        if (!OffsetFromText)
        {
            // The border would reach past the text's own edge, which Word can state and Writer cannot
            // draw: it keeps the margin instead.
            return side.Space + side.Width > textInset ? textInset : side.Space;
        }

        Length inset = textInset - side.Space - side.Width;
        return inset > Length.Zero ? inset : Length.Zero;
    }
}

/// <summary>
/// A page border placed on one page: the rectangle its sides stand on, and the sides.
/// </summary>
/// <remarks>
/// <para>
/// The rectangle is the border's <em>outer</em> extent and is already shrunk for the shadow, because
/// <strong>the shadow shrinks the box rather than hanging off the paper</strong> — measured off
/// 26.2.4.2's own content stream and the one thing about a page border that cannot be guessed from
/// the specification.
/// </para>
/// <para>
/// Separate from <see cref="PageBorders"/> so that a page carries a rectangle rather than the four
/// insets and a rule for combining them: the paginator is the only layer that knows both the sheet
/// and the text area, and it is also the only one that knows whether a page is the first of its
/// section.
/// </para>
/// </remarks>
public sealed record PlacedPageBorder
{
    /// <summary>The rectangle the four sides stand on, shadow already taken off.</summary>
    public required DocRect Outer { get; init; }

    /// <summary>The top side.</summary>
    public PageBorderSide Top { get; init; }

    /// <summary>The left side.</summary>
    public PageBorderSide Left { get; init; }

    /// <summary>The bottom side.</summary>
    public PageBorderSide Bottom { get; init; }

    /// <summary>The right side.</summary>
    public PageBorderSide Right { get; init; }

    /// <summary>The shadow's width, or zero when the border casts none.</summary>
    public Length Shadow { get; init; }
}

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
/// How a section starts relative to the one before it.
/// </summary>
/// <remarks>
/// Every format states this and none of them can leave it out, because it decides whether a section change
/// costs a page. Word's <c>w:type</c>, DOC's <c>sprmSBkc</c> and RTF's <c>\sbk*</c> all say the same four
/// things; ODF has no equivalent at all, because a change of master page in ODF <em>is</em> a page break —
/// there is no way to state a continuous one.
/// </remarks>
public enum SectionBreak
{
    /// <summary>The section starts on a new page, which is what every format defaults to.</summary>
    NextPage,

    /// <summary>
    /// The section continues on the same page.
    /// </summary>
    /// <remarks>
    /// The one break that is not a break. Used for a stretch of two-column text in the middle of a page,
    /// which is the whole reason it exists — the geometry changes and the page does not.
    /// </remarks>
    Continuous,

    /// <summary>The section starts on the next even-numbered page, leaving a blank one if need be.</summary>
    EvenPage,

    /// <summary>The section starts on the next odd-numbered page.</summary>
    OddPage,

    /// <summary>
    /// The section starts where the next column would, which is a new page only when there is no next column.
    /// </summary>
    /// <remarks>
    /// Meaningless without columns, which is why it reads as continuous in a single-column section and why it
    /// went unmodelled while layout laid every section out one column wide. In a multi-column section it is
    /// the break that fills the rest of a column with nothing — a heading forced to the top of column two.
    /// </remarks>
    NewColumn,
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

    /// <summary>
    /// US Letter portrait with one-inch margins: what a Word-family filter starts a section from
    /// before the document has stated anything.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is <em>not</em> the locale's paper. <c>SectionPropertyMap</c>'s constructor
    /// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:459-467) builds a
    /// <c>PaperInfo aLetter( PAPER_LETTER )</c> and inserts its width and height unconditionally,
    /// with all four margins at one inch (<c>:429-434</c>) — so every DOCX and RTF that states no
    /// page size gets Letter on a machine whose own default is A4. The locale-dependent
    /// <c>SvxPaperInfo::GetDefaultPaperSize()</c> governs a *new* Writer document
    /// (<c>lcl_DefaultPageFormat</c>, <c>sw/source/core/doc/docdesc.cxx</c>:80), which is a
    /// different question and a different answer.
    /// </para>
    /// <para>
    /// Measured on 24.2.7.2 in a container whose own default is A4, which is what separates the two:
    /// a <c>.txt</c> converted through Writer comes out A4, while an RTF with no <c>\paperw</c> and a
    /// DOCX with no <c>w:sectPr</c> both come out 612×792 pt with text starting at 72.1 pt. Reaching
    /// for A4 here put 13 corpus RTFs on the wrong paper and reflowed every line in them.
    /// </para>
    /// </remarks>
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

    /// <summary>
    /// True when the header's height is the one stated and content that outgrows it overflows rather than
    /// moving the body.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>SwFrameSize::Fixed</c> against <c>SwFrameSize::Minimum</c>, and it decides whether a
    /// running head that needs more room than the margin reserved for it pushes the body down.
    /// <c>SwHeadFootFrame::FormatPrt</c> grows a header only where the frame's size is a minimum and
    /// <c>SwHeaderAndFooterEatSpacingItem</c> is on, which is what the DOC and DOCX importers always set
    /// (<c>ww8par6.cxx</c>:652, <c>dmapper/PropertyMap.cxx</c>:1148) — so the Word formats leave this
    /// false and their headers grow.
    /// </para>
    /// <para>
    /// ODF says which it means: <c>svg:height</c> is a fixed height and <c>fo:min-height</c> is a floor.
    /// A page style stating the first gets a header that does not move the body, however much it holds.
    /// </para>
    /// </remarks>
    public bool HasFixedHeaderHeight { get; init; }

    /// <summary>True when the footer's height is fixed, as <see cref="HasFixedHeaderHeight"/> is.</summary>
    public bool HasFixedFooterHeight { get; init; }

    /// <summary>
    /// How far below the body's last possible line the footer's first line starts, or null when the
    /// footer sits on the bottom of the space reserved for it instead.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two formats genuinely disagree, and the disagreement is visible: rendering the same one-line
    /// footer through both puts it five points apart on an A4 page. Word's footer is <em>bottom-aligned</em>
    /// — its last line sits at <c>pageHeight - w:footer</c>, so a footer that grows a second line grows
    /// upwards into the page. ODF's is <em>top-aligned</em> below the body — its first line sits the footer
    /// style's own spacing below where body text stops, so a footer that grows a second line grows downwards
    /// and the bottom margin shrinks.
    /// </para>
    /// <para>
    /// Null rather than zero for the Word case, because zero is a meaningful ODF answer: a footer style with
    /// no spacing at all does start immediately below the body, which is not the same as hugging the bottom
    /// of the page.
    /// </para>
    /// </remarks>
    public Length? FooterOffset { get; init; }

    /// <summary>How many columns the text area is divided into; one for ordinary text.</summary>
    public int Columns { get; init; } = 1;

    /// <summary>The gap between columns, when there is more than one.</summary>
    public Length ColumnGap { get; init; }

    /// <summary>
    /// The columns' own widths, for a section that states them one by one instead of asking for equal
    /// ones; null for the ordinary case.
    /// </summary>
    /// <remarks>
    /// Every Word-family format has both spellings — DOC's <c>sprmSFEvenlySpaced</c> with a
    /// <c>sprmSDxaColWidth</c> per column, DOCX's <c>w:equalWidth="0"</c> with a <c>w:col</c> per column
    /// — and the stated widths are not a refinement of the even ones: a two-column section can be one
    /// third and two thirds, which is a hundred points out on an A4 measure. Kept beside
    /// <see cref="Columns"/> and <see cref="ColumnGap"/> rather than replacing them, because the even
    /// case is the overwhelmingly common one and is exactly described by the pair.
    /// </remarks>
    public ColumnRuler? ColumnRuler { get; init; }

    /// <summary>
    /// True when the section itself reads right to left, which reverses its columns.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A section's direction is a separate statement from its paragraphs' — OOXML's
    /// <c>w:sectPr/w:bidi</c>, ODF's <c>style:writing-mode</c> on the page layout, RTF's
    /// <c>\rtlsect</c>, WW8's <c>sprmSFBiDi</c> — and the one thing it changes that shows on a page
    /// is which column comes first. Measured: a two-column A4 page whose layout says <c>rl-tb</c>
    /// has its first column drawn at 319 pt, which is the right-hand one, and its margins stay
    /// where they were.
    /// </para>
    /// <para>
    /// It does <em>not</em> mirror the margins, and it does not decide a paragraph's direction: a
    /// paragraph takes that from its own properties, which is why a right-to-left section holding
    /// left-to-right paragraphs is an ordinary thing to meet.
    /// </para>
    /// </remarks>
    public bool IsRightToLeft { get; init; }

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

    /// <summary>The border drawn round the page, or null when the section declares none.</summary>
    /// <remarks>
    /// Page furniture rather than content — see <see cref="PageBorders"/>. Null rather than a
    /// no-sides value so that the overwhelming majority of sections, which declare nothing, cost
    /// one null check at drawing time and nothing at all in the layout.
    /// </remarks>
    public PageBorders? Borders { get; init; }

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
    /// <remarks>
    /// The even answer, which is the first column's for a section that states its widths one by one —
    /// see <see cref="ColumnWidthAt"/>, which is what a caller that knows the column should ask.
    /// </remarks>
    public Length ColumnWidth => ColumnWidthAt(0);

    /// <summary>The width of one particular column.</summary>
    /// <param name="column">The column, counted from zero at the leading edge.</param>
    public Length ColumnWidthAt(int column)
    {
        if (Columns <= 1) return TextWidth;

        if (Ruler is { } ruler) return ruler.WidthAt(column);

        Length available = TextWidth - (ColumnGap * (Columns - 1));
        return available > Length.Zero ? available / Columns : Length.Zero;
    }

    /// <summary>
    /// The stated column widths fitted to this page's measure, or null when the columns are even.
    /// </summary>
    /// <remarks>
    /// Fitted rather than taken as written, because the widths are stated against the measure the
    /// producer had in mind and a section's own margins may not agree with it. Writer does the same:
    /// <c>SwFormatCol</c> holds wish widths and <c>Calc</c> apportions the frame's real width between
    /// them. A ruler whose count disagrees with <see cref="Columns"/> is ignored rather than trusted,
    /// which is the lenient reading a malformed section needs.
    /// </remarks>
    public ColumnRuler? Ruler
        => ColumnRuler is { } stated && stated.Count == Columns && Columns > 1
            ? stated.FittedTo(TextWidth)
            : null;

    /// <summary>The text area's rectangle on the page.</summary>
    public DocRect TextArea =>
        new(Margins.Left + Gutter, Margins.Top, TextWidth, TextHeight);

    /// <summary>
    /// One column's rectangle on the page.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Columns are equal and evenly spaced unless the section says otherwise, which is what
    /// <see cref="Columns"/> and <see cref="ColumnGap"/> describe and what every format writes for an
    /// ordinary two-column stretch. A section that states a width per column carries a
    /// <see cref="ColumnRuler"/> instead and is laid out from that.
    /// </para>
    /// <para>
    /// Clamped to the columns that exist, so a caller asking for one past the end gets the last rather than
    /// a rectangle off the side of the sheet.
    /// </para>
    /// </remarks>
    /// <param name="column">The column, counted from zero at the leading edge.</param>
    public DocRect ColumnArea(int column)
    {
        int columns = Math.Max(1, Columns);
        int at = Math.Clamp(column, 0, columns - 1);

        // "Leading" rather than "left": a right-to-left section fills its rightmost column first,
        // which is the whole of what its direction does to a page. Measured against LibreOffice —
        // a two-column A4 page in rl-tb draws its first line at 319 pt.
        if (IsRightToLeft) at = columns - 1 - at;

        if (Ruler is { } ruler)
        {
            return new DocRect(
                Margins.Left + Gutter + ruler.OffsetOf(at), Margins.Top, ruler.WidthAt(at), TextHeight);
        }

        Length width = ColumnWidth;

        return new DocRect(
            Margins.Left + Gutter + ((width + ColumnGap) * at), Margins.Top, width, TextHeight);
    }

    /// <summary>
    /// The rectangle the header occupies.
    /// </summary>
    /// <remarks>
    /// <para>
    /// From the page's top edge by <see cref="HeaderDistance"/>, which is where every format but ODF
    /// states it — and the reader has already converted ODF's own spelling. Its height is
    /// <see cref="HeaderHeight"/> less the spacing that separates it from the body, because that spacing
    /// belongs to the gap rather than to the header: text drawn into it would sit closer to the body than
    /// the document asked for.
    /// </para>
    /// <para>
    /// The height can be nought, and a nought-height header is <em>not</em> the same as no header. A
    /// Word document setting <c>w:top</c> and <c>w:header</c> to the same value is ordinary — it reserves
    /// no room and lets the header grow into the body's — and Writer renders one: its header is a
    /// dynamic-height frame with a 1 mm floor, so it takes whatever room its content needs
    /// (<c>SectionPropertyMap::PrepareHeaderFooterProperties</c>, <c>PropertyMap.cxx:1148</c>). Returning
    /// an empty rectangle for that case dropped the header's every line and every frame anchored in it.
    /// </para>
    /// </remarks>
    public DocRect HeaderArea
        => new(Margins.Left + Gutter, HeaderDistance, TextWidth, Floor(Margins.Top - HeaderDistance));

    /// <summary>
    /// The rectangle the footer occupies.
    /// </summary>
    /// <remarks>
    /// Measured from the page's <em>bottom</em> edge, because that is how a footer's distance is stated —
    /// so it is the rectangle's <em>bottom</em> that <see cref="FooterDistance"/> fixes, and its top that
    /// gives way. It reaches up to where the body's text area ends, since the space between the two
    /// belongs to neither. Its height can be nought for the same reason a header's can; see
    /// <see cref="HeaderArea"/>.
    /// <para>
    /// Anchoring the top instead is wrong in exactly the case a form runs into: a document may set
    /// <c>w:footer</c> <em>larger</em> than <c>w:bottom</c> — <c>easa-form-1.docx</c> says 488 and 357 —
    /// which puts the body's bottom edge below the footer's, so a rectangle grown downwards from the body
    /// starts past the place the footer belongs and a nought-height one lands there outright. Measured
    /// against the reference on that document: LibreOffice ends the footer's text at 570.9 pt of a
    /// 595.35 pt page, which is the stated 488 twips from the edge, and anchoring the top drew it at
    /// 577.3.
    /// </para>
    /// </remarks>
    public DocRect FooterArea
    {
        get
        {
            Length bottom = Size.Height - FooterDistance;
            Length height = Floor(bottom - Margins.Top - TextHeight);
            return new DocRect(Margins.Left + Gutter, bottom - height, TextWidth, height);
        }
    }

    /// <summary>A length with its negative values clamped away, which a rectangle cannot carry.</summary>
    private static Length Floor(Length length) => length > Length.Zero ? length : Length.Zero;
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

    /// <summary>
    /// How the section starts relative to the one before it.
    /// </summary>
    /// <remarks>
    /// Meaningless for the first section of a document, which starts where the document starts however this
    /// reads. Every format's default is <see cref="SectionBreak.NextPage"/>, so that is this one's too.
    /// </remarks>
    public SectionBreak Break { get; init; }

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

    /// <summary>
    /// The sequence the section's page numbers are written in: <c>w:pgNumType/@w:fmt</c>,
    /// <c>sprmSNfcPgn</c>, ODF's <c>style:num-format</c> on the page layout.
    /// </summary>
    /// <remarks>
    /// <para>
    /// On the section rather than on the field, because that is where all four formats state it and
    /// because a running head is inherited across sections while its numbering is not: a document whose
    /// front matter is <c>lowerRoman</c> and whose body is <c>decimal</c> states one header once and
    /// changes the format at the section break. 21 of this corpus's 134 DOCX declare a
    /// <c>lowerRoman</c> section.
    /// </para>
    /// <para>
    /// Typed as <see cref="Layout.NoteNumberFormat"/> because a page number and a note citation are
    /// written in the same five sequences under four different attribute names, and two enums naming one
    /// set is how the two spellings drift apart.
    /// </para>
    /// </remarks>
    public Layout.NoteNumberFormat PageNumberFormat { get; init; } = Layout.NoteNumberFormat.Arabic;

    /// <summary>
    /// True when the section's columns are balanced — its content shared evenly between them rather than
    /// filling each in turn down to the bottom of the page.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Meaningless when <see cref="PageGeometry.Columns"/> is one. It is the section that decides, not the
    /// page: Writer models a balanced stretch as a <c>SwSectionFrame</c> whose height is searched for
    /// (<c>SwLayoutFrame::FormatWidthCols</c>) and an unbalanced one as a page style, which fills.
    /// </para>
    /// <para>
    /// Both Word readers decide it the same way and it is a property of the <em>next</em> section rather
    /// than of this one: a multi-column section balances when the section after it starts with a
    /// continuous break, and does not when a page break follows it or when it is the last. That is the
    /// literal shape of both importers — <c>if (aNext == aEnd || !aNext-&gt;IsContinuous())
    /// pRet-&gt;SetFormatAttr(SwFormatNoBalancedColumns(true))</c> (<c>ww8par.cxx</c>:4576), and
    /// <c>pPrevSection-&gt;DontBalanceTextColumns()</c> reached from the page-break branch of
    /// <c>SectionPropertyMap::CloseSectionGroup</c> (<c>dmapper/PropertyMap.cxx</c>:1919) with the
    /// last-section case handled in <c>ApplyColumnProperties</c>. Both are also switched off wholesale by
    /// the compatibility flag Word writes as <c>w:noColumnBalance</c> and <c>fNoColumnBalance</c>.
    /// </para>
    /// <para>
    /// Default false, so a reader that says nothing gets the filling behaviour that was here before.
    /// </para>
    /// </remarks>
    public bool BalancesColumns { get; init; }

    /// <summary>
    /// True when the section is a Writer <em>text section</em> rather than a page style, so its own
    /// horizontal geometry applies where it starts instead of on the next sheet.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A continuous section normally inherits the page style of the section above it, which is why a
    /// change of margins waits for the next page — see the note beside the break handling in
    /// <c>Paginator</c>. A multi-column stretch is the exception, and it is an exception because Writer
    /// models it as a different object: <c>SectionPropertyMap::CloseSectionGroup</c> says so in as many
    /// words — <em>"prefer setting column properties into a section, not a page style if at all
    /// possible"</em> (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:1905-1913) — and the WW8
    /// importer does the same, putting the <em>difference</em> between the Word section's margins and
    /// the page style's onto that section's own <c>SvxLRSpaceItem</c>:
    /// <c>nSectionLeft = rSection.GetPageLeft() - nPageLeft</c>
    /// (<c>sw/source/filter/ww8/ww8par6.cxx</c>:735-745). A <c>SwSectionFrame</c> begins where the flow
    /// reaches it, so both its columns and its indents take effect mid-page.
    /// </para>
    /// <para>
    /// ODF states that object directly, as a <c>text:section</c> whose section style carries
    /// <c>style:columns</c> and <c>fo:margin-left</c>/<c>fo:margin-right</c>; those margins are stated
    /// <em>relative to the page's</em>, which is why the reader adds them to the master's own before
    /// handing the geometry over. Default false, so nothing but a reader that says otherwise changes.
    /// </para>
    /// </remarks>
    public bool IsTextSection { get; init; }

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
        => PageFurnitureSlots.For(
            Headers, pageNumber, isFirstPageOfSection, HasDifferentFirstPage, HasDifferentEvenPages);

    /// <summary>The footer for a page, by the same rules as <see cref="HeaderFor"/>.</summary>
    public WritingBody? FooterFor(int pageNumber, bool isFirstPageOfSection)
        => PageFurnitureSlots.For(
            Footers, pageNumber, isFirstPageOfSection, HasDifferentFirstPage, HasDifferentEvenPages);
}

/// <summary>
/// Which of a section's furniture slots a page uses.
/// </summary>
/// <remarks>
/// Generic in what the slots hold, because two passes ask the same question of different things: the
/// extraction pass wants the header's content and the layout pass wants its paragraphs. The rule is worth
/// having in one place — the falling-back is the part that is easy to miss, since a section with only a
/// default header still has a header on its first page.
/// </remarks>
public static class PageFurnitureSlots
{
    /// <summary>The slot a page takes, or null when the section fills none of them.</summary>
    /// <param name="slots">What the section has, by slot.</param>
    /// <param name="pageNumber">The page's printed number, which is what decides even from odd.</param>
    /// <param name="isFirstPageOfSection">True for the section's own first page.</param>
    /// <param name="hasDifferentFirstPage">True when the section asked for a distinct first page.</param>
    /// <param name="hasDifferentEvenPages">True when the section distinguishes even pages.</param>
    public static T? For<T>(
        IReadOnlyDictionary<PageFurnitureSlot, T> slots,
        int pageNumber,
        bool isFirstPageOfSection,
        bool hasDifferentFirstPage,
        bool hasDifferentEvenPages)
        where T : class
    {
        ArgumentNullException.ThrowIfNull(slots);

        if (isFirstPageOfSection
            && hasDifferentFirstPage
            && slots.TryGetValue(PageFurnitureSlot.First, out T? first))
        {
            return first;
        }

        if (hasDifferentEvenPages
            && pageNumber % 2 == 0
            && slots.TryGetValue(PageFurnitureSlot.Even, out T? even))
        {
            return even;
        }

        return slots.GetValueOrDefault(PageFurnitureSlot.Default);
    }
}

/// <summary>
/// The widths a section states for its columns, one by one, and the gaps between them.
/// </summary>
/// <remarks>
/// <para>
/// Word's own model, and Writer's: a <c>SwFormatCol</c> holds a wish width per column plus the halves of
/// the gaps on either side of it (<c>SwWW8ImplReader::SetCols</c>, <c>sw/source/filter/ww8/ww8par6.cxx</c>
/// :449). What is stored here is the plain pair — the text widths and the gaps — because that is what both
/// formats write and what a rectangle needs; the halving is Writer's way of dividing the frame and is not
/// a fact about the document.
/// </para>
/// <para>
/// <see cref="Gaps"/> holds one fewer entry than <see cref="Widths"/>: a section states a spacing
/// <em>after</em> each column but the last, and Word writes nothing after the last one.
/// </para>
/// </remarks>
/// <param name="Widths">Each column's text width, in order from the leading edge.</param>
/// <param name="Gaps">The gap after each column but the last.</param>
public sealed record ColumnRuler(IReadOnlyList<Length> Widths, IReadOnlyList<Length> Gaps)
{
    /// <summary>How many columns the ruler describes.</summary>
    public int Count => Widths.Count;

    /// <summary>The widths and the gaps together, which is the measure the ruler was written against.</summary>
    public Length Total
    {
        get
        {
            Length total = Length.Zero;
            foreach (Length width in Widths) total += width;
            foreach (Length gap in Gaps) total += gap;
            return total;
        }
    }

    /// <summary>One column's width, clamped to the columns that exist.</summary>
    /// <param name="column">The column, counted from zero at the leading edge.</param>
    public Length WidthAt(int column)
        => Count == 0 ? Length.Zero : Widths[Math.Clamp(column, 0, Count - 1)];

    /// <summary>How far one column's leading edge sits from the text area's.</summary>
    /// <param name="column">The column, counted from zero at the leading edge.</param>
    public Length OffsetOf(int column)
    {
        int at = Math.Clamp(column, 0, Math.Max(0, Count - 1));
        Length offset = Length.Zero;

        for (int i = 0; i < at; i++)
        {
            offset += Widths[i];
            if (i < Gaps.Count) offset += Gaps[i];
        }

        return offset;
    }

    /// <summary>
    /// The same ruler stretched or squeezed so its columns and gaps fill a given measure.
    /// </summary>
    /// <remarks>
    /// The gaps keep their stated size and the widths take the difference in proportion, which is what
    /// Writer's own apportioning does: <c>SwFormatCol::Calc</c> distributes the frame's width between the
    /// columns' wish widths and leaves the fixed left and right insets — the gap halves — alone. A ruler
    /// that already sums to the measure, which is the case for every file that states its widths against
    /// the margins it also states, is returned unchanged.
    /// </remarks>
    /// <param name="measure">The width the columns and gaps have to fill.</param>
    public ColumnRuler FittedTo(Length measure)
    {
        Length gaps = Length.Zero;
        foreach (Length gap in Gaps) gaps += gap;

        Length stated = Total - gaps;
        Length available = measure - gaps;

        if (stated <= Length.Zero || available <= Length.Zero || stated == available) return this;

        List<Length> widths = new(Widths.Count);
        Length running = Length.Zero;

        for (int i = 0; i < Widths.Count; i++)
        {
            // The last column takes what is left rather than its own share, so rounding cannot leave the
            // columns a few EMUs short of the measure or a few over the right margin.
            Length width = i == Widths.Count - 1
                ? available - running
                : Length.FromEmu((long)(Widths[i].Emu * (double)available.Emu / stated.Emu));

            widths.Add(width > Length.Zero ? width : Length.Zero);
            running += widths[i];
        }

        return new ColumnRuler(widths, Gaps);
    }
}

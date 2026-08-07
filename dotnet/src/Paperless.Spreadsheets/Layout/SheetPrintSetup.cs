using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>A rectangular block of cells, in zero-based column and row indices.</summary>
/// <param name="FirstColumn">The leftmost column, inclusive.</param>
/// <param name="FirstRow">The topmost row, inclusive.</param>
/// <param name="LastColumn">The rightmost column, inclusive.</param>
/// <param name="LastRow">The bottom row, inclusive.</param>
public readonly record struct SheetRange(int FirstColumn, int FirstRow, int LastColumn, int LastRow)
{
    /// <summary>True when the range covers at least one cell.</summary>
    public bool IsValid => LastColumn >= FirstColumn && LastRow >= FirstRow;

    /// <summary>How many columns the range spans.</summary>
    public int ColumnCount => IsValid ? LastColumn - FirstColumn + 1 : 0;

    /// <summary>How many rows the range spans.</summary>
    public int RowCount => IsValid ? LastRow - FirstRow + 1 : 0;
}

/// <summary>
/// The order pages are numbered in when a sheet spills over both axes.
/// </summary>
/// <remarks>
/// The names are the printed order, not the iteration order, which is where this is easy to
/// get backwards: <see cref="DownThenAcross"/> walks a <em>column band</em> to the bottom of
/// the sheet before starting the next one, so its outer loop is over columns. Every format
/// defaults to it — Calc's <c>ATTR_PAGE_TOPDOWN</c> is true by default
/// (<c>sc/source/core/data/docpool.cxx:175</c>), SpreadsheetML's <c>pageOrder</c> is
/// <c>downThenOver</c>, and BIFF's <c>SETUP</c> flag is named for the exception rather than
/// the rule (<c>EXC_SETUP_INROWS</c>).
/// </remarks>
public enum PagePrintOrder
{
    /// <summary>Down the sheet first, then across: Calc's <c>bTopDown</c>.</summary>
    DownThenAcross,

    /// <summary>Across the sheet first, then down.</summary>
    AcrossThenDown,
}

/// <summary>
/// How a sheet is scaled onto its pages.
/// </summary>
/// <remarks>
/// The three modes are mutually exclusive and every format states exactly one of them, so
/// this is a discriminated choice rather than three independent numbers. Calc keeps them as
/// three separate pool items and resolves the precedence in
/// <c>ScPrintFunc::CalcZoom</c> (<c>sc/source/ui/view/printfun.cxx:2816</c>): fit-to-a-page-count
/// first, then fit-to-width-and-height, then a plain percentage.
/// </remarks>
public enum PrintScaleMode
{
    /// <summary>A fixed percentage — 100 for unscaled.</summary>
    Percentage,

    /// <summary>Shrink until the sheet fits a given number of pages across and down.</summary>
    FitToPages,

    /// <summary>Shrink until the whole sheet fits in a given total number of pages.</summary>
    FitToPageCount,
}

/// <summary>
/// A sheet's print setup: everything that decides where its pages fall.
/// </summary>
/// <remarks>
/// <para>
/// This is the whole of a spreadsheet's page geometry. A word-processing document states its
/// page size and then flows text into it; a spreadsheet has no pages at all until a print
/// setup is applied, so what would be a peripheral concern in Writer is the load-bearing
/// structure here.
/// </para>
/// <para>
/// The margins are stated the way Calc stores them rather than the way any one file format
/// does, because the three formats disagree and Calc is what the reference rendering comes
/// from. <see cref="TopMargin"/> is the distance from the paper's edge to the top of the
/// <em>header</em> when a header is switched on, and <see cref="HeaderHeight"/> is the whole
/// band the header occupies including the gap between it and the first row — so the first row
/// of cells sits at <c>TopMargin + HeaderHeight</c>. Calc says so in as many words at
/// <c>sc/source/filter/oox/pagesettings.cxx:970</c> ("In Calc, TopMargin property is distance
/// to top of header if enabled"), and the BIFF filter repeats it at
/// <c>sc/source/filter/excel/xipage.cxx:305</c>.
/// </para>
/// <para>
/// The consequence is worth stating because it is what makes the OOXML and BIFF readers
/// simple: in both of those formats the sum <c>TopMargin + HeaderHeight</c> comes back out as
/// the file's own <c>top</c> margin, whether or not it has a header. ODF is the format that
/// genuinely states the two separately, because ODF is Calc's own.
/// </para>
/// </remarks>
public sealed record SheetPrintSetup
{
    /// <summary>The defaults a sheet gets when its file states nothing.</summary>
    /// <remarks>
    /// A4 with two-centimetre margins and a half-centimetre header and footer band, which is
    /// what <c>ScStyleSheet::GetItemSet</c> puts in a fresh page style
    /// (<c>sc/source/core/data/stlsheet.cxx:148-215</c>). The paper size there is
    /// <c>SvxPaperInfo::GetDefaultPaperSize()</c> and therefore locale-dependent — Letter in
    /// an American locale — which Paperless cannot reproduce without locale infrastructure it
    /// does not have, so A4 is used and the difference is recorded in the module's TODO.
    /// </remarks>
    public static SheetPrintSetup Default { get; } = new();

    /// <summary>The paper size, already turned the way the page prints.</summary>
    /// <remarks>
    /// Already turned, rather than portrait dimensions plus a flag, because the two ways a
    /// format can state landscape have to be reconciled somewhere and the reader is where the
    /// evidence is: SpreadsheetML names a paper size from a table of portrait dimensions and
    /// swaps them when <c>orientation="landscape"</c>
    /// (<c>sc/source/filter/oox/pagesettings.cxx:945</c>), while ODF writes the final
    /// <c>fo:page-width</c> and <c>fo:page-height</c> and a <c>style:print-orientation</c>
    /// that merely agrees with them.
    /// </remarks>
    public DocSize PageSize { get; init; } =
        new(Length.FromTwips(11906), Length.FromTwips(16838));

    /// <summary>True when the document declared the page landscape.</summary>
    public bool IsLandscape { get; init; }

    /// <summary>The distance from the left paper edge to the first printed column.</summary>
    public Length LeftMargin { get; init; } = Length.FromTwips(1134);

    /// <summary>The distance from the right paper edge to the last printed column.</summary>
    public Length RightMargin { get; init; } = Length.FromTwips(1134);

    /// <summary>The distance from the top paper edge to the top of the header band.</summary>
    public Length TopMargin { get; init; } = Length.FromTwips(1134);

    /// <summary>The distance from the bottom paper edge to the bottom of the footer band.</summary>
    public Length BottomMargin { get; init; } = Length.FromTwips(1134);

    /// <summary>The header band's whole height, its gap to the first row included.</summary>
    /// <remarks>Zero when the sheet has no header, which is what switches it off.</remarks>
    public Length HeaderHeight { get; init; }

    /// <summary>The footer band's whole height, its gap to the last row included.</summary>
    public Length FooterHeight { get; init; }

    /// <summary>Which of the three scaling modes the sheet uses.</summary>
    public PrintScaleMode ScaleMode { get; init; } = PrintScaleMode.Percentage;

    /// <summary>The scale percentage, when <see cref="ScaleMode"/> is a percentage.</summary>
    public int ScalePercentage { get; init; } = 100;

    /// <summary>
    /// How many pages wide the sheet must fit, or zero for "as many as it takes".
    /// </summary>
    /// <remarks>
    /// Zero rather than null because zero is what both formats write for the unconstrained
    /// axis and what Calc's <c>ScPageScaleToItem</c> tests for
    /// (<c>printfun.cxx:2969</c>, <c>!nW || nPagesX &lt;= nW</c>). Fitting to width alone is
    /// the common case: it is what "fit all columns on one page" produces.
    /// </remarks>
    public int FitToPagesWide { get; init; }

    /// <summary>How many pages tall the sheet must fit, or zero for unconstrained.</summary>
    public int FitToPagesTall { get; init; }

    /// <summary>The total page budget, when <see cref="ScaleMode"/> is a page count.</summary>
    public int FitToPageCount { get; init; }

    /// <summary>The order the pages are numbered in.</summary>
    public PagePrintOrder PageOrder { get; init; }

    /// <summary>
    /// The declared print ranges, or empty when the sheet prints whatever it holds.
    /// </summary>
    /// <remarks>
    /// A list rather than one range because all three formats allow several, and several is
    /// not a curiosity: it is how a workbook prints two tables from one sheet on two pages.
    /// Calc paginates each in turn and concatenates the results
    /// (<c>ScPrintFunc::DoPrint</c>, <c>printfun.cxx:2712</c>).
    /// </remarks>
    public IReadOnlyList<SheetRange> PrintAreas { get; init; } = [];

    /// <summary>
    /// True when a sheet with no print range prints all of itself.
    /// </summary>
    /// <remarks>
    /// The default, and only ODF can say otherwise: <c>table:print="false"</c> on a
    /// <c>table:table</c> means "this sheet is not printed at all", which is a different thing
    /// from being hidden (<c>sc/source/filter/xml/xmltabi.cxx:175</c>). It only takes effect
    /// once <em>some</em> sheet in the workbook declares a print range, because that is what
    /// makes Calc consult the flag at all (<c>ScPrintFunc::InitParam</c>,
    /// <c>printfun.cxx:1027</c>).
    /// </remarks>
    public bool PrintsEntireSheet { get; init; } = true;

    /// <summary>The columns repeated at the left of every page, or null.</summary>
    public SheetRange? RepeatColumns { get; init; }

    /// <summary>The rows repeated at the top of every page, or null.</summary>
    public SheetRange? RepeatRows { get; init; }

    /// <summary>Columns at which the author forced a page break.</summary>
    /// <remarks>
    /// Each entry is the column that <em>starts</em> a page, which is how all three formats
    /// state it and how Calc stores it.
    /// </remarks>
    public IReadOnlyList<int> ManualColumnBreaks { get; init; } = [];

    /// <summary>Rows at which the author forced a page break.</summary>
    public IReadOnlyList<int> ManualRowBreaks { get; init; } = [];

    /// <summary>True when the row and column headings are printed down the side and along the top.</summary>
    /// <remarks>
    /// Pagination has to know: the headings take a centimetre of width and 12.8 points of
    /// height off every page before any cell is placed
    /// (<c>PRINT_HEADER_WIDTH</c>/<c>PRINT_HEADER_HEIGHT</c>,
    /// <c>sc/source/ui/inc/printfun.hxx:45</c>).
    /// </remarks>
    public bool PrintsHeadings { get; init; }

    /// <summary>True when the cell grid is printed.</summary>
    public bool PrintsGrid { get; init; }

    /// <summary>True when the sheet's cell notes are listed on pages of their own after it.</summary>
    /// <remarks>
    /// Excel's "Comments: at end of sheet". <c>ATTR_PAGE_NOTES</c> in Calc, from
    /// <c>EXC_SETUP_PRINTNOTES</c> in a BIFF <c>SETUP</c> record
    /// (<c>sc/source/filter/excel/xipage.cxx:84</c>, <c>:257</c>) and from
    /// <c>pageSetup/@cellComments</c> in SpreadsheetML. Pagination has to know: the pages are
    /// extra ones after the sheet's content, not decoration on it. See <see cref="SheetNotes"/>.
    /// </remarks>
    public bool PrintsNotes { get; init; }

    /// <summary>True when the printed block is centred horizontally on the page.</summary>
    public bool CentresHorizontally { get; init; }

    /// <summary>True when the printed block is centred vertically on the page.</summary>
    public bool CentresVertically { get; init; }

    /// <summary>The number the sheet's first page is numbered, or zero to continue.</summary>
    public int FirstPageNumber { get; init; }

    /// <summary>The header text, in the format's own field syntax, or null.</summary>
    /// <remarks>
    /// Kept as written as well as parsed, because the raw string is what the file said and a
    /// caller reporting on a workbook may want it. <see cref="Header"/> is what drawing uses.
    /// </remarks>
    public string? HeaderText { get; init; }

    /// <summary>The footer text, in the format's own field syntax, or null.</summary>
    public string? FooterText { get; init; }

    /// <summary>The header's three parts with their fields identified, or null when it has none.</summary>
    /// <remarks>
    /// Beside <see cref="HeaderText"/> rather than instead of it. Only the parsed form knows
    /// that <c>&amp;P</c> is a page number rather than two characters, and only it keeps the
    /// left, centre and right parts apart — which matters because Calc draws the three into the
    /// <em>same</em> rectangle with three different alignments rather than as one line.
    /// </remarks>
    public SheetHeaderFooter? Header { get; init; }

    /// <summary>The footer's three parts, or null when the sheet has no footer.</summary>
    public SheetHeaderFooter? Footer { get; init; }

    /// <summary>
    /// The gap between the header's text and the first printed row.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Inside <see cref="HeaderHeight"/> rather than added to it, which is the detail that costs
    /// a reader a quarter of a centimetre on every page if it is read the other way round.
    /// Calc's <c>aHdr.nHeight</c> is the whole band and <c>aHdr.nDistance</c> is a part of it:
    /// the text is laid out into a rectangle <c>nHeight - nDistance</c> tall at the very top of
    /// the band and the gap is what is left below it (<c>ScPrintFunc::PrintHF</c>,
    /// <c>sc/source/ui/view/printfun.cxx:1808</c>).
    /// </para>
    /// <para>
    /// Measured on <c>sheet-decor-ods.ods</c>, whose header declares 0.75 cm with a 0.25 cm
    /// margin below it: LibreOffice clips the header text to a rectangle 14.099 pt tall
    /// starting exactly at the top margin, and puts the top of the first printed row 21.11 pt
    /// below that margin. So the band is 0.75 cm in total and its text gets 0.5 cm of it.
    /// </para>
    /// </remarks>
    public Length HeaderGap { get; init; } = Length.FromTwips(142);

    /// <summary>The gap between the last printed row and the footer's text.</summary>
    public Length FooterGap { get; init; } = Length.FromTwips(142);

    /// <summary>The rectangle the header's text is drawn into.</summary>
    /// <remarks>
    /// Its own left and right margins, which are not the page's:
    /// <c>nLineStartX = aPageRect.Left() + rParam.nLeft</c> (<c>printfun.cxx:1800</c>). ODF
    /// states them on <c>style:header-footer-properties</c>, and a header that states none
    /// inherits the page style's — which is why <c>sheet-decor-ods.ods</c> puts its header
    /// between 113.4 pt and 481.85 pt on a page whose own margins are 56.7 pt: two centimetres
    /// of page margin plus two centimetres the header inherited.
    /// </remarks>
    public Length HeaderLeftMargin { get; init; }

    /// <summary>The header's own right margin, measured from the page's right margin inwards.</summary>
    public Length HeaderRightMargin { get; init; }

    /// <summary>The footer's own left margin.</summary>
    public Length FooterLeftMargin { get; init; }

    /// <summary>The footer's own right margin.</summary>
    public Length FooterRightMargin { get; init; }

    /// <summary>The rectangle the sheet's cells are printed into, headings included.</summary>
    /// <remarks>
    /// The paper less the margins and the two furniture bands. Pagination does not use this —
    /// it needs the same rectangle in whole twips at the print scale, which
    /// <c>SheetPagination</c> derives itself so that its arithmetic matches Calc's roundings —
    /// so this is for placing what a page holds rather than for deciding what it holds.
    /// </remarks>
    public DocRect PrintableArea
    {
        get
        {
            Length width = PageSize.Width - LeftMargin - RightMargin;
            Length height = PageSize.Height - TopMargin - BottomMargin - HeaderHeight - FooterHeight;

            return new DocRect(
                LeftMargin,
                TopMargin + HeaderHeight,
                width > Length.Zero ? width : Length.Zero,
                height > Length.Zero ? height : Length.Zero);
        }
    }
}

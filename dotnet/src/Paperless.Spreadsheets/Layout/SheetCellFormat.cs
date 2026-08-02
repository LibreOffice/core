using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// How a cell's text sits across its column.
/// </summary>
/// <remarks>
/// <see cref="General"/> is not a synonym for <see cref="Left"/>: it is resolved against the
/// cell's <em>value</em> at drawing time, so a number goes right and a string left. Calc calls it
/// <c>SvxCellHorJustify::Standard</c> and resolves it in <c>getAlignmentFromContext</c>
/// (<c>sc/source/ui/view/output2.cxx:1443</c>), which is why a spreadsheet cannot borrow a word
/// processor's alignment enumeration — there, the default is a constant.
/// </remarks>
public enum SheetHorizontalAlignment
{
    /// <summary>Not stated; decided from the cell's type when it is drawn.</summary>
    General,

    /// <summary>Against the left edge.</summary>
    Left,

    /// <summary>Centred between the edges.</summary>
    Centre,

    /// <summary>Against the right edge.</summary>
    Right,

    /// <summary>Stretched to both edges, which also forces wrapping.</summary>
    Justify,

    /// <summary>The text repeated until the column is full.</summary>
    Fill,

    /// <summary>Justified, the last line included.</summary>
    Distributed,
}

/// <summary>How a cell's text sits down its row.</summary>
/// <remarks>
/// <see cref="Standard"/> means bottom. Calc says so in one line —
/// <c>if (eAttrVerJust == SvxCellVerJustify::Standard) eAttrVerJust = SvxCellVerJustify::Bottom;</c>
/// (<c>output2.cxx:348</c>) — and it is worth keeping the two apart anyway, because the file
/// formats distinguish "not stated" from "bottom" and a round trip should too.
/// </remarks>
public enum SheetVerticalAlignment
{
    /// <summary>Not stated, which draws as <see cref="Bottom"/>.</summary>
    Standard,

    /// <summary>Against the top of the row.</summary>
    Top,

    /// <summary>Centred in the row.</summary>
    Centre,

    /// <summary>Against the bottom of the row.</summary>
    Bottom,

    /// <summary>Spread over the row's height, which also forces wrapping.</summary>
    Justify,

    /// <summary>Spread over the row's height, the last line included.</summary>
    Distributed,
}

/// <summary>
/// How a cell's text is underlined.
/// </summary>
/// <remarks>
/// The two accounting forms are Excel's own and Calc keeps them apart from the ordinary ones
/// (<c>XclFontData</c>'s <c>EXC_FONTUNDERL_SINGLE_ACC</c>, <c>sc/source/filter/inc/xlstyle.hxx</c>):
/// they run the full width of the cell rather than the width of the text, and Calc maps both to
/// its own <c>LINESTYLE_SINGLE</c> and <c>LINESTYLE_DOUBLE</c>. They are folded here for the same
/// reason — the distinction is a width rule that the drawing path does not implement — but kept
/// distinguishable in the file formats' own vocabulary would buy nothing else.
/// </remarks>
public enum SheetUnderline
{
    /// <summary>No line.</summary>
    None,

    /// <summary>One line under the text.</summary>
    SingleLine,

    /// <summary>Two lines under the text.</summary>
    DoubleLine,
}

/// <summary>
/// Everything about a cell that decides how its text is drawn.
/// </summary>
/// <remarks>
/// <para>
/// A resolved format rather than a reference into a style chain: the three readers resolve their
/// own chains — SpreadsheetML's <c>cellXfs</c> over <c>cellStyleXfs</c>, BIFF's cell <c>XF</c>
/// over its style <c>XF</c>, ODF's automatic style over its named parent — and hand layout the
/// answer. Pooling is by <em>index</em> in <see cref="SheetCellFormats"/>, which is where the
/// cost of a uniformly-formatted million-cell region is avoided; duplicating the resolution here
/// would only move the same work.
/// </para>
/// <para>
/// Deliberately holds nothing about fills, borders or protection. Those belong to whoever draws
/// the decoration around the text, and keeping them out means this record can be compared for
/// equality to pool formats without two cells that differ only in their border colour becoming
/// two text formats.
/// </para>
/// </remarks>
public sealed record SheetCellFormat
{
    /// <summary>
    /// What a cell that states nothing is drawn in.
    /// </summary>
    /// <remarks>
    /// Ten-point Liberation Sans, which is what Calc puts in a new spreadsheet on Linux
    /// (<c>DefaultFontType::LATIN_SPREADSHEET</c>) and therefore what every reference rendering
    /// of a document naming no font is measured in.
    /// </remarks>
    public static SheetCellFormat Default { get; } = new();

    /// <summary>The font family the cell asks for, or null for the sheet's default.</summary>
    public string? FontFamily { get; init; }

    /// <summary>The em size.</summary>
    public Length FontSize { get; init; } = Length.FromPoints(10);

    /// <summary>The weight, on the usual 100–900 scale; 400 is regular and 700 bold.</summary>
    public int FontWeight { get; init; } = 400;

    /// <summary>True when the face is italic or oblique.</summary>
    public bool IsItalic { get; init; }

    /// <summary>
    /// The line drawn under the text, if any.
    /// </summary>
    /// <remarks>
    /// Not a property of the face: the three formats state it beside the weight and the slant and
    /// it is drawn as a rule rather than shaped, so it belongs to the format rather than to the
    /// font resolution. Every workbook with a hyperlink has one, since the hyperlink style is an
    /// underlined blue font, and a column heading ruled off from its data is the other common case.
    /// </remarks>
    public SheetUnderline Underline { get; init; }

    /// <summary>True when a line is drawn through the text.</summary>
    /// <remarks>Read and drawn on the same path as <see cref="Underline"/>, from the same records.</remarks>
    public bool IsStruckThrough { get; init; }

    /// <summary>The colour the text is filled with.</summary>
    public Colour Colour { get; init; } = Colour.Black;

    /// <summary>How the text sits across the column.</summary>
    public SheetHorizontalAlignment Horizontal { get; init; } = SheetHorizontalAlignment.General;

    /// <summary>How it sits down the row.</summary>
    public SheetVerticalAlignment Vertical { get; init; } = SheetVerticalAlignment.Standard;

    /// <summary>True when the text breaks into lines rather than overflowing.</summary>
    public bool Wraps { get; init; }

    /// <summary>True when the font is scaled down until the text fits the column.</summary>
    public bool ShrinksToFit { get; init; }

    /// <summary>
    /// How far the text is pushed in from its start edge, over and above the cell margin.
    /// </summary>
    /// <remarks>
    /// A length rather than Excel's indent level, because ODF states one directly
    /// (<c>fo:margin-left</c>) and Excel states a count of levels. Both readers convert:
    /// one Excel level is 200 twips, which is what <c>XclImpCellAlign::FillToItemSet</c> writes
    /// and calls "1 Excel unit == 10 pt == 200 twips" (<c>xistyle.cxx:846</c>).
    /// </remarks>
    public Length Indent { get; init; }

    /// <summary>
    /// The angle the text is turned through, anticlockwise, in whole degrees from -90 to 90.
    /// </summary>
    /// <remarks>
    /// Calc's own range, and both Excel formats are folded into it on the way in: their 0–180
    /// scale means 0–90 anticlockwise and 91–180 clockwise by 1–90
    /// (<c>XclTools::GetScRotation</c>).
    /// </remarks>
    public int RotationDegrees { get; init; }

    /// <summary>True when the characters are stacked one under another rather than turned.</summary>
    public bool IsStacked { get; init; }

    /// <summary>True when the text is turned or stacked, so it cannot be drawn as a plain run.</summary>
    public bool IsRotated => IsStacked || RotationDegrees != 0;

    /// <summary>
    /// What kind of number format the cell's value is rendered through.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The <em>kind</em> rather than the code, because drawing asks two questions of it and
    /// neither needs the format itself. Is it <c>General</c>, which decides whether a too-narrow
    /// numeric cell shows <c>###</c> or re-renders itself shorter
    /// (<c>ScDrawStringsVars::SetTextToWidthOrHash</c>, <c>output2.cxx:635</c>)? And is it a plain
    /// number, which is the case that never wraps whatever the cell's wrap flag says
    /// (<c>output2.cxx:1834</c>, i#111387)? Keeping the kind also lets the ODF reader answer at
    /// all: ODF states a structured <c>number:*-style</c> rather than an Excel format code, so it
    /// has a kind and no code.
    /// </para>
    /// <para>
    /// Extraction reports the cell's full text whatever this says, which is a recorded decision
    /// rather than an oversight: <c>###</c> is a function of a column width, and extracted text
    /// has no column width.
    /// </para>
    /// </remarks>
    public Core.Numbers.NumberFormatKind NumberFormatKind { get; init; } = Core.Numbers.NumberFormatKind.General;

    /// <summary>True when the cell's number format is <c>General</c>, which is the default.</summary>
    public bool HasGeneralFormat => NumberFormatKind == Core.Numbers.NumberFormatKind.General;

    /// <summary>
    /// True when the cell's format is a plain number rather than a date, a time or text.
    /// </summary>
    /// <remarks><c>General</c> counts: it is <c>SvNumFormatType::NUMBER</c> in Calc too.</remarks>
    public bool HasPlainNumberFormat =>
        NumberFormatKind is Core.Numbers.NumberFormatKind.General or Core.Numbers.NumberFormatKind.Number;

    /// <summary>
    /// The cell's number format code, when the reader has one, for the single question the
    /// <see cref="NumberFormatKind"/> cannot answer.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>*c</c> fill directive expands to as many copies of its character as the column has
    /// room for, so an accounting cell's currency symbol sits against the left edge and its
    /// digits against the right. Where that expansion goes depends on the rendered text, so the
    /// value has to be put through the code again with
    /// <c>NumberFormatter.FillMarker</c> left in — which needs the code, not its kind.
    /// </para>
    /// <para>
    /// Null when the reader states no code: ODF writes a structured <c>number:*-style</c>, and
    /// a cell with no explicit format keeps <c>General</c>, which has no fill either way. Only
    /// <see cref="Core.Numbers.NumberFormatCode.HasFillDirective"/> is ever asked of it, so a
    /// null costs nothing.
    /// </para>
    /// </remarks>
    public Core.Numbers.NumberFormatCode? NumberFormat { get; init; }
}

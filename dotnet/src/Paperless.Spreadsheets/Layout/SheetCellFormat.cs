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
    /// The number format the cell's value was rendered through, or null when it is
    /// <c>General</c> or unknown.
    /// </summary>
    /// <remarks>
    /// Here rather than beside the value because only <em>drawing</em> needs it: a numeric cell
    /// too narrow for its text shows <c>###</c> when its format is anything but <c>General</c>,
    /// and re-renders itself shorter when it is <c>General</c>
    /// (<c>ScDrawStringsVars::SetTextToWidthOrHash</c>, <c>output2.cxx:635</c>). Extraction
    /// reports the full text either way, which is a recorded decision and not an oversight —
    /// <c>###</c> is a function of the column width, and extracted text has no column width.
    /// </remarks>
    public Numbers.NumberFormatCode? NumberFormat { get; init; }

    /// <summary>True when the cell's number format is <c>General</c>, which is the default.</summary>
    public bool HasGeneralFormat => NumberFormat is null || NumberFormat.IsGeneral;
}

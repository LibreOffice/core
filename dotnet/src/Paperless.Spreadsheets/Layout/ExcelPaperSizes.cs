using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// The paper sizes Excel names by index, in both SpreadsheetML and BIFF.
/// </summary>
/// <remarks>
/// <para>
/// Neither format writes a paper size: both write an index into a table Windows defines, and
/// the table is the only place the dimensions exist. LibreOffice carries its own copy at
/// <c>sc/source/filter/excel/xlpage.cxx:49-141</c>, and this is a port of the entries a real
/// file uses — the entries beyond them are envelope and rotated-Japanese sizes that no
/// spreadsheet in the corpus reaches, and an unknown index falls back rather than guessing.
/// </para>
/// <para>
/// Every entry is portrait. Landscape is a separate flag in both formats and the two are
/// combined by swapping, which is what <c>XclPageData::GetScPaperSize</c> does at the end of
/// the same file. An index of zero means "not stated", and the table's own zeroth entry is a
/// pair of zeroes for exactly that reason.
/// </para>
/// </remarks>
public static class ExcelPaperSizes
{
    /// <summary>Whether a table entry's dimensions are millimetres or inches.</summary>
    private enum Unit
    {
        Inch,
        Millimetre,
    }

    private static readonly (double Width, double Height, Unit Unit)[] Table =
    [
        /*  0 */ (0, 0, Unit.Inch),                  // undefined
        /*  1 */ (8.5, 11, Unit.Inch),               // Letter
        /*  2 */ (8.5, 11, Unit.Inch),               // Letter Small
        /*  3 */ (11, 17, Unit.Inch),                // Tabloid
        /*  4 */ (17, 11, Unit.Inch),                // Ledger
        /*  5 */ (8.5, 14, Unit.Inch),               // Legal
        /*  6 */ (5.5, 8.5, Unit.Inch),              // Statement
        /*  7 */ (7.25, 10.5, Unit.Inch),            // Executive
        /*  8 */ (297, 420, Unit.Millimetre),        // A3
        /*  9 */ (210, 297, Unit.Millimetre),        // A4
        /* 10 */ (210, 297, Unit.Millimetre),        // A4 Small
        /* 11 */ (148, 210, Unit.Millimetre),        // A5
        /* 12 */ (257, 364, Unit.Millimetre),        // B4 (JIS)
        /* 13 */ (182, 257, Unit.Millimetre),        // B5 (JIS)
        /* 14 */ (8.5, 13, Unit.Inch),               // Folio
        /* 15 */ (215, 275, Unit.Millimetre),        // Quarto
        /* 16 */ (10, 14, Unit.Inch),                // 10x14
        /* 17 */ (11, 17, Unit.Inch),                // 11x17
        /* 18 */ (8.5, 11, Unit.Inch),               // Note
    ];

    /// <summary>A4, which is the fallback for an index the table does not cover.</summary>
    /// <remarks>
    /// A4 rather than Letter, matching <see cref="SheetPrintSetup.Default"/> and for the same
    /// reason: LibreOffice's own fallback is <c>SvxPaperInfo::GetDefaultPaperSize()</c>, which
    /// is locale-dependent, and A4 is what it returns in every locale but the American ones.
    /// </remarks>
    public static (Length Width, Length Height) A4 { get; } =
        (Millimetres(210), Millimetres(297));

    /// <summary>The portrait dimensions of a paper index.</summary>
    /// <param name="index">The index the file states.</param>
    public static (Length Width, Length Height) Portrait(int index)
    {
        if (index < 0 || index >= Table.Length) return A4;

        (double width, double height, Unit unit) = Table[index];
        if (width <= 0 || height <= 0) return A4;

        return unit == Unit.Inch
            ? (Inches(width), Inches(height))
            : (Millimetres(width), Millimetres(height));
    }

    /// <summary>Converts to twips the way LibreOffice's own table does: rounded up at a half.</summary>
    private static Length Inches(double inches)
        => Length.FromTwips((long)((inches * 1440) + 0.5));

    private static Length Millimetres(double millimetres)
        => Length.FromTwips((long)((millimetres * 1440 / 25.4) + 0.5));
}

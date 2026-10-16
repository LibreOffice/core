using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;

namespace XRay.Office.Spreadsheets.Layout;

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
        => TryPortrait(index, out (Length Width, Length Height) size) ? size : A4;

    /// <summary>The portrait dimensions of a paper index, and whether the table knew it.</summary>
    /// <remarks>
    /// Callers need the difference, because an index the table does not cover is not merely a
    /// missing measurement — it also suppresses the landscape swap. See <see cref="Page"/>.
    /// </remarks>
    /// <param name="index">The index the file states.</param>
    /// <param name="size">The portrait dimensions, or <see cref="A4"/> when the index is unknown.</param>
    /// <returns>Whether <paramref name="index"/> named a size this table holds.</returns>
    public static bool TryPortrait(int index, out (Length Width, Length Height) size)
    {
        if (index < 0 || index >= Table.Length)
        {
            size = A4;
            return false;
        }

        (double width, double height, Unit unit) = Table[index];
        if (width <= 0 || height <= 0)
        {
            size = A4;
            return false;
        }

        size = unit == Unit.Inch
            ? (Inches(width), Inches(height))
            : (Millimetres(width), Millimetres(height));
        return true;
    }

    /// <summary>
    /// The page box a stated paper index and orientation imply.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An orientation only rotates a paper the application recognises.</strong> Where the
    /// index is one LibreOffice cannot resolve, it writes no size onto the page style at all and
    /// the locale default is left standing — in its own portrait orientation, with the file's
    /// <c>orientation</c> discarded along with the size. Applying the swap to the fallback instead
    /// turns every such sheet 90 degrees.
    /// </para>
    /// <para>
    /// Measured on the installed 26.2.4.2 rather than read out of the source tree, over
    /// <c>paperSize="0"</c>–<c>"135"</c> at <c>orientation="landscape"</c>: the indices it resolves
    /// swap (<c>9</c> gives 841.89 x 595.30, <c>8</c> gives 1190.55 x 841.89), and every index it
    /// does not — <c>0</c>, <c>48</c>, <c>49</c>, <c>71</c>–<c>74</c>, <c>77</c>, <c>84</c>–<c>87</c>
    /// and <c>91</c> upwards — renders 595.304 x 841.89, A4 portrait, despite asking for landscape.
    /// <c>ODs-February-2022-Airbus-Commercial-Aircraft.xlsx</c> states <c>paperSize="121"</c> on
    /// eight of its thirteen sheets and is 154 pages against 175 for exactly this reason.
    /// </para>
    /// </remarks>
    /// <param name="index">The paper index the file states.</param>
    /// <param name="landscape">Whether the file asks for landscape.</param>
    public static DocSize Page(int index, bool landscape)
    {
        if (!TryPortrait(index, out (Length Width, Length Height) size)) return Default;

        return landscape
            ? new DocSize(size.Height, size.Width)
            : new DocSize(size.Width, size.Height);
    }

    /// <summary>
    /// The page box to use when the file states no usable page setup at all.
    /// </summary>
    /// <remarks>
    /// Portrait, and deliberately not rotated by any orientation the file states, for the same
    /// reason as <see cref="Page"/>: the application's own paper is what stands. Measured —
    /// <c>usePrinterDefaults="1"</c> alongside <c>orientation="landscape"</c> renders A4 portrait
    /// even when the paper index is one LibreOffice resolves perfectly well, such as <c>8</c> or
    /// <c>9</c>.
    /// </remarks>
    public static DocSize Default { get; } = new(A4.Width, A4.Height);

    /// <summary>Converts to twips the way LibreOffice's own table does: rounded up at a half.</summary>
    private static Length Inches(double inches)
        => Length.FromTwips((long)((inches * 1440) + 0.5));

    private static Length Millimetres(double millimetres)
        => Length.FromTwips((long)((millimetres * 1440 / 25.4) + 0.5));
}

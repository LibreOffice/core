namespace Paperless.Spreadsheets.Numbers;

/// <summary>
/// The number formats a spreadsheet has without recording them.
/// </summary>
/// <remarks>
/// <para>
/// Format indices below 164 are built in: a file that formats a cell as a date usually just
/// says "format 14" and expects the reader to know what that means. A reader without this
/// table shows the serial number instead of the date, for the majority of dates in the
/// majority of workbooks.
/// </para>
/// <para>
/// The codes are LibreOffice's <c>spBuiltInFormats_DONTKNOW</c> table
/// (<c>sc/source/filter/excel/xlstyle.cxx:820-905</c>), which is the one it falls back to
/// when a file's language is unknown. That matters for the date formats specifically: index
/// 14 is <c>DD/MM/YYYY</c> here and <c>M/D/YYYY</c> in the US table, and which one a reader
/// shows depends on <em>its</em> locale rather than on anything in the file. Files written
/// by LibreOffice sidestep the question by writing an explicit FORMAT record for every
/// format they use, so this table is consulted mainly for files Excel wrote.
/// </para>
/// </remarks>
public static class BuiltInNumberFormats
{
    /// <summary>
    /// The first index a file may define for itself. Everything below is built in.
    /// </summary>
    /// <remarks>
    /// 164 in both BIFF5 and BIFF8 (<c>EXC_FORMAT_OFFSET5</c>/<c>EXC_FORMAT_OFFSET8</c>).
    /// Indices 82 to 163 are reserved and, per LibreOffice's own note, make Excel crash if a
    /// file uses them.
    /// </remarks>
    public const int FirstUserIndex = 164;

    private static readonly Dictionary<int, string> Codes = new()
    {
        [0] = "General",
        [1] = "0",
        [2] = "0.00",
        [3] = "#,##0",
        [4] = "#,##0.00",
        // 5 to 8 are currency formats, and a file that uses them writes them out itself.
        [9] = "0%",
        [10] = "0.00%",
        [11] = "0.00E+00",
        [12] = "# ?/?",
        [13] = "# ??/??",
        [14] = "DD/MM/YYYY",
        [15] = "DD-MMM-YY",
        [16] = "DD-MMM",
        [17] = "MMM-YY",
        [18] = "h:mm AM/PM",
        [19] = "h:mm:ss AM/PM",
        [20] = "hh:mm",
        [21] = "hh:mm:ss",
        [22] = "DD/MM/YYYY hh:mm",
        [37] = "#,##0;-#,##0",
        [38] = "#,##0;[RED]-#,##0",
        [39] = "#,##0.00;-#,##0.00",
        [40] = "#,##0.00;[RED]-#,##0.00",
        [45] = "mm:ss",
        [46] = "[h]:mm:ss",
        [47] = "mm:ss.0",
        [48] = "##0.0E+0",
        [49] = "@",
    };

    /// <summary>
    /// The indices 23 to 36 and 50 to 81 are international spellings of another built-in, and
    /// resolve to it.
    /// </summary>
    /// <remarks>
    /// Excel used these for the date and time formats of locales it shipped; LibreOffice maps
    /// them onto the base formats rather than reproducing each locale, and so does this.
    /// </remarks>
    private static readonly Dictionary<int, int> Aliases = new()
    {
        [23] = 0, [24] = 0, [25] = 0, [26] = 0,
        [27] = 14, [28] = 14, [29] = 14, [30] = 14, [31] = 14,
        [32] = 21, [33] = 21, [34] = 21, [35] = 21, [36] = 14,
        [50] = 14, [51] = 14, [52] = 14, [53] = 14, [54] = 14,
        [55] = 14, [56] = 14, [57] = 14, [58] = 14,
        [59] = 1, [60] = 2, [61] = 3, [62] = 4,
        [67] = 9, [68] = 10, [69] = 12, [70] = 13,
        [71] = 14, [72] = 14, [73] = 15, [74] = 16, [75] = 17,
        [76] = 20, [77] = 21, [78] = 22, [79] = 45, [80] = 46, [81] = 47,
    };

    /// <summary>The code for a built-in index, or null when the index is not built in.</summary>
    public static string? Code(int index)
    {
        if (Codes.TryGetValue(index, out string? code)) return code;
        if (Aliases.TryGetValue(index, out int target) && Codes.TryGetValue(target, out code)) return code;
        return null;
    }
}

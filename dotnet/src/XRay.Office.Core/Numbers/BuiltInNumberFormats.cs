namespace XRay.Office.Core.Numbers;

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
/// <strong>Which table is not decided by the file. It is decided by the locale of the
/// application doing the reading</strong> — <c>XclNumFmtBuffer</c>'s <c>meSysLang</c> is
/// <c>rRoot.GetSysLanguage()</c>, documented in <c>sc/source/filter/inc/xlstyle.hxx</c>:469 as
/// <em>"Current system language"</em>, and <c>InsertBuiltinFormats</c> walks from that language's
/// table up through its parents to <c>spBuiltInFormats_DONTKNOW</c>
/// (<c>sc/source/filter/excel/xlstyle.cxx</c>:1437-1470). The OOXML filter does the same thing
/// with the same shape of table and its own system locale
/// (<c>NumberFormatsBuffer::insertBuiltinFormats</c>,
/// <c>sc/source/filter/oox/numberformatsbuffer.cxx</c>:1919-1975).
/// </para>
/// <para>
/// So the codes below are the <em>en-US</em> row — <c>spBuiltInFormats_ENGLISH_US</c>
/// (<c>xlstyle.cxx</c>:937-953) over <c>spBuiltInFormats_ENGLISH</c> (:911-919) over
/// <c>spBuiltInFormats_DONTKNOW</c> (:820-905), which is what the reference binaries this tree is
/// calibrated against resolve to. This table used to be the <c>DONTKNOW</c> one, on the reading
/// that an unknown file language should fall back to it; that is the wrong axis, and it was
/// measurably wrong in three places. Measured on both installed binaries over a probe workbook
/// (<c>dotnet/probes/numfmt-r68/make-codes.py</c>, thirty cells, glyphs read out of the PDFs):
/// id 20 draws <c>2:20</c> and <c>0:00</c> where <c>DONTKNOW</c>'s <c>hh:mm</c> would draw
/// <c>02:20</c> and <c>00:00</c>; id 14 draws <c>8/21/2022</c> where <c>DD/MM/YYYY</c> would draw
/// <c>21/08/2022</c>; and ids 37-40 draw <c>(100)</c> and <c>(100.00)</c> where
/// <c>#,##0;-#,##0</c> would draw <c>-100</c>. 24.2.7.2 and 26.2.4.2 agree on all thirty.
/// </para>
/// <para>
/// The same table serves the OOXML readers through <see cref="Code"/>. It used to be duplicated
/// in <c>XlsxStyles</c> with the en-US answers while this one carried the <c>DONTKNOW</c> ones,
/// so the two disagreed about the same id in the same workbook depending on which reader asked;
/// there is now one table.
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

        // 5 to 8 are the currency formats, and 37 to 40 the same four with the symbol blank.
        // A BIFF file writes 5 to 8 out itself and the en-US BIFF table therefore leaves them
        // alone; the OOXML table states them, and stating them here is what a malformed file
        // that omits the record needs (numberformatsbuffer.cxx:295-320, :802).
        [5] = "$#,##0_);($#,##0)",
        [6] = "$#,##0_);[RED]($#,##0)",
        [7] = "$#,##0.00_);($#,##0.00)",
        [8] = "$#,##0.00_);[RED]($#,##0.00)",

        [9] = "0%",
        [10] = "0.00%",
        [11] = "0.00E+00",
        [12] = "# ?/?",
        [13] = "# ??/??",
        [14] = "M/D/YYYY",
        [15] = "D-MMM-YY",
        [16] = "D-MMM",
        [17] = "MMM-YY",
        [18] = "h:mm AM/PM",
        [19] = "h:mm:ss AM/PM",
        [20] = "h:mm",
        [21] = "h:mm:ss",
        [22] = "M/D/YYYY h:mm",

        [37] = "#,##0_);(#,##0)",
        [38] = "#,##0_);[RED](#,##0)",
        [39] = "#,##0.00_);(#,##0.00)",
        [40] = "#,##0.00_);[RED](#,##0.00)",

        // The accounting four. `*` reserves the fill and `??` holds the dash clear of the
        // column's decimal point — see NumberFormatter.FillMarker and the `?` placeholder.
        [41] = "_(* #,##0_);_(* (#,##0);_(* \"-\"_);_(@_)",
        [42] = "_($* #,##0_);_($* (#,##0);_($* \"-\"_);_(@_)",
        [43] = "_(* #,##0.00_);_(* (#,##0.00);_(* \"-\"??_);_(@_)",
        [44] = "_($* #,##0.00_);_($* (#,##0.00);_($* \"-\"??_);_(@_)",

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
    /// them onto the base formats rather than reproducing each locale, and so does this. The
    /// pairs are its own — <c>NUMFMT_REUSE</c> in <c>numberformatsbuffer.cxx</c>:466-524 and the
    /// matching <c>XclBuiltInFormat</c> reuse rows in <c>xlstyle.cxx</c>:845-902.
    /// </remarks>
    private static readonly Dictionary<int, int> Aliases = new()
    {
        [23] = 0, [24] = 0, [25] = 0, [26] = 0,
        [27] = 14, [28] = 14, [29] = 14, [30] = 14, [31] = 14,
        [32] = 21, [33] = 21, [34] = 21, [35] = 21, [36] = 14,
        [50] = 14, [51] = 14, [52] = 14, [53] = 14, [54] = 14,
        [55] = 14, [56] = 14, [57] = 14, [58] = 14,
        [59] = 1, [60] = 2, [61] = 3, [62] = 4,
        [63] = 5, [64] = 6, [65] = 7, [66] = 8,
        [67] = 9, [68] = 10, [69] = 12, [70] = 13,
        [71] = 14, [72] = 14, [73] = 15, [74] = 16, [75] = 17,
        [76] = 20, [77] = 21, [78] = 22, [79] = 45, [80] = 46, [81] = 47,
    };

    /// <summary>The code for a built-in index, or null when the index is not built in.</summary>
    /// <remarks>The OOXML answer; see <see cref="BiffCode"/> for the one BIFF difference.</remarks>
    public static string? Code(int index)
    {
        if (Codes.TryGetValue(index, out string? code)) return code;
        if (Aliases.TryGetValue(index, out int target) && Codes.TryGetValue(target, out code)) return code;
        return null;
    }

    /// <summary>
    /// The same, as a BIFF filter resolves it: ids 5-8 and 41-44 are not built in there.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The locale row is shared — that is the whole point of this file — but the two filters'
    /// tables do not cover the same <em>set</em> of ids. The BIFF tables state, in as many
    /// words, <c>// 5...8 contained in file</c> and <c>// 41...44 contained in file</c>
    /// (<c>sc/source/filter/excel/xlstyle.cxx</c>:826, :862) and carry no entry for either run;
    /// neither <c>spBuiltInFormats_ENGLISH</c> nor <c>spBuiltInFormats_ENGLISH_US</c> adds one,
    /// so a BIFF file that uses one of those eight without writing its own <c>FORMAT</c> record
    /// gets the standard format. The OOXML tables state all eight
    /// (<c>numberformatsbuffer.cxx</c>:294-320, :802), which is why <see cref="Code"/> does.
    /// </para>
    /// <para>
    /// 63-66 are a separate question and are built in on both sides: the en-US BIFF table names
    /// them outright (<c>xlstyle.cxx</c>:949-952) and the OOXML base table reuses 5-8 for them,
    /// with the same four codes.
    /// </para>
    /// <para>
    /// Corpus reach of the difference is nil — every BIFF workbook of the 947 that uses one of
    /// those eight writes its own <c>FORMAT</c> record for it — so this is reproducing a rule
    /// rather than closing a measured gap.
    /// </para>
    /// </remarks>
    public static string? BiffCode(int index)
        => index is (>= 5 and <= 8) or (>= 41 and <= 44) ? null : Code(index);
}

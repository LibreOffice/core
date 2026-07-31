using System.Xml.Linq;
using Paperless.Spreadsheets.Numbers;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// As much of <c>styles.xml</c> as extraction needs: the number format each cell format names.
/// </summary>
/// <remarks>
/// <para>
/// Only number formats are read, and only because a spreadsheet stores a date as a serial
/// number and a percentage as a fraction. Without resolving them a date cell extracts as
/// "46233", which is the file's truth and nobody's answer. Fonts, fills and borders are
/// deliberately left for rendering: extraction discards them.
/// </para>
/// <para>
/// Content and formatting stay apart. A cell records a style <em>index</em>, and the index is
/// resolved on demand rather than each cell being handed a copy of its format — which is what
/// makes a sheet with one uniformly-formatted million-cell region cheap.
/// </para>
/// </remarks>
public sealed class XlsxStyles
{
    private readonly Dictionary<int, string> _customCodes = [];
    private readonly List<int> _cellFormatIds = [];
    private readonly Dictionary<int, NumberFormatCode> _parsed = [];

    private XlsxStyles()
    {
    }

    /// <summary>Styles for a workbook with no styles part.</summary>
    public static XlsxStyles Empty { get; } = new();

    /// <summary>How many cell formats <c>cellXfs</c> declares.</summary>
    public int CellFormatCount => _cellFormatIds.Count;

    /// <summary>Reads a <c>styleSheet</c> root.</summary>
    public static XlsxStyles Read(XElement? root)
    {
        XlsxStyles styles = new();
        if (root is null) return styles;

        foreach (XElement format in Xlsx.Children(Xlsx.Child(root, "numFmts"), "numFmt"))
        {
            if (Xlsx.Integer(format, "numFmtId") is not { } id) continue;
            if (Xlsx.Attribute(format, "formatCode") is not { } code) continue;
            _ = styles._customCodes.TryAdd(id, code);
        }

        foreach (XElement xf in Xlsx.Children(Xlsx.Child(root, "cellXfs"), "xf"))
        {
            // An xf without numFmtId inherits nothing meaningful for extraction: 0 is General,
            // which is also what a cell with no style at all gets.
            styles._cellFormatIds.Add(Xlsx.Integer(xf, "numFmtId") ?? 0);
        }

        return styles;
    }

    /// <summary>
    /// The number format a cell's <c>s</c> attribute selects.
    /// </summary>
    /// <remarks>
    /// An index outside <c>cellXfs</c> falls back to <c>General</c> rather than throwing: a
    /// style index that does not resolve is a broken file, not an unreadable one.
    /// </remarks>
    public NumberFormatCode FormatFor(int? styleIndex)
    {
        if (styleIndex is not { } index || index < 0 || index >= _cellFormatIds.Count)
            return NumberFormatCode.General;

        return FormatForId(_cellFormatIds[index]);
    }

    /// <summary>The format code a number-format id names, custom or built in.</summary>
    public NumberFormatCode FormatForId(int numberFormatId)
    {
        if (_parsed.TryGetValue(numberFormatId, out NumberFormatCode? cached)) return cached;

        string? code = _customCodes.TryGetValue(numberFormatId, out string? custom)
            ? custom
            : BuiltinCode(numberFormatId);

        NumberFormatCode parsed = code is null
            ? NumberFormatCode.General
            : NumberFormatCode.Parse(code);
        _parsed[numberFormatId] = parsed;
        return parsed;
    }

    /// <summary>
    /// The format codes ids 0–49 stand for when the file does not spell them out.
    /// </summary>
    /// <remarks>
    /// <para>
    /// These are implicit: a file may use id 14 without declaring a <c>numFmt</c> for it, so a
    /// reader that only honours the declared ones shows every date as a serial number.
    /// </para>
    /// <para>
    /// The date, time and currency entries are genuinely locale-dependent — LibreOffice keeps a
    /// table per locale in <c>sc/source/filter/oox/numberformatsbuffer.cxx:436</c> and picks by
    /// the workbook's locale — and the codes here are its <c>en_US</c> row
    /// (<c>numberformatsbuffer.cxx:798</c>). A German workbook using id 14 therefore extracts
    /// its dates as <c>M/D/YYYY</c> rather than <c>DD.MM.YYYY</c>. Fixing that needs locale
    /// infrastructure Paperless does not yet have; it is recorded in this library's TODO.
    /// </para>
    /// </remarks>
    private static string? BuiltinCode(int id) => id switch
    {
        0 => "General",
        1 => "0",
        2 => "0.00",
        3 => "#,##0",
        4 => "#,##0.00",
        5 => "$#,##0_);($#,##0)",
        6 => "$#,##0_);[RED]($#,##0)",
        7 => "$#,##0.00_);($#,##0.00)",
        8 => "$#,##0.00_);[RED]($#,##0.00)",
        9 => "0%",
        10 => "0.00%",
        11 => "0.00E+00",
        12 => "# ?/?",
        13 => "# ??/??",
        14 => "M/D/YYYY",
        15 => "D-MMM-YY",
        16 => "D-MMM",
        17 => "MMM-YY",
        18 => "h:mm AM/PM",
        19 => "h:mm:ss AM/PM",
        20 => "h:mm",
        21 => "h:mm:ss",
        22 => "M/D/YYYY h:mm",

        // 23..36 and 50..81 are "international" aliases that reuse an earlier entry.
        23 or 24 or 25 or 26 => "General",
        27 or 28 or 29 or 30 or 31 or 36 => "M/D/YYYY",
        32 or 33 or 34 or 35 => "h:mm:ss",

        37 => "#,##0_);(#,##0)",
        38 => "#,##0_);[RED](#,##0)",
        39 => "#,##0.00_);(#,##0.00)",
        40 => "#,##0.00_);[RED](#,##0.00)",
        41 => "_(* #,##0_);_(* (#,##0);_(* \"-\"_);_(@_)",
        42 => "_($* #,##0_);_($* (#,##0);_($* \"-\"_);_(@_)",
        43 => "_(* #,##0.00_);_(* (#,##0.00);_(* \"-\"??_);_(@_)",
        44 => "_($* #,##0.00_);_($* (#,##0.00);_($* \"-\"??_);_(@_)",
        45 => "mm:ss",
        46 => "[h]:mm:ss",
        47 => "mm:ss.0",
        48 => "##0.0E+0",
        49 => "@",

        >= 50 and <= 58 => "M/D/YYYY",
        59 => "0",
        60 => "0.00",
        61 => "#,##0",
        62 => "#,##0.00",
        67 => "0%",
        68 => "0.00%",
        69 => "# ?/?",
        70 => "# ??/??",
        71 or 72 => "M/D/YYYY",
        73 => "D-MMM-YY",
        74 => "D-MMM",
        75 => "MMM-YY",
        76 => "h:mm",
        77 => "h:mm:ss",
        78 => "M/D/YYYY h:mm",
        79 => "mm:ss",
        80 => "[h]:mm:ss",
        81 => "mm:ss.0",

        _ => null,
    };
}

using System.Xml.Linq;
using XRay.Office.Core.Numbers;

namespace XRay.Office.Spreadsheets.Ooxml;

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
    private int _defaultFormatId;

    private XlsxStyles()
    {
    }

    /// <summary>Styles for a workbook with no styles part.</summary>
    public static XlsxStyles Empty { get; } = new();

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
            // An xf without numFmtId names General, which is also the schema's default.
            styles._cellFormatIds.Add(Xlsx.Integer(xf, "numFmtId") ?? 0);
        }

        styles._defaultFormatId = DefaultFormatId(root);
        return styles;
    }

    /// <summary>
    /// The number format a cell that states no <c>s</c> takes: the Default cell style's.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>It is not <c>cellXfs[0]</c>, and the difference is measurable.</strong> A cell
    /// element states its format as <c>@s</c>, and LibreOffice reads an absent one as
    /// <em>no XF at all</em> — <c>rAttribs.getInteger(XML_s, -1)</c>
    /// (<c>sc/source/filter/oox/sheetdatacontext.cxx</c>:371), after which
    /// <c>SheetDataBuffer::setCellFormat</c> returns immediately on a negative id
    /// (<c>sheetdatabuffer.cxx</c>:721). What the cell then shows is whatever the sheet already
    /// carries: the column's default pattern, the row's, or the document's Default cell style,
    /// which is the <c>cellStyleXfs</c> entry the <c>Normal</c> <c>cellStyle</c> names.
    /// </para>
    /// <para>
    /// Measured on a probe workbook whose <c>cellXfs[0]</c> and <c>cellStyleXfs[0]</c> carry
    /// different formats (<c>dotnet/probes/numfmt-r68/make-default.py</c>): both 26.2.4.2 and
    /// 24.2.7.2 draw the <em>cellStyleXfs</em> one for a cell with no <c>s</c> and the
    /// <em>cellXfs</em> one for a cell that states <c>s="0"</c>. Every workbook in the corpus
    /// happens to give the two the same id, so no corpus document can tell them apart — which is
    /// exactly why the rule had to be probed rather than inferred.
    /// </para>
    /// </remarks>
    private static int DefaultFormatId(XElement root)
    {
        List<int> styleFormatIds = [];
        foreach (XElement xf in Xlsx.Children(Xlsx.Child(root, "cellStyleXfs"), "xf"))
            styleFormatIds.Add(Xlsx.Integer(xf, "numFmtId") ?? 0);

        if (styleFormatIds.Count == 0) return 0;

        // `builtinId="0"` is the Normal style, which is the document's Default cell style; a
        // workbook that names none falls back to the first entry, which is where every producer
        // writes it.
        int index = 0;
        foreach (XElement style in Xlsx.Children(Xlsx.Child(root, "cellStyles"), "cellStyle"))
        {
            if (Xlsx.Integer(style, "builtinId") != 0) continue;
            if (Xlsx.Integer(style, "xfId") is { } xfId) index = xfId;
            break;
        }

        return index >= 0 && index < styleFormatIds.Count ? styleFormatIds[index] : 0;
    }

    /// <summary>
    /// The same table built from a binary styles part rather than from XML.
    /// </summary>
    /// <remarks>
    /// XLSB's <c>styles.bin</c> states exactly what <c>styles.xml</c> states — a <c>NUMFMT</c>
    /// record is an id and a code, an <c>XF</c> inside <c>CELLXFS</c> is a cell format naming a
    /// number-format id — so the resolution, the parse cache and above all the <em>built-in
    /// table</em> are shared. That last point is why this exists rather than a second class:
    /// LibreOffice reads both formats through one <c>NumberFormatsBuffer</c>, so ids 0–49 mean
    /// the same thing in both, and the separate table this library keeps for BIFF8 is separate
    /// because LibreOffice's BIFF filter is a different filter — not because the file is binary.
    /// </remarks>
    /// <param name="customCodes">The codes <c>NUMFMT</c> records declared, by id.</param>
    /// <param name="cellFormatIds">The number-format id of each <c>CELLXFS</c> entry, in order.</param>
    internal static XlsxStyles FromRecords(
        IEnumerable<KeyValuePair<int, string>> customCodes, IEnumerable<int> cellFormatIds)
    {
        XlsxStyles styles = new();
        foreach ((int id, string code) in customCodes) _ = styles._customCodes.TryAdd(id, code);
        styles._cellFormatIds.AddRange(cellFormatIds);
        return styles;
    }

    /// <summary>
    /// The number format a cell's <c>s</c> attribute selects.
    /// </summary>
    /// <remarks>
    /// A cell that states no <c>s</c> — and one whose <c>s</c> is outside <c>cellXfs</c>, which
    /// is a broken file rather than an unreadable one — takes the Default cell style. See
    /// <see cref="Default"/>.
    /// </remarks>
    public NumberFormatCode FormatFor(int? styleIndex)
    {
        if (styleIndex is not { } index || index < 0 || index >= _cellFormatIds.Count)
            return Default;

        return FormatForId(_cellFormatIds[index]);
    }

    /// <summary>
    /// The format a cell takes when neither it, its row nor its column states one.
    /// </summary>
    /// <remarks>See <c>DefaultFormatId</c> for why this is not <c>cellXfs[0]</c>.</remarks>
    public NumberFormatCode Default => FormatForId(_defaultFormatId);

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
    /// The format codes ids 0–81 stand for when the file does not spell them out.
    /// </summary>
    /// <remarks>
    /// <see cref="BuiltInNumberFormats"/> holds the table, shared with the BIFF reader. It used
    /// to be duplicated here with a different answer for ids 14, 20, 22 and 37–40, so a
    /// workbook's built-in format depended on which of the two readers opened it.
    /// </remarks>
    private static string? BuiltinCode(int id) => BuiltInNumberFormats.Code(id);
}

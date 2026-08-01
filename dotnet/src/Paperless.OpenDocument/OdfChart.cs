using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Extraction;

namespace Paperless.OpenDocument;

/// <summary>
/// Reads an ODF chart — <c>chart:chart</c> — into the content tree.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A chart is a sub-document, not an element.</strong> ODF puts it in its own
/// <c>office:document</c> with its own mime type,
/// <c>application/vnd.oasis.opendocument.chart</c>, which a packaged file stores in a directory
/// of its own (<c>Object 1/content.xml</c>) and a flat file writes inline inside the
/// <c>draw:object</c>. <see cref="Locate"/> hides that difference; everything after it is the
/// same walk.
/// </para>
/// <para>
/// <strong>The numbers come from the local table, which is the cache.</strong> A series names
/// its data with <c>chart:values-cell-range-address</c>, which on a spreadsheet points at real
/// sheet cells — <c>Revenue.B2:Revenue.B5</c> — and a chart in a text document or a deck points
/// at <c>local-table</c>. Either way the sub-document carries a
/// <c>table:table table:name="local-table"</c> holding what the application last computed, and
/// LibreOffice loads *that*: <c>SchXMLTableContext</c> builds the whole internal data provider
/// from the parsed rows and only then, if every range address resolved, swaps in a live provider
/// (<c>xmloff/source/chart/SchXMLTableContext.cxx:85-150</c>, <c>SchXMLChartContext::endFastElement</c>).
/// So this is the same rule the DrawingML reader follows for <c>c:numCache</c>, reached from the
/// other end: prefer the cache, and never open a second document to improve on it.
/// </para>
/// <para>
/// The section produced is deliberately the same shape
/// <c>Paperless.Ooxml.DrawingML.DrawingChart</c> produces — the title as both
/// <see cref="ContentSection.Name"/> and the first paragraph, then a paragraph per titled axis,
/// then one <see cref="ContentTable"/> — so that a caller indexing a mixed corpus never branches
/// on which family a chart came from.
/// </para>
/// </remarks>
public static class OdfChart
{
    /// <summary>The media type an ODF chart sub-document declares.</summary>
    public const string MediaType = "application/vnd.oasis.opendocument.chart";

    /// <summary>How many repeats of one row or column of the local table are walked into.</summary>
    /// <remarks>
    /// The same cap the format readers use. A local table is a handful of rows in every file
    /// measured, but <c>table:number-columns-repeated</c> is an unbounded attribute in an
    /// untrusted document and it sizes a loop.
    /// </remarks>
    private const int MaxRepeat = 1024;

    /// <summary>
    /// Finds the <c>chart:chart</c> a <c>draw:object</c> refers to, or null when it refers to
    /// something that is not a chart.
    /// </summary>
    /// <param name="drawObject">The <c>draw:object</c> element.</param>
    /// <param name="file">The document, for reaching a packaged sub-document's part.</param>
    /// <remarks>
    /// A <c>draw:object</c> is the wrapper for every embedded ODF document — a chart, a formula,
    /// a nested spreadsheet — so "is this a chart" can only be answered by looking inside. Doing
    /// that rather than trusting <c>META-INF/manifest.xml</c>'s media type for the directory is
    /// also the lenient reading: a manifest entry that is missing or wrong is a real thing in
    /// files written by tools, and the sub-document itself is not.
    /// </remarks>
    public static XElement? Locate(XElement drawObject, OdfFile file)
    {
        ArgumentNullException.ThrowIfNull(drawObject);
        ArgumentNullException.ThrowIfNull(file);

        // Flat XML: the whole sub-document sits inside the draw:object.
        foreach (XElement child in drawObject.Elements())
        {
            if (child.Name.NamespaceName != OdfNamespaces.Office) continue;
            if (child.Name.LocalName is not ("document" or "document-content")) continue;
            if (Chart(child) is { } inline) return inline;
        }

        string? href = drawObject.Attribute(XName.Get("href", OdfNamespaces.XLink))?.Value;
        if (string.IsNullOrWhiteSpace(href)) return null;

        string target = href.Trim();

        // Anything with a scheme points outside the package, and Paperless never fetches those.
        if (target.Contains("://", StringComparison.Ordinal)) return null;
        if (target.StartsWith("./", StringComparison.Ordinal)) target = target[2..];
        target = target.TrimEnd('/');
        if (target.Length == 0) return null;

        using Stream? content = file.OpenPart(target + "/content.xml");
        if (content is null) return null;

        return Chart(OdfXml.TryLoad(content, out _)?.Root);
    }

    /// <summary>
    /// Reads a chart.
    /// </summary>
    /// <param name="chart">The <c>chart:chart</c> element.</param>
    /// <returns>
    /// The chart as a section, or null when it states no title, no axis title and no data — in
    /// which case the caller should go on recording the object as a graphic, because there is
    /// nothing better to say about it.
    /// </returns>
    public static ContentSection? Read(XElement chart)
    {
        ArgumentNullException.ThrowIfNull(chart);

        string? title = TitleText(Child(chart, OdfNamespaces.Chart, "title"));
        string? subtitle = TitleText(Child(chart, OdfNamespaces.Chart, "subtitle"));

        List<string> axisTitles = [];
        XElement? plotArea = Child(chart, OdfNamespaces.Chart, "plot-area");
        foreach (XElement axis in Children(plotArea, OdfNamespaces.Chart, "axis"))
        {
            if (TitleText(Child(axis, OdfNamespaces.Chart, "title")) is { Length: > 0 } text)
                axisTitles.Add(text);
        }

        ContentTable? table = ReadLocalTable(chart);

        if (title is null && subtitle is null && axisTitles.Count == 0 && table is null) return null;

        ContentSection section = new() { Kind = SectionKind.Frame, Name = title };

        // Document order: chart:title, then chart:subtitle, then the plot area's axes. A
        // subtitle has no DrawingML counterpart at all — c:chart has one title element — so a
        // chart round-tripped through PPTX loses it, which is the format's doing and not this
        // reader's.
        if (title is not null) AddParagraph(section, title);
        if (subtitle is not null) AddParagraph(section, subtitle);
        foreach (string axisTitle in axisTitles) AddParagraph(section, axisTitle);
        if (table is not null) section.Children.Add(table);

        return section;
    }

    /// <summary>The <c>chart:chart</c> inside an <c>office:document</c> root, or null.</summary>
    private static XElement? Chart(XElement? root)
        => root?.Element(XName.Get("body", OdfNamespaces.Office))
               ?.Element(XName.Get("chart", OdfNamespaces.Office))
               ?.Element(XName.Get("chart", OdfNamespaces.Chart));

    /// <summary>
    /// A title's text: its <c>text:p</c> children, joined by newlines.
    /// </summary>
    /// <remarks>
    /// ODF states a chart title as paragraphs and nothing else — there is no linked-cell form of
    /// it, so unlike DrawingML there is no precedence to resolve. LibreOffice's own default
    /// title is invented at the point of display rather than written to the file, and is not
    /// reproduced here for the same reason it is not reproduced on the OOXML side: a chart with
    /// no <c>chart:title</c> has no title.
    /// </remarks>
    private static string? TitleText(XElement? title)
    {
        if (title is null) return null;

        List<string> lines = [];
        foreach (XElement paragraph in title.Elements(XName.Get("p", OdfNamespaces.Text)))
        {
            if (paragraph.Value is { Length: > 0 } text) lines.Add(text);
        }

        return lines.Count == 0 ? null : string.Join('\n', lines);
    }

    /// <summary>
    /// Reads <c>table:table</c> — the chart's own copy of its data.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The header row holds the series names and the header column the category labels, which is
    /// the layout the DrawingML reader assembles by hand from <c>c:ser/c:tx</c> and
    /// <c>c:cat</c>. ODF states it directly, and states whether it means it:
    /// <c>table:table-header-rows</c> and <c>table:table-header-columns</c> are what
    /// <c>SchXMLTableContext</c> reads to decide the offsets it applies the table at
    /// (<c>SchXMLTableContext.cxx:95-96</c>, <c>:206-218</c>). The header <em>row</em> is
    /// honoured, so a local table without one is reported as plain data with no header. The
    /// header <em>column</em> is not recorded at all: <see cref="ContentTable"/> has
    /// <see cref="ContentTable.HeaderRowCount"/> and no column counterpart, and the layout it
    /// would state — the first column is labels — is the one this table already has.
    /// </para>
    /// <para>
    /// <strong>A header cell can hold a <c>draw:g</c>, and it is not text.</strong> LibreOffice
    /// writes the cell's originating range into the local table as
    /// <c>&lt;draw:g&gt;&lt;svg:desc&gt;Feuille1.A1:Feuille1.A1&lt;/svg:desc&gt;&lt;/draw:g&gt;</c>
    /// beside the label. Reading a cell with <c>XElement.Value</c> would splice that address into
    /// the series name; only <c>text:p</c> children are text.
    /// </para>
    /// </remarks>
    private static ContentTable? ReadLocalTable(XElement chart)
    {
        XElement? table = null;
        foreach (XElement candidate in Children(chart, OdfNamespaces.Table, "table"))
        {
            table = candidate;
            break;
        }

        if (table is null) return null;

        bool headerRow = false;
        List<ContentTableRow> rows = [];
        int widest = 0;

        foreach ((XElement element, bool isHeader) in Rows(table))
        {
            int repeat = Math.Min(Repeat(element, "number-rows-repeated"), MaxRepeat);
            for (int copy = 0; copy < repeat; copy++)
            {
                int index = rows.Count;
                if (isHeader && index == 0) headerRow = true;

                ContentTableRow row = new() { Index = index };
                int column = 0;

                foreach (XElement cell in element.Elements())
                {
                    if (cell.Name.NamespaceName != OdfNamespaces.Table) continue;
                    if (cell.Name.LocalName is not ("table-cell" or "covered-table-cell")) continue;

                    int span = Math.Min(Repeat(cell, "number-columns-repeated"), MaxRepeat);
                    for (int at = 0; at < span; at++)
                    {
                        row.Children.Add(ReadCell(cell, index, column));
                        column++;
                    }
                }

                widest = Math.Max(widest, column);
                rows.Add(row);
            }
        }

        if (rows.Count == 0) return null;

        // A trailing run of wholly empty rows is the local table's padding rather than data:
        // a chart over four categories is written with four rows, but one whose range was
        // shortened keeps the rows it had, empty.
        while (rows.Count > 0 && IsEmpty(rows[^1])) rows.RemoveAt(rows.Count - 1);
        if (rows.Count == 0) return null;

        ContentTable result = new()
        {
            ColumnCount = widest,
            HeaderRowCount = headerRow ? 1 : 0,
        };
        foreach (ContentTableRow row in rows) result.Children.Add(row);
        return result;
    }

    private static ContentTableCell ReadCell(XElement cell, int row, int column)
    {
        List<string> lines = [];
        foreach (XElement paragraph in cell.Elements(XName.Get("p", OdfNamespaces.Text)))
            lines.Add(paragraph.Value);

        string text = string.Join('\n', lines);
        string? type = cell.Attribute(XName.Get("value-type", OdfNamespaces.Office))?.Value;

        object? value = type switch
        {
            "float" or "percentage" or "currency" => Number(cell),
            "boolean" => cell.Attribute(XName.Get("boolean-value", OdfNamespaces.Office))?.Value
                is { } flag && bool.TryParse(flag, out bool parsed) ? parsed : null,
            "string" => cell.Attribute(XName.Get("string-value", OdfNamespaces.Office))?.Value
                        ?? (text.Length == 0 ? null : text),
            _ => null,
        };

        ContentTableCell result = new() { Row = row, Column = column, Value = value };
        if (text.Length > 0) AddParagraph(result, text);
        return result;
    }

    private static double? Number(XElement cell)
        => cell.Attribute(XName.Get("value", OdfNamespaces.Office))?.Value is { } value
           && double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
            ? parsed
            : null;

    /// <summary>
    /// The table's rows, each flagged with whether it came from a header group.
    /// </summary>
    /// <remarks>
    /// ODF wraps them in <c>table:table-header-rows</c>, <c>table:table-rows</c> and
    /// <c>table:table-row-group</c>, and permits a bare <c>table:table-row</c> beside all three.
    /// </remarks>
    private static IEnumerable<(XElement Row, bool IsHeader)> Rows(XElement parent, bool header = false)
    {
        foreach (XElement child in parent.Elements())
        {
            if (child.Name.NamespaceName != OdfNamespaces.Table) continue;

            if (child.Name.LocalName == "table-row")
            {
                yield return (child, header);
            }
            else if (child.Name.LocalName is "table-header-rows" or "table-rows" or "table-row-group")
            {
                bool nested = header || child.Name.LocalName == "table-header-rows";
                foreach ((XElement row, bool isHeader) in Rows(child, nested))
                    yield return (row, isHeader);
            }
        }
    }

    private static bool IsEmpty(ContentTableRow row)
    {
        foreach (ContentNode cell in row.Children)
        {
            if (cell is ContentTableCell { Value: not null }) return false;
            if (cell.Children.Count > 0) return false;
        }
        return true;
    }

    private static void AddParagraph(ContentNode target, string text)
    {
        ContentParagraph paragraph = new();
        paragraph.Children.Add(new ContentRun { Text = text });
        target.Children.Add(paragraph);
    }

    private static int Repeat(XElement element, string name)
        => element.Attribute(XName.Get(name, OdfNamespaces.Table))?.Value is { } value
           && int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int count)
           && count > 0
            ? count
            : 1;

    private static XElement? Child(XElement? element, string ns, string name)
        => element?.Element(XName.Get(name, ns));

    private static IEnumerable<XElement> Children(XElement? element, string ns, string name)
        => element?.Elements(XName.Get(name, ns)) ?? [];
}

using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Extraction;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Reads a DrawingML chart part — <c>c:chartSpace</c> — into the content tree.
/// </summary>
/// <remarks>
/// <para>
/// <strong>What a chart is, for extraction.</strong> A title, some axis titles, and a table of
/// numbers whose columns are named by the series and whose rows are named by the categories.
/// That is precisely what the content tree already expresses, so a chart becomes a
/// <see cref="SectionKind.Frame"/> section holding paragraphs and one
/// <see cref="ContentTable"/> — no new node kind, and an indexer that already walks tables
/// finds a chart's numbers without being taught anything.
/// </para>
/// <para>
/// The section's shape, which the ODF reader in <c>Paperless.OpenDocument</c> reproduces
/// exactly so that a caller never branches on which family a chart came from:
/// </para>
/// <list type="bullet">
/// <item><description>
/// The chart's title, when it states one, is both <see cref="ContentSection.Name"/> and the
/// section's first paragraph. On the section so that it can be found without counting
/// paragraphs; as a paragraph so that it appears in <see cref="ContentNode.GetText"/>, which
/// never visits a name — the two do not double-count.
/// </description></item>
/// <item><description>
/// Then one paragraph per titled axis, in the order the part states them.
/// </description></item>
/// <item><description>
/// Then a table with <see cref="ContentTable.HeaderRowCount"/> 1: row 0 is an empty corner cell
/// followed by the series names, and each later row is a category label followed by that
/// category's value in each series. The corner cell is empty because the file says nothing
/// about it — ODF's own local table writes the same empty cell.
/// </description></item>
/// </list>
/// <para>
/// <strong>The numbers come from the cache, never from the workbook.</strong> A
/// <c>c:numRef</c> holds a <c>c:f</c> naming a range in an embedded workbook and a
/// <c>c:numCache</c> holding what the authoring application last computed for it. LibreOffice
/// reads only the cache: <c>DoubleSequenceContext::onCharacters</c>
/// (<c>oox/source/drawingml/chart/datasourcecontext.cxx:107-181</c>) stores <c>c:pt/c:v</c> into
/// the model and <c>c:f</c> into <c>maFormula</c>, and
/// <c>DataSequenceConverter::createDataSequence</c>
/// (<c>datasourceconverter.cxx:42-96</c>) builds the sequence from the cached
/// <c>maData</c> alone — the formula is carried only for round-trip export. So the cache is
/// what a reference renderer draws, and reaching the live values would additionally make this
/// library depend on <c>Paperless.Spreadsheets</c>, which the layering forbids. When a
/// reference carries no cache at all, this reader reports the series with no values rather
/// than inventing any; see the note on <see cref="ReadSequence"/>.
/// </para>
/// <para>
/// <strong>No title is invented.</strong> LibreOffice substitutes the single series' name, or
/// failing that the localised string "Chart Title"
/// (<c>chartspaceconverter.cxx:185-204</c>), when a chart has no <c>c:title</c>. Reporting
/// either would claim the file said something it does not, so a chart without a title has none.
/// </para>
/// </remarks>
public static class DrawingChart
{
    /// <summary>
    /// The <c>a:graphicData/@uri</c> that identifies a chart inside a graphic frame.
    /// </summary>
    /// <remarks>
    /// Identical to the chart vocabulary's own namespace, which is what OOXML does for every
    /// <c>graphicData</c> payload.
    /// </remarks>
    public const string ChartUri = OoxmlNamespaces.DrawingMLChart;

    /// <summary>How many <c>c:pt</c> a cache is trusted to declare.</summary>
    /// <remarks>
    /// <c>c:ptCount/@val</c> is an unbounded <c>xsd:unsignedInt</c> written by whoever produced
    /// the file, and it sizes an array before a single point has been read. A cap keeps a
    /// hostile or merely broken part from asking for four gigabytes of nulls; no real chart
    /// comes close, since Excel itself refuses more than 32000 points in a series.
    /// </remarks>
    private const int MaxPointCount = 65536;

    /// <summary>
    /// Reads a chart part.
    /// </summary>
    /// <param name="chartSpace">
    /// The <c>c:chartSpace</c> root of a chart part, or the <c>c:chart</c> inside one.
    /// </param>
    /// <returns>
    /// The chart as a section, or null when the part states no title, no axis title and no
    /// series — in which case the caller should keep recording the frame as a graphic, because
    /// there is nothing better to say about it.
    /// </returns>
    public static ContentSection? Read(XElement chartSpace)
    {
        ArgumentNullException.ThrowIfNull(chartSpace);

        XElement? chart = Is(chartSpace, "chart") ? chartSpace : Child(chartSpace, "chart");
        if (chart is null) return null;

        XElement? plotArea = Child(chart, "plotArea");

        string? title = TitleText(Child(chart, "title"));
        List<string> axisTitles = [];
        foreach (XElement axis in plotArea?.Elements() ?? [])
        {
            // CT_PlotArea's axis elements are catAx, valAx, dateAx and serAx, and every one of
            // them carries an optional c:title of the same type. Matching on the suffix reads
            // all four without listing them, and cannot collide with anything else in the
            // element group.
            if (!axis.Name.LocalName.EndsWith("Ax", StringComparison.Ordinal)) continue;
            if (axis.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (TitleText(Child(axis, "title")) is { Length: > 0 } text) axisTitles.Add(text);
        }

        List<Series> series = ReadSeries(plotArea);

        if (title is null && axisTitles.Count == 0 && series.Count == 0) return null;

        ContentSection section = new() { Kind = SectionKind.Frame, Name = title };
        if (title is not null) AddParagraph(section, title);
        foreach (string axisTitle in axisTitles) AddParagraph(section, axisTitle);
        if (series.Count > 0) section.Children.Add(BuildTable(series));

        return section;
    }

    /// <summary>One series: its name, and the labelled points behind it.</summary>
    private readonly record struct Series(string? Name, string?[] Categories, Point[] Values);

    /// <summary>
    /// One cached point: the number the file states, and the text it states it as.
    /// </summary>
    /// <remarks>
    /// Both, because they answer different questions and only one of them is recoverable from
    /// the other. <see cref="Number"/> is what a caller computing with the chart wants;
    /// <see cref="Text"/> is the literal from <c>c:v</c>, which is the closest thing to
    /// "displayed" available here — running the value through <c>c:formatCode</c> would need a
    /// number formatter, and the only one Paperless has lives in <c>Paperless.Spreadsheets</c>,
    /// above this library.
    /// </remarks>
    private readonly record struct Point(double? Number, string? Text);

    private static List<Series> ReadSeries(XElement? plotArea)
    {
        List<Series> series = [];
        if (plotArea is null) return series;

        foreach (XElement group in plotArea.Elements())
        {
            // CT_PlotArea holds a repeatable choice of plot-type groups, every one of which is
            // named "…Chart": barChart, lineChart, pieChart, scatterChart, areaChart, and the
            // 3-D, doughnut, radar, bubble, stock, surface and of-pie variants beside them.
            // Reading them by that suffix covers the ones this reader was written for and does
            // not fall over on the ones it was not, since a c:ser is the same element in all of
            // them.
            if (group.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!group.Name.LocalName.EndsWith("Chart", StringComparison.Ordinal)) continue;

            foreach (XElement element in Children(group, "ser"))
            {
                // c:cat is what every category-based type states; a scatter chart has no
                // categories at all and states its x values as c:xVal instead, which is the
                // same CT_AxDataSource and reads identically. Likewise c:val and c:yVal.
                (string?[] categories, _) = ReadSequence(Child(element, "cat") ?? Child(element, "xVal"));
                (string?[] text, double?[] numbers) =
                    ReadSequence(Child(element, "val") ?? Child(element, "yVal"));

                Point[] points = new Point[text.Length];
                for (int at = 0; at < points.Length; at++) points[at] = new Point(numbers[at], text[at]);

                series.Add(new Series(Label(Child(element, "tx")), categories, points));
            }
        }

        return series;
    }

    /// <summary>
    /// Lays the series out as the table a spreadsheet would hold them in.
    /// </summary>
    /// <remarks>
    /// The categories are taken from the first series that states any. Every series in a
    /// category-based chart repeats the same <c>c:cat</c> — a bar chart with three series
    /// writes the identical <c>c:strCache</c> three times — so taking the first is not a guess;
    /// and where they genuinely differ, as on a scatter chart whose series have their own
    /// <c>c:xVal</c>, no single column of labels can be right and the first series' is the one
    /// LibreOffice's category axis shows.
    /// </remarks>
    private static ContentTable BuildTable(List<Series> series)
    {
        int rows = 0;
        string?[] categories = [];
        foreach (Series one in series)
        {
            rows = Math.Max(rows, one.Values.Length);
            if (categories.Length == 0 && one.Categories.Length > 0) categories = one.Categories;
        }
        rows = Math.Max(rows, categories.Length);

        // A trailing run of rows with nothing in them is padding, not data: c:ptCount counts
        // the range the chart was drawn over, and a range extended past the last filled cell —
        // or an absurd count in a hostile file — would otherwise become that many empty rows.
        // An interior gap is kept, because it is a category whose value is genuinely missing.
        while (rows > 0 && IsBlank(series, categories, rows - 1)) rows--;

        ContentTable table = new() { ColumnCount = series.Count + 1, HeaderRowCount = 1 };

        ContentTableRow header = new() { Index = 0 };
        header.Children.Add(new ContentTableCell { Row = 0, Column = 0 });
        for (int column = 0; column < series.Count; column++)
        {
            ContentTableCell cell = new()
            {
                Row = 0,
                Column = column + 1,
                Value = series[column].Name,
            };
            if (series[column].Name is { Length: > 0 } name) AddParagraph(cell, name);
            header.Children.Add(cell);
        }
        table.Children.Add(header);

        for (int row = 0; row < rows; row++)
        {
            ContentTableRow line = new() { Index = row + 1 };

            string? category = row < categories.Length ? categories[row] : null;
            ContentTableCell label = new() { Row = row + 1, Column = 0, Value = category };
            if (category is { Length: > 0 }) AddParagraph(label, category);
            line.Children.Add(label);

            for (int column = 0; column < series.Count; column++)
            {
                Point point = row < series[column].Values.Length
                    ? series[column].Values[row]
                    : default;

                ContentTableCell cell = new()
                {
                    Row = row + 1,
                    Column = column + 1,

                    // The number when there is one, and the raw string when the cache held
                    // something that will not parse — a chart may cache "#N/A" for a point its
                    // range could not compute, and dropping it would report the point as blank.
                    Value = point.Number ?? (object?)point.Text,
                };
                if (point.Text is { Length: > 0 } shown) AddParagraph(cell, shown);
                line.Children.Add(cell);
            }

            table.Children.Add(line);
        }

        return table;
    }

    /// <summary>True when no series and no category label says anything about a row.</summary>
    private static bool IsBlank(List<Series> series, string?[] categories, int row)
    {
        if (row < categories.Length && categories[row] is { Length: > 0 }) return false;

        foreach (Series one in series)
        {
            if (row >= one.Values.Length) continue;
            if (one.Values[row] is { Text: { Length: > 0 } } or { Number: not null }) return false;
        }

        return true;
    }

    /// <summary>
    /// A series' name, from <c>c:ser/c:tx</c>.
    /// </summary>
    /// <remarks>
    /// <c>CT_SerTx</c> is a choice of <c>c:strRef</c> and a bare <c>c:v</c>, and the reference's
    /// cache may hold more than one point when the label spans several cells. LibreOffice joins
    /// those with a single space and keeps one label
    /// (<c>datasourceconverter.cxx:50-73</c>, "the internal data table does not support complex
    /// labels"), which is what this does.
    /// </remarks>
    private static string? Label(XElement? source)
    {
        if (source is null) return null;

        if (Child(source, "v") is { } literal && literal.Value.Length > 0) return literal.Value;

        (string?[] text, _) = ReadSequence(source);

        StringBuilder joined = new();
        foreach (string? part in text)
        {
            if (part is not { Length: > 0 }) continue;
            if (joined.Length > 0) joined.Append(' ');
            joined.Append(part);
        }

        return joined.Length == 0 ? null : joined.ToString();
    }

    /// <summary>
    /// The text of a <c>c:title</c>.
    /// </summary>
    /// <remarks>
    /// Three sources, in LibreOffice's own precedence
    /// (<c>TextConverter::createStringSequence</c>, <c>titleconverter.cxx:87-160</c>): the rich
    /// text body under <c>c:tx/c:rich</c>; failing that the paragraphs some producers put
    /// directly in <c>c:txPr</c>, which the schema means for formatting alone and which
    /// LibreOffice reads anyway with the comment "which seems odd, but handle it here"; and
    /// failing that the cached string a <c>c:tx/c:strRef</c> points at. The fourth source
    /// LibreOffice has — a manufactured default — is deliberately not ported.
    /// </remarks>
    private static string? TitleText(XElement? title)
    {
        if (title is null) return null;

        XElement? tx = Child(title, "tx");

        if (Child(tx, "rich") is { } rich && DrawingTextBody.Text(rich) is { Length: > 0 } text)
            return text;

        if (Child(title, "txPr") is { } properties
            && DrawingTextBody.Text(properties) is { Length: > 0 } fallback)
        {
            return fallback;
        }

        return Label(tx);
    }

    /// <summary>
    /// Reads a data sequence's cached points, sparse indices and all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The container is whichever of <c>c:multiLvlStrRef</c>, <c>c:strRef</c>, <c>c:strLit</c>,
    /// <c>c:numRef</c> and <c>c:numLit</c> the source holds — the same five
    /// <c>DataSourceContext::onCreateContext</c> accepts
    /// (<c>datasourcecontext.cxx:335-374</c>). A <c>…Ref</c> contributes its <c>…Cache</c>; the
    /// <c>c:f</c> beside it is read past, deliberately, and so is the workbook it names.
    /// </para>
    /// <para>
    /// <strong><c>c:pt/@idx</c> is sparse and the trap is silent.</strong> A chart over a range
    /// with a gap in it writes <c>ptCount="6"</c> and five <c>c:pt</c> whose indices skip the
    /// blank one, so reading the points in document order shifts every value after the gap onto
    /// the wrong category — a chart that still looks plausible. The array is therefore sized
    /// from <c>c:ptCount</c> and each point placed at its own index.
    /// </para>
    /// <para>
    /// <strong>A reference with no cache yields nothing, and that is the answer.</strong> Some
    /// producers write <c>c:numRef</c> with only a <c>c:f</c>. The series is then reported with
    /// its name and no values rather than with values fetched from the embedded workbook: doing
    /// otherwise would mean opening a second document from inside a reader that must not depend
    /// on the spreadsheet library, and would report numbers no reference renderer draws.
    /// </para>
    /// </remarks>
    private static (string?[] Text, double?[] Numbers) ReadSequence(XElement? source)
    {
        if (source is null) return ([], []);

        if (Child(source, "multiLvlStrRef") is { } multiLevel)
            return ReadMultiLevel(Child(multiLevel, "multiLvlStrCache"));

        XElement? cache =
            Child(Child(source, "strRef"), "strCache")
            ?? Child(source, "strLit")
            ?? Child(Child(source, "numRef"), "numCache")
            ?? Child(source, "numLit");

        if (cache is null) return ([], []);

        int count = Count(cache);
        string?[] text = new string?[count];
        double?[] numbers = new double?[count];

        foreach (XElement point in Children(cache, "pt"))
        {
            int index = Index(point);
            if (index < 0 || index >= count) continue;

            string value = Child(point, "v")?.Value ?? string.Empty;
            text[index] = value;
            numbers[index] = double.TryParse(
                value, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
                ? parsed
                : null;
        }

        return (text, numbers);
    }

    /// <summary>
    /// Flattens a <c>c:multiLvlStrCache</c> into one label per point.
    /// </summary>
    /// <remarks>
    /// Excel writes the levels innermost first, so a category grouped as 2024 → Q1 is
    /// <c>c:lvl[0]</c> "Q1" and <c>c:lvl[1]</c> "2024". Joining them from the outermost inwards
    /// gives "2024 Q1", which reads the way the axis is drawn. LibreOffice keeps the levels
    /// apart instead, as a two-dimensional category axis
    /// (<c>StringSequenceContext::onCharacters</c>, <c>datasourcecontext.cxx:316-322</c>,
    /// indexing by <c>level * ptCount + idx</c>); one column of labels cannot hold that, and a
    /// joined label loses less than the innermost level alone would.
    /// </remarks>
    private static (string?[] Text, double?[] Numbers) ReadMultiLevel(XElement? cache)
    {
        if (cache is null) return ([], []);

        int count = Count(cache);
        List<XElement> levels = [.. Children(cache, "lvl")];
        StringBuilder[] labels = new StringBuilder[count];
        for (int at = 0; at < count; at++) labels[at] = new StringBuilder();

        for (int level = levels.Count - 1; level >= 0; level--)
        {
            foreach (XElement point in Children(levels[level], "pt"))
            {
                int index = Index(point);
                if (index < 0 || index >= count) continue;

                string value = Child(point, "v")?.Value ?? string.Empty;
                if (value.Length == 0) continue;

                if (labels[index].Length > 0) labels[index].Append(' ');
                labels[index].Append(value);
            }
        }

        string?[] text = new string?[count];
        for (int at = 0; at < count; at++)
            text[at] = labels[at].Length == 0 ? null : labels[at].ToString();

        return (text, new double?[count]);
    }

    private static void AddParagraph(ContentNode target, string text)
    {
        ContentParagraph paragraph = new();
        paragraph.Children.Add(new ContentRun { Text = text });
        target.Children.Add(paragraph);
    }

    /// <summary>
    /// A cache's declared length, clamped.
    /// </summary>
    /// <remarks>
    /// A cache with no <c>c:ptCount</c> at all is still readable: the highest index its points
    /// carry stands in, which is what the element would have said.
    /// </remarks>
    private static int Count(XElement cache)
    {
        int declared = Drawing.Number(Child(cache, "ptCount"), "val") ?? -1;
        if (declared < 0)
        {
            foreach (XElement point in cache.Descendants(Name("pt")))
                declared = Math.Max(declared, Index(point) + 1);
        }

        return Math.Clamp(declared, 0, MaxPointCount);
    }

    private static int Index(XElement point) => Drawing.Number(point, "idx") ?? -1;

    private static XName Name(string localName)
        => XName.Get(localName, OoxmlNamespaces.DrawingMLChart);

    private static XElement? Child(XElement? element, string localName)
        => element?.Element(Name(localName));

    private static IEnumerable<XElement> Children(XElement? element, string localName)
        => element?.Elements(Name(localName)) ?? [];

    private static bool Is(XElement element, string localName)
        => element.Name.NamespaceName == OoxmlNamespaces.DrawingMLChart
           && element.Name.LocalName == localName;
}

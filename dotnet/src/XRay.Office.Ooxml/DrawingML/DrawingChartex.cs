using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Graphics;

namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// Reads an extended ("chartex") chart part — <c>cx:chartSpace</c> — into a
/// <see cref="ChartPlot"/>.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Chartex is the 2014 vocabulary, and it is not <c>c:</c> with a different prefix.</strong>
/// There is no plot-group element: every <c>cx:series</c> carries a <c>@layoutId</c> naming its own
/// type, and the data lives in a <c>cx:chartData</c> block the series point at by id. LibreOffice
/// reads the eight layout ids in <c>PlotAreaContext::onCreateContext</c>
/// (<c>oox/source/drawingml/chart/plotareacontext.cxx</c>:203-247, this tree, which is 27.2 and not
/// the reference binary's source) and manufactures one type group per series so that the rest of
/// its chart machinery keeps working.
/// </para>
/// <para>
/// <strong>What 26.2.4.2 actually draws is far less than what it resolves, and this reader
/// reproduces the drawing.</strong> Measured on both corpus witnesses with
/// <c>--convert-to fods</c>, the reference resolves the chart to
/// <c>chart:class="ooo:com.sun.star.chart2.ClusteredColumnChartType"</c> with four series — the two
/// that carry a <c>cx:dataId</c> get an eleven-cell value range each and the two
/// <c>paretoLine</c> series get an empty one — eleven categories, two axes, a wall and a floor.
/// Its <em>rendering</em> of the same page carries a white plot rectangle and
/// <strong>two eleven-point polylines and nothing else</strong>: no columns, no axis lines, no tick
/// labels, no category labels, no legend and no title. Counted out of the reference bank's own
/// content stream on both witnesses — <c>probes/chart-rest-r104</c> §2.
/// </para>
/// <para>
/// So the mapping is: <em>every series that names a <c>cx:dataId</c> becomes a line series; every
/// series that does not — which is what a <c>paretoLine</c> is, it derives from its
/// <c>@ownerIdx</c> — is dropped</em>; and the plot draws no axis, no label and no legend. Drawing
/// the columns the layout id names, or the axis furniture a chart normally carries, would put ink
/// on the page that the reference does not have.
/// </para>
/// <para>
/// <strong>The numbers are in the workbook, never in the part.</strong> A <c>c:</c> chart caches its
/// points; chartex has a cache element (<c>cx:lvl</c>/<c>cx:pt</c>) but neither corpus witness uses
/// it — both state a bare <c>cx:f</c> naming a hidden <c>_xlchart.v1.n</c> defined name. The cache
/// is read when it is there and the resolver is asked otherwise, which is the same order
/// <c>ExcelChartConverter::createDataSequence</c> uses for <c>c:</c>.
/// </para>
/// </remarks>
public static class DrawingChartex
{
    /// <summary>The <c>a:graphicData/@uri</c> that identifies a chartex chart in a frame.</summary>
    /// <remarks>Identical to the chartex namespace, as OOXML does for every payload.</remarks>
    public const string ChartUri = OoxmlNamespaces.ExtendedChart;

    /// <summary>How many points one sequence is trusted to hold.</summary>
    private const int MaximumPoints = 65536;

    /// <summary>Reads a <c>cx:chartSpace</c>, or null when it holds nothing drawable.</summary>
    /// <param name="chartSpace">The part's root element.</param>
    /// <param name="theme">The theme <c>a:schemeClr</c> resolves against.</param>
    /// <param name="styles">The theme's format matrix, for the automatic line colour.</param>
    /// <param name="ranges">
    /// The workbook resolver a <c>cx:f</c> is put through. Null leaves a series with no values,
    /// which is what a chartex chart outside a workbook has.
    /// </param>
    public static ChartPlot? Read(
        XElement chartSpace,
        DrawingTheme? theme = null,
        DrawingStyleMatrix? styles = null,
        ChartRangeResolver? ranges = null)
    {
        ArgumentNullException.ThrowIfNull(chartSpace);
        if (!Is(chartSpace, "chartSpace")) return null;

        XElement? plotArea = Child(Child(chartSpace, "chart"), "plotArea");
        XElement? region = Child(plotArea, "plotAreaRegion");
        if (region is null) return null;

        Dictionary<string, Dimension> data = ReadData(Child(chartSpace, "chartData"), ranges);

        int style = DrawingChartAutoFormat.StyleOf(chartSpace);
        List<ChartSeries> series = [];
        string?[] categories = [];

        foreach (XElement element in Children(region, "series"))
        {
            // A series with no `cx:dataId` derives its points from another series — that is what
            // `paretoLine`'s `@ownerIdx` means — and 26.2.4.2 draws nothing for it.
            if (Attribute(Child(element, "dataId"), "val") is not { } id) continue;
            if (!data.TryGetValue(id, out Dimension dimension)) continue;
            if (dimension.Values.Count == 0) continue;

            // The *first* type group's categories and not the longest: LibreOffice takes
            // `rTypeGroups.front()->createCategorySequence()`
            // (oox/source/drawingml/chart/axisconverter.cxx:290, this tree). It matters here —
            // both witnesses' second `cx:data` block names a category range one cell longer than
            // the first, the header row included, and taking that one spreads eleven points over
            // twelve slots. Measured: with the first block's the polyline spans the plot's whole
            // width, 63.67 to 556.86 against 26.2.4.2's 63.65 to 556.83.
            if (categories.Length == 0) categories = dimension.Categories;

            Colour? line = LineOf(Child(element, "spPr"), theme)
                           ?? FillOf(Child(element, "spPr"), theme)
                           ?? DrawingChartAutoFormat.ColourOf(
                               style, ChartAutoObject.LinearSeries, stroke: true,
                               series.Count, series.Count, theme, styles);

            series.Add(new ChartSeries(NameOf(element), dimension.Values, Line: line));
        }

        if (series.Count == 0) return null;

        return new ChartPlot
        {
            Kind = ChartPlotKind.Line,
            Series = series,
            Categories = categories,

            // The reference's own page carries none of this — §2 of the probe. Every one of these
            // defaults to on, so leaving them alone would draw axis lines, ticks and tick labels
            // where 26.2.4.2 draws two polylines.
            ValueAxisVisible = false,
            CategoryAxisVisible = false,
            SecondaryAxisVisible = false,
            ValueLabelsVisible = false,
            CategoryLabelsVisible = false,
            SecondaryLabelsVisible = false,
            ValueTicks = ChartTickMark.None,
            CategoryTicks = ChartTickMark.None,
            SecondaryTicks = ChartTickMark.None,
            Legend = ChartLegendPosition.None,

            // `cx:spPr` on the chart space, which both witnesses state as `a:noFill` — so the
            // chart area is transparent and only the plot rectangle is painted.
            Background = FillOf(Child(chartSpace, "spPr"), theme),
            PlotBackground = FillOf(Child(Child(region, "plotSurface"), "spPr"), theme)
                             ?? Colour.White,
        };
    }

    /// <summary>One <c>cx:data</c> block: its category labels and its values.</summary>
    private readonly record struct Dimension(string?[] Categories, IReadOnlyList<double?> Values);

    private static Dictionary<string, Dimension> ReadData(
        XElement? chartData, ChartRangeResolver? ranges)
    {
        Dictionary<string, Dimension> map = [];
        if (chartData is null) return map;

        foreach (XElement block in Children(chartData, "data"))
        {
            if (Attribute(block, "id") is not { } id) continue;

            string?[] categories = [];
            List<double?> values = [];

            foreach (XElement dimension in block.Elements())
            {
                if (dimension.Name.NamespaceName != OoxmlNamespaces.ExtendedChart) continue;

                bool numeric = dimension.Name.LocalName == "numDim";
                if (!numeric && dimension.Name.LocalName != "strDim") continue;

                Sequence sequence = ReadSequence(dimension, ranges);

                if (numeric && values.Count == 0) values = sequence.Numbers;
                else if (!numeric && categories.Length == 0) categories = [.. sequence.Text];
            }

            map[id] = new Dimension(categories, values);
        }

        return map;
    }

    /// <summary>The cached points a dimension states, or what its formula resolves to.</summary>
    private static Sequence ReadSequence(XElement dimension, ChartRangeResolver? ranges)
    {
        List<string?> text = [];
        List<double?> numbers = [];

        // `cx:lvl/cx:pt idx="n"` — the cache. Sparse, so the index decides the slot.
        foreach (XElement level in Children(dimension, "lvl"))
        {
            foreach (XElement point in Children(level, "pt"))
            {
                if (Drawing.Number(point, "idx") is not { } index) continue;
                if (index is < 0 or >= MaximumPoints) continue;

                while (text.Count <= index) { text.Add(null); numbers.Add(null); }

                string value = point.Value;
                text[index] = value;

                if (double.TryParse(
                        value,
                        System.Globalization.NumberStyles.Float,
                        System.Globalization.CultureInfo.InvariantCulture,
                        out double number))
                {
                    numbers[index] = number;
                }
            }

            break;
        }

        if (text.Count > 0) return new Sequence(text, numbers);

        string? formula = Child(dimension, "f")?.Value;
        if (formula is null || ranges is null) return new Sequence(text, numbers);
        if (ranges(formula) is not { } resolved) return new Sequence(text, numbers);

        return new Sequence([.. resolved.Text], [.. resolved.Numbers]);
    }

    private readonly record struct Sequence(List<string?> Text, List<double?> Numbers);

    /// <summary>The series' name — <c>cx:tx/cx:txData/cx:v</c>, the literal Excel cached.</summary>
    private static string? NameOf(XElement series)
    {
        string? name = Child(Child(Child(series, "tx"), "txData"), "v")?.Value;
        return string.IsNullOrEmpty(name) ? null : name;
    }

    private static Colour? FillOf(XElement? properties, DrawingTheme? theme)
    {
        if (Drawing.Child(properties, "solidFill") is not { } fill) return null;

        foreach (XElement child in fill.Elements())
            if (DrawingColour.Read(child) is { } colour) return colour.Resolve(theme);

        return null;
    }

    private static Colour? LineOf(XElement? properties, DrawingTheme? theme)
    {
        XElement? line = Drawing.Child(properties, "ln");
        if (line is null) return null;
        if (Drawing.Child(line, "noFill") is not null) return null;
        return FillOf(line, theme);
    }

    private static XName Name(string localName)
        => XName.Get(localName, OoxmlNamespaces.ExtendedChart);

    private static bool Is(XElement element, string localName)
        => element.Name == Name(localName);

    private static XElement? Child(XElement? element, string localName)
        => element?.Element(Name(localName));

    private static IEnumerable<XElement> Children(XElement? element, string localName)
        => element?.Elements(Name(localName)) ?? [];

    private static string? Attribute(XElement? element, string name)
        => element?.Attribute(name)?.Value;
}

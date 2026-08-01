using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Paperless.OpenDocument;

namespace Paperless.Presentations.OpenDocument;

/// <summary>
/// Reads an ODF <c>chart:chart</c> sub-document into the model a renderer draws.
/// </summary>
/// <remarks>
/// <para>
/// The ODF counterpart to <see cref="DrawingChartPlot"/>, producing the same
/// <see cref="ChartPlot"/> so that a chart is drawn by one engine whichever family it came from.
/// </para>
/// <para>
/// <strong>ODF states the plot rectangle and OOXML does not, and that is the measurement this
/// whole feature turned on.</strong> Every <c>chart:plot-area</c> LibreOffice writes carries a
/// <c>chart:coordinate-region</c> — the inner plot rectangle, axes' extent, labels excluded —
/// alongside the outer one. Measured on <c>chart-bar-deck.odp</c>: the file states
/// <c>svg:x="2.258cm" svg:y="1.594cm" svg:width="17.674cm" svg:height="8.538cm"</c> and
/// LibreOffice's own PDF for the same file draws the wall rectangle at 2258, 1594, 17672, 8537
/// in hundredths of a millimetre. So the composition heuristic in <see cref="ChartLayout"/>,
/// which every OOXML chart has to go through, is not used at all here.
/// </para>
/// <para>
/// <strong>Where this lives, and why it is not where it belongs.</strong> The natural home is
/// <c>Paperless.OpenDocument</c>, beside <see cref="OdfChart"/>. It cannot go there:
/// <see cref="ChartPlot"/> is defined in <c>Paperless.Ooxml</c> and the two libraries sit at the
/// same layer with no reference between them. The model wants to be in <c>Paperless.Core</c>
/// beside the rest of the drawing IR, which is a Core change that was not worth making with
/// three other agents building against it. Until then this reader is duplicated per family
/// rather than shared, which is the cost recorded in the TODO.
/// </para>
/// </remarks>
internal static class OdfChartPlot
{
    /// <summary>How many series or categories are read from one chart.</summary>
    /// <remarks>
    /// A local table's <c>table:number-columns-repeated</c> is unbounded in an untrusted file and
    /// it sizes an array; the same cap <see cref="OdfChart"/> applies, for the same reason.
    /// </remarks>
    private const int MaxPoints = 65536;

    /// <summary>
    /// The colour a series with no stated fill is drawn in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Not the drawing layer's default shape fill — that is <c>0x729FCF</c>
    /// (<c>include/svx/xdef.hxx:85</c>) and would give a chart the wrong blue. ODF's chart
    /// import has a default of its own: <c>ColorPropertySet</c> is constructed with
    /// <c>m_nDefaultColor( 0x0099ccff )  // blue 8</c>
    /// (<c>xmloff/source/chart/ColorPropertySet.cxx:81</c>), and it hands that colour to every
    /// series whose style states no fill.
    /// </para>
    /// <para>
    /// <strong>Without it an ODF chart draws no bars at all, and looks like a data bug.</strong>
    /// <c>chart-bar-deck.odp</c>'s series style <c>ch9</c> carries a <c>style:chart-properties</c>
    /// and a <c>style:text-properties</c> and no <c>style:graphic-properties</c> whatever — so
    /// there is no fill, no stroke and, before this, nothing painted. LibreOffice's own PDF for
    /// the file draws ten rectangles in <c>0.6 0.8 1</c>, which is exactly <c>#99CCFF</c>: eight
    /// bars and two legend keys. The same chart as <c>.pptx</c> states <c>99ccff</c> on every
    /// series explicitly, because that is what LibreOffice wrote out when it converted the file
    /// — which is how the default became visible in one family and not the other.
    /// </para>
    /// </remarks>
    private static readonly Colour DefaultSeriesFill = Colour.FromRgb(0x99CCFF);

    /// <summary>
    /// Reads a chart sub-document's geometry, or null when there is nothing to draw.
    /// </summary>
    /// <param name="chart">The <c>chart:chart</c> element.</param>
    /// <param name="styles">The sub-document's own styles, for the series' fills.</param>
    public static ChartPlot? Read(XElement chart, OdfChartStyles styles)
    {
        ArgumentNullException.ThrowIfNull(chart);
        ArgumentNullException.ThrowIfNull(styles);

        XElement? plotArea = Child(chart, OdfNamespaces.Chart, "plot-area");
        if (plotArea is null) return null;

        // Only a bar or column chart, for the reason DrawingChartPlot documents: the layout
        // engine draws rectangles against two axes, and a pie chart has neither. ODF states the
        // type twice — on chart:chart and again on each chart:series — and either will do, since
        // LibreOffice writes them to agree. A chart:class this does not draw yields null and the
        // frame goes back to drawing nothing, which is what it did before charts were drawn.
        string? kind = Attribute(chart, OdfNamespaces.Chart, "class");
        if (kind is not ("chart:bar" or "bar")) return null;

        List<XElement> series = [.. Children(plotArea, OdfNamespaces.Chart, "series")];
        if (series.Count == 0) return null;

        OdfChartTable table = OdfChartTable.Read(chart);

        List<ChartSeries> plotted = [];
        foreach (XElement element in series)
        {
            string? style = Attribute(element, OdfNamespaces.Chart, "style-name");

            plotted.Add(new ChartSeries(
                table.LabelOf(Attribute(element, OdfNamespaces.Chart, "label-cell-address")),
                table.ValuesOf(Attribute(element, OdfNamespaces.Chart, "values-cell-range-address")),
                styles.Fill(style) ?? DefaultSeriesFill,
                styles.Line(style),
                styles.LineWidth(style)));
        }

        XElement? categories = null;
        XElement? categoryAxis = null;
        XElement? valueAxis = null;

        foreach (XElement axis in Children(plotArea, OdfNamespaces.Chart, "axis"))
        {
            string? dimension = Attribute(axis, OdfNamespaces.Chart, "dimension");
            if (dimension == "x") { categoryAxis = axis; categories ??= Child(axis, OdfNamespaces.Chart, "categories"); }
            else if (dimension == "y") valueAxis ??= axis;
        }

        string? plotStyle = Attribute(plotArea, OdfNamespaces.Chart, "style-name");

        return new ChartPlot
        {
            Title = TextOf(Child(chart, OdfNamespaces.Chart, "title")),
            CategoryAxisTitle = TextOf(Child(categoryAxis, OdfNamespaces.Chart, "title")),
            ValueAxisTitle = TextOf(Child(valueAxis, OdfNamespaces.Chart, "title")),
            Categories = table.Categories,
            Series = plotted,

            // chart:bar is ODF's name for a *horizontal* bar chart and chart:bar with
            // chart:vertical="false" is the column one — the opposite of what the names suggest.
            // LibreOffice writes chart:class="chart:bar" for both and distinguishes them with
            // the style's chart:vertical, which defaults to false, meaning columns.
            Direction = styles.IsVertical(plotStyle) ? ChartBarDirection.Bar : ChartBarDirection.Column,
            GapWidth = styles.Number(plotStyle, "gap-width") ?? 100.0,
            Overlap = styles.Number(plotStyle, "overlap") ?? 0.0,
            IsStacked = styles.Flag(plotStyle, "stacked") ?? false,
            ValueScale = ScaleOf(valueAxis, styles),
            Legend = LegendOf(Child(chart, OdfNamespaces.Chart, "legend")),
            Background = styles.Fill(Attribute(chart, OdfNamespaces.Chart, "style-name")),
            PlotBackground = styles.Fill(
                Attribute(Child(plotArea, OdfNamespaces.Chart, "wall"), OdfNamespaces.Chart, "style-name")),
            TitleSize = styles.FontSize(
                Attribute(Child(chart, OdfNamespaces.Chart, "title"), OdfNamespaces.Chart, "style-name"))
                ?? Length.FromPoints(13),
            AxisTitleSize = styles.FontSize(
                Attribute(Child(categoryAxis, OdfNamespaces.Chart, "title"), OdfNamespaces.Chart, "style-name"))
                ?? Length.FromPoints(9),
            LabelSize = styles.FontSize(Attribute(valueAxis, OdfNamespaces.Chart, "style-name"))
                ?? Length.FromPoints(10),
            PlotArea = Region(plotArea),
            Space = SpaceOf(chart),
        };
    }

    /// <summary>
    /// The inner plot rectangle the file states, or null when it states none.
    /// </summary>
    /// <remarks>
    /// <c>chart:coordinate-region</c> rather than <c>chart:plot-area</c>'s own
    /// <c>svg:x</c>…<c>svg:height</c>: the plot area's rectangle is the <em>outer</em> one, which
    /// includes the axis labels and the ticks, and using it puts the bars where the labels go.
    /// Measured on <c>chart-bar-deck.odp</c>, whose plot area is 1.451, 1.395, 18.481 × 9.384 cm
    /// and whose coordinate region is 2.258, 1.594, 17.674 × 8.538 — a difference of 0.8 cm on
    /// the left edge, which on a 22 cm chart is 3.7% of its width.
    /// </remarks>
    private static DocRect? Region(XElement plotArea)
    {
        XElement? region = Child(plotArea, OdfNamespaces.Chart, "coordinate-region");
        if (region is null) return null;

        Length? x = OdfValue.ParseLength(Attribute(region, OdfNamespaces.SvgCompatible, "x"));
        Length? y = OdfValue.ParseLength(Attribute(region, OdfNamespaces.SvgCompatible, "y"));
        Length? width = OdfValue.ParseLength(Attribute(region, OdfNamespaces.SvgCompatible, "width"));
        Length? height = OdfValue.ParseLength(Attribute(region, OdfNamespaces.SvgCompatible, "height"));

        return x is null || y is null || width is null || height is null
            ? null
            : new DocRect(x.Value, y.Value, width.Value, height.Value);
    }

    /// <summary>The chart's own coordinate space, from <c>chart:chart/@svg:width</c>.</summary>
    private static DocSize? SpaceOf(XElement chart)
    {
        Length? width = OdfValue.ParseLength(Attribute(chart, OdfNamespaces.SvgCompatible, "width"));
        Length? height = OdfValue.ParseLength(Attribute(chart, OdfNamespaces.SvgCompatible, "height"));
        return width is null || height is null ? null : new DocSize(width.Value, height.Value);
    }

    private static ChartScaleRequest ScaleOf(XElement? axis, OdfChartStyles styles)
    {
        string? style = Attribute(axis, OdfNamespaces.Chart, "style-name");
        return new ChartScaleRequest(
            styles.Number(style, "minimum"),
            styles.Number(style, "maximum"),
            styles.Number(style, "interval-major"),
            styles.Flag(style, "reverse-direction") ?? false);
    }

    private static ChartLegendPosition LegendOf(XElement? legend)
        => legend is null
            ? ChartLegendPosition.None
            : Attribute(legend, OdfNamespaces.Chart, "legend-position") switch
            {
                "start" => ChartLegendPosition.Left,
                "top" => ChartLegendPosition.Top,
                "bottom" => ChartLegendPosition.Bottom,
                _ => ChartLegendPosition.Right,
            };

    private static string? TextOf(XElement? element)
    {
        if (element is null) return null;

        System.Text.StringBuilder joined = new();
        foreach (XElement paragraph in element.Descendants(XName.Get("p", OdfNamespaces.Text)))
        {
            string text = paragraph.Value;
            if (text.Length == 0) continue;
            if (joined.Length > 0) joined.Append(' ');
            joined.Append(text);
        }

        return joined.Length == 0 ? null : joined.ToString();
    }

    private static XElement? Child(XElement? element, string ns, string name)
        => element?.Element(XName.Get(name, ns));

    private static IEnumerable<XElement> Children(XElement? element, string ns, string name)
        => element?.Elements(XName.Get(name, ns)) ?? [];

    private static string? Attribute(XElement? element, string ns, string name)
        => element?.Attribute(XName.Get(name, ns))?.Value;

    /// <summary>
    /// The <c>local-table</c> a chart sub-document carries, read once per chart.
    /// </summary>
    /// <remarks>
    /// The values come from here rather than from the sheet a
    /// <c>chart:values-cell-range-address</c> may point at, for exactly the reason
    /// <see cref="OdfChart"/> documents: <c>SchXMLTableContext</c> fills LibreOffice's internal
    /// data provider from the parsed table and only swaps in a live one afterwards, so the table
    /// is what the reference draws.
    /// </remarks>
    private sealed class OdfChartTable
    {
        private List<string?> _headers = [];
        private List<List<double?>> _columns = [];

        public IReadOnlyList<string?> Categories { get; private init; } = [];

        public static OdfChartTable Read(XElement chart)
        {
            OdfChartTable table = new();
            List<string?> categories = [];

            XElement? local = null;
            foreach (XElement candidate in chart.Descendants(XName.Get("table", OdfNamespaces.Table)))
            {
                local = candidate;
                break;
            }

            if (local is null) return new OdfChartTable { Categories = categories };

            bool header = true;
            foreach (XElement row in Rows(local))
            {
                List<string?> text = [];
                List<double?> numbers = [];

                foreach (XElement cell in row.Elements(XName.Get("table-cell", OdfNamespaces.Table)))
                {
                    int repeat = Repeat(cell, "number-columns-repeated");
                    string? value = cell.Attribute(XName.Get("value", OdfNamespaces.Office))?.Value;
                    double? number = double.TryParse(
                        value, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
                        ? parsed
                        : null;

                    string? shown = cell.Value.Length == 0 ? null : cell.Value;

                    for (int at = 0; at < repeat && text.Count < MaxPoints; at++)
                    {
                        text.Add(shown);
                        numbers.Add(number);
                    }
                }

                if (header)
                {
                    // The header row is an empty corner cell followed by one label per series.
                    for (int at = 1; at < text.Count; at++) table._headers.Add(text[at]);
                    header = false;
                    continue;
                }

                categories.Add(text.Count > 0 ? text[0] : null);

                for (int at = 1; at < numbers.Count; at++)
                {
                    while (table._columns.Count < at) table._columns.Add([]);
                    table._columns[at - 1].Add(numbers[at]);
                }
            }

            return new OdfChartTable
            {
                Categories = categories,
                _headers = table._headers,
                _columns = table._columns,
            };
        }

        /// <summary>
        /// A series' name, from the header cell its <c>chart:label-cell-address</c> names.
        /// </summary>
        /// <remarks>
        /// The address is <c>local-table.$B$1</c>, whose column letter is what identifies the
        /// series. Parsing the letter rather than the whole address is enough because a chart's
        /// local table always puts the labels in row 1 and the values below them — which is the
        /// layout <see cref="OdfChart"/> already relies on, from the other direction.
        /// </remarks>
        public string? LabelOf(string? address)
        {
            int column = ColumnOf(address);
            return column >= 1 && column - 1 < _headers.Count ? _headers[column - 1] : null;
        }

        /// <summary>A series' values, from the column its range address names.</summary>
        public List<double?> ValuesOf(string? address)
        {
            int column = ColumnOf(address);
            return column >= 1 && column - 1 < _columns.Count ? _columns[column - 1] : [];
        }

        /// <summary>The zero-based column a <c>local-table.$B$2:.$B$5</c> address names.</summary>
        private static int ColumnOf(string? address)
        {
            if (address is null) return -1;

            int at = address.IndexOf('$', StringComparison.Ordinal);
            if (at < 0) return -1;

            int column = 0;
            for (int index = at + 1; index < address.Length; index++)
            {
                char character = char.ToUpperInvariant(address[index]);
                if (character is < 'A' or > 'Z') break;
                column = column * 26 + (character - 'A' + 1);
            }

            return column - 1;
        }

        private static IEnumerable<XElement> Rows(XElement table)
        {
            foreach (XElement child in table.Descendants(XName.Get("table-row", OdfNamespaces.Table)))
                yield return child;
        }

        private static int Repeat(XElement cell, string name)
        {
            string? value = cell.Attribute(XName.Get(name, OdfNamespaces.Table))?.Value;
            return int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int parsed)
                ? Math.Clamp(parsed, 1, MaxPoints)
                : 1;
        }
    }
}

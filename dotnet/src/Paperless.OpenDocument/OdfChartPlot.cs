using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.OpenDocument;

/// <summary>
/// Reads an ODF <c>chart:chart</c> sub-document into the model a renderer draws.
/// </summary>
/// <remarks>
/// <para>
/// The ODF counterpart to <c>DrawingChartPlot</c> in <c>Paperless.Ooxml</c>, producing the same
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
/// <strong>Where this lives, and why it moved here.</strong> It began in
/// <c>Paperless.Presentations</c>, because <see cref="ChartPlot"/> was defined in
/// <c>Paperless.Ooxml</c> and <c>Paperless.OpenDocument</c> — a sibling with no reference to it —
/// could not name the type it had to return. That put an ODF reader in a family library and left
/// a spreadsheet needing a second copy of it. Moving the model and the layout engine into
/// <c>Paperless.Core.Charts</c>, which is where the rest of the drawing IR already lives, let
/// this reader come down beside <see cref="OdfChart"/> and serve ODP, ODS and ODT from one place.
/// </para>
/// </remarks>
public static class OdfChartPlot
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

        // ODF states the type twice — on chart:chart and again on each chart:series — and either
        // will do, since LibreOffice writes them to agree. A chart:class this does not draw yields
        // null and the frame goes back to drawing nothing, which is what it did before charts were
        // drawn, rather than being drawn as some other type.
        if (KindOf(Attribute(chart, OdfNamespaces.Chart, "class")) is not { } kind) return null;

        List<XElement> series = [.. Children(plotArea, OdfNamespaces.Chart, "series")];
        if (series.Count == 0) return null;

        OdfChartTable table = OdfChartTable.Read(chart);

        List<ChartSeries> plotted = [];
        foreach (XElement element in series)
        {
            string? style = Attribute(element, OdfNamespaces.Chart, "style-name");

            List<double?> values =
                table.ValuesOf(Attribute(element, OdfNamespaces.Chart, "values-cell-range-address"));

            // ODF states the type on each series as well as on the chart, which is how it writes
            // a combination chart: the same chart:plot-area holds a chart:series
            // chart:class="chart:bar" beside a chart:series chart:class="chart:line". Reading it
            // per series is all a combination chart needs here.
            plotted.Add(new ChartSeries(
                table.LabelOf(Attribute(element, OdfNamespaces.Chart, "label-cell-address")),
                values,
                styles.Fill(style) ?? DefaultSeriesFill,
                styles.Line(style),
                styles.LineWidth(style),
                PointFills(element, values.Count, styles),
                KindOf(Attribute(element, OdfNamespaces.Chart, "class")) ?? kind));
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
            Kind = kind,

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
            ValueGrid = GridOf(valueAxis, styles),
            CategoryGrid = GridOf(categoryAxis, styles),
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
    /// What geometry a <c>chart:class</c> means, or null for one that is not drawn.
    /// </summary>
    /// <remarks>
    /// The prefix is written in full — <c>chart:bar</c> — because the attribute holds a QName and
    /// the <c>chart</c> prefix is bound in every document LibreOffice writes; the bare form is
    /// accepted too, for a writer that bound a different prefix. A ring is drawn as a circle,
    /// losing its hole; <c>chart:radar</c>, <c>chart:stock</c>, <c>chart:bubble</c> and
    /// <c>chart:surface</c> yield null and draw nothing at all.
    /// </remarks>
    private static ChartPlotKind? KindOf(string? stated)
    {
        string? kind = stated;
        if (kind is null) return null;

        int colon = kind.IndexOf(':', StringComparison.Ordinal);
        if (colon >= 0) kind = kind[(colon + 1)..];

        return kind switch
        {
            "bar" => ChartPlotKind.Bar,
            "line" => ChartPlotKind.Line,
            "circle" or "ring" => ChartPlotKind.Pie,
            "area" => ChartPlotKind.Area,
            "scatter" => ChartPlotKind.Scatter,
            _ => null,
        };
    }

    /// <summary>
    /// The colour an axis' major gridlines are drawn in, or null when it has none.
    /// </summary>
    /// <remarks>
    /// A <c>chart:grid chart:class="major"</c> inside the axis, whose style states the stroke.
    /// A grid whose style states none takes chart2's own default, <c>0xB3B3B3</c>
    /// (<c>chart2/source/model/main/GridProperties.cxx:64-66</c>), and one whose style states
    /// <c>draw:stroke="none"</c> is not drawn.
    /// </remarks>
    private static Colour? GridOf(XElement? axis, OdfChartStyles styles)
    {
        foreach (XElement grid in Children(axis, OdfNamespaces.Chart, "grid"))
        {
            if (Attribute(grid, OdfNamespaces.Chart, "class") is { } stated && stated != "major")
                continue;

            string? style = Attribute(grid, OdfNamespaces.Chart, "style-name");
            return styles.HasStroke(style) ? styles.Line(style) ?? DefaultGrid : null;
        }

        return null;
    }

    /// <summary>chart2's own gridline colour, gray30.</summary>
    private static readonly Colour DefaultGrid = Colour.FromRgb(0xB3B3B3);

    /// <summary>
    /// The per-point fills a series states, or null when it states none.
    /// </summary>
    /// <remarks>
    /// <c>chart:data-point</c>, in order, each optionally carrying <c>chart:repeated</c> — which
    /// is how a pie whose eight wedges are all default writes one element rather than eight, and
    /// how a pie with one recoloured wedge writes three. Only a pie normally states any.
    /// </remarks>
    private static Colour?[]? PointFills(XElement series, int count, OdfChartStyles styles)
    {
        Colour?[]? fills = null;
        int at = 0;

        foreach (XElement point in Children(series, OdfNamespaces.Chart, "data-point"))
        {
            int repeat = 1;
            if (int.TryParse(
                    Attribute(point, OdfNamespaces.Chart, "repeated"),
                    NumberStyles.Integer,
                    CultureInfo.InvariantCulture,
                    out int stated))
            {
                repeat = Math.Clamp(stated, 1, MaxPoints);
            }

            Colour? fill = styles.Fill(Attribute(point, OdfNamespaces.Chart, "style-name"));

            for (int copy = 0; copy < repeat && at < MaxPoints; copy++, at++)
            {
                if (fill is null) continue;

                fills ??= new Colour?[Math.Max(count, at + 1)];
                if (at >= fills.Length) continue;
                fills[at] = fill;
            }
        }

        return fills;
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

                    string? shown = CellText(cell);

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

        /// <summary>
        /// The zero-based column a range address names, in either of the two forms ODF writes.
        /// </summary>
        /// <remarks>
        /// <para>
        /// <strong>A deck writes one form and a spreadsheet writes the other, and only the first
        /// has dollars in it.</strong> A chart with no live data names its own table —
        /// <c>local-table.$B$2:.$B$5</c> — and a chart over real cells names them:
        /// <c>Revenue.B2:Revenue.B5</c>. Reading the address by finding a <c>$</c> works for every
        /// deck and for no spreadsheet, so a sheet's chart resolved every series to no column,
        /// plotted nothing, and drew an axis of 0–15 from the empty-data default while its title,
        /// its categories and its legend all came out right. That is the shape of failure worth
        /// naming: everything textual was correct and only the numbers were missing.
        /// </para>
        /// <para>
        /// So: take the range's first cell, drop the sheet or table name at the last dot before it,
        /// ignore any dollars, and read the column letters.
        /// </para>
        /// </remarks>
        private static int ColumnOf(string? address)
        {
            if (address is null) return -1;

            int colon = address.IndexOf(':', StringComparison.Ordinal);
            ReadOnlySpan<char> first = colon < 0 ? address : address.AsSpan(0, colon);

            int dot = first.LastIndexOf('.');
            ReadOnlySpan<char> cell = dot < 0 ? first : first[(dot + 1)..];

            int column = 0;
            bool any = false;

            foreach (char character in cell)
            {
                if (character == '$') continue;

                char upper = char.ToUpperInvariant(character);
                if (upper is < 'A' or > 'Z') break;

                column = (column * 26) + (upper - 'A' + 1);
                any = true;
            }

            return any ? column - 1 : -1;
        }

        /// <summary>
        /// A local-table cell's text, from its own paragraphs alone.
        /// </summary>
        /// <remarks>
        /// Not <c>XElement.Value</c>. A chart written by Calc puts a
        /// <c>draw:g/svg:desc</c> beside the paragraph holding the address the cell came from, and
        /// the whole-subtree value concatenates the two — a category read as
        /// <c>Q1Revenue.A2:Revenue.A5</c>, drawn under the bars exactly like that.
        /// </remarks>
        private static string? CellText(XElement cell)
        {
            System.Text.StringBuilder joined = new();

            foreach (XElement paragraph in cell.Elements(XName.Get("p", OdfNamespaces.Text)))
                joined.Append(paragraph.Value);

            return joined.Length == 0 ? null : joined.ToString();
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

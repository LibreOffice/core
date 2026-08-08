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

        // chart:series-source lives on the plot area's own style and says which way round the
        // local table is; see OdfChartTable.
        string? sourceStyle = Attribute(plotArea, OdfNamespaces.Chart, "style-name");
        bool seriesInRows = styles.Text(sourceStyle, "series-source") == "rows";

        OdfChartTable table = OdfChartTable.Read(chart, seriesInRows);

        // The plot area's own style carries the labels every series inherits, which is where
        // LibreOffice writes "label every point of this chart" — chart:data-label-number on the
        // plot area rather than on each series.
        string? areaStyle = Attribute(plotArea, OdfNamespaces.Chart, "style-name");
        ChartDataLabel? areaLabel = LabelOf(areaStyle, styles, kind, null);

        // ODF orders a stock plot's series open, LOW, HIGH, close — the reverse of OOXML's middle
        // pair — and drops the open when the chart is not a Japanese candlestick
        // (xmloff/source/chart/SchXMLChartContext.cxx:1051-1085, whose own comment reads "with
        // japanese candlesticks: open, low, high, close; otherwise: low, high, close"). See
        // ChartStockRole; reading one vocabulary's order into the other inverts every whisker
        // whose high and low are not already in the right places.
        string? plotStyleName = Attribute(plotArea, OdfNamespaces.Chart, "style-name");
        bool japanese = styles.Flag(plotStyleName, "japanese-candle-stick") ?? false;

        ChartStockRole[] stockRoles = japanese
            ?
            [
                ChartStockRole.Open, ChartStockRole.Low, ChartStockRole.High, ChartStockRole.Close,
            ]
            : [ChartStockRole.Low, ChartStockRole.High, ChartStockRole.Close];

        int stockRole = 0;

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
            ChartPlotKind own = KindOf(Attribute(element, OdfNamespaces.Chart, "class")) ?? kind;

            ChartStockRole role = ChartStockRole.None;
            if (own is ChartPlotKind.Stock && stockRole < stockRoles.Length)
                role = stockRoles[stockRole++];

            plotted.Add(new ChartSeries(
                table.LabelOf(
                    Attribute(element, OdfNamespaces.Chart, "label-cell-address"),
                    Attribute(element, OdfNamespaces.Chart, "values-cell-range-address")),
                values,
                styles.Fill(style) ?? DefaultSeriesFill,
                styles.Line(style),
                styles.LineWidth(style),
                PointFills(element, values.Count, styles),
                own)
            {
                Marker = MarkerOf(style, styles, own),
                Label = LabelOf(style, styles, own, areaLabel),
                PointLabels = PointLabelsOf(element, values.Count, styles, own, areaLabel),
                Trendlines = TrendlinesOf(element, styles),
                StockRole = role,
            });
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
            Rings = IsRing(Attribute(chart, OdfNamespaces.Chart, "class")),

            // chart:bar is ODF's name for a *horizontal* bar chart and chart:bar with
            // chart:vertical="false" is the column one — the opposite of what the names suggest.
            // LibreOffice writes chart:class="chart:bar" for both and distinguishes them with
            // the style's chart:vertical, which defaults to false, meaning columns.
            Direction = styles.IsVertical(plotStyle) ? ChartBarDirection.Bar : ChartBarDirection.Column,
            GapWidth = styles.Number(plotStyle, "gap-width") ?? 100.0,
            Overlap = styles.Number(plotStyle, "overlap") ?? 0.0,
            IsStacked = styles.Flag(plotStyle, "stacked") ?? false,
            ValueScale = ScaleOf(valueAxis, styles),
            ValueFormat = styles.Format(Attribute(valueAxis, OdfNamespaces.Chart, "style-name")),
            CategoryFormat = styles.Format(Attribute(categoryAxis, OdfNamespaces.Chart, "style-name")),
            CategoryAxisText = AxisTextOf(
                Attribute(categoryAxis, OdfNamespaces.Chart, "style-name"), styles),

            // chart:visible="false" is ODF's c:delete: an axis that is present in the file so that
            // its scale and its grid survive a round trip, and drawn as nothing.
            ValueAxisVisible = Visible(valueAxis, styles),
            CategoryAxisVisible = Visible(categoryAxis, styles),
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

            // The legend's own style, not the value axis'. See ChartPlot.LegendSize.
            LegendSize = styles.FontSize(Attribute(
                Child(chart, OdfNamespaces.Chart, "legend"), OdfNamespaces.Chart, "style-name")),
            PlotArea = Region(plotArea),
            Space = SpaceOf(chart),

            // chart:filled-radar is the only radar class that fills; chart:radar draws a stroked
            // polygon and takes its markers from the series' own chart:symbol-type, which is what
            // makes ODF's two classes cover OOXML's three c:radarStyle values.
            RadarStyle = Filled(Attribute(chart, OdfNamespaces.Chart, "class"))
                ? ChartRadarStyle.Filled
                : ChartRadarStyle.Standard,

            // chart:stock-range-line is ODF's c:hiLowLines and chart:japanese-candle-stick is its
            // c:upDownBars; the two markers are the gain and loss fills.
            HasHighLowLines = Child(plotArea, OdfNamespaces.Chart, "stock-range-line") is not null,
            HasUpDownBars = japanese,
            StockGainFill = styles.Fill(Attribute(
                Child(plotArea, OdfNamespaces.Chart, "stock-gain-marker"),
                OdfNamespaces.Chart,
                "style-name")),
            StockLossFill = styles.Fill(Attribute(
                Child(plotArea, OdfNamespaces.Chart, "stock-loss-marker"),
                OdfNamespaces.Chart,
                "style-name")),
        };
    }

    /// <summary>Whether a <c>chart:class</c> is the filled radar rather than the stroked one.</summary>
    private static bool Filled(string? stated)
    {
        if (stated is null) return false;

        int colon = stated.IndexOf(':', StringComparison.Ordinal);
        return (colon >= 0 ? stated[(colon + 1)..] : stated) == "filled-radar";
    }

    /// <summary>
    /// What geometry a <c>chart:class</c> means, or null for one that is not drawn.
    /// </summary>
    /// <remarks>
    /// The prefix is written in full — <c>chart:bar</c> — because the attribute holds a QName and
    /// the <c>chart</c> prefix is bound in every document LibreOffice writes; the bare form is
    /// accepted too, for a writer that bound a different prefix. A ring keeps its hole — see
    /// <see cref="ChartPlot.Rings"/>, which <see cref="IsRing"/> sets — and <c>chart:surface</c>
    /// is read as a bar chart, which is the substitution LibreOffice's own importer makes for it;
    /// <c>Paperless.Ooxml.DrawingML.DrawingChartPlot.KindOf</c> carries the measurement that
    /// settled that.
    /// </remarks>
    private static ChartPlotKind? KindOf(string? stated)
    {
        string? kind = stated;
        if (kind is null) return null;

        int colon = kind.IndexOf(':', StringComparison.Ordinal);
        if (colon >= 0) kind = kind[(colon + 1)..];

        return kind switch
        {
            "bar" or "surface" => ChartPlotKind.Bar,
            "line" => ChartPlotKind.Line,
            "circle" or "ring" => ChartPlotKind.Pie,
            "area" => ChartPlotKind.Area,
            "scatter" => ChartPlotKind.Scatter,
            "radar" or "filled-radar" => ChartPlotKind.Radar,
            "bubble" => ChartPlotKind.Bubble,
            "stock" => ChartPlotKind.Stock,
            _ => null,
        };
    }

    /// <summary>Whether a <c>chart:class</c> is the doughnut one.</summary>
    /// <remarks>
    /// ODF spells it <c>chart:ring</c> and states nothing about the hole's size, which suits the
    /// reference exactly: <c>PieChart</c> derives every ring's radius from the ring count alone.
    /// See <see cref="ChartPlot.Rings"/>.
    /// </remarks>
    private static bool IsRing(string? stated)
    {
        if (stated is null) return false;
        int colon = stated.IndexOf(':', StringComparison.Ordinal);
        return (colon >= 0 ? stated[(colon + 1)..] : stated) == "ring";
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

    /// <summary>Whether an axis is drawn — <c>chart:visible="false"</c> says it is not.</summary>
    private static bool Visible(XElement? axis, OdfChartStyles styles)
        => axis is null
           || styles.Flag(Attribute(axis, OdfNamespaces.Chart, "style-name"), "visible") != false;

    /// <summary>
    /// What a series' style says its data labels show, or null for none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// ODF folds the four OOXML flags into two attributes: <c>chart:data-label-number</c>, whose
    /// values are <c>none</c>, <c>value</c>, <c>percentage</c> and <c>value-and-percentage</c>,
    /// and <c>chart:data-label-text</c>, a boolean meaning the category name
    /// (<c>xmloff/source/chart/SchXMLSeriesHelper</c> and <c>PropertyMap.hxx</c>'s
    /// <c>Label</c> mapping). There is no separate "series name" flag, which is why nothing here
    /// sets <see cref="ChartDataLabel.ShowSeries"/>.
    /// </para>
    /// <para>
    /// A style that states neither attribute inherits the level above rather than defaulting to
    /// showing nothing, which is how a plot area saying <c>chart:data-label-number="value"</c>
    /// labels every series under it.
    /// </para>
    /// </remarks>
    private static ChartDataLabel? LabelOf(
        string? style, OdfChartStyles styles, ChartPlotKind kind, ChartDataLabel? inherited)
    {
        string? number = styles.Text(style, "data-label-number");
        bool? text = styles.Flag(style, "data-label-text");
        string? position = styles.Text(style, "label-position");

        if (number is null && text is null && position is null) return inherited;

        bool value = number switch
        {
            "value" or "value-and-percentage" => true,
            null => inherited?.ShowValue ?? false,
            _ => false,
        };

        bool percent = number switch
        {
            "percentage" or "value-and-percentage" => kind == ChartPlotKind.Pie,
            null => inherited?.ShowPercent ?? false,
            _ => false,
        };

        return new ChartDataLabel
        {
            ShowValue = value,
            ShowPercent = percent,
            ShowCategory = text ?? inherited?.ShowCategory ?? false,
            ValueFormat = styles.Format(style) ?? inherited?.ValueFormat,
            Separator = percent && !value ? "\n" : inherited?.Separator ?? "; ",
            Placement = PlacementOf(position) ?? inherited?.Placement,
        };
    }

    /// <summary>The per-point label overrides a series states, or null.</summary>
    /// <remarks>
    /// <c>chart:data-point</c> in order, honouring <c>chart:repeated</c> exactly as the fills do —
    /// a pie whose eight wedges are all default writes one element and not eight.
    /// </remarks>
    private static ChartDataLabel?[]? PointLabelsOf(
        XElement series,
        int count,
        OdfChartStyles styles,
        ChartPlotKind kind,
        ChartDataLabel? inherited)
    {
        ChartDataLabel?[]? labels = null;
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

            string? style = Attribute(point, OdfNamespaces.Chart, "style-name");
            ChartDataLabel? own = LabelOf(style, styles, kind, inherited);

            for (int copy = 0; copy < repeat && at < MaxPoints; copy++, at++)
            {
                if (own is null || ReferenceEquals(own, inherited)) continue;

                labels ??= new ChartDataLabel?[Math.Max(count, at + 1)];
                if (at >= labels.Length) continue;
                labels[at] = own;
            }
        }

        return labels;
    }

    private static ChartLabelPlacement? PlacementOf(string? stated) => stated switch
    {
        "outside" => ChartLabelPlacement.Outside,
        "inside" => ChartLabelPlacement.Inside,
        "center" => ChartLabelPlacement.Centre,
        "near-origin" => ChartLabelPlacement.NearOrigin,
        "top" => ChartLabelPlacement.Top,
        "bottom" => ChartLabelPlacement.Bottom,
        "left" => ChartLabelPlacement.Left,
        "right" => ChartLabelPlacement.Right,
        "avoid-overlap" => ChartLabelPlacement.BestFit,
        _ => null,
    };

    /// <summary>
    /// What marker a series draws.
    /// </summary>
    /// <remarks>
    /// <c>chart:symbol-type</c> is <c>none</c>, <c>automatic</c> or <c>named-symbol</c>, and only
    /// the last carries a <c>chart:symbol-name</c>. A scatter chart whose style states nothing
    /// gets one anyway, for the same reason its OOXML counterpart does: without markers a scatter
    /// series that states no line draws nothing at all.
    /// </remarks>
    private static ChartMarker MarkerOf(string? style, OdfChartStyles styles, ChartPlotKind kind)
    {
        string? type = styles.Text(style, "symbol-type");

        if (type is null)
            return kind == ChartPlotKind.Scatter ? ChartMarker.Square : ChartMarker.None;

        if (type == "none") return ChartMarker.None;
        if (type != "named-symbol") return ChartMarker.Square;

        return styles.Text(style, "symbol-name") switch
        {
            "circle" => ChartMarker.Circle,
            "diamond" => ChartMarker.Diamond,
            "arrow-up" or "arrow-down" or "arrow-left" or "arrow-right" => ChartMarker.Triangle,
            "plus" => ChartMarker.Cross,
            "asterisk" or "x" => ChartMarker.Star,
            _ => ChartMarker.Square,
        };
    }

    /// <summary>
    /// What an axis' style says about how its labels are set.
    /// </summary>
    /// <remarks>
    /// <para>
    /// ODF states three of the four and defaults the fourth. <c>chart:text-overlap</c> and
    /// <c>chart:label-arrangement</c> are chart properties; the rotation is
    /// <c>style:rotation-angle</c> on the axis' <em>text</em> properties, in whole degrees
    /// anticlockwise, which is the direction ODF and this model already agree on and OOXML does
    /// not. Line breaking has no ODF attribute at all, so it stays at chart2's own model default
    /// of false (<c>Axis.cxx:239</c>) — which is the opposite of what OOXML's importer sets, and
    /// it is why an ODF axis can reach the rotation path without a label having to wrap first.
    /// </para>
    /// <para>
    /// The arrangement defaults to <c>ChartAxisArrangeOrderType_AUTO</c> (<c>Axis.cxx:242</c>),
    /// which is <see cref="ChartLabelStagger.Auto"/> — so an ODF axis may stagger where an OOXML
    /// one may not. In practice it rarely does; see <see cref="ChartAxisLabels"/> for why the
    /// route to staggering is nearly closed.
    /// </para>
    /// </remarks>
    private static ChartAxisText AxisTextOf(string? style, OdfChartStyles styles)
    {
        double degrees = styles.Rotation(style) ?? 0.0;
        degrees -= 360.0 * Math.Floor(degrees / 360.0);

        return new ChartAxisText(
            degrees * Math.PI / 180.0,
            OverlapAllowed: styles.Flag(style, "text-overlap") ?? false,
            LineBreakAllowed: false,
            Stagger: styles.Text(style, "label-arrangement") switch
            {
                "side-by-side" => ChartLabelStagger.SideBySide,
                "stagger-even" => ChartLabelStagger.Even,
                "stagger-odd" => ChartLabelStagger.Odd,
                _ => ChartLabelStagger.Auto,
            });
    }

    /// <summary>
    /// The trendlines a series carries, or null when it carries none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>chart:regression-curve</c> plus, for the mean line, <c>chart:mean-value</c>. Everything
    /// about the fit is on the curve's <em>style</em> —
    /// <c>chart:regression-type</c>, <c>-max-degree</c>, <c>-period</c>, <c>-moving-type</c>,
    /// <c>-extrapolate-forward</c>, <c>-extrapolate-backward</c>, <c>-force-intercept</c>,
    /// <c>-intercept-value</c> — and only the two display flags are on the element, on its child
    /// <c>chart:equation</c>. That child's absence means neither is shown, which is the opposite
    /// of OOXML's rule and is why the two readers cannot share a default.
    /// </para>
    /// <para>
    /// <strong>ODF bakes the equation's position into the file, exactly as it bakes the plot
    /// rectangle.</strong> <c>chart:equation/@svg:x</c> and <c>@svg:y</c> are in the chart's own
    /// coordinate space — the same space <c>chart:coordinate-region</c> uses — so an ODF chart
    /// needs no equivalent of <c>VSeriesPlotter</c>'s default placement at all.
    /// </para>
    /// </remarks>
    private static List<ChartTrendline>? TrendlinesOf(
        XElement series, OdfChartStyles styles)
    {
        List<ChartTrendline>? trendlines = null;

        foreach (XElement element in Children(series, OdfNamespaces.Chart, "regression-curve"))
        {
            string? style = Attribute(element, OdfNamespaces.Chart, "style-name");
            XElement? equation = Child(element, OdfNamespaces.Chart, "equation");

            bool forced = styles.Flag(style, "regression-force-intercept") ?? false;

            trendlines ??= [];
            trendlines.Add(new ChartTrendline
            {
                Kind = RegressionKindOf(styles.Text(style, "regression-type")),
                Order = (int)(styles.Number(style, "regression-max-degree") ?? 2.0),
                Period = (int)(styles.Number(style, "regression-period") ?? 2.0),
                Moving = styles.Text(style, "regression-moving-type") switch
                {
                    "central" => ChartMovingAverage.Central,
                    "averaged-abscissa" => ChartMovingAverage.AveragedAbscissa,
                    _ => ChartMovingAverage.Prior,
                },
                Forward = styles.Number(style, "regression-extrapolate-forward") ?? 0.0,
                Backward = styles.Number(style, "regression-extrapolate-backward") ?? 0.0,
                Intercept = forced
                    ? styles.Number(style, "regression-intercept-value") ?? 0.0
                    : null,
                ShowEquation = OdfValue.ParseBoolean(
                    Attribute(equation, OdfNamespaces.Chart, "display-equation")) ?? false,
                ShowRSquared = OdfValue.ParseBoolean(
                    Attribute(equation, OdfNamespaces.Chart, "display-r-square")) ?? false,
                Name = styles.Text(style, "regression-name"),
                Line = styles.Line(style),
                LineWidth = styles.LineWidth(style),
                EquationAt = EquationAt(equation),
            });
        }

        // chart:mean-value is a horizontal line at the series' average and not a regression at
        // all, which is exactly why RegressionCurveHelper::isMeanValueLine is tested before every
        // degree and period is read in VSeriesPlotter::createRegressionCurvesShapes.
        foreach (XElement element in Children(series, OdfNamespaces.Chart, "mean-value"))
        {
            string? style = Attribute(element, OdfNamespaces.Chart, "style-name");
            XElement? equation = Child(element, OdfNamespaces.Chart, "equation");

            trendlines ??= [];
            trendlines.Add(new ChartTrendline
            {
                Kind = ChartTrendlineKind.Mean,
                ShowEquation = OdfValue.ParseBoolean(
                    Attribute(equation, OdfNamespaces.Chart, "display-equation")) ?? false,
                ShowRSquared = OdfValue.ParseBoolean(
                    Attribute(equation, OdfNamespaces.Chart, "display-r-square")) ?? false,
                Line = styles.Line(style),
                LineWidth = styles.LineWidth(style),
                EquationAt = EquationAt(equation),
            });
        }

        return trendlines;
    }

    /// <summary>Where <c>chart:equation</c> states the label goes, or null.</summary>
    private static (Length X, Length Y)? EquationAt(XElement? equation)
    {
        if (OdfValue.ParseLength(Attribute(equation, OdfNamespaces.SvgCompatible, "x"))
            is not { } x)
        {
            return null;
        }

        return OdfValue.ParseLength(Attribute(equation, OdfNamespaces.SvgCompatible, "y"))
            is { } y
            ? (x, y)
            : null;
    }

    /// <summary>The six spellings of <c>chart:regression-type</c>.</summary>
    /// <remarks>
    /// <c>XML_SCH_CONTEXT_SPECIAL_REGRESSION_TYPE</c> in
    /// <c>xmloff/source/chart/PropertyMaps.cxx:1018-1032</c>, which maps each to the same six
    /// <c>com.sun.star.chart2.*RegressionCurve</c> services OOXML's <c>c:trendlineType</c> does —
    /// so the two vocabularies differ only in spelling, and <c>power</c> is the one word both use.
    /// </remarks>
    private static ChartTrendlineKind RegressionKindOf(string? stated) => stated switch
    {
        "polynomial" => ChartTrendlineKind.Polynomial,
        "exponential" => ChartTrendlineKind.Exponential,
        "logarithmic" => ChartTrendlineKind.Logarithmic,
        "power" => ChartTrendlineKind.Power,
        "moving-average" => ChartTrendlineKind.MovingAverage,
        _ => ChartTrendlineKind.Linear,
    };

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
    /// <para>
    /// <c>chart:coordinate-region</c> rather than <c>chart:plot-area</c>'s own
    /// <c>svg:x</c>…<c>svg:height</c>: the plot area's rectangle is the <em>outer</em> one, which
    /// includes the axis labels and the ticks, and using it puts the bars where the labels go.
    /// Measured on <c>chart-bar-deck.odp</c>, whose plot area is 1.451, 1.395, 18.481 × 9.384 cm
    /// and whose coordinate region is 2.258, 1.594, 17.674 × 8.538 — a difference of 0.8 cm on
    /// the left edge, which on a 22 cm chart is 3.7% of its width.
    /// </para>
    /// <para>
    /// <strong>And it is written under either of two namespaces.</strong> The element began as a
    /// LibreOffice extension and 47 of the 71 charts in <c>chart2/qa/extras/data/</c> that state
    /// one still write it that way; the corpus deck uses the standardised spelling, so reading only
    /// that one looked entirely correct and quietly sent two ODF charts in three through the OOXML
    /// heuristic.
    /// </para>
    /// </remarks>
    private static DocRect? Region(XElement plotArea)
    {
        // Both spellings, and the extension one is the commoner: 47 of the 71 charts in
        // chart2/qa/extras/data/ that state a coordinate region at all write it as
        // chartooo:coordinate-region. See OdfNamespaces.ChartExtension.
        XElement? region = Child(plotArea, OdfNamespaces.Chart, "coordinate-region")
                           ?? Child(plotArea, OdfNamespaces.ChartExtension, "coordinate-region");

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
    /// <para>
    /// The values come from here rather than from the sheet a
    /// <c>chart:values-cell-range-address</c> may point at, for exactly the reason
    /// <see cref="OdfChart"/> documents: <c>SchXMLTableContext</c> fills LibreOffice's internal
    /// data provider from the parsed table and only swaps in a live one afterwards, so the table
    /// is what the reference draws.
    /// </para>
    /// <para>
    /// <strong>Which cell a series' range points at is written in the table, not deducible from
    /// the address.</strong> Calc writes a <c>draw:g/svg:desc</c> inside one cell of every column,
    /// holding the sheet range that column was copied from — <c>SchXMLTableContext</c>'s
    /// <c>maRowDescriptions</c>/<c>maColumnDescriptions</c>, which
    /// <c>SchXMLTableHelper::applyTableToInternalDataProvider</c> then uses to map a series'
    /// stated range onto a column of the local table. Reading the column <em>letter</em> instead
    /// works only when the chart's data happens to start in column B, which is the common case
    /// and is why it survived so long: <c>labelString.ods</c> charts <c>Sheet1.D6:Sheet1.D8</c>
    /// against a two-column local table, so the letter said column 4 and the series came out
    /// nameless. Measured over the round-tripped ODF corpus, the legend those missing names
    /// silently emptied was worth a mean <strong>26 pt</strong> on the plot rectangle's right
    /// edge.
    /// </para>
    /// </remarks>
    private sealed class OdfChartTable
    {
        private List<string?> _headers = [];
        private List<List<double?>> _columns = [];
        private Dictionary<string, int> _byRange = [];

        public IReadOnlyList<string?> Categories { get; private init; } = [];

        /// <param name="chart">The <c>chart:chart</c> element.</param>
        /// <param name="rows">
        /// Whether the plot area states <c>chart:series-source="rows"</c>, which transposes the
        /// whole table: the header row then holds the categories and each data row is one series,
        /// named by its own first cell. Thirteen of the corpus' 107 ODF charts state it, and
        /// reading one of them as columns turns the series names into categories and leaves every
        /// series nameless — the model looks plausible and every part of it is wrong.
        /// </param>
        public static OdfChartTable Read(XElement chart, bool rows)
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

            // The whole grid first, because the transposed reading needs the columns and the
            // upright one needs the rows.
            List<List<Cell>> grid = [];

            foreach (XElement row in Rows(local))
            {
                List<Cell> cells = [];

                foreach (XElement cell in row.Elements(XName.Get("table-cell", OdfNamespaces.Table)))
                {
                    int repeat = Repeat(cell, "number-columns-repeated");
                    string? value = cell.Attribute(XName.Get("value", OdfNamespaces.Office))?.Value;
                    double? number = double.TryParse(
                        value, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
                        ? parsed
                        : null;

                    Cell one = new(CellText(cell), number, RangeOf(cell));

                    for (int at = 0; at < repeat && cells.Count < MaxPoints; at++) cells.Add(one);
                }

                grid.Add(cells);
                if (grid.Count >= MaxPoints) break;
            }

            if (grid.Count == 0) return new OdfChartTable { Categories = categories };

            int width = 0;
            foreach (List<Cell> row in grid) width = Math.Max(width, row.Count);

            if (rows)
            {
                // Series in rows: the header row is the categories, each later row is a series.
                for (int at = 1; at < grid[0].Count; at++) categories.Add(grid[0][at].Text);

                for (int row = 1; row < grid.Count; row++)
                {
                    table._headers.Add(grid[row].Count > 0 ? grid[row][0].Text : null);

                    List<double?> values = [];
                    for (int at = 1; at < grid[row].Count; at++) values.Add(grid[row][at].Number);
                    table._columns.Add(values);

                    if (Marker(grid, row, width) is { } range) table._byRange[range] = row - 1;
                }
            }
            else
            {
                for (int at = 1; at < grid[0].Count; at++) table._headers.Add(grid[0][at].Text);

                for (int row = 1; row < grid.Count; row++)
                {
                    categories.Add(grid[row].Count > 0 ? grid[row][0].Text : null);

                    for (int at = 1; at < grid[row].Count; at++)
                    {
                        while (table._columns.Count < at) table._columns.Add([]);
                        table._columns[at - 1].Add(grid[row][at].Number);
                    }
                }

                for (int column = 1; column < width; column++)
                {
                    if (Column(grid, column) is { } range) table._byRange[range] = column - 1;
                }
            }

            return new OdfChartTable
            {
                Categories = categories,
                _headers = table._headers,
                _columns = table._columns,
                _byRange = table._byRange,
            };

            static string? Column(List<List<Cell>> grid, int column)
            {
                foreach (List<Cell> row in grid)
                {
                    if (column < row.Count && row[column].Range is { Length: > 0 } range)
                        return range;
                }

                return null;
            }

            static string? Marker(List<List<Cell>> grid, int row, int width)
            {
                for (int at = 0; at < grid[row].Count && at < width; at++)
                {
                    if (grid[row][at].Range is { Length: > 0 } range) return range;
                }

                return null;
            }
        }

        /// <summary>One cell of the local table: what it shows, what it is, and where it came from.</summary>
        private readonly record struct Cell(string? Text, double? Number, string? Range);

        /// <summary>
        /// A series' name, from the header cell its range names.
        /// </summary>
        public string? LabelOf(string? label, string? values)
        {
            int column = ColumnOf(label ?? values, values);
            return column >= 0 && column < _headers.Count ? _headers[column] : null;
        }

        /// <summary>A series' values, from the column its range address names.</summary>
        public List<double?> ValuesOf(string? address)
        {
            int column = ColumnOf(address, address);
            return column >= 0 && column < _columns.Count ? _columns[column] : [];
        }

        /// <summary>
        /// The zero-based series a range address names: by the marker the table itself carries,
        /// and by the column letter when there is none.
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
        /// The letter is only the fallback, because it is right by accident: it agrees with the
        /// table's own column numbering exactly when the charted range starts at column B. The
        /// marker Calc writes beside each column is the real answer and is tried first — see the
        /// remarks on <see cref="OdfChartTable"/> for what reading the letter alone cost.
        /// </para>
        /// </remarks>
        private int ColumnOf(string? address, string? values)
        {
            if (address is null) return -1;

            if (_byRange.TryGetValue(address, out int mapped)) return mapped;
            if (values is not null && _byRange.TryGetValue(values, out mapped)) return mapped;

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

            return any ? column - 2 : -1;
        }

        /// <summary>The sheet range a cell states it was copied from, or null.</summary>
        /// <remarks>
        /// <c>draw:g/svg:desc</c>, written by Calc beside the paragraph. It is the only thing in
        /// the file that ties a series' stated range to a column of the local table.
        /// </remarks>
        private static string? RangeOf(XElement cell)
        {
            foreach (XElement group in cell.Elements(XName.Get("g", OdfNamespaces.Draw)))
            {
                foreach (XElement description in group.Elements(
                             XName.Get("desc", OdfNamespaces.SvgCompatible)))
                {
                    if (description.Value is { Length: > 0 } text) return text;
                }
            }

            return null;
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

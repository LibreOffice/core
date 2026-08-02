using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Numbers;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Reads a <c>c:chartSpace</c> into the model a renderer draws — <see cref="ChartPlot"/>.
/// </summary>
/// <remarks>
/// <para>
/// The drawing counterpart to <see cref="DrawingChart"/>, which reads the same part into the
/// content tree. Two readers over one part rather than one reader with two outputs, because they
/// want disjoint halves of it: this one needs the fills, the gap width, the axis scaling and the
/// legend position and never looks at a formula; that one needs the cached strings and never
/// looks at an <c>a:solidFill</c>. Extraction is the common case and must not pay for geometry.
/// </para>
/// <para>
/// <strong>Only a bar or column chart, and everything else reads as null.</strong> The layout
/// engine draws rectangles against a category axis and a value axis; a pie chart has neither,
/// and a chart part is not obliged to say so in any way a suffix match can see. Matching
/// <c>c:barChart</c> and <c>c:bar3DChart</c> by name rather than taking the first
/// <c>…Chart</c> group is what stops a pie being drawn as eight bars under two axes that do not
/// exist. Measured over LibreOffice's own <c>chart2/qa/extras/data/pptx/</c>: the loose match
/// drew <em>82 words</em> of axis labels onto
/// <c>PieChartWithAutomaticLayout_SizeAndPosition.pptx</c>, against a reference that draws one.
/// </para>
/// <para>
/// A part holding several groups — a column chart with a line series over it writes a
/// <c>c:barChart</c> and a <c>c:lineChart</c> sharing an axis — draws its bars and drops its
/// line. That is visibly incomplete rather than subtly wrong, which is the failure mode to
/// prefer. <see cref="DrawingChart"/> still reads every group of every type, so the content tree
/// holds all the numbers whatever gets drawn: a chart type that is not drawn loses its picture
/// and not its data.
/// </para>
/// </remarks>
public static class DrawingChartPlot
{
    /// <summary>How many <c>c:pt</c> a cache is trusted to declare.</summary>
    /// <remarks>The same ceiling <see cref="DrawingChart"/> applies, for the same reason.</remarks>
    private const int MaxPointCount = 65536;

    /// <summary>
    /// Reads a chart part's geometry, or null when there is nothing to draw.
    /// </summary>
    /// <param name="chartSpace">The <c>c:chartSpace</c> root, or the <c>c:chart</c> inside it.</param>
    /// <param name="theme">The theme, for resolving a <c>a:schemeClr</c> fill.</param>
    /// <param name="office2007">
    /// Whether Office 2007 wrote the package — <see cref="OoxmlMetadata.IsOffice2007(XElement?)"/>.
    /// It inverts the default of every unstated data-label and trendline flag; see
    /// <see cref="LabelOf"/>.
    /// </param>
    public static ChartPlot? Read(
        XElement chartSpace, DrawingTheme? theme = null, bool office2007 = false)
    {
        ArgumentNullException.ThrowIfNull(chartSpace);

        XElement? chart = Is(chartSpace, "chart") ? chartSpace : Child(chartSpace, "chart");
        if (chart is null) return null;

        XElement? plotArea = Child(chart, "plotArea");
        if (plotArea is null) return null;

        // Every drawable group, in document order. A chart part may hold several sharing one pair
        // of axes — a column chart with a line over it is a c:barChart and a c:lineChart side by
        // side — and taking only the first loses whole series.
        List<XElement> groups = [];
        List<ChartPlotKind> kinds = [];

        foreach (XElement candidate in plotArea.Elements())
        {
            if (candidate.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (KindOf(candidate.Name.LocalName) is not { } matched) continue;
            groups.Add(candidate);
            kinds.Add(matched);
        }

        if (groups.Count == 0) return null;

        // Which c:valAx is the primary and which the secondary, by axis id. A scatter chart states
        // two c:valAx over one pair of ids and neither is a secondary axis; a combination chart
        // with two scales states two pairs, and a group's c:axId is what says which pair it uses.
        ChartAxes axes = ChartAxes.Read(plotArea, groups);

        List<ChartSeries> series = [];
        string?[] categories = [];

        for (int at = 0; at < groups.Count; at++)
        {
            (List<ChartSeries> read, string?[] labels) =
                ReadSeries(groups[at], kinds[at], theme, axes.IndexOf(groups[at]), office2007);

            if (categories.Length == 0 && labels.Length > 0) categories = labels;
            series.AddRange(read);
        }

        if (series.Count == 0) return null;

        // The bar group decides the shape of the category axis and the bar arithmetic, so where
        // there is one it is the chart's own kind whatever came first in the file; that is
        // SeriesPlotterContainer's own rule, which ORs shifted-category positioning over every
        // chart type present (SeriesPlotterContainer.cxx:372-373).
        int primary = kinds.IndexOf(ChartPlotKind.Bar);
        if (primary < 0) primary = 0;

        XElement group = groups[primary];
        ChartPlotKind kind = kinds[primary];

        string? grouping = Value(Child(group, "grouping"));

        // The stock group, wherever it is in the part — its whisker and candle settings live on
        // it, not on whichever group happens to be the chart's own kind. testStockChart.docx puts
        // a c:barChart for the volume series before its c:stockChart, so "the first group" and
        // "the stock group" are two different elements there.
        XElement? stock = null;
        XElement? ofPie = null;
        XElement? radar = null;
        XElement? bubble = null;

        for (int at = 0; at < groups.Count; at++)
        {
            switch (kinds[at])
            {
                case ChartPlotKind.Stock: stock ??= groups[at]; break;
                case ChartPlotKind.OfPie: ofPie ??= groups[at]; break;
                case ChartPlotKind.Radar: radar ??= groups[at]; break;
                case ChartPlotKind.Bubble: bubble ??= groups[at]; break;
                default: break;
            }
        }

        XElement? upDown = Child(stock, "upDownBars");

        return new ChartPlot
        {
            Title = TitleText(Child(chart, "title")),
            // A scatter chart's horizontal axis is its domain and not its category axis, and its
            // title hangs off that element — so reading only c:catAx loses it entirely. The same
            // fallback CategoryAxisVisible already takes, and tdf127720.pptx is what shows it:
            // "Dissolved Oxygen (%)" is three words the reference draws and this did not.
            CategoryAxisTitle = TitleText(Child(axes.Domain ?? axes.Category, "title")),
            ValueAxisTitle = TitleText(Child(axes.Value, "title")),
            Categories = categories,
            Series = series,
            Kind = kind,

            // A doughnut is a pie of concentric rings; the element name is the whole of the file's
            // statement, since c:holeSize reaches nothing in the reference. See ChartPlot.Rings.
            Rings = group.Name.LocalName == "doughnutChart",
            Direction = Value(Child(group, "barDir")) == "bar"
                ? ChartBarDirection.Bar
                : ChartBarDirection.Column,

            // c:gapWidth and c:overlap default to 150 and 0 in the schema, but LibreOffice's
            // importer defaults them to 100 and 0 (oox/source/drawingml/chart/typegroupmodel.cxx)
            // and every file the corpus holds states them. 100 is used here so that a part that
            // omits them agrees with the reference rather than with the specification.
            // A candlestick has no c:gapWidth of its own: what sizes its box is the one inside
            // c:upDownBars, 150 in the corpus file.
            GapWidth = Number(Child(group, "gapWidth")) ?? Number(Child(upDown, "gapWidth")) ?? 100.0,
            Overlap = Number(Child(group, "overlap")) ?? 0.0,
            IsStacked = grouping is "stacked" or "percentStacked",
            ValueScale = ScaleOf(axes.Value),
            ValueFormat = FormatOf(axes.Value),
            CategoryFormat = FormatOf(axes.Category),
            CategoryAxisText = AxisTextOf(axes.Domain ?? axes.Category),
            DataTable = DataTableOf(Child(plotArea, "dTable"), theme),
            SecondaryValueScale = axes.Secondary is null ? null : ScaleOf(axes.Secondary),
            SecondaryValueFormat = FormatOf(axes.Secondary),
            SecondaryValueAxisTitle = TitleText(Child(axes.Secondary, "title")),
            DomainScale = ScaleOf(axes.Domain),
            DomainFormat = FormatOf(axes.Domain),
            ValueAxisVisible = Shown(axes.Value),
            SecondaryAxisVisible = Shown(axes.Secondary),
            CategoryAxisVisible = Shown(axes.Domain ?? axes.Category),
            Legend = LegendOf(Child(chart, "legend")),
            Background = FillOf(Child(chartSpace, "spPr"), theme),
            PlotBackground = FillOf(Child(plotArea, "spPr"), theme),
            ValueGrid = GridOf(axes.Value, theme),
            CategoryGrid = GridOf(axes.Category, theme) ?? GridOf(axes.Domain, theme),
            TitleSize = SizeOf(Child(chart, "title")) ?? Length.FromPoints(13),
            AxisTitleSize = AxisTitleSizeOf(plotArea) ?? Length.FromPoints(9),
            LabelSize = AxisLabelSizeOf(plotArea) ?? Length.FromPoints(10),
            // Fractions of the frame, and no Space: an OOXML chart has no coordinate space of
            // its own — the frame is the space — which is what keeps it out of the stretch an
            // ODF chart goes through.
            PlotAreaFraction = ManualLayout(Child(plotArea, "layout")),

            RadarStyle = Value(Child(radar, "radarStyle")) switch
            {
                "filled" => ChartRadarStyle.Filled,
                "marker" => ChartRadarStyle.Marker,
                _ => ChartRadarStyle.Standard,
            },

            OfPieType = Value(Child(ofPie, "ofPieType")) == "bar"
                ? ChartOfPieType.Bar
                : ChartOfPieType.Pie,

            // Only auto and pos reach chart2 at all; every other c:splitType falls through to the
            // positional split, which is what TypeGroupConverter does with them
            // (typegroupconverter.cxx:474-481).
            SplitType = Value(Child(ofPie, "splitType")) == "pos"
                ? ChartSplitType.Position
                : ChartSplitType.Auto,
            SplitPosition = (int)Math.Clamp(Number(Child(ofPie, "splitPos")) ?? 2.0, 1.0, 4096.0),

            BubbleScale = Math.Clamp(Number(Child(bubble, "bubbleScale")) ?? 100.0, 0.0, 300.0),
            BubbleSizeRepresents = Value(Child(bubble, "sizeRepresents")) == "w"
                ? ChartBubbleSize.Width
                : ChartBubbleSize.Area,

            HasHighLowLines = Child(stock, "hiLowLines") is not null,
            HasUpDownBars = upDown is not null,
            StockGainFill = FillOf(Child(Child(upDown, "upBars"), "spPr"), theme),
            StockLossFill = FillOf(Child(Child(upDown, "downBars"), "spPr"), theme),
        };
    }

    /// <summary>
    /// The plot rectangle a <c>c:manualLayout</c> states, as fractions of the frame.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Written only when the author dragged the plot area; an automatically laid-out chart has no
    /// <c>c:layout</c> content at all, which is the case for every OOXML chart in the corpus and
    /// for 0 of the 192 chart documents in LibreOffice's own <c>chart2/qa/extras/data/</c> that
    /// were checked. So this is the rare path and the computed layout is the common one — the
    /// reverse of ODF, where <c>chart:coordinate-region</c> is always written.
    /// </para>
    /// <para>
    /// <c>c:layoutTarget val="inner"</c> means the rectangle is the plot area proper; the default,
    /// <c>outer</c>, means it includes the axis labels. Only <c>inner</c> is honoured, because an
    /// outer rectangle needs the label sizes subtracted from it and that is the computation this
    /// was meant to avoid — an outer layout falls back to the computed one, which is at worst as
    /// wrong as it would have been.
    /// </para>
    /// </remarks>
    private static (double X, double Y, double Width, double Height)? ManualLayout(XElement? layout)
    {
        XElement? manual = Child(layout, "manualLayout");
        if (manual is null) return null;
        if (Value(Child(manual, "layoutTarget")) != "inner") return null;

        if (Number(Child(manual, "x")) is not { } x) return null;
        if (Number(Child(manual, "y")) is not { } y) return null;
        if (Number(Child(manual, "w")) is not { } width) return null;
        if (Number(Child(manual, "h")) is not { } height) return null;

        return (x, y, width, height);
    }

    /// <summary>
    /// Which geometry an element of <c>CT_PlotArea</c>'s group means, or null when it is not one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Matched by name and not by the <c>…Chart</c> suffix the content reader uses. That suffix
    /// match takes any group, which drew a pie with the bar engine: measured over LibreOffice's own
    /// <c>chart2/qa/extras/data/pptx/</c>, it put <em>82 words</em> of category and value-axis
    /// labels onto <c>PieChartWithAutomaticLayout_SizeAndPosition.pptx</c> against a reference
    /// that draws one.
    /// </para>
    /// <para>
    /// The 3-D variants map onto their flat counterparts, because what this model carries — the
    /// series, the fills, the scale — is the same in both and a flat drawing of a 3-D chart is
    /// nearer the reference than nothing. A doughnut keeps its hole; see
    /// <see cref="ChartPlot.Rings"/>.
    /// </para>
    /// <para>
    /// <strong><c>c:surfaceChart</c> and <c>c:surface3DChart</c> are bar charts, because that is
    /// what the reference draws.</strong> An earlier version left them unread on the reasoning
    /// that a surface is a height field needing a real 3-D projection, that LibreOffice has no
    /// <c>SurfaceChart</c> either, and that the corpus has none to measure. The first two are true
    /// and the conclusion drawn from them was wrong: <c>SERVICE_CHART2_SURFACE</c> is spelled
    /// <c>"com.sun.star.chart2.ColumnChartType"</c> with the comment <c>// Todo</c>
    /// (<c>oox/source/drawingml/chart/typegroupconverter.cxx:79</c>) and the type-group switch
    /// forces <c>mnGrouping = XML_standard</c> under "create a deep 3D bar chart from surface
    /// charts" (<c>:198-199, 217-218</c>) — so the reference's answer to a surface chart is a bar
    /// chart. Measured on a <c>c:surfaceChart</c> made by renaming the plot group in
    /// <c>chart2/qa/extras/data/pptx/chart.pptx</c>: LibreOffice's PDF draws a legend of three
    /// series, four category names and a value axis labelled <c>0 1 … 10</c> — 25 words, against
    /// the <em>nothing</em> a slide whose only shape is the chart frame contributes when the type
    /// is unread. So the substitution is reachable and it is not a picture of nothing. Reading it
    /// as a bar chart gives 21 of those 25; the four that are missing are the tick labels, because
    /// a three-dimensional wall auto-scales to <c>0 1 … 10</c> where the flat one lands on
    /// <c>0 2 … 12</c>. What a flat engine loses is the projection, not the data.
    /// </para>
    /// </remarks>
    private static ChartPlotKind? KindOf(string localName) => localName switch
    {
        "barChart" or "bar3DChart" => ChartPlotKind.Bar,
        "surfaceChart" or "surface3DChart" => ChartPlotKind.Bar,
        "lineChart" or "line3DChart" => ChartPlotKind.Line,
        "pieChart" or "pie3DChart" or "doughnutChart" => ChartPlotKind.Pie,
        "areaChart" or "area3DChart" => ChartPlotKind.Area,
        "scatterChart" => ChartPlotKind.Scatter,
        "radarChart" => ChartPlotKind.Radar,
        "bubbleChart" => ChartPlotKind.Bubble,
        "stockChart" => ChartPlotKind.Stock,
        "ofPieChart" => ChartPlotKind.OfPie,
        _ => null,
    };

    /// <summary>
    /// The colour an axis' major gridlines are drawn in, or null when it has none.
    /// </summary>
    /// <remarks>
    /// <c>c:majorGridlines</c> is usually empty, so its presence is the whole of the file's
    /// statement and the colour is a default: <c>0xB3B3B3</c>, which chart2 sets on
    /// <c>GridProperties</c> (<c>chart2/source/model/main/GridProperties.cxx:64-66</c>). A stated
    /// <c>a:ln/a:noFill</c> means no gridline at all, which is how a chart turns one off without
    /// removing the element.
    /// </remarks>
    private static Colour? GridOf(XElement? axis, DrawingTheme? theme)
    {
        if (Child(axis, "majorGridlines") is not { } grid) return null;

        XElement? properties = Child(grid, "spPr");
        if (Drawing.Child(Drawing.Child(properties, "ln"), "noFill") is not null) return null;

        return LineOf(properties, theme) ?? DefaultGrid;
    }

    /// <summary>
    /// An axis' number format, or null when it states none or states <c>General</c>.
    /// </summary>
    /// <remarks>
    /// <c>c:numFmt/@formatCode</c>. <c>General</c> reads as null rather than as a format code
    /// because that is what it means: <c>ObjectFormatter::convertNumberFormat</c> asks the number
    /// formats supplier for its standard index instead of converting the string
    /// (<c>oox/source/drawingml/chart/objectformatter.cxx:1132</c>). <c>@sourceLinked</c> is not
    /// consulted here — the source's own format is a cell format in a workbook this reader cannot
    /// reach, and what the axis states is the only thing available; LibreOffice reaches the same
    /// place for an axis anyway, its own comment recording that "Setting
    /// LinkNumberFormatToSource does not really work, at least not for axis".
    /// </remarks>
    /// <summary>
    /// Whether an axis is drawn — <c>c:delete val="1"</c> says it is not.
    /// </summary>
    /// <remarks>
    /// An absent axis is drawn, which is what a chart part with no <c>c:catAx</c> at all means for
    /// a pie; an absent <c>c:delete</c> is also drawn, because the schema's default is
    /// <c>false</c>. So the only thing that hides one is an explicit <c>1</c>.
    /// </remarks>
    private static bool Shown(XElement? axis)
        => axis is null || Number(Child(axis, "delete")) is not 1.0;

    private static NumberFormatCode? FormatOf(XElement? axis)
    {
        if (Drawing.Attribute(Child(axis, "numFmt"), "formatCode") is not { Length: > 0 } code)
            return null;

        if (string.Equals(code, "General", StringComparison.OrdinalIgnoreCase)) return null;

        NumberFormatCode parsed = NumberFormatCode.Parse(code);
        return parsed.IsGeneral ? null : parsed;
    }

    /// <summary>
    /// What an axis states about how its labels are set.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A rotation outside ±90° reads as none at all.</strong>
    /// <c>ObjectFormatter::convertTextRotation</c> throws away anything outside
    /// <c>[-5400000, 5400000]</c> — "MS Office UI allows values only in range of [-90,90]" —
    /// before negating and normalising into <c>[0, 360)</c>
    /// (<c>oox/source/drawingml/chart/objectformatter.cxx:1085-1093</c>). Both
    /// <c>bnc889755.pptx</c> and <c>tdf106217.pptx</c> state <c>rot="-60000000"</c>, which is a
    /// thousand degrees and reads as zero — so their labels are turned by the layout and not by
    /// the file, which is the whole point of the exercise and is invisible if the clamp is missed.
    /// </para>
    /// <para>
    /// The other three follow from the same attribute in
    /// <c>AxisConverter::convertFromModel</c> (<c>axisconverter.cxx:348-368</c>): overlap is
    /// allowed only where the file states a rotation of exactly zero, wrapping is allowed unless a
    /// non-zero rotation is in force, and staggering is turned off outright — "do not stagger
    /// labels in two lines" — which is why an OOXML axis rotates where an ODF one might stagger.
    /// </para>
    /// <para>
    /// <strong>A <c>c:dateAx</c> gets none of that, and it is the difference between two decks
    /// that look identical.</strong> Those three lines live in the <c>else</c> of a test on
    /// <c>bDateAxis</c> (<c>axisconverter.cxx:348</c>), so a date axis keeps chart2's own model
    /// defaults instead — no overlap, <em>no</em> wrapping, arrangement automatic
    /// (<c>chart2/source/model/main/Axis.cxx:239-242</c>). Wrapping off is what lets a date axis
    /// turn its labels 45° the moment they collide, where a category axis must first find a label
    /// that does not fit even broken. <c>bnc889755.pptx</c> and <c>tdf106217.pptx</c> state the
    /// same out-of-range rotation, hold labels of much the same width, and reach the same 45° by
    /// two different routes — the first because it is a <c>c:dateAx</c>, the second because
    /// "Netherlands" is one word too wide for its slot.
    /// </para>
    /// </remarks>
    private static ChartAxisText AxisTextOf(XElement? axis)
    {
        XElement? body = Drawing.Child(Child(axis, "txPr"), "bodyPr");
        int? stated = Drawing.Number(body, "rot");

        double rotation = stated is { } turns and >= -5400000 and <= 5400000
            ? -turns / 60000.0
            : 0.0;

        rotation -= 360.0 * Math.Floor(rotation / 360.0);

        bool date = axis is not null && Is(axis, "dateAx");

        return new ChartAxisText(
            rotation * Math.PI / 180.0,
            OverlapAllowed: !date && stated is 0,
            LineBreakAllowed: !date && rotation is 0.0 or 90.0 or 270.0,
            Stagger: date ? ChartLabelStagger.Auto : ChartLabelStagger.SideBySide);
    }

    /// <summary>
    /// The data table under the plot, or null when the chart has none.
    /// </summary>
    /// <remarks>
    /// All four flags default to <c>false</c> here and not to <c>!bMSO2007Doc</c>: unlike the
    /// <c>c:show*</c> family beside them, <c>DataTableContext</c> reads each as
    /// <c>getBool(XML_val, false)</c> and <c>DataTableModel</c> initialises each to false
    /// (<c>oox/source/drawingml/chart/datatablecontext.cxx:48-62</c>).
    /// </remarks>
    private static ChartDataTable? DataTableOf(XElement? table, DrawingTheme? theme)
        => table is null
            ? null
            : new ChartDataTable(
                Flag(table, "showHorzBorder") ?? false,
                Flag(table, "showVertBorder") ?? false,
                Flag(table, "showOutline") ?? false,
                Flag(table, "showKeys") ?? false,
                LineOf(Child(table, "spPr"), theme) ?? DefaultGrid);

    /// <summary>chart2's own gridline colour, gray30.</summary>
    private static readonly Colour DefaultGrid = Colour.FromRgb(0xB3B3B3);

    /// <summary>What one axis states about its scale.</summary>
    private static ChartScaleRequest ScaleOf(XElement? axis)
    {
        if (axis is null) return default;

        XElement? scaling = Child(axis, "scaling");

        return new ChartScaleRequest(
            Number(Child(scaling, "min")),
            Number(Child(scaling, "max")),
            Number(Child(axis, "majorUnit")),
            Value(Child(scaling, "orientation")) == "maxMin");
    }

    /// <summary>
    /// Which <c>c:*Ax</c> plays which role, and which plot group is measured against which.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A chart part names its axes by number and not by position.</strong> Every plot
    /// group lists a pair (or a triple, in 3-D) of <c>c:axId</c>, and every axis states its own
    /// <c>c:axId</c>; the pairing is what says which value axis a group is drawn against. Taking
    /// "the first <c>c:valAx</c>" instead is right only for a chart with one, and a chart with two
    /// is exactly the case the secondary axis exists for.
    /// </para>
    /// <para>
    /// <strong>A scatter chart has two <c>c:valAx</c> and no secondary axis.</strong> Both its
    /// dimensions are numeric, so the vocabulary spells the X axis <c>c:valAx</c> too and the two
    /// are told apart by <c>c:crossAx</c>: the X axis is the one the <em>other</em> axis crosses,
    /// and it is the one whose id appears first in the group's <c>c:axId</c> list
    /// (<c>oox/source/drawingml/chart/typegroupconverter.cxx</c> pairs them in that order).
    /// Reading the second as a secondary axis draws a chart with two value axes and no X scale at
    /// all, which is the trap this type costs an hour to.
    /// </para>
    /// </remarks>
    private sealed class ChartAxes
    {
        /// <summary>The primary value axis, or null.</summary>
        public XElement? Value { get; private init; }

        /// <summary>The secondary value axis, or null when there is one scale.</summary>
        public XElement? Secondary { get; private init; }

        /// <summary>The category or date axis, or null.</summary>
        public XElement? Category { get; private init; }

        /// <summary>A scatter chart's X axis, or null for a category chart.</summary>
        public XElement? Domain { get; private init; }

        private readonly Dictionary<XElement, int> _byGroup = [];

        /// <summary>Which value axis a plot group is measured against: 0 or 1.</summary>
        public int IndexOf(XElement group) => _byGroup.GetValueOrDefault(group, 0);

        public static ChartAxes Read(XElement plotArea, List<XElement> groups)
        {
            List<XElement> value = [];
            XElement? category = null;

            foreach (XElement axis in plotArea.Elements())
            {
                if (axis.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;

                switch (axis.Name.LocalName)
                {
                    case "valAx": value.Add(axis); break;
                    case "catAx" or "dateAx" or "serAx": category ??= axis; break;
                    default: break;
                }
            }

            // A scatter chart is the two-valAx case that is not a secondary axis. Its groups list
            // the X axis' id first, so the axis matching that id is the domain and the other is
            // the value axis.
            bool scatter = category is null && value.Count >= 2 && groups.Count > 0;

            ChartAxes axes = new()
            {
                Category = category,
                Domain = scatter ? Matching(value, First(groups[0])) ?? value[0] : null,
            };

            List<XElement> remaining = [];
            foreach (XElement axis in value)
                if (!ReferenceEquals(axis, axes.Domain)) remaining.Add(axis);

            if (remaining.Count == 0) remaining = value;

            ChartAxes resolved = new()
            {
                Category = category,
                Domain = axes.Domain,
                Value = remaining.Count > 0 ? remaining[0] : null,
                Secondary = remaining.Count > 1 ? remaining[1] : null,
            };

            if (resolved.Secondary is { } second && IdOf(second) is { } secondId)
            {
                foreach (XElement group in groups)
                {
                    foreach (XElement id in Children(group, "axId"))
                    {
                        if (Value(id) != secondId) continue;
                        resolved._byGroup[group] = 1;
                        break;
                    }
                }
            }

            return resolved;
        }

        private static string? First(XElement group)
        {
            foreach (XElement id in Children(group, "axId")) return Value(id);
            return null;
        }

        private static string? IdOf(XElement axis) => Value(Child(axis, "axId"));

        private static XElement? Matching(List<XElement> axes, string? id)
        {
            if (id is null) return null;

            foreach (XElement axis in axes)
                if (IdOf(axis) == id) return axis;

            return null;
        }
    }

    private static ChartLegendPosition LegendOf(XElement? legend)
        => legend is null
            ? ChartLegendPosition.None
            : Value(Child(legend, "legendPos")) switch
            {
                "l" => ChartLegendPosition.Left,
                "t" => ChartLegendPosition.Top,
                "b" => ChartLegendPosition.Bottom,
                "tr" => ChartLegendPosition.Right,

                // c:legendPos is optional and its default is "r", so a c:legend with nothing in
                // it still draws a legend on the right.
                _ => ChartLegendPosition.Right,
            };

    private static (List<ChartSeries> Series, string?[] Categories) ReadSeries(
        XElement group, ChartPlotKind kind, DrawingTheme? theme, int axisIndex, bool office2007)
    {
        List<ChartSeries> series = [];
        string?[] categories = [];

        // c:scatterStyle decides whether a scatter series draws its line, its markers or both.
        // "marker" alone is the case that matters: drawing the line and not the markers leaves an
        // empty plot area, because the file asked for no line.
        string? scatterStyle = Value(Child(group, "scatterStyle"));
        bool scatterLine = kind != ChartPlotKind.Scatter || scatterStyle != "marker";
        string? radarStyle = Value(Child(group, "radarStyle"));

        // A group's own c:dLbls is the default every series in it inherits.
        ChartDataLabel? groupLabel = LabelOf(Child(group, "dLbls"), null, kind, office2007);

        // Which of a stock plot's four numbers each of its series carries, by position. Four
        // series are open, high, low, close and three are high, low, close — which is
        // TypeGroupConverter's own "int nRoleIdx = (aSeries.size() == 3) ? 1 : 0" over the roles
        // values-first, values-max, values-min, values-last
        // (oox/source/drawingml/chart/typegroupconverter.cxx:517-527). ODF orders the middle pair
        // the other way round; see ChartStockRole.
        ChartStockRole[] stockRoles =
        [
            ChartStockRole.Open, ChartStockRole.High, ChartStockRole.Low, ChartStockRole.Close,
        ];

        int stockRole = kind != ChartPlotKind.Stock
            ? -1
            : Children(group, "ser").Count() == 3 ? 1 : 0;

        foreach (XElement element in Children(group, "ser"))
        {
            (string?[] labels, _) = ReadSequence(Child(element, "cat") ?? Child(element, "xVal"));
            if (categories.Length == 0 && labels.Length > 0) categories = labels;

            XElement? valueSource = Child(element, "val") ?? Child(element, "yVal");
            (_, double?[] numbers) = ReadSequence(valueSource);

            // The format the *data* carries, which is what a label showing a value falls back to
            // when it states none of its own — VSeriesPlotter's detectNumberFormatKey, which asks
            // the data sequence rather than the axis. Measured on tdf105517.pptx: its one visible
            // label reads 220,000 in the reference and 220000 without this, the grouping coming
            // from a c:formatCode of "#,##0" inside the c:numCache and from nowhere else.
            NumberFormatCode? sourceFormat = CacheFormat(valueSource);

            double?[]? domain = null;
            if (kind is ChartPlotKind.Scatter or ChartPlotKind.Bubble
                && Child(element, "xVal") is { } xVal)
            {
                (_, double?[] xs) = ReadSequence(xVal);
                if (xs.Length > 0) domain = xs;
            }

            // The bubble's third dimension. c:bubbleSize is a sequence like any other and is the
            // only thing that makes a bubble chart more than a scatter chart with round markers.
            double?[]? sizes = null;
            if (kind == ChartPlotKind.Bubble && Child(element, "bubbleSize") is { } bubbleSize)
            {
                (_, double?[] read) = ReadSequence(bubbleSize);
                if (read.Length > 0) sizes = read;
            }

            XElement? properties = Child(element, "spPr");
            XElement? seriesLabels = Child(element, "dLbls");

            series.Add(new ChartSeries(
                DrawingChartText.Label(Child(element, "tx")),
                numbers,
                FillOf(properties, theme),
                LineOf(properties, theme),
                LineWidthOf(properties),
                PointFills(element, numbers.Length, theme),
                kind)
            {
                XValues = domain,
                Marker = MarkerOf(Child(element, "marker"), kind, scatterStyle, radarStyle),
                HasLine = scatterLine
                          && Drawing.Child(Drawing.Child(properties, "ln"), "noFill") is null,
                Label = WithSource(LabelOf(seriesLabels, groupLabel, kind, office2007), sourceFormat),
                PointLabels = PointLabelsOf(
                    seriesLabels, numbers.Length, groupLabel, kind, sourceFormat, office2007),
                AxisIndex = axisIndex,
                Trendlines = TrendlinesOf(element, theme, office2007),
                SizeValues = sizes,
                InvertIfNegative = Flag(element, "invertIfNegative") ?? false,
                StockRole = stockRole >= 0 && stockRole < stockRoles.Length
                    ? stockRoles[stockRole]
                    : ChartStockRole.None,
            });

            if (stockRole >= 0) stockRole++;
        }

        return (series, categories);
    }

    /// <summary>
    /// The trendlines a series carries, or null when it carries none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An unstated <c>c:dispEq</c> or <c>c:dispRSqr</c> means "show it".</strong>
    /// <c>TrendlineModel</c>'s constructor is <c>mbDispEquation( !bMSO2007Doc )</c>
    /// (<c>oox/source/drawingml/chart/seriesmodel.cxx:86-92</c>) and
    /// <c>TrendlineContext</c> reads each flag as <c>getBool( XML_val, !bMSO2007Doc )</c>
    /// (<c>seriescontext.cxx:307-312</c>). It is the same rule the five data-label flags follow,
    /// and it is the reason both are stated here as <c>?? true</c> rather than <c>?? false</c>:
    /// the file Excel writes when it means "no equation" carries an explicit <c>val="0"</c>.
    /// </para>
    /// <para>
    /// <c>c:intercept</c> is <em>presence</em> and not a value —
    /// <c>ForceIntercept</c> is <c>mfIntercept.has_value()</c> — so a stated intercept of zero
    /// forces the fit through the origin where an absent one leaves it free.
    /// </para>
    /// </remarks>
    private static List<ChartTrendline>? TrendlinesOf(
        XElement series, DrawingTheme? theme, bool office2007)
    {
        List<ChartTrendline>? trendlines = null;

        foreach (XElement element in Children(series, "trendline"))
        {
            XElement? properties = Child(element, "spPr");

            trendlines ??= [];
            trendlines.Add(new ChartTrendline
            {
                Kind = TrendlineKindOf(Value(Child(element, "trendlineType"))),
                Order = Drawing.Number(Child(element, "order"), "val") ?? 2,
                Period = Drawing.Number(Child(element, "period"), "val") ?? 2,
                Forward = Real(Child(element, "forward")) ?? 0.0,
                Backward = Real(Child(element, "backward")) ?? 0.0,
                Intercept = Child(element, "intercept") is { } intercept
                    ? Real(intercept) ?? 0.0
                    : null,
                ShowEquation = Flag(element, "dispEq") ?? !office2007,
                ShowRSquared = Flag(element, "dispRSqr") ?? !office2007,
                Name = Child(element, "name")?.Value,
                Line = LineOf(properties, theme),
                LineWidth = LineWidthOf(properties),
            });
        }

        return trendlines;
    }

    /// <summary>The six spellings of <c>c:trendlineType</c>.</summary>
    /// <remarks>
    /// <c>TrendlineConverter::convertFromModel</c> maps each to a
    /// <c>com.sun.star.chart2.*RegressionCurve</c> service
    /// (<c>oox/source/drawingml/chart/seriesconverter.cxx:684-706</c>); the default when the
    /// element is absent is <c>linear</c>, as <c>TrendlineModel</c>'s constructor states.
    /// </remarks>
    private static ChartTrendlineKind TrendlineKindOf(string? stated) => stated switch
    {
        "poly" => ChartTrendlineKind.Polynomial,
        "exp" => ChartTrendlineKind.Exponential,
        "log" => ChartTrendlineKind.Logarithmic,
        "power" => ChartTrendlineKind.Power,
        "movingAvg" => ChartTrendlineKind.MovingAverage,
        _ => ChartTrendlineKind.Linear,
    };

    /// <summary>A <c>@val</c> read as a real number, or null when the element states none.</summary>
    private static double? Real(XElement? element)
        => Value(element) is { } text
           && double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double v)
            ? v
            : null;

    /// <summary>
    /// What marker a series draws, or none.
    /// </summary>
    /// <remarks>
    /// <c>c:marker/c:symbol</c>. Absent means <c>auto</c> for a scatter chart — which draws one —
    /// and none for a line chart, which is the asymmetry that makes a scatter chart look empty if
    /// it is treated like a line chart with no markers stated. <c>c:scatterStyle val="line"</c> or
    /// <c>"smooth"</c> turns them off again, and <c>c:radarStyle val="marker"</c> turns them on
    /// for a radar chart the same way — which is the whole difference between that style and
    /// <c>standard</c>, both of which draw a stroked polygon.
    /// </remarks>
    private static ChartMarker MarkerOf(
        XElement? marker, ChartPlotKind kind, string? scatterStyle, string? radarStyle)
    {
        string? symbol = Value(Child(marker, "symbol"));

        if (symbol is null)
        {
            bool automatic =
                (kind == ChartPlotKind.Scatter && scatterStyle is not ("line" or "smooth"))
                || (kind == ChartPlotKind.Radar && radarStyle == "marker");

            return automatic ? ChartMarker.Square : ChartMarker.None;
        }

        return symbol switch
        {
            "none" => ChartMarker.None,
            "circle" => ChartMarker.Circle,
            "diamond" => ChartMarker.Diamond,
            "triangle" => ChartMarker.Triangle,
            "x" => ChartMarker.Star,
            "plus" => ChartMarker.Cross,
            "star" => ChartMarker.Star,
            _ => ChartMarker.Square,
        };
    }

    /// <summary>
    /// One level of <c>c:dLbls</c>, resolved against the level above it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An unstated flag means "true", not "false" — unless Office 2007 wrote the
    /// file.</strong> <c>SeriesConverter::convertDataLabel</c> reads each of the five as
    /// <c>value_or( !bMSO2007Doc )</c> (<c>seriesconverter.cxx:139-144</c>) — so on anything but a
    /// file Office 2007 wrote, a <c>c:dLbls</c> that states nothing shows everything. The
    /// ubiquitous "no labels" form Excel writes is not silence but six explicit zeroes, which is
    /// why defaulting to false looks right on every file that has them and loses every label on
    /// the files that do not.
    /// </para>
    /// <para>
    /// Office 2007 is the exception, and it is not a rare one: it wrote a bare
    /// <c>&lt;c:dLbls/&gt;</c> to mean "no labels at all", so reading that as "show everything"
    /// prints the category, the value and the series name beside every point of every series.
    /// Measured on <c>171128IPAP.pptx</c>, whose nine line charts each carry an empty
    /// <c>c:dLbls</c> over 42 quarters: 11026 words drawn against a reference's 4705, the series
    /// name "Manufacturing" alone appearing 349 times. Office 2007 also leaves the label
    /// settings of a data point alone when the point states none of the seven elements, which is
    /// what the <c>stated</c> test below reproduces —
    /// <c>lclConvertLabelFormatting</c>'s <c>bHasAnyElement</c>.
    /// </para>
    /// <para>
    /// <c>c:delete val="1"</c> is the other spelling of "nothing here", and it wins over the
    /// inherited level rather than falling through to it.
    /// </para>
    /// </remarks>
    private static ChartDataLabel? LabelOf(
        XElement? labels, ChartDataLabel? inherited, ChartPlotKind kind, bool office2007)
    {
        if (labels is null) return inherited;

        // A deleted label is an empty label and not an absent one. Returning null here made
        // ChartSeries.LabelAt fall back to the series' label for exactly the points the file had
        // switched off — tdf105517.pptx deletes ten of a series' eleven and the eleventh is the
        // only label the reference draws.
        if (Number(Child(labels, "delete")) is 1.0) return Deleted;

        // An Office 2007 c:dLbls that states none of the seven settings states nothing at all,
        // and leaves whatever it inherited exactly as it was.
        if (office2007 && !StatesLabelSetting(labels)) return inherited;

        bool shown = !office2007;
        bool value = Flag(labels, "showVal") ?? inherited?.ShowValue ?? shown;

        // A percentage is a pie's business and nobody else's: bShowPercent is ANDed with
        // meTypeCategory == TYPECATEGORY_PIE (seriesconverter.cxx:141). Honouring it on a column
        // chart puts a second number on every bar of several corpus decks.
        bool percent = kind == ChartPlotKind.Pie
                       && (Flag(labels, "showPercent") ?? inherited?.ShowPercent ?? shown);
        bool category = Flag(labels, "showCatName") ?? inherited?.ShowCategory ?? shown;
        bool name = Flag(labels, "showSerName") ?? inherited?.ShowSeries ?? shown;

        // The stated format goes to whichever of the two properties the label will use, which is
        // the percentage one whenever a percentage is shown and the format is not source-linked.
        XElement? numFmt = Child(labels, "numFmt");
        string? code = Drawing.Attribute(numFmt, "formatCode");
        bool sourceLinked = Drawing.Attribute(numFmt, "sourceLinked") is "1" or "true";
        bool asPercent = percent && !sourceLinked && code is { Length: > 0 };
        bool general = code is null
                       || string.Equals(code, "General", StringComparison.OrdinalIgnoreCase);

        NumberFormatCode? Parsed(string? text)
            => text is { Length: > 0 } ? NumberFormatCode.Parse(text) : null;

        string? separator = Child(labels, "separator")?.Value;
        List<ChartLabelPart>? custom = CustomLabel(Child(Child(labels, "tx"), "rich"));

        return new ChartDataLabel
        {
            ShowValue = value,
            ShowPercent = percent,
            ShowCategory = category,
            ShowSeries = name,
            ValueFormat = asPercent || general
                ? inherited?.ValueFormat
                : Parsed(code),
            PercentFormat = asPercent
                ? Parsed(general ? "0%" : code)
                : inherited?.PercentFormat,

            // "; " unless a percentage is shown without a value, which Office writes on its own
            // line (seriesconverter.cxx:168-172).
            Separator = separator ?? (percent && !value ? "\n" : inherited?.Separator ?? "; "),
            Placement = PlacementOf(Value(Child(labels, "dLblPos"))) ?? inherited?.Placement,

            // TitleText takes the element that *holds* a c:tx, not the c:tx itself. Handing it
            // the child instead silently returned null for every custom label in the corpus —
            // CustomDataLabel_tdf115107.pptx draws five of them and none appeared.
            Text = custom is null ? TitleText(labels) ?? inherited?.Text : null,
            Parts = custom ?? inherited?.Parts,
        };
    }

    /// <summary>The per-point labels a <c>c:dLbls</c> overrides, or null when it overrides none.</summary>
    private static readonly ChartDataLabel Deleted = new();

    /// <summary>
    /// Whether a <c>c:dLbls</c> or <c>c:dLbl</c> states any of the seven settings Office 2007
    /// treats as "this element says something" — <c>lclConvertLabelFormatting</c>'s
    /// <c>bHasAnyElement</c> (<c>seriesconverter.cxx:130-137</c>).
    /// </summary>
    private static bool StatesLabelSetting(XElement labels)
        => Child(labels, "separator") is not null
           || Child(labels, "dLblPos") is not null
           || Child(labels, "showVal") is not null
           || Child(labels, "showCatName") is not null
           || Child(labels, "showSerName") is not null
           || Child(labels, "showPercent") is not null
           || Child(labels, "showLegendKey") is not null;

    private static ChartDataLabel?[]? PointLabelsOf(
        XElement? labels,
        int count,
        ChartDataLabel? inherited,
        ChartPlotKind kind,
        NumberFormatCode? source,
        bool office2007)
    {
        if (labels is null) return null;

        ChartDataLabel? seriesLevel = LabelOf(labels, inherited, kind, office2007);
        ChartDataLabel?[]? points = null;

        foreach (XElement point in Children(labels, "dLbl"))
        {
            int index = Drawing.Number(Child(point, "idx"), "val") ?? -1;
            if (index < 0 || index >= MaxPointCount) continue;

            points ??= new ChartDataLabel?[Math.Max(count, index + 1)];
            if (index >= points.Length) continue;

            points[index] = WithSource(LabelOf(point, seriesLevel, kind, office2007), source);
        }

        return points;
    }

    /// <summary>
    /// A custom label's runs, or null when the label states no template.
    /// </summary>
    /// <remarks>
    /// A <c>c:rich</c> whose runs are all literal is left to <see cref="TitleText"/>, because a
    /// plain string needs no resolution; only a body holding at least one <c>a:fld</c> becomes a
    /// template. A field's <c>@type</c> is what says which value it stands for — its own
    /// <c>a:t</c> is a localised placeholder such as <c>[WARTOŚĆ]</c>, and drawing that verbatim
    /// is what five of <c>CustomDataLabel_tdf115107.pptx</c>'s labels did before this.
    /// </remarks>
    private static List<ChartLabelPart>? CustomLabel(XElement? rich)
    {
        if (rich is null) return null;

        List<ChartLabelPart> parts = [];
        bool anyField = false;
        bool first = true;

        foreach (XElement paragraph in rich.Elements(Drawing.Name("p")))
        {
            if (!first) parts.Add(new ChartLabelPart(ChartLabelField.NewLine, "\n"));
            first = false;

            foreach (XElement run in paragraph.Elements())
            {
                if (run.Name.NamespaceName != OoxmlNamespaces.DrawingML) continue;

                switch (run.Name.LocalName)
                {
                    case "r":
                        parts.Add(new ChartLabelPart(
                            ChartLabelField.Literal, run.Element(Drawing.Name("t"))?.Value ?? ""));
                        break;

                    case "br":
                        parts.Add(new ChartLabelPart(ChartLabelField.NewLine, "\n"));
                        break;

                    case "fld":
                    {
                        string text = run.Element(Drawing.Name("t"))?.Value ?? "";
                        ChartLabelField field = run.Attribute("type")?.Value switch
                        {
                            "VALUE" => ChartLabelField.Value,
                            "CATEGORYNAME" => ChartLabelField.Category,
                            "SERIESNAME" => ChartLabelField.Series,
                            "PERCENTAGE" => ChartLabelField.Percentage,
                            "CELLRANGE" => ChartLabelField.CellRange,
                            _ => ChartLabelField.Literal,
                        };

                        // A CELLREF field is a placeholder LibreOffice itself does not resolve
                        // ("TODO: for now doesn't show placeholder", VSeriesPlotter.cxx:541), so
                        // it contributes nothing rather than its own bracketed name.
                        if (field == ChartLabelField.Literal
                            && run.Attribute("type")?.Value is "CELLREF")
                        {
                            anyField = true;
                            break;
                        }

                        if (field != ChartLabelField.Literal) anyField = true;
                        parts.Add(new ChartLabelPart(field, text));
                        break;
                    }

                    default: break;
                }
            }
        }

        return anyField ? parts : null;
    }

    /// <summary>A label given the data's own format where it states none.</summary>
    private static ChartDataLabel? WithSource(ChartDataLabel? label, NumberFormatCode? source)
        => label is null || source is null || label.ValueFormat is not null
            ? label
            : label with { ValueFormat = source };

    /// <summary>
    /// The format code a cached numeric sequence carries, or null.
    /// </summary>
    /// <remarks><c>c:numCache/c:formatCode</c>, an element rather than an attribute.</remarks>
    private static NumberFormatCode? CacheFormat(XElement? source)
    {
        XElement? cache = Child(Child(source, "numRef"), "numCache") ?? Child(source, "numLit");
        if (Child(cache, "formatCode")?.Value is not { Length: > 0 } code) return null;
        if (string.Equals(code, "General", StringComparison.OrdinalIgnoreCase)) return null;

        NumberFormatCode parsed = NumberFormatCode.Parse(code);
        return parsed.IsGeneral ? null : parsed;
    }

    private static ChartLabelPlacement? PlacementOf(string? stated) => stated switch
    {
        "outEnd" => ChartLabelPlacement.Outside,
        "inEnd" => ChartLabelPlacement.Inside,
        "ctr" => ChartLabelPlacement.Centre,
        "inBase" => ChartLabelPlacement.NearOrigin,
        "t" => ChartLabelPlacement.Top,
        "b" => ChartLabelPlacement.Bottom,
        "l" => ChartLabelPlacement.Left,
        "r" => ChartLabelPlacement.Right,
        "bestFit" => ChartLabelPlacement.BestFit,
        _ => null,
    };

    private static bool? Flag(XElement? parent, string localName)
        => Value(Child(parent, localName)) switch
        {
            "1" or "true" => true,
            "0" or "false" => false,
            _ => null,
        };

    /// <summary>
    /// The per-point fills a series states, or null when it states none.
    /// </summary>
    /// <remarks>
    /// <c>c:dPt</c>, each carrying a <c>c:idx</c> and its own <c>c:spPr</c>. Only a pie normally
    /// has them, and without them every wedge is the series' one colour — which reads as a broken
    /// renderer rather than as an unread element.
    /// </remarks>
    private static Colour?[]? PointFills(XElement series, int count, DrawingTheme? theme)
    {
        Colour?[]? fills = null;

        foreach (XElement point in Children(series, "dPt"))
        {
            int index = Drawing.Number(Child(point, "idx"), "val") ?? -1;
            if (index < 0 || index >= Math.Max(count, MaxPointCount)) continue;
            if (FillOf(Child(point, "spPr"), theme) is not { } fill) continue;

            fills ??= new Colour?[Math.Max(count, index + 1)];
            if (index >= fills.Length) continue;
            fills[index] = fill;
        }

        return fills;
    }

    /// <summary>A shape property bag's solid fill, or null when it has none.</summary>
    /// <remarks>
    /// Only <c>a:solidFill</c>. A gradient or picture fill on a bar is legal and rare; drawing
    /// nothing for one leaves the bar's outline, which reads as "something is here and it is not
    /// coloured in" rather than as a missing bar.
    /// </remarks>
    private static Colour? FillOf(XElement? properties, DrawingTheme? theme)
    {
        XElement? fill = Drawing.Child(properties, "solidFill");
        if (fill is null) return null;

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

    /// <summary>
    /// A line's width, or zero when it states none.
    /// </summary>
    /// <remarks>
    /// Zero is not "no line": <c>a:ln w="0"</c> is what LibreOffice's own export writes for a
    /// hairline, and it appears on every bar of the corpus chart. The renderer draws a zero-width
    /// stroke as the thinnest the device can, which is what the reference PDF does with
    /// <c>0 w</c>.
    /// </remarks>
    private static Length LineWidthOf(XElement? properties)
    {
        XElement? line = Drawing.Child(properties, "ln");
        return Drawing.Number(line, "w") is { } emu && emu > 0
            ? Length.FromEmu(emu)
            : Length.Zero;
    }

    /// <summary>
    /// The size a titled element's text is set at, from the first <c>a:defRPr</c> or
    /// <c>a:rPr</c> under it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>@sz</c> is in hundredths of a point — <c>sz="1300"</c> is thirteen points — and it may
    /// sit on either the paragraph's default run properties or an individual run's. Taking the
    /// first of either in document order gets the common case, which is a chart whose title is
    /// one run and states the same size in both places.
    /// </para>
    /// <para>
    /// This is read because it decides layout, not appearance. The main title's height is
    /// subtracted from the top of the chart before the plot area is placed, so assuming ten
    /// points where the file says thirteen puts the plot area — and therefore every bar's base —
    /// several points too high.
    /// </para>
    /// </remarks>
    private static Length? SizeOf(XElement? element)
    {
        if (element is null) return null;

        foreach (XElement properties in element.Descendants())
        {
            if (properties.Name.NamespaceName != OoxmlNamespaces.DrawingML) continue;
            if (properties.Name.LocalName is not ("defRPr" or "rPr")) continue;
            if (Drawing.Number(properties, "sz") is not { } hundredths || hundredths <= 0) continue;

            return Length.FromPoints(hundredths / 100.0);
        }

        return null;
    }

    private static Length? AxisTitleSizeOf(XElement plotArea)
    {
        foreach (XElement axis in plotArea.Elements())
        {
            if (axis.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!axis.Name.LocalName.EndsWith("Ax", StringComparison.Ordinal)) continue;
            if (SizeOf(Child(axis, "title")) is { } size) return size;
        }

        return null;
    }

    /// <summary>The size the axis <em>labels</em> are set at — <c>c:txPr</c>, not the title's.</summary>
    private static Length? AxisLabelSizeOf(XElement plotArea)
    {
        foreach (XElement axis in plotArea.Elements())
        {
            if (axis.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!axis.Name.LocalName.EndsWith("Ax", StringComparison.Ordinal)) continue;
            if (SizeOf(Child(axis, "txPr")) is { } size) return size;
        }

        return null;
    }

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

        return DrawingChartText.Label(tx);
    }

    /// <summary>Reads a cached data sequence, sparse indices and all.</summary>
    /// <remarks>
    /// The same rule <see cref="DrawingChart"/> documents at length: the array is sized from
    /// <c>c:ptCount</c> and every point placed at its own <c>@idx</c>, because the indices skip
    /// blanks and reading in document order slides every later value onto the wrong category. A
    /// chart drawn that way has the right bars against the wrong labels and looks entirely
    /// plausible.
    /// </remarks>
    private static (string?[] Text, double?[] Numbers) ReadSequence(XElement? source)
    {
        if (source is null) return ([], []);

        XElement? cache =
            Child(Child(source, "strRef"), "strCache")
            ?? Child(source, "strLit")
            ?? Child(Child(source, "numRef"), "numCache")
            ?? Child(source, "numLit")
            ?? Child(Child(source, "multiLvlStrRef"), "multiLvlStrCache");

        if (cache is null) return ([], []);

        int declared = Drawing.Number(Child(cache, "ptCount"), "val") ?? -1;
        if (declared < 0)
        {
            foreach (XElement point in cache.Descendants(Name("pt")))
                declared = Math.Max(declared, (Drawing.Number(point, "idx") ?? -1) + 1);
        }

        int count = Math.Clamp(declared, 0, MaxPointCount);
        string?[] text = new string?[count];
        double?[] numbers = new double?[count];

        foreach (XElement point in cache.Descendants(Name("pt")))
        {
            int index = Drawing.Number(point, "idx") ?? -1;
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

    private static double? Number(XElement? element)
    {
        string? value = Drawing.Attribute(element, "val");
        return double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
            ? parsed
            : null;
    }

    private static string? Value(XElement? element) => Drawing.Attribute(element, "val");

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

/// <summary>
/// The one piece of chart-part text reading both chart readers need.
/// </summary>
/// <remarks>
/// Factored out rather than duplicated because a series' label is the one thing the content
/// reader and the drawing reader genuinely share, and because getting it wrong in one of them
/// produces a chart whose legend and whose table disagree about what a series is called.
/// </remarks>
internal static class DrawingChartText
{
    /// <summary>
    /// A series' or title's label, from a <c>c:tx</c>-shaped element.
    /// </summary>
    /// <remarks>
    /// <c>CT_SerTx</c> is a choice of <c>c:strRef</c> and a bare <c>c:v</c>, and a reference's
    /// cache may hold several points when the label spans cells. LibreOffice joins those with one
    /// space and keeps a single label — "the internal data table does not support complex labels"
    /// (<c>oox/source/drawingml/chart/datasourceconverter.cxx:50-73</c>).
    /// </remarks>
    internal static string? Label(XElement? source)
    {
        if (source is null) return null;

        XName v = XName.Get("v", OoxmlNamespaces.DrawingMLChart);
        if (source.Element(v) is { } literal && literal.Value.Length > 0) return literal.Value;

        System.Text.StringBuilder joined = new();
        XName pt = XName.Get("pt", OoxmlNamespaces.DrawingMLChart);

        foreach (XElement point in source.Descendants(pt))
        {
            if (point.Element(v)?.Value is not { Length: > 0 } value) continue;
            if (joined.Length > 0) joined.Append(' ');
            joined.Append(value);
        }

        return joined.Length == 0 ? null : joined.ToString();
    }
}

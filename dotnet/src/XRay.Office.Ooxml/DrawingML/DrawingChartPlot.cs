using System.Globalization;
using System.Text.RegularExpressions;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.Core.Units;

namespace XRay.Office.Ooxml.DrawingML;

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
    /// <summary>
    /// The line a chart space with no <c>a:ln</c> of its own is drawn with, outside Impress.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>LineFormatter</c>'s constructor (<c>oox/source/drawingml/chart/objectformatter.cxx:826-852</c>)
    /// gives every <c>OBJECTTYPE_CHARTSPACE</c> a solid line of
    /// <c>GraphicHelper::getDefaultChartAreaLineStyle()</c> at
    /// <c>getDefaultChartAreaLineWidth()</c> — 9525 EMU, 0.75 pt, "what MSO 2016 writes fixing
    /// incomplete MSO 2010 documents" — coloured <c>D9D9D9</c>, "what MSO 2016 use as a default
    /// color for chartspace border". tdf#81437 and tdf#82217.
    /// </para>
    /// <para>
    /// <strong>The Impress filter is the exception, not the rule</strong>, and reading it as the
    /// rule is what left this unimplemented for two rounds: the guard is
    /// <c>!aFilterName.startsWithIgnoreAsciiCase("Impress")</c> (tdf#150176), so a Calc or Writer
    /// chart gets the border and a slide's does not. Four blind readers across rounds 61 and 62
    /// reported it on three unrelated <em>spreadsheet</em> documents and <c>pdf-ops.py</c> agreed
    /// every time; the reference's own stroke on <c>023_Waterfall_Chart_Template_for_Excel</c> is
    /// at (68.17, 425.79)-(530.67, 755.77).
    /// </para>
    /// <para>
    /// A stated <c>a:ln</c> still wins, because <c>convertFormatting</c> assigns the automatic
    /// line first and the shape's own over it — and an <c>a:ln/a:noFill</c> is a line the file
    /// turns off, which is why <see cref="SuppressesLine"/> and not <see cref="LineOf"/> decides.
    /// </para>
    /// </remarks>
    private static readonly Colour AutomaticChartAreaLine = Colour.FromRgb(0xD9D9D9);

    /// <summary>
    /// <c>getDefaultChartAreaLineWidth()</c>'s 9525 EMU, as the reference keeps it — 26 hundredths
    /// of a millimetre, 0.73701 pt, and not the 0.75 the EMU spells.
    /// </summary>
    private static readonly Length AutomaticChartAreaLineWidth =
        DrawingChartAutoFormat.LineWidth(9525);

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
    /// <param name="styles">
    /// The theme's <c>a:fmtScheme</c>, for the width of the line an automatically-formatted
    /// series draws. Null when the caller has none, which leaves such a line at the hairline the
    /// reader would otherwise give it.
    /// </param>
    /// <param name="ranges">
    /// Resolves a sequence's <c>c:f</c> against the cells it names, when the caller has the
    /// workbook to resolve it in. Null — the default, and what the presentation and
    /// word-processing readers pass — keeps the cached points as the only source. See
    /// <see cref="ChartRangeResolver"/> for why the two differ.
    /// </param>
    /// <param name="automaticChartAreaLine">
    /// Whether a chart space that states no line of its own gets the automatic grey one. True for
    /// every host but Impress — see <see cref="ChartPlot.Border"/>. It defaults to the Impress
    /// answer because the exception is Impress's and because this reader's fixtures are
    /// presentations; the two hosts that want it pass it explicitly.
    /// </param>
    public static ChartPlot? Read(
        XElement chartSpace,
        DrawingTheme? theme = null,
        bool office2007 = false,
        DrawingStyleMatrix? styles = null,
        ChartRangeResolver? ranges = null,
        bool automaticChartAreaLine = false)
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

            if (matched == ChartPlotKind.OfPie && IsExploded(candidate))
                matched = ChartPlotKind.Pie;

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
        double?[] categoryValues = [];

        // The automatic-format context, which is a property of the whole chart space rather than
        // of any one group: the style index, and the largest c:ser/c:idx anywhere in the plot
        // area — which is what the accent cycle's shade/tint step divides by
        // (plotareaconverter.cxx:452-457).
        ChartAutoContext automatic = new(
            DrawingChartAutoFormat.StyleOf(chartSpace),
            MaximumSeriesIndex(groups),
            styles,
            groups.Count == 1 ? Flag(groups[0], "varyColors") : false,
            groups.Count == 1);

        // The categories belong to the primary axes set, and to the *first group of it*, which is
        // not always the first group in the part. `if (nAxesSetIdx == 0) aScaleData.Categories =
        // rTypeGroups.front()->createCategorySequence()`
        // (oox/source/drawingml/chart/axisconverter.cxx:282-290) — one set of categories per
        // coordinate system, taken from whichever axes set was converted as index 0.
        //
        // Which set that is comes from plotareaconverter.cxx:466-468: normally the first type
        // group's, but a *combined* chart — exactly two type groups of different kinds — puts the
        // set whose value axis comes first in the plot area's own element order at index 0
        // instead, whichever group wrote it. On 055_Project_timeline_with_milestones the
        // c:barChart is written first and its invisible position series carries the thirteen
        // milestone names, while the c:lineChart's c:dateAx pair is the primary one and carries
        // the seventeen dates the reference draws — on the axis and as every data label's
        // [CATEGORY NAME]. Three corpus chart parts in three documents are this shape;
        // ChartAxes.IndexOf already answers 0 and 1 correctly for all fifteen parts that hold
        // two axes sets at all. See probes/chart-secaxis/axesset-census.py.
        bool primaryCategories = false;

        for (int at = 0; at < groups.Count; at++)
        {
            int axisIndex = axes.IndexOf(groups[at]);

            (List<ChartSeries> read, string?[] labels, double?[] numbers) = ReadSeries(
                groups[at], kinds[at], theme, axisIndex, office2007, automatic, ranges);

            if (labels.Length > 0
                && (categories.Length == 0 || (axisIndex == 0 && !primaryCategories)))
            {
                categories = labels;
                categoryValues = numbers;
                primaryCategories = axisIndex == 0;
            }

            series.AddRange(read);
        }

        if (series.Count == 0) return null;

        // The colour every piece of the chart's text falls back to. One value for the whole
        // chart, because all three of ObjectFormatter's auto-text tables carry the same two
        // colours and differ only in size and weight.
        Colour automaticText =
            DrawingChartAutoFormat.TextColourOf(automatic.Style, theme) ?? Colour.Black;

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

        // A c:dateAx is a continuous serial scale and not a run of category slots, so it is
        // resolved here — before the plot is built — and the points are put in date order with it.
        IReadOnlyList<string?> orderedCategories = categories;
        IReadOnlyList<ChartSeries> orderedSeries = series;

        ChartDateAxis? dateAxis =
            DateAxisOf(chartSpace, axes.Category, axes.Crossing, categoryValues);
        if (dateAxis is not null)
        {
            (dateAxis, orderedCategories, orderedSeries) =
                ChartDateScale.SortByDate(dateAxis, orderedCategories, orderedSeries);
        }

        return new ChartPlot
        {
            DateAxis = dateAxis,
            // The automatic title LibreOffice substitutes when the part states an empty
            // <c:title> — or none at all and has not deleted it. See DrawingChartTitle, which
            // carries the rule, the corpus census and the four controls that measured it.
            Title = TitleText(Child(chart, "title"))
                    ?? DrawingChartTitle.Automatic(chart, office2007),
            // A scatter chart's horizontal axis is its domain and not its category axis, and its
            // title hangs off that element — so reading only c:catAx loses it entirely. The same
            // fallback CategoryAxisVisible already takes, and tdf127720.pptx is what shows it:
            // "Dissolved Oxygen (%)" is three words the reference draws and this did not.
            CategoryAxisTitle = AxisTitleText(
                Child(axes.Domain ?? axes.Category, "title"), kind),
            ValueAxisTitle = AxisTitleText(Child(axes.Value, "title"), kind),
            Categories = orderedCategories,
            // The rows a complex category axis is drawn as. The join above stays what it was,
            // because a legend entry and a data label want one string; see
            // ChartPlot.CategoryLevels. A date axis has re-ordered its categories by then, so a
            // levelled axis and a date axis are not both honoured -- no corpus chart is both.
            CategoryLevels = dateAxis is null ? CategoryLevelsOf(plotArea) : null,
            Series = orderedSeries,
            Kind = kind,

            // A doughnut is a pie of concentric rings; the element name is the whole of the file's
            // statement, since c:holeSize reaches nothing in the reference. See ChartPlot.Rings.
            Rings = group.Name.LocalName == "doughnutChart",

            // The one three-dimensional chart there is anything to measure — see
            // ChartPlot.Elevation. `c:depthPercent` is deliberately not read: rendering the same
            // document at 20, 50, 100 and 200 gives 26.2.4.2 byte-identical geometry at every
            // one of nine elevations, because the pie branch of View3DConverter never passes it
            // on. probes/pie3d-r187.
            Elevation = group.Name.LocalName == "pie3DChart"
                ? Math.Clamp(Number(Child(Child(chart, "view3D"), "rotX")) ?? 15.0, 0.0, 90.0)
                : null,
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
            IsPercentStacked = grouping is "percentStacked",
            ValueScale = ScaleOf(axes.Value),
            ValueFormat = FormatOf(axes.Value),
            CategoryFormat = FormatOf(axes.Category),
            CategoryAxisText = AxisTextOf(axes.Domain ?? axes.Category),
            ValueAxisText = AxisTextOf(axes.Value),
            SecondaryValueAxisText = AxisTextOf(axes.Secondary),
            DataTable = DataTableOf(Child(plotArea, "dTable"), theme),
            SecondaryValueScale = axes.Secondary is null ? null : ScaleOf(axes.Secondary),
            SecondaryValueFormat = FormatOf(axes.Secondary),
            SecondaryValueAxisTitle = AxisTitleText(Child(axes.Secondary, "title"), kind),
            DomainScale = ScaleOf(axes.Domain),

            // The horizontal axis a scatter group is measured against is normally the second
            // c:valAx, and ChartAxes.Domain finds it there. A combination chart pairing a
            // scatter group with an area or line one has only one c:valAx, so the X axis is the
            // chart's c:catAx or c:dateAx and Domain is null — and the ticks along it were then
            // written through General, printing a date column's serial numbers. The same
            // `?? axes.Category` fallback is already what the axis text, its visibility and its
            // labels take. The scale is deliberately not given the same fallback: it decides
            // where the points sit and not only how a tick reads.
            DomainFormat = FormatOf(axes.Domain ?? axes.Category),
            ValueAxisVisible = Shown(axes.Value),
            SecondaryAxisVisible = Shown(axes.Secondary),
            CategoryAxisVisible = Shown(axes.Domain ?? axes.Category),
            ValueLabelsVisible = Labelled(axes.Value),
            SecondaryLabelsVisible = Labelled(axes.Secondary),
            CategoryLabelsVisible = Labelled(axes.Domain ?? axes.Category),
            ValueTicks = TicksOf(axes.Value),
            SecondaryTicks = TicksOf(axes.Secondary),
            CategoryTicks = TicksOf(axes.Domain ?? axes.Category),
            CategoriesBetween = CrossBetween(axes, group),
            CategoriesReversed = Reversed(axes.Category),
            ValueLabelPosition = LabelPositionOf(axes.Value),
            ValueAxisCrossing = CrossingOf(axes.Value),
            SecondaryLabelPosition = LabelPositionOf(axes.Secondary),
            CategoryLabelPosition = LabelPositionOf(axes.Domain ?? axes.Category),
            CategoryAxisCrossing = CrossingOf(axes.Domain ?? axes.Category),
            CategoryCrossesAt = CrossesAt(axes.Domain ?? axes.Category),
            Legend = LegendOf(Child(chart, "legend")),
            Background = FillOf(Child(chartSpace, "spPr"), theme)
                         ?? DrawingChartAutoFormat.FrameFillOf(
                                automatic.Style, ChartAutoFrame.ChartSpace, theme),
            Border = LineOf(Child(chartSpace, "spPr"), theme)
                     ?? (automaticChartAreaLine && !SuppressesLine(Child(chartSpace, "spPr"))
                         ? AutomaticChartAreaLine
                         : null),
            BorderWidth = LineWidthOf(Child(chartSpace, "spPr")) is { } stated
                          && stated > Length.Zero
                ? stated
                : automaticChartAreaLine && LineOf(Child(chartSpace, "spPr"), theme) is null
                    && !SuppressesLine(Child(chartSpace, "spPr"))
                    ? AutomaticChartAreaLineWidth
                    : Length.Zero,
            PlotBackground = FillOf(Child(plotArea, "spPr"), theme)
                             ?? DrawingChartAutoFormat.FrameFillOf(
                                    automatic.Style, ChartAutoFrame.PlotArea, theme),
            ValueGrid = GridOf(axes.Value, theme, automatic),
            CategoryGrid = GridOf(axes.Category, theme, automatic)
                           ?? GridOf(axes.Domain, theme, automatic),
            ValueMinorGrid = MinorGridOf(axes.Value, theme, automatic),
            CategoryMinorGrid = MinorGridOf(axes.Category, theme, automatic)
                                ?? MinorGridOf(axes.Domain, theme, automatic),
            ValueAxisLine = AxisLineOf(axes.Value, theme, automatic),
            SecondaryAxisLine = AxisLineOf(axes.Secondary, theme, automatic),
            CategoryAxisLine = AxisLineOf(axes.Domain ?? axes.Category, theme, automatic),
            ValueMinorIntervals = MinorIntervals(axes.Value),
            // The three automatic-text sizes and weights, which are *not* chart2's model
            // defaults — see AutoText below for why an OOXML chart never reaches those.
            TitleSize = SizeOf(Child(chart, "title"))
                        ?? AutoText(chartSpace, 18.0, 120),
            AxisTitleSize = AxisTitleSizeOf(plotArea)
                            ?? AutoText(chartSpace, 10.0, 100),
            IsTitleBold = BoldOf(Child(chart, "title")) ?? AutoWeight(chartSpace) ?? true,
            IsAxisTitleBold = AxisTitleBoldOf(plotArea) ?? AutoWeight(chartSpace) ?? true,
            LabelSize = AxisLabelSizeOf(plotArea)
                        ?? AutoText(chartSpace, 10.0, 100),

            // The axes' own c:txPr, which states the weight of their *labels*. Unlike the two
            // titles this falls back to regular, because the auto-text table leaves
            // spOtherTexts regular — but the chart space's own c:txPr comes between the two,
            // exactly as it does for the size. See AutoWeight.
            IsLabelBold = AxisLabelBoldOf(plotArea) ?? AutoWeight(chartSpace) ?? false,

            // The five text colours. Each is read where its own object states it, and each falls
            // back to the chart style's own automatic text colour — tx1 below style 41 and lt1
            // from 41 up, which is what makes the text of a dark chart white instead of
            // invisible. It used to fall back to black, which is what tx1 resolves to on all but
            // seven of the corpus' chart-bearing files and is not what a dark style states at
            // all. See DrawingChartAutoFormat.TextColourOf and ChartPlot.LabelColour.
            LabelColour = AxisLabelColourOf(plotArea, theme) ?? automaticText,
            TitleColour = ColourOf(Child(chart, "title"), theme) ?? automaticText,
            AxisTitleColour = AxisTitleColourOf(plotArea, theme)
                              ?? ColourOf(Child(chart, "title"), theme) ?? automaticText,
            DataLabelColour = DataLabelColourOf(plotArea, theme)
                              ?? AxisLabelColourOf(plotArea, theme) ?? automaticText,
            LegendColour = ColourOf(Child(chart, "legend"), theme)
                           ?? AxisLabelColourOf(plotArea, theme) ?? automaticText,

            // The legend's own c:txPr, not the axes' — every length in the legend is a fraction
            // of it. Read from the legend element directly rather than through its descendants,
            // because a c:legendEntry carries a c:txPr of its own and precedes the legend's.
            LegendSize = SizeOf(Child(Child(chart, "legend"), "txPr")),
            IsLegendBold = BoldOf(Child(Child(chart, "legend"), "txPr")) ?? AutoWeight(chartSpace),

            // And the legend's own face, which FamilyOf's part-wide search gets wrong whenever
            // some *other* element of the part states one. See LegendFamilyOf.
            LegendFamily = LegendFamilyOf(chart, chartSpace, theme),

            // A series' c:dLbls/c:txPr, which is where a data label states its own size — not on
            // an axis. 20 of the corpus's 61 chart parts state one that differs from the axes'.
            DataLabelSize = DataLabelSizeOf(plotArea),
            IsDataLabelBold = DataLabelBoldOf(plotArea) ?? AutoWeight(chartSpace),
            TextFamily = FamilyOf(chartSpace, theme),

            // The main title's own face, when it names one. Read from c:title alone and left null
            // otherwise, so a chart whose title says nothing goes on taking TextFamily and the
            // stamping pass in ChartLayout.InFamily still decides every other label.
            TitleFamily = LiteralFamily(Child(chart, "title")),
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
    /// <para>
    /// <strong>A rectangle that does not fit the chart is moved, not shrunk, and that is the
    /// whole of the correction.</strong> The importer resolves the four fractions against the
    /// chart's page and hands the result to <c>XDiagramPositioning</c>
    /// (<c>PlotAreaConverter::convertPositionFromModel</c>,
    /// <c>oox/source/drawingml/chart/plotareaconverter.cxx</c>:510-538), which for
    /// <c>inner</c> is <c>setDiagramPositionExcludingAxes</c> and therefore
    /// <c>DiagramHelper::setDiagramPositioning</c>
    /// (<c>chart2/source/tools/DiagramHelper.cxx</c>:434-476). That function clamps each of the
    /// four to <c>[0, 1]</c> and then, if the position and the size still overrun,
    /// <em>moves the position back</em> — <c>aNewPos.Primary = 1.0 - aNewSize.Primary</c> — so the
    /// plot keeps the width the file asked for and loses the overhang off the far edge instead.
    /// See <see cref="Fitted"/> for the measurement.
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

        (x, width) = Fitted(x, width);
        (y, height) = Fitted(y, height);

        return (x, y, width, height);
    }

    /// <summary>
    /// One axis of a stated plot rectangle, fitted inside the chart the way the importer fits it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>lcl_ensureRange0to1</c> on each half and then <c>if (pos + size &gt; 1) pos = 1 - size</c>
    /// (<c>chart2/source/tools/DiagramHelper.cxx</c>:425-465). The order matters: the size is
    /// clamped first, so a file stating a width larger than the chart gets the whole chart and a
    /// position of zero rather than a negative position.
    /// </para>
    /// <para>
    /// <strong>Measured on <c>N2_E_Maestroni_Swarm_COP.pptx</c>, whose Gantt states
    /// <c>x=0.20148</c> and <c>w=0.82271</c> — 1.0242 between them.</strong> Taking the pair
    /// verbatim runs the plot 17.4 pt off the right edge of a 720 pt frame; moving the position
    /// puts it at <c>1 - 0.82271 = 0.17729</c>, which is 127.65 pt. 26.2.4.2 draws that chart's
    /// plot rectangle from 225.581 to 719.660 on the page, and that chart's primitives are fitted
    /// into the frame from 119.083 to 719.660 — an embedded chart is stretched onto its own drawn
    /// extent, <c>svx/source/sdr/contact/viewcontactofsdrole2obj.cxx</c>:88-116 — so the
    /// reference's own left edge in the chart's coordinates is
    /// <c>(225.581 - 119.083) / (719.660 - 119.083) × 720 = 127.65</c> — the same number, with no
    /// free parameter. Shrinking the width instead would put it at 145.06.
    /// </para>
    /// <para>
    /// <strong>A second witness, built rather than found, separates the two readings by 89 pt.</strong>
    /// <c>probes/chart-layout/mkprobe3.py</c>'s <c>OVER.pptx</c> states <c>x = 0.30</c> and
    /// <c>w = 0.85</c> in a 590.4 pt frame — an overrun sixty times Maestroni's. 26.2.4.2 centres
    /// its first value label on the plot's left edge at 138.25, against
    /// <c>50.4 + 0.15 × 590.4 = 138.9</c> for moving the position and 227.5 for shrinking the size.
    /// </para>
    /// <para>
    /// The importer does this arithmetic in whole 1/100 mm and this does it in the fractions
    /// themselves, which on that chart is a difference of 0.02 pt.
    /// </para>
    /// </remarks>
    private static (double Position, double Size) Fitted(double position, double size)
    {
        position = Math.Clamp(position, 0.0, 1.0);
        size = Math.Clamp(size, 0.0, 1.0);

        return position + size > 1.0 ? (1.0 - size, size) : (position, size);
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
    private static ChartGrid? GridOf(
        XElement? axis, DrawingTheme? theme, ChartAutoContext automatic)
    {
        if (Child(axis, "majorGridlines") is not { } grid) return null;

        XElement? properties = Child(grid, "spPr");
        if (Drawing.Child(Drawing.Child(properties, "ln"), "noFill") is not null) return null;

        return AutomaticLine(ChartAutoLine.MajorGrid, properties, theme, automatic);
    }

    /// <summary>
    /// One piece of a chart's furniture: what it states, and the automatic format under it.
    /// </summary>
    /// <remarks>
    /// <c>LineFormatter::convertFormatting</c> is two lines —
    /// <c>aLineProps.assignUsed(*mxAutoLine)</c> then <c>assignUsed(shape's own)</c> — so the
    /// automatic entry is the base and each thing the shape states wins over it *separately*.
    /// That is why an <c>a:ln</c> carrying only a <c>w</c> keeps the automatic colour, which is
    /// exactly what <c>Demick_JetBlue.pptx</c>'s value axis does, and why reading "states an
    /// <c>a:ln</c>" as "states everything" draws it black.
    /// </remarks>
    private static ChartGrid AutomaticLine(
        ChartAutoLine what,
        XElement? properties,
        DrawingTheme? theme,
        ChartAutoContext automatic)
    {
        Colour? colour = LineOf(properties, theme)
                         ?? DrawingChartAutoFormat.LineColourOf(
                             what, automatic.Style, theme, automatic.Styles);

        Length width = StatedLineWidth(properties)
                       ?? DrawingChartAutoFormat.AutomaticLineWidth(automatic.Styles);

        return new ChartGrid(colour ?? DefaultGrid, width, DashOf(properties));
    }

    /// <summary>
    /// How an axis draws its own line and tick marks — <c>c:spPr/a:ln</c> over the automatic
    /// entry.
    /// </summary>
    /// <remarks>
    /// A deleted axis is not drawn at all, so this is never asked about one; an
    /// <c>a:ln/a:noFill</c> is a line the file turns off, and there is nowhere in
    /// <see cref="ChartGrid"/> to say so, so it is drawn as the automatic colour rather than
    /// invented away. Two corpus axes state it and both are also <c>c:delete val="1"</c>.
    /// </remarks>
    private static ChartGrid AxisLineOf(
        XElement? axis, DrawingTheme? theme, ChartAutoContext automatic)
        => AutomaticLine(ChartAutoLine.Axis, Child(axis, "spPr"), theme, automatic);

    /// <summary>
    /// An axis' minor gridlines, with the width and dash they state, or null when it has none.
    /// </summary>
    /// <remarks>
    /// Unlike <c>c:majorGridlines</c>, a <c>c:minorGridlines</c> in the corpus usually carries an
    /// <c>a:ln</c>, and both of the things it puts there are visible: a stated width and a stated
    /// <c>a:prstDash</c>. Reading only the colour draws 110 solid hairlines where the reference
    /// draws 110 dashed half-point ones — see <see cref="ChartGrid"/>.
    /// </remarks>
    private static ChartGrid? MinorGridOf(
        XElement? axis, DrawingTheme? theme, ChartAutoContext automatic)
    {
        if (Child(axis, "minorGridlines") is not { } grid) return null;

        XElement? properties = Child(grid, "spPr");
        if (Drawing.Child(Drawing.Child(properties, "ln"), "noFill") is not null) return null;

        return AutomaticLine(ChartAutoLine.MinorGrid, properties, theme, automatic);
    }

    /// <summary>
    /// How many sub-intervals this axis' minor grid divides one major interval into.
    /// </summary>
    /// <remarks>
    /// <c>AxisConverter::convertFromModel</c>'s <c>REALNUMBER</c>/<c>PERCENT</c> branch
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx:389-409</c>), which is the only place the
    /// count is decided for an OOXML axis: <c>round(majorUnit / minorUnit)</c> when both are
    /// stated and the quotient is sane, <b>5</b> when <c>c:minorUnit</c> is absent — its own
    /// comment is <c>tdf#114168 … as MS Excel do</c> — and 9 for a logarithmic axis that states
    /// one. A stated minor unit alone, with no major, leaves the count <em>unset</em> and
    /// <c>ScaleAutomatism</c>'s default of 2 stands.
    /// </remarks>
    private static int MinorIntervals(XElement? axis)
    {
        bool logarithmic = Value(Child(Child(axis, "scaling"), "logBase")) is not null;
        double? major = Number(Child(axis, "majorUnit"));
        double? minor = Number(Child(axis, "minorUnit"));

        if (logarithmic) return minor is null ? 2 : 9;
        if (major is { } step && minor is { } sub && sub > 0 && sub <= step)
        {
            double count = (step / sub) + 0.5;
            return count is >= 1.0 and < 1001.0 ? (int)count : 2;
        }

        return minor is null ? 5 : 2;
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

    /// <summary>
    /// Whether an axis draws its tick labels — <c>c:tickLblPos val="none"</c> says it does not.
    /// </summary>
    /// <remarks>
    /// One line in <c>AxisConverter::convertFromModel</c>:
    /// <c>aAxisProp.setProperty(PROP_DisplayLabels, mrModel.mnTickLabelPos != XML_none)</c>
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx:221</c>). Every other value —
    /// <c>nextTo</c>, <c>high</c>, <c>low</c>, and an absent element — leaves the labels on and
    /// only moves where they sit, which this does not model. It is deliberately *not* folded into
    /// <see cref="Shown"/>: the axis line and its ticks survive, and the plot area gives up the
    /// tick's length either way. See <c>ChartPlot.ValueLabelsVisible</c>.
    /// </remarks>
    private static bool Labelled(XElement? axis)
        => !string.Equals(Value(Child(axis, "tickLblPos")), "none", StringComparison.Ordinal);

    /// <summary>Where an axis puts its major tick marks — <c>c:majorTickMark</c>.</summary>
    /// <remarks>
    /// <para>
    /// <c>lclGetTickMark</c> (<c>oox/source/drawingml/chart/axisconverter.cxx:104-115</c>):
    /// <c>in</c> is <c>INNER</c>, <c>out</c> is <c>OUTER</c>, <c>cross</c> is both, and anything
    /// else is neither. Only <c>OUTER</c> is charged to the plot area, which is why this is read
    /// at all — see <c>ChartPlot.ValueTicks</c> and the six-arm probe behind it.
    /// </para>
    /// <para>
    /// <strong>An absent element is not <c>none</c>.</strong> <c>AxisModel</c>'s constructor
    /// defaults it to <c>out</c> for an MSO-2007 chart part and to <c>cross</c> for a later one
    /// (<c>oox/source/drawingml/chart/axismodel.cxx:42-48</c>) — the two differ in where the tick
    /// is drawn and not in what it reserves, so the distinction between them is invisible to the
    /// plot rectangle and <c>Outer</c> is taken for both. The corpus states the element on 481 of
    /// its 494 axes, so the default decides 13 of them, in two documents.
    /// </para>
    /// </remarks>
    private static ChartTickMark TicksOf(XElement? axis) => Value(Child(axis, "majorTickMark")) switch
    {
        "none" => ChartTickMark.None,
        "in" => ChartTickMark.Inner,
        "cross" => ChartTickMark.Cross,
        _ => ChartTickMark.Outer,
    };

    /// <summary>
    /// Whether the file says the categories occupy slots — <c>c:crossBetween</c> on the value
    /// axis the category axis crosses — or null when it says nothing that reaches the question.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Feeds <see cref="ChartPlot.CategoriesBetween"/>, which carries the nine-arm measurement
    /// this is written from. Three things happen here and each is one of that table's columns:
    /// </para>
    /// <list type="bullet">
    /// <item>a chart with no <c>c:catAx</c> at all — a scatter, a bubble, a pie — has no category
    /// axis to shift, and answers null;</item>
    /// <item>a radar chart answers null whatever the element says, because
    /// <c>axisconverter.cxx:295-296</c> forces <c>RADARLINE</c> and <c>RADARAREA</c> to unshifted
    /// ahead of reading it — and the corpus holds three slides radar charts stating
    /// <c>between</c>, which is exactly the case that would go wrong;</item>
    /// <item>with the element absent, a <c>c:lineChart</c> or a <c>c:stockChart</c> is shifted and
    /// everything else answers null and keeps the type test's own answer.</item>
    /// </list>
    /// <para>
    /// A bar or column chart is not special-cased here: it is
    /// <see cref="ChartPlot.ShiftedCategories"/> that ignores this value for one, and it does so
    /// because the running binary does.
    /// </para>
    /// </remarks>
    private static bool? CrossBetween(ChartAxes axes, XElement group)
    {
        if (axes.Category is null) return null;

        string name = group.Name.LocalName;
        if (name is "radarChart") return null;

        return Value(Child(axes.Crossing, "crossBetween")) switch
        {
            "between" => true,
            "midCat" => false,
            _ => name is "lineChart" or "line3DChart" or "stockChart" ? true : null,
        };
    }

    /// <summary>
    /// The date axis a <c>c:dateAx</c> asks for, or null when the category axis is an ordinary
    /// run of slots.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The element name is not the whole of the statement.</strong>
    /// <c>AxisConverter::convertFromModel</c> gives a <c>c:dateAx</c> <c>AxisType::DATE</c> and
    /// copies <c>c:auto</c> onto <c>ScaleData::AutoDateAxis</c>
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx</c>); chart2 then asks
    /// <c>ExplicitCategoriesProvider::isDateAxis</c> whether the categories really are dates
    /// before it uses the scale, and that test is the categories' <em>number format</em>. So an
    /// automatic date axis over a column of plain numbers is drawn as a category axis, and only a
    /// <c>c:auto val="0"</c> forces the scale on regardless. This is the same pair of rules
    /// <c>XlsChartReader</c> applies to <c>CHDATERANGE</c>'s two flags, reached from the other
    /// vocabulary.
    /// </para>
    /// <para>
    /// <strong>Both stated limits and the tick interval come in as serials.</strong>
    /// <c>c:scaling/c:min</c> and <c>c:max</c> on a date axis are already serial numbers, unlike
    /// <c>CHDATERANGE</c>'s, which count their own base unit from the null date. <c>c:majorUnit</c>
    /// is a count and <c>c:majorTimeUnit</c> the unit it counts, defaulting to days
    /// (<c>CT_DateAx</c>'s <c>ST_TimeUnit</c> default), and <c>c:baseTimeUnit</c> is the
    /// resolution — the finest unit the axis distinguishes — which the scale otherwise derives
    /// from the data.
    /// </para>
    /// <para>
    /// <strong><c>c:date1904</c> decides what a serial names.</strong> It sits on
    /// <c>c:chartSpace</c> rather than on the axis, and it is four years and a day of error when
    /// it is missed.
    /// </para>
    /// </remarks>
    /// <param name="chartSpace">The <c>c:chartSpace</c>, for <c>c:date1904</c>.</param>
    /// <param name="axis">The category axis element, or null when the chart has none.</param>
    /// <param name="crossing">
    /// The value axis this one crosses, which is where <c>c:crossBetween</c> is stated.
    /// </param>
    /// <param name="values">The category cells read as numbers.</param>
    private static ChartDateAxis? DateAxisOf(
        XElement chartSpace, XElement? axis, XElement? crossing, double?[] values)
    {
        if (axis is null || !Is(axis, "dateAx") || values.Length == 0) return null;

        NumberFormatCode? format = FormatOf(axis);

        // c:auto is absent on plenty of parts and its schema default is true, so an unstated one
        // takes the checked path rather than forcing the scale on.
        bool automatic = Flag(axis, "auto") ?? true;
        if (automatic && format is not { IsDateTime: true }) return null;

        XElement? scaling = Child(axis, "scaling");

        ChartTimeInterval? interval = null;
        if (Number(Child(axis, "majorUnit")) is { } unit and > 0.0)
        {
            interval = new ChartTimeInterval(
                (int)Math.Clamp(unit, 1.0, int.MaxValue),
                TimeUnitOf(Value(Child(axis, "majorTimeUnit"))) ?? ChartTimeUnit.Day);
        }

        return ChartDateScale.Resolve(
            values,
            format,
            Number(Child(scaling, "min")),
            Number(Child(scaling, "max")),
            interval,
            TimeUnitOf(Value(Child(axis, "baseTimeUnit"))),
            Flag(chartSpace, "date1904") == true
                ? SpreadsheetDateSystem.Date1904
                : SpreadsheetDateSystem.Date1900,
            ShiftedCategoriesOf(crossing));
    }

    /// <summary>
    /// Whether the categories sit between the ticks rather than on them.
    /// </summary>
    /// <remarks>
    /// <c>AxisConverter::convertFromModel</c> reads it off the *crossing* axis — the value axis —
    /// as <c>mnCrossBetween == XML_between</c>, and defaults it to true for a bar, line or stock
    /// group when the attribute is absent
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx</c>:292-301). The two type overrides
    /// above that test, a 3-D bar and a radar, are not modelled here: no corpus chart pairs
    /// either with a date axis.
    /// </remarks>
    private static bool ShiftedCategoriesOf(XElement? crossing)
        => Value(Child(crossing, "crossBetween")) switch
        {
            "between" => true,
            "midCat" => false,
            _ => true,
        };

    /// <summary>The three <c>ST_TimeUnit</c> spellings, or null when the element is absent.</summary>
    private static ChartTimeUnit? TimeUnitOf(string? stated) => stated switch
    {
        "days" => ChartTimeUnit.Day,
        "months" => ChartTimeUnit.Month,
        "years" => ChartTimeUnit.Year,
        _ => null,
    };

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
    /// <para>
    /// <strong>A <c>c:valAx</c> gets none of it either, and for a second reason.</strong> Those
    /// three lines are inside <c>switch (aScaleData.AxisType) case CATEGORY: case SERIES: case
    /// DATE:</c> (<c>axisconverter.cxx:324-326</c>), and a <c>c:valAx</c> is set to
    /// <c>AxisType::REALNUMBER</c> or <c>PERCENT</c> — on the X axis at <c>:306</c> and on the Y
    /// axis at <c>:311</c> — so it never enters that case at all and keeps the same model defaults
    /// a date axis keeps. A <c>c:serAx</c> is <c>AxisType::SERIES</c> and <em>does</em> enter it,
    /// so it is set like a category axis. <strong>Wrapping off is what lets a value axis turn 45°
    /// instead of thinning</strong>: <c>canAutoAdjustLabelPlacement</c> refuses while
    /// <c>m_bLineBreakAllowed</c> is true (<c>VCartesianAxis.cxx:539-556</c>), so an axis carrying
    /// it can only raise its rhythm. Measured on
    /// <c>027_Simple_personal_cash_flow_statement</c> page 6, whose savings chart runs its money
    /// axis along the bottom: 26.2.4.2 draws its eight labels turned and this tree drew four
    /// upright, having dropped the other four to the rhythm. It is also the arm chart2 would
    /// refuse anyway — <c>isBreakOfLabelsAllowed</c> opens with <em>"no break for value
    /// axis"</em>, <c>!m_bUseTextLabels</c> (<c>:522-524</c>) — so no OOXML value axis wraps under
    /// either rule, and this only settles what happens when its labels collide.
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

        // Only the axes chart2 gives `AxisType::CATEGORY` or `SERIES` and that are not a
        // `c:dateAx` reach the three lines above; everything else — a date axis, a value axis,
        // and a chart with no such element at all — keeps chart2's own model defaults.
        bool converted = axis is not null && (Is(axis, "catAx") || Is(axis, "serAx"));

        return new ChartAxisText(
            rotation * Math.PI / 180.0,
            OverlapAllowed: converted && stated is 0,
            LineBreakAllowed: converted && rotation is 0.0 or 90.0 or 270.0,
            Stagger: converted ? ChartLabelStagger.SideBySide : ChartLabelStagger.Auto);
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

    /// <summary>
    /// chart2's own gridline colour, gray30 — the last resort when there is no theme to ask.
    /// </summary>
    /// <remarks>
    /// <strong>It is not what an OOXML chart draws</strong>, and reaching it means
    /// <see cref="DrawingChartAutoFormat.LineColourOf"/> found neither a theme nor a format
    /// matrix. Kept because inventing black there would be worse than the value chart2's own
    /// model carries (<c>chart2/source/model/main/GridProperties.cxx:64-66</c>).
    /// </remarks>
    private static readonly Colour DefaultGrid = Colour.FromRgb(0xB3B3B3);


    /// <summary>What one axis states about its scale.</summary>
    /// <summary>
    /// Whether one axis states that it runs from its maximum down to its minimum.
    /// </summary>
    /// <remarks>
    /// The same <c>c:scaling/c:orientation</c> <see cref="ScaleOf"/> reads, asked of the category
    /// axis, where there is no scale to carry it: a category axis has no minimum or maximum for a
    /// <see cref="ChartScaleRequest"/> to hold, and its orientation still decides which end the
    /// first category is at. See <see cref="ChartPlot.CategoriesReversed"/>.
    /// </remarks>
    /// <summary>Which end of the crossing axis an axis puts its tick labels at.</summary>
    /// <remarks>
    /// <c>none</c> is deliberately not here: it says the labels are not drawn at all and is
    /// <see cref="Labelled"/>'s business, so it falls through to the default alongside every
    /// unknown value — which is what <c>lclGetLabelPosition</c> does with it too
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx:92-101</c>).
    /// </remarks>
    /// <summary>Which end of the crossing axis a value axis stands at.</summary>
    /// <remarks>
    /// <c>c:crossesAt</c> states a value rather than an end and is not read: on a category
    /// crossing axis it is a category index, and no corpus chart states one on a value axis.
    /// </remarks>
    private static ChartAxisCrossing CrossingOf(XElement? axis)
        => Value(Child(axis, "crosses")) switch
        {
            "min" => ChartAxisCrossing.Minimum,
            "max" => ChartAxisCrossing.Maximum,
            _ => ChartAxisCrossing.Automatic,
        };

    /// <summary>The value an axis states its line stands at, or null when it names an end.</summary>
    /// <remarks>
    /// <c>c:crossesAt</c>, which <c>bManualCrossing</c> gives precedence over <c>c:crosses</c>
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx:443-455</c>). It is read for the
    /// <em>category</em> axis, where the crossing axis is numeric and the value is a real one; on
    /// a value axis it would be a category index and no corpus chart states one.
    /// </remarks>
    private static double? CrossesAt(XElement? axis)
        => double.TryParse(
               Value(Child(axis, "crossesAt")),
               NumberStyles.Float, CultureInfo.InvariantCulture, out double at)
           && double.IsFinite(at)
            ? at
            : null;

    private static ChartValueLabelPosition LabelPositionOf(XElement? axis)
        => Value(Child(axis, "tickLblPos")) switch
        {
            "low" => ChartValueLabelPosition.Low,
            "high" => ChartValueLabelPosition.High,
            _ => ChartValueLabelPosition.NextTo,
        };

    private static bool Reversed(XElement? axis)
        => Value(Child(Child(axis, "scaling"), "orientation")) == "maxMin";

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

        /// <summary>
        /// The value axis the category axis crosses — <c>c:catAx/c:crossAx</c> — or null.
        /// </summary>
        /// <remarks>
        /// <c>c:crossBetween</c> is stated on this axis and on no other, and
        /// <c>oox/source/drawingml/chart/plotareaconverter.cxx:229-231</c> hands exactly this
        /// axis to the category axis' converter as its <c>pCrossingAxis</c>. On a chart with one
        /// value axis it is <see cref="Value"/>; on a combination chart with a secondary axis it
        /// need not be, and taking the primary would read the wrong element.
        /// </remarks>
        public XElement? Crossing { get; private init; }

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
                Crossing = Matching(value, Value(Child(category, "crossAx")))
                           ?? (remaining.Count > 0 ? remaining[0] : null),
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

    /// <summary>
    /// What a series takes from the chart's automatic formatting when it states nothing.
    /// </summary>
    /// <param name="Style">The chart space's <c>c:style/@val</c>.</param>
    /// <param name="MaxSeriesIndex">The largest <c>c:ser/c:idx</c> in the whole plot area.</param>
    /// <param name="Styles">
    /// The theme's format matrix, for the width of the line an automatic series draws. Null when
    /// the caller has none, which leaves the width at the reader's hairline default rather than
    /// inventing one.
    /// </param>
    /// <param name="VaryColours">
    /// <c>c:varyColors</c> on the plot area's only group, which makes a frame series colour its
    /// points from the cycle instead of taking one colour for the whole series. Null when the
    /// group states none, whose default is true on anything but a file Office 2007 wrote —
    /// <c>TypeGroupModel</c>'s <c>mbVaryColors( !bMSO2007Doc )</c>.
    /// </param>
    /// <param name="SingleGroup">
    /// Whether the plot area holds exactly one group, which is <c>bSupportsVaryColorsByPoint</c>.
    /// </param>
    private readonly record struct ChartAutoContext(
        int Style, int MaxSeriesIndex, DrawingStyleMatrix? Styles, bool? VaryColours, bool SingleGroup);

    /// <summary>
    /// The largest <c>c:ser/c:idx</c> over every plot group, or −1 when none states one.
    /// </summary>
    /// <remarks>
    /// Taken across the groups rather than within one, because the accent cycle is numbered over
    /// the whole plot area: a combination chart's line group carries <c>c:idx val="2"</c> and
    /// takes accent 3 even though it is the only series in its group.
    /// </remarks>
    private static int MaximumSeriesIndex(List<XElement> groups)
    {
        int maximum = -1;

        foreach (XElement group in groups)
            foreach (XElement element in Children(group, "ser"))
                maximum = Math.Max(maximum, Drawing.Number(Child(element, "idx"), "val") ?? -1);

        return maximum;
    }

    private static (List<ChartSeries> Series, string?[] Categories, double?[] CategoryValues)
        ReadSeries(
        XElement group,
        ChartPlotKind kind,
        DrawingTheme? theme,
        int axisIndex,
        bool office2007,
        ChartAutoContext automatic = default,
        ChartRangeResolver? ranges = null)
    {
        List<ChartSeries> series = [];
        string?[] categories = [];
        double?[] categoryValues = [];

        // c:scatterStyle decides whether a scatter series draws its line, its markers or both.
        // "marker" alone is the case that matters: drawing the line and not the markers leaves an
        // empty plot area, because the file asked for no line.
        string? scatterStyle = Value(Child(group, "scatterStyle"));
        bool scatterLine = kind != ChartPlotKind.Scatter || scatterStyle != "marker";
        string? radarStyle = Value(Child(group, "radarStyle"));

        // A group's own c:dLbls is the default every series in it inherits.
        ChartDataLabel? groupLabel = LabelOf(Child(group, "dLbls"), null, kind, office2007);

        bool smooth = SmoothOf(group, kind, office2007);

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

        int seriesInGroup = Children(group, "ser").Count();

        int stockRole = kind != ChartPlotKind.Stock
            ? -1
            : seriesInGroup == 3 ? 1 : 0;

        foreach (XElement element in Children(group, "ser"))
        {
            (string?[] labels, double?[] labelNumbers) = ReadSequence(
                Child(element, "cat") ?? Child(element, "xVal"), ranges);
            if (categories.Length == 0 && labels.Length > 0)
            {
                categories = labels;

                // The same cells read as numbers, which is what a c:dateAx scales against. Kept
                // beside the text rather than instead of it: a date axis labels its *ticks* and
                // still wants the strings for a chart that turns out not to be one.
                categoryValues = labelNumbers;
            }

            XElement? valueSource = Child(element, "val") ?? Child(element, "yVal");
            (_, double?[] numbers) = ReadSequence(valueSource, ranges);

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
                (_, double?[] xs) = ReadSequence(xVal, ranges);
                if (xs.Length > 0) domain = xs;
            }

            // The bubble's third dimension. c:bubbleSize is a sequence like any other and is the
            // only thing that makes a bubble chart more than a scatter chart with round markers.
            double?[]? sizes = null;
            if (kind == ChartPlotKind.Bubble && Child(element, "bubbleSize") is { } bubbleSize)
            {
                (_, double?[] read) = ReadSequence(bubbleSize, ranges);
                if (read.Length > 0) sizes = read;
            }

            XElement? properties = Child(element, "spPr");
            XElement? seriesLabels = Child(element, "dLbls");

            // The automatic colours the chart's style gives this series, which its own c:spPr
            // then overrides. Both halves matter: without the automatic one a series stating
            // nothing is drawn black and its legend key blank, and without the override a series
            // stating a colour loses it.
            int seriesIndex = Drawing.Number(Child(element, "idx"), "val") ?? -1;
            ChartAutoObject frame = IsFrameSeries(kind)
                ? ChartAutoObject.FilledSeries
                : ChartAutoObject.LinearSeries;

            Colour? autoFill = DrawingChartAutoFormat.ColourOf(
                automatic.Style, frame, stroke: false, seriesIndex, automatic.MaxSeriesIndex,
                theme, automatic.Styles);
            Colour? autoLine = DrawingChartAutoFormat.ColourOf(
                automatic.Style, frame, stroke: true, seriesIndex, automatic.MaxSeriesIndex,
                theme, automatic.Styles);

            series.Add(new ChartSeries(
                SeriesName(Child(element, "tx"), ranges)
                ?? GeneratedSeriesName(valueSource, ranges),
                numbers,
                SuppressesFill(properties) ? null : FillOf(properties, theme) ?? autoFill,
                SuppressesLine(properties) ? null : LineOf(properties, theme) ?? autoLine,
                SeriesLineWidth(element, properties, automatic, frame, theme, seriesIndex),
                PointFills(
                    element,
                    numbers.Length,
                    theme,
                    kind,
                    automatic,
                    seriesIndex,
                    seriesInGroup,
                    office2007),
                kind)
            {
                XValues = domain,
                Marker = MarkerOf(element, kind, scatterStyle, radarStyle, seriesIndex),
                MarkerFill = MarkerFillOf(element, theme),
                MarkerLine = LineOf(MarkerProperties(element), theme),
                MarkerSize = MarkerSizeOf(element),
                HasLine = scatterLine && !SuppressesLine(properties),
                Smooth = smooth,
                DashPattern = DashOf(properties),
                LineCap = CapOf(properties),
                Label = WithSource(LabelOf(seriesLabels, groupLabel, kind, office2007), sourceFormat),
                PointLabels = PointLabelsOf(
                    seriesLabels, numbers.Length, groupLabel, kind, sourceFormat, office2007,
                    DataLabelsRangeOf(element)),
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

        return (series, categories, categoryValues);
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
    /// Whether an of-pie group states an exploded series, which is what makes 26.2.4.2 draw it
    /// as a plain pie.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Measured at the reference, one attribute at a time and in both directions.</strong>
    /// The corpus holds two <c>c:ofPieChart</c> documents and they part company here:
    /// </para>
    /// <list type="bullet">
    /// <item><description>
    /// <c>029_Unit_Circle_Chart_Pie_Theme_8a922142.docx</c> states
    /// <c>&lt;c:explosion val="1"/&gt;</c> on its series. 26.2.4.2 draws it as one circle of
    /// four wedges with four labels and a four-entry legend — no second plot, no composite
    /// slice, no connector lines — and its own <c>--convert-to fodt</c> of the file carries a
    /// bare <c>chart:class="chart:circle"</c> with no <c>loext:sub-pie</c>.
    /// <strong>Delete that one element and the export carries
    /// <c>loext:sub-pie="true"</c>.</strong>
    /// </description></item>
    /// <item><description>
    /// <c>028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx</c> states no explosion at all
    /// (it carries seventeen <c>c:dPt</c>, so the per-point elements are not what decides it)
    /// and the reference draws its pie-plus-bar in full.
    /// <strong>Add <c>&lt;c:explosion val="1"/&gt;</c> to its series and the export stops
    /// carrying <c>loext:sub-bar</c>.</strong>
    /// </description></item>
    /// </list>
    /// <para>
    /// So the attribute decides it in both directions on both documents. <strong>The mechanism
    /// is not established</strong>: <c>PieChartTypeTemplate::matchesTemplate</c> reads the
    /// series' <c>Offset</c> and answers <c>PieChartOffsetMode_ALL_EXPLODED</c>
    /// (<c>chart2/source/model/template/PieChartTypeTemplate.cxx</c>:307-366, this tree), which
    /// is the obvious suspect for re-templating the diagram out of its of-pie type, but that
    /// reading requires every point's offset to equal the series' and <c>029</c>'s do not — so
    /// it is a lead and not the answer. What is recorded here is the measurement.
    /// </para>
    /// <para>
    /// <strong>It is not the whole of what separates us from the reference on that
    /// document, and the rest is left open.</strong> With the explosion removed, 26.2.4.2 draws
    /// <c>029</c> as a <em>bar</em>-of-pie when <c>c:ofPieType</c> is changed to <c>bar</c> and
    /// still as a plain pie when it is left at <c>pie</c> — at four, five, six and seven points
    /// — while <c>028</c> with <c>val="pie"</c> forced onto it draws a full pie-of-pie at
    /// sixteen. So the pie sub-type carries a second condition that a small series fails and
    /// this tree does not model; <c>probes/chart-smooth-r102</c> has the variants. It costs
    /// nothing here because the corpus's only pie-of-pie is the exploded one.
    /// </para>
    /// </remarks>
    private static bool IsExploded(XElement group)
    {
        foreach (XElement series in Children(group, "ser"))
        {
            if (Drawing.Number(Child(series, "explosion"), "val") is > 0) return true;
        }

        return false;
    }

    /// <summary>
    /// Whether a plot group's lines are drawn as flattened cubic splines.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Three things decide it, and all three are the reference's, not the schema's.
    /// </para>
    /// <para>
    /// <strong>Only a two-dimensional line, stock or scatter group can be smoothed.</strong>
    /// <c>TypeGroupConverter::convertLineSmooth</c> does nothing unless
    /// <c>!isSeriesFrameFormat() &amp;&amp; meTypeCategory != TYPECATEGORY_RADAR</c>
    /// (<c>oox/source/drawingml/chart/typegroupconverter.cxx</c>:686), and
    /// <c>isSeriesFrameFormat()</c> is <c>mb3dChart || mbSeriesIsFrame2d</c> (<c>:261-264</c>) —
    /// which the type table sets false for exactly <c>TYPEID_LINE</c>, <c>TYPEID_STOCK</c> and
    /// <c>TYPEID_SCATTER</c> (<c>:95-117</c>). A bar, an area, a pie, a bubble or a radar
    /// group's <c>c:smooth</c> is read and discarded. Stock is in the list for fidelity and has
    /// no corpus witness, and <see cref="ChartLayout"/> draws a stock plot as candles rather
    /// than as a polyline in any case.
    /// </para>
    /// <para>
    /// <strong>One smoothed series smooths the group.</strong> See
    /// <see cref="ChartSeries.Smooth"/>: the property is set on the chart type, once per series,
    /// so the last series to state anything wins and any series stating <c>1</c> is enough
    /// — <c>convertLineSmooth</c> is only reached for a series whose <c>mbSmooth</c> is true
    /// (<c>:586-587</c>).
    /// </para>
    /// <para>
    /// <strong>An unstated <c>c:smooth</c> is <c>!office2007</c> and not false.</strong>
    /// <c>SeriesModel</c>'s <c>mbSmooth( !bMSO2007Doc )</c>
    /// (<c>oox/source/drawingml/chart/seriesmodel.cxx</c>:124). Measured both ways over the
    /// three corpus documents holding a line or scatter group that states no <c>c:smooth</c> at
    /// all: <c>Demick_JetBlue.pptx</c> and <c>171128IPAP.pptx</c> both declare
    /// <c>&lt;Application&gt;Microsoft Office PowerPoint&lt;/Application&gt;</c> with
    /// <c>&lt;AppVersion&gt;12.0000&lt;/AppVersion&gt;</c> and 26.2.4.2's own ODF export of them
    /// carries no <c>chart:interpolation</c> on those groups;
    /// <c>microsoft_learn_multi_chart_examples.xlsx</c> declares <c>AppVersion 3.1</c>, is
    /// therefore not an Office 2007 file, and its export carries
    /// <c>chart:interpolation="cubic-spline"</c> on two of its five charts — whose reference
    /// PDF then draws 60 and 200 segments for a four-point and an eleven-point series.
    /// </para>
    /// </remarks>
    private static bool SmoothOf(XElement group, ChartPlotKind kind, bool office2007)
    {
        if (kind is not (ChartPlotKind.Line or ChartPlotKind.Scatter or ChartPlotKind.Stock))
            return false;

        // mb3dChart is the other half of isSeriesFrameFormat, and a c:line3DChart is the one
        // spelling that reaches here with it set.
        if (group.Name.LocalName is "line3DChart") return false;

        bool smooth = false;

        foreach (XElement element in Children(group, "ser"))
            smooth |= Flag(element, "smooth") ?? !office2007;

        return smooth;
    }

    /// <summary>
    /// What marker a series draws, or none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>c:marker/c:symbol</c>. <c>c:scatterStyle val="line"</c> or <c>"smooth"</c> turns
    /// markers off, and <c>c:radarStyle val="marker"</c> turns them on — which is the whole
    /// difference between that style and <c>standard</c>, both of which draw a stroked polygon.
    /// </para>
    /// <para>
    /// <strong>A series of a non-frame type that states no <c>c:marker</c> at all draws an
    /// automatic marker, and the shape cycles with the series index.</strong>
    /// <c>SeriesModel</c>'s constructor is <c>mnMarkerSymbol( XML_auto )</c>
    /// (<c>seriesmodel.cxx:119</c>) and <c>TypeGroupConverter::convertMarker</c> maps that to
    /// <c>SymbolStyle_AUTO</c>, which <c>VDataSeries::getSymbolProperties</c> then resolves to
    /// <c>StandardSymbol = m_nGlobalSeriesIndex</c> (<c>VDataSeries.cxx:875-883</c>) — square,
    /// diamond, arrow-down, arrow-up and so on. It is <c>c:marker/c:symbol</c> that turns them
    /// off, because that context reads <c>getToken( XML_val, XML_none )</c>
    /// (<c>seriescontext.cxx:445</c>), and Excel writes exactly that on a plain line chart.
    /// </para>
    /// <para>
    /// The group's own <c>&lt;c:marker val="0"/&gt;</c> does <em>not</em> turn them off:
    /// <c>TypeGroupModel::mbShowMarker</c> is parsed at <c>typegroupcontext.cxx:216-218</c> and
    /// read by nothing in the whole of <c>oox</c> and <c>chart2</c> — the reference's own
    /// property-read-and-never-used. Honouring it here would draw fewer markers than the
    /// reference on every file that states it.
    /// </para>
    /// </remarks>
    private static ChartMarker MarkerOf(
        XElement? series,
        ChartPlotKind kind,
        string? scatterStyle,
        string? radarStyle,
        int seriesIndex)
    {
        string? symbol = Value(Child(Child(series, "marker"), "symbol"));

        if (symbol is null)
        {
            bool suppressed =
                (kind == ChartPlotKind.Scatter && scatterStyle is "line" or "smooth")
                || (kind == ChartPlotKind.Radar && radarStyle != "marker")
                || IsFrameSeries(kind);

            return suppressed ? ChartMarker.None : AutomaticMarker(seriesIndex);
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
    /// The shape an automatic marker takes at a series index.
    /// </summary>
    /// <remarks>
    /// <c>ShapeFactory</c>'s standard symbol list in the order
    /// <c>TypeGroupConverter::convertMarker</c> names it (<c>typegroupconverter.cxx:637-650</c>):
    /// 0 square, 1 diamond, 2 arrow down, 3 arrow up. Only the shapes
    /// <see cref="ChartMarker"/> can draw are distinguished; the rest of the list cycles back
    /// through them rather than through nothing, on the same reasoning that puts
    /// <c>dot</c> and <c>dash</c> on a square.
    /// </remarks>
    private static ChartMarker AutomaticMarker(int seriesIndex)
    {
        ChartMarker[] cycle =
        [
            ChartMarker.Square,
            ChartMarker.Diamond,
            ChartMarker.Triangle,
            ChartMarker.Triangle,
            ChartMarker.Circle,
            ChartMarker.Cross,
            ChartMarker.Star,
            ChartMarker.Square,
        ];

        return cycle[Math.Max(seriesIndex, 0) % cycle.Length];
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
        //
        // A bar-of-pie is in that category too, and leaving it out cost every label of
        // 028_Unit_Circle_Chart_Optimized_Graph its percentage: TYPEID_OFPIE sits beside
        // TYPEID_PIE and TYPEID_DOUGHNUT on TYPECATEGORY_PIE in the type table
        // (oox/source/drawingml/chart/typegroupconverter.cxx:103-105).
        bool percent = kind is ChartPlotKind.Pie or ChartPlotKind.OfPie
                       && (Flag(labels, "showPercent") ?? inherited?.ShowPercent ?? shown);
        bool category = Flag(labels, "showCatName") ?? inherited?.ShowCategory ?? shown;
        bool name = Flag(labels, "showSerName") ?? inherited?.ShowSeries ?? shown;

        // c:showLegendKey. Unlike the other four this one defaults *off* even outside the Office
        // 2007 arm: `lclConvertLabelFormatting` initialises `ShowLegendSymbol` from
        // `rDataLabel.mobShowLegendKey.get(false)` and never from `bDefaultShown`
        // (seriesconverter.cxx:139). Sixty-two of them in five sheets documents.
        bool key = Flag(labels, "showLegendKey") ?? inherited?.ShowLegendKey ?? false;

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
            ShowLegendKey = key,
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
        bool office2007,
        IReadOnlyList<string?>? dataLabelsRange = null)
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

            ChartDataLabel? read =
                WithSource(LabelOf(point, seriesLevel, kind, office2007), source);

            points[index] = WithCellRange(read, point, index, dataLabelsRange);
        }

        return points;
    }

    /// <summary>The Office extension a <c>CELLRANGE</c> field's text actually comes from.</summary>
    /// <remarks><c>{02D57815-91ED-43cb-92C2-25804820EDAC}</c>'s namespace.</remarks>
    private const string Chart2012 = "http://schemas.microsoft.com/office/drawing/2012/chart";

    /// <summary>
    /// A series' data-label range — the strings a <c>CELLRANGE</c> field stands for, by point.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A <c>CELLRANGE</c> field's own <c>a:t</c> is the placeholder and never the
    /// answer.</strong> Excel writes the literal <c>[CELLRANGE]</c> there and puts the text in
    /// <c>c:ser/c:extLst/c:ext/c15:datalabelsRange/c15:dlblRangeCache</c>, keyed by point index.
    /// <c>SeriesConverter::createDataSeries</c> reads exactly that —
    /// <c>mrModel.mrParent.mpLabelsSource-&gt;mxDataSeq-&gt;maData.find(mrModel.mnIndex)</c>, then
    /// <c>xCustomLabel-&gt;setString(oaLabelText)</c>
    /// (<c>oox/source/drawingml/chart/seriesconverter.cxx:366-410</c>), with
    /// <c>mpLabelsSource</c> bound at <c>seriescontext.cxx:461</c>.
    /// </para>
    /// <para>
    /// Measured on <c>047_Date_tracker_Gantt_chart_bf34f3a8.xlsx</c>: nine data labels reading
    /// <c>[CELLRANGE]</c> against a reference that reads <c>Activity 1</c> … <c>Activity 7</c>,
    /// <c>Today</c> and <c>Milestone 1</c> — the cache is in the file and was never opened.
    /// </para>
    /// <para>
    /// The cache and not the resolver, deliberately: the converter reads the parsed
    /// <c>DataSequenceModel::maData</c> map, which is the <c>c15:dlblRangeCache</c> the file
    /// carries, and never asks the data provider for those cells.
    /// </para>
    /// </remarks>
    private static string?[]? DataLabelsRangeOf(XElement series)
    {
        XName cache = XName.Get("dlblRangeCache", Chart2012);
        XElement? found = null;

        foreach (XElement extension in Children(Child(series, "extLst"), "ext"))
        {
            found = extension
                .Descendants(cache)
                .FirstOrDefault();

            if (found is not null) break;
        }

        if (found is null) return null;

        int declared = Drawing.Number(Child(found, "ptCount"), "val") ?? -1;

        foreach (XElement point in found.Descendants(Name("pt")))
            declared = Math.Max(declared, (Drawing.Number(point, "idx") ?? -1) + 1);

        int count = Math.Clamp(declared, 0, MaxPointCount);
        if (count == 0) return null;

        string?[] text = new string?[count];

        foreach (XElement point in found.Descendants(Name("pt")))
        {
            int index = Drawing.Number(point, "idx") ?? -1;
            if (index < 0 || index >= count) continue;

            text[index] = Child(point, "v")?.Value;
        }

        return text;
    }

    /// <summary>
    /// One point's label with its <c>CELLRANGE</c> fields resolved against the series' range.
    /// </summary>
    /// <remarks>
    /// Gated on <c>c15:showDataLabelsRange</c>, which is what <c>DataLabelContext</c> parses into
    /// <c>mobShowDataLabelsRange</c> (<c>seriescontext.cxx:134-139</c>) and what
    /// <c>seriesconverter.cxx:368</c> tests before it looks the text up at all. Without a value
    /// for this point the placeholder stands, exactly as the source leaves
    /// <c>oaLabelText.value_or("")</c> unset when the map has no entry.
    /// </remarks>
    private static ChartDataLabel? WithCellRange(
        ChartDataLabel? label,
        XElement point,
        int index,
        IReadOnlyList<string?>? dataLabelsRange)
    {
        if (label?.Parts is not { Count: > 0 } parts) return label;
        if (dataLabelsRange is null || index >= dataLabelsRange.Count) return label;
        if (dataLabelsRange[index] is not { } resolved) return label;

        bool shown = Child(Child(point, "extLst"), "ext")
                         ?.Descendants(XName.Get("showDataLabelsRange", Chart2012))
                         .Any(flag => flag.Attribute("val")?.Value is not ("0" or "false"))
                     ?? false;

        if (!shown) return label;

        List<ChartLabelPart> rewritten = new(parts.Count);
        bool changed = false;

        foreach (ChartLabelPart part in parts)
        {
            if (part.Field == ChartLabelField.CellRange)
            {
                // Literal rather than CellRange, because the field is now answered: a part still
                // carrying the field kind when the label is composed is one nothing resolved,
                // and ChartDataLabel.Resolve draws that as nothing at all.
                rewritten.Add(new ChartLabelPart(ChartLabelField.Literal, resolved));
                changed = true;
            }
            else
            {
                rewritten.Add(part);
            }
        }

        return changed ? label with { Parts = rewritten } : label;
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
    private static Colour?[]? PointFills(
        XElement series,
        int count,
        DrawingTheme? theme,
        ChartPlotKind kind = ChartPlotKind.Bar,
        ChartAutoContext automatic = default,
        int seriesIndex = -1,
        int seriesCount = 1,
        bool office2007 = false)
    {
        Colour?[]? fills = null;

        bool frame = IsFrameSeries(kind);
        bool byPoint = frame
                       && automatic.SingleGroup
                       && (automatic.VaryColours ?? !office2007)
                       && (IsPie(kind) || seriesCount == 1);
        bool varies = frame && (IsPie(kind) || byPoint);

        // A pie always colours its points from the cycle, and any other frame series does when
        // the chart says c:varyColors and holds one group with one series
        // (typegroupconverter.cxx:496-501, seriesconverter.cxx:940-962). Where the colours vary
        // it is the *point* index that walks the accents and the point count that sizes the
        // shade/tint cycle; where they do not, every point takes the series' own colour, which
        // is what a pie with c:varyColors val="0" draws. Without any of this a pie with no c:dPt
        // is one flat disc.
        if (varies && count > 0)
        {
            for (int index = 0; index < count; index++)
            {
                Colour? cycled = DrawingChartAutoFormat.ColourOf(
                    automatic.Style,
                    ChartAutoObject.FilledSeries,
                    stroke: false,
                    byPoint ? index : seriesIndex,
                    byPoint ? count - 1 : automatic.MaxSeriesIndex,
                    theme,
                    automatic.Styles);

                if (cycled is null) continue;

                fills ??= new Colour?[count];
                fills[index] = cycled;
            }
        }

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

    /// <summary>
    /// Whether a plot kind's series is drawn as an area rather than as a line.
    /// </summary>
    /// <remarks>
    /// <c>TypeGroupInfo::mbSeriesIsFrame2d</c>
    /// (<c>oox/source/drawingml/chart/typegroupconverter.cxx:92-117</c>), which is what
    /// <c>getSeriesObjectType</c> switches on to pick between the filled and linear automatic
    /// format tables. A radar chart is in both columns depending on its <c>c:radarStyle</c>, and
    /// a stock chart is linear despite drawing boxes.
    /// </remarks>
    private static bool IsFrameSeries(ChartPlotKind kind) => kind is
        ChartPlotKind.Bar or ChartPlotKind.Area or ChartPlotKind.Pie
        or ChartPlotKind.OfPie or ChartPlotKind.Bubble;

    /// <summary>Whether a plot kind is one of the three <c>TYPECATEGORY_PIE</c> types.</summary>
    private static bool IsPie(ChartPlotKind kind)
        => kind is ChartPlotKind.Pie or ChartPlotKind.OfPie;

    /// <summary>
    /// A chart's own <c>a:ln/@w</c>, or null when it states none.
    /// </summary>
    /// <remarks>
    /// Distinguished from a stated zero, which is a hairline the file asked for and must not be
    /// replaced by the theme's width. <see cref="LineWidthOf"/> cannot tell the two apart because
    /// it answers with a <c>Length</c>.
    /// </remarks>
    /// <summary>
    /// The dash array a series' <c>a:ln</c> asks for, or null for a solid line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:prstDash</c> names one of ten patterns whose lengths are percentages of the pen width,
    /// so the expansion needs the width — see <see cref="DashPresets"/>. It runs against the
    /// <em>stated</em> width and not the automatic one, because a series that states a dash states
    /// a line, and every corpus file that carries one carries <c>a:ln w="…"</c> with it.
    /// </para>
    /// <para>
    /// <c>a:ln/@cap</c> of <c>rnd</c> or <c>sq</c> shortens each ink length and lengthens the gap,
    /// which is why the cap is read here rather than defaulted: the three threshold lines of
    /// <c>southern-classic-kennesaw-state-university-final.pptx</c> are <c>cap="rnd"</c> and would
    /// otherwise be drawn a third longer than the reference's dots.
    /// </para>
    /// </remarks>
    private static IReadOnlyList<Length>? DashOf(XElement? properties)
    {
        XElement? line = Drawing.Child(properties, "ln");
        if (line is null) return null;

        return DashPresets.Pattern(
            Drawing.Attribute(Drawing.Child(line, "prstDash"), "val"),
            StatedLineWidth(properties) ?? Length.Zero,
            CapOf(properties) != LineCap.Butt);
    }

    /// <summary>
    /// <c>a:ln/@cap</c>, which decides both how a line ends and what its dashes look like.
    /// </summary>
    /// <remarks>
    /// <c>flat</c> and an absent attribute are butt — <c>ST_LineCap</c>'s default is <c>flat</c>
    /// and <c>LineProperties::pushToPropMap</c> maps it to <c>DrawingLineCap_BUTT</c>. The other
    /// two matter because <see cref="DashPresets"/> has already taken 99% off each ink length on
    /// their account: drawn butt, that array is a row of hairlines where the file asked for dots.
    /// </remarks>
    private static LineCap CapOf(XElement? properties) =>
        Drawing.Attribute(Drawing.Child(properties, "ln"), "cap") switch
        {
            "rnd" => LineCap.Round,
            "sq" => LineCap.Square,
            _ => LineCap.Butt,
        };

    /// <summary>
    /// How wide a series' own line is drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Three cases, and the middle one is the one that is easy to miss. A series stating
    /// <c>a:ln/@w</c> takes that width. A series stating <strong>no <c>a:ln</c> at all</strong>
    /// takes the automatic entry — the theme's subtle line times the chart style's
    /// <c>mnRelLineWidth</c>, which is 300 % at style 1 and 500 % at style 18. And a series that
    /// states an <c>a:ln</c> <em>without</em> a <c>w</c> takes neither: it is drawn at a flat
    /// <see cref="SeriesLineDefault"/>.
    /// </para>
    /// <para>
    /// Measured at 26.2.4.2 on one fixture varied one attribute at a time, read back through its
    /// own <c>--convert-to ods</c> so the number is the reference's resolved model rather than a
    /// rasterised guess (<c>probes/stroke-resid-r117/results.md</c> §4.4):
    /// </para>
    /// <list type="bullet">
    ///   <item><description>
    ///     no <c>c:spPr</c>, theme 9525, <c>c:style val="1"</c> → <c>svg:stroke-width="0.079cm"</c>
    ///     = 9525 × 300 % as a whole 1/100 mm.
    ///   </description></item>
    ///   <item><description>
    ///     no <c>c:spPr</c>, theme 9525, <c>c:style val="18"</c> → <c>0.132cm</c> = 9525 × 500 %.
    ///     Wrapping the style in the <c>mc:AlternateContent</c> that pairs
    ///     <c>c14:style val="118"</c> with a <c>c:style val="18"</c> fallback changes nothing,
    ///     which is the fallback winning as <see cref="DrawingChartAutoFormat.StyleOf"/> says.
    ///   </description></item>
    ///   <item><description>
    ///     the same chart with <c>&lt;c:spPr&gt;&lt;a:ln&gt;&lt;a:solidFill/&gt;&lt;/a:ln&gt;</c>
    ///     and no <c>w</c> → <c>0.035cm</c>, and <strong>raising the theme's subtle line to 38100
    ///     moves every gridline from 0.026 to 0.106 cm and leaves the series at 0.035</strong>. So
    ///     it is a constant and not a scaled theme width.
    ///   </description></item>
    ///   <item><description>
    ///     and it is not a property of the series being drawn as a line: a <em>bar</em> series and
    ///     a <em>pie</em> series stating the same <c>a:ln</c> at the same style are both
    ///     <c>0.035cm</c> too, even though <c>spFilledSeriesLines</c> is
    ///     <c>AUTOFORMAT_INVISIBLE</c> over styles 17-32 and so gives them no automatic width at
    ///     all to replace.
    ///   </description></item>
    /// </list>
    /// <para>
    /// A <c>c:dPt</c> is the exception and <see cref="PointsStateTheirOwnLine"/> carries it.
    /// </para>
    /// <para>
    /// Reading it the other way — applying the relative multiplier to a series that states an
    /// <c>a:ln</c> — draws <c>064_Small_business_cash_flow</c>'s alert line at 3.74 pt against the
    /// reference's 0.99, which is what this rule was found by.
    /// </para>
    /// <para>
    /// <strong>Where 35 hundredths of a millimetre comes from is not established.</strong> It is
    /// chart2's own answer once the OOXML importer has set no width, and it could not be located
    /// in the 27.2 tree; the measurements above are what this rests on.
    /// </para>
    /// </remarks>
    /// <param name="element">The <c>c:ser</c>, for its <c>c:dPt</c> children.</param>
    /// <param name="properties">The series' <c>c:spPr</c>.</param>
    /// <param name="automatic">The chart-space automatic-format context.</param>
    /// <param name="frame">Whether the series is drawn as a line or as an area.</param>
    /// <param name="theme">The theme, for the automatic entry.</param>
    /// <param name="seriesIndex">The series' <c>c:idx</c>.</param>
    private static Length SeriesLineWidth(
        XElement element,
        XElement? properties,
        ChartAutoContext automatic,
        ChartAutoObject frame,
        DrawingTheme? theme,
        int seriesIndex)
    {
        if (StatedLineWidth(properties) is { } stated) return stated;

        if (Drawing.Child(properties, "ln") is null)
        {
            return AutoLineWidth(automatic, frame, theme, seriesIndex);
        }

        return PointsStateTheirOwnLine(element) ? Length.Zero : SeriesLineDefault;
    }

    /// <summary>
    /// Whether every mark this series draws takes its outline from a <c>c:dPt</c> rather than from
    /// the series.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>c:dPt</c>'s own <c>a:ln</c> is resolved per point, and a point's answer is <em>not</em>
    /// the series' — measured on the pie fixture with one <c>c:dPt</c> added, whose <c>a:ln</c>
    /// states a colour and no <c>w</c>: 26.2.4.2 strokes that one wedge at <c>0 w</c> and the other
    /// three, which take the series' identical <c>a:ln</c>, at 0.99036. So a stated <c>a:ln</c> with
    /// no width is <see cref="SeriesLineDefault"/> on a series and a hairline on a point.
    /// </para>
    /// <para>
    /// <strong>This model has no per-point line width</strong> — <c>ChartSeries.PointFills</c> has
    /// no companion — so every mark takes the series' one. This asks whether any point states an
    /// <c>a:ln</c> of its own and, when one does, gives the whole series the point's answer. That is
    /// exact for a series where <em>every</em> point states one, which is the shape that occurs:
    /// <c>bitesize-writing-a-report.pptx</c>'s pie carries ten of them and 26.2.4.2 strokes all
    /// twenty of its wedge outlines at <c>0 w</c>. It is wrong for a series where only some points
    /// state one, and the proper fix is a per-point width in the model rather than this.
    /// </para>
    /// </remarks>
    /// <param name="element">The <c>c:ser</c>.</param>
    private static bool PointsStateTheirOwnLine(XElement element)
    {
        foreach (XElement point in Children(element, "dPt"))
        {
            if (Drawing.Child(Child(point, "spPr"), "ln") is not null) return true;
        }

        return false;
    }

    /// <summary>35 hundredths of a millimetre — see <see cref="SeriesLineWidth"/>.</summary>
    private static readonly Length SeriesLineDefault = Length.FromMm100(35);

    private static Length? StatedLineWidth(XElement? properties)
        => Drawing.Number(Drawing.Child(properties, "ln"), "w") is { } emu
            ? DrawingChartAutoFormat.LineWidth(Math.Max(emu, 0))
            : null;

    /// <summary>
    /// How wide a series' automatic line is: the theme's subtle line style scaled by the chart
    /// style's relative width.
    /// </summary>
    /// <remarks>
    /// <c>LineFormatter</c> takes <c>Theme::getLineStyle(THEMED_STYLE_SUBTLE)</c> and multiplies
    /// its width by <c>mnRelLineWidth / 100</c> (<c>objectformatter.cxx:826-853</c>). Every theme
    /// Office ships states 9525 EMU there, so a chart at the default style draws its lines at
    /// 2.25 pt — which against a hairline is the difference between a chart and a wireframe.
    /// Null when there is no format matrix to ask, so nothing is invented.
    /// </remarks>
    private static Length AutoLineWidth(
        ChartAutoContext automatic, ChartAutoObject frame, DrawingTheme? theme, int seriesIndex)
    {
        if (automatic.Styles is not { } styles) return Length.Zero;
        if (seriesIndex < 0) return Length.Zero;

        int relative = DrawingChartAutoFormat.RelativeLineWidth(automatic.Style, frame);
        if (relative <= 0) return Length.Zero;

        XElement? line = styles.LineStyle(DrawingChartAutoFormat.SubtleStyleIndex);
        if (Drawing.Number(line, "w") is not { } emu || emu <= 0) return Length.Zero;

        _ = theme;
        return DrawingChartAutoFormat.LineWidth(emu * relative / 100);
    }

    /// <summary>A shape property bag's solid fill, or null when it has none.</summary>
    /// <remarks>
    /// <para>
    /// <c>a:solidFill</c>, and failing that the middle stop of an <c>a:gradFill</c>. A chart's
    /// model carries one colour per series rather than a paint, so a gradient cannot be drawn as
    /// one here — but reading it as "no fill" draws no bar at all, and a bar in one of its own
    /// colours is much nearer the reference than an empty row. Measured on
    /// <c>N2_E_Maestroni_Swarm_COP.pptx</c>, a stacked Gantt whose first series is a
    /// three-stop gradient with an <c>a:alpha</c> on every stop: the reference draws 111 pale
    /// bars and we drew none, so the chart read as having one bar per row instead of two.
    /// </para>
    /// <para>
    /// The stop nearest the middle rather than the first, because a DrawingML gradient's end
    /// stops are routinely its extremes — a <c>tint</c> at one end and a <c>shade</c> at the
    /// other — and the middle is what the shape reads as at a glance. Alpha survives, so a
    /// gradient the file made translucent stays translucent.
    /// </para>
    /// </remarks>
    private static Colour? FillOf(XElement? properties, DrawingTheme? theme)
    {
        if (Drawing.Child(properties, "solidFill") is { } fill)
        {
            foreach (XElement child in fill.Elements())
                if (DrawingColour.Read(child) is { } colour) return colour.Resolve(theme);

            return null;
        }

        if (DrawingFill.ReadGradient(Drawing.Child(properties, "gradFill")) is not { } gradient)
            return null;

        DrawingGradientStop? middle = null;
        double best = double.MaxValue;

        foreach (DrawingGradientStop stop in gradient.Stops)
        {
            double distance = Math.Abs(stop.Position - 0.5);
            if (distance >= best) continue;

            best = distance;
            middle = stop;
        }

        return middle?.Colour.Resolve(theme);
    }

    private static Colour? LineOf(XElement? properties, DrawingTheme? theme)
    {
        XElement? line = Drawing.Child(properties, "ln");
        if (line is null) return null;
        if (Drawing.Child(line, "noFill") is not null) return null;
        return FillOf(line, theme);
    }

    /// <summary>
    /// Whether these shape properties state <em>no line at all</em>, as against stating nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="LineOf"/> returns null for both, which is right for a colour and wrong for the
    /// question a caller with a fallback is asking. <c>c:ser/c:spPr/a:ln/a:noFill</c> is a
    /// <em>suppression</em>: <c>LineFormatter::convertFormatting</c> resolves it to
    /// <c>LineStyle_NONE</c> (<c>objectformatter.cxx:857-889</c> through
    /// <c>LineProperties::pushToPropMap</c>), so the automatic colour the chart's style would
    /// otherwise have given the series must not be substituted for it.
    /// </para>
    /// <para>
    /// Absence is the opposite and stays the opposite: a series with no <c>a:ln</c> is what the
    /// automatic table exists for. Collapsing the two is why a scatter series whose file says it
    /// has no line was carrying one in <see cref="ChartSeries.Line"/> — invisible in the polyline,
    /// which <see cref="ChartSeries.HasLine"/> already suppressed, and visible in every consumer
    /// that reads the colour without consulting that flag.
    /// </para>
    /// </remarks>
    private static bool SuppressesLine(XElement? properties)
        => Drawing.Child(Drawing.Child(properties, "ln"), "noFill") is not null;

    /// <summary>
    /// Whether these shape properties state <em>no fill at all</em>, as against stating nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same distinction <see cref="SuppressesLine"/> draws, one element up.
    /// <c>c:ser/c:spPr/a:noFill</c> resolves to <c>FillStyle_NONE</c> and must not be replaced by
    /// the colour the chart's style would otherwise give the series; a <c>c:spPr</c> with no fill
    /// element at all is the case the automatic table exists for.
    /// </para>
    /// <para>
    /// <strong>Found by a blind reviewer, from the picture alone.</strong> Sent
    /// <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c>'s page 8 with no access to the package, it
    /// reported that ours caps each column with a silver block reaching 100% where the reference
    /// shows only the plot background — and listed "drawn but with a fill equal to the background"
    /// among the causes it could not separate from "not drawn". The file settles it: that series,
    /// <c>Non suivi</c>, is <c>&lt;c:spPr&gt;&lt;a:noFill/&gt;&lt;/c:spPr&gt;</c> and nothing else.
    /// </para>
    /// </remarks>
    private static bool SuppressesFill(XElement? properties)
        => Drawing.Child(properties, "noFill") is not null;

    /// <summary>A series' <c>c:marker/c:spPr</c>, or null when it states none.</summary>
    private static XElement? MarkerProperties(XElement? series)
        => Child(Child(series, "marker"), "spPr");

    /// <summary>
    /// The side of a series' marker, which an OOXML chart always has.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>c:marker/c:size</c> is in whole points and
    /// <c>TypeGroupConverter::convertMarker</c> makes the symbol
    /// <c>convertPointToMm100(nOoxSize)</c> square (<c>typegroupconverter.cxx:652-654</c>).
    /// That conversion is <c>o3tl::convert(n, pt, mm100)</c>, which is
    /// <c>round(n × 2540 / 72)</c> — so a stated 6 is 212 hundredths of a millimetre and
    /// <b>6.0094 pt</b> rather than 6, and the hundredth matters because it is what the
    /// reference's own operators carry.
    /// </para>
    /// <para>
    /// <strong>This never returns null, and that is the rule rather than an oversight.</strong>
    /// <c>SeriesModel</c>'s constructor is <c>mnMarkerSize( 5 )</c>
    /// (<c>seriesmodel.cxx:118</c>) and <c>convertMarker</c> is reached for every series of a
    /// chart type that is not a <c>seriesFrameFormat</c>, so an OOXML marker's size is stated or
    /// it is five points — it is never chart2's unset 250 × 250 default. A series whose chart
    /// type <em>is</em> a frame format never draws a marker at all, so the value is inert there.
    /// The null that <see cref="ChartSeries.MarkerSize"/> documents belongs to the ODF and binary
    /// readers, which have no such element and must keep the 250.
    /// </para>
    /// <para>
    /// Out-of-range values are clamped the way the schema states the type: <c>c:size/@val</c> is
    /// 2 … 72, and one corpus part states 62. Anything outside that is treated as unstated,
    /// which is what a reader that cannot trust its input should do with a number it cannot draw.
    /// </para>
    /// </remarks>
    private static Length MarkerSizeOf(XElement? series)
    {
        const int defaultPoints = 5;

        double points = Number(Child(Child(series, "marker"), "size")) is { } stated
                        && stated is >= 2 and <= 72
            ? stated
            : defaultPoints;

        return Length.FromMm100((long)Math.Round(points * 2540.0 / 72.0,
                                                 MidpointRounding.AwayFromZero));
    }

    /// <summary>
    /// The colour a marker is filled in when it states shape properties of its own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>TypeGroupConverter::convertMarker</c> (<c>typegroupconverter.cxx:657-678</c>) takes
    /// <c>xShapeProps->getFillProperties().maFillColor</c> for the symbol's fill and, when there
    /// is none, falls back to the symbol's <em>line</em> colour — the fix for tdf#124817, whose
    /// comment says so in as many words. Both halves matter on this corpus: the marker is stated
    /// with a <c>a:solidFill</c> on <c>FAAAI…</c> and with a three-stop <c>a:gradFill</c> on
    /// <c>8_P-Pavese_AIRBUS…</c>, where only the fallback finds a colour at all.
    /// </para>
    /// <para>
    /// <strong>A gradient is deliberately not read here, though <see cref="FillOf"/> reads one
    /// for a series.</strong> <c>maFillColor</c> is set by <c>a:solidFill</c> alone, so the
    /// reference genuinely does not see the gradient's stops, and taking the middle stop instead
    /// would draw the AIRBUS markers in a colour LibreOffice never computes. The two policies
    /// differ because the questions do: an unfilled bar is a missing row, an unfilled marker has
    /// a line colour standing behind it.
    /// </para>
    /// </remarks>
    private static Colour? MarkerFillOf(XElement? series, DrawingTheme? theme)
    {
        XElement? properties = MarkerProperties(series);
        if (properties is null) return null;

        if (Drawing.Child(properties, "solidFill") is { } fill)
        {
            foreach (XElement child in fill.Elements())
                if (DrawingColour.Read(child) is { } colour) return colour.Resolve(theme);
        }

        return LineOf(properties, theme);
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
            ? DrawingChartAutoFormat.LineWidth(emu)
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
        foreach (XElement properties in RunProperties(element))
        {
            if (Drawing.Number(properties, "sz") is not { } hundredths || hundredths <= 0) continue;

            return Length.FromPoints(hundredths / 100.0);
        }

        return null;
    }

    /// <summary>
    /// The character-property elements under a titled element, <strong>runs first</strong>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>c:rich</c> writes its paragraph's <c>a:pPr/a:defRPr</c> before the runs it defaults,
    /// so document order puts the *default* first and taking the first of either reads the value
    /// the run overrides rather than the value the run states. That is not a corner: on
    /// <c>003_advanced_excel_pie</c> the title's paragraph default is <c>sz="1300" b="0"</c> in
    /// Arial and its single run is <c>sz="1800" b="1"</c> in Calibri, and LibreOffice 26.2.4.2
    /// draws the run — <strong>18.01 pt Carlito Bold</strong>, measured off its own PDF, against
    /// the 13.00 pt Liberation Sans this used to produce.
    /// </para>
    /// <para>
    /// The fallback is what keeps every other caller unchanged: an axis' <c>c:txPr</c> and a
    /// <c>c:dLbls</c> hold a paragraph and no runs at all, so there is no <c>a:rPr</c> to prefer
    /// and the <c>a:defRPr</c> is read exactly as before. Censused over all 946 corpus documents:
    /// 169 hold a chart part and <strong>39 hold a run that states something different from its
    /// paragraph's default</strong> — 37 sheets, one deck and one document.
    /// </para>
    /// <para>
    /// A title of several runs in different faces still collapses to one answer, because the
    /// model carries one size, one weight and one family per titled element. The first run is a
    /// better single answer than the paragraph default it overrides.
    /// </para>
    /// </remarks>
    private static IEnumerable<XElement> RunProperties(XElement? element)
    {
        if (element is null) yield break;

        foreach (bool runsOnly in (bool[])[true, false])
        {
            foreach (XElement properties in element.Descendants())
            {
                if (properties.Name.NamespaceName != OoxmlNamespaces.DrawingML) continue;
                if (properties.Name.LocalName != (runsOnly ? "rPr" : "defRPr")) continue;

                yield return properties;
            }
        }
    }

    /// <summary>
    /// The family a chart part's text is set in, or null when neither the part nor the theme
    /// names one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The <c>c:chartSpace</c>'s own <c>c:txPr</c> first — the part's <em>global</em> statement,
    /// which is the same element <see cref="AutoText"/> reads a global size from — then the first
    /// <em>literal</em> <c>a:latin/@typeface</c> anywhere in the part, then the theme's minor
    /// Latin face. Anything beginning with a plus — <c>+mn-lt</c>, <c>+mj-lt</c> — is a
    /// <em>reference</em> to the theme rather than a name, so taking it as one asks the resolver
    /// for a family no system has and every label is measured in a fallback.
    /// </para>
    /// <para>
    /// <strong>Document order is not a precedence rule, and reading it as one cost a whole
    /// deck.</strong> <c>c:chart</c> precedes <c>c:txPr</c> under <c>c:chartSpace</c>, so a part
    /// whose <em>title</em> names a face and whose chart space names another had every axis
    /// label, legend entry and data label drawn in the title's. Measured on page 38 of
    /// <c>171128IPAP.pptx</c>, whose <c>chart7.xml</c> states <c>Arial</c> on
    /// <c>c:title/c:txPr</c> and <c>Calibri</c> on <c>c:chartSpace/c:txPr</c>: the reference draws
    /// 31 records in Carlito-Bold and 2 in LiberationSans — its title, in Arial — and we drew 34
    /// in LiberationSans-Bold. Two of the corpus's 61 chart parts state two faces this way.
    /// </para>
    /// <para>
    /// The title's own face is still lost, because the model carries one family: what this
    /// changes is which of the two the whole chart takes, and the chart-wide one is right for
    /// every element but the title.
    /// </para>
    /// <para>
    /// <strong>Falling back to the theme's minor face is not a guess.</strong> All three of the
    /// automatic text entries LibreOffice's chart import carries — chart titles, axis titles, and
    /// everything else — name <c>XML_minor</c>
    /// (<c>oox/source/drawingml/chart/objectformatter.cxx</c>:415-434), and the <c>c:txPr</c> most
    /// decks write states <c>+mn-lt</c>, which resolves to the same face. Measured on the slides
    /// corpus: of the fifteen decks holding chart parts, ten state <c>+mn-lt</c> and four state
    /// nothing at all, so the theme is what decides fourteen of them.
    /// </para>
    /// <para>
    /// This is the same rule <c>DocxPictures.LabelFamily</c> already applies on the words track,
    /// deliberately: it is one reader serving three families, and two nearly-identical rules in
    /// two places is how they come apart. Only the final fallback differs — that one ends in
    /// <c>"Calibri"</c> and this ends in null, because which face "nothing stated" means is the
    /// consumer's question and a slide, a sheet and a text frame answer it through different
    /// caches. See <see cref="ChartPlot.TextFamily"/>.
    /// </para>
    /// </remarks>
    private static string? FamilyOf(XElement chartSpace, DrawingTheme? theme)
        => LiteralFamily(Child(chartSpace, "txPr"))
           ?? LiteralFamily(chartSpace)
           ?? theme?.Fonts?.MinorLatin;

    /// <summary>The face the legend's entries are set in.</summary>
    /// <remarks>
    /// <para>
    /// <strong>The legend's own <c>c:txPr</c>, then the chart space's, then the theme's minor
    /// face — and never some other element's.</strong> <see cref="FamilyOf"/>'s middle term is a
    /// search of the whole part, which is a reasonable approximation for a chart whose objects
    /// all state the same thing and wrong for one whose axes state a face its legend does not.
    /// </para>
    /// <para>
    /// <c>001_advanced_powerpoint_bar.pptx</c> is the case, and it is 33 of the corpus' slides
    /// decks and 36 of its sheets ones: <c>c:catAx/c:txPr</c> and <c>c:valAx/c:txPr</c> state
    /// <c>Arial</c>, <c>c:legend</c> and <c>c:chartSpace</c> state nothing, and 26.2.4.2 draws the
    /// axis labels in LiberationSans and the legend in Carlito — the theme's Calibri. That is
    /// <c>ObjectFormatter</c>'s automatic text table, which names <c>XML_minor</c> for every
    /// automatic entry (<c>objectformatter.cxx</c>:415-434) and lets an object's own
    /// <c>c:txPr</c> override it for that object alone.
    /// </para>
    /// <para>
    /// Null when the part states nothing and there is no theme, which leaves
    /// <see cref="ChartPlot.TextFamily"/> deciding exactly as before.
    /// </para>
    /// </remarks>
    private static string? LegendFamilyOf(
        XElement? chart, XElement chartSpace, DrawingTheme? theme)
        => LiteralFamily(Child(Child(chart, "legend"), "txPr"))
           ?? LiteralFamily(Child(chartSpace, "txPr"))
           ?? theme?.Fonts?.MinorLatin;

    /// <summary>The first literal <c>a:latin/@typeface</c> under an element, or null.</summary>
    private static string? LiteralFamily(XElement? element)
    {
        if (element is null) return null;

        // A run's own face before the paragraph default it overrides, for the reason
        // `RunProperties` gives; then any other literal face under the element, which is what
        // reaches a `c:txPr` that states one outside a run.
        foreach (XElement properties in RunProperties(element))
        {
            if (Face(properties.Element(XName.Get("latin", OoxmlNamespaces.DrawingML))) is { } named)
                return named;
        }

        foreach (XElement latin in element.Descendants(
                     XName.Get("latin", OoxmlNamespaces.DrawingML)))
        {
            if (Face(latin) is { } named) return named;
        }

        return null;

        static string? Face(XElement? latin)
        {
            string? typeface = latin?.Attribute("typeface")?.Value;
            if (string.IsNullOrWhiteSpace(typeface)) return null;

            return typeface[0] == '+' ? null : typeface;
        }
    }

    /// <summary>
    /// The size a chart part's automatic text takes when the part itself states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An OOXML chart never reaches <c>chart2</c>'s model defaults.</strong> The import
    /// applies its own auto-text table before any of them
    /// (<c>oox/source/drawingml/chart/objectformatter.cxx</c>:415-434, applied by
    /// <c>TextFormatter::TextFormatter</c>:906-929): a chart title is <c>1800</c> and bold, an
    /// axis title <c>1000</c> and bold, and everything else — axis labels, legend entries, data
    /// labels — <c>1000</c> and not bold. Citing <c>chart2/source/model/main/Title.cxx</c>'s 13 pt
    /// here was right for an ODF chart and wrong for every OOXML one.
    /// </para>
    /// <para>
    /// <paramref name="relative"/> is <c>mnRelFontSize</c>, and it bites only when the
    /// <em>chart space</em> states a size of its own: <c>TextFormatter</c> keeps the absolute
    /// default until the global <c>c:txPr</c> supplies a height, and then takes that height times
    /// the percentage instead (<c>:926-928</c>). So a chart whose <c>c:txPr</c> says 1400 gets a
    /// 16.8 pt title, not an 18 pt one. Six of the slides corpus's 61 chart parts state it.
    /// </para>
    /// <para>
    /// Measured against LibreOffice's own model rather than only against its ink:
    /// <c>Demick_JetBlue.pptx</c>, whose five chart parts state no <c>sz</c> and no <c>b</c>
    /// anywhere, round-trips through <c>--convert-to odp</c> with the chart title carrying
    /// <c>fo:font-size="18pt" fo:font-weight="bold"</c>, its two axis titles
    /// <c>10pt</c>/<c>bold</c>, and its axes and legend <c>10pt</c> with no weight at all.
    /// </para>
    /// </remarks>
    /// <param name="chartSpace">The <c>c:chartSpace</c>, whose direct <c>c:txPr</c> is the global one.</param>
    /// <param name="points">The table's absolute default, in points.</param>
    /// <param name="relative">The table's percentage of the global size.</param>
    private static Length AutoText(XElement chartSpace, double points, int relative)
        => SizeOf(Child(chartSpace, "txPr")) is { } global
            ? global * (relative / 100.0)
            : Length.FromPoints(points);

    /// <summary>
    /// The weight the <em>chart space</em>'s own <c>c:txPr</c> states, or null when it states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>It sits between the auto-text table and the object's own statement, and it is the
    /// same element <see cref="AutoText"/> already reads the size out of.</strong>
    /// <c>TextFormatter</c> seeds its automatic properties from the table — a title bold, an axis
    /// title bold, everything else regular — and then calls
    /// <c>mxAutoText-&gt;assignUsed(*pTextProps)</c> with the chart space's own
    /// <c>c:txPr</c>'s first paragraph's <c>a:defRPr</c>
    /// (<c>oox/source/drawingml/chart/objectformatter.cxx</c>:906-929, the properties fetched by
    /// <c>lclGetTextProperties</c>, <c>:897-901</c>, and the chart space's text body handed in at
    /// <c>:950</c>). <c>assignUsed</c> copies every property the source <em>states</em>, so a
    /// global <c>b</c> replaces the table's weight for every piece of the chart's text that does
    /// not state one of its own — and a global <c>b="0"</c> makes an otherwise-bold title
    /// regular just as a <c>b="1"</c> makes a regular axis label bold.
    /// </para>
    /// <para>
    /// <strong>Reach: 17 chart parts in 8 corpus documents, and both directions occur.</strong>
    /// Twelve parts in six documents state <c>b="0"</c> and five parts in two state
    /// <c>b="1"</c>; <c>029_Annual_budget</c> is the witness for the second, where every axis
    /// label, category label and data label on its three charts is drawn bold by the reference.
    /// See <c>probes/chart-secaxis/results.md</c>.
    /// </para>
    /// </remarks>
    /// <param name="chartSpace">The <c>c:chartSpace</c>, whose direct <c>c:txPr</c> is the global one.</param>
    private static bool? AutoWeight(XElement chartSpace) => BoldOf(Child(chartSpace, "txPr"));

    /// <summary>
    /// Whether a titled element's text states a weight, and which — <c>@b</c> on the first
    /// <c>a:defRPr</c> or <c>a:rPr</c> under it, in document order.
    /// </summary>
    /// <remarks>
    /// Null rather than false when nothing states one, because "stated regular" and "stated
    /// nothing" are different answers here: the auto-text table makes an unstated chart title
    /// bold, so collapsing the two would draw <c>b="0"</c> bold. Five of the corpus's chart parts
    /// state <c>b="0"</c> on a title and three state <c>b="1"</c>.
    /// </remarks>
    private static bool? BoldOf(XElement? element)
    {
        foreach (XElement properties in RunProperties(element))
        {
            if (properties.Attribute("b")?.Value is not { Length: > 0 } stated) continue;

            return stated is "1" or "true";
        }

        return null;
    }

    /// <summary>The weight the axis <em>titles</em> state, from the first axis that states one.</summary>
    /// <remarks>
    /// One answer for every axis title, which is the same simplification
    /// <see cref="AxisTitleSizeOf"/> already makes and for the same reason: the model carries one
    /// axis-title size and one axis-title weight. Of the corpus's 38 axis titles, 13 state a
    /// weight and no document states both values on different axes of one chart.
    /// </remarks>
    private static bool? AxisTitleBoldOf(XElement plotArea)
    {
        foreach (XElement axis in plotArea.Elements())
        {
            if (axis.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!axis.Name.LocalName.EndsWith("Ax", StringComparison.Ordinal)) continue;
            if (BoldOf(Child(axis, "title")) is { } bold) return bold;
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
    /// <summary>The weight the axis <em>labels</em> state, from the first axis that states one.</summary>
    /// <remarks>
    /// The same one-answer-per-chart simplification <see cref="AxisLabelSizeOf"/> makes, and the
    /// model carries one label weight for the same reason it carries one label size. Read from
    /// the axis' own <c>c:txPr</c> rather than from its descendants, because a <c>c:title</c>
    /// under the same axis carries a <c>c:txPr</c> of its own and states the <em>title's</em>
    /// weight — which is a different question and already has its own reader.
    /// </remarks>
    private static bool? AxisLabelBoldOf(XElement plotArea)
    {
        foreach (XElement axis in plotArea.Elements())
        {
            if (axis.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!axis.Name.LocalName.EndsWith("Ax", StringComparison.Ordinal)) continue;
            if (BoldOf(Child(axis, "txPr")) is { } bold) return bold;
        }

        return null;
    }

    /// <summary>
    /// The size a series' <em>data</em> labels state, from the first series that states one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>c:dLbls</c> hangs off the series (and, for a whole type group, off the group), and its
    /// own <c>c:txPr</c> is a different statement from the axes'. Read from the <c>c:dLbls</c>
    /// element's direct <c>c:txPr</c> child rather than from its descendants, because a
    /// <c>c:dLbl</c> for one point carries a <c>c:txPr</c> of its own and precedes it — the same
    /// trap the legend's reader documents.
    /// </para>
    /// <para>
    /// One answer for every series, which is the simplification the model already makes for the
    /// axes and the axis titles. Of the corpus's 61 chart parts, 18 state a data-label size and
    /// none states two different ones across its own series.
    /// </para>
    /// </remarks>
    private static Length? DataLabelSizeOf(XElement plotArea)
        => DataLabelProperties(plotArea).Select(SizeOf).FirstOrDefault(size => size is not null);

    /// <summary>The weight a series' data labels state, from the first series that states one.</summary>
    /// <remarks>
    /// Read beside the size because it comes from the same element, and separate from
    /// <see cref="AxisLabelBoldOf"/> because an unstated data-label weight must keep falling back
    /// to the axis labels' — see <c>ChartPlot.IsDataLabelBold</c>.
    /// </remarks>
    private static bool? DataLabelBoldOf(XElement plotArea)
        => DataLabelProperties(plotArea).Select(BoldOf).FirstOrDefault(bold => bold is not null);

    /// <summary>Every <c>c:dLbls/c:txPr</c> in the plot area, series before type group.</summary>
    private static IEnumerable<XElement> DataLabelProperties(XElement plotArea)
    {
        foreach (XElement group in plotArea.Elements())
        {
            if (group.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;

            foreach (XElement series in Children(group, "ser"))
            {
                if (Child(Child(series, "dLbls"), "txPr") is { } stated) yield return stated;
            }

            if (Child(Child(group, "dLbls"), "txPr") is { } shared) yield return shared;
        }
    }

    /// <summary>
    /// The colour a run of chart text states, resolved, or null when it states none.
    /// </summary>
    /// <remarks>
    /// The same descent <see cref="SizeOf"/> makes — a real <c>a:rPr</c> first, then the
    /// <c>a:defRPr</c> that stands in for one — because a <c>c:title</c> carries its colour on
    /// its runs and a <c>c:txPr</c> carries it on the paragraph's default, and a title that
    /// states both must answer with the run's.
    /// </remarks>
    private static Colour? ColourOf(XElement? element, DrawingTheme? theme)
    {
        foreach (XElement properties in RunProperties(element))
        {
            if (Drawing.Child(properties, "solidFill") is not { } fill) continue;

            foreach (XElement child in fill.Elements())
                if (DrawingColour.Read(child)?.Resolve(theme) is { } colour) return colour;
        }

        return null;
    }

    /// <summary>The tick-label colour, from whichever axis states one first.</summary>
    /// <remarks>The shape of <see cref="AxisLabelSizeOf"/>, over <c>c:txPr</c>.</remarks>
    private static Colour? AxisLabelColourOf(XElement plotArea, DrawingTheme? theme)
    {
        foreach (XElement axis in plotArea.Elements())
        {
            if (axis.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!axis.Name.LocalName.EndsWith("Ax", StringComparison.Ordinal)) continue;
            if (ColourOf(Child(axis, "txPr"), theme) is { } colour) return colour;
        }

        return null;
    }

    /// <summary>The axis-title colour, from whichever axis title states one first.</summary>
    /// <remarks>The shape of <see cref="AxisTitleSizeOf"/>.</remarks>
    private static Colour? AxisTitleColourOf(XElement plotArea, DrawingTheme? theme)
    {
        foreach (XElement axis in plotArea.Elements())
        {
            if (axis.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!axis.Name.LocalName.EndsWith("Ax", StringComparison.Ordinal)) continue;
            if (ColourOf(Child(axis, "title"), theme) is { } colour) return colour;
        }

        return null;
    }

    /// <summary>
    /// The data-label colour, from the plot area's own <c>c:dLbls</c> or from a group's.
    /// </summary>
    /// <remarks>
    /// A <c>c:dLbls</c> hangs off the plot area, off a type group and off each <c>c:ser</c>, and
    /// the outermost one that states a colour is taken. Per-series data-label colours are not
    /// modelled: <see cref="ChartPlot.DataLabelColour"/> is one colour for the plot, which is
    /// what every corpus chart that states one uses it as.
    /// </remarks>
    private static Colour? DataLabelColourOf(XElement plotArea, DrawingTheme? theme)
    {
        if (ColourOf(Child(plotArea, "dLbls"), theme) is { } stated) return stated;

        foreach (XElement group in plotArea.Elements())
        {
            if (group.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!group.Name.LocalName.EndsWith("Chart", StringComparison.Ordinal)) continue;
            if (ColourOf(Child(group, "dLbls"), theme) is { } colour) return colour;
        }

        return null;
    }

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

    /// <summary>
    /// An axis title's text, with LibreOffice's own caption where the element states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A <c>c:title</c> with no text in it is still a title, and the reference draws
    /// <c>Axis Title</c> in it.</strong> <c>AxisConverter::convertFromModel</c> passes
    /// <c>OoxResId(STR_DIAGRAM_AXISTITLE)</c> as the default string to
    /// <c>TitleConverter::convertFromModel</c> whenever the model holds a title element and the
    /// type group is not a radar (<c>oox/source/drawingml/chart/axisconverter.cxx:461-470</c>),
    /// and <c>TextConverter::createStringSequence</c> falls back to that default once the rich
    /// body, the <c>c:txPr</c> paragraphs and the <c>c:tx</c> cache have all come back empty
    /// (<c>titleconverter.cxx:141-160</c>). Unlike the chart title's, this arm has no
    /// tdf#146487 escape and no <c>autoTitleDeleted</c>: the element being present is the whole
    /// condition.
    /// </para>
    /// <para>
    /// Measured on <c>040_Blood_pressure_tracker_872b6833.xlsx</c>, whose category axis carries
    /// <c>&lt;c:title&gt;</c> with a <c>c:overlay</c>, a <c>c:spPr</c> and a <c>c:txPr</c> and no
    /// <c>c:tx</c> at all: the reference prints <c>Axis Title</c> under the axis and we printed
    /// nothing.
    /// </para>
    /// </remarks>
    private static string? AxisTitleText(XElement? title, ChartPlotKind kind)
    {
        if (title is null) return null;
        if (TitleText(title) is { Length: > 0 } stated) return stated;

        // "in radar charts, title objects may exist, but are not shown".
        return kind is ChartPlotKind.Radar ? null : DrawingChartTitle.AxisTitle;
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

    /// <summary>
    /// The name a Calc-hosted series with no <c>c:tx</c> at all is given — <c>Column C</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An unnamed series is not a series without a legend entry.</strong>
    /// <c>DataSeries::getLabelForRole</c> falls through to
    /// <c>xValueSeq-&gt;generateLabel(LabelOrigin_SHORT_SIDE)</c> when the label sequence is
    /// absent or empty (<c>chart2/source/model/main/DataSeries.cxx:641-672</c>), and Calc answers
    /// it: <c>ScChart2DataSequence::generateLabel</c> sums the columns and rows of every range
    /// token, labels along the short side, and <c>GenerateLabelStrings</c> writes
    /// <c>ScResId(STR_COLUMN) + " " + &lt;column letter&gt;</c> or
    /// <c>ScResId(STR_ROW) + " " + &lt;1-based row&gt;</c>
    /// (<c>sc/source/ui/unoobj/chart2uno.cxx:3147-3243</c>). Equal counts return an empty
    /// sequence and therefore no name.
    /// </para>
    /// <para>
    /// Measured on <c>057_Simple_balance_sheet_Use_this_template_e2d4cbb2.xlsx</c>, whose two
    /// <c>c:ser</c> state a <c>c:spPr</c>, a <c>c:cat</c> and a <c>c:val</c> and no <c>c:tx</c>:
    /// the reference draws a two-entry legend reading <c>Column C</c> and <c>Column D</c> — its
    /// value ranges are unions of six column-C and column-D fragments, twenty rows against six
    /// columns — and we drew no legend at all.
    /// </para>
    /// <para>
    /// <strong>Gated on there being a resolver, which is the Calc host and only Calc.</strong>
    /// A deck's or a document's sequences are <c>CachedDataSequence</c>s, whose
    /// <c>generateLabel</c> returns an empty sequence, so the reference names those series by
    /// another route entirely (<c>lcl_getDataSequenceLabel</c> over the values) and never by this
    /// one. Firing it without a resolver would invent legends on every unnamed pptx series.
    /// </para>
    /// <para>
    /// The strings are LibreOffice's localized <c>STR_COLUMN</c>/<c>STR_ROW</c>, in the en-US
    /// resource this project's reference binary runs under — the same standing caveat
    /// <see cref="DrawingChartTitle.DiagramTitle"/> carries.
    /// </para>
    /// </remarks>
    private static string? GeneratedSeriesName(XElement? values, ChartRangeResolver? ranges)
    {
        if (ranges is null || values is null) return null;
        if (FormulaOf(values) is not { } formula) return null;

        int columns = 0;
        int rows = 0;
        string? firstColumn = null;
        int firstRow = -1;

        foreach ((string Column, int Row, string EndColumn, int EndRow) area in Areas(formula))
        {
            columns += ColumnNumber(area.EndColumn) - ColumnNumber(area.Column) + 1;
            rows += area.EndRow - area.Row + 1;

            firstColumn ??= area.Column;
            if (firstRow < 0) firstRow = area.Row;
        }

        if (firstColumn is null || firstRow < 0) return null;

        // LabelOrigin_SHORT_SIDE: label along whichever side is shorter, and nothing at all when
        // the two are equal.
        if (rows > columns) return "Column " + firstColumn;
        if (columns > rows) return "Row " + firstRow.ToString(CultureInfo.InvariantCulture);

        return null;
    }

    /// <summary>Every A1 area in a <c>c:f</c>, sheet qualifiers and unions included.</summary>
    /// <remarks>
    /// <para>
    /// <c>AccumulateRangeSize</c> walks the token list a formula parsed into; this reads the same
    /// areas straight out of the text, which is all the two figures it feeds need. A reference
    /// this cannot read contributes nothing, exactly as an unresolvable token does.
    /// </para>
    /// <para>
    /// The sheet qualifier is cut off before the pattern runs, because a name such as
    /// <c>Q1Data</c> reads as the cell <c>Q1</c> otherwise. A union's members are separated at the
    /// commas the file writes them with.
    /// </para>
    /// </remarks>
    private static IEnumerable<(string Column, int Row, string EndColumn, int EndRow)> Areas(
        string formula)
    {
        foreach (string member in formula.Trim('(', ')').Split(','))
        {
            int qualifier = member.LastIndexOf('!');
            string area = qualifier >= 0 ? member[(qualifier + 1)..] : member;

            foreach ((string, int, string, int) found in AreasIn(area)) yield return found;
        }
    }

    /// <summary>The areas in one member of a union, its sheet qualifier already cut off.</summary>
    private static IEnumerable<(string Column, int Row, string EndColumn, int EndRow)> AreasIn(
        string area)
    {
        foreach (Match match in AreaPattern.Matches(area))
        {
            string column = match.Groups[1].Value.ToUpperInvariant();
            string endColumn = match.Groups[3].Success
                ? match.Groups[3].Value.ToUpperInvariant()
                : column;

            if (!int.TryParse(
                    match.Groups[2].Value, NumberStyles.None, CultureInfo.InvariantCulture,
                    out int row))
            {
                continue;
            }

            int endRow = row;
            if (match.Groups[4].Success
                && !int.TryParse(
                    match.Groups[4].Value, NumberStyles.None, CultureInfo.InvariantCulture,
                    out endRow))
            {
                continue;
            }

            yield return (column, row, endColumn, endRow);
        }
    }

    /// <summary>One A1 area: <c>$B$6</c> or <c>$B$6:$C$11</c>, dollars optional.</summary>
    private static readonly Regex AreaPattern = new(
        @"\$?([A-Za-z]{1,3})\$?([0-9]{1,7})(?::\$?([A-Za-z]{1,3})\$?([0-9]{1,7}))?",
        RegexOptions.CultureInvariant);

    /// <summary>A column letter's 1-based number.</summary>
    private static int ColumnNumber(string letters)
    {
        int number = 0;

        foreach (char letter in letters)
            number = (number * 26) + (letter - 'A' + 1);

        return number;
    }

    /// <summary>
    /// A series' name: its live cells where the caller can reach them, else its cached points.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A <c>c:tx/c:strRef</c> need not carry a <c>c:strCache</c> at all</strong>, and a
    /// reader that only looks at the cache then leaves the series unnamed — which drops the
    /// <em>whole legend</em>, because a legend is built from the entries that have a name.
    /// Measured on <c>microsoft_learn_multi_chart_examples.xlsx</c>, whose five charts state
    /// <c>&lt;tx&gt;&lt;strRef&gt;&lt;f&gt;'Examples'!B4&lt;/f&gt;&lt;/strRef&gt;&lt;/tx&gt;</c>
    /// and no cache anywhere in the part: the reference draws <c>Product A/B/C</c> beside its
    /// line, column and area charts and we drew no legend on any of the three.
    /// </para>
    /// <para>
    /// The order is <see cref="ReadSequence"/>'s and for the same reason — the <c>c:f</c> wins and
    /// the cache is the fallback, because <c>ExcelChartConverter::createDataSequence</c> resolves
    /// the formula and falls back to the cached data only when there is no formula
    /// (<c>sc/source/filter/oox/excelchartconverter.cxx:76-105</c>). With no resolver, which is
    /// every host but Calc, this is exactly the cache-only reader it was.
    /// </para>
    /// <para>
    /// Several cells join with one space, which is <see cref="DrawingChartText.Label"/>'s rule and
    /// <c>datasourceconverter.cxx:50-73</c>'s.
    /// </para>
    /// </remarks>
    private static string? SeriesName(XElement? tx, ChartRangeResolver? ranges)
    {
        if (tx is null) return null;

        if (ranges is not null && FormulaOf(tx) is { } formula && ranges(formula) is { } live)
        {
            System.Text.StringBuilder joined = new();

            foreach (string? cell in live.Text)
            {
                if (cell is not { Length: > 0 }) continue;
                if (joined.Length > 0) joined.Append(' ');
                joined.Append(cell);
            }

            if (joined.Length > 0) return joined.ToString();
        }

        return DrawingChartText.Label(tx);
    }

    /// <summary>Reads a data sequence: its live cells where the caller can reach them, else its
    /// cached points.</summary>
    /// <remarks>
    /// <para>
    /// <strong>The <c>c:f</c> wins and the cache is the fallback</strong>, when a resolver is
    /// given at all — <c>ExcelChartConverter::createDataSequence</c>'s own order
    /// (<c>sc/source/filter/oox/excelchartconverter.cxx:76-94</c>). With no resolver the cache is
    /// the only source, which is the <c>ChartConverter</c> base and the right answer for a deck or
    /// a document whose numbers live in a workbook this reader must not open. See
    /// <see cref="ChartRangeResolver"/>.
    /// </para>
    /// <para>
    /// Reading the cache: the same rule <see cref="DrawingChart"/> documents at length — the array
    /// is sized from <c>c:ptCount</c> and every point placed at its own <c>@idx</c>, because the
    /// indices skip blanks and reading in document order slides every later value onto the wrong
    /// category. A chart drawn that way has the right bars against the wrong labels and looks
    /// entirely plausible.
    /// </para>
    /// </remarks>
    private static (string?[] Text, double?[] Numbers) ReadSequence(
        XElement? source, ChartRangeResolver? ranges = null)
    {
        if (source is null) return ([], []);

        // A multi-level cache before *everything*, the resolver included, and that ordering is
        // the whole of this branch.
        //
        // <strong>A c:multiLvlStrRef's c:f names a rectangle, and the resolver hands back its
        // cells as one flat run.</strong> The reference does not read them that way:
        // ExplicitCategoriesProvider builds one category per *row* and stacks the columns as the
        // levels of its label, so a two-column, eight-row range is eight categories with a
        // two-line name each — not sixteen categories. ChartRangeValues carries no shape, so the
        // rectangle cannot be recovered from it, and the c15-free c:multiLvlStrCache Excel wrote
        // is the only source in the file that states the level structure at all.
        //
        // Measured on 040_Blood_pressure_tracker_872b6833.xlsx, whose c:cat is
        // 'BLOOD PRESSURE DATA'!$C$12:$D$19 with an eight-point, two-level cache: resolving the
        // range gave sixteen categories — AM, 11/6/2022, PM, 11/6/2022, … — so the axis drew
        // sixteen slots of which eight were empty and every label was a level rather than a
        // category. The reference draws eight, each reading AM (or PM) over its date.
        if (Child(Child(source, "multiLvlStrRef"), "multiLvlStrCache") is { } levelled)
            return ReadMultiLevel(levelled);

        // An *empty* resolved sequence is a real answer — a range every cell of which is an
        // Excel table's totals row — and must not fall through to the cache. See
        // ChartRangeResolver for the two states a resolver distinguishes.
        if (ranges is not null && FormulaOf(source) is { } formula
            && ranges(formula) is { } live)
        {
            return ([.. live.Text], [.. live.Numbers]);
        }

        XElement? cache =
            Child(Child(source, "strRef"), "strCache")
            ?? Child(source, "strLit")
            ?? Child(Child(source, "numRef"), "numCache")
            ?? Child(source, "numLit");

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

    /// <summary>
    /// The levels of a complex category axis, innermost first, or null when there is one level.
    /// </summary>
    /// <remarks>
    /// One entry per category per level, in the file's own order — Excel writes <c>c:lvl</c>
    /// innermost first and <c>VCartesianAxis</c> draws level zero nearest the axis line, so no
    /// reversal happens here. A point the cache does not state stays null, which
    /// <see cref="ChartPlot.CategoryLevels"/> reads as a continuation of the run above it.
    /// <para>
    /// Taken from the first series that states one. A chart's series share a category axis, and
    /// the ones that state a <c>c:cat</c> at all state the same rectangle.
    /// </para>
    /// </remarks>
    private static List<IReadOnlyList<string?>>? CategoryLevelsOf(XElement plotArea)
    {
        foreach (XElement group in plotArea.Elements())
        {
            if (group.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;

            foreach (XElement series in group.Elements(Name("ser")))
            {
                if (Child(Child(Child(series, "cat"), "multiLvlStrRef"), "multiLvlStrCache")
                    is not { } cache)
                {
                    continue;
                }

                int declared = Drawing.Number(Child(cache, "ptCount"), "val") ?? -1;
                if (declared < 0)
                {
                    foreach (XElement point in cache.Descendants(Name("pt")))
                        declared = Math.Max(declared, (Drawing.Number(point, "idx") ?? -1) + 1);
                }

                int count = Math.Clamp(declared, 0, MaxPointCount);
                if (count == 0) continue;

                List<IReadOnlyList<string?>> levels = [];
                foreach (XElement level in cache.Elements(Name("lvl")))
                {
                    string?[] texts = new string?[count];
                    foreach (XElement point in level.Elements(Name("pt")))
                    {
                        int index = Drawing.Number(point, "idx") ?? -1;
                        if (index < 0 || index >= count) continue;

                        string value = Child(point, "v")?.Value ?? string.Empty;
                        if (value.Length > 0) texts[index] = value;
                    }

                    levels.Add(texts);
                }

                return levels.Count > 1 ? levels : null;
            }
        }

        return null;
    }

    /// <summary>
    /// A <c>c:multiLvlStrCache</c> flattened into one label per point, outermost level first.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This used to be read by the flat walk above and that was a silent defect.</strong>
    /// Each <c>c:lvl</c> numbers its own points from zero, so walking every <c>c:pt</c> descendant
    /// and assigning by <c>@idx</c> makes each level overwrite the one before it. Excel writes the
    /// levels innermost first, so the survivor was the <em>outermost</em> level and every label
    /// and legend entry of a three-level category came out as <c>Branch 1</c> where the reference
    /// draws <c>Branch 1 Stem 2 Leaf 5</c>. Two blind reviewers, each given only a rendered page
    /// and forbidden to read anything else, transcribed both halves of
    /// <c>027_Unit_Circle_Chart_Graphical_Chart</c> and
    /// <c>028_Unit_Circle_Chart_Optimized_Graph</c> and reported exactly that.
    /// </para>
    /// <para>
    /// Joined with a space from the outermost level inwards, skipping the levels that state
    /// nothing at an index — <c>lcl_getExplicitSimpleCategories</c>,
    /// <c>chart2/source/tools/ExplicitCategoriesProvider.cxx:376-395</c>, which builds exactly
    /// this string and is what <c>getSimpleCategories</c> hands to the legend and to every data
    /// label. LibreOffice keeps the levels apart as well, as the rows of a complex category axis;
    /// one label per point cannot hold that, and the join is what the reference draws wherever a
    /// single string is drawn. <see cref="DrawingChart"/>'s extraction reader has always joined
    /// them this way, so this also stops the two readers of one element disagreeing.
    /// </para>
    /// <para>
    /// No numbers: a category level is a string, and parsing <c>Leaf 12</c> as a double gives
    /// nothing anyway.
    /// </para>
    /// </remarks>
    private static (string?[] Text, double?[] Numbers) ReadMultiLevel(XElement cache)
    {
        int declared = Drawing.Number(Child(cache, "ptCount"), "val") ?? -1;
        if (declared < 0)
        {
            foreach (XElement point in cache.Descendants(Name("pt")))
                declared = Math.Max(declared, (Drawing.Number(point, "idx") ?? -1) + 1);
        }

        int count = Math.Clamp(declared, 0, MaxPointCount);
        List<XElement> levels = [.. cache.Elements(Name("lvl"))];
        StringBuilder[] labels = new StringBuilder[count];
        for (int at = 0; at < count; at++) labels[at] = new StringBuilder();

        for (int level = levels.Count - 1; level >= 0; level--)
        {
            foreach (XElement point in levels[level].Elements(Name("pt")))
            {
                int index = Drawing.Number(point, "idx") ?? -1;
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

    /// <summary>The <c>c:f</c> a reference states, or null when the sequence is a literal.</summary>
    /// <remarks>
    /// Only the three <c>…Ref</c> containers carry one; <c>c:strLit</c> and <c>c:numLit</c> hold
    /// the numbers themselves and name nothing, which is exactly the case
    /// <c>maFormula.isEmpty()</c> tests for.
    /// </remarks>
    private static string? FormulaOf(XElement source)
    {
        foreach (string container in (string[])["numRef", "strRef", "multiLvlStrRef"])
        {
            if (Child(Child(source, container), "f")?.Value is { Length: > 0 } formula)
                return formula;
        }

        return null;
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

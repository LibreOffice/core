using Paperless.Core.Numbers;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Core.Charts;

/// <summary>How a chart label sits against the point it is placed at.</summary>
public enum ChartLabelAnchor
{
    /// <summary>Centred horizontally, its top at the point — a category label below the axis.</summary>
    CentreTop = 0,

    /// <summary>
    /// Centred horizontally, its <em>bottom</em> at the point — a data label above a bar.
    /// </summary>
    /// <remarks>
    /// The counterpart of <see cref="CentreTop"/> and the anchor every "outside" data label on a
    /// column chart takes. Reusing <see cref="Centre"/> for it instead puts half the text inside
    /// the bar, which reads as a label that has not been offset rather than as a missing anchor.
    /// </remarks>
    CentreBottom,

    /// <summary>Right-aligned, centred vertically on the point — a value label left of the axis.</summary>
    RightMiddle,

    /// <summary>Left-aligned, centred vertically — a legend entry.</summary>
    LeftMiddle,

    /// <summary>Centred both ways — a title.</summary>
    Centre,
}

/// <summary>One piece of text a chart draws, with where and how it goes.</summary>
/// <param name="Text">The characters.</param>
/// <param name="At">The point the anchor refers to.</param>
/// <param name="Anchor">How the text sits against that point.</param>
/// <param name="Size">The em size.</param>
/// <param name="Colour">Its colour.</param>
/// <param name="Rotation">
/// Its rotation in radians, anticlockwise. Only a value-axis title is ever rotated, by a quarter
/// turn — which both formats state, OOXML as <c>a:bodyPr rot="-5400000"</c> and ODF as
/// <c>style:rotation-angle="90"</c>.
/// </param>
/// <param name="Stretch">
/// An extra horizontal scale applied to the glyphs, 1 for text drawn at its natural width.
/// </param>
/// <remarks>
/// <strong><paramref name="Stretch"/> exists because a glyph run carries one em and a
/// non-square stretch has two.</strong> An embedded chart is composed at its own size and scaled
/// into its frame by <c>(sx, sy)</c>; the positions take both, and <paramref name="Size"/> can
/// only take one, so the type came out <c>sx/sy</c> too wide or too narrow — 12% on
/// <c>chart-bar-sheet.ods</c>, whose 12 × 7 cm chart sits in a frame 0.625 as wide and 0.709 as
/// tall. Carrying the residual here and letting each consumer put it into its own transform is
/// what closes that, and it costs a chart that is not stretched nothing at all: the factor is
/// exactly 1 and both consumers take their unstretched path.
/// </remarks>
public readonly record struct ChartLabel(
    string Text,
    DocPoint At,
    ChartLabelAnchor Anchor,
    Length Size,
    Colour Colour,
    double Rotation = 0.0,
    double Stretch = 1.0);

/// <summary>One filled rectangle — a bar, a legend key, the plot area's wall.</summary>
/// <param name="Bounds">Where it goes.</param>
/// <param name="Fill">Its fill, or null when it is outline only.</param>
/// <param name="Line">Its outline colour, or null when it has none.</param>
/// <param name="LineWidth">The outline's width.</param>
public readonly record struct ChartBox(
    DocRect Bounds,
    Colour? Fill,
    Colour? Line = null,
    Length LineWidth = default);

/// <summary>One straight line — an axis, a tick, a gridline.</summary>
/// <param name="From">Its start.</param>
/// <param name="To">Its end.</param>
/// <param name="Colour">Its colour.</param>
/// <param name="Width">Its width; zero is a hairline.</param>
public readonly record struct ChartLine(
    DocPoint From, DocPoint To, Colour Colour, Length Width = default);

/// <summary>
/// One free-form mark — a line chart's polyline, an area's filled region, a pie's wedge.
/// </summary>
/// <remarks>
/// A path rather than a rectangle because those three cannot be expressed as one, and a path
/// rather than three shapes because a renderer treats them identically: fill it, stroke it, or
/// both. <see cref="GraphicsPath"/> lives in <c>Paperless.Core.Graphics</c>, so producing one here
/// costs the layout nothing it did not already depend on.
/// </remarks>
/// <param name="Path">The outline, already closed where it should be.</param>
/// <param name="Fill">Its fill, or null when it is a stroke only — which is a line chart.</param>
/// <param name="Line">Its outline colour, or null for none.</param>
/// <param name="LineWidth">The outline's width; zero is a hairline.</param>
public readonly record struct ChartShape(
    GraphicsPath Path,
    Colour? Fill,
    Colour? Line = null,
    Length LineWidth = default);

/// <summary>
/// A chart laid out: every mark it draws, in paint order, in the frame's coordinates.
/// </summary>
/// <param name="PlotArea">The inner plot rectangle — the axes' extent, labels excluded.</param>
/// <param name="Boxes">The filled and outlined rectangles, back to front.</param>
/// <param name="Lines">The axes, ticks and gridlines.</param>
/// <param name="Labels">The text.</param>
/// <param name="Shapes">
/// The paths — wedges, polylines and areas — drawn after <paramref name="Boxes"/> and before
/// <paramref name="Labels"/>, which is where the reference draws them.
/// </param>
/// <param name="DiagramArea">
/// The <em>outer</em> rectangle the diagram was laid out in — what is left of the frame once the
/// page margin, the main title and the legend have been taken off, and before the axes' own
/// labels are reserved out of it.
/// </param>
/// <remarks>
/// <strong><paramref name="DiagramArea"/> is here because it is the seam the plot rectangle
/// splits along, and because ODF states it.</strong> LibreOffice's composition is two
/// independent halves — <c>getAvailablePosAndSizeForDiagram</c> plus <c>lcl_createTitle</c> and
/// <c>lcl_createLegend</c> produce <c>maRemainingSpace</c>, and only then does
/// <c>VDiagram::adjustInnerSize</c> shrink that by whatever the axis labels turned out to
/// occupy. An ODF chart writes <em>both</em> rectangles: <c>chart:plot-area</c>'s own
/// <c>svg:x</c>…<c>svg:height</c> is the first and <c>chart:coordinate-region</c> is the second,
/// so subtracting them gives the four label reservations LibreOffice itself arrived at. Carrying
/// the outer rectangle out of the layout is what lets a measurement say which of the two halves
/// a discrepancy is in, rather than only that the plot rectangle is a point out.
/// </remarks>
public sealed record ChartDrawing(
    DocRect PlotArea,
    IReadOnlyList<ChartBox> Boxes,
    IReadOnlyList<ChartLine> Lines,
    IReadOnlyList<ChartLabel> Labels,
    IReadOnlyList<ChartShape> Shapes,
    DocRect DiagramArea = default);

/// <summary>
/// Measures a single line of chart text, so that layout can reserve room for it.
/// </summary>
/// <remarks>
/// An interface rather than a font cache because <see cref="ChartLayout"/> sits below every
/// library that owns one, and because the two consumers — a slide and a sheet — resolve faces
/// through caches of their own that may not agree on a substitution.
/// </remarks>
public interface IChartTextMeasurer
{
    /// <summary>The advance width and line height of a single line of text.</summary>
    /// <param name="text">The characters.</param>
    /// <param name="size">The em size.</param>
    DocSize Measure(string text, Length size);
}

/// <summary>
/// Composes a chart: where the plot area goes, and every mark inside and around it.
/// </summary>
/// <remarks>
/// <para>
/// A port of the composition in <c>chart2/source/view/main/ChartView.cxx</c> and the bar
/// geometry in <c>chart2/source/view/charttypes/BarChart.cxx</c>, at the level of "what
/// rectangle does each thing occupy". What it deliberately does <em>not</em> port is
/// LibreOffice's two-pass refinement, in which the axis labels are laid out, measured, and the
/// whole diagram re-laid-out around them until it settles
/// (<c>ChartView::impl_createDiagramAndContent</c>). One pass with measured labels gets the plot
/// rectangle close; the second pass moves it by a point or two.
/// </para>
/// <para>
/// <strong>Which is why the file is consulted first.</strong> ODF states the answer outright, in
/// <c>chart:coordinate-region</c>, so an ODF chart skips the composition entirely and is exact
/// by construction. OOXML states nothing unless the author moved the plot area by hand, so a
/// PPTX or XLSX chart goes through the computation. That asymmetry is the whole reason the two
/// paths exist, and it is the second time on this feature that ODF turned out to carry a baked
/// answer OOXML does not.
/// </para>
/// <para>
/// <strong>The arithmetic inside the plot area is exact and is the part that matters.</strong>
/// Measured against LibreOffice's PDF for <c>chart-bar-deck.pptx</c>, whose plot area is
/// 500.967 pt wide over four categories and two clustered series with
/// <c>c:gapWidth val="100"</c>: a category slot is 125.242 pt, a bar is
/// <c>slot / (series + gap/100)</c> = 41.747 pt against a reference 41.754, and the first bar's
/// left edge is <c>slotLeft + gap/100 × barWidth / 2</c> = 20.874 pt into the slot against a
/// reference 20.863. Both agree to within the hundredth of a millimetre LibreOffice rounds its
/// own coordinates to.
/// </para>
/// </remarks>
public static partial class ChartLayout
{
    /// <summary>The length of a major tick mark, outside the axis.</summary>
    /// <remarks>
    /// <c>AXIS2D_TICKLENGTH = 150</c> hundredths of a millimetre
    /// (<c>chart2/source/view/inc/ViewDefines.hxx:30</c>), commented "value like in old chart"
    /// — so it is a fixed length rather than one derived from the font, and it does not scale
    /// with the chart. Confirmed in the reference PDF: every tick runs exactly 4.252 pt outside
    /// the axis, which is 150 hundredths of a millimetre to three decimal places.
    /// </remarks>
    private static readonly Length TickLength = Length.FromMm100(150);

    /// <summary>The gap between a tick and the label beside it.</summary>
    /// <remarks><c>AXIS2D_TICKLABELSPACING = 100</c> (<c>ViewDefines.hxx:31</c>).</remarks>
    private static readonly Length LabelSpacing = Length.FromMm100(100);

    /// <summary>The extra gap below a main title, beyond the proportional one.</summary>
    /// <remarks>
    /// <c>lcl_createTitle</c> adds a flat 135 hundredths of a millimetre for a main title, on
    /// top of 2% of the chart height (<c>ChartView.cxx:1066-1069</c>).
    /// </remarks>
    private static readonly Length TitleGap = Length.FromMm100(135);

    /// <summary>The gap between the category axis' labels and its title.</summary>
    /// <remarks><c>ChartView.cxx:1070-1073</c>, a flat 420 rather than a proportion.</remarks>
    private static readonly Length CategoryTitleGap = Length.FromMm100(420);

    /// <summary>The gap between the value axis' labels and its title.</summary>
    /// <remarks><c>ChartView.cxx:1074-1077</c>, a flat 450.</remarks>
    private static readonly Length ValueTitleGap = Length.FromMm100(450);

    /// <summary>The margin round the whole chart, as a fraction of its own size.</summary>
    /// <remarks>
    /// <c>constPageLayoutDistancePercentage = 0.02</c> (<c>ChartView.cxx:918</c>), applied to the
    /// chart's width horizontally and its height vertically — so it is not a square margin on a
    /// chart that is not square.
    /// </remarks>
    private const double PageMargin = 0.02;

    /// <summary>The margin round a pie chart, which is a flat length rather than a fraction.</summary>
    /// <remarks>
    /// <c>constPageLayoutFixedDistance = 350</c> hundredths of a millimetre, applied instead of
    /// <see cref="PageMargin"/> and only when the first chart type is
    /// <c>CHART2_SERVICE_NAME_CHARTTYPE_PIE</c> — "Only pie chart uses fixed size margins"
    /// (<c>ChartView.cxx:919, 935-940</c>). A doughnut is a chart type of its own and keeps the
    /// proportional margin, which is why <see cref="ChartPlot.Rings"/> gates this.
    /// </remarks>
    private static readonly Length PieMargin = Length.FromMm100(350);

    /// <summary>The gap between the legend and the diagram, left or right.</summary>
    /// <remarks>
    /// <para>
    /// <c>lcl_getLegendLeftRightMargin</c> returns a flat <c>210</c> hundredths of a millimetre
    /// and <c>lcl_getLegendTopBottomMargin</c> a flat <c>185</c>
    /// (<c>chart2/source/view/main/VLegend.cxx:662-671</c>, both under "#i109336# Improve auto
    /// positioning in chart"). <c>lcl_calculatePositionAndRemainingSpace</c> takes the legend's
    /// own width <em>plus that margin</em> off the remaining space (<c>:752-784</c>).
    /// </para>
    /// <para>
    /// <strong>It is not the page margin, and using the page margin costs 6.5 pt on a
    /// 623 pt frame.</strong> Measured on <c>chart-bar-deck.pptx</c>, whose chart is composed at
    /// 623.622 × 340.157: two per cent of that width is 12.472 against the legend's own 5.953, so
    /// the plot rectangle's right edge came out 6.519 pt short of the reference's — which was most
    /// of the 1.79 pt that edge was out by even after the legend's own width was corrected.
    /// </para>
    /// </remarks>
    private static readonly Length LegendMarginX = Length.FromMm100(210);

    /// <summary>The gap between the legend and the diagram, above or below.</summary>
    /// <remarks><c>VLegend.cxx:668-671</c>; see <see cref="LegendMarginX"/>.</remarks>
    private static readonly Length LegendMarginY = Length.FromMm100(185);

    /// <summary>The colour LibreOffice draws an axis, its ticks and its labels in.</summary>
    private static readonly Colour AxisColour = Colour.Black;

    /// <summary>
    /// A chart text shape's horizontal inset, as a fraction of the font height.
    /// </summary>
    /// <remarks>
    /// <c>ShapeFactory::createText</c> sets <c>TextLeftDistance</c> and <c>TextRightDistance</c>
    /// to <c>round(fontHeight × 0.18)</c> and the vertical pair to <c>× 0.30</c>, both in
    /// hundredths of a millimetre, under the comment
    /// "#i109336# Improve auto positioning in chart"
    /// (<c>chart2/source/view/main/ShapeFactory.cxx:2279-2299</c>). Every piece of text a chart
    /// draws goes through that function, so what the layout reserves is the <em>shape's</em>
    /// size and not the text's — 36% of the font height wider and 60% taller. On the corpus
    /// chart's 13 pt title that is 2.75 pt of extra height, all of it above the plot area, and
    /// it was the single largest term missing from the first version of this layout.
    /// </remarks>
    private const double TextShapeInsetX = 0.18;

    /// <summary>A chart text shape's vertical inset, as a fraction of the font height.</summary>
    /// <remarks><c>ShapeFactory.cxx:2285</c>; see <see cref="TextShapeInsetX"/>.</remarks>
    private const double TextShapeInsetY = 0.30;

    /// <summary>
    /// The size of the shape a piece of chart text is drawn in, insets included.
    /// </summary>
    /// <remarks>
    /// What the composition reserves room for. Measuring the text alone under-reserves on every
    /// side at once, which moves the plot area up and left and leaves the labels crowding the
    /// frame's edges.
    /// </remarks>
    private static DocSize Shape(IChartTextMeasurer measurer, string text, Length size)
    {
        DocSize measured = MeasureLines(measurer, text, size);
        return new DocSize(
            measured.Width + size * (TextShapeInsetX * 2),
            measured.Height + size * (TextShapeInsetY * 2));
    }

    /// <summary>
    /// The lines a title breaks into, which is more than one when the file says so.
    /// </summary>
    /// <remarks>
    /// A chart's title is one string with its own line breaks in it — Excel's BIFF <c>CHSTRING</c>
    /// writes <c>"Chart 8\n2012 Average Fuel Consumption Rates"</c>, and DrawingML writes a second
    /// <c>a:p</c> — and the break is part of the title rather than wrapping the engine chose. A
    /// measurer handed the whole string reports one line, so a two-line title reserves half the
    /// room it needs and the plot area starts a line too high.
    /// </remarks>
    private static string[] LinesOf(string text)
        => text.AsSpan().IndexOfAny('\n', '\r') < 0
            ? [text]
            : text.Split(['\n', '\r'], StringSplitOptions.RemoveEmptyEntries);

    /// <summary>How much room a possibly multi-line label needs: the widest line, all the heights.</summary>
    private static DocSize MeasureLines(IChartTextMeasurer measurer, string text, Length size)
    {
        string[] lines = LinesOf(text);
        if (lines.Length <= 1) return measurer.Measure(text, size);

        Length width = Length.Zero;
        Length height = Length.Zero;
        foreach (string line in lines)
        {
            DocSize measured = measurer.Measure(line, size);
            width = Length.Max(width, measured.Width);
            height += measured.Height;
        }

        return new DocSize(width, height);
    }

    /// <summary>Lays a chart out inside a frame.</summary>
    /// <param name="plot">The chart.</param>
    /// <param name="frame">The graphic frame's rectangle, in the caller's coordinates.</param>
    /// <param name="measurer">Measures a line of text, for reserving room.</param>
    public static ChartDrawing Place(ChartPlot plot, DocRect frame, IChartTextMeasurer measurer)
    {
        ArgumentNullException.ThrowIfNull(plot);
        ArgumentNullException.ThrowIfNull(measurer);

        if (frame.Width <= Length.Zero || frame.Height <= Length.Zero)
            return new ChartDrawing(DocRect.Empty, [], [], [], []);

        // A chart with a coordinate space of its own is composed at its own size and the whole
        // picture is then stretched into the frame. See Stretch.
        if (plot.Space is not { } space
            || space.Width <= Length.Zero
            || space.Height <= Length.Zero
            || (space.Width == frame.Width && space.Height == frame.Height))
        {
            return Compose(plot, frame, measurer);
        }

        DocRect own = new(Length.Zero, Length.Zero, space.Width, space.Height);
        return Stretch(Compose(plot, own, measurer), own, frame);
    }

    /// <summary>
    /// Stretches a chart composed at its own size onto the frame that displays it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An embedded chart is rendered at its own size and scaled, not re-laid-out.</strong>
    /// Measured on <c>chart-bar-sheet.ods</c>, whose chart document states <c>svg:width="12cm"</c>
    /// by <c>svg:height="7cm"</c> and whose frame on the sheet is 2.952 in by 1.9547 in — a scale
    /// of 0.625 across and 0.709 down. In LibreOffice's own PDF the chart's 13 pt title measures
    /// 62.1 pt wide against the 99.4 pt the same title measures in the same chart's
    /// <c>.xlsx</c> form, which is 0.625 exactly; its height ratio is 0.708. So the type is
    /// stretched with everything else, by two different factors, and the chart is <em>not</em>
    /// re-composed for the smaller frame.
    /// </para>
    /// <para>
    /// <strong>Which is what decides the tick count, so it is not cosmetic.</strong> Composing
    /// <c>chart-bar-sheet.ods</c> in its frame gives an axis 77 pt long, which has room for six
    /// intervals and lands on <c>0 50 … 200</c>; composing it at its own 12 × 7 cm gives one
    /// 108.8 pt long, room for nine, and <c>0 20 … 180</c> — which is what the reference draws.
    /// </para>
    /// <para>
    /// <strong>The one thing that cannot follow the em, and where it goes instead.</strong> A
    /// glyph run carries one em size, so the type is scaled by the vertical factor alone and the
    /// residual <c>sx/sy</c> — 12% on this chart — is carried on
    /// <see cref="ChartLabel.Stretch"/> for each consumer to fold into its own transform. Dropping
    /// it, which is what this did at first, draws every word of a stretched chart
    /// <c>sx/sy</c> too wide against a reference that is exact.
    /// </para>
    /// </remarks>
    private static ChartDrawing Stretch(ChartDrawing drawing, DocRect from, DocRect frame)
    {
        double sx = (double)frame.Width.Emu / from.Width.Emu;
        double sy = (double)frame.Height.Emu / from.Height.Emu;

        DocPoint At(DocPoint point)
            => new(frame.X + point.X * sx, frame.Y + point.Y * sy);

        DocRect Box(DocRect rectangle)
            => new(
                frame.X + rectangle.X * sx,
                frame.Y + rectangle.Y * sy,
                rectangle.Width * sx,
                rectangle.Height * sy);

        List<ChartBox> boxes = new(drawing.Boxes.Count);
        foreach (ChartBox box in drawing.Boxes)
            boxes.Add(box with { Bounds = Box(box.Bounds), LineWidth = box.LineWidth * sy });

        List<ChartLine> lines = new(drawing.Lines.Count);
        foreach (ChartLine line in drawing.Lines)
            lines.Add(line with { From = At(line.From), To = At(line.To), Width = line.Width * sy });

        // The em follows the vertical factor because that is what a line height is; the residual
        // sx/sy goes onto the label as a horizontal scale for the consumer to apply.
        double residual = sy == 0.0 ? 1.0 : sx / sy;

        List<ChartLabel> labels = new(drawing.Labels.Count);
        foreach (ChartLabel label in drawing.Labels)
        {
            labels.Add(label with
            {
                At = At(label.At),
                Size = label.Size * sy,
                Stretch = label.Stretch * residual,
            });
        }

        List<ChartShape> shapes = new(drawing.Shapes.Count);
        foreach (ChartShape shape in drawing.Shapes)
            shapes.Add(shape with { Path = Stretched(shape.Path), LineWidth = shape.LineWidth * sy });

        return new ChartDrawing(
            Box(drawing.PlotArea), boxes, lines, labels, shapes, Box(drawing.DiagramArea));

        GraphicsPath Stretched(GraphicsPath path)
        {
            GraphicsPath moved = new();

            foreach (PathCommand command in path.Commands)
            {
                switch (command.Verb)
                {
                    case PathVerb.MoveTo: moved.MoveTo(At(command.Point)); break;
                    case PathVerb.LineTo: moved.LineTo(At(command.Point)); break;
                    case PathVerb.CubicTo:
                        moved.CubicTo(At(command.Control1), At(command.Control2), At(command.Point));
                        break;
                    default: moved.Close(); break;
                }
            }

            return moved;
        }
    }

    /// <summary>Back to front, which is the order a combination chart's groups are painted in.</summary>
    private static readonly ChartPlotKind[] DrawingOrder =
    [
        ChartPlotKind.Area,
        ChartPlotKind.Bar,
        ChartPlotKind.Stock,
        ChartPlotKind.Line,
        ChartPlotKind.Scatter,
        ChartPlotKind.Radar,
        ChartPlotKind.Bubble,
        ChartPlotKind.Pie,
        ChartPlotKind.OfPie,
    ];

    /// <summary>Composes a chart in the coordinates it is measured in.</summary>
    private static ChartDrawing Compose(ChartPlot plot, DocRect frame, IChartTextMeasurer measurer)
    {
        List<ChartBox> boxes = [];
        List<ChartLine> lines = [];
        List<ChartLabel> labels = [];
        List<ChartShape> shapes = [];

        if (plot.Background is { } background)
            boxes.Add(new ChartBox(frame, background));

        int categories = plot.CategoryCount();
        (double? dataMinimum, double? dataMaximum) = plot.ValueRange(0);

        // A radar chart's radius axis is capped at two intervals whatever it is drawn at:
        // VPolarRadiusAxis::estimateMaximumAutoMainIncrementCount returns a flat 2
        // (chart2/source/view/axes/VPolarRadiusAxis.cxx:87-90) where the cartesian one derives a
        // count from the axis' length. That is the whole of why a radar chart's web has three
        // rings and not eleven, and it is measured: radar-chart-labels.docx peaks at 40 and
        // LibreOffice draws rings at 0, 20 and 40.
        ChartScaleResult scale = ChartScale.Resolve(
            plot.ValueScale,
            dataMinimum,
            dataMaximum,
            maximumIntervals: plot.Kind is ChartPlotKind.Radar
                ? RadarIntervalCount
                : ChartScale.MaximumAutoIntervalCount);

        // A scatter chart's X is a numeric dimension with a scale of its own rather than a run of
        // category slots, so it is resolved here and threaded through everything that maps a point
        // across the plot area. Null for every other type, which is what keeps the category path
        // unchanged.
        ChartScaleResult? domain = DomainScaleOf(plot);

        (double? secondMinimum, double? secondMaximum) = plot.ValueRange(1);
        ChartScaleResult? secondary = plot.HasSecondaryAxis
            ? ChartScale.Resolve(plot.SecondaryValueScale!.Value, secondMinimum, secondMaximum)
            : null;

        DocRect area = PlotAreaOf(
            plot, frame, scale, secondary, domain, categories, measurer, null);

        if (area.Width <= Length.Zero || area.Height <= Length.Zero)
            return new ChartDrawing(DocRect.Empty, boxes, lines, labels, shapes);

        bool columns = plot.Direction == ChartBarDirection.Column;

        // Whether the category labels fit, and what to do about it if they do not. This is the
        // other half of LibreOffice's outer loop: the arrangement is decided from the rectangle
        // the labels were reserved room in, and a rotated or staggered arrangement is deeper than
        // an upright one, so the rectangle has to be composed again around it. One refinement is
        // enough — the second arrangement is over an axis that is shorter by the labels' own
        // depth, and a third pass has never changed it on the corpus.
        ChartAxisLabelLayout? arranged = null;

        if (plot.HasAxes && columns && plot.CategoryAxisVisible
            && domain is null && plot.DataTable is null)
        {
            arranged = ArrangeCategories(plot, area, categories, measurer);

            if (arranged is { } first && !IsPlain(first))
            {
                area = PlotAreaOf(
                    plot, frame, scale, secondary, domain, categories, measurer, arranged);

                if (area.Width <= Length.Zero || area.Height <= Length.Zero)
                    return new ChartDrawing(DocRect.Empty, boxes, lines, labels, shapes);

                arranged = ArrangeCategories(plot, area, categories, measurer);
                area = PlotAreaOf(
                    plot, frame, scale, secondary, domain, categories, measurer, arranged);

                if (area.Width <= Length.Zero || area.Height <= Length.Zero)
                    return new ChartDrawing(DocRect.Empty, boxes, lines, labels, shapes);
            }
        }

        // LibreOffice's second pass, narrowed to the one thing it changes here. How many ticks an
        // axis may have is decided from how long that axis turned out and how tall — or wide — a
        // label turned out, and neither is known until the labels have been laid out once. So the
        // first pass runs at ten intervals, the count is re-derived from the rectangle it produced,
        // and the scale and the rectangle are computed again if it came out lower.
        if (plot.HasAxes)
        {
            int fitting = IntervalsThatFit(plot, area, columns, scale, measurer);
            if (fitting < ChartScale.MaximumAutoIntervalCount)
            {
                scale = ChartScale.Resolve(
                    plot.ValueScale, dataMinimum, dataMaximum, maximumIntervals: fitting);

                if (secondary is not null)
                {
                    secondary = ChartScale.Resolve(
                        plot.SecondaryValueScale!.Value, secondMinimum, secondMaximum,
                        maximumIntervals: fitting);
                }

                area = PlotAreaOf(
                    plot, frame, scale, secondary, domain, categories, measurer, arranged);

                if (area.Width <= Length.Zero || area.Height <= Length.Zero)
                    return new ChartDrawing(DocRect.Empty, boxes, lines, labels, shapes);
            }
        }

        if (plot.PlotBackground is { } wall) boxes.Add(new ChartBox(area, wall));

        if (plot.HasAxes)
        {
            AddValueAxis(plot, area, scale, columns, plot.ValueFormat, false, lines, labels);

            if (secondary is { } second && plot.SecondaryAxisVisible)
            {
                AddValueAxis(
                    plot, area, second, columns, plot.SecondaryValueFormat, true, lines, labels);
            }

            if (domain is { } across) AddDomainAxis(plot, area, across, columns, lines, labels);
            else AddCategoryAxis(
                    plot, area, categories, columns, arranged, measurer, lines, labels);

            AddDataTable(plot, frame, area, categories, columns, measurer, lines, labels);
        }
        else if (plot.Kind is ChartPlotKind.Radar)
        {
            AddRadarAxis(plot, area, scale, categories, lines, labels);
        }

        // Every plot group is drawn, not only the first, and the order is back to front: areas
        // fill, bars sit on them, lines go over both. A part holding a c:barChart and a
        // c:lineChart over one pair of axes is an ordinary combination chart, and drawing only
        // the first group loses whole series — measured on
        // stacked-non-stacked-mix-y-axis.pptx, whose third chart holds one area series and two
        // bar series and came out with one of the three.
        //
        // Split again by axis index, because a series on the secondary axis is measured against a
        // different scale and drawing it against the primary one puts it at a plausible but wrong
        // height — the same failure the automatic scale itself has.
        foreach (ChartPlotKind kind in DrawingOrder)
        {
            for (int axis = 0; axis <= 1; axis++)
            {
                if (axis == 1 && secondary is null) continue;

                List<ChartSeries> subset = plot.SeriesOf(kind, axis);
                if (subset.Count == 0) continue;

                ChartPlot part = plot with { Series = subset };
                ChartScaleResult against = axis == 1 ? secondary!.Value : scale;

                switch (kind)
                {
                    case ChartPlotKind.Pie:
                        AddWedges(part, area, shapes, labels);
                        break;
                    case ChartPlotKind.Area:
                        AddAreas(part, area, against, categories, columns, shapes, labels);
                        break;
                    case ChartPlotKind.Line:
                    case ChartPlotKind.Scatter:
                        AddLines(part, area, against, domain, categories, columns, shapes, labels);
                        break;
                    case ChartPlotKind.Radar:
                        AddRadar(part, area, against, categories, shapes, labels);
                        break;
                    case ChartPlotKind.Bubble:
                        AddBubbles(part, area, against, domain, shapes, labels);
                        break;
                    case ChartPlotKind.Stock:
                        AddCandles(part, area, against, categories, boxes, lines, labels);
                        break;
                    case ChartPlotKind.OfPie:
                        AddOfPie(part, area, shapes, lines, labels);
                        break;
                    default:
                        AddBars(part, area, against, categories, columns, boxes, labels);
                        break;
                }
            }
        }

        // The trendlines go over every series and under the titles, which is the order
        // ChartView adds the regression group in — after the series shapes and into the same
        // diagram group.
        for (int axis = 0; axis <= 1; axis++)
        {
            if (axis == 1 && secondary is null) continue;

            AddTrendlines(
                plot,
                frame,
                area,
                axis == 1 ? secondary!.Value : scale,
                domain,
                categories,
                columns,
                axis,
                shapes,
                labels);
        }

        AddTitles(plot, frame, area, measurer, labels);
        AddLegend(plot, frame, area, measurer, boxes, labels);

        return new ChartDrawing(
            area, boxes, lines, labels, shapes, DiagramAreaOf(plot, frame, measurer));
    }

    /// <summary>How many points a fitted curve is sampled at.</summary>
    /// <remarks>
    /// <c>nPointCount = 100 * fPointScale</c> in
    /// <c>VSeriesPlotter::createRegressionCurvesShapes</c> (<c>VSeriesPlotter.cxx:1363</c>), where
    /// the scale is the extrapolated range over the axis' range and is clamped to 1000. A hundred
    /// samples over a curve a few hundred points wide puts a vertex every two or three points,
    /// which is finer than the rounding the reference's own coordinates carry.
    /// </remarks>
    private const int CurveSamples = 100;

    /// <summary>
    /// The trendlines of every series measured against one value axis.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A category chart's X values are 1, 2, 3 … and not 0, 1, 2.</strong>
    /// <c>VDataSeries::getAllX</c> synthesises them when the series states none —
    /// "first category (index 0) matches with real number 1.0"
    /// (<c>chart2/source/view/main/VDataSeries.cxx:760-772</c>) — and the fit is over those, so a
    /// linear trendline over four categories has its intercept at x = 0, one whole category to the
    /// left of the first bar. Fitting over indices instead moves the intercept by the slope and
    /// writes a different equation for the same picture.
    /// </para>
    /// <para>
    /// <strong>The curve is clipped to the plot area rather than allowed outside it.</strong>
    /// <c>Clipping::clipPolygonAtRectangle</c> against the scaled logic rectangle
    /// (<c>VSeriesPlotter.cxx:1421</c>) — which is what makes <c>c:forward</c> safe: a trendline
    /// extrapolated ten categories past the data stops at the wall. Clipping each segment against
    /// the fraction interval [0, 1] in both dimensions is the same operation once the point has
    /// been mapped, and it costs no geometry of its own.
    /// </para>
    /// </remarks>
    private static void AddTrendlines(
        ChartPlot plot,
        DocRect frame,
        DocRect area,
        ChartScaleResult scale,
        ChartScaleResult? domain,
        int categories,
        bool columns,
        int axis,
        List<ChartShape> shapes,
        List<ChartLabel> labels)
    {
        foreach (ChartSeries series in plot.Series)
        {
            if (series.AxisIndex != axis) continue;
            if (series.Trendlines is not { Count: > 0 } trendlines) continue;

            IReadOnlyList<double?> xs = XValuesOf(series, categories);
            if (xs.Count == 0) continue;

            foreach (ChartTrendline trendline in trendlines)
            {
                ChartRegression fit = ChartRegression.Fit(trendline, xs, series.Values);
                if (!fit.IsUsable) continue;

                (double? minimum, double? maximum) = Extent(xs);
                if (minimum is not { } from || maximum is not { } to) continue;

                bool mean = trendline.Kind == ChartTrendlineKind.Mean;
                if (!mean)
                {
                    from -= trendline.Backward;
                    to += trendline.Forward;
                }

                double span = domain is { } across ? across.Span : Math.Max(1.0, categories);
                double stretch = span == 0.0 ? 1.0 : (to - from) / span;
                int samples = (int)(CurveSamples * Math.Min(Math.Abs(stretch), 1000.0));

                GraphicsPath path = new();
                bool open = false;
                DocPoint? first = null;

                foreach ((double x, double y) in fit.Curve(from, to, samples))
                {
                    if (!double.IsFinite(x) || !double.IsFinite(y))
                    {
                        open = false;
                        continue;
                    }

                    double alongFraction = FractionAlong(plot, domain, x, categories);
                    double upFraction = scale.Fraction(y);

                    if (alongFraction is < 0.0 or > 1.0 || upFraction is < 0.0 or > 1.0)
                    {
                        open = false;
                        continue;
                    }

                    DocPoint point = Point(area, alongFraction, upFraction, columns);
                    first ??= point;

                    if (open) path.LineTo(point);
                    else path.MoveTo(point);

                    open = true;
                }

                if (path.Commands.Count >= 2)
                {
                    shapes.Add(new ChartShape(
                        path,
                        null,
                        trendline.Line ?? series.Line ?? series.Fill ?? Colour.Black,
                        trendline.LineWidth));
                }

                if (fit.Equation(trendline, plot.ValueFormat) is not { Length: > 0 } equation)
                    continue;

                // ODF states where the equation goes; OOXML states nothing, so it falls back to
                // the curve's own top-left, which is what aDefaultPos is.
                DocPoint at = trendline.EquationAt is { } stated
                    ? new DocPoint(frame.X + stated.X, frame.Y + stated.Y)
                    : first ?? new DocPoint(area.Left, area.Top);

                foreach (string line in equation.Split('\n'))
                {
                    if (line.Length == 0) continue;
                    labels.Add(new ChartLabel(
                        line, at, ChartLabelAnchor.LeftMiddle, plot.LabelSize, AxisColour));
                    at = new DocPoint(at.X, at.Y + plot.LabelSize * ChartLineHeight);
                }
            }
        }
    }

    /// <summary>A chart's line height as a multiple of the em.</summary>
    /// <remarks>
    /// A chart's text shapes are not slide shapes: <c>chart2</c>'s view makes plain text shapes
    /// and sets no <c>FixedCellHeight</c>, so a line is the face's own 1.1499 em rather than
    /// EditEngine's flat 1.2. Only the equation needs it here, being the one piece of chart text
    /// that is more than one line.
    /// </remarks>
    private const double ChartLineHeight = 1.1499;

    /// <summary>
    /// The X values a trendline is fitted over: the series' own, or 1, 2, 3 … per category.
    /// </summary>
    private static IReadOnlyList<double?> XValuesOf(ChartSeries series, int categories)
    {
        if (series.XValues is { Count: > 0 } stated) return stated;

        int count = Math.Max(series.Values.Count, categories);
        double?[] indices = new double?[count];
        for (int at = 0; at < count; at++) indices[at] = at + 1;
        return indices;
    }

    /// <summary>The smallest and largest finite value in a sequence.</summary>
    private static (double? Minimum, double? Maximum) Extent(IReadOnlyList<double?> values)
    {
        double minimum = double.PositiveInfinity;
        double maximum = double.NegativeInfinity;

        foreach (double? point in values)
        {
            if (point is not { } value || !double.IsFinite(value)) continue;
            minimum = Math.Min(minimum, value);
            maximum = Math.Max(maximum, value);
        }

        return double.IsInfinity(minimum) ? (null, null) : (minimum, maximum);
    }

    /// <summary>
    /// Where a fitted X sits across the plot area, on either kind of horizontal axis.
    /// </summary>
    /// <remarks>
    /// A scatter chart's is its domain scale's fraction. A category chart's follows from
    /// <see cref="ChartPlot.ShiftedCategories"/>: on a shifted axis category <em>n</em> occupies
    /// the slot from <c>(n−1)/count</c> to <c>n/count</c>, so <c>x = n</c> falls at its centre;
    /// on an unshifted one the categories are points and <c>x = n</c> falls at
    /// <c>(n−1)/(count−1)</c>. Both agree with <see cref="CategoryAt"/> at integer <em>x</em>,
    /// which is the check that the trendline and the bars it crosses are in the same space.
    /// </remarks>
    private static double FractionAlong(
        ChartPlot plot, ChartScaleResult? domain, double x, int categories)
    {
        if (domain is { } across) return across.Fraction(x);
        if (categories <= 0) return 0.5;
        if (plot.ShiftedCategories) return (x - 0.5) / categories;
        return categories == 1 ? 0.5 : (x - 1.0) / (categories - 1);
    }

    /// <summary>
    /// A scatter chart's X scale, or null when the chart is plotted against categories.
    /// </summary>
    /// <remarks>
    /// <c>c:xVal</c> is a data sequence like any other and gets an axis scale of its own through
    /// the same automatism — except that <c>isExpandWideValuesToZero</c> is false for an X axis
    /// (<c>VSeriesPlotter::isExpandWideValuesToZero</c> tests <c>nDimensionIndex == 1</c>,
    /// <c>VSeriesPlotter.cxx:1742-1746</c>), so a domain running 20 to 120 keeps its minimum near
    /// 20 rather than being pulled down to zero the way a value axis would be.
    /// </remarks>
    private static ChartScaleResult? DomainScaleOf(ChartPlot plot)
    {
        double minimum = double.PositiveInfinity;
        double maximum = double.NegativeInfinity;
        bool any = false;

        foreach (ChartSeries series in plot.Series)
        {
            // A bubble chart is a scatter chart with a third number, so its X is a numeric
            // dimension with a scale of its own in exactly the same way — and its chart part has
            // the same pair of c:valAx and the same trap in reading them.
            if ((series.Kind ?? plot.Kind) is not (ChartPlotKind.Scatter or ChartPlotKind.Bubble))
                continue;

            if (series.XValues is not { } values) continue;

            foreach (double? point in values)
            {
                if (point is not { } value || !double.IsFinite(value)) continue;
                minimum = Math.Min(minimum, value);
                maximum = Math.Max(maximum, value);
                any = true;
            }
        }

        return any
            ? ChartScale.Resolve(plot.DomainScale, minimum, maximum, expandToZero: false)
            : null;
    }

    /// <summary>
    /// How many major intervals the value axis has room for, once its length is known.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>VCartesianAxis::estimateMaximumAutoMainIncrementCount</c>
    /// (<c>chart2/source/view/axes/VCartesianAxis.cxx:1559-1618</c>): the axis line's own length
    /// divided by the largest label shape measured so far — its <em>height</em> for a vertical
    /// axis and its <em>width</em> for a horizontal one — and ten whenever nothing has been
    /// measured yet, which is what makes the first pass ten.
    /// </para>
    /// <para>
    /// <strong>The text, not the shape it sits in.</strong> Every other reservation in this file
    /// uses <see cref="Shape"/>, which adds <c>ShapeFactory</c>'s insets; this one must not, and
    /// the corpus pins it down from both sides. <c>chart-bar-sheet.xlsx</c> draws its axis 54.6 pt
    /// long and LibreOffice labels it <c>0 50 … 200</c>: that is four intervals, which
    /// <c>54.6 / 11.5</c> gives and <c>54.6 / 17.5</c> does not — 17.5 gives three, and three
    /// forces the interval to 100. <c>chart-bar-sheet.ods</c> draws its axis 108.8 pt long and is
    /// labelled <c>0 20 … 180</c>: nine intervals, which <c>108.8 / 11.5</c> gives and
    /// <c>108.8 / 17.5</c> does not.
    /// </para>
    /// <para>
    /// <strong>And it is what separates a chart from a smaller copy of the same chart.</strong>
    /// <c>chart-bar-deck.odp</c> and <c>chart-bar-sheet.ods</c> hold the same eight numbers,
    /// peaking at 168, and LibreOffice labels the deck <c>0 20 … 180</c> over an axis 242 pt long
    /// and the sheet's <c>.xlsx</c> form <c>0 50 … 200</c> over one 55 pt long. Reproducing the
    /// deck without this and reusing it draws every bar 10% too tall against ticks that read
    /// perfectly plausibly.
    /// </para>
    /// </remarks>
    private static int IntervalsThatFit(
        ChartPlot plot,
        DocRect area,
        bool columns,
        ChartScaleResult scale,
        IChartTextMeasurer measurer)
    {
        // A stated interval is honoured whatever fits; only the automatic one is re-derived.
        if (plot.ValueScale.MajorUnit is { } stated && stated > 0.0)
            return ChartScale.MaximumAutoIntervalCount;

        Length available;
        Length needed;

        if (columns)
        {
            available = area.Height;
            needed = measurer.Measure("0", plot.LabelSize).Height;
        }
        else
        {
            available = area.Width;
            needed = Length.Zero;

            foreach (double tick in scale.MajorTicks())
            {
                Length width = measurer.Measure(
                    ChartDataLabel.Write(tick, plot.ValueFormat), plot.LabelSize).Width;
                if (width > needed) needed = width;
            }
        }

        if (needed <= Length.Zero) return ChartScale.MaximumAutoIntervalCount;

        return Math.Clamp(
            (int)(available.Emu / needed.Emu),
            ChartScale.MinimumAutoIntervalCount,
            ChartScale.MaximumAutoIntervalCount);
    }

    /// <summary>
    /// The rectangle the diagram <em>and its axes</em> are laid out in: the frame less the page
    /// margin, the main title, the legend and the axis titles, and before any axis <em>label</em>
    /// is reserved out of it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is <c>CreateShapeParam2D::maRemainingSpace</c> at the moment
    /// <c>VDiagram::createShapes</c> is handed it, and it is worth having as a value of its own
    /// because <strong>an ODF chart states it</strong>: <c>chart:plot-area</c>'s own
    /// <c>svg:x</c>…<c>svg:height</c> is this rectangle and the <c>chart:coordinate-region</c>
    /// inside it is the inner one, so a file gives both halves of the composition separately and
    /// a discrepancy can be attributed to one of them. Measured over the 98 charts in
    /// <c>chart2/qa/extras/data/</c>'s ODF documents that state both: splitting them apart is what
    /// showed that the error was in this rectangle — the legend's width — and not in the label
    /// reservations everyone had been looking at.
    /// </para>
    /// <para>
    /// <strong>The order is title, legend, axis titles, and only then the page margin.</strong>
    /// <c>ChartView::createShapes2D</c> (<c>:1934-1975</c>) starts from the whole page and lets
    /// <c>lcl_createTitle</c>, <c>lcl_createLegend</c> and <c>createAxisTitleShapes2D</c> each
    /// subtract what they took; <c>getAvailablePosAndSizeForDiagram</c> (<c>:921-943</c>) applies
    /// the two per cent afterwards, on all four sides. Every term here is additive on a distinct
    /// edge, so composing them in the other order gives the same rectangle — but the axis titles
    /// belong <em>here</em> and not among the label reservations, which is where they used to be:
    /// they come off before the diagram exists, so they cannot be part of what the diagram's own
    /// second pass measures.
    /// </para>
    /// <para>
    /// <strong>A pie's margin is a flat 350, not two per cent.</strong>
    /// <c>constPageLayoutFixedDistance</c>, under the comment "Only pie chart uses fixed size
    /// margins" (<c>ChartView.cxx:919, 935-940</c>), and it is gated on the chart type being pie
    /// rather than on the chart having no axes — a doughnut is a different chart type and keeps the
    /// proportional margin.
    /// </para>
    /// </remarks>
    private static DocRect DiagramAreaOf(ChartPlot plot, DocRect frame, IChartTextMeasurer measurer)
    {
        Length marginX = plot.Kind is ChartPlotKind.Pie or ChartPlotKind.OfPie && !plot.Rings
            ? PieMargin
            : frame.Width * PageMargin;
        Length marginY = plot.Kind is ChartPlotKind.Pie or ChartPlotKind.OfPie && !plot.Rings
            ? PieMargin
            : frame.Height * PageMargin;

        Length left = frame.X + marginX;
        Length top = frame.Y + marginY;
        Length right = frame.Right - marginX;
        Length bottom = frame.Bottom - marginY;

        // The main title's own distance is always the two per cent, even on a pie: the flat 350
        // belongs to `getAvailablePosAndSizeForDiagram` alone and `lcl_createTitle` reads
        // `constPageLayoutDistancePercentage` directly (`ChartView.cxx:1058-1061`). Adding the
        // pie's margin twice puts a titled pie 8 pt low on a 12 cm chart.
        if (plot.Title is { Length: > 0 } title)
        {
            top += Shape(measurer, title, plot.TitleSize).Height
                   + (frame.Height * PageMargin) + TitleGap;
        }

        // The legend is laid out against what the titles left of the *page*, before the two per
        // cent is taken off — the margin is applied last (`getAvailablePosAndSizeForDiagram`),
        // and the room the legend has to wrap its entries into columns is the larger rectangle.
        LegendBox legend = Legend(plot, LegendSpace(plot, frame, measurer), measurer);

        switch (plot.Legend)
        {
            case ChartLegendPosition.Right: right -= legend.Width + LegendMarginX; break;
            case ChartLegendPosition.Left: left += legend.Width + LegendMarginX; break;
            case ChartLegendPosition.Top: top += legend.Height + LegendMarginY; break;
            case ChartLegendPosition.Bottom: bottom -= legend.Height + LegendMarginY; break;
            default: break;
        }

        // The axis titles, which are laid out against the page and not against the diagram. The
        // one that goes at the bottom is whichever axis runs horizontally — the category axis on
        // a column chart, the value axis on a bar chart — because the alignment is a property of
        // the position (`TITLE_AT_STANDARD_X_AXIS_POSITION` is always `ALIGN_BOTTOM`) rather than
        // of the axis.
        if (plot.HasAxes)
        {
            bool columns = plot.Direction == ChartBarDirection.Column;

            string? beside = columns ? plot.ValueAxisTitle : plot.CategoryAxisTitle;
            string? below = columns ? plot.CategoryAxisTitle : plot.ValueAxisTitle;

            if (below is { Length: > 0 } under)
                bottom -= Shape(measurer, under, plot.AxisTitleSize).Height + CategoryTitleGap;
            if (beside is { Length: > 0 } side)
                left += Shape(measurer, side, plot.AxisTitleSize).Height + ValueTitleGap;

            if (plot.SecondaryValueAxisTitle is { Length: > 0 } second && plot.SecondaryAxisVisible)
                right -= Shape(measurer, second, plot.AxisTitleSize).Height + ValueTitleGap;
        }

        return right <= left || bottom <= top
            ? DocRect.Empty
            : new DocRect(left, top, right - left, bottom - top);
    }

    /// <summary>
    /// The inner plot rectangle: stated by the file when it states one, computed otherwise.
    /// </summary>
    /// <remarks>
    /// The <c>arranged</c> parameter is how the category labels came out when they needed
    /// rotating, thinning or staggering: the room reserved under the axis is then the
    /// arrangement's own depth rather than one line. It is null both on the first pass, before
    /// anything has been arranged, and for every axis whose labels fit as they are — which is what
    /// keeps the reservation on an ordinary chart exactly what it was.
    /// </remarks>
    private static DocRect PlotAreaOf(
        ChartPlot plot,
        DocRect frame,
        ChartScaleResult scale,
        ChartScaleResult? secondary,
        ChartScaleResult? domain,
        int categories,
        IChartTextMeasurer measurer,
        ChartAxisLabelLayout? arranged)
    {
        // Absolute, in the chart's own coordinates — ODF's chart:coordinate-region, which is
        // already in whatever space Place composed in.
        if (plot.PlotArea is { } stated)
            return new DocRect(frame.X + stated.X, frame.Y + stated.Y, stated.Width, stated.Height);

        // Fractions of the frame — OOXML's c:manualLayout, which states no space of its own.
        if (plot.PlotAreaFraction is { } fraction)
        {
            return new DocRect(
                frame.X + frame.Width * fraction.X,
                frame.Y + frame.Height * fraction.Y,
                frame.Width * fraction.Width,
                frame.Height * fraction.Height);
        }

        // The computed path, which is what every OOXML chart takes: the outer rectangle, then the
        // axes' labels and titles out of it — AXIS2D_TICKLENGTH and AXIS2D_TICKLABELSPACING for
        // the gaps, ChartView.cxx:1070-1077 for the axis titles.
        DocRect area = DiagramAreaOf(plot, frame, measurer);
        if (area.Width <= Length.Zero || area.Height <= Length.Zero) return DocRect.Empty;

        Length left = area.Left;
        Length top = area.Top;
        Length right = area.Right;
        Length bottom = area.Bottom;

        // A chart with no axes — a pie — reserves nothing for labels it does not draw, and what is
        // left after the title and the legend is the whole diagram.
        if (!plot.HasAxes)
        {
            // Except a radar chart, whose category labels sit *outside* the web on all four sides
            // rather than along one edge. Measured on chart2/qa/extras/data/docx/radar-chart-labels.docx,
            // whose frame is 431.2 x 251.2 pt: LibreOffice draws the outermost web at a radius of
            // 104.8 pt, and reserving a text shape's height above and below — 11.5 pt of text plus
            // ShapeFactory's two 0.30 em insets, so 17.5 — leaves 206.2 pt and a radius of 103.1.
            // Reserving nothing gives 120.6, which is 15% too big and puts the top vertex through
            // its own label.
            if (plot.Kind is ChartPlotKind.Radar && plot.CategoryAxisVisible)
            {
                Length wide = WidestCategoryLabel(plot, categories, measurer);
                Length tall = Shape(measurer, "0", plot.LabelSize).Height;

                left += wide;
                right -= wide;
                top += tall;
                bottom -= tall;
            }

            return right <= left || bottom <= top
                ? DocRect.Empty
                : Squared(plot, new DocRect(left, top, right - left, bottom - top));
        }

        bool columns = plot.Direction == ChartBarDirection.Column;

        // The value axis' labels are as wide as the widest of them; the category axis' are one
        // line tall. Both sit a tick length plus a label spacing away from the axis.
        // A deleted axis reserves nothing, which is the whole of what makes the plot area grow
        // into the room its labels would have taken.
        Length valueLabel = plot.ValueAxisVisible
            ? WidestValueLabel(scale, plot.ValueFormat, plot.LabelSize, measurer)
            : Length.Zero;

        Length labelHeight = measurer.Measure("0", plot.LabelSize).Height;

        // A scatter chart's horizontal axis is numeric, so what sits under it is the widest of its
        // own ticks rather than the widest category name — and the last of them overhangs the
        // right edge by half its width exactly as a horizontal value axis' does.
        Length categoryLabel = !plot.CategoryAxisVisible
            ? Length.Zero
            : domain is { } across
                ? WidestValueLabel(across, plot.DomainFormat, plot.LabelSize, measurer)
                : WidestCategoryLabel(plot, categories, measurer);

        Length valueSpace = plot.ValueAxisVisible ? TickLength + LabelSpacing : Length.Zero;
        Length categorySpace = plot.CategoryAxisVisible ? TickLength + LabelSpacing : Length.Zero;
        Length valueHeight = plot.ValueAxisVisible ? labelHeight : Length.Zero;

        // Upright labels reserve one line of *text*, which is what every measurement in this file
        // was fitted against and what the six corpus charts still agree with. Rotated, thinned or
        // staggered labels reserve their arrangement's own depth instead — the rotated shape's
        // height, insets included, which is what LibreOffice reserves and is several times a line
        // on an axis of long names turned 45°.
        Length categoryHeight = plot.DataTable is not null
            ? DataTableHeight(plot, measurer)
            : !plot.CategoryAxisVisible
                ? Length.Zero
                : arranged is { } layout && !IsPlain(layout)
                    ? layout.Reserved
                    : labelHeight;

        if (columns)
        {
            left += valueLabel + valueSpace;
            bottom -= categoryHeight + categorySpace;

            // A secondary value axis is drawn on the far side of the plot area and reserves its
            // own labels there, which is the whole of what makes room for it. Its *title* was
            // taken off in DiagramAreaOf, with the other three.
            if (secondary is { } second && plot.SecondaryAxisVisible)
            {
                right -= WidestValueLabel(
                             second, plot.SecondaryValueFormat, plot.LabelSize, measurer)
                         + TickLength + LabelSpacing;
            }

            // On an unshifted axis the first and the last label are centred on the plot area's own
            // corners, so half of each hangs outside it — and the left edge takes whichever of
            // that and the value labels is the deeper. See EndLabelOverhang.
            (Length firstLabel, Length lastLabel) =
                arranged is { } ends && !IsPlain(ends)
                    ? (Length.Zero, Length.Zero)
                    : EndLabelOverhang(plot, domain, categories, measurer);

            if (area.Left + firstLabel > left) left = area.Left + firstLabel;

            if (secondary is null || !plot.SecondaryAxisVisible)
            {
                if (area.Right - lastLabel < right) right = area.Right - lastLabel;
            }

            // The topmost value label is centred on the axis' top, so half of it sticks out
            // above the plot area and would be clipped by the frame without this.
            top += valueHeight / 2;
        }
        else
        {
            left += categoryLabel + categorySpace;
            bottom -= valueHeight + valueSpace;

            // The last value label is centred on the axis' right end, so half of it overhangs.
            right -= valueLabel / 2;
        }

        return right <= left || bottom <= top
            ? DocRect.Empty
            : new DocRect(left, top, right - left, bottom - top);
    }

    /// <summary>The value axis: its line, its ticks, its gridlines and its labels.</summary>
    /// <remarks>
    /// <strong>A secondary axis is the same axis on the other side of the plot area, and it draws
    /// no gridlines.</strong> Two sets of gridlines over one plot area are two grids at different
    /// spacings, which LibreOffice does not draw either: <c>c:majorGridlines</c> on the secondary
    /// <c>c:valAx</c> is honoured by chart2 only when the primary has none.
    /// </remarks>
    private static void AddValueAxis(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        bool columns,
        NumberFormatCode? format,
        bool secondary,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        // The axis line itself runs the full extent of the plot area on the side the value axis
        // is on: the left edge for columns, the bottom edge for bars — and the far side of each
        // for a secondary axis.
        Length axisX = secondary ? area.Right : area.Left;
        Length axisY = secondary ? area.Top : area.Bottom;
        int outward = secondary ? 1 : -1;

        // A deleted axis keeps its gridlines and loses everything else, so the line, the ticks and
        // the labels are all gated and the grid inside the loop is not.
        bool visible = secondary ? plot.SecondaryAxisVisible : plot.ValueAxisVisible;

        if (visible)
        {
            lines.Add(columns
                ? new ChartLine(
                    new DocPoint(axisX, area.Top), new DocPoint(axisX, area.Bottom), AxisColour)
                : new ChartLine(
                    new DocPoint(area.Left, axisY), new DocPoint(area.Right, axisY), AxisColour));
        }

        foreach (double tick in scale.MajorTicks())
        {
            double along = scale.Fraction(tick);

            if (columns)
            {
                // A fraction of 0 is the axis minimum, which is the *bottom* of a column
                // chart's plot area — hence the subtraction rather than an addition.
                Length y = area.Bottom - area.Height * along;

                if (!secondary && plot.ValueGrid is { } grid)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(area.Left, y), new DocPoint(area.Right, y), grid));
                }

                if (!visible) continue;

                lines.Add(new ChartLine(
                    new DocPoint(axisX + TickLength * outward, y),
                    new DocPoint(axisX, y),
                    AxisColour));

                labels.Add(new ChartLabel(
                    ChartDataLabel.Write(tick, format),
                    new DocPoint(axisX + (TickLength + LabelSpacing) * outward, y),
                    secondary ? ChartLabelAnchor.LeftMiddle : ChartLabelAnchor.RightMiddle,
                    plot.LabelSize,
                    AxisColour));
            }
            else
            {
                Length x = area.Left + area.Width * along;

                if (!secondary && plot.ValueGrid is { } grid)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(x, area.Top), new DocPoint(x, area.Bottom), grid));
                }

                if (!visible) continue;

                lines.Add(new ChartLine(
                    new DocPoint(x, axisY),
                    new DocPoint(x, axisY - TickLength * outward),
                    AxisColour));

                labels.Add(new ChartLabel(
                    ChartDataLabel.Write(tick, format),
                    new DocPoint(x, axisY - (TickLength + LabelSpacing) * outward),
                    secondary ? ChartLabelAnchor.CentreBottom : ChartLabelAnchor.CentreTop,
                    plot.LabelSize,
                    AxisColour));
            }
        }
    }

    /// <summary>
    /// A scatter chart's X axis: numeric ticks along the category axis' place.
    /// </summary>
    /// <remarks>
    /// The same shape as <see cref="AddValueAxis"/> with the roles of the two dimensions swapped,
    /// which is what a scatter chart is: <c>VSeriesPlotter</c> builds it from two
    /// <c>ExplicitScaleData</c> rather than one scale and a category list, and the only structural
    /// difference from a line chart is that the point's position across the plot area comes from
    /// its own number instead of from its index.
    /// </remarks>
    private static void AddDomainAxis(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult domain,
        bool columns,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        if (plot.CategoryAxisVisible)
        {
            lines.Add(columns
                ? new ChartLine(
                    new DocPoint(area.Left, area.Bottom),
                    new DocPoint(area.Right, area.Bottom),
                    AxisColour)
                : new ChartLine(
                    new DocPoint(area.Left, area.Top),
                    new DocPoint(area.Left, area.Bottom),
                    AxisColour));
        }

        foreach (double tick in domain.MajorTicks())
        {
            double along = domain.Fraction(tick);
            string text = ChartDataLabel.Write(tick, plot.DomainFormat);

            if (columns)
            {
                Length x = area.Left + area.Width * along;

                if (plot.CategoryGrid is { } grid)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(x, area.Top), new DocPoint(x, area.Bottom), grid));
                }

                if (!plot.CategoryAxisVisible) continue;

                lines.Add(new ChartLine(
                    new DocPoint(x, area.Bottom), new DocPoint(x, area.Bottom + TickLength), AxisColour));
                labels.Add(new ChartLabel(
                    text,
                    new DocPoint(x, area.Bottom + TickLength + LabelSpacing),
                    ChartLabelAnchor.CentreTop,
                    plot.LabelSize,
                    AxisColour));
            }
            else
            {
                Length y = area.Bottom - area.Height * along;
                if (!plot.CategoryAxisVisible) continue;

                lines.Add(new ChartLine(
                    new DocPoint(area.Left - TickLength, y), new DocPoint(area.Left, y), AxisColour));
                labels.Add(new ChartLabel(
                    text,
                    new DocPoint(area.Left - TickLength - LabelSpacing, y),
                    ChartLabelAnchor.RightMiddle,
                    plot.LabelSize,
                    AxisColour));
            }
        }
    }

    /// <summary>
    /// The category axis: its line, one tick per category boundary, and one label per category.
    /// </summary>
    /// <remarks>
    /// <strong>Ticks fall between categories and labels fall in the middle of them.</strong> Four
    /// categories produce five ticks, not four — the reference PDF draws them at the plot area's
    /// left edge, its right edge and the three boundaries between — and the labels sit at the
    /// centres, which is what <c>c:crossBetween val="between"</c> asks for and what every bar
    /// chart uses. Drawing a tick per label instead puts every category label half a slot from
    /// its own bars.
    /// </remarks>
    private static void AddCategoryAxis(
        ChartPlot plot,
        DocRect area,
        int categories,
        bool columns,
        ChartAxisLabelLayout? arranged,
        IChartTextMeasurer measurer,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        if (plot.CategoryAxisVisible)
        {
            lines.Add(columns
                ? new ChartLine(
                    new DocPoint(area.Left, area.Bottom),
                    new DocPoint(area.Right, area.Bottom),
                    AxisColour)
                : new ChartLine(
                    new DocPoint(area.Left, area.Top),
                    new DocPoint(area.Left, area.Bottom),
                    AxisColour));
        }

        if (categories <= 0) return;

        // A shifted axis is divided into slots, so n categories give n + 1 boundaries; an
        // unshifted one is marked at n points, the first and last on the plot area's own edges.
        int ticks = plot.ShiftedCategories ? categories : categories - 1;

        for (int at = 0; at <= ticks; at++)
        {
            double along = ticks == 0 ? 0.0 : (double)at / ticks;

            if (columns)
            {
                Length x = area.Left + area.Width * along;

                if (plot.CategoryGrid is { } grid)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(x, area.Top), new DocPoint(x, area.Bottom), grid));
                }

                if (plot.CategoryAxisVisible)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(x, area.Bottom),
                        new DocPoint(x, area.Bottom + TickLength),
                        AxisColour));
                }
            }
            else
            {
                Length y = area.Bottom - area.Height * along;

                if (plot.CategoryGrid is { } grid)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(area.Left, y), new DocPoint(area.Right, y), grid));
                }

                if (plot.CategoryAxisVisible)
                {
                    lines.Add(new ChartLine(
                        new DocPoint(area.Left - TickLength, y),
                        new DocPoint(area.Left, y),
                        AxisColour));
                }
            }
        }

        // A data table's header row *is* the category labels, so the axis draws none of its own —
        // VAxisProperties.cxx:336-343 turns DisplayLabels off outright. Drawing both is what puts
        // every category name on the page twice.
        if (!plot.CategoryAxisVisible || plot.DataTable is not null) return;

        ChartAxisLabelLayout layout =
            arranged ?? new ChartAxisLabelLayout(0.0, 1, false, Length.Zero);

        int rhythm = Math.Max(1, layout.Rhythm);
        double cosine = Math.Abs(Math.Cos(layout.Rotation));
        double sine = Math.Abs(Math.Sin(layout.Rotation));

        for (int at = 0; at < categories; at++)
        {
            if (at >= plot.Categories.Count) continue;

            // Thinned out: every nth label survives and the rest are simply not drawn, which is
            // removeShapesAtWrongRhythm. Tick zero always survives, so an axis never ends up with
            // no labels at all however crowded it is.
            if (at % rhythm != 0) continue;

            if (ChartDataLabel.WriteCategory(plot.Categories[at], plot.CategoryFormat)
                is not { Length: > 0 } text)
            {
                continue;
            }

            double centre = CategoryAt(plot, at, categories);

            if (!columns)
            {
                labels.Add(new ChartLabel(
                    text,
                    new DocPoint(
                        area.Left - TickLength - LabelSpacing,
                        area.Bottom - area.Height * centre),
                    ChartLabelAnchor.RightMiddle,
                    plot.LabelSize,
                    AxisColour));

                continue;
            }

            Length x = area.Left + area.Width * centre;
            Length top = area.Bottom + TickLength + LabelSpacing;

            // The second row of a staggered axis sits one row below the first.
            if (layout.Staggered && at / rhythm % 2 == 1) top += layout.Reserved / 2;

            if (layout.Rotation == 0.0)
            {
                labels.Add(new ChartLabel(
                    text, new DocPoint(x, top), ChartLabelAnchor.CentreTop,
                    plot.LabelSize, AxisColour));

                continue;
            }

            // A rotated label is placed by the centre of its rotated bounding box, because that is
            // the only thing a glyph run — which carries an origin and advances, not a matrix —
            // can be positioned by after the fact. Measured against LibreOffice's own PDF for
            // bnc889755.pptx: its rotated labels' boxes are centred on the tick horizontally and
            // start at the tick-to-text distance below the axis, which is exactly this.
            DocSize box = Shape(measurer, text, plot.LabelSize);
            Length depth = box.Width * sine + box.Height * cosine;

            labels.Add(new ChartLabel(
                text,
                new DocPoint(x, top + depth / 2),
                ChartLabelAnchor.Centre,
                plot.LabelSize,
                AxisColour,
                layout.Rotation));
        }
    }

    /// <summary>How tall the data table is: one row per series plus the header row.</summary>
    /// <remarks>
    /// A cell's text distances are the same <c>0.18</c>/<c>0.30 × fontHeight</c> every other piece
    /// of chart text gets — <c>DataTableView::setCellCharAndParagraphProperties</c> computes them
    /// with the identical two constants (<c>DataTableView.cxx:171-180</c>) — so a row is exactly
    /// one <see cref="Shape"/> tall and nothing new needs measuring.
    /// </remarks>
    private static Length DataTableHeight(ChartPlot plot, IChartTextMeasurer measurer)
        => Shape(measurer, "0", plot.LabelSize).Height * (plot.Series.Count + 1);

    /// <summary>
    /// The table of numbers under the plot: its grid, its header row and one row per series.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>DataTableView::createShapes</c> (<c>chart2/source/view/main/DataTableView.cxx:246-540</c>)
    /// reduced to what a display list needs. Its columns are the category slots — the flag chart2
    /// calls <c>m_bDataTableAlignAxisValuesWithColumns</c>, true for dimension 0, means the column
    /// width <em>is</em> the axis step — and its one extra column, on the left, holds the series
    /// names in the room the value axis' labels already occupy.
    /// </para>
    /// <para>
    /// <strong>The header row is the category axis' labels and not a copy of them.</strong> See
    /// <see cref="AddCategoryAxis"/>: the axis stops drawing labels the moment a table appears, so
    /// the names are written once, here.
    /// </para>
    /// </remarks>
    private static void AddDataTable(
        ChartPlot plot,
        DocRect frame,
        DocRect area,
        int categories,
        bool columns,
        IChartTextMeasurer measurer,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        if (plot.DataTable is not { } table) return;
        if (!columns || categories <= 0 || plot.Series.Count == 0) return;

        Length row = Shape(measurer, "0", plot.LabelSize).Height;
        Length top = area.Bottom;
        Length column = area.Width / categories;

        // The header column reaches back over the room the value axis' labels take, which is where
        // LibreOffice puts it: the table starts at the axis' own start and its row headers hang to
        // the left of it.
        Length left = frame.X + frame.Width * PageMargin;
        if (left > area.Left) left = area.Left;

        int rows = plot.Series.Count + 1;
        Length bottom = top + row * rows;

        if (table.Outline)
        {
            lines.Add(new ChartLine(
                new DocPoint(left, top), new DocPoint(area.Right, top), table.Line));
            lines.Add(new ChartLine(
                new DocPoint(left, bottom), new DocPoint(area.Right, bottom), table.Line));
            lines.Add(new ChartLine(
                new DocPoint(left, top), new DocPoint(left, bottom), table.Line));
            lines.Add(new ChartLine(
                new DocPoint(area.Right, top), new DocPoint(area.Right, bottom), table.Line));
        }

        if (table.HorizontalBorders)
        {
            for (int at = 1; at < rows; at++)
            {
                Length y = top + row * at;
                lines.Add(new ChartLine(
                    new DocPoint(left, y), new DocPoint(area.Right, y), table.Line));
            }
        }

        if (table.VerticalBorders)
        {
            lines.Add(new ChartLine(
                new DocPoint(area.Left, top), new DocPoint(area.Left, bottom), table.Line));

            for (int at = 1; at < categories; at++)
            {
                Length x = area.Left + column * at;
                lines.Add(new ChartLine(
                    new DocPoint(x, top), new DocPoint(x, bottom), table.Line));
            }
        }

        for (int at = 0; at < categories; at++)
        {
            if (at >= plot.Categories.Count) continue;
            if (ChartDataLabel.WriteCategory(plot.Categories[at], plot.CategoryFormat)
                is not { Length: > 0 } text)
            {
                continue;
            }

            labels.Add(new ChartLabel(
                text,
                new DocPoint(area.Left + column * (at + 0.5), top + row / 2),
                ChartLabelAnchor.Centre,
                plot.LabelSize,
                AxisColour));
        }

        for (int series = 0; series < plot.Series.Count; series++)
        {
            Length middle = top + row * (series + 1) + row / 2;
            ChartSeries plotted = plot.Series[series];

            if (table.Keys && plotted.Fill is { } key)
            {
                // The key is a filled square of 0.6 of the font height
                // (DataTableView.cxx:364), drawn here as a stroke of that width so that it needs
                // no box of its own.
                Length side = plot.LabelSize * 0.6;

                lines.Add(new ChartLine(
                    new DocPoint(left + side / 4, middle),
                    new DocPoint(left + side * 1.25, middle),
                    key,
                    side));
            }

            if (plotted.Name is { Length: > 0 } name)
            {
                Length indent = table.Keys ? plot.LabelSize * 1.1 : plot.LabelSize * 0.18;
                labels.Add(new ChartLabel(
                    name,
                    new DocPoint(left + indent, middle),
                    ChartLabelAnchor.LeftMiddle,
                    plot.LabelSize,
                    AxisColour));
            }

            for (int at = 0; at < categories; at++)
            {
                if (at >= plotted.Values.Count) continue;
                if (plotted.Values[at] is not { } value || !double.IsFinite(value)) continue;

                labels.Add(new ChartLabel(
                    ChartDataLabel.Write(value, plot.ValueFormat),
                    new DocPoint(area.Left + column * (at + 0.5), middle),
                    ChartLabelAnchor.Centre,
                    plot.LabelSize,
                    AxisColour));
            }
        }
    }

    /// <summary>
    /// Whether an arrangement is the one every uncrowded axis gets.
    /// </summary>
    /// <remarks>
    /// The test that keeps a chart whose labels fit on exactly the measurements it had before any
    /// of this existed: upright, every label drawn, one row.
    /// </remarks>
    private static bool IsPlain(ChartAxisLabelLayout layout)
        => layout.Rotation == 0.0 && layout.Rhythm <= 1 && !layout.Staggered;

    /// <summary>
    /// How the category labels come out on the axis the plot rectangle gives them.
    /// </summary>
    private static ChartAxisLabelLayout ArrangeCategories(
        ChartPlot plot, DocRect area, int categories, IChartTextMeasurer measurer)
    {
        string?[] texts = new string?[categories];
        Length[] centres = new Length[categories];

        for (int at = 0; at < categories; at++)
        {
            texts[at] = at < plot.Categories.Count
                ? ChartDataLabel.WriteCategory(plot.Categories[at], plot.CategoryFormat)
                : null;

            centres[at] = area.Left + area.Width * CategoryAt(plot, at, categories);
        }

        return ChartAxisLabels.Resolve(
            texts, centres, plot.CategoryAxisText, plot.LabelSize, measurer);
    }


    /// <summary>
    /// Where a category sits along the axis, 0 at the plot area's start and 1 at its end.
    /// </summary>
    /// <remarks>
    /// The middle of its slot on a shifted axis and the tick itself on an unshifted one; see
    /// <see cref="ChartPlot.ShiftedCategories"/> for which chart type gets which. A single
    /// category on an unshifted axis has nowhere to be but the middle, which is what LibreOffice
    /// draws and what the division would otherwise make a division by zero.
    /// </remarks>
    private static double CategoryAt(ChartPlot plot, int index, int categories)
    {
        if (categories <= 0) return 0.5;
        if (plot.ShiftedCategories) return (index + 0.5) / categories;
        return categories == 1 ? 0.5 : (double)index / (categories - 1);
    }

    /// <summary>
    /// A line or scatter chart: one polyline per series through its points.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>VSeriesPlotter</c>'s line plotter joins consecutive points and <em>breaks the line at a
    /// gap</em> rather than bridging it — a category a series has no value for ends the current
    /// run and starts a new one (<c>AreaChart::createShapes</c>'s
    /// <c>PolyPolygonShapeInfo</c> handling of <c>bIsVisible</c>). Bridging instead draws a
    /// straight segment across the hole, which is the one thing a reader cannot tell from a real
    /// value.
    /// </para>
    /// <para>
    /// <strong>A scatter chart's points are placed by their own X and not by their index.</strong>
    /// When <paramref name="domain"/> is set the fraction across the plot area is
    /// <c>domain.Fraction(x)</c>, which is the only structural difference between the two types;
    /// spacing them evenly instead is right whenever the X values happen to be evenly spaced and
    /// wrong in proportion to how unevenly they are not.
    /// </para>
    /// </remarks>
    private static void AddLines(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        ChartScaleResult? domain,
        int categories,
        bool columns,
        List<ChartShape> shapes,
        List<ChartLabel> labels)
    {
        if (categories <= 0) return;

        foreach (ChartSeries series in plot.Series)
        {
            GraphicsPath path = new();
            bool open = false;
            List<(DocPoint At, int Index, double Value)> points = [];

            int count = domain is not null && series.XValues is { } xs
                ? Math.Max(categories, xs.Count)
                : categories;

            for (int at = 0; at < count; at++)
            {
                if (at >= series.Values.Count
                    || series.Values[at] is not { } value
                    || !double.IsFinite(value)
                    || AcrossAt(plot, series, domain, at, categories) is not { } across)
                {
                    open = false;
                    continue;
                }

                DocPoint point = Point(area, across, scale.Fraction(value), columns);
                points.Add((point, at, value));

                if (open) path.LineTo(point);
                else path.MoveTo(point);

                open = true;
            }

            // Stroked in the series' fill when it states no line of its own, because that is what
            // both formats mean by a line series' colour: OOXML puts it on a:ln and ODF on the
            // series' stroke, and a series that states only a fill is drawn in that fill.
            Colour stroke = series.Line ?? series.Fill ?? Colour.Black;

            if (series.HasLine && path.Commands.Count >= 2)
                shapes.Add(new ChartShape(path, null, stroke, series.LineWidth));

            if (series.Marker != ChartMarker.None)
            {
                Length size = plot.LabelSize * MarkerSize;
                foreach ((DocPoint at, _, _) in points)
                    shapes.Add(Marker(series.Marker, at, size, series.Fill ?? stroke, stroke));
            }

            AddPointLabels(plot, series, points, ChartLabelPlacement.Right, area, labels);
        }
    }

    /// <summary>
    /// Where one point sits across the plot area, 0 at the start and 1 at the end.
    /// </summary>
    /// <remarks>
    /// The scatter case and the category case in one place, because the two differ only here.
    /// Null when a scatter point states no X, which is a genuine gap in the domain sequence and
    /// breaks the polyline exactly as a missing Y does.
    /// </remarks>
    private static double? AcrossAt(
        ChartPlot plot, ChartSeries series, ChartScaleResult? domain, int at, int categories)
    {
        if (domain is not { } across || series.XValues is not { } xs)
            return at < categories ? CategoryAt(plot, at, categories) : null;

        if (at >= xs.Count || xs[at] is not { } x || !double.IsFinite(x)) return null;
        return across.Fraction(x);
    }

    /// <summary>
    /// The data labels of a series drawn at points rather than as rectangles.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A line, a scatter and an area chart all name a single point, so the placement reduces to
    /// which side of it the text goes: <c>RIGHT</c> for a line and a scatter, <c>CENTER</c> for an
    /// area (<c>oox/source/drawingml/chart/typegroupconverter.cxx:98-99</c>). Anything else the
    /// file states is honoured where it maps onto one of the four anchors and falls back to the
    /// type's own default where it does not.
    /// </para>
    /// <para>
    /// The gap is a fifth of the type size, which is <c>LabelPositionHelper</c>'s own offset once
    /// its <c>fXOffset</c>/<c>fYOffset</c> pair is reduced to the non-3-D case.
    /// </para>
    /// </remarks>
    private static void AddPointLabels(
        ChartPlot plot,
        ChartSeries series,
        List<(DocPoint At, int Index, double Value)> points,
        ChartLabelPlacement fallback,
        DocRect area,
        List<ChartLabel> labels)
    {
        if (points.Count == 0) return;

        double total = series.Total();
        Length gap = plot.LabelSize / 5;

        foreach ((DocPoint at, int index, double value) in points)
        {
            if (series.LabelAt(index) is not { Draws: true } label) continue;

            string? text = label.Compose(
                index < plot.Categories.Count ? plot.Categories[index] : null,
                series.Name,
                value,
                total);

            if (text is not { Length: > 0 }) continue;

            (DocPoint where, ChartLabelAnchor anchor) = (label.Placement ?? fallback) switch
            {
                ChartLabelPlacement.Left =>
                    (new DocPoint(at.X - gap, at.Y), ChartLabelAnchor.RightMiddle),
                ChartLabelPlacement.Top or ChartLabelPlacement.Outside =>
                    (new DocPoint(at.X, at.Y - gap), ChartLabelAnchor.CentreBottom),
                ChartLabelPlacement.Bottom =>
                    (new DocPoint(at.X, at.Y + gap), ChartLabelAnchor.CentreTop),
                ChartLabelPlacement.Centre or ChartLabelPlacement.Inside =>
                    (at, ChartLabelAnchor.Centre),
                _ => (new DocPoint(at.X + gap, at.Y), ChartLabelAnchor.LeftMiddle),
            };

            // A label that would fall outside the plot area is pulled back inside it, which is
            // what "bestFit" asks for and what LibreOffice does for every placement anyway
            // (LabelPositionHelper::correctPositionForRotation's clamp against the diagram).
            if (where.X < area.Left) where = new DocPoint(area.Left, where.Y);
            if (where.X > area.Right) where = new DocPoint(area.Right, where.Y);

            labels.Add(new ChartLabel(text, where, anchor, plot.LabelSize, AxisColour));
        }
    }

    /// <summary>A marker's side, as a fraction of the label size.</summary>
    /// <remarks>
    /// <c>VDataSeries::getSymbolProperties</c> defaults a symbol to 250 × 250 hundredths of a
    /// millimetre, which on the 10 pt labels every chart in the corpus uses is 0.71 of the em.
    /// Expressing it against the type rather than absolutely is what makes it survive the
    /// stretch an embedded chart goes through.
    /// </remarks>
    private const double MarkerSize = 0.7;

    /// <summary>One marker, as a path centred on the point.</summary>
    private static ChartShape Marker(
        ChartMarker kind, DocPoint at, Length size, Colour fill, Colour stroke)
    {
        Length half = size / 2;
        GraphicsPath path = new();

        switch (kind)
        {
            case ChartMarker.Circle:
            {
                // Four cubics, the standard 0.5523 approximation; a circle drawn as a square is
                // the one marker shape a reader notices at a glance.
                Length k = half * 0.5522847498307936;
                path.MoveTo(new DocPoint(at.X + half, at.Y));
                path.CubicTo(
                    new DocPoint(at.X + half, at.Y + k), new DocPoint(at.X + k, at.Y + half),
                    new DocPoint(at.X, at.Y + half));
                path.CubicTo(
                    new DocPoint(at.X - k, at.Y + half), new DocPoint(at.X - half, at.Y + k),
                    new DocPoint(at.X - half, at.Y));
                path.CubicTo(
                    new DocPoint(at.X - half, at.Y - k), new DocPoint(at.X - k, at.Y - half),
                    new DocPoint(at.X, at.Y - half));
                path.CubicTo(
                    new DocPoint(at.X + k, at.Y - half), new DocPoint(at.X + half, at.Y - k),
                    new DocPoint(at.X + half, at.Y));
                path.Close();
                return new ChartShape(path, fill, null);
            }

            case ChartMarker.Diamond:
                path.MoveTo(new DocPoint(at.X, at.Y - half));
                path.LineTo(new DocPoint(at.X + half, at.Y));
                path.LineTo(new DocPoint(at.X, at.Y + half));
                path.LineTo(new DocPoint(at.X - half, at.Y));
                path.Close();
                return new ChartShape(path, fill, null);

            case ChartMarker.Triangle:
                path.MoveTo(new DocPoint(at.X, at.Y - half));
                path.LineTo(new DocPoint(at.X + half, at.Y + half));
                path.LineTo(new DocPoint(at.X - half, at.Y + half));
                path.Close();
                return new ChartShape(path, fill, null);

            case ChartMarker.Cross:
                path.MoveTo(new DocPoint(at.X - half, at.Y));
                path.LineTo(new DocPoint(at.X + half, at.Y));
                path.MoveTo(new DocPoint(at.X, at.Y - half));
                path.LineTo(new DocPoint(at.X, at.Y + half));
                return new ChartShape(path, null, stroke);

            case ChartMarker.Star:
                path.MoveTo(new DocPoint(at.X - half, at.Y - half));
                path.LineTo(new DocPoint(at.X + half, at.Y + half));
                path.MoveTo(new DocPoint(at.X + half, at.Y - half));
                path.LineTo(new DocPoint(at.X - half, at.Y + half));
                return new ChartShape(path, null, stroke);

            default:
                path.MoveTo(new DocPoint(at.X - half, at.Y - half));
                path.LineTo(new DocPoint(at.X + half, at.Y - half));
                path.LineTo(new DocPoint(at.X + half, at.Y + half));
                path.LineTo(new DocPoint(at.X - half, at.Y + half));
                path.Close();
                return new ChartShape(path, fill, null);
        }
    }

    /// <summary>
    /// An area chart: one filled region per series, from the baseline up to its points.
    /// </summary>
    /// <remarks>
    /// Stacked areas pile onto a running total per category exactly as stacked bars do, so the
    /// lower edge of a series is the upper edge of the one below it rather than the baseline.
    /// Unstacked areas are drawn in file order and overlap, which is what LibreOffice draws and
    /// what makes a later series hide an earlier one — the reason a real area chart is usually
    /// stacked.
    /// </remarks>
    private static void AddAreas(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        int categories,
        bool columns,
        List<ChartShape> shapes,
        List<ChartLabel> labels)
    {
        if (categories <= 0) return;

        double baseline = Math.Clamp(scale.Fraction(0.0), 0.0, 1.0);
        double[] running = new double[categories];
        double[] previous = new double[categories];

        for (int at = 0; at < categories; at++) previous[at] = baseline;

        foreach (ChartSeries series in plot.Series)
        {
            List<DocPoint> upper = [];
            List<DocPoint> lower = [];

            for (int at = 0; at < categories; at++)
            {
                double value = at < series.Values.Count && series.Values[at] is { } stated
                               && double.IsFinite(stated)
                    ? stated
                    : 0.0;

                double top;

                if (plot.IsStacked)
                {
                    running[at] += value;
                    top = scale.Fraction(running[at]);
                }
                else
                {
                    top = scale.Fraction(value);
                }

                double across = CategoryAt(plot, at, categories);
                upper.Add(Point(area, across, top, columns));
                lower.Add(Point(area, across, plot.IsStacked ? previous[at] : baseline, columns));
            }

            if (upper.Count < 2) continue;

            GraphicsPath path = new();
            path.MoveTo(upper[0]);
            for (int at = 1; at < upper.Count; at++) path.LineTo(upper[at]);
            for (int at = lower.Count - 1; at >= 0; at--) path.LineTo(lower[at]);
            path.Close();

            shapes.Add(new ChartShape(path, series.Fill, series.Line, series.LineWidth));

            List<(DocPoint, int, double)> points = [];
            for (int at = 0; at < categories && at < upper.Count; at++)
            {
                if (at >= series.Values.Count) continue;
                if (series.Values[at] is not { } stated || !double.IsFinite(stated)) continue;
                points.Add((upper[at], at, stated));
            }

            AddPointLabels(plot, series, points, ChartLabelPlacement.Centre, area, labels);

            if (plot.IsStacked)
            {
                for (int at = 0; at < categories; at++) previous[at] = scale.Fraction(running[at]);
            }
        }
    }

    /// <summary>
    /// A pie chart: one wedge per category of the first series.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A pie plots one series and colours it per point.</strong> Where a bar chart's
    /// colours belong to the series, a pie's belong to the points — <c>c:dPt</c>, ODF's
    /// <c>chart:data-point</c> — and the legend names the categories rather than the series. Only
    /// the first series is drawn, which is what a pie of several series means to every application
    /// but Excel's "pie of pie".
    /// </para>
    /// <para>
    /// <strong>The first wedge starts at twelve o'clock and they run clockwise.</strong>
    /// <c>PieChart::createShapes</c> begins at <c>fAngleDegree = 90</c> and subtracts, which is
    /// what both formats' default <c>firstSliceAng="0"</c> means. Starting at three o'clock, which
    /// is what a naive polar conversion gives, turns every wedge a quarter turn and is the kind of
    /// error that looks like a data ordering bug.
    /// </para>
    /// <para>
    /// The circle is inscribed in the plot area, its radius the smaller half-extent, which is
    /// <c>PolarPlottingPositionHelper</c>'s unit circle mapped through the diagram's rectangle.
    /// </para>
    /// </remarks>
    private static void AddWedges(
        ChartPlot plot, DocRect area, List<ChartShape> shapes, List<ChartLabel> labels)
    {
        if (plot.Series.Count == 0) return;

        DocPoint pieCentre = new(area.X + area.Width / 2, area.Y + area.Height / 2);
        Length outer = area.Width < area.Height ? area.Width / 2 : area.Height / 2;
        if (outer <= Length.Zero) return;

        // A doughnut is one ring per series, innermost first; a pie is the first series alone.
        // Ring k of n runs from k/(n+1) to (k+1)/(n+1) of the outer radius, which is
        // PiePositionHelper::getInnerAndOuterRadius with PieChart's m_fRadiusOffset of 1 — a
        // half-radius hole for the single-ring case. See ChartPlot.Rings.
        int rings = plot.Rings ? plot.Series.Count : 1;

        for (int ring = 0; ring < rings; ring++)
        {
            AddRing(
                plot,
                plot.Series[ring],
                pieCentre,
                plot.Rings ? outer * ((ring + 1) / (double)(rings + 1)) : Length.Zero,
                plot.Rings ? outer * ((ring + 2) / (double)(rings + 1)) : outer,
                shapes,
                labels);
        }
    }

    /// <summary>One ring of wedges: a whole pie when the inner radius is zero.</summary>
    private static void AddRing(
        ChartPlot plot,
        ChartSeries series,
        DocPoint centre,
        Length hole,
        Length radius,
        List<ChartShape> shapes,
        List<ChartLabel> labels)
    {
        double total = series.Total();
        if (!(total > 0.0)) return;
        if (radius <= Length.Zero) return;

        double start = Math.PI / 2;

        for (int at = 0; at < series.Values.Count; at++)
        {
            if (series.Values[at] is not { } value || !double.IsFinite(value)) continue;

            double sweep = Math.Abs(value) / total * (2 * Math.PI);
            if (sweep <= 0.0) { continue; }

            shapes.Add(new ChartShape(
                hole > Length.Zero
                    ? Annulus(centre, hole, radius, start, -sweep)
                    : Wedge(centre, radius, start, -sweep),
                series.FillAt(at),
                series.Line,
                series.LineWidth));

            // The label sits on the wedge's own bisector. Its distance from the centre is what the
            // placement decides: chart2's AVOID_OVERLAP and CENTER both put it inside, at 0.5 of
            // the radius (PolarLabelPositionHelper::getLabelScreenPositionAndAlignment's
            // fLogicZ / bCenter branch), and OUTSIDE puts it just beyond the rim at 1.1. Putting
            // every pie label at the centre of the circle instead stacks them all on one another,
            // which reads as one label rather than as eight.
            if (series.LabelAt(at) is { Draws: true } label)
            {
                double middle = start - sweep / 2;
                double reach = label.Placement is ChartLabelPlacement.Outside ? 1.1 : 0.5;

                string? text = label.Compose(
                    at < plot.Categories.Count ? plot.Categories[at] : null,
                    series.Name,
                    value,
                    total);

                if (text is { Length: > 0 })
                {
                    // On a ring the fraction runs across the ring rather than from the centre,
                    // so a "centred" label lands in the band and not in the hole.
                    Length along = hole + (radius - hole) * reach;

                    labels.Add(new ChartLabel(
                        text,
                        new DocPoint(
                            centre.X + along * Math.Cos(middle),
                            centre.Y - along * Math.Sin(middle)),
                        ChartLabelAnchor.Centre,
                        plot.LabelSize,
                        AxisColour));
                }
            }

            start -= sweep;
        }
    }

    /// <summary>
    /// One wedge, as a path: a radius out, an arc, and a radius back.
    /// </summary>
    /// <remarks>
    /// The arc is split into segments of at most a quarter turn and each is the standard cubic
    /// approximation, whose control handles are <c>4/3 × tan(θ/4) × r</c> from the end points along
    /// the tangents. One cubic for a whole half-circle is visibly flat at the sides; a quarter is
    /// accurate to a thousandth of the radius, which is below anything a page can show.
    /// </remarks>
    private static GraphicsPath Wedge(DocPoint centre, Length radius, double start, double sweep)
    {
        GraphicsPath path = new();
        path.MoveTo(centre);
        path.LineTo(On(start));

        int segments = Math.Max(1, (int)Math.Ceiling(Math.Abs(sweep) / (Math.PI / 2)));
        double step = sweep / segments;
        double handle = 4.0 / 3.0 * Math.Tan(step / 4.0);

        double angle = start;

        for (int at = 0; at < segments; at++)
        {
            DocPoint from = On(angle);
            DocPoint to = On(angle + step);

            // P1 = P0 + k·r·T(θ0) and P2 = P3 − k·r·T(θ1), where T is the tangent d/dθ of the
            // parametrisation — which in a y-down space is (−sin θ, −cos θ) rather than
            // (−sin θ, cos θ). Getting that one sign wrong bends every arc the wrong way and
            // draws a pie as a pinwheel, which is what it looks like and not what it is.
            path.CubicTo(
                new DocPoint(
                    from.X - radius * (handle * Math.Sin(angle)),
                    from.Y - radius * (handle * Math.Cos(angle))),
                new DocPoint(
                    to.X + radius * (handle * Math.Sin(angle + step)),
                    to.Y + radius * (handle * Math.Cos(angle + step))),
                to);

            angle += step;
        }

        path.LineTo(centre);
        path.Close();
        return path;

        // The y term is negated because a document's y axis points down and an angle's does not.
        DocPoint On(double at)
            => new(centre.X + radius * Math.Cos(at), centre.Y - radius * Math.Sin(at));
    }

    /// <summary>
    /// One segment of a ring: out along a radius, round the outer arc, back in, and round the
    /// inner arc the other way.
    /// </summary>
    /// <remarks>
    /// A doughnut's wedge, and the only structural difference from <see cref="Wedge"/> is that the
    /// two radial edges stop at the hole rather than meeting at the centre. Drawing it as a filled
    /// wedge instead — which is what a doughnut did before <see cref="ChartPlot.Rings"/> existed —
    /// loses the hole and, on a chart of several series, draws every ring on top of the last so
    /// only the outermost is visible.
    /// </remarks>
    private static GraphicsPath Annulus(
        DocPoint centre, Length hole, Length radius, double start, double sweep)
    {
        GraphicsPath path = new();
        path.MoveTo(At(hole, start));
        path.LineTo(At(radius, start));
        Arc(radius, start, sweep);
        path.LineTo(At(hole, start + sweep));
        Arc(hole, start + sweep, -sweep);
        path.Close();
        return path;

        DocPoint At(Length r, double angle)
            => new(centre.X + r * Math.Cos(angle), centre.Y - r * Math.Sin(angle));

        void Arc(Length r, double from, double turn)
        {
            int segments = Math.Max(1, (int)Math.Ceiling(Math.Abs(turn) / (Math.PI / 2)));
            double step = turn / segments;
            double handle = 4.0 / 3.0 * Math.Tan(step / 4.0);
            double angle = from;

            for (int at = 0; at < segments; at++)
            {
                DocPoint a = At(r, angle);
                DocPoint b = At(r, angle + step);

                path.CubicTo(
                    new DocPoint(
                        a.X - r * (handle * Math.Sin(angle)),
                        a.Y - r * (handle * Math.Cos(angle))),
                    new DocPoint(
                        b.X + r * (handle * Math.Sin(angle + step)),
                        b.Y + r * (handle * Math.Cos(angle + step))),
                    b);

                angle += step;
            }
        }
    }

    /// <summary>
    /// A point in the plot area from its two fractions, whichever way round the axes are.
    /// </summary>
    private static DocPoint Point(DocRect area, double across, double up, bool columns)
        => columns
            ? new DocPoint(area.Left + area.Width * across, area.Bottom - area.Height * up)
            : new DocPoint(area.Left + area.Width * up, area.Bottom - area.Height * across);

    /// <summary>
    /// The bars.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A category slot is the plot area divided by the category count; a bar is that slot divided
    /// by <c>series + gapWidth/100 + (−overlap/100) × (series − 1)</c>, which is
    /// <c>CategoryPositionHelper::getScaledSlotWidth</c>
    /// (<c>chart2/source/view/charttypes/CategoryPositionHelper.cxx:37-45</c>) with
    /// <c>setOuterDistance(gapWidth/100)</c> and <c>setInnerDistance(−overlap/100)</c> from
    /// <c>BarChart.cxx:78-80</c>. So a clustered pair with the default gap of 100 divides its
    /// slot into three, and a stacked pair with an overlap of 100 divides it into two — of which
    /// the two series occupy the same one.
    /// </para>
    /// <para>
    /// The bar's base is the axis' zero, or the axis minimum when the whole scale is above or
    /// below zero. That is what makes a negative value draw downwards from the same line its
    /// positive neighbour draws up from, rather than upwards from the bottom of the plot.
    /// </para>
    /// </remarks>
    private static void AddBars(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        int categories,
        bool columns,
        List<ChartBox> boxes,
        List<ChartLabel> labels)
    {
        if (categories <= 0 || plot.Series.Count == 0) return;

        int series = plot.Series.Count;
        double outer = Math.Clamp(plot.GapWidth / 100.0, 0.0, 6.0);
        double inner = Math.Clamp(-plot.Overlap / 100.0, -1.0, 1.0);

        double denominator = series + outer + inner * (series - 1);
        if (!(denominator > 0.0)) return;

        double slotFraction = 1.0 / (categories * denominator);
        double baseline = Math.Clamp(scale.Fraction(0.0), 0.0, 1.0);

        // A stacked chart's series pile onto a running total per category rather than each
        // starting from the baseline, and positives and negatives pile separately so that a
        // mixed category does not cancel itself out.
        double[] positive = new double[categories];
        double[] negative = new double[categories];

        for (int index = 0; index < series; index++)
        {
            ChartSeries one = plot.Series[index];

            for (int at = 0; at < categories; at++)
            {
                if (at >= one.Values.Count) continue;
                if (one.Values[at] is not { } value || !double.IsFinite(value)) continue;

                double from;
                double to;

                if (plot.IsStacked)
                {
                    ref double running = ref (value >= 0.0 ? ref positive[at] : ref negative[at]);
                    from = scale.Fraction(running);
                    running += value;
                    to = scale.Fraction(running);
                }
                else
                {
                    from = baseline;
                    to = scale.Fraction(value);
                }

                // The slot the bar sits in, as a fraction of the plot area's long side.
                double slotStart = (double)at / categories
                    + (outer / 2.0 + index * (1.0 + inner)) * slotFraction;

                DocRect bounds = columns
                    ? Rectangle(
                        area.Left + area.Width * slotStart,
                        area.Bottom - area.Height * Math.Max(from, to),
                        area.Width * slotFraction,
                        area.Height * Math.Abs(to - from))
                    : Rectangle(
                        area.Left + area.Width * Math.Min(from, to),
                        area.Bottom - area.Height * (slotStart + slotFraction),
                        area.Width * Math.Abs(to - from),
                        area.Height * slotFraction);

                boxes.Add(new ChartBox(bounds, one.Fill, one.Line, one.LineWidth));

                if (one.LabelAt(at) is { Draws: true } label)
                    AddBarLabel(plot, one, label, at, value, bounds, to >= from, columns, labels);
            }
        }
    }

    /// <summary>
    /// One bar's data label, placed against the rectangle it names.
    /// </summary>
    /// <remarks>
    /// <c>OUTSIDE</c> is a bar chart's default (<c>typegroupconverter.cxx:95</c>) and means beyond
    /// the growing end — above a positive column and below a negative one, right of a positive bar
    /// and left of a negative one. <c>INSIDE</c> is the same end from the other side,
    /// <c>NEAR_ORIGIN</c> is the baseline end and <c>CENTER</c> is the middle, which is what a
    /// stacked chart normally uses because an outside label would sit on the series above.
    /// </remarks>
    private static void AddBarLabel(
        ChartPlot plot,
        ChartSeries series,
        ChartDataLabel label,
        int index,
        double value,
        DocRect bounds,
        bool grows,
        bool columns,
        List<ChartLabel> labels)
    {
        string? text = label.Compose(
            index < plot.Categories.Count ? plot.Categories[index] : null,
            series.Name,
            value,
            series.Total());

        if (text is not { Length: > 0 }) return;

        ChartLabelPlacement placement = label.Placement ?? ChartLabelPlacement.Outside;
        Length gap = plot.LabelSize / 5;

        DocPoint at;
        ChartLabelAnchor anchor;

        if (columns)
        {
            Length far = grows ? bounds.Top : bounds.Bottom;
            Length near = grows ? bounds.Bottom : bounds.Top;
            Length middle = bounds.Y + bounds.Height / 2;

            (at, anchor) = placement switch
            {
                ChartLabelPlacement.Centre or ChartLabelPlacement.BestFit =>
                    (new DocPoint(bounds.X + bounds.Width / 2, middle), ChartLabelAnchor.Centre),
                ChartLabelPlacement.Inside =>
                    (new DocPoint(bounds.X + bounds.Width / 2, far + gap),
                     grows ? ChartLabelAnchor.CentreTop : ChartLabelAnchor.CentreBottom),
                ChartLabelPlacement.NearOrigin =>
                    (new DocPoint(bounds.X + bounds.Width / 2, near),
                     grows ? ChartLabelAnchor.CentreBottom : ChartLabelAnchor.CentreTop),
                _ =>
                    (new DocPoint(bounds.X + bounds.Width / 2, far - (grows ? gap : -gap)),
                     grows ? ChartLabelAnchor.CentreBottom : ChartLabelAnchor.CentreTop),
            };
        }
        else
        {
            Length far = grows ? bounds.Right : bounds.Left;
            Length near = grows ? bounds.Left : bounds.Right;
            Length middle = bounds.Y + bounds.Height / 2;

            (at, anchor) = placement switch
            {
                ChartLabelPlacement.Centre or ChartLabelPlacement.BestFit =>
                    (new DocPoint(bounds.X + bounds.Width / 2, middle), ChartLabelAnchor.Centre),
                ChartLabelPlacement.Inside =>
                    (new DocPoint(far - (grows ? gap : -gap), middle),
                     grows ? ChartLabelAnchor.RightMiddle : ChartLabelAnchor.LeftMiddle),
                ChartLabelPlacement.NearOrigin =>
                    (new DocPoint(near, middle),
                     grows ? ChartLabelAnchor.LeftMiddle : ChartLabelAnchor.RightMiddle),
                _ =>
                    (new DocPoint(far + (grows ? gap : -gap), middle),
                     grows ? ChartLabelAnchor.LeftMiddle : ChartLabelAnchor.RightMiddle),
            };
        }

        labels.Add(new ChartLabel(text, at, anchor, plot.LabelSize, AxisColour));
    }

    /// <summary>The chart's title and its two axis titles.</summary>
    /// <remarks>
    /// Each is centred on what it names — the title on the whole frame, the category axis' title
    /// on the plot area's width, the value axis' on its height — which is
    /// <c>changePositionOfAxisTitle</c> (<c>ChartView.cxx:995-1046</c>) once the anchor is
    /// resolved to a rectangle. The value axis' title is rotated a quarter turn anticlockwise,
    /// which both formats state and neither leaves to the renderer.
    /// </remarks>
    private static void AddTitles(
        ChartPlot plot,
        DocRect frame,
        DocRect area,
        IChartTextMeasurer measurer,
        List<ChartLabel> labels)
    {
        if (plot.Title is { Length: > 0 } title)
        {
            // Line by line, top down, from the same origin the reservation above measured from,
            // so a two-line title fills exactly the band that was kept for it.
            Length pen = frame.Y + (frame.Height * PageMargin);
            foreach (string line in LinesOf(title))
            {
                Length height = measurer.Measure(line, plot.TitleSize).Height;
                labels.Add(new ChartLabel(
                    line,
                    new DocPoint(frame.X + frame.Width / 2, pen + height / 2),
                    ChartLabelAnchor.Centre,
                    plot.TitleSize,
                    AxisColour));
                pen += height;
            }
        }

        // Which title goes where is decided by the axis' direction rather than by its role, and
        // a bar chart turns both a quarter turn: the categories run up the left edge and the
        // values along the bottom. The room was already reserved that way — `PlotAreaOf` chooses
        // `beside` and `below` from `plot.Direction` — and drawing them the other way round put
        // each title in the other's reserved band.
        bool columns = plot.Direction == ChartBarDirection.Column;
        string? beside = columns ? plot.ValueAxisTitle : plot.CategoryAxisTitle;
        string? below = columns ? plot.CategoryAxisTitle : plot.ValueAxisTitle;

        if (below is { Length: > 0 } under)
        {
            Length height = measurer.Measure(under, plot.AxisTitleSize).Height;
            labels.Add(new ChartLabel(
                under,
                new DocPoint(
                    area.X + area.Width / 2,
                    frame.Bottom - (frame.Height * PageMargin) - height / 2),
                ChartLabelAnchor.Centre,
                plot.AxisTitleSize,
                AxisColour));
        }

        if (beside is { Length: > 0 } side)
        {
            Length height = measurer.Measure(side, plot.AxisTitleSize).Height;
            labels.Add(new ChartLabel(
                side,
                new DocPoint(
                    frame.X + (frame.Width * PageMargin) + height / 2,
                    area.Y + area.Height / 2),
                ChartLabelAnchor.Centre,
                plot.AxisTitleSize,
                AxisColour,
                Math.PI / 2));
        }
    }

    /// <summary>
    /// The legend: one key and one name per series.
    /// </summary>
    /// <remarks>
    /// The grid comes from <see cref="Legend"/>, so what is drawn and what was reserved cannot
    /// disagree: entries are dealt across each row and then down, which is the order
    /// <c>lcl_placeLegendEntries</c> fills its columns in (<c>VLegend.cxx:570-620</c>). A side
    /// legend is centred on the frame's vertical middle and set against its far edge, a top or
    /// bottom one centred horizontally — <c>lcl_getDefaultPosition</c>'s <c>LINE_END</c> anchored
    /// <c>RIGHT</c> at one legend margin from the page edge, and so on for the other three.
    /// </remarks>
    private static void AddLegend(
        ChartPlot plot,
        DocRect frame,
        DocRect area,
        IChartTextMeasurer measurer,
        List<ChartBox> boxes,
        List<ChartLabel> labels)
    {
        if (plot.Legend == ChartLegendPosition.None) return;

        List<(string Name, Colour? Fill, Colour? Line, Length Width)> named = Entries(plot);
        if (named.Count == 0) return;

        DocRect space = LegendSpace(plot, frame, measurer);
        LegendBox box = Legend(plot, space, measurer);
        if (box.Columns <= 0 || box.Rows <= 0) return;

        bool vertical = plot.Legend is ChartLegendPosition.Left or ChartLegendPosition.Right;

        // A side legend is centred on the page; a top or bottom one is set one legend margin
        // inside what the titles left — lcl_getDefaultPosition's PAGE_START measures its distance
        // from the remaining space's own top rather than from the page's (VLegend.cxx:693-716).
        Length originY = vertical
            ? frame.Y + (frame.Height - box.Height) / 2
            : plot.Legend == ChartLegendPosition.Top
                ? space.Top + LegendMarginY
                : space.Bottom - LegendMarginY - box.Height;

        Length originX = plot.Legend switch
        {
            ChartLegendPosition.Right => frame.Right - LegendMarginX - box.Width,
            ChartLegendPosition.Left => frame.X + LegendMarginX,
            _ => frame.X + (frame.Width - box.Width) / 2,
        };

        // Each column is as wide as its own widest name, so the columns are walked rather than
        // stepped by a constant.
        Length columnX = originX + box.PaddingX;

        for (int column = 0; column < box.Columns; column++)
        {
            Length widest = Length.Zero;

            for (int row = 0; row < box.Rows; row++)
            {
                int at = column + (row * box.Columns);
                if (at >= named.Count) break;

                (string name, Colour? fill, Colour? outline, Length width) = named[at];
                Length rowY = originY + box.PaddingY + (box.RowHeight * row);

                boxes.Add(new ChartBox(
                    Rectangle(
                        columnX,
                        rowY + (box.RowHeight - box.Key.Height) / 2,
                        box.Key.Width,
                        box.Key.Height),
                    fill,
                    outline,
                    width));

                labels.Add(new ChartLabel(
                    name,
                    new DocPoint(
                        columnX + box.Key.Width + box.KeyGap + (plot.LabelSize * TextShapeInsetX),
                        rowY + (box.RowHeight / 2)),
                    ChartLabelAnchor.LeftMiddle,
                    plot.LabelSize,
                    AxisColour));

                Length shape = Shape(measurer, name, plot.LabelSize).Width;
                if (shape > widest) widest = shape;
            }

            columnX += box.Key.Width + box.KeyGap + widest
                       + Larger(Millimetre, plot.LabelSize * 0.66);
        }
    }

    /// <summary>
    /// What the legend lists: the series, or a pie's categories.
    /// </summary>
    /// <remarks>
    /// <strong>A pie's legend names its categories.</strong> It plots one series, so listing the
    /// series would give a legend of one entry beside a picture of eight wedges;
    /// <c>VLegend</c> takes its entries from the plotter, and <c>PieChart</c> supplies one per
    /// point with that point's own colour (<c>VSeriesPlotter::createLegendEntries</c>'s
    /// <c>bIsPie</c> branch). Getting this wrong is worth several words of a word count and the
    /// whole legend of a picture.
    /// </remarks>
    /// <remarks>
    /// <strong>And a category with no name of its own is numbered, not skipped.</strong> A chart
    /// part need not state a <c>c:cat</c> at all — <c>barOfPieChart.xlsx</c> and
    /// <c>pieOfPieChart.xlsx</c> state a <c>c:val</c> and nothing else — and LibreOffice then
    /// generates the 1-based index as each category's name, which is
    /// <c>ExplicitCategoriesProvider</c>'s <c>lcl_getGeneratedCategories</c>. Its PDF for
    /// <c>barOfPieChart.xlsx</c> draws a nine-entry legend reading <c>1 2 … 9</c> against a
    /// spreadsheet whose own cells read <c>9 8 … 1</c>; skipping the unnamed categories instead
    /// draws no legend at all, which is nine of that file's words.
    /// </remarks>
    /// <remarks>
    /// The generation is gated on the chart stating <em>no</em> category sequence at all, not on
    /// an individual name being empty. A stated but blank category is a blank label and stays
    /// one — <c>ExplicitCategoriesProvider</c> generates the whole run or none of it — and
    /// numbering the blanks inside a stated sequence would invent labels on every sparse pie in
    /// the corpus.
    /// </remarks>
    private static List<(string Name, Colour? Fill, Colour? Line, Length Width)> Entries(
        ChartPlot plot)
    {
        List<(string, Colour?, Colour?, Length)> entries = [];

        if (plot.Kind is ChartPlotKind.Pie or ChartPlotKind.OfPie)
        {
            ChartSeries? first = plot.Series.Count > 0 ? plot.Series[0] : null;
            int points = Math.Max(plot.Categories.Count, first?.Values.Count ?? 0);

            bool generated = plot.Categories.Count == 0;

            for (int at = 0; at < points; at++)
            {
                string? stated = generated
                    ? (at + 1).ToString(System.Globalization.CultureInfo.InvariantCulture)
                    : at < plot.Categories.Count
                        ? ChartDataLabel.WriteCategory(plot.Categories[at], plot.CategoryFormat)
                        : null;

                if (stated is not { Length: > 0 }) continue;

                entries.Add((stated, first?.FillAt(at), first?.Line, first?.LineWidth ?? Length.Zero));
            }

            return entries;
        }

        foreach (ChartSeries series in plot.Series)
        {
            if (series.Name is not { Length: > 0 } name) continue;
            entries.Add((name, series.Fill, series.Line, series.LineWidth));
        }

        return entries;
    }

    /// <summary>
    /// A legend's size and the grid it arranges its entries in.
    /// </summary>
    /// <param name="Width">The whole legend's width, padding included.</param>
    /// <param name="Height">The whole legend's height, padding included.</param>
    /// <param name="Columns">How many columns the entries were dealt into.</param>
    /// <param name="Rows">How many rows.</param>
    /// <param name="Key">The symbol's extent, square unless a line key widens it.</param>
    /// <param name="KeyGap">The gap between a symbol and the name beside it.</param>
    /// <param name="PaddingX">The margin inside the legend's left and right edges.</param>
    /// <param name="PaddingY">The margin inside its top and bottom edges.</param>
    /// <param name="RowHeight">One entry's height, insets included.</param>
    private readonly record struct LegendBox(
        Length Width,
        Length Height,
        int Columns,
        int Rows,
        DocSize Key,
        Length KeyGap,
        Length PaddingX,
        Length PaddingY,
        Length RowHeight);

    /// <summary>
    /// How much room the legend takes, ported from <c>lcl_placeLegendEntries</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>chart2/source/view/main/VLegend.cxx:263-320, 507-640</c>. Every length is a fraction of
    /// the legend's own font height with a one-millimetre floor — padding <c>0.33</c>, the gap
    /// between columns <c>0.66</c>, the gap between a key and its name <c>0.22</c>, and the key
    /// itself <c>0.6</c> square — all under the comment "#i109336# Improve auto positioning in
    /// chart". The name's width is the text <em>shape's</em>, so it carries
    /// <see cref="TextShapeInsetX"/> twice like every other chart text.
    /// </para>
    /// <para>
    /// <strong>This is the largest single term in the plot rectangle, and it is not the label
    /// reservations everyone had been looking at.</strong> Measured over the 100 charts in
    /// <c>chart2/qa/extras/data/</c>'s ODF documents that state a coordinate region after a round
    /// trip through <c>soffice</c>: the mean error on the right edge was <strong>28.5 pt</strong>
    /// against 6.5 on the left and 4.3 on the top, and the right edge is where a legend goes on
    /// 88 of them. The estimate it replaces — a key of <c>0.7</c> line heights, a gap of
    /// <c>0.4</c>, and the widest name — had no padding at all and no second column.
    /// </para>
    /// <para>
    /// <strong>A line chart's key is 800 hundredths of a millimetre wide whatever the font
    /// is.</strong> <c>VSeriesPlotter::getPreferredLegendKeyAspectRatio</c>
    /// (<c>VSeriesPlotter.cxx:2538-2582</c>) returns <c>(1000, 1000)</c> — a square — for a filled
    /// series, and <c>(800, -1)</c> for one that draws a line, <c>(1600, -1)</c> when the line is
    /// dashed. A negative height means the width is absolute rather than a ratio
    /// (<c>VLegend.cxx:976-984</c>), so a line chart's key is 22.7 pt where a bar chart's is
    /// 0.6 em — 17 pt of difference on a legend, which is most of a whole reservation.
    /// </para>
    /// <para>
    /// <strong>And the entries wrap into columns.</strong> A side legend is
    /// <c>ChartLegendExpansion_HIGH</c>: it fits as many rows as the space allows and then starts
    /// another column (<c>:507-525</c>), so a chart of fourteen series against a short frame is
    /// two or three columns wide rather than one. <c>tdf146463.ods</c> is that chart, and reading
    /// it as one column put the plot rectangle's right edge 120 pt out.
    /// </para>
    /// </remarks>
    private static LegendBox Legend(ChartPlot plot, DocRect available, IChartTextMeasurer measurer)
    {
        if (plot.Legend == ChartLegendPosition.None) return default;

        List<(string Name, Colour? Fill, Colour? Line, Length Width)> named = Entries(plot);
        if (named.Count == 0) return default;

        Length font = plot.LabelSize;
        Length paddingX = Larger(Millimetre, font * 0.33);
        Length offsetX = Larger(Millimetre, font * 0.66);
        Length paddingY = Larger(Millimetre, font * 0.20);
        Length offsetY = Larger(Millimetre, font * 0.20);
        Length keyGap = Larger(Millimetre, font * 0.22);

        Length keyHeight = font * 0.6;
        Length keyWidth = Larger(keyHeight, LineKeyWidth(plot));

        Length widest = Length.Zero;
        Length tallest = Length.Zero;
        List<Length> widths = new(named.Count);

        foreach ((string name, _, _, _) in named)
        {
            DocSize shape = Shape(measurer, name, font);
            widths.Add(shape.Width);
            if (shape.Width > widest) widest = shape.Width;
            if (shape.Height > tallest) tallest = shape.Height;
        }

        Length entryWidth = offsetX + keyWidth + keyGap + widest;
        Length entryHeight = offsetY + tallest;

        int columns, rows;

        if (plot.Legend is ChartLegendPosition.Left or ChartLegendPosition.Right)
        {
            // HIGH: as many rows as fit, then a second column.
            long fit = entryHeight <= Length.Zero
                ? 0
                : (available.Height - paddingY * 2.0).Emu / entryHeight.Emu;

            columns = fit <= 0 ? 0 : (int)Math.Ceiling(named.Count / (double)fit);
            rows = columns == 0 ? 0 : (int)Math.Ceiling(named.Count / (double)columns);
        }
        else
        {
            // WIDE: as many columns as fit, then a second row.
            long fit = entryWidth <= Length.Zero
                ? 0
                : (available.Width - paddingX * 2.0).Emu / entryWidth.Emu;

            rows = fit <= 0 ? 0 : (int)Math.Ceiling(named.Count / (double)fit);
            columns = rows == 0 ? 0 : (int)Math.Ceiling(named.Count / (double)rows);
        }

        if (rows <= 0 || columns <= 0) return default;

        // Each column is as wide as its own widest entry — the entries are dealt across the row
        // and then down, so column c holds entries c, c + columns, c + 2 × columns …
        Length total = Length.Zero;
        for (int column = 0; column < columns; column++)
        {
            Length column_ = Length.Zero;
            for (int row = 0; row < rows; row++)
            {
                int at = column + (row * columns);
                if (at >= widths.Count) break;
                if (widths[at] > column_) column_ = widths[at];
            }

            total += keyWidth + keyGap + column_;
        }

        return new LegendBox(
            (paddingX * 2.0) + total + (offsetX * (columns - 1)),
            (paddingY * 2.0) + (tallest * rows) + (offsetY * (rows - 1)),
            columns,
            rows,
            new DocSize(keyWidth, keyHeight),
            keyGap,
            paddingX,
            paddingY,
            tallest);
    }

    /// <summary>
    /// The rectangle the legend is fitted into: the frame less the main title, and nothing else.
    /// </summary>
    /// <remarks>
    /// <c>lcl_createLegend</c> is handed <c>maRemainingSpace</c> as it stands after the two
    /// titles and before <c>getAvailablePosAndSizeForDiagram</c> applies the page margin
    /// (<c>ChartView.cxx:1934-1968</c>), so the legend has the whole page less the title to wrap
    /// its entries into. It decides only how many rows fit before a second column starts, so it
    /// changes nothing on a legend of a handful of entries and everything on one of fourteen.
    /// </remarks>
    private static DocRect LegendSpace(ChartPlot plot, DocRect frame, IChartTextMeasurer measurer)
    {
        Length top = frame.Y;

        if (plot.Title is { Length: > 0 } title)
        {
            top += Shape(measurer, title, plot.TitleSize).Height
                   + (frame.Height * PageMargin) + TitleGap;
        }

        return top >= frame.Bottom
            ? DocRect.Empty
            : new DocRect(frame.X, top, frame.Width, frame.Bottom - top);
    }

    /// <summary>One millimetre, the floor under every one of the legend's spacings.</summary>
    /// <remarks><c>VLegend.cxx:287-292</c>'s repeated <c>std::max( 100.0, … )</c>.</remarks>
    private static readonly Length Millimetre = Length.FromMm100(100);

    /// <summary>
    /// The flat width a legend key takes when the series it stands for draws a line.
    /// </summary>
    /// <remarks>
    /// 800 hundredths of a millimetre, or 1600 when any line is dashed — the dashed case is not
    /// distinguished here because no `ChartSeries` carries a dash pattern yet, and the undashed
    /// figure is the one the corpus exercises. See <see cref="Legend"/>.
    /// </remarks>
    private static Length LineKeyWidth(ChartPlot plot)
    {
        foreach (ChartSeries series in plot.Series)
        {
            ChartPlotKind kind = series.Kind ?? plot.Kind;
            bool draws = kind is ChartPlotKind.Line or ChartPlotKind.Scatter or ChartPlotKind.Radar;
            if (draws && series.HasLine && series.Line is not null) return Length.FromMm100(800);
        }

        return Length.Zero;
    }

    /// <summary>The larger of two lengths.</summary>
    private static Length Larger(Length one, Length other) => one > other ? one : other;

    /// <summary>The width of the widest value-axis label.</summary>
    private static Length WidestValueLabel(
        ChartScaleResult scale,
        NumberFormatCode? format,
        Length size,
        IChartTextMeasurer measurer)
    {
        Length widest = Length.Zero;
        foreach (double tick in scale.MajorTicks())
        {
            Length width = measurer.Measure(ChartDataLabel.Write(tick, format), size).Width;
            if (width > widest) widest = width;
        }

        return widest;
    }

    /// <summary>The width of the widest category label.</summary>
    private static Length WidestCategoryLabel(
        ChartPlot plot, int categories, IChartTextMeasurer measurer)
    {
        Length widest = Length.Zero;
        for (int at = 0; at < categories && at < plot.Categories.Count; at++)
        {
            if (ChartDataLabel.WriteCategory(plot.Categories[at], plot.CategoryFormat)
                is not { Length: > 0 } text)
            {
                continue;
            }

            Length width = measurer.Measure(text, plot.LabelSize).Width;
            if (width > widest) widest = width;
        }

        return widest;
    }

    /// <summary>
    /// How far the first and the last label on a horizontal category or domain axis stick out
    /// past the plot rectangle's own edges.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Only an <em>unshifted</em> axis has an overhang, and that is the whole rule.</strong>
    /// A shifted axis — a column or bar chart's — puts category <em>n</em> in the middle of the
    /// <em>n</em>th slot, so its first and last labels sit half a slot inside the rectangle and
    /// only overhang when a label is wider than its own slot. An unshifted axis, which is what a
    /// line, area, scatter, bubble or stock chart has, puts the first point <em>on</em> the left
    /// edge and the last <em>on</em> the right, and their labels are centred there — so half of
    /// each hangs outside, and <c>ShapeFactory::getRectangleOfShape</c> reports it as consumed.
    /// </para>
    /// <para>
    /// Measured over the 98 charts in <c>chart2/qa/extras/data/</c>'s ODF documents that state
    /// both rectangles: every bar chart among them reserves <em>nothing</em> to the right of its
    /// plot area, and every line and scatter chart reserves between 2.66 and 25.5 pt — which is
    /// half its last label and nothing else. Reserving the widest label rather than the last one
    /// is wrong on any axis whose longest name is in the middle.
    /// </para>
    /// </remarks>
    private static (Length First, Length Last) EndLabelOverhang(
        ChartPlot plot,
        ChartScaleResult? domain,
        int categories,
        IChartTextMeasurer measurer)
    {
        if (!plot.CategoryAxisVisible || plot.ShiftedCategories || categories <= 0)
            return (Length.Zero, Length.Zero);

        string first, last;

        if (domain is { } across)
        {
            double[] ticks = [.. across.MajorTicks()];
            if (ticks.Length == 0) return (Length.Zero, Length.Zero);
            first = ChartDataLabel.Write(ticks[0], plot.DomainFormat);
            last = ChartDataLabel.Write(ticks[^1], plot.DomainFormat);
        }
        else
        {
            int end = Math.Min(categories, plot.Categories.Count) - 1;
            if (end < 0) return (Length.Zero, Length.Zero);
            first = ChartDataLabel.WriteCategory(plot.Categories[0], plot.CategoryFormat) ?? "";
            last = ChartDataLabel.WriteCategory(plot.Categories[end], plot.CategoryFormat) ?? "";
        }

        return (measurer.Measure(first, plot.LabelSize).Width / 2,
                measurer.Measure(last, plot.LabelSize).Width / 2);
    }

    /// <summary>
    /// Constrains a plot rectangle to the aspect ratio its chart type demands, centring it in the
    /// room it was given.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>VDiagram::adjustPosAndSize_2d</c> (<c>VDiagram.cxx:101-118</c>): when the preferred
    /// aspect ratio has a positive X <em>and</em> a positive Y the rectangle is not stretched to
    /// fill, it is scaled to the smaller of the two factors and centred —
    /// <c>calculateNewSizeRespectingAspectRatio</c> plus
    /// <c>calculateTopLeftPositionToCenterObject</c>. Which types demand one is a one-line answer
    /// per plotter: <c>PieChart</c> and <c>NetChart</c> return <c>(1, 1, 1)</c>, and every other
    /// two-dimensional plotter returns <c>(-1, -1, -1)</c>, which means "arbitrary"
    /// (<c>BarChart.cxx:127</c>, <c>AreaChart.cxx:121</c>, <c>BubbleChart.cxx:134</c>,
    /// <c>CandleStickChart.cxx:60</c>).
    /// </para>
    /// <para>
    /// <strong>Missing this is the single largest error in the whole plot rectangle, and it looks
    /// like nothing.</strong> A pie drawn in a wide rectangle is still a circle, because the wedge
    /// geometry takes the smaller dimension anyway — but it is a circle in the wrong
    /// <em>place</em>, up to half the slack to the left of where the reference draws it. Over the
    /// eight pies in the ODF corpus the left edge was out by between 66 and 135 pt, more than a
    /// third of the total error over all 98 charts, and every one of them fell to under a tenth of
    /// a point.
    /// </para>
    /// </remarks>
    private static DocRect Squared(ChartPlot plot, DocRect area)
    {
        if (plot.Kind is not (ChartPlotKind.Pie or ChartPlotKind.OfPie or ChartPlotKind.Radar))
            return area;
        if (area.Width <= Length.Zero || area.Height <= Length.Zero) return area;

        Length side = area.Width < area.Height ? area.Width : area.Height;
        return new DocRect(
            area.X + (area.Width - side) / 2,
            area.Y + (area.Height - side) / 2,
            side,
            side);
    }

    /// <summary>A rectangle from its edges, never negative in either direction.</summary>
    private static DocRect Rectangle(Length x, Length y, Length width, Length height)
        => new(x, y, width < Length.Zero ? Length.Zero : width,
               height < Length.Zero ? Length.Zero : height);
}

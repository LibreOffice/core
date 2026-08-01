using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Ooxml.DrawingML;

/// <summary>How a chart label sits against the point it is placed at.</summary>
public enum ChartLabelAnchor
{
    /// <summary>Centred horizontally, its top at the point — a category label below the axis.</summary>
    CentreTop = 0,

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
public readonly record struct ChartLabel(
    string Text,
    DocPoint At,
    ChartLabelAnchor Anchor,
    Length Size,
    Colour Colour,
    double Rotation = 0.0);

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
/// A chart laid out: every mark it draws, in paint order, in the frame's coordinates.
/// </summary>
/// <param name="PlotArea">The inner plot rectangle — the axes' extent, labels excluded.</param>
/// <param name="Boxes">The filled and outlined rectangles, back to front.</param>
/// <param name="Lines">The axes, ticks and gridlines.</param>
/// <param name="Labels">The text.</param>
public sealed record ChartDrawing(
    DocRect PlotArea,
    IReadOnlyList<ChartBox> Boxes,
    IReadOnlyList<ChartLine> Lines,
    IReadOnlyList<ChartLabel> Labels);

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
public static class ChartLayout
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
        DocSize measured = measurer.Measure(text, size);
        return new DocSize(
            measured.Width + size * (TextShapeInsetX * 2),
            measured.Height + size * (TextShapeInsetY * 2));
    }

    /// <summary>Lays a chart out inside a frame.</summary>
    /// <param name="plot">The chart.</param>
    /// <param name="frame">The graphic frame's rectangle, in the caller's coordinates.</param>
    /// <param name="measurer">Measures a line of text, for reserving room.</param>
    public static ChartDrawing Place(ChartPlot plot, DocRect frame, IChartTextMeasurer measurer)
    {
        ArgumentNullException.ThrowIfNull(plot);
        ArgumentNullException.ThrowIfNull(measurer);

        List<ChartBox> boxes = [];
        List<ChartLine> lines = [];
        List<ChartLabel> labels = [];

        if (frame.Width <= Length.Zero || frame.Height <= Length.Zero)
            return new ChartDrawing(DocRect.Empty, boxes, lines, labels);

        if (plot.Background is { } background)
            boxes.Add(new ChartBox(frame, background));

        int categories = plot.CategoryCount();
        (double? dataMinimum, double? dataMaximum) = plot.ValueRange();
        ChartScaleResult scale = ChartScale.Resolve(plot.ValueScale, dataMinimum, dataMaximum);

        DocRect area = PlotAreaOf(plot, frame, scale, categories, measurer);
        if (area.Width <= Length.Zero || area.Height <= Length.Zero)
            return new ChartDrawing(DocRect.Empty, boxes, lines, labels);

        if (plot.PlotBackground is { } wall) boxes.Add(new ChartBox(area, wall));

        bool columns = plot.Direction == ChartBarDirection.Column;

        AddValueAxis(plot, area, scale, columns, lines, labels);
        AddCategoryAxis(plot, area, categories, columns, lines, labels);
        AddBars(plot, area, scale, categories, columns, boxes);
        AddTitles(plot, frame, area, measurer, labels);
        AddLegend(plot, frame, area, measurer, boxes, labels);

        return new ChartDrawing(area, boxes, lines, labels);
    }

    /// <summary>
    /// The inner plot rectangle: stated by the file when it states one, computed otherwise.
    /// </summary>
    private static DocRect PlotAreaOf(
        ChartPlot plot,
        DocRect frame,
        ChartScaleResult scale,
        int categories,
        IChartTextMeasurer measurer)
    {
        if (plot.PlotArea is { } stated) return Map(stated, plot.Space, frame);

        // The computed path, which is what every OOXML chart takes. Start from the frame less
        // the proportional page margin, then take away the title, the legend and the axes'
        // labels and titles in that order — ChartView.cxx:920-990 for the margin,
        // lcl_createTitle for the title, lcl_createLegend for the legend.
        Length marginX = frame.Width * PageMargin;
        Length marginY = frame.Height * PageMargin;

        Length left = frame.X + marginX;
        Length top = frame.Y + marginY;
        Length right = frame.Right - marginX;
        Length bottom = frame.Bottom - marginY;

        if (plot.Title is { Length: > 0 } title)
            top += Shape(measurer, title, plot.TitleSize).Height + marginY + TitleGap;

        Length legend = LegendWidth(plot, measurer);
        switch (plot.Legend)
        {
            case ChartLegendPosition.Right: right -= legend + marginX; break;
            case ChartLegendPosition.Left: left += legend + marginX; break;
            case ChartLegendPosition.Top: top += LegendHeight(plot, measurer) + marginY; break;
            case ChartLegendPosition.Bottom: bottom -= LegendHeight(plot, measurer) + marginY; break;
            default: break;
        }

        bool columns = plot.Direction == ChartBarDirection.Column;

        // The value axis' labels are as wide as the widest of them; the category axis' are one
        // line tall. Both sit a tick length plus a label spacing away from the axis.
        Length valueLabel = WidestValueLabel(scale, plot.LabelSize, measurer);
        Length labelHeight = measurer.Measure("0", plot.LabelSize).Height;
        Length categoryLabel = WidestCategoryLabel(plot, categories, measurer);

        if (columns)
        {
            left += valueLabel + TickLength + LabelSpacing;
            bottom -= labelHeight + TickLength + LabelSpacing;

            if (plot.ValueAxisTitle is { Length: > 0 } valueTitle)
                left += Shape(measurer, valueTitle, plot.AxisTitleSize).Height + ValueTitleGap;
            if (plot.CategoryAxisTitle is { Length: > 0 } categoryTitle)
                bottom -= Shape(measurer, categoryTitle, plot.AxisTitleSize).Height + CategoryTitleGap;

            // The topmost value label is centred on the axis' top, so half of it sticks out
            // above the plot area and would be clipped by the frame without this.
            top += labelHeight / 2;
        }
        else
        {
            left += categoryLabel + TickLength + LabelSpacing;
            bottom -= labelHeight + TickLength + LabelSpacing;

            if (plot.CategoryAxisTitle is { Length: > 0 } categoryTitle)
                left += Shape(measurer, categoryTitle, plot.AxisTitleSize).Height + ValueTitleGap;
            if (plot.ValueAxisTitle is { Length: > 0 } valueTitle)
                bottom -= Shape(measurer, valueTitle, plot.AxisTitleSize).Height + CategoryTitleGap;

            // The last value label is centred on the axis' right end, so half of it overhangs.
            right -= valueLabel / 2;
        }

        return right <= left || bottom <= top
            ? DocRect.Empty
            : new DocRect(left, top, right - left, bottom - top);
    }

    /// <summary>
    /// Maps a rectangle stated in the chart's own space onto the frame it is drawn in.
    /// </summary>
    /// <remarks>
    /// ODF states its geometry in the chart document's own <c>svg:width</c> by
    /// <c>svg:height</c> — 22 cm by 12 cm for the corpus deck — and the frame that displays it
    /// is whatever the containing document made it. The two are usually equal, because the
    /// container writes the object's size from the chart's; they are not equal when the frame
    /// has been resized without reopening the chart, and then everything must scale together or
    /// the plot area lands outside the frame.
    /// </remarks>
    private static DocRect Map(DocRect stated, DocSize? space, DocRect frame)
    {
        if (space is not { } size || size.Width <= Length.Zero || size.Height <= Length.Zero)
            return new DocRect(frame.X + stated.X, frame.Y + stated.Y, stated.Width, stated.Height);

        double scaleX = (double)frame.Width.Emu / size.Width.Emu;
        double scaleY = (double)frame.Height.Emu / size.Height.Emu;

        return new DocRect(
            frame.X + stated.X * scaleX,
            frame.Y + stated.Y * scaleY,
            stated.Width * scaleX,
            stated.Height * scaleY);
    }

    /// <summary>The value axis: its line, its ticks, its gridlines and its labels.</summary>
    private static void AddValueAxis(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        bool columns,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        // The axis line itself runs the full extent of the plot area on the side the value axis
        // is on: the left edge for columns, the bottom edge for bars.
        lines.Add(columns
            ? new ChartLine(
                new DocPoint(area.Left, area.Top), new DocPoint(area.Left, area.Bottom), AxisColour)
            : new ChartLine(
                new DocPoint(area.Left, area.Bottom), new DocPoint(area.Right, area.Bottom), AxisColour));

        foreach (double tick in scale.MajorTicks())
        {
            double along = scale.Fraction(tick);

            if (columns)
            {
                // A fraction of 0 is the axis minimum, which is the *bottom* of a column
                // chart's plot area — hence the subtraction rather than an addition.
                Length y = area.Bottom - area.Height * along;
                lines.Add(new ChartLine(
                    new DocPoint(area.Left - TickLength, y), new DocPoint(area.Left, y), AxisColour));
                labels.Add(new ChartLabel(
                    Format(tick),
                    new DocPoint(area.Left - TickLength - LabelSpacing, y),
                    ChartLabelAnchor.RightMiddle,
                    plot.LabelSize,
                    AxisColour));
            }
            else
            {
                Length x = area.Left + area.Width * along;
                lines.Add(new ChartLine(
                    new DocPoint(x, area.Bottom), new DocPoint(x, area.Bottom + TickLength), AxisColour));
                labels.Add(new ChartLabel(
                    Format(tick),
                    new DocPoint(x, area.Bottom + TickLength + LabelSpacing),
                    ChartLabelAnchor.CentreTop,
                    plot.LabelSize,
                    AxisColour));
            }
        }

        _ = plot;
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
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        lines.Add(columns
            ? new ChartLine(
                new DocPoint(area.Left, area.Bottom), new DocPoint(area.Right, area.Bottom), AxisColour)
            : new ChartLine(
                new DocPoint(area.Left, area.Top), new DocPoint(area.Left, area.Bottom), AxisColour));

        if (categories <= 0) return;

        for (int at = 0; at <= categories; at++)
        {
            double along = (double)at / categories;

            if (columns)
            {
                Length x = area.Left + area.Width * along;
                lines.Add(new ChartLine(
                    new DocPoint(x, area.Bottom), new DocPoint(x, area.Bottom + TickLength), AxisColour));
            }
            else
            {
                Length y = area.Bottom - area.Height * along;
                lines.Add(new ChartLine(
                    new DocPoint(area.Left - TickLength, y), new DocPoint(area.Left, y), AxisColour));
            }
        }

        for (int at = 0; at < categories; at++)
        {
            if (at >= plot.Categories.Count) continue;
            if (plot.Categories[at] is not { Length: > 0 } text) continue;

            double centre = (at + 0.5) / categories;

            labels.Add(columns
                ? new ChartLabel(
                    text,
                    new DocPoint(
                        area.Left + area.Width * centre,
                        area.Bottom + TickLength + LabelSpacing),
                    ChartLabelAnchor.CentreTop,
                    plot.LabelSize,
                    AxisColour)
                : new ChartLabel(
                    text,
                    new DocPoint(
                        area.Left - TickLength - LabelSpacing,
                        area.Bottom - area.Height * centre),
                    ChartLabelAnchor.RightMiddle,
                    plot.LabelSize,
                    AxisColour));
        }
    }

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
        List<ChartBox> boxes)
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
            }
        }
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
            Length height = measurer.Measure(title, plot.TitleSize).Height;
            labels.Add(new ChartLabel(
                title,
                new DocPoint(
                    frame.X + frame.Width / 2,
                    frame.Y + frame.Height * PageMargin + height / 2),
                ChartLabelAnchor.Centre,
                plot.TitleSize,
                AxisColour));
        }

        if (plot.CategoryAxisTitle is { Length: > 0 } category)
        {
            Length height = measurer.Measure(category, plot.AxisTitleSize).Height;
            labels.Add(new ChartLabel(
                category,
                new DocPoint(
                    area.X + area.Width / 2,
                    frame.Bottom - frame.Height * PageMargin - height / 2),
                ChartLabelAnchor.Centre,
                plot.AxisTitleSize,
                AxisColour));
        }

        if (plot.ValueAxisTitle is { Length: > 0 } value)
        {
            Length height = measurer.Measure(value, plot.AxisTitleSize).Height;
            labels.Add(new ChartLabel(
                value,
                new DocPoint(
                    frame.X + frame.Width * PageMargin + height / 2,
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
    /// Placed against the plot area's vertical centre for a side legend and its horizontal centre
    /// for a top or bottom one, which is <c>VLegend::createShapes</c>'s
    /// <c>LegendExpansion::HIGH</c> and <c>WIDE</c> reduced to the single column and single row
    /// they produce for a handful of series. A legend with more series than fit in one column is
    /// wrapped by LibreOffice into several; this draws one column and lets it run, which is
    /// wrong only for a chart with more series than the plot area is tall.
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

        List<ChartSeries> named = [];
        foreach (ChartSeries series in plot.Series)
            if (series.Name is { Length: > 0 }) named.Add(series);

        if (named.Count == 0) return;

        Length line = measurer.Measure("0", plot.LabelSize).Height;
        Length key = line * 0.7;
        Length gap = line * 0.4;

        bool vertical = plot.Legend is ChartLegendPosition.Left or ChartLegendPosition.Right;

        Length top = vertical
            ? area.Y + area.Height / 2 - line * named.Count / 2
            : plot.Legend == ChartLegendPosition.Top
                ? frame.Y + frame.Height * PageMargin
                : frame.Bottom - frame.Height * PageMargin - line;

        Length left = plot.Legend switch
        {
            ChartLegendPosition.Right => area.Right + measurer.Measure("0", plot.LabelSize).Width,
            ChartLegendPosition.Left => frame.X + frame.Width * PageMargin,
            _ => area.X + area.Width / 2 - LegendWidth(plot, measurer) / 2,
        };

        foreach (ChartSeries series in named)
        {
            boxes.Add(new ChartBox(
                Rectangle(left, top + (line - key) / 2, key, key),
                series.Fill,
                series.Line,
                series.LineWidth));

            labels.Add(new ChartLabel(
                series.Name!,
                new DocPoint(left + key + gap, top + line / 2),
                ChartLabelAnchor.LeftMiddle,
                plot.LabelSize,
                AxisColour));

            if (vertical) top += line;
            else left += key + gap + measurer.Measure(series.Name!, plot.LabelSize).Width + gap * 2;
        }
    }

    /// <summary>How wide a vertical legend is, keys and names together.</summary>
    private static Length LegendWidth(ChartPlot plot, IChartTextMeasurer measurer)
    {
        if (plot.Legend == ChartLegendPosition.None) return Length.Zero;

        Length line = measurer.Measure("0", plot.LabelSize).Height;
        Length widest = Length.Zero;
        int count = 0;

        foreach (ChartSeries series in plot.Series)
        {
            if (series.Name is not { Length: > 0 } name) continue;
            count++;
            Length width = measurer.Measure(name, plot.LabelSize).Width;
            if (width > widest) widest = width;
        }

        if (count == 0) return Length.Zero;

        bool vertical = plot.Legend is ChartLegendPosition.Left or ChartLegendPosition.Right;
        Length one = line * 0.7 + line * 0.4 + widest;
        return vertical ? one : one * count;
    }

    /// <summary>How tall a horizontal legend is.</summary>
    private static Length LegendHeight(ChartPlot plot, IChartTextMeasurer measurer)
        => LegendWidth(plot, measurer) > Length.Zero
            ? measurer.Measure("0", plot.LabelSize).Height
            : Length.Zero;

    /// <summary>The width of the widest value-axis label.</summary>
    private static Length WidestValueLabel(
        ChartScaleResult scale, Length size, IChartTextMeasurer measurer)
    {
        Length widest = Length.Zero;
        foreach (double tick in scale.MajorTicks())
        {
            Length width = measurer.Measure(Format(tick), size).Width;
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
            if (plot.Categories[at] is not { Length: > 0 } text) continue;
            Length width = measurer.Measure(text, plot.LabelSize).Width;
            if (width > widest) widest = width;
        }

        return widest;
    }

    /// <summary>
    /// A tick's label.
    /// </summary>
    /// <remarks>
    /// The axis' <c>c:numFmt</c> is deliberately not applied. Doing so needs a number formatter,
    /// and the only one Paperless has lives in <c>Paperless.Spreadsheets</c>, above this library
    /// — the same layering that keeps <see cref="DrawingChart"/> from resolving a
    /// <c>c:f</c> against its own workbook. What is drawn instead is the shortest round-trip
    /// form, which is what "General" produces for every whole-number scale in the corpus and
    /// what LibreOffice draws for one.
    /// </remarks>
    private static string Format(double value)
    {
        // A tick that is a whole number is written without a decimal point, which is what
        // "General" does and what a scale of 0, 20, 40 … needs; anything else keeps enough
        // digits to distinguish neighbouring ticks and no more.
        double rounded = Math.Round(value);
        if (Math.Abs(value - rounded) < 1e-9 && Math.Abs(value) < 1e15)
            return rounded.ToString("0", System.Globalization.CultureInfo.InvariantCulture);

        return value.ToString("G6", System.Globalization.CultureInfo.InvariantCulture);
    }

    /// <summary>A rectangle from its edges, never negative in either direction.</summary>
    private static DocRect Rectangle(Length x, Length y, Length width, Length height)
        => new(x, y, width < Length.Zero ? Length.Zero : width,
               height < Length.Zero ? Length.Zero : height);
}

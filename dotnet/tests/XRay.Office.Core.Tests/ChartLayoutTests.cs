using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// What each plot type composes, and the two decisions that are invisible in any one mark.
/// </summary>
/// <remarks>
/// <para>
/// The engine's output is a list of rectangles, lines, paths and labels, so the useful assertions
/// are counts and extents rather than pixels: how many wedges a pie has, whether a line chart
/// reaches both edges of its plot area, whether a chart with no axes drew any tick labels. Those
/// are exactly the properties a whole-page comparison reports as "a bit different" and cannot
/// name.
/// </para>
/// <para>
/// Text is measured by a stand-in rather than by a real face, because none of what is asserted
/// here depends on a particular font and a test that loaded one would fail on a machine without
/// it. The stand-in's line height is 1.15 em, which is Liberation Sans' to three decimal places.
/// </para>
/// </remarks>
public class ChartLayoutTests
{
    /// <summary>A measurer with no fonts: half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartPlot Bars() => new()
    {
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series = [new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))],
    };

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    [Fact]
    public void AGridlineIsDrawnAcrossThePlotAreaAtEveryMajorTick()
    {
        ChartDrawing without = Place(Bars());
        ChartDrawing with = Place(Bars() with { ValueGrid = new ChartGrid(Colour.FromRgb(0xB3B3B3)) });

        // Ten ticks on the corpus scale, so ten more lines and no other change.
        with.Lines.Count.ShouldBe(without.Lines.Count + 10);

        List<ChartLine> grid =
            [.. with.Lines.Where(line => line.Colour == Colour.FromRgb(0xB3B3B3))];

        grid.Count.ShouldBe(10);

        // Each spans the plot area's full width, which is what distinguishes a gridline from the
        // tick mark at the same height — the tick runs 4.25 pt *outside* the axis.
        foreach (ChartLine line in grid)
        {
            line.From.X.ShouldBe(with.PlotArea.Left);
            line.To.X.ShouldBe(with.PlotArea.Right);
            line.From.Y.ShouldBe(line.To.Y);
        }
    }

    [Fact]
    public void APieDrawsAWedgePerPointAndNoAxisAtAll()
    {
        ChartPlot pie = Bars() with
        {
            Kind = ChartPlotKind.Pie,
            Legend = ChartLegendPosition.Right,
        };

        ChartDrawing drawing = Place(pie);

        drawing.Shapes.Count.ShouldBe(4);

        // No axis line, no tick, no gridline: a pie has neither axis, and the first version of the
        // reader drew both — 82 words of invented labels on a chart the reference gives one.
        drawing.Lines.ShouldBeEmpty();

        // And the labels it does draw are the legend's, which for a pie names the categories
        // rather than the single series.
        List<string> text = [.. drawing.Labels.Select(label => label.Text)];
        text.ShouldBe(["Q1", "Q2", "Q3", "Q4"], ignoreOrder: true);
    }

    [Fact]
    public void APiesWedgesStartAtTwelveOClockAndRunClockwise()
    {
        ChartPlot pie = new()
        {
            Kind = ChartPlotKind.Pie,
            Categories = ["A", "B", "C", "D"],
            Series = [new ChartSeries("s", [1.0, 1.0, 1.0, 1.0], Colour.Black)],
        };

        ChartDrawing drawing = Place(pie);
        drawing.Shapes.Count.ShouldBe(4);

        DocPoint centre = new(
            drawing.PlotArea.X + drawing.PlotArea.Width / 2,
            drawing.PlotArea.Y + drawing.PlotArea.Height / 2);

        // Every wedge starts at the centre and its first straight segment is the radius it opens
        // on. Four equal quarters open at 12, 3, 6 and 9 o'clock in that order.
        List<DocPoint> opens =
            [.. drawing.Shapes.Select(shape => shape.Path.Commands[1].Point)];

        Near(opens[0].X, centre.X);
        opens[0].Y.ShouldBeLessThan(centre.Y);

        opens[1].X.ShouldBeGreaterThan(centre.X);
        Near(opens[1].Y, centre.Y);

        Near(opens[2].X, centre.X);
        opens[2].Y.ShouldBeGreaterThan(centre.Y);

        static void Near(Length actual, Length expected)
            => Math.Abs(actual.Emu - expected.Emu).ShouldBeLessThan(Length.FromPoints(0.01).Emu);
    }

    [Fact]
    public void ALineChartTouchesBothEdgesWhereABarChartNeverDoes()
    {
        ChartDrawing line = Place(Bars() with { Kind = ChartPlotKind.Line });

        line.Shapes.Count.ShouldBe(1);

        List<DocPoint> points = [.. line.Shapes[0].Path.Commands.Select(command => command.Point)];
        points.Count.ShouldBe(4);

        // ShiftedCategoryPosition is false for a line chart, so the first point sits on the plot
        // area's left edge and the last on its right. A bar chart's leftmost bar starts a fraction
        // of a slot in, which is the whole difference between the two axes.
        points[0].X.ShouldBe(line.PlotArea.Left);
        points[^1].X.ShouldBe(line.PlotArea.Right);

        ChartDrawing bars = Place(Bars());
        List<ChartShape> columns = bars.Filled();
        columns.Count.ShouldBe(4);
        columns[0].Bounds().Left.ShouldBeGreaterThan(bars.PlotArea.Left);
    }

    [Fact]
    public void ALineIsBrokenAtAGapRatherThanBridgedAcrossIt()
    {
        ChartPlot gapped = Bars() with
        {
            Kind = ChartPlotKind.Line,
            Series = [new ChartSeries("North", [120.0, null, 143.0, 168.0], Colour.Black)],
        };

        GraphicsPath path = Place(gapped).Shapes[0].Path;

        // Two subpaths, so two MoveTo: bridging the hole would give one MoveTo and three LineTo,
        // and would draw a straight segment no reader could tell from a real value.
        path.Commands.Count(command => command.Verb == PathVerb.MoveTo).ShouldBe(2);
        path.Commands.Count(command => command.Verb == PathVerb.LineTo).ShouldBe(1);
    }

    [Fact]
    public void AnAreaIsAClosedRegionBetweenItsPointsAndTheBaseline()
    {
        ChartDrawing drawing = Place(Bars() with { Kind = ChartPlotKind.Area });

        drawing.Shapes.Count.ShouldBe(1);

        GraphicsPath path = drawing.Shapes[0].Path;
        path.Commands[^1].Verb.ShouldBe(PathVerb.Close);
        drawing.Shapes[0].Fill.ShouldNotBeNull();

        // Four points along the top and four back along the baseline.
        path.Commands.Count(command => command.Verb is PathVerb.MoveTo or PathVerb.LineTo)
            .ShouldBe(8);
    }

    [Fact]
    public void ASmallChartGetsFewerTicksThanALargeOneWithTheSameNumbers()
    {
        // The second pass. Both charts hold the same 88..168; the large one has room for ten
        // intervals and lands on 0..180 in steps of 20, and the small one has room for four and
        // is forced up the 1-2-5 ladder to steps of 50.
        ChartPlot plot = Bars();

        int large = ChartLayout
            .Place(plot, Frame, new Ruler())
            .Labels.Count(label => label.Text is "20");

        ChartDrawing small = ChartLayout.Place(
            plot,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(220), Length.FromPoints(120)),
            new Ruler());

        large.ShouldBe(1);
        small.Labels.Select(label => label.Text).ShouldNotContain("20");
        small.Labels.Select(label => label.Text).ShouldContain("50");
    }

    [Fact]
    public void AChartWithASpaceOfItsOwnIsStretchedRatherThanRecomposed()
    {
        // An OLE chart is rendered at its own stated size and scaled into the frame that shows it,
        // which is what makes chart-bar-sheet.ods draw the same ten ticks its .odp twin does even
        // though its frame is two thirds the size.
        ChartPlot plot = Bars() with
        {
            Space = new DocSize(Length.FromPoints(400), Length.FromPoints(300)),
            PlotArea = new DocRect(
                Length.FromPoints(50), Length.FromPoints(20),
                Length.FromPoints(300), Length.FromPoints(200)),
        };

        ChartDrawing half = ChartLayout.Place(
            plot,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(200), Length.FromPoints(150)),
            new Ruler());

        half.PlotArea.X.ShouldBe(Length.FromPoints(25));
        half.PlotArea.Y.ShouldBe(Length.FromPoints(10));
        half.PlotArea.Width.ShouldBe(Length.FromPoints(150));
        half.PlotArea.Height.ShouldBe(Length.FromPoints(100));

        // The type is stretched with everything else, so a 10 pt label is drawn at 5 pt.
        half.Labels.ShouldAllBe(label => label.Size <= Length.FromPoints(7));
    }

    /// <summary>A pie whose data labels are single unbreakable tokens of a stated width.</summary>
    /// <remarks>
    /// One token and no spaces on purpose: <c>LinesOf</c> leaves a word wider than the whole
    /// allowance whole rather than breaking it, so the label's block width is the same at both
    /// candidate first-pass radii and the *only* thing that decides whether it fits inside its
    /// slice is the radius. A label with spaces re-wraps as the radius changes and hides exactly
    /// the difference these tests are about.
    /// </remarks>
    private static ChartPlot TokenLabelPie(int glyphs) => new()
    {
        Kind = ChartPlotKind.Pie,
        Categories = ["A", "B", "C", "D"],
        Series =
        [
            new ChartSeries("s", [1.0, 1.0, 1.0, 1.0], Colour.Black)
            {
                Label = new ChartDataLabel
                {
                    Text = new string('A', glyphs),
                    Placement = ChartLabelPlacement.BestFit,
                },
            },
        ],
    };

    /// <summary>
    /// A pie's first pass is drawn at a fraction of the diagram, not at the whole of it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>VDiagram::reduceToMinimumSize</c> shrinks the diagram to <c>round(side / 2.2)</c> before
    /// any series exists, and the axis-label pass that would normally grow it straight back is
    /// guarded by <c>!bIsPieOrDonut</c> — so on a pie the best-fit labels of pass 1 are laid out
    /// around a radius of 63.7 pt on this 400x300 frame, not 140.1.
    /// </para>
    /// <para>
    /// <strong>This is a discriminator and not a golden number.</strong> A twenty-glyph token is
    /// 100 pt wide under the <see cref="Ruler"/>: its diagonal clears
    /// <c>0.975 x 140.1 = 136.6</c> and so would fit inside its own quarter slice at the full
    /// radius, and does not clear <c>0.975 x 63.7 = 62.1</c> at the reduced one. Modelling pass 1
    /// at full size therefore consumes exactly the diagram and shrinks nothing; modelling it at
    /// the reduced size pushes all four labels outside and the pie comes out materially smaller.
    /// The four-glyph control is the same chart with a label that fits at either radius, and it
    /// must not shrink at all.
    /// </para>
    /// <para>
    /// On the corpus this is worth the whole of round 60's open item:
    /// <c>003_advanced_excel_pie</c>'s pie moves from centre (382.80, 467.68) radius 104.70 to
    /// (408.81, 464.81) radius 100.01, against the reference's (408.84, 464.74) and 99.78 —
    /// 26.04 pt of centre error down to 0.03, on all four corpus pies at once.
    /// </para>
    /// </remarks>
    [Fact]
    public void APiesFirstPassIsLaidOutAtOneTwoPointTwothOfTheDiagram()
    {
        ChartDrawing control = Place(TokenLabelPie(4));
        ChartDrawing shrunk = Place(TokenLabelPie(20));

        // The control's labels fit inside their slices at either radius, so nothing is consumed
        // outside the diagram and the pie keeps the whole square.
        control.PlotArea.Height.ShouldBe(control.DiagramArea.Height);

        // The twenty-glyph one does not fit at the reduced radius. Under a full-sized first pass
        // it would fit, and this assertion would read `ShouldBe` rather than `ShouldBeLessThan`.
        shrunk.PlotArea.Height.ShouldBeLessThan(control.PlotArea.Height);

        // And it is a real shrink rather than a rounding: better than a tenth off the square.
        (shrunk.PlotArea.Height.Points / control.PlotArea.Height.Points).ShouldBeLessThan(0.9);

        // A pie stays square through the shrink — `calculateNewSizeRespectingAspectRatio` takes
        // the smaller factor and `Squared` re-centres what is left.
        shrunk.PlotArea.Width.ShouldBe(shrunk.PlotArea.Height);
    }

    /// <summary>The reduced first pass is centred on the diagram, not offset with it.</summary>
    /// <remarks>
    /// <c>reduceToMinimumSize</c> puts its rectangle at <c>(x + w, y + h)</c> — down and to the
    /// right of the diagram's own corner, and *not* centred — and then
    /// <c>adjustPosAndSize</c> squares it, which re-centres only the longer axis. The observable
    /// consequence is that a pie whose labels all overflow comes out shifted; on the corpus that
    /// shift is what takes <c>003</c>'s centre x from 382.80 to 408.81 against 408.84. Here it is
    /// enough to pin that the shrunk pie has moved and is still inside the diagram.
    /// </remarks>
    [Fact]
    public void AShrunkPieStaysInsideTheDiagramItWasReducedFrom()
    {
        ChartDrawing shrunk = Place(TokenLabelPie(20));

        shrunk.PlotArea.Left.ShouldBeGreaterThanOrEqualTo(shrunk.DiagramArea.Left);
        shrunk.PlotArea.Top.ShouldBeGreaterThanOrEqualTo(shrunk.DiagramArea.Top);
        shrunk.PlotArea.Right.ShouldBeLessThanOrEqualTo(shrunk.DiagramArea.Right);
        shrunk.PlotArea.Bottom.ShouldBeLessThanOrEqualTo(shrunk.DiagramArea.Bottom);
    }

    /// <summary>
    /// A main title's first line starts a flat 135 plus its own text-shape inset below the two
    /// per cent.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>lcl_createTitle</c> (<c>ChartView.cxx:1058-1069</c>) puts a <c>MAIN_TITLE</c> shape's
    /// top at <c>rRemainingSpace.Y + int(pageHeight x 0.02) + 135</c> hundredths of a millimetre,
    /// and <c>ShapeFactory::createText</c> (<c>ShapeFactory.cxx:2283-2286</c>) then insets the
    /// text inside the shape by <c>round(fontHeight_mm100 x 0.30)</c>.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 rather than argued: <c>probes/sheets-r61/probe-titlepos.py</c> renders
    /// eighteen one-variable rewrites of <c>003_advanced_excel_pie</c>'s chart part — nine sizes
    /// from 6 to 36 pt, bold and regular — and <c>y_ours - y_ref</c> tracked
    /// <c>(135 + round(0.30 x size)) / 100 mm</c> across the whole range with no free parameter.
    /// Both terms were already in <c>DiagramAreaOf</c>'s reservation and neither was in the pen,
    /// which is why the title sat 9.57 pt high on every chart in the corpus.
    /// </para>
    /// <para>
    /// <strong>A drift guard, not a law test</strong> — it restates the arithmetic. What gives it
    /// teeth is that dropping either term fails it, which is what
    /// <c>verify-test.sh</c> was pointed at.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(6.0)]
    [InlineData(10.0)]
    [InlineData(13.0)]
    [InlineData(18.0)]
    [InlineData(36.0)]
    public void AMainTitlesFirstLineClearsTheFlatGapAndItsOwnUpperInset(double points)
    {
        Length size = Length.FromPoints(points);
        ChartDrawing drawing = Place(Bars() with { Title = "Title", TitleSize = size });

        ChartLabel title = drawing.Labels.Single(label => label.Text == "Title");

        Length expected =
            Frame.Y
            + (Frame.Height * 0.02)
            + Length.FromMm100(135)
            + Length.FromMm100((long)Math.Round(size.Mm100 * 0.30, MidpointRounding.AwayFromZero))
            + (size * 1.15 / 2);

        title.At.Y.ShouldBe(expected);
    }

    /// <summary>The title is drawn inside the band the layout kept for it.</summary>
    /// <remarks>
    /// The reservation in <c>DiagramAreaOf</c> has carried the flat 135 and the 0.30 inset since
    /// the layout was written; the pen carried neither until round 61, so the two disagreed by
    /// exactly those terms and the title was drawn above the band that was reserved. This asserts
    /// the property that failure violated, at four sizes, and it is the reason the fix could not
    /// be a fitted constant: the constant is already in the tree, twice, and was simply not being
    /// applied on the drawing path.
    /// </remarks>
    [Theory]
    [InlineData(6.0)]
    [InlineData(13.0)]
    [InlineData(18.0)]
    [InlineData(36.0)]
    public void AMainTitleIsDrawnInsideTheBandTheDiagramReservedForIt(double points)
    {
        Length size = Length.FromPoints(points);
        ChartDrawing drawing = Place(Bars() with { Title = "Title", TitleSize = size });

        ChartLabel title = drawing.Labels.Single(label => label.Text == "Title");

        Length top = title.At.Y - (size * 1.15 / 2);
        Length bottom = title.At.Y + (size * 1.15 / 2);

        top.ShouldBeGreaterThan(Frame.Y + (Frame.Height * 0.02));
        bottom.ShouldBeLessThanOrEqualTo(drawing.DiagramArea.Top);
    }
}

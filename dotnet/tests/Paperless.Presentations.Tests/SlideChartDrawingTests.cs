using Paperless.Core;
using Paperless.Core.Charts;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks what a chart on a slide draws, against numbers read out of LibreOffice's own PDF.
/// </summary>
/// <remarks>
/// <para>
/// The corpus deck is <c>chart-bar-deck.{odp,fodp,pptx}</c> — one hand-written 22 × 12 cm
/// clustered column chart and LibreOffice's two conversions of it — holding two series of four
/// values each over the categories Q1 to Q4, with a title, both axis titles and a legend on the
/// right. LibreOffice's PDF for it draws, in this order: a white chart-area rectangle, a grey
/// wall from 106.526 to 607.493 pt across and 116.815 to 358.809 up, five category ticks, ten
/// value ticks labelled 0 to 180, the two axis lines, eight bars each followed by its own
/// outline, and the labels.
/// </para>
/// <para>
/// <strong>The expectations differ by family and the difference is the finding.</strong> An ODF
/// chart states its inner plot rectangle in <c>chart:coordinate-region</c>, so the ODP assertions
/// are exact — every bar within 0.06 pt of the reference. An OOXML chart states nothing unless
/// the author dragged the plot area, so the PPTX goes through the composition heuristic and its
/// tolerance is 2 pt on a 623 pt frame. Both draw the same bars in the same proportions, because
/// the arithmetic <em>inside</em> the plot area is shared and is exact in both.
/// </para>
/// </remarks>
public class SlideChartDrawingTests
{
    /// <summary>
    /// A twentieth of a point, for anything an ODF chart states outright.
    /// </summary>
    /// <remarks>
    /// <c>chart:coordinate-region</c> is written to three decimal places of a centimetre, so a
    /// stated 2.258 cm is 0.03 mm — 0.09 pt — of quantisation before anything is computed. This
    /// is loose enough to absorb that and nothing else.
    /// </remarks>
    private const double StatedTolerance = 0.1;

    /// <summary>
    /// Two points, for a plot rectangle the composition heuristic had to compute.
    /// </summary>
    /// <remarks>
    /// The residual after the port: measured on <c>chart-bar-deck.pptx</c> against LibreOffice's
    /// own PDF, the plot area comes out 1.29 pt left of, 0.75 pt below, 0.50 pt narrower and
    /// 0.76 pt taller than the reference's. That is 0.2% of the frame's width, and it is
    /// LibreOffice's second layout pass — the one that re-lays the diagram out around the labels
    /// it has just measured — which this does not do.
    /// </remarks>
    private const double ComputedTolerance = 2.0;

    private static LaidOutSlide Slide(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides[0];
    }

    /// <summary>The bars: the shapes filled in the series' colour, in drawing order.</summary>
    private static List<DocRect> Bars(LaidOutSlide slide)
        => [.. slide.Shapes
            .Where(shape => shape.Fill == Paint.Solid(Colour.FromRgb(0x99CCFF)))
            .Select(Bounds)
            .Where(bounds => bounds.Width.Points > 20)];

    private static DocRect Bounds(PlacedShape shape)
    {
        double left = double.MaxValue, top = double.MaxValue;
        double right = double.MinValue, bottom = double.MinValue;

        foreach (PathCommand command in shape.Outline.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;
            left = Math.Min(left, command.Point.X.Points);
            top = Math.Min(top, command.Point.Y.Points);
            right = Math.Max(right, command.Point.X.Points);
            bottom = Math.Max(bottom, command.Point.Y.Points);
        }

        return new DocRect(
            Core.Units.Length.FromPoints(left),
            Core.Units.Length.FromPoints(top),
            Core.Units.Length.FromPoints(right - left),
            Core.Units.Length.FromPoints(bottom - top));
    }

    [Fact]
    public void AnOdfChartsPlotAreaIsTheRectangleTheFileStates()
    {
        // chart-bar-deck.odp states chart:coordinate-region svg:x="2.258cm" svg:y="1.594cm"
        // svg:width="17.674cm" svg:height="8.538cm", and LibreOffice's own PDF draws the wall at
        // 2258, 1594, 17672, 8537 in hundredths of a millimetre — the same rectangle to within
        // the rounding of a centimetre-formatted attribute. So no heuristic is involved.
        LaidOutSlide slide = Slide("chart-bar-deck.odp");

        DocRect wall = slide.Shapes
            .Where(shape => shape.Fill == Paint.Solid(Colour.FromRgb(0xD9D9D9)))
            .Select(Bounds)
            .Single();

        // In PDF's own coordinates the reference draws 106.526 to 607.493 across; the slide's
        // origin is its top-left, so the vertical numbers are the page height less those.
        wall.X.Points.ShouldBe(106.526, StatedTolerance);
        wall.Right.Points.ShouldBe(607.493, StatedTolerance);
        wall.Width.Points.ShouldBe(500.967, StatedTolerance);
        wall.Height.Points.ShouldBe(241.994, StatedTolerance);
    }

    [Fact]
    public void AnOdfChartsBarsLandWhereLibreOfficeDrawsThem()
    {
        // Every bar of chart-bar-deck.odp, quoted from LibreOffice's PDF: the four of the first
        // series then the four of the second, each 41.75 pt wide, all rising from 116.84.
        (double Left, double Height)[] reference =
        [
            (127.39, 161.32), (252.62, 127.73), (377.86, 192.25), (503.09, 225.86),
            (169.14, 118.32), (294.38, 177.48), (419.61, 135.78), (544.85, 162.68),
        ];

        List<DocRect> bars = Bars(Slide("chart-bar-deck.odp"));
        bars.Count.ShouldBe(8);

        for (int at = 0; at < reference.Length; at++)
        {
            bars[at].X.Points.ShouldBe(reference[at].Left, StatedTolerance);
            bars[at].Width.Points.ShouldBe(41.75, StatedTolerance);
            bars[at].Height.Points.ShouldBe(reference[at].Height, StatedTolerance);
        }
    }

    [Fact]
    public void ABarsWidthIsItsSlotDividedByTheSeriesCountPlusTheGap()
    {
        // A category slot is the plot area's width over four categories, 125.242 pt, and two
        // clustered series with c:gapWidth val="100" divide it into three —
        // CategoryPositionHelper::getScaledSlotWidth with setOuterDistance(1.0). The first bar
        // starts half a gap into the slot. Both come straight out of the reference's numbers:
        // 41.75 wide, starting 20.86 into a slot that starts at 106.526.
        List<DocRect> bars = Bars(Slide("chart-bar-deck.odp"));

        bars[0].Width.Points.ShouldBe(125.242 / 3.0, StatedTolerance);
        (bars[0].X.Points - 106.526).ShouldBe(125.242 / 3.0 / 2.0, StatedTolerance);

        // And the second series' first bar butts straight onto the first's, because c:overlap
        // is zero: no gap within a category, one bar's width of gap between categories.
        bars[4].X.Points.ShouldBe(bars[0].X.Points + bars[0].Width.Points, StatedTolerance);
    }

    [Fact]
    public void ABarsHeightIsItsValueOverTheAxisRange()
    {
        // The check that catches a wrong axis scale, which is the failure that looks reasonable.
        // The plot is 241.994 pt for 180 units, so 120 is 161.33 pt and 168 is 225.86 — and an
        // axis that ran to 200 instead would draw them at 145.2 and 203.3.
        List<DocRect> bars = Bars(Slide("chart-bar-deck.odp"));

        double perUnit = bars[3].Height.Points / 168.0;
        bars[0].Height.Points.ShouldBe(120 * perUnit, 0.02);
        bars[1].Height.Points.ShouldBe(95 * perUnit, 0.02);
        bars[2].Height.Points.ShouldBe(143 * perUnit, 0.02);

        (241.994 / 180.0).ShouldBe(perUnit, 0.01);
    }

    [Fact]
    public void AnOoxmlChartIsComposedAndLandsWithinTwoPointsOfTheReference()
    {
        // The same chart as .pptx, whose c:plotArea has no c:layout at all — so this is the
        // composition heuristic's output rather than the file's. The residual is recorded on
        // ComputedTolerance and is entirely in the plot rectangle: the bars inside it are in the
        // same proportions as the ODF pair's, to a fortieth of a point.
        List<DocRect> bars = Bars(Slide("chart-bar-deck.pptx"));
        bars.Count.ShouldBe(8);

        bars[0].X.Points.ShouldBe(127.39, ComputedTolerance);
        bars[0].Width.Points.ShouldBe(41.75, ComputedTolerance);
        bars[0].Height.Points.ShouldBe(161.32, ComputedTolerance);
        bars[3].Height.Points.ShouldBe(225.86, ComputedTolerance);

        double perUnit = bars[3].Height.Points / 168.0;
        bars[0].Height.Points.ShouldBe(120 * perUnit, 0.025);
    }

    [Fact]
    public void AnUnstyledOdfSeriesIsDrawnInTheImportersOwnDefaultBlue()
    {
        // chart-bar-deck.odp's series style ch9 states a chart-properties and a text-properties
        // and no graphic-properties whatever, so there is no fill in the file at all. ODF's
        // chart import defaults one — 0x0099ccff, "blue 8"
        // (xmloff/source/chart/ColorPropertySet.cxx:81) — and LibreOffice's PDF paints ten
        // rectangles in 0.6 0.8 1, which is that colour: eight bars and two legend keys.
        // Without the default the chart draws its axes and no bars, which reads as a data bug.
        Slide("chart-bar-deck.odp").Shapes
            .Count(shape => shape.Fill == Paint.Solid(Colour.FromRgb(0x99CCFF)))
            .ShouldBe(10);
    }

    [Fact]
    public void TheAxesTheirTicksAndTheirLabelsAreAllDrawn()
    {
        LaidOutSlide slide = Slide("chart-bar-deck.odp");

        // Two axis lines, five category ticks and ten value ticks: seventeen strokes, which is
        // exactly what LibreOffice's PDF writes for this chart.
        slide.Shapes.Count(shape => shape.Line is not null && shape.Fill is null).ShouldBe(17);

        // And the ticks' labels are real glyph runs rather than nothing, so a chart's numbers
        // survive pdftotext — which is what makes the whole-file word-count check meaningful.
        string text = string.Join(
            ' ',
            slide.Shapes
                .Where(shape => shape.Text is not null)
                .SelectMany(shape => shape.Text!.Runs)
                .Select(run => run.Run.Text));

        text.ShouldContain("180");
        text.ShouldContain("Q1");
        text.ShouldContain("Regional revenue");
        text.ShouldContain("Quarter");
        text.ShouldContain("North");
    }

    [Fact]
    public void AFlatOdfChartDrawsTheChartRatherThanItsMarkup()
    {
        // chart-bar-deck.fodp inlines the whole chart sub-document inside the draw:object, and
        // before charts were read a descendant search for text:p turned it into eighteen words
        // stacked as paragraphs. It now draws the same eight bars its packaged twin does.
        Bars(Slide("chart-bar-deck.fodp")).Count.ShouldBe(8);
    }

    /// <summary>The value axis' title is turned anticlockwise, so it reads bottom to top.</summary>
    /// <remarks>
    /// <c>ChartLabel.Rotation</c> is anticlockwise and slide coordinates grow downwards, so the
    /// transform that carries it must negate. Handing the angle straight to
    /// <c>AffineTransform.Rotation</c> turns every rotated piece of chart text the other way —
    /// measured on a probe whose axis title reads <c>Alpha Omega</c>, which came out top to
    /// bottom against LibreOffice's bottom to top, and on <c>Demick_JetBlue.pptx</c>, whose 45
    /// degree category labels descended to the right against the reference's ascending.
    /// </remarks>
    [Fact]
    public void TheValueAxisTitleIsTurnedAnticlockwise()
    {
        LaidOutSlide slide = Slide("features/chart-bar-deck.pptx");

        List<AffineTransform> turned =
            [.. slide.Shapes
                .Select(shape => shape.Text)
                .Where(text => text is { IsUpright: false })
                .Select(text => text!.Transform)
                // A non-identity transform is not necessarily a turned one: an autofit chart
                // label carries a horizontal stretch, whose B is exactly zero.
                .Where(transform => Math.Abs(transform.B) > 0.001)];

        turned.ShouldNotBeEmpty();

        // The text's own advance direction is +x. Under a quarter turn anticlockwise in a
        // y-down space it must come out pointing at -y, which is up the page.
        foreach (AffineTransform transform in turned)
        {
            (transform.A * transform.A + transform.B * transform.B).ShouldBe(1.0, 0.001);
            transform.B.ShouldBeLessThan(0.0);
        }
    }
}

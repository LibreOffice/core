using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A chart's text on a slide takes its <em>vertical</em> metrics from <c>chart2</c>'s own 96 dpi
/// device, and not from the 600 dpi one Impress lays every other text out against.
/// </summary>
/// <remarks>
/// <para>
/// The counterpart of <see cref="SlideChartDeviceAdvanceTests"/> and the other half of one rule.
/// <c>chart2</c>'s view builds a chart's labels as plain text shapes on the
/// <c>VirtualDevice</c> that <c>DrawModelWrapper</c> creates from
/// <c>Application::GetDefaultDevice()</c>
/// (<c>chart2/source/view/main/DrawModelWrapper.cxx</c>:88-99), which asks for no
/// <c>RefDevMode</c> at all and so keeps the platform default of <strong>96 dpi</strong>
/// (<c>SvpSalGraphics::GetResolution</c>, <c>vcl/headless/svpgdi.cxx</c>:44) where
/// <c>SdModule</c> asks for 600. A device pixel is <b>0.75 pt</b> against Impress's 0.24, and an
/// <c>OutputDevice</c> instantiates a font at a whole number of them — so ascent, descent, the
/// line height and the baseline are all derived at the rounded size.
/// </para>
/// <para>
/// <strong>What carries it is <see cref="SlideTextBody.Device"/>.</strong> Both
/// <c>SlideTextLayout.Height</c>, which reserves the label's room, and
/// <c>SlideTextLayout.Place</c>, which puts its baselines down, read the device off the body, so
/// the two cannot come apart — which is the failure round 60 recorded on the sheets track, where
/// a height and an ascent that moved separately cancelled on a single-line label and were wrong
/// everywhere else.
/// </para>
/// <para>
/// The expected values are computed from the faces' own <c>hhea</c> numbers, quoted as constants,
/// rather than read back out of the code under test. Measured on both reference binaries in
/// <c>probes/chart-vertical/</c>: 144 of 144 baseline-to-baseline distances inside 0.019 pt of
/// this rule, against as much as 1.208 pt for exact scaling.
/// </para>
/// </remarks>
public class SlideChartDeviceMetricTests
{
    private const int UnitsPerEm = 2048;

    /// <summary>Liberation Sans, whose <c>hhea</c> line gap is 67/2048 and is not zero.</summary>
    private const int SansAscent = 1854;
    private const int SansDescent = 434;
    private const int SansLineGap = 67;

    /// <summary>Liberation Serif, which <c>chart-face-stated.pptx</c> names outright.</summary>
    private const int SerifAscent = 1825;
    private const int SerifDescent = 443;

    /// <summary>
    /// Sizes that separate the rule from any fixed fraction of the em.
    /// </summary>
    /// <remarks>
    /// 10 pt is 13 whole pixels for 13.333 and stacks <em>shorter</em> than the design metric;
    /// 11 pt is 15 for 14.667 and stacks <em>taller</em>; 12 pt is already 16 and the two answers
    /// meet. A correction with one sign fails one of the three.
    /// </remarks>
    public static TheoryData<double, int> Sizes => new() { { 10.0, 13 }, { 11.0, 15 }, { 12.0, 16 } };

    [Theory]
    [MemberData(nameof(Sizes))]
    public void TwoLinesOfAChartLabelAreStackedByTheNinetySixDpiDevice(double points, int pixels)
    {
        Length size = Length.FromPoints(points);
        PixelEm(size).ShouldBe(pixels, $"{points} pt is {pixels} whole pixels at 96 dpi");

        List<PlacedGlyphRun> runs = Place(size, MetricGrid.Chart);
        runs.Count.ShouldBe(2, "one run per line");

        Length pitch = runs[1].Run.Origin.Y - runs[0].Run.Origin.Y;
        pitch.Emu.ShouldBe(Height(SansAscent, SansDescent, pixels).Emu);
    }

    /// <summary>
    /// Impress's own device gives a different answer, so the chart device is doing something.
    /// </summary>
    /// <remarks>
    /// The control that the assertion above is about the device rather than about arithmetic that
    /// would have come out the same anyway. At 10 pt Impress's 600 dpi grid stacks Liberation Sans
    /// 0.24 pt taller and at 11 pt 0.11 pt shorter than <c>chart2</c>'s 96 dpi one; nothing about
    /// the body but its <see cref="SlideTextBody.Device"/> differs between the two calls.
    /// </remarks>
    [Theory]
    [InlineData(10.0)]
    [InlineData(11.0)]
    public void ImpressOwnDeviceStacksTheSameTwoLinesDifferently(double points)
    {
        Length size = Length.FromPoints(points);

        Length chart = Pitch(Place(size, MetricGrid.Chart));
        Length impress = Pitch(Place(size, MetricGrid.Presentation));

        // 0.085 pt at 10 pt and 0.113 at 11 — small, and a whole unit of the coarser device
        // rather than a rounding artefact: 600 dpi in 1/100 mm is a pixel of 4.233 units where
        // 96 dpi is 26.458, so the two grids cannot land on the same answer unless the metric
        // happens to fall on both.
        Math.Abs((chart - impress).Points).ShouldBeGreaterThan(0.05);
    }

    /// <summary>
    /// The face's external leading is not in a chart's line, and the sawtooth runs both ways.
    /// </summary>
    /// <remarks>
    /// EditEngine adds the external leading only under <c>IsAddExtLeading()</c>
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:3133-3135), which is off — and a chart's label
    /// is an EditEngine text made by <c>chart2</c>. Liberation Sans is the face that can tell:
    /// its gap is 67/2048 where Carlito's is zero.
    /// </remarks>
    [Fact]
    public void TenPointStacksShorterThanTheDesignMetricAndElevenPointTaller()
    {
        double design(double points) => (SansAscent + SansDescent + SansLineGap) * points / UnitsPerEm;

        Pitch(Place(Length.FromPoints(10.0), MetricGrid.Chart)).Points
            .ShouldBeLessThan(design(10.0) - 0.2);
        Pitch(Place(Length.FromPoints(11.0), MetricGrid.Chart)).Points
            .ShouldBeGreaterThan(design(11.0) + 0.05);
    }

    /// <summary>
    /// A real chart's value-axis label sits on its own tick by the device's ascent and height.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The path assertion, on the shapes <see cref="SlideChart"/> actually produces: everything
    /// above measures the layout a chart label is given, and this checks that a chart is given it.
    /// A value-axis label is centred on its tick, so <c>tick − baseline</c> is
    /// <c>ascent − height/2</c> — the quantity <c>probes/chart-vertical/tickoffset.py</c> reads
    /// out of the reference's own PDF, where it matches this rule on <b>72 of 72</b> cases against
    /// 24.2.7.2 with no free parameter and on 72 of 72 against 26.2.4.2 with one constant of a
    /// hundredth of a millimetre.
    /// </para>
    /// <para>
    /// <strong><c>chart-face-stated.pptx</c> rather than the theme-minor deck beside it, because
    /// this quantity nearly cancels and the face decides how nearly.</strong> A label is drawn at
    /// <c>blockCentre − height/2 + ascent</c>, so an error shared by the height and the ascent
    /// disappears from it — which is exactly why round 60's sheets defect went unseen. Liberation
    /// Mono at 10 pt puts <c>chart2</c>'s device and Impress's 0.014 pt apart here and Liberation
    /// Serif puts them <b>0.043 pt</b> apart, so this deck can tell them apart and that one
    /// cannot. The second assertion is what states that: the answer this test accepts must
    /// exclude the 600 dpi one.
    /// </para>
    /// </remarks>
    [Fact]
    public void AValueAxisLabelSitsOnItsTickByTheDevicesOwnMetrics()
    {
        LaidOutSlide slide = Slide("chart-face-stated.pptx");

        (GlyphRun label, Length baseline, _) = Baseline(slide, "180");
        label.FontSize.Points.ShouldBe(10.0, 0.02, "the deck states ten point axis labels");

        int pixels = PixelEm(label.FontSize);
        pixels.ShouldBe(13, "13 whole pixels for 13.34");

        Length tick = NearestTick(slide, baseline);

        // A slide's y grows downwards where a PDF's grows up, so the probe's `tick − baseline`
        // is this subtraction the other way round. The quantity is the same one.
        double offset = (baseline - tick).Points;
        double expected = Pixels(SerifAscent, pixels).Points
                          - (Height(SerifAscent, SerifDescent, pixels).Points / 2.0);

        offset.ShouldBe(expected, 0.02);

        // And the answer Impress's own 600 dpi device would have given is outside that band, so
        // the assertion above is about which device rather than about arithmetic both agree on.
        int impress = (int)Math.Round(
            label.FontSize.Mm100 * 600.0 / 2540.0, MidpointRounding.AwayFromZero);
        double other = Pixels(SerifAscent, impress, 600).Points
                       - (Height(SerifAscent, SerifDescent, impress, 600).Points / 2.0);

        Math.Abs(offset - other).ShouldBeGreaterThan(0.02);
    }

    /// <summary>Two lines of one chart label, laid out on a stated device.</summary>
    private static List<PlacedGlyphRun> Place(Length size, MetricGrid device)
    {
        // The body a chart label is: no insets, no wrap, top anchored, and the face's own metrics
        // rather than a fixed fraction of the em. `SlideChart.Measurer.Body` builds exactly this.
        SlideTextBody body = new()
        {
            Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
            Wraps = false,
            Anchor = TextAnchor.Top,
            FontIndependentLineSpacing = false,
            Device = device,
            Paragraphs =
            [
                Paragraph("Mg", size),
                Paragraph("Mg", size),
            ],
        };

        return SlideTextLayout.Place(
            body,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(400)),
            new SlideFonts());
    }

    private static SlideParagraph Paragraph(string text, Length size)
        => new(text,
               [new SlideTextRun(0, text.Length, "Liberation Sans", size, 400, false, Colour.Black)],
               TextAlignment.Start);

    private static Length Pitch(List<PlacedGlyphRun> runs)
    {
        runs.Count.ShouldBe(2);
        return runs[1].Run.Origin.Y - runs[0].Run.Origin.Y;
    }

    private static LaidOutSlide Slide(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides[0];
    }

    /// <summary>One named run, and its baseline in the slide's own coordinates.</summary>
    /// <remarks>
    /// A <see cref="PlacedText"/>'s runs are in the shape's coordinates and its
    /// <see cref="PlacedText.Transform"/> is what puts them on the slide, while a
    /// <see cref="PlacedShape.Outline"/> is already transformed — so a run and a tick are not
    /// comparable until this has been applied.
    /// </remarks>
    private static (GlyphRun Run, Length Baseline, Length Left) Baseline(
        LaidOutSlide slide, string text)
    {
        List<(GlyphRun Run, Length Baseline, Length Left)> found = [];
        foreach (PlacedShape shape in slide.Shapes)
        {
            if (shape.Text is not { } placed) continue;

            foreach (PlacedGlyphRun run in placed.Runs)
            {
                if (run.Run.Text != text) continue;

                AffineTransform matrix = placed.Transform;
                double x = (run.Run.Origin.X.Emu * matrix.A)
                           + (run.Run.Origin.Y.Emu * matrix.C)
                           + matrix.E;
                double y = (run.Run.Origin.X.Emu * matrix.B)
                           + (run.Run.Origin.Y.Emu * matrix.D)
                           + matrix.F;
                found.Add((run.Run,
                           Length.FromEmu((long)Math.Round(y)),
                           Length.FromEmu((long)Math.Round(x))));
            }
        }

        found.Count.ShouldBe(1, $"one run reading {text}");
        return found[0];
    }

    /// <summary>
    /// The value axis' tick nearest a label's baseline: a short horizontal two-point stroke.
    /// </summary>
    /// <remarks>
    /// Selected by shape rather than by index, and required to be unambiguous: the nearest tick
    /// has to be at least a third of the tick spacing closer than the next one, or the pairing is
    /// the thing being asserted rather than the offset.
    /// </remarks>
    private static Length NearestTick(LaidOutSlide slide, Length baseline)
    {
        List<Length> ticks = [];
        foreach (PlacedShape shape in slide.Shapes)
        {
            if (shape.Text is not null) continue;
            if (shape.Outline.Commands.Count != 2) continue;
            if (shape.Outline.Commands[0].Verb != PathVerb.MoveTo) continue;
            if (shape.Outline.Commands[1].Verb != PathVerb.LineTo) continue;

            DocPoint from = shape.Outline.Commands[0].Point;
            DocPoint to = shape.Outline.Commands[1].Point;
            Length across = Length.FromEmu(Math.Abs(to.X.Emu - from.X.Emu));
            Length down = Length.FromEmu(Math.Abs(to.Y.Emu - from.Y.Emu));

            // Horizontal and short: on a column chart the value axis is the upright one, so its
            // ticks are the only short horizontal strokes on the slide. The category axis' are
            // upright, both axis lines are long, and every bar is a five-command rectangle.
            if (down > Length.FromPoints(0.1)) continue;
            if (across > Length.FromPoints(12)) continue;

            ticks.Add(from.Y);
        }

        ticks.Count.ShouldBeGreaterThan(2, "the value axis draws its ticks");
        ticks.Sort((first, second) =>
            Math.Abs((first - baseline).Emu).CompareTo(Math.Abs((second - baseline).Emu)));

        Math.Abs((ticks[1] - baseline).Emu)
            .ShouldBeGreaterThan(Math.Abs((ticks[0] - baseline).Emu) * 3,
                                 "the label belongs to one tick and not between two");

        return ticks[0];
    }

    /// <summary>The em in whole 96 dpi pixels, through the device's own map unit.</summary>
    private static int PixelEm(Length size)
        => (int)Math.Round(size.Mm100 * 96.0 / 2540.0, MidpointRounding.AwayFromZero);

    /// <summary>A design-unit metric at a whole-pixel em, back in hundredths of a millimetre.</summary>
    /// <param name="designUnits">The metric, in the face's design units.</param>
    /// <param name="pixelEm">The em in whole device pixels.</param>
    /// <param name="dpi">The device's resolution: 96 for <c>chart2</c>'s, 600 for Impress's.</param>
    private static Length Pixels(int designUnits, int pixelEm, int dpi = 96)
        => Length.FromMm100((long)Math.Round(
            Math.Round(designUnits * (double)pixelEm / UnitsPerEm, MidpointRounding.AwayFromZero)
            * 2540.0 / dpi, MidpointRounding.AwayFromZero));

    /// <summary>
    /// EditEngine's line height: the taller of converting each metric on its own and converting
    /// their sum in one step (<c>editeng/source/editeng/impedit3.cxx</c>:1516-1518).
    /// </summary>
    /// <param name="ascent">The face's ascent, in design units.</param>
    /// <param name="descent">Its descent, positive, in design units.</param>
    /// <param name="pixelEm">The em in whole device pixels.</param>
    /// <param name="dpi">The device's resolution.</param>
    private static Length Height(int ascent, int descent, int pixelEm, int dpi = 96)
    {
        long up = (long)Math.Round(
            ascent * (double)pixelEm / UnitsPerEm, MidpointRounding.AwayFromZero);
        long down = (long)Math.Round(
            descent * (double)pixelEm / UnitsPerEm, MidpointRounding.AwayFromZero);

        Length separately = Length.FromMm100(
            (long)Math.Round(up * 2540.0 / dpi, MidpointRounding.AwayFromZero)
            + (long)Math.Round(down * 2540.0 / dpi, MidpointRounding.AwayFromZero));
        Length together = Length.FromMm100(
            (long)Math.Round((up + down) * 2540.0 / dpi, MidpointRounding.AwayFromZero));

        return separately > together ? separately : together;
    }
}

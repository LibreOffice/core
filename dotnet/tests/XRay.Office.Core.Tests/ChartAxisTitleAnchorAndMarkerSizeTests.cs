using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// Two placements a chart takes from the reference's second pass rather than from its first.
/// </summary>
/// <remarks>
/// <para>
/// <c>lcl_createTitle</c> puts an axis title somewhere provisional and reserves its band; once the
/// diagram exists, <c>changePositionOfAxisTitle</c> moves it again
/// (<c>ChartView.cxx:1996-1998</c>). The two passes do not agree, and they do not even use the
/// same distance constant — the reservation's is a flat 420 hundredths of a millimetre and the
/// placement's is two per cent of the page's height. A renderer that transcribes only the first
/// pass reserves the right band and draws in the wrong place inside it, which is the failure mode
/// that leaves the picture the right size.
/// </para>
/// <para>
/// The marker's size is the same shape of defect from the other end: chart2's unset default is
/// 250 × 250 hundredths of a millimetre and <em>no OOXML chart ever keeps it</em>, because
/// <c>TypeGroupConverter::convertMarker</c> assigns a size from <c>c:marker/c:size</c> or from
/// <c>mnMarkerSize(5)</c> for every series it reaches.
/// </para>
/// <para>
/// Both are asserted on the model rather than on a format, because both reach every family: a
/// sheet's chart and a slide's chart compose through the same <see cref="ChartLayout"/>.
/// </para>
/// </remarks>
public class ChartAxisTitleAnchorAndMarkerSizeTests
{
    /// <summary>Half an em per character, 1.15 em a line — Liberation Sans to three places.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    private static ChartPlot Columns() => new()
    {
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series = [new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))],
        CategoryAxisTitle = "Aircraft Type",
    };

    private static ChartLabel Below(ChartDrawing drawing)
        => drawing.Labels.First(label => label.Text == "Aircraft Type");

    /// <summary>
    /// The title under the plot is centred on the diagram rectangle, not on the plot rectangle.
    /// </summary>
    /// <remarks>
    /// The two coincide on a symmetric chart and part company as soon as something is taken off
    /// one side only — here a secondary value axis with a title of its own, which is
    /// <c>Demick_JetBlue.pptx</c>'s shape. The reference's own extents on that deck's page 4 put
    /// the title's ink centre at 352.78 where the inner plot rectangle's centre is 374.55 and the
    /// diagram rectangle's is 352.80.
    /// </remarks>
    [Fact]
    public void TheTitleUnderThePlotIsCentredOnTheDiagramRectangle()
    {
        ChartDrawing drawing = Place(Columns() with
        {
            SecondaryValueAxisTitle = "Load Factor",
            SecondaryAxisVisible = true,
            Series =
            [
                new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF)),
                new ChartSeries("Rate", [0.8, 0.9, 0.7, 0.6], Colour.FromRgb(0xC0504D))
                {
                    AxisIndex = 1,
                },
            ],
        });

        Length plotCentre = drawing.PlotArea.X + (drawing.PlotArea.Width / 2);
        Length diagramCentre = drawing.DiagramArea.X + (drawing.DiagramArea.Width / 2);

        diagramCentre.ShouldNotBe(plotCentre, "the arms must differ or the test proves nothing");

        Below(drawing).At.X.Emu.ShouldBeInRange(diagramCentre.Emu - 2000L, diagramCentre.Emu + 2000L);
    }

    /// <summary>
    /// The plainest chart there is takes the same rule, and its two centres are not the same point.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The second arm of the discriminator, and it is here because the obvious control — "a
    /// symmetric chart puts the two centres together" — is **false**, which is worth writing down.
    /// A column chart with one series and no secondary axis still has its value-axis labels down
    /// the left and nothing down the right, so the diagram rectangle is wider on the left than the
    /// plot rectangle and the two centres are 11 pt apart on a 400 pt frame.
    /// </para>
    /// <para>
    /// So there is no chart on which the two readings agree, and every chart in the corpus that
    /// draws a title below its plot moves. That is the opposite of what the census suggested and
    /// it is the reason this test exists rather than the equality it replaced.
    /// </para>
    /// </remarks>
    [Fact]
    public void ThePlainestChartTakesTheSameRuleAndItsTwoCentresDiffer()
    {
        ChartDrawing drawing = Place(Columns());

        Length plotCentre = drawing.PlotArea.X + (drawing.PlotArea.Width / 2);
        Length diagramCentre = drawing.DiagramArea.X + (drawing.DiagramArea.Width / 2);

        plotCentre.Emu.ShouldBeGreaterThan(diagramCentre.Emu,
            "the value axis' labels widen the diagram rectangle on the left alone");

        Below(drawing).At.X.Emu.ShouldBeInRange(diagramCentre.Emu - 2000L, diagramCentre.Emu + 2000L);
    }

    /// <summary>
    /// Two per cent of the frame's height sits between the diagram rectangle and the title.
    /// </summary>
    /// <remarks>
    /// <c>changePositionOfAxisTitle</c>'s <c>ALIGN_BOTTOM</c> arm is
    /// <c>rect.Y + rect.Height + h/2 + pageHeight × 0.02</c> (<c>ChartView.cxx:1012-1015</c>).
    /// Measured on <c>Demick_JetBlue.pptx</c> page 4, whose chart frame is 331.2 pt tall: the
    /// reference's title top is 6.50 pt below where we drew it and the term is 6.62.
    /// </remarks>
    [Fact]
    public void TheTitleUnderThePlotClearsTheDiagramByTwoPerCentOfTheHeight()
    {
        ChartDrawing drawing = Place(Columns());

        // The Ruler's line height for a 9 pt title, plus `ShapeFactory::createText`'s 0.30 inset
        // above and below it -- the shape's height, which is what the arithmetic halves.
        Length half = Length.FromPoints(((9 * 1.15) + (9 * 0.6)) / 2);
        Length gap = Below(drawing).At.Y - half - drawing.DiagramArea.Bottom;

        Length expected = Frame.Height * 0.02;

        gap.Emu.ShouldBeInRange(expected.Emu - 4000L, expected.Emu + 4000L);
    }

    /// <summary>
    /// Making the frame taller makes that clearance taller with it, because it is a proportion.
    /// </summary>
    /// <remarks>
    /// The discriminator against the flat 420 hundredths of a millimetre the <em>reservation</em>
    /// uses: a constant would not move at all, and reading the reservation's constant into the
    /// placement is exactly the mistake this pair of rules invites.
    /// </remarks>
    [Fact]
    public void ThatClearanceIsAProportionAndNotAConstant()
    {
        DocRect tall = Frame with { Height = Frame.Height * 2 };

        ChartDrawing one = Place(Columns());
        ChartDrawing two = ChartLayout.Place(Columns(), tall, new Ruler());

        Length half = Length.FromPoints(((9 * 1.15) + (9 * 0.6)) / 2);
        Length shortGap = Below(one).At.Y - half - one.DiagramArea.Bottom;
        Length tallGap = Below(two).At.Y - half - two.DiagramArea.Bottom;

        tallGap.Emu.ShouldBeGreaterThan(shortGap.Emu + 40000L,
            "twice the height is twice the clearance, not the same constant");
    }

    private static ChartPlot Markers(Length? size) => new()
    {
        Kind = ChartPlotKind.Line,
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        LabelSize = Length.FromPoints(10),
        Series =
        [
            new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))
            {
                Marker = ChartMarker.Square,
                HasLine = true,
                MarkerSize = size,
            },
        ],
    };

    /// <summary>
    /// The side of the marker shapes on a line chart, which are the only small closed paths on it.
    /// </summary>
    /// <remarks>
    /// The series' own polyline is four commands too, so counting commands does not separate them;
    /// its extent is the whole plot area and a marker's is a few points, so the extent does.
    /// </remarks>
    private static Length Side(ChartDrawing drawing)
    {
        List<Length> sides = [];

        foreach (ChartShape shape in drawing.Shapes)
        {
            // Close carries a default point, which is the origin — including it makes every
            // closed path as wide as its own distance from the left edge of the frame.
            List<Length> xs = [.. shape.Path.Commands
                .Where(command => command.Verb != PathVerb.Close)
                .Select(command => command.Point.X)];
            if (xs.Count < 3) continue;

            Length width = xs.Max() - xs.Min();
            if (width > Length.FromPoints(40)) continue;

            sides.Add(width);
        }

        return sides.Count == 0 ? Length.Zero : sides.Max();
    }

    /// <summary>A stated marker size is the marker's side, whatever the labels are set at.</summary>
    /// <remarks>
    /// <c>003_advanced_powerpoint_line.pptx</c> states <c>&lt;c:size val="6"/&gt;</c>, whose
    /// conversion to hundredths of a millimetre is 212 and back to points is 6.0094 — which is
    /// what 26.2.4.2 draws there and what we drew as 7.00 until this was read.
    /// </remarks>
    [Fact]
    public void AStatedMarkerSizeIsTheMarkersSide()
    {
        Length stated = Length.FromMm100(212);

        Side(Place(Markers(stated))).Emu.ShouldBeInRange(stated.Emu - 2000L, stated.Emu + 2000L);
    }

    /// <summary>
    /// A series stating no size keeps chart2's own unset default, which is not the labels' size.
    /// </summary>
    /// <remarks>
    /// The control that keeps every ODF and binary chart where it was: those readers have no
    /// <c>c:marker</c> to read, so <see cref="ChartSeries.MarkerSize"/> stays null and the
    /// fallback — 0.7 of the label size, the transcription of
    /// <c>VDataSeries::getSymbolProperties</c>' 250 — is what draws them.
    /// </remarks>
    [Fact]
    public void AMarkerStatingNoSizeKeepsTheUnsetDefault()
    {
        Length fallback = Length.FromPoints(10) * 0.7;

        Side(Place(Markers(null))).Emu.ShouldBeInRange(fallback.Emu - 2000L, fallback.Emu + 2000L);
    }

    /// <summary>The stated size wins over the label size rather than scaling with it.</summary>
    /// <remarks>
    /// Two arms that a "fraction of the label size" reading cannot both satisfy: the same stated
    /// size against two different label sizes must draw the same marker.
    /// </remarks>
    [Fact]
    public void TheStatedSizeDoesNotMoveWithTheLabelSize()
    {
        Length stated = Length.FromMm100(212);

        ChartPlot small = Markers(stated) with { LabelSize = Length.FromPoints(8) };
        ChartPlot large = Markers(stated) with { LabelSize = Length.FromPoints(18) };

        Side(Place(small)).Emu.ShouldBeInRange(Side(Place(large)).Emu - 2000L, Side(Place(large)).Emu + 2000L);
        Side(Place(small)).Emu.ShouldBeInRange(stated.Emu - 2000L, stated.Emu + 2000L);
    }

    /// <summary>And a marker larger than the fallback is drawn larger, not clamped to it.</summary>
    /// <remarks>
    /// Fourteen corpus series state 14 points or more and one states 62, so the change is not
    /// one-directional: reading the stated size makes some markers smaller and some larger, and a
    /// test that only ever shrinks them would miss an implementation that took a minimum.
    /// </remarks>
    [Fact]
    public void AMarkerLargerThanTheFallbackIsDrawnLarger()
    {
        Length stated = Length.FromPoints(18);

        Side(Place(Markers(stated))).Emu.ShouldBeInRange(stated.Emu - 2000L, stated.Emu + 2000L);
        Side(Place(Markers(stated))).Emu.ShouldBeGreaterThan(Side(Place(Markers(null))).Emu);
    }
}

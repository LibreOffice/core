using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A pie's <c>bestFit</c> data labels: the legend key, the inner placement, the outside fallback
/// and the diagram shrink that follows from them.
/// </summary>
/// <remarks>
/// <para>
/// The fixture is <c>003_advanced_excel_pie</c>'s own chart — five points at 93, 100, 107, 114 and
/// 121 in a 510 × 283 pt frame with a title, a right legend, <c>bestFit</c> placement and
/// <c>c:showLegendKey</c> — because every figure asserted here was read off that document's
/// reference rendering through the installed 26.2.4.2 before any of this was written.
/// </para>
/// <para>
/// The measurer returns the <em>reference's own</em> advances for these five strings rather than a
/// synthetic per-character width, because the best-fit test is genuinely knife-edge on this chart:
/// the four wrapped labels clear their slices by between 1 and 5 degrees. A ruler that is 5% out
/// changes which side of the test they fall on, which is a real property of the algorithm and not
/// an artefact of the fixture — see <c>probes/sheets-r59/results.md</c> § "the residual".
/// </para>
/// </remarks>
public class ChartPieBestFitTests
{
    /// <summary>The reference's own Carlito advances, keyed by the string.</summary>
    private sealed class Carlito : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
        {
            // Read off 003_advanced_excel_pie's reference rendering: the 19-glyph label's block is
            // 88.16 wide including its 8.818 pt key and gap, and a wrapped line's is 74.0.
            double width = text.Length switch
            {
                19 => 79.34,
                17 => 65.18,
                16 => 65.18,
                3 => 18.63,
                _ => 4.2 * text.Length,
            };

            return new DocSize(
                Length.FromPoints(width * (size.Points / 10.01) * (bold ? 1.1 : 1.0)),
                Length.FromPoints(11.23 * (size.Points / 10.01)));
        }
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(510.01), Length.FromPoints(283.35));

    private static readonly double[] Values = [93, 100, 107, 114, 121];

    private static ChartDataLabel Label(ChartLabelPlacement? placement, bool key = true) => new()
    {
        ShowValue = true,
        ShowPercent = true,
        ShowCategory = true,
        ShowSeries = true,
        ShowLegendKey = key,
        Separator = "; ",
        Placement = placement,
    };

    private static ChartPlot Pie(ChartLabelPlacement? placement, bool key = true, bool rings = false)
        => new()
        {
            Kind = ChartPlotKind.Pie,
            Rings = rings,
            Title = "Rolling 12-month trend",
            TitleSize = Length.FromPoints(18),
            LabelSize = Length.FromPoints(10),
            Legend = ChartLegendPosition.Right,
            Categories = ["M1", "M2", "M3", "M4", "M5"],
            Series =
            [
                new ChartSeries(
                    "Actual",
                    [.. Values.Select(v => (double?)v)],
                    Colour.FromRgb(0x4F81BD),
                    PointFills:
                    [
                        Colour.FromRgb(0x4F81BD), Colour.FromRgb(0xC0504D), Colour.FromRgb(0x9BBB59),
                        Colour.FromRgb(0x8064A2), Colour.FromRgb(0x4BACC6),
                    ])
                {
                    Label = Label(placement, key),
                },
            ],
        };

    private static double Radius(ChartDrawing drawing)
    {
        DocRect area = drawing.PlotArea;
        return (area.Width < area.Height ? area.Width : area.Height).Points / 2;
    }

    /// <summary>The label keys — the small squares, not the five legend swatches at the right.</summary>
    private static List<DocRect> Keys(ChartDrawing drawing)
        => [.. drawing.Shapes
            .Select(shape => Bounds(shape.Path))
            .Where(box => Math.Abs(box.Width.Points - box.Height.Points) < 0.05
                          && box.Width.Points is > 4 and < 8)];

    private static DocRect Bounds(GraphicsPath path)
    {
        double x0 = double.MaxValue, y0 = double.MaxValue, x1 = double.MinValue, y1 = double.MinValue;
        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb is PathVerb.Close) continue;

            List<DocPoint> points = command.Verb is PathVerb.CubicTo
                ? [command.Control1, command.Control2, command.Point]
                : [command.Point];

            foreach (DocPoint point in points)
            {
                x0 = Math.Min(x0, point.X.Points);
                y0 = Math.Min(y0, point.Y.Points);
                x1 = Math.Max(x1, point.X.Points);
                y1 = Math.Max(y1, point.Y.Points);
            }
        }

        return x1 < x0
            ? DocRect.Empty
            : new DocRect(Length.FromPoints(x0), Length.FromPoints(y0),
                          Length.FromPoints(x1 - x0), Length.FromPoints(y1 - y0));
    }

    /// <summary>
    /// A label's legend key is six tenths of the font height square, and its text starts a further
    /// key-width-plus-one-millimetre to the right.
    /// </summary>
    /// <remarks>
    /// <c>nSymbolHeight = int(fViewFontSize × 0.6)</c> and
    /// <c>nXDiff = symbolWidth + int(max(100, fViewFontSize × 0.22))</c>, both in hundredths of a
    /// millimetre — 5.98 pt and 8.818 pt at 10 pt, measured on the reference to the hundredth.
    /// <strong>Before this, nothing drew the key at all</strong>: <c>c:showLegendKey</c> was read
    /// only by <c>StatesLabelSetting</c>'s existence test and its value went nowhere.
    /// </remarks>
    [Fact]
    public void ALabelLegendKeyIsSixTenthsOfTheFontHeightSquare()
    {
        List<DocRect> keys = Keys(ChartLayout.Place(Pie(ChartLabelPlacement.BestFit), Frame, new Carlito()));

        keys.ShouldNotBeEmpty();
        foreach (DocRect key in keys)
        {
            key.Width.Points.ShouldBe(5.98, 0.02);
            key.Height.Points.ShouldBe(5.98, 0.02);
        }
    }

    /// <summary>A label that states no legend key draws none.</summary>
    [Fact]
    public void ALabelThatStatesNoLegendKeyDrawsNone()
        => Keys(ChartLayout.Place(Pie(ChartLabelPlacement.BestFit, key: false), Frame, new Carlito()))
            .ShouldBeEmpty();

    /// <summary>
    /// One of the five labels does not fit inside its slice, and the reference leaves the legend
    /// key of the discarded inner attempt on the page beside the rebuilt one.
    /// </summary>
    /// <remarks>
    /// <c>xShapes->remove(aPieLabelInfo.xTextShape)</c> in <c>PieChart::createTextLabelShape</c>'s
    /// outside fallback takes the text away and leaves its sibling key behind. The reference draws
    /// <strong>six</strong> keys for five labels on <c>003_advanced_excel_pie</c> page 1, at
    /// (390.67, 504.20) and (462.90, 556.07) in the same series colour, and that duplicate is what
    /// identifies the mechanism from outside the binary.
    /// </remarks>
    [Fact]
    public void PieLabelKeepsTheDiscardedInnerKey()
        => Keys(ChartLayout.Place(Pie(ChartLabelPlacement.BestFit), Frame, new Carlito()))
            .Count.ShouldBe(6);

    /// <summary>
    /// Exactly one label is rebuilt outside the rim, and it is the one on the narrowest slice.
    /// </summary>
    [Fact]
    public void OnlyTheLabelOnTheNarrowestSliceIsRebuiltOutside()
    {
        ChartDrawing drawing = ChartLayout.Place(Pie(ChartLabelPlacement.BestFit), Frame, new Carlito());

        DocRect area = drawing.PlotArea;
        DocPoint centre = new(area.X + (area.Width / 2), area.Y + (area.Height / 2));
        double radius = Radius(drawing);

        int beyond = Keys(drawing).Count(key => Math.Sqrt(
            Math.Pow((key.X + (key.Width / 2)).Points - centre.X.Points, 2)
            + Math.Pow((key.Y + (key.Height / 2)).Points - centre.Y.Points, 2)) > radius);

        beyond.ShouldBe(1);
    }

    /// <summary>
    /// A pie whose labels are pushed outside the rim is smaller than the same pie whose labels are
    /// centred inside it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The whole of the diagram's second pass. Measured on the corpus witness through the
    /// installed binary: <c>ctr</c> and <c>inEnd</c> both draw the pie at radius
    /// <strong>110.44</strong> and <c>bestFit</c> and <c>outEnd</c> at <strong>99.78</strong> —
    /// identical to each other, to the digit — because a label rebuilt outside the rim enlarges
    /// the rectangle the diagram consumed and <c>VDiagram::adjustInnerSize</c> gives that back.
    /// </para>
    /// <para>
    /// The fixture lengthens the category names so that every label leaves its slice, including
    /// the one at the bottom of the pie, because <strong>the shrink is driven by the
    /// <em>vertical</em> overflow on this frame</strong>: the diagram is 465 pt wide and 221 tall,
    /// so a label hanging 40 pt off the right edge changes nothing and one hanging 12 pt below the
    /// bottom changes everything. That asymmetry is the reason
    /// <see cref="ACentredOrInsideLabelLeavesTheDiagramAlone"/> is a separate test rather than the
    /// same one with a different placement.
    /// </para>
    /// </remarks>
    [Fact]
    public void ALabelRebuiltOutsideTheRimShrinksTheDiagram()
    {
        ChartPlot fitted = Pie(ChartLabelPlacement.BestFit) with
        {
            Categories = ["Category one", "Category two", "Category three", "Category four",
                          "Category five"],
        };

        Radius(ChartLayout.Place(fitted, Frame, new Carlito()))
            .ShouldBeLessThan(Radius(ChartLayout.Place(
                fitted with { Kind = ChartPlotKind.Pie, Series = CentredLabels(fitted) },
                Frame,
                new Carlito())) - 2.0);
    }

    /// <summary>The same series with its labels centred rather than best-fitted.</summary>
    private static IReadOnlyList<ChartSeries> CentredLabels(ChartPlot plot)
        => [.. plot.Series.Select(series => series with
        {
            Label = series.Label! with { Placement = ChartLabelPlacement.Centre },
        })];

    /// <summary>
    /// The control: a placement that keeps every label inside the pie leaves the diagram alone.
    /// </summary>
    /// <remarks>
    /// <c>ctr</c>, <c>inEnd</c> and a pie with no labels at all must all draw the same circle, and
    /// the reference says they do — 110.44 in every case against 110.44 with the labels deleted.
    /// This is the test that would catch a shrink applied unconditionally, which would move every
    /// pie in three corpora rather than the five documents that state <c>bestFit</c>.
    /// </remarks>
    [Fact]
    public void ACentredOrInsideLabelLeavesTheDiagramAlone()
    {
        double none = Radius(ChartLayout.Place(
            Pie(ChartLabelPlacement.Centre, key: false) with
            {
                Series = [new ChartSeries("Actual", [.. Values.Select(v => (double?)v)],
                                          Colour.FromRgb(0x4F81BD))],
            },
            Frame,
            new Carlito()));

        Radius(ChartLayout.Place(Pie(ChartLabelPlacement.Centre), Frame, new Carlito()))
            .ShouldBe(none, 0.01);
        Radius(ChartLayout.Place(Pie(ChartLabelPlacement.Inside), Frame, new Carlito()))
            .ShouldBe(none, 0.01);
    }

    /// <summary>
    /// A doughnut takes none of this: <c>bMovementAllowed &amp;&amp; !m_bUseRings</c>.
    /// </summary>
    /// <remarks>
    /// <c>AVOID_OVERLAP</c> is converted to <c>CENTER</c> for every pie, and only a pie that is not
    /// a ring chart is then allowed to move. A doughnut whose labels moved would be a change to
    /// eight sheets documents and twenty-three decks that state <c>c:doughnutChart</c>, none of
    /// which the reference moves.
    /// </remarks>
    [Fact]
    public void ADoughnutIsNotBestFitted()
    {
        ChartDrawing rings = ChartLayout.Place(
            Pie(ChartLabelPlacement.BestFit, rings: true), Frame, new Carlito());

        Keys(rings).ShouldBeEmpty();
        Radius(rings).ShouldBe(
            Radius(ChartLayout.Place(Pie(ChartLabelPlacement.Centre, rings: true), Frame, new Carlito())),
            0.01);
    }

    /// <summary>
    /// A pie label wraps at four fifths of the pie's radius, and not at any other width.
    /// </summary>
    /// <remarks>
    /// <para>
    /// "A reasonable start for bestFitting a 90deg slice oriented on an Axis is 80% of the radius."
    /// The plot area is stated here so the radius is a fixed 102 pt and the allowance therefore
    /// 81.6, which sits between the nineteen-glyph label's 79.34 and the twenty-glyph labels'
    /// 84.00 — so exactly four of the five must wrap and the fifth must not.
    /// </para>
    /// <para>
    /// That is the same split the reference draws on <c>003_advanced_excel_pie</c>, for the same
    /// reason: at its shrunk radius of 99.78 the allowance is 79.82 and the one label narrower
    /// than that is the one drawn on a single line.
    /// </para>
    /// </remarks>
    [Fact]
    public void ALabelWiderThanFourFifthsOfTheRadiusWraps()
    {
        ChartPlot stated = Pie(ChartLabelPlacement.Centre) with
        {
            PlotAreaFraction = (0.1, 0.05, 204.0 / 510.01, 204.0 / 283.35),
        };

        List<ChartLabel> labels = [.. ChartLayout
            .Place(stated, Frame, new Carlito())
            .Labels
            .Where(label => label.Text.Contains(';'))];

        labels.Count.ShouldBe(5);
        labels.Count(label => label.Text.Contains('\n')).ShouldBe(4);

        // The control: widen the pie past the widest label and none of them wraps.
        ChartPlot wider = stated with
        {
            PlotAreaFraction = (0.05, 0.02, 260.0 / 510.01, 260.0 / 283.35),
        };

        ChartLayout.Place(wider, Frame, new Carlito())
            .Labels
            .Count(label => label.Text.Contains('\n'))
            .ShouldBe(0);
    }
}

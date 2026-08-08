using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// What a legend takes out of the frame, and what is left for the diagram once the titles have
/// had their share.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why these are worth pinning.</strong> The legend's width is the largest single term in
/// the plot rectangle and it is invisible in any picture: a legend one key too narrow still lists
/// every series, still reads correctly, and moves every bar in the chart. Measured over the 100
/// charts in <c>chart2/qa/extras/data/</c>'s ODF documents that state a
/// <c>chart:coordinate-region</c> after a round trip through <c>soffice</c>, the mean error on
/// the plot rectangle's right edge was <strong>28.5 pt</strong> against 6.5 on its left — and 74
/// of those charts put their legend on the right.
/// </para>
/// <para>
/// The numbers here are <c>lcl_placeLegendEntries</c>'s
/// (<c>chart2/source/view/main/VLegend.cxx:286-293</c>), so they are asserted as arithmetic over
/// the stand-in ruler rather than as constants: a change to the ruler must not be able to make a
/// wrong formula pass.
/// </para>
/// </remarks>
public class ChartLegendLayoutTests
{
    /// <summary>Half an em per character, 1.15 em a line — Liberation Sans to three places.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static readonly Length Millimetre = Length.FromMm100(100);

    private static ChartPlot Bars(params string[] names)
    {
        List<ChartSeries> series = [];
        foreach (string name in names)
            series.Add(new ChartSeries(name, [1.0, 2.0, 3.0], Colour.FromRgb(0x99CCFF)));

        return new ChartPlot
        {
            Categories = ["Q1", "Q2", "Q3"],
            Series = series,
            Legend = ChartLegendPosition.Right,
        };
    }

    private static ChartDrawing Place(ChartPlot plot, DocRect? frame = null)
        => ChartLayout.Place(plot, frame ?? Frame, new Ruler());

    /// <summary>
    /// The room a right-hand legend takes is padding, key, gap and the name's <em>plain
    /// text</em> — the one piece of chart text that is not measured as a text shape.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>2 × max(1 mm, 0.33 em) + 0.6 em + max(1 mm, 0.22 em) + text</c>, plus
    /// <c>lcl_getLegendLeftRightMargin</c>'s flat 210 between the legend and the diagram. The
    /// estimate this replaced — a key of 0.7 line heights and a gap of 0.4, with no padding at
    /// all — is 6.7 pt narrower on this chart, all of it taken out of the plot area.
    /// </para>
    /// <para>
    /// <strong>And no <c>2 × 0.18 em</c> inset, which this test used to assert.</strong>
    /// <c>lcl_createTextShapes</c> calls the plain-<c>OUString</c> overload of
    /// <c>ShapeFactory::createText</c> (<c>ShapeFactory.cxx:2042</c>), which sets no text
    /// distances; the overload that sets them (<c>:2168</c>) is reached only from
    /// <c>VTitle</c>. Measured on <c>research/probes/slides-r23</c>'s seven decks rather than
    /// taken from that source, because the source is a development branch and the installed
    /// binary made the references: the distance from a key's right edge to its name's pen is
    /// <strong>2.83, 2.83 and 3.07 pt at a 7, 10 and 14 pt legend font</strong>, which is
    /// <c>max(1 mm, 0.22 × font)</c> three times over with nothing added. Adding the inset
    /// would put it at 4.10, 4.64 and 5.60 — and 4.64 is what we drew.
    /// </para>
    /// </remarks>
    [Fact]
    public void ARightHandLegendReservesItsPaddingItsKeyAndTheNamesPlainText()
    {
        ChartPlot plot = Bars("North");
        Length font = plot.LabelSize;

        Length padding = Larger(Millimetre, font * 0.33);
        Length keyGap = Larger(Millimetre, font * 0.22);
        Length key = font * 0.6;
        Length name = font * (0.5 * "North".Length);

        Length expected = (padding * 2.0) + key + keyGap + name + Length.FromMm100(210);

        DocRect area = Place(plot).PlotArea;

        // The right edge is the frame less the page margin less all of that. Compared as a
        // difference against the same chart with no legend, so the assertion is about the legend
        // alone and not about every other reservation.
        DocRect without = Place(plot with { Legend = ChartLegendPosition.None }).PlotArea;

        Near(without.Right - area.Right, expected);
    }

    /// <summary>
    /// Two entries are drawn one row height <em>and one row gap</em> apart, which is exactly
    /// what the height reserved for the box is a sum of.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>lcl_placeLegendEntries</c> reserves <c>2 × paddingY + rows × rowHeight +
    /// (rows − 1) × offsetY</c> and then steps by <c>rowHeight + offsetY</c>. Stepping by the
    /// height alone — which is what we did — leaves the reservation and the placement
    /// disagreeing by one gap per row, so the entries crowd into the top of a box sized for
    /// them spaced out. On a two-entry legend that is the difference between the box being
    /// centred on the frame and its content being.
    /// </para>
    /// <para>
    /// The probe decks put the reference's pitch at <strong>10.34, 14.09 and 19.33 pt</strong>
    /// for a 7, 10 and 14 pt legend font. At 10 pt we drew 17.35 — a line height plus
    /// <c>0.60 × font</c> of text-shape inset and no gap — and now draw 14.18.
    /// </para>
    /// </remarks>
    [Fact]
    public void LegendRowsAreSteppedByTheRowHeightAndTheRowGap()
    {
        ChartPlot plot = Bars("North", "South");
        Length font = plot.LabelSize;

        ChartDrawing drawing = Place(plot);

        List<ChartBox> keys =
            [.. drawing.Boxes.Where(box => box.Bounds.X > drawing.DiagramArea.Right)];

        keys.Count.ShouldBe(2);

        // The ruler's line is 1.15 em and the gap is max(1 mm, 0.20 em); a bare 1.15 em would
        // be the stepping bug and 1.15 em + 0.60 em the inset one, so the three are distinct.
        Length first = keys[0].Bounds.Y;
        Length second = keys[1].Bounds.Y;
        Length pitch = first > second ? first - second : second - first;

        Near(pitch, (font * 1.15) + Larger(Millimetre, font * 0.20));
    }

    /// <summary>
    /// A series that draws a line gets a key 800 hundredths of a millimetre wide, whatever the
    /// font size is.
    /// </summary>
    /// <remarks>
    /// <c>VSeriesPlotter::getPreferredLegendKeyAspectRatio</c> returns <c>(1000, 1000)</c> for a
    /// filled series — a square key of 0.6 em — and <c>(800, -1)</c> for one that draws a line. A
    /// negative height means the width is an absolute length rather than a ratio
    /// (<c>VLegend.cxx:976-984</c>), so at ten point the line chart's key is 22.68 pt against the
    /// bar chart's 6.0. Seventeen points of legend, and nothing in the picture says which rule
    /// was used.
    /// </remarks>
    [Fact]
    public void ALineChartsLegendKeyIsAFlatWidthAndNotAFractionOfTheFont()
    {
        ChartPlot bars = Bars("North");
        ChartPlot lines = bars with
        {
            Kind = ChartPlotKind.Line,
            Series =
            [
                new ChartSeries("North", [1.0, 2.0, 3.0], null, Colour.Black, Length.FromPoints(1))
                {
                    Kind = ChartPlotKind.Line,
                },
            ],
        };

        Length square = bars.LabelSize * 0.6;
        Length flat = Length.FromMm100(800);

        // The diagram's available rectangle rather than the plot rectangle, because a line
        // chart's categories are unshifted and reserve half a label at each end where a bar
        // chart's reserve nothing — a difference that has nothing to do with the legend.
        Length barRight = Place(bars).DiagramArea.Right;
        Length lineRight = Place(lines).DiagramArea.Right;

        // Everything but the key is the same, so the difference is the key's alone.
        Near(barRight - lineRight, flat - square);
    }

    /// <summary>
    /// A legend with more entries than the frame is tall starts a second column rather than
    /// running off the bottom.
    /// </summary>
    /// <remarks>
    /// <c>ChartLegendExpansion_HIGH</c> (<c>VLegend.cxx:507-525</c>): as many rows as the space
    /// allows, then <c>ceil(entries / rows)</c> columns. <c>tdf146463.ods</c> is the corpus case
    /// — fourteen series against a 255 pt frame — and reading it as one column put the plot
    /// rectangle's right edge 120 pt out.
    /// </remarks>
    [Fact]
    public void ALegendTallerThanTheFrameWrapsIntoASecondColumn()
    {
        string[] many =
        [
            "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
            "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
        ];

        DocRect narrow = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(80));

        ChartDrawing drawing = Place(Bars(many), narrow);

        // Every key is drawn — none is dropped — and they are not all in one column. The keys
        // are the boxes outside the diagram; the bars share their fill and are inside it.
        List<ChartBox> keys =
            [.. drawing.Boxes.Where(box => box.Bounds.X > drawing.DiagramArea.Right)];

        keys.Count.ShouldBe(many.Length);

        HashSet<long> columns = [.. keys.Select(box => box.Bounds.X.Emu)];
        columns.Count.ShouldBeGreaterThan(1);
    }

    /// <summary>
    /// An axis title comes out of the diagram's <em>available</em> rectangle, beside the legend
    /// and the main title — not out of the label reservations inside it.
    /// </summary>
    /// <remarks>
    /// <c>createAxisTitleShapes2D</c> (<c>ChartView.cxx:2042-2090</c>) runs before
    /// <c>getAvailablePosAndSizeForDiagram</c> and subtracts from <c>maRemainingSpace</c>, so by
    /// the time the diagram exists the title is already gone. The distinction matters because
    /// <c>ChartDrawing.DiagramArea</c> is compared against an ODF file's own
    /// <c>chart:plot-area</c> rectangle, which is that same <c>maRemainingSpace</c>; attributing
    /// the title to the wrong half makes every such comparison wrong by the title's width while
    /// the plot rectangle itself stays right.
    /// </remarks>
    [Fact]
    public void AnAxisTitleIsTakenOutOfTheDiagramAreaAndNotOutOfTheLabelReservations()
    {
        ChartPlot plot = Bars("North") with { Legend = ChartLegendPosition.None };
        ChartPlot titled = plot with { ValueAxisTitle = "Revenue" };

        ChartDrawing bare = Place(plot);
        ChartDrawing with = Place(titled);

        Length title = (titled.AxisTitleSize * 1.15) + (titled.AxisTitleSize * (0.30 * 2))
                       + Length.FromMm100(450);

        Near(with.DiagramArea.Left - bare.DiagramArea.Left, title);

        // And the plot rectangle moves by exactly the same amount, because nothing else changed.
        Near(with.PlotArea.Left - bare.PlotArea.Left, title);
    }

    /// <summary>
    /// The diagram area is the whole frame less the margin when there is no title and no legend.
    /// </summary>
    /// <remarks>
    /// <c>constPageLayoutDistancePercentage = 0.02</c> on all four sides
    /// (<c>ChartView.cxx:918, 936-943</c>). It is the anchor every other reservation is measured
    /// from, and it is the one number an ODF file's <c>chart:plot-area</c> rectangle agrees with
    /// exactly on a chart that has neither.
    /// </remarks>
    [Fact]
    public void ADiagramWithNoTitleAndNoLegendGetsTheFrameLessTwoPerCent()
    {
        ChartDrawing drawing = Place(Bars("North") with { Legend = ChartLegendPosition.None });

        Near(drawing.DiagramArea.Left, Frame.Width * 0.02);
        Near(drawing.DiagramArea.Top, Frame.Height * 0.02);
        Near(drawing.DiagramArea.Right, Frame.Right - (Frame.Width * 0.02));
        Near(drawing.DiagramArea.Bottom, Frame.Bottom - (Frame.Height * 0.02));
    }

    /// <summary>Equal to within a fiftieth of a point, which is the rounding of a Length.</summary>
    private static void Near(Length got, Length expected)
        => got.Emu.ShouldBeInRange(expected.Emu - 200L, expected.Emu + 200L);

    private static Length Larger(Length one, Length other) => one > other ? one : other;
}

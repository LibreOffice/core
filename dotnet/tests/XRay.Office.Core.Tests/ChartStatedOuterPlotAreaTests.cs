using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A BIFF chart states its plot area, and the labels still come out of it.
/// </summary>
/// <remarks>
/// <para>
/// <c>XclImpChChart::Convert</c> (<c>sc/source/filter/excel/xichart.cxx</c>:4030-4048, this tree)
/// calls <c>setDiagramPositionIncludingAxes</c> with the primary axes set's <c>CHFRAMEPOS</c>
/// rectangle, so what the file states is the <em>outer</em> rectangle — the one the axes' labels
/// are then taken out of — and not the wall. That is why this stands beside
/// <see cref="ChartPlot.PlotArea"/>, which is ODF's inner <c>chart:coordinate-region</c>.
/// </para>
/// <para>
/// <strong>The numbers are 26.2.4.2's own.</strong> `Template Pilot Logbook JAR-FCL V3.0.xls`'s
/// second chart states <c>CHFRAMEPOS(134, 335, 3326, 3412)</c> on a frame that
/// <c>--convert-to fods</c> resolves to 28.609 cm x 14.447 cm, and the same conversion resolves
/// its plot area to <c>svg:x="1.192cm" svg:y="1.418cm" svg:width="23.623cm"
/// svg:height="12.147cm"</c>. Both charts of that workbook agree with the arithmetic below in x
/// and in y; `probes/chart-resid-r99/results.md` §1 has the working.
/// </para>
/// </remarks>
public sealed class ChartStatedOuterPlotAreaTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    /// <summary>The logbook's second chart's frame: 28.609 cm x 14.447 cm.</summary>
    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromMm100(28609), Length.FromMm100(14447));

    private static ChartPlot Bars() => new()
    {
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series = [new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))],
    };

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    /// <summary>
    /// The stated rectangle resolves to the one 26.2.4.2 resolves, to within a hundredth of a
    /// millimetre.
    /// </summary>
    /// <remarks>
    /// <see cref="ChartDrawing.DiagramArea"/> is the outer rectangle the labels come out of, so
    /// it is what the stated rectangle should equal. The plot area itself is inside it by the
    /// value axis' label band, which is what the next case pins.
    /// </remarks>
    [Fact]
    public void TheStatedRectangleResolvesToTheReferencesOwn()
    {
        ChartDrawing drawn = Place(
            Bars() with { OuterPlotAreaUnits = (134, 335, 3326, 3412), ValueAxisVisible = false,
                          CategoryAxisVisible = false });

        drawn.PlotArea.X.Mm100.ShouldBe(1192);
        drawn.PlotArea.Y.Mm100.ShouldBe(1418);
        drawn.PlotArea.Width.Mm100.ShouldBe(23623);
        drawn.PlotArea.Height.Mm100.ShouldBe(12147);
    }

    /// <summary>The wall's right edge is the stated rectangle's, because nothing is on that side.</summary>
    [Fact]
    public void TheWallsRightEdgeIsTheStatedRectanglesRightEdge()
    {
        ChartDrawing drawn = Place(Bars() with { OuterPlotAreaUnits = (134, 335, 3326, 3412) });

        drawn.PlotArea.Right.Mm100.ShouldBe(1192 + 23623);
        drawn.PlotArea.Left.ShouldBeGreaterThan(Length.FromMm100(1192));
    }

    /// <summary>Stating nothing leaves the computed path exactly where it was.</summary>
    [Fact]
    public void StatingNothingLeavesTheComputedPathAlone()
    {
        ChartDrawing stated = Place(Bars() with { OuterPlotAreaUnits = (134, 335, 3326, 3412) });
        ChartDrawing computed = Place(Bars());

        computed.PlotArea.ShouldNotBe(stated.PlotArea);

        // The computed path spreads the diagram across the whole frame less its two per cent;
        // the logbook's chart states a plot area three centimetres narrower than that, which is
        // the whole of why the seat was open.
        computed.PlotArea.Right.ShouldBeGreaterThan(stated.PlotArea.Right);
    }

    /// <summary>
    /// The border gap is taken off each edge before the frame is divided, and added back to the
    /// position — so a rectangle stating the whole 4000 units is <em>wider</em> than the frame.
    /// </summary>
    /// <remarks>
    /// That is <c>CalcHmmFromChartRect</c> (<c>xichart.cxx</c>:320-327) applying
    /// <c>CalcHmmFromChartX</c> to the width as well as to the x, which looks like a slip and is
    /// what the reference does; the two logbook charts measure it.
    /// </remarks>
    [Fact]
    public void TheBorderGapIsAddedToTheWidthAsWellAsToThePosition()
    {
        ChartDrawing drawn = Place(
            Bars() with { OuterPlotAreaUnits = (0, 0, 4000, 4000), ValueAxisVisible = false,
                          CategoryAxisVisible = false });

        drawn.PlotArea.X.Mm100.ShouldBe(250);
        drawn.PlotArea.Y.Mm100.ShouldBe(250);
        drawn.PlotArea.Width.Mm100.ShouldBe(28609 - 500 + 250);
        drawn.PlotArea.Height.Mm100.ShouldBe(14447 - 500 + 250);
    }
}

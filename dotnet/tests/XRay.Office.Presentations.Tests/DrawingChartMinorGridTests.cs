using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// <c>c:minorGridlines</c>: whether it is read, how finely it divides, and what it is painted in.
/// </summary>
/// <remarks>
/// <para>
/// Read from markup literals for the reason <c>DrawingChartPlotLabelTests</c> gives — the shape
/// being tested is the markup's. The literals are <c>Demick_JetBlue.pptx</c>'s chart 1 and
/// <c>N2_E_Maestroni_Swarm_COP.pptx</c>'s chart 1, cut down to the axis.
/// </para>
/// <para>
/// The five is not a guess and not chart2's default of 2:
/// <c>AxisConverter::convertFromModel</c> sets it for an OOXML value axis that states no
/// <c>c:minorUnit</c> (<c>oox/source/drawingml/chart/axisconverter.cxx:405-409</c>,
/// <c>tdf#114168</c>). Confirmed on 26.2.4.2 by measurement rather than by reading: the
/// reference's own page 4 of <c>Demick_JetBlue.pptx</c> has 8 major gridlines 25.97 pt apart and
/// 28 minor ones 5.19 pt apart, and 25.97 / 5.19 = 5.00.
/// </para>
/// </remarks>
public class DrawingChartMinorGridTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Read(string plotArea)
        => DrawingChartPlot.Read(XElement.Parse(
               $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{plotArea}</c:chart></c:chartSpace>"))
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    private static string Bars(string valueAxis, string categoryAxis = "")
        => $"""
            <c:plotArea><c:barChart><c:ser><c:val><c:numRef><c:numCache>
              <c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
            </c:numCache></c:numRef></c:val></c:ser></c:barChart>
            <c:valAx><c:axId val="1"/>{valueAxis}<c:crossAx val="2"/></c:valAx>
            <c:catAx><c:axId val="2"/>{categoryAxis}<c:crossAx val="1"/></c:catAx>
            </c:plotArea>
            """;

    [Fact]
    public void AnAxisThatStatesNoMinorGridlinesHasNone()
    {
        // The control. c:majorGridlines alone must not turn the minor grid on, which is the
        // mistake that would draw a mesh on every chart in the corpus.
        ChartPlot plot = Read(Bars("<c:majorGridlines/>"));

        plot.ValueGrid.ShouldNotBeNull();
        plot.ValueMinorGrid.ShouldBeNull();
        plot.CategoryMinorGrid.ShouldBeNull();
    }

    [Fact]
    public void AStatedMinorGridlinesIsReadOnEitherAxis()
    {
        ChartPlot plot = Read(Bars("<c:minorGridlines/>", "<c:minorGridlines/>"));

        plot.ValueMinorGrid.ShouldNotBeNull();
        plot.CategoryMinorGrid.ShouldNotBeNull();

        // And it is independent of the major grid, which this axis does not state.
        plot.ValueGrid.ShouldBeNull();
    }

    [Fact]
    public void AnExplicitNoFillTurnsTheMinorGridOffRatherThanDrawingItInADefaultColour()
    {
        ChartPlot plot = Read(Bars(
            "<c:minorGridlines><c:spPr><a:ln><a:noFill/></a:ln></c:spPr></c:minorGridlines>"));

        plot.ValueMinorGrid.ShouldBeNull();
    }

    [Fact]
    public void AValueAxisStatingNoMinorUnitDividesEachIntervalIntoFive()
    {
        Read(Bars("<c:minorGridlines/>")).ValueMinorIntervals.ShouldBe(5);
    }

    [Fact]
    public void StatingBothUnitsMakesTheCountTheirQuotient()
    {
        Read(Bars("<c:minorGridlines/><c:majorUnit val=\"10\"/><c:minorUnit val=\"2\"/>"))
            .ValueMinorIntervals.ShouldBe(5);

        Read(Bars("<c:minorGridlines/><c:majorUnit val=\"12\"/><c:minorUnit val=\"3\"/>"))
            .ValueMinorIntervals.ShouldBe(4);
    }

    [Fact]
    public void ALogarithmicAxisStatingAMinorUnitDividesIntoNine()
    {
        Read(Bars(
                "<c:scaling><c:logBase val=\"10\"/></c:scaling><c:minorGridlines/>"
                + "<c:minorUnit val=\"1\"/>"))
            .ValueMinorIntervals.ShouldBe(9);
    }

    [Fact]
    public void TheMinorGridCarriesTheWidthAndDashItStates()
    {
        // N2_E_Maestroni_Swarm_COP.pptx's own minor grid, verbatim. Drawing it solid and
        // hairline instead is 0.66 of that document's unsigned ink.
        ChartGrid grid = Read(Bars(
            "<c:minorGridlines><c:spPr><a:ln w=\"6350\"><a:prstDash val=\"sysDash\"/></a:ln>"
            + "</c:spPr></c:minorGridlines>")).ValueMinorGrid!.Value;

        // `convertEmuToHmm` rounds a DrawingML width to a whole hundredth of a millimetre, so the
        // reference never holds the EMU the file states. `DrawingChartAutoFormat.LineWidth` has
        // the two legs; `probes/stroke-resid-r117/results.md` §3 has the measurements.
        grid.Width.ShouldBe(Length.FromMm100(18));
        grid.Dash.ShouldNotBeNull();
        grid.Dash!.Count.ShouldBeGreaterThan(1);
    }

    [Fact]
    public void TheLayoutDrawsFourLinesBetweenEveryPairOfMajorTicksAndNoneOutsideThem()
    {
        // The minor grid is given a colour of its own purely so the two sets can be told apart
        // here: the two take different automatic entries -- tx1 at tint 75000 and at tint
        // 50000 through the theme's subtle line style -- and this fixture states neither a theme
        // nor a chart style, so both fall to the same last-resort grey and the test asks only
        // about the geometry.  DrawingChartAutoFormat.LineColourOf carries the colours.
        ChartPlot plot = Read(Bars(
            "<c:majorGridlines/><c:minorGridlines><c:spPr><a:ln><a:solidFill>"
            + "<a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr></c:minorGridlines>"));

        ChartDrawing drawing = ChartLayout.Place(
            plot,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300)),
            new Ruler());

        Colour minor = plot.ValueMinorGrid!.Value.Colour;
        Colour major = plot.ValueGrid!.Value.Colour;

        List<double> majorYs = [.. drawing.Lines
            .Where(line => line.Colour == major && line.From.Y == line.To.Y
                           && line.From.X != line.To.X)
            .Select(line => line.From.Y.Points)
            .Distinct()
            .Order()];

        List<double> minorYs = [.. drawing.Lines
            .Where(line => line.Colour == minor && line.From.Y == line.To.Y
                           && line.From.X != line.To.X)
            .Select(line => line.From.Y.Points)
            .Distinct()
            .Order()];

        majorYs.Count.ShouldBeGreaterThan(1);

        // Four between each pair, and nothing beyond the outermost major tick — the reference
        // draws none outside them either.
        minorYs.Count.ShouldBe((majorYs.Count - 1) * 4);
        minorYs[0].ShouldBeGreaterThan(majorYs[0]);
        minorYs[^1].ShouldBeLessThan(majorYs[^1]);
    }

    /// <summary>A measurer with no font behind it; every glyph is half an em wide.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(Length.FromPoints(text.Length * size.Points * 0.5), size);
    }
}

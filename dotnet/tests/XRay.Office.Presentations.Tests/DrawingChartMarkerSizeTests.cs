using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// An OOXML marker's side, which the file states and which is never chart2's unset default.
/// </summary>
/// <remarks>
/// <para>
/// <c>c:marker/c:size</c> is in whole points and <c>TypeGroupConverter::convertMarker</c> makes
/// the symbol <c>convertPointToMm100(nOoxSize)</c> square
/// (<c>oox/source/drawingml/chart/typegroupconverter.cxx:652-654</c>), defaulting to
/// <c>mnMarkerSize( 5 )</c> (<c>seriesmodel.cxx:118</c>). We drew every marker at 0.7 of the
/// label size instead — the transcription of <c>VDataSeries::getSymbolProperties</c>' 250 × 250,
/// which is the default for a symbol <em>nobody set</em> and which no OOXML chart ever keeps.
/// </para>
/// <para>
/// Measured against 26.2.4.2 before this was written.
/// <c>003_advanced_powerpoint_line.pptx</c> states
/// <c>&lt;c:symbol val="circle"/&gt;&lt;c:size val="6"/&gt;</c>; the reference draws its eight
/// markers <strong>6.01 pt</strong> square and we drew them <strong>7.00</strong>. Round 62
/// measured that same pair and attributed it to a legend key, which does not exist on that page —
/// its census' floor had admitted the plot's own data markers. The number was right and the
/// object was not.
/// </para>
/// <para>
/// The conversion is rational and the hundredth is load-bearing: <c>6 × 2540 / 72</c> is 211.67,
/// which rounds to 212 hundredths of a millimetre and back to <b>6.0094 pt</b>, not to 6.
/// </para>
/// </remarks>
public class DrawingChartMarkerSizeTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <param name="marker">The series' whole <c>c:marker</c> element, or null for none.</param>
    private static ChartSeries Read(string? marker)
        => DrawingChartPlot.Read(
               XElement.Parse(
                   $"""
                    <c:chartSpace xmlns:c="{C}" xmlns:a="{A}"><c:chart>
                      <c:plotArea><c:lineChart><c:ser>
                        {marker ?? ""}
                        <c:val><c:numRef><c:numCache>
                          <c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt>
                          <c:pt idx="1"><c:v>2</c:v></c:pt>
                        </c:numCache></c:numRef></c:val>
                      </c:ser></c:lineChart>
                      <c:catAx><c:axId val="2"/><c:crossAx val="1"/></c:catAx>
                      <c:valAx><c:axId val="1"/><c:crossAx val="2"/></c:valAx>
                      </c:plotArea>
                    </c:chart></c:chartSpace>
                    """),
               DrawingTheme.Read(null),
               office2007: false,
               null)
           ?.Series[0]
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    private static string Marker(string symbol, int? size)
        => $"<c:marker><c:symbol val=\"{symbol}\"/>"
           + (size is null ? "" : $"<c:size val=\"{size}\"/>") + "</c:marker>";

    /// <summary>The corpus case, to the hundredth the rounding produces.</summary>
    [Fact]
    public void AStatedSizeIsConvertedThroughHundredthsOfAMillimetre()
        => Read(Marker("circle", 6)).MarkerSize!.Value.Mm100.ShouldBe(212L);

    /// <summary>Five points is the default, and it is not the unset 250.</summary>
    /// <remarks>
    /// The discriminating arm: <c>mnMarkerSize(5)</c> is 176 hundredths of a millimetre where
    /// chart2's unset symbol is 250, so a reader that fell through to the model's fallback and
    /// one that applies the OOXML default give different answers here.
    /// </remarks>
    [Fact]
    public void ASeriesStatingNoSizeIsFivePointsAndNotTheUnsetTwoHundredAndFifty()
    {
        Read(Marker("circle", null)).MarkerSize!.Value.Mm100.ShouldBe(176L);
        Read(Marker("circle", null)).MarkerSize!.Value.Mm100.ShouldNotBe(250L);
    }

    /// <summary>A series with no <c>c:marker</c> at all still takes the OOXML default.</summary>
    /// <remarks>
    /// <c>convertMarker</c> is reached for every series of a chart type that is not a
    /// <c>seriesFrameFormat</c>, whatever the file states — so the absence of the element is not
    /// the absence of a size. The null that <see cref="ChartSeries.MarkerSize"/> documents is for
    /// the ODF and binary readers, which never come through here.
    /// </remarks>
    [Fact]
    public void ASeriesWithNoMarkerElementStillTakesTheOoxmlDefault()
        => Read(null).MarkerSize!.Value.Mm100.ShouldBe(176L);

    /// <summary>The whole stated range, so the conversion is checked at more than one point.</summary>
    /// <remarks>
    /// <c>ST_MarkerSize</c> is 2 … 72. The corpus states 5, 6, 7, 8, 9, 10, 12, 14, 15, 18 and 62,
    /// so the arithmetic is exercised across a factor of twelve rather than at one value.
    /// </remarks>
    [Theory]
    [InlineData(2, 71)]
    [InlineData(5, 176)]
    [InlineData(6, 212)]
    [InlineData(7, 247)]
    [InlineData(14, 494)]
    [InlineData(18, 635)]
    [InlineData(62, 2187)]
    [InlineData(72, 2540)]
    public void EveryStatedSizeRoundsTheSameWay(int points, long mm100)
        => Read(Marker("square", points)).MarkerSize!.Value.Mm100.ShouldBe(mm100);

    /// <summary>Outside the schema's range it is treated as unstated rather than drawn.</summary>
    /// <remarks>
    /// Leniency, per <c>CLAUDE.md</c>'s reading rule: a 0 or a 400 is not a size this can draw,
    /// and falling back to the format's own default is what keeps the rest of the chart.
    /// </remarks>
    [Theory]
    [InlineData(0)]
    [InlineData(1)]
    [InlineData(400)]
    public void AnOutOfRangeSizeFallsBackToTheDefault(int points)
        => Read(Marker("square", points)).MarkerSize!.Value.Mm100.ShouldBe(176L);

    /// <summary>And the symbol it states is still read beside it.</summary>
    /// <remarks>
    /// The control against a change that read the size out of the same element and lost the shape:
    /// round 62's page reading described a *round* reference marker where ours was square, and the
    /// shape was already right — only the size was not.
    /// </remarks>
    [Fact]
    public void TheSymbolIsStillReadBesideTheSize()
    {
        Read(Marker("circle", 6)).Marker.ShouldBe(ChartMarker.Circle);
        Read(Marker("diamond", 6)).Marker.ShouldBe(ChartMarker.Diamond);
        Read(Marker("none", 6)).Marker.ShouldBe(ChartMarker.None);
    }
}

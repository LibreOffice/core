using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// What a marker is painted in: its own colour where the file states one, the series' where it
/// does not.
/// </summary>
/// <remarks>
/// <para>
/// The reader half is in <c>XRay.Office.Presentations.Tests</c>; this is the painter, which is
/// shared with the ODF chart reader and therefore has to leave a marker stating nothing exactly
/// as it was. <see cref="AMarkerStatingNothingIsStillPaintedFromTheSeries"/> and
/// <see cref="ARadarMarkerStatingNothingIsStillPaintedFromTheSeries"/> are that guarantee, and
/// they are why the ODP, ODS and ODT renderings are untouched by construction rather than by a
/// sweep having happened not to notice.
/// </para>
/// <para>
/// Measured against <c>slides/batch-008/pptx/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c> page 15,
/// whose left scatter chart the reference fills ten markers of in <c>70AD47</c> — the colour the
/// marker's own <c>a:ln</c> states — while the series' automatic stroke is a blue. Painting from
/// the series drew all ten in <c>5B9BD5</c>.
/// </para>
/// </remarks>
public class ChartMarkerPaintTests
{
    /// <summary>A measurer with no fonts: half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static readonly Colour SeriesBlue = new(0x5B, 0x9B, 0xD5);
    private static readonly Colour MarkerGreen = new(0x70, 0xAD, 0x47);
    private static readonly Colour MarkerRed = new(0xC0, 0x00, 0x00);

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    private static ChartPlot Scatter(ChartSeries series) => new()
    {
        Kind = ChartPlotKind.Scatter,
        Categories = ["A", "B", "C"],
        Series = [series],
        ValueScale = new ChartScaleRequest(0.0, 40.0, 20.0),
    };

    private static ChartPlot Radar(ChartSeries series) => new()
    {
        Kind = ChartPlotKind.Radar,
        RadarStyle = ChartRadarStyle.Marker,
        Categories = ["A", "B", "C", "D", "E"],
        Series = [series],
        ValueScale = new ChartScaleRequest(0.0, 40.0, 20.0),
    };

    /// <summary>Every colour the drawing fills a shape in, with how many shapes take it.</summary>
    private static Dictionary<Colour, int> Fills(ChartDrawing drawing)
    {
        Dictionary<Colour, int> counted = [];
        foreach (ChartShape shape in drawing.Shapes)
        {
            if (shape.Fill is not { } fill) continue;
            counted[fill] = counted.GetValueOrDefault(fill) + 1;
        }

        return counted;
    }

    [Fact]
    public void AMarkerIsPaintedInItsOwnColourRatherThanTheSeries()
    {
        ChartDrawing drawing = Place(Scatter(
            new ChartSeries("S", [10.0, 20.0, 30.0], null, SeriesBlue)
            {
                Marker = ChartMarker.Circle,
                MarkerFill = MarkerGreen,
                HasLine = false,
            }));

        Fills(drawing).ShouldContainKeyAndValue(MarkerGreen, 3);
        Fills(drawing).ShouldNotContainKey(SeriesBlue);
    }

    [Fact]
    public void AMarkerStatingNothingIsStillPaintedFromTheSeries()
    {
        // The ODF guarantee. Nothing above Core sets these members for an ODF chart, so this is
        // the case every ODP, ODS and ODT rendering takes, and it must be the old behaviour
        // exactly.
        ChartDrawing drawing = Place(Scatter(
            new ChartSeries("S", [10.0, 20.0, 30.0], null, SeriesBlue)
            {
                Marker = ChartMarker.Circle,
                HasLine = false,
            }));

        Fills(drawing).ShouldContainKeyAndValue(SeriesBlue, 3);
    }

    [Fact]
    public void ASeriesFillStillBeatsItsLineWhenTheMarkerStatesNothing()
    {
        // The old precedence, pinned: series.Fill ?? stroke. A marker member of null must not
        // reorder what was already there.
        ChartDrawing drawing = Place(Scatter(
            new ChartSeries("S", [10.0, 20.0, 30.0], MarkerRed, SeriesBlue)
            {
                Marker = ChartMarker.Circle,
                HasLine = false,
            }));

        Fills(drawing).ShouldContainKeyAndValue(MarkerRed, 3);
    }

    [Fact]
    public void AMarkerColourBeatsTheSeriesFillToo()
    {
        ChartDrawing drawing = Place(Scatter(
            new ChartSeries("S", [10.0, 20.0, 30.0], MarkerRed, SeriesBlue)
            {
                Marker = ChartMarker.Circle,
                MarkerFill = MarkerGreen,
                HasLine = false,
            }));

        Fills(drawing).ShouldContainKeyAndValue(MarkerGreen, 3);
        Fills(drawing).ShouldNotContainKey(MarkerRed);
    }

    [Fact]
    public void AMarkerWithNoSeriesColourLeftIsNotDrawnBlack()
    {
        // The coupling, at the painter. A series whose file suppressed its line has no colour to
        // lend, and Colour.Black is what the fallback chain ends in; the marker's own colour is
        // what has to reach the shape instead. This is the FAAAI case exactly.
        ChartDrawing drawing = Place(Scatter(
            new ChartSeries("S", [10.0, 20.0, 30.0])
            {
                Marker = ChartMarker.Circle,
                MarkerFill = MarkerGreen,
                HasLine = false,
            }));

        Fills(drawing).ShouldContainKeyAndValue(MarkerGreen, 3);
        Fills(drawing).ShouldNotContainKey(Colour.Black);
    }

    [Fact]
    public void AStrokedMarkerIsStrokedInTheColourItStates()
    {
        // Cross and Star are the two shapes drawn as a stroke rather than as a fill
        // (ChartLayout.cs:2240-2252), so they are the only ones a marker's own a:ln reaches.
        // Without this case the whole MarkerLine path is unmeasured: a mutation that dropped it
        // at the painter went undetected until this test was added.
        ChartDrawing drawing = Place(Scatter(
            new ChartSeries("S", [10.0, 20.0, 30.0], null, SeriesBlue)
            {
                Marker = ChartMarker.Cross,
                MarkerLine = MarkerGreen,
                HasLine = false,
            }));

        drawing.Shapes.Count(s => s.Line == MarkerGreen).ShouldBe(3);
        drawing.Shapes.ShouldNotContain(s => s.Line == SeriesBlue);
    }

    [Fact]
    public void ARadarMarkerTakesItsOwnColour()
    {
        ChartDrawing drawing = Place(Radar(
            new ChartSeries("S", [10.0, 20.0, 30.0, 20.0, 10.0], null, SeriesBlue)
            {
                Marker = ChartMarker.Circle,
                MarkerFill = MarkerGreen,
            }));

        Fills(drawing).ShouldContainKeyAndValue(MarkerGreen, 5);
    }

    [Fact]
    public void ARadarMarkerStatingNothingIsStillPaintedFromTheSeries()
    {
        ChartDrawing drawing = Place(Radar(
            new ChartSeries("S", [10.0, 20.0, 30.0, 20.0, 10.0], null, SeriesBlue)
            {
                Marker = ChartMarker.Circle,
            }));

        Fills(drawing).ShouldContainKeyAndValue(SeriesBlue, 5);
    }
}

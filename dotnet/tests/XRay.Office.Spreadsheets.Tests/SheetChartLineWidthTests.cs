using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What width a workbook chart's own rules are stroked at, and in what unit.
/// </summary>
/// <remarks>
/// <para>
/// **Every number below is read out of LibreOffice 26.2.4.2's own PDF of
/// <c>sheet-chart-auto-line.xlsx</c>**, which <c>probes/stroke-resid-r117/make-chart-fixtures.py</c>
/// builds: one bar series, one line series with a diamond marker, and a value axis whose
/// <c>c:majorGridlines</c> and whose axis line state no <c>c:spPr</c> at all. Its theme's
/// <c>a:lnStyleLst</c> first entry is <c>w="9525"</c>.
/// </para>
/// <para>
/// Two things were wrong and this pins both. The <em>automatic</em> width comes from that theme
/// entry, and the spreadsheet route handed <c>DrawingChartPlot.Read</c> a null style matrix — so
/// every automatic axis line and gridline in a workbook was stroked at width zero. And a
/// DrawingML width is kept as a whole hundredth of a millimetre, not as its EMU: 9525 EMU is
/// 26.458 of them and the reference keeps 26, which is 0.73701 pt and never 0.75.
/// </para>
/// <para>
/// The reference's own numbers on this fixture are 0.7357 and 2.2354, which are those widths
/// times the chart's fit scale of about 0.998 — the anisotropic squeeze
/// <c>ChartLayout.DrawnExtent</c> models and which is not this seat. The assertions are on the
/// unscaled widths with a tolerance that spans it, and the point of the fixture is the
/// <em>unit</em>: 0.737 rather than 0.75, and not zero.
/// </para>
/// </remarks>
public sealed class SheetChartLineWidthTests
{
    private static IReadOnlyList<DrawnStroke> Strokes(string fixture)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(fixture));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return [.. sink.Pages.SelectMany(page => page.StrokedPaths)];
    }

    [Fact]
    public void AnAutomaticAxisLineTakesTheThemesSubtleLineWidth()
    {
        // 26.2.4.2 draws all 55 of this chart's automatic lines at 0.7357 pt, which is
        // 26/100 mm times the chart's own fit scale. Before the style matrix reached the
        // spreadsheet route every one of them was stroked at zero.
        IReadOnlyList<DrawnStroke> automatic = [.. Strokes("sheet-chart-auto-line.xlsx")
            .Where(stroke => Math.Abs(stroke.Stroke.Width.Points - 0.737) < 0.005)];

        automatic.Count.ShouldBeGreaterThan(10);
    }

    [Fact]
    public void NoneOfTheChartsFurnitureIsStrokedAtZero()
    {
        // The regression this file exists for. Before the theme's format matrix reached the
        // spreadsheet route, every automatic axis line, tick and gridline on this fixture was
        // stroked at width zero — 24 of them. The four markers legitimately stroke at zero, so
        // the assertion is on everything that is not one: nothing in the automatic grey, and
        // nothing in the series' own line colour, may be a hairline.
        IReadOnlyList<DrawnStroke> hairlines = [.. Strokes("sheet-chart-auto-line.xlsx")
            .Where(stroke => stroke.Stroke.Width.Points is > -0.0001 and < 0.0001)];

        hairlines.Count.ShouldBe(4);
        foreach (DrawnStroke hairline in hairlines)
        {
            // A marker is the fixture's `c:size val="7"`, so it is small; a gridline spans the
            // plot. Either dimension being over twenty points means the furniture came back.
            Math.Max(hairline.Bounds.Width.Points, hairline.Bounds.Height.Points)
                .ShouldBeLessThan(20.0);
        }
    }

    [Fact]
    public void AStatedWidthIsKeptAsAWholeHundredthOfAMillimetre()
    {
        // The line series states `a:ln w="28575"` — 2.25 pt, which is 79.375 hundredths of a
        // millimetre. `convertEmuToHmm` rounds it to 79, and 26.2.4.2 strokes the polyline at
        // 2.2354 pt: 79 × 0.0283465 × the same fit scale. 2.25 is not drawn.
        IReadOnlyList<double> widths = [.. Strokes("sheet-chart-auto-line.xlsx")
            .Select(stroke => Math.Round(stroke.Stroke.Width.Points, 4))
            .Distinct()
            .Order()];

        widths.ShouldContain(width => Math.Abs(width - 2.2394) < 0.006);
        widths.ShouldNotContain(width => Math.Abs(width - 2.25) < 0.0005);
    }

    [Fact]
    public void AFilledMarkerIsStrokedInItsOwnFillColour()
    {
        // chart2 gives every symbol a fill and a border and the border is the fill colour:
        // `convertMarker` only reaches for the marker's `a:ln` when there is no fill at all
        // (the tdf#124817 branch). The fixture states an ED7D31 fill and a 203864 outline, and
        // 26.2.4.2 strokes all four markers in ED7D31 with 203864 nowhere on the page.
        IReadOnlyList<DrawnStroke> markers = [.. Strokes("sheet-chart-auto-line.xlsx")
            .Where(stroke => stroke.Stroke.Width.Points is > -0.0001 and < 0.0001
                             || stroke.Stroke.Width.Points < 0.05)];

        markers.Count.ShouldBe(4);
        foreach (DrawnStroke marker in markers)
        {
            SolidPaint paint = marker.Stroke.Paint.ShouldBeOfType<SolidPaint>();
            (paint.Colour.ToArgb() & 0x00FFFFFFu).ShouldBe(0x00ED7D31u);
        }
    }

    [Fact]
    public void ThreeSeriesStatingThreeDifferentThingsGetThreeDifferentWidths()
    {
        // `sheet-chart-series-width.xlsx` is one line chart at `c:style val="18"` over a theme
        // whose subtle line is 9525 EMU, holding three series that differ only in what they say
        // about their own line. 26.2.4.2 resolves them to 0.079, 0.035 and 0.132 cm — read out of
        // its own `--convert-to ods` — and strokes them at 2.23537, 0.99036 and 3.73505 pt.
        //
        //   states `a:ln w="28575"`           -> 79/100 mm, the stated width quantised
        //   states `a:ln` with a fill, no `w` -> 35/100 mm, a constant: raising the theme's
        //                                        subtle line to 38100 moves every gridline from
        //                                        26 to 106 and leaves this at 35
        //   states no `c:spPr` at all         -> 132/100 mm, 9525 EMU times style 18's 500 %
        //
        // A fourth series is a *bar* stating the same `a:ln` with no `w`, and it is 35/100 mm too
        // — so the flat default is not a property of being drawn as a line, even though
        // `spFilledSeriesLines` is AUTOFORMAT_INVISIBLE at style 18 and gives a bar no automatic
        // width at all to replace.
        IReadOnlyList<double> widths = [.. Strokes("sheet-chart-series-width.xlsx")
            .Select(stroke => Math.Round(stroke.Stroke.Width.Points, 4))
            .Distinct()
            .Order()];

        widths.ShouldContain(width => Math.Abs(width - 2.2394) < 0.006);
        widths.ShouldContain(width => Math.Abs(width - 0.9921) < 0.003);
        widths.ShouldContain(width => Math.Abs(width - 3.7417) < 0.010);
    }

    [Fact]
    public void AFilledSeriesStatingALineWithNoWidthGetsTheSameFlatDefault()
    {
        // 26.2.4.2 strokes the four bar outlines at 0.99036 in the 203864 the bar's `a:ln` names,
        // the same 35/100 mm the line series beside them gets.
        IReadOnlyList<DrawnStroke> bars = [.. Strokes("sheet-chart-series-width.xlsx")
            .Where(stroke => stroke.Stroke.Paint is SolidPaint paint
                             && (paint.Colour.ToArgb() & 0x00FFFFFFu) == 0x00203864u)];

        bars.Count.ShouldBe(4);
        foreach (DrawnStroke bar in bars)
        {
            bar.Stroke.Width.Points.ShouldBe(0.9921, 0.003);
        }
    }

    [Fact]
    public void ASeriesStatingALineWithNoWidthDoesNotTakeTheChartStylesMultiplier()
    {
        // The regression this case exists for. Applying the automatic entry to a series that
        // states an `a:ln` draws the middle series at 3.74 pt rather than 0.99 — which is what
        // `064_Small_business_cash_flow`'s alert line came out at, against the reference's 0.99.
        Strokes("sheet-chart-series-width.xlsx")
            .Count(stroke => Math.Abs(stroke.Stroke.Width.Points - 3.7417) < 0.010)
            .ShouldBe(1);
    }
}

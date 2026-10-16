using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// What width a Writer chart's own rules are stroked at.
/// </summary>
/// <remarks>
/// <para>
/// The words half of the same seat the spreadsheets track's
/// <c>SheetChartLineWidthTests</c> pins: <c>DocxPictures.LoadChart</c> handed
/// <c>DrawingChartPlot.Read</c> no style matrix, so an axis line or gridline stating no width
/// of its own was drawn at zero. The fixture is the identical chart part in a Writer document —
/// <c>probes/stroke-resid-r117/make-chart-fixtures.py</c> builds both from one string — so the
/// two routes can be compared directly.
/// </para>
/// <para>
/// **The numbers are LibreOffice 26.2.4.2's own**, from its PDF of
/// <c>words-chart-auto-line.docx</c>: 55 automatic lines at 0.7357 pt, one series polyline at
/// 2.2354, and four markers at zero — the same answers it gives for the workbook, to five
/// figures.
/// </para>
/// </remarks>
public sealed class FrameChartLineWidthTests
{
    private static IReadOnlyList<DrawnStroke> Strokes(string fixture)
    {
        RecordingDrawingSink sink = new();

        using (IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
                   Corpus.Require(fixture)))
        {
            IPageSequence pages = document.Layout();
            for (int at = 0; at < pages.Count; at++) pages[at].Draw(sink);
        }

        return [.. sink.Pages.SelectMany(page => page.StrokedPaths)];
    }

    [Fact]
    public void AnAutomaticAxisLineTakesTheThemesSubtleLineWidth()
    {
        // 9525 EMU in the theme's first `a:lnStyleLst` entry, kept as 26 hundredths of a
        // millimetre: 0.73701 pt, which 26.2.4.2 draws at 0.7357 after the chart's fit scale.
        IReadOnlyList<DrawnStroke> automatic = [.. Strokes("words-chart-auto-line.docx")
            .Where(stroke => Math.Abs(stroke.Stroke.Width.Points - 0.737) < 0.005)];

        automatic.Count.ShouldBeGreaterThan(10);
    }

    [Fact]
    public void NoneOfTheChartsFurnitureIsStrokedAtZero()
    {
        // The regression: every one of those lines used to be a hairline. Only the four markers
        // may be, and a marker is seven points across.
        IReadOnlyList<DrawnStroke> hairlines = [.. Strokes("words-chart-auto-line.docx")
            .Where(stroke => stroke.Stroke.Width.Points is > -0.0001 and < 0.0001)];

        hairlines.Count.ShouldBe(4);
        foreach (DrawnStroke hairline in hairlines)
        {
            Math.Max(hairline.Bounds.Width.Points, hairline.Bounds.Height.Points)
                .ShouldBeLessThan(20.0);
        }
    }
}

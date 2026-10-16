using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What width a cell border is stroked at: the style's own twips, times the sheet's print
/// scale, floored at a tenth of a point.
/// </summary>
/// <remarks>
/// <para>
/// **Every number below is read out of LibreOffice 26.2.4.2's own PDF of the fixture**, not out
/// of the C++ tree, which is 27.2.0.0.alpha0+ here and not the reference binary's source. The
/// fixtures are one border style per cell, thirteen cells, and differ only in the
/// <c>pageSetup/@scale</c> the sheet states; <c>probes/sheet-border-r115/make.py</c> builds them
/// and <c>results.md</c> has the nine-scale sweep they were cut from.
/// </para>
/// <para>
/// The seat is the print scale. Calc gives <c>svx::frame::Style</c> a width in twips times
/// <c>nScaleX</c>, which for a print or a PDF export is the constant twips-to-1/100 mm factor,
/// and the zoom arrives as the fractional scale of the map mode the whole page is drawn through
/// — so the widths are multiplied by it along with the coordinates. This tree scaled the
/// coordinates and drew every border at full size, which is the whole of the flat 0.75 pt
/// recorded as O56.
/// </para>
/// </remarks>
public sealed class SheetBorderWidthTests
{
    private static IReadOnlyList<DrawnStroke> Strokes(string fixture)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(fixture));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return [.. sink.Pages.SelectMany(page => page.StrokedPaths)];
    }

    /// <summary>Every distinct stroke width on the fixture, in points, narrowest first.</summary>
    /// <remarks>
    /// The distinct set rather than a per-cell reading, because which cell edge becomes which
    /// stroke depends on the run coalescing that <see cref="SheetBorderRunTests"/> pins, and
    /// these cases are about the widths alone.
    /// </remarks>
    private static IReadOnlyList<double> Widths(string fixture)
        => [.. Strokes(fixture)
            .Select(stroke => Math.Round(stroke.Stroke.Width.Points, 3))
            .Distinct()
            .Order()];

    [Fact]
    public void TheFourSolidStylesAreTheirStatedTwips()
    {
        // 26.2.4.2 writes 0.75004, 1.75008 and 2.49983 for thin, medium and thick — 15, 35 and
        // 50 twips, unrounded, straight out of the PDF writer's own points conversion.
        IReadOnlyList<double> widths = Widths("sheet-border-widths.xlsx");

        widths.ShouldContain(0.75);
        widths.ShouldContain(1.75);
        widths.ShouldContain(2.5);
    }

    [Fact]
    public void AHairBorderIsFlooredAtATenthOfAPoint()
    {
        // `hair` is one twip, 0.05 pt, and 26.2.4.2 draws it at 0.1 — one pixel of the PDF
        // export's 720 dpi device (`PDFPage::appendLineInfo`, vcl/source/pdf/PDFPage.cxx:497-505).
        // At 400 per cent the same border is drawn at 0.19956, so it is a floor and not a
        // constant.
        Widths("sheet-border-widths.xlsx").ShouldContain(0.1);
    }

    [Fact]
    public void APatternedBorderGoesThroughAWholeHundredthOfAMillimetre()
    {
        // A dashed style reaches the metafile as a LineInfo whose width is rounded to a whole
        // logic unit where a solid one keeps its double, so a dotted `thin` is 26 rather than
        // 26.46 hundredths of a millimetre. 26.2.4.2: 0.737 against a solid 0.75004, and
        // 1.75745 against 1.75008.
        IReadOnlyList<double> widths = Widths("sheet-border-widths.xlsx");

        widths.ShouldContain(width => Math.Abs(width - 0.737) < 0.002);
        widths.ShouldContain(width => Math.Abs(width - 1.757) < 0.002);
    }

    [Fact]
    public void ASpreadsheetMlDoubleRuleIsTenFifteenTen()
    {
        // `Border::convertBorderLine` calls lclSetBorderLineWidth(rBorderLine, 10, 15, 10) for
        // `double`, so the whole rule is 1.75 pt and not the 2.5 a third-of-thick reading gives.
        // 26.2.4.2 strokes the two lines at 0.50002 each with their centres 1.248 pt apart.
        IReadOnlyList<DrawnStroke> pair = [.. Strokes("sheet-border-widths.xlsx")
            .Where(stroke => Math.Abs(stroke.Stroke.Width.Points - 0.5) < 0.01)
            .Where(stroke => stroke.Bounds.Width > stroke.Bounds.Height)
            .OrderBy(stroke => stroke.Bounds.Y.Points)];

        pair.Count.ShouldBeGreaterThanOrEqualTo(2);
        (pair[1].Bounds.Y.Points - pair[0].Bounds.Y.Points).ShouldBe(1.248, 0.02);
    }

    [Fact]
    public void ABiffDoubleRuleIsTenThirtyTen()
    {
        // BIFF states one width and the style name, so the split is DOUBLE_THIN's own
        // BorderWidthImpl — the two lines pinned at 10 twips and the distance taking the rest of
        // the 50. 26.2.4.2 on the same workbook saved as .xls: 0.50002 pt lines 1.984 pt apart,
        // against 1.248 as .xlsx.
        IReadOnlyList<DrawnStroke> pair = [.. Strokes("sheet-border-widths.xls")
            .Where(stroke => Math.Abs(stroke.Stroke.Width.Points - 0.5) < 0.01)
            .Where(stroke => stroke.Bounds.Width > stroke.Bounds.Height)
            .OrderBy(stroke => stroke.Bounds.Y.Points)];

        pair.Count.ShouldBeGreaterThanOrEqualTo(2);
        (pair[1].Bounds.Y.Points - pair[0].Bounds.Y.Points).ShouldBe(1.984, 0.02);
    }

    [Fact]
    public void EveryWidthTakesThePrintScale()
    {
        // The same thirteen cells at `pageSetup scale="42"`. 26.2.4.2 writes 0.31503, 0.73508 and
        // 1.04999 for thin, medium and thick, 0.21002 for a double's lines, and 0.30956 and
        // 0.73817 for the two patterned widths — every one of them the unscaled width times 0.42.
        IReadOnlyList<double> widths = Widths("sheet-border-widths-scaled.xlsx");

        widths.ShouldContain(width => Math.Abs(width - 0.315) < 0.002);
        widths.ShouldContain(width => Math.Abs(width - 0.735) < 0.002);
        widths.ShouldContain(width => Math.Abs(width - 1.05) < 0.002);
        widths.ShouldContain(width => Math.Abs(width - 0.21) < 0.002);
        widths.ShouldContain(width => Math.Abs(width - 0.3096) < 0.002);
        widths.ShouldContain(width => Math.Abs(width - 0.7382) < 0.002);
    }

    [Fact]
    public void TheFloorSurvivesThePrintScale()
    {
        // `hair` at 42 per cent is 0.021 pt and 26.2.4.2 still draws it at 0.1, which is what
        // makes the rule a floor on the drawn width rather than a minimum stated width.
        Widths("sheet-border-widths-scaled.xlsx").ShouldContain(0.1);
    }

    [Fact]
    public void NothingIsDrawnAtTheUnscaledWidthOnAScaledSheet()
    {
        // The regression this file exists for: before round 115 every one of these was 0.75,
        // 1.75 or 2.5 whatever the sheet's scale.
        IReadOnlyList<double> widths = Widths("sheet-border-widths-scaled.xlsx");

        widths.ShouldNotContain(0.75);
        widths.ShouldNotContain(1.75);
        widths.ShouldNotContain(2.5);
    }
}

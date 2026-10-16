using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A header or footer band reaches the paper at the sheet's <em>print scale</em>, and the four
/// page margins do not.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScPrintFunc::GetDocPageSize</c> builds the page rectangle in <em>document twips</em>:
/// </para>
/// <code>
/// aPageRect.SetTop( ( aPageRect.Top() + nTopMargin ) * 100 / nZoom + aHdr.nHeight );
/// </code>
/// <para>
/// (<c>sc/source/ui/view/printfun.cxx:3002-3003</c>.) Each margin is divided by the zoom and each
/// band is added whole. A document twip reaches the paper at <c>zoom/100</c> of a physical twip,
/// because the map mode the page is drawn through carries the zoom as its scale fraction
/// (<c>ScPrintFunc::InitModes</c>, <c>printfun.cxx:2645</c>) — so the margin comes back out at
/// full size and <strong>the band arrives at <c>nHeight × zoom/100</c></strong>.
/// </para>
/// <para>
/// This is round 56's 18.46 pt body offset, whose brief guessed the header height was being
/// counted twice. It is not: <c>fm-provider-service-measures</c> p36 is a
/// <c>fitToHeight="17"</c> sheet whose band is 35.45 pt at a zoom of about 48 %, and
/// <c>FY2023-AIP-grants</c> p1 is a <c>scale="43"</c> sheet whose band is pinned at 32.4 —
/// <c>H × (1 − zoom)</c> is 18.5 and 18.47 against the 18.46 and 18.49 measured. A band counted
/// twice would have given the same number on both, and would not have moved with the scale.
/// </para>
/// <para>
/// <c>SheetPagination.DocPageSize</c> has ported the same arithmetic since it was written, which
/// is why page counts were never wrong; what implemented the opposite was
/// <see cref="SheetPrintSetup.PrintableAreaAt"/>, which is what <em>places</em> what a page holds.
/// The band's own <em>text</em> was already drawn at the zoom, which is why a scaled sheet's
/// header was the right size over a body that was not in the right place.
/// </para>
/// <para>
/// The fixture is <c>probes/sheets-r57/make-band-scale-fixture.py</c>'s: letter portrait, a
/// workbook default of Liberation Sans 11, one 14 pt header line over a 32.4 pt stated band, and
/// three worksheets asking one question each. Every expectation below is read off LibreOffice
/// 26.2.4.2's own PDF of it — the body token's ink box tops out at <strong>56.179</strong> pt on
/// the unscaled sheet, <strong>35.455</strong> at 40 % and <strong>38.010</strong> on the pinned
/// band at 50 %. Taking the band at full size puts all three at about 56.2.
/// </para>
/// </remarks>
public sealed class SheetBandScaleTests
{
    private static IReadOnlyList<IReadOnlyList<DrawnGlyphRun>> PageRuns()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-band-scale-xlsx.xlsx"));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return [.. sink.Pages.Select(page => (IReadOnlyList<DrawnGlyphRun>)page.Runs)];
    }

    private static DrawnGlyphRun Run(string text)
        => PageRuns().SelectMany(page => page).Single(run => run.Text == text);

    /// <summary>
    /// The control, and it runs first: an unscaled sheet is untouched by any of this.
    /// </summary>
    /// <remarks>
    /// 56.179 as an ink-box top in the reference, and 11 pt Liberation Sans puts 9.96 pt of
    /// ascent over its baseline, so 66.13. At a scale of 1.0 every term in
    /// <see cref="SheetPrintSetup.PrintableAreaAt"/> is the term it was before this rule existed,
    /// so a change that moves this one is measuring something else.
    /// </remarks>
    [Fact]
    public void AnUnscaledSheetPutsItsBodyBelowTheWholeBand()
    {
        Run("ZZBODY1ZZ").Origin.Y.Points.ShouldBeInRange(65.6, 66.7);
    }

    /// <summary>A sheet printed at 40 % reserves 40 % of its band, not all of it.</summary>
    /// <remarks>
    /// The band is 32.4 pt stated plus the 2.10 pt by which a 14 pt line's real height exceeds
    /// its bare point size, so 34.50; at 40 % the body starts at
    /// <c>21.6 + 13.80 = 35.40</c> and the reference's ink box tops out at 35.455. Reserving the
    /// whole band puts it at 56.1. Baselines rather than ink tops here, so the expectation is
    /// <c>35.455 + 4.40 × 0.905 = 39.44</c> against the control's 66.13 — the two together are
    /// what discriminate, and neither does it alone.
    /// </remarks>
    [Fact]
    public void AScaledSheetReservesOnlyTheScaledPartOfItsBand()
    {
        Run("ZZBODY2ZZ").Origin.Y.Points.ShouldBeInRange(39.0, 39.9);
    }

    /// <summary>
    /// And a <em>pinned</em> band scales exactly as a dynamic one does.
    /// </summary>
    /// <remarks>
    /// `Pinned` states three 11 pt header lines in a 32.4 pt band, so the nominal height is 33
    /// and the filter pins the band rather than letting it grow
    /// (<c>pagesettings.cxx:1032</c>). The band is therefore exactly 32.4 and, at 50 %, the body
    /// starts at <c>21.6 + 16.2 = 37.8</c>; the reference's ink box tops out at 38.010, so the
    /// baseline is <c>38.010 + 5.50 × 0.905 = 42.99</c>. This separates the two arms of
    /// <see cref="SheetBandHeight"/>: a rule that scaled only the <em>growth</em> would leave
    /// this sheet where it was.
    /// </remarks>
    [Fact]
    public void APinnedBandScalesLikeADynamicOne()
    {
        Run("ZZBODY3ZZ").Origin.Y.Points.ShouldBeInRange(42.5, 43.4);
    }

    /// <summary>
    /// The band's own text is unaffected, on the scaled sheet as much as on the unscaled one.
    /// </summary>
    /// <remarks>
    /// Drawn at <c>14 × 0.40 = 5.6</c> pt, which the reference's PDF confirms, and hard against
    /// the band's top at the <c>header</c> margin on both. This is the half that was already
    /// right, and it is asserted so that a change to the body's origin cannot be made by moving
    /// the band instead.
    /// </remarks>
    [Fact]
    public void TheBandsOwnTextIsDrawnAtTheScaleAndAtTheHeaderMargin()
    {
        IReadOnlyList<IReadOnlyList<DrawnGlyphRun>> pages = PageRuns();

        DrawnGlyphRun unscaled = pages[0].Single(run => run.Text == "ZZTOPZZ");
        DrawnGlyphRun scaled = pages[1].Single(run => run.Text == "ZZTOPZZ");

        unscaled.Run.FontSize.Points.ShouldBe(14.0, 0.05);
        scaled.Run.FontSize.Points.ShouldBe(5.6, 0.05);

        // Both sit on the header margin, 0.3 in, plus their own ascent — which is 40 % as much on
        // the scaled sheet, so the two baselines are 21.6 + 12.7 and 21.6 + 5.1.
        unscaled.Origin.Y.Points.ShouldBeInRange(33.5, 35.0);
        scaled.Origin.Y.Points.ShouldBeInRange(26.0, 27.5);
    }

    /// <summary>
    /// The rectangle itself: the bands scale and the margins do not.
    /// </summary>
    /// <remarks>
    /// The unit under the three drawing tests above, asserted separately because the arithmetic
    /// is where the asymmetry lives and a reader of the drawing tests cannot see it. A rule that
    /// scaled the margins too would move the 40 % case by a further 13 pt.
    /// </remarks>
    [Fact]
    public void ThePrintableAreaScalesTheBandsAndNotTheMargins()
    {
        SheetPrintSetup setup = new()
        {
            TopMargin = Length.FromPoints(20),
            BottomMargin = Length.FromPoints(30),
            HeaderHeight = Length.FromPoints(40),
            FooterHeight = Length.FromPoints(60),
            PageSize = new DocSize(Length.FromPoints(500), Length.FromPoints(800)),
        };

        DocRect whole = setup.PrintableAreaAt(1.0);
        whole.Y.Points.ShouldBe(60, 0.001);
        whole.Height.Points.ShouldBe(650, 0.001);

        DocRect half = setup.PrintableAreaAt(0.5);
        half.Y.Points.ShouldBe(40, 0.001);
        half.Height.Points.ShouldBe(700, 0.001);
    }
}

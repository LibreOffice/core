using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A band's clip rectangle is as tall as the band at the print scale, and it cuts the ink that
/// falls outside it rather than letting it stand.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScPrintFunc::PrintHF</c> sets one clip region before it draws a band's three areas
/// (<c>sc/source/ui/view/printfun.cxx:1870</c>). <see cref="SheetBandFaceAndClipTests"/> pins what
/// that rectangle decides about drawing an area <em>at all</em>; this pins the rectangle itself and
/// the middle branch of <c>ImpEditEngine::DrawText_ToPosition</c>
/// (<c>editeng/source/editeng/impedit3.cxx:3367-3389</c>), where an area that overlaps the clip only
/// partly is embedded in a <c>MaskPrimitive2D</c> and cut.
/// </para>
/// <para>
/// <strong>The rectangle is scaled vertically and not horizontally.</strong> <c>aPageRect</c> is in
/// document twips, so <c>nLineWidth</c> comes off it and arrives at full size while
/// <c>nHeight - nDistance</c> is added whole and arrives at <c>nHeight × zoom</c>
/// (<c>ScPrintFunc::GetDocPageSize</c>, <c>printfun.cxx:3002</c>, over the map mode's zoom fraction
/// at <c>:2645</c>) — the same asymmetry <see cref="SheetPrintSetup.PrintableAreaAt"/> carries for
/// the body.
/// </para>
/// <para>
/// <c>sheet-band-scale-clip.xlsx</c> is authored for this: letter landscape at a print scale of
/// <strong>43 %</strong>, margins of 0.75 in top and 0.3 in header — so a 32.4 pt band — and a
/// three-line centred header that needs about 17.3 pt of it. Every figure below is read off the
/// two installed references' own content streams, which agree with each other:
/// <c>17.995 576.503 755.94 13.910 re W* n</c> at 24.2.7.2 and
/// <c>17.996 576.504 755.897 13.889</c> at 26.2.4.2, both of which are
/// <c>32.4 × 0.4293 = 13.91</c> tall and <c>774 - 18 = 756</c> wide. Both draw
/// <c>HEADERTHREE</c> and both cut it part way down its glyphs;
/// <c>pdftotext</c> reads all three lines off either page.
/// </para>
/// </remarks>
public sealed class SheetBandScaleClipTests
{
    private static PlacedDrawingSink Draw()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-band-scale-clip.xlsx"));

        PlacedDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return sink;
    }

    /// <summary>The band-wide clips: the ones that span the page between its two margins.</summary>
    private static IReadOnlyList<DocRect> BandClips(PlacedDrawingSink sink)
        => [.. sink.Clips.Where(clip => clip.Width.Points > 400)];

    [Fact]
    public void TheHeaderBandIsClippedToItsOwnHeightAtThePrintScale()
    {
        DocRect clip = BandClips(Draw()).ShouldHaveSingleItem();

        // 21.587 and 21.607 in the two references; the band's top is the header margin, unscaled.
        clip.Y.Points.ShouldBeInRange(21.4, 21.8);

        // 13.910 and 13.889 in the two references. Taking the band at full size gives 32.4 and
        // clips nothing at all, which is what this file caught.
        clip.Height.Points.ShouldBeInRange(13.6, 14.2);
    }

    [Fact]
    public void TheHeaderBandsClipIsAsWideAsTheBandAtFullSize()
    {
        // 755.94 and 755.897 in the two references — the page between its 18 pt margins, with the
        // print scale applied to the height and not to this.
        BandClips(Draw()).ShouldHaveSingleItem().Width.Points.ShouldBeInRange(750.0, 760.0);
    }

    [Fact]
    public void TheLineTheBandCannotHoldIsStillInTheTextLayer()
    {
        // `ClipPathKeepingText`, not `ClipPath`: the reference cuts the same ink and `pdftotext`
        // still reads all three lines off its page.
        IEnumerable<string> drawn = Draw().Runs.Select(run => run.Run.Text);
        drawn.ShouldContain("HEADERTHREE");
    }

    [Fact]
    public void TheBodyIsNotClippedByTheBand()
    {
        // The control on the clip's balance: a `Save` that is not restored would put the body's
        // own cell inside the header's rectangle, and the page would print blank below the band.
        PlacedDrawingSink sink = Draw();
        sink.Runs.Select(run => run.Run.Text).ShouldContain("BODYCELL");

        (_, DocPoint origin) = sink.Runs.Single(run => run.Run.Text == "BODYCELL");
        origin.Y.Points.ShouldBeGreaterThan(BandClips(sink)[0].Bottom.Points);
    }
}

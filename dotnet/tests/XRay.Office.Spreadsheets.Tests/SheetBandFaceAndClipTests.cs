using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A header or footer band is a clip rectangle, and its text is drawn in the workbook's own
/// default cell font.
/// </summary>
/// <remarks>
/// <para>
/// Two rules, both measured on LibreOffice <strong>26.2.4.2</strong>, both in
/// <c>SheetPageDecoration.DrawBand</c>, and both found by the same probe
/// (<c>probes/sheets-r56/probe-bandclip.py</c>).
/// </para>
/// <para>
/// <strong>The clip.</strong> <c>ScPrintFunc::PrintHF</c> sets a clip region of exactly
/// <c>Rectangle(aStart, Size(nLineWidth, nHeight - nDistance))</c>
/// (<c>sc/source/ui/view/printfun.cxx:1870</c>) and then draws the left, centre and right areas
/// into it as three separate pieces of text. <c>ImpEditEngine::DrawText_ToPosition</c> takes each
/// area's whole primitive range and returns having emitted <em>nothing</em> when that range
/// misses the clip (<c>editeng/source/editeng/impedit3.cxx:3367-3379</c>); when it overlaps only
/// partly it wraps the area in a <c>MaskPrimitive2D</c> of the clip polygon (<c>:3380-3389</c>),
/// which keeps every line in the primitive tree and <em>cuts</em> all of them when they are
/// rendered. So the decision to draw at all is <em>per area</em> and never per line — which is
/// what <see cref="AnAreaWhoseInkIsInsideTheBandIsStillDrawnBesideOneThatIsNot"/> pins — and the
/// cut that follows it is geometric, which is what <see cref="SheetBandScaleClipTests"/> pins.
/// </para>
/// <para>
/// This is what round 55 recorded as an unexplained "text-fit threshold, about 0.27x the point
/// size". There is no threshold: the apparent one is <c>ascent - inkAscent</c>, the distance
/// from a line's top to the top of its ink. A bisection in 0.1 pt steps at three sizes puts it at
/// 0.2056 to 0.2087 em, and the corpus case a threshold could never have produced is this
/// fixture's: a band far wider than any bracket whose ink is pushed clean out of it by empty
/// leading lines. <c>FAA-2019-0995-0002_attachment_2.xlsx</c> is that case — a 5.67 pt band,
/// seven empty lines, then <c>PAGE </c> and <c>&amp;P OF &amp;N</c> at 9 pt — and it drew twenty
/// tokens across five pages that the reference does not have.
/// </para>
/// <para>
/// <strong>The face.</strong> <c>ScPrintFunc::MakeEditEngine</c> fills the band's EditEngine
/// defaults from <c>getDefaultCellAttribute</c> and overrides only the height <em>unit</em>
/// (<c>printfun.cxx:1769-1774</c>), so a band naming no face of its own is drawn in whatever a
/// plain cell of that workbook would be. <see cref="SheetBandHeight"/> has sized bands that way
/// since it was written; the drawing used a fixed ten-point Liberation Sans until round 56, so
/// the two halves of the same band disagreed on all 81 corpus workbooks that state band content.
/// </para>
/// <para>
/// The fixture is authored by <c>probes/sheets-r56/make-band-clip-fixture.py</c> — letter
/// portrait, a workbook default of <strong>Times New Roman 14</strong> so that both the family
/// and the size are observable at once, and two worksheets asking one question each. Every
/// expectation below is read off LibreOffice's own PDF of it:
/// </para>
/// <list type="bullet">
/// <item><c>Areas</c>, a 7.2 pt band: <c>KEEPLEFT</c>'s ink box tops out at
/// <strong>21.576</strong> pt against a band top of 21.60, and <c>DROPRIGHT</c> is nowhere in the
/// document.</item>
/// <item><c>Sliver</c> and <c>Slice</c>, one 14 pt line in a <strong>2.0 pt</strong> and a
/// <strong>4.0 pt</strong> band: the reference draws <c>WIDEBAND</c> and not <c>THINBAND</c>,
/// which is the pair that pins where a line's ink begins.</item>
/// <item><c>Face</c>, a roomy footer: <c>FACECODE</c> at x 50.400 in
/// <c>LiberationMono</c>, <c>PLAINFACE</c> at 267.500 in <c>LiberationSerif</c>, and
/// <c>BIGFACE</c> at 458.900 spanning 27 pt of height for its <c>&amp;24</c>. The reference's PDF
/// embeds exactly <c>LiberationSerif</c> and <c>LiberationMono</c> and no sans face at all.</item>
/// </list>
/// </remarks>
public sealed class SheetBandFaceAndClipTests
{
    private static IReadOnlyList<DrawnGlyphRun> Runs()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-band-clip-xlsx.xlsx"));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return [.. sink.Pages.SelectMany(page => page.Runs)];
    }

    private static DrawnGlyphRun Run(string text) => Runs().Single(run => run.Text == text);

    [Fact]
    public void AnAreaWhoseInkFallsBelowItsBandIsNotDrawnAtAll()
    {
        // Seven empty lines put this area's only ink about 110 pt below a band 7.2 pt tall, and
        // the reference's PDF holds no `DROPRIGHT` on any page.
        Runs().Select(run => run.Text).ShouldNotContain("DROPRIGHT");
    }

    [Fact]
    public void AnAreaWhoseInkIsInsideTheBandIsStillDrawnBesideOneThatIsNot()
    {
        // The discriminator between clipping per area and clipping per band. Both areas share one
        // rectangle and only one of them misses it.
        Runs().Select(run => run.Text).ShouldContain("KEEPLEFT");
    }

    [Fact]
    public void AShortAreaInAPinnedBandStartsAtTheBandsOwnTop()
    {
        // 21.576 as an ink-box top in the reference, and 14 pt Liberation Serif puts about
        // 12.7 pt of ascent over its baseline, so about 34.3. Centring this area in the *other*
        // area's 124 pt — which is what a band flagged dynamic did before the paper height was
        // clamped to the rectangle — puts it 54 pt lower, and the clip then deletes it.
        Run("KEEPLEFT").Origin.Y.Points.ShouldBeInRange(33.0, 35.5);
    }

    [Fact]
    public void ABandNamingNoFaceTakesTheWorkbooksDefaultSize()
    {
        // Fourteen point, from `<font><sz val="14"/>`, and not `SheetBandText.DefaultSize`.
        Run("PLAINFACE").Run.FontSize.Points.ShouldBe(14.0, 0.001);
    }

    [Fact]
    public void ABandNamingNoFaceTakesTheWorkbooksDefaultFamily()
    {
        // Times New Roman resolves to Liberation Serif here, which is what the reference's PDF
        // embeds; it embeds no sans face at all.
        Run("PLAINFACE").Run.Font.FamilyName.ShouldBe("Liberation Serif");
    }

    [Fact]
    public void AFaceCodeNamesTheFamilyItsOwnRunIsDrawnIn()
    {
        // `&"Courier New"` — which `SheetHeaderFooter.ParseCodes` read and threw away until
        // round 56, while `SheetBandHeight` read the same code to size the band.
        Run("FACECODE").Run.Font.FamilyName.ShouldBe("Liberation Mono");
    }

    [Fact]
    public void ASizeCodeStillBeatsTheWorkbooksDefault()
    {
        // `&24` against a workbook default of 14, and the family is still the workbook's.
        Run("BIGFACE").Run.FontSize.Points.ShouldBe(24.0, 0.001);
        Run("BIGFACE").Run.Font.FamilyName.ShouldBe("Liberation Serif");
    }

    [Fact]
    public void TheWorkbooksDefaultFontReachesTheBandThroughThePrintSetup()
    {
        // **The wiring, and it is here because the mutation that blanks it breaks none of the
        // tests above on their own account.** Everything else in this file goes through
        // `SheetPageDecoration`, which falls back to `SheetDefaultFont.Calc` when no band font is
        // set — so a reader that stopped setting `BandFont` would put every assertion above back
        // on ten-point Liberation Sans, and only this one names the reader.
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-band-clip-xlsx.xlsx"));

        SheetPrintSetup setup = ((SpreadsheetPages)document.Layout()).Sheets[0].Setup;

        setup.BandFont.ShouldNotBeNull();
        setup.BandFont.Size.Points.ShouldBe(14.0, 0.001);
        setup.BandFont.Family.ShouldBe("Times New Roman");
    }

    [Fact]
    public void ABandNarrowerThanTheDistanceToTheInkDrawsNothing()
    {
        // `Sliver`: a 2.0 pt band with one 14 pt line, whose ink starts about 2.9 pt below the
        // line's top. The reference's page 3 holds no `THINBAND`.
        //
        // **This is the pair that pins what "ink" means**, and neither half does it alone.
        // Taking the line box's top instead would draw this one; taking the whole line height
        // would drop `WIDEBAND` as well.
        Runs().Select(run => run.Text).ShouldNotContain("THINBAND");
    }

    [Fact]
    public void ABandWiderThanThatDistanceDrawsEverything()
    {
        // `Slice`: the same line in a 4.0 pt band, which the reference draws with its ink box
        // topping out at 21.576 against a band top of 21.60 — hard against it, because a band
        // this short centres nothing.
        Run("WIDEBAND").Origin.Y.Points.ShouldBeInRange(33.0, 35.5);
    }

    [Fact]
    public void TheInkOfALineStartsWhereTheReferenceStartsDrawing()
    {
        // The measured bracket, in the units the rule is written in. 26.2.4.2 begins to draw a
        // one-line band between 1.59 and 1.70 pt at 8 pt, between 2.21 and 2.30 at 11 pt and
        // between 4.11 and 4.20 at 20 pt — the mm100 rounding the margins go through is what
        // makes those brackets rather than points. `ascent - capHeight` has to land inside each.
        foreach ((double size, double low, double high) in
                 ((double, double, double)[])[(8, 1.59, 1.70), (11, 2.21, 2.30), (20, 4.11, 4.20)])
        {
            Length em = Length.FromPoints(size);
            Length inkTop = SheetBandText.AscentAt(em, null) - SheetBandText.CapHeightAt(em, null);

            // Biased towards drawing rather than centred in the bracket: being wrong the other
            // way deletes a header. See `SheetBandText.RoundCapitalOvershoot`.
            inkTop.Points.ShouldBeLessThan(high);
            inkTop.Points.ShouldBeGreaterThan(low - 0.25);
        }
    }
}

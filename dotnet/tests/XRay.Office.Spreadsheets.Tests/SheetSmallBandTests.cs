using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A SpreadsheetML header or footer band prints whenever its margins leave it any room at all,
/// starts at the band's own top edge when its text does not fit, and prints nothing when the two
/// margins are equal.
/// </summary>
/// <remarks>
/// <para>
/// Three separate rules, and each was wrong in a different way. The first is the one that cost
/// words: <c>XlsxPrintSetup</c> and <c>XlsbPrintSetup</c> set no band gap, so both inherited
/// <see cref="SheetPrintSetup"/>'s ODF default of <strong>142 twips</strong>;
/// <c>SheetPageDecoration.DrawBand</c> lays text into <c>bandHeight - gap</c> and returns on a
/// negative rectangle, so <em>every</em> band under 7.1 pt was dropped outright — no ink and no
/// words. Calc's own distance is <c>max(0, statedBand - nominal)</c>
/// (<c>sc/source/filter/oox/pagesettings.cxx:1029-1041</c>), which is nothing on a band that was
/// already too short; <c>XlsPrintSetup</c> has had that rule since it was written and the other
/// two readers simply never called it.
/// </para>
/// <para>
/// The fixture is authored to the three shapes rather than copied, one worksheet each, letter
/// portrait, a 0.5 in top margin and no header:
/// </para>
/// <list type="bullet">
/// <item><c>Pinned</c> — 0.30 in bottom against a 0.25 in footer margin, so the stated band is
/// <strong>0.05 in = 3.6 pt</strong> and its 9 pt text cannot fit. Its footer also carries both
/// spellings of <c>&amp;K</c> at once.</item>
/// <item><c>Zero</c> — 0.30 in against 0.30 in, a stated band of exactly nothing.</item>
/// <item><c>Roomy</c> — 0.60 in against 0.25 in, a stated band of 25.2 pt, which fits.</item>
/// <item><c>Spill</c> — the pinned band again, but with a footer of <em>three</em> 9 pt lines.
/// Two of them fit between the band's top at 770.4 pt and the 792 pt page edge; the third has its
/// baseline past the paper and neither side's PDF holds it.</item>
/// <item><c>Snug</c> — 0.30 in against 0.10 in, a stated band of 14.4 pt. This one exists to
/// catch the <em>over-general</em> version of the clamp and nothing else: it fits, but only just,
/// and a clamp taken against the text rectangle's top rather than the band's own edge moves it
/// 2.2 pt. A probe caught that before this fixture did.</item>
/// </list>
/// <para>
/// Every expectation below is read off LibreOffice <strong>26.2.4.2</strong>'s own PDF of the
/// fixture: <c>LEFTPIN</c> and <c>RIGHTPIN</c> at y 770.355, no <c>ZEROBAND</c> anywhere, and
/// <c>ROOMYBAND</c> at y 762.800 and <c>SNUGBAND</c> at 773.600. 770.4 pt is the pinned band's
/// top edge on a 792 pt page
/// (<c>792 - 0.30 in</c>) to a twentieth of a point, which is what pins the second rule.
/// </para>
/// <para>
/// <strong>The zero band is a reversal.</strong> <c>SheetPageDecoration</c> used to say a band of
/// no height is still drawn, measured on 24.2.7.2 against
/// <c>2012-GA-Survey-Chapter-6-Tables-16Dec2013-V2.xls</c>. On 26.2.4.2 that document's reference
/// PDF holds no footer at all while ours held four <c>Page 6 - N</c> lines. The mechanism the old
/// note identified survived the version move; the behaviour attached to it did not.
/// </para>
/// </remarks>
public sealed class SheetSmallBandTests
{
    private static IReadOnlyList<DrawnPage> Draw()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-small-band-xlsx.xlsx"));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return sink.Pages;
    }

    private static IEnumerable<DrawnGlyphRun> Runs()
        => Draw().SelectMany(page => page.Runs);

    [Fact]
    public void AFooterBandTooShortForItsTextIsStillDrawn()
    {
        IReadOnlyList<DrawnGlyphRun> runs = [.. Runs()];

        runs.Select(run => run.Text).ShouldContain("LEFTPIN");
        runs.Select(run => run.Text).ShouldContain("RIGHTPIN");
    }

    [Fact]
    public void AFooterBandTooShortForItsTextStartsAtTheBandsOwnTopEdge()
    {
        DrawnGlyphRun pinned = Runs().Single(run => run.Text == "LEFTPIN");

        // `Origin` is the baseline; the reference's figures below are `pdftotext -bbox` ink-box
        // tops, which sit one ascent above it. The band's top edge is 792 - 0.30 in = 770.4 pt,
        // the reference's ink box tops out at 770.355, and 9 pt Liberation Sans puts 8.15 pt of
        // ascent over its baseline — so 778.5. Bottom-aligning the text on the footer margin
        // instead, which is right for a band that fits, puts it 7.5 pt lower still.
        pinned.Origin.Y.Points.ShouldBeInRange(777.5, 779.5);
    }

    [Fact]
    public void AFooterBandWhoseMarginsAreEqualIsNotDrawnAtAll()
        => Runs().Select(run => run.Text).ShouldNotContain("ZEROBAND");

    [Fact]
    public void AFooterBandWithRoomToSpareStillSitsOnItsFooterMargin()
    {
        DrawnGlyphRun roomy = Runs().Single(run => run.Text == "ROOMYBAND");

        // 762.800 as an ink-box top in the reference, so about 771.8 as a baseline for this
        // 10 pt line. This is the case that already worked, and it is here because the clamp that
        // fixes the pinned band must not reach it.
        roomy.Origin.Y.Points.ShouldBeInRange(770.8, 772.8);
    }

    [Fact]
    public void ABandThatOnlyJustFitsIsNotLiftedToItsBandTop()
    {
        DrawnGlyphRun snug = Runs().Single(run => run.Text == "SNUGBAND");

        // 773.600 as an ink-box top in the reference, so about 782.5 as a baseline. The band is
        // 14.4 pt and the text fits it, so the clamp must not fire — and it does fire, putting
        // this 2.2 pt out, if it is taken against `top` rather than against the band's own edge.
        snug.Origin.Y.Points.ShouldBeInRange(781.5, 783.5);
    }

    [Fact]
    public void AFooterThatOverflowsThePaperLeavesItsLastLineOffThePage()
    {
        IReadOnlyList<DrawnGlyphRun> runs = [.. Runs()];

        runs.Select(run => run.Text).ShouldContain("SPILLONE");
        runs.Select(run => run.Text).ShouldContain("SPILLTWO");

        // **A drift guard, and it records a refutation.** LibreOffice's PDF of this sheet holds
        // `SPILLONE` and `SPILLTWO` and not `SPILLTHREE`, and so does ours — but not because
        // either of us decided not to draw it. The third line's baseline lands past the 792 pt
        // page edge and the PDF writer drops it there.
        //
        // A round of this work spent a sweep on the wrong reading of that. Ten authored probes at
        // a 3.6 pt band showed a header keeping all nine of its lines while a footer keeps two,
        // which looks exactly like a per-line clip to the band — and clipping to the band drops
        // the second line of any two-line footer, which cost `fm-provider-service-measures.xlsx`
        // thirty words the reference does draw. Clipping to the *paper* instead was measured
        // against those same twelve probes with the clip in and with it out: **the two agree on
        // all twelve**, so the rule earns nothing that the page boundary does not already give.
        //
        // That last sentence used to say a header of eight empty lines followed by a text line
        // was "genuinely unexplained", with `FAA-2019-0995-0002_attachment_2.xlsx` as the corpus
        // instance at twenty words. **It is explained and it is fixed.** `PrintHF` clips each
        // area to the band's own rectangle and `DrawText_ToPosition` emits nothing for an area
        // whose ink misses it entirely — see `SheetBandFaceAndClipTests`, whose `Areas` sheet is
        // that shape. The reason it is not a *per-line* rule, and so does not touch the three
        // lines here, is the same measurement: LibreOffice keeps every line of an area that
        // overlaps the clip at all.
        DrawnGlyphRun third = runs.Single(run => run.Text == "SPILLTHREE");
        third.Origin.Y.Points.ShouldBeGreaterThan(792.0);
    }

    [Fact]
    public void TheThemeFormOfTheColourCodeLeavesNoTextBehind()
    {
        // `&K01+049` is six characters and two of them are hex. A reader that eats hex digits
        // stops at the `+` and draws `+049` — thirty times over five corpus workbooks, against a
        // reference that draws it none.
        Runs().Select(run => run.Text).ShouldNotContain(text => text.Contains("+049"));

        Runs().Single(run => run.Text == "LEFTPIN").ShouldNotBeNull();
    }

    [Fact]
    public void APinnedBandKeepsNoGapAndABandThatFitsKeepsTheDefault()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-small-band-xlsx.xlsx"));

        IReadOnlyList<SheetLayout> sheets = ((SpreadsheetPages)document.Layout()).Sheets;

        sheets[0].Setup.FooterGap.ShouldBe(Length.Zero);
        sheets[2].Setup.FooterGap.Twips.ShouldBe(142);
    }

    [Fact]
    public void TheDistanceIsNothingOnAPinnedBandAndTheFallbackOnOneThatFits()
    {
        Length fallback = Length.FromTwips(142);

        SheetBandHeight.BodyDistance("&C&9Tight", Length.FromInches(0.05), null, fallback)
            .ShouldBe(Length.Zero);

        SheetBandHeight.BodyDistance("&C&9Roomy", Length.FromInches(0.5), null, fallback)
            .ShouldBe(fallback);
    }
}

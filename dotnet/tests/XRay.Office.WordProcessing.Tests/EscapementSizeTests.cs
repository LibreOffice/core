using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The size a superscript or subscript is set at: <c>size x proportion / 100</c>, truncated to a whole
/// twip.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwSubFont::SetSize</c> shrinks the font with integer division on the layout's own unit —
/// <c>m_aSize.Height() * GetPropr() / 100</c>, <c>sw/source/core/inc/swfont.hxx</c>:772-783 — and
/// Writer's unit is the twip, so 58 per cent of eight point is 92.8 twips and the reference sets
/// <b>92</b>. Rounding sets 93, and the two rules part company at 29 of the 57 half-point sizes
/// between two and thirty point.
/// </para>
/// <para>
/// <strong>Measured at 26.2.4.2 through an advance, not through a font size.</strong> That
/// binary's PDF writer prints every <c>Tf</c> at a whole tenth of a point, so an eleven point
/// superscript reads back out of the PDF as 6.4 pt where the layout used 127 twips, or 6.35 — and a
/// rule fitted to that channel is a rule fitted to the writer. Differencing two right-aligned lines
/// holding two and forty-two copies of one digit cancels the label, the margin and the side bearing
/// and leaves forty advances at whatever size the layout used: truncation is right at <b>81 of 81</b>
/// arms and at <b>59 of 59</b> of those where truncating, rounding to a twip and rounding to a tenth
/// of a point disagree — 33 base sizes at proportion 58, and proportions 25 to 99 at two base sizes.
/// <c>probes/escsize-r153/results.md</c>.
/// </para>
/// </remarks>
public sealed class EscapementSizeTests
{
    /// <summary>
    /// Base sizes at which truncating and rounding differ, with the twip count measured off
    /// 26.2.4.2's own advances in <c>probes/escsize-r153/data/adv-ref.txt</c>.
    /// </summary>
    [Theory]
    [InlineData(5.5, 63)]
    [InlineData(6.0, 69)]
    [InlineData(8.0, 92)]
    [InlineData(10.5, 121)]
    [InlineData(11.0, 127)]
    [InlineData(13.0, 150)]
    [InlineData(15.5, 179)]
    [InlineData(18.0, 208)]
    public void AnAutomaticSuperscriptTakesTheTwipBelowFiftyEightPerCent(double points, int twips)
        => Escapement.Superscript.SizeOf(Length.FromPoints(points)).Twips.ShouldBe(twips);

    /// <summary>A subscript shrinks by the same rule; only the rise changes sign.</summary>
    [Fact]
    public void ASubscriptShrinksExactlyAsASuperscriptDoes()
        => Escapement.Subscript.SizeOf(Length.FromPoints(8.0)).Twips
            .ShouldBe(Escapement.Superscript.SizeOf(Length.FromPoints(8.0)).Twips);

    /// <summary>
    /// Proportions ODF can state, at eleven point — 220 twips — from the same measurement.
    /// </summary>
    [Theory]
    [InlineData(25, 55)]
    [InlineData(49, 107)]
    [InlineData(53, 116)]
    [InlineData(58, 127)]
    [InlineData(63, 138)]
    [InlineData(89, 195)]
    [InlineData(99, 217)]
    public void AStatedProportionTruncatesToo(int proportion, int twips)
        => new Escapement(33, proportion).SizeOf(Length.FromPoints(11.0)).Twips.ShouldBe(twips);

    /// <summary>Neither an absent proportion nor a stated hundred costs the run anything.</summary>
    /// <remarks>
    /// Worth its own assertion because the truncation would otherwise apply to them as well, and a
    /// size that is not a whole twip would then shrink by a fraction of one for stating nothing.
    /// </remarks>
    [Theory]
    [InlineData(0)]
    [InlineData(100)]
    public void NoProportionAndAHundredPerCentBothLeaveTheSizeAlone(int proportion)
    {
        Length size = Length.FromPoints(11.3);
        new Escapement(33, proportion).SizeOf(size).ShouldBe(size);
    }

    /// <summary>
    /// And the ODF reader reaches the arithmetic rather than passing the percentage through: the
    /// fixture's arms all draw the same word at eleven point, so a width ratio is the size ratio.
    /// </summary>
    /// <remarks>
    /// 26.2.4.2 draws the fixture's 58, 49 and 25 per cent arms at 127.12, 107.12 and 55.03 twips
    /// of the unescaped arm's width, against this tree's 127, 107 and 55 — the residual is the ink
    /// box the measurement is read from, and the rounded answer would be 128, 108 and 55.
    /// </remarks>
    [Theory]
    [InlineData(1, 127)]
    [InlineData(2, 107)]
    [InlineData(3, 116)]
    [InlineData(4, 55)]
    [InlineData(5, 220)]
    public void AnOdfTextPositionIsShrunkByTheSameRule(int arm, int twips)
        => Ratio(arm).ShouldBe(twips / 220.0, 0.0005);

    /// <summary>
    /// An escaped span inside an unescaped paragraph has to defeat the uniform-paragraph shortcut,
    /// which measures every run at the paragraph's own format.
    /// </summary>
    [Fact]
    public void AnEscapedSpanInsideAnUnescapedParagraphIsStillShrunk()
    {
        Ratio(6).ShouldBe(1.0, 0.0005);
        Ratio(7).ShouldBe(127 / 220.0, 0.0005);
    }

    private static double Ratio(int arm)
    {
        List<DrawnWord> words = Words();
        return (words[arm].Right - words[arm].Left) / (words[0].Right - words[0].Left);
    }

    private static List<DrawnWord> Words()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require("words-escapement-size.fodt")))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1);
            pages[0].Draw(sink);
        }

        List<DrawnWord> words = DrawnWords.On(sink.Pages[0]);
        words.Count.ShouldBe(8, "six paragraphs, the last of which holds two words");
        return words;
    }
}

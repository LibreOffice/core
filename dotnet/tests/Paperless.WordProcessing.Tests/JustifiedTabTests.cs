using Paperless.Core.Documents;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Justification on a line that holds a tab.
/// </summary>
/// <remarks>
/// <para>
/// A tab does not stop a line being justified, and the whole of a tabbed line's slack goes to the blanks
/// after its <em>last</em> tab. Both halves are <c>SwTextAdjuster::CalcNewBlock</c>
/// (<c>sw/source/core/text/itradj.cxx:255</c>), which spans a line "between two RandPortions or
/// FixPortions (Tabs and Flys)" and gives each span its own space-add: a tab is a <c>SwFixPortion</c>
/// whose fix width is its whole width (<c>sw/source/core/text/txttab.cxx:569</c>), so its glue is nought
/// and the span it closes is stretched by nothing. Only the last span meets the right margin's glue.
/// </para>
/// <para>
/// The one exception is <c>bDoNotJustifyTab</c> (<c>itradj.cxx:292</c>): a centre, right or decimal tab
/// followed by a manual break leaves that line ragged, where a left tab followed by the same break does
/// not.
/// </para>
/// <para>
/// The fixture is authored for this test — six justified paragraphs of nonsense words, one per case.
/// Every figure below was measured against LibreOffice 24.2.7.2's own rendering of it at A4 with 2 cm
/// margins, so the right margin is 538.58 pt. Before the fix a tabbed line was never justified at all:
/// the first line of <c>AAleft</c> ended at 483.0 pt rather than 538.6.
/// </para>
/// </remarks>
public sealed class JustifiedTabTests
{
    /// <summary>The right margin: A4 less two centimetres, in points.</summary>
    private const double RightMargin = 595.276 - 56.7;

    /// <summary>
    /// How far a drawn word may sit from LibreOffice's, in points.
    /// </summary>
    /// <remarks>
    /// A point. The whole of the width disagreement between HarfBuzz and Writer accumulates along a
    /// justified line — measured at 0.57 pt at the end of the untabbed control paragraph, which this fix
    /// does not touch — so the tolerance has to clear that and nothing more. The effect under test is
    /// 19 to 55 pt.
    /// </remarks>
    private const double TolerancePoints = 1.0;

    [Theory]
    [InlineData("AAleft", 538.21)]
    [InlineData("BBtwo", 538.11)]
    [InlineData("CCright", 537.96)]
    public void ATabbedLineIsStillJustifiedToTheMargin(string firstWord, double libreOffice)
    {
        List<DrawnWord> line = LineStartingWith(firstWord);

        line[^1].Right.ShouldBe(
            RightMargin,
            TolerancePoints,
            $"the line beginning {firstWord} reaches the margin, as LibreOffice's does at {libreOffice}");
    }

    /// <summary>
    /// A left tab before a manual break does not stop the line being justified.
    /// </summary>
    /// <remarks>
    /// <c>bDoNotJustifyTab</c> is cleared by a <c>TabLeft</c> portion, so the break portion after it
    /// finds it false and the line is stretched like any other. LibreOffice ends this one at 538.50 pt.
    /// </remarks>
    [Fact]
    public void ALeftTabBeforeAManualBreakDoesNotStopTheLineBeingJustified()
    {
        List<DrawnWord> line = LineStartingWith("DDleftbreak");

        line[^1].Right.ShouldBe(RightMargin, TolerancePoints);
    }

    /// <summary>
    /// A right tab before a manual break does stop it.
    /// </summary>
    /// <remarks>
    /// The same paragraph with a right stop instead. <c>TabRight</c> sets <c>bDoNotJustifyTab</c>, the
    /// break portion then finds it true, and <c>FinishSpaceAdd</c> abandons the line ragged: LibreOffice
    /// ends it at 262.22 pt, where its words naturally fall, rather than at the margin.
    /// </remarks>
    [Fact]
    public void ARightTabBeforeAManualBreakDoesStopIt()
    {
        List<DrawnWord> line = LineStartingWith("EErightbreak");

        line[^1].Right.ShouldBe(262.22, TolerancePoints);
    }

    /// <summary>
    /// Only the blanks after the last tab are stretched.
    /// </summary>
    /// <remarks>
    /// The <c>BBtwo</c> paragraph has two tabs, so its first line has three stretches. LibreOffice leaves
    /// the blanks of the first two at their natural 3.05 pt and widens the ones after the second tab to
    /// 4.93 — because a span closed by a tab is given that tab's own glue, which is nought.
    /// </remarks>
    [Fact]
    public void TheBlanksBeforeTheLastTabAreNotStretched()
    {
        List<DrawnWord> line = LineStartingWith("BBtwo");

        // "BBtwo tabs:" | tab | "mid one two" | tab | "alpha bravo …"
        double natural = NaturalBlank();
        double beforeFirstTab = line[1].Left - line[0].Right;
        double onMiddleStretch = line[3].Left - line[2].Right;
        double afterLastTab = line[6].Left - line[5].Right;

        beforeFirstTab.ShouldBe(natural, 0.2, "the blanks before the first tab keep their natural width");
        onMiddleStretch.ShouldBe(natural, 0.2, "the blanks between the tabs keep theirs too");
        afterLastTab.ShouldBeGreaterThan(
            natural + 0.5, "only the blanks after the last tab carry the line's slack");
    }

    /// <summary>
    /// A blank at its natural width, taken from the unjustified last line of the control paragraph.
    /// </summary>
    private static double NaturalBlank()
    {
        List<DrawnWord> words = Drawn();
        double lastBaseline = words[^1].Baseline;
        List<DrawnWord> last = [.. words.Where(word => word.Baseline == lastBaseline)
                                        .OrderBy(word => word.Left)];

        return last[1].Left - last[0].Right;
    }

    /// <summary>The words of the line whose first word starts with the text named.</summary>
    /// <remarks>
    /// A prefix rather than the whole word, because a right stop can leave the label touching the text
    /// after it — <see cref="DrawnWords"/> splits on a gap, so <c>CCright one two:</c> and the word after
    /// the tab come back as one. LibreOffice's own PDF has them the same way.
    /// </remarks>
    private static List<DrawnWord> LineStartingWith(string firstWord)
    {
        List<DrawnWord> words = Drawn();

        DrawnWord head = words.First(word => word.Text.StartsWith(firstWord, StringComparison.Ordinal));
        return [.. words.Where(word => word.Baseline == head.Baseline).OrderBy(word => word.Left)];
    }

    private static List<DrawnWord> Drawn()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source =
               DocumentSource.FromFile(Corpus.Require("justified-tabs.docx")))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages.SelectMany(DrawnWords.On)];
    }
}

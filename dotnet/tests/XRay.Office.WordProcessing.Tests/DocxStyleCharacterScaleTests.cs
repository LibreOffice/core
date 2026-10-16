using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// <c>w:w</c> — character width scaling — stated by a paragraph <em>style</em> over runs that state
/// nothing of their own.
/// </summary>
/// <remarks>
/// <para>
/// The property was resolved correctly through the style chain the whole time. What lost it is that
/// such a paragraph is <strong>uniform</strong>: every run has the style's formatting, so the run
/// splitter folds them away and the paragraph reaches the layout with no runs at all. Three
/// separate fallbacks then rebuild one run from the paragraph's own face, size, shaping and
/// tracking — and none of the three carried a width.
/// </para>
/// <list type="number">
/// <item><description>
/// <c>PageParagraph.Measure</c>, which builds the <c>FormattedRun</c> the measurement uses;
/// </description></item>
/// <item><description>
/// <c>PageDrawing.RunsIn</c>, which builds the <c>PageRun</c> the pen uses;
/// </description></item>
/// <item><description>
/// and the one that actually decided the line breaks — <strong>the shortcut in
/// <c>Paginator</c> and <c>FlowLayouter</c></strong>, which hands the breaker the paragraph's
/// <em>text</em>, a face, a size and its shaping, and measures from those alone. Tracking
/// survives that overload because <c>EffectiveShaping</c> carries it; a scale has nowhere to
/// ride, so the lines were broken at the unscaled widths.
/// </description></item>
/// </list>
/// <para>
/// <strong>Fixing only the first two is worse than fixing none</strong>, and that is why the arm
/// that wraps is in the fixture: with the pen scaled and the breaker not, every line is drawn at
/// exactly <c>scale ×</c> its unscaled width and broken in the unscaled places. A one-line probe
/// cannot tell that state from a correct one.
/// </para>
/// <para>
/// Measured against 26.2.4.2 on <c>Regulations Governing the Status…docx</c>, whose title style
/// <c>SL</c> states <c>&lt;w:w w:val="96"/&gt;</c> and whose title runs state no <c>w:rPr</c>:
/// <c>Experts on Mission</c> went from <b>227.56 pt</b> to <b>218.09</b> against the reference's
/// <b>218.06</b>, and the title's five lines now break on the same words — the reference pulls
/// <c>Duties</c> onto line 2 and so, now, does this tree.
/// </para>
/// </remarks>
public sealed class DocxStyleCharacterScaleTests
{
    private const string Fixture = "words-style-char-scale.docx";

    /// <summary>Every drawn line of the fixture, in order, as (text, width in points).</summary>
    private static List<(string Text, double Width)> Lines()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages[0].Runs.Select(run => (run.Text.Trim(), run.Width.Points))];
    }

    [Fact]
    public void AStyleStatedScaleSqueezesAParagraphWhoseRunsStateNothing()
    {
        List<(string Text, double Width)> lines = Lines();

        // The three one-line arms: the style states 60 per cent, the run states it, and neither
        // does. The first two have to agree with each other and be 0.6 of the third.
        lines[0].Width.ShouldBe(lines[1].Width, 0.01,
            "a scale stated by the style and by the run are the same scale");
        lines[0].Width.ShouldBe(lines[2].Width * 0.6, 0.5);
    }

    [Fact]
    public void TheScaleReachesTheLineBreakerAndNotOnlyThePen()
    {
        List<string> text = [.. Lines().Select(line => line.Text)];

        // Arm D is the scaled style over eighteen words and arm E is the same words unscaled.
        // 26.2.4.2 breaks the first into two lines and the second into three; a tree that scaled
        // the pen alone broke both into the same three.
        text[3].ShouldBe("alpha bravo charlie delta echo foxtrot golf hotel india juliett kilo lima mike");
        text[4].ShouldBe("november oscar papa quebec romeo");

        text[5].ShouldBe("alpha bravo charlie delta echo foxtrot golf hotel");
        text[6].ShouldBe("india juliett kilo lima mike november oscar papa");
        text[7].ShouldBe("quebec romeo");
    }

    [Fact]
    public void TheScaledWrapIsWhereTheReferencePutsIt()
    {
        List<(string Text, double Width)> lines = Lines();

        // Read out of 26.2.4.2's own PDF of this fixture: 434.895 and 206.899, where its span for
        // the first includes the trailing space this tree's does not.
        lines[3].Width.ShouldBe(431.96, 0.5);
        lines[4].Width.ShouldBe(207.13, 0.5);
    }
}

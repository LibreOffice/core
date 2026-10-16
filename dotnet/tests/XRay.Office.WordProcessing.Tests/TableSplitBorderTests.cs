using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A table row cut by a page boundary is ruled once at the cut, on each page, and the cut's rule is
/// the cell's <em>bottom</em> one.
/// </summary>
/// <remarks>
/// <para>
/// The last third of the rule <see cref="TableBorderTopAlignmentTests"/> and round 149 closed: a
/// horizontal band's top edge sits on the boundary and hangs downwards, and the row below pays for the
/// whole of it. At a page cut there is no row below, so the question is what the part itself pays —
/// and it pays twice over, once at each end.
/// </para>
/// <para>
/// [src] <c>lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines</c>
/// (<c>sw/source/core/layout/paintfrm.cxx</c>:2815-2833, used at :3089): the first row of a follow
/// table draws its <b>top</b> line from the cell's <b>bottom</b> border style. The master's own foot
/// draws that same rule, so both halves of a split row are ruled by it.
/// </para>
/// <para>
/// [bin] <c>words-table-split.docx</c> is one 46-row table whose cells state <c>w:top nil</c> and a
/// 1 pt <c>w:bottom</c>, with row 30 long enough to straddle the page. 26.2.4.2 rules the foot of
/// page 1 at <b>759.201..760.201</b> and the head of page 2 at <b>70.901..71.901</b>, both hanging
/// downwards from the frame edge, and the follow's first line starts at 71.901 — so the follow really
/// is charged the whole of it. All 48 of the fixture's bands now agree within 0.05 pt.
/// <c>probes/split-r152/</c>.
/// </para>
/// <para>
/// <b>Three separate errors had to go, and each was worth about a line or a rule.</b> The part was
/// charged its own <em>top</em> rule, which this table states as <c>nil</c>, so it was charged nothing
/// and one line too many fitted above the cut; the part did not charge the boundary band <em>above</em>
/// it, so it overran the text area by a band; and a part that <em>finished</em> its row charged a band
/// at its foot that the next row also charged, ruling one boundary twice a band apart.
/// </para>
/// </remarks>
public sealed class TableSplitBorderTests
{
    private const string Fixture = "words-table-split.docx";

    /// <summary>The cut is ruled once on each page, not twice.</summary>
    /// <remarks>
    /// The seat this closes — O83 — is *"the grid line at the split is drawn twice, 0.25 pt apart"*.
    /// Every boundary in this fixture carries exactly one band, the cut included.
    /// </remarks>
    [Fact]
    public void EveryBoundaryIsRuledExactlyOnce()
    {
        List<List<double>> pages = Rules();

        foreach (List<double> tops in pages)
        {
            tops.Distinct().Count().ShouldBe(tops.Count, "no boundary may be ruled twice");

            for (int i = 1; i < tops.Count; i++)
            {
                (tops[i] - tops[i - 1]).ShouldBeGreaterThan(
                    2.0, "two bands a rule apart are one boundary ruled twice");
            }
        }
    }

    /// <summary>The master part's rule hangs below its last line and inside the text area.</summary>
    /// <remarks>
    /// The text area ends at 771.04 pt. Before round 152 the part ran to 771.75 — it fitted a line the
    /// reference rejects, because nothing reserved the band the cut would need.
    /// </remarks>
    [Fact]
    public void TheCutsRuleFitsInsideTheTextArea()
    {
        List<double> first = Rules()[0];

        double cut = first[^1];
        cut.ShouldBe(759.201, 0.06, "26.2.4.2 rules the foot of page 1 here");
        (cut + 1.0).ShouldBeLessThanOrEqualTo(771.05, "the band must fit above the text area's foot");
    }

    /// <summary>The follow part is ruled at its head and pays for it.</summary>
    /// <remarks>
    /// Its first line starts below the band rather than inside it, which is what says the follow is
    /// charged the whole rule rather than merely having one drawn over it.
    /// </remarks>
    [Fact]
    public void TheFollowPartIsRuledAtItsHeadAndPaysForIt()
    {
        Rules()[1][0].ShouldBe(70.901, 0.06, "26.2.4.2 rules the head of page 2 here");
    }

    /// <summary>
    /// The boundary below a follow part that finished its row is ruled where the reference rules it.
    /// </summary>
    /// <remarks>
    /// This is the assertion that pins the third of the round's three errors, and the count-based test
    /// above does not: a part that completes its row charged a band at its foot that the row after it
    /// charged again, so the boundary was ruled twice a band apart — 118.050 and 119.050 against
    /// 26.2.4.2's single 118.101 — and everything below was pushed down with it. Asserted against the
    /// reference's own position rather than by counting, because the drawing sink and the written PDF
    /// do not agree about how many paths that doubled boundary produces and only the position is the
    /// same question in both.
    /// </remarks>
    [Fact]
    public void TheBoundaryBelowAFinishedFollowPartIsRuledOnce()
    {
        Rules()[1][1].ShouldBe(118.101, 0.06, "the first interior boundary of page 2");
    }

    /// <summary>Every horizontal rule of each page, as its band's top edge, in order.</summary>
    private static List<List<double>> Rules()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages.Select(page => (List<double>)
            [.. page.StrokedPaths
                .Where(stroke => stroke.Bounds.Height <= Length.FromPoints(0.01))
                .Select(stroke => (stroke.Bounds.Y - (stroke.Stroke.Width / 2)).Points)
                .Order()])];
    }
}

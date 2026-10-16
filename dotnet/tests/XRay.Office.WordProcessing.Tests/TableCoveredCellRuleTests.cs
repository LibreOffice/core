using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A cell that a vertical merge covers states rules that are charged to the row's <em>height</em>
/// although the row does not hold the cell and, at its top edge, nothing is drawn for them.
/// </summary>
/// <remarks>
/// <para>
/// A covered cell is dropped as the rows are resolved, because the merge above owns its space and
/// nothing draws it. Writer drops it from nothing: it builds a cell frame for the covered box like any
/// other, and <c>lcl_GetTopSpace</c> (<c>sw/source/core/layout/tabfrm.cxx</c>:5175-5194) walks every
/// lower of the row taking the maximum of each one's own <c>CalcLineSpace(TOP, true)</c>
/// <b>without testing the row span</b> — although eight other places in the same file test
/// <c>getRowSpan() &lt; 1</c> precisely to skip such a cell.
/// </para>
/// <para>
/// [bin] Measured on 26.2.4.2 over 34 one-attribute arms, <c>probes/vmergetop-r154/</c>. This fixture is
/// those arms reduced to the four pairs that carry the rule, one pair per place it is charged and one arm
/// per page, and its expectations are 26.2.4.2's own rendering of it — every baseline and every rule of
/// all eight pages, to the hundredth.
/// </para>
/// <para>
/// The seat is O78's second, and on <c>FAA 2025-26 Holdover Tables.docx</c> it is the last 2.05 pt: the
/// page-82 table's foot lands within 0.05 pt of 26.2.4.2 with it, and its 33 row deltas sum to zero.
/// </para>
/// </remarks>
public sealed class TableCoveredCellRuleTests
{
    private const string Fixture = "words-vmerge-covered-rule.docx";

    private const int FloorCovered = 0;
    private const int FloorBare = 1;
    private const int BandCovered = 2;
    private const int BandBare = 3;
    private const int BelowCovered = 4;
    private const int BelowBare = 5;
    private const int OuterCovered = 6;
    private const int OuterBare = 7;

    /// <summary>
    /// A <c>w:trHeight</c> floor is raised by the covered cell's stated top rule, exactly as it would be
    /// by an ordinary cell's.
    /// </summary>
    /// <remarks>
    /// The floor is 450 twips — 22.50 pt — and the arms differ in one attribute: whether the covered cell
    /// states a 1 pt top or <c>nil</c>. 26.2.4.2 draws the row 23.50 pt tall and 22.50 pt tall.
    /// </remarks>
    [Fact]
    public void ACoveredCellsTopRuleRaisesTheRowsDeclaredHeight()
    {
        RowHeight(FloorCovered).ShouldBe(23.50, 0.01);
        RowHeight(FloorBare).ShouldBe(22.50, 0.01);
    }

    /// <summary>
    /// It is charged to the band above the row as well — and <em>nothing is drawn</em> for it, because the
    /// edge is interior to the merge.
    /// </summary>
    /// <remarks>
    /// Every other edge at that boundary states <c>nil</c>, so the covered cell's 2 pt top is the only
    /// statement there. 26.2.4.2 pushes the lower row's baseline down by the whole 2 pt and draws no rule:
    /// the one place in this engine where the height and the ink are deliberately different sets of
    /// statements.
    /// </remarks>
    [Fact]
    public void TheBandAboveTheRowIsChargedForItAndNoRuleIsDrawn()
    {
        BaselineGap(BandCovered).ShouldBe(11.20, 0.01);
        BaselineGap(BandBare).ShouldBe(9.20, 0.01);

        Rules(BandCovered).Count.ShouldBe(1, "the table's own top rule, and nothing at the covered edge");
        Rules(BandBare).Count.ShouldBe(1);
    }

    /// <summary>
    /// Its stated <em>bottom</em> is charged at the boundary below it — and that one <em>is</em> drawn,
    /// because it is the merge's own outer edge rather than an interior one.
    /// </summary>
    [Fact]
    public void TheBandBelowItIsChargedForItAndThatRuleIsDrawn()
    {
        BaselineGap(BelowCovered).ShouldBe(11.20, 0.01);
        BaselineGap(BelowBare).ShouldBe(9.20, 0.01);

        Rules(BelowCovered).Count.ShouldBe(3);
        Rules(BelowBare).Count.ShouldBe(2, "the covered cell states nothing, so that boundary is bare");
    }

    /// <summary>And the band below the table's last row, which belongs to no row at all.</summary>
    /// <remarks>
    /// <see cref="XRay.Office.WordProcessing.Layout.PageTableRow.CoveredBottomRule"/> is charged to the table
    /// after its row rectangles are built, so it is a third place the covered cell could be dropped.
    /// </remarks>
    [Fact]
    public void TheBandBelowTheTablesLastRowIsChargedForItToo()
    {
        Rules(OuterCovered).Count.ShouldBe(2);
        Rules(OuterBare).Count.ShouldBe(1);

        // The charge itself is only visible BELOW the table, which is why this pair alone carries a mark
        // after it: the band under the last row moves nothing inside the table.
        BaselineGap(OuterCovered).ShouldBe(11.20, 0.01);
        BaselineGap(OuterBare).ShouldBe(9.20, 0.01);

        // 26.2.4.2 draws this arm's two bands centred on 36.5 and 58.4, 1 pt and 2 pt wide, so their
        // top edges — which is what a band is measured by since round 149 — are 21.40 pt apart.
        (Rules(OuterCovered)[^1] - Rules(OuterCovered)[0]).ShouldBe(21.40, 0.01);
    }

    /// <summary>The height between the arm's second and third grid lines, which is its lower row.</summary>
    private static double RowHeight(int arm)
    {
        List<double> rules = Rules(arm);
        rules.Count.ShouldBe(3, "the arm draws three grid lines");
        return rules[2] - rules[1];
    }

    /// <summary>
    /// The gap between the last two baselines on the arm's page, which is what a boundary charge moves.
    /// </summary>
    private static double BaselineGap(int arm)
    {
        List<double> baselines = [.. DrawnWords.On(Pages()[arm]).Select(word => word.Baseline).Distinct().Order()];
        baselines.Count.ShouldBeGreaterThanOrEqualTo(2);
        return baselines[^1] - baselines[^2];
    }

    /// <summary>The tops of the arm's horizontal rules, in order.</summary>
    private static List<double> Rules(int arm) =>
        [.. Pages()[arm].StrokedPaths
            .Where(stroke => stroke.Bounds.Height <= Length.FromPoints(0.01))
            .Select(stroke => (stroke.Bounds.Y - (stroke.Stroke.Width / 2)).Points)
            .Distinct()
            .Order()];

    private static List<DrawnPage> Pages()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(8, "one arm per page");
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages];
    }
}

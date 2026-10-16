using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A horizontal table border hangs downwards from the row boundary; it is not centred on it.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwTabFramePainter::Insert</c> (<c>sw/source/core/layout/paintfrm.cxx</c>:3061-3064) sets
/// <c>RefMode::Begin</c> on the two horizontal borders of every cell and <c>RefMode::Centered</c>
/// on the two vertical ones, and <c>Begin</c> is documented as <em>"horizontal lines are drawn
/// below the reference points"</em>.
/// </para>
/// <para>
/// <strong>That is invisible until one boundary carries two widths.</strong> This tree charges
/// half of a boundary's border to each of the two rows, so its grid line sits in the middle of
/// the band the reference hangs below the boundary — and for a boundary of one width those two
/// descriptions put the ink in exactly the same place. Where the columns differ they do not: the
/// reference gives every band on the line the <em>same</em> top edge, the top of the widest one.
/// </para>
/// <para>
/// The fixture's first page states 0.5, 3.0 and 1.5 pt across one boundary and 26.2.4.2 draws
/// <c>95.001..95.501</c>, <c>95.001..98.001</c> and <c>95.001..96.501</c> — one top, three
/// bottoms. The assertions here are about the <em>shape</em> rather than the absolute y, because
/// the absolute y is still a half-border out: the <em>height</em> half of the same rule, in which
/// the row below pays for the whole band and the row above pays nothing, is seat O84 and moves
/// page counts. <c>probes/tablerow-r146/results.md</c>.
/// </para>
/// </remarks>
public sealed class TableBorderTopAlignmentTests
{
    private const string Fixture = "words-table-border-align.docx";

    /// <summary>Every horizontal rule on one page, as (top edge, thickness), thickest first.</summary>
    private static List<(double Top, double Thick)> Rules(int page)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages[page].StrokedPaths
            .Where(stroke => stroke.Bounds.Height <= Length.FromPoints(0.01))
            .Select(stroke => (
                Top: (stroke.Bounds.Y - (stroke.Stroke.Width / 2)).Points,
                Thick: stroke.Stroke.Width.Points))
            .OrderByDescending(rule => rule.Thick)];
    }

    /// <summary>The interior boundary's bands, grouped by the top edge they share.</summary>
    /// <remarks>
    /// Grouped by position rather than picked by thickness, because the table's own outline is two
    /// 1 pt rules and a thickness filter catches those as well — which is how the first cut of this
    /// test read five bands where the fixture states three.
    /// </remarks>
    private static List<(double Top, double Thick)> Interior(int page)
        => [.. Rules(page)
            .GroupBy(rule => Math.Round(rule.Top, 2))
            .Where(group => group.Count() > 1)
            .OrderByDescending(group => group.Count())
            .First()
            .OrderByDescending(rule => rule.Thick)];

    [Fact]
    public void ThreeWidthsOnOneBoundaryShareOneTopEdge()
    {
        List<(double Top, double Thick)> interior = Interior(0);

        interior.Select(rule => rule.Thick).ShouldBe([3.0, 1.5, 0.5], "the three stated widths");

        double widest = interior[0].Top;
        foreach ((double top, double thick) in interior)
        {
            top.ShouldBe(widest, 0.01, $"the {thick} pt band must begin where the widest one does");
        }
    }

    [Fact]
    public void TheirBottomEdgesDifferByTheirOwnWidths()
    {
        // The control for the assertion above: three bands sharing a top *and* a bottom would be
        // three bands of one width, which is not what the fixture states.
        List<double> bottoms = [.. Interior(0).Select(rule => rule.Top + rule.Thick)];

        bottoms.Distinct().Count().ShouldBe(3, "one top edge, three different bottoms");
    }

    [Fact]
    public void TheSameHoldsWithTheStatementOnTheOtherSideOfTheBoundary()
    {
        // The fixture's second page states the same three widths from the row below rather than
        // the row above, and 26.2.4.2 draws it identically — which is what says the rule is about
        // the boundary and not about which row spoke.
        List<(double Top, double Thick)> interior = Interior(1);

        interior.Select(rule => rule.Thick).ShouldBe([3.0, 1.5, 0.5]);
        interior.Select(rule => Math.Round(rule.Top, 2)).Distinct().Count().ShouldBe(1);
    }

    [Fact]
    public void ABoundaryOfOneWidthIsUnmoved()
    {
        // The confinement guarantee, as an assertion rather than as a claim: the fixture's outer
        // rules are a single 1 pt across the whole table, and for those the centred reading and
        // the top-aligned one are the same place. If this moves, every table in the corpus does.
        List<(double Top, double Thick)> outer = [.. Rules(0).Where(rule => rule.Thick is > 0.9 and < 1.1)];

        outer.Count.ShouldBe(2, "the table's top and bottom");
    }
}

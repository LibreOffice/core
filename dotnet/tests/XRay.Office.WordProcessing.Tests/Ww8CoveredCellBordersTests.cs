using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// In a <c>.doc</c>, a cell a vertical merge covers carries the <em>merge master's</em> borders, and
/// whatever its own <c>TC</c> states is discarded.
/// </summary>
/// <remarks>
/// <para>
/// Nothing draws a covered cell, so this reaches exactly one number — the row's height, through
/// <see cref="XRay.Office.WordProcessing.Layout.PageTableRow.CoveredTopRule"/>, which round 154 added for
/// all four readers. That round could close only two of this format's six failing arms and recorded the
/// rest as O105: <em>"a `.doc`'s covered cell carries borders that the file does not state, so reading
/// them is not enough"</em>.
/// </para>
/// <para>
/// [bin] The instrument that separates the two readings is 26.2.4.2's own <c>--convert-to fodt</c> of the
/// <c>.doc</c>, which prints every cell's resolved borders. Two one-attribute arms decide it: a master
/// stating a 3 pt top over a covered cell stating <c>nil</c> gives the covered cell <b>3 pt</b>, and a
/// master stating 1 pt under a covered cell stating 3 pt gives it <b>1 pt</b> — the master's in both
/// directions and the cell's own in neither. The ODF export names it as plainly as the numbers do: the
/// covered cell comes out carrying the master's own cell style, <c>TableN.A1</c>, where an unmerged lower
/// cell carries its own <c>TableN.A2</c>. <c>probes/ww8covered-r156/</c>.
/// </para>
/// <para>
/// This fixture is that measurement: four arms, one per page, each a two-row table on a
/// <c>w:trHeight</c> floor of 450 twips — 22.50 pt — so the row's height is the floor plus whatever top
/// rule is charged. Its expectations are 26.2.4.2's own rendering of it.
/// </para>
/// </remarks>
public sealed class Ww8CoveredCellBordersTests
{
    private const string Fixture = "words-vmerge-master-borders.doc";

    /// <summary>
    /// The covered cell states <c>nil</c> and the master states 3 pt, and the row is charged <b>3 pt</b>.
    /// </summary>
    [Fact]
    public void ACoveredCellTakesTheMastersRuleWhenItStatesNone() => RowHeight(0).ShouldBe(25.50, 0.01);

    /// <summary>
    /// The covered cell states 3 pt and the master states 1 pt, and the row is charged <b>1 pt</b> — so
    /// this is the master's rule rather than the thicker of the two.
    /// </summary>
    [Fact]
    public void ACoveredCellsOwnRuleIsDiscardedEvenWhenItIsThicker() => RowHeight(1).ShouldBe(23.50, 0.01);

    /// <summary>
    /// The controls: the same two tables with no merge, where the lower cell keeps its own statement.
    /// </summary>
    /// <remarks>
    /// Without them the rule would be indistinguishable from "a lower cell's top rule is ignored", which
    /// is false — arm D is charged the 3 pt it states under a master that states one point.
    /// </remarks>
    [Theory]
    [InlineData(2, 22.50)]
    [InlineData(3, 25.50)]
    public void AnUnmergedCellKeepsItsOwn(int arm, double height) => RowHeight(arm).ShouldBe(height, 0.01);

    /// <summary>The lower row's height, between the arm's second and third grid lines.</summary>
    private static double RowHeight(int arm)
    {
        List<double> rules =
            [.. Pages()[arm].StrokedPaths
                .Where(stroke => stroke.Bounds.Height <= Length.FromPoints(0.01))
                .Select(stroke => (stroke.Bounds.Y - (stroke.Stroke.Width / 2)).Points)
                .Distinct()
                .Order()];

        rules.Count.ShouldBe(3, "each arm draws three grid lines");
        return rules[2] - rules[1];
    }

    private static List<DrawnPage> Pages()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(4, "one arm per page");
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages];
    }
}

using Paperless.Core.Units;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Where a DOC's <c>sprmPFDyaBeforeAuto</c> and <c>sprmPFDyaAfterAuto</c> margins are not drawn.
/// </summary>
/// <remarks>
/// <para>
/// The two sprms ask for fourteen points, and Word draws nothing at four places the flag alone does not
/// distinguish: the first paragraph of a flow, the two edges of a table cell, and between two items of
/// one list. LibreOffice applies all four as post-conditions on the finished node —
/// <c>SwWW8ImplReader::FinalizeTextNode</c> (<c>sw/source/filter/ww8/ww8par.cxx</c>:2627-2681) and
/// <c>WW8TabDesc::SetPamInCell</c> (<c>ww8par2.cxx</c>:2896-2935).
/// </para>
/// <para>
/// Driven from hand-built blocks rather than from a fixture, and deliberately. LibreOffice <em>reads</em>
/// these two sprms and never writes them — <c>ww8par6.cxx</c>:6045 maps them on import and nothing under
/// <c>sw/source/filter/ww8/</c> emits them — so a DOC exercising them cannot be produced by converting
/// anything with <c>soffice</c>, and no file in <c>tests/corpus</c> contains one. What that leaves
/// untested is the sprm decode itself; the rules it feeds are what these cover, and the decode is
/// evidenced by the corpus instead (<c>A380MaenPressRel.doc</c>, three pages against our two before the
/// change and three after).
/// </para>
/// </remarks>
public sealed class DocAutoSpacingTests
{
    /// <summary>Fourteen points, the margin the flag stands for.</summary>
    private static readonly Length Auto = Length.FromTwips(280);

    [Fact]
    public void AFlowsFirstParagraphLosesItsAutoSpaceBefore()
    {
        List<Ww8LayoutBlock> blocks = [Auto1("first"), Auto1("second")];

        Ww8DocumentReader.SuppressAutoSpacing(blocks, collapsesSpacing: true);

        blocks[0].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Length.Zero);
        blocks[0].Paragraph!.Value.Format.SpaceAfter.ShouldBe(Auto, "only the top edge is an edge");
        blocks[1].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Auto);
        blocks[1].Paragraph!.Value.Format.SpaceAfter.ShouldBe(Auto);
    }

    [Fact]
    public void ACellsFirstAndLastParagraphsLoseTheEdgeTheyTouch()
    {
        List<Ww8LayoutBlock> cell = [Auto1("head"), Auto1("middle"), Auto1("tail")];
        List<Ww8LayoutBlock> blocks = [Table(cell)];

        Ww8DocumentReader.SuppressAutoSpacing(blocks, collapsesSpacing: true);

        IReadOnlyList<Ww8LayoutBlock> after = blocks[0].Table!.Rows[0].Cells[0].Blocks;

        after[0].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Length.Zero);
        after[0].Paragraph!.Value.Format.SpaceAfter.ShouldBe(Auto);

        after[1].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Auto, "the middle touches no wall");
        after[1].Paragraph!.Value.Format.SpaceAfter.ShouldBe(Auto);

        after[2].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Auto);
        after[2].Paragraph!.Value.Format.SpaceAfter.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// A stated margin at the same edge survives, because the suppression is of the auto rule.
    /// </summary>
    /// <remarks>
    /// The distinction is why the paragraph carries how the margin was arrived at rather than being
    /// compared against fourteen points: a form that spaces its cells deliberately must keep it.
    /// </remarks>
    [Fact]
    public void AStatedMarginAtTheSameEdgeSurvives()
    {
        Ww8DocumentReader.Ww8LayoutParagraph stated = Paragraph("stated") with
        {
            Format = Margins(Length.FromTwips(200), Length.FromTwips(200)),
            HasAutoSpaceBefore = false,
            HasAutoSpaceAfter = false,
        };

        List<Ww8LayoutBlock> blocks = [new Ww8LayoutBlock(stated)];

        Ww8DocumentReader.SuppressAutoSpacing(blocks, collapsesSpacing: true);

        blocks[0].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Length.FromTwips(200));
    }

    /// <summary>
    /// Two items of one list have no auto margin between them, and the last of them keeps its own.
    /// </summary>
    /// <remarks>
    /// The shape LibreOffice reaches by zeroing both margins of every numbered paragraph and giving the
    /// lower one back to whichever turned out to be the last, once the rule changes or the numbering
    /// stops.
    /// </remarks>
    [Fact]
    public void TwoItemsOfOneListHaveNoAutoMarginBetweenThem()
    {
        List<Ww8LayoutBlock> blocks =
        [
            Auto1("before"),
            Listed("item one", rule: 1),
            Listed("item two", rule: 1),
            Auto1("after"),
        ];

        Ww8DocumentReader.SuppressAutoSpacing(blocks, collapsesSpacing: true);

        // The list's first item keeps the margin separating it from the paragraph above.
        blocks[1].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Auto);
        blocks[1].Paragraph!.Value.Format.SpaceAfter.ShouldBe(Length.Zero);

        blocks[2].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Length.Zero);

        // ...and the last regains the margin that separates the list from what follows it.
        blocks[2].Paragraph!.Value.Format.SpaceAfter.ShouldBe(Auto);
    }

    /// <summary>
    /// Two adjacent lists keep a margin between them, and it belongs to the first one's last item.
    /// </summary>
    /// <remarks>
    /// Which side it lands on is not a detail: LibreOffice zeroes the upper margin of <em>any</em>
    /// paragraph whose predecessor was numbered, whether or not it is the same list
    /// (<c>if (m_pPrevNumRule &amp;&amp; m_bParaAutoBefore)</c>), and separates two lists by restoring
    /// the <em>lower</em> margin of the item that ended the first. Asserting it on the second list's
    /// first item would pass on an implementation that draws the gap twice.
    /// </remarks>
    [Fact]
    public void TwoAdjacentListsAreSeparatedByTheFirstListsLowerMargin()
    {
        List<Ww8LayoutBlock> blocks = [Listed("one", rule: 1), Listed("two", rule: 2)];

        Ww8DocumentReader.SuppressAutoSpacing(blocks, collapsesSpacing: true);

        blocks[0].Paragraph!.Value.Format.SpaceAfter.ShouldBe(
            Auto, "a different rule is a different list");
        blocks[1].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// A document that switched HTML auto-spacing off keeps its margins at every edge.
    /// </summary>
    /// <remarks>
    /// Every one of LibreOffice's four tests is written <c>&amp;&amp; !fDontUseHTMLAutoSpacing</c>, so
    /// the flag decides whether the suppression happens at all and not merely what it suppresses.
    /// </remarks>
    [Fact]
    public void ADocumentNotUsingHtmlAutoSpacingKeepsEveryMargin()
    {
        List<Ww8LayoutBlock> blocks = [Auto1("first"), Table([Auto1("head"), Auto1("tail")])];

        Ww8DocumentReader.SuppressAutoSpacing(blocks, collapsesSpacing: false);

        blocks[0].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Auto);

        IReadOnlyList<Ww8LayoutBlock> cell = blocks[1].Table!.Rows[0].Cells[0].Blocks;
        cell[0].Paragraph!.Value.Format.SpaceBefore.ShouldBe(Auto);
        cell[^1].Paragraph!.Value.Format.SpaceAfter.ShouldBe(Auto);
    }

    private static Ww8LayoutBlock Auto1(string text) => new(Paragraph(text));

    private static Ww8LayoutBlock Listed(string text, int rule)
        => new(Paragraph(text) with { ListRule = rule });

    private static Ww8LayoutBlock Table(List<Ww8LayoutBlock> cell)
        => new(new Ww8LayoutTable(
            [Length.FromTwips(4000)],
            [new Ww8LayoutRow([new Ww8LayoutCell(0, 1, 1, default, cell)], IsHeader: false)],
            HeaderRowCount: 0,
            LeftIndent: Length.Zero));

    private static Ww8DocumentReader.Ww8LayoutParagraph Paragraph(string text)
        => new(
            SectionIndex: 0,
            Text: text,
            Format: Margins(Auto, Auto),
            FamilyName: null,
            Size: Length.FromPoints(11),
            Weight: 400,
            IsItalic: false,
            Language: null,
            IsInTable: false)
        {
            HasAutoSpaceBefore = true,
            HasAutoSpaceAfter = true,
        };

    private static ParagraphFormat Margins(Length before, Length after)
        => ParagraphFormat.Default with { SpaceBefore = before, SpaceAfter = after };
}

using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A DOC table that belongs in a Word text frame rather than in the flow.
/// </summary>
/// <remarks>
/// <para>
/// A masthead in a Word 97 document is normally a small table floated over the top of the first page,
/// and a DOC says so by putting the frame's paragraph sprms — <c>sprmPPc</c>, <c>sprmPDxaAbs</c>,
/// <c>sprmPDyaAbs</c>, <c>sprmPWr</c> — on the table's own paragraphs. Word reads them at one place
/// only, the first paragraph of the first cell of a row, and moves that row entire:
/// <em>"if it is the first cell of a row then the whole table row jumps into the new frame, if it isn't
/// then the paragraph attributes are applied except for the floating frame stuff"</em>
/// (<c>SwWW8ImplReader::TestApo</c>, <c>sw/source/filter/ww8/ww8par2.cxx</c>:440).
/// </para>
/// <para>
/// <strong>The cost of leaving such a table in the flow is its whole height, taken from the first
/// page.</strong> Measured on <c>AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc</c> against
/// LibreOffice 26.2.4.2: its header block is a three-row table in a page-relative frame, and laid out in
/// the flow it pushed the body down by <strong>72.3 pt</strong> — the frame's own height — so page 1
/// ended three lines early, page 2 inherited them and lost five at its foot, and the document paginated
/// 21 pages against the reference's 20. Lifted, the two agree at 20 pages, at 7482 extractable words to
/// 7482, and per page on nineteen pages of twenty.
/// </para>
/// <para>
/// The mechanism rather than that page count is what these assert, because the page count can be
/// restored by any number of unrelated changes to line height: what must hold is that a framed table
/// leaves the flow, that a table and the paragraphs stating the same position form one frame, and that
/// a table nobody framed stays exactly where the document put it.
/// </para>
/// </remarks>
public sealed class Ww8FramedTableTests
{
    /// <summary>The masthead's own position, read out of the document named above.</summary>
    /// <remarks>
    /// Binding <c>0xA0</c> is page-relative horizontally and paragraph-relative vertically; the
    /// <c>YOffset</c> is what makes it a frame rather than a restatement of the defaults.
    /// </remarks>
    private static readonly Ww8TextFramePosition Masthead = Ww8TextFramePosition.None with
    {
        Binding = 0xA0,
        XOffset = 3988,
        YOffset = 52,
        Width = 6673,
        Wrap = 2,
        StatesVerticalPosition = true,
    };

    private static readonly Ww8TextFramePosition Elsewhere = Masthead with { XOffset = 1000 };

    [Fact]
    public void AFramedTableLeavesTheFlowAndHangsOnTheParagraphAfterIt()
    {
        List<Ww8LayoutBlock> lifted = Ww8DocumentReader.LiftTextFrames(
            [Table(Masthead), Paragraph("body")]);

        // The table is gone from the flow: what is left is the one body paragraph.
        lifted.Count.ShouldBe(1);
        lifted[0].Paragraph!.Value.Text.ShouldBe("body");

        // And it is carried as a frame by the paragraph that followed it, which is where Writer's
        // insertion point ends up once `StopApo` has left the fly.
        IReadOnlyList<Ww8LayoutTextFrame> frames = lifted[0].Paragraph!.Value.TextFrames!;
        frames.Count.ShouldBe(1);
        frames[0].Position.ShouldBe(Masthead);
        frames[0].Blocks.Count.ShouldBe(1);
        frames[0].Blocks[0].Table.ShouldNotBeNull();
    }

    [Fact]
    public void ATableThatStatesNoFrameStaysWhereTheDocumentPutIt()
    {
        List<Ww8LayoutBlock> lifted = Ww8DocumentReader.LiftTextFrames(
            [Paragraph("before"), Table(Ww8TextFramePosition.None), Paragraph("after")]);

        lifted.Count.ShouldBe(3);
        lifted[1].Table.ShouldNotBeNull();
        lifted[2].Paragraph!.Value.TextFrames.ShouldBeNull();
    }

    [Fact]
    public void ATableAndTheParagraphsAroundItStatingOnePositionAreOneFrame()
    {
        // A masthead is regularly a caption paragraph, the table, and a rule paragraph, all carrying the
        // same sprms. Writer closes a frame when `TestSameApo` finds a different `WW8FlyPara`, not when
        // the block kind changes, so all three belong to one fly.
        List<Ww8LayoutBlock> lifted = Ww8DocumentReader.LiftTextFrames(
            [Paragraph("head", Masthead), Table(Masthead), Paragraph("foot", Masthead), Paragraph("body")]);

        lifted.Count.ShouldBe(1);
        IReadOnlyList<Ww8LayoutTextFrame> frames = lifted[0].Paragraph!.Value.TextFrames!;
        frames.Count.ShouldBe(1);
        frames[0].Blocks.Count.ShouldBe(3);
    }

    [Fact]
    public void TwoTablesNamingDifferentPositionsAreTwoFrames()
    {
        List<Ww8LayoutBlock> lifted = Ww8DocumentReader.LiftTextFrames(
            [Table(Masthead), Table(Elsewhere), Paragraph("body")]);

        lifted.Count.ShouldBe(1);
        IReadOnlyList<Ww8LayoutTextFrame> frames = lifted[0].Paragraph!.Value.TextFrames!;
        frames.Count.ShouldBe(2);
        frames[0].Position.ShouldBe(Masthead);
        frames[1].Position.ShouldBe(Elsewhere);
    }

    [Fact]
    public void EveryRowHasToNameTheSameFrameForTheTableToLeaveTheFlow()
    {
        // Word would put the disagreeing rows in frames of their own; a table is assembled here as one
        // block and there is nowhere to put half of it, so the whole table keeps the flow instead.
        Ww8DocumentReader.AgreedRowFrame([Masthead, Masthead, Masthead]).ShouldBe(Masthead);
        Ww8DocumentReader.AgreedRowFrame([Masthead, Elsewhere]).IsEmpty.ShouldBeTrue();
        Ww8DocumentReader.AgreedRowFrame([Masthead, Ww8TextFramePosition.None]).IsEmpty.ShouldBeTrue();
        Ww8DocumentReader.AgreedRowFrame([]).IsEmpty.ShouldBeTrue();
    }

    private static Ww8LayoutBlock Paragraph(
        string text, Ww8TextFramePosition frame = default)
        => new(new Ww8DocumentReader.Ww8LayoutParagraph(
            SectionIndex: 0,
            Text: text,
            Format: new Text.Layout.ParagraphFormat(),
            FamilyName: null,
            Size: Length.FromPoints(11),
            Weight: 400,
            IsItalic: false,
            Language: null,
            IsInTable: false)
        {
            TextFrame = frame,
        });

    private static Ww8LayoutBlock Table(Ww8TextFramePosition frame)
        => new(new Ww8LayoutTable(
            [Length.FromTwips(4000)],
            [new Ww8LayoutRow([new Ww8LayoutCell(0, 1, 1, default, [])], IsHeader: false)],
            HeaderRowCount: 0,
            LeftIndent: Length.Zero)
        {
            TextFrame = frame,
        });
}

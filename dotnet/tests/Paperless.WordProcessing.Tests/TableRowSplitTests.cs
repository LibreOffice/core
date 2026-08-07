using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A table row whose content outgrows the page it starts on, and what decides whether it may be broken.
/// </summary>
/// <remarks>
/// <para>
/// Writer breaks such a row: <c>SwTabFrame::Split</c> hands it to <c>lcl_InsertNewFollowFlowLine</c>,
/// which gives the same row a second frame on the next page and moves the text that did not fit into it.
/// Moving the whole row instead costs a page every time a long row lands near the bottom of one, which is
/// what a form or a holdover table is made of.
/// </para>
/// <para>
/// The document can forbid it — DOCX's <c>w:cantSplit</c> and its three siblings — and so can the row's
/// own shape: a repeated heading and a row of exact height never break. Both directions are asserted,
/// because a break that ignores the flag is as wrong as one that never happens.
/// </para>
/// </remarks>
public sealed class TableRowSplitTests
{
    /// <summary>
    /// A row taller than the room left on its page leaves what fits behind and carries the rest over.
    /// </summary>
    [Fact]
    public void ARowTallerThanTheRoomLeftIsBrokenAcrossThePage()
    {
        List<LaidOutPage> pages = Paginate(Table(rows: 1, linesPerCell: 15));

        pages.Count.ShouldBe(2, "fifteen lines do not fit on one short page");

        pages[0].Tables.Count.ShouldBe(1);
        pages[1].Tables.Count.ShouldBe(1);

        // The same row on both pages, which is what a break within a row means.
        pages[0].Tables[0].FirstRow.ShouldBe(0);
        pages[1].Tables[0].FirstRow.ShouldBe(0);

        LinesOn(pages[0]).ShouldBeGreaterThan(0);
        LinesOn(pages[1]).ShouldBeGreaterThan(0);
        (LinesOn(pages[0]) + LinesOn(pages[1])).ShouldBe(15, "no line may be lost or drawn twice");
    }

    /// <summary>
    /// The same row, with the document forbidding the break, moves whole and costs the page.
    /// </summary>
    /// <remarks>
    /// The page it left is then empty of the table altogether, which is the behaviour the flag buys and
    /// the reason reading it had to come before breaking anything.
    /// </remarks>
    [Fact]
    public void ARowTheDocumentForbidsBreakingMovesWhole()
    {
        List<LaidOutPage> pages = Paginate(
            Table(rows: 2, linesPerCell: 8, canSplit: false));

        pages.Count.ShouldBe(2);

        // Row one fits; row two does not, may not be broken, and so goes over whole.
        pages[0].Tables[0].RowEnd.ShouldBe(1);
        pages[1].Tables[0].FirstRow.ShouldBe(1);

        LinesOn(pages[0]).ShouldBe(8);
        LinesOn(pages[1]).ShouldBe(8);
    }

    /// <summary>
    /// The rows above the one that breaks stay where they are.
    /// </summary>
    [Fact]
    public void TheRowsBeforeTheBreakStayOnTheFirstPage()
    {
        List<LaidOutPage> pages = Paginate(Table(rows: 3, linesPerCell: 4));

        pages.Count.ShouldBe(2);

        // Two whole rows and part of the third on page one; the rest of the third on page two.
        pages[0].Tables[0].FirstRow.ShouldBe(0);
        pages[0].Tables[0].RowEnd.ShouldBe(3);
        pages[1].Tables[0].FirstRow.ShouldBe(2);

        (LinesOn(pages[0]) + LinesOn(pages[1])).ShouldBe(12);
        LinesOn(pages[1]).ShouldBeGreaterThan(0);
        LinesOn(pages[1]).ShouldBeLessThan(4, "part of the third row belongs to the first page");
    }

    /// <summary>
    /// A row whose height the document stated as exact is a size rather than a floor, so it never breaks.
    /// </summary>
    [Fact]
    public void ARowOfExactHeightIsNeverBroken()
    {
        List<LaidOutPage> pages = Paginate(
            Table(rows: 2, linesPerCell: 8, exactHeight: Length.FromPoints(150)));

        pages.Count.ShouldBe(2);
        pages[0].Tables[0].RowEnd.ShouldBe(1);
        pages[1].Tables[0].FirstRow.ShouldBe(1);
    }

    /// <summary>
    /// A row is as tall as its content plus the space around it, and a part of it must be measured the
    /// same way — otherwise a row that does not fit is judged to fit and moves whole.
    /// </summary>
    /// <remarks>
    /// The two measures come apart at the ends of a cell's flow: a first paragraph's space-before sits
    /// above its first line and a last paragraph's space-after below its last, and both count towards the
    /// row's height (<c>AddParaSpacingToTableCells</c>) while neither is ink. Cutting the row at its last
    /// line therefore looked cheaper than the row itself, so the cut "held every remaining line", the
    /// split was declined as pointless, and the caller — which had already found the row too tall by the
    /// other measure — sent it to the next page and left the difference blank. Measured on
    /// <c>f445896eb008d14c1746fc37d412dc22.docx</c> as 205.8 pt of empty page.
    /// </remarks>
    [Fact]
    public void ARowIsSplitWhenOnlyItsParagraphSpacingDoesNotFit()
    {
        PageTable table = Table(rows: 1, linesPerCell: 4, spacing: Length.FromPoints(20));

        (List<PlacedTableCell> cells, List<Length> heights) =
            TableLayouter.LayOut(table, new DocPoint(Length.Zero, Length.Zero));

        // Room for every line the row holds, and not for the spacing above and below them.
        Length room = heights[0] - Length.FromPoints(1);

        TableLayouter.RowSlice? slice =
            TableLayouter.SliceRow(table.Rows[0], cells, Length.Zero, room);

        slice.ShouldNotBeNull("a row that does not fit must give up the lines that do");
        slice.Value.IsComplete.ShouldBeFalse("something has to be left for the next page");
        slice.Value.Height.ShouldBeLessThanOrEqualTo(room);
        slice.Value.Cells.Sum(cell => cell.Content?.Lines.Count ?? 0)
            .ShouldBeInRange(1, 3, "some lines stay behind and at least one goes over");
    }

    /// <summary>
    /// A stated minimum row height is a floor on the row's size and <em>not</em> a bar on breaking it:
    /// a row whose declared minimum is far larger than the room left is still broken where it stands.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This pins a negative result that cost a round to establish, because the opposite is plausible and
    /// is even spelled out in Writer's own source — "the remaining size is less than the minimum row
    /// height, then don't even try to split the row, just move it forward"
    /// (<c>SwTabFrame::Split</c>, <c>sw/source/core/layout/tabfrm.cxx:1188-1196</c>). That branch is
    /// reached only for a table inside a splittable fly, and LibreOffice 24.2.7.2 does not apply it to
    /// body text.
    /// </para>
    /// <para>
    /// Measured on the fixture beside this test — <c>table-row-min-height</c>, whose middle row states an
    /// at-least height of 5.2 cm with 3.5 cm left on the page — and on two further sweeps of the same
    /// shape at A4 with the declared height varied from nought to 10 cm: LibreOffice breaks the row in
    /// every one of them, keeping exactly the lines that fit, and the declared height changes nothing.
    /// Making the minimum bar the break instead cost <c>batch-008</c> a document and bought none.
    /// </para>
    /// </remarks>
    [Fact]
    public void AStatedMinimumHeightDoesNotStopARowBeingBroken()
    {
        PageTable free = Table(rows: 2, linesPerCell: 8);
        PageTable floored = Table(rows: 2, linesPerCell: 8, minHeight: Length.FromTwips(1400));

        // The floor is under the row's own height, so the two tables lay out identically and only the
        // break decision could differ.
        TableLayouter.LayOut(floored, new DocPoint(Length.Zero, Length.Zero)).RowHeights
            .ShouldBe(TableLayouter.LayOut(free, new DocPoint(Length.Zero, Length.Zero)).RowHeights);

        List<LaidOutPage> broken = Paginate(free);
        List<LaidOutPage> stillBroken = Paginate(floored);

        broken.Count.ShouldBe(2);
        stillBroken.Count.ShouldBe(2);
        LinesOn(broken[0]).ShouldBeGreaterThan(8, "the second row gives up the lines that fit");
        LinesOn(stillBroken[0]).ShouldBe(LinesOn(broken[0]), "the floor decides nothing here");
        LinesOn(stillBroken[1]).ShouldBe(LinesOn(broken[1]));
    }

    /// <summary>
    /// The same negative result on a real document, in all four formats.
    /// </summary>
    /// <remarks>
    /// <c>table-row-min-height</c> is a 10 cm page whose middle row states an at-least height of 5.2 cm
    /// and holds twelve paragraphs, with about 3.5 cm left on page one. LibreOffice 24.2.7.2 breaks that
    /// row and keeps the seven paragraphs that fit — verified by converting the one flat-ODF source to
    /// each format and rendering all of them, so a reader that drops the height or reads it as a bar on
    /// breaking shows up here rather than three batches later.
    /// </remarks>
    [Theory]
    [InlineData("table-row-min-height.docx")]
    [InlineData("table-row-min-height.doc")]
    [InlineData("table-row-min-height.odt")]
    [InlineData("table-row-min-height.rtf")]
    public void TheFlooredRowOfTheCorpusFixtureIsBrokenRatherThanMoved(string name)
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        pages.Pages.Count.ShouldBe(2, $"{name} is two pages");

        List<PlacedTable> first = [.. pages.Pages[0].Tables];
        List<PlacedTable> second = [.. pages.Pages[1].Tables];

        first.Count.ShouldBe(1);
        second.Count.ShouldBe(1);

        // Row one is the floored row, and it is on both pages: the part on page one ends past it and the
        // part on page two begins at it.
        first[0].FirstRow.ShouldBe(0);
        first[0].RowEnd.ShouldBeGreaterThan(1, $"{name}: the floored row must give up the lines that fit");
        second[0].FirstRow.ShouldBe(1);

        LinesIn(first[0], 1).ShouldBeGreaterThan(0, $"{name}: part of the floored row stays behind");
        LinesIn(second[0], 1).ShouldBeGreaterThan(0, $"{name}: the rest of it carries over");
    }

    /// <summary>How many lines of one row of a table part were drawn, over every cell.</summary>
    private static int LinesIn(PlacedTable table, int row)
        => table.Cells.Where(cell => cell.Row == row).Sum(cell => cell.Content?.Lines.Count ?? 0);

    /// <summary>How many of a page's table lines were drawn, over every cell of every part.</summary>
    private static int LinesOn(LaidOutPage page)
        => page.Tables
            .SelectMany(table => table.Cells)
            .Sum(cell => cell.Content?.Lines.Count ?? 0);

    private static List<LaidOutPage> Paginate(PageTable table)
        => new Paginator(PaginationOptions.Word).Paginate(
            [table], new WritingSection { Page = Geometry });

    private static PageTable Table(
        int rows,
        int linesPerCell,
        bool canSplit = true,
        Length exactHeight = default,
        Length spacing = default,
        Length minHeight = default)
        => new()
        {
            ColumnWidths = [Length.FromTwips(4000)],
            Rows =
            [
                .. Enumerable.Range(0, rows).Select(row => new PageTableRow
                {
                    Cells =
                    [
                        new PageTableCell
                        {
                            Blocks =
                            [
                                .. Enumerable.Range(0, linesPerCell)
                                    .Select(line => Paragraph($"row {row} line {line}", spacing)),
                            ],
                        },
                    ],
                    CanSplit = canSplit,
                    MinHeight = exactHeight > Length.Zero ? exactHeight : minHeight,
                    HasExactHeight = exactHeight > Length.Zero,
                }),
            ],
        };

    /// <summary>
    /// A page just tall enough for sixteen of these lines, so a row of more than that has to give.
    /// </summary>
    private static PageGeometry Geometry => new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(4200)),
        Margins = new PageMargins(
            Length.FromTwips(720), Length.FromTwips(720),
            Length.FromTwips(720), Length.FromTwips(720)),
    };

    private static PageParagraph Paragraph(string text, Length spacing = default) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
        Format = ParagraphFormat.Default with { SpaceBefore = spacing, SpaceAfter = spacing },
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}

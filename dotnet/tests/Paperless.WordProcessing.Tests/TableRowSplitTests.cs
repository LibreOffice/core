using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
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
        Length exactHeight = default)
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
                                    .Select(line => Paragraph($"row {row} line {line}")),
                            ],
                        },
                    ],
                    CanSplit = canSplit,
                    MinHeight = exactHeight,
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

    private static PageParagraph Paragraph(string text) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}

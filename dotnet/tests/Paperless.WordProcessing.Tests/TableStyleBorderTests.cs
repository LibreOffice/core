using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A table whose borders live in its <em>style</em>, which is where <c>Table Grid</c> keeps every line
/// it draws.
/// </summary>
/// <remarks>
/// <para>
/// Two gaps in one: a table style's <c>w:tblPr</c> was never consulted, and <c>w:insideH</c> and
/// <c>w:insideV</c> were never read from anywhere. <c>Table Grid</c> — the style Word puts on nearly
/// every table — states nothing but a <c>w:tblBorders</c> holding all six sides, and a table using it
/// states no borders of its own, so between them the commonest table in Word drew no line at all.
/// </para>
/// <para>
/// Which of the six sides reaches a given cell is <c>lcl_computeCellBorders</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>:126), and a table too small for
/// an interior line has it erased first (the same file, 915–940). Measured against LibreOffice 24.2.7.2
/// by counting the stroke operators in its own PDF of these four fixtures: 8 for the three-by-three, 6
/// for the single column, 6 for the single row and 4 for the single cell — and 0, 0, 0, 0 from this
/// engine before the fix.
/// </para>
/// <para>
/// It is height as well as ink. A border is a band the text may not enter and neighbouring rows share
/// it, so the three-by-three's last line moved from 109.945 to 111.445 pt against the reference's
/// 111.796 — the standing 0.35 pt being where the first line's leading sits, which is a separate
/// difference and unrelated to borders.
/// </para>
/// </remarks>
public sealed class TableStyleBorderTests
{
    private static readonly Length HalfPoint = Length.FromPoints(0.5);

    private static readonly Length Thick = Length.FromPoints(3);

    /// <summary>Every one of a three-by-three's cells is boxed, from the style alone.</summary>
    /// <remarks>
    /// Asserted per cell rather than as a count, because the interior lines are the half that was
    /// missing entirely and they appear only on the sides an edge cell does not have.
    /// </remarks>
    [Fact]
    public void EveryCellOfAGridIsBoxed()
    {
        PageTable table = Table("table-style-grid.docx");

        foreach (PageTableRow row in table.Rows)
        {
            foreach (PageTableCell cell in row.Cells)
            {
                cell.Borders.Left.Width.ShouldBe(HalfPoint);
                cell.Borders.Right.Width.ShouldBe(HalfPoint);
                cell.Borders.Top.Width.ShouldBe(HalfPoint);
                cell.Borders.Bottom.Width.ShouldBe(HalfPoint);
            }
        }
    }

    /// <summary>
    /// The interior lines are the interior ones: three points inside, half a point round the edge.
    /// </summary>
    /// <remarks>
    /// The <c>table-style-thick-inside</c> fixtures state <c>w:insideH</c> and <c>w:insideV</c> at
    /// <c>w:sz="24"</c> against an outline of 4, which is the observation that separates "the interior
    /// line is drawn" from "the outline is drawn twice" — on <c>Table Grid</c>, where all six sides are
    /// the same width, the two are indistinguishable in the output. LibreOffice's own PDF of this
    /// fixture holds four strokes at 0.5 pt and four at 3.
    /// </remarks>
    [Fact]
    public void TheInteriorLinesAreThickerThanTheOutline()
    {
        PageTable table = Table("table-style-thick-inside.docx");

        PageTableCell middle = table.Rows[1].Cells[1];
        middle.Left().ShouldBe(Thick);
        middle.Right().ShouldBe(Thick);
        middle.Top().ShouldBe(Thick);
        middle.Bottom().ShouldBe(Thick);

        PageTableCell corner = table.Rows[0].Cells[0];
        corner.Left().ShouldBe(HalfPoint, "the table's own left edge");
        corner.Top().ShouldBe(HalfPoint);
        corner.Right().ShouldBe(Thick, "and its right is the first interior line");
        corner.Bottom().ShouldBe(Thick);
    }

    /// <summary>
    /// A table one column wide gets no vertical interior line, because there is no interior to divide.
    /// </summary>
    /// <remarks>
    /// LibreOffice erases <c>insideV</c> outright for such a table
    /// (<c>DomainMapperTableHandler.cxx</c>:925-935), and its PDF of the thick fixture proves it: four
    /// strokes at 0.5 and only two at 3, which are the two horizontal interior lines. Keeping it would
    /// draw the column's own edges three points wide instead of half a point.
    /// </remarks>
    [Fact]
    public void ASingleColumnStillGetsItsHorizontalRules()
    {
        PageTable table = Table("table-style-thick-inside-onecolumn.docx");

        table.Rows.Count.ShouldBe(3);
        table.Rows[1].Cells[0].Top().ShouldBe(Thick, "the interior rule above the middle row");
        table.Rows[1].Cells[0].Bottom().ShouldBe(Thick);
        table.Rows[1].Cells[0].Left().ShouldBe(HalfPoint, "the outline, not an interior line");
        table.Rows[1].Cells[0].Right().ShouldBe(HalfPoint);
    }

    /// <summary>And a table one row tall gets no horizontal interior line, for the same reason.</summary>
    [Fact]
    public void ASingleRowStillGetsItsVerticalRules()
    {
        PageTable table = Table("table-style-thick-inside-onerow.docx");

        table.Rows.Count.ShouldBe(1);
        table.Rows[0].Cells[1].Left().ShouldBe(Thick, "the interior rule left of the middle cell");
        table.Rows[0].Cells[1].Right().ShouldBe(Thick);
        table.Rows[0].Cells[1].Top().ShouldBe(HalfPoint, "the outline, not an interior line");
        table.Rows[0].Cells[1].Bottom().ShouldBe(HalfPoint);
    }

    /// <summary>A one-cell table is all outline and no interior at all.</summary>
    [Fact]
    public void ASingleCellIsAllOutline()
    {
        PageTable table = Table("table-style-grid-onecell.docx");

        PageTableCell only = table.Rows[0].Cells[0];
        only.Left().ShouldBe(HalfPoint);
        only.Right().ShouldBe(HalfPoint);
        only.Top().ShouldBe(HalfPoint);
        only.Bottom().ShouldBe(HalfPoint);
    }

    private static PageTable Table(string name)
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageTable>().Single();
    }
}

/// <summary>Reading a cell's four border widths without four lines of noise at each use.</summary>
internal static class CellBorderWidths
{
    public static Length Left(this PageTableCell cell) => cell.Borders.Left.Width;

    public static Length Right(this PageTableCell cell) => cell.Borders.Right.Width;

    public static Length Top(this PageTableCell cell) => cell.Borders.Top.Width;

    public static Length Bottom(this PageTableCell cell) => cell.Borders.Bottom.Width;
}

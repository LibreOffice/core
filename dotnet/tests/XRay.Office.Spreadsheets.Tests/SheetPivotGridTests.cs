using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The border grid a pivot table is given when it is imported, which no cell of the file states.
/// </summary>
/// <remarks>
/// <para>
/// Calc does not print the formatting Excel put on a pivot table's cells: it imports the
/// definition and regenerates the output through <c>ScDPOutput</c>, which rules the result in
/// two widths of plain black — 20 twips inside the table and 40 on its edge
/// (<c>sc/source/core/data/dpoutput.cxx</c>:76-79). On a workbook whose pivot cells state no
/// border of their own that grid is the <em>whole</em> of what the reference draws around the
/// table, and reading none of it was the largest single residual in the sheets corpus:
/// <c>alle einzeln.xlsx</c> at <b>225.44</b> of summed unsigned ink over 186 pages.
/// </para>
/// <para>
/// <c>features/sheet-pivot-grid.xlsx</c> is written by
/// <c>probes/pivot-gen-r107/make-fixture.py</c> — by hand, so that every number below can be
/// traced to one element of the file. Two row fields and two data fields, with a subtotal under
/// each outer member and a grand total, which is the four shapes <c>ScDPOutput</c> frames
/// differently: a member's block, a nested member's own column, a subtotal's strip across the
/// row headers, and the table's outer edge.
/// </para>
/// <para>
/// <strong>Every expected number is LibreOffice 26.2.4.2's own.</strong> They are read out of
/// the <c>.fods</c> that binary writes for this same workbook, where the generated grid appears
/// as ordinary <c>fo:border*</c> on automatic styles parented to <c>Pivot Table *</c> —
/// <c>0.99pt</c> for the reference's 20 twips and <c>2.01pt</c> for its 40, both rounded through
/// hundredths of a millimetre. The reference lays the table out on exactly the cells Excel
/// wrote, <c>A1:D9</c>, which is what makes the comparison cell for cell.
/// </para>
/// </remarks>
public sealed class SheetPivotGridTests
{
    private const string Fixture = "sheet-pivot-grid.xlsx";

    /// <summary>
    /// The whole grid, one row of the table per string, four cells to a row, four edges to a
    /// cell in the order left, right, top, bottom, in twips.
    /// </summary>
    /// <remarks>
    /// Transcribed from 26.2.4.2's <c>.fods</c> of the fixture rather than typed: see
    /// <c>probes/pivot-gen-r107/fods-borders.py</c>, which expands the sheet into a cell grid and
    /// resolves each cell's style chain.
    /// </remarks>
    private static readonly string[] Reference =
    [
        "40,0,40,0   0,20,40,0   20,20,40,20  0,40,40,20",
        "40,20,20,20 20,20,20,20 20,0,0,20    0,40,0,20",
        "40,20,20,0  20,20,20,0  20,0,20,0    0,40,20,0",
        "40,20,0,20  20,20,0,20  20,0,0,20    0,40,0,20",
        "40,0,20,20  0,20,20,20  20,0,20,20   0,40,20,20",
        "40,20,20,0  20,20,20,0  20,0,20,0    0,40,20,0",
        "40,20,0,20  20,20,0,20  20,0,0,20    0,40,0,20",
        "40,0,20,20  0,20,20,20  20,0,20,20   0,40,20,20",
        "40,0,20,40  0,20,20,40  20,0,20,40   0,40,20,40",
    ];

    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        return (SpreadsheetPages)document.Layout();
    }

    private static SheetCellBorders Borders(SpreadsheetPages pages, int sheet, int row, int column)
        => pages.Sheets[sheet].Formatting.At(row, column).Borders;

    private static SheetCellFormat Format(SpreadsheetPages pages, int sheet, int row, int column)
        => pages.Sheets[sheet].Formats.At(row, column);

    private static string Spell(SheetCellBorders borders)
        => string.Join(
            ",",
            new[] { borders.Left, borders.Right, borders.Top, borders.Bottom }
                .Select(edge => edge.IsNone ? 0 : edge.Primary.Twips));

    /// <summary>Every edge of every cell of the table is the reference's.</summary>
    [Fact]
    public void TheGeneratedGridIsTheReferencesCellForCell()
    {
        SpreadsheetPages pages = Pages();
        List<string> wrong = [];

        for (int row = 0; row < Reference.Length; row++)
        {
            string[] expected = Reference[row].Split(' ', StringSplitOptions.RemoveEmptyEntries);
            for (int column = 0; column < expected.Length; column++)
            {
                string got = Spell(Borders(pages, 1, row, column));
                if (got != expected[column])
                    wrong.Add($"{(char)('A' + column)}{row + 1}: {got} against {expected[column]}");
            }
        }

        wrong.ShouldBeEmpty();
    }

    /// <summary>
    /// The table's own edge is the wider of the two rules and everything inside it the narrower.
    /// </summary>
    /// <remarks>
    /// <c>OutputBlockFrame</c> (<c>dpoutput.cxx</c>:217-258) chooses per edge, comparing the
    /// block's own bounds against <c>mnTabStartCol</c>, <c>mnTabStartRow</c>, <c>mnTabEndCol</c>
    /// and <c>mnTabEndRow</c> — so the same call draws a 40-twip line on one side of a block and
    /// a 20-twip one on the other.
    /// </remarks>
    [Fact]
    public void TheOuterEdgeIsFortyTwipsAndTheInnerLinesAreTwenty()
    {
        SpreadsheetPages pages = Pages();

        Borders(pages, 1, 0, 0).Left.Primary.ShouldBe(Length.FromTwips(40), "A1's left is the table's");
        Borders(pages, 1, 0, 0).Top.Primary.ShouldBe(Length.FromTwips(40), "A1's top is the table's");
        Borders(pages, 1, 8, 3).Right.Primary.ShouldBe(Length.FromTwips(40), "D9's right is the table's");
        Borders(pages, 1, 8, 3).Bottom.Primary.ShouldBe(Length.FromTwips(40), "D9's bottom is the table's");

        Borders(pages, 1, 2, 0).Right.Primary.ShouldBe(Length.FromTwips(20), "between the row fields");
        Borders(pages, 1, 2, 2).Left.Primary.ShouldBe(Length.FromTwips(20), "at the data area's edge");
    }

    /// <summary>The grid is drawn in plain black, whatever the workbook's theme says.</summary>
    /// <remarks><c>SC_DP_FRAME_COLOR</c> is <c>Color(0,0,0)</c> outright, <c>dpoutput.cxx</c>:79.</remarks>
    [Fact]
    public void TheGridIsPlainBlack()
        => Borders(Pages(), 1, 4, 0).Top.Colour.ShouldBe(Colour.Black);

    /// <summary>
    /// A subtotal row is framed across the row headers as one block, so its inner cells take a
    /// top and a bottom and no side at all.
    /// </summary>
    /// <remarks>
    /// This is <c>HeaderCell</c>'s own frame (<c>dpoutput.cxx</c>:788), which
    /// <c>outputRowHeader</c> never draws: a subtotal member takes the <c>AddRow</c> branch there
    /// and nothing else. Measured by taking the call out of the predictor in
    /// <c>probes/pivot-gen-r107/pivot-grid.py</c>: without it this fixture loses 10 of its 99
    /// stated edges and <c>alle einzeln.xlsx</c> 30 of its 30250, all of them absent rather than
    /// drawn at the wrong width.
    /// </remarks>
    [Fact]
    public void ASubtotalRowIsFramedAcrossTheRowHeaders()
    {
        SpreadsheetPages pages = Pages();

        // A5 is `North Result`, B5 the cell beside it: one block A5:B5, so B5 has no left edge.
        Spell(Borders(pages, 1, 4, 0)).ShouldBe("40,0,20,20");
        Spell(Borders(pages, 1, 4, 1)).ShouldBe("0,20,20,20");
    }

    /// <summary>
    /// A sheet with no pivot table on it gains nothing, in a workbook where another sheet has one.
    /// </summary>
    /// <remarks>
    /// The regression this guards is not hypothetical and cost the round that found it a whole
    /// measurement: <c>XlsxCellDecoration.Read</c> answers the shared
    /// <see cref="SheetFormatting.Empty"/> singleton for a sheet that states no decoration, and a
    /// pivot sheet is very often exactly such a sheet — so writing the generated grid into the
    /// instance it was handed put <c>alle einzeln.xlsx</c>'s pivot grid on 36 pages of its data
    /// sheet and cost 276 of summed ink there.
    /// </remarks>
    [Fact]
    public void TheOtherSheetOfTheSameWorkbookIsUntouched()
    {
        SpreadsheetPages pages = Pages();

        for (int row = 0; row < 9; row++)
        {
            for (int column = 0; column < 4; column++)
                Borders(pages, 0, row, column).IsNone.ShouldBeTrue($"the data sheet at {row},{column}");
        }
    }

    /// <summary>
    /// The generated styles: <c>Category</c> and <c>Title</c> justify left, <c>Title</c> and
    /// <c>Result</c> are bold, and the other four state nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One string per row of the table, one letter per cell: <c>.</c> a style that states
    /// nothing, <c>L</c> left justified, <c>B</c> bold, <c>X</c> both. Read out of the same
    /// <c>.fods</c> as the border table — the reference writes each generated style as a named
    /// <c>Pivot_20_Table_20_*</c> cell style and parents the cell's automatic style to it, so
    /// what a cell takes is the parent's name.
    /// </para>
    /// <para>
    /// The left justification is not cosmetic on a pivot's row headers: a member whose caption
    /// is a number — this fixture's outer field is text, but <c>alle einzeln.xlsx</c>'s second
    /// row field is a start number — is drawn against the <em>left</em> edge of its column
    /// where <see cref="SheetHorizontalAlignment.General"/> would put it against the right.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheGeneratedStylesAreTheReferencesCellForCell()
    {
        SpreadsheetPages pages = Pages();
        string[] reference =
        [
            "....",   // the corner and the data field's own caption
            "..LL",   // the row-field headers, then the column members
            "LL..",
            "LL..",
            "XXBB",   // North's subtotal
            "LL..",
            "LL..",
            "XXBB",   // South's subtotal
            "XXBB",   // the grand total
        ];

        List<string> wrong = [];
        for (int row = 0; row < reference.Length; row++)
        {
            for (int column = 0; column < 4; column++)
            {
                SheetCellFormat format = Format(pages, 1, row, column);
                bool left = format.Horizontal == SheetHorizontalAlignment.Left;
                bool bold = format.FontWeight >= 700;
                char got = (left, bold) switch
                {
                    (true, true) => 'X', (true, false) => 'L', (false, true) => 'B', _ => '.',
                };
                if (got != reference[row][column])
                    wrong.Add($"{(char)('A' + column)}{row + 1}: {got} against {reference[row][column]}");
            }
        }

        wrong.ShouldBeEmpty();
    }

    /// <summary>
    /// A row field that is not the innermost indents its member cell by thirteen pixels.
    /// </summary>
    /// <remarks>
    /// 195 twips, which the reference's own <c>.fods</c> of this fixture writes as
    /// <c>fo:margin-left="0.1354in"</c> on exactly two cells — the two places the outer field's
    /// members start. The innermost field takes none, because <c>bLast</c> drops
    /// <c>nMinIndentLevel</c> out of the sum (<c>dpoutput.cxx</c>:1135-1137).
    /// </remarks>
    [Fact]
    public void AnOuterRowFieldsMemberIsIndented()
    {
        SpreadsheetPages pages = Pages();

        Format(pages, 1, 2, 0).Indent.ShouldBe(Length.FromTwips(195), "A3, where North starts");
        Format(pages, 1, 5, 0).Indent.ShouldBe(Length.FromTwips(195), "A6, where South starts");

        Format(pages, 1, 3, 0).Indent.ShouldBe(Length.Zero, "A4 continues North and states nothing");
        Format(pages, 1, 2, 1).Indent.ShouldBe(Length.Zero, "B3 is the innermost field");
        Format(pages, 1, 4, 0).Indent.ShouldBe(Length.Zero, "A5 is a subtotal");
    }
}

using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The two pivot layouts that are not one row field in one column — a hidden header and a run of
/// compact row fields packed into a single column — and the clearing that decides what a pivot
/// cell keeps of the workbook's own formatting.
/// </summary>
/// <remarks>
/// <para>
/// <c>features/sheet-pivot-packed.xlsx</c> is written by
/// <c>probes/pivot-res-r108/make-fixture.py</c> — by hand, so that every number below can be
/// traced to one element of the file, and over a <c>styles.xml</c> that states one border and it
/// is empty. Its two <c>cellXf</c> are there to be thrown away or kept: the workbook default
/// justifies right, which the clearing keeps because it is the <c>Normal</c> cell style's, and
/// the other states a centring, an indent and a bold weight, which the clearing does not. No
/// edge, no generated style and no indent asserted here is reachable from the file.
/// </para>
/// <para>
/// Its <c>Cleared</c> sheet is <c>Hidden</c> again over cells that hard-state a centring, an
/// indent and a bold weight, none of which survives the clearing the reference does before it
/// draws. Its <c>Hidden</c> sheet states <c>firstHeaderRow="0"</c>, which
/// <c>mpDPObject-&gt;SetHideHeader(maLocationModel.mnFirstHeaderRow == 0)</c>
/// (<c>sc/source/filter/oox/pivottablebuffer.cxx</c>:1368) turns into <c>mnHeaderSize = 0</c>
/// (<c>dpoutput.cxx</c>:884): there is no field-button row, so <c>mnMemberStartRow</c> is
/// <c>mnTabStartRow</c> and the column members take the table's own outer rule on their top.
/// Its <c>Packed</c> sheet states two compact row fields, which
/// <c>GetColumnsForRowFields</c> (<c>:854-868</c>) packs into one row-label column — the button
/// cell then takes <c>MultiFieldCell</c> rather than <c>FieldCell</c> and so is not boxed
/// (<c>:1087-1090</c>), and the outer field's drill step lands on the inner field's members in
/// the same column (<c>:1135-1137</c>).
/// </para>
/// <para>
/// <strong>Every expected number is LibreOffice 26.2.4.2's own</strong>, read out of the
/// <c>.fods</c> that binary writes for this same workbook — <c>0.99pt</c> for the reference's
/// 20 twips and <c>2.01pt</c> for its 40. The reference lays both tables out on exactly the
/// cells Excel wrote, <c>A1:C4</c>, <c>A1:C9</c> and <c>A1:C4</c>, which is what makes the
/// comparison cell for cell. The C++ tree those line numbers are read in declares 27.2.0.0.alpha0+ and is not the
/// reference binary's source; the <c>.fods</c> is.
/// </para>
/// </remarks>
public sealed class SheetPivotPackedTests
{
    private const string Fixture = "sheet-pivot-packed.xlsx";
    private const int BoldWeight = 700;
    private const int Data = 0;
    private const int Hidden = 1;
    private const int Packed = 2;
    private const int Cleared = 3;

    /// <summary>
    /// The hidden-header table, one row per string, three cells to a row, four edges to a cell in
    /// the order left, right, top, bottom, in twips.
    /// </summary>
    private static readonly string[] HiddenReference =
    [
        "40,20,40,20  20,0,40,20   0,40,40,20",
        "40,20,20,0   20,0,20,0    0,40,20,0",
        "40,20,0,0    20,0,0,20    0,40,0,20",
        "40,20,20,40  20,0,20,40   0,40,20,40",
    ];

    /// <summary>The packed table, in the same spelling.</summary>
    private static readonly string[] PackedReference =
    [
        "40,20,40,0   20,20,40,20  0,40,40,20",
        "40,20,0,20   20,0,0,20    0,40,0,20",
        "40,20,20,0   20,20,20,0   0,40,20,0",
        "40,20,0,0    20,20,0,0    0,40,0,0",
        "40,20,0,20   20,20,0,20   0,40,0,20",
        "40,20,20,0   20,20,20,0   0,40,20,0",
        "40,20,0,0    20,20,0,0    0,40,0,0",
        "40,20,0,20   20,20,0,20   0,40,0,20",
        "40,20,20,40  20,0,20,40   0,40,20,40",
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

    private static void Compare(SpreadsheetPages pages, int sheet, string[] reference)
    {
        List<string> wrong = [];
        for (int row = 0; row < reference.Length; row++)
        {
            string[] expected = reference[row].Split(' ', StringSplitOptions.RemoveEmptyEntries);
            for (int column = 0; column < expected.Length; column++)
            {
                string got = Spell(Borders(pages, sheet, row, column));
                if (got != expected[column])
                    wrong.Add($"{(char)('A' + column)}{row + 1}: {got} against {expected[column]}");
            }
        }

        wrong.ShouldBeEmpty();
    }

    /// <summary>Every edge of the hidden-header table is the reference's.</summary>
    [Fact]
    public void TheHiddenHeaderGridIsTheReferencesCellForCell()
        => Compare(Pages(), Hidden, HiddenReference);

    /// <summary>Every edge of the packed table is the reference's.</summary>
    [Fact]
    public void ThePackedGridIsTheReferencesCellForCell()
        => Compare(Pages(), Packed, PackedReference);

    /// <summary>
    /// A hidden header puts the column members on the table's first row, where they take the
    /// outer rule.
    /// </summary>
    /// <remarks>
    /// This is the whole of what <c>mnHeaderSize = 0</c> does: the member row is the start row,
    /// so <c>OutputBlockFrame</c> sees <c>nStartRow == mnTabStartRow</c> and writes the 40-twip
    /// line where a table with a button row would have written the 20-twip one a row lower.
    /// The row Excel wrote for the button is not skipped — the table simply has one row fewer
    /// and still lands on <c>A1:C4</c>.
    /// </remarks>
    [Fact]
    public void AHiddenHeadersMemberRowTakesTheTablesOwnTopRule()
    {
        SpreadsheetPages pages = Pages();

        Borders(pages, Hidden, 0, 1).Top.Primary.ShouldBe(Length.FromTwips(40), "B1 is the table's top");
        Format(pages, Hidden, 0, 1).Horizontal
            .ShouldBe(SheetHorizontalAlignment.Left, "B1 is a column member, so Category");
        Borders(pages, Hidden, 3, 0).Bottom.Primary
            .ShouldBe(Length.FromTwips(40), "A4 is the table's last row");
        Borders(pages, Hidden, 4, 0).IsNone.ShouldBeTrue("A5 is outside the table");
    }

    /// <summary>
    /// Packed row fields share one column, and the button cell above them is not boxed.
    /// </summary>
    /// <remarks>
    /// <c>MultiFieldCell</c> (<c>dpoutput.cxx</c>:794-812) writes the caption and the button
    /// flag and applies the <c>Field</c> style, which carries nothing — it never calls
    /// <c>lcl_SetFrame</c>, so A2 has no top edge of its own where a <c>FieldCell</c> would
    /// have given it one.
    /// </remarks>
    [Fact]
    public void ThePackedButtonCellIsNotBoxed()
    {
        SpreadsheetPages pages = Pages();

        Spell(Borders(pages, Packed, 1, 0)).ShouldBe("40,20,0,20", "A2, the packed button cell");
        Format(pages, Packed, 1, 0).Horizontal
            .ShouldBe(SheetHorizontalAlignment.Right, "A2 takes no generated style");
        Borders(pages, Packed, 2, 1).Left.Primary
            .ShouldBe(Length.FromTwips(20), "B3 is the data area, one column in from A");
    }

    /// <summary>
    /// Every member of a packed column is indented, the innermost field's included.
    /// </summary>
    /// <remarks>
    /// <c>nIndent = 13 * (bLast ? nFieldIndentLevel : nMinIndentLevel + nFieldIndentLevel)</c>
    /// px (<c>dpoutput.cxx</c>:1135-1137): the outer field is not last so it takes the drill
    /// step, and the inner field is last but sits at <c>nFieldIndentLevel</c> 1 because the
    /// field outside it was packed into the same column. Both come to 195 twips, which the
    /// reference's own <c>.fods</c> writes as <c>fo:margin-left="0.1354in"</c> on all six member
    /// cells and on neither the button cell nor the grand total.
    /// </remarks>
    [Fact]
    public void EveryMemberOfAPackedColumnIsIndented()
    {
        SpreadsheetPages pages = Pages();

        for (int row = 2; row <= 7; row++)
        {
            Format(pages, Packed, row, 0).Indent
                .ShouldBe(Length.FromTwips(195), $"A{row + 1} is a member of the packed column");
        }

        Format(pages, Packed, 1, 0).Indent.ShouldBe(Length.Zero, "A2 is the button cell");
        Format(pages, Packed, 8, 0).Indent.ShouldBe(Length.Zero, "A9 is the grand total");
        Format(pages, Packed, 2, 1).Indent.ShouldBe(Length.Zero, "B3 is a data cell");
    }

    /// <summary>
    /// A cell of a pivot's range keeps nothing the workbook hard-stated on it, and keeps
    /// everything the workbook's Default cell style states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The reference empties the range twice before <c>ScDPOutput</c> writes —
    /// <c>clearContents(VALUE | … | HARDATTR | STYLES | …)</c> over the stated range
    /// (<c>pivottablebuffer.cxx</c>:1331-1336) and
    /// <c>DeleteAreaTab(…, InsertDeleteFlags::ALL)</c> over the computed one
    /// (<c>dpoutput.cxx</c>:1226) — so the generated styles replace what a pivot cell states
    /// rather than merging with it. The <c>Cleared</c> sheet states
    /// <c>horizontal="center" indent="3"</c> and a bold font on every one of its cells and the
    /// reference draws it exactly as it draws <c>Hidden</c>, which states none of them.
    /// </para>
    /// <para>
    /// What clearing leaves is the Default cell style, not nothing: the fixture's <c>Normal</c>
    /// <c>cellStyle</c> justifies right, and the reference's own <c>.fods</c> gives every cell
    /// of both pivots that has no generated style of its own <c>fo:text-align="end"</c>.
    /// <c>049_Expenses_calculator</c> is the corpus witness — 47 cells of its pivot resolve a
    /// left justification and a 210-twip indent that way and the reference keeps all 47.
    /// </para>
    /// </remarks>
    [Fact]
    public void AHardStatedFormatInsideThePivotIsCleared()
    {
        SpreadsheetPages pages = Pages();
        List<string> wrong = [];

        for (int row = 0; row < 4; row++)
        {
            for (int column = 0; column < 3; column++)
            {
                SheetCellFormat cleared = Format(pages, Cleared, row, column);
                SheetCellFormat plain = Format(pages, Hidden, row, column);
                string got = $"{cleared.Horizontal},{cleared.FontWeight},{cleared.Indent.Twips}";
                string want = $"{plain.Horizontal},{plain.FontWeight},{plain.Indent.Twips}";
                if (got != want)
                    wrong.Add($"{(char)('A' + column)}{row + 1}: {got} against {want}");
            }
        }

        wrong.ShouldBeEmpty();

        // And what the sheet's own Default states survives: nothing generated touches B2.
        Format(pages, Cleared, 1, 1).Horizontal
            .ShouldBe(SheetHorizontalAlignment.Right, "B2 falls back to the Default cell style");
        Format(pages, Cleared, 1, 1).Indent.ShouldBe(Length.Zero, "and the Default states none");
        Format(pages, Cleared, 0, 0).FontWeight
            .ShouldBeLessThan(BoldWeight, "A1 is a corner cell and the bold font is gone");
    }

    /// <summary>The clearing stops at the pivot's own rectangle.</summary>
    [Fact]
    public void ACellOutsideThePivotKeepsWhatItStates()
    {
        SpreadsheetPages pages = Pages();

        // The Cleared sheet's pivot is A1:C4; row 5 holds nothing and states nothing, and the
        // Data sheet's cells are outside every pivot on the workbook.
        Format(pages, Data, 0, 0).Horizontal
            .ShouldBe(SheetHorizontalAlignment.Right, "A1 of the data sheet");
        Format(pages, Data, 0, 0).Indent.ShouldBe(Length.Zero);
    }

    /// <summary>The sheet holding the source data gains nothing from either pivot.</summary>
    [Fact]
    public void TheDataSheetIsUntouched()
    {
        SpreadsheetPages pages = Pages();

        for (int row = 0; row < 9; row++)
        {
            for (int column = 0; column < 3; column++)
                Borders(pages, Data, row, column).IsNone.ShouldBeTrue($"the data sheet at {row},{column}");
        }
    }
}

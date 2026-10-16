using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What a pivot table's own <c>&lt;format&gt;</c> records put back over the cleared range.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScDPOutput::Output</c> empties the output range, writes the results, applies the four
/// generated <c>Pivot Table *</c> styles, and then ends with <c>maFormatOutput.apply</c>
/// (<c>sc/source/core/data/dpoutput.cxx</c>:1190). Everything a pivot cell shows that is not one
/// of those four styles comes from a record.
/// </para>
/// <para>
/// <c>regression/pivot-format-records.xlsx</c> is <c>pivot-default-style.xlsx</c> with a
/// <c>&lt;formats&gt;</c> element added and nothing else changed, so the pair is a single-variable
/// experiment: the control shows the clearing bare and this one shows what the records put back.
/// Five records, one per arm, and <strong>every expected value below was read out of the
/// <c>.fods</c> LibreOffice 26.2.4.2 writes for this same file</strong>
/// (<c>probes/pivot-fmt-r110/make-format-fixture.py</c>, <c>fods-face.py</c>):
/// </para>
/// <list type="table">
/// <item><description>A1 and B1 — the corner and the first data field's header — resolve to the
/// <c>Normal</c> style, Liberation Sans 11 black.</description></item>
/// <item><description>C1 is red, from a <em>label</em> record with one reference on the
/// data-layout dimension naming index 1: the second data field's header and no other
/// cell.</description></item>
/// <item><description>B2:C4, the whole data area, is Liberation Mono 8 pt <strong>bold</strong> on
/// <c>#00b050</c> — and the record's green, not the <c>#ffff00</c> the cells themselves state,
/// which the clearing removed. The two are different colours on purpose: a fixture whose two
/// candidate sources agree cannot tell them apart, and the first cut of this one had them the
/// same.</description></item>
/// <item><description>A4 is bold and otherwise the <c>Normal</c> style — the generated
/// grand-total <c>Title</c>, untouched by any record.</description></item>
/// <item><description>Nothing anywhere is 18 pt Liberation Serif, which one of the five records
/// states.</description></item>
/// </list>
/// <para>
/// The last three of those are the three arms where the C++ tree read here — which declares
/// 27.2.0.0.alpha0+ and is not the reference binary's source — predicts something else. See
/// <c>XlsxPivotFormats</c>, whose remarks record how each was settled against 26.2.4.2 on a real
/// corpus workbook as well.
/// </para>
/// </remarks>
public sealed class SheetPivotFormatRecordsTests
{
    private const string Fixture = "pivot-format-records.xlsx";
    private const int BoldWeight = 700;
    private const int Formatted = 1;

    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        return (SpreadsheetPages)document.Layout();
    }

    private static SheetCellFormat Format(SpreadsheetPages pages, int row, int column)
        => pages.Sheets[Formatted].Formats.At(row, column);

    private static Colour? Background(SpreadsheetPages pages, int row, int column)
        => pages.Sheets[Formatted].Formatting.At(row, column).Background;

    /// <summary>
    /// A record with no references matches every line, so it paints the whole data area.
    /// </summary>
    /// <remarks>
    /// The broad-match arm of <c>findMatchingLines</c>, and the one that carries
    /// <c>033_Event_planning_tracker</c>: a bare <c>&lt;pivotArea outline="0"/&gt;</c> reaches all
    /// seventy-two of its cells.
    /// </remarks>
    [Fact]
    public void ARecordWithNoReferencesPaintsTheWholeDataArea()
    {
        SpreadsheetPages pages = Pages();
        List<string> wrong = [];

        for (int row = 1; row < 4; row++)
        {
            for (int column = 1; column < 3; column++)
            {
                SheetCellFormat format = Format(pages, row, column);
                string got = $"{format.FontFamily} {format.FontSize.Points:0.#} {format.FontWeight}";
                if (got != "Liberation Mono 8 700")
                    wrong.Add($"{(char)('A' + column)}{row + 1}: {got}");
            }
        }

        wrong.ShouldBeEmpty("26.2.4.2 gives all six Liberation Mono 8 pt bold");
    }

    /// <summary>
    /// A <c>dxf</c> whose fill states a background and no pattern type is a <em>solid</em> fill.
    /// </summary>
    /// <remarks>
    /// <c>Fill::finalizeImport</c>'s <c>mbDxf</c> arm moves the background into the pattern colour
    /// and forces the pattern solid (<c>sc/source/filter/oox/stylesbuffer.cxx</c>:1988-1994). The
    /// same rule with the colour left automatic is what paints
    /// <c>033_Event_planning_tracker</c>'s black band, which is 17.4 % of one page and which r108
    /// recorded as a fill of unknown origin.
    /// </remarks>
    [Fact]
    public void ADxfFillWithNoPatternTypeIsSolid()
    {
        SpreadsheetPages pages = Pages();

        for (int row = 1; row < 4; row++)
        {
            for (int column = 1; column < 3; column++)
            {
                Background(pages, row, column)
                    .ShouldBe(Colour.FromRgb(0x00B050), $"{(char)('A' + column)}{row + 1}");
            }
        }

        // And the clearing took the cells' own yellow off everything outside the data area: the
        // whole rectangle states `cellXfs[1]`, whose fill is `#ffff00`, and 26.2.4.2 draws none
        // of it.
        Background(pages, 0, 0).ShouldBeNull("the corner cell states #ffff00 and is cleared");
        Background(pages, 0, 1).ShouldBeNull("and so is the first data field's header");
        Background(pages, 3, 0).ShouldBeNull("and the grand total's own label");
    }

    /// <summary>
    /// A label record's reference on the data-layout dimension picks one column header.
    /// </summary>
    [Fact]
    public void ALabelRecordReachesOneColumnHeaderAndNoOtherCell()
    {
        SpreadsheetPages pages = Pages();

        Format(pages, 0, 2).Colour
            .ShouldBe(Colour.FromRgb(0xFF0000), "C1 is the second data field's header");
        Format(pages, 0, 1).Colour.ShouldBe(Colour.Black, "B1 is the first data field's");
        Format(pages, 0, 0).Colour.ShouldBe(Colour.Black, "and A1 is the corner");
    }

    /// <summary>
    /// A record whose area is neither data-only nor label-only reaches nothing at all.
    /// </summary>
    /// <remarks>
    /// The kind is Data when <c>dataOnly</c> — default <strong>true</strong> — else Label when
    /// <c>labelOnly</c>, else None, and <c>applyMatchedLines</c> has an arm for Label and an arm
    /// for Data and no third one. Six of <c>033</c>'s eighty-four records are inert that way, and
    /// two of the six state a font that would otherwise reach its whole table.
    /// </remarks>
    [Fact]
    public void ARecordThatIsNeitherDataOnlyNorLabelOnlyReachesNothing()
    {
        SpreadsheetPages pages = Pages();
        List<string> wrong = [];

        for (int row = 0; row < 4; row++)
        {
            for (int column = 0; column < 3; column++)
            {
                if (Format(pages, row, column).FontSize == Length.FromPoints(18))
                    wrong.Add($"{(char)('A' + column)}{row + 1}");
            }
        }

        wrong.ShouldBeEmpty(
            "the fixture's inert record states 18 pt Liberation Serif and 26.2.4.2 draws it "
            + "nowhere");
    }

    /// <summary>
    /// The cells no record reaches keep the cleared base, and the generated bold still lands.
    /// </summary>
    /// <remarks>
    /// The row labels take no record here, so they are the control that says the records are
    /// placed rather than applied everywhere — and A4's bold is the generated grand-total
    /// <c>Title</c> surviving underneath them.
    /// </remarks>
    [Fact]
    public void TheCellsNoRecordReachesKeepTheClearedBase()
    {
        SpreadsheetPages pages = Pages();

        for (int row = 0; row < 4; row++)
        {
            SheetCellFormat label = Format(pages, row, 0);
            label.FontFamily.ShouldBe("Liberation Sans", $"A{row + 1}");
            label.FontSize.ShouldBe(Length.FromPoints(11), $"A{row + 1}");
            label.Colour.ShouldBe(Colour.Black, $"A{row + 1}");
        }

        Format(pages, 3, 0).FontWeight.ShouldBe(BoldWeight, "A4 is the grand total");
        Format(pages, 1, 0).FontWeight.ShouldBeLessThan(BoldWeight, "A2 is an ordinary member");
    }
}

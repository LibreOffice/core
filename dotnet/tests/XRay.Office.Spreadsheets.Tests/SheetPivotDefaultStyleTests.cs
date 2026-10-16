using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What an emptied pivot cell falls back to, on the one workbook that can tell the two candidates
/// apart.
/// </summary>
/// <remarks>
/// <para>
/// <c>clearContents(… HARDATTR | STYLES …)</c>
/// (<c>sc/source/filter/oox/pivottablebuffer.cxx</c>:1331-1336) and
/// <c>DeleteAreaTab(…, InsertDeleteFlags::ALL)</c> (<c>dpoutput.cxx</c>:1226) take a pivot cell
/// back to Calc's <strong>Default</strong> cell style. That style is the <c>cellStyleXfs</c>
/// entry the <c>Normal</c> <c>cellStyle</c> names — <em>not</em> <c>cellXfs[0]</c>, which is only
/// what a cell stating <c>s="0"</c> resolves through. Every workbook in the corpus gives the two
/// the same content on a font, so no real document can separate them.
/// </para>
/// <para>
/// <c>regression/pivot-default-style.xlsx</c> separates them, and every number below was read out
/// of the <c>.fods</c> that <strong>LibreOffice 26.2.4.2</strong> writes for this same file
/// (<c>probes/pivot-fmt-r110/make-default-fixture.py</c>, <c>fods-face.py</c>). It states three
/// fonts that differ in face, size and colour at once:
/// </para>
/// <list type="bullet">
/// <item><description>the <c>Normal</c> <c>cellStyleXf</c> — Liberation Sans 11, black;</description></item>
/// <item><description><c>cellXfs[0]</c> — Liberation Serif 18, red;</description></item>
/// <item><description><c>cellXfs[1]</c>, on every cell of the pivot's own rectangle — Liberation
/// Mono 8, bold, green, on a yellow fill.</description></item>
/// </list>
/// <para>
/// 26.2.4.2 resolves all twelve cells of the emptied rectangle to <strong>Liberation Sans 11
/// black</strong> — the <c>Normal</c> style — and the control sheet's <c>s="0"</c> block to
/// Liberation Serif 18 red, so the reading is not a reading of the instrument.
/// </para>
/// <para>
/// Nothing puts any of it back here, because this fixture states no <c>&lt;formats&gt;</c>: on a
/// pivot that does state them, <c>XlsxPivotFormats</c> puts a face, a size, a colour and a fill
/// back over exactly this base.
/// </para>
/// </remarks>
public sealed class SheetPivotDefaultStyleTests
{
    private const string Fixture = "pivot-default-style.xlsx";
    private const int BoldWeight = 700;
    private const int Cleared = 1;
    private const int Plain = 2;

    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        return (SpreadsheetPages)document.Layout();
    }

    private static SheetCellFormat Format(SpreadsheetPages pages, int sheet, int row, int column)
        => pages.Sheets[sheet].Formats.At(row, column);

    /// <summary>
    /// The cleared cells take the <c>Normal</c> style's face and size, not <c>cellXfs[0]</c>'s
    /// and not their own.
    /// </summary>
    /// <remarks>
    /// 26.2.4.2 resolves all twelve to Liberation Sans 11 pt. This fixture states no
    /// <c>&lt;formats&gt;</c>, so nothing puts anything back over the clearing and the cleared
    /// base is the whole answer. A pivot that <em>does</em> state them keeps whatever they put
    /// back, which is <c>XlsxPivotFormats</c>' business rather than this fixture's.
    /// </remarks>
    [Fact]
    public void AnEmptiedPivotCellTakesTheNormalCellStylesFaceAndSize()
    {
        SpreadsheetPages pages = Pages();
        List<string> wrong = [];

        for (int row = 0; row < 4; row++)
        {
            for (int column = 0; column < 3; column++)
            {
                SheetCellFormat format = Format(pages, Cleared, row, column);
                if (format.FontFamily != "Liberation Sans"
                    || format.FontSize != Length.FromPoints(11))
                {
                    wrong.Add($"{(char)('A' + column)}{row + 1}: {format.FontFamily} {format.FontSize}");
                }
            }
        }

        wrong.ShouldBeEmpty(
            "26.2.4.2 gives all twelve Liberation Sans 11 — the Normal cellStyleXf's font, where "
            + "cellXfs[0] states Liberation Serif 18 and the cells themselves state Liberation "
            + "Mono 8");
    }

    /// <summary>And its colour, by the same reading.</summary>
    [Fact]
    public void AnEmptiedPivotCellTakesTheNormalCellStylesColour()
    {
        SpreadsheetPages pages = Pages();
        List<string> wrong = [];

        for (int row = 0; row < 4; row++)
        {
            for (int column = 0; column < 3; column++)
            {
                SheetCellFormat format = Format(pages, Cleared, row, column);
                if (format.Colour != Colour.Black)
                    wrong.Add($"{(char)('A' + column)}{row + 1}: {format.Colour}");
            }
        }

        wrong.ShouldBeEmpty("26.2.4.2 gives all twelve #000000, not the cells' own #00a000");
    }

    /// <summary>
    /// The generated styles still land on top of the cleared base.
    /// </summary>
    /// <remarks>
    /// Row 4 is the grand total, which takes <c>Pivot Table Result</c>/<c>Title</c> and is bold in
    /// the reference's <c>.fods</c>; rows 1 to 3 are not.
    /// </remarks>
    [Fact]
    public void TheGrandTotalRowIsStillBoldAndTheRowsAboveAreNot()
    {
        SpreadsheetPages pages = Pages();

        for (int column = 0; column < 3; column++)
        {
            Format(pages, Cleared, 3, column).FontWeight
                .ShouldBe(BoldWeight, $"the grand total at row 4 column {column + 1}");
        }

        for (int row = 1; row < 3; row++)
        {
            for (int column = 0; column < 3; column++)
            {
                Format(pages, Cleared, row, column).FontWeight
                    .ShouldBeLessThan(BoldWeight, $"row {row + 1} column {column + 1}");
            }
        }
    }

    /// <summary>
    /// The control: a cell stating <c>s="0"</c> keeps <c>cellXfs[0]</c>, which is a different
    /// format from the one the pivot cleared to.
    /// </summary>
    /// <remarks>
    /// Without this the test above would pass just as well on a tree that had confused the two
    /// tables the other way round, or on a fixture whose two entries were secretly the same.
    /// </remarks>
    [Fact]
    public void ACellStatingTheDefaultCellFormatIsNotTheNormalStyle()
    {
        SpreadsheetPages pages = Pages();

        //  Plain A7:C10 states s="0" outright.
        SheetCellFormat stated = Format(pages, Plain, 6, 0);
        stated.FontFamily.ShouldBe("Liberation Serif", "cellXfs[0]'s face");
        stated.FontSize.ShouldBe(Length.FromPoints(18));
        stated.Colour.ShouldBe(Colour.FromRgb(0xFF0000));

        //  Plain A1:C4 states s="1", the format the pivot's own cells carry, and no pivot is on
        //  this sheet to clear it.
        SheetCellFormat hard = Format(pages, Plain, 0, 0);
        hard.FontFamily.ShouldBe("Liberation Mono");
        hard.FontSize.ShouldBe(Length.FromPoints(8));
        hard.FontWeight.ShouldBe(BoldWeight);
    }
}

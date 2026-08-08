using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A turned cell's row is its text's <em>width</em>, and a quarter turn is not a rotation.
/// </summary>
/// <remarks>
/// <para>
/// Calc measures a turned cell along <c>ScColumn::GetNeededSize</c>'s direct-output branch
/// (<c>sc/source/core/data/column2.cxx:311-370</c>), which wraps nothing: the whole string is
/// measured on one line and its width goes through the angle, so a turned cell's row grows without
/// bound with its text. Three things decide the answer and each is separable in the fixture:
/// </para>
/// <list type="number">
/// <item>
/// <description>
/// <strong>Exactly ninety degrees is an orientation, not an angle.</strong>
/// <c>ScPatternAttr::GetCellOrientation</c> (<c>patattr.cxx:529-547</c>) reads 9000 and 27000 as
/// <c>BottomUp</c> and <c>TopBottom</c>, and <c>nRotate</c> is only read when the orientation came
/// back <c>Standard</c> (<c>column2.cxx:231-238</c>). So the quarter turns take a branch where the
/// height is simply the text's width, and no other angle does.
/// </description>
/// </item>
/// <item>
/// <description>
/// <strong>A wrapping cell is capped at six font heights, and a quarter-turned one is not.</strong>
/// <c>SC_ROT_BREAK_FACTOR</c> (<c>column2.cxx:74, :363</c>) sits inside the rotation branch, so the
/// 45°, 30° and 60° sheets stop at 1373 twips for eleven point however long the string is while the
/// 90° sheet reaches 7358 for the same string.
/// </description>
/// </item>
/// <item>
/// <description>
/// <strong>A quarter-turned cell has no floor either.</strong> The sheet's minimum reaches the row
/// only through <c>lcl_GetAttribHeight</c> (<c>column2.cxx:889</c>), which
/// <c>bStdAllowed</c> (<c>:925</c>) gates on the same orientation test — hence 149 twips for a
/// single letter at 90° against 257 for the same letter at 45°.
/// </description>
/// </item>
/// </list>
/// <para>
/// The expected heights are LibreOffice 24.2.7.2's own, read out of its flat-ODF round trip of this
/// fixture; <c>dotnet/probes/sheets-r26/</c> holds the generator and the round trip. All 216 are
/// reproduced exactly, and the width model is what made that possible: a glyph advances by whole
/// device pixels, so a string's width is the sum of its rounded advances rather than the rounded
/// sum. Fourteen of the eighteen distinct widths agree under either reading and four do not — the
/// twelve-point ones, where the difference reaches 1.4%.
/// </para>
/// <para>
/// Sheet names read size, angle and wrap: <c>p11a45w</c> is eleven point at 45° with wrapping on.
/// Rows are the same six strings throughout, of 1, 5, 10, 20, 40 and 72 characters.
/// </para>
/// </remarks>
public sealed class SheetRotatedRowHeightTests
{
    private const string Fixture = "sheet-row-height-rotated.fods";

    [Theory]
    [InlineData("p10a90n", 149, 522, 865, 1776, 3552, 6373)]
    [InlineData("p10a90w", 149, 522, 865, 1776, 3552, 6373)]
    [InlineData("p10a45n", 257, 522, 776, 1417, 2671, 4671)]
    [InlineData("p10a45w", 257, 522, 776, 1194, 1194, 1194)]
    [InlineData("p10a30n", 257, 462, 626, 1089, 1970, 3388)]
    [InlineData("p10a30w", 257, 462, 626, 1089, 1194, 1194)]
    [InlineData("p10a60n", 257, 567, 850, 1641, 3179, 5626)]
    [InlineData("p10a60w", 257, 567, 850, 1194, 1194, 1194)]
    [InlineData("p10a270n", 149, 522, 865, 1776, 3552, 6373)]
    [InlineData("p10a270w", 149, 522, 865, 1776, 3552, 6373)]
    [InlineData("p10a315n", 257, 522, 776, 1417, 2671, 4671)]
    [InlineData("p10a315w", 257, 522, 776, 1194, 1194, 1194)]
    [InlineData("p11a90n", 164, 597, 999, 2044, 4104, 7358)]
    [InlineData("p11a90w", 164, 597, 999, 2044, 4104, 7358)]
    [InlineData("p11a45n", 313, 611, 895, 1641, 3089, 5388)]
    [InlineData("p11a45w", 313, 611, 895, 1373, 1373, 1373)]
    [InlineData("p11a30n", 328, 537, 746, 1268, 2298, 3925)]
    [InlineData("p11a30w", 328, 537, 746, 1268, 1373, 1373)]
    [InlineData("p11a60n", 276, 641, 999, 1895, 3686, 6507)]
    [InlineData("p11a60w", 276, 641, 999, 1373, 1373, 1373)]
    [InlineData("p11a270n", 164, 597, 999, 2044, 4104, 7358)]
    [InlineData("p11a270w", 164, 597, 999, 2044, 4104, 7358)]
    [InlineData("p11a315n", 313, 611, 895, 1641, 3089, 5388)]
    [InlineData("p11a315w", 313, 611, 895, 1373, 1373, 1373)]
    [InlineData("p12a90n", 164, 626, 1059, 2149, 4328, 7731)]
    [InlineData("p12a90w", 164, 626, 1059, 2149, 4328, 7731)]
    [InlineData("p12a45n", 313, 641, 955, 1716, 3268, 5671)]
    [InlineData("p12a45w", 313, 641, 955, 1462, 1462, 1462)]
    [InlineData("p12a30n", 328, 567, 776, 1328, 2417, 4119)]
    [InlineData("p12a30w", 328, 567, 776, 1328, 1462, 1462)]
    [InlineData("p12a60n", 300, 686, 1059, 1999, 3880, 6835)]
    [InlineData("p12a60w", 300, 686, 1059, 1462, 1462, 1462)]
    [InlineData("p12a270n", 164, 626, 1059, 2149, 4328, 7731)]
    [InlineData("p12a270w", 164, 626, 1059, 2149, 4328, 7731)]
    [InlineData("p12a315n", 313, 641, 955, 1716, 3268, 5671)]
    [InlineData("p12a315w", 313, 641, 955, 1462, 1462, 1462)]
    public void RowsAreTheReferenceHeights(
        string sheet, int one, int five, int ten, int twenty, int forty, int seventyTwo)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(Fixture));

        SheetLayout layout = ((SpreadsheetPages)document.Layout())
            .Sheets.Single(s => s.Name == sheet);

        int[] expected = [one, five, ten, twenty, forty, seventyTwo];
        for (int row = 0; row < expected.Length; row++)
            layout.Grid.Rows.SizeAt(row).Twips.ShouldBe(expected[row], $"row {row}");
    }
}

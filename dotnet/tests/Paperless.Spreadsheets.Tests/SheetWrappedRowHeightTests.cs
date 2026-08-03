using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A wrapping row is as tall as its lines, measured the coarse way Calc measures them.
/// </summary>
/// <remarks>
/// <para>
/// The EditEngine branch of <c>ScColumn::GetNeededSize</c>
/// (<c>sc/source/core/data/column2.cxx:409-600</c>) formats against a reference device, and the
/// device is the whole difference between its answer and an accurate one. Three quantisations,
/// all of them whole pixels on a headless 96 dpi <c>VirtualDevice</c>: the em size, the ascent and
/// the descent. The margins truncate to a pixel each, and the pixel total is turned back into
/// twips by dividing by the 0.067 that <c>ScSizeDeviceProvider</c> computes rather than by the
/// exact 1/15 (<c>sc/source/ui/docshell/sizedev.cxx:48-50</c>).
/// </para>
/// <para>
/// The fixture's six rows are LibreOffice 24.2.7.2's own answers, read from its flat-ODF export:
/// 0.2083, 0.5492, 0.9016, 0.3063, 0.8291 and 1.3681 inches, which are 300, 791, 1298, 441, 1194
/// and 1970 twips. They are chosen so that the two halves of the rule both decide a row. Rows 1
/// and 4 hold one word that fits, and there the arithmetic height wins — 441 for eighteen point
/// against the 417 its single measured line asks for. Rows 2, 3, 5 and 6 hold three and five
/// words, one to a line, and there the measurement wins.
/// </para>
/// <para>
/// The line heights this implies are 17 pixels for twelve point and 26 for eighteen, where exact
/// scaling of Liberation Serif's <c>hhea</c> gives 17.71 and 26.57. Rounding rather than
/// quantising would put row 2 at 823 twips against LibreOffice's 791, so the test would fail on
/// the difference this exists to capture.
/// </para>
/// </remarks>
public sealed class SheetWrappedRowHeightTests
{
    private static SheetAxis Rows(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Grid.Rows;
    }

    [Theory]
    [InlineData(0, 300)]    // twelve point, one line: the arithmetic height, at the sheet minimum
    [InlineData(1, 791)]    // three lines: 3 x 17 px + 2, over 0.067
    [InlineData(2, 1298)]   // five lines:  5 x 17 px + 2
    [InlineData(3, 441)]    // eighteen point, one line: the arithmetic height wins over 417
    [InlineData(4, 1194)]   // three lines: 3 x 26 px + 2
    [InlineData(5, 1970)]   // five lines:  5 x 26 px + 2
    public void AWrappedRowIsAsTallAsItsLines(int row, int twips)
        => Rows("sheet-row-height-wrap.xlsx").SizeAt(row).Twips.ShouldBe(twips);
}

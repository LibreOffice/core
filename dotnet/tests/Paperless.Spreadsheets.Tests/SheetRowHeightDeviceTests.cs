using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A row is measured against a device Calc never draws with, and the device is 6% narrower
/// horizontally than the page is.
/// </summary>
/// <remarks>
/// <para>
/// The height a wrapping cell asks for is a line count before it is a pitch, so the width the text
/// is broken at decides the row as much as the font's metrics do. Two things make that width
/// Calc's rather than the document's, and both are on the horizontal only:
/// </para>
/// <list type="number">
/// <item>
/// <description>
/// <c>ScColumn::GetNeededSize</c> formats through a <c>ScFieldEditEngine</c> whose reference device
/// is a headless 96 dpi <c>VirtualDevice</c> (<c>column2.cxx:410-424</c>), and a device can only
/// instantiate a font at a whole number of pixels. Eleven point is 15 pixels rather than 14.667, so
/// its advances come out 2.3% <em>wide</em>; ten point is 13 rather than 13.333, so they come out
/// 2.5% <em>narrow</em>; twelve point is 16 exactly and comes out unchanged.
/// </description>
/// </item>
/// <item>
/// <description>
/// <c>ScSizeDeviceProvider</c> divides <c>nPPTX</c> — and not <c>nPPTY</c> — by the document's
/// print-to-screen factor (<c>sizedev.cxx:52</c>), and <c>nPPTX</c> is what the column width is
/// multiplied by to get the EditEngine's paper (<c>column2.cxx:463</c>). So the paper is narrower
/// than the column by that factor while the pitch is unaffected by it.
/// </description>
/// </item>
/// </list>
/// <para>
/// The fixture is built to separate the two. Each pair of sheets holds the same four strings at two
/// column widths fifty twips apart, chosen to straddle a break point, so one sheet of the pair
/// wraps the first string and the other does not. At twelve point the em rounding is neutral and
/// only the print-to-screen factor can decide the pair; at ten point the em rounding acts in the
/// opposite direction to the factor, so an implementation carrying only one of them puts that pair
/// on the wrong side.
/// </para>
/// <para>
/// The expected heights are LibreOffice 24.2.7.2's own, read from its flat-ODF round trip of this
/// same file: 0.1783, 0.3311, 0.2071, 0.3936, 0.4146, 0.4634, 0.5799 and 0.6110 inches. Every row
/// here reproduces one of them to the twip. Note what is <em>not</em> different: a PDF of the same
/// strings shows LibreOffice drawing them at Paperless's advances to within a quarter point over
/// four hundred, so this is the measuring device and not the face.
/// </para>
/// </remarks>
public sealed class SheetRowHeightDeviceTests
{
    private const string Fixture = "sheet-row-height-device.fods";

    private static SheetAxis Rows(int sheet)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(Fixture));

        return ((SpreadsheetPages)document.Layout()).Sheets[sheet].Grid.Rows;
    }

    // Sheet order: 10 pt at 3800 and 3850 twips, 11 pt at 4400 and 4450, 12 pt at 4700 and 4750.
    // Row order: an unbreakable mixed word, forty-two n, forty-two M, and a wrapping sentence.
    [Theory]
    // Ten point. The em rounds *down* to 13 pixels, which alone would fit the first string on one
    // line at both widths; the print-to-screen factor is what puts it on two at 3800.
    [InlineData(0, 0, 477)]
    [InlineData(0, 1, 477)]
    [InlineData(0, 2, 477)]
    [InlineData(0, 3, 701)]
    [InlineData(1, 0, 257)]
    [InlineData(1, 1, 477)]
    [InlineData(1, 2, 477)]
    [InlineData(1, 3, 701)]
    // Eleven point. The em rounds *up* to 15 pixels and both effects push the same way.
    [InlineData(2, 0, 567)]
    [InlineData(2, 1, 567)]
    [InlineData(2, 2, 567)]
    [InlineData(2, 3, 835)]
    [InlineData(3, 0, 298)]
    [InlineData(3, 1, 567)]
    [InlineData(3, 2, 567)]
    [InlineData(3, 3, 835)]
    // Twelve point. The em is 16 pixels exactly, so the print-to-screen factor decides alone.
    [InlineData(4, 0, 597)]
    [InlineData(4, 1, 597)]
    [InlineData(4, 2, 597)]
    [InlineData(4, 3, 880)]
    [InlineData(5, 0, 313)]
    [InlineData(5, 1, 597)]
    [InlineData(5, 2, 597)]
    [InlineData(5, 3, 880)]
    public void ARowIsBrokenAtTheWidthTheMeasuringDeviceHas(int sheet, int row, int twips)
        => Rows(sheet).SizeAt(row).Twips.ShouldBe(twips);

    /// <summary>
    /// Fifty twips of column decide a line, and both halves of the device are needed to put the
    /// pair on the right side of it.
    /// </summary>
    /// <remarks>
    /// Stated separately from the table above because it is the assertion that bites. An
    /// implementation measuring at the size the file states, or handing the EditEngine the column
    /// width undivided, still reproduces most of the table — the strings are long enough that a
    /// few per cent rarely changes a line count — and gets every one of these three pairs wrong,
    /// because each pair was chosen to sit across a break.
    /// </remarks>
    [Theory]
    [InlineData(0, 1)]   // ten point:    two lines at 3800 twips, one at 3850
    [InlineData(2, 3)]   // eleven point: two lines at 4400, one at 4450
    [InlineData(4, 5)]   // twelve point: two lines at 4700, one at 4750
    public void TheNarrowerSheetOfEachPairWrapsAndTheWiderOneDoesNot(int narrow, int wide)
    {
        long wrapped = Rows(narrow).SizeAt(0).Twips;
        long single = Rows(wide).SizeAt(0).Twips;

        single.ShouldBeLessThan(wrapped);
        (wrapped - single).ShouldBeGreaterThan(200);
    }
}

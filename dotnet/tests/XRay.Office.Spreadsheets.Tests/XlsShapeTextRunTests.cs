using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The formatting runs a BIFF text box's <c>TXO</c> carries, which decide its faces and sizes.
/// </summary>
/// <remarks>
/// <para>
/// The string was read and the runs were not: the reader took the characters and stopped, and
/// every shape's text was built as one run per line at a hardcoded ten point in the default
/// face. On <c>TICAPCapability_Final.xls</c> page 3 that drew the whole box at 5.70 pt regular
/// where 26.2.4.2 draws it at 6.30 pt with five bold spans.
/// </para>
/// <para>
/// The runs are in the <em>second</em> <c>CONTINUE</c> after the record — the first holds the
/// characters — eight bytes each, a character index and a <c>FONT</c> index with four reserved
/// (<c>XclImpDrawing::ReadTxo</c>, <c>sc/source/filter/excel/xiescher.cxx</c>:4242-4271, and
/// <c>XclImpString::ReadObjFormats</c>, <c>xistring.cxx</c>). The last entry names the character
/// index one past the string and is a terminator.
/// </para>
/// <para>
/// The fixture is 26.2.4.2's own <c>.xls</c> export of a flat ODS whose one text box holds a
/// 16 pt bold span followed by an 8 pt regular one; its <c>TXO</c> declares 18 characters and 24
/// bytes of runs, which is <c>(0, font 6)</c>, <c>(8, font 7)</c> and the terminator
/// <c>(18, font 0)</c>. Both numbers below are read out of 26.2.4.2's own PDF of that file,
/// which draws <c>BIG BOLD</c> at 15.99 pt bold and <c> and small</c> at 7.99 pt regular.
/// </para>
/// </remarks>
public sealed class XlsShapeTextRunTests
{
    private static SheetShapeText ShapeText(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(name));

        SheetLayout sheet = ((SpreadsheetPages)document.Layout()).Sheets[0];
        sheet.Drawings.Items.Count.ShouldBe(1, "drawings on the sheet");
        return sheet.Drawings.Items[0].Text.ShouldNotBeNull("the TXO's string was read");
    }

    [Fact]
    public void AParagraphIsCutIntoOneRunPerFormattingRun()
    {
        SheetShapeText text = ShapeText("sheet-shape-runs-xls.xls");

        text.Paragraphs.Count.ShouldBe(1, "paragraphs in the box");
        text.Paragraphs[0].Runs.Count.ShouldBe(2, "runs in the paragraph");

        text.Paragraphs[0].Runs[0].Text.ShouldBe("BIG BOLD");
        text.Paragraphs[0].Runs[1].Text.ShouldBe(" and small");
    }

    /// <summary>
    /// Each run takes the size and weight of the <c>FONT</c> its index names.
    /// </summary>
    /// <remarks>
    /// Before the runs were read both spans came back at the hardcoded ten point, so a reader
    /// that ignored the run array would fail both assertions with the same wrong number — which
    /// is why the second one matters as much as the first.
    /// </remarks>
    [Fact]
    public void EachRunTakesItsOwnFontsSizeAndWeight()
    {
        SheetShapeText text = ShapeText("sheet-shape-runs-xls.xls");

        SheetShapeRun big = text.Paragraphs[0].Runs[0];
        SheetShapeRun small = text.Paragraphs[0].Runs[1];

        big.Size.Points.ShouldBe(16.0, 0.01);
        big.Bold.ShouldBeTrue("the first run's FONT states weight 700");

        small.Size.Points.ShouldBe(8.0, 0.01);
        small.Bold.ShouldBeFalse("the second run's FONT states weight 400");
    }
}

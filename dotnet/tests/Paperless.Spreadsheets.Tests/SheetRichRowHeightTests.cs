using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A cell in several formats is measured too, and its height is a sum rather than a product.
/// </summary>
/// <remarks>
/// <para>
/// EditEngine gives each line the largest ascent and the largest descent of the portions standing
/// on it (<c>ImpEditEngine::CreateLines</c>,
/// <c>editeng/source/editeng/impedit3.cxx:1500-1519</c>, over the maxima
/// <c>RecalcFormatterFontMetrics</c> accumulates at <c>:3159-3163</c>), so a cell whose middle word
/// is set larger is taller by the difference on <em>that line alone</em> and not by it on every
/// line. Row 3 is the whole of the assertion: 17 + 26 + 17 device pixels rather than 3 × 26.
/// </para>
/// <para>
/// The fixture's four rows are LibreOffice 24.2.7.2's own answers, read from its flat-ODF export of
/// this file: 0.5492, 0.5492, 0.6425 and 0.6425 inches, which are 791, 791, 925 and 925 twips. Each
/// row states <c>ht="20"</c> — 400 twips — without <c>customHeight</c>, so a row this cannot measure
/// keeps 400 and every assertion below fails.
/// </para>
/// <para>
/// Rows 1 and 2 are the negative half in both directions: the plain cell must still measure the way
/// <see cref="SheetWrappedRowHeightTests"/> fixed it, and the rich cell differing only in colour
/// must land on the same number, because a colour changes no advance width and no metric.
/// </para>
/// </remarks>
public sealed class SheetRichRowHeightTests
{
    private static SheetAxis Rows(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Grid.Rows;
    }

    [Theory]
    [InlineData(0, 791)]    // plain, three twelve-point lines: 3 x 17 px + 2, over 0.067
    [InlineData(1, 791)]    // the same in three colours: a colour is not a metric
    [InlineData(2, 925)]    // the middle word at eighteen point: 17 + 26 + 17 px + 2
    [InlineData(3, 925)]    // the first word at eighteen point: 26 + 17 + 17 px + 2
    public void ARichRowIsAsTallAsItsTallestPortionPerLine(int row, int twips)
        => Rows("sheet-row-height-rich.xlsx").SizeAt(row).Twips.ShouldBe(twips);
}

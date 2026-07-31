using Paperless.Core.Documents;
using Paperless.Spreadsheets;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Paginates a real workbook and compares its pages with the ones LibreOffice prints.
/// </summary>
/// <remarks>
/// <para>
/// A spreadsheet has no intrinsic pagination, so the page count is not a detail of the layout —
/// it <em>is</em> the layout. That makes it the single cheapest and most discriminating
/// assertion available: a scale read wrong, a print area missed, a manual break honoured when it
/// should have been suppressed or a repeated header band not subtracted from the page all show
/// up as a different number of pages, and none of them can hide behind a tolerance.
/// </para>
/// <para>
/// The second assertion is which cells land on which page, checked through the words LibreOffice
/// drew. The corpus documents are filled with cells naming their own coordinates —
/// <c>R05C03</c> is row five, column three — so a page's first word states exactly where that
/// page starts, and a comparison needs no interpretation. That is what catches the errors a page
/// count cannot: a page order read backwards produces the right number of pages holding the
/// wrong cells.
/// </para>
/// </remarks>
public sealed class SheetPaginationComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-sheet-pages").FullName;

    public void Dispose()
    {
        _libreOffice.Dispose();
        try
        {
            Directory.Delete(_workDirectory, recursive: true);
        }
        catch (IOException)
        {
            // A leftover temporary directory is not worth failing a test over.
        }
    }

    [Theory]
    [InlineData("sheet-print-ods.ods")]
    [InlineData("sheet-print-xlsx.xlsx")]
    [InlineData("sheet-print-xls.xls")]
    [InlineData("sheet-features.ods")]
    [InlineData("sheet-ooxml-features.xlsx")]
    [InlineData("xls-features.xls")]
    public void AWorkbookPrintsToAsManyPagesAsLibreOfficeDoes(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        string pdf = _libreOffice.ConvertToPdf(path, _workDirectory);
        int reference = PdfPageSizes.Read(pdf).Count;

        Assert.SkipWhen(reference == 0, "pdftotext is not installed");

        using IPaginatedDocument document = Open(path);
        int actual = document.Layout().Count;

        actual.ShouldBe(
            reference,
            $"{name}: page count differs — Paperless {actual}, LibreOffice {reference}");
    }

    [Theory]
    [InlineData("sheet-print-ods.ods")]
    [InlineData("sheet-print-xlsx.xlsx")]
    [InlineData("sheet-print-xls.xls")]
    public void EveryPageStartsAtTheCellLibreOfficeStartsItAt(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        string pdf = _libreOffice.ConvertToPdf(path, _workDirectory);
        List<PdfWord> words = PdfWords.Read(pdf);

        Assert.SkipWhen(words.Count == 0, "pdftotext is not installed");

        using IPaginatedDocument document = Open(path);
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        for (int at = 0; at < pages.Count; at++)
        {
            // The top-left word of the reference page, which for these documents is the first
            // cell of the page's block — or of its repeated header band, when it has one.
            int index = at;
            PdfWord first = words.Where(word => word.PageIndex == index)
                                 .OrderBy(word => word.Top)
                                 .ThenBy(word => word.Left)
                                 .First();

            string expected = FirstCellOf(pages.Pages[at]);

            first.Text.ShouldBe(
                expected,
                $"{name}: page {at + 1} starts at a different cell."
                + $"\n  LibreOffice: {first.Text}\n  Paperless:   {expected}");
        }
    }

    /// <summary>
    /// The text of the first cell a page shows, repeated bands included.
    /// </summary>
    /// <remarks>
    /// The repeated bands come first on the page, so a page below a repeated row band starts with
    /// that band's first cell rather than with its own. Which is the point of asserting on the
    /// drawn word: it is the only thing that notices a band placed after the content instead of
    /// before it.
    /// </remarks>
    private static string FirstCellOf(SheetPage page)
    {
        SheetPagePlacement placement = page.Placement;

        int column = placement.RepeatColumns is { } columns
            ? columns.FirstColumn
            : placement.Cells.FirstColumn;

        int row = placement.RepeatRows is { } rows
            ? rows.FirstRow
            : placement.Cells.FirstRow;

        // The cell's own text rather than a name rebuilt from the coordinates, so that the
        // corpus documents are free to write whatever they like in a cell — which they do: the
        // five sheets use five different prefixes so a page landing on the wrong sheet is as
        // obvious as one landing on the wrong row.
        return page.Sheet.CellAt(row, column)?.GetText() ?? string.Empty;
    }

    private static IPaginatedDocument Open(string path)
        => (IPaginatedDocument)new SpreadsheetReader().Read(DocumentSource.FromFile(path));
}

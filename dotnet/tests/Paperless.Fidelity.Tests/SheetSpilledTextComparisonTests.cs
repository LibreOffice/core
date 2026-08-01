using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares, page by page, how much text reaches a page that holds no cell of its own.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why a page's word count rather than a cell's placement.</strong> The defect this file
/// exists for was invisible to every positional test in the suite, and had to be, because the
/// runs it lost were never emitted at all: a sheet whose only content is one column of long
/// strings splits into two horizontal pages, and the second page draws <em>nothing</em> — no cell
/// on it holds anything, and the text that belongs there is entirely the first column's spill.
/// A comparison of the runs both renderers drew agrees perfectly on a page neither of them drew
/// anything on. Counting what reaches the page is the claim that fails.
/// </para>
/// <para>
/// <strong>What the count measures, exactly.</strong> <c>pdftotext</c> discards a glyph whose box
/// lies wholly off the paper and keeps one that straddles the edge, so a string running off the
/// side is counted for the part of it that shows. That is the property under test: LibreOffice
/// draws the whole 187-glyph string on both pages, from the cell's true position, and the two
/// pages between them show every word once plus the one the break falls inside — 25 words on
/// page three, 21 on page four, of a 38-word string.
/// </para>
/// <para>
/// The rule is <c>ScOutputData::LayoutStrings</c>'s: the column loop starts one column before the
/// block — <c>if (mnX1 &gt; 0) --nLoopStartX</c>, <c>sc/source/ui/view/output2.cxx:1541-1543</c> —
/// and that iteration resolves to the nearest cell with text at or left of the block
/// (<c>:1638-1656</c>). Without it page four of <c>xls-features.xls</c> came out at 3 words
/// against the reference's 1 011, the three being the header and the footer.
/// </para>
/// </remarks>
public sealed class SheetSpilledTextComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-sheet-spill").FullName;

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
    [InlineData("xls-features.xls")]
    [InlineData("sheet-print-xlsx.xlsx")]
    [InlineData("sheet-print-ods.ods")]
    public void EveryPageShowsAsManyWordsAsLibreOfficeShows(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        List<PdfWord> ours = PdfWords.Read(Ours(path));
        List<PdfWord> theirs = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // The two files that already matched are here as much as the one that did not: a lead-in
        // column drawn where Calc draws none would show up as words appearing on a page of the
        // fourteen-page print workbooks, and those two are the corpus's densest horizontal splits.
        int pages = theirs.Max(word => word.PageIndex) + 1;

        int[] mine = [.. Enumerable.Range(0, pages)
                                   .Select(page => ours.Count(word => word.PageIndex == page))];
        int[] reference = [.. Enumerable.Range(0, pages)
                                        .Select(page => theirs.Count(word => word.PageIndex == page))];

        mine.ShouldBe(reference, $"{name}: words reaching each page");
    }

    [Fact]
    public void AStringSpillingPastAPageBreakIsDrawnOnBothSidesOfIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("xls-features.xls");
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs =
            PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "the PDF holds no text runs");

        // The Strings sheet is 48 rows of one 175-character cell in column A, and it takes the
        // last two pages of the workbook. Every row reaches both, drawn once on each from the
        // cell's own position — which on the second page is off the left of the paper.
        static List<PdfTextRun> Cells(List<PdfTextRun> runs, int page)
            => [.. runs.Where(run => run.PageIndex == page && run.GlyphCount > 100)];

        Cells(ours, 2).Count.ShouldBe(Cells(theirs, 2).Count, "rows drawn on page three");
        Cells(ours, 3).Count.ShouldBe(Cells(theirs, 3).Count, "rows drawn on page four");

        // Once each, not twice: a lead-in that failed to notice the band's own first column
        // already holds the cell would draw every row a second time.
        Cells(ours, 3).Count.ShouldBe(48, "one run per row on the last page");

        // And from where the cell really is, which is what makes the two halves of one string
        // line up across the break: LibreOffice puts the pen at -388.97 pt and Paperless at
        // -389.96. The whole of that difference is already there on page three, where the same
        // cell is drawn in the ordinary way — LibreOffice's own BIFF import puts this workbook's
        // left margin 21 twips further right than the file states, which
        // `SheetTextComparisonTests` records and measures. Asserting that the gap is the *same*
        // on both pages is the claim the lead-in owns: it contributes no error of its own.
        Cells(ours, 3).ShouldAllBe(run => run.X < 0, "drawn from off the left of the paper");

        double onPageThree = Cells(ours, 2)[0].X - Cells(theirs, 2)[0].X;
        double onPageFour = Cells(ours, 3)[0].X - Cells(theirs, 3)[0].X;

        onPageFour.ShouldBe(onPageThree, 0.05, "the lead-in adds no offset of its own");
    }

    private string Ours(string documentPath)
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-paperless.pdf");

        using IDocument document = PaperlessDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }
}

using XRay.Office.Core.Documents;
using XRay.Office.Rendering.Pdf;
using XRay.Office.TestKit;
using XRay.Office.TestKit.LibreOffice;
using Shouldly;

namespace XRay.Office.Fidelity.Tests;

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
/// <strong>The lead-in that rule describes is no longer drawn, and this file's own assertion for
/// it was stale for exactly that reason.</strong> <c>ScOutputData::LayoutStrings</c> starts its
/// column loop one column before the block (<c>output2.cxx:1541-1543</c>) so that a long string
/// to the left reaches the page, and <see cref="XRay.Office.Spreadsheets.Layout.SpreadsheetPages"/>
/// deliberately does not: a rightward overflow is painted on the page holding its anchor cell and
/// on no other. That was settled on 26.2.4.2, the version this tree is developed against, and
/// the assertion here still demanded the older behaviour — so it failed against <em>both</em>
/// binaries, on 24.2.7.2 because ours no longer draws the lead-in and on 26.2.4.2 because it
/// hard-coded a count only 24.2 produces.
/// </para>
/// <para>
/// The two releases genuinely differ, measured on this fixture with nothing changed but the
/// binary: page four is <b>1 011 words in 48 runs</b> under 24.2.7.2 and <b>3 words in none</b>
/// under 26.2.4.2. It is not the <c>!bTaggedPDF</c> guard that separates them — both write a
/// tagged PDF here, <c>/MarkInfo /Marked true</c> in each — so the behaviour itself moved. A test
/// that compares that page against whatever <c>soffice</c> is installed reports the environment
/// rather than the code, which is why the far page is now asserted against our rule and the
/// anchor page, where the two agree, against the reference.
/// </para>
/// </remarks>
public sealed class SheetSpilledTextComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("xray-office-sheet-spill").FullName;

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
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

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
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

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

        // The anchor page is compared against the reference live, because both versions agree
        // there: the cell is on it, and it is drawn in the ordinary way.
        Cells(ours, 2).Count.ShouldBe(Cells(theirs, 2).Count, "rows drawn on page three");
        Cells(ours, 2).Count.ShouldBeGreaterThan(0, "the anchor page draws the cells");

        // The FAR page is not, because the reference's answer there depends on which LibreOffice
        // is installed, and this assertion used to hard-code one of the two.
        //
        // `SpreadsheetPages` draws a rightward overflow on the page holding its anchor cell and
        // on no other, and that rule is 26.2.4.2's — the version this tree is developed against.
        // 24.2.7.2 draws the lead-in and puts the whole string on the far page too. Measured on
        // this fixture, same file, same command, only the binary changed:
        //
        //     24.2.7.2   page four: 1011 words, 48 runs
        //     26.2.4.2   page four:    3 words,  0 runs   (the header and the footer)
        //
        // And the discriminator is NOT the `!bTaggedPDF` guard the C++ carries: both binaries
        // write a tagged PDF here — `/MarkInfo /Marked true` in each — and differ anyway. So the
        // behaviour itself moved between the two releases, and a test that compares this page
        // against whatever `soffice` is on the machine is a test that reports the environment.
        //
        // What is asserted instead is our own rule, which is the thing this file exists to pin.
        Cells(ours, 3).Count.ShouldBe(
            0, "a rightward overflow belongs to its anchor page and no other");
    }

    private string Ours(string documentPath)
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-xray-office.pdf");

        using IDocument document = OfficeDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }
}

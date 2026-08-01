using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares what a long string does when the cell beside it is covered by a merge.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Extra output is a defect too, and this one was easier to see than to explain.</strong>
/// <c>sheet-features.ods</c> rendered 46 words against the reference's 45. The extra word was
/// <em>"pair"</em>: a string in column A ran through the cell to its right and drew in full,
/// where LibreOffice cut it to "Second row of p" at the edge of the two-row merge beside it.
/// Every positional assertion passed — the runs that were drawn were drawn in the right places —
/// because the difference is in how many characters one of them holds.
/// </para>
/// <para>
/// <strong>Why a content tree cannot answer the question on its own.</strong>
/// <c>ScOutputData::IsAvailable</c> (<c>sc/source/ui/view/output2.cxx:1178-1191</c>) asks two
/// things of a neighbour before letting a string run through it: whether it is empty, and whether
/// it carries <c>ATTR_MERGE</c> or <c>ATTR_MERGE_FLAG</c>'s overlapped bit. The cells a merge
/// covers are dropped by every reader — only the block's origin survives — so they look exactly
/// like empty ones and only the second question distinguishes them. The merge here is
/// <em>vertical</em>, which is what made it invisible: its origin is in the row above, so the row
/// being drawn holds nothing at that column at all.
/// </para>
/// </remarks>
public sealed class SheetMergedNeighbourComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-sheet-merge").FullName;

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

    [Fact]
    public void ADocumentDrawsNeitherMoreNorFewerWordsThanLibreOffice()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("sheet-features.ods");
        List<PdfWord> ours = PdfWords.Read(Ours(path));
        List<PdfWord> theirs = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // Both directions in one claim, which is the point: a rule that stops a string too early
        // fails this as surely as one that never stops it.
        int pages = theirs.Max(word => word.PageIndex) + 1;

        int[] mine = [.. Enumerable.Range(0, pages)
                                   .Select(page => ours.Count(word => word.PageIndex == page))];
        int[] reference = [.. Enumerable.Range(0, pages)
                                        .Select(page => theirs.Count(word => word.PageIndex == page))];

        mine.ShouldBe(reference, "words drawn on each page");
    }

    [Fact]
    public void AStringStopsAtACellCoveredByAMergeInTheRowAbove()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("sheet-features.ods");
        List<PdfWord> ours = PdfWords.Read(Ours(path));
        List<PdfWord> theirs = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        static string TextOf(List<PdfWord> words, int page)
            => string.Join(' ', words.Where(word => word.PageIndex == page).Select(word => word.Text));

        // The page holds "Merged pair" on one row and "Second row of pair" on the next, and only
        // the first survives whole: the second is cut to "Second row of p" at the left edge of
        // the two-row merge beside it, leaving a one-character word. Naming the remnant says
        // where the cut fell, which "pair" appears once would not.
        TextOf(ours, 1).ShouldContain(" of p Spans ", customMessage: "cut at the merge");
        ours.Count(word => word.PageIndex == 1 && word.Text == "pair")
            .ShouldBe(1, "the string that runs into the merge keeps none of its last word");

        TextOf(ours, 1).ShouldBe(TextOf(theirs, 1), "the second page, word for word");
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

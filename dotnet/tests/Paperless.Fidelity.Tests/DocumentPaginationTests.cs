using System.Globalization;
using System.Text.RegularExpressions;
using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ooxml;
using Paperless.WordProcessing.OpenDocument;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Paginates a real document, from its bytes, and compares the pages with LibreOffice's.
/// </summary>
/// <remarks>
/// <para>
/// The whole path at once, with nothing hand-built: the file is read, its styles resolved, its page
/// geometry taken from its own master page, its paragraphs measured in the fonts it names and shaped the
/// way Writer shapes them, and the pages filled. Every earlier test isolates one link; this one is the
/// only evidence that the links are connected.
/// </para>
/// <para>
/// The comparison is the first few words of each page, because that is what a reader would notice and
/// what every upstream error moves. A wrong font size, a missed indent, a line spacing read from the
/// wrong attribute or an off-by-one in the fill loop all show up the same way — as a page starting at
/// the wrong word — and the numbered paragraphs make it obvious which way the error goes.
/// </para>
/// </remarks>
public sealed class DocumentPaginationTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-document-pages").FullName;

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
    [InlineData("paginated.fodt")]
    [InlineData("paginated.odt")]
    [InlineData("paginated.docx")]
    [InlineData("paginated.doc")]
    [InlineData("paginated.rtf")]
    public void ARealDocumentPaginatesTheWayLibreOfficeDoes(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<List<string>> reference = RenderedPages(path);
        List<List<string>> actual = PaperlessPages(path);

        actual.Count.ShouldBe(
            reference.Count,
            $"{fileName}: page count differs — Paperless {actual.Count}, "
            + $"LibreOffice {reference.Count}");

        for (int i = 0; i < reference.Count; i++)
        {
            // The first words, not every word: a page holds hundreds and a difference in any of them
            // means the same thing, so comparing four says as much and says it legibly.
            string expected = string.Join(' ', reference[i].Take(5));
            string got = string.Join(' ', actual[i].Take(5));

            got.ShouldBe(
                expected,
                $"{fileName}: page {i + 1} starts differently."
                + $"\n  LibreOffice: {expected}\n  Paperless:   {got}");
        }
    }

    [Theory]
    [InlineData("paginated.fodt")]
    [InlineData("paginated.odt")]
    [InlineData("paginated.docx")]
    [InlineData("paginated.doc")]
    [InlineData("paginated.rtf")]
    public void EveryParagraphOfTheDocumentLandsOnSomePage(string fileName)
    {
        // Pagination places lines; it must not lose them. A dropped paragraph would look like a
        // slightly short document rather than a bug, and the page-start comparison above would still
        // pass for every page before the one it was dropped from.
        using IDocument document = Open(Corpus.Require(fileName));
        WordProcessingPages pages = Paginate(document);

        List<int> paragraphIndices =
        [
            .. pages.Pages.SelectMany(p => p.Lines).Select(l => l.ParagraphIndex).Distinct().Order(),
        ];

        paragraphIndices.ShouldNotBeEmpty();
        paragraphIndices.ShouldBe(
            [.. Enumerable.Range(paragraphIndices[0], paragraphIndices.Count)],
            "the paragraphs on the pages have to be a contiguous run with none missing");

        // And no page is left empty, which is what a fill loop that mishandles a constraint produces.
        pages.Pages.ShouldAllBe(p => p.Lines.Count > 0);
    }

    [Fact]
    public void AHeadingIsNeverStrandedAtTheFootOfAPage()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        // The corpus document's headings keep with the next paragraph, which is what every real
        // template does. A heading alone at the foot of a page is the most visible pagination fault
        // there is, and it is what a keep-with-next chain that does not walk back produces.
        using IDocument document = Open(Corpus.Require("paginated.fodt"));
        WordProcessingPages pages = Paginate(document);

        IReadOnlyList<PageParagraph> paragraphs = Paragraphs(document);
        paragraphs.ShouldContain(
            p => p.Format.KeepWithNext,
            "the corpus document has to contain a keep-with-next paragraph for this to test anything");

        foreach (LaidOutPage page in pages.Pages.Take(pages.Count - 1))
        {
            int last = page.Lines[^1].ParagraphIndex;
            if (!paragraphs[last].Format.KeepWithNext) continue;

            page.Lines.ShouldContain(
                l => l.ParagraphIndex == last + 1,
                $"page {page.Index + 1} ends with a keep-with-next paragraph and nothing after it");
        }
    }

    [Fact]
    public void TheDocumentsOwnPageGeometryIsUsed()
    {
        // Not the defaults: the corpus document's page is A4 with a 2.5 cm left margin and a 2 cm
        // right one, so a layout that fell back to uniform margins would give every page the wrong
        // text width and every paragraph the wrong line count.
        using IDocument document = Open(Corpus.Require("paginated.fodt"));
        WordProcessingPages pages = Paginate(document);

        LaidOutPage first = pages.Pages[0];
        first.BodyArea.X.Millimetres.ShouldBe(25.0, tolerance: 0.1);
        first.BodyArea.Width.Millimetres.ShouldBe(210.0 - 25.0 - 20.0, tolerance: 0.1);
        first.Size.Height.Millimetres.ShouldBe(297.0, tolerance: 0.1);
    }

    [Theory]
    [InlineData("paginated.fodt")]
    [InlineData("paginated.odt")]
    [InlineData("paginated.docx")]
    [InlineData("paginated.doc")]
    [InlineData("paginated.rtf")]
    public void EveryFormatResolvesTheDocumentsOwnFont(string fileName)
    {
        // The corpus document asks for Carlito, which is installed. A substitution here would mean the
        // resolver failed to find an installed family, and every measurement after it would be against
        // a different font — so this fails loudly rather than letting the page comparison fail
        // mysteriously.
        Assert.SkipUnless(
            Directory.Exists("/usr/share/fonts"), "no font directory on this machine");

        using IDocument document = Open(Corpus.Require(fileName));
        WordProcessingPages pages = Paginate(document);

        // The document names Carlito, which is installed. Every format has to find it: a resolver that
        // failed would substitute something with different advance widths, and every measurement after
        // that would be against the wrong font — so this fails on the font rather than letting the page
        // comparison fail mysteriously three tests later.
        pages.Paragraphs.ShouldNotBeEmpty(fileName);
        pages.Paragraphs
            .Select(p => p.Face.FamilyName)
            .Distinct()
            .ShouldAllBe(name => name == "Carlito", fileName);
    }

    // ------------------------------------------------------------------------- the machinery

    private static IDocument Open(string path)
    {
        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        return new WordProcessingReader().Read(source);
    }

    private static WordProcessingPages Paginate(IDocument document)
    {
        IPaginatedDocument paginated = document.ShouldBeAssignableTo<IPaginatedDocument>()!;
        return (WordProcessingPages)paginated.Layout();
    }

    /// <summary>
    /// The paragraphs the document's own layout source produces.
    /// </summary>
    /// <remarks>
    /// Built again rather than taken from the pages, because a <see cref="PlacedLine"/> carries an index
    /// into this list and not the text itself — the paginator places lines and leaves the strings where
    /// they were. Reading them twice is cheap next to shaping them once.
    /// </remarks>
    private static IReadOnlyList<PageParagraph> Paragraphs(IDocument document)
        => [.. Paginate(document).Paragraphs];

    /// <summary>The words on each page Paperless produced, in order.</summary>
    /// <remarks>
    /// Taken from each line's own character range rather than from the paragraph's text, so a page
    /// starting mid-paragraph is compared against the words that are actually on it.
    /// </remarks>
    private static List<List<string>> PaperlessPages(string path)
    {
        using IDocument document = Open(path);
        WordProcessingPages pages = Paginate(document);

        List<List<string>> words = [];
        foreach (LaidOutPage page in pages.Pages)
        {
            List<string> onPage = [];
            foreach (PlacedLine line in page.Lines)
            {
                onPage.AddRange(pages.TextOf(line)
                    .Split(' ', StringSplitOptions.RemoveEmptyEntries));
            }
            words.Add(onPage);
        }

        return words;
    }

    /// <summary>The words on each page LibreOffice rendered, in order.</summary>
    private List<List<string>> RenderedPages(string documentPath)
    {
        string pdf = _libreOffice.ConvertToPdf(documentPath, _workDirectory);

        System.Diagnostics.ProcessStartInfo start = new("pdftotext")
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };
        start.ArgumentList.Add("-bbox");
        start.ArgumentList.Add(pdf);
        start.ArgumentList.Add("-");

        using System.Diagnostics.Process process =
            System.Diagnostics.Process.Start(start)
            ?? throw new InvalidOperationException("pdftotext did not start");

        string output = process.StandardOutput.ReadToEnd();
        process.WaitForExit((int)LibreOfficeRunner.Timeout.TotalMilliseconds);

        Assert.SkipWhen(
            process.ExitCode != 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        List<List<string>> pages = [];
        foreach (Match page in Regex.Matches(
                     output, "<page[^>]*>(.*?)</page>", RegexOptions.Singleline))
        {
            pages.Add(
            [
                .. Regex.Matches(page.Groups[1].Value, "<word[^>]*>([^<]*)</word>")
                    .Select(m => System.Net.WebUtility.HtmlDecode(m.Groups[1].Value)),
            ]);
        }

        return pages;
    }
}

using System.Diagnostics;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Threading;
using XRay.Office.Markup;
using XRay.Office.Rendering.Pdf;
using XRay.Office.Rendering.Raster;
using XRay.Office.Spreadsheets.Html;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// Cancelling a long operation, and the time limit that stands in for the caller when there is none.
/// </summary>
/// <remarks>
/// <para>
/// Three things are asked of every entry point that takes a token. A token that is already cancelled
/// throws before any work is done. A token cancelled while the work is running ends it promptly, and the
/// bound on "promptly" is generous here (seconds) because the machine running the suite is not the one
/// the documents are meant for; what matters is that it is a small fraction of the time the uncancelled
/// work takes, which each test establishes first. And a token that is never cancelled changes nothing:
/// the existing suites are that test for the token-free overloads, and <see cref="AnUncancelledTokenChangesNothing"/>
/// is it for the new ones.
/// </para>
/// <para>
/// The inputs are generated, not committed: a document that takes seconds to lay out is megabytes of
/// XML, and what makes it slow is repetition, which is cheap to write out.
/// </para>
/// </remarks>
public sealed class CancellationTests
{
    /// <summary>
    /// How long the cancelled work may run on after the token fires.
    /// </summary>
    private static readonly TimeSpan Bound = TimeSpan.FromSeconds(10);

    private static byte[] BigDocx { get; } = GeneratedDocuments.Docx(8_000);

    private static byte[] BigXlsx { get; } = GeneratedDocuments.Xlsx(40_000, 10);

    private static CancellationToken Cancelled()
    {
        CancellationTokenSource source = new();
        source.Cancel();
        return source.Token;
    }

    private static IDocument OpenDocx(CancellationToken token = default)
        => OfficeDocument.Open(DocumentSource.FromBytes(BigDocx, "big.docx"), token);

    // ------------------------------------------------------------------------------ already cancelled

    [Fact]
    public void OpeningWithACancelledTokenThrowsBeforeReadingAnything()
    {
        Should.Throw<OperationCanceledException>(() => OfficeDocument.Open(DocumentSource.FromBytes(BigDocx), Cancelled()));
        Should.Throw<OperationCanceledException>(() => OfficeDocument.Extract(DocumentSource.FromBytes(BigDocx), Cancelled()));

        string path = Corpus.Require("word-features.docx");
        Should.Throw<OperationCanceledException>(() => OfficeDocument.Open(path, Cancelled()));
        Should.Throw<OperationCanceledException>(() => OfficeDocument.Extract(path, Cancelled()));
    }

    [Fact]
    public async Task OpenAsyncHonoursTheTokenToo()
    {
        await Should.ThrowAsync<OperationCanceledException>(
            () => OfficeDocument.OpenAsync(DocumentSource.FromBytes(BigDocx), Cancelled()));
        await Should.ThrowAsync<OperationCanceledException>(
            () => OfficeDocument.ExtractAsync(Corpus.Require("word-features.docx"), Cancelled()));

        using IDocument document = await OfficeDocument.OpenAsync(Corpus.Require("word-features.docx"));
        document.Content.GetText().ShouldContain("Heading");
    }

    [Fact]
    public void LayoutWithACancelledTokenThrows()
    {
        using IDocument document = OfficeDocument.Open(Corpus.Require("word-features.docx"));

        Should.Throw<OperationCanceledException>(
            () => ((IPaginatedDocument)document).Layout(null, Cancelled()));
    }

    [Fact]
    public void RenderingWithACancelledTokenThrowsAndWritesNothing()
    {
        using IDocument document = OfficeDocument.Open(Corpus.Require("word-features.docx"));
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using MemoryStream pdf = new();
        Should.Throw<OperationCanceledException>(() => new PdfRenderer().Render(pages, pdf, Cancelled()));
        pdf.Length.ShouldBe(0, "cancelled before the first byte, so there is no truncated file to discard");

        using MemoryStream png = new();
        Should.Throw<OperationCanceledException>(
            () => new RasterRenderer().Render(pages[0], png, RasterFormat.Png, Cancelled()));
        png.Length.ShouldBe(0);

        Should.Throw<OperationCanceledException>(() => new RasterRenderer().Rasterise(pages[0], Cancelled()));
    }

    [Fact]
    public void TheHtmlAndMarkdownWritersHonourTheTokenToo()
    {
        using IDocument book = OfficeDocument.Open(DocumentSource.FromBytes(BigXlsx, "big.xlsx"));
        SpreadsheetPages sheets = (SpreadsheetPages)((IPaginatedDocument)book).Layout(new LayoutOptions { MaxPages = 1 });

        using MemoryStream html = new();
        Should.Throw<OperationCanceledException>(
            () => SheetHtmlWriter.Write(sheets, html, null, Cancelled()));

        Should.Throw<OperationCanceledException>(
            () => MarkdownWriter.ToMarkdown(book.Content, null, Cancelled()));
        Should.Throw<OperationCanceledException>(
            () => XhtmlWriter.ToXhtml(book.Content, null, Cancelled()));
    }

    // ----------------------------------------------------------------------------- cancelled part way

    [Fact]
    public void CancellingAHugeLayoutPartWayReturnsPromptly()
    {
        using IDocument document = OpenDocx();

        // Established first: laid out in full, this takes a good deal longer than the bound, or the
        // test would pass for a layout that ignored the token.
        Stopwatch whole = Stopwatch.StartNew();
        using (CancellationTokenSource limit = new(TimeSpan.FromSeconds(60)))
        {
            ((IPaginatedDocument)document).Layout(new LayoutOptions { MaxPages = 40 }, limit.Token);
        }

        whole.Stop();

        using CancellationTokenSource source = new();
        source.CancelAfter(TimeSpan.FromMilliseconds(Math.Max(50, whole.ElapsedMilliseconds / 4)));

        Stopwatch watch = Stopwatch.StartNew();
        Exception? thrown = Record.Exception(() => ((IPaginatedDocument)document).Layout(null, source.Token));
        watch.Stop();

        thrown.ShouldBeOfType<OperationCanceledException>("a caller's own token is not a timeout");
        watch.Elapsed.ShouldBeLessThan(Bound);
    }

    [Fact]
    public void CancellingTheReadOfAHugeWorkbookPartWayReturnsPromptly()
    {
        using CancellationTokenSource source = new();
        source.CancelAfter(TimeSpan.FromMilliseconds(30));

        Stopwatch watch = Stopwatch.StartNew();
        Exception? thrown = Record.Exception(
            () => OfficeDocument.Open(DocumentSource.FromBytes(BigXlsx, "big.xlsx"), source.Token));
        watch.Stop();

        thrown.ShouldBeOfType<OperationCanceledException>();
        watch.Elapsed.ShouldBeLessThan(Bound);
    }

    [Fact]
    public void CancellingTheReadOfAHugeDocumentPartWayReturnsPromptly()
    {
        using CancellationTokenSource source = new();
        source.CancelAfter(TimeSpan.FromMilliseconds(20));

        Stopwatch watch = Stopwatch.StartNew();
        Exception? thrown = Record.Exception(
            () => OfficeDocument.Open(DocumentSource.FromBytes(GeneratedDocuments.Docx(60_000), "huge.docx"), source.Token));
        watch.Stop();

        thrown.ShouldBeOfType<OperationCanceledException>();
        watch.Elapsed.ShouldBeLessThan(Bound);
    }

    [Fact]
    public void CancellingARenderPartWayStopsBetweenPages()
    {
        using IDocument document = OpenDocx();
        IPageSequence pages = ((IPaginatedDocument)document).Layout(new LayoutOptions { MaxPages = 60 });
        pages.Count.ShouldBeGreaterThan(20);

        using CancellationTokenSource source = new();
        int drawn = 0;
        IEnumerable<IPage> Counting()
        {
            for (int i = 0; i < pages.Count; i++)
            {
                if (++drawn == 5) source.Cancel();
                yield return pages[i];
            }
        }

        using MemoryStream pdf = new();
        Should.Throw<OperationCanceledException>(() => new PdfRenderer().Render(Counting(), pdf, source.Token));

        // The page being drawn when the token fired may finish; the ones after it are not started.
        drawn.ShouldBeLessThan(8);
    }

    // ------------------------------------------------------------------------------------- the timeout

    [Fact]
    public void ALayoutTimeoutThrowsTheTimeoutSubtypeAndNotACallerCancellation()
    {
        using IDocument document = OpenDocx();

        LayoutTimeoutException timeout = Should.Throw<LayoutTimeoutException>(
            () => ((IPaginatedDocument)document).Layout(new LayoutOptions { Timeout = TimeSpan.FromMilliseconds(40) }));

        // Catchable as a cancellation, which is what it is, and distinguishable from one.
        timeout.ShouldBeAssignableTo<OperationCanceledException>();
        timeout.Timeout.ShouldBe(TimeSpan.FromMilliseconds(40));
        timeout.CancellationToken.CanBeCanceled.ShouldBeFalse();
    }

    [Fact]
    public void ATimeoutAndATokenTogetherReportWhicheverFiredFirst()
    {
        using IDocument document = OpenDocx();

        // The caller's token fires well before the limit does.
        using CancellationTokenSource early = new(TimeSpan.FromMilliseconds(40));
        Exception? byCaller = Record.Exception(() => ((IPaginatedDocument)document).Layout(
            new LayoutOptions { Timeout = TimeSpan.FromSeconds(120) }, early.Token));
        byCaller.ShouldBeOfType<OperationCanceledException>();
        byCaller.ShouldNotBeOfType<LayoutTimeoutException>();

        // And the limit before the caller's token.
        using CancellationTokenSource late = new(TimeSpan.FromSeconds(120));
        Exception? byLimit = Record.Exception(() => ((IPaginatedDocument)document).Layout(
            new LayoutOptions { Timeout = TimeSpan.FromMilliseconds(40) }, late.Token));
        byLimit.ShouldBeOfType<LayoutTimeoutException>();
    }

    [Fact]
    public void ALimitThatIsNeverReachedChangesNothing()
    {
        using IDocument document = OfficeDocument.Open(Corpus.Require("word-features.docx"));

        IPageSequence plain = ((IPaginatedDocument)document).Layout();
        IPageSequence limited = ((IPaginatedDocument)document).Layout(new LayoutOptions { Timeout = TimeSpan.FromMinutes(5) });

        limited.Count.ShouldBe(plain.Count);
    }

    [Fact]
    public void ANonPositiveTimeoutMeansNoLimit()
    {
        using IDocument document = OfficeDocument.Open(Corpus.Require("word-features.docx"));

        ((IPaginatedDocument)document).Layout(new LayoutOptions { Timeout = TimeSpan.Zero }).Count.ShouldBeGreaterThan(0);
        ((IPaginatedDocument)document).Layout(new LayoutOptions { Timeout = TimeSpan.FromSeconds(-1) }).Count.ShouldBeGreaterThan(0);
    }

    [Fact]
    public void ATimeoutIsAlsoHonouredForSpreadsheetsAndDecks()
    {
        using IDocument deck = OfficeDocument.Open(Corpus.Require("deck-features.pptx"));
        Should.Throw<LayoutTimeoutException>(() => ((IPaginatedDocument)deck).Layout(
            new LayoutOptions { Timeout = TimeSpan.FromTicks(1) })).ShouldNotBeNull();

        using IDocument book = OfficeDocument.Open(DocumentSource.FromBytes(BigXlsx, "big.xlsx"));
        Should.Throw<LayoutTimeoutException>(() => ((IPaginatedDocument)book).Layout(
            new LayoutOptions { Timeout = TimeSpan.FromMilliseconds(1) }));
    }

    // --------------------------------------------------------------------------------- nothing changes

    [Fact]
    public void AnUncancelledTokenChangesNothing()
    {
        string path = Corpus.Require("word-features.docx");
        using CancellationTokenSource source = new();

        using IDocument plain = OfficeDocument.Open(path);
        using IDocument withToken = OfficeDocument.Open(path, source.Token);
        withToken.Content.GetText().ShouldBe(plain.Content.GetText());

        IPageSequence a = ((IPaginatedDocument)plain).Layout();
        IPageSequence b = ((IPaginatedDocument)withToken).Layout(null, source.Token);
        b.Count.ShouldBe(a.Count);

        PdfRenderOptions fixedDate = new() { CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero) };
        using MemoryStream first = new();
        using MemoryStream second = new();
        new PdfRenderer(fixedDate).Render(a, first);
        new PdfRenderer(fixedDate).Render(b, second, source.Token);
        second.ToArray().ShouldBe(first.ToArray(), "the same document renders to the same bytes with or without a token");

        MarkdownWriter.ToMarkdown(withToken.Content, null, source.Token).ShouldBe(MarkdownWriter.ToMarkdown(plain.Content));
    }

    [Fact]
    public void TheAmbientScopeNestsAndRestores()
    {
        OfficeOperation.IsCancellable.ShouldBeFalse();
        OfficeOperation.ThrowIfCancelled();

        using CancellationTokenSource outer = new();
        using (OfficeOperation.Begin(cancellationToken: outer.Token))
        {
            OfficeOperation.IsCancellable.ShouldBeTrue();

            using CancellationTokenSource inner = new();
            using (OfficeOperation.Begin(cancellationToken: inner.Token))
            {
                // Cancelling the enclosing operation cancels the nested one.
                outer.Cancel();
                Should.Throw<OperationCanceledException>(OfficeOperation.ThrowIfCancelled);
            }

            Should.Throw<OperationCanceledException>(OfficeOperation.ThrowIfCancelled);
        }

        OfficeOperation.IsCancellable.ShouldBeFalse();
        OfficeOperation.ThrowIfCancelled();
    }

    [Theory]
    [InlineData("word-features.docx")]
    [InlineData("bidi-columns.doc")]
    [InlineData("bidi-drawing.rtf")]
    [InlineData("bookmark-field.odt")]
    [InlineData("chart-bar-sheet.xlsx")]
    [InlineData("chart-bar-sheet.ods")]
    [InlineData("deck-features.pptx")]
    [InlineData("ppt-features.ppt")]
    [InlineData("sheet-shape-group-margin.xls")]
    [InlineData("chart-bar-deck.odp")]
    public void ACancellationDuringTheReadIsNeverSwallowedIntoADiagnostic(string file)
    {
        // A reader that turned the cancellation into "the part could not be read" would return a document
        // with a quiet hole in it instead of stopping. The token here is cancelled from inside the first
        // read of the file's own bytes, so it fires while the reader is running rather than before it.
        using CancellationTokenSource source = new();
        using CancelOnReadStream stream = new(File.ReadAllBytes(Corpus.Require(file)), source);

        Exception? thrown = Record.Exception(
            () => OfficeDocument.Open(DocumentSource.FromStream(stream, file), source.Token));

        thrown.ShouldBeOfType<OperationCanceledException>(file);
    }

    /// <summary>A seekable stream that cancels a token the first time it is read.</summary>
    private sealed class CancelOnReadStream(byte[] bytes, CancellationTokenSource source) : MemoryStream(bytes, writable: false)
    {
        public override int Read(byte[] buffer, int offset, int count)
        {
            int read = base.Read(buffer, offset, count);
            source.Cancel();
            return read;
        }

        public override int Read(Span<byte> buffer)
        {
            int read = base.Read(buffer);
            source.Cancel();
            return read;
        }
    }
}

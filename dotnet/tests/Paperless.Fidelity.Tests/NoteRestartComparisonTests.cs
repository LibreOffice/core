using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Pins the per-page note restart: the numbering, and that it is what LibreOffice renders.
/// </summary>
/// <remarks>
/// <para>
/// The rule a reader cannot resolve. A note's number under a per-page restart is its position within its
/// page, the width of the citation follows from the number, and where the citing line breaks follows from
/// the width — so the answer depends on the pagination that depends on the answer. Writer settles it by
/// numbering over pages that already exist and then damping: <c>SwRootFrame::UpdateFootnoteNums</c>
/// (<c>ftnfrm.cxx</c>:971) walks the finished pages, and <c>txtftn.cxx</c>:560 validates the frame one
/// iteration later under the comment <em>"We break the oscillation"</em>. This engine does the same, in
/// <c>Layout/NoteRenumbering.cs</c>.
/// </para>
/// <para>
/// <c>note-restart.*</c> is the ground truth and the corpus was built for it: two pages, eight footnotes,
/// four to a page, and all four formats keep the rule through LibreOffice's own export in four different
/// spellings — <c>text:start-numbering-at="page"</c>, <c>w:numRestart w:val="eachPage"</c>,
/// <c>\ftnrstpg</c> and the DOP's <c>rncFootnote</c>. All of them render page one citing 1, 2, 3, 4 and
/// page two citing 1, 2, 3, 4 again.
/// </para>
/// </remarks>
public sealed class NoteRestartComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-restart").FullName;

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

    /// <summary>
    /// Each page's notes are cited from one again, at the anchor and at the head of the note alike.
    /// </summary>
    /// <remarks>
    /// Asserted against the laid-out model rather than against a rendering, because this is the half that
    /// is format-independent and because it has teeth in every format: a PDF text extractor puts a
    /// superscript wherever it likes and two of the five files show no citation in their extracted note
    /// lines at all. Both copies are checked — LibreOffice draws a note's number twice, and a renumbering
    /// that rewrote only the sentence would leave the note at the foot of the page still claiming to be
    /// the note it used to be.
    /// </remarks>
    [Theory]
    [InlineData("note-restart.fodt")]
    [InlineData("note-restart.odt")]
    [InlineData("note-restart.docx")]
    [InlineData("note-restart.doc")]
    [InlineData("note-restart.rtf")]
    public void EveryPageCountsItsNotesFromOne(string fileName)
    {
        using IDocument document = PaperlessDocument.Open(Corpus.Require(fileName));
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        pages.Count.ShouldBe(2, $"{fileName}: pages");

        int checked_ = 0;
        for (int index = 0; index < pages.Count; index++)
        {
            LaidOutPage page = pages.Pages[index];
            IReadOnlyList<PageBlock> blocks = page.Blocks ?? pages.Blocks;

            int expected = 0;
            foreach ((PageParagraph paragraph, PageNote note) in NotesOn(blocks, page))
            {
                expected++;
                string wanted = expected.ToString(System.Globalization.CultureInfo.InvariantCulture);

                note.Citation.ShouldBe(
                    wanted, $"{fileName}: note {expected} on page {index + 1}");

                // The number as it will actually be drawn, in the sentence and at the head of the note.
                // The record and the text are two different things and only the text is rendered.
                paragraph.Text.Substring(note.Offset, note.Citation.Length).ShouldBe(
                    wanted, $"{fileName}: the anchor on page {index + 1}, note {expected}");

                PageParagraph body = note.Blocks[0].ShouldBeOfType<PageParagraph>();
                body.Text.Substring(note.BodyOffset, note.Citation.Length).ShouldBe(
                    wanted, $"{fileName}: the note's own head on page {index + 1}, note {expected}");

                checked_++;
            }

            expected.ShouldBe(4, $"{fileName}: notes on page {index + 1}");
        }

        checked_.ShouldBe(8, $"{fileName}: notes checked");
    }

    /// <summary>
    /// And that those are the numbers LibreOffice renders, read out of both PDFs.
    /// </summary>
    /// <remarks>
    /// The other half, and the one that would catch a rule applied consistently and wrongly — numbering
    /// from one per page is a self-consistent answer whether or not it is Writer's. Compared as extracted
    /// text rather than as pens, because what is being checked here is which characters were drawn.
    /// </remarks>
    [Theory]
    [InlineData("note-restart.fodt")]
    [InlineData("note-restart.odt")]
    [InlineData("note-restart.docx")]
    [InlineData("note-restart.doc")]
    [InlineData("note-restart.rtf")]
    public void TheNumbersAreTheOnesLibreOfficeDraws(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        string? ours = ExtractedText(Ours(path));
        string? theirs = ExtractedText(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(ours is null || theirs is null, "pdftotext is not available; install poppler-utils");

        TestKit.Comparison.TextComparer.Normalise(ours!, foldSpaces: true)
            .ShouldBe(
                TestKit.Comparison.TextComparer.Normalise(theirs!, foldSpaces: true),
                $"{fileName}: the text drawn differs from LibreOffice's");
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>
    /// The notes a page holds, in the order they are cited, with the paragraphs citing them.
    /// </summary>
    /// <remarks>
    /// The same rule pagination uses — a note belongs to the page holding the line that contains its
    /// anchor — restated here rather than reached for, so that the test would still fail if the engine
    /// changed its mind about which page a note is on.
    /// </remarks>
    private static IEnumerable<(PageParagraph Paragraph, PageNote Note)> NotesOn(
        IReadOnlyList<PageBlock> blocks, LaidOutPage page)
    {
        foreach (PlacedLine line in page.Lines)
        {
            if (line.ParagraphIndex >= blocks.Count) continue;
            if (blocks[line.ParagraphIndex] is not PageParagraph paragraph) continue;

            foreach (PageNote note in paragraph.Notes)
            {
                if (note.Offset >= line.Box.Line.Start && note.Offset < line.Box.Line.End)
                {
                    yield return (paragraph, note);
                }
            }
        }
    }

    /// <summary>Renders a document to a PDF of our own, and returns its path.</summary>
    private string Ours(string documentPath)
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-restart.pdf");

        using IDocument document = PaperlessDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }

    private static string? ExtractedText(string pdf)
    {
        System.Diagnostics.ProcessStartInfo start = new("pdftotext") { RedirectStandardOutput = true };
        start.ArgumentList.Add("-layout");
        start.ArgumentList.Add(pdf);
        start.ArgumentList.Add("-");

        try
        {
            using System.Diagnostics.Process? process = System.Diagnostics.Process.Start(start);
            if (process is null) return null;

            string output = process.StandardOutput.ReadToEnd();
            process.WaitForExit((int)LibreOfficeRunner.Timeout.TotalMilliseconds);
            return process.ExitCode == 0 ? output : null;
        }
        catch (System.ComponentModel.Win32Exception)
        {
            return null;
        }
    }
}

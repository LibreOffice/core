using XRay.Office.Core.Documents;
using XRay.Office.Rendering.Pdf;
using XRay.Office.TestKit;
using XRay.Office.TestKit.LibreOffice;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.Fidelity.Tests;

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
// [reference moved 24.2.7.2 -> 26.2.4.2, closed] `TheNumbersAreTheOnesLibreOfficeDraws` failed on
// `note-restart.docx` on four characters, and every one of them was `pdftotext -layout`'s own column
// padding: see `WithoutLayoutIndent` for the measurement. Nothing about the rendering differs.
public sealed class NoteRestartComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("xray-office-restart").FullName;

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
        using IDocument document = OfficeDocument.Open(Corpus.Require(fileName));
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
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

        string path = Corpus.Require(fileName);
        string? ours = ExtractedText(Ours(path));
        string? theirs = ExtractedText(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(ours is null || theirs is null, "pdftotext is not available; install poppler-utils");

        WithoutLayoutIndent(TestKit.Comparison.TextComparer.Normalise(ours!, foldSpaces: true))
            .ShouldBe(
                WithoutLayoutIndent(
                    TestKit.Comparison.TextComparer.Normalise(theirs!, foldSpaces: true)),
                $"{fileName}: the text drawn differs from LibreOffice's");
    }

    /// <summary>
    /// Drops the leading spaces <c>pdftotext -layout</c> pads a line with, keeping the characters.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>-layout</c> reconstructs columns on a character grid, so a line's indent is a count of grid
    /// cells rather than anything the document drew — and a pen half a cell either side of a boundary
    /// rounds to a different count. This document's note bodies sit exactly there.
    /// </para>
    /// <para>
    /// Measured, the note body's first word on page one: <b>59.750 pt under 24.2.7.2, 59.700 under
    /// 26.2.4.2 and 59.640 in ours</b>, against a body text left edge of 56.8 pt in both references and
    /// 56.7 in ours — so every rendering puts it 2.90 to 2.94 pt into a 5.5 pt character cell, and
    /// poppler pads the reference with one space under 26.2.4.2 and two under 24.2.7.2. Four such lines
    /// on page one is the whole of what this comparison saw: with the indent dropped, our extracted text
    /// and 26.2.4.2's are identical byte for byte.
    /// </para>
    /// <para>
    /// The trailing end is already trimmed by <c>TextComparer.Normalise</c>, and for the same reason.
    /// What is being checked here is which characters were drawn, which is why dropping the padding is a
    /// correction and not a loosening: an indent poppler invented is not a character the document has.
    /// </para>
    /// </remarks>
    /// <param name="text">The extracted text, already normalised.</param>
    private static string WithoutLayoutIndent(string text)
        => string.Join('\n', text.Split('\n').Select(line => line.TrimStart(' ')));

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

        using IDocument document = OfficeDocument.Open(documentPath);
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

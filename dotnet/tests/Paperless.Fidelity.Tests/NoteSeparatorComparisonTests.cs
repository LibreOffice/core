using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Attributes the one note separator in the corpus that does not agree with LibreOffice.
/// </summary>
/// <remarks>
/// <para>
/// <c>PdfOutputComparisonTests</c> compares the rule above a page's footnotes against LibreOffice's own
/// filled rectangle for four of the five spellings of <c>footnotes.*</c> and leaves the RTF out. This
/// says why, in a form that fails if the reason stops being true.
/// </para>
/// <para>
/// The reason is not the separator. Its geometry is a function of where the notes landed — a quarter of
/// the text width, half a point thick, 0.1 cm above the first note's line box — and the notes land
/// somewhere else in LibreOffice's RTF rendering because it sets them in a different face. The file
/// states <c>\f4</c>, which its own font table calls Carlito, both directly on the note paragraph and
/// through the <c>\s26</c> style the paragraph names; LibreOffice's RTF import loses both and falls back
/// to the document's default face. Its PDF of <c>footnotes.rtf</c> embeds Liberation Serif beside
/// Carlito and draws the note text in it, where its PDF of <c>footnotes.fodt</c> — the same document —
/// embeds Carlito alone.
/// </para>
/// <para>
/// Liberation Serif is the shorter face: 11.55 pt to Carlito's 12.20 at ten point, 13 twips a line. Two
/// notes, 26 twips, 1.30 pt — which is the 1.286 pt the separator differs by, less the sub-twip rounding
/// of a note area that is bottom-aligned rather than stacked from the top.
/// </para>
/// <para>
/// Bisected on hand-written RTF rather than assumed, because "the import drops what is inside a footnote
/// group" is too broad to be true: a note stating <c>\f1</c> with no style reference keeps its face, and
/// the same note with a <c>\sN</c> in front of it loses the style's font <em>and</em> the direct one.
/// </para>
/// </remarks>
public sealed class NoteSeparatorComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, two twips, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>The em size the corpus document sets its notes at; the body is eleven.</summary>
    private const double NoteSizePoints = 10;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-notesep").FullName;

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
    /// LibreOffice sets the RTF notes in a face the file does not name, and only the RTF ones.
    /// </summary>
    /// <remarks>
    /// The evidence rather than the consequence, and the cheapest form of it: a PDF names the font
    /// resource each run is drawn with, so "the notes are in a different face from the body" is a
    /// comparison of two strings within one file and needs no font matching at all.
    /// </remarks>
    [Fact]
    public void LibreOfficeSetsTheRtfNotesInADifferentFaceFromTheBody()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<PdfTextRun> rtf = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(Corpus.Require("footnotes.rtf"), _workDirectory));
        List<PdfTextRun> odf = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(Corpus.Require("footnotes.fodt"), _workDirectory));

        FaceOfNotes(rtf).ShouldNotBe(
            FaceOfBody(rtf),
            "LibreOffice's RTF rendering draws the notes in the face the file names after all, so the "
            + "separator difference this file exists to explain has some other cause");

        FaceOfNotes(odf).ShouldBe(
            FaceOfBody(odf),
            "LibreOffice's ODF rendering of the same document should set body and notes in one face");
    }

    /// <summary>
    /// The separator's whole disagreement is the note area being shorter by a substituted face.
    /// </summary>
    /// <remarks>
    /// The separator sits a fixed distance above the first note line, and the note area is bottom-aligned
    /// in the body's rectangle — so shortening every note line by the same amount raises the whole area's
    /// top, and the rule with it, by the line count times the difference. If that arithmetic accounts for
    /// the gap, nothing is left over for this engine to have got wrong.
    /// </remarks>
    [Fact]
    public void TheRtfSeparatorGapIsExactlyTheShorterNoteLines()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("footnotes.rtf");
        string ours = Ours(path);
        string theirs = _libreOffice.ConvertToPdf(path, _workDirectory);

        List<double> ourNotes = NoteBaselines(PdfTextRuns.Read(ours));
        List<double> theirNotes = NoteBaselines(PdfTextRuns.Read(theirs));

        ourNotes.Count.ShouldBe(2, "footnotes.rtf: note lines drawn");
        theirNotes.Count.ShouldBe(theirNotes.Count, "footnotes.rtf: note lines rendered");

        double ourPitch = ourNotes[1] - ourNotes[0];
        double theirPitch = theirNotes[1] - theirNotes[0];

        // We honour the file, which is what the other four formats of the same document render as.
        ourPitch.ShouldBeGreaterThan(
            theirPitch,
            $"footnotes.rtf: our note pitch is {ourPitch:F3} pt and LibreOffice's {theirPitch:F3} pt");

        PdfFill ourRule = OnlyFill(ours);
        PdfFill theirRule = OnlyFill(theirs);

        // Bottom-aligned, so the top of the area — and the rule above it — rises by every line's worth
        // of the difference.
        double explained = ourNotes.Count * (theirPitch - ourPitch);
        double observed = ourRule.Top - theirRule.Top;

        Math.Abs(observed - explained).ShouldBeLessThanOrEqualTo(
            TolerancePoints,
            $"footnotes.rtf: the rule sits {observed:F3} pt from LibreOffice's, of which "
            + $"{explained:F3} pt is its shorter note lines — the remainder is unaccounted for");
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>Renders a document to a PDF of our own, and returns its path.</summary>
    private string Ours(string documentPath)
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-notesep.pdf");

        using IDocument document = PaperlessDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }

    /// <summary>The one rectangle the document fills, which is the rule above its notes.</summary>
    private static PdfFill OnlyFill(string pdfPath)
    {
        List<PdfFill> fills = [.. PdfFills.Read(pdfPath).Distinct()];
        fills.Count.ShouldBe(1, $"{Path.GetFileName(pdfPath)}: filled rectangles");
        return fills[0];
    }

    /// <summary>
    /// The baselines the note text sits on, one per line.
    /// </summary>
    /// <remarks>
    /// By size, which separates the notes from the body without knowing anything else about the page:
    /// the corpus document sets its body at eleven point and its notes at ten. The citation at the head
    /// of a note is smaller again and raised, so it lands on a baseline of its own and is excluded by the
    /// same test.
    /// </remarks>
    private static List<double> NoteBaselines(List<PdfTextRun> runs)
    {
        List<double> baselines = [];

        foreach (PdfTextRun run in runs)
        {
            if (Math.Abs(run.FontSize - NoteSizePoints) > 0.001) continue;
            if (baselines.Count > 0 && Math.Abs(baselines[^1] - run.Y) <= 0.5) continue;

            baselines.Add(run.Y);
        }

        return baselines;
    }

    private static string FaceOfNotes(List<PdfTextRun> runs)
        => runs.First(run => Math.Abs(run.FontSize - NoteSizePoints) <= 0.001).FontResource;

    private static string FaceOfBody(List<PdfTextRun> runs)
        => runs.First(run => run.FontSize > NoteSizePoints + 0.5).FontResource;
}

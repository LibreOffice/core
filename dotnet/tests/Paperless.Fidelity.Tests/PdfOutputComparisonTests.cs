using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares the PDF Paperless writes against the PDF LibreOffice writes, operator for operator.
/// </summary>
/// <remarks>
/// <para>
/// The strongest comparison available anywhere in this project, and it costs nothing extra to
/// have: the fidelity harness in <c>tests/Paperless.TestKit/LibreOffice/</c> was written to
/// read LibreOffice's PDFs — <see cref="PdfTextRuns"/> for pen positions and sizes,
/// <see cref="PdfFills"/> for filled rectangles, <see cref="PdfStrokes"/> for stroked lines —
/// and nothing in it is specific to LibreOffice. Pointed at our own output it compares two
/// content streams rather than two pictures, which is both a sharper question than "do these
/// look the same" and a far easier one to attribute when the answer is no.
/// </para>
/// <para>
/// It is also what makes the PDF backend verifiable to the same tenth of a point as the layout
/// underneath it. An image diff of the same two pages answers with a percentage; this answers
/// with "the pen was here and it should have been there".
/// </para>
/// <para>
/// <b>Compared per line, not per run.</b> A line is the unit both writers agree on; a
/// <em>portion</em> is not, and the two disagree about it in two measured ways that are about
/// where LibreOffice chooses to end a <c>BT … ET</c> block rather than about where anything is
/// drawn:
/// </para>
/// <list type="bullet">
///   <item>
///     It draws the <b>blank at the end of a wrapped line</b> as a run of its own. On
///     <c>paginated.fodt</c> that is eleven extra one-glyph runs, each at the right-hand end of
///     a line it has already drawn — <c>537.1 514.689 Td … 1 glyph</c>, the space after
///     "ligature s".
///   </item>
///   <item>
///     It splits a <b>decimal tab</b> at the decimal point. In <c>tabbed.odt</c> the entry
///     <c>12.5</c> is two runs, <c>12</c> at 456.65 pt and <c>.5</c> at 467.8, where we draw one
///     at 456.55. Same four glyphs, same place, two blocks instead of one.
///   </item>
/// </list>
/// <para>
/// Grouping runs by baseline makes both differences vanish without weakening anything that
/// matters: the line's pen position, its size and how many glyphs it holds are all still
/// compared exactly.
/// </para>
/// </remarks>
public sealed class PdfOutputComparisonTests : IDisposable
{
    /// <summary>
    /// A tenth of a point, two twips, as everywhere else in this project.
    /// </summary>
    private const double TolerancePoints = 0.1;

    /// <summary>
    /// What LibreOffice's PDF export adds to every pen position horizontally.
    /// </summary>
    /// <remarks>
    /// Two twips, additive and not a scale, recorded in <c>src/Paperless.WordProcessing/TODO.md</c>
    /// under "Known deviations, measured" and reproduced here because this comparison sees it
    /// directly: over the ten documents below the difference between our pen and its pen is
    /// exactly -0.100 pt on eight of them, and between -0.034 and -0.142 on the two whose lines
    /// are justified, where its own per-blank rounding moves the start of a line as well.
    /// </remarks>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-pdfout").FullName;

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
    [InlineData("columns.odt")]
    [InlineData("sections.docx")]
    [InlineData("justified.odt")]
    [InlineData("tabbed.odt")]
    [InlineData("footnotes.docx")]
    public void EveryLineIsShownWhereLibreOfficeShowsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfTextRun> ours = LinesOf(PdfTextRuns.Read(Ours(path)));
        List<PdfTextRun> theirs = LinesOf(PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        ours.Count.ShouldBe(theirs.Count, $"{fileName}: number of drawn lines");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfTextRun mine = ours[i];
            PdfTextRun reference = theirs[i];
            string where = $"{fileName}: line {i + 1} on page {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: page");

            // The pen, which is the one number a whole-page offset or a wrong margin shows up in.
            Math.Abs(mine.X - (reference.X - PdfPenOffsetPoints)).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{where}: pen at {mine.X:F3} pt, {reference.X - PdfPenOffsetPoints:F3} pt rendered");

            // The size, exactly. A PDF states it in the Tf operator, so there is nothing to round:
            // a difference here is a font size read wrongly, not a measurement.
            mine.FontSize.ShouldBe(reference.FontSize, 0.001, $"{where}: font size");

            // And how many glyphs the line holds. One fewer is allowed and only one: LibreOffice
            // draws the blank a wrapped line ends with and we do not, which is a glyph that
            // occupies the margin and marks nothing.
            (reference.GlyphCount - mine.GlyphCount).ShouldBeInRange(
                0, 1, $"{where}: {mine.GlyphCount} glyphs drawn, {reference.GlyphCount} rendered");
        }
    }

    [Theory]
    [InlineData("paginated.fodt")]
    [InlineData("paginated.docx")]
    [InlineData("paginated.doc")]
    [InlineData("paginated.rtf")]
    [InlineData("columns.odt")]
    [InlineData("sections.docx")]
    [InlineData("footnotes.docx")]
    public void EveryBaselineFallsOnLibreOfficesPitch(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfTextRun> ours = LinesOf(PdfTextRuns.Read(Ours(path)));
        List<PdfTextRun> theirs = LinesOf(PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        ours.Count.ShouldBe(theirs.Count, $"{fileName}: number of drawn lines");

        // As a pitch rather than an absolute baseline, which is a statement about what this file
        // is for rather than about what is comparable: both sides state a real baseline, so the
        // absolute form is meaningful and holds to 0.051 pt over every one of these documents.
        // ParagraphLeadingComparisonTests asserts it that way, because that is the only form in
        // which the leading a paragraph hands to the next one is visible at all — a pitch
        // comparison cancels it. Here the pitch is the right question: this file is checking that
        // the PDF backend writes the layout it was given.
        int compared = 0;
        for (int i = 1; i < theirs.Count; i++)
        {
            if (theirs[i].PageIndex != theirs[i - 1].PageIndex) continue;

            double drawn = ours[i].Y - ours[i - 1].Y;
            double rendered = theirs[i].Y - theirs[i - 1].Y;

            Math.Abs(drawn - rendered).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: pitch into line {i + 1} is {drawn:F3} pt drawn, {rendered:F3} pt rendered");

            compared++;
        }

        compared.ShouldBeGreaterThan(10, $"{fileName}: too few comparable pitches to prove anything");
    }

    [Theory]
    [InlineData("table-borders.fodt")]
    [InlineData("table-borders.odt")]
    [InlineData("table-borders.docx")]
    [InlineData("table-borders.doc")]
    [InlineData("table-borders.rtf")]
    public void EveryBorderIsStrokedWhereLibreOfficeStrokesIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfStroke> ours = PdfStrokes.Read(Ours(path));
        List<PdfStroke> theirs = PdfStrokes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        theirs.Count.ShouldBeGreaterThan(0, $"{fileName}: the reference strokes nothing, so this proves nothing");
        ours.Count.ShouldBe(theirs.Count, $"{fileName}: number of strokes");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfStroke mine = ours[i];
            PdfStroke reference = theirs[i];
            string where = $"{fileName}: stroke {i + 1} on page {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: page");
            mine.IsHorizontal.ShouldBe(reference.IsHorizontal, $"{where}: direction");

            // The pen width, which for a border is half of what is being checked: a line in the
            // right place at the wrong weight is a different border.
            Math.Abs(mine.Width - reference.Width).ShouldBeLessThanOrEqualTo(
                TolerancePoints, $"{where}: {mine.Width:F3} pt pen drawn, {reference.Width:F3} pt rendered");

            Close(mine.FromX, reference.FromX, $"{where}: start x");
            Close(mine.ToX, reference.ToX, $"{where}: end x");
            Close(mine.FromY, reference.FromY, $"{where}: start y");
            Close(mine.ToY, reference.ToY, $"{where}: end y");
        }
    }

    [Theory]
    [InlineData("footnotes.fodt")]
    [InlineData("footnotes.odt")]
    [InlineData("footnotes.docx")]
    [InlineData("footnotes.doc")]
    // footnotes.rtf is left out, and what it is left out for is a layout finding rather than a
    // drawing one: its note separator is drawn 757.465 pt down the page where LibreOffice puts it
    // at 758.751, a difference of 1.286 pt — about 26 twips — in the height of the note area the
    // RTF reader reserves. The other four formats agree with LibreOffice to a hundredth of a
    // point on the same document, so this is the RTF reader's alone. Recorded in
    // src/Paperless.Rendering/TODO.md; not fixed here because the layout is another library's.
    [InlineData("table-shading.fodt")]
    [InlineData("table-shading.odt")]
    [InlineData("table-shading.docx")]
    [InlineData("table-shading.rtf")]
    public void EveryShadeAndRuleIsFilledWhereLibreOfficeFillsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);

        // Distinct rectangles on both sides, because a reference can paint one twice and does:
        // LibreOffice's render of table-shading fills each shaded cell exactly two times over, at
        // identical coordinates. What is being compared is which rectangles were filled, not how
        // many times each was visited.
        List<PdfFill> ours = [.. PdfFills.Read(Ours(path)).Distinct()];
        List<PdfFill> theirs = [.. PdfFills.Read(_libreOffice.ConvertToPdf(path, _workDirectory)).Distinct()];

        theirs.Count.ShouldBeGreaterThan(0, $"{fileName}: the reference fills nothing, so this proves nothing");
        ours.Count.ShouldBe(theirs.Count, $"{fileName}: number of filled rectangles");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfFill mine = ours[i];
            PdfFill reference = theirs[i];
            string where = $"{fileName}: fill {i + 1} on page {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: page");
            Close(mine.Left, reference.Left, $"{where}: left");
            Close(mine.Top, reference.Top, $"{where}: top");
            Close(mine.Width, reference.Width, $"{where}: width");
            Close(mine.Height, reference.Height, $"{where}: height");
        }
    }

    [Theory]
    [InlineData("page-geometry.fodt")]
    [InlineData("page-geometry.odt")]
    [InlineData("page-geometry.docx")]
    [InlineData("page-geometry.doc")]
    [InlineData("page-geometry.rtf")]
    public void EveryPageTakesTheSheetSizeLibreOfficeGivesIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfPageSize> ours = PdfPageSizes.Read(Ours(path));
        List<PdfPageSize> theirs = PdfPageSizes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // The cheapest check there is that a section's geometry reached the file: a document that
        // carried the first section's page setup throughout gets three portrait pages where the
        // reference gives two portrait and a landscape, and no comparison of positions says so as
        // plainly.
        ours.Count.ShouldBe(theirs.Count, $"{fileName}: page count");

        for (int i = 0; i < theirs.Count; i++)
        {
            Close(ours[i].Width, theirs[i].Width, $"{fileName}: page {i + 1} width");
            Close(ours[i].Height, theirs[i].Height, $"{fileName}: page {i + 1} height");
        }
    }

    [Theory]
    [InlineData("prose-odt.odt")]
    [InlineData("prose-docx.docx")]
    [InlineData("paginated.fodt")]
    [InlineData("tabbed.odt")]
    public void TheTextInOurPdfCanBeFoundAndCopied(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        string? ours = ExtractedText(Ours(path));
        string? theirs = ExtractedText(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(ours is null || theirs is null, "pdftotext is not available; install poppler-utils");

        // The whole reason the display list carries glyph ids and a cluster map rather than
        // outlines. A PDF of outlines extracts as nothing at all, and one whose ToUnicode is wrong
        // extracts as mojibake — neither of which any comparison of positions would notice.
        ours.ShouldNotBeNullOrWhiteSpace();
        Paperless.TestKit.Comparison.TextComparer.Normalise(ours!, foldSpaces: true)
            .ShouldBe(
                Paperless.TestKit.Comparison.TextComparer.Normalise(theirs!, foldSpaces: true),
                $"{fileName}: the text a reader gets out of our PDF differs from LibreOffice's");
    }

    [Fact]
    public void APdfWrittenTwiceIsTheSameFile()
    {
        string path = Corpus.Require("paginated.fodt");

        // Byte determinism is what makes a checksum meaningful, and the only varying field in a
        // PDF that is not content is the creation date — which is why the option exists.
        byte[] first = File.ReadAllBytes(Ours(path));
        byte[] second = File.ReadAllBytes(Ours(path, "second"));

        second.ShouldBe(first);
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>Renders a document to a PDF of our own, and returns its path.</summary>
    private string Ours(string documentPath, string suffix = "paperless")
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-{suffix}.pdf");

        using IDocument document = PaperlessDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);

        // A fixed creation date, so two runs of the same document produce the same bytes.
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }

    /// <summary>
    /// One entry per drawn line: the first run on each baseline, carrying the whole line's glyphs.
    /// </summary>
    /// <remarks>
    /// Runs arrive in the order they were drawn, so a line is a maximal stretch sharing a page and
    /// a baseline. Half a point of slack on the baseline, which is well under any line pitch in
    /// the corpus and well over the 0.051 pt the two writers differ by.
    /// </remarks>
    private static List<PdfTextRun> LinesOf(List<PdfTextRun> runs)
    {
        List<PdfTextRun> lines = [];

        foreach (PdfTextRun run in runs)
        {
            if (lines.Count > 0
                && lines[^1].PageIndex == run.PageIndex
                && Math.Abs(lines[^1].Y - run.Y) <= 0.5)
            {
                lines[^1] = lines[^1] with { GlyphCount = lines[^1].GlyphCount + run.GlyphCount };
                continue;
            }

            lines.Add(run);
        }

        return lines;
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

    /// <summary>
    /// Asserts two measurements agree to a tenth of a point.
    /// </summary>
    /// <remarks>
    /// The bound is inclusive, and the epsilon is what makes it so. A shaded row comes out
    /// 18.950 pt tall against LibreOffice's 18.850 — exactly two twips, exactly the bound — and
    /// the subtraction of those two doubles is 0.10000000000002274. Without the epsilon the
    /// comparison would reject a difference of precisely the size it is meant to allow, which is
    /// a statement about binary floating point and not about the rendering.
    /// </remarks>
    private static void Close(double actual, double expected, string what)
        => Math.Abs(actual - expected).ShouldBeLessThanOrEqualTo(
            TolerancePoints + 1e-9, $"{what}: {actual:F3} pt drawn, {expected:F3} pt rendered");
}

using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares the cell text Paperless draws with the cell text LibreOffice draws, run for run.
/// </summary>
/// <remarks>
/// <para>
/// Both sides are read with <see cref="PdfTextRuns"/> rather than with <c>PdfWords</c>, because a
/// word box is not a baseline: its top sits above the baseline by an ascent the PDF never states,
/// so a vertical comparison against boxes can only ever be relative. Cell text is exactly where
/// that matters — a cell's baseline is the row's bottom less the descent, and getting the
/// ascent/descent split wrong moves every baseline while leaving every box in place.
/// </para>
/// <para>
/// <strong>The corpus document has no header or footer, deliberately.</strong> Calc recomputes a
/// dynamic band's height from the text in it and floors the result at the declared height
/// (<c>UpdateHFHeight</c>, <c>sc/source/ui/view/printfun.cxx:846</c>), which Paperless does not do
/// yet; on a workbook with a header that is worth 1.3 pt of vertical offset on every row of every
/// page, and it would swamp everything this file is about. Switching the bands off in the document
/// is what makes the comparison a comparison of cell text.
/// </para>
/// <para>
/// <strong>Two differences are LibreOffice's own arithmetic and are reproduced rather than
/// tolerated.</strong> Its output device works in hundredths of a millimetre, and it reaches them
/// through Calc's whole twips: a position truncates and a font height rounds, so ten-point text is
/// emitted at 10.0063 pt and a 12.8 pt row is 12.7843 pt tall. The first is invisible; the second
/// accumulates to 0.86 pt by the sixty-seventh row of a page, which is why
/// <c>SheetDeviceUnits</c> exists.
/// </para>
/// <para>
/// <strong>The XLS is compared for what is drawn, not for where.</strong> LibreOffice's own BIFF
/// import gives the same workbook a slightly different <em>geometry</em> from its ODF and
/// SpreadsheetML forms: the page's left margin lands 21 twips further right — its ODS render
/// starts the first column's text at 57.685 pt and its XLS render at 58.677 pt — and its columns
/// come out 0.06% narrower, 230.17 pt across the first four against 230.40. Both are LibreOffice
/// reading the file rather than Paperless drawing it, and neither changes the page count. The
/// second has a knock-on worth naming: a shrink-to-fit scale is an integer percentage of the
/// available width, so a column a fraction narrower shrinks to 85% where the other two formats
/// shrink to 87%, and a clipped string keeps one character fewer. The XLS therefore takes the run
/// count and the em sizes, which do not depend on a column being the width we think it is, and the
/// positional and glyph-count assertions cover the two formats that agree about the geometry.
/// </para>
/// </remarks>
public sealed class SheetTextComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, two twips, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-sheet-text").FullName;

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
    [InlineData("sheet-cell-text.fods")]
    [InlineData("sheet-cell-text.xlsx")]
    public void EveryCellIsDrawnWhereLibreOfficeDrawsIt(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        List<PdfTextRun> ours = Upright(PdfTextRuns.Read(Ours(path)));
        List<PdfTextRun> theirs = Upright(PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, $"{name}: number of drawn text runs");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfTextRun mine = ours[i];
            PdfTextRun reference = theirs[i];
            string where = $"{name}: run {i + 1} on page {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: page");
            mine.X.ShouldBe(reference.X, TolerancePoints, $"{where}: pen across");
            mine.Y.ShouldBe(reference.Y, TolerancePoints, $"{where}: baseline");
            mine.FontSize.ShouldBe(reference.FontSize, 0.005, $"{where}: font size");

            // The glyph count is what catches the two rules that change the *text*: a shrunk
            // string keeps all of its characters and a hashed or clipped one does not.
            mine.GlyphCount.ShouldBe(reference.GlyphCount, $"{where}: glyph count");
            mine.Colour.ShouldBe(reference.Colour, $"{where}: colour");
        }
    }

    [Theory]
    [InlineData("sheet-cell-text.fods")]
    [InlineData("sheet-cell-text.xlsx")]
    public void EveryCellShowsTheCharactersLibreOfficeShows(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        List<PdfTextRun> ours = Upright(PdfTextRuns.Read(Ours(path)));
        List<PdfTextRun> theirs = Upright(PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // What each run holds and how big it is, without where it sits — which is the whole of
        // the comparison the XLS can take, and a strictly weaker claim than the one above.
        string[] mine = [.. ours.Select(run => $"{run.PageIndex}:{run.GlyphCount}:{run.FontSize:0.00}")];
        string[] reference =
            [.. theirs.Select(run => $"{run.PageIndex}:{run.GlyphCount}:{run.FontSize:0.00}")];

        mine.ShouldBe(reference, $"{name}: the runs drawn, as page, glyph count and size");
    }

    [Theory]
    [InlineData("sheet-cell-text.fods")]
    [InlineData("sheet-cell-text.xlsx")]
    [InlineData("sheet-cell-text.xls")]
    public void EveryFormatDrawsTheSameRunsInTheSameFaces(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        List<PdfTextRun> ours = Upright(PdfTextRuns.Read(Ours(path)));
        List<PdfTextRun> theirs = Upright(PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // The weakest claim of the three, and the only one the XLS can make. The run count is
        // what catches a wrap flag that was not read — the wrapped cell is three runs or one —
        // and the sizes are what catch a font table that was not read at all, since the document
        // sets four of them. Neither depends on a column being the width we think it is, which is
        // what LibreOffice's own BIFF reading disagrees with us about.
        ours.Count.ShouldBe(theirs.Count, $"{name}: number of drawn text runs");

        int[] mine = [.. ours.Select(run => (int)Math.Round(run.FontSize)).Order()];
        int[] reference = [.. theirs.Select(run => (int)Math.Round(run.FontSize)).Order()];

        mine.ShouldBe(reference, $"{name}: the em sizes drawn, to the nearest point");
    }

    [Theory]
    [InlineData("sheet-cell-text.fods")]
    [InlineData("sheet-cell-text.xlsx")]
    [InlineData("sheet-cell-text.xls")]
    public void ANarrowNumericColumnShortensItsNumberOrHashesIt(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        List<PdfWord> words = PdfWords.Read(Ours(path));

        Assert.SkipWhen(words.Count == 0, "pdftotext is not available; install poppler-utils");

        string text = string.Join(' ', words.Select(word => word.Text));

        // Row four holds 123 456 789 012 twice: once under General, which re-renders itself with
        // as many characters as the column has digit widths and falls back to scientific notation
        // from there, and once under a fixed format, which cannot shorten and shows hashes
        // instead. The asymmetry is the rule — a port that hashes every number that does not fit
        // produces plausible output and disagrees with the reference on every wide number.
        text.ShouldContain("1.2E+11", customMessage: $"{name}: General shortened to fit");
        text.ShouldContain("###", customMessage: $"{name}: a fixed format hashed");
        text.ShouldNotContain("123456789012", customMessage: $"{name}: the full number is not drawn");
    }

    [Theory]
    [InlineData("sheet-cell-text.fods")]
    [InlineData("sheet-cell-text.xlsx")]
    public void ALongStringOverflowsIntoEmptyCellsAndIsCutOffByOccupiedOnes(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        List<PdfTextRun> ours = Upright(PdfTextRuns.Read(Ours(path)));
        List<PdfTextRun> theirs = Upright(PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // Rows five and six differ only in whether the cell to the right holds anything. The
        // 48-character string of row five keeps all of its glyphs because it has four empty
        // columns to run through; the 31-character one of row six is cut short at its neighbour,
        // and LibreOffice shortens the string it draws rather than relying on the clip alone —
        // so 31 glyphs appear in neither PDF.
        ours.Count(run => run.GlyphCount == 48)
            .ShouldBe(theirs.Count(run => run.GlyphCount == 48), $"{name}: unobstructed overflow");

        ours.ShouldContain(run => run.GlyphCount == 48, $"{name}: the overflow is drawn in full");
        ours.ShouldNotContain(
            run => run.GlyphCount == 31, $"{name}: the blocked string is shortened");
        theirs.ShouldNotContain(
            run => run.GlyphCount == 31, $"{name}: LibreOffice shortens it too");
    }

    /// <summary>
    /// The runs drawn with an absolute pen, in reading order rather than in drawing order.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two adjustments, and both are about comparing like with like rather than about leniency.
    /// </para>
    /// <para>
    /// Rotated cells live on the corpus document's second sheet and are dropped here, because the
    /// two renderers express a turned run differently — LibreOffice writes a text matrix and
    /// Paperless a transform around an ordinary pen — so <see cref="PdfTextRuns"/> sees one and
    /// not the other. That the rotation was <em>read</em> is asserted in
    /// <c>Paperless.Spreadsheets.Tests</c>, where it needs no rendering at all.
    /// </para>
    /// <para>
    /// And the runs are sorted, because drawing order is not a property worth asserting: Calc has
    /// two text paths and runs them in turn, so its wrapped and shrunk cells are emitted after
    /// every plain string on the page (<c>DrawStrings</c> then <c>DrawEdit</c>,
    /// <c>sc/source/ui/view/output2.cxx</c>). Sorting by baseline and then by pen puts both sides
    /// in the order a reader would see them. The sort is on the raw values rather than on rounded
    /// ones: the closest two baselines in the document are 0.28 pt apart and the two renderers
    /// agree on each to within 0.05 pt, so no pair can cross.
    /// </para>
    /// </remarks>
    private static List<PdfTextRun> Upright(List<PdfTextRun> runs)
        => [.. runs.Where(run => run.PageIndex == 0)
                   .OrderBy(run => run.PageIndex)
                   .ThenBy(run => run.Y)
                   .ThenBy(run => run.X)];

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

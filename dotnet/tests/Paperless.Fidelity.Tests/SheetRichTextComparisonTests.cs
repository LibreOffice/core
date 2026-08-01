using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares a cell whose text changes format part-way through, portion for portion.
/// </summary>
/// <remarks>
/// <para>
/// The claim this file makes that <c>SheetTextComparisonTests</c> cannot is that a cell is
/// <em>several</em> runs. A rich cell emits one glyph run per portion in both renderers, so the
/// run count alone catches a reader that flattened the formatting away — the corpus document's
/// first sheet is eight cells and twenty-one runs — and the pen of each portion after the first is
/// the sum of the widths before it, which is what catches a portion measured in the wrong face.
/// </para>
/// <para>
/// <strong>The rules being checked are not the same as the plain path's.</strong> A rich cell's
/// line is as tall as its tallest portion rather than as the cell's own font, its colour is the
/// portion's rather than the cell's, and a wrapping one breaks against the runs it is made of — a
/// bold word is wider than the same characters set regular, so measuring the line in the cell's
/// face alone puts the break in the wrong place. The corpus document exercises each in turn.
/// </para>
/// <para>
/// <strong>The three formats state a run's format three different ways</strong>, which is what
/// makes drawing the same three cells the same a claim worth testing: ODF's <c>text:span</c> is a
/// delta over the cell's own text properties, SpreadsheetML's <c>rPr</c> is a complete font over
/// the workbook's default, and BIFF's formatting run names a whole <c>FONT</c> record.
/// </para>
/// <para>
/// <strong>The XLS is compared for less.</strong> LibreOffice's own BIFF import gives the workbook
/// a page margin 21 twips further right than its ODF and SpreadsheetML forms — measured on
/// <c>sheet-cell-text</c> and recorded there — so the XLS takes the run count and the em sizes,
/// which do not depend on a column being the width we think it is.
/// </para>
/// </remarks>
public sealed class SheetRichTextComparisonTests : IDisposable
{
    /// <summary>
    /// A quarter of a point, and it is not the project's usual tenth for a measured reason.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A rich cell goes through EditEngine in Calc and through the same shaper as any other cell
    /// here, and the two measure a <em>portion</em> differently. LibreOffice's portion widths are
    /// always a whole hundredth of a millimetre — <c>One </c> in ten-point Liberation Sans comes
    /// out at exactly 762 of them, 21.600 pt, against the 765.29 the font's own advances give —
    /// so a portion's pen drifts, and the drift accumulates across a cell: 0.09 pt after one
    /// portion and 0.21 pt after four. The plain path has no such difference and still agrees to
    /// 0.006 pt.
    /// </para>
    /// <para>
    /// Four models of that quantisation were measured against ten portions of this document and
    /// none reproduces it: per-character truncation to whole hundredths is the closest at 0.057 pt
    /// against our 0.09, per-character truncation to whole twips reproduces <c>One </c> exactly and
    /// nothing else, and rounding in either unit is worse than truncating. One portion comes out
    /// <em>wider</em> in LibreOffice than the font's advances allow, which no rounding rule
    /// explains and hinting would. So the drift is recorded rather than reproduced, and the bound
    /// is set where it lies.
    /// </para>
    /// </remarks>
    private const double TolerancePoints = 0.25;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-sheet-rich").FullName;

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
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    public void EveryPortionIsDrawnWhereLibreOfficeDrawsIt(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        List<PdfTextRun> ours = Upright(PdfTextRuns.Read(Ours(path)));
        List<PdfTextRun> theirs = Upright(PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, $"{name}: number of drawn portions");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfTextRun mine = ours[i];
            PdfTextRun reference = theirs[i];
            string where = $"{name}: portion {i + 1} on page {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: page");
            mine.X.ShouldBe(reference.X, TolerancePoints, $"{where}: pen across");
            mine.Y.ShouldBe(reference.Y, TolerancePoints, $"{where}: baseline");
            mine.FontSize.ShouldBe(reference.FontSize, 0.005, $"{where}: font size");
            mine.GlyphCount.ShouldBe(reference.GlyphCount, $"{where}: glyph count");

            // The colour is the portion's own, and it is the assertion that catches a run read
            // against the wrong base: the blue cell's bold half is blue in both renderers because
            // its rPr restates the colour, and the bold-first cell's tail is black in both
            // although its cell's own font is bold.
            mine.Colour.ShouldBe(reference.Colour, $"{where}: colour");
        }
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    [InlineData("sheet-rich-text.xls")]
    public void EveryFormatSplitsTheSameCellsIntoTheSameRuns(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        List<PdfTextRun> ours = Upright(PdfTextRuns.Read(Ours(path)));
        List<PdfTextRun> theirs = Upright(PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // The count is the whole of the claim a flattened reader fails: eight rich cells drawn as
        // one run each is eight runs, and drawn as portions is twenty-one.
        ours.Count.ShouldBe(theirs.Count, $"{name}: number of drawn portions");

        int[] mine = [.. ours.Select(run => (int)Math.Round(run.FontSize)).Order()];
        int[] reference = [.. theirs.Select(run => (int)Math.Round(run.FontSize)).Order()];

        mine.ShouldBe(reference, $"{name}: the em sizes drawn, to the nearest point");
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    [InlineData("sheet-rich-text.xls")]
    public void ARichCellIsMoreThanOneRun(string name)
    {
        string path = Corpus.Require(name);
        List<PdfTextRun> ours = Upright(PdfTextRuns.Read(Ours(path)));

        Assert.SkipWhen(ours.Count == 0, "pdftotext is not available; install poppler-utils");

        // Needs no LibreOffice: eight cells that hold two or more formats cannot be drawn in
        // fewer than sixteen runs however the geometry lands, and a reader that read past the
        // formatting draws exactly eight.
        ours.Count.ShouldBeGreaterThan(
            16, $"{name}: a cell of several formats is drawn as several runs");

        // Three em sizes appear on the sheet — 7, 10 and 14 — and two of them exist only inside a
        // cell rather than on one, so a flattened reading has one.
        int[] sizes = [.. ours.Select(run => (int)Math.Round(run.FontSize)).Distinct().Order()];
        sizes.ShouldBe([7, 10, 14], $"{name}: the em sizes a run may state");
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    public void AWrappingRichCellBreaksWhereLibreOfficeBreaksIt(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        List<PdfTextRun> ours = Upright(PdfTextRuns.Read(Ours(path)));
        List<PdfTextRun> theirs = Upright(PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // The wrapping cell mixes 10 pt, bold 10 pt and 14 pt, so where its line falls depends on
        // measuring each portion in its own face; breaking it against the cell's font alone moves
        // the break by a word, which moves a portion onto another line and changes how many
        // baselines the sheet has. Counting baselines rather than matching a string is what makes
        // that assertion independent of where a portion sits to a fraction of a point.
        int mine = ours.Select(run => Math.Round(run.Y, 0)).Distinct().Count();
        int reference = theirs.Select(run => Math.Round(run.Y, 0)).Distinct().Count();

        mine.ShouldBe(reference, $"{name}: how many lines of text the sheet holds");

        // And the glyph counts per line, which say where each break fell rather than only that
        // there was one.
        int[] mineCounts = [.. ours.Select(run => run.GlyphCount)];
        int[] referenceCounts = [.. theirs.Select(run => run.GlyphCount)];
        mineCounts.ShouldBe(referenceCounts, $"{name}: the characters on each line");
    }

    /// <summary>
    /// The runs drawn with an absolute pen on the rich-text sheet, in reading order.
    /// </summary>
    /// <remarks>
    /// The first page only, for the same reason <c>SheetTextComparisonTests</c> takes it: the
    /// document's later sheets hold turned text and a picture, and a turned run is written with a
    /// text matrix that <see cref="PdfTextRuns"/> reads from one renderer and not the other.
    /// Sorting by baseline and then by pen puts both sides in the order a reader would see them,
    /// which drawing order does not — Calc runs its plain and its EditEngine text paths in turn.
    /// </remarks>
    private static List<PdfTextRun> Upright(List<PdfTextRun> runs)
        => [.. runs.Where(run => run.PageIndex == 0)
                   .OrderBy(run => run.Y)
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

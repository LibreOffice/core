using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.Spreadsheets;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares everything a printed sheet draws that is not a cell's text with what LibreOffice
/// draws: its fills, its borders, its grid, its headings and its header and footer.
/// </summary>
/// <remarks>
/// <para>
/// Operator for operator rather than pixel for pixel, which is what makes the comparison worth
/// having. A background is a filled rectangle and a border is a stroked line, and LibreOffice's
/// own PDF export writes both explicitly — so the count, the direction, the pen width, the
/// endpoints and the colour are all directly readable and all directly comparable. A raster
/// comparison of the same page would report one number and say nothing about which of the five
/// was wrong.
/// </para>
/// <para>
/// The tolerances are not slack: LibreOffice's whole layer works in 1/100 mm, so a row 12.813 pt
/// tall is stored as 451 units and comes back out as 12.784 — a systematic 0.03 pt a row that
/// accumulates down the page and cannot be removed without routing every sheet measurement
/// through the same grid. Over the seven rows of <c>sheet-decor-ods.ods</c> that is 0.38 pt, so
/// positions are compared to half a point and widths, which suffer no such drift, to a fiftieth.
/// </para>
/// </remarks>
public sealed class SheetDecorationComparisonTests : IDisposable
{
    /// <summary>How far a coordinate may differ before it is a disagreement rather than rounding.</summary>
    private const double PositionTolerance = 0.5;

    /// <summary>
    /// How far the whole printed block may sit from where LibreOffice puts it vertically.
    /// </summary>
    /// <remarks>
    /// Separate from <see cref="PositionTolerance"/> and applied once per page rather than per
    /// stroke, because there is one known cause and it moves everything by the same amount: the
    /// height of a header band whose file asks for it to be computed from the header's own text.
    /// Calc measures that text with its own EditEngine while the OOXML filter measures it again
    /// while importing, and the two disagree — measured on <c>sheet-decor-xlsx.xlsx</c>,
    /// LibreOffice's band is 18.13 pt against the 17.10 pt the file's own margins give, so every
    /// line on the page sits 1.03 pt lower than ours. Reproducing it needs the header font's
    /// metrics inside the reader; it is recorded in the module's TODO. Everything <em>within</em>
    /// the page is still compared to half a point.
    /// </remarks>
    private const double BlockOffsetTolerance = 1.5;

    /// <summary>How far a pen width may differ. Widths do not accumulate, so this is tight.</summary>
    private const double WidthTolerance = 0.02;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-sheet-decor").FullName;

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
    [InlineData("sheet-decor-ods.ods")]
    [InlineData("sheet-decor-xlsx.xlsx")]
    [InlineData("sheet-decor-xls.xls")]
    public void ACellsBackgroundIsPaintedWhereLibreOfficePaintsIt(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        (List<PdfFill> reference, List<PdfFill> actual) = Fills(name);
        Assert.SkipWhen(reference.Count == 0, "the reference PDF holds no fills");

        actual.Count.ShouldBe(
            reference.Count,
            $"{name}: {actual.Count} fills against LibreOffice's {reference.Count}");

        double offset = VerticalOffset(name, reference[0].Top, actual[0].Top);

        // In drawing order, which both sides walk row by row and then column by column, so a
        // fill landing on the wrong cell shows up as a mismatch rather than being absorbed by a
        // set comparison.
        for (int at = 0; at < reference.Count; at++)
        {
            PdfFill expected = reference[at];
            PdfFill found = actual[at];

            found.Colour.ShouldBe(
                expected.Colour,
                $"{name}: fill {at} is #{found.Colour:X6}, LibreOffice's is #{expected.Colour:X6}");

            found.Left.ShouldBe(expected.Left, PositionTolerance, $"{name}: fill {at} left");
            found.Top.ShouldBe(
                expected.Top + offset, PositionTolerance, $"{name}: fill {at} top");
            found.Width.ShouldBe(expected.Width, PositionTolerance, $"{name}: fill {at} width");
            found.Height.ShouldBe(expected.Height, PositionTolerance, $"{name}: fill {at} height");
        }
    }

    [Theory]
    [InlineData("sheet-decor-ods.ods")]
    [InlineData("sheet-decor-xlsx.xlsx")]
    [InlineData("sheet-decor-xls.xls")]
    public void AShareEdgeGoesToTheHeavierOfTheTwoCellsBorders(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        (List<PdfRule> reference, List<PdfRule> actual) = Rules(name);
        Assert.SkipWhen(reference.Count == 0, "the reference PDF holds no strokes");

        // A border is anything drawn thicker than the hairline the grid and the headings use.
        List<PdfRule> expected = [.. reference.Where(rule => rule.Width > 0.2)];
        List<PdfRule> found = [.. actual.Where(rule => rule.Width > 0.2)];

        found.Count.ShouldBe(
            expected.Count,
            $"{name}: {found.Count} borders against LibreOffice's {expected.Count}"
            + $"\n  LibreOffice: {Describe(expected)}\n  Paperless:   {Describe(found)}");

        double offset = VerticalOffset(name, expected[0].FromY, found[0].FromY);

        for (int at = 0; at < expected.Count; at++)
        {
            PdfRule want = expected[at];
            PdfRule got = found[at];

            // The colour is the whole assertion about the conflict: the corpus states each
            // shared edge as a thick coloured border on one cell and a hairline black one on
            // the other, so drawing the loser is visible as black where red was expected.
            got.Colour.ShouldBe(
                want.Colour,
                $"{name}: border {at} is #{got.Colour:X6}, LibreOffice's is #{want.Colour:X6}");

            got.Width.ShouldBe(want.Width, WidthTolerance, $"{name}: border {at} pen width");
            got.IsHorizontal.ShouldBe(want.IsHorizontal, $"{name}: border {at} direction");
            got.FromX.ShouldBe(want.FromX, PositionTolerance, $"{name}: border {at} start x");
            got.FromY.ShouldBe(
                want.FromY + offset, PositionTolerance, $"{name}: border {at} start y");
            got.ToX.ShouldBe(want.ToX, PositionTolerance, $"{name}: border {at} end x");
            got.ToY.ShouldBe(
                want.ToY + offset, PositionTolerance, $"{name}: border {at} end y");
        }
    }

    [Theory]
    [InlineData("sheet-decor-ods.ods")]
    [InlineData("sheet-decor-xlsx.xlsx")]
    [InlineData("sheet-decor-xls.xls")]
    public void ThePrintedGridHasALineAtTheFarEdgeOfEveryColumnAndRow(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        (List<PdfRule> reference, List<PdfRule> actual) = Rules(name);
        Assert.SkipWhen(reference.Count == 0, "the reference PDF holds no strokes");

        // Everything at the hairline width: the grid, the heading boxes and the frame. They are
        // compared as one set because they are drawn with one pen and the interesting property
        // is where they fall, not which of the three produced them.
        List<PdfRule> expected = [.. reference.Where(rule => rule.Width <= 0.2)];
        List<PdfRule> found = [.. actual.Where(rule => rule.Width <= 0.2)];

        found.Count.ShouldBe(
            expected.Count,
            $"{name}: {found.Count} hairlines against LibreOffice's {expected.Count}");

        double offset = VerticalOffset(name, expected[0].FromY, found[0].FromY);

        for (int at = 0; at < expected.Count; at++)
        {
            PdfRule want = expected[at];
            PdfRule got = found[at];

            // Black, not the pale grey the grid has on screen: ScPrintFunc::PrintPage starts
            // from COL_BLACK and only takes the screen's colours when told to.
            got.Colour.ShouldBe(0u, $"{name}: hairline {at} is not black");

            got.IsHorizontal.ShouldBe(want.IsHorizontal, $"{name}: hairline {at} direction");
            got.FromX.ShouldBe(want.FromX, PositionTolerance, $"{name}: hairline {at} start x");
            got.FromY.ShouldBe(
                want.FromY + offset, PositionTolerance, $"{name}: hairline {at} start y");
            got.ToX.ShouldBe(want.ToX, PositionTolerance, $"{name}: hairline {at} end x");
            got.ToY.ShouldBe(
                want.ToY + offset, PositionTolerance, $"{name}: hairline {at} end y");
        }
    }

    [Theory]
    [InlineData("sheet-decor-ods.ods", "Decor", "Decoration", "Page1of1")]
    [InlineData("sheet-decor-xlsx.xlsx", "Decor", "Decoration", "Page1of1")]
    [InlineData("sheet-decor-xls.xls", "Decor", "Decoration", "Page1of1")]
    public void TheHeaderExpandsItsFieldsIntoThreeSeparatelyAlignedParts(
        string name, string left, string centre, string right)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        string ours = Render(path);
        List<PdfTextRun> reference =
            InBand(PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));
        List<PdfTextRun> actual = InBand(PdfTextRuns.Read(ours));

        Assert.SkipWhen(reference.Count == 0, "the reference PDF holds no header or footer");

        // Three parts in the header and two in the footer, drawn as one run each. LibreOffice
        // splits its own into more — the header's right part arrives as four, one per field and
        // one per literal — so the counts are not comparable and the positions are: each of ours
        // has to start where one of LibreOffice's parts starts.
        actual.Count.ShouldBe(5, $"{name}: the header and footer drew {actual.Count} runs, not 5");

        foreach (PdfTextRun run in actual)
        {
            reference.ShouldContain(
                other => Math.Abs(other.X - run.X) < 1.0
                         && Math.Abs(other.Y - run.Y) <= BlockOffsetTolerance,
                $"{name}: a band run at {run.X:0.00},{run.Y:0.00} matches nothing LibreOffice drew"
                + $"\n  LibreOffice: {string.Join(", ", reference.Select(r => $"{r.X:0.00},{r.Y:0.00}"))}");
        }

        // And what they say. None of these strings is anywhere in the file: they are &A, a
        // literal, and "Page &P of &N". A reader that copied the codes through or printed a
        // field's cached value produces something else for every one of them.
        List<string> lines = Lines(ours);

        lines.ShouldContain(
            line => line.Contains(left, StringComparison.Ordinal),
            $"{name}: the header's &A did not expand to \"{left}\"");
        lines.ShouldContain(
            line => line.Contains(centre, StringComparison.Ordinal),
            $"{name}: the header's literal centre part is missing");
        lines.ShouldContain(
            line => line.Contains(right, StringComparison.Ordinal),
            $"{name}: the header's &P and &N did not expand to \"{right}\"");
        lines.ShouldContain(
            line => line.Contains(Path.GetFileName(path), StringComparison.Ordinal),
            $"{name}: the footer's &F did not expand to the file name");
    }

    /// <summary>The text runs that fall in a page's header or footer band.</summary>
    /// <remarks>
    /// By position rather than by any marker in the PDF, because there is none: the bands are
    /// simply the strips above and below the printed block, and on all three corpus files the
    /// cells run from 74 pt to 180 pt down a 842 pt page.
    /// </remarks>
    private static List<PdfTextRun> InBand(IEnumerable<PdfTextRun> runs)
        => [.. runs.Where(run => run.Y < 74 || run.Y > 700)];

    /// <summary>
    /// Each line of a PDF as one string, with every space between its words removed.
    /// </summary>
    /// <remarks>
    /// Removed because a glyph run written with per-glyph kerning adjustments — which is what
    /// Paperless's PDF writer produces from a shaped run — makes <c>pdftotext</c> report every
    /// letter as its own word with a <em>zero-width</em> box, so neither the gaps nor the widths
    /// can be used to put them back together. The trap cost an hour: the header was placed
    /// correctly and matched LibreOffice's own PDF to two decimal places, and a word-by-word
    /// assertion still failed on "Decor" being five words. The artefact is in the reader, not on
    /// the page, and joining a whole line is the honest way round it — what is being asserted is
    /// that the field expanded at all, and where it sits is asserted from the run positions.
    /// </remarks>
    private static List<string> Lines(string pdfPath)
        => [.. PdfWords.Read(pdfPath)
            .GroupBy(word => (word.PageIndex, Math.Round(word.Top, 1)))
            .Select(line => string.Concat(line.OrderBy(word => word.Left).Select(word => word.Text)))];

    /// <summary>
    /// How far the whole page sits below where LibreOffice puts it, bounded and reported once.
    /// </summary>
    private static double VerticalOffset(string name, double reference, double actual)
    {
        double offset = actual - reference;

        Math.Abs(offset).ShouldBeLessThanOrEqualTo(
            BlockOffsetTolerance,
            $"{name}: the printed block sits {offset:0.00} pt from LibreOffice's, which is more "
            + "than the header band's known disagreement");

        return offset;
    }

    private (List<PdfFill> Reference, List<PdfFill> Actual) Fills(string name)
    {
        string path = Corpus.Require(name);
        return (PdfFills.Read(_libreOffice.ConvertToPdf(path, _workDirectory)),
                PdfFills.Read(Render(path)));
    }

    private (List<PdfRule> Reference, List<PdfRule> Actual) Rules(string name)
    {
        string path = Corpus.Require(name);
        return (PdfRules.Read(_libreOffice.ConvertToPdf(path, _workDirectory)),
                PdfRules.Read(Render(path)));
    }

    private static string Describe(IEnumerable<PdfRule> rules)
        => string.Join(", ", rules.Select(
            rule => $"{(rule.IsHorizontal ? "h" : "v")}@{rule.FromX:0.0},{rule.FromY:0.0}"
                    + $" {rule.Length:0.0}pt w{rule.Width:0.00} #{rule.Colour:X6}"));

    /// <summary>Lays a workbook out and writes it as a PDF beside the reference.</summary>
    private string Render(string path)
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)new SpreadsheetReader().Read(DocumentSource.FromFile(path));

        string output = Path.Combine(
            _workDirectory, Path.GetFileNameWithoutExtension(path) + "-paperless.pdf");

        using FileStream stream = File.Create(output);
        new PdfRenderer().Render(document.Layout(), stream);
        return output;
    }
}

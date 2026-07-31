using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that a page's header and footer land where LibreOffice puts them.
/// </summary>
/// <remarks>
/// <para>
/// Which is two questions, and the second is the one that catches more. A header has to be drawn in its own
/// strip at the top of the page — that is the obvious half. But a header also <em>moves the body</em>: ODF
/// states <c>fo:margin-top</c> as the distance to the top of the <em>header</em>, so a document that grows
/// one pushes its body text down by the header's height and the spacing below it. Get that wrong and every
/// line on every page is out, which is why this compares the whole page's text rather than the strip's.
/// </para>
/// <para>
/// The comparison is absolute rather than differential: a header's position is stated by the document
/// geometry, so nothing accumulates across a line the way justification's stretch does.
/// </para>
/// </remarks>
public sealed class FurnitureComparisonTests : IDisposable
{
    /// <summary>How far a drawn word may differ from LibreOffice's, in points.</summary>
    /// <remarks>
    /// Two twips horizontally. Vertically the same bound holds for the body, but the furniture is compared
    /// against a word box's top rather than a baseline — see <see cref="VerticalTolerancePoints"/>.
    /// </remarks>
    private const double TolerancePoints = 0.1;

    /// <summary>
    /// How far a drawn baseline may sit from the top of LibreOffice's word box, in points.
    /// </summary>
    /// <remarks>
    /// A box's top is above the baseline by the font's ascent, which the PDF never states — so the vertical
    /// comparison is of <em>differences</em> between words, and this bounds how much the ascent may vary
    /// between two words of the same size. It is the ascent residual: VCL rounds a face's metrics through
    /// its reference device, which differs from scaling the design values by up to a twip.
    /// </remarks>
    private const double VerticalTolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-furniture").FullName;

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
    [InlineData("furniture.fodt")]
    public void EveryPageDrawsItsHeaderAndFooterWhereLibreOfficeDoes(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        RecordingDrawingSink sink = Record(path);
        List<PdfWord> rendered = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(
            rendered.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        int pages = rendered.Select(word => word.PageIndex).Distinct().Count();
        sink.Pages.Count.ShouldBe(pages, $"{fileName}: page count");
        pages.ShouldBeGreaterThan(1, $"{fileName}: one page would not test a repeated header");

        int compared = 0;
        for (int page = 0; page < pages; page++)
        {
            List<DrawnWord> drawn = InDrawnOrder(DrawnWords.On(sink.Pages[page]));
            List<PdfWord> reference = InReadingOrder(
                [.. rendered.Where(word => word.PageIndex == page)]);

            string.Join(' ', drawn.Select(word => word.Text))
                .ShouldBe(
                    string.Join(' ', reference.Select(word => word.Text)),
                    $"{fileName}: page {page + 1}'s drawn text differs from the rendered text");

            // The header's word is first in reading order and the footer's last, so a page whose header
            // was drawn into the body — or not at all — fails on the text before any position is compared.
            drawn[0].Text.ShouldBe("Headword", $"{fileName}: page {page + 1} starts with the header");
            drawn[^1].Text.ShouldBe("zeta", $"{fileName}: page {page + 1} ends with the footer");

            // The first word of each line only. A word deep inside a line carries the reference's own
            // accumulated error — LibreOffice rounds each glyph's advance into the embedded subset at a
            // thousandth of an em, so poppler's idea of where the tenth word starts is a tenth of a point
            // out — while the first word of a line starts at a position the geometry states outright. Where
            // a line begins is also what this test is about; what happens along it is the mixed-run test's.
            for (int i = 0; i < reference.Count; i++)
            {
                if (i > 0 && Math.Abs(reference[i].Top - reference[i - 1].Top) < 0.5) continue;

                string where = $"{fileName}: page {page + 1}, word {i + 1} (\"{reference[i].Text}\")";

                Math.Abs(drawn[i].Left - (reference[i].Left - PdfPenOffsetPoints))
                    .ShouldBeLessThanOrEqualTo(
                        TolerancePoints,
                        $"{where}: starts at {drawn[i].Left:F3} pt drawn, "
                        + $"{reference[i].Left - PdfPenOffsetPoints:F3} pt rendered");

                compared++;
            }

            // Vertically, the gap between the header's baseline and the footer's is the whole page's
            // arithmetic in one number: the header strip, the body's height and the footer's own strip. It
            // is compared as a difference of box tops because both words are the same size, so the ascent
            // that separates a box top from a baseline cancels.
            double drawnSpan = drawn[^1].Baseline - drawn[0].Baseline;
            double renderedSpan = reference[^1].Top - reference[0].Top;

            Math.Abs(drawnSpan - renderedSpan).ShouldBeLessThanOrEqualTo(
                VerticalTolerancePoints,
                $"{fileName}: page {page + 1}: the header and footer are {drawnSpan:F3} pt apart "
                + $"drawn, {renderedSpan:F3} pt rendered");
        }

        compared.ShouldBeGreaterThan(50, $"{fileName}: too few lines compared to prove anything");
    }

    [Theory]
    [InlineData("furniture.fodt")]
    public void AHeaderPushesTheBodyDown(string fileName)
    {
        string path = Corpus.Require(fileName);
        RecordingDrawingSink sink = Record(path);

        LaidOutPage page = ((WordProcessingPages)Layout(path)).Pages[0];

        // ODF states fo:margin-top as the distance to the top of the *header*, so a document with one has a
        // body that starts below it: the header's own height and the spacing beneath it come out of that
        // margin. A reader that took the margin as the body's would draw the body over the header.
        page.Header.ShouldNotBeNull($"{fileName}: the page has no header");
        page.Header.Area.Y.ShouldBeLessThan(
            page.BodyArea.Y, $"{fileName}: the header is not above the body");
        page.Header.Area.Bottom.ShouldBeLessThanOrEqualTo(
            page.BodyArea.Y, $"{fileName}: the header overlaps the body");

        // And the footer sits below the body, hugging the bottom margin rather than floating at the top of
        // the space reserved for it.
        page.Footer.ShouldNotBeNull($"{fileName}: the page has no footer");
        page.Footer.Area.Y.ShouldBeGreaterThanOrEqualTo(
            page.BodyArea.Bottom, $"{fileName}: the footer overlaps the body");

        List<DrawnWord> drawn = DrawnWords.On(sink.Pages[0]);
        double footerBaseline = drawn.Single(word => word.Text == "Footword").Baseline;
        footerBaseline.ShouldBeGreaterThan(
            page.Footer.Area.Y.Points, $"{fileName}: the footer's text is above its own area");
    }

    // ------------------------------------------------------------------------- the machinery

    private static List<PdfWord> InReadingOrder(List<PdfWord> words)
        => [.. words.OrderBy(word => Math.Round(word.Top, 1)).ThenBy(word => word.Left)];

    private static List<DrawnWord> InDrawnOrder(List<DrawnWord> words)
        => [.. words.OrderBy(word => Math.Round(word.Baseline, 1)).ThenBy(word => word.Left)];

    private static IPageSequence Layout(string path)
    {
        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        using IDocument document = new WordProcessingReader().Read(source);

        return ((IPaginatedDocument)document).Layout();
    }

    private static RecordingDrawingSink Record(string path)
    {
        RecordingDrawingSink sink = new();

        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        using IDocument document = new WordProcessingReader().Read(source);

        IPageSequence pages = ((IPaginatedDocument)document).Layout();
        for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);

        return sink;
    }
}

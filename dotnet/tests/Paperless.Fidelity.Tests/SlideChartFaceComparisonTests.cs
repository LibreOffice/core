using System.Text.RegularExpressions;
using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// A chart's text is set in the theme's minor Latin face, with a stated <c>a:latin</c>
/// overriding it — not in a fixed default.
/// </summary>
/// <remarks>
/// <para>
/// <c>SlideChart</c> set every chart label in Liberation Sans for four rounds, on the evidence
/// that <c>pdffonts</c> reported Liberation Sans in LibreOffice's own PDF of
/// <c>chart-bar-deck.pptx</c>. That deck's chart states <c>&lt;a:latin typeface="Arial"/&gt;</c>
/// eleven times, and fontconfig substitutes Arial with Liberation Sans — so the measurement is
/// equally consistent with a fixed face and with the stated one, and the corpus cannot separate
/// them without also changing the theme, the sizes and the data at once.
/// </para>
/// <para>
/// So the two decks here are that deck with <em>one</em> thing moved. Both set the theme's minor
/// Latin face to <strong>Liberation Mono</strong>, because a monospace is the widest available
/// separation from either proportional candidate and it moves the plot area's left edge as well
/// as the embedded font list. <c>chart-face-theme-minor.pptx</c> then states no <c>a:latin</c> at
/// all and <c>chart-face-stated.pptx</c> states a literal <c>Liberation Serif</c>. LibreOffice
/// 24.2.7.2 embeds LiberationMono in the first and LiberationSerif in the second, which is the
/// pattern only "the theme's minor face, overridden by a stated one" predicts: a fixed face gives
/// two identical answers, and reading only a stated face leaves the first on a fallback.
/// </para>
/// <para>
/// <strong>The face assertion alone would be a metadata test</strong>, and a face that is merely
/// named right moves no ink — the round-nineteen <c>/BaseFont</c> finding is exactly that shape.
/// So the third case measures <em>one digit's advance</em>, taken as the gap between the pen of
/// the value axis' three-digit labels and the pen of its two-digit ones. They are right-aligned
/// on the same edge, so that gap is one digit and nothing else: 6.01 pt in ten-point Liberation
/// Mono against 5.55 in Liberation Sans.
/// </para>
/// <para>
/// <strong>Two more obvious quantities were measured first and both are the wrong thing to
/// assert.</strong> An absolute pen position carries the composition as well as the face: the
/// value axis' labels land 0.36 pt from the reference with this change and 0.96 pt without —
/// better — while the legend lands 2.49 pt out with it and 1.39 pt out without, because the
/// composition has a legend-reservation error of its own that the wrong face was partly
/// cancelling. And a word's ink width carries the two writers' show splitting: the reference
/// positions each digit of "100" separately, so poppler ends its box at the last glyph's ink and
/// reports 17.25 where our single show reports the full 18.03 advance. Neither difference is
/// about the face. The gap between two of the axis' own labels is.
/// </para>
/// </remarks>
public sealed partial class SlideChartFaceComparisonTests : IDisposable
{
    /// <summary>One digit's advance in ten-point Liberation Mono, in points.</summary>
    /// <remarks>
    /// 0.6009 em at the 10.01 pt the labels are drawn at. Liberation Sans' digit is 0.5560 em, or
    /// 5.55 pt, so the two faces are 0.46 pt apart here — five times the tolerance below, and a
    /// quantity neither renderer's plot rectangle can move.
    /// </remarks>
    private const double MonospacedDigitAdvance = 6.01;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-chart-face").FullName;

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

    [Fact]
    public void AChartUnstatedTakesTheThemesMinorFace()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        const string deck = "chart-face-theme-minor.pptx";

        Faces(Ours(deck)).ShouldContain("LiberationMono");
        Faces(_libreOffice.ConvertToPdf(Corpus.Require(deck), _workDirectory))
            .ShouldContain("LiberationMono");
    }

    [Fact]
    public void AChartStatingAFaceTakesTheStatedOneInstead()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        const string deck = "chart-face-stated.pptx";

        List<string> ours = Faces(Ours(deck));
        ours.ShouldContain("LiberationSerif");
        ours.ShouldNotContain("LiberationMono");

        List<string> theirs = Faces(_libreOffice.ConvertToPdf(Corpus.Require(deck), _workDirectory));
        theirs.ShouldContain("LiberationSerif");
        theirs.ShouldNotContain("LiberationMono");
    }

    /// <summary>
    /// The face is what the labels are <em>measured</em> in, not only what they are named in.
    /// </summary>
    [Fact]
    public void TheThemesFaceDecidesTheValueLabelsAdvances()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        const string deck = "chart-face-theme-minor.pptx";

        double ours = DigitAdvance(PdfTextRuns.Read(Ours(deck)));
        double theirs = DigitAdvance(
            PdfTextRuns.Read(_libreOffice.ConvertToPdf(Corpus.Require(deck), _workDirectory)));

        // Ours against the literal first, so this tests Paperless rather than an agreement.
        ours.ShouldBe(MonospacedDigitAdvance, 0.1, "our digit advance");
        theirs.ShouldBe(MonospacedDigitAdvance, 0.1, "the reference's digit advance");
    }

    /// <summary>
    /// One digit's advance: the gap between the pens of the value axis' three- and two-digit
    /// labels, which share a right edge.
    /// </summary>
    /// <remarks>
    /// Each group is the <em>commonest</em> pen among the runs of that glyph count rather than
    /// the only one, and is required to hold at least three of them — the axis draws five
    /// three-digit labels and four two-digit ones. Requiring a single pen instead failed on our
    /// own output, because a rotated axis title contributes runs whose <c>Td</c> is in its own
    /// rotated space and lands in the same band; taking the mode is what makes the measurement
    /// about the labels rather than about everything to the left of the plot area.
    /// </remarks>
    private static double DigitAdvance(List<PdfTextRun> runs)
    {
        List<PdfTextRun> axis = [.. runs.Where(r => r.PageIndex == 0 && r.X < 150.0)];

        return CommonestPen(axis, glyphs: 2) - CommonestPen(axis, glyphs: 3);
    }

    /// <summary>The pen shared by most of the runs holding <paramref name="glyphs"/> glyphs.</summary>
    private static double CommonestPen(List<PdfTextRun> runs, int glyphs)
    {
        IGrouping<double, PdfTextRun> flush = runs
            .Where(r => r.GlyphCount == glyphs)
            .GroupBy(r => Math.Round(r.X, 2))
            .MaxBy(group => group.Count())
            ?? throw new InvalidOperationException($"no {glyphs}-glyph runs left of the plot area");

        flush.Count().ShouldBeGreaterThanOrEqualTo(
            3, $"value-axis labels flush at x = {flush.Key}");

        return flush.Key;
    }

    /// <summary>Every <c>/BaseFont</c> in the file, subset prefix stripped.</summary>
    private static List<string> Faces(string pdfPath)
    {
        string text = System.Text.Encoding.Latin1.GetString(File.ReadAllBytes(pdfPath));
        return [.. BaseFont().Matches(text).Select(m => m.Groups[1].Value).Distinct()];
    }

    private string Ours(string deck)
    {
        string source = Corpus.Require(deck);
        string destination = Path.Combine(
            _workDirectory, Path.GetFileNameWithoutExtension(deck) + "-paperless.pdf");

        using IDocument document = PaperlessDocument.Open(source);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }

    [GeneratedRegex(@"/BaseFont\s*/(?:[A-Z]{6}\+)?([A-Za-z0-9-]+)")]
    private static partial Regex BaseFont();
}

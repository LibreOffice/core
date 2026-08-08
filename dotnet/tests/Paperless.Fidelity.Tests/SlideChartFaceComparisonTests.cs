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
/// So the pen positions are asserted too: the category labels along the bottom of the chart are
/// where the face's advance widths land them, and Liberation Mono puts them 5 pt from where
/// Liberation Sans does. Ours is asserted against literals first and the reference compared
/// against the same ones, so this tests Paperless rather than whatever the two of us agree on.
/// </para>
/// </remarks>
public sealed partial class SlideChartFaceComparisonTests : IDisposable
{
    /// <summary>A tenth of a point: finer than the 5 pt the face moves these labels by.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>
    /// The x of the four category labels along the bottom of the chart, in points, as read out of
    /// LibreOffice 24.2.7.2's own PDF of <c>chart-face-theme-minor.pptx</c>.
    /// </summary>
    /// <remarks>
    /// Their spacing is the plot area's, which the value-axis labels' width reserves, so these
    /// four numbers carry both halves of the claim: the face the labels are measured in and the
    /// rectangle that measurement produced.
    /// </remarks>
    private static readonly double[] CategoryLabelX = [164.26, 287.99, 411.74, 535.47];

    /// <summary>The y those four sit on, which separates them from every other run on the page.</summary>
    private const double CategoryLabelY = 101.54;

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
    public void TheThemesFaceDecidesWhereTheCategoryLabelsLand()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        const string deck = "chart-face-theme-minor.pptx";

        List<double> ours = CategoryLabels(PdfTextRuns.Read(Ours(deck)));
        List<double> theirs = CategoryLabels(
            PdfTextRuns.Read(_libreOffice.ConvertToPdf(Corpus.Require(deck), _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "no text runs came back from the reference");

        ours.Count.ShouldBe(CategoryLabelX.Length, "category labels in our rendering");
        theirs.Count.ShouldBe(CategoryLabelX.Length, "category labels in the reference");

        for (int at = 0; at < CategoryLabelX.Length; at++)
        {
            ours[at].ShouldBe(CategoryLabelX[at], TolerancePoints, $"our category label {at}");
            theirs[at].ShouldBe(CategoryLabelX[at], TolerancePoints, $"the reference's label {at}");
        }
    }

    private static List<double> CategoryLabels(List<PdfTextRun> runs)
        => [.. runs.Where(r => r.PageIndex == 0 && Math.Abs(r.Y - CategoryLabelY) < 1.0)
                   .Select(r => r.X)
                   .Order()];

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

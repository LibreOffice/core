using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that a tab advances to the stop LibreOffice advances to.
/// </summary>
/// <remarks>
/// <para>
/// Word by word and absolutely, which for tabs is both possible and the point. A tab's width is not a
/// property of the font — it is the distance to the next stop — so the word after one starts at an absolute
/// position that the document states, and comparing it needs none of the differential machinery
/// justification does. The quantisation that spoils a long line does not accumulate here either, because
/// each stretch between tabs is short and starts afresh at its stop.
/// </para>
/// <para>
/// The corpus document exercises the four things that can go wrong separately: the default interval, which
/// is a quarter over a centimetre in LibreOffice and not the half inch Word uses; explicit left stops;
/// centre and right stops, where the stretch's own width decides where it starts; and a decimal stop, where
/// the position of a separator inside the stretch decides it.
/// </para>
/// </remarks>
public sealed class TabStopComparisonTests : IDisposable
{
    /// <summary>How far a drawn word may differ from LibreOffice's, in points.</summary>
    /// <remarks>
    /// A tenth of a point, two twips — the same bound the run comparison uses. A stop read in the wrong
    /// unit, or a default interval of 720 twips where LibreOffice uses 709, misses by pounds rather than
    /// pence: eleven twips per tab, over half a point.
    /// </remarks>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    /// <remarks>
    /// Measured at three different left margins; <see cref="MixedRunComparisonTests"/> records the
    /// evidence. Additive, horizontal only, and a property of the export rather than of the layout.
    /// </remarks>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-tabbed").FullName;

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
    [InlineData("tabbed.fodt")]
    [InlineData("tabbed.docx")]
    [InlineData("tabbed.doc")]
    [InlineData("tabbed.rtf")]
    public void EveryTabAdvancesToLibreOfficesStop(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<DrawnWord> drawn = Drawn(path);
        List<PdfWord> rendered = InReadingOrder(
            PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(
            rendered.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        string.Join(' ', drawn.Select(word => word.Text))
            .ShouldBe(
                string.Join(' ', rendered.Select(word => word.Text)),
                $"{fileName}: the drawn text differs from the rendered text");

        int afterTab = 0;
        for (int i = 0; i < rendered.Count; i++)
        {
            string where = $"{fileName}: word {i + 1} (\"{rendered[i].Text}\")";

            Math.Abs(drawn[i].Left - (rendered[i].Left - PdfPenOffsetPoints))
                .ShouldBeLessThanOrEqualTo(
                    TolerancePoints,
                    $"{where}: starts at {drawn[i].Left:F3} pt drawn, "
                    + $"{rendered[i].Left - PdfPenOffsetPoints:F3} pt rendered");

            // Words that a tab moved rather than words that simply follow a space: every word here but
            // the first of its line is one, since the corpus document separates them all with tabs.
            if (i > 0 && Math.Abs(rendered[i].Top - rendered[i - 1].Top) < 0.5) afterTab++;
        }

        afterTab.ShouldBeGreaterThan(
            20, $"{fileName}: only {afterTab} words followed a tab, which proves too little");
    }

    // ------------------------------------------------------------------------- the machinery

    private static List<PdfWord> InReadingOrder(List<PdfWord> words)
        => [.. words
            .OrderBy(word => word.PageIndex)
            .ThenBy(word => Math.Round(word.Top, 1))
            .ThenBy(word => word.Left)];

    private static List<DrawnWord> Drawn(string path)
    {
        RecordingDrawingSink sink = new();

        using (FileStream stream = File.OpenRead(path))
        {
            using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return
        [
            .. sink.Pages.SelectMany(page => DrawnWords.On(page)
                .OrderBy(word => Math.Round(word.Baseline, 1))
                .ThenBy(word => word.Left)),
        ];
    }
}

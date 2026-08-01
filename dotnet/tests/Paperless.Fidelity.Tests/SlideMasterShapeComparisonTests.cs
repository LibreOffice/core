using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// What a slide inherits from its master and its layout that is not a placeholder.
/// </summary>
/// <remarks>
/// <para>
/// <strong>An Impress master page is a PPTX master and a PPTX layout merged into one</strong>, so
/// a logo on the master and a strapline on the layout are drawn under every slide that uses that
/// layout. <c>oox/source/ppt/presentationfragmenthandler.cxx:246-296</c> makes one
/// <c>SlidePersist</c> per layout, imports the master fragment into it, then the layout fragment
/// into the same one, and calls <c>createXShapes</c> once over the pair.
/// </para>
/// <para>
/// This was an open question for a long time and the numbers made it look marginal: only six of
/// the 389 decks in <c>sd/qa/unit/data/pptx</c> carry a master <c>p:sp</c> with no <c>p:ph</c> and
/// non-empty text, and three of those carry only the <c>&#8249;#&#8250;</c> slide-number glyph. It
/// is not marginal, because on the four real ones LibreOffice draws the text and Paperless drew
/// nothing: <c>master-slides.pptx</c>'s "Copyright © SUSE", <c>cshapes.pptx</c>'s Novell
/// strapline and <c>tdf149865.pptx</c>'s "Copyright © SUSE 2021" were simply lost.
/// </para>
/// <para>
/// <strong>The trap is <c>slide-sections.pptx</c>, and it is worth naming.</strong> LibreOffice
/// draws its master's strapline on none of its seven pages and its layout's on the seventh, which
/// reads exactly like a visibility flag — and none of the four decks states <c>showMasterSp</c> at
/// all. It is not a flag: that master's three text boxes sit at y = 6 959 601 on a 6 858 000 slide
/// and at x = −2 250 002 and x = −950 805, off the page in three directions. They are drawn, into
/// nothing. The seventh page differs only because it is the only slide using the layout that
/// carries the on-page one.
/// </para>
/// <para>
/// The fixture states the flag anyway, on its second slide, because nothing in the corpus
/// otherwise exercises it and a rule with no negative case is not pinned.
/// </para>
/// </remarks>
public sealed class SlideMasterShapeComparisonTests : IDisposable
{
    /// <summary>A twentieth of a point, which is what these runs actually agree to.</summary>
    private const double TolerancePoints = 0.1;

    private const string Deck = "slide-master-shapes.pptx";

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-master-shapes").FullName;

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
    /// Every run LibreOffice draws, Paperless draws, at the same pen and the same size.
    /// </summary>
    /// <remarks>
    /// Five runs on two slides: the first slide's own title and body, its layout's strapline and
    /// its master's, and the second slide's title alone — the second states
    /// <c>showMasterSp="0"</c> and so inherits neither. Comparing the whole list rather than
    /// hunting for the two straplines is deliberate: drawing a master's <em>placeholder</em>
    /// prompts as well would put "Click to edit Master title style" on every slide of every deck,
    /// and only a count catches that.
    /// </remarks>
    [Fact]
    public void EveryInheritedShapeIsDrawnWhereLibreOfficeDrawsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs =
            PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, "number of drawn runs");

        List<PdfTextRun> mine = [.. ours.OrderBy(r => r.PageIndex).ThenBy(r => r.Y).ThenBy(r => r.X)];
        List<PdfTextRun> reference =
            [.. theirs.OrderBy(r => r.PageIndex).ThenBy(r => r.Y).ThenBy(r => r.X)];

        for (int i = 0; i < reference.Count; i++)
        {
            string where = $"run {i + 1}";

            mine[i].PageIndex.ShouldBe(reference[i].PageIndex, $"{where}: slide");
            mine[i].X.ShouldBe(reference[i].X, TolerancePoints, $"{where}: pen across");
            mine[i].Y.ShouldBe(reference[i].Y, TolerancePoints, $"{where}: baseline");
            mine[i].FontSize.ShouldBe(reference[i].FontSize, TolerancePoints, $"{where}: em size");
            mine[i].GlyphCount.ShouldBe(reference[i].GlyphCount, $"{where}: glyph count");
        }
    }

    /// <summary>
    /// A slide stating <c>showMasterSp="0"</c> inherits nothing, as LibreOffice's does.
    /// </summary>
    /// <remarks>
    /// Stated separately from the run-by-run comparison because it is the half that a reader
    /// ignoring the attribute would still pass: the counts would agree if the flag were honoured
    /// on neither side, and this says which side of the page the two straplines are missing from.
    /// </remarks>
    [Fact]
    public void ASlideSuppressingMasterShapesInheritsNothing()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));

        Assert.SkipWhen(ours.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count(run => run.PageIndex == 0).ShouldBe(4, "runs on the inheriting slide");
        ours.Count(run => run.PageIndex == 1).ShouldBe(1, "runs on the suppressing slide");
    }

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

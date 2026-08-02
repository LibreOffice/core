using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Where a slide draws a superscript, against where LibreOffice draws it.
/// </summary>
/// <remarks>
/// <para>
/// Two numbers decide a raised run and the formats state only one of them: DrawingML's
/// <c>a:rPr/@baseline</c> gives the offset, and the importer supplies the size from
/// <c>DFLT_ESC_PROP</c> (<c>oox/source/drawingml/textcharacterproperties.cxx:196-199</c>). Both
/// were read into a <c>RunEmphasis</c> flag for extraction and neither reached the layout, so
/// every ordinal in every deck sat on the baseline at full size.
/// </para>
/// <para>
/// The <em>size</em> is what a corpus notices, because it moves line breaks: a run set at 58% is
/// 42% narrower, and a line that fits shrunk wraps unshrunk. On
/// <c>slides/batch-003/pptx/NCW-2024-Guide-.pptx</c> one such wrap pushed the last paragraph of
/// an already-overflowing text box off the bottom of the slide.
/// </para>
/// <para>
/// <c>slide-escapement.pptx</c> states the same three characters twice — plain and raised — so a
/// layout that ignored the attribute and one that applied it to everything both fail, which no
/// single-case document can distinguish.
/// </para>
/// </remarks>
public sealed class SlideEscapementComparisonTests : IDisposable
{
    /// <summary>
    /// A twentieth of a point across and a tenth down.
    /// </summary>
    /// <remarks>
    /// Wider than this project's usual tenth for the size alone: LibreOffice quantises the
    /// shrunk font height to its reference device's hundredth of a millimetre, so 58% of 24 pt
    /// comes out 13.897 against the exact 13.92.
    /// </remarks>
    private const double TolerancePoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-escapement").FullName;

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
    /// The raised and lowered runs land where LibreOffice puts them, at the size it uses.
    /// </summary>
    /// <remarks>
    /// Matched by position rather than by index, because LibreOffice merges the plain paragraph's
    /// three runs into one text block and we emit one per <c>a:r</c> — a difference in how many
    /// draw calls carry the same ink, which no reader can see. What is compared is the run that
    /// is off the baseline in each PDF, of which each has exactly two.
    /// </remarks>
    [Fact]
    public void AnEscapedRunIsRaisedAndShrunkTheWayLibreOfficeDoesIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("slide-escapement.pptx");
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs = PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "no text runs read from the reference PDF");

        List<PdfTextRun> mine = Escaped(ours);
        List<PdfTextRun> reference = Escaped(theirs);

        // Two: the superscript in the second box and the subscript in the third. A layout that
        // escaped nothing gives none, and one that escaped everything gives six.
        reference.Count.ShouldBe(2);
        mine.Count.ShouldBe(reference.Count);

        for (int i = 0; i < reference.Count; i++)
        {
            string where = i == 0 ? "the superscript" : "the subscript";
            mine[i].X.ShouldBe(reference[i].X, 0.5, $"{where}: pen");
            mine[i].Y.ShouldBe(reference[i].Y, TolerancePoints, $"{where}: baseline");
            mine[i].FontSize.ShouldBe(reference[i].FontSize, TolerancePoints, $"{where}: size");
        }
    }

    /// <summary>
    /// The plain paragraph is drawn on its own baseline at its own size, in both.
    /// </summary>
    /// <remarks>
    /// The control half. Its first box states the same characters as the second and nothing
    /// raised, so it is what proves the escapement is the run's property and not the shape's.
    /// </remarks>
    [Fact]
    public void ThePlainParagraphIsUntouched()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("slide-escapement.pptx");
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs = PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "no text runs read from the reference PDF");

        double top = theirs[0].Y;
        foreach (PdfTextRun run in ours)
        {
            if (run.Y > top + TolerancePoints) continue;

            run.Y.ShouldBe(top, TolerancePoints, "a plain run left its line's baseline");
            run.FontSize.ShouldBe(theirs[0].FontSize, TolerancePoints, "a plain run was resized");
        }
    }

    /// <summary>
    /// The runs drawn smaller than the deck's own text size, in drawing order.
    /// </summary>
    /// <remarks>
    /// Selected by size rather than by index, because the two PDFs cut the same ink into
    /// different numbers of text blocks — LibreOffice merges the plain paragraph's three runs
    /// into one and we emit one per <c>a:r</c>. Every run in the document is 24 pt except the two
    /// escaped ones, so "smaller than the largest" names exactly them in either file, and names
    /// nothing at all in a build that has stopped shrinking.
    /// </remarks>
    private static List<PdfTextRun> Escaped(List<PdfTextRun> runs)
    {
        double largest = 0;
        foreach (PdfTextRun run in runs) largest = Math.Max(largest, run.FontSize);

        List<PdfTextRun> escaped = [];
        foreach (PdfTextRun run in runs)
        {
            if (run.FontSize < largest * 0.9) escaped.Add(run);
        }

        return escaped;
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

using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Word 2013's justification: a full line may squeeze its blanks to hold another word.
/// </summary>
/// <remarks>
/// <para>
/// The pair of corpus documents is one justified paragraph twice, differing only in the
/// <c>compatibilityMode</c> their <c>settings.xml</c> declares — 15 against 12. LibreOffice turns
/// <c>JustifyLinesWithShrinking</c> on for the first and not for the second
/// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:10172</c>), and sets the same text in
/// four lines rather than five.
/// </para>
/// <para>
/// A pair rather than one document, because the effect has to be shown to be <em>conditional</em>: an
/// engine that shrank every justified line regardless would agree with the reference on the first
/// document and disagree on the second, and one that shrank none would do the opposite.
/// </para>
/// <para>
/// Line ends are compared against the reference rather than against stored numbers, so the test stays
/// honest across a LibreOffice upgrade. Measured on 24.2.7.2.
/// </para>
/// </remarks>
public sealed class JustificationShrinkComparisonTests : IDisposable
{
    /// <summary>How far a drawn line's right edge may sit from LibreOffice's, in points.</summary>
    /// <remarks>
    /// A point and a half. This compares an absolute right edge, so it carries the whole of the width
    /// disagreement between HarfBuzz and Writer over a 480 pt line: measured here, our stretched lines
    /// end on the margin at 538.58 pt and LibreOffice's at 537.42–537.81, a difference of up to 1.16 pt
    /// that is the same with and without shrinking. What the tolerance has to exclude is a whole word,
    /// and the shortest on these lines is eight points wide.
    /// </remarks>
    private const double TolerancePoints = 1.5;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-shrink").FullName;

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
    [InlineData("justify-shrink-2013.docx")]
    [InlineData("justify-shrink-2007.docx")]
    public void TheParagraphBreaksWhereLibreOfficeBreaksIt(string document)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(document);

        List<double> reference = Rendered(path);
        Assert.SkipWhen(
            reference.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        List<double> ours = LineEnds(Drawn(path).Select(word => (word.Right, word.Baseline)));

        ours.Count.ShouldBe(
            reference.Count,
            $"{document} set in {ours.Count} lines against LibreOffice's {reference.Count}");

        for (int i = 0; i < ours.Count; i++)
        {
            Math.Abs(ours[i] - reference[i]).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{document} line {i + 1} ends at {ours[i]:F2} pt against LibreOffice's "
                + $"{reference[i]:F2} pt");
        }
    }

    /// <summary>
    /// The mode-15 document really does set shorter, or the pair proves nothing.
    /// </summary>
    /// <remarks>
    /// Guards the fixture rather than the engine: if a LibreOffice upgrade or an edit to the documents
    /// left the two paragraphs breaking alike, the test above would pass against an engine that ignored
    /// the setting entirely.
    /// </remarks>
    [Fact]
    public void TheReferenceItselfSetsTheModeFifteenDocumentInFewerLines()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<double> newer = Rendered(Corpus.Require("justify-shrink-2013.docx"));
        Assert.SkipWhen(
            newer.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        List<double> older = Rendered(Corpus.Require("justify-shrink-2007.docx"));

        newer.Count.ShouldBeLessThan(older.Count);
    }

    private List<double> Rendered(string path)
        => LineEnds(
            PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory))
                    .Select(word => (word.Right, word.Top)));

    /// <summary>The right edge of the last word on each line, top to bottom.</summary>
    private static List<double> LineEnds(IEnumerable<(double Right, double Top)> words)
        => [.. words.GroupBy(word => Math.Round(word.Top, 1))
                    .OrderBy(line => line.Key)
                    .Select(line => line.Max(word => word.Right))];

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

        return [.. sink.Pages.SelectMany(DrawnWords.On)];
    }
}

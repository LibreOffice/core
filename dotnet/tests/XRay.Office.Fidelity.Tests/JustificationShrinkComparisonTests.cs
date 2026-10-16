using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using XRay.Office.TestKit.LibreOffice;
using XRay.Office.WordProcessing;
using Shouldly;

namespace XRay.Office.Fidelity.Tests;

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
// [rule ported 2026-09-06; the residual is handed back] 26.2.4.2 shrinks less than 24.2.7.2 did and
// this pair is where it shows. 24.2.7.2 set the mode-15 document in **4** lines and the mode-12 one in
// 5; 26.2.4.2 sets both in 5. That was recorded as upstream work to follow, and it has now been
// followed: `portxt.cxx`:769-805 does not take the deepest squeeze the 75% floor allows, it weighs the
// blanks the longer line would squeeze against the blanks the shorter line would stretch, discounting
// the stretch by 1/1.7. `JustificationShrink.PrefersShrinking` is that comparison and it calls all six
// decided rows of a text-width sweep of this very paragraph — see `probes/words-justify-shrink/`.
//
// With it in place the pair matches 26.2.4.2 **line for line, both documents**, at text widths of 9639
// and 9637 twips. The fixture's own width is 9638, and that is the one width in the neighbourhood where
// the reference disagrees with itself: sweeping the disputed line's own paragraph a twip at a time,
// 26.2.4.2 takes the word `line` at all thirteen widths from 9631 to 9644 except 9638. 9638 twips is
// exactly the natural advance of that line as the reference measures it — 481.9000 pt against our
// 481.9400 — so the fixture's measure lands on the tie, and 0.8 twips of advance puts us the other side
// of it. `TheParagraphBreaksWhereLibreOfficeBreaksIt` therefore still fails on
// `justify-shrink-2013.docx`, now on line 3 rather than on the line count, and what it is failing on is
// the advance divergence at a tie the reference itself does not resolve consistently. It is not ours to
// close here, and the page width was deliberately not moved to make it green.
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
        Directory.CreateTempSubdirectory("xray-office-shrink").FullName;

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
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

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
    /// The mode-15 document really does set tighter, or the pair proves nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Guards the fixture rather than the engine: if a LibreOffice upgrade or an edit to the documents
    /// left the two paragraphs breaking alike, the test above would pass against an engine that ignored
    /// the setting entirely.
    /// </para>
    /// <para>
    /// <b>Asserted as "tighter" and not "in fewer lines", which is a correction and not a
    /// loosening.</b> A line saved is one consequence of shrinking and not the thing itself, and how
    /// much shrinking buys is exactly what moved between the two references: 24.2.7.2 sets the mode-15
    /// document in 4 lines against the mode-12 one's 5, and 26.2.4.2 sets both in 5 — with the mode-15
    /// document's last line ending at <b>113.70 pt against the mode-12 one's 164.92</b>, so it has
    /// carried 51 pt more text into the four lines above. Either measure says the same thing about the
    /// fixture, and stating both is what makes this survive the next upgrade as well.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheReferenceItselfSetsTheModeFifteenDocumentTighter()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

        List<double> newer = Rendered(Corpus.Require("justify-shrink-2013.docx"));
        Assert.SkipWhen(
            newer.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        List<double> older = Rendered(Corpus.Require("justify-shrink-2007.docx"));

        if (newer.Count != older.Count)
        {
            newer.Count.ShouldBeLessThan(
                older.Count,
                "the mode-15 document should not need more lines than the mode-12 one");
            return;
        }

        newer[^1].ShouldBeLessThan(
            older[^1] - TolerancePoints,
            $"set in {newer.Count} lines apiece, the mode-15 document's last line ends at "
            + $"{newer[^1]:F2} pt and the mode-12 one's at {older[^1]:F2} — the shrinking has to have "
            + "carried more text into the lines above, or the pair proves nothing");
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

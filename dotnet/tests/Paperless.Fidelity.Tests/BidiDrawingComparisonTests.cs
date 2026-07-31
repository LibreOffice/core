using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that a mixed-direction line is <em>drawn</em> where LibreOffice draws it.
/// </summary>
/// <remarks>
/// <para>
/// The sibling of <see cref="BidiItemisationComparisonTests"/>, one step further along: that one
/// compares the sub-runs a paragraph is cut into, and this compares where the engine actually puts
/// them. LibreOffice emits one <c>BT … ET</c> block per portion with an absolute pen, and it splits
/// a portion at every change of direction — so its pens <em>are</em> the reordering, and a run
/// drawn in logical order lands on a pen belonging to a different portion.
/// </para>
/// <para>
/// The comparison is "every one of LibreOffice's pens is one of ours" rather than the converse, and
/// the asymmetry is the same one the itemisation comparison documents: Paperless splits at every
/// UAX #24 script boundary and Writer at its own three coarse font classes
/// (<c>i18nutil/source/utility/scriptclass.cxx</c>), so a space between Latin and Hebrew starts a
/// sub-run here that Writer keeps inside its Hebrew portion. Ours are therefore finer, and what can
/// be asserted exactly is that every boundary Writer <em>does</em> draw at is one Paperless draws at
/// too, at the same pen.
/// </para>
/// <para>
/// The document's right-to-left paragraphs are aligned with ODF's physical <c>left</c> and
/// <c>right</c> rather than its relative <c>start</c> and <c>end</c>, because
/// <c>ParagraphAdjust::START</c> only arrived in LibreOffice 26.2 and 24.2 renders a start-aligned
/// right-to-left paragraph against the left margin. See the comment in the document itself.
/// </para>
/// </remarks>
public sealed class BidiDrawingComparisonTests : IDisposable
{
    /// <summary>
    /// How far a drawn pen may differ from LibreOffice's, in points.
    /// </summary>
    /// <remarks>
    /// A quarter of a point, which is the itemisation comparison's own bound and is a measurement
    /// rather than a guess: VCL lays each portion out through its reference device and rounds, and
    /// over this document's portions the largest difference is 0.221 pt. It does not accumulate,
    /// because every pen is absolute. LibreOffice's constant two-twip horizontal offset — the
    /// documented deviation — is inside it as well.
    /// </remarks>
    private const double TolerancePoints = 0.25;

    /// <summary>How many paragraphs the corpus document holds, each one line long.</summary>
    private const int Paragraphs = 7;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-bidi-drawing").FullName;

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
    public void EveryPortionLibreOfficeDrawsStartsWhereOneOfOursDoes()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<List<PdfTextRun>> reference = ReferenceLines();
        List<List<DrawnGlyphRun>> drawn = DrawnLines();

        int compared = 0;

        for (int line = 0; line < Paragraphs; line++)
        {
            // Measured from each side's own line start rather than from the page, so that the two
            // things a pen carries are asserted separately: where the line begins is the previous
            // test, and this is where the runs sit along it. Absolute pens would fold LibreOffice's
            // constant two-twip offset into every one of these comparisons.
            double from = drawn[line].Min(run => run.Origin.X.Points);
            double theirs = reference[line].Min(portion => portion.X);

            List<double> pens = [.. drawn[line].Select(run => run.Origin.X.Points - from)];

            foreach (PdfTextRun portion in reference[line])
            {
                double at = portion.X - theirs;

                pens.Any(pen => Math.Abs(pen - at) <= TolerancePoints).ShouldBeTrue(
                    $"line {line + 1}: LibreOffice starts a portion {at:F2} pt along it, "
                    + $"where Paperless draws runs at {string.Join(", ", pens.Select(p => $"{p:F2}"))}");
                compared++;
            }
        }

        // A count, so that a reference that lost its portions cannot leave this comparing nothing.
        // Seven lines, nineteen portions between them.
        compared.ShouldBe(19, "the reference's portions");
    }

    [Fact]
    public void EveryLineStartsWhereLibreOfficeStartsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<List<PdfTextRun>> reference = ReferenceLines();
        List<List<DrawnGlyphRun>> drawn = DrawnLines();

        for (int line = 0; line < Paragraphs; line++)
        {
            // The leftmost pen, which for a right-aligned line is the whole line's measurement as
            // well as its placement: it is the right margin less everything the line holds. That is
            // what makes this one number worth asserting on a mirrored paragraph — an indent put on
            // the wrong side, an alignment not mirrored, or a sub-run shaped in the wrong face all
            // move it.
            double mine = drawn[line].Min(run => run.Origin.X.Points);
            double theirs = reference[line].Min(portion => portion.X);

            mine.ShouldBe(
                theirs,
                TolerancePoints,
                $"line {line + 1} starts at {mine:F2} pt and LibreOffice starts it at {theirs:F2}");
        }
    }

    [Fact]
    public void TheRunsOfALineAreDrawnLeftToRight()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<List<DrawnGlyphRun>> drawn = DrawnLines();
        int reordered = 0;

        foreach (List<DrawnGlyphRun> line in drawn)
        {
            for (int i = 1; i < line.Count; i++)
            {
                line[i].Origin.X.ShouldBeGreaterThan(
                    line[i - 1].Origin.X, "a run is drawn left of the one before it");
            }
        }

        // And the drawing really does reorder: on a line whose runs are drawn in logical order the
        // concatenated text is the paragraph's own. Three of the seven paragraphs move a run, and
        // fewer do than mix directions — a single right-to-left island between two left-to-right
        // stretches reverses in place and leaves the run order alone, and a paragraph that is
        // right-to-left throughout is one run whose glyphs the shaper reverses. Asserting the count
        // keeps a document that flattened from leaving this checking nothing.
        List<string> texts = [.. Texts()];

        for (int line = 0; line < Paragraphs; line++)
        {
            string concatenated = string.Concat(drawn[line].Select(run => run.Run.Text));
            if (concatenated != texts[line]) reordered++;

            concatenated.Order().ShouldBe(
                texts[line].Order(),
                $"line {line + 1}: the drawn text is not a permutation of the paragraph's");
        }

        reordered.ShouldBe(3, "the paragraphs whose runs move");
    }

    [Fact]
    public void TheLatinParagraphIsStillOneRun()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        // The no-op, against the reference rather than against ourselves: LibreOffice draws the
        // Latin paragraph as one portion and so must Paperless. A run split at a boundary it does
        // not need loses the shaping context across it and measures very slightly wide.
        ReferenceLines()[0].Count.ShouldBe(1, "LibreOffice draws the Latin line as one portion");
        DrawnLines()[0].Count.ShouldBe(1, "and so does Paperless");
    }

    /// <summary>The reference PDF's portions, grouped into lines by their baseline.</summary>
    private List<List<PdfTextRun>> ReferenceLines()
    {
        string path = Corpus.Require("bidi-drawing.fodt");
        List<PdfTextRun> runs = PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        List<List<PdfTextRun>> lines = [];
        double baseline = double.NaN;

        foreach (PdfTextRun run in runs)
        {
            if (lines.Count == 0 || Math.Abs(run.Y - baseline) > 0.001)
            {
                lines.Add([]);
                baseline = run.Y;
            }

            lines[^1].Add(run);
        }

        lines.Count.ShouldBe(Paragraphs, "the reference has one line per paragraph");
        return lines;
    }

    /// <summary>The glyph runs Paperless draws, grouped into lines by their baseline.</summary>
    /// <remarks>
    /// Through the whole document rather than through a hand-built paragraph, because what is being
    /// compared includes the reader's answer about the paragraph's direction and the page's own
    /// geometry — both of which a hand-built input would supply itself and so not test.
    /// </remarks>
    private static List<List<DrawnGlyphRun>> DrawnLines()
    {
        RecordingDrawingSink sink = new();

        using (IDocument document = Open(Corpus.Require("bidi-drawing.fodt")))
        {
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1, "the document is one page");
            pages[0].Draw(sink);
        }

        List<List<DrawnGlyphRun>> lines = [];

        foreach (DrawnGlyphRun run in sink.Pages[0].Runs)
        {
            if (lines.Count == 0
                || Math.Abs(run.Origin.Y.Points - lines[^1][0].Origin.Y.Points) > 0.001)
            {
                lines.Add([]);
            }

            lines[^1].Add(run);
        }

        lines.Count.ShouldBe(Paragraphs, "one drawn line per paragraph");
        return lines;
    }

    /// <summary>The paragraphs' text, in order, as Paperless read it.</summary>
    private static IEnumerable<string> Texts()
    {
        using IDocument document = Open(Corpus.Require("bidi-drawing.fodt"));

        return ((IPaginatedDocument)document).Layout() is WordProcessing.Layout.WordProcessingPages pages
            ? [.. pages.Paragraphs.Where(p => p.Text.Length > 0).Select(p => p.Text)]
            : [];
    }

    private static IDocument Open(string path)
    {
        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        return new WordProcessingReader().Read(source);
    }
}

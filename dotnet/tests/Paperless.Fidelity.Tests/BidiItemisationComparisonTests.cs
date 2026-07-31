using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.Text.Itemisation;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks the direction sub-runs a paragraph is cut into against LibreOffice's own text portions.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice writes one <c>BT … ET</c> block per portion, and a portion ends at every change of
/// direction, script or font — which is precisely the boundary this is computing. Better still, it
/// emits them in <em>logical</em> order and positions each with an absolute pen, so one PDF gives
/// both halves of the answer: the glyph counts, read cumulatively, are the portion boundaries as
/// character offsets, and the pens are the visual order the reordering produced.
/// </para>
/// <para>
/// Two things are asserted, and only two, because only two are comparable. Every boundary between
/// two embedding levels must be one of LibreOffice's portion boundaries — the converse does not
/// hold, and deliberately: Writer also splits a portion at a change of <em>font</em> script, and its
/// font scripts are the three coarse classes <c>GetScriptClass</c> assigns
/// (<c>i18nutil/source/utility/scriptclass.cxx</c>), not the UAX #24 scripts a shaper is told about.
/// A space is <c>WEAK</c> there and an opening parenthesis is <c>LATIN</c>, so "‏שלום (abc) עולם"
/// gets a Writer boundary at the bracket that no shaper-level itemisation has. And the pen of the
/// leftmost portion covering each sub-run must rise strictly along the order the reordering puts
/// them in, which is rule L2 measured rather than argued.
/// </para>
/// <para>
/// The document is set in DejaVu Sans, the only face on a bare Linux carrying Latin, Hebrew and
/// Arabic together, and its text is chosen so that nothing ligates. That is what makes a portion's
/// glyph count its character count, and the test asserts it rather than assuming it — a ligature
/// anywhere would silently shift every offset after it.
/// </para>
/// </remarks>
public sealed class BidiItemisationComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-bidi").FullName;

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

    /// <summary>The paragraphs of <c>bidi.fodt</c>, in order, and the direction each is declared in.</summary>
    /// <remarks>
    /// Repeated here rather than read out of the document because the reference is LibreOffice's
    /// rendering of it, not Paperless's reading of it: a bug in the ODF reader would otherwise make
    /// this test compare the wrong text and pass.
    /// </remarks>
    private static readonly (string Text, BidiDirection Direction)[] Paragraphs =
    [
        ("Latin only, with 123 numbers.", BidiDirection.LeftToRight),
        ("שלום עולם", BidiDirection.LeftToRight),
        ("Start שלום end.", BidiDirection.LeftToRight),
        ("Start שלום 123 עולם end.", BidiDirection.LeftToRight),
        ("שלום (abc) עולם", BidiDirection.LeftToRight),
        ("שלום abc עולם 42 xyz", BidiDirection.LeftToRight),
        ("שלום end עולם", BidiDirection.RightToLeft),
        ("שלום 123 עולם", BidiDirection.RightToLeft),
        ("Latin start שלום done", BidiDirection.RightToLeft),
    ];

    [Fact]
    public void EveryDirectionBoundaryIsOneLibreOfficeAlsoSplitsAt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<List<PdfTextRun>> lines = ReferenceLines();

        for (int i = 0; i < Paragraphs.Length; i++)
        {
            (string text, BidiDirection direction) = Paragraphs[i];
            List<PdfTextRun> portions = lines[i];

            portions.Sum(portion => portion.GlyphCount).ShouldBe(
                text.Length,
                $"paragraph {i + 1} (\"{text}\"): the reference's glyphs do not count its "
                + "characters, so something in it ligated and every offset after it is wrong");

            HashSet<int> reference = [.. Boundaries(portions)];
            List<int> mine = [.. BidiParagraph.Resolve(text, direction).Runs
                .Select(run => run.Start)];

            foreach (int boundary in mine)
            {
                reference.ShouldContain(
                    boundary,
                    $"paragraph {i + 1} (\"{text}\"): a level run starts at {boundary}, "
                    + $"where LibreOffice splits at {string.Join(", ", reference.Order())}");
            }
        }
    }

    [Fact]
    public void TheRunsAreDrawnInTheOrderTheReorderingPutsThem()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<List<PdfTextRun>> lines = ReferenceLines();
        int reordered = 0;

        for (int i = 0; i < Paragraphs.Length; i++)
        {
            (string text, BidiDirection direction) = Paragraphs[i];
            List<PdfTextRun> portions = lines[i];
            List<int> starts = Boundaries(portions);

            BidiParagraph bidi = BidiParagraph.Resolve(text, direction);
            IReadOnlyList<BidiRun> visual = bidi.InVisualOrder();

            if (!visual.Select(run => run.Start).SequenceEqual(bidi.Runs.Select(run => run.Start)))
            {
                reordered++;
            }

            double previous = double.NegativeInfinity;

            foreach (BidiRun run in visual)
            {
                // The leftmost pen of the portions this run covers. A run split by Writer into
                // several portions has each of them positioned separately, and for a right-to-left
                // run those pens descend — so the run's own left edge is the smallest of them.
                double left = portions
                    .Where((_, at) => starts[at] < run.End && run.Start < End(starts, portions, at))
                    .Min(portion => portion.X);

                left.ShouldBeGreaterThan(
                    previous,
                    $"paragraph {i + 1} (\"{text}\"): the run at {run.Start} is drawn at {left}, "
                    + $"which is not to the right of the run before it in visual order");

                previous = left;
            }
        }

        // The document is only evidence if some of it actually reorders, and fewer of its paragraphs
        // do than mix directions: a single right-to-left run between two left-to-right ones reverses
        // in place and leaves the run order alone. Five of the nine genuinely move a run, and
        // asserting the count keeps an edit that flattened the document from leaving this test
        // quietly checking nothing.
        reordered.ShouldBe(5, "the wrong number of paragraphs reorder to be worth comparing");
    }

    /// <summary>The reference PDF's portions, grouped into lines by their baseline.</summary>
    /// <remarks>
    /// Every paragraph in the document is short enough for one line, so a line is a paragraph. The
    /// portions of a line are consecutive in the content stream because LibreOffice draws them in
    /// reading order, which is what lets this group by "the baseline changed" rather than by sorting.
    /// </remarks>
    private List<List<PdfTextRun>> ReferenceLines()
    {
        string path = Corpus.Require("bidi.fodt");
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

        lines.Count.ShouldBe(
            Paragraphs.Length,
            "the reference has a different number of lines than the document has paragraphs");

        return lines;
    }

    /// <summary>Where each portion starts, as a character offset, from the glyph counts before it.</summary>
    private static List<int> Boundaries(List<PdfTextRun> portions)
    {
        List<int> starts = [];
        int at = 0;

        foreach (PdfTextRun portion in portions)
        {
            starts.Add(at);
            at += portion.GlyphCount;
        }

        return starts;
    }

    /// <summary>One past the last character of a portion.</summary>
    private static int End(List<int> starts, List<PdfTextRun> portions, int at)
        => starts[at] + portions[at].GlyphCount;
}

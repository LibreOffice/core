using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.Text.Fonts;
using Paperless.Text.Itemisation;
using Paperless.Text.Layout;
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
/// Three things are asserted. First, every boundary between
/// two embedding levels must be one of LibreOffice's portion boundaries — the converse does not
/// hold, and deliberately: Writer also splits a portion at a change of <em>font</em> script, and its
/// font scripts are the three coarse classes <c>GetScriptClass</c> assigns
/// (<c>i18nutil/source/utility/scriptclass.cxx</c>), not the UAX #24 scripts a shaper is told about.
/// A space is <c>WEAK</c> there and an opening parenthesis is <c>LATIN</c>, so "‏שלום (abc) עולם"
/// gets a Writer boundary at the bracket that no shaper-level itemisation has. Second, the pen of
/// the leftmost portion covering each sub-run must rise strictly along the order the reordering
/// puts them in, which is rule L2 measured rather than argued. Third, the gap between one portion's
/// pen and the next one to its right is that portion's width — the shaping of the sub-run itself,
/// and the only assertion here that would notice a run given the right boundaries and the wrong
/// face.
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
    /// <summary>How far a measured width may differ from LibreOffice's, in points.</summary>
    /// <remarks>
    /// A tenth of a point — two twips — for the same reason the mixed-run comparison uses it: both
    /// sides here are pen positions rather than ink, and LibreOffice lays out in whole twips, so the
    /// floor is the rounding rather than the measurement.
    /// </remarks>
    private const double TolerancePoints = 0.1;

    /// <summary>How far a measured sub-run width may differ from the width LibreOffice drew, in points.</summary>
    /// <remarks>
    /// A quarter of a point, which is looser than the pen comparisons elsewhere and is a measurement
    /// rather than a guess. Over the twenty-two portions of this document the largest difference is
    /// 0.221 pt, on the six characters of "Start " — VCL lays each portion out through its reference
    /// device and rounds, and the result differs from scaling the design values by a few twips per
    /// portion. It does not accumulate: each portion's pen is absolute, so the error is per portion
    /// and bounded, which is why the *boundary* and *ordering* assertions above can be exact while
    /// this one cannot.
    ///
    /// What this catches is not a twip. It is a sub-run shaped with the wrong face, script or
    /// direction, and that is wrong by points — when this document's right-to-left paragraphs were
    /// silently falling back to FreeSans and Liberation Serif, before their style spelled its font
    /// out instead of inheriting it, the same portions were out by 5.1 pt.
    /// </remarks>
    private const double WidthTolerancePoints = 0.25;

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

    /// <summary>The face the document is set in, or null when it is not installed.</summary>
    private static OpenTypeFace? DejaVuSans()
    {
        foreach (string directory in (string[])["/usr/share/fonts/truetype/dejavu", "/usr/share/fonts"])
        {
            if (!Directory.Exists(directory)) continue;

            string[] found = Directory.GetFiles(directory, "DejaVuSans.ttf", SearchOption.AllDirectories);
            if (found.Length > 0) return OpenTypeFace.ReadFile(found[0]);
        }

        return null;
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

    [Fact]
    public void EverySubRunIsTheWidthLibreOfficeDrewIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        OpenTypeFace? face = DejaVuSans();
        Assert.SkipWhen(face is null, "DejaVu Sans is not installed; see the corpus README");

        List<List<PdfTextRun>> lines = ReferenceLines();
        Length emSize = Length.FromPoints(12);
        int compared = 0;

        for (int i = 0; i < Paragraphs.Length; i++)
        {
            (string text, BidiDirection direction) = Paragraphs[i];

            MeasuredParagraph measured = MeasuredParagraph.Measure(
                text,
                [new FormattedRun(0, text.Length, face!, emSize)],
                shaper: null,
                new ItemisationOptions { BaseDirection = direction });

            List<int> starts = Boundaries(lines[i]);

            // Sorted by pen, which for a tiled line is left-to-right visual order — so the gap to
            // the next portion is the width of this one, whichever direction it reads in.
            List<int> byPen = [.. Enumerable.Range(0, lines[i].Count)
                .OrderBy(at => lines[i][at].X)];

            for (int k = 0; k + 1 < byPen.Count; k++)
            {
                int at = byPen[k];
                double drawn = lines[i][byPen[k + 1]].X - lines[i][at].X;
                double mine = measured
                    .WidthBetween(starts[at], starts[at] + lines[i][at].GlyphCount)
                    .Points;

                mine.ShouldBe(
                    drawn,
                    WidthTolerancePoints,
                    $"paragraph {i + 1} (\"{text}\"): the portion at {starts[at]} is "
                    + $"{drawn:F2} pt wide in the reference and {mine:F2} pt here");
                compared++;
            }
        }

        // Only the last portion of each line has no successor to measure against, so a nine-line
        // document with twenty-eight portions leaves nineteen gaps. Asserting the count keeps a
        // reference that lost its portions from leaving this test measuring nothing.
        compared.ShouldBeGreaterThanOrEqualTo(15, "too few portions to compare widths across");
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

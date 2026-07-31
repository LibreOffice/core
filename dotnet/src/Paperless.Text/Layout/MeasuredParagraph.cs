using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;

namespace Paperless.Text.Layout;

/// <summary>
/// One run of a paragraph: a range of its text with its own face and size.
/// </summary>
/// <remarks>
/// <para>
/// Runs partition a paragraph's text and never overlap, which is what lets a prefix width be the sum of
/// whole runs plus part of one. Formatting that <em>does</em> overlap — a bold range crossing a hyperlink
/// — is resolved into non-overlapping runs before it reaches here, because measurement needs one answer
/// per character and the document model's interval tags do not give one.
/// </para>
/// <para>
/// Only what changes a measurement is here. A run's colour and its underline do not move a line break, so
/// they travel with whatever draws it rather than with this.
/// </para>
/// </remarks>
/// <param name="Start">The run's first character, as an index into the paragraph's text.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="Face">The face it is set in.</param>
/// <param name="EmSize">The em size it is set at.</param>
/// <param name="Shaping">How it is shaped.</param>
public readonly record struct FormattedRun(
    int Start,
    int Length,
    OpenTypeFace Face,
    Length EmSize,
    ShapingOptions Shaping = default)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;

    /// <summary>True when the run covers a character.</summary>
    public bool Covers(int index) => index >= Start && index < End;
}

/// <summary>
/// A shaped run, positioned within its paragraph.
/// </summary>
/// <param name="Run">The run's own range and formatting.</param>
/// <param name="Shaped">Its glyphs and prefix widths, in its own face's design units.</param>
/// <param name="Metrics">The line metrics its face resolves to.</param>
public readonly record struct MeasuredRun(
    FormattedRun Run,
    ShapedText Shaped,
    LineMetrics Metrics);

/// <summary>
/// A paragraph measured across its runs.
/// </summary>
/// <remarks>
/// <para>
/// The thing a mixed-formatting paragraph needs that a single-face one does not: widths summed in
/// <em>EMUs</em> rather than in design units. Two runs at different sizes, or in faces with different
/// design grids, have no common unit smaller than the document's — so each run is scaled once from its
/// own grid and the results added, which is the only order that is both exact per run and comparable
/// across runs.
/// </para>
/// <para>
/// The line height comes from the runs on the line, not from the paragraph: a 24 pt word in an 11 pt
/// paragraph makes its own line taller and leaves the others alone. That is why the metrics travel with
/// each run rather than being resolved once, and why <see cref="HeightOf"/> takes a range.
/// </para>
/// </remarks>
public sealed class MeasuredParagraph
{
    private readonly MeasuredRun[] _runs;
    private readonly long[] _prefixEmu;

    private MeasuredParagraph(string text, MeasuredRun[] runs, long[] prefixEmu)
    {
        Text = text;
        _runs = runs;
        _prefixEmu = prefixEmu;
    }

    /// <summary>The paragraph's text.</summary>
    public string Text { get; }

    /// <summary>The runs, in order, partitioning the text.</summary>
    public IReadOnlyList<MeasuredRun> Runs => _runs;

    /// <summary>
    /// Shapes a paragraph's runs and builds its prefix widths.
    /// </summary>
    /// <remarks>
    /// Runs are sorted and clamped to the text rather than trusted: they come from a document, and one
    /// that overlaps or runs past the end would otherwise corrupt every width after it. A gap between two
    /// runs is filled from the run before it, since a character with no formatting is set in whatever
    /// preceded it.
    /// </remarks>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="runs">Its runs; may be empty, in which case the paragraph measures as nothing.</param>
    /// <param name="shaper">The shaper to use, or null for the default.</param>
    public static MeasuredParagraph Measure(
        string text, IReadOnlyList<FormattedRun> runs, ITextShaper? shaper = null)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(runs);

        ITextShaper engine = shaper ?? TextShaper.Default;
        List<MeasuredRun> measured = [];
        long[] prefix = new long[text.Length + 1];

        foreach (FormattedRun run in Normalise(text, runs))
        {
            ShapedText shaped = engine.Shape(
                run.Face, text.AsSpan(run.Start, run.Length), run.Shaping);

            measured.Add(new MeasuredRun(run, shaped, LineSpacing.Resolve(run.Face)));

            // Each run's own prefix widths, scaled from its own grid into EMUs, laid into the paragraph's
            // table at its own offset. Summing in design units instead would add numbers from two
            // different grids.
            for (int i = 1; i <= run.Length; i++)
            {
                prefix[run.Start + i] =
                    prefix[run.Start] + shaped.WidthUpTo(i, run.EmSize).Emu;
            }
        }

        // Any position no run covered — a text with no runs at all, or a trailing gap — carries the width
        // of the last position that was covered, so the table stays monotonic.
        for (int i = 1; i <= text.Length; i++)
        {
            if (prefix[i] < prefix[i - 1]) prefix[i] = prefix[i - 1];
        }

        return new MeasuredParagraph(text, [.. measured], prefix);
    }

    /// <summary>The width of the characters between two indices.</summary>
    public Length WidthBetween(int start, int end)
        => Length.FromEmu(At(end) - At(start));

    /// <summary>The whole paragraph's width.</summary>
    public Length Width => Length.FromEmu(_prefixEmu[^1]);

    /// <summary>
    /// The natural line height and ascent for a range of the text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The maxima across the runs the range touches, taken three ways rather than one: the tallest run's
    /// height, the largest ascent, and the largest descent, with the height then raised to hold the ascent
    /// and descent if together they exceed it. That is <c>SwLineLayout::CalcLine</c>
    /// (<c>sw/source/core/text/porlay.cxx</c>), which accumulates <c>Height()</c>, <c>mnAscent</c> and
    /// <c>nMaxDescent</c> independently and finishes with
    /// <c>if (nMaxDescent &gt; Height() - mnAscent) Height(nMaxDescent + mnAscent)</c>.
    /// </para>
    /// <para>
    /// Taking the tallest run's ascent instead would agree whenever one run is tallest in every direction,
    /// which is the case for two sizes of one family — and disagree the moment a line mixes families,
    /// since a face with a deep descent can own the descent while a different face owns the ascent.
    /// </para>
    /// <para>
    /// An empty range takes the run that would contain it, so an empty line is still as tall as the text
    /// that would go on it.
    /// </para>
    /// </remarks>
    public (Length Height, Length Ascent) HeightOf(int start, int end)
    {
        Length height = Length.Zero;
        Length ascent = Length.Zero;
        Length descent = Length.Zero;

        foreach (MeasuredRun run in _runs)
        {
            bool touches = run.Run.Start < end && start < run.Run.End;
            bool contains = start == end && run.Run.Covers(start);
            if (!touches && !contains) continue;

            Accumulate(run, ref height, ref ascent, ref descent);
        }

        // No run at all, which happens for an empty paragraph. The first run's metrics are the
        // paragraph's own, and a paragraph with no runs has nothing to be tall for.
        if (height == Length.Zero && _runs.Length > 0)
        {
            Accumulate(_runs[0], ref height, ref ascent, ref descent);
        }

        return (Length.Max(height, ascent + descent), ascent);
    }

    /// <summary>Folds one run into the running maxima a line's height is built from.</summary>
    private static void Accumulate(
        MeasuredRun run, ref Length height, ref Length ascent, ref Length descent)
    {
        // Twips throughout, because Writer lays out in whole twips and a fraction kept here would
        // eventually move a line onto a different page.
        Length runHeight = Length.FromTwips(run.Metrics.ScaledLineHeight(run.Run.EmSize).Twips);
        Length runAscent = Length.FromTwips(run.Metrics.ScaledAscent(run.Run.EmSize).Twips);

        height = Length.Max(height, runHeight);
        ascent = Length.Max(ascent, runAscent);
        descent = Length.Max(descent, runHeight - runAscent);
    }

    /// <summary>
    /// The runs overlapping a range, clipped to it.
    /// </summary>
    /// <remarks>
    /// What drawing a line needs: one glyph run per formatting change on the line, each with the
    /// characters that actually fall on it. A run spanning a line break is clipped at the break, which is
    /// why this returns ranges rather than the runs themselves.
    /// </remarks>
    public List<FormattedRun> RunsBetween(int start, int end)
    {
        List<FormattedRun> clipped = [];

        foreach (MeasuredRun run in _runs)
        {
            int from = Math.Max(run.Run.Start, start);
            int to = Math.Min(run.Run.End, end);
            if (to <= from) continue;

            clipped.Add(run.Run with { Start = from, Length = to - from });
        }

        return clipped;
    }

    private long At(int index) => _prefixEmu[Math.Clamp(index, 0, _prefixEmu.Length - 1)];

    /// <summary>
    /// Sorts the runs, clips them to the text, and fills the gaps between them.
    /// </summary>
    /// <remarks>
    /// A document can state runs out of order, overlapping, or past the end of the text it is describing.
    /// Each is repaired rather than rejected: an overlap is resolved in favour of the earlier run, since
    /// something has to win and the first is the one the document mentioned first, and a gap is filled
    /// from the run before it because a character with no formatting of its own inherits what preceded it.
    /// </remarks>
    private static List<FormattedRun> Normalise(string text, IReadOnlyList<FormattedRun> runs)
    {
        List<FormattedRun> sorted = [.. runs
            .Where(run => run.Length > 0 && run.Start < text.Length)
            .OrderBy(run => run.Start)];

        List<FormattedRun> result = [];
        int at = 0;

        foreach (FormattedRun run in sorted)
        {
            int start = Math.Max(run.Start, at);
            int end = Math.Min(run.End, text.Length);
            if (end <= start) continue;

            // A gap before this run belongs to whatever came before it, or to this run when nothing did.
            if (start > at)
            {
                FormattedRun filler = result.Count > 0 ? result[^1] : run;
                result.Add(filler with { Start = at, Length = start - at });
            }

            result.Add(run with { Start = start, Length = end - start });
            at = end;
        }

        if (at < text.Length && result.Count > 0)
        {
            result.Add(result[^1] with { Start = at, Length = text.Length - at });
        }

        return result;
    }
}

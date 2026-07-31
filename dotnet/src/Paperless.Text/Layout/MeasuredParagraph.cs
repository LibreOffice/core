using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Itemisation;
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
    private readonly TextItem[] _items;

    private MeasuredParagraph(
        string text, MeasuredRun[] runs, long[] prefixEmu, TextItem[] items, byte paragraphLevel)
    {
        Text = text;
        _runs = runs;
        _prefixEmu = prefixEmu;
        _items = items;
        ParagraphLevel = paragraphLevel;
    }

    /// <summary>The paragraph's text.</summary>
    public string Text { get; }

    /// <summary>
    /// The runs, in logical order, one per formatting change and per direction, script or face change
    /// within it.
    /// </summary>
    /// <remarks>
    /// Logical rather than visual, which is what Writer's own line portions are: its PDF export emits
    /// them in the order the characters are stored and positions each with an absolute pen. Drawing
    /// right-to-left text needs the visual order, and <see cref="Items"/> carries the levels it is
    /// derived from.
    /// </remarks>
    public IReadOnlyList<MeasuredRun> Runs => _runs;

    /// <summary>
    /// The direction and script sub-runs the paragraph was cut into, in logical order.
    /// </summary>
    /// <remarks>
    /// One entry per stretch of one embedding level and one script — the same partition the runs
    /// were shaped against, minus the formatting changes. A caller drawing the paragraph needs these
    /// to put the runs in visual order; <see cref="TextItemiser.InVisualOrder"/> does that.
    /// </remarks>
    public IReadOnlyList<TextItem> Items => _items;

    /// <summary>The paragraph's own embedding level: even for left to right, odd for right to left.</summary>
    public byte ParagraphLevel { get; }

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
    /// <param name="itemisation">
    /// How to cut the paragraph into sub-runs before shaping, or null for the neutral settings: left
    /// to right, and no glyph fallback. A paragraph of Latin prose is cut into one sub-run per
    /// formatting run either way, and shaped in exactly the calls it was shaped in before sub-runs
    /// existed.
    /// </param>
    public static MeasuredParagraph Measure(
        string text,
        IReadOnlyList<FormattedRun> runs,
        ITextShaper? shaper = null,
        ItemisationOptions? itemisation = null)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(runs);

        ITextShaper engine = shaper ?? TextShaper.Default;
        List<FormattedRun> formatted = Normalise(text, runs);
        ItemisationOptions options = itemisation ?? DefaultItemisation(formatted);

        // The bidi algorithm is resolved over the whole paragraph, never per formatting run: a run
        // boundary is a change of font, not a change of direction, and resolving each separately
        // would let a bold word inside a Hebrew sentence see the paragraph's direction instead of
        // the sentence's.
        BidiParagraph bidi = BidiParagraph.Resolve(text, options.BaseDirection);
        List<TextItem> items = TextItemiser.Itemise(text, bidi);

        List<MeasuredRun> measured = [];
        long[] prefix = new long[text.Length + 1];
        long running = 0;

        foreach (FormattedRun run in formatted)
        {
            foreach (FormattedRun part in SubRuns(text, run, items, options))
            {
                ShapedText shaped = engine.Shape(
                    part.Face, text.AsSpan(part.Start, part.Length), part.Shaping);

                measured.Add(new MeasuredRun(part, shaped, LineSpacing.Resolve(part.Face)));

                // Each sub-run's own prefix widths, scaled from its own grid into EMUs and added to
                // the running total. Summing in design units instead would add numbers from two
                // different grids; reading the running total off the table instead would break the
                // moment a control character left a gap between two sub-runs.
                for (int i = 1; i <= part.Length; i++)
                {
                    prefix[part.Start + i] = running + shaped.WidthUpTo(i, part.EmSize).Emu;
                }

                running += shaped.Width(part.EmSize).Emu;
            }
        }

        // Any position no sub-run covered — a text with no runs at all, a trailing gap, or a format
        // control character that was cut out — carries the width of the last position that was
        // covered, so the table stays monotonic and the control measures as nothing.
        for (int i = 1; i <= text.Length; i++)
        {
            if (prefix[i] < prefix[i - 1]) prefix[i] = prefix[i - 1];
        }

        return new MeasuredParagraph(
            text, [.. measured], prefix, [.. items], bidi.ParagraphLevel);
    }

    /// <summary>
    /// The itemisation to use when the caller stated none.
    /// </summary>
    /// <remarks>
    /// The base direction comes from the runs' own shaping options, so a caller that has been saying
    /// <c>RightToLeft</c> on every run of a Hebrew paragraph keeps getting a right-to-left paragraph.
    /// That was the only way to say it before this existed, and silently changing what it meant would
    /// re-align every such document.
    /// </remarks>
    private static ItemisationOptions DefaultItemisation(List<FormattedRun> runs)
        => runs.Count > 0 && runs[0].Shaping.RightToLeft
            ? new ItemisationOptions { BaseDirection = BidiDirection.RightToLeft }
            : ItemisationOptions.Default;

    /// <summary>
    /// Cuts one formatting run into the pieces a shaper can take.
    /// </summary>
    /// <remarks>
    /// The whole run unchanged is the common case and is returned untouched — not merely equivalent,
    /// but the same <see cref="ShapingOptions"/> the caller passed, so a paragraph of Latin prose
    /// reaches HarfBuzz in the identical call it did before any of this existed. Anything else is
    /// split at every change of direction, script or face, and each piece is told which it is.
    /// </remarks>
    private static List<FormattedRun> SubRuns(
        string text, FormattedRun run, List<TextItem> items, ItemisationOptions options)
    {
        List<FormattedRun> parts = [];

        foreach (TextItem item in items)
        {
            int start = Math.Max(item.Start, run.Start);
            int end = Math.Min(item.End, run.End);
            if (end <= start) continue;

            foreach (FaceRun face in FontItemiser.Split(
                         text, start, end - start, run.Face,
                         options.GlyphFallback, options.OnGlyphFallback))
            {
                bool wholeRun = face.Start == run.Start && face.End == run.End;
                bool plain = wholeRun && !item.IsRightToLeft && !face.IsFallback;

                parts.Add(run with
                {
                    Start = face.Start,
                    Length = face.Length,
                    Face = face.Face,
                    Shaping = plain
                        ? run.Shaping
                        : run.Shaping with
                        {
                            Script = item.Script,
                            RightToLeft = item.IsRightToLeft,
                        },
                });
            }
        }

        return parts;
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

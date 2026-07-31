using Paperless.Core.Graphics;

namespace Paperless.TestKit;

/// <summary>One word as a page drew it.</summary>
/// <param name="Left">Where its first glyph's pen was, in points from the page's left edge.</param>
/// <param name="Right">Where the pen ended, so the difference is the word's advance width.</param>
/// <param name="Baseline">Its baseline, in points from the page's top.</param>
/// <param name="Size">The em size it was drawn at, in points.</param>
/// <param name="Family">The resolved family it was drawn in.</param>
/// <param name="Text">The word.</param>
public readonly record struct DrawnWord(
    double Left,
    double Right,
    double Baseline,
    double Size,
    string Family,
    string Text);

/// <summary>
/// Recovers words from a recorded page's glyph runs.
/// </summary>
/// <remarks>
/// <para>
/// The unit a word-box comparison needs, which is neither the glyph run nor the line. It exists because
/// the strongest available check on justification is where every word sits: justification leaves the runs
/// and their origins exactly where they were and widens the blanks between the words, so a run-by-run
/// comparison sees nothing at all.
/// </para>
/// <para>
/// A run boundary deliberately does not end a word, so a formatting change in the middle of one still
/// yields the whole word. Three things do end one: a space, a change of baseline — a line's trailing space
/// is not drawn, so two consecutive lines' glyphs would otherwise run together — and a horizontal gap,
/// which is what a tab leaves behind, since a tab advances the pen without drawing anything.
/// </para>
/// </remarks>
public static class DrawnWords
{
    /// <summary>
    /// How far two glyphs may be apart, in points, and still belong to one word.
    /// </summary>
    /// <remarks>
    /// Half a point. A tab leaves a gap of at least a whole stop's worth, and the glyphs of a word are
    /// adjacent by construction — the pen leaves no gap between them at all — so anything between the two
    /// would do. Half a point is small enough to split a tab and large enough not to split a kerned pair,
    /// whose spacing is negative rather than positive anyway.
    /// </remarks>
    private const double GapEndingAWord = 0.5;

    /// <summary>The words on a recorded page, in the order they were drawn.</summary>
    public static List<DrawnWord> On(DrawnPage page)
    {
        ArgumentNullException.ThrowIfNull(page);

        List<Cell> cells = Cells(page);
        List<DrawnWord> words = [];

        int at = 0;
        while (at < cells.Count)
        {
            if (cells[at].Chars == " ") { at++; continue; }

            int start = at;
            double baseline = cells[at].Baseline;
            at++;

            while (at < cells.Count
                   && cells[at].Chars != " "
                   && cells[at].Baseline == baseline
                   && cells[at].Left - cells[at - 1].Right <= GapEndingAWord)
            {
                at++;
            }

            words.Add(new DrawnWord(
                cells[start].Left,
                cells[at - 1].Right,
                baseline,
                cells[start].Size,
                cells[start].Family,
                string.Concat(cells[start..at].Select(cell => cell.Chars))));
        }

        return words;
    }

    /// <summary>
    /// One glyph as drawn: where it sits, and which characters it stands for.
    /// </summary>
    /// <param name="Left">Its pen position.</param>
    /// <param name="Right">The pen after it, so the difference is its advance.</param>
    /// <param name="Baseline">Its run's baseline.</param>
    /// <param name="Size">Its run's em size.</param>
    /// <param name="Family">Its run's resolved family.</param>
    /// <param name="Chars">
    /// The characters it stands for — plural, because a glyph can be a ligature covering two of them, so
    /// recovering a word's text means asking each glyph what it spells rather than indexing the string.
    /// </param>
    private readonly record struct Cell(
        double Left, double Right, double Baseline, double Size, string Family, string Chars);

    private static List<Cell> Cells(DrawnPage page)
    {
        List<Cell> cells = [];

        foreach (DrawnGlyphRun run in page.Runs)
        {
            IReadOnlyList<PositionedGlyph> glyphs = run.Run.Glyphs;
            IReadOnlyList<int> clusters = run.Run.ClusterMap;

            for (int i = 0; i < glyphs.Count && i < clusters.Count; i++)
            {
                int from = clusters[i];
                int to = i + 1 < clusters.Count ? clusters[i + 1] : run.Text.Length;
                if (to <= from || to > run.Text.Length) continue;

                double left = (run.Origin.X + glyphs[i].Offset.X).Points;

                cells.Add(new Cell(
                    left,
                    left + glyphs[i].Advance.Points,
                    run.Origin.Y.Points,
                    run.Run.FontSize.Points,
                    run.Run.Font.FamilyName,
                    run.Text[from..to]));
            }
        }

        return cells;
    }
}

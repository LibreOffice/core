using Paperless.TestKit.LibreOffice;

namespace Paperless.TestKit;

/// <summary>
/// Puts words into reading order: down the page by line, then across each line.
/// </summary>
/// <remarks>
/// <para>
/// Sorting on the exact vertical looks like it would do this and does not, for one reason: a superscript sits
/// about two points above the line it belongs to, so its own vertical puts it <em>before</em> every word of
/// that line. A document laid out perfectly then fails a word-order comparison on the citation alone. Words
/// are grouped into lines with a tolerance first, and only ordered across the page within a line.
/// </para>
/// <para>
/// The tolerance is deliberately much larger than a rise and smaller than a line height, so the grouping
/// needs to know nothing about the document it is grouping.
/// </para>
/// </remarks>
public static class ReadingOrder
{
    /// <summary>
    /// How far two words' verticals may differ and still be one line, in points.
    /// </summary>
    /// <remarks>
    /// Six points: more than a superscript's rise at any ordinary size, and less than the line height of
    /// anything above about five point.
    /// </remarks>
    public const double SameLinePoints = 6;

    /// <summary>The words of a reference page, in reading order.</summary>
    public static List<PdfWord> Of(IReadOnlyList<PdfWord> words)
        => InLines(words, word => word.Top, word => word.Left);

    /// <summary>The words of a drawn page, in reading order.</summary>
    public static List<DrawnWord> Of(IReadOnlyList<DrawnWord> words)
        => InLines(words, word => word.Baseline, word => word.Left);

    /// <summary>
    /// The first word drawn at a given size, which is what a vertical comparison has to be anchored at.
    /// </summary>
    /// <remarks>
    /// A box's top sits above its baseline by the font's ascent, and the PDF never states that ascent — so a
    /// vertical comparison can only ever be of <em>differences</em>, and a difference only cancels the ascent
    /// between words of the same size. Measuring a 10 pt note line against an 11 pt body line leaves the
    /// difference of the two ascents behind: on the corpus documents here that is 0.95 pt, nearly ten times the
    /// tolerance, and it reads as a placement error while being nothing of the kind.
    /// </remarks>
    /// <param name="words">The page's words.</param>
    /// <param name="size">The size to anchor at, in points.</param>
    public static int FirstOfSize(IReadOnlyList<DrawnWord> words, double size)
    {
        ArgumentNullException.ThrowIfNull(words);

        for (int i = 0; i < words.Count; i++)
        {
            if (Math.Abs(words[i].Size - size) < 0.01) return i;
        }

        return 0;
    }

    private static List<T> InLines<T>(
        IReadOnlyList<T> words, Func<T, double> vertical, Func<T, double> horizontal)
    {
        ArgumentNullException.ThrowIfNull(words);

        List<List<T>> lines = [];

        foreach (T word in words.OrderBy(vertical))
        {
            if (lines.Count > 0 && vertical(word) - vertical(lines[^1][0]) <= SameLinePoints)
            {
                lines[^1].Add(word);
                continue;
            }

            lines.Add([word]);
        }

        return [.. lines.SelectMany(line => line.OrderBy(horizontal))];
    }
}

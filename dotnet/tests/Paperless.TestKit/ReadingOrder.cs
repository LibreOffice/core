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

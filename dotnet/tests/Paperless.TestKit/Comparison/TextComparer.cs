using System.Globalization;
using System.Text;

namespace Paperless.TestKit.Comparison;

/// <summary>
/// Compares extracted text against a LibreOffice reference extraction.
/// </summary>
/// <remarks>
/// <para>
/// The in-process counterpart of <c>.claude/skills/extraction-comparison/scripts/compare-text.py</c>,
/// and it normalises the same way for the same reasons — comparing raw bytes produces noise
/// that hides real bugs. Each step exists to remove a difference that is never meaningful,
/// and nothing is case-folded and no punctuation is stripped, because those would hide real
/// bugs rather than noise.
/// </para>
/// <para>
/// The reference filters are lossy and opinionated, so exact equality is the wrong assertion
/// for anything but the simplest document: the Writer text filter drops headers, footers,
/// comments, notes and shape text, renders a table one cell per line, and indents list items;
/// the CSV filter covers only the first sheet and drops number formatting. What is worth
/// asserting is that <em>nothing the reference found is missing</em>, which is what
/// <see cref="FindMissingTokens"/> is for.
/// </para>
/// </remarks>
public static class TextComparer
{
    /// <summary>
    /// Normalises text so that only meaningful differences remain.
    /// </summary>
    /// <param name="text">The text to normalise.</param>
    /// <param name="foldSpaces">
    /// Whether to fold a non-breaking space to an ordinary one. Off by default: NBSP versus
    /// space <em>is</em> a real difference and can be a genuine bug, so it is only folded when
    /// the caller has established it is not what they are chasing.
    /// </param>
    public static string Normalise(string text, bool foldSpaces = false)
    {
        ArgumentNullException.ThrowIfNull(text);

        // A BOM is an encoding artefact, not content; LibreOffice writes one.
        text = text.TrimStart('﻿');
        text = text.Replace("\r\n", "\n", StringComparison.Ordinal)
                   .Replace('\r', '\n');
        if (foldSpaces) text = text.Replace(' ', ' ');

        // NFC: the same character can be composed or decomposed, and the two are visually and
        // semantically identical.
        text = text.Normalize(NormalizationForm.FormC);

        StringBuilder result = new(text.Length);
        int blankRun = 0;
        foreach (string line in text.Split('\n'))
        {
            string trimmed = line.TrimEnd();
            if (trimmed.Length == 0)
            {
                // The filters emit inconsistent blank-line padding around blocks, so a run of
                // blanks collapses to one.
                blankRun++;
                continue;
            }
            if (blankRun > 0 && result.Length > 0) result.Append('\n');
            blankRun = 0;
            result.Append(trimmed).Append('\n');
        }
        return result.ToString();
    }

    /// <summary>
    /// Splits text into the tokens a comparison should consider.
    /// </summary>
    /// <remarks>
    /// Whitespace-delimited, with commas and tabs also treated as separators so that the CSV
    /// filter's <c>a,b,c</c> and Paperless's tab-separated row yield the same tokens. Tokens
    /// made entirely of punctuation are dropped: the reference filters substitute their own
    /// list markers — a hyphen for a bullet, a question mark for a character the export
    /// encoding cannot represent — and chasing those is chasing the filter, not a bug.
    /// </remarks>
    public static List<string> Tokenise(string text)
    {
        ArgumentNullException.ThrowIfNull(text);

        List<string> tokens = [];
        foreach (string raw in Normalise(text).Split(
                     [' ', '\t', '\n', ',', ';', ' '], StringSplitOptions.RemoveEmptyEntries))
        {
            bool hasContent = false;
            foreach (char character in raw)
            {
                if (char.IsLetterOrDigit(character)) { hasContent = true; break; }
            }
            if (hasContent) tokens.Add(raw);
        }
        return tokens;
    }

    /// <summary>
    /// The reference tokens that do not appear in the actual text, with their shortfall.
    /// </summary>
    /// <remarks>
    /// This is the assertion that means something: Paperless extracting <em>more</em> than the
    /// reference is usually correct, but anything the reference found and Paperless did not is
    /// content that went missing. Counts are compared, not just presence, so losing one of
    /// three occurrences of a word is still caught.
    /// </remarks>
    /// <returns>Each missing token with how many occurrences are unaccounted for.</returns>
    public static IReadOnlyList<(string Token, int Missing)> FindMissingTokens(
        string reference, string actual)
    {
        Dictionary<string, int> expected = Counts(Tokenise(reference));
        Dictionary<string, int> found = Counts(Tokenise(actual));

        List<(string, int)> missing = [];
        foreach ((string token, int count) in expected)
        {
            found.TryGetValue(token, out int actualCount);
            if (actualCount < count) missing.Add((token, count - actualCount));
        }
        return missing;

        static Dictionary<string, int> Counts(List<string> tokens)
        {
            Dictionary<string, int> counts = new(StringComparer.Ordinal);
            foreach (string token in tokens)
                counts[token] = counts.TryGetValue(token, out int existing) ? existing + 1 : 1;
            return counts;
        }
    }

    /// <summary>
    /// A token similarity between 0 and 1, as twice the longest common subsequence over the
    /// combined length.
    /// </summary>
    /// <remarks>
    /// Order-sensitive on purpose. "Right characters, wrong order" is a real failure mode — a
    /// table traversed column-major, shapes read in z-order instead of position order — and a
    /// set comparison would score it as perfect.
    /// </remarks>
    public static double Similarity(string reference, string actual)
    {
        List<string> left = Tokenise(reference);
        List<string> right = Tokenise(actual);
        if (left.Count == 0 && right.Count == 0) return 1.0;
        if (left.Count == 0 || right.Count == 0) return 0.0;

        // Two rows of the LCS table rather than the whole thing: a long document has tens of
        // thousands of tokens and the full table would be hundreds of megabytes.
        int[] previous = new int[right.Count + 1];
        int[] current = new int[right.Count + 1];

        for (int i = 1; i <= left.Count; i++)
        {
            for (int j = 1; j <= right.Count; j++)
            {
                current[j] = string.Equals(left[i - 1], right[j - 1], StringComparison.Ordinal)
                    ? previous[j - 1] + 1
                    : Math.Max(previous[j], current[j - 1]);
            }
            (previous, current) = (current, previous);
            Array.Clear(current);
        }

        return 2.0 * previous[right.Count] / (left.Count + right.Count);
    }

    /// <summary>
    /// A description of the first line where two texts diverge, or null when they agree.
    /// </summary>
    public static string? DescribeFirstDivergence(string reference, string actual)
    {
        string[] expected = Normalise(reference).Split('\n');
        string[] found = Normalise(actual).Split('\n');

        for (int line = 0; line < Math.Max(expected.Length, found.Length); line++)
        {
            string left = line < expected.Length ? expected[line] : "<end of text>";
            string right = line < found.Length ? found[line] : "<end of text>";
            if (string.Equals(left, right, StringComparison.Ordinal)) continue;

            return string.Create(CultureInfo.InvariantCulture,
                                 $"line {line + 1}:\n  reference: '{left}'\n  actual:    '{right}'");
        }
        return null;
    }
}

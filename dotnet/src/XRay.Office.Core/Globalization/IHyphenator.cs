namespace XRay.Office.Core.Globalization;

/// <summary>Splits words for hyphenation.</summary>
/// <remarks>
/// <para>
/// Optional: documents that do not enable automatic hyphenation never need it. When a
/// document does enable it, hyphenation patterns are language-specific and LibreOffice uses
/// Hunspell's, so matching its line breaks requires the same pattern files. See
/// <see cref="HyphenationPatterns"/> for the reader and <see cref="Hyphenators"/> for where the
/// files are found.
/// </para>
/// <para>
/// <strong>It lives beside <see cref="WindowsLanguages"/> rather than with the line breaker
/// because the chart layout needs it and <c>XRay.Office.Text</c> is downstream of
/// <c>XRay.Office.Core</c>.</strong> A category axis' rotate-or-wrap decision is a hyphenation
/// question before it is a width question — <c>probes/chart-hyph-r105</c> establishes that by
/// holding every character of a witness fixed and switching only the reference's dictionary off
/// — and <c>ChartAxisLabels</c> is in this assembly. Paragraph line breaking can consume the
/// same interface when it needs one, since <c>XRay.Office.Text</c> references this assembly.
/// </para>
/// </remarks>
public interface IHyphenator
{
    /// <summary>
    /// Returns the UTF-16 offsets within a word where a hyphen may be inserted, ascending.
    /// Returns an empty list when the word must not be hyphenated or no pattern file is
    /// available for the language.
    /// </summary>
    /// <remarks>
    /// An offset is the length of the part that stays on the line: offset <c>3</c> in
    /// <c>hyphenation</c> means <c>hy-</c> then <c>phenation</c>. Never <c>0</c> and never the
    /// word's own length, both of which are breaks at the word's edges rather than inside it.
    /// </remarks>
    /// <param name="word">The word to break, without surrounding whitespace.</param>
    /// <param name="language">A BCP 47 tag. Ignored by a single-language pattern set.</param>
    IReadOnlyList<int> FindHyphenationPoints(ReadOnlySpan<char> word, string language);
}

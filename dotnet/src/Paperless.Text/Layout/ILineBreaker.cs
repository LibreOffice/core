namespace Paperless.Text.Layout;

/// <summary>
/// Finds the positions in a string where a line may be broken, per Unicode UAX #14.
/// </summary>
/// <remarks>
/// LibreOffice delegates this to ICU's rule-based break iterator, and its line breaks
/// are therefore ICU's. Anything that does not implement the same rules will disagree
/// on CJK text, on hyphens and dashes, and around punctuation — and because a single
/// differing break reflows everything after it, these disagreements compound down the
/// page rather than staying local. See <c>dotnet/research/05-infrastructure.md</c>
/// section F.
/// </remarks>
public interface ILineBreaker
{
    /// <summary>
    /// Returns the UTF-16 indices at which a break is permitted, in ascending order.
    /// The end of the text is always a permitted break.
    /// </summary>
    /// <param name="text">The text to analyse.</param>
    /// <param name="language">A BCP 47 tag; some rules are language-specific.</param>
    IReadOnlyList<int> FindBreakOpportunities(ReadOnlySpan<char> text, string? language = null);

    /// <summary>
    /// Returns the UTF-16 indices at which a break is <em>required</em>, in ascending order.
    /// </summary>
    /// <remarks>
    /// A subset of <see cref="FindBreakOpportunities"/>, and the distinction is not cosmetic: an
    /// opportunity is somewhere a line <em>may</em> end, and a filler that treats them all alike will
    /// happily run a manual line break through the middle of a line because the text still fits. UAX #14
    /// calls these BK, CR, LF and NL; a word processor's manual line break is one of them.
    /// </remarks>
    /// <param name="text">The text to analyse.</param>
    /// <param name="language">A BCP 47 tag; some rules are language-specific.</param>
    IReadOnlyList<int> FindMandatoryBreaks(ReadOnlySpan<char> text, string? language = null);
}

/// <summary>Splits words for hyphenation.</summary>
/// <remarks>
/// Optional: documents that do not enable automatic hyphenation never need it. When a
/// document does enable it, hyphenation dictionaries are language-specific and
/// LibreOffice uses Hunspell's, so matching its line breaks requires the same
/// dictionaries.
/// </remarks>
public interface IHyphenator
{
    /// <summary>
    /// Returns the UTF-16 offsets within a word where a hyphen may be inserted.
    /// Returns an empty list when the word must not be hyphenated or no dictionary is
    /// available for the language.
    /// </summary>
    IReadOnlyList<int> FindHyphenationPoints(ReadOnlySpan<char> word, string language);
}

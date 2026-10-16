namespace XRay.Office.Text.Layout;

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

// IHyphenator moved to XRay.Office.Core.Globalization.IHyphenator, and this note is the pointer
// rather than a duplicate declaration. The chart category axis is the first consumer and
// ChartAxisLabels lives in XRay.Office.Core, which this assembly is downstream of; keeping the
// interface here would have meant XRay.Office.Core could not name it. Paragraph line breaking
// reaches it through the project reference when it needs one — see
// XRay.Office.Core/Globalization/IHyphenator.cs and HyphenationPatterns.cs.

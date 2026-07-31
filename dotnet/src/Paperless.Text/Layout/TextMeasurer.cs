using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;

namespace Paperless.Text.Layout;

/// <summary>
/// Measures text by shaping it.
/// </summary>
/// <remarks>
/// <para>
/// The sum of the glyphs' advances, scaled once from the face's design grid to the requested em size.
/// That is what decides where a line runs past the margin, so it decides where the line breaks — and a
/// break in a different place moves everything after it, so this measurement compounds down the page
/// rather than staying local.
/// </para>
/// <para>
/// Measuring means shaping, not adding up the font's advance widths. The difference is kerning and
/// ligatures, and it is larger than it sounds: a line of English prose at 12 pt in Carlito accumulates
/// about a quarter of an em of kerning, roughly three points, which is enough to decide whether the
/// line's last word fits. Skipping it does not produce slightly-different-looking output — it produces
/// a different set of line breaks from Writer's, and then a different pagination.
/// </para>
/// <para>
/// Callers measuring the same text repeatedly — which is what filling lines does — should shape once
/// with <see cref="Shape"/> and ask the result for prefix widths, rather than calling
/// <see cref="Measure"/> per candidate. Shaping is contextual as well as expensive, so measuring a
/// prefix on its own is both slower and, for a face with contextual features, not quite the same
/// number.
/// </para>
/// </remarks>
public sealed class TextMeasurer
{
    private readonly OpenTypeFace _face;
    private readonly ITextShaper _shaper;

    /// <summary>Creates a measurer over a face, shaping with the default shaper.</summary>
    public TextMeasurer(OpenTypeFace face)
        : this(face, TextShaper.Default)
    {
    }

    /// <summary>Creates a measurer over a face and a shaper.</summary>
    public TextMeasurer(OpenTypeFace face, ITextShaper shaper)
    {
        ArgumentNullException.ThrowIfNull(face);
        ArgumentNullException.ThrowIfNull(shaper);
        _face = face;
        _shaper = shaper;
    }

    /// <summary>The face being measured with.</summary>
    public OpenTypeFace Face => _face;

    /// <summary>The shaper being measured with.</summary>
    public ITextShaper Shaper => _shaper;

    /// <summary>Shapes a run, giving something that can answer for every prefix of it.</summary>
    public ShapedText Shape(ReadOnlySpan<char> text, ShapingOptions options = default)
        => _shaper.Shape(_face, text, options);

    /// <summary>The width of a run of text at an em size.</summary>
    public Length Measure(ReadOnlySpan<char> text, Length emSize, ShapingOptions options = default)
        => Shape(text, options).Width(emSize);

    /// <summary>The width of a run of text in the face's design units.</summary>
    public long MeasureInDesignUnits(ReadOnlySpan<char> text, ShapingOptions options = default)
        => Shape(text, options).AdvanceInDesignUnits;

    /// <summary>Scales a measurement in design units to an em size.</summary>
    public Length Scale(long designUnits, Length emSize)
        => _face.UnitsPerEm <= 0
            ? Length.Zero
            : Length.FromEmu((long)Math.Round((double)designUnits * emSize.Emu / _face.UnitsPerEm));
}

/// <summary>One line of a laid-out paragraph.</summary>
/// <param name="Start">The line's first character, as an index into the paragraph's text.</param>
/// <param name="End">One past its last character, trailing spaces included.</param>
/// <param name="VisibleEnd">
/// One past its last character excluding the trailing spaces. The two differ because a line's trailing
/// spaces are not part of its width — they hang past the margin rather than pushing a word to the next
/// line, which is why a paragraph of "a b c" does not break after every space.
/// </param>
/// <param name="Width">The width of the line's visible text.</param>
/// <param name="EndsParagraph">True when this is the paragraph's last line.</param>
public readonly record struct TextLine(
    int Start,
    int End,
    int VisibleEnd,
    Length Width,
    bool EndsParagraph)
{
    /// <summary>How many characters the line spans, trailing spaces included.</summary>
    public int Length => End - Start;

    /// <summary>The line's text, given the paragraph's.</summary>
    public ReadOnlySpan<char> TextIn(ReadOnlySpan<char> paragraph)
        => paragraph[Start..Math.Min(End, paragraph.Length)];

    /// <summary>The line's visible text, without the trailing spaces.</summary>
    public ReadOnlySpan<char> VisibleTextIn(ReadOnlySpan<char> paragraph)
        => paragraph[Start..Math.Min(VisibleEnd, paragraph.Length)];
}

/// <summary>
/// Fills lines to a width, the way a word processor does.
/// </summary>
/// <remarks>
/// <para>
/// Greedy, one line at a time: take break opportunities until the next one would overflow, then break
/// at the last that fit. That is what Writer does, and it is deliberately <em>not</em> Knuth-Plass —
/// the total-fit algorithm produces better-looking paragraphs and different line breaks, and different
/// line breaks are precisely what must not happen when the point is to agree with LibreOffice.
/// </para>
/// <para>
/// Two details decide more breaks than the measurement does. A line's trailing spaces do not count
/// towards its width, so a space that would overflow the margin does not push its word to the next
/// line. And a single word too long for the line is not left to overflow forever: it takes the line
/// alone and is allowed to exceed it, because the alternative is an empty line followed by the same
/// problem.
/// </para>
/// <para>
/// The paragraph is shaped once and every candidate width read off the result. Shaping each candidate
/// would be quadratic in the paragraph's length, and would also answer slightly differently, since a
/// prefix shaped alone is not always a prefix of the shaped whole.
/// </para>
/// </remarks>
public sealed class LineFiller
{
    private readonly TextMeasurer _measurer;
    private readonly ILineBreaker _breaker;

    /// <summary>Creates a filler over a measurer and a break iterator.</summary>
    public LineFiller(TextMeasurer measurer, ILineBreaker? breaker = null)
    {
        ArgumentNullException.ThrowIfNull(measurer);
        _measurer = measurer;
        _breaker = breaker ?? LineBreaker.Instance;
    }

    /// <summary>
    /// Breaks a paragraph measured across its runs into lines that fit a width.
    /// </summary>
    /// <remarks>
    /// The same greedy walk as the single-face overload, differing only in where a candidate's width comes
    /// from — a <see cref="MeasuredParagraph"/> answers across runs where a <see cref="TextMeasurer"/>
    /// answers for one face. Keeping one algorithm matters more than the sharing: two fillers would break
    /// lines differently the first time one was changed.
    /// </remarks>
    /// <param name="measured">The paragraph, already shaped run by run.</param>
    /// <param name="availableWidth">The width a line has to fit in.</param>
    /// <param name="firstLineWidth">The width available to the first line, when it differs.</param>
    /// <param name="language">A BCP 47 tag, for the language-specific break rules.</param>
    /// <param name="tabs">
    /// The paragraph's tab stops, or null when it has none. Only consulted for a line that holds a tab,
    /// so a paragraph without one measures exactly as it would without this parameter.
    /// </param>
    public List<TextLine> Fill(
        MeasuredParagraph measured,
        Length availableWidth,
        Length? firstLineWidth = null,
        string? language = null,
        ParagraphFormat? tabs = null)
    {
        ArgumentNullException.ThrowIfNull(measured);

        return Fill(
            measured.Text,
            availableWidth,
            firstLineWidth,
            language,
            measured.WidthBetween,
            tabs);
    }

    /// <summary>
    /// Breaks a paragraph into lines that fit a width.
    /// </summary>
    /// <param name="text">The paragraph's text, without its terminating mark.</param>
    /// <param name="emSize">The em size the text is set at.</param>
    /// <param name="availableWidth">The width a line has to fit in.</param>
    /// <param name="firstLineWidth">
    /// The width available to the first line, when it differs — a first-line indent narrows it and a
    /// hanging indent widens it. Defaults to <paramref name="availableWidth"/>.
    /// </param>
    /// <param name="language">A BCP 47 tag, for the language-specific break rules.</param>
    /// <param name="options">
    /// How to shape. The default is what Writer does, so passing nothing gives Writer's line breaks.
    /// </param>
    /// <param name="tabs">
    /// The paragraph's tab stops, or null when it has none. Only consulted for a line that holds a tab,
    /// so a paragraph without one measures exactly as it would without this parameter.
    /// </param>
    public List<TextLine> Fill(
        string text,
        Length emSize,
        Length availableWidth,
        Length? firstLineWidth = null,
        string? language = null,
        ShapingOptions? options = null,
        ParagraphFormat? tabs = null)
    {
        ArgumentNullException.ThrowIfNull(text);

        ShapingOptions shaping = options ?? new ShapingOptions(Language: language);
        ShapedText shaped = text.Length == 0
            ? ShapedText.Empty
            : _measurer.Shape(text, shaping);

        return Fill(
            text,
            availableWidth,
            firstLineWidth,
            language,
            (from, to) => shaped.WidthBetween(from, to, emSize),
            tabs);
    }

    /// <summary>
    /// The fill loop itself, over any way of measuring a range.
    /// </summary>
    /// <remarks>
    /// Taking the measurement as a function is what keeps one algorithm serving both the single-face and
    /// the mixed-run cases. The alternative — two loops — would break lines differently the first time
    /// either was touched, and the whole point of this code is that it breaks them where Writer does.
    /// </remarks>
    private List<TextLine> Fill(
        string text,
        Length availableWidth,
        Length? firstLineWidth,
        string? language,
        Func<int, int, Length> widthBetween,
        ParagraphFormat? tabs = null)
    {
        List<TextLine> lines = [];
        if (text.Length == 0)
        {
            lines.Add(new TextLine(0, 0, 0, Length.Zero, EndsParagraph: true));
            return lines;
        }

        IReadOnlyList<int> opportunities = _breaker.FindBreakOpportunities(text, language);
        HashSet<int> mandatory = [.. _breaker.FindMandatoryBreaks(text, language)];

        int lineStart = 0;
        int nextOpportunity = 0;

        while (lineStart < text.Length)
        {
            Length limit = lines.Count == 0 ? firstLineWidth ?? availableWidth : availableWidth;

            int chosen = -1;
            Length chosenWidth = Length.Zero;
            int chosenVisibleEnd = lineStart;

            // Walk the opportunities after the line's start, keeping the last that fits.
            int probe = nextOpportunity;
            while (probe < opportunities.Count)
            {
                int end = opportunities[probe];
                if (end <= lineStart)
                {
                    probe++;
                    continue;
                }

                int visibleEnd = TrimTrailingSpaces(text, lineStart, end);
                Length width = Measure(text, lineStart, visibleEnd, widthBetween, tabs);

                if (width > limit && chosen >= 0) break;

                chosen = end;
                chosenWidth = width;
                chosenVisibleEnd = visibleEnd;
                probe++;

                // A required break ends the line whether or not the text would have fitted: a manual
                // line break is not a suggestion, and running past one would put two of the document's
                // lines on one of the page's.
                if (mandatory.Contains(end)) break;

                // The first opportunity is taken whatever it measures: a word too long for the line
                // gets the line to itself rather than an empty line followed by the same problem.
                if (width > limit) break;
            }

            if (chosen <= lineStart)
            {
                // No opportunity at all past this point, which happens when the text ends without one.
                chosen = text.Length;
                chosenVisibleEnd = TrimTrailingSpaces(text, lineStart, chosen);
                chosenWidth = Measure(text, lineStart, chosenVisibleEnd, widthBetween, tabs);
            }

            lines.Add(new TextLine(
                lineStart, chosen, chosenVisibleEnd, chosenWidth,
                EndsParagraph: chosen >= text.Length));

            lineStart = chosen;
            nextOpportunity = probe;
        }

        return lines;
    }

    /// <summary>
    /// The width of a candidate line, with its tabs resolved when it has any.
    /// </summary>
    /// <remarks>
    /// A tab's width is where it lands rather than what the font says, so a line holding one cannot be
    /// measured as a difference of two prefix widths. The check for a tab comes first because nearly every
    /// line has none, and one that has none must measure exactly as it did before tabs existed.
    /// </remarks>
    private static Length Measure(
        string text,
        int start,
        int end,
        Func<int, int, Length> widthBetween,
        ParagraphFormat? tabs)
        => tabs is not null && TabRuler.HasTab(text, start, end)
            ? TabRuler.WidthOf(text, start, end, tabs, widthBetween)
            : widthBetween(start, end);

    /// <summary>
    /// Where a line's visible text ends: past its trailing spaces and its terminating break.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Only the space characters that can hang past a margin. A tab is not one of them — it advances to
    /// a stop and so has a width the line has to hold — and neither is a non-breaking space, whose
    /// whole purpose is to be part of the text around it.
    /// </para>
    /// <para>
    /// A mandatory break ends the line and takes no room on it, which is Writer's break portion: zero
    /// width, and no glyph. Leaving it in would put a <c>.notdef</c> box at the end of every line that
    /// ends in a manual break — and, worse, would count its advance towards the line's width, so a
    /// justified line would stretch by one glyph too little.
    /// </para>
    /// </remarks>
    private static int TrimTrailingSpaces(string text, int start, int end)
    {
        int at = Math.Min(end, text.Length);
        while (at > start && (text[at - 1] == ' ' || EndsLine(text[at - 1]))) at--;
        return at;
    }

    /// <summary>
    /// True for a character UAX #14 gives a mandatory break: one that ends a line and takes no room on it.
    /// </summary>
    /// <remarks>
    /// All of them, not just the line separator, because the readers do not agree on which to use — ODF's
    /// and OOXML's manual breaks arrive as U+2028 and RTF's as a newline, and both mean the same thing.
    /// </remarks>
    private static bool EndsLine(char character)
        => character is '\u2028' or '\u2029' or '\n' or '\r' or '\u000B' or '\u000C' or '\u0085';
}

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
/// <param name="ShrinkAllowance">
/// How much wider than its room the line was allowed to be, because a justified line's blanks may be
/// squeezed below their natural width. Zero for every line that was filled the ordinary way.
/// <para>
/// It travels with the line because the filler is the only thing that knows it — the room a line was
/// filled to, and the natural width of the blanks on it — and whatever justifies the line afterwards
/// needs the same number to know how far it may squeeze. Recomputing it there would mean measuring the
/// blanks a second time, and a second measurement that disagreed by a twip would put the line's last word
/// past the margin. See <see cref="JustificationShrink"/>.
/// </para>
/// </param>
public readonly record struct TextLine(
    int Start,
    int End,
    int VisibleEnd,
    Length Width,
    bool EndsParagraph,
    Length ShrinkAllowance = default)
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
    /// The paragraph's format, or null. Two things are read from it: its tab stops, consulted only for a
    /// line that holds a tab, and whether a justified line may squeeze its blanks
    /// (<see cref="ParagraphFormat.ShrinksJustifiedBlanks"/>). A paragraph that is neither tabbed nor
    /// justified measures exactly as it would without this parameter.
    /// </param>
    /// <param name="widthOfLine">
    /// The width one line may use, asked once per line just before it is filled. Null for the ordinary
    /// case, where every line but the first gets the same width; it exists for text flowing round a
    /// floating frame, where the width depends on how far down the paragraph the line sits.
    /// </param>
    public List<TextLine> Fill(
        MeasuredParagraph measured,
        Length availableWidth,
        Length? firstLineWidth = null,
        string? language = null,
        ParagraphFormat? tabs = null,
        Func<int, IReadOnlyList<TextLine>, Length>? widthOfLine = null)
    {
        ArgumentNullException.ThrowIfNull(measured);

        return Fill(
            measured.Text,
            availableWidth,
            firstLineWidth,
            language,
            measured.WidthBetween,
            tabs,
            widthOfLine);
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
    /// The paragraph's format, or null. Two things are read from it: its tab stops, consulted only for a
    /// line that holds a tab, and whether a justified line may squeeze its blanks
    /// (<see cref="ParagraphFormat.ShrinksJustifiedBlanks"/>). A paragraph that is neither tabbed nor
    /// justified measures exactly as it would without this parameter.
    /// </param>
    /// <param name="widthOfLine">
    /// The width one line may use, asked once per line just before it is filled. Null for the ordinary
    /// case, where every line but the first gets the same width; it exists for text flowing round a
    /// floating frame, where the width depends on how far down the paragraph the line sits.
    /// </param>
    public List<TextLine> Fill(
        string text,
        Length emSize,
        Length availableWidth,
        Length? firstLineWidth = null,
        string? language = null,
        ShapingOptions? options = null,
        ParagraphFormat? tabs = null,
        Func<int, IReadOnlyList<TextLine>, Length>? widthOfLine = null)
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
            tabs,
            widthOfLine);
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
        ParagraphFormat? tabs = null,
        Func<int, IReadOnlyList<TextLine>, Length>? widthOfLine = null)
    {
        List<TextLine> lines = [];
        if (text.Length == 0)
        {
            lines.Add(new TextLine(0, 0, 0, Length.Zero, EndsParagraph: true));
            return lines;
        }

        IReadOnlyList<int> opportunities = _breaker.FindBreakOpportunities(text, language);
        HashSet<int> mandatory = [.. _breaker.FindMandatoryBreaks(text, language)];

        // A justified paragraph in a file that asks for Word 2013's justification may overrun its room
        // by whatever squeezing its blanks recovers. Resolved once per paragraph rather than per
        // candidate line, so a paragraph that is not justified measures exactly as it did before this
        // existed.
        bool shrinks = tabs is { ShrinksJustifiedBlanks: true }
                       && tabs.Alignment is TextAlignment.Justify or TextAlignment.Distribute;

        int lineStart = 0;
        int nextOpportunity = 0;

        while (lineStart < text.Length)
        {
            Length limit = widthOfLine is not null
                ? widthOfLine(lines.Count, lines)
                : lines.Count == 0 ? firstLineWidth ?? availableWidth : availableWidth;

            int chosen = -1;
            Length chosenWidth = Length.Zero;
            int chosenVisibleEnd = lineStart;
            Length chosenAllowance = Length.Zero;

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
                Length width = Measure(
                    text, lineStart, visibleEnd, widthBetween, tabs, lines.Count == 0);

                // The allowance belongs to the candidate rather than to the line already chosen: the
                // word being tried brings a blank with it, and that blank can be squeezed too.
                Length allowance = shrinks
                    ? JustificationShrink.AllowanceFor(text, lineStart, visibleEnd, widthBetween)
                    : Length.Zero;

                if (width > limit + allowance && chosen >= 0) break;

                chosen = end;
                chosenWidth = width;
                chosenVisibleEnd = visibleEnd;
                chosenAllowance = allowance;
                probe++;

                // A required break ends the line whether or not the text would have fitted: a manual
                // line break is not a suggestion, and running past one would put two of the document's
                // lines on one of the page's.
                if (mandatory.Contains(end)) break;

                // The first opportunity is taken whatever it measures, and chopped below if it does
                // not fit: an empty line followed by the same problem is not an option.
                if (width > limit + allowance) break;
            }

            if (chosen <= lineStart)
            {
                // No opportunity at all past this point, which happens when the text ends without one.
                chosen = text.Length;
                chosenVisibleEnd = TrimTrailingSpaces(text, lineStart, chosen);
                chosenWidth = Measure(
                    text, lineStart, chosenVisibleEnd, widthBetween, tabs, lines.Count == 0);
                chosenAllowance = shrinks
                    ? JustificationShrink.AllowanceFor(
                        text, lineStart, chosenVisibleEnd, widthBetween)
                    : Length.Zero;
            }

            // Nothing between this line's start and its first break opportunity fits. The word is
            // chopped rather than left hanging over the margin — see the remarks on this class.
            if (chosenWidth > limit + chosenAllowance
                && Chop(text, lineStart, chosen, limit, widthBetween, tabs, lines.Count == 0)
                       is { } cut)
            {
                chosen = cut;
                chosenVisibleEnd = cut;
                chosenWidth = Measure(text, lineStart, cut, widthBetween, tabs, lines.Count == 0);

                // A chop lands inside a word, so the line it leaves holds no blank between two words
                // and there is nothing left to squeeze.
                chosenAllowance = Length.Zero;
                probe = nextOpportunity;
            }

            lines.Add(new TextLine(
                lineStart, chosen, chosenVisibleEnd, chosenWidth,
                EndsParagraph: chosen >= text.Length,
                ShrinkAllowance: chosenAllowance));

            lineStart = chosen;
            nextOpportunity = probe;
        }

        // A break on the very last character opens a line the loop above never enters, because it
        // leaves lineStart == text.Length. That line is empty and it is still a line: LibreOffice
        // makes a paragraph ending in one exactly as tall as the same paragraph followed by an
        // empty one. Measured on slide-trailing-break.pptx, four boxes differing only in where the
        // break sits — a trailing a:br and an explicit empty paragraph both put the next
        // paragraph's baseline 48.02 pt down, against 24.01 for no break at all.
        //
        // Only a *line* separator, never a paragraph one. The readers do not agree on which
        // character to use for a manual break, and two of the characters EndsLine accepts —
        // '\r' and '\n' — are also what a reader may leave on the end of a paragraph's text to
        // mean the paragraph ends there. Adding a line for those would lengthen every paragraph in
        // the corpus, so the set here is the strict subset that can only be a break inside one.
        if (lines.Count > 0 && IsLineSeparator(text[^1]))
        {
            lines.Add(new TextLine(
                text.Length, text.Length, text.Length, Length.Zero, EndsParagraph: true));
        }

        return lines;
    }

    /// <summary>
    /// True for a character that can only be a manual line break, never the end of a paragraph.
    /// </summary>
    /// <remarks>
    /// A strict subset of <see cref="EndsLine"/>. OOXML's <c>a:br</c> and <c>w:br</c> and
    /// ODF's <c>text:line-break</c> all arrive as U+2028, and a binary PowerPoint's is U+000B; the
    /// three the subset drops — <c>'\r'</c>, <c>'\n'</c> and U+2029 — are the ones a reader might
    /// be using to mark the end of the paragraph itself.
    /// </remarks>
    private static bool IsLineSeparator(char character)
        => character is '\u2028' or '\u000B' or '\u000C' or '\u0085';

    /// <summary>
    /// Where to cut a word that does not fit the line it starts, or null to leave it alone.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The last character position whose text still fits, and never the line's own start — a line
    /// of no characters would make no progress and the fill loop would not terminate. LibreOffice
    /// arrives at the same position from the other end: <c>ImpEditEngine::ImpBreakLine</c> walks
    /// the character-position array while it is under the remaining width
    /// (<c>editeng/source/editeng/impedit3.cxx:2016-2018</c>) and, when the break iterator offers
    /// nothing inside the line, takes it outright — "No separator in line =&gt; Chop!",
    /// <c>impedit3.cxx:2236-2247</c>, with the same guard against an empty line. Writer's
    /// <c>SwTextGuess::Guess</c> ends in the same place, cutting at <c>m_nCutPos</c> when no break
    /// position was found (<c>sw/source/core/text/guess.cxx:832-839</c>).
    /// </para>
    /// <para>
    /// It only applies to a word that is alone on its line: a line that has already fitted
    /// something breaks before the oversized word instead, and the word is chopped on the line it
    /// then starts. That is why the caller tests the chosen width rather than each candidate's.
    /// </para>
    /// <para>
    /// A cut never lands between a surrogate pair. It can still land inside a grapheme cluster
    /// whose base and marks are separate code points, which is what LibreOffice's own
    /// <c>iterateCodePoints</c> does too.
    /// </para>
    /// </remarks>
    private static int? Chop(
        string text,
        int lineStart,
        int end,
        Length limit,
        Func<int, int, Length> widthBetween,
        ParagraphFormat? tabs,
        bool isFirstLine)
    {
        int first = Whole(text, lineStart + 1);
        if (first >= end) return null;

        // Binary search: prefix widths only grow, so the last position that fits is found in a
        // logarithm of the word's length rather than by measuring every prefix of it.
        int low = first;
        int high = end - 1;
        int best = first;

        while (low <= high)
        {
            int middle = Whole(text, low + ((high - low) / 2));
            if (middle <= first) { low = first + 1; continue; }
            if (middle >= end) { high = middle - 2; continue; }

            if (Measure(text, lineStart, middle, widthBetween, tabs, isFirstLine) <= limit)
            {
                best = middle;
                low = middle + 1;
            }
            else
            {
                high = middle - 1;
            }
        }

        return best;
    }

    /// <summary>A cut position moved forward off the second half of a surrogate pair.</summary>
    private static int Whole(string text, int index)
    {
        while (index < text.Length && char.IsLowSurrogate(text[index])) index++;
        return index;
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
        ParagraphFormat? tabs,
        bool isFirstLine)
        => tabs is not null && TabRuler.HasTab(text, start, end)
            ? TabRuler.WidthOf(text, start, end, tabs, widthBetween, isFirstLine)
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

using Paperless.Core.Units;

namespace Paperless.Text.Layout;

/// <summary>One stretch of a line between tabs, and where it sits.</summary>
/// <param name="Start">Its first character, as an index into the paragraph's text.</param>
/// <param name="End">One past its last character.</param>
/// <param name="Left">Where it starts, measured from the line's own left edge.</param>
/// <param name="Width">How wide its text is.</param>
public readonly record struct TabbedSegment(int Start, int End, Length Left, Length Width)
{
    /// <summary>Where the segment ends.</summary>
    public Length Right => Left + Width;

    /// <summary>True when the segment holds no characters, which an adjacent pair of tabs produces.</summary>
    public bool IsEmpty => End <= Start;
}

/// <summary>
/// Resolves the tabs in a line: how far each one advances, and where the text after it sits.
/// </summary>
/// <remarks>
/// <para>
/// A tab is the one character whose width is not a property of the font. It advances to the next stop, so
/// its width depends on where in the line it happens to fall — which means a line holding one cannot be
/// measured by adding up glyph advances, and the same walk has to serve both the measuring and the
/// drawing. Two walks would eventually disagree, and the symptom would be text drawn a little to one side
/// of the column it was measured into.
/// </para>
/// <para>
/// The stops themselves come from the paragraph and are measured from where its text starts, not from the
/// page — so an indented paragraph's stops move with it, which is what makes a tabbed list line up under
/// an indent.
/// </para>
/// </remarks>
public static class TabRuler
{
    /// <summary>
    /// How many segments a line is split into before the rest is measured without tabs.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input. A real line holds a handful of tabs; a generated file can hold
    /// thousands, and each one costs a stop lookup and a measurement.
    /// </remarks>
    public const int MaxSegments = 4096;

    /// <summary>The character a tab is.</summary>
    private const char Tab = '\t';

    /// <summary>
    /// Splits a range of a line at its tabs and places each stretch.
    /// </summary>
    /// <remarks>
    /// The pen never goes backwards: a stop behind the text already set is skipped, which is what Writer
    /// does when a centred or right stop cannot fit what it was given. Without that a long entry in a
    /// tabbed column would draw over the one before it.
    /// </remarks>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="start">Where the line starts.</param>
    /// <param name="end">Where the line's visible text ends.</param>
    /// <param name="format">The paragraph's stops and default interval.</param>
    /// <param name="widthBetween">Measures a range of the text, tabs excluded.</param>
    public static List<TabbedSegment> Segments(
        string text,
        int start,
        int end,
        ParagraphFormat format,
        Func<int, int, Length> widthBetween)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(format);
        ArgumentNullException.ThrowIfNull(widthBetween);

        int last = Math.Min(end, text.Length);
        List<TabbedSegment> segments = [];
        Length pen = Length.Zero;
        int at = start;

        // The first stretch begins at the line's left edge; every later one begins at a stop.
        TabStop? pending = null;

        while (at <= last && segments.Count < MaxSegments)
        {
            int tab = text.IndexOf(Tab, at);
            int stretchEnd = tab >= 0 && tab < last ? tab : last;

            Length width = widthBetween(at, stretchEnd);
            Length left = pending is { } stop
                ? Place(stop, pen, width, text, at, stretchEnd, widthBetween)
                : pen;

            segments.Add(new TabbedSegment(at, stretchEnd, left, width));
            pen = left + width;

            if (stretchEnd >= last) break;

            // The tab itself: it takes no width of its own, it moves the pen to the next stop.
            pending = format.NextTabStop(pen);
            at = stretchEnd + 1;
        }

        return segments;
    }

    /// <summary>The width of a range of a line, with its tabs resolved.</summary>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="start">Where the line starts.</param>
    /// <param name="end">Where the measured text ends.</param>
    /// <param name="format">The paragraph's stops and default interval.</param>
    /// <param name="widthBetween">Measures a range of the text, tabs excluded.</param>
    public static Length WidthOf(
        string text,
        int start,
        int end,
        ParagraphFormat format,
        Func<int, int, Length> widthBetween)
    {
        List<TabbedSegment> segments = Segments(text, start, end, format, widthBetween);
        return segments.Count == 0 ? Length.Zero : segments[^1].Right;
    }

    /// <summary>True when a range holds a tab, and so needs any of this.</summary>
    /// <remarks>
    /// Asked before the machinery is used, because a paragraph without tabs — nearly all of them — should
    /// measure exactly as it did before this existed: one shaped prefix table and a subtraction.
    /// </remarks>
    public static bool HasTab(string text, int start, int end)
    {
        ArgumentNullException.ThrowIfNull(text);

        int at = text.IndexOf(Tab, Math.Max(start, 0));
        return at >= 0 && at < Math.Min(end, text.Length);
    }

    /// <summary>
    /// Where a stretch of text sits relative to the stop that introduced it.
    /// </summary>
    /// <remarks>
    /// A left stop puts the text's start on it, a right stop its end, a centre stop its middle, and a
    /// decimal stop its separator — which is why this needs the stretch's width and, for the decimal case,
    /// its text. A stop already behind the pen cannot be honoured at all, so the text simply continues.
    /// </remarks>
    private static Length Place(
        TabStop stop,
        Length pen,
        Length width,
        string text,
        int start,
        int end,
        Func<int, int, Length> widthBetween)
    {
        Length left = stop.Alignment switch
        {
            TabAlignment.Right => stop.Position - width,
            TabAlignment.Centre => stop.Position - (width / 2.0),
            TabAlignment.DecimalSeparator =>
                stop.Position - widthBetween(start, SeparatorIn(text, start, end)),
            _ => stop.Position,
        };

        return left < pen ? pen : left;
    }

    /// <summary>
    /// Where the decimal separator in a stretch is, or its end when it has none.
    /// </summary>
    /// <remarks>
    /// Both spellings, because the separator is a comma across most of Europe and a document does not say
    /// which it meant. Taking the <em>last</em> one: <c>1.234,56</c> uses both, and it is the comma that
    /// separates the fraction. A stretch with no separator aligns on its end, which is what makes a column
    /// of whole numbers line up with a column of fractional ones.
    /// </remarks>
    private static int SeparatorIn(string text, int start, int end)
    {
        int last = Math.Min(end, text.Length);
        int found = -1;

        for (int at = start; at < last; at++)
        {
            if (text[at] is '.' or ',') found = at;
        }

        return found >= 0 ? found : last;
    }
}

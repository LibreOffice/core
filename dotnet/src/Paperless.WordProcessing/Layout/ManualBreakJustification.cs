using Paperless.Core.Units;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// Un-stretches the lines a manual break ends, for the documents that ask for it.
/// </summary>
/// <remarks>
/// <para>
/// A justified paragraph stretches every line but its last, and a line ended by a shift-return
/// is not the last line — so it gets stretched, and a two-word line ends up with its words at
/// opposite margins. Word before the 2000s did that too, and the compatibility option
/// <c>w:doNotExpandShiftReturn</c> is how a file says it wants the older behaviour: a line ended
/// by a break is left ragged, exactly as a paragraph's last line is.
/// </para>
/// <para>
/// LibreOffice maps the option to its <c>DoNotJustifyLinesWithManualBreak</c> document setting
/// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx</c>:10160). Measured on a
/// two-line justified paragraph split by a <c>w:br</c>: with the flag absent LibreOffice draws
/// the first line's last run at x = 538.75 pt, hard against the right margin, and with it
/// present at x = 154.0 pt, where the words naturally end. That is the largest single effect any
/// of the compatibility options has on this engine's output, which is why it is the one wired.
/// </para>
/// <para>
/// Only the drawing changes. The line still breaks in the same place, so nothing below it moves
/// — which is why this can be applied to a finished layout instead of being threaded into the
/// line filler.
/// </para>
/// </remarks>
internal static class ManualBreakJustification
{
    /// <summary>
    /// The character a <c>w:br</c>, an RTF <c>\line</c> or a WW8 <c>U+000B</c> becomes in a
    /// paragraph's text.
    /// </summary>
    /// <remarks>
    /// U+2028, chosen by the readers because UAX #14 classes it as a mandatory break, so the
    /// break iterator honours it with no special case. That also makes it the only marker
    /// available here: a line ended by a break is a line whose text ends with this.
    /// </remarks>
    private const char LineSeparator = '\u2028';

    /// <summary>
    /// The same paragraph with the stretch removed from every line a manual break ends.
    /// </summary>
    /// <param name="paragraph">The laid-out paragraph.</param>
    /// <param name="text">Its text, which the lines index into.</param>
    public static LaidOutParagraph Suppress(LaidOutParagraph paragraph, string text)
    {
        ArgumentNullException.ThrowIfNull(paragraph);
        ArgumentNullException.ThrowIfNull(text);

        if (!text.Contains(LineSeparator, StringComparison.Ordinal)) return paragraph;

        List<LineBox> lines = new(paragraph.Lines.Count);
        bool changed = false;

        foreach (LineBox box in paragraph.Lines)
        {
            // The separator is the line's last character, but whether it counts as visible
            // depends on the filler's trailing-blank trimming — so the tail from the visible end
            // to the end is searched, and the character before the visible end as well. Looking
            // only past the visible end finds nothing, because U+2028 is not one of the blanks
            // the filler trims.
            int from = Math.Max(box.Line.Start, box.Line.VisibleEnd - 1);
            int to = Math.Min(box.Line.End, text.Length);
            bool endsWithBreak = text.AsSpan(from, Math.Max(0, to - from)).Contains(LineSeparator);

            // Only the stretch. A negative space-add is a line whose blanks were squeezed to bring it
            // back inside the margin (see `JustificationShrink`), and taking that off would leave the
            // line's last word past the margin rather than merely un-stretched — the flag asks for a
            // ragged line, not an overflowing one.
            if (endsWithBreak && box.SpaceAdd > Length.Zero)
            {
                lines.Add(box with { SpaceAdd = Length.Zero });
                changed = true;
            }
            else
            {
                lines.Add(box);
            }
        }

        return changed ? paragraph with { Lines = lines } : paragraph;
    }
}

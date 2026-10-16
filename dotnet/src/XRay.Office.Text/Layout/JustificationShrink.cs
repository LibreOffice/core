using XRay.Office.Core.Units;

namespace XRay.Office.Text.Layout;

/// <summary>
/// How far a justified line may squeeze its blanks below their natural width.
/// </summary>
/// <remarks>
/// <para>
/// A justified line has always been able to <em>stretch</em> its blanks; Word 2013 also lets it
/// <em>compress</em> them, which means a line can hold text that does not fit it at natural widths. The
/// consequence is not cosmetic: the same text sets in fewer lines, so a document paginates shorter.
/// LibreOffice states it in as many words where it turns the behaviour on —
/// <c>"new paragraph justification has been introduced in version 15, breaking text layout
/// interoperability: new line shrinking needs less space i.e. it typesets the same text with less lines
/// and pages"</c>, <c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:10172</c>, which sets
/// <c>JustifyLinesWithShrinking</c> for every file whose <c>compatibilityMode</c> is 15 or more.
/// </para>
/// <para>
/// The rule is applied in <c>SwTextPortion::Format_</c>
/// (<c>sw/source/core/text/portxt.cxx:545</c>): having guessed a break the ordinary way, a full
/// justified line is guessed again with its blanks at a <em>minimum word spacing</em>, and the longer
/// guess taken. For a file that says nothing more than <c>compatibilityMode</c> — every Word 2013 file,
/// which is what <c>bOldInterop</c> names there — that minimum is
/// <see cref="MinimumBlankProportion">75%</see>.
/// </para>
/// <para>
/// Stated as an allowance rather than as a second break attempt, because a greedy filler already walks
/// the break opportunities in order: a candidate line fits when its natural width is within its room plus
/// <see cref="AllowanceFor"/>, which is exactly "every blank on it can be squeezed to 75% and it then
/// fits". That formulation is self-consistent — the line finally chosen satisfies the constraint it was
/// measured against — where the reference's is an estimate, since it has to guess the blank count of a
/// line it has not broken yet and adds one for the space the new word brings with it.
/// </para>
/// <para>
/// Measured on <c>BID_ACKNOWLEDGEMENT_FORM_FOR_A320.docx</c>, whose first justified line holds sixteen
/// words summing to 417.63 pt in a 468.0 pt column: we set fifteen gaps at 3.358 pt and the reference
/// seventeen words with sixteen gaps at 1.894 pt, against a natural Carlito space of 2.26 pt. That is a
/// blank at 83.8% of its natural width — inside the 75% floor, and the reason the reference fits a word
/// we do not.
/// </para>
/// <para>
/// <b>Taking the maximum shrink is 24.2.7.2's rule and 26.2.4.2 weighs it instead.</b> The 75% floor
/// still says how far a blank <em>may</em> be squeezed; what changed is that reaching the floor no
/// longer decides the break. Having guessed at the minimum spacing, `portxt.cxx`:769-805 compares the
/// blanks the longer line would squeeze against the blanks the shorter line would stretch, weighting
/// the stretch by <see cref="ExpansionWeight">1/1.7</see>, and keeps the shorter line when stretching
/// wins. <see cref="PrefersShrinking"/> is that comparison.
/// </para>
/// <para>
/// Measured on 26.2.4.2 by sweeping the text width of the corpus paragraph and reading the first
/// line's decision off both the mode-15 and the mode-12 rendering of it — the mode-12 one being the
/// un-shrunk break, since shrinking is off there. Five widths leave the next word reachable inside the
/// floor; the weighted comparison calls all five, and so does the corpus fixture's second line:
/// </para>
/// <code>
/// stretch  squeeze  weighted x squeezed   26.2.4.2
///   1.552    0.839               1.112    shrank
///   1.531    0.920               1.207    shrank
///   2.295    0.991               1.746    shrank
///   2.156    0.858               1.441    shrank
///   1.391    0.795               0.978    did not
///   1.392    0.800               0.985    did not   (the fixture's line 2)
/// </code>
/// <para>
/// The other seven widths in that sweep are decided by the floor alone — the next word needs the
/// blanks below 75% — so they say nothing about the weighting and are consistent either way. The rule
/// also carries a second clause, "shrink when not shrinking would stretch the blanks past the maximum
/// word spacing"; **it did not discriminate on any measured line** and the tree's own value for that
/// maximum in this mode is 100%, which would shrink every one of the six rows above and contradicts
/// four of them, so it is deliberately not implemented. The tree is 27.2.0.0.alpha0+ and the reference
/// is 26.2.4.2; where the two disagree the measurement wins.
/// </para>
/// </remarks>
public static class JustificationShrink
{
    /// <summary>
    /// The narrowest a blank may be squeezed to, as a proportion of its natural width.
    /// </summary>
    /// <remarks>
    /// Seventy-five per cent — <c>nMinimum = bOldInterop ? 75 : …</c> in
    /// <c>SwTextPortion::Format_</c> (<c>sw/source/core/text/portxt.cxx</c>), where
    /// <c>bOldInterop</c> is the file that asks for shrinking and states no word-spacing bounds of its
    /// own, which is every Word 2013 and later document.
    /// </remarks>
    public const double MinimumBlankProportion = 0.75;

    /// <summary>
    /// How much of a stretched blank counts when stretching is weighed against squeezing.
    /// </summary>
    /// <remarks>
    /// The reciprocal of 1.7 — <c>fExpansionWeight</c> in <c>SwTextPortion::Format_</c>
    /// (<c>sw/source/core/text/portxt.cxx</c>). A line's blanks are compared as ratios of their natural
    /// width, and the stretched line's excess is discounted by this before the two are compared, which
    /// is what makes a line prefer a moderate stretch over a deep squeeze.
    /// </remarks>
    public const double ExpansionWeight = 1.0 / 1.7;

    /// <summary>
    /// How much wider than its room a line may be, given the blanks it holds.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A quarter of the natural width of the line's blanks, which is what squeezing each of them to
    /// <see cref="MinimumBlankProportion"/> recovers. Blanks are measured one at a time out of the
    /// paragraph's own prefix widths rather than from a nominal space width, so a line mixing sizes or
    /// faces is charged what its own blanks are worth. LibreOffice measures ten spaces in the line's
    /// current font and divides by ten, which is the same quantity for the uniform line and an
    /// approximation for the mixed one.
    /// </para>
    /// <para>
    /// A line holding a tab gets nothing. Writer refuses to shrink a tabulated line outright
    /// (<c>tdf#164499</c>, the <c>InTabGrp</c> test at <c>portxt.cxx</c>:571), and it has to be refused
    /// here as well as where the line is justified: a line admitted on an allowance it is then not
    /// squeezed by would simply run past the margin.
    /// </para>
    /// </remarks>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="start">Where the line starts.</param>
    /// <param name="end">Where its visible text ends.</param>
    /// <param name="widthBetween">The natural width of a range of the paragraph.</param>
    public static Length AllowanceFor(
        string text, int start, int end, Func<int, int, Length> widthBetween)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(widthBetween);

        if (end > text.Length) end = text.Length;
        if (end <= start) return Length.Zero;
        if (TabRuler.HasTab(text, start, end)) return Length.Zero;

        long blanks = 0;

        for (int at = start; at < end; at++)
        {
            if (text[at] == ' ') blanks += widthBetween(at, at + 1).Emu;
        }

        return Length.FromEmu((long)(blanks * (1.0 - MinimumBlankProportion)));
    }

    /// <summary>
    /// The natural width of the blanks a candidate line holds.
    /// </summary>
    /// <remarks>
    /// What <see cref="AllowanceFor"/> takes a quarter of, and what <see cref="PrefersShrinking"/>
    /// measures a stretch or a squeeze against. Separate because the two want the same quantity for
    /// different reasons: the floor is a proportion of it, and the comparison is a ratio to it.
    /// </remarks>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="start">Where the line starts.</param>
    /// <param name="end">Where its visible text ends.</param>
    /// <param name="widthBetween">The natural width of a range of the paragraph.</param>
    public static Length BlanksOn(string text, int start, int end, Func<int, int, Length> widthBetween)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(widthBetween);

        if (end > text.Length) end = text.Length;
        if (end <= start) return Length.Zero;
        if (TabRuler.HasTab(text, start, end)) return Length.Zero;

        long blanks = 0;

        for (int at = start; at < end; at++)
        {
            if (text[at] == ' ') blanks += widthBetween(at, at + 1).Emu;
        }

        return Length.FromEmu(blanks);
    }

    /// <summary>
    /// Whether a line that only fits by squeezing is preferred to the shorter line that fits without.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both candidates are stated as what they do to a blank, as a proportion of its natural width:
    /// the shorter line stretches its blanks to <c>stretch</c> ≥ 1 because a justified line fills its
    /// room, and the longer one squeezes them to <c>squeeze</c> ≤ 1. The longer line wins when the
    /// stretch, discounted by <see cref="ExpansionWeight"/>, is at least as far from natural as the
    /// squeeze is — <c>weighted ≥ 1 / squeeze</c>, written here as a product so that a squeeze of zero
    /// cannot divide.
    /// </para>
    /// <para>
    /// Answered <see langword="true"/> — take the longer line, as 24.2.7.2 always did — when the
    /// comparison has nothing to say: when either candidate holds no blank to move, so there is no
    /// ratio, or when the shorter line does not fit its room either, so it is not the un-stretched
    /// alternative this weighs against. The 75% floor is not applied here; it is the caller's
    /// admission test and this decides only between two candidates that have already passed it.
    /// </para>
    /// </remarks>
    /// <param name="room">The line's room.</param>
    /// <param name="shorterWidth">The natural width of the shorter candidate.</param>
    /// <param name="shorterBlanks">The natural width of the shorter candidate's blanks.</param>
    /// <param name="longerWidth">The natural width of the longer candidate, which exceeds the room.</param>
    /// <param name="longerBlanks">The natural width of the longer candidate's blanks.</param>
    public static bool PrefersShrinking(
        Length room,
        Length shorterWidth,
        Length shorterBlanks,
        Length longerWidth,
        Length longerBlanks)
    {
        if (shorterBlanks <= Length.Zero || longerBlanks <= Length.Zero) return true;
        if (shorterWidth > room) return true;

        double stretch = 1.0 + ((room - shorterWidth).Emu / (double)shorterBlanks.Emu);
        double squeeze = 1.0 - ((longerWidth - room).Emu / (double)longerBlanks.Emu);

        return (1.0 + ((stretch - 1.0) * ExpansionWeight)) * squeeze >= 1.0;
    }
}

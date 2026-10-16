namespace XRay.Office.Text.Fonts;

/// <summary>How many lines a run's underline is drawn with.</summary>
/// <remarks>
/// <para>
/// A tri-state rather than a <c>bool</c> because the reference draws a double underline with
/// <em>two thinner lines at their own offsets</em> and not with one line twice over: see
/// <see cref="LineSpacing.RuleWidths.DoubleUnderline"/> for the thickness and
/// <see cref="LineSpacing.RuleWidths.DoubleUnderlineSecond"/> for the separation. Modelling it as
/// a flag is what made <c>w:u w:val="double"</c>, <c>\uldb</c>,
/// <c>style:text-underline-type="double"</c> and WW8's <c>sprmCKul</c> operand 3 all reach the
/// drawing pass as a plain underline.
/// </para>
/// <para>
/// <strong>The line <em>style</em> is deliberately not here.</strong> ODF states the style and the
/// type as two attributes, WordprocessingML folds both into one <c>w:val</c>, and RTF has a control
/// word per combination — but the reference collapses every dotted, dashed and wavy form to a
/// solid line in this engine already, and the count of lines and the line's WEIGHT are the only
/// parts of the vocabulary that change the geometry. A reader that meets <c>dotDotDash</c> answers
/// <see cref="SingleLine"/> exactly as it always did, and one that meets <c>dashDotDotHeavy</c>
/// answers <see cref="BoldLine"/>, because the heavy forms differ from their light twins in
/// thickness as well as in pattern.
/// </para>
/// <para>
/// The spreadsheet track has carried the same tri-state since long before this, as
/// <c>SheetUnderline</c>; that type stays where it is because it also folds Excel's two
/// <em>accounting</em> forms, which are a width rule rather than a line count and which no
/// word-processing or presentation format states. The members are spelled the way
/// <c>SheetUnderline</c>'s are for the same reason it is: <c>Single</c> and <c>Double</c> are
/// type names and CA1720 rejects both.
/// </para>
/// </remarks>
public enum TextUnderline
{
    /// <summary>No line.</summary>
    None,

    /// <summary>One line under the text.</summary>
    SingleLine,

    /// <summary>Two thinner lines under the text.</summary>
    DoubleLine,

    /// <summary>One line, about twice as thick as an ordinary one.</summary>
    /// <remarks>
    /// <c>LINESTYLE_BOLD</c> and the six patterned forms that share its weight —
    /// <c>w:u</c>'s <c>thick</c> and its five <c>…Heavy</c> values plus <c>wavyHeavy</c>,
    /// RTF's <c>\ulth</c> family and <c>\ulhwave</c>, <c>sprmCKul</c> operands 6, 20, 23, 25, 26,
    /// 27 and 55, and ODF's <c>style:text-underline-width="bold"</c> or <c>"thick"</c>. It is a
    /// third size and not a scaled first: <c>((descent × 50) + 50) / 100</c> against the single's
    /// <c>× 25</c>, with a guard that bumps it by one where the two would come out equal.
    /// </remarks>
    BoldLine,
}

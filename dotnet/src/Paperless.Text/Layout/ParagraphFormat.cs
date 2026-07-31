using Paperless.Core.Units;

namespace Paperless.Text.Layout;

/// <summary>How a paragraph's lines are aligned in the width available to them.</summary>
public enum TextAlignment
{
    /// <summary>Against the start edge: left for left-to-right text, right for right-to-left.</summary>
    Start,

    /// <summary>Against the end edge.</summary>
    End,

    /// <summary>Centred.</summary>
    Centre,

    /// <summary>Stretched to both edges, the last line excepted.</summary>
    Justify,

    /// <summary>
    /// Stretched to both edges including the last line.
    /// </summary>
    /// <remarks>
    /// A separate value rather than a flag on <see cref="Justify"/> because it is a separate setting in
    /// every format, and because the difference is visible on exactly the line a reader looks at last.
    /// </remarks>
    Distribute,
}

/// <summary>How the distance from one baseline to the next is decided.</summary>
public enum LineSpacingMode
{
    /// <summary>
    /// A multiple of the line's natural height. Single spacing is this at one.
    /// </summary>
    Proportional,

    /// <summary>
    /// At least a given height, and the natural height when that is larger.
    /// </summary>
    /// <remarks>
    /// The mode that keeps a large character on a tightly spaced line from being clipped, which is why
    /// it is the one a well-behaved template uses rather than <see cref="Exact"/>.
    /// </remarks>
    AtLeast,

    /// <summary>
    /// Exactly a given height, whatever the text needs.
    /// </summary>
    /// <remarks>
    /// Taller text is clipped rather than given room. That is the point — a form or a table of figures
    /// needs its rows to line up more than it needs every glyph whole.
    /// </remarks>
    Exact,

    /// <summary>The natural height plus a fixed amount of extra space between lines.</summary>
    Leading,
}

/// <summary>
/// How far apart a paragraph's baselines sit.
/// </summary>
/// <remarks>
/// <para>
/// One type for what the four formats spell four ways. DOCX writes <c>w:spacing w:line</c> with a
/// <c>w:lineRule</c> of <c>auto</c>, <c>atLeast</c> or <c>exact</c>, where <c>auto</c> counts in
/// two-hundred-and-fortieths of a line rather than in a percentage. ODF splits it across three
/// attributes — <c>fo:line-height</c> for a percentage or an exact length,
/// <c>style:line-height-at-least</c>, and <c>style:line-spacing</c> for leading. RTF's <c>\sl</c> is
/// twips with a sign, where a negative value means exact. DOC's <c>sprmPDyaLine</c> is the same idea
/// again.
/// </para>
/// <para>
/// Where the extra space goes matters as much as how much there is, and the answer is not symmetrical:
/// Writer puts proportional spacing's extra height <em>above</em> the text, so a 200%-spaced paragraph
/// has its first baseline pushed down rather than a gap left under its last line.
/// </para>
/// </remarks>
/// <param name="Mode">Which rule applies.</param>
/// <param name="Proportion">
/// The multiple, for <see cref="LineSpacingMode.Proportional"/>. One is single spacing.
/// </param>
/// <param name="Value">
/// The length, for the three modes that take one. Ignored for
/// <see cref="LineSpacingMode.Proportional"/>.
/// </param>
public readonly record struct LineSpacingRule(
    LineSpacingMode Mode,
    double Proportion,
    Length Value)
{
    /// <summary>
    /// Single spacing: the font's own line height, unmodified.
    /// </summary>
    /// <remarks>
    /// Not named <c>Single</c>, which collides with the floating-point type and reads as a number
    /// rather than as a spacing.
    /// </remarks>
    public static LineSpacingRule SingleSpaced { get; } =
        new(LineSpacingMode.Proportional, 1.0, Length.Zero);

    /// <summary>
    /// The largest multiple honoured, beyond which the value is treated as a producer error.
    /// </summary>
    /// <remarks>
    /// A page is finite, and a proportion of a few thousand turns one paragraph into a document that
    /// paginates until it runs out of memory. Twenty times single spacing is far beyond anything a
    /// human asks for.
    /// </remarks>
    public const double MaxProportion = 20.0;

    /// <summary>A multiple of the natural line height.</summary>
    public static LineSpacingRule Multiple(double proportion)
        => new(LineSpacingMode.Proportional, Math.Clamp(proportion, 0.0, MaxProportion), Length.Zero);

    /// <summary>At least a height, growing for taller text.</summary>
    public static LineSpacingRule AtLeast(Length value)
        => new(LineSpacingMode.AtLeast, 1.0, value);

    /// <summary>Exactly a height, clipping taller text.</summary>
    public static LineSpacingRule Exactly(Length value)
        => new(LineSpacingMode.Exact, 1.0, value);

    /// <summary>The natural height plus a fixed gap.</summary>
    public static LineSpacingRule PlusLeading(Length value)
        => new(LineSpacingMode.Leading, 1.0, value);

    /// <summary>
    /// The smallest proportion honoured; below it Writer clamps rather than obeying.
    /// </summary>
    /// <remarks>
    /// Fifty per cent. <c>SwTextFormatter::CalcRealHeight</c> raises anything lower to it, with the
    /// comment that Word will render less "but it's just not readable" — and zero, which a document does
    /// write, means single spacing rather than none.
    /// </remarks>
    public const double MinProportion = 0.5;

    /// <summary>
    /// The height a line gets, given the height its text naturally wants.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Computed in <em>whole twips</em>, with integer arithmetic, because that is Writer's layout unit
    /// and the rounding is observable. An 11 pt line of Carlito is 268.55 twips of text, which Writer
    /// rounds to 269; 115% spacing then adds <c>15 * 269 / 100</c> truncated to 40, giving 309 twips
    /// exactly. Keeping the exact value instead gives 308.79, and the two-hundredths of a point per line
    /// accumulates over a page to nearly half a point — which is enough to fit one line more than Writer
    /// does and so to move the page break and every one after it.
    /// </para>
    /// <para>
    /// Never zero for a line with text in it: a paragraph whose spacing resolved to nothing would stack
    /// every line on one baseline, which is worse output than ignoring the document.
    /// </para>
    /// </remarks>
    public Length Apply(Length naturalHeight)
    {
        long natural = naturalHeight.Twips;

        long twips = Mode switch
        {
            LineSpacingMode.Proportional => natural + Extra(natural),
            LineSpacingMode.AtLeast => Math.Max(natural, Value.Twips),
            LineSpacingMode.Exact => Value.Twips,
            LineSpacingMode.Leading => natural + Value.Twips,
            _ => natural,
        };

        return twips > 0 ? Length.FromTwips(twips) : Length.FromTwips(natural);
    }

    /// <summary>
    /// The extra twips proportional spacing adds, the way Writer computes them.
    /// </summary>
    /// <remarks>
    /// A percentage in whole points of a per cent, applied with integer division so the result truncates
    /// rather than rounds — <c>nTmp -= 100; nTmp *= baseHeight; nTmp /= 100</c> in
    /// <c>CalcRealHeight</c>. Rounding instead would be off by a twip on most sizes.
    /// </remarks>
    private long Extra(long naturalTwips)
    {
        double clamped = Math.Clamp(Proportion, 0.0, MaxProportion);
        long percent = clamped <= 0 ? 100 : (long)Math.Round(clamped * 100);
        if (percent < MinProportion * 100) percent = (long)(MinProportion * 100);

        return (percent - 100) * naturalTwips / 100;
    }
}

/// <summary>What a tab advances to.</summary>
public enum TabAlignment
{
    /// <summary>Text starts at the stop.</summary>
    Left,

    /// <summary>Text is centred on the stop.</summary>
    Centre,

    /// <summary>Text ends at the stop.</summary>
    Right,

    /// <summary>
    /// The text's decimal separator sits on the stop.
    /// </summary>
    /// <remarks>
    /// Locale-dependent, since the separator is a comma in most of Europe. A column of figures is the
    /// whole reason this alignment exists, so getting the separator wrong makes the column ragged in
    /// exactly the place it was set up to be straight.
    /// </remarks>
    DecimalSeparator,

    /// <summary>Not a stop at all: a vertical rule drawn at the position.</summary>
    Bar,
}

/// <summary>
/// One tab stop.
/// </summary>
/// <param name="Position">Its distance from the text area's start edge, not from the indent.</param>
/// <param name="Alignment">What the text does at the stop.</param>
/// <param name="Leader">
/// The character filling the space before the stop, or <c>'\0'</c> for none — a dot leader in a table
/// of contents is the common case.
/// </param>
public readonly record struct TabStop(Length Position, TabAlignment Alignment = TabAlignment.Left, char Leader = '\0');

/// <summary>
/// A paragraph's resolved layout properties.
/// </summary>
/// <remarks>
/// <para>
/// Resolved, not as any format states them: the style chain has already been walked and the toggles
/// already applied, so this is what the paragraph <em>is</em> rather than what its file said. That is
/// what lets one layout engine serve four importers, and it is where each format's peculiarities stop.
/// </para>
/// <para>
/// It lives in <c>Paperless.Text</c> rather than beside the word-processing model because a
/// spreadsheet cell and a slide's text box lay their paragraphs out with the same rules —
/// LibreOffice's EditEngine plays exactly this part for the same reason.
/// </para>
/// </remarks>
public sealed record ParagraphFormat
{
    /// <summary>The defaults: start-aligned, single-spaced, no indents.</summary>
    public static ParagraphFormat Default { get; } = new();

    /// <summary>How the lines sit in the width available to them.</summary>
    public TextAlignment Alignment { get; init; } = TextAlignment.Start;

    /// <summary>How far the paragraph is indented from the text area's start edge.</summary>
    public Length StartIndent { get; init; }

    /// <summary>How far it is indented from the end edge.</summary>
    public Length EndIndent { get; init; }

    /// <summary>
    /// How much further the first line is indented, which may be negative.
    /// </summary>
    /// <remarks>
    /// Negative is the hanging indent a numbered list is built from: the number sits out to the left of
    /// the text that follows it, so the first line starts before the rest.
    /// </remarks>
    public Length FirstLineIndent { get; init; }

    /// <summary>Space above the paragraph.</summary>
    public Length SpaceBefore { get; init; }

    /// <summary>Space below it.</summary>
    public Length SpaceAfter { get; init; }

    /// <summary>
    /// True when the space between two paragraphs of the same style is suppressed.
    /// </summary>
    /// <remarks>
    /// DOCX's <c>w:contextualSpacing</c> and ODF's <c>style:contextual-spacing</c>. It is what keeps a
    /// list from having a gap between every bullet while still having one before the list.
    /// </remarks>
    public bool HasContextualSpacing { get; init; }

    /// <summary>How far apart the baselines sit.</summary>
    public LineSpacingRule LineSpacing { get; init; } = LineSpacingRule.SingleSpaced;

    /// <summary>The paragraph's own tab stops, in ascending position order.</summary>
    public IReadOnlyList<TabStop> TabStops { get; init; } = [];

    /// <summary>
    /// The interval at which tabs fall when no stop covers them.
    /// </summary>
    /// <remarks>
    /// Every format has a document-wide default — DOCX's <c>w:defaultTabStop</c>, RTF's
    /// <c>\deftab</c> — and a tab past the last explicit stop lands on the next multiple of it. A
    /// value of zero would make a tab advance nowhere and loop, so it falls back to half an inch.
    /// </remarks>
    public Length DefaultTabInterval { get; init; } = Length.FromTwips(720);

    /// <summary>True when the paragraph must stay on the same page as the next one.</summary>
    public bool KeepWithNext { get; init; }

    /// <summary>True when the paragraph must not be split across pages at all.</summary>
    public bool KeepTogether { get; init; }

    /// <summary>
    /// How many lines must stay together at the foot of a page, or zero for no constraint.
    /// </summary>
    /// <remarks>
    /// The orphan count. Two is the usual setting, and the reason a paragraph sometimes moves wholly to
    /// the next page rather than leaving one line behind.
    /// </remarks>
    public int OrphanLines { get; init; }

    /// <summary>How many lines must stay together at the head of a page, or zero for no constraint.</summary>
    public int WidowLines { get; init; }

    /// <summary>True when the paragraph starts a new page.</summary>
    public bool StartsNewPage { get; init; }

    /// <summary>The width the paragraph's first line has, given the text area's.</summary>
    public Length FirstLineWidth(Length textAreaWidth)
        => Clamp(textAreaWidth - StartIndent - EndIndent - FirstLineIndent);

    /// <summary>The width its other lines have.</summary>
    public Length BodyWidth(Length textAreaWidth)
        => Clamp(textAreaWidth - StartIndent - EndIndent);

    /// <summary>
    /// Where a line starts, measured from the text area's start edge.
    /// </summary>
    /// <remarks>
    /// The first line's own indent is added only to the first line, which is what makes a hanging
    /// indent hang: with a negative first-line indent the first line starts to the left of the rest.
    /// </remarks>
    public Length LineStart(bool isFirstLine)
        => isFirstLine ? StartIndent + FirstLineIndent : StartIndent;

    private static Length Clamp(Length value) => value > Length.Zero ? value : Length.Zero;
}

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

    /// <summary>
    /// True when the paragraph itself reads right to left, whatever its text says.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The paragraph's declared writing mode — ODF's <c>style:writing-mode</c>, OOXML's
    /// <c>w:bidi</c>, RTF's <c>\rtlpar</c>, WW8's <c>sprmPFBiDi</c> — and not a guess from its
    /// content: an Arabic sentence in an English paragraph still starts at the left margin, and an
    /// empty right-to-left paragraph still puts its caret on the right.
    /// </para>
    /// <para>
    /// It decides three separate things, which is why one flag is not enough on its own: the base
    /// embedding level the bidi algorithm resolves against, which side <see cref="StartIndent"/> and
    /// <see cref="FirstLineIndent"/> are measured from, and which edge
    /// <see cref="TextAlignment.Start"/> means. Writer gets all three at once by laying a
    /// right-to-left frame out as though it were left to right and mirroring the result
    /// (<c>SwTextFrame::SwitchLTRtoRTL</c>, <c>sw/source/core/text/txtfrm.cxx:682</c>), which is
    /// exactly what <see cref="ParagraphLayouter"/> does with this.
    /// </para>
    /// </remarks>
    public bool IsRightToLeft { get; init; }

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

    /// <summary>
    /// Which paragraph style the paragraph is set in, or null when its reader does not say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Carried for one purpose: <see cref="HasContextualSpacing"/> suppresses the gap only between
    /// paragraphs of the <em>same style</em>, and Writer decides that by comparing the two nodes'
    /// <c>SwTextFormatColl</c> pointers (<c>lcl_IdenticalStyles</c>,
    /// <c>sw/source/core/layout/flowfrm.cxx:1503</c>) rather than by comparing what the styles say.
    /// The difference is not academic: a heading based on the body style inherits its indents, its
    /// alignment and its line spacing, so any comparison of resolved properties calls the two
    /// identical and swallows the space above every heading in the document.
    /// </para>
    /// <para>
    /// An opaque key rather than a name — a WW8 <c>istd</c> and an RTF <c>\s</c> are numbers — and the
    /// <em>named</em> style rather than the automatic one, since ODF's automatic styles are direct
    /// formatting and LibreOffice gives the node its parent as a format collection. Null means the
    /// reader cannot say, and two nulls are not a match: the readers that know say so, and one that
    /// does not falls back to the older comparison rather than claiming an identity it has not
    /// established.
    /// </para>
    /// </remarks>
    public string? StyleKey { get; init; }

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

    /// <summary>
    /// Whether the tab stops are measured from the paragraph's own indent rather than from the text area.
    /// </summary>
    /// <remarks>
    /// Writer's <c>TABS_RELATIVE_TO_INDENT</c>, and another compatibility flag rather than a property: the
    /// same stop at 12 cm means twelve centimetres into an indented paragraph in an ODF document and twelve
    /// centimetres into the <em>page</em> in a Word one. Both importers say so outright — <c>ww8par.cxx</c>
    /// and <c>writerfilter</c>'s <c>DomainMapper</c> each set it to false, and the registry default for a
    /// native document is true — so it defaults to Writer's answer and every Word-family reader turns it
    /// off. Getting it wrong shifts a tabbed column by the indent, which on a dotted table of contents is
    /// the difference between one line and three.
    /// </remarks>
    public bool TabsRelativeToIndent { get; init; } = true;

    /// <summary>
    /// Whether a justified line may squeeze its blanks below their natural width to fit another word.
    /// </summary>
    /// <remarks>
    /// A document-wide compatibility flag rather than a paragraph property, carried here for the same
    /// reason <see cref="TabsRelativeToIndent"/> is: the layout engine has to know it and a paragraph is
    /// the only thing that reaches it. LibreOffice spells it <c>JustifyLinesWithShrinking</c> and
    /// writerfilter sets it for every file declaring <c>compatibilityMode</c> 15 or more
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:10172</c>). See
    /// <see cref="JustificationShrink"/> for what it does and how far it goes.
    /// </remarks>
    public bool ShrinksJustifiedBlanks { get; init; }

    /// <summary>
    /// Where the paragraph's tab stops are measured from, relative to the text area's start edge.
    /// </summary>
    public Length TabOrigin => TabsRelativeToIndent ? StartIndent : Length.Zero;

    /// <summary>
    /// Where a line's start edge sits relative to <see cref="TabOrigin"/>, which is negative inside a
    /// hanging indent.
    /// </summary>
    /// <param name="isFirstLine">True for the paragraph's first line, which is the only one that hangs.</param>
    public Length TabLineOffset(bool isFirstLine)
        => LineStart(isFirstLine) - TabOrigin;

    /// <summary>
    /// The stop a tab at a position advances to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The first explicit stop strictly beyond the position, or — past the last of them — a left stop at
    /// the next multiple of <see cref="DefaultTabInterval"/>. Strictly beyond, because a tab always moves:
    /// a tab landing exactly on a stop advances to the next one rather than nowhere.
    /// </para>
    /// <para>
    /// The multiple is counted from the paragraph's text start, not from the last explicit stop, which is
    /// what makes the default stops of an untabbed paragraph fall on a regular grid.
    /// </para>
    /// <para>
    /// A tab still inside a hanging indent is the exception, and it is not a small one: it advances to the
    /// paragraph's own indent whatever stops the paragraph declares, which is what makes
    /// "<c>1.1</c> ⇥ <c>Purpose</c>" in a table of contents set the title against the indent instead of
    /// throwing it at a right-aligned leader stop by the margin and wrapping the line. Writer states the
    /// rule in as many words in <c>SwTextFormatter::NewTabPortion</c>
    /// (<c>sw/source/core/text/txttab.cxx</c>): "the new tab portion is inside the hanging indent … a tab
    /// stop at the left margin is allowed … the determined next tab stop is beyond the left margin".
    /// </para>
    /// </remarks>
    /// <param name="position">Where the tab is, measured from <see cref="TabOrigin"/>.</param>
    public TabStop NextTabStop(Length position)
    {
        // The paragraph's own indent, in the same coordinates as the stops. A tab before it is inside the
        // hanging indent, and only the first line of a paragraph with a negative first-line indent has
        // anything before it at all.
        Length indent = TabsRelativeToIndent ? Length.Zero : StartIndent;

        foreach (TabStop stop in TabStops)
        {
            if (stop.Position <= position) continue;

            return position < indent && stop.Position > indent ? new TabStop(indent) : stop;
        }

        if (position < indent) return new TabStop(indent);

        Length interval = DefaultTabInterval > Length.Zero
            ? DefaultTabInterval
            : Length.FromTwips(720);

        // The next multiple strictly past the position: an exact multiple advances a whole interval.
        long steps = (position.Emu / interval.Emu) + 1;
        return new TabStop(Length.FromEmu(steps * interval.Emu));
    }

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

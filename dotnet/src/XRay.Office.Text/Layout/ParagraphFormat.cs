using XRay.Office.Core.Units;

namespace XRay.Office.Text.Layout;

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
    public Length Apply(Length naturalHeight) => Apply(naturalHeight, naturalHeight);

    /// <summary>
    /// The height a line gets, when the height its <em>text</em> wants differs from the height of the
    /// line.
    /// </summary>
    /// <param name="naturalHeight">The line's own height, an as-character object included.</param>
    /// <param name="baseHeight">
    /// The height the line's text alone wants. Proportional spacing is a percentage <em>of this</em>,
    /// added to <paramref name="naturalHeight"/> — never a scaling of the line.
    /// </param>
    /// <remarks>
    /// The two are the same on any line of plain text, and differ on a line an inline picture has made
    /// taller. <c>SwTextFormatter::CalcRealHeight</c> (<c>sw/source/core/text/itrform2.cxx</c>:2441-2453)
    /// is explicit — <em>"extend line height by (nPropLineSpace - 100) percent of the font height"</em> —
    /// and the height it takes the percentage of is <c>GetLineSpacingBaseHeight()</c>, which only a
    /// portion that <c>IsUsedToCalcLineSpacingHeight</c> ever raises. A fly-in-content is not one.
    ///
    /// Measured on <c>1257259179492_2007_TPPT_102_Supporting_Doc_2.doc</c>, a 214.5 pt inline picture
    /// alone in a 12 pt paragraph at 150%: LibreOffice gives the line 221.5 pt and scaling it gives
    /// 321.9, a difference of 100.4 pt on page one that pushed four lines of the introduction onto a
    /// tenth page the reference does not have.
    /// </remarks>
    public Length Apply(Length naturalHeight, Length baseHeight)
    {
        long natural = naturalHeight.Twips;

        // Below a hundred per cent Writer takes the other branch and scales the whole line, object
        // included — `SvxLineSpaceRule::Auto` with `PROP_LINE_SPACING_SHRINKS_FIRST_LINE`, which does
        // `nTmp *= nLineHeight` rather than the base height. Measured on the authored probes: a 150 pt
        // picture at 75% comes back 112.5 pt, which is 150 × 0.75 and not 150 − 25% of the text.
        long basis = Proportion >= 1.0 && baseHeight.Twips > 0 ? baseHeight.Twips : natural;

        long twips = Mode switch
        {
            LineSpacingMode.Proportional => natural + Extra(basis),
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
/// It lives in <c>XRay.Office.Text</c> rather than beside the word-processing model because a
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
    /// Whether a right, centred or decimal stop declared past the text frame's right edge is honoured at
    /// that edge instead of where it was declared — and whether the blank such a stop advances across may
    /// overrun the line's own right edge without breaking the line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's rule and Writer's alone: <c>SwTabPortion::PostFormat</c>
    /// (<c>sw/source/core/text/txttab.cxx</c>:503) sets, above the comment <em>"If the tab position is
    /// larger than the right margin, it gets scaled down by default"</em>,
    /// <c>nRight = std::min(GetTabPos(), rInf.GetTextFrame()-&gt;getFrameArea().Right())</c> when
    /// <c>TabOverSpacing</c> is on — which <c>WriterFilter.cxx</c>:325 turns on for every writerfilter
    /// document — and <c>std::min(GetTabPos(), rInf.Width())</c> only for a document carrying neither
    /// compatibility flag. A <em>left</em> stop past the edge is never clamped: <c>PreFormat</c> breaks
    /// the line there instead, which is what this engine does already.
    /// </para>
    /// <para>
    /// <strong>The frame's edge, not the line's, and the difference is the paragraph's right indent.</strong>
    /// Clamping at the line's edge instead was measured on the corpus as a right-aligned stop landing
    /// short by exactly that indent: 18.09 and 18.10 pt on the two <c>mcar</c> revisions, whose
    /// <c>toc 4</c> declares <c>w:right="360"</c>, and 28.45 pt on <c>EHEST-SMS</c>, whose <c>toc 2</c>
    /// declares <c>w:right="1134"</c> and puts its stop 566 twips inside the frame. A probe rendered
    /// through LibreOffice 26.2.4.2 — one right stop per paragraph at ten positions crossing the text
    /// area's edge, at three right indents — put every stop at its declared position to within 2 twips
    /// and moved none of them by any indent.
    /// </para>
    /// <para>
    /// <strong>The half of the rule that makes that survivable.</strong> A stop inside the right indent
    /// puts the text after the tab past the line's right edge, and a line-filler that counted the tab's
    /// stretch against the line's width would break there — which is how a contents entry becomes four
    /// lines, one for its number, one for its title, one of leader dots and one for its page. Writer
    /// does not count it: for a right, centred or decimal stop it fits the following text with the tab
    /// still one twip wide (<c>PreFormat</c> only calls <c>SetLastTab</c>) and settles the tab's width
    /// afterwards, in <c>PostFormat</c>. So this flag also switches the filler over to that two-pass
    /// width; see <see cref="TabRuler.WidthOf"/>.
    /// </para>
    /// <para>
    /// <strong>What is still approximated.</strong> Writer's bound is an absolute page coordinate
    /// compared against a line-relative stop position, so a stop declared past the text area is honoured
    /// out into the page's right margin, as far as the page edge — the probe above shows a stop at 10799
    /// twips in a 9360-twip text area drawn at 12239 twips, 2879 twips into the margin. The bound here is
    /// the text area's own right edge, so such a stop stops at the margin. That band is bounded by the
    /// overshoot and is a great deal smaller than the indent this replaces.
    /// </para>
    /// <para>
    /// A flag rather than unconditional behaviour because the code that states it is the text frame's,
    /// so it governs a word-processing document and says nothing about a slide's text body or a
    /// spreadsheet cell, which Impress and Calc lay out through other code entirely. Every
    /// word-processing reader turns it on; the presentation and spreadsheet layouts leave it off and
    /// place such a stop exactly where it was declared, as they did before this existed.
    /// </para>
    /// </remarks>
    public bool ClampsTabsAtLineEdge { get; init; }

    /// <summary>
    /// Whether the document carries LibreOffice's <c>TabOverSpacing</c> compatibility setting.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word 2013 and later, and so every file <c>writerfilter</c> reads: it is set unconditionally in
    /// <c>sw/source/writerfilter/filter/WriterFilter.cxx</c>:325, which serves DOCX and RTF. A binary
    /// <c>.doc</c> does not carry it, and ODF carries it only when a settings file states it.
    /// </para>
    /// <para>
    /// <strong>What a <c>.doc</c> carries instead is <c>TAB_OVER_MARGIN</c></strong>, the older and wider
    /// setting — <c>ww8par.cxx</c>:2047, beside the <c>TAB_COMPAT</c> at :1949 — and WriterFilter's own
    /// comment says why the two are not the same thing: <em>"TabOverSpacing … is a subset of
    /// TabOverMargin. TabOverMargin looks at tabs beyond the normal text area, while TabOverSpacing only
    /// refers to a tab beyond the paragraph margin"</em> (:321-324). So the three word-processing formats
    /// take three different branches of <c>PreFormat</c>, not two, and this flag names only one of them.
    /// An earlier note here said the WW8 filter sets <c>TAB_COMPAT</c> "and no more", which is wrong.
    /// </para>
    /// <para>
    /// It decides what a <em>left</em> stop past the frame does to the line, and the two answers are
    /// opposite. With it, <c>SwTabPortion::PreFormat</c> sets <c>bFull</c> and the line breaks at the tab
    /// (<c>sw/source/core/text/txttab.cxx</c>:429-440). Without it, the same tab reaches the
    /// <c>TAB_COMPAT</c> rescue below, which takes the break back where the tab is the paragraph's last
    /// character (<c>txttab.cxx</c>:448-458). So the same footer keeps its page number on one line in a
    /// <c>.doc</c> and does not in a <c>.docx</c>. See <see cref="TabRuler.WidthOf"/>.
    /// </para>
    /// <para>
    /// Distinct from <see cref="ClampsTabsAtLineEdge"/>, which every word-processing reader turns on:
    /// that one is about where an <em>aligned</em> stop past the frame is honoured, and Writer answers
    /// that the same way for both settings bar the boundary it measures against.
    /// </para>
    /// </remarks>
    public bool TabsOverSpacing { get; init; }

    /// <summary>
    /// Whether a paragraph ending in a no-break space and ordinary blanks spills onto two more lines.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Measured, not ported.</strong> A paragraph whose text ends with U+00A0 followed by one
    /// or more U+0020 and nothing else costs Writer <em>two</em> extra line pitches: everything bound
    /// to the no-break space moves onto a line of its own, and an empty line follows it. Under block
    /// adjustment only the first of the two happens. Trailing ordinary blanks cost nothing however many
    /// there are, a no-break space with nothing after it costs nothing, and a visible character after
    /// the blanks removes the effect entirely.
    /// </para>
    /// <para>
    /// Established over nine tails on a real paragraph, five adjustments, a one-line paragraph and a
    /// synthetic body swept from one to four lines — <c>probes/trailing-nbsp-wrap/</c>, which also
    /// records why an ad-hoc version of that last sweep reported the opposite. The C++ this shadows is
    /// <c>SwTextGuess::maybeAdjustPositionsForBlockAdjust</c>
    /// (<c>sw/source/core/text/guess.cxx:78-130</c>), whose own comment says it returns false "to create
    /// a trailing <c>SwHolePortion</c>" and whose gate on block adjustment is exactly the split above;
    /// beside it <c>IsBlank</c> (<c>:47</c>) is the "elided at the end of a line" set and excludes
    /// U+00A0, which <c>TrimTrailingSpaces</c> already agrees with. The portion arithmetic that turns
    /// that hole into two pitches has not been traced, so this reproduces the measured outcome rather
    /// than claiming the mechanism.
    /// </para>
    /// <para>
    /// A flag for the same reason <see cref="ClampsTabsAtLineEdge"/> is one: it was measured through
    /// Writer's text frame and says nothing about a slide's text body or a spreadsheet cell, which
    /// Impress and Calc lay out through other code. Every word-processing reader turns it on.
    /// </para>
    /// </remarks>
    public bool SpillsTrailingNoBreakSpace { get; init; }

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
    public TabStop NextTabStop(Length position) => NextTabStop(position, null);

    /// <summary>
    /// The stop a tab at a position advances to, with a list level's own stop in the search.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The tab that follows a numbering label is an ordinary tab, and the ruler it searches is the
    /// paragraph's with the level's stop <em>merged into it</em> — Writer inserts it into the line's own
    /// copy of the paragraph's <c>SvxTabStopItem</c> in <c>SwLineInfo::InitLineInfo</c>
    /// (<c>sw/source/core/text/inftxt.cxx</c>:124-137) and then runs the same
    /// <c>SwTextFormatter::GetTabStop</c> over it as for any other tab. So the level's stop competes with
    /// the paragraph's rather than pre-empting them: a paragraph stop nearer the pen wins.
    /// </para>
    /// <para>
    /// That is not a nicety. The pen after a label sits inside the hanging indent, so the search position
    /// is <em>behind</em> the paragraph's stops and every one of them is still ahead — including one
    /// declared at the indent itself, which is exactly where a Word bullet list puts its text. Measured on
    /// <c>info-bulletin-601.doc</c> (words/extra-001), whose level states a 36 pt list tab while the
    /// paragraph declares a stop at its own 21.30 pt indent: LibreOffice sets the item's text at 21.30 pt
    /// and preferring the level's stop set it at 36 pt, shortening every bulleted first line by 14.7 pt.
    /// </para>
    /// <para>
    /// The one thing the merge does not do is let the hanging-indent rule below overrule the level's own
    /// stop. Writer guards it with <c>nNextPos != GetListTabStopPosition()</c>
    /// (<c>txttab.cxx</c>:269-272), so a level whose stop is the nearest one ahead keeps it even though
    /// the pen is inside the indent.
    /// </para>
    /// </remarks>
    /// <param name="position">Where the tab is, measured from <see cref="TabOrigin"/>.</param>
    /// <param name="listTabStop">
    /// The level's own stop, in the same coordinates, or null for a tab that is not a label's follower.
    /// </param>
    public TabStop NextTabStop(Length position, Length? listTabStop)
    {
        // The paragraph's own indent, in the same coordinates as the stops. A tab before it is inside the
        // hanging indent, and only the first line of a paragraph with a negative first-line indent has
        // anything before it at all.
        Length indent = TabsRelativeToIndent ? Length.Zero : StartIndent;

        TabStop? nearest = null;

        foreach (TabStop stop in TabStops)
        {
            if (stop.Position <= position) continue;

            nearest = stop;
            break;
        }

        // The level's stop takes its place in the ruler by position, so it wins only where it is the
        // first one past the pen.
        bool tookListStop = false;

        if (listTabStop is { } list && list > position
            && (nearest is not { } found || list < found.Position))
        {
            nearest = new TabStop(list);
            tookListStop = true;
        }

        if (nearest is { } hit)
        {
            return !tookListStop && position < indent && hit.Position > indent
                ? new TabStop(indent)
                : hit;
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

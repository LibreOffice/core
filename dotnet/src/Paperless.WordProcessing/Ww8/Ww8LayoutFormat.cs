using System.Globalization;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// The layout properties a DOC's sprms set on a paragraph, before translation.
/// </summary>
/// <remarks>
/// <para>
/// Nullable throughout, so "the style chain said nothing" stays distinguishable from "it said zero" —
/// which matters because a zero indent is meaningful and a missing one has to fall through to the next
/// layer. The chain is applied by overwriting, so a value set nowhere stays null all the way down.
/// </para>
/// <para>
/// Kept apart from <see cref="ParagraphFormat"/> because the translation needs two things this does not
/// have: the em size, since WW8 states line spacing as a signed multiple whose sign is its mode, and
/// which of the two indent sprms a document used. Both are settled in
/// <see cref="Ww8LayoutFormat.ToParagraphFormat"/>.
/// </para>
/// </remarks>
public readonly record struct Ww8LayoutFormat
{
    /// <summary>The alignment, as <c>sprmPJc</c>'s value.</summary>
    public int? Justification { get; init; }

    /// <summary>
    /// True when the alignment came from <c>sprmPJc80</c>, whose sides are physical.
    /// </summary>
    /// <remarks>
    /// The two sprms carry the same four values and mean different things by them, which is the sort
    /// of difference that is invisible until a document is right to left. <c>sprmPJc</c> (0x2461) is
    /// bidi-relative — zero is the paragraph's start edge — and <c>sprmPJc80</c> (0x2403) is
    /// absolute, so zero is the left margin whichever way the paragraph reads. LibreOffice's own
    /// reader spells the distinction out in <c>SwWW8ImplReader::Read_Justify</c>
    /// (<c>sw/source/filter/ww8/ww8par6.cxx:4805</c>, "tdf#121110: Jc80 justify is absolute, not
    /// bidi-relative"), and RTF sets this too, since its <c>\ql</c> and <c>\qr</c> are physical for
    /// the same reason.
    /// </remarks>
    public bool IsJustificationAbsolute { get; init; }

    /// <summary>
    /// True when the paragraph reads right to left, from <c>sprmPFBiDi</c>.
    /// </summary>
    /// <remarks>
    /// 0x2441, one byte (<c>sw/source/filter/ww8/sprmids.hxx:429</c>). RTF's <c>\rtlpar</c> is the
    /// same statement and lands here too, which is why this is on the shared format rather than in
    /// either reader.
    /// </remarks>
    public bool? IsRightToLeft { get; init; }

    /// <summary>The left indent in twips.</summary>
    public int? LeftIndent { get; init; }

    /// <summary>The right indent in twips.</summary>
    public int? RightIndent { get; init; }

    /// <summary>The first line's extra indent in twips, which may be negative.</summary>
    public int? FirstLineIndent { get; init; }

    /// <summary>The space above the paragraph in twips.</summary>
    public int? SpaceBefore { get; init; }

    /// <summary>The space below it in twips.</summary>
    public int? SpaceAfter { get; init; }

    /// <summary>
    /// The twips <c>sprmPFDyaBeforeAuto</c> and <c>sprmPFDyaAfterAuto</c> stand for.
    /// </summary>
    /// <remarks>
    /// Fourteen points, and not a property of the paragraph: WW8 states auto-spacing as a flag and
    /// LibreOffice substitutes a constant for it in <c>GetParagraphAutoSpace</c>
    /// (<c>sw/source/filter/ww8/ww8par6.cxx:4609</c>) with the comment "Seems to be always 14points in
    /// this case". It is the same number the DOCX path uses for <c>w:beforeAutospacing</c>, because it
    /// is the same rule reached through a different spelling.
    /// </remarks>
    public const int HtmlAutoSpacingTwips = 280;

    /// <summary>
    /// What the same two sprms stand for once <c>fDontUseHTMLAutoSpacing</c> is set.
    /// </summary>
    /// <remarks>
    /// Five points. The flag does not mean "no spacing" — it means the pre-HTML value, which is what
    /// the other branch of <c>GetParagraphAutoSpace</c> returns.
    /// </remarks>
    public const int WordAutoSpacingTwips = 100;

    /// <summary>
    /// True when the space above came from <c>sprmPFDyaBeforeAuto</c> rather than being stated.
    /// </summary>
    /// <remarks>
    /// Kept apart from <see cref="SpaceBefore"/> because the suppression rules ask how the margin was
    /// arrived at, not what it is: LibreOffice zeroes an <em>auto</em> margin at a cell's top edge and on
    /// the document's first paragraph, and leaves a stated one of the same size alone.
    /// </remarks>
    public bool? HasAutoSpaceBefore { get; init; }

    /// <inheritdoc cref="HasAutoSpaceBefore"/>
    public bool? HasAutoSpaceAfter { get; init; }

    /// <summary>
    /// <c>sprmPDyaLine</c>'s <c>dyaLine</c>: the spacing, whose sign is its mode.
    /// </summary>
    /// <remarks>
    /// Positive means at-least and negative means exact, and the magnitude is twips — <em>unless</em>
    /// <see cref="IsMultipleLineSpacing"/> is set, in which case it is a multiple in two-hundred-and-
    /// fortieths of a line, the same unit OOXML's <c>auto</c> rule uses. Three meanings in one signed
    /// number, which is why they are kept raw here and resolved together.
    /// </remarks>
    public int? LineSpacing { get; init; }

    /// <summary>True when <c>sprmPDyaLine</c>'s <c>fMultLinespace</c> is set.</summary>
    public bool IsMultipleLineSpacing { get; init; }

    /// <summary>
    /// The paragraph's tab stops, already accumulated through the style chain.
    /// </summary>
    /// <remarks>
    /// A resolved list rather than the sprm's own contents, because <c>sprmPChgTabsPapx</c> states a
    /// <em>change</em>: a set of positions to delete and a set to add. Applying the style chain and then
    /// the direct formatting in order is what turns those changes into a list, which is exactly the order
    /// the sprms are applied in anyway.
    /// </remarks>
    public IReadOnlyList<TabStop>? TabStops { get; init; }

    /// <summary>True when the paragraph must not be split across pages.</summary>
    public bool? KeepTogether { get; init; }

    /// <summary>True when it must stay with the paragraph after it.</summary>
    public bool? KeepWithNext { get; init; }

    /// <summary>True when it starts a page.</summary>
    public bool? StartsNewPage { get; init; }

    /// <summary>True when widow and orphan control applies.</summary>
    public bool? HasWidowControl { get; init; }

    /// <summary>True when the space between two paragraphs of the same style is suppressed.</summary>
    public bool? HasContextualSpacing { get; init; }

    /// <summary>
    /// The paragraph style's <c>istd</c>, for the "same style" half of contextual spacing.
    /// </summary>
    /// <remarks>
    /// Not a layout sprm but the index the sprms were resolved through, so it is set by the resolver
    /// rather than by <c>ApplyLayoutSprms</c>. See <see cref="ParagraphFormat.StyleKey"/>.
    /// </remarks>
    public ushort? StyleIndex { get; init; }

    /// <summary>The font size in half-points, from <c>sprmCHps</c>.</summary>
    public int? FontSizeHalfPoints { get; init; }

    /// <summary>The font table index, from <c>sprmCRgFtc0</c>.</summary>
    public int? FontIndex { get; init; }

    /// <summary>True when the text is bold.</summary>
    public bool? IsBold { get; init; }

    /// <summary>True when it is italic.</summary>
    public bool? IsItalic { get; init; }

    /// <summary>True when <c>sprmCFCaps</c> draws the run in capitals.</summary>
    public bool? IsCapitalised { get; init; }

    /// <summary>True when <c>sprmCFSmallCaps</c> draws it in small capitals.</summary>
    public bool? IsSmallCapitalised { get; init; }

    /// <summary>
    /// The case the run is drawn in, from the two toggles above.
    /// </summary>
    /// <remarks>
    /// Full capitals win where a document sets both, because <c>SvxCaseMapItem</c> holds one value and
    /// LibreOffice's <c>SwWW8ImplReader</c> applies <c>sprmCFCaps</c> after <c>sprmCFSmallCaps</c> in
    /// sprm order — which is the order they appear in a CHPX.
    /// </remarks>
    public Layout.PageCaseMap CaseMap
        => IsCapitalised == true ? Layout.PageCaseMap.Uppercase
            : IsSmallCapitalised == true ? Layout.PageCaseMap.SmallCaps
            : Layout.PageCaseMap.None;

    /// <summary>
    /// True when <c>sprmCKul</c> asks for a rule under the run.
    /// </summary>
    /// <remarks>
    /// Not a toggle and not a boolean: the operand is a <c>kul</c> naming the line's <em>style</em>, of
    /// which nought is "none" and 255 is "none, and cancel whatever the style said". Every other value
    /// is some kind of line, and all of them are drawn as one — <c>SwWW8ImplReader::Read_Underline</c>
    /// (<c>sw/source/filter/ww8/ww8par6.cxx</c>) maps eleven <c>kul</c> values onto seven
    /// <c>FontLineStyle</c>s, and nothing below this models more than one.
    /// </remarks>
    public bool? IsUnderlined { get; init; }

    /// <summary>
    /// True when <c>sprmCFStrike</c> or <c>sprmCFDStrike</c> draws a rule through the run.
    /// </summary>
    /// <remarks>
    /// Two sprms and one flag, matching what the extraction half of this reader already does with them:
    /// the doubled form is a second line rather than a different decoration, and the page model carries
    /// one rule.
    /// </remarks>
    public bool? IsStruckThrough { get; init; }

    /// <summary>
    /// True when the run asks for pair kerning, from <c>sprmCHpsKern</c>.
    /// </summary>
    /// <remarks>
    /// The operand is a font size in half-points — the size at or above which Word kerns — and
    /// LibreOffice keeps only whether it is nonzero:
    /// <c>NewAttr(SvxAutoKernItem(static_cast&lt;bool&gt;(nAutoKern), RES_CHRATR_AUTOKERN))</c>
    /// (<c>SwWW8ImplReader::Read_FontKern</c>, <c>sw/source/filter/ww8/ww8par6.cxx:4184</c>). RTF's
    /// <c>\kerning</c> is the same statement and lands here too, which is why this is on the shared
    /// format rather than in either reader.
    /// </remarks>
    public bool? AutoKerning { get; init; }

    /// <summary>The Windows language id, from <c>sprmCRgLid0</c>.</summary>
    public int? LanguageId { get; init; }

    /// <summary>
    /// The text colour, or null for the automatic colour.
    /// </summary>
    /// <remarks>
    /// WW8 states it two ways and a document can carry either. <c>sprmCIco</c> is an index into a fixed
    /// seventeen-entry palette whose first entry is "automatic"; <c>sprmCCv</c> is a full COLORREF, which
    /// is <em>BGR</em> rather than RGB. Reading the latter without swapping the outer bytes turns every
    /// red word blue.
    /// </remarks>
    public Colour? Colour { get; init; }

    /// <summary>
    /// The band drawn behind the run, or null when it has none.
    /// </summary>
    /// <remarks>
    /// Word's highlighter, <c>sprmCHighlight</c> — an index into the same seventeen-colour palette
    /// <c>sprmCIco</c> uses, whose nought entry is "automatic" and means <em>no</em> band here rather
    /// than the document's default colour. Distinct from cell or paragraph shading, which is a property
    /// of the box rather than of the characters.
    /// </remarks>
    public Colour? Highlight { get; init; }

    /// <summary>
    /// The superscript or subscript <c>sprmCIss</c> asks for, or null when the run states neither.
    /// </summary>
    /// <remarks>
    /// Unresolved, as everywhere else: the rise is a fraction of the <em>face's</em> height, and this reader
    /// has no faces. WW8's other spelling, <c>sprmCHpsPos</c>, states a half-point offset outright and is
    /// not read yet — a document using one gets no shift rather than a wrong one.
    /// </remarks>
    public Layout.Escapement? Escapement { get; init; }

    /// <summary>
    /// Translates into the layout engine's own properties.
    /// </summary>
    /// <param name="emSize">
    /// The em size the paragraph's text is set at, which the multiple-spacing mode needs: a spacing of
    /// 360 means one and a half lines and a line's height depends on the font.
    /// </param>
    public ParagraphFormat ToParagraphFormat(Length emSize)
    {
        // On unless the document turns it off, which is Word's default and not the struct's. A DOC
        // states widow control with sprmPFWidowControl and most never state it at all: the flag lives
        // on the root paragraph style, and Word writes a style's properties only where they differ
        // from the built-in defaults. LibreOffice restores the same default explicitly —
        // WW8RStyle::Set1StyleDefaults (sw/source/filter/ww8/ww8par2.cxx:3751) puts SvxWidowsItem(2)
        // and SvxOrphansItem(2) on every paragraph style with no parent that did not set the sprm,
        // and its flat-ODF export of a document whose Normal style carries no sprms at all shows
        // fo:widows="2" on Standard.
        //
        // Reading the absence as "off" is not a small difference: it lets a paragraph's last line
        // stand alone at the top of a page, and a page that keeps two lines it should have pushed
        // shifts every paragraph after it.
        bool widows = HasWidowControl ?? true;

        bool rightToLeft = IsRightToLeft ?? false;

        return new ParagraphFormat
        {
            IsRightToLeft = rightToLeft,
            Alignment = Alignment(Justification, IsJustificationAbsolute && rightToLeft),
            TabStops = TabStops ?? [],
            StartIndent = Twips(LeftIndent),
            EndIndent = Twips(RightIndent),
            FirstLineIndent = Twips(FirstLineIndent),
            SpaceBefore = Twips(SpaceBefore),
            SpaceAfter = Twips(SpaceAfter),
            HasContextualSpacing = HasContextualSpacing ?? false,
            StyleKey = StyleIndex?.ToString(CultureInfo.InvariantCulture),
            LineSpacing = Spacing(),
            KeepWithNext = KeepWithNext ?? false,
            KeepTogether = KeepTogether ?? false,

            // Word states one flag where ODF states two counts, and it means two of each.
            OrphanLines = widows ? 2 : 0,
            WidowLines = widows ? 2 : 0,
            StartsNewPage = StartsNewPage ?? false,
        };
    }

    /// <summary>
    /// The alignment, from <c>sprmPJc</c>'s numbering.
    /// </summary>
    /// <remarks>
    /// Word's own order: 0 left, 1 centre, 2 right, 3 justified. There are further values for the
    /// distributed and Thai variants, and treating an unknown one as left is what Word does with them.
    /// The two ends swap when the value is <em>physical</em> and the paragraph is right to left —
    /// see <see cref="IsJustificationAbsolute"/> — because the left margin is then the paragraph's
    /// end rather than its start.
    /// </remarks>
    private static TextAlignment Alignment(int? justification, bool swapEnds) => justification switch
    {
        1 => TextAlignment.Centre,
        2 => swapEnds ? TextAlignment.Start : TextAlignment.End,
        3 => TextAlignment.Justify,
        4 => TextAlignment.Distribute,
        _ => swapEnds ? TextAlignment.End : TextAlignment.Start,
    };

    /// <summary>
    /// The line spacing, from the three meanings <c>dyaLine</c> packs into one signed number.
    /// </summary>
    /// <remarks>
    /// The multiple flag is checked first because it changes the unit as well as the mode: with it set
    /// the value is two-hundred-and-fortieths of a line and its sign is irrelevant, and without it the
    /// value is twips whose sign chooses between at-least and exact. Reading a multiple as twips gives a
    /// line height of eighteen points where the document asked for one and a half lines.
    /// </remarks>
    private LineSpacingRule Spacing()
    {
        if (LineSpacing is not { } value || value == 0) return LineSpacingRule.SingleSpaced;

        if (IsMultipleLineSpacing) return LineSpacingRule.Multiple(Math.Abs(value) / 240.0);

        return value > 0
            ? LineSpacingRule.AtLeast(Length.FromTwips(value))
            : LineSpacingRule.Exactly(Length.FromTwips(-value));
    }

    private static Length Twips(int? value)
        => value is { } twips ? Length.FromTwips(twips) : Length.Zero;
}

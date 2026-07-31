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

    /// <summary>The font size in half-points, from <c>sprmCHps</c>.</summary>
    public int? FontSizeHalfPoints { get; init; }

    /// <summary>The font table index, from <c>sprmCRgFtc0</c>.</summary>
    public int? FontIndex { get; init; }

    /// <summary>True when the text is bold.</summary>
    public bool? IsBold { get; init; }

    /// <summary>True when it is italic.</summary>
    public bool? IsItalic { get; init; }

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
        bool widows = HasWidowControl ?? false;

        return new ParagraphFormat
        {
            Alignment = Alignment(Justification),
            TabStops = TabStops ?? [],
            StartIndent = Twips(LeftIndent),
            EndIndent = Twips(RightIndent),
            FirstLineIndent = Twips(FirstLineIndent),
            SpaceBefore = Twips(SpaceBefore),
            SpaceAfter = Twips(SpaceAfter),
            HasContextualSpacing = HasContextualSpacing ?? false,
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
    /// </remarks>
    private static TextAlignment Alignment(int? justification) => justification switch
    {
        1 => TextAlignment.Centre,
        2 => TextAlignment.End,
        3 => TextAlignment.Justify,
        4 => TextAlignment.Distribute,
        _ => TextAlignment.Start,
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

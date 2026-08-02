using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;

namespace Paperless.Presentations.Layout;

/// <summary>Where a text body sits inside its shape, vertically.</summary>
/// <remarks>
/// DrawingML's <c>a:bodyPr/@anchor</c> and ODF's <c>draw:textarea-vertical-align</c>, which
/// spell the same three positions differently. Justified anchoring — spreading the paragraphs to
/// fill the shape — is a fourth value both formats have and neither corpus deck uses; it is read
/// as <see cref="Top"/> rather than silently as <see cref="Middle"/>, which is what LibreOffice
/// falls back to for a single paragraph anyway.
/// </remarks>
public enum TextAnchor
{
    /// <summary>The text block starts at the top of the text rectangle.</summary>
    Top = 0,

    /// <summary>It is centred vertically.</summary>
    Middle,

    /// <summary>It ends at the bottom.</summary>
    Bottom,
}

/// <summary>
/// A shape's text body before layout: its paragraphs, its insets, and how it is anchored.
/// </summary>
/// <remarks>
/// The presentation family's equivalent of the word processor's <c>PageParagraph</c> list, and
/// deliberately its own type rather than a reuse: a slide's text is bounded by the shape rather
/// than flowed down a page, so what layout needs to know is the rectangle and the anchor, and
/// none of the pagination properties — widows, keep-with-next, page breaks — mean anything.
/// </remarks>
public sealed record SlideTextBody
{
    /// <summary>The paragraphs, in order.</summary>
    public IReadOnlyList<SlideParagraph> Paragraphs { get; init; } = [];

    /// <summary>
    /// The insets between the shape's text rectangle and the text.
    /// </summary>
    /// <remarks>
    /// Defaulted to DrawingML's own defaults — 0.1 inch left and right, 0.05 inch top and bottom
    /// (<c>a:bodyPr</c>'s <c>lIns</c>, <c>tIns</c>, <c>rIns</c>, <c>bIns</c>) — because a body
    /// that states none gets exactly those, and a reader defaulting them to zero puts every line
    /// of every unstated text box 7.2 pt too far left.
    /// </remarks>
    public Margins Insets { get; init; } = DefaultInsets;

    /// <summary>DrawingML's default text insets: 91440 EMU across, 45720 EMU down.</summary>
    public static Margins DefaultInsets { get; } = new(
        Length.FromEmu(91440), Length.FromEmu(45720),
        Length.FromEmu(91440), Length.FromEmu(45720));

    /// <summary>Where the block sits vertically.</summary>
    public TextAnchor Anchor { get; init; }

    /// <summary>
    /// How far the text is turned inside the shape, clockwise, in radians.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:bodyPr/@rot</c>, and what a SmartArt <c>autoTxRot</c> resolves to. It is <em>not</em>
    /// the shape's own rotation: the shape stays where it is and only its text turns, which is
    /// why it belongs to the body rather than to the placement. LibreOffice keeps the two apart
    /// the same way, as <c>TextPreRotateAngle</c> beside <c>RotateAngle</c>.
    /// </para>
    /// <para>
    /// A quarter turn swaps the text rectangle's width and height about its centre, because the
    /// lines then run down the shape rather than across it; a half turn leaves the rectangle
    /// alone. Only multiples of a quarter turn arise: <c>autoTxRot</c> produces nothing else.
    /// </para>
    /// </remarks>
    public double Rotation { get; init; }

    /// <summary>
    /// Whether the text is shrunk until it fits the shape — DrawingML's <c>a:normAutofit</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// When this is set the layouter solves the fit itself and <see cref="FontScale"/> is
    /// <em>ignored</em>, because that is what the reference does: LibreOffice 24.2 reads
    /// <c>a:normAutofit/@fontScale</c> into a field
    /// (<c>oox/source/drawingml/textbodypropertiescontext.cxx:236</c>) and never reads that field
    /// again, so the authoring application's stated answer is discarded and
    /// <c>SdrTextObj::autoFitTextForCompatibility</c> searches for its own. See
    /// <see cref="SlideTextLayout"/> for the search and for what it is measured against.
    /// </para>
    /// <para>
    /// <c>a:normAutofit/@lnSpcReduction</c> is modelled nowhere at all, which is deliberate: the
    /// same handler does not read it either — the <c>normAutofit</c> case reads
    /// <c>XML_fontScale</c> and nothing else — so a body carrying one must lay out exactly as a
    /// body that does not. Paperless did apply it, and it was worth 20 per cent of a line on the
    /// one shape in <c>slides/batch-001</c> that states it: the subtitle of
    /// <c>BMFE-06-03 (Gerflor) Smoke Density and Toxicity.pptx</c> shrank its lines, so the
    /// fit search thought the text nearly fitted unshrunk and drew it at 20 pt where the
    /// reference draws 15.
    /// </para>
    /// <para>
    /// This is a text-only fit. <c>a:spAutoFit</c> is the other direction — the shape grows to its
    /// text rather than the text shrinking to the shape — and is not this flag.
    /// </para>
    /// </remarks>
    public bool AutoFit { get; init; }

    /// <summary>
    /// The multiplier <c>a:normAutofit/@fontScale</c> asks for, or one when it states none.
    /// </summary>
    /// <remarks>
    /// Applied to every run's size when <see cref="AutoFit"/> is <em>not</em> set — which after
    /// the fit search means the ODF path and hand-built bodies only. The value in the file is what
    /// the authoring application arrived at when it last shrank the text to fit.
    /// </remarks>
    public double FontScale { get; init; } = 1.0;


    /// <summary>
    /// Whether the text wraps at the shape's width.
    /// </summary>
    /// <remarks>
    /// <c>a:bodyPr/@wrap="none"</c> means it does not: the line runs on past the shape and the
    /// shape grows around it. Modelled as an unbounded width rather than as clipping, which is
    /// what makes a `wrap="none"` label come out on one line as its author saw it.
    /// </remarks>
    public bool Wraps { get; init; } = true;

    /// <summary>
    /// Whether the line height comes from the font size rather than from the font's metrics.
    /// </summary>
    /// <remarks>
    /// <para>
    /// EditEngine's <c>FixedCellHeight</c>, which ODF spells
    /// <c>style:font-independent-line-spacing</c>. When it is on the ascent is the font height
    /// outright and the line is 1.2 times it, whatever face the text is set in
    /// (<c>editeng/source/editeng/impedit3.cxx:501,3138-3141</c>); when it is off the face's own
    /// ascent and descent decide, as they do in a word processor.
    /// </para>
    /// <para>
    /// True by default because that is what a PPTX gets: the importer sets it on every text body
    /// it reads (<c>oox/source/ppt/pptshapecontext.cxx:186</c>). A natively authored ODP states
    /// it per paragraph style and usually does not, which is why the two paths give the same
    /// deck slightly different baselines and why this is a property of the body rather than a
    /// constant.
    /// </para>
    /// </remarks>
    public bool FontIndependentLineSpacing { get; init; } = true;
}

/// <summary>One paragraph of a shape's text.</summary>
/// <param name="Text">Its text, without a terminating mark.</param>
/// <param name="Runs">
/// Its runs, partitioning the text. Never empty for non-empty text: a paragraph with no stated
/// formatting still carries one run, so that the size an empty line is as tall as is known.
/// </param>
/// <param name="Alignment">How its lines are placed across the text rectangle.</param>
/// <param name="SpaceBefore">The space above it.</param>
/// <param name="SpaceAfter">The space below it.</param>
/// <param name="LineSpacing">Its line-spacing rule.</param>
/// <param name="StartIndent">Its indent from the start edge.</param>
/// <param name="FirstLineIndent">The extra indent on its first line, negative for a hanging one.</param>
/// <param name="Language">A BCP 47 tag, for the language-specific break rules.</param>
/// <param name="Marker">The bullet or number drawn before it, or null when it has none.</param>
public sealed record SlideParagraph(
    string Text,
    IReadOnlyList<SlideTextRun> Runs,
    TextAlignment Alignment = TextAlignment.Start,
    Length SpaceBefore = default,
    Length SpaceAfter = default,
    LineSpacingRule LineSpacing = default,
    Length StartIndent = default,
    Length FirstLineIndent = default,
    string? Language = null,
    SlideMarker? Marker = null)
{
    /// <summary>The slide formats' own default tab distance: one inch.</summary>
    public static Length DefaultTabDistance { get; } = Length.FromEmu(Length.EmuPerInch);

    /// <summary>
    /// How far apart the stops a tab advances to are, when the paragraph states none of its own.
    /// </summary>
    /// <remarks>
    /// <strong>A slide's is an inch, not the half inch a word processor uses.</strong> PowerPoint
    /// stores it as 0x240 master units and DrawingML as <c>a:defTabSz</c> defaulting to 914400
    /// EMU, and both are one inch; <see cref="ParagraphFormat.DefaultTabInterval"/> defaults to
    /// Word's 720 twips because that is what a document is. The difference compounds: a paragraph
    /// positioned by three tabs lands an inch and a half to the left of where it belongs, which on
    /// a ten-inch slide is fifteen per cent of the page.
    /// </remarks>
    public Length DefaultTabInterval { get; init; } = DefaultTabDistance;
}

/// <summary>
/// The bullet or number a paragraph is labelled with.
/// </summary>
/// <remarks>
/// <para>
/// A marker is drawn as its own glyph run at its own pen, in its own face and usually at its own
/// size — LibreOffice writes it as a separate <c>/Lbl</c> block in the PDF, and on
/// <c>deck-features.pptx</c>'s outline that is a 12.6 pt run beside 28 pt text, because
/// <c>a:buSzPct val="45000"</c> says 45%.
/// </para>
/// <para>
/// It is <em>not</em> part of the paragraph's text, which is why it is here rather than prefixed
/// to it: a marker does not wrap, does not participate in the line breaking, and would change
/// every character offset the runs index by if it were spliced in.
/// </para>
/// </remarks>
/// <param name="Text">The characters to draw.</param>
/// <param name="Typeface">The family it is set in, or null for the paragraph's own.</param>
/// <param name="Scale">Its size as a fraction of the first run's, one for the same size.</param>
/// <param name="Colour">Its colour, or null for the first run's.</param>
/// <param name="IsSymbol">
/// Whether it is a fixed character rather than a generated number, which decides where it sits
/// vertically.
/// <para>
/// <strong>The two are placed by different rules and the difference is a point.</strong>
/// <c>Outliner::StripBullet</c> branches on <c>SVX_NUM_CHAR_SPECIAL</c>: a symbol is drawn from
/// the bullet <em>area's</em> bottom, which centres it against the line's text, and anything else
/// is drawn at <c>nFirstLineMaxAscent</c>, which is the text's own baseline
/// (<c>editeng/source/outliner/outliner.cxx:918</c>). Measured on
/// <c>slide-shape-features.pptx</c>, whose list is <c>a:buAutoNum</c>: LibreOffice draws its
/// first number at 89.972 and centring it would put it at 89.036.
/// </para>
/// </param>
public readonly record struct SlideMarker(
    string Text,
    string? Typeface = null,
    double Scale = 1.0,
    Colour? Colour = null,
    bool IsSymbol = true);

/// <summary>
/// A run raised or lowered off its baseline, and shrunk while it is up there.
/// </summary>
/// <remarks>
/// <para>
/// Two numbers rather than one because a slide's formats state two: DrawingML's
/// <c>a:rPr/@baseline</c> gives the offset alone and the importer supplies the size
/// (<c>oox/source/drawingml/textcharacterproperties.cxx:196-199</c>), and a binary PowerPoint's
/// <c>PPT_CharAttr_Escapement</c> does the same
/// (<c>filter/source/msfilter/svdfppt.cxx:5764-5775</c>). Both end as one
/// <c>SvxEscapementItem(nEsc, nProp)</c>, which is this pair.
/// </para>
/// <para>
/// <strong>The percentage is of the em size here, not of the font's height.</strong> That is
/// where a slide differs from a word processor: EditEngine draws the run at
/// <c>GetFontSize().Height() × nEsc / 100</c> above the pen
/// (<c>editeng/source/items/svxfont.cxx:549-558</c>), where Writer's <c>swfont.cxx</c> takes the
/// same percentage of the unshrunk font's ascent-plus-descent. Using the wrong one of the two
/// misplaces a superscript by about a fifth of its rise.
/// </para>
/// <para>
/// The size matters more than the offset does, because it moves line breaks: a 12 pt run set at
/// 58% is 42% narrower, so a line that fits with the shrink wraps without it. Measured on
/// <c>slides/batch-003/pptx/NCW-2024-Guide-.pptx</c>, whose dates are written
/// <c>5<sup>th</sup> March</c>: drawing the ordinals full size wraps one line of a text box that
/// already overflows the slide, which pushes its last paragraph off the bottom edge.
/// </para>
/// </remarks>
/// <param name="Percent">
/// How far the run moves, as a percentage of its em size; positive raises it and negative lowers
/// it.
/// </param>
/// <param name="Proportion">
/// The size the run is set at, as a percentage of the size it would otherwise take. Zero and 100
/// both mean no change, so a default-constructed value is "no escapement at all".
/// </param>
public readonly record struct SlideEscapement(int Percent, int Proportion)
{
    /// <summary>The size an escaped run is set at when the file states only an offset.</summary>
    /// <remarks><c>DFLT_ESC_PROP</c>, <c>include/editeng/escapementitem.hxx:30</c>.</remarks>
    public const int AutomaticProportion = 58;

    /// <summary>Neither moved nor resized.</summary>
    public static SlideEscapement None => default;

    /// <summary>True when the run sits on its baseline at its own size.</summary>
    public bool IsNone => Percent == 0 && Proportion is 0 or 100;

    /// <summary>The size the run is actually set at, given the size it would otherwise take.</summary>
    public Length SizeOf(Length emSize)
        => Proportion is 0 or 100 ? emSize : emSize * (Proportion / 100.0);

    /// <summary>How far the run sits above its baseline, negative for a subscript.</summary>
    /// <param name="emSize">The size the run would take were it not escaped.</param>
    public Length RiseOf(Length emSize)
        => Percent == 0 ? Length.Zero : emSize * (Percent / 100.0);
}

/// <summary>One run of a paragraph: a range of its text with its own face, size and colour.</summary>
/// <param name="Start">The run's first character.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="Typeface">The family it asks for, or null for the deck's default.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1–1000 scale.</param>
/// <param name="IsItalic">Whether it is italic.</param>
/// <param name="Colour">The colour it is drawn in.</param>
/// <param name="Tracking">
/// A fixed distance added between the run's characters — <c>a:rPr/@spc</c>, stated in hundredths
/// of a point and commonly negative. See <see cref="Paperless.Text.Layout.FormattedRun.Tracking"/>
/// for how it is charged.
/// </param>
/// <param name="IsUnderlined">
/// Whether a rule is drawn under it. A decoration rather than a glyph in every format here —
/// <c>a:rPr/@u</c> in DrawingML, bit 2 of a PPT character-property mask — so it moves no line
/// break and is drawn from the face's own <c>post</c> metrics after the text is placed.
/// </param>
/// <param name="IsStruckThrough">Whether a rule is drawn through it.</param>
/// <param name="Escapement">
/// How far off its baseline the run sits and how much it shrinks to sit there — a superscript or
/// a subscript. Unlike the decorations above, this <em>does</em> move line breaks, because the
/// shrink is what makes the run narrower.
/// </param>
public readonly record struct SlideTextRun(
    int Start,
    int Length,
    string? Typeface,
    Length Size,
    int Weight,
    bool IsItalic,
    Colour Colour,
    Length Tracking = default,
    bool IsUnderlined = false,
    bool IsStruckThrough = false,
    SlideEscapement Escapement = default)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;
}

/// <summary>
/// Resolves the faces a slide's text needs, once per distinct request.
/// </summary>
/// <remarks>
/// The same shape as the word processor's cache and for the same reason: a deck has a handful of
/// typefaces and hundreds of runs, and resolving one means walking a substitution chain and
/// reading a font file. Its own type rather than a shared one because the two libraries sit at
/// the same layer and neither may depend on the other.
/// </remarks>
public sealed class SlideFonts
{
    private readonly SystemFontResolver _fonts;
    private readonly Dictionary<(string?, int, bool), (OpenTypeFace? Face, FontReference? Reference)>
        _resolved = [];

    /// <summary>Creates a cache over a resolver, or over the installed fonts.</summary>
    /// <param name="fonts">The resolver to use, or null to build one over the installed fonts.</param>
    public SlideFonts(SystemFontResolver? fonts = null)
        => _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());

    /// <summary>The substitutions made so far, which is the first thing a comparison checks.</summary>
    public IReadOnlyList<FontSubstitution> Substitutions => _fonts.Substitutions;

    /// <summary>The face and reference a request resolves to, both null when nothing could be read.</summary>
    public (OpenTypeFace? Face, FontReference? Reference) Resolve(
        string? family, int weight, bool isItalic)
    {
        (string?, int, bool) key = (family, weight, isItalic);
        if (_resolved.TryGetValue(key, out (OpenTypeFace?, FontReference?) cached)) return cached;

        (OpenTypeFace? Face, FontReference? Reference) resolved = default;
        try
        {
            FontReference reference = _fonts.Resolve(
                new FontRequest(family ?? string.Empty, weight, isItalic));
            resolved = (_fonts.LoadOpenType(reference), reference);
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // A face that cannot be read costs the shape its text, not the deck its layout.
        }

        _resolved[key] = resolved;
        return resolved;
    }
}

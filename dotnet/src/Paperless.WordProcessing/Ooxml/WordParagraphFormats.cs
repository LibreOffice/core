using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>The character formatting a stretch of a paragraph's text is set in.</summary>
/// <param name="FamilyName">The family the document asks for, before substitution.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">True when the text is italic.</param>
/// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
/// <param name="Colour">The colour the text is drawn in, or null when nothing set one.</param>
/// <param name="Escapement">
/// The superscript or subscript <c>w:vertAlign</c> asks for, unresolved. Kept as the pair of percentages
/// rather than as a length and a size because the rise is a fraction of the face's <em>height</em>, which is
/// not known until the face has been loaded — see <see cref="Layout.Escapement"/>.
/// </param>
/// <param name="CaseMap">The case <c>w:caps</c> or <c>w:smallCaps</c> asks the text to be drawn in.</param>
/// <param name="Highlight">
/// The band <c>w:highlight</c> draws behind the text, or null when it names none.
/// </param>
/// <param name="IsUnderlined">True when <c>w:u</c> names a line style other than <c>none</c>.</param>
/// <param name="IsStruckThrough">True when <c>w:strike</c> or <c>w:dstrike</c> is on.</param>
/// <param name="AutoKerning">
/// True when the run asks for pair kerning, which <c>w:kern</c> is the only way to ask for.
/// <em>Off</em> is the default, and that is not an omission — see
/// <see cref="WordParagraphFormats.AutoKerningOf"/>.
/// </param>
/// <param name="Tracking">
/// The fixed distance <c>w:spacing</c> puts between the run's characters, zero for none — and not to be
/// confused with the <c>w:spacing</c> of a <c>w:pPr</c>, which is the gap between paragraphs. This one
/// lives in <c>w:rPr</c>, is stated in twips, and is commonly negative. See
/// <see cref="Paperless.Text.Layout.FormattedRun.Tracking"/> for what a reader owes it.
/// </param>
public readonly record struct WordTextStyle(
    string? FamilyName,
    Length Size,
    int Weight,
    bool IsItalic,
    string? Language,
    Colour? Colour = null,
    Layout.Escapement Escapement = default,
    PageCaseMap CaseMap = PageCaseMap.None,
    Colour? Highlight = null,
    bool IsUnderlined = false,
    bool IsStruckThrough = false,
    bool AutoKerning = false,
    Length Tracking = default)
{
    /// <summary>The key a face cache is keyed on: what actually decides which font file is loaded.</summary>
    public (string? Family, int Weight, bool Italic) FaceKey => (FamilyName, Weight, IsItalic);
}

/// <summary>
/// Which of a list level's indents a paragraph takes from the level rather than from itself.
/// </summary>
/// <remarks>
/// Writer's <c>::sw::ListLevelIndents</c>. Two flags rather than one answer because the two margins are
/// separate items there — a style stating only a left indent leaves the level's hanging one in force.
/// </remarks>
[Flags]
internal enum ListLevelIndents
{
    /// <summary>Neither: the paragraph's own indents stand.</summary>
    No = 0,

    /// <summary>The level's <c>w:hanging</c> or <c>w:firstLine</c> applies.</summary>
    FirstLine = 1,

    /// <summary>The level's <c>w:start</c> or <c>w:left</c> applies.</summary>
    LeftMargin = 2,
}

/// <summary>
/// Resolves a DOCX paragraph's properties into the layout properties the engine takes.
/// </summary>
/// <remarks>
/// <para>
/// The layers are already walked by <see cref="WordStyles"/>, including ECMA-376's toggle rule for the
/// run properties. What is left is the translation, and OOXML's units are its own hazard: nearly
/// everything is twips, but a font size is <em>half-points</em> and line spacing in the <c>auto</c> rule
/// is <em>two-hundred-and-fortieths of a line</em> rather than a percentage. Reading a font size as
/// points halves every document, and reading <c>w:line="360"</c> as anything but 150% spaces it wrongly.
/// </para>
/// <para>
/// The other trap is that direct paragraph formatting and the style chain have to be consulted in that
/// order for every property separately. A paragraph whose style sets an indent and whose own
/// <c>w:pPr</c> sets a spacing needs both, so resolving "the paragraph properties" as one element and
/// falling back only when it is absent loses whichever half the direct formatting did not mention.
/// </para>
/// </remarks>
internal static class WordParagraphFormats
{
    /// <summary>The em size used when nothing in the chain states one.</summary>
    /// <remarks>
    /// Ten points, which is what Word's own <c>w:docDefaults</c> falls back to when a document omits
    /// them — not the eleven or twelve a template usually sets.
    /// </remarks>
    private static readonly Length DefaultSize = Length.FromPoints(10);

    /// <summary>The <c>auto</c> line rule's unit: a line is two hundred and forty of them.</summary>
    private const double LineUnitsPerLine = 240.0;

    /// <summary>The twips <c>w:beforeAutospacing</c>/<c>w:afterAutospacing</c> stand for.</summary>
    /// <remarks>
    /// Fourteen points — the HTML browser margin Word inherited, and the figure LibreOffice's importer
    /// substitutes in <c>DomainMapper.cxx</c>'s <c>LN_CT_Spacing_beforeAutospacing</c>. The value is a
    /// constant rather than anything derived from the text: it is a browser's paragraph margin, not a
    /// typographic measure.
    /// </remarks>
    internal static readonly Length HtmlAutoSpacing = Length.FromTwips(280);

    /// <summary>The same, for a document that switched HTML auto spacing off.</summary>
    /// <remarks>
    /// <c>w:doNotUseHTMLParagraphAutoSpacing</c> does not mean "no spacing" — it means five points,
    /// which is the literal <c>w:before="100"</c> such files also carry.
    /// </remarks>
    internal static readonly Length WordAutoSpacing = Length.FromTwips(100);

    /// <summary>Resolves a paragraph's layout properties.</summary>
    /// <param name="styles">The document's styles.</param>
    /// <param name="paragraphProperties">The paragraph's own <c>w:pPr</c>, or null.</param>
    /// <param name="defaultTabInterval">The document's <c>w:defaultTabStop</c>.</param>
    /// <param name="autoSpacing">
    /// What <c>w:beforeAutospacing</c> and <c>w:afterAutospacing</c> resolve to, which the document's
    /// compatibility settings decide. Null takes <see cref="HtmlAutoSpacing"/>, the ordinary case.
    /// </param>
    /// <param name="tableStyle">
    /// The <c>w:pPr</c> chain of the table style the paragraph sits in, or null outside a table.
    /// </param>
    /// <param name="shrinksJustifiedBlanks">
    /// True when a justified line may squeeze its blanks below their natural width, which is what the
    /// document's <c>compatibilityMode</c> decides. See
    /// <see cref="ParagraphFormat.ShrinksJustifiedBlanks"/>.
    /// </param>
    internal static ParagraphFormat Resolve(
        WordStyles styles,
        XElement? paragraphProperties,
        Length defaultTabInterval,
        Length? autoSpacing = null,
        IReadOnlyList<XElement>? tableStyle = null,
        bool shrinksJustifiedBlanks = false)
    {
        ArgumentNullException.ThrowIfNull(styles);

        string? styleId = Word.Attribute(Word.Child(paragraphProperties, "pStyle"), "val")
                          ?? styles.DefaultStyleId(WordStyleType.Paragraph);

        // Attribute by attribute rather than element by element: see WordStyles.ParagraphPropertyLayers.
        List<XElement> indent =
            styles.ParagraphPropertyLayers("ind", paragraphProperties, styleId, tableStyle);
        List<XElement> spacings =
            styles.ParagraphPropertyLayers("spacing", paragraphProperties, styleId, tableStyle);

        Length auto = autoSpacing ?? HtmlAutoSpacing;

        return new ParagraphFormat
        {
            Alignment = Alignment(Word.Attribute(
                Layer(styles, paragraphProperties, styleId, "jc", tableStyle), "val")),

            // w:bidi, which OOXML states on the paragraph and not on its runs. w:rtl on a run is
            // deliberately not read: LibreOffice's own importer discards it —
            // `case NS_ooxml::LN_EG_RPrBase_rtl: break;`,
            // sw/source/writerfilter/dmapper/DomainMapper.cxx:2511 — and resolves direction from
            // the text against this instead, so honouring it would put runs where Writer does not.
            IsRightToLeft = IsOn(styles, paragraphProperties, styleId, "bidi", tableStyle),

            // w:start and w:left are the same attribute under two names: the first is the
            // reading-direction form ECMA-376 standardised on and the second is what Word 2007 wrote
            // and what most files in existence still carry.
            StartIndent = Across(indent, "start", "left") ?? Length.Zero,
            EndIndent = Across(indent, "end", "right") ?? Length.Zero,
            FirstLineIndent = FirstLine(indent),

            SpaceBefore = AutoOr(spacings, "beforeAutospacing", "before", auto),
            SpaceAfter = AutoOr(spacings, "afterAutospacing", "after", auto),
            HasContextualSpacing =
                IsOn(styles, paragraphProperties, styleId, "contextualSpacing", tableStyle),

            // Which style, for the "same style" half of contextual spacing. Already resolved to the
            // document's default when the paragraph names none, so two unstyled paragraphs match.
            StyleKey = styleId,

            // The one part of w:spacing that is *not* attribute-wise: line and lineRule are a pair, and
            // LibreOffice maps them onto one UNO struct that a layer either replaces whole or leaves
            // alone. So the innermost layer mentioning either decides both.
            LineSpacing = Spacing(spacings.Find(
                s => Word.Attribute(s, "line") is not null
                     || Word.Attribute(s, "lineRule") is not null)),

            KeepWithNext = IsOn(styles, paragraphProperties, styleId, "keepNext", tableStyle),
            KeepTogether = IsOn(styles, paragraphProperties, styleId, "keepLines", tableStyle),

            // Word states widow control as one flag rather than two counts, and it means two of each —
            // which is why a document with it on sometimes has a visibly short page.
            OrphanLines = IsOn(styles, paragraphProperties, styleId, "widowControl", tableStyle) ? 2 : 0,
            WidowLines = IsOn(styles, paragraphProperties, styleId, "widowControl", tableStyle) ? 2 : 0,

            StartsNewPage = StartsNewPage(styles, paragraphProperties, styleId, tableStyle),
            TabStops = Tabs(Layer(styles, paragraphProperties, styleId, "tabs", tableStyle)),
            DefaultTabInterval =
                defaultTabInterval > Length.Zero ? defaultTabInterval : Length.FromTwips(720),

            // Word measures its tab stops from the text area rather than from the paragraph's indent:
            // writerfilter's DomainMapper sets TABS_RELATIVE_TO_INDENT to false on every document it
            // maps, citing #i24363#.
            TabsRelativeToIndent = false,

            ShrinksJustifiedBlanks = shrinksJustifiedBlanks,
        };
    }

    /// <summary>
    /// Which of a list level's two indents a paragraph will actually take.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The port of Writer's <c>SwTextNode::AreListLevelIndentsApplicableImpl</c>
    /// (<c>sw/source/core/txtnode/ndtxt.cxx:4851</c>), asked separately for each of its two items —
    /// <c>RES_MARGIN_TEXTLEFT</c> and <c>RES_MARGIN_FIRSTLINE</c> — because OOXML writes both into one
    /// <c>w:ind</c> and a level that states only a hanging indent must not lose it to a style that
    /// states only a left one.
    /// </para>
    /// <para>
    /// The rule has three arms, and the middle one is the one that is easy to miss: an indent set
    /// <em>hard on the paragraph</em> beats the list, but a numbering rule applied <em>directly to the
    /// paragraph</em> beats the style chain's indents. Only when the numbering arrives through a style
    /// does the chain get a say, and then it is a race — whichever of the two the walk meets first, an
    /// indent or the style carrying the numbering, decides.
    /// </para>
    /// <para>
    /// Getting the middle arm wrong is what glues a list label to its item's first word: Word's own
    /// <c>ListParagraph</c> style states <c>w:ind w:left</c> and no hanging, so a paragraph with a direct
    /// <c>w:numPr</c> whose level asks for <c>w:hanging="360"</c> ends up with no hanging at all, and the
    /// label is drawn where the text starts.
    /// </para>
    /// </remarks>
    /// <param name="styles">The document's styles.</param>
    /// <param name="paragraphProperties">The paragraph's own <c>w:pPr</c>, or null.</param>
    internal static ListLevelIndents ListLevelIndentsApplicable(
        WordStyles styles, XElement? paragraphProperties)
    {
        ArgumentNullException.ThrowIfNull(styles);

        return Applicable(styles, paragraphProperties, LeftAttributes, ListLevelIndents.LeftMargin)
               | Applicable(
                   styles, paragraphProperties, FirstLineAttributes, ListLevelIndents.FirstLine);
    }

    /// <summary>The <c>w:ind</c> attributes that set Writer's <c>RES_MARGIN_TEXTLEFT</c>.</summary>
    private static readonly string[] LeftAttributes = ["start", "left"];

    /// <summary>The ones that set <c>RES_MARGIN_FIRSTLINE</c>.</summary>
    private static readonly string[] FirstLineAttributes = ["hanging", "firstLine"];

    /// <summary>One item's answer, which is the whole of the ported rule.</summary>
    private static ListLevelIndents Applicable(
        WordStyles styles, XElement? paragraphProperties, string[] attributes, ListLevelIndents item)
    {
        // A hard-set indent on the paragraph beats the list, whatever else is true.
        if (Sets(Word.Child(paragraphProperties, "ind"), attributes)) return ListLevelIndents.No;

        // A numbering rule applied directly to the paragraph beats the style chain's indents. A
        // w:numId of zero is "not numbered" rather than a reference, so it does not count as one.
        if (Word.Value(Word.Child(paragraphProperties, "numPr"), "numId") is { } numId
            && numId != "0")
        {
            return item;
        }

        // Otherwise the numbering came through a style, and the walk decides: an indent met before the
        // style that carries the numbering wins, and the numbering wins if it is met first.
        string? styleId = Word.Attribute(Word.Child(paragraphProperties, "pStyle"), "val")
                          ?? styles.DefaultStyleId(WordStyleType.Paragraph);

        for (int depth = 0; depth < WordStyles.MaxBasedOnDepth; depth++)
        {
            if (styles.Find(styleId, WordStyleType.Paragraph) is not { } style) break;

            if (Sets(Word.Child(style.ParagraphProperties, "ind"), attributes))
            {
                return ListLevelIndents.No;
            }

            if (Word.Child(style.ParagraphProperties, "numPr") is not null) return item;

            styleId = style.BasedOn;
        }

        return item;
    }

    /// <summary>True when a <c>w:ind</c> states any of the attributes one margin item is made of.</summary>
    private static bool Sets(XElement? indent, string[] attributes)
    {
        if (indent is null) return false;

        foreach (string attribute in attributes)
        {
            if (Word.Attribute(indent, attribute) is not null) return true;
        }

        return false;
    }

    /// <summary>
    /// The <c>w:shd</c> that decides a paragraph's background, from whichever layer states one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Through the same layering every other paragraph property takes, which is the whole point of
    /// exposing it: a heading is shaded by its <em>style</em> far more often than by its own
    /// <c>w:pPr</c>, so a reader that looked only at the direct formatting would find no background on
    /// the documents that most obviously have one.
    /// </para>
    /// <para>
    /// The element rather than a colour, because resolving the colour needs the theme and the four
    /// <c>w:themeFill*</c> attributes that go with a fill — which is the reader's business, not the
    /// layering's.
    /// </para>
    /// </remarks>
    /// <param name="styles">The document's styles.</param>
    /// <param name="paragraphProperties">The paragraph's own <c>w:pPr</c>, or null.</param>
    internal static XElement? ShadingOf(WordStyles styles, XElement? paragraphProperties)
    {
        ArgumentNullException.ThrowIfNull(styles);

        string? styleId = Word.Attribute(Word.Child(paragraphProperties, "pStyle"), "val")
                          ?? styles.DefaultStyleId(WordStyleType.Paragraph);

        return Layer(styles, paragraphProperties, styleId, "shd");
    }

    /// <summary>
    /// Resolves the character formatting a paragraph's text is set in.
    /// </summary>
    /// <remarks>
    /// Through <see cref="WordStyles.ResolveRunProperty"/>, so the toggle rule applies: bold set by both
    /// the paragraph style and a character style comes out <em>off</em>. Here only the paragraph's own
    /// run properties are in play, which is what an unstyled run inherits.
    /// </remarks>
    internal static WordTextStyle ResolveText(
        WordStyles styles,
        XElement? paragraphProperties,
        DrawingTheme? theme = null)
    {
        ArgumentNullException.ThrowIfNull(styles);

        // A paragraph's mark carries its own run properties, and they are what a run with no properties
        // of its own inherits.
        return ResolveRun(styles, paragraphProperties, Word.Child(paragraphProperties, "rPr"), theme);
    }

    /// <summary>
    /// Resolves the character formatting of one run inside a paragraph.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The run's own <c>w:rPr</c> and the character style its <c>w:rStyle</c> names, layered over the
    /// paragraph style — which is what <see cref="WordStyles.ResolveRunProperty"/> takes, toggle rule
    /// included. That rule is the reason this cannot be done property-set by property-set: bold set by
    /// both the paragraph style and the character style comes out <em>off</em>, so the two layers have to
    /// be visible to the resolver at the same time.
    /// </para>
    /// <para>
    /// Note what is <em>not</em> here: a run in OOXML does not nest, so there is no cascade to walk. A
    /// hyperlink wraps runs rather than formatting them, and the blue underline comes from the
    /// <c>Hyperlink</c> character style that each of those runs names itself.
    /// </para>
    /// </remarks>
    /// <param name="styles">The document's styles.</param>
    /// <param name="paragraphProperties">The paragraph's <c>w:pPr</c>, for its <c>w:pStyle</c>.</param>
    /// <param name="runProperties">The run's own <c>w:rPr</c>, or null.</param>
    /// <param name="theme">The document's theme, for a <c>w:themeColor</c>, or null.</param>
    internal static WordTextStyle ResolveRun(
        WordStyles styles,
        XElement? paragraphProperties,
        XElement? runProperties,
        DrawingTheme? theme = null)
    {
        ArgumentNullException.ThrowIfNull(styles);

        string? styleId = Word.Attribute(Word.Child(paragraphProperties, "pStyle"), "val")
                          ?? styles.DefaultStyleId(WordStyleType.Paragraph);
        string? characterStyleId = Word.Attribute(Word.Child(runProperties, "rStyle"), "val");

        List<XElement> fonts =
            styles.RunPropertyLayers("rFonts", runProperties, styleId, characterStyleId);
        WordProperty size = styles.ResolveRunProperty("sz", runProperties, styleId, characterStyleId);
        WordProperty bold = styles.ResolveRunProperty("b", runProperties, styleId, characterStyleId);
        WordProperty italic = styles.ResolveRunProperty("i", runProperties, styleId, characterStyleId);
        WordProperty language =
            styles.ResolveRunProperty("lang", runProperties, styleId, characterStyleId);
        WordProperty colour = styles.ResolveRunProperty("color", runProperties, styleId, characterStyleId);
        WordProperty vertical =
            styles.ResolveRunProperty("vertAlign", runProperties, styleId, characterStyleId);
        WordProperty capitals =
            styles.ResolveRunProperty("caps", runProperties, styleId, characterStyleId);
        WordProperty smallCapitals =
            styles.ResolveRunProperty("smallCaps", runProperties, styleId, characterStyleId);
        WordProperty highlight =
            styles.ResolveRunProperty("highlight", runProperties, styleId, characterStyleId);
        WordProperty underline =
            styles.ResolveRunProperty("u", runProperties, styleId, characterStyleId);
        WordProperty strike =
            styles.ResolveRunProperty("strike", runProperties, styleId, characterStyleId);
        WordProperty doubleStrike =
            styles.ResolveRunProperty("dstrike", runProperties, styleId, characterStyleId);
        WordProperty kerning =
            styles.ResolveRunProperty("kern", runProperties, styleId, characterStyleId);

        // The character `w:spacing`, which shares its name with the paragraph one and nothing else. The
        // resolution only ever looks inside `w:rPr`, so the two cannot reach each other.
        WordProperty tracking =
            styles.ResolveRunProperty("spacing", runProperties, styleId, characterStyleId);

        Length resolvedSize = HalfPoints(size.Element) ?? DefaultSize;

        return new WordTextStyle(
            Family(fonts, theme?.Fonts),
            resolvedSize,
            bold.IsOn ? 700 : 400,
            italic.IsOn,
            Word.Attribute(language.Element, "val"),
            WordThemeColour.Read(colour.Element, theme),
            EscapementOf(vertical.Element),
            // Both are toggles and both are the same item, so the two cannot combine: Word writes at most
            // one of them per layer, and full capitals win where a file states both — which is what
            // `SvxCaseMapItem` holding a single value forces on any reader.
            capitals.IsOn ? PageCaseMap.Uppercase
                : smallCapitals.IsOn ? PageCaseMap.SmallCaps
                : PageCaseMap.None,
            HighlightOf(Word.Attribute(highlight.Element, "val")),
            // `w:u` is not a toggle and its `w:val` is a line style rather than a switch, so `IsOn` is
            // the wrong reading of it: it would take `w:u w:val="none"` — which is how a run turns off
            // an underline its style set — for an underline. The extraction side reads it the same way.
            underline.HasValue && underline.Value is not (null or "none"),
            // Folded onto one flag, as the extraction side folds them: `w:dstrike` is a second line
            // rather than a different decoration, and nothing below this models a doubled rule.
            strike.IsOn || doubleStrike.IsOn,
            AutoKerningOf(kerning),
            TrackingOf(tracking));
    }

    /// <summary>
    /// The distance a character <c>w:spacing</c> puts between letters, zero when it states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Twips, and signed: <c>DomainMapper</c> converts the value straight to <c>CharKerning</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:2468–2480), which is
    /// <c>SvxKerningItem</c> — a constant added between characters rather than the face's own pair
    /// kerning, which is <c>w:kern</c> and a different item entirely.
    /// </para>
    /// <para>
    /// Negative is the common case in real files and it is the one that matters: 58 of the words track's
    /// 134 DOCX state one, at values down to −28 twips, and a run of −16 over a fifty-character line is
    /// 40 pt of width the reference does not spend. A reader ignoring it breaks its lines late and
    /// paginates long.
    /// </para>
    /// </remarks>
    private static Length TrackingOf(WordProperty spacing)
        => spacing.IntegerValue is { } twips and not 0 ? Length.FromTwips(twips) : Length.Zero;

    /// <summary>
    /// Whether a <c>w:kern</c> switches pair kerning on.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>w:kern</c> is not a switch. Its <c>w:val</c> is a font size in half-points, and Word reads it
    /// as a <em>threshold</em>: kern text set at or above this size and leave smaller text alone. Writer
    /// has no such item — <c>RES_CHRATR_AUTOKERN</c> is a plain boolean — so LibreOffice drops the
    /// threshold and keeps only whether one was stated. Its importer says exactly that, in a comment
    /// beside the line that does it: <c>// auto kerning is bound to a minimum font size in Word - but
    /// not in Writer :-(</c>, then
    /// <c>rContext-&gt;Insert(PROP_CHAR_AUTO_KERNING, uno::Any(nIntValue != 0))</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx:2482</c>).
    /// </para>
    /// <para>
    /// So a document asking to kern from 16 pt upwards has every one of its runs kerned, at 8 pt as
    /// much as at 24. Reproducing Word's threshold instead would be more faithful to the format and
    /// less faithful to the reference, and the reference is what this is measured against.
    /// </para>
    /// <para>
    /// Absent, it is off — which is the whole point of reading it at all. A loaded Writer document
    /// starts from the pool default of <c>false</c> (<c>sw/source/core/bastyp/init.cxx:300</c>), and
    /// <c>SwDocShell::Load</c> resets the user default to it on every load through
    /// <c>RemoveAllFormatLanguageDependencies</c> (<c>sw/source/uibase/app/docsh.cxx:227</c>). The
    /// DOCX importer states it a second time for good measure, seeding its own default character
    /// properties with <c>CharAutoKerning=false</c> and citing that same function
    /// (<c>StyleSheetTable.cxx:354</c>).
    /// </para>
    /// </remarks>
    /// <param name="kerning">The resolved <c>w:kern</c>.</param>
    internal static bool AutoKerningOf(WordProperty kerning)
        => kerning.HasValue && kerning.IntegerValue is { } threshold && threshold != 0;

    /// <summary>
    /// The colour a <c>w:highlight</c> names, or null when it names none.
    /// </summary>
    /// <remarks>
    /// <c>ST_HighlightColor</c> is a closed list of sixteen names and <c>none</c>, with no way to state an
    /// arbitrary colour — which is what distinguishes Word's highlighter from character shading. The values
    /// are <c>DomainMapper::getColorFromId</c>'s (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:5198)
    /// and are worth taking from there rather than from the obvious guess: <c>green</c> is bright green and
    /// <c>darkGreen</c> is the one an eye would call green, exactly as in the DOC palette.
    /// </remarks>
    private static Colour? HighlightOf(string? name) => name switch
    {
        "black" => Colour.FromRgb(0x000000),
        "blue" => Colour.FromRgb(0x0000FF),
        "cyan" => Colour.FromRgb(0x00FFFF),
        "green" => Colour.FromRgb(0x00FF00),
        "magenta" => Colour.FromRgb(0xFF00FF),
        "red" => Colour.FromRgb(0xFF0000),
        "yellow" => Colour.FromRgb(0xFFFF00),
        "white" => Colour.FromRgb(0xFFFFFF),
        "darkBlue" => Colour.FromRgb(0x000080),
        "darkCyan" => Colour.FromRgb(0x008080),
        "darkGreen" => Colour.FromRgb(0x008000),
        "darkMagenta" => Colour.FromRgb(0x800080),
        "darkRed" => Colour.FromRgb(0x800000),
        "darkYellow" => Colour.FromRgb(0x808000),
        "darkGray" => Colour.FromRgb(0x808080),
        "lightGray" => Colour.FromRgb(0xC0C0C0),

        // "none" and anything the list does not hold, which is the automatic colour: no band.
        _ => null,
    };

    /// <summary>
    /// The superscript or subscript a <c>w:vertAlign</c> asks for.
    /// </summary>
    /// <remarks>
    /// Two values and no numbers: <c>superscript</c> and <c>subscript</c>, which carry both halves of the
    /// automatic pair with them — the shift <em>and</em> the smaller size — exactly as ODF's <c>super</c> and
    /// <c>sub</c> keywords do, and which LibreOffice imports as the same item. <c>baseline</c> is the third
    /// value and means neither, which is also what an absent element means.
    /// </remarks>
    private static Layout.Escapement EscapementOf(XElement? vertAlign) =>
        Word.Attribute(vertAlign, "val") switch
        {
            "superscript" => Layout.Escapement.Superscript,
            "subscript" => Layout.Escapement.Subscript,
            _ => Layout.Escapement.None,
        };

    /// <summary>
    /// A paragraph property from the direct formatting first, then the style chain, then the defaults.
    /// </summary>
    /// <remarks>
    /// Per property rather than per element, because a paragraph's own <c>w:pPr</c> is not a replacement
    /// for its style's — it is an overlay, and each child of it overrides only its counterpart.
    /// </remarks>
    private static XElement? Layer(
        WordStyles styles,
        XElement? paragraphProperties,
        string? styleId,
        string localName,
        IReadOnlyList<XElement>? tableStyle = null)
    {
        List<XElement> layers = styles.ParagraphPropertyLayers(
            localName, paragraphProperties, styleId, tableStyle);

        return layers.Count > 0 ? layers[0] : null;
    }

    /// <summary>
    /// The paragraph's tab stops, from a <c>w:tabs</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Taken whole from whichever layer declares the element, because <c>w:tabs</c> is a list and the
    /// direct formatting replaces the style's rather than adding to it.
    /// </para>
    /// <para>
    /// <c>w:val="clear"</c> is a stop that <em>removes</em> one the style set — Word's way of cancelling an
    /// inherited stop — so it contributes nothing here. Keeping it as a left stop would put a column
    /// boundary exactly where the document asked for none.
    /// </para>
    /// </remarks>
    private static List<TabStop> Tabs(XElement? tabs)
    {
        List<TabStop> stops = [];

        foreach (XElement tab in Word.Children(tabs, "tab"))
        {
            string? kind = Word.Attribute(tab, "val");
            if (kind == "clear") continue;

            if (Word.Attribute(tab, "pos") is not { } text
                || !long.TryParse(text, CultureInfo.InvariantCulture, out long twips))
            {
                continue;
            }

            string? leader = Word.Attribute(tab, "leader");

            stops.Add(new TabStop(
                Length.FromTwips(twips),
                kind switch
                {
                    "center" => TabAlignment.Centre,
                    "right" or "end" => TabAlignment.Right,
                    "decimal" => TabAlignment.DecimalSeparator,
                    _ => TabAlignment.Left,
                },
                leader switch
                {
                    "dot" => '.',
                    "hyphen" => '-',
                    "underscore" => '_',
                    "middleDot" => '\u00B7',
                    _ => '\0',
                }));
        }

        stops.Sort((left, right) => left.Position.Emu.CompareTo(right.Position.Emu));
        return stops;
    }

    private static bool IsOn(
        WordStyles styles,
        XElement? paragraphProperties,
        string? styleId,
        string localName,
        IReadOnlyList<XElement>? tableStyle = null)
        => Layer(styles, paragraphProperties, styleId, localName, tableStyle) is { } found
           && Word.IsOn(found);

    /// <summary>
    /// The alignment, from <c>w:jc</c>.
    /// </summary>
    /// <remarks>
    /// OOXML's four values are <em>direction-relative</em>, unlike ODF's and RTF's: <c>left</c> is
    /// the older spelling of <c>start</c> and means the right margin in a <c>w:bidi</c> paragraph.
    /// LibreOffice reaches the same answer by swapping them on import — "Paragraph justification
    /// reverses its meaning in an RTL context",
    /// <c>sw/source/writerfilter/dmapper/DomainMapper.cxx:2176</c> — and its own export of a
    /// right-aligned right-to-left ODF paragraph writes <c>w:jc w:val="start"</c>, which is the
    /// same statement read the other way round.
    /// </remarks>
    private static TextAlignment Alignment(string? value) => value switch
    {
        "end" or "right" => TextAlignment.End,
        "center" or "centre" => TextAlignment.Centre,
        "both" => TextAlignment.Justify,
        "distribute" => TextAlignment.Distribute,
        _ => TextAlignment.Start,
    };

    /// <summary>
    /// The first line's extra indent, which OOXML states as one of two mutually exclusive attributes.
    /// </summary>
    /// <remarks>
    /// <c>w:firstLine</c> is positive and <c>w:hanging</c> is its negation — a hanging indent is written
    /// as a positive number under a different name. Reading <c>w:hanging</c> without negating it indents
    /// a numbered list's first line instead of outdenting it, which puts every number in the wrong place.
    /// </remarks>
    /// <remarks>
    /// The two are one property, so the innermost layer naming either settles it — a paragraph whose
    /// own <c>w:ind</c> states <c>w:firstLine</c> must not have its style's <c>w:hanging</c> applied on
    /// top of it, and vice versa.
    /// </remarks>
    private static Length FirstLine(List<XElement> indent)
    {
        foreach (XElement layer in indent)
        {
            if (Twips(layer, "hanging") is { } hanging) return -hanging;
            if (Twips(layer, "firstLine") is { } first) return first;
        }

        return Length.Zero;
    }

    /// <summary>
    /// The first of several attribute spellings of one property, taken from the innermost layer naming
    /// any of them.
    /// </summary>
    /// <remarks>
    /// <c>w:start</c> and <c>w:left</c> are the same property under two names: the first is the
    /// reading-direction form ECMA-376 standardised on, the second is what Word 2007 wrote and what most
    /// files in existence still carry. A layer stating either has settled the property, so the search
    /// must not carry on outwards looking for the other spelling.
    /// </remarks>
    private static Length? Across(List<XElement> layers, params string[] names)
    {
        foreach (XElement layer in layers)
        {
            foreach (string name in names)
            {
                if (Twips(layer, name) is { } value) return value;
            }
        }

        return null;
    }

    /// <summary>
    /// A paragraph margin, with <c>w:beforeAutospacing</c>/<c>w:afterAutospacing</c> taking priority.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The auto flag is not a hint that some value elsewhere should be used — it <em>replaces</em> the
    /// margin the same <c>w:spacing</c> states, which is why files carry <c>w:before="100"
    /// w:beforeAutospacing="1"</c> and lay out with fourteen points rather than five. LibreOffice's
    /// importer inserts the literal value with overwrite off and the auto value over it.
    /// </para>
    /// <para>
    /// A flag explicitly switched off (<c>w:beforeAutospacing="0"</c>) states nothing at all: it leaves
    /// the layer's own <c>w:before</c>, and where there is none the search continues outwards.
    /// </para>
    /// </remarks>
    private static Length AutoOr(
        List<XElement> layers, string autoName, string name, Length autoSpacing)
        => IsAuto(layers, autoName, name) ? autoSpacing : Stated(layers, name);

    /// <summary>Whether the auto flag wins over any stated value, which is what decides the margin.</summary>
    private static bool IsAuto(List<XElement> layers, string autoName, string name)
    {
        foreach (XElement layer in layers)
        {
            if (Word.Attribute(layer, autoName) is { } flag && IsSwitchedOn(flag)) return true;
            if (Twips(layer, name) is not null) return false;
        }

        return false;
    }

    /// <summary>The innermost stated value, for the case the auto flag did not win.</summary>
    private static Length Stated(List<XElement> layers, string name)
    {
        foreach (XElement layer in layers)
        {
            if (Twips(layer, name) is { } value) return value;
        }

        return Length.Zero;
    }

    /// <summary>
    /// Whether a paragraph's space before or after is the HTML auto margin rather than a stated one.
    /// </summary>
    /// <remarks>
    /// Asked by the table reader, because the auto margin is suppressed at a cell's edges and a stated
    /// one is not — see <c>DocxLayoutSource.SuppressAutoSpacingInCell</c>. It re-derives the answer from
    /// the same layers <see cref="Resolve"/> used rather than recording it on the format, since a
    /// <see cref="ParagraphFormat"/> is shared with three other readers that have no such rule.
    /// </remarks>
    /// <param name="styles">The document's styles.</param>
    /// <param name="paragraphProperties">The paragraph's own <c>w:pPr</c>, or null.</param>
    /// <param name="tableStyle">The <c>w:pPr</c> chain of the table style it sits in.</param>
    /// <param name="before">True to ask about the space before, false for the space after.</param>
    internal static bool IsAutoSpaced(
        WordStyles styles,
        XElement? paragraphProperties,
        IReadOnlyList<XElement>? tableStyle,
        bool before)
    {
        ArgumentNullException.ThrowIfNull(styles);

        string? styleId = Word.Attribute(Word.Child(paragraphProperties, "pStyle"), "val")
                          ?? styles.DefaultStyleId(WordStyleType.Paragraph);

        return IsAuto(
            styles.ParagraphPropertyLayers("spacing", paragraphProperties, styleId, tableStyle),
            before ? "beforeAutospacing" : "afterAutospacing",
            before ? "before" : "after");
    }

    /// <summary>An OOXML on/off attribute, which real files spell three ways.</summary>
    private static bool IsSwitchedOn(string value)
        => value is not ("0" or "false" or "off");

    /// <summary>
    /// The line spacing, from <c>w:spacing</c>'s value and rule together.
    /// </summary>
    /// <remarks>
    /// The rule decides what the value <em>means</em>, and the units change with it: under
    /// <c>atLeast</c> and <c>exact</c> the value is twips, and under <c>auto</c> — which is the default
    /// when no rule is stated — it is two-hundred-and-fortieths of a line, so 240 is single and 360 is
    /// one and a half. Treating the <c>auto</c> value as twips gives a line height of eighteen points
    /// where a document asked for one and a half lines.
    /// </remarks>
    private static LineSpacingRule Spacing(XElement? spacing)
    {
        if (Word.Attribute(spacing, "line") is not { } text
            || !long.TryParse(text, CultureInfo.InvariantCulture, out long line)
            || line == 0)
        {
            return LineSpacingRule.SingleSpaced;
        }

        return Word.Attribute(spacing, "lineRule") switch
        {
            "atLeast" => LineSpacingRule.AtLeast(Length.FromTwips(Math.Abs(line))),
            "exact" => LineSpacingRule.Exactly(Length.FromTwips(Math.Abs(line))),
            _ => LineSpacingRule.Multiple(Math.Abs(line) / LineUnitsPerLine),
        };
    }

    /// <summary>
    /// Whether the paragraph starts a page.
    /// </summary>
    /// <remarks>
    /// <c>w:pageBreakBefore</c> says so directly. A <c>w:sectPr</c> in the paragraph's properties ends a
    /// section <em>at</em> this paragraph rather than before it, so it is not a break before this one —
    /// which is the opposite of what its position in the file suggests.
    /// </remarks>
    private static bool StartsNewPage(
        WordStyles styles,
        XElement? paragraphProperties,
        string? styleId,
        IReadOnlyList<XElement>? tableStyle)
        => IsOn(styles, paragraphProperties, styleId, "pageBreakBefore", tableStyle);

    /// <summary>
    /// A measurement in twips, signed, or null when the attribute is absent.
    /// </summary>
    /// <remarks>
    /// Signed because a negative indent is legal and used: a table caption often hangs into the margin.
    /// </remarks>
    private static Length? Twips(XElement? element, string attribute)
        => Word.Attribute(element, attribute) is { } text
           && long.TryParse(text, CultureInfo.InvariantCulture, out long twips)
            ? Length.FromTwips(twips)
            : null;

    /// <summary>
    /// A font size, which OOXML states in half-points.
    /// </summary>
    /// <remarks>
    /// <c>w:sz w:val="24"</c> is twelve points. Reading it as points sets every document at half size,
    /// which is the sort of error that is obvious on sight and invisible in a unit test that only checks
    /// the value round-trips.
    /// </remarks>
    private static Length? HalfPoints(XElement? element)
        => Word.Attribute(element, "val") is { } text
           && double.TryParse(text, CultureInfo.InvariantCulture, out double halves)
           && halves is > 0 and <= 4000
            ? Length.FromPoints(halves / 2)
            : null;

    /// <summary>
    /// The family from <c>w:rFonts</c>, preferring the one the text is actually in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>w:rFonts</c> names up to four families at once — ASCII, high-ANSI, complex-script and East
    /// Asian — because a run can contain all four kinds of character. Latin text is the ASCII one, and
    /// <c>w:cs</c> or <c>w:eastAsia</c> would be the wrong choice for it; picking whichever attribute
    /// comes first would depend on the producer's attribute order.
    /// </para>
    /// <para>
    /// The four are inherited independently, which is why this takes the layers rather than one
    /// element. A run stating only <c>w:cs</c> — Word's way of setting a complex-script face, written
    /// beside a <c>w:szCs</c> in three quarters of the documents in the corpus — still takes its Latin
    /// family from its style. Reading only the innermost element leaves that run with no ASCII family
    /// at all, so the search falls through to the complex-script one and sets ordinary Latin text in
    /// it. The fallback order below is therefore a last resort for a run that genuinely names nothing
    /// else, not the ordinary path.
    /// </para>
    /// <para>
    /// Each of the four can be named <em>indirectly</em> instead, by a companion attribute pointing at
    /// the theme's font scheme: <c>w:asciiTheme="minorHAnsi"</c> means "the theme's minor Latin face".
    /// Word writes that form for every run of an unmodified Office document, so a reader that ignores
    /// it falls all the way back to the <c>w:docDefaults</c> face — typically Times New Roman where the
    /// theme says Calibri. That is not merely the wrong shapes: the substitutes for those two have
    /// different vertical metrics (Liberation Serif's line box is 2355/2048 of the em, Carlito's is
    /// 2500/2048), so every line comes out six per cent short and enough of them eventually cost a
    /// page break. The theme attribute wins over its direct companion when both are present, which is
    /// what Word does and what <c>DomainMapper::lcl_attribute</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:453) says out loud.
    /// </para>
    /// </remarks>
    private static string? Family(IReadOnlyList<XElement> layers, DrawingFontScheme? scheme)
    {
        foreach ((string attribute, string themeAttribute) in Slots)
        {
            foreach (XElement fonts in layers)
            {
                if (SlotFamily(fonts, scheme, attribute, themeAttribute) is { } name) return name;
            }
        }

        return null;
    }

    /// <summary>
    /// The four scripts a <c>w:rFonts</c> can name, each with the attribute naming it indirectly.
    /// </summary>
    private static readonly (string Direct, string Theme)[] Slots =
    [
        ("ascii", "asciiTheme"),
        ("hAnsi", "hAnsiTheme"),
        ("cs", "cstheme"),
        ("eastAsia", "eastAsiaTheme"),
    ];

    /// <summary>
    /// One script's family from a <c>w:rFonts</c>, the indirect attribute beating the direct one.
    /// </summary>
    /// <param name="fonts">The <c>w:rFonts</c> element, or null.</param>
    /// <param name="scheme">The theme's font scheme, or null when the document has no theme.</param>
    /// <param name="direct">The attribute naming the family outright, such as <c>ascii</c>.</param>
    /// <param name="themed">Its companion, such as <c>asciiTheme</c>.</param>
    internal static string? SlotFamily(
        XElement? fonts, DrawingFontScheme? scheme, string direct, string themed)
    {
        if (ThemeFace(scheme, Word.Attribute(fonts, themed)) is { Length: > 0 } resolved)
        {
            return resolved;
        }

        return Word.Attribute(fonts, direct) is { Length: > 0 } name ? name : null;
    }

    /// <summary>
    /// The typeface one of <c>ST_Theme</c>'s eight names stands for, or null when there is no theme.
    /// </summary>
    /// <remarks>
    /// The scheme holds one Latin face rather than separate ASCII and high-ANSI ones, so
    /// <c>majorAscii</c> and <c>majorHAnsi</c> name the same thing — as they do in LibreOffice's
    /// <c>resolveMajorMinorTypeFace</c> (<c>sw/source/writerfilter/dmapper/ThemeHandler.cxx</c>:323).
    /// </remarks>
    internal static string? ThemeFace(DrawingFontScheme? scheme, string? name) => name switch
    {
        null => null,
        "majorAscii" or "majorHAnsi" => scheme?.ForReference("major", "latin"),
        "majorEastAsia" => scheme?.ForReference("major", "ea"),
        "majorBidi" => scheme?.ForReference("major", "cs"),
        "minorAscii" or "minorHAnsi" => scheme?.ForReference("minor", "latin"),
        "minorEastAsia" => scheme?.ForReference("minor", "ea"),
        "minorBidi" => scheme?.ForReference("minor", "cs"),
        _ => null,
    };
}

using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;

namespace XRay.Office.WordProcessing.Ooxml;

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
/// <param name="Underline">
/// How <c>w:u</c> underlines the run: <c>none</c> and an unstated property draw nothing,
/// <c>double</c> and <c>wavyDouble</c> draw two lines, and the other fifteen values of
/// <c>ST_Underline</c> draw one. See <see cref="TextUnderline"/>.
/// </param>
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
/// <see cref="XRay.Office.Text.Layout.FormattedRun.Tracking"/> for what a reader owes it.
/// </param>
/// <param name="WidthPerCent">
/// How wide <c>w:w</c> draws the run's glyphs against their own design, 100 for none. See
/// <see cref="XRay.Office.Text.Layout.TextWidthScale"/> for why the factor it produces is not the
/// percentage.
/// </param>
/// <param name="DeclaredClass">
/// The generic class the document has in force at this run — <see cref="FontFamilyClass.Unknown"/>
/// when nothing states one — for a family the font matcher cannot find.
/// <strong>It is inherited, and it is not a property of <see cref="FamilyName"/>.</strong>
/// See <see cref="WordParagraphFormats.StatedClass"/> for the rule and the probe behind it.
/// </param>
/// <param name="AsianLanguage">
/// <c>w:lang/@w:eastAsia</c>, the language of the East Asian font item, or null when nothing states
/// one.
/// </param>
/// <param name="ComplexLanguage">
/// <c>w:lang/@w:bidi</c>, the language of the complex-script font item, or null when nothing states
/// one.
/// </param>
/// <param name="Hint">
/// <c>w:rFonts/@w:hint</c>, which says which item an <em>ambiguous</em> character belongs to. See
/// <see cref="WriterScripts.ForRun"/>.
/// </param>
/// <param name="Script">
/// Which of the three character-font items this run's text is set from, western until a caller that
/// has the text says otherwise. See <see cref="OnScript"/>.
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
    TextUnderline Underline = TextUnderline.None,
    bool IsStruckThrough = false,
    bool AutoKerning = false,
    Length Tracking = default,
    int WidthPerCent = 100,
    FontFamilyClass DeclaredClass = FontFamilyClass.Unknown,
    string? AsianLanguage = null,
    string? ComplexLanguage = null,
    WriterScriptHint Hint = WriterScriptHint.Automatic,
    WriterScript Script = WriterScript.Western)
{
    /// <summary>The key a face cache is keyed on: what actually decides which font file is loaded.</summary>
    /// <remarks>
    /// <para>
    /// <see cref="DeclaredClass"/> is part of it, and leaving it out is a cache collision rather than an
    /// omission: one family named under a <c>swiss</c> ancestor and under a <c>roman</c> one resolves to
    /// two different faces, and whichever run reached the cache first would decide for both.
    /// </para>
    /// <para>
    /// So are <see cref="Script"/> and the language it selects, for the same reason and a sharper
    /// one: the item decides both the generic appended to the fontconfig pattern and its
    /// <c>FC_LANG</c>, so the same family named in a western run and in an East Asian one is two
    /// requests with two answers.
    /// </para>
    /// </remarks>
    public (string? Family, int Weight, bool Italic, FontFamilyClass Class, WriterScript Script,
        string? Language) FaceKey
        => (FamilyName, Weight, IsItalic, DeclaredClass, Script, ItemLanguage);

    /// <summary>
    /// The language of the font item this run's script selects, never null.
    /// </summary>
    /// <remarks>
    /// Writer's three default language items stand in where the document states none:
    /// <c>SwDoc::SwDoc</c> resolves them through
    /// <c>MsLangId::resolveSystemLanguageByScriptType</c>, which answers <c>LANGUAGE_ENGLISH_US</c>,
    /// <c>LANGUAGE_CHINESE_SIMPLIFIED</c> and <c>LANGUAGE_HINDI</c>
    /// (<c>sw/source/core/doc/docnew.cxx</c>:383-398,
    /// <c>i18nlangtag/source/isolang/mslangid.cxx</c>:135-165). It is not a detail: it is what sends
    /// a Hebrew character in a complex-script run to FreeSans rather than to DejaVu Sans.
    /// </remarks>
    public string ItemLanguage => Script switch
    {
        WriterScript.Asian => AsianLanguage ?? WriterScripts.DefaultLanguage(WriterScript.Asian),
        WriterScript.Complex => ComplexLanguage ?? WriterScripts.DefaultLanguage(WriterScript.Complex),
        _ => Language ?? WriterScripts.DefaultLanguage(WriterScript.Western),
    };

    /// <summary>The same style, told which font item the run's own text selects.</summary>
    /// <param name="text">The run's text.</param>
    public WordTextStyle OnScript(ReadOnlySpan<char> text)
        => this with { Script = WriterScripts.ForRun(text, Hint) };

    /// <summary>
    /// The item this run's glyph fallback is asked under: a family, a class and a language.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Built here rather than at the resolver because only the reader knows which of the three
    /// items the run's script selects, and the class is the item's rather than the document's — see
    /// <see cref="Layout.WordFallbackClass.ForScript"/>.
    /// </para>
    /// <para>
    /// <strong>A run naming no family at all states no item</strong>, and that is the same
    /// distinction <see cref="Layout.WordFallbackClass.ForDeclared"/> carries: "no font named" is
    /// answered by <c>DefaultFonts</c> rather than by a fallback shape, and a pattern built round an
    /// empty family would ask fontconfig about nothing. Such a run keeps the answer it has always
    /// had — the generic the face it resolved to is filed under.
    /// </para>
    /// </remarks>
    public FontItem FontItem
        => string.IsNullOrWhiteSpace(FamilyName)
            ? default
            : new FontItem(
                FamilyName,
                Layout.WordFallbackClass.ForScript(FamilyName, DeclaredClass, Script),
                ItemLanguage);
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
    /// Ten points, which is what <c>StyleSheetTable</c> puts in <c>m_pDefaultCharProps</c> before a
    /// document's own <c>w:docDefaults</c> are laid over it
    /// (<c>sw/source/writerfilter/dmapper/StyleSheetTable.cxx</c>:341-350, <em>"set font height
    /// default to 10pt"</em>) — not the eleven or twelve a template usually sets. See
    /// <see cref="NoDocDefaultsSize"/> for the one case where it is not this.
    /// </remarks>
    private static readonly Length DefaultSize = Length.FromPoints(10);

    /// <summary>The em size for a package that declares no <c>w:rPrDefault</c> at all.</summary>
    /// <remarks>
    /// <para>
    /// Eleven points in Calibri, which is a <em>different</em> default rather than a variant of
    /// <see cref="DefaultSize"/>: a DOCX import begins at Calibri 11 pt
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:182-193, tdf#108350, <em>"In Word
    /// since version 2007, the default document font is Calibri 11 pt. If a DOCX document doesn't
    /// contain font information, we should assume the intended font to provide best layout
    /// match."</em>) and is reset to Times New Roman 10 pt only when a <c>w:rPrDefault</c> element
    /// is actually seen (<c>StyleSheetTable.cxx</c>:2161-2167). Presence decides, not content.
    /// </para>
    /// <para>
    /// Measured in <c>dotnet/probes/words-empty-paragraph-height/</c> against both installed
    /// references, which agree on every row. An empty paragraph between two 12 pt lines, the gap
    /// between those lines: <b>27.25 pt</b> with no styles part, and 25.35, 26.00, 29.90 and 30.90
    /// with a <c>w:rPrDefault</c> that is respectively empty, names Carlito, states
    /// <c>w:sz w:val="28"</c>, and does both. Only the first row moved.
    /// </para>
    /// <para>
    /// <b>No document in the words corpus reaches it</b> — all 272 of its DOCX-family files declare a
    /// <c>w:rPrDefault</c>, so every one of them takes the Times New Roman 10 pt branch. Its witness
    /// is a hand-built package, which is what the probe fixtures and several test fixtures here are.
    /// </para>
    /// </remarks>
    private static readonly Length NoDocDefaultsSize = Length.FromPoints(11);

    /// <summary>The family for the same case. See <see cref="NoDocDefaultsSize"/>.</summary>
    private const string NoDocDefaultsFamily = "Calibri";

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

    /// <summary>The same again, for a document saved in web view.</summary>
    /// <remarks>
    /// <para>
    /// Under two and a half points rather than fourteen. LibreOffice's importer branches on the
    /// document's <c>w:view</c> in both <c>LN_CT_Spacing_beforeAutospacing</c> and
    /// <c>LN_CT_Spacing_afterAutospacing</c> (<c>DomainMapper.cxx</c>:927 and :948) —
    /// <c>if (GetView() == LN_Value_doc_ST_View_web) default_spacing = 49; else 280;</c> — under a
    /// comment conceding that 49 "is just the old value that should be removed, once the root cause in
    /// <c>SwTabFrm::MakeAll()</c> is fixed". It is nonetheless what the reference draws, and the
    /// difference is 11.55 pt at every auto-spaced paragraph boundary.
    /// </para>
    /// <para>
    /// Found on <c>May 25 bulletin focus on carers in the workplace.docx</c>, the only document in the
    /// words corpus that declares web view, where a blind reading of page 2 reported our bullet items
    /// "spaced apart" against the reference's "tightly stacked" before anything had been measured.
    /// </para>
    /// </remarks>
    internal static readonly Length WebAutoSpacing = Length.FromTwips(49);

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
        List<XElement> spacings = GroupedMargins(
            styles.ParagraphPropertyLayers("spacing", paragraphProperties, styleId, tableStyle),
            paragraphProperties);

        Length auto = autoSpacing ?? HtmlAutoSpacing;

        // Widow and orphan control, which is *on* for a paragraph that states nothing whenever the
        // package declared a `w:docDefaults/w:pPrDefault`. See
        // WordStyles.HasDefaultParagraphPropertiesElement for the mechanism, the citation and the
        // probes; the short form is that LibreOffice puts ParaWidows and ParaOrphans at 2 on the
        // built-in default style from that element's presence alone, and Word writes it empty.
        //
        // The layer walk still decides wherever anything states the flag, and it already includes the
        // docDefaults' own `w:pPr` as its outermost layer — which is what makes a
        // `<w:pPrDefault><w:pPr><w:widowControl w:val="0"/></w:pPr></w:pPrDefault>` turn it back off,
        // matching writerfilter's bOverwrite=false and the authored probe.
        bool widowControl =
            Layer(styles, paragraphProperties, styleId, "widowControl", tableStyle) is { } stated
                ? Word.IsOn(stated)
                : styles.HasDefaultParagraphPropertiesElement;

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
            OrphanLines = widowControl ? 2 : 0,
            WidowLines = widowControl ? 2 : 0,

            StartsNewPage = StartsNewPage(styles, paragraphProperties, styleId, tableStyle),
            TabStops = Tabs(
                styles.ParagraphPropertyLayers("tabs", paragraphProperties, styleId, tableStyle)),
            DefaultTabInterval =
                defaultTabInterval > Length.Zero ? defaultTabInterval : Length.FromTwips(720),

            // Word measures its tab stops from the text area rather than from the paragraph's indent:
            // writerfilter's DomainMapper sets TABS_RELATIVE_TO_INDENT to false on every document it
            // maps, citing #i24363#.
            TabsRelativeToIndent = false,
            ClampsTabsAtLineEdge = true,
            SpillsTrailingNoBreakSpace = true,

            // WriterFilter.cxx:325 sets TabOverSpacing on every document it reads, unconditionally.
            TabsOverSpacing = true,

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
        DrawingTheme? theme = null,
        IReadOnlyList<XElement>? tableStyleRunProperties = null,
        WordFontTable? fontTable = null)
    {
        ArgumentNullException.ThrowIfNull(styles);

        // A paragraph's mark carries its own run properties, and they are what a run with no properties
        // of its own inherits.
        return ResolveRun(
            styles, paragraphProperties, Word.Child(paragraphProperties, "rPr"), theme,
            tableStyleRunProperties, fontTable);
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
    /// <param name="tableStyleRunProperties">
    /// The enclosing table style's <c>w:rPr</c> layers for this cell, most specific first, or null
    /// outside a table. See <see cref="WordStyles.TableStyleRunProperties"/>.
    /// </param>
    /// <param name="fontTable">
    /// The document's <c>word/fontTable.xml</c>, or null when the class is not wanted. It is needed
    /// here rather than where the face is resolved because the class is decided by which
    /// <em>layer</em> named a classified font, and the layers are gone by then — see
    /// <see cref="StatedClass"/>.
    /// </param>
    /// <param name="ignoreCharacterStyle">
    /// True to drop the run's <c>w:rStyle</c> layer while keeping its own <c>w:rPr</c>, which is what a
    /// run inside a table of contents gets: <c>DomainMapper.cxx</c>:3037-3047 resolves the style name,
    /// finds it, and then declines to insert <c>PROP_CHAR_STYLE_NAME</c> when
    /// <c>DomainMapper_Impl::IsInTOC</c> — "do not add it elements in TOC: they will receive later
    /// another style references from TOC". Word writes every contents entry as a run naming the
    /// <c>Hyperlink</c> character style, so honouring it draws the whole list blue and underlined where
    /// the reference draws it in the <c>TOC N</c> paragraph style alone.
    /// </param>
    internal static WordTextStyle ResolveRun(
        WordStyles styles,
        XElement? paragraphProperties,
        XElement? runProperties,
        DrawingTheme? theme = null,
        IReadOnlyList<XElement>? tableStyleRunProperties = null,
        WordFontTable? fontTable = null,
        bool ignoreCharacterStyle = false)
    {
        ArgumentNullException.ThrowIfNull(styles);

        string? styleId = Word.Attribute(Word.Child(paragraphProperties, "pStyle"), "val")
                          ?? styles.DefaultStyleId(WordStyleType.Paragraph);
        string? characterStyleId = ignoreCharacterStyle
            ? null
            : Word.Attribute(Word.Child(runProperties, "rStyle"), "val");

        List<XElement> fonts =
            styles.RunPropertyLayers(
                "rFonts", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty size = styles.ResolveRunProperty("sz", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty bold = styles.ResolveRunProperty("b", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty italic = styles.ResolveRunProperty("i", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty language =
            styles.ResolveRunProperty("lang", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty colour = styles.ResolveRunProperty("color", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty vertical =
            styles.ResolveRunProperty("vertAlign", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty capitals =
            styles.ResolveRunProperty("caps", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty smallCapitals =
            styles.ResolveRunProperty("smallCaps", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty highlight =
            styles.ResolveRunProperty("highlight", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        // Not `ResolveRunProperty`: a `w:u` stating no `w:val` is transparent rather than an
        // underline, so the search has to see every layer. See `WordCharacterFormat.UnderlineOf`.
        List<XElement> underline = styles.RunPropertyLayers(
            "u", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty strike =
            styles.ResolveRunProperty("strike", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty doubleStrike =
            styles.ResolveRunProperty("dstrike", runProperties, styleId, characterStyleId, tableStyleRunProperties);
        WordProperty kerning =
            styles.ResolveRunProperty("kern", runProperties, styleId, characterStyleId, tableStyleRunProperties);

        // The character `w:spacing`, which shares its name with the paragraph one and nothing else. The
        // resolution only ever looks inside `w:rPr`, so the two cannot reach each other.
        WordProperty tracking =
            styles.ResolveRunProperty("spacing", runProperties, styleId, characterStyleId, tableStyleRunProperties);

        // `w:w`, whose one-letter name is why it is easy to miss and impossible to grep for: it shares
        // its spelling with the width attribute of half the format.
        WordProperty scale =
            styles.ResolveRunProperty("w", runProperties, styleId, characterStyleId, tableStyleRunProperties);

        // A package that declares no `w:rPrDefault` keeps the importer's own Calibri 11 pt instead
        // of the Times New Roman 10 pt every other document is reset to. See `NoDocDefaultsSize`.
        bool docDefaults = styles.HasDefaultRunPropertiesElement;
        Length resolvedSize =
            HalfPoints(size.Element) ?? (docDefaults ? DefaultSize : NoDocDefaultsSize);

        return new WordTextStyle(
            Family(fonts, theme?.Fonts) ?? (docDefaults ? null : NoDocDefaultsFamily),
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
            WordCharacterFormat.UnderlineOf(underline),
            // Folded onto one flag, as the extraction side folds them: `w:dstrike` is a second line
            // rather than a different decoration, and nothing below this models a doubled rule.
            strike.IsOn || doubleStrike.IsOn,
            AutoKerningOf(kerning),
            TrackingOf(tracking),
            WidthOf(scale),
            StatedClass(fonts, fontTable),
            Word.Attribute(language.Element, "eastAsia"),
            Word.Attribute(language.Element, "bidi"),
            HintOf(fonts));
    }

    /// <summary>
    /// What <c>w:rFonts/@w:hint</c> says an ambiguous character's script is, across the layers.
    /// </summary>
    /// <remarks>
    /// The innermost layer that states one wins, as for every other <c>w:rFonts</c> attribute.
    /// <c>DomainMapper::lcl_attribute</c> maps <c>eastAsia</c> onto
    /// <c>ScriptHintType::ASIAN</c> and <c>cs</c> onto <c>COMPLEX</c>, and anything else — including
    /// <c>default</c> — onto <c>AUTOMATIC</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:969-988).
    /// </remarks>
    internal static WriterScriptHint HintOf(IReadOnlyList<XElement> layers)
    {
        if (layers is null) return WriterScriptHint.Automatic;

        foreach (XElement fonts in layers)
        {
            switch (Word.Attribute(fonts, "hint"))
            {
                case "eastAsia": return WriterScriptHint.Asian;
                case "cs": return WriterScriptHint.Complex;
                case { Length: > 0 }: return WriterScriptHint.Automatic;
            }
        }

        return WriterScriptHint.Automatic;
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
    /// The percentage a <c>w:w</c> draws the run's glyphs at, 100 when it states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ST_TextScale</c>, whose range is 1 to 600, and <c>DomainMapper</c> hands it to
    /// <c>PROP_CHAR_SCALE_WIDTH</c> unchanged
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>, <c>NS_ooxml::LN_EG_RPrBase_w</c>) — which
    /// is <c>SvxCharScaleWidthItem</c>, and which VCL then applies as the font's width. It is <em>not</em>
    /// a font size: the line stays as tall as it was and only the glyphs narrow.
    /// </para>
    /// <para>
    /// It was read by nothing at all, and the corpus is full of it: <b>1677 <c>w:w</c> across 20 DOCX</b>,
    /// of which 237 say 100 and so mean nothing. Of the 1440 that do, <b>1226 say 99</b> — a Word habit,
    /// and one that costs a line's width one and a quarter per cent rather than one, for the reason in
    /// <see cref="XRay.Office.Text.Layout.TextWidthScale"/>. The documents that carry it are the ones the
    /// project's own notes reach for when they discuss reflow: <c>AWR OPS-AOC 044</c> states 25,
    /// <c>ESPN-R - MCF - RA - Ed1</c> 112, and <c>Annex-10-to-the-Aircraft-Maintenance-Specialist</c>
    /// 1100.
    /// </para>
    /// <para>
    /// Out-of-range and unparseable values fall back to 100 rather than to the stated number, which is
    /// what an unread property already did: a scale of zero would collapse the run to nothing.
    /// </para>
    /// </remarks>
    private static int WidthOf(WordProperty scale)
        => scale.IntegerValue is { } percent and >= 1 and <= 600
            ? percent
            : TextWidthScale.Natural;

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
    /// The paragraph's tab stops, merged down its style chain.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>w:tabs</c> <em>adds to</em> the set its style already has rather than replacing it, one stop at
    /// a time and keyed on position. That is not a guess: <c>DomainMapper</c> seeds the set from the
    /// paragraph style's own stops before it reads a single <c>w:tab</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:2604, <c>InitTabStopFromStyle</c>) and then
    /// folds each one in with <c>IncorporateTabStop</c> (<c>DomainMapper_Impl.cxx</c>:1485), which replaces
    /// a stop standing at the same position, deletes it when the incoming stop is <c>w:val="clear"</c>, and
    /// appends otherwise.
    /// </para>
    /// <para>
    /// <c>w:val="clear"</c> is the proof that merging is the rule: a set that replaced its style's outright
    /// would leave a clear entry nothing to cancel. Taking the innermost <c>w:tabs</c> whole loses every
    /// stop the style set and the paragraph did not restate — the ordinary shape of a running head, where
    /// the style holds the centre and right stops and the paragraph moves one of them. Measured on
    /// <c>johnson_hall_service_log.pdf.docx</c>, whose footer clears the style's right stop at 9360 twips
    /// and adds its own at 10710: with only the paragraph's own stops the third of its three tabs has none
    /// left to reach, so the trailing text starts past the margin and the footer wraps to two lines, taking
    /// a line off the body and a table row off the page.
    /// </para>
    /// </remarks>
    /// <param name="layers">Every layer stating a <c>w:tabs</c>, innermost first.</param>
    private static List<TabStop> Tabs(List<XElement> layers)
    {
        List<TabStop> stops = [];

        // Outermost first — the document defaults, then the style chain from its root down, then the
        // paragraph's own — because each layer edits what the one outside it left.
        for (int i = layers.Count - 1; i >= 0; i--) Incorporate(stops, layers[i]);

        stops.Sort((left, right) => left.Position.Emu.CompareTo(right.Position.Emu));
        return stops;
    }

    /// <summary>Folds one layer's <c>w:tab</c> children into a set of stops, keyed on position.</summary>
    private static void Incorporate(List<TabStop> stops, XElement? tabs)
    {
        foreach (XElement tab in Word.Children(tabs, "tab"))
        {
            if (Word.Attribute(tab, "pos") is not { } text
                || !Word.Long(text, out long twips))
            {
                continue;
            }

            Length position = Length.FromTwips(twips);
            int existing = stops.FindIndex(stop => stop.Position == position);

            string? kind = Word.Attribute(tab, "val");

            // A clear removes the stop standing at that position, and finding none does nothing at all —
            // Writer keeps it in the list marked deleted and filters it out at the end, which is the same
            // answer by a longer route.
            if (kind == "clear")
            {
                if (existing >= 0) stops.RemoveAt(existing);
                continue;
            }

            string? leader = Word.Attribute(tab, "leader");

            TabStop stop = new(
                position,
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
                });

            if (existing >= 0) stops[existing] = stop; else stops.Add(stop);
        }
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
            GroupedMargins(
                styles.ParagraphPropertyLayers("spacing", paragraphProperties, styleId, tableStyle),
                paragraphProperties),
            before ? "beforeAutospacing" : "afterAutospacing",
            before ? "before" : "after");
    }

    /// <summary>
    /// The <c>w:spacing</c> layers as a paragraph that sets one of its two margins directly sees
    /// them — which is without the pool completion its style may carry.
    /// </summary>
    /// <remarks>
    /// <para>
    /// writerfilter's <c>tdf#118521</c> block, <c>DomainMapper_Impl.cxx</c>:3110-3138: <em>"set
    /// paragraph top or bottom margin based on the paragraph style if we already set the other
    /// margin with direct formatting"</em>. The two margins are one <c>SvxULSpaceItem</c>, so
    /// setting either directly forces both to be written directly, and the one the file did not
    /// state is fetched from <c>GetPropertyFromParaStyleSheet</c> — the DOCX <c>w:basedOn</c> chain
    /// and then <c>w:docDefaults</c>, never Writer's pool.
    /// </para>
    /// <para>
    /// So the completion <see cref="WordStyles.PoolCompletedSide"/> marks is invisible on exactly
    /// these paragraphs, and dropping the marked attribute from the layer list is the whole of the
    /// difference: everything below it in the chain is already the DOCX inheritance the C++ walks.
    /// </para>
    /// <para>
    /// The condition is three-way and not two — <c>bTopSet != bBottomSet || bBottomSet !=
    /// bContextSet</c> — so a paragraph stating <em>only</em> <c>w:contextualSpacing</c> and no
    /// <c>w:spacing</c> at all triggers it and loses the completion on both margins. That arm is
    /// measured rather than assumed: `direct-one-sided-spacing.py`'s <c>ctx-only</c> row reads 60
    /// below where <c>style-only</c> reads 120.
    /// </para>
    /// <para>
    /// What counts as "set" is the attribute being present, not its value and not the element's:
    /// <c>w:before="0"</c> triggers it and a <c>w:spacing</c> carrying only <c>w:line</c> does not.
    /// Both are rows in that probe.
    /// </para>
    /// </remarks>
    /// <param name="layers">The <c>w:spacing</c> layers, innermost first.</param>
    /// <param name="paragraphProperties">The paragraph's own <c>w:pPr</c>, or null.</param>
    private static List<XElement> GroupedMargins(
        List<XElement> layers, XElement? paragraphProperties)
    {
        XElement? direct = Word.Child(paragraphProperties, "spacing");
        bool top = Word.Attribute(direct, "before") is not null
                   || Word.Attribute(direct, "beforeAutospacing") is not null;
        bool bottom = Word.Attribute(direct, "after") is not null
                      || Word.Attribute(direct, "afterAutospacing") is not null;
        bool context = Word.Child(paragraphProperties, "contextualSpacing") is not null;

        if (top == bottom && bottom == context) return layers;

        List<XElement>? rewritten = null;
        for (int i = 0; i < layers.Count; i++)
        {
            string? completed = WordStyles.PoolCompletedSide(layers[i]);
            if (completed is null) continue;
            if (completed == "before" ? top : bottom) continue;

            rewritten ??= [.. layers];
            XElement without = new(layers[i]);
            without.Attribute(Word.Name(completed))?.Remove();
            rewritten[i] = without;
        }

        return rewritten ?? layers;
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
            || !Word.Long(text, out long line)
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
           && Word.Long(text, out long twips)
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
    /// The generic class the document has in force across these layers, or
    /// <see cref="FontFamilyClass.Unknown"/> when no layer states one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The class is inherited and the family name is not a property of it.</strong> It is
    /// set only where <c>w:rFonts/@w:ascii</c> names a font <c>word/fontTable.xml</c> files under
    /// <c>roman</c> or <c>swiss</c>; every other statement of a family leaves whatever an outer
    /// layer put there. So the innermost layer that files its <c>w:ascii</c> name wins, and a layer
    /// that names an unfiled font — or names it through the theme, or does not name one at all —
    /// contributes nothing.
    /// </para>
    /// <para>
    /// That is <c>DomainMapper::lcl_attribute</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:436): <c>LN_CT_Fonts_ascii</c>
    /// inserts <c>PROP_CHAR_FONT_NAME</c> unconditionally and <c>PROP_CHAR_FONT_FAMILY</c> only when
    /// <c>FontTable::getFontEntryByName</c> answered something other than <c>DONTKNOW</c> — and
    /// <c>FontTable::lcl_sprm</c> maps <em>only</em> <c>roman</c> and <c>swiss</c>, dropping
    /// <c>auto</c>, <c>modern</c>, <c>script</c>, <c>decorative</c> and <c>w:pitch</c> on the floor.
    /// <c>LN_CT_Fonts_asciiTheme</c> inserts the name and never the family, and
    /// <c>LN_CT_Fonts_hAnsi</c> is <c>break; //unsupported</c>.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 with 28 authored packages of one paragraph and one run, so the PDF's
    /// font list has exactly one entry that can move
    /// (<c>probes/words-r55/family-inheritance.py</c>): a family the table files <c>auto</c> answers
    /// DejaVu <b>Sans</b> under a <c>swiss</c> <c>docDefaults</c>, under a <c>swiss</c> <c>Normal</c>
    /// consumed by a style <c>basedOn</c> it, through two style levels, and when the consumer is
    /// direct run formatting; DejaVu Serif under a <c>roman</c> ancestor or with its own
    /// <c>roman</c> entry; and DejaVu Serif when nothing anywhere states a class, which is the roman
    /// default round 54 measured. A font named through <c>w:asciiTheme</c> takes the ancestor's class
    /// even when the table files that same name <c>swiss</c>.
    /// </para>
    /// <para>
    /// <strong>Round 54 recorded this refuted and it is not.</strong> Its counter-example was read
    /// off the whole of <c>24-25_FAA_Holdover_Tables.docx</c>'s embedded font list, and that document
    /// draws DejaVu Sans for four other reasons — <c>Century Gothic</c>, <c>Tahoma</c>,
    /// <c>Charlotte Sans Book</c> and <c>CWFZGM+Myriad-BoldItalic</c> are all declared <c>swiss</c>
    /// in the same table — so the observable could not move whatever the edit did. Its own shape is
    /// the mechanism exactly: <c>Normal</c> names <c>Arial</c>, filed <c>swiss</c>, and
    /// <c>Heading2</c>, <c>Heading3</c> and <c>Caption</c> are <c>basedOn Normal</c> and name
    /// <c>Arial Bold</c>, filed <c>auto</c>.
    /// </para>
    /// </remarks>
    /// <param name="layers">The <c>w:rFonts</c> layers, innermost first.</param>
    /// <param name="fontTable">The document's font table, or null when there is nothing to ask.</param>
    internal static FontFamilyClass StatedClass(
        IReadOnlyList<XElement> layers, WordFontTable? fontTable)
    {
        if (fontTable is null || layers is null) return FontFamilyClass.Unknown;

        foreach (XElement fonts in layers)
        {
            // The direct attribute only. A layer naming its font through the theme sets the name and
            // leaves the class where it was, which is measured rather than inferred: a theme font the
            // table files `swiss` still comes out DejaVu Serif under a roman ancestor.
            if (Word.Attribute(fonts, "ascii") is not { Length: > 0 } name) continue;
            if (fontTable.ShapeOf(name).Class is var stated and not FontFamilyClass.Unknown)
            {
                return stated;
            }
        }

        return FontFamilyClass.Unknown;
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

using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Paperless.Text.Layout;

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
public readonly record struct WordTextStyle(
    string? FamilyName,
    Length Size,
    int Weight,
    bool IsItalic,
    string? Language,
    Colour? Colour = null,
    Layout.Escapement Escapement = default)
{
    /// <summary>The key a face cache is keyed on: what actually decides which font file is loaded.</summary>
    public (string? Family, int Weight, bool Italic) FaceKey => (FamilyName, Weight, IsItalic);
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
    internal static ParagraphFormat Resolve(
        WordStyles styles,
        XElement? paragraphProperties,
        Length defaultTabInterval,
        Length? autoSpacing = null,
        IReadOnlyList<XElement>? tableStyle = null)
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
        };
    }

    /// <summary>
    /// Whether a paragraph or its style chain states an indent of its own.
    /// </summary>
    /// <remarks>
    /// The question a list level has to ask before applying its own indents, because a hard-set indent
    /// beats the list — Writer's <c>SwTextNode::AreListLevelIndentsApplicable</c>, which walks the same
    /// two layers this does and returns false as soon as either sets the item.
    /// </remarks>
    /// <param name="styles">The document's styles.</param>
    /// <param name="paragraphProperties">The paragraph's own <c>w:pPr</c>, or null.</param>
    internal static bool DeclaresIndent(WordStyles styles, XElement? paragraphProperties)
    {
        ArgumentNullException.ThrowIfNull(styles);

        string? styleId = Word.Attribute(Word.Child(paragraphProperties, "pStyle"), "val")
                          ?? styles.DefaultStyleId(WordStyleType.Paragraph);

        return Layer(styles, paragraphProperties, styleId, "ind") is not null;
    }

    /// <summary>
    /// Resolves the character formatting a paragraph's text is set in.
    /// </summary>
    /// <remarks>
    /// Through <see cref="WordStyles.ResolveRunProperty"/>, so the toggle rule applies: bold set by both
    /// the paragraph style and a character style comes out <em>off</em>. Here only the paragraph's own
    /// run properties are in play, which is what an unstyled run inherits.
    /// </remarks>
    internal static WordTextStyle ResolveText(WordStyles styles, XElement? paragraphProperties)
    {
        ArgumentNullException.ThrowIfNull(styles);

        // A paragraph's mark carries its own run properties, and they are what a run with no properties
        // of its own inherits.
        return ResolveRun(styles, paragraphProperties, Word.Child(paragraphProperties, "rPr"));
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

        WordProperty fonts = styles.ResolveRunProperty("rFonts", runProperties, styleId, characterStyleId);
        WordProperty size = styles.ResolveRunProperty("sz", runProperties, styleId, characterStyleId);
        WordProperty bold = styles.ResolveRunProperty("b", runProperties, styleId, characterStyleId);
        WordProperty italic = styles.ResolveRunProperty("i", runProperties, styleId, characterStyleId);
        WordProperty language =
            styles.ResolveRunProperty("lang", runProperties, styleId, characterStyleId);
        WordProperty colour = styles.ResolveRunProperty("color", runProperties, styleId, characterStyleId);
        WordProperty vertical =
            styles.ResolveRunProperty("vertAlign", runProperties, styleId, characterStyleId);

        Length resolvedSize = HalfPoints(size.Element) ?? DefaultSize;

        return new WordTextStyle(
            Family(fonts.Element),
            resolvedSize,
            bold.IsOn ? 700 : 400,
            italic.IsOn,
            Word.Attribute(language.Element, "val"),
            WordThemeColour.Read(colour.Element, theme),
            EscapementOf(vertical.Element));
    }

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
    {
        foreach (XElement layer in layers)
        {
            if (Word.Attribute(layer, autoName) is { } flag && IsSwitchedOn(flag)) return autoSpacing;
            if (Twips(layer, name) is { } value) return value;
        }

        return Length.Zero;
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
    /// <c>w:rFonts</c> names up to four families at once — ASCII, high-ANSI, complex-script and East
    /// Asian — because a run can contain all four kinds of character. Latin text is the ASCII one, and
    /// <c>w:cs</c> or <c>w:eastAsia</c> would be the wrong choice for it; picking whichever attribute
    /// comes first would depend on the producer's attribute order.
    /// </remarks>
    private static string? Family(XElement? fonts)
    {
        foreach (string attribute in (string[])["ascii", "hAnsi", "cs", "eastAsia"])
        {
            if (Word.Attribute(fonts, attribute) is { Length: > 0 } name) return name;
        }
        return null;
    }
}

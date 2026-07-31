using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// The character formatting a stretch of a paragraph's text is set in.
/// </summary>
/// <remarks>
/// What layout needs plus the colour drawing needs. The colour rides along rather than being resolved
/// separately because both come from one walk of the same style cascade, and matching two sets of ranges
/// back up afterwards would cost more than carrying the field.
/// </remarks>
/// <param name="FamilyName">The family the document asks for, before substitution.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">True when the text is italic or oblique.</param>
/// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
/// <param name="Colour">The colour the text is drawn in, or null when nothing set one.</param>
/// <param name="Rise">
/// How far the text is raised above the baseline; negative lowers it. From
/// <c>style:text-position</c>, which states it as a percentage of the font size — so a superscript is a
/// rise <em>and</em> a smaller size, and the two are stated in the same attribute but are independent.
/// </param>
public readonly record struct OdfTextStyle(
    string? FamilyName,
    Length Size,
    int Weight,
    bool IsItalic,
    string? Language,
    Colour? Colour = null,
    Length Rise = default)
{
    /// <summary>The key a face cache is keyed on: what actually decides which font file is loaded.</summary>
    public (string? Family, int Weight, bool Italic) FaceKey => (FamilyName, Weight, IsItalic);
}

/// <summary>
/// Resolves an ODF paragraph style into the layout properties the engine takes.
/// </summary>
/// <remarks>
/// <para>
/// The style chain is already walked by <see cref="OdfStyles.ResolveProperty(string?, OdfStyleFamily,
/// OdfPropertyKind, string, string)"/>, which is the point of having built it first: a paragraph's
/// automatic style, its parent, the named style behind that and the family default are one lookup, and
/// "set here" stays distinguishable from "defaulted" for the properties where that matters.
/// </para>
/// <para>
/// What is left is the translation, and ODF spells several of these in ways that need care. Line
/// spacing is spread across three attributes and told apart partly by units — <c>fo:line-height</c>
/// carries a percentage <em>or</em> a length, and the length means exact. Indents are
/// <c>fo:margin-left</c> and <c>fo:margin-right</c> rather than anything called an indent, so the same
/// attribute names mean a paragraph's indents here and a page's margins on a page layout.
/// </para>
/// </remarks>
internal static class OdfParagraphFormats
{
    /// <summary>
    /// The em size used when the document states none anywhere in the chain, and the base a percentage is
    /// taken of.
    /// </summary>
    /// <remarks>
    /// Twelve points, which is the value in Writer's item pool and therefore what a paragraph style with
    /// no <c>fo:font-size</c> renders at — measured, by laying out a document whose only text properties
    /// name a family. A document relying on the default and laid out at anything else breaks its lines in
    /// different places from its first paragraph onwards.
    /// </remarks>
    private static readonly Length DefaultSize = Length.FromPoints(12);

    /// <summary>Resolves a paragraph style's layout properties.</summary>
    internal static ParagraphFormat Resolve(OdfStyles styles, string? styleName)
    {
        ArgumentNullException.ThrowIfNull(styles);

        return new ParagraphFormat
        {
            Alignment = Alignment(styles, styleName),
            StartIndent = Measure(styles, styleName, "margin-left"),
            EndIndent = Measure(styles, styleName, "margin-right"),
            FirstLineIndent = Measure(styles, styleName, "text-indent"),
            SpaceBefore = Measure(styles, styleName, "margin-top"),
            SpaceAfter = Measure(styles, styleName, "margin-bottom"),
            HasContextualSpacing =
                Paragraph(styles, styleName, OdfNamespaces.Style, "contextual-spacing")
                    .AsBoolean() == true,
            LineSpacing = Spacing(styles, styleName),

            // "always" against "auto", not a boolean — ODF spells both out, and treating a missing
            // attribute as false is right while treating "auto" as true is not.
            KeepWithNext = Paragraph(styles, styleName, OdfNamespaces.FoCompatible, "keep-with-next")
                .Is("always"),
            KeepTogether = Paragraph(styles, styleName, OdfNamespaces.FoCompatible, "keep-together")
                .Is("always"),

            OrphanLines = Count(styles, styleName, "orphans"),
            WidowLines = Count(styles, styleName, "widows"),
            StartsNewPage = StartsNewPage(styles, styleName),
            TabStops = Tabs(styles, styleName),
            DefaultTabInterval = TabInterval(styles),
        };
    }

    /// <summary>
    /// Resolves the character formatting a paragraph's text is set in.
    /// </summary>
    /// <remarks>
    /// From the paragraph style's own text properties, which is what a run with no span style of its own
    /// is set in — so this is the paragraph's baseline, and <see cref="ResolveText(OdfStyles,
    /// IReadOnlyList{OdfStyleReference})"/> is what a span inside it resolves through.
    /// </remarks>
    internal static OdfTextStyle ResolveText(OdfStyles styles, string? styleName)
        => ResolveText(styles, [new OdfStyleReference(styleName, OdfStyleFamily.Paragraph)]);

    /// <summary>
    /// Resolves the character formatting a cascade of styles produces.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The cascade runs outermost first — the paragraph style, then each enclosing <c>text:span</c>'s
    /// style — because ODF has no inline formatting and a span's automatic style is how one bold word is
    /// written. The innermost style that sets a property wins, and only if none does at all do the family
    /// defaults apply, which is what <see cref="OdfStyles.ResolveProperty(IReadOnlyList{
    /// OdfStyleReference}, OdfPropertyKind, string, string)"/> implements.
    /// </para>
    /// <para>
    /// Two properties cannot go through that method. The family has two spellings that mean the same item
    /// to LibreOffice, so an outer <c>fo:font-family</c> must not beat an inner <c>style:font-name</c> —
    /// the level has to be decided before the spelling. And a percentage size is relative to the enclosing
    /// level's, so resolving it means walking outwards multiplying rather than taking one value.
    /// </para>
    /// </remarks>
    /// <param name="styles">The document's styles.</param>
    /// <param name="cascade">The style references, outermost first.</param>
    internal static OdfTextStyle ResolveText(
        OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade)
    {
        ArgumentNullException.ThrowIfNull(styles);
        ArgumentNullException.ThrowIfNull(cascade);

        return new OdfTextStyle(
            FamilyIn(styles, cascade),
            SizeIn(styles, cascade),
            Weight(Cascaded(styles, cascade, OdfNamespaces.FoCompatible, "font-weight").Value),
            Cascaded(styles, cascade, OdfNamespaces.FoCompatible, "font-style").Value
                is "italic" or "oblique",
            LanguageTag(
                Cascaded(styles, cascade, OdfNamespaces.FoCompatible, "language").Value,
                Cascaded(styles, cascade, OdfNamespaces.FoCompatible, "country").Value),
            Cascaded(styles, cascade, OdfNamespaces.FoCompatible, "color").AsColour(),
            RiseIn(styles, cascade));
    }

    /// <summary>
    /// The baseline shift the cascade asks for, as a length.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>style:text-position</c> carries one or two values: a vertical position and, optionally, a font
    /// size percentage. The position is a percentage of the font size, or one of the keywords <c>super</c>
    /// and <c>sub</c> — which LibreOffice takes as ±33%, the automatic values its dialogue offers. A
    /// percentage is positive upwards, so <c>sub</c> is the negative case rather than a separate one.
    /// </para>
    /// <para>
    /// The size half is deliberately not read here: it belongs to <c>SizeIn</c>, which already walks the
    /// cascade multiplying percentages, and reading it in two places would apply it twice to a span whose
    /// style states both.
    /// </para>
    /// </remarks>
    private static Length RiseIn(OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade)
    {
        string? stated = Cascaded(styles, cascade, OdfNamespaces.Style, "text-position").Value;
        if (string.IsNullOrWhiteSpace(stated)) return Length.Zero;

        string position = stated.Split(' ', StringSplitOptions.RemoveEmptyEntries) is [string first, ..]
            ? first
            : stated;

        Length size = SizeIn(styles, cascade);

        return position switch
        {
            "super" => size * AutomaticRise / 100.0,
            "sub" => size * -AutomaticRise / 100.0,
            _ => OdfValue.ParsePercentage(position) is { } percent
                ? OdfWriterUnits.ToCore(size * percent / 100.0)
                : Length.Zero,
        };
    }

    /// <summary>
    /// The rise <c>super</c> and <c>sub</c> mean, as a percentage of the font size.
    /// </summary>
    /// <remarks>
    /// A third, which is what LibreOffice's own character dialogue offers as its automatic value and what
    /// its ODF import uses for the two keywords. The keywords are the common case by a wide margin, because
    /// that dialogue is how a superscript is usually applied.
    /// </remarks>
    private const double AutomaticRise = 33;

    /// <summary>
    /// The family the cascade asks for, deciding the level before the spelling.
    /// </summary>
    /// <remarks>
    /// <c>style:font-name</c> names an <c>office:font-face-decls</c> entry rather than a family, and
    /// LibreOffice writes that form far more often than <c>fo:font-family</c> — so a reader that only
    /// looks at the latter finds no font at all in most real documents. A declaration that names no
    /// family at all is still a family name in practice, since producers use the family as the
    /// declaration's name.
    /// </remarks>
    private static string? FamilyIn(OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade)
    {
        for (int i = cascade.Count - 1; i >= 0; i--)
        {
            OdfStyleReference at = cascade[i];

            if (Unquote(Own(styles, at, OdfNamespaces.FoCompatible, "font-family").Value)
                is { } direct)
            {
                return direct;
            }

            if (Own(styles, at, OdfNamespaces.Style, "font-name").Value is { } declared)
            {
                return FamilyOfDeclaration(styles, declared);
            }
        }

        if (Unquote(Defaulted(styles, cascade, OdfNamespaces.FoCompatible, "font-family").Value)
            is { } fallback)
        {
            return fallback;
        }

        return Defaulted(styles, cascade, OdfNamespaces.Style, "font-name").Value is { } name
            ? FamilyOfDeclaration(styles, name)
            : null;
    }

    private static string? FamilyOfDeclaration(OdfStyles styles, string declared)
        => styles.FontFaces.TryGetValue(declared, out OdfFontFace? face)
            ? Unquote(face.FontFamily) ?? Unquote(declared)
            : Unquote(declared);

    /// <summary>
    /// The em size the cascade asks for.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The innermost level that states a size at all wins, which is ordinary containment inheritance. What
    /// is not ordinary is the percentage: <c>fo:font-size="150%"</c> is 150% of the <em>item pool's</em>
    /// twelve points, not of the enclosing text's size and not of the style's parent's either.
    /// </para>
    /// <para>
    /// That is measured, and it is measured because it is surprising. A 150% span inside an 11 pt paragraph
    /// renders at 18 pt rather than 16.5, and a 150% style whose <c>style:parent-style-name</c> declares
    /// 20 pt also renders at 18 rather than 30. The cause is where xmloff applies it: a percentage arrives
    /// as <c>CharPropHeight</c>, and <c>SvxFontHeightItem</c> resolves a proportion against the height the
    /// item set holds <em>at that moment</em> — which, for a style being built up from nothing, is the pool
    /// default. The parent chain is resolved later, so it never enters the arithmetic, and nested
    /// percentages do not compound for the same reason.
    /// </para>
    /// </remarks>
    private static Length SizeIn(OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade)
    {
        for (int i = cascade.Count - 1; i >= 0; i--)
        {
            OdfProperty stated = Own(styles, cascade[i], OdfNamespaces.FoCompatible, "font-size");
            if (!stated.HasValue) continue;

            if (stated.AsPercentage() is { } proportion and > 0 and < 100)
            {
                return OdfWriterUnits.ToCore(DefaultSize * proportion);
            }

            if (stated.AsLength() is { } absolute) return OdfWriterUnits.ToCore(absolute);
        }

        return OdfWriterUnits.ToCore(
            Defaulted(styles, cascade, OdfNamespaces.FoCompatible, "font-size").AsLength()
            ?? DefaultSize);
    }

    private static OdfProperty Paragraph(
        OdfStyles styles, string? styleName, string ns, string name)
        => styles.ResolveProperty(
            styleName, OdfStyleFamily.Paragraph, OdfPropertyKind.Paragraph, ns, name);

    private static OdfProperty Text(OdfStyles styles, string? styleName, string ns, string name)
        => styles.ResolveProperty(
            styleName, OdfStyleFamily.Paragraph, OdfPropertyKind.Text, ns, name);

    /// <summary>A text property resolved through the whole cascade, defaults included.</summary>
    private static OdfProperty Cascaded(
        OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade, string ns, string name)
        => styles.ResolveProperty(cascade, OdfPropertyKind.Text, ns, name);

    /// <summary>A text property from one level of the cascade and its parents, defaults excluded.</summary>
    private static OdfProperty Own(
        OdfStyles styles, OdfStyleReference at, string ns, string name)
        => styles.ResolveWithoutDefaults(at.Name, at.Family, OdfPropertyKind.Text, ns, name);

    /// <summary>A text property from the cascade's family defaults alone.</summary>
    private static OdfProperty Defaulted(
        OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade, string ns, string name)
    {
        for (int i = cascade.Count - 1; i >= 0; i--)
        {
            OdfProperty found = styles.ResolveFromDefaults(
                cascade[i].Family, OdfPropertyKind.Text, ns, name);
            if (found.HasValue) return found;
        }

        return OdfProperty.Unset;
    }

    /// <summary>An indent or spacing, on Writer's own whole-twip grid.</summary>
    private static Length Measure(OdfStyles styles, string? styleName, string name)
        => OdfWriterUnits.ToCore(
               Paragraph(styles, styleName, OdfNamespaces.FoCompatible, name).AsLength())
           ?? Length.Zero;

    private static int Count(OdfStyles styles, string? styleName, string name)
    {
        int? value = Paragraph(styles, styleName, OdfNamespaces.FoCompatible, name).AsInt();
        return value is > 0 and < 100 ? value.Value : 0;
    }

    /// <summary>
    /// The alignment, with ODF's own vocabulary rather than CSS's.
    /// </summary>
    /// <remarks>
    /// <c>start</c> and <c>end</c> rather than left and right, because ODF states the reading direction
    /// and not the page side — though it permits <c>left</c> and <c>right</c> too, and producers write
    /// them. Justified text is <c>justify</c>, and whether its last line is stretched as well is a
    /// separate attribute (<c>style:justify-single-word</c> is a different question again).
    /// </remarks>
    private static TextAlignment Alignment(OdfStyles styles, string? styleName)
    {
        OdfProperty align = Paragraph(styles, styleName, OdfNamespaces.FoCompatible, "text-align");

        return align.Value switch
        {
            "end" or "right" => TextAlignment.End,
            "center" or "centre" => TextAlignment.Centre,
            "justify" => Paragraph(styles, styleName, OdfNamespaces.FoCompatible, "text-align-last")
                    .Is("justify")
                ? TextAlignment.Distribute
                : TextAlignment.Justify,
            _ => TextAlignment.Start,
        };
    }

    /// <summary>
    /// The line spacing, from whichever of ODF's three attributes the chain resolves.
    /// </summary>
    /// <remarks>
    /// Checked in the order the specification gives them precedence: an explicit minimum beats a
    /// leading, and both beat <c>fo:line-height</c>. That last one carries either a percentage or a
    /// length, and which it is decides the <em>mode</em> and not just the value — <c>150%</c> is
    /// proportional and <c>0.5cm</c> is exact, so parsing it as a number would silently turn a fixed
    /// height into a multiple.
    /// </remarks>
    private static LineSpacingRule Spacing(OdfStyles styles, string? styleName)
    {
        if (Paragraph(styles, styleName, OdfNamespaces.Style, "line-height-at-least").AsLength()
            is { } atLeast)
        {
            return LineSpacingRule.AtLeast(OdfWriterUnits.ToCore(atLeast));
        }

        if (Paragraph(styles, styleName, OdfNamespaces.Style, "line-spacing").AsLength()
            is { } leading)
        {
            return LineSpacingRule.PlusLeading(OdfWriterUnits.ToCore(leading));
        }

        OdfProperty height = Paragraph(styles, styleName, OdfNamespaces.FoCompatible, "line-height");
        if (!height.HasValue || height.Is("normal")) return LineSpacingRule.SingleSpaced;

        if (height.AsPercentage() is { } proportion) return LineSpacingRule.Multiple(proportion);
        if (height.AsLength() is { } exact)
        {
            return LineSpacingRule.Exactly(OdfWriterUnits.ToCore(exact));
        }

        return LineSpacingRule.SingleSpaced;
    }

    /// <summary>
    /// The paragraph's own tab stops.
    /// </summary>
    /// <remarks>
    /// <para>
    /// From the innermost style in the chain that declares a <c>style:tab-stops</c>, taken whole rather
    /// than merged with its parent's: the element is a list, and ODF replaces the list rather than adding
    /// to it — a paragraph that sets one stop has one stop, not its style's four and one more.
    /// </para>
    /// <para>
    /// The types are ODF's own vocabulary, and <c>char</c> is the one that needs care: it means "align on a
    /// character", with <c>style:char</c> saying which, so it is only a decimal stop when that character is
    /// a separator. A <c>char</c> stop naming nothing behaves as a right stop, which is what LibreOffice
    /// renders — verified against its own output rather than assumed.
    /// </para>
    /// </remarks>
    private static List<TabStop> Tabs(OdfStyles styles, string? styleName)
    {
        OdfStyle? current = styles.Find(styleName, OdfStyleFamily.Paragraph);
        HashSet<string> visited = new(StringComparer.Ordinal);

        for (int depth = 0; current is not null && depth < OdfStyles.MaxParentChainDepth; depth++)
        {
            if (current.Properties(OdfPropertyKind.Paragraph)?.Child(OdfNamespaces.Style, "tab-stops")
                is { } declared)
            {
                return ReadStops(declared);
            }

            if (!visited.Add(current.Name)) break;
            current = styles.Find(current.ParentStyleName, OdfStyleFamily.Paragraph);
        }

        return styles.GetDefault(OdfStyleFamily.Paragraph)
                   ?.Properties(OdfPropertyKind.Paragraph)
                   ?.Child(OdfNamespaces.Style, "tab-stops") is { } fromDefaults
            ? ReadStops(fromDefaults)
            : [];

    }

    private static List<TabStop> ReadStops(XElement element)
    {
        List<TabStop> stops = [];

        foreach (XElement stop in element.Elements(XName.Get("tab-stop", OdfNamespaces.Style)))
        {
            if (OdfValue.ParseLength(stop.Attribute(XName.Get("position", OdfNamespaces.Style))?.Value)
                is not { } position)
            {
                continue;
            }

            string? type = stop.Attribute(XName.Get("type", OdfNamespaces.Style))?.Value;
            string? character = stop.Attribute(XName.Get("char", OdfNamespaces.Style))?.Value;
            string? leader = stop.Attribute(XName.Get("leader-text", OdfNamespaces.Style))?.Value;

            stops.Add(new TabStop(
                OdfWriterUnits.ToCore(position),
                type switch
                {
                    "center" or "centre" => TabAlignment.Centre,
                    "right" => TabAlignment.Right,
                    "char" when character is "." or "," => TabAlignment.DecimalSeparator,
                    "char" => TabAlignment.Right,
                    _ => TabAlignment.Left,
                },
                leader is { Length: > 0 } ? leader[0] : '\0'));
        }

        stops.Sort((left, right) => left.Position.Emu.CompareTo(right.Position.Emu));
        return stops;
    }

    /// <summary>
    /// The document's default tab interval.
    /// </summary>
    /// <remarks>
    /// A quarter over a centimetre, which is LibreOffice's own default and not the half inch Word uses —
    /// measured: a tab in a paragraph declaring no stops lands 709 twips along, not 720. The value lives on
    /// the default paragraph style rather than in a document setting, which is why it is read from there.
    /// </remarks>
    private static Length TabInterval(OdfStyles styles)
    {
        OdfProperty declared = styles.ResolveFromDefaults(
            OdfStyleFamily.Paragraph, OdfPropertyKind.Paragraph,
            OdfNamespaces.Style, "tab-stop-distance");

        return OdfWriterUnits.ToCore(declared.AsLength()) is { } interval
               && interval > Length.Zero
            ? interval
            : Length.FromMillimetres(12.5);
    }

    /// <summary>
    /// Whether the paragraph starts a page.
    /// </summary>
    /// <remarks>
    /// Two spellings mean it, and a document uses whichever its producer preferred:
    /// <c>fo:break-before="page"</c>, and a <c>style:master-page-name</c> on the paragraph style, which
    /// changes the page description and therefore has to start a page to do so. Missing the second one
    /// loses the break at every section change in an ODF document, since that is how ODF has no section
    /// list.
    /// </remarks>
    private static bool StartsNewPage(OdfStyles styles, string? styleName)
    {
        if (Paragraph(styles, styleName, OdfNamespaces.FoCompatible, "break-before").Is("page"))
        {
            return true;
        }

        OdfStyle? style = styles.Find(styleName, OdfStyleFamily.Paragraph);
        return !string.IsNullOrEmpty(style?.MasterPageName);
    }

    /// <summary>
    /// A weight keyword or number on the OpenType scale.
    /// </summary>
    /// <remarks>
    /// ODF allows both the CSS keywords and the numeric values, and <c>bold</c> is by far the common
    /// case. An unrecognised value is regular rather than an error: a document naming a weight nobody
    /// has still has to render.
    /// </remarks>
    private static int Weight(string? value) => value switch
    {
        null or "" or "normal" => 400,
        "bold" => 700,
        _ => int.TryParse(value, out int numeric) && numeric is >= 1 and <= 1000 ? numeric : 400,
    };

    /// <summary>
    /// A BCP 47 tag from ODF's separate language and country attributes.
    /// </summary>
    /// <remarks>
    /// ODF keeps them apart where every other format writes one tag, and a country without a language is
    /// meaningless — so the language decides whether there is a tag at all.
    /// </remarks>
    private static string? LanguageTag(string? language, string? country)
    {
        if (string.IsNullOrWhiteSpace(language)) return null;
        return string.IsNullOrWhiteSpace(country) ? language : $"{language}-{country}";
    }

    /// <summary>
    /// Strips the quotes and the fallback list a CSS-style font family carries.
    /// </summary>
    /// <remarks>
    /// <c>fo:font-family</c> is CSS syntax, so a real document says
    /// <c>'Liberation Serif', 'Times New Roman', serif</c>. Passing that whole string to a font resolver
    /// matches nothing; taking the first name is what LibreOffice does with it.
    /// </remarks>
    private static string? Unquote(string? family)
    {
        if (string.IsNullOrWhiteSpace(family)) return null;

        string first = family.Split(',')[0].Trim();
        if (first.Length >= 2
            && ((first[0] == '\'' && first[^1] == '\'') || (first[0] == '"' && first[^1] == '"')))
        {
            first = first[1..^1];
        }

        return first.Length > 0 ? first : null;
    }
}

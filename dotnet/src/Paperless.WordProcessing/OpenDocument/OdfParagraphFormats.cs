using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// The character formatting a paragraph's text is set in.
/// </summary>
/// <remarks>
/// Only what layout needs, which is less than what drawing will: the face, the size, and the language
/// the break rules take. Colour and decoration do not change where a line breaks, so they are not
/// resolved here — and resolving them would mean walking the runs rather than the paragraph, which is
/// the next pass rather than this one.
/// </remarks>
/// <param name="FamilyName">The family the document asks for, before substitution.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">True when the text is italic or oblique.</param>
/// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
public readonly record struct OdfTextStyle(
    string? FamilyName,
    Length Size,
    int Weight,
    bool IsItalic,
    string? Language);

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
    /// <summary>The em size used when the document states none anywhere in the chain.</summary>
    /// <remarks>
    /// Ten points, which is Writer's own default — not twelve. A document relying on the default and
    /// laid out at twelve breaks its lines in different places from the first paragraph onwards.
    /// </remarks>
    private static readonly Length DefaultSize = Length.FromPoints(10);

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
        };
    }

    /// <summary>
    /// Resolves the character formatting a paragraph's text is set in.
    /// </summary>
    /// <remarks>
    /// From the paragraph style's own text properties, which is what an unstyled run inherits. A run
    /// with its own span style overrides this, and resolving that needs the runs — so this is the
    /// paragraph's baseline rather than the final answer for every character in it.
    /// </remarks>
    internal static OdfTextStyle ResolveText(OdfStyles styles, string? styleName)
    {
        ArgumentNullException.ThrowIfNull(styles);

        string? family = Text(styles, styleName, OdfNamespaces.FoCompatible, "font-family").Value;

        // style:font-name names a font face declaration rather than a family, and LibreOffice writes
        // that form far more often than fo:font-family — so a reader that only looks at the latter
        // finds no font at all in most real documents.
        if (string.IsNullOrWhiteSpace(family)
            && Text(styles, styleName, OdfNamespaces.Style, "font-name").Value is { } declared
            && styles.FontFaces.TryGetValue(declared, out OdfFontFace? face))
        {
            family = face.FontFamily ?? declared;
        }
        family ??= Text(styles, styleName, OdfNamespaces.Style, "font-name").Value;

        return new OdfTextStyle(
            Unquote(family),
            Text(styles, styleName, OdfNamespaces.FoCompatible, "font-size").AsLength() ?? DefaultSize,
            Weight(Text(styles, styleName, OdfNamespaces.FoCompatible, "font-weight").Value),
            Text(styles, styleName, OdfNamespaces.FoCompatible, "font-style").Value
                is "italic" or "oblique",
            LanguageTag(
                Text(styles, styleName, OdfNamespaces.FoCompatible, "language").Value,
                Text(styles, styleName, OdfNamespaces.FoCompatible, "country").Value));
    }

    private static OdfProperty Paragraph(
        OdfStyles styles, string? styleName, string ns, string name)
        => styles.ResolveProperty(
            styleName, OdfStyleFamily.Paragraph, OdfPropertyKind.Paragraph, ns, name);

    private static OdfProperty Text(OdfStyles styles, string? styleName, string ns, string name)
        => styles.ResolveProperty(
            styleName, OdfStyleFamily.Paragraph, OdfPropertyKind.Text, ns, name);

    private static Length Measure(OdfStyles styles, string? styleName, string name)
        => Paragraph(styles, styleName, OdfNamespaces.FoCompatible, name).AsLength() ?? Length.Zero;

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
            return LineSpacingRule.AtLeast(atLeast);
        }

        if (Paragraph(styles, styleName, OdfNamespaces.Style, "line-spacing").AsLength()
            is { } leading)
        {
            return LineSpacingRule.PlusLeading(leading);
        }

        OdfProperty height = Paragraph(styles, styleName, OdfNamespaces.FoCompatible, "line-height");
        if (!height.HasValue || height.Is("normal")) return LineSpacingRule.SingleSpaced;

        if (height.AsPercentage() is { } proportion) return LineSpacingRule.Multiple(proportion);
        if (height.AsLength() is { } exact) return LineSpacingRule.Exactly(exact);

        return LineSpacingRule.SingleSpaced;
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

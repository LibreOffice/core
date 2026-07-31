using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Units;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>The character formatting a paragraph's text is set in.</summary>
/// <param name="FamilyName">The family the document asks for, before substitution.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">True when the text is italic.</param>
/// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
public readonly record struct WordTextStyle(
    string? FamilyName,
    Length Size,
    int Weight,
    bool IsItalic,
    string? Language);

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

    /// <summary>Resolves a paragraph's layout properties.</summary>
    /// <param name="styles">The document's styles.</param>
    /// <param name="paragraphProperties">The paragraph's own <c>w:pPr</c>, or null.</param>
    /// <param name="defaultTabInterval">The document's <c>w:defaultTabStop</c>.</param>
    internal static ParagraphFormat Resolve(
        WordStyles styles, XElement? paragraphProperties, Length defaultTabInterval)
    {
        ArgumentNullException.ThrowIfNull(styles);

        string? styleId = Word.Attribute(Word.Child(paragraphProperties, "pStyle"), "val")
                          ?? styles.DefaultStyleId(WordStyleType.Paragraph);

        XElement? indent = Layer(styles, paragraphProperties, styleId, "ind");
        XElement? spacing = Layer(styles, paragraphProperties, styleId, "spacing");

        return new ParagraphFormat
        {
            Alignment = Alignment(Word.Attribute(
                Layer(styles, paragraphProperties, styleId, "jc"), "val")),

            // w:start and w:left are the same attribute under two names: the first is the
            // reading-direction form ECMA-376 standardised on and the second is what Word 2007 wrote
            // and what most files in existence still carry.
            StartIndent = Twips(indent, "start") ?? Twips(indent, "left") ?? Length.Zero,
            EndIndent = Twips(indent, "end") ?? Twips(indent, "right") ?? Length.Zero,
            FirstLineIndent = FirstLine(indent),

            SpaceBefore = Twips(spacing, "before") ?? Length.Zero,
            SpaceAfter = Twips(spacing, "after") ?? Length.Zero,
            HasContextualSpacing = IsOn(styles, paragraphProperties, styleId, "contextualSpacing"),
            LineSpacing = Spacing(spacing),

            KeepWithNext = IsOn(styles, paragraphProperties, styleId, "keepNext"),
            KeepTogether = IsOn(styles, paragraphProperties, styleId, "keepLines"),

            // Word states widow control as one flag rather than two counts, and it means two of each —
            // which is why a document with it on sometimes has a visibly short page.
            OrphanLines = IsOn(styles, paragraphProperties, styleId, "widowControl") ? 2 : 0,
            WidowLines = IsOn(styles, paragraphProperties, styleId, "widowControl") ? 2 : 0,

            StartsNewPage = StartsNewPage(styles, paragraphProperties, styleId),
            DefaultTabInterval =
                defaultTabInterval > Length.Zero ? defaultTabInterval : Length.FromTwips(720),
        };
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

        string? styleId = Word.Attribute(Word.Child(paragraphProperties, "pStyle"), "val")
                          ?? styles.DefaultStyleId(WordStyleType.Paragraph);

        // A paragraph's mark carries its own run properties, and they are what the paragraph's text
        // inherits when no run overrides them.
        XElement? runProperties = Word.Child(paragraphProperties, "rPr");

        WordProperty fonts = styles.ResolveRunProperty("rFonts", runProperties, styleId, null);
        WordProperty size = styles.ResolveRunProperty("sz", runProperties, styleId, null);
        WordProperty bold = styles.ResolveRunProperty("b", runProperties, styleId, null);
        WordProperty italic = styles.ResolveRunProperty("i", runProperties, styleId, null);
        WordProperty language = styles.ResolveRunProperty("lang", runProperties, styleId, null);

        return new WordTextStyle(
            Family(fonts.Element),
            HalfPoints(size.Element) ?? DefaultSize,
            bold.IsOn ? 700 : 400,
            italic.IsOn,
            Word.Attribute(language.Element, "val"));
    }

    /// <summary>
    /// A paragraph property from the direct formatting first, then the style chain, then the defaults.
    /// </summary>
    /// <remarks>
    /// Per property rather than per element, because a paragraph's own <c>w:pPr</c> is not a replacement
    /// for its style's — it is an overlay, and each child of it overrides only its counterpart.
    /// </remarks>
    private static XElement? Layer(
        WordStyles styles, XElement? paragraphProperties, string? styleId, string localName)
    {
        if (Word.Child(paragraphProperties, localName) is { } direct) return direct;

        WordProperty fromStyle = styles.ResolveInStyleChain(
            styleId, WordStyleType.Paragraph, runProperty: false, localName);
        if (fromStyle.HasValue) return fromStyle.Element;

        return styles.ResolveInDocumentDefaults(runProperty: false, localName).Element;
    }

    private static bool IsOn(
        WordStyles styles, XElement? paragraphProperties, string? styleId, string localName)
    {
        if (Word.Child(paragraphProperties, localName) is { } direct) return Word.IsOn(direct);

        WordProperty fromStyle = styles.ResolveInStyleChain(
            styleId, WordStyleType.Paragraph, runProperty: false, localName);
        if (fromStyle.HasValue) return fromStyle.IsOn;

        return styles.ResolveInDocumentDefaults(runProperty: false, localName).IsOn;
    }

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
    private static Length FirstLine(XElement? indent)
    {
        if (Twips(indent, "hanging") is { } hanging) return -hanging;
        return Twips(indent, "firstLine") ?? Length.Zero;
    }

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
        WordStyles styles, XElement? paragraphProperties, string? styleId)
        => IsOn(styles, paragraphProperties, styleId, "pageBreakBefore");

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

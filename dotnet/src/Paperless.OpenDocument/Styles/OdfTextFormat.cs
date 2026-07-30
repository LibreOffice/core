using System.Globalization;
using Paperless.Core.Extraction;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.OpenDocument.Styles;

/// <summary>Where a run sits relative to the baseline.</summary>
public enum OdfTextPosition
{
    /// <summary>On the baseline.</summary>
    Baseline = 0,

    /// <summary>Raised.</summary>
    Superscript,

    /// <summary>Lowered.</summary>
    Subscript,
}

/// <summary>
/// The character formatting in force for a run of text, resolved through its style cascade.
/// </summary>
/// <remarks>
/// <para>
/// Only the properties extraction can act on are surfaced here. Everything else stays
/// available through <see cref="OdfStyles"/>, so this type can grow when layout
/// needs more rather than carrying fields nothing reads.
/// </para>
/// <para>
/// Each field records what was resolved, not merely what was set: a null means nothing in
/// the cascade <em>or</em> the family defaults supplied a value, which for a font name or
/// size means the document is relying on the application's own defaults.
/// </para>
/// </remarks>
public sealed record OdfTextFormat
{
    /// <summary>Nothing resolved — the formatting an empty cascade produces.</summary>
    public static readonly OdfTextFormat None = new();

    /// <summary>
    /// The font as named by <c>style:font-name</c>, which refers to an
    /// <see cref="OdfFontFace"/> declaration, or the first family from
    /// <c>fo:font-family</c> when the style names one directly.
    /// </summary>
    public string? FontName { get; init; }

    /// <summary>The absolute font size, when the style gives one in absolute units.</summary>
    public Length? FontSize { get; init; }

    /// <summary>
    /// The font size as a fraction of the parent's, when the style gives a percentage
    /// (<c>fo:font-size="120%"</c>). Kept separate because resolving it needs the parent
    /// size, which is a layout concern rather than an extraction one.
    /// </summary>
    public double? RelativeFontSize { get; init; }

    /// <summary>True when the run is bold.</summary>
    public bool IsBold { get; init; }

    /// <summary>True when the run is italic or oblique.</summary>
    public bool IsItalic { get; init; }

    /// <summary>True when the run is underlined, by any underline style.</summary>
    public bool IsUnderlined { get; init; }

    /// <summary>True when the run is struck through, by any line-through style.</summary>
    public bool IsStruckThrough { get; init; }

    /// <summary>Whether the run is raised or lowered.</summary>
    public OdfTextPosition Position { get; init; }

    /// <summary>The text colour, when the style sets one.</summary>
    public Colour? Colour { get; init; }

    /// <summary>The text background — a highlight — when the style sets one.</summary>
    public Colour? BackgroundColour { get; init; }

    /// <summary>
    /// The run's language as a BCP 47 tag, assembled from <c>fo:language</c> and
    /// <c>fo:country</c>.
    /// </summary>
    public string? Language { get; init; }

    /// <summary>
    /// True when the run is marked as not spell-checked and carrying no language, which
    /// ODF writes as <c>fo:language="none"</c>.
    /// </summary>
    public bool IsLanguageNone { get; init; }

    /// <summary>The coarse emphasis flags the content tree records.</summary>
    public RunEmphasis Emphasis
    {
        get
        {
            RunEmphasis emphasis = RunEmphasis.None;
            if (IsBold) emphasis |= RunEmphasis.Bold;
            if (IsItalic) emphasis |= RunEmphasis.Italic;
            if (IsUnderlined) emphasis |= RunEmphasis.Underline;
            if (IsStruckThrough) emphasis |= RunEmphasis.Strikethrough;
            if (Position == OdfTextPosition.Superscript) emphasis |= RunEmphasis.Superscript;
            if (Position == OdfTextPosition.Subscript) emphasis |= RunEmphasis.Subscript;
            return emphasis;
        }
    }

    /// <summary>
    /// Resolves the character formatting produced by a cascade of styles, outermost first.
    /// </summary>
    /// <param name="styles">The document's styles.</param>
    /// <param name="cascade">
    /// The style references from outermost to innermost: typically the paragraph style, then
    /// any enclosing spans, then the innermost span. Character formatting on the inner
    /// styles overrides the outer ones, and the family defaults apply only where nothing in
    /// the cascade sets a value at all.
    /// </param>
    public static OdfTextFormat Resolve(OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade)
    {
        ArgumentNullException.ThrowIfNull(styles);
        ArgumentNullException.ThrowIfNull(cascade);
        if (cascade.Count == 0) return None;

        OdfProperty fontSize = Get(OdfNamespaces.FoCompatible, "font-size");
        OdfProperty language = Get(OdfNamespaces.FoCompatible, "language");
        OdfProperty country = Get(OdfNamespaces.FoCompatible, "country");

        return new OdfTextFormat
        {
            FontName = Get(OdfNamespaces.Style, "font-name").Value
                       ?? FirstFamily(Get(OdfNamespaces.FoCompatible, "font-family").Value),
            FontSize = fontSize.AsLength(),
            RelativeFontSize = fontSize.AsPercentage(),
            IsBold = IsBoldWeight(Get(OdfNamespaces.FoCompatible, "font-weight").Value),
            IsItalic = Get(OdfNamespaces.FoCompatible, "font-style").Value is "italic" or "oblique",
            IsUnderlined = IsLineOn(Get(OdfNamespaces.Style, "text-underline-style").Value),
            IsStruckThrough = IsLineOn(Get(OdfNamespaces.Style, "text-line-through-style").Value),
            Position = ParsePosition(Get(OdfNamespaces.Style, "text-position").Value),
            Colour = Get(OdfNamespaces.FoCompatible, "color").AsColour(),
            BackgroundColour = Get(OdfNamespaces.FoCompatible, "background-color").AsColour(),
            Language = ComposeLanguageTag(language.Value, country.Value),
            IsLanguageNone = language.Is("none"),
        };

        OdfProperty Get(string ns, string name)
            => styles.ResolveProperty(cascade, OdfPropertyKind.Text, ns, name);
    }

    /// <summary>
    /// Whether an <c>fo:font-weight</c> value counts as bold.
    /// </summary>
    /// <remarks>
    /// The value is a CSS weight, so it can be a keyword or one of the numeric steps.
    /// LibreOffice treats 600 and above as bold, and matching that keeps a document
    /// authored with <c>font-weight="600"</c> looking the same in both.
    /// </remarks>
    private static bool IsBoldWeight(string? value) => value switch
    {
        null or "normal" or "lighter" => false,
        "bold" or "bolder" => true,
        _ => int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int weight)
             && weight >= 600,
    };

    /// <summary>
    /// Whether an underline or line-through style is drawn. ODF spells "off" as
    /// <c>none</c>; any other value — solid, dotted, wave — draws a line.
    /// </summary>
    private static bool IsLineOn(string? value) => value is not (null or "none");

    private static OdfTextPosition ParsePosition(string? value)
    {
        if (value is null) return OdfTextPosition.Baseline;

        // The value is "super", "sub", or a vertical offset optionally followed by a size,
        // e.g. "33% 58%". A positive offset raises, a negative one lowers.
        ReadOnlySpan<char> text = value.AsSpan().Trim();
        int space = text.IndexOf(' ');
        ReadOnlySpan<char> offset = space < 0 ? text : text[..space];

        if (offset.Equals("super", StringComparison.Ordinal)) return OdfTextPosition.Superscript;
        if (offset.Equals("sub", StringComparison.Ordinal)) return OdfTextPosition.Subscript;

        double? percent = OdfValue.ParsePercentage(offset.ToString());
        return percent switch
        {
            > 0 => OdfTextPosition.Superscript,
            < 0 => OdfTextPosition.Subscript,
            _ => OdfTextPosition.Baseline,
        };
    }

    /// <summary>
    /// The first family from a CSS family list, unquoted:
    /// <c>'Liberation Serif', serif</c> becomes <c>Liberation Serif</c>.
    /// </summary>
    private static string? FirstFamily(string? familyList)
    {
        if (familyList is null) return null;
        ReadOnlySpan<char> first = familyList.AsSpan();
        int comma = first.IndexOf(',');
        if (comma >= 0) first = first[..comma];
        first = first.Trim().Trim('\'').Trim('"').Trim();
        return first.IsEmpty ? null : first.ToString();
    }

    /// <summary>
    /// Assembles a BCP 47 tag from ODF's separate language and country attributes.
    /// </summary>
    /// <remarks>
    /// <c>fo:language="none"</c> is ODF's marker for "deliberately no language" rather than a
    /// language code, and must not become the tag "none".
    /// </remarks>
    private static string? ComposeLanguageTag(string? language, string? country)
    {
        if (string.IsNullOrEmpty(language) || language == "none") return null;
        return string.IsNullOrEmpty(country) ? language : $"{language}-{country}";
    }
}

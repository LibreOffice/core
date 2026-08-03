using System.Text;
using System.Xml.Linq;
using Paperless.Core.Numbering;

namespace Paperless.OpenDocument.Styles;

/// <summary>What a list level draws in front of its items.</summary>
public enum OdfListLabelKind
{
    /// <summary>A generated number, letter or roman numeral.</summary>
    Number = 0,

    /// <summary>A fixed bullet character.</summary>
    Bullet,

    /// <summary>An image.</summary>
    Image,
}

/// <summary>
/// One level of a <c>text:list-style</c> or <c>text:outline-style</c>.
/// </summary>
public sealed class OdfListLevel
{
    internal OdfListLevel(XElement element)
    {
        Level = OdfValue.ParseInt(element.Attribute(XName.Get("level", OdfNamespaces.Text))?.Value) ?? 1;
        Kind = element.Name.LocalName switch
        {
            "list-level-style-bullet" => OdfListLabelKind.Bullet,
            "list-level-style-image" => OdfListLabelKind.Image,
            _ => OdfListLabelKind.Number,
        };

        NumberFormat = element.Attribute(XName.Get("num-format", OdfNamespaces.Style))?.Value;
        Prefix = element.Attribute(XName.Get("num-prefix", OdfNamespaces.Style))?.Value;
        Suffix = element.Attribute(XName.Get("num-suffix", OdfNamespaces.Style))?.Value;
        LetterSynchronised =
            OdfValue.ParseBoolean(element.Attribute(XName.Get("num-letter-sync", OdfNamespaces.Style))?.Value)
            ?? false;
        StartValue = OdfValue.ParseInt(element.Attribute(XName.Get("start-value", OdfNamespaces.Text))?.Value) ?? 1;
        DisplayLevels =
            OdfValue.ParseInt(element.Attribute(XName.Get("display-levels", OdfNamespaces.Text))?.Value) ?? 1;
        BulletCharacter = element.Attribute(XName.Get("bullet-char", OdfNamespaces.Text))?.Value;
        // `text:style-name`, not `text:text-style-name`. The attribute is named for its family the way
        // every other ODF style reference is, and a list level is the one place where reading the longer
        // spelling looks plausible — it was read that way here, so this was always null and the character
        // style a level applies to its label was never found at all.
        TextStyleName = element.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;

        XElement? levelProperties = element.Element(XName.Get("list-level-properties", OdfNamespaces.Style));
        LevelProperties = levelProperties is null
            ? null
            : new OdfPropertySet(OdfPropertyKind.ListLevel, levelProperties);

        XElement? textProperties = element.Element(XName.Get("text-properties", OdfNamespaces.Style));
        Typeface =
            textProperties?.Attribute(XName.Get("font-family", OdfNamespaces.FoCompatible))?.Value
            ?? textProperties?.Attribute(XName.Get("font-name", OdfNamespaces.Style))?.Value;
        RelativeSize = OdfValue.ParsePercentage(
            textProperties?.Attribute(XName.Get("font-size", OdfNamespaces.FoCompatible))?.Value);
    }

    /// <summary>
    /// The face the label itself is set in, when the level states one directly.
    /// </summary>
    /// <remarks>
    /// A bullet level almost always does, and it names a symbol font — <c>StarBats</c>,
    /// <c>OpenSymbol</c>, <c>Wingdings</c> — whose character is meaningless in any other face. The
    /// separate <see cref="TextStyleName"/> is a named character style and is the rarer spelling.
    /// </remarks>
    public string? Typeface { get; }

    /// <summary>
    /// The label's size as a fraction of the item's own text, or null when it states none.
    /// </summary>
    /// <remarks>
    /// Written as <c>fo:font-size="45%"</c> in every list style LibreOffice generates, which is
    /// what makes a bullet beside 28 pt text a 12.6 pt glyph.
    /// </remarks>
    public double? RelativeSize { get; }

    /// <summary>The one-based nesting level this definition applies to.</summary>
    public int Level { get; }

    /// <summary>What this level draws as its label.</summary>
    public OdfListLabelKind Kind { get; }

    /// <summary>
    /// The <c>style:num-format</c>: <c>1</c>, <c>a</c>, <c>A</c>, <c>i</c> or <c>I</c>.
    /// An empty string means "numbered, but draw no number" — which is how LibreOffice
    /// writes an outline level that contributes to the hierarchy without being displayed.
    /// </summary>
    public string? NumberFormat { get; }

    /// <summary>Text drawn before the number or bullet.</summary>
    public string? Prefix { get; }

    /// <summary>Text drawn after the number or bullet, commonly <c>.</c> or <c>)</c>.</summary>
    public string? Suffix { get; }

    /// <summary>
    /// Whether alphabetic numbering carries into a doubled letter (<c>aa</c>) rather than
    /// advancing (<c>ab</c>) past the twenty-sixth item.
    /// </summary>
    public bool LetterSynchronised { get; }

    /// <summary>The value the first item at this level takes.</summary>
    public int StartValue { get; }

    /// <summary>
    /// How many levels the label shows, so that level 3 with a value of 3 can render as
    /// <c>1.2.3</c>.
    /// </summary>
    public int DisplayLevels { get; }

    /// <summary>The bullet character, for a bullet level.</summary>
    public string? BulletCharacter { get; }

    /// <summary>
    /// The character style applied to the label itself, from <c>text:style-name</c>.
    /// </summary>
    /// <remarks>
    /// The label's own formatting, and the only place a level can state it absolutely: the level element
    /// carries <c>fo:font-size</c> as a percentage of the item's text (see <see cref="RelativeSize"/>),
    /// while an exact size lives here. LibreOffice's own WW8 import writes a level's <c>grpprlChpx</c>
    /// out this way — a <c>.doc</c> whose bullet level sets 12 pt over 11 pt text round-trips to a
    /// <c>WW8Num1z0</c> character style and a level naming it.
    /// </remarks>
    public string? TextStyleName { get; }

    /// <summary>The label's geometry, when the level declares any.</summary>
    public OdfPropertySet? LevelProperties { get; }
}

/// <summary>
/// A <c>text:list-style</c> or <c>text:outline-style</c>: up to ten numbered or bulleted
/// levels.
/// </summary>
/// <remarks>
/// ODF expresses list <em>structure</em> by nesting <c>text:list</c> elements and list
/// <em>appearance</em> by a separately named list style, which is the opposite of the
/// flat "paragraph plus level attribute" arrangement DOCX and DOC use. Extraction keeps
/// both: the nesting gives <c>ContentParagraph.ListLevel</c>, the style gives the rendered
/// marker.
/// </remarks>
public sealed class OdfListStyle
{
    private readonly Dictionary<int, OdfListLevel> _levels = [];

    internal OdfListStyle(XElement element)
    {
        Name = element.Attribute(XName.Get("name", OdfNamespaces.Style))?.Value ?? string.Empty;
        DisplayName = element.Attribute(XName.Get("display-name", OdfNamespaces.Style))?.Value;
        IsOutlineStyle = element.Name.LocalName == "outline-style";
        ConsecutiveNumbering =
            OdfValue.ParseBoolean(element.Attribute(XName.Get("consecutive-numbering", OdfNamespaces.Text))?.Value)
            ?? false;

        foreach (XElement child in element.Elements())
        {
            if (!child.Name.LocalName.Contains("level-style", StringComparison.Ordinal)) continue;
            OdfListLevel level = new(child);
            _levels[level.Level] = level;
        }
    }

    /// <summary>The style's name, as referenced by <c>text:style-name</c> on a list.</summary>
    public string Name { get; }

    /// <summary>The user-visible name, when recorded separately.</summary>
    public string? DisplayName { get; }

    /// <summary>
    /// True for <c>text:outline-style</c>, the single per-document style that numbers
    /// headings. It is not referenced by name from content; headings pick it up implicitly.
    /// </summary>
    public bool IsOutlineStyle { get; }

    /// <summary>Whether numbering runs continuously across all levels rather than per level.</summary>
    public bool ConsecutiveNumbering { get; }

    /// <summary>The defined levels, keyed by their one-based level number.</summary>
    public IReadOnlyDictionary<int, OdfListLevel> Levels => _levels;

    /// <summary>
    /// The definition for a level, falling back to the deepest defined level above it.
    /// </summary>
    /// <remarks>
    /// Real files often define only level 1 and rely on it for deeper nesting, so falling
    /// back is what makes a three-deep list built from a one-level style render at all.
    /// </remarks>
    public OdfListLevel? GetLevel(int level)
    {
        for (int candidate = level; candidate >= 1; candidate--)
        {
            if (_levels.TryGetValue(candidate, out OdfListLevel? found)) return found;
        }
        return null;
    }

    /// <summary>
    /// Renders the label for an item, given the counter value at every level from 1 up to
    /// <paramref name="level"/>.
    /// </summary>
    /// <param name="level">The one-based level of the item being labelled.</param>
    /// <param name="counters">
    /// Counter values indexed by level minus one. Values for levels deeper than
    /// <paramref name="level"/> are ignored.
    /// </param>
    /// <returns>
    /// The label as it would be drawn, or null when this level draws nothing — an outline
    /// level with an empty <c>style:num-format</c>, or a bullet level with no character.
    /// </returns>
    public string? FormatLabel(int level, IReadOnlyList<int> counters)
    {
        ArgumentNullException.ThrowIfNull(counters);
        OdfListLevel? definition = GetLevel(level);
        if (definition is null) return null;

        if (definition.Kind == OdfListLabelKind.Bullet)
        {
            return definition.BulletCharacter is { Length: > 0 } bullet
                ? definition.Prefix + OutlineNumbers.NormaliseBullet(bullet) + definition.Suffix
                : null;
        }
        if (definition.Kind == OdfListLabelKind.Image) return null;

        // An empty (but present) num-format means the level is unnumbered. A missing one
        // defaults to decimal, which is what a list style written by a converter that
        // omitted the attribute means.
        if (definition.NumberFormat is { Length: 0 }) return null;

        int displayFrom = Math.Max(1, level - Math.Max(1, definition.DisplayLevels) + 1);
        StringBuilder label = new();
        label.Append(definition.Prefix);

        for (int component = displayFrom; component <= level; component++)
        {
            if (component > displayFrom) label.Append('.');
            int value = component - 1 < counters.Count ? counters[component - 1] : 1;

            // Each component is formatted with its own level's number format, so a level-2
            // roman numeral under a level-1 decimal renders as "1.ii" rather than "1.2".
            OdfListLevel? componentLevel = GetLevel(component);
            label.Append(FormatNumber(value, componentLevel?.NumberFormat,
                                      componentLevel?.LetterSynchronised ?? false));
        }

        label.Append(definition.Suffix);
        return label.ToString();
    }

    /// <summary>
    /// Formats one counter value in an ODF number format.
    /// </summary>
    /// <param name="value">The counter value; values below 1 render as the value itself.</param>
    /// <param name="format">
    /// The <c>style:num-format</c>. Anything other than the five formats ODF defines falls
    /// back to decimal rather than being dropped — an unrecognised format is far more
    /// likely to be a native numbering Paperless has not implemented than a reason to
    /// render no label at all.
    /// </param>
    /// <param name="letterSynchronised">
    /// Whether alphabetic numbering doubles the letter (<c>aa</c>) rather than advancing
    /// (<c>ab</c>) past 26.
    /// </param>
    public static string FormatNumber(int value, string? format, bool letterSynchronised = false)
        => format switch
        {
            "a" => OutlineNumbers.Alphabetic(value, upperCase: false, letterSynchronised),
            "A" => OutlineNumbers.Alphabetic(value, upperCase: true, letterSynchronised),
            "i" => OutlineNumbers.Roman(value, upperCase: false),
            "I" => OutlineNumbers.Roman(value, upperCase: true),
            _ => OutlineNumbers.Digits(value),
        };
}

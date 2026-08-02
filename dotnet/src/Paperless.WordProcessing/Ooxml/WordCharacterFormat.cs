using System.Xml.Linq;
using Paperless.Core.Extraction;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// The character formatting in force for a run, resolved across every WordprocessingML layer.
/// </summary>
/// <remarks>
/// Only what extraction can act on is surfaced. The resolution behind it is the interesting
/// part — see <see cref="WordStyles.ResolveRunProperty"/> for the toggle rule that decides
/// whether bold inside a bold style is bold.
/// </remarks>
public sealed record WordCharacterFormat
{
    /// <summary>Nothing resolved.</summary>
    public static readonly WordCharacterFormat None = new();

    /// <summary>True when the run is bold.</summary>
    public bool IsBold { get; init; }

    /// <summary>True when the run is italic.</summary>
    public bool IsItalic { get; init; }

    /// <summary>True when the run is underlined by any underline style.</summary>
    public bool IsUnderlined { get; init; }

    /// <summary>True when the run is struck through, single or double.</summary>
    public bool IsStruckThrough { get; init; }

    /// <summary>True when the run is raised.</summary>
    public bool IsSuperscript { get; init; }

    /// <summary>True when the run is lowered.</summary>
    public bool IsSubscript { get; init; }

    /// <summary>True when the run is in small capitals or all capitals.</summary>
    public bool IsCapitalised { get; init; }

    /// <summary>
    /// True when the run is marked hidden (<c>w:vanish</c>), which means no reader displays it.
    /// </summary>
    public bool IsHidden { get; init; }

    /// <summary>The font size, when a layer sets one.</summary>
    public Length? FontSize { get; init; }

    /// <summary>The Latin-script font name, when a layer sets one.</summary>
    public string? FontName { get; init; }

    /// <summary>The text colour, when a layer sets one and it is not "automatic".</summary>
    public Colour? Colour { get; init; }

    /// <summary>The run's language as a BCP 47 tag, which is how OOXML already stores it.</summary>
    public string? Language { get; init; }

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
            if (IsSuperscript) emphasis |= RunEmphasis.Superscript;
            if (IsSubscript) emphasis |= RunEmphasis.Subscript;
            return emphasis;
        }
    }

    /// <summary>
    /// Resolves the formatting of a run.
    /// </summary>
    /// <param name="styles">The document's styles.</param>
    /// <param name="directRunProperties">The run's own <c>w:rPr</c>, or null.</param>
    /// <param name="paragraphStyleId">The paragraph style in force, or null.</param>
    /// <param name="theme">
    /// The document's theme, for a <c>w:themeColor</c>. Null leaves a themed colour unresolved
    /// rather than guessing at it, which is what a package with no theme part deserves.
    /// </param>
    public static WordCharacterFormat Resolve(
        WordStyles styles,
        XElement? directRunProperties,
        string? paragraphStyleId,
        DrawingTheme? theme = null)
    {
        ArgumentNullException.ThrowIfNull(styles);

        // The character style is named by the run itself, inside the very property set being
        // resolved — so it has to be read before anything else can be.
        string? characterStyleId = Word.Value(directRunProperties, "rStyle");

        WordProperty underline = Get("u");
        WordProperty verticalAlignment = Get("vertAlign");
        WordProperty size = Get("sz");
        WordProperty colour = Get("color");

        return new WordCharacterFormat
        {
            IsBold = Get("b").IsOn,
            IsItalic = Get("i").IsOn,
            // w:u carries the line style in w:val, so "none" is off and anything else is on.
            IsUnderlined = underline.HasValue && underline.Value is not (null or "none"),
            IsStruckThrough = Get("strike").IsOn || Get("dstrike").IsOn,
            IsSuperscript = verticalAlignment.Value == "superscript",
            IsSubscript = verticalAlignment.Value == "subscript",
            IsCapitalised = Get("caps").IsOn || Get("smallCaps").IsOn,
            IsHidden = Get("vanish").IsOn,
            // w:sz is in half-points, which is the unit that catches people out: 24 is 12pt.
            FontSize = size.IntegerValue is { } halfPoints and > 0
                ? Length.FromPoints(halfPoints / 2.0)
                : null,
            // Every layer, not the innermost one: w:rFonts is inherited attribute by attribute, so
            // a run naming only a complex-script face still has its style's Latin one.
            FontName = LatinFamily(
                styles.RunPropertyLayers(
                    "rFonts", directRunProperties, paragraphStyleId, characterStyleId),
                theme),
            Colour = WordThemeColour.Read(colour.Element, theme),
            Language = Get("lang").Value,
        };

        WordProperty Get(string localName)
            => styles.ResolveRunProperty(
                localName, directRunProperties, paragraphStyleId, characterStyleId);
    }

    /// <summary>
    /// The Latin family the innermost layer to name one gives, or null when none does.
    /// </summary>
    /// <remarks>
    /// Only the ASCII slot, unlike the layout path's search: this is reported as "the font", and a
    /// document whose runs name only an East Asian face has no Latin font to report rather than an
    /// East Asian one masquerading as one.
    /// </remarks>
    private static string? LatinFamily(List<XElement> layers, DrawingTheme? theme)
    {
        foreach (XElement fonts in layers)
        {
            if (WordParagraphFormats.SlotFamily(fonts, theme?.Fonts, "ascii", "asciiTheme") is { } name)
            {
                return name;
            }
        }

        return null;
    }
}

using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Ooxml;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// Resolves the four ways SpreadsheetML can name a colour.
/// </summary>
/// <remarks>
/// <c>rgb</c> is an ARGB string; <c>indexed</c> points into the legacy 56-entry palette,
/// which a workbook may override with its own <c>indexedColors</c>; <c>theme</c> points into
/// the theme's colour scheme in a fixed order that is <em>not</em> the order the scheme
/// element writes them in; and any of the three may carry a <c>tint</c> that lightens or
/// darkens the result.
/// </remarks>
internal sealed class XlsxPalette
{
    /// <summary>
    /// The theme slots, in the order <c>theme="n"</c> numbers them.
    /// </summary>
    /// <remarks>
    /// Light and dark are <em>swapped</em> against the scheme's own element order: slot 0 is
    /// <c>lt1</c> and slot 1 is <c>dk1</c>, which is a documented quirk of SpreadsheetML's
    /// indices rather than a mistake. LibreOffice writes the same swap
    /// (<c>oox/source/drawingml/themeelementscontext.cxx</c> reads the scheme in element
    /// order, and <c>sc/source/filter/oox/stylesbuffer.cxx</c> maps the index through
    /// <c>getColorByIndex</c>).
    /// </remarks>
    private static readonly string[] ThemeSlots =
        ["lt1", "dk1", "lt2", "dk2", "accent1", "accent2", "accent3", "accent4", "accent5",
         "accent6", "hlink", "folHlink"];

    private readonly Dictionary<int, Colour> _indexed = [];
    private readonly List<Colour> _theme = [];

    public static XlsxPalette Read(XElement? styles, XElement? theme)
    {
        XlsxPalette palette = new();

        int at = 0;
        foreach (XElement colour in Xlsx.Children(
                     Xlsx.Child(Xlsx.Child(styles, "colors"), "indexedColors"), "rgbColor"))
        {
            if (ParseRgb(Xlsx.Attribute(colour, "rgb")) is { } parsed) palette._indexed[at] = parsed;
            at++;
        }

        XElement? scheme = theme?
            .Element(XName.Get("themeElements", OoxmlNamespaces.DrawingML))?
            .Element(XName.Get("clrScheme", OoxmlNamespaces.DrawingML));

        foreach (string slot in ThemeSlots)
        {
            palette._theme.Add(SchemeColour(scheme, slot) ?? Colour.Black);
        }

        return palette;
    }

    /// <summary>The colour a <c>color</c> element names, or null when it names none.</summary>
    public Colour? Read(XElement? element)
    {
        if (element is null) return null;
        if (Xlsx.Flag(element, "auto")) return null;

        Colour? colour = null;

        if (Xlsx.Attribute(element, "rgb") is { } rgb) colour = ParseRgb(rgb);
        else if (Xlsx.Integer(element, "indexed") is { } indexed) colour = Indexed(indexed);
        else if (Xlsx.Integer(element, "theme") is { } theme) colour = Theme(theme);

        if (colour is not { } found) return null;

        double tint = Xlsx.Double(Xlsx.Attribute(element, "tint")) ?? 0;
        return Math.Abs(tint) < 0.0001 ? found : Tint(found, tint);
    }

    /// <summary>The colour a theme slot names, or null when the index names none.</summary>
    /// <remarks>
    /// The slots are the SpreadsheetML order, not the scheme's own — see <c>ThemeSlots</c>.
    /// </remarks>
    public Colour? Theme(int index)
        => index >= 0 && index < _theme.Count ? _theme[index] : null;

    /// <summary>The colour an <c>indexed</c> value names, or null when it names none.</summary>
    public Colour? Indexed(int index)
    {
        if (_indexed.TryGetValue(index, out Colour stated)) return stated;

        // 64 and 65 are "automatic foreground" and "automatic background", which have no
        // colour of their own: they resolve to the window text and window background, and
        // Calc renders them as black on white.
        return index switch
        {
            >= 0 and < 64 => Colour.FromRgb(DefaultIndexed[index]),
            64 or 81 => Colour.Black,
            65 => Colour.White,
            _ => null,
        };
    }

    private static Colour? SchemeColour(XElement? scheme, string slot)
    {
        XElement? entry = scheme?.Element(XName.Get(slot, OoxmlNamespaces.DrawingML));
        if (entry is null) return null;

        XElement? srgb = entry.Element(XName.Get("srgbClr", OoxmlNamespaces.DrawingML));
        if (srgb?.Attribute("val")?.Value is { } value) return ParseRgb(value);

        XElement? system = entry.Element(XName.Get("sysClr", OoxmlNamespaces.DrawingML));
        if (system?.Attribute("lastClr")?.Value is { } last) return ParseRgb(last);

        return null;
    }

    private static Colour? ParseRgb(string? value)
    {
        if (value is null) return null;

        string text = value.Trim().TrimStart('#');
        if (!uint.TryParse(text, NumberStyles.HexNumber, CultureInfo.InvariantCulture,
                           out uint packed))
        {
            return null;
        }

        // Eight digits are ARGB and six are RGB; the alpha is dropped either way, because a
        // half-transparent cell fill is not something any of the three formats really means.
        return Colour.FromRgb(text.Length > 6 ? packed & 0x00FFFFFFu : packed);
    }

    /// <summary>
    /// Lightens or darkens a colour by a tint, which is a luminance modulation in HSL.
    /// </summary>
    /// <remarks>
    /// <see cref="XlsxTint"/> holds the transform and the measurements behind it. This used
    /// to compute the same target luminance and then apply the <em>difference</em> as one
    /// additive offset to all three RGB channels, which clamps whichever channel is already
    /// brightest and so shifts the hue — turning the stock gold accent into lemon.
    /// </remarks>
    private static Colour Tint(Colour colour, double tint) => XlsxTint.Apply(colour, tint);

    /// <summary>
    /// The legacy 64-entry palette a workbook that declares none falls back to.
    /// </summary>
    /// <remarks>
    /// LibreOffice's own copy, <c>spPreDefColors</c>
    /// (<c>sc/source/filter/oox/stylesbuffer.cxx</c>) — entries 0-7 repeat as 8-15, which is
    /// not an error in the transcription but how the palette has always been defined.
    /// </remarks>
    private static readonly uint[] DefaultIndexed =
    [
        0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
        0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
        0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080,
        0x9999FF, 0x993366, 0xFFFFCC, 0xCCFFFF, 0x660066, 0xFF8080, 0x0066CC, 0xCCCCFF,
        0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
        0x00CCFF, 0xCCFFFF, 0xCCFFCC, 0xFFFF99, 0x99CCFF, 0xFF99CC, 0xCC99FF, 0xFFCC99,
        0x3366FF, 0x33CCCC, 0x99CC00, 0xFFCC00, 0xFF9900, 0xFF6600, 0x666699, 0x969696,
        0x003366, 0x339966, 0x003300, 0x333300, 0x993300, 0x993366, 0x333399, 0x333333,
    ];
}

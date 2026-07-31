using Paperless.Core.Graphics;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// DrawingML's named colours, which <c>a:prstClr</c> refers to by name.
/// </summary>
/// <remarks>
/// <para>
/// A hundred and forty of them, transcribed from <c>oox/source/drawingml/color.cxx</c>'s
/// <c>constDmlColors</c> (line 40) rather than from the X11 list they resemble. They are not
/// the same list: DrawingML abbreviates (<c>dkBlue</c>, <c>ltGray</c>, <c>medPurple</c>) where
/// X11 spells the word out, and at least one value differs —
/// <c>ltGoldenrodYellow</c> is <c>FAFA78</c> here and <c>FAFAD2</c> in X11.
/// </para>
/// <para>
/// VML's sixteen-name list and WordprocessingML's <c>w:highlight</c> list are different again
/// and belong with their own readers; this is the DrawingML one.
/// </para>
/// </remarks>
public static class PresetColours
{
    private static readonly Dictionary<string, uint> ByName = new(StringComparer.Ordinal)
    {
        ["aliceBlue"] = 0xF0F8FFu, ["antiqueWhite"] = 0xFAEBD7u,
        ["aqua"] = 0x00FFFFu, ["aquamarine"] = 0x7FFFD4u,
        ["azure"] = 0xF0FFFFu, ["beige"] = 0xF5F5DCu,
        ["bisque"] = 0xFFE4C4u, ["black"] = 0x000000u,
        ["blanchedAlmond"] = 0xFFEBCDu, ["blue"] = 0x0000FFu,
        ["blueViolet"] = 0x8A2BE2u, ["brown"] = 0xA52A2Au,
        ["burlyWood"] = 0xDEB887u, ["cadetBlue"] = 0x5F9EA0u,
        ["chartreuse"] = 0x7FFF00u, ["chocolate"] = 0xD2691Eu,
        ["coral"] = 0xFF7F50u, ["cornflowerBlue"] = 0x6495EDu,
        ["cornsilk"] = 0xFFF8DCu, ["crimson"] = 0xDC143Cu,
        ["cyan"] = 0x00FFFFu, ["deepPink"] = 0xFF1493u,
        ["deepSkyBlue"] = 0x00BFFFu, ["dimGray"] = 0x696969u,
        ["dkBlue"] = 0x00008Bu, ["dkCyan"] = 0x008B8Bu,
        ["dkGoldenrod"] = 0xB8860Bu, ["dkGray"] = 0xA9A9A9u,
        ["dkGreen"] = 0x006400u, ["dkKhaki"] = 0xBDB76Bu,
        ["dkMagenta"] = 0x8B008Bu, ["dkOliveGreen"] = 0x556B2Fu,
        ["dkOrange"] = 0xFF8C00u, ["dkOrchid"] = 0x9932CCu,
        ["dkRed"] = 0x8B0000u, ["dkSalmon"] = 0xE9967Au,
        ["dkSeaGreen"] = 0x8FBC8Bu, ["dkSlateBlue"] = 0x483D8Bu,
        ["dkSlateGray"] = 0x2F4F4Fu, ["dkTurquoise"] = 0x00CED1u,
        ["dkViolet"] = 0x9400D3u, ["dodgerBlue"] = 0x1E90FFu,
        ["firebrick"] = 0xB22222u, ["floralWhite"] = 0xFFFAF0u,
        ["forestGreen"] = 0x228B22u, ["fuchsia"] = 0xFF00FFu,
        ["gainsboro"] = 0xDCDCDCu, ["ghostWhite"] = 0xF8F8FFu,
        ["gold"] = 0xFFD700u, ["goldenrod"] = 0xDAA520u,
        ["gray"] = 0x808080u, ["green"] = 0x008000u,
        ["greenYellow"] = 0xADFF2Fu, ["honeydew"] = 0xF0FFF0u,
        ["hotPink"] = 0xFF69B4u, ["indianRed"] = 0xCD5C5Cu,
        ["indigo"] = 0x4B0082u, ["ivory"] = 0xFFFFF0u,
        ["khaki"] = 0xF0E68Cu, ["lavender"] = 0xE6E6FAu,
        ["lavenderBlush"] = 0xFFF0F5u, ["lawnGreen"] = 0x7CFC00u,
        ["lemonChiffon"] = 0xFFFACDu, ["lime"] = 0x00FF00u,
        ["limeGreen"] = 0x32CD32u, ["linen"] = 0xFAF0E6u,
        ["ltBlue"] = 0xADD8E6u, ["ltCoral"] = 0xF08080u,
        ["ltCyan"] = 0xE0FFFFu, ["ltGoldenrodYellow"] = 0xFAFA78u,
        ["ltGray"] = 0xD3D3D3u, ["ltGreen"] = 0x90EE90u,
        ["ltPink"] = 0xFFB6C1u, ["ltSalmon"] = 0xFFA07Au,
        ["ltSeaGreen"] = 0x20B2AAu, ["ltSkyBlue"] = 0x87CEFAu,
        ["ltSlateGray"] = 0x778899u, ["ltSteelBlue"] = 0xB0C4DEu,
        ["ltYellow"] = 0xFFFFE0u, ["magenta"] = 0xFF00FFu,
        ["maroon"] = 0x800000u, ["medAquamarine"] = 0x66CDAAu,
        ["medBlue"] = 0x0000CDu, ["medOrchid"] = 0xBA55D3u,
        ["medPurple"] = 0x9370DBu, ["medSeaGreen"] = 0x3CB371u,
        ["medSlateBlue"] = 0x7B68EEu, ["medSpringGreen"] = 0x00FA9Au,
        ["medTurquoise"] = 0x48D1CCu, ["medVioletRed"] = 0xC71585u,
        ["midnightBlue"] = 0x191970u, ["mintCream"] = 0xF5FFFAu,
        ["mistyRose"] = 0xFFE4E1u, ["moccasin"] = 0xFFE4B5u,
        ["navajoWhite"] = 0xFFDEADu, ["navy"] = 0x000080u,
        ["oldLace"] = 0xFDF5E6u, ["olive"] = 0x808000u,
        ["oliveDrab"] = 0x6B8E23u, ["orange"] = 0xFFA500u,
        ["orangeRed"] = 0xFF4500u, ["orchid"] = 0xDA70D6u,
        ["paleGoldenrod"] = 0xEEE8AAu, ["paleGreen"] = 0x98FB98u,
        ["paleTurquoise"] = 0xAFEEEEu, ["paleVioletRed"] = 0xDB7093u,
        ["papayaWhip"] = 0xFFEFD5u, ["peachPuff"] = 0xFFDAB9u,
        ["peru"] = 0xCD853Fu, ["pink"] = 0xFFC0CBu,
        ["plum"] = 0xDDA0DDu, ["powderBlue"] = 0xB0E0E6u,
        ["purple"] = 0x800080u, ["red"] = 0xFF0000u,
        ["rosyBrown"] = 0xBC8F8Fu, ["royalBlue"] = 0x4169E1u,
        ["saddleBrown"] = 0x8B4513u, ["salmon"] = 0xFA8072u,
        ["sandyBrown"] = 0xF4A460u, ["seaGreen"] = 0x2E8B57u,
        ["seaShell"] = 0xFFF5EEu, ["sienna"] = 0xA0522Du,
        ["silver"] = 0xC0C0C0u, ["skyBlue"] = 0x87CEEBu,
        ["slateBlue"] = 0x6A5ACDu, ["slateGray"] = 0x708090u,
        ["snow"] = 0xFFFAFAu, ["springGreen"] = 0x00FF7Fu,
        ["steelBlue"] = 0x4682B4u, ["tan"] = 0xD2B48Cu,
        ["teal"] = 0x008080u, ["thistle"] = 0xD8BFD8u,
        ["tomato"] = 0xFF6347u, ["turquoise"] = 0x40E0D0u,
        ["violet"] = 0xEE82EEu, ["wheat"] = 0xF5DEB3u,
        ["white"] = 0xFFFFFFu, ["whiteSmoke"] = 0xF5F5F5u,
        ["yellow"] = 0xFFFF00u, ["yellowGreen"] = 0x9ACD32u,
    };

    /// <summary>The colour a preset name refers to, or null when the name is not one.</summary>
    public static Colour? Lookup(string? name)
        => name is not null && ByName.TryGetValue(name, out uint rgb) ? Colour.FromRgb(rgb) : null;
}

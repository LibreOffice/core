using Paperless.Core.Graphics;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// The two ways a DOC states a colour: an index into Word's fixed palette, and a raw <c>COLORREF</c>.
/// </summary>
/// <remarks>
/// Shared rather than repeated because both spellings turn up in three unrelated places — a run's colour, a
/// border's, and a cell's shading — and the raw form is the one that is easy to get wrong: a
/// <c>COLORREF</c> is <em>BGR</em>, so reading it as RGB turns every red border blue.
/// </remarks>
internal static class Ww8Colours
{
    /// <summary>
    /// The seventeen colours an <c>ico</c> index names.
    /// </summary>
    /// <remarks>
    /// Copied from <c>SwWW8ImplReader::GetCol</c> (<c>sw/source/filter/ww8/ww8par6.cxx</c>), whose order is
    /// not the obvious one: index 2 is <em>light</em> blue and index 9 is blue, so the palette runs bright
    /// colours first and dark ones second. Index 0 is the automatic colour and is null rather than black,
    /// so whatever default applies in the context can apply.
    /// </remarks>
    private static readonly Colour?[] Palette =
    [
        null,
        Colour.FromRgb(0x000000),
        Colour.FromRgb(0x0000FF),
        Colour.FromRgb(0x00FFFF),
        Colour.FromRgb(0x00FF00),
        Colour.FromRgb(0xFF00FF),
        Colour.FromRgb(0xFF0000),
        Colour.FromRgb(0xFFFF00),
        Colour.FromRgb(0xFFFFFF),
        Colour.FromRgb(0x000080),
        Colour.FromRgb(0x008080),
        Colour.FromRgb(0x008000),
        Colour.FromRgb(0x800080),
        Colour.FromRgb(0x800000),
        Colour.FromRgb(0x808000),
        Colour.FromRgb(0x808080),
        Colour.FromRgb(0xC0C0C0),
    ];

    /// <summary>
    /// The colour an <c>ico</c> index names, or null for the automatic colour.
    /// </summary>
    /// <param name="index">The index. Out of range counts as automatic, which is what LibreOffice does.</param>
    public static Colour? At(int index)
        => index >= 0 && index < Palette.Length ? Palette[index] : null;

    /// <summary>
    /// A four-byte <c>COLORREF</c> as a colour, or null when it says "automatic".
    /// </summary>
    /// <param name="value">The little-endian value: blue, green, red, then a flag byte.</param>
    /// <remarks>
    /// The fourth byte is not an alpha channel Paperless can use — Word writes 0xFF there to mean the
    /// automatic colour and zero otherwise, and LibreOffice tests the whole word against
    /// <c>0xFF000000</c> for exactly that. Anything else with the byte set is a colour Word would treat as
    /// transparent, which for the two callers here — a border and a cell's shading — means "nothing
    /// stated" just the same.
    /// </remarks>
    public static Colour? FromColorRef(uint value)
        => (value & 0xFF000000) != 0
            ? null
            : Colour.FromRgb(((value & 0xFF) << 16) | (value & 0xFF00) | ((value >> 16) & 0xFF));
}

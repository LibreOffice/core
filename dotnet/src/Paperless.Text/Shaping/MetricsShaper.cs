using Paperless.Text.Fonts;

namespace Paperless.Text.Shaping;

/// <summary>
/// A shaper that only knows the font's advance widths: one glyph per code point, no kerning, no
/// ligatures, no mark positioning.
/// </summary>
/// <remarks>
/// <para>
/// The fallback for when the native shaper cannot be loaded, and the baseline a comparison against
/// LibreOffice can attribute differences to. It is exactly right for the unkerned case and knowably
/// wrong otherwise, which is more useful than an approximation of kerning would be: a wrong kern is
/// worse than a known-absent one, because it cannot be reasoned about.
/// </para>
/// <para>
/// It is also the honest answer for a face with no <c>GPOS</c> and no <c>kern</c> table, which is most
/// CJK fonts and a good many older Latin ones — for those this shaper and HarfBuzz agree exactly.
/// </para>
/// </remarks>
public sealed class MetricsShaper : ITextShaper
{
    /// <summary>The shared instance; the shaper holds no state.</summary>
    public static MetricsShaper Instance { get; } = new();

    /// <inheritdoc/>
    public ShapedText Shape(OpenTypeFace face, ReadOnlySpan<char> text, ShapingOptions options = default)
    {
        ArgumentNullException.ThrowIfNull(face);
        if (text.Length == 0) return new ShapedText([], [0], face.UnitsPerEm);

        List<ShapedGlyph> glyphs = new(text.Length);
        for (int i = 0; i < text.Length;)
        {
            int codePoint = text[i];
            int width = 1;

            if (char.IsHighSurrogate(text[i]) && i + 1 < text.Length && char.IsLowSurrogate(text[i + 1]))
            {
                codePoint = char.ConvertToUtf32(text[i], text[i + 1]);
                width = 2;
            }

            ushort glyph = face.Characters.GlyphFor(codePoint);
            glyphs.Add(new ShapedGlyph(glyph, i, glyph == 0 ? 0 : face.AdvanceOf(glyph), 0, 0));
            i += width;
        }

        ShapedGlyph[] shaped = [.. glyphs];
        return new ShapedText(shaped, ShapedText.PrefixWidths(shaped, text.Length), face.UnitsPerEm);
    }
}

using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Text.Shaping;

/// <summary>
/// Converts a run of characters into positioned glyphs.
/// </summary>
/// <remarks>
/// LibreOffice shapes with HarfBuzz, so Paperless does too (via HarfBuzzSharp) rather
/// than using a different engine: shaping differences would change advance widths,
/// which would change line breaks, which would change pagination.
/// </remarks>
public interface ITextShaper
{
    /// <summary>
    /// Shapes a single-font, single-direction, single-script run.
    /// </summary>
    /// <param name="text">The characters to shape.</param>
    /// <param name="face">The face to shape with.</param>
    /// <param name="fontSize">The em size to compute advances at.</param>
    /// <param name="options">Directionality, script and language.</param>
    GlyphRun Shape(
        ReadOnlySpan<char> text,
        Fonts.IFontFace face,
        Length fontSize,
        ShapingOptions options);
}

/// <summary>Options controlling how a run is shaped.</summary>
/// <param name="IsRightToLeft">Whether the run reads right-to-left.</param>
/// <param name="Script">An ISO 15924 script code, e.g. <c>Latn</c> or <c>Arab</c>.</param>
/// <param name="Language">A BCP 47 language tag; affects language-specific features.</param>
/// <param name="EnableKerning">Whether to apply the font's kerning.</param>
/// <param name="EnableLigatures">Whether to apply standard ligatures.</param>
/// <param name="LetterSpacing">Extra tracking added to each glyph's advance.</param>
public readonly record struct ShapingOptions(
    bool IsRightToLeft = false,
    string? Script = null,
    string? Language = null,
    bool EnableKerning = true,
    bool EnableLigatures = true,
    Length LetterSpacing = default);

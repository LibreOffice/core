using Paperless.Text.Fonts;

namespace Paperless.Text.Shaping;

/// <summary>
/// How a run of text is to be shaped.
/// </summary>
/// <remarks>
/// <para>
/// Named after what it switches <em>off</em>, so that <c>default</c> means what LibreOffice means by
/// default: kerning and the optional ligatures applied, left to right. LibreOffice's layout arguments
/// are the same way round — <c>SalLayoutFlags::DisableKerning</c> and <c>DisableLigatures</c> push a
/// feature with value zero onto an otherwise empty feature list, and an empty list leaves HarfBuzz's
/// own defaults in place (<c>vcl/source/gdi/CommonSalLayout.cxx</c>). Matching that means a caller who
/// says nothing gets Writer's behaviour rather than an unkerned approximation of it.
/// </para>
/// <para>
/// Kerning is not cosmetic. A line of ordinary English prose at 12 pt carries something like a quarter
/// of an em of accumulated kerning, which is enough to decide whether its last word fits — so a shaper
/// that skips it breaks lines in different places than Writer does, and every line after the first
/// difference is wrong too.
/// </para>
/// </remarks>
/// <param name="Language">
/// A BCP 47 tag. Some features are language-specific, and it is passed through to the shaper for the
/// same reason LibreOffice passes it.
/// </param>
/// <param name="Script">
/// An ISO 15924 code such as <c>Latn</c> or <c>Arab</c>. Left null, the shaper infers one from the
/// text.
/// </param>
/// <param name="DisableKerning">Suppresses the <c>kern</c> feature.</param>
/// <param name="DisableLigatures">
/// Suppresses <c>liga</c> and <c>clig</c> — the optional ligatures only. The orthographically required
/// ones stay, because a script that needs them is unreadable without them.
/// </param>
/// <param name="RightToLeft">Shapes the run right to left.</param>
public readonly record struct ShapingOptions(
    string? Language = null,
    string? Script = null,
    bool DisableKerning = false,
    bool DisableLigatures = false,
    bool RightToLeft = false);

/// <summary>
/// Turns characters into positioned glyphs.
/// </summary>
/// <remarks>
/// Keyed on <see cref="OpenTypeFace"/> and answering in design units rather than at an em size,
/// because that is what the rest of layout needs: advances summed on the design grid and scaled once
/// keep a long line's width equal to the sum of its parts, and a measurement rounded per glyph does
/// not.
/// </remarks>
public interface ITextShaper
{
    /// <summary>
    /// Shapes a run of text with a face.
    /// </summary>
    /// <remarks>
    /// The whole run at once, not character by character: shaping is contextual, so the result for a
    /// run is not the concatenation of the results for its parts.
    /// </remarks>
    ShapedText Shape(OpenTypeFace face, ReadOnlySpan<char> text, ShapingOptions options = default);
}

using Paperless.Core.Graphics;

namespace Paperless.Text.Fonts;

/// <summary>
/// Turns a font request from a document into a concrete face that exists on this
/// machine.
/// </summary>
/// <remarks>
/// <para>
/// This is the single largest source of divergence between Paperless output and any
/// reference renderer, so it is deliberately pluggable and deliberately explicit
/// about what it did.
/// </para>
/// <para>
/// To match LibreOffice, a resolver must reproduce its substitution order: the
/// document's own font table, then LibreOffice's built-in substitution tables, then
/// the platform's (fontconfig on Linux), then a last-resort default. The
/// metric-compatible pairs matter most in practice — Calibri to Carlito, Cambria to
/// Caladea, Arial to Liberation Sans, Times New Roman to Liberation Serif — because
/// those substitutions preserve advance widths and so preserve line breaks. A
/// non-metric-compatible substitution reflows the text and every subsequent page
/// diverges. See <c>dotnet/research/06-rendering.md</c> section B.
/// </para>
/// </remarks>
public interface IFontResolver
{
    /// <summary>
    /// Resolves a request to an available face, substituting when necessary. Never
    /// returns null: a last-resort fallback is always chosen so rendering can proceed.
    /// </summary>
    FontReference Resolve(FontRequest request);

    /// <summary>Loads the face data behind a resolved reference.</summary>
    IFontFace LoadFace(FontReference reference);
}

/// <summary>A font as a document asks for it.</summary>
/// <param name="FamilyName">The requested family name.</param>
/// <param name="Weight">Requested weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">Whether italic was requested.</param>
/// <param name="Pitch">The requested pitch, used as a substitution hint.</param>
/// <param name="EmbeddedFaceKey">
/// A key into the document's own embedded fonts, when it embeds one for this request.
/// Embedded faces always win: they are what the author saw.
/// </param>
public readonly record struct FontRequest(
    string FamilyName,
    int Weight = 400,
    bool IsItalic = false,
    FontPitch Pitch = FontPitch.Unknown,
    string? EmbeddedFaceKey = null);

/// <summary>Whether a font is proportional or fixed-width.</summary>
public enum FontPitch
{
    /// <summary>Not stated by the document.</summary>
    Unknown = 0,

    /// <summary>Proportionally spaced.</summary>
    Variable,

    /// <summary>Fixed-width.</summary>
    Fixed,
}

/// <summary>A loaded font face: metrics, character coverage and glyph outlines.</summary>
public interface IFontFace : IDisposable
{
    /// <summary>The reference this face was loaded from.</summary>
    FontReference Reference { get; }

    /// <summary>
    /// Design units per em, from the font's <c>head</c> table. Glyph metrics are
    /// expressed in these units and scale linearly with the em size.
    /// </summary>
    int UnitsPerEm { get; }

    /// <summary>
    /// The vertical metrics used to derive line height.
    /// </summary>
    /// <remarks>
    /// Which of a font's several competing metric sets to believe is not obvious, and
    /// getting it wrong shifts every baseline on the page. LibreOffice's precedence
    /// rules — hhea versus OS/2 <c>usWin*</c> versus OS/2 typo metrics, plus
    /// per-font overrides — are documented in
    /// <c>dotnet/research/06-rendering.md</c> section B and must be reproduced here.
    /// </remarks>
    FontVerticalMetrics VerticalMetrics { get; }

    /// <summary>True when the face has a glyph for the given Unicode scalar value.</summary>
    bool HasGlyphFor(int codePoint);
}

/// <summary>
/// The vertical metrics that determine baseline placement and line height, in font
/// design units.
/// </summary>
/// <param name="Ascent">Distance above the baseline.</param>
/// <param name="Descent">Distance below the baseline, as a positive value.</param>
/// <param name="LineGap">Extra leading between lines.</param>
/// <param name="UnderlinePosition">Underline offset from the baseline, negative below.</param>
/// <param name="UnderlineThickness">Underline stroke width.</param>
/// <param name="StrikeoutPosition">Strikethrough offset from the baseline.</param>
/// <param name="StrikeoutThickness">Strikethrough stroke width.</param>
public readonly record struct FontVerticalMetrics(
    int Ascent,
    int Descent,
    int LineGap,
    int UnderlinePosition,
    int UnderlineThickness,
    int StrikeoutPosition,
    int StrikeoutThickness);

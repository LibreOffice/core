namespace XRay.Office.Text.Fonts;

/// <summary>
/// Whether this renderer can put ink on the page for a glyph of a face.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The floor under glyph fallback.</strong> A fallback search asks one question — does this
/// face's <c>cmap</c> cover the character — and a face can answer yes and still have nothing to
/// draw with: the glyph's shape may live in a table this renderer does not read. That is exactly
/// what happened when <c>U+2714</c> began resolving to Noto Color Emoji, which carries
/// <c>CBDT</c>/<c>CBLC</c> and neither <c>glyf</c> nor <c>CFF </c>: the right face at the right
/// advance, drawing nothing. A blank where the reference draws a glyph is worse for a reader than a
/// wrong glyph, so a candidate that cannot paint is passed over and the search moves on to the next
/// one — see <c>SystemFontResolver.Covers</c>, which is the single place all three fallback stages
/// go through.
/// </para>
/// <para>
/// Passing over a candidate is a floor and not the fix. The fix is being able to paint it, and for
/// the one colour face installed here that is <see cref="ColourBitmaps"/>. What remains uncovered:
/// </para>
/// <list type="bullet">
/// <item><description><strong><c>COLR</c>/<c>CPAL</c> is not read.</strong> A layered colour glyph
/// is a list of ordinary outlines each carrying a palette colour, which this renderer could compose
/// — but <em>no face installed on this machine carries the tables</em> and no corpus document
/// reaches one, so there is nothing to measure an implementation against. Such a face is reported
/// unpaintable here and the fallback search moves to the next candidate, which draws the character
/// in a monochrome face rather than leaving a blank.</description></item>
/// <item><description><strong><c>sbix</c> and <c>SVG </c> are not read</strong> either, and behave
/// the same way. Apple's colour emoji face is <c>sbix</c>; it is not installed here.</description></item>
/// </list>
/// <para>
/// <c>CFF </c> counts as paintable and that is deliberate, because it is a limitation of one
/// <em>backend</em> rather than of the face: the rasteriser reads Type 2 charstrings through Skia
/// and draws such a face correctly, and only the PDF writer declines to embed the program — see
/// <c>PdfFontCatalogue.IsCompactFontFormat</c>, which names a face it cannot embed and keeps its
/// widths, so the pen positions and the line breaks are unchanged and a reader substitutes a face.
/// Rejecting CFF here would move a line break to work around a writer.
/// </para>
/// </remarks>
public static class GlyphPainting
{
    /// <summary>True when something in the face can produce ink for the glyph.</summary>
    /// <param name="face">The face to ask about.</param>
    /// <param name="glyphId">The glyph index within it.</param>
    public static bool CanPaint(OpenTypeFace? face, ushort glyphId)
    {
        if (face is null) return false;

        // An outline face is paintable whatever this particular glyph's outline turns out to be:
        // an empty `glyf` record is a space, which is "nothing to draw" rather than "cannot draw",
        // and a fallback that rejected a face over its spaces would reject the face over its text.
        if (face.File.Has("glyf") && face.File.Has("loca")) return true;
        if (face.File.Has("CFF ") || face.File.Has("CFF2")) return true;

        return ColourBitmaps.Of(face, glyphId) is not null;
    }

    /// <summary>The same question about a character, for a face that has been asked to cover it.</summary>
    /// <param name="face">The face to ask about.</param>
    /// <param name="codePoint">The character it claims to cover.</param>
    public static bool CanPaintCharacter(OpenTypeFace? face, int codePoint)
    {
        if (face is null) return false;

        ushort glyph = face.Characters.GlyphFor(codePoint);
        return glyph != 0 && CanPaint(face, glyph);
    }
}

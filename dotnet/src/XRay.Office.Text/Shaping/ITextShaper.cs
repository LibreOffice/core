using XRay.Office.Text.Fonts;

namespace XRay.Office.Text.Shaping;

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
    bool RightToLeft = false)
{
    /// <summary>
    /// These options as they apply to a run carrying <paramref name="tracking"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Tracking suppresses the optional ligatures.</strong> A ligature exists to fix a
    /// collision between two letters set at their natural distance; once a designer has pushed
    /// them apart there is no collision, and drawing one anyway sets a joined pair inside a
    /// line that is loose everywhere else. LibreOffice states the rule in one place —
    /// <c>vcl/source/outdev/text.cxx:996-998</c> turns <c>Font::IsFixKerning()</c>, which is
    /// <c>mnSpacing != 0</c>, into <c>SalLayoutFlags::DisableLigatures</c>, and
    /// <c>CommonSalLayout.cxx:453</c> turns that into <c>liga=0, clig=0</c>. The item feeding
    /// it is <c>RES_CHRATR_KERNING</c>/<c>EE_CHAR_KERNING</c>, which is exactly what
    /// <c>w:spacing</c>, <c>\expndtw</c>, <c>a:rPr/@spc</c> and <c>fo:letter-spacing</c> all
    /// land in.
    /// </para>
    /// <para>
    /// It is not a cosmetic difference and it reaches further than the glyphs. Measured on
    /// <c>words/batch-008/…/FAA-2017-0628-0002_attachment_1.docx</c>, whose cover footer is a
    /// 10 pt Carlito-Bold run tracked at <c>w:spacing="60"</c>: forming Carlito's <c>t</c>+<c>i</c>
    /// ligature there put <em>one</em> glyph in the PDF whose <c>ToUnicode</c> entry mapped to
    /// two characters — and poppler responds to a multi-character entry by dropping its
    /// intra-word gap tolerance from 0.400 em to 0.100 em for the whole line, below the 0.300 em
    /// the tracking itself puts between every pair. The line's 45 glyphs then extracted as 45
    /// separate words against the reference's 8, on a document whose whitespace-stripped
    /// character stream was byte-identical to the reference's. So a single wrong ligature cost
    /// 28 words of a 638-word document and made the run unsearchable.
    /// </para>
    /// <para>
    /// The <em>required</em> ligatures are untouched, here as in LibreOffice: <c>rlig</c> is not
    /// in the list, so tracked Arabic still joins.
    /// </para>
    /// </remarks>
    public ShapingOptions WithTracking(Core.Units.Length tracking)
        => tracking == Core.Units.Length.Zero ? this : this with { DisableLigatures = true };
}

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

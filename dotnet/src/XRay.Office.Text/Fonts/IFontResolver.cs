using XRay.Office.Core.Graphics;

namespace XRay.Office.Text.Fonts;

/// <summary>
/// Turns a font request from a document into a concrete face that exists on this
/// machine.
/// </summary>
/// <remarks>
/// <para>
/// This is the single largest source of divergence between X-Ray.Office output and any
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
/// <param name="DeclaredClass">
/// The shape the <em>document</em> says the family has — <c>w:family</c> in a DOCX's font table,
/// the <c>ff</c> bits of a DOC's <c>FFN</c>, <c>style:font-family-generic</c> in ODF,
/// <c>&lt;family val="N"/&gt;</c> on a SpreadsheetML font. Distinct from the shape LibreOffice's
/// substitution table files the <em>name</em> under, and it wins over it: measured on 26.2.4.2, a
/// request for <c>Garamond</c> declared <c>swiss</c> falls back to DejaVu Sans where the same name
/// undeclared falls back to DejaVu Serif, and <c>Futura</c> declared <c>roman</c> falls back to
/// DejaVu Serif where undeclared it falls back to DejaVu Sans.
/// <para>
/// <strong>Both halves of that last sentence are measurements through <em>different filters</em>,
/// which round 54 separated and this comment used to run together.</strong> Undeclared,
/// <c>Futura</c> falls back to DejaVu Sans through the ODF, XLSX and PPTX filters and to DejaVu
/// <em>Serif</em> through the DOCX, DOC and RTF ones, because those three default the class to
/// roman before the request is built — so a word-processing reader hands
/// <see cref="FontFamilyClass.Serif"/> here for a family its font table never mentions. See
/// <c>XRay.Office.WordProcessing.Layout.WordFallbackClass</c>, which is where that default lives and
/// why it is not in the resolver.
/// </para>
/// <see cref="FontFamilyClass.Unknown"/> when the document says nothing, which is the common case.
/// <para>
/// <strong>It is consulted before the substitution chain, not after it.</strong>
/// <c>FontConfigManager::Substitute</c> is LibreOffice's *pre-match* substitution and runs before
/// <c>VCL.xcu</c> is read at all, appending the class as a second <c>FC_FAMILY</c>. The four names
/// that can tell the two orderings apart — <c>Times</c>, <c>Helvetica</c>, <c>Albany</c> and
/// <c>Thorndale</c>, each of which has an installed chain entry — all answer DejaVu once a class is
/// declared. Two things survive it, both measured: a *strong* metric alias bound to the requested
/// name itself, and a pi face. See <c>SystemFontResolver.DeclaredGenericFor</c>.
/// </para>
/// </param>
/// <param name="Language">
/// The language of the font item this request came from, as a BCP 47 tag, or null when the caller
/// carries none.
/// <para>
/// <strong>It is not decoration and it is not the <em>document's</em> language: it is the language
/// of one of the three character-font items, and it outranks the declared class.</strong>
/// <c>FontConfigManager::Substitute</c> adds it to the pattern as <c>FC_LANG</c>
/// (<c>vcl/unx/generic/font/fontconfig.cxx</c>:1092, 1118-1119) and fontconfig scores
/// <c>PRI_LANG</c> above <c>PRI_FAMILY_WEAK</c>, so among the faces covering a missing character
/// the ones supporting the language come first and the generic's preference list only breaks the
/// tie. A word-processing document that names no language still has three of them — Writer resolves
/// <c>en-US</c>, <c>zh-CN</c> and <c>hi-IN</c> through
/// <c>MsLangId::resolveSystemLanguageByScriptType</c> — which is why a Hebrew character in a
/// complex-script run falls back to FreeSans here and not to DejaVu Sans. See
/// <see cref="FontLanguages"/> for what is modelled and <see cref="WriterScripts"/> for which item
/// a run's text selects.
/// </para>
/// </param>
public readonly record struct FontRequest(
    string FamilyName,
    int Weight = 400,
    bool IsItalic = false,
    FontPitch Pitch = FontPitch.Unknown,
    string? EmbeddedFaceKey = null,
    FontFamilyClass DeclaredClass = FontFamilyClass.Unknown,
    string? Language = null);

/// <summary>
/// The shape a document declares for one of the families it names, beside the name itself.
/// </summary>
/// <param name="Class">
/// The generic family, or <see cref="FontFamilyClass.Unknown"/> when the document declares none.
/// </param>
/// <param name="Pitch">The declared pitch, or <see cref="FontPitch.Unknown"/>.</param>
/// <remarks>
/// A DOCX states this in <c>word/fontTable.xml</c>, a DOC in the <c>FFN</c> that names each family,
/// and ODF in <c>office:font-face-decls</c>. All three carry the same two facts and all three are
/// dropped on the floor by a resolver that only ever sees a family name — which is what
/// <see cref="FontRequest"/> used to be handed.
/// </remarks>
public readonly record struct DeclaredFontShape(
    FontFamilyClass Class = FontFamilyClass.Unknown,
    FontPitch Pitch = FontPitch.Unknown);

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

using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;

namespace XRay.Office.Core.Graphics;

/// <summary>
/// A run of already-shaped, already-positioned glyphs from one font at one size.
/// </summary>
/// <remarks>
/// <para>
/// Shaping happens once, in <c>XRay.Office.Text</c>, during layout. By the time a run
/// reaches a backend the glyph IDs and offsets are final. Backends must draw them as
/// given rather than re-shaping the text, because layout has already committed to
/// these advances when it decided where lines break — re-shaping would produce
/// output that disagrees with the line breaks around it.
/// </para>
/// <para>
/// <see cref="Text"/> and <see cref="ClusterMap"/> exist so that a PDF backend can
/// emit a correct <c>ToUnicode</c> mapping and so extracted text stays selectable
/// and searchable. They are not used for drawing.
/// </para>
/// </remarks>
public sealed record GlyphRun
{
    /// <summary>The font to draw with.</summary>
    public required FontReference Font { get; init; }

    /// <summary>The em size the glyph positions were computed at.</summary>
    public required Length FontSize { get; init; }

    /// <summary>
    /// The run's origin: the start of the baseline, not the top-left of a box.
    /// </summary>
    public required DocPoint Origin { get; init; }

    /// <summary>The glyphs, in visual (left-to-right) order.</summary>
    public required IReadOnlyList<PositionedGlyph> Glyphs { get; init; }

    /// <summary>
    /// The source text this run came from, for text extraction and PDF
    /// <c>ToUnicode</c> mapping.
    /// </summary>
    public required string Text { get; init; }

    /// <summary>
    /// Maps each glyph in <see cref="Glyphs"/> to its starting UTF-16 index in
    /// <see cref="Text"/>. Multiple glyphs may map to one index (a decomposed
    /// character) and multiple indices to one glyph (a ligature), which is exactly
    /// why an explicit map is needed rather than a positional assumption.
    /// </summary>
    public required IReadOnlyList<int> ClusterMap { get; init; }

    /// <summary>
    /// True when the run reads right-to-left. Glyphs are still stored in visual
    /// order; this records the source directionality for extraction and for PDF
    /// structure.
    /// </summary>
    public bool IsRightToLeft { get; init; }

    /// <summary>
    /// How far each glyph is squeezed or stretched across, 1.0 for a face drawn as it is drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A font whose width is set to something other than its height: VCL's <c>Font::SetAverageFontWidth</c>,
    /// which a word processor reaches through <c>w:rPr/w:w</c> and RTF through <c>\charscalex</c>. It is
    /// <em>not</em> tracking, which puts a fixed gap between glyphs and leaves them their own shape, and it
    /// is not a font size, which would change the line's height too.
    /// </para>
    /// <para>
    /// The glyph positions and advances in <see cref="Glyphs"/> are already scaled — they are where the
    /// glyphs go on the page — so a consumer that only reads positions needs to know nothing about this.
    /// What it carries is the <em>shape</em>: at 0.5 a backend must draw each glyph half as wide about its
    /// own origin, and drawing it at its natural width would overlap the one after it.
    /// </para>
    /// <para>
    /// Both backends have the operator for it exactly: PDF's text matrix takes it as its <c>a</c> term, and
    /// Skia's <c>SKFont.ScaleX</c> is the same number.
    /// </para>
    /// </remarks>
    public double WidthScale { get; init; } = 1.0;
}

/// <summary>One glyph placed relative to a <see cref="GlyphRun"/>'s origin.</summary>
/// <param name="GlyphId">
/// The glyph index within the font. Not a character code — the mapping from
/// characters to glyphs has already been done by shaping.
/// </param>
/// <param name="Offset">
/// Position relative to the run origin. Y is usually zero but is non-zero for marks
/// attached above or below a base glyph.
/// </param>
/// <param name="Advance">
/// How far the pen moves after this glyph. Kept per-glyph rather than derived from
/// the font because justification and letter-spacing adjust it.
/// </param>
public readonly record struct PositionedGlyph(ushort GlyphId, DocPoint Offset, Length Advance);

/// <summary>
/// Identifies a resolved physical font face.
/// </summary>
/// <remarks>
/// This is the outcome of font resolution, not a request for it. The requested
/// family is kept in <see cref="RequestedFamily"/> because the substitution that
/// happened is the single biggest cause of rendering divergence between XRay.Office
/// and any reference renderer, and diagnosing that requires knowing what was asked
/// for as well as what was used.
/// </remarks>
public sealed record FontReference
{
    /// <summary>The resolved family name of the face actually used.</summary>
    public required string FamilyName { get; init; }

    /// <summary>
    /// The family name the document asked for. Differs from <see cref="FamilyName"/>
    /// when substitution occurred.
    /// </summary>
    public string? RequestedFamily { get; init; }

    /// <summary>Weight on the OpenType 1-1000 scale; 400 is regular, 700 is bold.</summary>
    public int Weight { get; init; } = 400;

    /// <summary>True for italic or oblique faces.</summary>
    public bool IsItalic { get; init; }

    /// <summary>
    /// True when italic was asked for and the resolved face has none, so the slant has to be
    /// drawn rather than chosen.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is <c>LogicalFontInstance::NeedsArtificialItalic()</c>
    /// (<c>vcl/source/font/LogicalFontInstance.cxx</c>): <em>the request is italic and the face
    /// is not</em>. It is a property of the pairing and not of either half, which is why it sits
    /// here beside <see cref="IsItalic"/> — that records what the face is, this records that the
    /// request went unmet.
    /// </para>
    /// <para>
    /// A backend honours it by shearing, and the shear is
    /// <see cref="SyntheticObliqueShear"/>. Nothing about the run's metrics changes: the
    /// reference passes the same slant to HarfBuzz as <c>hb_font_set_synthetic_slant</c>, which
    /// moves outlines and mark attachments and leaves advances alone. Measured on an authored
    /// five-family deck through 26.2.4.2 — the roman and italic halves of a `DejaVu Sans` line
    /// carry the <em>same</em> <c>TJ</c> array and the same pen origin at 12, 24 and 40 pt. So a
    /// document laid out without this and one laid out with it break their lines identically.
    /// </para>
    /// </remarks>
    public bool SyntheticOblique { get; init; }

    /// <summary>
    /// A stable key identifying the underlying face data, used to cache loaded faces
    /// and to deduplicate embedded fonts in PDF output.
    /// </summary>
    public required string FaceKey { get; init; }

    /// <summary>
    /// True when the face was substituted for an unavailable one. Rendering is still
    /// produced, but fidelity against a reference renderer is only as good as the
    /// substitute's metric compatibility.
    /// </summary>
    public bool IsSubstituted => RequestedFamily is not null && RequestedFamily != FamilyName;

    /// <summary>
    /// How far a <see cref="SyntheticOblique"/> run leans: the <c>c</c> term of its text matrix,
    /// so that a point <c>y</c> above the baseline moves right by <c>y</c> times this.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>0.3462535606, and the digits are not a rounding of a third.</strong> The reference
    /// declares <c>ARTIFICIAL_ITALIC_SKEW</c> as
    /// <c>float((1&lt;&lt;16)/3) / (1&lt;&lt;16)</c> = 0.3333333432674408
    /// (<c>vcl/inc/font/LogicalFontInstance.hxx:52-53</c>) and then hands it to
    /// <c>Matrix3::skew</c>, which takes its arguments as <em>angles</em> and writes
    /// <c>tan</c> of them (<c>vcl/source/pdf/pdfwriter_impl.cxx:5707,5767</c>). So the number
    /// that reaches the page is <c>tan(0.3333333432674408)</c>, and a shear of exactly one third
    /// would be wrong in the fourth decimal.
    /// </para>
    /// <para>
    /// It is one value for every face and every size: over the 302-document slides corpus the
    /// reference writes <strong>587</strong> sheared text matrices and every one of them reads
    /// <c>0.3462535606</c>.
    /// </para>
    /// <para>
    /// The screen path is not the same number — <c>cairotextrender.cxx:251</c> applies the raw
    /// 1/3 rather than its tangent — but the PDF is what this project is measured against.
    /// </para>
    /// </remarks>
    public const double SyntheticObliqueShear = 0.3462535606;
}

/// <summary>
/// A raster image, either decoded to pixels or still in the bytes the file stored.
/// </summary>
/// <remarks>
/// <para>
/// Pixels are 8-bit straight (non-premultiplied) RGBA in row-major order with no row
/// padding. Straight alpha matches how the office formats store transparency masks,
/// and skipping padding keeps the buffer directly usable by every backend.
/// </para>
/// <para>
/// <see cref="EncodedBytes"/> preserves the original compressed data when there was
/// any. A PDF backend can then pass a JPEG through untouched instead of
/// re-compressing it, which is both faster and lossless.
/// </para>
/// <para>
/// <strong>A reader may emit one without decoding it</strong>, through
/// <see cref="Encoded"/>: a picture in a document is a `.png` or `.jpeg` in a package
/// entry or a record, and the only thing that can turn it into pixels is a codec.
/// Decoding lives in <c>XRay.Office.Rendering</c> because that is where SkiaSharp lives,
/// and requiring pixels here would have forced every reader to depend on the rasteriser
/// — which would mean <c>xray-office extract</c> paying for a codec it never uses, against
/// the rule that extraction must not pay for rendering. So the invariant is *either*
/// <see cref="Pixels"/> or <see cref="EncodedBytes"/>, and a backend decodes what it was
/// given when it needs pixels.
/// </para>
/// </remarks>
public sealed record RasterImage
{
    /// <summary>Width in pixels, or zero when the image has not been decoded yet.</summary>
    public int Width { get; init; }

    /// <summary>Height in pixels, or zero when the image has not been decoded yet.</summary>
    public int Height { get; init; }

    /// <summary>
    /// Straight RGBA pixels, row-major, <c>Width * Height * 4</c> bytes — empty when the
    /// image is still encoded. Check <see cref="IsDecoded"/> before reading it.
    /// </summary>
    public ReadOnlyMemory<byte> Pixels { get; init; }

    /// <summary>
    /// The original encoded bytes, when the source was a compressed format that a
    /// backend may be able to pass through unmodified.
    /// </summary>
    public ReadOnlyMemory<byte> EncodedBytes { get; init; }

    /// <summary>The media type of <see cref="EncodedBytes"/>, e.g. <c>image/jpeg</c>.</summary>
    public string? EncodedMediaType { get; init; }

    /// <summary>
    /// A two-colour recolouring still to be applied, or null when the picture is drawn as
    /// stored.
    /// </summary>
    /// <remarks>
    /// A recolouring is stated by a reader and carried out by whatever decodes the picture,
    /// for the same reason the pixels are: turning a JPEG into a ramp between two colours
    /// needs a codec, and a codec is what this layer cannot have. See
    /// <see cref="DuotoneRecolour"/> for what it means.
    /// </remarks>
    public DuotoneRecolour? Duotone { get; init; }

    /// <summary>
    /// A brightness/contrast recolouring still to be applied, or null when the picture is
    /// drawn as stored.
    /// </summary>
    /// <remarks>
    /// Deferred to the decoder for the same reason <see cref="Duotone"/> is, and applied after
    /// it — which is the order the reference applies them in. See
    /// <see cref="LuminanceRecolour"/>.
    /// </remarks>
    public LuminanceRecolour? Luminance { get; init; }

    /// <summary>
    /// A colour to be knocked out of the picture, or null when nothing is.
    /// </summary>
    /// <remarks>
    /// Deferred to the decoder for the same reason <see cref="Duotone"/> is — matching a colour
    /// needs pixels, and pixels need a codec. Applied <em>before</em> the other two, because the
    /// colour it matches is the one the file stored. See <see cref="ColourKnockout"/>.
    /// </remarks>
    public ColourKnockout? Knockout { get; init; }

    /// <summary>True when <see cref="Pixels"/> holds the decoded image.</summary>
    public bool IsDecoded => !Pixels.IsEmpty;

    /// <summary>
    /// An image a reader has taken from a file but not decoded, to be decoded by whichever
    /// backend needs pixels.
    /// </summary>
    /// <param name="bytes">The bytes exactly as the file stored them.</param>
    /// <param name="mediaType">
    /// The media type the file declared, where it declared one. It is a hint only: a
    /// decoder should sniff the bytes, because office files mislabel images as routinely
    /// as they mislabel themselves.
    /// </param>
    public static RasterImage Encoded(ReadOnlyMemory<byte> bytes, string? mediaType = null)
        => new() { EncodedBytes = bytes, EncodedMediaType = mediaType };
}

/// <summary>
/// One colour of a picture made fully transparent — PowerPoint's <em>Set Transparent Color</em>.
/// </summary>
/// <remarks>
/// <para>
/// The binary formats state it as Escher property 263, <c>pictureTransparent</c>. LibreOffice
/// reads it at <c>filter/source/msfilter/msdffimp.cxx:3894-3903</c> and applies it through
/// <c>Bitmap::CombineMaskOr</c> (<c>vcl/source/bitmap/bitmap.cxx:2517</c>) into
/// <c>Bitmap::CreateAlphaMask</c> (<c>vcl/source/bitmap/bitmappaint.cxx:684</c>).
/// </para>
/// <para>
/// <b>The match is an independent per-channel box, not a distance.</b> A pixel is knocked out
/// when each of its three channels is within <see cref="Tolerance"/> of the stated colour's,
/// tested separately — so the matched region is a cube in RGB space rather than a sphere. The
/// resulting alpha is <b>binary</b>: fully transparent or fully opaque, never in between. It is
/// OR-combined with whatever alpha the picture already carried, so a knockout can only ever add
/// transparency.
/// </para>
/// <para>
/// <b>Bitmaps only.</b> The reference applies it under
/// <c>aGraf.GetType() == GraphicType::Bitmap</c>, so a WMF or EMF carrying the property gets
/// nothing — which is why this sits on <see cref="RasterImage"/> and has no vector counterpart.
/// </para>
/// </remarks>
/// <param name="Colour">The colour to knock out.</param>
/// <param name="Tolerance">
/// How far each channel may differ and still match. LibreOffice passes 9
/// (<c>CombineMaskOr(…, 9)</c>) and that number is read from source rather than measured: the one
/// corpus deck that exercises the property knocks out pure white on a palette PNG, where any
/// tolerance from 0 to 9 gives the same 51 361 pixels. Being wrong about it costs a fringe pixel
/// on an anti-aliased edge.
/// </param>
public readonly record struct ColourKnockout(Colour Colour, int Tolerance)
{
    /// <summary>The tolerance LibreOffice's own call site passes.</summary>
    public const int DefaultTolerance = 9;

    /// <summary>Whether a pixel's three channels all fall inside the box.</summary>
    /// <param name="red">The pixel's red channel.</param>
    /// <param name="green">The pixel's green channel.</param>
    /// <param name="blue">The pixel's blue channel.</param>
    public bool Matches(byte red, byte green, byte blue)
        => Math.Abs(red - Colour.R) <= Tolerance
           && Math.Abs(green - Colour.G) <= Tolerance
           && Math.Abs(blue - Colour.B) <= Tolerance;
}

/// <summary>
/// A picture mapped onto the ramp between two colours by its own brightness — DrawingML's
/// <c>a:duotone</c>.
/// </summary>
/// <remarks>
/// <para>
/// Every pixel is replaced by <c>dark + (light − dark) × luminance</c>, so the picture keeps
/// its shading and loses its hue. It is how an Office theme paints one grey texture in a
/// deck's own colours: the same background image serves every colour scheme, and the two
/// ends are usually <c>phClr</c> under different shades.
/// </para>
/// <para>
/// This is a description rather than a result. It survives into
/// <see cref="RasterImage.Duotone"/> because the transform needs decoded pixels and the
/// readers have no decoder; whichever backend decodes the picture applies it and clears the
/// field, and must also drop <see cref="RasterImage.EncodedBytes"/> when it does — otherwise
/// a PDF writer passes the original JPEG through as <c>DCTDecode</c> and the recolouring
/// never reaches the page.
/// </para>
/// </remarks>
/// <param name="Dark">The colour a black pixel becomes — <c>a:duotone</c>'s first child.</param>
/// <param name="Light">The colour a white pixel becomes — its second.</param>
public readonly record struct DuotoneRecolour(Colour Dark, Colour Light);

/// <summary>
/// A picture's brightness and contrast, as DrawingML's <c>a:blip/a:lum</c> states them and as
/// the reference resolves them.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The stated pair is not always the pair that is applied</strong>, and that is the
/// whole of why this type holds the two numbers rather than a ready-made ramp.
/// <c>GraphicProperties::pushToPropMap</c> (<c>oox/source/drawingml/fillproperties.cxx</c>:797-926,
/// at tag <c>libreoffice-24.2.7.2</c>) sorts them into three cases that use two different
/// arithmetics:
/// </para>
/// <list type="number">
/// <item><description><strong>70 and −70 exactly</strong> — PowerPoint's "Washout" — is mapped
/// to LibreOffice's own <c>ColorMode_WATERMARK</c> and the stated pair is thrown away
/// (<c>fillproperties.cxx</c>:826-831). Watermark is a fixed <em>+50 luminance and −70
/// contrast</em> (<c>WATERMARK_LUM_OFFSET</c>, <c>WATERMARK_CON_OFFSET</c>,
/// <c>vcl/source/graphic/GraphicObject.cxx</c>:53-54, applied at
/// <c>drawinglayer/source/primitive2d/graphicprimitivehelper2d.cxx</c>:758-768). So the file
/// says 70 and the picture is lightened by 50.</description></item>
/// <item><description><strong>Both non-zero otherwise</strong> is baked into the bitmap with
/// <em>MSO's</em> formula, which spends half the brightness before the contrast and half after
/// (<c>Bitmap::Adjust</c>'s <c>msoBrightness</c> branch,
/// <c>vcl/source/bitmap/bitmap.cxx</c>:1694-1698).</description></item>
/// <item><description><strong>One of the two alone</strong> becomes
/// <c>PROP_AdjustLuminance</c> or <c>PROP_AdjustContrast</c> and goes through
/// <c>BColorModifier_RGBLuminanceContrast</c>, which spends the whole brightness after the
/// contrast (<c>basegfx/source/color/bcolormodifier.cxx</c>:367-441).</description></item>
/// </list>
/// <para>
/// Measured rather than merely cited, because this checkout's source is a 27.2 alpha and the
/// binary that made the reference is 24.2.7.2: against
/// <c>N2_E_Maestroni_Swarm_COP.pptx</c>'s washed-out title slide, predicting the reference's
/// pixels from ours costs a mean absolute error of <strong>7.15 of 255</strong> under case one
/// and <strong>30.98</strong> under the stated 70/−70 through the same modifier, against 163
/// for drawing the picture untouched. The mapping is the binary's, not just the source's.
/// </para>
/// <para>
/// [24.2.7-audit: VERIFIED 2026-08-21, slides-r62 — case one still holds on 26.2.4.2, measured
/// on the same document. <c>N2_E_Maestroni_Swarm_COP.pptx</c>'s title slide renders against the
/// 26.2.4.2 reference at <c>diff% 1.71, |ink|% 0.01</c> with case one implemented, and at 100 dpi
/// its mean channel is <strong>224.02 against the reference's 223.68</strong>, MAE 2.23 over the
/// whole page and 4.75 over the middle band. The competing reading — the stated 70/−70 put
/// through the same modifier — was measured at MAE <strong>30.98</strong> when this site was
/// written, so the two readings are more than an order of magnitude apart and the page separates
/// them outright. Cases two and three are <strong>not</strong> re-checked: no corpus document
/// states a lone <c>a:lum</c> brightness or a non-washout pair on a slide, so there is nothing on
/// this track to point a probe at. The sibling claim in
/// <c>XRay.Office.Ooxml/DrawingML/DrawingFill.cs</c> — that <c>a:lum</c>'s division is integer and
/// truncating — was verified independently by the words track in round 61.]
/// </para>
/// <para>
/// Carried to the decoder rather than applied by the reader for the same reason
/// <see cref="DuotoneRecolour"/> is, and applied after it.
/// </para>
/// </remarks>
/// <param name="Brightness">Whole per cent, as stated, from −100 to 100.</param>
/// <param name="Contrast">Whole per cent, as stated, from −100 to 100.</param>
public readonly record struct LuminanceRecolour(int Brightness, int Contrast)
{
    /// <summary>True when the pair asks for nothing and the picture is drawn as stored.</summary>
    public bool IsIdentity => Brightness == 0 && Contrast == 0;

    /// <summary>True when the pair is PowerPoint's washout, which is not applied as stated.</summary>
    public bool IsWashout => Brightness == 70 && Contrast == -70;
}

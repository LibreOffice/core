using System.Collections.Frozen;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// WordArt: the text of a body carrying an <c>a:prstTxWarp</c>, drawn as warped outlines.
/// </summary>
/// <remarks>
/// <para>
/// A text body whose <c>a:bodyPr</c> states a <c>prstTxWarp</c> other than <c>textNoShape</c> is not
/// text in the reference's output. The importer puts the shape into text-path mode
/// (<c>FontworkHelpers::putCustomShapeIntoTextPathMode</c>,
/// <c>oox/source/drawingml/fontworkhelpers.cxx:75</c>; called from
/// <c>oox/source/drawingml/shape.cxx:2208</c> for a slide and
/// <c>oox/source/shape/WpsContext.cxx:989</c> for a Writer text box), and
/// <c>EnhancedCustomShapeEngine::render2</c> then <em>replaces the whole shape</em> with the object
/// <c>EnhancedCustomShapeFontWork::CreateFontWork</c> builds — filled curves carrying no glyph and
/// no <c>ToUnicode</c>, drawn with the shape's fill and its outline, and no box around them.
/// </para>
/// <para>
/// So the answer this returns is a path, and the caller draws it instead of the shape and instead
/// of the shape's text. Null means "not warped, or not warpable here" and the caller falls back to
/// whatever it does for ordinary text — see <see cref="Outline"/> for the cases.
/// </para>
/// </remarks>
public static class Fontwork
{
    /// <summary>The <c>prst</c> value that means no warp at all.</summary>
    public const string NoWarp = "textNoShape";

    /// <summary>
    /// The LibreOffice Fontwork type each OOXML <c>prst</c> maps to.
    /// </summary>
    /// <remarks>
    /// <c>oox/source/drawingml/presetgeometrynames.cxx</c>, transcribed whole. All forty warps map
    /// to a preset <see cref="FontworkPresets"/> carries, so a name that is not in this table is a
    /// malformed file rather than a gap.
    /// </remarks>
    private static readonly FrozenDictionary<string, string> FontworkTypes =
        new Dictionary<string, string>(StringComparer.Ordinal)
        {
            ["textNoShape"] = string.Empty,
            ["textPlain"] = "fontwork-plain-text",
            ["textStop"] = "fontwork-stop",
            ["textTriangle"] = "fontwork-triangle-up",
            ["textTriangleInverted"] = "fontwork-triangle-down",
            ["textChevron"] = "fontwork-chevron-up",
            ["textChevronInverted"] = "fontwork-chevron-down",
            ["textRingInside"] = "mso-spt142",
            ["textRingOutside"] = "mso-spt143",
            ["textArchUp"] = "fontwork-arch-up-curve",
            ["textArchDown"] = "fontwork-arch-down-curve",
            ["textCircle"] = "fontwork-circle-curve",
            ["textButton"] = "fontwork-open-circle-curve",
            ["textArchUpPour"] = "fontwork-arch-up-pour",
            ["textArchDownPour"] = "fontwork-arch-down-pour",
            ["textCirclePour"] = "fontwork-circle-pour",
            ["textButtonPour"] = "fontwork-open-circle-pour",
            ["textCurveUp"] = "fontwork-curve-up",
            ["textCurveDown"] = "fontwork-curve-down",
            ["textCanUp"] = "mso-spt174",
            ["textCanDown"] = "mso-spt175",
            ["textWave1"] = "fontwork-wave",
            ["textWave2"] = "mso-spt157",
            ["textDoubleWave1"] = "mso-spt158",
            ["textWave4"] = "mso-spt159",
            ["textInflate"] = "fontwork-inflate",
            ["textDeflate"] = "mso-spt161",
            ["textInflateBottom"] = "mso-spt162",
            ["textDeflateBottom"] = "mso-spt163",
            ["textInflateTop"] = "mso-spt164",
            ["textDeflateTop"] = "mso-spt165",
            ["textDeflateInflate"] = "mso-spt166",
            ["textDeflateInflateDeflate"] = "mso-spt167",
            ["textFadeRight"] = "fontwork-fade-right",
            ["textFadeLeft"] = "fontwork-fade-left",
            ["textFadeUp"] = "fontwork-fade-up",
            ["textFadeDown"] = "fontwork-fade-down",
            ["textSlantUp"] = "fontwork-slant-up",
            ["textSlantDown"] = "fontwork-slant-down",
            ["textCascadeUp"] = "fontwork-fade-up-and-right",
            ["textCascadeDown"] = "fontwork-fade-up-and-left",
        }.ToFrozenDictionary(StringComparer.Ordinal);

    /// <summary>
    /// The LibreOffice Fontwork type of each MS-ODRAW WordArt shape type, <c>mso_sptTextPlainText</c>
    /// onwards.
    /// </summary>
    /// <remarks>
    /// <c>svx/source/customshapes/EnhancedCustomShapeTypeNames.cxx:171-211</c>, in the order
    /// <c>include/svx/msdffdef.hxx:412-451</c> numbers them, which is contiguous from 136 to 175 and is
    /// why this is an array and not a dictionary. VML and Escher both name a WordArt shape by that
    /// number rather than by a DrawingML <c>prst</c>: a <c>v:shapetype</c> writes it as
    /// <c>o:spt="136"</c> and reuses it as <c>type="#_x0000_t136"</c>, and
    /// <c>oox/source/vml/vmlshape.cxx:1329</c> hands it straight to
    /// <c>SdrObjCustomShape::MergeDefaultAttributes</c>, which resolves it through the same table.
    /// </remarks>
    private static readonly string[] ShapeTypes =
    [
        "fontwork-plain-text",          // 136 mso_sptTextPlainText
        "fontwork-stop",                // 137 mso_sptTextStop
        "fontwork-triangle-up",         // 138 mso_sptTextTriangle
        "fontwork-triangle-down",       // 139 mso_sptTextTriangleInverted
        "fontwork-chevron-up",          // 140 mso_sptTextChevron
        "fontwork-chevron-down",        // 141 mso_sptTextChevronInverted
        "mso-spt142",                   // 142 mso_sptTextRingInside
        "mso-spt143",                   // 143 mso_sptTextRingOutside
        "fontwork-arch-up-curve",       // 144 mso_sptTextArchUpCurve
        "fontwork-arch-down-curve",     // 145 mso_sptTextArchDownCurve
        "fontwork-circle-curve",        // 146 mso_sptTextCircleCurve
        "fontwork-open-circle-curve",   // 147 mso_sptTextButtonCurve
        "fontwork-arch-up-pour",        // 148 mso_sptTextArchUpPour
        "fontwork-arch-down-pour",      // 149 mso_sptTextArchDownPour
        "fontwork-circle-pour",         // 150 mso_sptTextCirclePour
        "fontwork-open-circle-pour",    // 151 mso_sptTextButtonPour
        "fontwork-curve-up",            // 152 mso_sptTextCurveUp
        "fontwork-curve-down",          // 153 mso_sptTextCurveDown
        "fontwork-fade-up-and-right",   // 154 mso_sptTextCascadeUp
        "fontwork-fade-up-and-left",    // 155 mso_sptTextCascadeDown
        "fontwork-wave",                // 156 mso_sptTextWave1
        "mso-spt157",                   // 157 mso_sptTextWave2
        "mso-spt158",                   // 158 mso_sptTextWave3
        "mso-spt159",                   // 159 mso_sptTextWave4
        "fontwork-inflate",             // 160 mso_sptTextInflate
        "mso-spt161",                   // 161 mso_sptTextDeflate
        "mso-spt162",                   // 162 mso_sptTextInflateBottom
        "mso-spt163",                   // 163 mso_sptTextDeflateBottom
        "mso-spt164",                   // 164 mso_sptTextInflateTop
        "mso-spt165",                   // 165 mso_sptTextDeflateTop
        "mso-spt166",                   // 166 mso_sptTextDeflateInflate
        "mso-spt167",                   // 167 mso_sptTextDeflateInflateDeflate
        "fontwork-fade-right",          // 168 mso_sptTextFadeRight
        "fontwork-fade-left",           // 169 mso_sptTextFadeLeft
        "fontwork-fade-up",             // 170 mso_sptTextFadeUp
        "fontwork-fade-down",           // 171 mso_sptTextFadeDown
        "fontwork-slant-up",            // 172 mso_sptTextSlantUp
        "fontwork-slant-down",          // 173 mso_sptTextSlantDown
        "mso-spt174",                   // 174 mso_sptTextCanUp
        "mso-spt175",                   // 175 mso_sptTextCanDown
    ];

    /// <summary>The lowest MS-ODRAW shape type that is a WordArt shape.</summary>
    public const int FirstShapeType = 136;

    /// <summary>
    /// The four warps whose text keeps its stated size instead of filling the shape.
    /// </summary>
    /// <remarks>
    /// <c>fontworkhelpers.cxx:173-179</c>. It is conditional on the shape <em>not</em> having come
    /// from a binary WordArt object, which for the two importers X-Ray.Office reads means: a Writer
    /// text box always takes it (<c>WpsContext.cxx:989</c> passes <c>bFromWordArt</c> false), a
    /// slide shape takes it unless the file marks the shape <c>PROP_FromWordArt</c>.
    /// </remarks>
    private static readonly FrozenSet<string> KeepsFontSize =
        new[] { "textArchDown", "textArchUp", "textCircle", "textButton" }
            .ToFrozenSet(StringComparer.Ordinal);

    /// <summary>Whether a <c>prst</c> value asks for a warp at all.</summary>
    /// <remarks>
    /// The test is "not <c>textNoShape</c>", not "present": <c>textNoShape</c> is overwhelmingly the
    /// common value and means the identity. <c>textPlain</c> curves nothing either but <em>is</em> a
    /// warp — the reference puts its shape into text-path mode and draws outlines, which is why its
    /// words are absent from the reference's text layer too.
    /// </remarks>
    public static bool IsWarp(string? preset)
        => !string.IsNullOrEmpty(preset) && !string.Equals(preset, NoWarp, StringComparison.Ordinal);

    /// <summary>The LibreOffice Fontwork type a <c>prst</c> names, or null when it names none.</summary>
    public static string? FontworkTypeOf(string? preset)
        => preset is not null && FontworkTypes.TryGetValue(preset, out string? type) && type.Length > 0
            ? type
            : null;

    /// <summary>
    /// The LibreOffice Fontwork type an MS-ODRAW shape type number names, or null when it is not one.
    /// </summary>
    /// <param name="shapeType">
    /// A <c>v:shapetype/@o:spt</c>, or the number in a <c>#_x0000_t136</c> reference, which is the
    /// same thing written twice.
    /// </param>
    public static string? FontworkTypeOfShapeType(int shapeType)
        => shapeType >= FirstShapeType && shapeType < FirstShapeType + ShapeTypes.Length
            ? ShapeTypes[shapeType - FirstShapeType]
            : null;

    /// <summary>
    /// The warped outlines of a body's text, in the shape's own coordinates, or null.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Null has four causes: the body states no warp; it states a <c>prst</c> that is not one of
    /// the forty (a malformed file, since <see cref="FontworkPresets"/> carries them all); the face
    /// has no <c>glyf</c> outlines (see <see cref="GlyphOutlines"/>); or the text is empty. The
    /// first means "draw it the ordinary way"; the other three mean "the reference drew curves and
    /// this cannot", and both families answer that by drawing nothing rather than by drawing
    /// unwarped text.
    /// </para>
    /// <para>
    /// The result's origin is the shape's top-left corner and its coordinates are EMUs, so a caller
    /// places it by translating. It is a fill path — the reference fills it and strokes the same
    /// geometry with the shape's pen — and it is one path holding every character of every line.
    /// </para>
    /// </remarks>
    /// <param name="request">The body, the box it sits in, and the face its runs are set in.</param>
    public static GraphicsPath? Outline(FontworkRequest request)
    {
        ArgumentNullException.ThrowIfNull(request);

        if (request.FontworkType is null && !IsWarp(request.Preset)) return null;
        if ((request.FontworkType ?? FontworkTypeOf(request.Preset)) is not { } type) return null;
        if (FontworkPresets.Find(type) is not { } preset) return null;
        if (!GlyphOutlines.CanOutline(request.Face)) return null;

        // Blank lines are dropped rather than laid out. The reference keeps them — an empty
        // paragraph contributes no outline but still grows the text area by one line height
        // (`EnhancedCustomShapeFontWork.cxx:576-583`) — so a body whose lines are separated by a
        // blank one is spaced differently here. No warped body in the corpus has one, and the
        // difference is invisible on the single-line bodies that do exist.
        List<string> lines = [];
        foreach (string line in request.Lines)
        {
            if (!string.IsNullOrEmpty(line)) lines.Add(line);
        }

        if (lines.Count == 0) return null;
        if (request.Box.IsEmpty) return null;

        bool scalesX = request.KeepsFontSize
            ?? (!request.FromWordArt && request.Preset is not null && KeepsFontSize.Contains(request.Preset));

        return FontworkFitting.Fit(
            preset,
            request.AdjustmentValues ?? Adjustments(type, request.Adjustments),
            request.Box,
            lines,
            request.Face,
            request.FontSize,
            request.Alignment,
            VerticalAdjustOfType(type),
            scalesX);
    }

    /// <summary>Where a multi-line warp's lines gather, which the preset alone decides.</summary>
    /// <remarks><c>oox/source/drawingml/shape.cxx:863-874</c>.</remarks>
    public static FontworkVerticalAdjust VerticalAdjustOf(string? preset)
        => VerticalAdjustOfType(FontworkTypeOf(preset));

    /// <summary>The same, keyed on the LibreOffice Fontwork type rather than on a <c>prst</c>.</summary>
    private static FontworkVerticalAdjust VerticalAdjustOfType(string? type) => type switch
    {
        "fontwork-arch-up-curve" or "fontwork-circle-curve" => FontworkVerticalAdjust.Bottom,
        "fontwork-arch-down-curve" => FontworkVerticalAdjust.Top,
        _ => FontworkVerticalAdjust.Centre,
    };

    /// <summary>
    /// The <c>a:prstTxWarp/a:avLst</c> guides, converted from DrawingML units to WordArt's.
    /// </summary>
    /// <remarks>
    /// <c>fontworkhelpers.cxx:95-150</c>, and the conversion is not one factor. An angle handle —
    /// the arch and circle family's <c>adj</c>/<c>adj1</c> — is 1/60000 of a degree in DrawingML and
    /// a plain degree in WordArt. Everything else is a percentage against 100000 in DrawingML and an
    /// absolute value in a 21600 viewbox in WordArt, so it scales by 0.216 — except a wave's
    /// <c>adj2</c>, which states an offset from the horizontal centre rather than a position, and a
    /// pour shape's <c>gdRefR</c>, which is relative to a radius and takes half.
    /// </remarks>
    private static double[] Adjustments(string type, IReadOnlyList<FontworkAdjustment> stated)
    {
        double[] values = new double[stated.Count];

        for (int i = 0; i < stated.Count; i++)
        {
            (string name, double value) = stated[i];

            bool polar =
                type is "fontwork-arch-down-curve" or "fontwork-arch-up-curve"
                     or "fontwork-open-circle-curve" or "fontwork-circle-curve"
                || (name == "adj1" && type is "fontwork-arch-down-pour" or "fontwork-arch-up-pour"
                        or "fontwork-open-circle-pour" or "fontwork-circle-pour");

            if (polar)
            {
                // Only the sine and cosine of it are ever used, so the range does not matter; this
                // is `NormAngle360` on the degree the DrawingML value states in 1/60000ths.
                double degrees = value / 60000.0 % 360.0;
                values[i] = degrees < 0 ? degrees + 360.0 : degrees;
                continue;
            }

            bool waveOffset = name == "adj2"
                && type is "mso-spt158" or "fontwork-wave" or "mso-spt157" or "mso-spt159";
            bool pourRadius = name == "adj2"
                && type is "fontwork-arch-down-pour" or "fontwork-arch-up-pour"
                        or "fontwork-open-circle-pour" or "fontwork-circle-pour";

            values[i] = waveOffset ? (value + 50000.0) * 0.216
                : pourRadius ? value * 0.108
                : value * 0.216;
        }

        return values;
    }
}

/// <summary>One <c>a:prstTxWarp/a:avLst/a:gd</c>: its name and the number its formula states.</summary>
/// <param name="Name">The guide's name, <c>adj</c>, <c>adj1</c> or <c>adj2</c>.</param>
/// <param name="Value">
/// The number from its <c>fmla="val n"</c>, in DrawingML units — 1/60000 of a degree for an angle
/// handle and 1/100000 of the shape for everything else.
/// </param>
public readonly record struct FontworkAdjustment(string Name, double Value);

/// <summary>
/// Which end of a Fontwork's curve its lines gather toward when there is more than one.
/// </summary>
/// <remarks>
/// Fontwork does not read the shape's text anchor the way a text box does; the importer sets one
/// per preset so that a multi-line "follow path" warp sits where MS Office puts it —
/// <c>oox/source/drawingml/shape.cxx:863-874</c>, which is bottom for <c>textArchUp</c> and
/// <c>textCircle</c>, top for <c>textArchDown</c>, and centre for every other preset. A
/// single-line body is unaffected: the offset is a multiple of the number of lines less one.
/// </remarks>
public enum FontworkVerticalAdjust
{
    /// <summary>The lines straddle the curve, which is every preset but the arch family.</summary>
    Centre,

    /// <summary>They gather above it.</summary>
    Top,

    /// <summary>They gather below it.</summary>
    Bottom,
}

/// <summary>Where a Fontwork's text sits along its path.</summary>
/// <remarks>
/// Fontwork ignores paragraph alignment and reads the shape's text anchor instead, so the importer
/// converts one into the other (<c>WpsContext.cxx:465-488</c>). Centre is the default it starts
/// from.
/// </remarks>
public enum FontworkAlignment
{
    /// <summary>Against the start of the path.</summary>
    Left,

    /// <summary>Centred along it.</summary>
    Centre,

    /// <summary>Against the end of it.</summary>
    Right,
}

/// <summary>Everything a warp needs to be laid out.</summary>
public sealed record FontworkRequest
{
    /// <summary>The <c>a:prstTxWarp/@prst</c> value, or null when the shape states no DrawingML warp.</summary>
    /// <remarks>
    /// VML and Escher name a WordArt shape by an MS-ODRAW shape type number instead, so a caller on
    /// either of those paths leaves this null and sets <see cref="FontworkType"/>.
    /// </remarks>
    public string? Preset { get; init; }

    /// <summary>
    /// The LibreOffice Fontwork type, when the caller already has one, or null to derive it from
    /// <see cref="Preset"/>.
    /// </summary>
    /// <remarks>
    /// <see cref="Fontwork.FontworkTypeOfShapeType"/> is where a VML or Escher caller gets it. Setting
    /// it also bypasses the <c>textNoShape</c> gate, which is a DrawingML spelling: a shape type
    /// number in the WordArt range is a warp by construction.
    /// </remarks>
    public string? FontworkType { get; init; }

    /// <summary>Its <c>a:avLst</c> guides, in the order the file states them.</summary>
    public IReadOnlyList<FontworkAdjustment> Adjustments { get; init; } = [];

    /// <summary>
    /// The adjustment values already in MS-ODRAW's units, or null to convert
    /// <see cref="Adjustments"/> from DrawingML's.
    /// </summary>
    /// <remarks>
    /// A <c>v:shape/@adj</c> is a comma-separated list of values in the shape's own 21600 viewbox —
    /// the units the preset tables are written in — so the VML path has nothing to convert and the
    /// conversion <see cref="Fontwork"/> applies to a DrawingML <c>a:gd</c> would be wrong twice over.
    /// </remarks>
    public IReadOnlyList<double>? AdjustmentValues { get; init; }

    /// <summary>
    /// Whether the text keeps its stated size, or null to derive it from the preset.
    /// </summary>
    /// <remarks>
    /// The <c>TextPath/ScaleX</c> property, which <c>EnhancedCustomShapeFontWork.cxx:107-111</c> reads
    /// as <c>FWData::bScaleX</c>. DrawingML derives it from the preset and from
    /// <see cref="FromWordArt"/>; VML does not derive it at all —
    /// <c>oox/source/vml/vmlformatting.cxx:966-975</c> writes <c>ScaleX</c> and
    /// <c>SameLetterHeights</c> as literal <c>false</c> for every <c>v:textpath</c>, whatever the
    /// shape type — so the VML adapter states it here rather than letting the arch family derive
    /// a <c>true</c> the reference never sets.
    /// </remarks>
    public bool? KeepsFontSize { get; init; }

    /// <summary>
    /// Whether the shape came from a binary WordArt object rather than from a text box.
    /// </summary>
    /// <remarks>
    /// It decides only whether the arch family keeps its font size; see
    /// <c>fontworkhelpers.cxx:173-179</c>. False for a Writer text box always, and for a slide
    /// shape unless the file marks it <c>fromWordArt</c>.
    /// </remarks>
    public bool FromWordArt { get; init; }

    /// <summary>The shape's rectangle, which the warp is fitted into.</summary>
    public required DocSize Box { get; init; }

    /// <summary>The lines of text, one per paragraph.</summary>
    public required IReadOnlyList<string> Lines { get; init; }

    /// <summary>The face the text is set in.</summary>
    /// <remarks>
    /// One face for the whole body, because Fontwork is one: the reference reads
    /// <c>EE_CHAR_FONTINFO</c> off the <em>shape</em> and cannot style a portion of the text
    /// separately (<c>EnhancedCustomShapeFontWork.cxx:210-227</c>).
    /// </remarks>
    public required OpenTypeFace Face { get; init; }

    /// <summary>The stated font size.</summary>
    /// <remarks>
    /// Used only by the four presets that keep it. For every other warp the text is scaled to fill
    /// the shape and the size the document states has no effect at all, which is why the
    /// reference's WordArt is so much larger than the run it came from.
    /// </remarks>
    public required Length FontSize { get; init; }

    /// <summary>Where the text sits along the path.</summary>
    public FontworkAlignment Alignment { get; init; } = FontworkAlignment.Centre;
}

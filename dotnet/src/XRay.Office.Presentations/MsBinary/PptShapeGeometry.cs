using XRay.Office.Core.Geometry;

namespace XRay.Office.Presentations.MsBinary;

/// <summary>
/// Maps the drawing layer's built-in shape types onto the preset geometry the slide layouter
/// already expands, and answers the two questions a shape's own record does not.
/// </summary>
/// <remarks>
/// <para>
/// The binary format names a shape by a number (<c>MSO_SPT</c>,
/// <c>include/svx/msdffdef.hxx:276</c>) where DrawingML names it by a string, and the two
/// vocabularies describe the <em>same</em> two hundred shapes — LibreOffice runs both through one
/// table (<c>svx/source/customshapes/EnhancedCustomShapeGeometry.cxx</c>). So the bridge is a
/// lookup rather than a second geometry engine, and it covers every type naming a preset the
/// DrawingML evaluator knows; everything else falls back to its bounding rectangle, which is where
/// the shape is, in the right colour, with the wrong outline.
/// </para>
/// <para>
/// <strong>An adjustment value is in a different unit in each vocabulary.</strong> The binary
/// form states it in a 21600-unit view box; DrawingML states it in hundred-thousandths. Feeding
/// one to the other unconverted makes a rounded rectangle either perfectly square or a stadium,
/// with nothing in between to notice.
/// </para>
/// <para>
/// <strong>And the conversion is only sound where the two forms measure the same thing.</strong>
/// A scale factor is not a translation: DrawingML's adjustments per preset are defined against
/// that preset's own guides, and the binary vocabulary's are defined against
/// <c>EnhancedCustomShapeGeometry</c>'s handles for the corresponding <c>MSO_SPT</c>, which for
/// most shapes are neither the same quantity nor the same count. So a converted value is passed
/// only for the presets where the two definitions were <em>measured</em> to coincide, and every
/// other preset is drawn at its stated defaults — a right arrow with a default head is right in
/// outline and slightly wrong in proportion, where a right arrow fed a foreign adjustment is
/// neither.
/// </para>
/// <para>
/// <strong>Which those are is a measurement, not a reading.</strong> Round 127 expanded all 148
/// name-mapped types twice — once by 26.2.4.2 itself, from a flat ODF naming each type and no
/// path, and once here — at three in-range values of <c>adjustValue</c> and at two aspect ratios,
/// and compared the drawn coordinates. <see cref="AdjustmentInViewBox"/> is the set where the
/// plain rescale reproduced the reference at every one of those six points;
/// <see cref="MirrorsVertically"/> is the one preset whose two definitions are reflections of
/// each other. <c>probes/pptgeom-r127/adjust-conversion.tsv</c> and <c>preset-census.tsv</c>.
/// </para>
/// </remarks>
internal static class PptShapeGeometry
{
    /// <summary>The property holding a preset's first adjustment handle.</summary>
    public const ushort AdjustValue = 327;

    /// <summary>The property naming the kind of fill.</summary>
    public const ushort FillType = 384;

    /// <summary>
    /// Whether the shape resizes itself around its text — <c>DFF_Prop_FitTextToShape</c>.
    /// </summary>
    /// <remarks>
    /// A bit field rather than a boolean, and only bit 1 — <c>fFitShapeToText</c>, value 2 — is
    /// the one the PowerPoint import reads (<c>svdfppt.cxx:1051</c>). Bit 0 is
    /// <c>fFitTextToShape</c>, which the drawing layer ignores.
    /// </remarks>
    public const ushort FitTextToShape = 191;

    /// <summary>The <see cref="FitTextToShape"/> bit meaning "grow the shape to its text".</summary>
    public const uint FitShapeToText = 2;

    /// <summary>
    /// The property a group shape carries when it is really a table —
    /// <c>DFF_Prop_tableProperties</c>.
    /// </summary>
    /// <remarks>
    /// It lives in the <em>tertiary</em> property table, <c>msofbtUDefProp</c>, and is read only
    /// off a shape that holds no text of its own — a group's descriptor
    /// (<c>filter/source/msfilter/svdfppt.cxx</c>:1202-1240). A low bit set is what the reference
    /// tests (<c>nTableProperties &amp; 3</c>); it then wants
    /// <see cref="TableRowProperties"/> beside it, and takes the group for a table only when both
    /// arrive.
    /// </remarks>
    public const ushort TableProperties = 927;

    /// <summary>The row heights beside <see cref="TableProperties"/>, as a complex value.</summary>
    /// <remarks>
    /// Six bytes of counts and then one <c>sal_uInt32</c> per row. The heights themselves are not
    /// read here: what the group's members are drawn in is their own anchors, and the array only
    /// has to be <em>present</em> for the reference to build a table.
    /// </remarks>
    public const ushort TableRowProperties = 928;

    /// <summary>
    /// <c>mso_sptLine</c>, the only shape type the reference turns into a two-point
    /// <c>SdrPathObj</c> and therefore the only one a table group counts as a rule.
    /// </summary>
    /// <remarks>
    /// <c>SvxMSDffManager::ImportShape</c> (<c>filter/source/msfilter/msdffimp.cxx</c>:4403-4412)
    /// builds an <c>SdrObjKind::Line</c> from the shape's own bound rectangle for this type and
    /// this type alone, unless it is extruded; <c>IsLine</c> (<c>svdfppt.cxx</c>:7183) then asks
    /// for exactly such an object with two points, and <c>CreateTable</c> sends every member that
    /// answers yes to <c>ApplyCellLineAttributes</c> and every other member to a cell.
    /// </remarks>
    public const ushort LineShape = 20;

    /// <summary>
    /// <c>DFF_Prop_fc3DLightFace</c>'s boolean word; bit 3 is <c>f3D</c>, and an extruded line is
    /// not imported as a line at all (<c>msdffimp.cxx</c>:4403).
    /// </summary>
    public const ushort ThreeDimensionalFlags = 703;

    /// <summary>The <see cref="ThreeDimensionalFlags"/> bit that makes a shape extruded.</summary>
    public const uint Extruded = 8;

    /// <summary>How lines are joined; the property's own default is a mitre.</summary>
    public const ushort LineJoin = 470;

    /// <summary>How lines are ended.</summary>
    public const ushort LineEndCap = 471;

    /// <summary>A solid fill, which is the only kind resolved.</summary>
    public const uint SolidFill = 0;

    /// <summary>A mitred join, the drawing layer's default for everything but an arc.</summary>
    public const uint MiterJoin = 1;

    /// <summary>The wrap mode meaning "do not wrap", so a line runs past the shape.</summary>
    public const uint WrapNone = 2;

    /// <summary>The view box a binary adjustment value is measured in.</summary>
    private const int AdjustmentViewBox = 21600;

    /// <summary>The view box DrawingML measures an adjustment in.</summary>
    private const int DrawingMlViewBox = 100000;

    /// <summary>
    /// Which shape types are <em>not</em> filled unless the shape says so.
    /// </summary>
    /// <remarks>
    /// <c>mso_DefaultFillingTable</c>, <c>EnhancedCustomShapeGeometry.cxx:6156</c>: one word per
    /// sixteen types, a set bit meaning "not filled by default". Arcs, lines and the whole
    /// bracket/brace family are in it; a plain rectangle and a text box are not, which is why a
    /// text box that never mentions a fill still gets one.
    /// </remarks>
    private static ReadOnlySpan<ushort> UnfilledByDefault =>
    [
        0x0000, 0x0018, 0x01FF, 0x0000, 0x0C00, 0x01E0, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0600, 0x0000, 0x0000, 0x0000, 0x0000,
    ];

    /// <summary>
    /// Which shape types are <em>not</em> stroked unless the shape says so.
    /// </summary>
    /// <remarks>
    /// <c>mso_DefaultStrokingTable</c>, <c>EnhancedCustomShapeGeometry.cxx:6198</c>. Exactly one
    /// entry: a picture frame, which would otherwise get a black box round every image.
    /// </remarks>
    private static ReadOnlySpan<ushort> UnstrokedByDefault =>
    [
        0x0000, 0x0000, 0x0000, 0x0000, 0x0800, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    ];

    /// <summary>
    /// The DrawingML preset name a shape type expands as, or null for its bounding rectangle.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Transcribed from <c>GETVMLShapeType</c>'s table
    /// (<c>filter/source/msfilter/util.cxx</c>:1072-1290), which is the one place LibreOffice
    /// states the correspondence in the direction this needs it, joined to the numbering in
    /// <c>MSO_SPT</c> (<c>include/svx/msdffdef.hxx</c>:274). Only the 148 entries naming a preset
    /// <see cref="XRay.Office.Ooxml.DrawingML.PresetShapeGeometry"/> actually knows are kept, so a
    /// name here always resolves.
    /// </para>
    /// <para>
    /// <strong>The absentees are absent on purpose.</strong> Type 0 is <c>mso_sptNotPrimitive</c>,
    /// which is what a group and a freeform both carry — it has no outline of its own and its path
    /// comes from <see cref="PptCustomGeometry"/>. Types 24-31 and 136-175 are the WordArt
    /// vocabulary, whose "geometry" is a path text is bent along rather than a shape. And a
    /// picture frame (75), a text box (202) and a host control (201) are their bounding rectangle
    /// already, so naming <c>rect</c> for them would evaluate a preset to reach the fallback.
    /// </para>
    /// <para>
    /// The gain is not the six shapes that were transcribed by hand growing to a hundred and
    /// forty-eight; it is that the binary path stops being a second, much smaller shape
    /// vocabulary. Every preset the DrawingML reader can draw is now reachable from a
    /// <c>.ppt</c> — which is what the shared evaluator was for.
    /// </para>
    /// </remarks>
    /// <param name="shapeType">The <c>msofbtSp</c> record's instance.</param>
    public static string? PresetOf(ushort shapeType) => shapeType switch
    {
        2 => "roundRect",
        3 => "ellipse",
        4 => "diamond",
        5 => "triangle",
        6 => "rtTriangle",
        7 => "parallelogram",
        8 => "trapezoid",
        9 => "hexagon",
        10 => "octagon",
        11 => "plus",
        12 => "star5",
        13 => "rightArrow",
        15 => "homePlate",
        16 => "cube",
        17 => "wedgeRoundRectCallout",
        18 => "star16",
        19 => "arc",
        20 => "line",
        21 => "plaque",
        22 => "can",
        23 => "donut",
        32 => "straightConnector1",
        33 => "bentConnector2",
        34 => "bentConnector3",
        35 => "bentConnector4",
        36 => "bentConnector5",
        37 => "curvedConnector2",
        38 => "curvedConnector3",
        39 => "curvedConnector4",
        40 => "curvedConnector5",
        41 => "callout1",
        42 => "callout2",
        43 => "callout3",
        44 => "accentCallout1",
        45 => "accentCallout2",
        46 => "accentCallout3",
        47 => "borderCallout1",
        48 => "borderCallout2",
        49 => "borderCallout3",
        50 => "accentBorderCallout1",
        51 => "accentBorderCallout2",
        52 => "accentBorderCallout3",
        53 => "ribbon",
        54 => "ribbon2",
        55 => "chevron",
        56 => "pentagon",
        57 => "noSmoking",
        58 => "star8",
        59 => "star16",
        60 => "star32",
        61 => "wedgeRectCallout",
        62 => "wedgeRoundRectCallout",
        63 => "wedgeEllipseCallout",
        64 => "wave",
        65 => "foldedCorner",
        66 => "leftArrow",
        67 => "downArrow",
        68 => "upArrow",
        69 => "leftRightArrow",
        70 => "upDownArrow",
        71 => "irregularSeal1",
        72 => "irregularSeal2",
        73 => "lightningBolt",
        74 => "heart",
        76 => "quadArrow",
        77 => "leftArrowCallout",
        78 => "rightArrowCallout",
        79 => "upArrowCallout",
        80 => "downArrowCallout",
        81 => "leftRightArrowCallout",
        82 => "upDownArrowCallout",
        83 => "quadArrowCallout",
        84 => "bevel",
        85 => "leftBracket",
        86 => "rightBracket",
        87 => "leftBrace",
        88 => "rightBrace",
        89 => "leftUpArrow",
        90 => "bentUpArrow",
        91 => "bentArrow",
        92 => "star24",
        93 => "stripedRightArrow",
        94 => "notchedRightArrow",
        95 => "blockArc",
        96 => "smileyFace",
        97 => "verticalScroll",
        98 => "horizontalScroll",
        99 => "circularArrow",
        101 => "uturnArrow",
        102 => "curvedRightArrow",
        103 => "curvedLeftArrow",
        104 => "curvedUpArrow",
        105 => "curvedDownArrow",
        106 => "cloudCallout",
        107 => "ellipseRibbon",
        108 => "ellipseRibbon2",
        109 => "flowChartProcess",
        110 => "flowChartDecision",
        111 => "flowChartInputOutput",
        112 => "flowChartPredefinedProcess",
        113 => "flowChartInternalStorage",
        114 => "flowChartDocument",
        115 => "flowChartMultidocument",
        116 => "flowChartTerminator",
        117 => "flowChartPreparation",
        118 => "flowChartManualInput",
        119 => "flowChartManualOperation",
        120 => "flowChartConnector",
        121 => "flowChartPunchedCard",
        122 => "flowChartPunchedTape",
        123 => "flowChartSummingJunction",
        124 => "flowChartOr",
        125 => "flowChartCollate",
        126 => "flowChartSort",
        127 => "flowChartExtract",
        128 => "flowChartMerge",
        129 => "flowChartOfflineStorage",
        130 => "flowChartOnlineStorage",
        131 => "flowChartMagneticTape",
        132 => "flowChartMagneticDisk",
        133 => "flowChartMagneticDrum",
        134 => "flowChartDisplay",
        135 => "flowChartDelay",
        176 => "flowChartAlternateProcess",
        177 => "flowChartOffpageConnector",
        178 => "callout1",
        179 => "accentCallout1",
        180 => "borderCallout1",
        181 => "accentBorderCallout1",
        182 => "leftRightUpArrow",
        183 => "sun",
        184 => "moon",
        185 => "bracketPair",
        186 => "bracePair",
        187 => "star4",
        188 => "doubleWave",
        189 => "actionButtonBlank",
        190 => "actionButtonHome",
        191 => "actionButtonHelp",
        192 => "actionButtonInformation",
        193 => "actionButtonForwardNext",
        194 => "actionButtonBackPrevious",
        195 => "actionButtonEnd",
        196 => "actionButtonBeginning",
        197 => "actionButtonReturn",
        198 => "actionButtonDocument",
        199 => "actionButtonSound",
        200 => "actionButtonMovie",
        _ => null,
    };

    /// <summary>
    /// The shape types whose stated <c>adjustValue</c> means, to the reference's own drawn
    /// coordinates, exactly what the same fraction means to the DrawingML preset of that name.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Measured rather than argued. Each type was drawn by 26.2.4.2 from a flat ODF naming it and
    /// stating only <c>draw:modifiers</c>, at <c>adjustValue</c> 2000, 5000 and 9000 — inside the
    /// 0-10800 range a one-handle Escher preset's own handle declares — and in a square box and a
    /// 2:1 box, the pair that separates a guide measured across the width from one measured across
    /// <c>ss = min(w, h)</c>. A type is here only when this tree's preset, fed
    /// <c>value x 100000/21600</c>, reproduced the reference's own drawn coordinates at all six
    /// points, to within 1 % of the box across the whole outline.
    /// </para>
    /// <para>
    /// <strong>A blanket rescale would be worse than discarding the value.</strong> Applied to all
    /// 105 name-mapped types that have an Escher adjustment at all, it reproduces the reference on
    /// a minority and breaks the rest, because most presets' two handles are not the same quantity:
    /// a star's is an inner radius in one and a fraction of the box in the other, a callout's four
    /// pairs are stated (x, y) in Escher and (y, x) in DrawingML, and a trapezoid's is measured
    /// across a different edge — see <see cref="AdjustmentAcrossWidth"/>.
    /// </para>
    /// <para>
    /// <strong>The connectors pass that test and are still not here, because a <c>.ppt</c>
    /// connector is not drawn from its preset at all.</strong> For every type from
    /// <c>mso_sptStraightConnector1</c> to <c>mso_sptCurvedConnector5</c> — 32 to 40 —
    /// <c>SvxMSDffManager::ImportShape</c> (<c>filter/source/msfilter/msdffimp.cxx</c>:4391,
    /// 4792-4888) builds the custom shape, takes its line geometry, and then <em>throws the object
    /// away</em>: what is drawn is an <c>SdrEdgeObj</c> whose two ends are reset to the bounding
    /// rectangle's corners and whose kind comes from <c>DFF_Prop_cxstyle</c>. Feeding the preset
    /// the stated adjustment therefore moves a bend the reference routes for itself. Measured on
    /// the three corpus decks holding 57 such shapes: the mean interior-vertex residual against
    /// 26.2.4.2 went from 6.63 pt to 8.73 pt and the count agreeing within a point did not move
    /// (6 of 11 paired polylines), so the arm is declined rather than taken. Seated as O76.
    /// <c>probes/pptgeom-r127/connscore.py</c>.
    /// </para>
    /// </remarks>
    /// <param name="shapeType">The <c>msofbtSp</c> record's instance.</param>
    private static bool AdjustmentInViewBox(ushort shapeType) => shapeType switch
    {
        2 or 5 or 10 or 11 or 16 or 21 or 22 => true,     // roundRect, triangle, octagon .. can
        84 or 85 or 86 or 184 or 185 => true,             // bevel, the brackets, moon, bracketPair
        _ => false,
    };

    /// <summary>
    /// The shape types whose <c>adjustValue</c> Escher measures across the shape's <em>width</em>
    /// where the DrawingML preset of that name measures across <c>ss = min(w, h)</c>.
    /// </summary>
    /// <remarks>
    /// <c>mso_sptTrapezoidCalc</c> insets the narrow edge from <c>adj</c> to <c>21600 - adj</c>
    /// across the shape's own 21600-unit width; <c>trapezoid</c>'s <c>x2</c> is
    /// <c>ss x a/100000</c>. So the value needs the extra factor <c>w/ss</c>, and a plain rescale
    /// is right only where the shape happens to be square or taller than it is wide — which is why
    /// these three read as correct in a square box and wrong in a wide one. With the factor they
    /// are right in all three aspect ratios measured: 1:1, 2:1 and 1:2, at three values each.
    /// <c>probes/pptgeom-r127/acrosswidth.py</c>.
    /// </remarks>
    /// <param name="shapeType">The <c>msofbtSp</c> record's instance.</param>
    private static bool AdjustmentAcrossWidth(ushort shapeType) => shapeType is 7 or 8 or 9;

    /// <summary>
    /// The shape types whose Escher definition is a vertical reflection of the DrawingML preset
    /// this maps them to, so the expanded outline has to be turned over before it is placed.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>mso_sptTrapezoidVert</c> (<c>EnhancedCustomShapeGeometry.cxx</c>:345-348) is
    /// <c>{0,0} {21600,0} {21600-adj,21600} {adj,21600}</c> — full width at the shape's
    /// <em>top</em>. <c>trapezoid</c> in <c>PresetShapeGeometry.txt</c> is
    /// <c>m l b / l x2 t / l x3 t / l r b</c> — full width at the <em>bottom</em>. Mapping the two
    /// by name therefore draws the shape upside down whatever <c>fFlipV</c> says, because the flag
    /// is applied to the wrong base.
    /// </para>
    /// <para>
    /// <strong>And it is the only one.</strong> All 148 name-mapped types were expanded by
    /// 26.2.4.2 itself and here, in a square box, and compared under the identity and under all
    /// three reflections: 92 draw the same shape, 50 differ in some way no reflection accounts for,
    /// one has no Escher definition at all, and five fit a reflection. Four of the five are the
    /// <c>callout3</c> family, whose tail is drawn entirely from adjustment values the two
    /// vocabularies default to opposite sides of the box — their path templates are identical —
    /// leaving the trapezoid as the only reflected template.
    /// <c>probes/pptgeom-r127/preset-census.tsv</c>.
    /// </para>
    /// </remarks>
    /// <param name="shapeType">The <c>msofbtSp</c> record's instance.</param>
    public static bool MirrorsVertically(ushort shapeType) => shapeType == 8;

    /// <summary>
    /// A binary adjustment value in the units the layouter's presets expect, or null when the two
    /// vocabularies do not measure the same thing and the preset's own default is the better
    /// answer.
    /// </summary>
    /// <param name="shapeType">The shape type, which decides whether the value means anything.</param>
    /// <param name="value">The <c>adjustValue</c> property, in 21600ths.</param>
    /// <param name="size">
    /// The shape's extent; only the presets in <see cref="AdjustmentAcrossWidth"/> consult it.
    /// </param>
    public static double? Adjustment(ushort shapeType, int value, DocSize size)
    {
        if (AdjustmentAcrossWidth(shapeType))
        {
            double shortest = Math.Min(size.Width.Emu, size.Height.Emu);
            return shortest <= 0
                ? null
                : (double)value * DrawingMlViewBox / AdjustmentViewBox * (size.Width.Emu / shortest);
        }

        return AdjustmentInViewBox(shapeType)
            ? (double)value * DrawingMlViewBox / AdjustmentViewBox
            : null;
    }

    /// <summary>Whether a shape of this type is filled when it does not say.</summary>
    public static bool IsFilledByDefault(ushort shapeType) => !InTable(UnfilledByDefault, shapeType);

    /// <summary>Whether a shape of this type is stroked when it does not say.</summary>
    public static bool IsStrokedByDefault(ushort shapeType) => !InTable(UnstrokedByDefault, shapeType);

    private static bool InTable(ReadOnlySpan<ushort> table, ushort shapeType)
        => shapeType < table.Length * 16
           && (table[shapeType >> 4] & (1 << (shapeType & 0x0F))) != 0;
}

namespace Paperless.Presentations.MsBinary;

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
/// only for the two presets where the two definitions coincide, and every other preset is drawn at
/// its stated defaults — a right arrow with a default head is right in outline and slightly wrong
/// in proportion, where a right arrow fed a foreign adjustment is neither.
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
    /// <see cref="Paperless.Ooxml.DrawingML.PresetShapeGeometry"/> actually knows are kept, so a
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
    /// A binary adjustment value in the units the layouter's presets expect.
    /// </summary>
    /// <param name="shapeType">The shape type, which decides whether the value means anything.</param>
    /// <param name="value">The <c>adjustValue</c> property, in 21600ths.</param>
    public static int? Adjustment(ushort shapeType, int value)
        => PresetOf(shapeType) is "roundRect" or "triangle"
            ? (int)((long)value * DrawingMlViewBox / AdjustmentViewBox)
            : null;

    /// <summary>Whether a shape of this type is filled when it does not say.</summary>
    public static bool IsFilledByDefault(ushort shapeType) => !InTable(UnfilledByDefault, shapeType);

    /// <summary>Whether a shape of this type is stroked when it does not say.</summary>
    public static bool IsStrokedByDefault(ushort shapeType) => !InTable(UnstrokedByDefault, shapeType);

    private static bool InTable(ReadOnlySpan<ushort> table, ushort shapeType)
        => shapeType < table.Length * 16
           && (table[shapeType >> 4] & (1 << (shapeType & 0x0F))) != 0;
}

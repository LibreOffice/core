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
/// lookup rather than a second geometry engine, and it covers exactly the six the layouter
/// expands; everything else falls back to its bounding rectangle, which is where the shape is, in
/// the right colour, with the wrong outline.
/// </para>
/// <para>
/// <strong>An adjustment value is in a different unit in each vocabulary.</strong> The binary
/// form states it in a 21600-unit view box; DrawingML states it in hundred-thousandths. Feeding
/// one to the other unconverted makes a rounded rectangle either perfectly square or a stadium,
/// with nothing in between to notice.
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
    /// <param name="shapeType">The <c>msofbtSp</c> record's instance.</param>
    public static string? PresetOf(ushort shapeType) => shapeType switch
    {
        2 => "roundRect",
        3 => "ellipse",
        4 => "diamond",
        5 => "triangle",
        6 => "rtTriangle",
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

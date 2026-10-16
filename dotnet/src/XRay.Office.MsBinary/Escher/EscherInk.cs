using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.MsBinary.Escher;

/// <summary>
/// A shape's fill and outline, as the Escher property table states them.
/// </summary>
/// <remarks>
/// <para>
/// The four properties are <c>fillColor</c> (385), <c>fFilled</c> (443), <c>lineColor</c> (448)
/// and <c>lineWidth</c> (459), with <c>fLine</c> (508) deciding whether there is an outline at
/// all. Nothing in this tree read any of them until round 84 — a worksheet shape's fill was
/// unread in all three of its formats — and the <c>.xls</c> half of that reaches this file.
/// </para>
/// <para>
/// <strong>Round 84's two conservative choices were both wrong, and together they read almost
/// nothing.</strong> They were <em>presence is the test</em> — a colour is taken only where the
/// shape states one, although MS-ODRAW defaults <c>fillColor</c> to white and <c>lineColor</c> to
/// black — and <em>only the literal <c>MSO_CLR</c> form is honoured</em>. Measured over the 64
/// corpus <c>.xls</c>: <strong>92 of the 106</strong> stated fill and line colours are palette
/// references rather than literals, and shapes stating <c>fLine</c> true with no <c>lineColor</c>
/// are a class of their own. On <c>TICAPCapability_Final.xls</c> the two <c>Instructions</c> text
/// boxes state <c>fillColor 0x08000041</c> and <c>lineColor 0x08000040</c> — palette 65 and 64,
/// which are Excel's <em>window background</em> and <em>window text</em>, so white and black —
/// and the reference draws a white panel with a 0.42 pt black border where this drew nothing at
/// all. <see cref="EscherColour"/> resolves the reference, and the two defaults are taken.
/// </para>
/// <para>
/// <strong>Whether there is a fill at all is a property of the shape <em>type</em> when the shape
/// does not say.</strong> <c>DffPropertyReader::ApplyFillAttributes</c>
/// (<c>filter/source/msfilter/msdffimp.cxx</c>:1313-1323) starts from the format's own default —
/// <c>mso_PropSetDefaults</c> gives property 447 the value <c>0x001C</c> and 511 the value
/// <c>0x001E</c>, so <em>both</em> <c>fFilled</c> and <c>fLine</c> default true
/// (<c>filter/source/msfilter/dffpropset.cxx</c>) — and then clears the bit again unless the
/// shape stated it <em>hard</em> or the type is filled (stroked) by default. So an unstated
/// boolean is the type's answer, not a constant: <c>mso_DefaultFillingTable</c> and
/// <c>mso_DefaultStrokingTable</c> (<c>svx/source/customshapes/EnhancedCustomShapeGeometry.cxx</c>:6156-6213)
/// are the tables, and they matter here because a picture frame is neither filled nor stroked by
/// default while a text box and a rectangle are both.
/// </para>
/// <para>
/// <strong>A stated boolean is honoured either way</strong>, which the corpus needs in both
/// directions: 216 of its worksheet shapes state property 511 as <c>0x00080000</c> — <c>fLine</c>
/// hard, and false — and 52 state 447 as <c>0x00100010</c>, which is <c>fFilled</c> hard and true.
/// </para>
/// </remarks>
public static class EscherInk
{
    /// <summary>The width the format gives a line that states none: one point, in EMUs.</summary>
    /// <remarks>
    /// <c>GetPropertyValue(DFF_Prop_lineWidth, 9525)</c>, <c>msdffimp.cxx</c>:916. The property's
    /// own table default is unset, so this constant is the reference's and not the format's.
    /// </remarks>
    private const uint DefaultLineWidth = 9525;

    /// <summary>
    /// The colour a shape with a fill and no <c>fillColor</c> is filled with.
    /// </summary>
    /// <remarks>
    /// White twice over: <c>mso_PropSetDefaults</c> gives property 385 the value <c>0xffffff</c>,
    /// and Calc puts the same answer on the object a second time — <em>"filled without color -&gt;
    /// set system window color"</em>, <c>XclImpDffConverter::ProcessObj</c>,
    /// <c>sc/source/filter/excel/xiescher.cxx</c>:3693-3695.
    /// </remarks>
    private const uint DefaultFillColour = 0x00FFFFFFu;

    /// <summary>A shape's resolved interior and outline.</summary>
    /// <param name="Fill">The interior colour, or null when the shape is not filled flat.</param>
    /// <param name="Stroke">The outline colour, or null when it has none.</param>
    /// <param name="StrokeWidth">How wide that outline is.</param>
    public readonly record struct Ink(Colour? Fill, Colour? Stroke, Length StrokeWidth)
    {
        /// <summary>
        /// True when the shape is filled with a bitmap rather than with a colour.
        /// </summary>
        /// <remarks>
        /// <c>mso_fillPattern</c>, <c>mso_fillTexture</c> and <c>mso_fillPicture</c>, which are
        /// the three fill types <c>ApplyFillAttributes</c> resolves to
        /// <c>drawing::FillStyle_BITMAP</c> (<c>filter/source/msfilter/msdffimp.cxx</c>:1335-1339
        /// in this tree). <see cref="Fill"/> is null for all three: their colour is inside the
        /// tile and building it needs the blip store, which this class does not have. See
        /// <see cref="EscherPatternFill"/>.
        /// </remarks>
        public bool BitmapFill { get; init; }

        /// <summary>True when there is something to paint.</summary>
        public bool HasInk => Fill is not null || Stroke is not null || BitmapFill;
    }

    /// <summary>Reads a shape's fill and outline.</summary>
    /// <param name="properties">The shape's property table.</param>
    /// <param name="shapeType">
    /// The <c>msofbtSp</c> shape type, which decides the fill and the outline wherever the shape
    /// states neither.
    /// </param>
    /// <param name="scheme">
    /// Resolves the host's palette index; null for a host that has none. See
    /// <see cref="EscherColour.Resolve"/>.
    /// </param>
    public static Ink Read(
        EscherPropertyTable properties, ushort shapeType = 0, Func<int, Colour?>? scheme = null)
    {
        ArgumentNullException.ThrowIfNull(properties);

        bool filled = properties.StatesBoolean(EscherPropertyIds.Filled)
            ? properties.Boolean(EscherPropertyIds.Filled)
            : IsFilledByDefault(shapeType);

        // Only the fill types that paint something. The reference's switch
        // (msdffimp.cxx:1330-1400) leaves eXFill at NONE for anything it does not name, which is
        // `mso_fillBackground` and every value past it. A gradient is still painted here as its
        // foreground colour, which is one colour short of right and a great deal closer than
        // nothing.
        uint fillType = properties.Value(EscherPropertyIds.FillType);
        if (fillType > LastPaintingFillType) filled = false;

        // A pattern, a texture and a picture are the three the reference resolves to
        // `FillStyle_BITMAP`, and their colour is inside the tile rather than in `fillColor`.
        // Painting the foreground colour over the whole shape put a solid grey block where
        // `apron-area.xls` states a hatch; the caller builds the tile through
        // `EscherPatternFill` and this reports only that there is one to build.
        bool bitmap = filled && fillType is >= FirstBitmapFillType and <= LastBitmapFillType;

        Colour? fill = filled && !bitmap
            ? EscherColour.Resolve(
                properties.Value(EscherPropertyIds.FillColour, DefaultFillColour),
                Colour.White,
                scheme)
            : null;

        bool lined = properties.StatesBoolean(EscherPropertyIds.Lined)
            ? properties.Boolean(EscherPropertyIds.Lined)
            : IsStrokedByDefault(shapeType);

        Colour? stroke = lined
            ? EscherColour.Resolve(
                properties.Value(EscherPropertyIds.LineColour), Colour.Black, scheme)
            : null;

        Length width = stroke is null
            ? Length.Zero
            : Length.FromEmu(properties.Value(EscherPropertyIds.LineWidth, DefaultLineWidth));

        return new Ink(fill, stroke, width) { BitmapFill = bitmap };
    }

    /// <summary>The first <c>MSO_FILLTYPE</c> the reference paints as a bitmap.</summary>
    /// <remarks><c>mso_fillPattern</c>; the next two are texture and picture.</remarks>
    private const uint FirstBitmapFillType = 1;

    /// <summary>The last of them, <c>mso_fillPicture</c>.</summary>
    private const uint LastBitmapFillType = 3;

    /// <summary>The last <c>MSO_FILLTYPE</c> that paints anything.</summary>
    /// <remarks>
    /// <c>mso_fillShadeTitle</c>. The next value, <c>mso_fillBackground</c>, means "use whatever
    /// is behind the shape" and falls into the reference's <c>default:</c>.
    /// </remarks>
    private const uint LastPaintingFillType = 8;

    /// <summary>
    /// Whether a shape type is filled when its shape says nothing about it.
    /// </summary>
    /// <remarks>
    /// <c>IsCustomShapeFilledByDefault</c> and its <c>mso_DefaultFillingTable</c>,
    /// <c>svx/source/customshapes/EnhancedCustomShapeGeometry.cxx</c>:6156-6167. A set bit means
    /// <em>not</em> filled, and only the first 256 types are in the table; everything above is
    /// filled.
    /// </remarks>
    public static bool IsFilledByDefault(ushort shapeType)
        => shapeType >= 0x100
            || (DefaultFilling[shapeType >> 4] & (1 << (shapeType & 0xF))) == 0;

    /// <summary>
    /// Whether a shape type is stroked when its shape says nothing about it.
    /// </summary>
    /// <remarks>
    /// <c>IsCustomShapeStrokedByDefault</c> and its <c>mso_DefaultStrokingTable</c> (same file,
    /// <c>:6197-6213</c>). One bit is set in the whole table and it is shape 75, the picture
    /// frame.
    /// </remarks>
    public static bool IsStrokedByDefault(ushort shapeType)
        => shapeType >= 0x100
            || (DefaultStroking[shapeType >> 4] & (1 << (shapeType & 0xF))) == 0;

    /// <summary>A set bit marks a shape type that is <em>not</em> filled by default.</summary>
    private static readonly ushort[] DefaultFilling =
    [
        0x0000, 0x0018, 0x01ff, 0x0000, 0x0c00, 0x01e0, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0600, 0x0000, 0x0000, 0x0000, 0x0000,
    ];

    /// <summary>A set bit marks a shape type that is <em>not</em> stroked by default.</summary>
    private static readonly ushort[] DefaultStroking =
    [
        0x0000, 0x0000, 0x0000, 0x0000, 0x0800, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    ];
}

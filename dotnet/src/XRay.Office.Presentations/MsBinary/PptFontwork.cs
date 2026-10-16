using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.MsBinary.Escher;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Presentations.MsBinary;

/// <summary>
/// An Escher WordArt shape — one whose <c>msofbtOPT</c> sets the <c>fGtext</c> bit — drawn as
/// warped outlines.
/// </summary>
/// <remarks>
/// <para>
/// This is the binary sibling of the word processor's <c>DocxVmlFontwork</c> and of
/// <c>SlideFontwork</c>, and it is the route by which the least is carried in the shape and the
/// most in the property table. There is no text body at all: <strong>the words are an Escher
/// property</strong>, <c>gtextUNICODE</c> (192), and so are the face (<c>gtextFont</c>, 197), the
/// size (<c>gtextSize</c>, 195) and every switch.
/// <c>SvxMSDffManager::ImportShape</c> reads exactly those
/// (<c>filter/source/msfilter/msdffimp.cxx</c>:4423-4530) and
/// <c>DffPropertyReader::ApplyAttributes</c> (<c>:2516-2570</c>) turns the bits into the
/// <c>TextPath</c> property set the Fontwork engine consumes.
/// </para>
/// <para>
/// <strong>The switch is one bit and it is not the shape type.</strong>
/// <c>bIsFontwork = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) &amp; 0x4000 ) != 0</c>
/// — property 255 — so a shape is WordArt because it says so, and the type only chooses which of
/// the forty <c>fontwork-*</c> presets is bent. Censused on the corpus's 51 <c>.ppt</c>: two shapes
/// carry the bit and both are in the 136…175 text-path band, so nothing outside it is reachable
/// there (<c>dotnet/probes/slides-r107/fgtext.txt</c>).
/// </para>
/// <para>
/// <strong>The fill is the shape's own, unlike the DrawingML path's.</strong>
/// <c>CreateSdrObjectFromParagraphOutlines</c> gives the path object
/// <c>SfxItemSet aSet(rSdrObjCustomShape.GetMergedItemSet())</c> with only the shadow cleared
/// (<c>svx/source/customshapes/EnhancedCustomShapeFontWork.cxx</c>:1123-1126), and an Escher
/// WordArt has no runs to have copied a character fill from — so what fills the glyphs is
/// <c>fillType</c>/<c>fillColor</c>, which is <see cref="PptFills"/>'s ordinary answer. That is why
/// the caller passes the shape's resolved fill rather than a run colour.
/// </para>
/// <para>
/// <strong>Nothing here is derived that the file states.</strong> <c>ApplyAttributes</c> writes
/// <c>ScaleX</c> from bit <c>0x40</c> of property 255 literally, so the four presets that keep
/// their font size on the DrawingML path do not derive it here — which is also why
/// <c>FontworkRequest.FromWordArt</c> is not consulted: stating <c>KeepsFontSize</c> bypasses the
/// derivation it feeds.
/// </para>
/// </remarks>
internal static class PptFontwork
{
    /// <summary>The property holding the WordArt's own text, as UTF-16 with a terminator.</summary>
    /// <remarks><c>DFF_Prop_gtextUNICODE</c>, <c>include/svx/msdffdef.hxx</c>:121.</remarks>
    public const ushort GeometryText = 192;

    /// <summary>Where the text sits along the path — an <c>MSO_GEOTEXTALIGN</c>.</summary>
    public const ushort GeometryTextAlign = 194;

    /// <summary>The stated point size, in 16.16 fixed point.</summary>
    public const ushort GeometryTextSize = 195;

    /// <summary>The family the WordArt is set in.</summary>
    public const ushort GeometryTextFont = 197;

    /// <summary>The group entry the WordArt booleans are packed into.</summary>
    /// <remarks>
    /// <c>DFF_Prop_gtextFStrikethrough</c>. The reference masks this value directly rather than
    /// going through <c>GetPropertyBool</c>, so the masks below are its own.
    /// </remarks>
    public const ushort GeometryTextFlags = 255;

    /// <summary>Whether the shape is WordArt at all — <c>fGtext</c>.</summary>
    private const uint TextPathOn = 0x4000;

    /// <summary>Whether the glyphs keep their stated size — the <c>ScaleX</c> property.</summary>
    private const uint ScaleX = 0x40;

    /// <summary>Bold.</summary>
    private const uint Bold = 0x20;

    /// <summary>Italic.</summary>
    private const uint Italic = 0x10;

    /// <summary>The first adjustment property, <c>DFF_Prop_adjustValue</c>.</summary>
    private const ushort FirstAdjustment = 327;

    /// <summary>The last one the reference looks for, <c>DFF_Prop_adjust10Value</c>.</summary>
    private const ushort LastAdjustment = 336;

    /// <summary>The first shape type whose sole adjustment is a polar angle.</summary>
    /// <remarks>See <see cref="AdjustmentIsAngle"/>.</remarks>
    private const int FirstCurveShapeType = 144;

    /// <summary>The last of them.</summary>
    private const int LastCurveShapeType = 151;

    /// <summary>What the reference reads a bold WordArt's weight as.</summary>
    private const int BoldWeight = 700;

    /// <summary>And an ordinary one's.</summary>
    private const int NormalWeight = 400;

    /// <summary>Whether the shape asks to be drawn as WordArt.</summary>
    /// <remarks>
    /// <c>msdffimp.cxx</c>:4424-4425. The bit alone; the shape type is consulted only for the
    /// preset.
    /// </remarks>
    public static bool IsWordArt(EscherPropertyTable properties)
    {
        ArgumentNullException.ThrowIfNull(properties);
        return (properties.Value(GeometryTextFlags) & TextPathOn) != 0;
    }

    /// <summary>
    /// The warped outlines of an Escher WordArt shape, in the shape's own coordinates, or null.
    /// </summary>
    /// <remarks>
    /// Null when the shape is not WordArt, when its type names no <c>fontwork-*</c> preset, when it
    /// states no text, or when the face it names has no <c>glyf</c> outlines — the last three being
    /// the same "the reference drew curves and this cannot" the other two families answer by
    /// drawing nothing.
    /// </remarks>
    /// <param name="shape">The shape, for its type and its property table.</param>
    /// <param name="size">Its rectangle, which the warp is fitted into.</param>
    /// <param name="fonts">The resolver, for the face <c>gtextFont</c> names.</param>
    public static GraphicsPath? Outline(EscherShape shape, DocSize size, SlideFonts fonts)
    {
        ArgumentNullException.ThrowIfNull(shape);
        ArgumentNullException.ThrowIfNull(fonts);

        EscherPropertyTable properties = shape.Properties;
        if (!IsWordArt(properties)) return null;
        if (Fontwork.FontworkTypeOfShapeType(shape.ShapeType) is not { } type) return null;

        string? text = properties.Text(GeometryText);
        if (text is not { Length: > 0 }) return null;

        uint flags = properties.Value(GeometryTextFlags);

        (OpenTypeFace? face, FontReference? _) = fonts.Resolve(
            properties.Text(GeometryTextFont),
            (flags & Bold) != 0 ? BoldWeight : NormalWeight,
            (flags & Italic) != 0);

        if (face is null) return null;

        List<string> lines = Lines(text);

        // A shape asking to keep a size it does not state cannot keep it; every other preset
        // ignores the size outright, so the fallback is only reachable through a malformed file.
        Length stated = StatedSize(properties);
        bool keepsSize = (flags & ScaleX) != 0 && stated > Length.Zero;

        return Fontwork.Outline(new FontworkRequest
        {
            FontworkType = type,
            AdjustmentValues = Adjustments(properties, shape.ShapeType),
            KeepsFontSize = keepsSize,
            Box = size,
            Lines = lines,
            Face = face,
            FontSize = stated,
            Alignment = Alignment(properties),
        });
    }

    /// <summary>The paragraphs of a WordArt's string.</summary>
    /// <remarks>
    /// <c>SvxMSDffManager::ReadObjText</c> (<c>msdffimp.cxx</c>:3698-3717) breaks on <c>0x0a</c> and
    /// on <c>0x0d</c> alike, and swallows the other one when it follows — so <c>CRLF</c> and
    /// <c>LFCR</c> are one break and a lone <c>CR</c> is a break of its own. Splitting on both
    /// characters without that rule turns every Windows line ending into a blank paragraph.
    /// </remarks>
    private static List<string> Lines(string text)
    {
        List<string> lines = [];
        int start = 0;

        for (int at = 0; at < text.Length; at++)
        {
            char character = text[at];
            if (character is not ('\n' or '\r')) continue;

            lines.Add(text[start..at]);

            char partner = character == '\n' ? '\r' : '\n';
            if (at + 1 < text.Length && text[at + 1] == partner) at++;

            start = at + 1;
        }

        if (start < text.Length) lines.Add(text[start..]);
        return lines;
    }

    /// <summary>The point size <c>gtextSize</c> states, or zero when it states none.</summary>
    /// <remarks>
    /// <c>msdffimp.cxx</c>:2626-2627 puts it through <c>SvxMSDffManager::ScalePt</c>, which is
    /// <c>GetMapFactor( MapUnit::MapPoint, … ) / 65536</c> (<c>:3199-3204</c>) — so the stored
    /// number is a point in 16.16 fixed point and the division is the whole conversion.
    /// </remarks>
    private static Length StatedSize(EscherPropertyTable properties)
        => properties.Has(GeometryTextSize)
            ? Length.FromPoints(properties.Value(GeometryTextSize) / 65536.0)
            : Length.Zero;

    /// <summary>
    /// The adjustment values, in the units the preset tables are written in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Escher states them as plain integers in the 21600 view box, <em>except</em> where the
    /// shape's default handle is polar: <c>ApplyAttributes</c> divides such a value by 65536
    /// (<c>msdffimp.cxx</c>:2591-2599, on the mask <c>:2166-2174</c> builds), because a polar
    /// handle's position is an angle in 16.16 fixed-point degrees. See
    /// <see cref="AdjustmentIsAngle"/>.
    /// </para>
    /// <para>
    /// Trailing properties the shape does not state are left at the preset's own defaults, which is
    /// what the reference reaches by leaving them <c>PropertyState_DEFAULT_VALUE</c>: this returns
    /// a list only as long as the highest adjustment stated, and
    /// <see cref="XRay.Office.Ooxml.DrawingML.FontworkPresets"/> fills the rest in.
    /// </para>
    /// </remarks>
    private static List<double>? Adjustments(EscherPropertyTable properties, int shapeType)
    {
        int highest = -1;
        for (ushort id = LastAdjustment; id >= FirstAdjustment; id--)
        {
            if (!properties.Has(id)) continue;
            highest = id - FirstAdjustment;
            break;
        }

        if (highest < 0) return null;

        List<double> values = new(highest + 1);
        for (int index = 0; index <= highest; index++)
        {
            int stated = properties.SignedValue((ushort)(FirstAdjustment + index));
            values.Add(AdjustmentIsAngle(shapeType, index) ? stated / 65536.0 : stated);
        }

        return values;
    }

    /// <summary>
    /// Whether an adjustment of this shape type is a 16.16 fixed-point angle rather than a
    /// view-box coordinate.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>msdffimp.cxx</c>:2166-2174 sets the conversion bit for handle <em>i</em> when that handle
    /// is <c>SvxMSDffHandleFlags::POLAR</c> and its <c>nPositionY</c> is at least <c>0x256</c> or at
    /// most <c>0x107</c> — <c>0x100 + n</c> being "adjustment n", so the test admits the first eight
    /// adjustments — and :2591-2599 then divides that adjustment by 65536.
    /// </para>
    /// <para>
    /// Across the whole WordArt vocabulary that is exactly the eight <c>*Curve</c> and <c>*Pour</c>
    /// types, 144 to 151, each of which declares one handle, at index 0, with
    /// <c>nPositionY = 0x100</c>
    /// (<c>svx/source/customshapes/EnhancedCustomShapeGeometry.cxx</c>, the
    /// <c>mso_sptTextArchUpCurveHandle</c>, <c>…ArchDownCurve…</c>, <c>…CircleCurve…</c>,
    /// <c>…ButtonCurve…</c>, <c>…ArchPour…</c>, <c>…CirclePour…</c> and <c>…ButtonPour…</c> tables).
    /// The other thirty-two declare a <c>RANGE</c> handle and are converted by nothing. A pour's
    /// second adjustment is its radius and is <em>not</em> converted: the conversion is keyed on the
    /// handle index, and a pour has one handle.
    /// </para>
    /// </remarks>
    private static bool AdjustmentIsAngle(int shapeType, int index)
        => index == 0 && shapeType is >= FirstCurveShapeType and <= LastCurveShapeType;

    /// <summary>Where the text sits along the path.</summary>
    /// <remarks>
    /// <c>msdffimp.cxx</c>:4466-4480, whose default for an absent <c>gtextAlign</c> is
    /// <c>mso_alignTextCenter</c>. The three justifying modes become
    /// <c>SDRTEXTHORZADJUST_BLOCK</c>, which the Fontwork layouter treats as left —
    /// <c>case SDRTEXTHORZADJUST_BLOCK : break; // don't know</c>,
    /// <c>EnhancedCustomShapeFontWork.cxx</c>:621.
    /// </remarks>
    private static FontworkAlignment Alignment(EscherPropertyTable properties)
        => properties.Value(GeometryTextAlign, MsoAlignTextCentre) switch
        {
            MsoAlignTextLeft => FontworkAlignment.Left,
            MsoAlignTextRight => FontworkAlignment.Right,
            MsoAlignTextStretch or MsoAlignTextLetterJust or MsoAlignTextWordJust
                => FontworkAlignment.Left,
            _ => FontworkAlignment.Centre,
        };

    /// <summary><c>mso_alignTextStretch</c>, the first of <c>MSO_GEOTEXTALIGN</c>.</summary>
    private const uint MsoAlignTextStretch = 0;

    /// <summary><c>mso_alignTextCenter</c>, and the reference's default.</summary>
    private const uint MsoAlignTextCentre = 1;

    /// <summary><c>mso_alignTextLeft</c>.</summary>
    private const uint MsoAlignTextLeft = 2;

    /// <summary><c>mso_alignTextRight</c>.</summary>
    private const uint MsoAlignTextRight = 3;

    /// <summary><c>mso_alignTextLetterJust</c>.</summary>
    private const uint MsoAlignTextLetterJust = 4;

    /// <summary><c>mso_alignTextWordJust</c>.</summary>
    private const uint MsoAlignTextWordJust = 5;
}

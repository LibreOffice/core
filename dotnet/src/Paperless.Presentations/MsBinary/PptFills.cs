using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.MsBinary.Escher;
using Paperless.Presentations.Layout;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// Turns an Escher property table's fill properties into a <see cref="Paint"/>.
/// </summary>
/// <remarks>
/// <para>
/// Ported from <c>DffPropertyReader::ApplyFillAttributes</c> and
/// <c>DffPropertyReader::ImportGradientColor</c>
/// (<c>filter/source/msfilter/msdffimp.cxx:1313</c> and <c>:2840</c>). Shared by shape fills and
/// by page backgrounds, because in this format they are the same thing read twice: a background
/// is an ordinary <c>SpContainer</c> sitting beside the patriarch with <c>fBackground</c> set,
/// and LibreOffice runs it through the same <c>ApplyAttributes</c>
/// (<c>svdfppt.cxx:3055-3060</c>).
/// </para>
/// <para>
/// <strong>Resolving only <c>mso_fillSolid</c> is not a small omission.</strong> A deck's whole
/// design usually lives in its master's background shape, and six of the corpus's fifty-one PPT
/// files fill that shape with a picture while eight fill it with a shade. Reading the
/// <c>fillColor</c> of a picture fill gives whatever colour the writer happened to leave there —
/// on <c>010605Vul.ppt</c> a white page in place of a dark green one carrying the deck's
/// photographs, its emblem and its strapline, which is 60% of the page's ink on all
/// twenty-one slides.
/// </para>
/// </remarks>
internal static class PptFills
{
    // Fill kinds — MSO_FILLTYPE, include/svx/msdffdef.hxx.

    /// <summary>A uniform colour.</summary>
    public const uint Solid = 0;

    /// <summary>An eight-by-eight bitmap tiled and recoloured with the two fill colours.</summary>
    public const uint Pattern = 1;

    /// <summary>A tiled bitmap carrying its own colours.</summary>
    public const uint Texture = 2;

    /// <summary>One picture stretched across the shape.</summary>
    public const uint Picture = 3;

    /// <summary>A shade from one end of the shape to the other.</summary>
    public const uint Shade = 4;

    /// <summary>A shade from the shape's bounding rectangle inwards to a point.</summary>
    public const uint ShadeCentre = 5;

    /// <summary>A shade from the shape's outline inwards.</summary>
    public const uint ShadeShape = 6;

    /// <summary>A shade like <see cref="Shade"/>, with the angle scaled to the shape.</summary>
    public const uint ShadeScale = 7;

    /// <summary>PowerPoint's "shade to title" variant of <see cref="Shade"/>.</summary>
    public const uint ShadeTitle = 8;

    /// <summary>Show whatever is behind the shape — the page's own background.</summary>
    public const uint UseBackground = 9;

    /// <summary>The kind of fill, <c>DFF_Prop_fillType</c>.</summary>
    public const ushort FillType = 384;

    /// <summary>The second fill colour, <c>DFF_Prop_fillBackColor</c>.</summary>
    public const ushort FillBackColour = 387;

    /// <summary>The blip a pattern, texture or picture fill draws, <c>DFF_Prop_fillBlip</c>.</summary>
    public const ushort FillBlip = 390;

    /// <summary>A texture tile's width in EMUs, <c>DFF_Prop_fillWidth</c>.</summary>
    public const ushort FillWidth = 393;

    /// <summary>A texture tile's height in EMUs, <c>DFF_Prop_fillHeight</c>.</summary>
    public const ushort FillHeight = 394;

    /// <summary>The shade direction, 16.16 fixed-point degrees, <c>DFF_Prop_fillAngle</c>.</summary>
    public const ushort FillAngle = 395;

    /// <summary>Where along the ramp the first colour sits, as a percentage, <c>DFF_Prop_fillFocus</c>.</summary>
    public const ushort FillFocus = 396;

    /// <summary>The inner rectangle's right edge, 16.16 fraction, <c>DFF_Prop_fillToRight</c>.</summary>
    public const ushort FillToRight = 399;

    /// <summary>The inner rectangle's bottom edge, 16.16 fraction, <c>DFF_Prop_fillToBottom</c>.</summary>
    public const ushort FillToBottom = 400;

    /// <summary>A 16.16 fraction of one, which is what <c>fillToRight</c> compares against.</summary>
    private const uint WholeFraction = 0x10000;

    /// <summary>
    /// The paint a shape's fill properties describe, or null when it has no fill.
    /// </summary>
    /// <param name="properties">The shape's property table, already inherited through its master.</param>
    /// <param name="filled">Whether the shape is filled at all, resolved by the caller.</param>
    /// <param name="scheme">The page's colour scheme, for the two fill colours.</param>
    /// <param name="box">
    /// The box the fill is measured in, in the space <paramref name="space"/> maps to the slide.
    /// A gradient's ends and a stretched bitmap's extent both come from it.
    /// </param>
    /// <param name="space">The matrix taking <paramref name="box"/> onto the slide.</param>
    /// <param name="picture">
    /// The blip <see cref="FillBlip"/> names, when the caller could resolve it to a raster. Null
    /// leaves a bitmap fill unpainted rather than approximating it with a colour, which is the
    /// rule the rest of this reader follows.
    /// </param>
    public static Paint? Resolve(
        EscherPropertyTable properties,
        bool filled,
        PptColourScheme scheme,
        DocRect box,
        AffineTransform space,
        RasterImage? picture)
    {
        ArgumentNullException.ThrowIfNull(properties);
        if (!filled) return null;

        uint kind = properties.Value(FillType, Solid);

        return kind switch
        {
            Solid => Resolved(properties, EscherPropertyIds.FillColour, scheme) is { } colour
                ? Paint.Solid(colour)
                : null,

            Pattern or Texture or Picture => Bitmap(properties, kind, box, picture),

            Shade or ShadeCentre or ShadeShape or ShadeScale or ShadeTitle
                => Gradient(properties, kind, scheme, box) is { } gradient
                    ? gradient with { Transform = space }
                    : null,

            // "Use the background" is drawn by drawing nothing: the page's own background is
            // already behind the shape, which is what LibreOffice's XFillUseSlideBackgroundItem
            // arranges the long way round.
            _ => null,
        };
    }

    /// <summary>
    /// A pattern, texture or picture fill.
    /// </summary>
    /// <remarks>
    /// A picture is stretched once across the box and the other two tile
    /// (<c>msdffimp.cxx:1443-1456</c>). A texture states its tile in EMUs; a pattern does not,
    /// so its tile is the bitmap's own size. <strong>A pattern is not recoloured here</strong> —
    /// LibreOffice rewrites an eight-by-eight monochrome tile's two colours from
    /// <c>fillColor</c> and <c>fillBackColor</c>, which needs a decoder this library keeps out
    /// of the readers. No corpus deck uses one; drawing the tile as authored is closer than
    /// drawing nothing.
    /// </remarks>
    private static BitmapPaint? Bitmap(
        EscherPropertyTable properties, uint kind, DocRect box, RasterImage? picture)
    {
        if (picture is not { } image) return null;
        if (kind == Picture) return new BitmapPaint(image, box.Size, box.Origin, Stretch: true);

        DocSize natural = SlideImages.NaturalSize(image.EncodedBytes.Span) ?? box.Size;

        Length width = Length.FromEmu(properties.Value(FillWidth));
        Length height = Length.FromEmu(properties.Value(FillHeight));
        DocSize tile = new(
            width > Length.Zero ? width : natural.Width,
            height > Length.Zero ? height : natural.Height);

        if (tile.Width <= Length.Zero || tile.Height <= Length.Zero) return null;

        return new BitmapPaint(image, tile, box.Origin, Stretch: false);
    }

    /// <summary>
    /// A shaded fill, as the linear or centred ramp LibreOffice resolves it to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ImportGradientColor</c> (<c>msdffimp.cxx:2840</c>) decides three things and this
    /// follows all three. <em>Which way round the colours go</em> is a parity built from four
    /// independent conditions — a non-negative angle, a zero or negative focus, a focus near the
    /// middle, and a shape-shaped or centred fill — each of which flips it. <em>The angle</em> is
    /// <c>3600 − Fix16ToAngle(fillAngle)</c> in tenths of a degree, which puts it in the same
    /// anticlockwise-from-downwards convention ODF states and this library already carries.
    /// <em>The geometry</em> is linear unless the focus is between 40 and 60, which makes it
    /// axial, or the fill is shape-shaped or centred, which makes it rectangular.
    /// </para>
    /// <para>
    /// The multi-stop array in <c>fillShadeColors</c> is deliberately not read: LibreOffice loads it
    /// and then uses it only for the bitmap substitution of a centred shade, so honouring it here
    /// would draw a ramp the reference does not.
    /// </para>
    /// </remarks>
    private static GradientPaint? Gradient(
        EscherPropertyTable properties, uint kind, PptColourScheme scheme, DocRect box)
    {
        int rawAngle = properties.SignedValue(FillAngle);
        bool swap = rawAngle >= 0;

        double angle = Angle(rawAngle);

        int focus = properties.SignedValue(FillFocus);
        if (focus == 0)
        {
            swap = !swap;
        }
        else if (focus < 0)
        {
            focus = -focus;
            swap = !swap;
        }

        bool axial = focus is > 40 and < 60;
        if (axial) swap = !swap;

        double focusX = focus / 100.0;
        double focusY = focus / 100.0;
        bool rectangular = false;

        switch (kind)
        {
            case ShadeShape:
                rectangular = true;
                focusX = focusY = 0.5;
                swap = !swap;
                break;

            case ShadeCentre:
                rectangular = true;
                focusX = properties.Value(FillToRight) == WholeFraction ? 1 : 0;
                focusY = properties.Value(FillToBottom) == WholeFraction ? 1 : 0;
                swap = !swap;
                break;

            default:
                break;
        }

        Colour first = Resolved(properties, EscherPropertyIds.FillColour, scheme) ?? Colour.White;
        Colour second = Resolved(properties, FillBackColour, scheme) ?? Colour.White;
        if (swap) (first, second) = (second, first);

        // BGradient takes its stops as (second, first): the ramp starts at the *back* colour once
        // the swap above has been applied.
        if (rectangular)
        {
            // A centred kind takes the ramp inside out — SlideGradients.Centred wants the centre's
            // colour first, and a BGradient's first stop paints the outer edge.
            DocPoint centre = new(
                box.Left + (box.Width * focusX), box.Top + (box.Height * focusY));

            return SlideGradients.Centred(
                GradientKind.Rectangular,
                box,
                centre,
                [new GradientStop(0, first), new GradientStop(1, second)]);
        }

        double radians = angle * Math.PI / 180.0;
        double dx = Math.Sin(radians);
        double dy = Math.Cos(radians);

        return axial
            ? SlideGradients.Linear(box, dx, dy, SlideGradients.Axial(second, first))
            : SlideGradients.Linear(
                box, dx, dy, [new GradientStop(0, second), new GradientStop(1, first)]);
    }

    /// <summary>
    /// A <c>fillAngle</c> as degrees in the convention <see cref="SlideGradients"/> expects.
    /// </summary>
    /// <remarks>
    /// <c>Fix16ToAngle</c> (<c>msdffimp.cxx:393</c>) negates the 16.16 value and normalises it to
    /// a full turn, then <c>ImportGradientColor</c> subtracts that from a full turn — so the
    /// result is the stated angle again, except that a stated zero short-circuits to zero rather
    /// than to a turn. Written out rather than cancelled, because the two normalisations are what
    /// keep a stated 400° and a stated −40° apart.
    /// </remarks>
    private static double Angle(int rawAngle)
    {
        if (rawAngle == 0) return 0;

        double degrees = rawAngle / 65536.0;
        return Normalised(360 - Normalised(-degrees));
    }

    /// <summary>An angle brought into a single turn.</summary>
    private static double Normalised(double degrees)
    {
        double turned = degrees - (Math.Floor(degrees / 360.0) * 360.0);
        return turned >= 360 ? 0 : turned;
    }

    private static Colour? Resolved(EscherPropertyTable properties, ushort id, PptColourScheme scheme)
        => PptColour.Resolve(properties.Value(id, 0xFFFFFF), scheme);
}

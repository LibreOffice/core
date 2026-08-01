using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// A pen, brush, font, palette or region a metafile created and can select.
/// </summary>
/// <remarks>
/// One closed hierarchy rather than five parallel tables, because the formats use one handle
/// space for all of them: <c>SelectObject</c> takes a handle and the object's own type decides
/// what it changes. A file that deletes a brush and creates a font gets the brush's handle
/// back, so a per-type table would answer the wrong object.
/// </remarks>
public abstract record GraphicsObject;

/// <summary>
/// A pen: what a metafile strokes outlines with.
/// </summary>
/// <remarks>
/// <see cref="Width"/> and <see cref="DashPattern"/> are already in EMUs, because GDI states
/// them in logical units and the mapping in force when the pen was <em>created</em> is the one
/// that applies — not the mapping in force when it is used. LibreOffice maps them in
/// <c>CreateObject</c> for the same reason (<c>emfio/source/reader/mtftools.cxx:1027-1035</c>),
/// and a file that changes the window extent between creating a pen and drawing with it will
/// disagree with any decoder that maps late.
/// </remarks>
/// <param name="Colour">The stroke colour.</param>
/// <param name="Width">The stroke width in EMUs; zero means a hairline.</param>
/// <param name="Style">The raw style word, kept so a caller can see bits this record folded away.</param>
/// <param name="DashPattern">Alternating dash and gap lengths, or null for a solid line.</param>
/// <param name="Cap">How ends are drawn.</param>
/// <param name="Join">How corners are drawn.</param>
public sealed record MetafilePen(
    Colour Colour,
    Length Width,
    uint Style = PenStyle.Solid,
    IReadOnlyList<Length>? DashPattern = null,
    LineCap Cap = LineCap.Butt,
    LineJoin Join = LineJoin.Miter) : GraphicsObject
{
    /// <summary>A black hairline, which is what a device context starts with.</summary>
    public static MetafilePen Default { get; } = new(Core.Graphics.Colour.Black, Length.Zero);

    /// <summary>A pen that strokes nothing.</summary>
    public static MetafilePen None { get; } = new(Core.Graphics.Colour.Transparent, Length.Zero, PenStyle.Null);

    /// <summary>True when nothing is stroked.</summary>
    public bool IsNull => (Style & PenStyle.StyleMask) == PenStyle.Null || Colour.IsTransparent;

    /// <summary>The pen as a <see cref="Stroke"/>, or null when it strokes nothing.</summary>
    /// <param name="miterLimit">
    /// The limit past which a mitred join is bevelled. It is device-context state in GDI —
    /// <c>EMR_SETMITERLIMIT</c> sets it, not the pen record — so it is supplied by the caller
    /// rather than stored here.
    /// </param>
    public Stroke? ToStroke(double miterLimit = 10.0) => IsNull
        ? null
        : new Stroke(Paint.Solid(Colour), Width, Cap, Join, miterLimit, DashPattern);

    /// <summary>
    /// The dash pattern a GDI style word asks for, at a given dot length.
    /// </summary>
    /// <remarks>
    /// GDI names five styles and states no lengths; the lengths are the device's. LibreOffice
    /// picks a dot of one pen width plus one logical unit and a dash of three of those
    /// (<c>mtftools.cxx:1066-1080</c>), which is what these ratios reproduce. Reproducing them
    /// rather than choosing our own matters because a dashed border drawn with a different
    /// duty cycle reads as a different border, not as the same one slightly off.
    /// </remarks>
    public static IReadOnlyList<Length>? Dashes(uint style, Length dot)
    {
        if (dot <= Length.Zero) dot = Length.FromEmu(1);
        Length dash = dot * 3.0;

        return (style & PenStyle.StyleMask) switch
        {
            PenStyle.Dash => [dash, dot],
            PenStyle.Dot or PenStyle.Alternate => [dot, dot],
            PenStyle.DashDot => [dash, dot, dot, dot],
            PenStyle.DashDotDot => [dash, dot, dot, dot, dot, dot],
            _ => null,
        };
    }
}

/// <summary>
/// A brush: what a metafile fills interiors with.
/// </summary>
/// <param name="Style">Which kind of fill this is.</param>
/// <param name="Colour">The fill colour, and for a hatch the colour of its lines.</param>
/// <param name="Hatch">Which of the six GDI hatches, when <paramref name="Style"/> is hatched.</param>
/// <param name="Pattern">The tile, when the brush is a pattern or DIB brush.</param>
/// <param name="PatternSize">The tile's size in pixels, for deriving how big to draw it.</param>
public sealed record MetafileBrush(
    BrushStyle Style,
    Colour Colour,
    HatchStyle Hatch = HatchStyle.Horizontal,
    RasterImage? Pattern = null,
    DocSize PatternSize = default) : GraphicsObject
{
    /// <summary>An opaque white brush, which is what a device context starts with.</summary>
    public static MetafileBrush Default { get; } = new(BrushStyle.Solid, Core.Graphics.Colour.White);

    /// <summary>A brush that fills nothing.</summary>
    public static MetafileBrush None { get; } = new(BrushStyle.Null, Core.Graphics.Colour.Transparent);

    /// <summary>True when nothing is filled.</summary>
    public bool IsNull => Style == BrushStyle.Null || (Style == BrushStyle.Solid && Colour.IsTransparent);
}

/// <summary>
/// A font, as a metafile's <c>LOGFONT</c> reduced to what a shaper needs.
/// </summary>
/// <remarks>
/// <see cref="Size"/> is in EMUs and mapped at creation, like a pen's width, and for the same
/// reason. <see cref="Escapement"/> is kept in tenths of a degree — GDI's unit — rather than
/// converted, because it is also the unit both metafile formats state it in and converting it
/// early would mean converting it back to compare against a saved state.
/// </remarks>
/// <param name="Family">The face name the file asked for.</param>
/// <param name="Size">The em size in EMUs, always positive.</param>
/// <param name="Weight">Weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">True for italic or oblique.</param>
/// <param name="IsUnderlined">True when the file asked for an underline.</param>
/// <param name="IsStruckOut">True when the file asked for a strikethrough.</param>
/// <param name="Escapement">Baseline rotation in tenths of a degree, anticlockwise.</param>
/// <param name="CharacterSet">The GDI character set byte, which decides how 8-bit text decodes.</param>
/// <param name="WidthScale">
/// The ratio of the stated character width to the height, or zero when the file left the width
/// to the font. A condensed or expanded face is expressed this way and nowhere else.
/// </param>
public sealed record MetafileFont(
    string Family,
    Length Size,
    int Weight = 400,
    bool IsItalic = false,
    bool IsUnderlined = false,
    bool IsStruckOut = false,
    int Escapement = 0,
    byte CharacterSet = 1,
    double WidthScale = 0) : GraphicsObject
{
    /// <summary>
    /// The font a device context starts with.
    /// </summary>
    /// <remarks>
    /// Arial at 12 pt, which is LibreOffice's choice
    /// (<c>mtftools.cxx:1233-1235</c>: family "Arial", height 423 in 1/100 mm). A metafile that
    /// draws text without ever creating a font is common enough that the default is load-bearing
    /// rather than defensive.
    /// </remarks>
    public static MetafileFont Default { get; } = new("Arial", Length.FromMm100(423));
}

/// <summary>A logical palette, which colour references index into.</summary>
/// <param name="Colours">The entries, in order.</param>
public sealed record MetafilePalette(IReadOnlyList<Colour> Colours) : GraphicsObject
{
    /// <summary>An empty palette.</summary>
    public static MetafilePalette Empty { get; } = new([]);

    /// <summary>
    /// The colour a 32-bit COLORREF names.
    /// </summary>
    /// <remarks>
    /// A COLORREF whose top byte is 0x01 is a palette index rather than a colour
    /// (<c>MtfTools::ReadColor</c>, <c>mtftools.cxx:512-530</c>). An out-of-range index falls
    /// back to black rather than throwing, because it is the commonest thing a truncated
    /// palette record produces.
    /// </remarks>
    public Colour Resolve(uint colorRef)
    {
        if ((colorRef & 0xFFFF0000) == 0x01000000)
        {
            int index = (int)(colorRef & 0xFFFF);
            return index < Colours.Count ? Colours[index] : Core.Graphics.Colour.Black;
        }

        return new Colour((byte)colorRef, (byte)(colorRef >> 8), (byte)(colorRef >> 16));
    }
}

/// <summary>
/// A region: an area built from disjoint rectangles, used as a clip or filled directly.
/// </summary>
/// <remarks>
/// <para>
/// A GDI region is stored as a scan list of non-overlapping rectangles, which is the one shape
/// of region a sink that only intersects can express exactly: several disjoint subpaths in one
/// path are their own union under either fill rule. That is why regions are decoded here rather
/// than reduced to their bounding box, as <c>emfio</c> does
/// (<c>wmfreader.cxx:1246-1251</c> creates a placeholder object and draws nothing).
/// </para>
/// <para>
/// It does not extend to combining two regions: <c>RGN_DIFF</c> and <c>RGN_XOR</c> still need
/// real path arithmetic.
/// </para>
/// </remarks>
/// <param name="Path">The region as one path of disjoint rectangular subpaths.</param>
/// <param name="Bounds">The region's bounding rectangle.</param>
/// <param name="RectangleCount">How many rectangles the scan list held.</param>
public sealed record MetafileRegion(GraphicsPath Path, DocRect Bounds, int RectangleCount) : GraphicsObject
{
    /// <summary>True when the region is a single rectangle, the case every consumer handles exactly.</summary>
    public bool IsRectangular => RectangleCount <= 1;
}

/// <summary>A created object whose kind Paperless does not model, so that its handle is still consumed.</summary>
/// <remarks>
/// Handles are assigned by position in the object table, so an unrecognised
/// <c>CreateBitmap</c> that consumed no slot would shift every handle after it and repaint the
/// picture in the wrong colours. LibreOffice's <c>CreateObject()</c> with no argument exists for
/// exactly this (<c>mtftools.cxx:1104</c>).
/// </remarks>
public sealed record UnsupportedGraphicsObject : GraphicsObject
{
    /// <summary>The single instance; the object carries no state.</summary>
    public static UnsupportedGraphicsObject Instance { get; } = new();
}

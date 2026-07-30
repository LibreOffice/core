using Paperless.Core.Units;

namespace Paperless.Core.Geometry;

/// <summary>A point in document space.</summary>
/// <param name="X">Horizontal offset, increasing rightwards.</param>
/// <param name="Y">Vertical offset, increasing downwards.</param>
public readonly record struct DocPoint(Length X, Length Y)
{
    /// <summary>The origin.</summary>
    public static DocPoint Origin => default;

    /// <summary>Translates the point.</summary>
    public DocPoint Offset(Length dx, Length dy) => new(X + dx, Y + dy);
}

/// <summary>A width/height pair in document space.</summary>
/// <param name="Width">Horizontal extent.</param>
/// <param name="Height">Vertical extent.</param>
public readonly record struct DocSize(Length Width, Length Height)
{
    /// <summary>An empty size.</summary>
    public static DocSize Empty => default;

    /// <summary>True when either dimension is zero or negative.</summary>
    public bool IsEmpty => Width <= Length.Zero || Height <= Length.Zero;
}

/// <summary>
/// An axis-aligned rectangle in document space, stored as origin plus size.
/// </summary>
/// <param name="X">Left edge.</param>
/// <param name="Y">Top edge.</param>
/// <param name="Width">Horizontal extent.</param>
/// <param name="Height">Vertical extent.</param>
public readonly record struct DocRect(Length X, Length Y, Length Width, Length Height)
{
    /// <summary>An empty rectangle at the origin.</summary>
    public static DocRect Empty => default;

    /// <summary>Creates a rectangle from a position and a size.</summary>
    public DocRect(DocPoint origin, DocSize size)
        : this(origin.X, origin.Y, size.Width, size.Height) { }

    /// <summary>Creates a rectangle from two opposite corners, normalising the result.</summary>
    public static DocRect FromCorners(DocPoint a, DocPoint b) => new(
        a.X < b.X ? a.X : b.X,
        a.Y < b.Y ? a.Y : b.Y,
        a.X < b.X ? b.X - a.X : a.X - b.X,
        a.Y < b.Y ? b.Y - a.Y : a.Y - b.Y);

    /// <summary>The left edge.</summary>
    public Length Left => X;

    /// <summary>The top edge.</summary>
    public Length Top => Y;

    /// <summary>The right edge.</summary>
    public Length Right => X + Width;

    /// <summary>The bottom edge.</summary>
    public Length Bottom => Y + Height;

    /// <summary>The top-left corner.</summary>
    public DocPoint Origin => new(X, Y);

    /// <summary>The rectangle's extent.</summary>
    public DocSize Size => new(Width, Height);

    /// <summary>True when the rectangle encloses no area.</summary>
    public bool IsEmpty => Size.IsEmpty;

    /// <summary>Translates the rectangle.</summary>
    public DocRect Offset(Length dx, Length dy) => new(X + dx, Y + dy, Width, Height);

    /// <summary>Shrinks the rectangle inwards by the given insets.</summary>
    public DocRect Deflate(Margins insets) => new(
        X + insets.Left,
        Y + insets.Top,
        Width - insets.Left - insets.Right,
        Height - insets.Top - insets.Bottom);

    /// <summary>True when the point falls inside the rectangle, treating it as half-open.</summary>
    public bool Contains(DocPoint p) => p.X >= Left && p.X < Right && p.Y >= Top && p.Y < Bottom;

    /// <summary>True when the two rectangles share any area.</summary>
    public bool IntersectsWith(DocRect other) =>
        other.Left < Right && other.Right > Left && other.Top < Bottom && other.Bottom > Top;

    /// <summary>The smallest rectangle enclosing both inputs.</summary>
    public DocRect Union(DocRect other)
    {
        if (IsEmpty) return other;
        if (other.IsEmpty) return this;
        Length left = Left < other.Left ? Left : other.Left;
        Length top = Top < other.Top ? Top : other.Top;
        Length right = Right > other.Right ? Right : other.Right;
        Length bottom = Bottom > other.Bottom ? Bottom : other.Bottom;
        return new DocRect(left, top, right - left, bottom - top);
    }
}

/// <summary>
/// Four independent edge insets, as used for page margins, cell padding and
/// text-frame insets.
/// </summary>
/// <param name="Left">Left inset.</param>
/// <param name="Top">Top inset.</param>
/// <param name="Right">Right inset.</param>
/// <param name="Bottom">Bottom inset.</param>
public readonly record struct Margins(Length Left, Length Top, Length Right, Length Bottom)
{
    /// <summary>All edges zero.</summary>
    public static Margins Zero => default;

    /// <summary>The same inset on all four edges.</summary>
    public static Margins Uniform(Length all) => new(all, all, all, all);
}

/// <summary>
/// A 2D affine transform, laid out as the six meaningful entries of a 3x3
/// homogeneous matrix. Mirrors <c>basegfx::B2DHomMatrix</c>.
/// </summary>
/// <remarks>
/// Coordinates are transformed as
/// <c>x' = A*x + C*y + E</c>, <c>y' = B*x + D*y + F</c>,
/// matching the PDF and PostScript convention (and SkiaSharp's <c>SKMatrix</c>).
/// Translation is expressed in EMUs so that it composes with <see cref="Length"/>.
/// </remarks>
public readonly record struct AffineTransform(double A, double B, double C, double D, double E, double F)
{
    /// <summary>The identity transform.</summary>
    public static readonly AffineTransform Identity = new(1, 0, 0, 1, 0, 0);

    /// <summary>A pure translation, in EMUs.</summary>
    public static AffineTransform Translation(double dxEmu, double dyEmu) => new(1, 0, 0, 1, dxEmu, dyEmu);

    /// <summary>A pure scale about the origin.</summary>
    public static AffineTransform Scale(double sx, double sy) => new(sx, 0, 0, sy, 0, 0);

    /// <summary>A rotation about the origin, clockwise for positive angles in a y-down space.</summary>
    public static AffineTransform Rotation(double radians)
    {
        double cos = Math.Cos(radians);
        double sin = Math.Sin(radians);
        return new AffineTransform(cos, sin, -sin, cos, 0, 0);
    }

    /// <summary>True when this is (numerically) the identity transform.</summary>
    public bool IsIdentity => A == 1 && B == 0 && C == 0 && D == 1 && E == 0 && F == 0;

    /// <summary>
    /// Returns the transform equivalent to applying <paramref name="first"/> and
    /// then <paramref name="second"/>.
    /// </summary>
    public static AffineTransform Concat(AffineTransform first, AffineTransform second) => new(
        first.A * second.A + first.B * second.C,
        first.A * second.B + first.B * second.D,
        first.C * second.A + first.D * second.C,
        first.C * second.B + first.D * second.D,
        first.E * second.A + first.F * second.C + second.E,
        first.E * second.B + first.F * second.D + second.F);
}

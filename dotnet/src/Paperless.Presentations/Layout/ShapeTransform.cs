using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Presentations.Layout;

/// <summary>
/// Composes the matrix that takes a shape's own coordinates onto the slide.
/// </summary>
/// <remarks>
/// <para>
/// Ported from <c>Shape::createAndInsert</c>
/// (<c>oox/source/drawingml/shape.cxx:1098-1224</c>), which builds one cumulative matrix per
/// shape in this order: scale the unit square to the shape's extent, <em>mirror at the
/// shape's centre</em>, translate to its offset, map through the parent group's child
/// coordinate space, and only then <em>rotate at the shape's centre</em>. The two
/// centre-relative steps are <c>lcl_mirrorAtCenter</c> (<c>shape.cxx:882</c>) and
/// <c>lcl_RotateAtCenter</c> (<c>shape.cxx:910</c>).
/// </para>
/// <para>
/// <strong>Flip comes before rotation, and the order is observable.</strong> A rectangle looks
/// the same either way, which is why it is easy to get wrong and hard to notice; a shape that
/// is not symmetric does not. Measured on <c>shape-geometry.pptx</c> slide 4, whose fourth
/// shape is an <c>rtTriangle</c> at (288 pt, 216 pt) sized 144 × 72 with <c>flipH="1"</c> and
/// <c>rot="5400000"</c>: LibreOffice's PDF puts its vertices at (324, 324), (396, 324) and
/// (324, 180). Flipping first gives exactly those; rotating first gives (396, 180), (324, 180)
/// and (396, 324) — a different triangle in a different corner.
/// </para>
/// <para>
/// Everything here is in EMUs and in a y-down space, which is what
/// <see cref="AffineTransform.Rotation"/> means by a positive angle being clockwise. OOXML's
/// <c>rot</c> is clockwise too, in sixtieth-thousandths of a degree, so the conversion is a
/// scale and no sign change — unlike ODF's, which runs the other way.
/// </para>
/// </remarks>
public static class ShapeTransform
{
    /// <summary>How many units of <c>a:xfrm/@rot</c> make one degree.</summary>
    public const double RotationUnitsPerDegree = 60000.0;

    /// <summary>
    /// The matrix taking a shape's local coordinates — its own box, origin at the top left —
    /// onto the slide.
    /// </summary>
    /// <param name="bounds">The shape's offset and extent, in its parent's coordinate space.</param>
    /// <param name="rotation">Its rotation, clockwise, in radians.</param>
    /// <param name="flipHorizontal">Whether it is mirrored left to right.</param>
    /// <param name="flipVertical">Whether it is mirrored top to bottom.</param>
    /// <param name="space">
    /// The matrix taking the parent's coordinate space onto the slide; the identity at the top
    /// level.
    /// </param>
    public static AffineTransform Place(
        DocRect bounds,
        double rotation,
        bool flipHorizontal,
        bool flipVertical,
        AffineTransform space)
    {
        double halfWidth = bounds.Width.Emu / 2.0;
        double halfHeight = bounds.Height.Emu / 2.0;

        // Centred first, because both the mirror and the rotation are about the shape's own
        // centre and expressing each as its own translate-conjugate pair would compose the same
        // matrix three times over.
        AffineTransform transform = AffineTransform.Translation(-halfWidth, -halfHeight);

        if (flipHorizontal || flipVertical)
        {
            transform = AffineTransform.Concat(
                transform, AffineTransform.Scale(flipHorizontal ? -1 : 1, flipVertical ? -1 : 1));
        }

        if (rotation != 0)
        {
            transform = AffineTransform.Concat(transform, AffineTransform.Rotation(rotation));
        }

        transform = AffineTransform.Concat(
            transform,
            AffineTransform.Translation(bounds.X.Emu + halfWidth, bounds.Y.Emu + halfHeight));

        return AffineTransform.Concat(transform, space);
    }

    /// <summary>
    /// The matrix taking a group's <em>child</em> coordinate space onto the slide.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A group states two rectangles: <c>a:off</c>/<c>a:ext</c>, where it sits, and
    /// <c>a:chOff</c>/<c>a:chExt</c>, the coordinate space its children are written in. When
    /// they differ the children are scaled — which is the whole point of a group, and what makes
    /// a group's contents resize together.
    /// </para>
    /// <para>
    /// LibreOffice does this by translating away the child origin and scaling by
    /// <c>parentScale / chSize</c> (<c>shape.cxx:1174-1198</c>), where <c>parentScale</c> is the
    /// decomposed scale of the group's own cumulative matrix — so the factors cumulate through
    /// nested groups rather than being taken from the immediate parent. Composing the group's own
    /// placement on top, as this does, gives the same matrix: the group's placement <em>is</em>
    /// that decomposition, reassembled.
    /// </para>
    /// <para>
    /// A missing or zero <c>chExt</c> means a factor of one rather than a division by zero;
    /// oox permits the attribute to be absent and LibreOffice substitutes the group's own extent.
    /// </para>
    /// </remarks>
    /// <param name="bounds">The group's own offset and extent, in its parent's space.</param>
    /// <param name="childOrigin">The <c>a:chOff</c> the children are measured from.</param>
    /// <param name="childExtent">The <c>a:chExt</c> the children are measured in.</param>
    /// <param name="rotation">The group's own rotation, clockwise, in radians.</param>
    /// <param name="flipHorizontal">Whether the group is mirrored left to right.</param>
    /// <param name="flipVertical">Whether it is mirrored top to bottom.</param>
    /// <param name="space">The matrix taking the group's own parent space onto the slide.</param>
    public static AffineTransform GroupSpace(
        DocRect bounds,
        DocPoint childOrigin,
        DocSize childExtent,
        double rotation,
        bool flipHorizontal,
        bool flipVertical,
        AffineTransform space)
    {
        double scaleX = childExtent.Width.Emu != 0
            ? bounds.Width.Emu / (double)childExtent.Width.Emu
            : 1.0;
        double scaleY = childExtent.Height.Emu != 0
            ? bounds.Height.Emu / (double)childExtent.Height.Emu
            : 1.0;

        AffineTransform inner = AffineTransform.Concat(
            AffineTransform.Translation(-childOrigin.X.Emu, -childOrigin.Y.Emu),
            AffineTransform.Scale(scaleX, scaleY));

        // The group's own placement, applied to a box already at the origin — the scale above
        // has put it there — which is what Place does when the offset is folded back in.
        return AffineTransform.Concat(
            inner,
            Place(
                new DocRect(bounds.X, bounds.Y, bounds.Width, bounds.Height),
                rotation,
                flipHorizontal,
                flipVertical,
                space));
    }

    /// <summary>Converts an <c>a:xfrm/@rot</c> value to radians, clockwise.</summary>
    public static double Radians(int rotationUnits)
        => rotationUnits / RotationUnitsPerDegree * Math.PI / 180.0;

    /// <summary>
    /// The scale a placement applies to each of the two axes, rotation and mirror removed.
    /// </summary>
    /// <remarks>
    /// The length of the image of each unit basis vector, which is what
    /// <c>basegfx::B2DHomMatrix::decompose</c> extracts and what
    /// <c>Shape::createAndInsert</c> uses as the shape's own size
    /// (<c>oox/source/drawingml/shape.cxx:1129-1140</c>). A shape's placement gains a scale only
    /// from a parent group whose <c>a:chExt</c> differs from its <c>a:ext</c>, so at the top
    /// level and in an unscaled group this is (1, 1).
    /// </remarks>
    public static (double X, double Y) ScaleOf(AffineTransform transform)
        => (Math.Sqrt((transform.A * transform.A) + (transform.B * transform.B)),
            Math.Sqrt((transform.C * transform.C) + (transform.D * transform.D)));

    /// <summary>
    /// The same placement with a scale divided out of it, for coordinates already scaled by hand.
    /// </summary>
    /// <remarks>
    /// <c>WithoutScale(m, sx, sy)</c> applied to <c>(x·sx, y·sy)</c> gives what <c>m</c> gives
    /// for <c>(x, y)</c>. It exists so a measurement made in absolute units — a text rectangle,
    /// whose font sizes are absolute and cannot be scaled with it — can still travel through the
    /// matrix that positions and rotates the shape.
    /// </remarks>
    /// <param name="transform">The placement to divide.</param>
    /// <param name="scaleX">The horizontal factor to remove; a zero leaves the axis alone.</param>
    /// <param name="scaleY">The vertical factor to remove; a zero leaves the axis alone.</param>
    public static AffineTransform WithoutScale(
        AffineTransform transform, double scaleX, double scaleY)
    {
        if (scaleX is 0 or 1 && scaleY is 0 or 1) return transform;

        double x = scaleX == 0 ? 1 : scaleX;
        double y = scaleY == 0 ? 1 : scaleY;

        return new AffineTransform(
            transform.A / x, transform.B / x,
            transform.C / y, transform.D / y,
            transform.E, transform.F);
    }

    /// <summary>A rectangle with both its offset and its extent multiplied.</summary>
    public static DocRect Scaled(DocRect rectangle, double scaleX, double scaleY)
        => scaleX == 1 && scaleY == 1
            ? rectangle
            : new DocRect(
                Length.FromEmu((long)Math.Round(rectangle.X.Emu * scaleX)),
                Length.FromEmu((long)Math.Round(rectangle.Y.Emu * scaleY)),
                Length.FromEmu((long)Math.Round(rectangle.Width.Emu * scaleX)),
                Length.FromEmu((long)Math.Round(rectangle.Height.Emu * scaleY)));

    /// <summary>
    /// The rectangle a shape occupies on the slide <em>before</em> its own rotation.
    /// </summary>
    /// <remarks>
    /// Not the outline's bounding box, which for a rotated shape is larger than the shape — and
    /// not the local rectangle either, since a group can scale it. Taken as the placed centre
    /// plus the lengths the transform gives the box's two edge vectors, so a rotation leaves it
    /// alone and a scale does not. This is what text layout is measured against and what a
    /// placement comparison asserts on.
    /// </remarks>
    /// <param name="placement">The matrix taking the shape's local box onto the slide.</param>
    /// <param name="size">The local box's extent.</param>
    public static DocRect PlacedBounds(AffineTransform placement, DocSize size)
    {
        DocPoint centre = Apply(
            placement,
            new DocPoint(
                Length.FromEmu(size.Width.Emu / 2), Length.FromEmu(size.Height.Emu / 2)));

        double width = size.Width.Emu * Math.Sqrt((placement.A * placement.A) + (placement.B * placement.B));
        double height = size.Height.Emu * Math.Sqrt((placement.C * placement.C) + (placement.D * placement.D));

        return new DocRect(
            Length.FromEmu(centre.X.Emu - (long)Math.Round(width / 2)),
            Length.FromEmu(centre.Y.Emu - (long)Math.Round(height / 2)),
            Length.FromEmu((long)Math.Round(width)),
            Length.FromEmu((long)Math.Round(height)));
    }

    /// <summary>Applies a transform to a point.</summary>
    public static DocPoint Apply(AffineTransform transform, DocPoint point)
    {
        double x = point.X.Emu;
        double y = point.Y.Emu;

        return new DocPoint(
            Length.FromEmu((long)Math.Round((transform.A * x) + (transform.C * y) + transform.E)),
            Length.FromEmu((long)Math.Round((transform.B * x) + (transform.D * y) + transform.F)));
    }

    /// <summary>Applies a transform to every point of a path, producing a new one.</summary>
    /// <remarks>
    /// Point by point rather than by handing the matrix to a backend, because an affine map takes
    /// a cubic Bezier's control points to the control points of the mapped curve exactly — so a
    /// transformed path is still the same shape, and every consumer sees slide coordinates
    /// without composing anything.
    /// </remarks>
    public static GraphicsPath Apply(AffineTransform transform, GraphicsPath path)
    {
        ArgumentNullException.ThrowIfNull(path);
        if (transform.IsIdentity) return path;

        GraphicsPath mapped = new();
        foreach (PathCommand command in path.Commands)
        {
            switch (command.Verb)
            {
                case PathVerb.MoveTo:
                    mapped.MoveTo(Apply(transform, command.Point));
                    break;
                case PathVerb.LineTo:
                    mapped.LineTo(Apply(transform, command.Point));
                    break;
                case PathVerb.CubicTo:
                    mapped.CubicTo(
                        Apply(transform, command.Control1),
                        Apply(transform, command.Control2),
                        Apply(transform, command.Point));
                    break;
                default:
                    mapped.Close();
                    break;
            }
        }

        return mapped;
    }
}

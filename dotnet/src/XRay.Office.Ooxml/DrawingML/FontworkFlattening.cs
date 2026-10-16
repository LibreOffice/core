namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// Turns a cubic segment into the polyline the draw layer turns it into.
/// </summary>
/// <remarks>
/// <para>
/// A port of <c>basegfx::utils::adaptiveSubdivideByAngle</c> and the two recursions behind it —
/// <c>ImpSubDivAngleStart</c> and <c>ImpSubDivAngle</c>,
/// <c>basegfx/source/curve/b2dcubicbezier.cxx:44-255</c> — at the 2.25 degree bound that function
/// substitutes when a caller states none (<c>ANGLE_BOUND_START_VALUE</c>,
/// <c>basegfx/source/polygon/b2dpolygontools.cxx:41</c>).
/// </para>
/// <para>
/// <strong>The obvious simplification of it is wrong on exactly the curves this feature needs.</strong>
/// The criterion is the angle between the vector from the start point to its control point and the
/// vector from the <em>end</em> point to <em>its</em> control point — near-opposite means straight —
/// and applying it to a whole segment declares a symmetric S-curve straight, because an S-curve's
/// two end tangents point exactly opposite ways. Every WordArt wave is such an S. That is why
/// <c>ImpSubDivAngleStart</c> splits once at the midpoint before it tests anything, and why this
/// does too: with the shortcut the wave rails come out as two straight lines and the warp is a
/// slanted rectangle.
/// </para>
/// </remarks>
internal static class FontworkFlattening
{
    /// <summary>The angle bound a caller that states none gets, in degrees.</summary>
    private const double AngleBoundDegrees = 2.25;

    /// <summary>How much the bound is relaxed at each level of recursion.</summary>
    /// <remarks><c>FACTOR_FOR_UNSHARPEN</c>, <c>b2dcubicbezier.cxx:33</c>.</remarks>
    private const double Unsharpen = 1.6;

    /// <summary>The recursion limit, which is also the finest subdivision.</summary>
    private const int MaximumDepth = 8;

    /// <summary>The tolerance <c>basegfx</c> compares two doubles with.</summary>
    private const double Epsilon = 0.00001;

    /// <summary>
    /// Appends the flattening of one cubic segment; the start point is assumed already there.
    /// </summary>
    public static void Append(
        FontworkPoint start,
        FontworkPoint control1,
        FontworkPoint control2,
        FontworkPoint end,
        List<FontworkPoint> into)
    {
        double bound = AngleBoundDegrees * Math.PI / 180.0;
        int depth = MaximumDepth;

        (double leftX, double leftY) = (control1.X - start.X, control1.Y - start.Y);
        (double rightX, double rightY) = (control2.X - end.X, control2.Y - end.Y);
        bool leftZero = IsZero(leftX, leftY);
        bool rightZero = IsZero(rightX, rightY);
        bool allParallel = false;

        if (leftZero && rightZero)
        {
            depth = 0;
        }
        else
        {
            double baseX = end.X - start.X;
            double baseY = end.Y - start.Y;

            if (!IsZero(baseX, baseY))
            {
                bool leftParallel = leftZero || AreParallel(leftX, leftY, baseX, baseY);
                bool rightParallel = rightZero || AreParallel(rightX, rightY, baseX, baseY);

                if (leftParallel && rightParallel)
                {
                    allParallel = true;

                    if (!leftZero)
                    {
                        double factor = Math.Abs(baseX) > Math.Abs(baseY)
                            ? leftX / baseX
                            : leftY / baseY;

                        if (factor is >= 0.0 and <= 1.0) leftZero = true;
                    }

                    if (!rightZero)
                    {
                        double factor = Math.Abs(baseX) > Math.Abs(baseY)
                            ? rightX / -baseX
                            : rightY / -baseY;

                        if (factor is >= 0.0 and <= 1.0) rightZero = true;
                    }

                    if (leftZero && rightZero) depth = 0;
                }
            }
        }

        if (depth > 0)
        {
            FontworkPoint s1l = Middle(start, control1);
            FontworkPoint s1c = Middle(control1, control2);
            FontworkPoint s1r = Middle(control2, end);
            FontworkPoint s2l = Middle(s1l, s1c);
            FontworkPoint s2r = Middle(s1c, s1r);
            FontworkPoint s3c = Middle(s2l, s2r);

            bool smallLeft = allParallel && leftZero;
            if (!smallLeft)
            {
                FontworkPoint a = leftZero ? Minus(s2l, s1l) : Minus(s1l, start);
                FontworkPoint b = Minus(s2l, s3c);
                smallLeft = Math.Abs(Angle(a, b)) > Math.PI - bound;
            }

            bool smallRight = allParallel && rightZero;
            if (!smallRight)
            {
                FontworkPoint a = Minus(s2r, s3c);
                FontworkPoint b = rightZero ? Minus(s2r, s1r) : Minus(s1r, end);
                smallRight = Math.Abs(Angle(a, b)) > Math.PI - bound;
            }

            if (smallLeft && smallRight)
            {
                depth = 0;
            }
            else
            {
                if (smallLeft) into.Add(s3c);
                else Recurse(start, s1l, s2l, s3c, into, bound, depth);

                if (smallRight) into.Add(end);
                else Recurse(s3c, s2r, s1r, end, into, bound, depth);
            }
        }

        if (depth == 0) into.Add(end);
    }

    /// <summary><c>ImpSubDivAngle</c>: test, then bisect, relaxing the bound as it goes.</summary>
    private static void Recurse(
        FontworkPoint start,
        FontworkPoint control1,
        FontworkPoint control2,
        FontworkPoint end,
        List<FontworkPoint> into,
        double bound,
        int depth)
    {
        if (depth > 0)
        {
            FontworkPoint left = Minus(control1, start);
            FontworkPoint right = Minus(control2, end);

            if (IsZero(left.X, left.Y)) left = Minus(control2, start);
            if (IsZero(right.X, right.Y)) right = Minus(control1, end);

            if (Math.Abs(Angle(left, right)) > Math.PI - bound) depth = 0;
            else bound *= Unsharpen;
        }

        if (depth == 0)
        {
            into.Add(end);
            return;
        }

        FontworkPoint s1l = Middle(start, control1);
        FontworkPoint s1c = Middle(control1, control2);
        FontworkPoint s1r = Middle(control2, end);
        FontworkPoint s2l = Middle(s1l, s1c);
        FontworkPoint s2r = Middle(s1c, s1r);
        FontworkPoint s3c = Middle(s2l, s2r);

        Recurse(start, s1l, s2l, s3c, into, bound, depth - 1);
        Recurse(s3c, s2r, s1r, end, into, bound, depth - 1);
    }

    private static FontworkPoint Middle(FontworkPoint a, FontworkPoint b)
        => new((a.X + b.X) / 2, (a.Y + b.Y) / 2);

    private static FontworkPoint Minus(FontworkPoint a, FontworkPoint b)
        => new(a.X - b.X, a.Y - b.Y);

    /// <summary>The signed angle from one vector to another, as <c>B2DVector::angle</c> gives it.</summary>
    private static double Angle(FontworkPoint a, FontworkPoint b)
        => Math.Atan2((a.X * b.Y) - (a.Y * b.X), (a.X * b.X) + (a.Y * b.Y));

    private static bool IsZero(double x, double y)
        => Math.Abs(x) <= Epsilon && Math.Abs(y) <= Epsilon;

    private static bool AreParallel(double ax, double ay, double bx, double by)
        => Math.Abs((ax * by) - (ay * bx)) <= Epsilon;
}

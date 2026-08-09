using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.Core.Charts;

/// <summary>How an axis arranges labels that would otherwise collide.</summary>
/// <remarks>
/// <c>AxisLabelStaggering</c> (<c>chart2/source/view/axes/VAxisProperties.hxx:47-53</c>), reached
/// from the axis' <c>ArrangeOrder</c> property. OOXML's importer sets it to
/// <see cref="SideBySide"/> unconditionally — "do not stagger labels in two lines",
/// <c>oox/source/drawingml/chart/axisconverter.cxx:366</c> — and ODF leaves it at chart2's own
/// default of <see cref="Auto"/>.
/// </remarks>
public enum ChartLabelStagger
{
    /// <summary>Stagger onto two rows only if one row collides.</summary>
    Auto = 0,

    /// <summary>Never stagger.</summary>
    SideBySide,

    /// <summary>Push the even-numbered labels onto the second row.</summary>
    Even,

    /// <summary>Push the odd-numbered labels onto the second row.</summary>
    Odd,
}

/// <summary>
/// What an axis' file says about how its labels are set, before any of it is resolved.
/// </summary>
/// <param name="Rotation">
/// The stated rotation in radians, anticlockwise. Zero is the overwhelming case and is also what
/// an out-of-range OOXML <c>rot</c> reads as — see <see cref="ChartAxisLabels"/>.
/// </param>
/// <param name="OverlapAllowed">
/// Whether labels may be drawn on top of one another — the axis' <c>TextOverlap</c>. When true
/// nothing below is done at all: every label is drawn where it falls.
/// </param>
/// <param name="LineBreakAllowed">
/// Whether a label may wrap onto a second line — <c>TextBreak</c>. It defaults to false in
/// chart2's own model and true in OOXML's importer, and the difference decides which of two
/// quite different paths a colliding axis takes.
/// </param>
/// <param name="Stagger">How the axis arranges colliding labels.</param>
public readonly record struct ChartAxisText(
    double Rotation = 0.0,
    bool OverlapAllowed = false,
    bool LineBreakAllowed = false,
    ChartLabelStagger Stagger = ChartLabelStagger.Auto);

/// <summary>
/// How an axis' labels came out: rotated, thinned, staggered, and how much room they take.
/// </summary>
/// <param name="Rotation">The rotation to draw them at, in radians anticlockwise.</param>
/// <param name="Rhythm">Draw every <em>n</em>th label; 1 draws all of them.</param>
/// <param name="Staggered">Whether alternate labels go onto a second row.</param>
/// <param name="Reserved">
/// How deep the labels are, measured away from the axis — which is what the plot rectangle has to
/// give up for them.
/// </param>
public readonly record struct ChartAxisLabelLayout(
    double Rotation,
    int Rhythm,
    bool Staggered,
    Length Reserved);

/// <summary>
/// Decides whether an axis' labels are rotated, thinned or staggered, and how deep they end up.
/// </summary>
/// <remarks>
/// <para>
/// A port of <c>VCartesianAxis::createTextShapes</c> and <c>createTextShapesSimple</c>
/// (<c>chart2/source/view/axes/VCartesianAxis.cxx:703-1090</c>), which are one function written
/// twice: lay every label out, and the moment two of them collide, change one thing about the
/// arrangement and <em>start over</em>. The loop is the algorithm — each attempt returns false and
/// the caller re-runs it (<c>createLabels</c>, <c>:1753</c>, is literally
/// <c>while (!createTextShapes(…)) {}</c>) — so this reproduces the restart rather than trying to
/// compute the answer in one pass.
/// </para>
/// <para>
/// <strong>Two of the four escape routes are dead in modern LibreOffice, and knowing which saves
/// reproducing them.</strong> Auto-staggering has the same prerequisites as auto-rotation
/// (<c>canAutoAdjustLabelPlacement</c>, <c>:1478</c>), and the second collision test prefers
/// rotation to staggering unless the document asks otherwise — "starting from LibreOffice 5.1 the
/// rotated layout is preferred to staggering for axis labels" (<c>:914-917</c>). The first
/// collision test does try staggering first, but it asks whether the next <em>tick</em> falls
/// inside the previous label's box, and for a horizontal axis the labels sit a tick length below
/// the ticks, so it never fires. What is left is: rotate to 45°, then thin the labels out.
/// Staggering therefore only happens when the file states it outright, which ODF can and OOXML
/// cannot.
/// </para>
/// <para>
/// <strong>Two decks in the reference corpus turned out to measure the PDF writer rather than the
/// layout, and both look like layout bugs.</strong> <c>tdf106217.pptx</c> extracts <em>no</em>
/// category names from LibreOffice's own PDF and draws all eight rotated in the picture — they are
/// written as glyph <em>outlines</em>, which <c>pdftotext</c> cannot read, because the chart is
/// scaled unequally and a 45° rotation on top of that is a shear the PDF text state does not
/// carry. <c>bnc889755.pptx</c> extracts 89 words for 16 month names, because LibreOffice emits
/// one <c>Tj</c> per glyph for rotated text and <c>pdftotext</c> splits at every one. Neither
/// number is reachable by drawing the labels correctly, and both were read as missing features
/// before the content streams were opened.
/// </para>
/// </remarks>
public static class ChartAxisLabels
{
    /// <summary>The angle auto-rotation settles on.</summary>
    /// <remarks>
    /// <c>AxisLabelProperties::autoRotate45</c> (<c>VAxisProperties.cxx:403-408</c>) sets exactly
    /// 45°, disallows line breaking and drops back to side-by-side, all three at once — so the
    /// rotated arrangement is final and the loop cannot come back and stagger it.
    /// </remarks>
    private const double AutoRotation = Math.PI / 4.0;

    /// <summary>How many restarts are tolerated before the arrangement is taken as it is.</summary>
    /// <remarks>
    /// The C++ loop is unbounded because each restart strictly narrows the arrangement and it
    /// always terminates; a bound here costs nothing and makes a pathological axis — one label per
    /// pixel — finish rather than spin.
    /// </remarks>
    private const int MaximumAttempts = 64;

    /// <summary>
    /// The proportion of the space between two ticks a wrapping label is given.
    /// </summary>
    /// <remarks>
    /// <c>createTextShapes</c> reduces the limit by 5% "to have a visible distance between the
    /// labels" (<c>VCartesianAxis.cxx:753-759</c>).
    /// </remarks>
    private const double BreakingWidth = 0.95;

    /// <summary>
    /// Resolves how a horizontal axis' labels are arranged.
    /// </summary>
    /// <param name="texts">The labels, in order; null or empty entries are not drawn.</param>
    /// <param name="centres">Where each label's tick is along the axis, in order.</param>
    /// <param name="stated">What the file says about the axis' text.</param>
    /// <param name="size">The label's em size.</param>
    /// <param name="measurer">Measures a line of text.</param>
    /// <param name="bold">
    /// Whether the labels are set in the family's bold face. It reaches the arrangement and not
    /// only the drawing because a bold face is wider, so it decides whether two labels collide —
    /// and hence whether the axis is rotated at all. See <see cref="ChartPlot.IsLabelBold"/>.
    /// </param>
    public static ChartAxisLabelLayout Resolve(
        IReadOnlyList<string?> texts,
        IReadOnlyList<Length> centres,
        ChartAxisText stated,
        Length size,
        ChartText measurer,
        bool bold = false)
    {
        ArgumentNullException.ThrowIfNull(texts);
        ArgumentNullException.ThrowIfNull(centres);

        double rotation = stated.Rotation;
        bool lineBreak = stated.LineBreakAllowed;
        ChartLabelStagger stagger = stated.Stagger;
        int rhythm = 1;

        int count = Math.Min(texts.Count, centres.Count);
        if (count == 0) return new ChartAxisLabelLayout(rotation, 1, false, Length.Zero);

        // The shape each label sits in, insets included — what collides is the shape and not the
        // text, exactly as everything else this file reserves room for.
        DocSize[] boxes = new DocSize[count];
        for (int at = 0; at < count; at++)
        {
            boxes[at] = texts[at] is { Length: > 0 } text
                ? Shape(measurer, text, size, bold)
                : default;
        }

        Length spacing = Spacing(centres, count);

        for (int attempt = 0; attempt < MaximumAttempts; attempt++)
        {
            bool staggered = stagger is ChartLabelStagger.Even or ChartLabelStagger.Odd;

            // A label that wraps in the room between two ticks makes chart2 give up on wrapping
            // altogether and start again (VCartesianAxis.cxx:888-903). That restart is what lets
            // auto-rotation happen at all on an OOXML axis, whose importer turns line breaking
            // *on*: canAutoAdjustLabelPlacement refuses while it is on, so the wrap is the only
            // route from "labels collide" to "labels are turned 45°".
            if (lineBreak && !stated.OverlapAllowed && rotation == 0.0
                && Wraps(texts, count, spacing, staggered, size, measurer, bold))
            {
                lineBreak = false;
                continue;
            }

            // canAutoAdjustLabelPlacement, VCartesianAxis.cxx:1478-1495 — and it deliberately
            // does *not* test the arrangement, which is why an OOXML axis whose staggering is
            // turned off may still be rotated.
            bool canAdjust = !stated.OverlapAllowed && !lineBreak && rotation == 0.0;

            if (stated.OverlapAllowed
                || !Collides(boxes, centres, count, rhythm, staggered, rotation))
            {
                return new ChartAxisLabelLayout(
                    rotation, rhythm, staggered,
                    Depth(boxes, count, rhythm, staggered, rotation));
            }

            if (canAdjust)
            {
                rotation = AutoRotation;
                lineBreak = false;
                stagger = ChartLabelStagger.SideBySide;
                rhythm = 1;
                continue;
            }

            rhythm++;
        }

        return new ChartAxisLabelLayout(
            rotation, rhythm, stagger is ChartLabelStagger.Even or ChartLabelStagger.Odd,
            Depth(boxes, count, rhythm, false, rotation));
    }

    /// <summary>The smallest gap between two consecutive ticks.</summary>
    /// <remarks>
    /// <c>TickFactory2D::getTickScreenDistance</c> takes the first two, which are the same as any
    /// other pair on an evenly divided axis and smaller than some on a date axis with irregular
    /// steps; the smallest is the one a collision would be found at.
    /// </remarks>
    private static Length Spacing(IReadOnlyList<Length> centres, int count)
    {
        Length smallest = Length.Zero;

        for (int at = 1; at < count; at++)
        {
            Length gap = centres[at] - centres[at - 1];
            if (gap < Length.Zero) gap = Length.Zero - gap;
            if (gap <= Length.Zero) continue;
            if (smallest <= Length.Zero || gap < smallest) smallest = gap;
        }

        return smallest;
    }

    /// <summary>Whether any label would wrap in the room one tick's worth of axis gives it.</summary>
    private static bool Wraps(
        IReadOnlyList<string?> texts,
        int count,
        Length spacing,
        bool staggered,
        Length size,
        ChartText measurer,
        bool bold)
    {
        if (spacing <= Length.Zero) return false;

        Length limit = spacing * (staggered ? 2.0 : 1.0) * BreakingWidth
                       - size * (TextShapeInsetX * 2);

        if (limit <= Length.Zero) return false;

        for (int at = 0; at < count; at++)
        {
            if (texts[at] is not { Length: > 0 } text) continue;

            foreach (string word in Words(text))
                if (measurer.Measure(word, size, bold).Width > limit) return true;
        }

        return false;
    }

    /// <summary>
    /// The runs a label would be broken into at word boundaries.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The trigger is a break <em>inside</em> a word, not a second line.</strong>
    /// <c>lcl_hasWordBreak</c> (<c>VCartesianAxis.cxx:369-404</c>) walks the laid-out shape's
    /// lines and returns true only where a line <em>starts in the middle of a word</em> —
    /// <c>GetWordIndices(…) &amp;&amp; nWordStart != nLineStart</c>. A label that merely wraps at
    /// a space or a hyphen does not count, because the second line then begins exactly where a
    /// word does.
    /// </para>
    /// <para>
    /// That distinction is the whole of it: <c>tdf106217.pptx</c>'s "Netherlands" is one word
    /// wider than its slot and can only be broken mid-word, so it triggers; a label like
    /// <c>Oct-12</c> breaks cleanly after the hyphen and does not. Testing "does the label fit"
    /// instead of "does any of its words fit" fires on both, and then every crowded category axis
    /// in the corpus rotates whether the reference does or not.
    /// </para>
    /// </remarks>
    private static IEnumerable<string> Words(string text)
    {
        int start = 0;

        for (int at = 0; at < text.Length; at++)
        {
            if (text[at] is not (' ' or '\t' or '-' or '‐' or '/')) continue;

            if (at > start) yield return text[start..(at + 1)];
            start = at + 1;
        }

        if (start < text.Length) yield return text[start..];
    }

    /// <summary>Whether any two labels that are actually drawn would overlap.</summary>
    /// <remarks>
    /// <c>doesOverlap</c> (<c>VCartesianAxis.cxx:186-207</c>) intersects the two labels' rotated
    /// rectangles. Both carry the same rotation, so the intersection reduces exactly to the
    /// separating-axis test in the rectangles' own frame: turn the offset between their centres
    /// back through the rotation and compare it against the two boxes' half-extents. Comparing
    /// axis-aligned bounding boxes instead — which is the obvious shortcut — is what makes 45°
    /// look like it does not help: on <c>bnc889755.pptx</c>'s 36.5 pt tick spacing the bounding
    /// box of a rotated 38 × 21 pt label is 41.7 pt wide and still collides, while the rotated
    /// rectangles are 25.8 pt apart along their own short axis against a half-height of 21 and do
    /// not.
    /// </remarks>
    private static bool Collides(
        DocSize[] boxes,
        IReadOnlyList<Length> centres,
        int count,
        int rhythm,
        bool staggered,
        double rotation)
    {
        // Staggering puts alternate labels on two rows, so what a label collides with is the one
        // two places away rather than the one beside it.
        int step = Math.Max(1, rhythm) * (staggered ? 2 : 1);

        double cosine = Math.Abs(Math.Cos(rotation));
        double sine = Math.Abs(Math.Sin(rotation));

        int previous = -1;

        for (int at = 0; at < count; at++)
        {
            if (at % Math.Max(1, rhythm) != 0) continue;
            if (boxes[at].Width <= Length.Zero) continue;

            if (previous >= 0 && at - previous >= step)
            {
                double gap = Math.Abs((centres[at] - centres[previous]).Emu);
                double along = gap * cosine;
                double across = gap * sine;

                double width = (boxes[at].Width.Emu + boxes[previous].Width.Emu) / 2.0;
                double height = (boxes[at].Height.Emu + boxes[previous].Height.Emu) / 2.0;

                if (along < width && across < height) return true;
            }

            if (previous < 0 || at - previous >= step) previous = at;
        }

        return false;
    }

    /// <summary>How deep the labels reach away from the axis.</summary>
    /// <remarks>
    /// <c>ShapeFactory::getSizeAfterRotation</c> (<c>ShapeFactory.cxx:2511-2539</c>):
    /// <c>W·sin θ + H·cos θ</c> for the height of a rotated shape, with the angle folded into the
    /// first quadrant first. Staggering doubles it, because the second row sits below the first.
    /// </remarks>
    private static Length Depth(
        DocSize[] boxes, int count, int rhythm, bool staggered, double rotation)
    {
        double cosine = Math.Abs(Math.Cos(rotation));
        double sine = Math.Abs(Math.Sin(rotation));

        Length deepest = Length.Zero;

        for (int at = 0; at < count; at++)
        {
            if (at % Math.Max(1, rhythm) != 0) continue;
            if (boxes[at].Width <= Length.Zero) continue;

            Length depth = boxes[at].Width * sine + boxes[at].Height * cosine;
            if (depth > deepest) deepest = depth;
        }

        return staggered ? deepest * 2.0 : deepest;
    }

    /// <summary>See <c>ChartLayout</c>'s constant of the same name.</summary>
    private const double TextShapeInsetX = 0.18;

    /// <summary>See <c>ChartLayout</c>'s constant of the same name.</summary>
    private const double TextShapeInsetY = 0.30;

    /// <summary>The shape a piece of chart text is drawn in, insets included.</summary>
    private static DocSize Shape(ChartText measurer, string text, Length size, bool bold = false)
    {
        DocSize measured = measurer.Measure(text, size, bold);
        return new DocSize(
            measured.Width + size * (TextShapeInsetX * 2),
            measured.Height + size * (TextShapeInsetY * 2));
    }
}

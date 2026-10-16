using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Core.Charts;

/// <summary>
/// A pie chart's data labels: the legend key, the best-fit placement and the diagram shrink that
/// falls out of it.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Every law in this file was read off a reference rendering before the C++ was opened.</strong>
/// <c>dotnet/probes/sheets-r59/probe-pieradius.py</c> renders sixteen one-variable rewrites of
/// <c>003_advanced_excel_pie</c>'s own chart part through the installed 26.2.4.2 and reads the pie's
/// centre and radius back out of the first wedge, whose bounding box's lower-left corner is the pie
/// centre exactly. It says:
/// </para>
/// <list type="bullet">
/// <item><description>
/// <c>dLblPos="ctr"</c> and <c>"inEnd"</c> draw the pie at radius <strong>110.44</strong> — which is
/// what this layout already produced, to 0.16%. Our geometry was never wrong; it was right for the
/// wrong placement.
/// </description></item>
/// <item><description>
/// <c>"bestFit"</c> and <c>"outEnd"</c> draw it at <strong>99.78</strong>, centre
/// (408.84, 464.74) — <em>identical to each other, to the digit</em>.
/// </description></item>
/// <item><description>
/// The shrink is not a constant. Short label text (category only, value only) leaves the radius at
/// 110.44; 8 pt labels give 101.28 and 16 pt give 72.08. It is driven by what the labels consume.
/// </description></item>
/// </list>
/// <para>
/// The mechanism, corroborated afterwards in <c>PieChart.cxx</c> and <c>ChartView.cxx</c>:
/// <c>bestFit</c> is <c>AVOID_OVERLAP</c>, which <c>createTextLabelShape</c> draws as <c>CENTER</c>
/// and then hands to <c>performLabelBestFitInnerPlacement</c>; a label that will not fit inside its
/// own slice is <em>rebuilt</em> at the <c>OUTSIDE</c> anchor with a different wrapping width, and
/// the pie/donut branch of <c>impl_createDiagramAndContent</c> then recreates the whole diagram at
/// <c>VDiagram::adjustInnerSize(consumedOuterRect)</c>.
/// </para>
/// <para>
/// <strong>The reference draws six label keys for five labels</strong>, and that oddity is what
/// identifies the mechanism from outside the binary: the outside fallback does
/// <c>xShapes-&gt;remove(xTextShape)</c>, which takes the text away and leaves the key of the
/// discarded inner attempt behind. It is reproduced here for the same reason it happens there —
/// the inner attempt is a real shape — and <c>PieLabelKeepsTheDiscardedInnerKey</c> pins it.
/// </para>
/// </remarks>
public static partial class ChartLayout
{
    /// <summary>The radial offset added to an <c>OUTSIDE</c> pie label's anchor.</summary>
    /// <remarks>
    /// A flat <c>150</c> hundredths of a millimetre — <c>nScreenValueOffsetInRadiusDirection</c>
    /// in <c>PieChart::createTextLabelShape</c>, under the comment "this value should depend on
    /// the font height" and not depending on it. Predicted x 462.87 for
    /// <c>003_advanced_excel_pie</c>'s first label against a measured 462.90.
    /// </remarks>
    private static readonly Length OutsideLabelOffset = Length.FromMm100(150);

    /// <summary>The smallest gap between a label's legend key and its text.</summary>
    /// <remarks><c>std::max(100.0, fViewFontSize * 0.22)</c>, "minimum 1mm".</remarks>
    private static readonly Length LabelKeyGapFloor = Length.FromMm100(100);

    /// <summary>A label's legend key, as a fraction of the font height.</summary>
    private const double LabelKeyHeight = 0.6;

    /// <summary>The gap between a label's legend key and its text, as a fraction of the font height.</summary>
    private const double LabelKeyGap = 0.22;

    /// <summary>How far a best-fit label's box is kept off the slice's rim.</summary>
    /// <remarks><c>fPieBorderOffset = 0.025</c> in <c>performLabelBestFitInnerPlacement</c>.</remarks>
    private const double PieBorderOffset = 0.025;

    /// <summary>A best-fit label's wrapping width, as a fraction of the pie's radius.</summary>
    /// <remarks>
    /// "A reasonable start for bestFitting a 90deg slice oriented on an Axis is 80% of the radius"
    /// — <c>PieChart::createTextLabelShape</c>. It is what makes the corpus witness's five labels
    /// come out as they do: at the shrunk radius the allowance is 79.8 pt, the four twenty-glyph
    /// labels are wider than that and wrap onto two lines, and the nineteen-glyph one is not and
    /// does not. The reference draws exactly that — one unwrapped label and four wrapped.
    /// </remarks>
    private const double BestFitWidthFraction = 0.8;

    /// <summary>An outside label's wrapping width, as a share of the diagram's available width.</summary>
    /// <remarks>
    /// "Based on observation, Microsoft uses 1/5 of the chart space as its text limit" — the cap
    /// applied on top of the 80%-of-the-room-to-the-edge allowance.
    /// </remarks>
    private const double CompatWidthFraction = 0.2;

    /// <summary>Where one pie data label ends up, and what it is made of.</summary>
    /// <param name="Lines">Its text, already wrapped.</param>
    /// <param name="Block">
    /// The whole label's rectangle — key, gap and text — which is what the best-fit test measures
    /// and what the diagram shrink adds up. Not the text's rectangle: those differ by the key.
    /// </param>
    /// <param name="Key">The legend key's square, or null when the label states none.</param>
    /// <param name="KeyFill">The key's colour, which is the point's own fill.</param>
    /// <param name="GhostKey">
    /// The key of a discarded inner attempt, which the reference leaves on the page. See the type
    /// remarks.
    /// </param>
    private readonly record struct PiePlacedLabel(
        string[] Lines,
        DocRect Block,
        DocRect? Key,
        Colour? KeyFill,
        DocRect? GhostKey);

    /// <summary>
    /// The label layout of one pie, in the caller's coordinates.
    /// </summary>
    /// <param name="plot">The chart.</param>
    /// <param name="series">The series whose points are labelled.</param>
    /// <param name="centre">The pie's centre.</param>
    /// <param name="radius">The pie's outer radius.</param>
    /// <param name="available">
    /// The diagram's available rectangle — <c>m_aAvailableOuterRect</c>, which an outside label's
    /// wrapping width is measured against.
    /// </param>
    /// <param name="measurer">Measures a line.</param>
    /// <param name="verticalRadius">
    /// The rim's vertical semi-axis. It is <paramref name="radius"/> for a flat pie and the
    /// squashed one for a three-dimensional pie, whose rim is an ellipse — see
    /// <c>ChartLayout.Pie3D</c>. Only the anchors take it: the wrapping allowance and the
    /// best-fit test are horizontal measures, and the reference's own wrapping width comes from
    /// the rim's <em>x</em> alone (<c>nOuterX</c>).
    /// </param>
    /// <param name="startDegrees">
    /// Where the first wedge opens, counter-clockwise from three o'clock. Twelve o'clock for an
    /// ordinary pie; an of-pie's main ring opens at minus half its composite wedge instead, so
    /// that the composite closes across three o'clock — <c>createOneRing</c>'s <c>sAngle</c>
    /// (<c>PieChart.cxx</c>:1229-1244).
    /// </param>
    /// <param name="limit">
    /// How many of the series' points the ring holds, when it does not hold all of them. An
    /// of-pie's main ring holds the first <c>n − splitPos</c> and then a composite wedge that
    /// <c>createOneRing</c> deliberately gives no label (<c>:1346-1350</c>), so the labels stop
    /// there while the shares stay fractions of the whole series.
    /// </param>
    private static List<PiePlacedLabel> PieLabels(
        ChartPlot plot,
        ChartSeries series,
        DocPoint centre,
        Length radius,
        Length verticalRadius,
        DocRect available,
        ChartText measurer,
        double startDegrees = 90.0,
        int? limit = null)
    {
        List<PiePlacedLabel> placed = [];

        double total = series.Total();
        if (!(total > 0.0) || radius <= Length.Zero) return placed;

        Length size = plot.DataLabelFont;
        bool bold = plot.DataLabelBold;

        // int(fontHeight × 0.6) and max(100, fontHeight × 0.22), both truncated in hundredths of a
        // millimetre exactly as VSeriesPlotter does — 5.98 pt and 8.818 pt at 10 pt, measured.
        Length key = Length.FromMm100((int)(size.Mm100 * LabelKeyHeight));
        Length keyGap = key + Length.FromMm100((int)Math.Max(
            LabelKeyGapFloor.Mm100, size.Mm100 * LabelKeyGap));

        double start = startDegrees;
        int last = Math.Min(limit ?? series.Values.Count, series.Values.Count);

        for (int at = 0; at < last; at++)
        {
            if (series.Values[at] is not { } value || !double.IsFinite(value)) continue;

            double sweep = Math.Abs(value) / total * 360.0;
            if (sweep <= 0.0) continue;

            double bisector = start - (sweep / 2);
            start -= sweep;

            if (series.LabelAt(at) is not { Draws: true } label) continue;

            string? text = label.Compose(
                CategoryTextAt(plot, at), series.Name, value, total);

            if (text is not { Length: > 0 }) continue;

            ChartLabelPlacement placement = label.Placement ?? ChartLabelPlacement.BestFit;
            Length gap = label.ShowLegendKey ? keyGap : Length.Zero;

            // The inner attempt, which is also the whole of the CENTER case: wrapped at 80% of the
            // radius and centred on the middle of the ring.
            string[] lines = LinesOf(
                measurer, text, size, bold, radius * BestFitWidthFraction);
            DocSize block = BlockOf(measurer, lines, size, bold, gap);

            DocPoint at05 = new(
                centre.X + (radius * 0.5 * Math.Cos(Radians(bisector))),
                centre.Y - (verticalRadius * 0.5 * Math.Sin(Radians(bisector))));

            // CENTER and everything that is not INSIDE or OUTSIDE take the middle of the ring
            // and LABEL_ALIGN_CENTER: `bCenter` is exactly
            // `nLabelPlacement != OUTSIDE && != INSIDE`
            // (PolarLabelPositionHelper.cxx:68-76), and AVOID_OVERLAP has already been turned
            // into CENTER by the time it is asked (PieChart.cxx:434-437).
            if (placement is not (ChartLabelPlacement.BestFit
                                  or ChartLabelPlacement.Outside
                                  or ChartLabelPlacement.Inside))
            {
                placed.Add(Assemble(lines, Centred(at05, block), block, gap, key,
                                    label.ShowLegendKey ? series.FillAt(at) : null, null));
                continue;
            }

            DocRect? ghost = null;

            if (placement is ChartLabelPlacement.BestFit)
            {
                if (BestFitInner(bisector, sweep, radius.Points, block.Width.Points,
                                 block.Height.Points) is { } inner)
                {
                    DocPoint fitted = new(
                        centre.X + Length.FromPoints(inner.X),
                        centre.Y - Length.FromPoints(inner.Y));

                    placed.Add(Assemble(lines, Centred(fitted, block), block, gap, key,
                                        label.ShowLegendKey ? series.FillAt(at) : null, null));
                    continue;
                }

                // It did not fit. The reference removes the text shape and rebuilds it outside,
                // and the key of this attempt stays on the page.
                if (label.ShowLegendKey)
                    ghost = KeyOf(Centred(at05, block), block, key);
            }

            // OUTSIDE: the rim point on the bisector, plus a flat 150 in the radius direction, and
            // a wrapping width taken from the room between that point and the diagram's own edge.
            // INSIDE is the same rim point less the same 150, with the alignment mirrored.
            (DocRect outside, string[] outLines, DocSize outBlock) = OutsidePlacement(
                plot, measurer, text, size, bold, gap, centre, radius, verticalRadius,
                bisector, available, placement is ChartLabelPlacement.Inside);

            placed.Add(Assemble(outLines, outside, outBlock, gap, key,
                                label.ShowLegendKey ? series.FillAt(at) : null, ghost));
        }

        return placed;
    }

    /// <summary>The rectangle a label's key, gap and lines occupy together.</summary>
    private static DocSize BlockOf(
        ChartText measurer, string[] lines, Length size, bool bold, Length gap)
    {
        Length width = Length.Zero;
        Length height = Length.Zero;

        foreach (string line in lines)
        {
            DocSize measured = measurer.Measure(line, size, bold);
            width = Length.Max(width, measured.Width);
            height += measured.Height;
        }

        return new DocSize(width + gap, height);
    }

    /// <summary>The block rectangle centred on a point.</summary>
    private static DocRect Centred(DocPoint at, DocSize block)
        => new(at.X - (block.Width / 2), at.Y - (block.Height / 2), block.Width, block.Height);

    /// <summary>
    /// The label's legend key: a square at the block's left, a quarter of the block's height down.
    /// </summary>
    /// <remarks>
    /// <c>aSymbolPosition.Y += ((aTextSize.Height / nLineCountForSymbolsize) / 4)</c>, and
    /// <c>nLineCountForSymbolsize</c> is 1 for every label whose separator is not a newline. That
    /// quarter is measured on the reference to the hundredth on both a one-line label
    /// (562.05 predicted and drawn) and a two-line one (466.41 against 466.38).
    /// </remarks>
    private static DocRect KeyOf(DocRect block, DocSize size, Length key)
        => new(block.X, block.Y + (size.Height / 4), key, key);

    private static PiePlacedLabel Assemble(
        string[] lines,
        DocRect block,
        DocSize size,
        Length gap,
        Length key,
        Colour? fill,
        DocRect? ghost)
        => new(lines, block, fill is null ? null : KeyOf(block, size, key), fill, ghost);

    /// <summary>
    /// An <c>OUTSIDE</c> label: where its block goes and how its text wraps.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The anchor is the rim point on the bisector plus <see cref="OutsideLabelOffset"/> along the
    /// radius, and the alignment decides <em>which corner of the block</em> sits on it —
    /// <c>PolarLabelPositionHelper::getLabelScreenPositionAndAlignmentForUnitCircleValues</c>'s
    /// eight-way table, folded into "a horizontal family and a vertical family".
    /// </para>
    /// <para>
    /// Measured against all five slices of the <c>outEnd</c> rewrite: the four families place the
    /// block's left or right edge on the anchor to <strong>0.12 pt</strong>, and the block's own
    /// height — <em>n</em> line heights, with no vertical text inset in it — lands all five first
    /// baselines to <strong>0.09 pt</strong>. Including the 0.30-em inset that
    /// <c>ShapeFactory::createText</c> sets misses by 4.5 pt, so the inset is not in the group's
    /// box however plainly it is in the shape's properties.
    /// </para>
    /// </remarks>
    private static (DocRect Block, string[] Lines, DocSize Size) OutsidePlacement(
        ChartPlot plot,
        ChartText measurer,
        string text,
        Length size,
        bool bold,
        Length gap,
        DocPoint centre,
        Length radius,
        Length verticalRadius,
        double bisector,
        DocRect available,
        bool inside = false)
    {
        double angle = Norm360(bisector);
        double cos = Math.Cos(Radians(angle));
        double sin = Math.Sin(Radians(angle));

        // The rim point's own X, relative to the chart frame, is what the allowance is measured
        // from — `nOuterX` in PieChart::createTextLabelShape.
        Length rim = centre.X + (radius * cos);

        // An INSIDE label keeps the default guess, `0.8 * fPieRadius`: the room-to-the-edge arm
        // of that function is guarded by `nLabelPlacement == OUTSIDE` and the comment beside it
        // is a TODO asking for a better guess for INSIDE (PieChart.cxx:544-576).
        Length allowance = inside
            ? radius * BestFitWidthFraction
            : available.Width * CompatWidthFraction;

        if (!inside && available.Width > Length.Zero)
        {
            Length room = angle < 90 || angle > 270
                ? available.Width - (rim - available.X)
                : rim - available.X;

            Length reach = Length.Max(room, Length.Zero - room);

            allowance = Length.Min(reach * BestFitWidthFraction, allowance);
        }

        string[] lines = LinesOf(measurer, text, size, bold, allowance);
        DocSize block = BlockOf(measurer, lines, size, bold, gap);

        Length offset = inside ? Length.Zero - OutsideLabelOffset : OutsideLabelOffset;

        DocPoint anchor = new(
            centre.X + ((radius + offset) * cos),
            centre.Y - ((verticalRadius + offset) * sin));

        // The eight-way alignment table, as two independent families. `right` puts the block's
        // left edge on the anchor and grows rightward; `top` puts the block's bottom edge on it
        // and grows upward, which in these coordinates — y downward, as LibreOffice's screen — is
        // a subtraction.
        //
        // The table is written as `bOutside ? A : B` on all eight rows and B is A's opposite on
        // every one of them (PolarLabelPositionHelper.cxx:112-137), so an INSIDE label is the
        // same two families swapped — which is what makes its block hang back over the slice
        // instead of away from it.
        bool right = angle <= 5 || angle >= 355 || angle < 85 || angle > 275;
        bool left = (angle > 95 && angle < 175) || angle is >= 175 and <= 185
                    || (angle > 185 && angle < 265);
        bool up = angle is (< 85 and > 5) or (> 95 and < 175);
        bool down = angle is (> 185 and < 265) or (> 275 and < 355);

        if (inside)
        {
            (right, left) = (left, right);
            (up, down) = (down, up);
        }

        Length x = right ? anchor.X + (block.Width / 2)
                 : left ? anchor.X - (block.Width / 2)
                 : anchor.X;

        Length y = up ? anchor.Y - (block.Height / 2)
                 : down ? anchor.Y + (block.Height / 2)
                 : anchor.Y;

        return (Centred(new DocPoint(x, y), block), lines, block);
    }

    /// <summary>
    /// Where a label's box sits inside its own slice, or null when it does not fit.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A transcription of <c>PieChart::performLabelBestFitInnerPlacement</c>
    /// (<c>PieChart.cxx:1961-2272</c>), which is a closed-form construction rather than a search:
    /// the box is slid along the bisector until its nearest edge touches the arc, and the fit
    /// fails when the diagonal to its far corner is longer than the radius or when the ray to one
    /// of its vertices leaves the slice.
    /// </para>
    /// <para>
    /// <strong>Everything here is in the pie's own coordinates with Y upward</strong>, which is
    /// how the routine is written; the caller flips it. Mixing the two conventions puts every
    /// label in the wrong half of the chart and is the single easiest way to get this wrong.
    /// </para>
    /// <para>
    /// This is a port, and the prediction file says so: its <em>inputs</em> are our own text
    /// measurements, not LibreOffice's, so a label whose box we measure a little wider than the
    /// reference does can fall out of the slice the reference keeps it in.
    /// </para>
    /// </remarks>
    /// <param name="bisector">The slice's bisecting ray, in degrees, Y upward.</param>
    /// <param name="sweep">The slice's angular width in degrees.</param>
    /// <param name="radius">The pie's outer radius, in points.</param>
    /// <param name="width">The label block's width, in points.</param>
    /// <param name="height">The label block's height, in points.</param>
    private static (double X, double Y)? BestFitInner(
        double bisector, double sweep, double radius, double width, double height)
    {
        double half = sweep / 2.0;
        double ray = Norm360(bisector);

        double pie = radius * (1 - PieBorderOffset);
        if (pie <= 0.0 || width <= 0.0 || height <= 0.0) return null;

        // -45 <= alpha < 315
        double alphaDeg = Norm360(ray + 45) - 45;
        double alphaRad = Radians(alphaDeg);

        // 0 left, 1 bottom, 2 right, 3 top — and an even index means the nearest edge is vertical.
        int sector = (int)Math.Floor((alphaDeg + 45) / 45.0);
        int nearest = sector / 2;

        double nearestLength = width;
        double orthogonalLength = height;
        bool axisIsY = nearest % 2 == 0;

        if (axisIsY)
        {
            nearestLength = height;
            orthogonalLength = width;
        }

        int index = sector - 1;
        double indexMod2 = (index + 8) % 2;
        double sign = 2.0 * (indexMod2 - 0.5);
        double np = (nearestLength / 2.0)
                    * (1 + (sign * ((alphaDeg - (45 * (index + indexMod2))) / 45.0)));
        double pm = nearestLength - np;
        double pf = Math.Sqrt((pm * pm) + (orthogonalLength * orthogonalLength));

        if (pf > pie) return null;

        double beta = Math.Atan2(orthogonalLength, pm);
        double alphaMod90 = ((alphaDeg + 45) % 90.0) - 45;
        double signum = alphaMod90 == 0.0 ? 0.0 : alphaMod90 < 0 ? -1.0 : 1.0;
        double theta = (signum * alphaRad) + ((Math.PI / 2) * (1 - (signum * nearest))) + beta;
        if (theta > Math.PI) theta = (2 * Math.PI) - theta;

        double cp;
        if (theta % Math.PI == 0.0)
        {
            cp = pie - pf;
        }
        else
        {
            double sinTheta = Math.Sin(theta);
            double delta = Math.Asin(pf * sinTheta / pie);
            cp = pie * Math.Sin(Math.PI - (theta + delta)) / sinTheta;
        }

        double px = Math.Cos(alphaRad) * cp;
        double py = Math.Sin(alphaRad) * cp;

        double dx = ray is >= 90 and < 270 ? -1.0 : 1.0;
        double dy = ray >= 180 ? -1.0 : 1.0;

        double nx = px, ny = py;
        if (axisIsY) ny -= dy * np; else nx -= dx * np;

        double mx = nx, my = ny;
        if (axisIsY) my += dy * nearestLength; else mx += dx * nearestLength;

        double gx = nx, gy = ny;
        if (axisIsY) gx += dx * orthogonalLength; else gy += dy * orthogonalLength;

        if (Between(px, py, mx, my) > half) return null;

        bool crosses = axisIsY
            ? (ny >= 0 && my <= 0) || (ny <= 0 && my >= 0)
            : (nx >= 0 && mx <= 0) || (nx <= 0 && mx >= 0);

        if (crosses)
        {
            if (Between(px, py, nx, ny) > half) return null;
        }
        else if (Between(px, py, gx, gy) > half)
        {
            return null;
        }

        double bx = nx, by = ny;

        if (axisIsY)
        {
            by += dy * nearestLength / 2;
            bx += dx * orthogonalLength / 2;
        }
        else
        {
            bx += dx * nearestLength / 2;
            by += dy * orthogonalLength / 2;
        }

        return (bx, by);
    }

    /// <summary>The unsigned angle between two vectors, in degrees.</summary>
    private static double Between(double ax, double ay, double bx, double by)
    {
        double la = Math.Sqrt((ax * ax) + (ay * ay));
        double lb = Math.Sqrt((bx * bx) + (by * by));
        if (la <= 0.0 || lb <= 0.0) return 0.0;

        double cos = ((ax * bx) + (ay * by)) / (la * lb);
        return Degrees(Math.Acos(Math.Clamp(cos, -1.0, 1.0)));
    }

    private static double Radians(double degrees) => degrees * Math.PI / 180.0;

    private static double Degrees(double radians) => radians * 180.0 / Math.PI;

    /// <summary>An angle folded into [0, 360).</summary>
    private static double Norm360(double degrees)
    {
        double at = degrees % 360.0;
        return at < 0 ? at + 360.0 : at;
    }

    /// <summary>
    /// What a pie and its labels together take up, in the caller's coordinates.
    /// </summary>
    /// <remarks>
    /// <c>ShapeFactory::getRectangleOfShape(mxDiagramWithAxesShapes)</c> — the bounding box of the
    /// whole diagram group, which for a pie is the wall plus the wedges plus every label group.
    /// A plain pie's wedges are inscribed in the wall, so only the wall and the label blocks
    /// contribute; an of-pie's composition is not, and a doughnut's labels are not.
    /// </remarks>
    /// <remarks>
    /// <para>
    /// <strong>An of-pie overflows its own diagram square by design.</strong> The composition runs
    /// from <c>m_fLeftShift − m_fLeftScale</c> = −1.4167 unit radii to <c>m_fBarRight</c> = 1.25
    /// (a bar form) or <c>m_fRightShift + m_fRightScale</c> = 1.0833 (a second-pie form) across,
    /// and to ±<c>m_fLeftScale</c> = ±0.6667 down (<c>PieChart.hxx:258-269</c>), so it is 2.667
    /// unit radii wide against the square's 2. Leaving it out makes the consumed rectangle the
    /// wall's, which grows the diagram back to the whole available rectangle every time.
    /// </para>
    /// <para>
    /// <strong>A doughnut's labels never leave the ring.</strong> <c>AVOID_OVERLAP</c> is turned
    /// into <c>CENTER</c> for a ring chart and the labels cannot move
    /// (<c>bMovementAllowed &amp;&amp; !m_bUseRings</c>), so nothing a doughnut draws is outside
    /// its own wall — which is what 26.2.4.2's own resolved view says on all fifteen corpus
    /// doughnuts, whose <c>&lt;chart:coordinate-region&gt;</c> is 0.9999 of the square inscribed
    /// in their <c>&lt;chart:plot-area&gt;</c>. Running this file's single-pie label placer over a
    /// ring would invent outside labels and shrink every one of them.
    /// </para>
    /// </remarks>
    private static DocRect PieConsumedRect(
        ChartPlot plot, DocRect area, DocRect available, ChartText measurer)
    {
        List<ChartSeries> pie = plot.SeriesOf(ChartPlotKind.Pie, 0);
        bool split = pie.Count == 0;
        if (split) pie = plot.SeriesOf(ChartPlotKind.OfPie, 0);
        if (pie.Count == 0) return area;

        // A three-dimensional pie is an ellipse fitted to the whole rectangle, so its centre and
        // its two semi-axes come from the same fit that draws it rather than from the inscribed
        // circle. Its wall is inside the rectangle by construction, so only the labels can
        // consume anything beyond it.
        Pie3DGeometry? solid = plot.Elevation is { } elevation && !split && !plot.Rings
            ? Pie3DFit(area, elevation)
            : null;

        DocPoint centre = solid?.Centre
            ?? new DocPoint(area.X + (area.Width / 2), area.Y + (area.Height / 2));
        Length radius = solid?.A ?? (Length.Min(area.Width, area.Height) / 2);
        Length verticalRadius = solid?.B ?? radius;

        Length left = area.Left, top = area.Top, right = area.Right, bottom = area.Bottom;

        // The of-pie's own drawn extent, which AddOfPie composes from the same constants. Below
        // the four-point minimum it draws a plain pie instead and there is nothing to add.
        //
        // And its LABELS belong to the main pie, which is two thirds of the unit radius and sits
        // three quarters of one to the left of the unit centre -- not to the unit circle, which
        // nothing is drawn on. Measuring them on the unit circle is what left this pass with
        // nothing to consume on 028_Unit_Circle_Chart_Optimized_Graph: at pass 1's unit radius of
        // 84.25 pt every one of its sixteen labels passed the inner best-fit test and the
        // consumed rectangle came back exactly as tall as the diagram square, so the second pass
        // grew the diagram to the whole available rectangle and drew a unit radius of 185.3
        // against 26.2.4.2's 135.5.
        DocPoint ring = centre;
        Length ringRadius = radius;
        double ringStart = 90.0;
        int? ringPoints = null;

        if (split && pie[0].Values.Count >= OfPieMinimumPoints)
        {
            double far = plot.OfPieType is ChartOfPieType.Bar
                ? OfPieBarRight
                : OfPieRightShift + OfPieRightScale;

            left = Length.Min(left, centre.X + (radius * (OfPieLeftShift - OfPieLeftScale)));
            right = Length.Max(right, centre.X + (radius * far));
            top = Length.Min(top, centre.Y - (radius * OfPieLeftScale));
            bottom = Length.Max(bottom, centre.Y + (radius * OfPieLeftScale));

            ring = new DocPoint(centre.X + (radius * OfPieLeftShift), centre.Y);
            ringRadius = radius * OfPieLeftScale;
            (ringStart, ringPoints) = OfPieRingLabels(plot, pie[0]);
        }

        if (!plot.Rings)
        {
            foreach (PiePlacedLabel placed in PieLabels(
                         plot, pie[0], ring, ringRadius, split ? ringRadius : verticalRadius,
                         available, measurer, ringStart, ringPoints))
            {
                left = Length.Min(left, placed.Block.Left);
                top = Length.Min(top, placed.Block.Top);
                right = Length.Max(right, placed.Block.Right);
                bottom = Length.Max(bottom, placed.Block.Bottom);
            }
        }

        return new DocRect(left, top, right - left, bottom - top);
    }

    /// <summary>
    /// The rectangle a diagram's <em>first</em> pass is drawn at.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>VDiagram::reduceToMinimumSize</c> (<c>VDiagram.cxx:635-651</c>), called from
    /// <c>impl_createDiagramAndContent</c> at <c>ChartView.cxx:557-560</c> before a single series
    /// shape exists:
    /// </para>
    /// <code>
    /// // It is preferable to use full size than minimum for pie charts
    /// if (!rParam.mbUseFixedInnerSize)
    ///     aVDiagram.reduceToMinimumSize();
    /// </code>
    /// <para>
    /// <strong>The comment is a complaint, not a description.</strong> The guard is on
    /// <c>mbUseFixedInnerSize</c> — a manual <c>c:layout</c> on the plot area — and not on the
    /// chart type, so a pie is reduced too; <c>git blame</c> puts that line at 2019-05-28, well
    /// before 26.2.4.2. What normally undoes it is the axis-label pass at <c>:588</c>, whose
    /// <c>adjustInnerSize</c> grows the diagram straight back out — and that pass is guarded by
    /// <c>!bIsPieOrDonut</c>. So on a pie, and on a pie alone, the labels of pass 1 are laid out
    /// around a diagram <em>one 2.2th</em> of the available rectangle, and the pie's own second
    /// pass at <see cref="AdjustInnerSize"/> is the only thing that grows it back.
    /// </para>
    /// <para>
    /// That is what decides which labels pass 1 rebuilds outside, and it is the whole of the
    /// difference round 60 measured and could not close: at radius 110.72 the best-fit wrapping
    /// allowance is 88.6 pt and four of <c>003_advanced_excel_pie</c>'s five labels fit inside
    /// their slices, so nothing reaches left of the pie and <c>consumed.Left</c> came out at the
    /// diagram's own left edge. At radius 50.33 the allowance is 40.3 pt, every label fails the
    /// inner fit, and the consumed rectangle overruns on all four sides — which is the shape the
    /// reference's answer has to be solved back to.
    /// </para>
    /// <para>
    /// The rounding is <c>std::round</c>, away from zero, on hundredths of a millimetre; the
    /// intersection with the available rectangle and then the aspect ratio are
    /// <c>adjustPosAndSize</c>'s own order (<c>VDiagram.cxx:89-127</c>) and swapping them moves
    /// the pass-1 centre by tens of points.
    /// </para>
    /// </remarks>
    private static DocRect ReducedToMinimum(ChartPlot plot, DocRect available)
    {
        if (available.Width <= Length.Zero || available.Height <= Length.Zero) return available;

        Length width = Length.FromMm100(
            (long)Math.Round(available.Width.Mm100 / 2.2, MidpointRounding.AwayFromZero));
        Length height = Length.FromMm100(
            (long)Math.Round(available.Height.Mm100 / 2.2, MidpointRounding.AwayFromZero));

        Length left = Length.Max(available.X + width, available.Left);
        Length top = Length.Max(available.Y + height, available.Top);
        Length right = Length.Min(available.X + width + width, available.Right);
        Length bottom = Length.Min(available.Y + height + height, available.Bottom);

        return right <= left || bottom <= top
            ? available
            : Squared(plot, new DocRect(left, top, right - left, bottom - top));
    }

    /// <summary>
    /// The diagram rectangle a pie is redrawn at once its labels have been measured.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>VDiagram::adjustInnerSize</c> followed by <c>adjustPosAndSize</c>: the inner rectangle
    /// grows or shrinks by exactly what the drawn extent over- or under-ran the available one, with
    /// a floor of a third of the available rectangle so a chart whose labels are enormous still
    /// draws a pie; then the result is clipped back inside the available rectangle and re-squared,
    /// because a 2D pie's preferred aspect ratio is 1 and
    /// <c>calculateNewSizeRespectingAspectRatio</c> takes the smaller factor.
    /// </para>
    /// <para>
    /// <strong>It is applied once, not iterated.</strong> The pie branch of
    /// <c>impl_createDiagramAndContent</c> recreates the series exactly one more time and whatever
    /// that pass consumes is what is drawn — which is why the second pass's labels can overflow
    /// the available rectangle again and the reference lets them.
    /// </para>
    /// </remarks>
    private static DocRect AdjustInnerSize(
        ChartPlot plot, DocRect available, DocRect current, DocRect consumed)
    {
        if (available.Width <= Length.Zero || available.Height <= Length.Zero) return current;

        Length deltaWidth = available.Width - consumed.Width;
        if (current.Width + deltaWidth < available.Width / 3)
            deltaWidth = (available.Width / 3) - current.Width;

        Length deltaHeight = available.Height - consumed.Height;
        if (current.Height + deltaHeight < available.Height / 3)
            deltaHeight = (available.Height / 3) - current.Height;

        Length width = current.Width + deltaWidth;
        Length height = current.Height + deltaHeight;

        Length x = current.X;
        Length diffLeft = consumed.Left - available.Left;
        Length diffRight = available.Right - consumed.Right;

        if (diffLeft >= Length.Zero)
        {
            x -= diffLeft;
        }
        else if (diffRight >= Length.Zero)
        {
            x += diffRight > Length.Zero - diffLeft ? Length.Zero - diffLeft
               : diffRight > Magnitude(deltaWidth) ? diffRight
               : Magnitude(deltaWidth);
        }

        Length y = current.Y;
        Length diffUp = consumed.Top - available.Top;
        Length diffDown = available.Bottom - consumed.Bottom;

        if (diffUp >= Length.Zero)
        {
            y -= diffUp;
        }
        else if (diffDown >= Length.Zero)
        {
            y += diffDown > Length.Zero - diffUp ? Length.Zero - diffUp
               : diffDown > Magnitude(deltaHeight) ? diffDown
               : Magnitude(deltaHeight);
        }

        Length left = Length.Max(x, available.Left);
        Length top = Length.Max(y, available.Top);
        Length right = Length.Min(x + width, available.Right);
        Length bottom = Length.Min(y + height, available.Bottom);

        return right <= left || bottom <= top
            ? current
            : Squared(plot, new DocRect(left, top, right - left, bottom - top));
    }

    /// <summary>A length's magnitude — <c>abs</c>, which <see cref="Length"/> does not carry.</summary>
    private static Length Magnitude(Length value)
        => value >= Length.Zero ? value : Length.Zero - value;
}

using XRay.Office.Core.Geometry;
using XRay.Office.Core.Globalization;
using XRay.Office.Core.Units;

namespace XRay.Office.Core.Charts;

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
/// <param name="Texts">
/// The labels as they are to be <em>drawn</em> — with the line breaks the arrangement put in them
/// — or null when none of them wrapped and the caller's own strings stand.
/// <para>
/// <strong>A wrapped label is a different shape from the string it came from, and until this was
/// carried the arrangement measured the string.</strong> An axis whose labels may break gets one
/// tick's worth of width per label and wraps inside it; measuring the unwrapped run instead finds
/// a collision that is not there and thins the axis out. See <see cref="ChartAxisLabels"/>.
/// </para>
/// </param>
public readonly record struct ChartAxisLabelLayout(
    double Rotation,
    int Rhythm,
    bool Staggered,
    Length Reserved,
    IReadOnlyList<string?>? Texts = null);

/// <summary>Which way an axis runs across the page.</summary>
/// <remarks>
/// <para>
/// <c>TickFactory2D::isHorizontalAxis</c> and <c>::isVerticalAxis</c>
/// (<c>chart2/source/view/axes/Tickmarks.cxx</c>:169-192), which decide it from the axis line's
/// own two ends. A bar chart's category axis runs down the left of the plot and its value axis
/// along the bottom; a column chart's are the other way round.
/// </para>
/// <para>
/// <strong>It is not a cosmetic distinction: three of the four escape routes are closed on a
/// vertical axis.</strong> See <see cref="ChartAxisLabels"/>.
/// </para>
/// </remarks>
public enum ChartAxisDirection
{
    /// <summary>Along the page — a column chart's category axis.</summary>
    Horizontal = 0,

    /// <summary>Down the page — a bar chart's category axis.</summary>
    Vertical,
}

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
/// <para>
/// <strong>A vertical category axis is the same loop with three of its four moves taken
/// away.</strong> <c>canAutoAdjustLabelPlacement</c> (<c>VCartesianAxis.cxx:539-556</c>) is the
/// joint prerequisite for auto-rotation <em>and</em> auto-staggering, and its last three lines
/// are the whole rule: <c>if (bIsHorizontalAxis) return !m_bStackCharacters; if (bIsVerticalAxis)
/// return m_bStackCharacters; return false;</c> — "automatic adjusting labels only works for
/// horizontal axis with horizontal text or vertical axis with vertical text". Ordinary
/// horizontal type on an axis running down the page therefore never turns 45 degrees and never
/// staggers; the only move left is to thin the labels out. What it keeps is line breaking, whose
/// own gate (<c>isBreakOfLabelsAllowed</c>, <c>:513-535</c>) returns <c>bIsVerticalAxis</c> for a
/// swapped chart, and whose limit is not the tick spacing at all: <c>:768-773</c> replaces it
/// with <c>pTickFactory-&gt;getXaxisStartPos().getX()</c>, the whole room between the chart's own
/// left edge and the axis, and takes <em>no</em> five per cent off it.
/// </para>
/// <para>
/// <strong>Measured, on seventy-five decks built for it and rendered through 26.2.4.2</strong>
/// (<c>probes/chart-layout/</c>): a bar chart whose eight category names are 184 pt wide draws
/// every one of them on <em>one</em> line and moves the plot's left edge to make room, and so
/// does the same chart with those names written as a single unbreakable word — because an
/// automatically laid-out plot gives up exactly the width the widest label needs, which makes
/// the wrap limit its own fixed point and never binding. Give the same chart a
/// <c>c:manualLayout</c> that fixes the plot at 0.10 of the frame and the labels break onto four
/// lines whose longest is 58.7 pt against the stated band's 59.0, and the axis thins to every
/// second, third and fifth label at 8, 16 and 32 categories. Neither deck rotates a label or
/// staggers one.
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
    /// <param name="direction">
    /// Which way the axis runs. A <see cref="ChartAxisDirection.Vertical"/> axis measures its
    /// labels' extent along the axis by their <em>height</em> and reserves their width, and it
    /// may neither rotate nor stagger — see <see cref="ChartAxisLabels"/>.
    /// </param>
    /// <param name="room">
    /// The width a <see cref="ChartAxisDirection.Vertical"/> axis' labels have between the
    /// chart's own edge and the axis, which is the limit they break at. Zero leaves them
    /// unbroken. Ignored on a horizontal axis, whose limit is <see cref="WrapFraction"/> of one
    /// tick's worth of axis.
    /// </param>
    /// <param name="hyphenator">
    /// What decides where a word may be broken inside itself. Null takes
    /// <see cref="Hyphenators.For"/>'s answer for the default language, which is what a chart
    /// gets in production; pass <see cref="Hyphenators.None"/> for the no-dictionary state.
    /// <para>
    /// <strong>It reaches the arrangement rather than only the drawing, and that is the whole of
    /// this seat.</strong> <c>PropertyMapper::getTextLabelMultiPropertyLists</c> sets
    /// <c>ParaIsHyphenation</c> true inside the same <c>if (nLimitedSpace &gt; 0)</c> that sets
    /// <c>TextMaximumFrameWidth</c> and nowhere else, and <c>DrawModelWrapper</c> installs
    /// <c>LinguMgr::GetHyphenator()</c> on the chart's outliner — so a label's second line may
    /// begin inside a word that the first line took a hyphenated piece of, which is exactly what
    /// <see cref="Wraps"/> is looking for.
    /// </para>
    /// </param>
    public static ChartAxisLabelLayout Resolve(
        IReadOnlyList<string?> texts,
        IReadOnlyList<Length> centres,
        ChartAxisText stated,
        Length size,
        ChartText measurer,
        bool bold = false,
        ChartAxisDirection direction = ChartAxisDirection.Horizontal,
        Length room = default,
        IHyphenator? hyphenator = null)
    {
        ArgumentNullException.ThrowIfNull(texts);
        ArgumentNullException.ThrowIfNull(centres);

        hyphenator ??= Hyphenators.For(null);

        bool vertical = direction == ChartAxisDirection.Vertical;

        double rotation = stated.Rotation;
        bool lineBreak = stated.LineBreakAllowed;
        ChartLabelStagger stagger = vertical ? ChartLabelStagger.SideBySide : stated.Stagger;
        int rhythm = 1;

        int count = Math.Min(texts.Count, centres.Count);
        if (count == 0) return new ChartAxisLabelLayout(rotation, 1, false, Length.Zero);

        Length spacing = Spacing(centres, count);

        for (int attempt = 0; attempt < MaximumAttempts; attempt++)
        {
            bool staggered = stagger is ChartLabelStagger.Even or ChartLabelStagger.Odd;

            // The room one label is laid out in before it breaks. A horizontal axis gets one
            // tick's worth less five per cent; a vertical one gets the whole band between the
            // chart's own edge and the axis, and no reduction at all
            // (VCartesianAxis.cxx:768-773).
            Length limit = vertical
                ? room
                : spacing * (staggered ? 2.0 : 1.0) * WrapFraction;

            // A label that has to break *inside a word* to fit that room makes chart2 give up on
            // wrapping altogether and start again (VCartesianAxis.cxx:888-903), and a label that
            // merely breaks at a blank does not — measured on three fixed-layout probe decks
            // whose lines begin with a hyphen and with a bracket and are wrapped anyway. That
            // restart is what lets auto-rotation happen at all on an OOXML axis, whose importer
            // turns line breaking *on*: canAutoAdjustLabelPlacement refuses while it is on, so
            // the wrap is the only route from "labels collide" to "labels are turned 45°".
            if (lineBreak && !stated.OverlapAllowed && rotation == 0.0
                && Wraps(texts, count, limit, size, measurer, bold, hyphenator))
            {
                lineBreak = false;
                continue;
            }

            // With line breaking still on and the labels upright, each one is laid out inside
            // `TextMaximumFrameWidth` — the limit above — and wraps at its word boundaries. That
            // is the shape `doesOverlap` intersects and the shape the axis reserves room for, and
            // it is *not* the single-line run: `ACCOUNT MANAGER` set on one line is nearly two
            // ticks wide and set on two is under one.
            //
            // Measured on 033_Event_planning_tracker_Use_this_template_f29a848e.xlsx, whose six
            // category names are all two words: the reference draws all six on two lines each and
            // we drew three, having found a collision between runs it never laid out.
            IReadOnlyList<string?>? wrapped =
                lineBreak && rotation == 0.0 && limit > Length.Zero
                    ? Wrap(texts, count, limit, size, measurer, bold)
                    : null;

            // The shape each label sits in, insets included — what collides is the shape and not
            // the text, exactly as everything else this file reserves room for.
            DocSize[] boxes = Boxes(wrapped ?? texts, count, size, measurer, bold);

            // canAutoAdjustLabelPlacement, VCartesianAxis.cxx:539-556 — and it deliberately
            // does *not* test the arrangement, which is why an OOXML axis whose staggering is
            // turned off may still be rotated. Its last three lines close both routes on an axis
            // running down the page —
            // "automatic adjusting labels only works for horizontal axis with horizontal text or
            // vertical axis with vertical text" — so a vertical axis can only thin.
            bool canAdjust =
                !vertical && !stated.OverlapAllowed && !lineBreak && rotation == 0.0;

            // The separation two labels must keep is not the whole tick spacing: the reference
            // keeps the same `nReduce` clear between them that it takes off the wrap limit —
            // see the block beside WrapFraction below for the measurement.
            Length reserve = vertical
                ? Length.Zero
                : spacing * (staggered ? 2.0 : 1.0) * (1.0 - WrapFraction);

            if (stated.OverlapAllowed
                || !Collides(
                    boxes, centres, count, rhythm, staggered, rotation, vertical, reserve))
            {
                return new ChartAxisLabelLayout(
                    rotation, rhythm, staggered,
                    Depth(boxes, count, rhythm, staggered, rotation, vertical),
                    wrapped);
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

        bool lastStaggered = stagger is ChartLabelStagger.Even or ChartLabelStagger.Odd;

        return new ChartAxisLabelLayout(
            rotation, rhythm, lastStaggered,
            Depth(
                Boxes(texts, count, size, measurer, bold), count, rhythm, false, rotation,
                vertical));
    }

    /// <summary>The shape each label sits in.</summary>
    private static DocSize[] Boxes(
        IReadOnlyList<string?> texts, int count, Length size, ChartText measurer, bool bold)
    {
        DocSize[] boxes = new DocSize[count];

        for (int at = 0; at < count; at++)
        {
            boxes[at] = at < texts.Count && texts[at] is { Length: > 0 } text
                ? Shape(measurer, text, size, bold)
                : default;
        }

        return boxes;
    }

    /// <summary>
    /// The labels broken at word boundaries to fit a width, or null when none of them needs it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The greedy fill a text shape does: words are added to the current line while they fit and a
    /// new line is started when the next one does not.
    /// </para>
    /// <para>
    /// <strong>Only a blank is a break opportunity here, where <see cref="Words"/> also splits at
    /// a hyphen and a slash</strong>, and the difference is deliberate: <see cref="Words"/>
    /// measures runs and never rebuilds the string, while this rewrites the label, and a
    /// hyphen-split rebuilt with a space would turn <c>Oct-12</c> into <c>Oct- 12</c>. Splitting
    /// on the blank alone round-trips exactly. The cost is a hyphenated label that the reference
    /// wraps and this leaves on one line, which is the conservative direction: it can only find a
    /// collision the reference does not, never hide one it does.
    /// </para>
    /// <para>
    /// A word wider than the whole limit is left on a line of its own rather than broken inside
    /// itself: <see cref="Wraps"/> has already run at this point and turned line breaking off for
    /// exactly that case, so reaching here with one means the arrangement is not going to wrap at
    /// all.
    /// </para>
    /// </remarks>
    private static string?[]? Wrap(
        IReadOnlyList<string?> texts,
        int count,
        Length limit,
        Length size,
        ChartText measurer,
        bool bold)
    {
        if (limit <= Length.Zero) return null;

        string?[]? wrapped = null;

        for (int at = 0; at < count; at++)
        {
            if (at >= texts.Count || texts[at] is not { Length: > 0 } text) continue;
            if (measurer.Measure(text, size, bold).Width <= limit) continue;

            string[] words = text.Split(' ');
            if (words.Length < 2) continue;

            System.Text.StringBuilder built = new();
            string line = string.Empty;

            foreach (string word in words)
            {
                string candidate = line.Length == 0 ? word : line + " " + word;

                if (line.Length > 0 && measurer.Measure(candidate, size, bold).Width > limit)
                {
                    if (built.Length > 0) built.Append('\n');
                    built.Append(line);
                    line = word;
                    continue;
                }

                line = candidate;
            }

            // Whether the label broke at all cannot be asked of the finished string's length: a
            // break replaces exactly one blank with exactly one newline, so `ACCOUNT MANAGER` set
            // on two lines is the same fifteen characters it was on one. Asking it that way threw
            // away every label that broke at a single space — which is what a two-word category
            // name is — and left the axis to thin itself out instead. `built` holds the lines
            // already closed and is empty until the first break is taken, so it is the question.
            if (built.Length == 0) continue;

            built.Append('\n');
            built.Append(line);

            wrapped ??= [.. texts.Take(count)];
            wrapped[at] = built.ToString();
        }

        return wrapped;
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

    /// <summary>
    /// How much of one tick's worth of axis a label's word is given before it breaks inside
    /// itself — <c>createTextShapes</c>'s own 5% reduction.
    /// </summary>
    /// <remarks>
    /// See <see cref="Wraps"/>, which carries the measurement. It is a <em>horizontal</em> axis'
    /// constant: a vertical category axis' limit is the whole band left of it, with no reduction
    /// (<c>VCartesianAxis.cxx:768-773</c>).
    /// </remarks>
    private const double WrapFraction = 0.95;

    // ---------------------------------------------------------------------------------------
    // The gap two labels have to keep, and why it is the same five per cent.
    //
    // `doesOverlap` (VCartesianAxis.cxx:186-207) intersects the two label *shapes*, and reading
    // this tree's chart2 says a label's shape is its text: `createSingleLabel` uses the
    // `createText` overload at ShapeFactory.cxx:2042, which sets no text distance, and
    // `SdrTextObj::AdjustTextFrameWidthAndHeight` (svx/source/svdraw/svdotxat.cxx:44-240) grows an
    // auto-grow frame to `Outliner::CalcTextSize().Width() + 1` of 1/100 mm. On that reading two
    // labels collide when their mean width exceeds the tick spacing.
    //
    // 26.2.4.2 does not behave that way, and the discriminator is a pair of labels of DIFFERENT
    // widths, which separates the collision from the wrap: with equal labels the two thresholds
    // are 1.00 and 0.95 of the spacing and only the smaller is ever reached, so a one-word ladder
    // cannot see the collision at all. Alternating a long label with a short one along one axis
    // and sweeping the chart frame's width (`probes/chart-collide-r110`, four pitches from 54.2
    // to 121.7 pt, runs of one letter so no ligature or kern pair can enter):
    //
    //   pitch 54.193        turned from mean 51.369, upright to 51.158  -> pitch x (.0521,.0560]
    //   pitch 71.85 / 72.04 turned / upright about mean 67.972         ->         (.0539,.0564]
    //   pitch 108.398       turned from 103.063, upright to 102.739    ->         (.0492,.0522]
    //   pitch 121.56/121.67 turned / upright about mean 115.258        ->         (.0519,.0527]
    //
    // A CONSTANT reserve is refuted outright - 2.9 pt at pitch 54 against 6.4 at pitch 122 - so
    // whatever it is goes with the spacing. A single clean FRACTION is not established either: the
    // two rows whose labels are runs of one letter, the only two with no tail-advance question in
    // them, are (.0539,.0564] and (.0519,.0527] and do not intersect until the advance model's own
    // +/-0.13 % is folded in, which leaves (.0529,.0540].
    //
    // The five per cent `nReduce` that `createTextShapes` already takes off the wrap limit "to have
    // a visible distance between the labels" (VCartesianAxis.cxx:753-759) is 0.0500 - below that
    // window by 6 % of itself and by 0.3 % of a tick. It is what is implemented anyway, and
    // deliberately: 0.053 is a number fitted to one chart with no mechanism behind it; 0.050 is the
    // source's own and is already this file's WrapFraction; the measurements with no free parameter
    // at all - the one-word frame sweeps, which put the wrap limit at 0.95 to [0.94895, 0.95125)
    // over five pitches - agree with the source exactly; and all 176 chart-bearing corpus
    // renderings are byte-identical under either. The 6 % is recorded as open, not fitted away.
    //
    // Two limits of the measurement, both deliberate. It is a *horizontal* axis' constant: a
    // vertical category axis' wrap limit has no reduction in it either
    // (VCartesianAxis.cxx:768-773), so the reserve is zero there and nothing was measured. And it
    // is added to the collision test alone, not to `Depth`: what is measured is the
    // separation the reference keeps between two labels, and whether its shape is genuinely wider
    // than its text — which would also deepen the band a rotated axis reserves — is not.
    // ---------------------------------------------------------------------------------------

    /// <summary>Whether any label would wrap in the room one tick's worth of axis gives it.</summary>
    /// <remarks>
    /// <para>
    /// <strong>The room is 0.95 of the tick spacing — the source's own constant, and it is not
    /// fitted.</strong> <c>createTextShapes</c> reduces the limit by 5% "to have a visible
    /// distance between the labels" (<c>VCartesianAxis.cxx:753-759</c>) and hands the result to
    /// the shape as <c>TextMaximumFrameWidth</c>. Round 30 measured [0.990, 1.056] of the spacing
    /// on <c>research/probes/slides-r30/make-rot-probe.py</c>'s decks and rejected 0.95 as "0.88
    /// of it on those decks"; round 63 re-ran those decks against the reference and found that
    /// what round 30 had located was not this boundary at all.
    /// </para>
    /// <para>
    /// <strong>Passing this test does not turn the axis; it turns line breaking off.</strong>
    /// <c>lcl_hasWordBreak</c> sets <c>m_bLineBreakAllowed = false</c> and restarts the layout
    /// (<c>VCartesianAxis.cxx:888-903</c>), and the 45° only follows if the labels then
    /// <em>collide</em> as single lines. Round 63's decks separate the two by giving one label a
    /// space in it.
    /// <para>
    /// <strong>On a one-word label the two boundaries very nearly coincide, and this remark used
    /// to say they were 0.95 and 1.00 of the spacing.</strong> They are not: the collision keeps
    /// the same five per cent clear that the wrap does, so a single word past 0.95 of a tick
    /// restarts the wrap <em>and</em> collides, and the axis turns. Round 110 swept the tick pitch
    /// continuously at five run lengths and read the one-word turn threshold at
    /// [0.94895, 0.95125) of the spacing, with no free parameter — see the block beside
    /// <see cref="WrapFraction"/>. What the one-word case cannot show is the collision on its
    /// own, which is why that round had to alternate two label widths.
    /// </para>
    /// <para>
    /// <strong>0.95 is the room, and it is now measured directly rather than inferred from when
    /// the axis turns.</strong> A label made of several short words can never break inside a word,
    /// so the axis never restarts and the label is simply drawn wrapped — and <em>which words land
    /// on which line</em> is the room, read straight out of the reference's own PDF with no
    /// threshold model in between. On <c>ooo</c>-free runs of Carlito at 11 pt
    /// (<c>probes/chart-wrap-r111</c> §1), four composition transitions swept continuously by the
    /// chart frame's width:
    /// </para>
    /// <list type="table">
    /// <item><description>line 1 takes a third token between pitch 52.003 and 52.464, needing
    /// 49.438 pt → the room is in [0.9423, 0.9507) of the pitch.</description></item>
    /// <item><description>line 1 takes a fourth between 73.178 and 73.632, needing 69.713 →
    /// [0.9468, 0.9527).</description></item>
    /// <item><description>line 2 takes a third word between 61.074 and 61.528, needing 58.282 →
    /// [0.9472, 0.9543).</description></item>
    /// <item><description>the same at <strong>eight</strong> categories rather than five, between
    /// 51.972 and 52.154 → [0.9479, 0.9512), so the room does not move with the category
    /// count.</description></item>
    /// </list>
    /// <para>
    /// They intersect at <strong>[0.9472, 0.9507)</strong> and 0.95 is inside, with no free
    /// parameter beyond the advance.
    /// </para>
    /// <para>
    /// <strong>What is NOT the room is the two-word restart, and round 110's "the wrap restarts at
    /// 0.875 of the pitch" is a statement about a different quantity.</strong> On a two-word label
    /// 26.2.4.2 restarts — turns the axis — while the label's widest word still fits that 0.95
    /// room, and the boundary is not a fraction of the pitch at all. Measured on the same
    /// instrument, all at 11 pt, the threshold pitch at which a widest word of 47.285 pt stops
    /// restarting the axis (<c>probes/chart-wrap-r111</c> §2):
    /// </para>
    /// <list type="table">
    /// <item><description><strong>It moves with the width of the label's OTHER words</strong>, at
    /// eight prefix widths from 10.327 to 35.596 pt: 53.2→55.9 pt of pitch, a slope in
    /// (0.0949, 0.1147) pt of pitch per pt of prefix.</description></item>
    /// <item><description><strong>Width, not character count</strong>: <c>iiii</c> (4 characters,
    /// 10.327 pt) sits with <c>oo</c> (2 characters, 11.865 pt) and nowhere near <c>oooo</c>; a
    /// per-character rule predicts 54.58 against a measured (52.912, 53.517].</description></item>
    /// <item><description><strong>Position-blind</strong>: a prefix of two words of the same total
    /// behaves as one (<c>ooo ooo</c>, (55.936, 56.541], against <c>oooooo</c>'s
    /// (55.634, 55.936]), and a word added <em>after</em> the long one counts too.</description></item>
    /// <item><description><strong>And it moves with the number of categories, which no property of
    /// the label and the pitch can do.</strong> The same label at eight categories restarts at
    /// (52.444, 52.721] where five categories give (53.819, 54.122]; a second pair, at a pitch of
    /// 33, splits (32.879, 33.162] against (33.658, 33.953]. Both are ~2.4 % of the
    /// pitch.</description></item>
    /// </list>
    /// <para>
    /// So the restart is decided against a geometry that is <em>not</em> the one 26.2.4.2 draws,
    /// and it is not implemented here: every form that fits needs three fitted terms and still
    /// misses the category count. A plot-squeeze account — the labels measured unwrapped
    /// (<c>createMaximumLabels</c>) overflowing the plot and shrinking the pitch the decision is
    /// taken at — has the right sign and the right category dependence and is <strong>refuted by
    /// the four transitions above</strong>: their label is 110 pt unwrapped against a 52 pt pitch,
    /// far more overflow than any two-word case here, and their room is still exactly 0.95.
    /// It is not hyphenation either (r110 §4.2, and a run of one letter has no hyphenation point
    /// to take). Carried as open.
    /// </para>
    /// </para>
    /// <list type="table">
    /// <item><description><c>Middle Column</c> among twelve categories at 10 pt in Liberation
    /// Sans, tick spacing swept continuously by the chart frame's own width: LibreOffice is
    /// upright at 35.476 and turned at 35.348, and <c>Column</c> measures 33.597 on
    /// <c>chart2</c>'s device — so the limit is in [0.9470, 0.9505) of the
    /// spacing.</description></item>
    /// <item><description>The same twelve at 11 pt, where the pixel em rounds the other way:
    /// upright at 40.864, turned at 40.703, <c>Column</c> 38.765, so [0.9486, 0.9524).
    /// <strong>The same two boundaries read on the unquantised metrics are [0.9713, 0.9748) and
    /// [0.9276, 0.9312) and do not intersect at all</strong>, which is round 62's pixel-em law
    /// arriving from a second, independent observable.</description></item>
    /// <item><description>Replacing that label's second word so the first word decides
    /// (<c>MiddleMiddleMi Column</c>), or shortening it (<c>Middle Colum</c>), or lengthening it
    /// (<c>Middle Columnn</c>), moves the boundary to where <c>0.95 × spacing</c> puts it and
    /// nowhere else; changing the <em>first</em> word alone (<c>Mi Column</c>) does not move it at
    /// all.</description></item>
    /// </list>
    /// <para>
    /// The 5% is taken in integer 1/100 mm by the source and so is at most 0.08% larger than a
    /// flat 0.95 at these spacings — a difference the decks cannot see and this does not model.
    /// </para>
    /// </remarks>
    private static bool Wraps(
        IReadOnlyList<string?> texts,
        int count,
        Length limit,
        Length size,
        ChartText measurer,
        bool bold,
        IHyphenator? hyphenator)
    {
        if (limit <= Length.Zero) return false;

        // The first label is not tested — `nTick > 0` guards the whole check
        // (VCartesianAxis.cxx:892). It is not a detail. Round 63's `C` deck's widest word is its
        // *first* label, `START` at 31.96 against a limit of 29.16, and LibreOffice leaves that
        // axis upright; testing the first label turns it instead.
        for (int at = 1; at < count; at++)
        {
            if (texts[at] is not { Length: > 0 } text) continue;

            foreach (string word in Words(text))
                if (measurer.Measure(word, size, bold).Width > limit) return true;

            if (hyphenator is not null && Hyphenates(text, limit, size, measurer, bold, hyphenator))
                return true;
        }

        return false;
    }

    /// <summary>
    /// Whether the hyphenating fill would start one of a label's lines inside a word.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This is the arm of <c>lcl_hasWordBreak</c> that a width rule cannot reach.</strong>
    /// The test above asks whether any single word is wider than the slot, which is one way for a
    /// line to begin mid-word and the only one available without a dictionary. The other is
    /// hyphenation: with a pattern file loaded the outliner puts as much of the next word on the
    /// current line as a hyphen allows, and the line after it then begins in the middle of that
    /// word — which <c>lcl_hasWordBreak</c> (<c>VCartesianAxis.cxx:369-404</c>) reports, and
    /// which clears <c>m_bLineBreakAllowed</c> and restarts the axis at <c>:888-905</c>. Every
    /// label of <c>038_Competitive_Advantage_Card</c> is two words that fit their slot one to a
    /// line; the reference turns that axis 45 degrees and it is this that makes it.
    /// </para>
    /// <para>
    /// <strong>Measured on the reference with the words held fixed and only the dictionary
    /// varied</strong> (<c>probes/chart-hyph-r105</c> §1.1): tagging that witness' labels
    /// <c>en-US</c>, <c>en-GB</c>, <c>fr-FR</c>, <c>es-ES</c> or nothing at all — the four locales
    /// LibreOffice 26.2.4.2 ships patterns for — turns the axis, and <c>de-DE</c>, <c>ru-RU</c> or
    /// <c>zxx</c> wraps it upright. The control is the same sentence with an unhyphenatable second
    /// word, <c>Cost Stretched</c>, which wraps under all eight.
    /// </para>
    /// <para>
    /// <strong>The limit on how much of the word may stay is a CHARACTER COUNT, not a width, and
    /// measuring the hyphen instead is wrong in the permissive direction.</strong>
    /// <c>ImpBreakLine</c> walks the line's own <c>CharPosArray</c> to <c>nMaxBreakPos</c>, the
    /// first character of the line that does not fit, and asks the hyphenator for a point at most
    /// <c>nMaxBreakPos - nWordStart - 1</c> characters into the word —
    /// <c>editeng/source/editeng/impedit3.cxx</c>:2143-2165, whose <c>+1</c> is commented
    /// <em>"Before the dickey letter"</em> and reserves one character's room for the hyphen
    /// rather than measuring one. <c>Hyphenator::hyphenate</c> then enforces it as
    /// <c>i &lt; Leading</c> (<c>hyphenimp.cxx</c>:445). A hyphen is narrower than the letter it
    /// stands in for, so asking whether <c>Cost Ser-</c> fits admits breaks the reference
    /// refuses: <c>Cost Service</c> is the measured case, and it is the one word of twenty-two
    /// that the width form got wrong (§2 of <c>probes/hyphen-r106</c>). Read out of a tree that
    /// is 27.2 alpha rather than the 26.2.4.2 reference binary, and confirmed against that binary
    /// on the twenty-two authored fixtures rather than taken on trust.
    /// </para>
    /// <para>
    /// The fill is the greedy one <see cref="Wrap"/> already models, over blank-separated words,
    /// and only the boolean is wanted — so once a hyphenated break is found there is no need to
    /// carry the broken text, and where none is found nothing was hyphenated and
    /// <see cref="Wrap"/>'s plain answer stands unchanged. That is why the drawn text is untouched
    /// by this: a label that hyphenates never reaches the drawing, because line breaking is turned
    /// off and the arrangement starts again.
    /// </para>
    /// </remarks>
    private static bool Hyphenates(
        string text, Length limit, Length size, ChartText measurer, bool bold,
        IHyphenator hyphenator)
    {
        string[] words = text.Split(' ');
        if (words.Length < 2) return false;

        string line = string.Empty;

        foreach (string word in words)
        {
            string candidate = line.Length == 0 ? word : line + " " + word;

            if (line.Length == 0 || measurer.Measure(candidate, size, bold).Width <= limit)
            {
                line = candidate;
                continue;
            }

            // `nWordLen > 3` (impedit3.cxx:2152): a word of three characters or fewer is not
            // offered to the hyphenator at all, whatever its patterns say.
            if (word.Length <= 3)
            {
                line = word;
                continue;
            }

            // How many of the word's characters fit after what is already on the line. The
            // comparison is strict because CharPosArray holds each character's end and the walk
            // is `< nRemainingWidth`.
            int fits = 0;
            while (fits < word.Length
                   && measurer.Measure(line + " " + word[..(fits + 1)], size, bold).Width < limit)
            {
                fits++;
            }

            // One character of that room belongs to the hyphen, so the piece that stays behind is
            // at most `fits - 1` characters long. A point inside that leaves the next line
            // starting in the middle of the word, which is the break the reference restarts on.
            foreach (int point in hyphenator.FindHyphenationPoints(word, Hyphenators.DefaultLanguage))
                if (point >= 2 && point <= fits - 1) return true;

            line = word;
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

            // A hyphen or a slash is part of the run it ends — the break comes after it and its
            // width is on the line. A blank is not: it hangs past the line's end, so counting it
            // makes every space-separated label 0.28 em too wide for the test above. Measured on
            // round 63's `C` deck, whose widest tested word `Middle` is 28.72 against a limit of
            // 29.16: LibreOffice leaves it upright, and `Middle ` at 31.43 would turn it.
            if (at > start)
                yield return text[at] is ' ' or '\t' ? text[start..at] : text[start..(at + 1)];

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
        double rotation,
        bool vertical,
        Length reserve)
    {
        // Staggering puts alternate labels on two rows, so what a label collides with is the one
        // two places away rather than the one beside it.
        int step = Math.Max(1, rhythm) * (staggered ? 2 : 1);

        // The offset between two neighbours runs along the axis, so turning it back through the
        // label's own rotation gives (gap·cos, gap·sin) on a horizontal axis and (gap·sin,
        // gap·cos) on a vertical one — the same test with the two trigonometric terms swapped.
        // Upright, that is the whole difference between "two labels collide when their widths
        // meet" and "when their heights do".
        double cosine = Math.Abs(vertical ? Math.Sin(rotation) : Math.Cos(rotation));
        double sine = Math.Abs(vertical ? Math.Cos(rotation) : Math.Sin(rotation));

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

                // `reserve` is added to the sum of the two half-widths, which is exactly the
                // same arithmetic as widening *each* box by it — so it turns with the labels
                // rather than being a gap measured along the axis.
                double width =
                    ((boxes[at].Width.Emu + boxes[previous].Width.Emu) / 2.0) + reserve.Emu;
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
        DocSize[] boxes, int count, int rhythm, bool staggered, double rotation, bool vertical)
    {
        // Away from the axis is across the page for a horizontal axis and along it for a vertical
        // one, so the two terms swap here exactly as they do in <see cref="Collides"/>: an
        // upright label on a vertical axis reserves its own width.
        double cosine = Math.Abs(vertical ? Math.Sin(rotation) : Math.Cos(rotation));
        double sine = Math.Abs(vertical ? Math.Cos(rotation) : Math.Sin(rotation));

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

    /// <summary>
    /// The shape an axis label is drawn in, which is its text and nothing else.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An axis label has no insets, and <c>ChartLayout</c>'s 0.18/0.30 em are a data
    /// table's.</strong> <c>PropertyMapper::getTextLabelMultiPropertyLists</c>
    /// (<c>chart2/source/view/main/PropertyMapper.cxx:492-525</c>) — which is what
    /// <c>VCartesianAxis</c> builds every tick label from — sets auto-grow and the two adjusts and
    /// <em>no</em> <c>TextLeftDistance</c> or <c>TextUpperDistance</c> at all, and the SDR default
    /// for both is zero (<c>svx/source/svdraw/svdattr.cxx:247</c>). The two places that do set
    /// them set them explicitly and for their own reasons:
    /// <c>DataTableView::setCellCharAndParagraphProperties</c> at 0.18/0.30 of the font height,
    /// and <c>getPreparedTextShapePropertyLists</c> at a flat 250/125 1/100 mm for a shape that
    /// may show a border.
    /// </para>
    /// <para>
    /// <strong>It is the label's height that decides the rhythm, so 0.6 em of invented inset is
    /// worth a third of the labels.</strong> Two 45°-rotated labels clear each other as soon as
    /// their separation along the axis reaches <c>height × √2</c>, so an inset that inflates a
    /// 11.6 pt box to 17.6 pt raises the threshold from 16.4 pt to 24.9 pt. Measured on
    /// <c>Template Pilot Logbook JAR-FCL V3.0.xls</c>, whose date axis carries 679 ticks: the
    /// reference labels every 23rd and draws 30, and with the insets we labelled every 37th and
    /// drew 18.
    /// </para>
    /// </remarks>
    private static DocSize Shape(ChartText measurer, string text, Length size, bool bold = false)
    {
        if (text.AsSpan().IndexOf('\n') < 0) return measurer.Measure(text, size, bold);

        // A wrapped label is as wide as its widest line and as deep as the sum of them, which is
        // what a text shape with auto-grow-height comes out as.
        Length width = Length.Zero;
        Length height = Length.Zero;

        foreach (string line in text.Split('\n'))
        {
            DocSize one = measurer.Measure(line, size, bold);
            if (one.Width > width) width = one.Width;
            height += one.Height;
        }

        return new DocSize(width, height);
    }
}

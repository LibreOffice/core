using XRay.Office.Core.Units;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// The line a border is drawn as, once a format's own name for it has been resolved.
/// </summary>
/// <remarks>
/// These are LibreOffice's <c>SvxBorderLineStyle</c> members rather than WordprocessingML's
/// <c>ST_Border</c>, because Writer maps many of the latter onto one of the former and the reference
/// output is what a renderer has to reproduce: <c>triple</c>, <c>doubleWave</c> and
/// <c>dashDotStroked</c> all come out as a plain double rule, and every one of the 165 art borders
/// (<c>apples</c>, <c>pumpkin1</c>, <c>zanyTriangles</c>) comes out as nothing at all. See
/// <see cref="BorderRules.FromWord"/> for the map and <c>editeng/source/items/borderline.cxx</c>:105-190
/// for the original.
/// </remarks>
public enum BorderLine
{
    /// <summary>One unbroken rule the full stated width.</summary>
    Solid = 0,

    /// <summary>One rule of dots.</summary>
    Dotted,

    /// <summary>One rule of dashes.</summary>
    Dashed,

    /// <summary>Dashes with a small gap, and a floor of 1 pt on the width.</summary>
    FineDashed,

    /// <summary>Alternating dashes and dots.</summary>
    DashDot,

    /// <summary>A dash and two dots.</summary>
    DashDotDot,

    /// <summary>Two rules of equal width, one width apart.</summary>
    Doubled,

    /// <summary>A scaling rule, a fixed gap, and a fixed thin rule.</summary>
    ThinThickSmallGap,

    /// <summary>A fixed thin rule, a fixed gap, and a scaling rule.</summary>
    ThickThinSmallGap,

    /// <summary>Half the width, a quarter gap, a quarter rule.</summary>
    ThinThickMediumGap,

    /// <summary>A quarter rule, a quarter gap, half the width.</summary>
    ThickThinMediumGap,

    /// <summary>Two fixed rules with a scaling gap between them.</summary>
    ThinThickLargeGap,

    /// <summary>The same, the thicker rule inside.</summary>
    ThickThinLargeGap,

    /// <summary>A quarter, a quarter, and half the width as the gap.</summary>
    Embossed,

    /// <summary><inheritdoc cref="Embossed" path="/summary"/></summary>
    Engraved,

    /// <summary>A fixed thin rule outside a scaling one.</summary>
    Outset,

    /// <summary>A scaling rule outside a fixed thin one.</summary>
    Inset,
}

/// <summary>
/// One border's decomposition: the rule at its outer edge, the gap, and the rule inside that.
/// </summary>
/// <param name="Outer">The rule drawn at the border's outer edge. Never zero for a border that draws.</param>
/// <param name="Gap">The space between the two rules; zero when there is only one.</param>
/// <param name="Inner">The rule drawn on the text's side of the gap, or zero when there is only one.</param>
/// <remarks>
/// <em>Outer</em> is away from the text: the top of a top border, the left of a left one. Writer calls
/// these <c>line1</c>, <c>dist</c> and <c>line2</c> and the naming of the styles does not follow them —
/// <see cref="BorderLine.ThinThickSmallGap"/>'s <c>line1</c> is the one that scales with the stated
/// width, so at 3 pt it is the <em>thicker</em> of the two.
/// </remarks>
public readonly record struct BorderBands(Length Outer, Length Gap, Length Inner)
{
    /// <summary>True when the border draws two rules rather than one.</summary>
    public bool HasTwoRules => Inner > Length.Zero;

    /// <summary>The whole width across, which is the room the border takes.</summary>
    public Length Total => Outer + Gap + Inner;
}

/// <summary>
/// What a border's stated line style and width actually draw: how wide the whole rule is, and how it
/// is divided into strokes.
/// </summary>
/// <remarks>
/// <para>
/// A border's <c>w:sz</c> is not its drawn width. Word states the width of <em>a</em> line and the style
/// says how many there are, so a 3 pt <c>double</c> covers 9 pt of page — three bands of three — and a
/// 3 pt <c>thick</c> covers 6. Ignoring that both loses the second rule and shortens every paragraph
/// carrying one by two thirds of its border.
/// </para>
/// <para>
/// Measured before it was ported, on 56 authored documents varying the style and <c>w:sz</c>
/// (<c>probes/words-border-style/rules.py</c>), reading the strokes out of a 300 dpi raster and the
/// room out of the following text's own y. At <c>w:sz="24"</c> — 3 pt — 24.2.7.2 draws:
/// </para>
/// <list type="table">
///   <listheader><term>style</term><description>strokes, and the room taken</description></listheader>
///   <item><term>single</term><description>one of 3.12 pt; 3 pt of room</description></item>
///   <item><term>double, triple</term><description>3.12, a 2.88 gap, 3.12; 9 pt of room</description></item>
///   <item><term>thick</term><description>one of 6.0; 6 pt of room</description></item>
///   <item><term>thinThickSmallGap</term><description>3.12 then 0.72; 4.5 pt</description></item>
///   <item><term>thickThinSmallGap</term><description>0.72 then 2.88; 4.5 pt</description></item>
///   <item><term>outset</term><description>0.72 then 2.64; 6 pt</description></item>
///   <item><term>dotted, dashed, dotDash, wave</term><description>as single; 3 pt</description></item>
/// </list>
/// <para>
/// The arithmetic below is <c>editeng</c>'s own rather than a fit to those numbers, and reproduces all
/// of them: <c>ConvertBorderWidthFromWord</c> (<c>borderline.cxx</c>:204-265) turns the stated width
/// into the total, and <c>BorderWidthImpl</c> (<c>svtools/source/control/ctrlbox.cxx</c>:105-150) divides
/// the total into the two rules and the gap. Each of the three is either a constant in twips or a ratio
/// of the total, and a component that scales has the *other two* constants taken off it — which is what
/// makes <c>thinThickSmallGap</c>'s scaling rule come out as the stated width exactly.
/// </para>
/// </remarks>
public static class BorderRules
{
    /// <summary>Writer's own floor on the gap between two rules: 2 twips.</summary>
    /// <remarks><c>MINGAPWIDTH</c>, <c>svtools/source/control/ctrlbox.cxx</c>:74.</remarks>
    private const long MinimumGap = 2;

    /// <summary>0.75 pt, the fixed rule the three-dimensional and small-gap styles are built from.</summary>
    private const long ThinRule = 15;

    /// <summary>
    /// The line and the width a Word border style number and its stated <c>w:sz</c> come to, or null
    /// when the style draws nothing.
    /// </summary>
    /// <param name="wordStyle">
    /// The <c>brcType</c> a <c>BRC</c> states, which is also what <c>w:val</c> maps onto — see the map in
    /// the body. 0 is <c>none</c>, 255 is <c>nil</c>, and everything from 64 up is an art border.
    /// </param>
    /// <param name="stated">The stated width, <c>w:sz</c> eighths of a point or <c>BRC.dptLineWidth</c>.</param>
    /// <remarks>
    /// A stated width of nothing is 0.75 pt rather than nothing: <c>ConvertBorderWidthFromWord</c> opens
    /// by substituting 15 twips for a zero, which is what makes an RTF <c>\brdrs</c> with no
    /// <c>\brdrw</c> draw at all.
    /// </remarks>
    public static (BorderLine Line, Length Width)? FromWord(int wordStyle, Length stated)
    {
        BorderLine? line = LineOf(wordStyle);
        if (line is not { } style) return null;

        long width = stated.Twips <= 0 ? ThinRule : stated.Twips;

        long total = style switch
        {
            // `thick` and `hairline` are a single rule of a different width rather than a style of
            // their own — the one place where the Word number is still needed after the map.
            BorderLine.Solid => wordStyle switch
            {
                2 => width * 2,
                5 => Math.Max(width, 1),
                _ => width,
            },

            BorderLine.FineDashed => width is > 0 and < 20 ? 20 : width,
            BorderLine.Doubled => width * 3,
            BorderLine.ThinThickMediumGap or BorderLine.ThickThinMediumGap
                or BorderLine.Embossed or BorderLine.Engraved => width * 2,
            BorderLine.ThinThickSmallGap or BorderLine.ThickThinSmallGap => width + ThinRule + ThinRule,
            BorderLine.ThinThickLargeGap => width + 30 + ThinRule,
            BorderLine.ThickThinLargeGap => width + ThinRule + 30,
            BorderLine.Outset or BorderLine.Inset => (width * 2) + ThinRule,
            _ => width,
        };

        // The width handed back is what the border actually covers, which for `outset` and `inset` is
        // three quarters of a point less than the figure `ConvertBorderWidthFromWord` produces: their
        // scaling component is halved and then has the fixed rule taken off it, twice, so a twip of the
        // total goes nowhere. Writer charges the page the parts rather than the figure — a 3 pt `outset`
        // costs 6.05 pt in the probe, not 6.75 — and a caller that stored the figure would over-charge
        // every one of the 388 `outset` borders in `PAT-047` by 0.75 pt.
        return (style, Divide(style, total).Total);
    }

    /// <summary>
    /// The border style number a WordprocessingML <c>w:val</c> names.
    /// </summary>
    /// <remarks>
    /// <c>lcl_convertBorderStyleFromToken</c>, <c>sw/source/writerfilter/dmapper/ConversionHelper.cxx</c>
    /// :41-239. Only the 28 line styles are listed: the other 165 values are art borders, which map to
    /// 64 and above and which <see cref="FromWord"/> answers null for, so an unknown name lands there
    /// too rather than being guessed at as a plain rule. That is Writer's own reading — its map returns
    /// <c>none</c> for a token it does not know.
    /// </remarks>
    public static int WordStyleOf(string? value) => value switch
    {
        "nil" => 255,
        "single" => 1,
        "thick" => 2,
        "double" => 3,
        "dotted" => 6,
        "dashed" => 7,
        "dotDash" => 8,
        "dotDotDash" => 9,
        "triple" => 10,
        "thinThickSmallGap" => 11,
        "thickThinSmallGap" => 12,
        "thinThickThinSmallGap" => 13,
        "thinThickMediumGap" => 14,
        "thickThinMediumGap" => 15,
        "thinThickThinMediumGap" => 16,
        "thinThickLargeGap" => 17,
        "thickThinLargeGap" => 18,
        "thinThickThinLargeGap" => 19,
        "wave" => 20,
        "doubleWave" => 21,
        "dashSmallGap" => 22,
        "dashDotStroked" => 23,
        "threeDEmboss" => 24,
        "threeDEngrave" => 25,
        "outset" => 26,
        "inset" => 27,
        _ => 0,
    };

    /// <summary>
    /// How a border of this line and total width divides into its rules and the gap between them.
    /// </summary>
    public static BorderBands Bands(BorderLine line, Length width)
        // `width` is what the border covers; `BorderWidthImpl` divides the figure it was made from, and
        // for `outset` and `inset` the two differ by the fixed rule. Adding it back is a fixed point:
        // 121 twips becomes 136, whose bands are 15, 53 and 53 — 121 again.
        => Divide(line, width.Twips + (line is BorderLine.Outset or BorderLine.Inset ? ThinRule : 0));

    /// <summary>The three components of a border whose <em>stated</em> total is <paramref name="total"/>.</summary>
    /// <remarks>
    /// <c>BorderWidthImpl::GetLine1</c>, <c>GetLine2</c> and <c>GetGap</c>,
    /// <c>svtools/source/control/ctrlbox.cxx</c>:105-150.
    /// </remarks>
    private static BorderBands Divide(BorderLine line, long total)
    {
        (Ratio one, Ratio two, Ratio gap) = Shape(line);

        long outer = one.Of(total, Constant(two) + Constant(gap));
        long inner = two.Of(total, Constant(one) + Constant(gap));
        long between = gap.Of(total, Constant(one) + Constant(two));

        // Writer's own floor, and only where there really are two rules to keep apart.
        if (between < MinimumGap && one.Rate > 0 && two.Rate > 0) between = MinimumGap;

        // `fdo#51777`: a double border one twip wide would round its outer rule away entirely, and
        // Writer keeps a twip of it rather than drawing nothing.
        if (outer == 0 && one.Scales && one.Rate > 0 && total > 0) outer = 1;

        return inner > 0
            ? new BorderBands(Length.FromTwips(outer), Length.FromTwips(between), Length.FromTwips(inner))
            : new BorderBands(Length.FromTwips(outer), Length.Zero, Length.Zero);

        static long Constant(Ratio r) => r.Scales ? 0 : (long)r.Rate;
    }

    /// <summary>
    /// The unit Writer's dash patterns are counted in: 10 twips, half a point.
    /// </summary>
    /// <remarks>
    /// <c>fPatScFact</c> (<c>svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx</c>:600) times the
    /// style's own <c>PatternScale</c>, which is 1.0 for everything a document states, in the draw
    /// layer's units — twips for Writer.
    /// </remarks>
    private const long DashUnit = 10;

    /// <summary>
    /// The dash pattern a border of this line is drawn with, or null for an unbroken rule.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>svtools::GetDashing</c>, <c>svtools/source/control/ctrlbox.cxx</c>:248-282, scaled by
    /// <see cref="DashUnit"/>. The array starts with ink and ends with a gap.
    /// </para>
    /// <para>
    /// <strong>The lengths do not scale with the rule's width</strong>, which is the thing to know and
    /// the thing a reader guesses wrong: a dotted border is half-point dots a point apart whether the
    /// rule is a quarter point thick or three points. Measured at 600 dpi on the probe's own output,
    /// <c>w:sz</c> 8 against 24: <c>dotted</c> 0.48 pt of ink and 1.0 of gap at both, <c>dashed</c>
    /// 8.04 and 2.52 at both, <c>dashSmallGap</c> 3.0 and 1.0, <c>dotDash</c> 8.04, 2.52, 2.5, 2.52.
    /// <see cref="Core.Graphics.DashPresets"/> is the wrong source for the same reason — DrawingML's
    /// presets <em>are</em> multiples of the pen.
    /// </para>
    /// </remarks>
    public static IReadOnlyList<Length>? Dashes(BorderLine line)
    {
        long[]? pattern = line switch
        {
            BorderLine.Dotted => [1, 2],
            BorderLine.Dashed => [16, 5],
            BorderLine.FineDashed => [6, 2],
            BorderLine.DashDot => [16, 5, 5, 5],
            BorderLine.DashDotDot => [16, 5, 5, 5, 5, 5],
            _ => null,
        };

        return pattern is null ? null : [.. pattern.Select(n => Length.FromTwips(n * DashUnit))];
    }

    /// <summary>
    /// The Word border style number's line, or null when it draws nothing — <c>none</c>, <c>nil</c>, and
    /// every art border.
    /// </summary>
    /// <remarks>
    /// <c>ConvertBorderStyleFromWord</c>, <c>editeng/source/items/borderline.cxx</c>:136-190. The
    /// collapses are Writer's: it has no triple, no double wave and no thin-thick-thin, and says so in
    /// its own comments.
    /// </remarks>
    private static BorderLine? LineOf(int wordStyle) => wordStyle switch
    {
        1 or 2 or 5 or 20 => BorderLine.Solid,
        6 => BorderLine.Dotted,
        7 => BorderLine.Dashed,
        22 => BorderLine.FineDashed,
        8 => BorderLine.DashDot,
        9 => BorderLine.DashDotDot,
        3 or 10 or 21 or 23 => BorderLine.Doubled,
        11 => BorderLine.ThinThickSmallGap,
        12 or 13 => BorderLine.ThickThinSmallGap,
        14 => BorderLine.ThinThickMediumGap,
        15 or 16 => BorderLine.ThickThinMediumGap,
        17 => BorderLine.ThinThickLargeGap,
        18 or 19 => BorderLine.ThickThinLargeGap,
        24 => BorderLine.Embossed,
        25 => BorderLine.Engraved,
        26 => BorderLine.Outset,
        27 => BorderLine.Inset,
        _ => null,
    };

    /// <summary>
    /// The three components as <c>BorderWidthImpl</c> states them: outer rule, inner rule, gap.
    /// </summary>
    private static (Ratio One, Ratio Two, Ratio Gap) Shape(BorderLine line) => line switch
    {
        // fdo#46112, fdo#38542, fdo#43249: the varying widths must sum to one.
        BorderLine.Doubled => (Ratio.Scaled(1.0 / 3), Ratio.Scaled(1.0 / 3), Ratio.Scaled(1.0 / 3)),

        BorderLine.ThinThickSmallGap => (Ratio.Scaled(1.0), Ratio.Fixed(ThinRule), Ratio.Fixed(ThinRule)),
        BorderLine.ThickThinSmallGap => (Ratio.Fixed(ThinRule), Ratio.Scaled(1.0), Ratio.Fixed(ThinRule)),
        BorderLine.ThinThickMediumGap => (Ratio.Scaled(0.5), Ratio.Scaled(0.25), Ratio.Scaled(0.25)),
        BorderLine.ThickThinMediumGap => (Ratio.Scaled(0.25), Ratio.Scaled(0.5), Ratio.Scaled(0.25)),
        BorderLine.ThinThickLargeGap => (Ratio.Fixed(30), Ratio.Fixed(ThinRule), Ratio.Scaled(1.0)),
        BorderLine.ThickThinLargeGap => (Ratio.Fixed(ThinRule), Ratio.Fixed(30), Ratio.Scaled(1.0)),

        // Writer's comment: the widths follow 0.75 pt up to 3 pt and then 3 pt.
        BorderLine.Embossed or BorderLine.Engraved
            => (Ratio.Scaled(0.25), Ratio.Scaled(0.25), Ratio.Scaled(0.5)),

        BorderLine.Outset => (Ratio.Fixed(ThinRule), Ratio.Scaled(0.5), Ratio.Scaled(0.5)),
        BorderLine.Inset => (Ratio.Scaled(0.5), Ratio.Fixed(ThinRule), Ratio.Scaled(0.5)),

        // Every single-rule style, dashed and dotted included: the whole width, no second rule, no gap.
        _ => (Ratio.Scaled(1.0), Ratio.Fixed(0), Ratio.Fixed(0)),
    };

    /// <summary>One of the three components: either a constant in twips or a share of the total.</summary>
    private readonly record struct Ratio(double Rate, bool Scales)
    {
        public static Ratio Scaled(double rate) => new(rate, true);

        public static Ratio Fixed(long twips) => new(twips, false);

        /// <summary>
        /// The component's own width, the constants among the other two taken off it when it scales.
        /// </summary>
        public long Of(long total, long others)
            => Scales ? Math.Max(0, (long)((Rate * total) + 0.5) - others) : (long)Rate;
    }
}

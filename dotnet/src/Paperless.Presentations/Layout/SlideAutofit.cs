using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.Presentations.Layout;

/// <summary>
/// The shrink-to-fit half of <see cref="SlideTextLayout"/>: measure the text, find it too tall,
/// and re-measure at a smaller size until it fits.
/// </summary>
/// <remarks>
/// <para>
/// A port of <c>SdrTextObj::autoFitTextForCompatibility</c>
/// (<c>svx/source/svdraw/svdotext.cxx</c>), reached from <c>ImpAutoFitText</c> whenever a shape's
/// <c>SDRATTR_TEXT_FITTOSIZE</c> is <c>AUTOFIT</c> — which DrawingML spells
/// <c>a:bodyPr/a:normAutofit</c> and a SmartArt <c>tx</c> node gets whether it asks or not.
/// </para>
/// <para>
/// <strong>Three things about it are surprising, and all three are load-bearing.</strong>
/// </para>
/// <para>
/// <em>The stated scale is thrown away.</em> <c>a:normAutofit/@fontScale</c> is the answer the
/// authoring application arrived at, and the reference reads it into
/// <c>TextBodyProperties::mnFontScale</c> and then never reads that field again
/// (<c>oox/source/drawingml/textbodypropertiescontext.cxx:240</c>, LibreOffice 24.2) — so the fit
/// is always solved from scratch, against LibreOffice's own metrics rather than PowerPoint's.
/// Honouring the stated scale instead disagrees with the reference on every autofitted shape
/// whose author measured with different fonts, which in a corpus rendered against Carlito and
/// Caladea is all of them. <c>@lnSpcReduction</c> is not read at all.
/// </para>
/// <para>
/// <em>It is not the algorithm master has.</em> LibreOffice 25.2 replaced the search with a walk
/// down a fixed table of twelve scale levels
/// (<c>editeng/source/editeng/impedit3.cxx</c>, <c>constScaleLevels</c>), so reading the tree this
/// checkout holds — 27.2 alpha — describes an engine that is not the one producing the reference
/// renders. What is ported here is 24.2.7.2, the installed <c>soffice</c>, fetched by tag.
/// <strong>Check which version wrote the reference before porting anything out of this tree.</strong>
/// </para>
/// <para>
/// <em>The fit measures the same line box it draws.</em> A slide's line is 1.2 em whatever face
/// the text is set in (see <see cref="SlideTextLayout"/>), and the search compares that same
/// height — <c>CalcTextSizeNTP</c> is the ordinary formatter, not a separate metric. An earlier
/// reading had the fit measuring the face's own ascent plus descent; it came from a probe deck
/// whose first shape was the one under test, which is the state leak described below.
/// </para>
/// <para>
/// <em>The comparison is against the text height less 50 units of a hundredth of a millimetre.</em>
/// <c>aCurrentTextBoxSize.extendBy(0, -50)</c> — 1.417 pt of slack, which is what lets a single
/// 40 pt line sit in a 46.5 pt box unshrunk where 1.2 em alone would want 48.
/// </para>
/// <para>
/// <strong>The trap that cost an afternoon: the first shape on a slide measures differently.</strong>
/// LibreOffice's draw outliner is shared between objects and <c>SetFixedCellHeight</c> only
/// invalidates the formatting when the flag <em>changes</em>, so the first text object a page
/// lays out is formatted before the flag takes hold and draws its lines at the face's
/// ascent + descent rather than at 1.2 em. On a probe deck of eight boxes that made Liberation
/// Sans look like the one face whose autofit line height was its own metrics and the other three
/// look like 1.2 em; putting Liberation Serif first moved the anomaly to Liberation Serif. It is a
/// state leak in the reference, not a rule, and it is deliberately not reproduced — but any
/// measurement whose first shape is the one under test is measuring it.
/// </para>
/// </remarks>
public static partial class SlideTextLayout
{
    /// <summary>The slack the reference allows a text box, in hundredths of a millimetre.</summary>
    private const long FitSlackMm100 = 50;

    /// <summary>How many times the search halves its interval.</summary>
    private const int FitIterations = 10;

    /// <summary>The line-spacing scales the search may fall back on, in per cent.</summary>
    /// <remarks>
    /// Tried in order at each candidate font scale and abandoned as soon as one fits, so a shape
    /// that fits on font size alone never tightens its lines. They are part of the search even
    /// when the answer is 100: a 90 per cent solution at a larger font can be a closer fit than a
    /// 100 per cent one at a smaller font, and it is the closest fit that wins.
    /// </remarks>
    private static readonly double[] FitSpacings = [1.0, 0.9, 0.8];

    /// <summary>
    /// How a fit's answer is applied to a body: a font multiplier and a line-spacing multiplier.
    /// </summary>
    /// <param name="Font">The multiplier on every run's em size.</param>
    /// <param name="Spacing">The multiplier on every line's height, one for none.</param>
    /// <param name="RoundToPoints">
    /// Whether a scaled size is rounded to a whole point.
    /// <para>
    /// <c>Outliner::setRoundFontSizeToPt</c>, which the fit turns on and nothing else does
    /// (<c>svdotext.cxx</c>, "We need to round the font size nearest integer pt size"). It rounds
    /// twice — the run's own size to a whole point, then the scaled result to a whole point again
    /// (<c>editeng/source/editeng/impedit3.cxx:2993-2999</c> in 24.2) — which is why a shrunken
    /// size in a reference PDF is a whole number of points: 65 pt becomes 49, not 49.4.
    /// </para>
    /// </param>
    private readonly record struct Scaling(double Font, double Spacing, bool RoundToPoints)
    {
        /// <summary>No scaling at all.</summary>
        public static Scaling None { get; } = new(1.0, 1.0, false);

        /// <summary>The scaling a body states, for the paths that do not solve a fit.</summary>
        public static Scaling Stated(SlideTextBody body)
            => body.FontScale is > 0 and not 1.0 ? new(body.FontScale, 1.0, false) : None;

        /// <summary>A run's em size after the font multiplier.</summary>
        /// <remarks>
        /// <para>
        /// <strong>A whole number of points, held as a whole number of hundredths of a
        /// millimetre</strong> — and the second half of that is what decides the search, not just
        /// how the size is reported. <c>roundToNearestPt</c> converts to points, rounds, and
        /// converts back (<c>impedit4.cxx:3128</c>), and the caller then rounds the result to an
        /// integer of the draw layer's own unit, so 27 pt is 953 rather than 952.5 and the line
        /// it sits on is 1.2 × 953 = 1144, not 1143.6.
        /// </para>
        /// <para>
        /// That single unit matters because the search compares fits by how close to one they
        /// are. On a 40 pt line in a 32 pt box, 30 pt at nine-tenths spacing and 27 pt at full
        /// spacing are both 32.4 pt of text; quantised they are 1143 and 1144, the second is the
        /// closer fit by one hundredth of a millimetre, and the reference draws 27. Measured in
        /// exact points they tie, the earlier candidate keeps the prize, and we drew 30.
        /// </para>
        /// </remarks>
        public Length Scaled(Length size)
        {
            if (Font is <= 0 or 1.0) return Quantised(size);

            if (!RoundToPoints)
            {
                return Quantised(Length.FromEmu((long)Math.Round(size.Emu * Font)));
            }

            double points = Rounded((double)Quantised(size).Mm100 / Mm100PerPoint);

            return Length.FromMm100((long)Rounded(Rounded(points * Font) * Mm100PerPoint));
        }
    }

    /// <summary>
    /// A character height on the grid the draw layer can actually hold it on.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A slide's em size is never an exact number of points, and ours was.</strong> The
    /// height lives in an <c>SvxFontHeightItem</c> in the model's own map unit, which for a draw
    /// object is a hundredth of a millimetre — so a 20 pt run is drawn at
    /// <strong>706 units, 20.0126 pt</strong>, and every advance width, line break and autofit
    /// measurement in the reference is taken at that size rather than at 20.
    /// </para>
    /// <para>
    /// Measured on the round-seventeen baseline sweep with
    /// <c>research/probes/slides-r17/mm100-grid.py</c>: of the reference's show operators over
    /// forty documents, <strong>82.27% sit on the 1/100 mm grid against our 45.81%</strong>, and
    /// every one of the fifteen commonest sizes we wrote that it cannot hold is a whole number of
    /// points — 24, 16, 20, 12, 28, 17, 10, 9, 15, 44. The residual 18% on the reference's side is
    /// text it rasterises or plays out of a metafile, which is not on any grid by construction.
    /// </para>
    /// <para>
    /// The conversion is the one the property setter performs, not a direct ratio:
    /// <c>SvxFontHeightItem::PutValue</c> takes <c>nHeight = (long)(fPoint * 20.0 + 0.5)</c> —
    /// points to twips — and then <c>convertTwipToMm100</c>, which is
    /// <c>(n * 127 + 36) / 72</c> (<c>editeng/source/items/textitem.cxx:774-776</c>, 24.2.7.2).
    /// For a whole number of points the twip step is exact and the pair reduces to
    /// <c>o3tl::convert(pt, pt, mm100)</c>, which is what the PPT filter calls directly — so one
    /// implementation is faithful to all three readers. For a DrawingML <c>sz</c> of 1333 it is
    /// not: 13.33 pt is 267 twips and therefore <strong>471</strong> units, where the direct ratio
    /// gives 470.
    /// </para>
    /// <para>
    /// Applied here rather than in the three readers because this is the one place every measured
    /// and drawn em passes through — <c>LargestSize</c> reads it back off <c>RunStyle.Size</c>,
    /// the shaper takes it as <c>FormattedRun.EmSize</c>, and the sink writes it as <c>/Tf</c>.
    /// </para>
    /// </remarks>
    private static Length Quantised(Length size)
    {
        if (size.Emu <= 0) return size;

        long twips = (long)((size.Points * 20.0) + 0.5);

        return Length.FromMm100(((twips * 127) + 36) / 72);
    }

    /// <summary>Hundredths of a millimetre in a point, which is where the fit's rounding happens.</summary>
    private const double Mm100PerPoint = 2540.0 / 72.0;

    /// <summary>
    /// <c>basegfx::fround</c>: half away from zero, which is not what <c>Math.Round</c> does.
    /// </summary>
    /// <remarks>
    /// .NET rounds a half to the even neighbour, so 952.5 becomes 952 where the reference gets
    /// 953 — and 27 pt of text on a 953 line fits a box that 27 pt on a 952 line does not.
    /// </remarks>
    private static double Rounded(double value) => Math.Floor(value + 0.5);

    /// <summary>
    /// Solves a body's fit, or returns the scaling it states when it asks for none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The search is a bisection on the font scale between nothing and the stated maximum, ten
    /// halvings deep, exactly as the reference runs it — including the two details that decide
    /// where it lands. Each candidate is snapped <em>down</em> to a tenth of a point of the body's
    /// own font height before it is tried, so the scales visited are a grid rather than a
    /// continuum; and the answer kept is not the last one tried but the <em>closest fit at or
    /// above one</em> seen anywhere in the search, which is why the loop runs its full ten
    /// iterations rather than stopping when the interval is small.
    /// </para>
    /// <para>
    /// Measurements are memoised by the pair they are made at. The bisection converges long before
    /// its tenth iteration — on a 40 pt line the last four iterations all snap to the same tenth
    /// of a point — so the cache turns thirty measurements per shape into six or seven, which on a
    /// 48-node diagram is the difference between a second and half a minute.
    /// </para>
    /// </remarks>
    private static Scaling Solve(SlideTextBody body, DocRect area, SlideFonts fonts)
    {
        if (!body.AutoFit) return Scaling.Stated(body);
        if (area.Height <= Length.Zero) return Scaling.None;

        double fontHeightPoints = GridFontHeightPoints(body);

        Dictionary<(double, double), double> measured = [];

        // The reference measures the box with tools::Rectangle, whose GetSize() counts both edges
        // — bottom - top + 1 — so the height it fits against is one hundredth of a millimetre
        // more than the shape states.
        long available = area.Height.Mm100 + 1;

        double Fit(double font, double spacing)
        {
            if (measured.TryGetValue((font, spacing), out double cached)) return cached;

            long height =
                Measure(body, area.Width, fonts, new Scaling(font, spacing, true), body.FontIndependentLineSpacing)
                    .TotalToLastNonEmpty.Mm100
                - FitSlackMm100;

            double factor = height <= 0 ? double.MaxValue : (double)available / height;

            measured[(font, spacing)] = factor;
            return factor;
        }

        double current = Fit(1.0, 1.0);
        if (current >= 1.0) return Scaling.None;

        double minimum = 0.0;
        double maximum = 1.0;
        double bestFont = 0.0;
        double bestSpacing = 1.0;
        double best = current;

        for (int iteration = 0; iteration < FitIterations; iteration++)
        {
            double candidate = minimum + ((maximum - minimum) / 2);

            // Snapped down to a tenth of a point of the body's own font height, which is what
            // makes the search's grid the same grid the reference walks.
            double points = Math.Floor(fontHeightPoints * candidate * 10.0) / 10.0;
            double font = points / fontHeightPoints;

            double fit = 0.0;
            foreach (double spacing in FitSpacings)
            {
                if (fit >= 1.0) continue;

                fit = Fit(font, spacing);

                if (spacing == 1.0)
                {
                    if (fit > 1.0) minimum = font; else maximum = font;
                }

                if ((best < 1.0 && fit > best) || (fit >= 1.0 && fit < best))
                {
                    bestFont = font;
                    bestSpacing = spacing;
                    best = fit;
                }
            }
        }

        return bestFont > 0 ? new Scaling(bestFont, bestSpacing, true) : Scaling.None;
    }

    /// <summary>
    /// The pool default the reference falls back on: 24 pt, held as 847 hundredths of a
    /// millimetre.
    /// </summary>
    /// <remarks>
    /// <c>SdrEngineDefaults::GetFontHeight()</c> is <c>o3tl::convert(24, pt, mm100)</c>
    /// (<c>include/svx/svdetc.hxx:69</c>) and <c>SdrModel</c> makes it the pool's
    /// <c>EE_CHAR_FONTHEIGHT</c> (<c>svdmodel.cxx:133</c>, <c>SetTextDefaults</c>). 24 pt is
    /// 846.67 hundredths of a millimetre, which that conversion rounds to <strong>847</strong> —
    /// and the rounding is the point, not a detail. See <see cref="GridFontHeightPoints"/>.
    /// </remarks>
    private const long DefaultFontHeightMm100 = 847;

    /// <summary>
    /// The font height the search snaps its candidates to, in points.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The reference reads <c>EE_CHAR_FONTHEIGHT</c> off the <em>object's</em> item set and uses
    /// it for one purpose: each candidate scale is floored to a tenth of a point <em>of that
    /// height</em> before it is tried, so the search walks a grid of <c>0.1 / height</c> rather
    /// than a continuum.
    /// </para>
    /// <para>
    /// <strong>It is a length in hundredths of a millimetre, and it is therefore never a whole
    /// number of points.</strong> That is the whole of this method, and getting it wrong was
    /// worth six of thirty-three probe boxes. <c>autoFitTextForCompatibility</c> converts the
    /// item's height from hundredths of a millimetre
    /// (<c>svx/source/svdraw/svdotext.cxx</c>, 24.2.7), so a 20 pt default is 706 units and comes
    /// back as <strong>20.0126 pt</strong>, not 20. The difference decides which candidates the
    /// bisection ever visits: at the 87.5 per cent candidate a grid of exactly 12 puts the scaled
    /// size on precisely 17.5 pt, which rounds <em>up</em> to 18, overshoots the box, and drops
    /// the search's ceiling below every larger candidate; the reference's 11.99055 lands the same
    /// candidate on 17.489, rounds <em>down</em> to 17, and the search keeps climbing. Every
    /// disagreement measured was that shape — we settled for a looser fit than the reference.
    /// </para>
    /// <para>
    /// Measured on two probe decks of 33 autofit boxes each, one stating 20 pt and one 40 pt,
    /// simulating 24.2.7's search against its own rendering
    /// (<c>research/probes/slides-r15/sim-autofit.py</c>). A round twelve agrees on
    /// <strong>27 of 33 and 33 of 33</strong>; the body's own character height through hundredths
    /// of a millimetre agrees on <strong>33 and 33</strong>. The pool default alone — 847 units,
    /// 24.00945 pt — manages 33 and 30, which is what refutes reading a fixed default here.
    /// </para>
    /// <para>
    /// <strong>Which run's height, when a body states several, is not separated by any probe
    /// here.</strong> A deck putting a 20 pt paragraph in front of three 40 pt ones comes back
    /// 33 of 33 under either reading, so first-run and largest-run are indistinguishable on the
    /// evidence; the largest is taken because it is the more stable of the two — a body's leading
    /// run is as often a stray label as its dominant size. Treat that half as inferred.
    /// </para>
    /// <para>
    /// The predecessor of this note recorded the opposite conclusion — that a fixed twelve beat
    /// the run's own size, 225 probe boxes to 210. It was measured with the run's size in
    /// <em>points</em>, which for the 25, 32 and 40 pt boxes it used is a whole number every
    /// time, so what that experiment actually compared was two whole-point grids.
    /// </para>
    /// </remarks>
    private static double GridFontHeightPoints(SlideTextBody body)
    {
        long mm100 = 0;

        foreach (SlideParagraph paragraph in body.Paragraphs)
        {
            foreach (SlideTextRun run in paragraph.Runs)
            {
                if (run.Size.Mm100 > mm100) mm100 = run.Size.Mm100;
            }
        }

        return (mm100 > 0 ? mm100 : DefaultFontHeightMm100) / Mm100PerPoint;
    }
}

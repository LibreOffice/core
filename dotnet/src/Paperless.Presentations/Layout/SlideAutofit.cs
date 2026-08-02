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
            if (Font is <= 0 or 1.0) return size;

            if (!RoundToPoints)
            {
                return Length.FromEmu((long)Math.Round(size.Emu * Font));
            }

            double points = Rounded((double)size.Mm100 / Mm100PerPoint);

            return Length.FromMm100((long)Rounded(Rounded(points * Font) * Mm100PerPoint));
        }
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

        const double fontHeightPoints = GridFontHeightPoints;

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
    /// The font height the search snaps its candidates to, in points.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The reference reads <c>EE_CHAR_FONTHEIGHT</c> off the <em>object's</em> item set — the
    /// shape's default character height, not any run's — and uses it for one purpose: each
    /// candidate scale is floored to a tenth of a point <em>of that height</em> before it is
    /// tried, so the search walks a grid of <c>0.1 / height</c> rather than a continuum.
    /// Paperless models no such default, so this is EditEngine's own: 240 twips, twelve points
    /// (<c>editeng/source/editeng/eerdll.cxx:130</c>).
    /// </para>
    /// <para>
    /// <strong>The largest run's size is the wrong answer, and it is the tempting one.</strong>
    /// Measured over 227 probe boxes — a text box per box height from 8 to 100 pt at 25, 32 and
    /// 40 pt, one line and two, in four faces — taking the grid from the run's own size agrees
    /// with the reference on 210 and the fixed twelve agrees on 225. The failures are not near
    /// misses: a 40 pt line in a 20 pt box comes out at 17 pt against the reference's 19,
    /// because the grid decides which candidates the bisection ever visits and a grid derived
    /// from the run steps straight past the answer.
    /// </para>
    /// <para>
    /// Twelve is not the only value that works — 18 and 20 also agree on 225 of the 227, while
    /// 16 and 24 agree on 208. So the grid is load-bearing and its exact value, within a band,
    /// is not; twelve is chosen because it is the one the reference would actually hold for a
    /// shape that states no default of its own.
    /// </para>
    /// </remarks>
    private const double GridFontHeightPoints = 12.0;
}

using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;

namespace XRay.Office.Presentations.Layout;

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
/// <em>The stated scale is thrown away — in 24.2.</em> <c>a:normAutofit/@fontScale</c> is the
/// answer the authoring application arrived at, and 24.2 read it into
/// <c>TextBodyProperties::mnFontScale</c> and then never read that field again — so the fit was
/// always solved from scratch, against LibreOffice's own metrics rather than PowerPoint's.
/// Honouring the stated scale instead disagrees with the reference on every autofitted shape
/// whose author measured with different fonts, which in a corpus rendered against Carlito and
/// Caladea is all of them.
/// </para>
/// <para>
/// <strong>The source chain that reads it exists in 26.2.4.2 and the drawn answer still does not
/// follow it</strong> — the paragraph after next is the measurement, and it is the reason this
/// tree is <em>not</em> "behind" here. The chain, as round 85 read it:
/// <c>oox/source/drawingml/textbodypropertiescontext.cxx</c>:242-243 now sets
/// <c>PROP_TextFitToSizeFontScale</c> and <c>PROP_TextFitToSizeSpacingScale</c> from the element,
/// <c>SvxShape</c> puts them on the <c>SdrTextFitToSizeTypeItem</c>
/// (<c>svx/source/unodraw/unoshape.cxx</c>:2343-2367), and <c>setupAutoFitText</c> hands them
/// straight to the outliner rather than searching —
/// <c>if (fFontScale &gt; 0.0 &amp;&amp; fSpacingScale &gt; 0.0 &amp;&amp; !mbInEditMode)</c>,
/// <c>svx/source/svdraw/svdotext.cxx</c>:1238-1247 — after which
/// <c>ImpEditEngine::ScaleContentToFitWindow</c> walks <c>constScaleLevels</c> only if that first
/// format <em>overflows</em> (<c>editeng/source/editeng/impedit3.cxx</c>:303-333).
/// <strong>The gate on it is <c>@lnSpcReduction</c> and not <c>@fontScale</c></strong>: the
/// spacing scale is <c>1.0 - lnSpcReduction/100000</c> and the attribute's own default is 100000,
/// so an element stating <c>fontScale</c> alone yields a spacing scale of <strong>zero</strong>,
/// fails the guard, and is searched from scratch exactly as before. Censused over the corpus:
/// <strong>326 <c>a:normAutofit</c> state <c>fontScale</c> and 209 state <c>lnSpcReduction</c>,
/// the latter in 40 of the 251 <c>.pptx</c></strong>. A <c>.ppt</c> is unaffected either way:
/// <c>filter/source/msfilter/svdfppt.cxx</c>:1099 builds the item from its type alone and both of
/// its scales default to zero (<c>include/svx/sdtfsitm.hxx</c>:68-69).
/// </para>
/// <para>
/// <strong>Measured at the reference in round 94, and the drawn size does not follow the stated
/// pair at all — so this tree is right to search and must not be "caught up".</strong> Ten
/// one-attribute variants of each of two corpus decks, rendered through 26.2.4.2, drawn
/// <c>Tf</c> sizes read out of its own PDF (<c>probes/slides-ink-r94/fontscale-variants.tsv</c>):
/// on <c>171128IPAP.pptx</c> slide 31 the stated 90000, an absent attribute, 50000 and 25000 all
/// draw 29.991 pt, and so do all four of those beside a stated <c>lnSpcReduction</c> — which is
/// the arm this paragraph predicts would bite. Removing the <c>a:normAutofit</c> element outright
/// is the only variant that moves anything, to 32.003. On
/// <c>NWD-GLA-Community-Outreach-Day-Oct-2025.pptx</c> slide 5, whose own element states
/// <c>fontScale="25000" lnSpcReduction="20000"</c>, nine variants including
/// <c>fontScale="100000" lnSpcReduction="10000"</c> draw the identical 13.011 and 14.995 pt, and
/// the unscaled sizes are 51.987 and 60.009 — so what is drawn is <c>constScaleLevels</c>' last
/// row, 0.250, and not the file's number.
/// </para>
/// <para>
/// <strong>The trap in measuring it is that PowerPoint's stored scale is near the answer the
/// search finds</strong>, because both are fitting the same text to the same box: at the file's
/// own value the two hypotheses agree, and only an absurd variant separates them. And even where
/// the pair does arrive, <c>ScaleContentToFitWindow</c> seeds from it, formats <em>once</em>, and
/// walks the table from row 0 whenever that format overflows — so it is a first guess a single
/// overflow discards, never an override. <c>@lnSpcReduction</c> is read nowhere here and does not
/// need to be.
/// </para>
/// <para>
/// <em>It is not a search.</em> LibreOffice 25.2 replaced the bisection with a walk down a fixed
/// table of twelve <c>(font, spacing)</c> levels (<c>editeng/source/editeng/impedit3.cxx</c>,
/// <c>constScaleLevels</c>): format unscaled, and while that overflows take the <em>first</em>
/// [24.2.7-audit: FIXED 2026-08-20, round slides-r52 — was WRONG: — 25.2 replaced the bisection with
/// constScaleLevels; worth -155.40 abs_ink, -11.1% of the slides track.]
/// level that fits. What stood here for thirty rounds was the bisection of 24.2.7.2, which was
/// the installed <c>soffice</c> when it was written and stopped being it when this container
/// moved to <strong>26.2.4.2</strong> — the comment saying so was in this file the whole time.
/// The table is now what is implemented; see <see cref="FitLevels"/> for the 36-deck measurement
/// that establishes it against the installed binary rather than against this tree.
/// <strong>Check which version wrote the reference before porting anything out of this tree.</strong>
/// </para>
/// <para>
/// <strong>The level's second column is a line-spacing scale, and its first row uses it at full
/// font size.</strong> So the reference's first answer to an overflow is not a smaller font, it
/// is tighter leading and tighter paragraph spacing at the size the file states — which is the
/// half of "the text sizes are different" that reads as inter-paragraph spacing rather than as
/// size, and which two blind reviewers separated from the size question independently.
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
    /// <summary>
    /// The twelve <c>(font, spacing)</c> rows the fit may answer with, in the order it tries them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>constScaleLevels</c>, <c>editeng/source/editeng/impedit3.cxx</c>:286. The search formats
    /// once unscaled, and if that overflows walks this table from the top and keeps the
    /// <strong>first</strong> row that fits — so the answer is one of eleven font scales and never
    /// anything between them, and each font scale carries its own line-spacing scale.
    /// </para>
    /// <para>
    /// <strong>Measured against the installed 26.2.4.2, not read out of the tree.</strong>
    /// 36 one-slide decks, one variable — box height 60…480 pt — each in its own file so the
    /// reference's shared-outliner state leak cannot reach it, a single 40 pt paragraph in a
    /// 360 pt box (<c>dotnet/probes/slides-r52/make-fit-probe.py</c>). The nine distinct sizes the
    /// reference draws are exactly <c>40 ×</c> the first nine rows and nothing else, the spacing
    /// beside them is 0.90 above 0.85 and 0.80 at and below it, and <em>both</em> 0.850 rows
    /// appear: a 228 pt box takes <c>{0.850, 0.900}</c> and a 216 pt box <c>{0.850, 0.800}</c>.
    /// </para>
    /// <para>
    /// <strong>Row 0 is not a no-op.</strong> Its font scale is one and its spacing scale is 0.9,
    /// so the reference's first answer to an overflow is to tighten the leading and the paragraph
    /// spacing at full size — which is the other half of "the text sizes are different": at a
    /// 168 pt box both sides draw 28 pt and the reference's baseline pitch is 26.90 against our
    /// 33.62.
    /// </para>
    /// <para>
    /// This replaces the bisection of 24.2.7.2 that stood here for thirty rounds. That search
    /// snapped its candidates to a tenth of a point of the body's own character height and kept
    /// the closest fit at or above one, which is why it could answer with any whole point at all;
    /// on the grid above it agreed with 26.2.4.2 on 13 of 36 boxes. Its <c>GridFontHeightPoints</c>
    /// — the body's largest run height taken through hundredths of a millimetre, worth 33 of 33
    /// probe boxes against a round twelve's 27 — was a property of that grid and has gone with it.
    /// </para>
    /// <para>
    /// The 50 unit slack the 24.2 comparison allowed
    /// (<c>aCurrentTextBoxSize.extendBy(0, -50)</c>) has gone too, and that is measured rather
    /// than inferred from the function's disappearance: stepping the box 330…340 pt at one point,
    /// the reference stops scaling at <strong>exactly 336</strong> and scales at 335, where
    /// 1.417 pt of slack would have put the boundary at 334.
    /// </para>
    /// </remarks>
    private static readonly (double Font, double Spacing)[] FitLevels =
    [
        (1.000, 0.900),
        (0.925, 0.900),
        (0.850, 0.900),
        (0.850, 0.800),
        (0.775, 0.800),
        (0.700, 0.800),
        (0.625, 0.800),
        (0.550, 0.800),
        (0.475, 0.800),
        (0.400, 0.800),
        (0.325, 0.800),
        (0.250, 0.800),
    ];

    /// <summary>The smallest font scale a fit may answer with; below it the reference gives up.</summary>
    /// <remarks>
    /// <para>
    /// <strong>An autofitted body never shrinks past a quarter — it overflows instead.</strong>
    /// <see cref="FitLevels"/>' last row is <c>{0.250, 0.800}</c> and the walk stops there whether
    /// or not the text fits, so a placeholder holding far more text than it has room for is drawn
    /// at a quarter size and allowed to run past its own bottom edge.
    /// </para>
    /// <para>
    /// That is not merely "too small" for the search this replaced; it was <em>nothing</em>. The
    /// bisection's interval was <c>[0, 1]</c> and a body overflowing twentyfold drove it into the
    /// thousandths, where <see cref="Scaling.Scaled"/>'s rounding to a whole point rounds the em
    /// to <strong>0</strong> and the page receives no text-showing operator for the body at all.
    /// Measured on <c>NWD-GLA-Community-Outreach-Day-Oct-2025.pptx</c>, whose slides 5, 6 and 12
    /// each put seventeen paragraphs of 52–88 pt text in a 1152128 EMU (90.7 pt) subtitle: we drew
    /// the title and nothing else, where 26.2.4.2 draws the body at stated × 0.250 exactly —
    /// 60 pt → 15, 52 → 13, 88 → 22, 72 → 18, 77 → 19 (<c>/F 18.992 Tf</c> in its page 12 stream).
    /// The table reproduces all seven of those sizes through our own metrics, which is the sense
    /// in which it subsumes the clamp rather than contradicting it.
    /// </para>
    /// </remarks>
    private const double FitFloor = 0.250;

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

            // ImpEditEngine::SeekCursor, impedit3.cxx:3005-3012. The height it scales is the
            // one it read back off the device, and both roundings happen in hundredths of a
            // millimetre rather than in points. The order is load-bearing: at a stated 30 pt the
            // reference draws 25 at level 0.850 and 17 at level 0.550, and 25.5 rounding down
            // while 16.5 rounds up is what 1058.333... x 0.85 = 25.49999999999999 and
            // 1058.333... x 0.55 = 16.50000000000000 give. Multiplying in points gives 26 and 17.
            double height = RoundedToPoints(DeviceRealised(Quantised(size)).Mm100);

            return Length.FromMm100((long)Rounded(RoundedToPoints(height * Font)));
        }
    }

    /// <summary>
    /// The height a bullet or number is drawn at: the run's own height through the fit's font
    /// scale and the marker's relative size, with <em>no</em> rounding to whole points.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>Outliner::ImpCalcBulletFont</c> (<c>editeng/source/outliner/outliner.cxx:851-855</c>)
    /// is one multiplication and one <c>basegfx::fround</c>, taken on the paragraph font's height
    /// in the model's own map unit:
    /// <c>fround(aStdFont.GetFontSize().Height() × GetBulletRelSize()/100 × fFontY)</c>. The
    /// whole-point rounding <see cref="Scaling.Scaled"/> transcribes belongs to
    /// <c>Outliner::setRoundFontSizeToPt</c>, which sizes the text portions and which the bullet
    /// path never reaches.
    /// </para>
    /// <para>
    /// So a fitted paragraph draws its bullet and its text at <em>different</em> sizes, and that
    /// pair is what identifies the rule rather than either figure alone. Measured on
    /// <c>slides/done-006/ppt/Lepore.ppt</c> page 2 — one stated 24 pt body, a fit of 0.850 — the
    /// reference draws eleven text baselines at <strong>20.013 pt</strong>, which is
    /// <c>round(24 × 0.85) = 20</c>, and six bullets at <strong>20.409 pt</strong>, which is
    /// <c>fround(847 × 0.85) = 720</c> hundredths of a millimetre.
    /// </para>
    /// </remarks>
    private static Length ScaledMarker(Scaling scaling, Length runSize, double relative)
    {
        double factor = (scaling.Font is > 0 ? scaling.Font : 1.0)
                        * (relative is > 0 ? relative : 1.0);

        return factor == 1.0
            ? Quantised(runSize)
            : Length.FromMm100((long)Rounded(Quantised(runSize).Mm100 * factor));
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
    /// [24.2.7-audit: VERIFIED 2026-08-21, round 60 — and the <em>citation</em> has moved while
    /// the arithmetic has not. <c>convertTwipToMm100</c> no longer spells <c>(n * 127 + 36) / 72</c>
    /// out; it delegates to <c>o3tl::convert(n, twip, mm100)</c>
    /// (<c>include/tools/UnitConversion.hxx:23-26</c>), whose positive branch is
    /// <c>(n * num + den / 2) / den</c> with <c>num = 127</c> and <c>den = 72</c> — the same
    /// expression, <c>den / 2</c> being the 36. <c>SvxFontHeightItem::PutValue</c> still rounds
    /// through twips with <c>fPoint * 20.0 + 0.5</c>
    /// (<c>editeng/source/items/textitem.cxx:943, 976</c>). The worked case still divides the two
    /// readings: 13.33 pt is 267 twips, <c>(267 * 127 + 36) / 72 = 471</c> against the direct
    /// ratio's 470. Nothing to change; recorded so the next reader is not sent to a line that no
    /// longer holds the formula.]
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

    /// <summary>
    /// <c>ImpEditEngine::roundToNearestPt</c>: a length in hundredths of a millimetre, rounded to
    /// a whole number of points and converted straight back, still as a double.
    /// </summary>
    /// <remarks>
    /// <c>o3tl::convert</c> multiplies before it divides, and the residue of that decides the
    /// two cases where a level lands a size on exactly half a point — see <see cref="Scaling.Scaled"/>.
    /// Keeping the expression in this shape is therefore not a style choice.
    /// </remarks>
    private static double RoundedToPoints(double mm100)
        => Rounded(mm100 * 72.0 / 2540.0) * 2540.0 / 72.0;

    /// <summary>The draw layer's reference device resolution, which is what quantises an em.</summary>
    private const double ReferenceDeviceDpi = 600.0;

    /// <summary>
    /// The em an autofitted body's lines are measured at: the reference device's realisation of
    /// the character height, which is a whole number of its pixels.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An autofitted shape does not measure its lines at the size it states, and a plain
    /// one does.</strong> <c>ImpEditEngine::SeekCursor</c> takes a different branch whenever
    /// <c>maStatus.DoStretch()</c> — which <c>SdrTextObj::ImpSetupDrawOutlinerForPaint</c> sets for
    /// <c>IsFitToSize() || IsAutoFit()</c> and for nothing else. That branch pushes the font at the
    /// device, <em>reads the size back out of the device's own metric</em> and puts it on the font:
    /// <c>rFont.SetPhysFont(*pDev); Size aRealSz(aMetric.GetFontSize()); … rFont.SetFontSize(aRealSz)</c>
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:2985-3062, 24.2.7). During formatting <c>pDev</c>
    /// is the reference device, so the height the line spacing is computed from is the item height
    /// rounded to whole device pixels and back.
    /// </para>
    /// <para>
    /// Measured rather than inferred, on <c>research/probes/slides-r21/make-pitch-probe.py</c> —
    /// one slide per size, the same three paragraphs in an <c>a:noAutofit</c> box and in an
    /// <c>a:normAutofit</c> box far too tall to shrink. Over 53 sizes from 6 to 58 pt the plain box
    /// is <c>fround(em × 1.2)</c> every time and the autofitted box differs on <strong>34 of the
    /// 53</strong>, by −3, −1, +1 or +3 hundredths of a millimetre and never more. Fitting a pixel
    /// round trip over 30 to 4000 dpi reproduces all 53 at <strong>600 dpi and at no other
    /// resolution</strong>; eight further fractional sizes, four of them with the two boxes
    /// disagreeing, come back 8 of 8; and Carlito reproduces Liberation Sans row for row, which is
    /// what "font-independent" line spacing should do.
    /// </para>
    /// <para>
    /// <strong>It reaches only the unscaled case, and that is the reference's own condition rather
    /// than a simplification.</strong> When <c>fFontY != 1.0</c> the same branch immediately puts
    /// the height through <c>roundToNearestPt</c> twice (<c>impedit3.cxx</c>:3007-3012), which
    /// rounds to a whole point and discards the device grid entirely — so <see cref="Scaling.Scaled"/>
    /// is already faithful for every shrunken body and must not be touched.
    /// </para>
    /// <para>
    /// <strong>Do not read the em off a reference PDF's <c>/Tf</c>.</strong> There is a second,
    /// unrelated round trip at paint time through the PDF export device at 720 dpi, and it applies
    /// to plain and autofitted shapes alike: a 13.33 pt run is held as 471 units, measured at 470
    /// and drawn as <strong>473</strong>. Three different numbers for one size is what makes this
    /// term look like noise from the content stream alone.
    /// </para>
    /// </remarks>
    private static Length DeviceRealised(Length em)
    {
        if (em.Mm100 <= 0) return em;

        double pixels = Rounded(em.Mm100 * ReferenceDeviceDpi / 2540.0);

        return Length.FromMm100((long)Rounded(pixels * 2540.0 / ReferenceDeviceDpi));
    }

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
    /// <c>ImpEditEngine::ScaleContentToFitWindow</c> (<c>impedit3.cxx</c>:303-333): format once
    /// unscaled, and while the formatted height overflows the box walk <see cref="FitLevels"/>
    /// from the top, taking the first row that fits. There is no search and no interval — the
    /// answer is a row of the table or, when nothing fits, its last row.
    /// </para>
    /// <para>
    /// <strong>The comparison is <c>height &gt; box</c>, so equality fits, and there is no
    /// slack.</strong> The box is one hundredth of a millimetre taller than the shape states
    /// because the reference measures it with <c>tools::Rectangle::GetSize()</c>, which counts
    /// both edges.
    /// </para>
    /// <para>
    /// Measurements are memoised by the pair they are made at, as they were for the bisection this
    /// replaces; the walk visits at most thirteen and usually two or three.
    /// </para>
    /// </remarks>
    private static Scaling Solve(SlideTextBody body, DocRect area, SlideFonts fonts)
    {
        if (!body.AutoFit) return Scaling.Stated(body);
        if (area.Height <= Length.Zero) return Scaling.None;

        // The reference measures the box with tools::Rectangle, whose GetSize() counts both edges
        // — bottom - top + 1 — so the height it fits against is one hundredth of a millimetre
        // more than the shape states.
        long available = area.Height.Mm100 + 1;

        bool Fits(double font, double spacing)
            => Measure(body, area.Width, fonts, new Scaling(font, spacing, true),
                       body.FontIndependentLineSpacing)
                   .TotalToLastNonEmpty.Mm100 <= available;

        if (Fits(1.0, 1.0)) return Scaling.None;

        foreach ((double font, double spacing) in FitLevels)
        {
            if (Fits(font, spacing)) return new Scaling(font, spacing, true);
        }

        // The walk stops at the last row whether or not the text fits, and the body overflows.
        (double floorFont, double floorSpacing) = FitLevels[^1];

        return new Scaling(floorFont, floorSpacing, true);
    }
}

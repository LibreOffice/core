using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbering;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Itemisation;
using XRay.Office.Text.Layout;
using XRay.Office.Text.Shaping;

namespace XRay.Office.Presentations.Layout;

/// <summary>
/// Lays a shape's text body out inside its text rectangle.
/// </summary>
/// <remarks>
/// <para>
/// Line breaking, indents and horizontal alignment all come from <c>XRay.Office.Text</c>'s
/// <see cref="ParagraphLayouter"/>, which is the same engine the word processor uses and had
/// better stay so. What this adds is the two things a slide does differently: where the baselines
/// sit, and where the block as a whole sits inside the shape.
/// </para>
/// <para>
/// <strong>A slide's line height is a fraction of the font size, not of the font's metrics.</strong>
/// The PPTX importer sets <c>FontIndependentLineSpacing</c> on every text body it reads
/// (<c>oox/source/ppt/pptshapecontext.cxx:186</c>), and EditEngine then computes the line's
/// ascent as the font height outright and its descent as
/// <c>ImplCalculateFontIndependentLineSpacing(height) − ascent</c>
/// (<c>editeng/source/editeng/impedit3.cxx:3138-3141</c>), where that function is
/// <c>fround(height × 12 / 10)</c> (<c>impedit3.cxx:501-505</c>). So the baseline is one em below
/// the top of the line and the next line is 1.2 em further down — whatever face the text is set
/// in.
/// </para>
/// <para>
/// That is worth stating precisely because it is not a small difference. Liberation Sans reports
/// an ascent of 0.905 em, so a reader using the font's metrics puts an 18 pt first baseline
/// 16.30 pt below the text top where LibreOffice puts it at 18.00 — a point and a half, on every
/// line of every shape. Measured on <c>shape-geometry.pptx</c> slide 3: LibreOffice's PDF draws
/// the first text box's only line at 89.972 pt down a page whose shape starts at 71.972, and its
/// middle-anchored box's baseline at 259.172, which is the block-height arithmetic above to
/// within 0.014 pt.
/// </para>
/// </remarks>
public static partial class SlideTextLayout
{
    /// <summary>
    /// The numerator and denominator of EditEngine's font-independent line height.
    /// </summary>
    /// <remarks>
    /// Kept as the fraction rather than as 1.2 so the rounding matches: LibreOffice rounds the
    /// product to a whole unit of its own layout resolution, and a double multiply followed by a
    /// separate round is the same arithmetic.
    /// </remarks>
    private const double LineHeightFactor = 12.0 / 10.0;

    /// <summary>
    /// The weight a character bullet's face is asked for at, whatever the text it labels.
    /// </summary>
    /// <remarks>
    /// Both producers of a bullet font are normal: <c>nBulletFontWeight</c> starts and stays at
    /// <c>css::awt::FontWeight::NORMAL</c> in the oox import
    /// (<c>oox/source/drawingml/textparagraphproperties.cxx:315</c>) and
    /// <c>SdStyleSheetPool::GetBulletFont</c> sets <c>WEIGHT_NORMAL</c>
    /// (<c>sd/source/core/stlpool.cxx:1174</c>).
    /// </remarks>
    private const int MarkerWeight = 400;

    /// <summary>
    /// Lays a body out and returns its glyph runs, positioned in the given rectangle's space.
    /// </summary>
    /// <param name="body">The text body.</param>
    /// <param name="textRectangle">
    /// The shape's text rectangle, in whatever space the caller wants the runs in — the shape's
    /// own for a rotated shape, the slide's for an upright one. The insets are applied here.
    /// </param>
    /// <param name="fonts">The face cache.</param>
    /// <param name="markerPictures">
    /// Where to put the picture bullets, when the caller draws them. A picture marker is not a
    /// glyph run and cannot travel in the return value; passing null lays the text out exactly as
    /// before and simply does not collect them, which is what a caller measuring a height wants.
    /// </param>
    public static List<PlacedGlyphRun> Place(
        SlideTextBody body,
        DocRect textRectangle,
        SlideFonts fonts,
        List<PlacedPicture>? markerPictures = null)
    {
        ArgumentNullException.ThrowIfNull(body);
        ArgumentNullException.ThrowIfNull(fonts);

        // Before anything is measured, because a symbol run's recode changes the glyph and
        // therefore its advance, and an advance decided after the line break is decided too late.
        body = SlideSymbolRuns.Normalise(body, fonts);

        DocRect area = OnGrid(textRectangle).Deflate(OnGrid(body.Insets));
        List<PlacedGlyphRun> placed = [];
        if (body.Paragraphs.Count == 0) return placed;

        (List<Block> blocks, Length total, Length toLastNonEmpty) =
            Measure(body, area.Width, fonts, Solve(body, area, fonts), body.FontIndependentLineSpacing);

        if (blocks.Count == 0) return placed;

        // A shape that solves a fit is anchored by the same height the fit measured, so trailing
        // empty paragraphs push nothing down; one that does not is anchored by its whole text.
        //
        // Measured on the subtitle of `BMFE-06-03 (Gerflor) Smoke Density and Toxicity.pptx`,
        // three bottom-anchored paragraphs of which the last is empty. Deleting that paragraph
        // from LibreOffice's own flat-ODF export of the deck leaves the remaining line at
        // byte-identical coordinates while the shape autofits, and moves it 33 pt — a line and its
        // space — once `style:shrink-to-fit` is turned off. So this is a property of the fit
        // rather than of empty paragraphs, and applying it everywhere would move every
        // middle-anchored and bottom-anchored box that ends in a blank line.
        Length anchored = body.AutoFit ? toLastNonEmpty : total;

        Length top = area.Y + body.Anchor switch
        {
            TextAnchor.Middle => (area.Height - anchored) / 2,
            TextAnchor.Bottom => area.Height - anchored,
            _ => Length.Zero,
        };

        for (int index = 0; index < blocks.Count; index++)
        {
            Block block = blocks[index];

            if (index != 0) top += block.SpaceBefore;

            IReadOnlyList<PlacedLine> lines = block.Lines;

            for (int line = 0; line < lines.Count; line++)
            {
                if (line == 0)
                {
                    EmitMarker(
                        placed, markerPictures, block, lines[0], area.X, top, fonts, body.Device);
                }

                Emit(placed, block, lines[line], area.X, top, line == 0);

                // A field's continuation line is one *ascent* below its predecessor, not one line
                // height: EditEngine draws it inside the line that holds the field, and the only
                // distance it has to move by is `pLine->GetMaxAscent()`
                // (`editeng/source/editeng/impedit3.cxx`:3778-3795, whose comment says so —
                // "pLine->GetHeight() will not proceed as needed ... a compressed look").
                // See `ContinuesField`.
                top += line + 1 < lines.Count && lines[line + 1].ContinuesField
                    ? lines[line].Ascent
                    : lines[line].Height;
            }

            if (index != blocks.Count - 1) top += block.SpaceAfter;
        }

        return placed;
    }

    /// <summary>
    /// A text rectangle on the grid the draw layer can actually hold it on.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The companion of <see cref="Quantised(Length)"/>, which put the em on that grid, and the
    /// other half of the same defect: <strong>a shape's rectangle is an integer number of
    /// hundredths of a millimetre in the reference and carried the file's full EMU precision
    /// here.</strong> oox builds a shape's matrix in EMUs and scales it into hundredths of a
    /// millimetre at the end (<c>oox/source/drawingml/shape.cxx</c>:1226-1230), and
    /// <c>SvxShape</c> then hands the result to <c>SdrObject::SetSnapRect</c>, whose
    /// <c>tools::Rectangle</c> holds four <c>sal_Int32</c> in the model's map unit. A
    /// <c>SdrTextObj</c>'s text rectangle is that rectangle less four
    /// <c>SdrMetricItem</c> text distances, which are integers of the same unit.
    /// </para>
    /// <para>
    /// <strong>Both edges are rounded, not the extent</strong>, which is why this cannot be
    /// written as a rounding of the width and height: the reference's height is
    /// <c>round(bottom) − round(top)</c>, so a box of a given extent is one unit taller or
    /// shorter depending on <em>where on the slide it sits</em>. Rounding the extent alone
    /// reproduces neither.
    /// </para>
    /// <para>
    /// The absolute error this removes is at most half a unit an edge — 0.014 pt, an order of
    /// magnitude below the em fix. What makes it worth having anyway is that the shrink-to-fit
    /// search is a <em>threshold</em> comparison rather than a proportional one: the answer is
    /// decided by whether the measured text clears <c>available</c>, and round seventeen pinned
    /// one such decision to a window 16 units wide. A quantity that is only ever one or two units
    /// out still lands on the wrong side of a boundary that close.
    /// </para>
    /// <para>
    /// Applied in <see cref="Place"/> and deliberately not in <see cref="Height"/>, whose caller
    /// is a table's row sizing and hands over a width it has already computed from column edges
    /// rather than a rectangle. Quantising a width on its own is not this rule — the rule is
    /// about two edges — and a table's own grid is a separate measurement.
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, round slides-r53 — probed against the
    /// installed 26.2.4.2, not read from the C++; the claim still holds.]
    /// <strong>Re-checked against 26.2.4.2 on 2026-08-21</strong> (`TODO.24-2-7-audit.md`), by
    /// probe rather than by reading: twelve boxes whose top edge steps by 40 EMU — one ninth of a
    /// unit — from 1944.000 to 1945.222 hundredths of a millimetre. The reference draws its first
    /// baseline at exactly <em>two</em> values across all twelve, 444.9260 pt for the five tops at
    /// or below 1944.444 and 444.8980 pt for the seven at or above 1944.556. The step is 0.0280 pt,
    /// which is one unit, and the transition sits on the half — so the edge is quantised, and by
    /// <c>round</c> rather than by truncation. The claim holds unchanged
    /// (<c>probes/slides-r53/results.md</c>).
    /// </para>
    /// </remarks>
    private static DocRect OnGrid(DocRect rectangle)
    {
        long left = rectangle.Left.Mm100;
        long top = rectangle.Top.Mm100;

        return new DocRect(
            Length.FromMm100(left),
            Length.FromMm100(top),
            Length.FromMm100(rectangle.Right.Mm100 - left),
            Length.FromMm100(rectangle.Bottom.Mm100 - top));
    }

    /// <summary>The four text distances, each an integer of the model's own unit.</summary>
    private static Margins OnGrid(Margins insets) => new(
        Length.FromMm100(insets.Left.Mm100),
        Length.FromMm100(insets.Top.Mm100),
        Length.FromMm100(insets.Right.Mm100),
        Length.FromMm100(insets.Bottom.Mm100));

    /// <summary>
    /// How tall a body's text is once broken to a width, insets excluded.
    /// </summary>
    /// <remarks>
    /// The measurement a table row needs and nothing else does: a row's stated <c>a:tr/@h</c> is a
    /// <em>minimum</em>, and LibreOffice grows the row to its tallest cell's content
    /// (<c>svx/source/table/tablelayouter.cxx:1026-1029</c>). Sharing the measurement with
    /// <see cref="Place"/> rather than approximating it is the point: a row that grows must grow
    /// by exactly what the text then occupies, or the cell's own baselines land somewhere else.
    /// </remarks>
    /// <param name="body">The text body.</param>
    /// <param name="width">The width available for the lines, inside the insets.</param>
    /// <param name="fonts">The face cache.</param>
    public static Length Height(SlideTextBody body, Length width, SlideFonts fonts)
    {
        ArgumentNullException.ThrowIfNull(body);
        ArgumentNullException.ThrowIfNull(fonts);

        // The same normalisation Place does, and for the same reason: a cell whose height this
        // decides must be measured over the glyphs that are actually going to be drawn in it.
        body = SlideSymbolRuns.Normalise(body, fonts);

        return Measure(body, width, fonts, Scaling.Stated(body), body.FontIndependentLineSpacing)
            .Total;
    }

    /// <summary>
    /// Breaks every paragraph of a body and totals their heights.
    /// </summary>
    /// <remarks>
    /// <strong>The outer two spacings do not count.</strong> A paragraph's space-before is not
    /// applied to the first paragraph of a body and its space-after is not applied to the last —
    /// <c>ImpEditEngine::CalcHeight</c>, <c>editeng/source/editeng/impedit2.cxx:4792-4802</c>,
    /// which guards the upper with <c>if (nPortion)</c> and the lower with
    /// <c>if (nPortion != lastIndex())</c> under the comment "not in the last". Paragraph
    /// spacing is therefore a gap <em>between</em> paragraphs and never padding inside the box.
    /// Applying it at the ends as well grows the block, and a middle-anchored node then draws its
    /// only line off centre: on <c>tdf125551.pptx</c>, whose diagram paragraphs each state
    /// <c>spcAft</c> of 35% on 32 pt text, every label moved 5.6 pt — half of the 11.2 pt that
    /// the trailing space added — until this matched LibreOffice.
    /// </remarks>
    /// <param name="Blocks">The paragraphs, measured and broken.</param>
    /// <param name="Total">Every paragraph's height, which is what the block occupies.</param>
    /// <param name="TotalToLastNonEmpty">
    /// The height down to the bottom of the last paragraph that has text, which is what the
    /// shrink-to-fit search measures against. See <see cref="HeightToLastNonEmpty"/>.
    /// </param>
    private readonly record struct Measurement(
        List<Block> Blocks, Length Total, Length TotalToLastNonEmpty);

    private static Measurement Measure(
        SlideTextBody body,
        Length available,
        SlideFonts fonts,
        Scaling scaling,
        bool fontIndependentLineSpacing)
    {
        // wrap="none" is expressed as an effectively unbounded width rather than as clipping,
        // which is what keeps an unwrapped label on the single line its author saw.
        Length width = body.Wraps && available > Length.Zero
            ? available
            : Length.FromEmu(int.MaxValue);

        List<Block> blocks = [];
        Length total = Length.Zero;

        for (int index = 0; index < body.Paragraphs.Count; index++)
        {
            SlideParagraph paragraph = body.Paragraphs[index];
            Block? block = Measure(
                paragraph, body, width, fonts, scaling, fontIndependentLineSpacing, index,
                body.Wraps ? null : available);
            if (block is null) continue;

            total += block.Height;
            blocks.Add(block);
        }

        if (blocks.Count != 0)
        {
            total -= blocks[0].SpaceBefore;
            total -= blocks[^1].SpaceAfter;
        }

        return new Measurement(blocks, total, HeightToLastNonEmpty(blocks));
    }

    /// <summary>
    /// The height down to the bottom of the last paragraph that has text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The shrink-to-fit search measures this rather than the whole block, because the reference
    /// does: <c>autoFitTextForCompatibility</c> calls <c>Outliner::CalcTextSizeNTP</c>
    /// (<c>svx/source/svdraw/svdotext.cxx:1293,1358</c>), whose height comes from
    /// <c>ImpEditEngine::Calc1ColumnTextHeight</c> — which records the running bottom offset only
    /// while the paragraph it is looking at is not empty:
    /// </para>
    /// <code>
    /// if (pHeightNTP &amp;&amp; !rInfo.rPortion.IsEmpty())
    ///     *pHeightNTP = nHeight;
    /// </code>
    /// <para>
    /// <em>NTP</em> is "no trailing paragraphs", and the asymmetry is the whole point: an empty
    /// paragraph in the <em>middle</em> still counts, because a later paragraph with text sets the
    /// bottom to an offset that already includes it. Only a run of empty paragraphs at the end is
    /// dropped. Measured against LibreOffice 26.2.4.2 on
    /// <c>slides/batch-002/ppt/gfopportunitiesforlinkagespres_2010_en.ppt</c>, whose eighth slide
    /// carries four empty paragraphs after its three bullets: the reference fits that text at
    /// 25 pt, and moving three of those empty paragraphs into the middle of the body makes the
    /// same LibreOffice fit it at 21 pt with nine-tenths line spacing — which is exactly what
    /// XRay.Office produced for the untouched deck while it measured every paragraph.
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, round slides-r53 — probed against the
    /// installed 26.2.4.2, not read from the C++; the claim still holds.]
    /// <strong>Re-checked against 26.2.4.2 on 2026-08-21</strong> (`TODO.24-2-7-audit.md`), on an
    /// authored deck rather than on the corpus document the original figure came from: one 40 pt
    /// three-paragraph body in one 240 pt autofit box, four slides differing only in where its four
    /// empty paragraphs sit. Four at the end and none at all both fit at <strong>18.992 pt</strong>
    /// over twelve lines; three of them moved into the middle, and all four in the middle, both fit
    /// at <strong>15.987 pt</strong> over nine. Trailing empty paragraphs are still dropped from
    /// the measured height and interior ones are still counted, and the two arrangements are still
    /// a whole table row apart.
    /// </para>
    /// </remarks>
    private static Length HeightToLastNonEmpty(List<Block> blocks)
    {
        int last = blocks.Count - 1;
        while (last >= 0 && blocks[last].Paragraph.Text.Length == 0) last--;
        if (last < 0) return Length.Zero;

        Length height = Length.Zero;
        for (int index = 0; index <= last; index++) height += blocks[index].Height;

        // The same two exclusions the full total makes, against the truncated run: a body's first
        // paragraph gets no space-before, and the paragraph the measurement ends at contributes no
        // space-after because the next paragraph's top is where that gap would be spent.
        height -= blocks[0].SpaceBefore;
        height -= blocks[last].SpaceAfter;
        return height;
    }

    /// <summary>
    /// Draws a paragraph's bullet or number, on its first line and at its own pen.
    /// </summary>
    /// <remarks>
    /// <para>
    /// At <c>marL + indent</c>, which for the usual hanging indent is where the text would have
    /// started and where the text no longer does. Its own run rather than a prefix on the
    /// paragraph's, because it is a different face at a different size and it does not wrap.
    /// </para>
    /// <para>
    /// <strong>And it does not sit on the text's baseline.</strong> A bullet is
    /// <em>centred against the line's text</em>: <c>Outliner::ImpCalcBulletArea</c> puts its box
    /// at <c>firstLineHeight − firstLineTextHeight/2 − bulletHeight/2</c> below the paragraph's
    /// top and <c>Outliner::StripBullet</c> then draws it from that box's bottom less the bullet
    /// font's descent — which is its top plus the bullet's <em>ascent</em>
    /// (<c>editeng/source/outliner/outliner.cxx:1464-1467,946-955</c>). So the offset from the
    /// text's baseline is
    /// <c>lineHeight − textHeight/2 + (markerAscent − markerDescent)/2 − lineAscent</c>, and with
    /// single spacing that reduces to aligning the two faces' half-way marks.
    /// </para>
    /// <para>
    /// <strong>A generated number is not.</strong> The same function branches on
    /// <c>SVX_NUM_CHAR_SPECIAL</c> and draws everything else at the text's own baseline, which is
    /// why <see cref="SlideMarker.IsSymbol"/> exists — see its remarks for the measurement.
    /// </para>
    /// <para>
    /// Measured on two decks that had both drifted the same way for the same reason.
    /// <c>deck-features.pptx</c>'s 28 pt outline under the font-independent rule gives
    /// 1186 − 593 + 106.5 − 988 = −288.5 hundredths of a millimetre, which is 8.176 pt above the
    /// text; LibreOffice draws it 8.19 above. <c>slides-features.odp</c>'s same-sized outline
    /// under the face's own metrics gives 1103 − 551.5 + 106.5 − 894 = −236, which is 6.690 pt;
    /// LibreOffice draws it 6.718 above. The bullet's own metrics carry most of it, and the face
    /// they come from is <em>OpenSymbol</em> in both — a StarBats or a Wingdings bullet resolves
    /// there, and its hhea ascent and descent of 1420 and 442 on a 2048 em are why
    /// <c>(markerAscent − markerDescent)/2</c> is 106.5 for a 12.6 pt marker in both files.
    /// </para>
    /// </remarks>
    /// <summary>
    /// A line re-aligned against the shape's real width, for a body that does not wrap.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>wrap="none"</c> body is laid out at an effectively unbounded width, because that is
    /// what keeps an unwrapped label on the one line its author saw. The width used for breaking
    /// must not also be the width used for <em>aligning</em>: a centred paragraph measured against
    /// two billion EMUs is offset by half of it, which is a mile off the right-hand edge of the
    /// page. Measured on <c>SRDMG(16)024_60 GHz onboard airplanes.pptx</c>, whose layout carries a
    /// centred, unwrapped strapline: six words lost from all 27 pages.
    /// </para>
    /// <para>
    /// The offset is allowed to go <em>negative</em>, unlike the wrapping case where a line wider
    /// than its stretch starts at the left edge. An unwrapped shape is one Impress gives
    /// <c>TextAutoGrowWidth</c>, so it widens about its own centre and its text overhangs both
    /// edges equally: the reference draws that strapline from 519.00 pt to 708.32 pt in a box
    /// running 520.90 to 706.50 — 1.9 pt proud at each end, which is exactly half the excess.
    /// </para>
    /// </remarks>
    private static LineBox Realigned(
        LineBox box, ParagraphFormat format, Length? alignAgainst, bool isFirstLine)
    {
        if (alignAgainst is not { } width) return box;

        Length start = format.LineStart(isFirstLine);
        Length slack = width - start - box.Line.Width;

        Length offset = format.Alignment switch
        {
            TextAlignment.Centre => Length.FromEmu(slack.Emu / 2),
            TextAlignment.End => slack,
            _ => Length.Zero,
        };

        return box with { Left = start + offset };
    }

    private static void EmitMarker(
        List<PlacedGlyphRun> placed,
        List<PlacedPicture>? markerPictures,
        Block block,
        PlacedLine line,
        Length areaLeft,
        Length top,
        SlideFonts fonts,
        MetricGrid device)
    {
        if (MarkerPicture(block.Paragraph) is { } bitmap)
        {
            EmitMarkerPicture(markerPictures, block, line, areaLeft, top, bitmap);
            return;
        }

        if (Shaped(block.Paragraph, block.Scaling, fonts) is not
            { Face: { } face, Shaped: { } shaped } marked)
        {
            return;
        }

        SlideMarker marker = marked.Marker;
        SlideTextRun first = block.Paragraph.Runs[0];
        Length size = marked.Size;

        Length pen = areaLeft + block.Paragraph.StartIndent + block.Paragraph.FirstLineIndent;

        Length baseline = top + line.Ascent;

        if (marker.IsSymbol)
        {
            LineMetrics metrics = LineSpacing.Resolve(face, device);

            // The box's height is the bullet at the paragraph's UNSCALED size; the descent taken
            // off the bottom of it is the drawn size's. See BulletBoxHeight.
            long box = BulletBoxHeight(metrics, first, marker).Mm100;
            Length descent = Rounded(metrics.ScaledDescent(size));

            // Outliner::ImpCalcBulletArea's vertical, in the hundredth of a millimetre it is
            // computed in — Top = H - TH + TH/2 - box/2, Bottom = Top + box - 1, where the -1 is
            // tools::Rectangle's own both-edges convention (include/tools/gen.hxx:597) — and
            // Outliner::StripBullet then takes the descent off Bottom to reach the baseline
            // (outliner.cxx:1461-1467, :951-956).
            long height = line.Height.Mm100;
            long text = line.TextHeight.Mm100;
            long bottom = height - text + (text / 2) - (box / 2) + box - 1;

            baseline = top + Length.FromMm100(bottom) - descent;
        }

        placed.Add(new PlacedGlyphRun(
            Build(shaped, marked.Text, size, marked.Reference ?? Reference(face),
                  new DocPoint(pen, baseline), Length.Zero),
            marker.Colour ?? first.Colour));
    }

    /// <summary>
    /// Places a picture bullet in the box <c>ImpCalcBulletArea</c> computes for it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The vertical is the same expression a character symbol takes and stops one step earlier: a
    /// bitmap is drawn from the box's <em>top</em> at the graphic's own size —
    /// <c>DrawBulletInfo(*pFmt-&gt;GetBrush()-&gt;GetGraphicObject(), aBulletPos,
    /// pPara-&gt;GetBulletSize())</c> with <c>aBulletPos.Y = rStartPos.Y + aBulletArea.Top()</c>
    /// (<c>editeng/source/outliner/outliner.cxx</c>:965-999) — where a character is drawn from
    /// its bottom and then lifted by its descent. <c>SVX_NUM_BITMAP</c> is excluded from
    /// <c>ImpCalcBulletArea</c>'s baseline branch (<c>:1470</c>), so the box is the centred one in
    /// both cases.
    /// </para>
    /// <para>
    /// It costs no glyph and no shaping, which is the whole reason it is a separate path: the
    /// character route would recode the level's bullet slot, resolve a face for it and draw
    /// whatever that face holds — on a deck whose level is a bitmap, a Private Use code point no
    /// installed face has, at half again the text's size. See <see cref="SlideMarkerPicture"/>.
    /// </para>
    /// </remarks>
    private static void EmitMarkerPicture(
        List<PlacedPicture>? markerPictures,
        Block block,
        PlacedLine line,
        Length areaLeft,
        Length top,
        SlideMarkerPicture picture)
    {
        if (markerPictures is null) return;
        if (picture.Width <= Length.Zero || picture.Height <= Length.Zero) return;

        Length pen = areaLeft + block.Paragraph.StartIndent + block.Paragraph.FirstLineIndent;

        long height = line.Height.Mm100;
        long text = line.TextHeight.Mm100;
        long box = picture.Height.Mm100;
        long boxTop = height - text + (text / 2) - (box / 2);

        markerPictures.Add(new PlacedPicture(
            picture.Image,
            new DocRect(pen, top + Length.FromMm100(boxTop), picture.Width, picture.Height))
        {
            Vector = picture.Vector,
            IsInline = true,
        });
    }

    /// <summary>
    /// The height of the box a character bullet is centred in: the bullet's ascent plus descent
    /// at the paragraph's <em>unscaled</em> size, whatever the fit scaled the drawn one to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>Outliner::ImpCalcBulletArea</c> takes the box's height from <c>ImplGetBulletSize</c>,
    /// which sets <c>ImpCalcBulletFont</c>'s font on the reference device and reads
    /// <c>GetTextHeight()</c> back — and <strong>caches the answer on the paragraph</strong>
    /// (<c>editeng/source/outliner/outliner.cxx</c>:1315-1355). The autofit search formats the
    /// same outliner once unscaled and then again at each row of <c>constScaleLevels</c>
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:303-333), so the call that fills that cache is
    /// the <em>unscaled</em> one and the shrunken passes read it back. The bullet is still
    /// <em>drawn</em> at the scaled size, because <c>StripBullet</c> calls
    /// <c>ImpCalcBulletFont</c> again for the font it paints with (<c>:906-913</c>) and that one
    /// does multiply by <c>getScalingParameters().fFontY</c> (<c>:851-855</c>). So a fitted
    /// paragraph centres a small bullet in a large box.
    /// </para>
    /// <para>
    /// <strong>This checkout has the cache keyed on the scaling parameters and would not do
    /// it</strong> — <c>IsBulletInvalid</c> compares a stored <c>ScalingParameters</c>
    /// (<c>include/editeng/outliner.hxx</c>:154-172) — and the checkout declares
    /// <c>27.2.0.0.alpha0+</c>. <strong>26.2.4.2 does do it</strong>, and that is measured rather
    /// than read: <c>probes/slides-r100/make-bullet-fit-probe.py</c> is one 24 pt bulleted body
    /// per slide in a box swept 60…420 pt, so the search answers a different row each time. The
    /// ten slides it does not scale agree with 26.2.4.2 to <strong>0.057 pt</strong>; on the six
    /// it does, the reference's bullet is 1.615 to 6.533 pt lower than the box-centred rule puts
    /// it, and the residual is
    /// <c>(1 − fontScale) × unscaledBulletHeight / 2</c> at every one —
    /// 231, 203, 144, 115, 58 and 58 hundredths of a millimetre against that expression's 231.0,
    /// 202.1, 144.4, 115.5, 57.8 and 57.8, over five distinct font scales. The drawn bullet size,
    /// the drawn text size and the body's baseline pitch agree exactly on all sixteen, so nothing
    /// but this term is in question.
    /// </para>
    /// <para>
    /// Reach: it is the whole of round 99's O28 baseline residual. On the 263 corpus pages whose
    /// show sequence agrees with 26.2.4.2's <em>and</em> whose every body show sits within 0.10 pt
    /// of the reference's baseline, the bullets beyond 0.10 pt go from <strong>315 of 1243 to
    /// 20</strong>.
    /// </para>
    /// </remarks>
    private static Length BulletBoxHeight(
        LineMetrics metrics, SlideTextRun first, SlideMarker marker)
    {
        // The cache was filled before the search, so with no font scale at all — the marker's own
        // relative size is not the search's and was already in it.
        Length unscaled = ScaledMarker(Scaling.None, first.Size, marker.Scale);

        return Rounded(metrics.ScaledAscent(unscaled)) + Rounded(metrics.ScaledDescent(unscaled));
    }

    /// <summary>
    /// A paragraph's marker shaped and sized, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Shared by the placement and by the width the first line has to clear, so the two cannot
    /// disagree about how wide the bullet is — which would put the text a fraction of a point
    /// inside it or a fraction clear of it on every bulleted line in the deck.
    /// </para>
    /// <para>
    /// <b>A paragraph with no text draws no marker</b>, and both of LibreOffice's presentation
    /// readers say so in as many words. <c>oox/source/drawingml/textparagraph.cxx:193-197</c>
    /// — "empty paragraphs do not have bullets in ppt" — sets <c>NumberingLevel</c> to −1 when
    /// the paragraph's runs came to nothing, and
    /// <c>filter/source/msfilter/svdfppt.cxx:2363-2366</c> — "in PPT empty paragraphs never gets
    /// a bullet" — puts <c>EE_PARA_BULLETSTATE</c> false on the same condition. Both fire on the
    /// paragraph's own character count and neither looks at what the level says, so an author's
    /// blank line between two bullets is a blank line rather than a bare bullet.
    /// </para>
    /// <para>
    /// Measured across the slides track by counting extracted lines holding nothing but a bullet
    /// glyph: <b>75 of the 163 documents drew more of them than the reference, 2405 lines in
    /// all</b> — 293 on <c>2015-Civil-Rights-Website-training.ppt</c>, 185 on
    /// <c>71393_pp7.ppt</c>, 170 on <c>171128IPAP.pptx</c>. Both families, so it is this layout
    /// rather than either reader.
    /// </para>
    /// <para>
    /// <strong>But the bullet it does not draw still has a box, and that box is a floor on the
    /// line's height</strong> — which is what <c>requireText: false</c> is for. The floor comes
    /// off <c>Outliner::GetBulletArea</c>, and that reaches <c>Outliner::GetNumberFormat</c>
    /// (<c>editeng/source/outliner/outliner.cxx</c>:1289-1312), which asks the paragraph's
    /// <em>depth</em> and its numbering rule and never looks at <c>EE_PARA_BULLETSTATE</c>. So
    /// suppressing the glyph does not remove the box. See <see cref="BulletFloored"/>.
    /// </para>
    /// <para>
    /// <strong>A character bullet whose file names no face is drawn from OpenSymbol, not from the
    /// paragraph's face.</strong> <c>Outliner::ImpCalcBulletFont</c> takes
    /// <c>pFmt-&gt;GetBulletFont()</c> for a <c>SVX_NUM_CHAR_SPECIAL</c> format and only falls back
    /// to the paragraph's own font when there is none
    /// (<c>editeng/source/outliner/outliner.cxx:828-847</c>) — and for Impress there always is one,
    /// because the numbering rule is built over <c>SdStyleSheetPool::GetBulletFont</c>, which is
    /// OpenSymbol at normal weight (<c>sd/source/core/stlpool.cxx:1169-1183</c>). The readers hand
    /// a null <see cref="SlideMarker.Typeface"/> up for exactly the cases where nothing was named:
    /// a DrawingML paragraph with no <c>a:buFont</c> in its chain or with <c>a:buFontTx</c>, a
    /// SmartArt node, a binary outline whose level names no font.
    /// </para>
    /// <para>
    /// Measured through <c>soffice</c> 26.2.4.2 on a probe deck of five paragraphs in one body,
    /// read out of the PDF's own text operators: <c>a:buChar</c> with no <c>a:buFont</c> over text
    /// in Courier New draws the bullet from <b>OpenSymbol</b>, the same over text in Times New
    /// Roman draws it from <b>OpenSymbol</b>, <c>a:buFontTx</c> draws it from <b>OpenSymbol</b>,
    /// <c>a:buFont typeface="Arial"</c> draws it from <b>Liberation Sans</b>, and an
    /// <c>a:buAutoNum</c> with no font draws its number from <b>Liberation Mono</b> — the text's
    /// own face, which is the fallback branch above and the reason this switches on
    /// <see cref="SlideMarker.IsSymbol"/>.
    /// </para>
    /// <para>
    /// <strong>The weight and the slope come from the bullet's face, not from the text's.</strong>
    /// Both sources of a bullet font are normal and upright by construction — the oox import writes
    /// <c>nBulletFontWeight = FontWeight::NORMAL</c> into the descriptor it pushes
    /// (<c>oox/source/drawingml/textparagraphproperties.cxx:315</c>) and
    /// <c>SdStyleSheetPool::GetBulletFont</c> sets <c>WEIGHT_NORMAL</c> and <c>ITALIC_NONE</c>. On
    /// the same probe, a bold paragraph whose <c>a:buFont</c> is Arial draws its text from
    /// Liberation Sans Bold and its bullet from Liberation Sans; its <c>a:buAutoNum</c> sibling
    /// draws the number from Liberation Sans Bold, with the text.
    /// </para>
    /// </remarks>
    private static MarkedParagraph? Shaped(
        SlideParagraph paragraph, Scaling scaling, SlideFonts fonts, bool requireText = true)
    {
        if (paragraph.Marker is not { } marker) return null;
        if (marker.Text.Length == 0) return null;
        if (requireText && paragraph.Text.Length == 0) return null;
        if (paragraph.Runs.Count == 0) return null;

        SlideTextRun first = paragraph.Runs[0];

        // A character bullet has a face of its own whether or not the file names one, and it is
        // never bold and never italic. A generated number has neither of those: it is drawn in
        // the paragraph's own font, weight and slope included.
        string? family = marker.IsSymbol
            ? marker.Typeface ?? SymbolFontRecode.SubstituteFamily
            : marker.Typeface ?? first.Typeface;
        int weight = marker.IsSymbol ? MarkerWeight : first.Weight;
        bool italic = !marker.IsSymbol && first.IsItalic;

        (OpenTypeFace? face, FontReference? reference) = fonts.Resolve(family, weight, italic);

        if (face is null) return null;

        string text = marker.Text;
        if (Recoded(marker, reference) is { } recoded)
        {
            // The recode and the face go together: the code point means nothing anywhere but
            // OpenSymbol, so a resolution that failed leaves both alone rather than drawing it
            // out of whatever the request happened to land on.
            (OpenTypeFace? symbol, FontReference? symbolReference) = fonts.Resolve(
                SymbolFontRecode.SubstituteFamily, weight, italic);

            if (symbol is not null)
            {
                (face, reference, text) = (symbol, symbolReference, recoded);
            }
        }

        if (ReferenceEquals(text, marker.Text))
        {
            text = OutlineNumbers.NormaliseBullet(marker.Text);
        }

        // A marker whose face has no glyph for it falls back exactly as a run does.
        //
        // <strong>A marker resolves on its own path and never asked.</strong> The run path hands
        // `SlideFonts.Fallback` to the shared layouter (`ItemisationOptions.GlyphFallback`, below),
        // so a character Carlito cannot draw is drawn from something that can; this method shapes
        // its one character against whatever `Resolve` answered and draws `.notdef` when that face
        // does not hold it -- which for a face that declines to draw a missing-glyph box is
        // nothing at all, and the marker simply vanishes. It is invisible to every text
        // comparison, because the recoded code point still reaches the PDF with a ToUnicode.
        //
        // It bites hardest on a *recoded* symbol slot, because the recode's target is chosen from
        // LibreOffice's table rather than from OpenSymbol's coverage and the two do not agree.
        // Measured on `slides/done-013/ppt/FAA_Form_337.ppt` page 4: five Monotype Sorts slots
        // 0xB6-0xBA recode to U+2776-U+277A (`aMonotypeSortsTab`'s F0B0 block), OpenSymbol has a
        // glyph for none of the five, and the reference draws all five circled numerals out of
        // <b>DejaVu Sans</b> -- its page's /F1 -- while we drew five .notdefs and no marker at all.
        // The same holds for `Wingdings 2` slot 0xF4, which recodes to the Private Use U+E5CD that
        // no installed face has.
        //
        // Where it looks depends on the face it is looking from. A marker that landed on OpenSymbol
        // is a pi face, and LibreOffice never hands one to fontconfig -- only to
        // `ImplInitGenericGlyphFallback`'s fixed list, which is what still finds DejaVu Sans for the
        // five slots above and what correctly finds *nothing* for a recode target no listed family
        // holds. See `IGlyphFallbackResolver.SymbolFallbackFor`.
        if (First(text) is { } wanted
            && face is { } resolved
            && !resolved.HasGlyphFor(wanted)
            && (SymbolFontRecode.IsSubstituteFamily(resolved.FamilyName)
                    ? fonts.Fallback.SymbolFallbackFor(wanted, weight, italic)
                    : fonts.Fallback.FallbackFor(wanted, weight, italic, resolved)) is { } substitute)
        {
            face = substitute;
            reference = fonts.Fallback.ReferenceFor(substitute, italic) ?? reference;
        }

        // The marker shrinks with the text it labels: the fit scales the whole outliner, and a
        // bullet left at its authored size on a node scaled to a third overwhelms its own line.
        //
        // But it does NOT take the run's rounding. A run's size goes through
        // Outliner::setRoundFontSizeToPt, which rounds the scaled height to a whole point twice;
        // the bullet is sized by Outliner::ImpCalcBulletFont, which the fit never reaches:
        //
        //     double fFontScaleY = pFmt->GetBulletRelSize() / 100.0 * getScalingParameters().fFontY;
        //     double fScaledLineHeight = aStdFont.GetFontSize().Height() * fFontScaleY;
        //     aBulletFont.SetFontSize(Size(0, basegfx::fround(fScaledLineHeight)));
        //
        // (editeng/source/outliner/outliner.cxx:851-855.) One multiplication, one fround, and it
        // is taken on the item's own height in hundredths of a millimetre -- so a fitted bullet
        // is not a whole number of points where its text is.
        //
        // Measured on `slides/done-006/ppt/Lepore.ppt` page 2, whose body states 24 pt and whose
        // fit answers 0.850: the reference draws the TEXT at 20.013 pt -- round(24 x 0.85) = 20 --
        // and the six BULLETS on the same page at 20.409, which is 847 x 0.85 = 719.95 -> 720
        // hundredths of a millimetre, 24 x 0.85 unrounded. Two sizes on one page, from one stated
        // size, and the pair is what identifies the rule.
        Length size = ScaledMarker(scaling, first.Size, marker.Scale);

        ShapedText shaped = TextShaper.Default.Shape(face, text, default);
        return shaped.Glyphs.Count == 0
            ? null
            : new MarkedParagraph(marker, text, face, reference, size, shaped);
    }

    /// <summary>
    /// A symbol marker's slot turned into the glyph the resolved face actually holds.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A symbol face is a set of glyph slots, and its slot numbers mean nothing to any
    /// other face.</strong> Both readers hand the slot over in the Private Use Area, where a
    /// symbol-encoded font really maps it. What happens next depends on what the request resolved
    /// to, which is why it is decided here and not in either reader.
    /// </para>
    /// <para>
    /// <strong>The trigger is that the face itself is absent, not that the request happened to
    /// resolve to OpenSymbol.</strong> When the face is installed, the slot is drawn from it
    /// unchanged. When it is not — and Wingdings, Webdings and Monotype Sorts are not fonts Linux
    /// has — LibreOffice substitutes OpenSymbol and recodes, whose F000–F0FF coverage is ten code
    /// points, so drawing the slot there instead would be <c>.notdef</c>.
    /// </para>
    /// <para>
    /// Keying on the resolved family was the first reading and it was too narrow. It works for
    /// the faces <c>VCL.xcu</c> happens to give a substitution chain — Wingdings' names
    /// <c>opensymbol</c> fourth — and silently fails for the ones it does not: nothing in that
    /// table mentions <c>monotypesorts</c> or <c>mtextra</c>, so those went to fontconfig and came
    /// back as a text face. LibreOffice never asks fontconfig about a symbol font at all
    /// (<c>FcPreMatchSubstitution::FindFontSubstitute</c> returns false outright for one,
    /// <c>vcl/unx/generic/font/fontsubst.cxx:100-107</c>), which is why the absence of a chain
    /// costs it nothing. Caught by the fixture, where Monotype Sorts drew U+2022 while the
    /// reference drew the glyph.
    /// </para>
    /// <para>
    /// Returns null when nothing should change, which leaves the caller to collapse whatever is
    /// left in the Private Use Area to U+2022 — a symbol face with no table, or one whose own
    /// file is installed — exactly as this layout did for every symbol bullet before the tables
    /// existed.
    /// </para>
    /// </remarks>
    /// <summary>The first code point of a marker's text, or null when it has none.</summary>
    /// <remarks>
    /// A marker is one character in every case the readers produce -- a <c>a:buChar</c> slot, a
    /// PPT <c>bulletChar</c>, or a generated number's first digit -- and only the first decides
    /// whether the face can draw it, which is the question a fallback answers.
    /// </remarks>
    private static int? First(string text)
    {
        if (text.Length == 0) return null;

        return char.IsHighSurrogate(text[0]) && text.Length > 1 && char.IsLowSurrogate(text[1])
            ? char.ConvertToUtf32(text[0], text[1])
            : text[0];
    }

    private static string? Recoded(SlideMarker marker, FontReference? reference)
    {
        if (marker is not { IsSymbol: true, Text.Length: 1 }) return null;

        // The table, and the face's own file being absent. Shared with the run path rather than
        // restated: `a:rPr/a:sym` reaches the same decision from the other end, and two copies of
        // a rule whose second clause has already been got wrong once is one copy too many.
        if (!SlideSymbolRuns.Recodes(marker.Typeface, reference)) return null;

        return SymbolFontRecode.TryRecode(marker.Typeface, marker.Text[0], out char recoded)
            ? recoded.ToString()
            : null;
    }

    /// <summary>A paragraph's marker, resolved once for both the width and the placement.</summary>
    /// <param name="Marker">The marker as its reader stated it.</param>
    /// <param name="Text">
    /// What is drawn, which is <see cref="SlideMarker.Text"/> after <see cref="Recoded"/> has had
    /// the resolved face's say. It is carried beside the marker rather than replacing it because
    /// the shaped run and the string handed to <c>Build</c> must be the same text, and the marker
    /// is resolved twice — once for the width the first line clears and once for the placement.
    /// </param>
    /// <param name="Face">The face the marker resolved to, or null when nothing could be read.</param>
    /// <param name="Reference">That face's resolution record, for the embedded-font catalogue.</param>
    /// <param name="Size">The marker's size, after its own scale and the body's fit.</param>
    /// <param name="Shaped">The shaped run, shared by the placement and by the width.</param>
    private readonly record struct MarkedParagraph(
        SlideMarker Marker,
        string Text,
        OpenTypeFace? Face,
        FontReference? Reference,
        Length Size,
        ShapedText? Shaped);

    /// <summary>
    /// How far the marker reaches past the paragraph's left indent, which its first line clears.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A bullet claims its own width and text never starts inside it.</strong> EditEngine
    /// records the bullet area's right edge on the paragraph portion and then, for the first line
    /// only, takes <c>nStartX = max(textLeft + firstLineOffset, bulletX)</c>
    /// (<c>editeng/source/editeng/impedit3.cxx:846-851</c>, over the <c>BulletX</c> set at
    /// <c>:798-802</c>). A hanging indent wide enough for the marker therefore decides the
    /// position, and one too narrow — or absent, which is what a binary PowerPoint outline
    /// normally has, both offsets zero — is overridden by the marker's own advance.
    /// </para>
    /// <para>
    /// Measured on <c>WC_Update-Aug03.ppt</c>, whose body paragraphs state <c>textOfs</c> 216 and
    /// <c>bulletOfs</c> 0 on the master and nothing of their own: LibreOffice draws the bullet at
    /// 49.10 pt and the line's first word at 62.87, which is the bullet's right edge at 57.14 plus
    /// the leading space the author typed. Without this rule the text starts at the bullet's own
    /// pen and the two overlap — legible on the page, and one word short per line to anything
    /// reading the text back, because the bullet and the first word extract as one token.
    /// </para>
    /// <para>
    /// <strong>The width is the marker's at its UNSCALED size, although it is drawn at the fit's.
    /// </strong> <c>Outliner::ImplGetBulletSize</c> measures the bullet once and caches it on the
    /// paragraph (<c>editeng/source/outliner/outliner.cxx</c>:1386-1424,
    /// <c>pPara->SetBulletSize(aSize, aScalingParameters)</c>), and that cache is filled before the
    /// autofit search runs — so the <em>whole</em> cached size, width as well as height, is the
    /// unfitted one. <see cref="BulletBoxHeight"/> already takes the height that way, for the same
    /// reason and with its own measurement; this is the width half of one rule.
    /// </para>
    /// <para>
    /// Measured on <c>Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx</c>
    /// page 13, whose master states <c>lvl2 marL="742950" indent="-285750"</c> — a 58.5 pt margin
    /// with the bullet 22.5 pt to its left — and whose runs state <strong>56 pt</strong> under a
    /// <c>normAutofit</c> that draws them at 14. Its en-dash bullet is 0.5562 em, so 31.13 pt at 56
    /// and 7.79 at 14: 26.2.4.2 starts every second-level first line 31.13 pt past the bullet, and
    /// this tree started it at the 22.5 pt margin. Pinned over twelve one-attribute variants of that
    /// one master (<c>probes/slidebullet-r191</c>): four hanging indents, two margins, three bullet
    /// characters (<c>–</c> 31.13, <c>W</c> 52.87, <c>i</c> clamped to the margin) and three stated
    /// run sizes (28 pt clamped, 56 → 31.13, 112 → 62.25, against 15.57, 31.14 and 62.29 predicted)
    /// — every one exact, and the bullet's own stated <c>sz</c> and the file's <c>fontScale</c> both
    /// decide nothing, the latter because LibreOffice recomputes the autofit rather than honouring
    /// it.
    /// </para>
    /// <para>
    /// It is a cascade rather than a shift: on that page our line fits 94 glyphs where the
    /// reference's fits 92, so a two-line paragraph becomes three and everything below drops one
    /// pitch — <c>diff%</c> 15.93 at <c>|ink|%</c> 0.14, which is the signature of a page where
    /// nothing is drawn differently and a great deal is drawn in the wrong place.
    /// </para>
    /// </remarks>
    private static Length MarkerReach(
        SlideParagraph paragraph, Scaling scaling, SlideFonts fonts)
    {
        Length width;

        if (MarkerPicture(paragraph) is { } picture)
        {
            width = picture.Width;
        }
        else if (Shaped(paragraph, scaling, fonts) is { Shaped: { } shaped } marked)
        {
            width = shaped.Width(Unscaled(paragraph, marked));
        }
        else
        {
            return Length.Zero;
        }

        // Never negative: the marker's right edge only ever pushes the first line further right,
        // and a hanging indent wider than the marker leaves the line where the file put it.
        Length reach = paragraph.FirstLineIndent + width;
        return reach > Length.Zero ? reach : Length.Zero;
    }

    /// <summary>
    /// The size the marker's cached width was measured at, which is its size before the fit.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A generated number takes the size it is drawn at, and one corpus deck says so.</strong>
    /// <c>30-04-2021 merged NDoH and NICD_Presentation HBV BD meeting 05May2021_1.pptx</c> page 9
    /// numbers its second level, and 26.2.4.2 starts those five first lines at the margin — 62.90 pt,
    /// which is what this tree drew before the rule and 2.84 pt left of what it drew after it. The
    /// reading that fits is <c>Paragraph::IsBulletInvalid</c>, which compares the bullet's own text
    /// as well as the scaling parameters: a generated number differs per paragraph, so the cache is
    /// refilled while the search already has a scale, where a fixed character never changes and is
    /// measured once, before it.
    /// </para>
    /// <para>
    /// Falls back to the size the marker is drawn at when the paragraph has no run to take a size
    /// from, which <see cref="Shaped"/> has already made impossible — it returns null for an empty
    /// run list — so the fallback is there to keep this total rather than to be reached.
    /// </para>
    /// </remarks>
    /// <param name="paragraph">The paragraph the marker labels.</param>
    /// <param name="marked">Its shaped marker.</param>
    private static Length Unscaled(SlideParagraph paragraph, MarkedParagraph marked)
        => marked.Marker.IsSymbol && paragraph.Runs.Count > 0
            ? ScaledMarker(Scaling.None, paragraph.Runs[0].Size, marked.Marker.Scale)
            : marked.Size;

    /// <summary>
    /// The picture a paragraph's marker is, when it has one and the paragraph draws it.
    /// </summary>
    /// <remarks>
    /// The same two guards a character marker takes in <see cref="Shaped"/>: a paragraph with no
    /// text draws no marker at all, and one with no runs has nothing to draw beside. The box a
    /// paragraph <em>without</em> text still keeps is <see cref="BulletFloored"/>'s and asks the
    /// marker directly.
    /// </remarks>
    private static SlideMarkerPicture? MarkerPicture(SlideParagraph paragraph)
        => paragraph.Marker is { Picture: { } picture }
           && paragraph.Text.Length > 0
           && paragraph.Runs.Count > 0
            ? picture
            : null;

    /// <summary>
    /// The stretches of a paragraph that are fields, and so break between characters.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A field is one portion to EditEngine, and an over-long one is not moved down and not
    /// offered to the break iterator: it is filled to the cell
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:1101-1200). So <em>“the reference breaks a long
    /// URL at any character where we break only at <c>-</c> and <c>/</c>”</em> is not a URL rule
    /// and not a hyphenation rule.
    /// </para>
    /// <para>
    /// Established by variant rather than by reading: on
    /// <c>0335fab9-79f0-4944-b92c-f223837ca2d8.odp</c> page 6, stripping the <c>text:a</c> elements
    /// and keeping their text makes 26.2.4.2 wrap at the same places this tree does and draw every
    /// pitch at 1.2 em. See <c>probes/odp-visual-r80/</c>.
    /// </para>
    /// <para>
    /// <strong>One stretch per run, and adjacent runs are not merged</strong> — which is what the
    /// DrawingML importer produces, one <c>com.sun.star.text.TextField.URL</c> per <c>a:r</c>
    /// (<c>oox/source/drawingml/textrun.cxx</c>:149-157). Measured: a link split across two runs
    /// and the same link in one are drawn span for span identically by 26.2.4.2, because a break
    /// at the junction is never the one the fill wants. <c>probes/pptx-field-r82/</c>'s <c>v8</c>.
    /// </para>
    /// <para>
    /// A one-character run is skipped because nothing inside it can break; the only thing it
    /// forgoes is the stretch's own start, which is an opportunity for exactly one line in a
    /// hundred thousand and which no corpus document reaches.
    /// </para>
    /// </remarks>
    /// <returns>
    /// Null when the paragraph holds no field, which is the overwhelming majority and is the whole
    /// cost of this on everything else — the shrink-to-fit search measures a body once per
    /// candidate scale, so a per-paragraph allocation here would be paid a dozen times a shape.
    /// </returns>
    private static List<CellBrokenSpan>? Fields(SlideParagraph paragraph)
    {
        List<CellBrokenSpan>? fields = null;

        foreach (SlideTextRun run in paragraph.Runs)
        {
            if (!run.IsField || run.Length <= 1) continue;

            fields ??= [];
            fields.Add(new CellBrokenSpan(run.Start, run.Length));
        }

        return fields;
    }

    /// <summary>Whether a line starting at an index is a field's spill rather than its own line.</summary>
    /// <remarks>
    /// Strictly inside: a line that begins exactly where a field begins is the line the field
    /// starts on, which is an ordinary one.
    /// </remarks>
    private static bool ContinuesField(IReadOnlyList<CellBrokenSpan> fields, int start)
    {
        foreach (CellBrokenSpan field in fields)
        {
            if (field.BreaksInside(start)) return true;
        }

        return false;
    }

    /// <summary>
    /// Breaks one paragraph into lines and gives each the height EditEngine would.
    /// </summary>
    /// <remarks>
    /// The break positions and the horizontal placement come from the shared layouter; only the
    /// vertical is recomputed. Doing it the other way — teaching the layouter this rule — would
    /// put a presentation-specific metric into the engine three families share.
    /// </remarks>
    private static Block? Measure(
        SlideParagraph paragraph,
        SlideTextBody body,
        Length width,
        SlideFonts fonts,
        Scaling scaling,
        bool fontIndependentLineSpacing,
        int paragraphIndex,
        Length? alignAgainst = null)
    {
        List<FormattedRun> runs = [];
        List<RunStyle> styles = [];
        OpenTypeFace? first = null;

        foreach (SlideTextRun run in paragraph.Runs)
        {
            (OpenTypeFace? face, FontReference? reference) =
                fonts.Resolve(run.Typeface, run.Weight, run.IsItalic);
            if (face is null) continue;

            first ??= face;
            Length size = scaling.Scaled(run.Size);

            // A superscript is measured at the size it is drawn at, which is the whole reason the
            // shrink has to reach the layouter rather than the painter: 58% of the em is 42% less
            // advance, and a line that fits at that width wraps at the full one.
            Length escaped = run.Escapement.SizeOf(size);

            runs.Add(new FormattedRun(run.Start, run.Length, face, escaped, Tracking: run.Tracking));
            styles.Add(new RunStyle(
                run.Colour, reference, face, run.IsUnderlined, run.IsStruckThrough,
                run.Escapement.RiseOf(size), size, run.IsShadowed, run.Escapement));
        }

        if (first is null) return null;

        // A hanging indent under a marker is the room the marker occupies, not a first-line
        // indent: LibreOffice draws the bullet at marL + indent and the paragraph's own first
        // line at marL. Measured on deck-features.pptx, whose outline states
        // marL="216000" indent="-216000" — 17.01 pt — and whose reference PDF puts the bullet at
        // 56.69 pt and the text at 73.70. Applying the indent to the text as well puts every
        // bulleted line a whole hanging indent to the left of where it belongs.
        ParagraphFormat format = new()
        {
            Alignment = paragraph.Alignment,
            StartIndent = paragraph.StartIndent,
            FirstLineIndent = paragraph.Marker is null
                ? paragraph.FirstLineIndent
                : MarkerReach(paragraph, scaling, fonts),
            LineSpacing = paragraph.LineSpacing,
            DefaultTabInterval = paragraph.DefaultTabInterval,
        };

        // Itemised by face, so a character the run's own face cannot draw is measured and drawn
        // from a face that can. It has to be stated here rather than left to the default, because
        // the default is deliberately no fallback at all — see `ItemisationOptions`. Everything
        // else about the cut is unchanged: a paragraph whose face covers its own text comes back
        // as the same single sub-run per formatting run it did before, in the same shaping call.
        MeasuredParagraph measured = MeasuredParagraph.Measure(
            paragraph.Text, runs, itemisation: new ItemisationOptions { GlyphFallback = fonts.Fallback });
        ParagraphLayouter layouter = new(first);
        List<CellBrokenSpan>? fields = Fields(paragraph);
        LaidOutParagraph laid = layouter.Layout(
            measured, format, width, paragraph.Language, cellBroken: fields);

        List<PlacedLine> lines = [];
        bool firstLine = true;
        for (int index = 0; index < laid.Lines.Count; index++)
        {
            LineBox unaligned = laid.Lines[index];
            LineBox box = Realigned(unaligned, format, alignAgainst, firstLine);
            firstLine = false;

            // The line EditEngine appends for an empty paragraph, or after a paragraph's trailing
            // hard line break, is measured by a different function and takes none of the fit's
            // line-spacing scale. See Appended.
            bool appended = index == laid.Lines.Count - 1 && box.Line.Start >= box.Line.End;

            if (!fontIndependentLineSpacing)
            {
                // The face's own metrics — but its ascent and descent only, with no external
                // leading. EditEngine adds the leading only when IsAddExtLeading() is on, which is
                // a Writer compatibility flag and off in Impress
                // (editeng/source/editeng/impedit3.cxx:3131-3136). Liberation Sans declares a line
                // gap of 67/2048, so keeping it makes an 18 pt line 20.70 pt where LibreOffice
                // draws 20.15 — half a point per line, measured on the wrapping cell of
                // slide-table-grid.pptx, whose four reference baselines are 20.154 pt apart.
                // To End rather than VisibleEnd: a trailing blank is a portion of the line and
                // EditEngine measures every portion of it. See LargestSize, which carries the
                // citation and the witness — and MeasuredEnd, for the one portion it skips.
                (Length ascent, Length metric) = FaceHeight(
                    runs, styles, box.Line.Start, MeasuredEnd(measured.Text, box.Line), body.Device);

                Length faceHeight = metric > Length.Zero ? metric : box.Height;
                // Through LineSpacingRule.Apply, whose whole-twip arithmetic this branch wants:
                // it is the ODF path, whose line height is the face's own metrics rather than a
                // fraction of the em, and slides-features.odp's sixth outline baseline moves
                // 0.155 pt off LibreOffice's without it. The font-independent branch below is the
                // one that needs finer units — see Spacing.
                Length faceLine = paragraph.LineSpacing.Apply(faceHeight);

                // …but a rule that changes nothing must not change the *unit* either. `Apply`
                // works in whole twips and this branch's height is a whole hundredth of a
                // millimetre, which is 0.567 of a twip — so single spacing, which is what nearly
                // every paragraph asks for, would round the device's own answer off its own grid.
                // Measured against LibreOffice on the 195-pair table: with the round trip 113 of
                // 195 line heights are exact and the other 82 are out by one unit in both
                // directions; without it, 195 of 195.
                if (faceLine.Twips == faceHeight.Twips) faceLine = faceHeight;

                Length faceAscent = ascent > Length.Zero ? ascent : box.Baseline;

                lines.Add(appended
                    ? BulletFloored(
                        Appended(box, faceAscent, faceHeight, paragraph, paragraphIndex),
                        paragraph, fonts, body.Device)
                    : Spaced(
                        new PlacedLine(box, faceAscent, faceLine, faceHeight),
                        scaling));
                continue;
            }

            // An autofitted body that is not being scaled measures its lines at the *device's*
            // realisation of the em rather than at the em. See DeviceRealised.
            bool realised = body.AutoFit && scaling.Font is <= 0 or 1.0;

            // One em of ascent and 1.2 em of box -- except where an escaped run makes the line
            // taller than that, which is what `FixedCellBox` is for. The height is a whole
            // hundredth of a millimetre, which is the unit EditEngine holds a line height in:
            // SetHeight takes a sal_uInt16 of the outliner's own map unit, and for a draw object
            // that unit is 1/100 mm. Keeping the exact EMU instead leaves a line height the
            // reference cannot represent, and the error accumulates down the block.
            Length em;
            Length natural;

            if (appended && paragraph.Text.Length > 0)
            {
                em = Realised(EndSize(runs, styles), realised);
                natural = FixedCellHeight(em);
            }
            else
            {
                (em, natural) = FixedCellBox(
                    runs, styles, box.Line.Start, MeasuredEnd(measured.Text, box.Line), realised);
            }

            if (appended)
            {
                lines.Add(BulletFloored(
                    Appended(box, em, natural, paragraph, paragraphIndex),
                    paragraph, fonts, body.Device));
                continue;
            }

            // EditEngine takes one of two branches here, and they are not the same arithmetic.
            // A paragraph that states a proportional line spacing takes
            // SvxInterLineSpaceRule::Prop, which multiplies the stated proportion and the fit
            // search's spacing scale together and rounds the *product* once; a paragraph that
            // states none takes the ::Off branch, which has only the fit's scale to apply and
            // no four-fifths on the ascent (impedit3.cxx:1553-1602). Applying the two
            // factors in sequence rounds twice and lands a hundredth of a millimetre out — see
            // Spacing and Proportioned.
            //
            // [24.2.7-audit: VERIFIED 2026-08-21, round slides-r53 — probed against the
            // installed 26.2.4.2, not read from the C++; the claim still holds.]
            // Re-checked against 26.2.4.2 on 2026-08-21 (TODO.24-2-7-audit.md) together with
            // Proportioned and ProportionedAscent, on make-linespace-probe.py: 44 authored boxes,
            // four em sizes x (no a:lnSpc, ten a:lnSpc percentages from 40 to 200). The reference's
            // baseline pitch is reproduced EXACTLY on all 44 and its first baseline to a uniform
            // 0.028 pt. The two-branch split survives: 40% draws a pitch of 19.191 pt where the
            // Off branch would draw 47.991.
            if (Proportion(paragraph.LineSpacing) is { } proportion)
            {
                Length height = Proportioned(natural, proportion * scaling.Spacing);

                lines.Add(new PlacedLine(
                    box,
                    ProportionedAscent(em, natural, height, proportion * scaling.Spacing),
                    height,
                    natural));
                continue;
            }

            // A stated line HEIGHT takes neither of those branches. EditEngine tests the four
            // rules in order -- SvxLineSpaceRule::Min, then ::Fix, then InterLineSpaceRule::Prop,
            // then ::Off -- so a fixed or minimum height is exclusive of the fit's ::Off scaling
            // and of the four-fifths, and it moves the ascent by the whole of the height's change.
            if (Stated(paragraph.LineSpacing, natural, scaling) is { } stated)
            {
                lines.Add(new PlacedLine(box, em + (stated - natural), stated, natural));
                continue;
            }

            // Nothing here states a proportion, so the ascent is one em: the ::Off branch is the
            // only one left to touch it, and Spaced is what transcribes that.
            //
            // Unless the paragraph reaches this arm by STATING exactly one hundred per cent, which
            // is a different answer from stating nothing and which only a .ppt can do. The Prop
            // arm above is entered on `GetInterLineSpaceRule() == Prop` and does nothing at all at
            // 100, so the ::Off arm below it -- the only place the fit's fSpacingY is applied --
            // is unreachable for such a paragraph. See SlideParagraph.LineSpacingStated for the
            // two import routes that make this binary-only and for the deck that measures it.
            PlacedLine plain =
                new PlacedLine(box, em, Spacing(paragraph.LineSpacing, natural), natural);

            lines.Add(paragraph.LineSpacingStated ? plain : Spaced(plain, scaling));
        }

        // A line that starts inside a field is one the field spilled onto, and is stacked by a
        // different rule. Marked after the fact rather than inside the four arms above, so that the
        // heights they compute — which are the *enclosing* line's and are what the spill is measured
        // from — stay exactly as they were.
        if (fields is not null)
        {
            for (int index = 0; index < lines.Count; index++)
            {
                if (ContinuesField(fields, lines[index].Box.Line.Start))
                {
                    lines[index] = lines[index] with { ContinuesField = true };
                }
            }
        }

        // Deliberately not the lines a field spilled onto. EditEngine's formatter never sees them
        // — the whole field is one portion of one `EditLine`, and the sub-lines exist only in the
        // painter — so the height that anchors the block and that the shrink-to-fit search measures
        // counts the field's line once. Measured on `field-variants.py`'s `v4`/`v6` against
        // 26.2.4.2: a middle-anchored box whose second paragraph is a URL spilling onto three
        // visual lines is centred as though it held two, and the spill hangs below the box.
        Length total = Length.Zero;
        foreach (PlacedLine line in lines)
        {
            if (!line.ContinuesField) total += line.Height;
        }

        return new Block(
            paragraph, measured, styles, lines,
            total + ScaledSpace(paragraph.SpaceBefore, scaling)
                  + ScaledSpace(paragraph.SpaceAfter, scaling),
            scaling, format, fonts.Fallback);
    }

    /// <summary>
    /// A paragraph's own space above or below, after the fit's line-spacing scale.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The fit's spacing scale reaches a paragraph's space, not only its lines.</strong>
    /// EditEngine puts every <c>SvxULSpaceItem</c> through <c>scaleYSpacingValue</c>
    /// (<c>editeng/source/editeng/impedit2.cxx</c>, <c>ImpEditEngine::CalcHeight</c>), and that
    /// helper is a no-op only when <c>maStatus.DoStretch()</c> is clear or the scale is one
    /// (<c>impedit.hxx</c>:797-801). <c>DoStretch()</c> is <em>set</em> on exactly this path:
    /// <c>SdrTextObj::ImpSetupDrawOutlinerForPaint</c> turns
    /// <c>EEControlBits::STRETCHING</c> on whenever <c>IsFitToSize() || IsAutoFit()</c>
    /// (<c>svx/source/svdraw/svdotext.cxx</c>:1177-1183) and only then calls
    /// <c>setupAutoFitText</c>. So an autofitted shape always satisfies the guard, and the
    /// scale applies.
    /// </para>
    /// <para>
    /// Measured rather than argued, on <c>research/probes/slides-r20/make-spacing-probe.py</c>:
    /// twelve autofit boxes of one text at twelve box heights, each holding four paragraphs of
    /// two lines with the second forced by a hard break, so the line count cannot move with the
    /// font size. The reference disagreed with us on three of the twelve — <em>every one of them
    /// a box where we kept a larger font at nine- or eight-tenths spacing and it took a smaller
    /// font at full spacing</em> — and scaling the paragraph space is what turns all three round.
    /// The other nine land on full spacing, where this is a no-op by construction.
    /// </para>
    /// <para>
    /// The predecessor of this note recorded the opposite, citing the same guard: that
    /// <c>scaleYSpacingValue</c> "returns its argument unchanged unless <c>DoStretch()</c> is
    /// set". The citation is right and the inference from it is backwards — under autofit the
    /// flag is set.
    /// </para>
    /// </remarks>
    /// <remarks>
    /// <para>
    /// <strong>The scale truncates, it does not round, and the space is a whole hundredth of a
    /// millimetre before it is scaled at all.</strong> <c>scaleYSpacingValue</c> answers a
    /// <c>double</c> and every caller in <c>CalcHeight</c> assigns it to an integer —
    /// <c>sal_uInt16 nUpper = scaleYSpacingValue(rULItem.GetUpper())</c> and
    /// <c>rPortion.mnHeight += scaleYSpacingValue(rULItem.GetLower())</c>
    /// (<c>editeng/source/editeng/impedit2.cxx</c>:4792-4802) — so the fraction is dropped. The
    /// unscaled value is an integer too, because a <c>SvxULSpaceItem</c> holds it in the model's
    /// own map unit.
    /// </para>
    /// <para>
    /// Measured on page 2 of <c>2015-Civil-Rights-Website-training.ppt</c>, whose outline states
    /// 8 pt of space before every paragraph — 282 units — and whose fit answers a spacing scale of
    /// nine tenths. The reference's bullet-to-bullet distance puts the scaled space at
    /// <strong>253</strong> units, where <c>fround(253.8)</c> would be 254; over that slide's
    /// seven gaps the difference is 0.198 pt in where the last baseline lands, and the reference's
    /// is reproduced to 0.010 pt only with the truncation.
    /// </para>
    /// </remarks>
    private static Length ScaledSpace(Length space, Scaling scaling)
    {
        if (space.Emu == 0) return space;

        double scale = scaling.Spacing is <= 0 or > 1.0 ? 1.0 : scaling.Spacing;

        return Length.FromMm100((long)(space.Mm100 * scale));
    }

    /// <summary>
    /// The tallest ascent and the tallest ascent-plus-descent among the runs a line touches.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Per run rather than per paragraph, for the same reason <see cref="LargestSize"/> is: a
    /// bigger word on a line makes that line taller and leaves the others alone. Both quantities
    /// come from the same face resolution the shared layouter uses, so the only difference from
    /// its answer is the line gap.
    /// </para>
    /// <para>
    /// A run that sits off its baseline is measured at its shrunk size and then given its rise
    /// back, which is <c>RecalcFormatterFontMetrics</c>'s closing rule —
    /// <c>ascent × propr / 100 + em × esc / 100</c> upwards and the mirror of it downwards
    /// (<c>editeng/source/editeng/impedit3.cxx:3164-3181</c>). At 58% of Liberation Sans's
    /// 0.905 em ascent plus DrawingML's usual 30% rise that comes to 0.83 em against 0.91 plain,
    /// so an ordinal never makes its own line taller; a file stating a rise past 42% does, and
    /// this is the arithmetic that lets it.
    /// </para>
    /// </remarks>
    private static (Length Ascent, Length Height) FaceHeight(
        List<FormattedRun> runs, List<RunStyle> styles, int start, int end, MetricGrid device)
    {
        Length ascent = Length.Zero;
        Length height = Length.Zero;

        for (int i = 0; i < runs.Count; i++)
        {
            FormattedRun run = runs[i];
            bool touches = run.Start < end && start < run.End;
            bool contains = start == end && run.Covers(start);
            if (!touches && !contains) continue;

            LineMetrics metrics = LineSpacing.Resolve(run.Face, device);
            Length up = Rounded(metrics.ScaledAscent(run.EmSize));
            Length down = Rounded(metrics.ScaledDescent(run.EmSize));

            Length rise = i < styles.Count ? styles[i].Rise : Length.Zero;
            if (rise > Length.Zero) up += rise;
            else if (rise < Length.Zero) down -= rise;

            ascent = Length.Max(ascent, up);
            height = Length.Max(height, up + down);
        }

        return (ascent, height);
    }

    /// <summary>
    /// A metric rounded to a whole hundredth of a millimetre, which is the unit VCL keeps it in.
    /// </summary>
    /// <remarks>
    /// <c>FontMetricData::ImplCalcLineSpacing</c> ends <c>mnAscent = round(fAscent)</c> in the
    /// device's own logical unit (<c>vcl/source/font/fontmetric.cxx:538-540</c>), and Impress's
    /// reference device is in 1/100 mm — so an 18 pt Liberation Sans line is 575 + 135 units and
    /// not 574.79 + 134.55. Worth a tenth of a point over four lines, which is the difference
    /// between agreeing with the reference and not.
    /// <para>
    /// <b>Idempotent since <see cref="MetricGrid.Presentation"/> landed, and kept for that
    /// reason rather than removed.</b> The grid already returns whole hundredths of a millimetre,
    /// so this rounds nothing; what it still does is state the unit at the point the value is
    /// consumed, and catch a caller that reaches this arithmetic without a grid. The rounding it
    /// used to do on its own was the right unit and the wrong order — it rounded an exactly
    /// scaled metric, where the device rounds the em to whole pixels first and then the metric,
    /// which is worth a unit on 425 of 507 measured (face, size) pairs.
    /// </para>
    /// </remarks>
    private static Length Rounded(Length metric)
        => Length.FromMm100((long)Math.Round((double)metric.Emu / Length.EmuPerMm100));

    /// <summary>The fraction of a tightened line EditEngine puts above the baseline.</summary>
    private const double ShortSpacingAscent = 0.8;

    /// <summary>The largest em size among the runs a line touches.</summary>
    /// <remarks>
    /// <para>
    /// The line's own runs rather than the paragraph's, because a 32 pt word in an 18 pt paragraph
    /// makes <em>its</em> line taller and leaves the others alone — which is the same rule the
    /// shared layouter applies to font metrics, restated for a metric that is not the font's.
    /// </para>
    /// <para>
    /// A superscript counts at the size it would have taken, not the size it was shrunk to:
    /// <c>RecalcFormatterFontMetrics</c> forces the proportion back to 100% before it reads a
    /// metric, so the ordinal in "5th" leaves its line exactly as tall as the date beside it
    /// (<c>editeng/source/editeng/impedit3.cxx:3121-3126</c>).
    /// </para>
    /// <para>
    /// <strong>The runs a line touches run to its <see cref="TextLine.End"/>, not to its
    /// <see cref="TextLine.VisibleEnd"/> — a trailing blank is on the line and is measured.</strong>
    /// EditEngine walks <em>every</em> portion of the line, <c>GetStartPortion()</c> to
    /// <c>GetEndPortion()</c> inclusive, skipping only a <c>PortionKind::LINEBREAK</c>, and takes the
    /// largest ascent and the largest descent it finds (<c>editeng/source/editeng/impedit3.cxx</c>:
    /// 1496-1519); a portion that happens to be blank is not exempt. Trailing blanks are excluded
    /// from a line's <em>width</em> — that is what <c>VisibleEnd</c> is for, and it is still what
    /// the line is drawn and aligned by — but nothing excludes them from its <em>height</em>.
    /// </para>
    /// <para>
    /// It is only ever visible when the trailing blank is bigger than the text before it, which a
    /// real deck does: <c>pres_ioc_phuket.ppt</c> page 26's white box ends its wrapped paragraph
    /// with a single space at <strong>28 pt</strong> after 19 pt italic text, and 26.2.4.2 draws
    /// that space — a one-glyph 28.01 pt show at the end of the last line — and sizes the line by
    /// it. The box states <c>draw:auto-grow-height="true"</c> with <c>fo:min-height="0cm"</c>, so
    /// the height it is drawn at is the height of the block: 26.2.4.2 gives it
    /// <strong>3.267 cm</strong> and, measuring that last line at 19 pt instead, we gave it
    /// <strong>2.884 cm</strong>. The difference is
    /// <c>fround(988 × 1.2) − fround(670 × 1.2) = 1186 − 804 = 382</c> hundredths of a millimetre
    /// against the 383 measured off the two content streams, and the reference's own line pitch
    /// confirms which line moved: from the second baseline to the third it is <strong>31.81 pt</strong>
    /// where ours is 22.79, and 31.81 pt is 1122 units, which is
    /// <c>(804 − 670) + 988</c> — the 19 pt line's descent plus the <em>28 pt</em> line's ascent.
    /// </para>
    /// <para>
    /// The whole box closes on it. Its three lines are 24 pt, 19 pt and 28 pt, so the block is
    /// <c>1016 + 804 + 1186 = 3006</c> plus 0.13 cm of padding at each end — <strong>3266</strong>
    /// against the 3267 the reference's own flat-ODP export states for the shape — where measuring
    /// the last line at 19 pt gives <c>1016 + 804 + 804 + 260 = 2884</c>, which is the 81.77 pt we
    /// drew.
    /// </para>
    /// </remarks>
    private static Length LargestSize(
        List<FormattedRun> runs, List<RunStyle> styles, int start, int end)
    {
        Length largest = Length.Zero;

        for (int i = 0; i < runs.Count; i++)
        {
            FormattedRun run = runs[i];
            bool touches = run.Start < end && start < run.End;
            bool contains = start == end && run.Covers(start);
            if (touches || contains) largest = Length.Max(largest, Nominal(runs, styles, i));
        }

        if (largest > Length.Zero) return largest;

        // An empty paragraph still occupies a line, and it is as tall as the text that would go
        // on it: the first run's size, which is what the paragraph mark carries.
        return runs.Count > 0 ? Nominal(runs, styles, 0) : Length.FromPoints(18);
    }

    /// <summary>
    /// A line's ascent and its height under font-independent line spacing: one em of ascent and
    /// 1.2 em of box for ordinary text, and more than that where an escaped run reaches further.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ImpEditEngine::CreateLines</c> finishes a line by walking every portion of it through
    /// <c>RecalcFormatterFontMetrics</c> and taking the largest ascent and the largest descent it
    /// meets — separately (<c>editeng/source/editeng/impedit3.cxx</c>:1496-1519). Under
    /// <c>IsFixedCellHeight()</c> that function answers <c>ascent = fontHeight</c> and
    /// <c>descent = fround(1.2 × fontHeight) − fontHeight</c> (<c>:3138-3142</c>), so for text
    /// carrying no escapement the sum is <c>fround(1.2 × em)</c> exactly and this is the rule
    /// <see cref="LargestSize"/> plus <see cref="FixedCellHeight"/> already gave.
    /// </para>
    /// <para>
    /// <strong>A raised or lowered run is measured twice and the larger answer wins each side</strong>
    /// (<c>:3164-3183</c>). The function forces the proportion back to 100 before it takes the
    /// metric, so an escaped portion contributes the <em>full</em> ascent like any other; then,
    /// for a subscript, it re-derives the descent as
    /// <c>descent × proportion / 100 − fontHeight × escapement / 100</c> — integer arithmetic in
    /// hundredths of a millimetre, and the second term is a <em>subtraction of a negative</em>, so
    /// a subscript's descent grows by the whole of its drop. A superscript does the same to the
    /// ascent. Neither ever shrinks the line, because both are compared against the unescaped
    /// value already accumulated.
    /// </para>
    /// <para>
    /// <strong>What it costs is a line a quarter of an em taller wherever a subscript appears,
    /// and on an autofitted body that is a whole <c>constScaleLevels</c> row.</strong> At 28 pt —
    /// 988 hundredths of a millimetre — the plain descent is <c>fround(988 × 1.2) − 988 = 198</c>
    /// and a <c>-25% 58%</c> subscript's is <c>198 × 58 / 100 + 988 × 25 / 100 = 114 + 247 = 361</c>,
    /// so the line is 1349 rather than 1186.
    /// </para>
    /// <para>
    /// <strong>Measured at 26.2.4.2 by variant rather than derived, and it is the drop that
    /// does it rather than the shrink.</strong> <c>pods05.ppt</c> page 32 holds an autofitted box
    /// whose runs carry <c>style:text-position="-25% 58%"</c> subscripts; the reference draws that
    /// page's dominant text at 9 pt where this tree drew 11. Take 26.2.4.2's own flat ODP of the
    /// deck and render it straight back through the same binary and page 32 still comes out at
    /// 9 pt, so the round trip is faithful there. Change <em>only</em> the offset, to
    /// <c>0% 58%</c> — the same shrunken size, no drop — and the same binary draws it at
    /// <strong>11 pt, this tree's answer</strong>. Change only the proportion instead, to
    /// <c>-25% 100%</c>, and it stays at 9: the drop alone moves the row, and the shrink on its
    /// own does not move it far enough to cross one. <c>probes/slides-r107/results.md</c> §2.
    /// </para>
    /// </remarks>
    private static (Length Ascent, Length Height) FixedCellBox(
        List<FormattedRun> runs, List<RunStyle> styles, int start, int end, bool realised)
    {
        long ascent = 0;
        long descent = 0;
        bool measured = false;

        // A line consisting of nothing but its own hard break measures zero, and EditEngine then
        // falls back to `ImplCalculateFontIndependentLineSpacing` of the break character's own
        // font with no escapement anywhere in it (`impedit3.cxx`:1480-1491). That is the
        // `start == end` case, so it deliberately passes `escaped` false below.
        bool wholeLine = start != end;

        for (int i = 0; i < runs.Count; i++)
        {
            FormattedRun run = runs[i];
            bool touches = run.Start < end && start < run.End;
            bool contains = start == end && run.Covers(start);
            if (!touches && !contains) continue;

            measured = true;
            Accumulate(i, wholeLine);
        }

        if (!measured)
        {
            // An empty paragraph still occupies a line, and it is as tall as the text that would
            // go on it: the first run's size, which is what the paragraph mark carries.
            if (runs.Count > 0)
            {
                Accumulate(0, false);
            }
            else
            {
                Length fallback = Realised(Length.FromPoints(18), realised);
                ascent = fallback.Mm100;
                descent = FixedCellHeight(fallback).Mm100 - ascent;
            }
        }

        return (Length.FromMm100(ascent), Length.FromMm100(ascent + descent));

        void Accumulate(int index, bool escaped)
        {
            long height = Realised(Nominal(runs, styles, index), realised).Mm100;
            long own = height;
            long below = FixedCellHeight(Length.FromMm100(height)).Mm100 - height;

            SlideEscapement escapement =
                escaped && index < styles.Count ? styles[index].Escapement : default;

            if (escapement.Percent != 0)
            {
                // `DFLT_ESC_PROP` is what an unstated proportion means, and zero is how this tree
                // spells "no escapement at all" -- neither reaches here with a non-zero percent.
                long proportion = escapement.Proportion is 0 or 100 ? 100 : escapement.Proportion;
                long rise = height * escapement.Percent / 100;

                if (escapement.Percent > 0) own = Math.Max(own, (own * proportion / 100) + rise);
                else below = Math.Max(below, (below * proportion / 100) - rise);
            }

            ascent = Math.Max(ascent, own);
            descent = Math.Max(descent, below);
        }
    }

    /// <summary>The box a font-independent line of this em size occupies: <c>fround(1.2 em)</c>.</summary>
    /// <remarks>
    /// <c>ImplCalculateFontIndependentLineSpacing</c>, <c>impedit3.cxx</c>:501-505, whose own
    /// constant is <c>12.0 / 10.0</c> and whose rounding is <c>basegfx::fround</c>.
    /// </remarks>
    private static Length FixedCellHeight(Length em)
        => Length.FromMm100((long)Math.Floor((em.Mm100 * LineHeightFactor) + 0.5));

    /// <summary>The em size, through the device's grid when the caller asks for it.</summary>
    private static Length Realised(Length em, bool realised) => realised ? DeviceRealised(em) : em;

    /// <summary>
    /// Where a line's <em>height</em> measurement stops: its <see cref="TextLine.End"/>, less the
    /// hard line break that ends it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A hard line break does not size the line it ends, and every other portion of that
    /// line does.</strong> EditEngine's metrics loop walks <c>GetStartPortion()</c> to
    /// <c>GetEndPortion()</c> inclusive and skips exactly one kind —
    /// <c>if ( rTP.GetKind() != PortionKind::LINEBREAK )</c>, with the comment beside it naming
    /// the case: <em>"problem with hard font height attribute, when everything but the line break
    /// has this attribute"</em> (<c>editeng/source/editeng/impedit3.cxx</c>:1498-1516). The kind is
    /// set by <c>EE_FEATURE_LINEBR</c> and by nothing else (<c>:1088-1099</c>), and the break is
    /// the line's last character — <c>pLine->SetEnd(nPortionStart + 1)</c> (<c>:1441-1449</c>) — so
    /// skipping the portion is dropping one character off the end of the range. Read in this
    /// checkout, which declares <c>27.2.0.0.alpha0+</c> and is not the reference binary's source;
    /// the paragraph after next is the same claim measured at 26.2.4.2 itself.
    /// </para>
    /// <para>
    /// <strong>A line that is nothing but the break keeps it, and that is the same source rather
    /// than an exception to it.</strong> <c>EditLine::CalcTextSize</c> adds nothing for a
    /// <c>LINEBREAK</c> portion (<c>editeng/source/editeng/EditLine.cxx</c>:69-71), so such a line
    /// measures zero and takes the fallback above the loop: <c>SeekCursor(pNode,
    /// pLine-&gt;GetStart()+1, aTmpFont)</c> and then, under <c>IsFixedCellHeight()</c>,
    /// <c>ImplCalculateFontIndependentLineSpacing(aTmpFont.GetFontHeight())</c>
    /// (<c>impedit3.cxx</c>:1478-1491) — the height of the break's <em>own</em> character.
    /// Trimming the last character here leaves <c>start == end</c>, which is precisely the case
    /// <see cref="LargestSize"/> and <see cref="FaceHeight"/> answer from the run that covers
    /// <c>start</c>, so no special case is needed for it.
    /// </para>
    /// <para>
    /// <strong>Measured at 26.2.4.2, one variable, five boxes per slide</strong>
    /// (<c>probes/slides-r100/make-break-probe.py</c>, <c>break-probe.txt</c>). Each box holds
    /// 18 pt text and one construct whose size is swept 18, 24, 28, 36, 54, 72 pt, and the boxes
    /// are read out of the reference's own PDF by their baselines:
    /// </para>
    /// <list type="bullet">
    /// <item><description><c>BREAK</c> — <c>"AAA"</c>, a break at the swept size, <c>"BBB"</c>.
    /// Both baselines are <strong>462.019 and 440.419 on all six slides</strong>: the break's size
    /// changes nothing.</description></item>
    /// <item><description><c>BLANK</c> — a trailing space at the swept size, and <c>RUN</c> — a
    /// visible glyph at it. Both move on every slide, identically to each other, 462.019 → 408.019
    /// over the sweep. So a blank sizes its line exactly as a glyph does, which is round 99's rule
    /// re-measured at the binary on a deck built for this.</description></item>
    /// <item><description><c>TWICE</c> — two breaks in a row, so the middle line holds nothing but
    /// one. Its height <em>does</em> follow the swept size, and follows it as
    /// <c>fround(1.2 × em)</c> exactly: the first-to-third baseline distance is
    /// <c>127 + fround(1.2 × em) + 635</c> hundredths of a millimetre on all six, 1524 at 18 pt
    /// through 3810 at 72 pt.</description></item>
    /// </list>
    /// <para>
    /// <strong>What it was worth.</strong> Reaching <see cref="TextLine.End"/> rather than
    /// <see cref="TextLine.VisibleEnd"/> is right, and it brought this in with it: a line
    /// separator is trailing whitespace, so the old range already excluded it by accident.
    /// <c>Inducement-to-Insurance-Business.ppt</c> page 14 is the corpus witness — a bottom-anchored
    /// title whose paragraph ends <c>…Section 8 of RESPA?</c> and then a line break set in
    /// <strong>54 pt</strong> after 28 pt text. 26.2.4.2 gives that line the same 30.246 pt pitch
    /// as every other line of the block; we gave it 52.696, pushing the block 28 pt further up and
    /// one more line off the top of the page.
    /// </para>
    /// </remarks>
    private static int MeasuredEnd(ReadOnlySpan<char> text, TextLine line)
    {
        int end = Math.Min(line.End, text.Length);

        return end > line.Start && IsLineSeparator(text[end - 1]) ? end - 1 : end;
    }

    /// <summary>
    /// True for a character that can only be a manual line break, never the end of a paragraph.
    /// </summary>
    /// <remarks>
    /// The set <c>TextMeasurer</c> fills lines by: OOXML's <c>a:br</c> and <c>w:br</c> and ODF's
    /// <c>text:line-break</c> all arrive as U+2028 and a binary PowerPoint's as U+000B. The three
    /// characters a paragraph may end with — <c>'\r'</c>, <c>'\n'</c> and U+2029 — are deliberately
    /// not in it: they are not a <c>LINEBREAK</c> portion and never reach a line's interior.
    /// </remarks>
    private static bool IsLineSeparator(char character)
        => character is '\u2028' or '\u000B' or '\u000C' or '\u0085';

    /// <summary>The size a run would take were it not escaped.</summary>
    private static Length Nominal(List<FormattedRun> runs, List<RunStyle> styles, int index)
    {
        Length nominal = index < styles.Count ? styles[index].NominalSize : Length.Zero;
        return nominal > Length.Zero ? nominal : runs[index].EmSize;
    }

    /// <summary>
    /// The height a paragraph's stated line spacing gives a line, in the draw layer's own unit.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A pass-through for single spacing rather than a call to
    /// <see cref="LineSpacingRule.Apply(Length)"/>, and the reason is the unit rather than the arithmetic.
    /// <c>Apply</c> computes in <strong>whole twips</strong>, because that is Writer's layout unit
    /// and the truncation there is observable; Impress lays out in hundredths of a millimetre, and
    /// a twip is 0.05 pt against a hundredth of a millimetre's 0.028, so round-tripping a line
    /// height through twips loses resolution the draw layer has.
    /// </para>
    /// <para>
    /// A proportion of zero counts as single, because that is what <c>Apply</c> itself does with
    /// it — a default-constructed rule and an explicit 100 per cent are the same rule — and a
    /// hand-built body states neither.
    /// </para>
    /// <para>
    /// Invisible until the shrink-to-fit search reads it. The search picks the candidate whose
    /// height comes closest to filling the box, and a 27 pt line at full spacing and a 30 pt line
    /// at nine-tenths differ by exactly one hundredth of a millimetre — 1144 against 1143. Through
    /// twips both are 648, the search sees a tie, keeps the earlier candidate, and draws 30 where
    /// LibreOffice draws 27. Three of the eighty-eight boxes in the fit probe deck turned on this
    /// one unit.
    /// </para>
    /// <para>
    /// <strong>The proportional case needs the same unit, and it used to be the one exception.</strong>
    /// The paragraph above argues that a twip is too coarse for the draw layer and then routed every
    /// proportion other than single through <c>Apply</c> anyway. EditEngine computes the
    /// proportional height in hundredths of a millimetre with one rounding —
    /// <c>nHeight = fround(pLine-&gt;GetHeight() * fProportionalScale * fSpacingFactor)</c> below
    /// a hundred per cent, and a truncating <c>sal_Int32</c> conversion of the same product above it
    /// (<c>editeng/source/editeng/impedit3.cxx:1553-1580</c>). <c>basegfx::fround</c> is
    /// <c>(Int)(x + 0.5)</c> for a positive value (<c>include/basegfx/numeric/ftools.hxx:39-50</c>),
    /// so the two branches round in opposite directions and both are reproduced here.
    /// [24.2.7-audit: VERIFIED 2026-08-21, round slides-r53 — probed against the
    /// installed 26.2.4.2, not read from the C++; the claim still holds.]
    /// <strong>Re-checked against 26.2.4.2 on 2026-08-21</strong> — see the note in
    /// <see cref="Proportioned"/>.
    /// </para>
    /// <para>
    /// Worth a line height rather than a rounding curiosity, because the fit search reads it. On the
    /// 40 pt probe box at 8 pt and nine-tenths spacing a 338-unit natural line gives the reference
    /// <c>fround(338 × 0.8 × 0.9) = 243</c>; through whole twips 338 becomes 191.62 twips, rounds to
    /// 192, loses a fifth to 154, and comes back 271.6 — <strong>244.5</strong> after the ninety per
    /// cent. Over six lines that is 1417 against the reference's 1408 in a box of 1413, so the
    /// reference fits 8 pt at nine-tenths by five hundredths of a millimetre and we did not, falling
    /// back to 7 pt at full spacing and drawing the text a point too small.
    /// </para>
    /// <para>
    /// Two divergences from <see cref="LineSpacingRule.Apply(Length)"/> come with the change, both
    /// deliberate. <c>Apply</c> raises a proportion below fifty per cent to fifty, which is Writer's
    /// <c>CalcRealHeight</c> rule and not EditEngine's; measured across the slides track,
    /// <strong>nothing states a line proportion under fifty per cent at all</strong> — the minimum
    /// is sixty — so the clamp is inert here and dropping it is unmeasurable rather than an
    /// improvement (<c>research/probes/slides-r16/lnspc-scan.py</c>). And <c>Apply</c> works in
    /// whole percentage points with truncating integer division, which is a second quantisation on
    /// top of the unit; the reference's own whole-percent quantisation happens at import instead
    /// (<c>oox/inc/drawingml/textspacing.hxx:52</c>), which is where we do it too.
    /// </para>
    /// </remarks>
    /// <remarks>
    /// <para>
    /// <strong>The paragraph above described this method and the code did the opposite.</strong>
    /// A rule that changes nothing — single spacing, or a proportion of exactly one hundred per
    /// cent — went to <see cref="LineSpacingRule.Apply(Length)"/>, whose <em>first</em> line is
    /// <c>naturalHeight.Twips</c>, so it round-trips through whole twips before its
    /// <c>_ =&gt; natural</c> arm hands the value back. That is a quantisation on every
    /// single-spaced slide line in the corpus, and it survives only line heights that are a whole
    /// multiple of 3.6 pt.
    /// </para>
    /// <para>
    /// Measured on <c>research/probes/slides-r21/make-pitch-probe.py</c>, whose plain boxes are
    /// <c>fround(em × 1.2)</c> on all 53 reference sizes: through the twip trip an 8 pt line is
    /// drawn at 338.67 units against the reference's 338, a 10 pt line at 423.33 against 424, and
    /// a 28 pt line at 1185.2 against 1186. Under a unit it is invisible in a page image and it is
    /// not invisible to the fit search, which decides between two candidates on one hundredth of a
    /// millimetre — see the paragraph above about 1144 against 1143.
    /// </para>
    /// </remarks>
    private static Length Spacing(LineSpacingRule rule, Length natural)
        => Proportion(rule) is { } proportion ? Proportioned(natural, proportion)
            : Neutral(rule) ? natural
            : rule.Apply(natural);

    /// <summary>
    /// The height a paragraph <em>states</em>, when it states one, after the fit's spacing scale.
    /// </summary>
    /// <remarks>
    /// <para>
    /// EditEngine's <c>SvxLineSpaceRule::Fix</c> and <c>::Min</c> branches
    /// (<c>editeng/source/editeng/impedit3.cxx:1530-1552</c>), which are the <em>first</em> two
    /// arms of a four-way chain whose other two are the proportional rules
    /// <see cref="Proportioned"/> and <see cref="Spaced"/> transcribe. A stated height therefore
    /// excludes them both: no four-fifths, and no second application of the fit's scale.
    /// </para>
    /// <code>
    /// nFixHeight = fround(scaleYSpacingValue(GetLineHeight()));
    /// nTxtHeight = pLine-&gt;GetHeight();
    /// pLine-&gt;SetMaxAscent(pLine-&gt;GetMaxAscent() + (nFixHeight - nTxtHeight));
    /// pLine-&gt;SetHeight(nFixHeight, nTxtHeight);
    /// </code>
    /// <para>
    /// <strong>The ascent moving with the height is the whole of this, and it was missing.</strong>
    /// What stood here sent a stated height through <see cref="LineSpacingRule.Apply(Length)"/> —
    /// Writer's whole-twip arithmetic — and left the ascent at one em, so a paragraph asking for
    /// an exact 24 pt line in 12 pt text had its first baseline 9.58 pt above where the reference
    /// draws it and every line after it with the same offset.
    /// </para>
    /// <para>
    /// Measured against <strong>26.2.4.2</strong> on <c>make-linespace-probe.py</c>, twelve
    /// <c>a:lnSpc/a:spcPts</c> boxes — 10, 24 and 50 pt of stated height in 11, 12, 24 and 40 pt
    /// text, four lines each, <c>a:noAutofit</c> so no fit scale is in play. Every one of the
    /// twelve now lands on the reference's pitch to a thousandth of a point and on its first
    /// baseline to the same <strong>0.028 pt</strong> — one hundredth of a millimetre — that
    /// every other case on that probe carries, stated-height or not, so nothing about the stated
    /// height contributes to it. That is the constant <c>SlideTextPlacementTests</c> already
    /// records as the shift LibreOffice's PDF export puts on every pen.
    /// </para>
    /// <para>
    /// The arithmetic can go negative — 40 pt text in a stated 10 pt line puts the ascent at
    /// 2.0 pt, and a smaller stated height would put it below zero. EditEngine holds the ascent
    /// in a <c>sal_uInt16</c> and wraps; that is deliberately not reproduced.
    /// </para>
    /// <para>
    /// The unit is a whole hundredth of a millimetre, not a twip: the reference draws a stated
    /// 24 pt line at 24.009 pt, which is 847 units, and <c>Apply</c>'s twip round trip gives
    /// 24.000. Same reason <see cref="Spacing"/> gives.
    /// </para>
    /// <para>
    /// Reach, censused over the corpus before the change: <strong>769 <c>a:lnSpc/a:spcPts</c>
    /// sites in 23 distinct <c>.pptx</c> documents</strong> — 48 of them in
    /// <c>NAS-Infrastructure-Roadmaps-v16.0.pptx</c>, the largest single <c>abs_ink</c> on the
    /// track — plus 85 negative-line-feed paragraphs in one <c>.ppt</c>. No <c>.odp</c> in the
    /// corpus states a fixed line height at all, and the ODF branch above does not come through
    /// here.
    /// </para>
    /// </remarks>
    private static Length? Stated(LineSpacingRule rule, Length natural, Scaling scaling)
    {
        if (rule.Mode is not (LineSpacingMode.Exact or LineSpacingMode.AtLeast)) return null;
        if (rule.Value <= Length.Zero) return null;

        // scaleYSpacingValue, which is the identity at one and is never above one here: the fit's
        // table holds no spacing above 1.0.
        double scale = scaling.Spacing is <= 0 or >= 1.0 ? 1.0 : scaling.Spacing;
        Length stated = Length.FromMm100((long)Math.Floor((rule.Value.Mm100 * scale) + 0.5));

        // ::Min grows a short line and leaves a tall one alone; ::Fix takes the stated height
        // whichever way it falls.
        return rule.Mode == LineSpacingMode.AtLeast && stated <= natural ? null : stated;
    }

    /// <summary>Whether a rule leaves a natural line height alone.</summary>
    /// <remarks>
    /// Exactly the rules <see cref="LineSpacingRule.Apply(Length)"/> would return the natural height for,
    /// so this changes the unit the answer is held in and never the answer itself. The three modes
    /// that state an absolute height keep going through <c>Apply</c>, because a stated height is a
    /// length the file gives rather than one the line derives.
    /// </remarks>
    private static bool Neutral(LineSpacingRule rule) => rule.Mode switch
    {
        LineSpacingMode.AtLeast or LineSpacingMode.Exact or LineSpacingMode.Leading => false,
        LineSpacingMode.Proportional => rule.Proportion is <= 0 or 1.0,
        _ => true,
    };

    /// <summary>
    /// The proportion a paragraph states, or <c>null</c> when it states none that changes anything.
    /// </summary>
    /// <remarks>
    /// The test for EditEngine's <c>SvxInterLineSpaceRule::Prop</c> branch, and therefore for which
    /// of two different arithmetics a line height goes through. A proportion of zero counts as
    /// stating nothing, because that is what <c>Apply</c> does with it and what the guard in
    /// <c>impedit3.cxx:1561</c> does with it — a default-constructed rule and an explicit hundred
    /// per cent are the same rule, and a hand-built body states neither.
    /// </remarks>
    private static double? Proportion(LineSpacingRule rule)
        => rule.Mode == LineSpacingMode.Proportional && rule.Proportion is > 0 and not 1.0
            ? rule.Proportion
            : null;

    /// <summary>
    /// A natural line height under a proportion, in the draw layer's unit and rounded once.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>proportion</c> is the <em>product</em> of everything that scales the line — the
    /// paragraph's own <c>a:lnSpc</c> and the shrink-to-fit search's spacing scale — because
    /// EditEngine multiplies them together and rounds the result once:
    /// <c>nHeight = fround(pLine-&gt;GetHeight() * fProportionalScale * fSpacingFactor)</c> below a
    /// hundred per cent, and a truncating <c>sal_Int32</c> conversion of the same product above it
    /// (<c>editeng/source/editeng/impedit3.cxx:1568,1575</c>). <c>basegfx::fround</c> is
    /// <c>(Int)(x + 0.5)</c> for a positive value
    /// (<c>include/basegfx/numeric/ftools.hxx:39-50</c>), so the two branches round in opposite
    /// directions and both are reproduced here.
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, round slides-r53 — probed against the
    /// installed 26.2.4.2, not read from the C++; the claim still holds.]
    /// <strong>Re-checked against 26.2.4.2 on 2026-08-21</strong> (<c>TODO.24-2-7-audit.md</c>) on
    /// <c>probes/slides-r53/make-linespace-probe.py</c>: forty <c>a:lnSpc/a:spcPct</c> boxes —
    /// 40, 50, 60, 80, 90, 93, 100, 110, 150 and 200 per cent at 11, 12, 24 and 40 pt, four lines
    /// each, <c>a:noAutofit</c>. Our pitch equals the reference's on <strong>40 of 40</strong> to a
    /// thousandth of a point, so both roundings survive the version move. The measurement also
    /// settles the deliberate divergence recorded above: at 40 per cent the reference draws a
    /// 19.191 pt pitch on a 47.991 pt natural line, which is <c>fround(0.40 x natural)</c> and not
    /// the 50 per cent <c>Apply</c> would have clamped it to. <strong>EditEngine has no such
    /// clamp in 26.2.4.2 either.</strong>
    /// </para>
    /// <para>
    /// <strong>Rounding the two factors separately is not a smaller version of this; it is a
    /// different answer.</strong> Measured against the six pitches
    /// <c>slide-autofit-grid.pptx</c>'s reference PDF states, one rounding of the product is exact
    /// on <strong>6 of 6</strong> to a thousandth of a point, rounding twice is exact on
    /// <strong>1</strong>, and the whole-twip arithmetic this replaces on <strong>none</strong>
    /// (<c>research/probes/slides-r16/fold-check.py</c>). Rounding twice is also worse than the
    /// defect on two of the six.
    /// </para>
    /// </remarks>
    private static Length Proportioned(Length natural, double proportion)
    {
        if (proportion is <= 0 or 1.0) return natural;

        double scaled = natural.Mm100 * proportion;

        return Length.FromMm100(proportion < 1.0
            ? (long)Math.Floor(scaled + 0.5)
            : (long)scaled);
    }

    /// <summary>
    /// Where the baseline sits in a line the <c>Prop</c> branch has scaled.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One em under the plain font-independent rule, and <strong>not</strong> one em as soon as a
    /// paragraph states a proportional line spacing other than 100%: EditEngine moves the baseline
    /// with the box rather than leaving it where the font would put it. Below a hundred per cent it
    /// <em>caps</em> the ascent at
    /// <c>fround(GetTxtHeight() × fSpacingFactor × fProportionalScale × 0.8)</c> and never raises
    /// it, which is what keeps a line whose ascent was already short where it was; above a hundred
    /// the ascent moves by the whole of the height's change
    /// (<c>editeng/source/editeng/impedit3.cxx:1564-1578</c>).
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, round slides-r53 — probed against the
    /// installed 26.2.4.2, not read from the C++; the claim still holds.]
    /// <strong>Re-checked against 26.2.4.2 on 2026-08-21</strong> (<c>TODO.24-2-7-audit.md</c>) on
    /// the same forty boxes <see cref="Proportioned"/> names, whose first baseline is what this
    /// method decides. Below a hundred per cent the reference's ascent is
    /// <c>fround(1.2 x em x proportion x 0.8)</c> — 35.676 pt at 93 per cent of 40 pt, where the
    /// arithmetic gives 35.712 and the uniform 0.028 pt residual accounts for the rest — and above
    /// it the ascent moves by the whole of the height's change: 63.937 at 150 per cent, where
    /// <c>em + (height − natural)</c> is 63.981. Both arms hold.
    /// </para>
    /// <para>
    /// The four-fifths is not derivable from anything; it is a constant EditEngine took from
    /// Writer's line formatter and it decides the first baseline of every shape in a deck that
    /// tightens its spacing. Measured on <c>ppt-features.ppt</c>, whose paragraphs all state 93%:
    /// the reference puts the 40 pt title's baseline 35.7 pt below the text top, where one em would
    /// be 40 and <c>1.2 × 40 × 0.93 × 0.8</c> is 35.71.
    /// </para>
    /// <para>
    /// It is also why <see cref="Spaced"/> cannot stand in for this. That method transcribes the
    /// <c>Off</c> branch — the one a paragraph stating no line spacing takes — which has the fit's
    /// scale to apply and <em>no</em> four-fifths. Running both in sequence would apply the fit's
    /// scale twice and the four-fifths to only one of them, which is why the caller picks a branch
    /// rather than composing them.
    /// </para>
    /// <para>
    /// <c>proportion</c> is the product of the paragraph's own and the fit's, and the rounding is a
    /// whole hundredth of a millimetre for the reason <see cref="Proportioned"/> gives.
    /// </para>
    /// </remarks>
    private static Length ProportionedAscent(
        Length em, Length natural, Length height, double proportion)
    {
        if (proportion is <= 0 or 1.0) return em;

        if (proportion < 1.0)
        {
            Length reduced = Length.FromMm100(
                (long)Math.Floor((natural.Mm100 * proportion * ShortSpacingAscent) + 0.5));
            return Length.Min(em, reduced);
        }

        return em + (height - natural);
    }

    /// <summary>
    /// The line EditEngine <em>appends</em> — for an empty paragraph, and after a paragraph's
    /// trailing hard line break — which is measured by a different rule from every other line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>It never picks up the shrink-to-fit's line-spacing scale.</strong>
    /// <c>ImpEditEngine::CreateAndInsertEmptyLine</c>
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:1851-1996) is reached twice — from
    /// <c>createLinesForEmptyParagraph</c> (<c>:514-524</c>) for a paragraph with no characters,
    /// and from the end of <c>CreateLines</c> (<c>:1843-1844</c>) when the paragraph's last line
    /// ended on an <c>EE_FEATURE_LINEBR</c> (<c>:1088-1094</c>) — and it carries its own copy of
    /// the four line-spacing arms. That copy has <strong>no
    /// <c>SvxInterLineSpaceRule::Off</c> arm at all</strong>, and none of its three arms calls
    /// <c>scaleYSpacingValue</c>, so <c>fSpacingY</c> reaches such a line by no route whatever.
    /// The ordinary arms are <c>:1528-1602</c>, and every one of those does.
    /// </para>
    /// <para>
    /// Three further differences come with it, and all three are in the same block
    /// (<c>:1912-1972</c>). <c>Min</c> and <c>Fix</c> take the paragraph's stated height
    /// <em>raw</em> where the ordinary arms scale it. <c>Prop</c> is whole-percent integer
    /// arithmetic — <c>nH = nTxtHeight * GetPropLineSpace() / 100</c>, which truncates — where the
    /// ordinary arm is <c>fround</c> of a double. And <c>Prop</c> is skipped entirely for the
    /// <em>first</em> paragraph of the body, under the comment "Not the very first line": the test
    /// is <c>nPara || pTmpLine-&gt;GetStartPortion()</c>, and a freshly constructed
    /// <c>EditLine</c> has a start portion of zero, so it reduces to the paragraph index.
    /// </para>
    /// <para>
    /// <strong>This is what decides a nearly-full autofitted box, and it decides it in the
    /// direction that makes the box overflow.</strong> Measured on page 2 of
    /// <c>slides/done-013/ppt/2015-Civil-Rights-Website-training.ppt</c>, an outline placeholder
    /// of 12870 units holding four empty paragraphs among its eight: at the fit's first level —
    /// full size, nine-tenths spacing — every one of its ten lines was being scaled to 1098 units,
    /// which fits by 113. Leaving the four appended lines at the 1220 that their own 90 per cent
    /// gives them puts that level at 13239, which overflows, so the reference's next level is
    /// taken and 30 pt is drawn where this tree drew 32. Its four bullet-to-bullet distances are
    /// <c>1029 + 253 + 1143 + 253</c> and <c>1029 + 1143 + 253</c> units — 75.912 pt and
    /// 68.741 pt — which is the text line and the appended line disagreeing by 114 units, and the
    /// last baseline lands within <strong>0.010 pt</strong> of the reference's once both rules are
    /// in place.
    /// </para>
    /// <para>
    /// The bullet area's own height can raise such a line further
    /// (<c>:1974-1985</c>, which halves the difference into the ascent). That is
    /// <see cref="BulletFloored"/>, and it is applied to this method's answer rather than inside
    /// it, because it is outside the four line-spacing arms in the reference too.
    /// </para>
    /// </remarks>
    private static PlacedLine Appended(
        LineBox box, Length em, Length natural, SlideParagraph paragraph, int paragraphIndex)
    {
        // SvxLineSpaceRule::Min and ::Fix, impedit3.cxx:1928-1950 -- the stated height, unscaled,
        // with the ascent moved by the whole of the change. Neither is guarded by the paragraph
        // index.
        if (paragraph.LineSpacing.Mode is LineSpacingMode.Exact or LineSpacingMode.AtLeast
            && paragraph.LineSpacing.Value > Length.Zero)
        {
            Length stated = paragraph.LineSpacing.Value;

            return paragraph.LineSpacing.Mode == LineSpacingMode.AtLeast && stated <= natural
                ? new PlacedLine(box, em, natural, natural)
                : new PlacedLine(box, em + (stated - natural), stated, natural);
        }

        // SvxInterLineSpaceRule::Prop, impedit3.cxx:1951-1969. Not for the body's first paragraph.
        if (paragraphIndex != 0 && Proportion(paragraph.LineSpacing) is { } proportion)
        {
            long percent = (long)Rounded(proportion * 100.0);
            Length height = Length.FromMm100(natural.Mm100 * percent / 100);

            // nDiff = GetHeight() - nH, capped at the ascent so it cannot go negative.
            Length drop = natural - height;
            if (drop > em) drop = em;

            return new PlacedLine(box, em - drop, height, natural);
        }

        return new PlacedLine(box, em, natural, natural);
    }

    /// <summary>
    /// Raises an <em>empty</em> paragraph's line to its bullet's box when the bullet is the taller
    /// of the two, halving the difference into the ascent.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The last block of <c>ImpEditEngine::CreateAndInsertEmptyLine</c>
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:1974-1985, this tree):
    /// </para>
    /// <code>
    /// if ( !bLineBreak )
    /// {
    ///     tools::Long nMinHeight = aBulletArea.GetHeight();
    ///     if ( nMinHeight &gt; static_cast&lt;tools::Long&gt;(pTmpLine-&gt;GetHeight()) )
    ///     {
    ///         tools::Long nDiff = nMinHeight - static_cast&lt;tools::Long&gt;(pTmpLine-&gt;GetHeight());
    ///         // distribute nDiff upwards and downwards
    ///         pTmpLine-&gt;SetMaxAscent( pTmpLine-&gt;GetMaxAscent() + nDiff/2 );
    ///         pTmpLine-&gt;SetHeight( nMinHeight );
    ///     }
    /// }
    /// </code>
    /// <para>
    /// Four properties of that hunk decide where it can be seen, and all four were measured
    /// against 26.2.4.2 before it was written — the source below is
    /// <c>27.2.0.0.alpha0+</c> and is the explanation, not the evidence.
    /// </para>
    /// <list type="bullet">
    /// <item><description>
    /// <strong>It is only for a paragraph with no characters.</strong> <c>bLineBreak</c> is
    /// <c>GetNode()-&gt;Len() &gt; 0</c>, so the line appended after a paragraph's trailing hard
    /// break is excluded and a paragraph holding text is never floored by its own bullet.
    /// </description></item>
    /// <item><description>
    /// <strong>It is the last thing done to the line</strong>, after the <c>Min</c>, <c>Fix</c>
    /// and <c>Prop</c> arms — so a proportional line spacing cannot shrink the line below the
    /// box, and the floor does not scale with the percentage.
    /// </description></item>
    /// <item><description>
    /// <strong>The box exists even when no bullet is drawn.</strong> <c>GetBulletArea</c> goes
    /// through <c>Outliner::GetNumberFormat</c>, which reads the paragraph's depth and the
    /// numbering rule and never <c>EE_PARA_BULLETSTATE</c>; the empty-paragraph suppression the
    /// readers apply (see <see cref="Shaped"/>) is a painting rule.
    /// </description></item>
    /// <item><description>
    /// <strong>A level whose format is <c>SVX_NUM_NUMBER_NONE</c> has a box of no height</strong>
    /// — <c>Outliner::ImplGetBulletSize</c> returns <c>Size(0, 0)</c> for it
    /// (<c>outliner.cxx</c>:1329-1332) — and so does a paragraph at no level at all, whose
    /// <c>GetNumberFormat</c> is null. Both are a null <see cref="SlideParagraph.Marker"/> here.
    /// </description></item>
    /// </list>
    /// <para>
    /// <strong>Measured by single-variable experiment at 26.2.4.2</strong>, on its own flat ODP of
    /// <c>slides/done-012/ppt/JesuitAssocOfStudentPersonnel.ppt</c> page 24 cut to one slide, with
    /// the shrink-to-fit turned off so the answer is a continuous baseline pitch rather than a
    /// <c>constScaleLevels</c> row (<c>probes/slides-size2-r110</c>). Two 20 pt bulleted items
    /// separated by one empty paragraph, everything else held fixed:
    /// </para>
    /// <list type="bullet">
    /// <item><description>
    /// Sweeping <c>fo:line-height</c> 100, 95, 90, 85, 80, 70, 60, 50 %, the reference's empty
    /// line is <strong>787 hundredths of a millimetre at every percentage at or below 90</strong>
    /// and follows the text line above it at 95 and 100. 787 is Liberation Sans' ascent plus
    /// descent at 20 pt; <c>1.2 em × 0.8</c> is 678, which is what this tree drew.
    /// </description></item>
    /// <item><description>
    /// Moving the same empty paragraph between five structures at 80 %: second
    /// <c>&lt;text:p&gt;</c> of a <c>&lt;text:list-item&gt;</c> — floored; its own
    /// <c>&lt;text:list-item&gt;</c> — floored; a <c>&lt;text:list-header&gt;</c> of a list style
    /// whose level 1 is a <c>text:list-level-style-bullet</c> — floored; a
    /// <c>&lt;text:list-header&gt;</c> of one whose level 1 is a
    /// <c>text:list-level-style-number</c> with an empty <c>style:num-format</c> — <em>not</em>
    /// floored; a bare <c>&lt;text:p&gt;</c> outside every list — <em>not</em> floored. Swapping
    /// the paragraph <em>style</em> between the floored and unfloored cases moves nothing, so it
    /// is the level's bullet and not the paragraph's margins.
    /// </description></item>
    /// <item><description>
    /// <strong>The box is the bullet's own, which identifies it.</strong> Setting the level's
    /// <c>fo:font-size</c> to 50 % removes the floor entirely (the box is 394, below the line);
    /// 200 % raises the empty line to 1576, which is the face's ascent plus descent at 40 pt; and
    /// naming <c>DejaVu Sans</c> instead of <c>Arial</c> at 100 % moves it 787 → 821, which is
    /// that face's <c>(1901 + 483) / 2048</c> against Liberation Sans' <c>(1854 + 434) / 2048</c>.
    /// </description></item>
    /// </list>
    /// <para>
    /// The height is <see cref="BulletBoxHeight"/>'s — the same cached, <em>unscaled</em> box the
    /// bullet is centred in, for the same reason and on the same measurement.
    /// </para>
    /// </remarks>
    private static PlacedLine BulletFloored(
        PlacedLine line, SlideParagraph paragraph, SlideFonts fonts, MetricGrid device)
    {
        // bLineBreak: an appended line whose paragraph has characters is the line after a
        // trailing hard break, and the reference's guard excludes it.
        if (paragraph.Text.Length != 0) return line;

        // ...and an empty paragraph has a box only where its reader left it at its level. See
        // SlideParagraph.EmptyKeepsMarkerLevel: OOXML sets the level to -1 and the other two do
        // not, so this is a .ppt and ODF rule and the OOXML column cannot reach it.
        if (!paragraph.EmptyKeepsMarkerLevel) return line;

        Length box;

        // A picture bullet's box is `pFmt->GetGraphicSize()` rather than a face's ascent plus
        // descent (`Outliner::ImplGetBulletSize`, `outliner.cxx`:1345-1350), and it is stated in
        // absolute units at import — so no font is resolved for it and the fit does not scale it.
        if (paragraph.Marker is { Picture: { } picture })
        {
            box = picture.Height;
        }
        else if (Shaped(paragraph, Scaling.None, fonts, requireText: false) is
                 { Face: { } face } marked)
        {
            box = BulletBoxHeight(
                LineSpacing.Resolve(face, device), paragraph.Runs[0], marked.Marker);
        }
        else
        {
            return line;
        }

        if (box <= line.Height) return line;

        // nDiff/2 is integer division of hundredths of a millimetre, and SetHeight's one-argument
        // form sets the text height to the same value (EditLine.cxx:79-86).
        Length half = Length.FromMm100((box.Mm100 - line.Height.Mm100) / 2);

        return line with { Ascent = line.Ascent + half, Height = box, TextHeight = box };
    }

    /// <summary>
    /// The em an appended line after a trailing line break is measured at: the paragraph's
    /// <em>last</em> run's size.
    /// </summary>
    /// <remarks>
    /// <c>CreateAndInsertEmptyLine</c> seeks the cursor to the paragraph's end for such a line and
    /// to position zero for an empty paragraph —
    /// <c>SeekCursor(pNode, bLineBreak ? pNode-&gt;Len() : 0, aTmpFont)</c>
    /// (<c>impedit3.cxx</c>:1896) — so the two ends of a paragraph whose runs differ in size give
    /// different answers. <see cref="LargestSize"/> already answers the first run for an empty
    /// range, which is the second case.
    /// </remarks>
    private static Length EndSize(List<FormattedRun> runs, List<RunStyle> styles)
        => runs.Count > 0 ? Nominal(runs, styles, runs.Count - 1) : Length.FromPoints(18);

    /// <summary>
    /// Applies the fit's spacing scale to a line, which moves its baseline as well as its box.
    /// </summary>
    /// <remarks>
    /// <para>
    /// EditEngine's <c>SvxInterLineSpaceRule::Off</c> branch — the one a paragraph that states no
    /// line spacing takes — turns the scale into a proportional spacing and applies it to both:
    /// the height is multiplied and the ascent is <em>capped</em> at the text height times the
    /// same factor, never raised (<c>editeng/source/editeng/impedit3.cxx:1584-1600</c>). Capping
    /// rather than assigning is what keeps a line whose ascent was already short where it was.
    /// </para>
    /// <para>
    /// <strong>In hundredths of a millimetre, like every other line height here.</strong> The
    /// branch is <c>fround(pLine-&gt;GetHeight() * fSpacingFactor)</c> and <c>SetHeight</c> takes a
    /// <c>sal_uInt16</c> of the outliner's map unit, which for a draw object is 1/100 mm — so the
    /// reference cannot hold a line height finer than that whatever the arithmetic produces.
    /// Scaling in EMU instead leaves fractions of a unit the reference has nowhere to put, and
    /// they accumulate down the block: this is the same defect as the one
    /// <see cref="Proportioned"/> documents, in the branch that takes no stated proportion.
    /// </para>
    /// <para>
    /// The paragraph that *does* state one never reaches here — its caller takes the <c>Prop</c>
    /// branch instead, because the two are alternatives in EditEngine and composing them would
    /// apply this scale twice.
    /// </para>
    /// </remarks>
    private static PlacedLine Spaced(PlacedLine line, Scaling scaling)
    {
        if (scaling.Spacing is <= 0 or >= 1.0) return line;

        Length ascent = Length.FromMm100(
            (long)Math.Floor((line.TextHeight.Mm100 * scaling.Spacing) + 0.5));
        Length height = Length.FromMm100(
            (long)Math.Floor((line.Height.Mm100 * scaling.Spacing) + 0.5));

        return line with
        {
            Ascent = line.Ascent > Length.Zero && line.Ascent <= ascent ? line.Ascent : ascent,
            Height = height,
        };
    }

    /// <summary>
    /// Emits one line's glyph runs, one per formatting change along it, placed at its tab stops.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A tab is the one character whose width is not a property of the font: it advances the pen to
    /// the next stop. Letting it be shaped like any other character advances the pen by whatever
    /// the face happens to give U+0009, which is nothing to do with where the stop is —
    /// <c>policy-pesentation.ppt</c>'s conclusion is positioned by three of them and landed an inch
    /// and a half to the left of where LibreOffice draws it.
    /// </para>
    /// <para>
    /// The word processor has done this since tabs existed there
    /// (<c>PageDrawing.Stretches</c>); this is the same <see cref="TabRuler"/> over the slide's own
    /// paragraph format, and an untabbed line — nearly all of them — goes through the identical
    /// code path as a single stretch at offset zero, so the two cannot drift apart.
    /// </para>
    /// </remarks>
    private static void Emit(
        List<PlacedGlyphRun> placed,
        Block block,
        PlacedLine line,
        Length areaLeft,
        Length top,
        bool isFirstLine)
    {
        int start = line.Box.Line.Start;
        int end = Math.Min(line.Box.Line.VisibleEnd, block.Measured.Text.Length);
        if (end <= start) return;

        Length lineLeft = areaLeft + line.Box.Left;
        Length baseline = top + line.Ascent;

        List<TabbedSegment> stretches = Stretches(block, start, end, isFirstLine);

        for (int index = 0; index < stretches.Count; index++)
        {
            // The justification belongs to the last stretch alone: a tab is a fixed portion whose glue is
            // nought, so the stretch it closes is stretched by nothing and only the last one reaches the
            // right margin's glue. `ParagraphLayouter.Justification` counts the same blanks.
            Length spaceAdd = index == stretches.Count - 1 ? line.Box.SpaceAdd : Length.Zero;

            EmitStretch(
                placed, block, stretches[index], lineLeft + stretches[index].Left, baseline, spaceAdd);
        }
    }

    /// <summary>The stretches a line's tabs divide it into, each placed at its stop.</summary>
    private static List<TabbedSegment> Stretches(
        Block block, int start, int end, bool isFirstLine)
    {
        if (!TabRuler.HasTab(block.Measured.Text, start, end))
        {
            return [new TabbedSegment(start, end, Length.Zero, Length.Zero)];
        }

        return TabRuler.Segments(
            block.Measured.Text, start, end, block.Format,
            block.Measured.WidthBetween, isFirstLine);
    }

    /// <summary>Emits one stretch of a line, starting at a pen the caller has placed.</summary>
    private static void EmitStretch(
        List<PlacedGlyphRun> placed,
        Block block,
        TabbedSegment segment,
        Length pen,
        Length baseline,
        Length spaceAdd)
    {
        int start = segment.Start;
        int end = segment.End;
        if (end <= start) return;

        foreach (FormattedRun run in block.Measured.RunsBetween(start, end))
        {
            string text = block.Measured.Text[run.Start..run.End];
            ShapedText shaped = TextShaper.Default.Shape(run.Face, text, run.EffectiveShaping);
            if (shaped.Glyphs.Count == 0) continue;

            // A superscript's own baseline, which the rules under and through it share:
            // EditEngine moves the pen and leaves the line's baseline where it was
            // (editeng/source/items/svxfont.cxx:549-558).
            Length pitch = baseline - block.RiseAt(run.Start);

            GlyphRun glyphs = Build(
                shaped, text, run.EmSize,
                block.FontFor(run.Start, run.Face) ?? Reference(run.Face),
                new DocPoint(pen, pitch),
                spaceAdd,
                run.Tracking);

            Length advance = Length.Zero;
            foreach (PositionedGlyph glyph in glyphs.Glyphs) advance += glyph.Advance;

            Colour colour = block.ColourAt(run.Start);

            // The shadow is the same glyphs drawn first, down and to the right, and it carries
            // neither the underline nor the strikethrough — those are drawn once, over the top.
            if (block.ShadowedAt(run.Start) && ShadowOffset(run.Face, run.EmSize) is { } offset)
            {
                placed.Add(new PlacedGlyphRun(
                    glyphs with { Origin = new DocPoint(pen + offset, pitch + offset) },
                    ShadowColour(colour),
                    null));
            }

            placed.Add(new PlacedGlyphRun(
                glyphs,
                colour,
                Rules(block.DecorationAt(run.Start), run.Face, run.EmSize, pen, pitch, advance)));

            // The pen carries across the runs of a line, so the second run starts where the first
            // ended rather than back at the margin.
            pen += advance;
        }
    }

    /// <summary>
    /// How far down and right a shadowed run's second copy is drawn, or null when the offset
    /// rounds to nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>vcl/source/outdev/text.cxx:394-407</c> computes it in <em>device units</em> from the
    /// font instance's line height:
    /// <c>nOff = 1 + ((mnLineHeight − 24) / 24)</c>, with C's truncating division. PDF export runs
    /// on a 720 dpi reference device, so one device unit is a tenth of a point and the ladder is
    /// 0.1 pt wide.
    /// </para>
    /// <para>
    /// <b>Which line height feeds it was the open question, and it is now measured rather than
    /// read.</b> The three offsets a previous round took from two decks bounded the factor to
    /// [1.073, 1.122] em, an interval containing both of Liberation Sans's candidate sums — its
    /// <c>hhea</c> ascent+descent (1.1172) and its <c>OS/2</c> typo sum plus typo gap (1.0884) —
    /// without separating them. A probe of 171 sizes from 10 to 95 pt in half-point steps,
    /// authored as a flat ODF document with <c>fo:text-shadow</c> (which reaches this same vcl
    /// code, the rule living at draw time rather than in any importer) and converted by the
    /// reference binary, separates them decisively: the <c>hhea</c> sum mismatches 3 of 171 under
    /// a naive single rounding and <b>0 of 171</b> under the rounding below, where the typo sum
    /// mismatches 107. Repeated against DejaVu Sans, whose two candidates are further apart
    /// (1.1641 against 1.2002), it is again 0 of 171 against 133. <b>342 of 342 exact.</b> The
    /// same ladder reproduces the three <c>.ppt</c> offsets that posed the question — 14, 15 and
    /// 17 device units at 32.00, 33.99 and 38.01 pt.
    /// </para>
    /// <para>
    /// The rounding is <em>per metric, not on the sum</em>: rounding ascent and descent separately
    /// is what makes it exact, and rounding their sum once misses 3 of 171 on Liberation Sans and
    /// 4 on DejaVu. <b>The probe could not see the third rounding</b> — every size on its ladder
    /// was a whole half-point, so the em was already a whole number of device units and rounding
    /// it changed nothing. The corpus decided that one; see the comment on <c>perEm</c> below. <see cref="LineSpacing.Resolve(OpenTypeFace, MetricGrid?, bool)"/> is the
    /// right source because it already implements vcl's own precedence — <c>hhea</c>, then
    /// <c>OS/2</c>'s Windows metrics, then the typographic ones when the face asks for them by
    /// name — which is exactly what fills <c>ImplFontMetricData</c>. The line <em>gap</em> is
    /// excluded: <c>mnLineHeight</c> is ascent plus descent.
    /// </para>
    /// </remarks>
    /// <param name="face">The face the run is drawn in.</param>
    /// <param name="emSize">The size it is drawn at.</param>
    private static Length? ShadowOffset(OpenTypeFace face, Length emSize)
    {
        LineMetrics metrics = LineSpacing.Resolve(face);
        if (metrics.UnitsPerEm <= 0) return null;

        // The reference device is 720 dpi, so a point is ten units and the font's size in them is
        // the pixels-per-em the metric is scaled by. It is rounded to a whole unit first, because
        // a device font is requested at an integer height and its metrics are scaled to that —
        // and a slide's autofit routinely asks for a fractional size, so this is the common case
        // rather than an edge. Thailand17's shadowed body text is drawn at 29.9906 pt, where
        // rounding the em first gives 300 units and a 1.4 pt offset, which is the reference's, and
        // not rounding gives 299.906 and 1.3 pt, which is a unit short on 38 runs of one deck.
        double perEm = Math.Round(emSize.Points * DeviceUnitsPerPoint, MidpointRounding.AwayFromZero);
        long lineHeight =
            (long)Math.Round(metrics.Ascent * perEm / metrics.UnitsPerEm, MidpointRounding.AwayFromZero)
            + (long)Math.Round(metrics.Descent * perEm / metrics.UnitsPerEm, MidpointRounding.AwayFromZero);

        long units = 1 + ((lineHeight - 24) / 24);
        return units <= 0 ? null : Length.FromPoints(units / DeviceUnitsPerPoint);
    }

    /// <summary>Device units in a point on the 720 dpi reference device PDF export runs on.</summary>
    private const double DeviceUnitsPerPoint = 10.0;

    /// <summary>
    /// The colour a shadow is drawn in, which depends only on how dark the text is.
    /// </summary>
    /// <remarks>
    /// <c>vcl/source/outdev/text.cxx:399-403</c> paints the shadow light grey under black or
    /// near-black text and black under everything else, so the shadow of a black heading is
    /// visible rather than invisible. <b>Measured over nineteen text colours</b> against the
    /// reference binary: the cut is at <c>Color::GetLuminance</c> — <c>(B×29 + G×151 + R×76) >> 8</c>
    /// — being under 8, which the <c>== COL_BLACK</c> half of the condition is subsumed by. The
    /// probe pins the boundary on both sides: <c>#050505</c> (luminance 5) takes the grey branch
    /// and <c>#0A0A0A</c> (luminance 10) the black one, and so do the two colours that separate
    /// the luminance from a channel maximum — <c>#080000</c> (luminance 2, grey) and
    /// <c>#001000</c> (luminance 9, black).
    /// </remarks>
    /// <param name="text">The colour the run itself is drawn in.</param>
    private static Colour ShadowColour(Colour text)
        => ((text.B * 29) + (text.G * 151) + (text.R * 76)) >> 8 < 8
            ? Colour.FromRgb(0xC0C0C0)
            : Colour.Black;

    /// <summary>
    /// The rectangles a run's underline and strikethrough fill, or null when it has neither.
    /// </summary>
    /// <remarks>
    /// Computed here because this is the last point at which the <see cref="OpenTypeFace"/> is in
    /// hand: the offset and thickness are the face's own <c>post</c> and <c>OS/2</c> values
    /// through <see cref="LineSpacing.ResolveDecorations(OpenTypeFace, LineMetrics)"/>, which falls back to a fraction of
    /// the em for a face declaring zero — otherwise a font that declines to say would draw a
    /// rule of no thickness, which is to say none.
    /// </remarks>
    private static List<DocRect>? Rules(
        (bool Underline, bool Strikethrough) decoration,
        OpenTypeFace face,
        Length size,
        Length left,
        Length baseline,
        Length width)
    {
        if (!decoration.Underline && !decoration.Strikethrough) return null;
        if (size <= Length.Zero || width <= Length.Zero) return null;

        int unitsPerEm = face.UnitsPerEm > 0 ? face.UnitsPerEm : 1000;
        LineMetrics line = LineSpacing.Resolve(face);

        // BOTH the thickness and the offset come off the PDF writer's own 720 dpi device, in whole
        // hundredths of a millimetre, for a slide exactly as for a sheet. Round 120 took only the
        // thickness from there and left the offset in design units; round 123 measured what that
        // costs and moved it. See `LineSpacing.ResolveRuleWidths`.
        LineSpacing.RuleWidths widths =
            LineSpacing.ResolveRuleWidths(face, line, size, MetricGrid.TextLine);

        List<DocRect> rules = [];

        if (decoration.Underline)
        {
            Length thickness = widths.Underline;
            if (thickness > Length.Zero)
            {
                rules.Add(new DocRect(
                    left, baseline + widths.UnderlineOffset, width, thickness));
            }
        }

        if (decoration.Strikethrough)
        {
            Length thickness = widths.Strikeout;
            if (thickness > Length.Zero)
            {
                rules.Add(new DocRect(
                    left, baseline + widths.StrikeoutOffset, width, thickness));
            }
        }

        return rules.Count == 0 ? null : rules;
    }

    /// <summary>Builds a glyph run from a shaped stretch of text at an origin.</summary>
    /// <remarks>
    /// Each glyph's offset is relative to the run's origin and the pen accumulates across them,
    /// which is what makes a run one draw call. The vertical offset is negated because a shaper's
    /// is up-positive and document space is down-positive.
    /// </remarks>
    private static GlyphRun Build(
        ShapedText shaped,
        string text,
        Length emSize,
        FontReference font,
        DocPoint origin,
        Length spaceAdd,
        Length tracking = default)
    {
        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);

        Length pen = Length.Zero;
        int remaining = shaped.Glyphs.Count;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, emSize);

            // Tracking is the gap *between* characters, so the last glyph of the run does not
            // carry one — which is also what keeps the drawn pen within a tracking unit of the
            // width the measurement charged. See FormattedRun.Tracking.
            if (tracking != Length.Zero && --remaining > 0) advance += tracking;

            if (spaceAdd != Length.Zero
                && glyph.Cluster >= 0
                && glyph.Cluster < text.Length
                && text[glyph.Cluster] == ' ')
            {
                advance += spaceAdd;
            }

            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(
                    pen + shaped.Scale(glyph.OffsetX, emSize),
                    -shaped.Scale(glyph.OffsetY, emSize)),
                advance));

            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new GlyphRun
        {
            Font = font,
            FontSize = emSize,
            Origin = origin,
            Glyphs = glyphs,
            Text = text,
            ClusterMap = clusters,
        };
    }

    /// <summary>
    /// A reference for a face that did not come through a resolver.
    /// </summary>
    /// <remarks>
    /// The last resort, and it names the family because that is all an
    /// <see cref="OpenTypeFace"/> knows: it is a parsed table directory with no memory of the file
    /// it was read out of. A backend given this can group runs by font and can measure them, but
    /// it cannot open the face — so a PDF built from it references the family and embeds no font
    /// program. Everything laid out here reaches <see cref="Emit"/> with
    /// <see cref="RunStyle.Font"/> set instead; see the remark on that.
    /// </remarks>
    private static FontReference Reference(OpenTypeFace face) => new()
    {
        FamilyName = face.FamilyName ?? string.Empty,
        Weight = face.Weight,
        IsItalic = face.IsItalic,
        FaceKey = face.FamilyName ?? string.Empty,
    };

    /// <summary>
    /// What a run carries that changes how it is drawn but not how wide it is.
    /// </summary>
    /// <param name="Colour">The colour it is drawn in.</param>
    /// <param name="Font">
    /// The reference the run's face was resolved through, whose <c>FaceKey</c> is the font file's
    /// own path.
    /// </param>
    /// <param name="Face">
    /// The face that reference names, kept so that a sub-run drawn in a <em>different</em> face
    /// cannot be embedded from it. See <see cref="Block.FontFor"/>.
    /// </param>
    /// <remarks>
    /// <para>
    /// The font reference travels here rather than on <see cref="FormattedRun"/> for the same
    /// reason the colour does: <see cref="MeasuredParagraph"/> keeps only what changes a
    /// measurement, and which file a face was loaded from moves no line break.
    /// </para>
    /// <para>
    /// It has to travel <em>somewhere</em>, though, and that is the whole of this fix. Rebuilding
    /// the reference from the face — <c>FaceKey = face.FamilyName</c>, which is what
    /// <see cref="Reference"/> still does for hand-built input — hands the PDF writer a key
    /// <c>FileFontProvider</c> cannot open, so every deck rendered to PDF referenced its faces and
    /// embedded none of them. Measured with <c>pdffonts</c> on <c>deck-features.pptx</c>: both
    /// <c>LiberationSans</c> and <c>OpenSymbol</c> reported <c>emb no</c>, while the same
    /// document's text extracted at 43 of 43 words matching LibreOffice — which is exactly why no
    /// existing check could see it.
    /// </para>
    /// </remarks>
    /// <param name="IsUnderlined">Whether a rule is drawn under it.</param>
    /// <param name="IsStruckThrough">Whether a rule is drawn through it.</param>
    /// <param name="Rise">
    /// How far above its line's baseline the run is drawn, negative for a subscript. The
    /// <em>size</em> half of an escapement travels on the measured run, because it moves line
    /// breaks; this half does not, so it travels with the colour and the decorations.
    /// </param>
    /// <param name="NominalSize">
    /// The size the run would take were it not escaped. A line's height is derived from this
    /// rather than from the shrunk size, because EditEngine forces the proportion back to 100%
    /// before it asks the font for a metric
    /// (<c>editeng/source/editeng/impedit3.cxx:3121-3126</c>).
    /// </param>
    /// <param name="IsShadowed">
    /// Whether the run casts the per-character drop shadow. Travels with the colour and the
    /// decorations rather than with the measured run, because the shadow is drawn from the same
    /// glyphs at an offset and so moves no line break.
    /// </param>
    /// <param name="Escapement">
    /// The whole escapement, offset and proportion together, because the height a line takes needs
    /// both and neither <see cref="Rise"/> nor <see cref="NominalSize"/> carries the pair. See
    /// <see cref="FixedCellBox"/>.
    /// </param>
    private readonly record struct RunStyle(
        Colour Colour,
        FontReference? Font,
        OpenTypeFace? Face,
        bool IsUnderlined = false,
        bool IsStruckThrough = false,
        Length Rise = default,
        Length NominalSize = default,
        bool IsShadowed = false,
        SlideEscapement Escapement = default);

    /// <summary>One paragraph, measured and broken.</summary>
    private sealed record Block(
        SlideParagraph Paragraph,
        MeasuredParagraph Measured,
        IReadOnlyList<RunStyle> Styles,
        IReadOnlyList<PlacedLine> Lines,
        Length Height,
        Scaling Scaling,
        ParagraphFormat Format,
        IGlyphFallbackResolver? Fallback = null)
    {
        /// <summary>The space above the paragraph, after the fit's spacing scale.</summary>
        /// <remarks>
        /// See <see cref="ScaledSpace"/> for why the scale reaches this and for the probe that
        /// settled it. Kept here as well as in <see cref="Height"/> because the two must agree:
        /// the height a fit is chosen by and the height the block is then stacked at are the same
        /// quantity, and letting them differ moves every anchored block by the difference.
        /// </remarks>
        public Length SpaceBefore => ScaledSpace(Paragraph.SpaceBefore, Scaling);

        /// <summary>The space below the paragraph, scaled the same way.</summary>
        public Length SpaceAfter => ScaledSpace(Paragraph.SpaceAfter, Scaling);

        /// <summary>The colour covering a character, or black when no run does.</summary>
        /// <remarks>
        /// Looked up by position rather than carried on the measured run, because
        /// <see cref="MeasuredParagraph"/> keeps only what changes a measurement — a colour does
        /// not move a line break, so it travels with whatever draws the text.
        /// </remarks>
        public Colour ColourAt(int index) => StyleAt(index).Colour;

        /// <summary>The decorations covering a character, both false when no run does.</summary>
        public (bool Underline, bool Strikethrough) DecorationAt(int index)
        {
            RunStyle style = StyleAt(index);
            return (style.IsUnderlined, style.IsStruckThrough);
        }

        /// <summary>Whether the character casts the per-character drop shadow.</summary>
        public bool ShadowedAt(int index) => StyleAt(index).IsShadowed;

        /// <summary>
        /// How far above the line's baseline a character is drawn, zero for ordinary text.
        /// </summary>
        public Length RiseAt(int index) => StyleAt(index).Rise;

        /// <summary>
        /// The resolved reference for a sub-run, or null when nothing here can name its face.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Matched on the face and not only on the position. The runs a caller measures are cut
        /// further before they are drawn — by direction, by script, and by <c>FontItemiser</c>
        /// where the run's own face has no glyph for a character — and only the last of those
        /// changes the face. A reference handed to a sub-run drawn in a face it does not name
        /// would embed <em>the wrong font file</em>, which is worse than embedding none: the
        /// glyph indices the shaper produced belong to the other face, so the page would draw
        /// confidently wrong letters.
        /// </para>
        /// <para>
        /// Reference equality is the right test because the face cache hands back one instance
        /// per resolved request, and <c>FontItemiser</c> passes the primary face straight through
        /// when it does not substitute, so the guard costs a pointer comparison.
        /// </para>
        /// <para>
        /// When the faces <em>do</em> differ the sub-run was substituted, and the resolver that
        /// found the substitute is the only thing that can name the file it came from — an
        /// <see cref="OpenTypeFace"/> is a parsed table directory with no memory of its own path,
        /// so a reference built from the face alone reaches the PDF writer with no font program
        /// behind it and is announced without being embedded, which the corpus gate scores as a
        /// failure and rightly: a reader without that font installed sees nothing. This is the
        /// same route <c>PageDrawing.ByFace</c> takes in the word processor.
        /// </para>
        /// </remarks>
        /// <param name="index">A character the sub-run covers.</param>
        /// <param name="face">The face the sub-run will actually be shaped and drawn in.</param>
        public FontReference? FontFor(int index, OpenTypeFace face)
        {
            RunStyle style = StyleAt(index);
            if (ReferenceEquals(style.Face, face)) return style.Font;

            // A substituted face has no memory of what was asked for, so the lean has to be carried
            // across with the request. Both states count as italic: a family whose italic is
            // installed answers with an italic face, and one whose italic is not answers upright
            // with a synthetic oblique instead — and 26.2.4.2 shears the fallback for both. See
            // IGlyphFallbackResolver.ReferenceFor(OpenTypeFace, bool), which was measured through
            // `.pptx` and `.fodp` among four other filters.
            bool italic = (style.Face?.IsItalic ?? false) || (style.Font?.SyntheticOblique ?? false);
            return Fallback?.ReferenceFor(face, italic);
        }

        private RunStyle StyleAt(int index)
        {
            for (int i = 0; i < Paragraph.Runs.Count && i < Styles.Count; i++)
            {
                if (index >= Paragraph.Runs[i].Start && index < Paragraph.Runs[i].End)
                    return Styles[i];
            }

            return Styles.Count > 0 ? Styles[0] : new RunStyle(Colour.Black, null, null);
        }
    }

    /// <summary>One line, with the height EditEngine's rule gives it.</summary>
    /// <param name="Box">The shared layouter's box: which characters, and where across the area.</param>
    /// <param name="Ascent">
    /// How far the baseline sits below the line's top, which under the font-independent rule is
    /// the em size itself.
    /// </param>
    /// <param name="Height">The distance to the next line's top.</param>
    /// <param name="TextHeight">
    /// The height before the paragraph's line-spacing rule was applied, which is what EditEngine
    /// calls the line's <em>text</em> height (<c>SetHeight(nHeight, nTxtHeight)</c>,
    /// <c>editeng/source/editeng/impedit3.cxx:1574-1579</c>). Only a bullet needs it, and it needs
    /// it because the bullet is centred on the text rather than on the line: a paragraph set at
    /// 150% keeps its marker where single spacing would have put it.
    /// </param>
    /// <param name="ContinuesField">
    /// Whether the line is the spill of a field that began on an earlier one — see
    /// <see cref="SlideTextRun.IsField"/>. Such a line is a line to the painter and not to the
    /// formatter: it is stacked one ascent below its predecessor and contributes nothing to the
    /// block's measured height.
    /// </param>
    private readonly record struct PlacedLine(
        LineBox Box,
        Length Ascent,
        Length Height,
        Length TextHeight,
        bool ContinuesField = false);
}

using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Numbering;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;

namespace Paperless.Presentations.Layout;

/// <summary>
/// Lays a shape's text body out inside its text rectangle.
/// </summary>
/// <remarks>
/// <para>
/// Line breaking, indents and horizontal alignment all come from <c>Paperless.Text</c>'s
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
    /// Lays a body out and returns its glyph runs, positioned in the given rectangle's space.
    /// </summary>
    /// <param name="body">The text body.</param>
    /// <param name="textRectangle">
    /// The shape's text rectangle, in whatever space the caller wants the runs in — the shape's
    /// own for a rotated shape, the slide's for an upright one. The insets are applied here.
    /// </param>
    /// <param name="fonts">The face cache.</param>
    public static List<PlacedGlyphRun> Place(
        SlideTextBody body, DocRect textRectangle, SlideFonts fonts)
    {
        ArgumentNullException.ThrowIfNull(body);
        ArgumentNullException.ThrowIfNull(fonts);

        DocRect area = textRectangle.Deflate(body.Insets);
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

            bool first = true;
            foreach (PlacedLine line in block.Lines)
            {
                if (first) EmitMarker(placed, block, line, area.X, top, fonts);

                Emit(placed, block, line, area.X, top, first);
                first = false;
                top += line.Height;
            }

            if (index != blocks.Count - 1) top += block.SpaceAfter;
        }

        return placed;
    }

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

        foreach (SlideParagraph paragraph in body.Paragraphs)
        {
            Block? block = Measure(
                paragraph, body, width, fonts, scaling, fontIndependentLineSpacing,
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
    /// dropped. Measured against LibreOffice 24.2.7.2 on
    /// <c>slides/batch-002/ppt/gfopportunitiesforlinkagespres_2010_en.ppt</c>, whose eighth slide
    /// carries four empty paragraphs after its three bullets: the reference fits that text at
    /// 25 pt, and moving three of those empty paragraphs into the middle of the body makes the
    /// same LibreOffice fit it at 21 pt with nine-tenths line spacing — which is exactly what
    /// Paperless produced for the untouched deck while it measured every paragraph.
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
        Block block,
        PlacedLine line,
        Length areaLeft,
        Length top,
        SlideFonts fonts)
    {
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
            LineMetrics metrics = LineSpacing.Resolve(face);
            Length ascent = Rounded(metrics.ScaledAscent(size));
            Length descent = Rounded(metrics.ScaledDescent(size));

            baseline = top
                       + line.Height
                       - Length.FromEmu(line.TextHeight.Emu / 2)
                       + Length.FromEmu((ascent.Emu - descent.Emu) / 2);
        }

        placed.Add(new PlacedGlyphRun(
            Build(shaped, marked.Text, size, marked.Reference ?? Reference(face),
                  new DocPoint(pen, baseline), Length.Zero),
            marker.Colour ?? first.Colour));
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
    /// </remarks>
    private static MarkedParagraph? Shaped(
        SlideParagraph paragraph, Scaling scaling, SlideFonts fonts)
    {
        if (paragraph.Marker is not { } marker) return null;
        if (marker.Text.Length == 0) return null;
        if (paragraph.Text.Length == 0) return null;
        if (paragraph.Runs.Count == 0) return null;

        SlideTextRun first = paragraph.Runs[0];
        (OpenTypeFace? face, FontReference? reference) = fonts.Resolve(
            marker.Typeface ?? first.Typeface, first.Weight, first.IsItalic);

        if (face is null) return null;

        string text = marker.Text;
        if (Recoded(marker, reference) is { } recoded)
        {
            // The recode and the face go together: the code point means nothing anywhere but
            // OpenSymbol, so a resolution that failed leaves both alone rather than drawing it
            // out of whatever the request happened to land on.
            (OpenTypeFace? symbol, FontReference? symbolReference) = fonts.Resolve(
                SymbolFontRecode.SubstituteFamily, first.Weight, first.IsItalic);

            if (symbol is not null)
            {
                (face, reference, text) = (symbol, symbolReference, recoded);
            }
        }

        if (ReferenceEquals(text, marker.Text))
        {
            text = OutlineNumbers.NormaliseBullet(marker.Text);
        }

        // The marker shrinks with the text it labels: the fit scales the whole outliner, and a
        // bullet left at its authored size on a node scaled to a third overwhelms its own line.
        Length runSize = scaling.Scaled(first.Size);
        Length size = marker.Scale is > 0 and not 1.0
            ? Length.FromEmu((long)Math.Round(runSize.Emu * marker.Scale))
            : runSize;

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
    private static string? Recoded(SlideMarker marker, FontReference? reference)
    {
        if (marker is not { IsSymbol: true, Text.Length: 1 }) return null;
        if (!SymbolFontRecode.IsRecodeable(marker.Typeface)) return null;

        // The face's own file is present, so its slots are drawable as they stand.
        if (reference is not null
            && !reference.IsSubstituted
            && !SymbolFontRecode.IsSubstituteFamily(reference.FamilyName))
        {
            return null;
        }

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
    /// </remarks>
    private static Length MarkerReach(
        SlideParagraph paragraph, Scaling scaling, SlideFonts fonts)
    {
        if (Shaped(paragraph, scaling, fonts) is not { Shaped: { } shaped } marked)
            return Length.Zero;

        // Never negative: the marker's right edge only ever pushes the first line further right,
        // and a hanging indent wider than the marker leaves the line where the file put it.
        Length reach = paragraph.FirstLineIndent + shaped.Width(marked.Size);
        return reach > Length.Zero ? reach : Length.Zero;
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
                run.Escapement.RiseOf(size), size));
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

        MeasuredParagraph measured = MeasuredParagraph.Measure(paragraph.Text, runs);
        ParagraphLayouter layouter = new(first);
        LaidOutParagraph laid = layouter.Layout(
            measured, format, width, paragraph.Language);

        List<PlacedLine> lines = [];
        bool firstLine = true;
        foreach (LineBox unaligned in laid.Lines)
        {
            LineBox box = Realigned(unaligned, format, alignAgainst, firstLine);
            firstLine = false;
            if (!fontIndependentLineSpacing)
            {
                // The face's own metrics — but its ascent and descent only, with no external
                // leading. EditEngine adds the leading only when IsAddExtLeading() is on, which is
                // a Writer compatibility flag and off in Impress
                // (editeng/source/editeng/impedit3.cxx:3131-3136). Liberation Sans declares a line
                // gap of 67/2048, so keeping it makes an 18 pt line 20.70 pt where LibreOffice
                // draws 20.15 — half a point per line, measured on the wrapping cell of
                // slide-table-grid.pptx, whose four reference baselines are 20.154 pt apart.
                (Length ascent, Length metric) =
                    FaceHeight(runs, styles, box.Line.Start, box.Line.VisibleEnd);

                Length faceHeight = metric > Length.Zero ? metric : box.Height;
                // Through LineSpacingRule.Apply, whose whole-twip arithmetic this branch wants:
                // it is the ODF path, whose line height is the face's own metrics rather than a
                // fraction of the em, and slides-features.odp's sixth outline baseline moves
                // 0.155 pt off LibreOffice's without it. The font-independent branch below is the
                // one that needs finer units — see Spacing.
                Length faceLine = paragraph.LineSpacing.Apply(faceHeight);

                lines.Add(Spaced(
                    new PlacedLine(
                        box,
                        ascent > Length.Zero ? ascent : box.Baseline,
                        faceLine,
                        faceHeight),
                    scaling));
                continue;
            }

            Length em = LargestSize(runs, styles, box.Line.Start, box.Line.VisibleEnd);

            // The rule itself: one em of ascent, 1.2 em of box, then whatever the paragraph's own
            // spacing does to it. A paragraph stating 150% gets 1.5 x 1.2 em, which is what
            // EditEngine's proportional spacing applies to the height it just computed.
            // Rounded to a whole hundredth of a millimetre, which is the unit EditEngine holds a
            // line height in: SetHeight takes a sal_uInt16 of the outliner's own map unit, and for
            // a draw object that unit is 1/100 mm. Keeping the exact EMU instead leaves a line
            // height the reference cannot represent, and the error accumulates down the block.
            Length natural = Length.FromMm100(
                (long)Math.Floor((em.Mm100 * LineHeightFactor) + 0.5));
            Length height = Spacing(paragraph.LineSpacing, natural);

            lines.Add(Spaced(
                new PlacedLine(
                    box,
                    Ascent(em, natural, height, paragraph.LineSpacing),
                    height,
                    natural),
                scaling));
        }

        Length total = Length.Zero;
        foreach (PlacedLine line in lines) total += line.Height;

        return new Block(
            paragraph, measured, styles, lines,
            total + paragraph.SpaceBefore + paragraph.SpaceAfter, scaling, format);
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
        List<FormattedRun> runs, List<RunStyle> styles, int start, int end)
    {
        Length ascent = Length.Zero;
        Length height = Length.Zero;

        for (int i = 0; i < runs.Count; i++)
        {
            FormattedRun run = runs[i];
            bool touches = run.Start < end && start < run.End;
            bool contains = start == end && run.Covers(start);
            if (!touches && !contains) continue;

            LineMetrics metrics = LineSpacing.Resolve(run.Face);
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
    /// </remarks>
    private static Length Rounded(Length metric)
        => Length.FromMm100((long)Math.Round((double)metric.Emu / Length.EmuPerMm100));

    /// <summary>
    /// Where the baseline sits inside a line whose height proportional spacing has changed.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One em under the plain font-independent rule, and <strong>not</strong> one em as soon as a
    /// paragraph states a proportional line spacing other than 100%: EditEngine moves the baseline
    /// with the box rather than leaving it where the font would put it. Below 100% the new ascent
    /// is <c>round(lineHeight × proportion × 0.8)</c> and the old one is kept if it is already
    /// smaller; above it the ascent grows by exactly the height the line gained
    /// (<c>editeng/source/editeng/impedit3.cxx:1553-1580</c>).
    /// </para>
    /// <para>
    /// The four-fifths is not derivable from anything; it is a constant EditEngine took from
    /// Writer's line formatter and it decides the first baseline of every shape in a deck that
    /// tightens its spacing. Measured on <c>ppt-features.ppt</c>, whose paragraphs all state 93%:
    /// the reference puts the 40 pt title's baseline 35.7 pt below the text top, where one em
    /// would be 40 and <c>1.2 × 40 × 0.93 × 0.8</c> is 35.71.
    /// </para>
    /// <para>
    /// Guarded on the proportion so that it costs nothing for the two families whose corpus decks
    /// state none: <see cref="LineSpacingRule.SingleSpaced"/> is exactly 100% and takes neither
    /// branch.
    /// </para>
    /// </remarks>
    private static Length Ascent(Length em, Length natural, Length height, LineSpacingRule spacing)
    {
        if (spacing.Mode != LineSpacingMode.Proportional || spacing.Proportion == 1.0) return em;

        if (spacing.Proportion < 1.0)
        {
            Length reduced = Length.FromEmu(
                (long)Math.Round(natural.Emu * spacing.Proportion * ShortSpacingAscent));
            return Length.Min(em, reduced);
        }

        return em + (height - natural);
    }

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
    /// <see cref="LineSpacingRule.Apply"/>, and the reason is the unit rather than the arithmetic.
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
    /// </remarks>
    private static Length Spacing(LineSpacingRule rule, Length natural)
        => rule.Mode == LineSpacingMode.Proportional && rule.Proportion is <= 0 or 1.0
            ? natural
            : rule.Apply(natural);

    /// <summary>
    /// Applies the fit's spacing scale to a line, which moves its baseline as well as its box.
    /// </summary>
    /// <remarks>
    /// EditEngine's <c>SvxInterLineSpaceRule::Off</c> branch — the one a paragraph that states no
    /// line spacing takes — turns the scale into a proportional spacing and applies it to both:
    /// the height is multiplied and the ascent is <em>capped</em> at the text height times the
    /// same factor, never raised (<c>editeng/source/editeng/impedit3.cxx:1584-1600</c>). Capping
    /// rather than assigning is what keeps a line whose ascent was already short where it was.
    /// </remarks>
    private static PlacedLine Spaced(PlacedLine line, Scaling scaling)
    {
        if (scaling.Spacing is <= 0 or >= 1.0) return line;

        Length ascent = Length.FromEmu(
            (long)Math.Round(line.TextHeight.Emu * scaling.Spacing));
        Length height = Length.FromEmu((long)Math.Round(line.Height.Emu * scaling.Spacing));

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
            ShapedText shaped = TextShaper.Default.Shape(run.Face, text, run.Shaping);
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

            placed.Add(new PlacedGlyphRun(
                glyphs,
                block.ColourAt(run.Start),
                Rules(block.DecorationAt(run.Start), run.Face, run.EmSize, pen, pitch, advance)));

            // The pen carries across the runs of a line, so the second run starts where the first
            // ended rather than back at the margin.
            pen += advance;
        }
    }

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
        FontVerticalMetrics metrics =
            LineSpacing.ResolveDecorations(face, LineSpacing.Resolve(face));

        Length Scaled(int designUnits) => size * ((double)designUnits / unitsPerEm);

        List<DocRect> rules = [];

        if (decoration.Underline)
        {
            // The face records the underline's offset as negative below the baseline.
            Length thickness = Scaled(metrics.UnderlineThickness);
            if (thickness > Length.Zero)
            {
                rules.Add(new DocRect(
                    left, baseline - Scaled(metrics.UnderlinePosition), width, thickness));
            }
        }

        if (decoration.Strikethrough)
        {
            Length thickness = Scaled(metrics.StrikeoutThickness);
            if (thickness > Length.Zero)
            {
                rules.Add(new DocRect(
                    left, baseline - Scaled(metrics.StrikeoutPosition), width, thickness));
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
    private readonly record struct RunStyle(
        Colour Colour,
        FontReference? Font,
        OpenTypeFace? Face,
        bool IsUnderlined = false,
        bool IsStruckThrough = false,
        Length Rise = default,
        Length NominalSize = default);

    /// <summary>One paragraph, measured and broken.</summary>
    private sealed record Block(
        SlideParagraph Paragraph,
        MeasuredParagraph Measured,
        IReadOnlyList<RunStyle> Styles,
        IReadOnlyList<PlacedLine> Lines,
        Length Height,
        Scaling Scaling,
        ParagraphFormat Format)
    {
        /// <summary>The space above the paragraph.</summary>
        /// <remarks>
        /// <strong>Not touched by the fit's spacing scale, which reaches only the lines.</strong>
        /// The reference does put a paragraph's own space through <c>scaleYSpacingValue</c>
        /// (<c>ImpEditEngine::CalcHeight</c>, <c>editeng/source/editeng/impedit2.cxx:4406,4412</c>
        /// in 24.2) — but that helper returns its argument unchanged unless
        /// <c>maStatus.DoStretch()</c> is set (<c>impedit.hxx:748-754</c>), whereas the line
        /// heights are scaled by <c>mfSpacingScaleY</c> directly and unconditionally
        /// (<c>impedit3.cxx:1493-1528</c>). Scaling the paragraph space as well was measured
        /// wrong: it shrank the sixth slide of
        /// <c>slides/batch-002/ppt/gfopportunitiesforlinkagespres_2010_en.ppt</c> a step below
        /// the reference, where leaving it alone reproduces the reference's text width to
        /// 0.2 per cent.
        /// </remarks>
        public Length SpaceBefore => Paragraph.SpaceBefore;

        /// <summary>The space below the paragraph, likewise unscaled.</summary>
        public Length SpaceAfter => Paragraph.SpaceAfter;

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
        /// when it does not substitute. Nothing in this library turns glyph fallback on today, so
        /// the guard costs a pointer comparison and buys the invariant outright.
        /// </para>
        /// </remarks>
        /// <param name="index">A character the sub-run covers.</param>
        /// <param name="face">The face the sub-run will actually be shaped and drawn in.</param>
        public FontReference? FontFor(int index, OpenTypeFace face)
        {
            RunStyle style = StyleAt(index);
            return ReferenceEquals(style.Face, face) ? style.Font : null;
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
    private readonly record struct PlacedLine(
        LineBox Box, Length Ascent, Length Height, Length TextHeight);
}

using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Itemisation;
using XRay.Office.Text.Shaping;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>One stretch of a shaped cell run set in a single face, size and colour.</summary>
/// <remarks>
/// A plain cell has exactly one of these and a rich one has several. The offset is measured from
/// the run's own start, so placing the run is one addition per segment rather than a re-shape.
/// </remarks>
/// <param name="Glyphs">Its glyphs, positioned from the segment's own origin.</param>
/// <param name="Clusters">The cluster map, as indices into <paramref name="Text"/>.</param>
/// <param name="Face">The face it is set in, for the metrics its line height needs.</param>
/// <param name="Size">The em size.</param>
/// <param name="Colour">Its own colour, or null to take the cell's.</param>
/// <param name="Text">The characters it covers.</param>
/// <param name="Offset">How far along the run it starts.</param>
/// <param name="Width">How far its pen travels.</param>
/// <param name="Underline">The line under this segment alone.</param>
/// <param name="StruckThrough">Whether this segment alone is struck through.</param>
/// <remarks>
/// <see cref="Underline"/> and <see cref="StruckThrough"/> are per segment because they are the
/// two properties a rich cell can change part-way through a line and the only ones a rule has to
/// be placed for. <see cref="Offset"/> and <see cref="Width"/> already say where that rule goes,
/// which is what makes this cheap: the geometry was always here, and the decorations were being
/// taken from the cell instead.
/// </remarks>
internal sealed record SheetTextSegment(
    List<PositionedGlyph> Glyphs,
    List<int> Clusters,
    SheetFace Face,
    Length Size,
    Colour? Colour,
    string Text,
    Length Offset,
    Length Width,
    SheetUnderline Underline = SheetUnderline.None,
    bool StruckThrough = false);

/// <summary>A shaped piece of cell text, positioned once it is placed.</summary>
/// <remarks>
/// <para>
/// Shaped without an origin and given one later, because a cell's horizontal position depends on
/// the run's own width — a number is drawn against the right edge of its column, so where it
/// starts cannot be known until it has been measured.
/// </para>
/// <para>
/// A list of segments rather than one, because a cell's text may change format part-way through.
/// Concatenating separately shaped segments is exactly right here and would not be in a word
/// processor: cell text is not kerned (<see cref="SheetText.NoKerning"/>), so there is no pair
/// adjustment to lose across a format boundary, and Calc splits a cell's text into EditEngine
/// portions on the same boundaries anyway. Measured on Liberation Sans: neither its <c>GPOS</c>
/// nor its legacy <c>kern</c> table has a pair for any boundary in the corpus document, so
/// shaping the portions together would give the same widths.
/// </para>
/// </remarks>
internal sealed class SheetTextRun
{
    private readonly List<SheetTextSegment> _segments;

    internal SheetTextRun(List<SheetTextSegment> segments, Length width)
    {
        _segments = segments;
        Width = width;
    }

    /// <summary>How far the run's pen travels.</summary>
    public Length Width { get; }

    /// <summary>
    /// How wide the run is once the blanks it ends with are taken off.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A wrapped line keeps the spaces it broke after — see <c>SheetTextLayout.Wrap</c>, which
    /// takes a line to its <c>End</c> rather than to its <c>VisibleEnd</c> because Calc's own
    /// output draws them. Centring against that width is what those spaces must <em>not</em> be
    /// allowed to do: they hang off the right, so half of them is subtracted from the left and the
    /// line starts before the cell does.
    /// </para>
    /// <para>
    /// EditEngine names the same quantity — <c>tdf#168135</c>, "exclude trailing spaces from
    /// centering width" (<c>ImpEditEngine::CreateLines</c>,
    /// <c>editeng/source/editeng/impedit3.cxx:1646-1682</c>). It is only <em>one</em> of the two
    /// bounds a centred line is placed by here, because our wrapped lines can overflow the width
    /// they were broken against where EditEngine's cannot; see
    /// <c>SheetTextLayout.AlignedWidth</c>, which is the sole caller and holds the arithmetic and
    /// the measurements.
    /// </para>
    /// <para>
    /// Only U+0020 counts, which is what EditEngine tests for. A no-break space is not a blank
    /// here and neither is a tab; a cell holding either takes the plain width.
    /// </para>
    /// </remarks>
    public Length WithoutTrailingBlanks
    {
        get
        {
            Length width = Width;

            for (int index = _segments.Count - 1; index >= 0; index--)
            {
                SheetTextSegment segment = _segments[index];

                int trimmed = segment.Text.Length;
                while (trimmed > 0 && segment.Text[trimmed - 1] == ' ') trimmed--;

                if (trimmed == segment.Text.Length) return width;

                if (trimmed == 0)
                {
                    width -= segment.Width;
                    continue;
                }

                // Part of the segment survives, so only the glyphs the trimmed characters own
                // come off. The cluster map is what says which those are — a segment is not one
                // glyph per character in general, and a blank is not always one glyph.
                for (int glyph = 0; glyph < segment.Glyphs.Count; glyph++)
                {
                    if (segment.Clusters[glyph] >= trimmed) width -= segment.Glyphs[glyph].Advance;
                }

                return width;
            }

            return width;
        }
    }

    /// <summary>The pieces the run is made of, in reading order.</summary>
    public IReadOnlyList<SheetTextSegment> Segments => _segments;

    /// <summary>The em size the run is set at, which is its first segment's.</summary>
    public Length Size => _segments.Count > 0 ? _segments[0].Size : Length.Zero;

    /// <summary>
    /// The distance from the baseline to the top of the tallest segment.
    /// </summary>
    /// <remarks>
    /// The maximum rather than the cell's own, because a line is as tall as what is on it: a
    /// sixteen-point word inside a ten-point cell pushes the whole line's baseline down. That is
    /// EditEngine's rule as much as Writer's, and it is why the metrics travel with the segment.
    /// </remarks>
    public Length Ascent => Extreme(segment => segment.Face.AscentAt(segment.Size));

    /// <summary>The distance from the baseline to the bottom of the deepest segment.</summary>
    public Length Descent => Extreme(segment => segment.Face.DescentAt(segment.Size));

    /// <summary>How tall Calc considers this run's line: ascent plus descent, with no line gap.</summary>
    public Length LineHeight => Ascent + Descent;

    /// <summary>The run placed at a baseline origin, one glyph run per segment.</summary>
    /// <param name="origin">Where the run's pen starts.</param>
    public IEnumerable<(GlyphRun Run, Colour? Colour)> At(DocPoint origin)
    {
        foreach (SheetTextSegment segment in _segments)
        {
            yield return (
                new GlyphRun
                {
                    Font = segment.Face.Reference,
                    FontSize = segment.Size,
                    Origin = new DocPoint(origin.X + segment.Offset, origin.Y),
                    Glyphs = segment.Glyphs,
                    Text = segment.Text,
                    ClusterMap = segment.Clusters,
                },
                segment.Colour);
        }
    }

    private Length Extreme(Func<SheetTextSegment, Length> of)
    {
        Length largest = Length.Zero;
        foreach (SheetTextSegment segment in _segments)
        {
            Length value = of(segment);
            if (value > largest) largest = value;
        }
        return largest;
    }
}

/// <summary>
/// Shapes cell text in a resolved face.
/// </summary>
/// <remarks>
/// Shaping only — where the run goes is <see cref="SheetTextLayout"/>'s business. The two are
/// kept apart because measuring is what pagination needs (<see cref="SheetTextOverflow"/> widens
/// a sheet's print area by measuring strings) and placing is what drawing needs, and the first
/// must not drag in the second.
/// </remarks>
internal static class SheetText
{
    /// <summary>
    /// How a cell's text is shaped: kerned pairs off.
    /// </summary>
    /// <remarks>
    /// Calc says so in as many words — "There is no cell attribute for kerning, default is kerning
    /// OFF, all kerning is stored at an EditText object that is drawn using EditEngine"
    /// (<c>ScDrawStringsVars::SetPattern</c>, <c>sc/source/ui/view/output2.cxx:405-409</c>) — and
    /// it is measurable rather than theoretical: HarfBuzz kerns <c>1.2E+11</c> by 152 design
    /// units, which puts a right-aligned cell 0.74 pt away from where LibreOffice puts it. That
    /// is a difference no font metric or margin would explain, so it is worth having the reason
    /// written down beside the flag.
    /// </remarks>
    public static readonly ShapingOptions NoKerning = new(DisableKerning: true);

    /// <summary>Shapes a string, or null when there is no face to shape it with.</summary>
    /// <param name="text">The text to shape.</param>
    /// <param name="face">The face to shape it in.</param>
    /// <param name="size">The em size to scale the advances to.</param>
    /// <param name="colour">Its own colour, or null to take the cell's.</param>
    public static SheetTextRun? Shape(string text, SheetFace? face, Length size, Colour? colour = null)
    {
        if (text.Length == 0 || face is not { } resolved) return null;

        List<SheetTextSegment> segments = [];
        Length offset = Length.Zero;

        // No decorations here: a plain cell's underline is the cell's own and is drawn across the
        // whole line by SheetTextLayout, which is both correct and cheaper than carrying it on
        // every segment. Only a rich cell needs the per-segment answer.
        Append(segments, text, resolved, size, colour, SheetUnderline.None, false, ref offset);

        return segments.Count == 0 ? null : new SheetTextRun(segments, offset);
    }

    /// <summary>
    /// The same run with a line's spare width shared out among its blanks.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ImpEditEngine::ImpAdjustBlocks</c>
    /// (<c>editeng/source/editeng/impedit3.cxx:2306-2420</c>), which is what a cell whose
    /// <c>horizontal</c> alignment is <c>justify</c> or <c>distributed</c> reaches: Calc maps both
    /// to <c>SvxAdjust::Block</c> and the engine calls this for every line with room left over
    /// (<c>:1694-1701</c>). The space is shared among the line's <em>blanks</em> — one share each,
    /// with the first few gaps taking a unit of the remainder — and not among its characters.
    /// </para>
    /// <para>
    /// Three of the routine's rules are reproduced and one is not. A blank that is the line's own
    /// last character is not a gap ("If the last character is a blank, it is rejected", <c>:2361</c>)
    /// and its width is given back to the space being shared, which is why the measure below is
    /// <see cref="SheetTextRun.WithoutTrailingBlanks"/>; the correction loop then skips the line's
    /// final position again (<c>:2400</c>); and a line with no gap at all is left alone
    /// (<c>:2356</c>). What is <strong>not</strong> reproduced is the CJK arm, which adds a gap at
    /// every boundary between an Asian character and its neighbour (<c>:2334-2343</c>), and the
    /// Kashida arm for Arabic (<c>:2350</c>). A justified cell of Asian text therefore keeps its
    /// Latin spacing here; nothing in the corpus has one.
    /// </para>
    /// <para>
    /// The trailing blanks themselves keep their advances rather than being collapsed. Calc zeroes
    /// the last one so that the line's own reported width lands on the paper's, and here the line
    /// is placed from the left in every case a justified cell can take
    /// (<c>SheetTextLayout.Resolve</c>), so what a trailing blank does is hang past the right edge
    /// exactly as it already did.
    /// </para>
    /// </remarks>
    /// <param name="run">The line to stretch.</param>
    /// <param name="available">The paper width the line was broken against.</param>
    public static SheetTextRun Justified(SheetTextRun run, Length available)
    {
        ArgumentNullException.ThrowIfNull(run);

        Length extra = available - run.WithoutTrailingBlanks;
        if (extra <= Length.Zero) return run;

        // Where the run's visible text stops: a blank past that point is the line's trailing
        // space, which is not a gap.
        int lastVisible = -1;
        int seen = 0;

        foreach (SheetTextSegment segment in run.Segments)
        {
            for (int glyph = 0; glyph < segment.Glyphs.Count; glyph++, seen++)
            {
                if (!IsBlank(segment, glyph)) lastVisible = seen;
            }
        }

        int gaps = 0;
        seen = 0;

        foreach (SheetTextSegment segment in run.Segments)
        {
            for (int glyph = 0; glyph < segment.Glyphs.Count; glyph++, seen++)
            {
                if (seen < lastVisible && IsBlank(segment, glyph)) gaps++;
            }
        }

        if (gaps == 0) return run;

        long share = extra.Emu / gaps;
        long remainder = extra.Emu - (share * gaps);

        List<SheetTextSegment> widened = new(run.Segments.Count);
        Length offset = Length.Zero;
        int taken = 0;
        seen = 0;

        foreach (SheetTextSegment segment in run.Segments)
        {
            List<PositionedGlyph> glyphs = new(segment.Glyphs.Count);

            // A glyph's own offset is where it is drawn — the advances are bookkeeping, and the
            // PDF backend positions from `Offset` — so everything after a widened gap has to move
            // with it. The offsets are relative to the segment, so only the extra added inside
            // this segment is carried here; what earlier segments added is already in `offset`.
            Length local = Length.Zero;

            for (int glyph = 0; glyph < segment.Glyphs.Count; glyph++, seen++)
            {
                PositionedGlyph positioned = segment.Glyphs[glyph] with
                {
                    Offset = new DocPoint(
                        segment.Glyphs[glyph].Offset.X + local,
                        segment.Glyphs[glyph].Offset.Y),
                };

                if (seen < lastVisible && IsBlank(segment, glyph))
                {
                    Length add = Length.FromEmu(share + (taken < remainder ? 1 : 0));
                    taken++;
                    positioned = positioned with { Advance = positioned.Advance + add };
                    local += add;
                }

                glyphs.Add(positioned);
            }

            Length width = segment.Width + local;
            widened.Add(segment with { Glyphs = glyphs, Offset = offset, Width = width });
            offset += width;
        }

        return new SheetTextRun(widened, offset);
    }

    /// <summary>Whether one glyph of a segment stands for a space character.</summary>
    private static bool IsBlank(SheetTextSegment segment, int glyph)
    {
        int cluster = glyph < segment.Clusters.Count ? segment.Clusters[glyph] : -1;
        return cluster >= 0 && cluster < segment.Text.Length && segment.Text[cluster] == ' ';
    }

    /// <summary>
    /// Shapes a range of a rich cell's text, one segment per portion it crosses.
    /// </summary>
    /// <remarks>
    /// The range is a range of the <em>cell's</em> text rather than a substring handed in, because
    /// the portions index into that text: shortening a clipped rich cell or taking one wrapped line
    /// of it has to keep the formatting lined up with the characters, and a substring loses the
    /// offset that says how.
    /// </remarks>
    /// <param name="text">The cell's whole text.</param>
    /// <param name="portions">Its portions, normalised to cover it exactly once.</param>
    /// <param name="scale">The print zoom, which multiplies every em size.</param>
    /// <param name="start">The first character to shape.</param>
    /// <param name="end">One past the last.</param>
    /// <param name="percent">A further shrink, as an integer percentage; 100 for none.</param>
    public static SheetTextRun? ShapeRich(
        string text,
        IReadOnlyList<SheetTextPortion> portions,
        double scale,
        int start,
        int end,
        long percent = 100)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(portions);

        int first = Math.Max(start, 0);
        int last = Math.Min(end, text.Length);
        if (last <= first) return null;

        List<SheetTextSegment> segments = [];
        Length offset = Length.Zero;

        foreach (SheetTextPortion portion in portions)
        {
            int from = Math.Max(portion.Start, first);
            int to = Math.Min(portion.End, last);
            if (to <= from) continue;

            // Neither of these two falls back to nothing. Dropping a portion drops its
            // *characters*, which is the worst failure this path has: the text is in the file and
            // in the extraction, and only the page loses it — silently, with no diagnostic and no
            // gap to notice. A portion whose face will not resolve is drawn in the sheet's own
            // default face and one stating no usable size at the default size, both of which are
            // wrong in a way a reader can see and argue with. What remains below is a scale of
            // nought or less, which is degenerate for the whole page rather than for one portion,
            // and takes every portion with it as the single-face path already does.
            SheetFace? resolved = SheetFonts.For(portion.Format) ?? DefaultFace;
            if (resolved is not { } face) continue;

            Length size = SizeOf(SizeStatedBy(portion.Format), scale, percent);
            if (size <= Length.Zero) continue;

            Append(
                segments, text[from..to], face, size, portion.Format.Colour,
                portion.Format.Underline, portion.Format.IsStruckThrough, ref offset);
        }

        return segments.Count == 0 ? null : new SheetTextRun(segments, offset);
    }

    /// <summary>
    /// Shapes one stretch, splitting it again wherever its own face has no glyph.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A cell's face is chosen from a family name and coverage is a property of a character, so a
    /// workbook whose cells name a Latin face and hold Japanese would otherwise draw a row of
    /// missing-glyph boxes — at that face's <c>.notdef</c> advance, which is nothing like the width
    /// of the ideographs the reference renderer lays out after its own fallback. LibreOffice does
    /// the same split after shaping rather than before (<c>OutputDevice::ImplGlyphFallbackLayout</c>).
    /// </para>
    /// <para>
    /// A stretch whose face covers it is one segment and reaches the shaper in the identical call it
    /// did before this existed, which matters: shaping is contextual, so a run split it did not need
    /// measures very slightly differently.
    /// </para>
    /// </remarks>
    private static void Append(
        List<SheetTextSegment> segments,
        string text,
        SheetFace face,
        Length size,
        Colour? colour,
        SheetUnderline underline,
        bool struckThrough,
        ref Length offset)
    {
        // What is actually drawn, which is not always what the cell holds: a legacy pi face that
        // is not installed has its slots recoded into OpenSymbol's, and that has to happen before
        // anything is itemised or measured — an OpenSymbol circled-times and a DejaVu Sans "Ä" are
        // not the same picture and not the same width. Every index survives; see SheetSymbolText.
        string drawn = SheetSymbolText.Recode(text, face);

        List<FaceRun> runs = FontItemiser.Split(
            drawn, 0, drawn.Length, face.Face, SheetFonts.Fallback);

        foreach (FaceRun run in runs)
        {
            if (run.Length <= 0) continue;

            // Nothing to name the face with is nothing to embed it through, so a fallback the
            // resolver cannot reference falls back again to the primary face's missing-glyph box —
            // which is what happened before this existed — rather than to a font the PDF would
            // announce without carrying.
            // The request travels with the substitution: a fallback face is not the face the cell
            // asked for, so its own IsItalic says nothing about whether the cell wanted a lean.
            // Both states count — a real italic face, and an upright one already carrying a
            // synthetic oblique. See IGlyphFallbackResolver.ReferenceFor(OpenTypeFace, bool).
            bool italic = face.Face.IsItalic || face.Reference.SyntheticOblique;

            SheetFace drawnFace =
                run.IsFallback && SheetFonts.ForFallback(run.Face, italic) is { } resolved
                    ? resolved
                    : face;

            segments.Add(Segment(
                drawn.Substring(run.Start, run.Length),
                text.Substring(run.Start, run.Length),
                drawnFace, size, colour, offset,
                underline, struckThrough, out Length width));
            offset += width;
        }
    }

    /// <summary>
    /// The em size a portion asks for, which is the default when it asks for nothing usable.
    /// </summary>
    /// <remarks>
    /// A run stating <c>sz="0"</c> — or a reader that could not make a size out of what it stated
    /// — must not be drawn at nothing. <see cref="SheetDeviceUnits.SnapFontSize(Length, double)"/>
    /// floors a size at one device pixel rather than at zero, so such a portion used to be drawn
    /// at 0.113 pt: present in the PDF's text layer and invisible on the page, which is a worse
    /// answer than either drawing it properly or leaving it out. The default is what the device
    /// would select for a zero height anyway.
    /// </remarks>
    public static Length SizeStatedBy(SheetCellFormat format)
        => format.FontSize > Length.Zero ? format.FontSize : SheetCellFormat.Default.FontSize;

    /// <summary>
    /// A portion's em size, snapped to the drawing device's unit before anything scales it.
    /// </summary>
    /// <remarks>
    /// The same two roundings a plain cell's size goes through, in the same order: the shrink
    /// percentage is applied in whole twips because Calc's own shrink loop works there, and the
    /// result is snapped to a hundredth of a millimetre because that is the unit the device draws
    /// in. See <see cref="SheetDeviceUnits"/>.
    /// </remarks>
    public static Length SizeOf(Length stated, double scale, long percent)
    {
        Length scaled = SheetDeviceUnits.SnapFontSize(stated, scale);

        return percent == 100
            ? scaled
            : SheetDeviceUnits.SnapFontSize(Length.FromTwips(scaled.Twips * percent / 100));
    }

    /// <summary>Shapes one stretch of one face.</summary>
    /// <remarks>
    /// <paramref name="drawn"/> and <paramref name="text"/> differ only for a recoded pi face, and
    /// then only code point for code point — the shaper is given the slots OpenSymbol actually
    /// holds while the segment keeps the characters the document does, so the cluster map still
    /// indexes the text and the PDF's <c>ToUnicode</c> carries what the reference's carries.
    /// See <see cref="SheetSymbolText"/>.
    /// </remarks>
    private static SheetTextSegment Segment(
        string drawn, string text, SheetFace face, Length size, Colour? colour, Length offset,
        SheetUnderline underline, bool struckThrough, out Length width)
    {
        ShapedText shaped = TextShaper.Default.Shape(face.Face, drawn, NoKerning);

        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);
        Length pen = Length.Zero;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, size);
            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(
                    pen + shaped.Scale(glyph.OffsetX, size),
                    -shaped.Scale(glyph.OffsetY, size)),
                advance));
            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        width = pen;
        return new SheetTextSegment(
            glyphs, clusters, face, size, colour, text, offset, pen, underline, struckThrough);
    }

    /// <summary>How wide a string is in a face, without keeping the run.</summary>
    /// <param name="text">The text to measure.</param>
    /// <param name="face">The face; null measures as nothing.</param>
    /// <param name="size">The em size.</param>
    public static Length Measure(string text, SheetFace? face, Length size)
        => Shape(text, face, size)?.Width ?? Length.Zero;

    /// <summary>
    /// How wide a string is on a device that can only advance by whole pixels.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>OutputDevice::GetTextWidth</c> returns the sum of the layout's glyph advances, and a
    /// glyph's advance on a raster device is a whole number of pixels. So the width of a string is
    /// the <em>sum of the rounded advances</em> and not the rounded sum, and the two differ by more
    /// than a pixel as soon as the string is long: at twelve point on a 96 dpi device a
    /// seventy-two character run measures 516 pixels one way and 523 the other, which is 1.4%.
    /// </para>
    /// <para>
    /// Fitted against LibreOffice 24.2.7.2's own row heights for turned cells — six string lengths
    /// at ten, eleven and twelve point, read out of its flat-ODF round trip of
    /// <c>sheet-row-height-rotated.fods</c>. <strong>All eighteen are reproduced exactly by this
    /// and fourteen by rounding the total</strong>, which is what says the rounding is per glyph.
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, round 58 — the same fixture round-tripped through the
    /// installed 26.2.4.2 gives <strong>216 of 216 row heights unchanged</strong>, including all
    /// 72 quarter-turn heights, which are the ones that measure <c>GetTextWidth</c> directly: a
    /// cell at exactly 90° or 270° takes <c>ScPatternAttr::GetCellOrientation</c>'s branch, where
    /// the row height simply <em>is</em> the string's width, so nothing else stands between the
    /// rule and the number. The discriminator is built into the fixture rather than argued: four
    /// of its eighteen distinct widths — the twelve-point ones — differ between rounding per
    /// glyph and rounding the total, by up to 1.4%, and none of them moved.
    /// <c>probes/sheets-r58/audit_rotatedwidth.py</c>. This was the last unverified site in
    /// <c>XRay.Office.Spreadsheets</c>.]
    /// </para>
    /// <para>
    /// Only <see cref="SheetOptimalRowHeights"/> wants this: it is the one place a length measured
    /// on Calc's measuring device has to come back as that device's own integer. Drawing measures
    /// on the page, where nothing is quantised.
    /// </para>
    /// </remarks>
    /// <param name="text">The text to measure.</param>
    /// <param name="face">The face; null measures as nothing.</param>
    /// <param name="size">The em size, already snapped to the device's own pixel grid.</param>
    /// <param name="twipsPerPixel">The device's pixel, in twips.</param>
    public static long MeasurePixels(string text, SheetFace? face, Length size, int twipsPerPixel)
    {
        if (twipsPerPixel <= 0 || Shape(text, face, size) is not { } run) return 0;

        long pixels = 0;
        foreach (SheetTextSegment segment in run.Segments)
        {
            foreach (PositionedGlyph glyph in segment.Glyphs)
            {
                pixels += (long)Math.Round(
                    glyph.Advance.Twips / (double)twipsPerPixel, MidpointRounding.AwayFromZero);
            }
        }

        return pixels;
    }

    /// <summary>The face a sheet's cells fall back to, resolved once.</summary>
    public static SheetFace? DefaultFace => SheetFonts.For(SheetCellFormat.Default);
}

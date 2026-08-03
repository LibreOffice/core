using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;

namespace Paperless.Text.Layout;

/// <summary>
/// One stretch of a laid-out paragraph's line, positioned.
/// </summary>
/// <remarks>
/// A stretch rather than a line, because a line beside a floating frame that touches neither margin has
/// text on both sides of it — one baseline, two stretches. Every stretch of a line carries the
/// <em>line's</em> geometry: the same <see cref="Top"/>, <see cref="Height"/>, <see cref="Baseline"/> and
/// <see cref="SpaceAbove"/>, differing only in which characters it holds and where it starts across the
/// page. All but the last have <see cref="SharesLineWithNext"/> set, which is what tells whoever stacks
/// boxes to advance once per line rather than once per box — and what stops a page break from falling
/// between two stretches of one line.
/// </remarks>
/// <param name="Line">Which characters the stretch holds, and how wide its visible text is.</param>
/// <param name="Left">
/// Where the line's text starts, measured from the text area's start edge — so the paragraph's indents
/// and the line's alignment are already in it.
/// </param>
/// <param name="Top">The top of the line's box, measured from the paragraph's top.</param>
/// <param name="Height">The box's height: the distance from this line's top to the next line's.</param>
/// <param name="Baseline">
/// Where the text sits within the box, measured from <paramref name="Top"/>. Not derivable from the
/// height, because the space line spacing adds sits above the text rather than being shared around it.
/// </param>
/// <param name="SpaceAbove">
/// How much of the box is empty space above the text. Kept separately because whoever fills pages needs
/// it twice: the first line on a page draws its text at the top margin rather than a line's worth below
/// it, and the last line on a page is allowed to hang this much past the bottom. Recomputing it there
/// would mean knowing the spacing rule, which is the paragraph's business rather than the page's.
/// </param>
/// <param name="SpaceAdd">
/// How much is added to every blank on this line to justify it, or zero when it is not justified.
/// Justification stretches a line rather than shifting it, so unlike the other alignments it cannot be
/// folded into <paramref name="Left"/> — it changes where each word after the first blank sits.
/// </param>
/// <param name="SharesLineWithNext">
/// True when the box after this one is a further stretch of the <em>same</em> line rather than the next
/// line down. False for every box of a paragraph that flows round nothing, which is nearly all of them.
/// </param>
public readonly record struct LineBox(
    TextLine Line,
    Length Left,
    Length Top,
    Length Height,
    Length Baseline,
    Length SpaceAbove,
    Length SpaceAdd = default,
    bool SharesLineWithNext = false)
{
    /// <summary>The same line with the space above its text removed, box and all.</summary>
    /// <remarks>
    /// <para>
    /// What the first line on a page becomes. Writer treats the leading proportional spacing adds as part
    /// of the <em>paragraph's</em> upper space rather than as part of the line, and drops it at the top of
    /// a text frame — so the first line's text sits on the top margin and everything below it moves up by
    /// the same amount. Both the height and the baseline shrink, which is what keeps the pitch between
    /// the first line and the second equal to the pitch everywhere else.
    /// </para>
    /// <para>
    /// The visible consequence is a page that holds one line more: a 200%-spaced A4 page fits
    /// twenty-five lines this way and twenty-four with the leading kept.
    /// </para>
    /// </remarks>
    public LineBox WithoutSpaceAbove()
        => this with
        {
            Height = Height - SpaceAbove,
            Baseline = Baseline - SpaceAbove,
            SpaceAbove = Length.Zero,
        };

    /// <summary>The absolute baseline, given where the paragraph starts.</summary>
    public Length BaselineFrom(Length paragraphTop) => paragraphTop + Top + Baseline;

    /// <summary>The width of the line's visible text.</summary>
    public Length Width => Line.Width;
}

/// <summary>
/// A paragraph after layout: its lines, and how much vertical room it took.
/// </summary>
/// <param name="Lines">The lines, in order, positioned relative to the paragraph's top.</param>
/// <param name="SpaceBefore">The space above the first line, already reduced by any collapsing.</param>
/// <param name="SpaceAfter">The space below the last line.</param>
public sealed record LaidOutParagraph(
    IReadOnlyList<LineBox> Lines,
    Length SpaceBefore,
    Length SpaceAfter)
{
    /// <summary>The height of the lines alone, without the space around them.</summary>
    public Length TextHeight =>
        Lines.Count == 0 ? Length.Zero : Lines[^1].Top + Lines[^1].Height;

    /// <summary>The whole paragraph's height, the space above and below included.</summary>
    public Length Height => SpaceBefore + TextHeight + SpaceAfter;
}

/// <summary>
/// Lays a paragraph out: breaks it into lines, spaces them, and places each one horizontally.
/// </summary>
/// <remarks>
/// <para>
/// The step between line breaking and pagination. Breaking decides <em>where</em> the lines divide;
/// this decides how tall each one is, where its baseline sits inside it, and where it starts across the
/// page — which together are what pagination adds up and what drawing needs.
/// </para>
/// <para>
/// The decision that matters, because it is invisible to a pitch comparison and moves every page break:
/// the space line spacing adds sits <em>above</em> the text, not below it and not shared around it.
/// Writer's drawing ascent is <c>ascent + realHeight - height</c>, so a double-spaced line is a
/// single-spaced line with a blank line stacked on top. Putting the space below gives identical
/// baseline pitches and a different paragraph height — which is why this was found by a pagination
/// test rather than by the four line-spacing tests that pass either way.
/// </para>
/// <para>
/// Exact spacing does not use the font's ascent at all: Writer puts the baseline at four fifths of the
/// box, so a fixed-height row does not shift when its font changes.
/// </para>
/// </remarks>
public sealed class ParagraphLayouter
{
    private readonly TextMeasurer _measurer;
    private readonly LineFiller _filler;
    private readonly LineMetrics _metrics;

    /// <summary>Creates a layouter over a face.</summary>
    /// <param name="face">The face the paragraph is set in.</param>
    /// <param name="breaker">The break iterator, or null for the default.</param>
    /// <param name="grid">
    /// The device grid the face's vertical metrics are rounded through, or null to scale them exactly.
    /// See <see cref="MetricGrid"/>.
    /// </param>
    public ParagraphLayouter(OpenTypeFace face, ILineBreaker? breaker = null, MetricGrid? grid = null)
    {
        ArgumentNullException.ThrowIfNull(face);
        _measurer = new TextMeasurer(face);
        _filler = new LineFiller(_measurer, breaker);
        _metrics = LineSpacing.Resolve(face, grid);
    }

    /// <summary>The line metrics the face resolved to, and which set they came from.</summary>
    public LineMetrics Metrics => _metrics;

    /// <summary>The measurer, for callers that need to measure the same text again.</summary>
    public TextMeasurer Measurer => _measurer;

    /// <summary>
    /// Lays a paragraph out in a given width.
    /// </summary>
    /// <param name="text">The paragraph's text, without its terminating mark.</param>
    /// <param name="format">Its resolved layout properties.</param>
    /// <param name="emSize">The em size its text is set at.</param>
    /// <param name="textAreaWidth">The width available before the paragraph's own indents.</param>
    /// <param name="language">A BCP 47 tag, for the language-specific break rules.</param>
    /// <param name="follows">
    /// The format of the paragraph immediately above, when there is one. Used only for contextual
    /// spacing, which suppresses the gap between two paragraphs of the same shape.
    /// </param>
    /// <param name="options">How to shape; the default is what Writer does.</param>
    /// <param name="obstacles">
    /// The floating frames the text has to flow around, or null when there are none — which is nearly
    /// always, and is the path every paragraph took before frames existed.
    /// </param>
    public LaidOutParagraph Layout(
        string text,
        ParagraphFormat? format = null,
        Length? emSize = null,
        Length? textAreaWidth = null,
        string? language = null,
        ParagraphFormat? follows = null,
        ShapingOptions? options = null,
        ILineObstacles? obstacles = null)
    {
        ArgumentNullException.ThrowIfNull(text);

        ParagraphFormat paragraph = format ?? ParagraphFormat.Default;
        Length size = emSize ?? Length.FromPoints(12);
        Length areaWidth = textAreaWidth ?? Length.FromMillimetres(170);

        // Snapped to whole twips before anything else uses it, because that is Writer's layout unit and
        // every later sum inherits the difference: a fifth of a twip per line accumulates over a page to
        // enough to fit one line more than Writer does.
        Length natural = Length.FromTwips(_metrics.ScaledLineHeight(size).Twips);
        Length height = paragraph.LineSpacing.Apply(natural);
        (Length baseline, Length spaceAbove) =
            BaselineIn(height, natural, size, paragraph.LineSpacing.Mode);

        WrappedLines? wrapped = obstacles is { IsEmpty: false }
            ? new WrappedLines(obstacles, paragraph, areaWidth, (_, _) => height)
            : null;

        List<TextLine> lines = _filler.Fill(
            text,
            size,
            paragraph.BodyWidth(areaWidth),
            paragraph.FirstLineWidth(areaWidth),
            language,
            options,
            paragraph,
            wrapped is null ? null : wrapped.WidthOfLine);

        List<LineBox> boxes = new(lines.Count);
        Length top = Length.Zero;

        for (int i = 0; i < lines.Count; i++)
        {
            bool isFirst = i == 0;
            LineSpace space = wrapped?.At(i, lines) ?? new LineSpace(
                paragraph.LineStart(isFirst),
                isFirst ? paragraph.FirstLineWidth(areaWidth) : paragraph.BodyWidth(areaWidth));

            // A stretch that shares its line with the next one keeps that line's geometry and does not
            // advance the pen down the paragraph — the box after it sits on the same baseline. Only when
            // there really is a box after it: the obstacles are asked whether a further stretch exists
            // before the filler has decided whether it has text for one, so the paragraph's last box can
            // be told it has a successor that was never emitted.
            bool shares = i + 1 < lines.Count && (wrapped?.SharesLineWithNext(i) ?? false);

            Length spaceAdd = Justification(
                paragraph, lines[i], text, space.Width, isLast: i == lines.Count - 1);

            boxes.Add(new LineBox(
                lines[i],
                LeftOf(paragraph, lines[i], space, areaWidth, spaceAdd, isLast: i == lines.Count - 1),
                top,
                height + space.Descent,
                baseline + space.Descent,
                spaceAbove,
                spaceAdd, shares));

            if (!shares) top += height + space.Descent;
        }

        return new LaidOutParagraph(
            boxes,
            SpaceBetween(follows, paragraph),
            paragraph.SpaceAfter);
    }

    /// <summary>
    /// Lays a paragraph out across its runs, where each line is as tall as its own tallest run.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The difference from the single-face overload is that the line height is no longer a paragraph-wide
    /// constant: a 24 pt word in an 11 pt paragraph makes <em>its</em> line taller and leaves the rest
    /// alone. So the spacing rule is applied per line, against the natural height of the tallest run that
    /// line touches, rather than once for the whole paragraph.
    /// </para>
    /// <para>
    /// The layouter's own face is not used here — the runs carry theirs. The instance is still needed for
    /// the break iterator and the fill loop, both of which are face-independent.
    /// </para>
    /// </remarks>
    /// <param name="measured">The paragraph, already shaped run by run.</param>
    /// <param name="format">Its resolved layout properties.</param>
    /// <param name="textAreaWidth">The width available before the paragraph's own indents.</param>
    /// <param name="language">A BCP 47 tag, for the language-specific break rules.</param>
    /// <param name="follows">The format of the paragraph above, for contextual spacing.</param>
    /// <param name="obstacles">The floating frames the text has to flow around, or null for none.</param>
    public LaidOutParagraph Layout(
        MeasuredParagraph measured,
        ParagraphFormat? format = null,
        Length? textAreaWidth = null,
        string? language = null,
        ParagraphFormat? follows = null,
        ILineObstacles? obstacles = null)
    {
        ArgumentNullException.ThrowIfNull(measured);

        ParagraphFormat paragraph = format ?? ParagraphFormat.Default;
        Length areaWidth = textAreaWidth ?? Length.FromMillimetres(170);

        WrappedLines? wrapped = obstacles is { IsEmpty: false }
            ? new WrappedLines(obstacles, paragraph, areaWidth, HeightOfLine)
            : null;

        List<TextLine> lines = _filler.Fill(
            measured,
            paragraph.BodyWidth(areaWidth),
            paragraph.FirstLineWidth(areaWidth),
            language,
            paragraph,
            wrapped is null ? null : wrapped.WidthOfLine);

        List<LineBox> boxes = new(lines.Count);
        Length top = Length.Zero;

        for (int i = 0; i < lines.Count; i++)
        {
            bool isFirst = i == 0;
            LineSpace space = wrapped?.At(i, lines) ?? new LineSpace(
                paragraph.LineStart(isFirst),
                isFirst ? paragraph.FirstLineWidth(areaWidth) : paragraph.BodyWidth(areaWidth));

            bool shares = i + 1 < lines.Count && (wrapped?.SharesLineWithNext(i) ?? false);

            // Across every stretch of the line, not just this one: a line whose text left of a frame is
            // 11 pt and whose text right of it is 24 pt is a 24 pt line, and both stretches sit on the
            // baseline that gives. Measuring each stretch alone would put the two on different baselines.
            (Length natural, Length ascent) = BandHeight(measured, lines, wrapped, i);

            Length height = paragraph.LineSpacing.Apply(natural);
            (Length baseline, Length spaceAbove) =
                BaselineFrom(height, natural, ascent, paragraph.LineSpacing.Mode);

            Length spaceAdd = Justification(
                paragraph, lines[i], measured.Text, space.Width,
                isLast: i == lines.Count - 1);

            boxes.Add(new LineBox(
                lines[i],
                LeftOf(paragraph, lines[i], space, areaWidth, spaceAdd, isLast: i == lines.Count - 1),
                top,
                height + space.Descent,
                baseline + space.Descent,
                spaceAbove,
                spaceAdd, shares));

            if (!shares) top += height + space.Descent;
        }

        // How tall a line already broken is, and — for the one not broken yet — a guess. Writer makes the
        // same guess for the same reason (`SwTextFormatter::CalcFlyWidth` calls `CalcRealHeight` before it
        // knows the line's content): the frames a line must avoid depend on how tall it is, and how tall it
        // is depends on which runs it ends up holding.
        Length HeightOfLine(int index, IReadOnlyList<TextLine> broken)
        {
            if (index < broken.Count)
            {
                (Length own, _) = measured.HeightOf(broken[index].Start, broken[index].VisibleEnd);
                return paragraph.LineSpacing.Apply(own);
            }

            (Length fallback, _) = measured.HeightOf(0, Math.Min(1, measured.Text.Length));
            return paragraph.LineSpacing.Apply(fallback);
        }

        return new LaidOutParagraph(
            boxes,
            SpaceBetween(follows, paragraph),
            paragraph.SpaceAfter);
    }

    /// <summary>
    /// The natural height and ascent of the whole line a stretch belongs to.
    /// </summary>
    /// <remarks>
    /// A line divided by a floating frame is still one line, so its height is the tallest run on any of
    /// its stretches and every stretch draws on the baseline that gives. The walk goes both ways from the
    /// stretch asked about, since the caller reaches the stretches in order and each has to answer for the
    /// line rather than for itself. Without obstacles there is nothing to walk and this is the single
    /// measurement it always was.
    /// </remarks>
    private static (Length Natural, Length Ascent) BandHeight(
        MeasuredParagraph measured, List<TextLine> lines, WrappedLines? wrapped, int index)
    {
        (Length natural, Length ascent) = measured.HeightOf(lines[index].Start, lines[index].VisibleEnd);
        if (wrapped is null) return (natural, ascent);

        int first = index;
        while (first > 0 && wrapped.SharesLineWithNext(first - 1)) first--;

        int last = index;
        while (last + 1 < lines.Count && wrapped.SharesLineWithNext(last)) last++;

        for (int i = first; i <= last; i++)
        {
            if (i == index) continue;

            (Length own, Length up) = measured.HeightOf(lines[i].Start, lines[i].VisibleEnd);
            natural = Length.Max(natural, own);
            ascent = Length.Max(ascent, up);
        }

        return (natural, ascent);
    }

    /// <summary>
    /// The space above a paragraph, once contextual spacing has had its say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Contextual spacing suppresses the gap between two paragraphs that share a style, which is what
    /// keeps a bulleted list from having a gap between every bullet while still having one before the
    /// list. Both sides have to ask for it: a paragraph with contextual spacing after one without still
    /// gets its space.
    /// </para>
    /// <para>
    /// Note what is deliberately <em>not</em> here: Word collapses a paragraph's space-before against
    /// the previous paragraph's space-after by taking the larger, and Writer adds them. The two differ
    /// visibly on any document with both set, and which one applies is a compatibility flag rather than
    /// a property of the paragraph — so it belongs to whatever assembles paragraphs into a page, which
    /// knows the flag, and not here.
    /// </para>
    /// </remarks>
    private static Length SpaceBetween(ParagraphFormat? previous, ParagraphFormat current)
        => SharesContextualSpacing(previous, current) ? Length.Zero : current.SpaceBefore;

    /// <summary>
    /// True when contextual spacing suppresses the gap between two consecutive paragraphs.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The whole gap, not half of it: Writer zeroes the upper space outright
    /// (<c>nUpper = bContextualSpacing ? 0 : …</c> in <c>SwFlowFrame::CalcUpperSpace</c>, both branches),
    /// and that upper space is what the previous paragraph's space-after and this one's space-before both
    /// feed into. Suppressing only the space-before leaves the space-after standing, which is the whole
    /// gap on a list whose style states an after and no before — the common shape, since Word's own
    /// <c>List Paragraph</c> style is exactly that.
    /// </para>
    /// <para>
    /// So the caller that materialises the gap has to ask this too, and take the previous paragraph's
    /// space-after back off again. This is exposed rather than folded into the layout because a
    /// paragraph is laid out once and the gap above it is decided per page, by whatever is stacking them.
    /// </para>
    /// </remarks>
    /// <param name="previous">The paragraph above, or null when there is none in this frame.</param>
    /// <param name="current">The paragraph whose upper space is being decided.</param>
    public static bool SharesContextualSpacing(ParagraphFormat? previous, ParagraphFormat current)
    {
        ArgumentNullException.ThrowIfNull(current);

        // Both sides have to ask for it: a paragraph with contextual spacing after one without still gets
        // its space.
        return previous is not null
               && current.HasContextualSpacing
               && previous.HasContextualSpacing
               && IdenticalStyles(previous, current);
    }

    /// <summary>
    /// Whether two paragraphs are set in the same style, which is what "contextual" means.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer compares the two nodes' format collections outright — <c>lcl_IdenticalStyles</c>,
    /// <c>sw/source/core/layout/flowfrm.cxx:1503</c>, which is a pointer comparison — so
    /// <see cref="ParagraphFormat.StyleKey"/> is what answers this when a reader supplies it.
    /// </para>
    /// <para>
    /// The fallback compares the properties a style carries, and it is a fallback rather than the
    /// rule because it gets a common case badly wrong: a heading style based on the body style
    /// inherits its indents, its alignment and its line spacing, so the two look identical and the
    /// space above every heading disappears. Measured on <c>technical report template.docx</c>
    /// (words/batch-010), whose <c>Normal</c> carries the <c>w:contextualSpacing</c> that all three
    /// heading styles inherit: the reference leaves 12 pt above each <c>Heading 1</c> and 8 pt above
    /// each <c>Heading 2</c>, and we left nothing — which is a page over nine.
    /// </para>
    /// </remarks>
    private static bool IdenticalStyles(ParagraphFormat previous, ParagraphFormat current)
        => previous.StyleKey is not null && current.StyleKey is not null
            ? string.Equals(previous.StyleKey, current.StyleKey, StringComparison.Ordinal)
            : previous.LineSpacing == current.LineSpacing
              && previous.StartIndent == current.StartIndent
              && previous.Alignment == current.Alignment;

    /// <summary>
    /// Where the baseline sits inside a line box, and how much of the box is empty above the text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The space that line spacing adds goes <em>above</em> the text, always. That is not a choice:
    /// Writer computes a line's drawing ascent as <c>ascent + realHeight - height</c> — its own text
    /// height subtracted from the height it will advance by — in
    /// <c>SwTextIter::CalcAscentAndHeight</c> (<c>sw/source/core/text/itrtxt.cxx</c>), so every extra
    /// twip lands between the previous line and this one. Putting it below instead leaves the pitch
    /// unchanged and the paragraph's height different, which is invisible to a comparison of line
    /// pitches and moves every page break.
    /// </para>
    /// <para>
    /// Exact spacing is the exception, and its rule is a plain fraction rather than anything derived
    /// from the font: <c>SwTextFormatter::CalcRealHeight</c> sets the ascent to
    /// <c>(4 * nLineHeight) / 5</c> — eighty per cent of the box, whatever the face's own ascent is —
    /// and marks the line as clipping when the text does not fit. So a fixed-height line's baseline
    /// does not move when the font changes, which is what a form with fixed rows wants.
    /// </para>
    /// </remarks>
    private (Length Baseline, Length SpaceAbove) BaselineIn(
        Length height, Length natural, Length emSize, LineSpacingMode mode)
        => BaselineFrom(
            height, natural, Length.FromTwips(_metrics.ScaledAscent(emSize).Twips), mode);

    /// <summary>The same rule, against an ascent the caller has already resolved.</summary>
    /// <remarks>
    /// Shared with the per-run overload, where the ascent belongs to whichever run made the line tallest
    /// rather than to the layouter's own face.
    /// </remarks>
    private static (Length Baseline, Length SpaceAbove) BaselineFrom(
        Length height, Length natural, Length ascent, LineSpacingMode mode)
    {
        // Four fifths of the box, in twips and by integer division: CalcRealHeight's (4 * height) / 5.
        if (mode == LineSpacingMode.Exact)
        {
            return (Length.FromTwips(4 * height.Twips / 5), Length.Zero);
        }

        Length extra = height - natural;
        return extra > Length.Zero ? (ascent + extra, extra) : (ascent, Length.Zero);
    }

    /// <summary>
    /// How far a line is shifted for its alignment.
    /// </summary>
    /// <remarks>
    /// Justified lines are not shifted — they are stretched, which changes the glyph positions inside
    /// the line rather than where it starts, and so belongs with the run that draws it. A justified
    /// paragraph's <em>last</em> line is start-aligned, which is the one place the two justification
    /// modes differ.
    /// </remarks>
    /// <summary>
    /// How much a justified line adds to each of its blanks.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The line's slack divided over its blanks, which is <c>SwTextAdjuster::CalcNewBlock</c>
    /// (<c>sw/source/core/text/itradj.cxx</c>): <c>nSpaceAdd = nGluePortionWidth / nGluePortion</c>, where
    /// the glue width is the slack and the glue count is the number of blanks. Not over the characters —
    /// that is what <c>IsOneBlock</c> does for a line with no blanks at all, and it looks quite different.
    /// </para>
    /// <para>
    /// In hundredths of a twip, truncated, because Writer computes it in
    /// <c>SPACING_PRECISION_FACTOR</c> units and then divides by that factor as a <em>double</em> when it
    /// builds the kern array. Rounding to whole twips per space instead would drift by up to half a twip
    /// per blank, which on a ten-blank line is a quarter of a point at the right margin.
    /// </para>
    /// <para>
    /// A line with no blanks is left alone rather than stretched by letter spacing. Writer does stretch it
    /// — <c>IsOneBlock</c> — but only when the paragraph asks for distributed alignment, and a single
    /// unbroken word filling a line is rare enough that leaving it short is the better error.
    /// </para>
    /// <para>
    /// <strong>The amount can be negative.</strong> A line the filler admitted on a
    /// <see cref="TextLine.ShrinkAllowance"/> holds more text than fits at natural widths, and the same
    /// division run over a negative slack is what squeezes its blanks back inside the margin — see
    /// <see cref="JustificationShrink"/>. Unlike stretching, it applies to the paragraph's <em>last</em>
    /// line too: Writer refuses to skip block formatting for a last line whose natural width exceeds the
    /// paragraph's ("if the last line is longer than the paragraph width, it contains shrinking spaces:
    /// don't skip block format here", <c>SwTextAdjuster::FormatBlock</c>,
    /// <c>sw/source/core/text/itradj.cxx</c>), and it has to, because a squeezed line can be the one the
    /// paragraph ends on.
    /// </para>
    /// <para>
    /// <strong>A tab does not stop a line being justified, but it does decide which blanks carry the
    /// slack.</strong> Writer spans a line "between two RandPortions or FixPortions (Tabs and Flys)"
    /// (<c>sw/source/core/text/itradj.cxx:255</c>) and gives each span its own space-add. A tab is a
    /// <c>SwFixPortion</c> whose fix width is its whole width (<c>SetFixWidth(PrtWidth())</c>,
    /// <c>sw/source/core/text/txttab.cxx:569</c>), so its <c>GetPrtGlue()</c> is nought — the span closed
    /// by a tab is stretched by nothing, and only the last span, closed by the right margin's glue,
    /// receives the line's slack. Hence the count starts after the line's <em>last</em> tab.
    /// </para>
    /// </remarks>
    private static Length Justification(
        ParagraphFormat format, TextLine line, string text, Length available, bool isLast)
    {
        TextAlignment alignment = format.Alignment;
        bool justifiable = alignment is TextAlignment.Justify or TextAlignment.Distribute;
        if (!justifiable) return Length.Zero;

        Length slack = available - line.Width;

        if (slack > Length.Zero)
        {
            // The last line of a justified paragraph is not stretched; under distributed alignment it is,
            // which is the only difference between the two modes.
            if (alignment == TextAlignment.Justify && isLast) return Length.Zero;
        }
        else if (slack < Length.Zero)
        {
            // Only as far as the filler allowed for, and no further: an overrun this does not account for
            // is a line that overflowed for some other reason — an over-long word, an inline object — and
            // squeezing its blanks would not bring it inside the margin anyway.
            if (line.ShrinkAllowance <= Length.Zero) return Length.Zero;
            if (Length.Zero - slack > line.ShrinkAllowance) return Length.Zero;
        }
        else
        {
            return Length.Zero;
        }

        if (StopsAtAlignedTabBeforeBreak(format, line, text)) return Length.Zero;

        // Only the blanks after the line's last tab share the slack — see the remarks. A line without a
        // tab starts at its own start, which is every line in nearly every paragraph.
        int from = Math.Max(LastTabOn(text, line) + 1, line.Start);

        int blanks = 0;
        for (int at = from; at < Math.Min(line.VisibleEnd, text.Length); at++)
        {
            if (text[at] == ' ') blanks++;
        }

        if (blanks == 0) return Length.Zero;

        long hundredths = slack.Twips * 100 / blanks;
        return Length.FromEmu(hundredths * Length.EmuPerTwip / 100);
    }

    /// <summary>
    /// The one case a tab does stop a line being justified: a manual break reached with a centre, right
    /// or decimal stop in force.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>bDoNotJustifyTab</c> in <c>SwTextAdjuster::CalcNewBlock</c>
    /// (<c>sw/source/core/text/itradj.cxx:292</c>). A <c>TabCenter</c>, <c>TabRight</c> or
    /// <c>TabDecimal</c> portion sets it and a <c>TabLeft</c> clears it again, and it is consulted only
    /// where the loop meets a break portion — at which point <c>FinishSpaceAdd</c> zeroes what is left
    /// and the line is abandoned ragged. So the same text split by a manual break is justified after a
    /// left tab and ragged after a right one. Measured on a probe paragraph of two justified lines split
    /// by a <c>w:br</c>, right margin 538.58 pt: with a left stop LibreOffice 24.2.7.2 ends the broken
    /// line at 538.50, with a right stop at 262.22.
    /// </para>
    /// <para>
    /// <strong>Approximated in one direction, deliberately.</strong> Which stop a tab lands on depends on
    /// how far along the line it falls, which is not knowable from the line alone — resolving it here
    /// would mean measuring the line's text a second time, in this layer, for every justified paragraph.
    /// So the test is whether the paragraph <em>declares</em> a stop that is not left-aligned: a
    /// paragraph declaring none — nearly all of them, since the default interval's stops are all left —
    /// can never have one in force, and a paragraph declaring only aligned stops always does. A
    /// paragraph mixing the two, whose line ends in a manual break, and whose last tab lands on the left
    /// stop, is left ragged where Writer would justify it. That needs all three at once.
    /// </para>
    /// </remarks>
    private static bool StopsAtAlignedTabBeforeBreak(
        ParagraphFormat format, TextLine line, string text)
    {
        if (!TabRuler.HasTab(text, line.Start, line.VisibleEnd)) return false;

        bool aligned = false;
        foreach (TabStop stop in format.TabStops)
        {
            if (stop.Alignment == TabAlignment.Left) continue;
            aligned = true;
            break;
        }

        if (!aligned) return false;

        // The break itself, which the filler trims off the visible end rather than leaving on it — so the
        // question is whether anything between the visible end and the line's end ends a line. The
        // character before the visible end is included because U+2028 is not one of the blanks the filler
        // trims, so a line ending on it can carry it inside its visible range.
        int at = Math.Max(line.Start, Math.Min(line.VisibleEnd, text.Length) - 1);
        for (; at < Math.Min(line.End, text.Length); at++)
        {
            if (EndsLine(text[at])) return true;
        }

        return false;
    }

    /// <summary>The last tab on a line, or one before its start when it holds none.</summary>
    /// <remarks>
    /// Where a justified line's stretch begins. Searched from the visible end backwards because that is
    /// the only tab that matters: every earlier stretch was closed by a tab of its own and given that
    /// tab's glue, which is nought.
    /// </remarks>
    private static int LastTabOn(string text, TextLine line)
    {
        int at = Math.Min(line.VisibleEnd, text.Length) - 1;
        while (at >= line.Start && text[at] != '\t') at--;
        return at;
    }

    /// <summary>True for a character UAX #14 gives a mandatory break.</summary>
    /// <remarks>
    /// The same set <see cref="TextMeasurer"/> trims from a line's visible end, and for the same reason:
    /// the readers do not agree on which to use — OOXML's and ODF's manual breaks arrive as U+2028 and
    /// RTF's as a newline, and every one of them is Writer's break portion.
    /// </remarks>
    private static bool EndsLine(char character)
        => character is '\u2028' or '\u2029' or '\n' or '\r' or '\u000B' or '\u000C' or '\u0085';

    /// <summary>
    /// The stretch each line of an obstructed paragraph gets, resolved once and remembered.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Once, and that is the point of the class rather than a lambda: the width a line was <em>filled</em>
    /// at and the width it is <em>placed</em> at must be the same number. Asking the obstacles twice would
    /// invite them to answer differently — the second call knows the line's real height where the first
    /// only guessed it — and a line broken for one width and drawn in another either overflows a frame or
    /// stops short of it.
    /// </para>
    /// <para>
    /// A line's top is the sum of everything above it, descents included, which is why this cannot be a
    /// pure function of the index: a line pushed below a top-and-bottom wrapped frame moves every line
    /// after it down by the same amount. Recomputed from the front on each new line rather than kept as a
    /// running total, because the heights of the lines already broken are only knowable once they are —
    /// a paragraph is a few dozen lines and this path is taken only beside a frame.
    /// </para>
    /// <para>
    /// One index here is one <em>stretch</em>, not one line. A frame clear of both margins leaves room on
    /// both sides of it, so the filler is handed the gap on the left, then the gap on the right at the same
    /// height, and the two make one line. That is Writer's own shape: <c>CalcFlyWidth</c> is called again
    /// with the pen already past the fly portion (<c>itrform2.cxx</c>), and it answers for the stretch
    /// starting there. So a continuation is not a new question — it is the same question asked from
    /// further along, which is why <see cref="ILineObstacles"/> needs nothing added for it.
    /// </para>
    /// </remarks>
    private sealed class WrappedLines(
        ILineObstacles obstacles,
        ParagraphFormat format,
        Length areaWidth,
        Func<int, IReadOnlyList<TextLine>, Length> heightOfLine)
    {
        /// <summary>
        /// The narrowest gap worth giving text to, which is Writer's <c>MINLAY</c>.
        /// </summary>
        /// <remarks>
        /// 23 twips — <c>sw/inc/swtypes.hxx</c>, "minimal size for other frames", and the limit
        /// <c>CalcFlyWidth</c> compares a remaining stretch against before treating the line as full. It
        /// matters because the filler gives an over-long word the line to itself rather than leaving the
        /// line empty: without a floor, a two-millimetre gap beside a frame would be handed a whole word,
        /// which would then be drawn straight across the frame.
        /// </remarks>
        private static readonly Length SmallestStretch = Length.FromTwips(23);

        private readonly List<LineSpace> _spaces = [];
        private readonly List<Length> _heights = [];
        private readonly List<bool> _shares = [];

        /// <summary>The width to fill stretch <paramref name="index"/> to.</summary>
        public Length WidthOfLine(int index, IReadOnlyList<TextLine> broken)
            => At(index, broken).Width;

        /// <summary>True when the stretch after this one is more of the same line.</summary>
        public bool SharesLineWithNext(int index) => index < _shares.Count && _shares[index];

        /// <summary>The stretch <paramref name="index"/> gets, resolving it if it is new.</summary>
        public LineSpace At(int index, IReadOnlyList<TextLine> broken)
        {
            while (_spaces.Count <= index)
            {
                int line = _spaces.Count;

                // Only the stretches that end their line advance the paragraph. The ones before them sit
                // on the same baseline as the stretch after, so adding their height would leave a blank
                // line under every frame that text passes on both sides of.
                Length top = Length.Zero;
                for (int above = 0; above < line; above++)
                {
                    if (_shares[above]) continue;
                    top += _spaces[above].Descent + _heights[above];
                }

                bool continues = line > 0 && _shares[line - 1];

                // The line's own height, which for a continuation is the height its first stretch was
                // resolved at. Writer guesses the same way and for the same reason — the height depends on
                // the runs the stretch ends up holding, which is not known until it has been filled.
                Length height = continues ? _heights[line - 1] : heightOfLine(line, broken);

                LineSpace wanted = continues
                    ? Past(_spaces[line - 1], Wanted(BandStart(line)))
                    : Wanted(line);

                LineSpace space = obstacles.SpaceFor(top, height, wanted);

                _spaces.Add(space);
                _heights.Add(height);
                _shares.Add(HasMore(top, height, wanted, space));
            }

            // The heights of the lines already broken are now known exactly, so the tops of the lines
            // after them are too. Refreshing keeps the placing pass and the filling pass in step. A
            // continuation keeps its line's height rather than measuring its own share of the text.
            for (int line = 0; line < Math.Min(_heights.Count, broken.Count); line++)
            {
                _heights[line] = line > 0 && _shares[line - 1]
                    ? _heights[line - 1]
                    : heightOfLine(line, broken);
            }

            return _spaces[index];
        }

        /// <summary>The stretch the paragraph's own indents would have given a line.</summary>
        private LineSpace Wanted(int line)
            => new(
                format.LineStart(line == 0),
                line == 0 ? format.FirstLineWidth(areaWidth) : format.BodyWidth(areaWidth));

        /// <summary>Which stretch begins the line that stretch <paramref name="line"/> belongs to.</summary>
        private int BandStart(int line)
        {
            int first = line;
            while (first > 0 && _shares[first - 1]) first--;
            return first;
        }

        /// <summary>What is left of a wanted stretch once one has been taken out of its start.</summary>
        private static LineSpace Past(LineSpace taken, LineSpace wanted)
        {
            Length end = taken.Left + taken.Width;
            return new LineSpace(end, wanted.Left + wanted.Width - end);
        }

        /// <summary>
        /// Whether the same line has a further stretch past the one just resolved.
        /// </summary>
        /// <remarks>
        /// Asked of the obstacles from the far edge of the stretch taken, which is exactly what Writer's
        /// second <c>CalcFlyWidth</c> call does. Two answers are refused: one carrying a descent, since
        /// that is the line running out of room and dropping rather than continuing beside the frame, and
        /// one narrower than <see cref="SmallestStretch"/>, since a sliver would be handed a whole word.
        /// A line that no frame narrowed costs nothing here — the stretch taken reaches the end of the
        /// stretch wanted, so there is nothing left to ask about.
        /// </remarks>
        private bool HasMore(Length top, Length height, LineSpace wanted, LineSpace taken)
        {
            LineSpace rest = Past(taken, wanted);
            if (rest.Width <= SmallestStretch) return false;

            LineSpace next = obstacles.SpaceFor(top, height, rest);
            return next.Descent == Length.Zero && next.Width > SmallestStretch;
        }
    }

    /// <summary>
    /// Where a line's text starts, measured from the text area's <em>left</em> edge.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The one place a right-to-left paragraph differs from a left-to-right one, and it is a
    /// mirror rather than a special case: the room the line was given — indents included — is
    /// reflected in the text area, and the line is placed against the far side of it. That is
    /// Writer's own shape, which lays a right-to-left frame out as though it were left to right
    /// and reflects the rectangle when it paints (<c>SwTextFrame::SwitchLTRtoRTL</c>,
    /// <c>sw/source/core/text/txtfrm.cxx:682</c>), so the indents mirror without anything having
    /// to know which side they came from.
    /// </para>
    /// <para>
    /// Measured, because the two halves of the mirror are not both obvious. An ODF paragraph in
    /// <c>rl-tb</c> with <c>fo:margin-right="3cm"</c> is drawn three centimetres from the
    /// <em>left</em> margin — its end indent — and one with <c>fo:text-indent="2cm"</c> starts its
    /// first line two centimetres in from the right.
    /// </para>
    /// <para>
    /// A justified line is the exception the mirror needs told about: it has already been stretched
    /// to fill the room, so it has no slack to be pushed across and starts at the reflected left
    /// edge. Reflecting its alignment offset anyway would move it right by the slack it is about to
    /// consume and run it off the margin.
    /// </para>
    /// </remarks>
    private static Length LeftOf(
        ParagraphFormat paragraph,
        TextLine line,
        LineSpace space,
        Length areaWidth,
        Length spaceAdd,
        bool isLast)
    {
        Length offset = AlignmentOffset(paragraph.Alignment, line, space.Width, isLast);
        if (!paragraph.IsRightToLeft) return space.Left + offset;

        Length beyond = areaWidth - space.Left - space.Width;
        Length slack = space.Width - line.Width;
        if (slack < Length.Zero) slack = Length.Zero;

        return (beyond > Length.Zero ? beyond : Length.Zero)
               + (spaceAdd != Length.Zero ? Length.Zero : slack - offset);
    }

    private static Length AlignmentOffset(
        TextAlignment alignment, TextLine line, Length available, bool isLast)
    {
        Length slack = available - line.Width;
        if (slack <= Length.Zero) return Length.Zero;

        return alignment switch
        {
            TextAlignment.End => slack,
            TextAlignment.Centre => slack / 2,
            TextAlignment.Justify when isLast => Length.Zero,
            _ => Length.Zero,
        };
    }
}

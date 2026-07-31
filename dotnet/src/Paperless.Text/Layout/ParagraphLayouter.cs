using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;

namespace Paperless.Text.Layout;

/// <summary>
/// One line of a laid-out paragraph, positioned.
/// </summary>
/// <param name="Line">Which characters the line holds, and how wide its visible text is.</param>
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
public readonly record struct LineBox(
    TextLine Line,
    Length Left,
    Length Top,
    Length Height,
    Length Baseline,
    Length SpaceAbove,
    Length SpaceAdd = default)
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
    public ParagraphLayouter(OpenTypeFace face, ILineBreaker? breaker = null)
    {
        ArgumentNullException.ThrowIfNull(face);
        _measurer = new TextMeasurer(face);
        _filler = new LineFiller(_measurer, breaker);
        _metrics = LineSpacing.Resolve(face);
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
    public LaidOutParagraph Layout(
        string text,
        ParagraphFormat? format = null,
        Length? emSize = null,
        Length? textAreaWidth = null,
        string? language = null,
        ParagraphFormat? follows = null,
        ShapingOptions? options = null)
    {
        ArgumentNullException.ThrowIfNull(text);

        ParagraphFormat paragraph = format ?? ParagraphFormat.Default;
        Length size = emSize ?? Length.FromPoints(12);
        Length areaWidth = textAreaWidth ?? Length.FromMillimetres(170);

        List<TextLine> lines = _filler.Fill(
            text,
            size,
            paragraph.BodyWidth(areaWidth),
            paragraph.FirstLineWidth(areaWidth),
            language,
            options);

        // Snapped to whole twips before anything else uses it, because that is Writer's layout unit and
        // every later sum inherits the difference: a fifth of a twip per line accumulates over a page to
        // enough to fit one line more than Writer does.
        Length natural = Length.FromTwips(_metrics.ScaledLineHeight(size).Twips);
        Length height = paragraph.LineSpacing.Apply(natural);
        (Length baseline, Length spaceAbove) =
            BaselineIn(height, natural, size, paragraph.LineSpacing.Mode);

        List<LineBox> boxes = new(lines.Count);
        Length top = Length.Zero;

        for (int i = 0; i < lines.Count; i++)
        {
            bool isFirst = i == 0;
            Length available = isFirst
                ? paragraph.FirstLineWidth(areaWidth)
                : paragraph.BodyWidth(areaWidth);

            boxes.Add(new LineBox(
                lines[i],
                paragraph.LineStart(isFirst) + AlignmentOffset(
                    paragraph.Alignment, lines[i], available, isLast: i == lines.Count - 1),
                top,
                height,
                baseline,
                spaceAbove,
                Justification(
                    paragraph.Alignment, lines[i], text, available,
                    isLast: i == lines.Count - 1)));

            top += height;
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
    public LaidOutParagraph Layout(
        MeasuredParagraph measured,
        ParagraphFormat? format = null,
        Length? textAreaWidth = null,
        string? language = null,
        ParagraphFormat? follows = null)
    {
        ArgumentNullException.ThrowIfNull(measured);

        ParagraphFormat paragraph = format ?? ParagraphFormat.Default;
        Length areaWidth = textAreaWidth ?? Length.FromMillimetres(170);

        List<TextLine> lines = _filler.Fill(
            measured,
            paragraph.BodyWidth(areaWidth),
            paragraph.FirstLineWidth(areaWidth),
            language);

        List<LineBox> boxes = new(lines.Count);
        Length top = Length.Zero;

        for (int i = 0; i < lines.Count; i++)
        {
            bool isFirst = i == 0;
            Length available = isFirst
                ? paragraph.FirstLineWidth(areaWidth)
                : paragraph.BodyWidth(areaWidth);

            (Length natural, Length ascent) =
                measured.HeightOf(lines[i].Start, lines[i].VisibleEnd);

            Length height = paragraph.LineSpacing.Apply(natural);
            (Length baseline, Length spaceAbove) =
                BaselineFrom(height, natural, ascent, paragraph.LineSpacing.Mode);

            boxes.Add(new LineBox(
                lines[i],
                paragraph.LineStart(isFirst) + AlignmentOffset(
                    paragraph.Alignment, lines[i], available, isLast: i == lines.Count - 1),
                top,
                height,
                baseline,
                spaceAbove,
                Justification(
                    paragraph.Alignment, lines[i], measured.Text, available,
                    isLast: i == lines.Count - 1)));

            top += height;
        }

        return new LaidOutParagraph(
            boxes,
            SpaceBetween(follows, paragraph),
            paragraph.SpaceAfter);
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
    {
        if (previous is null) return current.SpaceBefore;

        bool contextual = current.HasContextualSpacing
                          && previous.HasContextualSpacing
                          && previous.LineSpacing == current.LineSpacing
                          && previous.StartIndent == current.StartIndent
                          && previous.Alignment == current.Alignment;

        return contextual ? Length.Zero : current.SpaceBefore;
    }

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
    /// </remarks>
    private static Length Justification(
        TextAlignment alignment, TextLine line, string text, Length available, bool isLast)
    {
        // The last line of a justified paragraph is not stretched; under distributed alignment it is,
        // which is the only difference between the two modes.
        bool justified = alignment == TextAlignment.Distribute
                         || (alignment == TextAlignment.Justify && !isLast);
        if (!justified) return Length.Zero;

        Length slack = available - line.Width;
        if (slack <= Length.Zero) return Length.Zero;

        int blanks = 0;
        for (int at = line.Start; at < Math.Min(line.VisibleEnd, text.Length); at++)
        {
            if (text[at] == ' ') blanks++;
        }

        if (blanks == 0) return Length.Zero;

        long hundredths = slack.Twips * 100 / blanks;
        return Length.FromEmu(hundredths * Length.EmuPerTwip / 100);
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

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
/// How much of the box is empty space above the text. Kept separately because whoever fills pages has
/// to be able to take it away: the first line at the top of a page loses the space above it, so its
/// text sits on the top margin rather than a line's worth below it, and recomputing it there would mean
/// knowing the spacing rule — which is the paragraph's business rather than the page's.
/// </param>
public readonly record struct LineBox(
    TextLine Line,
    Length Left,
    Length Top,
    Length Height,
    Length Baseline,
    Length SpaceAbove)
{
    /// <summary>The same line with the space above its text removed.</summary>
    /// <remarks>
    /// What the first line on a page becomes. Writer drops the space above the first line of a text
    /// frame — the observable consequence is that a 200%-spaced A4 page holds twenty-five lines rather
    /// than twenty-four, and every page break after the first therefore falls somewhere else.
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

        Length natural = _metrics.ScaledLineHeight(size);
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
                spaceAbove));

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
    {
        if (mode == LineSpacingMode.Exact) return (height * 0.8, Length.Zero);

        Length ascent = _metrics.ScaledAscent(emSize);
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

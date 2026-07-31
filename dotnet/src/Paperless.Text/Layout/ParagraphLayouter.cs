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
/// height, because the extra space that line spacing adds does not go where the font's own ascent
/// would put it.
/// </param>
public readonly record struct LineBox(
    TextLine Line,
    Length Left,
    Length Top,
    Length Height,
    Length Baseline)
{
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
/// Two decisions here are worth stating because they are asymmetrical and easy to get backwards.
/// Proportional line spacing puts its extra height <em>above</em> the text, so a double-spaced
/// paragraph pushes its first baseline down rather than leaving a gap under its last line. And exact
/// spacing shorter than the text does not move the baseline up past the top of the box: the text is
/// clipped from below, because that is what a form with fixed rows is asking for.
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
        Length baseline = BaselineIn(height, natural, size, paragraph.LineSpacing.Mode);

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
                baseline));

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
    /// Where the baseline sits inside a line box.
    /// </summary>
    /// <remarks>
    /// <para>
    /// For single spacing it is the font's own ascent, which is what puts consecutive baselines exactly
    /// one line height apart. For anything larger the question is where the extra space goes, and the
    /// answer is above the text: Writer pushes the first baseline down rather than leaving the gap under
    /// the last line, so a two-line double-spaced paragraph is as tall as four single-spaced lines and
    /// its first line of text is not at the top of it.
    /// </para>
    /// <para>
    /// Exact spacing is the case that needs a floor. A box shorter than the ascent would put the
    /// baseline above the box's own top and the text would climb into the paragraph before it, so the
    /// baseline is clamped to the box and the text is clipped from below instead — which is what a form
    /// with fixed rows is asking for when it sets a height its text does not fit in.
    /// </para>
    /// </remarks>
    private Length BaselineIn(Length height, Length natural, Length emSize, LineSpacingMode mode)
    {
        Length ascent = _metrics.ScaledAscent(emSize);

        if (mode == LineSpacingMode.Exact)
        {
            return height < ascent ? height : ascent;
        }

        Length extra = height - natural;
        return extra > Length.Zero ? ascent + extra : ascent;
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

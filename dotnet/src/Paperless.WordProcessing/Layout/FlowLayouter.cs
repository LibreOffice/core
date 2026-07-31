using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// Lays a run of paragraphs out into one rectangle, as a header, a footer or a table cell.
/// </summary>
/// <remarks>
/// <para>
/// The operation all three share, and the reason <see cref="PlacedFlow"/> is one type: stack the
/// paragraphs at the rectangle's width, with their spacing between them, and report where each line
/// landed. What none of the three does is <em>flow</em> — nothing here splits across a page, because a
/// header is furniture and a cell belongs to its row. The body's own paragraphs go through
/// <see cref="Paginator"/> instead, which is the same stacking plus everything that makes a page end.
/// </para>
/// <para>
/// Height is discovered rather than given, which is what a table needs: a row is as tall as its tallest
/// cell, so the cells have to be laid out before the row's height is known, and laying one out cannot
/// require it.
/// </para>
/// </remarks>
public static class FlowLayouter
{
    /// <summary>
    /// Lays paragraphs out into a rectangle, or returns null when there is nothing to place.
    /// </summary>
    /// <param name="paragraphs">The paragraphs, in order.</param>
    /// <param name="area">The rectangle to fill, whose width decides the line breaks.</param>
    /// <param name="offsetFromTop">
    /// Where the first line goes: zero to grow downwards from the area's top, a value to start that far
    /// below it, and null to <em>bottom-align</em> the whole block so that its last line rests on the
    /// area's bottom. Null is what a Word footer does; see <see cref="Model.PageGeometry.FooterOffset"/>.
    /// </param>
    /// <remarks>
    /// Nothing is clipped and nothing overflows into a second rectangle: content taller than the area is
    /// placed anyway and runs past its bottom, which is what Writer does with a fixed-height header whose
    /// text does not fit. A stated offset is honoured even then, so an overflowing footer grows downwards
    /// rather than climbing into the body.
    /// </remarks>
    public static PlacedFlow? LayOut(
        IReadOnlyList<PageParagraph> paragraphs, DocRect area, Length? offsetFromTop)
    {
        ArgumentNullException.ThrowIfNull(paragraphs);

        if (paragraphs.Count == 0 || area.Width <= Length.Zero) return null;

        List<PlacedLine> placed = [];
        Length top = Length.Zero;

        for (int i = 0; i < paragraphs.Count; i++)
        {
            PageParagraph paragraph = paragraphs[i];
            ParagraphLayouter layouter = new(paragraph.Face);
            ParagraphFormat? previous = i > 0 ? paragraphs[i - 1].Format : null;

            LaidOutParagraph layout = paragraph.HasRuns
                ? layouter.Layout(
                    MeasuredParagraph.Measure(
                        paragraph.Text,
                        [.. paragraph.Runs.Select(run => run.ToFormattedRun())]),
                    paragraph.Format,
                    area.Width,
                    paragraph.Language,
                    previous)
                : layouter.Layout(
                    paragraph.Text,
                    paragraph.Format,
                    paragraph.EmSize,
                    area.Width,
                    paragraph.Language,
                    previous,
                    paragraph.Shaping);

            top += layout.SpaceBefore;

            for (int line = 0; line < layout.Lines.Count; line++)
            {
                LineBox box = layout.Lines[line];

                // The first line loses the leading above its text, exactly as the first line of a page's
                // body does: the space belongs to the paragraph's upper margin and is dropped at the top
                // of a frame, and each of these three is a frame.
                if (placed.Count == 0) box = box.WithoutSpaceAbove();

                placed.Add(new PlacedLine(i, line, box, top));
                top += box.Height;
            }

            top += layout.SpaceAfter;
        }

        if (placed.Count == 0) return null;

        // Where the block as a whole goes. A bottom-aligned one only shifts when there is room to shift
        // into; a stated offset is taken as given even when the content is taller than the area.
        Length shift = offsetFromTop ?? (top < area.Height ? area.Height - top : Length.Zero);

        if (shift != Length.Zero)
        {
            for (int i = 0; i < placed.Count; i++)
            {
                placed[i] = placed[i] with { Top = placed[i].Top + shift };
            }
        }

        return new PlacedFlow
        {
            Paragraphs = paragraphs,
            Lines = placed,
            Area = area,
        };
    }

    /// <summary>
    /// How tall the paragraphs are at a given width, without placing them anywhere.
    /// </summary>
    /// <remarks>
    /// What a table's rows are sized from: a cell's height is its content's, and the row's is the tallest
    /// cell's. Measured by laying the flow out into a rectangle of unbounded height and asking where it
    /// ended, because that is the only answer that agrees with where the lines will actually be drawn —
    /// summing estimated line heights instead would drift from the real result exactly where it matters.
    /// </remarks>
    public static Length HeightOf(IReadOnlyList<PageParagraph> paragraphs, Length width)
    {
        PlacedFlow? flow = LayOut(
            paragraphs, new DocRect(Length.Zero, Length.Zero, width, Length.Zero), Length.Zero);

        return flow is null || flow.Lines.Count == 0
            ? Length.Zero
            : flow.Lines[^1].Top + flow.Lines[^1].Box.Height;
    }
}

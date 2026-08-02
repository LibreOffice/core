using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// Paints the text inside a shape anchored on a sheet.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The print zoom scales the type, not just the rectangle</strong>, for the same reason
/// <c>SheetChart</c> records: the box arrives already scaled and the sizes do not, so a body laid
/// out at full size inside a half-size rectangle wraps at half the words it should.
/// </para>
/// <para>
/// <strong>Wrapping is by whole words.</strong> That is what <c>wrap="square"</c> means and what
/// every text box in the corpus asks for; a body stating <c>wrap="none"</c> is drawn on one line.
/// A single word too wide for the box is left to run past it rather than broken, which is the one
/// place this is knowingly cruder than the cell engine — it shows only on an unbroken string wider
/// than its own shape, and a text box that narrow is not a case the corpus has.
/// </para>
/// </remarks>
internal static class SheetShapePainter
{
    /// <summary>Paints one shape's text into the rectangle its anchor gave it.</summary>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="text">The shape's text.</param>
    /// <param name="box">Where the shape lands on the page, already scaled.</param>
    /// <param name="scale">The print zoom, applied to the type.</param>
    public static void Draw(IDrawingSink sink, SheetShapeText text, DocRect box, double scale)
    {
        ArgumentNullException.ThrowIfNull(sink);
        ArgumentNullException.ThrowIfNull(text);

        if (text.IsEmpty || box.Width <= Length.Zero || box.Height <= Length.Zero) return;

        Length left = box.X + (text.LeftInset * scale);
        Length right = box.X + box.Width - (text.RightInset * scale);
        Length available = right - left;
        if (available <= Length.Zero) return;

        List<Line> lines = Lines(text, available, scale);
        if (lines.Count == 0) return;

        Length total = Length.Zero;
        foreach (Line line in lines) total += line.Height;

        Length top = box.Y + (text.TopInset * scale);
        Length room = box.Height - (text.TopInset * scale) - (text.BottomInset * scale);
        if (text.Anchor == SheetShapeAnchor.Middle && room > total) top += (room - total) / 2;
        else if (text.Anchor == SheetShapeAnchor.Bottom && room > total) top += room - total;

        Length pen = top;
        foreach (Line line in lines)
        {
            // A blank paragraph carries no run and only advances the pen, which is what keeps the
            // gap a text box puts between its blocks.
            if (line.Run is { } run)
            {
                Length x = line.Alignment switch
                {
                    SheetShapeAlignment.Centre => left + ((available - run.Width) / 2),
                    SheetShapeAlignment.Right => right - run.Width,
                    _ => left,
                };

                sink.DrawGlyphRun(
                    run.At(new DocPoint(x, pen + SheetBandText.AscentAt(line.Size))),
                    Paint.Solid(Colour.Black));
            }

            pen += line.Height;
        }
    }

    /// <summary>One laid-out line: its run, its size, the height it advances, and its alignment.</summary>
    private readonly record struct Line(
        BandRun? Run, Length Size, Length Height, SheetShapeAlignment Alignment);

    /// <summary>Shapes every paragraph into the lines it wraps to.</summary>
    private static List<Line> Lines(SheetShapeText text, Length available, double scale)
    {
        List<Line> lines = [];
        bool anyInk = false;

        foreach (SheetShapeParagraph paragraph in text.Paragraphs)
        {
            Length size = paragraph.Size;
            if (size <= Length.Zero) size = SheetShapeText.DefaultSize;
            size *= scale;

            Length height = SheetBandText.ChartLineHeightAt(size);
            string body = paragraph.Text;

            if (body.Length == 0)
            {
                lines.Add(new Line(null, size, height, paragraph.Alignment));
                continue;
            }

            foreach (string line in Wrap(body, size, available, text.Wraps))
            {
                if (SheetBandText.Shape(line, size) is not { } run) continue;
                lines.Add(new Line(run, size, height, paragraph.Alignment));
                anyInk = true;
            }
        }

        // Nothing shaped means no face resolved, and a column of blank advances is not worth
        // walking: the caller draws nothing rather than reserving room for it.
        return anyInk ? lines : [];
    }

    /// <summary>Breaks one paragraph into lines that fit the width.</summary>
    private static List<string> Wrap(string body, Length size, Length available, bool wraps)
    {
        if (!wraps) return [body];

        List<string> lines = [];
        string current = string.Empty;

        foreach (string word in body.Split(' ', StringSplitOptions.None))
        {
            string candidate = current.Length == 0 ? word : current + " " + word;
            if (current.Length > 0 && Width(candidate, size) > available)
            {
                lines.Add(current);
                current = word;
                continue;
            }

            current = candidate;
        }

        if (current.Length > 0) lines.Add(current);
        return lines.Count == 0 ? [body] : lines;
    }

    private static Length Width(string text, Length size)
        => SheetBandText.Shape(text, size)?.Width ?? Length.Zero;
}

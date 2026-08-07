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
/// <strong>Every run is measured at its own size and in its own face.</strong> A paragraph is not
/// one size and one face: <c>TextRun::insertAt</c> pushes each run's character properties
/// separately (<c>oox/source/drawingml/textrun.cxx:82-86</c>) and the EditEngine breaks a portion
/// at every one of those boundaries, so a paragraph mixing 11 pt body text with a 12 pt trailing
/// space wraps the body at eleven. Collapsing a paragraph to the largest size any run states —
/// which is what this did — measures every word of a long body at the size of one stray character
/// and breaks each line early. Measured on a probe round-tripped through LibreOffice's own
/// flat-ODS export: a body at <c>sz="1100"</c> followed by an unsized space wraps in the same
/// place as the same body alone, and a run stating <c>sz="1800"</c> after it leaves the body's own
/// breaks untouched.
/// </para>
/// <para>
/// <strong>A word may span a run boundary</strong>, so the wrap cannot be done run by run either.
/// <c>SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.xlsx</c> splits "either" across two runs
/// as <c>" e"</c> and <c>"ither the date…"</c>, which is what authoring tools leave behind when a
/// character property is applied and then undone. The paragraph is therefore flattened to text
/// with a per-character format beside it, wrapped as one string, and each line then cut back into
/// the maximal stretches that share a format.
/// </para>
/// <para>
/// <strong>Wrapping is by whole words.</strong> That is what <c>wrap="square"</c> means and what
/// every text box in the corpus asks for; a body stating <c>wrap="none"</c> is drawn on one line.
/// A single word too wide for the box is left to run past it rather than broken, which is the one
/// place this is knowingly cruder than the cell engine — it shows only on an unbroken string wider
/// than its own shape, and a text box that narrow is not a case the corpus has.
/// </para>
/// <para>
/// <strong>A body stating <c>vertOverflow="clip"</c> loses the lines that do not fit</strong>, and
/// loses them rather than merely hiding them — see
/// <see cref="SheetShapeText.ClipsVerticalOverflow"/>. Measured on
/// <c>Foreign_SA-CAT-I_and_CAT-II-III_Pub_0.xlsx</c>, whose notes box is 1.37 inches tall and holds
/// five paragraphs of caveats: LibreOffice prints its first sentence and we printed all of it,
/// 1556 words against 1504.
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

        // Calc's own condition: the clip applies only when the text really is taller than the box,
        // and while it applies the vertical adjustment is suppressed as well, so an overflowing
        // centred body starts at the top rather than being centred on a block that does not fit
        // (`bClipVerticalTextOverflow`, svdotextdecomposition.cxx:581-596).
        bool clipping = text.ClipsVerticalOverflow && total > room;

        if (!clipping)
        {
            if (text.Anchor == SheetShapeAnchor.Middle && room > total) top += (room - total) / 2;
            else if (text.Anchor == SheetShapeAnchor.Bottom && room > total) top += room - total;
        }

        Length pen = top;
        foreach (Line line in lines)
        {
            // Wholly inside or not drawn: LibreOffice accepts "only text portions completely
            // inside" the clip range and discards the rest outright, so an overflowing line is
            // absent from the output rather than half-drawn (svdoutl.hxx:56-59).
            if (clipping && pen + line.Height > top + room) break;

            // A blank paragraph carries no piece and only advances the pen, which is what keeps
            // the gap a text box puts between its blocks.
            if (line.Pieces.Count > 0)
            {
                Length x = line.Alignment switch
                {
                    SheetShapeAlignment.Centre => left + ((available - line.Width) / 2),
                    SheetShapeAlignment.Right => right - line.Width,
                    _ => left,
                };

                // The baseline is shared by every piece of the line and sits at the deepest
                // ascent any of them needs, so a large run does not drag a small one off it.
                Length baseline = pen + line.Ascent;
                foreach (BandRun piece in line.Pieces)
                {
                    sink.DrawGlyphRun(piece.At(new DocPoint(x, baseline)), Paint.Solid(Colour.Black));
                    x += piece.Width;
                }
            }

            pen += line.Height;
        }
    }

    /// <summary>The size and face one stretch of a paragraph is set in.</summary>
    private readonly record struct Format(Length Size, string? Family);

    /// <summary>
    /// One laid-out line: the shaped stretches it is made of, its width, the ascent its pieces
    /// share, the height it advances, and its alignment.
    /// </summary>
    private readonly record struct Line(
        IReadOnlyList<BandRun> Pieces,
        Length Width,
        Length Ascent,
        Length Height,
        SheetShapeAlignment Alignment);

    /// <summary>Shapes every paragraph into the lines it wraps to.</summary>
    private static List<Line> Lines(SheetShapeText text, Length available, double scale)
    {
        List<Line> lines = [];
        bool anyInk = false;

        foreach (SheetShapeParagraph paragraph in text.Paragraphs)
        {
            string body = paragraph.Text;
            Format[] formats = Formats(paragraph, body.Length, scale);

            if (body.Length == 0)
            {
                Format blank = Blank(paragraph, scale);
                lines.Add(new Line(
                    [],
                    Length.Zero,
                    SheetBandText.AscentAt(blank.Size, blank.Family),
                    SheetBandText.ChartLineHeightAt(blank.Size, blank.Family),
                    paragraph.Alignment));
                continue;
            }

            foreach ((int start, int end) in Wrap(body, formats, available, text.Wraps))
            {
                Line line = Compose(body, formats, start, end, paragraph.Alignment);
                lines.Add(line);
                if (line.Pieces.Count > 0) anyInk = true;
            }
        }

        // Nothing shaped means no face resolved, and a column of blank advances is not worth
        // walking: the caller draws nothing rather than reserving room for it.
        return anyInk ? lines : [];
    }

    /// <summary>The format of every character of the paragraph, with the zoom already in it.</summary>
    private static Format[] Formats(SheetShapeParagraph paragraph, int length, double scale)
    {
        Format[] formats = new Format[length];
        int at = 0;

        foreach (SheetShapeRun run in paragraph.Runs)
        {
            Format format = Scaled(run, scale);
            for (int i = 0; i < run.Text.Length && at < length; i++) formats[at++] = format;
        }

        // A model that named fewer characters than the text holds would leave the tail unset, and
        // a zero size shapes nothing at all; the last run's format is the honest continuation.
        Format tail = at > 0 ? formats[at - 1] : Scaled(default, scale);
        while (at < length) formats[at++] = tail;

        return formats;
    }

    /// <summary>The format a paragraph holding no text reserves its line at.</summary>
    private static Format Blank(SheetShapeParagraph paragraph, double scale)
        => Scaled(paragraph.Runs.Count > 0 ? paragraph.Runs[0] : default, scale);

    private static Format Scaled(SheetShapeRun run, double scale)
    {
        Length size = run.Size > Length.Zero ? run.Size : SheetShapeText.DefaultSize;
        return new Format(size * scale, run.Family);
    }

    /// <summary>
    /// Breaks one paragraph into the character ranges its lines cover.
    /// </summary>
    /// <remarks>
    /// Words are separated by single spaces, so a line is a contiguous range of the paragraph and
    /// every character keeps the format its run gave it. Splitting into strings and rejoining them
    /// would lose that correspondence, which is the whole reason the ranges are carried instead.
    /// </remarks>
    private static List<(int Start, int End)> Wrap(
        string body, Format[] formats, Length available, bool wraps)
    {
        if (!wraps) return [(0, body.Length)];

        List<(int Start, int End)> words = [];
        int from = 0;
        for (int i = 0; i <= body.Length; i++)
        {
            if (i == body.Length || body[i] == ' ')
            {
                words.Add((from, i));
                from = i + 1;
            }
        }

        List<(int Start, int End)> lines = [];
        int start = words[0].Start;
        int taken = 0;

        for (int i = 0; i < words.Count; i++)
        {
            if (taken > 0 && Width(body, formats, start, words[i].End) > available)
            {
                lines.Add((start, words[i - 1].End));
                start = words[i].Start;
                taken = 1;
                continue;
            }

            taken++;
        }

        lines.Add((start, words[^1].End));
        return lines;
    }

    /// <summary>
    /// Shapes one line's range into the maximal stretches that share a format.
    /// </summary>
    private static Line Compose(
        string body, Format[] formats, int start, int end, SheetShapeAlignment alignment)
    {
        List<BandRun> pieces = [];
        Length width = Length.Zero;
        Length ascent = Length.Zero;
        Length height = Length.Zero;

        int at = start;
        while (at < end)
        {
            int stop = at + 1;
            while (stop < end && formats[stop] == formats[at]) stop++;

            Format format = formats[at];

            // The line's metrics come from the formats it spans and not from the pieces that
            // shaped, so a face that cannot be resolved loses its ink and not the line's height.
            Length pieceAscent = SheetBandText.AscentAt(format.Size, format.Family);
            Length pieceHeight = SheetBandText.ChartLineHeightAt(format.Size, format.Family);
            if (pieceAscent > ascent) ascent = pieceAscent;
            if (pieceHeight > height) height = pieceHeight;

            if (SheetBandText.Shape(body[at..stop], format.Size, format.Family) is { } run)
            {
                pieces.Add(run);
                width += run.Width;
            }

            at = stop;
        }

        return new Line(pieces, width, ascent, height, alignment);
    }

    /// <summary>How wide one range of the paragraph is, measured stretch by stretch.</summary>
    private static Length Width(string body, Format[] formats, int start, int end)
    {
        Length width = Length.Zero;
        int at = start;

        while (at < end)
        {
            int stop = at + 1;
            while (stop < end && formats[stop] == formats[at]) stop++;

            if (SheetBandText.Shape(body[at..stop], formats[at].Size, formats[at].Family) is { } run)
            {
                width += run.Width;
            }

            at = stop;
        }

        return width;
    }
}

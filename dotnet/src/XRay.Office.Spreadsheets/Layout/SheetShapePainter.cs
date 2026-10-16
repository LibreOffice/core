using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;

namespace XRay.Office.Spreadsheets.Layout;

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
/// <strong>A line separator inside one <c>a:t</c> ends the paragraph.</strong> DrawingML has
/// <c>a:br</c> for a break, but an authoring tool may also put a bare <c>U+000A</c> in the run's
/// own characters, and LibreOffice never sees it as a character: every importer hands its string
/// to the EditEngine, and <c>ImpEditEngine::ImpInsertText</c> normalises the line ends to LF
/// (<c>convertLineEnd</c>), walks the string from separator to separator and calls
/// <c>ImpInsertParaBreak</c> at each one, with <c>// Start == End =&gt; empty line</c> for two in a
/// row (<c>editeng/source/editeng/impedit2.cxx</c>:2864-2983). So <c>…of Excel.\n\nIf the
/// shape…</c> is three paragraphs, the middle one empty, and it is a line <em>taller</em> than the
/// same characters run together. Shaping the separator instead — which this did — drops it to a
/// zero-width glyph and joins the sentences into <c>Excel.If</c>.
/// </para>
/// <para>
/// That is what decides whether the clip below fires at all, which is why the two belong in one
/// note: on <c>070_Equipment_inventory_list…xlsx</c>'s three slicer placeholders the notice is
/// six line slots with the break and five without, the boxes hold five, and 26.2.4.2 therefore
/// drops the last line of each while we drew all of it. Reading it as a lost paragraph
/// <em>and</em> a lost clip counts one cause twice.
/// </para>
/// <para>
/// <strong><c>horzOverflow</c> is not the horizontal sibling of that clip; nothing reads it.</strong>
/// <c>TextBodyPropertiesContext</c> stores the attribute as a string
/// (<c>oox/source/drawingml/textbodypropertiescontext.cxx</c>:83) which is put in a grab bag for
/// round-tripping (<c>oox/source/drawingml/shape.cxx</c>:2189) and re-exported
/// (<c>oox/source/export/drawingml.cxx</c>:4141, 4379) and never consulted by any layout. Only
/// <c>vertOverflow</c> sets a property — <c>PROP_TextClipVerticalOverflow</c>, <c>:85-97</c>.
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

        // The preset's own text rectangle rather than the anchor's box -- see
        // `SheetShapeText.Preset`. A shape stating no preset, and every BIFF one, gets the box.
        DocRect frame = Frame(text, box);
        if (frame.Width <= Length.Zero || frame.Height <= Length.Zero) return;

        Length left = frame.X + (text.LeftInset * scale);
        Length right = frame.X + frame.Width - (text.RightInset * scale);
        Length available = right - left;
        if (available <= Length.Zero) return;

        List<Line> lines = Lines(text, available, scale);
        if (lines.Count == 0) return;

        Length total = Length.Zero;
        foreach (Line line in lines) total += line.Height;

        Length top = frame.Y + (text.TopInset * scale);
        Length room = frame.Height - (text.TopInset * scale) - (text.BottomInset * scale);

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

        Length bottom = top + room;
        Length pen = top;
        foreach (Line line in lines)
        {
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
                foreach (Piece piece in line.Pieces)
                {
                    // Wholly inside or not drawn, and it is the *portion* that is tested rather
                    // than the line: LibreOffice keeps one only when its start position and both
                    // corners of its glyph bounding rectangle lie inside the clip range, and
                    // discards the rest outright rather than drawing the part that fits
                    // (`TextHierarchyBreakupBlockText::processDrawPortionInfo`,
                    // svx/source/svdraw/svdoutl.cxx:120-160). A dropped portion still advances the
                    // pen, because nothing reflows around it.
                    if (!clipping || Fits(piece.Run, baseline, top, bottom))
                    {
                        sink.DrawGlyphRun(
                            piece.Run.At(new DocPoint(x, baseline)),
                            Paint.Solid(piece.Colour ?? Colour.Black));
                    }

                    x += piece.Run.Width;
                }
            }

            pen += line.Height;
        }
    }

    /// <summary>Whether a portion's baseline and ink both sit inside the clip range.</summary>
    /// <remarks>
    /// Three tests, because the reference makes three: the start position, then the top left of
    /// the text bound rectangle, then its bottom right, each rejected on its own. The horizontal
    /// half of the range is unbounded, so only the vertical one is asked here.
    /// </remarks>
    private static bool Fits(BandRun piece, Length baseline, Length top, Length bottom)
    {
        if (baseline < top || baseline > bottom) return false;

        (Length above, Length below) = piece.Ink;
        return baseline - above >= top && baseline + below <= bottom;
    }

    /// <summary>The rectangle inside the shape that its text is laid out in.</summary>
    /// <remarks>
    /// <para>
    /// The box arrives already scaled by the print zoom and a preset's geometry is linear in its
    /// size, so evaluating the preset against the scaled box gives the scaled text rectangle with
    /// no second conversion.
    /// </para>
    /// <para>
    /// A preset this evaluator does not know, and a shape that states none at all, both answer the
    /// whole box -- which is what LibreOffice falls back to as well, and what every BIFF shape gets
    /// since the Escher path carries no preset name.
    /// </para>
    /// </remarks>
    private static DocRect Frame(SheetShapeText text, DocRect box)
    {
        if (text.Preset is not { Length: > 0 } preset) return box;

        DocSize size = new(box.Width, box.Height);
        if (CustomShapeGeometry.Preset(preset, size, text.Adjustments) is not { } geometry)
            return box;

        DocRect rectangle = geometry.TextRectangle;
        if (rectangle.Width <= Length.Zero || rectangle.Height <= Length.Zero) return box;

        return new DocRect(
            box.X + rectangle.X, box.Y + rectangle.Y, rectangle.Width, rectangle.Height);
    }

    /// <summary>One shaped stretch of a line, and the ink it is drawn in.</summary>
    /// <remarks>
    /// The colour rides beside the shaped run rather than inside it because shaping is a text
    /// question and the ink is not: <c>SheetBandText.Shape</c> answers the same glyphs for
    /// two runs that differ only in colour, and a <see cref="Format"/> that differs in colour
    /// alone must still end the stretch, because <c>Compose</c> cuts on the format and the two
    /// halves are drawn in different ink.
    /// </remarks>
    /// <param name="Run">The shaped glyphs.</param>
    /// <param name="Colour">The ink the run stated, or null for the drawing layer's black.</param>
    private readonly record struct Piece(BandRun Run, Colour? Colour);

    /// <summary>The size, face, weight and ink one stretch of a paragraph is set in.</summary>
    private readonly record struct Format(Length Size, string? Family, bool Bold, Colour? Colour);

    /// <summary>
    /// One laid-out line: the shaped stretches it is made of, its width, the ascent its pieces
    /// share, the height it advances, and its alignment.
    /// </summary>
    private readonly record struct Line(
        IReadOnlyList<Piece> Pieces,
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

            foreach ((int from, int to) in Blocks(body))
            {
                if (to <= from)
                {
                    Format blank = At(formats, from) ?? Blank(paragraph, scale);
                    lines.Add(new Line(
                        [],
                        Length.Zero,
                        SheetBandText.AscentAt(blank.Size, blank.Family, blank.Bold),
                        SheetBandText.ShapeLineHeightAt(blank.Size, blank.Family, blank.Bold),
                        paragraph.Alignment));
                    continue;
                }

                foreach ((int start, int end) in Wrap(body, formats, from, to, available, text.Wraps))
                {
                    Line line = Compose(body, formats, start, end, paragraph.Alignment);
                    lines.Add(line);
                    if (line.Pieces.Count > 0) anyInk = true;
                }
            }
        }

        // Nothing shaped means no face resolved, and a column of blank advances is not worth
        // walking: the caller draws nothing rather than reserving room for it.
        return anyInk ? lines : [];
    }

    /// <summary>
    /// The character ranges the paragraph's own line separators cut it into.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One range per paragraph the EditEngine would end up holding, in its own order:
    /// <c>ImpInsertText</c> inserts the text up to the next separator, and inserts a paragraph
    /// break whenever the separator was not the end of the string
    /// (<c>editeng/source/editeng/impedit2.cxx</c>:2980-2982). A body holding no separator is one
    /// range and an empty body is one empty range, so a paragraph that never had one behaves
    /// exactly as it did.
    /// </para>
    /// <para>
    /// <c>CR LF</c> and a bare <c>CR</c> are separators as well, because
    /// <c>convertLineEnd(rStr, LINEEND_LF)</c> runs first and turns both into one <c>LF</c>
    /// (<c>:2865</c>). Ranges rather than strings, for the reason
    /// <see cref="Wrap(string, Format[], int, int, Length, bool)"/> gives: every character keeps
    /// the format its run gave it, and splitting into strings would lose that correspondence.
    /// </para>
    /// </remarks>
    private static List<(int From, int To)> Blocks(string body)
    {
        List<(int From, int To)> blocks = [];
        int from = 0;

        for (int i = 0; i < body.Length; i++)
        {
            if (body[i] is not ('\n' or '\r')) continue;

            blocks.Add((from, i));
            if (body[i] == '\r' && i + 1 < body.Length && body[i + 1] == '\n') i++;
            from = i + 1;
        }

        blocks.Add((from, body.Length));
        return blocks;
    }

    /// <summary>The format at one position, or null where the paragraph has no characters.</summary>
    /// <remarks>
    /// An empty block sits <em>on</em> its own separator, so the format there is the one the run
    /// carrying the separator states — which is what a blank paragraph should reserve its line at,
    /// and the same rule <c>a:endParaRPr</c> states for a paragraph that never had any characters.
    /// A block starting past the end is the trailing separator's, and takes the last format.
    /// </remarks>
    private static Format? At(Format[] formats, int index)
    {
        if (formats.Length == 0) return null;
        return formats[index < formats.Length ? index : ^1];
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

    /// <remarks>
    /// The face is defaulted here rather than left null, because null means "the furniture's own"
    /// to <c>SheetBandText</c> — the workbook's default *cell* font — and a shape is not furniture.
    /// Its text belongs to the drawing layer's item pool, whose default is
    /// <see cref="SheetShapeText.DefaultFamily"/>; the two are different fonts and the same
    /// workbook uses both.
    /// </remarks>
    private static Format Scaled(SheetShapeRun run, double scale)
    {
        Length size = run.Size > Length.Zero ? run.Size : SheetShapeText.DefaultSize;
        string family = string.IsNullOrWhiteSpace(run.Family)
            ? SheetShapeText.DefaultFamily
            : run.Family;

        return new Format(size * scale, family, run.Bold, run.Colour);
    }

    /// <summary>
    /// Breaks one of <see cref="Blocks"/>' ranges into the character ranges its lines cover.
    /// </summary>
    /// <remarks>
    /// Words are separated by single spaces, so a line is a contiguous range of the paragraph and
    /// every character keeps the format its run gave it. Splitting into strings and rejoining them
    /// would lose that correspondence, which is the whole reason the ranges are carried instead —
    /// and the same reason the block arrives as <paramref name="from"/> and <paramref name="to"/>
    /// into the paragraph's own text rather than as a substring of it.
    /// </remarks>
    /// <param name="body">The whole paragraph's characters.</param>
    /// <param name="formats">The format of each of them.</param>
    /// <param name="from">Where this block starts in <paramref name="body"/>.</param>
    /// <param name="to">Where it ends, exclusive.</param>
    /// <param name="available">The width a line may occupy.</param>
    /// <param name="wraps">False for a body stating <c>wrap="none"</c>.</param>
    private static List<(int Start, int End)> Wrap(
        string body, Format[] formats, int from, int to, Length available, bool wraps)
    {
        if (!wraps) return [(from, to)];

        List<(int Start, int End)> words = [];
        int at = from;
        for (int i = from; i <= to; i++)
        {
            if (i == to || body[i] == ' ')
            {
                words.Add((at, i));
                at = i + 1;
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
        List<Piece> pieces = [];
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
            Length pieceAscent = SheetBandText.AscentAt(format.Size, format.Family, format.Bold);
            Length pieceHeight =
                SheetBandText.ShapeLineHeightAt(format.Size, format.Family, format.Bold);
            if (pieceAscent > ascent) ascent = pieceAscent;
            if (pieceHeight > height) height = pieceHeight;

            if (SheetBandText.Shape(body[at..stop], format.Size, format.Family, format.Bold)
                    is { } run)
            {
                pieces.Add(new Piece(run, format.Colour));
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

            if (SheetBandText.Shape(
                    body[at..stop], formats[at].Size, formats[at].Family, formats[at].Bold)
                    is { } run)
            {
                width += run.Width;
            }

            at = stop;
        }

        return width;
    }
}

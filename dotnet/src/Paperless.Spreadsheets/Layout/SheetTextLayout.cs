using System.Collections.Concurrent;
using Paperless.Core.Extraction;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Numbers;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;

namespace Paperless.Spreadsheets.Layout;

/// <summary>What one cell's text needs to know about the sheet around it.</summary>
/// <param name="Scale">The print zoom as a factor; everything drawn is multiplied by it.</param>
/// <param name="IsAvailable">
/// Whether the cell at a row and column is free for a neighbour's text to spill into — empty, and
/// neither merged nor overlapped. <c>ScOutputData::IsAvailable</c>,
/// <c>sc/source/ui/view/output2.cxx:1178</c>.
/// </param>
/// <param name="ColumnWidth">The printed width of a column, already scaled.</param>
/// <param name="BlockLeft">
/// The left edge of the block of columns being printed, scaled — Calc's <c>mnScrX</c>.
/// </param>
/// <param name="BlockRight">Its right edge, Calc's <c>mnScrX + mnScrW</c>.</param>
internal readonly record struct SheetTextContext(
    double Scale,
    Func<int, int, bool> IsAvailable,
    Func<int, Length> ColumnWidth,
    Length BlockLeft = default,
    Length BlockRight = default);

/// <summary>One cell as it is about to be drawn.</summary>
/// <param name="Text">The text the number format produced.</param>
/// <param name="Value">Its typed value; null for a blank cell and a string for a text one.</param>
/// <param name="Format">Its resolved text format.</param>
/// <param name="Row">The zero-based row.</param>
/// <param name="Column">The zero-based column.</param>
/// <param name="Box">Where the cell sits on the page, scaled.</param>
/// <param name="Portions">
/// The stretches its text is split into when they are not all in the cell's own format, or null
/// when they are. See <see cref="SheetRichText"/>.
/// </param>
/// <param name="IsField">
/// Whether the cell's whole content is one EditEngine field — a hyperlink. A field is drawn as one
/// indivisible portion, so it neither breaks across lines nor loses its tail to a narrow column.
/// See <see cref="SheetLayout.HoldsField"/>.
/// </param>
internal readonly record struct SheetCellText(
    string Text,
    object? Value,
    SheetCellFormat Format,
    int Row,
    int Column,
    DocRect Box,
    IReadOnlyList<SheetTextPortion>? Portions = null,
    bool IsField = false);

/// <summary>
/// Places and draws one cell's text.
/// </summary>
/// <remarks>
/// <para>
/// A port of Calc's own text output, <c>ScOutputData::LayoutStringsImpl</c>
/// (<c>sc/source/ui/view/output2.cxx:1595-2290</c>), which states the alignment, overflow,
/// clipping and <c>###</c> rules directly. The order it does things in is load-bearing and is
/// kept: resolve the alignment from the cell's <em>type</em>, work out how much room the text
/// needs, widen that room into empty neighbours, then shrink, wrap or hash whatever still does
/// not fit — each step reading the clip flags the step before it set.
/// </para>
/// <para>
/// <strong>Four margins of twenty twips each, and they are not decoration.</strong>
/// <c>ATTR_MARGIN</c>'s default is <c>SvxMarginItem(20, 20, 20, 20)</c>
/// (<c>svx/source/items/algitem.cxx:123</c>), and all four are measurable in a reference
/// rendering: a sheet with a two-centimetre page margin starts its first column's text at
/// 57.7 pt rather than 56.7, and bottom-aligns its baseline one point above the row's bottom
/// rather than on it.
/// </para>
/// <para>
/// <strong>A cell's line height is not the word processor's.</strong> Calc builds it from the
/// font metric alone — ascent plus descent, no external leading (<c>output2.cxx:734</c>) — where
/// Writer adds the line gap. Ten-point Liberation Sans wraps at a pitch of 11.17 pt here and
/// 11.50 pt there, so a three-line cell drawn with Writer's pitch has its last line a point low.
/// </para>
/// </remarks>
internal static class SheetTextLayout
{
    /// <summary>The margin between a cell's edge and its text, on all four sides.</summary>
    public static readonly Length CellMargin = Length.FromTwips(20);

    /// <summary>How many times the shrink loop is allowed to try again.</summary>
    /// <remarks><c>SC_SHRINKAGAIN_MAX</c>; each attempt takes a further tenth off the scale.</remarks>
    private const int ShrinkAttempts = 7;

    /// <summary>What a numeric cell that will not fit draws instead of its number.</summary>
    private const string HashText = "###";

    private static readonly ConcurrentDictionary<string, ParagraphLayouter> Layouters =
        new(StringComparer.Ordinal);

    /// <summary>
    /// Whether a cell is free for a neighbour's text to run through.
    /// </summary>
    /// <remarks>
    /// A cell object is not the same thing as a cell with something in it. Both readers
    /// materialise the gaps inside a row so that extracted text keeps its columns, so a row
    /// holding A1 and D1 carries four cells of which two are blank — and a test of "is there a
    /// cell here" stops a long string at B1 where Calc runs it through to D1
    /// (<c>ScOutputData::IsEmptyCellText</c>, <c>output2.cxx:1178</c>).
    /// </remarks>
    /// <param name="cell">The neighbouring cell, or null when the sheet has nothing there.</param>
    public static bool IsAvailable(ContentTableCell? cell)
        => cell is null || (cell.Value is null && cell.GetText().Length == 0);

    /// <summary>Draws one cell's text, or nothing when there is none to draw.</summary>
    /// <param name="sink">Where to draw.</param>
    /// <param name="context">The sheet around the cell.</param>
    /// <param name="cell">The cell.</param>
    public static void Draw(IDrawingSink sink, in SheetTextContext context, in SheetCellText cell)
    {
        ArgumentNullException.ThrowIfNull(sink);

        if (cell.Text.Length == 0) return;
        if (SheetFonts.For(cell.Format) is not { } face) return;

        Placement placement = Place(context, cell, face);
        if (placement.Lines.Count == 0) return;
        if (IsOutside(context, placement)) return;

        // The cell's own colour is the fallback rather than the answer: a rich cell's portions
        // carry theirs, and a plain one's segment carries none so that the two paths emit the
        // same paint for the same cell.
        Colour fallback = cell.Format.Colour;

        if (cell.Format.IsRotated)
        {
            DrawRotated(sink, context, cell, placement, fallback);
            return;
        }

        // Whatever a shortened string still hangs over the edge, and every wrapped line taller
        // than its row, is cut off rather than drawn across the neighbour. Calc sets the clip
        // region to the same rectangle it aligned in (output2.cxx:2126) and only when it is
        // needed, which is worth keeping: a clip per cell would put two operators around every
        // run in the file.
        bool clipped = placement.Clipped;
        if (clipped)
        {
            sink.Save();
            sink.ClipPath(Rectangle(new DocRect(
                placement.Left,
                placement.Top,
                placement.Right - placement.Left,
                placement.Bottom - placement.Top)));
        }

        try
        {
            foreach (PlacedLine line in placement.Lines)
            {
                foreach ((GlyphRun run, Colour? colour) in
                         line.Run.At(new DocPoint(line.X, line.Baseline)))
                {
                    // An empty paragraph's line carries a segment for its metrics and no glyphs;
                    // it has taken its height already and there is nothing to draw or underline.
                    if (run.Glyphs.Count == 0) continue;

                    sink.DrawGlyphRun(run, Paint.Solid(colour ?? fallback));
                }

                Decorate(sink, cell.Format, face, line, fallback);
            }
        }
        finally
        {
            if (clipped) sink.Restore();
        }
    }

    /// <summary>
    /// Whether the room the cell was given falls entirely outside the block being printed.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>bOutside</c> (<c>output2.cxx:2037</c>), and it is the whole reason a page does not
    /// carry every neighbour of its own first column. Calc's string loop starts one column
    /// <em>before</em> the block so that a long string reaching in from the left is drawn — but
    /// it then asks of every cell whether what it occupies overlaps the block at all, and draws
    /// nothing when it does not. A short string in that column is therefore skipped and a long
    /// one is not, because only the long one's output area, widened through its empty
    /// neighbours, reaches the paper.
    /// </para>
    /// <para>
    /// Measured: <c>ExampleWhiteListData.xlsx</c> drew twenty part numbers off the left edge of
    /// its last two pages — <strong>838 words against the reference's 821</strong> — because
    /// every one of them was the nearest cell left of a band and none of them spilled into it.
    /// </para>
    /// <para>
    /// Calc's rectangle is inclusive at the right, so a cell ending exactly where the block
    /// begins is outside it; hence the <c>&lt;=</c>.
    /// </para>
    /// </remarks>
    private static bool IsOutside(in SheetTextContext context, in Placement placement)
        => context.BlockRight > context.BlockLeft
           && (placement.Right <= context.BlockLeft || placement.Left >= context.BlockRight);

    /// <summary>
    /// Draws the rules a font asks for under and through one line of a cell.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A cell's underline is a font property in all three formats and is drawn by the output
    /// device rather than shaped, so it is a filled rectangle under the run and not a glyph. The
    /// offset and the thickness come from the face's own <c>post</c> and <c>OS/2</c> tables
    /// through <see cref="LineSpacing.ResolveDecorations(OpenTypeFace, LineMetrics)"/>, which is the same resolution and the
    /// same fallbacks the rest of the project uses — a font that declares neither would otherwise
    /// draw a zero-thickness line, which is to say none.
    /// </para>
    /// <para>
    /// Excel's two accounting underline styles run the full width of the <em>cell</em> rather
    /// than of the text; both are folded onto their plain counterparts here, so an accounting
    /// underline is as wide as its number. See <see cref="SheetUnderline"/>.
    /// </para>
    /// <para>
    /// Per line and per cell rather than per portion: a rich cell mixing an underlined run with a
    /// plain one underlines the whole line. The portions carry the format that would answer
    /// properly, and the run geometry to place a partial rule with does not exist yet.
    /// </para>
    /// </remarks>
    private static void Decorate(
        IDrawingSink sink, SheetCellFormat format, SheetFace face, PlacedLine line, Colour colour)
    {
        if (format.Underline == SheetUnderline.None && !format.IsStruckThrough) return;

        Length size = line.Run.Size;
        Length width = line.Run.Width;
        if (size <= Length.Zero || width <= Length.Zero) return;

        int unitsPerEm = face.Face.UnitsPerEm > 0 ? face.Face.UnitsPerEm : 1000;
        FontVerticalMetrics metrics = LineSpacing.ResolveDecorations(face.Face, face.Metrics);

        Length Scaled(int designUnits) => size * ((double)designUnits / unitsPerEm);

        if (format.Underline != SheetUnderline.None)
        {
            Length thickness = Scaled(metrics.UnderlineThickness);

            // The font records the underline's offset as negative below the baseline.
            Length top = line.Baseline - Scaled(metrics.UnderlinePosition);
            Rule(sink, line.X, top, width, thickness, colour);

            if (format.Underline == SheetUnderline.DoubleLine)
                Rule(sink, line.X, top + (thickness * 2), width, thickness, colour);
        }

        if (format.IsStruckThrough)
        {
            Length thickness = Scaled(metrics.StrikeoutThickness);
            Rule(sink, line.X, line.Baseline - Scaled(metrics.StrikeoutPosition),
                 width, thickness, colour);
        }
    }

    /// <summary>One horizontal rule, filled rather than stroked so its thickness is exact.</summary>
    private static void Rule(
        IDrawingSink sink, Length x, Length top, Length width, Length thickness, Colour colour)
    {
        if (thickness <= Length.Zero) return;

        sink.FillPath(Rectangle(new DocRect(x, top, width, thickness)), Paint.Solid(colour));
    }

    private static GraphicsPath Rectangle(DocRect rect)
        => new GraphicsPath()
           .MoveTo(new DocPoint(rect.X, rect.Y))
           .LineTo(new DocPoint(rect.X + rect.Width, rect.Y))
           .LineTo(new DocPoint(rect.X + rect.Width, rect.Y + rect.Height))
           .LineTo(new DocPoint(rect.X, rect.Y + rect.Height))
           .Close();

    // ------------------------------------------------------------------------------ placement

    private readonly record struct PlacedLine(SheetTextRun Run, Length X, Length Baseline);

    /// <summary>Where a cell's lines ended up, and what has to be cut off around them.</summary>
    /// <param name="Lines">The placed lines.</param>
    /// <param name="Clipped">True when the text still runs past what it was given.</param>
    /// <param name="Left">The left edge of the room it was given, neighbours included.</param>
    /// <param name="Right">The right edge of the same.</param>
    /// <param name="Top">The top of the clip, which is the cell's or the text's, whichever is higher.</param>
    /// <param name="Bottom">Its bottom, likewise.</param>
    private readonly record struct Placement(
        List<PlacedLine> Lines, bool Clipped = false,
        Length Left = default, Length Right = default,
        Length Top = default, Length Bottom = default);

    private static Placement Place(in SheetTextContext context, in SheetCellText cell, SheetFace face)
    {
        SheetCellFormat format = cell.Format;
        double scale = context.Scale;

        // Both snapped to the drawing device's hundredth of a millimetre, and the two round
        // differently. See SheetDeviceUnits: a font height rounds and a margin truncates, which is
        // what puts a ten-point cell's text at 10.0063 pt and its left edge 0.9921 pt inside the
        // column rather than a whole point.
        Length size = SheetDeviceUnits.SnapFontSize(format.FontSize) * scale;
        Length margin = SheetDeviceUnits.Snap(CellMargin) * scale;

        // The indent counts only when the cell states left or right alignment outright. Calc reads
        // ATTR_INDENT solely in that case (output2.cxx:445), so a General-aligned cell carrying an
        // indent is drawn without one — which looks like a bug in the port until the reference
        // renderer is measured and agrees.
        Length indent = format.Horizontal is SheetHorizontalAlignment.Left
                            or SheetHorizontalAlignment.Right
            ? SheetDeviceUnits.Snap(format.Indent) * scale
            : Length.Zero;

        bool isValue = cell.Value is not null and not string;
        SheetHorizontalAlignment horizontal = Resolve(format.Horizontal, isValue);

        // A field is one indivisible portion, so a cell that is nothing but a hyperlink does not
        // break however narrow its column is — and the clip `bWrapFields` turns on is what keeps
        // it from being drawn across its neighbour instead (output2.cxx:2560-2567, :3239).
        bool breaks = Breaks(format, isValue) && !cell.IsField;
        bool fills = format.Horizontal == SheetHorizontalAlignment.Fill && !breaks;
        bool shrinks = format.ShrinksToFit && !breaks && !fills;

        Length leftTotal = margin + indent;
        Length totalMargin = leftTotal + margin;

        (string text, int fillAt, char fillChar) = Fill(cell);

        // A value is never rich: SpreadsheetML's formatting runs and ODF's spans belong to a
        // string, and a number that showed several fonts would have nowhere to put them once it
        // was re-rendered as ### or in scientific notation.
        IReadOnlyList<SheetTextPortion>? portions =
            !isValue && cell.Portions is { Count: > 0 } stated ? stated : null;

        // Every re-shape below is a range of the cell's own text at a percentage of its size, so
        // that a rich cell keeps its portions lined up with its characters through shortening and
        // wrapping. A plain cell takes the same route with one segment and one face.
        SheetTextRun? ShapeRange(int start, int end, long percent)
        {
            if (portions is not null)
                return SheetText.ShapeRich(text, portions, scale, start, end, percent);

            Length em = percent == 100
                ? size
                : SheetDeviceUnits.SnapFontSize(Length.FromTwips(size.Twips * percent / 100));

            return em > Length.Zero
                ? SheetText.Shape(text[Math.Max(start, 0)..Math.Min(end, text.Length)], face, em)
                : null;
        }

        // How much of its stated size the cell is being drawn at. Only shrink-to-fit moves it, and
        // everything after that has to re-shape at the same percentage or a shortened cell comes
        // back at full size — which is the sort of change that shows as one character more or fewer
        // and nowhere else.
        long percent = 100;

        SheetTextRun? run = ShapeRange(0, text.Length, percent);
        if (run is null) return new Placement([]);

        Area area = OutputArea(
            context, cell, horizontal, run.Width + totalMargin, isValue || fills || shrinks || breaks);

        // A turned or stacked cell never reaches this path in Calc at all. `aVars.IsRotated()` or
        // a stacked orientation sets bUseEditEngine before GetOutputArea is even called
        // (output2.cxx:1800-1803), so DrawStrings skips the cell and `DrawEdit`/`DrawRotated` draw
        // it — and none of what follows is theirs: the EditEngine path neither shrinks a string to
        // fit, nor hashes a number, nor drops the characters it cannot show. It turns the text
        // about the cell's bottom-left corner and lets it run out of the cell, which is the whole
        // point of a 45-degree column heading. Measured on `sheet-rich-text.xlsx`: the reference
        // draws all fifteen characters of "Slanted heading" and Paperless drew eleven, and on the
        // .xls — whose columns LibreOffice's BIFF import makes a shade narrower — nine.
        if (format.IsRotated) area = area.Unclipped();

        Length available = cell.Box.Width - totalMargin;

        // Between the output area and the shrink, which is where Calc does it
        // (output2.cxx:1853): the fill is measured against the cell's own column and not
        // against the room a neighbour lent, so it must not see the widened area — and
        // everything after it re-measures the text it produced.
        if (fillAt >= 0 && portions is null
            && RepeatToFill(text, fillAt, fillChar, face, size, available, run.Width) is { } filled)
        {
            text = filled;
            run = ShapeRange(0, text.Length, percent) ?? run;
        }

        if (shrinks && area.IsClipped && available > Length.Zero && run.Width > Length.Zero)
        {
            (run, percent) = Shrink(ShapeRange, text.Length, run, available);
            if (run.Width <= available) area = area.Unclipped();
        }

        if (isValue && area.IsClipped)
        {
            (run, text) = Hash(cell, face, run.Size, available, run);
            if (run.Width + totalMargin <= area.Width) area = area.Unclipped();
        }

        // A cell holding a no-break space or one of the six other characters of
        // HasEditCharacters is drawn by DrawEditStandard rather than by DrawStrings, and that
        // path clips the string to the cell without dropping a character from it. Everything
        // else about the two agrees — the same GetOutputArea with the same lending from empty
        // neighbours, the same ### for a value that will not fit — so the only thing to skip is
        // the shortening. A cell whose whole content is a hyperlink field is on the same path,
        // for the same reason: it is an EditTextObject rather than a string.
        if (!isValue && !breaks && area.IsClipped
            && !cell.IsField && !HasEditCharacters(text, fillAt))
        {
            run = Shorten(run, text, ShapeRange, percent, horizontal, area);
        }

        List<SheetTextRun> lines = breaks
            ? Wrap(text, portions, face, size, scale, available, ShapeRange, percent)
            : [run];
        if (lines.Count == 0) return new Placement([]);

        // The block's height is the sum of its lines rather than a pitch times a count, because a
        // rich cell's lines are not all the same height: EditEngine makes a line as tall as the
        // tallest portion on it. For a cell in one face the two are the same number.
        Length textHeight = Length.Zero;
        foreach (SheetTextRun line in lines) textHeight += line.LineHeight;

        Length top = VerticalOffset(format.Vertical, cell.Box.Height, textHeight, margin);
        Length y = cell.Box.Y + top;

        List<PlacedLine> placed = new(lines.Count);
        foreach (SheetTextRun line in lines)
        {
            placed.Add(new PlacedLine(
                line,
                Horizontal(horizontal, cell.Box, line.Width, leftTotal, margin + indent, margin),
                y + line.Ascent));
            y += line.LineHeight;
        }

        // The clip never cuts the text vertically. Calc does not clip a printed cell's height
        // either unless the row's height was set by hand ("no vertical clipping when printing
        // cells with optimal height", output2.cxx:2093), and a wrapped cell taller than its row is
        // exactly the case that would lose a line to it.
        Length textTop = Length.Min(cell.Box.Y, placed[0].Baseline - lines[0].Ascent);
        Length textBottom = Length.Max(
            cell.Box.Y + cell.Box.Height, placed[^1].Baseline + lines[^1].Descent);

        return new Placement(placed, area.IsClipped, area.Left, area.Right, textTop, textBottom);
    }

    /// <summary>
    /// Calc's default alignment, which is the cell's <em>type</em> rather than a constant.
    /// </summary>
    /// <remarks>
    /// <c>getAlignmentFromContext</c> (<c>output2.cxx:1443</c>): a value goes right and everything
    /// else left. The right-to-left branch, which turns both round when the text begins with a
    /// right-to-left character, is not reproduced — nothing in the corpus reaches it and it needs
    /// the cell's writing direction, which no reader carries yet.
    /// </remarks>
    private static SheetHorizontalAlignment Resolve(SheetHorizontalAlignment stated, bool isValue)
        => stated switch
        {
            SheetHorizontalAlignment.General => isValue
                ? SheetHorizontalAlignment.Right
                : SheetHorizontalAlignment.Left,

            // Justified and distributed text is placed from the left and stretched; the stretch is
            // not reproduced, so they place as left. Fill repeats from the left as well.
            SheetHorizontalAlignment.Justify or SheetHorizontalAlignment.Distributed
                or SheetHorizontalAlignment.Fill => SheetHorizontalAlignment.Left,

            _ => stated,
        };

    /// <summary>
    /// Whether the cell wraps.
    /// </summary>
    /// <remarks>
    /// Justified alignment forces it, in either direction, and a plain number never takes it:
    /// "disable automatic line breaks for all number formats" (<c>output2.cxx:1834</c>, i#111387),
    /// which is why a wide number in a wrapping column shows <c>###</c> rather than folding onto a
    /// second line. A date or a time is not a plain number format and does wrap.
    /// </remarks>
    internal static bool Breaks(SheetCellFormat format, bool isValue)
    {
        bool breaks = format.Wraps
                      || format.Horizontal is SheetHorizontalAlignment.Justify
                          or SheetHorizontalAlignment.Distributed
                      || format.Vertical is SheetVerticalAlignment.Justify
                          or SheetVerticalAlignment.Distributed;

        return breaks && isValue ? !format.HasPlainNumberFormat : breaks;
    }

    /// <summary>
    /// Whether the cell's text holds a character that sends Calc to the EditEngine.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScDrawStringsVars::HasEditCharacters</c> (<c>output2.cxx:823-847</c>), consulted at
    /// <c>output2.cxx:1812</c> before anything about the output area has been decided. Seven code
    /// points force it — a no-break space, a soft hyphen, a zero-width space, the two bidi marks,
    /// a non-breaking hyphen and a word joiner — and the consequence is not cosmetic:
    /// <c>DrawStrings</c> skips the cell entirely and <c>DrawEditStandard</c> draws it, which
    /// clips the string to the cell and never shortens it. The plain path drops the characters it
    /// cannot show; the EditEngine path leaves them in the text layer behind a clip.
    /// </para>
    /// <para>
    /// The no-break space is excluded when the cell has a repeat directive, which is tdf#122676:
    /// "Ignore CHAR_NBSP (this is thousand separator in any number) if repeat character is set".
    /// The string tested is the cell's <em>display</em> text, so a number whose format groups with
    /// a no-break space reaches this the same way a piece of typed text does.
    /// </para>
    /// </remarks>
    /// <param name="text">The cell's display text.</param>
    /// <param name="fillAt">Where the repeat directive expands, or −1 when there is none.</param>
    internal static bool HasEditCharacters(string text, int fillAt = -1)
    {
        foreach (char c in text)
        {
            switch (c)
            {
                case '\u00A0' when fillAt < 0:  // CHAR_NBSP
                case '\u00AD':                  // CHAR_SHY
                case '\u200B':                  // CHAR_ZWSP
                case '\u200E':                  // CHAR_LRM
                case '\u200F':                  // CHAR_RLM
                case '\u2011':                  // CHAR_NBHY
                case '\u2060':                  // CHAR_WJ
                    return true;
            }
        }

        return false;
    }

    /// <summary>
    /// How many lines a cell's text breaks into at a width.
    /// </summary>
    /// <remarks>
    /// For <see cref="SheetOptimalRowHeights"/>, which needs the count and none of the rest of the
    /// placement — the height it is deriving is what decides where the lines go, so it cannot ask
    /// for them. A hard break starts a line of its own whatever the width is, which is why the
    /// text is split before it is wrapped rather than handed to the layouter whole.
    /// </remarks>
    /// <param name="text">The cell's text.</param>
    /// <param name="face">The face it is set in.</param>
    /// <param name="size">The em size.</param>
    /// <param name="available">The room its lines have, margins already taken off.</param>
    internal static int LineCount(string text, SheetFace face, Length size, Length available)
    {
        if (text.Length == 0) return 0;

        ParagraphLayouter? layouter = null;
        int lines = 0;

        foreach (string paragraph in
                 text.Replace("\r\n", "\n", StringComparison.Ordinal).Split(['\n', '\r']))
        {
            if (paragraph.Length == 0 || available <= Length.Zero)
            {
                lines++;
                continue;
            }

            layouter ??= Layouters.GetOrAdd(
                face.Reference.FaceKey, _ => new ParagraphLayouter(face.Face));

            LaidOutParagraph laid = layouter.Layout(
                paragraph, emSize: size, textAreaWidth: available, options: SheetText.NoKerning);

            lines += Math.Max(1, laid.Lines.Count);
        }

        return lines;
    }

    /// <summary>
    /// The character ranges a cell in several formats breaks into at a width.
    /// </summary>
    /// <remarks>
    /// The ranges rather than the count, because a rich cell's lines are not all the same height:
    /// EditEngine makes a line as tall as the tallest portion on it
    /// (<c>ImpEditEngine::CreateLines</c>, <c>editeng/source/editeng/impedit3.cxx:1516-1519</c>,
    /// over the per-portion maxima <c>RecalcFormatterFontMetrics</c> accumulates at <c>:3160</c>),
    /// so <see cref="SheetOptimalRowHeights"/> has to know which portions sit on which line. The
    /// breaking itself is the same run-aware path <see cref="Wrap"/> takes, so a row is measured
    /// against exactly the lines the cell will be drawn with.
    /// </remarks>
    /// <param name="text">The cell's text.</param>
    /// <param name="portions">The stretches it is split into.</param>
    /// <param name="face">The cell's own face, which names the layouter to break with.</param>
    /// <param name="available">The room its lines have, margins already taken off.</param>
    /// <param name="device">
    /// The grid every portion's em size is rounded onto before it is measured, or null to measure
    /// at the size the file states. Non-null only when a row is being measured rather than drawn:
    /// Calc decides a row's height against a 96 dpi virtual device, which can only set a font at a
    /// whole number of pixels. See <see cref="MetricGrid.ToEmSize"/>.
    /// </param>
    internal static IReadOnlyList<(int Start, int End)> RichLineRanges(
        string text,
        IReadOnlyList<SheetTextPortion> portions,
        SheetFace face,
        Length available,
        MetricGrid? device = null)
    {
        if (text.Length == 0) return [];

        ParagraphLayouter layouter = Layouters.GetOrAdd(
            face.Reference.FaceKey, _ => new ParagraphLayouter(face.Face));

        LaidOutParagraph laid = layouter.Layout(
            Measured(text, portions, scale: 1.0, device), textAreaWidth: available);

        List<(int Start, int End)> ranges = new(laid.Lines.Count);
        foreach (LineBox box in laid.Lines)
            ranges.Add((box.Line.Start, Math.Min(box.Line.End, text.Length)));

        return ranges;
    }

    // --------------------------------------------------------------------------------- fill

    /// <summary>
    /// Where a <c>*c</c> fill directive expands in this cell's text, and with which character.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The reader already produced the cell's text with the directive dropped, because
    /// extraction has no column to fill. Finding the position again means putting the value
    /// through the code a second time with <c>NumberFormatter.FillMarker</c> left in — which is
    /// only done for the formats that carry one, and those are the accounting formats.
    /// </para>
    /// <para>
    /// The re-render is trusted only when it reproduces the text the reader produced. The two
    /// calls resolve the workbook's epoch separately and layout does not carry it, so a date
    /// format with a fill would come back different — and a disagreement must change nothing
    /// rather than replace a correct string with a plausible one.
    /// </para>
    /// </remarks>
    private static (string Text, int At, char Fill) Fill(in SheetCellText cell)
    {
        if (cell.Format.NumberFormat is not { HasFillDirective: true } code) return (cell.Text, -1, '\0');
        if (cell.Value is not double value) return (cell.Text, -1, '\0');

        string marked = NumberFormatter.Format(code, value, keepFillMarkers: true);
        int at = marked.IndexOf(NumberFormatter.FillMarker, StringComparison.Ordinal);
        if (at < 0 || at + 1 >= marked.Length) return (cell.Text, -1, '\0');

        char fill = marked[at + 1];
        string plain = marked.Remove(at, 2);
        return plain == cell.Text ? (plain, at, fill) : (cell.Text, -1, '\0');
    }

    /// <summary>
    /// Pads the fill point with as many copies of the fill character as the column has room for.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScDrawStringsVars::RepeatToFill</c> (<c>output2.cxx:572</c>), including the two
    /// deliberate truncations it marks in its own comments. The character's width is taken from
    /// a twenty-character sample rather than from one copy — "measuring a string containing a
    /// single copy of the repeat char is inaccurate" — and both the width and the count are
    /// truncated towards zero, so the fill can never overrun the column by a rounding.
    /// </para>
    /// <para>
    /// Nothing is added when the space left is no wider than one character: an accounting cell
    /// in a column that only just fits its number shows its symbol against its digits, which is
    /// what Calc draws.
    /// </para>
    /// </remarks>
    /// <returns>The padded text, or null when nothing fits.</returns>
    private static string? RepeatToFill(
        string text, int at, char fill, SheetFace face, Length size, Length available, Length width)
    {
        const int SampleSize = 20;

        if (at > text.Length || available <= Length.Zero) return null;
        if (SheetText.Shape(new string(fill, SampleSize), face, size) is not { } sample) return null;

        double averageWidth = (double)sample.Width.Emu / SampleSize;
        long characterWidth = (long)averageWidth;
        if (characterWidth < 1) return null;

        long spaceToFill = (available - width).Emu;
        if (spaceToFill <= characterWidth) return null;

        int count = (int)(spaceToFill / averageWidth);
        return count <= 0 ? null : text.Insert(at, new string(fill, count));
    }

    // -------------------------------------------------------------------------- output area

    /// <summary>How far the text may run, and how much of it is cut off at either end.</summary>
    private readonly record struct Area(Length Left, Length Right, Length LeftMissing, Length RightMissing)
    {
        public bool LeftClip => LeftMissing > Length.Zero;

        public bool RightClip => RightMissing > Length.Zero;

        public bool IsClipped => LeftClip || RightClip;

        public Length Width => Right - Left;

        public Area Unclipped() => this with { LeftMissing = Length.Zero, RightMissing = Length.Zero };
    }

    /// <summary>
    /// The rectangle the text is allowed to occupy: the cell, widened into empty neighbours.
    /// </summary>
    /// <remarks>
    /// <c>ScOutputData::GetOutputArea</c> (<c>output2.cxx:1204</c>). Three of its conditions
    /// decide the visible behaviour. Only what is missing is walked for, so a left-aligned string
    /// spills to the right and a right-aligned one to the left; the walk stops at the first
    /// neighbour that is not available, which is what clips a long string beside an occupied cell
    /// rather than writing over it; and a value never spills at all — a number too wide shows
    /// <c>###</c> instead, which is the asymmetry that makes a spreadsheet's overflow rule
    /// surprising.
    /// </remarks>
    private static Area OutputArea(
        in SheetTextContext context,
        in SheetCellText cell,
        SheetHorizontalAlignment horizontal,
        Length needed,
        bool blocked)
    {
        Length left = cell.Box.X;
        Length right = cell.Box.X + cell.Box.Width;
        if (needed <= cell.Box.Width) return new Area(left, right, Length.Zero, Length.Zero);

        Length missing = needed - cell.Box.Width;
        Length leftMissing = Length.Zero;
        Length rightMissing = Length.Zero;

        switch (horizontal)
        {
            case SheetHorizontalAlignment.Left:
                rightMissing = missing;
                break;
            case SheetHorizontalAlignment.Right:
                leftMissing = missing;
                break;
            case SheetHorizontalAlignment.Centre:
                leftMissing = missing / 2;
                rightMissing = missing - leftMissing;
                break;
            default:
                break;
        }

        if (!blocked)
        {
            int at = cell.Column;
            while (rightMissing > Length.Zero
                   && at < SheetAddress.MaxColumn
                   && context.IsAvailable(cell.Row, at + 1))
            {
                at++;
                Length add = context.ColumnWidth(at);
                rightMissing -= add;
                right += add;
            }

            at = cell.Column;
            while (leftMissing > Length.Zero && at > 0 && context.IsAvailable(cell.Row, at - 1))
            {
                at--;
                Length add = context.ColumnWidth(at);
                leftMissing -= add;
                left -= add;
            }
        }

        return new Area(left, right, leftMissing, rightMissing);
    }

    // ------------------------------------------------------------------------------- shrink

    /// <summary>
    /// Scales the font down until the text fits, the way Calc's <c>ShrinkEditEngine</c> does.
    /// </summary>
    /// <remarks>
    /// A measure-and-retry rather than a division, and the first guess is a division: the scale is
    /// the integer percentage <c>available × 100 / textWidth</c>, and if that still does not fit
    /// it is cut by a tenth up to seven times (<c>output2.cxx:1864-1885</c>). The integer truncation
    /// is what makes the answer reproducible — <c>sheet-cell-text</c>'s shrunk cell comes out at
    /// 87% of ten point in both renderers, which is 8.70 pt rather than the 8.74 an exact
    /// proportion would give.
    /// </remarks>
    private static (SheetTextRun Run, long Percent) Shrink(
        Func<int, int, long, SheetTextRun?> shape, int length, SheetTextRun run, Length available)
    {
        long percent = available.Emu * 100 / run.Width.Emu;
        if (percent <= 0) return (run, 100);

        SheetTextRun scaled = run;
        long reached = 100;

        for (int attempt = 0; attempt <= ShrinkAttempts; attempt++)
        {
            if (shape(0, length, percent) is not { } shaped) break;

            scaled = shaped;
            reached = percent;
            if (shaped.Width <= available) break;

            percent = percent * 9 / 10;
            if (percent <= 0) break;
        }

        return (scaled, reached);
    }

    // --------------------------------------------------------------------------------- hash

    /// <summary>
    /// What a numeric cell too narrow for its text draws instead.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScDrawStringsVars::SetTextToWidthOrHash</c> (<c>output2.cxx:610</c>), and the rule is
    /// narrower than "a number that does not fit shows hashes". Only a <em>non-</em><c>General</c>
    /// format hashes outright; a <c>General</c> one is re-rendered with as many characters as the
    /// column has digit widths, and only falls back to scientific notation from there. That is why
    /// 123 456 789 012 in a 43 pt column draws as <c>1.2E+11</c> in Calc and not as <c>###</c> —
    /// the trap that cost the most time here, because a port that hashes every value that does not
    /// fit produces plausible output and disagrees with the reference on every wide number.
    /// </para>
    /// <para>
    /// Rendering only. <c>paperless extract</c> keeps reporting the full text, which is a recorded
    /// decision: hashes are a function of a column width that extracted text does not have.
    /// </para>
    /// </remarks>
    private static (SheetTextRun Run, string Text) Hash(
        in SheetCellText cell, SheetFace face, Length size, Length available, SheetTextRun run)
    {
        string text;

        if (cell.Value is double value && cell.Format.HasGeneralFormat)
        {
            Length digit = face.MaxDigitWidthAt(size);
            int characters = digit > Length.Zero ? (int)(available.Emu / digit.Emu) : 0;
            text = SheetGeneralWidth.Render(value, characters);
        }
        else
        {
            text = HashText;
        }

        return (SheetText.Shape(text, face, size) ?? run, text);
    }

    // ------------------------------------------------------------------------------ shorten

    /// <summary>
    /// Drops the characters a clipped string cannot show.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice does this for speed — "if the string is clipped, make it shorter for better
    /// performance since drawing by HarfBuzz is quite expensive" (<c>output2.cxx:2202</c>) — and
    /// it is reproduced because it is <em>visible</em> in the output rather than only faster: the
    /// PDF holds the shortened text, so a cell blocked by its neighbour extracts as the 23
    /// characters that fit rather than the 31 it holds. Reproducing it is what makes a
    /// run-for-run comparison of glyph counts mean anything.
    /// </para>
    /// <para>
    /// The estimate is deliberately crude on both sides — the ratio of visible width to total
    /// width, times the character count, plus one — so it over-keeps rather than under-keeps and
    /// the clip does the rest. Right-aligned text keeps its <em>end</em>, and keeping it needs no
    /// compensating shift: dropping the head of a string leaves every remaining glyph where it
    /// already was, and <see cref="Horizontal"/> is handed the shortened run's own width, so
    /// <c>right − margin − shortened</c> is exactly where the tail was standing. Shifting it right
    /// by the width dropped carried the whole run over the cell's right edge by that much.
    /// </para>
    /// </remarks>
    private static SheetTextRun Shorten(
        SheetTextRun run,
        string text,
        Func<int, int, long, SheetTextRun?> shape,
        long percent,
        SheetHorizontalAlignment horizontal,
        Area area)
    {
        if (run.Width <= Length.Zero || text.Length == 0) return run;

        if (horizontal == SheetHorizontalAlignment.Left && area.RightClip)
        {
            double ratio = (double)(run.Width - area.RightMissing).Emu / run.Width.Emu;
            if (ratio is <= 0.0 or >= 1.0) return run;

            int keep = Math.Clamp((int)(ratio * text.Length) + 1, 1, text.Length);
            return shape(0, keep, percent) ?? run;
        }

        if (horizontal == SheetHorizontalAlignment.Right && area.LeftClip)
        {
            double ratio = (double)(run.Width - area.LeftMissing).Emu / run.Width.Emu;
            if (ratio is <= 0.0 or >= 1.0) return run;

            int keep = Math.Clamp((int)(ratio * text.Length) + 1, 1, text.Length);
            return shape(text.Length - keep, text.Length, percent) ?? run;
        }

        return run;
    }

    // --------------------------------------------------------------------------------- wrap

    /// <summary>
    /// Breaks a wrapping cell into lines.
    /// </summary>
    /// <remarks>
    /// Through the shared <see cref="ParagraphLayouter"/> rather than a second line breaker: the
    /// greedy fill, the trailing-space rule and the "a word too long takes the line alone" rule
    /// are the same in a cell as in a paragraph, and having two implementations of them would mean
    /// two sets of break positions to keep in step. Only the vertical geometry is Calc's own, so
    /// only the line <em>ranges</em> are taken from the result and the pitch is applied here.
    /// </remarks>
    private static List<SheetTextRun> Wrap(
        string text,
        IReadOnlyList<SheetTextPortion>? portions,
        SheetFace face,
        Length size,
        double scale,
        Length available,
        Func<int, int, long, SheetTextRun?> shape,
        long percent)
    {
        SheetTextRun? whole = shape(0, text.Length, percent);
        if (whole is null) return [];

        // A hard break is not a suggestion, so the "it all fits" shortcut cannot take it: the
        // text has to reach the layouter, which breaks at one whatever the width says. Only the
        // shortcut is conditional — a cell with no break in it measures and draws exactly as it
        // did. `LineCount` beside this has always split on the break first, so before this the
        // reserved row height and the drawn lines were computed by two rules that disagreed.
        if (available <= Length.Zero || (whole.Width <= available && !HoldsHardBreak(text)))
            return [whole];

        ParagraphLayouter layouter = Layouters.GetOrAdd(
            face.Reference.FaceKey, _ => new ParagraphLayouter(face.Face));

        // A rich cell breaks against its own runs rather than against one face, through the
        // layouter's run-aware overload: a bold word is wider than the same characters set
        // regular, so measuring the line in the cell's face alone puts the break in the wrong
        // place. The single-face path is left exactly as it was.
        LaidOutParagraph laid = portions is null
            ? layouter.Layout(
                text, emSize: size, textAreaWidth: available, options: SheetText.NoKerning)
            : layouter.Layout(
                Measured(text, portions, scale), textAreaWidth: available);

        List<SheetTextRun> lines = [];
        foreach (LineBox box in laid.Lines)
        {
            // To End rather than to VisibleEnd: Calc's own output shows a line's trailing spaces,
            // so a reference PDF's first wrapped line of "Wrapped text that needs …" holds
            // eighteen glyphs, not the seventeen the visible text has. The break character
            // itself is the one thing dropped — it is Writer's break portion, "zero width, and
            // no glyph", and a cell whose lines carry it would both measure the character's
            // advance into a centred line's width and put it in the PDF's text layer.
            int start = box.Line.Start;
            int full = Math.Min(box.Line.End, text.Length);
            int end = full;
            while (end > start && IsHardBreak(text[end - 1])) end--;

            // A break on its own is an empty paragraph, and an empty paragraph is still a line
            // with a height. It is shaped from the break — a run's ascent and descent come from
            // its face and size rather than from its glyphs — and then emptied, so that the line
            // occupies its pitch without putting a .notdef box on the page or a U+000A in the
            // text layer.
            if (end == start)
            {
                if (shape(start, full, percent) is { } blank) lines.Add(Blank(blank));
                continue;
            }

            if (shape(start, end, percent) is { } shaped) lines.Add(shaped);
        }

        return lines.Count == 0 ? [whole] : lines;
    }

    /// <summary>
    /// The same line with nothing on it: one segment, kept for its metrics, holding no glyphs.
    /// </summary>
    /// <remarks>
    /// The first segment only. A line's height is the tallest thing on it and there is nothing on
    /// this one, so the face and size the paragraph would have been set in is the whole of what
    /// an empty paragraph contributes.
    /// </remarks>
    private static SheetTextRun Blank(SheetTextRun run)
    {
        SheetTextSegment first = run.Segments[0];

        return new SheetTextRun(
            [first with
            {
                Glyphs = [],
                Clusters = [],
                Text = string.Empty,
                Offset = Length.Zero,
                Width = Length.Zero,
            }],
            Length.Zero);
    }

    /// <summary>Whether a cell's text holds a break that starts a line whatever the width is.</summary>
    /// <remarks>
    /// The same two characters <see cref="LineCount"/> splits on, and deliberately no more: the
    /// row height it derives and the lines <see cref="Wrap"/> draws have to be computed from one
    /// rule. Every reader that can put a break inside a cell produces one of these — BIFF's own
    /// U+000A survives <c>ReadRawUnicodeString</c> unchanged, SpreadsheetML writes
    /// <c>&amp;#10;</c>, and ODF's <c>text:line-break</c> is read as <c>'\n'</c>.
    /// </remarks>
    private static bool IsHardBreak(char character) => character is '\n' or '\r';

    /// <inheritdoc cref="IsHardBreak"/>
    private static bool HoldsHardBreak(string text)
    {
        foreach (char c in text)
        {
            if (IsHardBreak(c)) return true;
        }

        return false;
    }

    /// <summary>A rich cell's text, shaped run by run so that it can be broken into lines.</summary>
    private static MeasuredParagraph Measured(
        string text,
        IReadOnlyList<SheetTextPortion> portions,
        double scale,
        MetricGrid? device = null)
    {
        List<FormattedRun> runs = [];
        foreach (SheetTextPortion portion in portions)
        {
            if (SheetFonts.For(portion.Format) is not { } face) continue;

            Length size = SheetText.SizeOf(portion.Format.FontSize, scale, 100);
            if (device is { } grid) size = grid.ToEmSize(size);

            runs.Add(new FormattedRun(
                portion.Start, portion.Length, face.Face, size, SheetText.NoKerning));
        }

        return MeasuredParagraph.Measure(text, runs);
    }

    // ---------------------------------------------------------------------------- placement

    /// <summary>Where a line starts, given its width and the cell's.</summary>
    /// <remarks>
    /// The centre case is not <c>(width - text) / 2</c>: Calc folds the two margins in
    /// asymmetrically — <c>(availWidth - textWidth + leftTotal - rightMargin) / 2</c>
    /// (<c>output2.cxx:2054</c>) — so an indented centred cell drifts right by half its indent.
    /// </remarks>
    private static Length Horizontal(
        SheetHorizontalAlignment horizontal,
        DocRect box,
        Length width,
        Length leftTotal,
        Length rightTotal,
        Length rightMargin)
        => horizontal switch
        {
            SheetHorizontalAlignment.Right => box.X + box.Width - width - rightTotal,
            SheetHorizontalAlignment.Centre => box.X + ((box.Width - width + leftTotal - rightMargin) / 2),
            _ => box.X + leftTotal,
        };

    /// <summary>How far below the cell's top the text block starts.</summary>
    /// <remarks>
    /// <c>Standard</c> is bottom, which Calc settles in one line before any drawing happens
    /// (<c>output2.cxx:348</c>). The centre case again folds the margins in asymmetrically.
    /// </remarks>
    private static Length VerticalOffset(
        SheetVerticalAlignment vertical, Length height, Length textHeight, Length margin)
        => vertical switch
        {
            SheetVerticalAlignment.Top or SheetVerticalAlignment.Justify
                or SheetVerticalAlignment.Distributed => margin,

            SheetVerticalAlignment.Centre => (height + margin - textHeight - margin) / 2,

            _ => height - textHeight - margin,
        };

    // ------------------------------------------------------------------------------ rotation

    /// <summary>
    /// Draws turned or stacked text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Calc turns the text about the cell's bottom-left corner and lets it run out of the cell,
    /// which is what makes a row of 45-degree column headings legible
    /// (<c>ScOutputData::DrawRotated</c>, <c>output2.cxx:4710</c>). Stacked text — Excel's
    /// rotation 255 and ODF's <c>style:direction="ttb"</c> — is a different shape again: one
    /// character per line, centred.
    /// </para>
    /// <para>
    /// What is <em>not</em> reproduced is the rotated cell's effect on its row's height, and the
    /// clipping of rotated text against its neighbours. Both need the rotated bounding box fed
    /// back into the row, which is a change to pagination rather than to drawing.
    /// </para>
    /// </remarks>
    private static void DrawRotated(
        IDrawingSink sink,
        in SheetTextContext context,
        in SheetCellText cell,
        Placement placement,
        Colour fallback)
    {
        if (cell.Format.IsStacked)
        {
            DrawStacked(sink, context, cell, placement, fallback);
            return;
        }

        Length margin = CellMargin * context.Scale;
        DocPoint pivot = new(cell.Box.X + margin, cell.Box.Y + cell.Box.Height - margin);

        sink.Save();
        try
        {
            sink.Transform(About(pivot, -cell.Format.RotationDegrees * Math.PI / 180.0));

            foreach (PlacedLine line in placement.Lines)
            {
                foreach ((GlyphRun run, Colour? colour) in line.Run.At(pivot))
                    sink.DrawGlyphRun(run, Paint.Solid(colour ?? fallback));
            }
        }
        finally
        {
            sink.Restore();
        }
    }

    /// <summary>A rotation about a point rather than about the page's origin.</summary>
    private static AffineTransform About(DocPoint pivot, double radians)
        => AffineTransform.Concat(
            AffineTransform.Concat(
                AffineTransform.Translation(-pivot.X.Emu, -pivot.Y.Emu),
                AffineTransform.Rotation(radians)),
            AffineTransform.Translation(pivot.X.Emu, pivot.Y.Emu));

    /// <summary>Draws one character under the next, which needs no transform at all.</summary>
    private static void DrawStacked(
        IDrawingSink sink,
        in SheetTextContext context,
        in SheetCellText cell,
        Placement placement,
        Colour fallback)
    {
        if (SheetFonts.For(cell.Format) is not { } face) return;

        Length size = placement.Lines[0].Run.Size;
        Length pitch = face.LineHeightAt(size);
        Length y = cell.Box.Y + (CellMargin * context.Scale) + face.AscentAt(size);

        foreach (char character in cell.Text)
        {
            if (SheetText.Shape(character.ToString(), face, size) is not { } glyph) continue;

            Length x = cell.Box.X + ((cell.Box.Width - glyph.Width) / 2);
            foreach ((GlyphRun run, Colour? colour) in glyph.At(new DocPoint(x, y)))
                sink.DrawGlyphRun(run, Paint.Solid(colour ?? fallback));
            y += pitch;
        }
    }
}

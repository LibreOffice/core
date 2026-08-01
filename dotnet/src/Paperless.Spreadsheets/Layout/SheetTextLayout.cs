using System.Collections.Concurrent;
using Paperless.Core.Extraction;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
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
internal readonly record struct SheetTextContext(
    double Scale,
    Func<int, int, bool> IsAvailable,
    Func<int, Length> ColumnWidth);

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
internal readonly record struct SheetCellText(
    string Text,
    object? Value,
    SheetCellFormat Format,
    int Row,
    int Column,
    DocRect Box,
    IReadOnlyList<SheetTextPortion>? Portions = null);

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
                    sink.DrawGlyphRun(run, Paint.Solid(colour ?? fallback));
                }
            }
        }
        finally
        {
            if (clipped) sink.Restore();
        }
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

        bool breaks = Breaks(format, isValue);
        bool fills = format.Horizontal == SheetHorizontalAlignment.Fill && !breaks;
        bool shrinks = format.ShrinksToFit && !breaks && !fills;

        Length leftTotal = margin + indent;
        Length totalMargin = leftTotal + margin;

        string text = cell.Text;

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

        Length shift = Length.Zero;
        if (!isValue && !breaks && area.IsClipped)
        {
            (run, shift) = Shorten(run, text, ShapeRange, percent, horizontal, area);
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
                Horizontal(horizontal, cell.Box, line.Width, leftTotal, margin + indent, margin) + shift,
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
    private static bool Breaks(SheetCellFormat format, bool isValue)
    {
        bool breaks = format.Wraps
                      || format.Horizontal is SheetHorizontalAlignment.Justify
                          or SheetHorizontalAlignment.Distributed
                      || format.Vertical is SheetVerticalAlignment.Justify
                          or SheetVerticalAlignment.Distributed;

        return breaks && isValue ? !format.HasPlainNumberFormat : breaks;
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
    /// the clip does the rest. Right-aligned text keeps its <em>end</em> and is shifted right by
    /// the width it lost, since its pen was placed for the whole string.
    /// </para>
    /// </remarks>
    private static (SheetTextRun Run, Length Shift) Shorten(
        SheetTextRun run,
        string text,
        Func<int, int, long, SheetTextRun?> shape,
        long percent,
        SheetHorizontalAlignment horizontal,
        Area area)
    {
        if (run.Width <= Length.Zero || text.Length == 0) return (run, Length.Zero);

        if (horizontal == SheetHorizontalAlignment.Left && area.RightClip)
        {
            double ratio = (double)(run.Width - area.RightMissing).Emu / run.Width.Emu;
            if (ratio is <= 0.0 or >= 1.0) return (run, Length.Zero);

            int keep = Math.Clamp((int)(ratio * text.Length) + 1, 1, text.Length);
            return (shape(0, keep, percent) ?? run, Length.Zero);
        }

        if (horizontal == SheetHorizontalAlignment.Right && area.LeftClip)
        {
            double ratio = (double)(run.Width - area.LeftMissing).Emu / run.Width.Emu;
            if (ratio is <= 0.0 or >= 1.0) return (run, Length.Zero);

            int keep = Math.Clamp((int)(ratio * text.Length) + 1, 1, text.Length);
            SheetTextRun? shorter = shape(text.Length - keep, text.Length, percent);
            return shorter is null ? (run, Length.Zero) : (shorter, run.Width - shorter.Width);
        }

        return (run, Length.Zero);
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
        if (available <= Length.Zero || whole.Width <= available) return [whole];

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
            // eighteen glyphs, not the seventeen the visible text has.
            if (shape(box.Line.Start, Math.Min(box.Line.End, text.Length), percent) is { } shaped)
                lines.Add(shaped);
        }

        return lines.Count == 0 ? [whole] : lines;
    }

    /// <summary>A rich cell's text, shaped run by run so that it can be broken into lines.</summary>
    private static MeasuredParagraph Measured(
        string text, IReadOnlyList<SheetTextPortion> portions, double scale)
    {
        List<FormattedRun> runs = [];
        foreach (SheetTextPortion portion in portions)
        {
            if (SheetFonts.For(portion.Format) is not { } face) continue;

            runs.Add(new FormattedRun(
                portion.Start,
                portion.Length,
                face.Face,
                SheetText.SizeOf(portion.Format.FontSize, scale, 100),
                SheetText.NoKerning));
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

using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>One cell note, with the cell it hangs off and what it says.</summary>
/// <param name="Column">The cell's column, zero-based.</param>
/// <param name="Row">The cell's row, zero-based.</param>
/// <param name="Text">
/// The note's text as the file states it, including the author line the authoring application
/// puts at the top of one. Excel writes "Author:\n" into the note itself rather than beside it,
/// so nothing here synthesises it and nothing strips it.
/// </param>
public readonly record struct SheetNote(int Column, int Row, string Text);

/// <summary>
/// The notes a sheet holds, and whether they are printed.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A note is not on the page it belongs to.</strong> Excel's "Comments: at end of sheet"
/// prints them on pages of their own after the sheet's content, listed as an address and the note
/// beside it. It reaches Calc as <c>ATTR_PAGE_NOTES</c> — from <c>EXC_SETUP_PRINTNOTES</c> in a
/// BIFF <c>SETUP</c> record (<c>sc/source/filter/excel/xipage.cxx:84</c>, <c>:257</c>) and from
/// <c>cellComments="atEnd"</c> in SpreadsheetML — and <c>ScPrintFunc</c> reads it into
/// <c>aTableParam.bNotes</c> (<c>sc/source/ui/view/printfun.cxx:944</c>).
/// </para>
/// <para>
/// <strong>The order is column-major.</strong> <c>ScPrintFunc::CountNotePages</c> walks the print
/// area's columns outermost and its rows innermost (<c>printfun.cxx:2591-2600</c>), so a sheet
/// with notes in D1, F2 and H2 lists them in that order and not in reading order. Checked against
/// LibreOffice 24.2.7.2's own PDF for <c>Hazard Analysis Template.xls</c>, whose note page runs
/// D1, F2, H2, J2, L1, N2, P2, R2.
/// </para>
/// <para>
/// Notes outside the printed range are not listed at all, which is why this is a sheet-level
/// collection filtered at pagination rather than a filtered list built by the reader: the printed
/// range is not known until the print setup and the used area have both been read.
/// </para>
/// </remarks>
public sealed record SheetNotes
{
    /// <summary>A sheet with no notes.</summary>
    public static SheetNotes Empty { get; } = new();

    /// <summary>The notes, in the order the reader found them.</summary>
    public IReadOnlyList<SheetNote> Items { get; init; } = [];

    /// <summary>True when there is nothing to print.</summary>
    public bool IsEmpty => Items.Count == 0;

    /// <summary>
    /// The notes inside the printed ranges, in the order Calc lists them.
    /// </summary>
    /// <remarks>
    /// One pass per range, in the sheet's own order, because that is what <c>CountNotePages</c>
    /// does for a sheet declaring several print areas — <c>nRepeats</c> is the range count and the
    /// column walk runs once per range (<c>printfun.cxx:2567-2602</c>). Inside one range the order
    /// is column-major.
    /// </remarks>
    /// <param name="ranges">The printed ranges.</param>
    public List<SheetNote> Printed(IReadOnlyList<SheetRange> ranges)
    {
        ArgumentNullException.ThrowIfNull(ranges);

        List<SheetNote> printed = [];
        foreach (SheetRange range in ranges)
        {
            List<SheetNote> inside = [];
            foreach (SheetNote note in Items)
            {
                if (note.Column < range.FirstColumn || note.Column > range.LastColumn) continue;
                if (note.Row < range.FirstRow || note.Row > range.LastRow) continue;
                if (note.Text.Length == 0) continue;
                inside.Add(note);
            }

            inside.Sort(static (left, right) => left.Column != right.Column
                ? left.Column.CompareTo(right.Column)
                : left.Row.CompareTo(right.Row));

            printed.AddRange(inside);
        }

        return printed;
    }
}

/// <summary>One note placed on a note page: its mark, its lines, and where it starts.</summary>
/// <param name="Mark">The cell's A1 address with a colon after it, as Calc prints it.</param>
/// <param name="Lines">The note's text, already wrapped to the page.</param>
/// <param name="Top">Where the note's first line begins, from the top of the printable area.</param>
internal readonly record struct PlacedNote(string Mark, IReadOnlyList<string> Lines, Length Top);

/// <summary>
/// Splits a sheet's notes into the pages Calc prints them on.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScPrintFunc::DoNotes</c> (<c>sc/source/ui/view/printfun.cxx:1930-2001</c>), which is short
/// enough to state whole. The page is divided in two by <c>nMarkLen</c>, the width of the string
/// <c>"GW99999:"</c> in the default cell font, capped at half the printable width; the marks are
/// drawn flush left and the note text starts after it and wraps in what is left. A note is placed
/// only while <c>nPosY + nTextHeight &lt; aPageRect.Bottom()</c>, and the pen then advances by the
/// note's height plus 200 twips.
/// </para>
/// <para>
/// <strong>A note too tall for an empty page stops the listing.</strong> The loop breaks with
/// nothing placed, <c>PrintNotes</c> returns zero, and <c>CountNotePages</c>'s
/// <c>while (nNoteAdd)</c> ends — so that note and every note after it is dropped rather than
/// spilling. Reproduced here rather than corrected, because the page count is what is being
/// matched.
/// </para>
/// <para>
/// Nothing is scaled. <c>DoNotes</c> sets <c>aTwipMode</c> rather than the page's zoomed map mode,
/// so a sheet printed at 60% still lists its notes at full size.
/// </para>
/// </remarks>
internal static class SheetNotePages
{
    /// <summary>The string whose width divides a note page, from <c>DoNotes</c>.</summary>
    private const string MarkSample = "GW99999:";

    /// <summary>The gap between two notes, in twips.</summary>
    private const int GapTwips = 200;

    /// <summary>Lays a sheet's printed notes out into pages.</summary>
    /// <param name="sheet">The sheet.</param>
    public static List<IReadOnlyList<PlacedNote>> Paginate(SheetLayout sheet)
    {
        ArgumentNullException.ThrowIfNull(sheet);

        List<IReadOnlyList<PlacedNote>> pages = [];
        if (!sheet.Setup.PrintsNotes || sheet.Notes.IsEmpty) return pages;

        // The sheet's own print ranges when it declares any, and otherwise the block it uses —
        // the same two cases `ScPrintFunc::CountNotePages` distinguishes with `bMultiArea`.
        IReadOnlyList<SheetRange> ranges = sheet.Setup.PrintAreas.Count > 0
            ? sheet.Setup.PrintAreas
            : [sheet.PrintedRange];

        List<SheetNote> notes = sheet.Notes.Printed(ranges);
        if (notes.Count == 0) return pages;

        DocRect area = sheet.Setup.PrintableArea;
        if (area.Width <= Length.Zero || area.Height <= Length.Zero) return pages;

        Length mark = MarkWidth(area.Width);
        Length available = area.Width - mark;
        if (available <= Length.Zero) return pages;

        Length lineHeight = SheetBandText.LineHeightAt(SheetBandText.DefaultSize);
        Length gap = Length.FromTwips(GapTwips);

        int at = 0;
        while (at < notes.Count)
        {
            List<PlacedNote> page = [];
            Length pen = Length.Zero;

            while (at < notes.Count)
            {
                List<string> lines = Wrap(notes[at].Text, available);
                Length height = lineHeight * lines.Count;

                // Calc's own strict comparison against the bottom of the printable area.
                if (pen + height >= area.Height) break;

                page.Add(new PlacedNote(Mark(notes[at]), lines, pen));
                pen += height + gap;
                at++;
            }

            // Nothing fitted, so nothing after it will either: the C++ loop ends the same way.
            if (page.Count == 0) break;
            pages.Add(page);
        }

        return pages;
    }

    /// <summary>How much of a note page the marks take.</summary>
    /// <param name="width">The printable width.</param>
    public static Length MarkWidth(Length width)
    {
        Length mark = SheetBandText.Shape(MarkSample, SheetBandText.DefaultSize)?.Width
                      ?? Length.Zero;

        Length half = width / 2;
        return mark > half ? half : mark;
    }

    /// <summary>The address Calc prints beside a note.</summary>
    private static string Mark(SheetNote note)
        => SheetAddress.Format(note.Column, note.Row) + ":";

    /// <summary>Wraps a note's text to the width left over beside the marks.</summary>
    /// <remarks>
    /// A note carries its own paragraph breaks — Excel writes the author on a line of its own —
    /// so the text is split on them first and each paragraph then wrapped by whole words, which
    /// is the same rule <see cref="SheetShapePainter"/> uses and for the same reason.
    /// </remarks>
    private static List<string> Wrap(string text, Length available)
    {
        List<string> lines = [];

        foreach (string paragraph in text.Replace("\r\n", "\n", StringComparison.Ordinal)
                                         .Split(['\n', '\r']))
        {
            if (paragraph.Length == 0)
            {
                lines.Add(string.Empty);
                continue;
            }

            string current = string.Empty;
            foreach (string word in paragraph.Split(' ', StringSplitOptions.None))
            {
                string candidate = current.Length == 0 ? word : current + " " + word;
                if (current.Length > 0 && Width(candidate) > available)
                {
                    lines.Add(current);
                    current = word;
                    continue;
                }

                current = candidate;
            }

            lines.Add(current);
        }

        return lines;
    }

    private static Length Width(string text)
        => SheetBandText.Shape(text, SheetBandText.DefaultSize)?.Width ?? Length.Zero;
}

/// <summary>
/// Draws one page of a sheet's notes: the marks down the left and the notes beside them.
/// </summary>
/// <remarks>
/// The furniture is the sheet's own — <c>PrintNotes</c> draws the header and the footer exactly as
/// <c>PrintPage</c> does (<c>sc/source/ui/view/printfun.cxx:2045-2056</c>) — and nothing else on
/// the page is: no grid, no headings, no drawing layer.
/// </remarks>
internal sealed class SheetNotePageDrawing(SheetLayout sheet, IReadOnlyList<PlacedNote> notes)
{
    private readonly SheetPageDecoration _decoration = new(sheet, default);

    /// <summary>Draws the page.</summary>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="context">What the header's fields stand for on this page.</param>
    public void Draw(IDrawingSink sink, SheetHeaderContext context)
    {
        ArgumentNullException.ThrowIfNull(sink);

        sink.BeginPage(sheet.Setup.PageSize);
        try
        {
            DocRect area = sheet.Setup.PrintableArea;
            Length mark = SheetNotePages.MarkWidth(area.Width);
            Length size = SheetBandText.DefaultSize;
            Length height = SheetBandText.LineHeightAt(size);
            Length ascent = SheetBandText.AscentAt(size);

            foreach (PlacedNote note in notes)
            {
                Length top = area.Y + note.Top;
                Write(sink, note.Mark, area.X, top + ascent, size);

                for (int line = 0; line < note.Lines.Count; line++)
                {
                    if (note.Lines[line].Length == 0) continue;
                    Write(
                        sink,
                        note.Lines[line],
                        area.X + mark,
                        top + (height * line) + ascent,
                        size);
                }
            }

            _decoration.DrawHeaderAndFooter(context, sink);
        }
        finally
        {
            sink.EndPage();
        }
    }

    private static void Write(IDrawingSink sink, string text, Length x, Length y, Length size)
    {
        if (SheetBandText.Shape(text, size) is not { } run) return;
        sink.DrawGlyphRun(run.At(new DocPoint(x, y)), Paint.Solid(Colour.Black));
    }
}

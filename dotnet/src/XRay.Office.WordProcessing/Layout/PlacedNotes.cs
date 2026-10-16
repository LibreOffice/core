namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// Which notes the text a page actually drew has cited.
/// </summary>
/// <remarks>
/// <para>
/// A note belongs to the page holding the <em>line</em> that contains its anchor. That rule is easy to
/// state and was, until this existed, implemented twice over the body flow alone — once in
/// <see cref="Paginator"/>, to charge the page for the room the note takes, and once in
/// <see cref="NoteRenumbering"/>, to count a page's notes for a per-page restart. Both walked the
/// paragraphs of the top-level flow and neither descended into a table, so a footnote cited from a table
/// cell was read, numbered and then silently dropped: no room was reserved for it, nothing drew it, and
/// the only visible trace was a page short by the note's words.
/// </para>
/// <para>
/// Measured on the words track: three documents cite a footnote from inside a table
/// (<c>TE.CAO.00125 … OJT Logbook</c>, <c>FO.FCTOA.00010 …</c> and
/// <c>EHEST-SMS-Safety-Management-Manual-V2</c>) and the reference draws every one of them at the foot of
/// the citing page, exactly as it draws a footnote cited from body text. LibreOffice makes no distinction
/// at all — <c>SwTextFrame::ConnectFootnote</c> hangs the note on the page frame the citing text frame is
/// in, whatever chain of cell and table frames lies between them.
/// </para>
/// <para>
/// A <em>placed</em> flow rather than a paragraph and a line range, because that is what a table gives:
/// <see cref="TableLayouter.SliceRow"/> hands back cells holding only the lines that fit above the cut, so
/// walking those lines is what makes a split row charge each of its two pages for the notes that page
/// draws and no others.
/// </para>
/// </remarks>
internal static class PlacedNotes
{
    /// <summary>
    /// The notes anchored in a set of placed lines, in the order the lines cite them.
    /// </summary>
    /// <param name="lines">The lines, whose <see cref="PlacedLine.ParagraphIndex"/> indexes into blocks.</param>
    /// <param name="blocks">The flow the lines were laid out from.</param>
    public static IEnumerable<PageNote> On(
        IReadOnlyList<PlacedLine> lines, IReadOnlyList<PageBlock> blocks)
    {
        foreach (PlacedLine line in lines)
        {
            if (line.ParagraphIndex < 0 || line.ParagraphIndex >= blocks.Count) continue;
            if (blocks[line.ParagraphIndex] is not PageParagraph paragraph) continue;
            if (paragraph.Notes.Count == 0) continue;

            foreach (PageNote note in paragraph.Notes)
            {
                // Endnotes collect at the end of the document rather than the foot of a page, so the page
                // that cites one is not the page it lands on and it takes no room here.
                if (note.Placement == NotePlacement.DocumentEnd) continue;

                if (note.Offset >= line.Box.Line.Start && note.Offset < line.Box.Line.End)
                {
                    yield return note;
                }
            }
        }
    }

    /// <summary>The notes anchored in the text a placed table drew, its nested tables included.</summary>
    /// <param name="table">The part of the table that landed on the page.</param>
    /// <param name="depth">How many tables deep this one is; the recursion's guard.</param>
    public static IEnumerable<PageNote> In(PlacedTable table, int depth = 0)
    {
        if (depth > FlowLayouter.MaxNesting) yield break;

        foreach (PlacedTableCell cell in table.Cells)
        {
            if (cell.Content is not { } flow) continue;

            foreach (PageNote note in In(flow, depth + 1)) yield return note;
        }
    }

    /// <summary>The notes anchored in the text a placed flow drew, its tables included.</summary>
    /// <param name="flow">The flow.</param>
    /// <param name="depth">How many tables deep it is; the recursion's guard.</param>
    public static IEnumerable<PageNote> In(PlacedFlow flow, int depth = 0)
    {
        foreach (PageNote note in On(flow.Lines, flow.Blocks)) yield return note;

        foreach (PlacedTable inner in flow.Tables)
        {
            foreach (PageNote note in In(inner, depth + 1)) yield return note;
        }
    }
}

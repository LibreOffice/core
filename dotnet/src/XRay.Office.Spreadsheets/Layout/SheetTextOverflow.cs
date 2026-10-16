using XRay.Office.Core.Extraction;
using XRay.Office.Core.Units;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// Widens a sheet's print area to cover text that overflows its column.
/// </summary>
/// <remarks>
/// <para>
/// A string too wide for its cell spills into the empty cells to its right, and Calc prints all
/// of it — so the printed area is wider than the area that holds cells. <c>AdjustPrintArea</c>
/// calls <c>ScTable::ExtendPrintArea</c> for exactly this
/// (<c>sc/source/core/data/table1.cxx:2127</c>, and the per-cell rule in
/// <c>MaybeAddExtraColumn</c> at <c>:2218</c>), and without it a sheet of long strings comes out
/// a page narrower than the reference.
/// </para>
/// <para>
/// It is worth being clear that this is the one place pagination depends on measuring text. Row
/// heights and column widths are stated in the file, so nothing else about where the pages fall
/// needs a font — which is why an inexact measurement here is survivable: the extension is by
/// <em>whole columns</em>, so being within one column's width of LibreOffice's answer gives the
/// same page. Measured on <c>xls-features.xls</c>, whose Strings sheet holds 183-character
/// strings in a 64-point column: the extension runs to column N either way, and the sheet takes
/// two pages.
/// </para>
/// <para>
/// Four of Calc's conditions are reproduced and matter. Only a cell holding <em>text</em>
/// overflows — a number too wide shows <c>###</c> instead, which takes no extra room. A cell that
/// <em>wraps</em> never overflows at all, so it never widens anything. Overflow
/// stops at the first non-empty cell to the right, so a value in the next column truncates the
/// string rather than being written over. And a right-aligned cell overflows to the left, which
/// costs no columns at the right-hand end.
/// </para>
/// <para>
/// The wrap condition is the one that dominated the corpus and it is worth stating why. A
/// wrapping column is usually a <em>wide</em> column of long prose, so the strings in it measure
/// to thousands of points; extending the print area by that much adds column after column of
/// empty sheet, each of which becomes a band of pages that hold nothing. Measured on
/// <c>AFS-400_Contacts.xlsx</c>, seven columns of contacts with three wrapping ones: 340 pages
/// against LibreOffice's 48, of which 289 of ours were blank and none of LibreOffice's were.
/// </para>
/// </remarks>
internal static class SheetTextOverflow
{
    /// <summary>
    /// How many cells will be measured before the walk gives up.
    /// </summary>
    /// <remarks>
    /// A guard against a hostile sheet rather than a tuning knob: measuring is shaping, and a
    /// million-row sheet of distinct strings would otherwise shape a million of them to decide a
    /// page boundary. Reaching the limit leaves the print area as wide as it had got, which
    /// under-reports rather than mis-reports.
    /// </remarks>
    private const int MeasurementBudget = 20_000;

    /// <summary>The resolution Calc's text-width cache counts its pixels in.</summary>
    private const double ReferenceDeviceDpi = 600;

    /// <summary>
    /// The width as Calc's own cache holds it: sixteen bits of 600 dpi pixels, and it wraps.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This reproduces an overflow in the reference, deliberately.</strong>
    /// <c>ScTable::MaybeAddExtraColumn</c> does not measure the cell — it reads the width out of
    /// <c>ScColumn</c>'s text-width cache, which stores it as a <c>sal_uInt16</c> in pixels of a
    /// 600 dpi reference device. 65536 of those pixels are 7864.32 pt, so a cell wider than that
    /// is cached as the <em>remainder</em>, and Calc extends the print area by the remainder
    /// rather than by the real width.
    /// </para>
    /// <para>
    /// Measured, on an authored one-cell probe of <em>n</em> repetitions of <c>M</c> in a 2 cm
    /// column at ten point, rendered through the installed 26.2.4.2 — the reference's page count
    /// is 18 at 945 characters, <strong>1</strong> at 950, back to 18 at 1890 and 2 at 2000,
    /// while ours runs 18, 18, 35, 37. A count that falls and then rises again cannot be a clamp
    /// or a budget; it is a modulus, and the period is the 16-bit boundary.
    /// </para>
    /// <para>
    /// It is worth two documents in the corpus and it is the whole of both.
    /// <c>grants-2005.xls</c> holds a 2831-character string in <c>H1292</c> measuring 12834 pt,
    /// 1.63 times the limit; wrapped it is 4970 pt, which stops the print area at column CA where
    /// we reached IB — 18 column bands of empty paper, and exactly the 219-against-201 gap.
    /// <c>CIS_Debian_Linux_8_Benchmark_v1.0.0.xls</c> is the same cell twice over, at
    /// <c>Level 1!F95</c> and <c>Level 2!F23</c>, and accounts for all 21 of its extra pages
    /// (51+37 blanks against 37+30).
    /// </para>
    /// <para>
    /// The extra pages are blank on both sides, so this is not about what is painted. Both
    /// renderers emit ink-free pages for an over-wide print area — 67 of the reference's 88 CIS
    /// pages carry no drawing operator at all — which is why the fix belongs here, in the one
    /// measurement that decides how wide the area is, and not in whatever decides to emit a page.
    /// </para>
    /// </remarks>
    private static Length CachedTextWidth(Length width)
    {
        if (width <= Length.Zero) return Length.Zero;

        long pixels = (long)Math.Round(width.Points * ReferenceDeviceDpi / 72.0);
        return Length.FromPoints((pixels & 0xFFFF) * 72.0 / ReferenceDeviceDpi);
    }

    /// <summary>
    /// How wide the cell's text is for the purpose of overflow: its widest paragraph where the
    /// importer made several, and the whole string where it made one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A cell the importer stored as an <c>EditTextObject</c> is measured by the
    /// EditEngine, and the EditEngine answers the widest <em>paragraph</em> rather than the
    /// length of the string.</strong> <c>ScColumn::GetNeededSize</c> takes the edit branch for
    /// <c>CELLTYPE_EDIT</c> (<c>sc/source/core/data/column2.cxx</c>:297-300), formats the cell
    /// against a paper 1000000 units wide (<c>:447</c>) so that nothing is broken for width, and
    /// reads the width off <c>pEngine-&gt;CalcTextWidth()</c> (<c>:565</c>) —
    /// <c>ImpEditEngine::CalcTextWidth</c> being a maximum over the paragraphs
    /// (<c>editeng/source/editeng/impedit2.cxx</c>:3507-3525). So a cell of thirty short
    /// paragraphs spills as far as its longest line and not as far as their sum.
    /// </para>
    /// <para>
    /// <strong>Which cells those are is the importer's decision and not the cell's</strong>, and
    /// it is the same distinction <see cref="SheetLayout.CellBreaksStartLines"/> already carries:
    /// Calc's ODF filter hands a multi-line string to the engine with single-line mode off, so it
    /// becomes one paragraph per break, while the BIFF and SpreadsheetML filters set
    /// <c>SetSingleLine</c> for a cell that does not wrap and the break stays inside one paragraph
    /// — which is one line, and therefore the whole string. Measuring the whole string in both is
    /// what this did before, and it is right for exactly one of the two.
    /// </para>
    /// <para>
    /// <strong>It is the whole of <c>CIS_Debian_Linux_8_Benchmark_v1.0.0.ods</c>'s 88 pages
    /// against 61</strong>, established on the reference rather than inferred. Not one of its 361
    /// multi-paragraph cells wraps, so nothing else keeps the spill in check, and 68 of the
    /// <c>Level 2</c> sheet's 382 paragraphs carry a raw newline. Rewriting every newline in the file as a space
    /// — one paragraph per cell, the same characters — makes 26.2.4.2 itself render
    /// <c>Level 1</c> in <strong>51</strong> pages and <c>Level 2</c> in <strong>36</strong>,
    /// which are this tree's two counts exactly; keeping only each cell's longest paragraph
    /// leaves the reference at its own 39 and 21. <c>dotnet/probes/ods-resid-r80/</c>.
    /// </para>
    /// </remarks>
    /// <param name="text">The cell's display text.</param>
    /// <param name="format">Its format, for the face and size.</param>
    /// <param name="breaksStartLines">
    /// Whether this workbook's importer makes a paragraph of each hard break — true for ODF and
    /// false for the two Excel families.
    /// </param>
    private static Length WidestParagraph(string text, SheetCellFormat format, bool breaksStartLines)
    {
        SheetFace? face = SheetFonts.For(format);

        if (!breaksStartLines || text.AsSpan().IndexOfAny('\n', '\r') < 0)
            return SheetText.Measure(text, face, format.FontSize);

        Length widest = Length.Zero;

        // `\r\n` is one break, as it is everywhere else a cell's paragraphs are counted; an empty
        // paragraph measures nothing and cannot be the widest, so it needs no special case.
        int at = 0;
        while (at <= text.Length)
        {
            int end = at;
            while (end < text.Length && text[end] is not ('\n' or '\r')) end++;

            Length width = SheetText.Measure(text[at..end], face, format.FontSize);
            if (width > widest) widest = width;

            if (end < text.Length && text[end] == '\r' && end + 1 < text.Length && text[end + 1] == '\n')
                end++;
            at = end + 1;
        }

        return widest;
    }

    /// <summary>The cell text margin either side, which counts towards the width needed.</summary>
    /// <remarks>
    /// The cell's own <c>ATTR_MARGIN</c> rather than the pool default, because an <c>.xls</c>
    /// states 40 twips on every format — see <see cref="SheetCellFormat.Margin"/>.
    /// </remarks>
    private static Length CellMarginsOf(SheetCellFormat format) => format.Margin * 2;

    /// <summary>The last column a sheet's contents reach, overflow included.</summary>
    /// <param name="sheet">The sheet.</param>
    /// <param name="used">The block of cells the sheet holds.</param>
    public static int ExtendedLastColumn(SheetLayout sheet, SheetRange used)
    {
        ArgumentNullException.ThrowIfNull(sheet);
        if (!used.IsValid) return used.LastColumn;

        Dictionary<(string Text, SheetCellFormat Format), Length> widths = [];
        int last = used.LastColumn;
        int measured = 0;

        foreach (ContentTableRow row in (sheet.Cells?.Children ?? []).OfType<ContentTableRow>())
        {
            if (row.Index < used.FirstRow || row.Index > used.LastRow) continue;

            foreach (ContentTableCell cell in row.Children.OfType<ContentTableCell>())
            {
                if (cell.Column < used.FirstColumn || cell.Column > used.LastColumn) continue;
                if (cell.Value is not null and not string) continue;
                if (sheet.Grid.Columns.IsHidden(cell.Column)) continue;

                string text = cell.GetOwnText();
                if (text.Length == 0) continue;

                // Nothing overflows into an occupied cell, and checking first is what keeps a
                // dense sheet from being measured at all — the same short-circuit Calc added in
                // tdf#128873.
                if (!SheetTextLayout.IsAvailable(sheet.CellAt(row.Index, cell.Column + 1))) continue;
                if (measured++ >= MeasurementBudget) return last;

                SheetCellFormat format = sheet.Formats.At(row.Index, cell.Column);

                // A cell that breaks its text into lines needs no width beyond its own column, so
                // it never widens the print area. `ScColumn::GetNeededSize` says it in one line —
                // `if ( bWidth && bBreak ) return 0;` (sc/source/core/data/column2.cxx:226) —
                // and it is the difference between a page and a hundred: a wrapping column of
                // long strings measures to thousands of points, and extending the print area by
                // that much manufactures column band after column band of blank paper.
                // Never a value here: the walk above kept only cells holding text.
                if (SheetTextLayout.Breaks(format, isValue: false)) continue;

                if (!widths.TryGetValue((text, format), out Length width))
                {
                    width = CachedTextWidth(
                        WidestParagraph(text, format, sheet.CellBreaksStartLines)
                        + CellMarginsOf(format) + format.Indent);
                    widths[(text, format)] = width;
                }

                Length missing = width - sheet.Grid.Columns.SizeAt(cell.Column);

                // Alignment decides which way the overflow goes, and only the rightward part
                // costs columns. Calc looks at exactly this and no more: a centred cell spills
                // both ways so half is missing to the right, and a right-aligned one spills only
                // to the left, which costs nothing at the right-hand end
                // (`ScTable::MaybeAddExtraColumn`, sc/source/core/data/table1.cxx:2264-2276).
                if (missing > Length.Zero)
                {
                    if (format.Horizontal == SheetHorizontalAlignment.Centre) missing /= 2;
                    else if (format.Horizontal == SheetHorizontalAlignment.Right) missing = Length.Zero;
                }

                int at = cell.Column;
                while (missing > Length.Zero && at < SheetAddress.MaxColumn)
                {
                    if (!SheetTextLayout.IsAvailable(sheet.CellAt(row.Index, at + 1))) break;
                    at++;
                    missing -= sheet.Grid.Columns.PrintedSizeAt(at);
                }

                if (at > last) last = at;
            }
        }

        return last;
    }
}

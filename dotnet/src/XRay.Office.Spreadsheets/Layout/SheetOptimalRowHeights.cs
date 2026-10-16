using XRay.Office.Core.Extraction;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// Recomputes the height of every row whose file states one only as a hint.
/// </summary>
/// <remarks>
/// <para>
/// A row height in a spreadsheet file is two different things wearing one attribute. When the
/// file also says the user chose it — SpreadsheetML's <c>customHeight</c>, BIFF's <c>fUnsynced</c>,
/// ODF's <c>style:use-optimal-row-height="false"</c> — it is a statement and is honoured. When it
/// does not, it is the writing application's own measurement of the row's content, and Calc
/// discards it and measures the content again before anything is drawn. All three import filters
/// do this and each does it in its own place:
/// <c>WorkbookGlobals::finalize</c> for SpreadsheetML
/// (<c>sc/source/filter/oox/workbookhelper.cxx:659</c>), <c>ImportExcel::AdjustRowHeight</c> for
/// BIFF (<c>sc/source/filter/excel/impop.cxx:1285</c>) and <c>ScXMLImport</c>'s recalc ranges for
/// ODF (<c>sc/source/filter/xml/xmlimprt.cxx:1438</c>). The stated height survives only as the
/// starting value.
/// </para>
/// <para>
/// <strong>The common case is arithmetic, not measurement, and that is what makes this shippable.</strong>
/// <c>ScColumn::GetOptimalHeight</c> (<c>sc/source/core/data/column2.cxx:898-1100</c>) splits every
/// cell into two kinds. A cell that neither wraps, rotates, stacks nor holds more than one line is
/// <em>standard only</em>, and its row asks for <c>lcl_GetAttribHeight</c>
/// (<c>column2.cxx:866-892</c>): the font's <em>size</em> times 1.18, plus the cell's top and
/// bottom margins, less 23 twips, floored at the sheet's minimum. No glyph is measured and no
/// device is involved, so it can be reproduced exactly. Anything else is measured through
/// <c>ScColumn::GetNeededSize</c> against a reference device.
/// </para>
/// <para>
/// <strong>That measurement is coarser than the one Calc draws with, and the coarseness is the
/// whole of it.</strong> It quantises to whole device pixels three times over — the em size, the
/// ascent and the descent — and the two margins truncate to a pixel each. Reproducing the formula
/// with accurate metrics is what put an earlier attempt 5.8% out; reproducing the quantisation
/// puts it on the number. See <see cref="WrappedHeight"/>, which was fitted to thirty probe rows
/// and reproduces all thirty exactly.
/// </para>
/// <para>
/// <strong>The device is coarse horizontally too, and differently.</strong> A row is a line count
/// before it is a height, so the width that decides where the text breaks is as much of the answer
/// as the pitch is — and two things make that width Calc's rather than the document's. The em is
/// rounded to whole pixels before a single advance is measured, so 11 pt text is set at 15 pixels
/// and runs 2.3% wide while 10 pt is set at 13 and runs 2.5% narrow. And the paper is the column
/// width times <c>nPPTX</c>, which <see cref="OutputFactor"/> has divided and <c>nPPTY</c> has not.
/// Neither is visible in the drawn page — LibreOffice draws the same text at the same advances
/// X-Ray.Office does, measured off its own PDF — and only the row it reserved for it disagrees.
/// </para>
/// <para>
/// A <em>turned</em> cell is measured by <see cref="RotatedHeight"/>, whose branch of
/// <c>GetNeededSize</c> wraps nothing: the whole string is measured on one line and its width is
/// put through the angle. What is still not reproduced is a <em>stacked</em> cell, and a turned one
/// Calc would have handed to an EditEngine instead — a rich cell, or one holding a hard break. A
/// row holding either takes the larger of the arithmetic answer and the height its file already
/// states. That fallback is not a fudge — the arithmetic answer really is a lower bound in Calc
/// too, because <c>bStdAllowed</c> stays true for an obliquely turned cell and its attribute height
/// is written into the array before any measurement is compared against it — and it means a row
/// this cannot measure is never shorter than the writer made it.
/// </para>
/// <para>
/// <strong>A quarter turn is the exception to that lower bound and to the sheet's minimum alike.</strong>
/// <c>bStdAllowed</c> is the cell's <em>orientation</em> being <c>Standard</c>
/// (<c>column2.cxx:925</c>), which exactly 90° and exactly 270° are not, so such a row gets neither
/// the arithmetic height nor the floor <c>lcl_GetAttribHeight</c> carries. See
/// <see cref="IsQuarterTurned"/>.
/// </para>
/// <para>
/// Measured on <c>National-Reports.xlsx</c>, whose 117 rows state <c>ht</c> and none states
/// <c>customHeight</c>: LibreOffice's own flat-ODF export gives <c>0.2083in</c> — 300 twips — for
/// every single-line row, and this computes 300 twips for each of them from a 12 pt font
/// (<c>trunc(240 × 1.18) = 283</c>, plus 40 twips of margin, less 23).
/// </para>
/// </remarks>
internal static class SheetOptimalRowHeights
{
    /// <summary>
    /// Calc's own fudge between a font's size and the row it needs.
    /// </summary>
    /// <remarks><c>nHeight *= 1.18</c>, <c>sc/source/core/data/column2.cxx:869</c>.</remarks>
    private const double FontHeightFactor = 1.18;

    /// <summary>
    /// The twips a row is allowed to be shorter than its font plus its margins.
    /// </summary>
    /// <remarks>
    /// <c>STD_ROWHEIGHT_DIFF</c>, <c>sc/inc/global.hxx:116</c>, whose comment states the whole
    /// formula: "standard row height: text + margin - STD_ROWHEIGHT_DIFF".
    /// </remarks>
    private const int StandardRowHeightDifference = 23;

    /// <summary>
    /// A cell's top and bottom margins, together, in twips.
    /// </summary>
    /// <remarks>
    /// The pair, because the arithmetic height adds both at once
    /// (<c>lcl_GetAttribHeight</c>, <c>column2.cxx:882-884</c>). It is the cell's own
    /// <c>ATTR_MARGIN</c> and not a constant, because the BIFF filter puts 40 twips on every
    /// format it builds where the pool default is 20 — see <see cref="SheetCellFormat.Margin"/>.
    /// At the pool default it is what turns a 12 pt font's 283 twips into the 300 Calc writes.
    /// The horizontal pair is not the same number of *pixels*: it comes off the paper as two
    /// truncations of the margin times <c>nPPTX</c>, and <c>nPPTX</c> has been divided by
    /// <see cref="OutputFactor"/>.
    /// </remarks>
    private static int VerticalMarginTwipsOf(SheetCellFormat format)
        => 2 * (int)format.Margin.Twips;

    /// <summary>
    /// The twips a pixel is worth on the device Calc measures rows against.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScSizeDeviceProvider</c> builds a plain <c>VirtualDevice</c> and derives its
    /// pixels-per-twip from it (<c>sc/source/ui/docshell/sizedev.cxx:43-50</c>); headless, that is
    /// 96 dpi, so a pixel is 15 twips.
    /// </para>
    /// <para>
    /// It is used in two places and they are not the same number as <see cref="PixelsPerTwip"/>'s
    /// reciprocal. Turning a paper width in pixels back into a length is
    /// <c>pDev->PixelToLogic(aPaper, aHMMMode)</c> (<c>column2.cxx:481-486</c>), which uses the
    /// device's real resolution — 2540/96 per pixel in hundredths of a millimetre, so 15 twips.
    /// The row height that comes back the other way is divided by <c>nPPTY</c>, which is Calc's
    /// rounded 0.067. Both are below, deliberately.
    /// </para>
    /// <para>
    /// It also matters because <c>lcl_pixelSizeChanged</c>
    /// (<c>sc/source/core/data/table2.cxx:3388</c>)
    /// refuses to replace a stated height with a computed one that rounds to the same pixel, so a
    /// file whose heights are already Calc's own answers keeps them exactly. That guard is applied
    /// here to all three formats although Calc applies it to two: the BIFF filter reaches rows
    /// through <c>SetOptimalHeightOnly</c>, which has no guard. Keeping it is the conservative
    /// difference — it can only leave a file's own height in place — and it is what makes a
    /// document LibreOffice wrote round-trip unchanged when the arithmetic here lands a twip or
    /// two away from Calc's.
    /// </para>
    /// </remarks>
    private const int TwipsPerPixel = 15;

    /// <summary>The resolution of the device Calc measures a row against.</summary>
    /// <remarks>
    /// A headless <c>VirtualDevice</c>'s, and the grid the font metrics are rounded onto before
    /// the row height is built out of them.
    /// </remarks>
    private const int ScreenDpi = 96;

    /// <summary>
    /// Pixels per twip on that device, as Calc computes it rather than as it is.
    /// </summary>
    /// <remarks>
    /// Not 1/15. <c>ScSizeDeviceProvider</c> derives the figure by converting a thousand twips to
    /// pixels and dividing — <c>LogicToPixel(Point(1000,1000), MapTwip).Y() / 1000.0</c>,
    /// <c>sc/source/ui/docshell/sizedev.cxx:48-50</c> — and that conversion returns whole pixels,
    /// so 666.67 becomes 667 and the ratio becomes 0.067 exactly. The 0.5% is not noise: every
    /// height below is a pixel count divided by this, and dividing by 1/15 instead puts a
    /// three-line twelve-point row at 795 twips where LibreOffice writes 791.
    /// </remarks>
    private const double PixelsPerTwip = 0.067;

    /// <summary>
    /// A cell's top or bottom margin in whole pixels on that device.
    /// </summary>
    /// <remarks>
    /// <c>static_cast&lt;tools::Long&gt;(20 * 0.067)</c> is 1, and the truncation is why the pair
    /// of them is worth 2 pixels rather than the 2.68 the twips would give. A BIFF cell's 40
    /// twips truncate to 2 by the same expression.
    /// </remarks>
    private static int MarginPixelsOf(SheetCellFormat format)
        => (int)(format.Margin.Twips * PixelsPerTwip);

    /// <summary>
    /// The print-to-screen factor the horizontal resolution — and only the horizontal — is
    /// divided by.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScSizeDeviceProvider</c> derives both resolutions from its virtual device and then does
    /// one more thing to one of them: <c>nPPTX /= rDocSh.GetOutputFactor()</c>
    /// (<c>sc/source/ui/docshell/sizedev.cxx:52</c>). <c>nPPTY</c> is left alone. So a row's
    /// <em>height</em> is worked out on a 96 dpi grid and its <em>paper width</em> is not, and the
    /// paper <c>GetNeededSize</c> hands the EditEngine is this much narrower than the column
    /// (<c>nDocWidth = GetOriginalWidth(nCol) * nPPTX</c>, <c>column2.cxx:463</c>).
    /// </para>
    /// <para>
    /// <c>ScDocShell::CalcOutputFactor</c> (<c>sc/source/ui/docshell/docsh3.cxx:380-426</c>) sets
    /// it once per document, before the import, as the ratio of one fixed 72-character test string
    /// measured on the reference device to the same string measured on the screen device — so it
    /// is a property of the machine and the application's default cell font rather than of the
    /// workbook, which is why it is a constant here alongside the 96 dpi and the 0.067. Like
    /// those two it is fitted to this machine's headless <c>soffice</c>: see the probe workbooks
    /// in <c>SheetRowHeightDeviceTests</c>, whose 2114 rows it reproduces exactly and which are
    /// what a different machine would have to be re-fitted against.
    /// </para>
    /// </remarks>
    private const double OutputFactor = 1.0345;

    /// <summary>
    /// The grid a sheet is laid out on, with its hinted row heights re-derived from its content.
    /// </summary>
    /// <param name="sheet">The sheet, for its cells, formats and merges.</param>
    /// <param name="grid">Its geometry as the file states it.</param>
    internal static SheetGrid Apply(SheetLayout sheet, SheetGrid grid)
    {
        if (grid.RowHeightsAreManual) return grid;

        // The drawings are part of the range, and this is the one place it is easy to miss.
        // `ScDocRowHeightUpdater::updateAll` is called with `bOnlyUsedRows`, which reads as "the
        // cells" and is not: it takes its last row from `ScDocument::GetPrintArea`
        // (`sc/source/core/data/dociter.cxx:1731-1734`), and that is the *document's* print area,
        // maxed against the drawing layer's bounding box (`documen2.cxx:644-664`). So a sheet whose
        // watermark hangs a hundred rows below its last cell has all hundred of those empty rows
        // measured and resized, and every page boundary below the cells moves with them.
        //
        // Measured on `SIL_TDB648.xlsx`: its `RAAS` sheet ends at row 33 and its watermark reaches
        // row 148, and LibreOffice's own flat-ODF export gives every row from 53 to 148 a height of
        // 0.1756in — 253 twips, the arithmetic height of the sheet's 10 pt default — where we kept
        // the file's stated default of 12.6 pt, snapped to 240. Thirteen twips a row over a hundred
        // rows is a whole band of pages: LibreOffice prints four bands of that sheet and we printed
        // three.
        //
        // The drawing extension is measured against the grid as *stated*, which is the grid Calc
        // has when it asks: the print area is found before the heights are recomputed, and this is
        // the routine that recomputes them.
        SheetRange range = SheetDrawingArea.Extend(
            SheetDecorationArea.Extend(
                sheet.UsedRange, sheet.Formatting, sheet.LastDataRowByColumn),
            sheet.Drawings,
            grid);
        if (!range.IsValid) return grid;

        int lastRow = range.LastRow;
        int firstColumn = range.FirstColumn;
        int lastColumn = range.LastColumn;
        if (lastRow < 0 || lastColumn < firstColumn) return grid;

        SheetCellFormats formats = sheet.Formats;
        int minimum = (int)grid.OptimalMinimumRowHeight.Twips;

        // The columns a row is measured across are the *allocated* ones and not the print area's:
        // `GetOptimalHeightsInColumn` walks `aCol` (`table1.cxx:88-127`), so a column a formatted
        // blank cell brought into existence is measured for every row of the sheet even though no
        // page ever reaches it. See `SheetCellFormats.LastAllocatedColumn`.
        int measuredLastColumn = Math.Max(lastColumn, formats.LastAllocatedColumn);

        // The columns a cell states nothing about resolve to the column's format, then to the
        // sheet's. Neither depends on the row, so both are folded once rather than per row.
        int baseline = AttributeHeight(formats.SheetDefault, minimum);
        foreach (SheetCellFormat column in formats.ColumnDefaults(firstColumn, measuredLastColumn))
            baseline = Math.Max(baseline, AttributeHeight(column, minimum));

        Dictionary<int, RowState> rows =
            CollectRows(sheet, formats, range, grid.Columns, minimum, measuredLastColumn);

        SheetAxis axis = grid.Rows;
        Dictionary<int, int> changes = [];
        int index = 0;

        while (index <= lastRow)
        {
            // A height the user chose is honoured, and it also ends the run Calc would have
            // grouped: `SetOptimalHeightsToRows` flushes its pending range at a manual row
            // (`sc/source/core/data/table1.cxx:221-226`).
            if (!axis.IsOptimalSize(index)) { index++; continue; }

            int height = Optimal(rows, index, baseline, axis);
            int last = index;
            while (last + 1 <= lastRow && axis.IsOptimalSize(last + 1)
                   && Optimal(rows, last + 1, baseline, axis) == height)
            {
                last++;
            }

            // Calc replaces a run's heights only when the new one is at least a pixel away from
            // one of them, and then replaces all of them.
            bool changed = false;
            for (int row = index; row <= last && !changed; row++)
            {
                long stated = axis.SizeAt(row).Twips;
                changed = stated != height && stated / TwipsPerPixel != height / TwipsPerPixel;
            }

            if (changed)
                for (int row = index; row <= last; row++) changes[row] = height;

            index = last + 1;
        }

        return changes.Count == 0 ? grid : grid with { Rows = Rebuild(axis, changes, lastRow) };
    }

    /// <summary>
    /// The axis again with the recomputed rows replaced and everything else exactly as stated.
    /// </summary>
    /// <remarks>
    /// Built in one ordered pass and handed to <see cref="SheetAxis.FromOrdered"/> rather than to
    /// the normalising constructor, whose cost is quadratic in the run count — and a sheet stating
    /// a height for each of ten thousand rows has ten thousand runs before this touches it.
    /// </remarks>
    private static SheetAxis Rebuild(SheetAxis axis, Dictionary<int, int> changes, int lastRow)
    {
        List<SheetSizeRun> runs = [];

        for (int row = 0; row <= lastRow; row++)
        {
            bool hidden = axis.IsHidden(row);
            bool optimal = changes.ContainsKey(row) || axis.IsOptimalSize(row);
            Length size = changes.TryGetValue(row, out int height)
                ? Length.FromTwips(height)
                : axis.SizeAt(row);

            // A row that is in every way the axis default needs no run of its own.
            if (!hidden && optimal && size == axis.DefaultSize) continue;

            if (runs.Count > 0 && runs[^1].Last + 1 == row && runs[^1].Size == size
                && runs[^1].IsHidden == hidden && runs[^1].IsOptimalSize == optimal)
            {
                runs[^1] = runs[^1] with { Last = row };
                continue;
            }

            runs.Add(new SheetSizeRun(row, row, size, hidden, optimal));
        }

        // Everything past the sheet's content keeps whatever the file said, which is also where
        // Calc stops: its SpreadsheetML path recomputes only the used rows
        // (`UpdateAllRowHeights(/*bOnlyUsedRows=*/true)`, `workbookhelper.cxx:659`).
        foreach (SheetSizeRun run in axis.Runs)
        {
            if (run.Last <= lastRow) continue;
            runs.Add(run.First <= lastRow ? run with { First = lastRow + 1 } : run);
        }

        return SheetAxis.FromOrdered(axis.DefaultSize, runs);
    }

    /// <summary>
    /// The height one row asks for, in twips.
    /// </summary>
    /// <remarks>
    /// The sheet's minimum is <em>not</em> applied here, and that is deliberate rather than an
    /// omission. Calc applies it inside <c>lcl_GetAttribHeight</c> (<c>column2.cxx:889-890</c>) —
    /// which is to say only along the arithmetic path, and only for a pattern whose orientation is
    /// standard. So a row holding nothing but quarter-turned cells has no floor at all, and
    /// LibreOffice really does write 149 twips for a single letter in a 10 pt cell turned through
    /// ninety degrees. Every other row still meets the floor because
    /// <see cref="AttributeHeight(SheetCellFormat, int)"/> now carries it.
    /// </remarks>
    private static int Optimal(
        Dictionary<int, RowState> rows, int row, int baseline, SheetAxis axis)
    {
        int height = baseline;
        bool unmeasurable = false;

        if (rows.TryGetValue(row, out RowState state))
        {
            height = Math.Max(state.Attribute, state.CoversEveryColumn ? 0 : baseline);
            height = Math.Max(height, state.Measured);
            unmeasurable = state.Unmeasurable;
        }

        // A row holding something Calc would have measured and this cannot takes the larger of the
        // arithmetic lower bound and what its file already states, so it is never shorter than the
        // writer made it.
        if (unmeasurable) height = Math.Max(height, (int)axis.SizeAt(row).Twips);

        return height;
    }

    /// <summary>What one row's own cells and row format contribute.</summary>
    /// <param name="Attribute">The tallest arithmetic height any of them asks for.</param>
    /// <param name="CoversEveryColumn">
    /// True when the row states a format for every column in range, so nothing falls through to a
    /// column or sheet default.
    /// </param>
    /// <param name="Measured">The tallest wrapped cell in it, in twips, or zero when it has none.</param>
    /// <param name="Unmeasurable">
    /// True when one of its cells would have gone through <c>ScColumn::GetNeededSize</c> along a
    /// path this does not reproduce — a turned or stacked cell.
    /// </param>
    private readonly record struct RowState(
        int Attribute, bool CoversEveryColumn, int Measured, bool Unmeasurable);

    private static Dictionary<int, RowState> CollectRows(
        SheetLayout sheet, SheetCellFormats formats, SheetRange range, SheetAxis columns,
        int minimum, int measuredLastColumn)
    {
        Dictionary<int, RowState> rows = [];

        // `bStdAllowed` (`column2.cxx:925`) is the cell's *orientation* being Standard, not its
        // angle being zero — so a cell turned by anything other than a quarter turn still writes
        // its arithmetic height into the array as a lower bound, and one turned by exactly ninety
        // degrees, or stacked, writes nothing at all. Measured: a single letter in a 10 pt cell
        // asks for 257 twips at 45° and 149 at 90°.
        void Contribute(int row, SheetCellFormat format)
        {
            rows.TryGetValue(row, out RowState state);
            rows[row] = state with
            {
                Attribute = Math.Max(
                    state.Attribute,
                    IsQuarterTurned(format) ? 0 : AttributeHeight(format, minimum)),
            };
        }

        IReadOnlyList<SheetRange> merges = sheet.MergedRanges;

        // Bounded by the range rather than filtered down to it, because a repeated element is
        // held as a rectangle and unfolding one whole would be the sheet's full extent. See
        // SheetCellFormats.CellsIn.
        foreach ((int row, int column, SheetCellFormat format)
                 in formats.CellsIn(range with { FirstRow = 0 }))
        {
            // Calc skips a cell that a merge covers and a merge anchor that spans rows, and takes
            // only the anchor of a purely horizontal one — `ScColumn::GetOptimalHeight`,
            // `sc/source/core/data/column2.cxx:917-925`.
            if (IsExcludedByMerge(merges, row, column)) continue;

            Contribute(row, format);
        }

        foreach (int row in RowsStatingEveryColumn(
                     formats, merges, range.FirstColumn, measuredLastColumn, range.LastRow))
        {
            rows.TryGetValue(row, out RowState covered);
            rows[row] = covered with { CoversEveryColumn = true };
        }

        for (int row = 0; row <= range.LastRow; row++)
        {
            if (formats.RowDefault(row) is { } format) Contribute(row, format);
        }

        // Only a cell that holds something is measured: `GetNeededSize` returns zero for an empty
        // one before it looks at the pattern (`column2.cxx:100-103`), so a wrap-formatted blank
        // does not make its row unmeasurable.
        foreach (ContentTableRow tableRow in (sheet.Cells?.Children ?? []).OfType<ContentTableRow>())
        {
            foreach (ContentTableCell cell in tableRow.Children.OfType<ContentTableCell>())
            {
                if (cell.Row < 0 || cell.Row > range.LastRow) continue;
                if (cell.Column < range.FirstColumn || cell.Column > range.LastColumn) continue;
                if (IsExcludedByMerge(merges, cell.Row, cell.Column)) continue;

                string text = cell.GetOwnText();
                if (text.Length == 0) continue;

                SheetCellFormat format = formats.At(cell.Row, cell.Column);

                // The same wrap decision the drawing path makes, so that a row is measured exactly
                // when its text will be broken — including Calc's rule that a plain number never
                // breaks however the cell is formatted.
                // A hyperlink cell is one EditEngine field, and a field is measured at one line
                // however narrow its column is. Missing that makes a column of URLs four or five
                // times too tall.
                //
                // **This is a rule about the height and not about the breaking, and the two really
                // do disagree.** The drawing path wraps a field — see `SheetFieldBreaker` — so the
                // gate here is not the same gate as the one that used to stand in
                // `SheetTextLayout.Place`, and removing it because that one was wrong would be a
                // second error. Measured on `dotnet/probes/sheets-wrap-01` under 26.2.4.2, the same
                // six cells converted to `.fods` with automatic row heights: the plain URL takes
                // `style:row-height="0.6425in"` (four lines) and the hyperlinked one
                // `"0.1756in"` (one), while the reference PDF of that very file draws the
                // hyperlinked cell's *three* wrapped lines straight over the rows beneath it. A
                // field's lines overflow their row rather than sizing it.
                //
                // A hard break in a cell that does *not* wrap reaches neither the height nor the
                // page, and that is the **format's** decision rather than the text's. Both
                // importers say so in the same words, and say it whether the string is plain or
                // rich: `bSingleLine = !pXf->getAlignment().getModel().mbWrapText` before
                // `putRichString`, which is `rEE.SetSingleLine(bSingleLine)`
                // (`sc/source/filter/oox/sheetdatabuffer.cxx:125-133`,
                // `worksheethelper.cxx:1607-1611`), and `bSingleLine = !pXF->GetLineBreak()`
                // before the same call on the BIFF side (`xihelper.cxx:246-256`). An EditEngine in
                // single-line mode makes one paragraph of the whole string, so the U+000A stays in
                // the text and starts nothing. The height agrees from the other end — Calc's
                // `bStdOnly` is `!bBreak` (`column2.cxx:930-935`) — and so does the drawing:
                // `HasEditCharacters` (`output2.cxx:823-847`) lists seven code points and U+000A is
                // not among them, so `DrawStrings` shows the whole string on one line with the
                // break contributing no glyph. Measured on `Capability_List…unsorted.xlsx`, whose
                // A452 is `19090-105 (SCD\n604-85001-23)` in a cell with no wrap: LibreOffice
                // states 285.1 twips for the row — one line — and its PDF holds
                // `19090-105 (SCD604-85001-23)` as a single run with no space in it.
                //
                // ODF disagrees, and it disagrees about the height as well as about the drawing.
                // `ScXMLImport` makes a cell of several `text:p` — or of one holding a raw
                // newline — a multi-paragraph edit cell whatever the wrap says (`PushParagraphEnd`,
                // `xmlcelli.cxx`:610-636), `SheetLayout.CellBreaksStartLines` draws every one of
                // those paragraphs, and `GetNeededSize`'s height branch is
                // `pEngine->GetTextHeight()` (`column2.cxx:571-577`) — so N paragraphs are N lines
                // of row. See `StandingEditLine`.
                //
                // **That half only became implementable once the 200-row rule was.** Giving every
                // such row the paragraphs' height on its own made
                // `Capability_List…unsorted.ods` 150 pages against the reference's 147, because
                // 26.2.4.2 draws its fifteen two-paragraph cells as two lines each inside rows it
                // leaves 14.23 pt tall — page 11 has `19090-105 (SCD` at 239.26 and
                // `604-85001-23)` at 252.75 in a row spanning 238.52 to 252.75, so the second line
                // is painted over the row beneath. Those rows are past row 200 and keep their
                // stored height; the rule that says so is `OdsPrintSetup.RecalculatedRowLimit`,
                // and it reaches this routine as the rows' optimal flag being false.
                bool breaks =
                    SheetTextLayout.Breaks(format, cell.Value is not null and not string)
                    && !sheet.HoldsField(cell.Row, cell.Column);

                IReadOnlyList<SheetTextPortion>? portions =
                    sheet.RichText.At(cell.Row, cell.Column, text);

                // A turned cell's size is its text's *width* put through the angle, which
                // `RotatedHeight` reproduces — but only along Calc's direct-output path, which is
                // the one a plain single-paragraph string takes. Anything Calc would have handed to
                // an EditEngine instead — a stacked cell, a rich one, one holding a hard break —
                // is measured through a different branch again and stays unmeasurable here.
                bool turned = format.RotationDegrees != 0 && !format.IsStacked;
                bool direct = turned
                              && portions is not ({ Count: > 0 })
                              && text.AsSpan().IndexOfAny('\n', '\r') < 0;
                bool opaque = (format.IsStacked || turned) && !direct;

                // A cell carrying formatting runs or a hard break is an `EditTextObject` however
                // narrow or wide its column is, so Calc measures it through the EditEngine branch
                // even when it does not wrap — and one EditEngine line is not the same number as
                // the arithmetic height. See <see cref="StandingEditLine"/>.
                //
                // **Two more things put a cell on that branch and neither is a property of its
                // string**, which is why they were missed for as long as the arithmetic has been
                // here. A cell whose content Calc replaced with a `SvxURLField` is an
                // `EditTextObject` holding one field — the same object a rich string makes — and a
                // cell whose *pattern* carries a conditional format clears `bStdOnly` outright
                // (`column2.cxx:937-941`, "conditional formatting: loop all cells"), so it is
                // measured through `GetNeededSize`'s direct-output branch, `pDev->GetTextHeight()`
                // plus the two margins. Both answers are one line of the cell's own face, so both
                // are `StandingEditLine`.
                //
                // Established by one-attribute variant against 26.2.4.2, reading the row heights
                // out of its own `--convert-to fods` rather than off a page. On
                // `hdss-bulletin-index-2019-2022.ods` rows 1–200 come back at **298** twips and
                // row 0 — the header, the one row holding no `text:a` — at **276**; rewriting
                // every `<text:a>` to its own text makes all 201 of them 276. On
                // `Special-Procedures_2025-07-10.ods`, which holds no hyperlink and five
                // conditional formats over whole columns, rows 6–200 come back at 298 and
                // deleting the `calcext:conditional-formats` element makes them 276. Both
                // documents are 11 pt Calibri throughout, where the arithmetic is
                // `trunc(220 × 1.18) + 40 − 23 = 276` and one line is
                // `(14 + 4 + 1 + 1) px / 0.067 = 298`.
                bool standing = !breaks && !turned && !opaque
                                && (portions is { Count: > 0 }
                                    || text.AsSpan().IndexOfAny('\n', '\r') >= 0
                                    || sheet.HoldsField(cell.Row, cell.Column)
                                    || sheet.CarriesCondition(cell.Row, cell.Column));

                if (!breaks && !turned && !opaque && !standing) continue;

                rows.TryGetValue(cell.Row, out RowState state);

                int measured = opaque
                    ? 0
                    : direct
                        ? RotatedHeight(format, text, breaks)
                        : standing
                            ? StandingEditLine(
                                format, portions,
                                ParagraphsOf(text, sheet.CellBreaksStartLines))
                            : WrappedHeight(
                                cell, format, text, portions, columns, sheet.MergedRanges);

                rows[cell.Row] = state with
                {
                    Measured = Math.Max(state.Measured, measured),
                    Unmeasurable = state.Unmeasurable || opaque
                                   || ((breaks || direct) && measured == 0),
                };
            }
        }

        return rows;
    }

    /// <summary>
    /// Whether the cell's <em>orientation</em> — rather than its angle — is not Calc's standard.
    /// </summary>
    /// <remarks>
    /// <c>ScPatternAttr::GetCellOrientation</c> (<c>patattr.cxx:529-547</c>) reads exactly 9000 and
    /// exactly 27000 as <c>BottomUp</c> and <c>TopBottom</c>; every other angle stays
    /// <c>Standard</c> and is carried as a rotation instead. The distinction decides two separate
    /// things below, so it is named once.
    /// </remarks>
    private static bool IsQuarterTurned(SheetCellFormat format)
        => format.IsStacked || Math.Abs(format.RotationDegrees) == 90;

    /// <summary>
    /// How many times its own font height a turned cell that wraps is allowed to be.
    /// </summary>
    /// <remarks><c>SC_ROT_BREAK_FACTOR</c>, <c>sc/source/core/data/column2.cxx:74</c>.</remarks>
    private const int RotatedBreakFactor = 6;

    /// <summary>
    /// The height a turned cell asks for, in twips, or zero when its face cannot be resolved.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The direct-output branch of <c>ScColumn::GetNeededSize</c>
    /// (<c>column2.cxx:311-370</c>), which is where a plain string in a turned cell is measured.
    /// <strong>Nothing is wrapped here</strong>: the whole string is measured on one line and its
    /// width is put through the angle, so a turned cell's row grows without bound with its text.
    /// The wrap flag reaches the answer only as the cap below.
    /// </para>
    /// <para>
    /// <strong>Ninety degrees is not a rotation at all, and that is the finding that decides this
    /// method's shape.</strong> <c>ScPatternAttr::GetCellOrientation</c>
    /// (<c>sc/source/core/data/patattr.cxx:529-547</c>) turns exactly 9000 and exactly 27000 into
    /// <c>SvxCellOrientation::BottomUp</c> and <c>TopBottom</c>, and <c>nRotate</c> is then read
    /// only when the orientation came back <c>Standard</c> (<c>column2.cxx:231-238</c>). So the
    /// quarter turns take the orientation branch — height is the text's width, full stop — and
    /// every other angle takes the rotation branch, where the cap applies. Measured on the probe:
    /// a wrapping cell at 45° stops at 1373 twips for eleven point however long its text is, and
    /// the same cell at 90° reaches 7358.
    /// </para>
    /// <para>
    /// The quantisation is the same 96 dpi virtual device <see cref="WrappedHeight"/> measures
    /// against: the em is set in whole pixels, the string's width comes back in whole pixels, and
    /// the margins truncate to one pixel each.
    /// </para>
    /// </remarks>
    /// <param name="format">The cell's format, for its face, size and angle.</param>
    /// <param name="text">Its text, measured on one line.</param>
    /// <param name="breaks">Whether the cell wraps, which is what arms the cap.</param>
    private static int RotatedHeight(SheetCellFormat format, string text, bool breaks)
    {
        if (SheetFonts.For(format) is not { } face) return 0;
        if (face.Metrics.UnitsPerEm <= 0) return 0;

        Length size = format.FontSize;
        if (size <= Length.Zero) return 0;

        MetricGrid grid = new(ScreenDpi);
        long width = SheetText.MeasurePixels(text, face, grid.ToEmSize(size), TwipsPerPixel);
        if (width <= 0) return 0;

        long pixels;
        if (IsQuarterTurned(format))
        {
            // The orientation branch (`column2.cxx:311-316`): width and height are swapped and
            // neither the angle nor the wrap flag is consulted again.
            pixels = width;
        }
        else
        {
            double radians = format.RotationDegrees * Math.PI / 180.0;
            double cosine = Math.Abs(Math.Cos(radians));
            double sine = Math.Abs(Math.Sin(radians));

            pixels = (long)((LinePixels(face, size, grid) * cosine) + (width * sine));

            if (breaks)
            {
                long em = grid.ToEmSize(size).Twips / TwipsPerPixel;
                pixels = Math.Min(pixels, em * RotatedBreakFactor);
            }
        }

        return pixels <= 0 ? 0 : (int)((pixels + (2 * MarginPixelsOf(format))) / PixelsPerTwip);
    }

    /// <summary>
    /// The height a wrapping cell asks for, in twips, or zero when its face cannot be resolved.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The EditEngine branch of <c>ScColumn::GetNeededSize</c>
    /// (<c>sc/source/core/data/column2.cxx:409-600</c>), reproduced with the quantisation that
    /// makes it Calc's answer rather than a more accurate one. Three roundings decide it and all
    /// three are device pixels on a 96 dpi virtual device: the em size is rounded to whole pixels,
    /// the ascent and the descent are each rounded to whole pixels at that em, and the cell's top
    /// and bottom margins truncate to one pixel each. That is exactly
    /// <see cref="XRay.Office.Text.Fonts.MetricGrid"/>, which Writer already had for
    /// <c>fUsePrinterMetrics</c> — the same rounding on a different device.
    /// </para>
    /// <para>
    /// Derived from thirty probe rows: six font sizes against five wrapped line counts, exported
    /// through LibreOffice 24.2.7.2's flat ODF. Every one of the thirty is reproduced exactly, and
    /// so is the odd one out — 18 pt, whose single word is wider than the column and takes two
    /// lines, giving 805 twips where the arithmetic alone asks for 441.
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, round 54 — re-run against the installed
    /// <strong>26.2.4.2</strong> on a freshly authored thirty of the same shape: six sizes (8, 10,
    /// 11, 12, 14, 18 pt) against one to five words that cannot share a line, no <c>ht</c> and no
    /// <c>customHeight</c> anywhere so Calc recomputes. <strong>30 of 30 agree to under half a
    /// twip</strong>, the largest disagreement being 0.05. The instrument is two independent
    /// readings of the same number — the y of an identical six-point marker in the next row less
    /// this row's, off both PDFs, and the reference's own <c>--convert-to fods</c>
    /// <c>style:row-height</c> — and the two agree to 0.6 twips throughout. The control ran first
    /// and passed: the twelve-point single-line row reads <strong>300</strong> twips, which is the
    /// figure this remark already states for <c>National-Reports.xlsx</c>. Twenty-four of the
    /// thirty rows are genuinely multi-line and so exercise this method rather than
    /// <c>lcl_GetAttribHeight</c>. <c>dotnet/probes/sheets-r54/audit_rowheight.py</c>.]
    /// </para>
    /// <para>
    /// A cell whose text is not all in one format is measured the same way with one difference:
    /// its lines are summed rather than counted, because EditEngine makes each line as tall as the
    /// tallest portion standing on it. See <see cref="RichPixels"/>.
    /// </para>
    /// </remarks>
    private static int WrappedHeight(
        ContentTableCell cell,
        SheetCellFormat format,
        string text,
        IReadOnlyList<SheetTextPortion>? portions,
        SheetAxis columns,
        IReadOnlyList<SheetRange> merges)
    {
        if (SheetFonts.For(format) is not { } face) return 0;
        if (face.Metrics.UnitsPerEm <= 0) return 0;

        Length size = format.FontSize;
        if (size <= Length.Zero) return 0;

        long width = 0;
        int span = Math.Max(1, SpanOf(cell, merges));
        for (int column = cell.Column; column < cell.Column + span; column++)
            width += columns.SizeAt(column).Twips;

        // `aPaper.setWidth(nDocWidth)`: the column in whole pixels, less a pixel of margin either
        // side, less the one the gridline takes — "output size is width-1 pixel (due to gridline)"
        // (`column2.cxx:466-470`). A left- or right-aligned indent comes off as well.
        double horizontal = PixelsPerTwip / OutputFactor;
        long paper = (long)(width * horizontal)
                     - (2 * (long)(format.Margin.Twips * horizontal)) - 1;
        if (format.Horizontal is SheetHorizontalAlignment.Left or SheetHorizontalAlignment.Right)
            paper -= (long)(format.Indent.Twips * horizontal);

        if (paper <= 0) return 0;

        Length available = Length.FromTwips(paper * TwipsPerPixel);
        MetricGrid grid = new(ScreenDpi);

        long pixels = portions is { Count: > 0 }
            ? RichPixels(text, portions, face, size, available, grid)
            : PlainPixels(text, face, size, available, grid);

        return pixels <= 0 ? 0 : (int)((pixels + (2 * MarginPixelsOf(format))) / PixelsPerTwip);
    }

    /// <summary>The pixels a cell in one face needs: its line count times one line.</summary>
    private static long PlainPixels(
        string text, SheetFace face, Length size, Length available, MetricGrid grid)
    {
        int lines = SheetTextLayout.LineCount(text, face, grid.ToEmSize(size), available);
        if (lines <= 0) return 0;

        long line = LinePixels(face, size, grid);
        return line <= 0 ? 0 : lines * line;
    }

    /// <summary>
    /// The pixels a cell in several formats needs, summed line by line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// EditEngine does not give a paragraph one pitch. It walks the portions of each line, keeps
    /// the largest ascent and the largest descent it saw separately, and makes the line their sum
    /// — <c>ImpEditEngine::CreateLines</c> (<c>editeng/source/editeng/impedit3.cxx:1500-1519</c>)
    /// over <c>RecalcFormatterFontMetrics</c> (<c>:3159-3163</c>). So a line carrying one word in
    /// a larger face is taller than its neighbours, and a cell's height is a sum rather than a
    /// product.
    /// </para>
    /// <para>
    /// A line no portion covers is the empty paragraph a trailing hard break leaves, and it takes
    /// the cell's own font — which is what EditEngine does with a paragraph holding no portion of
    /// its own.
    /// </para>
    /// </remarks>
    private static long RichPixels(
        string text,
        IReadOnlyList<SheetTextPortion> portions,
        SheetFace face,
        Length size,
        Length available,
        MetricGrid grid)
    {
        long total = 0;

        foreach ((int start, int end) in
                 SheetTextLayout.RichLineRanges(text, portions, face, available, grid))
        {
            long ascent = 0;
            long descent = 0;

            foreach (SheetTextPortion portion in portions)
            {
                if (portion.End <= start || portion.Start >= end) continue;
                if (SheetFonts.For(portion.Format) is not { } portionFace) return 0;
                if (portionFace.Metrics.UnitsPerEm <= 0) return 0;

                Length portionSize = portion.Format.FontSize;
                if (portionSize <= Length.Zero) return 0;

                ascent = Math.Max(
                    ascent,
                    grid.ToPixels(
                        portionFace.Metrics.Ascent, portionFace.Metrics.UnitsPerEm, portionSize));
                descent = Math.Max(
                    descent,
                    grid.ToPixels(
                        portionFace.Metrics.Descent, portionFace.Metrics.UnitsPerEm, portionSize));
            }

            long line = ascent + descent > 0 ? ascent + descent : LinePixels(face, size, grid);
            if (line <= 0) return 0;

            total += line;
        }

        return total;
    }

    /// <summary>
    /// The height an edit cell that does <em>not</em> wrap asks for, in twips.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A cell whose string carries formatting runs or a hard break is stored as an
    /// <c>EditTextObject</c> by both importers, and <c>ScColumn::GetOptimalHeight</c> clears
    /// <c>bStdOnly</c> for one (<c>column2.cxx:930-935</c>), so it is measured through
    /// <c>GetNeededSize</c>'s EditEngine branch whatever its wrap flag says. Nothing is broken
    /// for <em>width</em> there — the paper is a million units across — so the answer is one
    /// EditEngine line per paragraph: the largest ascent and the largest descent over its
    /// portions, each quantised to whole device pixels, plus a pixel of margin either side. That
    /// is <see cref="RichPixels"/> over a single range, written out here rather than routed
    /// through <see cref="WrappedHeight"/> because there is no paper to compute and no line to
    /// find.
    /// </para>
    /// <para>
    /// <strong>How many paragraphs there are is the importer's answer and not the string's.</strong>
    /// The BIFF and SpreadsheetML filters set <c>SetSingleLine</c> for a cell that does not wrap,
    /// so its hard break stays inside one paragraph and the height is one line; Calc's ODF filter
    /// never calls it, so the same characters are several paragraphs and
    /// <c>pEngine-&gt;GetTextHeight()</c> is that many lines. Measured on
    /// <c>dotnet/tests/corpus/features/sheet-cell-break-height.fods</c>, whose row 2 holds three
    /// paragraphs in a 6 cm non-wrapping cell: 26.2.4.2 starts row 3 <strong>34.61 pt</strong>
    /// below row 2 — three lines of 11.197 pt and a margin — against the 12.39 pt a single line
    /// gives.
    /// </para>
    /// <para>
    /// It is not the arithmetic height and the gap is not small. Measured on
    /// <c>dotnet/probes/sheets-rest-01/mkclipprobe.py</c>'s auto-height workbook under the
    /// installed 26.2.4.2, one string in a non-wrapping Calibri 11 cell: plain it takes
    /// <strong>276</strong> twips — <c>trunc(220 × 1.18) + 40 − 23</c>, the arithmetic answer
    /// exactly — and rich, hard-broken or both it takes <strong>298</strong>.
    /// <c>SIL_TDB648.xlsx</c>'s "TerrDB Verification" sheet is the corpus case: 285 twips a row
    /// for its plain rows and 298 for its rich footnote rows, and getting the second wrong by
    /// 13 twips a row is enough to fit two extra rows on a page.
    /// </para>
    /// <para>
    /// <strong>Two earlier cuts of that probe measured nothing, in two different ways, and both
    /// are worth knowing.</strong> The first was set in Arial 9, whose line is shorter than
    /// <c>ScGlobal::nStdRowHeight</c>, so every row came back at the 256-twip floor and plain and
    /// rich looked identical — a probe for a height needs a font tall enough that the floor does
    /// not bind. The second stated <c>fontId="0"</c> with no <c>applyFont</c> and no
    /// <c>cellStyles</c>, so LibreOffice drew the plain rows in its own application default while
    /// the rich rows took the face their <c>rPr</c> named; that one said a hard break did
    /// <em>not</em> reach this path, which is the opposite of the truth.
    /// </para>
    /// </remarks>
    /// <param name="format">The cell's format, for its face, size and margins.</param>
    /// <param name="portions">Its formatting runs, or null when the break alone made it an edit cell.</param>
    /// <param name="paragraphs">
    /// How many paragraphs the importer made of the cell. One for the two Excel families, whose
    /// filters put the engine into single-line mode, and one per hard break for ODF — see
    /// <see cref="SheetLayout.CellBreaksStartLines"/>.
    /// </param>
    private static int StandingEditLine(
        SheetCellFormat format, IReadOnlyList<SheetTextPortion>? portions, int paragraphs)
    {
        MetricGrid grid = new(ScreenDpi);
        long ascent = 0;
        long descent = 0;

        foreach (SheetCellFormat run in Faces(format, portions))
        {
            if (SheetFonts.For(run) is not { } face) return 0;
            if (face.Metrics.UnitsPerEm <= 0) return 0;

            Length size = run.FontSize;
            if (size <= Length.Zero) return 0;

            ascent = Math.Max(ascent, grid.ToPixels(face.Metrics.Ascent, face.Metrics.UnitsPerEm, size));
            descent = Math.Max(
                descent, grid.ToPixels(face.Metrics.Descent, face.Metrics.UnitsPerEm, size));
        }

        long pixels = (ascent + descent) * Math.Max(1, paragraphs);
        return pixels <= 0 ? 0 : (int)((pixels + (2 * MarginPixelsOf(format))) / PixelsPerTwip);
    }

    /// <summary>How many paragraphs the importer made of a cell's text.</summary>
    /// <remarks>
    /// The same <c>\n</c>/<c>\r\n</c>/<c>\r</c> split every other count of a cell's paragraphs
    /// makes, and it answers one whenever the importer left the engine in single-line mode.
    /// </remarks>
    private static int ParagraphsOf(string text, bool breaksStartLines)
    {
        if (!breaksStartLines) return 1;

        int count = 1;
        for (int at = 0; at < text.Length; at++)
        {
            if (text[at] == '\n') count++;
            else if (text[at] == '\r') { count++; if (at + 1 < text.Length && text[at + 1] == '\n') at++; }
        }

        return count;
    }

    /// <summary>
    /// The formats the one line is measured over: the portions when there are any, else the cell's.
    /// </summary>
    /// <remarks>
    /// EditEngine gives a paragraph with no portion of its own the cell's font, which is what a
    /// hard-broken but singly formatted cell is.
    /// </remarks>
    private static IEnumerable<SheetCellFormat> Faces(
        SheetCellFormat format, IReadOnlyList<SheetTextPortion>? portions)
    {
        if (portions is not { Count: > 0 }) return [format];
        return portions.Select(portion => portion.Format);
    }

    /// <summary>One line of a face at a size, in whole device pixels.</summary>
    private static long LinePixels(SheetFace face, Length size, MetricGrid grid)
        => grid.ToPixels(face.Metrics.Ascent, face.Metrics.UnitsPerEm, size)
           + grid.ToPixels(face.Metrics.Descent, face.Metrics.UnitsPerEm, size);

    /// <summary>How many columns a cell covers, taking the widest merge it anchors.</summary>
    private static int SpanOf(ContentTableCell cell, IReadOnlyList<SheetRange> merges)
    {
        int span = Math.Max(1, cell.ColumnSpan);
        foreach (SheetRange merge in merges)
        {
            if (merge.FirstRow != cell.Row || merge.FirstColumn != cell.Column) continue;
            span = Math.Max(span, merge.LastColumn - merge.FirstColumn + 1);
        }

        return span;
    }

    private static bool IsExcludedByMerge(IReadOnlyList<SheetRange> merges, int row, int column)
    {
        foreach (SheetRange merge in merges)
        {
            if (row < merge.FirstRow || row > merge.LastRow) continue;
            if (column < merge.FirstColumn || column > merge.LastColumn) continue;

            // The anchor of a merge one row tall is the one cell Calc still considers.
            bool anchor = row == merge.FirstRow && column == merge.FirstColumn;
            if (anchor && merge.FirstRow == merge.LastRow) continue;
            return true;
        }

        return false;
    }

    /// <summary>
    /// The rows that state a format for every column of a span, so nothing in them falls through
    /// to a column or sheet default.
    /// </summary>
    /// <remarks>
    /// Asked as intervals rather than as a count of cells, because the span reaches every
    /// allocated column and a repeated rectangle covering most of them must not be unfolded to
    /// answer it. A row's intervals are merged in place; a row Calc would find one uncovered
    /// column in is one whose default pattern's arithmetic height is a floor for it.
    /// </remarks>
    private static IEnumerable<int> RowsStatingEveryColumn(
        SheetCellFormats formats, IReadOnlyList<SheetRange> merges,
        int firstColumn, int lastColumn, int lastRow)
    {
        if (lastColumn < firstColumn) yield break;

        Dictionary<int, List<(int First, int Last)>> spans = [];
        foreach ((int row, int first, int last)
                 in formats.StatedSpans(0, lastRow, firstColumn, lastColumn))
        {
            if (!spans.TryGetValue(row, out List<(int, int)>? runs)) spans[row] = runs = [];
            runs.Add((first, last));
        }

        foreach ((int row, List<(int First, int Last)> runs) in spans)
        {
            runs.Sort(static (left, right) => left.First.CompareTo(right.First));

            int reached = firstColumn - 1;
            foreach ((int first, int last) in runs)
            {
                if (first > reached + 1) break;
                reached = Math.Max(reached, last);
            }

            if (reached >= lastColumn && !IsMergeCoveredInSpan(merges, row, firstColumn, lastColumn))
                yield return row;
        }
    }

    /// <summary>
    /// Whether any column of a span is one a merge takes out of the row-height walk.
    /// </summary>
    /// <remarks>
    /// Kept deliberately, and it is <em>not</em> what Calc does. A pattern a merge covers is
    /// skipped whole by <c>ScColumn::GetOptimalHeight</c> (`column2.cxx`:917-925) — neither its own
    /// height nor the default's is written from that column — so a row whose remaining columns all
    /// state a format is, in Calc's terms, covered. Treating it that way is *shorter*, and measured
    /// over the original corpus it takes `093_Volunteer_Sign_Up_Sheet_Template_Customizable_Format`
    /// from 2 pages to 1 against 26.2.4.2's 2. That workbook is 81 merges over 139 rows, and
    /// something else in how a merged row is measured is evidently leaning on the taller answer, so
    /// the conservative reading is kept until that is found: a merge-covered column keeps the
    /// sheet default in play exactly as it did before this walk replaced the cell count.
    /// </remarks>
    private static bool IsMergeCoveredInSpan(
        IReadOnlyList<SheetRange> merges, int row, int firstColumn, int lastColumn)
    {
        foreach (SheetRange merge in merges)
        {
            if (row < merge.FirstRow || row > merge.LastRow) continue;
            if (merge.LastColumn < firstColumn || merge.FirstColumn > lastColumn) continue;

            for (int column = Math.Max(merge.FirstColumn, firstColumn);
                 column <= Math.Min(merge.LastColumn, lastColumn);
                 column++)
            {
                if (IsExcludedByMerge(merges, row, column)) return true;
            }
        }

        return false;
    }

    /// <summary>
    /// The height a cell's font asks for without measuring anything.
    /// </summary>
    /// <remarks>
    /// <c>lcl_GetAttribHeight</c>, <c>sc/source/core/data/column2.cxx:866-892</c>. The truncation
    /// is Calc's: the running value is a <c>sal_uInt16</c>, so <c>nHeight *= 1.18</c> throws the
    /// fraction away before the margins are added — 240 twips becomes 283 rather than 283.2, and a
    /// 12 pt row lands on 300 twips exactly.
    /// </remarks>
    private static int AttributeHeight(SheetCellFormat format, int minimum)
    {
        int height = (int)(format.FontSize.Twips * FontHeightFactor);
        height += VerticalMarginTwipsOf(format);
        if (height > StandardRowHeightDifference) height -= StandardRowHeightDifference;
        return Math.Max(height, minimum);
    }
}

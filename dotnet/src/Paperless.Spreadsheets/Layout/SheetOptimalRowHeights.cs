using Paperless.Core.Extraction;
using Paperless.Core.Units;
using Paperless.Text.Fonts;

namespace Paperless.Spreadsheets.Layout;

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
/// Paperless does, measured off its own PDF — and only the row it reserved for it disagrees.
/// </para>
/// <para>
/// What is still not reproduced is a turned or stacked cell, whose size is its text's *width* put
/// through an angle. A row holding one takes the larger of the arithmetic answer and the
/// height its file already states. That fallback is not a fudge — the arithmetic answer really is
/// a lower bound in Calc too, because <c>bStdAllowed</c> stays true for such a cell and its
/// attribute height is written into the array before any measurement is compared against it — and
/// it means a row this cannot measure is never shorter than the writer made it.
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
    /// A cell's margin on any one side, in twips.
    /// </summary>
    /// <remarks>
    /// Calc's default <c>ATTR_MARGIN</c> is 20 twips on all four sides
    /// (<c>SvxMarginItem</c>'s default constructor, <c>svx/source/items/algitem.cxx:123-132</c>),
    /// and none of the three readers reads a cell's vertical margins — no format states one that
    /// Paperless keeps, and the indent it does keep is horizontal. So this is a constant rather
    /// than a lookup, and it is the constant that turns a 12 pt font's 283 twips into the 300 Calc
    /// writes.
    /// </remarks>
    private const int CellMarginTwips = 20;

    /// <summary>
    /// A cell's top and bottom margins, together, in twips.
    /// </summary>
    /// <remarks>
    /// The pair, because the arithmetic height adds both at once. The horizontal pair is not the
    /// same number of *pixels*: it comes off the paper as two truncations of
    /// <c>CellMarginTwips × nPPTX</c>, and <c>nPPTX</c> has been divided by
    /// <see cref="OutputFactor"/>.
    /// </remarks>
    private const int VerticalMarginTwips = 2 * CellMarginTwips;

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
    /// of them is worth 2 pixels rather than the 2.68 the twips would give.
    /// </remarks>
    private const int MarginPixels = 1;

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

        SheetRange range = SheetDecorationArea.Extend(
            sheet.UsedRange, sheet.Formatting, sheet.LastDataRowByColumn);
        if (!range.IsValid) return grid;

        int lastRow = range.LastRow;
        int firstColumn = range.FirstColumn;
        int lastColumn = range.LastColumn;
        if (lastRow < 0 || lastColumn < firstColumn) return grid;

        SheetCellFormats formats = sheet.Formats;

        // The columns a cell states nothing about resolve to the column's format, then to the
        // sheet's. Neither depends on the row, so both are folded once rather than per row.
        int baseline = AttributeHeight(formats.SheetDefault);
        foreach (SheetCellFormat column in formats.ColumnDefaults(firstColumn, lastColumn))
            baseline = Math.Max(baseline, AttributeHeight(column));

        Dictionary<int, RowState> rows = CollectRows(sheet, formats, range, grid.Columns);

        SheetAxis axis = grid.Rows;
        int minimum = (int)grid.OptimalMinimumRowHeight.Twips;
        Dictionary<int, int> changes = [];
        int index = 0;

        while (index <= lastRow)
        {
            // A height the user chose is honoured, and it also ends the run Calc would have
            // grouped: `SetOptimalHeightsToRows` flushes its pending range at a manual row
            // (`sc/source/core/data/table1.cxx:221-226`).
            if (!axis.IsOptimalSize(index)) { index++; continue; }

            int height = Optimal(rows, index, baseline, minimum, axis);
            int last = index;
            while (last + 1 <= lastRow && axis.IsOptimalSize(last + 1)
                   && Optimal(rows, last + 1, baseline, minimum, axis) == height)
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

    /// <summary>The height one row asks for, in twips.</summary>
    private static int Optimal(
        Dictionary<int, RowState> rows, int row, int baseline, int minimum, SheetAxis axis)
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

        return Math.Max(height, minimum);
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
        SheetLayout sheet, SheetCellFormats formats, SheetRange range, SheetAxis columns)
    {
        Dictionary<int, RowState> rows = [];
        Dictionary<int, int> stated = [];

        void Contribute(int row, SheetCellFormat format)
        {
            rows.TryGetValue(row, out RowState state);
            rows[row] = state with
            {
                Attribute = Math.Max(state.Attribute, format.IsStacked ? 0 : AttributeHeight(format)),
            };
        }

        IReadOnlyList<SheetRange> merges = sheet.MergedRanges;

        foreach ((int row, int column, SheetCellFormat format) in formats.Cells)
        {
            if (row < 0 || row > range.LastRow) continue;
            if (column < range.FirstColumn || column > range.LastColumn) continue;

            // Calc skips a cell that a merge covers and a merge anchor that spans rows, and takes
            // only the anchor of a purely horizontal one — `ScColumn::GetOptimalHeight`,
            // `sc/source/core/data/column2.cxx:917-925`.
            if (IsExcludedByMerge(merges, row, column)) continue;

            Contribute(row, format);
            stated[row] = stated.GetValueOrDefault(row) + 1;
        }

        int width = range.LastColumn - range.FirstColumn + 1;
        foreach ((int row, int count) in stated)
        {
            if (count < width) continue;
            rows[row] = rows[row] with { CoversEveryColumn = true };
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

                string text = cell.GetText();
                if (text.Length == 0) continue;

                SheetCellFormat format = formats.At(cell.Row, cell.Column);

                // The same wrap decision the drawing path makes, so that a row is measured exactly
                // when its text will be broken — including Calc's rule that a plain number never
                // breaks however the cell is formatted.
                // A hyperlink cell is one EditEngine field, and a field is never broken across
                // lines — so it is measured at one line however narrow its column is. Missing
                // that makes a column of URLs four or five times too tall.
                bool breaks =
                    (SheetTextLayout.Breaks(format, cell.Value is not null and not string)
                     || text.AsSpan().IndexOfAny('\n', '\r') >= 0)
                    && !sheet.HoldsField(cell.Row, cell.Column);

                // A turned or stacked cell takes a path through `GetNeededSize` this does not
                // reproduce: its size is the text's *width* turned through an angle.
                bool opaque = format.IsStacked || format.RotationDegrees != 0;

                if (!breaks && !opaque) continue;

                rows.TryGetValue(cell.Row, out RowState state);

                int measured = opaque
                    ? 0
                    : WrappedHeight(
                        cell,
                        format,
                        text,
                        sheet.RichText.At(cell.Row, cell.Column, text),
                        columns,
                        sheet.MergedRanges);

                rows[cell.Row] = state with
                {
                    Measured = Math.Max(state.Measured, measured),
                    Unmeasurable = state.Unmeasurable || opaque || (breaks && measured == 0),
                };
            }
        }

        return rows;
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
    /// <see cref="Paperless.Text.Fonts.MetricGrid"/>, which Writer already had for
    /// <c>fUsePrinterMetrics</c> — the same rounding on a different device.
    /// </para>
    /// <para>
    /// Derived from thirty probe rows: six font sizes against five wrapped line counts, exported
    /// through LibreOffice 24.2.7.2's flat ODF. Every one of the thirty is reproduced exactly, and
    /// so is the odd one out — 18 pt, whose single word is wider than the column and takes two
    /// lines, giving 805 twips where the arithmetic alone asks for 441.
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
                     - (2 * (long)(CellMarginTwips * horizontal)) - 1;
        if (format.Horizontal is SheetHorizontalAlignment.Left or SheetHorizontalAlignment.Right)
            paper -= (long)(format.Indent.Twips * horizontal);

        if (paper <= 0) return 0;

        Length available = Length.FromTwips(paper * TwipsPerPixel);
        MetricGrid grid = new(ScreenDpi);

        long pixels = portions is { Count: > 0 }
            ? RichPixels(text, portions, face, size, available, grid)
            : PlainPixels(text, face, size, available, grid);

        return pixels <= 0 ? 0 : (int)((pixels + (2 * MarginPixels)) / PixelsPerTwip);
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
    /// The height a cell's font asks for without measuring anything.
    /// </summary>
    /// <remarks>
    /// <c>lcl_GetAttribHeight</c>, <c>sc/source/core/data/column2.cxx:866-892</c>. The truncation
    /// is Calc's: the running value is a <c>sal_uInt16</c>, so <c>nHeight *= 1.18</c> throws the
    /// fraction away before the margins are added — 240 twips becomes 283 rather than 283.2, and a
    /// 12 pt row lands on 300 twips exactly.
    /// </remarks>
    private static int AttributeHeight(SheetCellFormat format)
    {
        int height = (int)(format.FontSize.Twips * FontHeightFactor);
        height += VerticalMarginTwips;
        if (height > StandardRowHeightDifference) height -= StandardRowHeightDifference;
        return height;
    }
}

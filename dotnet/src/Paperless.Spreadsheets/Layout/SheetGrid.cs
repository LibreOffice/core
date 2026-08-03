using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// One stretch of consecutive columns or rows that share a size and a visibility.
/// </summary>
/// <param name="First">The first index the run covers.</param>
/// <param name="Last">The last index it covers, inclusive.</param>
/// <param name="Size">The width of each column, or the height of each row, in the run.</param>
/// <param name="IsHidden">True when every column or row in the run is hidden.</param>
/// <param name="IsOptimalSize">
/// True when the size is the authoring application's own computation rather than something a
/// user set, so a reader is free to recompute it from what the cells hold.
/// <para>
/// Calc calls the opposite of this <c>CRFlags::ManualSize</c> and honours it on load: a row
/// without it is recomputed by <c>ScDocRowHeightUpdater</c> before anything is drawn, so the
/// height in the file is a cache rather than a statement. All three formats carry the flag —
/// ODF's <c>style:use-optimal-row-height</c>, SpreadsheetML's <c>customHeight</c> and BIFF's
/// <c>fUnsynced</c> — and the default when a file states nothing is "not manual", which is why
/// a hand-written sheet gets rows sized by its content and one LibreOffice wrote does not
/// change.
/// </para>
/// </param>
public readonly record struct SheetSizeRun(
    int First, int Last, Length Size, bool IsHidden, bool IsOptimalSize = false)
{
    /// <summary>How many columns or rows the run covers.</summary>
    public int Count => Last >= First ? Last - First + 1 : 0;
}

/// <summary>
/// A sheet's column widths and row heights, kept run-length.
/// </summary>
/// <remarks>
/// <para>
/// Run-length because that is how all three formats state them and because materialising them
/// per column or per row would be catastrophic on the axis that matters. A SpreadsheetML
/// <c>&lt;col&gt;</c> element carries <c>min</c> and <c>max</c> and routinely covers all
/// 16 384 columns; ODF's <c>table:number-columns-repeated</c> does the same; and a sheet has
/// 1 048 576 rows, essentially all of them at the default height. Calc itself keeps the row
/// heights in a segment tree for exactly this reason
/// (<c>ScFlatUInt16RowSegments</c>, <c>sc/inc/table.hxx:192</c>) and gets away with a flat
/// array for the columns only because there are so few of them.
/// </para>
/// <para>
/// The runs are stored sorted and non-overlapping, and anything they do not cover takes the
/// default. So a sheet that states nothing costs two empty lists, and the <c>zeroHeight</c>
/// workbook that hides every row costs one run.
/// </para>
/// </remarks>
public sealed class SheetAxis
{
    private readonly List<SheetSizeRun> _runs;

    /// <summary>Creates an axis from sorted, non-overlapping runs.</summary>
    /// <param name="defaultSize">The size of anything no run covers.</param>
    /// <param name="runs">The runs, which are sorted and merged on construction.</param>
    public SheetAxis(Length defaultSize, IEnumerable<SheetSizeRun>? runs = null)
    {
        DefaultSize = defaultSize;
        _runs = Normalise(runs);
    }

    // Reversed parameters so that this can never be reached by overload resolution from the
    // public constructor's call shape, which normalises and must stay the only way in from
    // outside.
    private SheetAxis(List<SheetSizeRun> runs, Length defaultSize)
    {
        DefaultSize = defaultSize;
        _runs = runs;
    }

    /// <summary>
    /// An axis from runs already known to be sorted, non-overlapping and neighbour-merged.
    /// </summary>
    /// <remarks>
    /// The public constructor normalises, and normalising is quadratic in the number of runs
    /// because a later run may cut a hole in any earlier one. A sheet that states a height for
    /// every one of its rows already has thousands of runs, so a caller that rebuilds the axis
    /// row by row — recomputing hinted heights is the one that does — must not pay that again for
    /// a list it has just built in order.
    /// </remarks>
    /// <param name="defaultSize">The size of anything no run covers.</param>
    /// <param name="runs">The runs, in index order, taken as given.</param>
    internal static SheetAxis FromOrdered(Length defaultSize, List<SheetSizeRun> runs)
        => new(runs, defaultSize);

    /// <summary>The size of a column or row no run covers.</summary>
    public Length DefaultSize { get; }

    /// <summary>The runs, in index order, with neither gaps overlapping nor runs touching.</summary>
    public IReadOnlyList<SheetSizeRun> Runs => _runs;

    /// <summary>The size of one column or row.</summary>
    /// <param name="index">The zero-based column or row index.</param>
    public Length SizeAt(int index)
    {
        int at = Find(index);
        return at < 0 ? DefaultSize : _runs[at].Size;
    }

    /// <summary>True when a column or row is hidden, and therefore contributes no size.</summary>
    /// <param name="index">The zero-based column or row index.</param>
    public bool IsHidden(int index)
    {
        int at = Find(index);
        return at >= 0 && _runs[at].IsHidden;
    }

    /// <summary>
    /// True when the stated size is the application's own computation rather than a user's.
    /// </summary>
    /// <remarks>
    /// True for anything no run covers, because a row a file never mentions has no manual height
    /// by definition. See <see cref="SheetSizeRun.IsOptimalSize"/>.
    /// </remarks>
    /// <param name="index">The zero-based column or row index.</param>
    public bool IsOptimalSize(int index)
    {
        int at = Find(index);
        return at < 0 || _runs[at].IsOptimalSize;
    }

    /// <summary>
    /// The size a column or row occupies on a page: zero when hidden, its size otherwise.
    /// </summary>
    /// <remarks>
    /// The distinction pagination cares about. Calc's break loop reads
    /// <c>ColHidden(nX) ? 0 : mpColWidth-&gt;GetValue(nX)</c>
    /// (<c>sc/source/core/data/table5.cxx:167</c>) — a hidden column keeps its width and
    /// contributes none of it, which is why the two are stored apart rather than a hidden
    /// column being given a width of zero.
    /// </remarks>
    /// <param name="index">The zero-based column or row index.</param>
    public Length PrintedSizeAt(int index)
    {
        int at = Find(index);
        if (at < 0) return DefaultSize;
        return _runs[at].IsHidden ? Length.Zero : _runs[at].Size;
    }

    /// <summary>
    /// The total printed size of a range, hidden entries excluded.
    /// </summary>
    /// <remarks>
    /// Walked by run rather than by index, so that summing a repeated header band across a
    /// sheet with a million default rows costs the number of runs rather than the number of
    /// rows.
    /// </remarks>
    /// <param name="first">The first index, inclusive.</param>
    /// <param name="last">The last index, inclusive.</param>
    public Length TotalPrintedSize(int first, int last)
    {
        if (last < first) return Length.Zero;

        long emu = 0;
        int at = first;
        while (at <= last)
        {
            int found = Find(at);
            if (found < 0)
            {
                // A gap between runs, or past the end: everything up to the next run takes the
                // default. NextRunStart never returns something at or below `at`.
                int until = Math.Min(last, NextRunStart(at) - 1);
                emu += DefaultSize.Emu * (until - at + 1);
                at = until + 1;
                continue;
            }

            SheetSizeRun run = _runs[found];
            int end = Math.Min(last, run.Last);
            if (!run.IsHidden) emu += run.Size.Emu * (end - at + 1);
            at = end + 1;
        }

        return Length.FromEmu(emu);
    }

    /// <summary>The index of the run covering an index, or a negative number when none does.</summary>
    private int Find(int index)
    {
        int low = 0;
        int high = _runs.Count - 1;
        while (low <= high)
        {
            int mid = (low + high) / 2;
            if (index < _runs[mid].First) high = mid - 1;
            else if (index > _runs[mid].Last) low = mid + 1;
            else return mid;
        }
        return -1;
    }

    /// <summary>Where the first run starting after an index begins, or <see cref="int.MaxValue"/>.</summary>
    private int NextRunStart(int index)
    {
        int low = 0;
        int high = _runs.Count - 1;
        int answer = int.MaxValue;
        while (low <= high)
        {
            int mid = (low + high) / 2;
            if (_runs[mid].First > index)
            {
                answer = _runs[mid].First;
                high = mid - 1;
            }
            else
            {
                low = mid + 1;
            }
        }
        return answer;
    }

    /// <summary>
    /// Sorts the runs, drops empty ones, and lets a later run win an overlap.
    /// </summary>
    /// <remarks>
    /// A later run winning is what the formats need: SpreadsheetML writes a default
    /// <c>&lt;col&gt;</c> spanning the sheet and then narrower ones over the top of it, and
    /// BIFF's <c>COLINFO</c> records do the same. Being lenient about overlap here is cheaper
    /// than making every reader resolve it.
    /// </remarks>
    private static List<SheetSizeRun> Normalise(IEnumerable<SheetSizeRun>? runs)
    {
        if (runs is null) return [];

        List<SheetSizeRun> ordered = [.. runs.Where(r => r.Count > 0)];
        if (ordered.Count == 0) return [];

        List<SheetSizeRun> result = [];
        foreach (SheetSizeRun run in ordered)
        {
            // Clip everything already placed against the newcomer, then add it. The lists are
            // short — one entry per distinct width — so the quadratic worst case never bites.
            List<SheetSizeRun> kept = new(result.Count + 2);
            foreach (SheetSizeRun existing in result)
            {
                if (existing.Last < run.First || existing.First > run.Last)
                {
                    kept.Add(existing);
                    continue;
                }

                if (existing.First < run.First)
                    kept.Add(existing with { Last = run.First - 1 });
                if (existing.Last > run.Last)
                    kept.Add(existing with { First = run.Last + 1 });
            }

            kept.Add(run);
            result = kept;
        }

        result.Sort((a, b) => a.First.CompareTo(b.First));

        // Merge neighbours that agree, which is what keeps a sheet whose every column was
        // written separately from carrying one run per column.
        List<SheetSizeRun> merged = [];
        foreach (SheetSizeRun run in result)
        {
            if (merged.Count > 0)
            {
                SheetSizeRun last = merged[^1];
                if (last.Last + 1 == run.First && last.Size == run.Size
                    && last.IsHidden == run.IsHidden && last.IsOptimalSize == run.IsOptimalSize)
                {
                    merged[^1] = last with { Last = run.Last };
                    continue;
                }
            }
            merged.Add(run);
        }

        return merged;
    }
}

/// <summary>
/// A sheet's geometry: how wide each column is and how tall each row.
/// </summary>
/// <remarks>
/// Extraction never needed either — a cell's text does not depend on the width of the column
/// it sits in — which is why this is new work rather than something the readers already had.
/// Pagination needs both, because they are the only thing a page's capacity is measured
/// against.
/// </remarks>
/// <param name="Columns">The column widths.</param>
/// <param name="Rows">The row heights.</param>
public sealed record SheetGrid(SheetAxis Columns, SheetAxis Rows)
{
    /// <summary>
    /// The column widths as the file still states them, when it states them in digits.
    /// </summary>
    /// <remarks>
    /// Null for ODF, which states a real length on every <c>table:table-column</c> and needs no
    /// font to become a measurement, and set by the three Excel readers, which do. See
    /// <see cref="SheetColumnDigits"/> for why the resolution is deferred rather than done while
    /// reading, and <see cref="WithDigitWidth"/> for where it lands.
    /// </remarks>
    public SheetColumnDigits? ColumnDigits { get; init; }

    /// <summary>
    /// The floor a recomputed row height is held to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Calc keeps this per sheet — <c>ScTable::GetOptimalMinRowHeight</c>
    /// (<c>sc/inc/table.hxx:882-887</c>) — and falls back to <c>ScGlobal::nStdRowHeight</c>, which
    /// is 256 twips, when nothing set it. **Only the OOXML filter sets it**, from the sheet's own
    /// <c>defaultRowHeight</c>: <c>pTable-&gt;SetOptimalMinRowHeight(maDefRowModel.mfHeight * 20)</c>
    /// (<c>sc/source/filter/oox/worksheethelper.cxx:965</c>). The BIFF and ODF filters do not, so
    /// a sheet from either is floored at 256 whatever its file says its default row is — which is
    /// why this is a property of its own rather than <see cref="SheetAxis.DefaultSize"/>, whose
    /// value the two happen to share for SpreadsheetML and do not for the other two.
    /// </para>
    /// <para>
    /// Excel's own default row height is exactly the height its default font asks for, so on a
    /// SpreadsheetML sheet the floor usually binds on nothing; it is what stops a sheet whose rows
    /// state a large <c>defaultRowHeight</c> from collapsing to a small font's measure.
    /// </para>
    /// </remarks>
    public Length OptimalMinimumRowHeight { get; init; } = StandardRowHeight;

    /// <summary>
    /// True when every row of the sheet is a user's choice, whatever its own flag says.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Only BIFF states this, and it states it in a place that is easy to read as being about
    /// something else. <c>DEFAULTROWHEIGHT</c> carries its own <c>fUnsynced</c> —
    /// <c>EXC_DEFROW_UNSYNCED</c>, <c>sc/source/filter/inc/xltable.hxx:114</c> — and
    /// <c>XclImpColRowSettings::Convert</c> answers it by marking <em>every row of the sheet</em>
    /// manual before it reads a single <c>ROW</c> record, with the comment "first access to row
    /// flags, do not ask for old flags" (<c>sc/source/filter/excel/colrowst.cxx:212-215</c>).
    /// Nothing later clears the bit — the per-row loop only ever sets it — so the sheet has no row
    /// Calc will re-measure, however its <c>ROW</c> records are flagged.
    /// </para>
    /// <para>
    /// Measured: recomputing without this cost eight <c>.xls</c> documents their page count across
    /// the sheets track and gained none, which is what led to the record being re-read. BIFF2's
    /// two-byte <c>DEFAULTROWHEIGHT</c> has no flags field and Calc passes the bit unconditionally
    /// (<c>ImportExcel::Defrowheight2</c>, <c>impop.cxx:598-604</c>).
    /// </para>
    /// </remarks>
    public bool RowHeightsAreManual { get; init; }

    /// <summary>
    /// The same grid with its columns measured in a font whose digit is worth so many twips.
    /// </summary>
    /// <remarks>
    /// A no-op when the widths were already lengths, so a caller need not ask which format the
    /// sheet came from.
    /// </remarks>
    /// <param name="digitWidthTwips">The advance of the default font's widest digit.</param>
    public SheetGrid WithDigitWidth(double digitWidthTwips)
        => ColumnDigits is { } digits
            ? this with { Columns = digits.Resolve(digitWidthTwips) }
            : this;

    /// <summary>Calc's own standard column width: 64 points.</summary>
    /// <remarks><c>STD_COL_WIDTH</c>, <c>sc/inc/global.hxx:107</c>.</remarks>
    public static Length StandardColumnWidth { get; } = Length.FromTwips(1280);

    /// <summary>
    /// The row height a sheet gets when its file states none.
    /// </summary>
    /// <remarks>
    /// 12.8 points, which is what LibreOffice writes as <c>defaultRowHeight</c> for a sheet in
    /// its own default 10-point font, and what a row of that font measures to. Calc derives it
    /// from the font rather than declaring it (<c>STD_ROWHEIGHT_DIFF</c>,
    /// <c>sc/inc/global.hxx:116</c>), so this is a stand-in that every real file overrides.
    /// </remarks>
    public static Length StandardRowHeight { get; } = Length.FromTwips(256);

    /// <summary>A grid with no run at all: every column and row at the standard size.</summary>
    public static SheetGrid Standard { get; } =
        new(new SheetAxis(StandardColumnWidth), new SheetAxis(StandardRowHeight));
}

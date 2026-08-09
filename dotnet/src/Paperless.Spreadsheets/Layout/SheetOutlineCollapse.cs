using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// One stretch of consecutive rows or columns sharing an outline level and collapse flag.
/// </summary>
/// <param name="First">The first index the run covers.</param>
/// <param name="Last">The last index it covers, inclusive.</param>
/// <param name="Level">The outline level, zero for a row or column in no group.</param>
/// <param name="IsCollapsed">
/// True when the run carries SpreadsheetML's <c>collapsed</c> flag, which belongs to the summary
/// row or column *outside* the group rather than to the group itself.
/// </param>
public readonly record struct SheetOutlineRun(int First, int Last, int Level, bool IsCollapsed);

/// <summary>
/// Which rows or columns a SpreadsheetML sheet's collapsed outline groups hide.
/// </summary>
/// <remarks>
/// <para>
/// <strong>SpreadsheetML does not have to say that a collapsed group's rows are hidden.</strong>
/// It states an <c>outlineLevel</c> per row and a <c>collapsed</c> flag on the summary row beside
/// the group, and a reader is expected to derive the rest. Excel usually also writes
/// <c>hidden="1"</c> on every detail row, so the derivation is invisible on most files — and a
/// file whose writer omitted it paginates entirely differently without it. Measured on
/// <c>Application_Compliance_Checklist_5_Apr_2021.xlsx</c>: 329 of one sheet's 1033 rows are
/// hidden by this rule and by nothing else in the part, which is 18 printed pages against 14 and
/// half again as much text on the page.
/// </para>
/// <para>
/// The rule is <c>WorksheetGlobals::convertOutlines</c>
/// (<c>sc/source/filter/oox/worksheethelper.cxx:1307-1334</c>), which keeps a stack of the first
/// index at each open level: a level higher than the stack pushes the current index once per new
/// level, and a lower one pops every level above it, grouping each popped level over
/// <c>[first, current-1]</c>. Only the *innermost* popped group is hidden — <c>bCollapsed</c> is
/// cleared after the first pop — and it is hidden only when the index that closed it carries the
/// flag, which is why the summary row itself stays visible. A gap between two stated rows is
/// processed with the sheet's default model, level zero and no flag
/// (<c>convertRows</c>, <c>:1243-1257</c>), so it closes every open group without hiding
/// anything.
/// </para>
/// <para>
/// BIFF needs none of this and deliberately does not use it. <c>XclImpOutlineBuffer</c> feeds
/// <c>ScOutlineArray::Insert</c> a collapse flag that records the group's state for the user
/// interface (<c>sc/source/filter/excel/exctools.cxx:84-129</c>) and never hides a row; a BIFF
/// <c>ROW</c> record carries <c>fHidden</c> itself, so the rows are already hidden by the time
/// the outline is built.
/// </para>
/// </remarks>
public static class SheetOutlineCollapse
{
    /// <summary>
    /// Appends one row or column to a sheet's outline runs, merged with the previous run when the
    /// two agree — which is what keeps a sheet stating a height on every row from carrying one
    /// outline run per row.
    /// </summary>
    /// <remarks>
    /// LibreOffice merges on the whole row model rather than on the outline alone
    /// (<c>RowModel::isMergeable</c>, <c>sc/source/filter/oox/worksheethelper.cxx:155-165</c>),
    /// which splits runs this does not. The outcome is the same either way: a run's level only
    /// does anything when it differs from the level already on the stack, and its collapse flag
    /// only when the level *falls*, so a split into two runs of equal level makes the second call
    /// a no-op.
    /// </remarks>
    /// <param name="runs">The runs so far, in ascending index order.</param>
    /// <param name="first">The first index the row or column covers.</param>
    /// <param name="last">The last index it covers, inclusive.</param>
    /// <param name="level">Its outline level.</param>
    /// <param name="collapsed">Whether it carries the collapse flag.</param>
    public static void Append(
        List<SheetOutlineRun> runs, int first, int last, int level, bool collapsed)
    {
        ArgumentNullException.ThrowIfNull(runs);

        if (runs.Count > 0 && runs[^1].Last + 1 == first
            && runs[^1].Level == level && runs[^1].IsCollapsed == collapsed)
        {
            runs[^1] = runs[^1] with { Last = last };
            return;
        }

        runs.Add(new SheetOutlineRun(first, last, level, collapsed));
    }

    /// <summary>The index ranges a sheet's collapsed outline groups hide.</summary>
    /// <param name="runs">
    /// The sheet's stated rows or columns in ascending index order. Anything the runs do not
    /// cover is a gap and takes the default model, exactly as the OOXML filter does.
    /// </param>
    /// <returns>Ascending, non-overlapping ranges, empty when nothing is collapsed.</returns>
    public static IReadOnlyList<(int First, int Last)> Hidden(IReadOnlyList<SheetOutlineRun> runs)
    {
        ArgumentNullException.ThrowIfNull(runs);

        List<(int First, int Last)> hidden = [];
        List<int> levels = [];
        int next = 0;

        foreach (SheetOutlineRun run in runs)
        {
            if (run.Last < run.First || run.First < next) continue;

            // The gap between two stated runs takes the default model, which closes every open
            // group and collapses none of them.
            if (next < run.First) Close(levels, next, 0, false, hidden);

            Close(levels, run.First, run.Level, run.IsCollapsed, hidden);
            next = run.Last + 1;
        }

        Close(levels, next, 0, false, hidden);
        return Normalise(hidden);
    }

    /// <summary>
    /// Marks every stated run a collapsed group covers as hidden, adding runs for the indices
    /// inside a group that the sheet states nothing about.
    /// </summary>
    /// <param name="rows">The stated rows, ascending and non-overlapping.</param>
    /// <param name="hidden">The ranges <see cref="Hidden"/> answered.</param>
    /// <param name="defaultSize">The height an index the sheet states nothing about takes.</param>
    public static List<SheetSizeRun> Apply(
        List<SheetSizeRun> rows, IReadOnlyList<(int First, int Last)> hidden, Length defaultSize)
    {
        ArgumentNullException.ThrowIfNull(rows);
        ArgumentNullException.ThrowIfNull(hidden);

        if (hidden.Count == 0) return rows;

        List<SheetSizeRun> merged = new(rows.Count + hidden.Count);
        int at = 0;

        foreach ((int first, int last) in hidden)
        {
            while (at < rows.Count && rows[at].Last < first) merged.Add(rows[at++]);

            int index = first;
            while (index <= last)
            {
                if (at >= rows.Count || rows[at].First > last)
                {
                    // Nothing stated for the rest of the group: one run at the default size.
                    merged.Add(new SheetSizeRun(index, last, defaultSize, true, true));
                    break;
                }

                if (rows[at].First > index)
                {
                    merged.Add(new SheetSizeRun(index, rows[at].First - 1, defaultSize, true, true));
                    index = rows[at].First;
                }

                int stop = Math.Min(rows[at].Last, last);
                merged.Add(rows[at] with { First = index, Last = stop, IsHidden = true });

                // A stated run may reach past the group; the part outside it stays as it was.
                if (rows[at].Last > last) rows[at] = rows[at] with { First = stop + 1 };
                else at++;

                index = stop + 1;
            }
        }

        while (at < rows.Count) merged.Add(rows[at++]);
        return merged;
    }

    /// <summary>
    /// Marks every stated column run a collapsed group covers as hidden. Columns need no filling
    /// in: a column the sheet states nothing about takes the default width and is visible, and a
    /// run for it would say exactly that.
    /// </summary>
    /// <param name="columns">The stated columns, in the order the file states them.</param>
    /// <param name="hidden">The ranges <see cref="Hidden"/> answered.</param>
    /// <param name="defaultWidth">The width an index the sheet states nothing about takes.</param>
    public static List<SheetDigitRun> Apply(
        List<SheetDigitRun> columns,
        IReadOnlyList<(int First, int Last)> hidden,
        SheetDigitWidth defaultWidth)
    {
        ArgumentNullException.ThrowIfNull(columns);
        ArgumentNullException.ThrowIfNull(hidden);

        if (hidden.Count == 0) return columns;

        List<SheetDigitRun> merged = new(columns.Count + hidden.Count);
        foreach ((int first, int last) in hidden)
            merged.Add(new SheetDigitRun(first, last, defaultWidth, true));

        // The stated runs come last so that a stated width still wins, which is the order
        // `SheetColumnDigits` already resolves overlaps in.
        merged.AddRange(columns.Select(run =>
            Covers(hidden, run) ? run with { IsHidden = true } : run));

        return merged;
    }

    private static bool Covers(IReadOnlyList<(int First, int Last)> hidden, SheetDigitRun run)
    {
        foreach ((int first, int last) in hidden)
            if (run.First >= first && run.Last <= last) return true;
        return false;
    }

    private static void Close(
        List<int> levels, int at, int level, bool collapsed, List<(int First, int Last)> hidden)
    {
        if (levels.Count < level)
        {
            while (levels.Count < level) levels.Add(at);
            return;
        }

        while (levels.Count > level)
        {
            int first = levels[^1];
            levels.RemoveAt(levels.Count - 1);

            if (collapsed && first <= at - 1) hidden.Add((first, at - 1));

            // Only the innermost group a single index closes is collapsed by it.
            collapsed = false;
        }
    }

    private static List<(int First, int Last)> Normalise(
        List<(int First, int Last)> ranges)
    {
        if (ranges.Count <= 1) return ranges;

        ranges.Sort((a, b) => a.First != b.First
            ? a.First.CompareTo(b.First)
            : a.Last.CompareTo(b.Last));

        List<(int First, int Last)> merged = [ranges[0]];
        foreach ((int first, int last) in ranges.Skip(1))
        {
            if (first <= merged[^1].Last + 1) merged[^1] = (merged[^1].First, Math.Max(merged[^1].Last, last));
            else merged.Add((first, last));
        }

        return merged;
    }
}

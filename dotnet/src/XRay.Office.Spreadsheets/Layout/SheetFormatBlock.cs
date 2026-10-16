namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// A rectangle of cells that all state one format, kept whole rather than expanded.
/// </summary>
/// <remarks>
/// <para>
/// ODF compresses a sheet with <c>table:number-columns-repeated</c> and
/// <c>table:number-rows-repeated</c>, and Calc's own export pads every sheet out to its full
/// extent that way: a single <c>table:table-row</c> repeated 1048565 times, holding a single
/// <c>table:table-cell</c> repeated 16381 times, is what an ordinary <c>.ods</c> ends with. The
/// product is seventeen billion cells, so the repeat has to be stored as the rectangle it is.
/// </para>
/// <para>
/// That is also how Calc stores it. The importer clamps each repeat to the sheet's own bounds
/// and records the block as one <c>ScRange</c>
/// (<c>sc/source/filter/xml/xmlcelli.cxx</c>:1368-1377), merged into an <c>ScRangeList</c> per
/// style name (<c>ScMyStyleRanges::AddRange</c>) and applied over ranges once the sheet has been
/// read; the store underneath is <c>ScAttrArray</c>, a list of runs.
/// </para>
/// </remarks>
/// <param name="FirstRow">The first row the block covers, inclusive.</param>
/// <param name="LastRow">The last row the block covers, inclusive.</param>
/// <param name="FirstColumn">The first column the block covers, inclusive.</param>
/// <param name="LastColumn">The last column the block covers, inclusive.</param>
/// <param name="Index">The format's index in the sheet's own pool.</param>
internal readonly record struct SheetFormatBlock(
    int FirstRow, int LastRow, int FirstColumn, int LastColumn, int Index);

/// <summary>
/// A sheet's repeated rectangles, grouped by row range so a lookup is a binary search.
/// </summary>
/// <remarks>
/// <para>
/// The grouping is not a refinement, it is what makes the store usable: a real workbook states
/// thousands of these — <c>STC_WebList.ods</c>'s <c>content.xml</c> carries <strong>13 083</strong>
/// <c>table:number-columns-repeated</c> attributes across 11 235 rows — and a linear scan per
/// cell lookup would cost more than expanding them did.
/// </para>
/// <para>
/// A reader walks a sheet's rows in order and each row element contributes one row range, so the
/// ranges arrive sorted and never overlap: every block of one <c>table:table-row</c> covers that
/// element's own rows, and the next element starts after them. They are sorted on sealing anyway,
/// because a producer that does not walk in order would otherwise fail silently.
/// </para>
/// </remarks>
internal sealed class SheetBlockIndex
{
    private readonly List<Band> _bands = [];

    /// <summary>True when nothing has been recorded.</summary>
    public bool IsEmpty => _bands.Count == 0;

    /// <summary>Records one rectangle, clamped to the sheet's own bounds.</summary>
    /// <remarks>
    /// Clamped rather than dropped, which is what <c>ScXMLTableRowContext</c> and
    /// <c>ScXMLTableRowCellContext</c> each do to their own repeat counts
    /// (<c>xmlrowi.cxx</c>:73-82, <c>xmlcelli.cxx</c>:186-195).
    /// </remarks>
    /// <param name="firstRow">The first row, inclusive.</param>
    /// <param name="lastRow">The last row, inclusive.</param>
    /// <param name="firstColumn">The first column, inclusive.</param>
    /// <param name="lastColumn">The last column, inclusive.</param>
    /// <param name="index">The format's index in the sheet's pool.</param>
    public void Add(int firstRow, int lastRow, int firstColumn, int lastColumn, int index)
    {
        if (firstRow < 0 || firstColumn < 0) return;
        if (lastRow < firstRow || lastColumn < firstColumn) return;
        if (firstRow > SheetAddress.MaxRow || firstColumn > SheetAddress.MaxColumn) return;

        lastRow = Math.Min(lastRow, SheetAddress.MaxRow);
        lastColumn = Math.Min(lastColumn, SheetAddress.MaxColumn);

        if (_bands.Count > 0
            && _bands[^1].FirstRow == firstRow
            && _bands[^1].LastRow == lastRow)
        {
            _bands[^1].Runs.Add((firstColumn, lastColumn, index));
            return;
        }

        _bands.Add(new Band(firstRow, lastRow, [(firstColumn, lastColumn, index)]));
    }

    /// <summary>Sorts the bands, so a lookup can binary-search them.</summary>
    public void Seal() => _bands.Sort(static (left, right) => left.FirstRow.CompareTo(right.FirstRow));

    /// <summary>The format index covering one cell, or null when no block does.</summary>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public int? At(int row, int column)
    {
        int at = Find(row);
        if (at < 0) return null;

        // Walked backwards within the band so a later run wins an overlap, which is the order
        // the file states them in.
        List<(int First, int Last, int Index)> runs = _bands[at].Runs;
        for (int run = runs.Count - 1; run >= 0; run--)
        {
            if (column >= runs[run].First && column <= runs[run].Last) return runs[run].Index;
        }

        return null;
    }

    /// <summary>Every block, in the order the bands hold them.</summary>
    public IEnumerable<SheetFormatBlock> Blocks
    {
        get
        {
            foreach (Band band in _bands)
            {
                foreach ((int first, int last, int index) in band.Runs)
                    yield return new SheetFormatBlock(band.FirstRow, band.LastRow, first, last, index);
            }
        }
    }

    /// <summary>
    /// The band whose row range holds a row, or -1.
    /// </summary>
    /// <remarks>
    /// The bands are disjoint, so the candidate is the last one starting at or before the row and
    /// there is no second one to check.
    /// </remarks>
    private int Find(int row)
    {
        int low = 0;
        int high = _bands.Count - 1;
        int found = -1;

        while (low <= high)
        {
            int mid = (low + high) / 2;
            if (_bands[mid].FirstRow <= row) { found = mid; low = mid + 1; }
            else high = mid - 1;
        }

        return found >= 0 && row <= _bands[found].LastRow ? found : -1;
    }

    private sealed record Band(int FirstRow, int LastRow, List<(int First, int Last, int Index)> Runs);
}

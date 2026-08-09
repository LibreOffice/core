namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// A sheet's merged blocks, indexed so a position can be resolved to the block covering it.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="SheetLayout.MergedRanges"/> is a list, and every question decoration asks of it is
/// per cell: a page's backgrounds, its borders and its grid each need to know, for every placed
/// cell, whether a merge covers it and where that merge starts. Walking the list per cell is
/// products of two large numbers — one workbook here states 7818 merges — so the list is bucketed
/// once per sheet instead.
/// </para>
/// <para>
/// Bucketed by a band of rows rather than by row, because a merge spanning a thousand rows would
/// otherwise take a thousand entries while one spanning eight takes eight. A band of 64 keeps the
/// common single-row merge at one entry and bounds a whole-column merge at 16384 rather than a
/// million.
/// </para>
/// </remarks>
internal sealed class SheetMerges
{
    /// <summary>How many rows share one bucket.</summary>
    private const int BandRows = 64;

    private readonly Dictionary<int, List<SheetRange>> _bands;

    private SheetMerges(Dictionary<int, List<SheetRange>> bands) => _bands = bands;

    /// <summary>A sheet with no merged block on it.</summary>
    public static SheetMerges Empty { get; } = new([]);

    /// <summary>True when the sheet states no merge at all.</summary>
    /// <remarks>
    /// Checked before a page walks its cells: an unmerged sheet — which is most of them — then
    /// pays one boolean rather than one dictionary lookup per cell per page.
    /// </remarks>
    public bool IsEmpty => _bands.Count == 0;

    /// <summary>Indexes a sheet's merged blocks.</summary>
    /// <param name="merges">Every merged block on the sheet.</param>
    public static SheetMerges Build(IReadOnlyList<SheetRange> merges)
    {
        if (merges.Count == 0) return Empty;

        Dictionary<int, List<SheetRange>> bands = [];

        foreach (SheetRange merge in merges)
        {
            if (!merge.IsValid || (merge.RowCount == 1 && merge.ColumnCount == 1)) continue;

            for (int band = merge.FirstRow / BandRows; band <= merge.LastRow / BandRows; band++)
            {
                if (!bands.TryGetValue(band, out List<SheetRange>? list))
                    bands[band] = list = [];

                list.Add(merge);
            }
        }

        return bands.Count == 0 ? Empty : new SheetMerges(bands);
    }

    /// <summary>The merged block covering a position, or null when none does.</summary>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public SheetRange? Covering(int row, int column)
    {
        if (_bands.Count == 0) return null;
        if (!_bands.TryGetValue(row / BandRows, out List<SheetRange>? list)) return null;

        foreach (SheetRange merge in list)
        {
            if (row >= merge.FirstRow && row <= merge.LastRow
                && column >= merge.FirstColumn && column <= merge.LastColumn)
            {
                return merge;
            }
        }

        return null;
    }

    /// <summary>
    /// Where a position's decoration comes from: the origin of the block covering it, or the
    /// position itself.
    /// </summary>
    /// <remarks>
    /// <c>ArrayImpl::GetMergedStyleSourceCell</c>, which walks back over the overlap flags to the
    /// block's top-left cell and reads every border style from there
    /// (<c>svx/source/dialog/framelinkarray.cxx:460-467</c>). Measured on
    /// <c>probes/sheets-r37/merge-decor.fods</c>: a covered cell's own fill and border are drawn
    /// nowhere at all.
    /// </remarks>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public (int Row, int Column) OriginOf(int row, int column)
        => Covering(row, column) is { } merge ? (merge.FirstRow, merge.FirstColumn) : (row, column);

    /// <summary>True when a position's top edge is inside a merged block rather than on it.</summary>
    public bool IsOverlappedTop(int row, int column)
        => Covering(row, column) is { } merge && row > merge.FirstRow;

    /// <summary>True when a position's left edge is inside a merged block rather than on it.</summary>
    public bool IsOverlappedLeft(int row, int column)
        => Covering(row, column) is { } merge && column > merge.FirstColumn;

    /// <summary>True when a position's bottom edge is inside a merged block rather than on it.</summary>
    public bool IsOverlappedBottom(int row, int column)
        => Covering(row, column) is { } merge && row < merge.LastRow;

    /// <summary>True when a position's right edge is inside a merged block rather than on it.</summary>
    public bool IsOverlappedRight(int row, int column)
        => Covering(row, column) is { } merge && column < merge.LastColumn;
}

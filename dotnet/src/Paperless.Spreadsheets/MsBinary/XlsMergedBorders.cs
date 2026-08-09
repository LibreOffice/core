using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.MsBinary;

/// <summary>
/// Moves a merged range's outer right and bottom borders onto the cell that states them.
/// </summary>
/// <remarks>
/// <para>
/// A BIFF writer states a border on <em>every</em> cell of a merged range, so the origin cell's
/// own right border is the range's first <em>interior</em> line and its own bottom border is the
/// line under its first row. Once the range is drawn from its origin — which is what Calc does,
/// see <see cref="SheetMerges"/> — those two edges would be the wrong ones.
/// </para>
/// <para>
/// So the import moves them: for a range spanning columns it copies the right border of
/// <c>(lastColumn, firstRow)</c> onto the origin, and for one spanning rows the bottom border of
/// <c>(firstColumn, lastRow)</c>, <em>before</em> merging
/// (<c>XclImpXFRangeBuffer::SetBorderLine</c> and its two callers,
/// <c>sc/source/filter/excel/xistyle.cxx:1976-1990</c> and <c>:2077-2090</c>). Left and top need
/// no such move because the origin already sits on them.
/// </para>
/// <para>
/// Deliberately in the BIFF reader and nowhere else. SpreadsheetML and ODF are read by importers
/// that do no equivalent — <c>ScXMLTableRowCellContext</c> and the OOXML
/// <c>SheetDataBuffer</c> apply the merge without touching a border — so applying it to all three
/// would be reproducing an Excel-specific repair on formats that do not need it.
/// </para>
/// </remarks>
internal static class XlsMergedBorders
{
    /// <summary>Applies the transfer to every merged range on one sheet.</summary>
    /// <remarks>
    /// In the order the ranges were read and reading back what earlier ranges wrote, because the
    /// import's loop does the same thing with <c>ApplyAttr</c>: two ranges sharing a cell see each
    /// other.
    /// </remarks>
    /// <param name="formatting">The sheet's resolved formatting, modified in place.</param>
    /// <param name="merges">The <c>MERGEDCELLS</c> ranges, in the order the file stated them.</param>
    public static void Apply(SheetFormatting formatting, IReadOnlyList<SheetRange> merges)
    {
        ArgumentNullException.ThrowIfNull(formatting);
        ArgumentNullException.ThrowIfNull(merges);

        if (formatting.IsEmpty || merges.Count == 0) return;

        foreach (SheetRange merge in merges)
        {
            if (!merge.IsValid) continue;

            SheetCellDecoration origin = formatting.At(merge.FirstRow, merge.FirstColumn);
            SheetCellBorders borders = origin.Borders;

            if (merge.LastColumn > merge.FirstColumn)
            {
                borders = borders with
                {
                    Right = formatting.At(merge.FirstRow, merge.LastColumn).Borders.Right,
                };
            }

            if (merge.LastRow > merge.FirstRow)
            {
                borders = borders with
                {
                    Bottom = formatting.At(merge.LastRow, merge.FirstColumn).Borders.Bottom,
                };
            }

            if (borders == origin.Borders) continue;

            SheetCellDecoration moved = origin with { Borders = borders };
            formatting.SetCell(
                merge.FirstRow, merge.FirstColumn, formatting.Intern(moved));
        }
    }
}

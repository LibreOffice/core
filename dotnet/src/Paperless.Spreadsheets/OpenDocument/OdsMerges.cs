using System.Xml.Linq;
using Paperless.OpenDocument;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.OpenDocument;

/// <summary>
/// The merged blocks a <c>table:table</c> states, as ranges.
/// </summary>
/// <remarks>
/// ODF states a merge on the block's own top-left cell —
/// <c>table:number-columns-spanned</c> and <c>table:number-rows-spanned</c> — and writes the rest
/// of the block as <c>table:covered-table-cell</c>. The extraction reader turns that into a span on
/// one <see cref="Paperless.Core.Extraction.ContentTableCell"/> and drops the covered ones, which
/// recovers every merge whose anchor survives; an anchor holding nothing, past the last filled cell
/// in its row, is trailing padding that the row drops before it reaches the tree. So the ranges are
/// re-read from the markup for <see cref="SheetLayout.StatedMerges"/>, where the difference matters:
/// <c>ScOutputData::IsAvailable</c> (<c>sc/source/ui/view/output2.cxx:1178-1191</c>) stops a
/// neighbour's long string at a merged or overlapped cell whether or not it holds anything.
/// </remarks>
internal static class OdsMerges
{
    /// <summary>How far a repeat count is honoured, matching the cell-format reader's own cap.</summary>
    /// <remarks>
    /// A row or cell repeated a million times is the sheet's padding rather than a million merges,
    /// and a merge inside such a repeat would be the same block stamped across the grid. Counting
    /// the repeat towards the index and reading only the first is what every other ODS walk does.
    /// </remarks>
    private const int MaxRepeat = 4096;

    /// <summary>Reads a sheet's merged ranges.</summary>
    /// <param name="table">The <c>table:table</c> element.</param>
    public static IReadOnlyList<SheetRange> Read(XElement table)
    {
        ArgumentNullException.ThrowIfNull(table);

        List<SheetRange> ranges = [];
        int row = 0;

        foreach (XElement rowElement in Rows(table))
        {
            int repeat = Repeat(rowElement, "number-rows-repeated");
            int first = Math.Min(repeat, MaxRepeat);
            if (first > 0 && row <= SheetAddress.MaxRow) ReadCells(rowElement, row, ranges);
            row += repeat;
            if (row > SheetAddress.MaxRow) break;
        }

        return ranges;
    }

    /// <summary>
    /// The cells whose content is a hyperlink, as one-cell ranges.
    /// </summary>
    /// <remarks>
    /// ODF writes a cell's hyperlink as a <c>text:a</c> around the text, which is how Calc's own
    /// <c>SvxURLField</c> round-trips — so a cell holding one is an edit cell holding a field, and
    /// a field is never broken across lines. See <see cref="SheetLayout.HyperlinkRanges"/>.
    /// </remarks>
    /// <param name="table">The <c>table:table</c> element.</param>
    public static IReadOnlyList<SheetRange> ReadHyperlinks(XElement table)
    {
        ArgumentNullException.ThrowIfNull(table);

        List<SheetRange> ranges = [];
        int row = 0;

        foreach (XElement rowElement in Rows(table))
        {
            int repeat = Repeat(rowElement, "number-rows-repeated");
            if (Math.Min(repeat, MaxRepeat) > 0 && row <= SheetAddress.MaxRow)
            {
                int column = 0;
                foreach (XElement cell in rowElement.Elements())
                {
                    if (cell.Name.NamespaceName != OdfNamespaces.Table) continue;
                    if (cell.Name.LocalName is not ("table-cell" or "covered-table-cell")) continue;

                    if (cell.Name.LocalName == "table-cell"
                        && column <= SheetAddress.MaxColumn
                        && cell.Descendants(XName.Get("a", OdfNamespaces.Text)).Any())
                    {
                        ranges.Add(new SheetRange(column, row, column, row));
                    }

                    column += Repeat(cell, "number-columns-repeated");
                    if (column > SheetAddress.MaxColumn) break;
                }
            }

            row += repeat;
            if (row > SheetAddress.MaxRow) break;
        }

        return ranges;
    }

    private static void ReadCells(XElement rowElement, int row, List<SheetRange> ranges)
    {
        int column = 0;

        foreach (XElement cell in rowElement.Elements())
        {
            if (cell.Name.NamespaceName != OdfNamespaces.Table) continue;
            if (cell.Name.LocalName is not ("table-cell" or "covered-table-cell")) continue;

            int repeat = Repeat(cell, "number-columns-repeated");

            if (cell.Name.LocalName == "table-cell")
            {
                int columns = Repeat(cell, "number-columns-spanned");
                int rows = Repeat(cell, "number-rows-spanned");

                if ((columns > 1 || rows > 1)
                    && column <= SheetAddress.MaxColumn
                    && (long)columns * rows <= 1_000_000)
                {
                    ranges.Add(new SheetRange(
                        column, row, column + columns - 1, row + rows - 1));
                }
            }

            column += repeat;
            if (column > SheetAddress.MaxColumn) break;
        }
    }

    /// <summary>The rows of a table, including those inside a row group.</summary>
    private static IEnumerable<XElement> Rows(XElement table)
        => table.Descendants(XName.Get("table-row", OdfNamespaces.Table));

    private static int Repeat(XElement element, string name)
    {
        string? stated = element.Attribute(XName.Get(name, OdfNamespaces.Table))?.Value;
        return OdfValue.ParseInt(stated) is { } value && value >= 1 ? value : 1;
    }
}

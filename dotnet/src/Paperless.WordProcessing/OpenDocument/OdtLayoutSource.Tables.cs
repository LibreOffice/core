using System.Xml.Linq;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.OpenDocument;

/// <content>
/// Reading a <c>table:table</c> into the grid the layout engine takes.
/// </content>
/// <remarks>
/// <para>
/// ODF describes a table by declaring its columns and then filling rows against them, which is very nearly
/// what layout wants — the grid is stated outright rather than implied by the widest row, so nothing has to
/// be inferred. Three details do the work:
/// </para>
/// <list type="bullet">
///   <item>
///     <c>table:number-columns-repeated</c> on a column, and <c>table:number-rows-repeated</c> on a row.
///     A table of twelve identical columns declares one and repeats it, so a reader that took each element
///     as one column would build a grid a twelfth of the right width.
///   </item>
///   <item>
///     <c>table:covered-table-cell</c>, which is a placeholder for a column swallowed by a merge to its
///     left or above. It holds no content and must not be laid out, but it <em>does</em> occupy its column
///     — so skipping it entirely would shift every cell after it one column left.
///   </item>
///   <item>
///     <c>table:table-header-rows</c>, whose rows are the ones that repeat when the table crosses a page.
///     It is a wrapper element rather than a flag, so the rows inside it are still ordinary rows and their
///     position in the table is what makes them headings.
///   </item>
/// </list>
/// </remarks>
public sealed partial class OdtLayoutSource
{
    /// <summary>
    /// Writer's own default cell padding, for a table whose cell style states none.
    /// </summary>
    /// <remarks>
    /// 0.097 cm, which is 55 twips — an odd number that comes from the 1/100 mm grid the draw layer uses
    /// rather than from anything a person chose. Both edges of it matter: it comes out of the cell's width,
    /// so a reader defaulting it to zero breaks a narrow cell's text a line later than Writer does.
    /// </remarks>
    private static readonly CellPadding DefaultCellPadding = CellPadding.Writer;

    /// <summary>Reads a table, or returns null when it declares no usable grid.</summary>
    private PageTable? Table(XElement element)
    {
        List<Length> columns = Columns(element);
        if (columns.Count == 0) return null;

        List<PageTableRow> rows = [];
        int headerRows = 0;
        ReadRows(element, rows, ref headerRows, isHeader: false, depth: 0);

        if (rows.Count == 0) return null;

        string? styleName = element.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        return new PageTable
        {
            ColumnWidths = columns,
            Rows = rows,
            HeaderRowCount = headerRows,
            LeftIndent = TableMeasure(styleName, OdfNamespaces.FoCompatible, "margin-left")
                ?? Length.Zero,
            SpaceBefore = TableMeasure(styleName, OdfNamespaces.FoCompatible, "margin-top")
                ?? Length.Zero,
            SpaceAfter = TableMeasure(styleName, OdfNamespaces.FoCompatible, "margin-bottom")
                ?? Length.Zero,
        };
    }

    /// <summary>
    /// The grid's column widths, in order.
    /// </summary>
    /// <remarks>
    /// A column whose style states no width gets nothing here rather than a guess, because a guess would be
    /// worse than the truth: LibreOffice's own writer always states them, and a document that does not is
    /// asking for the table to be fitted to the page — which needs the page, and is recorded as a gap
    /// rather than approximated.
    /// </remarks>
    private List<Length> Columns(XElement table)
    {
        List<Length> widths = [];
        Collect(table, 0);
        return widths;

        void Collect(XElement element, int depth)
        {
            if (depth > 8 || widths.Count >= PageTable.MaxColumns) return;

            foreach (XElement child in element.Elements())
            {
                if (child.Name.Namespace != OdfNamespaces.Table) continue;

                switch (child.Name.LocalName)
                {
                    // The grouping elements are transparent: a table can wrap its columns in
                    // table:table-columns, or group them for outlining, and the columns inside are the
                    // table's own either way.
                    case "table-columns" or "table-header-columns" or "table-column-group":
                        Collect(child, depth + 1);
                        break;

                    case "table-column":
                        Length width = ColumnWidth(child);
                        int repeat = Repeat(child, "number-columns-repeated");

                        for (int i = 0; i < repeat && widths.Count < PageTable.MaxColumns; i++)
                        {
                            widths.Add(width);
                        }

                        break;
                }
            }
        }
    }

    private Length ColumnWidth(XElement column)
    {
        string? styleName = column.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        return OdfWriterUnits.ToCore(
            OdfValue.ParseLength(
                _styles.ResolveProperty(
                    styleName, OdfStyleFamily.TableColumn, OdfPropertyKind.TableColumn,
                    OdfNamespaces.Style, "column-width").Value))
            ?? Length.Zero;
    }

    /// <summary>
    /// Reads a table's rows, following the grouping elements and noting which rows are headings.
    /// </summary>
    /// <remarks>
    /// The heading count is a run from the top, matching <c>SwTable::GetRowsToRepeat</c>, so it is only
    /// advanced while every row so far has been a heading. A <c>table:table-header-rows</c> appearing after
    /// ordinary rows — which is legal and meaningless — therefore repeats nothing, rather than repeating
    /// rows from the middle of the table.
    /// </remarks>
    private void ReadRows(
        XElement element, List<PageTableRow> rows, ref int headerRows, bool isHeader, int depth)
    {
        if (depth > 8) return;

        foreach (XElement child in element.Elements())
        {
            if (child.Name.Namespace != OdfNamespaces.Table) continue;
            if (rows.Count >= PageTable.MaxRows) return;

            switch (child.Name.LocalName)
            {
                case "table-header-rows":
                    ReadRows(child, rows, ref headerRows, isHeader: true, depth + 1);
                    break;

                case "table-rows" or "table-row-group":
                    ReadRows(child, rows, ref headerRows, isHeader, depth + 1);
                    break;

                case "table-row":
                    PageTableRow row = Row(child, isHeader);
                    int repeat = Repeat(child, "number-rows-repeated");

                    for (int i = 0; i < repeat && rows.Count < PageTable.MaxRows; i++)
                    {
                        rows.Add(row);
                        if (isHeader && headerRows == rows.Count - 1) headerRows = rows.Count;
                    }

                    break;
            }
        }
    }

    private PageTableRow Row(XElement element, bool isHeader)
    {
        List<PageTableCell> cells = [];
        int column = 0;

        foreach (XElement child in element.Elements())
        {
            if (child.Name.Namespace != OdfNamespaces.Table) continue;

            // A covered cell is a column the merge to its left or above swallowed. It carries no content
            // and gets no cell, but it advances the column counter — which is the whole reason ODF writes
            // it at all, since a row's cells are positional.
            if (child.Name.LocalName == "covered-table-cell")
            {
                column += Repeat(child, "number-columns-repeated");
                continue;
            }

            if (child.Name.LocalName != "table-cell") continue;

            int span = Math.Max(1, Attribute(child, "number-columns-spanned"));
            int rowSpan = Math.Max(1, Attribute(child, "number-rows-spanned"));
            int repeat = Repeat(child, "number-columns-repeated");

            string? styleName = child.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;
            List<PageParagraph> paragraphs = ReadFlow(child);

            for (int i = 0; i < repeat && column < PageTable.MaxColumns; i++)
            {
                cells.Add(new PageTableCell
                {
                    Paragraphs = paragraphs,
                    Column = column,
                    ColumnSpan = span,
                    RowSpan = rowSpan,
                    Padding = Padding(styleName),
                    VerticalAlignment = VerticalAlignment(styleName),
                });

                // One column per element, not one per column spanned. A cell covering two columns is
                // followed by a table:covered-table-cell for the second, and it is that placeholder which
                // accounts for the column — so advancing by the span here as well would count it twice and
                // push every cell after it off the end of the grid. ODF requires the placeholders and
                // LibreOffice always writes them, which is what makes this the safe direction to err in:
                // a producer that omits one leaves a gap rather than an overlap.
                column++;
            }
        }

        return new PageTableRow
        {
            Cells = cells,
            IsHeader = isHeader,
            MinHeight = RowHeight(element) ?? Length.Zero,
        };
    }

    /// <summary>
    /// A row's declared height, which is a floor rather than a size.
    /// </summary>
    /// <remarks>
    /// <c>style:min-row-height</c> says so outright. <c>style:row-height</c> reads as exact but is not:
    /// LibreOffice honours it only while the content fits, and grows the row otherwise — so both map to the
    /// same floor here, which is what a row whose text has been edited since it was written actually does.
    /// </remarks>
    private Length? RowHeight(XElement row)
    {
        string? styleName = row.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        return RowMeasure(styleName, "min-row-height") ?? RowMeasure(styleName, "row-height");
    }

    private Length? RowMeasure(string? styleName, string propertyName)
        => OdfWriterUnits.ToCore(
            OdfValue.ParseLength(
                _styles.ResolveProperty(
                    styleName, OdfStyleFamily.TableRow, OdfPropertyKind.TableRow,
                    OdfNamespaces.Style, propertyName).Value));

    /// <summary>
    /// A cell's padding, from the one-value form or the four separate ones.
    /// </summary>
    /// <remarks>
    /// <c>fo:padding</c> sets all four and each <c>fo:padding-left</c> and friends overrides its own side,
    /// which is CSS's rule and ODF's. The per-side value wins wherever it is present, so a style stating
    /// both is read the way a browser would read it.
    /// </remarks>
    private CellPadding Padding(string? styleName)
    {
        Length? all = CellMeasure(styleName, "padding");

        return new CellPadding(
            CellMeasure(styleName, "padding-left") ?? all ?? DefaultCellPadding.Left,
            CellMeasure(styleName, "padding-right") ?? all ?? DefaultCellPadding.Right,
            CellMeasure(styleName, "padding-top") ?? all ?? DefaultCellPadding.Top,
            CellMeasure(styleName, "padding-bottom") ?? all ?? DefaultCellPadding.Bottom);
    }

    private Length? CellMeasure(string? styleName, string propertyName)
        => OdfWriterUnits.ToCore(
            OdfValue.ParseLength(
                _styles.ResolveProperty(
                    styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell,
                    OdfNamespaces.FoCompatible, propertyName).Value));

    /// <summary>Where a cell's text sits when its row is taller than its content.</summary>
    /// <remarks>
    /// <c>automatic</c> is a real value and means the top, which is also what an unstated alignment means —
    /// so both fall through to the same answer rather than one of them being treated as unknown.
    /// </remarks>
    private CellVerticalAlignment VerticalAlignment(string? styleName)
        => _styles.ResolveProperty(
            styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell,
            OdfNamespaces.Style, "vertical-align").Value switch
        {
            "middle" => CellVerticalAlignment.Middle,
            "bottom" => CellVerticalAlignment.Bottom,
            _ => CellVerticalAlignment.Top,
        };

    private Length? TableMeasure(string? styleName, string propertyNamespace, string propertyName)
        => OdfWriterUnits.ToCore(
            OdfValue.ParseLength(
                _styles.ResolveProperty(
                    styleName, OdfStyleFamily.Table, OdfPropertyKind.Table,
                    propertyNamespace, propertyName).Value));

    /// <summary>
    /// A repeat count, clamped so that one bad attribute cannot allocate a grid.
    /// </summary>
    /// <remarks>
    /// Real documents write large repeats — a spreadsheet-shaped ODF table repeats its last column to
    /// 16384 to say "and the rest is empty" — so the value is honoured up to the grid limits and clamped
    /// rather than rejected. Absent or unparseable means one, which is what a plain column is.
    /// </remarks>
    private static int Repeat(XElement element, string attributeName)
    {
        int stated = Attribute(element, attributeName);
        return stated <= 0 ? 1 : Math.Min(stated, PageTable.MaxColumns);
    }

    private static int Attribute(XElement element, string attributeName)
        => int.TryParse(
            element.Attribute(XName.Get(attributeName, OdfNamespaces.Table))?.Value,
            System.Globalization.NumberStyles.Integer,
            System.Globalization.CultureInfo.InvariantCulture,
            out int value)
            ? value
            : 0;
}

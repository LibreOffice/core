using System.Xml.Linq;
using Paperless.Core.Graphics;
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
        List<Length?> declared = Columns(element);
        if (declared.Count == 0) return null;

        List<PageTableRow> rows = [];
        int headerRows = 0;
        ReadRows(element, rows, ref headerRows, isHeader: false, depth: 0);

        if (rows.Count == 0) return null;

        string? styleName = element.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        List<Length> columns = [.. declared.Select(width => width ?? Length.Zero)];

        return new PageTable
        {
            SectionIndex = _sectionIndex,
            ColumnWidths = columns,
            ColumnFit = Fit(declared, styleName),
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
    /// The grid's column widths, in order, with null for a column whose style states none.
    /// </summary>
    /// <remarks>
    /// Null rather than zero, because the two are different documents: zero is a column the file asked to
    /// be invisible and null is one it left to Writer, which then sizes it by
    /// <see cref="TableColumnFit"/>'s arithmetic. Reading the second as the first is what made a width-less
    /// table lay out with no columns at all.
    /// </remarks>
    private List<Length?> Columns(XElement table)
    {
        List<Length?> widths = [];
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
                        Length? width = ColumnWidth(child);
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

    private Length? ColumnWidth(XElement column)
    {
        string? styleName = column.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        return OdfWriterUnits.ToCore(
            OdfValue.ParseLength(
                _styles.ResolveProperty(
                    styleName, OdfStyleFamily.TableColumn, OdfPropertyKind.TableColumn,
                    OdfNamespaces.Style, "column-width").Value));
    }

    /// <summary>
    /// How the columns the file left blank are to be sized, or null when it stated every one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The half of <c>SwXMLTableContext::MakeTable</c> (<c>sw/source/filter/xml/xmltbli.cxx</c>:2467) that
    /// decides <em>which</em> distribution runs, which turns on the table's horizontal orientation rather
    /// than on its width. <c>table:align</c> maps onto <c>HoriOrientation</c> through
    /// <c>aXMLTableAlignMap</c> (<c>sw/source/filter/xml/xmlithlp.cxx</c>:307): left, centre and right are
    /// real orientations, <c>margins</c> is <c>FULL</c>, and an absent attribute is <c>FULL</c> too.
    /// </para>
    /// <para>
    /// <b>Under <c>FULL</c> the table's stated width is discarded</b> — the importer's own comment is "Even
    /// if a size is specified, it will be ignored!" — and the table is as wide as the area it sits in. So
    /// the same three width-less columns come out equal in a table with no <c>table:align</c> and in the
    /// ratio 3:2:4 in one that says <c>left</c>, on the same page, from the same widths. Measured both ways.
    /// </para>
    /// </remarks>
    /// <param name="declared">The columns, with null for each that stated no width.</param>
    /// <param name="styleName">The table's own style name.</param>
    private TableColumnFit? Fit(List<Length?> declared, string? styleName)
    {
        if (declared.All(width => width is not null)) return null;

        string? align = _styles.ResolveProperty(
            styleName, OdfStyleFamily.Table, OdfPropertyKind.Table,
            OdfNamespaces.Table, "align").Value;

        bool oriented = align is "left" or "center" or "right";
        Length? width = oriented ? TableMeasure(styleName, OdfNamespaces.Style, "width") : null;

        return new TableColumnFit
        {
            IsAuto = [.. declared.Select(column => column is null)],
            TableWidth = width,
            Rule = TableWidthRule.OpenDocument,
        };
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
            List<PageBlock> blocks = ReadCell(child);

            for (int i = 0; i < repeat && column < PageTable.MaxColumns; i++)
            {
                cells.Add(new PageTableCell
                {
                    Blocks = blocks,
                    Column = column,
                    ColumnSpan = span,
                    RowSpan = rowSpan,
                    Padding = Padding(styleName),
                    VerticalAlignment = VerticalAlignment(styleName),
                    Shading = Shading(styleName),
                    Borders = Borders(styleName),
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
            MinHeight = RowHeight(element).Height,
            HasExactHeight = RowHeight(element).IsExact,
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
    /// <summary>
    /// The row's declared height and whether it is exact.
    /// </summary>
    /// <remarks>
    /// ODF distinguishes the two by attribute name rather than by a rule: <c>style:min-row-height</c> is a
    /// floor and <c>style:row-height</c> is a height, and a row stating both means the floor — a minimum the
    /// content can exceed is a weaker claim than an exact size, so honouring the exact one would clip content
    /// the document said could grow.
    /// </remarks>
    /// <param name="row">The <c>table:table-row</c> element.</param>
    private (Length Height, bool IsExact) RowHeight(XElement row)
    {
        string? styleName = row.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        if (RowMeasure(styleName, "min-row-height") is { } floor) return (floor, false);

        return RowMeasure(styleName, "row-height") is { } exact
            ? (exact, true)
            : (Length.Zero, false);
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

    /// <summary>
    /// The colour behind a cell's text, or null when it has none.
    /// </summary>
    /// <remarks>
    /// <c>fo:background-color</c>, whose <c>transparent</c> is a real value meaning "no shading" rather than a
    /// colour — so it has to fall through to null rather than being parsed. ODF has no separate pattern or
    /// foreground colour for a cell the way RTF and WW8 do; the resolved colour is the whole answer.
    /// </remarks>
    private Colour? Shading(string? styleName)
    {
        OdfProperty stated = _styles.ResolveProperty(
            styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell,
            OdfNamespaces.FoCompatible, "background-color");

        return stated.Value == "transparent" ? null : stated.AsColour();
    }

    /// <summary>
    /// A cell's four borders.
    /// </summary>
    /// <remarks>
    /// <c>fo:border</c> sets all four and each <c>fo:border-left</c> and friends overrides its own side, which
    /// is CSS's rule and ODF's — the same cascade <see cref="Padding"/> follows for the same reason.
    /// </remarks>
    private CellBorders Borders(string? styleName)
    {
        TableBorder all = Border(styleName, "border");

        return new CellBorders(
            Border(styleName, "border-left", all),
            Border(styleName, "border-right", all),
            Border(styleName, "border-top", all),
            Border(styleName, "border-bottom", all));
    }

    /// <summary>
    /// One border from an ODF shorthand, or a fallback when the property says nothing.
    /// </summary>
    /// <remarks>
    /// The value is CSS's three-part shorthand — <c>0.5pt solid #ff0000</c> — in any order, and <c>none</c> is a
    /// value in its own right meaning there is no border rather than that nothing was said. So <c>none</c> has
    /// to beat the fallback: a style setting <c>fo:border</c> and then <c>fo:border-top="none"</c> means three
    /// borders, not four.
    /// </remarks>
    private TableBorder Border(string? styleName, string propertyName, TableBorder fallback = default)
    {
        string? stated = _styles.ResolveProperty(
            styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell,
            OdfNamespaces.FoCompatible, propertyName).Value;

        if (string.IsNullOrWhiteSpace(stated)) return fallback;
        if (stated.Trim() == "none") return default;

        Length width = Length.Zero;
        Colour colour = Colour.Black;

        foreach (string part in stated.Split(' ', StringSplitOptions.RemoveEmptyEntries))
        {
            if (OdfValue.ParseLength(part) is { } measured)
            {
                width = OdfWriterUnits.ToCore(measured);
                continue;
            }

            if (part.StartsWith('#') && OdfValue.ParseColour(part) is { } stated_colour)
            {
                colour = stated_colour;
            }
        }

        // A shorthand naming a style and a colour but no width still means a border: LibreOffice draws such a
        // one hairline, which is its thinnest visible stroke rather than nothing.
        if (width <= Length.Zero) width = HairlineBorder;

        return new TableBorder(width, colour);
    }

    /// <summary>The width a border with no stated one is drawn at: half a point, Writer's hairline.</summary>
    private static readonly Length HairlineBorder = Length.FromPoints(0.5);

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

using System.Xml.Linq;
using Paperless.Core.Extraction;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Reads a DrawingML table — <c>a:tbl</c> — into a <see cref="ContentTable"/>.
/// </summary>
/// <remarks>
/// <para>
/// A DrawingML table is a grid of text bodies and nothing else: every cell's content is an
/// <c>a:txBody</c> read exactly as a shape's is. So this is a thin walk over
/// <see cref="DrawingTextBody"/> rather than a second text reader, which is what keeps a
/// bullet inside a table cell rendering the way the same bullet does outside one.
/// </para>
/// <para>
/// It lives here rather than beside the presentation reader because the markup is DrawingML,
/// not PresentationML — the <c>p:graphicFrame</c> that carries it on a slide is the only
/// family-specific part, and that stays with the family.
/// </para>
/// </remarks>
public static class DrawingTable
{
    /// <summary>The <c>a:graphicData</c> URI that identifies a table.</summary>
    public const string TableUri = "http://schemas.openxmlformats.org/drawingml/2006/table";

    /// <summary>
    /// Reads a table.
    /// </summary>
    /// <param name="table">The <c>a:tbl</c> element.</param>
    /// <param name="options">Options passed to each cell's text body.</param>
    public static ContentTable Read(XElement table, DrawingTextOptions? options = null)
    {
        ArgumentNullException.ThrowIfNull(table);

        // The grid is authoritative for the column count: a row may hold fewer cells than the
        // table has columns when its trailing cells are merged away, so counting a row's cells
        // would under-report a table whose last row spans.
        int columns = Drawing.Children(Drawing.Child(table, "tblGrid"), "gridCol").Count();

        List<ContentTableRow> rows = [];
        int widest = 0;

        int rowIndex = 0;
        foreach (XElement row in Drawing.Children(table, "tr"))
        {
            ContentTableRow contentRow = new() { Index = rowIndex };

            int column = 0;
            foreach (XElement cell in Drawing.Children(row, "tc"))
            {
                int columnSpan = Math.Max(1, Drawing.Number(cell, "gridSpan") ?? 1);
                int rowSpan = Math.Max(1, Drawing.Number(cell, "rowSpan") ?? 1);

                // hMerge and vMerge mark the cells a span covers. They are present in the file
                // — the grid stays rectangular — but they hold no content and reporting them
                // would put an empty cell after every merged one.
                bool covered = Drawing.Flag(cell, "hMerge") == true
                               || Drawing.Flag(cell, "vMerge") == true;
                if (covered) { column++; continue; }

                ContentTableCell contentCell = new()
                {
                    Row = rowIndex,
                    Column = column,
                    ColumnSpan = columnSpan,
                    RowSpan = rowSpan,
                };

                if (Drawing.Child(cell, "txBody") is { } body)
                    DrawingTextBody.Read(body, contentCell, options);

                contentRow.Children.Add(contentCell);
                column += columnSpan;
            }

            rows.Add(contentRow);
            widest = Math.Max(widest, column);
            rowIndex++;
        }

        ContentTable result = new()
        {
            // A producer that omits a:tblGrid still has a table, so the widest row stands in
            // rather than reporting no columns at all.
            ColumnCount = columns > 0 ? columns : widest,
            // ST_Boolean firstRow marks the header row band. There is only ever one.
            HeaderRowCount = Drawing.Flag(Drawing.Child(table, "tblPr"), "firstRow") == true ? 1 : 0,
        };
        foreach (ContentTableRow row in rows) result.Children.Add(row);
        return result;
    }
}

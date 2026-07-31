using System.Xml.Linq;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// Which cell format each cell of one sheet uses.
/// </summary>
/// <remarks>
/// <para>
/// A second walk over the worksheet element, deliberately, and it is cheap: the part is already
/// parsed into an element tree, so this reads two attributes off elements that are in memory
/// rather than re-parsing anything. Threading it through <see cref="XlsxSheetReader"/> instead
/// would put a rendering-only concern in the middle of the extraction path, which every caller
/// pays for and almost none wants — and the module's own rule is that extraction must not pay for
/// fonts.
/// </para>
/// <para>
/// The same fallback order the format lookup states: a cell's own <c>s</c>, then its row's, then
/// its column's, then <c>cellXfs[0]</c>. A row's is only its default when <c>customFormat</c> says
/// so; without the flag the <c>s</c> on a <c>&lt;row&gt;</c> is Excel's record of what the row
/// happens to hold and applies to nothing.
/// </para>
/// </remarks>
internal static class XlsxSheetFormats
{
    /// <summary>Reads one sheet's per-cell format indices.</summary>
    /// <param name="worksheet">The <c>worksheet</c> root, or null when the part is missing.</param>
    /// <param name="formats">The workbook's cell formats, indexed as <c>cellXfs</c> orders them.</param>
    public static SheetCellFormats Read(XElement? worksheet, IReadOnlyList<SheetCellFormat> formats)
    {
        ArgumentNullException.ThrowIfNull(formats);
        if (worksheet is null || formats.Count == 0) return SheetCellFormats.Empty;

        SheetCellFormats.Builder builder = new();
        int[] pooled = new int[formats.Count];
        for (int at = 0; at < formats.Count; at++) pooled[at] = builder.Intern(formats[at]);

        builder.SetSheetDefault(pooled[0]);

        foreach (XElement column in Xlsx.Children(Xlsx.Child(worksheet, "cols"), "col"))
        {
            if (Index(column, "style") is not { } style) continue;

            int first = (Xlsx.Integer(column, "min") - 1) ?? 0;
            int last = (Xlsx.Integer(column, "max") - 1) ?? first;

            // A <col> may legitimately name the sheet's last column, and a run that wide is a
            // statement about the sheet's default rather than about sixteen thousand columns.
            if (last >= XlsxSheetReader.MaxColumns - 1)
            {
                builder.SetSheetDefault(pooled[style]);
                last = first;
            }

            for (int at = Math.Max(first, 0); at <= last && at < XlsxSheetReader.MaxColumns; at++)
            {
                builder.SetColumn(at, pooled[style]);
            }
        }

        int expectedRow = 0;
        foreach (XElement row in Xlsx.Children(Xlsx.Child(worksheet, "sheetData"), "row"))
        {
            int rowIndex = (Xlsx.Integer(row, "r") - 1) ?? expectedRow;
            if (rowIndex < 0) rowIndex = expectedRow;
            expectedRow = rowIndex + 1;

            if (Xlsx.Flag(row, "customFormat") && Index(row, "s") is { } rowStyle)
            {
                builder.SetRow(rowIndex, pooled[rowStyle]);
            }

            int expectedColumn = 0;
            foreach (XElement cell in Xlsx.Children(row, "c"))
            {
                int column = expectedColumn;
                if (Xlsx.Attribute(cell, "r") is { } reference
                    && Xlsx.TryParseCellReference(reference, out int parsed, out _))
                {
                    column = parsed;
                }
                if (column < 0) column = expectedColumn;
                expectedColumn = column + 1;

                if (Index(cell, "s") is { } style) builder.SetCell(rowIndex, column, pooled[style]);
            }
        }

        return builder.Build();

        int? Index(XElement element, string name)
            => Xlsx.Integer(element, name) is { } value && value >= 0 && value < formats.Count
                ? value
                : null;
    }
}

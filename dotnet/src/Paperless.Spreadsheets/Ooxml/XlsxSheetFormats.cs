using System.Globalization;
using System.Xml.Linq;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// Which cell format each cell of one sheet uses, and which cells hold more than one.
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
/// <para>
/// Rich text is read in the same pass because it needs the same two answers — which cell, and what
/// format does it resolve to — and because it can only be read from the cells: the shared string
/// table says which <em>strings</em> carry runs and nothing about where they are used.
/// </para>
/// </remarks>
internal static class XlsxSheetFormats
{
    /// <summary>Reads one sheet's per-cell format indices and its rich cells.</summary>
    /// <param name="worksheet">The <c>worksheet</c> root, or null when the part is missing.</param>
    /// <param name="table">The workbook's cell formats and palette.</param>
    /// <param name="file">The workbook, for its shared strings and number formats.</param>
    public static (SheetCellFormats Formats, SheetRichText RichText) Read(
        XElement? worksheet, XlsxCellFormatTable table, XlsxFile file)
    {
        ArgumentNullException.ThrowIfNull(table);
        ArgumentNullException.ThrowIfNull(file);

        IReadOnlyList<SheetCellFormat> formats = table.Formats;
        if (worksheet is null || formats.Count == 0)
            return (SheetCellFormats.Empty, SheetRichText.Empty);

        SheetCellFormats.Builder builder = new();
        SheetRichText.Builder rich = new();
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

            int? rowFormat = null;
            if (Xlsx.Flag(row, "customFormat") && Index(row, "s") is { } rowStyle)
            {
                builder.SetRow(rowIndex, pooled[rowStyle]);
                rowFormat = rowStyle;
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

                int? style = Index(cell, "s");
                if (style is { } own) builder.SetCell(rowIndex, column, pooled[own]);

                ReadRichCell(cell, rowIndex, column, style ?? rowFormat ?? 0);
            }
        }

        return (builder.Build(), rich.Build());

        int? Index(XElement element, string name)
            => Xlsx.Integer(element, name) is { } value && value >= 0 && value < formats.Count
                ? value
                : null;

        void ReadRichCell(XElement cell, int row, int column, int style)
        {
            (IReadOnlyList<XlsxRichRun> runs, string text) = RunsOf(cell, file);
            if (runs.Count == 0) return;

            SheetCellFormat cellFormat = formats[Math.Clamp(style, 0, formats.Count - 1)];
            List<SheetTextPortion> portions = [];

            foreach (XlsxRichRun run in runs)
            {
                if (run.Font is not { } font) continue;

                portions.Add(new SheetTextPortion(
                    run.Start, run.Length, table.Apply(cellFormat, font)));
            }

            rich.Set(row, column, text, cellFormat, portions);
        }
    }

    /// <summary>
    /// The formatting runs a cell's text carries, and the text they index into.
    /// </summary>
    /// <remarks>
    /// Two routes reach the same place: <c>t="s"</c> names a shared string whose runs were read
    /// with the table, and <c>t="inlineStr"</c> writes the whole rich string in the cell. Nothing
    /// else can be rich — a formula's cached string result and a number both carry one format.
    /// </remarks>
    private static (IReadOnlyList<XlsxRichRun> Runs, string Text) RunsOf(XElement cell, XlsxFile file)
    {
        switch (Xlsx.Attribute(cell, "t"))
        {
            case "s":
            {
                if (Xlsx.Child(cell, "v") is not { } value
                    || !int.TryParse(
                        value.Value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int index))
                {
                    return ([], string.Empty);
                }

                IReadOnlyList<XlsxRichRun>? runs = file.SharedStrings.RunsAt(index);
                return runs is null ? ([], string.Empty) : (runs, file.SharedStrings[index] ?? string.Empty);
            }

            case "inlineStr":
            {
                XElement? inline = Xlsx.Child(cell, "is");
                IReadOnlyList<XlsxRichRun>? runs = XlsxRichRuns.Read(inline);
                return runs is null
                    ? ([], string.Empty)
                    : (runs, XlsxSharedStrings.ReadRichString(inline));
            }

            default:
                return ([], string.Empty);
        }
    }
}

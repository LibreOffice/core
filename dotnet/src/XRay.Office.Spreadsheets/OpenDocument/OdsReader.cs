using System.Xml.Linq;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Formats;
using XRay.Office.OpenDocument;

namespace XRay.Office.Spreadsheets.OpenDocument;

/// <summary>
/// Reads ODF spreadsheets: <c>ods</c>, the <c>ots</c> template, and the flat <c>fods</c>.
/// </summary>
/// <remarks>
/// Each <c>table:table</c> becomes a <see cref="SectionKind.Sheet"/> section holding one
/// <see cref="ContentTable"/>. Hidden sheets are extracted and flagged rather than skipped:
/// a caller indexing content wants them, and a caller reproducing what a reader sees can
/// check <see cref="ContentSection.IsHidden"/>.
/// </remarks>
public sealed class OdsReader : OdfReader
{
    /// <inheritdoc/>
    protected override DocumentFamily Family => DocumentFamily.Spreadsheet;

    /// <summary>
    /// Reads a spreadsheet, with the print setup a layout needs.
    /// </summary>
    /// <remarks>
    /// Distinct from the inherited <see cref="OdfReader.Read"/> because that one answers with an
    /// <see cref="OdfDocument"/>, which is the right shape for all three ODF families and knows
    /// nothing about print ranges. This wraps it in something that can be paginated.
    /// </remarks>
    /// <param name="source">The document to read.</param>
    /// <param name="format">The identified format, recorded on the result.</param>
    public OdsSpreadsheetDocument ReadSpreadsheet(
        Core.Documents.DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        OdfDocument document = Read(source, format);
        try
        {
            return new OdsSpreadsheetDocument(
                document, OdsSpreadsheetDocument.ReadSheets(document, source.FileName));
        }
        catch
        {
            document.Dispose();
            throw;
        }
    }

    /// <inheritdoc/>
    protected override void ReadBody(XElement body, OdfContentReader reader, ContentDocument content)
    {
        ArgumentNullException.ThrowIfNull(body);
        ArgumentNullException.ThrowIfNull(reader);
        ArgumentNullException.ThrowIfNull(content);

        // Calc's cell filter does no white-space normalisation, so a raw newline inside a
        // cell's text:p is a line of its own rather than a space; see CellTextIsVerbatim.
        reader.CellTextIsVerbatim = true;

        // And a drawing anchored in a cell is in the drawing layer rather than in the cell, so
        // its paragraphs are kept in a frame section of their own; see CellShapesAreOwnFlow.
        reader.CellShapesAreOwnFlow = true;

        int index = 0;
        foreach (XElement table in body.Elements(XName.Get("table", OdfNamespaces.Table)))
        {
            ContentSection sheet = new()
            {
                Kind = SectionKind.Sheet,
                Index = index,
                Name = OdfContentReader.SheetName(table, index),
                IsHidden = reader.IsTableHidden(table),
            };
            sheet.Children.Add(reader.ReadTableElement(table));
            content.Children.Add(sheet);

            // Cell comments belong to the sheet that holds them.
            foreach (ContentNode node in reader.TakeHoisted()) content.Children.Add(node);
            index++;
        }
    }
}

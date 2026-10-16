using System.Text;
using System.Xml.Linq;
using XRay.Office.OpenDocument;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.OpenDocument;

/// <summary>
/// The cell notes a <c>table:table</c> holds, for the pages they may be listed on.
/// </summary>
/// <remarks>
/// <para>
/// ODF fastens a note to its cell by containment: <c>office:annotation</c> is a child of the
/// <c>table:table-cell</c> it belongs to, so the address comes from the walk rather than from an
/// attribute. That is the same shape as the cell-anchored drawing this reader already steps past,
/// and for the same reason it is read here rather than out of the content tree —
/// <c>OdfContentReader</c> hoists an annotation into a section of its own, which keeps it out of
/// the cell's text and also loses which cell it came from.
/// </para>
/// <para>
/// <strong>The author line is inside the text and is not <c>dc:creator</c>.</strong> LibreOffice
/// writes the name the note opens with as an ordinary <c>text:span</c> of the first paragraph and
/// puts <c>Unknown Author</c> in <c>dc:creator</c>, so a reader that composed the two would print
/// the wrong name twice. Read straight off 26.2.4.2's rendering of
/// <c>RMP 2011-2014 and Inventory.ods</c>, whose note page carries
/// <c>B54:</c> then <c>Elina Zheleva:</c> then <c>ex OPS.026</c> — the mark, and then the two
/// paragraphs of the annotation exactly as the file states them. See
/// <see cref="SheetNotes"/>, which is where the same string arrives from the two Excel families.
/// </para>
/// </remarks>
internal static class OdsNotes
{
    /// <summary>How far a repeat count is honoured, matching the other ODS walks' cap.</summary>
    private const int MaxRepeat = 4096;

    /// <summary>Reads a sheet's cell notes.</summary>
    /// <param name="table">The <c>table:table</c> element.</param>
    public static SheetNotes Read(XElement table)
    {
        ArgumentNullException.ThrowIfNull(table);

        List<SheetNote> notes = [];
        int row = 0;

        foreach (XElement rowElement in table.Descendants(XName.Get("table-row", OdfNamespaces.Table)))
        {
            int repeat = Repeat(rowElement, "number-rows-repeated");

            if (Math.Min(repeat, MaxRepeat) > 0 && row <= SheetAddress.MaxRow)
            {
                int column = 0;

                foreach (XElement cell in rowElement.Elements())
                {
                    if (cell.Name.NamespaceName != OdfNamespaces.Table) continue;
                    if (cell.Name.LocalName is not ("table-cell" or "covered-table-cell")) continue;

                    if (column <= SheetAddress.MaxColumn)
                    {
                        foreach (XElement annotation in
                                 cell.Elements(XName.Get("annotation", OdfNamespaces.Office)))
                        {
                            string text = TextOf(annotation);
                            if (text.Length > 0) notes.Add(new SheetNote(column, row, text));
                        }
                    }

                    column += Repeat(cell, "number-columns-repeated");
                    if (column > SheetAddress.MaxColumn) break;
                }
            }

            row += repeat;
            if (row > SheetAddress.MaxRow) break;
        }

        return notes.Count == 0 ? SheetNotes.Empty : new SheetNotes { Items = notes };
    }

    /// <summary>The annotation's paragraphs, one per line.</summary>
    private static string TextOf(XElement annotation)
    {
        StringBuilder text = new();

        foreach (XElement paragraph in
                 annotation.Descendants(XName.Get("p", OdfNamespaces.Text)))
        {
            if (text.Length > 0) text.Append('\n');
            text.Append(paragraph.Value);
        }

        return text.ToString().TrimEnd('\n');
    }

    private static int Repeat(XElement element, string name)
    {
        string? stated = element.Attribute(XName.Get(name, OdfNamespaces.Table))?.Value;
        return OdfValue.ParseInt(stated) is { } value && value >= 1 ? value : 1;
    }
}

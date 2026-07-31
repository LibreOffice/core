using Paperless.Core.Extraction;

namespace Paperless.Markup.Tests;

/// <summary>
/// Builds small content trees by hand, so a writer test states exactly what went in.
/// </summary>
/// <remarks>
/// The writers take the content tree and nothing else, which is what makes them testable
/// without opening a document at all — no reader, no format, no LibreOffice. A test that had to
/// find a corpus file exercising some structure would be testing the reader as much as the
/// writer, and would be unable to construct the awkward cases at all.
/// </remarks>
internal static class Tree
{
    public static ContentDocument Document(params ContentNode[] children)
    {
        ContentDocument document = new();
        foreach (ContentNode child in children) document.Children.Add(child);
        return document;
    }

    public static ContentSection Body(params ContentNode[] children)
        => Section(SectionKind.Body, null, children);

    public static ContentSection Section(SectionKind kind, string? name, params ContentNode[] children)
        => Section(kind, name, 0, children);

    public static ContentSection Section(SectionKind kind, string? name, int index, params ContentNode[] children)
    {
        ContentSection section = new() { Kind = kind, Name = name, Index = index };
        foreach (ContentNode child in children) section.Children.Add(child);
        return section;
    }

    /// <summary>A body section holding one paragraph of plain text — the commonest case.</summary>
    public static ContentDocument Prose(string text) => Document(Body(Para(text)));

    public static ContentParagraph Para(string text, RunEmphasis emphasis = RunEmphasis.None)
    {
        ContentParagraph paragraph = new();
        paragraph.Children.Add(new ContentRun { Text = text, Emphasis = emphasis });
        return paragraph;
    }

    public static ContentParagraph Heading(int level, string text)
    {
        ContentParagraph paragraph = new() { HeadingLevel = level };
        paragraph.Children.Add(new ContentRun { Text = text });
        return paragraph;
    }

    public static ContentParagraph Item(int level, string? marker, string text)
    {
        ContentParagraph paragraph = new() { ListLevel = level, ListMarker = marker };
        paragraph.Children.Add(new ContentRun { Text = text });
        return paragraph;
    }

    public static ContentParagraph Styled(string style, string text)
    {
        ContentParagraph paragraph = new() { StyleName = style };
        paragraph.Children.Add(new ContentRun { Text = text });
        return paragraph;
    }

    public static ContentParagraph Runs(params ContentRun[] runs)
    {
        ContentParagraph paragraph = new();
        foreach (ContentRun run in runs) paragraph.Children.Add(run);
        return paragraph;
    }

    public static ContentTable Table(int headerRows, params ContentTableRow[] rows)
    {
        ContentTable table = new()
        {
            HeaderRowCount = headerRows,
            ColumnCount = rows.Length == 0 ? 0 : rows.Max(row => row.Children.Count),
        };
        foreach (ContentTableRow row in rows) table.Children.Add(row);
        return table;
    }

    public static ContentTableRow Row(int index, params ContentTableCell[] cells)
    {
        ContentTableRow row = new() { Index = index };
        foreach (ContentTableCell cell in cells) row.Children.Add(cell);
        return row;
    }

    public static ContentTableCell Cell(string text, int columnSpan = 1, int rowSpan = 1)
    {
        ContentTableCell cell = new() { ColumnSpan = columnSpan, RowSpan = rowSpan };
        cell.Children.Add(Para(text));
        return cell;
    }

    public static ContentTableCell CellOf(params ContentNode[] content)
    {
        ContentTableCell cell = new();
        foreach (ContentNode node in content) cell.Children.Add(node);
        return cell;
    }
}

namespace Paperless.Core.Extraction;

/// <summary>
/// A node in the format-independent content tree that extraction produces.
/// </summary>
/// <remarks>
/// <para>
/// Extraction deliberately targets a single tree shared by all three document
/// families rather than one model per family. Callers indexing a mixed corpus want
/// "the text, the tables and the structure" and should not have to branch on
/// whether a file was a spreadsheet or a deck.
/// </para>
/// <para>
/// The tree keeps structure that carries meaning — headings, lists, tables, sheet
/// and slide boundaries — and discards presentation that does not, such as exact
/// positions and fonts. Anything needing that detail should render instead.
/// </para>
/// </remarks>
public abstract class ContentNode
{
    /// <summary>Child nodes, in document order.</summary>
    public IList<ContentNode> Children { get; } = [];

    /// <summary>
    /// Concatenates the text of this node and everything beneath it, inserting
    /// newlines at block boundaries.
    /// </summary>
    public string GetText()
    {
        System.Text.StringBuilder sb = new();
        AppendText(sb);
        return sb.ToString();
    }

    /// <summary>Appends this node's text to a builder. Overridden by leaf nodes.</summary>
    protected internal virtual void AppendText(System.Text.StringBuilder builder)
    {
        foreach (ContentNode child in Children) child.AppendText(builder);
    }
}

/// <summary>The root of an extracted document.</summary>
public sealed class ContentDocument : ContentNode
{
    /// <summary>The document's metadata.</summary>
    public Documents.DocumentMetadata Metadata { get; init; } = Documents.DocumentMetadata.Empty;
}

/// <summary>
/// A top-level division of the document: a section in a word-processing file, a
/// sheet in a spreadsheet, or a slide in a presentation.
/// </summary>
public sealed class ContentSection : ContentNode
{
    /// <summary>What kind of division this is.</summary>
    public required SectionKind Kind { get; init; }

    /// <summary>The zero-based index of this section among its siblings.</summary>
    public int Index { get; init; }

    /// <summary>The sheet or slide name, where the format records one.</summary>
    public string? Name { get; init; }

    /// <summary>
    /// True when the section is hidden — a hidden sheet or a skipped slide.
    /// Callers indexing content usually still want it; callers reproducing what a
    /// reader sees usually do not.
    /// </summary>
    public bool IsHidden { get; init; }
}

/// <summary>The kinds of <see cref="ContentSection"/>.</summary>
public enum SectionKind
{
    /// <summary>A word-processing body section.</summary>
    Body,

    /// <summary>A spreadsheet worksheet.</summary>
    Sheet,

    /// <summary>A presentation slide.</summary>
    Slide,

    /// <summary>A slide's speaker notes.</summary>
    SlideNotes,

    /// <summary>Page header content.</summary>
    Header,

    /// <summary>Page footer content.</summary>
    Footer,

    /// <summary>Footnote or endnote content.</summary>
    Note,

    /// <summary>A comment or annotation.</summary>
    Comment,
}

/// <summary>A paragraph of text.</summary>
public sealed class ContentParagraph : ContentNode
{
    /// <summary>
    /// The outline level when the paragraph is a heading: 1 for the top level,
    /// null when the paragraph is body text.
    /// </summary>
    public int? HeadingLevel { get; init; }

    /// <summary>The name of the paragraph style applied, as recorded in the file.</summary>
    public string? StyleName { get; init; }

    /// <summary>
    /// The list nesting depth when the paragraph is a list item, starting at 0.
    /// Null when the paragraph is not part of a list.
    /// </summary>
    public int? ListLevel { get; init; }

    /// <summary>The rendered list marker, e.g. "3." or a bullet character.</summary>
    public string? ListMarker { get; init; }

    /// <inheritdoc/>
    protected internal override void AppendText(System.Text.StringBuilder builder)
    {
        base.AppendText(builder);
        builder.Append('\n');
    }
}

/// <summary>A run of text with uniform character formatting.</summary>
public sealed class ContentRun : ContentNode
{
    /// <summary>The text.</summary>
    public required string Text { get; init; }

    /// <summary>The character style name applied, as recorded in the file.</summary>
    public string? StyleName { get; init; }

    /// <summary>The BCP 47 language tag applied to this run, if recorded.</summary>
    public string? Language { get; init; }

    /// <summary>Coarse emphasis flags, kept because they often carry meaning.</summary>
    public RunEmphasis Emphasis { get; init; }

    /// <summary>
    /// The target when the run is a hyperlink: a URL, or a document-internal
    /// reference such as a bookmark name or cell address.
    /// </summary>
    public string? HyperlinkTarget { get; init; }

    /// <inheritdoc/>
    protected internal override void AppendText(System.Text.StringBuilder builder)
        => builder.Append(Text);
}

/// <summary>Coarse character emphasis preserved during extraction.</summary>
[Flags]
public enum RunEmphasis
{
    /// <summary>No emphasis.</summary>
    None = 0,

    /// <summary>Bold.</summary>
    Bold = 1 << 0,

    /// <summary>Italic.</summary>
    Italic = 1 << 1,

    /// <summary>Underlined.</summary>
    Underline = 1 << 2,

    /// <summary>Struck through.</summary>
    Strikethrough = 1 << 3,

    /// <summary>Superscript.</summary>
    Superscript = 1 << 4,

    /// <summary>Subscript.</summary>
    Subscript = 1 << 5,
}

/// <summary>A table. Children are <see cref="ContentTableRow"/> nodes.</summary>
public sealed class ContentTable : ContentNode
{
    /// <summary>The number of columns, counting the widest row.</summary>
    public int ColumnCount { get; init; }

    /// <summary>How many leading rows are header rows.</summary>
    public int HeaderRowCount { get; init; }
}

/// <summary>A table row. Children are <see cref="ContentTableCell"/> nodes.</summary>
public sealed class ContentTableRow : ContentNode
{
    /// <summary>The zero-based row index within the table.</summary>
    public int Index { get; init; }
}

/// <summary>
/// A table cell, or a spreadsheet cell. Children hold the cell's content.
/// </summary>
public sealed class ContentTableCell : ContentNode
{
    /// <summary>The zero-based column index.</summary>
    public int Column { get; init; }

    /// <summary>The zero-based row index.</summary>
    public int Row { get; init; }

    /// <summary>How many columns the cell spans. 1 for an unmerged cell.</summary>
    public int ColumnSpan { get; init; } = 1;

    /// <summary>How many rows the cell spans. 1 for an unmerged cell.</summary>
    public int RowSpan { get; init; } = 1;

    /// <summary>
    /// The cell's underlying value for spreadsheet cells: a <see cref="double"/>,
    /// <see cref="string"/>, <see cref="bool"/>, <see cref="DateTime"/>, or a
    /// <see cref="CellError"/>. Null for word-processing table cells.
    /// </summary>
    /// <remarks>
    /// This is the unformatted value. The children hold the <em>displayed</em> text,
    /// which for a spreadsheet is the value run through its number format — the two
    /// differ, and consumers want different ones.
    /// </remarks>
    public object? Value { get; init; }

    /// <summary>
    /// The cell's formula without a leading <c>=</c>, in the source file's own
    /// syntax. Null when the cell is not a formula.
    /// </summary>
    public string? Formula { get; init; }
}

/// <summary>A spreadsheet error value.</summary>
public enum CellError
{
    /// <summary>An unrecognised or generic error.</summary>
    Unknown = 0,

    /// <summary>Division by zero.</summary>
    DivideByZero,

    /// <summary>Wrong value type.</summary>
    Value,

    /// <summary>Invalid cell reference.</summary>
    Reference,

    /// <summary>Unrecognised name.</summary>
    Name,

    /// <summary>Numeric overflow or invalid numeric operation.</summary>
    Number,

    /// <summary>Value not available.</summary>
    NotAvailable,

    /// <summary>Empty intersection of two ranges.</summary>
    Null,

    /// <summary>A circular reference.</summary>
    Circular,
}

/// <summary>
/// An embedded image or other graphic. Extraction records that it exists and any
/// alternative text, but does not decode the pixels.
/// </summary>
public sealed class ContentImage : ContentNode
{
    /// <summary>Alternative text or description, if the document records one.</summary>
    public string? AlternativeText { get; init; }

    /// <summary>The image's media type, e.g. <c>image/png</c>, if known.</summary>
    public string? MediaType { get; init; }

    /// <summary>The name of the container part holding the bytes, if applicable.</summary>
    public string? PartName { get; init; }
}

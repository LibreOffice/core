using System.Text;

namespace Paperless.WordProcessing.Model;

/// <summary>The kinds of content a body can hold, and the kinds of body there are.</summary>
public enum WritingBodyKind
{
    /// <summary>The document's main text.</summary>
    Body,

    /// <summary>A table cell's content.</summary>
    Cell,

    /// <summary>A page header.</summary>
    Header,

    /// <summary>A page footer.</summary>
    Footer,

    /// <summary>A footnote or endnote's text.</summary>
    Note,

    /// <summary>A comment's text.</summary>
    Comment,

    /// <summary>A frame's or text box's content.</summary>
    Frame,

    /// <summary>An "Insert &gt; Section" range, a table of contents body, or an index body.</summary>
    Section,
}

/// <summary>
/// A node in the layout-facing document model.
/// </summary>
/// <remarks>
/// <para>
/// A tree, not the flat array with paired start and end markers Writer uses. The markers exist in
/// LibreOffice because its node array is also its edit buffer and its undo unit; the research notes
/// say so explicitly, and say a conventional tree is semantically equivalent and the more natural C#
/// shape (<c>research/02-writer.md</c> section A.2).
/// </para>
/// <para>
/// What the flat array <em>is</em> needed for is document order: layout, tracked changes, bookmarks
/// and cross-references all ask "is this before that" and "is this inside that range". So every node
/// carries a document-order index, assigned by a walk of the finished tree, and comparisons use it
/// rather than searching the tree.
/// </para>
/// </remarks>
public abstract class WritingNode
{
    /// <summary>
    /// The node that owns this one, or null for a document's body and its other flows.
    /// </summary>
    /// <remarks>
    /// Any node, not just a body: a cell owns its content, a row owns its cells and a table owns its
    /// rows, and only a body-typed parent would leave those three unowned — which is exactly the case
    /// where a node could be aliased into two places without anything noticing.
    /// </remarks>
    public WritingNode? Parent { get; internal set; }

    /// <summary>
    /// This node's position in document order, or -1 before the order has been assigned.
    /// </summary>
    /// <remarks>
    /// Assigned by <see cref="WritingDocument.AssignDocumentOrder"/> rather than maintained as the
    /// tree is built: an importer appends thousands of nodes and would pay for a renumbering at each
    /// one, while nothing reads the order until the document is complete.
    /// </remarks>
    public int DocumentOrder { get; internal set; } = -1;

    /// <summary>
    /// One past the document-order index of this node's last descendant.
    /// </summary>
    /// <remarks>
    /// Together with <see cref="DocumentOrder"/> this is the node's whole extent, which is what makes
    /// "is X inside Y" a pair of integer comparisons rather than a walk up the parents.
    /// </remarks>
    public int DocumentOrderEnd { get; internal set; } = -1;

    /// <summary>True when this node contains another, or is it.</summary>
    public bool Contains(WritingNode other)
    {
        ArgumentNullException.ThrowIfNull(other);
        return DocumentOrder >= 0
            && other.DocumentOrder >= DocumentOrder
            && other.DocumentOrderEnd <= DocumentOrderEnd;
    }

    /// <summary>
    /// Records that a node now belongs to a container, refusing a node that already belongs to one.
    /// </summary>
    /// <remarks>
    /// A node has one owner because both its document order and any position into it are derived from
    /// where it sits — a node in two places would have two of each, and every comparison against it
    /// would depend on which one the caller happened to reach it through.
    /// </remarks>
    protected static void Claim(WritingNode node, WritingNode owner)
    {
        ArgumentNullException.ThrowIfNull(node);
        ArgumentNullException.ThrowIfNull(owner);

        if (node.Parent is not null)
        {
            throw new InvalidOperationException(
                "The node already belongs to another node. A node has one owner, because its document "
                + "order and any position into it are both derived from where it sits.");
        }

        node.Parent = owner;
    }

    /// <summary>Appends this node's text, for tests and diagnostics rather than for rendering.</summary>
    protected internal abstract void AppendText(StringBuilder text);

    /// <summary>This node's text.</summary>
    public string GetText()
    {
        StringBuilder text = new();
        AppendText(text);
        return text.ToString();
    }
}

/// <summary>
/// A run of nodes making up one flow of text: the body, a cell, a header, a note.
/// </summary>
/// <remarks>
/// The equivalent of the range between one of Writer's start nodes and its matching end node. Every
/// nesting in the model is a body inside something — which is why a table cell and a footnote are the
/// same type of container, differing only in <see cref="Kind"/>.
/// </remarks>
public sealed class WritingBody : WritingNode
{
    private readonly List<WritingNode> _children = [];

    /// <summary>Creates a body.</summary>
    /// <param name="kind">What sort of flow this is.</param>
    public WritingBody(WritingBodyKind kind) => Kind = kind;

    /// <summary>What sort of flow this is.</summary>
    public WritingBodyKind Kind { get; }

    /// <summary>A name for the flow, where the format records one.</summary>
    public string? Name { get; set; }

    /// <summary>The nodes in this flow, in order.</summary>
    public IReadOnlyList<WritingNode> Children => _children;

    /// <summary>Appends a node, taking ownership of it.</summary>
    /// <exception cref="InvalidOperationException">The node already has an owner.</exception>
    public T Add<T>(T node) where T : WritingNode
    {
        Claim(node, this);
        _children.Add(node);
        return node;
    }

    /// <inheritdoc/>
    protected internal override void AppendText(StringBuilder text)
    {
        ArgumentNullException.ThrowIfNull(text);
        foreach (WritingNode child in _children) child.AppendText(text);
    }
}

/// <summary>
/// A paragraph: its text as one string, plus the character formatting as intervals over it.
/// </summary>
/// <remarks>
/// The text includes the placeholder characters that stand for fields, note anchors, comment anchors
/// and inline frames — see <see cref="TextHint.IsAnchor"/>. Keeping them means every offset in
/// the document agrees on what a position counts, which is what the importers' own offset arithmetic
/// already assumes.
/// </remarks>
public sealed class WritingParagraph : WritingNode
{
    private readonly StringBuilder _text = new();

    /// <summary>The paragraph style's name, when one is applied.</summary>
    public string? StyleName { get; set; }

    /// <summary>The outline level when the paragraph is a heading, 1 for the top level.</summary>
    public int? HeadingLevel { get; set; }

    /// <summary>The list this paragraph belongs to, when it is a list item.</summary>
    public string? ListId { get; set; }

    /// <summary>The list nesting level, starting at zero.</summary>
    public int? ListLevel { get; set; }

    /// <summary>The rendered list label, which most formats store nowhere and the reader computes.</summary>
    public string? ListLabel { get; set; }

    /// <summary>Direct paragraph-level formatting, belonging to no style.</summary>
    public object? DirectFormatting { get; set; }

    /// <summary>The character formatting over this paragraph's text.</summary>
    public TextHints Hints { get; } = new();

    /// <summary>The paragraph's text, placeholder characters included.</summary>
    public string Text => _text.ToString();

    /// <summary>How many characters the paragraph holds, placeholders included.</summary>
    public int Length => _text.Length;

    /// <summary>Appends text at the end, leaving the attributes alone.</summary>
    public void Append(string text)
    {
        ArgumentNullException.ThrowIfNull(text);
        _text.Append(text);
    }

    /// <summary>
    /// Appends a placeholder character for something that occupies a position but has no text.
    /// </summary>
    /// <returns>The position the placeholder took, so a caller can attribute it.</returns>
    public int AppendAnchor()
    {
        int position = _text.Length;
        _text.Append(AnchorCharacter);
        return position;
    }

    /// <summary>
    /// The character standing in for a field, an anchor or an inline frame.
    /// </summary>
    /// <remarks>
    /// U+0001, as Writer uses. It has to be a character that cannot appear in text, because it is
    /// distinguishable from content only by the attribute at its position.
    /// </remarks>
    public const char AnchorCharacter = '\u0001';

    /// <summary>Inserts text, moving the attributes after it along.</summary>
    public void Insert(int position, string text)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentOutOfRangeException.ThrowIfNegative(position);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(position, _text.Length);

        _text.Insert(position, text);
        Hints.Insert(position, text.Length);
    }

    /// <summary>Deletes a range of text, trimming the attributes over it.</summary>
    public void Delete(int position, int length)
    {
        ArgumentOutOfRangeException.ThrowIfNegative(position);
        ArgumentOutOfRangeException.ThrowIfNegative(length);
        if (position + length > _text.Length) length = _text.Length - position;
        if (length <= 0) return;

        _text.Remove(position, length);
        Hints.Delete(position, length);
    }

    /// <inheritdoc/>
    protected internal override void AppendText(StringBuilder text)
    {
        ArgumentNullException.ThrowIfNull(text);
        if (ListLabel is { Length: > 0 } label) text.Append(label).Append(' ');

        // The placeholders stand for things with no text of their own, so they are dropped here even
        // though they are part of the paragraph's length.
        foreach (char character in _text.ToString())
        {
            if (character != AnchorCharacter) text.Append(character);
        }
        text.Append('\n');
    }
}

/// <summary>A table: rows of cells, each cell a body of its own.</summary>
public sealed class WritingTable : WritingNode
{
    private readonly List<WritingTableRow> _rows = [];

    /// <summary>The rows, in order.</summary>
    public IReadOnlyList<WritingTableRow> Rows => _rows;

    /// <summary>The table style's name, when one is applied.</summary>
    public string? StyleName { get; set; }

    /// <summary>
    /// The column grid: each column's right edge, so a cell's span is a pair of indexes into it.
    /// </summary>
    /// <remarks>
    /// Edges rather than widths, because that is what every format records and what makes rows with
    /// different cell counts comparable — the same reason both legacy readers derive spans from a
    /// grid rather than from a per-cell width.
    /// </remarks>
    public List<int> ColumnEdges { get; } = [];

    /// <summary>How many rows repeat as a header at the top of each page the table spans.</summary>
    public int HeaderRowCount { get; set; }

    /// <summary>Appends a row.</summary>
    /// <exception cref="InvalidOperationException">The row already has an owner.</exception>
    public WritingTableRow Add(WritingTableRow row)
    {
        Claim(row, this);
        row.Index = _rows.Count;
        _rows.Add(row);
        return row;
    }

    /// <inheritdoc/>
    protected internal override void AppendText(StringBuilder text)
    {
        ArgumentNullException.ThrowIfNull(text);
        foreach (WritingTableRow row in _rows) row.AppendText(text);
    }
}

/// <summary>One row of a table.</summary>
public sealed class WritingTableRow : WritingNode
{
    private readonly List<WritingTableCell> _cells = [];

    /// <summary>The table this row belongs to.</summary>
    public WritingTable? Table => Parent as WritingTable;

    /// <summary>The row's position in its table.</summary>
    public int Index { get; internal set; }

    /// <summary>True when the row repeats as a header on every page the table spans.</summary>
    public bool IsHeader { get; set; }

    /// <summary>The cells, left to right, excluding those a merge covers.</summary>
    public IReadOnlyList<WritingTableCell> Cells => _cells;

    /// <summary>Appends a cell.</summary>
    /// <exception cref="InvalidOperationException">The cell already has an owner.</exception>
    public WritingTableCell Add(WritingTableCell cell)
    {
        Claim(cell, this);
        _cells.Add(cell);
        return cell;
    }

    /// <inheritdoc/>
    protected internal override void AppendText(StringBuilder text)
    {
        ArgumentNullException.ThrowIfNull(text);
        for (int i = 0; i < _cells.Count; i++)
        {
            if (i > 0) text.Append('\t');
            text.Append(_cells[i].GetText().TrimEnd('\n'));
        }
        text.Append('\n');
    }
}

/// <summary>
/// One cell of a table row: a body, plus where it sits in the column grid.
/// </summary>
public sealed class WritingTableCell : WritingNode
{
    /// <summary>Creates a cell with an empty body.</summary>
    public WritingTableCell() => Content = new WritingBody(WritingBodyKind.Cell) { Parent = this };

    /// <summary>The row this cell belongs to.</summary>
    public WritingTableRow? Row => Parent as WritingTableRow;

    /// <summary>The cell's content.</summary>
    public WritingBody Content { get; }

    /// <summary>The grid column the cell starts at.</summary>
    public int Column { get; set; }

    /// <summary>How many grid columns the cell spans.</summary>
    public int ColumnSpan { get; set; } = 1;

    /// <summary>How many rows the cell spans.</summary>
    public int RowSpan { get; set; } = 1;

    /// <inheritdoc/>
    protected internal override void AppendText(StringBuilder text)
    {
        ArgumentNullException.ThrowIfNull(text);
        Content.AppendText(text);
    }
}

/// <summary>An image or embedded object that stands alone rather than inside a paragraph.</summary>
/// <remarks>
/// A node of its own, as Writer models it: a picture anchored to a paragraph is not part of that
/// paragraph's text, and only an as-character anchored one is — which the model expresses as an
/// <see cref="TextHintKind.InlineFrame"/> attribute over a placeholder character instead.
/// </remarks>
public sealed class WritingGraphic : WritingNode
{
    /// <summary>A description of the graphic, where the format records one.</summary>
    public string? Description { get; set; }

    /// <summary>The name of the package part holding the bytes, where there is one.</summary>
    public string? PartName { get; set; }

    /// <inheritdoc/>
    protected internal override void AppendText(StringBuilder text) { }
}

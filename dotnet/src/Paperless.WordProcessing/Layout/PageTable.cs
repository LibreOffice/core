using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// How much space a cell leaves between its border and its text.
/// </summary>
/// <remarks>
/// Every format has this and every format defaults it differently — Word to 108 twips left and right and
/// nothing vertically, ODF to 0.097 cm on all four sides — so the reader resolves it and this carries the
/// answer. It comes out of the cell's width, which makes it a line-breaking matter rather than a cosmetic
/// one: a cell 2 cm wide with 1 mm padding breaks its text at 18 mm.
/// </remarks>
/// <param name="Left">The gap at the cell's left edge.</param>
/// <param name="Right">At its right edge.</param>
/// <param name="Top">Above the first line.</param>
/// <param name="Bottom">Below the last.</param>
public readonly record struct CellPadding(Length Left, Length Right, Length Top, Length Bottom)
{
    /// <summary>Word's default: 0.19 cm to the left and right of the text, nothing above or below.</summary>
    public static CellPadding Word { get; } =
        new(Length.FromTwips(108), Length.FromTwips(108), Length.Zero, Length.Zero);

    /// <summary>What Writer gives a table drawn in it: 0.097 cm on all four sides.</summary>
    public static CellPadding Writer { get; } = Uniform(Length.FromMm100(97));

    /// <summary>The same gap on all four sides.</summary>
    public static CellPadding Uniform(Length all) => new(all, all, all, all);

    /// <summary>How much width the two horizontal gaps take together.</summary>
    public Length Horizontal => Left + Right;

    /// <summary>How much height the two vertical gaps take together.</summary>
    public Length Vertical => Top + Bottom;
}

/// <summary>Where a cell's text sits when its content is shorter than its row.</summary>
public enum CellVerticalAlignment
{
    /// <summary>Against the top of the cell, which is every format's default.</summary>
    Top,

    /// <summary>Centred in the spare height.</summary>
    Middle,

    /// <summary>Against the bottom.</summary>
    Bottom,
}

/// <summary>
/// A table waiting to be paginated: a column grid, and rows of cells that flow inside it.
/// </summary>
/// <remarks>
/// <para>
/// The grid is stated as column widths rather than as a count, because that is what every format states
/// and because a cell's width is what decides its line breaks. A cell spanning two columns is one cell
/// with a <see cref="PageTableCell.ColumnSpan"/> of two, whose width is the two columns' widths added —
/// which is the reverse of how Writer stores it internally, where the width is primary and the span
/// implied, but the same information.
/// </para>
/// <para>
/// Heights are absent on purpose: a row is as tall as its tallest cell, and a cell is as tall as its
/// content at its own width. Only a floor can be stated (<see cref="PageTableRow.MinHeight"/>), because
/// only a floor is what the formats mean by a row height — the value is honoured when the content is
/// shorter and ignored when it is taller.
/// </para>
/// </remarks>
public sealed record PageTable : PageBlock
{
    /// <summary>How many columns and rows are laid out before the rest are dropped.</summary>
    /// <remarks>
    /// A guard on untrusted input. Word's own limit is 63 columns and Writer's is far higher, but a
    /// generated file can declare a grid of any size, and the layout cost is the product of the two.
    /// </remarks>
    public const int MaxColumns = 256;

    /// <inheritdoc cref="MaxColumns"/>
    public const int MaxRows = 20000;

    /// <summary>The grid's column widths, left to right.</summary>
    public required IReadOnlyList<Length> ColumnWidths { get; init; }

    /// <summary>The rows, top to bottom.</summary>
    public required IReadOnlyList<PageTableRow> Rows { get; init; }

    /// <summary>How far the table's left edge sits from the body area's.</summary>
    /// <remarks>
    /// Its own value rather than a paragraph indent, because a table is indented as a whole and can be
    /// negative — a table pulled into the left margin is legal and used for full-bleed layouts.
    /// </remarks>
    public Length LeftIndent { get; init; }

    /// <summary>The space above the table.</summary>
    public Length SpaceBefore { get; init; }

    /// <summary>The space below it.</summary>
    public Length SpaceAfter { get; init; }

    /// <summary>
    /// How many rows at the top repeat when the table crosses a page break.
    /// </summary>
    /// <remarks>
    /// A count rather than a flag per row, matching <c>SwTable::GetRowsToRepeat</c>, because the feature is
    /// "the first N rows are the heading" — a repeat flag on a row further down does not make the rows
    /// above it headings, and every format states it as a run from the top.
    /// </remarks>
    public int HeaderRowCount { get; init; }

    /// <summary>How wide the table is, which is its columns added up.</summary>
    public Length Width
    {
        get
        {
            Length total = Length.Zero;
            foreach (Length column in ColumnWidths) total += column;
            return total;
        }
    }
}

/// <summary>One row of a table.</summary>
/// <remarks>
/// The cells it holds need not cover the grid: a row can be short of cells, and a format that merges
/// cells horizontally writes one wide cell rather than a placeholder for the columns it swallowed. So a
/// cell states which column it starts at rather than being found by its position in this list.
/// </remarks>
public sealed record PageTableRow
{
    /// <summary>The cells, left to right.</summary>
    public required IReadOnlyList<PageTableCell> Cells { get; init; }

    /// <summary>
    /// The row's declared height, which is a floor rather than a size.
    /// </summary>
    /// <remarks>
    /// Honoured when the content is shorter and ignored when it is taller, which is what "at least" means
    /// in all four formats. An exact row height exists in DOCX (<c>w:hRule="exact"</c>) and clips its
    /// content; it is not modelled yet, and a document using one gets the taller of the two instead.
    /// </remarks>
    public Length MinHeight { get; init; }

    /// <summary>True when the row is one of the table's repeating heading rows.</summary>
    public bool IsHeader { get; init; }
}

/// <summary>
/// One cell: where it sits in the grid, and the flow of paragraphs inside it.
/// </summary>
/// <remarks>
/// Its own paragraph list rather than a range of the body's, because a cell is a separate flow — its text
/// breaks at the cell's width and its lines are positioned from the cell's own top. The paragraphs inside
/// can be anything a body paragraph can be, per-run formatting and tab stops included, since they go
/// through the same layout path.
/// </remarks>
public sealed record PageTableCell
{
    /// <summary>The blocks inside the cell, in order.</summary>
    /// <remarks>
    /// Blocks rather than paragraphs, because a cell can hold a table — which is how every one of the four
    /// formats writes a nested table. A cell's content goes through <see cref="FlowLayouter"/>, the same
    /// path a header takes, so anything a header can hold a cell can hold.
    /// </remarks>
    public required IReadOnlyList<PageBlock> Blocks { get; init; }

    /// <summary>The grid column the cell starts at, counted from zero.</summary>
    public int Column { get; init; }

    /// <summary>How many grid columns it covers; one for an ordinary cell.</summary>
    public int ColumnSpan { get; init; } = 1;

    /// <summary>
    /// How many rows it covers downwards; one for an ordinary cell.
    /// </summary>
    /// <remarks>
    /// Stated only on the cell that <em>starts</em> the merge. The rows below it simply have no cell at
    /// that column, which is how three of the four formats write it — Writer's negative
    /// <c>mnRowSpan</c> follower boxes are an internal device for keeping its node array rectangular and
    /// have no counterpart here, since nothing downstream needs a placeholder for a cell that is not drawn.
    /// </remarks>
    public int RowSpan { get; init; } = 1;

    /// <summary>The gap between the cell's edges and its text.</summary>
    public CellPadding Padding { get; init; }

    /// <summary>Where the text sits when the row is taller than the content.</summary>
    public CellVerticalAlignment VerticalAlignment { get; init; }

    /// <summary>One past the last grid column the cell covers.</summary>
    public int ColumnEnd => Column + Math.Max(1, ColumnSpan);
}

/// <summary>
/// A table after placement: where its cells landed on a page.
/// </summary>
/// <remarks>
/// Cells rather than rows, because a cell is what gets drawn and a row is only how its height was decided
/// — and because a cell spanning rows belongs to no single one of them. Each cell carries its own
/// rectangle, so nothing downstream has to add row tops and column lefts back up.
/// </remarks>
public sealed record PlacedTable
{
    /// <summary>The table the cells came from, for a caller that needs what was not placed.</summary>
    public required PageTable Table { get; init; }

    /// <summary>The rectangle the table occupies, in page coordinates.</summary>
    public required DocRect Area { get; init; }

    /// <summary>The cells that landed here, in row-major order.</summary>
    public required IReadOnlyList<PlacedTableCell> Cells { get; init; }

    /// <summary>The first row of the table on this page, counted in the table's own rows.</summary>
    /// <remarks>
    /// Not always zero: a table split across a page break continues on the next page, and its second part
    /// starts at whichever row did not fit. Repeated heading rows are placed again and are <em>not</em>
    /// counted here, since they are not where the continuation resumed.
    /// </remarks>
    public int FirstRow { get; init; }

    /// <summary>One past the last row of the table on this page.</summary>
    public int RowEnd { get; init; }

    /// <summary>Which column of the page it sits in; zero for single-column text.</summary>
    public int Column { get; init; }

    /// <summary>True when nothing was placed.</summary>
    public bool IsEmpty => Cells.Count == 0;
}

/// <summary>One cell after placement.</summary>
/// <remarks>
/// The outer rectangle and the content are both carried because they are different rectangles: a border
/// and a background fill the outer one, and the text sits inside it by the cell's padding. Deriving one
/// from the other downstream would mean knowing the padding downstream.
/// </remarks>
public sealed record PlacedTableCell
{
    /// <summary>The cell as the document stated it.</summary>
    public required PageTableCell Cell { get; init; }

    /// <summary>Its whole rectangle, padding included, in page coordinates.</summary>
    public required DocRect Area { get; init; }

    /// <summary>Its text, laid out inside the padding, or null when the cell is empty.</summary>
    public PlacedFlow? Content { get; init; }

    /// <summary>Which row of the table it starts in.</summary>
    public int Row { get; init; }
}

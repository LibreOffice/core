using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// A point on a sheet, stated as a cell and an offset into it.
/// </summary>
/// <remarks>
/// Which is how all three formats anchor a drawing, and why a picture on a sheet cannot be placed
/// until the column widths are known: SpreadsheetML writes
/// <c>&lt;col&gt;/&lt;colOff&gt;/&lt;row&gt;/&lt;rowOff&gt;</c>, ODF writes a
/// <c>table:end-cell-address</c> with <c>table:end-x</c> and <c>table:end-y</c>, and BIFF's
/// <c>OBJ</c> client anchor states the offset as a fraction of the cell. Insert a column and every
/// picture to its right moves, which is the behaviour this shape exists to reproduce.
/// </remarks>
/// <param name="Column">The zero-based column.</param>
/// <param name="ColumnOffset">How far into that column the point sits.</param>
/// <param name="Row">The zero-based row.</param>
/// <param name="RowOffset">How far down that row it sits.</param>
public readonly record struct SheetCellPoint(
    int Column, Length ColumnOffset, int Row, Length RowOffset);

/// <summary>How a drawing is fastened to the sheet.</summary>
public enum SheetAnchorKind
{
    /// <summary>Both corners are cells: the drawing moves and resizes with them.</summary>
    TwoCell,

    /// <summary>The top left is a cell and the size is fixed: it moves but does not resize.</summary>
    OneCell,

    /// <summary>Neither corner is a cell: the drawing sits at a fixed place on the sheet.</summary>
    Absolute,
}

/// <summary>
/// One drawing anchored on a sheet: a picture, or a chart recorded but not drawn.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The image is carried encoded.</strong> <see cref="RasterImage.Encoded"/> holds the
/// bytes the file stored and nothing else; whichever backend needs pixels decodes them. That is
/// the layering rule rather than a convenience — a reader that decoded would put a codec in the
/// extraction path, which every caller pays for and almost none wants.
/// </para>
/// <para>
/// <strong>A chart is recorded and not drawn.</strong> Its series, axes and formatting live in
/// their own part and their own vocabulary, and reproducing a chart is a project of its own; but
/// dropping it would make "there is a chart here" indistinguishable from "there is nothing here".
/// So <see cref="IsChart"/> is set, <see cref="Image"/> is null, and nothing is painted — which is
/// what the presentations reader does with a <c>p:graphicFrame</c> it cannot draw.
/// </para>
/// </remarks>
public sealed record SheetDrawing
{
    /// <summary>How the drawing is fastened.</summary>
    public SheetAnchorKind Anchor { get; init; }

    /// <summary>Its top-left corner. Unused for <see cref="SheetAnchorKind.Absolute"/>.</summary>
    public SheetCellPoint From { get; init; }

    /// <summary>
    /// Its bottom-right corner, for a two-cell anchor.
    /// </summary>
    public SheetCellPoint To { get; init; }

    /// <summary>Its size, for a one-cell or absolute anchor.</summary>
    public DocSize Extent { get; init; }

    /// <summary>Its position on the sheet, for an absolute anchor.</summary>
    public DocPoint Position { get; init; }

    /// <summary>The picture, still encoded, or null when there is nothing to paint.</summary>
    public RasterImage? Image { get; init; }

    /// <summary>The shape's name, as the file records it.</summary>
    public string? Name { get; init; }

    /// <summary>Its alternative text or description, where the file records one.</summary>
    public string? Description { get; init; }

    /// <summary>True when the drawing is a chart, which is recorded rather than painted.</summary>
    public bool IsChart { get; init; }

    /// <summary>True when the drawing is hidden and therefore not printed.</summary>
    public bool IsHidden { get; init; }
}

/// <summary>
/// The drawings anchored on one sheet, in the order the file lists them.
/// </summary>
/// <remarks>
/// Order is z-order and is kept: a picture over a picture is decided by nothing else, and all
/// three formats state it the same way — the later shape is in front.
/// </remarks>
public sealed class SheetDrawings
{
    private readonly List<SheetDrawing> _items;

    /// <summary>Creates a sheet's drawings from the shapes read, in file order.</summary>
    /// <param name="items">The drawings.</param>
    public SheetDrawings(IEnumerable<SheetDrawing> items)
    {
        ArgumentNullException.ThrowIfNull(items);
        _items = [.. items];
    }

    /// <summary>A sheet with nothing drawn on it.</summary>
    public static SheetDrawings Empty { get; } = new([]);

    /// <summary>The drawings, back to front.</summary>
    public IReadOnlyList<SheetDrawing> Items => _items;

    /// <summary>True when the sheet has none.</summary>
    public bool IsEmpty => _items.Count == 0;
}

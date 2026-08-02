using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector;

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
/// <strong>A chart carries a model rather than a picture.</strong> <see cref="IsChart"/> says a
/// frame holds one; <see cref="Chart"/> holds what it takes to draw, when the chart is of a kind
/// the layout engine draws. The two are separate because they answer different questions: a chart
/// of an undrawn kind still sets the flag, so "there is a chart here" stays distinguishable from
/// "there is nothing here" even where the picture is missing.
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

    /// <summary>
    /// The picture as a display list — an SVG, a WMF, an EMF or an EMF+ — or null when it is a raster.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One or the other, never both, for every source but a DrawingML <c>svgBlip</c>: that one names an
    /// SVG and a raster fallback on the same <c>a:blip</c>, and both are kept so an empty decode still
    /// leaves a picture on the sheet.
    /// </para>
    /// <para>
    /// Decoded when something draws it and not while the sheet is read. That is the same rule as
    /// <see cref="RasterImage.Encoded"/>'s and for a sharper reason: the first metafile decode in a
    /// process costs about a second of font resolution, which a caller asking only for cell values must
    /// not pay.
    /// </para>
    /// </remarks>
    public Lazy<VectorImage>? Vector { get; init; }

    /// <summary>The shape's name, as the file records it.</summary>
    public string? Name { get; init; }

    /// <summary>Its alternative text or description, where the file records one.</summary>
    public string? Description { get; init; }

    /// <summary>True when the drawing is a chart, whether or not it can be drawn.</summary>
    public bool IsChart { get; init; }

    /// <summary>
    /// The chart's model, ready to lay out, or null when the frame holds no drawable chart.
    /// </summary>
    /// <remarks>
    /// Read in the same pass as the anchor, because the anchor is what gives the chart a rectangle
    /// and there is no second walk of the drawing part on the rendering path. Null both for a frame
    /// that is not a chart at all and for a chart whose type the engine does not draw — a doughnut,
    /// say — which is why <see cref="IsChart"/> is not simply this being non-null.
    /// </remarks>
    public ChartPlot? Chart { get; init; }

    /// <summary>The text inside the shape, or null when it holds none.</summary>
    /// <remarks>
    /// A text box is a shape carrying nothing but this, and it is the only content on the sheet
    /// that no walk of the cells can find — see <see cref="SheetShapeText"/>.
    /// </remarks>
    public SheetShapeText? Text { get; init; }

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

using System.Xml.Linq;
using Paperless.Core.Graphics;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Which of a table style's parts a table has switched on — DrawingML's <c>a:tblPr</c> flags.
/// </summary>
/// <remarks>
/// They are not decoration. A style declares a <c>a:firstRow</c> and a <c>a:band1H</c> whatever
/// the table does with them, and it is these six flags that decide whether a header row is drawn
/// in accent 1 with white text or as an ordinary banded row. A reader that applies every part it
/// finds paints a header on tables that have none.
/// </remarks>
/// <param name="FirstRow">Give row 0 the <c>a:firstRow</c> part.</param>
/// <param name="LastRow">Give the final row the <c>a:lastRow</c> part.</param>
/// <param name="FirstColumn">Give column 0 the <c>a:firstCol</c> part.</param>
/// <param name="LastColumn">Give the final column the <c>a:lastCol</c> part.</param>
/// <param name="BandRow">Alternate <c>a:band1H</c> and <c>a:band2H</c> down the body rows.</param>
/// <param name="BandColumn">Alternate <c>a:band1V</c> and <c>a:band2V</c> across the columns.</param>
public readonly record struct DrawingTableStyleOptions(
    bool FirstRow,
    bool LastRow,
    bool FirstColumn,
    bool LastColumn,
    bool BandRow,
    bool BandColumn)
{
    /// <summary>Reads an <c>a:tblPr</c>'s flags; all false when it states none.</summary>
    /// <param name="tablePropertes">The <c>a:tblPr</c>, or null.</param>
    public static DrawingTableStyleOptions Read(XElement? tablePropertes) => new(
        Drawing.Flag(tablePropertes, "firstRow") ?? false,
        Drawing.Flag(tablePropertes, "lastRow") ?? false,
        Drawing.Flag(tablePropertes, "firstCol") ?? false,
        Drawing.Flag(tablePropertes, "lastCol") ?? false,
        Drawing.Flag(tablePropertes, "bandRow") ?? false,
        Drawing.Flag(tablePropertes, "bandCol") ?? false);
}

/// <summary>What a table style says about one cell, before the cell's own <c>a:tcPr</c>.</summary>
/// <remarks>
/// Every member is nullable and null means "the style says nothing", which is what lets the
/// cell's own properties override exactly the parts it states. The six edges are kept apart
/// rather than resolved to four here because which of <c>insideH</c> and <c>top</c> wins depends
/// on where in the grid the cell sits.
/// </remarks>
public sealed record DrawingTableCellStyle
{
    /// <summary>The cell's background.</summary>
    public Colour? Fill { get; init; }

    /// <summary>Its left edge.</summary>
    public DrawingTableEdge? Left { get; init; }

    /// <summary>Its right edge.</summary>
    public DrawingTableEdge? Right { get; init; }

    /// <summary>Its top edge.</summary>
    public DrawingTableEdge? Top { get; init; }

    /// <summary>Its bottom edge.</summary>
    public DrawingTableEdge? Bottom { get; init; }

    /// <summary>The edge between this cell and the one below it.</summary>
    public DrawingTableEdge? InsideHorizontal { get; init; }

    /// <summary>The edge between this cell and the one beside it.</summary>
    public DrawingTableEdge? InsideVertical { get; init; }

    /// <summary>The colour the cell's text is drawn in.</summary>
    public Colour? TextColour { get; init; }

    /// <summary>Whether the cell's text is bold.</summary>
    public bool? Bold { get; init; }

    /// <summary>Whether the cell's text is italic.</summary>
    public bool? Italic { get; init; }

    /// <summary>True when the style said nothing at all about this cell.</summary>
    public bool IsEmpty
        => Fill is null && Left is null && Right is null && Top is null && Bottom is null
           && InsideHorizontal is null && InsideVertical is null
           && TextColour is null && Bold is null && Italic is null;
}

/// <summary>
/// A DrawingML table style — one <c>a:tblStyle</c> of the <c>tableStyles.xml</c> part — resolved
/// per cell.
/// </summary>
/// <remarks>
/// <para>
/// <strong>This is where a PowerPoint-authored table's appearance actually lives.</strong> Such a
/// table states <c>&lt;a:tblPr firstRow="1" bandRow="1"&gt;</c> and a style GUID, gives every
/// cell an empty <c>&lt;a:tcPr/&gt;</c>, and puts every fill and every border in the style part.
/// A reader that honours only <c>a:tcPr</c> draws the text and nothing else: no header band, no
/// banding, no grid — which on a dark header row means white text on white paper.
/// </para>
/// <para>
/// LibreOffice-authored tables are the opposite and are why this went unnoticed. Its PPTX export
/// writes an explicit <c>a:lnL</c>…<c>a:lnB</c> and an explicit fill on every cell, so the style
/// never decides anything and a table round-tripped through LibreOffice looks right without it.
/// </para>
/// <para>
/// The application order is <c>TableCell::pushToXCell</c>'s, at
/// <c>oox/source/drawingml/table/tablecell.cxx:300-500</c>, and it is an order rather than a
/// precedence table: each part is merged over what the previous parts left, so a later one wins.
/// Whole table, then first row, last row, first column, last column, then the horizontal band,
/// then the four corner cells, then the vertical band — and the cell's own <c>a:tcPr</c> over all
/// of it.
/// </para>
/// <para>
/// Two details in that order are easy to get wrong and both are visible. The banding index counts
/// the header row: <c>nBand = nRow; if (firstRow) nBand++</c>, so with a header present the first
/// body row is band 2 and not band 1. And <c>a:wholeTbl</c>'s <c>left</c>, <c>right</c>,
/// <c>top</c> and <c>bottom</c> are the <em>table's</em> outer frame rather than every cell's
/// four sides — applied only at the grid's edges, with <c>insideH</c> and <c>insideV</c> carrying
/// the interior.
/// </para>
/// </remarks>
public sealed class DrawingTableStyle
{
    private readonly Dictionary<string, XElement> _parts = new(StringComparer.Ordinal);

    private DrawingTableStyle()
    {
    }

    /// <summary>
    /// Reads the style a table names out of an <c>a:tblStyleLst</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A table that states no <c>a:tableStyleId</c> gets <strong>no style at all</strong>, and the
    /// part's <c>def</c> attribute is not consulted. It reads like the default table look a deck
    /// sets for itself, and PowerPoint does apply it; LibreOffice does not.
    /// <c>TableProperties::getUsedTableStyle</c>
    /// (<c>oox/source/drawingml/table/tableproperties.cxx:89-124</c>) searches the list only when
    /// the id is non-empty and otherwise returns a <c>static TableStyle</c> that has nothing in
    /// it — no <c>wholeTbl</c>, no banding, no first column.
    /// </para>
    /// <para>
    /// Measured rather than read off, because the source alone would not settle which renderer to
    /// follow. <c>slides/batch-011/pptx/section_1_our_rights_presentation.pptx</c> page 8 holds a
    /// three-column table with <c>firstRow="1" firstCol="1" bandRow="1"</c> and no
    /// <c>a:tableStyleId</c>; the reference leaves its first column white and we filled it
    /// <c>accent1</c> with white text on it. Putting the id of the very style the package declares
    /// as <c>def</c> into that <c>a:tblPr</c> and re-rendering makes the reference draw exactly
    /// what we drew — an orange first column and a tinted body — so the fallback is the whole of
    /// the difference.
    /// </para>
    /// <para>
    /// A style id that resolves to nothing likewise yields null rather than the default, because
    /// a table naming a style the package does not carry has no style, not the wrong one.
    /// </para>
    /// </remarks>
    /// <param name="tableStyles">The <c>a:tblStyleLst</c> root, or null.</param>
    /// <param name="styleId">The table's <c>a:tblPr/a:tableStyleId</c>, or null.</param>
    public static DrawingTableStyle? Read(XElement? tableStyles, string? styleId)
    {
        if (tableStyles is null) return null;
        if (string.IsNullOrEmpty(styleId)) return null;

        string wanted = styleId;

        foreach (XElement style in Drawing.Children(tableStyles, "tblStyle"))
        {
            if (!string.Equals(style.Attribute("styleId")?.Value, wanted, StringComparison.Ordinal))
                continue;

            DrawingTableStyle read = new();
            foreach (XElement part in style.Elements())
            {
                if (part.Name.NamespaceName == OoxmlNamespaces.DrawingML)
                    read._parts[part.Name.LocalName] = part;
            }

            return read._parts.Count == 0 ? null : read;
        }

        return null;
    }

    /// <summary>
    /// What the style says about the cell at a grid position.
    /// </summary>
    /// <param name="options">The table's <c>a:tblPr</c> flags.</param>
    /// <param name="row">The cell's row, counted from zero.</param>
    /// <param name="lastRow">The index of the final row.</param>
    /// <param name="column">The cell's column, counted from zero.</param>
    /// <param name="lastColumn">The index of the final column.</param>
    /// <param name="theme">The theme its scheme colours resolve against.</param>
    /// <param name="matrix">
    /// The theme's format matrix, for the parts that state an <c>a:fillRef</c> instead of a fill.
    /// </param>
    public DrawingTableCellStyle Resolve(
        DrawingTableStyleOptions options,
        int row,
        int lastRow,
        int column,
        int lastColumn,
        DrawingTheme? theme,
        DrawingStyleMatrix? matrix)
    {
        DrawingTableCellStyle resolved = new();

        resolved = ApplyWholeTable(resolved, row, lastRow, column, lastColumn, theme, matrix);

        if (options.FirstRow && row == 0) resolved = Apply(resolved, "firstRow", theme, matrix);
        if (options.LastRow && row == lastRow) resolved = Apply(resolved, "lastRow", theme, matrix);
        if (options.FirstColumn && column == 0) resolved = Apply(resolved, "firstCol", theme, matrix);
        if (options.LastColumn && column == lastColumn)
            resolved = Apply(resolved, "lastCol", theme, matrix);

        if (options.BandRow && InBody(options, row, lastRow, column, lastColumn))
        {
            // The header row counts as a band, so the first body row of a table with one is the
            // *second* band. Off by one here inverts the shading of every banded table.
            int band = options.FirstRow ? row + 1 : row;
            resolved = Apply(resolved, (band & 1) != 0 ? "band2H" : "band1H", theme, matrix);
        }

        if (row == 0 && column == 0) resolved = Apply(resolved, "nwCell", theme, matrix);
        if (row == lastRow && column == 0) resolved = Apply(resolved, "swCell", theme, matrix);
        if (row == 0 && column == lastColumn) resolved = Apply(resolved, "neCell", theme, matrix);
        if (row == lastRow && column == lastColumn)
            resolved = Apply(resolved, "seCell", theme, matrix);

        if (options.BandColumn && InBody(options, row, lastRow, column, lastColumn))
        {
            int band = options.FirstColumn ? column + 1 : column;
            resolved = Apply(resolved, (band & 1) != 0 ? "band2V" : "band1V", theme, matrix);
        }

        return resolved;
    }

    /// <summary>True when a cell is inside the banded body rather than in a header or a spine.</summary>
    private static bool InBody(
        DrawingTableStyleOptions options, int row, int lastRow, int column, int lastColumn)
        => (!options.FirstRow || row != 0)
           && (!options.LastRow || row != lastRow)
           && (!options.FirstColumn || column != 0)
           && (!options.LastColumn || column != lastColumn);

    /// <summary>
    /// <c>a:wholeTbl</c>, whose four sides are the table's frame rather than each cell's.
    /// </summary>
    private DrawingTableCellStyle ApplyWholeTable(
        DrawingTableCellStyle into,
        int row,
        int lastRow,
        int column,
        int lastColumn,
        DrawingTheme? theme,
        DrawingStyleMatrix? matrix)
    {
        if (!_parts.TryGetValue("wholeTbl", out XElement? part)) return into;

        XElement? cellStyle = Drawing.Child(part, "tcStyle");
        XElement? borders = Drawing.Child(cellStyle, "tcBdr");

        return Text(into, part, theme) with
        {
            Fill = FillOf(cellStyle, theme, matrix) ?? into.Fill,
            Left = column == 0 ? Edge(borders, "left", theme) ?? into.Left : into.Left,
            Right = column >= lastColumn ? Edge(borders, "right", theme) ?? into.Right : into.Right,
            Top = row == 0 ? Edge(borders, "top", theme) ?? into.Top : into.Top,
            Bottom = row >= lastRow ? Edge(borders, "bottom", theme) ?? into.Bottom : into.Bottom,
            InsideHorizontal = Edge(borders, "insideH", theme) ?? into.InsideHorizontal,
            InsideVertical = Edge(borders, "insideV", theme) ?? into.InsideVertical,
        };
    }

    /// <summary>Any other part, whose four sides really are the cell's own.</summary>
    private DrawingTableCellStyle Apply(
        DrawingTableCellStyle into, string name, DrawingTheme? theme, DrawingStyleMatrix? matrix)
    {
        if (!_parts.TryGetValue(name, out XElement? part)) return into;

        XElement? cellStyle = Drawing.Child(part, "tcStyle");
        XElement? borders = Drawing.Child(cellStyle, "tcBdr");

        return Text(into, part, theme) with
        {
            Fill = FillOf(cellStyle, theme, matrix) ?? into.Fill,
            Left = Edge(borders, "left", theme) ?? into.Left,
            Right = Edge(borders, "right", theme) ?? into.Right,
            Top = Edge(borders, "top", theme) ?? into.Top,
            Bottom = Edge(borders, "bottom", theme) ?? into.Bottom,
            InsideHorizontal = Edge(borders, "insideH", theme) ?? into.InsideHorizontal,
            InsideVertical = Edge(borders, "insideV", theme) ?? into.InsideVertical,
        };
    }

    /// <summary>
    /// A part's <c>a:tcTxStyle</c>: the colour, weight and slope its cells' text takes.
    /// </summary>
    /// <remarks>
    /// The colour is the element's own child rather than anything under the <c>a:fontRef</c> —
    /// the font reference beside it names a typeface index and carries a colour of its own that
    /// LibreOffice ignores here (<c>tablecell.cxx:230-234</c> reads <c>getTextColor()</c>, which
    /// <c>tablestyletextstylecontext.cxx</c> fills from the direct child).
    /// </remarks>
    private static DrawingTableCellStyle Text(
        DrawingTableCellStyle into, XElement part, DrawingTheme? theme)
    {
        XElement? style = Drawing.Child(part, "tcTxStyle");
        if (style is null) return into;

        Colour? colour = null;
        foreach (XElement child in style.Elements())
        {
            if (Drawing.Is(child, "fontRef")) continue;
            if (DrawingColour.Read(child)?.Resolve(theme) is { } resolved) { colour = resolved; break; }
        }

        return into with
        {
            TextColour = colour ?? into.TextColour,
            Bold = OnOff(Drawing.Attribute(style, "b")) ?? into.Bold,
            Italic = OnOff(Drawing.Attribute(style, "i")) ?? into.Italic,
        };
    }

    /// <summary>
    /// <c>ST_OnOffStyleType</c>, which is <c>on</c>/<c>off</c>/<c>def</c> and not a boolean.
    /// </summary>
    private static bool? OnOff(string? value) => value switch
    {
        "on" or "1" or "true" => true,
        "off" or "0" or "false" => false,
        _ => null,
    };

    /// <summary>A part's <c>a:fill</c>, or the theme fill its <c>a:fillRef</c> names.</summary>
    private static Colour? FillOf(
        XElement? cellStyle, DrawingTheme? theme, DrawingStyleMatrix? matrix)
    {
        if (Drawing.Child(cellStyle, "fill") is { } fill)
        {
            if (Drawing.Child(fill, "noFill") is not null) return null;
            return Solid(Drawing.Child(fill, "solidFill"), theme);
        }

        // A style part may name one of the theme's three fill styles instead of stating a fill,
        // which is the same reference a shape's p:style carries and resolves the same way.
        if (matrix is null || Drawing.Child(cellStyle, "fillRef") is null) return null;

        XElement wrapper = new(Drawing.Name("style"), new XElement(Drawing.Child(cellStyle, "fillRef")!));
        return Solid(Drawing.Child(matrix.Fill(wrapper, theme), "solidFill"), theme);
    }

    private static Colour? Solid(XElement? solidFill, DrawingTheme? theme)
    {
        if (solidFill is null) return null;

        foreach (XElement child in solidFill.Elements())
        {
            if (DrawingColour.Read(child)?.Resolve(theme) is { } colour) return colour;
        }

        return null;
    }

    /// <summary>
    /// One named side of a part's <c>a:tcBdr</c>, or null when it states none.
    /// </summary>
    /// <remarks>
    /// A style's border is an <c>a:ln</c> <em>inside</em> a named side — <c>a:top/a:ln</c> —
    /// where a cell's own is the side element itself, <c>a:lnT</c>. Reading the style's side as
    /// though it were a cell's finds no width and no fill and draws nothing.
    /// </remarks>
    private static DrawingTableEdge? Edge(XElement? borders, string side, DrawingTheme? theme)
        => DrawingTableGeometry.Edge(Drawing.Child(Drawing.Child(borders, side), "ln"), theme);
}

using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// A DrawingML table read for <em>drawing</em>: its grid, its cells' boxes, and their edges.
/// </summary>
/// <remarks>
/// <para>
/// The companion to <see cref="DrawingTable"/>, which reads the same <c>a:tbl</c> for extraction
/// and keeps only the text. Two readings rather than one for the reason every family here has
/// two: a caller indexing a deck wants neither a column width nor a border colour, and making the
/// content tree carry them would charge it for a feature it never uses.
/// </para>
/// <para>
/// What is <em>not</em> resolved here is the table style (<c>a:tblPr/@firstRow</c> and the
/// <c>tableStyles.xml</c> part it bands against). A cell's own <c>a:tcPr</c> is authoritative when
/// it states something, and every table LibreOffice writes states everything on every cell — its
/// PPTX export writes an explicit <c>a:lnL</c>…<c>a:lnB</c> and an explicit fill per cell, even
/// when the answer is <c>a:noFill</c>. So the style matters only for PowerPoint-authored decks,
/// and is recorded in the TODO rather than guessed at.
/// </para>
/// </remarks>
public static class DrawingTableGeometry
{
    /// <summary>DrawingML's default cell margins: 91440 EMU across, 45720 EMU down.</summary>
    /// <remarks>
    /// <c>oox/source/drawingml/table/tablecellcontext.cxx:67-70</c>, and they are the cell's
    /// margins rather than the text body's insets — a cell's <c>a:bodyPr</c> is ignored, which
    /// is measurable: <c>deck-features.pptx</c>'s table states <c>lIns="0" tIns="0"</c> on every
    /// cell body and LibreOffice still draws the first column's text 7.2 pt in from the table's
    /// left edge.
    /// </remarks>
    public static Margins DefaultMargins { get; } = new(
        Length.FromEmu(91440), Length.FromEmu(45720),
        Length.FromEmu(91440), Length.FromEmu(45720));

    /// <summary>
    /// The width a border of a stated EMU width is actually drawn at.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Not the stated width, and the difference is 15%.</strong> A cell border travels
    /// EMU → 1/100 mm → a <c>BorderLine2.LineWidth</c> that is <em>halved</em>
    /// (<c>oox/source/drawingml/table/tablecell.cxx:99-101</c>, all three widths set from
    /// <c>GetCoordinate(w) / 4</c> or <c>/ 2</c>) → an <c>SvxBorderLine</c> that the table's view
    /// contact then rescales as though it were in <em>twips</em>
    /// (<c>svx/source/table/viewcontactoftableobj.cxx:176-180</c>, <c>fTwipsToMM</c>). The two
    /// conversions do not cancel: a <c>w="12700"</c> — one point — comes out of LibreOffice's PDF
    /// as a pen of <strong>0.85009 pt</strong>, measured on a two-column table whose every cell
    /// states exactly that.
    /// </para>
    /// <para>
    /// Reproduced rather than corrected, because the reference is what a comparison is against;
    /// the arithmetic is integer at both steps, which is where the missing 15% comes from.
    /// </para>
    /// </remarks>
    public static Length BorderWidth(long emu)
    {
        // GetCoordinate: EMU to 1/100 mm, rounding as oox does (drawingmltypes.hxx).
        long hundredthsOfMillimetre = (emu + 180) / 360;

        // BorderLine2.LineWidth is half of that, in integer arithmetic.
        long half = hundredthsOfMillimetre / 2;

        // And then scaled as if it were twips: one twip is 635 EMU.
        return Length.FromEmu(half * Length.EmuPerTwip);
    }

    /// <summary>Reads a table's geometry.</summary>
    /// <param name="table">The <c>a:tbl</c> element.</param>
    /// <param name="theme">The theme, for scheme colours on fills and borders.</param>
    public static DrawingTableBox Read(XElement table, DrawingTheme? theme = null)
    {
        ArgumentNullException.ThrowIfNull(table);

        List<Length> columns = [];
        foreach (XElement column in Drawing.Children(Drawing.Child(table, "tblGrid"), "gridCol"))
        {
            columns.Add(Length.FromEmu(Emu(column, "w", 0)));
        }

        List<Length> rows = [];
        List<DrawingTableCellBox> cells = [];

        int rowIndex = 0;
        foreach (XElement row in Drawing.Children(table, "tr"))
        {
            rows.Add(Length.FromEmu(Emu(row, "h", 0)));

            int column = 0;
            foreach (XElement cell in Drawing.Children(row, "tc"))
            {
                int position = column++;

                // A covered cell is written out carrying hMerge/vMerge rather than omitted, so the
                // grid position is the ordinal and not a running total of the spans — the same
                // rule extraction follows, and getting it wrong shifts every cell after a merge
                // one column right.
                bool covered = Drawing.Flag(cell, "hMerge") == true
                               || Drawing.Flag(cell, "vMerge") == true;

                XElement? properties = Drawing.Child(cell, "tcPr");

                cells.Add(new DrawingTableCellBox
                {
                    Row = rowIndex,
                    Column = position,
                    RowSpan = Math.Max(1, Drawing.Number(cell, "rowSpan") ?? 1),
                    ColumnSpan = Math.Max(1, Drawing.Number(cell, "gridSpan") ?? 1),
                    IsCovered = covered,
                    Margins = MarginsOf(properties),
                    Anchor = Drawing.Attribute(properties, "anchor"),
                    Fill = FillOf(properties, theme),
                    TextBody = Drawing.Child(cell, "txBody"),
                    Left = Edge(Drawing.Child(properties, "lnL"), theme),
                    Right = Edge(Drawing.Child(properties, "lnR"), theme),
                    Top = Edge(Drawing.Child(properties, "lnT"), theme),
                    Bottom = Edge(Drawing.Child(properties, "lnB"), theme),
                });
            }

            rowIndex++;
        }

        return new DrawingTableBox
        {
            ColumnWidths = columns,
            RowHeights = rows,
            Cells = cells,
        };
    }

    private static Margins MarginsOf(XElement? properties) => new(
        Length.FromEmu(Emu(properties, "marL", 91440)),
        Length.FromEmu(Emu(properties, "marT", 45720)),
        Length.FromEmu(Emu(properties, "marR", 91440)),
        Length.FromEmu(Emu(properties, "marB", 45720)));

    private static Colour? FillOf(XElement? properties, DrawingTheme? theme)
    {
        XElement? solid = Drawing.Child(properties, "solidFill");
        if (solid is null) return null;

        foreach (XElement child in solid.Elements())
        {
            if (DrawingColour.Read(child)?.Resolve(theme) is { } colour) return colour;
        }

        return null;
    }

    /// <summary>
    /// One of a cell's four edges, or null when it draws nothing.
    /// </summary>
    /// <remarks>
    /// An <c>a:noFill</c> and an absent element are both null here, which is the same answer
    /// LibreOffice arrives at by two routes: an empty border never displaces a real one at a grid
    /// position and never draws anything itself
    /// (<c>svx/source/table/tablelayouter.cxx:944-948</c>, <c>HasPriority</c>). A line stating no
    /// width at all is one point, which is the default <c>tablecell.cxx:99</c> substitutes.
    /// </remarks>
    private static DrawingTableEdge? Edge(XElement? line, DrawingTheme? theme)
    {
        if (line is null) return null;
        if (Drawing.Child(line, "noFill") is not null) return null;

        XElement? solid = Drawing.Child(line, "solidFill");
        if (solid is null) return null;

        Colour? colour = null;
        foreach (XElement child in solid.Elements())
        {
            if (DrawingColour.Read(child)?.Resolve(theme) is { } resolved)
            {
                colour = resolved;
                break;
            }
        }

        if (colour is null) return null;

        return new DrawingTableEdge(
            BorderWidth(Emu(line, "w", 12700)),
            colour.Value,
            Drawing.Attribute(Drawing.Child(line, "prstDash"), "val"));
    }

    private static long Emu(XElement? element, string attribute, long whenAbsent)
        => long.TryParse(
            Drawing.Attribute(element, attribute), NumberStyles.Integer,
            CultureInfo.InvariantCulture, out long value)
            ? value
            : whenAbsent;
}

/// <summary>A table's grid and its cells, in the table's own coordinates.</summary>
/// <remarks>
/// The row heights are what the file <em>states</em>, which is a minimum rather than the answer:
/// LibreOffice grows a row to its tallest cell's content
/// (<c>svx/source/table/tablelayouter.cxx:1026-1029</c>), so the final height needs the text
/// measured and belongs with whoever can measure it.
/// </remarks>
public sealed record DrawingTableBox
{
    /// <summary>The <c>a:gridCol</c> widths, left to right.</summary>
    public IReadOnlyList<Length> ColumnWidths { get; init; } = [];

    /// <summary>The <c>a:tr/@h</c> heights, top to bottom, each a minimum.</summary>
    public IReadOnlyList<Length> RowHeights { get; init; } = [];

    /// <summary>Every cell, including the ones a span covers, in row-major order.</summary>
    public IReadOnlyList<DrawingTableCellBox> Cells { get; init; } = [];
}

/// <summary>One cell: where it sits in the grid, how it is painted, and what it holds.</summary>
public sealed record DrawingTableCellBox
{
    /// <summary>Its row, counted from zero.</summary>
    public required int Row { get; init; }

    /// <summary>Its grid column, counted from zero — its ordinal in the row.</summary>
    public required int Column { get; init; }

    /// <summary>How many rows it spans.</summary>
    public int RowSpan { get; init; } = 1;

    /// <summary>How many columns it spans.</summary>
    public int ColumnSpan { get; init; } = 1;

    /// <summary>True when it is covered by another cell's span and draws nothing itself.</summary>
    public bool IsCovered { get; init; }

    /// <summary>The distance from each edge to its text.</summary>
    public Margins Margins { get; init; } = DrawingTableGeometry.DefaultMargins;

    /// <summary>Its <c>a:tcPr/@anchor</c>, or null for the default.</summary>
    public string? Anchor { get; init; }

    /// <summary>Its background, or null when it states none.</summary>
    public Colour? Fill { get; init; }

    /// <summary>
    /// The element its text lives in, left unread so that each family reads it its own way.
    /// </summary>
    /// <remarks>
    /// An <c>a:txBody</c> from PresentationML, and the <c>table:table-cell</c> itself from ODF,
    /// which has no wrapper — the two vocabularies agree on nothing below this point, which is
    /// exactly why the reading is a delegate rather than a method here.
    /// </remarks>
    public XElement? TextBody { get; init; }

    /// <summary>Its left edge, or null when it draws none.</summary>
    public DrawingTableEdge? Left { get; init; }

    /// <summary>Its right edge.</summary>
    public DrawingTableEdge? Right { get; init; }

    /// <summary>Its top edge.</summary>
    public DrawingTableEdge? Top { get; init; }

    /// <summary>Its bottom edge.</summary>
    public DrawingTableEdge? Bottom { get; init; }
}

/// <summary>One cell edge's pen.</summary>
/// <param name="Width">How wide it is drawn, already through the conversion LibreOffice applies.</param>
/// <param name="Colour">Its colour.</param>
/// <param name="PresetDash">Its <c>a:prstDash/@val</c>, or null for a solid line.</param>
public readonly record struct DrawingTableEdge(Length Width, Colour Colour, string? PresetDash = null);

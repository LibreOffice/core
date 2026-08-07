using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Ooxml.DrawingML;

namespace Paperless.Presentations.OpenDocument;

/// <summary>
/// Reads a <c>table:table</c> inside a <c>draw:frame</c> into the grid model slide layout takes.
/// </summary>
/// <remarks>
/// <para>
/// The ODF counterpart of <see cref="DrawingTableGeometry"/>, and deliberately only that: it
/// produces the same <see cref="DrawingTableBox"/>, so <c>SlideTable.Place</c> lays an ODF table
/// out through the identical code path a PresentationML one goes through. LibreOffice arrives at
/// the same place by a longer route — both filters build an <c>SdrTableObj</c> and only its view
/// contact decides what is drawn (<c>svx/source/table/viewcontactoftableobj.cxx</c>) — so a
/// second layouter here would be a second chance to disagree with the reference.
/// </para>
/// <para>
/// <strong>Almost nothing about the two vocabularies lines up, which is why this exists.</strong>
/// A column's width is not on the column but in a <c>table-column</c> style; a covered cell is a
/// <c>table:covered-table-cell</c> element rather than a real cell carrying a merge flag; a run of
/// identical columns, rows or cells is written once with a <c>table:number-*-repeated</c> count;
/// and the rows may be wrapped in <c>table:table-header-rows</c>. Only the resulting geometry is
/// shared.
/// </para>
/// <para>
/// <strong>A draw table's cell properties are in three different property sets, and the borders
/// are in the one nobody would guess.</strong> The fill, the padding and the vertical alignment
/// arrive as <em>graphic</em> properties (LibreOffice writes them in a <c>loext:graphic-properties</c>
/// child, because a cell fills like a shape), and the four borders arrive as <em>paragraph</em>
/// properties — <c>XMLTableImport</c> chains <c>CreateParaExtPropMapper</c> onto the cell mapper
/// (<c>xmloff/source/table/XMLTableImport.cxx:256-258</c>) and that map states
/// <c>fo:border-left</c> and its three siblings against <c>style:paragraph-properties</c>
/// (<c>xmloff/source/text/txtprmap.cxx:427-431</c>). A reader looking in
/// <c>style:table-cell-properties</c>, which is where the specification's own cell map puts them
/// and where Writer and Calc do put them, finds an unbordered table on every deck LibreOffice
/// has ever written.
/// </para>
/// </remarks>
internal static class OdfTableGeometry
{
    /// <summary>
    /// The width a border stated as an <c>fo:border</c> length is actually drawn at.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Half the conversion the OOXML path suffers, and the same 15% error.</strong>
    /// <see cref="DrawingTableGeometry.BorderWidth"/> halves the stated width because
    /// <c>oox/source/drawingml/table/tablecell.cxx:99-101</c> does; ODF has no such step, because
    /// LibreOffice's exporter already wrote the halved number out —
    /// <c>slide-table-grid.pptx</c>'s <c>w="12700"</c> comes back from
    /// <c>soffice --convert-to odp</c> as <c>fo:border-left="0.48pt"</c>, which is 17/100 mm and
    /// exactly half of the 35 the point converts to.
    /// </para>
    /// <para>
    /// What both paths share is the second conversion: the table's view contact rescales an
    /// <c>SvxBorderLine</c> as though its 1/100 mm width were in <em>twips</em>
    /// (<c>svx/source/table/viewcontactoftableobj.cxx:176-180</c>). So 17 becomes 17 twips —
    /// 0.85009 pt — and the two front ends agree on the drawn pen to the last decimal, which is
    /// the point of doing it this way round rather than correcting either.
    /// </para>
    /// <para>
    /// The rounding into 1/100 mm is LibreOffice's own: <c>lcl_convertMeasure</c> adds a half and
    /// truncates (<c>sax/source/tools/converter.cxx:359-368</c>), so 0.48 pt is 16.93 and becomes
    /// 17 rather than 16.
    /// </para>
    /// </remarks>
    public static Length BorderWidth(Length stated)
    {
        long hundredthsOfMillimetre = (long)Math.Floor((stated.Emu / 360.0) + 0.5);
        return Length.FromEmu(hundredthsOfMillimetre * Length.EmuPerTwip);
    }

    /// <summary>Reads a table's geometry.</summary>
    /// <param name="file">The document, for its styles.</param>
    /// <param name="table">The <c>table:table</c> element.</param>
    public static DrawingTableBox Read(OdfFile file, XElement table)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(table);

        List<Length> columns = [];
        foreach (XElement column in Descend(table, "table-column"))
        {
            Length width = Property(
                file, column, OdfStyleFamily.TableColumn, OdfPropertyKind.TableColumn,
                OdfNamespaces.Style, "column-width").AsLength() ?? Length.Zero;

            for (int i = 0; i < Repeat(column, "number-columns-repeated"); i++) columns.Add(width);
        }

        List<Length> rows = [];
        List<DrawingTableCellBox> cells = [];

        foreach (XElement row in Descend(table, "table-row"))
        {
            Length height = Property(
                file, row, OdfStyleFamily.TableRow, OdfPropertyKind.TableRow,
                OdfNamespaces.Style, "row-height").AsLength() ?? Length.Zero;

            for (int repeat = 0; repeat < Repeat(row, "number-rows-repeated"); repeat++)
            {
                int index = rows.Count;
                rows.Add(height);

                int column = 0;
                foreach (XElement cell in row.Elements())
                {
                    if (cell.Name.NamespaceName != OdfNamespaces.Table) continue;

                    bool covered = cell.Name.LocalName == "covered-table-cell";
                    if (!covered && cell.Name.LocalName != "table-cell") continue;

                    for (int i = 0; i < Repeat(cell, "number-columns-repeated"); i++)
                    {
                        cells.Add(Cell(file, cell, index, column++, covered));
                    }
                }
            }
        }

        return new DrawingTableBox
        {
            ColumnWidths = columns,
            RowHeights = rows,
            Cells = cells,
        };
    }

    /// <summary>
    /// The rows or columns of a table, however they are grouped.
    /// </summary>
    /// <remarks>
    /// ODF lets a table wrap its rows in <c>table:table-header-rows</c> or <c>table:table-rows</c>
    /// and its columns in <c>table:table-columns</c>, and a table:table-row-group may nest. A
    /// reader taking only the direct children of <c>table:table</c> loses every row of a table
    /// with a heading, which is most tables a word processor writes and some a deck does.
    /// </remarks>
    private static IEnumerable<XElement> Descend(XElement parent, string localName)
    {
        foreach (XElement child in parent.Elements())
        {
            if (child.Name.NamespaceName != OdfNamespaces.Table) continue;

            if (child.Name.LocalName == localName)
            {
                yield return child;
                continue;
            }

            switch (child.Name.LocalName)
            {
                case "table-header-rows":
                case "table-rows":
                case "table-row-group":
                case "table-header-columns":
                case "table-columns":
                case "table-column-group":
                    foreach (XElement nested in Descend(child, localName)) yield return nested;
                    break;

                default:
                    break;
            }
        }
    }

    private static DrawingTableCellBox Cell(
        OdfFile file, XElement cell, int row, int column, bool covered)
    {
        OdfStyleReference style = new(
            cell.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value,
            OdfStyleFamily.TableCell);

        return new DrawingTableCellBox
        {
            Row = row,
            Column = column,
            RowSpan = Math.Max(1, Count(cell, "number-rows-spanned")),
            ColumnSpan = Math.Max(1, Count(cell, "number-columns-spanned")),
            IsCovered = covered,
            Margins = Margins(file, style),
            Anchor = Graphic(file, style, OdfNamespaces.Draw, "textarea-vertical-align").Value
                     ?? Cellwise(file, style, OdfNamespaces.Style, "vertical-align").Value,
            Fill = Fill(file, style) is { } background ? Paint.Solid(background) : null,
            TextBody = cell,
            Left = Edge(file, style, "left"),
            Right = Edge(file, style, "right"),
            Top = Edge(file, style, "top"),
            Bottom = Edge(file, style, "bottom"),
        };
    }

    /// <summary>
    /// A cell's text margins, which ODF spells as padding on the cell style.
    /// </summary>
    /// <remarks>
    /// Zero when the style states none, unlike DrawingML's 91440/45720 EMU defaults: ODF has no
    /// implied value, and every draw table LibreOffice writes states all four —
    /// <c>fo:padding-left="0.254cm"</c> is the 91440 EMU a converted deck came in with.
    /// </remarks>
    private static Margins Margins(OdfFile file, OdfStyleReference style) => new(
        Padding(file, style, "padding-left"),
        Padding(file, style, "padding-top"),
        Padding(file, style, "padding-right"),
        Padding(file, style, "padding-bottom"));

    private static Length Padding(OdfFile file, OdfStyleReference style, string name)
        => (Graphic(file, style, OdfNamespaces.FoCompatible, name).AsLength()
            ?? Cellwise(file, style, OdfNamespaces.FoCompatible, name).AsLength()
            ?? Graphic(file, style, OdfNamespaces.FoCompatible, "padding").AsLength()
            ?? Cellwise(file, style, OdfNamespaces.FoCompatible, "padding").AsLength())
           ?? Length.Zero;

    /// <summary>
    /// A cell's background.
    /// </summary>
    /// <remarks>
    /// Two spellings, because LibreOffice writes the newer one and the specification has the
    /// older: <c>draw:fill</c>/<c>draw:fill-color</c> in the graphic set, and
    /// <c>fo:background-color</c> in the cell set. An explicit <c>draw:fill="none"</c> means no
    /// fill and must not fall through to the background colour, or a cell the file deliberately
    /// left clear comes out painted.
    /// </remarks>
    private static Colour? Fill(OdfFile file, OdfStyleReference style)
    {
        OdfProperty fill = Graphic(file, style, OdfNamespaces.Draw, "fill");
        if (fill.HasValue)
        {
            return fill.Is("solid")
                ? Graphic(file, style, OdfNamespaces.Draw, "fill-color").AsColour()
                : null;
        }

        Colour? background =
            Cellwise(file, style, OdfNamespaces.FoCompatible, "background-color").AsColour();

        return background == Colour.Transparent ? null : background;
    }

    /// <summary>
    /// One of a cell's four edges, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// The <c>fo:border</c> shorthand is consulted only when the side-specific property says
    /// nothing, which is the order <c>XMLPropertySetMapper</c> resolves them in: the shorthand is
    /// the same property entry under a different token, so a style stating both ends with the
    /// later one and LibreOffice writes the sides.
    /// </remarks>
    private static DrawingTableEdge? Edge(OdfFile file, OdfStyleReference style, string side)
    {
        string? value = Border(file, style, $"border-{side}").Value ?? Border(file, style, "border").Value;
        return Pen(value);
    }

    private static OdfProperty Border(OdfFile file, OdfStyleReference style, string name)
        => file.Styles.ResolveProperty(
            style.Name, style.Family, OdfPropertyKind.Paragraph, OdfNamespaces.FoCompatible, name);

    /// <summary>
    /// Parses an <c>fo:border</c> value — a width, a style and a colour, in any order.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The tokens are identified by what they parse as rather than by position, which is what
    /// <c>XMLBorderHdl::importXML</c> does (<c>xmloff/source/style/bordrhdl.cxx:193-223</c>): it
    /// tries each token as a named width, then as a style, then as a colour, then as a length.
    /// Real files write "0.48pt solid #808080" and the specification permits "solid 0.05cm".
    /// </para>
    /// <para>
    /// A style of <c>none</c> or <c>hidden</c> draws nothing and returns null, which is the same
    /// answer an absent property gives — and the same one an empty edge gives at a grid position,
    /// where it never displaces a real one
    /// (<c>svx/source/table/tablelayouter.cxx:944-948</c>).
    /// </para>
    /// </remarks>
    private static DrawingTableEdge? Pen(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return null;

        Length? width = null;
        Colour? colour = null;
        string? line = null;

        foreach (string token in value.Split(
                     [' ', '\t', '\n', '\r'], StringSplitOptions.RemoveEmptyEntries))
        {
            if (colour is null && OdfValue.ParseColour(token) is { } read)
            {
                colour = read;
                continue;
            }

            if (width is null && Named(token) is { } named)
            {
                width = named;
                continue;
            }

            if (width is null && OdfValue.ParseLength(token) is { } measured)
            {
                width = measured;
                continue;
            }

            line ??= token;
        }

        if (line is "none" or "hidden") return null;
        if (line is null) return null;
        if (width is not { } stated || stated <= Length.Zero) return null;

        return new DrawingTableEdge(BorderWidth(stated), colour ?? Colour.Black);
    }

    /// <summary>The three named border widths ODF defines, in the sizes LibreOffice gives them.</summary>
    /// <remarks><c>xmloff/source/style/bordrhdl.cxx</c>'s <c>aBorderWidths</c>, in 1/100 mm.</remarks>
    private static Length? Named(string token) => token switch
    {
        "thin" => Length.FromMm100(1),
        "medium" => Length.FromMm100(88),
        "thick" => Length.FromMm100(175),
        _ => null,
    };

    private static OdfProperty Graphic(
        OdfFile file, OdfStyleReference style, string ns, string name)
        => file.Styles.ResolveProperty(
            style.Name, style.Family, OdfPropertyKind.Graphic, ns, name);

    private static OdfProperty Cellwise(
        OdfFile file, OdfStyleReference style, string ns, string name)
        => file.Styles.ResolveProperty(
            style.Name, style.Family, OdfPropertyKind.TableCell, ns, name);

    private static OdfProperty Property(
        OdfFile file,
        XElement element,
        OdfStyleFamily family,
        OdfPropertyKind kind,
        string ns,
        string name)
        => file.Styles.ResolveProperty(
            element.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value,
            family, kind, ns, name);

    /// <summary>
    /// A <c>table:number-*-repeated</c> count, clamped.
    /// </summary>
    /// <remarks>
    /// The attribute exists so that a spreadsheet can say "and 1048576 more like this one" in
    /// four characters, and a deck's table inherits the vocabulary without ever needing the
    /// range. Clamped so that a hostile or a spreadsheet-shaped value cannot make a slide
    /// allocate a million cells.
    /// </remarks>
    private static int Repeat(XElement element, string name)
        => Math.Clamp(Count(element, name), 1, 1024);

    private static int Count(XElement element, string name)
        => OdfValue.ParseInt(element.Attribute(XName.Get(name, OdfNamespaces.Table))?.Value) ?? 1;
}

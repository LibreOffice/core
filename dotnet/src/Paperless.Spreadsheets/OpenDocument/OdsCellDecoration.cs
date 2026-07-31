using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.OpenDocument;

/// <summary>
/// Reads a sheet's cell fills and borders, and its header and footer text, out of ODF.
/// </summary>
/// <remarks>
/// <para>
/// Its own pass over the sheet element rather than a hook in the extraction walk, and
/// deliberately so: extraction discards formatting on purpose, and threading a second output
/// through it would make every cell of every sheet pay for something only a rendering wants.
/// The walk here reads attributes and never touches a cell's content, so a sheet whose cells
/// name no style costs one attribute lookup per element.
/// </para>
/// <para>
/// ODF is the format that states this most directly, because Calc wrote it: a cell's
/// <c>table:style-name</c> names a <c>style:style</c> whose <c>style:table-cell-properties</c>
/// carry <c>fo:background-color</c> and the four <c>fo:border-*</c> attributes, each written as
/// the CSS shorthand "width style colour".
/// </para>
/// </remarks>
internal static class OdsCellDecoration
{
    /// <summary>
    /// How far one repeated cell or row is expanded before it is treated as padding.
    /// </summary>
    /// <remarks>
    /// ODF pads a sheet to its full width and height with repeat counts, so a
    /// <c>table:table-cell</c> carrying <c>table:number-columns-repeated="16384"</c> is
    /// ordinary and a row repeated a million times is too. Nearly all of them name the default
    /// style and intern to nothing, so this never fires on a file Calc wrote; it exists so that
    /// one that pads with a <em>styled</em> cell cannot materialise sixteen billion entries.
    /// </remarks>
    private const int MaxRepeat = 4096;

    private static readonly XName TableColumn = XName.Get("table-column", OdfNamespaces.Table);
    private static readonly XName TableRow = XName.Get("table-row", OdfNamespaces.Table);
    private static readonly XName TableCell = XName.Get("table-cell", OdfNamespaces.Table);
    private static readonly XName CoveredCell = XName.Get("covered-table-cell", OdfNamespaces.Table);
    private static readonly XNamespace TableNamespace = OdfNamespaces.Table;

    /// <summary>Walks a <c>table:table</c> and records what each cell paints.</summary>
    /// <param name="styles">The document's styles.</param>
    /// <param name="table">The sheet's element.</param>
    public static SheetFormatting Read(OdfStyles styles, XElement table)
    {
        ArgumentNullException.ThrowIfNull(styles);
        ArgumentNullException.ThrowIfNull(table);

        SheetFormatting formatting = new();
        Dictionary<string, int> handles = new(StringComparer.Ordinal);

        int column = 0;
        int row = 0;
        Walk(table);

        return formatting;

        void Walk(XElement parent)
        {
            foreach (XElement child in parent.Elements())
            {
                if (child.Name == TableColumn)
                {
                    int first = column;
                    column += Math.Max(1, Repeated(child, "number-columns-repeated"));

                    // A column's own style holds its width; the cells it formats are named by
                    // table:default-cell-style-name, which is a different attribute and a
                    // different family. Confusing the two gives every cell the column's width
                    // properties and no fill at all.
                    int format = Handle(Attribute(child, "default-cell-style-name"));
                    if (format > 0) formatting.SetColumns(first, column - 1, format);
                }
                else if (child.Name == TableRow)
                {
                    ReadRow(child);
                }
                else if (child.Name.Namespace == TableNamespace)
                {
                    // table:table-header-rows and the row and column groups wrap without
                    // interrupting the numbering, so they are descended into rather than
                    // counted — the same rule the print-setup walk follows.
                    Walk(child);
                }
            }
        }

        void ReadRow(XElement element)
        {
            int first = row;
            int repeat = Math.Max(1, Repeated(element, "number-rows-repeated"));
            row += repeat;

            int lastRow = Math.Min(row, first + MaxRepeat);

            int rowFormat = Handle(Attribute(element, "default-cell-style-name"));
            if (rowFormat >= 0)
            {
                for (int at = first; at < lastRow; at++) formatting.SetRow(at, rowFormat);
            }

            int column2 = 0;
            foreach (XElement cell in element.Elements())
            {
                if (cell.Name != TableCell && cell.Name != CoveredCell) continue;

                int span = Math.Max(1, Repeated(cell, "number-columns-repeated"));
                int format = Handle(Attribute(cell, "style-name"));

                // Zero is applied and only absence is skipped: a cell naming a style that paints
                // nothing has to cancel its column's fill, which is exactly what Calc writes as
                // table:style-name="Default".
                if (format >= 0)
                {
                    for (int offset = 0; offset < Math.Min(span, MaxRepeat); offset++)
                    {
                        for (int line = first; line < lastRow; line++)
                            formatting.SetCell(line, column2 + offset, format);
                    }
                }

                column2 += span;
            }
        }

        // Negative for "no style named at all", zero for "named one that paints nothing" — the
        // two are different answers and the second has to override a column's fill.
        int Handle(string? styleName)
        {
            if (string.IsNullOrEmpty(styleName)) return -1;
            if (handles.TryGetValue(styleName, out int cached)) return cached;

            SheetCellFormat format = Describe(styles, styleName);
            int handle = format.IsNone ? 0 : formatting.Intern(format);
            handles[styleName] = handle;
            return handle;
        }
    }

    /// <summary>Reads the header and footer of the master page a sheet prints under.</summary>
    /// <param name="header">The <c>style:header</c> element, or null.</param>
    public static SheetHeaderFooter? ReadBand(XElement? header)
    {
        if (header is null) return null;

        SheetHeaderPart left = Region(header, "region-left");
        SheetHeaderPart centre = Region(header, "region-center");
        SheetHeaderPart right = Region(header, "region-right");

        // A band with no regions at all but with a bare text:p is Calc's own shorthand for
        // "everything is centred", which is what its exporter writes for a header holding only
        // the sheet name.
        if (left.IsEmpty && centre.IsEmpty && right.IsEmpty)
        {
            SheetHeaderPart whole = Paragraphs(header);
            return whole.IsEmpty ? null : new SheetHeaderFooter(
                SheetHeaderPart.Empty, whole, SheetHeaderPart.Empty);
        }

        return new SheetHeaderFooter(left, centre, right);
    }

    private static SheetHeaderPart Region(XElement band, string localName)
    {
        XElement? region = band.Element(XName.Get(localName, OdfNamespaces.Style));
        return region is null ? SheetHeaderPart.Empty : Paragraphs(region);
    }

    /// <summary>
    /// Flattens a region's paragraphs into one run of segments.
    /// </summary>
    /// <remarks>
    /// Flattened because a header region is drawn as one piece of text with one alignment, and
    /// because the fields are the point: <c>text:page-number</c> and its siblings each carry a
    /// <em>cached</em> value as their text content, which is what the authoring application last
    /// showed and is exactly what must not be printed — page 1 of a document whose page 1 was
    /// deleted still says "1" in the file.
    /// </remarks>
    private static SheetHeaderPart Paragraphs(XElement region)
    {
        List<SheetHeaderSegment> segments = [];

        foreach (XElement paragraph in region.Elements(XName.Get("p", OdfNamespaces.Text)))
        {
            if (segments.Count > 0) segments.Add(SheetHeaderSegment.Literal("\n"));
            Collect(paragraph, segments);
        }

        return segments.Count == 0 ? SheetHeaderPart.Empty : new SheetHeaderPart(segments);
    }

    private static void Collect(XElement element, List<SheetHeaderSegment> segments)
    {
        foreach (XNode node in element.Nodes())
        {
            switch (node)
            {
                case XText text:
                    segments.Add(SheetHeaderSegment.Literal(text.Value));
                    break;

                case XElement child when Field(child) is { } field:
                    segments.Add(SheetHeaderSegment.Of(field));
                    break;

                case XElement child when child.Name == XName.Get("s", OdfNamespaces.Text):
                {
                    int count = int.TryParse(
                        child.Attribute(XName.Get("c", OdfNamespaces.Text))?.Value,
                        NumberStyles.Integer, CultureInfo.InvariantCulture, out int stated)
                        ? stated
                        : 1;
                    segments.Add(SheetHeaderSegment.Literal(new string(' ', Math.Clamp(count, 0, 256))));
                    break;
                }

                case XElement child when child.Name == XName.Get("tab", OdfNamespaces.Text):
                    segments.Add(SheetHeaderSegment.Literal("\t"));
                    break;

                // A text:span carries formatting and nothing else here, so its children are
                // taken and it is not.
                case XElement child:
                    Collect(child, segments);
                    break;
            }
        }
    }

    private static SheetHeaderField? Field(XElement element)
    {
        if (element.Name.NamespaceName != OdfNamespaces.Text) return null;

        return element.Name.LocalName switch
        {
            "page-number" => SheetHeaderField.PageNumber,
            "page-count" => SheetHeaderField.PageCount,
            "date" => SheetHeaderField.Date,
            "time" => SheetHeaderField.Time,
            "sheet-name" => SheetHeaderField.SheetName,
            "title" => SheetHeaderField.Title,
            "file-name" => string.Equals(
                element.Attribute(XName.Get("display", OdfNamespaces.Text))?.Value, "path",
                StringComparison.Ordinal)
                ? SheetHeaderField.FilePath
                : SheetHeaderField.FileName,
            _ => null,
        };
    }

    /// <summary>What one cell style paints.</summary>
    private static SheetCellFormat Describe(OdfStyles styles, string styleName)
    {
        Colour? background = null;

        // "transparent" is a value and not an absence: a cell style may say it explicitly to
        // cancel a fill it would otherwise inherit, so an unparseable colour must not become a
        // black one.
        string? fill = Property(styles, styleName, OdfNamespaces.FoCompatible, "background-color");
        if (fill is not null && !string.Equals(fill.Trim(), "transparent", StringComparison.Ordinal))
            background = OdfValue.ParseColour(fill);

        string? all = Property(styles, styleName, OdfNamespaces.FoCompatible, "border");

        SheetBorder Edge(string side, string widthProperty)
        {
            string? stated = Property(styles, styleName, OdfNamespaces.FoCompatible, "border-" + side)
                             ?? all;
            SheetBorder border = ParseBorder(stated);
            if (border.IsNone || !border.IsDouble) return border;

            // style:border-line-width-* carries the three widths of a double rule, which
            // fo:border cannot state: it gives only the total. Absent, the total is split
            // evenly, which is what editeng's own guess does.
            string? widths = Property(styles, styleName, OdfNamespaces.Style, widthProperty)
                             ?? Property(styles, styleName, OdfNamespaces.Style, "border-line-width");

            return ApplyLineWidths(border, widths);
        }

        SheetCellBorders borders = new(
            Edge("left", "border-line-width-left"),
            Edge("right", "border-line-width-right"),
            Edge("top", "border-line-width-top"),
            Edge("bottom", "border-line-width-bottom"));

        return background is null && borders.IsNone
            ? SheetCellFormat.None
            : new SheetCellFormat(background, borders);
    }

    private static string? Property(
        OdfStyles styles, string styleName, string ns, string localName)
    {
        OdfProperty found = styles.ResolveProperty(
            styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell, ns, localName);

        return found.HasValue ? found.Value : null;
    }

    /// <summary>
    /// Reads the <c>fo:border</c> shorthand: a width, a line style and a colour.
    /// </summary>
    /// <remarks>
    /// In any order and with any of the three missing, which is what CSS says and what real
    /// files do. "none" and a zero width both mean no border; anything else is a line, because
    /// a colour alone still draws — LibreOffice defaults an unstated width to a hairline rather
    /// than to nothing.
    /// </remarks>
    private static SheetBorder ParseBorder(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return SheetBorder.None;

        Length? width = null;
        Colour colour = Colour.Black;
        SheetBorderPattern pattern = SheetBorderPattern.Solid;
        bool doubled = false;
        bool none = false;

        foreach (string word in value.Split(
                     [' ', '\t'], StringSplitOptions.RemoveEmptyEntries))
        {
            switch (word.ToLowerInvariant())
            {
                case "none" or "hidden": none = true; break;
                case "solid": pattern = SheetBorderPattern.Solid; break;
                case "dotted": pattern = SheetBorderPattern.Dotted; break;
                case "dashed": pattern = SheetBorderPattern.Dashed; break;
                case "fine-dashed": pattern = SheetBorderPattern.FineDashed; break;
                case "dash-dot": pattern = SheetBorderPattern.DashDot; break;
                case "dash-dot-dot": pattern = SheetBorderPattern.DashDotDot; break;
                case "double" or "double-thin": doubled = true; break;
                case "groove" or "ridge" or "inset" or "outset": break;
                default:
                    if (OdfValue.ParseColour(word) is { } parsed) colour = parsed;
                    else if (OdfValue.ParseLength(word) is { } measured) width = measured;
                    break;
            }
        }

        if (none) return SheetBorder.None;

        // Snapped to whole twips, which is the unit LibreOffice keeps a border width in
        // (SvxBorderLine's width is twips) and the reason ODF states such odd numbers: a
        // 2.5 pt border round-trips through 1/100 mm and comes back as "2.49pt". Taking that
        // literally strokes at 2.49 pt where LibreOffice strokes at 2.49983.
        if (width is { } stated && stated <= Length.Zero) return SheetBorder.None;

        Length total = width is { } given
            ? Length.FromTwips(Math.Max(1, given.Twips))
            : Length.FromTwips(1);

        if (!doubled) return SheetBorder.Line(total, colour, pattern);

        Length line = total / 3;
        return new SheetBorder(line, total - line - line, line, colour, pattern);
    }

    /// <summary>Applies <c>style:border-line-width</c>, which is inner, gap, outer.</summary>
    private static SheetBorder ApplyLineWidths(SheetBorder border, string? widths)
    {
        if (string.IsNullOrWhiteSpace(widths)) return border;

        Length[] parts =
        [
            .. widths.Split([' '], StringSplitOptions.RemoveEmptyEntries)
                .Select(OdfValue.ParseLength)
                .Where(length => length is not null)
                .Select(length => length!.Value),
        ];

        return parts.Length == 3
            ? border with { Primary = parts[2], Distance = parts[1], Secondary = parts[0] }
            : border;
    }

    private static string? Attribute(XElement element, string localName)
        => element.Attribute(XName.Get(localName, OdfNamespaces.Table))?.Value;

    private static int Repeated(XElement element, string localName)
        => int.TryParse(
            Attribute(element, localName), NumberStyles.Integer, CultureInfo.InvariantCulture,
            out int parsed) && parsed > 0
            ? parsed
            : 1;
}

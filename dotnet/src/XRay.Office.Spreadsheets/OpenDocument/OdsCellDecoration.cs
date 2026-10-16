using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.OpenDocument;

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
    /// How many rows of a repeat state a row default of their own.
    /// </summary>
    /// <remarks>
    /// ODF pads a sheet to its full width and height with repeat counts, so a
    /// <c>table:table-cell</c> carrying <c>table:number-columns-repeated="16384"</c> is
    /// ordinary and a row repeated a million times is too. The cells of such a repeat are kept
    /// as one rectangle and cost nothing per row; a row <em>default</em> is one entry per row,
    /// so that half is still bounded.
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
            long first = row;
            int repeat = Math.Max(1, Repeated(element, "number-rows-repeated"));

            // Clamped to the sheet's own last row, which is what `ScXMLTableRowContext` does
            // with the same attribute (`sc/source/filter/xml/xmlrowi.cxx`:73-82). A repeat that
            // runs past the sheet's bounds is honoured up to them, not ignored.
            long lastRow = Math.Min(first + repeat - 1, SheetAddress.MaxRow);
            row = (int)Math.Min(first + repeat, SheetAddress.MaxRow + 1L);

            if (first > lastRow) return;

            int rowFormat = Handle(Attribute(element, "default-cell-style-name"));
            if (rowFormat >= 0)
            {
                long last = Math.Min(lastRow, first + MaxRepeat - 1);
                for (long at = first; at <= last; at++) formatting.SetRow((int)at, rowFormat);
            }

            long column2 = 0;
            foreach (XElement cell in element.Elements())
            {
                if (cell.Name != TableCell && cell.Name != CoveredCell) continue;
                if (column2 > SheetAddress.MaxColumn) break;

                int span = Math.Max(1, Repeated(cell, "number-columns-repeated"));
                long lastColumn = Math.Min(column2 + span - 1, SheetAddress.MaxColumn);
                int format = Handle(Attribute(cell, "style-name"));

                // Zero is applied and only absence is skipped: a cell naming a style that paints
                // nothing has to cancel its column's fill, which is exactly what Calc writes as
                // table:style-name="Default".
                //
                // The rectangle is recorded as a rectangle. Calc records the same one as a single
                // `ScRange` (`sc/source/filter/xml/xmlcelli.cxx`:1368-1377) and stores it as a run
                // of an `ScAttrArray`, so a sheet padded to its full extent costs one entry per
                // repeat rather than one per cell.
                if (format >= 0)
                {
                    if (first == lastRow && column2 == lastColumn)
                        formatting.SetCell((int)first, (int)column2, format);
                    else
                        formatting.SetCells(
                            (int)first, (int)lastRow, (int)column2, (int)lastColumn, format);
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

            SheetCellDecoration format = Describe(styles, styleName);
            int handle = format.IsNone ? 0 : formatting.Intern(format);
            handles[styleName] = handle;
            return handle;
        }
    }

    /// <summary>Reads the header and footer of the master page a sheet prints under.</summary>
    /// <remarks>
    /// <para>
    /// <strong>A <c>text:span</c>'s formatting is kept.</strong> It used to be dropped — the
    /// comment inside <c>Collect</c> read <em>"a text:span carries formatting and nothing else
    /// here, so its children are taken and it is not"</em> — which made every ODF band one size
    /// and one face, the workbook's default, whatever the file said. That is wrong twice: it
    /// draws the band in the wrong face, and it sizes it in the wrong one, because
    /// <c>ScPrintFunc::UpdateHFHeight</c> measures the band's real text
    /// (<c>sc/source/ui/view/printfun.cxx:817-836</c>). Measured against 26.2.4.2 over 30
    /// authored probes crossing five faces with six sizes: the reference's band grows with the
    /// header's own size at every size above the default's and ours did not move at all — 18.1 pt
    /// short at Carlito 24.
    /// </para>
    /// <para>
    /// The three properties a band's height and drawing turn on are the size, the family and the
    /// two slopes; everything else a span can state is colour and decoration, which
    /// <see cref="SheetHeaderSegment"/> does not model for any of the three formats.
    /// </para>
    /// </remarks>
    /// <param name="styles">The document's styles, for the spans' own text styles.</param>
    /// <param name="header">The <c>style:header</c> element, or null.</param>
    public static SheetHeaderFooter? ReadBand(OdfStyles styles, XElement? header)
    {
        ArgumentNullException.ThrowIfNull(styles);
        if (header is null) return null;

        SheetHeaderPart left = Region(styles, header, "region-left");
        SheetHeaderPart centre = Region(styles, header, "region-center");
        SheetHeaderPart right = Region(styles, header, "region-right");

        // A band with no regions at all but with a bare text:p is Calc's own shorthand for
        // "everything is centred", which is what its exporter writes for a header holding only
        // the sheet name.
        if (left.IsEmpty && centre.IsEmpty && right.IsEmpty)
        {
            SheetHeaderPart whole = Paragraphs(styles, header);
            return whole.IsEmpty ? null : new SheetHeaderFooter(
                SheetHeaderPart.Empty, whole, SheetHeaderPart.Empty);
        }

        return new SheetHeaderFooter(left, centre, right);
    }

    private static SheetHeaderPart Region(OdfStyles styles, XElement band, string localName)
    {
        XElement? region = band.Element(XName.Get(localName, OdfNamespaces.Style));
        return region is null ? SheetHeaderPart.Empty : Paragraphs(styles, region);
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
    private static SheetHeaderPart Paragraphs(OdfStyles styles, XElement region)
    {
        List<SheetHeaderSegment> segments = [];

        foreach (XElement paragraph in region.Elements(XName.Get("p", OdfNamespaces.Text)))
        {
            if (segments.Count > 0) segments.Add(SheetHeaderSegment.Literal("\n"));

            // A paragraph's own text style applies to everything on it that no span overrides,
            // exactly as a span's does to its children.
            Collect(
                styles, paragraph, segments,
                Run.None.Over(styles, TextStyleName(paragraph)));
        }

        return segments.Count == 0 ? SheetHeaderPart.Empty : new SheetHeaderPart(segments);
    }

    /// <summary>What a band's run is formatted with: the three properties its height turns on.</summary>
    /// <param name="Size">The em size the run states, or null to take the workbook's default.</param>
    /// <param name="Family">The family it states, or null for the default.</param>
    /// <param name="Bold">Whether it is bold, or null for the default.</param>
    /// <param name="Italic">Whether it is italic, or null for the default.</param>
    private readonly record struct Run(
        Length? Size, string? Family, bool? Bold, bool? Italic)
    {
        /// <summary>A run stating nothing, which takes the workbook's default cell font.</summary>
        public static Run None => default;

        /// <summary>
        /// This run with whatever a text style states written over it.
        /// </summary>
        /// <remarks>
        /// A nested <c>text:span</c> inherits its parent's run and overrides what its own style
        /// sets, which is what makes this a fold rather than a lookup. The family is resolved
        /// through both of its spellings at once — <c>style:font-name</c> and
        /// <c>fo:font-family</c> are one item, so an ancestor style stating the second must not
        /// beat this style stating the first (<c>XMLTextImportPropertyMapper::handleSpecialItem</c>,
        /// <c>xmloff/source/text/txtimppr.cxx:58-101</c>).
        /// </remarks>
        public Run Over(OdfStyles styles, string? styleName)
        {
            if (string.IsNullOrEmpty(styleName)) return this;

            OdfProperty face = styles.ResolveWithoutDefaults(
                styleName, OdfStyleFamily.Text, OdfPropertyKind.Text, FontSpellings,
                out int matched);

            string? family = Family;
            if (face.HasValue)
            {
                family = matched == 1 && face.Value is { } named
                         && styles.FontFaces.TryGetValue(named, out OdfFontFace? declared)
                    ? declared.FontFamily ?? named
                    : face.Value;
            }

            string? weight = Text(styles, styleName, OdfNamespaces.FoCompatible, "font-weight");
            string? slope = Text(styles, styleName, OdfNamespaces.FoCompatible, "font-style");

            return new Run(
                OdfValue.ParseLength(
                    Text(styles, styleName, OdfNamespaces.FoCompatible, "font-size")) ?? Size,
                family,
                weight is null ? Bold : Weight(weight) >= 600,
                slope is null ? Italic : slope is "italic" or "oblique");
        }

        private static string? Text(OdfStyles styles, string styleName, string ns, string name)
        {
            OdfProperty found = styles.ResolveWithoutDefaults(
                styleName, OdfStyleFamily.Text, OdfPropertyKind.Text, ns, name);

            return found.HasValue ? found.Value : null;
        }

        /// <summary>An ODF weight, which is a keyword or a hundreds number.</summary>
        private static int Weight(string value)
            => int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int n)
                ? n
                : value switch
                {
                    "bold" => 700,
                    "normal" => 400,
                    _ => 400,
                };

        /// <summary>The two spellings of a face, in the order one style's own attributes settle.</summary>
        private static readonly (string Namespace, string Name)[] FontSpellings =
        [
            (OdfNamespaces.FoCompatible, "font-family"),
            (OdfNamespaces.Style, "font-name"),
        ];
    }

    private static string? TextStyleName(XElement element)
        => element.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;

    private static void Collect(
        OdfStyles styles, XElement element, List<SheetHeaderSegment> segments, Run run)
    {
        foreach (XNode node in element.Nodes())
        {
            switch (node)
            {
                case XText text:
                    segments.Add(SheetHeaderSegment.Literal(
                        text.Value, run.Size, run.Family, run.Bold, run.Italic));
                    break;

                case XElement child when Field(child) is { } field:
                    segments.Add(SheetHeaderSegment.Of(
                        field, run.Size, run.Family, run.Bold, run.Italic));
                    break;

                case XElement child when child.Name == XName.Get("s", OdfNamespaces.Text):
                {
                    int count = int.TryParse(
                        child.Attribute(XName.Get("c", OdfNamespaces.Text))?.Value,
                        NumberStyles.Integer, CultureInfo.InvariantCulture, out int stated)
                        ? stated
                        : 1;
                    segments.Add(SheetHeaderSegment.Literal(
                        new string(' ', Math.Clamp(count, 0, 256)),
                        run.Size, run.Family, run.Bold, run.Italic));
                    break;
                }

                case XElement child when child.Name == XName.Get("tab", OdfNamespaces.Text):
                    segments.Add(SheetHeaderSegment.Literal(
                        "\t", run.Size, run.Family, run.Bold, run.Italic));
                    break;

                // A text:span states formatting and holds the text it applies to, so its children
                // are taken and its own style is folded over the run they are taken with.
                case XElement child:
                    Collect(styles, child, segments, run.Over(styles, TextStyleName(child)));
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
    private static SheetCellDecoration Describe(OdfStyles styles, string styleName)
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
            ? SheetCellDecoration.None
            : new SheetCellDecoration(background, borders);
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

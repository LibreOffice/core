using System.Globalization;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Spreadsheets.OpenDocument;

/// <summary>
/// Which cell format each cell of one ODF sheet is drawn in.
/// </summary>
/// <remarks>
/// <para>
/// A second walk over the table element, matching what <c>OdsSpreadsheetDocument.ReadSheets</c>
/// already does for the print setup and cheap for the same reason: the document is parsed into an
/// element tree, so this reads attributes off elements that are in memory. It also reuses the
/// existing <see cref="OdfStyles"/> resolver, which already walks a style's parent chain and its
/// family defaults — the cascade a cell style needs is exactly the one a paragraph style needs.
/// </para>
/// <para>
/// ODF's own two surprises. Alignment is not a single attribute: <c>style:text-align-source</c>
/// on the <em>cell</em> chooses between the paragraph's <c>fo:text-align</c> and the value's own
/// type, so a cell whose source is <c>value-type</c> is Calc's General alignment however the
/// paragraph is aligned. And a repeated row or column carries a
/// <c>table:default-cell-style-name</c> that applies to every cell in it that states none, which
/// is how a formatted column costs one attribute rather than a million.
/// </para>
/// </remarks>
internal static class OdsCellFormats
{
    /// <summary>Reads one sheet's cell formats and its rich cells.</summary>
    /// <param name="file">The document, for its styles.</param>
    /// <param name="table">The <c>table:table</c> element.</param>
    public static (SheetCellFormats Formats, SheetRichText RichText) Read(OdfFile file, XElement table)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(table);

        Reader reader = new(file.Styles);
        return reader.Read(table);
    }

    /// <summary>
    /// The workbook's default cell font: the <c>Default</c> cell style's own family and size.
    /// </summary>
    /// <remarks>
    /// <para>
    /// It is what a header or footer naming no face of its own is set in, and what an
    /// <em>empty</em> header area is measured as — <c>ScPrintFunc::MakeEditEngine</c> fills the
    /// band's EditEngine defaults from <c>getDefaultCellAttribute</c>
    /// (<c>sc/source/ui/view/printfun.cxx:1765-1772</c>). The other three readers have passed
    /// this to <see cref="SheetBandHeight"/> since round 56; the ODF one had nowhere to pass it
    /// to, because it read the band's declared height and never measured its text.
    /// </para>
    /// <para>
    /// Resolved through the same <see cref="Reader"/> a cell goes through, rather than by reading
    /// the style's attributes here, so that the two spellings of a face — <c>style:font-name</c>
    /// and <c>fo:font-family</c>, one item — are settled by level before they are settled by
    /// spelling. A converted workbook states the first on its automatic styles and both on
    /// <c>Default</c>, so reading them independently answers the wrong one.
    /// </para>
    /// </remarks>
    /// <param name="styles">The document's styles.</param>
    public static SheetDefaultFont DefaultFont(OdfStyles styles)
    {
        ArgumentNullException.ThrowIfNull(styles);

        Reader reader = new(styles);
        return reader.DefaultFont();
    }

    /// <summary>
    /// What each named cell style resolves to, for the conditional formats that apply them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Resolution is the same walk a cell's own style takes, parent by parent, which is the whole
    /// point: an ODF conditional style is a cell <em>style sheet</em> and the reference reads its
    /// font through that chain rather than out of the style's own element. See
    /// <see cref="OdsConditionalText"/>.
    /// </para>
    /// <para>
    /// A name the document does not define resolves to the <c>Default</c> cell style's font,
    /// which is where the chain would have ended anyway.
    /// </para>
    /// </remarks>
    /// <param name="styles">The document's styles.</param>
    /// <param name="names">The style names to resolve.</param>
    public static Dictionary<string, SheetCellFormat> ResolveNamed(
        OdfStyles styles, IEnumerable<string> names)
    {
        ArgumentNullException.ThrowIfNull(styles);
        ArgumentNullException.ThrowIfNull(names);

        Reader reader = new(styles);
        Dictionary<string, SheetCellFormat> resolved = new(StringComparer.Ordinal);
        foreach (string name in names)
        {
            if (string.IsNullOrEmpty(name) || resolved.ContainsKey(name)) continue;
            resolved[name] = reader.ResolveNamed(name);
        }

        return resolved;
    }

    private sealed class Reader(OdfStyles styles)
    {
        private readonly Dictionary<string, SheetCellFormat> _resolved = new(StringComparer.Ordinal);
        private readonly SheetCellFormats.Builder _builder = new();
        private readonly SheetRichText.Builder _rich = new();

        // The pool the builder is filling, mirrored so that a rich cell can be told what its own
        // format resolved to. The builder answers "which index"; the spans need the format itself,
        // and a span's style is a delta over it.
        private readonly Dictionary<int, SheetCellFormat> _pool = new() { [0] = SheetCellFormat.Default };
        private readonly Dictionary<int, int> _columnDefaults = [];
        private int _rowDefault;

        /// <summary>The last column carrying a default cell style, or -1 when none does.</summary>
        /// <remarks>
        /// A repeated run starting past it cannot meet a column default, which is what lets the run
        /// be skipped whole rather than column by column. See <see cref="ReadCells"/>.
        /// </remarks>
        private int _lastColumnDefault = -1;

        /// <summary>The pool index of the document's own default cell style.</summary>
        private int _sheetDefault;

        public (SheetCellFormats Formats, SheetRichText RichText) Read(XElement table)
        {
            // The document's default cell style, resolved once and recorded once. In Calc it is the
            // sheet's base attribute set — `ScDocument::GetDefPattern`, which every cell has by
            // being in the sheet — and not something each cell carries. Reading it per cell instead
            // made every padding cell in a Calc-written file state a format: a 10 kB `.ods` whose
            // sheet is padded to 16382 columns by 1048571 rows took 2.25 GB and 20 seconds to open,
            // because the padding names `table:style-name="Default"` and that resolved to a format
            // unequal to `SheetCellFormat.Default`.
            _sheetDefault = Intern(DefaultCellStyleName);
            _builder.SetSheetDefault(_sheetDefault);

            ReadColumns(table);
            ReadRows(table);
            return (_builder.Build(), _rich.Build());
        }

        /// <summary>
        /// The index a cell inherits where it states no format of its own, in <see cref="SheetCellFormats.At"/>'s
        /// own order: the row's default, then the column's, then the sheet's.
        /// </summary>
        private int InheritedIndex(int column)
        {
            if (_rowDefault != 0) return _rowDefault;
            if (_columnDefaults.TryGetValue(column, out int columnDefault) && columnDefault != 0)
                return columnDefault;

            return _sheetDefault;
        }

        private void ReadColumns(XElement table)
        {
            int column = 0;
            foreach (XElement element in Descend(table, "table-column"))
            {
                int repeat = Repeat(element, "number-columns-repeated");

                // A column naming none names Default, which is Calc's own reading of the absence
                // (`xmlcoli.cxx`:135-137) rather than a licence taken here.
                int index = Intern(Attribute(element, "default-cell-style-name") ?? DefaultCellStyleName);

                // A column default that only restates the sheet's is not one: recording it would
                // make every column in a padded file an entry, and resolve to the same format.
                if (index == 0 || index == _sheetDefault)
                {
                    column += repeat;
                    continue;
                }

                for (int at = 0; at < repeat && column < SheetAddress.MaxColumn; at++, column++)
                {
                    _builder.SetColumn(column, index);
                    _columnDefaults[column] = index;
                    _lastColumnDefault = Math.Max(_lastColumnDefault, column);
                }
            }
        }

        private void ReadRows(XElement table)
        {
            long row = 0;
            foreach (XElement element in Descend(table, "table-row"))
            {
                int repeat = Repeat(element, "number-rows-repeated");
                int rowStyle = Intern(Attribute(element, "default-cell-style-name"));

                // Clamped to the sheet's own last row rather than dropped or ignored, which is
                // what `ScXMLTableRowContext` does with the same attribute:
                // `min(declared, GetSheetLimits().GetMaxRowCount())`
                // (`sc/source/filter/xml/xmlrowi.cxx`:73-82).
                long lastRow = Math.Min(row + repeat - 1, SheetAddress.MaxRow);

                if (row <= lastRow)
                {
                    _rowDefault = rowStyle;

                    // A row default is one entry per row, so a row repeated a million times would
                    // be a million of them. The cap is what it was before the run became a block.
                    if (rowStyle != 0)
                    {
                        long last = Math.Min(lastRow, row + MaxRepeat - 1);
                        for (long at = row; at <= last; at++) _builder.SetRow((int)at, rowStyle);
                    }

                    // Read once for the whole run: re-reading the element per repeated row is
                    // what made a padded sheet cost rows times columns.
                    ReadCells(element, (int)row, (int)lastRow);
                }

                row = Math.Min(row + repeat, SheetAddress.MaxRow + 1L);
            }
        }

        private void ReadCells(XElement rowElement, int firstRow, int lastRow)
        {
            long column = 0;
            foreach (XElement cell in rowElement.Elements())
            {
                if (cell.Name.NamespaceName != OdfNamespaces.Table) continue;
                if (cell.Name.LocalName is not ("table-cell" or "covered-table-cell")) continue;
                if (column > SheetAddress.MaxColumn) break;

                string? styleName = Attribute(cell, "style-name");
                int repeat = Repeat(cell, "number-columns-repeated");
                int index = Intern(styleName);

                // The same clamp on the other axis: `min(GetMaxColCount(), max(declared, 1))`,
                // `sc/source/filter/xml/xmlcelli.cxx`:186-195.
                long lastColumn = Math.Min(column + repeat - 1, SheetAddress.MaxColumn);

                // Padding, skipped whole. A row is written out to the sheet's full width with one
                // repeated empty cell naming the default style, and expanding that run records
                // nothing that resolving the cell would not have answered anyway — Calc's own
                // attribute array merges an equal pattern into the surrounding run rather than
                // starting a new one (`ScAttrArray::SetPatternArea`). The run has to be past the
                // last column default for the inherited index to be the same across all of it, and
                // an element with children can carry text whose spans still need reading.
                if (!cell.HasElements
                    && column > _lastColumnDefault
                    && index == (_rowDefault != 0 ? _rowDefault : _sheetDefault))
                {
                    column += repeat;
                    continue;
                }

                // Inside the columns a column default reaches, what a cell inherits differs from
                // one column to the next, so the run is cut into the stretches that agree; past
                // the last of them it is the same for all of them and the rest is one block.
                long singly = Math.Min(lastColumn, _lastColumnDefault);
                for (long at = column; at <= singly;)
                {
                    bool differs = index != InheritedIndex((int)at);
                    long end = at;
                    while (end < singly && (index != InheritedIndex((int)end + 1)) == differs) end++;
                    if (differs) Record(firstRow, lastRow, (int)at, (int)end, index);
                    at = end + 1;
                }

                long block = Math.Max(column, _lastColumnDefault + 1L);
                if (block <= lastColumn && index != (_rowDefault != 0 ? _rowDefault : _sheetDefault))
                    Record(firstRow, lastRow, (int)block, (int)lastColumn, index);

                // Rich text is per cell and cannot be a block — a portion is an offset into one
                // cell's own text. A repeated cell carrying spans is not something Calc writes, so
                // the expansion is bounded by a budget over the whole rectangle rather than
                // modelled. The test is hoisted out of the loops because it walks the cell's
                // descendants and the loops can run to the budget.
                if (cell.HasElements && HasSpan(cell))
                {
                    long budget = MaxRepeat;
                    for (long line = firstRow; line <= lastRow && budget > 0; line++)
                    {
                        for (long at = column; at <= lastColumn && budget > 0; at++, budget--)
                            ReadSpans(cell, (int)line, (int)at, Effective(index, (int)at));
                    }
                }

                column += repeat;
            }

            void Record(int first, int last, int firstCol, int lastCol, int format)
            {
                if (first == last && firstCol == lastCol) _builder.SetCell(first, firstCol, format);
                else _builder.SetCells(first, last, firstCol, lastCol, format);
            }
        }

        // ------------------------------------------------------------------------- rich text

        /// <summary>
        /// Reads a cell whose text carries <c>text:span</c>s, as portions of its flattened text.
        /// </summary>
        /// <remarks>
        /// <para>
        /// ODF is the only one of the three formats where a cell's rich text is <em>also</em> the
        /// cell's text: the spans sit inside the same <c>text:p</c> elements the extraction path
        /// reads, so the offsets have to be counted against exactly the flattening that path
        /// performs — the same whitespace collapsing, the same expansion of <c>text:s</c>, and a
        /// newline between paragraphs. A mismatch would silently shift every span, so the text this
        /// counts is handed to <see cref="SheetRichText"/> and compared against what is drawn.
        /// </para>
        /// <para>
        /// A cell that names one style throughout and no span is not rich, and costs nothing: the
        /// walk stops at the first paragraph when it finds no span element.
        /// </para>
        /// </remarks>
        private void ReadSpans(XElement cell, int row, int column, SheetCellFormat format)
        {
            if (!HasSpan(cell)) return;

            StringBuilder text = new();
            List<SheetTextPortion> portions = [];
            Flattener state = new();

            foreach (XElement paragraph in cell.Elements(XName.Get("p", OdfNamespaces.Text)))
            {
                if (text.Length > 0)
                {
                    text.Append('\n');
                    state.Reset();
                }

                Flatten(paragraph, text, portions, format, null, state);
            }

            _rich.Set(row, column, text.ToString(), format, portions);
        }

        private static bool HasSpan(XElement cell)
            => cell.Elements(XName.Get("p", OdfNamespaces.Text))
                   .Any(paragraph => paragraph
                       .Descendants(XName.Get("span", OdfNamespaces.Text)).Any());

        /// <summary>Where the whitespace collapsing has got to, which is per paragraph.</summary>
        private sealed class Flattener
        {
            public bool AtStart { get; set; } = true;

            public bool LastWasSpace { get; set; }

            public void Reset()
            {
                AtStart = true;
                LastWasSpace = false;
            }
        }

        private void Flatten(
            XElement element,
            StringBuilder text,
            List<SheetTextPortion> portions,
            SheetCellFormat cellFormat,
            string? spanStyle,
            Flattener state)
        {
            int start = text.Length;

            foreach (XNode node in element.Nodes())
            {
                if (node is XText literal)
                {
                    Collapse(text, literal.Value, state);
                    continue;
                }

                if (node is not XElement child || child.Name.NamespaceName != OdfNamespaces.Text)
                    continue;

                switch (child.Name.LocalName)
                {
                    case "span":
                        Flatten(
                            child, text, portions, cellFormat,
                            child.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value,
                            state);
                        break;

                    case "a":
                        Flatten(child, text, portions, cellFormat, spanStyle, state);
                        break;

                    case "s":
                        Literal(text, new string(' ', Spaces(child)), state);
                        break;

                    case "tab":
                        Literal(text, "\t", state);
                        break;

                    case "line-break":
                        Literal(text, "\n", state);
                        break;

                    default:
                        break;
                }
            }

            if (spanStyle is null || text.Length == start) return;

            // A span's own portion is recorded after its children, so a nested span's portion comes
            // first and the outer one is trimmed against it by SheetRichText's normalisation rather
            // than overwriting it.
            portions.Add(new SheetTextPortion(
                start, text.Length - start, TextStyle(cellFormat, spanStyle)));
        }

        private static int Spaces(XElement element)
            => Math.Clamp(
                int.TryParse(
                    element.Attribute(XName.Get("c", OdfNamespaces.Text))?.Value,
                    NumberStyles.Integer, CultureInfo.InvariantCulture, out int count)
                    ? count
                    : 1,
                0, 4096);

        private static void Collapse(StringBuilder text, string value, Flattener state)
        {
            foreach (char character in value)
            {
                if (character is ' ' or '\t' or '\r' or '\n')
                {
                    if (state.AtStart || state.LastWasSpace) continue;
                    text.Append(' ');
                    state.LastWasSpace = true;
                }
                else
                {
                    text.Append(character);
                    state.LastWasSpace = false;
                    state.AtStart = false;
                }
            }
        }

        private static void Literal(StringBuilder text, string value, Flattener state)
        {
            if (value.Length == 0) return;
            text.Append(value);
            state.AtStart = false;
            state.LastWasSpace = true;
        }

        /// <summary>
        /// A <c>text:span</c>'s style, laid over the cell's own format.
        /// </summary>
        /// <remarks>
        /// The style family is <c>text</c> rather than <c>table-cell</c>, and only the character
        /// properties are read: a span inside a cell cannot state an alignment, a wrap or a
        /// rotation, and would have nowhere to put one if it did.
        /// </remarks>
        private SheetCellFormat TextStyle(SheetCellFormat cellFormat, string styleName)
        {
            (string? family, FontFamilyClass declared) =
                FontIdentity(styleName, OdfStyleFamily.Text);

            Length? size = Measure(Span(styleName, "font-size", OdfNamespaces.FoCompatible));
            string? weight = Span(styleName, "font-weight", OdfNamespaces.FoCompatible);
            string? posture = Span(styleName, "font-style", OdfNamespaces.FoCompatible);
            Colour? colour = Rgb(Span(styleName, "color", OdfNamespaces.FoCompatible));

            return cellFormat with
            {
                FontFamily = family ?? cellFormat.FontFamily,
                DeclaredFontClass = family is null ? cellFormat.DeclaredFontClass : declared,
                FontSize = size ?? cellFormat.FontSize,
                FontWeight = weight is null ? cellFormat.FontWeight : Weight(weight),
                IsItalic = posture is null
                    ? cellFormat.IsItalic
                    : posture is "italic" or "oblique",
                Underline = Underline(
                    Span(styleName, "text-underline-style", OdfNamespaces.Style),
                    Span(styleName, "text-underline-type", OdfNamespaces.Style))
                    ?? cellFormat.Underline,
                IsStruckThrough = Struck(
                    Span(styleName, "text-line-through-style", OdfNamespaces.Style))
                    ?? cellFormat.IsStruckThrough,
                Colour = colour ?? cellFormat.Colour,

                // A span's own `fo:color` is a hard character attribute and a cell-level one is
                // an engine default, which is what makes only the first of them beat a hyperlink
                // field's colour. See `SheetCellFormat.ColourIsHard`.
                ColourIsHard = colour is not null,
            };
        }

        private string? Span(string styleName, string property, string ns)
            => styles.ResolveProperty(
                styleName, OdfStyleFamily.Text, OdfPropertyKind.Text, ns, property).Value;

        /// <summary>
        /// How many repeats of one element are materialised.
        /// </summary>
        /// <remarks>
        /// A spreadsheet pads every row to the sheet's full width and the sheet to its full
        /// height, so an unbounded expansion here would record sixteen billion formats for a file
        /// that states none.
        /// </remarks>
        private const int MaxRepeat = 4096;

        /// <summary>The cell style everything in a sheet is in until it says otherwise.</summary>
        /// <remarks>
        /// <para>
        /// Not a guess at a convention: <c>Default</c> is the programmatic API name of Calc's
        /// default cell style, and its own ODF import says so — <c>ScXMLTableColContext</c>
        /// (<c>sc/source/filter/xml/xmlcoli.cxx</c>:135-137) substitutes it for a
        /// <c>table:table-column</c> that names no <c>table:default-cell-style-name</c>, and the
        /// export (<c>xmlexprt.cxx</c>:897, :2156) writes the name out for everything that is in it.
        /// A file naming no such style still resolves, because <c>OdfStyles</c> falls back to the
        /// anonymous <c>style:default-style</c> for a name it cannot find.
        /// </para>
        /// <para>
        /// It is the sheet's base pattern in Calc (<c>ScDocument::GetDefPattern</c>), which is what
        /// makes the padding free there: a run of cells whose pattern equals the surrounding one
        /// merges into it rather than each becoming an attribute of its own. Resolving it per cell
        /// instead is what cost a 10 kB two-sheet <c>.ods</c> 2.25 GB and 20 seconds to open, since
        /// Calc pads every row to 16382 columns and every sheet to 1048571 rows with cells naming
        /// exactly this style.
        /// </para>
        /// </remarks>
        private const string DefaultCellStyleName = "Default";

        private int Intern(string? styleName)
        {
            if (string.IsNullOrEmpty(styleName)) return 0;

            if (!_resolved.TryGetValue(styleName, out SheetCellFormat? format))
            {
                format = Resolve(styleName);
                _resolved[styleName] = format;
            }

            int index = _builder.Intern(format);
            _pool[index] = format;
            return index;
        }

        /// <summary>
        /// What a cell resolves to, in the order the lookup itself resolves: cell, row, column.
        /// </summary>
        /// <remarks>
        /// Repeated here because a span is a <em>delta</em> over the cell's format, so a bold word
        /// in a cell that takes its font from its column has to start from the column's font. Row
        /// before column is the same order <see cref="SheetCellFormats"/> states and the same order
        /// every one of the three formats writes.
        /// </remarks>
        private SheetCellFormat Effective(int cellIndex, int column)
        {
            int index = cellIndex != 0 ? cellIndex : InheritedIndex(column);

            return _pool.GetValueOrDefault(index, SheetCellFormat.Default);
        }

        /// <summary>The <c>Default</c> cell style's font, as a band's default.</summary>
        public SheetDefaultFont DefaultFont()
        {
            SheetCellFormat format = Resolve(DefaultCellStyleName);

            return new SheetDefaultFont(
                format.FontFamily,
                format.FontSize,
                format.FontWeight,
                format.IsItalic,
                format.DeclaredFontClass);
        }

        /// <summary>One named style's own resolved format.</summary>
        public SheetCellFormat ResolveNamed(string styleName) => Resolve(styleName);

        private SheetCellFormat Resolve(string styleName)
        {
            (string? family, FontFamilyClass declared) =
                FontIdentity(styleName, OdfStyleFamily.TableCell);

            return new SheetCellFormat
            {
                FontFamily = family,
                DeclaredFontClass = declared,
                FontSize = Points(Text(styleName, "font-size")) ?? Length.FromPoints(10),
                FontWeight = Weight(Text(styleName, "font-weight")),
                IsItalic = Text(styleName, "font-style") is "italic" or "oblique",
                Underline = Underline(
                    Text(styleName, "text-underline-style"),
                    Text(styleName, "text-underline-type")) ?? SheetUnderline.None,
                IsStruckThrough = Struck(Text(styleName, "text-line-through-style")) ?? false,
                Colour = Rgb(Text(styleName, "color")) ?? Colour.Black,
                Horizontal = HorizontalOf(styleName),
                Vertical = VerticalOf(Cell(styleName, "vertical-align", OdfNamespaces.Style)),
                Wraps = Cell(styleName, "wrap-option", OdfNamespaces.FoCompatible) == "wrap",
                ShrinksToFit = Cell(styleName, "shrink-to-fit", OdfNamespaces.Style) == "true",
                Indent = Length.FromEmu(Measure(Paragraph(styleName, "margin-left", OdfNamespaces.FoCompatible))?.Emu ?? 0),
                RotationDegrees = Rotation(Cell(styleName, "rotation-angle", OdfNamespaces.Style)),
                IsStacked = Cell(styleName, "direction", OdfNamespaces.Style) == "ttb",
                NumberFormatKind = FormatKind(styleName),

                // The format's own code, compiled from the `number:*-style` element tree the way
                // `xmloff` does. The kind above is what the drawn cell needs — it decides the
                // `###` rule — and the code is what states the format to a caller: the HTML
                // export's `sdnum`, and the `*` fill directive, which could not fire on this path
                // while the code was null.
                // A multi-section format is several elements linked by `style:map`, so the
                // resolver is what turns the one the cell names into the whole code.
                NumberFormat = OdfNumberFormat.Parse(DataStyleElement(styleName), DataStyleByName),
            };
        }

        /// <summary>
        /// What kind of value the cell's data style formats.
        /// </summary>
        /// <remarks>
        /// ODF names a structured <c>number:*-style</c> rather than an Excel format code, so the
        /// kind is all there is — and it is all drawing needs. A cell that names no data style at
        /// all is <c>General</c>, which is what makes a too-narrow number re-render itself shorter
        /// instead of showing hashes.
        /// </remarks>
        private Core.Numbers.NumberFormatKind FormatKind(string styleName)
        {
            string? name = DataStyleName(styleName);
            if (name is null) return Core.Numbers.NumberFormatKind.General;

            return styles.FindDataStyle(name)?.Kind switch
            {
                OdfDataStyleKind.Number or OdfDataStyleKind.Percentage
                    or OdfDataStyleKind.Currency => Core.Numbers.NumberFormatKind.Number,
                OdfDataStyleKind.Date or OdfDataStyleKind.Time => Core.Numbers.NumberFormatKind.DateTime,
                OdfDataStyleKind.Text => Core.Numbers.NumberFormatKind.Text,

                // A boolean style is a number format in Calc too, and an unrecognised one is
                // still a stated format — which is what the ### rule turns on.
                _ => Core.Numbers.NumberFormatKind.Number,
            };
        }

            /// <summary>The element of the data style a cell style names, or null when it names none.</summary>
        private XElement? DataStyleElement(string styleName)
            => styles.FindDataStyle(DataStyleName(styleName))?.Element;

        /// <summary>A data style by its own name, which is what a <c>style:map</c> names.</summary>
        /// <remarks>
        /// The styles a map points at are <c>style:volatile="true"</c> — no cell names one — so
        /// this is the only way they are reached, and it is by the style's own name rather than
        /// through the cell-style chain <see cref="DataStyleName"/> walks.
        /// </remarks>
        private XElement? DataStyleByName(string name) => styles.FindDataStyle(name)?.Element;

    /// <summary>The data style a cell style names, following its parent chain.</summary>
        /// <remarks>
        /// Walked here rather than through <c>ResolveProperty</c> because
        /// <c>style:data-style-name</c> is an attribute of the style element rather than one of
        /// its property sets, and the resolver only cascades properties.
        /// </remarks>
        private string? DataStyleName(string styleName)
        {
            string? name = styleName;
            for (int depth = 0; depth < OdfStyles.MaxParentChainDepth && name is not null; depth++)
            {
                OdfStyle? style = styles.Find(name, OdfStyleFamily.TableCell);
                if (style is null) return null;
                if (!string.IsNullOrEmpty(style.DataStyleName)) return style.DataStyleName;

                name = style.ParentStyleName;
            }

            return null;
        }

        /// <summary>
        /// ODF's alignment, which needs two attributes rather than one.
        /// </summary>
        /// <remarks>
        /// <c>style:text-align-source</c> can veto the paragraph's <c>fo:text-align</c> entirely:
        /// <c>value-type</c> means "align by what the cell holds", which is Calc's General, and it
        /// wins however the paragraph is aligned. Calc writes it on a cell whose alignment was
        /// explicitly reset, and writes no <c>fo:text-align</c> at all for one that was never
        /// aligned — so both routes to General have to be honoured or a column of figures comes
        /// out ragged.
        /// </remarks>
        private SheetHorizontalAlignment HorizontalOf(string styleName)
        {
            if (Cell(styleName, "text-align-source", OdfNamespaces.Style) is "value-type")
                return SheetHorizontalAlignment.General;

            return Paragraph(styleName, "text-align", OdfNamespaces.FoCompatible) switch
            {
                "start" or "left" => SheetHorizontalAlignment.Left,
                "center" => SheetHorizontalAlignment.Centre,
                "end" or "right" => SheetHorizontalAlignment.Right,
                "justify" => SheetHorizontalAlignment.Justify,
                _ => SheetHorizontalAlignment.General,
            };
        }

        private static SheetVerticalAlignment VerticalOf(string? value) => value switch
        {
            "top" => SheetVerticalAlignment.Top,
            "middle" => SheetVerticalAlignment.Centre,
            "bottom" => SheetVerticalAlignment.Bottom,
            _ => SheetVerticalAlignment.Standard,
        };

        /// <summary>
        /// ODF states the angle anticlockwise in degrees, which is Calc's own sense.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Calc's <c>ATTR_ROTATE_VALUE</c> runs over the whole turn — its own writer emits 27000 for
        /// what its dialog calls −90 — so a stated angle has to be folded into the half turn
        /// <see cref="SheetCellFormat.RotationDegrees"/> keeps before it is clamped. Clamping
        /// straight away is what the first version did, and it turned 270° into +90 and 315° into
        /// +90 as well: a cell whose text should read top-to-bottom read bottom-to-top instead, and
        /// a 45° heading leant the wrong way. Measured on a flat-ODS probe of six angles, three
        /// sizes and six string lengths, that mistake also put every 315° row on the wrong height —
        /// LibreOffice measures it as 45° and this measured it as a quarter turn.
        /// </para>
        /// <para>
        /// Beyond a quarter turn the fold still clamps, because the model has nowhere to put an
        /// upside-down cell; nothing in the corpus states one.
        /// </para>
        /// </remarks>
        private static int Rotation(string? value)
        {
            if (value is null
                || !double.TryParse(
                    value.Replace("deg", string.Empty, StringComparison.OrdinalIgnoreCase),
                    NumberStyles.Float, CultureInfo.InvariantCulture, out double degrees))
            {
                return 0;
            }

            int folded = ((int)Math.Round(degrees) % 360 + 360) % 360;
            if (folded > 180) folded -= 360;

            return Math.Clamp(folded, -90, 90);
        }

        /// <summary>
        /// The face a style asks for, deciding the level of the parent chain before the spelling.
        /// </summary>
        /// <remarks>
        /// <c>style:font-name</c> and <c>fo:font-family</c> are two spellings of one item, so an
        /// ancestor stating the second must not beat a child stating the first. See
        /// <see cref="OdfStyles.ResolveWithoutDefaults(string, OdfStyleFamily, OdfPropertyKind,
        /// IReadOnlyList{ValueTuple{string, string}}, out int)"/>, which is where the whole of the
        /// reasoning is. Every automatic cell style an <c>.ods</c> converted from a workbook
        /// carries states <c>style:font-name</c> and no <c>fo:font-family</c>, while the
        /// <c>Default</c> cell style it inherits from states both — so reading them independently
        /// draws every such sheet in the document default's face rather than the cell's.
        /// </remarks>
        private (string? Family, FontFamilyClass Declared) FontIdentity(
            string styleName, OdfStyleFamily family)
        {
            OdfProperty found = styles.ResolveWithoutDefaults(
                styleName, family, OdfPropertyKind.Text, FontSpellings, out int matched);

            if (!found.HasValue)
            {
                found = styles.ResolveFromDefaults(
                    family, OdfPropertyKind.Text, OdfNamespaces.FoCompatible, "font-family");
                matched = 0;

                if (!found.HasValue)
                {
                    found = styles.ResolveFromDefaults(
                        family, OdfPropertyKind.Text, OdfNamespaces.Style, "font-name");
                    matched = 1;
                }
            }

            if (!found.HasValue) return (null, FontFamilyClass.Unknown);

            return matched == 1
                ? (FontFaceFamily(found.Value), FontFaceClass(found.Value))
                : (found.Value, FontFamilyClass.Unknown);
        }

        /// <summary>The two spellings of a face, in the order one style's own attributes settle.</summary>
        /// <remarks>
        /// <c>fo:font-family</c> first, because a style stating both is settled by attribute order
        /// and LibreOffice's export writes <c>style:font-name</c> before it — the second write into
        /// <c>CTF_FONTFAMILYNAME</c> is the one that stands. What this pair changes is only which
        /// <em>level</em> answers; where one style states both, the answer is what it always was.
        /// </remarks>
        private static readonly (string Namespace, string Name)[] FontSpellings =
        [
            (OdfNamespaces.FoCompatible, "font-family"),
            (OdfNamespaces.Style, "font-name"),
        ];

        private string? FontFaceFamily(string? name)
            => name is not null && styles.FontFaces.TryGetValue(name, out OdfFontFace? face)
                ? face.FontFamily ?? name
                : name;

        /// <summary>
        /// The generic family an <c>office:font-face-decls</c> entry declares for a face.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Only reachable through <c>style:font-name</c>: <c>fo:font-family</c> states a family
        /// list directly and carries no generic, which is why this follows the same precedence the
        /// family itself does and answers nothing when the family came from the <c>fo:</c>
        /// property.
        /// </para>
        /// <para>
        /// Measured on 26.2.4.2 with a one-cell flat ODS naming <c>Bell MT</c>, which is installed
        /// nowhere here: <c>style:font-family-generic="roman"</c> renders it in DejaVu Serif and
        /// the same file without the attribute renders it in DejaVu Sans. The other five values —
        /// <c>swiss</c>, <c>modern</c>, <c>decorative</c>, <c>script</c>, <c>system</c> — each
        /// leave it exactly where the undeclared file left it, which is the same result the DOCX
        /// and DOC probes reached and the reason <see cref="SheetDeclaredFonts"/> carries two of
        /// the six.
        /// </para>
        /// </remarks>
        private FontFamilyClass FontFaceClass(string? name)
            => name is not null && styles.FontFaces.TryGetValue(name, out OdfFontFace? face)
                ? SheetDeclaredFonts.FromOdfGeneric(face.GenericFamily)
                : FontFamilyClass.Unknown;

        private string? Cell(string styleName, string property, string ns)
            => styles.ResolveProperty(
                styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell, ns, property).Value;

        private string? Paragraph(string styleName, string property, string ns)
            => styles.ResolveProperty(
                styleName, OdfStyleFamily.TableCell, OdfPropertyKind.Paragraph, ns, property).Value;

        private string? Text(string styleName, string property)
            => styles.ResolveProperty(
                styleName, OdfStyleFamily.TableCell, OdfPropertyKind.Text,
                OdfNamespaces.FoCompatible, property).Value
               ?? styles.ResolveProperty(
                   styleName, OdfStyleFamily.TableCell, OdfPropertyKind.Text,
                   OdfNamespaces.Style, property).Value;

        /// <summary>
        /// ODF's underline, which is stated as two properties rather than one.
        /// </summary>
        /// <remarks>
        /// <c>style:text-underline-style</c> names the dash pattern — solid, dotted, wave and a
        /// dozen more — and <c>style:text-underline-type</c> says single or double, so a double
        /// underline is <c>style="solid" type="double"</c> and not a style of its own. Only the
        /// count is reproduced; a dotted underline draws solid, which is the same simplification
        /// <see cref="SheetUnderline"/> already records for Excel's accounting forms.
        /// </remarks>
        /// <returns>Null when the style states nothing, so an inherited value survives.</returns>
        private static SheetUnderline? Underline(string? style, string? type) => style switch
        {
            null => null,
            "none" => SheetUnderline.None,
            _ => type == "double" ? SheetUnderline.DoubleLine : SheetUnderline.SingleLine,
        };

        /// <inheritdoc cref="Underline"/>
        private static bool? Struck(string? style) => style switch
        {
            null => null,
            "none" => false,
            _ => true,
        };

        private static int Weight(string? value)
            => value switch
            {
                null or "normal" => 400,
                "bold" => 700,
                _ => int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int weight)
                    ? Math.Clamp(weight, 1, 1000)
                    : 400,
            };

        private static Length? Points(string? value) => Measure(value);

        private static Colour? Rgb(string? value)
            => value is { Length: 7 } text && text[0] == '#'
               && uint.TryParse(
                   text[1..], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out uint rgb)
                ? Colour.FromRgb(rgb)
                : null;

        /// <summary>An ODF length, in any of the units the format allows.</summary>
        private static Length? Measure(string? value)
        {
            if (string.IsNullOrWhiteSpace(value)) return null;

            string text = value.Trim();
            int at = 0;
            while (at < text.Length && (char.IsAsciiDigit(text[at]) || text[at] is '-' or '+' or '.')) at++;
            if (at == 0) return null;

            if (!double.TryParse(
                    text[..at], NumberStyles.Float, CultureInfo.InvariantCulture, out double number))
            {
                return null;
            }

            return text[at..].Trim().ToLowerInvariant() switch
            {
                "cm" => Length.FromMillimetres(number * 10),
                "mm" => Length.FromMillimetres(number),
                "in" => Length.FromInches(number),
                "pt" => Length.FromPoints(number),
                "pc" => Length.FromPoints(number * 12),
                "px" => Length.FromPixels(number, 96),
                _ => Length.FromPoints(number),
            };
        }

        private static IEnumerable<XElement> Descend(XElement table, string name)
        {
            foreach (XElement child in table.Elements())
            {
                if (child.Name.NamespaceName != OdfNamespaces.Table) continue;

                // A header band or a group wraps the rows and columns it holds; its children are
                // still the sheet's own, in order.
                if (child.Name.LocalName == name)
                {
                    yield return child;
                }
                else if (child.Name.LocalName is "table-header-rows" or "table-header-columns"
                             or "table-row-group" or "table-column-group")
                {
                    foreach (XElement nested in Descend(child, name)) yield return nested;
                }
            }
        }

        private static string? Attribute(XElement element, string name)
            => element.Attribute(XName.Get(name, OdfNamespaces.Table))?.Value;

        private static int Repeat(XElement element, string name)
            => Attribute(element, name) is { } value
               && int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int count)
               && count > 0
                ? count
                : 1;
    }
}

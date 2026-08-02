using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.OpenDocument;

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

        public (SheetCellFormats Formats, SheetRichText RichText) Read(XElement table)
        {
            ReadColumns(table);
            ReadRows(table);
            return (_builder.Build(), _rich.Build());
        }

        private void ReadColumns(XElement table)
        {
            int column = 0;
            foreach (XElement element in Descend(table, "table-column"))
            {
                int repeat = Repeat(element, "number-columns-repeated");
                int index = Intern(Attribute(element, "default-cell-style-name"));

                for (int at = 0; at < repeat && column < SheetAddress.MaxColumn; at++, column++)
                {
                    _builder.SetColumn(column, index);
                    if (index != 0) _columnDefaults[column] = index;
                }
            }
        }

        private void ReadRows(XElement table)
        {
            int row = 0;
            foreach (XElement element in Descend(table, "table-row"))
            {
                int repeat = Repeat(element, "number-rows-repeated");
                int rowStyle = Intern(Attribute(element, "default-cell-style-name"));

                // A row repeated a million times is the sheet's padding, not a million formatted
                // rows: reading its cells once and stamping them across the sheet would
                // materialise the whole grid. Only the first is recorded, which is what the
                // extraction path does with the same attribute.
                int span = Math.Min(repeat, MaxRepeat);

                for (int at = 0; at < span && row < SheetAddress.MaxRow; at++, row++)
                {
                    _builder.SetRow(row, rowStyle);
                    _rowDefault = rowStyle;
                    ReadCells(element, row);
                }

                if (repeat > span) row += repeat - span;
            }
        }

        private void ReadCells(XElement rowElement, int row)
        {
            int column = 0;
            foreach (XElement cell in rowElement.Elements())
            {
                if (cell.Name.NamespaceName != OdfNamespaces.Table) continue;
                if (cell.Name.LocalName is not ("table-cell" or "covered-table-cell")) continue;

                string? styleName = Attribute(cell, "style-name");
                int repeat = Repeat(cell, "number-columns-repeated");
                int index = Intern(styleName);

                int span = Math.Min(repeat, MaxRepeat);
                for (int at = 0; at < span && column < SheetAddress.MaxColumn; at++, column++)
                {
                    _builder.SetCell(row, column, index);
                    ReadSpans(cell, row, column, Effective(index, column));
                }

                if (repeat > span) column += repeat - span;
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
            string? family = Span(styleName, "font-family", OdfNamespaces.FoCompatible)
                             ?? FontFaceFamily(Span(styleName, "font-name", OdfNamespaces.Style));

            Length? size = Measure(Span(styleName, "font-size", OdfNamespaces.FoCompatible));
            string? weight = Span(styleName, "font-weight", OdfNamespaces.FoCompatible);
            string? posture = Span(styleName, "font-style", OdfNamespaces.FoCompatible);
            Colour? colour = Rgb(Span(styleName, "color", OdfNamespaces.FoCompatible));

            return cellFormat with
            {
                FontFamily = family ?? cellFormat.FontFamily,
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
            int index = cellIndex != 0
                ? cellIndex
                : _rowDefault != 0 ? _rowDefault : _columnDefaults.GetValueOrDefault(column);

            return _pool.GetValueOrDefault(index, SheetCellFormat.Default);
        }

        private SheetCellFormat Resolve(string styleName)
        {
            string? family = Text(styleName, "font-family")
                             ?? FontFaceFamily(Text(styleName, "font-name"));

            return new SheetCellFormat
            {
                FontFamily = family,
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

        /// <summary>ODF states the angle anticlockwise in degrees, which is Calc's own sense.</summary>
        private static int Rotation(string? value)
            => value is not null
               && double.TryParse(
                   value.Replace("deg", string.Empty, StringComparison.OrdinalIgnoreCase),
                   NumberStyles.Float, CultureInfo.InvariantCulture, out double degrees)
                ? Math.Clamp((int)Math.Round(degrees), -90, 90)
                : 0;

        private string? FontFaceFamily(string? name)
            => name is not null && styles.FontFaces.TryGetValue(name, out OdfFontFace? face)
                ? face.FontFamily ?? name
                : name;

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

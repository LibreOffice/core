using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// The pivot table's own <c>&lt;format&gt;</c> records, and which of its cells each one lands on.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Clearing the output range is only half of what the reference does.</strong>
/// <c>ScDPOutput::Output</c> empties the range, writes the results, applies the four generated
/// <c>Pivot Table *</c> styles — and then ends with <c>maFormatOutput.apply</c>
/// (<c>sc/source/core/data/dpoutput.cxx</c>:1190), which lays the pivot's own <c>&lt;format&gt;</c>
/// records over the lot. Everything a pivot cell shows that is not one of those four styles comes
/// from here.
/// </para>
/// <para>
/// <strong>The null that proves the clearing.</strong>
/// <c>033_Event_planning_tracker</c> with its <c>&lt;formats&gt;</c> element deleted and nothing
/// else changed comes back from 26.2.4.2 with all ninety-one cells of its pivot at the
/// <c>Default</c> cell style — no face, no size, no colour, no fill — and only the generated bold
/// on the grand-total row. With the element back, the same ninety-one carry a 12 pt
/// <c>Consolas</c> on a black ground. <c>probes/pivot-fmt-r110</c>.
/// </para>
/// <para>
/// <strong>What the matcher is, and the three things in the C++ tree that 26.2.4.2 does not
/// do.</strong> The tree read here declares 27.2.0.0.alpha0+ and is not the reference binary's
/// source, and on this subsystem the two differ. Each of the three was settled by applying one
/// format at a time to <c>033</c> and reading 26.2.4.2's own <c>.fods</c>:
/// </para>
/// <list type="bullet">
/// <item><description><c>tryHandleGrandTotals</c> (<c>PivotTableFormatOutput.cxx</c>:582) does not
/// exist in 26.2.4.2. Its four <c>grandRow="1"</c> data formats paint the <em>whole</em> data
/// area, not the grand-total row, and its <c>grandRow="1"</c> label formats paint nothing at
/// all — which is exactly what ordinary matching gives them.</description></item>
/// <item><description>An <c>&lt;alignment&gt;</c> in a <c>dxf</c> reaches no cell. <c>033</c>
/// states nine of them, three of which match the whole data area, and 26.2.4.2's automatic style
/// for those cells carries no <c>fo:text-align</c> and no <c>fo:margin-left</c>. So the
/// justification and the indent stay the generated styles', which is what
/// <c>probes/pivot-res-r108</c> measured over 9970 cells before any of this was
/// read.</description></item>
/// <item><description><c>PivotAreaType</c> is parsed and never used — <c>type="all"</c>,
/// <c>type="button"</c> and <c>type="origin"</c> behave as <c>normal</c>
/// (<c>PivotTableFormat::finalizeImport</c>, which reads only <c>dataOnly</c>, <c>labelOnly</c>,
/// <c>grandRow</c>, <c>grandCol</c>, <c>offset</c>, <c>fieldPosition</c> and the
/// references).</description></item>
/// </list>
/// <para>
/// <strong>Scored against 26.2.4.2's own resolved view of all 9878 cells of the corpus's nineteen
/// generatable pivot rectangles, by value and not by "differs from the default":</strong> face
/// <strong>9878</strong>, size <strong>9878</strong>, colour <strong>9878</strong>, fill
/// <strong>9869</strong>. The nine are one workbook's field-button row and corner cell, where the
/// reference paints a fill this does not; every one is an under-application.
/// <c>probes/pivot-fmt-r110/check-dxf.py</c>.
/// </para>
/// </remarks>
internal static class XlsxPivotFormats
{
    /// <summary>The data-layout dimension, <c>sc::constDataDimension</c>.</summary>
    private const int DataDimension = -2;

    /// <summary>What one <c>dxf</c> changes about a cell.</summary>
    /// <param name="Text">The font half, as a differential over whatever the cell resolves to.</param>
    /// <param name="Fill">The colour its fill paints, or null when it states none.</param>
    internal readonly record struct Difference(SheetPivotDxf Text, Colour? Fill)
    {
        /// <summary>True when the entry changes nothing this tree models.</summary>
        public bool IsNone => Text.IsNone && Fill is null;
    }

    /// <summary>
    /// The <c>dxfs</c> table, in the order a <c>dxfId</c> indexes it.
    /// </summary>
    /// <param name="styleSheet">The <c>styleSheet</c> root, or null when the part is missing.</param>
    /// <param name="palette">The workbook's colours.</param>
    public static IReadOnlyList<Difference> Read(XElement? styleSheet, XlsxPalette palette)
    {
        ArgumentNullException.ThrowIfNull(palette);
        if (styleSheet is null) return [];

        List<Difference> table = [];
        foreach (XElement dxf in Xlsx.Children(Xlsx.Child(styleSheet, "dxfs"), "dxf"))
        {
            XElement? font = Xlsx.Child(dxf, "font");
            string? family = Xlsx.Attribute(Xlsx.Child(font, "name"), "val");
            SheetPivotDxf text = font is null ? default : new SheetPivotDxf
            {
                FontFamily = family,

                // The declaration follows the name it qualifies, for the reason
                // `XlsxCellFormats.Apply` gives for a rich-text run: a dxf that renames the face
                // and says nothing about its shape has said the shape is unknown.
                DeclaredFontClass = family is null
                    ? null
                    : Xlsx.Integer(Xlsx.Child(font, "family"), "val") is { } code
                        ? SheetDeclaredFonts.FromWindowsCode(code)
                        : FontFamilyClass.Unknown,
                FontSize = Xlsx.Double(Xlsx.Attribute(Xlsx.Child(font, "sz"), "val")) is > 0 and { } points
                    ? Length.FromPoints(points)
                    : null,
                Colour = palette.Read(Xlsx.Child(font, "color")),
                FontWeight = Toggle(Xlsx.Child(font, "b")) switch
                {
                    true => 700,
                    false => 400,
                    null => null,
                },
            };

            table.Add(new Difference(text, ReadFill(Xlsx.Child(dxf, "fill"), palette)));
        }

        return table;
    }

    /// <summary>
    /// The colour a <c>dxf</c>'s fill paints, or null when it paints nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The <c>mbDxf</c> arm of <c>Fill::finalizeImport</c>
    /// (<c>sc/source/filter/oox/stylesbuffer.cxx</c>:1978-2055), in the order it tests. Two of
    /// its cases decide whole documents here and neither is what the markup looks like it says:
    /// </para>
    /// <para>
    /// A <c>&lt;patternFill&gt;</c> that states a <c>bgColor</c> and <strong>no</strong>
    /// <c>patternType</c> is turned into a <em>solid</em> fill, and the colour it paints is the
    /// <c>bgColor</c>: the arm at <c>:1988-1994</c> moves <c>maFillColor</c> — which
    /// <c>importBgColor</c> filled, <c>:1890</c> — <em>into</em> <c>maPatternColor</c> and sets
    /// <c>mnPattern = XML_solid</c>, so the <c>fgColor</c>'s own value is pushed aside into
    /// <c>maFilterPatternColor</c> and never painted. What makes
    /// <c>033_Event_planning_tracker</c>'s band <strong>black</strong> is therefore that its
    /// <c>bgColor</c> is <c>auto="1"</c>, and an automatic colour in the pattern position
    /// resolves against the window <em>text</em> colour. That, and nothing in the workbook, is
    /// where the band comes from; it is 89 cells of one page and it is the fill
    /// <c>probes/pivot-res-r108</c> recorded as unexplained. The same file also states
    /// <c>patternType="none"</c> beside a <c>bgColor</c>, which applies nothing at all —
    /// <c>XML_none should not apply any color</c>, <c>:2009</c>.
    /// </para>
    /// <para>
    /// <strong>Round 110's write-up said this the other way round</strong> — that the solid takes
    /// the <em>pattern</em> colour and that an unstated pattern colour is what is automatic. The
    /// code below was always right; only the prose was not, and <c>033</c> cannot tell the two
    /// apart because its <c>fgColor</c> is unstated <em>and</em> its <c>bgColor</c> is
    /// <c>auto="1"</c>, so both readings paint black. A workbook stating a real <c>bgColor</c>
    /// under no <c>patternType</c> would separate them.
    /// </para>
    /// </remarks>
    private static Colour? ReadFill(XElement? fill, XlsxPalette palette)
    {
        XElement? pattern = Xlsx.Child(fill, "patternFill");
        if (pattern is null) return null;

        string? type = Xlsx.Attribute(pattern, "patternType");
        XElement? background = Xlsx.Child(pattern, "bgColor");
        XElement? foreground = Xlsx.Child(pattern, "fgColor");

        // `mbFillColorUsed && (!mbPatternUsed || solid)`: the background becomes the pattern
        // colour and the pattern becomes solid.
        if (background is not null && type is null or "solid")
            return Automatic(background, palette);

        // `!mbFillColorUsed && !mbPattColorUsed && mbPatternUsed && solid` turns the pattern off.
        if (background is null && foreground is null) return null;
        if (string.Equals(type, "none", StringComparison.Ordinal)) return null;

        return foreground is null ? Colour.Black : Automatic(foreground, palette);
    }

    /// <summary>
    /// A colour in the <em>pattern</em> position, where automatic is the window text colour.
    /// </summary>
    /// <remarks>
    /// <c>maPatternColor.getColor(rGraphicHelper, nWinTextColor)</c>, <c>stylesbuffer.cxx</c>:2046
    /// — and <c>lclGetMixedColor</c> at a solid pattern's <c>0x80</c> keeps the pattern colour
    /// whole. <c>XlsxPalette.Read</c> answers null for an automatic colour because in
    /// every other position that is what it means.
    /// </remarks>
    private static Colour Automatic(XElement stated, XlsxPalette palette)
        => palette.Read(stated) ?? Colour.Black;

    private static bool? Toggle(XElement? element)
        => element is null ? null : Xlsx.Flag(element, "val", true);

    /// <summary>Where the pivot's output sits, which is all the matcher needs of the layout.</summary>
    /// <param name="TabStartColumn">The table's own first column.</param>
    /// <param name="DataStartColumn">The first column of the data area.</param>
    /// <param name="DataStartRow">The first row of the data area.</param>
    internal readonly record struct Geometry(int TabStartColumn, int DataStartColumn, int DataStartRow);

    /// <summary>
    /// Applies every <c>&lt;format&gt;</c> of one pivot, in document order.
    /// </summary>
    /// <param name="root">The <c>pivotTableDefinition</c> root.</param>
    /// <param name="table">The workbook's <c>dxfs</c>, as <see cref="Read"/> built them.</param>
    /// <param name="geometry">Where the output sits.</param>
    /// <param name="onCell">Called for every cell each record lands on, in application order.</param>
    public static void Apply(
        XElement root, IReadOnlyList<Difference> table, Geometry geometry,
        Action<int, int, Difference> onCell)
    {
        ArgumentNullException.ThrowIfNull(root);
        ArgumentNullException.ThrowIfNull(table);
        ArgumentNullException.ThrowIfNull(onCell);

        List<XElement> records = [.. Xlsx.Children(Xlsx.Child(root, "formats"), "format")];
        if (records.Count == 0) return;

        int[] rowDimensions = Dimensions(root, "rowFields");
        int[] columnDimensions = Dimensions(root, "colFields");
        Line[] rowLines = Axis(Xlsx.Children(Xlsx.Child(root, "rowItems"), "i"), rowDimensions);
        Line[] columnLines = Axis(Xlsx.Children(Xlsx.Child(root, "colItems"), "i"), columnDimensions);

        // A label lands on the row that holds the column members, which is the data start less
        // one row per column field — `nColumnHeaderStartRow`, `PivotTableFormatOutput.cxx`:685-688.
        int headerStartRow = geometry.DataStartRow - columnDimensions.Length;

        foreach (XElement record in records)
        {
            Format format = new(record);

            // FormatType::None applies nothing: `applyMatchedLines` has an arm for Label and an
            // arm for Data and no third one.
            if (format.Kind == Kind.None) continue;
            if (format.Dxf < 0 || format.Dxf >= table.Count) continue;

            Difference difference = table[format.Dxf];
            if (difference.IsNone) continue;

            foreach ((Field[] rows, Field[] columns) in format.Entries(rowDimensions, columnDimensions))
            {
                List<int> hitRows = [];
                List<int> hitColumns = [];

                // A label matches only the axis it names, so a wildcard does not paint the other
                // one's cells.
                if (format.Kind != Kind.Label || IsSet(rows))
                {
                    foreach (int at in Matching(rowLines, rows, format.Kind))
                    {
                        if (format.Kind == Kind.Label)
                            onCell(geometry.TabStartColumn + First(rows), geometry.DataStartRow + at, difference);
                        else
                            hitRows.Add(geometry.DataStartRow + at);
                    }
                }

                if (format.Kind != Kind.Label || IsSet(columns))
                {
                    foreach (int at in Matching(columnLines, columns, format.Kind))
                    {
                        if (format.Kind == Kind.Label)
                            onCell(geometry.DataStartColumn + at, headerStartRow + First(columns), difference);
                        else
                            hitColumns.Add(geometry.DataStartColumn + at);
                    }
                }

                if (format.Kind != Kind.Data) continue;
                foreach (int row in hitRows)
                {
                    foreach (int column in hitColumns) onCell(column, row, difference);
                }
            }
        }
    }

    private static int[] Dimensions(XElement root, string axis)
        => [.. Xlsx.Children(Xlsx.Child(root, axis), "field")
                   .Select(field => Xlsx.Integer(field, "x") is { } x and >= 0 ? x : DataDimension)];

    private static bool IsSet(Field[] fields) => Array.Exists(fields, field => field.Stated);

    private static int First(Field[] fields)
    {
        int at = Array.FindIndex(fields, field => field.Stated);
        return at < 0 ? 0 : at;
    }

    /// <summary><c>findMatchingLines</c>, by line index.</summary>
    private static List<int> Matching(Line[] lines, Field[] wanted, Kind kind)
    {
        List<int> exact = [];
        List<int> broad = [];

        for (int at = 0; at < lines.Length; at++)
        {
            Member[] fields = lines[at].Fields;
            bool all = true;
            bool subtotalMatch = false;
            bool wildcard = false;

            for (int index = 0; index < fields.Length && all; index++)
            {
                Member member = fields[index];
                Field field = wanted[index];
                if (member.Dimension != field.Dimension) { all = false; break; }
                if (!field.Stated) continue;

                bool hit;
                if (field.HasSubtotal)
                {
                    hit = member.Index == field.Index && (member.IsSubtotal || !field.Selected);
                    subtotalMatch |= hit && member.IsSubtotal;
                }
                else if (field.MatchesAll)
                {
                    hit = !member.IsSubtotal;
                }
                else
                {
                    // A member is matched by name, which for one dimension's own ordered members
                    // is its index; the data-layout dimension is matched by position, which is
                    // the same number for it.
                    hit = member.Index == field.Index
                          && (field.Dimension == DataDimension || !member.IsSubtotal);
                }

                if (!hit) all = false;
            }

            for (int index = 0; index < fields.Length && all; index++)
            {
                if (wanted[index].Stated) continue;
                Member member = fields[index];
                if (kind == Kind.Label)
                {
                    if (!subtotalMatch && !member.IsMember && !member.Continues) all = false;
                }
                else if (kind == Kind.Data && !subtotalMatch && (member.IsMember || member.Continues))
                {
                    wildcard = true;
                }
            }

            if (!all) continue;
            (wildcard ? broad : exact).Add(at);
        }

        return kind == Kind.Data && exact.Count == 0 ? broad : exact;
    }

    private enum Kind { None, Data, Label }

    /// <summary>One field of one output line, <c>sc::FieldData</c>.</summary>
    private readonly record struct Member(
        int Dimension, int Index, bool IsMember, bool IsSubtotal, bool Continues);

    private sealed class Line
    {
        public Member[] Fields = [];
    }

    /// <summary>One field of one format entry, <c>sc::FormatOutputField</c>.</summary>
    private readonly record struct Field(
        int Dimension, int Index, bool MatchesAll, bool Selected, bool HasSubtotal, bool Stated);

    /// <summary>One <c>&lt;pivotArea&gt;</c> and its references, <c>sc::PivotTableFormat</c>.</summary>
    private sealed class Format
    {
        private static readonly string[] SubtotalFlags =
        [
            "defaultSubtotal", "sumSubtotal", "countASubtotal", "avgSubtotal", "maxSubtotal",
            "minSubtotal", "productSubtotal", "countSubtotal", "stdDevSubtotal", "stdDevPSubtotal",
            "varSubtotal", "varPSubtotal",
        ];

        private readonly List<(int Field, bool Selected, List<int> Indices, bool HasSubtotal)> _selections = [];

        public Format(XElement record)
        {
            Dxf = Xlsx.Integer(record, "dxfId") ?? -1;
            XElement? area = Xlsx.Child(record, "pivotArea");

            bool dataOnly = area is null || Xlsx.Flag(area, "dataOnly", true);
            bool labelOnly = area is not null && Xlsx.Flag(area, "labelOnly", false);
            Kind = dataOnly ? Kind.Data : labelOnly ? Kind.Label : Kind.None;

            foreach (XElement reference in Xlsx.Children(
                         Xlsx.Child(area, "references"), "reference"))
            {
                if (Xlsx.Attribute(reference, "field") is not { } stated) continue;
                if (!uint.TryParse(stated, out uint unsigned)) continue;

                // The data-layout field is spelled as an unsigned 4294967294, which read back as
                // a signed value is -2 — `sal_Int32(*rReference->mnField)`.
                _selections.Add((
                    unchecked((int)unsigned),
                    Xlsx.Flag(reference, "selected", true),
                    [.. Xlsx.Children(reference, "x").Select(x => Xlsx.Integer(x, "v") ?? 0)],
                    Array.Exists(SubtotalFlags, flag => Xlsx.Flag(reference, flag, false))));
            }
        }

        public int Dxf { get; }

        public Kind Kind { get; }

        /// <summary>
        /// <c>FormatOutput::prepare</c> — one entry per index of the widest reference.
        /// </summary>
        public IEnumerable<(Field[] Rows, Field[] Columns)> Entries(
            int[] rowDimensions, int[] columnDimensions)
        {
            // `nMaxNumberOfIndices` is *assigned* rather than maxed, so where two references
            // each name several members the last one decides how many entries the format makes.
            // Faithful to `FormatOutput::prepare` rather than to what it looks like it means; no
            // corpus pivot states two such references, so the two readings are not separated by
            // anything measurable here.
            int widest = 1;
            foreach ((_, _, List<int> indices, _) in _selections)
            {
                if (indices.Count > 1) widest = indices.Count;
            }

            for (int at = 0; at < widest; at++)
                yield return (Fields(rowDimensions, at), Fields(columnDimensions, at));
        }

        private Field[] Fields(int[] dimensions, int at)
        {
            Field[] fields = new Field[dimensions.Length];
            for (int index = 0; index < dimensions.Length; index++)
            {
                int dimension = dimensions[index];
                fields[index] = new Field(dimension, -1, false, true, false, false);
                foreach ((int field, bool selected, List<int> indices, bool subtotal) in _selections)
                {
                    if (field != dimension) continue;
                    fields[index] = new Field(
                        dimension,
                        indices.Count == 0 ? -1 : indices[indices.Count > 1 && at < indices.Count ? at : 0],
                        indices.Count == 0,
                        selected,
                        subtotal,
                        true);
                    break;
                }
            }

            return fields;
        }
    }

    /// <summary>
    /// One axis' output lines, with the member each field shows at each position.
    /// </summary>
    /// <remarks>
    /// The same walk <c>XlsxPivotGrid.ReadAxis</c> makes over <c>rowItems</c>/<c>colItems</c>,
    /// carrying the member index as well as the flags: an entry's <c>@r</c> is how many leading
    /// fields it inherits, its <c>&lt;x&gt;</c> children are the members it states, and a
    /// non-<c>data</c> <c>@t</c> makes the innermost one a subtotal. A field that continues takes
    /// the index of the last line that stated one, which is <c>fillLineAndFieldData</c>'s
    /// backward search.
    /// </remarks>
    private static Line[] Axis(IEnumerable<XElement> items, int[] dimensions)
    {
        List<XElement> entries = [.. items];
        Line[] lines = new Line[entries.Count];
        int fields = dimensions.Length;

        for (int at = 0; at < entries.Count; at++)
        {
            XElement item = entries[at];
            int repeated = Math.Clamp(Xlsx.Integer(item, "r") ?? 0, 0, Math.Max(fields, 0));
            List<int> stated = [.. Xlsx.Children(item, "x").Select(x => Xlsx.Integer(x, "v") ?? 0)];
            if (stated.Count == 0) stated.Add(0);
            int depth = Math.Min(repeated + stated.Count, fields);
            bool subtotal = (Xlsx.Attribute(item, "t") ?? "data") != "data";
            int inherited = subtotal ? Math.Min(repeated, Math.Max(depth - 1, 0)) : repeated;

            Member[] members = new Member[fields];
            for (int field = 0; field < fields; field++)
            {
                bool continues = field < inherited;
                bool isMember = subtotal ? depth > 0 && field == depth - 1
                                         : field >= repeated && field < depth;
                bool isSubtotal = subtotal && depth > 0 && field == depth - 1;

                int index = -1;
                if (dimensions[field] == DataDimension)
                {
                    // `rFieldData.nIndex = nMemberIndex` — the position, which for the
                    // data-layout dimension is the data field's own index.
                    index = at;
                }
                else if (field >= repeated && field - repeated < stated.Count)
                {
                    index = stated[field - repeated];
                }

                members[field] = new Member(dimensions[field], index, isMember, isSubtotal, continues);
            }

            lines[at] = new Line { Fields = members };
        }

        for (int at = 1; at < lines.Length; at++)
        {
            for (int field = 0; field < fields; field++)
            {
                if (!lines[at].Fields[field].Continues) continue;
                int back = at - 1;
                while (back >= 0 && lines[back].Fields[field].Continues) back--;
                if (back < 0) continue;
                lines[at].Fields[field] = lines[at].Fields[field] with
                {
                    Index = lines[back].Fields[field].Index,
                };
            }
        }

        return lines;
    }
}

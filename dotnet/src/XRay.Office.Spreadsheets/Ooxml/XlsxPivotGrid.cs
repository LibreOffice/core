using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// The border grid Calc draws around a pivot table, which no cell of the workbook states.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A pivot table's frame is generated at import, not read.</strong> Excel writes the
/// laid-out result into ordinary cells and puts the formatting in those cells' own styles; Calc
/// imports the <em>definition</em>, regenerates the output through <c>ScDPOutput</c>, and rules
/// the result itself in two widths of plain black — <c>SC_DP_FRAME_INNER_BOLD</c> 20 twips and
/// <c>SC_DP_FRAME_OUTER_BOLD</c> 40, <c>SC_DP_FRAME_COLOR</c> <c>Color(0,0,0)</c>
/// (<c>sc/source/core/data/dpoutput.cxx</c>:76-79 in the C++ tree read here, which declares
/// 27.2.0.0.alpha0+ and is not the reference binary's source). On a workbook whose pivot cells
/// state no border of their own, everything the reference draws around the table comes from
/// there and nothing at all comes from the file.
/// </para>
/// <para>
/// <strong>The member tree is in the file.</strong> Placing a line needs to know which rows a
/// row field's group covers, and <c>ScDPOutput</c> gets that from the data pilot's own result
/// sequences. It does not have to be recomputed: SpreadsheetML's <c>rowItems</c> and
/// <c>colItems</c> are that tree already laid out, one <c>&lt;i&gt;</c> per output row or
/// column, with <c>@r</c> naming how many leading fields the entry inherits from the one before
/// it and <c>@t</c> marking a subtotal. Read that way, an entry's <c>@r</c> is exactly
/// <c>MemberResultFlags::CONTINUE</c>, the fields it states are <c>HASMEMBER</c>, and a
/// non-<c>data</c> <c>@t</c> is <c>SUBTOTAL</c> on the innermost field it names —
/// see <see cref="Flags"/>.
/// </para>
/// <para>
/// <strong>Checked against 26.2.4.2's own resolved view, not derived from it.</strong> The
/// prediction is compared edge by edge, in both directions, against the <c>.fods</c> the
/// reference writes for the same workbook — an edge the reference states and the prediction does
/// not counts, which is what makes the score more than a precision. On <c>alle einzeln.xlsx</c>
/// it reproduces all <strong>9092</strong> bordered cells and all <strong>30250</strong> stated
/// edges exactly, with no edge missed and none invented; over all sixteen pivots it accepts —
/// in six of the corpus's eight worksheet-sourced pivot-bearing workbooks — <strong>31596</strong>
/// edges, 6324 generated cell styles and every generated indent agree and nothing disagrees. See
/// <c>dotnet/probes/pivot-gen-r107</c>.
/// </para>
/// <para>
/// <strong>What it declines to generate, and why each test is there.</strong> Calc's own layout
/// has to land on the same cells Excel wrote, because those are the cells being drawn.
/// <see cref="IsGeneratable"/> accepts a pivot only when the geometry Excel states is the
/// geometry <c>CalcSizes</c> (<c>:871-923</c>) would compute — one header row above the members,
/// the data starting one row per column field below them, and one column per row field — and
/// only when the cache is a worksheet range. A pivot over an external connection is not imported
/// as a data pilot at all, so the reference generates nothing for it: measured, the three
/// corpus workbooks whose caches are all <c>type="external"</c> produce <c>0</c>
/// <c>table:data-pilot-table</c> elements for their nine pivot parts, where the other eight
/// produce exactly one for each of their nineteen.
/// </para>
/// </remarks>
internal sealed class XlsxPivotGrid
{
    /// <summary>A line inside the table, <c>SC_DP_FRAME_INNER_BOLD</c>.</summary>
    private const int Inner = 20;

    /// <summary>A line on the table's own edge, <c>SC_DP_FRAME_OUTER_BOLD</c>.</summary>
    private const int Outer = 40;

    /// <summary><c>WEIGHT_BOLD</c>, on the hundred-point scale this tree keeps weights in.</summary>
    private const int BoldWeight = 700;

    /// <summary>One step of the indent a row field gives its member cell.</summary>
    /// <remarks>
    /// <c>o3tl::convert(13 * level, px, twip)</c> at <c>dpoutput.cxx</c>:1137 — thirteen pixels
    /// at 96 dpi is 195 twips, and the reference's own <c>.fods</c> writes it as
    /// <c>fo:margin-left="0.1354in"</c>. A field that is not the innermost takes one step while
    /// the drill-down buttons are on, and each compact field packed into the same column adds
    /// another for the fields inside it.
    /// </remarks>
    private const int IndentTwips = 195;

    /// <summary><c>MemberResultFlags</c>, <c>offapi/com/sun/star/sheet/MemberResultFlags.idl</c>.</summary>
    [Flags]
    private enum Flags
    {
        /// <summary>The position states no member of this field at all.</summary>
        None = 0,

        /// <summary>The field's member starts here.</summary>
        HasMember = 1,

        /// <summary>This position is the field's subtotal or the table's grand total.</summary>
        Subtotal = 2,

        /// <summary>The member above runs on into this position.</summary>
        Continue = 4,
    }

    /// <summary>
    /// The pivot styles that carry anything, <c>lcl_SetStyleById</c> (<c>dpoutput.cxx</c>:264).
    /// </summary>
    /// <remarks>
    /// The other four — <c>Value</c>, <c>Field</c>, <c>Corner</c> and <c>Top</c> — are created
    /// with an empty item set, so a cell taking one of them is drawn in whatever the workbook's
    /// default states and needs no entry here.
    /// </remarks>
    private enum Generated
    {
        /// <summary>Left justified. A row or column header's member cell.</summary>
        Category,

        /// <summary>Left justified and bold. A subtotal's row- or column-header strip.</summary>
        Title,

        /// <summary>Bold. The data cells of a subtotal's row or column.</summary>
        Result,
    }

    private readonly int _tabStartColumn;
    private readonly int _tabStartRow;
    private readonly int _dataStartColumn;
    private readonly int _dataStartRow;
    private readonly int _tabEndColumn;
    private readonly int _tabEndRow;
    private readonly int _clearedFirstRow;
    private readonly int _clearedLastRow;
    private readonly int _clearedFirstColumn;
    private readonly int _clearedLastColumn;
    private readonly Dictionary<(int Row, int Column), int[]> _edges = [];
    private readonly Dictionary<(int Row, int Column), Generated> _styles = [];
    private readonly Dictionary<(int Row, int Column), int> _indents = [];
    private readonly List<int> _columns = [];
    private readonly List<int> _rows = [];
    private readonly HashSet<int> _columnSeen = [];
    private readonly HashSet<int> _rowSeen = [];

    private XlsxPivotGrid(
        int tabStartColumn, int tabStartRow, int dataStartColumn, int dataStartRow,
        int tabEndColumn, int tabEndRow, SheetRange stated, int pageStartRow)
    {
        _tabStartColumn = tabStartColumn;
        _tabStartRow = tabStartRow;
        _dataStartColumn = dataStartColumn;
        _dataStartRow = dataStartRow;
        _tabEndColumn = tabEndColumn;
        _tabEndRow = tabEndRow;

        // The reference empties two rectangles before ScDPOutput writes anything: the range
        // Excel stated, through `clearContents(… HARDATTR | STYLES …)`
        // (`sc/source/filter/oox/pivottablebuffer.cxx`:1331-1336), and the range it computed
        // for itself, through `DeleteAreaTab(…, InsertDeleteFlags::ALL)` (`dpoutput.cxx`:1226)
        // from `maStartPos` — which is the stated start moved up over the page fields — to the
        // table's own end. Their union is what a cell in here resolves through.
        _clearedFirstRow = Math.Min(stated.FirstRow, pageStartRow);
        _clearedLastRow = Math.Max(stated.LastRow, tabEndRow);
        _clearedFirstColumn = Math.Min(stated.FirstColumn, tabStartColumn);
        _clearedLastColumn = Math.Max(stated.LastColumn, tabEndColumn);
    }

    /// <summary>
    /// Rules every pivot table on one sheet into that sheet's decoration.
    /// </summary>
    /// <remarks>
    /// Returns the decoration to use, which is <em>not</em> always the one passed in.
    /// <see cref="SheetFormatting.Empty"/> is a shared singleton that
    /// <c>XlsxCellDecoration.Read</c> returns for every sheet stating nothing, and this
    /// is the first writer in the tree to want to add to a sheet's decoration after it has been
    /// read — so writing into that instance would put one sheet's pivot grid on every plain
    /// sheet in the process. Measured on <c>alle einzeln.xlsx</c>, whose <c>Pivot</c> sheet
    /// states no decoration at all: the grid appeared on 36 pages of the other sheet and cost
    /// 276 of summed ink there.
    /// </remarks>
    /// <param name="pivots">The sheet's pivot table parts, with their cache source kinds.</param>
    /// <param name="formatting">The decoration the generated borders are merged into.</param>
    /// <param name="formats">The text formats the generated styles are laid over.</param>
    /// <param name="cleared">
    /// The format an emptied cell falls back to: the <c>Normal</c> <c>cellStyleXf</c>, which is
    /// Calc's <c>Default</c> cell style. Not the sheet's default <c>cellXf</c>, which is a
    /// different entry — <c>XlsxCellFormats.NormalStyleXf</c> records the probe workbook that
    /// separates them and what 26.2.4.2 answered on it.
    /// </param>
    /// <param name="differences">
    /// The workbook's <c>dxfs</c>, which the pivot's own <c>&lt;format&gt;</c> records index. They
    /// are applied last, after the generated styles, exactly as <c>maFormatOutput.apply</c> is the
    /// last statement of <c>ScDPOutput::Output</c> — see <see cref="XlsxPivotFormats"/>.
    /// </param>
    public static (SheetFormatting Formatting, SheetCellFormats Formats) Apply(
        IReadOnlyList<XlsxPivotTable> pivots, SheetFormatting formatting, SheetCellFormats formats,
        SheetCellFormat cleared, IReadOnlyList<XlsxPivotFormats.Difference> differences)
    {
        ArgumentNullException.ThrowIfNull(pivots);
        ArgumentNullException.ThrowIfNull(formatting);
        ArgumentNullException.ThrowIfNull(formats);
        ArgumentNullException.ThrowIfNull(cleared);
        ArgumentNullException.ThrowIfNull(differences);

        Dictionary<(int Row, int Column), SheetPivotStyle> styles = [];
        foreach (XlsxPivotTable pivot in pivots)
        {
            if (Build(pivot) is not { } grid) continue;
            if (ReferenceEquals(formatting, SheetFormatting.Empty)) formatting = new SheetFormatting();
            formatting.ClearBackgrounds(
                grid._clearedFirstRow, grid._clearedLastRow,
                grid._clearedFirstColumn, grid._clearedLastColumn);
            grid.MergeInto(formatting);
            grid.CollectStyles(styles, cleared);
            grid.LayFormatsOver(pivot.Root, differences, styles, formatting);
        }

        return (formatting, formats.WithPivotStyles(styles));
    }

    /// <summary>
    /// Lays the pivot's own <c>&lt;format&gt;</c> records over the generated styles.
    /// </summary>
    /// <remarks>
    /// Last, because that is where <c>maFormatOutput.apply</c> sits (<c>dpoutput.cxx</c>:1190),
    /// and cell by cell in document order, because <c>ApplyPattern</c> merges each record's own
    /// items into whatever is already there rather than replacing the lot.
    /// </remarks>
    private void LayFormatsOver(
        XElement root,
        IReadOnlyList<XlsxPivotFormats.Difference> differences,
        Dictionary<(int Row, int Column), SheetPivotStyle> styles,
        SheetFormatting formatting)
    {
        if (differences.Count == 0) return;

        XlsxPivotFormats.Apply(
            root, differences,
            new XlsxPivotFormats.Geometry(_tabStartColumn, _dataStartColumn, _dataStartRow),
            (column, row, difference) =>
            {
                if (row < _clearedFirstRow || row > _clearedLastRow) return;
                if (column < _clearedFirstColumn || column > _clearedLastColumn) return;

                if (!difference.Text.IsNone && styles.TryGetValue((row, column), out SheetPivotStyle style))
                {
                    SheetPivotDxf over = style.Dxf;
                    styles[(row, column)] = style with
                    {
                        Dxf = new SheetPivotDxf
                        {
                            FontFamily = difference.Text.FontFamily ?? over.FontFamily,
                            DeclaredFontClass = difference.Text.FontFamily is not null
                                ? difference.Text.DeclaredFontClass
                                : over.DeclaredFontClass,
                            FontSize = difference.Text.FontSize ?? over.FontSize,
                            Colour = difference.Text.Colour ?? over.Colour,
                            FontWeight = difference.Text.FontWeight ?? over.FontWeight,
                        },
                    };
                }

                if (difference.Fill is not { } fill) return;
                SheetCellDecoration stated = formatting.At(row, column);
                if (stated.Background == fill) return;
                formatting.SetCell(row, column, formatting.Intern(stated with { Background = fill }));
            });
    }

    /// <summary>
    /// Works out where every generated line falls, or null when the pivot is one the reference
    /// would not lay out over the cells Excel wrote.
    /// </summary>
    /// <param name="pivot">One pivot table part.</param>
    private static XlsxPivotGrid? Build(XlsxPivotTable pivot)
    {
        XElement root = pivot.Root;
        if (Xlsx.Child(root, "location") is not { } location) return null;
        if (!SheetAddress.TryParseRange(Xlsx.Attribute(location, "ref"), out SheetRange area)) return null;
        if (!area.IsValid) return null;

        int firstHeaderRow = Xlsx.Integer(location, "firstHeaderRow") ?? 1;
        int firstDataRow = Xlsx.Integer(location, "firstDataRow") ?? 1;
        int firstDataColumn = Xlsx.Integer(location, "firstDataCol") ?? 0;

        List<XElement> rowFields = [.. Xlsx.Children(Xlsx.Child(root, "rowFields"), "field")];
        List<XElement> columnFields = [.. Xlsx.Children(Xlsx.Child(root, "colFields"), "field")];
        List<XElement> rowItems = [.. Xlsx.Children(Xlsx.Child(root, "rowItems"), "i")];
        List<XElement> columnItems = [.. Xlsx.Children(Xlsx.Child(root, "colItems"), "i")];

        // bColumnFieldIsDataOnly, dpoutput.cxx:1205 — a pivot whose only column field is the
        // data-layout placeholder and which has one data field has no column field in Calc at
        // all: the member result is empty, `bSkip` drops the dimension (:595-596) and the
        // caption goes in the corner cell instead of on a row of its own. Excel writes the row
        // anyway, so the two layouts differ by one row and the test below refuses the pivot.
        int columnFieldCount = columnFields.Count;
        if (columnFieldCount == 1
            && (Xlsx.Integer(columnFields[0], "x") ?? 0) < 0
            && Xlsx.Children(Xlsx.Child(root, "dataFields"), "dataField").Count() <= 1)
        {
            columnFieldCount = 0;
        }

        // maRowCompactFlags / mbHasCompactRowField, pivottablebuffer.cxx:296 and
        // dpoutput.cxx:596-611 — a row field is compact when `subtotalTop`, `outline` and
        // `compact` all hold, each defaulting to true.
        bool[] compactRowFields = CompactRowFields(root, rowFields);
        bool hasCompactRowField = Array.IndexOf(compactRowFields, true) >= 0;

        if (!IsGeneratable(
                pivot, firstHeaderRow, firstDataRow, firstDataColumn,
                rowFields, compactRowFields, columnFieldCount, rowItems, columnItems))
        {
            return null;
        }

        // `nMinIndentLevel = mbExpandCollapse ? 1 : 0`, dpoutput.cxx:1136, and
        // `pSaveData->SetExpandCollapse(maDefModel.mbShowDrill)`,
        // sc/source/filter/oox/pivottablebuffer.cxx:1366 — the whole of the indent a
        // non-innermost row field gives its member is the drill-down step, so a pivot that
        // states `showDrill="0"` has none. Measured: `DynamicBubbleChart.xlsx` states it and
        // its five row fields, and the reference's own `.fods` of that workbook writes no
        // `fo:margin-left="0.1354in"` anywhere.
        bool showDrill = Xlsx.Flag(root, "showDrill", true);

        // Excel's `ref` excludes the filter rows; Calc's does not. `PivotTable::finalizeImport`
        // (`sc/source/filter/oox/pivottablebuffer.cxx`:1421-1426) puts the data pilot's own
        // start `maPageFields.size() + 1` rows above the stated range — clamped at row one —
        // and `CalcSizes` (`dpoutput.cxx`:892-907) then puts the table back that many rows
        // below it, so the table lands on the stated range exactly unless the clamp bit.
        int pageFields = Xlsx.Children(Xlsx.Child(root, "pageFields"), "pageField").Count();
        int pageStartRow = pageFields > 0 ? Math.Max(area.FirstRow - pageFields - 1, 0) : area.FirstRow;
        int tabStartColumn = area.FirstColumn;
        int tabStartRow = pageFields > 0 ? pageStartRow + pageFields + 1 : area.FirstRow;
        int memberStartRow = tabStartRow + firstHeaderRow;
        int dataStartRow = tabStartRow + firstDataRow;
        int dataStartColumn = tabStartColumn + firstDataColumn;

        // CalcSizes (dpoutput.cxx:912-923) takes the last row and column from the result's own
        // extent, not from the stated range: DynamicBubbleChart's ref stops one column short of
        // the data it describes, and the reference's outer rule is on the column the data ends
        // in.
        int rowCount = rowItems.Count;
        int columnCount = columnItems.Count;
        int tabEndRow = rowCount > 0 ? dataStartRow + rowCount - 1 : dataStartRow;
        int tabEndColumn = columnCount > 0 ? dataStartColumn + columnCount - 1 : dataStartColumn;

        XlsxPivotGrid grid = new(
            tabStartColumn, tabStartRow, dataStartColumn, dataStartRow, tabEndColumn, tabEndRow,
            area, pageStartRow);

        grid.PageFields(pageFields, tabStartColumn, pageStartRow);
        grid.ColumnHeaders(columnFieldCount, ReadAxis(columnItems, columnFieldCount, columnCount),
            memberStartRow, columnCount, hasCompactRowField);
        grid.RowHeaders(
            rowFields.Count, ReadAxis(rowItems, rowFields.Count, rowCount), rowCount, showDrill,
            compactRowFields, hasCompactRowField);
        grid.DataArea();
        return grid;
    }

    /// <summary>
    /// Whether Calc's own output would land on the cells Excel wrote.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Each test is one line of <c>CalcSizes</c> (<c>dpoutput.cxx</c>:871-923) read as a question
    /// about the stated geometry: <c>mnMemberStartRow = mnTabStartRow + mnHeaderSize</c>,
    /// <c>mnDataStartRow = mnMemberStartRow + mpColFields.size()</c> and
    /// <c>mnDataStartCol = mnMemberStartCol + GetColumnsForRowFields()</c> (<c>:854-868</c>).
    /// <c>mnHeaderSize</c> is <c>0</c> when the header is hidden and <c>1</c> otherwise, and the
    /// OOXML import ties the first to the stated first header row outright —
    /// <c>mpDPObject-&gt;SetHideHeader(maLocationModel.mnFirstHeaderRow == 0)</c>
    /// (<c>sc/source/filter/oox/pivottablebuffer.cxx</c>:1368) — so
    /// <c>mnHeaderSize == firstHeaderRow</c> for the two values that can arise. The third,
    /// <c>mnHeaderSize = 2</c> for a grid header layout (<c>:886</c>), cannot: <c>SetHeaderLayout</c>
    /// is called only by the BIFF and ODF importers, never by <c>sc/source/filter/oox</c>.
    /// </para>
    /// <para>
    /// A compact row field shares its column with the field outside it, so
    /// <c>GetColumnsForRowFields</c> returns fewer columns than there are fields; the stated
    /// <c>firstDataCol</c> is checked against that packed count rather than against the field
    /// count. What the packing changes besides the count is that the button row takes
    /// <c>MultiFieldCell</c> rather than <c>FieldCell</c> and so is not boxed
    /// (<c>:1087-1090</c>), and that each packed field adds an indent step to the fields inside
    /// it (<c>:1135-1137</c>).
    /// </para>
    /// <para>
    /// The data-layout dimension on the row axis is declined outright. With one data field its
    /// member result is empty, so <c>bSkip</c> drops the dimension (<c>:595-596</c>) and Calc
    /// lays out one row-label column fewer than Excel wrote. No document in the corpus states
    /// it — the census in <c>dotnet/probes/pivot-gen-r107</c> reports <c>rowDataPH=False</c> for
    /// all twenty-eight pivots of the eleven pivot-bearing workbooks — so this is a refusal to
    /// act where there is nothing to check the result against, not a modelled case.
    /// </para>
    /// </remarks>
    private static bool IsGeneratable(
        XlsxPivotTable pivot, int firstHeaderRow, int firstDataRow, int firstDataColumn,
        List<XElement> rowFields, bool[] compactRowFields, int columnFieldCount,
        List<XElement> rowItems, List<XElement> columnItems)
    {
        if (!pivot.HasWorksheetCache) return false;
        if (rowItems.Count == 0 || columnItems.Count == 0) return false;
        if (firstHeaderRow is not (0 or 1)) return false;
        if (firstDataRow != firstHeaderRow + columnFieldCount) return false;
        if (firstDataColumn != RowLabelColumns(compactRowFields) || firstDataColumn == 0)
            return false;

        int dataFields = Xlsx.Children(Xlsx.Child(pivot.Root, "dataFields"), "dataField").Count();
        if (dataFields <= 1 && rowFields.Exists(field => (Xlsx.Integer(field, "x") ?? -1) < 0))
            return false;

        return true;
    }

    /// <summary>
    /// Which of a pivot's row fields Calc lays out compactly, <c>maRowCompactFlags</c>.
    /// </summary>
    /// <remarks>
    /// <c>maModel.mbCompact = mbSubtotalTop &amp;&amp; mbOutline &amp;&amp; compact</c>
    /// (<c>sc/source/filter/oox/pivottablebuffer.cxx</c>:296) becomes
    /// <c>DataPilotFieldLayoutMode::COMPACT_LAYOUT</c> (<c>:812-814</c>), which the
    /// <c>ScDPOutput</c> constructor reads back into <c>maRowCompactFlags</c>
    /// (<c>dpoutput.cxx</c>:596-611). Each of the three attributes defaults to true, so a field
    /// stating none of them is compact.
    /// </remarks>
    private static bool[] CompactRowFields(XElement root, List<XElement> rowFields)
    {
        List<XElement> pivotFields = [.. Xlsx.Children(Xlsx.Child(root, "pivotFields"), "pivotField")];
        bool[] compact = new bool[rowFields.Count];
        for (int at = 0; at < rowFields.Count; at++)
        {
            int index = Xlsx.Integer(rowFields[at], "x") ?? -1;
            if (index < 0 || index >= pivotFields.Count) continue;
            XElement pivotField = pivotFields[index];
            compact[at] = Xlsx.Flag(pivotField, "subtotalTop", true)
                          && Xlsx.Flag(pivotField, "outline", true)
                          && Xlsx.Flag(pivotField, "compact", true);
        }

        return compact;
    }

    /// <summary>
    /// <c>GetColumnsForRowFields</c>, <c>dpoutput.cxx</c>:854-868.
    /// </summary>
    /// <remarks>
    /// A run of compact row fields is packed into one column: the count is one per non-compact
    /// field, plus one more when the innermost field is compact. With no compact field at all
    /// it is simply the field count, which is what Excel writes as <c>firstDataCol</c>.
    /// </remarks>
    private static int RowLabelColumns(bool[] compactRowFields)
    {
        if (Array.IndexOf(compactRowFields, true) < 0) return compactRowFields.Length;

        int columns = 0;
        foreach (bool compact in compactRowFields)
        {
            if (!compact) columns++;
        }

        if (compactRowFields.Length > 0 && compactRowFields[^1]) columns++;
        return columns;
    }

    /// <summary>
    /// One axis's <c>MemberResult</c> flags, per field and position.
    /// </summary>
    /// <remarks>
    /// <c>ScDPResultMember::FillMemberResults</c> (<c>sc/source/core/data/dptabres.cxx</c>:1425)
    /// sets <c>HASMEMBER</c> where a member starts, <c>CONTINUE</c> on every position it runs
    /// through, and on a subtotal <c>HASMEMBER | SUBTOTAL</c> with <c>CONTINUE</c> cleared —
    /// on the level whose subtotal it is, and on nothing deeper. An <c>&lt;i&gt;</c> states the
    /// same three things: <c>@r</c> leading fields are continuations, the <c>&lt;x&gt;</c>
    /// children after them start members, and a <c>@t</c> other than <c>data</c> makes the last
    /// of those the subtotal.
    /// </remarks>
    /// <param name="items">The axis's <c>rowItems</c> or <c>colItems</c> entries.</param>
    /// <param name="fields">How many fields the axis has.</param>
    /// <param name="count">How many positions it has.</param>
    private static Flags[][] ReadAxis(List<XElement> items, int fields, int count)
    {
        Flags[][] flags = new Flags[Math.Max(fields, 0)][];
        for (int field = 0; field < flags.Length; field++) flags[field] = new Flags[count];

        for (int position = 0; position < count && position < items.Count; position++)
        {
            XElement item = items[position];
            int repeated = Math.Clamp(Xlsx.Integer(item, "r") ?? 0, 0, fields);
            int stated = Math.Max(Xlsx.Children(item, "x").Count(), 1);
            int depth = Math.Min(repeated + stated, fields);
            bool subtotal = (Xlsx.Attribute(item, "t") ?? "data") != "data";

            int inherited = subtotal ? Math.Min(repeated, Math.Max(depth - 1, 0)) : repeated;
            for (int field = 0; field < inherited; field++) flags[field][position] |= Flags.Continue;

            if (subtotal)
            {
                if (depth > 0) flags[depth - 1][position] |= Flags.HasMember | Flags.Subtotal;
            }
            else
            {
                for (int field = repeated; field < depth; field++)
                    flags[field][position] |= Flags.HasMember;
            }
        }

        return flags;
    }

    /// <summary>
    /// <c>OutputBlockFrame</c>, <c>dpoutput.cxx</c>:217.
    /// </summary>
    /// <remarks>
    /// Only the block's own edges are written. <c>ScAttrArray::ApplyFrame</c> takes the box's
    /// left line for the first column and the inner vertical for the rest, and the inner
    /// vertical is marked invalid here, so an interior cell keeps whatever it had; the same for
    /// the horizontals unless <paramref name="horizontal"/> is set, which is the one case an
    /// interior row gets ruled.
    /// </remarks>
    private void Block(int startColumn, int startRow, int endColumn, int endRow, bool horizontal = false)
    {
        if (endColumn < startColumn || endRow < startRow) return;

        int left = startColumn == _tabStartColumn ? Outer : Inner;
        int top = startRow == _tabStartRow ? Outer : Inner;
        int right = endColumn == _tabEndColumn ? Outer : Inner;
        int bottom = endRow == _tabEndRow ? Outer : Inner;

        for (int row = startRow; row <= endRow; row++)
        {
            Edge(row, startColumn, 0, left);
            Edge(row, endColumn, 1, right);
        }

        for (int column = startColumn; column <= endColumn; column++)
        {
            Edge(startRow, column, 2, top);
            Edge(endRow, column, 3, bottom);
        }

        if (!horizontal) return;

        for (int row = startRow; row <= endRow; row++)
        {
            for (int column = startColumn; column <= endColumn; column++)
            {
                if (row != startRow) Edge(row, column, 2, Inner);
                if (row != endRow) Edge(row, column, 3, Inner);
            }
        }
    }

    /// <summary><c>lcl_SetFrame</c>, <c>dpoutput.cxx</c>:297 — all four edges of one cell.</summary>
    private void Box(int column, int row, int width)
    {
        for (int side = 0; side < 4; side++) Edge(row, column, side, width);
    }

    private void Edge(int row, int column, int side, int width)
    {
        if (row < 0 || column < 0) return;
        if (!_edges.TryGetValue((row, column), out int[]? cell))
        {
            cell = new int[4];
            _edges[(row, column)] = cell;
        }

        cell[side] = width;
    }

    private void AddRow(int row)
    {
        if (_rowSeen.Add(row)) _rows.Add(row);
    }

    private void AddColumn(int column)
    {
        if (_columnSeen.Add(column)) _columns.Add(column);
    }

    /// <summary>
    /// <c>outputPageFields</c>, <c>dpoutput.cxx</c>:969 — a hairline box round each filter value.
    /// </summary>
    /// <remarks>
    /// One page field to a row at <c>maStartPos.Row() + nField + (mbDoFilter ? 1 : 0)</c>, and
    /// <c>mbDoFilter</c> is false for every OOXML pivot — <c>pSaveData-&gt;SetFilterButton(false)</c>
    /// (<c>sc/source/filter/oox/pivottablebuffer.cxx</c>:1365), which
    /// <c>ScDPObject::CreateOutput</c> reads back as <c>bFilterButton</c>
    /// (<c>sc/source/core/data/dpobject.cxx</c>:531). The box is <c>lcl_SetFrame(…, 20)</c> on
    /// the value cell beside the caption, and nothing at all on the caption.
    /// </remarks>
    private void PageFields(int count, int tabStartColumn, int pageStartRow)
    {
        for (int field = 0; field < count; field++)
            Box(tabStartColumn + 1, pageStartRow + field, Inner);
    }

    /// <summary><c>outputColumnHeaders</c>, <c>dpoutput.cxx</c>:1002.</summary>
    private void ColumnHeaders(
        int fields, Flags[][] flags, int memberStartRow, int count, bool hasCompactRowField)
    {
        for (int field = 0; field < fields; field++)
        {
            // `if (!mbHasCompactRowField || nNumColFields == 1) FieldCell(…) else if (!nField)
            // MultiFieldCell(…)`, dpoutput.cxx:1010-1015. FieldCell boxes its cell,
            // MultiFieldCell does not — and it is written for the first field only.
            if (memberStartRow > _tabStartRow && (!hasCompactRowField || fields == 1))
                Box(_dataStartColumn + field, _tabStartRow, Inner);

            int rowPos = memberStartRow + field;
            for (int position = 0; position < count; position++)
            {
                int columnPos = _dataStartColumn + position;
                Flags flag = flags[field][position];

                if (flag.HasFlag(Flags.Subtotal))
                {
                    // HeaderCell, dpoutput.cxx:748 — a subtotal frames its own strip down to the
                    // data, which is the one frame outputColumnHeaders itself never draws, and
                    // styles that strip and the data under it.
                    Block(columnPos, memberStartRow + field, columnPos, _dataStartRow - 1);
                    Style(columnPos, memberStartRow + field, columnPos, _dataStartRow - 1, Generated.Title);
                    Style(columnPos, _dataStartRow, columnPos, _tabEndRow, Generated.Result);
                    AddColumn(columnPos);
                    continue;
                }

                if (!flag.HasFlag(Flags.HasMember)) continue;

                int end = position;
                while (end + 1 < count && flags[field][end + 1].HasFlag(Flags.Continue)) end++;
                int endColumnPos = _dataStartColumn + end;

                if (field + 1 >= fields)
                {
                    Style(columnPos, rowPos, columnPos, _dataStartRow - 1, Generated.Category);
                    continue;
                }

                if (field + 2 == fields)
                {
                    AddColumn(columnPos);
                    if (columnPos + 1 == endColumnPos)
                        Block(columnPos, rowPos, endColumnPos, rowPos + 1, true);
                }
                else
                {
                    Block(columnPos, rowPos, endColumnPos, rowPos);
                }

                Style(columnPos, rowPos, endColumnPos, _dataStartRow - 1, Generated.Category);
            }

            if (field == 0 && fields == 1 && memberStartRow > _tabStartRow)
                Block(_dataStartColumn, _tabStartRow, _tabEndColumn, memberStartRow - 1);
        }
    }

    /// <summary><c>outputRowHeader</c>, <c>dpoutput.cxx</c>:1075.</summary>
    /// <remarks>
    /// <c>vbSetBorder</c> is why the full-width frame is drawn once per row and not once per
    /// field: the outermost field that has a member on a row claims it, and the fields inside it
    /// only rule their own column.
    /// </remarks>
    private void RowHeaders(
        int fields, Flags[][] flags, int count, bool showDrill,
        bool[] compactRowFields, bool hasCompactRowField)
    {
        bool[] framed = new bool[count];
        int columnOffset = 0;   // nFieldColOffset — only a non-compact field takes a column
        int indentLevel = 0;    // nFieldIndentLevel — how deep this field sits in its column

        for (int field = 0; field < fields; field++)
        {
            if (!hasCompactRowField || fields == 1)
                Box(_tabStartColumn + field, _dataStartRow - 1, Inner);

            int columnPos = _tabStartColumn + columnOffset;

            // `bLast = mnRowDims == (nField + 1)` and
            // `nIndent = 13 * (bLast ? nFieldIndentLevel : nMinIndentLevel + nFieldIndentLevel)`
            // px, dpoutput.cxx:1135-1137. bLast counts fields, not the columns they are packed
            // into, so the innermost field of a packed column drops the drill step but keeps
            // whatever depth the fields outside it in that column have added.
            bool last = field + 1 == fields;
            int step = last ? 0 : showDrill ? 1 : 0;
            int indent = IndentTwips * (step + indentLevel);

            for (int position = 0; position < count; position++)
            {
                int rowPos = _dataStartRow + position;
                Flags flag = flags[field][position];

                if (flag.HasFlag(Flags.Subtotal))
                {
                    // HeaderCell, dpoutput.cxx:788.
                    Block(columnPos, rowPos, _dataStartColumn - 1, rowPos);
                    Style(columnPos, rowPos, _dataStartColumn - 1, rowPos, Generated.Title);
                    Style(_dataStartColumn, rowPos, _tabEndColumn, rowPos, Generated.Result);
                    AddRow(rowPos);
                    continue;
                }

                if (!flag.HasFlag(Flags.HasMember)) continue;
                if (field + 1 >= fields)
                {
                    Style(columnPos, rowPos, _dataStartColumn - 1, rowPos, Generated.Category);
                }
                else
                {
                    int end = position;
                    while (end + 1 < count && flags[field][end + 1].HasFlag(Flags.Continue)) end++;
                    int endRowPos = _dataStartRow + end;

                    AddRow(rowPos);
                    if (!framed[position])
                    {
                        Block(columnPos, rowPos, _tabEndColumn, endRowPos);
                        framed[position] = true;
                    }

                    Block(columnPos, rowPos, columnPos, endRowPos);
                    if (field == fields - 2)
                        Block(columnPos + 1, rowPos, columnPos + 1, endRowPos);

                    Style(columnPos, rowPos, _dataStartColumn - 1, endRowPos, Generated.Category);
                }

                // The indent is applied to every member cell, the innermost field's included:
                // it is outside the branch above in dpoutput.cxx, and only bLast keeping the
                // step at zero makes it nothing there when no field is packed.
                if (indent > 0) _indents[(rowPos, columnPos)] = indent;
            }

            if (compactRowFields[field]) indentLevel++;
            else
            {
                columnOffset++;
                indentLevel = 0;
            }
        }
    }

    /// <summary><c>ScDPOutputImpl::OutputDataArea</c>, <c>dpoutput.cxx</c>:127.</summary>
    /// <remarks>
    /// When every data row was added the whole strip is one block ruled horizontally; when not,
    /// the loop steps two rows at a time starting at <c>nCol % 2</c>, which is where the
    /// checkerboard in a part-ruled pivot comes from.
    /// </remarks>
    private void DataArea()
    {
        AddRow(_dataStartRow);
        AddColumn(_dataStartColumn);

        List<int> columns = [.. _columns, _tabEndColumn + 1];
        List<int> rows = [.. _rows, _tabEndRow + 1];
        columns.Sort();
        rows.Sort();

        bool allRows = _tabEndRow - _dataStartRow + 2 == rows.Count;

        for (int index = 0; index < columns.Count - 1; index++)
        {
            if (allRows)
            {
                Block(columns[index], rows[0], columns[index + 1] - 1, rows[^1] - 1, true);
                continue;
            }

            if (index < columns.Count - 2)
            {
                for (int at = index % 2; at < rows.Count - 2; at += 2)
                    Block(columns[index], rows[at], columns[index + 1] - 1, rows[at + 1] - 1);
                if (rows.Count >= 2)
                    Block(columns[index], rows[^2], columns[index + 1] - 1, rows[^1] - 1);
            }
            else
            {
                for (int at = 0; at < rows.Count - 1; at++)
                    Block(columns[index], rows[at], columns[index + 1] - 1, rows[at + 1] - 1);
            }
        }

        if (_tabStartColumn != _dataStartColumn)
        {
            if (_tabStartRow != _dataStartRow)
                Block(_tabStartColumn, _tabStartRow, _dataStartColumn - 1, _dataStartRow - 1);
            Block(_tabStartColumn, _dataStartRow, _dataStartColumn - 1, _tabEndRow);
        }

        Block(_dataStartColumn, _tabStartRow, _tabEndColumn, _dataStartRow - 1);
    }

    /// <summary>
    /// <c>lcl_SetStyleById</c> over a rectangle, <c>dpoutput.cxx</c>:264.
    /// </summary>
    /// <remarks>
    /// Last writer wins, because <c>ApplyStyleAreaTab</c> replaces a cell's style rather than
    /// merging into it — so the order these are called in is the order the reference calls them
    /// in, and a subtotal's <c>Title</c> survives only because no later member's <c>Category</c>
    /// covers a subtotal row.
    /// </remarks>
    private void Style(int startColumn, int startRow, int endColumn, int endRow, Generated style)
    {
        if (endColumn < startColumn || endRow < startRow) return;

        for (int row = startRow; row <= endRow; row++)
        {
            for (int column = startColumn; column <= endColumn; column++)
                _styles[(row, column)] = style;
        }
    }

    /// <summary>
    /// Everything the generated styles and the row-header indent change about one sheet's text.
    /// </summary>
    private void CollectStyles(
        Dictionary<(int Row, int Column), SheetPivotStyle> into, SheetCellFormat cleared)
    {
        // Every cell of the emptied rectangle states these properties outright, so the overlay
        // replaces what the workbook put on a pivot cell rather than merging with it — which is
        // what the two clearing calls above do. What it replaces them with is the Default cell
        // style and not nothing, and a workbook whose Default states an alignment or an indent
        // still states it afterwards. `049_Expenses_calculator` is that workbook — 47 cells of
        // its pivot resolve their left justification and indent that way and no cell of the
        // range states an `s` of its own, and the reference keeps all 47.
        //
        // `Cleared` carries the whole base format rather than four more nullable fields, because
        // the face, the declared class, the size and the colour are all taken from it whole; the
        // three below are a differential the generated style may overwrite, and `Dxf` is a fifth
        // laid over everything by `LayFormatsOver`. `SheetPivotStyle`'s remarks give the
        // cell-for-cell score for each property, and the null experiment that says the clearing
        // reaches all four.
        SheetPivotStyle bare = new()
        {
            FontWeight = cleared.FontWeight,
            Horizontal = cleared.Horizontal,
            Indent = cleared.Indent,
            Cleared = cleared,
        };

        for (int row = _clearedFirstRow; row <= _clearedLastRow; row++)
        {
            for (int column = _clearedFirstColumn; column <= _clearedLastColumn; column++)
                into[(row, column)] = bare;
        }

        foreach (KeyValuePair<(int Row, int Column), Generated> entry in _styles)
        {
            SheetPivotStyle style = into.GetValueOrDefault(entry.Key, bare);
            into[entry.Key] = entry.Value switch
            {
                Generated.Category => style with { Horizontal = SheetHorizontalAlignment.Left },
                Generated.Title => style with
                {
                    Horizontal = SheetHorizontalAlignment.Left,
                    FontWeight = BoldWeight,
                },
                _ => style with { FontWeight = BoldWeight },
            };
        }

        foreach (KeyValuePair<(int Row, int Column), int> entry in _indents)
        {
            SheetPivotStyle style = into.GetValueOrDefault(entry.Key, bare);
            into[entry.Key] = style with { Indent = Length.FromTwips(entry.Value) };
        }
    }

    /// <summary>
    /// Lays the generated edges over what each cell already states.
    /// </summary>
    /// <remarks>
    /// Edge by edge rather than cell by cell, because that is what
    /// <c>ScAttrArray::ApplyFrame</c> does: a frame call writes the edges it names and leaves
    /// the rest of the cell's border alone.
    /// <para>
    /// What the reference leaves alone is nothing, though. <c>PivotTable::finalizeImport</c>
    /// clears the stated range first — <c>clearContents(VALUE | … | HARDATTR | STYLES | …)</c>,
    /// <c>sc/source/filter/oox/pivottablebuffer.cxx</c>:1331-1336 — so a border the workbook
    /// states inside a pivot's own range is gone before <c>ScDPOutput</c> runs, and the
    /// reference's behaviour is replacement, not this merge. The two are not distinguished by
    /// anything in the corpus: of the nineteen pivot ranges in the eight worksheet-sourced
    /// pivot-bearing workbooks, <c>0</c> contain a cell whose <c>cellXfs</c> entry states any
    /// border at all (<c>probes/pivot-gen-r107/statedborders.py</c>), against 15 of the 19 that
    /// state an alignment. Merging is the narrower change of the two, so it is the one made.
    /// </para>
    /// </remarks>
    private void MergeInto(SheetFormatting formatting)
    {
        foreach (KeyValuePair<(int Row, int Column), int[]> entry in _edges)
        {
            int[] edges = entry.Value;
            SheetCellDecoration stated = formatting.At(entry.Key.Row, entry.Key.Column);
            SheetCellBorders borders = stated.Borders;

            SheetCellDecoration merged = stated with
            {
                Borders = new SheetCellBorders(
                    Rule(edges[0]) ?? borders.Left,
                    Rule(edges[1]) ?? borders.Right,
                    Rule(edges[2]) ?? borders.Top,
                    Rule(edges[3]) ?? borders.Bottom),
            };

            if (merged == stated) continue;
            formatting.SetCell(entry.Key.Row, entry.Key.Column, formatting.Intern(merged));
        }
    }

    private static SheetBorder? Rule(int twips)
        => twips <= 0 ? null : SheetBorder.Line(Length.FromTwips(twips), Colour.Black);
}

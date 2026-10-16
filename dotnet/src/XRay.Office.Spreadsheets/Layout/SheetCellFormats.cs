namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// One sheet's cell formats, kept apart from its cells.
/// </summary>
/// <remarks>
/// <para>
/// The second of the two things that make a spreadsheet unlike the other families: content and
/// formatting are stored independently, and merging them into per-cell objects is what makes a
/// sheet with one uniformly-formatted million-cell region expensive. So a cell holds an
/// <em>index</em> into a pool, and a cell that states nothing falls back to its row, then to its
/// column, then to the sheet — which is the order Calc resolves in and the order all three file
/// formats write.
/// </para>
/// <para>
/// Row before column, deliberately. SpreadsheetML says so directly: a <c>&lt;row&gt;</c> with
/// <c>customFormat</c> overrides the <c>&lt;col&gt;</c>'s <c>style</c>
/// (<c>sc/source/filter/oox/sheetdatabuffer.cxx</c>, which applies row formats after column
/// ones), and ODF's repeated <c>table:table-row</c> default cell style behaves the same way.
/// Getting the order backwards is invisible until a sheet formats both a row and a column, at
/// which point every cell in the crossing is wrong.
/// </para>
/// </remarks>
public sealed class SheetCellFormats
{
    private readonly List<SheetCellFormat> _pool;
    private readonly Dictionary<(int Row, int Column), int> _cells;
    private readonly SheetBlockIndex _blocks;
    private readonly Dictionary<int, int> _rows;
    private readonly Dictionary<int, int> _columns;
    private readonly int _sheet;
    private readonly Dictionary<(int Row, int Column), SheetConditionalText> _conditional;
    private readonly Dictionary<(int Row, int Column), SheetPivotStyle> _pivot;

    private SheetCellFormats(
        List<SheetCellFormat> pool,
        Dictionary<(int, int), int> cells,
        SheetBlockIndex blocks,
        Dictionary<int, int> rows,
        Dictionary<int, int> columns,
        int sheet,
        int lastAllocatedColumn,
        Dictionary<(int, int), SheetConditionalText>? conditional = null,
        Dictionary<(int, int), SheetPivotStyle>? pivot = null)
    {
        _pool = pool;
        _cells = cells;
        _blocks = blocks;
        _rows = rows;
        _columns = columns;
        _sheet = sheet;
        LastAllocatedColumn = lastAllocatedColumn;
        _conditional = conditional ?? [];
        _pivot = pivot ?? [];
    }

    /// <summary>A sheet whose every cell is in the default format.</summary>
    public static SheetCellFormats Empty { get; } =
        new([SheetCellFormat.Default], [], new SheetBlockIndex(), [], [], 0, -1);

    /// <summary>
    /// The same formats with a conditional-format overlay laid over them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A new instance sharing the pool rather than a setter, for one hard reason and one soft
    /// one. <see cref="Empty"/> is a shared singleton that a reader returns whenever a sheet
    /// states no formats at all, and a sheet whose only text formatting is a conditional rule is
    /// exactly that sheet — mutating it would put one workbook's rules on every other document
    /// in the process. And the store is otherwise immutable once built, which is what lets a
    /// layout be shared between the pages that draw it.
    /// </para>
    /// <para>
    /// The overlay is asked for on every <see cref="At"/>, so it is guarded by a count rather
    /// than by a lookup: a plain workbook pays one integer comparison per cell per page.
    /// </para>
    /// </remarks>
    /// <param name="conditional">What each cell's matching rule changes about its text.</param>
    public SheetCellFormats WithConditionalText(
        Dictionary<(int Row, int Column), SheetConditionalText> conditional)
    {
        ArgumentNullException.ThrowIfNull(conditional);

        return conditional.Count == 0
            ? this
            : new SheetCellFormats(
                _pool, _cells, _blocks, _rows, _columns, _sheet, LastAllocatedColumn, conditional,
                _pivot);
    }

    /// <summary>
    /// The same formats with a pivot table's generated cell styles laid over them.
    /// </summary>
    /// <remarks>
    /// A second overlay rather than a wider <see cref="SheetConditionalText"/>, because the two
    /// are different layers of the same cell and can both apply: a conditional format is
    /// evaluated over whatever the cell resolves to, which on a pivot's output is the generated
    /// style. A new instance for the reason <see cref="WithConditionalText"/> gives — a pivot
    /// sheet very often states no text formatting at all and is therefore holding the shared
    /// <see cref="Empty"/>.
    /// </remarks>
    /// <param name="pivot">What each cell's generated pivot style changes about its text.</param>
    public SheetCellFormats WithPivotStyles(Dictionary<(int Row, int Column), SheetPivotStyle> pivot)
    {
        ArgumentNullException.ThrowIfNull(pivot);

        return pivot.Count == 0
            ? this
            : new SheetCellFormats(
                _pool, _cells, _blocks, _rows, _columns, _sheet, LastAllocatedColumn, _conditional,
                pivot);
    }

    /// <summary>
    /// The last column the sheet <em>materialises</em>, or -1 when it materialises none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Not the same question as how far the data or the print area reaches, and Calc asks both.
    /// <c>ScTable::aCol</c> holds only the columns that have been allocated, and a column is
    /// allocated by anything applying a pattern to it — a cell that states a format and holds no
    /// value allocates one exactly as a cell holding a value does. That matters because
    /// <c>GetOptimalHeightsInColumn</c> (<c>sc/source/core/data/table1.cxx:88-127</c>) walks
    /// <em>every allocated column</em> rather than the print area's, and a column with no pattern
    /// at a row contributes the sheet's default pattern's arithmetic height there. So one
    /// formatted blank cell, anywhere, makes the default font's row height a floor for every row
    /// of the sheet.
    /// </para>
    /// <para>
    /// <strong>A format that runs to the sheet's last column allocates nothing</strong>, which is
    /// what keeps this from being 16383 on every file Calc has written:
    /// <c>ScTable::ApplyPatternArea</c> takes <c>maxCol = max(nStartCol, aCol.size()) - 1</c> and
    /// puts the remainder in <c>aDefaultColData</c>
    /// (<c>sc/source/core/data/table2.cxx:2980-2999</c>). Every <c>.ods</c> Calc writes pads each
    /// row with a <c>table:table-cell table:style-name="Default"</c> repeated to column 16383, and
    /// that run is exactly the case the clause excludes.
    /// </para>
    /// <para>
    /// A whole-column default — ODF's <c>table:default-cell-style-name</c>, SpreadsheetML's
    /// <c>&lt;col style&gt;</c> — is deliberately <em>not</em> counted, although a bounded one does
    /// allocate in Calc. The store keeps column defaults per column and cannot tell a bounded run
    /// from one written to the sheet's last column, so counting them would widen every file that
    /// states a trailing column style by sixteen thousand columns. The under-approximation is the
    /// safe direction: it can only leave the answer where it already was.
    /// </para>
    /// </remarks>
    public int LastAllocatedColumn { get; }

    /// <summary>
    /// Every stated format inside a block, as one column interval per row rather than per cell.
    /// </summary>
    /// <remarks>
    /// <see cref="CellsIn"/> unfolds a rectangle column by column, which is what a caller wanting
    /// each cell's <em>format</em> needs. A caller asking only <em>which columns a row states
    /// anything for</em> does not need the unfolding, and paying for it is what makes the question
    /// unaskable over a wide span: a padded sheet's rectangle is sixteen thousand columns wide.
    /// </remarks>
    /// <param name="firstRow">The first row, inclusive.</param>
    /// <param name="lastRow">The last row, inclusive.</param>
    /// <param name="firstColumn">The first column, inclusive.</param>
    /// <param name="lastColumn">The last column, inclusive.</param>
    public IEnumerable<(int Row, int FirstColumn, int LastColumn)> StatedSpans(
        int firstRow, int lastRow, int firstColumn, int lastColumn)
    {
        foreach (((int row, int column), int _) in _cells)
        {
            if (row < firstRow || row > lastRow) continue;
            if (column < firstColumn || column > lastColumn) continue;
            yield return (row, column, column);
        }

        foreach (SheetFormatBlock block in _blocks.Blocks)
        {
            int first = Math.Max(block.FirstColumn, firstColumn);
            int last = Math.Min(block.LastColumn, lastColumn);
            if (last < first) continue;

            for (int row = Math.Max(block.FirstRow, firstRow);
                 row <= Math.Min(block.LastRow, lastRow);
                 row++)
            {
                yield return (row, first, last);
            }
        }
    }

    /// <summary>The format a cell is drawn in.</summary>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public SheetCellFormat At(int row, int column)
    {
        SheetCellFormat stated = Stated(row, column);

        // The generated style first: a pivot's output is what the cell resolves to before any
        // rule is evaluated over it, and a conditional format wins over both.
        if (_pivot.Count > 0 && _pivot.TryGetValue((row, column), out SheetPivotStyle generated))
            stated = generated.Over(stated);

        return _conditional.Count > 0
               && _conditional.TryGetValue((row, column), out SheetConditionalText rule)
            ? rule.Over(stated)
            : stated;
    }

    /// <summary>What a cell states, before any conditional format is applied over it.</summary>
    private SheetCellFormat Stated(int row, int column)
    {
        if (_cells.TryGetValue((row, column), out int index)) return _pool[index];

        // The repeats a sheet is padded with, kept as rectangles: a real workbook states
        // thousands of them, so the store is grouped by row range and this is a binary search
        // rather than a scan.
        if (_blocks.At(row, column) is { } block) return _pool[block];

        if (_rows.TryGetValue(row, out index)) return _pool[index];
        if (_columns.TryGetValue(column, out index)) return _pool[index];
        return _pool[_sheet];
    }

    /// <summary>The format a cell that states nothing, in a row and column that state nothing, takes.</summary>
    public SheetCellFormat SheetDefault => _pool[_sheet];

    /// <summary>The format a whole row states, or null when it states none.</summary>
    /// <param name="row">The zero-based row.</param>
    public SheetCellFormat? RowDefault(int row)
        => _rows.TryGetValue(row, out int index) ? _pool[index] : null;

    /// <summary>The formats whole columns state, within a range.</summary>
    /// <remarks>
    /// A column format applies to every row at once, so a caller measuring rows can fold these in
    /// once rather than per row. Bounded by the range because a file may state a format for all
    /// sixteen thousand columns and only the ones a sheet reaches are allocated in Calc.
    /// </remarks>
    /// <param name="first">The first column of the range, inclusive.</param>
    /// <param name="last">The last column of the range, inclusive.</param>
    public IEnumerable<SheetCellFormat> ColumnDefaults(int first, int last)
    {
        foreach ((int column, int index) in _columns)
            if (column >= first && column <= last)
                yield return _pool[index];
    }

    /// <summary>Every cell that states a format of its own, with where it is.</summary>
    /// <remarks>
    /// Enumerated rather than indexed by row because the store is one dictionary keyed by
    /// position: a caller that wants them grouped by row gets them in one pass and groups them
    /// itself, where asking per row would rescan the whole sheet for each.
    /// </remarks>
    public IEnumerable<(int Row, int Column, SheetCellFormat Format)> Cells
    {
        get
        {
            foreach (((int row, int column), int index) in _cells)
                yield return (row, column, _pool[index]);
        }
    }

    /// <summary>
    /// Every cell that states a format of its own inside a block, with where it is.
    /// </summary>
    /// <remarks>
    /// <see cref="Cells"/> answers only the cells the file wrote one at a time; a repeated
    /// element is kept as a rectangle and would be missed. A caller measuring a range asks this
    /// instead, and the rectangle is unfolded into that range and no further — which is why it
    /// takes one: a sheet's padding is a rectangle sixteen thousand columns by a million rows,
    /// and unfolding it whole is what this store exists to avoid.
    /// </remarks>
    /// <param name="range">The block to unfold into.</param>
    public IEnumerable<(int Row, int Column, SheetCellFormat Format)> CellsIn(SheetRange range)
    {
        foreach (((int row, int column), int index) in _cells)
        {
            if (row < range.FirstRow || row > range.LastRow) continue;
            if (column < range.FirstColumn || column > range.LastColumn) continue;
            yield return (row, column, _pool[index]);
        }

        foreach (SheetFormatBlock block in _blocks.Blocks)
        {
            int firstRow = Math.Max(block.FirstRow, range.FirstRow);
            int lastRow = Math.Min(block.LastRow, range.LastRow);
            int firstColumn = Math.Max(block.FirstColumn, range.FirstColumn);
            int lastColumn = Math.Min(block.LastColumn, range.LastColumn);

            for (int row = firstRow; row <= lastRow; row++)
            {
                for (int column = firstColumn; column <= lastColumn; column++)
                {
                    // A cell stated one at a time beats the block it sits in, and has already
                    // been answered above.
                    if (_cells.ContainsKey((row, column))) continue;
                    yield return (row, column, _pool[block.Index]);
                }
            }
        }
    }

    /// <summary>Accumulates a sheet's formats while its cells are being read.</summary>
    /// <remarks>
    /// Pooling by value rather than by the file's own index, because the three formats index
    /// differently — SpreadsheetML by position in <c>cellXfs</c>, BIFF by <c>XF</c> ordinal, ODF
    /// by style name — and because two of the three routinely write several indices that resolve
    /// to the same text format. Pooling on the resolved record makes the lookup one dictionary
    /// for all three.
    /// </remarks>
    public sealed class Builder
    {
        private readonly List<SheetCellFormat> _pool = [SheetCellFormat.Default];
        private readonly Dictionary<SheetCellFormat, int> _indices = new()
        {
            [SheetCellFormat.Default] = 0,
        };

        private readonly Dictionary<(int, int), int> _cells = [];
        private readonly SheetBlockIndex _blocks = new();
        private readonly Dictionary<int, int> _rows = [];
        private readonly Dictionary<int, int> _columns = [];
        private int _sheet;

        /// <summary>The pool index a format has, adding it when it is new.</summary>
        /// <param name="format">The resolved format.</param>
        public int Intern(SheetCellFormat? format)
        {
            if (format is null) return 0;
            if (_indices.TryGetValue(format, out int index)) return index;

            index = _pool.Count;
            _pool.Add(format);
            _indices[format] = index;
            return index;
        }

        /// <summary>Records one cell's format.</summary>
        public void SetCell(int row, int column, int index)
        {
            if (row < 0 || column < 0 || index <= 0) return;
            _cells[(row, column)] = index;
        }

        /// <summary>
        /// Records the format a whole rectangle of cells states, without materialising them.
        /// </summary>
        /// <remarks>
        /// What ODF's <c>table:number-columns-repeated</c> and <c>table:number-rows-repeated</c>
        /// describe, and what Calc records for one: <c>ScXMLTableRowCellContext::AddNonFormulaCell</c>
        /// clamps the repeat to the sheet's own last column and row and hands
        /// <c>ScMyStylesImportHelper::AddRange</c> a single <c>ScRange</c>
        /// (<c>sc/source/filter/xml/xmlcelli.cxx</c>:1368-1377), which is merged into an
        /// <c>ScRangeList</c> per style name and applied over ranges at the end of the import.
        /// Calc's own store is a run array rather than a cell map, so a sheet padded to its full
        /// extent costs one entry per repeat and not one per cell.
        /// </remarks>
        /// <param name="firstRow">The first row, inclusive.</param>
        /// <param name="lastRow">The last row, inclusive.</param>
        /// <param name="firstColumn">The first column, inclusive.</param>
        /// <param name="lastColumn">The last column, inclusive.</param>
        /// <param name="index">The pool index, from <see cref="Intern"/>.</param>
        public void SetCells(int firstRow, int lastRow, int firstColumn, int lastColumn, int index)
        {
            if (index <= 0 || firstRow < 0 || firstColumn < 0) return;
            if (lastRow < firstRow || lastColumn < firstColumn) return;

            // Clamped rather than dropped: a repeat that runs past the sheet's own bounds is
            // honoured up to them, which is what `ScXMLTableRowContext` and
            // `ScXMLTableRowCellContext` each do to their own repeat counts.
            _blocks.Add(firstRow, lastRow, firstColumn, lastColumn, index);
        }

        /// <summary>Records a whole row's default format.</summary>
        public void SetRow(int row, int index)
        {
            if (row < 0 || index <= 0) return;
            _rows[row] = index;
        }

        /// <summary>Records a whole column's default format.</summary>
        public void SetColumn(int column, int index)
        {
            if (column < 0 || index <= 0) return;
            _columns[column] = index;
        }

        /// <summary>Records the format everything else falls back to.</summary>
        /// <remarks>
        /// Pool index 0 — <see cref="SheetCellFormat.Default"/> — is a legitimate answer here and
        /// not "no answer", which is why this does not carry the <c>index &lt;= 0</c> guard the
        /// cell, row and column setters do. SpreadsheetML states the sheet default twice: once as
        /// the workbook's <c>Normal</c> cell style and again, on a sheet that has one, as a
        /// <c>&lt;col&gt;</c> spanning to the last column, and the second must be able to take the
        /// plain default back off the first.
        /// </remarks>
        /// <param name="index">The pool index, from <see cref="Intern"/>.</param>
        public void SetSheetDefault(int index)
        {
            if (index >= 0) _sheet = index;
        }

        /// <summary>True when nothing but the default has been recorded.</summary>
        public bool IsEmpty =>
            _cells.Count == 0 && _blocks.IsEmpty && _rows.Count == 0 && _columns.Count == 0
            && _sheet == 0;

        /// <summary>The finished lookup.</summary>
        public SheetCellFormats Build()
        {
            if (IsEmpty) return Empty;

            _blocks.Seal();
            return new SheetCellFormats(
                _pool, _cells, _blocks, _rows, _columns, _sheet, LastAllocated());
        }

        /// <inheritdoc cref="SheetCellFormats.LastAllocatedColumn"/>
        private int LastAllocated()
        {
            int last = -1;
            foreach (((int _, int column), int _index) in _cells) last = Math.Max(last, column);

            foreach (SheetFormatBlock block in _blocks.Blocks)
            {
                // `ApplyPatternArea`'s own clause: a range ending at the sheet's last column
                // allocates only the columns before it that already existed, so the run itself
                // materialises nothing past where it starts.
                last = Math.Max(
                    last,
                    block.LastColumn >= SheetAddress.MaxColumn
                        ? block.FirstColumn - 1
                        : block.LastColumn);
            }

            return last;
        }
    }
}

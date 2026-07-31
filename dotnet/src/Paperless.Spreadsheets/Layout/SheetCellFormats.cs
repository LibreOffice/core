namespace Paperless.Spreadsheets.Layout;

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
    private readonly Dictionary<int, int> _rows;
    private readonly Dictionary<int, int> _columns;
    private readonly int _sheet;

    private SheetCellFormats(
        List<SheetCellFormat> pool,
        Dictionary<(int, int), int> cells,
        Dictionary<int, int> rows,
        Dictionary<int, int> columns,
        int sheet)
    {
        _pool = pool;
        _cells = cells;
        _rows = rows;
        _columns = columns;
        _sheet = sheet;
    }

    /// <summary>A sheet whose every cell is in the default format.</summary>
    public static SheetCellFormats Empty { get; } =
        new([SheetCellFormat.Default], [], [], [], 0);

    /// <summary>The format a cell is drawn in.</summary>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public SheetCellFormat At(int row, int column)
    {
        if (_cells.TryGetValue((row, column), out int index)) return _pool[index];
        if (_rows.TryGetValue(row, out index)) return _pool[index];
        if (_columns.TryGetValue(column, out index)) return _pool[index];
        return _pool[_sheet];
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
        public void SetSheetDefault(int index)
        {
            if (index > 0) _sheet = index;
        }

        /// <summary>True when nothing but the default has been recorded.</summary>
        public bool IsEmpty =>
            _cells.Count == 0 && _rows.Count == 0 && _columns.Count == 0 && _sheet == 0;

        /// <summary>The finished lookup.</summary>
        public SheetCellFormats Build()
            => IsEmpty ? Empty : new SheetCellFormats(_pool, _cells, _rows, _columns, _sheet);
    }
}

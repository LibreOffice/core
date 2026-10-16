using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// The pattern a border line is drawn with.
/// </summary>
/// <remarks>
/// Ordered the way <c>SvxBorderLineStyle</c> is, because the order is load-bearing rather than
/// decorative: <c>svx::frame::Style::operator&lt;</c> settles a tie between two one-twip single
/// lines by comparing the enumerators, and the <em>lower</em> one wins
/// (<c>svx/source/dialog/framelink.cxx:330</c>, <c>Type() &gt; rOther.Type()</c> means "this is
/// the lesser"). Renumbering these would silently change which of two hairlines survives a
/// shared edge.
/// </remarks>
public enum SheetBorderPattern
{
    /// <summary>An unbroken line.</summary>
    Solid,

    /// <summary>Evenly spaced dots.</summary>
    Dotted,

    /// <summary>Evenly spaced dashes.</summary>
    Dashed,

    /// <summary>Dash, dot, dash.</summary>
    DashDot,

    /// <summary>Dash, dot, dot, dash.</summary>
    DashDotDot,

    /// <summary>Short, closely spaced dashes — Excel's <c>mediumDashed</c> family.</summary>
    FineDashed,
}

/// <summary>
/// One edge of one cell: how wide, what colour, and whether it is a double rule.
/// </summary>
/// <remarks>
/// <para>
/// Modelled on <c>svx::frame::Style</c> rather than on any one file format, because that is the
/// primitive Calc and Writer both resolve their borders through
/// (<c>svx/source/dialog/framelink.cxx</c>): a border is a <em>primary</em> line, an optional
/// gap, and an optional <em>secondary</em> line, and the three together are its width. Every
/// format's spelling collapses onto it — ODF states <c>fo:border</c> as a width, a style and a
/// colour; SpreadsheetML names one of fourteen styles from a table; BIFF numbers the same
/// fourteen — so the readers convert once and nothing downstream branches on provenance.
/// </para>
/// <para>
/// Widths are twips because that is the unit all three formats' tables are written in.
/// LibreOffice's own constants say so in as many words: <c>EXC_BORDER_HAIR</c> is 1,
/// <c>EXC_BORDER_THIN</c> 15, <c>EXC_BORDER_MEDIUM</c> 35 and <c>EXC_BORDER_THICK</c> 50
/// (<c>sc/source/filter/inc/xlconst.hxx:250-253</c>), and the OOXML filter repeats the same four
/// numbers under different names (<c>sc/source/filter/inc/stylesbuffer.hxx:63-67</c>). Measured
/// in a LibreOffice PDF, a <c>thick</c> border strokes at 2.49983 pt and a <c>thin</c> one at
/// 0.75003 pt — exactly 50 and 15 twips.
/// </para>
/// </remarks>
/// <param name="Primary">The main line's width; zero means the edge states no border.</param>
/// <param name="Distance">The gap between the two lines of a double rule.</param>
/// <param name="Secondary">The second line's width, zero for a single rule.</param>
/// <param name="Colour">The line's colour.</param>
/// <param name="Pattern">Whether it is solid, dotted or dashed.</param>
public readonly record struct SheetBorder(
    Length Primary,
    Length Distance,
    Length Secondary,
    Colour Colour,
    SheetBorderPattern Pattern = SheetBorderPattern.Solid)
{
    /// <summary>No border at all, which is what an edge no format mentions has.</summary>
    public static SheetBorder None { get; }

    /// <summary>A plain single line.</summary>
    /// <param name="width">The line's width.</param>
    /// <param name="colour">Its colour.</param>
    /// <param name="pattern">Its pattern.</param>
    public static SheetBorder Line(
        Length width, Colour colour, SheetBorderPattern pattern = SheetBorderPattern.Solid)
        => width <= Length.Zero
            ? None
            : new SheetBorder(width, Length.Zero, Length.Zero, colour, pattern);

    /// <summary>True when the edge draws nothing.</summary>
    public bool IsNone => Primary <= Length.Zero && Secondary <= Length.Zero;

    /// <summary>True when the rule is two lines with a gap between them.</summary>
    public bool IsDouble => Secondary > Length.Zero;

    /// <summary>The whole width the rule occupies, gap and second line included.</summary>
    /// <remarks><c>svx::frame::Style::GetWidth</c>, which is what the shared-edge rule compares.</remarks>
    public Length Width => Primary + Distance + Secondary;

    /// <summary>
    /// True when this border beats another for a shared edge.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The whole of <c>svx::frame::Style::operator&lt;</c>
    /// (<c>svx/source/dialog/framelink.cxx:306-334</c>), in the order it tests: a wider rule
    /// wins; failing that a double rule beats a single one; failing that the double with the
    /// <em>narrower</em> gap wins; and two single rules one twip wide are settled by their
    /// pattern, the earlier enumerator winning.
    /// </para>
    /// <para>
    /// Colour is deliberately absent. Two borders of the same width and pattern in different
    /// colours are <em>equal</em> to Calc, and the tie goes to the cell whose own edge it is —
    /// which is why <see cref="SheetCellBorders.Resolve"/> passes the owning cell's edge first.
    /// </para>
    /// </remarks>
    /// <param name="other">The border to compare against.</param>
    public bool IsHeavierThan(SheetBorder other)
    {
        if (Width != other.Width) return Width > other.Width;
        if (IsDouble != other.IsDouble) return IsDouble;
        if (IsDouble && other.IsDouble && Distance != other.Distance) return Distance < other.Distance;

        if (Width == Length.FromTwips(1) && !IsDouble && !other.IsDouble && Pattern != other.Pattern)
            return Pattern < other.Pattern;

        return false;
    }
}

/// <summary>The four edges a cell can state a border on.</summary>
/// <param name="Left">Its left edge.</param>
/// <param name="Right">Its right edge.</param>
/// <param name="Top">Its top edge.</param>
/// <param name="Bottom">Its bottom edge.</param>
public readonly record struct SheetCellBorders(
    SheetBorder Left, SheetBorder Right, SheetBorder Top, SheetBorder Bottom)
{
    /// <summary>A cell that states no border at all.</summary>
    public static SheetCellBorders None { get; }

    /// <summary>True when none of the four edges draws anything.</summary>
    public bool IsNone => Left.IsNone && Right.IsNone && Top.IsNone && Bottom.IsNone;

    /// <summary>
    /// Which of two neighbours' borders is drawn on the edge they share.
    /// </summary>
    /// <remarks>
    /// The heavier one, which is <c>std::max</c> over <c>svx::frame::Style::operator&lt;</c> —
    /// literally what <c>Array::GetCellStyleLeft</c> writes:
    /// <c>max(own left style, right style of left neighbour)</c>
    /// (<c>svx/source/dialog/framelinkarray.cxx:796-799</c>), and the same for the other three
    /// edges. A tie goes to <paramref name="own"/> because <c>std::max</c> returns its first
    /// argument when neither is less than the other, and the first argument is always the cell
    /// being asked about.
    /// </remarks>
    /// <param name="own">The edge the cell being drawn states.</param>
    /// <param name="neighbour">The facing edge its neighbour states.</param>
    public static SheetBorder Resolve(SheetBorder own, SheetBorder neighbour)
        => neighbour.IsHeavierThan(own) ? neighbour : own;
}

/// <summary>
/// Everything about a cell that is drawn but is not its text.
/// </summary>
/// <remarks>
/// A value type keyed out of a palette rather than one object per cell, because a sheet's
/// formatting is a run-length structure over a very sparse set of distinct formats: a workbook
/// with a million cells routinely has a dozen. <see cref="SheetFormatting"/> holds the palette
/// and the index; this is one entry in it.
/// </remarks>
/// <param name="Background">The cell's fill, or null when it is transparent.</param>
/// <param name="Borders">Its four edges.</param>
public readonly record struct SheetCellDecoration(Colour? Background, SheetCellBorders Borders)
{
    /// <summary>A cell with no fill and no border, which is what most cells have.</summary>
    public static SheetCellDecoration None { get; }

    /// <summary>True when the cell draws nothing but its text.</summary>
    public bool IsNone => Background is null && Borders.IsNone;
}

/// <summary>
/// A sheet's cell formatting, kept apart from its cells.
/// </summary>
/// <remarks>
/// <para>
/// Apart, because content and formatting are stored independently in all three formats and
/// merging them would be ruinous on the sheets where it matters: a SpreadsheetML
/// <c>&lt;col style="3"/&gt;</c> formats sixteen thousand columns in one attribute, and a
/// <c>&lt;row s="7" customFormat="1"/&gt;</c> formats a whole row without writing a single
/// <c>&lt;c&gt;</c>. Both have to be honoured — a fill applied to a whole column prints on
/// every row of it, cells or no cells — so the lookup resolves cell, then row, then column,
/// then the sheet's default, which is the order Calc's own attribute lookup takes.
/// </para>
/// <para>
/// Formats are interned into a palette on construction, so two columns that agree cost one
/// entry and the index is an <c>int</c>.
/// </para>
/// </remarks>
public sealed class SheetFormatting
{
    private readonly List<SheetCellDecoration> _palette = [SheetCellDecoration.None];
    private readonly Dictionary<SheetCellDecoration, int> _byFormat = new() { [SheetCellDecoration.None] = 0 };
    private readonly Dictionary<(int Row, int Column), int> _cells = [];
    private readonly SheetBlockIndex _blocks = new();
    private readonly Dictionary<int, int> _rows = [];
    private readonly List<(int First, int Last, int Format)> _columns = [];
    private readonly Dictionary<(int Row, int Column), Colour> _conditional = [];
    private readonly Dictionary<(int Row, int Column), SheetDataBar> _bars = [];
    private readonly Dictionary<(int Row, int Column), SheetIcon> _icons = [];
    private int _default;
    private bool _hasDefaults;

    /// <summary>A sheet whose cells state no borders and no fills.</summary>
    public static SheetFormatting Empty { get; } = new();

    /// <summary>True when nothing anywhere on the sheet needs painting.</summary>
    /// <remarks>
    /// Checked before a page walks its cells, so a plain sheet pays one boolean rather than one
    /// dictionary lookup per cell per page.
    /// </remarks>
    public bool IsEmpty => _palette.Count == 1 && _cells.Count == 0 && _blocks.IsEmpty
                           && _rows.Count == 0 && _columns.Count == 0 && _conditional.Count == 0
                           && _bars.Count == 0 && _icons.Count == 0;

    /// <summary>Interns a format and returns the handle the setters take.</summary>
    /// <param name="format">The format to intern.</param>
    public int Intern(SheetCellDecoration format)
    {
        if (_byFormat.TryGetValue(format, out int found)) return found;

        int index = _palette.Count;
        _palette.Add(format);
        _byFormat[format] = index;
        return index;
    }

    /// <summary>
    /// Sets the format one cell states of its own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A handle of zero is meaningful and is <em>kept</em> once anything wider has been set: it
    /// means "this cell states a style and that style paints nothing", which has to beat the
    /// column's fill. That is not a hypothetical. Saving <c>sheet-decor.fods</c> as ODS, Calc
    /// moved a single blue cell onto its whole column as
    /// <c>table:default-cell-style-name="ce12"</c> and cancelled it seven rows down with
    /// <c>table:style-name="Default"</c> on the one cell that must stay white — so a reader that
    /// treats "states the default style" as "states nothing" paints a cell LibreOffice leaves
    /// blank.
    /// </para>
    /// <para>
    /// Before anything wider has been set there is nothing to override, so the entry is dropped
    /// — which is what keeps a plainly-formatted million-cell sheet from storing a zero per cell.
    /// </para>
    /// </remarks>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    /// <param name="format">A handle from <see cref="Intern"/>, or zero for "explicitly plain".</param>
    public void SetCell(int row, int column, int format)
    {
        if (row < 0 || column < 0) return;
        if (format <= 0 && !_hasDefaults) return;
        _cells[(row, column)] = Math.Max(0, format);
    }

    /// <summary>
    /// Sets the format a whole rectangle of cells states, without materialising them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// What ODF's two repeat counts describe between them, and Calc records exactly this: the
    /// importer clamps each to the sheet's own bounds and hands the styles helper one
    /// <c>ScRange</c> (<c>sc/source/filter/xml/xmlcelli.cxx</c>:1368-1377), merged per style name
    /// into an <c>ScRangeList</c> and applied over ranges. Expanding it instead is the difference
    /// between one entry and the 16381 × 1048567 cells a Calc-written sheet's padding declares.
    /// </para>
    /// <para>
    /// A zero handle is kept on the same terms as <see cref="SetCell"/>: once anything wider has
    /// been set it means "these cells state a style and it paints nothing".
    /// </para>
    /// </remarks>
    /// <param name="firstRow">The first row, inclusive.</param>
    /// <param name="lastRow">The last row, inclusive.</param>
    /// <param name="firstColumn">The first column, inclusive.</param>
    /// <param name="lastColumn">The last column, inclusive.</param>
    /// <param name="format">A handle from <see cref="Intern"/>, or zero for "explicitly plain".</param>
    public void SetCells(int firstRow, int lastRow, int firstColumn, int lastColumn, int format)
    {
        if (firstRow < 0 || firstColumn < 0) return;
        if (lastRow < firstRow || lastColumn < firstColumn) return;
        if (format <= 0 && !_hasDefaults) return;

        _blocks.Add(firstRow, lastRow, firstColumn, lastColumn, Math.Max(0, format));
    }

    /// <summary>
    /// Takes the background off every cell of a rectangle, leaving its borders alone.
    /// </summary>
    /// <remarks>
    /// <para>
    /// What <c>clearContents(… HARDATTR | STYLES …)</c> and <c>DeleteAreaTab(…, ALL)</c> do to a
    /// pivot table's output range before <c>ScDPOutput</c> writes into it. A fill is the one
    /// thing the clearing removes that is not a property of the text, so it cannot travel on
    /// <see cref="SheetPivotStyle"/> and needs a seat here.
    /// </para>
    /// <para>
    /// It reads what a cell <em>states</em> rather than <see cref="At"/>, so a conditional format
    /// evaluated over the same cell is not baked into it — a conditional fill is a later layer
    /// and the clearing happens at import, before any rule is evaluated. And it writes only where
    /// there is something to remove, so a pivot over a plainly-formatted rectangle costs a lookup
    /// per cell and no storage at all: measured over the corpus's nineteen generatable pivot
    /// rectangles, <strong>not one of their 9878 cells states a fill</strong>
    /// (<c>probes/pivot-fmt-r110/statedface.py</c>), so this is nil-reach there and is here
    /// because an authored fixture shows 26.2.4.2 doing it.
    /// </para>
    /// </remarks>
    /// <param name="firstRow">The first row, inclusive.</param>
    /// <param name="lastRow">The last row, inclusive.</param>
    /// <param name="firstColumn">The first column, inclusive.</param>
    /// <param name="lastColumn">The last column, inclusive.</param>
    public void ClearBackgrounds(int firstRow, int lastRow, int firstColumn, int lastColumn)
    {
        for (int row = Math.Max(firstRow, 0); row <= lastRow; row++)
        {
            for (int column = Math.Max(firstColumn, 0); column <= lastColumn; column++)
            {
                SheetCellDecoration stated = Stated(row, column);
                if (stated.Background is null) continue;

                // Written straight into the per-cell store rather than through
                // `SetCell`, which drops a zero handle until something wider has been set —
                // right for a reader that would otherwise store a zero per cell, and wrong here,
                // where the zero has to *replace* what the cell already states.
                _cells[(row, column)] = Intern(stated with { Background = null });
            }
        }
    }

    /// <summary>Sets the format a whole row applies to the cells that state none.</summary>
    /// <param name="row">The zero-based row.</param>
    /// <param name="format">A handle from <see cref="Intern"/>, or zero for "explicitly plain".</param>
    public void SetRow(int row, int format)
    {
        if (row < 0) return;
        if (format <= 0 && !_hasDefaults) return;

        _rows[row] = Math.Max(0, format);
        if (format > 0) _hasDefaults = true;
    }

    /// <summary>Sets the format a run of columns applies where nothing narrower does.</summary>
    /// <param name="first">The first column, inclusive.</param>
    /// <param name="last">The last column, inclusive.</param>
    /// <param name="format">A handle from <see cref="Intern"/>.</param>
    public void SetColumns(int first, int last, int format)
    {
        if (format <= 0 || last < first || first < 0) return;

        _columns.Add((first, last, format));
        _hasDefaults = true;
    }

    /// <summary>
    /// Sets the fill a conditional format paints on one cell, over whatever it states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A layer of its own rather than an interned format, for two reasons that pull the same way.
    /// </para>
    /// <para>
    /// It has to <em>beat</em> the cell's own fill: Calc rebuilds the cell's
    /// <c>ATTR_BACKGROUND</c> from the colour scale after the stated one and after any
    /// style-named condition (<c>sc/source/core/data/fillinfo.cxx:776-781</c>), and an authored
    /// scale over cells stating a solid green paints the scale on all eleven.
    /// </para>
    /// <para>
    /// And it must <strong>not</strong> reach <see cref="Cells"/>, <see cref="Rows"/> or
    /// <see cref="ColumnRuns"/>, which is what <see cref="SheetDecorationArea"/> reads to decide
    /// how far the sheet prints. A conditional format is not a cell attribute and does not extend
    /// the print area — measured, a scale declared over <c>B2:B40</c> on a sheet whose data stops
    /// at <c>B12</c> still prints one page on 26.2.4.2 — and one corpus rule is declared over
    /// <c>N18:Q1048576</c>, which would otherwise reformat a page count into a million rows.
    /// </para>
    /// </remarks>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    /// <param name="colour">The colour the rule resolved to for this cell.</param>
    public void SetConditionalBackground(int row, int column, Colour colour)
    {
        if (row < 0 || column < 0) return;
        _conditional[(row, column)] = colour;
    }

    /// <summary>Sets the data bar a <c>dataBar</c> rule draws over one cell.</summary>
    /// <remarks>
    /// A third layer beside the stated formats and the conditional fills, for the reason
    /// <see cref="SheetDataBar"/> gives: a bar covers part of a cell and is drawn over its
    /// background rather than instead of it, so it can be neither an interned format nor a
    /// conditional fill. It is kept out of <see cref="Cells"/>, <see cref="Rows"/> and
    /// <see cref="ColumnRuns"/> for the same reason a conditional fill is — a rule must not
    /// extend how far the sheet prints.
    /// </remarks>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    /// <param name="bar">The bar the rule resolved to for this cell.</param>
    public void SetConditionalBar(int row, int column, SheetDataBar bar)
    {
        if (row < 0 || column < 0) return;
        _bars[(row, column)] = bar;
    }

    /// <summary>The data bar drawn over one cell, or null when no rule reaches it.</summary>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public SheetDataBar? BarAt(int row, int column)
        => _bars.Count > 0 && _bars.TryGetValue((row, column), out SheetDataBar bar) ? bar : null;

    /// <summary>Sets the icon an <c>iconSet</c> rule draws over one cell.</summary>
    /// <remarks>
    /// A fourth layer, for the reason the third is one: an icon is painted over the cell's own
    /// background in its bottom-left corner rather than replacing anything, and it must not reach
    /// <see cref="SheetDecorationArea"/> — one corpus icon rule is stated over <c>I10:BL35</c> and
    /// another over a whole column, and a conditional format does not extend how far a sheet
    /// prints.
    /// <para>
    /// Set only where the reference would have an <c>ScIconSetInfo</c>. A cell whose bucket is a
    /// custom <c>NoIcons</c> gets no entry at all rather than an empty one, because such a cell
    /// keeps its own text however <c>showValue</c> is set
    /// (<c>sc/source/core/data/colorscale.cxx</c>:1236-1239).
    /// </para>
    /// </remarks>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    /// <param name="icon">The icon the rule resolved to for this cell.</param>
    public void SetConditionalIcon(int row, int column, SheetIcon icon)
    {
        if (row < 0 || column < 0) return;
        _icons[(row, column)] = icon;
    }

    /// <summary>The icon drawn over one cell, or null when no rule reaches it.</summary>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public SheetIcon? IconAt(int row, int column)
        => _icons.Count > 0 && _icons.TryGetValue((row, column), out SheetIcon icon) ? icon : null;

    /// <summary>
    /// Whether a rule takes one cell's own value off the page entirely.
    /// </summary>
    /// <remarks>
    /// <c>showValue="0"</c>. The reference does not hide the number behind the bar or the icon, it
    /// does not draw it: <c>ScOutputData::DrawStrings</c> clears <c>bDoCell</c> before laying
    /// anything out (<c>sc/source/ui/view/output2.cxx</c>:1691-1698), and 26.2.4.2's own PDF of
    /// <c>tests/corpus/features/sheet-cf-data-bar-only.xlsx</c> holds no text-showing operator at
    /// all. The row's height is unaffected, which is why this is asked at drawing time rather
    /// than folded into the cell's text.
    /// <para>
    /// Two families answer it and the two arms are separate lines in the reference
    /// (<c>:1694-1697</c>), so either alone is enough.
    /// </para>
    /// </remarks>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public bool HidesValue(int row, int column)
        => BarAt(row, column) is { ShowValue: false } || IconAt(row, column) is { ShowValue: false };

    /// <summary>Sets the format everything with no other answer takes.</summary>
    /// <param name="format">A handle from <see cref="Intern"/>.</param>
    public void SetDefault(int format)
    {
        if (format <= 0) return;

        _default = format;
        _hasDefaults = true;
    }

    /// <summary>
    /// The positions that state a format of their own, whatever it paints.
    /// </summary>
    /// <remarks>
    /// For <see cref="SheetDecorationArea"/>, which has to know where a border or a fill is
    /// before it can say how far the sheet prints.
    /// </remarks>
    internal IEnumerable<(int Row, int Column, SheetCellDecoration Format)> Cells
    {
        get
        {
            foreach (KeyValuePair<(int Row, int Column), int> entry in _cells)
                yield return (entry.Key.Row, entry.Key.Column, _palette[entry.Value]);
        }
    }

    /// <summary>
    /// The rectangles that state a format, as the file's repeat counts wrote them.
    /// </summary>
    /// <remarks>
    /// <see cref="Cells"/> answers only what was written one cell at a time; a repeat is kept
    /// whole, and <see cref="SheetDecorationArea"/> wants it that way — Calc's own attribute
    /// store is a list of runs and its print-area scan measures the runs' lengths, so a
    /// rectangle is the shape that scan needs rather than a shape it has to rebuild.
    /// </remarks>
    internal IEnumerable<(int FirstRow, int LastRow, int FirstColumn, int LastColumn, SheetCellDecoration Format)>
        CellBlocks
    {
        get
        {
            foreach (SheetFormatBlock block in _blocks.Blocks)
            {
                yield return (
                    block.FirstRow, block.LastRow, block.FirstColumn, block.LastColumn,
                    _palette[block.Index]);
            }
        }
    }

    /// <summary>The rows that state a format of their own.</summary>
    /// <inheritdoc cref="Cells"/>
    internal IEnumerable<(int Row, SheetCellDecoration Format)> Rows
    {
        get
        {
            foreach (KeyValuePair<int, int> entry in _rows)
                yield return (entry.Key, _palette[entry.Value]);
        }
    }

    /// <summary>
    /// The runs of whole columns that state a format, as the file wrote them.
    /// </summary>
    /// <remarks>
    /// The <em>last</em> column of a run is load-bearing and not merely informative. Calc's
    /// <c>ScTable::ApplyPatternArea</c> materialises a column only when the range being formatted
    /// stops short of the sheet's last column; a run that reaches it is written to the
    /// unallocated-column default instead and allocates nothing
    /// (<c>sc/source/core/data/table2.cxx:2988-2998</c>). Since the print-area scan iterates
    /// <c>aCol</c>, that is what decides how far right it can look — see
    /// <see cref="SheetDecorationArea"/>.
    /// </remarks>
    internal IEnumerable<(int First, int Last, SheetCellDecoration Format)> ColumnRuns
    {
        get
        {
            foreach ((int first, int last, int format) in _columns)
                yield return (first, last, _palette[format]);
        }
    }

    /// <summary>
    /// What a cell in a column takes when neither it nor its row states anything.
    /// </summary>
    /// <param name="column">The zero-based column.</param>
    internal SheetCellDecoration ColumnDefault(int column)
    {
        // Walked backwards for the same reason At does: a later run wins an overlap.
        for (int at = _columns.Count - 1; at >= 0; at--)
        {
            (int first, int last, int format) = _columns[at];
            if (column >= first && column <= last) return _palette[format];
        }

        return _palette[_default];
    }

    /// <summary>What is painted behind and around one cell.</summary>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public SheetCellDecoration At(int row, int column)
    {
        SheetCellDecoration stated = Stated(row, column);

        return _conditional.Count > 0 && _conditional.TryGetValue((row, column), out Colour fill)
            ? stated with { Background = fill }
            : stated;
    }

    /// <summary>What a cell states, before any conditional format is applied over it.</summary>
    private SheetCellDecoration Stated(int row, int column)
    {
        if (_cells.TryGetValue((row, column), out int cell)) return _palette[cell];

        // The repeats, kept as rectangles. A real workbook states thousands of them, so the
        // store groups them by row range and this is a binary search rather than a scan.
        if (_blocks.At(row, column) is { } block) return _palette[block];

        if (_rows.TryGetValue(row, out int inRow)) return _palette[inRow];

        // Walked backwards so a later run wins an overlap, which is what both formats need:
        // SpreadsheetML writes a sheet-wide <col> and then narrower ones over the top of it.
        for (int at = _columns.Count - 1; at >= 0; at--)
        {
            (int first, int last, int format) = _columns[at];
            if (column >= first && column <= last) return _palette[format];
        }

        return _palette[_default];
    }
}

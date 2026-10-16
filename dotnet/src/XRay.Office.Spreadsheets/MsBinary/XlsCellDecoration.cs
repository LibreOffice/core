using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.MsBinary;

/// <summary>
/// The parts of a BIFF <c>XF</c> record that decide what a cell looks like behind its text.
/// </summary>
/// <remarks>
/// Kept as the record's own numbers rather than as resolved colours, because a colour index
/// cannot be resolved until the <c>PALETTE</c> record has been seen and <c>PALETTE</c> may
/// follow the <c>XF</c>s in the globals substream. Resolution therefore happens once, when a
/// sheet asks.
/// </remarks>
/// <param name="LeftLine">The left edge's line style, 0-13.</param>
/// <param name="RightLine">The right edge's.</param>
/// <param name="TopLine">The top edge's.</param>
/// <param name="BottomLine">The bottom edge's.</param>
/// <param name="LeftColour">The left edge's palette index.</param>
/// <param name="RightColour">The right edge's.</param>
/// <param name="TopColour">The top edge's.</param>
/// <param name="BottomColour">The bottom edge's.</param>
/// <param name="Pattern">The fill pattern, 0 for none and 1 for solid.</param>
/// <param name="ForeColour">The pattern's foreground palette index.</param>
/// <param name="BackColour">Its background palette index.</param>
/// <param name="StatesBorder">True when this XF states its own border rather than inheriting.</param>
/// <param name="StatesArea">True when it states its own fill.</param>
internal readonly record struct XlsXfDecoration(
    int LeftLine,
    int RightLine,
    int TopLine,
    int BottomLine,
    int LeftColour,
    int RightColour,
    int TopColour,
    int BottomColour,
    int Pattern,
    int ForeColour,
    int BackColour,
    bool StatesBorder,
    bool StatesArea)
{
    /// <summary>Reads the BIFF8 layout: two border dwords and an area word.</summary>
    /// <remarks>
    /// <c>XclImpCellBorder::FillFromXF8</c> and <c>XclImpCellArea::FillFromXF8</c>
    /// (<c>sc/source/filter/excel/xistyle.cxx:914</c> and <c>:1065</c>). The fill pattern lives
    /// in the <em>second border</em> dword rather than in the area word, which is the field
    /// placement most easily got wrong: taking it from the area word gives every filled cell
    /// pattern zero and no fill at all.
    /// </remarks>
    public static XlsXfDecoration FromBiff8(
        uint border1, uint border2, ushort area, bool border, bool fill)
        => new(
            (int)(border1 & 0xF),
            (int)((border1 >> 4) & 0xF),
            (int)((border1 >> 8) & 0xF),
            (int)((border1 >> 12) & 0xF),
            (int)((border1 >> 16) & 0x7F),
            (int)((border1 >> 23) & 0x7F),
            (int)(border2 & 0x7F),
            (int)((border2 >> 7) & 0x7F),
            (int)((border2 >> 26) & 0x3F),
            area & 0x7F,
            (area >> 7) & 0x7F,
            border,
            fill);

    /// <summary>Reads the BIFF5 layout, whose bottom edge lives in the area dword.</summary>
    /// <remarks>
    /// <c>XclImpCellBorder::FillFromXF5</c> (<c>xistyle.cxx:901</c>). BIFF5 packs three edges
    /// into the border dword and the fourth into the top of the area dword, and its line-style
    /// fields are three bits wide rather than four.
    /// </remarks>
    public static XlsXfDecoration FromBiff5(uint area, uint border, bool hasBorder, bool fill)
        => new(
            (int)((border >> 3) & 0x7),
            (int)((border >> 6) & 0x7),
            (int)(border & 0x7),
            (int)((area >> 22) & 0x7),
            (int)((border >> 16) & 0x7F),
            (int)((border >> 23) & 0x7F),
            (int)((border >> 9) & 0x7F),
            (int)((area >> 25) & 0x7F),
            (int)((area >> 16) & 0x3F),
            (int)(area & 0x7F),
            (int)((area >> 7) & 0x7F),
            hasBorder,
            fill);
}

/// <summary>
/// The workbook's colour palette and its cell formats, and the conversion of the two into
/// something a page can paint.
/// </summary>
/// <remarks>
/// Its own type rather than more state on the workbook reader, because the reader is a
/// single-pass record walker with quite enough state already and none of this is needed until a
/// sheet is laid out.
/// </remarks>
internal sealed class XlsDecorationTable
{
    private readonly List<Colour> _palette = [];
    private readonly List<XlsXfDecoration> _formats = [];
    private readonly List<(bool IsCellXf, int Parent)> _lineage = [];

    /// <summary>True when no <c>XF</c> in the workbook paints anything.</summary>
    public bool IsEmpty => _formats.Count == 0;

    /// <summary>Records one <c>XF</c>'s decoration, in record order.</summary>
    /// <param name="decoration">What the record paints.</param>
    /// <param name="isCellXf">True for a cell <c>XF</c>, false for a style <c>XF</c>.</param>
    /// <param name="parentIndex">The <c>XF</c> index this one names as its style.</param>
    public void Add(XlsXfDecoration decoration, bool isCellXf, int parentIndex)
    {
        _formats.Add(decoration);
        _lineage.Add((isCellXf, parentIndex));
    }

    /// <summary>
    /// Reads a <c>PALETTE</c> record: the colours the workbook redefined.
    /// </summary>
    /// <remarks>
    /// The table it writes starts at index 8, not zero: indices 0 to 7 are the fixed colours no
    /// workbook may change (<c>EXC_COLOR_USEROFFSET</c>,
    /// <c>sc/source/filter/inc/xlstyle.hxx:205</c>). A reader that lands the table at zero
    /// shifts every colour in the file by eight entries, which is not subtle — black cells come
    /// out cyan.
    /// </remarks>
    /// <param name="count">How many entries the record declares.</param>
    /// <param name="colours">The entries, each already unpacked to RGB.</param>
    public void SetPalette(int count, IEnumerable<Colour> colours)
    {
        ArgumentNullException.ThrowIfNull(colours);

        _palette.Clear();
        foreach (Colour colour in colours.Take(Math.Max(0, count))) _palette.Add(colour);
    }

    /// <summary>
    /// What one <c>XF</c> index paints, or <see cref="SheetCellDecoration.None"/> when it paints
    /// nothing.
    /// </summary>
    /// <param name="index">The <c>XF</c> index a cell, row or column stated.</param>
    public SheetCellDecoration FormatOf(int index)
    {
        if (index < 0 || index >= _formats.Count) return SheetCellDecoration.None;

        XlsXfDecoration xf = _formats[index];
        bool ownAttributes = HasStyleParent(index);

        Colour? background = null;
        if ((xf.StatesArea || ownAttributes) && xf.Pattern != XlsPatternFill.None)
        {
            // A hatch is the two colours mixed at the weight the pattern number carries, not
            // either of them whole — see `XlsPatternFill`. `solid` is the same sum at a ratio of
            // nothing, which returns the foreground, so it is not a special case here. An `XF`
            // marks all three of its area parts used together (`XclImpCellArea::SetUsedFlags`,
            // `xistyle.cxx`:1036-1039), so both colours are always stated and the window-colour
            // fallbacks below are only reached for an index the palette does not hold.
            background = XlsPatternFill.Resolve(
                xf.Pattern,
                Colour(xf.ForeColour) ?? Core.Graphics.Colour.Black,
                Colour(xf.BackColour) ?? Core.Graphics.Colour.White);
        }

        SheetCellBorders borders = xf.StatesBorder || ownAttributes
            ? new SheetCellBorders(
                Edge(xf.LeftLine, xf.LeftColour),
                Edge(xf.RightLine, xf.RightColour),
                Edge(xf.TopLine, xf.TopColour),
                Edge(xf.BottomLine, xf.BottomColour))
            : SheetCellBorders.None;

        return background is null && borders.IsNone
            ? SheetCellDecoration.None
            : new SheetCellDecoration(background, borders);
    }

    /// <summary>
    /// True when this <c>XF</c> is a cell <c>XF</c> whose named style exists, in which case its
    /// own border and fill bytes are what Calc paints whatever its "used" flags say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The "used" flags do <em>not</em> decide the border or the fill of a cell <c>XF</c>.
    /// <c>XclImpXF::CreatePattern</c> (<c>sc/source/filter/excel/xistyle.cxx:1291-1294</c>) turns
    /// each flag <em>on</em> whenever the parent style either states nothing or states something
    /// different:
    /// </para>
    /// <code>
    /// if( !mbBorderUsed )
    ///     mbBorderUsed = !pParentXF->mbBorderUsed || !(maBorder == pParentXF->maBorder);
    /// </code>
    /// <para>
    /// So a cleared flag survives only when the parent states the <em>same</em> border — and the
    /// border the cell then inherits through its style sheet is that identical one. Both branches
    /// therefore land on this record's own bytes, which is why honouring the flag literally is a
    /// defect rather than a simplification: it drops borders and fills that Calc paints. Measured
    /// on <c>7-memento-2015-transports-aeriens-b.xls</c>, whose <c>XF</c> 115 carries a thin
    /// <c>#0066CC</c> box with the flag cleared and a parent style carrying the same box.
    /// </para>
    /// <para>
    /// A missing parent is the one case the flag still rules: <c>GetXF</c> returns null, Calc
    /// skips the block above, and the attribute stays unset. Style <c>XF</c>s have no parent to
    /// consult at all.
    /// </para>
    /// </remarks>
    /// <param name="index">The <c>XF</c> index.</param>
    private bool HasStyleParent(int index)
    {
        if (index >= _lineage.Count) return false;

        (bool isCellXf, int parent) = _lineage[index];
        return isCellXf && parent >= 0 && parent < _formats.Count;
    }

    /// <summary>
    /// One edge, from BIFF's fourteen line styles.
    /// </summary>
    /// <remarks>
    /// The table is <c>ppnLineParam</c> (<c>sc/source/filter/excel/xistyle.cxx:965-980</c>),
    /// whose widths are <c>EXC_BORDER_HAIR</c> 1, <c>EXC_BORDER_THIN</c> 15,
    /// <c>EXC_BORDER_MEDIUM</c> 35 and <c>EXC_BORDER_THICK</c> 50 twips
    /// (<c>sc/source/filter/inc/xlconst.hxx:250-253</c>). Style 6, <c>double</c>, is a
    /// <em>thick</em> rule rather than a thin one — an easy off-by-one against the OOXML table,
    /// where <c>double</c> is also thick but the neighbouring entries differ.
    /// <para>
    /// <strong>And its two lines are 10 twips each with a 30-twip gap, not a third of the rule
    /// each.</strong> BIFF states one width and the style name, so the split comes from
    /// <c>DOUBLE_THIN</c>'s own <c>BorderWidthImpl</c> — <c>CHANGE_DIST</c> with the two lines
    /// pinned at 10 (<c>editeng/source/items/borderline.cxx:358-360</c>) — where the OOXML
    /// filter states 10/15/10 outright. Confirmed at 26.2.4.2 on the same workbook saved both
    /// ways: as <c>.xls</c> the two lines stroke at 0.50002 pt with their centres 1.984 pt
    /// apart, a 1.484 pt gap, against 1.248 pt apart as <c>.xlsx</c>.
    /// </para>
    /// </remarks>
    private SheetBorder Edge(int style, int colour)
    {
        if (style == 6)
        {
            return new SheetBorder(
                Length.FromTwips(10),
                Length.FromTwips(30),
                Length.FromTwips(10),
                Colour(colour) ?? Core.Graphics.Colour.Black);
        }

        (int twips, SheetBorderPattern pattern) = style switch
        {
            1 => (15, SheetBorderPattern.Solid),
            2 => (35, SheetBorderPattern.Solid),
            3 => (15, SheetBorderPattern.FineDashed),
            4 => (15, SheetBorderPattern.Dotted),
            5 => (50, SheetBorderPattern.Solid),
            7 => (1, SheetBorderPattern.Solid),
            8 => (35, SheetBorderPattern.Dashed),
            9 => (15, SheetBorderPattern.DashDot),
            10 => (35, SheetBorderPattern.DashDot),
            11 => (15, SheetBorderPattern.DashDotDot),
            12 => (35, SheetBorderPattern.DashDotDot),
            13 => (35, SheetBorderPattern.DashDot),
            _ => (0, SheetBorderPattern.Solid),
        };

        return twips == 0
            ? SheetBorder.None
            : SheetBorder.Line(
                Length.FromTwips(twips), Colour(colour) ?? Core.Graphics.Colour.Black, pattern);
    }

    /// <summary>
    /// A palette index, honouring the workbook's own table and the fixed entries around it.
    /// </summary>
    /// <remarks>
    /// 64 and 65 are the system window's text and background, which have no entry at all and
    /// render as black on white; 0 to 7 are fixed and cannot be overridden. Everything between
    /// is the workbook's <c>PALETTE</c>, falling back to Excel's default when it wrote none.
    /// </remarks>
    private Colour? Colour(int index) => index switch
    {
        64 or 0x7FFF => Core.Graphics.Colour.Black,
        65 => Core.Graphics.Colour.White,
        >= 8 when index - 8 < _palette.Count => _palette[index - 8],
        >= 0 and < 64 => Core.Graphics.Colour.FromRgb(DefaultPalette[index]),
        _ => null,
    };

    /// <summary>
    /// Excel's default 64-entry palette, used when the workbook redefines none.
    /// </summary>
    /// <remarks>
    /// The same table SpreadsheetML's <c>indexed</c> colours use, which is not a coincidence:
    /// the OOXML filter inherited it from the BIFF one.
    /// </remarks>
    private static readonly uint[] DefaultPalette =
    [
        0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
        0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
        0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080,
        0x9999FF, 0x993366, 0xFFFFCC, 0xCCFFFF, 0x660066, 0xFF8080, 0x0066CC, 0xCCCCFF,
        0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
        0x00CCFF, 0xCCFFFF, 0xCCFFCC, 0xFFFF99, 0x99CCFF, 0xFF99CC, 0xCC99FF, 0xFFCC99,
        0x3366FF, 0x33CCCC, 0x99CC00, 0xFFCC00, 0xFF9900, 0xFF6600, 0x666699, 0x969696,
        0x003366, 0x339966, 0x003300, 0x333300, 0x993300, 0x993366, 0x333399, 0x333333,
    ];
}

/// <summary>
/// Which <c>XF</c> each cell, row and column of one sheet uses, before it is resolved.
/// </summary>
/// <remarks>
/// Collected while the sheet's records go by and turned into a
/// <see cref="SheetFormatting"/> once, at the end. Collecting indices rather than resolved
/// formats keeps the walk cheap and keeps the palette's arrival order from mattering.
/// </remarks>
internal sealed class XlsSheetDecoration
{
    private readonly Dictionary<(int Row, int Column), int> _cells = [];
    private readonly Dictionary<int, int> _rows = [];
    private readonly List<(int First, int Last, int Xf)> _columns = [];

    /// <summary>
    /// Notes the <c>XF</c> a cell record carried, index zero included.
    /// </summary>
    /// <remarks>
    /// Index zero is a real format and not an absence — it is the workbook's first <c>XF</c> —
    /// and a cell stating it has to beat whatever its <c>COLINFO</c> says, which is BIFF's own
    /// precedence. Dropping it would leave a plain cell inside a filled column filled.
    /// </remarks>
    public void SetCell(int row, int column, int xf)
    {
        if (row < 0 || column < 0 || xf < 0) return;
        _cells[(row, column)] = xf;
    }

    /// <summary>Notes a <c>ROW</c> record's own format, which only counts when it says so.</summary>
    /// <remarks>
    /// The flag is <c>fGhostDirty</c>, bit 7 of the row's option word: without it the
    /// <c>ixfe</c> field is present but inert, and honouring it anyway paints rows Excel does
    /// not (<c>XclImpXFRangeBuffer</c> consults <c>bDefaultFmt</c>,
    /// <c>sc/source/filter/excel/impop.cxx:1041</c>).
    /// </remarks>
    public void SetRow(int row, int xf) => _rows[row] = xf;

    /// <summary>Notes a <c>COLINFO</c> record's format.</summary>
    public void SetColumns(int first, int last, int xf)
    {
        if (last < first || xf < 0) return;
        _columns.Add((first, last, xf));
    }

    /// <summary>Resolves everything collected against the workbook's formats.</summary>
    /// <param name="table">The workbook's palette and cell formats.</param>
    public SheetFormatting Resolve(XlsDecorationTable table)
    {
        ArgumentNullException.ThrowIfNull(table);

        if (table.IsEmpty || (_cells.Count == 0 && _rows.Count == 0 && _columns.Count == 0))
            return SheetFormatting.Empty;

        SheetFormatting formatting = new();
        Dictionary<int, int> handles = [];

        foreach ((int first, int last, int xf) in _columns)
            formatting.SetColumns(first, last, Handle(xf));

        foreach ((int row, int xf) in _rows) formatting.SetRow(row, Handle(xf));

        foreach (((int row, int column), int xf) in _cells)
            formatting.SetCell(row, column, Handle(xf));

        return formatting.IsEmpty ? SheetFormatting.Empty : formatting;

        int Handle(int xf)
        {
            if (handles.TryGetValue(xf, out int cached)) return cached;

            SheetCellDecoration format = table.FormatOf(xf);
            int handle = format.IsNone ? 0 : formatting.Intern(format);
            handles[xf] = handle;
            return handle;
        }
    }
}

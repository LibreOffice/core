using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// How much space a cell leaves between its border and its text.
/// </summary>
/// <remarks>
/// Every format has this and every format defaults it differently — Word to 108 twips left and right and
/// nothing vertically, ODF to 0.097 cm on all four sides — so the reader resolves it and this carries the
/// answer. It comes out of the cell's width, which makes it a line-breaking matter rather than a cosmetic
/// one: a cell 2 cm wide with 1 mm padding breaks its text at 18 mm.
/// </remarks>
/// <param name="Left">The gap at the cell's left edge.</param>
/// <param name="Right">At its right edge.</param>
/// <param name="Top">Above the first line.</param>
/// <param name="Bottom">Below the last.</param>
public readonly record struct CellPadding(Length Left, Length Right, Length Top, Length Bottom)
{
    /// <summary>Word's default: 0.19 cm to the left and right of the text, nothing above or below.</summary>
    public static CellPadding Word { get; } =
        new(Length.FromTwips(108), Length.FromTwips(108), Length.Zero, Length.Zero);

    /// <summary>What Writer gives a table drawn in it: 0.097 cm on all four sides.</summary>
    public static CellPadding Writer { get; } = Uniform(Length.FromMm100(97));

    /// <summary>The same gap on all four sides.</summary>
    public static CellPadding Uniform(Length all) => new(all, all, all, all);

    /// <summary>How much width the two horizontal gaps take together.</summary>
    public Length Horizontal => Left + Right;

    /// <summary>How much height the two vertical gaps take together.</summary>
    public Length Vertical => Top + Bottom;
}

/// <summary>
/// One edge of a cell: how thick its border is and what colour.
/// </summary>
/// <remarks>
/// <para>
/// <em>"A width and a colour and no style, because a style is a rasteriser's problem"</em> stood here and is
/// measurably wrong, in the one way that matters: the style decides the width. Word states the width of
/// <em>a</em> line and the style says how many there are, so a 3 pt <c>double</c> covers 9 pt of the page
/// and a 3 pt <c>thick</c> covers 6 — which is a row height, not a rasteriser's problem. See
/// <see cref="BorderRules"/> for the measurement and the arithmetic.
/// </para>
/// <para>
/// The four formats do disagree on the set of names, and that is what the reading is for:
/// <see cref="BorderRules.WordStyleOf"/> takes OOXML's forty-odd, a <c>BRC</c>'s <c>brcType</c> is the same
/// number already, and RTF's control words and ODF's CSS words map onto it too.
/// </para>
/// </remarks>
/// <param name="Width">How thick the whole border is, both rules and the gap; zero means there is none.</param>
/// <param name="Colour">What colour it is drawn in.</param>
/// <param name="Line">The line it is drawn as. <see cref="BorderLine.Solid"/> for most of the corpus.</param>
public readonly record struct TableBorder(
    Length Width, Colour Colour, BorderLine Line = BorderLine.Solid)
{
    /// <summary>True when the edge has no border at all.</summary>
    public bool IsNone => Width <= Length.Zero;

    /// <summary>How the rule divides into strokes — one for a plain border, two for a double.</summary>
    public BorderBands Bands => BorderRules.Bands(Line, Width);
}

/// <summary>The four edges of a cell.</summary>
/// <remarks>
/// All four, rather than a nullable per side, because "no border" is a border of zero width and the two need
/// no telling apart: nothing is drawn either way, and nothing takes space either way.
/// </remarks>
/// <param name="Left">Its left edge.</param>
/// <param name="Right">Its right edge.</param>
/// <param name="Top">Its top edge.</param>
/// <param name="Bottom">Its bottom edge.</param>
public readonly record struct CellBorders(
    TableBorder Left, TableBorder Right, TableBorder Top, TableBorder Bottom)
{
    /// <summary>The same border on all four edges.</summary>
    public static CellBorders Uniform(TableBorder border)
        => new(border, border, border, border);

    /// <summary>True when no edge has a border.</summary>
    public bool IsNone => Left.IsNone && Right.IsNone && Top.IsNone && Bottom.IsNone;
}

/// <summary>
/// Which way a cell's text runs, when the document turns it out of the ordinary direction.
/// </summary>
/// <remarks>
/// <para>
/// The three answers LibreOffice's own DOCX importer reduces <c>w:textDirection</c>'s six values to
/// (<c>sw/source/writerfilter/dmapper/DomainMapperTableManager.cxx</c>:325-350): <c>btLr</c> becomes
/// <c>WritingMode2::BT_LR</c>, <c>tbRl</c> and <c>tbRlV</c> both become <c>TB_RL</c>, and <c>lrTb</c>,
/// <c>lrTbV</c> and <c>tbLrV</c> are all upright — the last of those is ignored outright with the comment
/// "we can't handle these". Measured against the installed 26.2.4.2, which confirms all six.
/// </para>
/// <para>
/// A direction rather than an angle, because what the two turned values change is not only the glyphs'
/// rotation: the line breaks at the cell's <em>height</em>, successive lines stack across its
/// <em>width</em>, and <see cref="PageTableCell.VerticalAlignment"/> stops meaning anything vertical. An
/// angle would carry the first of those and none of the rest.
/// </para>
/// </remarks>
public enum CellTextDirection
{
    /// <summary>Upright, left to right — every format's default and almost every cell.</summary>
    LeftToRight,

    /// <summary>
    /// Turned a quarter turn anticlockwise: glyphs run up the page and lines stack rightwards.
    /// </summary>
    /// <remarks>
    /// OOXML's <c>btLr</c>, and the only turned direction the sample corpus contains — 111 occurrences
    /// across ten of its two hundred word-processing documents, all of them DOCX.
    /// </remarks>
    BottomToTopLeftToRight,

    /// <summary>
    /// Turned a quarter turn clockwise: glyphs run down the page and lines stack leftwards.
    /// </summary>
    /// <remarks>OOXML's <c>tbRl</c> and <c>tbRlV</c>.</remarks>
    TopToBottomRightToLeft,
}

/// <summary>
/// Where text sits in a container taller than itself.
/// </summary>
/// <remarks>
/// A table cell's <c>w:vAlign</c> and a shape's <c>wps:bodyPr/@anchor</c> are the same question with
/// the same three answers, so they share an enum rather than each having one — which is why this is
/// no longer named for the cell that first needed it.
/// </remarks>
public enum VerticalTextAlignment
{
    /// <summary>Against the top, which is every format's default.</summary>
    Top,

    /// <summary>Centred in the spare height.</summary>
    Middle,

    /// <summary>Against the bottom.</summary>
    Bottom,
}

/// <summary>
/// A table waiting to be paginated: a column grid, and rows of cells that flow inside it.
/// </summary>
/// <remarks>
/// <para>
/// The grid is stated as column widths rather than as a count, because that is what every format states
/// and because a cell's width is what decides its line breaks. A cell spanning two columns is one cell
/// with a <see cref="PageTableCell.ColumnSpan"/> of two, whose width is the two columns' widths added —
/// which is the reverse of how Writer stores it internally, where the width is primary and the span
/// implied, but the same information.
/// </para>
/// <para>
/// Heights are mostly absent on purpose: a row is as tall as its tallest cell, and a cell is as tall as its
/// content at its own width. What a row can state is a <em>floor</em>
/// (<see cref="PageTableRow.MinHeight"/>), honoured when the content is shorter and ignored when it is
/// taller, which is what three of the four spellings mean. The fourth is
/// <see cref="PageTableRow.HasExactHeight"/>, the one that really is a height: it clips.
/// </para>
/// </remarks>
public sealed record PageTable : PageBlock
{
    /// <summary>How many columns and rows are laid out before the rest are dropped.</summary>
    /// <remarks>
    /// A guard on untrusted input. Word's own limit is 63 columns and Writer's is far higher, but a
    /// generated file can declare a grid of any size, and the layout cost is the product of the two.
    /// </remarks>
    public const int MaxColumns = 256;

    /// <inheritdoc cref="MaxColumns"/>
    public const int MaxRows = 20000;

    /// <summary>The grid's column widths, left to right.</summary>
    /// <remarks>
    /// What the file stated. When <see cref="ColumnFit"/> is not null some of these are placeholders for a
    /// column that stated nothing, and <see cref="WidthsWithin"/> rather than this is what the table is laid
    /// out at.
    /// </remarks>
    public required IReadOnlyList<Length> ColumnWidths { get; init; }

    /// <summary>
    /// How to size the columns the file left without a width, or null when it stated all of them.
    /// </summary>
    /// <remarks>
    /// Null is the ordinary case and deliberately the untouched one: a table that declares its grid is laid
    /// out from <see cref="ColumnWidths"/> and never reaches the fitting arithmetic. Only a table missing at
    /// least one width carries this — see <see cref="TableColumnFit"/> for what the two families then do,
    /// and for why it is not the content-measuring auto-layout it looks like it should be.
    /// </remarks>
    public TableColumnFit? ColumnFit { get; init; }

    /// <summary>
    /// The table's width as a percentage of the area it sits in, or null when it is an absolute width.
    /// </summary>
    /// <remarks>
    /// <para>
    /// OOXML's <c>w:tblW w:type="pct"</c>, whose unit is fiftieths of a percent, and which
    /// <c>DomainMapperTableManager::sprm</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapperTableManager.cxx</c>:191) turns into
    /// <c>SizeType::VARIABLE</c> with a percentage <em>clamped to 100</em> — a table stating 102.24% is
    /// laid out at 100%, not at 102.24%. The grid is then restated in the same proportions at that
    /// width, which is what <see cref="WidthsWithin"/> does.
    /// </para>
    /// <para>
    /// Not a nicety: <c>ESPN-R - MCF - RA - Ed1.docx</c>'s running header holds a
    /// <c>w:tblW w:w="5000" w:type="pct"</c> over a grid summing to 9633 twips, so ignoring the
    /// percentage drew that table 481.65 pt wide where Writer draws it 714.35 pt — the whole landscape
    /// text width — and <c>Page 26/58</c> then broke across two lines in a cell that should hold it on
    /// one. Scaling the grid to the stated percentage reproduces all four of the reference's column
    /// boundaries to within 0.04 pt.
    /// </para>
    /// </remarks>
    public int? RelativeWidth { get; init; }

    /// <summary>The rows, top to bottom.</summary>
    public required IReadOnlyList<PageTableRow> Rows { get; init; }

    /// <summary>
    /// The rows with every horizontal edge two cells share resolved to the one border both sides get.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A grid line between two rows is one line, and the two cells either side of it can disagree about it.
    /// The document is not read twice here: the winner is the <em>wider</em> of the two facing edges and it
    /// is written back to both, so a cell stating <c>w:top w:val="nil"</c> under a cell stating a bottom
    /// border ends up with that border. <see cref="Rows"/> keeps what the file said.
    /// </para>
    /// <para>
    /// This is not a drawing nicety — the two consumers are the <em>height</em> of the row, since half the
    /// resolved border is charged to each of the rows it separates, and the <em>drawing</em>, which without
    /// it leaves a hole wherever the two facing cells are on different pages and only one of them states
    /// the edge.
    /// </para>
    /// <para>
    /// Measured at 26.2.4.2 on <c>probes/wordstable-r131/borderprobe.docx</c>, eight two-row arms rendered
    /// three times into separate profiles: <c>bottom=1pt / top=nil</c> and <c>bottom=nil / top=1pt</c> both
    /// draw the line and are the same height as the arm that states both, <c>nil/nil</c> draws nothing and
    /// is a point shorter, and a 3 pt facing a 0.5 pt draws 3 pt whichever side states which. It is
    /// ECMA-376 Part 1 §17.4.62's conflict resolution as far as width goes; the style half of that
    /// precedence is not modelled, and a tie keeps the cell's own.
    /// </para>
    /// <para>
    /// Horizontal edges only. A vertical edge's two cells are always drawn on the same page, so the
    /// consolidation in <c>PageDrawing.Edges</c> already produces one stroke for them, and no row height
    /// depends on them.
    /// </para>
    /// </remarks>
    public IReadOnlyList<PageTableRow> RowsWithSharedEdges => _shared ??= ShareHorizontalEdges(Rows);

    private IReadOnlyList<PageTableRow>? _shared;

    /// <summary>Resolves each horizontal edge two cells share, giving both of them the wider border.</summary>
    private static IReadOnlyList<PageTableRow> ShareHorizontalEdges(IReadOnlyList<PageTableRow> rows)
    {
        int count = Math.Min(rows.Count, MaxRows);
        if (count < 2) return rows;

        // Which cell covers each column of each row, named by the row it was declared in and its place in
        // that row's list — a cell spanning rows appears in every row it covers but belongs to one.
        Dictionary<(int Row, int Column), Where> grid = [];
        for (int row = 0; row < count; row++)
        {
            IReadOnlyList<PageTableCell> cells = rows[row].Cells;
            for (int index = 0; index < cells.Count; index++)
            {
                PageTableCell cell = cells[index];
                int span = Math.Min(Math.Max(1, cell.RowSpan), count - row);
                for (int down = 0; down < span; down++)
                {
                    for (int column = cell.Column; column < cell.ColumnEnd; column++)
                    {
                        grid[(row + down, column)] = new Where(row, index, down == span - 1);
                    }
                }
            }
        }

        // The border facing each cell across each of its two horizontal edges, gathered before anything
        // is written back so that no cell is resolved against an already-resolved neighbour — and taken
        // as the NARROWEST over the columns the cell covers, not the widest. A cell facing three cells
        // of which one states nothing is separated from that one by nothing, and this model carries one
        // border per edge rather than one per column: taking the widest would draw a line across the
        // whole cell where the reference draws it across part, which on `A_320.doc` is 75 pt a page.
        Dictionary<(int Row, int Index, bool Top), Length> facing = [];

        foreach (((int row, int column), Where here) in grid)
        {
            // `below.Row == row + 1` is what says the cell under this one STARTS there: a cell spanning
            // rows appears in every row it covers and is declared in one, so a cell running past this
            // boundary faces nothing across it.
            bool under = grid.TryGetValue((row + 1, column), out Where below) && below.Row == row + 1;
            bool joins = here.Last && under;

            if (here.Last)
            {
                Narrow((here.Row, here.Index, false),
                    joins ? rows[below.Row].Cells[below.Index].Borders.Top.Width : Length.Zero);
            }

            if (joins)
            {
                Narrow((below.Row, below.Index, true),
                    rows[here.Row].Cells[here.Index].Borders.Bottom.Width);
            }
            else if (under)
            {
                Narrow((below.Row, below.Index, true), Length.Zero);
            }
        }

        void Narrow((int, int, bool) key, Length width)
        {
            facing[key] = facing.TryGetValue(key, out Length least) && least < width ? least : width;
        }

        if (facing.Count == 0) return rows;

        List<PageTableRow> resolved = new(rows.Count);
        for (int row = 0; row < rows.Count; row++)
        {
            IReadOnlyList<PageTableCell> cells = rows[row].Cells;
            List<PageTableCell>? replaced = null;

            for (int index = 0; row < count && index < cells.Count; index++)
            {
                PageTableCell cell = cells[index];
                CellBorders borders = cell.Borders;
                int last = Math.Min(row + Math.Max(1, cell.RowSpan), count) - 1;

                if (facing.TryGetValue((row, index, true), out Length above)
                    && above > borders.Top.Width)
                {
                    borders = borders with { Top = Facing(rows, row - 1, cell, above, bottom: true) };
                }

                if (facing.TryGetValue((row, index, false), out Length under)
                    && under > borders.Bottom.Width)
                {
                    borders = borders with { Bottom = Facing(rows, last + 1, cell, under, bottom: false) };
                }

                if (borders == cell.Borders) continue;

                replaced ??= [.. cells];
                replaced[index] = cell with { Borders = borders };
            }

            resolved.Add(replaced is null ? rows[row] : rows[row] with { Cells = replaced });
        }

        return resolved;
    }

    /// <summary>Where one cell of the grid was declared, and whether this is the last row it covers.</summary>
    private readonly record struct Where(int Row, int Index, bool Last);

    /// <summary>
    /// The border a cell inherits across one of its horizontal edges: the widest facing one, carrying the
    /// colour and line of the neighbour it came from rather than a made-up black.
    /// </summary>
    private static TableBorder Facing(
        IReadOnlyList<PageTableRow> rows, int row, PageTableCell cell, Length width, bool bottom)
    {
        if (row >= 0 && row < rows.Count)
        {
            foreach (PageTableCell other in rows[row].Cells)
            {
                if (other.ColumnEnd <= cell.Column || other.Column >= cell.ColumnEnd) continue;

                TableBorder edge = bottom ? other.Borders.Bottom : other.Borders.Top;
                if (edge.Width == width) return edge;
            }
        }

        return new TableBorder(width, Colour.Black);
    }

    /// <summary>How far the table's left edge sits from the body area's.</summary>
    /// <remarks>
    /// Its own value rather than a paragraph indent, because a table is indented as a whole and can be
    /// negative — a table pulled into the left margin is legal and used for full-bleed layouts.
    /// </remarks>
    public Length LeftIndent { get; init; }

    /// <summary>The space above the table.</summary>
    public Length SpaceBefore { get; init; }

    /// <summary>The space below it.</summary>
    public Length SpaceAfter { get; init; }

    /// <summary>
    /// How many rows at the top repeat when the table crosses a page break.
    /// </summary>
    /// <remarks>
    /// A count rather than a flag per row, matching <c>SwTable::GetRowsToRepeat</c>, because the feature is
    /// "the first N rows are the heading" — a repeat flag on a row further down does not make the rows
    /// above it headings, and every format states it as a run from the top.
    /// </remarks>
    public int HeaderRowCount { get; init; }

    /// <summary>
    /// True when the grid lines are joined by Word's rules rather than Writer's own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The one place a table's original format still shows after it has been read, and it changes the
    /// drawing rather than the layout. Everywhere else a grid line overshoots the line it meets by half a
    /// width, so the two make a solid corner. A Word table instead <em>shortens</em> an interior line by the
    /// <em>full</em> width of the outer line it meets, so the outline owns the corner outright and the
    /// interior line stops at the outline's inner edge.
    /// </para>
    /// <para>
    /// Measured on the corpus table's half-point borders: the DOC and DOCX renders both run their middle
    /// horizontals 56.95 to 538.35 where the ODF one runs them 56.45 to 538.85 — half a point at each end of
    /// five of the nine strokes. Invisible on paper, and the difference between agreeing with a reference
    /// rendering and not.
    /// </para>
    /// <para>
    /// LibreOffice spells it <c>DocumentSettingId::TABLE_ROW_KEEP</c>, which its DOC, DOCX and RTF filters
    /// all set and its ODF filter never does, and reads it back as <c>bWordTableCell</c> in
    /// <c>SwTabFramePainter::FindStylesForLine</c> (<c>sw/source/core/layout/paintfrm.cxx</c>). So it belongs
    /// to the file's provenance rather than to anything the document says, which is why it is a flag on the
    /// table and not a property of a border.
    /// </para>
    /// </remarks>
    public bool JoinsBordersLikeWord { get; init; }

    /// <summary>
    /// True when a row's declared height is a floor on its <em>content</em> rather than on the whole row,
    /// so the cells' margins and the top border are added to it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The second thing a table's provenance still decides, and unlike <see cref="JoinsBordersLikeWord"/>
    /// it changes the layout. LibreOffice calls it <c>DocumentSettingId::MIN_ROW_HEIGHT_INCL_BORDER</c>,
    /// its own comment calls it "MS Word 'atLeast' oddities", and the DOC, DOCX and RTF filters set it
    /// (<c>ww8par.cxx</c>:1966 and <c>DomainMapper.cxx</c>:156) while the ODF filter never does — the same
    /// three-against-one split, and reached the same way, as the flag above.
    /// </para>
    /// <para>
    /// It is not a nicety either way round. Off, the FAA Holdover Tables' 397-twip rows come out 20.85 pt
    /// against the reference's 22.00. On, the <c>table-exact-row</c> fixture's ODF forms come out a
    /// margin too tall — which is how the split was found, since applying it to all five formats fixed
    /// <c>.doc</c>, <c>.docx</c> and <c>.rtf</c> and broke <c>.odt</c> and <c>.fodt</c>.
    /// </para>
    /// </remarks>
    public bool MinHeightIncludesInsets { get; init; }

    /// <summary>
    /// Whether the table begins a page, the way
    /// <see cref="XRay.Office.Text.Layout.ParagraphFormat.StartsNewPage"/> does for a paragraph.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A DOCX has no "page break before this table": it puts the break on the empty paragraph in front of
    /// it, as <c>&lt;w:br w:type="page"/&gt;</c>, and the importer has to carry it forward onto whatever
    /// block comes next. That is easy to miss because the flag is only ever *read* where a paragraph is
    /// read, so a break landing in front of a table was consumed by the first paragraph inside the
    /// table's first cell, where it means nothing at all — the table went on drawing where it was.
    /// </para>
    /// <para>
    /// LibreOffice does carry it, and says so in its own import: <c>ESPN-R - MCF - Manual</c> has two such
    /// paragraphs, one before its "Section 2" form table and one before its "Post-flight briefing" table,
    /// and no <c>w:pageBreakBefore</c> anywhere in the document — yet the flat XML from
    /// <c>--convert-to fodt</c> carries <c>fo:break-before="page"</c> on <c>Table12</c> and
    /// <c>Table13</c>, the only two in the file. The reference leaves 255 pt of page 31 empty and opens
    /// the table on page 32; we filled it, and the document came out a page short.
    /// </para>
    /// </remarks>
    public bool StartsNewPage { get; init; }

    /// <summary>How wide the table is, which is its columns added up.</summary>
    /// <remarks>
    /// The declared columns, so a table whose grid the file left blank answers with what it declared rather
    /// than with what it will be laid out at. Use <see cref="WidthWithin"/> when the answer has to be the
    /// second.
    /// </remarks>
    public Length Width
    {
        get
        {
            Length total = Length.Zero;
            foreach (Length column in ColumnWidths) total += column;
            return total;
        }
    }

    /// <summary>
    /// The column widths the table is laid out at inside an area of a given width.
    /// </summary>
    /// <remarks>
    /// The same list as <see cref="ColumnWidths"/> for a table that declared its grid, so nothing stating
    /// its widths pays for this or can be changed by it. The area's width matters only to a table that
    /// stated neither its own width nor all of its columns', which is Writer's
    /// <c>HoriOrientation::FULL</c> — as wide as whatever it sits in.
    /// </remarks>
    /// <param name="available">The width of the area the table sits in.</param>
    public IReadOnlyList<Length> WidthsWithin(Length available)
    {
        if (RelativeWidth is { } percent)
        {
            Length target = Length.FromTwips(Math.Max(1, available.Twips * percent / 100));
            return ColumnFit is null ? Scaled(ColumnWidths, target) : ColumnFit.Resolve(ColumnWidths, target);
        }

        return ColumnFit is null ? ColumnWidths : ColumnFit.Resolve(ColumnWidths, available);
    }

    /// <summary>How wide the table is inside an area of a given width.</summary>
    /// <param name="available">The width of the area the table sits in.</param>
    public Length WidthWithin(Length available)
    {
        if (ColumnFit is null && RelativeWidth is null) return Width;

        Length total = Length.Zero;
        foreach (Length column in WidthsWithin(available)) total += column;
        return total;
    }

    /// <summary>
    /// The declared grid restated in the same proportions at a given total width.
    /// </summary>
    /// <remarks>
    /// Twips, and the last column takes the remainder, so the parts add back up to the total exactly — the
    /// same arithmetic <see cref="TableColumnFit"/> does, for the same reason.
    /// </remarks>
    private static IReadOnlyList<Length> Scaled(IReadOnlyList<Length> columns, Length target)
    {
        long grid = 0;
        foreach (Length column in columns) grid += Math.Max(0, column.Twips);

        if (grid <= 0 || columns.Count == 0) return columns;

        Length[] scaled = new Length[columns.Count];
        long used = 0;
        for (int i = 0; i < columns.Count - 1; i++)
        {
            long share = Math.Max(0, columns[i].Twips) * target.Twips / grid;
            scaled[i] = Length.FromTwips(share);
            used += share;
        }

        scaled[^1] = Length.FromTwips(Math.Max(0, target.Twips - used));
        return scaled;
    }

    /// <summary>
    /// How a <em>positioned</em> table is aligned across the area it sits in, or null when the table is
    /// placed by <see cref="LeftIndent"/> like every ordinary one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// OOXML's <c>w:tblpPr</c> and RTF's <c>\tposxc</c> and friends. A positioned table names an edge to
    /// align against rather than a distance, so it cannot be reduced to an indent while the table is
    /// being read: the answer depends on the text area's width, which the reader has not got.
    /// </para>
    /// <para>
    /// It matters most where it looks least likely to: a table <em>wider</em> than the text area. Left
    /// where an indent puts it, its right-hand columns fall off the paper and their ink is clipped away
    /// — visible as text that neither draws nor extracts, with no other symptom. Centred, as
    /// <c>w:tblpXSpec="center"</c> asks, the overflow is shared between the two margins and stays on the
    /// page. 21 of the words track's 134 DOCX files carry one of these, and all fifteen that state a
    /// spec state <c>center</c>.
    /// </para>
    /// </remarks>
    public FrameHorizontalAlignment? HorizontalPosition { get; init; }

    /// <summary>
    /// True when the table is <em>positioned</em> — it names a place on the page rather than following the
    /// text, which in Writer makes it a frame holding a table rather than a table in the flow.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Not the same question as <see cref="HorizontalPosition"/>, which is null for a positioned table that
    /// states no <c>w:tblpXSpec</c> — two of the corpus's four running heads holding one are positioned
    /// against the page's own edges by <c>w:tblpX</c>, with no spec at all. So the flag is carried
    /// separately rather than read off the alignment.
    /// </para>
    /// <para>
    /// What it changes is <em>where the flow it sits in has got to</em>: in a running head or foot
    /// through <see cref="FlowLayouter.LayOut"/>, and in the body through <c>Paginator.Fill</c>, which
    /// puts it at <see cref="VerticalOffset"/> from <see cref="VerticalOrigin"/> and leaves the flow
    /// where it was.
    /// </para>
    /// </remarks>
    public bool IsPositioned { get; init; }

    /// <summary>
    /// How far the top of a positioned table sits below <see cref="VerticalOrigin"/> — OOXML's
    /// <c>w:tblpY</c>. Meaningless unless <see cref="IsPositioned"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Read against the installed LibreOffice 26.2.4.2 on the corpus's eight positioned graph-paper
    /// templates, taking the reference's own first horizontal table rule out of its PDF. Predicting that
    /// rule's y from the page geometry and <c>w:tblpY</c> alone lands within <b>1.15 pt on seven of the
    /// eight</b> — the residual being the half border width the stroke is centred on — while our own,
    /// which ignored it, sat at the top margin on all eight and was out by up to 26 pt:
    /// </para>
    /// <code>
    /// doc  anchor  tblpY   predicted   reference   ours
    /// 080  page     1786      752.59      751.84   769.40
    /// 084  page     1025      790.64      790.09   769.40
    /// 089  page     1741      754.84      754.54   769.65
    /// 082  page     1513      766.24      765.44   769.40
    /// 085  page     1606      761.59      760.44   769.27
    /// 087  page     1025      790.64      789.84   769.46
    /// 083  (text)    525      743.40      743.34   769.65
    /// 081  (text)    579      494.35      491.15   520.41
    /// </code>
    /// <para>
    /// 081 is 3.2 pt out and is not explained; it is the one landscape document in the set and it passes
    /// the gate either way.
    /// </para>
    /// </remarks>
    public Length VerticalOffset { get; init; }

    /// <summary>
    /// What <see cref="VerticalOffset"/> is measured from — OOXML's <c>w:vertAnchor</c>.
    /// </summary>
    /// <remarks>
    /// <c>page</c> is the sheet's own top edge, <c>margin</c> the text area's, and <c>text</c> — which is
    /// also what an absent attribute means — the point the flow has reached, which is the anchor
    /// paragraph's top. The same three <c>TablePositionHandler::getTablePosition</c> maps onto
    /// <c>PAGE_FRAME</c>, <c>PAGE_PRINT_AREA</c> and <c>FRAME</c>
    /// (<c>sw/source/writerfilter/dmapper/TablePositionHandler.cxx:133-141</c>).
    /// </remarks>
    public FrameVerticalOrigin VerticalOrigin { get; init; } = FrameVerticalOrigin.Paragraph;

    /// <summary>
    /// The space a positioned table keeps clear below itself — OOXML's <c>w:bottomFromText</c>.
    /// </summary>
    /// <remarks>
    /// A frame's lower spacing rather than a table's space-after, which is why it is not
    /// <see cref="SpaceAfter"/>: Writer writes it as the fly's <c>fo:margin-bottom</c>, it belongs to the
    /// frame rather than to the table inside it, and it is only ever consulted for a table that
    /// <see cref="IsPositioned"/>.
    /// </remarks>
    public Length LowerSpacing { get; init; }

    /// <summary>
    /// Where the table's left edge sits inside an area of a given width, measured from that area's own
    /// left edge.
    /// </summary>
    /// <param name="available">The width of the area the table sits in.</param>
    public Length LeftWithin(Length available) => HorizontalPosition switch
    {
        FrameHorizontalAlignment.Left => Length.Zero,
        FrameHorizontalAlignment.Centre => (available - WidthWithin(available)) / 2,
        FrameHorizontalAlignment.Right => available - WidthWithin(available),

        // Including Offset, Inside and Outside: a stated distance is already an indent, and the two
        // binding-dependent edges need a page parity that nothing here carries.
        _ => LeftIndent,
    };
}

/// <summary>One row of a table.</summary>
/// <remarks>
/// The cells it holds need not cover the grid: a row can be short of cells, and a format that merges
/// cells horizontally writes one wide cell rather than a placeholder for the columns it swallowed. So a
/// cell states which column it starts at rather than being found by its position in this list.
/// </remarks>
public sealed record PageTableRow
{
    /// <summary>The cells, left to right.</summary>
    public required IReadOnlyList<PageTableCell> Cells { get; init; }

    /// <summary>
    /// The row's declared height, which is a floor unless <see cref="HasExactHeight"/> says otherwise.
    /// </summary>
    /// <remarks>
    /// Honoured when the content is shorter and ignored when it is taller, which is what "at least" means in
    /// all four formats and what a row height usually is.
    /// </remarks>
    public Length MinHeight { get; init; }

    /// <summary>
    /// True when <see cref="MinHeight"/> is an <em>exact</em> height rather than a floor.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The one row height that really is a height: the row is that tall whatever its content, so content
    /// taller than the row is clipped rather than growing it. Every format has a spelling for it, and three of
    /// the four say it by a <em>sign</em> rather than by a word — RTF's <c>\trrh</c> and WW8's
    /// <c>sprmTDyaRowHeight</c> are both "at least" when positive and exact when negative, and ODF
    /// distinguishes <c>style:row-height</c> from <c>style:min-row-height</c> by the attribute name. Only DOCX
    /// spells it out, as <c>w:hRule="exact"</c>.
    /// </para>
    /// <para>
    /// A negative height is therefore not an error to reject: it is how two of the four formats say this, and
    /// a reader that clamped it to zero would silently turn every exact row into an automatic one.
    /// </para>
    /// </remarks>
    public bool HasExactHeight { get; init; }

    /// <summary>
    /// The widest top and bottom rule stated by the cells this row <em>drops</em> because they continue a
    /// vertical merge — the cells that are not in <see cref="Cells"/> at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A covered cell is dropped because nothing draws it: the merge above owns its space. But Writer still
    /// builds a cell frame for it, and <c>lcl_GetTopSpace</c> (<c>sw/source/core/layout/tabfrm.cxx</c>:5175-5194)
    /// walks every lower of the row and takes the maximum of each one's own
    /// <c>CalcLineSpace(TOP, true)</c> <b>without testing the row span</b>, although the code around it
    /// tests <c>getRowSpan() &lt; 1</c> in eight other places. So a covered cell's stated rule is charged to
    /// the row's height while contributing nothing to the ink.
    /// </para>
    /// <para>
    /// [bin] Measured on 26.2.4.2 over 34 one-attribute arms in <c>probes/vmergetop-r154/</c>: a
    /// <c>w:trHeight</c> floor is raised by a covered cell's stated top (fixture 7, 2 arms of 18 differ), the
    /// band above a row includes it and <em>no rule is drawn for it</em> (fixture 8), the band at a boundary
    /// includes the covered cell's stated bottom from the row above and that one <em>is</em> drawn, because it
    /// is the merge's own outer edge (fixture 9), and so is the band below the table's last row (fixture 10).
    /// </para>
    /// <para>
    /// Zero when the row drops no cell, which is every row of a table with no vertical merge in it.
    /// </para>
    /// </remarks>
    public Length CoveredTopRule { get; init; }

    /// <inheritdoc cref="CoveredTopRule"/>
    public Length CoveredBottomRule { get; init; }

    /// <summary>True when the row is one of the table's repeating heading rows.</summary>
    public bool IsHeader { get; init; }

    /// <summary>
    /// Whether the row's own content may be broken across a page, which every format allows by default.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>SwFormatRowSplit</c>, read back by <c>SwRowFrame::IsRowSplitAllowed</c>
    /// (<c>sw/source/core/layout/tabfrm.cxx</c>). The default is <em>true</em> in all four formats, and a
    /// document says otherwise by a flag on the row: DOCX's <c>w:cantSplit</c>, RTF's <c>\trkeep</c>,
    /// WW8's <c>sprmTFCantSplit</c>, and ODF's <c>fo:keep-together="always"</c> on the row's style — the
    /// last of which is the negation of the other three, which is why each reader states its own sense
    /// rather than sharing one.
    /// </para>
    /// <para>
    /// Only the row's <em>content</em> is meant: a row that may not split moves to the next page whole,
    /// and the rows before it stay where they are. It is not the table's own
    /// <c>SwFormatLayoutSplit</c>, which is whether the table may break at all.
    /// </para>
    /// </remarks>
    public bool CanSplit { get; init; } = true;
}

/// <summary>
/// One cell: where it sits in the grid, and the flow of paragraphs inside it.
/// </summary>
/// <remarks>
/// Its own paragraph list rather than a range of the body's, because a cell is a separate flow — its text
/// breaks at the cell's width and its lines are positioned from the cell's own top. The paragraphs inside
/// can be anything a body paragraph can be, per-run formatting and tab stops included, since they go
/// through the same layout path.
/// </remarks>
public sealed record PageTableCell
{
    /// <summary>The blocks inside the cell, in order.</summary>
    /// <remarks>
    /// Blocks rather than paragraphs, because a cell can hold a table — which is how every one of the four
    /// formats writes a nested table. A cell's content goes through <see cref="FlowLayouter"/>, the same
    /// path a header takes, so anything a header can hold a cell can hold.
    /// </remarks>
    public required IReadOnlyList<PageBlock> Blocks { get; init; }

    /// <summary>The grid column the cell starts at, counted from zero.</summary>
    public int Column { get; init; }

    /// <summary>How many grid columns it covers; one for an ordinary cell.</summary>
    public int ColumnSpan { get; init; } = 1;

    /// <summary>
    /// How many rows it covers downwards; one for an ordinary cell.
    /// </summary>
    /// <remarks>
    /// Stated only on the cell that <em>starts</em> the merge. The rows below it simply have no cell at
    /// that column, which is how three of the four formats write it — Writer's negative
    /// <c>mnRowSpan</c> follower boxes are an internal device for keeping its node array rectangular and
    /// have no counterpart here, since nothing downstream needs a placeholder for a cell that is not drawn.
    /// </remarks>
    public int RowSpan { get; init; } = 1;

    /// <summary>The gap between the cell's edges and its text.</summary>
    public CellPadding Padding { get; init; }

    /// <summary>Where the text sits when the row is taller than the content.</summary>
    /// <remarks>
    /// For a turned cell this is still the alignment across the <em>line stack</em>, which is then
    /// horizontal rather than vertical — the property keeps its name because it is what every format
    /// spells <c>vAlign</c>, and because it is the same axis in the cell's own frame.
    /// </remarks>
    public VerticalTextAlignment VerticalAlignment { get; init; }

    /// <summary>
    /// Which way the cell's text runs; <see cref="CellTextDirection.LeftToRight"/> for almost every cell.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A rotated row-group label down the side of a table — <c>AIRCRAFT</c>, <c>ENGINES</c> — is what this
    /// is for, and it is the commonest use of it by a distance. Reading it as upright text does not merely
    /// draw the label the wrong way round: the paragraph then breaks at the <em>column's</em> width, which
    /// for a label column is a few points, so every line holds one glyph and the cell becomes as tall as
    /// the label is long. That turned a 7-page form into 9 on <c>A1. EASA Form 2.docx</c>.
    /// </para>
    /// <para>
    /// The layout consequences live in <see cref="TableLayouter"/>; the one worth knowing here is that a
    /// turned cell contributes <em>nothing</em> to its row's height, measured on the installed 26.2.4.2.
    /// A row holding only turned cells collapses to nothing and draws neither text nor borders.
    /// </para>
    /// </remarks>
    public CellTextDirection TextDirection { get; init; }

    /// <summary>True when the cell's text is turned out of the upright direction.</summary>
    public bool IsTurned => TextDirection != CellTextDirection.LeftToRight;

    /// <summary>
    /// The colour behind the cell's text, or null when the cell is not shaded.
    /// </summary>
    /// <remarks>
    /// The whole cell rectangle, padding included, which is measured rather than assumed: LibreOffice fills
    /// 56.7 to 141.75 pt for a cell whose column runs 56.7 to 141.8, so the fill covers the cell and stops half
    /// a border short of the next one. Null rather than white, because "no shading" and "shaded white" are
    /// different — one lets a page background through and the other does not.
    /// </remarks>
    public Colour? Shading { get; init; }

    /// <summary>The cell's four edges.</summary>
    /// <remarks>
    /// Per cell even though LibreOffice draws them consolidated — one stroke per grid line across the whole
    /// table rather than four round each cell. That is a drawing decision and this is what the document says:
    /// two cells sharing an edge can disagree about it, and the consolidation is what resolves that.
    /// </remarks>
    public CellBorders Borders { get; init; }

    /// <summary>One past the last grid column the cell covers.</summary>
    public int ColumnEnd => Column + Math.Max(1, ColumnSpan);
}

/// <summary>
/// A table after placement: where its cells landed on a page.
/// </summary>
/// <remarks>
/// Cells rather than rows, because a cell is what gets drawn and a row is only how its height was decided
/// — and because a cell spanning rows belongs to no single one of them. Each cell carries its own
/// rectangle, so nothing downstream has to add row tops and column lefts back up.
/// </remarks>
public sealed record PlacedTable
{
    /// <summary>The table the cells came from, for a caller that needs what was not placed.</summary>
    public required PageTable Table { get; init; }

    /// <summary>The rectangle the table occupies, in page coordinates.</summary>
    public required DocRect Area { get; init; }

    /// <summary>The cells that landed here, in row-major order.</summary>
    public required IReadOnlyList<PlacedTableCell> Cells { get; init; }

    /// <summary>The first row of the table on this page, counted in the table's own rows.</summary>
    /// <remarks>
    /// Not always zero: a table split across a page break continues on the next page, and its second part
    /// starts at whichever row did not fit. Repeated heading rows are placed again and are <em>not</em>
    /// counted here, since they are not where the continuation resumed.
    /// </remarks>
    public int FirstRow { get; init; }

    /// <summary>One past the last row of the table on this page.</summary>
    public int RowEnd { get; init; }

    /// <summary>Which column of the page it sits in; zero for single-column text.</summary>
    public int Column { get; init; }

    /// <summary>True when nothing was placed.</summary>
    public bool IsEmpty => Cells.Count == 0;
}

/// <summary>One cell after placement.</summary>
/// <remarks>
/// The outer rectangle and the content are both carried because they are different rectangles: a border
/// and a background fill the outer one, and the text sits inside it by the cell's padding. Deriving one
/// from the other downstream would mean knowing the padding downstream.
/// </remarks>
public sealed record PlacedTableCell
{
    /// <summary>The cell as the document stated it.</summary>
    public required PageTableCell Cell { get; init; }

    /// <summary>Its whole rectangle, padding included, in page coordinates.</summary>
    public required DocRect Area { get; init; }

    /// <summary>Its text, laid out inside the padding, or null when the cell is empty.</summary>
    /// <remarks>
    /// In page coordinates for an ordinary cell, and in the cell's <em>own</em> coordinates when
    /// <see cref="ContentTransform"/> is not null — see there.
    /// </remarks>
    public PlacedFlow? Content { get; init; }

    /// <summary>
    /// How to get from <see cref="Content"/>'s coordinates to the page's, or null when the two are the
    /// same — which they are for every cell but a turned one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A turned cell's text is laid out in an upright frame of its own, breaking at what is the cell's
    /// height on the page, and this quarter turn is what puts that frame where it belongs. Carrying the
    /// turn rather than pre-rotating the lines is what keeps every consumer of <see cref="PlacedFlow"/>
    /// working on one kind of flow: the line boxes, the glyph runs and the tab stops inside are all
    /// measured along the text's own direction, which is the only frame they mean anything in.
    /// </para>
    /// <para>
    /// A backend applies it by pushing the transform and drawing the flow exactly as it draws an upright
    /// one, which is also what LibreOffice does — its PDF writes a <c>0 1 -1 0 x y</c> text matrix per
    /// glyph and nothing else about the run changes.
    /// </para>
    /// </remarks>
    public AffineTransform? ContentTransform { get; init; }

    /// <summary>Which row of the table it starts in.</summary>
    public int Row { get; init; }
}

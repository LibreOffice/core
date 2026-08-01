using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// One printed page of a workbook: which sheet it came from, and which cells are on it.
/// </summary>
public sealed class SheetPage : IPage
{
    private readonly SheetPageDrawing _drawing;

    internal SheetPage(int index, int number, SheetLayout sheet, SheetPagePlacement placement)
    {
        Index = index;
        Number = number;
        Sheet = sheet;
        Placement = placement;
        _drawing = new SheetPageDrawing(sheet, placement);
    }

    /// <inheritdoc/>
    public int Index { get; }

    /// <summary>The printed page number, which restarts per sheet when the sheet says so.</summary>
    public int Number { get; }

    /// <summary>The sheet this page came from.</summary>
    public SheetLayout Sheet { get; }

    /// <summary>Which cells the page holds, at what scale, with which repeated bands.</summary>
    public SheetPagePlacement Placement { get; }

    /// <inheritdoc/>
    public DocSize Size => Sheet.Setup.PageSize;

    /// <inheritdoc/>
    /// <remarks>
    /// The sheet's name, which is what Calc puts in the default header and what a reader uses to
    /// tell one page of a workbook from another. The printed page number is
    /// <see cref="Number"/>; a spreadsheet's pages are identified by sheet far more usefully
    /// than by ordinal, since a workbook's page seven means nothing without knowing which sheet
    /// it is the seventh page of.
    /// </remarks>
    public string? Label => Sheet.Name;

    /// <summary>
    /// How many pages the whole printout has, which is what a header's <c>&amp;N</c> stands for.
    /// </summary>
    /// <remarks>
    /// A property of the job rather than of the page, and not knowable until every sheet has
    /// been paginated — which is why it is set afterwards rather than passed to the constructor.
    /// Calc keeps it in <c>ScHeaderFieldData::nTotalPages</c> and fills it in the same way.
    /// </remarks>
    public int PageCount { get; internal set; } = 1;

    /// <inheritdoc/>
    public void Draw(IDrawingSink sink) => _drawing.Draw(sink, HeaderContext());

    /// <summary>
    /// What this page's header and footer fields stand for.
    /// </summary>
    /// <remarks>
    /// <c>&amp;Z</c> gets the file's name rather than its path, because that is all a document
    /// read from a stream ever knows: <c>DocumentSource</c> carries a name hint and not a
    /// location. The name is nearer than nothing, and a caller with a path can say so by
    /// building the context itself.
    /// </remarks>
    private SheetHeaderContext HeaderContext() => new()
    {
        PageNumber = Number,
        PageCount = PageCount,
        SheetName = Sheet.Name,
        FileName = Sheet.FileName,
        FilePath = Sheet.FileName,
    };
}

/// <summary>
/// The pages a workbook printed to.
/// </summary>
/// <remarks>
/// <para>
/// The adapter between the paginator's own answer and the family-neutral
/// <see cref="IPageSequence"/>, in the same shape <c>WordProcessingPages</c> takes for the same
/// reason: a spreadsheet's page carries a block of cells and a scale, and a word-processing
/// page carries line boxes, so there is nothing worth abstracting over the two beyond a count
/// and a size.
/// </para>
/// <para>
/// Which sheets get pages is decided here rather than by the paginator, because the rule is
/// workbook-wide and slightly surprising. Calc prints every visible sheet — but as soon as
/// <em>any</em> sheet in the workbook declares a print range, the sheets that declare none are
/// printed only if their own "print entire sheet" flag is still set
/// (<c>ScPrintFunc::InitParam</c>, <c>sc/source/ui/view/printfun.cxx:1022-1046</c>). A hidden
/// sheet is never printed, which is the one place layout and extraction deliberately disagree:
/// extraction keeps hidden sheets and flags them.
/// </para>
/// </remarks>
public sealed class SpreadsheetPages : IPageSequence
{
    private readonly List<SheetPage> _pages;

    /// <summary>Lays a workbook's sheets out into pages.</summary>
    /// <param name="sheets">The sheets, in workbook order.</param>
    /// <param name="options">Layout options; null for the defaults.</param>
    public SpreadsheetPages(IReadOnlyList<SheetLayout> sheets, LayoutOptions? options = null)
    {
        ArgumentNullException.ThrowIfNull(sheets);

        Sheets = sheets;
        _pages = Build(sheets, options ?? LayoutOptions.Default);

        // The total is only knowable once every sheet has been paginated, and a header holding
        // "&N" needs it, so it is stamped on afterwards rather than threaded through the loop.
        foreach (SheetPage page in _pages) page.PageCount = _pages.Count;
    }

    /// <summary>The sheets the pages were laid out from, printed or not.</summary>
    public IReadOnlyList<SheetLayout> Sheets { get; }

    /// <inheritdoc/>
    public int Count => _pages.Count;

    /// <inheritdoc/>
    public IPage this[int index] => _pages[index];

    /// <summary>The pages, with the cell block and scale a renderer needs.</summary>
    public IReadOnlyList<SheetPage> Pages => _pages;

    /// <summary>True when a sheet contributes pages to a printout of the workbook.</summary>
    /// <param name="sheet">The sheet to test.</param>
    /// <param name="anySheetHasPrintRange">
    /// Whether any sheet in the workbook declares a print range, which is what makes the
    /// per-sheet flag matter at all.
    /// </param>
    public static bool IsPrinted(SheetLayout sheet, bool anySheetHasPrintRange)
    {
        ArgumentNullException.ThrowIfNull(sheet);

        if (sheet.IsHidden) return false;
        if (!anySheetHasPrintRange) return true;

        return sheet.Setup.PrintAreas.Count > 0 || sheet.Setup.PrintsEntireSheet;
    }

    private static List<SheetPage> Build(IReadOnlyList<SheetLayout> sheets, LayoutOptions options)
    {
        bool anyPrintRange = sheets.Any(sheet => sheet.Setup.PrintAreas.Count > 0);
        List<SheetPage> pages = [];

        // The page number continues across sheets unless a sheet asks to restart, which is what
        // Calc's nPageStart carries between tables (printfun.cxx:958).
        int number = 1;

        foreach (SheetLayout sheet in sheets)
        {
            if (!IsPrinted(sheet, anyPrintRange)) continue;

            if (sheet.Setup.FirstPageNumber > 0) number = sheet.Setup.FirstPageNumber;

            foreach (SheetPagePlacement placement in
                     SheetPagination.Paginate(sheet.Setup, sheet.Grid, sheet.PrintedRange))
            {
                if (options.MaxPages > 0 && pages.Count >= options.MaxPages) return pages;

                pages.Add(new SheetPage(pages.Count, number, sheet, placement));
                number++;
            }
        }

        return pages;
    }
}

/// <summary>One run of columns placed together, and where it starts.</summary>
/// <remarks>
/// A page has one or two: the repeated columns, when the sheet declares any, and its own. They
/// are separate because Calc prints them as separate blocks, so each has its own first column —
/// and it is the first column of a block that decides whose spill reaches into it.
/// </remarks>
/// <param name="First">The band's first column, hidden or not.</param>
/// <param name="Left">Where the band starts, scaled.</param>
internal readonly record struct ColumnBand(int First, Length Left);

/// <summary>
/// Places and draws the cells of one page.
/// </summary>
/// <remarks>
/// <para>
/// The geometry is <c>ScPrintFunc::PrintPage</c>'s (<c>printfun.cxx:2070</c>): the printable
/// rectangle is the paper less the margins and the header and footer bands, the row and column
/// headings take a fixed strip off its top and left when they are printed, the repeated bands
/// come next, and the page's own cells fill what is left. Everything but the headings is scaled
/// by the print zoom, which is why the offsets are accumulated in document twips and multiplied
/// once at the end.
/// </para>
/// <para>
/// Where the text goes inside a cell is <see cref="SheetTextLayout"/>'s business, not this
/// class's: alignment, wrapping, shrink-to-fit, indent, rotation, the overflow of a long string
/// into empty neighbours and the <c>###</c> a too-narrow numeric cell shows are all one body of
/// rules ported from Calc's own text output, and none of them needs to know what a page is. What
/// this class supplies is the cell's rectangle, the print zoom and a way to ask whether the
/// neighbours are free.
/// </para>
/// </remarks>
internal sealed class SheetPageDrawing(SheetLayout sheet, SheetPagePlacement placement)
{
    private readonly double _scale = Math.Max(1, placement.ZoomPercentage) / 100.0;
    private readonly SheetPageDecoration _decoration = new(sheet, placement);
    private readonly SheetPageGraphics _graphics =
        new(sheet, Math.Max(1, placement.ZoomPercentage) / 100.0);

    /// <summary>
    /// Draws the page: what is painted behind the cells, their text, and the page's furniture.
    /// </summary>
    /// <remarks>
    /// The order is <c>ScPrintFunc</c>'s (<c>printfun.cxx:1679-1713</c> and <c>:2344-2404</c>):
    /// backgrounds, borders, cell text, the grid, the drawing layer, the headings and the frame
    /// round them. Each step covers part of the one before it, so the order is correctness rather
    /// than taste — a background painted after a border would erase half of it, and a picture
    /// painted before the grid would be crossed by it.
    /// </remarks>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="context">
    /// What a header's fields stand for on this page, or null when nothing needs them.
    /// </param>
    public void Draw(IDrawingSink sink, SheetHeaderContext? context = null)
    {
        ArgumentNullException.ThrowIfNull(sink);

        sink.BeginPage(sheet.Setup.PageSize);
        try
        {
            DocPoint origin = BodyOrigin;
            List<PlacedColumn> columns = Columns(origin.X, out List<ColumnBand> bands);
            List<PlacedRow> rows = Rows(origin.Y);

            _decoration.DrawBackgrounds(columns, rows, sink);
            _decoration.DrawBorders(columns, rows, sink);

            foreach (PlacedRow row in rows)
            {
                foreach (ColumnBand band in bands) DrawLeadIn(band, row, sink);

                foreach (PlacedColumn column in columns)
                {
                    ContentTableCell? cell = sheet.CellAt(row.Row, column.Column);
                    if (cell is null) continue;

                    string text = cell.GetText();
                    if (text.Length == 0) continue;

                    DrawCell(text, cell, column, row, sink);
                }
            }

            _decoration.DrawGrid(columns, rows, sink);

            // After the grid and before the headings, which is Calc's own order: a picture is on
            // the front drawing layer and covers the gridlines under it (printfun.cxx:1695-1703).
            _graphics.Draw(sink, columns, rows);

            _decoration.DrawHeadings(HeadingOrigin, columns, rows, sink);
            _decoration.DrawHeaderAndFooter(context ?? new SheetHeaderContext(), sink);
        }
        finally
        {
            // Always closed, even when a sink throws: a page left open would make the next one
            // nest inside it, turning one bad page into a broken document.
            sink.EndPage();
        }
    }

    /// <summary>
    /// How much of the page the row and column heading strips take, at the print scale.
    /// </summary>
    /// <remarks>
    /// Zero on both axes when the sheet does not print them. Snapped, and snapped before the
    /// zoom, for the same reason a column is (<see cref="SheetDeviceUnits"/>) and with the same
    /// citation — <c>nHeaderWidth = PRINT_HEADER_WIDTH * nScaleX</c> (<c>printfun.cxx:2204</c>)
    /// is computed in unscaled hundredths of a millimetre and the map mode's fraction scales it
    /// afterwards. The strip's own error is small, a twentieth of a hundredth; what matters is
    /// that it leaves the block's origin on a whole device unit, so the snapped column widths
    /// accumulate onto the boundaries LibreOffice puts them on. Measured on
    /// <c>sheet-decor-ods.ods</c>: the first column starts at exactly 3000 hundredths — 85.039 pt
    /// — in both renderings.
    /// </remarks>
    private DocPoint HeadingStrip => sheet.Setup.PrintsHeadings
        ? new DocPoint(
            SheetDeviceUnits.Snap(SheetPageDecoration.HeadingWidth) * _scale,
            SheetDeviceUnits.Snap(SheetPageDecoration.HeadingHeight) * _scale)
        : new DocPoint(Length.Zero, Length.Zero);

    /// <summary>Where the block starts including its heading strips, which the frame encloses.</summary>
    private DocPoint HeadingOrigin
    {
        get
        {
            DocPoint body = BodyOrigin;
            if (!sheet.Setup.PrintsHeadings) return body;

            return new DocPoint(body.X - HeadingStrip.X, body.Y - HeadingStrip.Y);
        }
    }

    /// <summary>
    /// Where the page's cell block starts, centring included.
    /// </summary>
    /// <remarks>
    /// Centring is measured against what this page holds rather than against the sheet, which is
    /// what makes the last page of a sheet sit differently from the ones before it: Calc sums the
    /// widths of the columns on <em>this</em> page and halves the remainder
    /// (<c>ScPrintFunc::PrintPage</c>, <c>printfun.cxx:2150</c>).
    /// </remarks>
    private DocPoint BodyOrigin
    {
        get
        {
            SheetPrintSetup setup = sheet.Setup;
            DocRect area = setup.PrintableArea;

            // Two roundings and one scaling, in Calc's own order. The margin and the heading
            // strip each reach the device separately — nStartX = nLeftSpace * nScaleX and
            // nHeaderWidth = PRINT_HEADER_WIDTH * nScaleX, both truncated into hundredths of a
            // millimetre (printfun.cxx:2204 and :2220) — and only then does the map mode's
            // fraction apply the print zoom, which is why the strip is snapped before it is
            // scaled and the margin is not scaled at all (aPageRect divides it by the zoom
            // first, printfun.cxx:2104, so the device's multiplication gives it back).
            Length x = SheetDeviceUnits.Snap(area.X) + HeadingStrip.X;
            Length y = SheetDeviceUnits.Snap(area.Y) + HeadingStrip.Y;

            if (setup.CentresHorizontally)
            {
                Length spare = area.Width - Extent(Columns(x).Select(c => c.Width));
                if (spare > Length.Zero) x += spare / 2;
            }

            if (setup.CentresVertically)
            {
                Length spare = area.Height - Extent(Rows(y).Select(r => r.Height));
                if (spare > Length.Zero) y += spare / 2;
            }

            return new DocPoint(x, y);
        }
    }

    private static Length Extent(IEnumerable<Length> sizes)
    {
        Length total = Length.Zero;
        foreach (Length size in sizes) total += size;
        return total;
    }

    /// <summary>The columns on the page, repeated band first, each with its position.</summary>
    /// <remarks>
    /// Snapped to the device unit <em>before</em> the print zoom is applied, not after, and the
    /// difference is measurable on any scaled sheet: Calc hands its output device coordinates in
    /// unscaled hundredths of a millimetre and lets the map mode's fraction do the scaling, so a
    /// 72 pt column at 66% comes out at exactly 47.52 pt rather than at the 47.5087 that snapping
    /// the scaled value gives. See <see cref="SheetDeviceUnits"/>.
    /// </remarks>
    private List<PlacedColumn> Columns(Length left) => Columns(left, out _);

    /// <inheritdoc cref="Columns(Length)"/>
    /// <param name="left">Where the block starts.</param>
    /// <param name="bands">
    /// Receives where each band of columns begins, which is what a lead-in needs: Calc prints a
    /// page's repeated columns and its own columns as two separate <c>ScPrintFunc::PrintArea</c>
    /// calls (<c>printfun.cxx:2312</c> and <c>:2330</c>), each with its own first column, and each
    /// therefore with its own left-hand neighbour to look back at.
    /// </param>
    private List<PlacedColumn> Columns(Length left, out List<ColumnBand> bands)
    {
        List<PlacedColumn> placed = [];
        List<ColumnBand> starts = [];
        Length offset = Length.Zero;

        if (placement.RepeatColumns is { IsValid: true } repeat)
            Append(repeat.FirstColumn, repeat.LastColumn);

        Append(placement.Cells.FirstColumn, placement.Cells.LastColumn);
        bands = starts;
        return placed;

        void Append(int first, int last)
        {
            starts.Add(new ColumnBand(first, left + (offset * _scale)));

            for (int column = first; column <= last; column++)
            {
                if (sheet.Grid.Columns.IsHidden(column)) continue;

                Length width = SheetDeviceUnits.Snap(sheet.Grid.Columns.SizeAt(column));
                placed.Add(new PlacedColumn(column, left + (offset * _scale), width * _scale));
                offset += width;
            }
        }
    }

    /// <summary>The rows on the page, repeated band first.</summary>
    private List<PlacedRow> Rows(Length top)
    {
        List<PlacedRow> placed = [];
        Length offset = Length.Zero;

        if (placement.RepeatRows is { IsValid: true } repeat)
            Append(repeat.FirstRow, repeat.LastRow);

        Append(placement.Cells.FirstRow, placement.Cells.LastRow);
        return placed;

        void Append(int first, int last)
        {
            for (int row = first; row <= last; row++)
            {
                if (sheet.Grid.Rows.IsHidden(row)) continue;

                Length height = SheetDeviceUnits.Snap(sheet.Grid.Rows.SizeAt(row));
                placed.Add(new PlacedRow(row, top + (offset * _scale), height * _scale));
                offset += height;
            }
        }
    }

    /// <summary>
    /// The context a cell's text is laid out in: the zoom, the neighbours, the column widths.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The widths come from the sheet's grid rather than from the page's placed columns, because a
    /// string may spill past the last column on the page and Calc measures the spill against the
    /// document (<c>mpDoc-&gt;GetColWidth</c> in <c>GetOutputArea</c>). Using the page's columns
    /// would stop the overflow at the page boundary and draw a shorter string on the last column
    /// of every page.
    /// </para>
    /// <para>
    /// A neighbour is free only when it is both empty <em>and</em> outside every merge.
    /// <c>ScOutputData::IsAvailable</c> (<c>sc/source/ui/view/output2.cxx:1178-1191</c>) asks two
    /// questions and the second is the one a content tree cannot answer on its own: the cells a
    /// merge covers are dropped by every reader, so they look exactly like empty ones. Overflowing
    /// through them draws a string that Calc cuts short — measured on <c>sheet-features.ods</c>,
    /// where the reference shortens "Second row of pair" to "Second row of p" at the edge of the
    /// two-row merge beside it and Paperless drew all of it.
    /// </para>
    /// </remarks>
    private SheetTextContext Context => new(
        _scale,
        (row, column) => SheetTextLayout.IsAvailable(sheet.CellAt(row, column))
                         && !sheet.IsMerged(row, column),
        column => SheetDeviceUnits.Snap(sheet.Grid.Columns.PrintedSizeAt(column)) * _scale);

    /// <summary>
    /// Draws the cell left of a band whose text reaches into it, at the place it really is.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The half of Calc's string output that a page's own columns cannot supply.
    /// <c>ScOutputData::LayoutStrings</c> starts its column loop <em>one before</em> the block's
    /// first column — <c>if (mnX1 &gt; 0) --nLoopStartX; // start before mnX1 for rest of long text
    /// to the left</c> (<c>sc/source/ui/view/output2.cxx:1541-1543</c>) — and that extra iteration
    /// resolves to the nearest cell with text at or left of <c>mnX1</c>
    /// (<c>output2.cxx:1638-1656</c>). Without it a sheet whose only content is one column of long
    /// strings draws <em>nothing at all</em> on its second horizontal page, because no cell on that
    /// page holds anything: the text there is entirely another column's spill.
    /// </para>
    /// <para>
    /// The cell is placed at its true position, which is off the left of the block, and the text
    /// then overflows rightwards under the ordinary rules — <see cref="Context"/> already measures
    /// that against the document grid rather than against the page, which is what makes the two
    /// halves of one string line up across the break.
    /// </para>
    /// <para>
    /// It cannot draw a cell twice on one page. The walk starts at the band's own first column, so
    /// a band whose first column holds text yields that column and the <c>&lt; first</c> test
    /// rejects it; only a column strictly left of the band — and therefore not among the band's
    /// placed columns — is ever drawn this way. A cell in two <em>bands</em> of the same page, or
    /// on two pages, is Calc's behaviour and not a fault: each page draws the part of the string
    /// that falls on it.
    /// </para>
    /// </remarks>
    private void DrawLeadIn(ColumnBand band, PlacedRow row, IDrawingSink sink)
    {
        if (band.First <= 0) return;

        // Calc walks back from mnX1 itself, so a band whose own first column holds text stops
        // there and no lead-in is drawn (output2.cxx:1644-1646).
        int at = band.First;
        while (at > 0 && SheetTextLayout.IsAvailable(sheet.CellAt(row.Row, at))) at--;
        if (at >= band.First) return;

        ContentTableCell? cell = sheet.CellAt(row.Row, at);
        if (cell is null || SheetTextLayout.IsAvailable(cell)) return;
        if (sheet.Grid.Columns.IsHidden(at)) return;

        string text = cell.GetText();
        if (text.Length == 0) return;

        // A merge anywhere between the two suppresses the lead-in: Calc asks
        // HasAttrib(Merged | Overlapped) over exactly that span (output2.cxx:1652), because a
        // merged block's own origin is what draws its text and it may be neither of these cells.
        for (int between = at; between <= band.First; between++)
        {
            if (sheet.CellAt(row.Row, between) is { } spanning
                && (spanning.ColumnSpan > 1 || spanning.RowSpan > 1))
            {
                return;
            }
        }

        Length back = Length.Zero;
        for (int column = at; column < band.First; column++)
            back += SheetDeviceUnits.Snap(sheet.Grid.Columns.PrintedSizeAt(column));

        DrawCell(
            text,
            cell,
            new PlacedColumn(
                at,
                band.Left - (back * _scale),
                SheetDeviceUnits.Snap(sheet.Grid.Columns.PrintedSizeAt(at)) * _scale),
            row,
            sink);
    }

    private void DrawCell(
        string text, ContentTableCell cell, PlacedColumn column, PlacedRow row, IDrawingSink sink)
    {
        SheetTextLayout.Draw(sink, Context, new SheetCellText(
            text,
            cell.Value,
            sheet.Formats.At(row.Row, column.Column),
            row.Row,
            column.Column,
            new DocRect(column.X, row.Y, SpanWidth(cell, column), SpanHeight(cell, row)),
            sheet.RichText.At(row.Row, column.Column, text)));
    }

    /// <summary>How wide a cell is, a merge's further columns included.</summary>
    private Length SpanWidth(ContentTableCell cell, PlacedColumn column)
    {
        Length width = column.Width;
        for (int at = 1; at < Math.Max(1, cell.ColumnSpan); at++)
        {
            width += SheetDeviceUnits.Snap(
                sheet.Grid.Columns.PrintedSizeAt(column.Column + at)) * _scale;
        }
        return width;
    }

    /// <summary>How tall a cell is, a merge's further rows included.</summary>
    private Length SpanHeight(ContentTableCell cell, PlacedRow row)
    {
        Length height = row.Height;
        for (int at = 1; at < Math.Max(1, cell.RowSpan); at++)
        {
            height += SheetDeviceUnits.Snap(
                sheet.Grid.Rows.PrintedSizeAt(row.Row + at)) * _scale;
        }
        return height;
    }
}

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

    /// <inheritdoc/>
    public void Draw(IDrawingSink sink) => _drawing.Draw(sink);
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
/// What is drawn is deliberately less than what is placed. Each cell's text is laid down as one
/// glyph run at a baseline, aligned left for text and right for numbers, which is Calc's default
/// alignment for a cell that states none. Explicit alignment, wrapping, shrink-to-fit, rotation,
/// indent, the overflow of a long string into empty neighbours and the <c>###</c> a too-narrow
/// numeric cell shows are all cell <em>text</em> layout, which is a separate item on the module's
/// TODO — none of them moves a page boundary, because a row's height and a column's width are
/// stated in the file rather than derived from the text.
/// </para>
/// </remarks>
internal sealed class SheetPageDrawing(SheetLayout sheet, SheetPagePlacement placement)
{
    /// <summary>One centimetre, the width of the printed row headings.</summary>
    private static readonly Length HeadingWidth = Length.FromTwips(567);

    /// <summary>12.8 points, the height of the printed column headings.</summary>
    private static readonly Length HeadingHeight = Length.FromTwips(256);

    /// <summary>
    /// How far below a row's top the text sits, as a fraction of the row's height.
    /// </summary>
    /// <remarks>
    /// A stand-in for the real answer, which is the row's ascent in the cell's own font. Calc
    /// bottom-aligns cell text by default and leaves a small margin below it
    /// (<c>ATTR_MARGIN</c>, 20 twips), so the baseline sits close to the bottom of the row; four
    /// fifths of the height puts it there for the 10-point text in a 12.8-point row that every
    /// default sheet uses. Replacing it needs the font metrics that cell text layout will bring.
    /// </remarks>
    private const double BaselineFraction = 0.8;

    /// <summary>The margin between a cell's edge and its text.</summary>
    /// <remarks>
    /// <c>ATTR_MARGIN</c>'s default, 20 twips, which is measurable in any Calc rendering: the
    /// first column of a sheet with a two-centimetre left margin starts its text at 57.7 points
    /// rather than at 56.7.
    /// </remarks>
    private static readonly Length CellMargin = Length.FromTwips(20);

    private readonly double _scale = Math.Max(1, placement.ZoomPercentage) / 100.0;

    /// <summary>Draws the page: its grid if it prints one, then every cell's text.</summary>
    public void Draw(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);

        sink.BeginPage(sheet.Setup.PageSize);
        try
        {
            DocPoint origin = BodyOrigin;
            List<PlacedColumn> columns = Columns(origin.X);
            List<PlacedRow> rows = Rows(origin.Y);

            if (sheet.Setup.PrintsGrid) DrawGrid(columns, rows, sink);

            foreach (PlacedRow row in rows)
            {
                foreach (PlacedColumn column in columns)
                {
                    ContentTableCell? cell = sheet.CellAt(row.Row, column.Column);
                    if (cell is null) continue;

                    string text = cell.GetText();
                    if (text.Length == 0) continue;

                    DrawCell(text, cell, column, row, sink);
                }
            }
        }
        finally
        {
            // Always closed, even when a sink throws: a page left open would make the next one
            // nest inside it, turning one bad page into a broken document.
            sink.EndPage();
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

            Length x = area.X + (setup.PrintsHeadings ? HeadingWidth : Length.Zero);
            Length y = area.Y + (setup.PrintsHeadings ? HeadingHeight : Length.Zero);

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
    private List<PlacedColumn> Columns(Length left)
    {
        List<PlacedColumn> placed = [];
        Length x = left;

        if (placement.RepeatColumns is { IsValid: true } repeat)
            Append(repeat.FirstColumn, repeat.LastColumn);

        Append(placement.Cells.FirstColumn, placement.Cells.LastColumn);
        return placed;

        void Append(int first, int last)
        {
            for (int column = first; column <= last; column++)
            {
                if (sheet.Grid.Columns.IsHidden(column)) continue;

                Length width = sheet.Grid.Columns.SizeAt(column) * _scale;
                placed.Add(new PlacedColumn(column, x, width));
                x += width;
            }
        }
    }

    /// <summary>The rows on the page, repeated band first.</summary>
    private List<PlacedRow> Rows(Length top)
    {
        List<PlacedRow> placed = [];
        Length y = top;

        if (placement.RepeatRows is { IsValid: true } repeat)
            Append(repeat.FirstRow, repeat.LastRow);

        Append(placement.Cells.FirstRow, placement.Cells.LastRow);
        return placed;

        void Append(int first, int last)
        {
            for (int row = first; row <= last; row++)
            {
                if (sheet.Grid.Rows.IsHidden(row)) continue;

                Length height = sheet.Grid.Rows.SizeAt(row) * _scale;
                placed.Add(new PlacedRow(row, y, height));
                y += height;
            }
        }
    }

    private static void DrawGrid(List<PlacedColumn> columns, List<PlacedRow> rows, IDrawingSink sink)
    {
        if (columns.Count == 0 || rows.Count == 0) return;

        Length right = columns[^1].X + columns[^1].Width;
        Length bottom = rows[^1].Y + rows[^1].Height;
        Stroke stroke = new(Paint.Solid(Colour.FromRgb(0xB0B0B0)), Length.FromTwips(1));

        Length top = rows[0].Y;
        foreach (PlacedColumn column in columns) Line(column.X, top, column.X, bottom);
        Line(right, top, right, bottom);

        foreach (PlacedRow row in rows) Line(columns[0].X, row.Y, right, row.Y);
        Line(columns[0].X, bottom, right, bottom);

        void Line(Length x1, Length y1, Length x2, Length y2)
            => sink.StrokePath(
                new GraphicsPath().MoveTo(new DocPoint(x1, y1)).LineTo(new DocPoint(x2, y2)),
                stroke);
    }

    private void DrawCell(
        string text, ContentTableCell cell, PlacedColumn column, PlacedRow row, IDrawingSink sink)
    {
        Length size = Length.FromPoints(10) * _scale;
        SheetTextRun? run = SheetText.Shape(text, size);
        if (run is null) return;

        // Calc's default alignment, which is the cell's type and not a property: a string sits
        // against the left edge and a number against the right, so a column of figures lines up.
        bool rightAligned = cell.Value is not null and not string and not bool;
        Length margin = CellMargin * _scale;
        Length x = rightAligned
            ? column.X + column.Width - margin - run.Width
            : column.X + margin;

        Length baseline = row.Y + (row.Height * BaselineFraction);
        sink.DrawGlyphRun(run.At(new DocPoint(x, baseline)), Paint.Solid(Colour.Black));
    }

    private readonly record struct PlacedColumn(int Column, Length X, Length Width);

    private readonly record struct PlacedRow(int Row, Length Y, Length Height);
}

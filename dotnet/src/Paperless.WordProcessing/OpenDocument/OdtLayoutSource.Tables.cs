using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.OpenDocument;

/// <content>
/// Reading a <c>table:table</c> into the grid the layout engine takes.
/// </content>
/// <remarks>
/// <para>
/// ODF describes a table by declaring its columns and then filling rows against them, which is very nearly
/// what layout wants — the grid is stated outright rather than implied by the widest row, so nothing has to
/// be inferred. Three details do the work:
/// </para>
/// <list type="bullet">
///   <item>
///     <c>table:number-columns-repeated</c> on a column, and <c>table:number-rows-repeated</c> on a row.
///     A table of twelve identical columns declares one and repeats it, so a reader that took each element
///     as one column would build a grid a twelfth of the right width.
///   </item>
///   <item>
///     <c>table:covered-table-cell</c>, which is a placeholder for a column swallowed by a merge to its
///     left or above. It holds no content and must not be laid out, but it <em>does</em> occupy its column
///     — so skipping it entirely would shift every cell after it one column left.
///   </item>
///   <item>
///     <c>table:table-header-rows</c>, whose rows are the ones that repeat when the table crosses a page.
///     It is a wrapper element rather than a flag, so the rows inside it are still ordinary rows and their
///     position in the table is what makes them headings.
///   </item>
/// </list>
/// </remarks>
public sealed partial class OdtLayoutSource
{
    /// <summary>
    /// Writer's own default cell padding, for a table whose cell style states none.
    /// </summary>
    /// <remarks>
    /// 0.097 cm, which is 55 twips — an odd number that comes from the 1/100 mm grid the draw layer uses
    /// rather than from anything a person chose. Both edges of it matter: it comes out of the cell's width,
    /// so a reader defaulting it to zero breaks a narrow cell's text a line later than Writer does.
    /// </remarks>
    private static readonly CellPadding DefaultCellPadding = CellPadding.Writer;

    /// <summary>Reads a table, or returns null when it declares no usable grid.</summary>
    private PageTable? Table(XElement element)
    {
        string? styleName = element.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        List<Length> columns = Columns(element, styleName);
        if (columns.Count == 0) return null;

        List<PageTableRow> rows = [];
        int headerRows = 0;
        ReadRows(element, rows, ref headerRows, isHeader: false, depth: 0);

        if (rows.Count == 0) return null;

        return new PageTable
        {
            SectionIndex = _sectionIndex,
            ColumnWidths = columns,
            Rows = rows,
            HeaderRowCount = headerRows,
            LeftIndent = TableMeasure(styleName, OdfNamespaces.FoCompatible, "margin-left")
                ?? Length.Zero,
            SpaceBefore = TableMeasure(styleName, OdfNamespaces.FoCompatible, "margin-top")
                ?? Length.Zero,
            SpaceAfter = TableMeasure(styleName, OdfNamespaces.FoCompatible, "margin-bottom")
                ?? Length.Zero,
        };
    }

    /// <summary>
    /// The grid's column widths, in order, with the ones the document did not state resolved.
    /// </summary>
    /// <remarks>
    /// A column need not state a width, and this is not the exotic case it sounds like — a table written by
    /// anything other than an office suite usually states none at all, and one written by hand often states
    /// some. So the widths are collected as <em>drafts</em>, each either an absolute measure or a relative
    /// weight, and resolved together against the table's own width. Taking a missing width as zero, which is
    /// what this used to do, gives a column nothing to break its text in.
    /// </remarks>
    private List<Length> Columns(XElement table, string? tableStyleName)
    {
        List<ColumnDraft> drafts = [];
        Collect(table, 0);

        return Resolved(drafts, TableWidth(tableStyleName));

        void Collect(XElement element, int depth)
        {
            if (depth > 8 || drafts.Count >= PageTable.MaxColumns) return;

            foreach (XElement child in element.Elements())
            {
                if (child.Name.Namespace != OdfNamespaces.Table) continue;

                switch (child.Name.LocalName)
                {
                    // The grouping elements are transparent: a table can wrap its columns in
                    // table:table-columns, or group them for outlining, and the columns inside are the
                    // table's own either way.
                    case "table-columns" or "table-header-columns" or "table-column-group":
                        Collect(child, depth + 1);
                        break;

                    case "table-column":
                        ColumnDraft draft = ColumnDraftOf(child);
                        int repeat = Repeat(child, "number-columns-repeated");

                        for (int i = 0; i < repeat && drafts.Count < PageTable.MaxColumns; i++)
                        {
                            drafts.Add(draft);
                        }

                        break;
                }
            }
        }
    }

    /// <summary>
    /// One column's declared width, before the table's own width settles what it means.
    /// </summary>
    /// <param name="Twips">The absolute measure, or the relative weight when <paramref name="IsRelative"/>.</param>
    /// <param name="IsRelative">
    /// True when the number is a share of the table rather than a measure of the column.
    /// </param>
    private readonly record struct ColumnDraft(long Twips, bool IsRelative);

    /// <summary>
    /// The smallest width Writer gives a column: <c>MINLAY</c>, twenty-three twips.
    /// </summary>
    /// <remarks>
    /// It matters here for two reasons rather than one. It is the floor every column is clamped to, and it is
    /// also the <em>weight</em> a column with no stated width gets — so a table of three width-less columns
    /// arrives as three relative columns of 23, and how those become measures is entirely up to the table.
    /// </remarks>
    private const long MinimumColumnTwips = 23;

    /// <summary>One column's draft width, from whichever of the two spellings its style used.</summary>
    /// <remarks>
    /// <c>style:column-width</c> is a measure and <c>style:rel-column-width</c> a weight written as
    /// <c>23*</c>. A column with neither is a relative one of the minimum weight, which is what LibreOffice's
    /// <c>SwXMLTableColContext_Impl</c> starts from — <c>nWidth = MINLAY; bRelWidth = true</c> — and is the
    /// reason a table whose columns say nothing still divides itself evenly rather than collapsing.
    /// </remarks>
    private ColumnDraft ColumnDraftOf(XElement column)
    {
        string? styleName = column.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        if (OdfWriterUnits.ToCore(
                OdfValue.ParseLength(
                    _styles.ResolveProperty(
                        styleName, OdfStyleFamily.TableColumn, OdfPropertyKind.TableColumn,
                        OdfNamespaces.Style, "column-width").Value))
            is { } stated && stated > Length.Zero)
        {
            return new ColumnDraft(Math.Max(stated.Twips, MinimumColumnTwips), IsRelative: false);
        }

        string? relative = _styles.ResolveProperty(
            styleName, OdfStyleFamily.TableColumn, OdfPropertyKind.TableColumn,
            OdfNamespaces.Style, "rel-column-width").Value;

        return new ColumnDraft(
            Math.Max(RelativeWeight(relative), MinimumColumnTwips), IsRelative: true);
    }

    /// <summary>A <c>23*</c> relative width as its number, or the minimum when it is not one.</summary>
    private static long RelativeWeight(string? value)
        => long.TryParse(
            value?.TrimEnd('*'),
            System.Globalization.NumberStyles.Integer,
            System.Globalization.CultureInfo.InvariantCulture,
            out long weight)
            ? weight
            : MinimumColumnTwips;

    /// <summary>
    /// The table's own width, and whether it is a measure or a share of the text area.
    /// </summary>
    /// <remarks>
    /// Three cases, and the difference between them decides how the columns resolve. <c>style:width</c> is a
    /// measure. <c>style:rel-width</c> is a percentage of the text area. Neither means the table fills the
    /// text area, which is what <c>table:align="margins"</c> asks for and what Writer does regardless of the
    /// alignment when no width is stated.
    /// </remarks>
    private (long Twips, bool IsRelative) TableWidth(string? styleName)
    {
        if (TableMeasure(styleName, OdfNamespaces.Style, "width") is { } stated
            && stated > Length.Zero)
        {
            return (stated.Twips, false);
        }

        string? relative = _styles.ResolveProperty(
            styleName, OdfStyleFamily.Table, OdfPropertyKind.Table,
            OdfNamespaces.Style, "rel-width").Value;

        double share = OdfValue.ParsePercentage(relative) ?? 1.0;

        return ((long)Math.Round(_availableWidth.Twips * Math.Clamp(share, 0.0, 1.0)), true);
    }

    /// <summary>
    /// Turns the column drafts into measures, the way LibreOffice's own ODF import turns them into measures.
    /// </summary>
    /// <param name="drafts">The columns as declared.</param>
    /// <param name="table">The table's width, and whether that width is a measure or a share.</param>
    /// <remarks>
    /// <para>
    /// Two branches, because the answer genuinely differs and it is measured: the same three width-less
    /// columns come out <strong>equal</strong> in a table that states no width and <strong>160.6, 107.1 and
    /// 214.1 pt</strong> in a 17 cm one. The second set is not a mistake in the measurement and not
    /// content-based sizing — it is what
    /// <c>SwXMLTableContext::MakeTable</c> (<c>sw/source/filter/xml/xmltbli.cxx</c>) computes, and the ratio
    /// 3 : 2 : 4 comes out of a quirk worth naming: the loop that converts a relative column to an absolute
    /// one shrinks the space remaining after each column but keeps dividing by the <em>full</em> sum of the
    /// weights, so each column gets a third of what is left rather than a third of the whole.
    /// </para>
    /// <para>
    /// A table that states no measure keeps its columns relative through the import and lets layout divide the
    /// text area between them, which is why that case is a plain proportional division and does not inherit
    /// the quirk. Both are reproduced rather than picked between, because a document arrives written either
    /// way and LibreOffice's renders of the two differ by fifty points of column width.
    /// </para>
    /// </remarks>
    private static List<Length> Resolved(List<ColumnDraft> drafts, (long Twips, bool IsRelative) table)
    {
        if (drafts.Count == 0) return [];

        List<long> twips = table.IsRelative
            ? Shares(drafts, table.Twips)
            : Measures(drafts, table.Twips);

        return [.. twips.Select(value => Length.FromTwips(Math.Max(value, 0)))];
    }

    /// <summary>
    /// The columns of a table whose width is a share of the text area: a proportional division.
    /// </summary>
    /// <remarks>
    /// By accumulating the boundaries and truncating each rather than rounding each width, so that the
    /// widths add up to the table's exactly and the remainder lands where Writer's own division puts it —
    /// 9638 twips over three equal columns comes out 3212, 3213, 3213 rather than 3213, 3213, 3212. An
    /// absolute column keeps its measure as its weight, which is how a table mixing the two spellings stays
    /// in proportion.
    /// </remarks>
    private static List<long> Shares(List<ColumnDraft> drafts, long width)
    {
        long total = drafts.Sum(draft => draft.Twips);
        if (total <= 0) return [.. drafts.Select(_ => MinimumColumnTwips)];

        List<long> widths = new(drafts.Count);
        long cumulative = 0;
        long previous = 0;

        foreach (ColumnDraft draft in drafts)
        {
            cumulative += draft.Twips;
            long boundary = width * cumulative / total;
            widths.Add(boundary - previous);
            previous = boundary;
        }

        return widths;
    }

    /// <summary>
    /// The columns of a table whose width is a measure, following LibreOffice's own two steps.
    /// </summary>
    /// <remarks>
    /// First every relative column becomes absolute — with the shrinking-dividend quirk noted above, which
    /// is reproduced because it is what the reference renders — and then the whole set is scaled to the
    /// table's width if the two do not already agree. The scaling has two arms of its own: a set narrower
    /// than the table shares out the surplus in proportion, and a set wider than it is rebuilt from the
    /// minimum width upwards, which is the only way a table can shrink without a column vanishing.
    /// </remarks>
    private static List<long> Measures(List<ColumnDraft> drafts, long width)
    {
        List<long> widths = [.. drafts.Select(draft => draft.Twips)];

        long stated = drafts.Where(draft => !draft.IsRelative).Sum(draft => draft.Twips);
        long weights = drafts.Where(draft => draft.IsRelative).Sum(draft => draft.Twips);
        int relativeCount = drafts.Count(draft => draft.IsRelative);
        long smallestWeight = relativeCount == 0
            ? 0
            : drafts.Where(draft => draft.IsRelative).Min(draft => draft.Twips);

        if (relativeCount > 0)
        {
            long forRelative = Math.Max(0, width - stated);
            long surplusWeight = weights - (relativeCount * smallestWeight);
            long minimum = relativeCount * MinimumColumnTwips;
            long surplus = Math.Max(0, forRelative - minimum);

            // All at the minimum when there is not room for even that; the minimum plus a share of what is
            // left when there is room for the minimum but not for the weights; the weights otherwise.
            bool atMinimum = forRelative <= minimum;
            bool atMinimumPlusShare =
                !atMinimum
                && surplusWeight > 0
                && smallestWeight > 0
                && forRelative <= weights * MinimumColumnTwips / smallestWeight;

            int remaining = relativeCount;

            for (int i = 0; i < drafts.Count; i++)
            {
                if (!drafts[i].IsRelative) continue;

                long resolved;
                if (remaining == 1)
                {
                    // The last relative column takes whatever is left, which is what keeps the set adding up.
                    resolved = forRelative;
                }
                else if (atMinimum)
                {
                    resolved = MinimumColumnTwips;
                }
                else if (atMinimumPlusShare)
                {
                    resolved = MinimumColumnTwips
                               + ((drafts[i].Twips - smallestWeight) * surplus / surplusWeight);
                }
                else
                {
                    resolved = drafts[i].Twips * forRelative / weights;
                }

                widths[i] = resolved;
                forRelative -= resolved;
                stated += resolved;
                remaining--;
            }
        }

        return stated == width || stated <= 0 ? widths : Scaled(widths, stated, width);
    }

    /// <summary>Scales a set of absolute widths onto the table's width.</summary>
    private static List<long> Scaled(List<long> widths, long total, long width)
    {
        bool growing = total < width;
        long extra = growing ? width - total : width - (widths.Count * MinimumColumnTwips);
        long last = growing ? widths[^1] + extra : MinimumColumnTwips + extra;

        for (int i = 0; i < widths.Count - 1; i++)
        {
            long share = widths[i] * extra / total;
            widths[i] = growing ? widths[i] + share : MinimumColumnTwips + share;
            last -= share;
        }

        widths[^1] = last;
        return widths;
    }

    /// <summary>
    /// Reads a table's rows, following the grouping elements and noting which rows are headings.
    /// </summary>
    /// <remarks>
    /// The heading count is a run from the top, matching <c>SwTable::GetRowsToRepeat</c>, so it is only
    /// advanced while every row so far has been a heading. A <c>table:table-header-rows</c> appearing after
    /// ordinary rows — which is legal and meaningless — therefore repeats nothing, rather than repeating
    /// rows from the middle of the table.
    /// </remarks>
    private void ReadRows(
        XElement element, List<PageTableRow> rows, ref int headerRows, bool isHeader, int depth)
    {
        if (depth > 8) return;

        foreach (XElement child in element.Elements())
        {
            if (child.Name.Namespace != OdfNamespaces.Table) continue;
            if (rows.Count >= PageTable.MaxRows) return;

            switch (child.Name.LocalName)
            {
                case "table-header-rows":
                    ReadRows(child, rows, ref headerRows, isHeader: true, depth + 1);
                    break;

                case "table-rows" or "table-row-group":
                    ReadRows(child, rows, ref headerRows, isHeader, depth + 1);
                    break;

                case "table-row":
                    PageTableRow row = Row(child, isHeader);
                    int repeat = Repeat(child, "number-rows-repeated");

                    for (int i = 0; i < repeat && rows.Count < PageTable.MaxRows; i++)
                    {
                        rows.Add(row);
                        if (isHeader && headerRows == rows.Count - 1) headerRows = rows.Count;
                    }

                    break;
            }
        }
    }

    private PageTableRow Row(XElement element, bool isHeader)
    {
        List<PageTableCell> cells = [];
        int column = 0;

        foreach (XElement child in element.Elements())
        {
            if (child.Name.Namespace != OdfNamespaces.Table) continue;

            // A covered cell is a column the merge to its left or above swallowed. It carries no content
            // and gets no cell, but it advances the column counter — which is the whole reason ODF writes
            // it at all, since a row's cells are positional.
            if (child.Name.LocalName == "covered-table-cell")
            {
                column += Repeat(child, "number-columns-repeated");
                continue;
            }

            if (child.Name.LocalName != "table-cell") continue;

            int span = Math.Max(1, Attribute(child, "number-columns-spanned"));
            int rowSpan = Math.Max(1, Attribute(child, "number-rows-spanned"));
            int repeat = Repeat(child, "number-columns-repeated");

            string? styleName = child.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;
            List<PageBlock> blocks = ReadCell(child);

            for (int i = 0; i < repeat && column < PageTable.MaxColumns; i++)
            {
                cells.Add(new PageTableCell
                {
                    Blocks = blocks,
                    Column = column,
                    ColumnSpan = span,
                    RowSpan = rowSpan,
                    Padding = Padding(styleName),
                    VerticalAlignment = VerticalAlignment(styleName),
                    Shading = Shading(styleName),
                    Borders = Borders(styleName),
                });

                // One column per element, not one per column spanned. A cell covering two columns is
                // followed by a table:covered-table-cell for the second, and it is that placeholder which
                // accounts for the column — so advancing by the span here as well would count it twice and
                // push every cell after it off the end of the grid. ODF requires the placeholders and
                // LibreOffice always writes them, which is what makes this the safe direction to err in:
                // a producer that omits one leaves a gap rather than an overlap.
                column++;
            }
        }

        return new PageTableRow
        {
            Cells = cells,
            IsHeader = isHeader,
            MinHeight = RowHeight(element).Height,
            HasExactHeight = RowHeight(element).IsExact,
        };
    }

    /// <summary>
    /// A row's declared height, which is a floor rather than a size.
    /// </summary>
    /// <remarks>
    /// <c>style:min-row-height</c> says so outright. <c>style:row-height</c> reads as exact but is not:
    /// LibreOffice honours it only while the content fits, and grows the row otherwise — so both map to the
    /// same floor here, which is what a row whose text has been edited since it was written actually does.
    /// </remarks>
    /// <summary>
    /// The row's declared height and whether it is exact.
    /// </summary>
    /// <remarks>
    /// ODF distinguishes the two by attribute name rather than by a rule: <c>style:min-row-height</c> is a
    /// floor and <c>style:row-height</c> is a height, and a row stating both means the floor — a minimum the
    /// content can exceed is a weaker claim than an exact size, so honouring the exact one would clip content
    /// the document said could grow.
    /// </remarks>
    /// <param name="row">The <c>table:table-row</c> element.</param>
    private (Length Height, bool IsExact) RowHeight(XElement row)
    {
        string? styleName = row.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        if (RowMeasure(styleName, "min-row-height") is { } floor) return (floor, false);

        return RowMeasure(styleName, "row-height") is { } exact
            ? (exact, true)
            : (Length.Zero, false);
    }

    private Length? RowMeasure(string? styleName, string propertyName)
        => OdfWriterUnits.ToCore(
            OdfValue.ParseLength(
                _styles.ResolveProperty(
                    styleName, OdfStyleFamily.TableRow, OdfPropertyKind.TableRow,
                    OdfNamespaces.Style, propertyName).Value));

    /// <summary>
    /// A cell's padding, from the one-value form or the four separate ones.
    /// </summary>
    /// <remarks>
    /// <c>fo:padding</c> sets all four and each <c>fo:padding-left</c> and friends overrides its own side,
    /// which is CSS's rule and ODF's. The per-side value wins wherever it is present, so a style stating
    /// both is read the way a browser would read it.
    /// </remarks>
    private CellPadding Padding(string? styleName)
    {
        Length? all = CellMeasure(styleName, "padding");

        return new CellPadding(
            CellMeasure(styleName, "padding-left") ?? all ?? DefaultCellPadding.Left,
            CellMeasure(styleName, "padding-right") ?? all ?? DefaultCellPadding.Right,
            CellMeasure(styleName, "padding-top") ?? all ?? DefaultCellPadding.Top,
            CellMeasure(styleName, "padding-bottom") ?? all ?? DefaultCellPadding.Bottom);
    }

    private Length? CellMeasure(string? styleName, string propertyName)
        => OdfWriterUnits.ToCore(
            OdfValue.ParseLength(
                _styles.ResolveProperty(
                    styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell,
                    OdfNamespaces.FoCompatible, propertyName).Value));

    /// <summary>Where a cell's text sits when its row is taller than its content.</summary>
    /// <remarks>
    /// <c>automatic</c> is a real value and means the top, which is also what an unstated alignment means —
    /// so both fall through to the same answer rather than one of them being treated as unknown.
    /// </remarks>
    private CellVerticalAlignment VerticalAlignment(string? styleName)
        => _styles.ResolveProperty(
            styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell,
            OdfNamespaces.Style, "vertical-align").Value switch
        {
            "middle" => CellVerticalAlignment.Middle,
            "bottom" => CellVerticalAlignment.Bottom,
            _ => CellVerticalAlignment.Top,
        };

    /// <summary>
    /// The colour behind a cell's text, or null when it has none.
    /// </summary>
    /// <remarks>
    /// <c>fo:background-color</c>, whose <c>transparent</c> is a real value meaning "no shading" rather than a
    /// colour — so it has to fall through to null rather than being parsed. ODF has no separate pattern or
    /// foreground colour for a cell the way RTF and WW8 do; the resolved colour is the whole answer.
    /// </remarks>
    private Colour? Shading(string? styleName)
    {
        OdfProperty stated = _styles.ResolveProperty(
            styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell,
            OdfNamespaces.FoCompatible, "background-color");

        return stated.Value == "transparent" ? null : stated.AsColour();
    }

    /// <summary>
    /// A cell's four borders.
    /// </summary>
    /// <remarks>
    /// <c>fo:border</c> sets all four and each <c>fo:border-left</c> and friends overrides its own side, which
    /// is CSS's rule and ODF's — the same cascade <see cref="Padding"/> follows for the same reason.
    /// </remarks>
    private CellBorders Borders(string? styleName)
    {
        TableBorder all = Border(styleName, "border");

        return new CellBorders(
            Border(styleName, "border-left", all),
            Border(styleName, "border-right", all),
            Border(styleName, "border-top", all),
            Border(styleName, "border-bottom", all));
    }

    /// <summary>
    /// One border from an ODF shorthand, or a fallback when the property says nothing.
    /// </summary>
    /// <remarks>
    /// The value is CSS's three-part shorthand — <c>0.5pt solid #ff0000</c> — in any order, and <c>none</c> is a
    /// value in its own right meaning there is no border rather than that nothing was said. So <c>none</c> has
    /// to beat the fallback: a style setting <c>fo:border</c> and then <c>fo:border-top="none"</c> means three
    /// borders, not four.
    /// </remarks>
    private TableBorder Border(string? styleName, string propertyName, TableBorder fallback = default)
    {
        string? stated = _styles.ResolveProperty(
            styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell,
            OdfNamespaces.FoCompatible, propertyName).Value;

        if (string.IsNullOrWhiteSpace(stated)) return fallback;
        if (stated.Trim() == "none") return default;

        Length width = Length.Zero;
        Colour colour = Colour.Black;

        foreach (string part in stated.Split(' ', StringSplitOptions.RemoveEmptyEntries))
        {
            if (OdfValue.ParseLength(part) is { } measured)
            {
                width = OdfWriterUnits.ToCore(measured);
                continue;
            }

            if (part.StartsWith('#') && OdfValue.ParseColour(part) is { } stated_colour)
            {
                colour = stated_colour;
            }
        }

        // A shorthand naming a style and a colour but no width still means a border: LibreOffice draws such a
        // one hairline, which is its thinnest visible stroke rather than nothing.
        if (width <= Length.Zero) width = HairlineBorder;

        return new TableBorder(width, colour);
    }

    /// <summary>The width a border with no stated one is drawn at: half a point, Writer's hairline.</summary>
    private static readonly Length HairlineBorder = Length.FromPoints(0.5);

    private Length? TableMeasure(string? styleName, string propertyNamespace, string propertyName)
        => OdfWriterUnits.ToCore(
            OdfValue.ParseLength(
                _styles.ResolveProperty(
                    styleName, OdfStyleFamily.Table, OdfPropertyKind.Table,
                    propertyNamespace, propertyName).Value));

    /// <summary>
    /// A repeat count, clamped so that one bad attribute cannot allocate a grid.
    /// </summary>
    /// <remarks>
    /// Real documents write large repeats — a spreadsheet-shaped ODF table repeats its last column to
    /// 16384 to say "and the rest is empty" — so the value is honoured up to the grid limits and clamped
    /// rather than rejected. Absent or unparseable means one, which is what a plain column is.
    /// </remarks>
    private static int Repeat(XElement element, string attributeName)
    {
        int stated = Attribute(element, attributeName);
        return stated <= 0 ? 1 : Math.Min(stated, PageTable.MaxColumns);
    }

    private static int Attribute(XElement element, string attributeName)
        => int.TryParse(
            element.Attribute(XName.Get(attributeName, OdfNamespaces.Table))?.Value,
            System.Globalization.NumberStyles.Integer,
            System.Globalization.CultureInfo.InvariantCulture,
            out int value)
            ? value
            : 0;
}

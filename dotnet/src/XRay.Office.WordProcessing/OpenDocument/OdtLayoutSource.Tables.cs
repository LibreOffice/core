using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.WordProcessing.Layout;

namespace XRay.Office.WordProcessing.OpenDocument;

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
        List<DeclaredColumn> declared = Columns(element);
        if (declared.Count == 0) return null;

        List<PageTableRow> rows = [];
        int headerRows = 0;
        ReadRows(element, rows, ref headerRows, isHeader: false, depth: 0);

        if (rows.Count == 0) return null;

        string? styleName = element.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        List<Length> columns = [.. declared.Select(column => column.Width ?? Length.Zero)];

        return new PageTable
        {
            SectionIndex = _sectionIndex,
            ColumnWidths = columns,
            ColumnFit = Fit(declared, styleName),
            RelativeWidth = RelativeWidth(styleName),
            HorizontalPosition = HorizontalPosition(styleName),
            Rows = rows,
            HeaderRowCount = headerRows,
            StartsNewPage = StartsNewPage(styleName),
            // A document setting rather than a property of this filter -- see
            // `OdtLayoutSource.RowHeightsIncludeInsets`, and note that all 337 converted `.odt` state it.
            MinHeightIncludesInsets = _rowHeightsIncludeInsets,
            LeftIndent = TableMeasure(styleName, OdfNamespaces.FoCompatible, "margin-left")
                ?? Length.Zero,
            SpaceBefore = TableMeasure(styleName, OdfNamespaces.FoCompatible, "margin-top")
                ?? Length.Zero,
            SpaceAfter = TableMeasure(styleName, OdfNamespaces.FoCompatible, "margin-bottom")
                ?? Length.Zero,
        };
    }

    /// <summary>
    /// The grid's column widths, in order, with null for a column whose style states none.
    /// </summary>
    /// <remarks>
    /// Null rather than zero, because the two are different documents: zero is a column the file asked to
    /// be invisible and null is one it left to Writer, which then sizes it by
    /// <see cref="TableColumnFit"/>'s arithmetic. Reading the second as the first is what made a width-less
    /// table lay out with no columns at all.
    /// </remarks>
    private List<DeclaredColumn> Columns(XElement table)
    {
        List<DeclaredColumn> widths = [];
        Collect(table, 0);
        return widths;

        void Collect(XElement element, int depth)
        {
            if (depth > 8 || widths.Count >= PageTable.MaxColumns) return;

            foreach (XElement child in element.Elements())
            {
                if (!OdfNamespaces.IsTable(child.Name.NamespaceName)) continue;

                switch (child.Name.LocalName)
                {
                    // The grouping elements are transparent: a table can wrap its columns in
                    // table:table-columns, or group them for outlining, and the columns inside are the
                    // table's own either way.
                    case "table-columns" or "table-header-columns" or "table-column-group":
                        Collect(child, depth + 1);
                        break;

                    case "table-column":
                        DeclaredColumn width = ColumnWidth(child);
                        int repeat = Repeat(child, "number-columns-repeated");

                        for (int i = 0; i < repeat && widths.Count < PageTable.MaxColumns; i++)
                        {
                            widths.Add(width);
                        }

                        break;
                }
            }
        }
    }

    /// <summary>What one <c>table:table-column</c> states about its width.</summary>
    /// <param name="Width">
    /// The number the file stated, or null when it stated neither spelling. A proportion is carried here
    /// too, as a length in twips, because that is the unit Writer keeps it in — a column's relative width
    /// and its absolute one are the same <c>SwFormatFrameSize</c> field and are told apart by the size
    /// type beside it, not by their magnitude.
    /// </param>
    /// <param name="IsProportion">
    /// True when the number is <c>style:rel-column-width</c>'s proportion rather than
    /// <c>style:column-width</c>'s length. See <see cref="TableColumnFit.IsRelative"/>.
    /// </param>
    private readonly record struct DeclaredColumn(Length? Width, bool IsProportion);

    /// <summary>
    /// One column's stated width, absolute or proportional.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The absolute spelling wins when both are present, which is Writer's order rather than a choice:
    /// <c>aTableColItemMap</c> maps <c>style:column-width</c> onto <c>MID_FRMSIZE_COL_WIDTH</c> and
    /// <c>style:rel-column-width</c> onto <c>MID_FRMSIZE_REL_COL_WIDTH</c>
    /// (<c>sw/source/filter/xml/xmlitemm.cxx</c>:121-122), and the relative one sets the size type to
    /// <c>Variable</c> while the absolute one sets <c>Fixed</c> — so whichever is applied last decides,
    /// and the item map applies them in attribute order. Only 84 columns of the converted corpus state
    /// both, none of them in a document this round moved, so the tie-break is a correctness matter
    /// rather than a measured one.
    /// </para>
    /// <para>
    /// A proportion is <c>2677*</c>: an integer and a star, clamped to <c>[MINLAY, 65535]</c>
    /// (<c>xmlimpit.cxx</c>:971-986). The star is required — a value without one sets nothing at all, and
    /// the column stays width-less.
    /// </para>
    /// </remarks>
    private DeclaredColumn ColumnWidth(XElement column)
    {
        string? styleName = column.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        Length? absolute = OdfWriterUnits.ToCore(
            OdfValue.ParseLength(
                _styles.ResolveProperty(
                    styleName, OdfStyleFamily.TableColumn, OdfPropertyKind.TableColumn,
                    OdfNamespaces.Style, "column-width").Value));

        if (absolute is not null) return new DeclaredColumn(absolute, IsProportion: false);

        string? relative = _styles.ResolveProperty(
            styleName, OdfStyleFamily.TableColumn, OdfPropertyKind.TableColumn,
            OdfNamespaces.Style, "rel-column-width").Value;

        return Proportion(relative) is { } share
            ? new DeclaredColumn(Length.FromTwips(share), IsProportion: true)
            : new DeclaredColumn(null, IsProportion: false);
    }

    /// <summary>The number in a <c>2677*</c>, clamped as Writer clamps it, or null when there is none.</summary>
    private static int? Proportion(string? stated)
    {
        if (stated is null) return null;

        int star = stated.IndexOf('*', StringComparison.Ordinal);
        if (star < 0) return null;

        return int.TryParse(
                stated.AsSpan(0, star).Trim(), System.Globalization.NumberStyles.Integer,
                System.Globalization.CultureInfo.InvariantCulture, out int value)
            ? Math.Clamp(value, TableColumnFit.MinLay, ushort.MaxValue)
            : null;
    }

    /// <summary>
    /// How the columns the file left blank are to be sized, or null when it stated every one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The half of <c>SwXMLTableContext::MakeTable</c> (<c>sw/source/filter/xml/xmltbli.cxx</c>:2467) that
    /// decides <em>which</em> distribution runs, which turns on the table's horizontal orientation rather
    /// than on its width. <c>table:align</c> maps onto <c>HoriOrientation</c> through
    /// <c>aXMLTableAlignMap</c> (<c>sw/source/filter/xml/xmlithlp.cxx</c>:307): left, centre and right are
    /// real orientations, <c>margins</c> is <c>FULL</c>, and an absent attribute is <c>FULL</c> too.
    /// </para>
    /// <para>
    /// <b>Under <c>FULL</c> the table's stated width is discarded</b> — the importer's own comment is "Even
    /// if a size is specified, it will be ignored!" — and the table is as wide as the area it sits in. So
    /// the same three width-less columns come out equal in a table with no <c>table:align</c> and in the
    /// ratio 3:2:4 in one that says <c>left</c>, on the same page, from the same widths. Measured both ways.
    /// </para>
    /// </remarks>
    /// <param name="declared">The columns, with null for each that stated no width.</param>
    /// <param name="styleName">The table's own style name.</param>
    private TableColumnFit? Fit(List<DeclaredColumn> declared, string? styleName)
    {
        // A proportional column needs the distribution as much as a width-less one does: its number is a
        // share of the table rather than a length, so a grid of them cannot be used as it stands.
        if (declared.All(column => column is { Width: not null, IsProportion: false })) return null;

        Length? width = IsOriented(styleName)
            ? TableMeasure(styleName, OdfNamespaces.Style, "width")
            : null;

        return new TableColumnFit
        {
            IsAuto = [.. declared.Select(column => column.Width is null)],
            IsRelative = [.. declared.Select(column => column.IsProportion)],
            TableWidth = width,
            Rule = TableWidthRule.OpenDocument,
        };
    }

    /// <summary>
    /// The percentage of the surrounding width the table is laid out at, or null when it states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>style:rel-width</c> on the table's <c>style:table-properties</c>, which
    /// <c>aTableItemMap</c> maps onto <c>RES_FRM_SIZE</c>'s <c>MID_FRMSIZE_REL_WIDTH</c>
    /// (<c>sw/source/filter/xml/xmlitemm.cxx</c>:47) and <c>SvXMLImportItemMapper</c> turns into
    /// <c>SwFormatFrameSize::SetWidthPercent</c>, <b>clamped to 1..100</b>
    /// (<c>sw/source/filter/xml/xmlimpit.cxx</c>:936-949) — the same clamp OOXML's <c>w:tblW</c>
    /// percentage gets, so <see cref="PageTable.RelativeWidth"/> means one thing for both formats.
    /// </para>
    /// <para>
    /// <b>Only for a table with a real horizontal orientation.</b>
    /// <c>SwXMLTableContext::MakeTable_</c> (<c>sw/source/filter/xml/xmltbli.cxx</c>:2540-2582) reads the
    /// size — percentage included — only in the <c>default:</c> arm of its orientation switch: under
    /// <c>FULL</c> and <c>NONE</c>, which is <c>table:align="margins"</c> and an absent
    /// <c>table:align</c>, it sets <c>m_nWidth = MAX_WIDTH</c> and the percentage is never read. That is
    /// the same condition <see cref="Fit"/> already applies to <c>style:width</c>.
    /// </para>
    /// <para>
    /// Reading it is not cosmetic. LibreOffice's own <c>.odt</c> export writes a Word table's width as a
    /// percentage far more often than as a length: <b>29 of the 338 converted <c>.odt</c> hold an
    /// oriented <c>style:rel-width</c> table</b>, the two FAA Holdover Tables 115 and 101 of them each.
    /// A table laid out at the full measure where the file asks for 70% of it puts more text on every
    /// line of every cell, which is not a visible defect until the rows it shortens let a block onto a
    /// page the reference sends to the next one.
    /// </para>
    /// </remarks>
    /// <param name="styleName">The table's own style name.</param>
    private int? RelativeWidth(string? styleName)
    {
        if (!IsOriented(styleName)) return null;

        string? stated = _styles.ResolveProperty(
            styleName, OdfStyleFamily.Table, OdfPropertyKind.Table,
            OdfNamespaces.Style, "rel-width").Value;

        if (stated is null) return null;

        string trimmed = stated.Trim().TrimEnd('%');

        return double.TryParse(
                trimmed, System.Globalization.NumberStyles.Float,
                System.Globalization.CultureInfo.InvariantCulture, out double percent)
            ? (int)Math.Clamp(Math.Round(percent), 1, 100)
            : null;
    }

    /// <summary>Where the table sits across its area, or null to take <c>fo:margin-left</c> instead.</summary>
    /// <remarks>
    /// <para>
    /// <c>table:align</c> through <c>aXMLTableAlignMap</c>
    /// (<c>sw/source/filter/xml/xmlithlp.cxx</c>:307-316): <c>center</c> and <c>right</c> are real
    /// orientations and place the table against the middle and the far edge of the area.
    /// </para>
    /// <para>
    /// <c>left</c> is deliberately answered as null rather than as
    /// <see cref="FrameHorizontalAlignment.Left"/>. A left-aligned table that also states an
    /// <c>fo:margin-left</c> is <c>LEFT_AND_WIDTH</c> rather than <c>LEFT</c>
    /// (<c>xmltbli.cxx</c>:2521-2527), which keeps the margin; null takes
    /// <see cref="PageTable.LeftIndent"/>, which is that margin, and is the same answer as
    /// <c>Left</c> for the far commoner table that states no margin at all.
    /// </para>
    /// <para>
    /// It matters only for a table narrower than its area, so it is invisible until
    /// <see cref="RelativeWidth"/> or a stated <c>style:width</c> makes one.
    /// </para>
    /// </remarks>
    /// <param name="styleName">The table's own style name.</param>
    private FrameHorizontalAlignment? HorizontalPosition(string? styleName)
        => Align(styleName) switch
        {
            "center" => FrameHorizontalAlignment.Centre,
            "right" => FrameHorizontalAlignment.Right,
            _ => null,
        };

    /// <summary>The table's <c>table:align</c>, or null when it states none.</summary>
    private string? Align(string? styleName)
        => _styles.ResolveProperty(
            styleName, OdfStyleFamily.Table, OdfPropertyKind.Table,
            OdfNamespaces.Table, "align").Value;

    /// <summary>
    /// True when the table has a real horizontal orientation, which is what makes its stated width count.
    /// </summary>
    private bool IsOriented(string? styleName) => Align(styleName) is "left" or "center" or "right";

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
            if (!OdfNamespaces.IsTable(child.Name.NamespaceName)) continue;
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
        Length coveredTop = Length.Zero;
        Length coveredBottom = Length.Zero;
        int column = 0;

        foreach (XElement child in element.Elements())
        {
            if (!OdfNamespaces.IsTable(child.Name.NamespaceName)) continue;

            // A covered cell is a column the merge to its left or above swallowed. It carries no content
            // and gets no cell, but it advances the column counter — which is the whole reason ODF writes
            // it at all, since a row's cells are positional.
            if (child.Name.LocalName == "covered-table-cell")
            {
                // It gets no cell, and its stated rules are still charged to the row's height -- the ODF
                // spelling of what `PageTableRow.CoveredTopRule` carries for the three Word readers. A
                // covered cell in a file LibreOffice wrote names a style like any other cell.
                CellBorders coveredBorders =
                    Borders(child.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value);
                coveredTop = Length.Max(coveredTop, coveredBorders.Top.Width);
                coveredBottom = Length.Max(coveredBottom, coveredBorders.Bottom.Width);

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
                    TextDirection = TextDirection(styleName),
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
            CoveredTopRule = coveredTop,
            CoveredBottomRule = coveredBottom,
            IsHeader = isHeader,
            MinHeight = RowHeight(element).Height,
            HasExactHeight = RowHeight(element).IsExact,
            CanSplit = CanSplit(element),
        };
    }

    /// <summary>
    /// Whether the row's content may be broken across a page.
    /// </summary>
    /// <remarks>
    /// ODF states it the other way up from the three Word formats: <c>fo:keep-together</c> on a
    /// <c>style:table-row-properties</c> is <c>always</c> to forbid the break and <c>auto</c> to allow it,
    /// which <c>xmloff/source/text/txtprmap.cxx</c> maps to <c>IsSplitAllowed</c> through its negating
    /// <c>XML_TYPE_TEXT_NKEEP</c>. Absent means allowed, which is Writer's own default for a row and what
    /// its export writes explicitly.
    /// </remarks>
    private bool CanSplit(XElement row)
    {
        string? styleName = row.Attribute(XName.Get("style-name", OdfNamespaces.Table))?.Value;

        return _styles.ResolveProperty(
            styleName, OdfStyleFamily.TableRow, OdfPropertyKind.TableRow,
            OdfNamespaces.FoCompatible, "keep-together").Value != "always";
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
    private VerticalTextAlignment VerticalAlignment(string? styleName)
        => _styles.ResolveProperty(
            styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell,
            OdfNamespaces.Style, "vertical-align").Value switch
        {
            "middle" => VerticalTextAlignment.Middle,
            "bottom" => VerticalTextAlignment.Bottom,
            _ => VerticalTextAlignment.Top,
        };

    /// <summary>
    /// Which way a cell's text runs, from <c>writing-mode</c> on its <c>style:table-cell-properties</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's own ODF filter maps <em>both</em> spellings of the attribute onto one item: its table-cell
    /// item map holds <c>style:writing-mode</c> and <c>loext:writing-mode</c> side by side, each against
    /// <c>RES_FRAMEDIR</c> (<c>sw/source/filter/xml/xmlitemm.cxx</c>:281-282), and the value handler takes
    /// <c>bt-lr</c> as <c>SvxFrameDirection::Vertical_LR_BT</c> and <c>tb-rl90</c> as
    /// <c>Vertical_RL_TB90</c> whichever namespace carried them
    /// (<c>sw/source/filter/xml/xmlimpit.cxx</c>:1008-1030), leaving every other value to the ordinary
    /// <c>XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT</c> handler.
    /// </para>
    /// <para>
    /// So here the extension namespace is chosen by the <em>value</em> and not by the property, which is a
    /// shape none of this project's recorded ODF namespace surprises has: the exporter writes the attribute
    /// as <c>style:</c> for every value ODF 1.3 defines and as <c>loext:</c> for the two it does not —
    /// <c>sw/source/filter/xml/xmlexpit.cxx</c>:193-220 for this item, and
    /// <c>SvXMLExportPropertyMapper::_exportXML</c>'s <c>CheckExtendedNamespace</c>
    /// (<c>xmloff/source/style/xmlexppr.cxx</c>:947-952 and :1108-1118) for every other, under a comment
    /// saying there is no generic mechanism for it. Measured on 26.2.4.2 with one cell per spelling in one
    /// table: <c>style:writing-mode="bt-lr"</c> and <c>loext:writing-mode="bt-lr"</c> are drawn identically,
    /// so the extension namespace is an export convention and not an import requirement — and reading only
    /// the specification's spelling finds none of the 86 turned cells the converted corpus holds.
    /// </para>
    /// <para>
    /// The two spellings are therefore one property and are resolved together, the <em>level</em> before the
    /// spelling, so an outer style's spelling cannot beat an inner style's.
    /// </para>
    /// <para>
    /// <c>tb-lr</c> is a third direction and not one of the two: 26.2.4.2 turns it clockwise like
    /// <c>tb-rl</c> but stacks the lines <em>rightwards</em>. It is answered as
    /// <see cref="CellTextDirection.TopToBottomRightToLeft"/>, which is right about the glyphs and about the
    /// break and wrong only about which way a second line goes; upright — what LibreOffice's own DOCX
    /// importer does with the corresponding <c>tbLrV</c>, <em>"we can't handle these"</em> — would be wrong
    /// about all three. No document of the converted corpus states it on a cell.
    /// </para>
    /// </remarks>
    private CellTextDirection TextDirection(string? styleName)
    {
        OdfProperty stated = _styles.ResolveWithoutDefaults(
            styleName, OdfStyleFamily.TableCell, OdfPropertyKind.TableCell, WritingModeSpellings, out _);

        if (!stated.HasValue)
        {
            stated = _styles.ResolveFromDefaults(
                OdfStyleFamily.TableCell, OdfPropertyKind.TableCell, OdfNamespaces.Style, "writing-mode");
        }

        return stated.Value switch
        {
            "bt-lr" => CellTextDirection.BottomToTopLeftToRight,
            "tb-rl" or "tb-rl90" or "tb-lr" or "tb" => CellTextDirection.TopToBottomRightToLeft,
            _ => CellTextDirection.LeftToRight,
        };
    }

    /// <summary>
    /// The two spellings of <c>writing-mode</c>, the extension one first.
    /// </summary>
    /// <remarks>
    /// Order decides only what a single style stating both would answer, which no exporter writes; the
    /// extension spelling leads because it is the one that carries a turned value.
    /// </remarks>
    private static readonly (string Namespace, string Name)[] WritingModeSpellings =
    [
        (OdfNamespaces.LoExt, "writing-mode"),
        (OdfNamespaces.Style, "writing-mode"),
    ];

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

    /// <summary>
    /// Whether the table's own style asks for a page before it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The ODF spelling of what a DOCX says with a <c>&lt;w:br w:type="page"/&gt;</c> in the paragraph in
    /// front of the table — <see cref="Layout.PageTable.StartsNewPage"/>, whose own remark cites this
    /// attribute and which no ODF document could set until now. LibreOffice's exporter writes the break
    /// onto the <c>style:table-properties</c> of the table's automatic style, so a Word document's break
    /// before a table survives a round trip as an attribute of the table rather than of a paragraph.
    /// </para>
    /// <para>
    /// [bin] Measured on 26.2.4.2's own conversion of <c>words-vmerge-covered-rule.docx</c>, whose eight
    /// arms are one per page: the reference draws 8 pages of it and this tree drew <b>1</b> before this was
    /// read. <b>12 of the 337 converted <c>.odt</c> state it, over 40 table styles.</b>
    /// </para>
    /// </remarks>
    private bool StartsNewPage(string? styleName)
        => _styles.ResolveProperty(
            styleName, OdfStyleFamily.Table, OdfPropertyKind.Table,
            OdfNamespaces.FoCompatible, "break-before").Value == "page";

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

using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;

namespace XRay.Office.WordProcessing.Ooxml;

/// <content>
/// Reading a <c>w:tbl</c> into the grid the layout engine takes.
/// </content>
/// <remarks>
/// <para>
/// DOCX states the grid in <c>w:tblGrid</c> and then fills rows against it, which is the same arrangement
/// ODF uses — but the two express the same two facts differently, and each difference is a place to get it
/// wrong.
/// </para>
/// <list type="bullet">
///   <item>
///     A horizontal merge is <c>w:gridSpan</c> and there is <em>no placeholder</em> for the columns it
///     swallows, so a row's next cell starts at the previous cell's column plus its span. ODF writes a
///     <c>table:covered-table-cell</c> instead and advances by one. Applying either rule to the other
///     format shifts every cell after the first merge.
///   </item>
///   <item>
///     A vertical merge is <c>w:vMerge</c>, which is not a count but a state: <c>restart</c> begins one and
///     a bare <c>w:vMerge</c> continues it. So a cell's row span is not stated anywhere and has to be
///     counted by looking down the following rows for continuations in the same column — which means the
///     rows have to be read before any span is known.
///   </item>
///   <item>
///     Cell padding is stated twice: <c>w:tblCellMar</c> for the table and <c>w:tcMar</c> per cell, and the
///     cell overrides <em>per side</em>. LibreOffice's own export writes a <c>w:tcMar</c> holding only the
///     side that differs, so a reader taking the cell's block as a whole loses the other three.
///   </item>
///   <item>
///     <c>w:tblInd</c> is not where the table's left edge goes — see <see cref="LeftEdge"/>. Word measures
///     it to the cell's text and Writer places a table by the centre of its left border, so the two differ
///     by half a border and a reader taking the indent literally offsets the whole grid.
///   </item>
/// </list>
/// <para>
/// The measures are twips (<c>w:type="dxa"</c>), which is the unit Writer lays out in, so nothing needs
/// snapping — unlike ODF, whose centimetres have to be rounded onto the twip grid before they agree.
/// </para>
/// </remarks>
public sealed partial class DocxLayoutSource
{
    /// <summary>
    /// Word's default cell padding, for a table stating none: 108 twips at the sides, nothing vertically.
    /// </summary>
    /// <remarks>
    /// 0.19 cm, which is the value Word's own table dialogue starts at. It comes out of the cell's width,
    /// so defaulting it to zero breaks a narrow cell's text one word late.
    /// </remarks>
    private static readonly CellPadding DefaultCellPadding = CellPadding.Word;

    /// <summary>Reads a table, or returns null when it declares no usable grid.</summary>
    private PageTable? Table(XElement element)
    {
        XElement? properties = Word.Child(element, "tblPr");

        List<Length?> declared = Columns(element);
        if (declared.Count == 0) return null;

        List<Length> columns = [.. declared.Select(width => width ?? Length.Zero)];

        string? styleId = Word.Attribute(Word.Child(properties, "tblStyle"), "val");

        // The style's cell margins under the table's own, side by side with its borders and for the same
        // reason: `w:tblCellMar` is a table-level property, `endTableGetTableStyle` merges the style's in
        // before the table's, and a style that states one is stating how tall every row in the table is.
        CellPadding tablePadding = Padding(
            Word.Child(properties, "tblCellMar"),
            StyleCellPadding(styleId, DefaultCellPadding));

        // Taken before the rows are read, exactly as ReadParagraph takes it, and for the same reason: the
        // walk over the cells reads paragraphs, and the first of them would otherwise eat the break that
        // belongs to the table. Inside a cell it is inert, which is why the break simply vanished.
        bool breaksPage = _pageBreakPending;
        _pageBreakPending = false;

        List<PendingRow> rows = [];

        // Counted around the rows rather than around this table's own properties, because a cell's blocks
        // are read while the rows are, and a table inside one of them is what makes this table an enclosing
        // level. See LeftEdge for the one thing the count decides.
        // The table style's paragraph formatting applies to every paragraph in the table's cells, and it
        // is the layer that makes table text compact: `Table Grid`, which Word puts on nearly every table,
        // sets `w:spacing w:after="0" w:line="240"`. Saved and restored so a nested table's style applies
        // only inside it.
        IReadOnlyList<XElement>? enclosing = _tableStyle;
        IReadOnlyList<XElement>? enclosingRun = _tableStyleRun;
        _tableStyle = _styles.TableStyleParagraphProperties(styleId);

        // Which conditional layers this table asked for, and how many rows there are — the second
        // because `lastRow` cannot be decided while the rows are still being read.
        WordTableLook look = WordTableLook.Read(properties);
        int rowCount = CountRows(element, depth: 0);

        // How wide a band is belongs to the *style*, not to the table: there is no table-level element
        // for it at all. Resolved once here so that every row of the table counts in the same units.
        (int Rows, int Columns) bands = _styles.TableStyleBandSizes(styleId);

        _tableDepth++;
        try
        {
            ReadRows(element, rows, tablePadding, properties, depth: 0, styleId, look, rowCount, bands);
        }
        finally
        {
            _tableDepth--;
            _tableStyle = enclosing;
            _tableStyleRun = enclosingRun;
        }

        if (rows.Count == 0) return null;

        // Before LeftEdge, which measures the table's position from the first cell's left border.
        ApplyGridBorders(rows, TableBorders(properties, styleId));

        return new PageTable
        {
            SectionIndex = _sectionIndex,
            ColumnWidths = columns,
            ColumnFit = Fit(declared, properties),
            RelativeWidth = Percentage(Word.Child(properties, "tblW")),
            Rows = Resolved(rows),
            HeaderRowCount = HeadingRows(rows),
            LeftIndent = LeftEdge(properties, rows, isNested: _tableDepth > 0),
            HorizontalPosition = HorizontalPositionOf(properties),
            IsPositioned = Word.Child(properties, "tblpPr") is not null,
            VerticalOffset = Twips(Word.Child(properties, "tblpPr"), "tblpY") ?? Length.Zero,
            VerticalOrigin = VerticalOriginOf(Word.Child(properties, "tblpPr")),
            LowerSpacing = Twips(Word.Child(properties, "tblpPr"), "bottomFromText") ?? Length.Zero,
            JoinsBordersLikeWord = true,
            MinHeightIncludesInsets = true,
            StartsNewPage = breaksPage,
        };
    }

    /// <summary>
    /// How the table is aligned across the area it sits in, or null when it is placed by its indent.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>w:tblpXSpec</c> maps onto Writer's horizontal orientations exactly as
    /// <c>TablePositionHandler::getTablePosition</c> maps it
    /// (<c>sw/source/writerfilter/dmapper/TablePositionHandler.cxx:98</c>): centre, inside, left, outside
    /// and right, with anything else — including a table stating only <c>w:tblpX</c> — left as a plain
    /// distance, which <see cref="PageTable.LeftIndent"/> already is.
    /// </para>
    /// <para>
    /// Only the two anchors that resolve against the text area are honoured: <c>margin</c>, which is
    /// <c>PAGE_PRINT_AREA</c>, and <c>text</c>, which is <c>FRAME</c> — the paragraph's own column, the
    /// same rectangle for a body that has one column. <c>w:horzAnchor="page"</c> would need the page's
    /// own edges, which nothing on the way to <see cref="PageTable"/> carries, so a table anchored to the
    /// page keeps the placement it had rather than being centred against the wrong rectangle. Three of
    /// the corpus's eighteen anchored tables say <c>page</c>.
    /// </para>
    /// <para>
    /// The vertical half is read by <see cref="VerticalOriginOf"/> beside this — <c>w:tblpY</c> and
    /// <c>w:vertAnchor</c>, but <em>not</em> <c>w:tblpYSpec</c>, which names an edge (<c>top</c>,
    /// <c>center</c>, <c>bottom</c>) rather than a distance and which no corpus document states.
    /// </para>
    /// <para>
    /// The commoner mechanism by far is the plain <c>w:jc</c> beside it, which was not read either: 31 of
    /// the words track's 134 DOCX files state one and 315 of their 320 occurrences say <c>center</c>. Not
    /// read from a <em>table style</em> yet, which <c>StyleSheetTable</c> also honours
    /// (<c>StyleSheetTable.cxx:683</c>).
    /// </para>
    /// </remarks>
    private static FrameHorizontalAlignment? HorizontalPositionOf(XElement? tableProperties)
    {
        if (Word.Child(tableProperties, "tblpPr") is { } position)
        {
            if (Word.Attribute(position, "horzAnchor") is "page") return null;

            switch (Word.Attribute(position, "tblpXSpec"))
            {
                case "center": return FrameHorizontalAlignment.Centre;
                case "left": return FrameHorizontalAlignment.Left;
                case "right": return FrameHorizontalAlignment.Right;
                case "inside": return FrameHorizontalAlignment.Inside;
                case "outside": return FrameHorizontalAlignment.Outside;
                default: break;
            }
        }

        // A table's own `w:jc`, which is a different thing from the paragraph alignment of the same
        // name and reached only as a direct child of `w:tblPr`. `convertTableJustification`
        // (<c>sw/source/writerfilter/dmapper/ConversionHelper.cxx:473</c>) maps `center` and
        // `right`/`end` onto orientations and everything else — `left`, `start`, absent — onto
        // `LEFT_AND_WIDTH`, which is the stated indent and so already what this reader does.
        return Word.Attribute(Word.Child(tableProperties, "jc"), "val") switch
        {
            "center" => FrameHorizontalAlignment.Centre,
            "right" or "end" => FrameHorizontalAlignment.Right,
            _ => null,
        };
    }

    /// <summary>
    /// What a positioned table's <c>w:tblpY</c> is measured from.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The three <c>w:vertAnchor</c> values map onto Writer's relation constants exactly as
    /// <c>TablePositionHandler::getTablePosition</c> maps them
    /// (<c>sw/source/writerfilter/dmapper/TablePositionHandler.cxx:133-141</c>): <c>page</c> is
    /// <c>PAGE_FRAME</c>, <c>margin</c> is <c>PAGE_PRINT_AREA</c> and <c>text</c> is <c>FRAME</c>.
    /// </para>
    /// <para>
    /// An absent attribute reads as <c>text</c>, which is ECMA-376's default and is what the two corpus
    /// documents that omit it behave as: <c>083_Printable_Graph_Paper_Template_Customizable_Format</c>
    /// states <c>w:tblpY="525"</c> with no anchor and 26.2.4.2 draws its first rule 26.25 twentieths of
    /// a point below the flow's position, to 0.06 pt.
    /// </para>
    /// </remarks>
    private static FrameVerticalOrigin VerticalOriginOf(XElement? position)
        => Word.Attribute(position, "vertAnchor") switch
        {
            "page" => FrameVerticalOrigin.Page,
            "margin" => FrameVerticalOrigin.PageMargin,
            _ => FrameVerticalOrigin.Paragraph,
        };

    /// <summary>
    /// Where the table's left edge goes, which is not what <c>w:tblInd</c> says.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer positions a table by the <em>centre</em> of its left border; Word states an indent whose
    /// meaning depends on the file's compatibility mode, and
    /// <c>DomainMapperTableHandler::endTableGetTableStyle</c> —
    /// <c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>, the block commented "Table
    /// position in Office is computed in 2 different ways" — converts one to the other. Two rules, and the
    /// document picks between them:
    /// </para>
    /// <list type="bullet">
    ///   <item>
    ///     <b>Word 2013 and later</b> (<c>compatibilityMode</c> 15 or more), and <em>every</em> nested
    ///     table whatever the mode: the indent is to the outer edge of the left border, so the centre is
    ///     half a border further right. A nested table's indent is also floored at zero first.
    ///   </item>
    ///   <item>
    ///     <b>Word 2007 to 2010</b> (mode 14 or less, which is also what an absent
    ///     <c>compatibilityMode</c> means), for a table that is not nested: the indent is to the cell's
    ///     <em>text</em>, so the border's centre sits a whole cell padding to the <em>left</em> of it —
    ///     <c>max</c> of the first cell's left padding and half the border, subtracted rather than added.
    ///   </item>
    /// </list>
    /// <para>
    /// The difference is not academic: the corpus table indented <c>w:tblInd w:w="-5"</c> with a 0.5 pt
    /// border renders at the page's left margin under mode 15 and three points to the left of it under
    /// mode 12, because its cells are padded by 55 twips.
    /// </para>
    /// </remarks>
    /// <param name="properties">The <c>w:tblPr</c>.</param>
    /// <param name="rows">The rows, whose first cell states the border and padding the rules need.</param>
    /// <param name="isNested">True when another table encloses this one.</param>
    private Length LeftEdge(XElement? properties, List<PendingRow> rows, bool isNested)
    {
        XElement? indent = Word.Child(properties, "tblInd");
        Length stated = Twips(indent) ?? Length.Zero;

        // The first cell of the first row: only its border and padding move the table, because only its
        // left edge is the table's. A row indented differently from the first is not modelled.
        PageTableCell? first =
            rows.Count > 0 && rows[0].Cells.Count > 0 ? rows[0].Cells[0].Definition : null;
        Length border = first?.Borders.Left.Width ?? Length.Zero;

        // A positioned table is placed by `w:tblpX` and not by `w:tblInd`, and it is corrected twice
        // over rather than once — see `PositionedLeftEdge`.
        if (Word.Child(properties, "tblpPr") is { } floated && !isNested)
        {
            return PositionedLeftEdge(floated, first, border);
        }

        if (isNested || _compatibilityMode >= 15)
        {
            // A nested table's indent is relative to the enclosing cell's text area, which cannot be to the
            // left of it — a negative one is Word's way of saying "no indent" rather than an overhang.
            if (isNested && stated < Length.Zero) stated = Length.Zero;

            return stated + (border / 2);
        }

        // Only an indent the document actually states makes Word measure to the text. Without one Word
        // invents an indent of its own, and what it invents behaves like the modern rule.
        Length distance = indent is null
            ? border / 2
            : Length.Max(border / 2, first?.Padding.Left ?? Length.Zero);

        return stated - distance;
    }

    /// <summary>
    /// Where a positioned table's left edge sits, from <c>w:tblpX</c> rather than <c>w:tblInd</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A floated table becomes a frame, and <c>DomainMapperTableHandler::endTableGetTableStyle</c>
    /// moves that frame left twice: by the first cell's left margin when the file's
    /// <c>compatibilityMode</c> is below 15
    /// (<c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>:543), and by half the first
    /// cell's left border always (:612). Both are <c>lcl_DecrementHoriOrientPosition</c>, and they are
    /// a <em>sum</em> — unlike the <c>w:tblInd</c> rule beside this, which takes the larger of the two.
    /// </para>
    /// <para>
    /// Measured against 24.2.7.2 on an authored probe: a two-cell table floated with
    /// <c>w:horzAnchor="margin"</c> and no <c>w:tblpX</c> draws its first cell's text at
    /// <b>x = 71.65 pt</b> against the same table in the flow at <b>78.00</b> — 6.35 pt left, which is
    /// the 108-twip cell margin plus half a one-point border. Adding <c>w:tblpX="-594"</c> moves it to
    /// <b>41.95</b>, exactly 29.7 pt further, so the offset itself is applied unchanged.
    /// </para>
    /// <para>
    /// It is worth 35 pt on <c>087_Printable_Graph_Paper_Template_Green_Theme</c>, whose grid the
    /// reference draws from x = 35.3 and which we drew from 70.6 — a dense grid a whole page across,
    /// so every line of it landed between two of the reference's.
    /// </para>
    /// <para>
    /// Only the offset form. A table stating <c>w:tblpXSpec</c> is aligned rather than placed, and
    /// <c>lcl_DecrementHoriOrientPosition</c> writes a position that a non-<c>NONE</c> orientation then
    /// ignores — which is why <see cref="HorizontalPositionOf"/> answering non-null takes this out of
    /// the picture.
    /// </para>
    /// <para>
    /// <b><c>w:horzAnchor="page"</c> measures from the sheet's own left edge</b>, so the section's left
    /// margin comes off before the offset joins the text area's coordinates —
    /// <see cref="SectionLeftMargin"/>, which is the one piece of page geometry this reader carries. An
    /// earlier round excluded the page anchor outright on the grounds that nothing here knew the margin,
    /// and that left the offset unapplied rather than misapplied: measured on authored fixtures against
    /// <em>both</em> installed references, which agree to a tenth of a point, <c>w:tblpX="705"</c> draws
    /// its first cell's text at <b>x = 35.1</b> anchored to the page and at <b>107.1</b> anchored to the
    /// margin — 72 pt apart, which is the margin exactly. The two decrements above apply either way.
    /// </para>
    /// <para>
    /// It is worth 37 pt on <c>Case-Study-Heathrow-Airport.docx</c>, whose whole first page is one such
    /// table: the reference draws its first cell at x = 40.50, which is 705 twips from the sheet plus
    /// the cell's own 108-twip margin, and we drew it at 77.65 — at the page margin, as though the
    /// offset were not there. See <c>probes/words-page-anchored-table/</c>.
    /// </para>
    /// </remarks>
    private Length PositionedLeftEdge(XElement position, PageTableCell? first, Length border)
    {
        Length stated = Twips(position, "tblpX") ?? Length.Zero;

        Length margin = _compatibilityMode >= 15
            ? Length.Zero
            : first?.Padding.Left ?? Length.Zero;

        // The sheet's edge is `SectionLeftMargin` to the left of the text area every other length here
        // is measured from.
        Length origin = Word.Attribute(position, "horzAnchor") is "page"
            ? SectionLeftMargin
            : Length.Zero;

        return stated - origin - margin - (border / 2);
    }

    /// <summary>The grid's column widths, in order.</summary>
    /// <remarks>
    /// From <c>w:tblGrid</c> alone. A cell's own <c>w:tcW</c> is not consulted: it is advisory, disagrees
    /// with the grid in real documents, and Word itself lays a fixed table out from the grid — a reader
    /// preferring the cell's width would place two cells of one row at different edges.
    /// </remarks>
    private static List<Length?> Columns(XElement table)
    {
        List<Length?> widths = [];

        foreach (XElement column in Word.Children(Word.Child(table, "tblGrid"), "gridCol"))
        {
            if (widths.Count >= PageTable.MaxColumns) break;

            // A w:w of zero is how Word writes a column it has not sized, and it is not a zero-width
            // column: nothing in the format spells that, and the file that means it writes no w:w at all.
            Length? stated = Twips(column);
            widths.Add(stated is null || stated <= Length.Zero ? null : stated);
        }

        return widths;
    }

    /// <summary>
    /// How the columns the file left unsized are to be sized, or null when it sized every one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word's grid never reaches Writer as widths at all. <c>DomainMapperTableManager::endOfRowAction</c>
    /// turns it into relative <c>TableColumnSeparator</c>s and the table is built with <em>equal</em>
    /// columns before they are applied, so an unsized column's separator — which comes out at zero — is
    /// dropped and its divider stays where the equal division put it. See <see cref="TableColumnFit"/>.
    /// </para>
    /// <para>
    /// The table's own width is <c>w:tblW</c> when it states one in twips, and otherwise the grid added up
    /// (<c>DomainMapperTableManager.cxx</c>:647, "convert sum of grid twip values"). When that is nothing
    /// either the table is left variable and fills the area it sits in, which is what a
    /// <c>w:tblW w:w="0" w:type="auto"</c> beside a grid of zeroes means.
    /// </para>
    /// </remarks>
    /// <param name="declared">The grid, with null for each column that stated no width.</param>
    /// <param name="properties">The <c>w:tblPr</c>.</param>
    private static TableColumnFit? Fit(List<Length?> declared, XElement? properties)
    {
        if (declared.All(width => width is not null)) return null;

        // A percentage width is not a width the fit can honour, and it must not fall through to the grid
        // sum either: the area it is handed has already been scaled to the stated percentage, so leaving
        // the table's own width unstated is what makes it fill exactly that. See
        // <see cref="PageTable.RelativeWidth"/>.
        if (Percentage(Word.Child(properties, "tblW")) is not null)
        {
            return new TableColumnFit
            {
                IsAuto = [.. declared.Select(column => column is null)],
                TableWidth = null,
                Rule = TableWidthRule.Word,
            };
        }

        Length? width = Twips(Word.Child(properties, "tblW"));
        if (width is null || width <= Length.Zero)
        {
            Length grid = Length.Zero;
            foreach (Length? column in declared) grid += column ?? Length.Zero;
            width = grid > Length.Zero ? grid : null;
        }

        return new TableColumnFit
        {
            IsAuto = [.. declared.Select(column => column is null)],
            TableWidth = width,
            Rule = TableWidthRule.Word,
        };
    }

    /// <summary>Reads the rows, following the change-tracking wrappers a row can sit inside.</summary>
    private void ReadRows(
        XElement element,
        List<PendingRow> rows,
        CellPadding tablePadding,
        XElement? tableProperties,
        int depth,
        string? styleId,
        WordTableLook look,
        int rowCount,
        (int Rows, int Columns) bands)
    {
        if (depth > 8) return;

        foreach (XElement child in element.Elements())
        {
            if (rows.Count >= PageTable.MaxRows) return;

            if (Word.Is(child, "tr"))
            {
                rows.Add(Row(
                    child, tablePadding, tableProperties, styleId, look, rows.Count, rowCount, bands));
                continue;
            }

            // A row can be wrapped by a tracked insertion or a content control. Its cells are the table's
            // either way — a walk that stopped here would lose the row rather than the wrapper.
            if (Word.Is(child, "sdt") || Word.Is(child, "sdtContent")
                || Word.Is(child, "customXml") || Word.Is(child, "ins"))
            {
                ReadRows(
                    child, rows, tablePadding, tableProperties, depth + 1, styleId, look, rowCount, bands);
            }
        }
    }

    /// <summary>
    /// How many <c>w:tr</c> the table holds, counted before any of them is read.
    /// </summary>
    /// <remarks>
    /// Only <c>lastRow</c> needs this, and it needs it before the first row is read: a cell's
    /// conditional formatting decides how its text is measured, so it cannot be settled afterwards.
    /// Follows the same wrappers <see cref="ReadRows"/> does, since a row inside a tracked insertion is
    /// still a row and miscounting by one puts <c>lastRow</c> on the wrong one.
    /// </remarks>
    private static int CountRows(XElement element, int depth)
    {
        if (depth > 8) return 0;

        int count = 0;
        foreach (XElement child in element.Elements())
        {
            if (Word.Is(child, "tr")) count++;
            else if (Word.Is(child, "sdt") || Word.Is(child, "sdtContent")
                     || Word.Is(child, "customXml") || Word.Is(child, "ins"))
            {
                count += CountRows(child, depth + 1);
            }
        }

        return count;
    }

    private PendingRow Row(
        XElement element,
        CellPadding tablePadding,
        XElement? tableProperties,
        string? styleId,
        WordTableLook look,
        int rowIndex,
        int rowCount,
        (int Rows, int Columns) bands)
    {
        XElement? properties = Word.Child(element, "trPr");
        List<PendingCell> cells = [];
        int column = SkippedBefore(properties);

        // The row's own cells, so that `lastCol` names the last one this row actually has rather than the
        // grid's width — a row ending in a `w:gridSpan` reaches the grid's edge with fewer cells.
        List<XElement> children = [.. Cells(element, 0)];
        int lastIndex = children.Count - 1;
        int index = -1;

        foreach (XElement child in children)
        {
            if (column >= PageTable.MaxColumns) break;

            index++;
            XElement? cellProperties = Word.Child(child, "tcPr");
            int span = Math.Max(1, Number(Word.Child(cellProperties, "gridSpan")) ?? 1);

            // Set around `ReadCell` alone: the cell's paragraphs are read inside it, and a nested table
            // there restores its own on the way out.
            bool isFirstRow = rowIndex == 0;
            bool isLastRow = rowCount > 0 && rowIndex == rowCount - 1;
            bool isFirstColumn = index == 0;
            bool isLastColumn = index == lastIndex;

            WordTableStyleConditions conditions = new(
                look,
                isFirstRow,
                isLastRow,
                isFirstColumn,
                isLastColumn,
                // The band is counted over the rows and columns the edge layers do not claim, and
                // `012_Project_Timeline_Template_Black_and_Brown_Theme` is what fixes that: its bands
                // land on table rows 2, 4, 6 and 8, which is the heading row excluded. A row inside an
                // edge region has no band at all rather than band nought.
                Band(rowIndex, isFirstRow, isLastRow, look.FirstRow, look.LastRow, bands.Rows),
                Band(index, isFirstColumn, isLastColumn, look.FirstColumn, look.LastColumn, bands.Columns));

            _tableStyleRun = _styles.TableStyleRunProperties(styleId, conditions);

            // Read before the cell, because the cell's paragraphs are read inside `ReadCell` and a
            // continuation cell's must not count in their lists — see `_inCoveredCell`.
            VerticalMerge merge = Merge(cellProperties);
            bool outerCovered = _inCoveredCell;
            _inCoveredCell = outerCovered || merge == VerticalMerge.Continue;

            List<PageBlock> cellBlocks;
            try
            {
                cellBlocks = ReadCell(child);
            }
            finally
            {
                _inCoveredCell = outerCovered;
            }

            cells.Add(new PendingCell(
                new PageTableCell
                {
                    Blocks = cellBlocks,
                    Column = column,
                    ColumnSpan = span,
                    Padding = Padding(Word.Child(cellProperties, "tcMar"), tablePadding),
                    VerticalAlignment = VerticalAlignment(cellProperties),
                    TextDirection = TextDirection(cellProperties),
                    Shading = Shading(cellProperties)
                              ?? ConditionalShading(styleId, conditions),
                },
                merge,
                OwnBorders(cellProperties)));

            // By the span, because DOCX writes no placeholder for a swallowed column.
            column += span;
        }

        return new PendingRow(
            cells,
            IsHeading: Word.IsOn(Word.Child(properties, "tblHeader"))
                       || Word.Child(properties, "tblHeader") is not null,
            RowHeight(properties, children),
            // `w:cantSplit` is on when it is present without a `w:val`, which is how Word writes it, and
            // LibreOffice reads the same element the same way — "row can't break across pages if
            // nIntValue == 1" (`dmapper/TablePropertiesHandler.cxx`).
            CanSplit: !Word.IsOn(Word.Child(properties, "cantSplit")));
    }

    /// <summary>
    /// Which band a row or column falls in, or null when an edge layer claims it.
    /// </summary>
    /// <remarks>
    /// The index counts only the rows (or columns) outside the <c>firstRow</c>/<c>lastRow</c> regions
    /// <em>the table asked for</em>: a style declaring a <c>firstRow</c> layer that the table's
    /// <c>w:tblLook</c> switches off leaves its heading row in the banding, which is what Word does.
    /// Only the first row and the last can be in a region, so subtracting one for a claimed leading
    /// edge is the whole of the arithmetic.
    /// </remarks>
    /// <param name="index">The row's or cell's own index.</param>
    /// <param name="isFirst">Whether it is the leading one.</param>
    /// <param name="isLast">Whether it is the trailing one.</param>
    /// <param name="claimsFirst">Whether the table asked for the leading edge's layer.</param>
    /// <param name="claimsLast">Whether the table asked for the trailing edge's layer.</param>
    /// <param name="size">The band size, at least one.</param>
    private static int? Band(
        int index, bool isFirst, bool isLast, bool claimsFirst, bool claimsLast, int size)
    {
        if ((claimsFirst && isFirst) || (claimsLast && isLast)) return null;

        int within = index - (claimsFirst ? 1 : 0);
        return within < 0 ? null : within / Math.Max(1, size);
    }

    /// <summary>
    /// The fill a table style's conditional <c>w:tcPr</c> layers give a cell, or null for none.
    /// </summary>
    /// <remarks>
    /// Asked only after the cell's own <c>w:shd</c>, which is direct formatting and absolute. The
    /// layers arrive most specific first and the first one stating a <c>w:shd</c> wins outright —
    /// there is no blending between layers, only between a <c>w:shd</c>'s own foreground and
    /// background, which <see cref="ShadeColour"/> does.
    /// </remarks>
    private Colour? ConditionalShading(string? styleId, WordTableStyleConditions conditions)
    {
        foreach (XElement layer in _styles.TableStyleCellProperties(styleId, conditions))
        {
            if (Word.Child(layer, "shd") is { } shade) return ShadeColour(shade);
        }

        return null;
    }

    /// <summary>
    /// How many grid columns a row leaves empty before its first cell — <c>w:gridBefore</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A row need not start at the grid's first column. <c>w:gridBefore</c> says how many it skips, and
    /// like <c>w:gridSpan</c> there is <em>no placeholder cell</em> for the columns skipped — so a reader
    /// that starts every row at column zero puts the row's first cell in the wrong column and gives it the
    /// wrong width, and every cell after it too. On a title block whose narrow first column is skipped by
    /// the rows that carry the title, that means the title is measured against a column a fifth of its
    /// width and wraps to one word a line, which is enough to push the block onto a page of its own.
    /// </para>
    /// <para>
    /// LibreOffice reaches the same layout by *materialising* the skipped columns:
    /// <c>TableManager::endRow</c> (<c>sw/source/writerfilter/dmapper/TableManager.cxx</c>:667–702) adds
    /// <c>w:gridBefore</c> borderless empty cells to the front of the row. An absent cell and a borderless
    /// empty one draw the same nothing, so shifting the column index is the same answer with no cell to
    /// lay out.
    /// </para>
    /// <para>
    /// <c>w:wBefore</c> is deliberately not read. It is the width of the skipped span and is advisory in
    /// exactly the way <c>w:tcW</c> is — the grid decides, and a document whose <c>w:wBefore</c> disagrees
    /// with the columns it covers would otherwise put one row's cells at a different edge from the rest.
    /// <c>w:gridAfter</c> needs nothing at all: a row simply stops early, which it already does.
    /// </para>
    /// </remarks>
    private static int SkippedBefore(XElement? rowProperties)
    {
        int before = Number(Word.Child(rowProperties, "gridBefore")) ?? 0;
        return Math.Clamp(before, 0, PageTable.MaxColumns);
    }

    /// <summary>
    /// A row's cells, following the wrappers a cell can sit inside.
    /// </summary>
    /// <remarks>
    /// The same wrappers a row can sit inside, and for the same reason — but one level further down, which
    /// is where a form puts them: a content control over a single table cell is written as a
    /// <c>w:sdt</c> between the <c>w:tr</c> and its <c>w:tc</c>, and it is how Word marks up every
    /// fill-in box of a printed form. Taking only the row's direct <c>w:tc</c> children dropped the whole
    /// cell — the corpus's own proposal form lost thirty-six of them, a quarter of its text.
    /// </remarks>
    private static IEnumerable<XElement> Cells(XElement row, int depth)
    {
        if (depth > 8) yield break;

        foreach (XElement child in row.Elements())
        {
            if (Word.Is(child, "tc"))
            {
                yield return child;
                continue;
            }

            if (!Word.Is(child, "sdt") && !Word.Is(child, "sdtContent")
                && !Word.Is(child, "customXml") && !Word.Is(child, "ins"))
            {
                continue;
            }

            foreach (XElement nested in Cells(child, depth + 1)) yield return nested;
        }
    }

    /// <summary>
    /// A row's declared height, as a floor.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>w:hRule</c> names three cases and Writer honours two: <c>exact</c> is a fixed height, and
    /// <em>everything else is a floor</em> — <c>atLeast</c>, an absent rule, and <c>auto</c> alike.
    /// <c>MeasureHandler</c> opens at <c>SizeType::MIN</c> and its <c>LN_CT_Height_hRule</c> case tests
    /// only for <c>exact</c> (<c>sw/source/writerfilter/dmapper/MeasureHandler.cxx</c>:35, 70-76), so
    /// the word <c>auto</c> never reaches the layout at all and the stated <c>w:val</c> stands.
    /// </para>
    /// <para>
    /// Reading <c>auto</c> as "no height at all" was this reader's own invention and is refuted by both
    /// reference versions at once. Six rows stating <c>w:trHeight w:val="480" w:hRule="auto"</c>
    /// (<c>probes/words-row-height/pitch.py</c>): 24.2.7.2 draws them 480 twips apart and 26.2.4.2 draws
    /// them 489.6 to 740.4 apart depending on the border and the margins — its own <c>atLeast</c>
    /// figures exactly — while we drew them 241.2, which is the empty paragraph and nothing else.
    /// <b>No corpus document does it</b>: 11 230 <c>w:trHeight</c> elements across the DOCX corpus, and
    /// not one states <c>auto</c>.
    /// </para>
    /// </remarks>
    private (Length Height, bool IsExact) RowHeight(XElement? properties, List<XElement> cells)
    {
        XElement? height = Word.Child(properties, "trHeight");
        if (height is null) return (Length.Zero, false);

        string? rule = Word.Attribute(height, "hRule");

        // `w:val`, not `w:w`. A row height is a bare measurement rather than a `w:tblWidth`, so it carries
        // neither a type nor a `w:w` — and reading it with the width helper returns nothing at all, which for
        // an "at least" height is invisible (a zero floor is no floor) and for an exact one is a zero-height
        // row. That is how this was found: the bug had been silent since the heights were first read.
        Length measured =
            Word.Attribute(height, "val") is { } text
            && Word.Integer(text, out int twips)
                ? Length.FromTwips(Math.Abs(twips))
                : Length.Zero;

        return (measured, rule == "exact" || (measured > Length.Zero && MarkIsHidden(cells)));
    }

    /// <summary>
    /// True when the row's <c>w:trHeight</c> is a height rather than a floor because every cell hides
    /// its end mark and none of them holds anything.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This is the difference between a graph-paper grid and a wall of oblongs.</strong>
    /// <c>DomainMapperTableHandler::endTableGetRowProperties</c> — "we have CellHideMark on all cells,
    /// and also all cells are empty: force the row height to be exactly as specified, and not just as
    /// the minimum suggestion" (<c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>
    /// :1157-1162, with <c>lcl_hideMarks</c> at :1027 and <c>lcl_emptyRow</c> at :1085). Measured on
    /// <c>084_Printable_Graph_Paper_Template_Editable_Layout</c>, whose 48 rows declare
    /// <c>w:trHeight w:val="180"</c> — 9 pt, well under the 13.7 pt an 11 pt Calibri line asks for.
    /// Both references draw the row pitch at <b>9.00 pt</b> and we drew it at <b>14.40</b>, which is a
    /// grid 60% too tall and the <em>Title:</em> and <em>Date:</em> rules pushed off the sheet.
    /// </para>
    /// <para>
    /// <strong>A cell holding nothing but blanks counts as empty, and only below compatibility mode
    /// 15.</strong> That is a second rule feeding this one: <c>DomainMapper_Impl</c> trims trailing
    /// spaces, tabs and no-break spaces from a table cell's paragraph when
    /// <c>0 &lt; mode &lt;= 14</c> (tdf#77417, <c>DomainMapper_Impl.cxx</c>:3032-3045), so an
    /// all-blank cell in a Word 2010 file <em>is</em> empty by the time the row is measured and the
    /// same cell in a Word 2013 file is not. It is load-bearing rather than a nicety: every one of
    /// <c>084</c>'s 1728 cells holds a single no-break space, and it declares
    /// <c>compatibilityMode 14</c>. Probed across both binaries and four modes in
    /// <c>probes/words-hidemark-rowheight</c> — 24 documents, 24.2.7.2 and 26.2.4.2 agreeing on every
    /// one.
    /// </para>
    /// <para>
    /// <strong>Only the emptiness test reads that trim, not the layout.</strong> Trimming trailing
    /// blanks from every table-cell paragraph in every file below mode 15 is what LibreOffice really
    /// does, and it would reach far past this: the DOCX corpus holds 1 043 cell paragraphs ending in a
    /// blank across 61 documents below mode 15, each one a cell width and possibly a line break. That
    /// is a cascade to measure on its own round rather than a rider on this one.
    /// </para>
    /// <para>
    /// Guarded on a stated height, which LibreOffice does not need: it sets the size <em>type</em> and
    /// leaves the height to <c>w:trHeight</c>, so a row with no stated height keeps whatever default
    /// the model has. Here an exact height of zero would be a row that draws nothing, so a row stating
    /// none keeps its floor.
    /// </para>
    /// </remarks>
    /// <param name="cells">The row's <c>w:tc</c> elements.</param>
    private bool MarkIsHidden(List<XElement> cells)
    {
        if (cells.Count == 0) return false;

        foreach (XElement cell in cells)
        {
            XElement? properties = Word.Child(cell, "tcPr");

            // `lcl_hideMarks`: every cell must hide its mark, and none may be vertically merged —
            // "if anything is vertically merged, the row must not be set to fixed as Writer's layout
            // doesn't handle that well".
            if (!Word.IsOn(Word.Child(properties, "hideMark"))) return false;
            if (Word.Child(properties, "vMerge") is not null) return false;
            if (!IsEmptyCell(cell)) return false;
        }

        return true;
    }

    /// <summary>Whether a cell holds no text at all, in the sense the row-height rule needs.</summary>
    /// <remarks>
    /// One paragraph, no nested table, and nothing in it but <c>w:t</c> — a tab, a break, a picture or
    /// a field all put something in the cell's text range and make <c>lcl_emptyRow</c> false. The text
    /// itself must be empty, or, below compatibility mode 15, blank: see <see cref="MarkIsHidden"/>.
    /// </remarks>
    private bool IsEmptyCell(XElement cell)
    {
        // `0 < nMode && nMode <= 14`, and the lower bound is not decoration: a file stating no
        // compatibility mode at all leaves ours at −1, and both references treat such a file the
        // modern way — probed, `f-None-hide-nbsp` draws 13.92 pt where `f-14-hide-nbsp` draws 9.12.
        bool trims = _compatibilityMode is > 0 and <= 14;
        int paragraphs = 0;

        foreach (XElement child in cell.Elements())
        {
            if (Word.Is(child, "tcPr")) continue;
            if (!Word.Is(child, "p")) return false;
            if (++paragraphs > 1) return false;

            foreach (XElement inside in child.Descendants())
            {
                if (!Word.Is(inside, inside.Name.LocalName)) return false;

                switch (inside.Name.LocalName)
                {
                    case "pPr" or "rPr" or "r" or "proofErr" or "bookmarkStart" or "bookmarkEnd"
                         or "lastRenderedPageBreak":
                        continue;

                    case "t":
                        string text = inside.Value;
                        if (text.Length == 0) continue;
                        if (!trims) return false;
                        foreach (char character in text)
                            if (character is not (' ' or '\t' or '\u00a0')) return false;
                        continue;

                    default:
                        // A `w:pPr`'s or `w:rPr`'s own children are formatting, not content.
                        if (inside.Ancestors().Any(
                                ancestor => ancestor.Name.LocalName is "pPr" or "rPr"))
                        {
                            continue;
                        }

                        return false;
                }
            }
        }

        return paragraphs == 1;
    }

    /// <summary>
    /// A cell's padding, with each side falling back to the table's separately.
    /// </summary>
    /// <remarks>
    /// The per-side fallback is the whole point. LibreOffice's export writes a <c>w:tcMar</c> containing
    /// only the side that differs from the table's, so treating the element's presence as "the cell states
    /// all four" zeroes the other three — which moves the text up against the cell's top border and, worse,
    /// widens the space its text has to break in.
    /// </remarks>
    private static CellPadding Padding(XElement? margins, CellPadding fallback)
    {
        if (margins is null) return fallback;

        return new CellPadding(
            Side(margins, "start", "left") ?? fallback.Left,
            Side(margins, "end", "right") ?? fallback.Right,
            Side(margins, "top", null) ?? fallback.Top,
            Side(margins, "bottom", null) ?? fallback.Bottom);
    }

    /// <summary>
    /// One side of a margin block, under either of the two names OOXML has for it.
    /// </summary>
    /// <remarks>
    /// <c>w:start</c> and <c>w:end</c> are the logical names, which the transitional schema spells
    /// <c>w:left</c> and <c>w:right</c>. Both appear in the wild — LibreOffice writes the logical pair,
    /// Word the physical — and neither is a synonym in a right-to-left table, where start is the right.
    /// Bidirectional tables are not laid out yet, so taking them as equivalent is exactly as wrong as the
    /// rest of the reader already is about direction, and no more.
    /// </remarks>
    private static Length? Side(XElement margins, string logical, string? physical)
        => Twips(Word.Child(margins, logical))
           ?? (physical is null ? null : Twips(Word.Child(margins, physical)));

    /// <summary>
    /// The cell padding a table style states, or the fallback when neither it nor its parents do.
    /// </summary>
    /// <remarks>
    /// Applied per side and innermost-first, the same way the borders are: a style stating only a top
    /// margin inherits the other three from its <c>w:basedOn</c> chain and then from Word's default,
    /// rather than replacing all four.
    /// </remarks>
    private CellPadding StyleCellPadding(string? styleId, CellPadding fallback)
    {
        CellPadding padding = fallback;

        // Reversed so the outermost ancestor is applied first and the style's own last, which leaves the
        // innermost statement of each side standing.
        List<XElement> chain = _styles.TableStyleTableProperties(styleId);
        for (int i = chain.Count - 1; i >= 0; i--)
        {
            padding = Padding(Word.Child(chain[i], "tblCellMar"), padding);
        }

        return padding;
    }

    /// <summary>
    /// Which way a cell's text runs — <c>w:textDirection</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Six values collapsing to three, reproducing LibreOffice's own mapping in
    /// <c>DomainMapperTableManager.cxx</c>:325-350 rather than the specification's reading of them.
    /// <c>tbRlV</c> is folded onto <c>tbRl</c> there and <c>tbLrV</c> is dropped with the comment "we
    /// can't handle these"; both were re-measured against the installed 26.2.4.2 and both hold —
    /// <c>tbRlV</c> renders identically to <c>tbRl</c>, and <c>tbLrV</c> identically to no attribute at
    /// all. Following the specification instead would turn text the reference leaves upright.
    /// </para>
    /// <para>
    /// The default when the attribute is absent or unrecognised is upright, which is also what
    /// <c>w:val="lrTb"</c> asks for explicitly.
    /// </para>
    /// </remarks>
    private static CellTextDirection TextDirection(XElement? properties)
        => Word.Attribute(Word.Child(properties, "textDirection"), "val") switch
        {
            "btLr" => CellTextDirection.BottomToTopLeftToRight,
            "tbRl" or "tbRlV" => CellTextDirection.TopToBottomRightToLeft,
            _ => CellTextDirection.LeftToRight,
        };

    private static VerticalTextAlignment VerticalAlignment(XElement? properties)
        => Word.Attribute(Word.Child(properties, "vAlign"), "val") switch
        {
            "center" => VerticalTextAlignment.Middle,
            "bottom" => VerticalTextAlignment.Bottom,
            _ => VerticalTextAlignment.Top,
        };

    /// <summary>
    /// The colour behind a cell's text, or null when it has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>w:shd</c>'s <c>w:fill</c>, which is the colour, rather than its <c>w:color</c>, which is the pattern's
    /// foreground and only shows through a <c>w:val</c> that is not <c>clear</c> or <c>nil</c>. Word's
    /// <c>auto</c> means "let whatever is behind show", which is not a colour and so is null. The patterns
    /// themselves — <c>pct25</c> and its family — are not modelled: their fill colour is drawn solid, which is
    /// the right colour at the wrong density and much closer than nothing.
    /// </para>
    /// <para>
    /// The fill is themed through <c>w:themeFill</c> rather than through <c>w:themeColor</c>, which on this
    /// one element means the <em>pattern's foreground</em> instead — the only place in WordprocessingML where
    /// two themed colours sit on one element, and the reason <see cref="WordThemeColour"/>'s six-argument
    /// <c>Read</c> takes the four attribute names as parameters. Reading
    /// the fill from <c>w:themeColor</c> gives a plausible colour from the wrong slot on every shaded cell of
    /// any table whose shading also states a pattern.
    /// </para>
    /// </remarks>
    private Colour? Shading(XElement? properties) => ShadeColour(Word.Child(properties, "shd"));

    /// <summary>The colour a <c>w:shd</c> fills with, or null when it fills with nothing.</summary>
    /// <remarks>
    /// <para>
    /// Separate from <see cref="Shading"/> because a paragraph's shading is not simply the child of its own
    /// <c>w:pPr</c>: it can come from any layer of the style chain, and only the resolver knows which layer
    /// won. Both reach the same reading of the element once it has been found.
    /// </para>
    /// <para>
    /// <strong><c>w:shd</c> is a pattern and not a fill, and reading only its <c>w:fill</c> is why three
    /// black rectangles were missing from <c>AFS-050-004-F2_0i</c> page 2.</strong>
    /// <c>CellColorHandler::getProperties</c> (<c>sw/source/writerfilter/dmapper</c>) turns <c>w:val</c>
    /// into a weight out of a thousand and blends <c>w:color</c> over <c>w:fill</c> at it; only the
    /// zero-weight case — <c>clear</c>, and anything the table does not name — is the fill on its own.
    /// So <c>&lt;w:shd w:val="solid" w:color="auto" w:fill="auto"/&gt;</c> is a <em>black</em> cell, and
    /// that is the ordinary way a Word document writes a reversed-out header row.
    /// </para>
    /// <para>
    /// The two <c>auto</c>s are not the same value and that asymmetry is the file format's:
    /// <c>w:color="auto"</c> is black and <c>w:fill="auto"</c> is white
    /// (<c>CellColorHandler::lcl_attribute</c>). Measured on 26.2.4.2 over eight patterns —
    /// <c>probes/words-r59/autocolour.py</c> — which reproduce exactly: <c>pct50</c> auto over auto is
    /// <c>#7F7F7F</c>, <c>pct25</c> is <c>#BFBFBF</c>, <c>pct75</c> is <c>#3F3F3F</c>, every striped and
    /// crossed value is 333 and comes out <c>#AAAAAA</c>, and <c>pct50</c> red over blue is
    /// <c>#7F007F</c>. The division is integer and truncating, which is where those exact bytes come
    /// from.
    /// </para>
    /// <para>
    /// <strong><c>w:val="nil"</c> is not "no fill".</strong> It is absent from that table, so it takes
    /// the zero-weight branch and paints its <c>w:fill</c> like <c>clear</c>: the reference fills
    /// <c>nil</c> with <c>w:fill="000000"</c> black and reverses its text out. Returning null for it —
    /// which is what stood here — is the one reading the probe refutes outright rather than refines.
    /// </para>
    /// </remarks>
    private Colour? ShadeColour(XElement? shade)
    {
        if (shade is null) return null;

        Colour? fill = WordThemeColour.Read(
            shade, _theme, "fill", "themeFill", "themeFillTint", "themeFillShade");

        int weight = ShadingWeight(Word.Attribute(shade, "val"));
        if (weight <= 0) return fill;

        Colour foreground = WordThemeColour.Read(
            shade, _theme, "color", "themeColor", "themeTint", "themeShade") ?? Colour.Black;
        Colour background = fill ?? Colour.White;

        return new Colour(
            Mix(foreground.R, background.R, weight),
            Mix(foreground.G, background.G, weight),
            Mix(foreground.B, background.B, weight));

        static byte Mix(byte foreground, byte background, int weight)
            => (byte)(((foreground * weight) + (background * (1000 - weight))) / 1000);
    }

    /// <summary>
    /// How much of a <c>w:shd</c>'s foreground shows through, out of a thousand.
    /// </summary>
    /// <remarks>
    /// <c>CellColorHandler::getProperties</c>'s own table, value for value. The percentages are not
    /// uniformly ten times their name — <c>pct12</c> is 125, <c>pct15</c> 150, <c>pct37</c> 375,
    /// <c>pct62</c> 625 and <c>pct87</c> 875, because those five are Word's names for eighths — and
    /// every striped or crossed pattern, thin or not, is a flat 333 whatever its geometry, since
    /// Writer has no pattern brush to draw it with. Anything the table does not name is zero, which is
    /// the fill on its own.
    /// </remarks>
    private static int ShadingWeight(string? pattern) => pattern switch
    {
        null or "clear" or "nil" => 0,
        "solid" => 1000,
        "pct12" => 125,
        "pct15" => 150,
        "pct37" => 375,
        "pct62" => 625,
        "pct87" => 875,
        ['p', 'c', 't', .. var digits] when int.TryParse(digits, out int percent)
            && percent is > 0 and <= 100 => percent * 10,
        "horzStripe" or "vertStripe" or "reverseDiagStripe" or "diagStripe" or "horzCross"
            or "diagCross" or "thinHorzStripe" or "thinVertStripe" or "thinReverseDiagStripe"
            or "thinDiagStripe" or "thinHorzCross" or "thinDiagCross" => 333,
        _ => 0,
    };

    /// <summary>
    /// A cell's own four borders — <c>w:tcBorders</c> and nothing else — with null for a side it
    /// leaves unstated.
    /// </summary>
    /// <remarks>
    /// The table's are not folded in here any more, because which of the table's six sides reaches a cell
    /// depends on where that cell sits in the grid: see <see cref="ApplyGridBorders"/>. A cell's own
    /// <c>w:insideH</c>/<c>w:insideV</c> are deliberately not read either — <c>DomainMapperTableHandler</c>
    /// erases them before it does anything else, "meaningless without a context (tdf#82177)"
    /// (<c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>:814).
    /// </remarks>
    private CellBorderSet OwnBorders(XElement? cellProperties)
    {
        XElement? cell = Word.Child(cellProperties, "tcBorders");

        // `w:start`/`w:end` first and `w:left`/`w:right` as the fallback. OOXML has both — the logical pair is
        // the ISO spelling and the physical pair the legacy one — and LibreOffice's own export writes the
        // *logical* names, so a reader that knew only `w:left` finds no vertical borders at all and draws five
        // strokes where the reference draws nine. The two only differ in a right-to-left table, which nothing
        // here lays out yet.
        return new CellBorderSet(
            Border(cell, "start", "left"),
            Border(cell, "end", "right"),
            Border(cell, "top"),
            Border(cell, "bottom"));
    }

    /// <summary>
    /// The table's six borders: its own <c>w:tblBorders</c> over its style's, per side.
    /// </summary>
    /// <remarks>
    /// <c>DomainMapperTableHandler::endTableGetTableStyle</c> inserts the style's properties and then the
    /// table's own, so the table wins per property rather than wholesale
    /// (<c>DomainMapperTableHandler.cxx</c>:438-439). Reading the style's is what makes <c>Table Grid</c>
    /// draw anything at all: it states nothing but a <c>w:tblBorders</c>, and a table using it states no
    /// borders of its own, so a reader that consulted only <c>w:tblPr/w:tblBorders</c> drew no line
    /// anywhere in the commonest table Word writes.
    /// </remarks>
    private TableBorderSet TableBorders(XElement? tableProperties, string? styleId)
    {
        List<XElement?> layers = [Word.Child(tableProperties, "tblBorders")];
        foreach (XElement style in _styles.TableStyleTableProperties(styleId))
        {
            layers.Add(Word.Child(style, "tblBorders"));
        }

        return new TableBorderSet(
            Side("start", "left"),
            Side("end", "right"),
            Side("top", null),
            Side("bottom", null),
            Side("insideH", null),
            Side("insideV", null));

        TableBorder? Side(string side, string? legacySide)
        {
            foreach (XElement? layer in layers)
            {
                if (Border(layer, side, legacySide) is { } found) return found;
            }

            return null;
        }
    }

    /// <summary>
    /// One border from a <c>w:tblBorders</c> or <c>w:tcBorders</c> block, or null when it states none.
    /// </summary>
    /// <remarks>
    /// Null and a zero-width border are different answers and the difference decides the merge: a stated
    /// <c>w:val="none"</c> is a border of no width that <em>beats</em> whatever the layer below would have
    /// given, which is how a cell switches one edge of its table's grid off. LibreOffice keeps the same
    /// distinction by inserting a zero-width <c>BorderLine2</c> for <c>none</c> and then merging with
    /// <c>Insert(..., false)</c>, which does not overwrite.
    /// <para>
    /// <c>w:sz</c> is in <em>eighths</em> of a point, which is the one unit in OOXML that is neither twips nor
    /// half-points — reading it as either gives a border eight or four times too thick.
    /// </para>
    /// <para>
    /// The colour is themed the ordinary way — <c>w:color</c> caching what <c>w:themeColor</c> with
    /// <c>w:themeTint</c>/<c>w:themeShade</c> resolves to — so it goes through the same reader a
    /// <c>w:color</c> does. Black remains the fallback, because a border whose colour resolves to nothing is
    /// still a border.
    /// </para>
    /// </remarks>
    private TableBorder? Border(XElement? borders, string side, string? legacySide = null)
    {
        XElement? stated =
            Word.Child(borders, side)
            ?? (legacySide is null ? null : Word.Child(borders, legacySide));

        if (stated is null) return null;

        string? val = Word.Attribute(stated, "val");

        Length stateWidth =
            Word.Integer(Word.Attribute(stated, "sz"), out int eighths) && eighths > 0
                ? Length.FromPoints(eighths / 8.0)
                : HairlineBorder;

        // An art border draws nothing at all, the same answer `none` and `nil` give — see
        // `BorderRules.WordStyleOf`, and `BorderRules` for why the width comes back changed.
        if (val is null or "none" or "nil"
            || BorderRules.FromWord(BorderRules.WordStyleOf(val), stateWidth) is not { } rule)
        {
            return default(TableBorder);
        }

        Colour colour =
            WordThemeColour.Read(stated, _theme, "color", "themeColor", "themeTint", "themeShade")
            ?? Colour.Black;

        return new TableBorder(rule.Width, colour, rule.Line);
    }

    /// <summary>
    /// Gives every cell the borders its position in the grid earns it, once all the rows are read.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>lcl_computeCellBorders</c> (<c>DomainMapperTableHandler.cxx</c>:126), which is where
    /// <c>w:insideH</c> and <c>w:insideV</c> finally land: the interior lines are stated once for the whole
    /// table and become a <em>right</em> border on every cell but the last in its row, a <em>left</em> on
    /// every cell but the first, and a top and bottom on every row but the outermost edges. The cell's own
    /// <c>w:tcBorders</c> beats all of it, which is why this fills in only what is still null.
    /// </para>
    /// <para>
    /// A table too small for an interior line does not get one. LibreOffice erases <c>insideH</c> for a
    /// table a single row tall and <c>insideV</c> for one a single column wide, and both for a table of
    /// one cell (<c>DomainMapperTableHandler.cxx</c>:915-940) — without which the table's own edges would
    /// be drawn at the interior width instead of the outline's. Measured on a fixture stating a 3 pt
    /// interior against a 0.5 pt outline: LibreOffice's PDF of the single-row table holds four strokes at
    /// 0.5 and two at 3, the two being its <em>vertical</em> interiors, and the single-column table's is
    /// the mirror of that.
    /// </para>
    /// <para>
    /// Only the horizontal half of that is implemented, because the vertical half provably cannot bite
    /// here: a lone cell in a row is both the first and the last, and the two lines below that place a
    /// vertical interior are guarded on it being neither. LibreOffice needs its erasure because it hands
    /// the interior lines to the <em>table</em> as a <c>TableBorder</c> structure rather than to the
    /// cells.
    /// </para>
    /// <para>
    /// Run before the table is built rather than during the row walk, because three of the rules need
    /// facts no row knows on its own: whether it is the last, whether it is the only one, and how far a
    /// vertical merge starting in it reaches.
    /// </para>
    /// </remarks>
    private static void ApplyGridBorders(List<PendingRow> rows, TableBorderSet table)
    {
        int rowCount = rows.Count;

        for (int row = 0; row < rowCount; row++)
        {
            List<PendingCell> cells = rows[row].Cells;
            int lastCell = cells.Count - 1;
            bool isEndRow = row == rowCount - 1;

            TableBorder? horizontal = rowCount <= 1 ? null : table.InsideH;
            TableBorder? vertical = table.InsideV;

            for (int index = 0; index <= lastCell; index++)
            {
                PendingCell cell = cells[index];
                CellBorderSet own = cell.Own;

                bool isStartCol = index == 0;
                bool isEndCol = index == lastCell;

                // "Checking if current cell is vertically merged with all the other cells below to the
                // bottom", which is what earns the merge's first cell the table's bottom border.
                int continuations = cell.Merge == VerticalMerge.Restart
                    ? Continuations(rows, row, cell.Definition.Column)
                    : 0;
                bool mergedToBottom =
                    cell.Merge == VerticalMerge.Restart && row + continuations == rowCount - 1;

                TableBorder? left = own.Left;
                TableBorder? right = own.Right;
                TableBorder? top = own.Top;

                // "Only consider the bottom border setting from the last merged cell": a merge's own
                // bottom edge is the one its last continuation states, not the one its first does.
                TableBorder? bottom = continuations > 0
                    ? BottomOfMerge(rows, row + continuations, cell.Definition.Column)
                    : own.Bottom;

                if (isStartCol) left ??= table.Left;
                if (isEndCol) right ??= table.Right;

                if (vertical is not null)
                {
                    if (!isEndCol) right ??= vertical;
                    if (!isStartCol) left ??= vertical;
                }

                if (row == 0)
                {
                    top ??= table.Top;
                    if (horizontal is not null && !mergedToBottom) bottom ??= horizontal;
                }

                if (mergedToBottom) bottom ??= table.Bottom;

                if (isEndRow)
                {
                    bottom ??= table.Bottom;
                    if (horizontal is not null) top ??= horizontal;
                }

                if (row > 0 && !isEndRow && horizontal is not null)
                {
                    top ??= horizontal;
                    bottom ??= horizontal;
                }

                CellBorders borders = new(
                    left ?? default, right ?? default, top ?? default, bottom ?? default);

                cells[index] = cell with
                {
                    Definition = cell.Definition with
                    {
                        Borders = borders,
                        Padding = KeptOffTheBorder(cell.Definition.Padding, borders),
                    },
                };
            }
        }
    }

    /// <summary>
    /// Raises a cell's left and right margins so its text cannot sit under its own border.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word's cell border straddles the cell edge, so half of it lies inside the cell — and Word will
    /// not let the text run under that half however small the margin says. writerfilter reproduces it
    /// at import, in <c>lcl_adjustBorderDistance</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>:318–348), whose comment
    /// states the rule as Word's own:
    /// </para>
    /// <code>
    /// pad_l = max(bll/2, cml)
    /// pad_r = max(pad_l + blr/2, cml + cmr) - pad_l
    /// </code>
    /// <para>
    /// So the right margin is what is left of the wider of "clear of the right border" and "both
    /// declared margins" once the left one is taken — which is why a document with a thick border, no
    /// left margin and a 2 mm right margin gets no gap on the right at all.
    /// </para>
    /// <para>
    /// It applies to this reader and not to <see cref="Ww8.Ww8DocumentReader"/>:
    /// <c>WW8TabDesc::SetTabBorders</c> (<c>sw/source/filter/ww8/ww8par2.cxx</c>:3020–3042) sets a
    /// <c>.doc</c> cell's distance straight from <c>sprmTCellPadding</c> or the band's half-gap with
    /// no such floor. It is an import adjustment rather than a layout rule, and the layout charges
    /// only what it is given: with collapsing borders — which every Word table has —
    /// <c>SwCellFrame::Format</c> insets by <c>rBoxItem.GetDistance()</c> alone and never by the
    /// border width. Measured across 21 margin and border combinations by
    /// <c>dotnet/probes/cell-border-inset.py</c>, which also shows the ODF separating-border table
    /// that <em>does</em> charge the whole border, so the two cannot be confused again.
    /// </para>
    /// <para>
    /// It reduces to the declared margins whenever each is at least half its border, which is nearly
    /// always: Word's default margin is 108 twips and half a hairline border is 5.
    /// </para>
    /// </remarks>
    private static CellPadding KeptOffTheBorder(CellPadding padding, CellBorders borders)
    {
        Length left = Length.Max(padding.Left, borders.Left.Width / 2);
        Length right = Length.Max(left + (borders.Right.Width / 2), padding.Left + padding.Right) - left;

        return left == padding.Left && right == padding.Right
            ? padding
            : padding with { Left = left, Right = right };
    }

    /// <summary>The bottom border stated by the cell a vertical merge ends in, if it states one.</summary>
    private static TableBorder? BottomOfMerge(List<PendingRow> rows, int row, int column)
    {
        foreach (PendingCell cell in rows[row].Cells)
        {
            if (cell.Definition.Column == column) return cell.Own.Bottom;
        }

        return null;
    }

    /// <summary>The width a border with no usable <c>w:sz</c> is drawn at: half a point.</summary>
    private static readonly Length HairlineBorder = Length.FromPoints(0.5);

    /// <summary>What a cell's <c>w:vMerge</c> says about the vertical merge it is part of.</summary>
    /// <remarks>
    /// A bare <c>w:vMerge</c> with no <c>w:val</c> means <c>continue</c>, which is the one value the schema
    /// leaves implicit — and the common one, since a merge has one restart and many continuations.
    /// </remarks>
    private static VerticalMerge Merge(XElement? properties)
        => Word.Child(properties, "vMerge") switch
        {
            null => VerticalMerge.None,
            { } merge => Word.Attribute(merge, "val") switch
            {
                "restart" => VerticalMerge.Restart,
                "cont" or "continue" or null or "" => VerticalMerge.Continue,
                _ => VerticalMerge.None,
            },
        };

    /// <summary>
    /// Turns the merge states into row spans, and drops the continuation cells.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A restart's span is one plus however many of the rows below it hold a continuation at the same
    /// column. The run has to be consecutive: a gap means the merge ended and a later continuation belongs
    /// to a different one — or to nothing at all, which real documents also contain.
    /// </para>
    /// <para>
    /// The continuations themselves are dropped rather than emitted with a zero span, because nothing
    /// downstream needs a placeholder for a cell that is not drawn: the layout engine finds a cell by the
    /// column it states, so an absent cell simply leaves the column to the merge above it.
    /// </para>
    /// <para>
    /// One number does need them, and it is the row's <em>height</em>: a covered cell's stated top and bottom
    /// rules are charged to the boundary bands and to a <c>w:trHeight</c> floor although nothing is drawn for
    /// the top one. They leave here as <see cref="PageTableRow.CoveredTopRule"/> and
    /// <see cref="PageTableRow.CoveredBottomRule"/>, whose remarks carry the measurement.
    /// </para>
    /// </remarks>
    private static List<PageTableRow> Resolved(List<PendingRow> rows)
    {
        List<PageTableRow> resolved = new(rows.Count);

        for (int row = 0; row < rows.Count; row++)
        {
            List<PageTableCell> cells = [];
            Length coveredTop = Length.Zero;
            Length coveredBottom = Length.Zero;

            foreach (PendingCell cell in rows[row].Cells)
            {
                if (cell.Merge == VerticalMerge.Continue)
                {
                    // Dropped from the row's cells, because nothing draws it -- but its stated rules are
                    // still charged to the row's height, which is `PageTableRow.CoveredTopRule`'s remark.
                    coveredTop = Length.Max(coveredTop, cell.Definition.Borders.Top.Width);
                    coveredBottom = Length.Max(coveredBottom, cell.Definition.Borders.Bottom.Width);
                    continue;
                }

                int span = cell.Merge == VerticalMerge.Restart
                    ? 1 + Continuations(rows, row, cell.Definition.Column)
                    : 1;

                cells.Add(cell.Definition with { RowSpan = span });
            }

            resolved.Add(new PageTableRow
            {
                Cells = cells,
                CoveredTopRule = coveredTop,
                CoveredBottomRule = coveredBottom,
                IsHeader = rows[row].IsHeading,
                MinHeight = rows[row].Height.Height,
                HasExactHeight = rows[row].Height.IsExact,
                CanSplit = rows[row].CanSplit,
            });
        }

        return resolved;
    }

    /// <summary>How many consecutive rows below this one continue a merge at the same column.</summary>
    private static int Continuations(List<PendingRow> rows, int from, int column)
    {
        int count = 0;

        for (int row = from + 1; row < rows.Count; row++)
        {
            bool continues = false;
            foreach (PendingCell cell in rows[row].Cells)
            {
                if (cell.Definition.Column != column) continue;

                continues = cell.Merge == VerticalMerge.Continue;
                break;
            }

            if (!continues) break;

            count++;
        }

        return count;
    }

    /// <summary>
    /// How many rows at the top are headings.
    /// </summary>
    /// <remarks>
    /// A run from the top, matching <c>SwTable::GetRowsToRepeat</c>: <c>w:tblHeader</c> on a row further
    /// down does not make the rows above it headings, and Word only repeats a leading run either.
    /// </remarks>
    private static int HeadingRows(List<PendingRow> rows)
    {
        int count = 0;
        while (count < rows.Count && rows[count].IsHeading) count++;
        return count;
    }

    /// <summary>A <c>w:w</c> measure in twips, or null when the element states none.</summary>
    /// <remarks>
    /// Only <c>dxa</c> and the absent type are twips. A percentage or an <c>auto</c> width needs the page,
    /// which the reader does not have — so it reads as unstated rather than as a number in the wrong unit,
    /// which would be a column several times too wide.
    /// </remarks>
    private static Length? Twips(XElement? element)
    {
        if (element is null) return null;

        string? type = Word.Attribute(element, "type");
        if (type is not (null or "" or "dxa")) return null;

        return Word.Attribute(element, "w") is { } text
               && Word.Integer(text, out int twips)
            ? Length.FromTwips(twips)
            : null;
    }

    /// <summary>
    /// A <c>w:tblW</c> stated as a percentage of the area the table sits in, or null when it is not one.
    /// </summary>
    /// <remarks>
    /// The unit is fiftieths of a percent, so <c>5000</c> is 100%, and the result is clamped there:
    /// <c>nPercent = pMeasureHandler-&gt;getValue() / 50; if (nPercent &gt; 100) nPercent = 100;</c>
    /// (<c>DomainMapperTableManager.cxx</c>:193). A file writing <c>50%</c> instead of <c>2500</c> is
    /// legal under <c>ST_MeasurementOrPercent</c> and read as the percentage it says. Zero is not a
    /// width at all — <c>w:tblW w:w="0" w:type="pct"</c> is how a table says it has none — and a
    /// negative one is nonsense, so both read as absent.
    /// </remarks>
    private static int? Percentage(XElement? element)
    {
        if (element is null || Word.Attribute(element, "type") is not "pct") return null;
        if (Word.Attribute(element, "w") is not { } text) return null;

        string trimmed = text.Trim();
        bool literal = trimmed.EndsWith('%');
        if (literal) trimmed = trimmed[..^1];

        if (!Word.Integer(trimmed, out int stated)) return null;

        int percent = literal ? stated : stated / 50;
        return percent <= 0 ? null : Math.Min(percent, 100);
    }

    private static int? Number(XElement? element)
        => Word.Attribute(element, "val") is { } text
           && Word.Integer(text, out int value)
            ? value
            : null;

    /// <summary>
    /// True while the walk is inside a cell covered by a vertical merge above it, whose paragraphs are
    /// read but never drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="Resolved"/> drops a <c>w:vMerge</c> continuation cell outright, so nothing it holds
    /// reaches the page — but its paragraphs have already been read by then, and reading a numbered
    /// paragraph <em>advances that list's counter</em>. The counter is the only thing that escapes a
    /// cell that is not drawn, and it escapes into the numbers of every list item after it.
    /// </para>
    /// <para>
    /// LibreOffice states the same rule from the other end: setting a cell's <c>VerticalMerge</c>
    /// property walks every text node in the cell and clears its counted-in-list flag
    /// (<c>sw/source/core/unocore/unotbl.cxx</c>:978-990, <em>"Hack to allow clearing of numbering from
    /// the paragraphs in the merged cells"</em>), so a numbered paragraph in a covered cell neither
    /// shows a number nor advances the count.
    /// </para>
    /// <para>
    /// Measured on <c>B11. TE.CAO.00129 Experience logbook.docx</c>, whose ID column carries sixteen
    /// paragraphs at <c>w:numId="16"</c>, three of them empty and in <c>w:vMerge</c> continuation cells
    /// (table 2, rows 8, 17 and 18). The reference numbers the other thirteen 1 to 13; without this
    /// the three covered ones consume 8, 10 and 11 and the visible column reads 1–7, 9, 12–16. The same
    /// three-line shape mis-numbers <c>FO.FCTOA_.000129</c>'s activity sections 3.1, 3.3, 3.4, 3.6,
    /// 3.12 against the reference's 3.1 to 3.5.
    /// </para>
    /// <para>
    /// Only <em>continuation</em> cells, not the restart that begins the merge: the reference numbers
    /// a heading in a <c>w:vMerge w:val="restart"</c> cell normally — <c>FO.FCTOA_.000129</c> prints
    /// "2.1.1 Name and Address" and "3.1 Audit of Management …" from restart cells — because that cell
    /// is the one that is drawn.
    /// </para>
    /// </remarks>
    private bool _inCoveredCell;

    /// <summary>Which part of a vertical merge a cell is.</summary>
    private enum VerticalMerge
    {
        /// <summary>Not merged vertically at all.</summary>
        None,

        /// <summary>The top of a merge, whose span is counted from the rows below.</summary>
        Restart,

        /// <summary>A row covered by a merge above it, which is not drawn.</summary>
        Continue,
    }

    /// <summary>A cell before its row span is known.</summary>
    /// <param name="Definition">The cell as read, whose borders are filled in by
    /// <see cref="ApplyGridBorders"/> once the grid is known.</param>
    /// <param name="Merge">What its <c>w:vMerge</c> said.</param>
    /// <param name="Own">Its own <c>w:tcBorders</c>, with null for each side it left unstated.</param>
    private readonly record struct PendingCell(
        PageTableCell Definition, VerticalMerge Merge, CellBorderSet Own);

    /// <summary>
    /// The four borders a cell states for itself, each null when it states none.
    /// </summary>
    /// <remarks>
    /// Distinct from <see cref="CellBorders"/>, whose sides are always answers: this is the question,
    /// and a null side is what lets the table's own border through.
    /// </remarks>
    private readonly record struct CellBorderSet(
        TableBorder? Left, TableBorder? Right, TableBorder? Top, TableBorder? Bottom);

    /// <summary>
    /// The six borders a table states, its own over its style's, each null when neither states one.
    /// </summary>
    private readonly record struct TableBorderSet(
        TableBorder? Left,
        TableBorder? Right,
        TableBorder? Top,
        TableBorder? Bottom,
        TableBorder? InsideH,
        TableBorder? InsideV);

    /// <summary>A row before its cells' row spans are known.</summary>
    private readonly record struct PendingRow(
        List<PendingCell> Cells,
        bool IsHeading,
        (Length Height, bool IsExact) Height,
        bool CanSplit = true);
}

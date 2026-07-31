using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ooxml;

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

        List<Length> columns = Columns(element);
        if (columns.Count == 0) return null;

        CellPadding tablePadding = Padding(
            Word.Child(properties, "tblCellMar"), DefaultCellPadding);

        List<PendingRow> rows = [];
        ReadRows(element, rows, tablePadding, depth: 0);
        if (rows.Count == 0) return null;

        return new PageTable
        {
            SectionIndex = _sectionIndex,
            ColumnWidths = columns,
            Rows = Resolved(rows),
            HeaderRowCount = HeadingRows(rows),
            LeftIndent = Twips(Word.Child(properties, "tblInd")) ?? Length.Zero,
        };
    }

    /// <summary>The grid's column widths, in order.</summary>
    /// <remarks>
    /// From <c>w:tblGrid</c> alone. A cell's own <c>w:tcW</c> is not consulted: it is advisory, disagrees
    /// with the grid in real documents, and Word itself lays a fixed table out from the grid — a reader
    /// preferring the cell's width would place two cells of one row at different edges.
    /// </remarks>
    private static List<Length> Columns(XElement table)
    {
        List<Length> widths = [];

        foreach (XElement column in Word.Children(Word.Child(table, "tblGrid"), "gridCol"))
        {
            if (widths.Count >= PageTable.MaxColumns) break;

            widths.Add(Twips(column) ?? Length.Zero);
        }

        return widths;
    }

    /// <summary>Reads the rows, following the change-tracking wrappers a row can sit inside.</summary>
    private void ReadRows(
        XElement element, List<PendingRow> rows, CellPadding tablePadding, int depth)
    {
        if (depth > 8) return;

        foreach (XElement child in element.Elements())
        {
            if (rows.Count >= PageTable.MaxRows) return;

            if (Word.Is(child, "tr"))
            {
                rows.Add(Row(child, tablePadding));
                continue;
            }

            // A row can be wrapped by a tracked insertion or a content control. Its cells are the table's
            // either way — a walk that stopped here would lose the row rather than the wrapper.
            if (Word.Is(child, "sdt") || Word.Is(child, "sdtContent")
                || Word.Is(child, "customXml") || Word.Is(child, "ins"))
            {
                ReadRows(child, rows, tablePadding, depth + 1);
            }
        }
    }

    private PendingRow Row(XElement element, CellPadding tablePadding)
    {
        XElement? properties = Word.Child(element, "trPr");
        List<PendingCell> cells = [];
        int column = 0;

        foreach (XElement child in Word.Children(element, "tc"))
        {
            if (column >= PageTable.MaxColumns) break;

            XElement? cellProperties = Word.Child(child, "tcPr");
            int span = Math.Max(1, Number(Word.Child(cellProperties, "gridSpan")) ?? 1);

            cells.Add(new PendingCell(
                new PageTableCell
                {
                    Blocks = ReadCell(child),
                    Column = column,
                    ColumnSpan = span,
                    Padding = Padding(Word.Child(cellProperties, "tcMar"), tablePadding),
                    VerticalAlignment = VerticalAlignment(cellProperties),
                },
                Merge(cellProperties)));

            // By the span, because DOCX writes no placeholder for a swallowed column.
            column += span;
        }

        return new PendingRow(
            cells,
            IsHeading: Word.IsOn(Word.Child(properties, "tblHeader"))
                       || Word.Child(properties, "tblHeader") is not null,
            RowHeight(properties));
    }

    /// <summary>
    /// A row's declared height, as a floor.
    /// </summary>
    /// <remarks>
    /// <c>w:hRule</c> distinguishes three cases and only two are honoured here: <c>atLeast</c> and the
    /// absent-rule default are floors, and <c>auto</c> states no height at all. <c>exact</c> is a real
    /// height that clips its content, which is not modelled — such a row gets the taller of the two
    /// instead, which is wrong in the direction of showing the text rather than hiding it.
    /// </remarks>
    private static (Length Height, bool IsExact) RowHeight(XElement? properties)
    {
        XElement? height = Word.Child(properties, "trHeight");
        if (height is null) return (Length.Zero, false);

        string? rule = Word.Attribute(height, "hRule");
        if (rule == "auto") return (Length.Zero, false);

        // `w:val`, not `w:w`. A row height is a bare measurement rather than a `w:tblWidth`, so it carries
        // neither a type nor a `w:w` — and reading it with the width helper returns nothing at all, which for
        // an "at least" height is invisible (a zero floor is no floor) and for an exact one is a zero-height
        // row. That is how this was found: the bug had been silent since the heights were first read.
        Length measured =
            Word.Attribute(height, "val") is { } text
            && int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out int twips)
                ? Length.FromTwips(Math.Abs(twips))
                : Length.Zero;

        return (measured, rule == "exact");
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

    private static CellVerticalAlignment VerticalAlignment(XElement? properties)
        => Word.Attribute(Word.Child(properties, "vAlign"), "val") switch
        {
            "center" => CellVerticalAlignment.Middle,
            "bottom" => CellVerticalAlignment.Bottom,
            _ => CellVerticalAlignment.Top,
        };

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
    /// </remarks>
    private static List<PageTableRow> Resolved(List<PendingRow> rows)
    {
        List<PageTableRow> resolved = new(rows.Count);

        for (int row = 0; row < rows.Count; row++)
        {
            List<PageTableCell> cells = [];

            foreach (PendingCell cell in rows[row].Cells)
            {
                if (cell.Merge == VerticalMerge.Continue) continue;

                int span = cell.Merge == VerticalMerge.Restart
                    ? 1 + Continuations(rows, row, cell.Definition.Column)
                    : 1;

                cells.Add(cell.Definition with { RowSpan = span });
            }

            resolved.Add(new PageTableRow
            {
                Cells = cells,
                IsHeader = rows[row].IsHeading,
                MinHeight = rows[row].Height.Height,
                HasExactHeight = rows[row].Height.IsExact,
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
               && int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out int twips)
            ? Length.FromTwips(twips)
            : null;
    }

    private static int? Number(XElement? element)
        => Word.Attribute(element, "val") is { } text
           && int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out int value)
            ? value
            : null;

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
    private readonly record struct PendingCell(PageTableCell Definition, VerticalMerge Merge);

    /// <summary>A row before its cells' row spans are known.</summary>
    private readonly record struct PendingRow(
        List<PendingCell> Cells, bool IsHeading, (Length Height, bool IsExact) Height);
}

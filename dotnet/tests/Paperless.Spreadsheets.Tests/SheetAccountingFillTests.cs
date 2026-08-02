using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Numbers;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// An accounting format's <c>*</c> fill, as it reaches the paper.
/// </summary>
/// <remarks>
/// <para>
/// <c>_("$"* #,##0.00_)</c> is what Excel writes for every accounting cell, and the <c>*</c> in
/// it means "repeat the next character until the column is full" — which is what puts the
/// currency symbol against the left edge of the cell and the digits against the right.
/// Extraction has no column and so drops it, and the drawing path used to inherit that: a cell
/// showing <c>$ 1,234.50</c> in Calc showed <c>$1,234.50</c> here, with the symbol jammed
/// against its digits at the right-hand end.
/// </para>
/// <para>
/// The cost was two words per cell to any comparison that counts them, because the gap is what
/// separates <c>$</c> from the number. Measured on the corpus's first sheets batch, where three
/// of the four failures were nothing else: <c>REDAC_SCHEDULE_RPD_135.xls</c> 178 words against
/// LibreOffice's 202 — exactly its 24 currency cells — and
/// <c>einvoice-summary-worksheet.xlsx</c> 203 against 298.
/// </para>
/// <para>
/// Asserted through the drawn glyph run rather than through the formatter, because the
/// formatter's answer is deliberately <em>not</em> the expansion: it marks the point and the
/// character, and only a caller holding a column width can say how many copies go there.
/// </para>
/// </remarks>
public sealed class SheetAccountingFillTests
{
    private const string Accounting =
        "_(\"$\"* #,##0.00_);_(\"$\"* \\(#,##0.00\\);_(\"$\"* \"-\"??_);_(@_)";

    /// <summary>One cell of a given value, in a column of a given width.</summary>
    private static SheetLayout Sheet(double value, Length columnWidth, string code = Accounting)
    {
        NumberFormatCode format = NumberFormatCode.Parse(code);
        string text = NumberFormatter.Format(format, value);

        ContentTable table = new();
        ContentTableRow row = new() { Index = 0 };
        ContentTableCell cell = new() { Row = 0, Column = 0, Value = value };
        ContentParagraph paragraph = new();
        paragraph.Children.Add(new ContentRun { Text = text });
        cell.Children.Add(paragraph);
        row.Children.Add(cell);
        table.Children.Add(row);

        SheetCellFormats.Builder formats = new();
        formats.SetCell(0, 0, formats.Intern(new SheetCellFormat
        {
            NumberFormatKind = format.Sections[0].Kind,
            NumberFormat = format,
        }));

        return new SheetLayout
        {
            Name = "Sheet1",
            Cells = table,
            Formats = formats.Build(),
            Grid = new SheetGrid(new SheetAxis(columnWidth), new SheetAxis(Length.FromTwips(256))),
        };
    }

    private static string Drawn(SheetLayout sheet)
    {
        SpreadsheetPages pages = new([sheet]);
        RecordingDrawingSink sink = new();
        pages.Pages[0].Draw(sink);

        return string.Concat(sink.Pages[0].Runs.Select(run => run.Text));
    }

    /// <summary>
    /// The fill expands where the code puts it, and the symbol ends up nowhere near the digits.
    /// </summary>
    [Fact]
    public void TheFillSeparatesTheSymbolFromTheDigits()
    {
        // Four inches of column for a nine-character number, so there is a great deal to fill.
        string drawn = Drawn(Sheet(1234.5, Length.FromInches(4)));

        drawn.ShouldContain("$");
        drawn.ShouldContain("1,234.50");

        int symbol = drawn.IndexOf('$', StringComparison.Ordinal);
        int digits = drawn.IndexOf('1', StringComparison.Ordinal);

        // Everything between them is the fill, and there is a lot of it: without the expansion
        // the two are adjacent.
        digits.ShouldBeGreaterThan(symbol + 20);
        drawn[(symbol + 1)..digits].Trim().ShouldBeEmpty();
    }

    /// <summary>
    /// A column with no room to spare gets no fill at all, which is Calc's own early return.
    /// </summary>
    /// <remarks>
    /// <c>if ( nSpaceToFill &lt;= nCharWidth ) return;</c> (<c>output2.cxx:600</c>). Without it a
    /// cell that only just fits its number would gain a space it has no room for and be pushed
    /// into a clip.
    /// </remarks>
    [Fact]
    public void ANarrowColumnGetsNoFill()
    {
        // " $1,234.50 " measures 50.04 pt in ten-point Liberation Sans and the two cell margins
        // take a further 1.98, so a 54 pt column has under two points to spare — less than the
        // 2.78 pt a space costs, which is exactly the case Calc returns early on.
        string drawn = Drawn(Sheet(1234.5, Length.FromPoints(54)));

        int symbol = drawn.IndexOf('$', StringComparison.Ordinal);
        int digits = drawn.IndexOf('1', StringComparison.Ordinal);

        symbol.ShouldBeGreaterThanOrEqualTo(0);
        digits.ShouldBe(symbol + 1);
    }

    /// <summary>
    /// A format with no fill directive is untouched, however wide its column.
    /// </summary>
    [Fact]
    public void AFormatWithoutAFillIsDrawnAsItStands()
        => Drawn(Sheet(1234.5, Length.FromInches(4), "\"$\"#,##0.00")).ShouldBe("$1,234.50");
}

using System.Globalization;
using System.IO.Compression;
using System.Text;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A chart data range that ends on an Excel table's totals row.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScChart2DataSequence::BuildDataCache</c> (<c>sc/source/ui/unoobj/chart2uno.cxx:2616-2632</c>)
/// skips a cell when it is the <em>last row of the range being read</em>, a database range with a
/// totals row covers it, and that database range ends on the same row. Its comment: "Excel
/// behavior: if the last row is the totals row, the data is not added to the chart. If it's not
/// the last row, the data is added like normal."
/// </para>
/// <para>
/// <strong>Every expectation here is the observable of an authored variant rendered by LibreOffice
/// 26.2.4.2</strong>, not a reading of the C++. `029_Annual_budget_Use_this_template_30324a97.xlsx`
/// with `totalsRowCount="1"` renders one chart empty and the other seventeen bars; the same file
/// with the one attribute changed to `"0"` renders the first chart plotted and the second eighteen
/// bars. See `dotnet/probes/sheets-r53-totalsrow/`.
/// </para>
/// <para>
/// <strong>Each case asserts the shape it claims to be testing is present.</strong> A table part
/// is reached by relationship from the worksheet, so a fixture that fails to attach one would make
/// every case here pass by accident and test nothing —
/// <see cref="TheSameRangeWithNoTablePartAtAllKeepsItsLastRow"/> is the control that fails if the
/// package builder stops attaching the part, and the paired assertions inside each case pin the
/// difference rather than the absolute.
/// </para>
/// </remarks>
public sealed class XlsxChartTotalsRowTests
{
    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    private const string Rns = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    /// <summary>Six rows of two columns: B holds 10..60, C holds 1..6.</summary>
    private const string Rows = """
        <row r="1"><c r="B1" t="inlineStr"><is><t>Head</t></is></c><c r="C1" t="inlineStr"><is><t>Also</t></is></c></row>
        <row r="2"><c r="B2"><v>10</v></c><c r="C2"><v>1</v></c></row>
        <row r="3"><c r="B3"><v>20</v></c><c r="C3"><v>2</v></c></row>
        <row r="4"><c r="B4"><v>30</v></c><c r="C4"><v>3</v></c></row>
        <row r="5"><c r="B5"><v>40</v></c><c r="C5"><v>4</v></c></row>
        <row r="6"><c r="B6"><v>50</v></c><c r="C6"><v>5</v></c></row>
        """;

    /// <summary>A resolver over one sheet, with the table parts given attached to it.</summary>
    private static XlsxChartRanges Ranges(params string[] tables)
    {
        MemoryStream stream = new(Package(Rows, tables));
        XlsxFile file = XlsxFile.Open(stream);
        return new XlsxChartRanges(file, new XlsxSheetReader(file, []));
    }

    /// <summary>A table element, as Excel writes one.</summary>
    private static string Table(
        string reference, int totals = 1, int id = 1, string name = "Costs") =>
        string.Create(CultureInfo.InvariantCulture,
            $"""<table xmlns="{Ns}" id="{id}" name="{name}" displayName="{name}" ref="{reference}" totalsRowCount="{totals}"><tableColumns count="2"><tableColumn id="1" name="Head"/><tableColumn id="2" name="Also"/></tableColumns></table>""");

    /// <summary>The corpus shape: a range whose last row is the table's totals row.</summary>
    [Fact]
    public void TheLastRowOfARangeThatEndsOnATableTotalsRowIsDropped()
    {
        ChartRangeValues values =
            Ranges(Table("B1:C6")).Resolve("'Data'!$B$2:$B$6").ShouldNotBeNull();

        values.Numbers.ShouldBe([10.0, 20.0, 30.0, 40.0]);
        values.Text.ShouldBe(["10", "20", "30", "40"]);
    }

    /// <summary>The control: the identical range with no table part resolves to all five cells.</summary>
    /// <remarks>
    /// This is the case that fails first if the package builder ever stops attaching the table
    /// relationship — without it every other case here would pass by testing nothing.
    /// </remarks>
    [Fact]
    public void TheSameRangeWithNoTablePartAtAllKeepsItsLastRow()
    {
        Ranges().Resolve("'Data'!$B$2:$B$6").ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 30.0, 40.0, 50.0]);
    }

    /// <summary>
    /// A totals row that is <em>not</em> the last row of the range is read like any other cell.
    /// </summary>
    /// <remarks>
    /// The half of the rule that is easiest to drop, and the one that decides whether this is a
    /// property of the range or of the table: LibreOffice only ever tests
    /// <c>nRow == aRange.aEnd.Row()</c>.
    /// </remarks>
    [Fact]
    public void ATotalsRowAboveTheEndOfTheRangeIsReadLikeAnyOtherCell()
    {
        Ranges(Table("B1:C4")).Resolve("'Data'!$B$2:$B$6").ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 30.0, 40.0, 50.0]);
    }

    /// <summary>
    /// A range that is <em>wholly</em> a totals row resolves to no points, and that is not the
    /// same answer as failing to resolve.
    /// </summary>
    /// <remarks>
    /// This is `029_Annual_budget`'s left chart: both series read a single row that is the table's
    /// totals row, so both are empty and LibreOffice draws an empty plot at the axis's default
    /// scale. Answering null here would leave the cached points standing and draw the whole chart.
    /// </remarks>
    [Fact]
    public void ARangeThatIsWhollyATotalsRowResolvesToNoPointsRatherThanToNull()
    {
        ChartRangeValues values =
            Ranges(Table("B1:C6")).Resolve("'Data'!$B$6:$C$6").ShouldNotBeNull();

        values.Numbers.ShouldBeEmpty();
        values.Text.ShouldBeEmpty();
    }

    /// <summary>A table with no totals row hides nothing.</summary>
    [Fact]
    public void ATableWithoutATotalsRowHidesNothing()
    {
        Ranges(Table("B1:C6", totals: 0)).Resolve("'Data'!$B$2:$B$6").ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 30.0, 40.0, 50.0]);
    }

    /// <summary>
    /// A table whose totals row is <em>below</em> the end of the range hides nothing.
    /// </summary>
    /// <remarks>
    /// The mirror of <see cref="ATotalsRowAboveTheEndOfTheRangeIsReadLikeAnyOtherCell"/> and the
    /// case that separates "the database range ends on this row" from "a database range with
    /// totals covers this cell". A chart plotting the first five rows of a twenty-row table must
    /// keep all five.
    /// </remarks>
    [Fact]
    public void ATableWhoseTotalsRowIsBelowTheEndOfTheRangeHidesNothing()
    {
        Ranges(Table("B1:C8")).Resolve("'Data'!$B$2:$B$6").ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 30.0, 40.0, 50.0]);
    }

    /// <summary>
    /// The test is per column: a range wider than the table keeps the last cell of the columns
    /// the table does not cover.
    /// </summary>
    /// <remarks>
    /// LibreOffice asks <c>GetDBAtCursor(nCol, nRow, …)</c> inside the column loop. No corpus
    /// document has this shape — all four hits over 946 documents are one column or one row — so
    /// this case exists precisely because the corpus cannot pin it.
    /// </remarks>
    [Fact]
    public void AColumnTheTableDoesNotCoverKeepsItsLastCell()
    {
        ChartRangeValues values =
            Ranges(Table("B1:B6")).Resolve("'Data'!$B$5:$C$6").ShouldNotBeNull();

        // Row-major over B5,C5,B6,C6 with B6 dropped as the table's totals row.
        values.Numbers.ShouldBe([40.0, 4.0, 5.0]);
    }

    /// <summary>
    /// A table with no <c>displayName</c> never becomes a database range, so it has no totals row.
    /// </summary>
    /// <remarks>
    /// <c>Table::finalizeImport</c> (<c>sc/source/filter/oox/tablebuffer.cxx:107-113</c>) returns
    /// before creating the range when the id is not positive or the display name is empty, and a
    /// range that does not exist cannot be found by <c>GetDBAtCursor</c>.
    /// </remarks>
    [Fact]
    public void ATableWithNoDisplayNameHidesNothing()
    {
        Ranges(Table("B1:C6", name: "")).Resolve("'Data'!$B$2:$B$6").ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 30.0, 40.0, 50.0]);
    }

    /// <summary>The same, for a table whose id is not positive.</summary>
    [Fact]
    public void ATableWhoseIdIsNotPositiveHidesNothing()
    {
        Ranges(Table("B1:C6", id: 0)).Resolve("'Data'!$B$2:$B$6").ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 30.0, 40.0, 50.0]);
    }

    /// <summary>A minimal package of one sheet, with the table parts attached to that sheet.</summary>
    private static byte[] Package(string rows, string[] tables)
    {
        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            StringBuilder types = new();
            StringBuilder sheetRelationships = new();
            StringBuilder parts = new();

            for (int at = 0; at < tables.Length; at++)
            {
                string part = $"xl/tables/table{at + 1}.xml";

                types.Append(CultureInfo.InvariantCulture,
                    $"""<Override PartName="/{part}" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.table+xml"/>""");
                sheetRelationships.Append(CultureInfo.InvariantCulture,
                    $"""<Relationship Id="tId{at + 1}" Type="{Rns}/table" Target="/{part}"/>""");
                parts.Append(CultureInfo.InvariantCulture,
                    $"""<tablePart r:id="tId{at + 1}"/>""");

                Write(archive, part, tables[at]);
            }

            Write(archive, "[Content_Types].xml",
                """<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">"""
                + """<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>"""
                + """<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>"""
                + """<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>"""
                + types + "</Types>");

            Write(archive, "_rels/.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/officeDocument" Target="xl/workbook.xml"/>"""
                + "</Relationships>");

            Write(archive, "xl/_rels/workbook.xml.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/worksheet" Target="/xl/worksheets/sheet1.xml"/>"""
                + "</Relationships>");

            Write(archive, "xl/_rels/../worksheets/_rels/sheet1.xml.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + sheetRelationships + "</Relationships>");

            Write(archive, "xl/workbook.xml",
                $"""<workbook xmlns="{Ns}" xmlns:r="{Rns}"><sheets><sheet name="Data" sheetId="1" r:id="rId1"/></sheets></workbook>""");

            Write(archive, "xl/worksheets/sheet1.xml",
                $"""<worksheet xmlns="{Ns}" xmlns:r="{Rns}"><sheetData>{rows}</sheetData><tableParts count="{tables.Length}">{parts}</tableParts></worksheet>""");
        }

        return buffer.ToArray();
    }

    private static void Write(ZipArchive archive, string name, string content)
    {
        using Stream entry = archive.CreateEntry(name).Open();
        byte[] bytes = Encoding.UTF8.GetBytes(content);
        entry.Write(bytes, 0, bytes.Length);
    }
}

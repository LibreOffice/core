using System.Globalization;
using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Extraction;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A pivot table's repeated row labels, which Calc lays out itself and does not print.
/// </summary>
/// <remarks>
/// <para>
/// Calc imports a pivot table's <em>definition</em> and regenerates its output through
/// <c>ScDPOutput</c>, writing a row field's label only where its group starts. Excel's "Repeat All
/// Item Labels" (<c>x14:pivotField/@fillDownLabels</c>) writes the repeats into the cells; Calc
/// ignores it.
/// </para>
/// <para>
/// <strong>Every expectation here is an observable of LibreOffice 26.2.4.2</strong>, established on
/// `DynamicBubbleChart.xlsx` by three authored variants: the corpus file draws each department name
/// once; with `fillDownLabels="0"` it still draws it once; with the pivot part removed it draws all
/// three. See `dotnet/probes/sheets-r53-totalsrow/pivot-variants.py`.
/// </para>
/// <para>
/// <strong>Each case asserts the shape it claims to test is present.</strong> The pivot part is
/// reached by relationship from the worksheet, so a builder that stopped attaching it would make
/// every case here pass while testing nothing —
/// <see cref="TheSameCellsWithNoPivotPartAtAllAreAllDrawn"/> is the control that catches it.
/// </para>
/// </remarks>
public sealed class XlsxPivotLabelTests
{
    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    private const string Rns = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    /// <summary>
    /// `DynamicBubbleChart`'s shape in miniature: a header row, then two groups of two rows whose
    /// outer label repeats, with an inner column that repeats a value under two different keys.
    /// </summary>
    private const string Rows = """
        <row r="1"><c r="A1" t="inlineStr"><is><t>Dept</t></is></c><c r="B1" t="inlineStr"><is><t>Risk</t></is></c><c r="C1" t="inlineStr"><is><t>Cost</t></is></c></row>
        <row r="2"><c r="A2" t="inlineStr"><is><t>Finance</t></is></c><c r="B2"><v>1</v></c><c r="C2"><v>150</v></c></row>
        <row r="3"><c r="A3" t="inlineStr"><is><t>Finance</t></is></c><c r="B3"><v>2</v></c><c r="C3"><v>150</v></c></row>
        <row r="4"><c r="A4" t="inlineStr"><is><t>Purchase</t></is></c><c r="B4"><v>3</v></c><c r="C4"><v>150</v></c></row>
        """;

    /// <summary>The text of one cell as the reader materialises it, or the empty string.</summary>
    private static string Cell(ContentTable table, int row, int column)
    {
        foreach (ContentNode node in table.Children)
        {
            if (node is not ContentTableRow line || line.Index != row) continue;
            foreach (ContentNode child in line.Children)
            {
                if (child is ContentTableCell cell && cell.Column == column) return cell.GetText();
            }
        }
        return string.Empty;
    }

    /// <summary>Reads the one sheet, with the pivot parts given attached to it.</summary>
    private static ContentTable Read(params string[] pivots) => Read(Rows, pivots);

    /// <summary>The same, over cells the caller states.</summary>
    private static ContentTable Read(string rows, string[] pivots)
    {
        MemoryStream stream = new(Package(rows, pivots));
        XlsxFile file = XlsxFile.Open(stream);
        XlsxSheetReader reader = new(file, []);
        XlsxSheetEntry entry = file.Sheets[0];
        return reader.ReadSheet(file.LoadSheet(entry)!, entry);
    }

    /// <summary>A pivot table definition covering <c>A1:C4</c>.</summary>
    private static string Pivot(string reference = "A1:C4", int firstDataRow = 1, int firstDataCol = 2) =>
        string.Create(CultureInfo.InvariantCulture,
            $"""<pivotTableDefinition xmlns="{Ns}" name="P" cacheId="0"><location ref="{reference}" firstHeaderRow="1" firstDataRow="{firstDataRow}" firstDataCol="{firstDataCol}"/></pivotTableDefinition>""");

    /// <summary>The corpus shape: the outer label repeats and the repeat is not drawn.</summary>
    [Fact]
    public void ARowLabelThatRepeatsTheRowAboveIsNotDrawn()
    {
        ContentTable table = Read(Pivot());

        Cell(table, 1, 0).ShouldBe("Finance");   // the group starts here
        Cell(table, 2, 0).ShouldBeEmpty();       // and this repeat is Calc's blank
        Cell(table, 3, 0).ShouldBe("Purchase");  // a new group
    }

    /// <summary>The control: the identical cells with no pivot part are all drawn.</summary>
    /// <remarks>
    /// Fails first if the package builder ever stops attaching the pivot relationship, which would
    /// otherwise make every case in this class pass without exercising anything.
    /// </remarks>
    [Fact]
    public void TheSameCellsWithNoPivotPartAtAllAreAllDrawn()
    {
        ContentTable table = Read();

        Cell(table, 1, 0).ShouldBe("Finance");
        Cell(table, 2, 0).ShouldBe("Finance");
        Cell(table, 3, 0).ShouldBe("Purchase");
    }

    /// <summary>
    /// A repeated value whose <em>prefix</em> differs is drawn — the test is on the group, not on
    /// the cell above.
    /// </summary>
    /// <remarks>
    /// `Cost` holds <c>150</c> on all three data rows. Rows 2 and 3 sit under different `Risk`
    /// values and row 4 under a different `Dept`, so LibreOffice prints all three and a
    /// cell-above test would blank two of them. This is the case that decides whether the rule is
    /// about a pivot group or about adjacency.
    /// </remarks>
    [Fact]
    public void AValueRepeatedUnderADifferentGroupKeyIsStillDrawn()
    {
        ContentTable table = Read(Pivot(firstDataCol: 3));

        Cell(table, 1, 2).ShouldBe("150");
        Cell(table, 2, 2).ShouldBe("150");
        Cell(table, 3, 2).ShouldBe("150");
    }

    /// <summary>A column past <c>firstDataCol</c> is data, not a label, and is never blanked.</summary>
    [Fact]
    public void AColumnBeyondTheLabelColumnsIsNeverBlanked()
    {
        // firstDataCol=1 makes only column A a label column; C's repeats are data.
        ContentTable table = Read(Pivot(firstDataCol: 1));

        Cell(table, 2, 0).ShouldBeEmpty();
        Cell(table, 2, 2).ShouldBe("150");
        Cell(table, 3, 2).ShouldBe("150");
    }

    /// <summary>
    /// The first data row is never blanked, even when the header above it reads the same.
    /// </summary>
    /// <remarks>
    /// The scan starts at <c>firstDataRow + 1</c> because the first data row has no row above it
    /// <em>inside the pivot</em>. A field named the same as one of its own items is the shape that
    /// tells the two starting points apart: comparing against the header would blank the first
    /// group's label and leave the pivot with no label at all.
    /// </remarks>
    [Fact]
    public void TheFirstDataRowIsNotBlankedByAHeaderThatReadsTheSame()
    {
        const string Echoed = """
            <row r="1"><c r="A1" t="inlineStr"><is><t>Finance</t></is></c><c r="B1" t="inlineStr"><is><t>Risk</t></is></c></row>
            <row r="2"><c r="A2" t="inlineStr"><is><t>Finance</t></is></c><c r="B2"><v>1</v></c></row>
            <row r="3"><c r="A3" t="inlineStr"><is><t>Finance</t></is></c><c r="B3"><v>2</v></c></row>
            """;

        ContentTable table = Read(Echoed, [Pivot("A1:B3", firstDataCol: 1)]);

        Cell(table, 0, 0).ShouldBe("Finance");   // the header
        Cell(table, 1, 0).ShouldBe("Finance");   // the group starts here and must survive
        Cell(table, 2, 0).ShouldBeEmpty();       // this one is the repeat
    }

    /// <summary>
    /// A column at or beyond <c>firstDataCol</c> is data and is printed even when the whole label
    /// prefix repeats.
    /// </summary>
    /// <remarks>
    /// Synthetic, and deliberately so: a real pivot's row-label tuples are distinct, so no corpus
    /// document can put two identical label prefixes on consecutive rows. What the case pins is the
    /// <em>definition</em> of a label column — that it is <c>firstDataCol</c> that separates the two
    /// halves of the location, and not the location's whole width. Getting that wrong would blank
    /// a measure whose value happens to repeat.
    /// </remarks>
    [Fact]
    public void ADataColumnIsPrintedEvenWhenTheWholeLabelPrefixRepeats()
    {
        const string Twinned = """
            <row r="1"><c r="A1" t="inlineStr"><is><t>Dept</t></is></c><c r="B1" t="inlineStr"><is><t>Item</t></is></c><c r="C1" t="inlineStr"><is><t>Cost</t></is></c></row>
            <row r="2"><c r="A2" t="inlineStr"><is><t>Finance</t></is></c><c r="B2" t="inlineStr"><is><t>Rent</t></is></c><c r="C2"><v>150</v></c></row>
            <row r="3"><c r="A3" t="inlineStr"><is><t>Finance</t></is></c><c r="B3" t="inlineStr"><is><t>Rent</t></is></c><c r="C3"><v>150</v></c></row>
            """;

        ContentTable table = Read(Twinned, [Pivot("A1:C3", firstDataCol: 2)]);

        Cell(table, 2, 0).ShouldBeEmpty();   // label, repeated
        Cell(table, 2, 1).ShouldBeEmpty();   // label, prefix repeated too
        Cell(table, 2, 2).ShouldBe("150");   // data, always printed
    }

    /// <summary>
    /// An outer label that is <em>already</em> blank stops the scan, so the columns to its right
    /// keep their text.
    /// </summary>
    /// <remarks>
    /// <strong>This is the property the whole census rests on and it is a deliberate choice, not
    /// an accident of the loop.</strong> A pivot Excel laid out without "Repeat All Item Labels"
    /// already has its repeats blank, and stopping at the first blank means this rule can only
    /// ever remove text Excel <em>filled down</em> — it is a no-op on all ten of the corpus's other
    /// pivot documents by construction rather than by luck. Continuing past the blank would let it
    /// blank cells in a document that never asked for the repeats in the first place, which is the
    /// only way this change could reach a document the census does not name.
    /// </remarks>
    [Fact]
    public void AnOuterLabelThatIsAlreadyBlankStopsTheScan()
    {
        const string Gapped = """
            <row r="1"><c r="A1" t="inlineStr"><is><t>Dept</t></is></c><c r="B1" t="inlineStr"><is><t>Item</t></is></c></row>
            <row r="2"><c r="A2" t="inlineStr"><is><t>Finance</t></is></c><c r="B2" t="inlineStr"><is><t>Rent</t></is></c></row>
            <row r="3"><c r="B3" t="inlineStr"><is><t>Rent</t></is></c></row>
            """;

        ContentTable table = Read(Gapped, [Pivot("A1:B3", firstDataCol: 2)]);

        Cell(table, 2, 0).ShouldBeEmpty();
        Cell(table, 2, 1).ShouldBe("Rent");
    }

    /// <summary>A pivot whose location does not cover the cells leaves them alone.</summary>
    [Fact]
    public void APivotElsewhereOnTheSheetBlanksNothingHere()
    {
        ContentTable table = Read(Pivot("F1:H4"));

        Cell(table, 2, 0).ShouldBe("Finance");
    }

    /// <summary>A minimal package of one sheet with the pivot parts attached to it.</summary>
    private static byte[] Package(string rows, string[] pivots)
    {
        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            StringBuilder types = new();
            StringBuilder sheetRelationships = new();

            for (int at = 0; at < pivots.Length; at++)
            {
                string part = $"xl/pivotTables/pivotTable{at + 1}.xml";

                types.Append(CultureInfo.InvariantCulture,
                    $"""<Override PartName="/{part}" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml"/>""");
                sheetRelationships.Append(CultureInfo.InvariantCulture,
                    $"""<Relationship Id="pId{at + 1}" Type="{Rns}/pivotTable" Target="/{part}"/>""");

                Write(archive, part, pivots[at]);
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

            Write(archive, "xl/worksheets/_rels/sheet1.xml.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + sheetRelationships + "</Relationships>");

            Write(archive, "xl/workbook.xml",
                $"""<workbook xmlns="{Ns}" xmlns:r="{Rns}"><sheets><sheet name="Data" sheetId="1" r:id="rId1"/></sheets></workbook>""");

            Write(archive, "xl/worksheets/sheet1.xml",
                $"""<worksheet xmlns="{Ns}" xmlns:r="{Rns}"><sheetData>{rows}</sheetData></worksheet>""");
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

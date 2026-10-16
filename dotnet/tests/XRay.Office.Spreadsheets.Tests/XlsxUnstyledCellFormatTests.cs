using System.IO.Compression;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Which number format a cell takes when it, its row or its column states one — and which it
/// takes when none of them does.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A cell that states no <c>s</c> does not take <c>cellXfs[0]</c>.</strong> The importer
/// reads an absent <c>@s</c> as <em>no XF at all</em> — <c>rAttribs.getInteger(XML_s, -1)</c>
/// (<c>sc/source/filter/oox/sheetdatacontext.cxx</c>:371) — and
/// <c>SheetDataBuffer::setCellFormat</c> returns immediately on a negative id
/// (<c>sheetdatabuffer.cxx</c>:721), so the cell keeps whatever the sheet already put there: the
/// row's default pattern if the row says <c>customFormat</c>, else the column's
/// <c>&lt;col style=…&gt;</c>, else the document's Default cell style, which is the
/// <c>cellStyleXfs</c> entry the <c>Normal</c> <c>cellStyle</c> names.
/// </para>
/// <para>
/// All four rules were measured rather than read: <c>dotnet/probes/numfmt-r68/make-default.py</c>
/// builds a workbook giving the Default cell style, <c>cellXfs[0]</c>, a column and a
/// <c>customFormat</c> row four distinguishable formats and renders it through both installed
/// binaries. <strong>24.2.7.2 and 26.2.4.2 answer identically on all six cells</strong>, so this
/// is ours and not the version gap. The corpus cannot separate the first pair — every workbook in
/// it gives <c>cellXfs[0]</c> and the Default cell style the same id — which is exactly why it had
/// to be probed.
/// </para>
/// <para>
/// The defect this closes: <c>042_Business_monthly_budget_4e4d092f.xlsx</c> states
/// <c>numFmtId="40"</c> on both and writes most of its numeric cells with no <c>s</c> at all, so
/// they extracted as <c>General</c> — <c>54000</c>, <c>-6000</c>, <c>500</c> — where both
/// references print <c>54,000.00</c>, <c>(6,000.00)</c> and <c>500.00</c>.
/// </para>
/// </remarks>
public class XlsxUnstyledCellFormatTests
{
    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    private const string Rns = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    [Fact]
    public void ACellThatStatesNoStyleTakesTheDefaultCellStyleAndNotCellXfsZero()
    {
        List<string> cells = TextOf(Package());

        // cellStyleXfs[0] is "0.0" and cellXfs[0] is "0.000". The two answers are three
        // characters apart, which is what makes the case worth building.
        cells[0].ShouldBe("1.0", "a cell with no s takes the Default cell style");
        cells[1].ShouldBe("1.000", "an explicit s=\"0\" takes cellXfs[0]");
        cells[2].ShouldBe("1", "an explicit s pointing at General is General");
    }

    [Fact]
    public void AColumnStyleAndACustomFormatRowReachACellThatStatesNone()
    {
        List<string> cells = TextOf(Package());

        cells[3].ShouldBe("1.00000", "<col style> reaches a cell with no s of its own");
        cells[4].ShouldBe("1.0000000", "a customFormat row reaches it in preference to the column");
        cells[5].ShouldBe(
            "1.00000",
            "a row that states s WITHOUT customFormat does not reach its cells, so the "
            + "column's style still applies");
    }

    [Fact]
    public void TheCellsOwnStyleWinsOverBothTheRowAndTheColumn()
    {
        List<string> cells = TextOf(Package());

        cells[6].ShouldBe("1", "s wins over a customFormat row");
        cells[7].ShouldBe("1", "s wins over a column style");
    }

    /// <summary>The one filled cell of each row, in row order.</summary>
    private static List<string> TextOf(byte[] package)
    {
        MemoryStream stream = new(package);
        XlsxFile file = XlsxFile.Open(stream);
        XElement worksheet = file.LoadSheet(file.Sheets[0]).ShouldNotBeNull(
            "the probe worksheet must load, or nothing below is measured");

        ContentTable table = new XlsxSheetReader(file, []).ReadSheet(worksheet);

        List<string> values = [];
        foreach (ContentNode node in table.Children)
        {
            if (node is not ContentTableRow row) continue;
            foreach (ContentNode cell in row.Children)
            {
                string text = cell.GetText().Trim();
                if (text.Length > 0) values.Add(text);
            }
        }
        return values;
    }


    // cellStyleXfs[0] -> 0.0        the Default cell style, which `Normal` names
    // cellXfs[0]      -> 0.000
    // cellXfs[1]      -> General
    // cellXfs[2]      -> 0.00000    named by <col min="3" max="3" style="2">
    // cellXfs[3]      -> 0.0000000  named by the customFormat rows
    private const string Styles =
        $"""
        <styleSheet xmlns="{Ns}">
        <numFmts count="4">
        <numFmt numFmtId="200" formatCode="0.000"/><numFmt numFmtId="201" formatCode="0.0"/>
        <numFmt numFmtId="202" formatCode="0.00000"/><numFmt numFmtId="203" formatCode="0.0000000"/>
        </numFmts>
        <cellStyleXfs count="1"><xf numFmtId="201" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
        <cellXfs count="4">
        <xf numFmtId="200" fontId="0" fillId="0" borderId="0" xfId="0"/>
        <xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>
        <xf numFmtId="202" fontId="0" fillId="0" borderId="0" xfId="0" applyNumberFormat="1"/>
        <xf numFmtId="203" fontId="0" fillId="0" borderId="0" xfId="0" applyNumberFormat="1"/>
        </cellXfs>
        <cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
        </styleSheet>
        """;

    private const string Sheet =
        $"""
        <worksheet xmlns="{Ns}" xmlns:r="{Rns}">
        <cols><col min="3" max="3" width="24" style="2" customWidth="1"/></cols>
        <sheetData>
        <row r="1"><c r="B1"><v>1</v></c></row>
        <row r="2"><c r="B2" s="0"><v>1</v></c></row>
        <row r="3"><c r="B3" s="1"><v>1</v></c></row>
        <row r="4"><c r="C4"><v>1</v></c></row>
        <row r="5" s="3" customFormat="1"><c r="B5"><v>1</v></c></row>
        <row r="6" s="3"><c r="C6"><v>1</v></c></row>
        <row r="7" s="3" customFormat="1"><c r="B7" s="1"><v>1</v></c></row>
        <row r="8"><c r="C8" s="1"><v>1</v></c></row>
        </sheetData></worksheet>
        """;

    private static byte[] Package()
    {
        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml",
                """<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">"""
                + """<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>"""
                + """<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>"""
                + """<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>"""
                + """<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>"""
                + "</Types>");

            Write(archive, "_rels/.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/officeDocument" Target="xl/workbook.xml"/>"""
                + "</Relationships>");

            Write(archive, "xl/_rels/workbook.xml.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/worksheet" Target="/xl/worksheets/sheet1.xml"/>"""
                + $"""<Relationship Id="rId2" Type="{Rns}/styles" Target="/xl/styles.xml"/>"""
                + "</Relationships>");

            Write(archive, "xl/workbook.xml",
                $"""<workbook xmlns="{Ns}" xmlns:r="{Rns}"><sheets>"""
                + """<sheet name="Sheet1" sheetId="1" r:id="rId1"/></sheets></workbook>""");

            Write(archive, "xl/styles.xml", Styles);
            Write(archive, "xl/worksheets/sheet1.xml", Sheet);
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

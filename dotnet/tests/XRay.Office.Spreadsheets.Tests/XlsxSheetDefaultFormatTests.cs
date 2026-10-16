using System.IO.Compression;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Which <em>drawn</em> format a cell takes when neither it, its row nor its column states one.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="XlsxUnstyledCellFormatTests"/> asks this of the number format and answers it with
/// the <c>Normal</c> cell style. This asks it of the font and the alignment, which travel down a
/// different reader — <see cref="XlsxSheetFormats"/> into <see cref="SheetCellFormats"/> — and
/// which took <c>cellXfs[0]</c> until round 111. The two answering differently is worse than
/// either, because a workbook separating them would then be drawn in one entry's font and one
/// entry's number format.
/// </para>
/// <para>
/// <strong>The rule is the reference's, measured twice.</strong> On
/// <c>tests/corpus/regression/pivot-default-style.xlsx</c> — whose <c>Normal</c>
/// <c>cellStyleXf</c> is Liberation Sans 11 black and whose <c>cellXfs[0]</c> is Liberation Serif
/// 18 red — 26.2.4.2's own <c>.fods</c> gives the <c>Plain</c> sheet's <c>A13:C16</c>, which
/// state no <c>s</c> at all, the <strong>Normal</strong> font, and its <c>A7:C10</c>, which state
/// <c>s="0"</c>, the <c>cellXfs[0]</c> one. And on the corpus: of the 243 <c>.xlsx</c>/<c>.xlsm</c>
/// of the sheets track that carry both tables, three give the two entries different
/// <em>effective</em> content, and in 26.2.4.2's <c>.fods</c> of all three the <c>Default</c>
/// cell style carries the <c>Normal</c> entry's content —
/// <c>sectors-defense-and-aerospace.xlsx</c>, whose <c>cellXfs[0]</c> states
/// <c>vertical="top" wrapText="1"</c>, comes back <c>style:vertical-align="bottom"</c> with no
/// wrap. See <c>probes/sheet-default-r111</c>.
/// </para>
/// <para>
/// The package below is that corpus shape made separable: the <c>Normal</c> entry, the
/// <c>cellXfs[0]</c> entry and a column's entry state three different faces, sizes and colours,
/// and <c>cellXfs[0]</c> additionally states the alignment the two real workbooks state.
/// </para>
/// </remarks>
public sealed class XlsxSheetDefaultFormatTests
{
    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    private const string Rns = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    [Fact]
    public void ACellStatingNoStyleTakesTheNormalCellStyleAndNotCellXfsZero()
    {
        SheetCellFormats formats = FormatsOf(sheet: 0);

        SheetCellFormat plain = formats.At(0, 0);
        plain.FontFamily.ShouldBe("Liberation Sans", "A1 states no s, so it is the Normal style's");
        plain.FontSize.ShouldBe(Length.FromPoints(10));
        plain.Colour.ShouldBe(Colour.Black);
    }

    /// <summary>
    /// And the alignment with it, which is what the two corpus workbooks separate.
    /// </summary>
    /// <remarks>
    /// <c>cellXfs[0]</c> states <c>vertical="top" wrapText="1"</c> here exactly as
    /// <c>sectors-defense-and-aerospace.xlsx</c> does, and 26.2.4.2's <c>Default</c> cell style
    /// for that workbook is <c>style:vertical-align="bottom"</c> with no <c>fo:wrap-option</c>.
    /// </remarks>
    [Fact]
    public void TheUnstatedCellTakesNeitherTheAlignmentNorTheWrapOfCellXfsZero()
    {
        SheetCellFormat plain = FormatsOf(sheet: 0).At(0, 0);

        plain.Vertical.ShouldBe(SheetVerticalAlignment.Standard);
        plain.Wraps.ShouldBeFalse();
    }

    /// <summary>
    /// The control: <c>s="0"</c> still resolves through <c>cellXfs[0]</c>.
    /// </summary>
    /// <remarks>
    /// Without it the test above passes just as well on a tree that had swapped the two tables,
    /// or on a package whose two entries were secretly the same.
    /// </remarks>
    [Fact]
    public void ACellStatingStyleZeroStillTakesCellXfsZero()
    {
        SheetCellFormat stated = FormatsOf(sheet: 0).At(0, 1);

        stated.FontFamily.ShouldBe("Liberation Serif");
        stated.FontSize.ShouldBe(Length.FromPoints(18));
        stated.Colour.ShouldBe(Colour.FromRgb(0xFF0000));
        stated.Vertical.ShouldBe(SheetVerticalAlignment.Top);
        stated.Wraps.ShouldBeTrue();
    }

    /// <summary>A <c>&lt;col style&gt;</c> still reaches a cell that states none.</summary>
    [Fact]
    public void ABoundedColumnStyleStillWinsOverTheNormalCellStyle()
    {
        SheetCellFormat column = FormatsOf(sheet: 0).At(0, 2);

        column.FontFamily.ShouldBe("Liberation Mono");
        column.FontSize.ShouldBe(Length.FromPoints(8));
    }

    /// <summary>
    /// And a <c>&lt;col&gt;</c> running to the sheet's last column replaces the sheet default
    /// outright rather than being laid over the <c>Normal</c> style.
    /// </summary>
    /// <remarks>
    /// The second half of the rule, and the half that keeps this from being a substitution:
    /// <c>SheetDefault</c> is the <c>Normal</c> style <em>or</em>, on a sheet stating one, a
    /// full-width column's format. <c>Published_Issuances_2024.xlsx</c> is the corpus workbook
    /// that states one, which is why its <c>cellXfs[0]</c> reaches nothing either way.
    /// </remarks>
    [Fact]
    public void AFullWidthColumnStyleReplacesTheSheetDefault()
    {
        SheetCellFormats formats = FormatsOf(sheet: 1);

        formats.SheetDefault.FontFamily.ShouldBe("Liberation Mono");
        formats.At(0, 0).FontFamily.ShouldBe("Liberation Mono", "a cell stating no s of its own");
    }

    /// <summary>
    /// The table itself keeps both entries apart, which is what the sheet reader reads them from.
    /// </summary>
    [Fact]
    public void TheFormatTableSeparatesTheNormalStyleFromCellXfsZero()
    {
        using MemoryStream stream = new(Package());
        XlsxFile file = XlsxFile.Open(stream);
        XElement styles = file.StyleSheet.ShouldNotBeNull("the probe states a styles part");

        XlsxCellFormatTable table = XlsxCellFormats.Read(styles, file.Styles, null);

        table.StyleDefault.FontFamily.ShouldBe("Liberation Sans");
        table.Formats[0].FontFamily.ShouldBe("Liberation Serif");
    }

    private static SheetCellFormats FormatsOf(int sheet)
    {
        using MemoryStream stream = new(Package());
        XlsxFile file = XlsxFile.Open(stream);
        XElement worksheet = file.LoadSheet(file.Sheets[sheet]).ShouldNotBeNull(
            "the probe worksheet must load, or nothing below is measured");
        XElement styles = file.StyleSheet.ShouldNotBeNull("the probe states a styles part");

        XlsxCellFormatTable table = XlsxCellFormats.Read(styles, file.Styles, null);
        return XlsxSheetFormats.Read(worksheet, table, file).Formats;
    }

    //  fonts[0]  Liberation Sans 10 black   — the Normal cellStyleXf's
    //  fonts[1]  Liberation Serif 18 red    — cellXfs[0]'s, which also states the alignment
    //  fonts[2]  Liberation Mono 8          — cellXfs[1]'s, which the columns name
    private const string Styles =
        $"""
        <styleSheet xmlns="{Ns}">
        <fonts count="3">
        <font><sz val="10"/><color rgb="FF000000"/><name val="Liberation Sans"/></font>
        <font><sz val="18"/><color rgb="FFFF0000"/><name val="Liberation Serif"/></font>
        <font><sz val="8"/><color rgb="FF000000"/><name val="Liberation Mono"/></font>
        </fonts>
        <cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
        <cellXfs count="2">
        <xf numFmtId="0" fontId="1" fillId="0" borderId="0" xfId="0" applyFont="1" applyAlignment="1"><alignment vertical="top" wrapText="1"/></xf>
        <xf numFmtId="0" fontId="2" fillId="0" borderId="0" xfId="0" applyFont="1"/>
        </cellXfs>
        <cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
        </styleSheet>
        """;

    //  A1 states nothing, B1 states s="0", C1 is covered by a bounded <col style="1">.
    private const string Sheet1 =
        $"""
        <worksheet xmlns="{Ns}" xmlns:r="{Rns}">
        <cols><col min="3" max="3" width="24" style="1" customWidth="1"/></cols>
        <sheetData>
        <row r="1"><c r="A1" t="inlineStr"><is><t>a</t></is></c>
        <c r="B1" s="0" t="inlineStr"><is><t>b</t></is></c>
        <c r="C1" t="inlineStr"><is><t>c</t></is></c></row>
        </sheetData></worksheet>
        """;

    //  The same, with a <col> spanning to the sheet's last column.
    private const string Sheet2 =
        $"""
        <worksheet xmlns="{Ns}" xmlns:r="{Rns}">
        <cols><col min="1" max="16384" width="12" style="1"/></cols>
        <sheetData>
        <row r="1"><c r="A1" t="inlineStr"><is><t>a</t></is></c></row>
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
                + """<Override PartName="/xl/worksheets/sheet2.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>"""
                + """<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>"""
                + "</Types>");

            Write(archive, "_rels/.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/officeDocument" Target="xl/workbook.xml"/>"""
                + "</Relationships>");

            Write(archive, "xl/_rels/workbook.xml.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/worksheet" Target="/xl/worksheets/sheet1.xml"/>"""
                + $"""<Relationship Id="rId2" Type="{Rns}/worksheet" Target="/xl/worksheets/sheet2.xml"/>"""
                + $"""<Relationship Id="rId3" Type="{Rns}/styles" Target="/xl/styles.xml"/>"""
                + "</Relationships>");

            Write(archive, "xl/workbook.xml",
                $"""<workbook xmlns="{Ns}" xmlns:r="{Rns}"><sheets>"""
                + """<sheet name="Bounded" sheetId="1" r:id="rId1"/>"""
                + """<sheet name="FullWidth" sheetId="2" r:id="rId2"/></sheets></workbook>""");

            Write(archive, "xl/styles.xml", Styles);
            Write(archive, "xl/worksheets/sheet1.xml", Sheet1);
            Write(archive, "xl/worksheets/sheet2.xml", Sheet2);
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

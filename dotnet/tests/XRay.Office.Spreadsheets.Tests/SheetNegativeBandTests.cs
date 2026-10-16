using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Formats;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A worksheet may state a <c>header</c> margin larger than its <c>top</c>, which asks for a
/// header band of negative height. The body still starts at the page margin.
/// </summary>
/// <remarks>
/// <para>
/// Found by the 24.2.7.2 re-check of <c>SheetPageDecoration.cs</c>'s band guard
/// (<c>probes/sheets-r55/audit_pagedecoration.py</c>), which varied the two margins together
/// through a stated band of 0.4 in down to 0.72 pt, zero and negative. At every non-negative band
/// 26.2.4.2 starts the body at the <c>top</c> margin; at a negative one it still does, and we
/// started it at the <c>header</c> margin — <strong>18 pt</strong> lower on the probe's fixture.
/// </para>
/// <para>
/// The band's own ink was already right: a band of zero or less draws nothing on either side, and
/// that half of the site's claim survived the re-check. What did not is "the reference draws the
/// footer at every stated band above zero" — it draws nothing at 0.72 or 1.44 pt of 8 pt text
/// either, on a threshold that scales with the point size. That part is recorded at the site and
/// deliberately not implemented; this test is only about the body's origin.
/// </para>
/// <para>
/// Two corpus worksheets state a negative band and <strong>neither renders differently</strong>
/// for this fix, so it has no corpus witness and this test is the only thing holding it.
/// </para>
/// </remarks>
public sealed class SheetNegativeBandTests
{
    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    private const string Rns = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    private static SheetPrintSetup Setup(double top, double header)
    {
        using DocumentSource source = DocumentSource.FromBytes(
            Package(top, header), "band.xlsx");
        using OoxmlSpreadsheetDocument document = XlsxReader.Read(source, DocumentFormat.Xlsx);

        return document.Sheets[0].Setup;
    }

    private static byte[] Package(double top, double header)
    {
        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml",
                """<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">"""
                + """<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>"""
                + """<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>"""
                + """<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>"""
                + "</Types>");
            Write(archive, "_rels/.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/officeDocument" Target="xl/workbook.xml"/>"""
                + "</Relationships>");
            Write(archive, "xl/_rels/workbook.xml.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/worksheet" Target="/xl/worksheets/sheet1.xml"/>"""
                + "</Relationships>");
            Write(archive, "xl/workbook.xml",
                $"""<workbook xmlns="{Ns}" xmlns:r="{Rns}"><sheets><sheet name="One" sheetId="1" r:id="rId1"/></sheets></workbook>""");
            Write(archive, "xl/worksheets/sheet1.xml",
                $"""<worksheet xmlns="{Ns}" xmlns:r="{Rns}"><sheetData><row r="1"><c r="A1" t="inlineStr"><is><t>Body</t></is></c></row></sheetData>"""
                + $"""<pageMargins left="0.7" right="0.7" top="{top.ToString(System.Globalization.CultureInfo.InvariantCulture)}" bottom="0.75" header="{header.ToString(System.Globalization.CultureInfo.InvariantCulture)}" footer="0.3"/>"""
                + """<headerFooter><oddHeader>&amp;CHeading</oddHeader></headerFooter></worksheet>""");
        }

        return buffer.ToArray();
    }

    private static void Write(ZipArchive archive, string name, string content)
    {
        ZipArchiveEntry entry = archive.CreateEntry(name);
        using StreamWriter writer = new(entry.Open(), new UTF8Encoding(false));
        writer.Write(content);
    }

    /// <summary>
    /// The ordinary case, which is the control: the band starts at the <c>header</c> margin and
    /// the body follows it.
    /// </summary>
    /// <remarks>
    /// The band itself is 0.45 in as stated plus whatever the measured line height exceeds the
    /// bare point size by — <c>SheetBandHeight</c>, which is a separate port with its own tests —
    /// so this asserts the band's <em>origin</em> and not its height. The origin is the one thing
    /// the clamp below governs.
    /// </remarks>
    [Fact]
    public void AnOrdinaryBandStartsAtTheHeaderMargin()
    {
        SheetPrintSetup setup = Setup(top: 0.75, header: 0.3);

        setup.TopMargin.Points.ShouldBe(21.6, 0.001);
        setup.HeaderHeight.ShouldBeGreaterThan(Length.Zero);
    }

    /// <summary>A band of exactly zero: the body is at the top margin and the band has no height.</summary>
    [Fact]
    public void AZeroBandPutsTheBodyAtTheTopMarginAndReservesNothing()
    {
        SheetPrintSetup setup = Setup(top: 0.75, header: 0.75);

        setup.HeaderHeight.ShouldBe(Length.Zero);
        setup.TopMargin.Points.ShouldBe(54, 0.001);
    }

    /// <summary>
    /// A negative band: the header margin is beyond the page margin, and the body does not follow
    /// it down.
    /// </summary>
    [Fact]
    public void ANegativeBandDoesNotPushTheBodyPastTheTopMargin()
    {
        SheetPrintSetup setup = Setup(top: 0.75, header: 1.0);

        setup.HeaderHeight.ShouldBe(Length.Zero);
        setup.TopMargin.Points.ShouldBe(54, 0.001);
    }
}

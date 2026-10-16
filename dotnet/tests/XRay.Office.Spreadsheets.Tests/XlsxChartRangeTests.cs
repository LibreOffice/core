using System.IO.Compression;
using System.Text;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What a chart's <c>c:f</c> resolves to against the workbook that holds it.
/// </summary>
/// <remarks>
/// <para>
/// The Calc half of the split described on <see cref="ChartRangeResolver"/>. These cases are about
/// which references are answered and which are declined, because a decline leaves the cached
/// points standing and a wrong answer replaces them — so the two failures are not symmetric and
/// the boundary is worth pinning.
/// </para>
/// <para>
/// The workbook is written here rather than kept as a fixture because what is under test is the
/// reference syntax, and a fixture can only carry one spelling of it.
/// </para>
/// </remarks>
public sealed class XlsxChartRangeTests
{
    private static XlsxChartRanges Ranges()
    {
        MemoryStream stream = new(Package(
            ("Literature Mapping",
                """
                <row r="4"><c r="B4"><v>7</v></c></row>
                <row r="5"><c r="B5"><v>6</v></c></row>
                <row r="6"><c r="B6"><v>35</v></c></row>
                """),
            ("O'Brien", """<row r="1"><c r="A1"><v>12</v></c></row>""")));

        XlsxFile file = XlsxFile.Open(stream);
        return new XlsxChartRanges(file, new XlsxSheetReader(file, []));
    }

    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    private const string Rns = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    /// <summary>A minimal SpreadsheetML package of one sheet per pair given.</summary>
    private static byte[] Package(params (string Name, string Rows)[] sheets)
    {
        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            StringBuilder types = new();
            StringBuilder relationships = new();
            StringBuilder entries = new();

            for (int at = 0; at < sheets.Length; at++)
            {
                string part = $"xl/worksheets/sheet{at + 1}.xml";

                types.Append(System.Globalization.CultureInfo.InvariantCulture,
                    $"""<Override PartName="/{part}" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>""");

                relationships.Append(System.Globalization.CultureInfo.InvariantCulture,
                    $"""<Relationship Id="rId{at + 1}" Type="{Rns}/worksheet" Target="/{part}"/>""");

                entries.Append(System.Globalization.CultureInfo.InvariantCulture,
                    $"""<sheet name="{sheets[at].Name.Replace("'", "&apos;", StringComparison.Ordinal)}" sheetId="{at + 1}" r:id="rId{at + 1}"/>""");

                Write(archive, part,
                    $"""<worksheet xmlns="{Ns}" xmlns:r="{Rns}"><sheetData>{sheets[at].Rows}</sheetData></worksheet>""");
            }

            Write(archive, "[Content_Types].xml",
                """<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">"""
                + """<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>"""
                + """<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>"""
                + types + "</Types>");

            Write(archive, "_rels/.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/officeDocument" Target="xl/workbook.xml"/>"""
                + "</Relationships>");

            Write(archive, "xl/_rels/workbook.xml.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + relationships + "</Relationships>");

            Write(archive, "xl/workbook.xml",
                $"""<workbook xmlns="{Ns}" xmlns:r="{Rns}"><sheets>""" + entries + "</sheets></workbook>");
        }

        return buffer.ToArray();
    }

    private static void Write(ZipArchive archive, string name, string content)
    {
        using Stream entry = archive.CreateEntry(name).Open();
        byte[] bytes = Encoding.UTF8.GetBytes(content);
        entry.Write(bytes, 0, bytes.Length);
    }

    /// <summary>A quoted sheet name and a dollar-decorated column range.</summary>
    [Fact]
    public void AQuotedSheetNameAndAColumnRangeResolve()
    {
        ChartRangeValues values =
            Ranges().Resolve("'Literature Mapping'!$B$4:$B$6").ShouldNotBeNull();

        values.Numbers.ShouldBe([7.0, 6.0, 35.0]);
        values.Text.ShouldBe(["7", "6", "35"]);
    }

    /// <summary>An apostrophe inside a quoted sheet name is doubled, and has to be undoubled.</summary>
    /// <remarks>
    /// Splitting on the first quote instead, or failing to undouble, looks for a sheet called
    /// <c>O''Brien</c> and finds nothing — which silently falls back to the cache rather than
    /// failing, so it would never surface as an error.
    /// </remarks>
    [Fact]
    public void AnApostropheInAQuotedSheetNameIsUndoubled()
    {
        Ranges().Resolve("'O''Brien'!$A$1").ShouldNotBeNull().Numbers.ShouldBe([12.0]);
    }

    /// <summary>A single cell is a range of one, not a parse failure.</summary>
    [Fact]
    public void ASingleCellReferenceResolves()
    {
        Ranges().Resolve("'Literature Mapping'!$B$6").ShouldNotBeNull().Numbers.ShouldBe([35.0]);
    }

    /// <summary>A reference to a sheet the workbook does not hold is declined.</summary>
    /// <remarks>
    /// Declined rather than answered with blanks: the cache is the better answer for an external
    /// or deleted sheet, and it is what the C++ reaches by catching.
    /// </remarks>
    [Fact]
    public void AnUnknownSheetIsDeclined()
    {
        Ranges().Resolve("Nowhere!$A$1:$A$3").ShouldBeNull();
    }

    /// <summary>A range naming only empty cells is declined too.</summary>
    /// <remarks>
    /// The same reasoning one step further in: a column of nulls is not a better answer than a
    /// cache, and it is what a mis-parsed reference would produce, so it is treated as a failure
    /// to resolve rather than as a result.
    /// </remarks>
    [Fact]
    public void ARangeOverEmptyCellsIsDeclined()
    {
        Ranges().Resolve("'Literature Mapping'!$Z$40:$Z$45").ShouldBeNull();
    }

    /// <summary>Shapes this does not parse are declined rather than guessed at.</summary>
    /// <remarks>
    /// A multi-area union, a bare defined name and an external-workbook reference. LibreOffice's
    /// formula parser handles the first two; guessing at them would substitute wrong numbers for
    /// stale ones, which is the worse of the two errors.
    /// </remarks>
    [Theory]
    [InlineData("('Literature Mapping'!$B$4,'Literature Mapping'!$B$6)")]
    [InlineData("MyRange")]
    [InlineData("[1]Sheet1!$A$1:$A$3")]
    [InlineData("'Literature Mapping'!$B:$B")]
    public void UnsupportedReferenceShapesAreDeclined(string formula)
    {
        Ranges().Resolve(formula).ShouldBeNull();
    }
}

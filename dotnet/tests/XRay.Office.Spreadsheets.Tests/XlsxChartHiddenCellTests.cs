using System.Globalization;
using System.IO.Compression;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A chart data range that names a cell in a hidden row or column.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScChart2DataSequence::BuildDataCache</c> asks <c>ColHidden</c> and <c>RowHidden</c> per
/// cell and <c>continue</c>s past a hidden one — the cell is dropped from the sequence rather
/// than blanked — unless the diagram says <c>IncludeHiddenCells</c>
/// (<c>sc/source/ui/unoobj/chart2uno.cxx</c>:2636-2646). The OOXML importer sets that property
/// to <c>!c:plotVisOnly</c> (<c>oox/source/drawingml/chart/chartspaceconverter.cxx</c>:264), and
/// <c>plotVisOnly</c> itself defaults to <c>!bMSO2007Document</c>
/// (<c>chartspacefragment.cxx</c>:130-131, <c>chartspacemodel.cxx</c>:30).
/// </para>
/// <para>
/// <strong>The expectations are the observables of authored variants rendered by 26.2.4.2</strong>,
/// not a reading of the C++. <c>053_Personal_asset_inventory_5446d84b.xlsx</c> names
/// <c>Assets!$H$24:$H$30</c> and <c>Assets!$I$24:$I$30</c>, a pivot table's output in columns H
/// and I, both <c>hidden="1"</c>: the reference draws six categories to a $300,000 axis. Unhide
/// the two columns and change nothing else, and it draws seven — <c>Grand Total</c> included — to
/// a $400,000 axis. Strip the chart part's cached <c>c:pt</c> instead, and it draws nothing at
/// all, which is what says the six are the cache. <c>probes/chart-resid-r75/</c>.
/// </para>
/// <para>
/// <strong>Two neighbouring rules answer "nothing survived" differently, and that is measured
/// rather than reasoned.</strong> A range wholly inside an Excel table's totals row resolves to
/// an <em>empty</em> sequence and the chart draws nothing
/// (<see cref="XlsxChartTotalsRowTests"/>); a range wholly hidden resolves to <em>null</em> and
/// the cached points stand. The two documents behind them, <c>029_Annual_budget</c> and
/// <c>053_Personal_asset_inventory</c>, render that way under 26.2.4.2.
/// </para>
/// </remarks>
public sealed class XlsxChartHiddenCellTests
{
    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    private const string Rns = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
    private const string Cns = "http://schemas.openxmlformats.org/drawingml/2006/chart";

    /// <summary>Five value rows in column B and column C, none of them hidden by default.</summary>
    private static string Rows(params int[] hidden)
    {
        StringBuilder rows = new();
        for (int at = 1; at <= 5; at++)
        {
            string flag = Array.IndexOf(hidden, at) >= 0 ? " hidden=\"1\"" : string.Empty;
            rows.Append(CultureInfo.InvariantCulture,
                $"""<row r="{at}"{flag}><c r="B{at}"><v>{at * 10}</v></c><c r="C{at}"><v>{at}</v></c></row>""");
        }

        return rows.ToString();
    }

    /// <summary>A resolver over one sheet with the rows and columns given hidden.</summary>
    private static XlsxChartRanges Ranges(int[]? hiddenRows = null, string? hiddenColumns = null)
    {
        MemoryStream stream = new(Package(Rows(hiddenRows ?? []), hiddenColumns));
        XlsxFile file = XlsxFile.Open(stream);
        return new XlsxChartRanges(file, new XlsxSheetReader(file, []));
    }

    // ---- the rule ------------------------------------------------------------------------

    /// <summary>The control: nothing hidden, every cell of the range read.</summary>
    /// <remarks>
    /// This is what fails first if the fixture ever stops writing the <c>hidden</c> flags, which
    /// would make every other case here pass by testing nothing.
    /// </remarks>
    [Fact]
    public void ARangeWithNothingHiddenReadsEveryCell()
    {
        Ranges().Resolve("'Data'!$B$1:$B$5").ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 30.0, 40.0, 50.0]);
    }

    /// <summary>A hidden row's cell is dropped from the sequence, not blanked.</summary>
    [Fact]
    public void AHiddenRowIsDroppedFromTheSequence()
    {
        Ranges(hiddenRows: [3]).Resolve("'Data'!$B$1:$B$5").ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 40.0, 50.0]);
    }

    /// <summary>A hidden column's cells go the same way.</summary>
    [Fact]
    public void AHiddenColumnIsDroppedFromTheSequence()
    {
        // B is column 2 and C column 3; hiding B leaves the C cell of each row.
        Ranges(hiddenColumns: "2:2").Resolve("'Data'!$B$1:$C$2").ShouldNotBeNull()
            .Numbers.ShouldBe([1.0, 2.0]);
    }

    /// <summary>
    /// A range every cell of which is hidden answers null, so the chart's cached points stand.
    /// </summary>
    /// <remarks>
    /// <c>053_Personal_asset_inventory</c>'s shape, and the one place this rule differs from the
    /// totals-row one beside it. Answering an empty sequence instead would draw that chart's plot
    /// blank where 26.2.4.2 draws six bars.
    /// </remarks>
    [Fact]
    public void ARangeWhollyHiddenResolvesToNullSoTheCacheStands()
    {
        Ranges(hiddenColumns: "2:2").Resolve("'Data'!$B$1:$B$5").ShouldBeNull();
    }

    /// <summary>
    /// A chart that states <c>c:plotVisOnly val="0"</c> reads its hidden cells like any other.
    /// </summary>
    /// <remarks>
    /// The corpus control: <c>055_Project_timeline_with_milestones</c> states it, all seventeen
    /// cells of its sequence are hidden, and its rendering must not move.
    /// </remarks>
    [Fact]
    public void AChartThatDoesNotPlotVisibleCellsOnlyKeepsTheHiddenOnes()
    {
        Ranges(hiddenRows: [3]).Resolve("'Data'!$B$1:$B$5", plotVisibleOnly: false)
            .ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 30.0, 40.0, 50.0]);
    }

    /// <summary>The bound resolver carries the flag it was built with.</summary>
    [Fact]
    public void TheBoundResolverCarriesThePlotVisibleOnlyFlag()
    {
        XlsxChartRanges ranges = Ranges(hiddenRows: [3]);

        ranges.Resolver(plotVisibleOnly: true)("'Data'!$B$1:$B$5").ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 40.0, 50.0]);

        ranges.Resolver(plotVisibleOnly: false)("'Data'!$B$1:$B$5").ShouldNotBeNull()
            .Numbers.ShouldBe([10.0, 20.0, 30.0, 40.0, 50.0]);
    }

    // ---- c:plotVisOnly -------------------------------------------------------------------

    /// <summary>An absent <c>c:plotVisOnly</c> means visible cells only, except on a 2007 file.</summary>
    /// <remarks>
    /// <c>rAttribs.getBool(XML_val, !bMSO2007Document)</c> — the default is the Office generation
    /// rather than a constant, which is the half of this attribute that is easy to get wrong.
    /// </remarks>
    [Theory]
    [InlineData(null, false, true)]
    [InlineData(null, true, false)]
    [InlineData("1", false, true)]
    [InlineData("1", true, true)]
    [InlineData("0", false, false)]
    [InlineData("0", true, false)]
    public void PlotVisOnlyDefaultsToTheOfficeGeneration(
        string? stated, bool office2007, bool expected)
    {
        string element = stated is null
            ? string.Empty
            : string.Create(CultureInfo.InvariantCulture, $"""<plotVisOnly xmlns="{Cns}" val="{stated}"/>""");

        XElement chartSpace = XElement.Parse(
            $"""<chartSpace xmlns="{Cns}"><chart>{element}</chart></chartSpace>""");

        DrawingChart.PlotsVisibleCellsOnly(chartSpace, office2007).ShouldBe(expected);
    }

    /// <summary>A missing chart part answers the generation's own default.</summary>
    [Fact]
    public void APlotVisOnlyReadOfNothingAnswersTheDefault()
    {
        DrawingChart.PlotsVisibleCellsOnly(null).ShouldBeTrue();
        DrawingChart.PlotsVisibleCellsOnly(null, office2007: true).ShouldBeFalse();
    }

    // ---- fixture -------------------------------------------------------------------------

    /// <summary>A minimal package of one sheet, with the hidden flags the cases need.</summary>
    private static byte[] Package(string rows, string? hiddenColumns)
    {
        string cols = string.Empty;
        if (hiddenColumns is not null)
        {
            string[] bounds = hiddenColumns.Split(':');
            cols = string.Create(CultureInfo.InvariantCulture,
                $"""<cols><col min="{bounds[0]}" max="{bounds[1]}" width="9" hidden="1"/></cols>""");
        }

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
                $"""<workbook xmlns="{Ns}" xmlns:r="{Rns}"><sheets><sheet name="Data" sheetId="1" r:id="rId1"/></sheets></workbook>""");

            Write(archive, "xl/worksheets/sheet1.xml",
                $"""<worksheet xmlns="{Ns}" xmlns:r="{Rns}">{cols}<sheetData>{rows}</sheetData></worksheet>""");
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

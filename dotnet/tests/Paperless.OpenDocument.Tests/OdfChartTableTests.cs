using System.Xml.Linq;
using Paperless.Core.Charts;
using Shouldly;

namespace Paperless.OpenDocument.Tests;

/// <summary>
/// How a chart's series find their column of the <c>local-table</c>, and which way round that
/// table is.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Both cases here fail silently and look like data.</strong> A series that cannot find
/// its column is nameless, so the legend it should have filled comes out empty — and an empty
/// legend takes no room, which moves the plot rectangle's right edge by the whole legend's width.
/// Measured over the round-tripped ODF corpus in <c>chart2/qa/extras/data/</c>, that was worth a
/// mean <strong>26 pt</strong> on 100 charts, more than every label reservation put together.
/// </para>
/// <para>
/// The corpus deck cannot show either of them: it charts <c>local-table.$B$2</c> onwards, so the
/// column letter and the table's own numbering agree by accident, and it writes its series down
/// the columns.
/// </para>
/// </remarks>
public class OdfChartTableTests
{
    private const string Chart = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";
    private const string Svg = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
    private const string Style = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
    private const string Table = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
    private const string Text = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
    private const string Office = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    private const string Draw = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";

    private static ChartPlot Read(string series, string table, string styles = "", string plotStyle = "")
    {
        string markup =
            $"""
             <office:document xmlns:office="{Office}" xmlns:chart="{Chart}" xmlns:svg="{Svg}"
                              xmlns:style="{Style}" xmlns:table="{Table}" xmlns:text="{Text}"
                              xmlns:draw="{Draw}">
               <office:automatic-styles>{styles}</office:automatic-styles>
               <office:body><office:chart>
                 <chart:chart chart:class="chart:bar" svg:width="12cm" svg:height="7cm">
                   <chart:plot-area {plotStyle}>
                     <chart:axis chart:dimension="x" chart:name="primary-x"/>
                     <chart:axis chart:dimension="y" chart:name="primary-y"/>
                     {series}
                   </chart:plot-area>
                   <table:table table:name="local-table">{table}</table:table>
                 </chart:chart>
               </office:chart></office:body>
             </office:document>
             """;

        XElement document = XElement.Parse(markup);
        XElement chart = document.Descendants(XName.Get("chart", Chart)).Single();

        return OdfChartPlot.Read(chart, new OdfChartStyles(document)).ShouldNotBeNull();
    }

    /// <summary>
    /// A series whose sheet range is in column D still finds the second column of a two-column
    /// local table, because the table says which range each of its columns holds.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Calc writes a <c>draw:g/svg:desc</c> inside one cell of every column naming the sheet
    /// range it was copied from — <c>SchXMLTableContext</c>'s column descriptions, which
    /// <c>SchXMLTableHelper::applyTableToInternalDataProvider</c> then matches each series'
    /// stated range against. Reading the column <em>letter</em> instead is right only when the
    /// charted range starts at column B.
    /// </para>
    /// <para>
    /// <c>chart2/qa/extras/data/ods/labelString.ods</c> is the case: it charts
    /// <c>Sheet1.D6:Sheet1.D8</c> against a local table two columns wide, so the letter said
    /// column four, the series came out nameless and the legend came out empty.
    /// </para>
    /// </remarks>
    [Fact]
    public void ASeriesFindsItsColumnByTheRangeTheTableStatesAndNotByTheColumnLetter()
    {
        ChartPlot plot = Read(
            """
            <chart:series chart:values-cell-range-address="Sheet1.D6:Sheet1.D8"
                          chart:class="chart:bar"/>
            """,
            """
            <table:table-row>
              <table:table-cell/>
              <table:table-cell office:value-type="string"><text:p>Column C</text:p>
                <draw:g><svg:desc>Sheet1.C6:Sheet1.C8</svg:desc></draw:g></table:table-cell>
              <table:table-cell office:value-type="string"><text:p>LabelName</text:p>
                <draw:g><svg:desc>Sheet1.D6:Sheet1.D8</svg:desc></draw:g></table:table-cell>
            </table:table-row>
            <table:table-row>
              <table:table-cell><text:p>1</text:p></table:table-cell>
              <table:table-cell office:value="7"><text:p>7</text:p></table:table-cell>
              <table:table-cell office:value="11"><text:p>11</text:p></table:table-cell>
            </table:table-row>
            <table:table-row>
              <table:table-cell><text:p>2</text:p></table:table-cell>
              <table:table-cell office:value="8"><text:p>8</text:p></table:table-cell>
              <table:table-cell office:value="12"><text:p>12</text:p></table:table-cell>
            </table:table-row>
            """);

        plot.Series.Count.ShouldBe(1);
        plot.Series[0].Name.ShouldBe("LabelName");
        plot.Series[0].Values.ShouldBe([11.0, 12.0]);
    }

    /// <summary>
    /// The column letter still works when the table states no ranges, which is every chart that
    /// carries its own data.
    /// </summary>
    /// <remarks>
    /// A chart with no live source names its own table — <c>local-table.$B$2:.$B$5</c> — and
    /// writes no <c>svg:desc</c> anywhere, so the fallback is what every deck goes through.
    /// </remarks>
    [Fact]
    public void ASeriesWithNoStatedRangesStillResolvesByTheColumnLetter()
    {
        ChartPlot plot = Read(
            """
            <chart:series chart:values-cell-range-address="local-table.$C$2:.$C$3"
                          chart:class="chart:bar"/>
            """,
            """
            <table:table-row>
              <table:table-cell/>
              <table:table-cell><text:p>North</text:p></table:table-cell>
              <table:table-cell><text:p>South</text:p></table:table-cell>
            </table:table-row>
            <table:table-row>
              <table:table-cell><text:p>Q1</text:p></table:table-cell>
              <table:table-cell office:value="1"><text:p>1</text:p></table:table-cell>
              <table:table-cell office:value="4"><text:p>4</text:p></table:table-cell>
            </table:table-row>
            <table:table-row>
              <table:table-cell><text:p>Q2</text:p></table:table-cell>
              <table:table-cell office:value="2"><text:p>2</text:p></table:table-cell>
              <table:table-cell office:value="5"><text:p>5</text:p></table:table-cell>
            </table:table-row>
            """);

        plot.Series[0].Name.ShouldBe("South");
        plot.Series[0].Values.ShouldBe([4.0, 5.0]);
    }

    /// <summary>
    /// <c>chart:series-source="rows"</c> transposes the table: the header row is the categories
    /// and each later row is one series, named by its own first cell.
    /// </summary>
    /// <remarks>
    /// Thirteen of the 107 ODF charts in <c>chart2/qa/extras/data/</c> state it. Reading one of
    /// them the upright way round turns the series names into <em>categories</em> and leaves
    /// every series nameless — a model in which every part is wrong and every part is plausible:
    /// <c>tdf146463.ods</c> came out with fourteen categories named after its fourteen companies
    /// and an axis of five unnamed dates.
    /// </remarks>
    [Fact]
    public void SeriesSourceRowsReadsTheTableTheOtherWayRound()
    {
        ChartPlot plot = Read(
            """
            <chart:series chart:values-cell-range-address="Sheet1.C3:Sheet1.D3"
                          chart:class="chart:bar"/>
            <chart:series chart:values-cell-range-address="Sheet1.C4:Sheet1.D4"
                          chart:class="chart:bar"/>
            """,
            """
            <table:table-row>
              <table:table-cell/>
              <table:table-cell office:value-type="string"><text:p>Jan</text:p></table:table-cell>
              <table:table-cell office:value-type="string"><text:p>Feb</text:p></table:table-cell>
            </table:table-row>
            <table:table-row>
              <table:table-cell office:value-type="string"><text:p>North</text:p>
                <draw:g><svg:desc>Sheet1.C3:Sheet1.D3</svg:desc></draw:g></table:table-cell>
              <table:table-cell office:value="1"><text:p>1</text:p></table:table-cell>
              <table:table-cell office:value="2"><text:p>2</text:p></table:table-cell>
            </table:table-row>
            <table:table-row>
              <table:table-cell office:value-type="string"><text:p>South</text:p>
                <draw:g><svg:desc>Sheet1.C4:Sheet1.D4</svg:desc></draw:g></table:table-cell>
              <table:table-cell office:value="3"><text:p>3</text:p></table:table-cell>
              <table:table-cell office:value="4"><text:p>4</text:p></table:table-cell>
            </table:table-row>
            """,
            """
            <style:style style:name="cs1" style:family="chart">
              <style:chart-properties chart:series-source="rows"/>
            </style:style>
            """,
            """chart:style-name="cs1" """);

        plot.Categories.ShouldBe(["Jan", "Feb"]);
        plot.Series.Count.ShouldBe(2);
        plot.Series[0].Name.ShouldBe("North");
        plot.Series[0].Values.ShouldBe([1.0, 2.0]);
        plot.Series[1].Name.ShouldBe("South");
        plot.Series[1].Values.ShouldBe([3.0, 4.0]);
    }
}

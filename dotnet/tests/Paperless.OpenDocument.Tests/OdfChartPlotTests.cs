using System.Xml.Linq;
using Paperless.Core.Charts;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.OpenDocument.Tests;

/// <summary>
/// What the ODF drawing reader makes of the three statements that are easy to miss because the
/// corpus deck does not make them.
/// </summary>
/// <remarks>
/// Every case here was found by reading LibreOffice's own <c>chart2/qa/extras/data/</c> rather
/// than the corpus, and each is invisible on the corpus file: it states the coordinate region in
/// the standardised namespace, states no data labels, and leaves both axes visible.
/// </remarks>
public class OdfChartPlotTests
{
    private const string Chart = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";
    private const string ChartOoo = "http://openoffice.org/2010/chart";
    private const string Svg = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
    private const string Style = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
    private const string Table = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
    private const string Text = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
    private const string Office = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    private const string Fo = "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0";
    private const string Draw = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
    private const string Number = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";

    /// <summary>A one-series bar chart with the given extra markup inside the plot area.</summary>
    private static string Document(string plotExtra, string styles = "", string plotStyle = "") =>
        $"""
         <office:document xmlns:office="{Office}" xmlns:chart="{Chart}" xmlns:chartooo="{ChartOoo}"
                          xmlns:svg="{Svg}" xmlns:style="{Style}" xmlns:table="{Table}"
                          xmlns:text="{Text}" xmlns:fo="{Fo}" xmlns:draw="{Draw}"
                          xmlns:number="{Number}">
           <office:automatic-styles>{styles}</office:automatic-styles>
           <office:body><office:chart>
             <chart:chart chart:class="chart:bar" svg:width="12cm" svg:height="7cm">
               <chart:plot-area {plotStyle}>
                 {plotExtra}
                 <chart:axis chart:dimension="x" chart:name="primary-x"/>
                 <chart:axis chart:dimension="y" chart:name="primary-y"/>
                 <chart:series chart:values-cell-range-address="local-table.$B$2:.$B$3"
                               chart:class="chart:bar"/>
               </chart:plot-area>
               <table:table table:name="local-table">
                 <table:table-row>
                   <table:table-cell/><table:table-cell><text:p>North</text:p></table:table-cell>
                 </table:table-row>
                 <table:table-row>
                   <table:table-cell><text:p>Q1</text:p></table:table-cell>
                   <table:table-cell office:value="120"><text:p>120</text:p></table:table-cell>
                 </table:table-row>
                 <table:table-row>
                   <table:table-cell><text:p>Q2</text:p></table:table-cell>
                   <table:table-cell office:value="95"><text:p>95</text:p></table:table-cell>
                 </table:table-row>
               </table:table>
             </chart:chart>
           </office:chart></office:body>
         </office:document>
         """;

    private static ChartPlot Read(string plotExtra, string styles = "", string plotStyle = "")
    {
        XElement document = XElement.Parse(Document(plotExtra, styles, plotStyle));
        XElement chart = document.Descendants(XName.Get("chart", Chart)).Single();

        return OdfChartPlot.Read(chart, new OdfChartStyles(document))
               ?? throw new InvalidOperationException("the reader found nothing to draw");
    }

    /// <summary>
    /// The coordinate region is read under either namespace, and the extension one is the commoner.
    /// </summary>
    /// <remarks>
    /// Counted over <c>chart2/qa/extras/data/</c>'s ODF chart documents: 24 write
    /// <c>chart:coordinate-region</c> and 47 write <c>chartooo:coordinate-region</c>. The corpus
    /// deck writes the first, so reading only that looked entirely correct while two ODF charts in
    /// three silently took the OOXML layout heuristic instead of the exact rectangle in the file.
    /// </remarks>
    [Theory]
    [InlineData("chart")]
    [InlineData("chartooo")]
    public void TheCoordinateRegionIsReadUnderEitherNamespace(string prefix)
    {
        ChartPlot plot = Read(
            $"""
             <{prefix}:coordinate-region svg:x="2.258cm" svg:y="1.594cm"
                                         svg:width="17.674cm" svg:height="8.538cm"/>
             """);

        plot.PlotArea.ShouldNotBeNull();
        plot.PlotArea!.Value.X.Mm100.ShouldBe(2258);
        plot.PlotArea!.Value.Width.Mm100.ShouldBe(17674);
    }

    /// <summary>
    /// A plot area's <c>chart:data-label-number</c> labels every series under it.
    /// </summary>
    /// <remarks>
    /// ODF folds OOXML's five flags into two attributes and states them on whichever style is
    /// nearest, which for a chart LibreOffice writes is usually the plot area's rather than each
    /// series'. Reading only the series' style finds nothing on the commonest file.
    /// </remarks>
    [Fact]
    public void APlotAreasDataLabelNumberReachesEverySeries()
    {
        ChartPlot plot = Read(
            string.Empty,
            styles:
            """
            <style:style style:name="pa" style:family="chart">
              <style:chart-properties chart:data-label-number="value" chart:data-label-text="true"/>
            </style:style>
            """,
            plotStyle: """chart:style-name="pa" """);

        plot.Series[0].Label.ShouldNotBeNull();
        plot.Series[0].Label!.ShowValue.ShouldBeTrue();
        plot.Series[0].Label!.ShowCategory.ShouldBeTrue();
        plot.Series[0].Label!.ShowPercent.ShouldBeFalse();
    }

    /// <summary>
    /// An axis' <c>style:data-style-name</c> becomes the format its ticks are written through.
    /// </summary>
    /// <remarks>
    /// The ODF half of the layering move: the axis names a data style, the data style compiles to
    /// a format code, and the code is rendered by the same engine an OOXML axis uses.
    /// </remarks>
    [Fact]
    public void AnAxisDataStyleBecomesTheTickFormat()
    {
        XElement document = XElement.Parse(Document(
            string.Empty,
            styles:
            """
            <number:percentage-style style:name="N11">
              <number:number number:decimal-places="1" number:min-decimal-places="1"
                             number:min-integer-digits="1"/>
              <number:text>%</number:text>
            </number:percentage-style>
            <style:style style:name="ax" style:family="chart" style:data-style-name="N11"/>
            """));

        XElement chart = document.Descendants(XName.Get("chart", Chart)).Single();
        XElement axis = chart.Descendants(XName.Get("axis", Chart))
            .Single(element => element.Attribute(XName.Get("dimension", Chart))?.Value == "y");

        axis.SetAttributeValue(XName.Get("style-name", Chart), "ax");

        ChartPlot plot = OdfChartPlot.Read(chart, new OdfChartStyles(document))!;

        ChartDataLabel.Write(0.05, plot.ValueFormat).ShouldBe("5.0%");
    }

    /// <summary>An axis whose style says <c>chart:visible="false"</c> is not drawn.</summary>
    [Fact]
    public void AnInvisibleAxisIsHidden()
    {
        XElement document = XElement.Parse(Document(
            string.Empty,
            styles:
            """
            <style:style style:name="hidden" style:family="chart">
              <style:chart-properties chart:visible="false"/>
            </style:style>
            """));

        XElement chart = document.Descendants(XName.Get("chart", Chart)).Single();
        XElement axis = chart.Descendants(XName.Get("axis", Chart))
            .Single(element => element.Attribute(XName.Get("dimension", Chart))?.Value == "x");

        axis.SetAttributeValue(XName.Get("style-name", Chart), "hidden");

        ChartPlot plot = OdfChartPlot.Read(chart, new OdfChartStyles(document))!;

        plot.CategoryAxisVisible.ShouldBeFalse();
        plot.ValueAxisVisible.ShouldBeTrue();
    }
}

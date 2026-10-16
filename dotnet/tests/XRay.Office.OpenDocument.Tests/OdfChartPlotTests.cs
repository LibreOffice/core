using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.OpenDocument.Tests;

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
    private static string Document(
        string plotExtra, string styles = "", string plotStyle = "", string seriesStyle = "") =>
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
                               chart:class="chart:bar" {seriesStyle}/>
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

    private static ChartPlot Read(
        string plotExtra, string styles = "", string plotStyle = "", string seriesStyle = "")
    {
        XElement document = XElement.Parse(Document(plotExtra, styles, plotStyle, seriesStyle));
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
    /// ODF spends four attributes on OOXML's five flags and states them on whichever style is
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
    /// <c>chart:data-label-series</c> and <c>chart:data-label-symbol</c> are the other two
    /// attributes that merge into <c>DataCaption</c>, and the label carries the series' name
    /// between the category's and the value.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both are <c>MID_FLAG_MERGE_PROPERTY</c> rows against <c>PROP_DataCaption</c> beside
    /// <c>data-label-number</c> and <c>data-label-text</c>
    /// (<c>xmloff/source/chart/PropertyMaps.cxx</c>:247-250), and
    /// <c>handleSpecialItem</c> sets <c>ChartDataCaption::DATA_SERIES</c> and <c>::SYMBOL</c>
    /// from them (<c>:975-991</c>). Neither was read, and this reader's own remark asserted the
    /// first did not exist — it is stated 25 times in 8 of the 307 converted <c>.ods</c>.
    /// </para>
    /// <para>
    /// The order is <c>VSeriesPlotter::createDataLabel</c>'s four-slot list — category, series,
    /// value, percentage (<c>chart2/source/view/charttypes/VSeriesPlotter.cxx</c>:566-596) — so
    /// the assertion is on the composed string and not only on the flags.
    /// </para>
    /// </remarks>
    [Fact]
    public void ASeriesNameAndItsLegendKeyAreReadFromTheirOwnTwoAttributes()
    {
        ChartPlot plot = Read(
            string.Empty,
            styles:
            """
            <style:style style:name="pa" style:family="chart">
              <style:chart-properties chart:data-label-number="value" chart:data-label-text="true"
                                      chart:data-label-series="true"
                                      chart:data-label-symbol="true"/>
            </style:style>
            """,
            plotStyle: """chart:style-name="pa" """);

        ChartDataLabel label = plot.Series[0].Label.ShouldNotBeNull();
        label.ShowSeries.ShouldBeTrue();
        label.ShowLegendKey.ShouldBeTrue();

        label.Compose("Q1", "North", 120, 215).ShouldBe("Q1; North; 120");
    }

    /// <summary>
    /// A series style that mentions neither attribute keeps what the plot area said; one stating
    /// <c>chart:data-label-series="false"</c> clears it.
    /// </summary>
    /// <remarks>
    /// The merge is into one bit field, so <c>SCH_XML_UNSETFLAG</c> on a false clears exactly the
    /// bit the plot area set and leaves the others (<c>PropertyMaps.cxx</c>:984-991). That is why
    /// the inheritance here has to be per flag rather than per style — a series style stating only
    /// <c>chart:label-position</c> must not silently drop the plot area's series name, which is
    /// the shape LibreOffice writes for the four <c>advanced_excel_pie</c> workbooks.
    /// </remarks>
    [Theory]
    [InlineData("", true)]
    [InlineData("""chart:data-label-series="false" """, false)]
    public void ASeriesStyleOverridesTheFlagOnlyWhenItStatesIt(string seriesExtra, bool expected)
    {
        ChartPlot plot = Read(
            string.Empty,
            styles:
            $"""
             <style:style style:name="pa" style:family="chart">
               <style:chart-properties chart:data-label-number="value"
                                       chart:data-label-series="true"/>
             </style:style>
             <style:style style:name="se" style:family="chart">
               <style:chart-properties chart:label-position="outside" {seriesExtra}/>
             </style:style>
             """,
            plotStyle: """chart:style-name="pa" """,
            seriesStyle: """chart:style-name="se" """);

        plot.Series[0].Label.ShouldNotBeNull();
        plot.Series[0].Label!.ShowSeries.ShouldBe(expected);
        plot.Series[0].Label!.ShowValue.ShouldBeTrue();
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

    /// <summary>
    /// An axis' line breaking is <c>text:line-break</c>, in the <em>text</em> namespace.
    /// </summary>
    /// <remarks>
    /// <para>
    /// It sits on a <c>style:chart-properties</c> beside its <c>chart:</c> neighbours and is
    /// mapped as <c>PROP_TextBreak</c> under <c>XML_NAMESPACE_TEXT</c>
    /// (<c>xmloff/source/chart/PropertyMaps.cxx</c>:188), so a reader looking for
    /// <c>chart:line-break</c> finds it in no file at all — the same trap
    /// <c>OdpSlideLayout.IsPrinted</c> records for <c>drawooo:display</c> and
    /// <c>OdfNamespaces.ChartExtension</c> for <c>coordinate-region</c>.
    /// </para>
    /// <para>
    /// It is not a detail of spacing. <c>canAutoAdjustLabelPlacement</c> refuses outright while
    /// line breaking is on (<c>chart2/source/view/axes/VCartesianAxis.cxx</c>:544-545), so an
    /// axis whose labels collide <em>wraps</em> them instead of turning them 45°. Read as false,
    /// every crowded ODF category axis took the rotation. 379 statements in 92 of the converted
    /// corpus's 302 <c>.odp</c>; on <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.odp</c> page 16 the
    /// nine hour-range labels go from turned to upright and land within 0.94 pt of 26.2.4.2's,
    /// and <c>N2_E_Maestroni_Swarm_COP.odp</c> goes from 340 alphanumeric characters clear of
    /// the reference to 84.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("true", true)]
    [InlineData("false", false)]
    public void LineBreakingIsStatedInTheTextNamespace(string stated, bool expected)
    {
        XElement document = XElement.Parse(Document(
            string.Empty,
            styles:
            $"""
             <style:style style:name="ax" style:family="chart">
               <style:chart-properties text:line-break="{stated}"/>
             </style:style>
             """));

        XElement chart = document.Descendants(XName.Get("chart", Chart)).Single();
        XElement axis = chart.Descendants(XName.Get("axis", Chart))
            .Single(element => element.Attribute(XName.Get("dimension", Chart))?.Value == "x");

        axis.SetAttributeValue(XName.Get("style-name", Chart), "ax");

        ChartPlot plot = OdfChartPlot.Read(chart, new OdfChartStyles(document))!;

        plot.CategoryAxisText.LineBreakAllowed.ShouldBe(expected);
    }

    /// <summary>An axis stating nothing takes chart2's own default, which is no line breaking.</summary>
    /// <remarks><c>Axis.cxx</c>:239 — <c>TextBreak</c> is false in the model's own defaults.</remarks>
    [Fact]
    public void AnAxisStatingNothingDoesNotBreakItsLabels()
    {
        ChartPlot plot = Read(string.Empty);

        plot.CategoryAxisText.LineBreakAllowed.ShouldBeFalse();
    }
}

using System.Xml.Linq;
using XRay.Office.Core.Charts;
using Shouldly;

namespace XRay.Office.OpenDocument.Tests;

/// <summary>
/// Which axis of an ODF chart is which, and which series is measured against which of them.
/// </summary>
/// <remarks>
/// <para>
/// <strong>ODF states an axis' index nowhere.</strong> <c>SchXMLAxisContext</c> counts how many
/// axes of the same <c>chart:dimension</c> have already been read and takes the count as this
/// axis' index (<c>xmloff/source/chart/SchXMLAxisContext.cxx</c>:266-274), so document order is
/// the whole rule: the first <c>chart:dimension="y"</c> is the primary value axis and the second
/// is the secondary. A series names one of them through <c>chart:attached-axis</c>, matched
/// against <c>chart:name</c> among the <em>y</em> axes alone, and belongs to the secondary
/// exactly when the axis it names has an index above zero
/// (<c>SchXMLSeries2Context.cxx</c>:333-345 and :388-394).
/// </para>
/// <para>
/// The same counting rule decides the category axis, and getting it wrong is what these cases
/// were written for: LibreOffice writes a combination chart's <c>secondary-x</c> — invisible,
/// carrying <c>chart:visible="false"</c> — <em>after</em> the primary one, so a reader that
/// assigns rather than defaults ends up reading the category labels off an axis that is not
/// drawn and draws none of them.
/// </para>
/// </remarks>
public class OdfChartAxesTests
{
    private const string Chart = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";
    private const string Svg = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
    private const string Style = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
    private const string Table = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
    private const string Text = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
    private const string Office = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    private const string Loext =
        "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0";

    /// <summary>
    /// The shape LibreOffice exports for a bar-and-line combination chart with two value axes:
    /// a visible primary x, an invisible secondary x, a primary y and a secondary y, with the
    /// line series attached to the second of those.
    /// </summary>
    private const string Combination =
        """
        <office:document xmlns:office="{OFFICE}" xmlns:chart="{CHART}" xmlns:svg="{SVG}"
                         xmlns:style="{STYLE}" xmlns:table="{TABLE}" xmlns:text="{TEXT}"
                         xmlns:loext="{LOEXT}">
          <office:automatic-styles>
            <style:style style:name="hidden" style:family="chart">
              <style:chart-properties chart:visible="false"/>
            </style:style>
            <style:style style:name="second" style:family="chart">
              <style:chart-properties chart:minimum="0" chart:maximum="45000"/>
            </style:style>
          </office:automatic-styles>
          <office:body><office:chart>
            <chart:chart chart:class="chart:bar" svg:width="12cm" svg:height="7cm">
              <chart:plot-area>
                <chart:axis chart:dimension="x" chart:name="primary-x">
                  <chart:categories table:cell-range-address="local-table.$A$2:.$A$3"/>
                </chart:axis>
                <chart:axis chart:dimension="x" chart:name="secondary-x" chart:style-name="hidden"/>
                <chart:axis chart:dimension="y" chart:name="primary-y"/>
                <chart:axis chart:dimension="y" chart:name="secondary-y" chart:style-name="second"/>
                <chart:series chart:attached-axis="primary-y" chart:class="chart:bar"
                              chart:values-cell-range-address="local-table.$B$2:.$B$3"/>
                <chart:series chart:attached-axis="secondary-y" chart:class="chart:line"
                              chart:values-cell-range-address="local-table.$C$2:.$C$3"/>
              </chart:plot-area>
              <table:table table:name="local-table">
                <table:table-row>
                  <table:table-cell/>
                  <table:table-cell><text:p>North</text:p></table:table-cell>
                  <table:table-cell><text:p>Margin</text:p></table:table-cell>
                </table:table-row>
                <table:table-row>
                  <table:table-cell><text:p>Q1</text:p></table:table-cell>
                  <table:table-cell office:value="32000"><text:p>32000</text:p></table:table-cell>
                  <table:table-cell office:value="12"><text:p>12</text:p></table:table-cell>
                </table:table-row>
                <table:table-row>
                  <table:table-cell><text:p>Q2</text:p></table:table-cell>
                  <table:table-cell office:value="27000"><text:p>27000</text:p></table:table-cell>
                  <table:table-cell office:value="15"><text:p>15</text:p></table:table-cell>
                </table:table-row>
              </table:table>
            </chart:chart>
          </office:chart></office:body>
        </office:document>
        """;

    /// <summary>
    /// A scatter chart, whose x axis is numeric: the points' own abscissae live in
    /// <c>chart:domain</c> and each point names itself in a <c>chart:data-label</c>.
    /// </summary>
    private const string Scatter =
        """
        <office:document xmlns:office="{OFFICE}" xmlns:chart="{CHART}" xmlns:svg="{SVG}"
                         xmlns:style="{STYLE}" xmlns:table="{TABLE}" xmlns:text="{TEXT}"
                         xmlns:loext="{LOEXT}">
          <office:automatic-styles/>
          <office:body><office:chart>
            <chart:chart chart:class="chart:scatter" svg:width="12cm" svg:height="7cm">
              <chart:plot-area>
                <chart:axis chart:dimension="x" chart:name="primary-x"/>
                <chart:axis chart:dimension="y" chart:name="primary-y"/>
                <chart:series chart:class="chart:scatter"
                              chart:values-cell-range-address="local-table.$B$2:.$B$3">
                  <chart:domain table:cell-range-address="local-table.$C$2:.$C$3"/>
                  <chart:data-point>
                    <chart:data-label><text:p><text:span>Alpha</text:span></text:p></chart:data-label>
                  </chart:data-point>
                  <chart:data-point>
                    <chart:data-label><text:p><text:span>Beta</text:span></text:p></chart:data-label>
                  </chart:data-point>
                </chart:series>
              </chart:plot-area>
              <table:table table:name="local-table">
                <table:table-row>
                  <table:table-cell/>
                  <table:table-cell><text:p>y</text:p></table:table-cell>
                  <table:table-cell><text:p>x</text:p></table:table-cell>
                </table:table-row>
                <table:table-row>
                  <table:table-cell/>
                  <table:table-cell office:value="7"><text:p>7</text:p></table:table-cell>
                  <table:table-cell office:value="4"><text:p>4</text:p></table:table-cell>
                </table:table-row>
                <table:table-row>
                  <table:table-cell/>
                  <table:table-cell office:value="5"><text:p>5</text:p></table:table-cell>
                  <table:table-cell office:value="9"><text:p>9</text:p></table:table-cell>
                </table:table-row>
              </table:table>
            </chart:chart>
          </office:chart></office:body>
        </office:document>
        """;

    private static ChartPlot Read(string markup)
    {
        XElement document = XElement.Parse(
            markup.Replace("{OFFICE}", Office, StringComparison.Ordinal)
                .Replace("{CHART}", Chart, StringComparison.Ordinal)
                .Replace("{SVG}", Svg, StringComparison.Ordinal)
                .Replace("{STYLE}", Style, StringComparison.Ordinal)
                .Replace("{TABLE}", Table, StringComparison.Ordinal)
                .Replace("{TEXT}", Text, StringComparison.Ordinal)
                .Replace("{LOEXT}", Loext, StringComparison.Ordinal));

        XElement chart = document.Descendants(XName.Get("chart", Chart)).Single();

        return OdfChartPlot.Read(chart, new OdfChartStyles(document))
               ?? throw new InvalidOperationException("the reader found nothing to draw");
    }

    /// <summary>
    /// The category axis is the <em>first</em> <c>chart:dimension="x"</c>, not the last.
    /// </summary>
    /// <remarks>
    /// The combination chart's second x axis carries <c>chart:visible="false"</c>, so a reader
    /// that keeps the last one reports the category axis as hidden and draws no category labels
    /// at all — which is exactly what <c>combo_bar_line_chart.odp</c> showed: 166 alphanumeric
    /// characters against the reference's 219, the missing 53 being <c>Q1</c>…<c>Q4</c> and the
    /// secondary axis' own ticks.
    /// </remarks>
    [Fact]
    public void TheCategoryAxisIsTheFirstOfTheTwoAcross()
    {
        ChartPlot plot = Read(Combination);

        plot.CategoryAxisVisible.ShouldBeTrue();
        plot.Categories.ShouldBe(["Q1", "Q2"]);
    }

    /// <summary>
    /// The second <c>chart:dimension="y"</c> is the secondary value axis and carries its own scale.
    /// </summary>
    [Fact]
    public void TheSecondValueAxisIsTheSecondaryOne()
    {
        ChartPlot plot = Read(Combination);

        plot.SecondaryValueScale.ShouldNotBeNull();
        plot.SecondaryValueScale!.Value.Minimum.ShouldBe(0);
        plot.SecondaryValueScale!.Value.Maximum.ShouldBe(45000);

        // The primary keeps the scale its own style states, which here is none at all.
        plot.ValueScale.Minimum.ShouldBeNull();
        plot.ValueScale.Maximum.ShouldBeNull();
    }

    /// <summary>
    /// <c>chart:attached-axis</c> puts a series on the secondary axis by naming it.
    /// </summary>
    [Fact]
    public void AttachedAxisNamesWhichScaleASeriesIsMeasuredAgainst()
    {
        ChartPlot plot = Read(Combination);

        plot.Series.Count.ShouldBe(2);
        plot.Series[0].AxisIndex.ShouldBe(0);
        plot.Series[1].AxisIndex.ShouldBe(1);
        plot.HasSecondaryAxis.ShouldBeTrue();
    }

    /// <summary>
    /// A chart whose axes carry no <c>chart:name</c> puts every series on the primary.
    /// </summary>
    /// <remarks>
    /// The name is the only link ODF states, so an unnamed secondary axis can be attached to by
    /// nothing — <c>SchXMLSeries2Context</c> compares the attribute against each y axis' name and
    /// leaves <c>mpAttachedAxis</c> null when none matches, which is the primary.
    /// </remarks>
    [Fact]
    public void AnUnnamedSecondAxisAttractsNoSeries()
    {
        ChartPlot plot = Read(
            Combination.Replace(" chart:name=\"secondary-y\"", string.Empty, StringComparison.Ordinal));

        plot.Series[1].AxisIndex.ShouldBe(0);
        plot.HasSecondaryAxis.ShouldBeFalse();
    }

    /// <summary>
    /// A scatter chart's <c>chart:domain</c> is its X sequence, and its x axis is a value axis.
    /// </summary>
    /// <remarks>
    /// Both dimensions are numeric, so ODF spells the x axis exactly as a category axis is spelt
    /// and the chart's own <c>chart:class</c> is what tells them apart. Without the domain,
    /// <c>ChartLayout.DomainScaleOf</c> finds no numbers and the axis draws no labels — which is
    /// what <c>scatter_chart.odp</c> showed, 83 characters against 105.
    /// </remarks>
    [Fact]
    public void AScatterSeriesTakesItsAbscissaeFromChartDomain()
    {
        ChartPlot plot = Read(Scatter);

        plot.Series.Count.ShouldBe(1);
        plot.Series[0].XValues.ShouldBe([4.0, 9.0]);
    }

    /// <summary>
    /// A category chart has no domain, so nothing draws a second numeric axis under it.
    /// </summary>
    [Fact]
    public void ACategoryChartHasNoDomainSequence()
    {
        ChartPlot plot = Read(Combination);

        plot.Series[0].XValues.ShouldBeNull();
        plot.DomainFormat.ShouldBeNull();
    }

    /// <summary>
    /// A <c>chart:data-point</c> may state its label's words itself, and they replace the fields.
    /// </summary>
    /// <remarks>
    /// <c>SchXMLSeries2Context</c> turns the <c>chart:data-label</c>'s paragraphs into
    /// <c>CustomLabelFields</c> of type <c>TEXT</c> and sets <c>DataCaption</c> to <c>CUSTOM</c>
    /// (<c>SchXMLSeries2Context.cxx</c>:1245-1290), so the stated string is the whole label —
    /// which is what <see cref="ChartDataLabel.Text"/> means. It is the only way ODF puts a
    /// point's own words on a chart.
    /// </remarks>
    [Fact]
    public void APointsOwnDataLabelIsTheWholeOfItsText()
    {
        ChartPlot plot = Read(Scatter);

        ChartDataLabel? first = plot.Series[0].PointLabels?[0];
        ChartDataLabel? second = plot.Series[0].PointLabels?[1];

        first.ShouldNotBeNull();
        first!.Text.ShouldBe("Alpha");
        first.Draws.ShouldBeTrue();
        second!.Text.ShouldBe("Beta");
    }
}

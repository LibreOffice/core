using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Three statements a chart part makes about the whole chart rather than about one element: which
/// axes set the categories come from, what weight its text falls back to, and what an unresolved
/// <c>CELLRANGE</c> field draws.
/// </summary>
/// <remarks>
/// Each is invisible in the markup of the element it changes — the categories are read from a
/// plot group that is not the first one, the weight from an element after the plot area, and the
/// field's answer from an extension list on the series — which is why all three produced pages
/// that were entirely plausible and wrong.
/// </remarks>
public sealed class DrawingChartAxesSetTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string C15 = "http://schemas.microsoft.com/office/drawing/2012/chart";

    /// <summary>The extension list uri the 2012 chart extensions are carried under.</summary>
    private const string Chart2012Uri = "{CE6537A1-D6FC-4f65-9D91-7224C49458BB}";

    private static ChartPlot Read(string chart, string tail = "")
        => DrawingChartPlot.Read(XElement.Parse(
               $"""
                <c:chartSpace xmlns:c="{C}" xmlns:a="{A}" xmlns:c15="{C15}">
                  <c:chart>{chart}</c:chart>{tail}
                </c:chartSpace>
                """))
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>
    /// A combination chart's categories come from the group on the primary axes set, not from the
    /// group written first.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>if (nAxesSetIdx == 0) aScaleData.Categories = rTypeGroups.front()-&gt;createCategorySequence()</c>
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx</c>:282-290) takes them from one axes set
    /// only, and <c>plotareaconverter.cxx</c>:466-468 decides which set that is: for a
    /// <em>combined</em> chart — exactly two type groups of different kinds — it is the set whose
    /// value axis comes first in the plot area's own element order, whichever group wrote it.
    /// </para>
    /// <para>
    /// Here the <c>c:barChart</c> is written first and names axes 3/4, and the <c>c:lineChart</c>
    /// names 1/2 whose value axis is the first <c>c:valAx</c> in the part — so the dates win over
    /// the names. Three corpus chart parts in three documents are this shape;
    /// <c>055_Project_timeline_with_milestones</c> is where it was found, drawing thirteen
    /// milestone names on an axis the reference fills with dates and putting a name into every
    /// data label the reference gives a date.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheCategoriesComeFromThePrimaryAxesSet()
    {
        ChartPlot plot = Read(Combination());

        plot.Categories.ShouldBe(["1 Apr", "2 Apr"]);
    }

    /// <summary>
    /// The same part with the two <c>c:valAx</c> swapped puts the bar group's categories back.
    /// </summary>
    /// <remarks>
    /// The control on the case above, and it moves one thing: with the bar group's value axis
    /// written first, <c>nStartAxesSetIdx</c> is 0, its axes set is the primary one, and its
    /// names are what the diagram shows. Nothing else in the part changes.
    /// </remarks>
    [Fact]
    public void SwappingWhichValueAxisComesFirstSwapsTheCategories()
    {
        ChartPlot plot = Read(Combination(barAxisFirst: true));

        plot.Categories.ShouldBe(["Start", "Finish"]);
    }

    /// <summary>A one-group chart takes its categories from that group, as it always has.</summary>
    [Fact]
    public void AChartWithOneGroupIsUnchanged()
    {
        ChartPlot plot = Read(
            $"""
             <c:plotArea>
               <c:barChart>{Series("Start", "Finish", 20, 10)}<c:axId val="1"/><c:axId val="2"/></c:barChart>
               <c:catAx><c:axId val="1"/></c:catAx>
               <c:valAx><c:axId val="2"/></c:valAx>
             </c:plotArea>
             """);

        plot.Categories.ShouldBe(["Start", "Finish"]);
    }

    /// <summary>
    /// The chart space's own <c>c:txPr</c> supplies the weight every piece of text falls back to.
    /// </summary>
    /// <remarks>
    /// <c>TextFormatter</c> seeds its automatic properties from the auto-text table — a title
    /// bold, axis labels regular — and then <c>assignUsed</c>s the chart space's <c>a:defRPr</c>
    /// over them (<c>oox/source/drawingml/chart/objectformatter.cxx</c>:906-929, with the text
    /// body handed in at <c>:950</c>). So a global <c>b="1"</c> makes an axis label bold and a
    /// global <c>b="0"</c> makes the title regular; 17 corpus chart parts in 8 documents state one
    /// and both directions occur.
    /// </remarks>
    [Fact]
    public void TheChartSpacesOwnWeightIsWhatUnstatedTextFallsBackTo()
    {
        ChartPlot bold = Read(Plain(), Weight("1"));
        bold.IsLabelBold.ShouldBeTrue();
        bold.IsTitleBold.ShouldBeTrue();

        ChartPlot regular = Read(Plain(), Weight("0"));
        regular.IsLabelBold.ShouldBeFalse();
        regular.IsTitleBold.ShouldBeFalse();

        ChartPlot silent = Read(Plain());
        silent.IsLabelBold.ShouldBeFalse();
        silent.IsTitleBold.ShouldBeTrue();
    }

    /// <summary>An axis that states its own weight is not overruled by the chart space's.</summary>
    [Fact]
    public void AnAxisStatingItsOwnWeightKeepsIt()
    {
        ChartPlot plot = Read(
            $"""
             <c:plotArea>
               <c:barChart>{Series("Start", "Finish", 20, 10)}<c:axId val="1"/><c:axId val="2"/></c:barChart>
               <c:catAx><c:axId val="1"/><c:txPr><a:p><a:pPr><a:defRPr b="0"/></a:pPr></a:p></c:txPr></c:catAx>
               <c:valAx><c:axId val="2"/></c:valAx>
             </c:plotArea>
             """,
            Weight("1"));

        plot.IsLabelBold.ShouldBeFalse();
    }

    /// <summary>
    /// A <c>CELLRANGE</c> field with no cached string for its point draws nothing at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Never the placeholder. With a <c>c15:datalabelsRange</c> present LibreOffice substitutes
    /// the empty string where the cache has no entry for the point
    /// (<c>oaLabelText.value_or("")</c>, <c>seriesconverter.cxx</c>:366-410) and with none present
    /// it leaves <c>setDataLabelsRange</c> false, whereupon <c>VSeriesPlotter</c> writes an empty
    /// string for the field (<c>VSeriesPlotter.cxx</c>:535-541). Neither path can reach the
    /// <c>a:t</c>, which is a localised placeholder such as <c>[CELLRANGE]</c>.
    /// </para>
    /// <para>
    /// Measured on <c>055_Project_timeline_with_milestones</c>: two of its thirteen labels have
    /// no cache entry and we printed <c>[CELLRANGE]</c> over the timeline for both.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnUnresolvedCellRangeFieldDrawsNothing()
    {
        ChartPlot plot = Read(CellRangeLabels());

        plot.Series.Count.ShouldBe(1);
        Compose(plot, 0).ShouldBe("Alpha");
        Compose(plot, 1).ShouldBe("");
    }

    /// <summary>The point the cache does name still gets its string.</summary>
    /// <remarks>
    /// The control: dropping the placeholder must not drop the value it stands for, which is the
    /// half a previous round closed.
    /// </remarks>
    [Fact]
    public void AResolvedCellRangeFieldDrawsItsCachedString()
    {
        ChartPlot plot = Read(CellRangeLabels());

        plot.Series[0].LabelAt(0).ShouldNotBeNull();
        Compose(plot, 0).ShouldNotBe("[CELLRANGE]");
    }

    private static string Compose(ChartPlot plot, int point)
        => plot.Series[0].LabelAt(point)!.Compose(
            plot.Categories.Count > point ? plot.Categories[point] : null,
            plot.Series[0].Name,
            plot.Series[0].Values[point] ?? 0.0,
            plot.Series[0].Total()) ?? "";

    private static string Weight(string bold)
        => $"""<c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr b="{bold}"/></a:pPr></a:p></c:txPr>""";

    private static string Plain() =>
        $"""
         <c:title><c:tx><c:rich><a:p><a:r><a:t>Totals</a:t></a:r></a:p></c:rich></c:tx></c:title>
         <c:plotArea>
           <c:barChart>{Series("Start", "Finish", 20, 10)}<c:axId val="1"/><c:axId val="2"/></c:barChart>
           <c:catAx><c:axId val="1"/></c:catAx>
           <c:valAx><c:axId val="2"/></c:valAx>
         </c:plotArea>
         """;

    /// <summary>A bar group and a line group on two axis pairs, with the axes in either order.</summary>
    private static string Combination(bool barAxisFirst = false)
    {
        string axes = barAxisFirst
            ? """<c:valAx><c:axId val="4"/></c:valAx><c:valAx><c:axId val="2"/></c:valAx>"""
            : """<c:valAx><c:axId val="2"/></c:valAx><c:valAx><c:axId val="4"/></c:valAx>""";

        return $"""
                <c:plotArea>
                  <c:barChart>{Series("Start", "Finish", 20, 10)}<c:axId val="3"/><c:axId val="4"/></c:barChart>
                  <c:lineChart>{Series("1 Apr", "2 Apr", 1, 2)}<c:axId val="1"/><c:axId val="2"/></c:lineChart>
                  <c:dateAx><c:axId val="1"/></c:dateAx>
                  {axes}
                  <c:catAx><c:axId val="3"/><c:delete val="1"/></c:catAx>
                </c:plotArea>
                """;
    }

    private static string Series(string first, string second, double one, double two) =>
        $"""
         <c:ser>
           <c:cat><c:strRef><c:strCache><c:ptCount val="2"/>
             <c:pt idx="0"><c:v>{first}</c:v></c:pt><c:pt idx="1"><c:v>{second}</c:v></c:pt>
           </c:strCache></c:strRef></c:cat>
           <c:val><c:numRef><c:numCache><c:ptCount val="2"/>
             <c:pt idx="0"><c:v>{one}</c:v></c:pt><c:pt idx="1"><c:v>{two}</c:v></c:pt>
           </c:numCache></c:numRef></c:val>
         </c:ser>
         """;

    /// <summary>
    /// Two points whose labels are one <c>CELLRANGE</c> field, with a cache naming only the first.
    /// </summary>
    private static string CellRangeLabels() =>
        $"""
         <c:plotArea>
           <c:barChart>
             <c:ser>
               <c:dLbls>
                 {CellRangeLabel(0)}
                 {CellRangeLabel(1)}
                 <c:showVal val="0"/><c:showCatName val="0"/><c:showSerName val="0"/>
                 <c:showPercent val="0"/><c:showLegendKey val="0"/>
               </c:dLbls>
               <c:cat><c:strRef><c:strCache><c:ptCount val="2"/>
                 <c:pt idx="0"><c:v>One</c:v></c:pt><c:pt idx="1"><c:v>Two</c:v></c:pt>
               </c:strCache></c:strRef></c:cat>
               <c:val><c:numRef><c:numCache><c:ptCount val="2"/>
                 <c:pt idx="0"><c:v>20</c:v></c:pt><c:pt idx="1"><c:v>10</c:v></c:pt>
               </c:numCache></c:numRef></c:val>
               <c:extLst><c:ext uri="{Chart2012Uri}">
                 <c15:datalabelsRange>
                   <c15:f>Sheet1!$E$2:$E$3</c15:f>
                   <c15:dlblRangeCache>
                     <c:ptCount val="2"/>
                     <c:pt idx="0"><c:v>Alpha</c:v></c:pt>
                   </c15:dlblRangeCache>
                 </c15:datalabelsRange>
               </c:ext></c:extLst>
             </c:ser>
             <c:axId val="1"/><c:axId val="2"/>
           </c:barChart>
           <c:catAx><c:axId val="1"/></c:catAx>
           <c:valAx><c:axId val="2"/></c:valAx>
         </c:plotArea>
         """;

    private static string CellRangeLabel(int index) =>
        $"""
         <c:dLbl>
           <c:idx val="{index}"/>
           <c:tx><c:rich><a:p>
             <a:fld id="{Guid.Empty}" type="CELLRANGE">
               <a:t>[CELLRANGE]</a:t>
             </a:fld>
           </a:p></c:rich></c:tx>
           <c:showVal val="0"/><c:showCatName val="0"/><c:showSerName val="0"/>
           <c:showPercent val="0"/><c:showLegendKey val="0"/>
           <c:extLst><c:ext uri="{Chart2012Uri}">
             <c15:showDataLabelsRange val="1"/>
           </c:ext></c:extLst>
         </c:dLbl>
         """;
}

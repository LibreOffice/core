using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A multi-level category, and the chart types a percentage belongs to.
/// </summary>
/// <remarks>
/// <para>
/// Both defects here were found the same way and neither by a metric: two fresh reviewers, each
/// given one rendered page and forbidden to read any project file or run any command, transcribed
/// the halves of <c>027_Unit_Circle_Chart_Graphical_Chart</c> and
/// <c>028_Unit_Circle_Chart_Optimized_Graph</c> separately and reported that the reference's labels
/// and legend read <c>Branch 1 Stem 2 Leaf 5</c> where ours read <c>Branch 1</c>, and that
/// <c>028</c>'s reference puts a percentage on every label while ours carries none — <em>while
/// <c>027</c>'s ours does draw one</em>. That split is the whole diagnosis: one defect is in the
/// category reader and the other is in the chart-type test, and no single-document reading could
/// have separated them.
/// </para>
/// <para>
/// Measured on LibreOffice 26.2.4.2: <c>027</c> went 261 words against 378 to 376, and <c>029</c>
/// 107 against 114 to 111 — both from <c>words</c> to <c>match</c>.
/// </para>
/// </remarks>
public class DrawingChartCategoryTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Read(string inner)
        => DrawingChartPlot.Read(XElement.Parse(
               $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{inner}</c:chart></c:chartSpace>"))
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>Two points of values, so a series exists to hang a category on.</summary>
    private const string Values =
        """
        <c:val><c:numRef><c:numCache><c:ptCount val="2"/>
          <c:pt idx="0"><c:v>3</c:v></c:pt><c:pt idx="1"><c:v>17</c:v></c:pt>
        </c:numCache></c:numRef></c:val>
        """;

    /// <summary>Excel's own order: the innermost level first.</summary>
    private const string Levels =
        """
        <c:cat><c:multiLvlStrRef><c:f>Sheet1!$A$2:$C$3</c:f><c:multiLvlStrCache>
          <c:ptCount val="2"/>
          <c:lvl><c:pt idx="0"><c:v>Leaf 1</c:v></c:pt><c:pt idx="1"><c:v>Leaf 2</c:v></c:pt></c:lvl>
          <c:lvl><c:pt idx="0"><c:v>Stem 1</c:v></c:pt></c:lvl>
          <c:lvl><c:pt idx="0"><c:v>Branch 1</c:v></c:pt><c:pt idx="1"><c:v>Branch 2</c:v></c:pt></c:lvl>
        </c:multiLvlStrCache></c:multiLvlStrRef></c:cat>
        """;

    /// <summary>
    /// Every level of a multi-level category contributes, outermost first.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Each <c>c:lvl</c> numbers its own points from zero.</strong> The reader used to walk
    /// every <c>c:pt</c> descendant of the cache and assign by <c>@idx</c>, so each level
    /// overwrote the one before it and the survivor was the last written — the outermost. A
    /// three-level category came out as <c>Branch 1</c> in every label and every legend entry.
    /// </para>
    /// <para>
    /// The join is a space, outermost level inwards, skipping the levels that state nothing at an
    /// index: <c>lcl_getExplicitSimpleCategories</c>,
    /// <c>chart2/source/tools/ExplicitCategoriesProvider.cxx:376-395</c>, which is what
    /// <c>getSimpleCategories</c> hands to the legend and to every data label. The second point
    /// here is the ragged case — it states no middle level, and the reference draws
    /// <c>Branch 2 Leaf 2</c> rather than leaving a double space or dropping the level below it.
    /// </para>
    /// </remarks>
    [Fact]
    public void EveryLevelOfACategoryContributesOutermostFirst()
    {
        ChartPlot plot = Read($"<c:plotArea><c:pieChart><c:ser>{Values}{Levels}</c:ser></c:pieChart></c:plotArea>");

        plot.Categories.Count.ShouldBe(2);
        plot.Categories[0].ShouldBe("Branch 1 Stem 1 Leaf 1");
        plot.Categories[1].ShouldBe("Branch 2 Leaf 2");
    }

    /// <summary>The count comes from <c>c:ptCount</c>, so a sparse level does not shift anything.</summary>
    [Fact]
    public void ASparseLevelKeepsEveryPointAtItsOwnIndex()
    {
        ChartPlot plot = Read(
            "<c:plotArea><c:pieChart><c:ser>"
            + """
              <c:val><c:numRef><c:numCache><c:ptCount val="3"/>
                <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="2"><c:v>2</c:v></c:pt>
              </c:numCache></c:numRef></c:val>
              <c:cat><c:multiLvlStrRef><c:multiLvlStrCache>
                <c:ptCount val="3"/>
                <c:lvl><c:pt idx="0"><c:v>a</c:v></c:pt><c:pt idx="2"><c:v>c</c:v></c:pt></c:lvl>
                <c:lvl><c:pt idx="2"><c:v>Z</c:v></c:pt></c:lvl>
              </c:multiLvlStrCache></c:multiLvlStrRef></c:cat>
              """
            + "</c:ser></c:pieChart></c:plotArea>");

        plot.Categories.Count.ShouldBe(3);
        plot.Categories[0].ShouldBe("a");
        plot.Categories[1].ShouldBeNull("the file states nothing at index 1");
        plot.Categories[2].ShouldBe("Z c");
    }

    /// <summary>
    /// A bar-of-pie is a pie for the purpose of <c>c:showPercent</c>.
    /// </summary>
    /// <remarks>
    /// <c>bShowPercent</c> is ANDed with <c>meTypeCategory == TYPECATEGORY_PIE</c>
    /// (<c>oox/source/drawingml/chart/seriesconverter.cxx:140</c>), and the type table puts
    /// <c>TYPEID_OFPIE</c> in that category beside <c>TYPEID_PIE</c> and <c>TYPEID_DOUGHNUT</c>
    /// (<c>typegroupconverter.cxx:103-105</c>). Gating on the pie kind alone cost every label of
    /// <c>028_Unit_Circle_Chart_Optimized_Graph</c> and
    /// <c>029_Unit_Circle_Chart_Pie_Theme</c> its percentage.
    /// </remarks>
    [Theory]
    [InlineData("pieChart", true)]
    [InlineData("ofPieChart", true)]
    [InlineData("barChart", false)]
    [InlineData("lineChart", false)]
    public void APercentageIsAPiesBusinessAndABarOfPiesToo(string chart, bool expected)
    {
        ChartPlot plot = Read(
            $"<c:plotArea><c:{chart}><c:ser>{Values}"
            + "<c:dLbls><c:showVal val=\"0\"/><c:showCatName val=\"1\"/><c:showPercent val=\"1\"/>"
            + "<c:showSerName val=\"0\"/><c:showLegendKey val=\"0\"/><c:showBubbleSize val=\"0\"/></c:dLbls>"
            + $"</c:ser></c:{chart}></c:plotArea>");

        plot.Series.ShouldHaveSingleItem().Label.ShouldNotBeNull().ShowPercent.ShouldBe(expected);
    }

    /// <summary>
    /// A percentage shown without a value goes on its own line.
    /// </summary>
    /// <remarks>
    /// <c>seriesconverter.cxx:168-172</c>. Pinned here because the separator is what
    /// <c>FrameChart</c> now breaks the label on, and a separator quietly reverting to <c>"; "</c>
    /// would make that code unreachable without failing anything else.
    /// </remarks>
    [Fact]
    public void APercentageWithoutAValueIsWrittenOnItsOwnLine()
    {
        ChartPlot plot = Read(
            $"<c:plotArea><c:ofPieChart><c:ser>{Values}{Levels}"
            + "<c:dLbls><c:showVal val=\"0\"/><c:showCatName val=\"1\"/><c:showPercent val=\"1\"/>"
            + "<c:showSerName val=\"0\"/></c:dLbls>"
            + "</c:ser></c:ofPieChart></c:plotArea>");

        ChartDataLabel label = plot.Series.ShouldHaveSingleItem().Label.ShouldNotBeNull();
        label.Separator.ShouldBe("\n");
        label.Compose(plot.Categories[0], null, 3.0, 20.0).ShouldBe("Branch 1 Stem 1 Leaf 1\n15%");
    }
}

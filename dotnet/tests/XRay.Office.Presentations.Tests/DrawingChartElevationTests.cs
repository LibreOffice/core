using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What a <c>c:pie3DChart</c> takes from <c>c:view3D</c>, and what it deliberately does not.
/// </summary>
/// <remarks>
/// <para>
/// <c>View3DConverter::convertFromModel</c>'s pie branch
/// (<c>oox/source/drawingml/chart/plotareaconverter.cxx</c>:273-278, this tree) reads
/// <c>c:rotX</c>, clamps it to <c>[0, 90]</c> and defaults it to 15. It reads nothing else that
/// reaches the drawn shape: <c>c:rotY</c> becomes the first slice's angle, which nothing here
/// models, and <c>c:depthPercent</c> is passed on by no branch at all.
/// </para>
/// <para>
/// <strong>That last one is measured rather than inferred.</strong>
/// <c>probes/pie3d-r187</c> renders a corpus document at depths 20, 50, 100 and 200 across nine
/// elevations and 26.2.4.2's geometry is identical to the hundredth of a point at every one of
/// the thirty-six.
/// </para>
/// </remarks>
public class DrawingChartElevationTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Read(string group, string view3D = "")
        => DrawingChartPlot.Read(
               XElement.Parse(
                   $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{view3D}"
                   + $"<c:plotArea><c:{group}>{Series}</c:{group}></c:plotArea>"
                   + "</c:chart></c:chartSpace>"))
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    private const string Series =
        "<c:ser><c:val><c:numRef><c:numCache><c:ptCount val=\"1\"/>"
        + "<c:pt idx=\"0\"><c:v>1</c:v></c:pt></c:numCache></c:numRef></c:val></c:ser>";

    private static string View(string body) => $"<c:view3D>{body}</c:view3D>";

    /// <summary>A flat pie has no elevation, whatever the part states beside it.</summary>
    /// <remarks>
    /// The control that keeps the fit off every other chart in the corpus: a <c>c:view3D</c> can
    /// sit in a part whose groups are all two-dimensional, and only the group's own element name
    /// says whether the chart is drawn as a solid.
    /// </remarks>
    [Fact]
    public void AFlatPieHasNoElevationEvenBesideAView3D()
        => Read("pieChart", View("<c:rotX val=\"30\"/>")).Elevation.ShouldBeNull();

    /// <summary>Nor does a doughnut, nor a bar chart.</summary>
    [Theory]
    [InlineData("doughnutChart")]
    [InlineData("barChart")]
    [InlineData("lineChart")]
    public void NoOtherGroupTakesAnElevation(string group)
        => Read(group, View("<c:rotX val=\"30\"/>")).Elevation.ShouldBeNull();

    /// <summary>A <c>c:pie3DChart</c> takes <c>c:rotX</c> as stated.</summary>
    [Theory]
    [InlineData(0)]
    [InlineData(30)]
    [InlineData(50)]
    [InlineData(90)]
    public void APie3DTakesItsRotX(int stated)
        => Read("pie3DChart", View($"<c:rotX val=\"{stated}\"/>")).Elevation
            .ShouldBe((double)stated);

    /// <summary>Out of range it is clamped, not rejected.</summary>
    /// <remarks><c>getLimitedValue&lt;sal_Int32, sal_Int32&gt;(…, 0, 90)</c>.</remarks>
    [Theory]
    [InlineData(-30, 0.0)]
    [InlineData(120, 90.0)]
    public void AnOutOfRangeRotXIsClamped(int stated, double expected)
        => Read("pie3DChart", View($"<c:rotX val=\"{stated}\"/>")).Elevation.ShouldBe(expected);

    /// <summary>With no <c>c:rotX</c>, and with no <c>c:view3D</c> at all, it is fifteen.</summary>
    /// <remarks>
    /// <c>mrModel.monRotationX.value_or(15)</c>. A pie3D part that states no view is still a
    /// solid — the absent attribute is a default and not an absence of three-dimensionality.
    /// </remarks>
    [Fact]
    public void AnUnstatedRotXIsFifteen()
    {
        Read("pie3DChart", View("<c:rotY val=\"0\"/>")).Elevation.ShouldBe(15.0);
        Read("pie3DChart").Elevation.ShouldBe(15.0);
    }

    /// <summary><c>c:depthPercent</c> changes nothing, at any elevation.</summary>
    [Theory]
    [InlineData(20)]
    [InlineData(50)]
    [InlineData(100)]
    [InlineData(200)]
    public void ADepthPercentIsIgnored(int depth)
        => Read("pie3DChart",
                View($"<c:rotX val=\"40\"/><c:depthPercent val=\"{depth}\"/>"))
            .Elevation.ShouldBe(40.0);
}

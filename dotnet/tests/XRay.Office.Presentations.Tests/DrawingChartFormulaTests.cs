using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Which of a data sequence's two sources wins: the <c>c:f</c> or the cache.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice keeps two data providers and they answer this oppositely. The base one,
/// <c>ChartConverter::createDataSequence</c>
/// (<c>oox/source/drawingml/chart/chartconverter.cxx:117-152</c>), reads the cache and ignores the
/// formula, because a deck's chart names cells in a workbook the presentation filter must not
/// open. Calc overrides it: <c>ExcelChartConverter::createDataSequence</c>
/// (<c>sc/source/filter/oox/excelchartconverter.cxx:76-94</c>) takes the formula and only falls
/// back to the cache when there is none.
/// </para>
/// <para>
/// These tests live beside <see cref="DrawingChartTests"/> because the seam is in the shared
/// reader, and they exercise both sides of it through a resolver the caller supplies or does not.
/// The one workbook in the sheets corpus with chart parts —
/// <c>Keywords_Mapping_Graphs_and_Charts.xlsx</c>, 11 charts — writes a cache exactly one point
/// shorter than the range it declares on all 22 of its sequences, the pivot's grand-total row
/// being the missing one, so on that file the two sources disagree everywhere.
/// </para>
/// </remarks>
public class DrawingChartFormulaTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>A bar chart whose one series states a short cache beside a longer range.</summary>
    private const string Short = """
        <c:plotArea><c:barChart><c:ser>
          <c:cat><c:strRef><c:f>'Literature Mapping'!$A$4:$A$6</c:f><c:strCache>
            <c:ptCount val="2"/>
            <c:pt idx="0"><c:v>alpha</c:v></c:pt><c:pt idx="1"><c:v>beta</c:v></c:pt>
          </c:strCache></c:strRef></c:cat>
          <c:val><c:numRef><c:f>'Literature Mapping'!$B$4:$B$6</c:f><c:numCache>
            <c:ptCount val="2"/>
            <c:pt idx="0"><c:v>7</c:v></c:pt><c:pt idx="1"><c:v>6</c:v></c:pt>
          </c:numCache></c:numRef></c:val>
        </c:ser></c:barChart></c:plotArea>
        """;

    private static ChartPlot? Plot(string inner, ChartRangeResolver? ranges = null)
        => DrawingChartPlot.Read(
            XElement.Parse(
                $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{inner}</c:chart></c:chartSpace>"),
            styles: null,
            ranges: ranges);

    /// <summary>The three cells the range names, the third of them the one the cache omits.</summary>
    private static ChartRangeValues? Live(string formula) => formula switch
    {
        "'Literature Mapping'!$A$4:$A$6" => new ChartRangeValues(
            ["alpha", "beta", "Grand Total"], [null, null, null]),
        "'Literature Mapping'!$B$4:$B$6" => new ChartRangeValues(
            ["7", "6", "35"], [7.0, 6.0, 35.0]),
        _ => null,
    };

    /// <summary>With no resolver the cache is the only source — the deck and document path.</summary>
    /// <remarks>
    /// The control, and the one that must not move: every PPTX and DOCX chart reads this way and
    /// has no workbook to resolve against.
    /// </remarks>
    [Fact]
    public void WithoutAResolverTheCacheIsRead()
    {
        ChartPlot plot = Plot(Short).ShouldNotBeNull();

        plot.Series[0].Values.Count.ShouldBe(2);
        plot.Categories.Count.ShouldBe(2);
    }

    /// <summary>With one, the range wins and the point the cache omitted appears.</summary>
    [Fact]
    public void WithAResolverTheFormulaWins()
    {
        ChartPlot plot = Plot(Short, Live).ShouldNotBeNull();

        plot.Series[0].Values.ShouldBe([7.0, 6.0, 35.0]);
        plot.Categories.ShouldBe(["alpha", "beta", "Grand Total"]);
    }

    /// <summary>
    /// A resolver that cannot answer leaves the cache in place rather than emptying the series.
    /// </summary>
    /// <remarks>
    /// This is <c>createDataSequenceByFormulaTokens</c> throwing, which the C++ catches and lets
    /// fall through. A reader that took the null as an answer would drop every series whose range
    /// names a sheet it cannot find — a defined name, an external workbook, a deleted sheet.
    /// </remarks>
    [Fact]
    public void AResolverThatAnswersNullFallsBackToTheCache()
    {
        ChartPlot plot = Plot(Short, _ => null).ShouldNotBeNull();

        plot.Series[0].Values.Count.ShouldBe(2);
    }

    /// <summary>
    /// A resolver that answers an <em>empty</em> sequence leaves the series empty — the cache does
    /// not stand in for it.
    /// </summary>
    /// <remarks>
    /// <strong>Null and empty are different answers.</strong> Null is the C++ catching a throw and
    /// falling through to the cache (the case above). Empty is a sequence that resolved and named
    /// no readable cell, which Calc really produces: a range every cell of which is an Excel
    /// table's totals row is skipped outright by <c>ScChart2DataSequence::BuildDataCache</c>
    /// (<c>sc/source/ui/unoobj/chart2uno.cxx:2616-2632</c>), and the reference then draws an empty
    /// plot at the value axis's default scale. That is `029_Annual_budget`'s left chart, where
    /// falling back to the cache draws the whole thing — two series, twenty-two data labels and an
    /// axis to $4,500 — over a reference that draws none of it.
    /// </remarks>
    [Fact]
    public void AResolverThatAnswersAnEmptySequenceDoesNotFallBackToTheCache()
    {
        ChartPlot plot = Plot(Short, _ => new ChartRangeValues([], [])).ShouldNotBeNull();

        plot.Series[0].Values.ShouldBeEmpty();
        plot.Categories.ShouldBeEmpty();
    }

    /// <summary>A literal sequence has no <c>c:f</c>, so the resolver is never asked.</summary>
    /// <remarks>
    /// <c>maFormula.isEmpty()</c> is the C++'s test and it is a test of the <em>formula</em>, not
    /// of the container. A reader that asked the resolver for a literal would hand it an empty
    /// string and take whatever came back.
    /// </remarks>
    [Fact]
    public void ALiteralSequenceNeverReachesTheResolver()
    {
        List<string> asked = [];

        ChartPlot plot = Plot(
            """
            <c:plotArea><c:barChart><c:ser>
              <c:val><c:numLit><c:ptCount val="2"/>
                <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
              </c:numLit></c:val>
            </c:ser></c:barChart></c:plotArea>
            """,
            formula => { asked.Add(formula); return null; }).ShouldNotBeNull();

        asked.ShouldBeEmpty();
        plot.Series[0].Values.ShouldBe([1.0, 2.0]);
    }
}

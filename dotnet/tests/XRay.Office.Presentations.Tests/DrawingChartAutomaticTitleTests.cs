using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The title LibreOffice substitutes when a chart part states an empty <c>c:title</c>, or states
/// none and has not deleted it.
/// </summary>
/// <remarks>
/// <para>
/// <c>ChartSpaceConverter::convertFromModel</c> (<c>chartspaceconverter.cxx:177-208</c>). The
/// rule and the corpus measurements that confirm each of its arms on 26.2.4.2 are on
/// <c>DrawingChartTitle</c>; these are the shapes that separate its branches from one another.
/// </para>
/// <para>
/// <strong>Every case here is driven through the public reader</strong> rather than the internal
/// helper, because the question the corpus asks is "what text ends up on the page", and a helper
/// that answers correctly into a <see cref="ChartPlot"/> nobody reads would pass while drawing
/// nothing.
/// </para>
/// </remarks>
public class DrawingChartAutomaticTitleTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Read(string inner, bool office2007 = false)
        => DrawingChartPlot.Read(
               XElement.Parse(
                   $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">"
                   + $"<c:chart>{inner}</c:chart></c:chartSpace>"),
               office2007: office2007)
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>A series named <paramref name="name"/> through a <c>c:tx</c> cache.</summary>
    private static string Series(string? name, string values = "20000") =>
        "<c:ser>"
        + (name is null
            ? ""
            : "<c:tx><c:strRef><c:f>Sheet1!$B$1</c:f><c:strCache><c:ptCount val=\"1\"/>"
              + $"<c:pt idx=\"0\"><c:v>{name}</c:v></c:pt></c:strCache></c:strRef></c:tx>")
        + "<c:val><c:numRef><c:numCache><c:ptCount val=\"1\"/>"
        + $"<c:pt idx=\"0\"><c:v>{values}</c:v></c:pt></c:numCache></c:numRef></c:val>"
        + "</c:ser>";

    /// <summary>One type group of <paramref name="element"/> holding <paramref name="series"/>.</summary>
    private static string Group(string element, string series, int category = 1, int value = 2) =>
        $"<c:{element}>{series}<c:axId val=\"{category}\"/><c:axId val=\"{value}\"/></c:{element}>";

    private static string Plot(string groups, string axes = "<c:valAx><c:axId val=\"2\"/></c:valAx>")
        => $"<c:plotArea>{groups}{axes}</c:plotArea>";

    /// <summary>The whole chart body: a title element, a deletion flag, and a plot area.</summary>
    private static string Chart(string title, string deleted, string plot)
        => title + deleted + plot;

    private const string NotDeleted = "<c:autoTitleDeleted val=\"0\"/>";
    private const string Deleted = "<c:autoTitleDeleted val=\"1\"/>";
    private const string EmptyTitle = "<c:title><c:overlay val=\"0\"/></c:title>";

    // ---- the series-name branch --------------------------------------------------------------

    /// <summary>
    /// `005_Contextures_chart_sample`'s shape exactly: an empty title element, an explicit
    /// `autoTitleDeleted val="0"`, and one series named `Sales`. The reference draws `Sales`.
    /// </summary>
    [Fact]
    public void AnEmptyTitleOverOneNamedSeriesTakesTheSeriesName()
        => Read(Chart(EmptyTitle, NotDeleted, Plot(Group("barChart", Series("Sales")))))
            .Title.ShouldBe("Sales");

    /// <summary>
    /// With no title element at all the block is still entered when the file says the automatic
    /// title was not deleted — the `!mbAutoTitleDel || mxTitle.is()` disjunction.
    /// </summary>
    [Fact]
    public void ANotDeletedAutomaticTitleNeedsNoTitleElement()
        => Read(Chart("", NotDeleted, Plot(Group("barChart", Series("Sales")))))
            .Title.ShouldBe("Sales");

    /// <summary>
    /// A series named by a literal `c:v` rather than by a cached reference is named all the same:
    /// `TextContext::onCharacters` puts it in the same `maData[0]`.
    /// </summary>
    [Fact]
    public void ASeriesNamedByALiteralValueIsNamedTheSameWay()
        => Read(Chart(
                EmptyTitle, NotDeleted,
                Plot(Group(
                    "barChart",
                    "<c:ser><c:tx><c:v>Sales</c:v></c:tx>"
                    + "<c:val><c:numRef><c:numCache><c:ptCount val=\"1\"/>"
                    + "<c:pt idx=\"0\"><c:v>3</c:v></c:pt></c:numCache></c:numRef></c:val>"
                    + "</c:ser>"))))
            .Title.ShouldBe("Sales");

    /// <summary>
    /// A pie group's `mbSingleSeriesVis` makes the *first* series' name the automatic title even
    /// with three series stated, because only one of them is ever drawn.
    /// </summary>
    [Fact]
    public void APieChartTakesItsFirstSeriesNameEvenWithSeveralSeries()
        => Read(Chart(
                EmptyTitle, NotDeleted,
                Plot(Group("pieChart", Series("Sales") + Series("Costs") + Series("Margin")))))
            .Title.ShouldBe("Sales");

    /// <summary>
    /// And a doughnut does not. It is <c>TYPEID_DOUGHNUT</c>, whose "1stvis" column is clear —
    /// the distinction a first draft of the census got wrong, so it is asserted rather than
    /// assumed.
    /// </summary>
    [Fact]
    public void ADoughnutWithSeveralSeriesHasNoSingleSeriesName()
        => Read(Chart(
                EmptyTitle, NotDeleted,
                Plot(Group("doughnutChart", Series("Sales") + Series("Costs")))))
            .Title.ShouldBe(DiagramTitle);

    // ---- the literal branch ------------------------------------------------------------------

    private const string DiagramTitle = "Chart Title";

    /// <summary>
    /// `035_Chemistry_Column_PowerPoint_Chart`'s shape: an empty title, two series, and so no
    /// single-series name. The reference draws the localized literal.
    /// </summary>
    [Fact]
    public void AnEmptyTitleOverTwoSeriesTakesTheLocalizedLiteral()
        => Read(Chart(
                EmptyTitle, NotDeleted,
                Plot(Group("barChart", Series("Sales") + Series("Costs")))))
            .Title.ShouldBe(DiagramTitle);

    /// <summary>
    /// tdf#146487's first escape: a title carrying both a shape and an empty text body, over a
    /// series that *does* state a `c:tx`, is an empty title the author asked for. The literal is
    /// suppressed and nothing is drawn.
    /// </summary>
    /// <remarks>
    /// The series' cache is empty, which is what separates `getSingleSeriesTitle` (wants a
    /// string) from `isSingleSeriesTitle` (wants only the element) — the one shape in which the
    /// two disagree.
    /// </remarks>
    [Fact]
    public void AFormattedButEmptyTitleOverASingleSeriesIsLeftEmpty()
        => Read(Chart(
                "<c:title><c:spPr><a:noFill/></c:spPr>"
                + "<c:txPr><a:bodyPr/><a:lstStyle/><a:p><a:pPr/><a:endParaRPr/></a:p></c:txPr>"
                + "</c:title>",
                NotDeleted,
                Plot(Group(
                    "barChart",
                    "<c:ser><c:tx><c:strRef><c:f>Sheet1!$B$1</c:f>"
                    + "<c:strCache><c:ptCount val=\"0\"/></c:strCache></c:strRef></c:tx>"
                    + "<c:val><c:numRef><c:numCache><c:ptCount val=\"1\"/>"
                    + "<c:pt idx=\"0\"><c:v>3</c:v></c:pt></c:numCache></c:numRef></c:val>"
                    + "</c:ser>"))))
            .Title.ShouldBeNull();

    /// <summary>
    /// The same shape without the <c>c:spPr</c> is *not* the escape — the conjunction wants both
    /// — so the literal is drawn. Two cases differing in one element, which is what makes the
    /// case above a statement about the conjunction rather than about empty text bodies.
    /// </summary>
    [Fact]
    public void AnEmptyTextBodyWithNoShapePropertiesStillTakesTheLiteral()
        => Read(Chart(
                "<c:title>"
                + "<c:txPr><a:bodyPr/><a:lstStyle/><a:p><a:pPr/><a:endParaRPr/></a:p></c:txPr>"
                + "</c:title>",
                NotDeleted,
                Plot(Group(
                    "barChart",
                    "<c:ser><c:tx><c:strRef><c:f>Sheet1!$B$1</c:f>"
                    + "<c:strCache><c:ptCount val=\"0\"/></c:strCache></c:strRef></c:tx>"
                    + "<c:val><c:numRef><c:numCache><c:ptCount val=\"1\"/>"
                    + "<c:pt idx=\"0\"><c:v>3</c:v></c:pt></c:numCache></c:numRef></c:val>"
                    + "</c:ser>"))))
            .Title.ShouldBe(DiagramTitle);

    /// <summary>
    /// tdf#146487's second escape: an explicitly empty rich text body suppresses the literal
    /// whatever the series look like.
    /// </summary>
    [Fact]
    public void AnEmptyRichTextBodySuppressesTheLiteral()
        => Read(Chart(
                "<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:endParaRPr/></a:p></c:rich></c:tx></c:title>",
                NotDeleted,
                Plot(Group("barChart", Series("Sales") + Series("Costs")))))
            .Title.ShouldBeNull();

    // ---- the entry condition and the default -------------------------------------------------

    /// <summary>
    /// No title element and no deletion flag: the default is "deleted" for anything but an Office
    /// 2007 package, so nothing is drawn. This is the arm 157 of the corpus's 307 chart parts take
    /// and it has three corpus controls behind it.
    /// </summary>
    [Fact]
    public void NoTitleElementAndNoFlagDrawsNothing()
        => Read(Chart("", "", Plot(Group("barChart", Series("Sales")))))
            .Title.ShouldBeNull();

    /// <summary>The same part written by Office 2007 takes the other default.</summary>
    [Fact]
    public void NoTitleElementAndNoFlagTakesTheSeriesNameForAnOffice2007Package()
        => Read(Chart("", "", Plot(Group("barChart", Series("Sales")))), office2007: true)
            .Title.ShouldBe("Sales");

    /// <summary>
    /// A deleted automatic title still yields one when the part states a title element — the
    /// disjunction again, from the other side.
    /// </summary>
    [Fact]
    public void ADeletedAutomaticTitleIsStillDrawnWhenATitleElementIsStated()
        => Read(Chart(EmptyTitle, Deleted, Plot(Group("barChart", Series("Sales")))))
            .Title.ShouldBe("Sales");

    /// <summary>
    /// `005_Contextures_chart_sample`'s sixth chart, which is this round's negative control on
    /// the corpus: not deleted, no title element, and a series with no `c:tx`. Nothing to name
    /// the chart with and no title element to fall back into, so no title.
    /// </summary>
    [Fact]
    public void AnUnnamedSeriesWithNoTitleElementDrawsNothing()
        => Read(Chart("", NotDeleted, Plot(Group("barChart", Series(name: null)))))
            .Title.ShouldBeNull();

    // ---- the axes-set and type-group conditions ----------------------------------------------

    /// <summary>
    /// Two type groups on one axis pair — a column chart with a line over it — is not one type
    /// group, so `AxesSetConverter` never asks for a single series title.
    /// </summary>
    [Fact]
    public void TwoTypeGroupsOnOneAxisPairHaveNoSingleSeriesName()
        => Read(Chart(
                EmptyTitle, NotDeleted,
                Plot(Group("barChart", Series("Sales"))
                     + Group("lineChart", Series("Trend")))))
            .Title.ShouldBe(DiagramTitle);

    /// <summary>
    /// Two *axes sets* — the same two groups naming different axis ids — clear the automatic
    /// title outright, even though each set holds exactly one group with exactly one series.
    /// </summary>
    [Fact]
    public void TwoAxesSetsClearTheAutomaticTitleAltogether()
        => Read(Chart(
                EmptyTitle, NotDeleted,
                Plot(
                    Group("barChart", Series("Sales"))
                    + Group("lineChart", Series("Trend"), category: 3, value: 4),
                    "<c:valAx><c:axId val=\"2\"/></c:valAx><c:valAx><c:axId val=\"4\"/></c:valAx>")))
            .Title.ShouldBe(DiagramTitle);

    /// <summary>
    /// A type group with no series at all is skipped when the axes sets are built, so a part that
    /// states an empty group beside a real one still has one axes set holding one group.
    /// </summary>
    [Fact]
    public void AnEmptyTypeGroupDoesNotCountAsATypeGroup()
        => Read(Chart(
                EmptyTitle, NotDeleted,
                Plot(Group("lineChart", "", category: 3, value: 4)
                     + Group("barChart", Series("Sales")))))
            .Title.ShouldBe("Sales");

    // ---- precedence --------------------------------------------------------------------------

    /// <summary>
    /// A title with text of its own keeps it. The automatic string is `createStringSequence`'s
    /// *default* argument and is reached only when the model has nothing.
    /// </summary>
    [Fact]
    public void ATitleThatStatesItsOwnTextIsUnaffected()
        => Read(Chart(
                "<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:t>Quarterly</a:t></a:r></a:p>"
                + "</c:rich></c:tx></c:title>",
                NotDeleted,
                Plot(Group("barChart", Series("Sales")))))
            .Title.ShouldBe("Quarterly");

    /// <summary>
    /// <strong>A shape control, not a detector.</strong> The content tree and the drawing must
    /// answer the same thing or a chart's table and its picture disagree about what the chart is
    /// called; this fails if only one of the two readers is taught the rule.
    /// </summary>
    [Fact]
    public void TheContentTreeAndTheDrawingAgreeOnTheSubstitutedTitle()
    {
        string markup =
            $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>"
            + Chart(EmptyTitle, NotDeleted, Plot(Group("barChart", Series("Sales"))))
            + "</c:chart></c:chartSpace>";

        DrawingChart.Read(XElement.Parse(markup))!.Name.ShouldBe("Sales");
        DrawingChartPlot.Read(XElement.Parse(markup))!.Title.ShouldBe("Sales");
    }
}

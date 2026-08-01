using System.Xml.Linq;
using Paperless.Core.Extraction;
using Paperless.Ooxml.DrawingML;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests the shared DrawingML chart reader from markup literals.
/// </summary>
/// <remarks>
/// The reader lives in <c>Paperless.Ooxml</c> because a chart part is identical whether the
/// frame that points at it is on a slide, a worksheet or a page of a Word document, so nothing
/// here is PresentationML. Each case is a shape of markup a real producer writes and a naive
/// reader gets wrong.
/// </remarks>
public class DrawingChartTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ContentSection? Read(string inner)
        => DrawingChart.Read(XElement.Parse(
            $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{inner}</c:chart></c:chartSpace>"));

    /// <summary>A bar chart of one series over <paramref name="points"/> cached points.</summary>
    private static string Bar(string points, string name = "North") =>
        $"""
         <c:plotArea><c:barChart><c:ser>
           <c:tx><c:strRef><c:f>Sheet1!$B$1</c:f><c:strCache>
             <c:ptCount val="1"/><c:pt idx="0"><c:v>{name}</c:v></c:pt>
           </c:strCache></c:strRef></c:tx>
           {points}
         </c:ser></c:barChart></c:plotArea>
         """;

    private static List<string> Rows(ContentTable table)
        => [.. table.Children.OfType<ContentTableRow>()
                 .Select(row => string.Join('|', row.Children.Select(cell => cell.GetText())))];

    [Fact]
    public void ASeriesBecomesAColumnAndACategoryARow()
    {
        ContentSection? chart = Read(Bar(
            """
            <c:cat><c:strRef><c:strCache><c:ptCount val="2"/>
              <c:pt idx="0"><c:v>Q1</c:v></c:pt><c:pt idx="1"><c:v>Q2</c:v></c:pt>
            </c:strCache></c:strRef></c:cat>
            <c:val><c:numRef><c:numCache><c:ptCount val="2"/>
              <c:pt idx="0"><c:v>120</c:v></c:pt><c:pt idx="1"><c:v>95</c:v></c:pt>
            </c:numCache></c:numRef></c:val>
            """));

        ContentTable table = chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single();

        // The corner cell is empty because the file says nothing about it; ODF's own local
        // table writes the same empty cell rather than a label for the category column.
        Rows(table).ShouldBe(["|North", "Q1|120", "Q2|95"]);
        table.ColumnCount.ShouldBe(2);
        table.HeaderRowCount.ShouldBe(1);
    }

    [Fact]
    public void AValueIsBothANumberAndTheTextTheFileWroteItAs()
    {
        ContentSection? chart = Read(Bar(
            """
            <c:val><c:numRef><c:numCache><c:formatCode>0.0</c:formatCode><c:ptCount val="1"/>
              <c:pt idx="0"><c:v>1.5000000000000002</c:v></c:pt>
            </c:numCache></c:numRef></c:val>
            """));

        ContentTableCell cell = chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single()
            .Children.OfType<ContentTableRow>().Last()
            .Children.OfType<ContentTableCell>().Last();

        // The number for a caller computing with the chart; the file's own literal as the
        // displayed text, because running it through c:formatCode would need the number
        // formatter that lives in Paperless.Spreadsheets, above this library.
        cell.Value.ShouldBe(1.5000000000000002d);
        cell.GetText().ShouldBe("1.5000000000000002");
    }

    [Fact]
    public void ASparsePointIndexLeavesAGapRatherThanShiftingEverythingAfterIt()
    {
        ContentSection? chart = Read(Bar(
            """
            <c:cat><c:strRef><c:strCache><c:ptCount val="3"/>
              <c:pt idx="0"><c:v>Q1</c:v></c:pt><c:pt idx="2"><c:v>Q3</c:v></c:pt>
            </c:strCache></c:strRef></c:cat>
            <c:val><c:numRef><c:numCache><c:ptCount val="3"/>
              <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="2"><c:v>3</c:v></c:pt>
            </c:numCache></c:numRef></c:val>
            """));

        // The trap this exists for: a chart over a range with a blank in it declares ptCount=3
        // and writes two c:pt whose idx skips the blank. Reading them in document order puts
        // Q3's value on Q2 and leaves a chart that still looks plausible.
        Rows(chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single())
            .ShouldBe(["|North", "Q1|1", "|", "Q3|3"]);
    }

    [Fact]
    public void AReferenceWithNoCacheYieldsNoValuesRatherThanReachingForTheWorkbook()
    {
        ContentSection? chart = Read(Bar("<c:val><c:numRef><c:f>Sheet1!$B$2:$B$5</c:f></c:numRef></c:val>"));

        // The decision this reader is built on. The range is real and the embedded workbook
        // could answer it, but opening one would mean depending on Paperless.Spreadsheets and
        // would report numbers no reference renderer draws. The series' name survives.
        ContentTable table = chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single();
        Rows(table).ShouldBe(["|North"]);
    }

    [Fact]
    public void ANonNumericCachedPointKeepsItsText()
    {
        ContentSection? chart = Read(Bar(
            """
            <c:val><c:numRef><c:numCache><c:ptCount val="1"/>
              <c:pt idx="0"><c:v>#N/A</c:v></c:pt>
            </c:numCache></c:numRef></c:val>
            """));

        ContentTableCell cell = chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single()
            .Children.OfType<ContentTableRow>().Last()
            .Children.OfType<ContentTableCell>().Last();

        // A chart caches whatever the range evaluated to, errors included. Dropping it would
        // report the point as blank, which is a different claim.
        cell.Value.ShouldBe("#N/A");
        cell.GetText().ShouldBe("#N/A");
    }

    [Fact]
    public void ASeriesNameCachedAcrossSeveralCellsIsJoinedIntoOne()
    {
        ContentSection? chart = Read(
            """
            <c:plotArea><c:barChart><c:ser>
              <c:tx><c:strRef><c:strCache><c:ptCount val="2"/>
                <c:pt idx="0"><c:v>2024</c:v></c:pt><c:pt idx="1"><c:v>North</c:v></c:pt>
              </c:strCache></c:strRef></c:tx>
            </c:ser></c:barChart></c:plotArea>
            """);

        // datasourceconverter.cxx:50-73: "the internal data table does not support complex
        // labels ... merge the labels into a single one", joined with a space.
        chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single()
            .Children.OfType<ContentTableRow>().First().GetText().Trim().ShouldBe("2024 North");
    }

    [Fact]
    public void ASeriesNameStatedAsALiteralIsRead()
    {
        ContentSection? chart = Read("<c:plotArea><c:lineChart><c:ser><c:tx><c:v>South</c:v></c:tx>"
                                     + "</c:ser></c:lineChart></c:plotArea>");

        // CT_SerTx is a choice: a c:strRef or a bare c:v. A reader that only handles the
        // reference loses every series name in a chart typed by hand rather than linked.
        chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single()
            .Children.OfType<ContentTableRow>().First().GetText().Trim().ShouldBe("South");
    }

    [Fact]
    public void AScatterSeriesReadsItsXAndYValues()
    {
        ContentSection? chart = Read(
            """
            <c:plotArea><c:scatterChart><c:ser>
              <c:tx><c:v>Trial</c:v></c:tx>
              <c:xVal><c:numRef><c:numCache><c:ptCount val="2"/>
                <c:pt idx="0"><c:v>0.5</c:v></c:pt><c:pt idx="1"><c:v>1.5</c:v></c:pt>
              </c:numCache></c:numRef></c:xVal>
              <c:yVal><c:numRef><c:numCache><c:ptCount val="2"/>
                <c:pt idx="0"><c:v>10</c:v></c:pt><c:pt idx="1"><c:v>20</c:v></c:pt>
              </c:numCache></c:numRef></c:yVal>
            </c:ser></c:scatterChart></c:plotArea>
            """);

        // A scatter chart has no categories at all: its x values are the same CT_AxDataSource
        // under a different name, and a reader that only knows c:cat and c:val reports the
        // series as empty.
        Rows(chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single())
            .ShouldBe(["|Trial", "0.5|10", "1.5|20"]);
    }

    [Fact]
    public void AMultiLevelCategoryReadsOutermostFirst()
    {
        ContentSection? chart = Read(Bar(
            """
            <c:cat><c:multiLvlStrRef><c:multiLvlStrCache><c:ptCount val="2"/>
              <c:lvl><c:pt idx="0"><c:v>Q1</c:v></c:pt><c:pt idx="1"><c:v>Q2</c:v></c:pt></c:lvl>
              <c:lvl><c:pt idx="0"><c:v>2024</c:v></c:pt></c:lvl>
            </c:multiLvlStrCache></c:multiLvlStrRef></c:cat>
            <c:val><c:numRef><c:numCache><c:ptCount val="2"/>
              <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
            </c:numCache></c:numRef></c:val>
            """));

        // Excel writes the levels innermost first, so the outer group is the last c:lvl and a
        // point it does not restate belongs to the group above it.
        Rows(chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single())
            .ShouldBe(["|North", "2024 Q1|1", "Q2|2"]);
    }

    [Fact]
    public void ATitleComesFromItsRichTextAndAnAxisTitleFollowsIt()
    {
        ContentSection? chart = Read(
            """
            <c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:t>Regional revenue</a:t></a:r></a:p>
              </c:rich></c:tx></c:title>
            <c:plotArea>
              <c:barChart><c:ser><c:tx><c:v>North</c:v></c:tx></c:ser></c:barChart>
              <c:catAx><c:title><c:tx><c:rich><a:p><a:r><a:t>Quarter</a:t></a:r></a:p>
                </c:rich></c:tx></c:title></c:catAx>
              <c:valAx><c:title><c:tx><c:rich><a:p><a:r><a:t>Units</a:t></a:r></a:p>
                </c:rich></c:tx></c:title></c:valAx>
            </c:plotArea>
            """);

        chart.ShouldNotBeNull();

        // The title is on the section so it can be found without counting paragraphs, and is
        // also the first paragraph so that GetText sees it — a name is never visited by
        // GetText, so the two do not double-count.
        chart.Name.ShouldBe("Regional revenue");
        chart.Children.OfType<ContentParagraph>().Select(p => p.GetText().Trim())
            .ShouldBe(["Regional revenue", "Quarter", "Units"]);
        chart.GetText().ShouldStartWith("Regional revenue\nQuarter\nUnits\n");
    }

    [Fact]
    public void ATitleFallsBackToItsLinkedCellAndThenToNothing()
    {
        ContentSection? linked = Read(
            """
            <c:title><c:tx><c:strRef><c:f>Sheet1!$A$1</c:f><c:strCache>
              <c:ptCount val="1"/><c:pt idx="0"><c:v>From a cell</c:v></c:pt>
            </c:strCache></c:strRef></c:tx></c:title>
            <c:plotArea><c:barChart><c:ser><c:tx><c:v>North</c:v></c:tx></c:ser></c:barChart></c:plotArea>
            """);

        linked.ShouldNotBeNull().Name.ShouldBe("From a cell");

        ContentSection? none = Read(
            "<c:autoTitleDeleted val=\"0\"/>"
            + "<c:plotArea><c:barChart><c:ser><c:tx><c:v>North</c:v></c:tx></c:ser></c:barChart></c:plotArea>");

        // No title is invented. LibreOffice substitutes the single series' name, or the
        // localised "Chart Title" (chartspaceconverter.cxx:185-204); reporting either would
        // claim the file said something it does not.
        none.ShouldNotBeNull().Name.ShouldBeNull();
        none.Children.OfType<ContentParagraph>().ShouldBeEmpty();
    }

    [Fact]
    public void AChartWithNothingInItIsNotASection()
    {
        // The caller keeps recording the frame as a graphic when this returns null, so "there
        // is a chart here" stays distinguishable from "there is nothing here".
        Read("<c:plotArea><c:layout/></c:plotArea>").ShouldBeNull();
        DrawingChart.Read(XElement.Parse($"<c:chartSpace xmlns:c=\"{C}\"/>")).ShouldBeNull();
    }

    [Fact]
    public void EveryPlotTypeGroupContributesItsSeries()
    {
        ContentSection? chart = Read(
            """
            <c:plotArea>
              <c:barChart><c:ser><c:tx><c:v>Bars</c:v></c:tx></c:ser></c:barChart>
              <c:lineChart><c:ser><c:tx><c:v>Line</c:v></c:tx></c:ser></c:lineChart>
              <c:pieChart><c:ser><c:tx><c:v>Slices</c:v></c:tx></c:ser></c:pieChart>
              <c:areaChart><c:ser><c:tx><c:v>Area</c:v></c:tx></c:ser></c:areaChart>
            </c:plotArea>
            """);

        // CT_PlotArea holds a repeatable choice, so a combination chart states several groups
        // and one series must not shadow the others.
        chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single()
            .Children.OfType<ContentTableRow>().First().GetText().Trim()
            .ShouldBe("Bars\tLine\tSlices\tArea");
    }

    [Fact]
    public void AnAbsurdPointCountIsClamped()
    {
        ContentSection? chart = Read(Bar(
            "<c:val><c:numRef><c:numCache><c:ptCount val=\"4294967295\"/>"
            + "<c:pt idx=\"0\"><c:v>1</c:v></c:pt></c:numCache></c:numRef></c:val>"));

        // ptCount is an unbounded unsignedInt written by whoever produced the file, and it
        // sizes an array before a single point has been read. Clamped on the way in, and the
        // rows nothing was written into are trimmed again on the way out — so the table is the
        // header and the one real point rather than four billion blanks.
        Rows(chart.ShouldNotBeNull().Children.OfType<ContentTable>().Single())
            .ShouldBe(["|North", "|1"]);
    }
}

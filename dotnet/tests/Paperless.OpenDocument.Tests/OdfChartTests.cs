using System.Xml.Linq;
using Paperless.Core.Extraction;
using Shouldly;

namespace Paperless.OpenDocument.Tests;

/// <summary>
/// Tests the ODF chart reader from markup literals.
/// </summary>
/// <remarks>
/// The reader lives here rather than beside a family because <c>chart:chart</c> is the same
/// sub-document whether the <c>draw:object</c> pointing at it sits on a slide, in a spreadsheet
/// cell or in a paragraph. Each case is a shape LibreOffice's own export writes.
/// </remarks>
public class OdfChartTests
{
    private const string Ns =
        $"xmlns:chart=\"{OdfNamespaces.Chart}\" xmlns:table=\"{OdfNamespaces.Table}\" "
        + $"xmlns:text=\"{OdfNamespaces.Text}\" xmlns:office=\"{OdfNamespaces.Office}\" "
        + $"xmlns:draw=\"{OdfNamespaces.Draw}\" xmlns:svg=\"{OdfNamespaces.SvgCompatible}\"";

    private static ContentSection? Read(string inner)
        => OdfChart.Read(XElement.Parse($"<chart:chart {Ns}>{inner}</chart:chart>"));

    /// <summary>The local table LibreOffice writes for a two-series, two-category bar chart.</summary>
    private const string LocalTable =
        """
        <table:table table:name="local-table">
         <table:table-header-columns><table:table-column/></table:table-header-columns>
         <table:table-columns><table:table-column table:number-columns-repeated="2"/></table:table-columns>
         <table:table-header-rows><table:table-row>
          <table:table-cell><text:p/></table:table-cell>
          <table:table-cell office:value-type="string"><text:p>North</text:p></table:table-cell>
          <table:table-cell office:value-type="string"><text:p>South</text:p></table:table-cell>
         </table:table-row></table:table-header-rows>
         <table:table-rows>
          <table:table-row>
           <table:table-cell office:value-type="string"><text:p>Q1</text:p></table:table-cell>
           <table:table-cell office:value-type="float" office:value="120"><text:p>120</text:p></table:table-cell>
           <table:table-cell office:value-type="float" office:value="88"><text:p>88</text:p></table:table-cell>
          </table:table-row>
          <table:table-row>
           <table:table-cell office:value-type="string"><text:p>Q2</text:p></table:table-cell>
           <table:table-cell office:value-type="float" office:value="95"><text:p>95</text:p></table:table-cell>
           <table:table-cell office:value-type="float" office:value="132"><text:p>132</text:p></table:table-cell>
          </table:table-row>
         </table:table-rows>
        </table:table>
        """;

    private static List<string> Rows(ContentTable table)
        => [.. table.Children.OfType<ContentTableRow>()
                 .Select(row => string.Join('|', row.Children.Select(cell => cell.GetText())))];

    [Fact]
    public void TheLocalTableIsTheChartsData()
    {
        ContentSection chart = Read(LocalTable).ShouldNotBeNull();
        ContentTable table = chart.Children.OfType<ContentTable>().Single();

        // The layout the DrawingML reader assembles by hand out of c:ser/c:tx and c:cat, which
        // ODF states directly: series across the header row, categories down the first column.
        Rows(table).ShouldBe(["|North|South", "Q1|120|88", "Q2|95|132"]);
        table.ColumnCount.ShouldBe(3);
        table.HeaderRowCount.ShouldBe(1);
    }

    [Fact]
    public void AFloatCellCarriesItsNumberAndItsDisplayedText()
    {
        ContentTableCell cell = Read(LocalTable).ShouldNotBeNull()
            .Children.OfType<ContentTable>().Single()
            .Children.OfType<ContentTableRow>().Skip(1).First()
            .Children.OfType<ContentTableCell>().Skip(1).First();

        // office:value is the number and the text:p is what was shown, which is the same
        // division the DrawingML side gets from c:v — there, by keeping the literal.
        cell.Value.ShouldBe(120d);
        cell.GetText().ShouldBe("120");
    }

    [Fact]
    public void ALocalTableWithNoHeaderRowsGroupClaimsNoHeader()
    {
        ContentSection chart = Read(
            """
            <table:table table:name="local-table"><table:table-row>
             <table:table-cell office:value-type="float" office:value="1"><text:p>1</text:p></table:table-cell>
            </table:table-row></table:table>
            """).ShouldNotBeNull();

        // SchXMLTableContext decides the offsets it applies the table at from
        // table:table-header-rows and table:table-header-columns
        // (SchXMLTableContext.cxx:95-96, :206-218). Without the element there is no header,
        // and claiming one would name a data row as series labels.
        chart.Children.OfType<ContentTable>().Single().HeaderRowCount.ShouldBe(0);
    }

    [Fact]
    public void TitleSubtitleAndAxisTitlesComeOutInDocumentOrder()
    {
        ContentSection chart = Read(
            $"""
             <chart:title><text:p>Regional revenue</text:p></chart:title>
             <chart:subtitle><text:p>Financial year 2024</text:p></chart:subtitle>
             <chart:plot-area>
              <chart:axis chart:dimension="x"><chart:title><text:p>Quarter</text:p></chart:title></chart:axis>
              <chart:axis chart:dimension="y"><chart:title><text:p>Units</text:p></chart:title></chart:axis>
             </chart:plot-area>
             {LocalTable}
             """).ShouldNotBeNull();

        chart.Name.ShouldBe("Regional revenue");
        chart.Children.OfType<ContentParagraph>().Select(p => p.GetText().Trim())
            .ShouldBe(["Regional revenue", "Financial year 2024", "Quarter", "Units"]);

        // A subtitle has no DrawingML counterpart — c:chart has one title element — so a chart
        // round-tripped through PPTX loses it. That is the format's doing, not the reader's.
        chart.Kind.ShouldBe(SectionKind.Frame);
    }

    [Fact]
    public void ARangeAddressInAHeaderCellIsNotPartOfTheSeriesName()
    {
        ContentSection chart = Read(
            """
            <table:table table:name="local-table">
             <table:table-header-rows><table:table-row>
              <table:table-cell><text:p/></table:table-cell>
              <table:table-cell office:value-type="string">
               <text:p>North</text:p>
               <draw:g><svg:desc>Sheet1.B1:Sheet1.B1</svg:desc></draw:g>
              </table:table-cell>
             </table:table-row></table:table-header-rows>
            </table:table>
            """).ShouldNotBeNull();

        // LibreOffice writes the originating range into the local table beside the label.
        // Reading a cell with XElement.Value splices the address into the series name.
        chart.Children.OfType<ContentTable>().Single()
            .Children.OfType<ContentTableRow>().Single().GetText().Trim().ShouldBe("North");
    }

    [Fact]
    public void ARepeatedColumnBecomesThatManyCells()
    {
        ContentSection chart = Read(
            """
            <table:table table:name="local-table"><table:table-row>
             <table:table-cell office:value-type="string"><text:p>x</text:p></table:table-cell>
             <table:table-cell table:number-columns-repeated="3" office:value-type="float"
                 office:value="7"><text:p>7</text:p></table:table-cell>
            </table:table-row></table:table>
            """).ShouldNotBeNull();

        Rows(chart.Children.OfType<ContentTable>().Single()).ShouldBe(["x|7|7|7"]);
    }

    [Fact]
    public void TrailingEmptyRowsArePadding()
    {
        ContentSection chart = Read(
            """
            <table:table table:name="local-table">
             <table:table-row>
              <table:table-cell office:value-type="float" office:value="1"><text:p>1</text:p></table:table-cell>
             </table:table-row>
             <table:table-row table:number-rows-repeated="8"><table:table-cell/></table:table-row>
            </table:table>
            """).ShouldNotBeNull();

        // A chart whose range was shortened keeps the rows it had, empty. Reporting them would
        // make the chart's data look eight rows longer than it is.
        Rows(chart.Children.OfType<ContentTable>().Single()).ShouldBe(["1"]);
    }

    [Fact]
    public void AChartWithNothingInItIsNotASection()
    {
        // The caller goes on recording the object as a graphic when this returns null.
        Read(string.Empty).ShouldBeNull();
    }

    [Fact]
    public void AnEmbeddedObjectThatIsNotAChartIsNotLocated()
    {
        using OdfFile file = OdfFile.Open(new MemoryStream(
            System.Text.Encoding.UTF8.GetBytes(
                $"<office:document xmlns:office=\"{OdfNamespaces.Office}\" "
                + $"xmlns:text=\"{OdfNamespaces.Text}\"><office:body><office:text/>"
                + "</office:body></office:document>")));

        XElement formula = XElement.Parse(
            $"<draw:object xmlns:draw=\"{OdfNamespaces.Draw}\" "
            + $"xmlns:office=\"{OdfNamespaces.Office}\" xmlns:math=\"http://www.w3.org/1998/Math/MathML\">"
            + "<office:document office:mimetype=\"application/vnd.oasis.opendocument.formula\">"
            + "<office:body><office:formula/></office:body></office:document></draw:object>");

        // draw:object wraps every embedded ODF document there is, so "is this a chart" can only
        // be answered by looking inside — not by trusting the manifest's media type, which is
        // missing or wrong in files written by tools.
        OdfChart.Locate(formula, file).ShouldBeNull();
    }
}

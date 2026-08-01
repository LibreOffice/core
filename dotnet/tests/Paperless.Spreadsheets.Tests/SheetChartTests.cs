using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// What a workbook's reader makes of a chart anchored on a sheet.
/// </summary>
/// <remarks>
/// <c>chart-bar-sheet.{fods,ods,xlsx}</c> is one hand-written flat spreadsheet and LibreOffice
/// 24.2.7.2's two conversions of it: a four-row table in <c>A1:C5</c> and a bar chart over it
/// with a title, two axis titles, two named series and four labelled categories. The chart
/// references the sheet's own cells — <c>Revenue.B2:Revenue.B5</c> in ODF,
/// <c>Revenue!$B$2:$B$5</c> in SpreadsheetML — so it is the case where a live range exists and
/// the cache is preferred anyway.
/// </remarks>
public sealed class SheetChartTests
{
    private static IDocument Open(string name)
        => new SpreadsheetReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    private static ContentSection Chart(IDocument document)
        => document.Content.Children.OfType<ContentSection>()
            .Single(section => section.Kind == SectionKind.Frame);

    private static List<string> Rows(ContentTable table)
        => [.. table.Children.OfType<ContentTableRow>()
                 .Select(row => string.Join('|', row.Children.Select(cell => cell.GetText())))];

    [Theory]
    [InlineData("chart-bar-sheet.fods")]
    [InlineData("chart-bar-sheet.ods")]
    [InlineData("chart-bar-sheet.xlsx")]
    public void AChartIsASectionOfItsOwnAfterTheSheet(string name)
    {
        using IDocument document = Open(name);
        List<ContentSection> sections =
            [.. document.Content.Children.OfType<ContentSection>()];

        // Not inside the sheet, because a sheet section holds exactly one table and a chart is
        // another one — and in ODF the frame is a child of the cell it is fastened to, so
        // nesting would put a whole table inside a table cell. It follows the sheet the way a
        // cell comment does.
        sections[0].Kind.ShouldBe(SectionKind.Sheet, name);
        sections.Count(section => section.Kind == SectionKind.Frame).ShouldBe(1, name);
        sections[^1].Kind.ShouldBe(SectionKind.Frame, name);
    }

    [Theory]
    [InlineData("chart-bar-sheet.fods")]
    [InlineData("chart-bar-sheet.ods")]
    [InlineData("chart-bar-sheet.xlsx")]
    public void TheChartCarriesItsTitleAndAxisTitles(string name)
    {
        using IDocument document = Open(name);
        ContentSection chart = Chart(document);

        chart.Name.ShouldBe("Regional revenue", name);
        chart.Children.OfType<ContentParagraph>().Select(p => p.GetText().Trim())
            .ShouldBe(["Regional revenue", "Quarter", "Units"], name);
    }

    [Theory]
    [InlineData("chart-bar-sheet.fods")]
    [InlineData("chart-bar-sheet.ods")]
    [InlineData("chart-bar-sheet.xlsx")]
    public void TheCachedNumbersAreTheSameInAllThreeFormats(string name)
    {
        using IDocument document = Open(name);
        ContentTable table = Chart(document).Children.OfType<ContentTable>().Single();

        Rows(table).ShouldBe(
            [
                "|North|South",
                "Q1|120|88",
                "Q2|95|132",
                "Q3|143|101",
                "Q4|168|121",
            ],
            name);
        table.HeaderRowCount.ShouldBe(1, name);
    }

    [Theory]
    [InlineData("chart-bar-sheet.fods")]
    [InlineData("chart-bar-sheet.ods")]
    [InlineData("chart-bar-sheet.xlsx")]
    public void TheCacheAgreesWithTheSheetItReferences(string name)
    {
        using IDocument document = Open(name);
        ContentTable sheet = document.Content.Children.OfType<ContentSection>()
            .First(section => section.Kind == SectionKind.Sheet)
            .Children.OfType<ContentTable>().Single();
        ContentTable chart = Chart(document).Children.OfType<ContentTable>().Single();

        // The point of preferring the cache is that it is what the authoring application last
        // computed, so on a file nobody has edited behind the application's back it agrees with
        // the range. Both are read here so that a divergence shows up as a test failure rather
        // than as a plausible-looking chart.
        Rows(chart).ShouldBe(Rows(sheet).Take(5).ToList(), name);
    }

    [Theory]
    [InlineData("chart-bar-sheet.fods")]
    [InlineData("chart-bar-sheet.ods")]
    [InlineData("chart-bar-sheet.xlsx")]
    public void AValueIsANumberAndNotJustItsText(string name)
    {
        using IDocument document = Open(name);
        ContentTableCell cell = Chart(document).Children.OfType<ContentTable>().Single()
            .Children.OfType<ContentTableRow>().Skip(1).First()
            .Children.OfType<ContentTableCell>().Skip(1).First();

        cell.Value.ShouldBe(120d, name);
    }
}

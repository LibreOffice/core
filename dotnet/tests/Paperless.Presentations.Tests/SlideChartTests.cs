using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// What a deck's reader makes of a chart on a slide.
/// </summary>
/// <remarks>
/// <c>chart-bar-deck.{fodp,odp,pptx}</c> is one hand-written flat deck and LibreOffice
/// 24.2.7.2's two conversions of it: a single slide holding nothing but a bar chart with a
/// title, two axis titles, two named series and four labelled categories. It reads the same in
/// all three, through two entirely separate paths — <c>chart:chart</c> in a sub-document for
/// the ODF pair and <c>c:chartSpace</c> in <c>ppt/charts/chart1.xml</c> for the PPTX — which is
/// the thing worth testing: a caller indexing a mixed corpus must not have to branch.
/// </remarks>
public sealed class SlideChartTests
{
    private static IDocument Open(string name)
        => new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    private static ContentSection Chart(IDocument document)
        => Descendants(document.Content).OfType<ContentSection>()
            .Single(section => section.Kind == SectionKind.Frame);

    private static IEnumerable<ContentNode> Descendants(ContentNode node)
    {
        foreach (ContentNode child in node.Children)
        {
            yield return child;
            foreach (ContentNode nested in Descendants(child)) yield return nested;
        }
    }

    [Theory]
    [InlineData("chart-bar-deck.fodp")]
    [InlineData("chart-bar-deck.odp")]
    [InlineData("chart-bar-deck.pptx")]
    public void AChartOnASlideIsASectionOfItsOwn(string name)
    {
        using IDocument document = Open(name);

        Chart(document).Name.ShouldBe("Regional revenue", name);
    }

    [Theory]
    [InlineData("chart-bar-deck.fodp")]
    [InlineData("chart-bar-deck.odp")]
    [InlineData("chart-bar-deck.pptx")]
    public void TheTitleAndTheAxisTitlesAreParagraphsInDocumentOrder(string name)
    {
        using IDocument document = Open(name);

        Chart(document).Children.OfType<ContentParagraph>().Select(p => p.GetText().Trim())
            .ShouldBe(["Regional revenue", "Quarter", "Units"], name);
    }

    [Theory]
    [InlineData("chart-bar-deck.fodp")]
    [InlineData("chart-bar-deck.odp")]
    [InlineData("chart-bar-deck.pptx")]
    public void TheSeriesAndCategoriesAreOneTable(string name)
    {
        using IDocument document = Open(name);
        ContentTable table = Chart(document).Children.OfType<ContentTable>().Single();

        List<string> rows =
            [.. table.Children.OfType<ContentTableRow>()
                 .Select(row => string.Join('|', row.Children.Select(cell => cell.GetText())))];

        rows.ShouldBe(
            [
                "|North|South",
                "Q1|120|88",
                "Q2|95|132",
                "Q3|143|101",
                "Q4|168|121",
            ],
            name);

        table.HeaderRowCount.ShouldBe(1, name);
        table.ColumnCount.ShouldBe(3, name);
    }

    [Theory]
    [InlineData("chart-bar-deck.fodp")]
    [InlineData("chart-bar-deck.odp")]
    [InlineData("chart-bar-deck.pptx")]
    public void TheThreeFormatsExtractToTheSameText(string name)
    {
        using IDocument document = Open(name);

        // The whole point of one content tree: the DrawingML chart part and the ODF chart
        // sub-document share no markup at all and must still come out identical.
        document.Content.GetText().Replace("\r", string.Empty, StringComparison.Ordinal)
            .Trim().ShouldBe(
                "Regional revenue\nQuarter\nUnits\n\tNorth\tSouth\nQ1\t120\t88\n"
                + "Q2\t95\t132\nQ3\t143\t101\nQ4\t168\t121",
                name);
    }

    [Theory]
    [InlineData("chart-bar-deck.fodp")]
    [InlineData("chart-bar-deck.odp")]
    [InlineData("chart-bar-deck.pptx")]
    public void AChartThatWasReadIsNoLongerAlsoAnUnnamedGraphic(string name)
    {
        using IDocument document = Open(name);

        // The ContentImage placeholder existed because nothing was read. Keeping it beside the
        // chart's content would report the slide as holding two things.
        Descendants(document.Content).OfType<ContentImage>().ShouldBeEmpty(name);
    }
}

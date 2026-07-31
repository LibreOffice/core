using Markdig.Extensions.Tables;
using Markdig.Syntax;
using Markdig.Syntax.Inlines;
using Paperless.Core.Extraction;
using Shouldly;

namespace Paperless.Markup.Tests;

/// <summary>
/// Stage two: the XHTML tree transformed into GitHub-Flavored Markdown.
/// </summary>
/// <remarks>
/// Asserted by parsing the emitted Markdown, not by matching it. The output's exact spelling is
/// this writer's business and may change; what it <em>parses as</em> is the contract.
/// </remarks>
public class MarkdownWriterTests
{
    private static MarkdownDocument Parse(ContentNode content)
        => Markdown.Parse(MarkdownWriter.ToMarkdown(content));

    [Fact]
    public void HeadingsKeepTheirLevel()
    {
        MarkdownDocument parsed = Parse(Tree.Document(Tree.Body(
            Tree.Heading(1, "One"), Tree.Heading(2, "Two"), Tree.Heading(6, "Six"))));

        parsed.OfType<HeadingBlock>().Select(h => h.Level).ShouldBe([1, 2, 6]);
    }

    [Fact]
    public void ListsKeepTheirKindAndTheirNesting()
    {
        MarkdownDocument parsed = Parse(Tree.Document(Tree.Body(
            Tree.Item(0, "•", "First"),
            Tree.Item(1, "1.", "Nested one"),
            Tree.Item(1, "2.", "Nested two"),
            Tree.Item(0, "•", "Second"))));

        ListBlock outer = parsed.OfType<ListBlock>().Single();
        outer.IsOrdered.ShouldBeFalse();
        outer.Count.ShouldBe(2);

        ListBlock inner = outer.Descendants<ListBlock>().Single();
        inner.IsOrdered.ShouldBeTrue();
        inner.Count.ShouldBe(2);
    }

    [Fact]
    public void AnOrderedListKeepsItsStartNumberButNotItsMarkerStyle()
    {
        // The named loss: GFM numbers with digits only, so "a)" and "iv." collapse to decimal.
        // The start survives because GFM has a start.
        ListBlock list = Parse(Tree.Document(Tree.Body(
            Tree.Item(0, "7.", "Seven"), Tree.Item(0, "8.", "Eight")))).OfType<ListBlock>().Single();

        list.IsOrdered.ShouldBeTrue();
        list.OrderedStart.ShouldBe("7");
    }

    [Fact]
    public void ASubListThatCouldNotInterruptAParagraphGetsItsBlankLine()
    {
        // The trap: a bullet sub-list can follow its item's text directly, but an ordered one
        // may only interrupt a paragraph when it starts at 1. Numbered from 7 and packed tight,
        // the whole sub-list is read as ordinary text and disappears into the line above.
        MarkdownDocument parsed = Parse(Tree.Document(Tree.Body(
            Tree.Item(0, "•", "Parent"),
            Tree.Item(1, "7.", "Seven"))));

        ListBlock inner = parsed.OfType<ListBlock>().Single().Descendants<ListBlock>().ShouldHaveSingleItem();
        inner.IsOrdered.ShouldBeTrue();
        inner.OrderedStart.ShouldBe("7");
    }

    [Fact]
    public void AContinuationParagraphStaysInsideItsItem()
    {
        ListBlock list = Parse(Tree.Document(Tree.Body(
            Tree.Item(0, "1.", "First"),
            Tree.Item(0, null, "Still first"),
            Tree.Item(0, "2.", "Second")))).OfType<ListBlock>().Single();

        list.Count.ShouldBe(2);
        ((ListItemBlock)list[0]).OfType<ParagraphBlock>().Count().ShouldBe(2);
    }

    [Fact]
    public void APlainTableBecomesAPipeTable()
    {
        Table table = Parse(Tree.Document(Tree.Body(Tree.Table(
            1,
            Tree.Row(0, Tree.Cell("Region"), Tree.Cell("Units")),
            Tree.Row(1, Tree.Cell("North"), Tree.Cell("12"))))))
            .Descendants<Table>().ShouldHaveSingleItem();

        table.Count.ShouldBe(2);
        ((TableRow)table[0]).IsHeader.ShouldBeTrue();
    }

    [Fact]
    public void ATableDeclaringNoHeaderRowsPromotesItsFirstRow()
    {
        // GFM has no headerless table, so something must give. The first row is what a sheet and
        // nearly every document table puts labels in; the alternative is an empty header band
        // above every table in every output.
        Table table = Parse(Tree.Document(Tree.Body(Tree.Table(
            0,
            Tree.Row(0, Tree.Cell("A"), Tree.Cell("B")),
            Tree.Row(1, Tree.Cell("1"), Tree.Cell("2"))))))
            .Descendants<Table>().ShouldHaveSingleItem();

        ((TableRow)table[0]).IsHeader.ShouldBeTrue();
        table.Count.ShouldBe(2);
    }

    /// <summary>
    /// Tables GFM cannot express, each for a different reason.
    /// </summary>
    public static TheoryData<string, ContentTable> Inexpressible() => new()
    {
        {
            "a column span",
            Tree.Table(0, Tree.Row(0, Tree.Cell("Wide", columnSpan: 2)), Tree.Row(1, Tree.Cell("a"), Tree.Cell("b")))
        },
        {
            "a row span",
            Tree.Table(0, Tree.Row(0, Tree.Cell("Tall", rowSpan: 2), Tree.Cell("b")), Tree.Row(1, Tree.Cell("c")))
        },
        {
            "a nested table",
            Tree.Table(0, Tree.Row(0, Tree.CellOf(Tree.Table(0, Tree.Row(0, Tree.Cell("inner"))))))
        },
        {
            "a cell holding two paragraphs",
            Tree.Table(0, Tree.Row(0, Tree.CellOf(Tree.Para("one"), Tree.Para("two"))))
        },
        {
            "a cell holding a list",
            Tree.Table(0, Tree.Row(0, Tree.CellOf(Tree.Item(0, "•", "bullet"))))
        },
        {
            "ragged rows",
            Tree.Table(0, Tree.Row(0, Tree.Cell("a"), Tree.Cell("b")), Tree.Row(1, Tree.Cell("c")))
        },
    };

    [Theory]
    [MemberData(nameof(Inexpressible))]
    public void ATableGfmCannotExpressFallsBackToHtmlRatherThanBeingFlattened(string why, ContentTable table)
    {
        // The settled decision, and the loss it names: a fallback table is not Markdown, so a
        // consumer that strips HTML rather than parsing it loses the table. The alternative —
        // splicing a nested table's rows into its parent, repeating a spanned cell — invents a
        // grid the document does not have and is silently wrong rather than visibly HTML.
        string markdown = MarkdownWriter.ToMarkdown(Tree.Document(Tree.Body(table)));
        MarkdownDocument parsed = Markdown.Parse(markdown);

        parsed.Descendants<Table>().ShouldBeEmpty($"{why} was flattened into a pipe table:\n{markdown}");
        parsed.OfType<HtmlBlock>().ShouldHaveSingleItem($"{why} produced no HTML block:\n{markdown}");
        markdown.ShouldContain("<table>", Case.Sensitive, why);
    }

    [Theory]
    [MemberData(nameof(Inexpressible))]
    public void AnHtmlFallbackTableContainsNoBlankLine(string why, ContentTable table)
    {
        // A blank line ends an HTML block in CommonMark, so one inside the fallback would leave
        // the remaining rows to be parsed as Markdown — and they would come out as literal tags.
        string markdown = MarkdownWriter.ToMarkdown(Tree.Document(Tree.Body(table)));
        int start = markdown.IndexOf("<table>", StringComparison.Ordinal);
        int end = markdown.LastIndexOf("</table>", StringComparison.Ordinal);

        markdown[start..end].ShouldNotContain("\n\n", Case.Sensitive, why);
    }

    [Fact]
    public void AnHtmlFallbackKeepsTheSpansThatSentItThere()
    {
        string markdown = MarkdownWriter.ToMarkdown(Tree.Document(Tree.Body(Tree.Table(
            0,
            Tree.Row(0, Tree.Cell("Wide", columnSpan: 2)),
            Tree.Row(1, Tree.Cell("a"), Tree.Cell("b"))))));

        markdown.ShouldContain("colspan=\"2\"");
    }

    [Fact]
    public void SpeakerNotesAreSetApartFromTheSlide()
    {
        MarkdownDocument parsed = Parse(Tree.Document(
            Tree.Section(SectionKind.Slide, null, Tree.Para("On the slide")),
            Tree.Section(SectionKind.SlideNotes, null, Tree.Para("For the speaker"))));

        QuoteBlock quote = parsed.OfType<QuoteBlock>().ShouldHaveSingleItem();
        Markdown.Flatten(Markdown.Text(quote)).ShouldBe("Speaker notes For the speaker");
    }

    [Fact]
    public void ASlideGetsAHeadingAndItsNumber()
    {
        // Numbered from the section's own index rather than from a running count, so that
        // excluding a hidden slide leaves the rest with the numbers the document gives them.
        Parse(Tree.Document(
            Tree.Section(SectionKind.Slide, null, 0, Tree.Para("first")),
            Tree.Section(SectionKind.Slide, null, 2, Tree.Para("third"))))
            .OfType<HeadingBlock>().Select(Markdown.Text).ShouldBe(["Slide 1", "Slide 3"]);
    }

    [Fact]
    public void ASheetGetsItsTabNameAsAHeading()
    {
        Parse(Tree.Document(Tree.Section(SectionKind.Sheet, "Q1 actuals", Tree.Para("x"))))
            .OfType<HeadingBlock>().Select(Markdown.Text).ShouldBe(["Q1 actuals"]);
    }

    [Fact]
    public void AFootnoteIsSetApartAndKeepsTheNumberTheDocumentGaveIt()
    {
        // Not GFM's [^1], and that is settled on evidence: a footnote definition with no
        // reference is dropped outright by a conformant renderer, and the content tree carries
        // no inline anchor to reference it from — a reader bakes the anchor number into the text
        // of the run beside it. So the note is set apart the way a comment is, and its text
        // survives, which [^1] would not.
        string markdown = MarkdownWriter.ToMarkdown(Tree.Document(
            Tree.Body(Tree.Para("Body text.")),
            Tree.Section(SectionKind.Note, "1", Tree.Para("The note."))));

        markdown.ShouldContain("> **Note 1**");
        Markdown.PlainText(markdown).ShouldContain("The note.");
    }

    [Fact]
    public void AnUnreferencedGfmFootnoteDefinitionWouldLoseItsText()
    {
        // The measurement the decision above rests on, pinned so that it is a fact in the suite
        // rather than a claim in a comment. If a future parser starts rendering these, the
        // decision is worth revisiting and this test is what will say so.
        Markdown.PlainText("Body text.\n\n[^1]: The note.\n").ShouldNotContain("The note.");
    }

    [Fact]
    public void HeadersFootersCommentsAndFramesAreLabelledAndSetApart()
    {
        MarkdownDocument parsed = Parse(Tree.Document(
            Tree.Body(Tree.Para("Body.")),
            Tree.Section(SectionKind.Header, null, Tree.Para("Running head")),
            Tree.Section(SectionKind.Footer, null, Tree.Para("Page foot")),
            Tree.Section(SectionKind.Comment, "Ada Lovelace", Tree.Para("A remark")),
            Tree.Section(SectionKind.Frame, null, Tree.Para("In a box"))));

        parsed.OfType<QuoteBlock>().Select(q => Markdown.Flatten(Markdown.Text(q))).ShouldBe(
        [
            "Header Running head",
            "Footer Page foot",
            "Comment — Ada Lovelace A remark",
            "Text frame In a box",
        ]);
    }

    [Fact]
    public void AHyperlinkKeepsItsTarget()
    {
        ContentParagraph paragraph = Tree.Runs(
            new ContentRun { Text = "see this", HyperlinkTarget = "https://example.com/x" });

        LinkInline link = Parse(Tree.Document(Tree.Body(paragraph))).Descendants<LinkInline>().Single();
        link.Url.ShouldBe("https://example.com/x");
        link.IsImage.ShouldBeFalse();
    }

    [Fact]
    public void AnImageBecomesAnImageWithItsAlternativeText()
    {
        ContentImage image = new() { AlternativeText = "A chart", PartName = "media/image1.png" };

        LinkInline link = Parse(Tree.Document(Tree.Body(image))).Descendants<LinkInline>().Single();
        link.IsImage.ShouldBeTrue();
        link.Url.ShouldBe("media/image1.png");
        Markdown.Text(link).ShouldBe("A chart");
    }

    [Fact]
    public void UnderlineAndSuperscriptKeepTheirTextAndLoseTheirFormatting()
    {
        // Named losses rather than accidents: GFM has no syntax for either, so the alternatives
        // are raw HTML in the middle of a sentence or silently dropping the words.
        ContentParagraph paragraph = Tree.Runs(
            new ContentRun { Text = "under", Emphasis = RunEmphasis.Underline },
            new ContentRun { Text = "high", Emphasis = RunEmphasis.Superscript });

        string markdown = MarkdownWriter.ToMarkdown(Tree.Document(Tree.Body(paragraph)));
        Markdown.Flatten(Markdown.PlainText(markdown)).ShouldBe("underhigh");
        markdown.ShouldNotContain("<u>");
        markdown.ShouldNotContain("<sup>");
    }

    [Fact]
    public void StrikethroughSurvivesBecauseGfmHasIt()
    {
        MarkdownWriter.ToMarkdown(Tree.Document(Tree.Body(
            Tree.Para("gone", RunEmphasis.Strikethrough)))).ShouldContain("~~gone~~");
    }

    [Fact]
    public void ATruncatedTableSaysSoInTheMarkdownToo()
    {
        ContentTableRow[] rows = [.. Enumerable.Range(0, 20).Select(i => Tree.Row(i, Tree.Cell("r" + i)))];

        MarkdownWriter.ToMarkdown(
            Tree.Document(Tree.Body(Tree.Table(0, rows))),
            new MarkupOptions { MaxTableRows = 5 })
            .ShouldContain("15 further rows omitted");
    }

    [Fact]
    public void TheTwoStagesAgreeWhenDrivenSeparately()
    {
        // The contract that makes the split worth having: stage two is a function of stage one's
        // tree, so a caller who wants both outputs walks the content tree exactly once.
        ContentDocument document = Tree.Document(Tree.Body(
            Tree.Heading(1, "Title"), Tree.Para("Body"), Tree.Item(0, "•", "Point")));

        MarkdownWriter.FromXhtml(XhtmlWriter.ToDocument(document))
            .ShouldBe(MarkdownWriter.ToMarkdown(document));
    }

}

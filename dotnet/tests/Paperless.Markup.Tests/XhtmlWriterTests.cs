using System.Xml.Linq;
using Paperless.Core.Extraction;
using Shouldly;

namespace Paperless.Markup.Tests;

/// <summary>
/// Stage one: the content tree projected onto semantic XHTML.
/// </summary>
/// <remarks>
/// The assertions here are on the tree rather than on the serialised string, because the string
/// is what stage two never sees — <see cref="MarkdownWriter"/> consumes the
/// <see cref="XDocument"/>. Serialisation is asserted separately and only where it can go
/// wrong on its own.
/// </remarks>
public class XhtmlWriterTests
{
    private static XElement Body(ContentNode content, MarkupOptions? options = null)
        => XhtmlWriter.ToDocument(content, options).Root!.Element(Xhtml.Namespace + "body")!;

    private static IEnumerable<XElement> Find(XElement root, string name)
        => root.Descendants(Xhtml.Namespace + name);

    [Fact]
    public void HeadingsKeepTheirLevel()
    {
        XElement body = Body(Tree.Document(Tree.Body(
            Tree.Heading(1, "One"), Tree.Heading(2, "Two"), Tree.Heading(3, "Three"))));

        Find(body, "h1").Select(e => e.Value).ShouldBe(["One"]);
        Find(body, "h2").Select(e => e.Value).ShouldBe(["Two"]);
        Find(body, "h3").Select(e => e.Value).ShouldBe(["Three"]);
    }

    [Fact]
    public void AnOutlineLevelBelowSixIsClampedRatherThanInvented()
    {
        // Word's outline levels run to nine and HTML stops at six. Flattening keeps them
        // headings, which is the part that carries meaning; an <h9> means nothing to anything.
        XElement body = Body(Tree.Document(Tree.Body(Tree.Heading(9, "Deep"))));

        Find(body, "h6").ShouldHaveSingleItem().Value.ShouldBe("Deep");
    }

    [Fact]
    public void AHeadingDoesNotCarryTheEmphasisItsStyleSupplies()
    {
        // Every heading style in every format sets bold. Emitting it would wrap essentially
        // every heading in the corpus in <strong> and say nothing the h-level has not said.
        ContentParagraph heading = new() { HeadingLevel = 1 };
        heading.Children.Add(new ContentRun { Text = "Title", Emphasis = RunEmphasis.Bold | RunEmphasis.Italic });

        Find(Body(Tree.Document(Tree.Body(heading))), "strong").ShouldBeEmpty();
    }

    [Fact]
    public void FlatListParagraphsBecomeNestedLists()
    {
        // The shape every one of the four formats stores: a level and a rendered marker per
        // paragraph, with the ul/ol nesting left to be inferred.
        XElement body = Body(Tree.Document(Tree.Body(
            Tree.Item(0, "•", "First"),
            Tree.Item(1, "◦", "Nested"),
            Tree.Item(0, "•", "Third"))));

        XElement outer = Find(body, "ul").First();
        outer.Elements(Xhtml.Namespace + "li").Count().ShouldBe(2);
        XElement inner = Find(outer, "ul").ShouldHaveSingleItem();
        inner.Parent!.Name.LocalName.ShouldBe("li");
        inner.Value.Trim().ShouldBe("Nested");
    }

    [Fact]
    public void AMarkerCarryingALetterOrDigitMeansAnOrderedList()
    {
        // Decided from the marker rather than from a list of bullet glyphs, because there is no
        // end to that list: dingbats, images rendered as characters, CJK markers.
        Body(Tree.Document(Tree.Body(Tree.Item(0, "1.", "x")))).Descendants().Select(e => e.Name.LocalName)
            .ShouldContain("ol");
        Body(Tree.Document(Tree.Body(Tree.Item(0, "a)", "x")))).Descendants().Select(e => e.Name.LocalName)
            .ShouldContain("ol");
        Body(Tree.Document(Tree.Body(Tree.Item(0, "▪", "x")))).Descendants().Select(e => e.Name.LocalName)
            .ShouldContain("ul");
    }

    [Fact]
    public void AListParagraphWithNoMarkerContinuesTheItemAboveIt()
    {
        // How ODF and OOXML both express "a second paragraph inside item two". Starting a new
        // item for it would invent a list entry the document does not have.
        XElement body = Body(Tree.Document(Tree.Body(
            Tree.Item(0, "1.", "First"),
            Tree.Item(0, null, "Still first"))));

        XElement item = Find(body, "li").ShouldHaveSingleItem();
        item.Elements(Xhtml.Namespace + "p").Select(p => p.Value).ShouldBe(["First", "Still first"]);
    }

    [Fact]
    public void ABulletedListFollowedByANumberedOneAtTheSameLevelIsTwoLists()
    {
        XElement body = Body(Tree.Document(Tree.Body(
            Tree.Item(0, "•", "Bullet"),
            Tree.Item(0, "1.", "Number"))));

        Find(body, "ul").Count().ShouldBe(1);
        Find(body, "ol").Count().ShouldBe(1);
    }

    [Fact]
    public void AListStartingAtSomethingOtherThanOneRecordsItsStart()
    {
        Body(Tree.Document(Tree.Body(Tree.Item(0, "7.", "Seven"))))
            .Descendants(Xhtml.Namespace + "ol").Single().Attribute("start")!.Value.ShouldBe("7");
    }

    [Fact]
    public void ALevelJumpGetsAPlaceholderItemRatherThanAnOrphanList()
    {
        // A document that goes straight from level 0 to level 2 is malformed and common. The
        // nested list still has to hang off an <li> or the XHTML is not a list at all.
        XElement body = Body(Tree.Document(Tree.Body(
            Tree.Item(0, "•", "Top"),
            Tree.Item(2, "•", "Two levels down"))));

        Find(body, "ul").Count().ShouldBe(3);
        Find(body, "li").Count().ShouldBe(3);
    }

    [Fact]
    public void SpansAndNestingSurviveStageOne()
    {
        // The whole reason the lossy step happens last: GFM has neither of these, and XHTML has
        // both, so nothing is lost before the projection that cannot carry them.
        ContentTableCell nesting = Tree.CellOf(
            Tree.Para("Above"),
            Tree.Table(0, Tree.Row(0, Tree.Cell("Inner"))));

        XElement body = Body(Tree.Document(Tree.Body(Tree.Table(
            0,
            Tree.Row(0, Tree.Cell("Wide", columnSpan: 2)),
            Tree.Row(1, nesting, Tree.Cell("Tall", rowSpan: 2))))));

        Find(body, "td").First().Attribute("colspan")!.Value.ShouldBe("2");
        Find(body, "td").Single(c => c.Attribute("rowspan") is not null)
            .Attribute("rowspan")!.Value.ShouldBe("2");
        Find(body, "table").Count().ShouldBe(2);
    }

    [Fact]
    public void HeaderRowsBecomeTheadAndTh()
    {
        XElement body = Body(Tree.Document(Tree.Body(Tree.Table(
            1,
            Tree.Row(0, Tree.Cell("Label")),
            Tree.Row(1, Tree.Cell("Value"))))));

        Find(body, "thead").ShouldHaveSingleItem();
        Find(body, "th").ShouldHaveSingleItem().Value.Trim().ShouldBe("Label");
        Find(body, "td").ShouldHaveSingleItem().Value.Trim().ShouldBe("Value");
    }

    [Fact]
    public void ATableIsBoundedAndSaysSo()
    {
        // A sheet's used range is routinely tens of thousands of rows. The bound is the point;
        // announcing it is what stops a truncated table from reading as a complete one.
        ContentTableRow[] rows = [.. Enumerable.Range(0, 50).Select(i => Tree.Row(i, Tree.Cell("r" + i)))];
        XElement body = Body(
            Tree.Document(Tree.Body(Tree.Table(0, rows))),
            new MarkupOptions { MaxTableRows = 10 });

        Find(body, "tr").Count().ShouldBe(10);
        Find(body, "p").Single(p => p.Attribute("class")?.Value == "truncation")
            .Value.ShouldContain("40 further rows");
    }

    [Fact]
    public void EachSectionKindGetsTheElementThatSaysWhatItIs()
    {
        XElement body = Body(Tree.Document(
            Tree.Section(SectionKind.Slide, null, Tree.Para("Slide text")),
            Tree.Section(SectionKind.SlideNotes, null, Tree.Para("Notes")),
            Tree.Section(SectionKind.Header, "Standard", Tree.Para("Head")),
            Tree.Section(SectionKind.Footer, "Standard", Tree.Para("Foot")),
            Tree.Section(SectionKind.Comment, "Ada", Tree.Para("Remark")),
            Tree.Section(SectionKind.Note, "1", Tree.Para("Footnote"))));

        body.Elements().Select(e => e.Name.LocalName + "." + e.Attribute("class")!.Value).ShouldBe(
        [
            "section.slide", "aside.speaker-notes", "header.page-header",
            "footer.page-footer", "aside.comment", "aside.note",
        ]);
    }

    [Fact]
    public void SheetsAndSlidesGetAHeadingBecauseNeitherFamilyPutsOneInItsContent()
    {
        XElement body = Body(Tree.Document(
            Tree.Section(SectionKind.Sheet, "Q1 actuals", Tree.Para("x")),
            Tree.Section(SectionKind.Slide, null, Tree.Para("y"))));

        Find(body, "h1").Select(h => h.Value).ShouldBe(["Q1 actuals", "Slide 1"]);
    }

    [Fact]
    public void ABodySectionGetsNoSyntheticHeading()
    {
        // A word-processing document supplies its own headings; a synthetic one would sit above
        // them at the same level and make every document look like it had two h1s.
        Find(Body(Tree.Document(Tree.Body(Tree.Heading(1, "Real")))), "h1")
            .ShouldHaveSingleItem().Value.ShouldBe("Real");
    }

    [Fact]
    public void HiddenSectionsAreMarkedAndCanBeExcluded()
    {
        ContentSection hidden = new() { Kind = SectionKind.Slide, IsHidden = true };
        hidden.Children.Add(Tree.Para("Skipped"));
        ContentDocument document = Tree.Document(hidden);

        Body(document).Elements().ShouldHaveSingleItem()
            .Attribute("data-hidden")!.Value.ShouldBe("true");
        Body(document, new MarkupOptions { IncludeHiddenSections = false }).Elements().ShouldBeEmpty();
    }

    [Fact]
    public void ARunsEmphasisNestsInAStableOrder()
    {
        ContentParagraph paragraph = Tree.Runs(new ContentRun
        {
            Text = "loud",
            Emphasis = RunEmphasis.Bold | RunEmphasis.Italic | RunEmphasis.Strikethrough,
        });

        XElement strong = Find(Body(Tree.Document(Tree.Body(paragraph))), "strong").ShouldHaveSingleItem();
        strong.Elements().ShouldHaveSingleItem().Name.LocalName.ShouldBe("em");
        Find(strong, "s").ShouldHaveSingleItem().Value.ShouldBe("loud");
    }

    [Fact]
    public void ALineBreakInsideARunBecomesBr()
    {
        // The content tree stores a hard break as a newline inside the run's text. Left as a
        // newline, HTML would collapse it into a space and the break would vanish.
        Find(Body(Tree.Document(Tree.Body(Tree.Para("first\nsecond")))), "br").ShouldHaveSingleItem();
    }

    [Fact]
    public void ARunInADifferentLanguageIsTaggedAndOneInTheDocumentsIsNot()
    {
        ContentDocument document = new()
        {
            Metadata = Core.Documents.DocumentMetadata.Empty with { Language = "en-GB" },
        };
        ContentSection section = Tree.Body(
            Tree.Runs(new ContentRun { Text = "home", Language = "en-GB" }),
            Tree.Runs(new ContentRun { Text = "auswärts", Language = "de-DE" }));
        document.Children.Add(section);

        XElement body = Body(document);
        body.Descendants().Where(e => e.Attribute(XNamespace.Xml + "lang") is not null)
            .Select(e => e.Value).ShouldBe(["auswärts"]);
    }

    [Fact]
    public void AnImageBecomesAPlaceholderPointingAtThePartThatHoldsIt()
    {
        // Extraction records that a graphic exists but does not decode it, so the container part
        // is the only real reference available — and an empty src means "this page" in HTML,
        // which is worse than saying nothing.
        ContentImage image = new()
        {
            AlternativeText = "A chart",
            MediaType = "image/png",
            PartName = "word/media/image1.png",
        };

        XElement element = Find(Body(Tree.Document(Tree.Body(image))), "img").ShouldHaveSingleItem();
        element.Attribute("src")!.Value.ShouldBe("word/media/image1.png");
        element.Attribute("alt")!.Value.ShouldBe("A chart");
    }

    [Fact]
    public void AQuotationStyleBecomesABlockquote()
    {
        Find(Body(Tree.Document(Tree.Body(Tree.Styled("Quotations", "As it was said")))), "blockquote")
            .ShouldHaveSingleItem().Value.Trim().ShouldBe("As it was said");
    }

    [Fact]
    public void ACodeCharacterStyleBecomesCode()
    {
        ContentParagraph paragraph = Tree.Runs(new ContentRun { Text = "x = 1", StyleName = "Source Text" });

        Find(Body(Tree.Document(Tree.Body(paragraph))), "code").ShouldHaveSingleItem().Value.ShouldBe("x = 1");
    }

    [Fact]
    public void TextIsEscapedByThePlatformRatherThanByHand()
    {
        // The one place in the two stages where the platform already has the right answer,
        // including the characters XML forbids outright.
        string xhtml = XhtmlWriter.ToXhtml(Tree.Prose("a < b & c > d \"quoted\""));

        xhtml.ShouldContain("a &lt; b &amp; c &gt; d \"quoted\"");
        XDocument.Parse(xhtml).Descendants(Xhtml.Namespace + "p").Single().Value
            .ShouldBe("a < b & c > d \"quoted\"");
    }

    [Fact]
    public void AnEmptyParagraphIsNotSelfClosed()
    {
        // Well-formed as XML and catastrophic as HTML: a parser reads <p/> as an opening tag and
        // swallows the rest of the document. An empty table cell produces exactly this.
        string xhtml = XhtmlWriter.ToXhtml(
            Tree.Document(Tree.Body(Tree.Table(0, Tree.Row(0, Tree.Cell(string.Empty))))));

        xhtml.ShouldNotContain("<p/>");
        xhtml.ShouldNotContain("<p />");
        xhtml.ShouldContain("<p></p>");
    }

    [Fact]
    public void TheSerialisedDocumentIsWellFormedAndDeclaresUtf8()
    {
        string xhtml = XhtmlWriter.ToXhtml(Tree.Prose("plain"));

        // XmlWriter over a StringBuilder reports utf-16 whatever the settings say, which would
        // be a lie in a file callers save as UTF-8.
        xhtml.ShouldStartWith("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE html>\n");
        Should.NotThrow(() => XDocument.Parse(xhtml));
    }

    [Fact]
    public void IndentationNeverLandsBetweenTwoInlineElements()
    {
        // The trap that cost the most time here. XmlWriter's own Indent setting decides
        // progressively and would indent <p><strong>a</strong><em>b</em></p> into three lines,
        // inserting a space between "a" and "b" that the document does not contain.
        ContentParagraph paragraph = Tree.Runs(
            new ContentRun { Text = "a", Emphasis = RunEmphasis.Bold },
            new ContentRun { Text = "b", Emphasis = RunEmphasis.Italic });

        XhtmlWriter.ToXhtml(Tree.Document(Tree.Body(paragraph)))
            .ShouldContain("<p><strong>a</strong><em>b</em></p>");
    }
}

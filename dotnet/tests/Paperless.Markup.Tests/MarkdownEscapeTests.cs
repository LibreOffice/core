using Markdig.Syntax;
using Markdig.Syntax.Inlines;
using Paperless.Core.Extraction;
using Shouldly;

namespace Paperless.Markup.Tests;

/// <summary>
/// The escaping tests, which come first because everything after them is worthless if these
/// fail.
/// </summary>
/// <remarks>
/// Every one of these asserts by <em>parsing the output back</em>, never by matching the
/// emitted string. An assertion on the string would encode this writer's escaping choices and
/// would pass just as happily for a wrong one; the question that matters is whether an
/// independent parser reads the same text out that the document put in.
/// </remarks>
public class MarkdownEscapeTests
{
    /// <summary>
    /// Text that is entirely ordinary in a document and entirely dangerous in Markdown.
    /// </summary>
    /// <remarks>
    /// Each entry is a construct rather than a character, because a character is only dangerous
    /// in the positions where it can begin something: an asterisk anywhere, a hash only at the
    /// start of a line, a digit only when a "." follows it.
    /// </remarks>
    public static TheoryData<string> Hazards() =>
    [
        "*not emphasis*",
        "**not strong**",
        "_not emphasis_",
        "a*b*c intraword",
        "# not a heading",
        "## not a heading either",
        "> not a quotation",
        "- not a bullet",
        "+ not a bullet",
        "1. not an ordered list",
        "42) not an ordered list",
        "--- not a thematic break",
        "=== not a setext underline",
        "`not code`",
        "~~not struck~~",
        "~one tilde~",
        "[not a link](https://example.com/)",
        "![not an image](picture.png)",
        "<b>not html</b>",
        "<https://example.com/> not an autolink",
        "&amp; and &#65; are literal",
        @"C:\Users\ada\file.txt",
        "a | b | c",
        "|leading pipe",
        "trailing backslash \\",
        "***three stars***",
        "___three underscores___",
        "    four leading spaces",
        "\tleading tab",
        "1) 2) 3)",
        "^ % $ @ ! ? : ; \" '",
        "text ending in a hash #",
    ];

    [Theory]
    [MemberData(nameof(Hazards))]
    public void LiteralMarkdownSyntaxSurvivesAsText(string text)
    {
        // Whitespace is normalised on both sides and nothing else is. Markdown cannot preserve
        // a paragraph's leading indentation — four spaces there is an indented code block — so
        // the writer trims it, and that is the one difference allowed.
        string actual = Markdown.RoundTrip(Tree.Prose(text));

        Markdown.Flatten(actual).ShouldBe(
            Markdown.Flatten(text),
            $"emitted:\n{MarkdownWriter.ToMarkdown(Tree.Prose(text))}");
    }

    [Theory]
    [MemberData(nameof(Hazards))]
    public void LiteralMarkdownSyntaxStaysOneParagraph(string text)
    {
        // Stronger than the text check and catching a different failure: text can survive
        // intact while the block structure is destroyed. "# not a heading" emitted raw still
        // says "not a heading" and is a heading.
        MarkdownDocument parsed = Markdown.Parse(MarkdownWriter.ToMarkdown(Tree.Prose(text)));

        parsed.Count.ShouldBe(1, $"emitted:\n{MarkdownWriter.ToMarkdown(Tree.Prose(text))}");
        parsed[0].ShouldBeOfType<ParagraphBlock>();
    }

    [Theory]
    [MemberData(nameof(Hazards))]
    public void LiteralMarkdownSyntaxCarriesNoInlineConstructs(string text)
    {
        MarkdownDocument parsed = Markdown.Parse(MarkdownWriter.ToMarkdown(Tree.Prose(text)));
        ParagraphBlock paragraph = parsed.OfType<ParagraphBlock>().Single();

        // Every inline in the paragraph must be a literal or a plain escape. An EmphasisInline,
        // a LinkInline or a CodeInline here means a document's punctuation became syntax.
        foreach (Inline inline in paragraph.Inline!)
        {
            inline.ShouldBeAssignableTo<LeafInline>(
                $"'{text}' produced a {inline.GetType().Name}:\n"
                + MarkdownWriter.ToMarkdown(Tree.Prose(text)));
        }
    }

    [Theory]
    [MemberData(nameof(Hazards))]
    public void LiteralMarkdownSyntaxSurvivesInsideATableCell(string text)
    {
        // A table cell is the tightest position in Markdown: a row is one line, so an unescaped
        // pipe does not corrupt the text, it silently splits the row and shifts every cell after
        // it into the wrong column.
        ContentDocument document = Tree.Document(Tree.Body(Tree.Table(
            1,
            Tree.Row(0, Tree.Cell("Header A"), Tree.Cell("Header B")),
            Tree.Row(1, Tree.Cell(text), Tree.Cell("sentinel")))));

        string markdown = MarkdownWriter.ToMarkdown(document);
        Markdig.Extensions.Tables.Table table =
            Markdown.Parse(markdown).Descendants<Markdig.Extensions.Tables.Table>().Single();

        Markdig.Extensions.Tables.TableRow row = (Markdig.Extensions.Tables.TableRow)table[1];
        row.Count.ShouldBe(2, $"the row split. emitted:\n{markdown}");
        Markdown.Flatten(CellText(row[0])).ShouldBe(Markdown.Flatten(text), markdown);
        CellText(row[1]).Trim().ShouldBe("sentinel", markdown);
    }

    private static string CellText(Markdig.Syntax.MarkdownObject cell)
    {
        System.Text.StringBuilder text = new();
        foreach (LiteralInline literal in cell.Descendants<LiteralInline>()) text.Append(literal.Content.ToString());
        return text.ToString();
    }

    [Fact]
    public void IntrawordUnderscoresAreLeftAlone()
    {
        // The readability half of the escaping decision, and the reason the escaper works by
        // construct rather than by character class. CommonMark cannot read an underscore between
        // two alphanumerics as emphasis, so escaping one there is pure noise — and identifiers
        // and file names are full of them.
        MarkdownWriter.ToMarkdown(Tree.Prose("snake_case_identifier"))
            .ShouldContain("snake_case_identifier");
    }

    [Fact]
    public void AWordBoundaryUnderscoreIsEscaped()
    {
        MarkdownWriter.ToMarkdown(Tree.Prose("_emphasis_ here")).ShouldContain(@"\_emphasis\_");
    }

    [Fact]
    public void EmphasisAppliedByTheDocumentStillReadsAsEmphasis()
    {
        // The other direction, and it needs asserting too: escaping that swallowed the writer's
        // own delimiters would pass every test above and emit nothing but literals.
        MarkdownDocument parsed = Markdown.Parse(MarkdownWriter.ToMarkdown(
            Tree.Document(Tree.Body(Tree.Para("emphatic", RunEmphasis.Bold)))));

        parsed.Descendants<EmphasisInline>().ShouldHaveSingleItem().DelimiterCount.ShouldBe(2);
    }

    [Fact]
    public void ADestinationContainingSpacesIsWrappedRatherThanRewritten()
    {
        ContentParagraph paragraph = Tree.Runs(new ContentRun
        {
            Text = "link",
            HyperlinkTarget = "https://example.com/a file (v2).html",
        });

        string markdown = MarkdownWriter.ToMarkdown(Tree.Document(Tree.Body(paragraph)));
        LinkInline link = Markdown.Parse(markdown).Descendants<LinkInline>().Single();

        // A document's hyperlink target is not ours to percent-encode: the URL that comes back
        // out has to be the one that went in, character for character.
        link.Url.ShouldBe("https://example.com/a file (v2).html", markdown);
    }

    [Fact]
    public void AParenthesisedDestinationDoesNotTruncateTheLink()
    {
        ContentParagraph paragraph = Tree.Runs(new ContentRun
        {
            Text = "wiki",
            HyperlinkTarget = "https://example.com/Foo_(disambiguation)",
        });

        string markdown = MarkdownWriter.ToMarkdown(Tree.Document(Tree.Body(paragraph)));
        Markdown.Parse(markdown).Descendants<LinkInline>().Single().Url
            .ShouldBe("https://example.com/Foo_(disambiguation)", markdown);
    }

    [Fact]
    public void TextThatWouldCloseACodeSpanLengthensTheFence()
    {
        ContentParagraph paragraph = Tree.Runs(new ContentRun
        {
            Text = "a ` backtick",
            StyleName = "Source Text",
        });

        string markdown = MarkdownWriter.ToMarkdown(Tree.Document(Tree.Body(paragraph)));
        CodeInline code = Markdown.Parse(markdown).Descendants<CodeInline>().Single();

        code.Content.ShouldBe("a ` backtick", markdown);
    }

    [Fact]
    public void ARunEndingInWhitespaceKeepsItsEmphasisOutsideTheDelimiters()
    {
        // "** bold **" is not emphasis in any implementation, and a bold run that ends with the
        // space before the next word is the commonest shape there is.
        ContentParagraph paragraph = Tree.Runs(
            new ContentRun { Text = "bold ", Emphasis = RunEmphasis.Bold },
            new ContentRun { Text = "plain" });

        string markdown = MarkdownWriter.ToMarkdown(Tree.Document(Tree.Body(paragraph)));
        Markdown.Parse(markdown).Descendants<EmphasisInline>().ShouldHaveSingleItem();
        Markdown.Flatten(Markdown.PlainText(markdown)).ShouldBe("bold plain");
    }
}

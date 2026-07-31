using System.Text;
using System.Text.RegularExpressions;
using System.Xml.Linq;
using Markdig.Syntax;
using Paperless.Core.Extraction;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Markup.Tests;

/// <summary>
/// Real documents through both stages, checked against the content tree they came from.
/// </summary>
/// <remarks>
/// <para>
/// The hand-built trees elsewhere in this project state exactly what went in and can construct
/// cases no corpus file contains; these do the opposite job. A real document is where the
/// awkward combinations actually occur — a run that is bold and struck and a hyperlink at once,
/// a merged cell in a table whose neighbour is nested — and the assertion here is the one that
/// survives all of it: <b>nothing the content tree says may be lost by either projection, and
/// neither may say anything the tree does not.</b>
/// </para>
/// <para>
/// No LibreOffice needed. Comparing against the reference is a different question and lives in
/// <c>Paperless.Fidelity.Tests</c>; what is checked here is internal consistency, which is where
/// an escaping bug shows up.
/// </para>
/// </remarks>
public partial class CorpusRoundTripTests
{
    /// <summary>
    /// One document per family and per format family, chosen to cover what the writers do.
    /// </summary>
    /// <remarks>
    /// <c>sheet-xls.xls</c> and <c>sheet-csv.csv</c> are deliberately absent: those readers are
    /// not implemented yet, and a test that skipped on <c>UnsupportedFormatException</c> would
    /// go on reporting a skip long after they were. Add them when they read.
    /// </remarks>
    public static TheoryData<string> Documents() =>
    [
        "text-features.odt",
        "text-features-flat.fodt",
        "word-features.docx",
        "word-features.doc",
        "word-features.rtf",
        "tables.odt",
        "tables.docx",
        "table-nested.odt",
        "table-nested.docx",
        "deck-features.pptx",
        "slides-odp.odp",
        "slides-ppt.ppt",
        "sheet-xlsx.xlsx",
        "sheet-ods.ods",
        "prose-doc.doc",
        "prose-rtf.rtf",
    ];

    private static ContentDocument Extract(string name)
        => PaperlessDocument.Extract(Corpus.Require(name));

    [Theory]
    [MemberData(nameof(Documents))]
    public void TheXhtmlSaysWhatTheTreeSays(string name)
    {
        ContentDocument tree = Extract(name);
        XElement body = XhtmlWriter.ToDocument(tree).Root!.Element(Xhtml.Namespace + "body")!;

        Words(XhtmlText(body)).ShouldBeASubsequenceOf(
            Words(tree.GetText()),
            allowedAdditions: SyntheticWords(tree),
            because: XhtmlWriter.ToXhtml(tree));
    }

    [Theory]
    [MemberData(nameof(Documents))]
    public void TheMarkdownSaysWhatTheTreeSays(string name)
    {
        // The test the whole exercise exists for. An escaping bug leaves every character in the
        // output — a text diff against the tree would match — and only reading it back with a
        // parser shows that an asterisk ate the word beside it.
        ContentDocument tree = Extract(name);
        string markdown = MarkdownWriter.ToMarkdown(tree);

        // The list markers are dropped on the tree side rather than allowed for on the other:
        // GFM renumbers an ordered list with its own digits and renders a bullet from the list
        // kind, so "•" and "a)" are not words the Markdown can be expected to contain.
        Words(StripTags(Markdown.PlainText(markdown))).ShouldBeASubsequenceOf(
            Words(TreeText(tree, includeListMarkers: false)),
            allowedAdditions: SyntheticWords(tree),
            because: markdown);
    }

    [Theory]
    [MemberData(nameof(Documents))]
    public void TheMarkdownCarriesNoHtmlBeyondTheTableFallback(string name)
    {
        // An HTML block is expected exactly where a table needed the fallback. One anywhere else
        // means a "<" escaped its way out of a run and started raw HTML mid-document.
        string markdown = MarkdownWriter.ToMarkdown(Extract(name));

        foreach (HtmlBlock html in Markdown.Parse(markdown).Descendants<HtmlBlock>())
        {
            markdown[html.Span.Start..(html.Span.End + 1)].TrimStart()
                .ShouldStartWith("<table", Case.Sensitive, markdown);
        }
    }

    [Theory]
    [MemberData(nameof(Documents))]
    public void EveryHeadingKeepsItsLevelThroughBothStages(string name)
    {
        ContentDocument tree = Extract(name);
        List<int> expected = [];
        Collect(tree);

        // A sheet and a slide each contribute one synthetic h1, which is a stated decision and
        // not a heading the tree holds.
        expected.AddRange(tree.Children.OfType<ContentSection>()
            .Where(s => s.Kind is SectionKind.Sheet or SectionKind.Slide)
            .Select(_ => 1));
        expected.Sort();

        List<int> fromXhtml = [.. XhtmlWriter.ToDocument(tree).Descendants()
            .Where(e => e.Name.LocalName is ['h', >= '1' and <= '6'])
            .Select(e => e.Name.LocalName[1] - '0')];
        fromXhtml.Sort();

        List<int> fromMarkdown = [.. Markdown.Parse(MarkdownWriter.ToMarkdown(tree))
            .Descendants<HeadingBlock>().Select(h => h.Level)];
        fromMarkdown.Sort();

        fromXhtml.ShouldBe(expected, "the XHTML lost or invented a heading");
        fromMarkdown.ShouldBe(expected, "the Markdown lost or invented a heading");

        void Collect(ContentNode node)
        {
            foreach (ContentNode child in node.Children)
            {
                if (child is ContentParagraph { HeadingLevel: int level }) expected.Add(Math.Min(level, 6));
                Collect(child);
            }
        }
    }

    [Fact]
    public void ASheetIsBoundedAndTheBoundIsVisible()
    {
        // Every corpus sheet is small, so the bound has to be lowered to be exercised at all —
        // which is the point of asserting it: a truncated sheet that does not say so reads as a
        // complete one, and a used range of ten thousand rows is the normal case in the wild.
        string markdown = MarkdownWriter.ToMarkdown(
            Extract("sheet-xlsx.xlsx"), new MarkupOptions { MaxTableRows = 2 });

        markdown.ShouldContain("further rows omitted");
        Markdown.PlainText(markdown).ShouldNotContain("West");
    }

    [Fact]
    public void SpeakerNotesAreNotSplicedIntoTheSlide()
    {
        // The reason SlideNotes is its own section kind in the first place: run into the slide's
        // own text, a speaker note turns two unrelated sentences into one.
        string markdown = MarkdownWriter.ToMarkdown(Extract("deck-features.pptx"));

        markdown.ShouldContain("> **Speaker notes**");
        markdown.ShouldContain("> Speaker notes for the first slide.");
    }

    /// <summary>
    /// Words the writers add on purpose: the synthetic sheet and slide headings, and the labels
    /// that set a header, a footer, a comment, a frame or a note apart.
    /// </summary>
    /// <remarks>
    /// Enumerated from the tree rather than hard-coded, so a document that has no slides is not
    /// silently allowed to contain the word "Slide".
    /// </remarks>
    private static HashSet<string> SyntheticWords(ContentDocument tree)
    {
        HashSet<string> allowed = new(StringComparer.Ordinal);
        foreach (ContentSection section in tree.Children.OfType<ContentSection>())
        {
            switch (section.Kind)
            {
                case SectionKind.Sheet or SectionKind.Slide:
                    foreach (string word in Words(section.Name ?? string.Empty)) allowed.Add(word);
                    allowed.Add(section.Kind == SectionKind.Sheet ? "Sheet" : "Slide");
                    allowed.Add((section.Index + 1).ToString(System.Globalization.CultureInfo.InvariantCulture));
                    if (section.IsHidden) allowed.Add("Hidden.");
                    break;
                case SectionKind.SlideNotes: allowed.Add("Speaker"); allowed.Add("notes"); break;
                case SectionKind.Header: allowed.Add("Header"); break;
                case SectionKind.Footer: allowed.Add("Footer"); break;
                case SectionKind.Frame: allowed.Add("Text"); allowed.Add("frame"); break;
                case SectionKind.Comment:
                    allowed.Add("Comment");
                    allowed.Add("—");
                    foreach (string word in Words(section.Name ?? string.Empty)) allowed.Add(word);
                    break;
                case SectionKind.Note:
                    allowed.Add("Note");
                    foreach (string word in Words(section.Name ?? string.Empty)) allowed.Add(word);
                    break;
                default: break;
            }
        }
        return allowed;
    }

    /// <summary>The content tree's own text, with the list markers optionally left out.</summary>
    private static string TreeText(ContentNode node, bool includeListMarkers)
    {
        if (includeListMarkers) return node.GetText();

        StringBuilder text = new();
        Walk(node);
        return text.ToString();

        void Walk(ContentNode current)
        {
            switch (current)
            {
                case ContentRun run: text.Append(run.Text); break;
                case ContentParagraph paragraph:
                    foreach (ContentNode child in paragraph.Children) Walk(child);
                    text.Append('\n');
                    break;
                default:
                    foreach (ContentNode child in current.Children) Walk(child);
                    break;
            }
        }
    }

    /// <summary>
    /// The text an XHTML subtree says, reassembled the way the content tree says it.
    /// </summary>
    /// <remarks>
    /// Not <c>XElement.Value</c>, which would be wrong in both directions: it glues the two
    /// halves of a <c>&lt;br/&gt;</c> into one word, and it drops the list markers the writer
    /// parks on <c>data-marker</c> rather than emitting as text.
    /// </remarks>
    private static string XhtmlText(XElement element)
    {
        StringBuilder text = new();
        Walk(element);
        return text.ToString();

        void Walk(XElement current)
        {
            if (current.Attribute("data-marker")?.Value is { Length: > 0 } marker)
                text.Append(marker).Append(' ');

            foreach (XNode node in current.Nodes())
            {
                switch (node)
                {
                    case XText raw: text.Append(raw.Value); break;
                    case XElement { Name.LocalName: "br" }: text.Append('\n'); break;
                    case XElement child:
                        // Only a block boundary ends a word. Breaking after <strong> too would
                        // split "bold," — the comma is the next run — and report a loss the
                        // writer did not cause.
                        Walk(child);
                        if (IsBlock(child.Name.LocalName)) text.Append('\n');
                        break;
                    default: break;
                }
            }
        }
    }

    private static bool IsBlock(string name) => name is
        "p" or "h1" or "h2" or "h3" or "h4" or "h5" or "h6" or "li" or "ul" or "ol"
        or "table" or "thead" or "tbody" or "tr" or "th" or "td" or "blockquote" or "pre"
        or "section" or "aside" or "header" or "footer" or "div" or "figure";

    private static string StripTags(string text) => TagPattern().Replace(text, " ");

    [GeneratedRegex("<[^>]*>")]
    private static partial Regex TagPattern();

    private static List<string> Words(string text)
        => [.. text.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries)];
}

internal static class SubsequenceAssertions
{
    /// <summary>
    /// Asserts that every word of <paramref name="expected"/> appears in <paramref name="actual"/>
    /// in the same order, and that anything extra is on the allowed list.
    /// </summary>
    /// <remarks>
    /// Order matters and is half the point: a projection can preserve every word and still put a
    /// table's cells in the wrong sequence, or lift a footnote into the middle of the body.
    /// Equality is the wrong assertion because both writers add text deliberately — a sheet's
    /// name as a heading, "Speaker notes" as a label — so the additions are named instead of the
    /// comparison being loosened.
    /// </remarks>
    public static void ShouldBeASubsequenceOf(
        this List<string> actual,
        List<string> expected,
        HashSet<string> allowedAdditions,
        string because)
    {
        int i = 0;
        List<string> unexpected = [];
        foreach (string word in actual)
        {
            if (i < expected.Count && string.Equals(word, expected[i], StringComparison.Ordinal)) i++;
            else unexpected.Add(word);
        }

        if (i < expected.Count)
        {
            throw new ShouldAssertException(
                $"the projection lost '{expected[i]}' (word {i} of {expected.Count}) "
                + $"or moved it out of order.\n\n{because}");
        }

        unexpected.RemoveAll(allowedAdditions.Contains);
        unexpected.ShouldBeEmpty($"the projection says words the document does not.\n\n{because}");
    }
}

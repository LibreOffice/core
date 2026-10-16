using System.Text.RegularExpressions;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Rendering.Pdf;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// The same questions as <see cref="DocumentLinkTests"/> asked of every word-processing format, against
/// what the format's own extraction says the document contains.
/// </summary>
/// <remarks>
/// The expected values are not written down here: they are read out of the extracted content — the
/// hyperlink targets of its runs, the headings among its paragraphs — so the test says "the PDF links
/// wherever the document does", which stays true as the fixtures change, and cannot pass by agreeing with
/// itself because the extraction and the layout are separate readers of the file.
/// </remarks>
public sealed class FormatLinkTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 3, 5, 12, 30, 0, TimeSpan.Zero),
    };

    [Theory]
    [InlineData("word-features.docx")]
    [InlineData("word-features.doc")]
    [InlineData("word-features.rtf")]
    [InlineData("text-features.odt")]
    public void EveryExternalHyperlinkOfTheDocumentIsALinkInThePdf(string file)
    {
        using IDocument document = OfficeDocument.Open(Corpus.Require(file));

        List<string> expected = [.. Runs(document.Content)
            .Select(run => run.HyperlinkTarget)
            .OfType<string>()
            .Where(target => !target.StartsWith('#') && target.Length > 0)
            .Distinct(StringComparer.Ordinal)];

        expected.ShouldNotBeEmpty($"{file} was chosen because it holds a hyperlink");

        PdfFile pdf = Render(document);
        foreach (string target in expected)
        {
            string written = Uri.EscapeDataString(target).Length == 0 ? target : Address(target);
            pdf.Text.ShouldContain($"/URI{PdfSyntaxLiteral(written)}", customMessage: $"{file}: {target}");
        }
    }

    [Theory]
    [InlineData("word-features.docx")]
    [InlineData("word-features.doc")]
    [InlineData("word-features.rtf")]
    [InlineData("text-features.odt")]
    [InlineData("paginated.docx")]
    [InlineData("paginated.odt")]
    public void EveryHeadingOfTheDocumentIsAnOutlineEntry(string file)
    {
        using IDocument document = OfficeDocument.Open(Corpus.Require(file));

        List<string> headings = [.. Paragraphs(document.Content)
            .Where(p => p.HeadingLevel is not null)
            .Select(p => Normalise(p.GetText()))
            .Where(t => t.Length > 0)];

        headings.ShouldNotBeEmpty($"{file} was chosen because it holds a heading");

        PdfFile pdf = Render(document);
        List<string> entries = [.. pdf.Dictionaries().Values
            .Select(v => Regex.Match(v, @"^<</Title\((.*?)\)/Parent"))
            .Where(m => m.Success)
            .Select(m => Normalise(m.Groups[1].Value))];

        entries.ShouldNotBeEmpty();
        foreach (string heading in headings)
        {
            entries.ShouldContain(heading, $"{file}: heading '{heading}'");
        }
    }

    [Theory]
    [InlineData("word-features.docx")]
    [InlineData("bookmark-field.docx")]
    [InlineData("bookmark-field.doc")]
    [InlineData("bookmark-field.rtf")]
    [InlineData("bookmark-field.odt")]
    public void EveryBookmarkOfTheDocumentIsANamedDestination(string file)
    {
        using IDocument document = OfficeDocument.Open(Corpus.Require(file));
        PdfFile pdf = Render(document);

        // The names the file declares, as the PDF's own name tree lists them.
        MatchCollection names = Regex.Matches(pdf.Text, @"/Dests<</Names\[(.*?)\]>>", RegexOptions.Singleline);
        names.Count.ShouldBe(1, $"{file} holds bookmarks");
        Regex.Count(names[0].Groups[1].Value, @"\(([^)]+)\)\[\d+ 0 R/XYZ").ShouldBeGreaterThan(0);
    }

    [Fact]
    public void EachShownSlideOfADeckIsAnOutlineEntryUnderItsTitle()
    {
        using IDocument deck = OfficeDocument.Open(Corpus.Require("deck-features.pptx"));
        PdfFile pdf = Render(deck);

        IPageSequence pages = ((IPaginatedDocument)deck).Layout();
        List<(string Title, int Page)> entries = Entries(pdf);

        entries.Count.ShouldBe(pages.Count, "one entry per shown slide; a hidden slide is not a page");
        entries.Select(e => e.Page).ShouldBe(Enumerable.Range(0, pages.Count));
        entries.ShouldAllBe(e => e.Title.Length > 0);
    }

    [Fact]
    public void EachPrintedSheetOfAWorkbookIsAnOutlineEntryAtItsFirstPage()
    {
        using IDocument book = OfficeDocument.Open(Corpus.Require("chart-bar-sheet.xlsx"));
        PdfFile pdf = Render(book);

        List<(string Title, int Page)> entries = Entries(pdf);
        entries.ShouldBe([("Revenue", 0)]);
    }

    /// <summary>The outline's entries in file order, with the zero-based page each points at.</summary>
    private static List<(string Title, int Page)> Entries(PdfFile pdf)
    {
        Match kids = Regex.Match(pdf.Text, @"/Kids\[([^\]]*)\]");
        List<int> pages = [.. Regex.Matches(kids.Groups[1].Value, @"(\d+) 0 R")
            .Select(m => int.Parse(m.Groups[1].Value, System.Globalization.CultureInfo.InvariantCulture))];

        return [.. pdf.Dictionaries()
            .Select(o => Regex.Match(o.Value, @"^<</Title\((.*?)\)/Parent.*?/Dest\[(\d+) 0 R"))
            .Where(m => m.Success)
            .Select(m => (m.Groups[1].Value, pages.IndexOf(int.Parse(m.Groups[2].Value, System.Globalization.CultureInfo.InvariantCulture))))];
    }

    // ------------------------------------------------------------------------------------------------

    private static PdfFile Render(IDocument document)
    {
        IPageSequence pages = ((IPaginatedDocument)document).Layout();
        using MemoryStream output = new();
        new PdfRenderer(Reproducible).Render(document, pages, output);
        return PdfFile.Parse(output.ToArray());
    }

    /// <summary>An address as a /URI action spells it: spaces and anything outside ASCII percent-encoded.</summary>
    private static string Address(string target)
    {
        System.Text.StringBuilder text = new();
        foreach (System.Text.Rune rune in target.EnumerateRunes())
        {
            if (rune.Value is > ' ' and < 0x7F) text.Append((char)rune.Value);
            else if (rune.Value == ' ') text.Append("%20");
            else if (rune.Value >= 0x80)
            {
                foreach (byte b in System.Text.Encoding.UTF8.GetBytes(rune.ToString())) text.Append('%').Append(b.ToString("X2", System.Globalization.CultureInfo.InvariantCulture));
            }
        }

        return text.ToString();
    }

    private static string PdfSyntaxLiteral(string ascii)
        => "(" + ascii.Replace("\\", "\\\\", StringComparison.Ordinal).Replace("(", "\\(", StringComparison.Ordinal)
            .Replace(")", "\\)", StringComparison.Ordinal) + ")";

    private static string Normalise(string text) => string.Join(' ', text.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries));

    private static IEnumerable<ContentRun> Runs(ContentNode node)
    {
        if (node is ContentRun run) yield return run;
        foreach (ContentNode child in node.Children)
        {
            foreach (ContentRun inner in Runs(child)) yield return inner;
        }
    }

    private static IEnumerable<ContentParagraph> Paragraphs(ContentNode node)
    {
        if (node is ContentParagraph paragraph) yield return paragraph;
        foreach (ContentNode child in node.Children)
        {
            foreach (ContentParagraph inner in Paragraphs(child)) yield return inner;
        }
    }
}

using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests the ODT reader against corpus documents written by LibreOffice itself.
/// </summary>
/// <remarks>
/// Everything here reads a committed file rather than a hand-built fixture. A fixture only
/// proves the reader agrees with our idea of ODF; a document LibreOffice wrote proves it
/// agrees with the format as actually produced — including the details LibreOffice adds that
/// no reading of the specification would predict, such as rewriting nested character styles
/// or caching a footnote's recomputed citation.
/// </remarks>
public class OdtReaderTests
{
    private static IDocument Open(string name)
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    [Theory]
    [InlineData("prose-odt.odt", DocumentFormat.Odt)]
    [InlineData("prose-ott.ott", DocumentFormat.Ott)]
    [InlineData("prose-fodt.fodt", DocumentFormat.Fodt)]
    public void EveryOdfTextVariantReadsThroughTheSamePath(string name, DocumentFormat expected)
    {
        using IDocument document = Open(name);

        document.Format.ShouldBe(expected);
        document.Family.ShouldBe(DocumentFamily.WordProcessing);
        document.Diagnostics.ShouldNotContain(d => d.Severity == Core.Diagnostics.DiagnosticSeverity.Error);

        // The three variants hold the same document, so the packaged and flat readers must
        // produce the same text — that equivalence is the whole point of hiding the
        // difference behind OdfFile.
        document.Content.GetText().ShouldContain("Paperless test document");
        document.Content.GetText().ShouldContain("Item three");
    }

    [Fact]
    public void PackagedAndFlatFormsOfTheSameDocumentExtractIdentically()
    {
        using IDocument packaged = Open("text-features.odt");
        using IDocument flat = Open("text-features-flat.fodt");

        flat.Format.ShouldBe(DocumentFormat.Fodt);
        flat.Content.GetText().ShouldBe(packaged.Content.GetText());
    }

    [Fact]
    public void TheBodyIsOneSectionAndNotesComeAfterIt()
    {
        using IDocument document = Open("text-features.odt");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        sections[0].Kind.ShouldBe(SectionKind.Body);

        // Notes, comments and shape text are separate flows, not part of the paragraph that
        // anchors them, so each is its own section after the body.
        sections.ShouldContain(s => s.Kind == SectionKind.Note);
        sections.ShouldContain(s => s.Kind == SectionKind.Comment);
        sections.ShouldContain(s => s.Kind == SectionKind.Frame);
        sections.ShouldContain(s => s.Kind == SectionKind.Header);
        sections.ShouldContain(s => s.Kind == SectionKind.Footer);
    }

    [Fact]
    public void HeadingsCarryTheirOutlineLevelAndUserVisibleStyleName()
    {
        using IDocument document = Open("text-features.odt");
        List<ContentParagraph> headings =
            [.. Paragraphs(document.Content).Where(p => p.HeadingLevel is not null)];

        headings.Select(h => h.HeadingLevel).ShouldBe([1, 2, 2, 3]);
        headings[0].GetText().Trim().ShouldBe("Top level heading");

        // The reported name is the user-visible one, not the XML-escaped Heading_20_1.
        headings[0].StyleName.ShouldBe("Heading 1");
    }

    [Fact]
    public void CharacterFormattingIsResolvedThroughTheStyleCascade()
    {
        using IDocument document = Open("text-features.odt");
        List<ContentRun> runs = [.. Runs(document.Content)];

        runs.ShouldContain(r => r.Text == "bold" && r.Emphasis.HasFlag(RunEmphasis.Bold));
        runs.ShouldContain(r => r.Text == "italic" && r.Emphasis.HasFlag(RunEmphasis.Italic));
        runs.ShouldContain(r => r.Text == "underlined and struck"
                                && r.Emphasis.HasFlag(RunEmphasis.Underline)
                                && r.Emphasis.HasFlag(RunEmphasis.Strikethrough));
        runs.ShouldContain(r => r.Text == "superscript" && r.Emphasis.HasFlag(RunEmphasis.Superscript));

        // LibreOffice rewrites the doubly-styled run as a span nested inside a span, so bold
        // comes from the outer style and italic from the inner one. Both must survive.
        ContentRun nested = runs.First(r => r.Text.Contains("nested bold italic", StringComparison.Ordinal));
        nested.Emphasis.HasFlag(RunEmphasis.Bold).ShouldBeTrue();
        nested.Emphasis.HasFlag(RunEmphasis.Italic).ShouldBeTrue();
        nested.Language.ShouldBe("de-DE");
    }

    [Fact]
    public void RunsInheritTheLanguageFromTheDocumentDefaults()
    {
        using IDocument document = Open("text-features.odt");

        // fo:language lives on the paragraph family's default style, and character styles
        // fall back to it — a run that sets nothing still has a language.
        Runs(document.Content).First().Language.ShouldBe("en-GB");
    }

    [Fact]
    public void HyperlinksKeepTheirTarget()
    {
        using IDocument document = Open("text-features.odt");
        ContentRun link = Runs(document.Content)
            .First(r => r.HyperlinkTarget is not null);

        link.Text.ShouldBe("the LibreOffice site");
        link.HyperlinkTarget.ShouldBe("https://www.libreoffice.org/");
    }

    [Fact]
    public void ListNestingAndGeneratedMarkersAreBothRecorded()
    {
        using IDocument document = Open("text-features.odt");
        List<ContentParagraph> items =
            [.. Paragraphs(document.Content).Where(p => p.ListLevel is not null)];

        // ODF states nesting in the XML tree; the marker exists nowhere in the file and has
        // to be generated from the counters and the level's number format.
        items.Single(p => p.GetText().Contains("First bullet", StringComparison.Ordinal))
             .ListMarker.ShouldBe("•");
        items.Single(p => p.GetText().Contains("Nested bullet", StringComparison.Ordinal))
             .ListLevel.ShouldBe(1);

        items.Single(p => p.GetText().Contains("First numbered", StringComparison.Ordinal))
             .ListMarker.ShouldBe("1.");
        items.Single(p => p.GetText().Contains("Third numbered", StringComparison.Ordinal))
             .ListMarker.ShouldBe("3.");
        items.Single(p => p.GetText().Contains("Nested letter", StringComparison.Ordinal))
             .ListMarker.ShouldBe("a)");
        items.Single(p => p.GetText().Contains("Second nested letter", StringComparison.Ordinal))
             .ListMarker.ShouldBe("b)");

        // A second paragraph inside one list item is a continuation: same level, no marker.
        ContentParagraph continuation =
            items.Single(p => p.GetText().Contains("Continuation paragraph", StringComparison.Ordinal));
        continuation.ListLevel.ShouldBe(0);
        continuation.ListMarker.ShouldBeNull();
    }

    [Fact]
    public void TablesKeepSpansAndHeaderRows()
    {
        using IDocument document = Open("text-features.odt");
        ContentTable table = Tables(document.Content).Single();

        table.ColumnCount.ShouldBe(3);
        table.HeaderRowCount.ShouldBe(1);

        List<ContentTableRow> rows = [.. table.Children.Cast<ContentTableRow>()];
        rows.Count.ShouldBe(3);

        ContentTableCell merged = (ContentTableCell)rows[1].Children[0];
        merged.ColumnSpan.ShouldBe(2);
        merged.GetText().ShouldBe("Merged across two columns");

        // The covered cell the span hides is not a cell of its own, so the next real cell
        // keeps its true column index.
        ContentTableCell after = (ContentTableCell)rows[1].Children[1];
        after.Column.ShouldBe(2);

        // A row is one line of tab-separated cells, so an empty middle cell is an empty field
        // rather than a lost column.
        rows[2].GetText().ShouldBe("Row two A\t\tRow two C\n");
    }

    [Fact]
    public void WhiteSpaceFollowsOdfsCollapsingRules()
    {
        using IDocument document = Open("text-features.odt");
        string text = document.Content.GetText();

        // text:s states the extra space explicitly; the literal space before it is kept, so
        // there are exactly two. Getting this wrong is invisible until a diff.
        text.ShouldContain("Two spaces here:  done.");
        text.ShouldContain("A tab:\tdone.");
        text.ShouldContain("A line break:\nsecond line.");
    }

    [Fact]
    public void FootnoteCitationsStayInlineWhileBodiesAreHoisted()
    {
        using IDocument document = Open("text-features.odt");

        // The citation is text a reader sees in the paragraph; the body is a separate flow.
        // The cached citation is used rather than a recomputed one — this document's notes
        // configuration starts numbering at zero, so a naive "1" would be wrong.
        document.Content.Children.OfType<ContentSection>()
                .First(s => s.Kind == SectionKind.Body)
                .GetText().ShouldContain("a footnote reference0 here");

        document.Content.Children.OfType<ContentSection>()
                .Single(s => s.Kind == SectionKind.Note)
                .GetText().Trim().ShouldBe("The footnote body text.");
    }

    [Fact]
    public void CommentsRecordTheirAuthor()
    {
        using IDocument document = Open("text-features.odt");
        ContentSection comment = document.Content.Children.OfType<ContentSection>()
                                         .Single(s => s.Kind == SectionKind.Comment);

        comment.Name.ShouldBe("Alan Turing");
        comment.GetText().Trim().ShouldBe("A reviewer comment.");
    }

    [Fact]
    public void ATextBoxDoesNotSplitTheParagraphThatAnchorsIt()
    {
        using IDocument document = Open("text-features.odt");

        // Splicing the box's own flow into the anchoring paragraph would join two unrelated
        // sentences, so the anchoring paragraph reads continuously and the box is separate.
        Paragraphs(document.Content)
            .ShouldContain(p => p.GetText() == "Before the box.  After the box.\n");

        document.Content.Children.OfType<ContentSection>()
                .Single(s => s.Kind == SectionKind.Frame)
                .GetText().Trim().ShouldBe("Text inside a text box.");
    }

    [Fact]
    public void FieldsContributeTheirCachedResult()
    {
        using IDocument document = Open("text-features.odt");

        // A field's last computed value is stored as its element content, and that is what a
        // reference renderer shows — so recursing into unknown inline elements is what makes
        // fields work without a per-field implementation.
        document.Content.GetText().ShouldContain("title field: Paperless feature document.");
    }

    [Fact]
    public void HeadersAndFootersAreExtractedEvenThoughTheReferenceFilterDropsThem()
    {
        using IDocument document = Open("text-features.odt");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        sections.Single(s => s.Kind == SectionKind.Header)
                .GetText().Trim().ShouldBe("Feature document header");
        sections.Single(s => s.Kind == SectionKind.Footer)
                .GetText().Trim().ShouldStartWith("Footer, page");
    }

    [Fact]
    public void MetadataComesFromMetaXml()
    {
        using IDocument document = Open("text-features.odt");
        DocumentMetadata metadata = document.Metadata;

        metadata.Title.ShouldBe("Paperless feature document");
        metadata.Author.ShouldBe("Ada Lovelace");
        metadata.LastModifiedBy.ShouldBe("Grace Hopper");
        metadata.Keywords.ShouldBe(["extraction", "odf"]);
        metadata.RevisionNumber.ShouldBe(7);
        metadata.TotalEditingTime.ShouldBe(new TimeSpan(1, 23, 45));
        metadata.CustomProperties["Reviewer"].ShouldBe("Alan Turing");
        metadata.Statistics!.SheetCount.ShouldBeNull();
        metadata.Statistics!.PageCount.ShouldNotBeNull();
    }

    [Fact]
    public void AFormatWithNoReaderYetIsReportedAsUnsupportedRatherThanUnrecognised()
    {
        // "Not implemented" and "not a word-processing document" are different answers, and a
        // caller deciding whether to try another reader needs to tell them apart. Word 2003 XML is
        // synthesised rather than taken from the corpus: what matters is the reader's answer for a
        // format it claims but has not implemented, and the sniffer recognises this one from its
        // processing instruction alone.
        MemoryStream wordXml = new(System.Text.Encoding.UTF8.GetBytes(
            "<?xml version=\"1.0\"?><?mso-application progid=\"Word.Document\"?><w:wordDocument/>"));
        UnsupportedFormatException unimplemented = Should.Throw<UnsupportedFormatException>(
            () => new WordProcessingReader().Read(
                DocumentSource.FromStream(wordXml, "prose.xml")));
        unimplemented.Format.ShouldBe(DocumentFormat.WordXml2003);
        unimplemented.Message.ShouldContain("not implemented yet");

        UnsupportedFormatException wrongFamily = Should.Throw<UnsupportedFormatException>(
            () => Open("sheet-ods.ods"));
        wrongFamily.Message.ShouldContain("not a word-processing format");
    }

    private static IEnumerable<ContentParagraph> Paragraphs(ContentNode root)
        => Descendants(root).OfType<ContentParagraph>();

    private static IEnumerable<ContentRun> Runs(ContentNode root)
        => Descendants(root).OfType<ContentRun>();

    private static IEnumerable<ContentTable> Tables(ContentNode root)
        => Descendants(root).OfType<ContentTable>();

    private static IEnumerable<ContentNode> Descendants(ContentNode root)
    {
        foreach (ContentNode child in root.Children)
        {
            yield return child;
            foreach (ContentNode descendant in Descendants(child)) yield return descendant;
        }
    }
}

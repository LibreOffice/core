using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests the DOCX reader against a corpus document written by LibreOffice.
/// </summary>
/// <remarks>
/// The document is the same one as <c>text-features.odt</c>, converted — which makes it useful
/// twice over: it covers the same features through a completely different vocabulary, and where
/// the two extractions differ the difference is either a real defect or a genuine difference
/// between the files, both of which are worth knowing.
/// </remarks>
public class DocxReaderTests
{
    private static IDocument Open(string name)
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    [Fact]
    public void ADocxReadsWithItsFormatAndFamilyIdentified()
    {
        using IDocument document = Open("word-features.docx");

        document.Format.ShouldBe(DocumentFormat.Docx);
        document.Family.ShouldBe(DocumentFamily.WordProcessing);
        document.Diagnostics.ShouldNotContain(d => d.Severity == Core.Diagnostics.DiagnosticSeverity.Error);
    }

    [Fact]
    public void TheBodyIsOneSectionWithTheOtherFlowsAfterIt()
    {
        using IDocument document = Open("word-features.docx");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        sections[0].Kind.ShouldBe(SectionKind.Body);
        sections.Select(s => s.Kind).Distinct().ShouldBe(
            [SectionKind.Body, SectionKind.Note, SectionKind.Comment, SectionKind.Frame,
             SectionKind.Header, SectionKind.Footer],
            ignoreOrder: true);
    }

    [Fact]
    public void HeadingsComeFromTheOutlineLevelOnTheirStyle()
    {
        using IDocument document = Open("word-features.docx");
        List<ContentParagraph> headings =
            [.. Descendants(document.Content).OfType<ContentParagraph>()
                .Where(p => p.HeadingLevel is not null)];

        // w:outlineLvl is zero-based and lives on the style, not the paragraph, so this covers
        // both the off-by-one and the style-chain walk.
        headings.Select(h => h.HeadingLevel).ShouldBe([1, 2, 2, 3]);
        headings[0].GetText().Trim().ShouldBe("Top level heading");
        headings[0].StyleName.ShouldBe("Heading 1");
    }

    [Fact]
    public void CharacterFormattingIsResolvedAcrossTheLayers()
    {
        using IDocument document = Open("word-features.docx");
        List<ContentRun> runs = [.. Descendants(document.Content).OfType<ContentRun>()];

        runs.ShouldContain(r => r.Text == "bold" && r.Emphasis.HasFlag(RunEmphasis.Bold));
        runs.ShouldContain(r => r.Text == "italic" && r.Emphasis.HasFlag(RunEmphasis.Italic));
        runs.ShouldContain(r => r.Text == "underlined and struck"
                                && r.Emphasis.HasFlag(RunEmphasis.Underline)
                                && r.Emphasis.HasFlag(RunEmphasis.Strikethrough));
        runs.ShouldContain(r => r.Text == "superscript"
                                && r.Emphasis.HasFlag(RunEmphasis.Superscript));

        // Bold from the character style, italic from the run's own rPr, language from the run.
        ContentRun mixed = runs.First(r => r.Text.Contains("nested bold italic", StringComparison.Ordinal));
        mixed.Emphasis.HasFlag(RunEmphasis.Bold).ShouldBeTrue();
        mixed.Emphasis.HasFlag(RunEmphasis.Italic).ShouldBeTrue();
        mixed.Language.ShouldBe("de-DE");
        mixed.StyleName.ShouldBe("Strong");
    }

    [Fact]
    public void RunsInheritTheirLanguageFromTheDocumentDefaults()
    {
        using IDocument document = Open("word-features.docx");

        // w:lang is in w:docDefaults, so a run that sets nothing still has a language.
        Descendants(document.Content).OfType<ContentRun>().First().Language.ShouldBe("en-GB");
    }

    [Fact]
    public void FontSizeIsReadAsHalfPoints()
    {
        using IDocument document = Open("word-features.docx");
        Ooxml.OoxmlWordDocument docx = (Ooxml.OoxmlWordDocument)document;

        // w:sz counts half-points, which is the unit that catches people out: 36 is 18pt, not
        // 36pt. The Heading 1 style sets 36.
        Ooxml.WordCharacterFormat heading = Ooxml.WordCharacterFormat.Resolve(
            docx.File.Styles, directRunProperties: null, paragraphStyleId: "Heading1");
        heading.FontSize!.Value.Points.ShouldBe(18);
        heading.IsBold.ShouldBeTrue();
    }

    [Fact]
    public void HyperlinksResolveThroughTheRelationshipPart()
    {
        using IDocument document = Open("word-features.docx");

        // A DOCX stores the URL in document.xml.rels and refers to it by id, so this fails
        // outright if relationships are not resolved.
        ContentRun link = Descendants(document.Content).OfType<ContentRun>()
                                                      .First(r => r.HyperlinkTarget is not null);
        link.Text.ShouldBe("the LibreOffice site");
        link.HyperlinkTarget.ShouldBe("https://www.libreoffice.org/");
    }

    [Fact]
    public void ListLevelsAndGeneratedMarkersAreBothRecorded()
    {
        using IDocument document = Open("word-features.docx");
        List<ContentParagraph> items =
            [.. Descendants(document.Content).OfType<ContentParagraph>()
                .Where(p => p.ListLevel is not null)];

        // Unlike ODF, nesting is an attribute rather than XML structure, and the marker has to
        // be generated by advancing counters across the paragraphs in order.
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
    }

    [Fact]
    public void AContinuationParagraphIsNotNumbered()
    {
        using IDocument document = Open("word-features.docx");
        ContentParagraph continuation = Descendants(document.Content).OfType<ContentParagraph>()
            .Single(p => p.GetText().Contains("Continuation paragraph", StringComparison.Ordinal));

        // Word writes a numId of zero for it, which means "not numbered" rather than "list zero"
        // — reading it as a list id would restart the numbering at every continuation.
        continuation.ListLevel.ShouldBeNull();
        continuation.ListMarker.ShouldBeNull();
    }

    [Fact]
    public void NumberedHeadingsGetTheirMarkerButNotAListLevel()
    {
        using IDocument document = Open("word-features.docx");
        ContentParagraph heading = Descendants(document.Content).OfType<ContentParagraph>()
            .First(p => p.HeadingLevel == 1);

        // The heading styles here name a list whose format is "none", so there is no marker to
        // show. What matters is that the depth is reported once, as a heading level.
        heading.ListLevel.ShouldBeNull();
    }

    [Fact]
    public void TablesKeepSpansAndHeaderRows()
    {
        using IDocument document = Open("word-features.docx");
        ContentTable table = Descendants(document.Content).OfType<ContentTable>().Single();

        table.ColumnCount.ShouldBe(3);
        table.HeaderRowCount.ShouldBe(1);

        List<ContentTableRow> rows = [.. table.Children.Cast<ContentTableRow>()];
        ContentTableCell merged = (ContentTableCell)rows[1].Children[0];
        merged.ColumnSpan.ShouldBe(2);
        merged.GetText().ShouldBe("Merged across two columns");
        ((ContentTableCell)rows[1].Children[1]).Column.ShouldBe(2);

        rows[2].GetText().ShouldBe("Row two A\t\tRow two C\n");
    }

    [Fact]
    public void AFieldContributesItsResultAndNotItsCode()
    {
        using IDocument document = Open("word-features.docx");
        string text = document.Content.GetText();

        // The instruction is stored as text in w:instrText between the fldChar markers, so a
        // walk that emits everything puts " TITLE " into the output.
        text.ShouldContain("title field: Paperless feature document.");
        // The code's own spelling, which only appears if instrText was emitted. Case-sensitive,
        // because "title field" in the sentence above would match otherwise.
        text.Contains(" TITLE ", StringComparison.Ordinal).ShouldBeFalse();
    }

    [Fact]
    public void APageBreakDoesNotBecomeALineBreak()
    {
        using IDocument document = Open("word-features.docx");

        // A w:br of type "page" moves the following content to another page without breaking the
        // line, so treating every w:br as a newline adds one the document does not have.
        document.Content.GetText().ShouldContain("Before the box.  After the box.\nThis paragraph");
    }

    [Fact]
    public void FootnoteCitationsAreNumberedBecauseTheFileDoesNotCacheThem()
    {
        using IDocument document = Open("word-features.docx");

        // A DOCX records only the reference and the body; Word computes the number at layout
        // time. ECMA-376 §17.11.17 puts the default start at 1.
        document.Content.Children.OfType<ContentSection>()
                .First(s => s.Kind == SectionKind.Body)
                .GetText().ShouldContain("a footnote reference1 here");

        ContentSection note = document.Content.Children.OfType<ContentSection>()
                                      .Single(s => s.Kind == SectionKind.Note);
        note.Name.ShouldBe("1");
        note.GetText().Trim().ShouldBe("The footnote body text.");
    }

    [Fact]
    public void CommentsRecordTheirAuthor()
    {
        using IDocument document = Open("word-features.docx");
        ContentSection comment = document.Content.Children.OfType<ContentSection>()
                                         .Single(s => s.Kind == SectionKind.Comment);

        comment.Name.ShouldBe("Alan Turing");
        comment.GetText().Trim().ShouldBe("A reviewer comment.");
    }

    [Fact]
    public void ATextBoxIsReadOnceDespiteBeingStoredTwice()
    {
        using IDocument document = Open("word-features.docx");

        // The box is written as a DrawingML shape *and* as a VML fallback inside
        // mc:AlternateContent. Walking both would extract its text twice.
        ContentSection frame = document.Content.Children.OfType<ContentSection>()
                                       .Single(s => s.Kind == SectionKind.Frame);
        frame.GetText().Trim().ShouldBe("Text inside a text box.");

        document.Content.GetText().Split("Text inside a text box.").Length.ShouldBe(2);
    }

    [Fact]
    public void HeadersAndFootersAreReadFromThePartsTheSectionNames()
    {
        using IDocument document = Open("word-features.docx");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        sections.Single(s => s.Kind == SectionKind.Header)
                .GetText().Trim().ShouldBe("Feature document header");
        sections.Single(s => s.Kind == SectionKind.Footer)
                .GetText().Trim().ShouldStartWith("Footer, page");
    }

    [Fact]
    public void MetadataComesFromAllThreeDocPropsParts()
    {
        using IDocument document = Open("word-features.docx");
        DocumentMetadata metadata = document.Metadata;

        // core.xml
        metadata.Title.ShouldBe("Paperless feature document");
        metadata.Author.ShouldBe("Ada Lovelace");
        metadata.LastModifiedBy.ShouldBe("Grace Hopper");
        metadata.RevisionNumber.ShouldBe(7);
        metadata.Keywords.ShouldBe(["extraction", "odf"]);

        // app.xml, where TotalTime counts minutes rather than being a duration string.
        metadata.TotalEditingTime.ShouldBe(TimeSpan.FromMinutes(83));
        metadata.Statistics!.PageCount.ShouldBe(2);
        metadata.Statistics!.WordCount.ShouldBe(146);

        // custom.xml, where each value's type is the name of its variant element.
        metadata.CustomProperties["Reviewer"].ShouldBe("Alan Turing");
        metadata.CustomProperties["Revision"].ShouldBe(2.5);
        metadata.CustomProperties["Approved"].ShouldBe(true);
    }

    [Fact]
    public void TheSameDocumentInDocxAndOdtExtractsToTheSameText()
    {
        using IDocument docx = Open("word-features.docx");
        using IDocument odt = Open("text-features.odt");

        // The two files hold the same document through completely different vocabularies, so the
        // body text must agree exactly. Only the footnote citation differs, and for a reason
        // that is in the files rather than in the readers: the ODT caches the number 0 because
        // its notes configuration starts numbering there, while the DOCX caches no number at all
        // and the format's default start is 1.
        //
        // The footer is excluded because its page-number field is a *cached* result — 0 in the
        // ODT, 2 in the DOCX — and both readers deliberately report the cache rather than
        // recomputing it.
        string fromDocx = BodyText(docx).Replace("reference1", "reference0", StringComparison.Ordinal);
        fromDocx.ShouldBe(BodyText(odt));

        static string BodyText(IDocument document)
            => string.Concat(document.Content.Children.OfType<ContentSection>()
                                     .Where(s => s.Kind != SectionKind.Footer)
                                     .Select(s => s.GetText()));
    }

    private static IEnumerable<ContentNode> Descendants(ContentNode root)
    {
        foreach (ContentNode child in root.Children)
        {
            yield return child;
            foreach (ContentNode descendant in Descendants(child)) yield return descendant;
        }
    }
}

using System.Text;
using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests the RTF reader against a corpus document written by LibreOffice, and against small
/// hand-written documents for the parts of the grammar a real file exercises only incidentally.
/// </summary>
/// <remarks>
/// The corpus document is the same one as <c>text-features.odt</c> and
/// <c>word-features.docx</c>, converted. Three formats holding one document is what makes a
/// difference between them meaningful: it is either a defect or a genuine difference between the
/// files, and both are worth knowing about.
/// </remarks>
public class RtfReaderTests
{
    private static IDocument Open(string name)
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    /// <summary>Reads a hand-written RTF document from a string of ASCII-safe RTF.</summary>
    private static IDocument OpenLiteral(string rtf)
    {
        MemoryStream stream = new(Encoding.ASCII.GetBytes(rtf));
        DocumentSource source = DocumentSource.FromStream(stream, "literal.rtf");
        return new WordProcessingReader().Read(source);
    }

    [Fact]
    public void AnRtfDocumentReadsWithItsFormatAndFamilyIdentified()
    {
        using IDocument document = Open("word-features.rtf");

        document.Format.ShouldBe(DocumentFormat.Rtf);
        document.Family.ShouldBe(DocumentFamily.WordProcessing);
        document.Diagnostics.ShouldNotContain(d => d.Severity == Core.Diagnostics.DiagnosticSeverity.Error);
    }

    [Fact]
    public void TheBodyIsOneSectionWithTheOtherFlowsBesideIt()
    {
        using IDocument document = Open("word-features.rtf");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        sections[0].Kind.ShouldBe(SectionKind.Body);
        sections.Select(s => s.Kind).Distinct().ShouldBe(
            [SectionKind.Body, SectionKind.Note, SectionKind.Comment, SectionKind.Frame,
             SectionKind.Header, SectionKind.Footer],
            ignoreOrder: true);
    }

    [Fact]
    public void HeadingsComeFromTheOutlineLevelOnTheParagraphOrItsStyle()
    {
        using IDocument document = Open("word-features.rtf");
        List<ContentParagraph> headings =
            [.. Descendants(document.Content).OfType<ContentParagraph>()
                .Where(p => p.HeadingLevel is not null)];

        headings.Select(h => h.HeadingLevel).ShouldBe([1, 2, 2, 3]);
        headings[0].GetText().Trim().ShouldBe("Top level heading");

        // The stylesheet is what gives a style its name; RTF's \s is only a number.
        headings[0].StyleName.ShouldBe("Heading 1");
    }

    [Fact]
    public void CharacterFormattingComesFromTheStateInForce()
    {
        using IDocument document = Open("word-features.rtf");
        List<ContentRun> runs = [.. Descendants(document.Content).OfType<ContentRun>()];

        runs.ShouldContain(r => r.Text == "bold" && r.Emphasis.HasFlag(RunEmphasis.Bold));
        runs.ShouldContain(r => r.Text == "italic" && r.Emphasis.HasFlag(RunEmphasis.Italic));
        runs.ShouldContain(r => r.Text == "underlined and struck"
                                && r.Emphasis.HasFlag(RunEmphasis.Underline)
                                && r.Emphasis.HasFlag(RunEmphasis.Strikethrough));
        runs.ShouldContain(r => r.Text == "superscript"
                                && r.Emphasis.HasFlag(RunEmphasis.Superscript));

        ContentRun mixed = runs.First(r => r.Text.Contains("nested bold italic", StringComparison.Ordinal));
        mixed.Emphasis.HasFlag(RunEmphasis.Bold).ShouldBeTrue();
        mixed.Emphasis.HasFlag(RunEmphasis.Italic).ShouldBeTrue();
        mixed.StyleName.ShouldBe("Strong");
        // RTF records a Windows language id rather than a tag; 1031 is German.
        mixed.Language.ShouldBe("de-DE");
    }

    [Fact]
    public void ListMarkersAreReadRatherThanGenerated()
    {
        using IDocument document = Open("word-features.rtf");
        List<ContentParagraph> items =
            [.. Descendants(document.Content).OfType<ContentParagraph>()
                .Where(p => p.ListMarker is not null)];

        // RTF is the one format that writes the rendered label out, in a {\listtext} group. So
        // these are what the writer displayed rather than what Paperless computed — including the
        // nested bullet, whose code point arrives with a question-mark fallback beside it.
        items.ShouldContain(p => p.ListMarker == "•"
                                 && p.GetText().Contains("First bullet", StringComparison.Ordinal));
        items.ShouldContain(p => p.ListMarker == "◦"
                                 && p.GetText().Contains("Nested bullet", StringComparison.Ordinal));
        items.ShouldContain(p => p.ListMarker == "1."
                                 && p.GetText().Contains("First numbered", StringComparison.Ordinal));
        items.ShouldContain(p => p.ListMarker == "a)"
                                 && p.GetText().Contains("Nested letter", StringComparison.Ordinal));
    }

    [Fact]
    public void AContinuationParagraphHasNoMarker()
    {
        using IDocument document = Open("word-features.rtf");
        ContentParagraph continuation = Descendants(document.Content).OfType<ContentParagraph>()
            .Single(p => p.GetText().Contains("Continuation paragraph", StringComparison.Ordinal));

        continuation.ListMarker.ShouldBeNull();
    }

    [Fact]
    public void TablesKeepTheirRowsAndColumnSpans()
    {
        using IDocument document = Open("word-features.rtf");
        ContentTable table = Descendants(document.Content).OfType<ContentTable>().Single();

        table.ColumnCount.ShouldBe(3);
        List<ContentTableRow> rows = [.. table.Children.Cast<ContentTableRow>()];
        rows.Count.ShouldBe(3);

        // A horizontal merge is \clmgf on the first cell and \clmrg on the ones it swallows, so
        // the span is only knowable once the row has been read.
        ContentTableCell merged = (ContentTableCell)rows[1].Children[0];
        merged.ColumnSpan.ShouldBe(2);
        merged.GetText().ShouldBe("Merged across two columns");
        ((ContentTableCell)rows[1].Children[1]).Column.ShouldBe(2);

        rows[2].GetText().ShouldBe("Row two A\t\tRow two C\n");
    }

    [Fact]
    public void AHyperlinkComesFromItsFieldInstruction()
    {
        using IDocument document = Open("word-features.rtf");
        ContentRun link = Descendants(document.Content).OfType<ContentRun>()
                                                      .First(r => r.HyperlinkTarget is not null);

        // RTF has no hyperlink markup at all: a link is a field whose instruction reads
        // HYPERLINK "…", and the target has to be parsed out of it.
        link.Text.ShouldBe("the LibreOffice site");
        link.HyperlinkTarget.ShouldBe("https://www.libreoffice.org/");
    }

    [Fact]
    public void AFieldContributesItsResultAndNotItsInstruction()
    {
        using IDocument document = Open("word-features.rtf");
        string text = document.Content.GetText();

        text.ShouldContain("title field: Paperless feature document.");
        text.Contains("HYPERLINK", StringComparison.Ordinal).ShouldBeFalse();
        text.Contains("PAGE", StringComparison.Ordinal).ShouldBeFalse();
    }

    [Fact]
    public void NotesAndCommentsBecomeTheirOwnSections()
    {
        using IDocument document = Open("word-features.rtf");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        ContentSection note = sections.Single(s => s.Kind == SectionKind.Note);
        note.Name.ShouldBe("1");
        // The note's own \chftn mark is not repeated into its text: the section already carries
        // the number, and the other formats' readers do not repeat it either.
        note.GetText().Trim().ShouldBe("The footnote body text.");

        ContentSection comment = sections.Single(s => s.Kind == SectionKind.Comment);
        comment.Name.ShouldBe("Alan Turing");
        comment.GetText().Trim().ShouldBe("A reviewer comment.");
    }

    [Fact]
    public void HeadersFootersAndTextBoxesAreSeparateFlows()
    {
        using IDocument document = Open("word-features.rtf");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        sections.Single(s => s.Kind == SectionKind.Header)
                .GetText().Trim().ShouldBe("Feature document header");
        sections.Single(s => s.Kind == SectionKind.Footer)
                .GetText().Trim().ShouldStartWith("Footer, page");
        sections.Single(s => s.Kind == SectionKind.Frame)
                .GetText().Trim().ShouldBe("Text inside a text box.");
    }

    [Fact]
    public void MetadataComesFromTheInfoGroup()
    {
        using IDocument document = Open("word-features.rtf");
        DocumentMetadata metadata = document.Metadata;

        metadata.Title.ShouldBe("Paperless feature document");
        metadata.Subject.ShouldBe("Extraction coverage");
        metadata.Keywords.ShouldBe(["extraction", "odf"]);
        metadata.GeneratorApplication.ShouldNotBeNull();

        // RTF spells a timestamp as a group of numeric control words rather than a string.
        metadata.Created!.Value.Year.ShouldBe(2026);
        metadata.Created!.Value.Month.ShouldBe(1);
        metadata.Created!.Value.Offset.ShouldBe(TimeSpan.Zero);
    }

    [Fact]
    public void TheSameDocumentInRtfAndOdtExtractsToTheSameBodyText()
    {
        using IDocument rtf = Open("word-features.rtf");
        using IDocument odt = Open("text-features.odt");

        // The footnote citation differs for the same reason as in DOCX: the ODT caches 0 and RTF
        // records only \ftnstart1, so the count starts at 1.
        string fromRtf = BodyText(rtf).Replace("reference1", "reference0", StringComparison.Ordinal);
        fromRtf.ShouldBe(BodyText(odt));

        static string BodyText(IDocument document)
            => string.Concat(document.Content.Children.OfType<ContentSection>()
                                     .Where(s => s.Kind == SectionKind.Body)
                                     .Select(s => s.GetText()));
    }

    // ------------------------------------------------------- the grammar, in isolation

    [Fact]
    public void ASpaceAfterAControlWordIsItsDelimiterAndNotText()
    {
        // Exactly one space is consumed. A second is text, which is why "b  c" keeps one.
        OpenLiteral(@"{\rtf1\ansi\pard\b bold\b0  plain\par}")
            .Content.GetText().ShouldBe("bold plain\n");
    }

    [Fact]
    public void ABareNewlineIsIgnoredRatherThanBecomingASpace()
    {
        // RTF wraps long lines for transport. Treating the wrap as content inserts a space or a
        // break into every long paragraph.
        OpenLiteral("{\\rtf1\\ansi\\pard one\ntwo\\par}")
            .Content.GetText().ShouldBe("onetwo\n");
    }

    [Fact]
    public void ABackslashBeforeANewlineIsAParagraphBreak()
        => OpenLiteral("{\\rtf1\\ansi\\pard one\\\ntwo\\par}")
            .Content.GetText().ShouldBe("one\ntwo\n");

    [Fact]
    public void EscapedBracesAndBackslashesAreText()
        => OpenLiteral(@"{\rtf1\ansi\pard a\{b\}c\\d\par}")
            .Content.GetText().ShouldBe("a{b}c\\d\n");

    [Fact]
    public void AnUnknownIgnorableDestinationIsSkippedWhole()
    {
        // \* says the group is meaningless to a reader that does not know the destination. RTF
        // puts private extensions in the same syntax as content, so recursing emits their
        // internals as text.
        OpenLiteral(@"{\rtf1\ansi\pard keep{\*\mysecret drop this}more\par}")
            .Content.GetText().ShouldBe("keepmore\n");
    }

    [Fact]
    public void ARepeatingHeaderRowComesFromTrhdr()
    {
        // \trhdr marks a row that repeats at the top of every page the table spans. LibreOffice's own
        // RTF export omits it, so this is the only way to cover it — and worth covering, because the
        // alternative is reporting a header count Paperless never established.
        IDocument document = OpenLiteral(
            @"{\rtf1\ansi\pard\intbl" +
            @"\trowd\trhdr\cellx1000\cellx2000 A\cell B\cell\row" +
            @"\pard\intbl\trowd\cellx1000\cellx2000 c\cell d\cell\row}");

        ContentTable table = Descendants(document.Content).OfType<ContentTable>().Single();
        table.Children.Count.ShouldBe(2);
        table.HeaderRowCount.ShouldBe(1);
    }

    [Fact]
    public void ANestedTableNestsRatherThanFlatteningIntoItsCell()
    {
        // \itap gives the depth, \nestcell and \nestrow end the inner cells and rows, and the inner
        // row's definition arrives *after* its cells inside {\*\nesttableprops} — an ignorable
        // destination that must nonetheless be read, since it holds the row's geometry and its end.
        IDocument document = OpenLiteral(
            @"{\rtf1\ansi\pard\intbl\trowd\cellx5000\cellx9000 " +
            @"\pard\intbl\itap2 inner a\nestcell\pard\intbl\itap2 inner b\nestcell" +
            @"{\*\nesttableprops\trowd\cellx2000\cellx4000\nestrow}{\nonesttables\par}" +
            @"\pard\intbl outer left\cell\pard\intbl outer right\cell\row}");

        List<ContentTable> tables = [.. document.Content.Children.OfType<ContentSection>()
                                        .SelectMany(s => s.Children).OfType<ContentTable>()];
        tables.Count.ShouldBe(1);

        ContentTableCell host = (ContentTableCell)((ContentTableRow)tables[0].Children[0]).Children[0];
        ContentTable inner = host.Children.OfType<ContentTable>().Single();
        inner.GetText().ShouldBe("inner a\tinner b\n");

        // {\nonesttables …} is a plain-text approximation of the same table for readers that cannot
        // nest, so reading it as well would duplicate every nested cell.
        document.Content.GetText().Split("inner a", StringSplitOptions.None).Length.ShouldBe(2);
    }

    [Fact]
    public void HighBytesAreDecodedInTheDocumentsCodePage()
    {
        // 0x93 and 0x94 are curly quotes in Windows-1252 and control characters in Latin-1, which
        // is the difference that makes substituting Latin-1 corrupt ordinary punctuation.
        OpenLiteral(@"{\rtf1\ansi\ansicpg1252\pard \'93quoted\'94\par}")
            .Content.GetText().ShouldBe("“quoted”\n");
    }

    [Fact]
    public void ADifferentCodePageDecodesTheSameBytesDifferently()
    {
        // The same byte is a curly quote in 1252 and a Cyrillic letter in 1251. Honouring
        // \ansicpg is the whole difference between readable text and mojibake.
        OpenLiteral(@"{\rtf1\ansi\ansicpg1251\pard \'d0\'e0\'e7\par}")
            .Content.GetText().ShouldBe("Раз\n");
    }

    [Fact]
    public void AUnicodeEscapeWinsAndItsFallbackIsSkipped()
    {
        // A writer emits both the code point and an approximation of it in the document's code
        // page. Emitting both doubles every non-ASCII character, usually with a question mark
        // beside it — which is what makes the symptom look like an encoding bug.
        OpenLiteral("{\\rtf1\\ansi\\ansicpg1252\\pard a\\u8226\\'95b\\par}")
            .Content.GetText().ShouldBe("a•b\n");
    }

    [Fact]
    public void TheFallbackLengthFollowsTheUnicodeSkipCount()
    {
        // \uc2 says the approximation takes two characters, as a DBCS fallback does.
        OpenLiteral("{\\rtf1\\ansi\\ansicpg1252\\uc2\\pard a\\u8226\\'3f\\'3fb\\par}")
            .Content.GetText().ShouldBe("a•b\n");
    }

    [Fact]
    public void ANegativeUnicodeParameterIsAHighCodePoint()
    {
        // RTF writes \uN signed, so anything above U+7FFF arrives negative: -3600 is U+F1F0.
        OpenLiteral(@"{\rtf1\ansi\pard \u-3600\'3f\par}")
            .Content.GetText().ShouldBe("\uF1F0\n");
    }

    [Fact]
    public void HiddenTextIsNotExtracted()
    {
        // Unlike a header or a footnote, which a reader sees and only the reference filter drops,
        // hidden text is displayed by nothing.
        OpenLiteral(@"{\rtf1\ansi\pard shown {\v hidden }shown\par}")
            .Content.GetText().ShouldBe("shown shown\n");
    }

    [Fact]
    public void ABraceRestoresTheFormattingItSaved()
    {
        // Braces are RTF's only scoping: bold set inside a group ends with it.
        List<ContentRun> runs = [.. Descendants(
            OpenLiteral(@"{\rtf1\ansi\pard plain {\b bold }plain\par}").Content)
            .OfType<ContentRun>()];

        runs.Count.ShouldBe(3);
        runs[0].Emphasis.ShouldBe(RunEmphasis.None);
        runs[1].Emphasis.ShouldBe(RunEmphasis.Bold);
        runs[2].Emphasis.ShouldBe(RunEmphasis.None);
    }

    [Fact]
    public void SomethingThatIsNotRtfIsRejected()
    {
        // The format is stated rather than sniffed, because sniffing would reject it earlier as an
        // unrecognised document — and what is under test is the reader's own signature check, which
        // is what protects it from being handed the wrong bytes by a caller that knows better.
        using MemoryStream stream = new(Encoding.ASCII.GetBytes("this is not rtf"));
        using DocumentSource source = DocumentSource.FromStream(stream, "claimed.rtf");

        MalformedDocumentException thrown = Should.Throw<MalformedDocumentException>(
            () => Rtf.RtfReader.Read(source, DocumentFormat.Rtf));
        thrown.Message.ShouldContain("rtf");
    }

    [Fact]
    public void UnbalancedBracesDoNotLoseTheDocument()
    {
        // A truncated file is the common case of this, and the text before the truncation is
        // still worth returning.
        OpenLiteral(@"{\rtf1\ansi\pard text before{\b bold")
            .Content.GetText().ShouldContain("text before");
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

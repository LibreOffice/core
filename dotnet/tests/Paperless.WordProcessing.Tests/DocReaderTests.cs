using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Core.Graphics;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests the legacy binary Word reader against a corpus document written by LibreOffice.
/// </summary>
/// <remarks>
/// The corpus document is the same one as <c>text-features.odt</c>, <c>word-features.docx</c> and
/// <c>word-features.rtf</c>, converted. Holding one document in four formats is what makes a
/// difference between them meaningful: it is either a defect or a genuine difference between the
/// files, and both are worth knowing about — so the strongest test here is the one that asserts the
/// DOC and the DOCX extract to exactly the same text.
/// </remarks>
public class DocReaderTests
{
    private static IDocument Open(string name)
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    [Fact]
    public void ADocDocumentReadsWithItsFormatAndFamilyIdentified()
    {
        using IDocument document = Open("word-features.doc");

        document.Format.ShouldBe(DocumentFormat.Doc);
        document.Family.ShouldBe(DocumentFamily.WordProcessing);
        document.Diagnostics.ShouldNotContain(d => d.Severity == DiagnosticSeverity.Error);
    }

    [Fact]
    public void TheBodyIsOneSectionWithTheOtherFlowsBesideIt()
    {
        using IDocument document = Open("word-features.doc");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        sections[0].Kind.ShouldBe(SectionKind.Body);
        sections.Select(s => s.Kind).Distinct().ShouldBe(
            [SectionKind.Body, SectionKind.Note, SectionKind.Comment, SectionKind.Frame,
             SectionKind.Header, SectionKind.Footer],
            ignoreOrder: true);
    }

    [Fact]
    public void HeadingsComeFromTheOutlineLevelSprm()
    {
        using IDocument document = Open("word-features.doc");
        List<ContentParagraph> headings =
            [.. Descendants(document.Content).OfType<ContentParagraph>()
                .Where(p => p.HeadingLevel is not null)];

        // sprmPOutLvl is zero-based and reserves 9 for body text, so a level-1 heading arrives as 0.
        headings.Select(h => h.HeadingLevel).ShouldBe([1, 2, 2, 3]);
        headings[0].GetText().Trim().ShouldBe("Top level heading");

        // The style index in the PAPX is a number; the name comes from the STSH.
        headings[0].StyleName.ShouldBe("Heading 1");
    }

    [Fact]
    public void CharacterFormattingComesFromTheExceptionTables()
    {
        using IDocument document = Open("word-features.doc");
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
        // sprmCLid carries a Windows language id, not a tag; 1031 is German.
        mixed.Language.ShouldBe("de-DE");

        // The style's own name, from the STSH — Writer's "Strong Emphasis" is called "Strong" here.
        // The name's length prefix is two bytes: read as one, every character of every style name
        // comes back shifted by eight bits, so "Heading 1" arrives as CJK rather than as nothing.
        mixed.StyleName.ShouldBe("Strong");
        runs.ShouldContain(r => r.Text == "Strong Emphasis" && r.StyleName == "Strong");
    }

    [Fact]
    public void AParagraphStyleContributesTheCharacterFormattingOfItsRuns()
    {
        using IDocument document = Open("word-features.doc");
        ContentParagraph heading = Descendants(document.Content).OfType<ContentParagraph>()
            .First(p => p.HeadingLevel == 1);

        // Nothing in the heading's runs says "bold": the Heading 1 style's CHPX half does. A reader
        // that takes only the PAPX half of a paragraph style loses every character property it sets.
        heading.Children.OfType<ContentRun>()
               .ShouldAllBe(r => r.Emphasis.HasFlag(RunEmphasis.Bold));
    }

    [Fact]
    public void ListMarkersAreComputedFromTheListTables()
    {
        using IDocument document = Open("word-features.doc");
        List<ContentParagraph> items =
            [.. Descendants(document.Content).OfType<ContentParagraph>()
                .Where(p => p.ListMarker is not null)];

        // The label exists nowhere in the file: a paragraph names a list instance and a level, and
        // the reader counts. So these are what Paperless computed from the LSTF/LVL/LFO tables.
        items.ShouldContain(p => p.ListMarker == "•"
                                 && p.GetText().Contains("First bullet", StringComparison.Ordinal));
        items.ShouldContain(p => p.ListMarker == "◦"
                                 && p.GetText().Contains("Nested bullet", StringComparison.Ordinal));
        items.ShouldContain(p => p.ListMarker == "1."
                                 && p.GetText().Contains("First numbered", StringComparison.Ordinal));
        items.ShouldContain(p => p.ListMarker == "2."
                                 && p.GetText().Contains("Second numbered", StringComparison.Ordinal));
        items.ShouldContain(p => p.ListMarker == "a)"
                                 && p.GetText().Contains("Nested letter", StringComparison.Ordinal));
        items.ShouldContain(p => p.ListMarker == "b)"
                                 && p.GetText().Contains("Second nested letter", StringComparison.Ordinal));
    }

    [Fact]
    public void NumberingContinuesPastAnInterruptingParagraph()
    {
        using IDocument document = Open("word-features.doc");
        List<ContentParagraph> paragraphs = [.. Descendants(document.Content).OfType<ContentParagraph>()];

        // The third item comes after a continuation paragraph and a nested sub-list, and still
        // numbers 3: a deeper level restarting must not reset the level above it.
        ContentParagraph third = paragraphs.Single(
            p => p.GetText().Contains("Third numbered", StringComparison.Ordinal));
        third.ListMarker.ShouldBe("3.");

        ContentParagraph continuation = paragraphs.Single(
            p => p.GetText().Contains("Continuation paragraph", StringComparison.Ordinal));
        continuation.ListMarker.ShouldBeNull();
        continuation.ListLevel.ShouldBeNull();
    }

    [Fact]
    public void TablesKeepTheirRowsAndCells()
    {
        using IDocument document = Open("word-features.doc");
        ContentTable table = Descendants(document.Content).OfType<ContentTable>().Single();

        List<ContentTableRow> rows = [.. table.Children.Cast<ContentTableRow>()];
        rows.Count.ShouldBe(3);

        table.ColumnCount.ShouldBe(3);
        rows[0].GetText().ShouldBe("Head A\tHead B\tHead C\n");
        rows[2].GetText().ShouldBe("Row two A\t\tRow two C\n");

        // LibreOffice writes the merge as geometry alone: no cell carries a merge flag, and the
        // merged cell's right edge simply reaches where two columns end in the rows around it. So the
        // span comes from the table's column grid, exactly as it does in RTF.
        rows[1].Children.Count.ShouldBe(2);
        ContentTableCell merged = (ContentTableCell)rows[1].Children[0];
        merged.ColumnSpan.ShouldBe(2);
        merged.GetText().ShouldBe("Merged across two columns");
        ((ContentTableCell)rows[1].Children[1]).Column.ShouldBe(2);

        // sprmTTableHeader sits on the row-end paragraph of each repeating row, so only the run of
        // them at the top of the table counts as the header.
        table.HeaderRowCount.ShouldBe(1);
    }

    [Fact]
    public void AFieldContributesItsResultAndNotItsInstruction()
    {
        using IDocument document = Open("word-features.doc");
        string text = document.Content.GetText();

        text.ShouldContain("title field: Paperless feature document.");
        text.Contains("HYPERLINK", StringComparison.Ordinal).ShouldBeFalse();
        text.Contains(" TITLE ", StringComparison.Ordinal).ShouldBeFalse();
        text.Contains(" PAGE ", StringComparison.Ordinal).ShouldBeFalse();
    }

    [Fact]
    public void AHyperlinkComesFromItsFieldInstruction()
    {
        using IDocument document = Open("word-features.doc");
        ContentRun link = Descendants(document.Content).OfType<ContentRun>()
                                                      .First(r => r.HyperlinkTarget is not null);

        // WW8 has no hyperlink markup: a link is a field whose instruction reads HYPERLINK "…". The
        // instruction is skipped as text and read as an instruction, which is the only way to know
        // where a link points — the cached result says only what it looked like.
        link.Text.ShouldBe("the LibreOffice site");
        link.HyperlinkTarget.ShouldBe("https://www.libreoffice.org/");

        // And the link ends where the field does, rather than colouring the rest of the paragraph.
        List<ContentRun> runs = [.. Descendants(document.Content).OfType<ContentRun>()];
        runs.Where(r => r.HyperlinkTarget is not null).Count().ShouldBe(1);
    }

    [Fact]
    public void NotesAndCommentsBecomeTheirOwnSections()
    {
        using IDocument document = Open("word-features.doc");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        ContentSection note = sections.Single(s => s.Kind == SectionKind.Note);
        note.Name.ShouldBe("1");
        note.GetText().Trim().ShouldBe("The footnote body text.");

        // The author is not the reference record's first field: it follows a fixed 22 bytes of
        // initials, and the name itself comes from a bare array of strings rather than a string
        // table.
        ContentSection comment = sections.Single(s => s.Kind == SectionKind.Comment);
        comment.Name.ShouldBe("Alan Turing");
        comment.GetText().Trim().ShouldBe("A reviewer comment.");
    }

    [Fact]
    public void HeadersFootersAndTextBoxesAreSeparateFlows()
    {
        using IDocument document = Open("word-features.doc");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        // Word writes all twelve header slots whether a section uses them or not, and names them by
        // position rather than by any label in the file.
        ContentSection header = sections.Single(s => s.Kind == SectionKind.Header);
        header.Name.ShouldBe("odd header");
        header.GetText().Trim().ShouldBe("Feature document header");

        sections.Single(s => s.Kind == SectionKind.Footer)
                .GetText().Trim().ShouldStartWith("Footer, page");
        sections.Single(s => s.Kind == SectionKind.Frame)
                .GetText().Trim().ShouldBe("Text inside a text box.");
    }

    [Fact]
    public void AStoryHasNoTrailingEmptyParagraphFromItsTerminator()
    {
        using IDocument document = Open("word-features.doc");

        // Every subdocument story ends with a paragraph mark that terminates it rather than opening
        // another paragraph. Counting it produces one empty paragraph inside every header, footer,
        // note and text box in the document.
        foreach (ContentSection section in document.Content.Children.OfType<ContentSection>())
        {
            if (section.Kind == SectionKind.Body) continue;
            section.Children.OfType<ContentParagraph>().Count().ShouldBe(1, $"{section.Kind}");
        }
    }

    [Fact]
    public void ADrawingAnchorIsNotReportedAsAnImage()
    {
        using IDocument document = Open("word-features.doc");

        // The text box is written as a SHAPE field whose cached result is a drawing anchor and a
        // picture placeholder. Neither is an embedded image, and the shape's text has already
        // arrived as its own frame section.
        Descendants(document.Content).OfType<ContentImage>().ShouldBeEmpty();
    }

    [Fact]
    public void MetadataComesFromTheOlePropertySets()
    {
        using IDocument document = Open("word-features.doc");
        DocumentMetadata metadata = document.Metadata;

        metadata.Title.ShouldBe("Paperless feature document");
        metadata.Subject.ShouldBe("Extraction coverage");
        metadata.Author.ShouldNotBeNullOrWhiteSpace();
        metadata.Keywords.ShouldBe(["extraction", "odf"]);
        metadata.Description.ShouldNotBeNullOrWhiteSpace();
        metadata.Created.ShouldNotBeNull();
        metadata.Modified.ShouldNotBeNull();

        // Editing time is a FILETIME too, but it counts elapsed ticks from the 1601 epoch rather than
        // naming an instant — so a reader that treats the field as one thing reports the document as
        // having been edited for four centuries, or created in 1601.
        metadata.TotalEditingTime.ShouldNotBeNull();
        metadata.TotalEditingTime!.Value.ShouldBeLessThan(TimeSpan.FromDays(365));
        metadata.Created!.Value.Year.ShouldBeGreaterThan(2000);

        // No generator: the property set has a slot for the application name and LibreOffice leaves
        // it out. Reporting null is the honest answer rather than a guess from the file's shape.
        metadata.GeneratorApplication.ShouldBeNull();
    }

    [Fact]
    public void TheSameDocumentInDocAndDocxExtractsToTheSameText()
    {
        using IDocument doc = Open("word-features.doc");
        using IDocument docx = Open("word-features.docx");

        // Not just the body: the two readers should agree on every flow, in the same order. This is
        // the assertion that catches a mis-split subdocument, a lost story terminator or a list
        // counter that advanced at the wrong moment, none of which the structural tests above see
        // in isolation.
        doc.Content.GetText().ShouldBe(docx.Content.GetText());
    }

    /// <summary>
    /// The shade comes from the RGB array rather than from the palette one, and from its background.
    /// </summary>
    /// <remarks>
    /// Both facts need a colour to see, which is why they are asserted here rather than in the fidelity
    /// comparison: a fill's coordinates are the same either way. The document was written with
    /// <c>#CCCCCC</c>, which the newer <c>sprmTDefTableShd</c> carries exactly and the older
    /// <c>sprmTDefTableShd80</c> can only round to Word's nearest palette entry, <c>#C0C0C0</c>. And the
    /// fill pattern is "clear", so what shows is its <em>background</em>; reading the foreground gives
    /// black, since an automatic foreground is black and an automatic background is white.
    /// </remarks>
    [Fact]
    public void ACellsShadeIsTheRgbBackgroundRatherThanThePaletteOrTheForeground()
    {
        List<PageTableCell> cells = TableCells("table-shading.doc");

        cells.Take(3).Select(cell => cell.Shading)
            .ShouldAllBe(shade => shade == Colour.FromRgb(0xCCCCCC));

        cells.Skip(3).ShouldAllBe(cell => cell.Shading == null);
    }

    /// <summary>
    /// A border's width is eighths of a point, and its colour a palette index in the older BRC form.
    /// </summary>
    /// <remarks>
    /// <c>dptLineWidth</c> 4 is half a point — ten twips — which is the one number that would still place
    /// the text plausibly if it were read as twips or half-points, and would place it a point out. The
    /// colour arrives twice over as well: <c>ico</c> 6 in the cell descriptor, which is Word's "light red"
    /// and so <c>#FF0000</c>, and the same again as a real RGB in the <c>sprmTSetBrc</c> that follows.
    /// </remarks>
    [Fact]
    public void ACellsBorderIsHalfAPointOfPaletteRed()
    {
        foreach (PageTableCell cell in TableCells("table-borders.doc"))
        {
            foreach (TableBorder border in
                     (TableBorder[])[cell.Borders.Top, cell.Borders.Left,
                                     cell.Borders.Bottom, cell.Borders.Right])
            {
                border.Width.Points.ShouldBe(0.5, 0.001);
                border.Colour.ShouldBe(Colour.FromRgb(0xFF0000));
            }
        }
    }

    /// <summary>Every cell of the one table on a document's first page, in row order.</summary>
    private static List<PageTableCell> TableCells(string name)
    {
        using IDocument document = Open(name);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return [.. pages.Pages[0].Tables.SelectMany(table => table.Cells).Select(cell => cell.Cell)];
    }

    [Fact]
    public void AnEncryptedDocumentReportsThatAPasswordIsNeeded()
    {
        // Encryption is a bit in the FIB, so it is known before anything is decoded — which is what
        // makes a clear error possible rather than a stream of malformed-structure diagnostics.
        Ww8Fib fib = Ww8Fib.Parse(EncryptedFibBytes());
        fib.IsEncrypted.ShouldBeTrue();
    }

    /// <summary>
    /// A minimal FIB with the encryption flag set, for the one branch a corpus document cannot
    /// exercise without committing an encrypted file.
    /// </summary>
    private static byte[] EncryptedFibBytes()
    {
        byte[] fib = new byte[1024];
        fib[0] = 0xEC;
        fib[1] = 0xA5;
        fib[2] = 0xC1;
        fib[3] = 0x00;
        // The flag word: fEncrypted is bit 8.
        fib[10] = 0x00;
        fib[11] = 0x01;
        return fib;
    }

    private static IEnumerable<ContentNode> Descendants(ContentNode node)
    {
        foreach (ContentNode child in node.Children)
        {
            yield return child;
            foreach (ContentNode inner in Descendants(child)) yield return inner;
        }
    }
}

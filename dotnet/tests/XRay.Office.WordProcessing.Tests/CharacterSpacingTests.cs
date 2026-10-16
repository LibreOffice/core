using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Model;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The <c>w:spacing</c> of a <c>w:rPr</c>, which is tracking and not paragraph spacing.
/// </summary>
/// <remarks>
/// <para>
/// Two unrelated properties share the name. In a <c>w:pPr</c> it is the gap above and below a paragraph;
/// in a <c>w:rPr</c> it is a constant distance put <em>between characters</em> — <c>SvxKerningItem</c>,
/// which <c>DomainMapper</c> reaches by converting the twips straight to <c>CharKerning</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:2468–2480). It is not the face's own pair
/// kerning either; that is <c>w:kern</c> and a different item.
/// </para>
/// <para>
/// Nothing read the character one. <c>FormattedRun.Tracking</c> had been built for it and only the
/// presentations reader populated it, which is the shape of defect this project keeps finding: a property
/// modelled everywhere and consumed nowhere.
/// </para>
/// <para>
/// Found on <c>words/batch-011/docx/FG-AI4NDM-TR-skeleton_template.docx</c>, whose ITU cover sets
/// "International Telecommunication Union" at <c>w:spacing w:val="100"</c> — 5 pt between every letter.
/// 58 of the words track's 134 DOCX state one, most of them negative, and a run at −16 twips over a
/// fifty-character line is 40 pt of width the reference does not spend.
/// </para>
/// </remarks>
public sealed class CharacterSpacingTests
{
    /// <summary>Tracking widens a paragraph by one unit per character gap.</summary>
    /// <remarks>
    /// Asserted as a difference against the same text untracked rather than against an absolute width, so
    /// the test says what tracking costs without also pinning the face's advances.
    /// </remarks>
    [Theory]
    [InlineData(100)]
    [InlineData(-10)]
    public void TrackingChangesTheTextsWidthByOneUnitPerGap(int twips)
    {
        const string Sentence = "Tracking is a distance between letters.";

        Length bare = Width(Sentence, spacing: null);
        Length tracked = Width(Sentence, spacing: twips);

        // The measurement charges the gap before each character and the first pays nothing, so a
        // paragraph of n characters carries n − 1. See FormattedRun.Tracking.
        (tracked - bare).ShouldBe(Length.FromTwips(twips) * (Sentence.Length - 1));
    }

    /// <summary>
    /// The drawn pen advances every glyph by the tracking, a run's last included, so the text after a
    /// tracked run starts one unit further on.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This is one unit more than the measurement charges, and the difference is
    /// LibreOffice's own.</strong> <c>SvxFont::QuickGetTextSize</c> takes the trailing kern back off —
    /// which <c>XRay.Office.Text.Tests</c>' <c>CharacterTrackingTests</c> pins and which is right
    /// for the text-size query it describes — while the pen keeps it. Measured on both engines with a
    /// tracked run of three characters followed by an untracked one at a declared 2.25 pt
    /// (<c>probes/tracking-trailing-gap/</c>): Writer gives gaps of 2.20, 2.24 and then 2.29 into the
    /// untracked run, Impress 2.24, 2.26 and 2.29. The gap into the next run is the same as the gaps
    /// inside the tracked one.
    /// </para>
    /// <para>
    /// Worth a word on nearly every page of <c>OM template for non-complex NCC operators</c>, whose
    /// header is <c>Rev.</c> then a space run then <c>X</c> then <c>of</c> with no space between the
    /// last two: the <c>X</c> run's 45 twentieths open a 2.25 pt gap, so the reference's text layer
    /// reads <c>X of</c> where ours read <c>Xof</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheDrawnPenCarriesATrackedRunsTrailingUnit()
    {
        const int Twips = 45;

        Length tracked = DrawnAdvance("AAA", Twips);
        Length bare = DrawnAdvance("AAA", spacing: null);

        (tracked - bare).ShouldBe(
            Length.FromTwips(Twips) * 3,
            "three glyphs each carry the tracking, the last included");
    }

    /// <summary>How far the pen advances across a run as it is actually drawn.</summary>
    /// <remarks>
    /// Summed from the emitted glyphs rather than asked of the measurement, because the two answers
    /// differ by design here and it is the drawn one this pins.
    /// </remarks>
    private static Length DrawnAdvance(string text, int? spacing)
    {
        string runProperties = spacing is { } twips
            ? $"<w:rPr>{Face}<w:spacing w:val=\"{twips}\"/></w:rPr>"
            : $"<w:rPr>{Face}</w:rPr>";

        PageParagraph paragraph = Paragraph($"""
            <w:p>
              <w:r>{runProperties}<w:t xml:space="preserve">{text}</w:t></w:r>
            </w:p>
            """);

        List<PageBlock> blocks = [paragraph];
        List<LaidOutPage> pages = new Paginator(PaginationOptions.Word).Paginate(
            blocks, new WritingSection { Page = DrawGeometry });

        GlyphCollector collector = new();
        foreach (LaidOutPage page in pages) PageDrawing.Draw(page, blocks, collector);

        return collector.Advance;
    }

    /// <summary>The face these widths are measured in, named so that nothing else can choose it.</summary>
    private const string Face =
        "<w:rFonts w:ascii=\"Liberation Serif\" w:hAnsi=\"Liberation Serif\"/><w:sz w:val=\"20\"/>";

    /// <summary>An A4 page, wide enough that the run is never broken.</summary>
    private static PageGeometry DrawGeometry { get; } = new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = PageMargins.Uniform(Length.FromTwips(567)),
    };

    /// <summary>Adds up every glyph advance the page draws and ignores everything else.</summary>
    /// <remarks>
    /// The whole sink rather than a base class, because there is no no-op one to derive from — the
    /// interface has thirteen members and no default implementations.
    /// </remarks>
    private sealed class GlyphCollector : IDrawingSink
    {
        public Length Advance { get; private set; }

        public void DrawGlyphRun(GlyphRun run, Paint paint)
        {
            ArgumentNullException.ThrowIfNull(run);

            foreach (PositionedGlyph glyph in run.Glyphs) Advance += glyph.Advance;
        }

        public void BeginPage(DocSize size) { }

        public void EndPage() { }

        public void Save() { }

        public void Restore() { }

        public void Transform(AffineTransform transform) { }

        public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero) { }

        public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero) { }

        public void StrokePath(GraphicsPath path, Stroke stroke) { }

        public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0) { }

        public void BeginTransparencyGroup(double opacity) { }

        public void EndTransparencyGroup() { }
    }

    /// <summary>A run's own tracking survives the uniform-paragraph shortcut.</summary>
    /// <remarks>
    /// The readers drop a paragraph's run list when every run agrees with the paragraph mark, because a
    /// property that only decides what a mark looks like cannot decide where it lands. Tracking is not
    /// such a property — it is a width — so a tracked run inside an untracked paragraph has to keep its
    /// run or it is measured at the paragraph's answer. This is the case that would silently pass.
    /// </remarks>
    [Fact]
    public void ATrackedRunInsideAnUntrackedParagraphKeepsItsWidth()
    {
        PageParagraph paragraph = Paragraph(MixedDocument);

        paragraph.Runs.Count.ShouldBeGreaterThan(1,
            "the tracked run differs from the paragraph mark, so the shortcut must not fold it away");
        paragraph.Runs.Select(run => run.Tracking)
            .ShouldContain(Length.FromTwips(100));
    }

    /// <summary>The paragraph <c>w:spacing</c> is not mistaken for the character one.</summary>
    /// <remarks>
    /// The two elements have the same name and different parents, and the resolution that finds the
    /// character one only ever looks inside <c>w:rPr</c>. Without this the gap above a paragraph would be
    /// charged as tracking on every one of its letters, which is a defect no width test on a tracked
    /// document would catch.
    /// </remarks>
    [Fact]
    public void AParagraphSpacingIsNotReadAsTracking()
    {
        PageParagraph paragraph = Paragraph(ParagraphSpacingDocument);

        paragraph.Tracking.ShouldBe(Length.Zero);
        paragraph.Runs.Select(run => run.Tracking).ShouldAllBe(tracking => tracking == Length.Zero);
    }

    /// <summary>A tracked run does not ligate, all the way from the markup.</summary>
    /// <remarks>
    /// <para>
    /// The document-level end of <c>XRay.Office.Text.Tests.TrackingLigatureTests</c>, which has the
    /// rule and the reasoning. Here because the rule has to survive the whole path — the reader
    /// putting <c>w:spacing</c> on the run, the run reaching the measurement with its tracking
    /// intact, and the measurement passing it to the shaper — and each of those three has broken
    /// independently before.
    /// </para>
    /// <para>
    /// The glyph count is the assertion because it is what a PDF writer turns into
    /// <c>ToUnicode</c> entries: a ligature is one code mapping to two characters, and poppler
    /// answers a multi-character entry by shattering the whole line into single-character words.
    /// That cost <c>words/batch-008/…/FAA-2017-0628-0002_attachment_1.docx</c> 28 words of 638
    /// on text that was character-for-character identical to the reference's.
    /// </para>
    /// </remarks>
    [Fact]
    public void ATrackedRunDoesNotLigate()
    {
        // Calibri resolves to Carlito, whose liga lookup maps t+i to one glyph. Named explicitly
        // rather than left to the default, or the test measures whatever face the machine prefers.
        const string Text = "Formation";

        int plain = Glyphs(Text, spacing: null);
        int tracked = Glyphs(Text, spacing: 60);

        Assert.SkipWhen(plain == Text.Length, "the resolved face does not ligate ti");

        plain.ShouldBe(Text.Length - 1, "untracked, ti is one glyph");
        tracked.ShouldBe(Text.Length, "tracked, every character is its own glyph");
    }

    private static int Glyphs(string sentence, int? spacing)
    {
        string runProperties = spacing is { } twips
            ? $"<w:rPr><w:rFonts w:ascii=\"Calibri\" w:hAnsi=\"Calibri\"/><w:spacing w:val=\"{twips}\"/></w:rPr>"
            : "<w:rPr><w:rFonts w:ascii=\"Calibri\" w:hAnsi=\"Calibri\"/></w:rPr>";

        PageParagraph paragraph = Paragraph($"""
            <w:p>
              <w:r>{runProperties}<w:t xml:space="preserve">{sentence}</w:t></w:r>
            </w:p>
            """);

        return paragraph.Measure().Runs.Sum(run => run.Shaped.Glyphs.Count);
    }

    /// <summary>The corpus fixture is the width LibreOffice makes it.</summary>
    /// <remarks>
    /// <c>character-spacing.docx</c> was rendered by LibreOffice 24.2.7.2, which breaks its tracked
    /// paragraph onto three lines and its untracked twin onto two. Untracked, both come out at two.
    /// </remarks>
    [Fact]
    public void TheCorpusFixtureBreaksWhereLibreOfficeBreaksIt()
    {
        using IDocument document = new WordProcessingReader()
            .Read(DocumentSource.FromFile(Corpus.Require("character-spacing.docx")));
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        List<PageParagraph> paragraphs = [.. pages.Blocks.OfType<PageParagraph>()];
        PageParagraph tracked = paragraphs.First(p => p.Text.StartsWith("Tracked:", StringComparison.Ordinal));
        PageParagraph plain = paragraphs.First(p => p.Text.StartsWith("Plain:", StringComparison.Ordinal));

        Lines(pages, tracked).ShouldBe(3);
        Lines(pages, plain).ShouldBe(2);
    }

    private static int Lines(WordProcessingPages pages, PageParagraph paragraph)
        => pages.Pages.SelectMany(page => page.Lines)
            .Count(line => ReferenceEquals(pages.Blocks[line.ParagraphIndex], paragraph));

    private static Length Width(string sentence, int? spacing)
    {
        // The face and size are named rather than left to the package's own defaults, exactly as
        // `Glyphs` above names Calibri, and for a sharper reason: this measures an *exact* width
        // difference, so a face whose pair kerning the tracked run then suppresses puts that
        // kerning into the answer. Liberation Serif at 10 pt is what a package declaring no
        // `w:docDefaults/w:rPrDefault` resolved to here until `WordParagraphFormats` learned the
        // importer's real default for that case — Calibri 11 pt — which kerns this sentence by
        // 0.392 pt and made the difference 190.392 against 190.
        string runProperties = spacing is { } twips
            ? $"<w:rPr>{Face}<w:spacing w:val=\"{twips}\"/></w:rPr>"
            : $"<w:rPr>{Face}</w:rPr>";

        PageParagraph paragraph = Paragraph($"""
            <w:p>
              <w:r>{runProperties}<w:t xml:space="preserve">{sentence}</w:t></w:r>
            </w:p>
            """);

        return paragraph.Measure().WidthBetween(0, paragraph.Text.Length);
    }

    private const string MixedDocument = """
        <w:p>
          <w:r><w:t xml:space="preserve">plain </w:t></w:r>
          <w:r><w:rPr><w:spacing w:val="100"/></w:rPr><w:t>tracked</w:t></w:r>
        </w:p>
        """;

    private const string ParagraphSpacingDocument = """
        <w:p>
          <w:pPr><w:spacing w:before="240" w:after="240"/></w:pPr>
          <w:r><w:t>a paragraph whose spacing is between paragraphs</w:t></w:r>
        </w:p>
        """;

    private static PageParagraph Paragraph(string body)
    {
        using IDocument document = Open(body);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageParagraph>().First(block => block.Text.Length > 0);
    }

    private static IDocument Open(string body)
    {
        MemoryStream package = BuildPackage(body);
        using DocumentSource source = DocumentSource.FromStream(package, "tracking.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(string body)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                {body}
                <w:sectPr><w:pgSz w:w="11906" w:h="16838"/></w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }
}

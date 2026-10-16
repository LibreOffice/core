using System.Globalization;
using System.Text;
using XRay.Office.Cli;
using XRay.Office.Core.Documents;
using XRay.Office.Rendering.Pdf;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// The reader behind <c>xray-office analyze</c>, which replaced poppler in the corpus gate.
/// </summary>
/// <remarks>
/// <para>
/// It sits in the rendering tests because this is where PDFs are made, and every question it
/// answers is a question about a written file. Two of the cases below are deliberately built by
/// hand rather than rendered: a minimal PDF is the only way to state "this glyph is off the page"
/// or "this face has no font program" as an input rather than as a hope.
/// </para>
/// <para>
/// The reason the verb exists at all is that shelling out to <c>pdfinfo</c>, <c>pdftotext</c> and
/// <c>pdffonts</c> made the machine's poppler an undeclared input to every figure the project has
/// recorded — caught when our own word counts moved on 169 of 200 documents with the renderer's
/// source provably unchanged. These tests are the thing that keeps the replacement honest, so each
/// of them names what it would catch.
/// </para>
/// </remarks>
public sealed class PdfAnalysisTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    // --------------------------------------------------------------------------- geometry

    /// <summary>Page count and page size are read out of the file, not inferred.</summary>
    /// <remarks>
    /// The page count is the gate's first check and the one that makes every later check
    /// meaningful, so it is asserted against a media box this test wrote itself. The odd size is
    /// deliberate: 200×300 is nothing a default would produce by accident.
    /// </remarks>
    [Fact]
    public void PageCountAndPageSizeAreReadFromTheFile()
    {
        using TempPdf pdf = TempPdf.Write(MinimalPdf.Build(
            mediaBox: "[0 0 200 300]",
            content: "BT /F1 12 Tf 20 100 Td (one two three) Tj ET",
            fonts: [MinimalPdf.SimpleFont("Helvetica", embedded: false)]));

        PdfAnalysisResult result = PdfAnalysis.Analyze(pdf.Path);

        result.Error.ShouldBeNull();
        result.PageCount.ShouldBe(1);
        result.Pages.Count.ShouldBe(1);
        result.Pages[0].WidthPoints.ShouldBe(200);
        result.Pages[0].HeightPoints.ShouldBe(300);
        result.Pages[0].MediaWidthPoints.ShouldBe(200);
        result.DistinctPageSizeCount.ShouldBe(1);
    }

    /// <summary>Glyphs drawn outside the page are not text, because nothing ever shows them.</summary>
    /// <remarks>
    /// <para>
    /// The single largest term in the difference between this reader and <c>pdftotext</c>, and the
    /// one most likely to be lost in a refactor because dropping the filter makes nothing fail
    /// loudly — it makes the corpus figures quietly bigger. Poppler discards out-of-bounds
    /// characters in <c>TextPage::addChar</c>; before this reader did the same,
    /// <c>CIS_Debian_Linux_8_Benchmark_v1.0.0.xls</c> — half of whose 125 086 glyphs sit off the
    /// page — reported 18 106 words against poppler's 9290.
    /// </para>
    /// <para>
    /// The off-page run is placed at x=900 on a 200-point-wide page, well beyond any rounding.
    /// </para>
    /// </remarks>
    [Fact]
    public void GlyphsDrawnOffThePageAreNotCounted()
    {
        using TempPdf pdf = TempPdf.Write(MinimalPdf.Build(
            mediaBox: "[0 0 200 300]",
            content: "BT /F1 12 Tf 20 100 Td (on the page) Tj ET\n"
                     + "BT /F1 12 Tf 900 100 Td (far off to the right) Tj ET",
            fonts: [MinimalPdf.SimpleFont("Helvetica", embedded: false)]));

        PdfAnalysisResult result = PdfAnalysis.Analyze(pdf.Path);

        result.Error.ShouldBeNull();
        result.Words.Raw.ShouldBe(3, "only 'on the page' is on the page");
        result.Words.Alphanumeric.ShouldBe(3);
    }

    // ------------------------------------------------------------------------------ fonts

    /// <summary>A face named without a font program is reported unembedded.</summary>
    /// <remarks>
    /// This is the check the shell version got wrong for a long time by reading
    /// <c>pdffonts</c>'s column <c>NF-3</c> instead of <c>NF-4</c> — which reads <c>sub</c>, and
    /// happens to agree only for a font whose type name is more than one field. Every face
    /// XRay.Office writes is "TrueType", one field, so the gate's unembedded check tested nothing
    /// about our own output at all. Stated here as a property of the file rather than of a column.
    /// </remarks>
    [Fact]
    public void AFaceWithNoFontProgramIsReportedUnembedded()
    {
        using TempPdf pdf = TempPdf.Write(MinimalPdf.Build(
            mediaBox: "[0 0 200 300]",
            content: "BT /F1 12 Tf 20 100 Td (hello) Tj ET",
            fonts: [MinimalPdf.SimpleFont("Helvetica", embedded: false)]));

        PdfAnalysisResult result = PdfAnalysis.Analyze(pdf.Path);

        result.Fonts.Count.ShouldBe(1);
        result.Fonts[0].Name.ShouldBe("Helvetica");
        result.Fonts[0].Embedded.ShouldBeFalse();
        result.UnembeddedFontCount.ShouldBe(1);
    }

    /// <summary>A descriptor carrying a font program is reported embedded, whichever key holds it.</summary>
    /// <remarks>
    /// <para>
    /// The other half of the previous test, and not redundant with it: a predicate that always
    /// answers "no" passes that one.
    /// </para>
    /// <para>
    /// All three keys, because they are three separate reads and a reader can know one and not the
    /// others. <c>/FontFile</c> is a Type 1 program, <c>/FontFile2</c> a TrueType one, and
    /// <c>/FontFile3</c> one whose format its own stream names — which is where every CFF face
    /// lands. Covering only <c>/FontFile2</c> left the <c>/FontFile3</c> read unverified: removing
    /// it changed no test.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("FontFile")]
    [InlineData("FontFile2")]
    [InlineData("FontFile3")]
    public void AFaceWithAFontProgramIsReportedEmbedded(string fontFileKey)
    {
        using TempPdf pdf = TempPdf.Write(MinimalPdf.Build(
            mediaBox: "[0 0 200 300]",
            content: "BT /F1 12 Tf 20 100 Td (hello) Tj ET",
            fonts: [MinimalPdf.SimpleFont("ABCDEF+Whatever", embedded: true, fontFileKey)]));

        PdfAnalysisResult result = PdfAnalysis.Analyze(pdf.Path);

        result.Fonts.Count.ShouldBe(1);
        result.Fonts[0].Embedded.ShouldBeTrue();
        result.UnembeddedFontCount.ShouldBe(0);
    }

    /// <summary>A subset prefix is recognised by its shape, not by containing a plus sign.</summary>
    /// <remarks>
    /// ISO 32000-2 9.6.4 spells the prefix as exactly six upper-case letters and a plus. A plus
    /// sign is legal anywhere else in a font name, so testing for one alone mislabels
    /// <c>Foo+Bar</c> as a subset.
    /// </remarks>
    [Theory]
    [InlineData("ABCDEF+LiberationSerif", true)]
    // Seven upper-case letters, so the plus is at index 7 and this is not a subset prefix. The
    // case that discriminates position from mere presence: without it, testing name.Contains('+')
    // instead of name[6] == '+' passes every other case here.
    [InlineData("ABCDEFG+LiberationSerif", false)]
    [InlineData("Foo+Bar", false)]
    [InlineData("ABC123+LiberationSerif", false)]
    [InlineData("LiberationSerif", false)]
    public void ASubsetPrefixIsSixUpperCaseLettersAndAPlus(string baseFont, bool expected)
    {
        using TempPdf pdf = TempPdf.Write(MinimalPdf.Build(
            mediaBox: "[0 0 200 300]",
            content: "BT /F1 12 Tf 20 100 Td (hello) Tj ET",
            fonts: [MinimalPdf.SimpleFont(baseFont, embedded: true)]));

        PdfAnalysis.Analyze(pdf.Path).Fonts.ShouldHaveSingleItem().Subset.ShouldBe(expected);
    }

    /// <summary>A face reachable only through a form XObject is still found.</summary>
    /// <remarks>
    /// <c>pdffonts</c> recurses into XObject and pattern resources, so a reader that only looked at
    /// the page's own <c>/Font</c> would under-report against it — and would miss precisely the
    /// unembedded face that hides inside an imported drawing.
    /// </remarks>
    [Fact]
    public void AFaceReachableOnlyThroughAFormXObjectIsFound()
    {
        using TempPdf pdf = TempPdf.Write(MinimalPdf.BuildWithNestedForm(fanOut: 1, selfReferential: false));

        PdfAnalysisResult result = PdfAnalysis.Analyze(pdf.Path);

        result.Fonts.Count.ShouldBe(1);
        result.Fonts[0].Name.ShouldBe("Helvetica");
    }

    /// <summary>A page inherits its resources from the page tree, and its fonts with them.</summary>
    /// <remarks>
    /// <c>/Resources</c> is inheritable (ISO 32000-2 7.7.3.4), and a producer that shares one
    /// dictionary across every page routinely puts it on the <c>/Pages</c> node and nowhere else. A
    /// reader that only looks at the page dictionary then reports <em>zero fonts for the whole
    /// document</em> — a failure with no partial form, which is exactly the kind that reads as a
    /// corpus-wide result rather than as a bug. Not covered until reintroduction showed that
    /// replacing the parent walk with <c>null</c> broke nothing.
    /// </remarks>
    [Fact]
    public void APageInheritsItsResourcesFromThePageTree()
    {
        using TempPdf pdf = TempPdf.Write(MinimalPdf.BuildWithInheritedResources());

        PdfAnalysisResult result = PdfAnalysis.Analyze(pdf.Path);

        result.Error.ShouldBeNull();
        result.Fonts.Count.ShouldBe(1);
        result.Fonts[0].Name.ShouldBe("Helvetica");
        result.Fonts[0].Embedded.ShouldBeFalse();
    }

    /// <summary>A shared resource tree is walked once, not once per path through it.</summary>
    /// <remarks>
    /// <para>
    /// <b>This one is about termination, not about a number.</b> Resource dictionaries are shared,
    /// so descending without remembering costs O(fan-out ^ depth): the first version of this reader
    /// hung on 17 of the corpus's 534 documents — <c>1-secretariat__ppt.pdf</c> is 10 pages and 2402
    /// glyphs, reads in 0.7 s with the font walk removed, and had not finished the walk after 570 s.
    /// The depth cap alone does not save it; it only bounds the exponent.
    /// </para>
    /// <para>
    /// The timeout is the assertion. Eight mutually-referencing forms at the reader's depth cap is
    /// on the order of 8^8 descents without the guard, so this fails on a clock rather than on a
    /// value if the guard is removed — which is unusual and deliberate, because there is no wrong
    /// answer to assert against, only an answer that never arrives.
    /// </para>
    /// </remarks>
    [Fact(Timeout = 30000)]
    public void ASharedResourceTreeIsWalkedOnceRatherThanOncePerPath()
    {
        using TempPdf pdf = TempPdf.Write(MinimalPdf.BuildWithNestedForm(fanOut: 8, selfReferential: true));

        PdfAnalysisResult result = PdfAnalysis.Analyze(pdf.Path);

        result.Error.ShouldBeNull();
        result.Fonts.Count.ShouldBe(1, "one font dictionary, however many paths reach it");
    }

    /// <summary>Everything our own PDF writer emits carries its font program.</summary>
    /// <remarks>
    /// The same claim <c>PdfFontEmbeddingTests</c> makes, asked through a different reader. That is
    /// the point: a shared parser that mis-answers "is this embedded" would let both suites agree
    /// with each other and with nothing else.
    /// </remarks>
    [Theory]
    [InlineData("word-features.docx")]
    [InlineData("deck-features.pptx")]
    [InlineData("sheet-features.ods")]
    public void EveryFaceOurOwnWriterEmitsIsReportedEmbedded(string fileName)
    {
        using TempPdf pdf = TempPdf.Write(Render(fileName));

        PdfAnalysisResult result = PdfAnalysis.Analyze(pdf.Path);

        result.Fonts.ShouldNotBeEmpty($"{fileName} rendered no text, so this proves nothing");
        result.UnembeddedFontCount.ShouldBe(0);
    }

    // ------------------------------------------------------------------------------ words

    /// <summary>
    /// A document with a known number of words reports that number.
    /// </summary>
    /// <remarks>
    /// The known-answer control the round is built on, authored so the answer is true by
    /// construction rather than by agreement with another tool: 250 plain words, then ten list
    /// lines of a literal bullet and two words each, then five punctuation-only tokens. That is 285
    /// raw tokens, 270 of them words under the gate's metric, 10 bullets and 5 punctuation. Poppler
    /// independently reports 285 and 270 on the same rendering.
    /// </remarks>
    [Fact]
    public void AnAuthoredDocumentsWordsAreCountedExactly()
    {
        string words = string.Join(' ', Enumerable.Range(1, 250).Select(i => $"alpha{i:D3}"));
        string bullets = string.Join(' ', Enumerable.Range(1, 10).Select(i => $"• beta{i:D2} gamma{i:D2}"));
        const string Punctuation = "-- ... ( ) &amp;&amp;";

        using TempPdf pdf = TempPdf.Write(RenderFlatOdt(words, bullets, Punctuation));

        PdfWordCounts counts = PdfAnalysis.Analyze(pdf.Path).Words;

        counts.Raw.ShouldBe(285);
        counts.Alphanumeric.ShouldBe(270);
        counts.Bullet.ShouldBe(10);
        counts.Punctuation.ShouldBe(5);
        counts.PrivateUse.ShouldBe(0);
    }

    /// <summary>
    /// A word is a token carrying a Unicode letter or digit — the gate's definition, not ours.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The gate expresses it in Python as <c>any(c.isalnum() for c in w)</c>, and
    /// <c>str.isalnum()</c> is true for every numeric category, not only the decimal digits. So
    /// <c>½</c> (No), <c>Ⅻ</c> (Nl) and a superscript <c>²</c> (No) are words. Implementing the
    /// predicate as <see cref="char.IsLetterOrDigit(char)"/> stops at Nd and drops all three, which
    /// is a silent one-way disagreement with the metric this tool exists to serve — so it is
    /// asserted here rather than left to a comment.
    /// </para>
    /// <para>
    /// The bullet and the dash pin the other side: neither is a word, and the two land in different
    /// classes, which is what makes a difference legible instead of merely large.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheWordDefinitionFollowsTheGateAcrossEveryNumericCategory()
    {
        using TempPdf pdf = TempPdf.Write(RenderFlatOdt(
            "half ½ twelve Ⅻ superscript ² bullet • dash --"));

        PdfWordCounts counts = PdfAnalysis.Analyze(pdf.Path).Words;

        counts.Raw.ShouldBe(10);
        counts.Alphanumeric.ShouldBe(8, "half ½ twelve Ⅻ superscript ² bullet dash");
        counts.Bullet.ShouldBe(1);
        counts.Punctuation.ShouldBe(1);
    }

    /// <summary>Rotated text is grouped into words rather than into one word per glyph.</summary>
    /// <remarks>
    /// <para>
    /// The reason the default grouping is the orientation-aware one, stated as a test rather than
    /// only as a corpus figure. Every glyph of a 90°-rotated run sits at a different height, so a
    /// grouper that decides "same line" by comparing heights starts a new word at every character —
    /// and a chart's rotated axis labels are the commonest place this happens. On
    /// <c>Keywords_Mapping_Graphs_and_Charts.xlsx</c> that is 8225 words against poppler's 4519.
    /// </para>
    /// <para>
    /// Both groupings are asserted, because the point is the difference between them: the simpler
    /// one is not merely worse here, it returns one word per glyph.
    /// </para>
    /// </remarks>
    [Fact]
    public void RotatedTextIsGroupedIntoWordsUnderTheDefaultGrouping()
    {
        // A text matrix of [0 1 -1 0] turns the baseline through 90°.
        using TempPdf pdf = TempPdf.Write(MinimalPdf.Build(
            mediaBox: "[0 0 200 300]",
            content: "BT /F1 12 Tf 0 1 -1 0 100 40 Tm (rotated words here) Tj ET",
            fonts: [MinimalPdf.SimpleFont("Helvetica", embedded: false)]));

        PdfAnalysis.Analyze(pdf.Path).Words.Raw.ShouldBe(3);

        PdfAnalysis.Analyze(pdf.Path, grouping: WordGrouping.Simple).Words.Raw
            .ShouldBe("rotatedwordshere".Length, "the simpler grouper has no concept of orientation");
    }

    /// <summary>A Symbol or Wingdings glyph lands in its own class, not among the words.</summary>
    /// <remarks>
    /// A producer subsetting Symbol or Wingdings maps its glyphs into the Private Use Area, so a
    /// list bullet arrives as U+F0B7 rather than as U+2022 and no amount of looking for bullet code
    /// points finds it. It is reported separately from <see cref="PdfWordCounts.Bullet"/> because
    /// the two have different causes: a previous round's entire PUA census was fitted against a
    /// difference that turned out to be the extractor, and keeping the classes apart is what lets
    /// the next one tell those cases apart. The corpus carries 8622 such tokens across 534
    /// documents; nothing tested the class until reintroduction showed it could be deleted for free.
    /// </remarks>
    [Fact]
    public void APrivateUseSymbolIsCountedApartFromWordsAndFromBullets()
    {
        using TempPdf pdf = TempPdf.Write(RenderFlatOdt("alpha \uF0B7 beta \uF0B7 gamma"));

        PdfWordCounts counts = PdfAnalysis.Analyze(pdf.Path).Words;

        counts.Raw.ShouldBe(5);
        counts.Alphanumeric.ShouldBe(3, "alpha beta gamma");
        counts.PrivateUse.ShouldBe(2);
        counts.Bullet.ShouldBe(0);
    }

    /// <summary>The token classes add up to the raw count, on every document.</summary>
    /// <remarks>
    /// The classes exist so a comparison can show what a difference is made of. A decomposition
    /// that does not add up is how a term goes missing without anyone noticing, and it is the exact
    /// shape of the defect that produced 142 phantom <c>box</c> notes in an earlier round.
    /// </remarks>
    [Theory]
    [InlineData("word-features.docx")]
    [InlineData("deck-features.pptx")]
    [InlineData("sheet-features.ods")]
    [InlineData("text-features.odt")]
    public void TheTokenClassesPartitionTheRawCount(string fileName)
    {
        using TempPdf pdf = TempPdf.Write(Render(fileName));

        PdfWordCounts c = PdfAnalysis.Analyze(pdf.Path).Words;

        c.Raw.ShouldBeGreaterThan(0);
        (c.Alphanumeric + c.Bullet + c.PrivateUse + c.Punctuation).ShouldBe(c.Raw);
    }

    /// <summary>Reading the same file twice gives the same answer.</summary>
    /// <remarks>
    /// Not a truism. The word grouper fans out over orientation buckets and merges them under a
    /// lock when left to its defaults, which makes the order of its output — and therefore the
    /// extracted text — depend on thread scheduling. This reader pins it to one thread; without
    /// that pin the counts still match and the text does not.
    /// </remarks>
    [Fact]
    public void ReadingTheSameFileTwiceGivesTheSameTextAndTheSameCounts()
    {
        using TempPdf pdf = TempPdf.Write(Render("word-features.docx"));

        PdfAnalysisResult first = PdfAnalysis.Analyze(pdf.Path, includeText: true);
        PdfAnalysisResult second = PdfAnalysis.Analyze(pdf.Path, includeText: true);

        second.Words.ShouldBe(first.Words);
        second.Text.ShouldBe(first.Text);
        second.PageCount.ShouldBe(first.PageCount);
    }

    /// <summary>The raw policy counts every token; the alphanumeric policy counts fewer.</summary>
    /// <remarks>
    /// The two are a named option rather than a constant because what a word is, is a separate
    /// decision from how to measure it — and the project spent a round unable to tell a renderer
    /// change from a poppler change partly because the two were welded together.
    /// </remarks>
    [Fact]
    public void TheWordCountPolicySelectsBetweenTheTwoReportedTotals()
    {
        using TempPdf pdf = TempPdf.Write(RenderFlatOdt("one two • three --"));

        PdfAnalysisResult result = PdfAnalysis.Analyze(pdf.Path);

        result.WordCount(WordCountPolicy.Raw).ShouldBe(result.Words.Raw);
        result.WordCount(WordCountPolicy.Alphanumeric).ShouldBe(result.Words.Alphanumeric);
        result.Words.Alphanumeric.ShouldBeLessThan(result.Words.Raw);
    }

    // ----------------------------------------------------------------------------- errors

    /// <summary>An unreadable file is reported, not thrown.</summary>
    /// <remarks>
    /// A corpus sweep has to be able to record a broken document as broken and carry on; an
    /// exception here stops a 534-document run on its first bad file.
    /// </remarks>
    [Fact]
    public void AFileThatIsNotAPdfIsReportedRatherThanThrown()
    {
        using TempPdf pdf = TempPdf.Write("this is not a PDF at all"u8.ToArray());

        PdfAnalysisResult result = PdfAnalysis.Analyze(pdf.Path);

        result.Error.ShouldNotBeNull();
        result.PageCount.ShouldBe(0);
        result.Words.Raw.ShouldBe(0);
    }

    // --------------------------------------------------------------------------- plumbing

    private static byte[] Render(string fileName)
    {
        using IDocument document = OfficeDocument.Open(Corpus.Require(fileName));
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using MemoryStream output = new();
        new PdfRenderer(Reproducible).Render(pages, output);
        return output.ToArray();
    }

    /// <summary>Renders paragraphs of literal text through the real pipeline.</summary>
    /// <remarks>
    /// A flat ODT because it is the shortest input this project can render that lets a test state
    /// its own text. The text goes through layout, shaping and the PDF writer, so what comes back
    /// is a real rendering and not a hand-placed string.
    /// </remarks>
    private static byte[] RenderFlatOdt(params string[] paragraphs)
    {
        StringBuilder body = new();
        foreach (string paragraph in paragraphs) body.Append("<text:p>").Append(paragraph).Append("</text:p>");

        string flat =
            """<?xml version="1.0" encoding="UTF-8"?><office:document """
            + """xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" """
            + """xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" """
            + """office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">"""
            + "<office:body><office:text>" + body + "</office:text></office:body></office:document>";

        string path = Path.Combine(Path.GetTempPath(), $"xray-office-analysis-{Guid.NewGuid():N}.fodt");
        try
        {
            File.WriteAllText(path, flat, Encoding.UTF8);

            using IDocument document = OfficeDocument.Open(path);
            IPageSequence pages = ((IPaginatedDocument)document).Layout();

            using MemoryStream output = new();
            new PdfRenderer(Reproducible).Render(pages, output);
            return output.ToArray();
        }
        finally
        {
            File.Delete(path);
        }
    }

    /// <summary>A PDF on disk for the duration of a test, because the reader takes a path.</summary>
    private sealed class TempPdf : IDisposable
    {
        private TempPdf(string path) => Path = path;

        public string Path { get; }

        public static TempPdf Write(byte[] bytes)
        {
            string path = System.IO.Path.Combine(
                System.IO.Path.GetTempPath(), $"xray-office-analysis-{Guid.NewGuid():N}.pdf");
            File.WriteAllBytes(path, bytes);
            return new TempPdf(path);
        }

        public void Dispose()
        {
            try { File.Delete(Path); }
            catch (IOException) { /* a leaked temp file is not worth failing a test over */ }
        }
    }

    /// <summary>
    /// Builds PDFs small enough to state a single fact about.
    /// </summary>
    /// <remarks>
    /// Hand-assembled rather than produced by our own writer, because the cases that matter here
    /// are ones our writer will not produce: a glyph placed off the page, a face named with no font
    /// program, a resource tree that refers back to itself. Offsets and the cross-reference table
    /// are computed, so these are valid files rather than files a lenient parser tolerates.
    /// </remarks>
    private static class MinimalPdf
    {
        /// <summary>A simple Type 1 font dictionary, with or without a font program.</summary>
        /// <remarks>
        /// The embedded variant's font-file entry holds a stub rather than a real font program:
        /// what is under test is whether the reader reports the presence of the key the way
        /// <c>pdffonts</c>'s <c>emb</c> column does, and a valid TrueType would test the font
        /// parser instead. The key is a parameter because there are three of them and a reader that
        /// knows only <c>/FontFile2</c> reports every CFF face in the corpus as unembedded.
        /// </remarks>
        public static string SimpleFont(string baseFont, bool embedded, string fontFileKey = "FontFile2")
            => embedded
                ? $"<< /Type /Font /Subtype /TrueType /BaseFont /{baseFont} /FirstChar 32 "
                  + "/FontDescriptor << /Type /FontDescriptor /FontName /" + baseFont
                  + $" /Flags 32 /{fontFileKey} 999 0 R >> >>"
                : $"<< /Type /Font /Subtype /Type1 /BaseFont /{baseFont} >>";

        public static byte[] Build(string mediaBox, string content, string[] fonts)
        {
            List<string> objects = [];
            string names = string.Join(' ', fonts.Select((_, i) => $"/F{i + 1} {5 + i} 0 R"));

            objects.Add("<< /Type /Catalog /Pages 2 0 R >>");
            objects.Add("<< /Type /Pages /Kids [3 0 R] /Count 1 >>");
            objects.Add($"<< /Type /Page /Parent 2 0 R /MediaBox {mediaBox} "
                        + $"/Resources << /Font << {names} >> >> /Contents 4 0 R >>");
            objects.Add(Stream(content));
            objects.AddRange(fonts);

            return Assemble(objects);
        }

        /// <summary>
        /// A page whose only font sits inside a form XObject, optionally one that reaches itself.
        /// </summary>
        /// <param name="fanOut">How many form entries each resource dictionary lists.</param>
        /// <param name="selfReferential">
        /// When true every form's own resources list all the forms again, so the number of distinct
        /// paths through the tree grows as fan-out to the power of the walker's depth cap while the
        /// number of distinct objects stays at <paramref name="fanOut"/>. That gap is the test.
        /// </param>
        public static byte[] BuildWithNestedForm(int fanOut, bool selfReferential)
        {
            // 1 catalog, 2 pages, 3 page, 4 page content, 5 font, 6.. the forms.
            const int FirstForm = 6;
            string forms = string.Join(' ',
                Enumerable.Range(0, fanOut).Select(i => $"/X{i} {FirstForm + i} 0 R"));

            List<string> objects =
            [
                "<< /Type /Catalog /Pages 2 0 R >>",
                "<< /Type /Pages /Kids [3 0 R] /Count 1 >>",
                $"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 300] "
                + $"/Resources << /XObject << {forms} >> >> /Contents 4 0 R >>",
                Stream("/X0 Do"),
                SimpleFont("Helvetica", embedded: false),
            ];

            string formResources = selfReferential
                ? $"<< /Font << /F1 5 0 R >> /XObject << {forms} >> >>"
                : "<< /Font << /F1 5 0 R >> >>";

            for (int i = 0; i < fanOut; i++)
            {
                objects.Add(Stream(
                    "BT /F1 12 Tf 20 100 Td (inside a form) Tj ET",
                    $"/Type /XObject /Subtype /Form /BBox [0 0 200 300] /Resources {formResources}"));
            }

            return Assemble(objects);
        }

        /// <summary>A page whose <c>/Resources</c> is on its parent rather than on itself.</summary>
        public static byte[] BuildWithInheritedResources()
            =>
            [
                .. Assemble(
                [
                    "<< /Type /Catalog /Pages 2 0 R >>",
                    "<< /Type /Pages /Kids [3 0 R] /Count 1 /Resources << /Font << /F1 5 0 R >> >> >>",
                    "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 300] /Contents 4 0 R >>",
                    Stream("BT /F1 12 Tf 20 100 Td (hello) Tj ET"),
                    SimpleFont("Helvetica", embedded: false),
                ]),
            ];

        private static string Stream(string content, string? extra = null)
            => $"<< {extra} /Length {content.Length} >>\nstream\n{content}\nendstream";

        private static byte[] Assemble(List<string> objects)
        {
            StringBuilder pdf = new("%PDF-1.7\n");
            List<int> offsets = [];

            for (int i = 0; i < objects.Count; i++)
            {
                offsets.Add(pdf.Length);
                pdf.Append(CultureInfo.InvariantCulture, $"{i + 1} 0 obj\n{objects[i]}\nendobj\n");
            }

            int startxref = pdf.Length;
            pdf.Append(CultureInfo.InvariantCulture, $"xref\n0 {objects.Count + 1}\n");
            pdf.Append("0000000000 65535 f \n");
            foreach (int offset in offsets) pdf.Append(CultureInfo.InvariantCulture, $"{offset:D10} 00000 n \n");
            pdf.Append(CultureInfo.InvariantCulture,
                $"trailer\n<< /Size {objects.Count + 1} /Root 1 0 R >>\nstartxref\n{startxref}\n%%EOF");

            // Latin-1: every byte written above is ASCII, and a PDF's structure is bytes rather than
            // text, so a multi-byte encoding here would put the computed offsets out by the
            // difference and produce a file only a lenient parser could read.
            return Encoding.Latin1.GetBytes(pdf.ToString());
        }
    }
}

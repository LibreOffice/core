using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A run that leans only because its face has no italic keeps its run.
/// </summary>
/// <remarks>
/// <para>
/// Every reader folds a paragraph whose formatting does not <em>vary</em> into a single run, and every
/// property that has to reach the page is on that predicate with a sentence saying why — highlight,
/// underline, strike-through, case map. Slant was not, and for nearly every family it did not need to
/// be: an italic run of <c>Arial</c> resolves to <c>LiberationSans-Italic</c>, a different
/// <c>OpenTypeFace</c>, so <c>face != paragraphFace</c> already fires.
/// </para>
/// <para>
/// The families with <b>no italic installed at all</b> are exactly the fallback faces — DejaVu Sans and
/// DejaVu Serif ship Book and Bold and nothing else — so an italic run that falls back resolves to the
/// <em>same</em> face as its upright neighbour, passes every other test, and is drawn upright. And the
/// same fold read the other way drew whole paragraphs of upright text <em>leaning</em>, because a
/// paragraph mark that is italic donates its own font to every run folded into it.
/// </para>
/// <para>
/// Measured over the 337-document words corpus, <c>probes/words-r56/</c>: the reference shears 154 501
/// glyphs, we sheared 158 673 — 6 819 short on 38 documents and 10 991 long on eight at the same time.
/// <c>644730BRI0mna000BOX361539B00public0.doc</c> alone leaned 6 643 glyphs against the reference's
/// 2 171, an entire lead paragraph of upright prose set slanted.
/// </para>
/// <para>
/// The tests drive the <b>readers</b> rather than <c>PageRun.LeansDifferently</c>, and three of the four
/// arms have one here: reverting the predicate at any of the three fails a test in this file. The WW8
/// arm has no authored route — a <c>.doc</c> cannot be built in a test — and is covered only by the
/// corpus measurement.
/// </para>
/// </remarks>
public sealed class SyntheticObliqueRunTests
{
    /// <summary>A family with no italic anywhere, and no generic fontconfig files it under.</summary>
    private const string NoItalic = "Zqxwv Nonesuch";

    /// <summary>The premise: the two runs resolve to one and the same face.</summary>
    /// <remarks>
    /// Without this the rest of the file would pass for the wrong reason — every other test here would
    /// be satisfied by the ordinary <c>face != paragraphFace</c> clause that has always been there.
    /// </remarks>
    [Fact]
    public void TheItalicRunResolvesToTheVeryFaceItsNeighbourDoes()
    {
        PageParagraph paragraph = Docx(NoItalic, "<w:i/>");

        paragraph.Runs.Count.ShouldBe(2);
        paragraph.Runs[1].Face.ShouldBeSameAs(paragraph.Runs[0].Face);
        paragraph.Runs[1].Face.IsItalic.ShouldBeFalse();
    }

    [Fact]
    public void AnItalicRunInAnUprightParagraphKeepsItsLean()
    {
        PageParagraph paragraph = Docx(NoItalic, "<w:i/>");

        paragraph.HasRuns.ShouldBeTrue(
            "the run leans and the paragraph does not, so the fold would draw it upright");
        Oblique(paragraph.Runs[0]).ShouldBeFalse();
        Oblique(paragraph.Runs[1]).ShouldBeTrue();
    }

    /// <summary>The other direction, which is the larger half of the corpus defect.</summary>
    /// <remarks>
    /// An italic paragraph mark with an upright run in it. The fold takes the <em>paragraph's</em> font
    /// for every run, so before this the upright run was drawn leaning — which is what
    /// <c>644730BRI…</c>'s 4 472 surplus sheared glyphs were.
    /// </remarks>
    [Fact]
    public void AnUprightRunInALeaningParagraphDoesNotLean()
    {
        PageParagraph paragraph = Docx(NoItalic, "<w:i w:val=\"0\"/>", paragraphItalic: true);

        paragraph.HasRuns.ShouldBeTrue();
        Oblique(paragraph.Runs[0]).ShouldBeTrue();
        Oblique(paragraph.Runs[1]).ShouldBeFalse();
    }

    /// <summary>A family whose italic is installed is untouched, because it never leans.</summary>
    [Theory]
    [InlineData("Arial")]
    [InlineData("Courier New")]
    public void AnInstalledItalicIsNotASyntheticOne(string family)
    {
        PageParagraph paragraph = Docx(family, "<w:i/>");

        paragraph.Runs.Count.ShouldBe(2);
        paragraph.Runs[1].Face.ShouldNotBeSameAs(paragraph.Runs[0].Face);
        paragraph.Runs[1].Face.IsItalic.ShouldBeTrue();
        Oblique(paragraph.Runs[1]).ShouldBeFalse();
    }

    /// <summary>The predicate did not become "always vary".</summary>
    /// <remarks>
    /// <c>w:iCs</c> is complex-script italic and does not lean Latin text on 26.2.4.2 — measured,
    /// <c>probes/words-r56/oblique-uniform.py</c> case <c>nonesuch/iCs</c>, nought sheared glyphs on
    /// both sides. So this paragraph is uniform and must still take the shortcut. Without a control
    /// like this, a predicate that answered true unconditionally would pass every other test here and
    /// split every paragraph in the corpus.
    /// </remarks>
    [Fact]
    public void AParagraphThatOnlyLooksLikeItVariesStillTakesTheShortcut()
    {
        Docx(NoItalic, "<w:iCs/>").HasRuns.ShouldBeFalse();
        Docx(NoItalic, string.Empty).HasRuns.ShouldBeFalse();
    }

    /// <summary>
    /// Keeping the run costs no width, which is the whole argument for it being safe.
    /// </summary>
    /// <remarks>
    /// A run boundary breaks the shaping context, and <c>PageContent.Coalesce</c> is what puts it back:
    /// it rejoins adjacent <c>FormattedRun</c>s that are equal, and <c>FormattedRun</c> does not carry
    /// the font reference. So a paragraph split only by this measures exactly what it measured before —
    /// and if it did not, every affected document would gain a fraction of a line and eventually a page.
    /// The corpus sweep bears this out: nought page counts changed over 337 documents.
    /// </remarks>
    [Fact]
    public void TheKeptRunDoesNotChangeTheParagraphsWidth()
    {
        PageParagraph leaning = Docx(NoItalic, "<w:i/>");
        PageParagraph upright = Docx(NoItalic, string.Empty);

        leaning.Text.ShouldBe(upright.Text);
        leaning.Measure().WidthBetween(0, leaning.Text.Length)
            .ShouldBe(upright.Measure().WidthBetween(0, upright.Text.Length));
    }

    /// <summary>The RTF arm, which has no corpus witness of its own.</summary>
    [Fact]
    public void TheRtfReaderKeepsALeaningRunToo()
    {
        PageParagraph paragraph = Read(
            Encoding.ASCII.GetBytes(
                @"{\rtf1\ansi\deff0{\fonttbl{\f0\fnil " + NoItalic + @";}}"
                + @"\f0\fs24 plain \i slanted\i0\par}"),
            "oblique.rtf");

        paragraph.HasRuns.ShouldBeTrue();
        paragraph.Runs.Any(run => Oblique(run)).ShouldBeTrue();
        paragraph.Runs.Any(run => !Oblique(run)).ShouldBeTrue();
    }

    /// <summary>The WW8 arm, whose fixture had to be made rather than written.</summary>
    /// <remarks>
    /// <para>
    /// A <c>.doc</c> cannot be authored from a string, so <c>features/synthetic-oblique-run.doc</c> is
    /// LibreOffice's own Word 97 export of a flat ODF holding one paragraph of two runs in a family
    /// nothing resolves. That route matters: a <c>.docx</c> round trip bakes in Word's roman default,
    /// where an ODF <c>style:font-face</c> with no <c>style:font-family-generic</c> leaves the family
    /// at <c>FAMILY_DONTKNOW</c> and <c>wwFont::Write</c> puts <c>ff = 0</c> in the font table — so the
    /// reader really does resolve the fallback face here rather than a declared one.
    /// </para>
    /// <para>
    /// This is the arm with the largest measured effect on the corpus and it had no test at all:
    /// <c>644730BRI0mna000BOX361539B00public0.doc</c> leaned 6 643 glyphs against the reference's
    /// 2 171 before this change and 2 124 after.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheWw8ReaderKeepsALeaningRunToo()
    {
        PageParagraph paragraph = Read(
            File.ReadAllBytes(Corpus.Require("features/synthetic-oblique-run.doc")),
            "synthetic-oblique-run.doc");

        paragraph.HasRuns.ShouldBeTrue();
        paragraph.Runs.Any(run => Oblique(run)).ShouldBeTrue();
        paragraph.Runs.Any(run => !Oblique(run)).ShouldBeTrue();
    }

    /// <summary>The ODF arm, which has no corpus witness of its own either.</summary>
    /// <remarks>
    /// A flat ODF document, so the whole reader runs from a string. The words corpus holds no ODF text
    /// document at all — 0 of 337 — so without this the ODT site would ship on the argument alone.
    /// </remarks>
    [Fact]
    public void TheOdfReaderKeepsALeaningRunToo()
    {
        PageParagraph paragraph = Read(Encoding.UTF8.GetBytes(Fodt), "oblique.fodt");

        paragraph.HasRuns.ShouldBeTrue();
        paragraph.Runs.Any(run => Oblique(run)).ShouldBeTrue();
        paragraph.Runs.Any(run => !Oblique(run)).ShouldBeTrue();
    }

    private static bool Oblique(PageRun run) => run.Font?.SyntheticOblique ?? false;

    private const string Fodt = """
        <?xml version="1.0" encoding="UTF-8"?>
        <office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
                         xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
                         xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
                         xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
                         office:version="1.3"
                         office:mimetype="application/vnd.oasis.opendocument.text">
          <office:font-face-decls>
            <style:font-face style:name="Zqxwv Nonesuch" svg:font-family="Zqxwv Nonesuch"
                             xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"/>
          </office:font-face-decls>
          <office:automatic-styles>
            <style:style style:name="P1" style:family="paragraph">
              <style:text-properties style:font-name="Zqxwv Nonesuch" fo:font-size="12pt"/>
            </style:style>
            <style:style style:name="T1" style:family="text">
              <style:text-properties style:font-name="Zqxwv Nonesuch" fo:font-size="12pt"
                                     fo:font-style="italic"/>
            </style:style>
          </office:automatic-styles>
          <office:body><office:text>
            <text:p text:style-name="P1">plain <text:span text:style-name="T1">slanted</text:span></text:p>
          </office:text></office:body>
        </office:document>
        """;

    private static PageParagraph Docx(string family, string runProperties, bool paragraphItalic = false)
        => Read(Package(family, runProperties, paragraphItalic).ToArray(), "oblique.docx");

    private static PageParagraph Read(byte[] bytes, string fileName)
    {
        using MemoryStream stream = new(bytes);
        using DocumentSource source = DocumentSource.FromStream(stream, fileName);
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageParagraph>().First(block => block.Text.Length > 0);
    }

    private static MemoryStream Package(string family, string runProperties, bool paragraphItalic)
    {
        const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
        const string PkgR = "http://schemas.openxmlformats.org/package/2006/relationships";
        const string R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

        string mark = paragraphItalic ? "<w:pPr><w:rPr><w:i/></w:rPr></w:pPr>" : string.Empty;
        string firstRun = paragraphItalic ? "<w:rPr><w:i/></w:rPr>" : string.Empty;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="{W}"><w:body>
              <w:p>{mark}
                <w:r>{firstRun}<w:t xml:space="preserve">plain </w:t></w:r>
                <w:r><w:rPr>{runProperties}</w:rPr><w:t>slanted</w:t></w:r>
              </w:p>
              <w:sectPr><w:pgSz w:w="11906" w:h="16838"/></w:sectPr>
            </w:body></w:document>
            """;

        string styles = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>
              <w:rFonts w:ascii="{family}" w:hAnsi="{family}"/><w:sz w:val="24"/>
            </w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults></w:styles>
            """;

        string types = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/styles.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
            </Types>
            """;

        string rootRelationships = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="{PkgR}">
              <Relationship Id="rId1" Target="word/document.xml" Type="{R}/officeDocument"/>
            </Relationships>
            """;

        string documentRelationships = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="{PkgR}">
              <Relationship Id="rId8" Target="styles.xml" Type="{R}/styles"/>
            </Relationships>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", types);
            Write(archive, "_rels/.rels", rootRelationships);
            Write(archive, "word/_rels/document.xml.rels", documentRelationships);
            Write(archive, "word/document.xml", document);
            Write(archive, "word/styles.xml", styles);
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

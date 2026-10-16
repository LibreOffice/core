using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Where a <em>recomputed</em> field's value takes its character properties: from the field, not
/// from the result it replaces.
/// </summary>
/// <remarks>
/// <para>
/// A cached field result is a statement about a value that no longer applies, and its <c>w:rPr</c>
/// is part of that statement. 26.2.4.2 discards the whole of it — writerfilter builds a
/// <c>com.sun.star.text.TextField</c> and deletes the cached result text with it — so the value is
/// drawn in the properties the field's <em>instruction run</em> carries, falling back to the
/// paragraph style. Measured on seven one-attribute arms in a footer, where both engines recompute:
/// a cached result stating 20 pt red comes back at the style's 10 pt in five of them, and the two
/// whose <c>w:instrText</c> run states the 20 pt come back at 20 — <strong>including the one whose
/// cached result states 10</strong>, which inverts the sign and is what says the rule is the
/// instruction's rather than "the smaller of the two". <c>probes/fieldrpr-r168/results.md</c>.
/// </para>
/// <para>
/// <strong>Word honours <c>\* MERGEFORMAT</c> and the reference does not</strong>, which is the
/// whole of why this file asserts what it does. The switch means <em>preserve the formatting of the
/// previous result</em>, so a field carrying it keeps the cached run's properties in Word and this
/// tree follows Word by default; <c>XRAY_OFFICE_LIBREOFFICE_QUIRKS</c> applies the rule to every
/// field, which is 26.2.4.2's own reading. That arm is measured in the probe rather than here,
/// because the switch is an environment variable and a test that set it would reach every other
/// test running beside it. <c>TODO.word-parity.md</c> carries the entry and what it costs.
/// </para>
/// <para>
/// <strong>Only a field this tree actually recomputes.</strong> For anything else the cached text
/// <em>is</em> what gets drawn, so its own formatting is the right formatting — the reference
/// recomputes far more fields than this tree does, and a rule about a value's formatting cannot be
/// borrowed for a value that was never recomputed.
/// </para>
/// </remarks>
public sealed class FieldResultFormatTests
{
    /// <summary>The paragraph style's size, which a field falling back to it comes out at.</summary>
    private static readonly Length Style = Length.FromPoints(10);

    /// <summary>What every arm's cached result states, so that keeping it is visible.</summary>
    private static readonly Length Cached = Length.FromPoints(20);

    /// <summary>
    /// A compact field with no <c>\* MERGEFORMAT</c> is drawn in the paragraph's own style.
    /// </summary>
    /// <remarks>
    /// A <c>w:fldSimple</c> states no instruction run at all, so there is nothing between the value
    /// and the style — and its own <c>w:rPr</c> child does not count, which `SIMPLERPR` in the probe
    /// is the control for.
    /// </remarks>
    [Fact]
    public void ACompactFieldDropsItsCachedResultsFormatting()
        => SizeOf("nomerge").ShouldBe(Style);

    /// <summary>And takes its instruction run's where the field is written the long way.</summary>
    [Fact]
    public void AComplexFieldTakesItsInstructionRunsFormatting()
        => SizeOf("nomerge-complex").ShouldBe(Cached);

    /// <summary>
    /// A field carrying <c>\* MERGEFORMAT</c> keeps what its cached result states, which is what the
    /// switch asks for and what Word does.
    /// </summary>
    [Fact]
    public void AMergeFormatFieldKeepsItsCachedResultsFormatting()
        => SizeOf("merge").ShouldBe(Cached);

    /// <summary>The control: an ordinary run stating the same properties is unaffected.</summary>
    [Fact]
    public void APlainRunIsUntouched() => SizeOf("plain").ShouldBe(Cached);

    /// <summary>
    /// A page field with no <c>separate</c> marker has no cached result, and is drawn anyway.
    /// </summary>
    /// <remarks>
    /// Its value is written by <see cref="PageFields"/> over a span of the paragraph's text, so with
    /// no result there is no span and the field drew <em>nothing</em> — where 26.2.4.2 computes and
    /// draws it. A result is therefore made: one character, in the field's own properties, which
    /// pagination replaces before the flow is laid out so that its width never reaches the page.
    /// Censused at 10 such <c>PAGE</c> fields in 8 corpus documents, of which nine are Word's
    /// "page number in a frame" template leftover that neither engine draws — so the measured reach
    /// is nil and the fixture is what says the rule is right.
    /// </remarks>
    [Fact]
    public void ASeparatorLessPageFieldIsDrawnAtAll()
    {
        LaidOutPage page = Paginate()[0];
        page.Footer.ShouldNotBeNull();

        PageParagraph arm = page.Footer!.Blocks.OfType<PageParagraph>()
            .First(p => p.Text.StartsWith("noseparator", StringComparison.Ordinal));

        arm.Text.ShouldBe("noseparator1");
        SizeOf("noseparator").ShouldBe(Cached);
    }

    /// <summary>The size the run after the named label is drawn at.</summary>
    private static Length SizeOf(string label)
    {
        LaidOutPage page = Paginate()[0];
        page.Footer.ShouldNotBeNull();

        foreach (PageParagraph paragraph in page.Footer!.Blocks.OfType<PageParagraph>())
        {
            if (!paragraph.Text.StartsWith(label, StringComparison.Ordinal)) continue;

            // The label is the first run and the value is whatever follows it — or, where the two
            // agree, no run at all: the reader drops the run list for a paragraph whose runs all
            // match its own format, and a value that fell back to the style is exactly that case.
            // `PageRun` is a value type, so the absence has to be tested rather than pattern-matched.
            List<PageRun> after = [.. paragraph.Runs.Where(run => run.Start >= label.Length)];
            return after.Count > 0 ? after[0].EmSize : paragraph.EmSize;
        }

        throw new InvalidOperationException($"the fixture states no arm '{label}'");
    }

    private static IReadOnlyList<LaidOutPage> Paginate()
    {
        MemoryStream bytes = BuildPackage();
        using DocumentSource source = DocumentSource.FromStream(bytes, "report-2026.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages;
    }

    private static MemoryStream BuildPackage()
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/footer1.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>
              <Override PartName="/word/styles.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
              <Override PartName="/word/settings.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        const string DocumentRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="settings.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
              <Relationship Id="rId2" Target="footer1.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer"/>
              <Relationship Id="rId3" Target="styles.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
            </Relationships>
            """;

        // 10 pt, so that a value falling back to the style is distinguishable both from the 20 pt
        // its cached result states and from the document default a missing styles part would give.
        const string Styles = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
                <w:name w:val="Normal"/><w:rPr><w:sz w:val="20"/><w:szCs w:val="20"/></w:rPr>
              </w:style>
            </w:styles>
            """;

        // An empty settings part, because a DOCX without one takes a different set of OOXML
        // compatibility defaults and answers a different question — see `office-corpus`.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        // `FILENAME` because it is a field this walk recomputes and whose value is knowable from the
        // stream's own name, so no rasteriser and no pagination is needed to see it.
        const string Footer = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:p><w:r><w:t>plain</w:t></w:r>
                <w:r><w:rPr><w:sz w:val="40"/></w:rPr><w:t>XXXX</w:t></w:r></w:p>
              <w:p><w:r><w:t>merge</w:t></w:r>
                <w:fldSimple w:instr=" FILENAME   \* MERGEFORMAT ">
                  <w:r><w:rPr><w:sz w:val="40"/></w:rPr><w:t>cached.doc</w:t></w:r>
                </w:fldSimple></w:p>
              <w:p><w:r><w:t>nomerge</w:t></w:r>
                <w:fldSimple w:instr=" FILENAME ">
                  <w:r><w:rPr><w:sz w:val="40"/></w:rPr><w:t>cached.doc</w:t></w:r>
                </w:fldSimple></w:p>
              <w:p><w:r><w:t>noseparator</w:t></w:r>
                <w:r><w:fldChar w:fldCharType="begin"/></w:r>
                <w:r><w:rPr><w:sz w:val="40"/></w:rPr>
                  <w:instrText xml:space="preserve"> PAGE </w:instrText></w:r>
                <w:r><w:fldChar w:fldCharType="end"/></w:r></w:p>
              <w:p><w:r><w:t>nomerge-complex</w:t></w:r>
                <w:r><w:rPr><w:sz w:val="40"/></w:rPr><w:fldChar w:fldCharType="begin"/></w:r>
                <w:r><w:rPr><w:sz w:val="40"/></w:rPr>
                  <w:instrText xml:space="preserve"> FILENAME </w:instrText></w:r>
                <w:r><w:fldChar w:fldCharType="separate"/></w:r>
                <w:r><w:rPr><w:sz w:val="16"/></w:rPr><w:t>cached.doc</w:t></w:r>
                <w:r><w:fldChar w:fldCharType="end"/></w:r></w:p>
            </w:ftr>
            """;

        const string Document = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
                        xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
              <w:body>
                <w:p><w:r><w:t>Body.</w:t></w:r></w:p>
                <w:sectPr>
                  <w:footerReference w:type="default" r:id="rId2"/>
                  <w:pgSz w:w="12240" w:h="6000"/>
                  <w:pgMar w:top="720" w:right="1440" w:bottom="2400" w:left="1440"
                           w:header="360" w:footer="360"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream stream = new();
        using (ZipArchive archive = new(stream, ZipArchiveMode.Create, leaveOpen: true))
        {
            Add(archive, "[Content_Types].xml", ContentTypes);
            Add(archive, "_rels/.rels", RootRelationships);
            Add(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Add(archive, "word/settings.xml", Settings);
            Add(archive, "word/styles.xml", Styles);
            Add(archive, "word/footer1.xml", Footer);
            Add(archive, "word/document.xml", Document);
        }

        stream.Position = 0;
        return stream;

        static void Add(ZipArchive archive, string name, string content)
        {
            using StreamWriter writer = new(archive.CreateEntry(name).Open(), Encoding.UTF8);
            writer.Write(content);
        }
    }
}

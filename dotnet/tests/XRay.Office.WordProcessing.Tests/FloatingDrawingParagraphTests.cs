using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A paragraph holding nothing but <em>floating</em> drawings is an empty paragraph, so its paragraph
/// mark sizes it — and one holding an <em>inline</em> drawing is not.
/// </summary>
/// <remarks>
/// <para>
/// The reader used to emit one anchor character per <c>w:drawing</c>, floating or inline, because a
/// frame's offset has to mean the same thing wherever it was counted. That made every such paragraph
/// look text-bearing, so it took the <em>body</em> character style instead of the mark's — and where the
/// mark states a smaller size than the document default, the paragraph came out several times too tall.
/// Writer's import puts a <c>wp:anchor</c> into a fly and leaves the paragraph it was written in empty,
/// which is why the mark is what sizes it there.
/// </para>
/// <para>
/// <b>A floating drawing now emits no character at all</b>, which is the same rule stated at its
/// source rather than compensated for afterwards — see <see cref="FloatingAnchorLineBreakTests"/> for
/// the citation and for what the character cost besides the height. These four assertions are
/// unchanged by that and are kept as the height half of it.
/// </para>
/// <para>
/// Measured on <c>088_Printable_Graph_Paper_Template_Quality_layout_33051f6e.docx</c> against the
/// installed LibreOffice 26.2.4.2. Its grid table ends 8.45 pt above the bottom margin and is followed by
/// one paragraph whose mark is <c>w:sz="4"</c> — 2 pt — holding a single anchored logo. Read as
/// text-bearing that paragraph takes 11 pt from <c>docDefaults</c>, will not fit, and costs the document
/// a second page: <b>2 pages against the reference's 1</b>. Eleven authored variants of it, one variable
/// at a time, are recorded in <c>dotnet/probes/words-r50-chartset/</c>; the two that matter are that
/// deleting the drawing run alone gives 1 page on both stacks, and that raising the mark to
/// <c>w:sz="22"</c> gives 2 pages on <em>both</em> stacks — so the mark is honoured and the drawing is
/// the variable. No property of the frame — <c>wp:posOffset</c> at 0, −900000 or −266065, a
/// <c>wp:extent</c> cut to 9525 EMU, <c>behindDoc="1"</c>, <c>wrapNone</c> swapped for
/// <c>wrapSquare</c>, or <c>relativeFrom</c> changed from <c>paragraph</c> to <c>page</c> — moves it.
/// </para>
/// <para>
/// <see cref="AnInlineDrawingStillSizesItsParagraphAsText"/> is the control that keeps the rule narrow,
/// and it is the reason this is not simply "a paragraph with no <c>w:t</c> is empty": a
/// <c>wp:inline</c> genuinely occupies its line, and treating it as empty would take the height off
/// every as-character picture in the corpus.
/// </para>
/// </remarks>
public sealed class FloatingDrawingParagraphTests
{
    /// <summary>A lone anchored drawing leaves the paragraph empty, so the 2 pt mark sizes it.</summary>
    [Fact]
    public void AFloatingDrawingLeavesTheParagraphSizedByItsMark()
    {
        Read(Drawing(floating: true)).EmSize.ShouldBe(Length.FromPoints(2));
    }

    /// <summary>An inline drawing is a character on the line, so the body size still applies.</summary>
    [Fact]
    public void AnInlineDrawingStillSizesItsParagraphAsText()
    {
        Read(Drawing(floating: false)).EmSize.ShouldBe(Length.FromPoints(10));
    }

    /// <summary>Two anchored drawings in one paragraph are still nothing, and it is still the mark.</summary>
    [Fact]
    public void TwoFloatingDrawingsAreStillNothing()
    {
        Read(Drawing(floating: true) + Drawing(floating: true))
            .EmSize.ShouldBe(Length.FromPoints(2));
    }

    /// <summary>
    /// A floating drawing beside real text does not make the paragraph empty.
    /// </summary>
    /// <remarks>
    /// The rule tests the whole of the paragraph's text rather than counting its drawings, so a caption
    /// that happens to anchor a picture keeps its own size.
    /// </remarks>
    public sealed class WithText
    {
        /// <summary>Text beside the drawing keeps the body size.</summary>
        [Fact]
        public void TextBesideAFloatingDrawingKeepsTheBodySize()
        {
            Read(Drawing(floating: true) + """<w:r><w:t>Caption</w:t></w:r>""")
                .EmSize.ShouldBe(Length.FromPoints(10));
        }
    }

    /// <summary>One <c>w:r</c> holding a <c>w:drawing</c>, anchored or inline.</summary>
    /// <remarks>
    /// The two spellings differ in exactly one element and are otherwise identical, which is the point:
    /// only the <c>wp:anchor</c>/<c>wp:inline</c> distinction is under test. A <c>wp:extent</c> with a
    /// positive width and height is required or the frame reader returns nothing at all and the
    /// paragraph would be empty for the wrong reason.
    /// </remarks>
    private static string Drawing(bool floating)
    {
        const string Anchor =
            """<wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="1" """
            + """behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">"""
            + """<wp:simplePos x="0" y="0"/>"""
            + """<wp:positionH relativeFrom="column"><wp:posOffset>0</wp:posOffset></wp:positionH>"""
            + """<wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>""";

        const string Inline = """<wp:inline distT="0" distB="0" distL="0" distR="0">""";

        string open = floating ? Anchor : Inline;
        string close = floating ? "</wp:anchor>" : "</wp:inline>";
        string wrap = floating ? "<wp:wrapNone/>" : "";

        return $"""
            <w:r><w:drawing>
              {open}
                <wp:extent cx="1143000" cy="228600"/>
                {wrap}
                <wp:docPr id="1" name="Shape 1"/>
                <a:graphic>
                  <a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
                    <wps:wsp>
                      <wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="1143000" cy="228600"/></a:xfrm>
                        <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                      </wps:spPr>
                    </wps:wsp>
                  </a:graphicData>
                </a:graphic>
              {close}
            </w:drawing></w:r>
            """;
    }

    private static PageParagraph Read(string body)
    {
        using IDocument document = Open(body);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageParagraph>().First();
    }

    private static IDocument Open(string body)
    {
        MemoryStream package = BuildPackage(body);
        using DocumentSource source = DocumentSource.FromStream(package, "floating-drawing.docx");
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
              <Override PartName="/word/styles.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
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
              <Relationship Id="rId1" Target="styles.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
            </Relationships>
            """;

        // The body size and the mark's size are both stated outright and are deliberately far apart, so
        // an assertion on the paragraph's em size cannot be satisfied by a fallback.
        const string Styles = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:docDefaults>
                <w:rPrDefault>
                  <w:rPr>
                    <w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>
                    <w:sz w:val="20"/>
                  </w:rPr>
                </w:rPrDefault>
              </w:docDefaults>
              <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
                <w:name w:val="Normal"/>
              </w:style>
            </w:styles>
            """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
                        xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
                        xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
                        xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
                        xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
              <w:body>
                <w:p><w:pPr><w:rPr><w:sz w:val="4"/><w:szCs w:val="4"/></w:rPr></w:pPr>{body}</w:p>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/styles.xml", Styles);
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;
    }

    private static void Write(ZipArchive archive, string path, string content)
    {
        using Stream entry = archive.CreateEntry(path).Open();
        entry.Write(Encoding.UTF8.GetBytes(content));
    }
}

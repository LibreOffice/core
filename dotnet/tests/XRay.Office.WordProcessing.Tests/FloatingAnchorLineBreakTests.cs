using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A floating drawing puts <em>no character</em> in its paragraph's text, so it is not a place the
/// line can break.
/// </summary>
/// <remarks>
/// <para>
/// Writer says it in one place. <c>SwFormatFlyCnt</c> — and therefore the
/// <c>CH_TXTATR_BREAKWORD</c> that <c>GetCharOfTextAttr</c> gives <c>RES_TXTATR_FLYCNT</c>
/// (<c>sw/source/core/txtnode/thints.cxx</c>:3633-3652) — is inserted for <c>FLY_AS_CHAR</c> alone
/// (<c>SwDoc::SetFlyFrameAnchor</c>, <c>sw/source/core/doc/docfly.cxx</c>:337-348). A
/// <c>FLY_AT_CHAR</c> fly carries a <c>SwPosition</c> on its <c>SwFormatAnchor</c> and nothing in the
/// node's text at all.
/// </para>
/// <para>
/// What a character costs is a <em>break opportunity</em>. An inline object widens every prefix past
/// the boundary it occupies and none at or before it (<c>MeasuredParagraph</c>), so a picture wider
/// than the measure is placed anyway on a line it starts and moves to the next line when anything
/// precedes it. A run of anchor characters is "anything".
/// </para>
/// <para>
/// Measured on <c>PES-Technical-Report-Template_Jan_2019.docx</c> against 26.2.4.2, whose cover
/// paragraph is four <c>wp:anchor</c> runs followed by one <c>wp:inline</c> picture 612.5 pt wide on
/// a 612 pt zero-margin page. With the characters emitted the picture took a second line and
/// therefore a second page, and every page from 2 to 13 ran one behind the reference's. Bisected by
/// deleting runs from the document: the picture alone is right, the picture with <em>any one</em>
/// anchor before it is wrong, and moving the picture run to the head of the paragraph is right again.
/// Its <c>|ink|%</c> against 26.2.4.2 — round 67's ranking put it first in the whole words track at
/// 35.91 — is <b>2.37</b> with this closed.
/// </para>
/// </remarks>
public sealed class FloatingAnchorLineBreakTests
{
    /// <summary>A floating drawing contributes no character to the paragraph's text.</summary>
    [Fact]
    public void AFloatingDrawingIsNotACharacter()
    {
        Read(Floating + Floating).Text.ShouldBeEmpty();
    }

    /// <summary>An inline drawing does contribute one, since it occupies its line.</summary>
    [Fact]
    public void AnInlineDrawingIsACharacter()
    {
        Read(Inline).Text.Length.ShouldBe(1);
    }

    /// <summary>
    /// A floating drawing before an inline one leaves the inline one at offset nought.
    /// </summary>
    /// <remarks>
    /// The offset is the whole of the mechanism: at nought the object begins the line and is placed
    /// however wide it is, and past nought it can be pushed onto a line of its own.
    /// </remarks>
    [Fact]
    public void AnInlineDrawingAfterFloatingOnesStillStartsTheText()
    {
        PageParagraph paragraph = Read(Floating + Floating + Floating + Floating + Inline);

        paragraph.Text.Length.ShouldBe(1);
        paragraph.Frames.Count.ShouldBe(5);
        paragraph.Frames
            .Single(frame => frame.Anchor == FrameAnchor.AsCharacter)
            .AnchorOffset.ShouldBe(0);
    }

    /// <summary>
    /// An over-wide inline picture preceded by floating drawings still lays out on one line.
    /// </summary>
    /// <remarks>
    /// The consequence the mechanism buys, asserted as the paragraph's line count rather than as a
    /// page count: the picture is 612.5 pt wide in a 612 pt measure, so a second line would be a
    /// second page as well.
    /// </remarks>
    [Fact]
    public void AnOverWideInlinePictureAfterFloatingOnesTakesOneLine()
    {
        Lines(Floating + Floating + Floating + Floating + Inline).ShouldBe(1);
    }

    /// <summary>The same picture with nothing before it, which was right either way.</summary>
    [Fact]
    public void AnOverWideInlinePictureAloneTakesOneLine()
    {
        Lines(Inline).ShouldBe(1);
    }

    /// <summary>A <c>wp:anchor</c> shape, wrapped through so it is no obstacle.</summary>
    private const string Floating = """
        <w:r><w:drawing>
          <wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="1"
                     behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
            <wp:simplePos x="0" y="0"/>
            <wp:positionH relativeFrom="column"><wp:posOffset>457200</wp:posOffset></wp:positionH>
            <wp:positionV relativeFrom="paragraph"><wp:posOffset>457200</wp:posOffset></wp:positionV>
            <wp:extent cx="1143000" cy="228600"/>
            <wp:wrapNone/>
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
          </wp:anchor>
        </w:drawing></w:r>
        """;

    /// <summary>
    /// A <c>wp:inline</c> shape 612.5 pt wide and 792.65 pt tall — the corpus document's own extent,
    /// which is wider than the 612 pt page and taller than its zero-margin body.
    /// </summary>
    private const string Inline = """
        <w:r><w:drawing>
          <wp:inline distT="0" distB="0" distL="0" distR="0">
            <wp:extent cx="7778812" cy="10066699"/>
            <wp:docPr id="9" name="Cover"/>
            <a:graphic>
              <a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
                <wps:wsp>
                  <wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="7778812" cy="10066699"/></a:xfrm>
                    <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                  </wps:spPr>
                </wps:wsp>
              </a:graphicData>
            </a:graphic>
          </wp:inline>
        </w:drawing></w:r>
        """;

    private static int Lines(string body)
    {
        using IDocument document = Open(body);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Pages.Sum(page => page.Lines.Count(line => line.ParagraphIndex == 0));
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
        using DocumentSource source = DocumentSource.FromStream(package, "anchor-break.docx");
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

        // Letter, and every margin nought, which is the corpus document's own section: it is what makes
        // the 612.5 pt picture wider than the measure and the 792.65 pt one taller than the body.
        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
                        xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
                        xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
                        xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
                        xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
              <w:body>
                <w:p>{body}</w:p>
                <w:p><w:pPr><w:sectPr>
                  <w:pgSz w:w="12240" w:h="15840"/>
                  <w:pgMar w:top="0" w:right="0" w:bottom="0" w:left="0" w:header="0" w:footer="0"/>
                </w:sectPr></w:pPr></w:p>
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
    }

    private static void Write(ZipArchive archive, string path, string content)
    {
        using Stream entry = archive.CreateEntry(path).Open();
        entry.Write(Encoding.UTF8.GetBytes(content));
    }
}

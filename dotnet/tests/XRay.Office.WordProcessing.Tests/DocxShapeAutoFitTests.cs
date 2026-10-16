using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// <c>a:spAutoFit</c> takes a shape's height from its text and discards the <c>a:ext</c> it
/// states — except on a WordArt shape, which keeps the stated height however it autofits.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The fit is measured</strong>, on 26.2.4.2, over the twelve authored boxes in
/// <c>dotnet/probes/words-spautofit-r182/</c>: three text lengths crossed with a stated <c>cy</c>
/// of 200 pt and of 40 pt give <b>25.55, 62.45 and 117.80 pt whichever <c>cy</c> is stated</b>,
/// against a flat 200.00 and 40.00 for the same boxes under <c>a:noAutofit</c>. The stated height
/// is read as neither a floor nor a ceiling, which is why
/// <see cref="PageFrame.HeightFloor"/> exists — ODF's <c>fo:min-height</c> <em>is</em> a floor,
/// and a frame with less text than it states keeps its height.
/// </para>
/// <para>
/// <strong>The exception is the reference's own source.</strong>
/// <c>WpsContext::correctTextBoxesForWordArt</c> (<c>oox/source/shape/WpsContext.cxx</c>:1021-1023)
/// turns a shape whose body carries an <c>a:prstTxWarp</c> other than <c>textNoShape</c> into a
/// Fontwork and then clears <c>TextAutoGrowHeight</c>, saying why: <em>"Fontwork stretches the
/// text to the given path. So adapt shape size to text is nonsensical."</em>
/// </para>
/// <para>
/// It is not a corner. <c>words/done-014/docx/exhibit-06---technical-architecture-template.docx</c>
/// puts a <c>fromWordArt="1"</c> shape in each of its two headers, stating 412 × 248 pt around one
/// run of <b>1 pt</b> <c>DRAFT</c>; fitted to that run the diagonal watermark collapses to a speck,
/// and the document went from 0.13 to 3.18 unsigned ink and from no major page to two. It was the
/// only document in the 337 that the unqualified rule moved, and the corpus's only WordArt
/// autofitter.
/// </para>
/// </remarks>
public sealed class DocxShapeAutoFitTests
{
    /// <summary>A plain autofitting box takes its height from its text, not from <c>a:ext</c>.</summary>
    [Fact]
    public void AnAutoFittingBoxFitsItsText()
    {
        PageFrame frame = Only(Lay(autofit: "spAutoFit", warp: null));

        frame.GrowsToContent.ShouldBeTrue();
        frame.HeightFloor.ShouldBe(Length.Zero, "the stated cy is not even a floor");
    }

    /// <summary>A box that says nothing about fitting keeps the height it states.</summary>
    [Fact]
    public void ABoxWithoutAutoFitKeepsItsStatedHeight()
    {
        PageFrame frame = Only(Lay(autofit: "noAutofit", warp: null));

        frame.GrowsToContent.ShouldBeFalse();
        frame.HeightFloor.ShouldBeNull();
    }

    /// <summary>A WordArt shape keeps its stated height although it says it autofits.</summary>
    [Fact]
    public void AWordArtShapeIsNotFittedToItsText()
    {
        PageFrame frame = Only(Lay(autofit: "spAutoFit", warp: "textPlain"));

        frame.GrowsToContent.ShouldBeFalse("a Fontwork stretches its text along a path");
        frame.HeightFloor.ShouldBeNull();
    }

    /// <summary>
    /// <c>textNoShape</c> is the warp that is not one, so such a shape autofits like any other.
    /// </summary>
    /// <remarks>
    /// The control. <c>WpsContext::correctTextBoxesForWordArt</c> returns before touching the shape
    /// when the preset is empty or <c>textNoShape</c>, and Word writes exactly that on an ordinary
    /// rotated text box — <c>HC-Bulletin-template.docx</c>'s own caption box carries one. Reading
    /// the element rather than its value would therefore turn the fix off for the document it was
    /// written for.
    /// </remarks>
    [Fact]
    public void AnUnwarpedPresetDoesNotCountAsWordArt()
    {
        Only(Lay(autofit: "spAutoFit", warp: "textNoShape")).GrowsToContent.ShouldBeTrue();
    }

    /// <summary>
    /// A warp on a shape that is not a rectangle leaves the frame, and the frame still autofits.
    /// </summary>
    /// <remarks>
    /// <c>sType != "ooxml-rect"</c> (<c>WpsContext.cxx</c>:969-971), with the reason in the source:
    /// <em>"Word can combine its 'abc Transform' with a lot of shape types. LibreOffice can only
    /// render the old kind WordArt, which is based on a rectangle."</em>
    /// </remarks>
    [Fact]
    public void AWarpOnANonRectangleLeavesTheBoxFitting()
    {
        Only(Lay(autofit: "spAutoFit", warp: "textPlain", geometry: "ellipse"))
            .GrowsToContent.ShouldBeTrue();
    }

    private static PageFrame Only(WordProcessingPages pages)
    {
        foreach (LaidOutPage page in pages.Pages)
        {
            foreach (PlacedFrame frame in page.Frames)
            {
                if (frame.Frame.Name == "Box") return frame.Frame;
            }
        }

        throw new InvalidOperationException("the fixture has no frame named 'Box'");
    }

    private static WordProcessingPages Lay(string autofit, string? warp, string geometry = "rect")
    {
        using DocumentSource source = DocumentSource.FromStream(
            Package(autofit, warp, geometry), "shape-autofit.docx");
        using IDocument document = new WordProcessingReader().Read(source);
        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }

    private static MemoryStream Package(string autofit, string? warp, string geometry)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
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

        // An empty settings part, because a DOCX without one does not get LibreOffice's OOXML
        // compatibility defaults and answers a question nobody asked.
        const string DocumentRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="settings.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
            </Relationships>
            """;

        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        string warped = warp is null ? "" : $"""<a:prstTxWarp prst="{warp}"><a:avLst/></a:prstTxWarp>""";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
                        xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
                        xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
                        xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
              <w:body>
                <w:p><w:r><w:drawing>
                  <wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0"
                             relativeHeight="1" behindDoc="0" locked="0" layoutInCell="1"
                             allowOverlap="1">
                    <wp:simplePos x="0" y="0"/>
                    <wp:positionH relativeFrom="column"><wp:posOffset>0</wp:posOffset></wp:positionH>
                    <wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>
                    <wp:extent cx="4000000" cy="2540000"/>
                    <wp:wrapNone/>
                    <wp:docPr id="1" name="Box"/>
                    <a:graphic><a:graphicData
                          uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
                      <wps:wsp>
                        <wps:cNvSpPr txBox="1"/>
                        <wps:spPr>
                          <a:xfrm><a:off x="0" y="0"/><a:ext cx="4000000" cy="2540000"/></a:xfrm>
                          <a:prstGeom prst="{geometry}"><a:avLst/></a:prstGeom>
                          <a:solidFill><a:srgbClr val="00B0F0"/></a:solidFill>
                        </wps:spPr>
                        <wps:txbx><w:txbxContent>
                          <w:p><w:r><w:t>DRAFT</w:t></w:r></w:p>
                        </w:txbxContent></wps:txbx>
                        <wps:bodyPr wrap="square" lIns="91440" tIns="45720" rIns="91440"
                                    bIns="45720" anchor="t">{warped}<a:{autofit}/></wps:bodyPr>
                      </wps:wsp>
                    </a:graphicData></a:graphic>
                  </wp:anchor>
                </w:drawing></w:r></w:p>
                <w:sectPr>
                  <w:pgSz w:w="11906" w:h="16838"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
                           w:header="708" w:footer="708" w:gutter="0"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/settings.xml", Settings);
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

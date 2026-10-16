using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// What a vertically margin-relative frame — a watermark, most often — is positioned inside.
/// </summary>
/// <remarks>
/// <para>
/// <c>wp:positionV/@relativeFrom="margin"</c> is <c>RelOrientation::PAGE_PRINT_AREA</c>, and that area
/// is <em>not</em> <c>w:top</c>..<c>w:bottom</c>. <c>SwAnchoredObjectPosition::GetVertAlignmentValues</c>
/// (<c>sw/source/core/objectpositioning/anchoredobjectposition.cxx</c>:336-361) takes the page's print
/// area and then walks the page frame's lowers, subtracting each header frame's height from the area
/// and adding it to the offset, and subtracting each footer frame's height. So it runs from the header
/// frame's <em>bottom</em> to the footer frame's <em>top</em>.
/// </para>
/// <para>
/// Those coincide with the stated margins exactly while the running heads fit the room the margins
/// reserve for them, because Writer's DOCX import makes the page's own top margin <c>w:header</c> and
/// gives the header frame <c>w:top − w:header</c> as a dynamic height
/// (<c>dmapper/PropertyMap.cxx</c>:1148). They part company when one outgrows it — which is the same
/// quantity <c>Paginator.PushedDownBy</c> and <c>PulledUpBy</c> already apply to the body, so the
/// page's own body rectangle is the answer and no new measurement of the header is needed.
/// </para>
/// <para>
/// Measured in <c>dotnet/probes/words-margin-print-area/</c>, a 200 × 50 pt band centred vertically
/// against the margin on A4 with <c>w:top</c> = <c>w:header</c> = 708 twips, so the header has no room
/// reserved at all. Band centre in points, both installed references identical on every row:
/// </para>
/// <list type="table">
///   <item><term>no running heads</term><description>402.62, ours 402.75</description></item>
///   <item><term>a one-line header</term><description>409.50, ours <b>402.75</b> before</description></item>
///   <item><term>a three-line header</term><description>423.38, ours <b>402.75</b> before</description></item>
///   <item><term>a three-line footer</term><description>400.25, ours <b>402.75</b> before</description></item>
///   <item>
///     <term>a one-line header with <c>w:top</c> 2000</term>
///     <description>435.00, ours 435.00 — the control: reserved room that is not exceeded moves nothing</description>
///   </item>
/// </list>
/// <para>
/// On <c>DOA_Template_Form_Type_Certification_Programme.docx</c>, whose watermark this is, the
/// document's own header is a three-row table: pages 2 and 4 to 8 put the watermark's centre at
/// 342.7 pt against both references' 359.3, and now put it at 359.5.
/// </para>
/// <para>
/// Horizontally there is no such rule. The header/footer walk in the horizontal case is guarded by
/// <c>aRectFnSet.IsVert()</c> (the same file, :824), so it applies to a vertical writing mode only.
/// </para>
/// </remarks>
public sealed class FrameMarginAreaTests
{
    /// <summary>
    /// A header that outgrows the room its margins reserve carries the margin area down with it.
    /// </summary>
    /// <remarks>
    /// Asserted as "the frame is centred in the page's body rectangle" rather than against a stored
    /// point, because that is the rule — and it makes the two halves of the test say different things:
    /// the body rectangle has moved here and has not in <see cref="RoomThatIsNotExceededMovesNothing"/>.
    /// </remarks>
    [Theory]
    [InlineData(1)]
    [InlineData(3)]
    public void AHeaderTallerThanItsMarginCarriesTheMarginAreaDown(int headerLines)
    {
        (DocRect frame, DocRect body, DocRect text) = Centred(headerLines, top: 708);

        body.Y.ShouldBeGreaterThan(text.Y, "the header outgrew the room w:top reserves for it");
        Centre(frame).ShouldBe(Centre(body));
        Centre(frame).ShouldNotBe(Centre(text));
    }

    /// <summary>A header inside the room its margins reserve moves the area not at all.</summary>
    /// <remarks>
    /// The control the probe turns on: <c>w:top</c> 2000 twips against <c>w:header</c> 708 leaves 64.6 pt
    /// for a header needing about 14, and both references put the band at 435.00 pt either way. Without
    /// this the rule could be read as "the area starts below the header's content", which is a different
    /// rule that happens to agree whenever nothing is reserved.
    /// </remarks>
    [Fact]
    public void RoomThatIsNotExceededMovesNothing()
    {
        (DocRect frame, DocRect body, DocRect text) = Centred(headerLines: 1, top: 2000);

        body.Y.ShouldBe(text.Y);
        Centre(frame).ShouldBe(Centre(text));
    }

    /// <summary>With no running head at all the area is the text area, as it always was.</summary>
    [Fact]
    public void NoRunningHeadLeavesTheTextArea()
    {
        (DocRect frame, DocRect body, DocRect text) = Centred(headerLines: 0, top: 708);

        body.ShouldBe(text);
        Centre(frame).ShouldBe(Centre(text));
    }

    private static Length Centre(DocRect rectangle) => rectangle.Y + (rectangle.Height / 2);

    /// <summary>
    /// A one-page document with a margin-centred band and a header of the given height, and where the
    /// three rectangles that decide the band's position ended up.
    /// </summary>
    private static (DocRect Frame, DocRect Body, DocRect Text) Centred(int headerLines, int top)
    {
        using MemoryStream package = BuildPackage(headerLines, top);
        using DocumentSource source = DocumentSource.FromStream(package, "margin-area.docx");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        LaidOutPage page = pages.Pages[0];

        PlacedFrame band = page.Frames
            .Where(frame => frame.Frame.Anchor != FrameAnchor.AsCharacter)
            .ShouldHaveSingleItem();

        // The text area the section *states*, built from the fixture's own numbers rather than read
        // back off the layout — the whole question here is whether the body's rectangle moved away
        // from it, so taking both from the same place would assert nothing.
        DocRect stated = new(
            Length.FromTwips(1440),
            Length.FromTwips(top),
            Length.FromTwips(11906 - (2 * 1440)),
            Length.FromTwips(16838 - top - 1440));

        return (band.Area, page.BodyArea, stated);
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";

    private const string Namespaces =
        $"""xmlns:w="{W}" xmlns:r="{R}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}" """;

    private const string RunProperties =
        """<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr>""";

    /// <summary>A 200 × 50 pt band, centred both ways against the margin and wrapping nothing.</summary>
    private const string Band = """
        <w:r><w:drawing><wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0"
             relativeHeight="1" behindDoc="1" locked="0" layoutInCell="0" allowOverlap="1">
          <wp:simplePos x="0" y="0"/>
          <wp:positionH relativeFrom="margin"><wp:align>center</wp:align></wp:positionH>
          <wp:positionV relativeFrom="margin"><wp:align>center</wp:align></wp:positionV>
          <wp:extent cx="2540000" cy="635000"/>
          <wp:wrapNone/>
          <wp:docPr id="9" name="band"/>
          <a:graphic><a:graphicData uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
            <wps:wsp><wps:cNvSpPr/><wps:spPr>
              <a:xfrm><a:off x="0" y="0"/><a:ext cx="2540000" cy="635000"/></a:xfrm>
              <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
              <a:solidFill><a:srgbClr val="000000"/></a:solidFill>
            </wps:spPr><wps:bodyPr/></wps:wsp>
          </a:graphicData></a:graphic>
        </wp:anchor></w:drawing></w:r>
        """;

    private static MemoryStream BuildPackage(int headerLines, int top)
    {
        bool hasHeader = headerLines > 0;

        string contentTypes =
            """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
            """
            + (hasHeader
                ? """
                  <Override PartName="/word/header1.xml"
                            ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
                  """
                : string.Empty)
            + "</Types>";

        const string RootRelationships = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="{R}/officeDocument"/>
            </Relationships>
            """;

        string documentRelationships =
            """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
            """
            + (hasHeader ? $"""<Relationship Id="rIdH" Target="header1.xml" Type="{R}/header"/>""" : string.Empty)
            + "</Relationships>";

        string header = $"<?xml version=\"1.0\" encoding=\"UTF-8\"?><w:hdr {Namespaces}>"
            + string.Concat(Enumerable.Range(0, headerLines)
                .Select(line => $"<w:p><w:r>{RunProperties}<w:t>HEADER{line}</w:t></w:r></w:p>"))
            + "</w:hdr>";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document {Namespaces}>
              <w:body>
                <w:p>{Band}<w:r>{RunProperties}<w:t>BODYLINE</w:t></w:r></w:p>
                <w:sectPr>
                  {(hasHeader ? """<w:headerReference w:type="default" r:id="rIdH"/>""" : string.Empty)}
                  <w:pgSz w:w="11906" w:h="16838"/>
                  <w:pgMar w:top="{top}" w:right="1440" w:bottom="1440" w:left="1440"
                           w:header="708" w:footer="708" w:gutter="0"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", contentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/document.xml", document);
            Write(archive, "word/_rels/document.xml.rels", documentRelationships);
            if (hasHeader) Write(archive, "word/header1.xml", header);
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

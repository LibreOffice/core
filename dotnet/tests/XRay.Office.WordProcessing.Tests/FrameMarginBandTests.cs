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
/// The four margin <em>bands</em> a frame can be positioned against, which are not the margin area
/// <see cref="FrameMarginAreaTests"/> covers and are not each other.
/// </summary>
/// <remarks>
/// <para>
/// <c>relativeFrom</c> has nine values across the two axes and
/// <c>PositionHandler::lcl_attribute</c> (<c>sw/source/writerfilter/dmapper/GraphicHelpers.cxx</c>:57-135)
/// maps each onto a <c>RelOrientation</c>. Four of them name a band rather than the page:
/// <c>topMargin</c> and <c>bottomMargin</c> are the strips above and below the body, and
/// <c>leftMargin</c> and <c>rightMargin</c> the strips beside it. A fifth, <c>outsideMargin</c>,
/// reaches no case at all — that switch warns and leaves the default, which is the text column.
/// </para>
/// <para>
/// Every one of them had been read as "the page" or "the margin area", which agrees with the
/// reference on a plain offset against <c>leftMargin</c> — the band and the page share a left edge —
/// and on nothing else.
/// </para>
/// <para>
/// The expectations below are measured in <c>dotnet/probes/frame-area-r85/</c> on 34 one-page A4
/// fixtures carrying a red band, rendered through <b>both</b> installed references, which agree with
/// each other on all 34. This tree agreed with them on 10 before and on 34 after.
/// </para>
/// </remarks>
public sealed class FrameMarginBandTests
{
    // A4 and a 1 inch margin on every side, which is what `Package` builds.
    private const int PageWidth = 11906;
    private const int PageHeight = 16838;
    private const int Margin = 1440;
    private const int BandWidth = 800;    // 40 pt
    private const int BandHeight = 400;   // 20 pt

    /// <summary>
    /// <c>topMargin</c> is the strip from the sheet's top edge to the body's, and it is the case the
    /// C++ cannot be read off one function.
    /// </summary>
    /// <remarks>
    /// <c>GetVertAlignmentValues</c> puts <c>PAGE_PRINT_AREA_TOP</c> in the <em>same case</em> as
    /// <c>PAGE_FRAME</c> and answers the whole page's height
    /// (<c>sw/source/core/objectpositioning/anchoredobjectposition.cxx</c>:327-335), which would
    /// centre this band at 410.95 pt. <c>SwToContentAnchoredObjectPosition::CalcPosition</c>
    /// overrides that for exactly this relation
    /// (<c>tocntntanchoredobjectposition.cxx</c>:305-306, :367-370, :407-410), and both references
    /// draw it at 26.00.
    /// </remarks>
    [Theory]
    [InlineData("<wp:posOffset>0</wp:posOffset>", 0)]
    [InlineData("<wp:posOffset>457200</wp:posOffset>", 720)]           // 36 pt
    [InlineData("<wp:align>top</wp:align>", 0)]
    [InlineData("<wp:align>center</wp:align>", (Margin - BandHeight) / 2)]
    [InlineData("<wp:align>bottom</wp:align>", Margin - BandHeight)]
    public void TheTopMarginBandRunsFromTheSheetToTheBody(string position, int expectedTwips)
        => Placed(vertical: $"""<wp:positionV relativeFrom="topMargin">{position}</wp:positionV>""")
            .Y.ShouldBe(Length.FromTwips(expectedTwips));

    /// <summary>
    /// <c>bottomMargin</c> is the strip from the body's bottom edge to the sheet's — not from the
    /// print area's bottom, and not the same rectangle as <c>margin</c>.
    /// </summary>
    /// <remarks>
    /// <c>PAGE_PRINT_AREA_BOTTOM</c> takes <c>PrtWithoutHeaderAndFooter().Bottom</c> as its origin
    /// (<c>tocntntanchoredobjectposition.cxx</c>:617-628) and adds each footer frame's height back
    /// into its own height (<c>anchoredobjectposition.cxx</c>:365-389), so it reaches the sheet's
    /// own edge.
    /// </remarks>
    [Theory]
    [InlineData("<wp:posOffset>0</wp:posOffset>", PageHeight - Margin)]
    [InlineData("<wp:posOffset>457200</wp:posOffset>", PageHeight - Margin + 720)]
    [InlineData("<wp:align>top</wp:align>", PageHeight - Margin)]
    [InlineData("<wp:align>center</wp:align>", PageHeight - Margin + ((Margin - BandHeight) / 2))]
    [InlineData("<wp:align>bottom</wp:align>", PageHeight - BandHeight)]
    public void TheBottomMarginBandRunsFromTheBodyToTheSheet(string position, int expectedTwips)
        => Placed(vertical: $"""<wp:positionV relativeFrom="bottomMargin">{position}</wp:positionV>""")
            .Y.ShouldBe(Length.FromTwips(expectedTwips));

    /// <summary>
    /// <c>leftMargin</c> is <c>w:left</c> wide, so only an offset against it agrees with the page.
    /// </summary>
    /// <remarks>
    /// <c>RelOrientation::PAGE_LEFT</c>, <c>nWidth = GetLeftMargin(page)</c>
    /// (<c>anchoredobjectposition.cxx</c>:769-778). The first row is the coincidence that hid this:
    /// the band and the sheet share a left edge.
    /// </remarks>
    [Theory]
    [InlineData("<wp:posOffset>0</wp:posOffset>", 0)]
    [InlineData("<wp:align>left</wp:align>", 0)]
    [InlineData("<wp:align>center</wp:align>", (Margin - BandWidth) / 2)]
    [InlineData("<wp:align>right</wp:align>", Margin - BandWidth)]
    public void TheLeftMarginBandIsTheMarginAndNotTheSheet(string position, int expectedTwips)
        => Placed(horizontal: $"""<wp:positionH relativeFrom="leftMargin">{position}</wp:positionH>""")
            .X.ShouldBe(Length.FromTwips(expectedTwips));

    /// <summary>
    /// <c>rightMargin</c> starts at the text area's right edge, so even a plain offset moves.
    /// </summary>
    /// <remarks>
    /// <c>RelOrientation::PAGE_RIGHT</c>, measured from <c>GetPrtRight(page)</c>
    /// (<c>anchoredobjectposition.cxx</c>:779-788). Reading it as the sheet put a stated offset of
    /// zero at x = 0 where both references draw it at 523.25 pt.
    /// </remarks>
    [Theory]
    [InlineData("<wp:posOffset>0</wp:posOffset>", PageWidth - Margin)]
    [InlineData("<wp:align>left</wp:align>", PageWidth - Margin)]
    [InlineData("<wp:align>center</wp:align>", PageWidth - Margin + ((Margin - BandWidth) / 2))]
    [InlineData("<wp:align>right</wp:align>", PageWidth - BandWidth)]
    public void TheRightMarginBandStartsAtTheTextArea(string position, int expectedTwips)
        => Placed(horizontal: $"""<wp:positionH relativeFrom="rightMargin">{position}</wp:positionH>""")
            .X.ShouldBe(Length.FromTwips(expectedTwips));

    /// <summary>
    /// <c>insideMargin</c> is the sheet and <c>outsideMargin</c> is the text column — which is the
    /// asymmetry a reader would never guess, and it is not a symmetry the file intends.
    /// </summary>
    /// <remarks>
    /// <c>GraphicHelpers.cxx</c>:109-112 gives <c>insideMargin</c> <c>PAGE_FRAME</c> with a page
    /// toggle; <c>outsideMargin</c> has no case, so it keeps the handler's <c>RelOrientation::FRAME</c>
    /// default under a <c>SAL_WARN</c> (:130-132). Both references put a fixture stating
    /// <c>outsideMargin</c> with a zero offset at the column's own left edge, 72.00 pt.
    /// </remarks>
    [Theory]
    [InlineData("insideMargin", "<wp:posOffset>0</wp:posOffset>", 0)]
    [InlineData("outsideMargin", "<wp:posOffset>0</wp:posOffset>", Margin)]
    [InlineData("outsideMargin", "<wp:align>right</wp:align>", PageWidth - Margin - BandWidth)]
    public void TheInsideAndOutsideMarginsAreNotAPair(
        string relativeFrom, string position, int expectedTwips)
        => Placed(horizontal: $"""<wp:positionH relativeFrom="{relativeFrom}">{position}</wp:positionH>""")
            .X.ShouldBe(Length.FromTwips(expectedTwips));

    /// <summary>
    /// VML's own mapping crosses the horizontal pair over, and it is a different function.
    /// </summary>
    /// <remarks>
    /// <c>lcl_SetAnchorType</c> (<c>oox/source/vml/vmlshape.cxx</c>:687-692) pairs
    /// <c>inner-margin-area</c> with <c>right-margin-area</c> under <c>PAGE_RIGHT</c> and
    /// <c>outer-margin-area</c> with <c>left-margin-area</c> under <c>PAGE_LEFT</c> — the opposite
    /// way round from DrawingML's <c>insideMargin</c>/<c>outsideMargin</c> above. So the two
    /// branches of one <c>mc:AlternateContent</c> genuinely mean different things, and a reader that
    /// translated VML into the DrawingML vocabulary would put these two 523 pt apart.
    /// </remarks>
    [Theory]
    [InlineData("left-margin-area", 0)]
    [InlineData("outer-margin-area", 0)]
    [InlineData("right-margin-area", PageWidth - Margin)]
    [InlineData("inner-margin-area", PageWidth - Margin)]
    public void AVmlInnerMarginIsTheRightOneAndAnOuterMarginTheLeft(
        string relative, int expectedTwips)
        => PlacedVml($"mso-position-horizontal-relative:{relative};"
                     + "mso-position-vertical-relative:page")
            .X.ShouldBe(Length.FromTwips(expectedTwips));

    /// <summary>The vertical pair does not cross over, and shares the DrawingML relations.</summary>
    /// <remarks><c>vmlshape.cxx</c>:633-640.</remarks>
    [Theory]
    [InlineData("top-margin-area", 0)]
    [InlineData("bottom-margin-area", PageHeight - Margin)]
    [InlineData("margin", Margin)]
    [InlineData("page", 0)]
    public void AVmlMarginAreaIsTheSameBandItsDrawingMlSpellingNames(
        string relative, int expectedTwips)
        => PlacedVml("mso-position-horizontal-relative:page;"
                     + $"mso-position-vertical-relative:{relative}")
            .Y.ShouldBe(Length.FromTwips(expectedTwips));

    /// <summary>
    /// A <em>text box</em> the text wraps around is captured in the body; a wrap-through one is not,
    /// a bare shape never is, and neither is captured at all below <c>compatibilityMode</c> 15.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The band is 20 pt tall and stated 10 pt below the sheet's top, which puts it wholly inside a
    /// 72 pt top margin — so the only thing that can move it is the capture, and the capture can only
    /// move it to the body's own top.
    /// </para>
    /// <para>
    /// Measured on <c>008_Free_Genogram_Diagram_Template_Green_and_Yellow_Theme</c>, whose one
    /// <c>wp:wrapSquare</c> title box states <c>topMargin</c> with a 20.71 pt offset, by rewriting that
    /// one attribute and rendering each variant through 26.2.4.2: as <c>page</c> its text is drawn at
    /// y 83.56, as <c>margin</c> at 155.56, and as <c>topMargin</c> at <b>134.86</b> — which is neither,
    /// and is the page-relative placement clamped to the body's top. This tree answers 83.556, 155.556
    /// and 134.856. <c>dotnet/probes/frame-area-r85/</c>.
    /// </para>
    /// <para>
    /// <b>That document's box carries a <c>wps:txbx</c>, and this fixture did not — so it asserted the
    /// right answer for the wrong object.</b> <c>bConsidered</c> is
    /// <c>bWrapThrough || !bTextBox</c> for a drawing object
    /// (<c>anchoredobjectposition.cxx</c>:125-144), so a shape with no text box is exempt from the
    /// capture whatever its wrap. Measured on 26.2.4.2 over 93 one-attribute fixtures — three object
    /// kinds × five wraps × six positions, <c>probes/words-close-r95/</c> — the reference draws
    /// <c>shape-square-vbody-c15</c> at y 20.00 and <c>text-square-vbody-c15</c> at 72.00, and this
    /// tree reproduces all 91 that carry a scoreable band.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(true, "wrapSquare", 15, Margin)]
    [InlineData(true, "wrapNone", 15, 200)]
    [InlineData(true, "wrapSquare", 12, 200)]
    [InlineData(true, "wrapNone", 12, 200)]
    [InlineData(false, "wrapSquare", 15, 200)]
    [InlineData(false, "wrapNone", 15, 200)]
    [InlineData(false, "wrapSquare", 12, 200)]
    public void OnlyAWrappedTextBoxIsPulledIntoTheBody(
        bool textBox, string wrap, int compatibility, int expectedTwips)
        => Placed(
                vertical: """<wp:positionV relativeFrom="topMargin"><wp:posOffset>127000</wp:posOffset></wp:positionV>""",
                wrap: wrap,
                compatibility: compatibility,
                textBox: textBox)
            .Y.ShouldBe(Length.FromTwips(expectedTwips));

    /// <summary>The band's rectangle on page one, from a DrawingML anchor.</summary>
    // `textBox` writes a `wps:txbx`, which is what makes the shape half of a TextBox pair and
    // therefore something the page capture can reach at all -- see
    // OnlyAWrappedTextBoxIsPulledIntoTheBody.
    private static DocRect Placed(
        string? horizontal = null,
        string? vertical = null,
        string wrap = "wrapNone",
        int compatibility = 0,
        bool textBox = false)
        => Layout(Package(
            $"""
            <w:r><w:drawing><wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0"
                 relativeHeight="1" behindDoc="1" locked="0" layoutInCell="0" allowOverlap="1">
              <wp:simplePos x="0" y="0"/>
              {horizontal ?? """<wp:positionH relativeFrom="page"><wp:posOffset>1270000</wp:posOffset></wp:positionH>"""}
              {vertical ?? """<wp:positionV relativeFrom="page"><wp:posOffset>3810000</wp:posOffset></wp:positionV>"""}
              <wp:extent cx="508000" cy="254000"/>
              <wp:{wrap}/>
              <wp:docPr id="9" name="band"/>
              <a:graphic><a:graphicData uri="{Wps}">
                <wps:wsp><wps:cNvSpPr/><wps:spPr>
                  <a:xfrm><a:off x="0" y="0"/><a:ext cx="508000" cy="254000"/></a:xfrm>
                  <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                  <a:solidFill><a:srgbClr val="FF0000"/></a:solidFill>
                </wps:spPr>{(textBox
                    ? """<wps:txbx><w:txbxContent><w:p/></w:txbxContent></wps:txbx>"""
                    : string.Empty)}<wps:bodyPr/></wps:wsp>
              </a:graphicData></a:graphic>
            </wp:anchor></w:drawing></w:r>
            """, compatibility));

    /// <summary>The same band as a <c>v:rect</c>, whose <c>style</c> carries the two origins.</summary>
    private static DocRect PlacedVml(string origins)
        => Layout(Package(
            $"""
            <w:r><w:pict><v:rect id="band" fillcolor="#ff0000" stroked="f"
                 style="position:absolute;margin-left:0;margin-top:0;width:40pt;height:20pt;{origins}">
              <w10:wrap type="none"/></v:rect></w:pict></w:r>
            """));

    private static DocRect Layout(MemoryStream package)
    {
        using (package)
        {
            using DocumentSource source = DocumentSource.FromStream(package, "margin-band.docx");
            using IDocument document = new WordProcessingReader().Read(source);
            WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

            return pages.Pages[0].Frames
                .Where(frame => frame.Frame.Anchor != FrameAnchor.AsCharacter)
                .ShouldHaveSingleItem()
                .Area;
        }
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    private const string V = "urn:schemas-microsoft-com:vml";
    private const string W10 = "urn:schemas-microsoft-com:office:word";

    private const string Namespaces =
        $"""xmlns:w="{W}" xmlns:r="{R}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}" """
        + $"""xmlns:v="{V}" xmlns:w10="{W10}" """;

    private const string RunProperties =
        """<w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/></w:rPr>""";

    /// <summary>
    /// One A4 page with a one-line running head and foot, both fitting the room their margins
    /// reserve — so the body's own rectangle is exactly <c>w:top</c>..<c>pageHeight − w:bottom</c> and
    /// the header-overflow rule <see cref="FrameMarginAreaTests"/> covers cannot confound these.
    /// </summary>
    /// <summary>The package's content types, with the settings part when one is written.</summary>
    private static string ContentTypes(bool settings) => """
        <?xml version="1.0" encoding="UTF-8"?>
        <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
          <Default Extension="rels"
                   ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
          <Default Extension="xml" ContentType="application/xml"/>
          <Override PartName="/word/document.xml"
                    ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
          <Override PartName="/word/header1.xml"
                    ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
          <Override PartName="/word/footer1.xml"
                    ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>
        """
        + (settings
            ? """
              <Override PartName="/word/settings.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
              """
            : string.Empty)
        + "</Types>";

    private static MemoryStream Package(string band, int compatibility = 0)
    {
        string contentTypes = ContentTypes(compatibility > 0);
        const string RootRelationships = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml" Type="{R}/officeDocument"/>
            </Relationships>
            """;

        string documentRelationships = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rIdH" Target="header1.xml" Type="{R}/header"/>
              <Relationship Id="rIdF" Target="footer1.xml" Type="{R}/footer"/>
              {(compatibility > 0
                  ? $"""<Relationship Id="rIdS" Target="settings.xml" Type="{R}/settings"/>"""
                  : string.Empty)}
            </Relationships>
            """;

        string header = $"<?xml version=\"1.0\" encoding=\"UTF-8\"?><w:hdr {Namespaces}>"
            + $"<w:p><w:r>{RunProperties}<w:t>HDR</w:t></w:r></w:p></w:hdr>";
        string footer = $"<?xml version=\"1.0\" encoding=\"UTF-8\"?><w:ftr {Namespaces}>"
            + $"<w:p><w:r>{RunProperties}<w:t>FTR</w:t></w:r></w:p></w:ftr>";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document {Namespaces}>
              <w:body>
                <w:p>{band}<w:r>{RunProperties}<w:t>BODYLINE</w:t></w:r></w:p>
                <w:sectPr>
                  <w:headerReference w:type="default" r:id="rIdH"/>
                  <w:footerReference w:type="default" r:id="rIdF"/>
                  <w:pgSz w:w="{PageWidth}" w:h="{PageHeight}"/>
                  <w:pgMar w:top="{Margin}" w:right="{Margin}" w:bottom="{Margin}" w:left="{Margin}"
                           w:header="720" w:footer="720" w:gutter="0"/>
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
            Write(archive, "word/header1.xml", header);
            Write(archive, "word/footer1.xml", footer);
            if (compatibility > 0)
            {
                Write(archive, "word/settings.xml", $"""
                    <?xml version="1.0" encoding="UTF-8"?>
                    <w:settings {Namespaces}><w:compat>
                      <w:compatSetting w:name="compatibilityMode"
                                       w:uri="http://schemas.microsoft.com/office/word"
                                       w:val="{compatibility}"/>
                    </w:compat></w:settings>
                    """);
            }
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

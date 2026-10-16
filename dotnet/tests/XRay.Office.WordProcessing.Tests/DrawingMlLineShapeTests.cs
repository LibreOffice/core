using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A DrawingML shape whose preset geometry is a line is drawn as its box's diagonal, not as its box.
/// </summary>
/// <remarks>
/// <para>
/// <c>Shape::createAndInsert</c> maps <c>XML_line</c> and <c>XML_straightConnector1</c> to
/// <c>ConnectorType_LINE</c> (<c>oox/source/drawingml/shape.cxx</c>:2124-2127), which draws one
/// straight segment corner to corner of the shape's own rectangle. Word writes a flowchart's
/// arrows exactly this way — a <c>wps:wsp</c> with a <c>wps:cNvCnPr</c>, a <c>prstGeom</c> of
/// <c>line</c>, and frequently a zero extent in one dimension.
/// </para>
/// <para>
/// XRay.Office drew the rectangle instead, which puts three sides on the page that are not in the
/// file. Measured against 24.2.7.2 on the document these fixtures are reduced from: a red
/// 2 × 1 inch line shape, drawn by LibreOffice as a diagonal and by XRay.Office as a box. The VML
/// and Escher front ends already modelled it — <see cref="PageFrame.IsLine"/> — so this is the
/// DrawingML reading catching up with them rather than new drawing code.
/// </para>
/// </remarks>
public sealed class DrawingMlLineShapeTests
{
    [Theory]
    [InlineData("line")]
    [InlineData("straightConnector1")]
    public void ALinePresetIsDrawnAsOneSegment(string preset)
    {
        DrawnStroke stroke = OnlyStroke(preset);

        // The anchor puts a 2 x 1 inch shape an inch in from the column and a quarter down.
        stroke.Bounds.Width.Points.ShouldBe(144, 0.5, preset);
        stroke.Bounds.Height.Points.ShouldBe(72, 0.5, preset);
    }

    /// <summary>
    /// The control: any other preset keeps its four sides, so the reading is of the geometry and
    /// not of "a shape with no fill".
    /// </summary>
    /// <remarks>
    /// A rectangle and a diagonal have the same bounding box, so the two cannot be told apart by
    /// bounds. They are told apart by how much is drawn: one segment against a closed path of
    /// four, which the recorded path's own point count carries.
    /// </remarks>
    [Fact]
    public void ARectanglePresetKeepsItsFourSides()
        => OnlyStroke("rect").Bounds.Width.Points.ShouldBe(144, 0.5);

    [Fact]
    public void ALineIsTwoPointsAndARectangleIsMore()
    {
        PointsOf("line").ShouldBe(2);
        PointsOf("straightConnector1").ShouldBe(2);
        PointsOf("rect").ShouldBeGreaterThan(2);
    }

    /// <summary>
    /// Which diagonal follows the flips, and a shape flipped both ways is the line it started as.
    /// </summary>
    /// <remarks>
    /// Asserted on the <em>segment</em> — which corners the ends sit on — rather than on the order
    /// the two points are written in, because the order is a separate question that
    /// <see cref="TheHorizontalFlipReversesTheDirection"/> asks. This test read the order until
    /// arrowheads landed and the two came apart: it said <c>points[0].Y > points[1].Y</c>, which is
    /// true of the same diagonal traversed one way and false of it traversed the other.
    /// </remarks>
    [Theory]
    [InlineData("", false)]
    [InlineData(" flipH=\"1\"", true)]
    [InlineData(" flipV=\"1\"", true)]
    [InlineData(" flipH=\"1\" flipV=\"1\"", false)]
    public void TheFlipsChooseTheDiagonal(string flips, bool mirrored)
    {
        IReadOnlyList<DocPoint> points = OnlyStroke("line", flips).Points;

        // Mirrored is the bottom-left/top-right diagonal, on which the leftmost end is the lower.
        DocPoint left = points[0].X <= points[1].X ? points[0] : points[1];
        DocPoint right = points[0].X <= points[1].X ? points[1] : points[0];

        (left.Y > right.Y).ShouldBe(mirrored, flips);
    }

    /// <summary>
    /// <c>flipH</c> reverses the direction the line is drawn in, whichever diagonal it is on.
    /// </summary>
    /// <remarks>
    /// Invisible until the line carries an arrowhead, and then decisive: the organogram templates
    /// join their boxes with horizontal connectors that are flipped horizontally, carry a tail
    /// arrow, and are turned through 270°, so the arrow the reference draws pointing down came out
    /// pointing up on every one of them.
    /// </remarks>
    [Theory]
    [InlineData("", false)]
    [InlineData(" flipH=\"1\"", true)]
    [InlineData(" flipV=\"1\"", false)]
    [InlineData(" flipH=\"1\" flipV=\"1\"", true)]
    public void TheHorizontalFlipReversesTheDirection(string flips, bool reversed)
    {
        IReadOnlyList<DocPoint> points = OnlyStroke("line", flips).Points;

        // Reversed starts at the right-hand end, which is where a tail arrow would not be.
        (points[0].X > points[1].X).ShouldBe(reversed, flips);
    }

    // ------------------------------------------------------------------ helpers

    private static int PointsOf(string preset) => OnlyStroke(preset).Points.Count;

    private static DrawnStroke OnlyStroke(string preset, string flips = "")
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromStream(Package(preset, flips), "line.docx"))
        {
            using IDocument document = new WordProcessingReader().Read(source);
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return sink.Pages.SelectMany(page => page.StrokedPaths).ShouldHaveSingleItem();
    }

    private static MemoryStream Package(string preset, string flips)
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
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
                        xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
                        xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
                        xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
              <w:body>
                <w:p><w:r><w:drawing>
                  <wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="1"
                             behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
                    <wp:simplePos x="0" y="0"/>
                    <wp:positionH relativeFrom="column"><wp:posOffset>914400</wp:posOffset></wp:positionH>
                    <wp:positionV relativeFrom="paragraph"><wp:posOffset>228600</wp:posOffset></wp:positionV>
                    <wp:extent cx="1828800" cy="914400"/>
                    <wp:effectExtent l="0" t="0" r="0" b="0"/>
                    <wp:wrapNone/>
                    <wp:docPr id="1" name="Line 1"/>
                    <a:graphic><a:graphicData
                        uri="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
                      <wps:wsp>
                        <wps:cNvCnPr/>
                        <wps:spPr>
                          <a:xfrm{flips}><a:off x="0" y="0"/><a:ext cx="1828800" cy="914400"/></a:xfrm>
                          <a:prstGeom prst="{preset}"><a:avLst/></a:prstGeom>
                          <a:noFill/>
                          <a:ln w="28575"><a:solidFill><a:srgbClr val="FF0000"/></a:solidFill></a:ln>
                        </wps:spPr>
                        <wps:bodyPr/>
                      </wps:wsp>
                    </a:graphicData></a:graphic>
                  </wp:anchor>
                </w:drawing></w:r></w:p>
                <w:p><w:r><w:t>after</w:t></w:r></w:p>
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

        static void Write(ZipArchive archive, string path, string content)
        {
            using StreamWriter writer = new(archive.CreateEntry(path).Open(), Encoding.UTF8);
            writer.Write(content);
        }
    }
}

using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The arrowheads a Word line carries, and the fact that a line has a direction.
/// </summary>
/// <remarks>
/// <para>
/// <c>a:headEnd</c> and <c>a:tailEnd</c> were read on the slide side and not here, so a Word
/// flowchart's connectors drew as plain sticks. Censused over the 271 corpus <c>docx</c>:
/// <b>608 line ends across 38 documents</b> — 353 tails and 255 heads, with 208 of them in one
/// integrated-management-system manual.
/// </para>
/// <para>
/// The reading is <c>XRay.Office.Core</c>'s <see cref="LineEnds"/>, which moved down from
/// <c>XRay.Office.Presentations</c> for this: an arrowhead is a filled polygon beside a shortened
/// shaft, and that is LibreOffice's own decomposition rather than either format's. What these
/// assert is the wiring — that the element is read at all, that <c>none</c> is not a marker, and
/// that the marker lands on the end the file means.
/// </para>
/// </remarks>
public sealed class FrameLineEndTests
{
    /// <summary>Both ends are read, with their size attributes.</summary>
    [Fact]
    public void TheStatedMarkersAreRead()
    {
        PageFrame frame = Frame(
            """
            <a:ln>
              <a:solidFill><a:srgbClr val="000000"/></a:solidFill>
              <a:headEnd type="oval" w="sm" len="sm"/>
              <a:tailEnd type="triangle" w="lg" len="med"/>
            </a:ln>
            """);

        frame.HeadEnd.ShouldBe(new LineEnd("oval", "sm", "sm"));
        frame.TailEnd.ShouldBe(new LineEnd("triangle", "lg", "med"));
    }

    /// <summary>A tail alone leaves the head bare, which is much the commonest shape.</summary>
    /// <remarks>353 tails against 255 heads in the corpus, so most lines carry one end only.</remarks>
    [Fact]
    public void AnUnstatedEndCarriesNoMarker()
    {
        PageFrame frame = Frame(
            """
            <a:ln>
              <a:solidFill><a:srgbClr val="000000"/></a:solidFill>
              <a:tailEnd type="triangle"/>
            </a:ln>
            """);

        frame.HeadEnd.Type.ShouldBeNull();
        frame.TailEnd.Type.ShouldBe("triangle");
    }

    /// <summary>
    /// <c>type="none"</c> is written as often as the attribute is omitted, and means the same.
    /// </summary>
    /// <remarks>
    /// Kept apart from an unreadable type on purpose: <c>none</c> is a file saying "no marker
    /// here", so it must come back as no marker rather than as a marker whose shape nothing can
    /// draw — which would leave the shaft shortened for an arrowhead that never appears.
    /// </remarks>
    [Theory]
    [InlineData("""<a:tailEnd type="none"/>""")]
    [InlineData("<a:tailEnd/>")]
    public void AnEndOfNoneIsNoMarker(string end) =>
        Frame($"""
              <a:ln>
                <a:solidFill><a:srgbClr val="000000"/></a:solidFill>
                {end}
              </a:ln>
              """).TailEnd.Type.ShouldBeNull();

    /// <summary>A shape with no outline at all carries no markers either.</summary>
    /// <remarks>
    /// The markers are filled with the line's own paint, so an unstroked shape has nothing to draw
    /// one in. Reading them anyway would put a black arrowhead on a line the file does not draw.
    /// </remarks>
    [Fact]
    public void AShapeWithNoOutlineCarriesNoMarkers()
    {
        PageFrame frame = Frame("""<a:ln><a:tailEnd type="triangle"/></a:ln>""");

        frame.BorderColour.ShouldBeNull();
        frame.TailEnd.Type.ShouldBeNull();
    }

    /// <summary>
    /// A marker shortens the shaft it sits on, so the line stops short of its own end.
    /// </summary>
    /// <remarks>
    /// The overlap is LibreOffice's: the shaft gives up the marker's length less a fifteenth of its
    /// width, "a compromise between straight and peaked markers". Asserted as a bound rather than a
    /// figure because the exact arithmetic belongs to <see cref="LineEnds"/> and is pinned there;
    /// what matters here is that the shortening happens at all.
    /// </remarks>
    [Fact]
    public void AMarkerTakesRoomFromTheShaft()
    {
        Stroke pen = new(Paint.Solid(Colour.FromRgb(0x000000)), Length.FromPoints(1));
        GraphicsPath line = new GraphicsPath()
            .MoveTo(new DocPoint(Length.Zero, Length.Zero))
            .LineTo(new DocPoint(Length.FromPoints(100), Length.Zero));

        (GraphicsPath shaft, List<GraphicsPath> markers) =
            LineEnds.Apply(line, pen, default, new LineEnd("triangle", "med", "med"));

        markers.ShouldHaveSingleItem();
        shaft.Commands[^1].Point.X.ShouldBeLessThan(Length.FromPoints(100));
        shaft.Commands[^1].Point.X.ShouldBeGreaterThan(Length.FromPoints(90));
    }

    private static PageFrame Frame(string line)
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:anchor>
                <wp:extent cx="914400" cy="0"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wps:wsp>
                  <wps:spPr>
                    <a:xfrm><a:off x="0" y="0"/><a:ext cx="914400" cy="0"/></a:xfrm>
                    <a:prstGeom prst="straightConnector1"><a:avLst/></a:prstGeom>
                    {line}
                  </wps:spPr>
                </wps:wsp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        return DocxFrames
            .ReadAll(drawing, null, anchorOffset: 0, pictures: null)
            .ShouldHaveSingleItem();
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
}

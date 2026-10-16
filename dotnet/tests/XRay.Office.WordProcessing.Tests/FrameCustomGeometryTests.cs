using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A shape that writes its own path out — <c>a:custGeom</c> — rather than naming a preset.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="XRay.Office.Ooxml.DrawingML.CustomShapeGeometry"/> has evaluated these for the slide
/// side all along; this side asked only for <c>a:prstGeom</c>, so all <b>124 custom shapes across
/// 21 corpus documents</b> were painted as their bounding rectangles. The storyboard templates are
/// where it shows: their rings came out as squares, and their arrows — which are rotated — as
/// diamonds, because a rotated square is what a diamond looks like.
/// </para>
/// <para>
/// The second half is the fill/stroke split. A subpath states whether it is filled and whether it
/// is stroked, and every connector is one open subpath saying <c>fill="none"</c>: filling the whole
/// outline of one draws a solid blob where the file states a line. That reached the preset path
/// too, which returned a single outline used for both.
/// </para>
/// </remarks>
public sealed class FrameCustomGeometryTests
{
    /// <summary>A stated path is read, and it is the shape's rather than its box.</summary>
    /// <remarks>
    /// A triangle: three points, closed. The rectangle it would otherwise be drawn as has four and
    /// touches all four corners, so the point count and the missing corner both discriminate.
    /// </remarks>
    [Fact]
    public void AStatedPathIsRead()
    {
        PageFrame frame = Frame(Triangle);

        GraphicsPath outline = frame.FillOutline.ShouldNotBeNull();
        outline.Commands.Count(c => c.Verb is PathVerb.MoveTo or PathVerb.LineTo).ShouldBe(3);
    }

    /// <summary>Its points are in the shape's own coordinates, scaled from the path's own space.</summary>
    /// <remarks>
    /// <c>a:path</c> states its own <c>w</c> and <c>h</c>, which are the units the commands are
    /// written in; the shape's extent is what they map onto. Here the path space is 100 × 100 and
    /// the shape is 914400 EMU square, so the apex at <c>(50, 0)</c> lands at half the width.
    /// </remarks>
    [Fact]
    public void ThePathIsScaledToTheShapesExtent()
    {
        IReadOnlyList<PathCommand> commands = Frame(Triangle).FillOutline!.Commands;

        commands[0].Point.X.Emu.ShouldBeInRange(914400 / 2 - 2, (914400 / 2) + 2);
        commands[0].Point.Y.ShouldBe(Length.Zero);
    }

    /// <summary>A shape stating no geometry at all carries no path.</summary>
    [Fact]
    public void AShapeWithNoGeometryCarriesNoPath()
    {
        PageFrame frame = Frame("""<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>""");

        frame.FillOutline.ShouldBeNull();
        frame.StrokeOutline.ShouldBeNull();
    }

    /// <summary>
    /// A subpath saying <c>fill="none"</c> is stroked and not filled.
    /// </summary>
    /// <remarks>
    /// This is every connector in DrawingML, and getting it wrong is not subtle: the shape still
    /// takes a fill from its <c>a:fillRef</c>, so filling the outline paints a solid triangle where
    /// the file states two lines.
    /// </remarks>
    [Fact]
    public void AnUnfilledSubpathIsStrokedOnly()
    {
        PageFrame frame = Frame(Path("""fill="none" stroke="true" """));

        frame.FillOutline!.Commands.ShouldBeEmpty();
        frame.StrokeOutline!.Commands.ShouldNotBeEmpty();
    }

    /// <summary>And one saying <c>stroke="false"</c> is filled and not stroked.</summary>
    /// <remarks>
    /// The shading faces of the presets that fake a third dimension are written this way; stroking
    /// them turns a solid into a wireframe.
    /// </remarks>
    [Fact]
    public void AnUnstrokedSubpathIsFilledOnly()
    {
        PageFrame frame = Frame(Path("""fill="norm" stroke="false" """));

        frame.FillOutline!.Commands.ShouldNotBeEmpty();
        frame.StrokeOutline!.Commands.ShouldBeEmpty();
    }

    /// <summary>A group member states its own geometry the same way, and it is read.</summary>
    /// <remarks>
    /// The storyboard templates' shapes are group members, so a fix at the standalone site alone
    /// would have changed nothing on the documents that prompted it — the same asymmetry that left
    /// <c>a:prstGeom</c> unread inside groups after it was read outside them.
    /// </remarks>
    [Fact]
    public void AGroupMembersPathIsReadToo() =>
        // The group's envelope, then the member.
        Grouped()[1].FillOutline.ShouldNotBeNull();

    private const string Triangle = """
        <a:custGeom>
          <a:pathLst>
            <a:path w="100" h="100">
              <a:moveTo><a:pt x="50" y="0"/></a:moveTo>
              <a:lnTo><a:pt x="100" y="100"/></a:lnTo>
              <a:lnTo><a:pt x="0" y="100"/></a:lnTo>
              <a:close/>
            </a:path>
          </a:pathLst>
        </a:custGeom>
        """;

    private static string Path(string attributes) =>
        $"""
         <a:custGeom>
           <a:pathLst>
             <a:path w="100" h="100" {attributes}>
               <a:moveTo><a:pt x="50" y="0"/></a:moveTo>
               <a:lnTo><a:pt x="100" y="100"/></a:lnTo>
               <a:lnTo><a:pt x="0" y="100"/></a:lnTo>
               <a:close/>
             </a:path>
           </a:pathLst>
         </a:custGeom>
         """;

    private static PageFrame Frame(string geometry) =>
        DocxFrames.ReadAll(
            XElement.Parse(
                $"""
                <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
                  <wp:anchor>
                    <wp:extent cx="914400" cy="914400"/>
                    <wp:wrapNone/>
                    <a:graphic><a:graphicData><wps:wsp>
                      <wps:spPr>
                        <a:xfrm><a:off x="0" y="0"/><a:ext cx="914400" cy="914400"/></a:xfrm>
                        {geometry}
                      </wps:spPr>
                    </wps:wsp></a:graphicData></a:graphic>
                  </wp:anchor>
                </w:drawing>
                """),
            null, anchorOffset: 0, pictures: null)
        .ShouldHaveSingleItem();

    private static IReadOnlyList<PageFrame> Grouped() =>
        DocxFrames.ReadAll(
            XElement.Parse(
                $"""
                <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                           xmlns:wpg="{Wpg}">
                  <wp:anchor>
                    <wp:extent cx="914400" cy="914400"/>
                    <wp:wrapNone/>
                    <a:graphic><a:graphicData><wpg:wgp>
                      <wpg:grpSpPr>
                        <a:xfrm>
                          <a:off x="0" y="0"/><a:ext cx="914400" cy="914400"/>
                          <a:chOff x="0" y="0"/><a:chExt cx="914400" cy="914400"/>
                        </a:xfrm>
                      </wpg:grpSpPr>
                      <wps:wsp>
                        <wps:spPr>
                          <a:xfrm><a:off x="0" y="0"/><a:ext cx="457200" cy="457200"/></a:xfrm>
                          {Triangle}
                        </wps:spPr>
                      </wps:wsp>
                    </wpg:wgp></a:graphicData></a:graphic>
                  </wp:anchor>
                </w:drawing>
                """),
            null, anchorOffset: 0, pictures: null);

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    private const string Wpg = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";
}

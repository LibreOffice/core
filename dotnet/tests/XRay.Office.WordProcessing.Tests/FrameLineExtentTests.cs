using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A shape with no area is not a shape with nothing in it: a straight connector states a zero
/// extent on the axis it does not span, and it is the commonest shape in the corpus that does.
/// </summary>
/// <remarks>
/// <para>
/// The reader asked <c>width &lt;= 0 || height &lt;= 0</c> and dropped the drawing, at both the
/// anchor and the group member. That is the wrong question by exactly one case, and it is the case
/// that matters: a vertical rule is <c>&lt;a:ext cx="0" cy="3834765"/&gt;</c>, so an "or" discards
/// every axis-aligned line in a Word document before anything can decide whether to stroke it. The
/// diagonal in <c>PageDrawing.DrawFrame</c> written to draw such a shape could not have run —
/// nothing ever reached it.
/// </para>
/// <para>
/// Censused over the 271 corpus <c>docx</c>: <b>733 group members across 52 documents</b> have a
/// zero axis, and <b>every one of the 733 is a <c>line</c> or a <c>straightConnector1</c></b> — 640
/// and 93. Not one is a rectangle or a picture, so the guard was keeping nothing else out. Another
/// 94 top-level anchors across 31 documents are the same. The genogram and organogram templates
/// are where it shows: their boxes are joined by nothing but these, so the diagram drew as a grid
/// of captions with no lines between them at all.
/// </para>
/// </remarks>
public sealed class FrameLineExtentTests
{
    /// <summary>A vertical rule — no width, real height — is a frame.</summary>
    [Fact]
    public void AnAnchoredVerticalRuleSurvives()
    {
        PageFrame frame = Anchored("0", "3834765").ShouldHaveSingleItem();

        frame.Size.Width.ShouldBe(Length.Zero);
        frame.Size.Height.ShouldBe(Length.FromEmu(3834765));
    }

    /// <summary>And a horizontal one — no height, real width.</summary>
    /// <remarks>
    /// 22.45 pt rather than the EMU's exact 22.438: every extent this reader takes is snapped to
    /// whole twips, because that is the grid Word states its own measurements on and rounding once
    /// here is what keeps a frame's edge on the same twip as the text beside it.
    /// </remarks>
    [Fact]
    public void AnAnchoredHorizontalRuleSurvives() =>
        Anchored("284966", "0").ShouldHaveSingleItem().Size.Width
            .ShouldBe(Length.FromTwips(Length.FromEmu(284966).Twips));

    /// <summary>A group member states its extent in the group's own space, and the rule holds there too.</summary>
    /// <remarks>
    /// This is where the corpus's 733 actually are: the organogram and genogram connectors sit
    /// inside nested <c>wpg:grpSp</c>, never at the anchor. A fix at the anchor alone would have
    /// looked right on a synthetic file and changed nothing on a real one.
    /// </remarks>
    [Theory]
    [InlineData("0", "3834765")]
    [InlineData("284966", "0")]
    public void AGroupMembersRuleSurvivesToo(string cx, string cy) =>
        // The group's envelope, then the two members: the box and the rule between.
        Grouped(cx, cy).Count.ShouldBe(3);

    /// <summary>An extent that is zero in both axes is still nothing.</summary>
    [Fact]
    public void AnExtentOfNothingIsStillNothing() => Anchored("0", "0").ShouldBeEmpty();

    /// <summary>A negative extent is malformed rather than degenerate, and is refused.</summary>
    /// <remarks>
    /// A line has <em>no area</em>; a negative extent has a right edge left of its left one. Only
    /// the first of those is a shape a file can mean, so widening the guard must not widen to this.
    /// The magnitudes are whole twips because a <c>-1</c> would not test it: every extent is snapped
    /// to twips as it is read, so one EMU of either sign arrives as zero and takes the line branch.
    /// </remarks>
    [Theory]
    [InlineData("-914400", "3834765")]
    [InlineData("284966", "-914400")]
    [InlineData("-914400", "-914400")]
    public void ANegativeExtentIsRefused(string cx, string cy) => Anchored(cx, cy).ShouldBeEmpty();

    private static IReadOnlyList<PageFrame> Anchored(string cx, string cy) =>
        DocxFrames.ReadAll(
            XElement.Parse(
                $"""
                <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
                  <wp:anchor>
                    <wp:extent cx="{cx}" cy="{cy}"/>
                    <wp:wrapNone/>
                    <a:graphic><a:graphicData><wps:wsp>
                      <wps:spPr>
                        <a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
                        <a:prstGeom prst="line"><a:avLst/></a:prstGeom>
                        <a:ln w="6350"><a:solidFill><a:srgbClr val="000000"/></a:solidFill></a:ln>
                      </wps:spPr>
                    </wps:wsp></a:graphicData></a:graphic>
                  </wp:anchor>
                </w:drawing>
                """),
            null, anchorOffset: 0, pictures: null);

    private static IReadOnlyList<PageFrame> Grouped(string cx, string cy) =>
        DocxFrames.ReadAll(
            XElement.Parse(
                $"""
                <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                           xmlns:wpg="{Wpg}">
                  <wp:anchor>
                    <wp:extent cx="1000000" cy="4000000"/>
                    <wp:wrapNone/>
                    <a:graphic><a:graphicData><wpg:wgp>
                      <wpg:grpSpPr>
                        <a:xfrm>
                          <a:off x="0" y="0"/><a:ext cx="1000000" cy="4000000"/>
                          <a:chOff x="0" y="0"/><a:chExt cx="1000000" cy="4000000"/>
                        </a:xfrm>
                      </wpg:grpSpPr>
                      <wps:wsp>
                        <wps:spPr>
                          <a:xfrm><a:off x="0" y="0"/><a:ext cx="500000" cy="500000"/></a:xfrm>
                          <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                        </wps:spPr>
                      </wps:wsp>
                      <wps:wsp>
                        <wps:spPr>
                          <a:xfrm><a:off x="100000" y="600000"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>
                          <a:prstGeom prst="line"><a:avLst/></a:prstGeom>
                          <a:ln w="6350"><a:solidFill><a:srgbClr val="000000"/></a:solidFill></a:ln>
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

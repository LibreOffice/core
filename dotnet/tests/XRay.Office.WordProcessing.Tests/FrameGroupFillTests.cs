using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// <c>a:grpFill</c>: a shape whose fill is the fill of the group it is in.
/// </summary>
/// <remarks>
/// <para>
/// The fifth fill kind is not a fill at all but a reference upwards, and it was read as nothing —
/// so every shape stating one drew unfilled. Censused over the corpus, <b>661 shapes across 14
/// <c>docx</c></b> say it, and they are concentrated rather than scattered: eight genogram
/// templates carry 573 of them between them, which on those documents is most of the ink.
/// </para>
/// <para>
/// A group has no geometry of its own, so its <c>wpg:grpSpPr</c> fill exists only to be inherited.
/// That is why it is resolved on the way down the walk rather than turned into a frame of its own.
/// </para>
/// </remarks>
public sealed class FrameGroupFillTests
{
    /// <summary>A member asking for the group's fill gets it.</summary>
    [Fact]
    public void AMemberTakesTheGroupsFill() =>
        Grouped(group: """<a:solidFill><a:srgbClr val="F8CBAD"/></a:solidFill>""",
                member: "<a:grpFill/>")[1]
            .Fill.ShouldBe(Colour.FromRgb(0xF8CBAD));

    /// <summary>Including when the group's own fill is a gradient.</summary>
    /// <remarks>
    /// The reference is to the fill, not to a colour, so whichever kind the group states is what
    /// the child draws — and a gradient is carried on a different property from a flat colour.
    /// </remarks>
    [Fact]
    public void AGroupsGradientIsInheritedAsAGradient()
    {
        PageFrame member = Grouped(
            group: """
                   <a:gradFill>
                     <a:gsLst>
                       <a:gs pos="0"><a:srgbClr val="FF0000"/></a:gs>
                       <a:gs pos="100000"><a:srgbClr val="0000FF"/></a:gs>
                     </a:gsLst>
                     <a:lin ang="0" scaled="0"/>
                   </a:gradFill>
                   """,
            member: "<a:grpFill/>")[1];

        member.Gradient.ShouldNotBeNull();
        member.Fill.ShouldBeNull();
    }

    /// <summary>A member with a fill of its own keeps it.</summary>
    [Fact]
    public void AStatedFillIsNotReplacedByTheGroups() =>
        Grouped(group: """<a:solidFill><a:srgbClr val="F8CBAD"/></a:solidFill>""",
                member: """<a:solidFill><a:srgbClr val="00FF00"/></a:solidFill>""")[1]
            .Fill.ShouldBe(Colour.FromRgb(0x00FF00));

    /// <summary>
    /// A reference to a group that offers nothing leaves the shape unfilled, and the search stops.
    /// </summary>
    /// <remarks>
    /// It ends the search rather than falling through: a shape saying "use the group's fill" when
    /// the group has none means no fill, and continuing on to its <c>a:fillRef</c> would paint it
    /// the theme's accent instead. <c>oox</c>'s <c>FillProperties</c> stops on <c>XML_grpFill</c>
    /// for the same reason.
    /// </remarks>
    [Fact]
    public void AGroupWithNoFillLeavesTheMemberUnfilledRatherThanThemed()
    {
        PageFrame member = Grouped(group: "", member: "<a:grpFill/>", style: Style)[1];

        member.Fill.ShouldBeNull();
        member.Gradient.ShouldBeNull();
    }

    /// <summary>The chain resolves as far up as it is written.</summary>
    /// <remarks>
    /// A group may itself say <c>a:grpFill</c>, so an inner group passes on what the outer one
    /// offered rather than nothing. The genogram templates nest groups a dozen deep, so a chain
    /// that stopped at one level would have answered for very few of their 117.
    /// </remarks>
    [Fact]
    public void TheChainResolvesThroughANestedGroup()
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                       xmlns:wpg="{Wpg}">
              <wp:anchor>
                <wp:extent cx="914400" cy="914400"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wpg:wgp>
                  <wpg:grpSpPr>
                    {Transform}
                    <a:solidFill><a:srgbClr val="F8CBAD"/></a:solidFill>
                  </wpg:grpSpPr>
                  <wpg:grpSp>
                    <wpg:grpSpPr>{Transform}<a:grpFill/></wpg:grpSpPr>
                    <wps:wsp>
                      <wps:spPr>
                        <a:xfrm><a:off x="0" y="0"/><a:ext cx="457200" cy="457200"/></a:xfrm>
                        <a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom>
                        <a:grpFill/>
                      </wps:spPr>
                    </wps:wsp>
                  </wpg:grpSp>
                </wpg:wgp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        DocxFrames.ReadAll(drawing, null, anchorOffset: 0, pictures: null)[1]
            .Fill.ShouldBe(Colour.FromRgb(0xF8CBAD));
    }

    /// <summary>A shape outside any group that says it is unfilled, not themed.</summary>
    [Fact]
    public void AGroupFillOutsideAGroupIsNoFill()
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:anchor>
                <wp:extent cx="914400" cy="914400"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wps:wsp>
                  <wps:spPr>
                    <a:xfrm><a:off x="0" y="0"/><a:ext cx="914400" cy="914400"/></a:xfrm>
                    <a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom>
                    <a:grpFill/>
                  </wps:spPr>
                  {Style}
                </wps:wsp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        DocxFrames.ReadAll(drawing, null, anchorOffset: 0, pictures: null)
            .ShouldHaveSingleItem().Fill.ShouldBeNull();
    }

    private static IReadOnlyList<PageFrame> Grouped(string group, string member, string style = "") =>
        DocxFrames.ReadAll(
            XElement.Parse(
                $"""
                <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                           xmlns:wpg="{Wpg}">
                  <wp:anchor>
                    <wp:extent cx="914400" cy="914400"/>
                    <wp:wrapNone/>
                    <a:graphic><a:graphicData><wpg:wgp>
                      <wpg:grpSpPr>{Transform}{group}</wpg:grpSpPr>
                      <wps:wsp>
                        <wps:spPr>
                          <a:xfrm><a:off x="0" y="0"/><a:ext cx="457200" cy="457200"/></a:xfrm>
                          <a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom>
                          {member}
                        </wps:spPr>
                        {style}
                      </wps:wsp>
                    </wpg:wgp></a:graphicData></a:graphic>
                  </wp:anchor>
                </w:drawing>
                """),
            null, anchorOffset: 0, pictures: null);

    /// <summary>A style naming a themed fill, to prove the group reference wins over it.</summary>
    private const string Style = """
        <wps:style xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape">
          <a:fillRef idx="1" xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
            <a:schemeClr val="accent1"/>
          </a:fillRef>
        </wps:style>
        """;

    private const string Transform = """
        <a:xfrm>
          <a:off x="0" y="0"/><a:ext cx="914400" cy="914400"/>
          <a:chOff x="0" y="0"/><a:chExt cx="914400" cy="914400"/>
        </a:xfrm>
        """;

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    private const string Wpg = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";
}

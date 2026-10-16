using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A quarter-turned shape inside a group the file scales unevenly: the two scales meet the shape
/// <em>after</em> the turn, and the turn leaves its centre alone.
/// </summary>
/// <remarks>
/// <para>
/// A group child's rectangle is stated in the group's child space and its <c>rot</c> turns it
/// there, so a group whose <c>a:ext</c> and <c>a:chExt</c> differ by different ratios on the two
/// axes meets the shape once it is already turned. Scaling first and turning afterwards stretches
/// the wrong axis, and the two answers differ by the ratio of the scales.
/// </para>
/// <para>
/// Measured on <c>071_Storyboard_Template_Cartoon_Theme</c>, whose picture frames are quarter-turned
/// rectangles in groups scaled 1.000 across and 0.945 down. The frame is 156.9 × 265.0 pt as
/// written; scaled then turned it is 250.3 × 156.9, and turned then scaled it is 265.0 × 148.2 —
/// against pictures 261 × 145, which the reference borders evenly.
/// </para>
/// <para>
/// The centre is the other half and the easier one to get wrong while fixing the first: giving the
/// old top-left the new extent moves the shape by half the difference between the scales, which on
/// that document is 4.3 pt across and 7.3 pt down — enough to slide every frame off its picture.
/// </para>
/// </remarks>
public sealed class FrameTurnedInGroupTests
{
    /// <summary>
    /// The group's scales apply to the axes the shape ends up on.
    /// </summary>
    /// <remarks>
    /// The child is 1 × 2 inches in a child space the group maps at 1.0 across and 0.5 down. Turned
    /// a quarter, its width comes from the vertical scale and its height from the horizontal one, so
    /// the frame is 0.5 × 2 inches — and would be 1 × 1 the other way round.
    /// </remarks>
    [Theory]
    [InlineData("5400000")]
    [InlineData("16200000")]
    [InlineData("-5400000")]
    public void AQuarterTurnedMemberTakesTheScalesTheOtherWayRound(string rot)
    {
        PageFrame member = Member(rot);

        member.Size.Width.ShouldBe(Length.FromEmu(914400 / 2));
        member.Size.Height.ShouldBe(Length.FromEmu(914400 * 2));
    }

    /// <summary>An unturned member takes them the way they are written.</summary>
    [Theory]
    [InlineData(null)]
    [InlineData("0")]
    [InlineData("10800000")]
    public void AnUnturnedMemberTakesThemAsWritten(string? rot)
    {
        PageFrame member = Member(rot);

        member.Size.Width.ShouldBe(Length.FromEmu(914400));
        member.Size.Height.ShouldBe(Length.FromEmu(914400));
    }

    /// <summary>
    /// The turn leaves the shape's centre where it is.
    /// </summary>
    /// <remarks>
    /// This is what a fix for the extent alone gets wrong: keeping the mapped top-left and giving it
    /// the swapped extent moves the centre by half the difference. The child sits at (0,0) and is
    /// 1 × 2 inches, so its centre maps to (half an inch, half an inch) — and the frame's own offsets
    /// must put it there whichever way it is turned.
    /// </remarks>
    [Fact]
    public void TheTurnLeavesTheCentreWhereItIs()
    {
        PageFrame turned = Member("5400000");

        // GroupOffset is the member's top left inside the group's rectangle.
        Length centreX = turned.GroupOffset.X + (turned.Size.Width / 2);
        Length centreY = turned.GroupOffset.Y + (turned.Size.Height / 2);

        centreX.ShouldBe(Length.FromEmu(914400 / 2));
        centreY.ShouldBe(Length.FromEmu(914400 / 2));
    }

    /// <summary>An unturned member's centre is the same point, which is the control.</summary>
    [Fact]
    public void AnUnturnedMembersCentreIsTheSamePoint()
    {
        PageFrame upright = Member(null);

        (upright.GroupOffset.X + (upright.Size.Width / 2)).ShouldBe(Length.FromEmu(914400 / 2));
        (upright.GroupOffset.Y + (upright.Size.Height / 2)).ShouldBe(Length.FromEmu(914400 / 2));
    }

    /// <summary>
    /// A group scaled the same on both axes places a turned member exactly as an unturned one.
    /// </summary>
    /// <remarks>
    /// The whole distinction is the ratio between the two scales, so where there is none the two
    /// paths must agree to the EMU. Worth asserting because a uniform group is the common case and a
    /// bug here would move every rotated shape in the corpus rather than the few this is about.
    /// </remarks>
    [Fact]
    public void AUniformlyScaledGroupPlacesATurnedMemberWhereAnUprightOneGoes()
    {
        PageFrame turned = Member("5400000", childHeight: 1828800, groupHeight: 1828800);
        PageFrame upright = Member(null, childHeight: 1828800, groupHeight: 1828800);

        turned.Size.ShouldBe(upright.Size);
        turned.GroupOffset.ShouldBe(upright.GroupOffset);
    }

    /// <summary>
    /// A group whose child space is 1828800 tall mapped onto <paramref name="groupHeight"/>, holding
    /// one 914400 × 1828800 member at the origin.
    /// </summary>
    private static PageFrame Member(
        string? rot, long childHeight = 1828800, long groupHeight = 914400)
    {
        string attribute = rot is null ? string.Empty : $""" rot="{rot}" """;

        // The anchor states exactly what the member covers, so the group's fit to `wp:extent` —
        // see `FrameGroupExtentFitTests` — is the identity and the child transform is what these
        // assertions read. A turned member covers its rotated box, which is the other way round.
        bool turned = rot is "5400000" or "16200000" or "-5400000";
        long extentWidth = turned ? childHeight : 914400;
        long extentHeight = turned ? 914400 * groupHeight / childHeight : groupHeight;

        return DocxFrames.ReadAll(
            XElement.Parse(
                $"""
                <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                           xmlns:wpg="{Wpg}">
                  <wp:anchor>
                    <wp:extent cx="{extentWidth}" cy="{extentHeight}"/>
                    <wp:wrapNone/>
                    <a:graphic><a:graphicData><wpg:wgp>
                      <wpg:grpSpPr>
                        <a:xfrm>
                          <a:off x="0" y="0"/><a:ext cx="914400" cy="{groupHeight}"/>
                          <a:chOff x="0" y="0"/><a:chExt cx="914400" cy="{childHeight}"/>
                        </a:xfrm>
                      </wpg:grpSpPr>
                      <wps:wsp>
                        <wps:spPr>
                          <a:xfrm{attribute}>
                            <a:off x="0" y="0"/><a:ext cx="914400" cy="{childHeight}"/>
                          </a:xfrm>
                          <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                        </wps:spPr>
                      </wps:wsp>
                    </wpg:wgp></a:graphicData></a:graphic>
                  </wp:anchor>
                </w:drawing>
                """),
            null, anchorOffset: 0, pictures: null)[1];
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    private const string Wpg = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";
}

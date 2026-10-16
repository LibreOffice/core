using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A group's own <c>rot</c>, <c>flipH</c> and <c>flipV</c>, which were read for a shape and not for
/// a group.
/// </summary>
/// <remarks>
/// <para>
/// So every member of a turned or mirrored group was laid out upright and unmirrored. Censused over
/// the corpus, <b>74 groups across 15 <c>docx</c></b> state one — 43 a flip alone, 25 a rotation
/// alone, 6 both — and all 31 rotations are multiples of ninety degrees.
/// </para>
/// <para>
/// <c>055_Organogram_Template_Horizontal_Structure</c> is what it looks like: each of its four rows
/// of connectors is a group turned 90°, and unturned they run down the page through the boxes
/// instead of across between them — one black rule and sixteen arrows pointing the wrong way.
/// Rendered against 26.2.4.2 the nine affected documents' first-page ink falls from a mean of
/// <b>7.90 to 3.87</b>, and <c>052_Organogram_Template_Colorful_Flow_Chart</c> from 21.49 to 1.72.
/// </para>
/// <para>
/// The mark is the top-left quarter of the group; every case below says where that quarter lands.
/// A second member fills the group, so what the members cover is the group's own rectangle and the
/// fit to <c>wp:extent</c> — see <see cref="FrameGroupExtentFitTests"/> — is the identity.
/// </para>
/// </remarks>
public sealed class FrameGroupOrientationTests
{
    /// <summary>A group stating nothing leaves its members where they are written.</summary>
    [Fact]
    public void AnUnturnedGroupLeavesItsMembersAlone()
    {
        PageFrame mark = Mark();

        mark.GroupOffset.ShouldBe(Corner(0, 0));
        mark.Size.ShouldBe(new DocSize(Length.FromEmu(Width / 2), Length.FromEmu(Height / 2)));
        mark.RotationDegrees.ShouldBe(0);
    }

    /// <summary>
    /// A quarter turn moves the top-left quarter to the top right and stands it on its side.
    /// </summary>
    /// <remarks>
    /// The turn is about the group's own rectangle's centre, so the mark's centre — a quarter across
    /// and a quarter down — goes to five eighths across and the top edge. The frame keeps its stated
    /// rectangle and carries the 90°, because a frame is held unturned and turned when it is drawn.
    /// </remarks>
    [Fact]
    public void AQuarterTurnedGroupTurnsItsMembersAboutItsOwnCentre()
    {
        PageFrame mark = Mark(rot: "5400000");

        mark.RotationDegrees.ShouldBe(90);
        mark.Size.ShouldBe(new DocSize(Length.FromEmu(Width / 2), Length.FromEmu(Height / 2)));
        // The mark's centre, a quarter across and a quarter down, turns about the group's own
        // centre to five eighths across and the top edge; the frame's corner is half its width
        // back from there, so three eighths across and a quarter of its height above the top.
        mark.GroupOffset.X.ShouldBe(Length.FromEmu(Width * 3 / 8));
        mark.GroupOffset.Y.ShouldBe(Length.FromEmu(-Height / 4));
    }

    /// <summary>A half turn moves it to the opposite corner and leaves its axes as they were.</summary>
    [Fact]
    public void AHalfTurnedGroupMovesItsMembersToTheOppositeCorner()
    {
        PageFrame mark = Mark(rot: "10800000");

        mark.RotationDegrees.ShouldBe(180);
        mark.GroupOffset.ShouldBe(Corner(Width / 2, Height / 2));
    }

    /// <summary><c>flipH</c> mirrors across and turns nothing.</summary>
    [Fact]
    public void AHorizontallyFlippedGroupMirrorsItsMembersAcross()
    {
        PageFrame mark = Mark(flipH: true);

        mark.GroupOffset.ShouldBe(Corner(Width / 2, 0));
        mark.RotationDegrees.ShouldBe(0);
    }

    /// <summary>And <c>flipV</c> down.</summary>
    [Fact]
    public void AVerticallyFlippedGroupMirrorsItsMembersDown()
    {
        PageFrame mark = Mark(flipV: true);

        mark.GroupOffset.ShouldBe(Corner(0, Height / 2));
        mark.RotationDegrees.ShouldBe(0);
    }

    /// <summary>
    /// A half turn and a horizontal flip is a vertical mirror, and nothing is turned.
    /// </summary>
    /// <remarks>
    /// This is what <c>051_Organogram_Template_Basic_Theme</c> states, and it is the case that makes
    /// a rotation impossible to carry beside the matrix rather than read out of it: adding 180° to
    /// every member would stand each of that diagram's boxes on its head, where the reference draws
    /// them upright and mirrored. Asserted against the plain <c>flipV</c> because the two maps are
    /// the same map.
    /// </remarks>
    [Fact]
    public void AHalfTurnWithAHorizontalFlipIsAVerticalMirror()
    {
        PageFrame both = Mark(rot: "10800000", flipH: true);
        PageFrame mirrored = Mark(flipV: true);

        both.GroupOffset.ShouldBe(mirrored.GroupOffset);
        both.Size.ShouldBe(mirrored.Size);
        both.RotationDegrees.ShouldBe(0);
    }

    /// <summary>A member's own turn adds to the group's.</summary>
    [Fact]
    public void AMembersOwnTurnAddsToTheGroups()
    {
        Mark(rot: "5400000", memberRot: "5400000").RotationDegrees.ShouldBe(180);
        Mark(rot: "10800000", memberRot: "5400000").RotationDegrees.ShouldBe(270);
    }

    /// <summary>
    /// A group turned by anything but a quarter is left upright.
    /// </summary>
    /// <remarks>
    /// It would map the group's rectangles onto parallelograms, which an axis-aligned frame cannot
    /// hold. No corpus document states one, so leaving it is the honest answer rather than a
    /// half-applied approximation that moves every member.
    /// </remarks>
    [Fact]
    public void AGroupTurnedByAnythingElseIsLeftUpright()
    {
        PageFrame mark = Mark(rot: "1800000");

        mark.GroupOffset.ShouldBe(Corner(0, 0));
        mark.RotationDegrees.ShouldBe(0);
    }

    /// <summary>
    /// A group's turn moves a member's text box and leaves the text in it upright.
    /// </summary>
    /// <remarks>
    /// Measured against 26.2.4.2 on a text box inside a group stating <c>rot="10800000"</c>: the box
    /// lands at the opposite corner — which this reader reproduces to the point — and its text is
    /// still drawn upright at the top left, where turning it would stand it on its head.
    /// <c>oox</c>'s <c>lcl_mirrorAtCenter</c> is why: a parent's negative scale becomes the child's
    /// own <c>flipH</c>/<c>flipV</c>, and a half turn decomposes into exactly that pair — two
    /// mirrors, which move a rectangle and leave its text alone.
    /// </remarks>
    [Fact]
    public void AGroupsTurnDoesNotReachItsMembersText()
    {
        PageFrame mark = Mark(rot: "10800000", text: "ABC");

        mark.RotationDegrees.ShouldBe(180);
        mark.TextRotationDegrees.ShouldBe(0);
    }

    /// <summary>
    /// The outermost group's turn is applied to the whole drawing after it has been fitted.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A nested group's orientation is part of the child transform; the outermost group's is not,
    /// because LibreOffice turns that one as an <em>object</em>, once it has been sized to the
    /// anchor. The two orders disagree whenever the turn is a quarter, since the fit is then
    /// stretching a rectangle the turn has stood on its side.
    /// </para>
    /// <para>
    /// Measured against 26.2.4.2: a group turned 90° whose one member fills it puts a centred mark
    /// at <b>275 pt</b> across, exactly where the turn alone leaves it. Fitting afterwards — which
    /// is what composing the orientation into the child transform amounts to — stretches it 2 across
    /// and 0.5 down and lands it at 350. Ten of the corpus's 74 oriented groups are outermost ones,
    /// across five documents.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheOutermostGroupsTurnComesAfterTheFit()
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                       xmlns:wpg="{Wpg}">
              <wp:anchor>
                <wp:extent cx="{Width}" cy="{Height}"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wpg:wgp>
                  <wpg:grpSpPr>
                    <a:xfrm rot="5400000">
                      <a:off x="0" y="0"/><a:ext cx="{Width}" cy="{Height}"/>
                      <a:chOff x="0" y="0"/><a:chExt cx="{Width}" cy="{Height}"/>
                    </a:xfrm>
                  </wpg:grpSpPr>
                  {Square(0, 0, Width, Height)}
                  {Square(Width / 4, 3 * Height / 8, Width / 2, Height / 4)}
                </wpg:wgp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        // The envelope, the filler, then the mark.
        PageFrame mark = DocxFrames.ReadAll(drawing, null, anchorOffset: 0, pictures: null)[2];

        // The mark is at the group's own centre, so the turn leaves it there and only stands it on
        // its side. Fitting afterwards would have moved it and halved it.
        mark.RotationDegrees.ShouldBe(90);
        mark.Size.ShouldBe(new DocSize(Length.FromEmu(Width / 2), Length.FromEmu(Height / 4)));
        mark.GroupOffset.ShouldBe(Corner(Width / 4, 3 * Height / 8));
    }

    /// <summary>A turn inside a turn composes to a half turn.</summary>
    /// <remarks>
    /// The genogram and organogram templates nest a dozen deep, so the composition matters more than
    /// any single level: two quarter turns must come back as a half turn and the member's rectangle
    /// the way it was written, not as a quarter turn applied twice to the same axes.
    /// </remarks>
    [Fact]
    public void ATurnInsideATurnComposes()
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                       xmlns:wpg="{Wpg}">
              <wp:anchor>
                <wp:extent cx="{Width}" cy="{Height}"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wpg:wgp>
                  <wpg:grpSpPr>
                    <a:xfrm>
                      <a:off x="0" y="0"/><a:ext cx="{Width}" cy="{Height}"/>
                      <a:chOff x="0" y="0"/><a:chExt cx="{Width}" cy="{Height}"/>
                    </a:xfrm>
                  </wpg:grpSpPr>
                  <wpg:grpSp>
                    <wpg:grpSpPr>
                      <a:xfrm rot="5400000">
                        <a:off x="0" y="0"/><a:ext cx="{Width}" cy="{Height}"/>
                        <a:chOff x="0" y="0"/><a:chExt cx="{Width}" cy="{Height}"/>
                      </a:xfrm>
                    </wpg:grpSpPr>
                    <wpg:grpSp>
                      <wpg:grpSpPr>
                        <a:xfrm rot="5400000">
                          <a:off x="0" y="0"/><a:ext cx="{Width}" cy="{Height}"/>
                          <a:chOff x="0" y="0"/><a:chExt cx="{Width}" cy="{Height}"/>
                        </a:xfrm>
                      </wpg:grpSpPr>
                      {Square(0, 0, Width / 2, Height / 2)}
                      {Square(0, 0, Width, Height)}
                    </wpg:grpSp>
                  </wpg:grpSp>
                </wpg:wgp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        PageFrame mark = DocxFrames.ReadAll(drawing, null, anchorOffset: 0, pictures: null)[1];

        mark.RotationDegrees.ShouldBe(180);
        mark.Size.ShouldBe(new DocSize(Length.FromEmu(Width / 2), Length.FromEmu(Height / 2)));
    }

    private const long Width = 914400;
    private const long Height = 457200;

    private static DocPoint Corner(long x, long y)
        => new(Length.FromEmu(x), Length.FromEmu(y));

    private static string Square(long x, long y, long cx, long cy, string? rot = null, string? text = null)
        => $"""
            <wps:wsp>
              <wps:spPr>
                <a:xfrm{(rot is null ? "" : $""" rot="{rot}" """)}>
                  <a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/>
                </a:xfrm>
                <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
              </wps:spPr>
              {(text is null
                  ? ""
                  : $"<wps:txbx><w:txbxContent><w:p><w:r><w:t>{text}</w:t></w:r></w:p></w:txbxContent></wps:txbx>")}
            </wps:wsp>
            """;

    /// <summary>
    /// The top-left quarter of a <em>nested</em> group the anchor states exactly, with a second
    /// member filling the group so that the fit to <c>wp:extent</c> is the identity.
    /// </summary>
    /// <remarks>
    /// Nested, because a nested group's orientation composes into the child transform while the
    /// outermost group's is applied to the whole drawing after the fit — see
    /// <see cref="TheOutermostGroupsTurnComesAfterTheFit"/>, which is the other half of this.
    /// </remarks>
    private static PageFrame Mark(
        string? rot = null, bool flipH = false, bool flipV = false, string? memberRot = null,
        string? text = null)
    {
        bool onItsSide = rot is "5400000" or "16200000";
        long anchorWidth = onItsSide ? Height : Width;
        long anchorHeight = onItsSide ? Width : Height;

        string orientation =
            (rot is null ? "" : $""" rot="{rot}" """)
            + (flipH ? """ flipH="1" """ : "")
            + (flipV ? """ flipV="1" """ : "");

        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                       xmlns:wpg="{Wpg}">
              <wp:anchor>
                <wp:extent cx="{anchorWidth}" cy="{anchorHeight}"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wpg:wgp>
                  <wpg:grpSpPr>
                    <a:xfrm>
                      <a:off x="0" y="0"/><a:ext cx="{anchorWidth}" cy="{anchorHeight}"/>
                      <a:chOff x="0" y="0"/><a:chExt cx="{anchorWidth}" cy="{anchorHeight}"/>
                    </a:xfrm>
                  </wpg:grpSpPr>
                  <wpg:grpSp>
                    <wpg:grpSpPr>
                      <a:xfrm{orientation}>
                        <a:off x="0" y="0"/><a:ext cx="{Width}" cy="{Height}"/>
                        <a:chOff x="0" y="0"/><a:chExt cx="{Width}" cy="{Height}"/>
                      </a:xfrm>
                    </wpg:grpSpPr>
                    {Square(0, 0, Width / 2, Height / 2, memberRot, text)}
                    {Square(0, 0, Width, Height)}
                  </wpg:grpSp>
                </wpg:wgp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        return DocxFrames.ReadAll(drawing, null, anchorOffset: 0, pictures: null)[1];
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    private const string Wpg = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";
}

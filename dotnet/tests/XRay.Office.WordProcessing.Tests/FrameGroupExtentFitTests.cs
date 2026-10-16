using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A group is as big as what is in it, so Writer resizes it to the anchor's <c>wp:extent</c>.
/// </summary>
/// <remarks>
/// <para>
/// An <c>SdrObjGroup</c> has no size of its own — its rectangle is the union of its members — and
/// the one size a <c>w:drawing</c> actually declares is the anchor's <c>wp:extent</c>. So a file
/// whose members do not fill the child space its <c>a:chExt</c> describes is drawn by LibreOffice
/// larger than the child transform alone gives, by exactly the ratio of the two.
/// </para>
/// <para>
/// Established by probe against 26.2.4.2 rather than by reading, because <c>oox</c>'s own
/// composition — <c>Shape::createAndInsert</c>'s <c>aParentScale / maChSize</c> block — agrees with
/// this reader shape for shape and so cannot be where the difference lives. The probes are in
/// <c>dotnet/probes/words-group-extent-fit/</c>; each varies one thing and each is checkable by hand.
/// </para>
/// <para>
/// Censused over the corpus, 13 group anchors across 10 <c>docx</c> are out by more than 2 per cent:
/// five of the eight <c>Free_Genogram</c> templates, the disease concept map, the unit-circle chart,
/// a storyboard and the management-system manual.
/// </para>
/// </remarks>
public sealed class FrameGroupExtentFitTests
{
    /// <summary>A lone member covering a quarter of its child space is drawn at the full extent.</summary>
    [Fact]
    public void MembersAreGrownUntilTheyFillTheExtent()
    {
        PageFrame member = Grouped(Square(0, 0, ChildWidth / 4, ChildHeight / 4))[1];

        member.Size.Width.ShouldBe(Length.FromEmu(Width));
        member.Size.Height.ShouldBe(Length.FromEmu(Height));
    }

    /// <summary>A member that already fills its child space is left exactly alone.</summary>
    /// <remarks>
    /// The control, and the case nearly every file in the corpus is: the fit must be the identity
    /// there, or it would move every grouped drawing by a rounding step.
    /// </remarks>
    [Fact]
    public void AMemberThatAlreadyFillsItsChildSpaceIsUntouched()
    {
        PageFrame member = Grouped(Square(0, 0, ChildWidth, ChildHeight))[1];

        member.Size.Width.ShouldBe(Length.FromEmu(Width));
        member.Size.Height.ShouldBe(Length.FromEmu(Height));
        member.GroupOffset.X.ShouldBe(Length.Zero);
        member.GroupOffset.Y.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// The fit is to <c>wp:extent</c> and not to the group's own <c>a:ext</c>.
    /// </summary>
    /// <remarks>
    /// The two agree in almost every file, which is why the distinction has to be tested rather
    /// than assumed. Here the group states a quarter of the anchor's area; the reference draws the
    /// member at the anchor's, so <c>a:ext</c> decides the child scale and nothing else.
    /// </remarks>
    [Fact]
    public void TheFitIsToTheAnchorsExtentAndNotTheGroupsOwn()
    {
        PageFrame member =
            Grouped(Square(0, 0, ChildWidth, ChildHeight), groupWidth: Width / 2,
                    groupHeight: Height / 2)[1];

        member.Size.Width.ShouldBe(Length.FromEmu(Width));
        member.Size.Height.ShouldBe(Length.FromEmu(Height));
    }

    /// <summary>It is two factors, one per axis, not one.</summary>
    /// <remarks>
    /// Members covering three quarters of the width and half the height come back stretched 4/3
    /// across and 2 down. A single uniform factor would give one of those on both axes and leave
    /// the drawing the wrong shape.
    /// </remarks>
    [Fact]
    public void TheTwoAxesAreFittedIndependently()
    {
        PageFrame member = Grouped(
            Square(0, 0, ChildWidth / 4, ChildHeight / 4)
            + Square(ChildWidth / 2, ChildHeight / 4, ChildWidth / 4, ChildHeight / 4))[1];

        member.Size.Width.ShouldBe(Length.FromEmu(Width / 3));
        member.Size.Height.ShouldBe(Length.FromEmu(Height / 2));
    }

    /// <summary>It shrinks as readily as it grows.</summary>
    [Fact]
    public void MembersOverflowingTheirChildSpaceAreShrunk()
    {
        PageFrame member = Grouped(Square(0, 0, ChildWidth * 2, ChildHeight * 2))[1];

        member.Size.Width.ShouldBe(Length.FromEmu(Width));
        member.Size.Height.ShouldBe(Length.FromEmu(Height));
    }

    /// <summary>
    /// The corner it grows from is the members' own, not the anchor's.
    /// </summary>
    /// <remarks>
    /// So the drawn content can end up outside the rectangle the anchor reserved, which is what a
    /// group resized about its own snap rectangle does and what the reference was measured doing:
    /// a lone member stated a quarter of the way into its child space stays there and grows right
    /// and down from it.
    /// </remarks>
    [Fact]
    public void TheMembersOwnCornerStaysWhereItIs()
    {
        PageFrame member =
            Grouped(Square(ChildWidth / 2, ChildHeight / 2, ChildWidth / 4, ChildHeight / 4))[1];

        member.GroupOffset.X.ShouldBe(Length.FromEmu(Width / 2));
        member.GroupOffset.Y.ShouldBe(Length.FromEmu(Height / 2));
        member.Size.Width.ShouldBe(Length.FromEmu(Width));
    }

    /// <summary>
    /// What a turned member covers is its rotated box, and the fit reaches it turned.
    /// </summary>
    /// <remarks>
    /// A member filling a child space twice as wide as it is tall, turned a quarter, covers a tall
    /// box rather than a wide one — so the group is half as wide and twice as tall as the stated
    /// rectangles suggest, and the fit stretches it 2 across and 0.5 down. Both factors then reach
    /// the member the other way round, because it is held unturned and turned about its centre when
    /// it is drawn. The reference draws it 400 × 200 pt on the probe; the stated rectangle alone
    /// would give 200 × 400.
    /// </remarks>
    [Fact]
    public void ATurnedMemberIsFittedByWhatItCovers()
    {
        PageFrame member =
            Grouped(Square(0, 0, ChildWidth, ChildHeight, rot: "5400000"))[1];

        member.Size.Width.ShouldBe(Length.FromEmu(Width / 2));
        member.Size.Height.ShouldBe(Length.FromEmu(Height * 2));
    }

    /// <summary>A canvas is a frame of its own and is not fitted.</summary>
    /// <remarks>
    /// A <c>wpc:wpc</c> states no child space at all — its members are in its own coordinates — and
    /// it is a fixed rectangle the author drew into rather than a group taking its size from what
    /// is in it. Sizing it by its contents would move every canvas in the corpus that does not
    /// happen to be full.
    /// </remarks>
    [Fact]
    public void ACanvasIsLeftAlone()
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                       xmlns:wpc="{Wpc}">
              <wp:anchor>
                <wp:extent cx="{Width}" cy="{Height}"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wpc:wpc>
                  {Square(0, 0, Width / 4, Height / 4)}
                </wpc:wpc></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        PageFrame member = DocxFrames.ReadAll(drawing, null, anchorOffset: 0, pictures: null)[1];

        member.Size.Width.ShouldBe(Length.FromEmu(Width / 4));
        member.Size.Height.ShouldBe(Length.FromEmu(Height / 4));
    }

    private const long Width = 914400;
    private const long Height = 457200;
    private const long ChildWidth = Width * 2;
    private const long ChildHeight = Height * 2;

    private static string Square(long x, long y, long cx, long cy, string? rot = null)
        => $"""
            <wps:wsp>
              <wps:spPr>
                <a:xfrm{(rot is null ? "" : $""" rot="{rot}" """)}>
                  <a:off x="{x}" y="{y}"/><a:ext cx="{cx}" cy="{cy}"/>
                </a:xfrm>
                <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                <a:solidFill><a:srgbClr val="FF0000"/></a:solidFill>
              </wps:spPr>
            </wps:wsp>
            """;

    /// <summary>
    /// An anchor <see cref="Width"/> × <see cref="Height"/> holding a group whose child space is
    /// twice that on both axes.
    /// </summary>
    private static IReadOnlyList<PageFrame> Grouped(
        string members, long groupWidth = Width, long groupHeight = Height) =>
        DocxFrames.ReadAll(
            XElement.Parse(
                $"""
                <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}"
                           xmlns:wpg="{Wpg}">
                  <wp:anchor>
                    <wp:extent cx="{Width}" cy="{Height}"/>
                    <wp:wrapNone/>
                    <a:graphic><a:graphicData><wpg:wgp>
                      <wpg:grpSpPr>
                        <a:xfrm>
                          <a:off x="0" y="0"/><a:ext cx="{groupWidth}" cy="{groupHeight}"/>
                          <a:chOff x="0" y="0"/><a:chExt cx="{ChildWidth}" cy="{ChildHeight}"/>
                        </a:xfrm>
                      </wpg:grpSpPr>
                      {members}
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
    private const string Wpc = "http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas";
}

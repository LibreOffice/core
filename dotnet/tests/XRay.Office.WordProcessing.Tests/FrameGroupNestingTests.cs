using System.Text;
using System.Xml.Linq;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// How deep a group may nest before the reader stops descending.
/// </summary>
/// <remarks>
/// <para>
/// The bound was 8, on the stated grounds that "real files nest a group inside a group and stop".
/// They do not. Censused over the corpus, the deepest grouped shape in
/// <c>055_Organogram_Template_Horizontal_Structure</c> sits <b>twelve</b> groups down, and
/// <b>291 shapes across 10 documents</b> are deeper than eight — dropped, with no diagnostic, by a
/// bound that was a guess.
/// </para>
/// <para>
/// It shows as content simply absent, and the arithmetic is exact.
/// <c>002_Free_Genogram_Diagram_Template_Customizable_Format</c> lost 46 shapes that way, among
/// them precisely the 9 <c>ellipse</c> and 9 <c>rect</c> that are the people in the top two
/// generations of its family tree: the reference fills its <c>#D9D9D9</c> 15 times and its
/// <c>#F8CBAD</c> 14, and we filled them 6 and 5.
/// </para>
/// </remarks>
public sealed class FrameGroupNestingTests
{
    /// <summary>A shape twelve groups down is read, which is the deepest the corpus goes.</summary>
    /// <remarks>
    /// Two frames whatever the depth: the outermost group's envelope, which keeps the anchor's
    /// wrap, and the shape. A group is flattened rather than nested — the layout engine places one
    /// rectangle per frame and an inner group's rectangle is fully determined by its parent's — so
    /// the levels in between yield nothing of their own.
    /// </remarks>
    [Theory]
    [InlineData(1)]
    [InlineData(8)]
    [InlineData(9)]
    [InlineData(12)]
    [InlineData(63)]
    public void AShapeDeepInsideNestedGroupsIsStillRead(int depth) =>
        Nested(depth).Count.ShouldBe(2);

    /// <summary>Past the bound the walk gives up rather than the stack doing it for us.</summary>
    /// <remarks>
    /// An <c>XElement</c> tree from a parse cannot cycle, so the walk is finite whatever the bound
    /// says: it guards the stack, not the loop. This asserts it still guards, so that raising it
    /// once does not read as licence to remove it — and the envelope survives, because the group
    /// itself was read before the descent stopped.
    /// </remarks>
    [Fact]
    public void PastTheBoundTheWalkStops() => Nested(200).ShouldHaveSingleItem();

    /// <summary>The shape read at depth is the one the file states, not the envelope.</summary>
    /// <remarks>
    /// The frames a group yields are its envelope followed by its leaves, and an envelope carries
    /// no geometry of its own — so a test that counted frames alone would pass on a reader that
    /// emitted one envelope per level and no shape at all.
    /// </remarks>
    [Fact]
    public void TheDeepShapeIsTheOneTheFileStates() =>
        Nested(12)[^1].Preset.ShouldBe("ellipse");

    /// <summary>A drawing whose one shape sits inside that many nested groups.</summary>
    private static IReadOnlyList<PageFrame> Nested(int depth)
    {
        StringBuilder open = new();
        StringBuilder close = new();

        for (int level = 0; level < depth; level++)
        {
            open.Append(
                """
                <wpg:grpSp>
                  <wpg:grpSpPr>
                    <a:xfrm>
                      <a:off x="0" y="0"/><a:ext cx="914400" cy="914400"/>
                      <a:chOff x="0" y="0"/><a:chExt cx="914400" cy="914400"/>
                    </a:xfrm>
                  </wpg:grpSpPr>
                """);
            close.Append("</wpg:grpSp>");
        }

        XElement drawing = XElement.Parse(
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
                  {open}
                  <wps:wsp>
                    <wps:spPr>
                      <a:xfrm><a:off x="0" y="0"/><a:ext cx="457200" cy="457200"/></a:xfrm>
                      <a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom>
                    </wps:spPr>
                  </wps:wsp>
                  {close}
                </wpg:wgp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        return DocxFrames.ReadAll(drawing, null, anchorOffset: 0, pictures: null);
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    private const string Wpg = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";
}

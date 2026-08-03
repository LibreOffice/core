using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Model;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A <c>wpg:wgp</c> — one anchored drawing holding many shapes — and where its members land.
/// </summary>
/// <remarks>
/// <para>
/// The defect this covers is quiet in exactly the way a corpus is needed to find: the reader took
/// <c>Descendant(placed, "txbxContent")</c>, which is the <em>first</em> text box in the whole drawing,
/// so a group drew one member and lost the rest with no error anywhere. Measured on
/// <c>Press release_EUREKA labels ITEA 3 Cluster.docx</c> in <c>words/batch-007</c>, whose letterhead is
/// nineteen shapes with eighteen text boxes: 798 extractable words against LibreOffice's 823 before,
/// 838 after — the residual being a separate header-slot defect that draws the title block twice.
/// </para>
/// <para>
/// The transform is asserted rather than only the count, because a group whose members all land at the
/// group's origin extracts exactly as well as one laid out correctly and looks like a solid black box.
/// </para>
/// </remarks>
public sealed class FrameGroupTests
{
    /// <summary>
    /// Every member of a group becomes a frame, offset inside the group's own rectangle.
    /// </summary>
    /// <remarks>
    /// The child space here is scaled two to one against the group's rectangle, which is the case that
    /// tells a real transform from an accidental identity: Word writes a group whose <c>a:chExt</c>
    /// matches its <c>a:ext</c> most of the time, and the corpus letterhead does not — it counts its
    /// children in twips inside a rectangle stated in EMUs.
    /// </remarks>
    [Fact]
    public void EveryMemberOfAGroupBecomesAFrame()
    {
        IReadOnlyList<PageFrame> frames = DocxFrames.ReadAll(Drawing(), Blocks, anchorOffset: 0);

        // The envelope, then one frame per member.
        frames.Count.ShouldBe(4);

        frames[0].GroupSize.ShouldBeNull();
        frames[0].Size.Width.ShouldBe(Length.FromEmu(1828800));
        frames[0].Wrap.ShouldBe(TextWrap.Through);

        // chExt is 2000x1000 against an ext of 1828800x914400 EMUs, so one child unit is 914.4 EMUs
        // across and 914.4 down — and the members are stated at 0, 400 and 800 down.
        frames[1].GroupOffset.ShouldBe(new DocPoint(Length.Zero, Length.Zero));
        frames[2].GroupOffset.Y.ShouldBe(Length.FromTwips(Length.FromEmu(365760).Twips));
        frames[3].GroupOffset.Y.ShouldBe(Length.FromTwips(Length.FromEmu(731520).Twips));

        foreach (PageFrame member in frames.Skip(1))
        {
            member.GroupSize.ShouldBe(new DocSize(Length.FromEmu(1828800), Length.FromEmu(914400)));

            // A member draws no hole of its own — the envelope carries the anchor's wrap, so a
            // nineteen-box letterhead narrows the text once rather than nineteen times.
            member.Wrap.ShouldBe(TextWrap.Through);
            member.IsImage.ShouldBeFalse();
            member.Blocks.Count.ShouldBe(1);
        }

        frames.Skip(1).Select(member => ((PageParagraph)member.Blocks[0]).Text)
            .ShouldBe(["one", "two", "three"]);
    }

    /// <summary>
    /// A group's members are aligned by the group, and offset inside it afterwards.
    /// </summary>
    /// <remarks>
    /// The case that a flattening has to get right and the obvious implementation does not: a centred
    /// group is one rectangle centred on the page, so every member moves by the same amount. Centring
    /// each member on its own width instead stacks them.
    /// </remarks>
    [Fact]
    public void ACentredGroupPlacesItsMembersRelativeToTheWholeGroup()
    {
        PageGeometry geometry = new()
        {
            Size = new DocSize(Length.FromPoints(600), Length.FromPoints(800)),
            Margins = PageMargins.Uniform(Length.FromPoints(50)),
        };

        DocRect column = geometry.TextArea;

        PageFrame member = new()
        {
            Size = new DocSize(Length.FromPoints(100), Length.FromPoints(20)),
            GroupSize = new DocSize(Length.FromPoints(200), Length.FromPoints(60)),
            GroupOffset = new DocPoint(Length.FromPoints(30), Length.FromPoints(40)),
            HorizontalOrigin = FrameHorizontalOrigin.PageMargin,
            HorizontalAlignment = FrameHorizontalAlignment.Centre,
            VerticalOrigin = FrameVerticalOrigin.Page,
            VerticalAlignment = FrameVerticalAlignment.Top,
        };

        DocRect placed = FrameLayout.Place(member, geometry, column, Length.Zero);

        // The group is 200 pt wide in a 500 pt text area, so its left edge is at 50 + 150; the member
        // sits 30 pt further in. Centring the member's own 100 pt would have put it at 250.
        placed.X.ShouldBe(Length.FromPoints(230));
        placed.Y.ShouldBe(Length.FromPoints(40));
        placed.Width.ShouldBe(Length.FromPoints(100));
    }

    /// <summary>A drawing that is not a group is still exactly one frame.</summary>
    /// <remarks>
    /// The overwhelming majority of drawings, and what must not move: the single-frame path is the one
    /// every picture and text box in the corpus takes.
    /// </remarks>
    [Fact]
    public void AnUngroupedDrawingIsStillOneFrame()
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:anchor>
                <wp:extent cx="1828800" cy="914400"/>
                <wp:wrapSquare wrapText="bothSides"/>
                <a:graphic><a:graphicData>
                  <wps:wsp><wps:txbx><w:txbxContent><w:p><w:r><w:t>alone</w:t></w:r></w:p></w:txbxContent></wps:txbx></wps:wsp>
                </a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        IReadOnlyList<PageFrame> frames = DocxFrames.ReadAll(drawing, Blocks, anchorOffset: 0);

        frames.Count.ShouldBe(1);
        frames[0].GroupSize.ShouldBeNull();
        frames[0].Wrap.ShouldBe(TextWrap.Both);
        ((PageParagraph)frames[0].Blocks[0]).Text.ShouldBe("alone");
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    private const string Wpg = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";

    /// <summary>A grouped drawing of three text boxes, in the shape Word and LibreOffice both write.</summary>
    private static XElement Drawing()
        => XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}" xmlns:wpg="{Wpg}">
              <wp:anchor>
                <wp:positionH relativeFrom="column"><wp:posOffset>0</wp:posOffset></wp:positionH>
                <wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>
                <wp:extent cx="1828800" cy="914400"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData uri="{Wpg}">
                  <wpg:wgp>
                    <wpg:grpSpPr>
                      <a:xfrm>
                        <a:off x="0" y="0"/><a:ext cx="1828800" cy="914400"/>
                        <a:chOff x="0" y="0"/><a:chExt cx="2000" cy="1000"/>
                      </a:xfrm>
                    </wpg:grpSpPr>
                    {Member(0, "one")}
                    {Member(400, "two")}
                    {Member(800, "three")}
                  </wpg:wgp>
                </a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

    private static string Member(int y, string text)
        => $"""
           <wps:wsp>
             <wps:spPr><a:xfrm><a:off x="0" y="{y}"/><a:ext cx="2000" cy="300"/></a:xfrm></wps:spPr>
             <wps:txbx><w:txbxContent><w:p><w:r><w:t>{text}</w:t></w:r></w:p></w:txbxContent></wps:txbx>
           </wps:wsp>
           """;

    /// <summary>
    /// A text box's content, reduced to the one thing these assertions read.
    /// </summary>
    /// <remarks>
    /// Not the real DOCX walk: what is under test is which boxes are found and where they are put, and
    /// building a whole <see cref="DocxLayoutSource"/> to answer that would test the walk instead.
    /// </remarks>
    private static IReadOnlyList<PageBlock> Blocks(XElement box)
        => [.. box.Descendants()
            .Where(element => element.Name.LocalName == "p")
            .Select(paragraph => new PageParagraph
            {
                Text = string.Concat(
                    paragraph.Descendants()
                        .Where(element => element.Name.LocalName == "t")
                        .Select(element => element.Value)),
                Face = Face,
            })];

    /// <summary>A real face, since a <see cref="PageParagraph"/> requires one.</summary>
    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}

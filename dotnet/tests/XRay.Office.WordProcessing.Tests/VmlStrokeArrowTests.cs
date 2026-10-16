using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A VML outline's arrowheads, which were read for DrawingML and not for VML.
/// </summary>
/// <remarks>
/// <para>
/// <c>LineEnds.Apply</c> has turned a stroked path into a stroke plus a filled marker since the
/// round that read <c>a:headEnd</c>, and <c>DocxVmlFrames</c> set neither
/// <c>PageFrame.HeadEnd</c> nor <c>PageFrame.TailEnd</c>, so a VML connector was drawn as a bare
/// line. VML names the five shapes differently from DrawingML, so the reference translates rather
/// than reads: <c>lclGetDmlArrowType</c> (<c>oox/source/vml/vmlformatting.cxx</c>:599-611).
/// </para>
/// <para>
/// <strong>Reach is one document, and the census that says seventeen is counting the wrong
/// thing.</strong> 177 VML stroke arrows are stated in 17 corpus DOCX — but 16 of those 17 write
/// the same shape as DrawingML in an <c>mc:Choice</c> beside it, and already drew the reference's
/// arrowhead count exactly. Only <c>067_Work_Breakdown_Structure_Template_Gray_Theme</c>, whose
/// connectors are inside a <c>v:group</c> and therefore have no DrawingML twin, moved: 0 → 25
/// arrowhead-sized fills against 26.2.4.2's 25, and its summed unsigned ink 0.068 → 0.035. The
/// other 16 renderings are byte-identical. <c>probes/vmlarrow-r173</c>.
/// </para>
/// <para>
/// <strong>Two residuals that are not this.</strong>
/// <c>061_Nursing_Concept_Map_Template</c> draws none of its four because its connectors are
/// <c>_x0000_t38</c>, a <em>curved</em> connector, which <c>PaintOf</c> does not recognise at all
/// — so the whole line is missing, not just its head. And <c>ABCD-SDE-23-00</c> draws 39 small
/// fills against 14 before this change as well as after.
/// </para>
/// </remarks>
public sealed class VmlStrokeArrowTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string V = "urn:schemas-microsoft-com:vml";

    private static PageFrame Line(string stroke)
    {
        XElement pict = XElement.Parse(
            $"<w:pict xmlns:w=\"{W}\" xmlns:v=\"{V}\">"
            + "<v:line style=\"position:absolute\" from=\"0,0\" to=\"144pt,0\" "
            + $"strokecolor=\"black\">{stroke}</v:line></w:pict>");

        return DocxVmlFrames.ReadAll(pict, 0, null).ShouldHaveSingleItem();
    }

    /// <summary>Each of VML's five arrow names becomes the DrawingML one that draws it.</summary>
    [Theory]
    [InlineData("block", "triangle")]
    [InlineData("classic", "stealth")]
    [InlineData("diamond", "diamond")]
    [InlineData("oval", "oval")]
    [InlineData("open", "arrow")]
    public void EachArrowNameIsTranslated(string vml, string drawingml)
        => Line($"<v:stroke endarrow=\"{vml}\"/>").TailEnd.Type.ShouldBe(drawingml);

    /// <summary>The two ends are read separately and from their own attributes.</summary>
    [Fact]
    public void TheTwoEndsAreSeparate()
    {
        PageFrame frame = Line("<v:stroke startarrow=\"oval\" endarrow=\"block\"/>");

        frame.HeadEnd.Type.ShouldBe("oval");
        frame.TailEnd.Type.ShouldBe("triangle");
    }

    /// <summary>The size words translate too, and each end keeps its own.</summary>
    [Fact]
    public void TheSizeWordsAreTranslated()
    {
        PageFrame frame = Line(
            "<v:stroke startarrow=\"block\" startarrowwidth=\"wide\" startarrowlength=\"short\""
            + " endarrow=\"block\" endarrowwidth=\"narrow\" endarrowlength=\"long\"/>");

        frame.HeadEnd.ShouldBe(new LineEnd("triangle", "lg", "sm"));
        frame.TailEnd.ShouldBe(new LineEnd("triangle", "sm", "lg"));
    }

    /// <summary>A size the outline does not state is left for <c>LineEnds</c> to default.</summary>
    [Fact]
    public void AnUnstatedSizeIsLeftAlone()
    {
        LineEnd end = Line("<v:stroke endarrow=\"block\"/>").TailEnd;

        end.Width.ShouldBeNull();
        end.Length.ShouldBeNull();
    }

    /// <summary>A stated <c>none</c>, an unknown name and no <c>v:stroke</c> all draw nothing.</summary>
    [Theory]
    [InlineData("<v:stroke endarrow=\"none\"/>")]
    [InlineData("<v:stroke endarrow=\"\"/>")]
    [InlineData("<v:stroke endarrow=\"chevron\"/>")]
    [InlineData("<v:stroke/>")]
    [InlineData("")]
    public void NothingElseDrawsAMarker(string stroke)
    {
        PageFrame frame = Line(stroke);

        frame.HeadEnd.ShouldBe(default(LineEnd));
        frame.TailEnd.ShouldBe(default(LineEnd));
    }

    /// <summary>
    /// The attribute is only ever on the <c>v:stroke</c>, unlike the outline's other attributes.
    /// </summary>
    [Fact]
    public void TheShapesOwnAttributeIsNotAnArrow()
    {
        XElement pict = XElement.Parse(
            $"<w:pict xmlns:w=\"{W}\" xmlns:v=\"{V}\">"
            + "<v:line style=\"position:absolute\" from=\"0,0\" to=\"144pt,0\" "
            + "strokecolor=\"black\" endarrow=\"block\"/></w:pict>");

        DocxVmlFrames.ReadAll(pict, 0, null).ShouldHaveSingleItem()
            .TailEnd.ShouldBe(default(LineEnd));
    }

    /// <summary>An outline that is not stroked at all carries no marker either.</summary>
    [Fact]
    public void AnUnstrokedShapeCarriesNoMarker()
    {
        XElement pict = XElement.Parse(
            $"<w:pict xmlns:w=\"{W}\" xmlns:v=\"{V}\">"
            + "<v:line style=\"position:absolute\" from=\"0,0\" to=\"144pt,0\" stroked=\"f\">"
            + "<v:stroke endarrow=\"block\"/></v:line></w:pict>");

        DocxVmlFrames.ReadAll(pict, 0, null).ShouldHaveSingleItem()
            .TailEnd.ShouldBe(default(LineEnd));
    }
}

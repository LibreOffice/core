using System.Xml.Linq;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Where a shape's text sits when the shape is taller than the text — <c>wps:bodyPr/@anchor</c>.
/// </summary>
/// <remarks>
/// <para>
/// A table cell's <c>w:vAlign</c> has had this arithmetic since the table layouter was written, and
/// a frame had none of it: every shape's text sat against the top of its box whatever the file
/// said. It is invisible on a text box sized to its own text, which is why it survived — and
/// unmissable on a shape sized to be a shape.
/// </para>
/// <para>
/// Censused over the 271 corpus <c>docx</c>: <b>132 text-bearing shapes across 20 documents</b> ask
/// for <c>ctr</c>. Eight of the twenty are the Venn diagram templates, whose labels sit in circles
/// two or three times the height of a line — so a label drawn against the top of its circle lands
/// outside the ink it names.
/// </para>
/// </remarks>
public sealed class FrameTextAnchorTests
{
    /// <summary>The three anchors are read.</summary>
    [Theory]
    [InlineData("t", VerticalTextAlignment.Top)]
    [InlineData("ctr", VerticalTextAlignment.Middle)]
    [InlineData("b", VerticalTextAlignment.Bottom)]
    public void TheStatedAnchorIsRead(string anchor, VerticalTextAlignment expected) =>
        Frame($"""<wps:bodyPr anchor="{anchor}"/>""").TextAlignment.ShouldBe(expected);

    /// <summary>An unstated anchor is the top, which is what every format defaults to.</summary>
    [Theory]
    [InlineData("<wps:bodyPr/>")]
    [InlineData("")]
    public void AnUnstatedAnchorIsTheTop(string body) =>
        Frame(body).TextAlignment.ShouldBe(VerticalTextAlignment.Top);

    /// <summary>
    /// <c>just</c> and <c>dist</c> are read as the top rather than as a centre.
    /// </summary>
    /// <remarks>
    /// Both ask for the <em>lines</em> to be spread through the box rather than for the block to be
    /// moved, which is a different mechanism from an anchor; taking them as anything else would
    /// invent a shift the file did not ask for. No corpus document states either, so this is the
    /// arm that says so.
    /// </remarks>
    [Theory]
    [InlineData("just")]
    [InlineData("dist")]
    public void AVerticalJustificationIsNotAnAnchor(string anchor) =>
        Frame($"""<wps:bodyPr anchor="{anchor}"/>""")
            .TextAlignment.ShouldBe(VerticalTextAlignment.Top);

    /// <summary>A frame holding no text carries no anchor to apply.</summary>
    /// <remarks>
    /// A picture or a plain filled shape has no flow to move, and reading a <c>bodyPr</c> it may
    /// still carry would put a value on a frame nothing consults — harmless today and exactly the
    /// sort of thing a later reader mistakes for intent.
    /// </remarks>
    [Fact]
    public void AFrameWithNoTextCarriesNoAnchor() =>
        // No `wps:txbx`, so no flow: the default stands whatever the body says.
        Frame("""<wps:bodyPr anchor="ctr"/>""", text: null)
            .TextAlignment.ShouldBe(VerticalTextAlignment.Top);

    private static PageFrame Frame(string body, string? text = "Label")
    {
        string box = text is null
            ? string.Empty
            : $"""
              <wps:txbx><w:txbxContent><w:p><w:r><w:t>{text}</w:t></w:r></w:p></w:txbxContent></wps:txbx>
              """;

        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:anchor>
                <wp:extent cx="1828800" cy="1828800"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wps:wsp>
                  <wps:spPr>
                    <a:xfrm><a:off x="0" y="0"/><a:ext cx="1828800" cy="1828800"/></a:xfrm>
                    <a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom>
                  </wps:spPr>
                  {box}
                  {body}
                </wps:wsp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        return DocxFrames
            .ReadAll(drawing, _ => [], anchorOffset: 0, pictures: null)
            .ShouldHaveSingleItem();
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
}

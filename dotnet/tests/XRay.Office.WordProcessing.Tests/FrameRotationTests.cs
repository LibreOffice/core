using System.Xml.Linq;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A shape's <c>a:xfrm/@rot</c>, and the separate angle its text is drawn at.
/// </summary>
/// <remarks>
/// <para>
/// Not reading the shape's angle does not present as a shape tilted the wrong way. It presents as a
/// shape in the wrong <em>place</em> and of the wrong kind: an organogram's downward arrows are
/// horizontal connectors turned through 270°, so drawn square they come out as short horizontal
/// dashes beside the boxes rather than as vertical arrows between them. Censused over the 271
/// corpus <c>docx</c>: 298 shapes across 29 documents state a rotation — 128 <c>rect</c>, 122
/// <c>line</c> or <c>straightConnector1</c>, 17 <c>downArrow</c> — and 213 of the 298 are quarter
/// turns.
/// </para>
/// <para>
/// The text is the half that is easy to get wrong in the other direction. <c>wps:bodyPr/@rot</c> is
/// the text's own angle rather than an addition to the shape's, and <b>every one of the 112 rotated
/// text-bearing shapes in the corpus states <c>rot="0"</c></b> — so taking the shape's angle for the
/// text would have been wrong on all 112. The reference settles it independently:
/// <c>025_Unit_Circle_Chart_Cos_and_Sin_Model</c> arranges 32 labels round a circle at 32 different
/// angles and LibreOffice draws every one of them horizontal.
/// </para>
/// </remarks>
public sealed class FrameRotationTests
{
    /// <summary>The angle is read, out of sixtieths of a degree.</summary>
    /// <remarks>
    /// The three quarter turns are the corpus's own: 5400000 is 90°, 16200000 is 270° and appears
    /// 107 times, 10800000 is 180°. The last is a genuinely arbitrary angle from the unit-circle
    /// charts, which is why an integer-degrees reading would not do.
    /// </remarks>
    [Theory]
    [InlineData("5400000", 90.0)]
    [InlineData("16200000", 270.0)]
    [InlineData("10800000", 180.0)]
    [InlineData("20708959", 345.14931666666666)]
    public void TheStatedAngleIsRead(string rot, double expected) =>
        Frame(rot).RotationDegrees.ShouldBe(expected, 1e-9);

    /// <summary>A shape that states none is square to the page.</summary>
    [Theory]
    [InlineData(null)]
    [InlineData("0")]
    [InlineData("")]
    [InlineData("sideways")]
    public void AnUnstatedOrMalformedAngleIsNone(string? rot) =>
        Frame(rot).RotationDegrees.ShouldBe(0);

    /// <summary>A negative angle turns the other way rather than being refused.</summary>
    /// <remarks><c>ST_Angle</c> is signed, and files use it: −90° and 270° are the same shape.</remarks>
    [Fact]
    public void ANegativeAngleTurnsTheOtherWay() =>
        Frame("-5400000").RotationDegrees.ShouldBe(-90.0);

    /// <summary>
    /// The text takes the angle its body states, not the shape's.
    /// </summary>
    /// <remarks>
    /// This is the whole corpus's case: 112 of 112 rotated text-bearing shapes state
    /// <c>rot="0"</c> here, so a reader that made the text follow the shape would draw 112 labels
    /// on the slant that the reference draws flat.
    /// </remarks>
    [Fact]
    public void TheTextTakesItsBodysOwnAngle()
    {
        PageFrame frame = Frame("20708959", body: """<wps:bodyPr rot="0" vert="horz"/>""");

        frame.RotationDegrees.ShouldBe(345.14931666666666, 1e-9);
        frame.TextRotationDegrees.ShouldBe(0);
    }

    /// <summary>A body that states an angle of its own is drawn at that one.</summary>
    [Fact]
    public void ABodysStatedAngleIsUsedWhateverTheShapesIs() =>
        Frame("5400000", body: """<wps:bodyPr rot="16200000"/>""")
            .TextRotationDegrees.ShouldBe(270.0);

    /// <summary>With no body angle stated, the text turns with the shape.</summary>
    /// <remarks>
    /// The ordinary reading of a label on a tilted shape, and what the schema means by the two
    /// being separate: absent is not zero. No corpus document exercises it, which is stated here so
    /// that the next round knows the arm is inference rather than measurement.
    /// </remarks>
    [Theory]
    [InlineData("<wps:bodyPr/>")]
    [InlineData("")]
    public void WithNoBodyAngleTheTextTurnsWithTheShape(string body) =>
        Frame("5400000", body).TextRotationDegrees.ShouldBe(90.0);

    private static PageFrame Frame(string? rot, string body = "")
    {
        string attribute = rot is null ? string.Empty : $""" rot="{rot}" """;

        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:anchor>
                <wp:extent cx="914400" cy="457200"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wps:wsp>
                  <wps:spPr>
                    <a:xfrm{attribute}><a:off x="0" y="0"/><a:ext cx="914400" cy="457200"/></a:xfrm>
                    <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                  </wps:spPr>
                  {body}
                </wps:wsp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        return DocxFrames
            .ReadAll(drawing, null, anchorOffset: 0, pictures: null)
            .ShouldHaveSingleItem();
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
}

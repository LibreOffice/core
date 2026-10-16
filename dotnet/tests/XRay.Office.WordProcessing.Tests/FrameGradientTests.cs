using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A Word shape's <c>a:gradFill</c> is read, and it is carried unplaced so the drawing code can
/// give it the rectangle the layout engine settled on.
/// </summary>
/// <remarks>
/// <para>
/// 44 anchored shapes across 10 corpus <c>docx</c> state a gradient and every one drew as nothing.
/// The reason this matters more than one missing fill is what a gradient is usually <em>for</em> in
/// these files: on <c>020_Project_Timeline_Template_Modern_Theme</c> the unfilled shape is the
/// page-wide background rectangle, and its title, its three milestone captions and their body text
/// are all set in white. Leaving the paper white did not lose a fill, it lost four strings — drawn,
/// correctly positioned, and in the content stream all along.
/// </para>
/// <para>
/// The reading is <see cref="DrawingGradient"/>'s, which is the slide side's, so what these assert
/// is the wiring rather than the DrawingML: that the element is looked for at all, that a themed
/// <c>a:fillRef</c> gradient arrives by the same route as a stated one, and that the placement is
/// deferred rather than guessed.
/// </para>
/// </remarks>
public sealed class FrameGradientTests
{
    /// <summary>The stated gradient's stops and direction reach the frame.</summary>
    /// <remarks>
    /// <c>ang="5400000"</c> is DrawingML's sixtieths of a degree — 90°, straight down the page,
    /// which is the direction the witness document's background runs in.
    /// </remarks>
    [Fact]
    public void AStatedGradientIsRead()
    {
        GradientDescription ramp = Frame(Linear("5400000", "FF0000", "0000FF")).Gradient
            .ShouldNotBeNull();

        ramp.Kind.ShouldBe(GradientKind.Linear);
        ramp.AngleDegrees.ShouldBe(90);
        ramp.Stops.Select(stop => stop.Colour)
            .ShouldBe([Colour.FromRgb(0xFF0000), Colour.FromRgb(0x0000FF)]);
    }

    /// <summary>Its offsets are the file's own <c>pos</c>, as fractions.</summary>
    [Fact]
    public void TheStopPositionsAreRead() =>
        Frame(Linear("0", "FF0000", "0000FF")).Gradient!.Stops.Select(stop => stop.Offset)
            .ShouldBe([0.0, 1.0]);

    /// <summary>A <c>path="circle"</c> is radial, and its <c>a:fillToRect</c> is its centre.</summary>
    /// <remarks>
    /// Eleven of the corpus's 44 are circle paths, so this is not a schema-completeness case.
    /// </remarks>
    [Fact]
    public void APathGradientIsCentredWhereItsFillToRectSaysAndNotAtTheMiddle()
    {
        GradientDescription ramp = Frame(
            """
            <a:gradFill>
              <a:gsLst>
                <a:gs pos="0"><a:srgbClr val="FF0000"/></a:gs>
                <a:gs pos="100000"><a:srgbClr val="0000FF"/></a:gs>
              </a:gsLst>
              <a:path path="circle"><a:fillToRect l="100000" t="100000" r="0" b="0"/></a:path>
            </a:gradFill>
            """).Gradient.ShouldNotBeNull();

        ramp.Kind.ShouldBe(GradientKind.Radial);
        ramp.CentreX.ShouldBe(1.0);
        ramp.CentreY.ShouldBe(1.0);
    }

    /// <summary>
    /// A gradient and a colour are never both set, in either direction.
    /// </summary>
    /// <remarks>
    /// <c>PageFrame.Fill</c> is what an automatic font colour resolves against, and it wants one
    /// colour whatever the shape is painted with. A gradient-filled frame answers that question the
    /// way an unfilled one does, which is what it did before gradients were read at all.
    /// </remarks>
    [Fact]
    public void AGradientFilledFrameStatesNoFlatColour()
    {
        PageFrame frame = Frame(Linear("0", "FF0000", "0000FF"));

        frame.Gradient.ShouldNotBeNull();
        frame.Fill.ShouldBeNull();
    }

    /// <summary>And a flat-filled one states no gradient.</summary>
    [Fact]
    public void AColourFilledFrameStatesNoGradient()
    {
        PageFrame frame = Frame("""<a:solidFill><a:srgbClr val="FF0000"/></a:solidFill>""");

        frame.Fill.ShouldBe(Colour.FromRgb(0xFF0000));
        frame.Gradient.ShouldBeNull();
    }

    /// <summary>
    /// The gradient is carried without a rectangle, and answers for whichever one it is given.
    /// </summary>
    /// <remarks>
    /// This is the whole reason <see cref="GradientDescription"/> exists rather than a
    /// <see cref="GradientPaint"/> on the frame: a paint holds absolute points and a frame does not
    /// know where on the page it lands until the layout engine has placed it. Asserted by placing
    /// the same description in two boxes.
    /// </remarks>
    [Fact]
    public void ThePlacementIsDeferredRatherThanGuessed()
    {
        GradientDescription ramp = Frame(Linear("5400000", "FF0000", "0000FF")).Gradient!;

        GradientPaint near = ramp.Paint(new DocRect(
            Length.Zero, Length.Zero, Length.FromEmu(914400), Length.FromEmu(914400)));
        GradientPaint far = ramp.Paint(new DocRect(
            Length.FromEmu(4572000), Length.FromEmu(4572000),
            Length.FromEmu(914400), Length.FromEmu(914400)));

        // Straight down, so the ramp spans the box's height and is centred on it.
        near.Start.Y.ShouldBe(Length.Zero);
        near.End.Y.ShouldBe(Length.FromEmu(914400));
        far.Start.Y.ShouldBe(Length.FromEmu(4572000));
        far.End.Y.ShouldBe(Length.FromEmu(5486400));
    }

    /// <summary>
    /// A gradient the shape's <c>wps:style</c> names is read by the same code as a stated one.
    /// </summary>
    /// <remarks>
    /// The theme's second and third fill styles are gradients in every theme Office ships, so
    /// before this an <c>a:fillRef idx="2"</c> resolved to nothing while <c>idx="1"</c> resolved to
    /// a colour — the format matrix half-wired.
    /// </remarks>
    [Fact]
    public void AThemedGradientArrivesByTheSameRoute()
    {
        GradientDescription ramp = Frame(
            fill: null,
            style: """
                   <wps:style>
                     <a:lnRef idx="0"><a:schemeClr val="accent1"/></a:lnRef>
                     <a:fillRef idx="2"><a:schemeClr val="accent1"/></a:fillRef>
                   </wps:style>
                   """).Gradient.ShouldNotBeNull();

        ramp.Kind.ShouldBe(GradientKind.Linear);
        ramp.Stops.Count.ShouldBe(2);

        // phClr is accent1, and the theme's own tints act on it — so neither stop is the bare
        // accent, and the two differ, which is what makes it a gradient rather than a flat fill.
        ramp.Stops[0].Colour.ShouldNotBe(ramp.Stops[1].Colour);
    }

    /// <summary>A gradient with no readable stop is no fill rather than a black one.</summary>
    [Fact]
    public void AGradientWithNoColoursIsNoFill() =>
        Frame("<a:gradFill><a:gsLst/></a:gradFill>").Gradient.ShouldBeNull();

    private static string Linear(string angle, string from, string to) =>
        $"""
         <a:gradFill>
           <a:gsLst>
             <a:gs pos="0"><a:srgbClr val="{from}"/></a:gs>
             <a:gs pos="100000"><a:srgbClr val="{to}"/></a:gs>
           </a:gsLst>
           <a:lin ang="{angle}" scaled="0"/>
         </a:gradFill>
         """;

    private static PageFrame Frame(string? fill, string style = "")
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
                    <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                    {fill}
                  </wps:spPr>
                  {style}
                </wps:wsp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        return DocxFrames
            .ReadAll(drawing, null, anchorOffset: 0, pictures: null,
                     new DocxFrameContext(DrawingTheme.Read(Theme), CompatibilityMode: 15)
                     {
                         Styles = DrawingStyleMatrix.Read(Theme),
                     })
            .ShouldHaveSingleItem();
    }

    /// <summary>The stock Office theme, cut to the two lists a shape's style indexes into.</summary>
    private static readonly XElement Theme = XElement.Parse(
        $"""
        <a:theme xmlns:a="{A}">
          <a:themeElements>
            <a:clrScheme name="Office">
              <a:accent1><a:srgbClr val="5B9BD5"/></a:accent1>
            </a:clrScheme>
            <a:fmtScheme name="Office">
              <a:fillStyleLst>
                <a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
                <a:gradFill rotWithShape="1"><a:gsLst>
                  <a:gs pos="0"><a:schemeClr val="phClr"><a:tint val="67000"/></a:schemeClr></a:gs>
                  <a:gs pos="100000"><a:schemeClr val="phClr"><a:shade val="88000"/></a:schemeClr></a:gs>
                </a:gsLst><a:lin ang="5400000" scaled="0"/></a:gradFill>
              </a:fillStyleLst>
              <a:lnStyleLst>
                <a:ln w="6350"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
              </a:lnStyleLst>
            </a:fmtScheme>
          </a:themeElements>
        </a:theme>
        """);

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
}

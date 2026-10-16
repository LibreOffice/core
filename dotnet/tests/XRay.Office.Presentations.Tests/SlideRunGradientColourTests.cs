using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.Ooxml;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A run whose colour is stated as a fill rather than as a colour.
/// </summary>
/// <remarks>
/// <para>
/// <b>DrawingML lets a run carry any fill a shape can, and text is drawn with one colour.</b>
/// LibreOffice reduces whichever fill it finds to a single colour on the way in —
/// <c>FillProperties::getBestSolidColor</c> (<c>oox/source/drawingml/fillproperties.cxx:402</c>)
/// called from <c>TextCharacterProperties::pushToPropMap</c>
/// (<c>oox/source/drawingml/textcharacterproperties.cxx:115</c>), which sets <c>PROP_CharColor</c>
/// and then <c>PROP_CharTransparence</c> from that colour's alpha. Reading only
/// <c>a:solidFill</c> left every gradient-filled run with no colour of its own, so it inherited
/// whatever was above it in the chain and came out opaque black.
/// </para>
/// <para>
/// <b>The alpha is what makes it visible.</b> Measured on
/// <c>slides/batch-012/pptx/OnTrac_StarCertificationProgram-3Day.pptx</c>, whose
/// <c>slideMaster3.xml</c> draws an 82 pt background page number from a <c>defRPr</c> whose
/// <c>a:gradFill</c> is two identical <c>tx1</c> stops at <c>a:alpha val="10000"</c>: the
/// reference emits <c>0 0 0 rg</c> inside a transparency group under <c>/CA 0.1 /ca 0.1</c>, on
/// 12 of the deck's 15 pages. It is black at a tenth opacity, not a grey — the two agree only
/// over a white background, and this one is over a photograph.
/// </para>
/// <para>
/// <b>Which stop is not a matter of taste.</b> LibreOffice's stops are a map keyed by position,
/// and it takes the first — the lowest position, not the first in the file — unless there are
/// more than two, in which case it takes the second. <c>DrawingChartPlot.FillOf</c> implements a
/// different rule, nearest to the middle, deliberately and for a chart series; copying it here
/// would disagree with the reference on every three-stop run.
/// </para>
/// </remarks>
public class SlideRunGradientColourTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static XElement Body(string fill) => XElement.Parse(
        $"""
         <a:txBody xmlns:a="{A}">
           <a:bodyPr/>
           <a:p>
             <a:r><a:rPr lang="en-GB" sz="2400">{fill}</a:rPr><a:t>1</a:t></a:r>
           </a:p>
         </a:txBody>
         """);

    private static Colour ColourOf(string fill)
        => PptxTextBody.Read(Body(fill)).Paragraphs[0].Runs[0].Colour;

    /// <summary>The `OnTrac` declaration itself: two identical stops, both at 10% alpha.</summary>
    [Fact]
    public void TwoIdenticalStopsAtOneTenthAlphaAreBlackAtOneTenthAlpha()
    {
        Colour colour = ColourOf(
            """
            <a:gradFill><a:gsLst>
              <a:gs pos="0"><a:srgbClr val="000000"><a:alpha val="10000"/></a:srgbClr></a:gs>
              <a:gs pos="100000"><a:srgbClr val="000000"><a:alpha val="10000"/></a:srgbClr></a:gs>
            </a:gsLst><a:lin ang="5400000" scaled="0"/></a:gradFill>
            """);

        colour.WithAlpha(255).ShouldBe(Colour.FromRgb(0x000000));

        // 10% of 255 is 25.5; the byte it lands on writes /ca 0.102, which is the reference's
        // 0.1 to within one 255th and is where that value comes from.
        ((double)colour.A).ShouldBe(26, 1);
    }

    /// <summary>An <c>a:solidFill</c> still wins outright, and still carries its own alpha.</summary>
    [Fact]
    public void ASolidFillIsUnaffected()
    {
        Colour colour = ColourOf(
            """<a:solidFill><a:srgbClr val="FF0000"><a:alpha val="50000"/></a:srgbClr></a:solidFill>""");

        colour.WithAlpha(255).ShouldBe(Colour.FromRgb(0xFF0000));
        ((double)colour.A).ShouldBe(128, 1);
    }

    /// <summary>
    /// Two stops take the first, three take the second — LibreOffice's rule, and the reason the
    /// stops are ordered by position rather than by document order first.
    /// </summary>
    [Theory]
    [InlineData("""<a:gs pos="0"><a:srgbClr val="112233"/></a:gs><a:gs pos="100000"><a:srgbClr val="445566"/></a:gs>""", 0x112233)]
    [InlineData("""<a:gs pos="100000"><a:srgbClr val="445566"/></a:gs><a:gs pos="0"><a:srgbClr val="112233"/></a:gs>""", 0x112233)]
    [InlineData("""<a:gs pos="0"><a:srgbClr val="112233"/></a:gs><a:gs pos="50000"><a:srgbClr val="445566"/></a:gs><a:gs pos="100000"><a:srgbClr val="778899"/></a:gs>""", 0x445566)]
    public void TheStopIsTheFirstUnlessThereAreMoreThanTwo(string stops, int expected)
        => ColourOf($"<a:gradFill><a:gsLst>{stops}</a:gsLst></a:gradFill>")
            .WithAlpha(255)
            .ShouldBe(Colour.FromRgb((uint)expected));

    /// <summary>
    /// A run stating no fill states nothing, and a body with no chain above it falls back to
    /// opaque black. That is the state every gradient-filled run used to be in, so it is worth
    /// pinning: the fix must move the gradient case off this answer and leave this one on it.
    /// </summary>
    [Fact]
    public void ARunWithNoFillIsOpaqueBlack() => ColourOf("").ShouldBe(Colour.Black);

    /// <summary>
    /// An empty gradient is a file's error. It must fall through to the chain rather than be read
    /// as a colour — and, more sharply, rather than index a stop that is not there.
    /// </summary>
    [Fact]
    public void AGradientWithNoStopsFallsThrough()
        => ColourOf("<a:gradFill><a:gsLst/></a:gradFill>").ShouldBe(Colour.Black);
}

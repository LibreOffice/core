using System.Xml.Linq;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>v:line</c> states its extent in <c>from</c>/<c>to</c>, and its <c>style</c> rectangle is
/// not read at all.
/// </summary>
/// <remarks>
/// <para>
/// <c>DocxVmlFrames.Floating</c> required a <c>style</c> width and height, and every top-level
/// <c>v:line</c> in the corpus states neither — so every one returned null and no rule was drawn.
/// <c>LineShape::getAbsRectangle</c> (<c>oox/source/vml/vmlshape.cxx</c>) builds the rectangle out
/// of the two endpoints instead: <c>X = from.x</c>, <c>Y = from.y</c>,
/// <c>Width = to.x − X</c>, <c>Height = to.y − Y</c>.
/// </para>
/// <para>
/// <strong>Every value here is measured against 26.2.4.2</strong>, on eighteen one-attribute
/// probes whose line is read out of the PDF's own path operators against a marker rectangle that
/// fixes where the anchor's origin landed — `probes/vline-r145`. This tree reproduces all
/// eighteen, the group cases within 0.05 pt and the rest exactly.
/// </para>
/// <para>
/// <strong>The reach is two documents and it is not a small mark in either.</strong> Of the
/// corpus's 865 <c>v:line</c>, 859 are the VML half of an <c>mc:AlternateContent</c> whose
/// <c>mc:Choice</c> requires <c>wps</c> or <c>wpg</c>, so neither renderer reads them —
/// round 140's "150 in 36 documents" counts markup nobody looks at. What is left is a 180 pt
/// footer rule on <c>JEMIT_Template.docx</c> and a 468 pt header rule on <b>all 47 pages</b> of
/// <c>33004.docx</c>, and the second is now drawn at <c>x 72.00, y 63.35, w 468.00</c> against
/// the reference's identical figures.
/// </para>
/// </remarks>
public sealed class VmlLineExtentTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string V = "urn:schemas-microsoft-com:vml";

    private static XElement Pict(string inner)
        => XElement.Parse($"<w:pict xmlns:w=\"{W}\" xmlns:v=\"{V}\">{inner}</w:pict>");

    private static PageFrame? Line(string attributes)
        => DocxVmlFrames.ReadAll(
            Pict($"<v:line style=\"position:absolute\" strokecolor=\"#FF0000\" "
                 + $"strokeweight=\"2pt\" {attributes}/>"),
            0,
            null).SingleOrDefault();

    [Fact]
    public void TheBoxComesFromTheTwoEndpoints()
    {
        PageFrame frame = Line("from=\"0,0\" to=\"144pt,0\"").ShouldNotBeNull();

        frame.HorizontalOffset.Points.ShouldBe(0, 0.01);
        frame.VerticalOffset.Points.ShouldBe(0, 0.01);
        frame.Size.Width.Points.ShouldBe(144, 0.01);
        frame.Size.Height.Points.ShouldBe(0, 0.01);
        frame.IsLine.ShouldBeTrue("a v:line paints the box's diagonal, not its four sides");
    }

    [Theory]
    // The reference draws all four of these in exactly the same place, which is what says the
    // style rectangle is not consulted where `from`/`to` are present.
    [InlineData("left:100pt;top:50pt")]
    [InlineData("width:200pt;height:100pt")]
    [InlineData("left:100pt;top:50pt;width:200pt;height:100pt")]
    public void AStatedStyleRectangleIsIgnored(string extra)
    {
        PageFrame frame = DocxVmlFrames.ReadAll(
            Pict($"<v:line style=\"position:absolute;{extra}\" strokecolor=\"#FF0000\" "
                 + "from=\"0,0\" to=\"144pt,0\"/>"),
            0,
            null).ShouldHaveSingleItem();

        frame.HorizontalOffset.Points.ShouldBe(0, 0.01);
        frame.VerticalOffset.Points.ShouldBe(0, 0.01);
        frame.Size.Width.Points.ShouldBe(144, 0.01);
        frame.Size.Height.Points.ShouldBe(0, 0.01);
    }

    [Fact]
    public void ABareNumberIsAPixelAtNinetySixDotsPerInchAndNotAPoint()
    {
        // `decodeMeasureToHmm(…, bDefaultAsPixel: true)`. The probe stating `to="96,48"` is drawn
        // 72 pt across and 36 pt down, which is what separates this from `Css`, whose bare number
        // is a point and which serves the `style` properties this round did not measure.
        PageFrame frame = Line("from=\"0,0\" to=\"96,48\"").ShouldNotBeNull();

        frame.Size.Width.Points.ShouldBe(72, 0.01);
        frame.Size.Height.Points.ShouldBe(36, 0.01);
    }

    [Fact]
    public void AUnitSuffixIsHonoured()
    {
        PageFrame frame = Line("from=\"1in,0.5in\" to=\"3in,0.5in\"").ShouldNotBeNull();

        frame.HorizontalOffset.Points.ShouldBe(72, 0.01);
        frame.VerticalOffset.Points.ShouldBe(36, 0.01);
        frame.Size.Width.Points.ShouldBe(144, 0.01);
    }

    [Fact]
    public void ALineWhoseXDecreasesIsNotDrawnAndOneWhoseYDecreasesIs()
    {
        // `getAbsRectangle` subtracts without normalising, so a backwards line has a negative
        // width and 26.2.4.2 draws nothing for it — measured, on two probes that come back with
        // no ink at all. Normalising the endpoints into a rectangle would draw three lines where
        // the reference draws two, so the asymmetry is reproduced rather than tidied away.
        Line("from=\"144pt,0\" to=\"0,0\"").ShouldBeNull();
        Line("from=\"144pt,72pt\" to=\"0,0\"").ShouldBeNull();

        PageFrame up = Line("from=\"0,72pt\" to=\"144pt,0\"").ShouldNotBeNull();

        up.VerticalOffset.Points.ShouldBe(0, 0.01);
        up.Size.Height.Points.ShouldBe(72, 0.01);
        up.IsLineMirrored.ShouldBeTrue("the reference draws it as the box's anti-diagonal");
    }

    [Fact]
    public void AZeroHeightLineIsStillDrawn()
    {
        // Every horizontal rule in a header is a box exactly as tall as nothing, and the
        // zero-extent guard that keeps an empty shape out has to let this one through.
        Line("from=\"0,0\" to=\"468pt,0\"").ShouldNotBeNull();
        Line("from=\"0,0\" to=\"0,468pt\"").ShouldNotBeNull();
    }

    [Fact]
    public void AnUnstrokedLinePaintsNothing()
        => DocxVmlFrames.ReadAll(
                Pict("<v:line style=\"position:absolute\" from=\"0,0\" to=\"144pt,0\" "
                     + "strokecolor=\"#FF0000\" stroked=\"f\"/>"),
                0,
                null)
            .ShouldHaveSingleItem()
            .BorderColour.ShouldBeNull();

    [Fact]
    public void ASystemColourNameResolvesRatherThanPaintingNothing()
    {
        // `ConversionHelper::decodeColor` falls through the preset table to
        // `GraphicHelper::getSystemColor`, whose palette is a fixed table rather than the running
        // desktop's. `JEMIT_Template.docx`'s footer rule is the corpus's only
        // `strokecolor="windowText"`, and an unresolved name left it with no stroke at all.
        DocxVmlFrames.ReadAll(
                Pict("<v:line style=\"position:absolute\" from=\"0,0\" to=\"144pt,0\" "
                     + "strokecolor=\"windowText\" strokeweight=\".25pt\"/>"),
                0,
                null)
            .ShouldHaveSingleItem()
            .BorderColour.ShouldNotBeNull()
            .ToString().ShouldBe("#000000");
    }

    [Fact]
    public void InsideAGroupTheNumbersAreTheGroupsOwnCoordinateSpace()
    {
        // `LineShape::getRelRectangle` reads each token with `o3tl::toInt32`, which takes the
        // leading integer and drops any unit suffix — so `100pt` is a hundred units, not a
        // length. The group is 200 x 100 pt with coordsize 1000,1000.
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:group style=\"position:absolute;left:0;top:0;width:200pt;height:100pt\" "
                 + "coordsize=\"1000,1000\" coordorigin=\"0,0\">"
                 + "<v:line style=\"position:absolute\" from=\"250,250\" to=\"750,250\" "
                 + "strokecolor=\"#FF0000\"/></v:group>"),
            0,
            null);

        PageFrame frame = frames.ShouldHaveSingleItem();
        frame.HorizontalOffset.Points.ShouldBe(50, 0.05);
        frame.VerticalOffset.Points.ShouldBe(25, 0.05);
        frame.Size.Width.Points.ShouldBe(100, 0.05);
        frame.Size.Height.Points.ShouldBe(0, 0.05);
    }

    [Fact]
    public void AUnitSuffixInsideAGroupIsDroppedRatherThanConverted()
    {
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:group style=\"position:absolute;left:0;top:0;width:200pt;height:100pt\" "
                 + "coordsize=\"1000,1000\" coordorigin=\"0,0\">"
                 + "<v:line style=\"position:absolute\" from=\"0,0\" to=\"100pt,50pt\" "
                 + "strokecolor=\"#FF0000\"/></v:group>"),
            0,
            null);

        // 100 units of 0.2 pt and 50 of 0.1 pt, which is what 26.2.4.2 draws.
        PageFrame frame = frames.ShouldHaveSingleItem();
        frame.Size.Width.Points.ShouldBe(20, 0.05);
        frame.Size.Height.Points.ShouldBe(5, 0.05);
    }
}

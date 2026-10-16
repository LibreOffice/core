using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A VML <c>#_x0000_t136</c> shape carrying a <c>v:textpath</c> is WordArt and is drawn as curves.
/// </summary>
/// <remarks>
/// <para>
/// The corpus holds fifteen of these — the diagonal <c>EASA Example Documents</c> and <c>DRAFT</c>
/// watermarks on five aviation documents, every one of them in a <c>word/headerN.xml</c> and none
/// of them inside an <c>mc:Fallback</c>. Until this existed the reader drew nothing at all for
/// them, under a rule that a VML shape stating a geometry this cannot build is left unpainted
/// rather than painted as its rectangle.
/// </para>
/// <para>
/// <strong>The trap on this path is the other way round.</strong>
/// <c>WordArt_Shapes_Arrows_Catalog1.docx</c> holds <em>zero</em> <c>_x0000_t136</c> and 77
/// <c>v:textpath</c> hits, and every one of those is the <c>mc:Fallback</c> half of a DrawingML
/// shape whose <c>mc:Choice</c> is what the reference renders. Reading fallback content would draw
/// that whole catalogue twice. <c>OoxmlXml</c> resolves <c>mc:AlternateContent</c> to its chosen
/// branch before any of this runs, which is what keeps it safe, and the catalogue's 52 pages and
/// 2468 extracted words are the standing check on it.
/// </para>
/// </remarks>
public sealed class VmlFontworkTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string V = "urn:schemas-microsoft-com:vml";
    private const string O = "urn:schemas-microsoft-com:office:office";

    /// <summary>The shape type Word writes ahead of a WordArt shape, from a corpus header verbatim.</summary>
    private const string ShapeType =
        "<v:shapetype id=\"_x0000_t136\" coordsize=\"21600,21600\" o:spt=\"136\" adj=\"10800\""
        + " path=\"m@7,l@8,m@5,21600l@6,21600e\"><v:formulas>"
        + "<v:f eqn=\"sum #0 0 10800\"/><v:f eqn=\"prod #0 2 1\"/><v:f eqn=\"sum 21600 0 @1\"/>"
        + "<v:f eqn=\"sum 0 0 @2\"/><v:f eqn=\"sum 21600 0 @3\"/><v:f eqn=\"if @0 @3 0\"/>"
        + "<v:f eqn=\"if @0 21600 @1\"/><v:f eqn=\"if @0 0 @2\"/><v:f eqn=\"if @0 @4 21600\"/>"
        + "</v:formulas><v:path textpathok=\"t\"/><v:textpath on=\"t\" fitshape=\"t\"/></v:shapetype>";

    private const string Watermark =
        "<v:shape id=\"wm\" type=\"#_x0000_t136\" style=\"position:absolute;margin-left:0;"
        + "margin-top:0;width:583.25pt;height:53pt;rotation:315;z-index:-251655168;"
        + "mso-position-horizontal:center;mso-position-horizontal-relative:margin;"
        + "mso-position-vertical:center;mso-position-vertical-relative:margin\""
        + " fillcolor=\"silver\" stroked=\"f\"><v:fill opacity=\".5\"/>"
        + "<v:textpath style=\"font-family:&quot;Arial&quot;;font-size:1pt\""
        + " string=\"EASA example document\"/></v:shape>";

    /// <summary>The whole watermark: curves, silver at half opacity, unstroked, and turned 315°.</summary>
    [Fact]
    public void AWatermarkIsDrawnAsWarpedCurves()
    {
        PageFrame frame = One(ShapeType + Watermark);

        frame.FillOutline.ShouldNotBeNull("the reference replaces the shape with filled outlines");
        frame.StrokeOutline.ShouldBeSameAs(frame.FillOutline);
        frame.IsImage.ShouldBeFalse();

        // `fillcolor="silver"` is #C0C0C0 and `<v:fill opacity=".5"/>` halves its alpha.
        frame.Fill.ShouldNotBeNull();
        frame.Fill!.Value.R.ShouldBe((byte)0xC0);
        frame.Fill!.Value.A.ShouldBe((byte)128);
        frame.BorderColour.ShouldBeNull("the shape states stroked=\"f\"");

        frame.RotationDegrees.ShouldBe(315);
        frame.HorizontalAlignment.ShouldBe(FrameHorizontalAlignment.Centre);
        frame.VerticalAlignment.ShouldBe(FrameVerticalAlignment.Middle);
        frame.HorizontalOrigin.ShouldBe(FrameHorizontalOrigin.PageMargin);
        frame.VerticalOrigin.ShouldBe(FrameVerticalOrigin.PageMargin);
    }

    /// <summary>
    /// The declared height is thrown away and remeasured from the text, unless <c>trim</c> says not to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>TextpathModel::pushToPropMap</c>, <c>oox/source/vml/vmlformatting.cxx:1041-1056</c>, and it
    /// is the single largest thing a reader of this path can get wrong: the corpus's watermarks state
    /// <c>height:53pt</c> and <c>height:247.45pt</c> and the reference imports 57.5 and 138. Probed
    /// against the reference on five isolated (family, string) pairs, the ratio of <c>hhea</c>'s
    /// ascender less its descender to the sum of the design advances predicts the height LibreOffice
    /// arrives at to within 0.9%.
    /// </para>
    /// <para>
    /// <c>trim</c> has no default: <c>lclDecodeBool</c> yields nothing for an absent attribute and the
    /// test is <c>has_value() &amp;&amp; value()</c>, so unstated means resize. Reading it as "absent
    /// means true" leaves every watermark at its declared height — which on this document is close
    /// enough to look right and is 4.5 pt out.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheHeightIsRemeasuredFromTheTextUnlessTrimSaysOtherwise()
    {
        PageFrame resized = One(ShapeType + Watermark);
        PageFrame kept = One(ShapeType + Watermark.Replace(
            "<v:textpath style=", "<v:textpath trim=\"t\" style=", StringComparison.Ordinal));

        resized.Size.Width.ShouldBe(Length.FromPoints(583.25));
        kept.Size.ShouldBe(new Core.Geometry.DocSize(
            Length.FromPoints(583.25), Length.FromPoints(53)));

        // Liberation Sans: (1854 + 434) / 23337 of the width, which is 57.15 pt against the
        // reference's own 57.5.
        double points = resized.Size.Height.Emu / (double)Length.FromPoints(1).Emu;
        points.ShouldBeInRange(56.6, 58.0);
    }

    /// <summary>
    /// A shape type outside the WordArt range is not WordArt, whatever <c>v:textpath</c> it carries.
    /// </summary>
    /// <remarks>
    /// The range is <c>mso_sptTextPlainText</c> (136) to <c>mso_sptTextCanDown</c> (175),
    /// <c>include/svx/msdffdef.hxx:412-451</c>. A <c>v:shapetype</c> writes a stray
    /// <c>v:textpath</c> child freely — the corpus's own definition blocks all carry one — so the
    /// element is not on its own evidence of anything.
    /// </remarks>
    [Fact]
    public void ARectangleCarryingATextPathIsNotWordArt()
    {
        PageFrame frame = One(
            "<v:shapetype id=\"_x0000_t202\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\">"
            + "<v:textpath on=\"t\"/></v:shapetype>"
            + "<v:shape id=\"tb\" type=\"#_x0000_t202\" style=\"position:absolute;margin-left:0;"
            + "margin-top:0;width:100pt;height:20pt\" fillcolor=\"silver\">"
            + "<v:textpath style=\"font-family:&quot;Arial&quot;\" string=\"not wordart\"/></v:shape>");

        frame.FillOutline.ShouldBeNull();
        frame.Size.Height.ShouldBe(Length.FromPoints(20), "nothing remeasured it");
    }

    /// <summary>A shape type in range whose <c>v:textpath</c> states no string draws nothing.</summary>
    /// <remarks>
    /// <c>DOA_Template_Form_Type_Certification_Programme</c>'s third header holds exactly that: a
    /// 317.5 pt square <c>#_x0000_t136</c> with a <c>fillcolor</c> and no <c>v:textpath</c> at all.
    /// The reference draws nothing for it — <c>pushToPropMap</c> puts the shape into text-path mode
    /// only when it has a string, and the bare <c>fontwork-plain-text</c> geometry is two open lines
    /// with no fill.
    /// </remarks>
    [Fact]
    public void AWordArtShapeWithNoStringDrawsNothing()
    {
        PageFrame frame = One(
            ShapeType
            + "<v:shape id=\"wm\" type=\"#_x0000_t136\" style=\"position:absolute;margin-left:0;"
            + "margin-top:0;width:317.5pt;height:317.5pt\" fillcolor=\"silver\" stroked=\"f\">"
            + "<v:fill opacity=\".5\"/></v:shape>");

        frame.FillOutline.ShouldBeNull();
        frame.Fill.ShouldBeNull("a WordArt rectangle is not painted as a rectangle");
    }

    /// <summary>
    /// Every WordArt shape type number resolves to the Fontwork type LibreOffice gives it.
    /// </summary>
    /// <remarks>
    /// The spot checks are the two ends of the contiguous range and the three the corpus and the
    /// authored fixtures reach. Getting the offset wrong by one is the whole failure mode of a table
    /// indexed by a number, and it is silent: every warp would still draw, as the wrong shape.
    /// </remarks>
    [Theory]
    [InlineData(136, "fontwork-plain-text")]
    [InlineData(144, "fontwork-arch-up-curve")]
    [InlineData(156, "fontwork-wave")]
    [InlineData(167, "mso-spt167")]
    [InlineData(175, "mso-spt175")]
    public void AShapeTypeNumberNamesItsFontworkType(int number, string expected)
        => Fontwork.FontworkTypeOfShapeType(number).ShouldBe(expected);

    /// <summary>And a number outside it names none.</summary>
    [Theory]
    [InlineData(135)]
    [InlineData(176)]
    [InlineData(202)]
    public void AShapeTypeOutsideTheWordArtRangeNamesNoFontworkType(int number)
        => Fontwork.FontworkTypeOfShapeType(number).ShouldBeNull();

    private static XElement Pict(string inner)
        => XElement.Parse($"<w:pict xmlns:w=\"{W}\" xmlns:v=\"{V}\" xmlns:o=\"{O}\">{inner}</w:pict>");

    private static PageFrame One(string inner)
    {
        OpenTypeFace? face = Face();
        Assert.SkipWhen(face is null, "Liberation Sans is not installed; see check-env.sh");

        return DocxVmlFrames.ReadAll(Pict(inner), 0, null, _ => [], _ => face)
            .ShouldHaveSingleItem();
    }

    private static OpenTypeFace? Face()
    {
        string[] candidates =
        [
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        ];

        string? path = Array.Find(candidates, File.Exists);
        return path is null ? null : OpenTypeFace.ReadFile(path);
    }
}

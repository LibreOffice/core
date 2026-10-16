using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A Word shape that states no fill and no line width still paints, because its
/// <c>wps:style</c> names both out of the theme's format matrix.
/// </summary>
/// <remarks>
/// <para>
/// The reader took only what a shape stated about itself, and recorded that as the conservative
/// choice that "never invents ink". The corpus says it is not conservative, it is blank: censused
/// over the 271 <c>docx</c>, <b>511 shapes across 49 documents</b> state an <c>a:fillRef</c> and no
/// fill of their own. Six organogram templates and five genogram templates are made of nothing but
/// such boxes, so the entire diagram came out as empty outlines.
/// </para>
/// <para>
/// <c>020_Project_Timeline_Template_Modern_Theme</c> is the witness that pins the numbers, because
/// neither of them appears anywhere in <c>document.xml</c>. Its thirteen Gantt bars are
/// <c>homePlate</c> shapes whose <c>wps:spPr</c> holds an <c>a:xfrm</c>, an <c>a:prstGeom</c> and an
/// <c>a:ln</c> naming <c>002060</c> — and no fill, and no <c>w</c>. The theme's first fill style is
/// a bare <c>phClr</c>, so <c>a:fillRef idx="1"</c> over <c>accent1</c> is <c>#5B9BD5</c>; its
/// second line style is <c>w="12700"</c>, so <c>a:lnRef idx="2"</c> is one point. The reference's
/// content stream sets <c>1 w</c> fourteen times and fills that blue, and we drew neither.
/// </para>
/// <para>
/// The width is the reason the shape's own <c>a:ln</c> is laid over the theme's rather than
/// replacing it. Taken alone it gives a stroke of zero, and <c>PageDrawing.DrawFrame</c> drops a
/// zero-width border — so stating the outline colour explicitly is what <em>lost</em> the outline.
/// </para>
/// </remarks>
public sealed class FrameThemeStyleTests
{
    /// <summary>A shape with no fill of its own takes the one its <c>a:fillRef</c> names.</summary>
    [Fact]
    public void AShapeWithNoFillTakesTheThemesThroughItsStyleReference() =>
        Frame(fill: null, style: Style(fillRef: 1, lineRef: 0))
            .Fill.ShouldBe(Colour.FromRgb(0x5B9BD5));

    /// <summary>
    /// The index chooses the style and the reference's own colour is what its <c>phClr</c> becomes.
    /// </summary>
    /// <remarks>
    /// Both halves, because either alone is silently wrong: the index without the colour paints
    /// whatever the placeholder happens to resolve to, and the colour without the index loses the
    /// theme's width and dash. <c>accent2</c> is <c>ED7D31</c> in this theme.
    /// </remarks>
    [Theory]
    [InlineData("accent1", 0x5B9BD5u)]
    [InlineData("accent2", 0xED7D31u)]
    public void TheReferencesOwnColourIsSubstitutedForThePlaceholder(string scheme, uint rgb) =>
        Frame(fill: null, style: Style(fillRef: 1, lineRef: 0, fillScheme: scheme))
            .Fill.ShouldBe(Colour.FromRgb(rgb));

    /// <summary>A shape stating its own fill keeps it, whatever its style names.</summary>
    [Fact]
    public void AStatedFillBeatsTheThemes() =>
        Frame(fill: """<a:solidFill><a:srgbClr val="FF0000"/></a:solidFill>""",
              style: Style(fillRef: 1, lineRef: 0))
            .Fill.ShouldBe(Colour.FromRgb(0xFF0000));

    /// <summary>
    /// <c>a:noFill</c> means it, and so does every other fill kind the reader cannot yet draw.
    /// </summary>
    /// <remarks>
    /// "Stated none" and "said nothing" have always differed here and must keep differing. The
    /// gradient case is the one that would otherwise regress quietly: a shape carrying a real
    /// <c>a:gradFill</c> is painted with <em>that</em>, so answering the theme's flat colour as
    /// well would be a second fill under the first. It is asserted on <c>Fill</c> alone for that
    /// reason — the gradient is carried on <c>PageFrame.Gradient</c>, which
    /// <c>FrameGradientTests</c> covers.
    /// </remarks>
    [Theory]
    [InlineData("<a:noFill/>")]
    [InlineData("""<a:gradFill><a:gsLst><a:gs pos="0"><a:srgbClr val="FF0000"/></a:gs></a:gsLst></a:gradFill>""")]
    [InlineData("""<a:blipFill><a:blip/></a:blipFill>""")]
    public void AnyStatedFillKindStopsTheThemesBeingTaken(string fill) =>
        Frame(fill, Style(fillRef: 1, lineRef: 0)).Fill.ShouldBeNull();

    /// <summary>Only the first fill style is flat; the other two are gradients and arrive as such.</summary>
    /// <remarks>
    /// Every theme Office ships writes <c>a:fillStyleLst</c> as one <c>a:solidFill</c> followed by
    /// two <c>a:gradFill</c>s, so an index of 2 or 3 has no flat colour to give and must not be
    /// made to look as though it does. It reaches <c>PageFrame.Gradient</c> instead, by the same
    /// <c>DrawingGradient</c> a stated gradient reaches it by — see <c>FrameGradientTests</c>.
    /// </remarks>
    [Theory]
    [InlineData(2)]
    [InlineData(3)]
    public void AThemedGradientIsAGradientRatherThanAFlatColour(int index)
    {
        PageFrame frame = Frame(fill: null, style: Style(fillRef: index, lineRef: 0));

        frame.Fill.ShouldBeNull();
        frame.Gradient.ShouldNotBeNull();
    }

    /// <summary>An index of nothing, or past the end of the list, names no fill.</summary>
    /// <remarks>
    /// <c>idx="0"</c> is how a shape says it takes no fill from the theme, which
    /// <c>Theme::getFillStyle</c> answers with nothing.
    /// </remarks>
    [Theory]
    [InlineData(0)]
    [InlineData(9)]
    public void AnIndexOfNoneOrPastTheEndTakesNothing(int index) =>
        Frame(fill: null, style: Style(fillRef: index, lineRef: 0)).Fill.ShouldBeNull();

    /// <summary>
    /// The width comes from the theme while the colour comes from the shape.
    /// </summary>
    /// <remarks>
    /// This is the Gantt bar exactly: <c>&lt;a:ln&gt;&lt;a:solidFill&gt;002060</c> with no <c>w</c>,
    /// under an <c>a:lnRef idx="2"</c> whose entry is <c>w="12700"</c> — one point. Before the
    /// overlay the border was the shape's colour at zero width, which draws nothing at all.
    /// </remarks>
    [Fact]
    public void AStatedColourTakesTheThemesWidth()
    {
        PageFrame frame = Frame(
            fill: null,
            style: Style(fillRef: 1, lineRef: 2),
            line: """<a:ln><a:solidFill><a:srgbClr val="002060"/></a:solidFill></a:ln>""");

        frame.BorderColour.ShouldBe(Colour.FromRgb(0x002060));
        frame.BorderWidth.ShouldBe(Length.FromEmu(12700));
    }

    /// <summary>A stated width wins over the theme's, and the colour still comes from the shape.</summary>
    [Fact]
    public void AStatedWidthBeatsTheThemes()
    {
        PageFrame frame = Frame(
            fill: null,
            style: Style(fillRef: 1, lineRef: 2),
            line: """<a:ln w="57150"><a:solidFill><a:srgbClr val="002060"/></a:solidFill></a:ln>""");

        frame.BorderWidth.ShouldBe(Length.FromEmu(57150));
    }

    /// <summary>A shape stating no <c>a:ln</c> at all takes the theme's whole line.</summary>
    /// <remarks>
    /// Colour and width both, the colour being the reference's <c>accent1</c> substituted for the
    /// entry's <c>phClr</c>.
    /// </remarks>
    [Fact]
    public void AShapeWithNoLineTakesTheThemesEntirely()
    {
        PageFrame frame = Frame(fill: null, style: Style(fillRef: 0, lineRef: 2));

        frame.BorderColour.ShouldBe(Colour.FromRgb(0x5B9BD5));
        frame.BorderWidth.ShouldBe(Length.FromEmu(12700));
    }

    /// <summary>An outline the shape suppresses stays suppressed under a line reference.</summary>
    /// <remarks>
    /// <c>&lt;a:ln w="0"&gt;&lt;a:noFill/&gt;&lt;/a:ln&gt;</c> is what LibreOffice's own export
    /// writes for an unstroked shape, and it has to beat the matrix or every such shape gains an
    /// outline the file says it has not got.
    /// </remarks>
    [Fact]
    public void AnExplicitlySuppressedOutlineBeatsTheMatrix() =>
        Frame(fill: null, style: Style(fillRef: 1, lineRef: 2),
              line: """<a:ln w="0"><a:noFill/></a:ln>""")
            .BorderColour.ShouldBeNull();

    /// <summary>With no matrix at hand, a shape is painted from what it states and nothing else.</summary>
    /// <remarks>
    /// The default <c>DocxFrameContext</c> is what every caller had before the matrix was threaded
    /// through, and a caller that reads a drawing without opening the theme part still gets it.
    /// </remarks>
    [Fact]
    public void WithNoMatrixNothingIsTakenFromTheTheme()
    {
        PageFrame frame = Read(Drawing(null, Style(fillRef: 1, lineRef: 2), null), default);

        frame.Fill.ShouldBeNull();
        frame.BorderColour.ShouldBeNull();
    }

    private static PageFrame Frame(string? fill, string style, string? line = null) =>
        Read(Drawing(fill, style, line),
             new DocxFrameContext(DrawingTheme.Read(Theme), CompatibilityMode: 15)
             {
                 Styles = DrawingStyleMatrix.Read(Theme),
             });

    private static PageFrame Read(XElement drawing, DocxFrameContext context) =>
        DocxFrames
            .ReadAll(drawing, null, anchorOffset: 0, pictures: null, context)
            .ShouldHaveSingleItem();

    private static string Style(int fillRef, int lineRef, string fillScheme = "accent1") =>
        $"""
         <wps:style>
           <a:lnRef idx="{lineRef}"><a:schemeClr val="accent1"/></a:lnRef>
           <a:fillRef idx="{fillRef}"><a:schemeClr val="{fillScheme}"/></a:fillRef>
           <a:effectRef idx="0"><a:schemeClr val="accent1"/></a:effectRef>
           <a:fontRef idx="minor"><a:schemeClr val="lt1"/></a:fontRef>
         </wps:style>
         """;

    private static XElement Drawing(string? fill, string style, string? line) =>
        XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:anchor>
                <wp:extent cx="914400" cy="914400"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wps:wsp>
                  <wps:spPr>
                    <a:xfrm><a:off x="0" y="0"/><a:ext cx="914400" cy="914400"/></a:xfrm>
                    <a:prstGeom prst="homePlate"><a:avLst/></a:prstGeom>
                    {fill}
                    {line}
                  </wps:spPr>
                  {style}
                </wps:wsp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

    /// <summary>
    /// The witness document's own theme, cut to the two lists this reads.
    /// </summary>
    /// <remarks>
    /// The colours and widths are verbatim from <c>020_Project_Timeline_Template_Modern_Theme</c>'s
    /// <c>theme1.xml</c>, which is the stock Office theme: <c>accent1</c> <c>5B9BD5</c>, a first
    /// fill style that is a bare placeholder and two that are gradients, and line styles at a half,
    /// one and one and a half points.
    /// </remarks>
    private static readonly XElement Theme = XElement.Parse(
        $"""
        <a:theme xmlns:a="{A}">
          <a:themeElements>
            <a:clrScheme name="Office">
              <a:dk1><a:srgbClr val="000000"/></a:dk1>
              <a:lt1><a:srgbClr val="FFFFFF"/></a:lt1>
              <a:dk2><a:srgbClr val="44546A"/></a:dk2>
              <a:lt2><a:srgbClr val="E7E6E6"/></a:lt2>
              <a:accent1><a:srgbClr val="5B9BD5"/></a:accent1>
              <a:accent2><a:srgbClr val="ED7D31"/></a:accent2>
              <a:accent3><a:srgbClr val="A5A5A5"/></a:accent3>
              <a:accent4><a:srgbClr val="FFC000"/></a:accent4>
              <a:accent5><a:srgbClr val="4472C4"/></a:accent5>
              <a:accent6><a:srgbClr val="70AD47"/></a:accent6>
              <a:hlink><a:srgbClr val="0563C1"/></a:hlink>
              <a:folHlink><a:srgbClr val="954F72"/></a:folHlink>
            </a:clrScheme>
            <a:fmtScheme name="Office">
              <a:fillStyleLst>
                <a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
                <a:gradFill rotWithShape="1"><a:gsLst>
                  <a:gs pos="0"><a:schemeClr val="phClr"><a:tint val="67000"/></a:schemeClr></a:gs>
                  <a:gs pos="100000"><a:schemeClr val="phClr"><a:tint val="100000"/></a:schemeClr></a:gs>
                </a:gsLst><a:lin ang="5400000" scaled="0"/></a:gradFill>
                <a:gradFill rotWithShape="1"><a:gsLst>
                  <a:gs pos="0"><a:schemeClr val="phClr"><a:shade val="98000"/></a:schemeClr></a:gs>
                  <a:gs pos="100000"><a:schemeClr val="phClr"><a:shade val="88000"/></a:schemeClr></a:gs>
                </a:gsLst><a:lin ang="5400000" scaled="0"/></a:gradFill>
              </a:fillStyleLst>
              <a:lnStyleLst>
                <a:ln w="6350" cap="flat" cmpd="sng" algn="ctr">
                  <a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
                  <a:prstDash val="solid"/><a:miter lim="800000"/>
                </a:ln>
                <a:ln w="12700" cap="flat" cmpd="sng" algn="ctr">
                  <a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
                  <a:prstDash val="solid"/><a:miter lim="800000"/>
                </a:ln>
                <a:ln w="19050" cap="flat" cmpd="sng" algn="ctr">
                  <a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
                  <a:prstDash val="solid"/><a:miter lim="800000"/>
                </a:ln>
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

using System.Xml.Linq;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A Word document's anchored shapes declare their geometry, and it is read.
/// </summary>
/// <remarks>
/// <para>
/// It was not, for as long as the DOCX reader has existed. The same <c>spPr</c> was consulted for
/// the fill and the outline while <c>a:prstGeom</c> was passed over, so every anchored shape in a
/// Word file was painted as its bounding rectangle whatever it asked to be. The preset catalogue
/// was never the gap: all 187 presets are in <c>PresetShapeGeometry.txt</c> and the slide side has
/// resolved them all along.
/// </para>
/// <para>
/// Six corpus templates showed it at once, and they are ordinary business documents: a project
/// timeline's milestone circles came out as squares (<c>ellipse</c>, 32 uses across the six) and
/// its diamonds as squares (<c>diamond</c>), a roadmap's chevrons as bars (<c>homePlate</c>, 33),
/// alongside <c>rightArrow</c>, <c>roundRect</c> and <c>bentConnector3</c>.
/// </para>
/// </remarks>
public sealed class FramePresetGeometryTests
{
    /// <summary>The preset a shape names is carried to the drawing.</summary>
    [Theory]
    [InlineData("ellipse")]
    [InlineData("diamond")]
    [InlineData("homePlate")]
    [InlineData("rightArrow")]
    [InlineData("roundRect")]
    [InlineData("bentConnector3")]
    public void TheShapesPresetIsRead(string preset) =>
        Frame(preset).Preset.ShouldBe(preset);

    /// <summary>
    /// Three presets are deliberately not carried, and each for its own reason.
    /// </summary>
    /// <remarks>
    /// <c>rect</c> is the bounding box the drawing already paints, so resolving it through the
    /// catalogue would build a four-point path to arrive exactly where not asking arrives — and it
    /// is much the commonest preset in the corpus. <c>line</c> and <c>straightConnector1</c> mean
    /// the diagonal <c>PageFrame.IsLine</c> already draws; their preset outline is the box, so
    /// taking it here would put three sides on the page that are not in the file.
    /// </remarks>
    [Theory]
    [InlineData("rect")]
    [InlineData("line")]
    [InlineData("straightConnector1")]
    public void TheThreeThatAreLeftToTheirOwnHandlingAreNotCarried(string preset) =>
        Frame(preset).Preset.ShouldBeNull();

    /// <summary>A shape with no <c>a:prstGeom</c> at all carries none.</summary>
    [Fact]
    public void AShapeWithNoStatedGeometryCarriesNone() =>
        Frame(null).Preset.ShouldBeNull();

    /// <summary>An <c>a:avLst</c> value overrides the preset's own default.</summary>
    /// <remarks>
    /// Only a literal. <c>a:gd</c>'s <c>fmla</c> also carries computed guides — <c>*/ 3 4 5</c>
    /// and the like — which belong to the preset rather than to the shape, so anything that is not
    /// <c>val &lt;n&gt;</c> is left for the catalogue to supply. Taking a computed formula as a
    /// number would silently reshape the very presets whose adjustments are most intricate.
    /// </remarks>
    [Fact]
    public void AStatedAdjustmentIsRead()
    {
        PageFrame frame = Frame("roundRect", """<a:avLst><a:gd name="adj" fmla="val 12500"/></a:avLst>""");

        frame.Adjustments.ShouldNotBeNull();
        frame.Adjustments!["adj"].ShouldBe(12500);
    }

    /// <summary>A computed guide is left to the preset rather than read as a value.</summary>
    [Theory]
    [InlineData("""<a:avLst><a:gd name="adj" fmla="*/ 3 4 5"/></a:avLst>""")]
    [InlineData("""<a:avLst><a:gd name="adj" fmla="pin 0 adj 50000"/></a:avLst>""")]
    [InlineData("""<a:avLst><a:gd name="adj" fmla=""/></a:avLst>""")]
    [InlineData("<a:avLst/>")]
    public void AComputedOrEmptyGuideIsNotAnAdjustment(string values) =>
        Frame("roundRect", values).Adjustments.ShouldBeNull();

    /// <summary>A shape inside a group states its geometry the same way, and it is read.</summary>
    /// <remarks>
    /// It was not, and the asymmetry is the kind that survives a synthetic test: the standalone
    /// reader carried the preset and the group member — built as <c>envelope with { … }</c> — simply
    /// never set it, so every preset shape inside a group was still painted as its bounding
    /// rectangle after the standalone case was fixed. Censused over the 271 corpus <c>docx</c>: 247
    /// such shapes across 25 documents, <b>142 of them <c>ellipse</c></b>. That is the genogram
    /// templates, whose people are circles and squares and which drew as squares and squares.
    /// </remarks>
    [Theory]
    [InlineData("ellipse")]
    [InlineData("roundRect")]
    [InlineData("downArrow")]
    public void AGroupMembersPresetIsReadToo(string preset) =>
        // The group's envelope first, then the member.
        Grouped(preset)[1].Preset.ShouldBe(preset);

    /// <summary>And its stated adjustment with it.</summary>
    [Fact]
    public void AGroupMembersAdjustmentIsReadToo() =>
        Grouped("roundRect", """<a:avLst><a:gd name="adj" fmla="val 12500"/></a:avLst>""")[1]
            .Adjustments.ShouldNotBeNull()["adj"].ShouldBe(12500);

    private static IReadOnlyList<PageFrame> Grouped(string preset, string values = "") =>
        DocxFrames.ReadAll(
            XElement.Parse(
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
                      <wps:wsp>
                        <wps:spPr>
                          <a:xfrm><a:off x="0" y="0"/><a:ext cx="457200" cy="457200"/></a:xfrm>
                          <a:prstGeom prst="{preset}">{values}</a:prstGeom>
                        </wps:spPr>
                      </wps:wsp>
                    </wpg:wgp></a:graphicData></a:graphic>
                  </wp:anchor>
                </w:drawing>
                """),
            null, anchorOffset: 0, pictures: null);

    private static PageFrame Frame(string? preset, string values = "")
    {
        string geometry = preset is null
            ? string.Empty
            : $"""<a:prstGeom prst="{preset}">{values}</a:prstGeom>""";

        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:anchor>
                <wp:extent cx="914400" cy="914400"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wps:wsp>
                  <wps:spPr>
                    <a:xfrm><a:off x="0" y="0"/><a:ext cx="914400" cy="914400"/></a:xfrm>
                    {geometry}
                  </wps:spPr>
                </wps:wsp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        return DocxFrames
            .ReadAll(drawing, null, anchorOffset: 0, pictures: null,
                     new DocxFrameContext(null, InHeaderFooter: false, CompatibilityMode: 15))
            .ShouldHaveSingleItem();
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    private const string Wpg = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";
}

using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// <c>a:prstDash</c> on a Word drawing's outline, which was read on the chart, table and slide paths
/// and not on this one.
/// </summary>
/// <remarks>
/// <para>
/// The expansion itself is <see cref="DashPresets"/> in <c>XRay.Office.Core</c> and is pinned there;
/// what these assert is the wiring — that the element reaches the frame at all, and that the cap that
/// changes its arithmetic travels with it.
/// </para>
/// <para>
/// The corpus case is <c>WordArt_Shapes_Arrows_Catalog1.docx</c>, whose "Line and connector
/// formatting" page states one line each of <c>dash</c>, <c>dot</c>, <c>dashDot</c>, <c>lgDash</c>
/// and <c>sysDash</c>. All five drew solid; the reference draws all five dashed. Reading them took
/// that page's unaccounted ink from <b>0.22% to 0.03%</b> against 26.2.4.2, with no other page moved.
/// </para>
/// </remarks>
public sealed class FrameLineDashTests
{
    /// <summary>The preset's name reaches the frame.</summary>
    [Theory]
    [InlineData("dash")]
    [InlineData("dot")]
    [InlineData("dashDot")]
    [InlineData("lgDash")]
    [InlineData("sysDash")]
    public void TheStatedPresetIsRead(string preset)
    {
        Frame($"""<a:prstDash val="{preset}"/>""").BorderDash.ShouldBe(preset);
    }

    /// <summary>A line stating no dash is solid, which is nearly all of them.</summary>
    [Fact]
    public void AnUnstatedDashLeavesTheLineSolid()
    {
        PageFrame frame = Frame("");

        frame.BorderDash.ShouldBeNull();
        DashPresets.Pattern(frame.BorderDash, frame.BorderWidth).ShouldBeNull();
    }

    /// <summary>
    /// <c>solid</c> is written as often as the element is omitted and comes back as no pattern.
    /// </summary>
    /// <remarks>
    /// The name is still carried through as stated; it is <see cref="DashPresets"/> that maps both
    /// <c>solid</c> and an unrecognised token to no pattern, deliberately, rather than to
    /// LibreOffice's own substitution of <c>dash</c> for anything it does not know.
    /// </remarks>
    [Fact]
    public void SolidExpandsToNoPattern()
    {
        PageFrame frame = Frame("""<a:prstDash val="solid"/>""");

        DashPresets.Pattern(frame.BorderDash, frame.BorderWidth).ShouldBeNull();
    }

    /// <summary>The cap defaults to flat and is read when the line states one.</summary>
    /// <remarks>
    /// It is carried because it changes the dash arithmetic as well as the line's ends: MSO measures a
    /// round or square cap inside the ink, so LibreOffice moves 99% of each ink length into the gap
    /// (<c>oox/source/drawingml/lineproperties.cxx</c>:470-479) and a round-capped dashed line keeps
    /// its period while its strokes shorten.
    /// </remarks>
    [Theory]
    [InlineData(null, LineCap.Butt)]
    [InlineData("flat", LineCap.Butt)]
    [InlineData("rnd", LineCap.Round)]
    [InlineData("sq", LineCap.Square)]
    public void TheCapIsRead(string? cap, LineCap expected)
    {
        Frame("""<a:prstDash val="dash"/>""", cap).BorderCap.ShouldBe(expected);
    }

    /// <summary>
    /// A dashed line's pattern is the preset scaled by the pen, so a fatter pen dashes coarser.
    /// </summary>
    /// <remarks>
    /// <c>a:prstDash</c> names a pattern rather than stating one: <c>dash</c> is four pen-widths of ink
    /// and three of gap, so a 3 pt pen dashes at three times the period of a 1 pt one. This is the
    /// property that makes carrying the preset's name rather than an expanded array the right choice.
    /// </remarks>
    [Fact]
    public void ThePatternScalesWithThePen()
    {
        PageFrame thin = Frame("""<a:prstDash val="dash"/>""", width: 12700);
        PageFrame fat = Frame("""<a:prstDash val="dash"/>""", width: 38100);

        IReadOnlyList<Length> a = DashPresets.Pattern(thin.BorderDash, thin.BorderWidth).ShouldNotBeNull();
        IReadOnlyList<Length> b = DashPresets.Pattern(fat.BorderDash, fat.BorderWidth).ShouldNotBeNull();

        a.Count.ShouldBe(b.Count);
        b[0].Emu.ShouldBe(a[0].Emu * 3);
    }

    private static PageFrame Frame(string dash, string? cap = null, int width = 27940)
    {
        string capAttribute = cap is null ? "" : $""" cap="{cap}" """;
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:inline distT="0" distB="0" distL="0" distR="0">
                <wp:extent cx="4251960" cy="146050"/>
                <a:graphic><a:graphicData><wps:wsp>
                  <wps:spPr>
                    <a:xfrm><a:off x="0" y="0"/><a:ext cx="4251960" cy="146050"/></a:xfrm>
                    <a:prstGeom prst="line"><a:avLst/></a:prstGeom>
                    <a:ln w="{width}"{capAttribute}>
                      <a:solidFill><a:srgbClr val="2E74B5"/></a:solidFill>
                      {dash}
                      <a:round/>
                    </a:ln>
                  </wps:spPr>
                </wps:wsp></a:graphicData></a:graphic>
              </wp:inline>
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

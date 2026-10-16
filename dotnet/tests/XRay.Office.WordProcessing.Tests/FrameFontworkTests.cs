using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A Word text box whose body states an <c>a:prstTxWarp</c> is drawn as warped curves.
/// </summary>
/// <remarks>
/// <para>
/// The reference does not draw such a body as text at all: <c>WpsContext::onEndElement</c> takes
/// the text out of the frame, puts the shape into text-path mode and lets
/// <c>EnhancedCustomShapeFontWork::CreateFontWork</c> replace the whole shape with filled outlines.
/// So the two things to assert are that curves appear <em>and</em> that the words leave the flow —
/// either one alone would pass while the page was doubly wrong.
/// </para>
/// <para>
/// The geometry itself is asserted against what the shape's own box demands rather than against
/// stored coordinates: an envelope warp fills the box, and the four "follow path" warps keep the
/// run's stated size and so cannot.
/// </para>
/// </remarks>
public sealed class FrameFontworkTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
    private const string W14 = "http://schemas.microsoft.com/office/word/2010/wordml";

    /// <summary>Every one of the forty warps, drawn as curves.</summary>
    /// <remarks>
    /// The first twenty-five are what the corpus states on the words side. The nine below them are
    /// what it does not: the two <c>textRing*</c>, the four <c>*Pour</c>, the two
    /// <c>textDeflateInflate*</c>, and <c>textSlantUp</c>, which is a shared table away from
    /// <c>textSlantDown</c>. They are pinned against
    /// <c>/home/user/fixtures/fontwork-presets-{default,adjusted}.docx</c>, where they are measured
    /// rather than asserted — a table transcribed for a preset nothing checks is a transcription
    /// nothing checks, and the fixture is what makes them checkable. <c>ST_TextShapeType</c> has 41
    /// values; the forty-first is <c>textNoShape</c>, which is the identity.
    /// </remarks>
    [Theory]
    [InlineData("textArchUp")]
    [InlineData("textArchDown")]
    [InlineData("textCircle")]
    [InlineData("textButton")]
    [InlineData("textWave1")]
    [InlineData("textWave2")]
    [InlineData("textDoubleWave1")]
    [InlineData("textInflate")]
    [InlineData("textDeflate")]
    [InlineData("textInflateBottom")]
    [InlineData("textDeflateBottom")]
    [InlineData("textTriangle")]
    [InlineData("textTriangleInverted")]
    [InlineData("textChevron")]
    [InlineData("textChevronInverted")]
    [InlineData("textCascadeUp")]
    [InlineData("textCascadeDown")]
    [InlineData("textCurveUp")]
    [InlineData("textSlantDown")]
    [InlineData("textCanUp")]
    [InlineData("textCanDown")]
    [InlineData("textFadeRight")]
    [InlineData("textFadeLeft")]
    [InlineData("textStop")]
    [InlineData("textPlain")]
    [InlineData("textFadeUp")]
    [InlineData("textFadeDown")]
    [InlineData("textSlantUp")]
    [InlineData("textCurveDown")]
    [InlineData("textInflateTop")]
    [InlineData("textDeflateTop")]
    [InlineData("textWave4")]
    [InlineData("textRingInside")]
    [InlineData("textRingOutside")]
    [InlineData("textArchUpPour")]
    [InlineData("textArchDownPour")]
    [InlineData("textCirclePour")]
    [InlineData("textButtonPour")]
    [InlineData("textDeflateInflate")]
    [InlineData("textDeflateInflateDeflate")]
    public void AWarpedBodyBecomesCurvesAndLeavesTheFlow(string preset)
    {
        PageFrame frame = Frame(preset);

        frame.FillOutline.ShouldNotBeNull();
        frame.FillOutline!.Commands.Count.ShouldBeGreaterThan(100);
        frame.Blocks.ShouldBeEmpty();
    }

    /// <summary>An unwarped body is untouched: text in the flow, no outline of its own.</summary>
    [Theory]
    [InlineData(null)]
    [InlineData("textNoShape")]
    public void AnUnwarpedBodyKeepsItsText(string? preset)
    {
        PageFrame frame = Frame(preset);

        frame.FillOutline.ShouldBeNull();
        frame.Blocks.ShouldHaveSingleItem();
    }

    /// <summary>
    /// A warp on a shape that is not a rectangle keeps its text, as the reference does.
    /// </summary>
    /// <remarks>
    /// <c>WpsContext.cxx:966-970</c>. Word combines its "abc Transform" with any shape; LibreOffice
    /// can only render the rectangle-based kind and leaves the rest alone.
    /// </remarks>
    [Fact]
    public void AWarpOnANonRectangleIsIgnored()
    {
        PageFrame frame = Frame("textWave1", geometry: "ellipse");

        frame.FillOutline.ShouldBeNull();
        frame.Blocks.ShouldHaveSingleItem();
    }

    /// <summary>
    /// A warp XRay.Office cannot draw still takes the text out of the flow.
    /// </summary>
    /// <remarks>
    /// The reference has already emptied the frame by the time it decides what the curves look
    /// like, so a shape whose warp cannot be built leaves neither text nor curves. That is what the
    /// slides side has always done for an undrawable warp, and the two families have to agree.
    /// <para>
    /// Every one of `ST_TextShapeType`'s forty warps is now built, so the case has to be reached
    /// through a value no schema defines rather than through a preset that is merely unimplemented
    /// — which is itself the assertion that none is left. It is still a live branch: a face with no
    /// <c>glyf</c> outlines takes it too.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("textNotAPreset")]
    public void AWarpThatCannotBeDrawnStillLeavesNoText(string preset)
    {
        PageFrame frame = Frame(preset);

        frame.FillOutline.ShouldBeNull();
        frame.Blocks.ShouldBeEmpty();
    }

    /// <summary>
    /// An envelope warp fills the shape's box, whatever size the run states.
    /// </summary>
    /// <remarks>
    /// This is the surprising half of Fontwork and the half that decides how much ink lands on the
    /// page: twenty of the twenty-four presets normalise the text's own ink box to the unit square
    /// before mapping it between the two rails, so the stated size cancels out and a 25 pt run
    /// fills a 72 pt shape. Only the four "follow path" presets keep the size.
    /// </remarks>
    [Fact]
    public void AnEnvelopeWarpFillsTheShapesBox()
    {
        (Length left, Length top, Length right, Length bottom) = Extent(Frame("textInflate"));

        // Two points of slack: the rails are the edges of the box and the ink touches them only
        // where a letter happens to reach, which for `textInflate` is a point or so inside.
        Points(left).ShouldBe(0.0, 2.0);
        Points(right).ShouldBe(360.0, 2.0);
        Points(top).ShouldBe(0.0, 2.0);
        Points(bottom).ShouldBe(72.0, 2.0);
    }

    /// <summary>
    /// A "follow path" warp keeps the run's size, so it covers a line rather than the box.
    /// </summary>
    [Fact]
    public void AFollowPathWarpKeepsItsStatedSize()
    {
        (Length _, Length top, Length _, Length bottom) = Extent(Frame("textArchUp"));

        // One 25 pt line of capitals, not a 72 pt box.
        (bottom - top).ShouldBeLessThan(Length.FromPoints(30));
        (bottom - top).ShouldBeGreaterThan(Length.FromPoints(10));
    }

    /// <summary>
    /// The curves take the run's <c>w14:textFill</c> and <c>w14:textOutline</c>, not the shape's.
    /// </summary>
    /// <remarks>
    /// Fontwork has one fill for the whole object and cannot style a portion of its text, so the
    /// importer copies the character fill and outline onto the shape
    /// (<c>WpsContext.cxx:996-1014</c>). It is the only path on which those two elements are read:
    /// on an ordinary run LibreOffice draws neither, and reproducing that is measured rather than
    /// omitted.
    /// </remarks>
    [Fact]
    public void TheCurvesTakeTheRunsOwnFillAndOutline()
    {
        PageFrame frame = Frame("textWave1", effects: true);

        frame.Gradient.ShouldNotBeNull();
        frame.Gradient!.Stops.Count.ShouldBe(3);
        frame.BorderColour.ShouldBe(Colour.FromRgb(0x17365D));
        frame.BorderWidth.ShouldBe(Length.FromEmu(25560));
    }

    /// <summary>An unwarped run's text effects are still ignored, and that is measured.</summary>
    /// <remarks>
    /// The catalogue states 104 <c>w14:textFill</c> and 348 <c>w14:textOutline</c> on ordinary runs
    /// and LibreOffice draws none of them. Reading them here would move 63 of its shapes away from
    /// the reference rather than towards it.
    /// </remarks>
    [Fact]
    public void AnUnwarpedRunsTextEffectsAreStillIgnored()
    {
        PageFrame frame = Frame("textNoShape", effects: true);

        frame.Gradient.ShouldBeNull();
        frame.BorderColour.ShouldBeNull();
    }

    /// <summary>The adjustment guides are converted into the units the WordArt tables expect.</summary>
    /// <remarks>
    /// Not one factor: an angle handle is 1/60000 of a degree in DrawingML and a plain degree in
    /// WordArt, everything else is a per-mille of the shape against a 21600 view box, and a wave's
    /// second guide states an offset from the centre rather than a position
    /// (<c>fontworkhelpers.cxx:95-150</c>). Feeding the raw number through instead turns
    /// <c>textArchUp</c>'s 180 degrees into an arc of 10800000, which normalises to zero.
    /// </remarks>
    [Fact]
    public void TheStatedAdjustmentChangesTheCurve()
    {
        (Length _, Length top, Length _, Length bottom) =
            Extent(Frame("textArchUp", adjustment: 10800000));
        (Length _, Length flatTop, Length _, Length flatBottom) =
            Extent(Frame("textArchUp", adjustment: 0));

        // 180 degrees is the top of the box and 0 degrees the bottom of it, so the two arcs put
        // their text three quarters of the shape apart.
        ((flatTop + flatBottom) - (top + bottom)).ShouldBeGreaterThan(Length.FromPoints(100));
    }

    /// <summary>A length in points, for an assertion with a tolerance.</summary>
    private static double Points(Length length) => length.Emu / 12700.0;

    /// <summary>The bounding box of a frame's curves.</summary>
    private static (Length Left, Length Top, Length Right, Length Bottom) Extent(PageFrame frame)
    {
        GraphicsPath path = frame.FillOutline.ShouldNotBeNull();

        long left = long.MaxValue;
        long top = long.MaxValue;
        long right = long.MinValue;
        long bottom = long.MinValue;

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            left = Math.Min(left, command.Point.X.Emu);
            right = Math.Max(right, command.Point.X.Emu);
            top = Math.Min(top, command.Point.Y.Emu);
            bottom = Math.Max(bottom, command.Point.Y.Emu);
        }

        return (Length.FromEmu(left), Length.FromEmu(top), Length.FromEmu(right), Length.FromEmu(bottom));
    }

    /// <summary>A 360 x 72 pt text box holding one centred run, warped or not.</summary>
    private static PageFrame Frame(
        string? preset,
        string geometry = "rect",
        bool effects = false,
        int? adjustment = null)
    {
        string guides = adjustment is { } stated
            ? $"""<a:avLst><a:gd name="adj" fmla="val {stated}"/></a:avLst>"""
            : "<a:avLst/>";

        string warp = preset is null
            ? string.Empty
            : $"""<a:prstTxWarp prst="{preset}">{guides}</a:prstTxWarp>""";

        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}" xmlns:w14="{W14}">
              <wp:inline>
                <wp:extent cx="4572000" cy="914400"/>
                <a:graphic><a:graphicData><wps:wsp>
                  <wps:cNvSpPr txBox="1"/>
                  <wps:spPr>
                    <a:xfrm><a:off x="0" y="0"/><a:ext cx="4572000" cy="914400"/></a:xfrm>
                    <a:prstGeom prst="{geometry}"><a:avLst/></a:prstGeom>
                  </wps:spPr>
                  <wps:txbx><w:txbxContent><w:p>
                    <w:r><w:rPr>{(effects ? Effects : string.Empty)}</w:rPr><w:t>WORDART</w:t></w:r>
                  </w:p></w:txbxContent></wps:txbx>
                  <wps:bodyPr>{warp}</wps:bodyPr>
                </wps:wsp></a:graphicData></a:graphic>
              </wp:inline>
            </w:drawing>
            """);

        return DocxFrames
            .ReadAll(drawing, Content, anchorOffset: 0, pictures: null,
                     new DocxFrameContext(null, InHeaderFooter: false, CompatibilityMode: 15))
            .ShouldHaveSingleItem();
    }

    /// <summary>The run's <c>w14</c> text effects, as the WordArt catalogue writes them.</summary>
    private const string Effects = """
        <w14:textOutline w14:w="25560">
          <w14:solidFill><w14:srgbClr w14:val="17365d"/></w14:solidFill>
        </w14:textOutline>
        <w14:textFill><w14:gradFill><w14:gsLst>
          <w14:gs w14:pos="0"><w14:srgbClr w14:val="22d3ee"/></w14:gs>
          <w14:gs w14:pos="50000"><w14:srgbClr w14:val="2563eb"/></w14:gs>
          <w14:gs w14:pos="100000"><w14:srgbClr w14:val="7c3aed"/></w14:gs>
        </w14:gsLst><w14:lin w14:ang="0" w14:scaled="0"/></w14:gradFill></w14:textFill>
        """;

    /// <summary>
    /// The text the box holds, as the layout source would have resolved it.
    /// </summary>
    /// <remarks>
    /// A resolved paragraph rather than the markup, because that is what the reader is handed: the
    /// family and size of a run routinely come from a style, and the warp has to be built from the
    /// face those resolve to.
    /// </remarks>
    private static IReadOnlyList<PageBlock> Content(XElement box)
    {
        OpenTypeFace? face = Face();
        Assert.SkipWhen(face is null, "Liberation Sans is not installed; see check-env.sh");

        return
        [
            new PageParagraph
            {
                Text = "WORDART",
                Face = face!,
                EmSize = Length.FromPoints(25),
                Format = new ParagraphFormat { Alignment = TextAlignment.Centre },
            },
        ];
    }

    private static OpenTypeFace? Face()
    {
        string[] candidates =
        [
            "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
            "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf",
        ];

        string? path = Array.Find(candidates, File.Exists);
        return path is null ? null : OpenTypeFace.ReadFile(path);
    }
}

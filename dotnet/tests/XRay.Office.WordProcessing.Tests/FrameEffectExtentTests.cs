using System.IO.Compression;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// <c>wp:effectExtent</c> on an inline drawing, which is room on the line rather than size on the
/// drawing.
/// </summary>
/// <remarks>
/// <para>
/// A shadow, a glow or a fat stroke paints outside the <c>wp:extent</c> a drawing states, and
/// <c>wp:effectExtent</c> is how much on each side. For a <c>wp:inline</c> LibreOffice adds all four
/// edges straight to the object's own margins — <c>GraphicImport.cxx</c>:1036-1055, guarded by
/// <c>IMPORT_AS_DETECTED_INLINE</c> and a zero rotation, with the comment <em>"EffectExtent contains
/// all needed additional space, including fat stroke and shadow. Simple add it to the margins."</em>
/// Writer then rests the object's rectangle <em>including</em> that spacing on the baseline
/// (<c>SwFlyCntPortion::SetBase</c> sizing itself from
/// <c>SwAsCharAnchoredObjectPosition::GetObjBoundRectInclSpacing</c>), so the extent grows the line.
/// </para>
/// <para>
/// Measured in <c>dotnet/probes/words-inline-effectextent/</c>, against both installed references —
/// 24.2.7.2 and 26.2.4.2, which agree to the twip on every fixture. One 50.4 pt shape between two
/// 12 pt text lines, the gap between those lines measured against a zero-extent control:
/// </para>
/// <list type="table">
///   <item><term><c>l=t=r=b="0"</c></term><description>64.25 pt — the control</description></item>
///   <item><term><c>27432</c> (2.16 pt)</term><description>68.55 pt, <b>+4.30</b></description></item>
///   <item><term><c>91440</c> (7.2 pt)</term><description>78.65 pt, <b>+14.40</b></description></item>
///   <item><term><c>137160</c> (10.8 pt)</term><description>85.85 pt, <b>+21.60</b></description></item>
///   <item><term>top only</term><description>75.05 pt, <b>+10.80</b></description></item>
///   <item><term>bottom only</term><description>75.05 pt, <b>+10.80</b></description></item>
/// </list>
/// <para>
/// So each edge is independent and additive, and the growth is exactly the stated EMUs rounded to the
/// twip — 2.16 pt is 43.2 twips, which lands at 43 and doubles to the 4.30 above rather than to 4.32.
/// </para>
/// <para>
/// The corpus case is <c>WordArt_Shapes_Arrows_Catalog1.docx</c>, 340 unrotated inline shapes all
/// carrying one of those three extents. Without this the document paginated to <b>45 pages against
/// both references' 52</b>; with it, 52, holding the same shapes on every one of them.
/// </para>
/// </remarks>
public sealed class FrameEffectExtentTests
{
    /// <summary>The four edges are read from an inline drawing, each rounded to the twip.</summary>
    /// <remarks>
    /// <para>
    /// The rounding is the reader's shared <c>Emu</c> helper and it is what the reference does too:
    /// 27432 EMU is 2.16 pt, which is 43.2 twips and lands at 43, so a shape carrying that extent on
    /// its top and bottom grows its line by <b>4.30 pt</b> and not by 4.32. That is the measured
    /// figure — see the type's own remarks — so asserting the unrounded EMUs here would pin a value
    /// neither renderer produces. The other two edges divide evenly: 91440 is 144 twips and 137160 is
    /// 216.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheFourEdgesAreRead()
    {
        PageFrame frame = Inline("""<wp:effectExtent l="27432" t="91440" r="137160" b="0"/>""");

        frame.EffectExtent.Left.ShouldBe(Length.FromTwips(43));
        frame.EffectExtent.Top.ShouldBe(Length.FromTwips(144));
        frame.EffectExtent.Right.ShouldBe(Length.FromTwips(216));
        frame.EffectExtent.Bottom.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// The catalogue's three extents grow a line by the figures both references were measured at.
    /// </summary>
    /// <remarks>
    /// The whole of the pagination fix in one assertion: these are the +4.30, +14.40 and +21.60 pt
    /// from the probe, as the difference between the drawing's own height and the room it takes.
    /// </remarks>
    [Theory]
    [InlineData(27432, 4.30)]
    [InlineData(91440, 14.40)]
    [InlineData(137160, 21.60)]
    public void TheMeasuredGrowthIsReproduced(int emu, double points)
    {
        PageFrame frame = Inline($"""<wp:effectExtent l="0" t="{emu}" r="0" b="{emu}"/>""");

        (frame.InlineExtent.Height - frame.Size.Height).ShouldBe(Length.FromPoints(points));
    }

    /// <summary>
    /// The extent grows the room the drawing takes on its line and leaves the drawing's own size alone.
    /// </summary>
    /// <remarks>
    /// The split matters because the shape is still painted at the size the file gives it: 10.8 pt on
    /// each edge of a 50.4 pt shape makes a 72 pt line and a 50.4 pt shape, not a 72 pt shape.
    /// </remarks>
    [Fact]
    public void TheExtentGrowsTheLineAndNotTheDrawing()
    {
        PageFrame frame = Inline("""<wp:effectExtent l="137160" t="137160" r="137160" b="137160"/>""");

        frame.Size.Height.ShouldBe(Length.FromEmu(640080));
        frame.InlineExtent.Height.ShouldBe(Length.FromPoints(72));
        frame.InlineExtent.Width.ShouldBe(frame.Size.Width + Length.FromPoints(21.6));
    }

    /// <summary>A drawing stating no extent takes exactly its own size on the line.</summary>
    [Fact]
    public void NoExtentLeavesTheLineAtTheDrawingsOwnSize()
    {
        PageFrame frame = Inline("");

        frame.EffectExtent.ShouldBe(Margins.Zero);
        frame.InlineExtent.Height.ShouldBe(frame.Size.Height);
        frame.InlineExtent.Width.ShouldBe(frame.Size.Width);
    }

    /// <summary>
    /// The <c>dist*</c> attributes beside it are discarded on an inline drawing, not added to it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is not an omission. LibreOffice zeroes the matching margin merely because the attribute is
    /// present — <c>GraphicImport.cxx</c>:1387-1398 is four cases of
    /// <c>case LN_CT_Inline_distT: m_nTopMargin = 0;</c>, which never reads <c>nIntValue</c> at all.
    /// </para>
    /// <para>
    /// Measured: a fixture stating <c>distT="137160" distB="137160"</c> and no effect extent moves the
    /// line below it by <b>0.00 pt</b> against the zero control, on both installed references. So a
    /// reader that added the two would be 21.6 pt out on every such drawing.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheDistanceAttributesDoNotGrowAnInlineDrawing()
    {
        PageFrame frame = Inline("", dist: """distT="137160" distB="137160" distL="137160" distR="137160" """);

        frame.InlineExtent.Height.ShouldBe(frame.Size.Height);
        frame.InlineExtent.Width.ShouldBe(frame.Size.Width);
    }

    /// <summary>
    /// An anchored drawing carries no extent, because LibreOffice reaches it by a different route.
    /// </summary>
    /// <remarks>
    /// A floating drawing's extent goes into its <em>wrap</em> margins, through the much longer
    /// <c>WrapTextMode_PARALLEL</c> branch that needs the shape's own bound rectangle. Reading the four
    /// numbers there would be wrong rather than partial — see the note on
    /// <see cref="PageFrame.Spacing"/> for the measurement that keeps it unread.
    /// </remarks>
    [Fact]
    public void AnAnchoredDrawingTakesNoEffectExtent()
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:anchor distT="0" distB="0" distL="0" distR="0">
                <wp:extent cx="1828800" cy="640080"/>
                <wp:effectExtent l="137160" t="137160" r="137160" b="137160"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wps:wsp>
                  <wps:spPr>
                    <a:xfrm><a:off x="0" y="0"/><a:ext cx="1828800" cy="640080"/></a:xfrm>
                    <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                  </wps:spPr>
                </wps:wsp></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        PageFrame frame = DocxFrames
            .ReadAll(drawing, null, anchorOffset: 0, pictures: null)
            .ShouldHaveSingleItem();

        frame.EffectExtent.ShouldBe(Margins.Zero);
        frame.InlineExtent.Height.ShouldBe(frame.Size.Height);
    }

    /// <summary>A plain picture takes none of it, because LibreOffice stops treating it as a shape.</summary>
    /// <remarks>
    /// <para>
    /// <c>GraphicImport.cxx</c>:879 gates the whole margin block on <c>if (m_xShape.is())</c>, and
    /// <c>bUseShape = !m_xGraphicObject.is()</c> at :844 — an unrotated, effect-free picture is turned
    /// into a Writer graphic object and its shape disposed, so nothing below it runs.
    /// </para>
    /// <para>
    /// Measured on both references: a <c>pic:pic</c> at <c>137160</c> on all four edges moves the line
    /// below it by <b>0.00 pt</b>, where the same fixture built from a <c>wps:wsp</c> moves it by
    /// 21.60. This is the case that cost <c>gpp-pr-top-7-office-markets-4q-2023.docx</c> 3.35 pt on
    /// everything below its chart, and <c>TE.CAO.00125 … OJT Logbook.docx</c> a whole page off a
    /// 0.75 pt header logo.
    /// </para>
    /// </remarks>
    [Fact]
    public void APlainPictureTakesNoEffectExtent()
    {
        PageFrame frame = Picture("""<wp:effectExtent l="137160" t="137160" r="137160" b="137160"/>""");

        frame.EffectExtent.ShouldBe(Margins.Zero);
        frame.InlineExtent.Height.ShouldBe(frame.Size.Height);
        frame.InlineExtent.Width.ShouldBe(frame.Size.Width);
    }

    /// <summary>
    /// A picture carrying DrawingML effects keeps it, because that refuses the conversion.
    /// </summary>
    /// <remarks>
    /// <c>bContainsEffects</c> (<c>GraphicImport.cxx</c>:820-825) is any of <c>EffectProperties</c>,
    /// <c>3DEffectProperties</c> or <c>ArtisticEffectProperties</c> in the grab bag — written by
    /// <c>oox</c> from <c>a:effectLst</c>, <c>a:scene3d</c>/<c>a:sp3d</c> and the <c>a14</c> artistic
    /// effects. Measured: the same picture with an <c>a:outerShdw</c> moves its line by
    /// <b>+21.65 pt</b>, exactly as a shape does; with the shadow and no extent, by 0.00.
    /// </remarks>
    [Theory]
    [InlineData("""<a:effectLst><a:outerShdw blurRad="50800"><a:srgbClr val="000000"/></a:outerShdw></a:effectLst>""")]
    [InlineData("""<a:scene3d><a:camera prst="orthographicFront"/></a:scene3d>""")]
    [InlineData("""<a:sp3d extrusionH="57150"/>""")]
    public void APictureCarryingEffectsKeepsTheExtent(string effects)
    {
        PageFrame frame = Picture(
            """<wp:effectExtent l="137160" t="137160" r="137160" b="137160"/>""", effects);

        frame.InlineExtent.Height.ShouldBe(frame.Size.Height + Length.FromPoints(21.6));
    }

    /// <summary>
    /// A turned drawing takes its line box from Word's reserved rectangle and its own turned one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A rotation sends the import down the other branch of <c>nOOXAngle == 0</c>
    /// (<c>GraphicImport.cxx</c>:1055-1090), which keeps <em>both</em> rectangles: the object laid
    /// out is the turned snap rectangle, and each margin round it is the signed gap between that and
    /// the rectangle Word reserved — Word's own width/height swap applied to the stated extent, then
    /// expanded by the effect extent. The horizontal margins keep their sign and the vertical ones
    /// are clamped at nought (<c>GraphicImport.cxx</c>:1245-1249, tdf#141880), so the room taken is
    /// Word's box across and the larger of the two down.
    /// </para>
    /// <para>
    /// <strong>This asserted the opposite until 2026-09-06</strong> — that a turned drawing takes no
    /// effect extent and no extra room at all — on the strength of a fixture that varied only the
    /// vertical gap, where the turned height swallows the extent whole. Varying the width shows the
    /// extent plainly: at 20 degrees a <c>137160</c> extent moves the same fixture's line advance from
    /// <b>149.87 to 171.47 pt</b> on both references while the gap stays at 113.05 either way.
    /// </para>
    /// <para>
    /// The figures are from <c>dotnet/probes/words-inline-rotated-bbox/</c>, a 144 × 50.4 pt black
    /// rectangle, both installed references identical on every row.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(20, 0, 144.00, 96.60)]     // no swap: Word's width, the turned height
    [InlineData(20, 137160, 165.60, 96.60)] // the extent grows the width and is lost under the height
    [InlineData(45, 0, 50.40, 144.00)]     // swapped, and 144 beats the turned 137.45
    [InlineData(90, 0, 50.40, 144.00)]
    [InlineData(135, 0, 144.00, 137.45)]   // no swap at 135, so the turned height wins
    [InlineData(315, 0, 144.00, 137.45)]
    public void ATurnedDrawingTakesItsTurnedBoundingBox(
        int degrees, int emu, double width, double height)
    {
        PageFrame frame = Inline(
            $"""<wp:effectExtent l="{emu}" t="{emu}" r="{emu}" b="{emu}"/>""",
            rotation: degrees * 60000);

        frame.InlineExtent.Width.ShouldBe(Length.FromPoints(width));
        frame.InlineExtent.Height.ShouldBe(Length.FromPoints(height));
    }

    /// <summary>
    /// A turned drawing is centred in its line box rather than moved by the extent's left edge.
    /// </summary>
    /// <remarks>
    /// The margins round a turned object are symmetrical by construction — each is half the gap
    /// between Word's box and the snap rectangle — so the drawing's own rectangle shares its centre
    /// with the box. Measured on the same fixtures, the drawn rectangle's left edge in points on a
    /// line starting at 103.50: 20 degrees <b>99.25</b>, 45 degrees <b>60.00</b> (which hangs into
    /// the page margin), 135 degrees <b>106.75</b>. Those are the snap rectangle's edges; the offsets
    /// asserted here are the drawing's own, which is the smaller rectangle inside it.
    /// </remarks>
    [Theory]
    [InlineData(20, 0.00, 23.10)]
    [InlineData(45, -46.80, 46.80)]
    [InlineData(135, 0.00, 43.525)]
    public void ATurnedDrawingIsCentredInItsLineBox(int degrees, double x, double y)
    {
        PageFrame frame = Inline("""<wp:effectExtent l="0" t="0" r="0" b="0"/>""",
            rotation: degrees * 60000);

        frame.InlineOffset.X.ShouldBe(Length.FromPoints(x));
        frame.InlineOffset.Y.ShouldBe(Length.FromPoints(y));
    }

    /// <summary>An upright drawing is offset by the extent's left edge and by nothing downwards.</summary>
    /// <remarks>
    /// The other half of <see cref="PageFrame.InlineOffset"/>, and the asymmetry is measured rather
    /// than reasoned about — see <c>FrameLayout.HangInline</c> and
    /// <c>probes/words-inline-effectextent/make-x-fixture.py</c>.
    /// </remarks>
    [Fact]
    public void AnUprightDrawingTakesTheLeftEdgeAndNoTopEdge()
    {
        PageFrame frame = Inline("""<wp:effectExtent l="137160" t="91440" r="27432" b="0"/>""");

        frame.InlineOffset.X.ShouldBe(Length.FromPoints(10.8));
        frame.InlineOffset.Y.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// A drawing's ink is painted below its text by the top extent, and by nothing when the extent
    /// states only a bottom edge.
    /// </summary>
    /// <remarks>
    /// The two halves of an inline drawing genuinely land in different places and this is the split:
    /// <see cref="PlacedFrame.Area"/> is the text's rectangle and <see cref="PlacedFrame.Ink"/> the
    /// drawing's. Measured in <c>dotnet/probes/words-inline-shape-ink/</c> on a 144 x 50.4 pt shape
    /// between two 12 pt lines, both installed references identical: the fill band starts at
    /// 85.75 pt with no extent, at 88.00 with <c>t="27432"</c>, at 93.00 with <c>91440</c> and at
    /// 96.50 with <c>137160</c>, while a <c>b</c> edge of 137160 leaves it at 85.75 — and the
    /// <c>INSIDE</c> run of a <c>wps:txbx</c> stays at 104.66 through all of them.
    /// </remarks>
    [Theory]
    [InlineData(0, 0.0)]
    [InlineData(27432, 2.15)]
    [InlineData(91440, 7.2)]
    [InlineData(137160, 10.8)]
    public void TheInkIsPaintedBelowTheTextByTheTopExtent(int top, double points)
    {
        PageFrame frame = Inline($"""<wp:effectExtent l="0" t="{top}" r="0" b="137160"/>""");

        frame.InlineInkOffset.ShouldBe(Length.FromPoints(points));

        PlacedFrame placed = new(
            frame,
            new DocRect(Length.FromPoints(72), Length.FromPoints(100),
                        frame.Size.Width, frame.Size.Height));

        placed.Ink.X.ShouldBe(placed.Area.X);
        placed.Ink.Y.ShouldBe(placed.Area.Y + Length.FromPoints(points));
        placed.Ink.Width.ShouldBe(placed.Area.Width);
        placed.Ink.Height.ShouldBe(placed.Area.Height);
    }

    /// <summary>A turned drawing's ink is not offset: its two rectangles are centred instead.</summary>
    /// <remarks>
    /// <see cref="PageFrame.InlineOffset"/> centres a turned drawing in its line box in both axes,
    /// so adding the top extent on top of that would move it twice. The rotated fixtures in
    /// <c>probes/words-inline-effectextent/</c> read the same line advance with and without an
    /// extent, which is what says the turned branch is about the snap rectangle and not about this.
    /// </remarks>
    [Fact]
    public void ATurnedDrawingTakesNoInkOffset()
    {
        PageFrame frame = Inline(
            """<wp:effectExtent l="137160" t="137160" r="137160" b="137160"/>""",
            rotation: 20 * 60000);

        frame.InlineInkOffset.ShouldBe(Length.Zero);
    }

    private static PageFrame Picture(string effectExtent, string effects = "")
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:pic="{Pic}"
                       xmlns:r="{R}">
              <wp:inline distT="0" distB="0" distL="0" distR="0">
                <wp:extent cx="1828800" cy="640080"/>
                {effectExtent}
                <a:graphic><a:graphicData uri="{Pic}">
                  <pic:pic>
                    <pic:nvPicPr><pic:cNvPr id="1" name="p"/><pic:cNvPicPr/></pic:nvPicPr>
                    <pic:blipFill><a:blip r:embed="rId1"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>
                    <pic:spPr>
                      <a:xfrm><a:off x="0" y="0"/><a:ext cx="1828800" cy="640080"/></a:xfrm>
                      <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
                      {effects}
                    </pic:spPr>
                  </pic:pic>
                </a:graphicData></a:graphic>
              </wp:inline>
            </w:drawing>
            """);

        return DocxFrames
            .ReadAll(drawing, null, anchorOffset: 0, pictures: null)
            .ShouldHaveSingleItem();
    }

    private static PageFrame Inline(string effectExtent, string dist = "", int rotation = 0)
    {
        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:inline {dist}>
                <wp:extent cx="1828800" cy="640080"/>
                {effectExtent}
                <a:graphic><a:graphicData><wps:wsp>
                  <wps:spPr>
                    <a:xfrm{(rotation == 0 ? "" : $" rot=\"{rotation}\"")}><a:off x="0" y="0"/><a:ext cx="1828800" cy="640080"/></a:xfrm>
                    <a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
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
    private const string Pic = "http://schemas.openxmlformats.org/drawingml/2006/picture";
    private const string R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
}

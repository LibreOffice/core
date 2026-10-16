using System.Runtime.InteropServices;
using System.Text;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.MsBinary.Escher;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.MsBinary;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// An Escher WordArt shape is drawn as warped outlines, from properties rather than from a text
/// body.
/// </summary>
/// <remarks>
/// <para>
/// <c>SvxMSDffManager::ImportShape</c> makes a shape WordArt on one bit —
/// <c>( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) &amp; 0x4000 ) != 0</c>,
/// <c>filter/source/msfilter/msdffimp.cxx</c>:4423-4426 — and then reads its text from
/// <c>gtextUNICODE</c>, its face from <c>gtextFont</c> and its alignment from <c>gtextAlign</c>
/// (<c>:4433-4480</c>). <c>DffPropertyReader::ApplyAttributes</c> (<c>:2516-2570</c>) turns the
/// remaining bits of property 255 into the <c>TextPath</c> property set.
/// </para>
/// <para>
/// Every case here is checked against <see cref="Fontwork"/> called directly with the values the
/// reader is supposed to have decoded, so the assertions carry no transcribed geometry and a
/// conversion that is wrong cannot agree by accident. The two corpus witnesses' own property
/// tables are reproduced verbatim: <c>8.16_AOD_FINAL…ppt</c>'s arch (type 144, <c>adjustValue</c>
/// <c>0xFF4C000B</c>) and <c>pres_ioc_phuket.ppt</c>'s plain text (type 136, <c>adjustValue</c>
/// 10707) — see <c>probes/slides-r107/wordart-props.txt</c>.
/// </para>
/// </remarks>
public sealed class PptFontworkTests
{
    /// <summary>The AOD shape's rectangle: 3348 × 1317 master units of 1/576 inch.</summary>
    private static readonly DocSize Box = new(
        Length.FromEmu(3348L * Length.EmuPerInch / 576),
        Length.FromEmu(1317L * Length.EmuPerInch / 576));

    /// <summary>The AOD shape's text.</summary>
    private const string Text = "Do you know what these are?";

    /// <summary>Its face.</summary>
    private const string Face = "Times New Roman";

    /// <summary>Property 255 as the AOD shape states it: <c>fGtext</c> and kerning, nothing else.</summary>
    private const uint ArchFlags = 0xF2BB5200;

    /// <summary>Its <c>adjustValue</c>, which is −180 degrees in 16.16 fixed point.</summary>
    private const uint ArchAdjust = 0xFF4C000B;

    /// <summary>The arch preset the shape type 144 names.</summary>
    private const string ArchType = "fontwork-arch-up-curve";

    private readonly SlideFonts _fonts = new();

    /// <summary>
    /// The whole reader, against the engine called with the values it should have decoded.
    /// </summary>
    [Fact]
    public void AnArchWordArtIsTheSameCurvesTheEngineDrawsFromItsDecodedProperties()
    {
        GraphicsPath? read = PptFontwork.Outline(Arch(), Box, _fonts);
        read.ShouldNotBeNull();

        read.Commands.ShouldBe(Expected(ArchType, [ArchAngleInDegrees]).Commands);
    }

    /// <summary>
    /// A polar handle's adjustment is 16.16 fixed point, and reading it as a plain integer is not
    /// a rounding difference.
    /// </summary>
    /// <remarks>
    /// <c>msdffimp.cxx</c>:2166-2174 sets the conversion bit for a <c>POLAR</c> handle whose
    /// <c>nPositionY</c> is at most <c>0x107</c>, and <c>:2591-2599</c> divides that adjustment by
    /// 65536. <c>mso_sptTextArchUpCurveHandle</c> is such a handle.
    /// </remarks>
    [Fact]
    public void AnArchsAdjustmentIsAnAngleAndNotAViewBoxCoordinate()
    {
        GraphicsPath? read = PptFontwork.Outline(Arch(), Box, _fonts);
        read.ShouldNotBeNull();

        read.Commands.ShouldNotBe(
            Expected(ArchType, [unchecked((int)ArchAdjust)]).Commands,
            "the stored value is a fixed-point degree, not a number in the 21600 view box");
    }

    /// <summary>
    /// And a shape whose default handle is not polar keeps its adjustment as stated.
    /// </summary>
    /// <remarks>
    /// <c>mso_sptTextPlainTextHandle</c> is <c>SvxMSDffHandleFlags::RANGE</c>, so nothing divides
    /// it. This is <c>pres_ioc_phuket.ppt</c>'s shape.
    /// </remarks>
    [Fact]
    public void APlainTextWordArtsAdjustmentIsNotDivided()
    {
        EscherShape shape = WordArt(
            136, Str(192, Text), Str(197, Face), Num(255, 0xFFFF5700), Num(327, 10707));

        GraphicsPath? read = PptFontwork.Outline(shape, Box, _fonts);
        read.ShouldNotBeNull();

        read.Commands.ShouldBe(Expected("fontwork-plain-text", [10707]).Commands);
    }

    /// <summary>
    /// A shape with the text-path type and no <c>fGtext</c> bit is an ordinary shape.
    /// </summary>
    /// <remarks>
    /// The bit is the whole test at <c>msdffimp.cxx</c>:4424-4425, which is why a reader that keys
    /// on the type instead would draw a WordArt for a shape the reference draws as its geometry.
    /// </remarks>
    [Fact]
    public void AShapeThatDoesNotStateTheTextPathBitIsNotWordArt()
    {
        EscherShape shape = WordArt(
            144,
            Str(192, Text),
            Str(197, Face),
            Num(255, ArchFlags & ~0x4000u),
            Num(327, ArchAdjust));

        PptFontwork.IsWordArt(shape.Properties).ShouldBeFalse();
        PptFontwork.Outline(shape, Box, _fonts).ShouldBeNull();
    }

    /// <summary>
    /// And a shape carrying the bit outside the text-path band names no preset, so it draws
    /// nothing here.
    /// </summary>
    /// <remarks>
    /// <c>EnhancedCustomShapeTypeNames::Get</c> names a <c>fontwork-*</c> or <c>mso-spt*</c> preset
    /// only for 136…175 (<c>svx/source/customshapes/EnhancedCustomShapeTypeNames.cxx</c>:171-211).
    /// Censused over the 51 corpus <c>.ppt</c>: two shapes carry the bit and both are in the band
    /// (<c>probes/slides-r107/fgtext.txt</c>).
    /// </remarks>
    [Fact]
    public void AShapeOutsideTheTextPathBandDrawsNothing()
    {
        EscherShape shape = WordArt(1, Str(192, Text), Str(197, Face), Num(255, ArchFlags));

        PptFontwork.IsWordArt(shape.Properties).ShouldBeTrue();
        PptFontwork.Outline(shape, Box, _fonts).ShouldBeNull();
    }

    /// <summary>A WordArt stating no text draws nothing rather than an empty warp.</summary>
    [Fact]
    public void AWordArtWithNoTextDrawsNothing()
        => PptFontwork.Outline(
                WordArt(144, Str(197, Face), Num(255, ArchFlags), Num(327, ArchAdjust)),
                Box,
                _fonts)
            .ShouldBeNull();

    /// <summary>
    /// A carriage return, a line feed and the two together are one break each.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SvxMSDffManager::ReadObjText</c> (<c>msdffimp.cxx</c>:3698-3717) breaks on <c>0x0a</c> and
    /// on <c>0x0d</c> and swallows the other when it follows, so <c>CRLF</c> is one paragraph break
    /// and not two.
    /// </para>
    /// <para>
    /// <strong>These four cases pin that a break happens and not that the partner is swallowed</strong>,
    /// and that is measured rather than hoped: with the split replaced by <c>[text]</c> all four
    /// fail, and with it replaced by a naive <c>text.Split('\r', '\n')</c> all four still pass —
    /// because <see cref="Fontwork.Outline"/> drops a blank line rather than laying it out, so the
    /// spurious empty paragraph a naive split produces between <c>CR</c> and <c>LF</c> disappears
    /// before any geometry is built. The rule is followed anyway because it is the reference's, but
    /// no assertion in this tree can currently tell the two apart.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("Do you\rknow")]
    [InlineData("Do you\nknow")]
    [InlineData("Do you\r\nknow")]
    [InlineData("Do you\n\rknow")]
    public void EveryLineEndingIsOneParagraphBreak(string text)
    {
        EscherShape shape = WordArt(
            144, Str(192, text), Str(197, Face), Num(255, ArchFlags), Num(327, ArchAdjust));

        GraphicsPath? read = PptFontwork.Outline(shape, Box, _fonts);
        read.ShouldNotBeNull();

        read.Commands.ShouldBe(
            Expected(ArchType, [ArchAngleInDegrees], ["Do you", "know"]).Commands);
    }

    /// <summary>
    /// <c>gtextAlign</c> is read, and an absent one is centre rather than left.
    /// </summary>
    /// <remarks>
    /// <c>msdffimp.cxx</c>:4466 defaults it to <c>mso_alignTextCenter</c>. The three justifying
    /// modes reach <c>SDRTEXTHORZADJUST_BLOCK</c>, which the Fontwork layouter treats as left
    /// (<c>EnhancedCustomShapeFontWork.cxx</c>:621, <c>// don't know</c>).
    /// </remarks>
    [Theory]
    [InlineData(-1, FontworkAlignment.Centre)]  // not stated
    [InlineData(0, FontworkAlignment.Left)]     // mso_alignTextStretch
    [InlineData(1, FontworkAlignment.Centre)]   // mso_alignTextCenter
    [InlineData(2, FontworkAlignment.Left)]     // mso_alignTextLeft
    [InlineData(3, FontworkAlignment.Right)]    // mso_alignTextRight
    [InlineData(4, FontworkAlignment.Left)]     // mso_alignTextLetterJust
    [InlineData(5, FontworkAlignment.Left)]     // mso_alignTextWordJust
    [InlineData(6, FontworkAlignment.Centre)]   // mso_alignTextInvalid
    public void TheStatedAlignmentIsWhereTheTextSitsAlongThePath(
        int stated, FontworkAlignment expected)
    {
        List<Property> properties = [Str(192, Text), Str(197, Face)];
        if (stated >= 0) properties.Add(Num(194, (uint)stated));
        properties.Add(Num(255, ArchFlags));
        properties.Add(Num(327, ArchAdjust));

        GraphicsPath? read = PptFontwork.Outline(WordArt(144, [.. properties]), Box, _fonts);
        read.ShouldNotBeNull();

        read.Commands.ShouldBe(
            Expected(ArchType, [ArchAngleInDegrees], alignment: expected).Commands);
    }

    /// <summary>
    /// <c>ScaleX</c> is read from the file rather than derived from the preset, and a shape asking
    /// to keep a size it does not state cannot keep it.
    /// </summary>
    /// <remarks>
    /// <c>ApplyAttributes</c> writes <c>ScaleX</c> from bit <c>0x40</c> of property 255 literally
    /// (<c>msdffimp.cxx</c>:2556-2559), where the DrawingML path derives it for the four
    /// <c>*Curve</c> presets — of which the arch is one. So the two paths answer differently for
    /// the same preset, and this is where that is pinned.
    /// </remarks>
    [Fact]
    public void TheArchKeepsItsStatedSizeOnlyWhenTheFileSaysSoAndStatesOne()
    {
        // 0x40 set and `gtextSize` at 36 pt in 16.16 fixed point.
        EscherShape keeps = WordArt(
            144,
            Str(192, Text),
            Num(195, 36u << 16),
            Str(197, Face),
            Num(255, ArchFlags | 0x40),
            Num(327, ArchAdjust));

        PptFontwork.Outline(keeps, Box, _fonts)!.Commands.ShouldBe(
            Expected(
                ArchType,
                [ArchAngleInDegrees],
                keepsFontSize: true,
                fontSize: Length.FromPoints(36)).Commands);

        // The same bit with no size stated: there is nothing to keep, so the warp fills the shape.
        EscherShape cannot = WordArt(
            144,
            Str(192, Text),
            Str(197, Face),
            Num(255, ArchFlags | 0x40),
            Num(327, ArchAdjust));

        PptFontwork.Outline(cannot, Box, _fonts)!.Commands.ShouldBe(
            Expected(ArchType, [ArchAngleInDegrees]).Commands);
    }

    /// <summary>−180 degrees, which is what <c>0xFF4C000B</c> is once divided by 65536.</summary>
    private static double ArchAngleInDegrees => unchecked((int)ArchAdjust) / 65536.0;

    /// <summary>The AOD shape, property for property as the file states it.</summary>
    private static EscherShape Arch()
        => WordArt(144, Str(192, Text), Str(197, Face), Num(255, ArchFlags), Num(327, ArchAdjust));

    /// <summary>What the shared engine draws when it is handed the decoded values directly.</summary>
    private GraphicsPath Expected(
        string type,
        IReadOnlyList<double> adjustments,
        IReadOnlyList<string>? lines = null,
        FontworkAlignment alignment = FontworkAlignment.Centre,
        bool keepsFontSize = false,
        Length fontSize = default)
    {
        (OpenTypeFace? face, FontReference? _) = _fonts.Resolve(Face, 400, false);
        face.ShouldNotBeNull("the suite needs a face carrying outlines for " + Face);

        GraphicsPath? path = Fontwork.Outline(new FontworkRequest
        {
            FontworkType = type,
            AdjustmentValues = adjustments,
            KeepsFontSize = keepsFontSize,
            Box = Box,
            Lines = lines ?? [Text],
            Face = face,
            FontSize = fontSize,
            Alignment = alignment,
        });

        path.ShouldNotBeNull();
        return path;
    }

    /// <summary>One <c>msofbtOPT</c> entry: an identifier and either a value or a complex blob.</summary>
    private readonly record struct Property(ushort Id, uint Value, byte[]? Data);

    /// <summary>A property holding a plain value.</summary>
    private static Property Num(ushort id, uint value) => new(id, value, null);

    /// <summary>A property holding text, which the format stores as UTF-16 with a terminator.</summary>
    private static Property Str(ushort id, string text)
        => new(id, 0, Encoding.Unicode.GetBytes(text + '\0'));

    /// <summary>A shape carrying a property table built from the entries given, in order.</summary>
    /// <remarks>
    /// The complex block begins exactly <c>6 × count</c> bytes into the payload and the entries
    /// hold only their own lengths, so the properties have to be written in identifier order for
    /// the values to line up — which is what a real <c>msofbtOPT</c> does.
    /// </remarks>
    private static EscherShape WordArt(ushort shapeType, params Property[] properties)
    {
        List<byte> payload = [];
        List<byte> complex = [];

        foreach (Property property in properties)
        {
            ushort raw = property.Data is null ? property.Id : (ushort)(property.Id | 0x8000);
            payload.AddRange(BitConverter.GetBytes(raw));
            payload.AddRange(BitConverter.GetBytes(
                property.Data is null ? property.Value : (uint)property.Data.Length));
            if (property.Data is not null) complex.AddRange(property.Data);
        }

        payload.AddRange(complex);

        return new EscherShape
        {
            ShapeType = shapeType,
            Properties = EscherPropertyTable.Read(
                CollectionsMarshal.AsSpan(payload), properties.Length),
        };
    }
}

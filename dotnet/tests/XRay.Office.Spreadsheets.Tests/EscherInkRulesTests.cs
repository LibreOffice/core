using XRay.Office.Core.Graphics;
using XRay.Office.MsBinary.Escher;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The three rules that decide a BIFF shape's fill and outline, none of which is "the shape said
/// so".
/// </summary>
/// <remarks>
/// <para>
/// Round 84 read the four Escher ink properties and took two conservative positions that between
/// them resolved almost nothing: <em>presence is the test</em>, so a colour was taken only where
/// the shape stated one, and <em>only the literal <c>MSO_CLR</c> form is honoured</em>, so a
/// palette reference yielded nothing. Measured over the 64 corpus <c>.xls</c>
/// (<c>probes/sheet-shapefill-r92/msoclr-census.txt</c>): <b>92 of the 106</b> fill and line
/// colours a worksheet shape states are palette references and <b>14</b> are literals.
/// </para>
/// <para>
/// The values here are <c>TICAPCapability_Final.xls</c>' own — its two <c>Instructions</c> text
/// boxes state <c>fillColor 0x08000041</c> and <c>lineColor 0x08000040</c> and nothing else — and
/// the answers are 26.2.4.2's, read out of its flat ODF of that file, which gives both boxes
/// <c>draw:fill-color="#ffffff"</c> and <c>svg:stroke-color="#000000"</c> at
/// <c>svg:stroke-width="0.0102in"</c>.
/// </para>
/// </remarks>
public sealed class EscherInkRulesTests
{
    /// <summary><c>mso_sptTextBox</c>, which is filled and stroked by default.</summary>
    private const ushort TextBox = 202;

    /// <summary><c>mso_sptPictureFrame</c>, the one type that is neither.</summary>
    private const ushort PictureFrame = 75;

    /// <summary>Excel's palette: 64 is the window text colour and 65 the window background.</summary>
    private static Colour? Excel(int index) => index switch
    {
        64 => Colour.Black,
        65 => Colour.White,
        _ => null,
    };

    // ------------------------------------------------------- the colour is usually a reference

    /// <summary>
    /// A text box stating only two palette references is filled white and outlined black at the
    /// format's default width.
    /// </summary>
    [Fact]
    public void APaletteReferenceIsResolvedThroughTheWorkbooksColours()
    {
        EscherInk.Ink ink = EscherInk.Read(
            Table((EscherPropertyIds.FillColour, 0x08000041), (EscherPropertyIds.LineColour, 0x08000040)),
            TextBox,
            Excel);

        ink.Fill.ShouldBe(Colour.White);
        ink.Stroke.ShouldBe(Colour.Black);
        ink.StrokeWidth.Points.ShouldBe(0.75, 0.001);
    }

    /// <summary>The literal form is <c>0x00BBGGRR</c> and survives unchanged.</summary>
    /// <remarks>
    /// <c>orbus_togaf_tool_csq.xls</c>' two text boxes state exactly these, and they are the pair
    /// that tells a right decode from a byte-reversed one.
    /// </remarks>
    [Fact]
    public void ALiteralColourIsStillReadBlueGreenRed()
    {
        EscherInk.Ink ink = EscherInk.Read(
            Table((EscherPropertyIds.FillColour, 0x00D6F6FB), (EscherPropertyIds.LineColour, 0x00A1EAED)),
            TextBox,
            Excel);

        ink.Fill.ShouldBe(Colour.FromRgb(0xFBF6D6));
        ink.Stroke.ShouldBe(Colour.FromRgb(0xEDEAA1));
    }

    /// <summary>
    /// A reference the host's palette cannot answer falls back to white for a fill and black for
    /// an outline, which is what <c>MSO_CLR_ToColor</c> does per property.
    /// </summary>
    /// <remarks><c>filter/source/msfilter/msdffimp.cxx</c>:3440-3453.</remarks>
    [Fact]
    public void AnUnresolvableReferenceFallsBackPerProperty()
    {
        EscherInk.Ink ink = EscherInk.Read(
            Table((EscherPropertyIds.FillColour, 0x08000200), (EscherPropertyIds.LineColour, 0x08000200)),
            TextBox,
            Excel);

        ink.Fill.ShouldBe(Colour.White);
        ink.Stroke.ShouldBe(Colour.Black);
    }

    // ------------------------------------------------------------- absence is not "no ink"

    /// <summary>
    /// A text box that states no colour at all is still filled white and outlined black.
    /// </summary>
    /// <remarks>
    /// <c>mso_PropSetDefaults</c> gives <c>fillColor</c> the value <c>0xffffff</c> and
    /// <c>lineColor</c> zero (<c>filter/source/msfilter/dffpropset.cxx</c>), and Calc puts the
    /// window background on a filled object with no colour a second time
    /// (<c>xiescher.cxx</c>:3693-3695). <c>014_Contextures_chart_sample_991ecfc5.xls</c>'
    /// <c>Rectangle 6</c> is the corpus witness for the line half: it states <c>fLine</c> true and
    /// no <c>lineColor</c>, and 26.2.4.2 draws it <c>#000000</c>.
    /// </remarks>
    [Fact]
    public void AStatedFillWithNoColourIsWhiteAndAStatedLineWithNoneIsBlack()
    {
        EscherInk.Ink ink = EscherInk.Read(Table(), TextBox, Excel);

        ink.Fill.ShouldBe(Colour.White);
        ink.Stroke.ShouldBe(Colour.Black);
    }

    // ------------------------------------------------- an unstated boolean is the type's answer

    /// <summary>
    /// A picture frame stating nothing is neither filled nor stroked, where a text box stating
    /// nothing is both.
    /// </summary>
    /// <remarks>
    /// <c>mso_DefaultFillingTable</c> and <c>mso_DefaultStrokingTable</c>,
    /// <c>svx/source/customshapes/EnhancedCustomShapeGeometry.cxx</c>:6156-6213. Shape 75 is the
    /// only entry in the stroking table.
    /// </remarks>
    [Fact]
    public void AnUnstatedBooleanIsDecidedByTheShapeType()
    {
        EscherInk.Read(Table(), PictureFrame, Excel).HasInk.ShouldBeFalse();
        EscherInk.Read(Table(), TextBox, Excel).HasInk.ShouldBeTrue();
    }

    /// <summary>A stated boolean beats the type, in both directions.</summary>
    /// <remarks>
    /// 216 of the corpus's <c>.xls</c> worksheet shapes state property 511 as
    /// <c>0x00080000</c> — <c>fLine</c> hard and false — and 52 state 447 as <c>0x00100010</c>,
    /// which is <c>fFilled</c> hard and true.
    /// </remarks>
    [Fact]
    public void AStatedBooleanBeatsTheShapeType()
    {
        EscherInk.Ink off = EscherInk.Read(
            Table((FillGroup, 0x00100000), (LineGroup, 0x00080000),
                  (EscherPropertyIds.FillColour, 0x08000041)),
            TextBox,
            Excel);

        off.Fill.ShouldBeNull();
        off.Stroke.ShouldBeNull();

        EscherInk.Ink on = EscherInk.Read(
            Table((FillGroup, 0x00100010), (LineGroup, 0x00080008),
                  (EscherPropertyIds.FillColour, 0x08000041)),
            PictureFrame,
            Excel);

        on.Fill.ShouldBe(Colour.White);
        on.Stroke.ShouldBe(Colour.Black);
    }

    /// <summary>
    /// A fill type past the ones the reference paints leaves the shape unfilled.
    /// </summary>
    /// <remarks>
    /// <c>mso_fillBackground</c> is 9 and falls into the <c>default:</c> of
    /// <c>ApplyFillAttributes</c>' switch, which leaves the style at <c>NONE</c>
    /// (<c>msdffimp.cxx</c>:1330-1400).
    /// </remarks>
    [Fact]
    public void AFillTypeTheReferencePaintsNothingForIsNotFilled()
    {
        EscherInk.Read(
                Table((EscherPropertyIds.FillType, 9), (EscherPropertyIds.FillColour, 0x08000041)),
                TextBox,
                Excel)
            .Fill.ShouldBeNull();

        EscherInk.Read(
                Table((EscherPropertyIds.FillType, 4), (EscherPropertyIds.FillColour, 0x08000041)),
                TextBox,
                Excel)
            .Fill.ShouldBe(Colour.White, "a gradient is painted as its foreground colour");
    }

    /// <summary>The identifier a boolean group is written under: <c>fFilled | 31</c>.</summary>
    private const ushort FillGroup = 447;

    /// <summary><c>fLine | 31</c>.</summary>
    private const ushort LineGroup = 511;

    private static EscherPropertyTable Table(params (ushort Id, uint Value)[] entries)
    {
        byte[] content = new byte[entries.Length * 6];
        for (int i = 0; i < entries.Length; i++)
        {
            content[i * 6] = (byte)entries[i].Id;
            content[(i * 6) + 1] = (byte)(entries[i].Id >> 8);
            content[(i * 6) + 2] = (byte)entries[i].Value;
            content[(i * 6) + 3] = (byte)(entries[i].Value >> 8);
            content[(i * 6) + 4] = (byte)(entries[i].Value >> 16);
            content[(i * 6) + 5] = (byte)(entries[i].Value >> 24);
        }

        return EscherPropertyTable.Read(content, entries.Length);
    }
}

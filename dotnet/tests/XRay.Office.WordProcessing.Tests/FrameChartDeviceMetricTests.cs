using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A chart in a Writer frame takes its <em>vertical</em> metrics from <c>chart2</c>'s own 96 dpi
/// device too, and not from the face's design units.
/// </summary>
/// <remarks>
/// <para>
/// The counterpart of <see cref="FrameChartDeviceAdvanceTests"/> and the other half of one rule.
/// <c>chart2</c>'s view builds a chart's labels as plain text shapes on the
/// <c>VirtualDevice</c> that <c>DrawModelWrapper</c> creates from
/// <c>Application::GetDefaultDevice()</c>
/// (<c>chart2/source/view/main/DrawModelWrapper.cxx</c>:88-99), and that device is 96 dpi
/// (<c>SvpSalGraphics::GetResolution</c>, <c>vcl/headless/svpgdi.cxx</c>:44). An
/// <c>OutputDevice</c> instantiates a font at a whole number of device pixels, so
/// <em>every</em> metric it is then asked for — ascent, descent, and therefore the line height
/// and the baseline — is derived at that rounded size and lands on a whole 0.75 pt pixel.
/// </para>
/// <para>
/// <strong>The rule is restated here rather than read from the code under test.</strong> The
/// expected values are computed from the faces' own <c>hhea</c> numbers, quoted as constants, by
/// arithmetic written out in full: the em in whole pixels, each metric rounded to whole pixels at
/// that em, and back through whole hundredths of a millimetre, which is the device's map unit.
/// Both roundings are half away from zero, as C++ <c>round</c> and <c>llround</c> are.
/// </para>
/// <para>
/// Measured on both reference binaries in <c>probes/chart-vertical/</c>: three faces × twelve
/// sizes × two binaries × a deck and a Writer document, <strong>144 of 144</strong>
/// baseline-to-baseline distances inside 0.019 pt of this, where scaling the face's own metrics
/// exactly is out by as much as 1.208 pt.
/// </para>
/// </remarks>
public class FrameChartDeviceMetricTests
{
    /// <summary>Liberation Mono: 2048 per em, and no line gap at all.</summary>
    private const int MonoAscent = 1705;
    private const int MonoDescent = 615;

    /// <summary>Liberation Sans, whose <c>hhea</c> line gap is 67/2048 and is not zero.</summary>
    private const int SansAscent = 1854;
    private const int SansDescent = 434;
    private const int SansLineGap = 67;

    private const int UnitsPerEm = 2048;

    /// <summary>
    /// Sizes that separate the rule from any fixed fraction of the em.
    /// </summary>
    /// <remarks>
    /// At 10 pt the device sets <b>13</b> pixels for 13.333 and the line comes out <em>shorter</em>
    /// than the design metric; at 11 pt it sets <b>15</b> for 14.667 and it comes out
    /// <em>taller</em>; at 12 pt 16 is already whole and the two answers meet. A rule with one
    /// sign fails one of the three and a rule with no rounding fails two.
    /// </remarks>
    public static TheoryData<double, int> Sizes => new() { { 10.0, 13 }, { 11.0, 15 }, { 12.0, 16 } };

    [Theory]
    [MemberData(nameof(Sizes))]
    public void AChartsLineHeightIsDerivedAtItsWholePixelEm(double points, int pixels)
    {
        Length size = Length.FromPoints(points);
        PixelEm(size).ShouldBe(pixels, $"{points} pt is {pixels} whole pixels at 96 dpi");

        ChartFace face = ChartFace.For("Liberation Mono");

        face.LineHeightAt(size).Emu.ShouldBe(
            Height(MonoAscent, MonoDescent, pixels).Emu,
            $"ascent and descent at {pixels} px, and no leading");

        face.AscentAt(size).Emu.ShouldBe(
            Pixels(MonoAscent, pixels).Emu, $"the ascent at {pixels} px");
    }

    /// <summary>
    /// The sawtooth: ten point comes out short of the design metric and eleven point long.
    /// </summary>
    /// <remarks>
    /// Stated on its own because it is the half a "charts measure small" rule gets wrong. The
    /// correction is <c>round(px)/px</c> and that is above one as often as below it — the
    /// reference stacks Liberation Sans at 11.254 pt at 10 pt against a design 11.499, and at
    /// 12.756 at 11 pt against a design 12.649.
    /// </remarks>
    [Fact]
    public void TenPointStacksShorterThanTheDesignMetricAndElevenPointTaller()
    {
        ChartFace face = ChartFace.For("Liberation Sans");

        Design(SansAscent + SansDescent + SansLineGap, 10.0)
            .ShouldBeGreaterThan(face.LineHeightAt(Length.FromPoints(10.0)).Points + 0.2);

        face.LineHeightAt(Length.FromPoints(11.0)).Points
            .ShouldBeGreaterThan(Design(SansAscent + SansDescent + SansLineGap, 11.0) + 0.05);
    }

    /// <summary>
    /// The face's external leading is not in the line, because EditEngine does not add it.
    /// </summary>
    /// <remarks>
    /// <c>ImpEditEngine::RecalcFormatterFontMetrics</c> adds it only under
    /// <c>IsAddExtLeading()</c> (<c>editeng/source/editeng/impedit3.cxx</c>:3133-3135), which is
    /// false unless Writer's own compatibility flag turns it on — and a chart's label is an
    /// EditEngine text made by <c>chart2</c>, not a Writer paragraph. Liberation Sans is the face
    /// that can tell: its gap is 67/2048, where Carlito's is zero and every OOXML default would
    /// hide this.
    /// </remarks>
    [Fact]
    public void TheFacesExternalLeadingIsNotPartOfAChartsLine()
    {
        Length size = Length.FromPoints(12.0);
        ChartFace face = ChartFace.For("Liberation Sans");

        // 12 pt is 16 whole pixels, so the device applies no correction of its own and what is
        // left is the leading question alone.
        PixelEm(size).ShouldBe(16);

        face.LineHeightAt(size).Emu.ShouldBe(Height(SansAscent, SansDescent, 16).Emu);
        face.LineHeightAt(size).Points.ShouldBeLessThan(
            Design(SansAscent + SansDescent + SansLineGap, 12.0) - 0.3);
    }

    /// <summary>
    /// The height and the ascent are on the same device, which is what keeps a label on its mark.
    /// </summary>
    /// <remarks>
    /// A chart label is drawn at <c>blockCentre − blockHeight/2 + ascent</c>, so the quantity that
    /// decides where it sits against its tick is <c>ascent − height/2</c> — which is what
    /// <c>probes/chart-vertical/tickoffset.py</c> reads out of both reference binaries, and which
    /// is <b>within 0.02 pt of this on 72 of 72 cases against 24.2.7.2</b> with no free parameter
    /// at all, and on 72 of 72 against 26.2.4.2 once one constant of a hundredth of a millimetre
    /// is allowed for. Moving one of the two and not the other is the failure mode round 60
    /// recorded for the sheets track: the errors cancelled on a single-line label and showed up
    /// everywhere else.
    /// </remarks>
    [Theory]
    [MemberData(nameof(Sizes))]
    public void ALabelSitsOnItsMarkByTheDevicesOwnAscentAndHeight(double points, int pixels)
    {
        Length size = Length.FromPoints(points);
        ChartFace face = ChartFace.For("Liberation Mono");

        double offset = face.AscentAt(size).Points - (face.LineHeightAt(size).Points / 2.0);
        double expected =
            Pixels(MonoAscent, pixels).Points - (Height(MonoAscent, MonoDescent, pixels).Points / 2.0);

        offset.ShouldBe(expected, 0.001);
    }

    /// <summary>The em in whole 96 dpi pixels, through the device's own map unit.</summary>
    private static int PixelEm(Length size)
        => (int)Math.Round(size.Mm100 * 96.0 / 2540.0, MidpointRounding.AwayFromZero);

    /// <summary>A design-unit metric at a whole-pixel em, back in hundredths of a millimetre.</summary>
    private static Length Pixels(int designUnits, int pixelEm)
        => Length.FromMm100((long)Math.Round(
            Math.Round(designUnits * (double)pixelEm / UnitsPerEm, MidpointRounding.AwayFromZero)
            * 2540.0 / 96.0, MidpointRounding.AwayFromZero));

    /// <summary>
    /// EditEngine's line height: the taller of converting each metric on its own and converting
    /// their sum in one step (<c>editeng/source/editeng/impedit3.cxx</c>:1516-1518).
    /// </summary>
    private static Length Height(int ascent, int descent, int pixelEm)
    {
        long up = (long)Math.Round(
            ascent * (double)pixelEm / UnitsPerEm, MidpointRounding.AwayFromZero);
        long down = (long)Math.Round(
            descent * (double)pixelEm / UnitsPerEm, MidpointRounding.AwayFromZero);

        Length separately = Length.FromMm100(
            (long)Math.Round(up * 2540.0 / 96.0, MidpointRounding.AwayFromZero)
            + (long)Math.Round(down * 2540.0 / 96.0, MidpointRounding.AwayFromZero));
        Length together = Length.FromMm100(
            (long)Math.Round((up + down) * 2540.0 / 96.0, MidpointRounding.AwayFromZero));

        return separately > together ? separately : together;
    }

    /// <summary>The same metric scaled exactly, which is what the device is <em>not</em>.</summary>
    private static double Design(int designUnits, double points)
        => designUnits * points / UnitsPerEm;
}

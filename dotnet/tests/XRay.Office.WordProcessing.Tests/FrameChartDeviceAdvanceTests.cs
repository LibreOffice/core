using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A chart in a Writer frame takes <c>chart2</c>'s own 96 dpi device, exactly as a workbook's
/// and a slide's do.
/// </summary>
/// <remarks>
/// <para>
/// A chart's labels are not laid out by Writer. <c>chart2</c>'s view builds them as plain text
/// shapes on the <c>VirtualDevice</c> that <c>DrawModelWrapper</c> creates from
/// <c>Application::GetDefaultDevice()</c> with <c>MapUnit::Map100thMM</c>
/// (<c>chart2/source/view/main/DrawModelWrapper.cxx</c>:88-99), and that device is 96 dpi
/// (<c>SvpSalGraphics::GetResolution</c>, <c>vcl/headless/svpgdi.cxx</c>:44). A font is
/// instantiated at a whole number of device pixels, so the advance a chart measures is scaled by
/// <c>round(px)/px</c> before anything on the page sees it.
/// </para>
/// <para>
/// <strong>This is asserted on <see cref="ChartFace.Shape"/> rather than on a rendered document
/// because that one method is both paths.</strong> <see cref="ChartFace.Measure"/> reserves the
/// room from it and <c>FrameChart</c> draws the glyphs it returns, so a rule applied here cannot
/// come apart the way 24.2.7.2's does — that binary right-aligns the value axis' labels on their
/// design widths and then draws them from the device's narrower array, measured in
/// <c>probes/chart-text-metafile/facts.py</c>.
/// </para>
/// <para>
/// Liberation Mono is the face because every glyph in it has one design advance, 1229/2048 em,
/// so a per-glyph effect cannot hide behind a kern pair or a proportional width.
/// </para>
/// </remarks>
public class FrameChartDeviceAdvanceTests
{
    /// <summary>Liberation Mono's advance for every glyph, from its <c>hmtx</c>.</summary>
    private const double MonoAdvanceEm = 1229.0 / 2048.0;

    private const string Face = "Liberation Mono";

    /// <summary>Ten of one glyph, so the mean advance is the advance.</summary>
    private const string Text = "0000000000";

    /// <summary>Sizes that separate the rule from a constant: narrow, wide, and exact.</summary>
    /// <remarks>
    /// At 10 pt the device sets 13 pixels for 13.333 and the advance comes back 2.5% narrow; at
    /// 11 pt it sets 15 for 14.667 and it comes back 2.3% <em>wide</em>; at 12 pt 16 is already
    /// whole and nothing is applied. A correction with one sign, or one magnitude, fails two of
    /// the three. Both reference binaries follow the same sawtooth over twelve sizes — see
    /// <c>probes/chart-text-metafile/results.md</c>.
    /// </remarks>
    public static TheoryData<double, int> Sizes => new() { { 10.0, 13 }, { 11.0, 15 }, { 12.0, 16 } };

    [Theory]
    [MemberData(nameof(Sizes))]
    public void AChartsAdvanceIsScaledByItsWholePixelEm(double points, int pixels)
    {
        Length size = Length.FromPoints(points);

        // The premise, stated so a failure says which half broke: this size really does land on
        // that whole pixel count on a 96 dpi device.
        Math.Round(points * 96.0 / 72.0).ShouldBe(pixels);

        // The shipped rule, pinned against an independently written one rather than against
        // itself: this is the only place the two are allowed to be the same number.
        MetricGrid.Chart.PixelEmScale(size).ShouldBe(pixels / (points * 96.0 / 72.0), 0.0005);

        Advance(points).ShouldBe(MonoAdvanceEm * pixels / (points * 96.0 / 72.0), 0.0005);
    }

    /// <summary>
    /// The narrow case and the wide case fall on opposite sides of the face's own advance.
    /// </summary>
    /// <remarks>
    /// Stated separately because it is the part a sign error would survive: applying
    /// <c>px/round(px)</c> instead of <c>round(px)/px</c> passes nothing here, and applying the
    /// magnitude without the sign passes the theory above at 10 pt and fails at 11.
    /// </remarks>
    [Fact]
    public void TenPointComesBackNarrowAndElevenPointComesBackWide()
    {
        double ten = Advance(10.0);
        double eleven = Advance(11.0);

        ten.ShouldBeLessThan(MonoAdvanceEm * 0.99);
        eleven.ShouldBeGreaterThan(MonoAdvanceEm * 1.01);
        Advance(12.0).ShouldBe(MonoAdvanceEm, 0.0005);
    }

    /// <summary>The mean advance of ten identical glyphs, in ems of the size asked for.</summary>
    private static double Advance(double points)
    {
        Length size = Length.FromPoints(points);
        ChartRun run = ChartFace.For(Face).Shape(Text, size).ShouldNotBeNull();
        return run.Width.Emu / (double)Text.Length / size.Emu;
    }
}

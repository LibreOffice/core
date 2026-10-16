using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The rectangle a cropped picture is drawn into.
/// </summary>
/// <remarks>
/// <para>
/// The numbers are read off LibreOffice's own PDF, not derived. <c>Thailand17.ppt</c> page 22
/// carries a near-full-slide photograph whose Escher property table states
/// <c>cropFromLeft = 5243</c> (8.000%), <c>cropFromRight = 1748</c> (2.667%) and
/// <c>cropFromBottom = 6554</c> (10.00%), on a shape anchored at 655.625 × 528.25 pt. The
/// reference draws the picture into <c>(−47.96, −58.93)–(685.96, 528.04)</c> — 733.92 × 586.97 —
/// and clips it to the anchor.
/// </para>
/// <para>
/// That is the whole feature: a crop is a larger destination plus the clip every backend already
/// applies. It is also why the <c>+ 1</c> in LibreOffice's <c>lcl_ApplyCropping</c> must not be
/// ported — that rounding runs in the pixel space of a bitmap being trimmed, and plain fractions
/// reconcile the reference's rectangle here to better than a twentieth of a point.
/// </para>
/// </remarks>
public class PictureCropTests
{
    /// <summary>A point in EMUs, for stating the measured rectangles in the unit they were read in.</summary>
    private static Length Points(double value) => Length.FromEmu((long)Math.Round(value * 12700));

    /// <summary>The anchor <c>Thailand17.ppt</c> page 22 states, in points.</summary>
    private static DocRect Anchor => new(Points(10.75), Points(-0.25), Points(655.625), Points(528.25));

    [Fact]
    public void UncroppedMatchesTheReferenceDestination()
    {
        DocRect? placed = PictureCrop.Uncropped(Anchor, left: 5243 / 65536.0, top: 0,
            right: 1748 / 65536.0, bottom: 6554 / 65536.0);

        placed.ShouldNotBeNull();

        placed.Value.Width.Points.ShouldBe(733.92, 0.05);
        placed.Value.Height.Points.ShouldBe(586.97, 0.05);
        placed.Value.Left.Points.ShouldBe(-47.96, 0.05);

        // Nothing is cropped from the top, so the top edge is the anchor's own — which is the
        // half of the result that says the offset follows the crop rather than the size.
        placed.Value.Top.ShouldBe(Anchor.Top);
    }

    /// <summary>
    /// The whole point of the operation: the surviving fraction of the picture lands exactly on
    /// the rectangle the shape occupies, whatever the crop was.
    /// </summary>
    [Theory]
    [InlineData(0.08, 0.0, 0.02667, 0.10)]
    [InlineData(0.5, 0.5, 0.0, 0.0)]
    [InlineData(0.01, 0.02, 0.03, 0.04)]
    public void TheSurvivingPartFillsTheDestination(double left, double top, double right, double bottom)
    {
        DocRect? placed = PictureCrop.Uncropped(Anchor, left, top, right, bottom);

        placed.ShouldNotBeNull();

        double width = placed.Value.Width.Emu;
        double height = placed.Value.Height.Emu;

        (placed.Value.Left.Emu + (left * width)).ShouldBe(Anchor.Left.Emu, 1);
        (placed.Value.Top.Emu + (top * height)).ShouldBe(Anchor.Top.Emu, 1);
        (width * (1 - left - right)).ShouldBe(Anchor.Width.Emu, 1);
        (height * (1 - top - bottom)).ShouldBe(Anchor.Height.Emu, 1);
    }

    /// <summary>A crop that keeps nothing would divide by zero; a file can and does state one.</summary>
    [Theory]
    [InlineData(0.5, 0.0, 0.5, 0.0)]
    [InlineData(0.0, 0.9, 0.0, 0.2)]
    public void ACropThatKeepsNothingIsRefusedRatherThanDivided(
        double left, double top, double right, double bottom)
        => PictureCrop.Uncropped(Anchor, left, top, right, bottom).ShouldBeNull();

    /// <summary>A negative crop pads rather than trims, which is legal and which files state.</summary>
    [Fact]
    public void ANegativeCropShrinksThePicture()
    {
        DocRect? placed = PictureCrop.Uncropped(Anchor, left: -0.25, top: 0, right: -0.25, bottom: 0);

        placed.ShouldNotBeNull();
        ((double)placed.Value.Width.Emu).ShouldBe(Anchor.Width.Emu / 1.5, 2);
        placed.Value.Left.ShouldBeGreaterThan(Anchor.Left);
    }

    /// <summary>An uncropped picture is placed exactly where it was asked for, to the EMU.</summary>
    [Fact]
    public void NoCropIsTheIdentity()
        => PictureCrop.Uncropped(Anchor, 0, 0, 0, 0).ShouldBe(Anchor);

    [Fact]
    public void InsetLeavesTheStatedFractionsEmpty()
    {
        DocRect area = PictureCrop.Inset(Anchor, left: 0.1, top: 0.2, right: 0.3, bottom: 0.4);

        ((double)area.Width.Emu).ShouldBe(Anchor.Width.Emu * 0.6, 2);
        ((double)area.Height.Emu).ShouldBe(Anchor.Height.Emu * 0.4, 2);
        ((double)area.Left.Emu).ShouldBe(Anchor.Left.Emu + (Anchor.Width.Emu * 0.1), 2);
        ((double)area.Top.Emu).ShouldBe(Anchor.Top.Emu + (Anchor.Height.Emu * 0.2), 2);
    }

    /// <summary>A negative <c>a:fillRect</c> edge grows the fill past the shape, which is legal.</summary>
    [Fact]
    public void ANegativeInsetOverhangs()
    {
        DocRect area = PictureCrop.Inset(Anchor, left: -0.1, top: 0, right: -0.1, bottom: 0);

        area.Left.ShouldBeLessThan(Anchor.Left);
        area.Right.ShouldBeGreaterThan(Anchor.Right);
    }
}

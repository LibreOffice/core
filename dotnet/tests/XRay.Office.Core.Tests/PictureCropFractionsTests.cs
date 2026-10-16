using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The four crop fractions as a value, for the two tracks that cannot apply them where they read
/// them.
/// </summary>
/// <remarks>
/// A slide shape states its own rectangle, so <c>EscherPicture.Cropped</c> reads the properties
/// and finishes in one call. A sheet's drawing is anchored to <em>cells</em> and a Word frame is
/// placed by the layout engine, so on those two paths the fractions have to travel from the
/// reader to the painter — which is the whole reason this type exists rather than four
/// <c>double</c> parameters threaded through two model layers.
/// </remarks>
public class PictureCropFractionsTests
{
    /// <summary>288 × 216 pt at (72, 72), the rectangle both new fixtures place a picture in.</summary>
    private static DocRect Frame => new(
        Length.FromEmu(914400), Length.FromEmu(914400),
        Length.FromEmu(3657600), Length.FromEmu(2743200));

    [Fact]
    public void NoneIsTheIdentity()
    {
        PictureCropFractions.None.IsNone.ShouldBeTrue();
        PictureCropFractions.None.Apply(Frame).ShouldBe(Frame);
    }

    /// <summary>
    /// The default value is the uncropped one, which is what lets every model carrying this leave
    /// it unset and mean "the whole picture".
    /// </summary>
    [Fact]
    public void TheDefaultValueIsUncropped()
        => default(PictureCropFractions).Apply(Frame).ShouldBe(Frame);

    /// <summary>
    /// The same answer <see cref="PictureCrop.Uncropped"/> gives, because it is the same
    /// arithmetic and must not become a second copy of it.
    /// </summary>
    [Fact]
    public void ApplyIsUncropped()
    {
        DocRect? expected = PictureCrop.Uncropped(Frame, 0.10, 0.20, 0.30, 0.40);
        expected.ShouldNotBeNull();

        new PictureCropFractions(0.10, 0.20, 0.30, 0.40).Apply(Frame).ShouldBe(expected.Value);
    }

    /// <summary>
    /// The fixture's own numbers, stated here so the two readers below can be compared against a
    /// figure that came from neither of them: 288 / 0.6 = 480 pt across, 216 / 0.4 = 540 pt down.
    /// </summary>
    [Fact]
    public void TheFixtureCropIsFourHundredAndEightyByFiveHundredAndForty()
    {
        DocRect placed = new PictureCropFractions(0.10, 0.20, 0.30, 0.40).Apply(Frame);

        placed.Width.Points.ShouldBe(480, 0.01);
        placed.Height.Points.ShouldBe(540, 0.01);
        placed.Left.Points.ShouldBe(72 - 48, 0.01);
        placed.Top.Points.ShouldBe(72 - 108, 0.01);
    }

    /// <summary>
    /// A crop that keeps nothing is a file's error, and the rectangle it was given is a better
    /// answer than a hole: <see cref="PictureCrop.Uncropped"/> returns null there and this is
    /// where that null is turned back into a picture.
    /// </summary>
    [Fact]
    public void ACropThatKeepsNothingFallsBackToTheFrame()
        => new PictureCropFractions(0.6, 0, 0.6, 0).Apply(Frame).ShouldBe(Frame);

    /// <summary>
    /// A negative fraction shrinks rather than grows, and is legal: it is how a file states that
    /// the picture is smaller than the frame it sits in.
    /// </summary>
    [Fact]
    public void ANegativeFractionShrinksTheDestination()
    {
        DocRect placed = new PictureCropFractions(-0.25, 0, 0, 0).Apply(Frame);

        placed.Width.ShouldBeLessThan(Frame.Width);
        placed.Left.ShouldBeGreaterThan(Frame.Left);
    }

    /// <summary>
    /// A crop on one axis leaves the other alone. Drift here would be the sign of an offset
    /// applied before the size rather than after it.
    /// </summary>
    [Fact]
    public void CroppingOneAxisLeavesTheOther()
    {
        DocRect placed = new PictureCropFractions(0, 0.5, 0, 0).Apply(Frame);

        placed.Left.ShouldBe(Frame.Left);
        placed.Width.ShouldBe(Frame.Width);
        placed.Height.Points.ShouldBe(Frame.Height.Points * 2, 0.01);
    }
}

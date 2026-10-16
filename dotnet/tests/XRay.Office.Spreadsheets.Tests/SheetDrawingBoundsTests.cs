using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A drawing's bounding rectangle against the rectangle its anchor states.
/// </summary>
/// <remarks>
/// <para>
/// The two are the same thing for an ordinary picture and are not for a group holding a turned
/// shape, which is what <c>ScDrawLayer::GetPrintArea</c> and <c>ScDocument::HasAnyDraw</c> both
/// ask for (<c>GetCurrentBoundRect</c>). Worth its own tests because the difference is worth four
/// pages on <c>SIL_TDB648.xlsx</c> and neither the anchor nor the frame shows it.
/// </para>
/// <para>
/// The numbers below are arithmetic rather than measured: a rectangle <em>w</em> by <em>h</em>
/// turned by <em>a</em> has a bounding box of <c>w·|cos a| + h·|sin a|</c> by
/// <c>w·|sin a| + h·|cos a|</c>, and every assertion here is that formula applied to the part's
/// share of the frame.
/// </para>
/// </remarks>
public sealed class SheetDrawingBoundsTests
{
    private static SheetGrid Grid => SheetGrid.Standard;

    /// <summary>A drawing spanning columns 0-3 and rows 0-9 of a standard grid.</summary>
    private static SheetDrawing Framed(params SheetDrawingPart[] parts)
        => new()
        {
            Anchor = SheetAnchorKind.TwoCell,
            From = new SheetCellPoint(0, Length.Zero, 0, Length.Zero),
            To = new SheetCellPoint(4, Length.Zero, 10, Length.Zero),
            Parts = parts,
        };

    private static long FrameRight => SheetGrid.StandardColumnWidth.Twips * 4;

    private static long FrameBottom => SheetGrid.StandardRowHeight.Twips * 10;

    [Fact]
    public void ADrawingWithNoPartsIsItsOwnAnchor()
    {
        (long left, long top, long right, long bottom) =
            SheetDrawingBounds.Of(Framed(), Grid);

        left.ShouldBe(0);
        top.ShouldBe(0);
        right.ShouldBe(FrameRight);
        bottom.ShouldBe(FrameBottom);
    }

    [Fact]
    public void AnUnturnedPartFillingTheFrameChangesNothing()
    {
        (long _, long _, long right, long bottom) = SheetDrawingBounds.Of(
            Framed(new SheetDrawingPart(0, 0, 1, 1, 0)), Grid);

        right.ShouldBe(FrameRight);
        bottom.ShouldBe(FrameBottom);
    }

    /// <summary>
    /// A quarter turn swaps a part's extents, so a wide part reaches below a frame it fitted.
    /// </summary>
    /// <remarks>
    /// The cleanest case there is: at 90° the bounding box is the part's height by its width, and
    /// no trigonometry has to be believed to check it.
    /// </remarks>
    [Fact]
    public void AQuarterTurnedPartReachesPastTheFrameItFits()
    {
        // The full width of the frame and a tenth of its height, turned upright about its centre.
        (long left, long top, long right, long bottom) = SheetDrawingBounds.Of(
            Framed(new SheetDrawingPart(0, 0, 1, 0.1, 90)), Grid);

        long width = FrameRight;
        long height = (long)(FrameBottom * 0.1);
        long centreX = width / 2;
        long centreY = height / 2;

        Math.Abs(left - (centreX - (height / 2))).ShouldBeLessThanOrEqualTo(1);
        Math.Abs(right - (centreX + (height / 2))).ShouldBeLessThanOrEqualTo(1);
        Math.Abs(top - (centreY - (width / 2))).ShouldBeLessThanOrEqualTo(1);
        Math.Abs(bottom - (centreY + (width / 2))).ShouldBeLessThanOrEqualTo(1);

        // Which is the point: the part sat inside the frame and its bound rect does not.
        bottom.ShouldBeGreaterThan(height);
    }

    /// <summary>
    /// The bounds are the union of every part, not the last one.
    /// </summary>
    [Fact]
    public void TheBoundsUnionEveryPart()
    {
        (long _, long _, long right, long bottom) = SheetDrawingBounds.Of(
            Framed(
                new SheetDrawingPart(0, 0, 0.2, 0.2, 0),
                new SheetDrawingPart(0.8, 0.8, 0.2, 0.2, 0)),
            Grid);

        Math.Abs(right - (FrameRight)).ShouldBeLessThanOrEqualTo(1);
        Math.Abs(bottom - (FrameBottom)).ShouldBeLessThanOrEqualTo(1);
    }

    /// <summary>
    /// A shallow turn widens a part downwards and narrows nothing, which is the watermark case.
    /// </summary>
    /// <remarks>
    /// <c>SIL_TDB648.xlsx</c> in miniature: a strip 96% of the frame wide and 4% of it tall, turned
    /// 27°, whose bound rect reaches well below the strip's own box. On the real workbook the same
    /// shape carries the group's bottom 4.2% below the frame, which is a band of pages.
    /// </remarks>
    [Fact]
    public void AShallowTurnCarriesAPartBelowItsOwnBox()
    {
        (long _, long _, long _, long bottom) = SheetDrawingBounds.Of(
            Framed(new SheetDrawingPart(0.02, 0.90, 0.96, 0.04, 27)), Grid);

        double partWidth = FrameRight * 0.96;
        double partHeight = FrameBottom * 0.04;
        double radians = 27 * Math.PI / 180.0;
        double boxHeight = (partWidth * Math.Sin(radians)) + (partHeight * Math.Cos(radians));
        double centreY = FrameBottom * (0.90 + (0.04 / 2));

        Math.Abs(bottom - ((long)Math.Round(centreY + (boxHeight / 2)))).ShouldBeLessThanOrEqualTo(1);
        bottom.ShouldBeGreaterThan(FrameBottom);
    }

    /// <summary>
    /// The turn is applied to the part's <em>scaled</em> box, not to a box scaled afterwards.
    /// </summary>
    /// <remarks>
    /// The order matters because a frame stretches its parts by different factors across and down,
    /// and the bounding box of a turned rectangle is not linear in those factors. Two frames of the
    /// same area and different shape, holding the same part, must give different bounds; folding
    /// the turn into a fixed inset at read time would give the same one twice — the mistake that
    /// made each watermark on <c>SIL_TDB648.xlsx</c> 197 pt tall where the reference draws it 255.
    /// </remarks>
    [Fact]
    public void TheTurnIsAppliedAfterTheFrameHasScaledThePart()
    {
        SheetDrawingPart part = new(0.25, 0.25, 0.5, 0.5, 45);

        SheetDrawing wide = new()
        {
            Anchor = SheetAnchorKind.Absolute,
            Position = new DocPoint(Length.Zero, Length.Zero),
            Extent = new DocSize(Length.FromTwips(4000), Length.FromTwips(1000)),
            Parts = [part],
        };

        SheetDrawing tall = wide with
        {
            Extent = new DocSize(Length.FromTwips(1000), Length.FromTwips(4000)),
        };

        (long _, long _, long wideRight, long wideBottom) = SheetDrawingBounds.Of(wide, Grid);
        (long _, long _, long tallRight, long tallBottom) = SheetDrawingBounds.Of(tall, Grid);

        // Turned 45°, a 2000x500 part has a bound box of 1767 square; a 500x2000 one has the same.
        // The frames differ, so where that box lands differs — and it overflows the short edge of
        // each. Scaling a bound box computed before the turn would instead give 2500x2500 scaled
        // per axis, which overflows neither.
        wideBottom.ShouldBeGreaterThan(1000);
        tallRight.ShouldBeGreaterThan(1000);
        wideRight.ShouldBeLessThan(4000);
        tallBottom.ShouldBeLessThan(4000);
    }
}

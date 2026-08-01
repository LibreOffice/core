using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector.Metafiles;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The rectangle-set boolean under every clip combine mode.
/// </summary>
/// <remarks>
/// <para>
/// <b>These are the tests that decide whether a general polygon boolean is needed, so they are
/// worth reading before adding one.</b> A GDI region is a scan list of rectangles by definition,
/// and rectangle sets are closed under union, intersection, difference and symmetric difference
/// — so <c>RGN_OR</c>, <c>RGN_XOR</c> and GDI+'s <c>CombineModeComplement</c>, which were
/// reported as unexpressible, are exact arithmetic over integers. What is not covered here,
/// deliberately, is an arbitrary path as an operand; <c>src/Paperless.Vector/TODO.md</c> records
/// how often that turns up in real files.
/// </para>
/// <para>
/// Each assertion names the <em>area</em> as well as the rectangle count, because the failure
/// mode a band sweep has is not a wrong shape but a right shape decomposed differently — and a
/// count alone would pass a result whose bands were coalesced one row too far.
/// </para>
/// </remarks>
public class RegionAlgebraTests
{
    private static DocRect Mm(double x, double y, double w, double h) => new(
        Length.FromMillimetres(x),
        Length.FromMillimetres(y),
        Length.FromMillimetres(w),
        Length.FromMillimetres(h));

    private static double AreaMm(IEnumerable<DocRect> rectangles)
        => rectangles.Sum(r => r.Width.Millimetres * r.Height.Millimetres);

    [Fact]
    public void TwoOverlappingRectanglesUnionToTheirCombinedAreaAndNotToTheirSum()
    {
        DocRect[] result = RectangleRegion.Combine([Mm(0, 0, 10, 10)], [Mm(5, 5, 10, 10)], RegionOp.Union);

        // 100 + 100 − 25 overlap. Concatenating the two rectangles would answer 200 under any
        // measure that adds them up, which is exactly what a display list does when a union is
        // faked by emitting both subpaths under the non-zero rule and then reading the area back.
        AreaMm(result).ShouldBe(175.0, 0.0001);
        Disjoint(result);
    }

    [Fact]
    public void ASymmetricDifferenceKeepsWhatIsInExactlyOneOperand()
    {
        DocRect[] result = RectangleRegion.Combine(
            [Mm(0, 0, 10, 10)], [Mm(5, 5, 10, 10)], RegionOp.SymmetricDifference);

        AreaMm(result).ShouldBe(150.0, 0.0001);
        Disjoint(result);

        // The overlap is the one place nothing may be drawn.
        Covers(result, Mm(6, 6, 1, 1)).ShouldBeFalse();
        Covers(result, Mm(1, 1, 1, 1)).ShouldBeTrue();
        Covers(result, Mm(13, 13, 1, 1)).ShouldBeTrue();
    }

    [Fact]
    public void ADifferenceOfARectangleFromTheMiddleOfAnotherIsFourPiecesAndNotFive()
    {
        DocRect[] result = RectangleRegion.Combine([Mm(0, 0, 30, 30)], [Mm(10, 10, 10, 10)], RegionOp.Difference);

        AreaMm(result).ShouldBe(800.0, 0.0001);
        Disjoint(result);

        // Three bands: a full-width one above, two columns beside, a full-width one below. The
        // band sweep coalesces the columns' band rather than cutting each column separately,
        // which is what keeps the count from growing with every exclusion.
        result.Length.ShouldBe(4);
    }

    [Fact]
    public void APlainRectangleSurvivesEveryEdgeTheOtherOperandCutsThroughIt()
    {
        // The other operand's four horizontal edges cut the left one into bands; the bands agree,
        // so they must coalesce back into a single rectangle. Without the vertical coalescing
        // step this answers five rectangles that draw the same picture, and the next combine
        // then works from five, and the one after that from twenty-five.
        DocRect[] result = RectangleRegion.Combine(
            [Mm(0, 0, 10, 40)],
            [Mm(20, 5, 1, 1), Mm(20, 15, 1, 1), Mm(20, 25, 1, 1)],
            RegionOp.Intersect);

        result.ShouldBeEmpty();

        DocRect[] union = RectangleRegion.Normalise([Mm(0, 0, 10, 20), Mm(0, 20, 10, 20)]);
        union.ShouldHaveSingleItem();
        union[0].Height.Millimetres.ShouldBe(40.0, 0.0001);
    }

    [Fact]
    public void AGridOfBarsIsTheCaseThatGrowsAndItGrowsToNSquaredAndNoFurther()
    {
        // Four horizontal bars against four vertical ones. This is the shape the rectangle cap
        // exists for, and knowing its exact growth is what makes the cap a number rather than a
        // guess: n bars each way intersect in n² pieces, never more.
        List<DocRect> bars = [];
        List<DocRect> posts = [];
        for (int i = 0; i < 4; i++)
        {
            bars.Add(Mm(0, i * 10, 100, 5));
            posts.Add(Mm(i * 10, 0, 5, 100));
        }

        DocRect[] result = RectangleRegion.Combine(bars, posts, RegionOp.Intersect);

        result.Length.ShouldBe(16);
        AreaMm(result).ShouldBe(16 * 25.0, 0.0001);
        Disjoint(result);
    }

    [Fact]
    public void TheResultDoesNotDependOnTheOrderTheOperandsWereWrittenIn()
    {
        List<DocRect> scattered = [Mm(5, 5, 10, 10), Mm(0, 0, 10, 10), Mm(12, 0, 4, 20)];

        DocRect[] one = RectangleRegion.Normalise(scattered);
        DocRect[] other = RectangleRegion.Normalise([.. scattered.AsEnumerable().Reverse()]);

        one.ShouldBe(other);
    }

    [Fact]
    public void AClipUnionsAndXorsExactlyWhileItIsRectangularAndReportsWhenItIsNot()
    {
        MetafileClip clip = new();
        clip.Intersect(Mm(0, 0, 10, 10));
        clip.IsRectangular.ShouldBeTrue();

        clip.Union([Mm(20, 0, 10, 10)]);
        clip.HasUnsupportedOperation.ShouldBeFalse();

        Recorder recorder = new();
        recorder.Save();
        clip.Apply(recorder);

        // One ClipPath call carrying two disjoint subpaths, which is their union under either
        // fill rule — the same fact that lets a region be a clip at all.
        recorder.Clips.ShouldHaveSingleItem();
        recorder.Clips[0].Width.Millimetres.ShouldBe(30.0, 0.001);

        // Once an arbitrary path is in the clip, its area is no longer a rectangle set and a
        // union cannot be computed. That is reported rather than approximated.
        MetafileClip shaped = new();
        shaped.Intersect(GraphicsPath.Rectangle(Mm(0, 0, 10, 10)));
        shaped.IsRectangular.ShouldBeFalse();
        shaped.Union([Mm(20, 0, 10, 10)]);
        shaped.HasUnsupportedOperation.ShouldBeTrue();
    }

    [Fact]
    public void ComplementKeepsTheNewRegionOutsideTheOldClipAndNotTheOtherWayRound()
    {
        // The mode most easily implemented backwards, because it is the only one whose operands
        // are reversed: emfphelperdata.cxx:1553-1558 spells it diff(right, left).
        MetafileClip clip = new();
        clip.Intersect(Mm(0, 0, 10, 10));
        clip.Complement([Mm(5, 0, 10, 10)]);

        Recorder recorder = new();
        recorder.Save();
        clip.Apply(recorder);

        recorder.Clips.ShouldHaveSingleItem();
        recorder.Clips[0].X.Millimetres.ShouldBe(10.0, 0.001);
        recorder.Clips[0].Width.Millimetres.ShouldBe(5.0, 0.001);
    }

    [Fact]
    public void AUnionWithAnUnclippedStateIsTheIdentityRatherThanTheOperand()
    {
        // An unbounded clip already admits everything, so widening it to include a rectangle must
        // leave it unbounded. Treating "no rectangles yet" as the empty set instead — the obvious
        // reading of a null field — would narrow the picture to that one rectangle.
        MetafileClip clip = new();
        clip.Union([Mm(0, 0, 10, 10)]);

        clip.IsEmpty.ShouldBeTrue();
        clip.HasUnsupportedOperation.ShouldBeFalse();
    }

    private static void Disjoint(DocRect[] rectangles)
    {
        for (int i = 0; i < rectangles.Length; i++)
        {
            for (int j = i + 1; j < rectangles.Length; j++)
            {
                rectangles[i].IntersectsWith(rectangles[j])
                    .ShouldBeFalse($"{rectangles[i]} overlaps {rectangles[j]}");
            }
        }
    }

    private static bool Covers(DocRect[] rectangles, DocRect probe)
        => rectangles.Any(r => r.Left <= probe.Left && r.Top <= probe.Top
            && r.Right >= probe.Right && r.Bottom >= probe.Bottom);
}

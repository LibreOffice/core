using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// The geometry a <see cref="HatchPaint"/> expands into.
/// </summary>
/// <remarks>
/// <para>
/// The spacing rule here is read off LibreOffice's own PDF rather than derived. Page 24 of
/// <c>slides/batch-011/pptx/171128IPAP.pptx</c> — a <c>pct25</c> cell, so a two-way hatch two
/// millimetres apart at 45° — is drawn as a run of hairline segments whose first two are
/// <c>91.644 313.342 m 99.468 321.165 l S</c> and <c>91.644 305.291 m 107.518 321.165 l S</c>.
/// Consecutive lines are 8.051 pt apart along the cell's edge, and <c>2 mm × √2 = 8.05 pt</c>:
/// the stated distance is <b>perpendicular to the line</b>, not along the axis it crosses. GDI
/// states the other one, which is why <c>MetafilePainter</c> multiplies by √2 where this does
/// not — and why the two expansions have to be kept apart rather than merged.
/// </para>
/// <para>
/// The direction is the other half. Both of those segments run up and to the right in PDF's
/// y-up space, so they rise to the right on the page — which is what an angle stated
/// anticlockwise means once the document's y points down.
/// </para>
/// </remarks>
public class HatchingTests
{
    /// <summary>A ten-millimetre square, in the unit LibreOffice states a hatch's distance in.</summary>
    private static readonly DocRect Box = new(
        Length.Zero, Length.Zero, Length.FromMm100(1000), Length.FromMm100(1000));

    /// <summary>Where the horizontal family's five lines sit, in 1/100 mm.</summary>
    private static readonly long[] Expected = [200, 400, 600, 800, 1000];

    [Fact]
    public void AOneWayHatchIsSpacedByTheStatedDistanceFromTheLeadingEdge()
    {
        // Ten millimetres hatched every two: five lines, the first one whole distance in from
        // the top edge and the last on the bottom edge. Not centred — GeoTexSvxHatch emits
        // `a x distance` for a from 1 to fround(H/distance + 0.5) - 1, measured from the edge,
        // and the difference from a centred family is up to half a step on every line.
        (DocPoint From, DocPoint To)[] lines =
            [.. Hatching.Lines(Box, Hatch(HatchKind.OneWay, 200, 0))];

        lines.Length.ShouldBe(5);
        lines.Select(line => line.From.Y.Emu / Length.FromMm100(1).Emu)
            .ShouldBe(Expected);

        // Horizontal, so both ends sit on the same row.
        lines[0].From.Y.ShouldBe(lines[0].To.Y);
    }

    [Fact]
    public void TheExtraFamiliesAreAQuarterAndAnEighthTurnFromTheStatedAngle()
    {
        (DocPoint From, DocPoint To)[] one =
            [.. Hatching.Lines(Box, Hatch(HatchKind.OneWay, 200, 45))];
        (DocPoint From, DocPoint To)[] two =
            [.. Hatching.Lines(Box, Hatch(HatchKind.TwoWay, 200, 45))];
        (DocPoint From, DocPoint To)[] three =
            [.. Hatching.Lines(Box, Hatch(HatchKind.ThreeWay, 200, 45))];

        // Each kind includes the ones below it — the decomposition falls through — and over a
        // square box the second family is the first reflected, so it holds the same count. The
        // third is at 0°, which crosses a square in fewer lines than a diagonal does.
        two.Length.ShouldBe(one.Length * 2);
        three.Length.ShouldBe(
            two.Length + Hatching.Lines(Box, Hatch(HatchKind.OneWay, 200, 0)).Count());

        // The stated angle is drawn last, so a caller laying them down in order finishes with it,
        // as fillhatchprimitive2d.cxx's fall-through does.
        two[^1].ShouldBe(one[^1]);
        three[^1].ShouldBe(one[^1]);

        // A quarter turn either way is the same family of lines, so only the third distinguishes
        // the sign — and it is minus an eighth. Stated at 0° the three run at 135°, 90° and 0°
        // measured anticlockwise, never at 45°.
        Directions(Hatching.Lines(Box, Hatch(HatchKind.ThreeWay, 200, 0)))
            .ShouldBe([135, 90, 0]);
    }

    /// <summary>
    /// The distinct directions a hatch's segments run at, in degrees anticlockwise modulo a half
    /// turn, in the order they are first drawn.
    /// </summary>
    private static List<int> Directions(IEnumerable<(DocPoint From, DocPoint To)> lines)
    {
        List<int> seen = [];

        foreach ((DocPoint from, DocPoint to) in lines)
        {
            double radians = Math.Atan2(-(to.Y.Emu - from.Y.Emu), to.X.Emu - from.X.Emu);
            int degrees = (int)Math.Round((radians * 180.0 / Math.PI) + 180.0) % 180;
            if (!seen.Contains(degrees)) seen.Add(degrees);
        }

        return seen;
    }

    [Fact]
    public void ADiagonalIsSpacedPerpendicularlyAndRisesToTheRight()
    {
        (DocPoint From, DocPoint To)[] lines =
            [.. Hatching.Lines(Box, Hatch(HatchKind.OneWay, 200, 45))];

        // Two millimetres perpendicular at 45° is 2√2 mm measured along either axis — 283 in
        // 1/100 mm — which is the eight-point step in the reference PDF quoted above.
        double first = lines[0].From.X.Emu + lines[0].From.Y.Emu;
        double second = lines[1].From.X.Emu + lines[1].From.Y.Emu;
        ((second - first) / Length.FromMm100(1).Emu).ShouldBe(283, 1.0);

        // Rising to the right: x grows as y falls.
        (lines[0].To.X.Emu - lines[0].From.X.Emu).ShouldBeGreaterThan(0);
        (lines[0].To.Y.Emu - lines[0].From.Y.Emu).ShouldBeLessThan(0);
    }

    [Fact]
    public void AHatchWithNoDistanceDrawsNothingRatherThanLoopingForEver()
    {
        // An a:pattFill naming a preset createHatch does not know is exactly this: the distance
        // is left at its default of nought and the fill reaches the page as its background.
        Hatching.Lines(Box, Hatch(HatchKind.TwoWay, 0, 45)).ShouldBeEmpty();
        Hatching.Lines(DocRect.Empty, Hatch(HatchKind.OneWay, 200, 45)).ShouldBeEmpty();
    }

    [Fact]
    public void AHatchFinerThanTheBoxCanShowIsWidenedRatherThanTruncated()
    {
        // One EMU apart over a ten-millimetre box is 360000 lines, which is a solid fill drawn
        // the expensive way. The count is bounded and the family still spans the whole box —
        // truncating it instead would leave a stripe through the middle.
        (DocPoint From, DocPoint To)[] lines =
            [.. Hatching.Lines(
                Box, Hatch(HatchKind.OneWay, 200, 0) with { Distance = Length.FromEmu(1) })];

        lines.Length.ShouldBe(2000);
        lines[0].From.Y.ShouldBe(Length.FromEmu(180));
        lines[^1].From.Y.ShouldBe(Length.FromMm100(1000));
    }

    private static HatchPaint Hatch(HatchKind kind, int distanceMm100, double degrees)
        => new(
            Colour.FromRgb(0xFF0000),
            kind,
            Length.FromMm100(distanceMm100),
            degrees * Math.PI / 180.0,
            Background: null);
}

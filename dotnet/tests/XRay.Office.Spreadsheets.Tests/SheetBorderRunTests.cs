using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A grid line arrives as one stroke per maximal run of identically styled cell edges.
/// </summary>
/// <remarks>
/// <para>
/// Calc's border array emits one entry per cell edge
/// (<c>svx::frame::Array::CreateB2DPrimitiveRange</c>,
/// <c>svx/source/dialog/framelinkarray.cxx:1490-1537</c>), and
/// <c>SdrFrameBorderPrimitive2D::create2DDecomposition</c> then folds each new segment into any
/// already-emitted one it can
/// (<c>svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx:782-841</c>) through
/// <c>tryMergeBorderLinePrimitive2D</c>, which requires a shared endpoint, a zero cross product,
/// an equal <c>StrokeAttribute</c> and an equal <c>LineAttribute</c> — width, colour, join and
/// cap — on every sub-line
/// (<c>drawinglayer/source/primitive2d/borderlineprimitive2d.cxx:300-417</c>).
/// </para>
/// <para>
/// **Every expectation below is read out of LibreOffice 26.2.4.2's own PDF of the fixture**, not
/// out of the C++, which is a 27.2 tree here and not the reference binary. The census is in
/// <c>dotnet/probes/sheets-d-01/results.md</c>; the numbers are quoted per case.
/// </para>
/// <para>
/// The reason this matters is ink rather than tidiness. A cell edge is extended at each end by
/// half the width of the perpendicular border crossing there, so two abutting segments of one run
/// **overlap** by that crossing's full width — 0.75 pt on a thin border, doubling the ink at every
/// joint. Merging the run discards the interior extends and keeps only the outer two, which is
/// what the reference draws.
/// </para>
/// </remarks>
public sealed class SheetBorderRunTests
{
    private static IReadOnlyList<DrawnPage> Draw()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-border-runs.fods"));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return sink.Pages;
    }

    /// <summary>
    /// The page's horizontal strokes, grouped by the grid line they sit on, top to bottom.
    /// </summary>
    /// <remarks>
    /// Grouped and indexed by ordinal rather than keyed on a y coordinate so that the cases below
    /// state <em>how many strokes are on this line</em>, which is the whole question, without also
    /// asserting the row heights — a second thing that could move and would make every case here
    /// fail for an unrelated reason.
    /// </remarks>
    private static IReadOnlyList<IReadOnlyList<DrawnStroke>> Lines(DrawnPage page)
        => [.. page.StrokedPaths
            .Where(stroke => stroke.Bounds.Width > stroke.Bounds.Height)
            .GroupBy(stroke => Math.Round(stroke.Bounds.Y.Points, 1))
            .OrderBy(group => group.Key)
            .Select(group => (IReadOnlyList<DrawnStroke>)
                [.. group.OrderBy(stroke => stroke.Bounds.X.Points)])];

    /// <summary>The page's vertical strokes, left to right and then top to bottom.</summary>
    private static IReadOnlyList<DrawnStroke> Verticals(DrawnPage page)
        => [.. page.StrokedPaths
            .Where(stroke => stroke.Bounds.Height > stroke.Bounds.Width)
            .OrderBy(stroke => stroke.Bounds.X.Points)
            .ThenBy(stroke => stroke.Bounds.Y.Points)];

    private static Colour ColourOf(DrawnStroke stroke)
        => ((SolidPaint)stroke.Stroke.Paint).Colour;

    private static readonly Colour Red = Colour.FromRgb(0xFF0000);
    private static readonly Colour Blue = Colour.FromRgb(0x0000FF);
    private static readonly Colour Green = Colour.FromRgb(0x008000);

    /// <summary>Four cells wide, at 2 cm each, in points.</summary>
    private const double FourColumns = 226.772;

    [Fact]
    public void FourIdenticallyStyledEdgesAreOneStroke()
    {
        // Row 2. LibreOffice draws one stroke, 56.665 -> 283.436: the whole four-cell run, and no
        // overshoot at either end because nothing crosses there.
        IReadOnlyList<DrawnStroke> line = Lines(Draw()[0])[0];

        line.Count.ShouldBe(1);
        line[0].Bounds.Width.Points.ShouldBe(FourColumns, 0.1);
        ColourOf(line[0]).ShouldBe(Red);
    }

    [Fact]
    public void AColourChangeSplitsTheRunInTwo()
    {
        // Row 4. LibreOffice: red 56.665 -> 170.05, blue 170.05 -> 283.436. They abut exactly —
        // no crossing border at the joint, so neither end is extended.
        IReadOnlyList<DrawnStroke> line = Lines(Draw()[0])[1];

        line.Count.ShouldBe(2);
        ColourOf(line[0]).ShouldBe(Red);
        ColourOf(line[1]).ShouldBe(Blue);
        line[0].Bounds.Right.Points.ShouldBe(line[1].Bounds.X.Points, 0.05);
    }

    [Fact]
    public void AWidthChangeSplitsTheRunInTwo()
    {
        // Row 6. LibreOffice: 1.4 pt then 2.85 pt, same colour, abutting.
        IReadOnlyList<DrawnStroke> line = Lines(Draw()[0])[2];

        line.Count.ShouldBe(2);
        line[0].Stroke.Width.Points.ShouldBeLessThan(line[1].Stroke.Width.Points);
        ColourOf(line[0]).ShouldBe(Red);
        ColourOf(line[1]).ShouldBe(Red);
    }

    [Fact]
    public void APatternChangeSplitsTheRunInTwo()
    {
        // Row 8. Same colour and the same nominal width; only the dash array differs. This is the
        // case a colour-and-width comparison would merge and LibreOffice does not — its
        // StrokeAttribute test comes before the per-line one.
        IReadOnlyList<DrawnStroke> line = Lines(Draw()[0])[3];

        line.Count.ShouldBe(2);
        line[0].Stroke.DashPattern.ShouldBeNull();
        line[1].Stroke.DashPattern.ShouldNotBeNull();
    }

    [Fact]
    public void AHoleInTheMiddleOfARunIsNotBridged()
    {
        // Row 10: A and B state the border, C states nothing, D states it again. LibreOffice draws
        // 56.665 -> 170.05 and 226.743 -> 283.436, and nothing across C.
        IReadOnlyList<DrawnStroke> line = Lines(Draw()[0])[4];

        line.Count.ShouldBe(2);
        line[1].Bounds.X.Points.ShouldBeGreaterThan(line[0].Bounds.Right.Points + 50);
    }

    [Fact]
    public void ASingleRuleDoesNotMergeIntoADouble()
    {
        // Row 12 puts three strokes on three y values a fraction of a point apart: the double's
        // upper line, the single, and the double's lower line. The single spans A..B only.
        IReadOnlyList<IReadOnlyList<DrawnStroke>> lines = Lines(Draw()[0]);

        lines[5].Count.ShouldBe(1);
        lines[6].Count.ShouldBe(1);
        lines[7].Count.ShouldBe(1);
        lines[6][0].Bounds.Width.Points.ShouldBe(FourColumns / 2, 0.1);
    }

    [Fact]
    public void APerpendicularBorderCrossingAJointDoesNotBreakTheRun()
    {
        // Row 14: four identical top borders with a 2.85 pt green vertical crossing at the B|C
        // joint. LibreOffice still draws one stroke, 56.665 -> 283.436 — and it is *not* extended
        // at the crossing, because merging keeps only the run's outer extends.
        IReadOnlyList<DrawnStroke> line = Lines(Draw()[0])[8];

        line.Count.ShouldBe(1);
        line[0].Bounds.Width.Points.ShouldBe(FourColumns, 0.1);
    }

    [Fact]
    public void AJointThatIsBrokenStillOvershootsItsCrossing()
    {
        // Row 16: the same crossing vertical, but the run breaks on colour. LibreOffice draws
        // 56.665 -> 171.468 and 168.633 -> 283.436 — the two overlap by 2.835 pt, the green
        // vertical's full width. This is the control that stops the fix being read as "never
        // extend at a joint": the overshoot is real, it is the *merge* that removes it.
        IReadOnlyList<DrawnStroke> line = Lines(Draw()[0])[9];

        line.Count.ShouldBe(2);
        (line[0].Bounds.Right.Points - line[1].Bounds.X.Points).ShouldBe(2.835, 0.05);
    }

    [Fact]
    public void OneGridLineStatedByTwoDifferentCellAttributesIsOneStroke()
    {
        // Rows 17 and 18: C17 and D17 state a *bottom* border, A18 and B18 state the same *top*
        // border. One grid line, two attributes, four cells; LibreOffice draws one stroke,
        // 56.665 -> 283.436.
        IReadOnlyList<DrawnStroke> line = Lines(Draw()[0])[10];

        line.Count.ShouldBe(1);
        line[0].Bounds.Width.Points.ShouldBe(FourColumns, 0.1);
    }

    [Fact]
    public void TheOuterEndsOfAMergedRunKeepTheirOvershoot()
    {
        // Row 28: four identical top borders, with a green vertical at the run's left end and
        // another at its right. LibreOffice draws 55.247 -> 284.854, which is A's left edge and
        // D's right edge each pushed out by half the vertical's 2.835 pt.
        IReadOnlyList<DrawnStroke> line = Lines(Draw()[0])[11];

        line.Count.ShouldBe(1);
        line[0].Bounds.Width.Points.ShouldBe(FourColumns + 2.835, 0.1);
    }

    [Fact]
    public void AVerticalRunMergesDownAColumn()
    {
        // Column A, rows 20-23. LibreOffice draws one stroke 394.695 -> 462.613, four rows tall,
        // where a per-cell renderer draws four each overlapping the next by nothing at all (no
        // crossing horizontals) — so this case proves the *count*, not the ink.
        IReadOnlyList<DrawnStroke> verticals = Verticals(Draw()[0]);
        List<DrawnStroke> columnA =
            [.. verticals.Where(stroke => stroke.Bounds.Height.Points > 60)];

        columnA.Count.ShouldBe(1);
        columnA[0].Bounds.Height.Points.ShouldBe(67.92, 0.2);
        ColourOf(columnA[0]).ShouldBe(Green);
    }

    [Fact]
    public void AVerticalRunSplitsOnColourLikeAHorizontalOne()
    {
        // Column C carries five verticals, top to bottom: the row 14 crossing, the row 16
        // crossing, rows 20-21 in green, rows 22-23 in blue, and rows 25-26. LibreOffice draws
        // the third and fourth as two two-row strokes abutting at 428.654 rather than as four
        // one-row ones — merging is not an axis-specific rule, and colour breaks it either way.
        List<DrawnStroke> columnC =
            [.. Verticals(Draw()[0])
                .Where(stroke => Math.Abs(stroke.Bounds.X.Points - 170.079) < 0.5)];

        columnC.Count.ShouldBe(5);
        ColourOf(columnC[2]).ShouldBe(Green);
        ColourOf(columnC[3]).ShouldBe(Blue);
        columnC[2].Bounds.Height.Points.ShouldBe(33.96, 0.2);
        columnC[3].Bounds.Height.Points.ShouldBe(33.96, 0.2);
        columnC[2].Bounds.Bottom.Points.ShouldBe(columnC[3].Bounds.Y.Points, 0.05);
    }

    [Fact]
    public void AHiddenRowOrColumnDoesNotBreakARun()
    {
        // Sheet 2, its own page: a four-cell top border with column B hidden, and a four-row left
        // border with row 3 hidden. LibreOffice draws one stroke each — 56.665 -> 226.743 across
        // three visible columns and 717.306 -> 768.245 down three visible rows. A hidden line is
        // not in Calc's border array at all, which is why coalescing keys on the *placed* index
        // and not on the sheet's own row and column numbers.
        DrawnPage page = Draw()[1];

        Lines(page).Count.ShouldBe(1);
        Lines(page)[0].Count.ShouldBe(1);
        Lines(page)[0][0].Bounds.Width.Points.ShouldBe(FourColumns * 3 / 4, 0.1);

        Verticals(page).Count.ShouldBe(1);
        Verticals(page)[0].Bounds.Height.Points.ShouldBe(50.94, 0.2);
    }
}

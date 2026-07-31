using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks what a slide's table draws, against numbers read out of LibreOffice's own PDF.
/// </summary>
/// <remarks>
/// <para>
/// The corpus deck is <c>slide-table-grid.pptx</c>, hand-written so that every column is 2.5 inch
/// and every stated row 1 inch — a table at 72 pt whose grid lines land on 252, 432 and 612 pt.
/// Its first slide states every one of a cell's four edges and its second exercises the two
/// spans, the three anchors and a row with no stated height at all.
/// </para>
/// <para>
/// The expectations are quoted from the reference PDF in each test and need no LibreOffice to
/// check; <c>Paperless.Fidelity.Tests</c> re-derives them.
/// </para>
/// </remarks>
public class SlideTablePlacementTests
{
    /// <summary>A twentieth of a point, as everywhere else in the placement tests.</summary>
    private const double TolerancePoints = 0.05;

    /// <summary>
    /// A tenth of a point, for anything downstream of a row that grew to fit its text.
    /// </summary>
    /// <remarks>
    /// A grown row is the one measurement here that depends on a <em>line height</em> rather than
    /// on the file's own numbers, and Paperless's is 0.028 pt per line short of LibreOffice's —
    /// the rounding described in <c>SlideTextLayout.Rounded</c>. Two lines of it puts the bottom
    /// of that table 0.04 pt out.
    /// </remarks>
    private const double GrownRowTolerance = 0.1;

    private static SlidePages Layout(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    private static LaidOutSlide Slide(int index) => Layout("slide-table-grid.pptx").Slides[index];

    /// <summary>The shapes a table produced, which are all of the slide's.</summary>
    private static List<PlacedShape> Cells(LaidOutSlide slide)
        => [.. slide.Shapes.Where(shape => shape.Fill is not null || shape.Text is not null)];

    private static List<PlacedShape> Strokes(LaidOutSlide slide)
        => [.. slide.Shapes.Where(shape => shape.Line is not null)];

    [Fact]
    public void ACellIsFilledOverItsWholeRectangle()
    {
        // The header row's three cells, whose fill LibreOffice's PDF paints from
        // 72 to 252, 252 to 432 and 432 to 612 pt across, and 72 to 144 pt down.
        List<PlacedShape> cells = Cells(Slide(0));
        cells.Count.ShouldBe(9);

        DocRect first = Bounds(cells[0]);
        first.X.Points.ShouldBe(72, TolerancePoints);
        first.Y.Points.ShouldBe(72, TolerancePoints);
        first.Right.Points.ShouldBe(252, TolerancePoints);
        first.Bottom.Points.ShouldBe(144, TolerancePoints);

        cells[0].Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0xDCE6F1)));

        // And the fourth cell — the second row's first — has no fill at all rather than white,
        // because its a:tcPr states a:noFill.
        cells[3].Fill.ShouldBeNull();
    }

    [Fact]
    public void EveryGridLineIsOneStrokeAcrossTheWholeTable()
    {
        // Twelve strokes for a three by three grid: four horizontals across the full width and,
        // because the middle row's verticals differ in colour from the rows above and below,
        // three broken columns rather than three full-height ones. The reference PDF contains
        // exactly these twelve, in this order.
        List<PlacedShape> strokes = Strokes(Slide(0));
        strokes.Count.ShouldBe(12);

        // The first is the table's top rule: two points of dark blue, running from 71.121 to
        // 612.879 — half of the two-point verticals it meets, at each end.
        Stroke top = strokes[0].Line.ShouldNotBeNull();
        top.Width.Points.ShouldBe(1.75, TolerancePoints);
        top.Paint.ShouldBe(Paint.Solid(Colour.FromRgb(0x1F497D)));

        (DocPoint from, DocPoint to) = Ends(strokes[0]);
        from.X.Points.ShouldBe(71.121, TolerancePoints);
        from.Y.Points.ShouldBe(72, TolerancePoints);
        to.X.Points.ShouldBe(612.879, TolerancePoints);

        // The sixth is a one-point grey horizontal and runs to the same 71.121, not to 71.575:
        // the overshoot is half of what crosses it, not half of its own width.
        (DocPoint greyFrom, DocPoint greyTo) = Ends(strokes[5]);
        strokes[5].Line.ShouldNotBeNull().Width.Points.ShouldBe(0.85, TolerancePoints);
        greyFrom.X.Points.ShouldBe(71.121, TolerancePoints);
        greyFrom.Y.Points.ShouldBe(144, TolerancePoints);
        greyTo.X.Points.ShouldBe(612.879, TolerancePoints);
    }

    [Fact]
    public void TheWiderOfTwoCellsBordersWinsTheGridLineBetweenThem()
    {
        // The middle row's second cell states an orange three-point right edge where its
        // neighbour states a grey one-point left edge, and the reference draws 2.65006 pt of
        // orange at x = 432 between y = 144 and y = 216.
        PlacedShape orange = Strokes(Slide(0))
            .Single(shape => shape.Line!.Paint == Paint.Solid(Colour.FromRgb(0xFF6600)));

        orange.Line.ShouldNotBeNull().Width.Points.ShouldBe(2.65, TolerancePoints);

        (DocPoint from, DocPoint to) = Ends(orange);
        from.X.Points.ShouldBe(432, TolerancePoints);
        from.Y.Points.ShouldBe(143.575, TolerancePoints);
        to.Y.Points.ShouldBe(216.425, TolerancePoints);
    }

    [Fact]
    public void TwoBordersOfEqualWidthAreSettledByTheLaterCell()
    {
        // Two ties, both at the same grid position and both drawn by the reference. Between the
        // first row's second and third cells — a red right edge against a grey left edge, both
        // one point — grey wins, because the third cell is written later. Between the middle
        // row's first and second — grey against green — green wins for the same reason.
        List<PlacedShape> strokes = Strokes(Slide(0));

        strokes.ShouldNotContain(
            shape => shape.Line!.Paint == Paint.Solid(Colour.FromRgb(0xC00000)),
            "the red edge loses to the later cell's grey one");

        PlacedShape green = strokes
            .Single(shape => shape.Line!.Paint == Paint.Solid(Colour.FromRgb(0x008000)));

        (DocPoint from, DocPoint to) = Ends(green);
        from.X.Points.ShouldBe(252, TolerancePoints);
        from.Y.Points.ShouldBe(143.575, TolerancePoints);
        to.Y.Points.ShouldBe(216.425, TolerancePoints);
    }

    [Fact]
    public void ACellsTextStartsAtItsOwnMarginAndNotAtTheBodysInsets()
    {
        // 72 + 7.2 for the default marL of 91440 EMU, and 72 + 3.6 + the face's ascent down —
        // the reference draws the first cell's text at 79.2, 91.928.
        PlacedGlyphRun run = Cells(Slide(0))[0].Text.ShouldNotBeNull().Runs[0];

        run.Run.Origin.X.Points.ShouldBe(79.2, TolerancePoints);
        run.Run.Origin.Y.Points.ShouldBe(91.928, TolerancePoints);
    }

    [Fact]
    public void ACellsLineHeightIsTheFacesAndNotTheEm()
    {
        // The wrapping cell on the second slide, whose two reference baselines are 20.154 pt
        // apart. The em rule a slide *shape* uses would put them 21.6 apart.
        List<PlacedGlyphRun> runs =
            [.. Cells(Slide(1)).Single(shape => shape.Text?.Runs.Count == 2).Text!.Runs];

        double spacing = runs[1].Run.Origin.Y.Points - runs[0].Run.Origin.Y.Points;
        spacing.ShouldBe(20.154, GrownRowTolerance);
    }

    [Fact]
    public void ARowWithNoStatedHeightGrowsToItsText()
    {
        // The second slide's last row states h="0" and holds a cell that wraps onto two lines,
        // so the reference puts the table's bottom rule at 263.537 pt — 216 for the two stated
        // rows plus 3.6 + 2 x 20.154 + 3.6 for the third.
        List<PlacedShape> strokes = Strokes(Slide(1));

        (DocPoint from, _) = Ends(strokes[^1]);
        from.Y.Points.ShouldBe(263.537, GrownRowTolerance);
    }

    [Fact]
    public void AMergedCellCoversTheColumnsItSpans()
    {
        // The second slide's first row is one cell spanning two columns and one spanning none,
        // so its fill runs from 72 to 432 pt rather than to 252.
        DocRect merged = Bounds(Cells(Slide(1))[0]);

        merged.X.Points.ShouldBe(72, TolerancePoints);
        merged.Right.Points.ShouldBe(432, TolerancePoints);
    }

    private static DocRect Bounds(PlacedShape shape)
    {
        Length left = Length.FromEmu(long.MaxValue);
        Length top = Length.FromEmu(long.MaxValue);
        Length right = Length.FromEmu(long.MinValue);
        Length bottom = Length.FromEmu(long.MinValue);

        foreach (PathCommand command in shape.Outline.Commands)
        {
            if (command.Verb is PathVerb.Close) continue;

            left = Length.Min(left, command.Point.X);
            top = Length.Min(top, command.Point.Y);
            right = Length.Max(right, command.Point.X);
            bottom = Length.Max(bottom, command.Point.Y);
        }

        return new DocRect(left, top, right - left, bottom - top);
    }

    private static (DocPoint From, DocPoint To) Ends(PlacedShape shape)
    {
        List<PathCommand> commands = [.. shape.Outline.Commands];
        return (commands[0].Point, commands[^1].Point);
    }
}

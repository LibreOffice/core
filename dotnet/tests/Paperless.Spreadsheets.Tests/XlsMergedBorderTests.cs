using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.MsBinary;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// The repair the BIFF import makes to a merged range before it merges it.
/// </summary>
/// <remarks>
/// An Excel writer states a border on every cell of a merged range, so the origin's own right
/// border is the range's first interior line and its own bottom border is the line under its
/// first row. Since the range is drawn from its origin, those two edges have to be replaced by
/// the ones on the range's far side — <c>XclImpXFRangeBuffer::SetBorderLine</c>,
/// <c>sc/source/filter/excel/xistyle.cxx:1976-1990</c>, called for <c>RIGHT</c> when the range
/// spans columns and for <c>BOTTOM</c> when it spans rows.
/// </remarks>
public sealed class XlsMergedBorderTests
{
    private static readonly SheetBorder Thick =
        SheetBorder.Line(Length.FromTwips(40), Colour.FromRgb(0x0000FF));

    private static readonly SheetBorder Hair =
        SheetBorder.Line(Length.FromTwips(1), Colour.FromRgb(0x003366));

    [Fact]
    public void ARangesFarSideBordersAreMovedOntoItsOrigin()
    {
        SheetFormatting formatting = new();

        // The origin: heavy on the two edges it already sits on, hairline on the two that are
        // interior to the range.
        Set(formatting, 1, 1, new SheetCellBorders(Thick, Hair, Thick, Hair));

        // The far column and the far row, each stating the heavy outer line.
        Set(formatting, 1, 3, new SheetCellBorders(Hair, Thick, Thick, Hair));
        Set(formatting, 4, 1, new SheetCellBorders(Thick, Hair, Hair, Thick));

        XlsMergedBorders.Apply(formatting, [new SheetRange(1, 1, 3, 4)]);

        SheetCellBorders moved = formatting.At(1, 1).Borders;
        moved.Right.ShouldBe(Thick);
        moved.Bottom.ShouldBe(Thick);

        // Left and top need no move: the origin already sits on them.
        moved.Left.ShouldBe(Thick);
        moved.Top.ShouldBe(Thick);
    }

    [Fact]
    public void ARangeOneColumnWideMovesOnlyItsBottom()
    {
        SheetFormatting formatting = new();

        Set(formatting, 1, 1, new SheetCellBorders(Thick, Thick, Thick, Hair));
        Set(formatting, 3, 1, new SheetCellBorders(Hair, Hair, Hair, Thick));

        // bMultiCol is false, so SetBorderLine is never called for RIGHT — the origin keeps its
        // own right border however the range's other cells state theirs.
        XlsMergedBorders.Apply(formatting, [new SheetRange(1, 1, 1, 3)]);

        formatting.At(1, 1).Borders.Bottom.ShouldBe(Thick);
        formatting.At(1, 1).Borders.Right.ShouldBe(Thick);
    }

    [Fact]
    public void ARangeThatIsOneCellIsLeftAlone()
    {
        SheetFormatting formatting = new();
        Set(formatting, 1, 1, new SheetCellBorders(Thick, Hair, Thick, Hair));

        XlsMergedBorders.Apply(formatting, [new SheetRange(1, 1, 1, 1)]);

        formatting.At(1, 1).Borders.ShouldBe(new SheetCellBorders(Thick, Hair, Thick, Hair));
    }

    [Fact]
    public void ACellsFillSurvivesTheMove()
    {
        SheetFormatting formatting = new();
        Colour fill = Colour.FromRgb(0xFF0000);

        formatting.SetCell(1, 1, formatting.Intern(
            new SheetCellDecoration(fill, new SheetCellBorders(Thick, Hair, Thick, Hair))));
        Set(formatting, 1, 2, new SheetCellBorders(Hair, Thick, Thick, Hair));

        XlsMergedBorders.Apply(formatting, [new SheetRange(1, 1, 2, 1)]);

        formatting.At(1, 1).Background.ShouldBe(fill);
        formatting.At(1, 1).Borders.Right.ShouldBe(Thick);
    }

    private static void Set(SheetFormatting formatting, int row, int column, SheetCellBorders borders)
        => formatting.SetCell(row, column, formatting.Intern(new SheetCellDecoration(null, borders)));
}

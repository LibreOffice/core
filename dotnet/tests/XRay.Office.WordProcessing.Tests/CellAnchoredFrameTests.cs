using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A frame anchored inside a table cell is an obstacle for that cell's own text.
/// </summary>
/// <remarks>
/// <para>
/// Writer draws no distinction between a cell and any other text frame: <c>SwTextFly</c> belongs to the
/// frame being formatted, and a fly overlapping it narrows its lines or pushes them below whatever the
/// frame is. This engine reached its obstacles by top-level block index, and a paragraph inside a cell
/// has none — so a frame anchored in a cell was placed, drawn, and an obstacle for nobody, its own
/// anchor paragraph included. The row then ended at its text height with the frame hanging out of it.
/// </para>
/// <para>
/// Measured on <c>absrc-pac-01-info-note-en.doc</c> (words/pagination-001), whose masthead is two logos
/// anchored in the two narrow cells of the banner table's first row. The wider logo is 64.8 pt in a
/// 68.45 pt cell, so neither side of it has room for text at all and the cell's empty anchor paragraph
/// belongs below it. Row one is 36.375 to 105.125 pt with that and 36.375 to 74.025 without; the
/// reference starts its second row at 104.5, and everything in the left column — the organisation's
/// logo, the <c>Distr.</c> block, the title — was 31.24 pt too high without it. With it the block's
/// baselines land within 0.14 pt of the reference's.
/// </para>
/// </remarks>
public sealed class CellAnchoredFrameTests
{
    /// <summary>How far down the cell the frame reaches, from the table's own top.</summary>
    private static readonly Length FrameBottom = Length.FromPoints(40);

    /// <summary>The one cell's width, which the frame covers entirely so that no text fits beside it.</summary>
    private static readonly Length CellWidth = Length.FromPoints(100);

    /// <summary>
    /// A cell whose one paragraph anchors a wrapping frame is as tall as the frame plus its line.
    /// </summary>
    [Fact]
    public void ACellGrowsBelowTheFrameItsOwnParagraphAnchors()
    {
        PageTable table = Table(out PageParagraph anchor);

        Length bare = TableLayouter.LayOut(table, DocPoint.Origin).RowHeights[0];
        Length around = TableLayouter.LayOut(
            table,
            DocPoint.Origin,
            anchored: new AnchoredObstacles(
                paragraph => ReferenceEquals(paragraph, anchor)
                    ? [new WrapObstacle(
                        new DocRect(Length.Zero, Length.Zero, CellWidth, FrameBottom),
                        TextWrap.Both)]
                    : null,
                DocPoint.Origin)).RowHeights[0];

        // The line began at the cell's top, so the whole of the frame is what it descended by — plus
        // the twip `FrameObstacles` inflates a frame's hole by, since Writer's rectangles are inclusive.
        (around - bare).ShouldBe(FrameBottom + Length.FromTwips(1));
    }

    /// <summary>A cell whose paragraphs anchor nothing measures exactly as it did before this existed.</summary>
    [Fact]
    public void ACellAnchoringNothingIsUnaffected()
    {
        PageTable table = Table(out PageParagraph _);

        Length bare = TableLayouter.LayOut(table, DocPoint.Origin).RowHeights[0];
        Length asked = TableLayouter.LayOut(
            table,
            DocPoint.Origin,
            anchored: new AnchoredObstacles(_ => null, DocPoint.Origin)).RowHeights[0];

        asked.ShouldBe(bare);
    }

    private static PageTable Table(out PageParagraph anchor)
    {
        anchor = Paragraph();

        return new PageTable
        {
            ColumnWidths = [CellWidth],
            Rows =
            [
                new PageTableRow
                {
                    Cells =
                    [
                        new PageTableCell
                        {
                            Padding = new CellPadding(
                                Length.Zero, Length.Zero, Length.Zero, Length.Zero),
                            Blocks = [anchor],
                        },
                    ],
                },
            ],
        };
    }

    /// <summary>The cell's one paragraph: empty, as a cell holding only a logo is.</summary>
    private static PageParagraph Paragraph() => new()
    {
        Text = string.Empty,
        Face = Face,
        EmSize = Length.FromPoints(11),
        Format = ParagraphFormat.Default,
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}

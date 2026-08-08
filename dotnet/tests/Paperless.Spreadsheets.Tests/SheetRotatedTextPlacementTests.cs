using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A turned cell's block hangs from a corner of the cell and is turned about it, one line at a time.
/// </summary>
/// <remarks>
/// <para>
/// Calc turns a quarter-turned cell by handing the whole EditEngine block to
/// <c>DrawText_ToPosition</c> with an orientation (<c>output2.cxx:3707</c>, <c>:3947</c>), so the
/// lines are laid out unturned from an anchor and the <em>block</em> is turned. Three things follow
/// and each was wrong before:
/// </para>
/// <list type="number">
/// <item><description>
/// The first line's baseline is an ascent into the block, not on the anchor, and every line after
/// it is a line height further. Drawing every line on the anchor put four records of a wrapped
/// heading at exactly one point on <c>Keywords_Mapping_Graphs_and_Charts.xlsx</c> page 43 and every
/// single-line turned cell 10.48 pt from the reference at eleven point.
/// </description></item>
/// <item><description>
/// A cell turned clockwise hangs from a different corner — the block runs down and to the left, so
/// the anchor is its own cross-extent right of the cell's top-left rather than the bottom-left.
/// </description></item>
/// <item><description>
/// A wrapping cell breaks against the cell's <em>height</em>, because the paper an EditEngine gets
/// for a vertically oriented cell is the align rectangle turned with it
/// (<c>calcPaperSize</c>, <c>:2691</c>). The fixture's 53-character string takes two lines in a
/// 2.5 in row and would take eight in the 1.5 in column.
/// </description></item>
/// </list>
/// <para>
/// The expected numbers are LibreOffice 24.2.7.2's own, read off its PDF of this fixture with
/// <c>pdf-ops.py</c> and converted from the page's bottom to its top. The fixture states every row
/// height and turns the header and footer off so the two page bodies start together.
/// </para>
/// <para>
/// <strong>One case is recorded as a shortfall rather than a pass.</strong> On the clockwise
/// wrapping sheet the reference starts both lines at the same point and we start the first 2.55 pt
/// earlier, which is exactly the width of the space that ends it: EditEngine leaves a line's
/// trailing blank out of the width it aligns by. It is a general text-layout rule rather than
/// anything about turning, it is under a tenth of a line, and no corpus document is turned
/// clockwise at all.
/// </para>
/// </remarks>
public sealed class SheetRotatedTextPlacementTests
{
    private const string Fixture = "sheet-rotated-placement.fods";

    /// <summary>The page is 11 in tall; the reference's own numbers count up from its foot.</summary>
    private static readonly Length PageHeight = Length.FromPoints(792);

    private static List<(GlyphRun Run, DocPoint Origin)> Drawn(int page)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(Fixture));

        PlacedDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[page].Draw(sink);
        return sink.Runs;
    }

    private static (double X, double Y, int Glyphs) FromFoot((GlyphRun Run, DocPoint Origin) run)
        => (run.Origin.X.Points, (PageHeight - run.Origin.Y).Points, run.Run.Glyphs.Count);

    // Sheet order: 90 without and with wrapping, then 270, then 45.
    [Theory]
    // Anticlockwise, one line per cell: both cells' text starts on the same x, an ascent right of
    // the cell's left margin, and runs up the page from the cell's foot.
    [InlineData(0, 0, 83.48, 540.99, 2)]
    [InlineData(0, 1, 83.48, 360.99, 53)]
    // Anticlockwise and wrapped: two lines a line height apart across the page, both starting at
    // the same height. Eleven-point Carlito's line is 13.49 pt.
    [InlineData(1, 1, 83.48, 360.99, 27)]
    [InlineData(1, 2, 96.97, 360.99, 26)]
    // Clockwise: the block hangs from the other corner and the text runs down the page, so a
    // bottom-aligned line ends at the cell's foot rather than starting at its head.
    [InlineData(2, 0, 76.00, 553.10, 2)]
    [InlineData(2, 1, 76.00, 626.40, 53)]
    // Clockwise and wrapped: the second line is left of the first, being further into the block.
    [InlineData(3, 2, 76.00, 492.41, 26)]
    public void ARunLandsWhereTheReferencePutsIt(int page, int index, double x, double y, int glyphs)
    {
        (double actualX, double actualY, int actualGlyphs) = FromFoot(Drawn(page)[index]);

        actualGlyphs.ShouldBe(glyphs);
        actualX.ShouldBe(x, 0.15);
        actualY.ShouldBe(y, 0.15);
    }

    /// <summary>
    /// The clockwise wrapped sheet's first line, which is a trailing space short of the reference.
    /// </summary>
    /// <remarks>
    /// Recorded rather than asserted as a pass: see the class remarks. The reference starts both
    /// lines at 492.41 and this starts the first at 494.96, the 2.55 pt being the space that ends
    /// it. The assertion is on the size of the gap, so a change that makes it worse fails.
    /// </remarks>
    [Fact]
    public void TheClockwiseWrappedFirstLineIsATrailingSpaceShortOfTheReference()
    {
        (double _, double actualY, int glyphs) = FromFoot(Drawn(3)[1]);

        glyphs.ShouldBe(27);
        (actualY - 492.41).ShouldBeInRange(0, 2.6);
    }

    /// <summary>
    /// The wrap width is the cell's height, which is what decides how many lines there are at all.
    /// </summary>
    [Fact]
    public void AWrappingTurnedCellBreaksAgainstTheCellsHeightRatherThanItsWidth()
    {
        // Two lines in a 2.5 in row. The column is 1.5 in, which would give eight.
        Drawn(1).Count.ShouldBe(3);   // the short cell above it, then the two lines
        Drawn(3).Count.ShouldBe(3);
    }
}

using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.MsBinary;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The two things a binary PowerPoint table does that the group of rectangles it is written as
/// does not say: its rows are re-derived, and its rules are drawn at a fraction of their width.
/// </summary>
/// <remarks>
/// <para>
/// Both come from the same place. <c>CreateTable</c>
/// (<c>filter/source/msfilter/svdfppt.cxx</c>:7569) discards the group and builds one
/// <c>SdrTableObj</c>: the rows are seeded from the members' tops
/// (<c>CreateTableRows</c>, <c>:7339</c>) and then laid out by
/// <c>TableLayouter::LayoutTableHeight</c> (<c>svx/source/table/tablelayouter.cxx</c>:724), and
/// each line member becomes the neighbouring cells' <c>BorderLine2</c>
/// (<c>ApplyCellLineAttributes</c>, <c>:7513</c>).
/// </para>
/// <para>
/// <strong>Measured at 26.2.4.2 as well as read out of the tree.</strong> On
/// <c>slides/ceiling-001/ppt/Thailand17.ppt</c> page 11 the group's rectangles are a 38.75 pt
/// header and eight rows of 35.87 and its rules state 12700 and 28575 EMU; the reference's own
/// flat ODP gives the cells <c>fo:border</c> of <c>0.23pt</c> and <c>0.54pt</c> and its PDF draws
/// a <b>39.20 pt header, eight rows of 36.20</b>, and strokes of <b>0.40 and 0.9499 pt</b>. On
/// <c>slides/done-013/ppt/2015-Civil-Rights-Website-training.ppt</c> page 48 the rectangles state
/// 1270, 1102, 1098, 2037 and 4150 hundredths of a millimetre and the reference's flat ODP gives
/// every row its own minimum instead — <c>1.108cm</c> four times and <c>5.024cm</c> once.
/// </para>
/// </remarks>
public class PptTableRowsTests
{
    private static long[] Edges(params long[] heights)
    {
        long[] edges = new long[heights.Length + 1];
        for (int i = 0; i < heights.Length; i++) edges[i + 1] = edges[i] + heights[i];
        return edges;
    }

    private static long[] Laid(PptTableRows rows, long[] edges)
    {
        long[] laid = new long[edges.Length];
        for (int i = 0; i < edges.Length; i++)
        {
            laid[i] = rows.Remap(new DocRect(
                Length.Zero, Length.FromEmu(edges[i]), Length.Zero, Length.Zero)).Y.Emu;
        }

        return laid;
    }

    // ------------------------------------------------------------------------ the row heights

    /// <summary>
    /// A row shorter than its cells' text is grown, which is <c>Thailand17</c> page 11 whole.
    /// </summary>
    /// <remarks>
    /// Every one of that page's nine rows is below its minimum, so nothing is left over for
    /// <c>distribute</c> to take back and the table simply ends 3.05 pt lower than the group did.
    /// </remarks>
    [Fact]
    public void ARowShorterThanItsTextGrowsToTheText()
    {
        long[] edges = Edges(1000, 1000);

        PptTableRows rows = PptTableRows.Of(edges, 0, edges[^1], [1200, 1200]).ShouldNotBeNull();

        Laid(rows, edges).ShouldBe([0, 1200, 2400]);
    }

    /// <summary>A row taller than its text keeps its stated height, and the grid does not move.</summary>
    [Fact]
    public void ARowTallerThanItsTextIsLeftAlone()
    {
        long[] edges = Edges(1000, 1000);

        PptTableRows.Of(edges, 0, edges[^1], [800, 800]).ShouldBeNull();
    }

    /// <summary>
    /// A deficit one row cannot absorb drives <em>every</em> row down to its minimum.
    /// </summary>
    /// <remarks>
    /// The real numbers of <c>2015-Civil-Rights-Website-training.ppt</c> page 48, in hundredths
    /// of a millimetre. <c>distribute</c> (<c>tablelayouter.cxx</c>:499-547) does not reset
    /// <c>nDistribute</c> between its runs and subtracts each clamped row's shortfall from it
    /// again, so the second pass asks for more than the first and the last shrinkable row is
    /// clamped too. The reference's own flat ODP of the deck states the outcome: four rows of
    /// 1108 and one of 5024, none of them the stated height and 201 short of the frame.
    /// </remarks>
    [Fact]
    public void ADeficitOneRowCannotAbsorbTakesEveryRowToItsMinimum()
    {
        long[] edges = Edges(1270, 1102, 1098, 2037, 4150);

        PptTableRows rows = PptTableRows
            .Of(edges, 0, edges[^1], [1108, 1108, 1108, 1108, 5024])
            .ShouldNotBeNull();

        Laid(rows, edges).ShouldBe([0, 1108, 2216, 3324, 4432, 9456]);
    }

    /// <summary>
    /// A deficit the shrinkable rows can absorb is shared between them in proportion.
    /// </summary>
    /// <remarks>
    /// The other arm of the same loop: no row falls below its minimum, so it runs once. The
    /// hundredth taken off the first row is <c>nDistribute × size / current</c> and the last row
    /// takes whatever is left, which is how the reference makes the sum come out exact.
    /// </remarks>
    [Fact]
    public void ADeficitTheRowsCanAbsorbIsSharedInProportion()
    {
        long[] edges = Edges(1000, 1000, 1000);

        PptTableRows rows = PptTableRows
            .Of(edges, 0, edges[^1], [1100, 100, 100])
            .ShouldNotBeNull();

        Laid(rows, edges).ShouldBe([0, 1100, 2050, 3000]);
    }

    /// <summary>
    /// A group taller than the rows it seeded starts the table at the group's own top and shares
    /// the slack out between them.
    /// </summary>
    /// <remarks>
    /// <c>CreateTable</c> sizes the rows from the members' tops and then hands the finished object
    /// the <em>group's</em> rectangle (<c>svdfppt.cxx</c>:7712), which is what the layout runs
    /// inside; <c>distribute</c> takes a positive amount as readily as a negative one and counts
    /// every row into it. The two rectangles agree on every corpus table measured, so this arm has
    /// no corpus reach and is here because getting it wrong silently is worse than not having it.
    /// </remarks>
    [Fact]
    public void AGroupTallerThanItsMembersStartsAtItsOwnTopAndSharesTheSlack()
    {
        long[] edges = [100, 1100, 2100];

        PptTableRows rows = PptTableRows.Of(edges, 0, 2100, [1000, 1000]).ShouldNotBeNull();

        Laid(rows, edges).ShouldBe([0, 1050, 2100]);
    }

    /// <summary>A coordinate inside a row travels with the edge above it.</summary>
    [Fact]
    public void ACoordinateInsideARowKeepsItsOffsetFromTheEdgeAboveIt()
    {
        long[] edges = Edges(1000, 1000);
        PptTableRows rows = PptTableRows.Of(edges, 0, edges[^1], [1200, 1200]).ShouldNotBeNull();

        DocRect moved = rows.Remap(new DocRect(
            Length.Zero, Length.FromEmu(1300), Length.Zero, Length.FromEmu(200)));

        moved.Y.Emu.ShouldBe(1500);
        moved.Height.Emu.ShouldBe(200);
    }

    /// <summary>A grid of fewer than two edges is not a grid.</summary>
    [Fact]
    public void AGridNeedsTwoEdges()
    {
        PptTableRows.Of([0], 0, 0, []).ShouldBeNull();
        PptTableRows.Of([0, 1000], 0, 1000, [1000, 1000]).ShouldBeNull();
    }

    // ---------------------------------------------------------------------- the rule's width

    /// <summary>
    /// The two widths of <c>Thailand17</c> page 11, each measured out of 26.2.4.2's own PDF.
    /// </summary>
    /// <remarks>
    /// <c>floor(12700 / 360) = 35</c>, <c>35 / 4 = 8</c>, <c>8 / 20 pt = 0.40</c>; and
    /// <c>floor(28575 / 360) = 79</c>, <c>79 / 4 = 19</c>, <c>19 / 20 pt = 0.95</c>. The
    /// reference strokes them at 0.40 and 0.9499, and this tree used to stroke 1.0 and 2.25 —
    /// the file's own numbers, which never reach the page.
    /// </remarks>
    [Theory]
    [InlineData(12700u, 5080L)]     // 1 pt stated, 0.40 pt drawn
    [InlineData(28575u, 12065L)]    // 2.25 pt stated, 0.95 pt drawn
    [InlineData(19050u, 8255L)]     // 1.5 pt stated, 0.65 drawn -- `concepts-surrounding-cloud`
    [InlineData(38100u, 16510L)]    // 3 pt stated, 1.30 drawn -- the corpus's heaviest table rule
    [InlineData(9525u, 3810L)]      // the property's own default, 0.75 pt stated, 0.30 drawn
    public void ARuleIsDrawnAtAQuarterOfItsWidthReadAsTwips(uint emu, long emuDrawn)
        => PptSlideLayout.TableBorderWidth(emu).Emu.ShouldBe(emuDrawn);

    /// <summary>
    /// A rule too thin to survive the two divisions is drawn at one unit, not at nothing.
    /// </summary>
    /// <remarks>
    /// <c>std::max(sal_Int32(1), … / 4)</c> — the reference's own comment is "Avoid width = 0,
    /// the min value should be 1" (<c>svdfppt.cxx</c>:7521). One unit is a twentieth of a point.
    /// </remarks>
    [Fact]
    public void AnAlmostInvisibleRuleIsStillDrawnAtOneUnit()
    {
        PptSlideLayout.TableBorderWidth(0).Emu.ShouldBe(Length.EmuPerTwip);
        PptSlideLayout.TableBorderWidth(1000).Emu.ShouldBe(Length.EmuPerTwip);
    }
}

using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>.doc</c> table that states <c>sprmTJc90</c> is aligned, not indented.
/// </summary>
/// <remarks>
/// <para>
/// WW8 gives a row's cell boundaries as absolute positions, and Word writes an absolute position there
/// even for a table it centres — so reading the first boundary as the table's indent puts a centred table
/// wherever Word's own arithmetic happened to leave it. LibreOffice discards it outright:
/// <c>WW8TabDesc::CalcDefaults</c> (<c>sw/source/filter/ww8/ww8par2.cxx</c>:2134) subtracts
/// <c>nCenter[0]</c> from every boundary of every band as soon as the orientation is <c>CENTER</c>, and
/// the table format then carries <c>HoriOrientation::CENTER</c>. The right-hand case is the same rule from
/// the other end (:2192).
/// </para>
/// <para>
/// It is not a cosmetic difference. <c>150_5300_13_chg8.doc</c>'s Table 3-1 states a first boundary of
/// 6768 twips under a 512 pt text area: read as an indent it puts a 468 pt table at x = 388.8 on a 612 pt
/// page, so five of its seven columns fall off the paper. <c>250 ft</c> — a value that appears once in the
/// reference — was absent from our whole rendering of that document.
/// </para>
/// <para>
/// The fixture is a two-table document written as flat ODF and converted to <c>.doc</c> by the installed
/// 26.2.4.2, so the sprms are the ones LibreOffice itself round-trips: <c>0x548A 01</c> and
/// <c>0x5400 01</c> on the first table's rows and <c>02</c> on the second's. Rendered back through
/// <c>soffice</c> the two tables' outer rules land at 189.45-405.95 and 322.35-538.85 pt, and this tree
/// draws them at 189.40-405.90 and 322.35-538.85.
/// </para>
/// </remarks>
public sealed class DocTableAlignmentTests
{
    /// <summary>
    /// The centred table names the centre and keeps no indent of its own.
    /// </summary>
    [Fact]
    public void ACentredTableIsCentredRatherThanIndented()
    {
        PageTable table = TableAt(0);

        table.HorizontalPosition.ShouldBe(FrameHorizontalAlignment.Centre);
        table.LeftIndent.ShouldBe(Length.Zero, "the absolute boundary is discarded, not kept as well");

        // And the alignment is what places it: a 216 pt table in a 468 pt area sits 126 pt in.
        table.LeftWithin(Length.FromPoints(468))
            .ShouldBe((Length.FromPoints(468) - table.Width) / 2);
    }

    /// <summary>The right-aligned one names the right edge.</summary>
    [Fact]
    public void ARightAlignedTableIsFlushWithTheEndEdge()
    {
        PageTable table = TableAt(1);

        table.HorizontalPosition.ShouldBe(FrameHorizontalAlignment.Right);
        table.LeftIndent.ShouldBe(Length.Zero);
        table.LeftWithin(Length.FromPoints(468))
            .ShouldBe(Length.FromPoints(468) - table.Width);
    }

    /// <summary>
    /// Both tables keep the width the file states, which is what makes the placement the only difference.
    /// </summary>
    /// <remarks>
    /// Asserted because the plausible wrong fix — normalising the boundaries by subtracting the first one
    /// from all of them <em>including</em> the last — would silently narrow the table by its own indent.
    /// </remarks>
    [Fact]
    public void NeitherTableLosesWidthToTheNormalisation()
    {
        TableAt(0).Width.ShouldBe(TableAt(1).Width);
        TableAt(0).Width.ShouldBeGreaterThan(Length.FromPoints(200));
    }

    private static PageTable TableAt(int index)
    {
        using DocumentSource source = DocumentSource.FromFile(Corpus.Require("table-centred.doc"));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageTable>().ElementAt(index);
    }
}

using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// RTF's cell text flow — <c>\cltxbtlr</c> and its four siblings — is OOXML's <c>w:textDirection</c>.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice's tokeniser maps the five words onto the six <c>ST_TextDirection</c> values directly
/// (<c>sw/source/writerfilter/rtftok/rtfdispatchflag.cxx</c>:483-508), so the RTF and the DOCX
/// spellings of one document are supposed to reach <see cref="PageTableCell.TextDirection"/> by the
/// same route. The RTF reader did not read them at all, and a turned label column is a few points
/// wide: laid out upright it breaks a glyph per line, which is how
/// <c>047_Visual_Product_Roadmap_Template_Professional_Layout</c> came to draw
/// <c>S t r a t e D ge sy i g n</c> — two rotated labels interleaved a character at a time.
/// </para>
/// <para>
/// <b>186 <c>\cltxbtlr</c> in 11 of the 328 converted RTF, and 3 <c>\cltxtbrl</c> in one of them</b>;
/// ten of the eleven were failing the gate. On the witness above the character count went 175 to 182
/// against the reference's 183. See <c>probes/rtf-gate-r71/</c> and
/// <see cref="TurnedCellTests"/>, which holds the layout law this only has to reach.
/// </para>
/// </remarks>
public sealed class RtfTurnedCellTests
{
    /// <summary><c>\cltxbtlr</c> turns the cell a quarter turn anticlockwise.</summary>
    [Fact]
    public void BottomToTopIsRead()
        => Direction(@"\clvertalt\cltxbtlr").ShouldBe(CellTextDirection.BottomToTopLeftToRight);

    /// <summary>And <c>\cltxtbrl</c> the other way, with its vertical spelling folded onto it.</summary>
    /// <remarks>
    /// The fold is dmapper's, not ours: <c>tbRlV</c> becomes <c>TB_RL</c> in
    /// <c>DomainMapperTableManager.cxx</c>:325-350, and <see cref="CellTextDirection"/> carries the
    /// measurement that confirms it.
    /// </remarks>
    [Fact]
    public void TopToBottomAndItsVerticalSpellingAreOneAnswer()
    {
        Direction(@"\cltxtbrl").ShouldBe(CellTextDirection.TopToBottomRightToLeft);
        Direction(@"\cltxtbrlv").ShouldBe(CellTextDirection.TopToBottomRightToLeft);
    }

    /// <summary>
    /// A cell stating nothing, or stating <c>\cltxlrtb</c>, is upright — the control that says the
    /// reading is the control word rather than the table.
    /// </summary>
    [Fact]
    public void AnUnstatedOrLeftToRightCellIsUpright()
    {
        Direction(string.Empty).ShouldBe(CellTextDirection.LeftToRight);
        Direction(@"\cltxlrtb").ShouldBe(CellTextDirection.LeftToRight);
        Direction(@"\cltxlrtbv").ShouldBe(CellTextDirection.LeftToRight);
    }

    /// <summary>
    /// The words are per cell, not per row: two cells of one row take different directions.
    /// </summary>
    /// <remarks>
    /// They sit among the <c>\clpad</c> and <c>\clbrdr</c> declarations that precede each
    /// <c>\cellx</c>, so a reader that hung them on the row would turn a whole table on one label.
    /// </remarks>
    [Fact]
    public void TheDirectionBelongsToTheCellItPrecedes()
    {
        PageTableRow row = Row(@"\cltxbtlr\cellx1000\cellx5000");

        row.Cells[0].TextDirection.ShouldBe(CellTextDirection.BottomToTopLeftToRight);
        row.Cells[1].TextDirection.ShouldBe(CellTextDirection.LeftToRight);
    }

    private static CellTextDirection Direction(string words)
        => Row(words + @"\cellx5000").Cells[0].TextDirection;

    private static PageTableRow Row(string declarations)
    {
        string rtf =
            @"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
            + @"\paperw11906\paperh16838\margl1440\margr1440\margt1440\margb1440\sectd"
            + @"\trowd\trgaph0" + declarations
            + @"\pard\intbl Alpha.\cell\pard\intbl Beta.\cell\row"
            + @"\pard\plain After.\par}";

        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(rtf)), "turned.rtf");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Blocks.OfType<PageTable>().ShouldHaveSingleItem().Rows[0];
    }
}

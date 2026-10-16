using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The <c>\pard</c> inside a <c>{\listtext …}</c> label does not take the paragraph out of its cell.
/// </summary>
/// <remarks>
/// <para>
/// RTF has no list counters, so Word writes the rendered <c>1.</c> or <c>•</c> of every numbered item
/// into a <c>{\listtext\pard\plain \tab}</c> group ahead of the paragraph's own text. That <c>\pard</c>
/// belongs to the label. Table membership, though, is held on the flow rather than on the group stack —
/// it has to be, because it outlives the paragraph — so unlike everything else <c>\pard</c> resets it
/// did not come back when the group closed, and the paragraph carrying the label stopped being in the
/// table. Every cell of such a row was then read as body text and the row never formed.
/// </para>
/// <para>
/// LibreOffice states the rule in its own source: "\pard is allowed between \cell and \row, but in that
/// case it should not reset the fact that we're inside a table"
/// (<c>sw/source/writerfilter/rtftok/rtfdispatchflag.cxx</c>:588). Its renderer agrees — 26.2.4.2 draws
/// the document below as one two-cell row with the label in front of the first cell.
/// </para>
/// <para>
/// Reach: <b>21 709 such labels in 179 of the corpus's 338 converted RTF</b>, and the documents holding
/// the most of them are the column's largest deficits —
/// <c>02_mcar_part-2_and_IS_v2.10</c> (3603), <c>SPA-02_mcar_part-2</c> (3587),
/// <c>FAA 2025-26 Holdover Tables</c> (1410) and <c>150-5370-10H</c> (314).
/// </para>
/// </remarks>
public sealed class RtfListLabelInCellTests
{
    /// <summary>A cell whose paragraph carries a list label is still a cell.</summary>
    [Fact]
    public void ARowSurvivesAListLabelInItsFirstCell()
    {
        PageTable table = Lay(Labelled).Blocks.OfType<PageTable>().ShouldHaveSingleItem();

        table.Rows.Count.ShouldBe(1);
        table.Rows[0].Cells.Count.ShouldBe(2);
    }

    /// <summary>
    /// And it holds the same cells as the identical row written without a label.
    /// </summary>
    /// <remarks>
    /// The control: it is the label that used to break the row, not anything else in the declaration.
    /// </remarks>
    [Fact]
    public void ALabelledRowHoldsWhatAnUnlabelledOneHolds()
        => TextOf(Lay(Labelled)).ShouldBe(TextOf(Lay(Plain)));

    /// <summary>
    /// A <c>\pard</c> the body itself writes after <c>\row</c> still closes the table.
    /// </summary>
    /// <remarks>
    /// The other control, and the reason the reset is gated on the destination rather than removed:
    /// without it a paragraph after a table stays in it and the table never closes.
    /// </remarks>
    [Fact]
    public void ABodyPardAfterTheRowStillLeavesTheTable()
    {
        WordProcessingPages pages = Lay(Plain);

        pages.Blocks.OfType<PageTable>().ShouldHaveSingleItem();
        pages.Blocks.OfType<PageParagraph>()
            .Any(p => p.Text.Contains("After the table")).ShouldBeTrue();
    }

    private const string Labelled =
        @"\pard\plain\ilvl0\intbl{\listtext\pard\plain \tab}\ls1 \fi0\li0{\f0 ALPHA}\cell"
        + @"\pard\plain\intbl{\f0 BETA}\cell\row";

    private const string Plain =
        @"\pard\plain\intbl{\f0 ALPHA}\cell\pard\plain\intbl{\f0 BETA}\cell\row";

    private static string TextOf(WordProcessingPages pages)
    {
        PageTable table = pages.Blocks.OfType<PageTable>().ShouldHaveSingleItem();
        return string.Join(
            "|",
            table.Rows.SelectMany(r => r.Cells).Select(
                c => string.Concat(c.Blocks.OfType<PageParagraph>().Select(p => p.Text)).Trim()));
    }

    private static WordProcessingPages Lay(string row)
    {
        string rtf =
            @"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
            + @"{\*\listtable{\list\listtemplateid1{\listlevel\levelnfc0"
            + @"\leveltext\'02\'00.;\levelnumbers\'01;}\listid1}}"
            + @"{\*\listoverridetable{\listoverride\listid1\listoverridecount0\ls1}}"
            + @"\paperw11906\paperh16838\margl1440\margr1440\margt1440\margb1440\sectd"
            + @"\trowd\trgaph0\cellx3000\cellx6000" + row
            + @"\pard\plain After the table.\par}";

        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(rtf)), "listlabel.rtf");
        using IDocument document = new WordProcessingReader().Read(source);
        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }
}

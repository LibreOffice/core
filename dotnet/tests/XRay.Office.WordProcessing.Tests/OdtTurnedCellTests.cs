using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// An ODF cell's text direction is <c>writing-mode</c>, and the namespace it wears depends on its
/// <em>value</em>.
/// </summary>
/// <remarks>
/// <para>
/// Writer's ODF filter maps both spellings onto one item — its table-cell item map holds
/// <c>style:writing-mode</c> and <c>loext:writing-mode</c> side by side against <c>RES_FRAMEDIR</c>
/// (<c>sw/source/filter/xml/xmlitemm.cxx</c>:281-282) and the value handler takes <c>bt-lr</c> and
/// <c>tb-rl90</c> whichever namespace carried them
/// (<c>sw/source/filter/xml/xmlimpit.cxx</c>:1008-1030). The <em>exporter</em> is what makes the
/// namespace look like part of the property: it writes the two values ODF 1.3 does not define into the
/// extension namespace and every other value into <c>style:</c>
/// (<c>sw/source/filter/xml/xmlexpit.cxx</c>:193-220, and <c>CheckExtendedNamespace</c> at
/// <c>xmloff/source/style/xmlexppr.cxx</c>:947-952 for the generic path, under a comment saying there is
/// no generic mechanism for it).
/// </para>
/// <para>
/// So a reader that consults the specification's spelling alone finds <em>none</em> of the turned cells
/// a real document holds: the converted corpus states <b>86 <c>loext:writing-mode="bt-lr"</c> on table
/// cells in 11 of its 338 <c>.odt</c></b> and not one <c>style:</c> spelling of that value.
/// <see cref="TurnedCellTests"/> holds the layout law this only has to reach, and
/// <see cref="RtfTurnedCellTests"/> the same reading for RTF, which was closed a round earlier.
/// </para>
/// <para>
/// Ground truth is LibreOffice 26.2.4.2's own PDF of the fixture, read out of the content stream: on a
/// 1 in row of 108 pt columns UPRIGHT is drawn left to right at (73.550, 74.458), LOEXTBT and STYLEBT
/// bottom to top at x 181.958 and 289.958 — exactly one column apart and both ending at y 142.450 — and
/// CLOCKTB top to bottom at (488.758, 74.050).
/// </para>
/// </remarks>
public sealed class OdtTurnedCellTests
{
    /// <summary>Half a point, the tolerance the rest of this suite compares a position at.</summary>
    private const double Tolerance = 0.5;

    /// <summary>
    /// The extension spelling of <c>bt-lr</c> turns the cell a quarter turn anticlockwise.
    /// </summary>
    /// <remarks>
    /// This is the whole of the corpus's reach. Read as upright the label breaks at the column's width
    /// instead, which for a label column is a glyph per line — the defect that made
    /// <c>047_Visual_Product_Roadmap_Template_Professional_Layout</c> draw
    /// <c>S t r a t e g y</c> down its first column.
    /// </remarks>
    [Fact]
    public void TheExtensionSpellingOfBottomToTopIsRead()
        => Row().Cells[1].TextDirection.ShouldBe(CellTextDirection.BottomToTopLeftToRight);

    /// <summary>
    /// And so does the specification's spelling of the same value, which is the point of the pair.
    /// </summary>
    /// <remarks>
    /// Measured rather than assumed: 26.2.4.2 draws <c>style:writing-mode="bt-lr"</c> and
    /// <c>loext:writing-mode="bt-lr"</c> identically, one column apart, on this fixture. The extension
    /// namespace is therefore an export convention and not an import requirement, so reading one spelling
    /// and not the other would be right on every real document and wrong on the rule.
    /// </remarks>
    [Fact]
    public void SoIsTheSpecificationSpellingOfTheSameValue()
        => Row().Cells[2].TextDirection.ShouldBe(CellTextDirection.BottomToTopLeftToRight);

    /// <summary><c>tb-rl</c> is the other turn, and a cell stating nothing is upright.</summary>
    [Fact]
    public void TopToBottomTurnsTheOtherWayAndAnUnstatedCellIsUpright()
    {
        PageTableRow row = Row();

        row.Cells[3].TextDirection.ShouldBe(CellTextDirection.TopToBottomRightToLeft);
        row.Cells[0].TextDirection.ShouldBe(CellTextDirection.LeftToRight);
    }

    /// <summary>
    /// The two turned labels are drawn where 26.2.4.2 draws them: one line each, one column apart.
    /// </summary>
    /// <remarks>
    /// The reading has to reach the layout, and the number that shows it is the line count. A 1 in row
    /// gives the turned flow 69.1 pt of line, which fits <c>LOEXTBT</c> whole; read upright the same label
    /// in a 108 pt column would still be one line, so the discriminating assertion is the transform's
    /// origin — the flow starts at the bottom-left of the turned cell's inner box, 108 pt apart for the
    /// two spellings, exactly as the reference's two runs are.
    /// </remarks>
    [Fact]
    public void BothTurnedCellsAreDrawnWhereTheReferenceDrawsThem()
    {
        IReadOnlyList<PlacedTableCell> cells = PlacedCells();

        cells[1].ContentTransform.ShouldNotBeNull();
        cells[2].ContentTransform.ShouldNotBeNull();
        cells[1].Content!.Lines.Count.ShouldBe(1);
        cells[2].Content!.Lines.Count.ShouldBe(1);

        double first = cells[1].ContentTransform!.Value.E / (double)Length.FromPoints(1).Emu;
        double second = cells[2].ContentTransform!.Value.E / (double)Length.FromPoints(1).Emu;

        (second - first).ShouldBe(289.958 - 181.958, Tolerance);
    }

    /// <summary>
    /// The upright control is untouched, which is what says the turn is the cell's and not the table's.
    /// </summary>
    [Fact]
    public void TheUprightCellIsNotTurned()
    {
        Dictionary<string, DrawnWord> words = Words();

        words["UPRIGHT"].Left.ShouldBe(73.550, Tolerance);
        words["AFTER"].Left.ShouldBe(72.100, Tolerance);
    }

    /// <summary>The fixture's single table row.</summary>
    private static PageTableRow Row()
    {
        using DocumentSource source = DocumentSource.FromFile(Corpus.Require("odt-turned-cell.fodt"));
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Blocks.OfType<PageTable>().ShouldHaveSingleItem().Rows[0];
    }

    /// <summary>The fixture's placed cells, in column order.</summary>
    private static IReadOnlyList<PlacedTableCell> PlacedCells()
    {
        using DocumentSource source = DocumentSource.FromFile(Corpus.Require("odt-turned-cell.fodt"));
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        pages.Count.ShouldBe(1);

        return [.. pages.Pages[0].Tables.ShouldHaveSingleItem().Cells
            .OrderBy(cell => cell.Area.X.Emu)];
    }

    /// <summary>The first page's words, by their text.</summary>
    private static Dictionary<string, DrawnWord> Words()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require("odt-turned-cell.fodt")))
        {
            using IDocument document = new WordProcessingReader().Read(source);
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1);
            pages[0].Draw(sink);
        }

        return DrawnWords.On(sink.Pages[0]).ToDictionary(word => word.Text, word => word);
    }
}

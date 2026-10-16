using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A cell's trailing empty paragraph is a line of its row's height, whether or not the cell is
/// in several formats.
/// </summary>
/// <remarks>
/// <para>
/// Calc measures a multi-paragraph cell through the EditEngine and its height is
/// <c>pEngine-&gt;GetTextHeight()</c> over every paragraph the importer made
/// (<c>sc/source/core/data/column2.cxx</c>:571-577); ODF's filter makes one paragraph per
/// <c>text:p</c>, the trailing empty one included. So a cell ending in a hard break reserves a
/// line for the paragraph after it — a line that draws no glyph, which is why the difference
/// appears in the row height and nowhere in the text.
/// </para>
/// <para>
/// This tree already counted it for a cell in one format, because
/// <see cref="SheetTextLayout.LineCount"/> splits on the break before it wraps anything and a
/// trailing break leaves an empty final piece. A cell in several formats goes through
/// <see cref="SheetTextLayout.RichLineRanges"/> instead, and the layouter there ends a paragraph
/// on its break without opening the empty one after it — <c>"a\n"</c> lays out as one line. The
/// two answers have to agree, because Calc's do.
/// </para>
/// <para>
/// The fixture's expected heights were read out of 26.2.4.2's own <c>--convert-to fods</c> of it
/// before anything was asserted, and rows 0, 2 and 4 are the controls: two of them hold the same
/// characters without the empty paragraph and must not move, and the third is a single line.
/// <c>dotnet/probes/sheet-wrap-r99</c> has the measurement and the corpus reach.
/// </para>
/// </remarks>
public sealed class SheetTrailingParagraphHeightTests
{
    private static SheetAxis Rows()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-wrap-trailing-paragraph.fods"));

        return ((SpreadsheetPages)document.Layout()).Pages[0].Sheet.Grid.Rows;
    }

    /// <summary>Two paragraphs are two lines, in one format and in several alike.</summary>
    [Theory]
    [InlineData(0)]
    [InlineData(2)]
    public void ACellWithoutATrailingBreakIsAsTallAsItsParagraphs(int row)
        => Rows().SizeAt(row).Twips.ShouldBe(567);

    /// <summary>
    /// The same characters with an empty paragraph after them are one line taller — and the rich
    /// cell answers the same number as the plain one.
    /// </summary>
    [Theory]
    [InlineData(1)]
    [InlineData(3)]
    public void ATrailingEmptyParagraphIsALineOfItsOwn(int row)
        => Rows().SizeAt(row).Twips.ShouldBe(835);

    /// <summary>The step is exactly one line, and it is the same line for both cells.</summary>
    /// <remarks>
    /// Stated as the difference as well as as the two heights, because that is the quantity the
    /// reference's own <c>style:row-height</c> shows: 0.3937 in against 0.5799 in, twice.
    /// </remarks>
    [Fact]
    public void TheStepIsOneLineAndTheRichCellTakesTheSameOneAsThePlain()
    {
        SheetAxis rows = Rows();

        (rows.SizeAt(1) - rows.SizeAt(0)).Twips.ShouldBe(268);
        (rows.SizeAt(3) - rows.SizeAt(2)).Twips.ShouldBe(268);
        rows.SizeAt(3).Twips.ShouldBe(rows.SizeAt(1).Twips);
    }

    /// <summary>The single-line control, so that the fixture is not on the height floor.</summary>
    [Fact]
    public void TheSingleLineRowIsOneLine() => Rows().SizeAt(4).Twips.ShouldBe(298);
}

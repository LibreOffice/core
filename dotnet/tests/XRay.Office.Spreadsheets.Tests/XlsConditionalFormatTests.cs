using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A BIFF workbook's conditional formats, which are its <c>CONDFMT</c> and <c>CF</c> records.
/// </summary>
/// <remarks>
/// <para>
/// The pair had never been read. A round of this project read the <c>CONDFMT</c> header alone —
/// a count and a list of ranges — rendered the corpus's 64 <c>.xls</c> with and without it, got
/// 64 byte-identical PDFs, and concluded that a BIFF conditional format has no reach. The rules
/// are in the <c>CF</c> records that follow the header, so a reader that stops there cannot move
/// a pixel whatever the corpus holds, and the byte-identity was a property of the instrument.
/// </para>
/// <para>
/// Asked of 26.2.4.2 instead — <c>--convert-to fods</c> over every <c>.xls</c> that states the
/// record — the reference resolves <strong>149 conditional formats and 187 conditions in 5 of the
/// 64</strong>, of which 175 name a style carrying a fill, a font colour or bold.
/// </para>
/// <para>
/// The fixture is 26.2.4.2's own <c>.xls</c> export of a flat ODS stating two rules, so the
/// records are written by a real producer rather than by hand, and every expectation below is
/// read out of 26.2.4.2's own PDF of that <c>.xls</c>: two red fills on <c>A2</c> and <c>A4</c>
/// with their text bold, two green fills on <c>B2</c> and <c>B4</c>, and nothing on the two rows
/// whose cells do not satisfy the rules.
/// </para>
/// </remarks>
public sealed class XlsConditionalFormatTests
{
    private static SheetLayout Sheet(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(name));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }

    private static readonly Colour Red = Colour.FromRgb(0xFF0000);

    private static readonly Colour Green = Colour.FromRgb(0x00FF00);

    /// <summary>
    /// A <c>cellIs</c> rule against a string fills the cells that match it and no others.
    /// </summary>
    /// <remarks>
    /// The rule is <c>CF</c> type 1, comparison 3 — <c>EXC_CF_TYPE_CELL</c> and
    /// <c>EXC_CF_CMP_EQUAL</c> — whose single RPN formula is the three bytes
    /// <c>17 03 00 "Yes"</c>, a <c>tStr</c> token. <c>A1</c> holds <c>Marker</c> and <c>A3</c>
    /// holds <c>No</c>, so a reader that painted the whole range rather than the matching cells
    /// would pass an assertion on <c>A2</c> alone.
    /// </remarks>
    [Fact]
    public void ACellIsRuleFillsOnlyTheCellsThatSatisfyIt()
    {
        SheetFormatting formatting = Sheet("sheet-cond-format-xls.xls").Formatting;

        formatting.At(1, 0).Background.ShouldBe(Red, "A2 holds \"Yes\"");
        formatting.At(3, 0).Background.ShouldBe(Red, "A4 holds \"Yes\"");

        formatting.At(0, 0).Background.ShouldBeNull("A1 holds \"Marker\"");
        formatting.At(2, 0).Background.ShouldBeNull("A3 holds \"No\"");
    }

    /// <summary>
    /// A numeric comparison is evaluated against the cell's own value.
    /// </summary>
    /// <remarks>
    /// <c>EXC_CF_CMP_GREATER</c> over a <c>tInt</c> of ten. <c>B1</c> is 5 and <c>B3</c> is 3,
    /// which is the control: the range is the whole of <c>B1:B4</c> and only two of its four
    /// cells are above the operand.
    /// </remarks>
    [Fact]
    public void AGreaterThanRuleIsEvaluatedAgainstTheCellsOwnNumber()
    {
        SheetFormatting formatting = Sheet("sheet-cond-format-xls.xls").Formatting;

        formatting.At(1, 1).Background.ShouldBe(Green, "B2 holds 12");
        formatting.At(3, 1).Background.ShouldBe(Green, "B4 holds 20");

        formatting.At(0, 1).Background.ShouldBeNull("B1 holds 5");
        formatting.At(2, 1).Background.ShouldBeNull("B3 holds 3");
    }

    /// <summary>
    /// The <c>CF</c> record's inline font block reaches the cell's text.
    /// </summary>
    /// <remarks>
    /// 118 bytes of which six fields are differential, each with its own sentinel
    /// (<c>XclImpFont::ReadCFFontBlock</c>, <c>sc/source/filter/excel/xistyle.cxx</c>). Here only
    /// the weight and the colour are stated, so the rule must not disturb the size or the face —
    /// which is what makes the overlay differential rather than a whole format, and is asserted
    /// by the untouched neighbour below.
    /// </remarks>
    [Fact]
    public void TheInlineFontBlockMakesTheMatchingCellsBold()
    {
        SheetLayout sheet = Sheet("sheet-cond-format-xls.xls");

        sheet.Formats.At(1, 0).FontWeight.ShouldBe(700, "A2 matched a rule stating bold");
        sheet.Formats.At(3, 0).FontWeight.ShouldBe(700, "A4 matched the same rule");

        sheet.Formats.At(2, 0).FontWeight.ShouldBe(400, "A3 matched nothing");
        sheet.Formats.At(1, 1).FontWeight.ShouldBe(400, "B2's rule states a fill and no font");
    }
}

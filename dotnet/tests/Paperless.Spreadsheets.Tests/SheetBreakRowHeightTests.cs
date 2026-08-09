using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A hard break costs a row a line only when the cell's format wraps, and a break at the end of
/// the text costs one too.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The wrap flag decides, not the text.</strong> Both SpreadsheetML and BIFF hand a
/// multi-line string to an EditEngine put into single-line mode when the cell does not wrap —
/// <c>bSingleLine = !pXf-&gt;getAlignment().getModel().mbWrapText</c> then
/// <c>rEE.SetSingleLine(bSingleLine)</c> (<c>sc/source/filter/oox/sheetdatabuffer.cxx:125-133</c>
/// over <c>worksheethelper.cxx:1607-1611</c>), and <c>bSingleLine = !pXF-&gt;GetLineBreak()</c>
/// before the same call on the binary side (<c>sc/source/filter/excel/xihelper.cxx:246-256</c>).
/// One paragraph comes out of it, so the U+000A stays in the text and starts nothing. The height
/// path says the same thing from the other end — <c>bStdOnly</c> is <c>!bBreak</c>,
/// <c>sc/source/core/data/column2.cxx:930-935</c> — and so does the drawing, where
/// <c>ScDrawStringsVars::HasEditCharacters</c> (<c>output2.cxx:823-847</c>) lists seven code
/// points and U+000A is not one of them.
/// </para>
/// <para>
/// <strong>A trailing break is an empty paragraph, and it takes a line.</strong> That one is
/// about our own extraction rather than about Calc: <see cref="Paperless.Core.Extraction"/>'s
/// table cell used to strip every trailing newline where one of them was the empty final
/// paragraph itself.
/// </para>
/// <para>
/// Every figure below is LibreOffice 24.2.7.2's own, read out of <c>style:row-height</c> in its
/// flat-ODF export of this fixture: <strong>252.9, 700.7, 700.7, 476.8, 256.3</strong> twips
/// against our 256, 701, 701, 477, 256.
/// </para>
/// <para>
/// The 3.4 twips on row 1 are a different quantity and are left open. Calc keeps a multi-line
/// string as an <c>EditTextObject</c> even in single-line mode, so <c>HasEditCells</c> sends the
/// row through <c>GetNeededSize</c> — which measures one line and applies <em>no</em> floor —
/// while row 5's plain string takes <c>lcl_GetAttribHeight</c> and is floored at the sheet's
/// optimal minimum. LibreOffice therefore writes 252.9 for row 1 and 256.3 for row 5; we write
/// 256 for both, because we default that floor to the standard row height and reach it along the
/// arithmetic path in both cases. A fifth of a point on a row, and not what this fixture is for.
/// </para>
/// </remarks>
public sealed class SheetBreakRowHeightTests
{
    private const string Fixture = "sheet-row-height-break.xlsx";

    private static SheetAxis Rows()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(Fixture));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Grid.Rows;
    }

    /// <summary>The four heights, in the fixture's own order.</summary>
    [Theory]
    [InlineData(0, 256)]   // Alpha\nBravo\nCharlie, no wrap  — one line
    [InlineData(1, 701)]   // Alpha\nBravo\nCharlie, wrap     — three
    [InlineData(2, 701)]   // Alpha\nBravo\n,        wrap     — three, the last of them empty
    [InlineData(3, 477)]   // Alpha\nBravo,          wrap     — two
    [InlineData(4, 256)]   // Alpha,                 no wrap  — one
    public void TheRowIsTheHeightLibreOfficeGivesIt(int row, int twips)
        => Rows().SizeAt(row).Twips.ShouldBe(twips);

    /// <summary>
    /// The same text in the same column is three lines when the cell wraps and one when it does
    /// not.
    /// </summary>
    /// <remarks>
    /// The two rows differ in nothing but their <c>wrapText</c>, so this is the rule on its own
    /// with the font, the column and the string held still. Reading a break out of the text
    /// rather than out of the format makes them equal.
    /// </remarks>
    [Fact]
    public void ABreakCostsALineOnlyWhenTheCellWraps()
    {
        SheetAxis rows = Rows();

        rows.SizeAt(0).ShouldBeLessThan(rows.SizeAt(1));
        rows.SizeAt(0).ShouldBe(rows.SizeAt(4));
    }

    /// <summary>A break at the end of the text is worth a line, and exactly one.</summary>
    /// <remarks>
    /// Rows 3 and 4 are <c>Alpha\nBravo\n</c> and <c>Alpha\nBravo</c>. The difference between
    /// them is one empty paragraph, and it is also the difference between row 3 and the
    /// three-paragraph row 2 being none at all.
    /// </remarks>
    [Fact]
    public void ATrailingBreakIsAParagraphAndTakesALine()
    {
        SheetAxis rows = Rows();

        rows.SizeAt(2).ShouldBeGreaterThan(rows.SizeAt(3));
        rows.SizeAt(2).ShouldBe(rows.SizeAt(1));
        (rows.SizeAt(2).Twips - rows.SizeAt(3).Twips).ShouldBe(rows.SizeAt(3).Twips - 253);
    }
}

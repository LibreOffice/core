using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A hard break inside a cell starts a line of its own, whatever room the column has left.
/// </summary>
/// <remarks>
/// <para>
/// Calc keeps such a cell as an <c>EditTextObject</c> rather than as a string, and
/// <c>ScOutputData::LayoutStringsImpl</c> sends every <c>CELLTYPE_EDIT</c> cell to
/// <c>DrawEdit</c> without asking anything else about it (<c>output2.cxx:1711-1712</c>), where
/// one paragraph is one line. So the width the column has left decides where a paragraph is
/// broken and never whether it is.
/// </para>
/// <para>
/// The defect this pins is the opposite reading. <see cref="SheetTextLayout"/> shaped the cell's
/// whole text and returned it as a single line whenever that fitted the column, so
/// <c>Alpha\nBravo\nCharlie</c> in a six-centimetre column — which fits three times over — was
/// drawn concatenated on one line, while <c>LineCount</c> beside it split on the break first and
/// reserved three lines' height for it. Measured on <c>CSA_CCM_v1.2.xls</c>, 13 pages against 13
/// and 17079 extractable words against 15852, of which 1123 were this.
/// </para>
/// <para>
/// Every figure asserted here is LibreOffice 24.2.7.2's own, read out of its PDF of the fixture
/// with <c>pdftotext -bbox</c>; the fixture's comment lists them row by row. The pitch is
/// 11.197 pt, so "two pitches apart" below is the empty paragraph in row 3 taking a line.
/// </para>
/// <para>
/// Row 2 of the fixture is deliberately not asserted as correct. It is the same three strings in
/// a cell that does not wrap, and ODF's importer disagrees with the other two about it: it makes
/// a multi-paragraph edit cell whatever the wrap option says, so LibreOffice draws three lines,
/// while BIFF and SpreadsheetML both fold them onto one paragraph
/// (<c>XclImpStringHelper::SetToDocument</c>, <c>xihelper.cxx:250-256</c>;
/// <c>SheetDataBuffer::setStringCell</c>, <c>sheetdatabuffer.cxx:120-135</c>, which every string
/// holding U+000A reaches because <c>RichString::extractPlainString</c> refuses it at
/// <c>richstring.cxx:375</c>). The sheets corpus is entirely <c>.xls</c> and <c>.xlsx</c>, so the
/// one line we draw is what those two ask for and only the ODF side is outstanding.
/// </para>
/// </remarks>
public sealed class SheetHardBreakTests
{
    private static List<DrawnGlyphRun> Drawn(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);
        return sink.Pages[0].Runs;
    }

    /// <summary>
    /// The cell that fits its column three times over is still drawn on three lines.
    /// </summary>
    /// <remarks>
    /// The case the whole fix is about: a break the layouter never reached, because the text as
    /// one string was narrower than the room it had.
    /// </remarks>
    [Fact]
    public void ABreakBreaksEvenWhenTheWholeTextWouldFit()
    {
        List<DrawnGlyphRun> drawn = Drawn("sheet-cell-hard-break.fods");

        drawn.Select(r => r.Text).ShouldContain("Alpha");
        drawn.Select(r => r.Text).ShouldContain("Bravo");
        drawn.Select(r => r.Text).ShouldContain("Charlie");

        // Not merely present: on three lines, left-aligned together, one pitch apart. A run
        // holding all three would satisfy a "contains" test on any substring of it.
        DrawnGlyphRun alpha = drawn.Single(r => r.Text == "Alpha");
        DrawnGlyphRun bravo = drawn.Single(r => r.Text == "Bravo");
        DrawnGlyphRun charlie = drawn.Single(r => r.Text == "Charlie");

        bravo.Origin.X.ShouldBe(alpha.Origin.X);
        charlie.Origin.X.ShouldBe(alpha.Origin.X);
        (bravo.Origin.Y - alpha.Origin.Y).ShouldBe(charlie.Origin.Y - bravo.Origin.Y);
        (bravo.Origin.Y - alpha.Origin.Y).Points.ShouldBe(11.197, 0.05);
    }

    /// <summary>An empty paragraph between two others still takes a line.</summary>
    /// <remarks>
    /// LibreOffice puts <c>Golf</c> at 127.83 and <c>Hotel</c> at 150.22, which is two pitches
    /// rather than one. The line has no glyphs to find, so it is asserted as the gap it leaves.
    /// </remarks>
    [Fact]
    public void AnEmptyParagraphTakesALineOfItsOwn()
    {
        List<DrawnGlyphRun> drawn = Drawn("sheet-cell-hard-break.fods");

        DrawnGlyphRun golf = drawn.Single(r => r.Text == "Golf");
        DrawnGlyphRun hotel = drawn.Single(r => r.Text == "Hotel");

        (hotel.Origin.Y - golf.Origin.Y).Points.ShouldBe(2 * 11.197, 0.1);
    }

    /// <summary>
    /// A paragraph wider than its column is still wrapped, and the break still ends it.
    /// </summary>
    /// <remarks>
    /// Row 5: <c>Mike November</c> does not fit two centimetres and breaks in the middle, then
    /// the paragraph break ends the line whatever room <c>Oscar Papa</c> would have had after it.
    /// A fix that only special-cased short cells would put <c>Oscar</c> beside <c>November</c>.
    /// </remarks>
    [Fact]
    public void TheBreakEndsALineTheWrapWouldHaveCarriedOn()
    {
        List<DrawnGlyphRun> drawn = Drawn("sheet-cell-hard-break.fods");

        DrawnGlyphRun mike = drawn.Single(r => r.Text.StartsWith("Mike", StringComparison.Ordinal));
        DrawnGlyphRun november =
            drawn.Single(r => r.Text.StartsWith("November", StringComparison.Ordinal));
        DrawnGlyphRun oscar = drawn.Single(r => r.Text.StartsWith("Oscar", StringComparison.Ordinal));

        november.Origin.Y.ShouldBeGreaterThan(mike.Origin.Y);
        oscar.Origin.Y.ShouldBeGreaterThan(november.Origin.Y);
        oscar.Text.Trim().ShouldBe("Oscar Papa");
    }

    /// <summary>The control: a cell with no break wraps exactly as it always did.</summary>
    /// <remarks>
    /// Row 4, <c>India Juliett Kilo Lima</c> in the same two-centimetre column, which LibreOffice
    /// draws as <c>India Juliett</c> and <c>Kilo Lima</c>. Without this a change that sent every
    /// cell through the layouter, or one that broke on every space, would pass everything above.
    /// </remarks>
    [Fact]
    public void ACellWithNoBreakWrapsAsItDid()
    {
        List<DrawnGlyphRun> drawn = Drawn("sheet-cell-hard-break.fods");

        drawn.ShouldContain(r => r.Text.Trim() == "India Juliett");
        drawn.ShouldContain(r => r.Text.Trim() == "Kilo Lima");
    }

    /// <summary>A cell in more than one face breaks at its paragraphs too.</summary>
    /// <remarks>
    /// <see cref="SheetTextLayout"/> takes a different route for a cell whose portions are not
    /// all in its own format — the run-aware overload of the layouter rather than the
    /// single-face one — so the rich path needs its own row. Row 6's first paragraph is bold.
    /// </remarks>
    [Fact]
    public void ARichCellBreaksAtItsParagraphs()
    {
        List<DrawnGlyphRun> drawn = Drawn("sheet-cell-hard-break.fods");

        DrawnGlyphRun quebec = drawn.Single(r => r.Text == "Quebec");
        DrawnGlyphRun romeo = drawn.Single(r => r.Text == "Romeo");
        DrawnGlyphRun sierra = drawn.Single(r => r.Text == "Sierra");

        romeo.Origin.X.ShouldBe(quebec.Origin.X);
        (romeo.Origin.Y - quebec.Origin.Y).Points.ShouldBe(11.197, 0.05);
        (sierra.Origin.Y - romeo.Origin.Y).Points.ShouldBe(11.197, 0.05);
    }

    /// <summary>
    /// The break character is not drawn, so it cannot reach the PDF's text layer.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's break portion is "zero width, and no glyph", and a line whose shaped range still
    /// held its trailing <c>'\n'</c> both measured that character's advance into a centred line's
    /// width and put a U+000A into the text a reader can select.
    /// </para>
    /// <para>
    /// Row 2 is the stated exception and the reason this is not simply asserted over the page:
    /// its cell does not wrap, so it never reaches the line breaker at all and is still drawn as
    /// one run with its two breaks in it. That is the unimplemented half described on the class,
    /// and excluding it here is what keeps this test about the half that is implemented.
    /// </para>
    /// </remarks>
    [Fact]
    public void NoBrokenLineHoldsTheBreakItself()
        => Drawn("sheet-cell-hard-break.fods")
            .Where(r => !r.Text.StartsWith("Delta", StringComparison.Ordinal))
            .ShouldNotContain(r => r.Text.Contains('\n') || r.Text.Contains('\r'));

    /// <summary>
    /// The unimplemented half, named as a measurement rather than asserted as correct.
    /// </summary>
    /// <remarks>
    /// LibreOffice draws row 2 on three lines — <c>Delta</c> at 92.74, <c>Echo</c> at 103.93 and
    /// <c>Foxtrot</c> at 115.13 — because ODF makes a multi-paragraph edit cell whatever the
    /// wrap option says. We draw one run holding all three and both breaks, which is what the
    /// BIFF and SpreadsheetML importers ask for and what the whole sheets corpus therefore
    /// wants. This states the gap so that the next change to <see cref="SheetTextLayout"/> has
    /// to decide about it deliberately, and it is the assertion to delete rather than the one to
    /// keep passing.
    /// </remarks>
    [Fact]
    public void ANonWrappingCellStillLosesItsBreaks()
        => Drawn("sheet-cell-hard-break.fods")
            .ShouldContain(r => r.Text == "Delta\nEcho\nFoxtrot");

    /// <summary>
    /// The row heights and the drawn lines are computed from one rule.
    /// </summary>
    /// <remarks>
    /// <see cref="SheetTextLayout.LineCount"/> has always split on the break before wrapping and
    /// the drawing did not, which is how a row reserved three lines and had one put in it. The
    /// two now agree, and this is the assertion that says so directly rather than through a
    /// height.
    /// </remarks>
    [Theory]
    [InlineData("Alpha\nBravo\nCharlie", 3)]
    [InlineData("Golf\n\nHotel", 3)]
    [InlineData("Alpha", 1)]
    public void LineCountSplitsOnTheBreakBeforeItWraps(string text, int expected)
        => SheetTextLayout.LineCount(
            text,
            SheetText.DefaultFace!.Value,
            Core.Units.Length.FromPoints(10),
            Core.Units.Length.FromMillimetres(60)).ShouldBe(expected);
}

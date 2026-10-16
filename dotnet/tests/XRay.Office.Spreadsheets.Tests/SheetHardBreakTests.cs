using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

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
/// <strong>Row 2 is the case the two families answer differently, and this fixture is ODF, so it
/// is asserted as three lines.</strong> It is the same three strings in a cell that does not
/// wrap. BIFF and SpreadsheetML fold them onto one paragraph, by putting the EditEngine into
/// single-line mode for a cell whose format does not wrap
/// (<c>XclImpStringHelper::SetToDocument</c>, <c>xihelper.cxx</c>:246-256;
/// <c>SheetDataBuffer::setStringCell</c>, <c>sheetdatabuffer.cxx</c>:120-135, which every string
/// holding U+000A reaches because <c>RichString::extractPlainString</c> refuses it at
/// <c>richstring.cxx</c>:375) — and under <c>EEControlBits::SINGLELINE</c>
/// <c>ImpEditEngine::ImpInsertText</c> does not look for a separator at all
/// (<c>editeng/source/editeng/impedit2.cxx</c>:2876-2877). Calc's ODF filter never calls
/// <c>SetSingleLine</c>, so the break makes a paragraph whatever the wrap option says. The rule
/// is <see cref="SheetLayout.CellBreaksStartLines"/>; 26.2.4.2 draws <c>Delta</c> at 92.74,
/// <c>Echo</c> at 103.93 and <c>Foxtrot</c> at 115.13, which is the same three figures 24.2.7.2
/// gives.
/// </para>
/// </remarks>
public sealed class SheetHardBreakTests
{
    private static List<DrawnGlyphRun> Drawn(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
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
    /// Row 2 is included rather than excluded now that it breaks: no cell on this page keeps a
    /// break character, whether the line it ends was found by the wrap or by the paragraph.
    /// </para>
    /// </remarks>
    [Fact]
    public void NoBrokenLineHoldsTheBreakItself()
        => Drawn("sheet-cell-hard-break.fods")
            .ShouldNotContain(r => r.Text.Contains('\n') || r.Text.Contains('\r'));

    /// <summary>
    /// A cell that does not wrap still breaks at its own paragraphs, in an ODF sheet.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The half this fixture used to record as unimplemented. Row 2 holds the same three strings
    /// as row 1 in a cell whose format states no wrap, and 26.2.4.2 draws them on three lines at
    /// 92.74, 103.93 and 115.13 — one pitch apart, left-aligned together, exactly as row 1 —
    /// because Calc's ODF filter never puts the EditEngine into single-line mode. See the class
    /// remarks and <see cref="SheetLayout.CellBreaksStartLines"/>.
    /// </para>
    /// <para>
    /// It is the <em>importer</em> being asserted and not the cell, so this is a statement about
    /// a <c>.fods</c> and says nothing about the same three strings in a <c>.xls</c> or an
    /// <c>.xlsx</c>, where one line is right and is what the rest of the sheets corpus wants.
    /// </para>
    /// </remarks>
    [Fact]
    public void ANonWrappingOdfCellBreaksAtItsParagraphs()
    {
        List<DrawnGlyphRun> drawn = Drawn("sheet-cell-hard-break.fods");

        DrawnGlyphRun delta = drawn.Single(r => r.Text == "Delta");
        DrawnGlyphRun echo = drawn.Single(r => r.Text == "Echo");
        DrawnGlyphRun foxtrot = drawn.Single(r => r.Text == "Foxtrot");

        echo.Origin.X.ShouldBe(delta.Origin.X);
        foxtrot.Origin.X.ShouldBe(delta.Origin.X);
        (echo.Origin.Y - delta.Origin.Y).Points.ShouldBe(11.197, 0.05);
        (foxtrot.Origin.Y - echo.Origin.Y).Points.ShouldBe(11.197, 0.05);
    }

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

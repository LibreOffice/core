using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A spreadsheet cell's <c>text:p</c> is taken exactly as written; a word-processing one is
/// collapsed.
/// </summary>
/// <remarks>
/// <para>
/// ODF's text content model is shared by all three applications and its <em>importer</em> is not.
/// A Calc cell's paragraphs are read by Calc's own filter rather than by <c>xmloff</c>'s text
/// import, and <c>ScXMLCellTextParaContext::characters</c> appends the characters it is handed
/// with no normalisation at all (<c>sc/source/filter/xml/celltextparacontext.cxx</c>:36-39). So a
/// cell keeps its leading, trailing and repeated spaces — and, the half that moves a page, a raw
/// <c>U+000A</c> inside a <c>text:p</c> is a paragraph break rather than white space:
/// <c>ScStringUtil::isMultiline</c> is a search for <c>\n</c> or <c>\r</c>
/// (<c>sc/source/core/tool/stringutil.cxx</c>:426-429) and it is what turns the string into an
/// <c>EditTextObject</c> (<c>sc/source/filter/xml/xmlcelli.cxx</c>:625-629), whose paragraphs the
/// EditEngine then splits at each separator.
/// </para>
/// <para>
/// It is not a hypothetical spelling. LibreOffice's own conversion to <c>.ods</c> writes a
/// multi-line cell exactly this way, with no <c>text:line-break</c> anywhere in the file:
/// <strong>659 such paragraphs in 16 of the 307 converted <c>.ods</c></strong>, 361 of them in
/// <c>CIS_Debian_Linux_8_Benchmark_v1.0.0.ods</c>, which drew 38 941 of the reference's 76 210
/// alphanumeric characters because every one of those cells lost every line after the first.
/// </para>
/// <para>
/// Every figure below is LibreOffice 26.2.4.2's own, read out of its PDF of
/// <c>sheet-cell-verbatim-text.fods</c> with PyMuPDF; the fixture's comment lists them row by
/// row and <c>dotnet/probes/ods-page-r77/results.md</c> §1 has the measurement.
/// </para>
/// </remarks>
public sealed class SheetVerbatimCellTextTests
{
    private static List<DrawnGlyphRun> Drawn(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(name));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);
        return sink.Pages[0].Runs;
    }

    private static string CellText(string name, int row, int column)
    {
        using IDocument document = OfficeDocument.Open(Corpus.Require(name));
        ContentTable table = document.Content.Children
            .OfType<ContentSection>().First(s => s.Kind == SectionKind.Sheet)
            .Children.OfType<ContentTable>().First();

        return table.Children.OfType<ContentTableRow>()
                    .SelectMany(r => r.Children.OfType<ContentTableCell>())
                    .Single(c => c.Row == row && c.Column == column)
                    .GetText().TrimEnd('\n');
    }

    /// <summary>A raw newline inside a cell's paragraph is a break, not a space.</summary>
    /// <remarks>
    /// The rule the whole change is about, asserted on the extracted text so that it is stated
    /// once at the seat rather than only through the drawing.
    /// </remarks>
    [Theory]
    [InlineData(4, "ABC\nDEF")]
    [InlineData(5, "ABC\r\nDEF")]
    [InlineData(6, "ABC\n  DEF")]
    public void ARawNewlineInACellIsKept(int row, string expected)
        => CellText("sheet-cell-verbatim-text.fods", row, 1).ShouldBe(expected);

    /// <summary>Leading, trailing and repeated spaces are kept too.</summary>
    /// <remarks>
    /// The same rule seen from its other side, and the reason it is stated as "no normalisation"
    /// rather than as "newlines are special": the filter that collapses runs of white space and
    /// trims a paragraph's ends is <c>xmloff</c>'s, and a cell never goes through it.
    /// </remarks>
    [Theory]
    [InlineData(1, "  ABC")]
    [InlineData(2, "ABC  ")]
    [InlineData(3, "A   B")]
    public void ACellKeepsTheWhiteSpaceItStates(int row, string expected)
        => CellText("sheet-cell-verbatim-text.fods", row, 1).ShouldBe(expected);

    /// <summary>
    /// The control: a word-processing table cell's paragraph is still collapsed.
    /// </summary>
    /// <remarks>
    /// <see cref="XRay.Office.OpenDocument.OdfContentReader.CellTextIsVerbatim"/> is off for every
    /// reader but the spreadsheet one, so an ODT — where <c>xmloff</c>'s text import really is
    /// what reads the cell — keeps ODF's own white-space rule. Without this a change that simply
    /// stopped collapsing would pass everything above.
    /// </remarks>
    /// <remarks>
    /// The fixture's one cell states <c>"  Alpha\nBravo   Charlie  "</c> across two source lines.
    /// 26.2.4.2's own text export of it is <c>"Alpha Bravo Charlie "</c> — the trailing pair
    /// collapsed to one space rather than dropped — and this asserts that string exactly.
    /// </remarks>
    [Fact]
    public void AWordProcessingTableCellStillCollapses()
    {
        using IDocument document = OfficeDocument.Open(
            Corpus.Require("table-cell-collapse.fodt"));

        document.Content.GetText().TrimEnd('\n').ShouldBe("Alpha Bravo Charlie ");
    }

    /// <summary>
    /// Two spaces the file states are two spaces on the page, not one.
    /// </summary>
    /// <remarks>
    /// 26.2.4.2 draws <c>'  ABC'</c> 26.10 pt wide against <c>'ABC'</c>'s 20.55, and
    /// <c>'ABC  '</c> at the same 26.10 — so the trailing pair is measured as well as the
    /// leading one. Asserted as the two being equal and both wider than the plain row, which is
    /// the shape of the claim and does not pin our own advance widths.
    /// </remarks>
    [Fact]
    public void TheStatedSpacesReachThePage()
    {
        List<DrawnGlyphRun> drawn = Drawn("sheet-cell-verbatim-text.fods");

        // The topmost of the four `ABC` runs is row 0's; the other three start a two-line cell.
        DrawnGlyphRun plain = drawn.Where(r => r.Text == "ABC")
                                   .MinBy(r => r.Origin.Y.Points)!;
        DrawnGlyphRun lead = drawn.Single(r => r.Text == "  ABC");
        DrawnGlyphRun trail = drawn.Single(r => r.Text == "ABC  ");

        lead.Width.Points.ShouldBeGreaterThan(plain.Width.Points + 4);
        trail.Width.Points.ShouldBe(lead.Width.Points, 0.01);
    }

    /// <summary>
    /// A cell that does not wrap draws its raw newline as a line, and CRLF as one break.
    /// </summary>
    /// <remarks>
    /// The fixture's column B states <c>fo:wrap-option="no-wrap"</c>, so nothing on it can be
    /// broken for width and every line is a paragraph. 26.2.4.2 puts the <c>newline</c> row's two
    /// lines at 171.12 and 182.31 and the <c>crlf</c> row's at 199.46 and 210.66 — one pitch
    /// apart in both, so <c>\r\n</c> is one break rather than two. The fixture writes that pair
    /// as <c>&amp;#13;&amp;#10;</c>, because an XML parser normalises a literal CRLF in content to
    /// a bare LF before any importer sees it.
    /// </remarks>
    [Fact]
    public void ANewlineDrawsALineInACellThatDoesNotWrap()
    {
        List<DrawnGlyphRun> drawn = Drawn("sheet-cell-verbatim-text.fods");

        List<DrawnGlyphRun> abc = [.. drawn.Where(r => r.Text == "ABC")
                                           .OrderBy(r => r.Origin.Y.Points)];
        List<DrawnGlyphRun> def = [.. drawn.Where(r => r.Text == "DEF")
                                           .OrderBy(r => r.Origin.Y.Points)];

        // plain, newline, crlf, nlindent
        abc.Count.ShouldBe(4);

        // Only the two two-line rows produce a `DEF`; `nlindent`'s keeps its indent.
        def.Count.ShouldBe(2);

        for (int at = 0; at < 2; at++)
        {
            (def[at].Origin.Y - abc[at + 1].Origin.Y).Points.ShouldBe(11.197, 0.05);
            def[at].Origin.X.ShouldBe(abc[at + 1].Origin.X);
        }

        drawn.ShouldContain(r => r.Text == "  DEF");
    }

    /// <summary>
    /// The paragraphs of a non-wrapping cell are drawn a line apart, and the row is as tall as
    /// all of them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>sheet-cell-break-height.fods</c> is four rows, the second holding
    /// <c>Alpha\nBravo\nCharlie</c> in a cell that does not wrap and a row asking for an automatic
    /// height. 26.2.4.2 draws the three at 70.54, 81.74 and 92.94 and starts row 3 at 105.15 — it
    /// gave the row all three lines, which is <strong>34.61 pt</strong> below <c>Alpha</c>.
    /// </para>
    /// <para>
    /// <strong>Both halves are now asserted, and they had to land together.</strong> The height is
    /// <c>pEngine-&gt;GetTextHeight()</c> over as many paragraphs as the importer made
    /// (<c>ScColumn::GetNeededSize</c>, <c>sc/source/core/data/column2.cxx</c>:571-577), and Calc's
    /// ODF filter makes one per hard break where the two Excel filters make one in all —
    /// see <see cref="SheetOptimalRowHeights"/>'s <c>StandingEditLine</c>. On its own that moved
    /// <c>Capability_List_9-14-2022_Dallas_Combined-Aircraft_Manuf_unsorted.ods</c> — fifteen
    /// two-paragraph cells, all past row 200 — from the reference's 147 pages to 150, because an
    /// ODF sheet recalculates its automatic row heights for its <em>first 200 rows only</em> and
    /// those rows keep the height the writer stored. That rule is
    /// <c>OdsPrintSetup.RecalculatedRowLimit</c> and
    /// <c>dotnet/tests/corpus/features/sheet-row-height-limit.fods</c> is its own fixture; this
    /// file is four rows, so it is on the recalculating side of the boundary.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheParagraphsAreDrawnALineApartAndTheRowIsAsTallAsAllOfThem()
    {
        List<DrawnGlyphRun> drawn = Drawn("sheet-cell-break-height.fods");

        DrawnGlyphRun alpha = drawn.Single(r => r.Text == "Alpha");
        DrawnGlyphRun bravo = drawn.Single(r => r.Text == "Bravo");
        DrawnGlyphRun charlie = drawn.Single(r => r.Text == "Charlie");

        bravo.Origin.X.ShouldBe(alpha.Origin.X);
        charlie.Origin.X.ShouldBe(alpha.Origin.X);
        (bravo.Origin.Y - alpha.Origin.Y).Points.ShouldBe(11.197, 0.05);
        (charlie.Origin.Y - bravo.Origin.Y).Points.ShouldBe(11.197, 0.05);

        // 26.2.4.2's own figure: `three` sits 34.61 pt below `Alpha`, three lines and a margin.
        DrawnGlyphRun three = drawn.Single(r => r.Text == "three");
        (three.Origin.Y - alpha.Origin.Y).Points.ShouldBe(34.61, 0.35);
    }
}

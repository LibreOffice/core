using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Every line of a header or footer prints, at the size its own codes state, against the edge the
/// margin fixes.
/// </summary>
/// <remarks>
/// <para>
/// The band was drawn in four wrong ways at once and each is a separate case below. Only the
/// first line of each part was drawn; the <c>&amp;n</c> size code was parsed and discarded; the
/// three parts were each centred in the whole band rather than in the band's own text height; and
/// the text was centred where Calc anchors it.
/// </para>
/// <para>
/// The anchoring is what <c>ATTR_PAGE_DYNAMIC</c> decides. <c>ScPrintFunc::PrintHF</c> gives the
/// EditEngine a paper height of <c>nHeight - nDistance</c> and shifts each of the three areas
/// down by half of what it does not fill
/// (<c>sc/source/ui/view/printfun.cxx:1876-1912</c>), and <c>UpdateHFHeight</c> has already set
/// <c>nHeight</c> to the tallest of the three areas plus the distance (<c>:789-848</c>). So the
/// band is exactly as tall as its tallest part, that part fills it, and the shorter ones are
/// centred against it. <c>XclImpPageSettings::Finalize</c> marks every Excel band dynamic
/// (<c>sc/source/filter/excel/xipage.cxx:316-331</c>).
/// </para>
/// <para>
/// <c>sheet-outline-collapse.xlsx</c> carries the fixture: A4 portrait, 0.3 in header and footer
/// margins, a two-line 14 pt centred header, and a footer whose left part is two 8 pt lines and
/// whose right part is one line stating no size at all. Every expectation is read off LibreOffice
/// 24.2.7.2's own PDF of it with <c>pdftotext -bbox</c>.
/// </para>
/// <para>
/// Verified by reintroducing each defect separately — see the class-level table in
/// <c>dotnet/probes/sheets-r31/README.md</c> for which cases each one fails.
/// </para>
/// </remarks>
public sealed class SheetBandLinesTests
{
    private static IReadOnlyList<DrawnGlyphRun> Runs()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-outline-collapse.xlsx"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);
        return [.. sink.Pages[0].Runs];
    }

    private static DrawnGlyphRun Run(string text) => Runs().First(
        run => run.Text.StartsWith(text, StringComparison.Ordinal));

    /// <summary>
    /// Both header lines are drawn, and the second sits one line below the first.
    /// </summary>
    /// <remarks>
    /// LibreOffice puts the two glyph boxes at 21.581 and 37.231 pt from the page top, a pitch of
    /// 15.65 pt — which is 14 pt through Liberation Sans's <c>hhea</c> ascent and descent,
    /// 2288/2048. The origins below are baselines, so they sit one ascent lower and the *pitch*
    /// is the figure to compare.
    /// </remarks>
    [Fact]
    public void BothHeaderLinesAreDrawn()
    {
        DrawnGlyphRun one = Run("Head one");
        DrawnGlyphRun two = Run("Head two");

        (two.Origin.Y - one.Origin.Y).Points.ShouldBe(15.65, 0.1);
    }

    /// <summary>
    /// The header sits against the top of its band, which is the header margin.
    /// </summary>
    /// <remarks>
    /// 0.3 in is 21.6 pt, and LibreOffice's first header glyph box starts at 21.581. The baseline
    /// is one ascent below that: Liberation Sans's <c>hhea</c> ascent is 1854/2048, so 14 pt gives
    /// 12.67 pt and a baseline at 34.3.
    /// </remarks>
    [Fact]
    public void TheHeaderIsAnchoredToTheHeaderMargin()
    {
        Run("Head one").Origin.Y.Points.ShouldBe(34.27, 0.2);
    }

    /// <summary>
    /// A stated <c>&amp;14</c> is the size the line is drawn at, not the sheet's ten point.
    /// </summary>
    [Fact]
    public void TheStatedSizeIsTheSizeDrawn()
    {
        Run("Head one").Run.FontSize.Points.ShouldBe(14, 0.01);
    }

    /// <summary>
    /// The footer sits against the bottom of its band, which is the footer margin.
    /// </summary>
    /// <remarks>
    /// The page is 841.89 pt tall and the footer margin 21.6, so the band's bottom edge is at
    /// 820.29. The left part is two 8 pt lines — 8.94 pt each — and LibreOffice puts its last
    /// glyph box at 811.311–820.239, hard against that edge.
    /// </remarks>
    [Fact]
    public void TheFooterIsAnchoredToTheFooterMargin()
    {
        DrawnGlyphRun first = Run("Foot left");
        DrawnGlyphRun last = Runs().Last(run => run.Text.StartsWith("Foot left", StringComparison.Ordinal));

        (last.Origin.Y - first.Origin.Y).Points.ShouldBe(8.94, 0.1);
        last.Origin.Y.Points.ShouldBe(818.55, 0.3);
    }

    /// <summary>
    /// A part shorter than the band is centred in it, rather than anchored like the band.
    /// </summary>
    /// <remarks>
    /// The footer's right part is one line stating no size, so it takes the sheet's ten point and
    /// is 11.17 pt tall against the left part's 17.87. LibreOffice draws its glyph box at
    /// 805.701–816.861, which is 3.35 pt below the band's top edge at 802.43 — exactly half the
    /// 6.7 pt difference. Anchoring each part independently instead puts it a point and a fifth
    /// lower, which is the reading this case exists to rule out.
    /// </remarks>
    [Fact]
    public void AShorterPartIsCentredAgainstTheTallestOne()
    {
        DrawnGlyphRun right = Run("Foot right");

        right.Run.FontSize.Points.ShouldBe(10, 0.01);
        right.Origin.Y.Points.ShouldBe(814.75, 0.3);
    }

    /// <summary>
    /// A section switch resets the font, so the right part is not drawn at the left part's size.
    /// </summary>
    /// <remarks>
    /// <c>&amp;L&amp;8…&amp;RFoot right</c>: the eight belongs to the left part alone.
    /// <c>XclImpHFConverter::SetNewPortion</c> calls <c>ResetFontData</c>, which restores the
    /// workbook's first font (<c>sc/source/filter/excel/xihelper.cxx:534-548</c>), and the OOXML
    /// parser does the same with <c>maFontModel = getDefaultFontModel()</c>
    /// (<c>sc/source/filter/oox/pagesettings.cxx:868-876</c>). Carrying the size across draws
    /// this line at eight point and 8.4 pt too far right.
    /// </remarks>
    [Fact]
    public void ASectionSwitchResetsTheSize()
    {
        SheetHeaderFooter band = SheetHeaderFooter.ParseCodes("&L&8Left&RRight");

        band.Left.Segments[0].Size.ShouldBe(Length.FromPoints(8));
        band.Right.Segments[0].Size.ShouldBeNull();
    }

    /// <summary>A line break inside one part makes two lines of it.</summary>
    [Fact]
    public void ParsingSplitsAPartIntoLines()
    {
        SheetHeaderFooter band = SheetHeaderFooter.ParseCodes("&C&12one\ntwo\nthree");
        IReadOnlyList<IReadOnlyList<SheetHeaderPiece>> lines =
            band.Centre.Lines(new SheetHeaderContext());

        lines.Count.ShouldBe(3);
        lines[0][0].Text.ShouldBe("one");
        lines[2][0].Text.ShouldBe("three");
        lines[2][0].Size.ShouldBe(Length.FromPoints(12));
    }

    /// <summary>
    /// A blank line between two that draw keeps its height; a trailing one takes no room.
    /// </summary>
    /// <remarks>
    /// Both shapes occur in one corpus footer —
    /// <c>&amp;RPage &amp;P of &amp;N\n\nMay contain trade secrets…</c> — and treating the blank
    /// line as nothing would lift everything after it by a line.
    /// </remarks>
    [Fact]
    public void ABlankLineInTheMiddleKeepsItsPlace()
    {
        SheetHeaderFooter band = SheetHeaderFooter.ParseCodes("&Rone\n\ntwo\n");
        IReadOnlyList<IReadOnlyList<SheetHeaderPiece>> lines =
            band.Right.Lines(new SheetHeaderContext());

        lines.Count.ShouldBe(3);
        lines[1].ShouldBeEmpty();
        lines[2][0].Text.ShouldBe("two");
    }
}

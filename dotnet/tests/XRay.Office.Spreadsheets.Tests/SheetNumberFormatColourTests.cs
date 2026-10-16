using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A number format's colour clause: <c>[Red]</c> in a subformat colours the cells that subformat
/// formats.
/// </summary>
/// <remarks>
/// <para>
/// It was discarded. <c>NumberFormatSection</c> dropped every bracketed body it did not recognise
/// as a currency symbol, an elapsed unit or a calendar directive, under the comment that a colour
/// "changes appearance rather than the text this extracts" — true of the text, and false of the
/// ink. Nothing in the tree applied a format colour in any format.
/// </para>
/// <para>
/// <strong>The ten names and their values are LibreOffice's own, in its own order.</strong>
/// <c>ImpSvNumberformatScan::StandardColor</c> is <c>{ BLACK, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN,
/// LIGHTRED, LIGHTMAGENTA, BROWN, GRAY, YELLOW, WHITE }</c> against the keywords <c>BLACK BLUE
/// GREEN CYAN RED MAGENTA BROWN GREY YELLOW WHITE</c>
/// (<c>svl/source/numbers/zforscan.cxx</c>:98-111, 145-147). So <c>[Blue]</c> is <b>#0000FF</b>
/// and not <c>#000080</c> — which is why the fixture states a blue section as well as a red one:
/// a red-only fixture cannot tell the two tables apart.
/// </para>
/// <para>
/// <strong>Which subformat a cell takes is decided by its value</strong>, so the colour is
/// resolved per cell at the draw site and not per style on <see cref="SheetCellFormat"/>. The
/// fixture's fourth cell holds the same <c>-2.5</c> as its second under a <em>colourless</em>
/// format, so an implementation that coloured by value rather than by the selected section fails
/// on it.
/// </para>
/// <para>
/// Confirmed against 26.2.4.2's own rendering of this fixture, span for span:
/// <c>1.5</c> #000000, <c>-2.5</c> #ff0000, <c>0.0</c> #0000ff, <c>-2.50</c> #000000.
/// </para>
/// <para>
/// Reach, measured rather than censused: 240 corpus <c>xlsx</c>, of which <b>14 state a colour
/// clause</b>, 12 use one on a numeric cell (12 479 cells), and <b>8 hold a value that actually
/// selects a coloured subformat — 137 cells</b>, which is the figure to quote. Sweeping the whole
/// 307-document sheets track twice under <c>SOURCE_DATE_EPOCH</c> moves <b>7 renderings</b> and
/// leaves 300 byte-identical. The eighth selecting document,
/// <c>certification-type-certificates-…-MAdB-Light-Prop</c>, does not move and should not: its
/// red comes from explicit cell font colours, and this tree already matched the reference there
/// exactly — 4838 red spans, 686 black, 343 pages on both sides.
/// </para>
/// <para>
/// <strong><c>[COLOR n]</c> is not resolved</strong>, and that is deliberate: LibreOffice takes it
/// from the palette at <c>SvtPathOptions().GetPalettePath()</c>, i.e. the running installation's
/// own <c>share/palette/standard.soc</c>. Index 9 there is <c>#dddddd</c> "Light Gray 4", which is
/// exactly what 26.2.4.2 paints for <c>[Color10]</c>. Corpus reach of the gap is <b>18 cells in
/// one document</b>. See <c>probes/numfmtcolour-r139/results.md</c>, which also records that the
/// ODF twin is still wrong for a larger reason — <c>style:map</c> is not followed for number
/// styles at all.
/// </para>
/// </remarks>
public sealed class SheetNumberFormatColourTests
{
    private const string Fixture = "sheet-numfmt-colour.xlsx";

    private static List<DrawnGlyphRun> Runs()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);
        return sink.Pages[0].Runs;
    }

    private static Colour ColourOf(string text)
        => ((SolidPaint)Runs().Single(r => r.Text.Trim() == text).Paint).Colour;

    [Theory]
    // The positive section names no colour, so the cell's own is kept.
    [InlineData("1.5", 0x000000)]
    // `[Red]` is COL_LIGHTRED.
    [InlineData("-2.5", 0xFF0000)]
    // `[Blue]` is COL_LIGHTBLUE — #0000FF, not #000080.
    [InlineData("0.0", 0x0000FF)]
    // The same value as the red cell, under a format stating no colour at all: the colour belongs
    // to the selected subformat and not to the sign of the number.
    [InlineData("-2.50", 0x000000)]
    public void ACellTakesTheColourOfTheSubformatItsValueSelects(string text, int expected)
        => ColourOf(text).ShouldBe(Colour.FromRgb((uint)expected));

    [Theory]
    [InlineData("[BLACK]0", 0x000000)]
    [InlineData("[BLUE]0", 0x0000FF)]
    [InlineData("[GREEN]0", 0x00FF00)]
    [InlineData("[CYAN]0", 0x00FFFF)]
    [InlineData("[RED]0", 0xFF0000)]
    [InlineData("[MAGENTA]0", 0xFF00FF)]
    [InlineData("[BROWN]0", 0x808000)]
    [InlineData("[GREY]0", 0x808080)]
    [InlineData("[YELLOW]0", 0xFFFF00)]
    [InlineData("[WHITE]0", 0xFFFFFF)]
    // `GetColor` upper-cases the word before matching, so every spelling is one colour.
    [InlineData("[Red]0", 0xFF0000)]
    [InlineData("[red]0", 0xFF0000)]
    // Excel writes GRAY where LibreOffice keys on GREY; they name one colour.
    [InlineData("[GRAY]0", 0x808080)]
    public void EachOfLibreOfficesTenColourKeywordsIsRead(string code, int expected)
        => NumberFormatCode.Parse(code).Sections[0].Colour
            .ShouldBe(Colour.FromRgb((uint)expected));

    [Theory]
    // A palette index needs the running installation's own standard.soc, so it stays unresolved
    // rather than being guessed at.
    [InlineData("[COLOR10]0")]
    [InlineData("[Color 10]0")]
    // Not colours at all.
    [InlineData("[ENG]0")]
    [InlineData("[$-409]0")]
    [InlineData("0.00")]
    public void AnythingElseInBracketsLeavesTheSectionColourless(string code)
        => NumberFormatCode.Parse(code).Sections[0].Colour.ShouldBeNull();

    [Fact]
    public void TheColourBelongsToTheSubformatRatherThanToTheFormat()
    {
        NumberFormatCode code = NumberFormatCode.Parse("0.0;[Red]-0.0;[Blue]0.0");

        code.SelectFor(1.5).Colour.ShouldBeNull("the positive subformat names none");
        code.SelectFor(-2.5).Colour.ShouldBe(Colour.FromRgb(0xFF0000));
        code.SelectFor(0).Colour.ShouldBe(Colour.FromRgb(0x0000FF));
    }
}

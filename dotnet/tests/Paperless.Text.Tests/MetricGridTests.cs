using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// Tests the device grid a document laid out against a printer measures its fonts on.
/// </summary>
/// <remarks>
/// <para>
/// The numbers here are Liberation Sans's own — 2048 units to the em, a 1854 <c>hhea</c> ascender, a
/// −434 descender and a 67 line gap — stated rather than read from the installed file, so the arithmetic
/// is tested without the test depending on a font being present.
/// </para>
/// <para>
/// The expectations are measurements rather than derivations. LibreOffice's PDF of
/// <c>words/batch-018/doc/A_320.doc</c>, whose <c>Dop</c> sets <c>fUsePrinterMetrics</c>, puts
/// consecutive lines of an 11 pt paragraph 13.00 pt apart — 260 twips — and of a 12 pt paragraph
/// 13.95 pt apart, where the design units alone give 12.65 and 13.80. Its PDF of
/// <c>words/batch-020/doc/150_5300_13_chg10.doc</c> gives the Liberation Serif figures the third case
/// checks: 10.60, 11.30 and 11.55 pt at 9, 9.5 and 10 pt.
/// </para>
/// </remarks>
public class MetricGridTests
{
    private static LineMetrics LiberationSans(MetricGrid? grid = null, bool leadingAbove = false)
        => new(1854, 434, 67, LineMetricSource.HorizontalHeader, 2048, grid, leadingAbove);

    private static LineMetrics LiberationSerif(MetricGrid? grid = null)
        => new(1825, 443, 87, LineMetricSource.HorizontalHeader, 2048, grid);

    // Twips rather than points, because that is the unit the layout engine snaps every line height to
    // before anything else uses it — and comparing points would be comparing the EMU remainder as well.
    [Theory]
    [InlineData(11, 253)]
    [InlineData(12, 276)]
    public void WithoutAGridAFaceScalesExactly(double points, long expected)
        => LiberationSans().ScaledLineHeight(Length.FromPoints(points)).Twips.ShouldBe(expected);

    [Theory]
    [InlineData(11, 260)]
    [InlineData(12, 279)]
    public void OnAPrinterGridTheSameFaceRoundsUpToLibreOfficesAnswer(double points, long expected)
        => LiberationSans(MetricGrid.Printer).ScaledLineHeight(Length.FromPoints(points))
            .Twips.ShouldBe(expected);

    [Theory]
    [InlineData(9, 212)]
    [InlineData(9.5, 226)]
    [InlineData(10, 231)]
    public void TheGridIsNotAScaleFactor(double points, long expected)
    {
        // Three sizes of one face, and no single multiplier produces all three from the design units:
        // the rounding happens twice, at the em size and again at each metric, so the error the grid
        // introduces is a sawtooth rather than a percentage. A fix that scaled instead would match one
        // of these and miss the other two.
        LineMetrics gridded = LiberationSerif(MetricGrid.Printer);
        gridded.ScaledLineHeight(Length.FromPoints(points)).Twips.ShouldBe(expected);
    }

    [Fact]
    public void TheLeadingSitsAboveTheTextRatherThanBelowIt()
    {
        // SwFntObj::GetFontAscent adds the external leading to the ascent everywhere but macOS, so a
        // gridded ascent exceeds the bare ascent by exactly the gap and the descent is unchanged.
        // Only a Writer document reaches the grid at all — it is what `fUsePrinterMetrics` asks for —
        // so this case is stated the way Writer asks for it.
        Length em = Length.FromPoints(11);
        LineMetrics gridded = LiberationSans(MetricGrid.Printer, leadingAbove: true);

        Length ascent = gridded.ScaledAscent(em);
        Length descent = gridded.ScaledDescent(em);

        (ascent + descent).ShouldBe(gridded.ScaledLineHeight(em));
        ascent.Twips.ShouldBe(212);
        descent.Twips.ShouldBe(48);
    }

    [Fact]
    public void TheLeadingSitsAboveTheTextWithoutAGridToo()
    {
        // The gridless path is the usual one — only a document laid out against a printer passes a
        // grid — and it used to charge the line gap to neither the ascent nor the descent. The gap was
        // still inside the line height, so the pitch *within* a paragraph was right and only the first
        // line of each page was wrong, which is what let it survive: it cancels everywhere except
        // against the top margin.
        //
        // Read out of LibreOffice's own PDF content stream: Liberation Sans at 11 pt inside a 72 pt top
        // margin puts Writer's first baseline at 82.3008 pt, so the ascent is 206 twips and not 199.
        // 1854 + 67 over 2048 at 11 pt is 206.35 twips; 1854 alone is 199.15.
        Length em = Length.FromPoints(11);
        LineMetrics writer = LiberationSans(leadingAbove: true);

        Length ascent = writer.ScaledAscent(em);
        Length descent = writer.ScaledDescent(em);

        ascent.Twips.ShouldBe(206);
        descent.Twips.ShouldBe(47);
        (ascent + descent).ShouldBe(writer.ScaledLineHeight(em));
    }

    [Fact]
    public void AnEngineThatDoesNotAddTheLeadingLeavesTheLineShortOfItsHeight()
    {
        // The other half, and the reason this is a flag rather than a rule: EditEngine — which is what
        // Impress, Calc and Writer's own drawing objects format through — adds the external leading
        // only when `IsAddExtLeading()`, and that is false unless something turns it on
        // (editeng/source/editeng/impedit3.cxx:3133-3135, impedit2.cxx:118, svdmodel.cxx:161). Its
        // line box is `nMaxAscent + nMaxDescent` with no gap in it, so ascent + descent is *shorter*
        // than the face's line height by exactly the gap, and that is correct rather than a defect.
        //
        // Measured: LibreOffice Impress puts two 18 pt Liberation Sans baselines in a table cell
        // 20.154 pt apart, which is ascent-plus-descent; the gap would make it 20.698.
        Length em = Length.FromPoints(11);
        LineMetrics editEngine = LiberationSans();

        Length ascent = editEngine.ScaledAscent(em);
        Length descent = editEngine.ScaledDescent(em);

        ascent.Twips.ShouldBe(199);
        descent.Twips.ShouldBe(47);
        (ascent + descent).ShouldBeLessThan(editEngine.ScaledLineHeight(em));
    }

    [Fact]
    public void AFaceStatingNoLineGapIsUnaffectedByWhereTheLeadingSits()
    {
        // Carlito's hhea gap is zero, which is why the placement error was invisible on every OOXML
        // document that resolves its fonts through the theme — and nearly all of this corpus does. A
        // face with no gap must come out identical either way, so this pins that the difference is a
        // *placement* and not an addition.
        Length em = Length.FromPoints(11);
        LineMetrics carlito = new(1950, 550, 0, LineMetricSource.HorizontalHeader, 2048);

        carlito.ScaledAscent(em)
            .ShouldBe((carlito with { LeadingAboveText = true }).ScaledAscent(em));
        (carlito.ScaledAscent(em) + carlito.ScaledDescent(em))
            .ShouldBe(carlito.ScaledLineHeight(em));
    }

    [Fact]
    public void AGridOfNoResolutionMeasuresNothingRatherThanDividingByZero()
    {
        MetricGrid degenerate = new(0);

        degenerate.ToPixels(1854, 2048, Length.FromPoints(11)).ShouldBe(0);
        degenerate.ToLength(100).ShouldBe(Length.Zero);
    }

    // ---------------------------------------------------------------- advance widths
    //
    // Every expectation below is a width LibreOffice itself drew, read out of the content stream of
    // an authored pair that differs in one bit — `dotnet/probes/printer-metric-advance.py`, which
    // writes one body through LibreOffice's DOC export and then patches WW8Dop's fUsePrinterMetrics
    // both ways. The rule they pin is
    //
    //     floor( N . advance . round(size/72 . 300) / upem ) device pixels, then to twips
    //
    // and it is exact on all 96 of the probe's rows. Three alternatives are stated in code below so
    // that adopting any of them fails here rather than being re-proposed:
    //
    //   * scaling exactly, with no device in it at all      — fails ExactScalingIsNotWhatAPrinterMeasures
    //   * rounding *each glyph's* advance to a whole pixel  — fails RoundingEachGlyphIsNotTheRule
    //   * rounding the total instead of truncating it       — fails TheTotalIsTruncatedAndNotRounded
    //
    // The middle one matters most: it is what GenericSalLayout::LayoutText appears to say
    // (vcl/source/gdi/CommonSalLayout.cxx:826-831) and it is not what the binary does, because a
    // mapped device turns subpixel positioning on.

    private const int Upem = 2048;

    // Liberation Serif 'n' 1024, 'i' 569, 'M' 1821; Liberation Sans 'n' 1139, 'M' 1706. Stated, so
    // the test does not depend on a font file being installed.
    [Theory]
    [InlineData(1024, 9.0, 1, 91)]      // Serif 'n': 19.0 px exactly, and still floored
    [InlineData(1024, 9.0, 64, 5837)]
    [InlineData(569, 9.0, 1, 48)]       // Serif 'i': 10.5576 px -> 10
    [InlineData(569, 9.0, 16, 806)]
    [InlineData(569, 9.0, 64, 3240)]
    [InlineData(1821, 9.0, 1, 158)]     // Serif 'M'
    [InlineData(1821, 9.0, 64, 10378)]
    [InlineData(1139, 9.0, 16, 1622)]   // Sans 'n', the row that separates total from per glyph
    [InlineData(1139, 9.0, 64, 6490)]
    [InlineData(1706, 10.0, 1, 163)]    // Sans 'M' at a size whose em rounds up by a third of a pixel
    [InlineData(1024, 12.0, 64, 7680)]  // 12 pt sets 50 px exactly, so nothing moves
    public void APrinterMeasuresAnAdvanceOnItsPixelGrid(int advance, double points, int count, long twips)
        => MetricGrid.Printer
            .ToAdvance((long)advance * count, Upem, Length.FromPoints(points))
            .Twips.ShouldBe(twips);

    [Fact]
    public void ExactScalingIsNotWhatAPrinterMeasures()
    {
        // 64 Liberation Serif 'n' at 9 pt: the design units alone give 288 pt, and the device draws
        // 291.85. The em is 37.5 px and the device can only set 38, so every advance is 1.33% wider.
        Length em = Length.FromPoints(9);
        long exact = (long)Math.Round(1024L * 64 * em.Emu / (double)Upem);

        Length.FromEmu(exact).Twips.ShouldBe(5760);
        MetricGrid.Printer.ToAdvance(1024L * 64, Upem, em).Twips.ShouldBe(5837);
    }

    [Fact]
    public void RoundingEachGlyphIsNotTheRule()
    {
        // Liberation Sans 'n' at 9 pt is 21.1338 px. Rounding each glyph gives 21 px, so sixteen of
        // them measure 336 px = 1613 twips; the device measures the sixteen together and truncates
        // once, 338 px = 1622. Nine twips on one word, and it compounds along a line.
        Length em = Length.FromPoints(9);
        long perGlyph = 16 * (long)Math.Round(1139 * 38.0 / Upem);

        MetricGrid.Printer.ToLength(perGlyph).Twips.ShouldBe(1613);
        MetricGrid.Printer.ToAdvance(1139L * 16, Upem, em).Twips.ShouldBe(1622);
    }

    [Fact]
    public void TheTotalIsTruncatedAndNotRounded()
    {
        // Sixteen Liberation Serif 'i' at 9 pt come to 168.9219 px. Rounded that is 169 px = 811
        // twips; LibreOffice draws 806, which is 168.
        Length em = Length.FromPoints(9);

        MetricGrid.Printer.ToLength((long)Math.Round(569 * 16 * 38.0 / Upem)).Twips.ShouldBe(811);
        MetricGrid.Printer.ToAdvance(569L * 16, Upem, em).Twips.ShouldBe(806);
    }

    [Fact]
    public void AGridOfNoResolutionMeasuresNoAdvanceRatherThanDividingByZero()
    {
        new MetricGrid(0).ToAdvance(1024, Upem, Length.FromPoints(11)).ShouldBe(Length.Zero);
        MetricGrid.Printer.ToAdvance(1024, 0, Length.FromPoints(11)).ShouldBe(Length.Zero);
    }
}

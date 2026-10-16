using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

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
/// The expectations are measurements rather than derivations, and <strong>they were all re-measured
/// on 2026-08-15 against the installed 26.2.4.2 because the stored ones no longer reproduce.</strong>
/// They had been taken when the printer device was 300 dpi; it is 600 here, so every figure that
/// discriminates the two moved. Three independent sources agree and are quoted below rather than one:
/// </para>
/// <list type="bullet">
/// <item><description>
/// <c>probes/printer-metric-advance.py</c>, which varies <c>fUsePrinterMetrics</c> on one authored
/// body and reads both baseline pitches out of the content stream — Liberation Serif at 9/10/11/12 pt
/// gives 10.300/11.550/12.750/13.800 and Liberation Sans 10.350/11.500/12.600/13.800.
/// </description></item>
/// <item><description>
/// the banked reference rendering of <c>words/pagination-001/doc/A_320.doc</c>, whose <c>Dop</c> sets
/// the flag: consecutive lines of an 11 pt Liberation Sans paragraph 12.60 pt apart, 252 twips.
/// </description></item>
/// <item><description>
/// the banked reference rendering of <c>words/table-001/doc/150_5300_13_chg10.doc</c>: Liberation
/// Serif at 9, 9.5 and 10 pt gives 10.30, 10.80 and 11.55 pt.
/// </description></item>
/// </list>
/// <para>
/// The superseded figures — 13.00 and 13.95 pt for Liberation Sans at 11 and 12, and 10.60/11.30/11.55
/// for Liberation Serif — are exactly what a 300 dpi grid produces, so what moved is the headless
/// default printer and not anyone's reading of the rule. Nothing about the document decides it, which
/// is why it has to be re-measured rather than inherited.
/// </para>
/// </remarks>
public class MetricGridTests
{
    private static LineMetrics LiberationSans(MetricGrid? grid = null, bool leadingAbove = false)
        => new(1854, 434, 67, LineMetricSource.HorizontalHeader, 2048, grid, leadingAbove);

    private static LineMetrics LiberationSerif(MetricGrid? grid = null, bool leadingAbove = false)
        => new(1825, 443, 87, LineMetricSource.HorizontalHeader, 2048, grid, leadingAbove);

    // Twips rather than points, because that is the unit the layout engine snaps every line height to
    // before anything else uses it — and comparing points would be comparing the EMU remainder as well.
    [Theory]
    [InlineData(11, 253)]
    [InlineData(12, 276)]
    public void WithoutAGridAFaceScalesExactly(double points, long expected)
        => LiberationSans().ScaledLineHeight(Length.FromPoints(points)).Twips.ShouldBe(expected);

    // 11 pt is the size that discriminates for this face: ungridded it is 253 and the printer draws
    // 252. At 12 pt the two devices agree at 276, which is why 12 is stated in the case above and not
    // here — a size where a broken grid gives the right answer is not a test of the grid.
    [Theory]
    [InlineData(11, 252)]
    [InlineData(9, 207)]
    [InlineData(10, 230)]
    public void OnAPrinterGridTheSameFaceIsLibreOfficesAnswerAndNotTheDesignUnits(
        double points, long expected)
        => LiberationSans(MetricGrid.Printer, leadingAbove: true)
            .ScaledLineHeight(Length.FromPoints(points))
            .Twips.ShouldBe(expected);

    [Theory]
    [InlineData(9, 206)]
    [InlineData(9.5, 216)]
    [InlineData(10, 231)]
    public void TheGridIsNotAScaleFactor(double points, long expected)
    {
        // Three sizes of one face, and no single multiplier produces all three from the design units:
        // the rounding happens twice, at the em size and again at each metric, so the error the grid
        // introduces is a sawtooth rather than a percentage. A fix that scaled instead would match one
        // of these and miss the other two.
        LineMetrics gridded = LiberationSerif(MetricGrid.Printer, leadingAbove: true);
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
        ascent.Twips.ShouldBe(206);
        descent.Twips.ShouldBe(46);
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
    // both ways. **Re-measured 2026-08-15 on 26.2.4.2 and every figure moved**, for the reason the
    // class remark gives: the device is 600 dpi and the stored figures were 300 dpi's.
    //
    // The rule is
    //
    //     floor( N . advance . round(size/72 . 600) / upem ) device pixels, then to twips
    //
    // and unlike the vertical rule it is **not** exact: it reproduces 37 of the probe's 96 rows and
    // the other 59 are out by one or two twips. That is a real open residual and it is recorded as a
    // test below rather than left in a write-up. It is also a hundredfold improvement — at 300 dpi
    // the same rule is out by as much as 137 twips, 6.85 pt on a 64-glyph run.
    //
    // Dropping the truncation, which fits 52 of 96, was considered and not done: it is better on 36
    // rows and *worse* on 17, so the evidence does not choose between them and adopting it would be
    // fitting rather than reading. The floor is what the C++ says.
    //
    // Two alternatives are still stated in code so that adopting either fails here rather than being
    // re-proposed:
    //
    //   * scaling exactly, with no device in it at all      — fails ExactScalingIsNotWhatAPrinterMeasures
    //   * rounding *each glyph's* advance to a whole pixel  — fails RoundingEachGlyphIsNotTheRule
    //
    // The second matters most: it is what GenericSalLayout::LayoutText appears to say
    // (vcl/source/gdi/CommonSalLayout.cxx:826-831) and it is not what the binary does, because a
    // mapped device turns subpixel positioning on.

    private const int Upem = 2048;

    // Liberation Serif 'n' 1024, 'i' 569, 'M' 1821; Liberation Sans 'n' 1139, 'i' 455, 'M' 1706.
    // Stated, so the test does not depend on a font file being installed. Every row here is one the
    // probe measured *and* this rule reproduces exactly; the rows it does not are the subject of
    // TheAdvanceRuleIsNotExactAndTheResidueIsRecordedRatherThanHidden.
    [Theory]
    [InlineData(1024, 9.0, 64, 5760)]    // Serif 'n': 9 pt sets 75 px exactly, so nothing moves
    [InlineData(1024, 10.0, 64, 6374)]   // 10 pt wants 83.33 px and gets 83, so advances shrink
    [InlineData(569, 10.0, 4, 221)]      // Serif 'i' at the same size
    [InlineData(1821, 10.0, 64, 11335)]  // Serif 'M'
    [InlineData(1024, 11.0, 16, 1766)]   // 11 pt wants 91.67 px and gets 92, so advances grow
    [InlineData(1821, 11.0, 64, 12564)]
    [InlineData(1024, 12.0, 64, 7680)]   // 12 pt sets 100 px exactly, so nothing moves
    public void APrinterMeasuresAnAdvanceOnItsPixelGrid(int advance, double points, int count, long twips)
        => MetricGrid.Printer
            .ToAdvance((long)advance * count, Upem, Length.FromPoints(points))
            .Twips.ShouldBe(twips);

    [Theory]
    [InlineData(1139, 9.0, 64, 6406)]    // Sans 'n'
    [InlineData(1139, 10.0, 64, 7090)]
    [InlineData(455, 11.0, 64, 3139)]    // Sans 'i'
    [InlineData(1706, 10.0, 16, 2654)]   // Sans 'M'
    [InlineData(1139, 12.0, 64, 8542)]
    public void TheSameRuleHoldsForTheOtherFace(int advance, double points, int count, long twips)
        => MetricGrid.Printer
            .ToAdvance((long)advance * count, Upem, Length.FromPoints(points))
            .Twips.ShouldBe(twips);

    [Fact]
    public void TheAdvanceRuleIsNotExactAndTheResidueIsRecordedRatherThanHidden()
    {
        // Four rows the probe measured and this rule misses, in both directions, so that a change
        // that fixes them shows up here as a *failure* to update rather than as silence. Sixteen
        // Liberation Serif 'i' at 10 pt: LibreOffice draws 885 twips and the truncation gives 883.
        // Four Liberation Serif 'n' at 11 pt: LibreOffice draws 441 and this gives 442, the other
        // way. Neither is more than a tenth of a point, and at 300 dpi the same rows were out by up
        // to 137 twips.
        MetricGrid.Printer.ToAdvance(569L * 16, Upem, Length.FromPoints(10)).Twips.ShouldBe(883);
        MetricGrid.Printer.ToAdvance(1821L, Upem, Length.FromPoints(10)).Twips.ShouldBe(175);
        MetricGrid.Printer.ToAdvance(1024L * 4, Upem, Length.FromPoints(11)).Twips.ShouldBe(442);
        MetricGrid.Printer.ToAdvance(569L * 64, Upem, Length.FromPoints(11)).Twips.ShouldBe(3924);
    }

    [Fact]
    public void ExactScalingIsNotWhatAPrinterMeasures()
    {
        // 64 Liberation Serif 'n' at 10 pt: the design units alone give 320 pt, and the device draws
        // 318.70. The em is 83.33 px and the device can only set 83, so every advance is 0.4%
        // narrower. 9 and 12 pt are the wrong sizes to ask this at, because 600 dpi sets both of
        // them exactly and the two answers coincide — which is what the stored version of this test,
        // written when the device was 300 dpi, did not have to worry about.
        Length em = Length.FromPoints(10);
        long exact = (long)Math.Round(1024L * 64 * em.Emu / (double)Upem);

        Length.FromEmu(exact).Twips.ShouldBe(6400);
        MetricGrid.Printer.ToAdvance(1024L * 64, Upem, em).Twips.ShouldBe(6374);
    }

    [Fact]
    public void RoundingEachGlyphIsNotTheRule()
    {
        // Liberation Sans 'M' at 11 pt is 76.6406 px. Rounding each glyph gives 77 px, so sixteen of
        // them measure 1232 px = 2957 twips; the device measures the sixteen together, 1226.25 px,
        // and truncates once — 2942, which is what LibreOffice draws. Fifteen twips on one word, and
        // it compounds along a line.
        Length em = Length.FromPoints(11);
        long perGlyph = 16 * (long)Math.Round(1706 * 92.0 / Upem);

        MetricGrid.Printer.ToLength(perGlyph).Twips.ShouldBe(2957);
        MetricGrid.Printer.ToAdvance(1706L * 16, Upem, em).Twips.ShouldBe(2942);
    }

    [Fact]
    public void AGridOfNoResolutionMeasuresNoAdvanceRatherThanDividingByZero()
    {
        new MetricGrid(0).ToAdvance(1024, Upem, Length.FromPoints(11)).ShouldBe(Length.Zero);
        MetricGrid.Printer.ToAdvance(1024, 0, Length.FromPoints(11)).ShouldBe(Length.Zero);
        new MetricGrid(0).PixelEmScale(Length.FromPoints(11)).ShouldBe(1.0);
        MetricGrid.Chart.PixelEmScale(Length.Zero).ShouldBe(1.0);
    }

    /// <summary>
    /// The chart device's em is a whole number of pixels, and the correction is a sawtooth.
    /// </summary>
    /// <remarks>
    /// <para>
    /// At 10 pt a 96 dpi device sets <b>13</b> pixels for 13.333, so every advance comes out 2.5%
    /// narrower than the size the file states; at 11 pt it sets <b>15</b> for 14.667 and they come
    /// out 2.3% <em>wider</em>. A rule that always narrows fails the second half and a rule that
    /// never quantises fails both. At 9, 12 and 18 pt the device sets the size exactly and the
    /// correction is one — which is what makes a chart at those sizes render as it did before.
    /// </para>
    /// <para>
    /// The ratios are the reference's own, at fourteen sizes, read out of its own <c>TJ</c> arrays
    /// with our renderer never running: <c>probes/sheets-r62/probe-chartwidth.txt</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheChartDevicesEmIsAWholeNumberOfPixelsAndTheCorrectionIsASawtooth()
    {
        MetricGrid.Chart.PixelEmScale(Length.FromPoints(10)).ShouldBe(13 / (10 * 96.0 / 72.0), 1e-9);
        MetricGrid.Chart.PixelEmScale(Length.FromPoints(11)).ShouldBe(15 / (11 * 96.0 / 72.0), 1e-9);

        MetricGrid.Chart.PixelEmScale(Length.FromPoints(10)).ShouldBeLessThan(1.0);
        MetricGrid.Chart.PixelEmScale(Length.FromPoints(11)).ShouldBeGreaterThan(1.0);
        MetricGrid.Chart.PixelEmScale(Length.FromPoints(14)).ShouldBeGreaterThan(1.0);
        MetricGrid.Chart.PixelEmScale(Length.FromPoints(16)).ShouldBeLessThan(1.0);

        foreach (double exact in new[] { 9.0, 12.0, 18.0, 24.0 })
            MetricGrid.Chart.PixelEmScale(Length.FromPoints(exact)).ShouldBe(1.0, 1e-12);
    }
}

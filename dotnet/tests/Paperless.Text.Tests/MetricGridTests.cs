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
    private static LineMetrics LiberationSans(MetricGrid? grid = null)
        => new(1854, 434, 67, LineMetricSource.HorizontalHeader, 2048, grid);

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
        Length em = Length.FromPoints(11);
        LineMetrics gridded = LiberationSans(MetricGrid.Printer);

        Length ascent = gridded.ScaledAscent(em);
        Length descent = gridded.ScaledDescent(em);

        (ascent + descent).ShouldBe(gridded.ScaledLineHeight(em));
        ascent.Twips.ShouldBe(212);
        descent.Twips.ShouldBe(48);
    }

    [Fact]
    public void AGridOfNoResolutionMeasuresNothingRatherThanDividingByZero()
    {
        MetricGrid degenerate = new(0);

        degenerate.ToPixels(1854, 2048, Length.FromPoints(11)).ShouldBe(0);
        degenerate.ToLength(100).ShouldBe(Length.Zero);
    }
}

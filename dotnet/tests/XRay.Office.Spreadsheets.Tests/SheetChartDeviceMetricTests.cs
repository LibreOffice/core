using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A chart's text is quantised onto <c>chart2</c>'s own 96 dpi device, and these are the
/// reference's own numbers.
/// </summary>
/// <remarks>
/// <para>
/// Every expected value below was read out of a rendering made by the installed
/// LibreOffice 26.2.4.2, not computed from the model being tested:
/// <c>probes/sheets-r60/probe-chartvmetrics2.py</c> renders thirteen sizes on three faces as a
/// one-line and a three-line chart title and measures the baseline pitch between the lines, and
/// <c>law-chartvmetrics.py</c> reads them back. Each rendering is divided by its own
/// <c>drawn em / stated em</c> before it is compared, which is never more than 0.7%.
/// </para>
/// <para>
/// <strong>The three faces are what separate the two candidate laws.</strong> Carlito's
/// <c>hhea</c> line gap is zero and Liberation Sans' is 67/2048 and Liberation Serif's 87/2048, so
/// a model that keeps the external leading is wrong on two of the three and right on the first;
/// and the sizes where <c>size × 4/3</c> is not a whole number are what separate the pixel
/// rounding from exact scaling. A single face at a single size separates neither.
/// </para>
/// <para>
/// The tolerance is a tenth of a point because that is the instrument's own worst case over the
/// 39 measurements — the em is read off a PDF text matrix to two decimals — not a slack chosen to
/// let a model through.
/// </para>
/// </remarks>
public sealed class SheetChartDeviceMetricTests
{
    /// <summary>The reference's own pitches: (family, stated size, measured baseline pitch).</summary>
    public static TheoryData<string, double, double> ReferencePitches => new()
    {
        { "Calibri", 6.0, 7.503 },
        { "Calibri", 8.0, 9.730 },
        { "Calibri", 10.0, 11.219 },
        { "Calibri", 11.0, 13.460 },
        { "Calibri", 12.0, 14.232 },
        { "Calibri", 14.0, 17.232 },
        { "Calibri", 16.0, 19.585 },
        { "Calibri", 18.0, 21.708 },
        { "Calibri", 20.0, 24.692 },
        { "Calibri", 24.0, 29.314 },
        { "Calibri", 28.0, 33.791 },
        { "Calibri", 32.0, 39.797 },
        { "Calibri", 40.0, 47.992 },
        { "Arial", 6.0, 6.741 },
        { "Arial", 8.0, 8.990 },
        { "Arial", 10.0, 11.219 },
        { "Arial", 11.0, 12.720 },
        { "Arial", 12.0, 12.731 },
        { "Arial", 14.0, 15.731 },
        { "Arial", 16.0, 17.339 },
        { "Arial", 18.0, 20.179 },
        { "Arial", 20.0, 22.461 },
        { "Arial", 24.0, 27.064 },
        { "Arial", 28.0, 30.790 },
        { "Arial", 32.0, 36.034 },
        { "Arial", 40.0, 44.252 },
        { "Times New Roman", 6.0, 6.741 },
        { "Times New Roman", 10.0, 11.219 },
        { "Times New Roman", 11.0, 11.960 },
        { "Times New Roman", 12.0, 12.731 },
        { "Times New Roman", 16.0, 18.084 },
        { "Times New Roman", 18.0, 19.439 },
        { "Times New Roman", 24.0, 27.064 },
        { "Times New Roman", 32.0, 35.291 },
        { "Times New Roman", 40.0, 43.520 },
    };

    [Theory]
    [MemberData(nameof(ReferencePitches))]
    public void AChartStacksItsLinesAtTheReferencesOwnPitch(string family, double size, double pitch)
    {
        SheetBandText.ChartLineHeightAt(Length.FromPoints(size), family)
            .Points.ShouldBe(pitch, 0.10);
    }

    /// <summary>
    /// The pitch is a whole number of 96 dpi pixels — 0.75 pt — at every size on every face.
    /// </summary>
    /// <remarks>
    /// This is the shape of the law rather than one of its values, and it is what a continuous
    /// model cannot satisfy: <c>ascent + descent</c> for Carlito is 1.2207 em, which is a whole
    /// number of pixels at almost no size at all.
    /// </remarks>
    [Theory]
    [InlineData("Calibri")]
    [InlineData("Arial")]
    [InlineData("Times New Roman")]
    public void EveryChartLinePitchIsAWholeNumberOf96DpiPixels(string family)
    {
        foreach (double size in new[] { 6.0, 8.0, 10.0, 11.0, 12.0, 14.0, 16.0, 18.0, 20.0, 24.0 })
        {
            double pixels = SheetBandText.ChartLineHeightAt(Length.FromPoints(size), family).Points
                / 0.75;
            Math.Abs(pixels - Math.Round(pixels)).ShouldBeLessThan(0.05, $"{family} at {size} pt");
        }
    }

    /// <summary>
    /// A chart label's ascent at ten point in Carlito is nine points, not the face's 9.52.
    /// </summary>
    /// <remarks>
    /// Measured by requiring a CENTER data label's block centre to come out size-independent over
    /// ten sizes — <c>law-chartvmetrics.py</c> § B, where the pixel law spreads it by 0.042 pt and
    /// the face's own continuous metrics by 0.603. It is 12 pixels of 13, and the reference draws
    /// 9.00.
    /// </remarks>
    [Fact]
    public void AChartLabelsAscentIsTheDevicesAndNotTheFaces()
    {
        SheetBandText.ChartAscentAt(Length.FromPoints(10), "Calibri").Points
            .ShouldBe(9.0, 0.05);
        SheetBandText.AscentAt(Length.FromPoints(10), "Calibri").Points
            .ShouldBe(9.52, 0.05);
    }

    /// <summary>
    /// A chart's line is not a cell's line, and both differ from a drawing shape's.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Three devices and three answers at the same size in the same face: <c>chart2</c>'s 96 dpi,
    /// Calc's 720 dpi output device, and — for a drawing shape's text — the face's own
    /// <c>ascent + descent</c> on no device at all. This pins that they are three separate
    /// answers, so that a future change to one cannot silently take the others with it.
    /// </para>
    /// <para>
    /// <strong>The shape answer was <c>11.50</c> for Arial until round 69 and that was the external
    /// leading, which an EditEngine line does not carry.</strong> The value here is measured:
    /// <c>probes/shape-lineheight-01/</c> renders sixteen wrapping text boxes — four faces × four
    /// sizes on one unscaled sheet — through 26.2.4.2 and reads the baseline pitch off the
    /// reference's own text origins. Over 19 boxes <c>ascent + descent</c> is right to a mean of
    /// 0.008 pt, carrying the leading is out by a mean of 0.237 and by 1.02 pt at 24 pt, and
    /// putting the metrics through Calc's own 720 dpi grid is out by 0.035.
    /// </para>
    /// <para>
    /// <strong>Arial is in this assertion and Calibri cannot replace it.</strong> Carlito's
    /// <c>hhea</c> line gap is zero, so its shape height is 12.21 under either rule and it cannot
    /// tell them apart; Liberation Sans' is 67/2048, which is the 0.33 pt that separates 11.17 from
    /// 11.50. That is why the defect survived four rounds of Carlito workbooks.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheChartCellAndShapeMetricsAreThreeSeparateAnswers()
    {
        Length size = Length.FromPoints(10);

        SheetBandText.ChartLineHeightAt(size, "Calibri").Points.ShouldBe(11.25, 0.05);
        SheetBandText.LineHeightAt(size, "Calibri").Points.ShouldBe(12.20, 0.06);
        SheetBandText.ShapeLineHeightAt(size, "Calibri").Points.ShouldBe(12.21, 0.02);

        SheetBandText.ChartLineHeightAt(size, "Arial").Points.ShouldBe(11.25, 0.05);
        // 11.17 and not 11.50: Liberation Sans' 67/2048 of external leading is 0.327 pt at ten
        // point, and the tolerance is a fifteenth of it, so a change that quietly restores the
        // leading cannot pass this.
        SheetBandText.ShapeLineHeightAt(size, "Arial").Points.ShouldBe(11.17, 0.02);
    }

    /// <summary>The whole-pixel em a 96 dpi device sets, by stated size.</summary>
    /// <remarks>
    /// <c>round(size × 96/72)</c>, and the sizes are chosen so that the ratio it implies is
    /// <b>above</b> one at 8, 11, 14 and 20 pt and <b>below</b> it at 10, 13, 16, 22 and 28 and
    /// exactly one at 9, 12 and 18. A model that always narrows fails this, a model that never
    /// quantises fails it, and a wrong constant fails it in a different place at each size.
    /// </remarks>
    public static TheoryData<double, int> ChartPixelEms => new()
    {
        { 6.0, 8 }, { 8.0, 11 }, { 9.0, 12 }, { 10.0, 13 }, { 11.0, 15 }, { 12.0, 16 },
        { 13.0, 17 }, { 14.0, 19 }, { 16.0, 21 }, { 18.0, 24 }, { 20.0, 27 }, { 22.0, 29 },
        { 28.0, 37 },
    };

    /// <summary>
    /// A chart's advance width goes through the device's whole-pixel em, and it is a sawtooth.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The horizontal counterpart of <see cref="EveryChartLinePitchIsAWholeNumberOf96DpiPixels"/>,
    /// and the ratios are the reference's own.
    /// <c>probes/sheets-r62/probe-chartwidth.py</c> renders fourteen one-variable rewrites of
    /// <c>003_advanced_excel_pie</c>'s chart part through the installed 26.2.4.2 and reads the
    /// drawn advance out of the reference's own <c>TJ</c> arrays — whose per-glyph adjustments are
    /// in thousandths of the text em and so are independent of the chart's scale, the page and the
    /// size the PDF writer chose. Measured against <c>round(size × 96/72) / (size × 96/72)</c> the
    /// residual is at most 0.005 at every one of the fourteen, and always in the direction the
    /// estimator is known to be biased (it takes the modal adjustment per glyph, and a real kern
    /// pair narrows one occurrence).
    /// </para>
    /// <para>
    /// <b>The tolerance is not slack.</b> The step between adjacent laws here is 2 to 3 per cent —
    /// 0.975 against 1.023 between 10 and 11 pt — so 0.006 cannot let a wrong law through; it is
    /// there because the advance is rounded once per glyph and the sample string is twenty of them.
    /// </para>
    /// </remarks>
    [Theory]
    [MemberData(nameof(ChartPixelEms))]
    public void AChartsAdvanceWidthGoesThroughTheDevicesWholePixelEm(double size, int pixels)
    {
        const string sample = "M3; Actual; 107; 20%";
        Length em = Length.FromPoints(size);

        double natural = SheetBandText.Shape(sample, em, "Calibri", false)!.Width.Points;
        double chart = SheetBandText.ChartShape(sample, em, "Calibri", false)!.Width.Points;

        (chart / natural).ShouldBe(pixels / (size * 96.0 / 72.0), 0.006, $"at {size} pt");
    }

    /// <summary>
    /// The two labels that decide <c>003_advanced_excel_pie</c>'s best-fit test come out at the
    /// widths the reference actually drew them, and the ungridded ruler does not.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Read off 26.2.4.2's own rendering of that document's page 1 with <c>pdftotext -bbox</c>:
    /// the unwrapped label's ink is <b>79.66</b> pt wide and a wrapped line's <b>65.43</b>, against
    /// our unquantised 81.55 and 67.08. That 1.7 pt is the whole of the defect this fixes —
    /// <c>M3</c>'s block came out 75.90 wide against a threshold of 75.23 and missed the inner
    /// placement by 0.33 of a degree, which put the label outside the slice the reference keeps it
    /// in and three tokens rather than one onto the page-2 remainder of the chart.
    /// </para>
    /// <para>
    /// Both answers are asserted, not only the right one: the old ruler is pinned as the number it
    /// is so that a later change cannot quietly move the chart path back onto it.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheChartRunReproducesTheReferencesOwnDrawnLabelWidths()
    {
        Length ten = Length.FromPoints(10);

        SheetBandText.ChartShape("M1; Actual; 93; 17%", ten, "Calibri", false)!.Width.Points
            .ShouldBe(79.48, 0.30);
        SheetBandText.ChartShape("M3; Actual; 107;", ten, "Calibri", false)!.Width.Points
            .ShouldBe(65.37, 0.30);

        SheetBandText.Shape("M1; Actual; 93; 17%", ten, "Calibri", false)!.Width.Points
            .ShouldBe(81.55, 0.30);
        SheetBandText.Shape("M3; Actual; 107;", ten, "Calibri", false)!.Width.Points
            .ShouldBe(67.08, 0.30);
    }

    /// <summary>
    /// A chart run's glyphs keep the size the file states; only the pen moves.
    /// </summary>
    /// <remarks>
    /// The reference draws <c>003</c>'s labels at 10.008 pt with the narrower advances, not at
    /// 9.75 pt — the em is quantised for <em>measuring</em> and not for drawing. Shaping at the
    /// pixel em instead would scale the glyphs as well and is what this separates.
    /// </remarks>
    [Fact]
    public void AChartRunsGlyphsKeepTheStatedSizeAndOnlyThePenMoves()
    {
        Length ten = Length.FromPoints(10);

        SheetBandText.ChartShape("M1; Actual; 93; 17%", ten, "Calibri", false)!
            .At(new DocPoint(Length.Zero, Length.Zero)).FontSize.ShouldBe(ten);
    }
}

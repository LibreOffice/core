using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// Impress's and Calc's reference devices, against distances LibreOffice itself drew.
/// </summary>
/// <remarks>
/// <para>
/// The companion to <see cref="ReferenceGridTests"/>, which is Writer's. Three applications, three
/// devices, and no two of them the same:
/// </para>
/// <list type="table">
/// <item><description>Writer — 8640 dpi in twips, six pixels to the unit</description></item>
/// <item><description>Impress and Draw — <b>600 dpi in 1/100 mm</b>, a pixel worth 4.233 units</description></item>
/// <item><description>Calc — <b>720 dpi in 1/100 mm</b>, a pixel worth 3.528 units</description></item>
/// </list>
/// <para>
/// Every expectation below is read out of LibreOffice 26.2.4.2's own PDF. `probes/refdev-01/`
/// authors one page per (face, size) — six baselines in a slide's text box, two in a spreadsheet's
/// cell — and reads the text matrices at full stream precision, so the first baseline's distance
/// below the frame top <em>is</em> the ascent and the gap between two baselines <em>is</em> the
/// line height. The grid reproduces <b>507 of 507</b> pairs on Impress and <b>468 of 468</b> on
/// Calc, ascent and line height both, across thirteen faces.
/// </para>
/// <para>
/// The design-unit metrics are stated here rather than read from the installed font files, so the
/// arithmetic is under test without the test depending on a face being present.
/// </para>
/// </remarks>
public class ApplicationGridTests
{
    // hhea ascender, hhea descender negated, hhea line gap, units per em — the three numbers
    // `FontMetricData::ImplCalcLineSpacing` ends up believing, and the em they are in.
    private static LineMetrics Serif(MetricGrid grid)
        => new(1825, 443, 87, LineMetricSource.HorizontalHeader, 2048, grid);

    private static LineMetrics Sans(MetricGrid grid)
        => new(1854, 434, 67, LineMetricSource.HorizontalHeader, 2048, grid);

    private static LineMetrics Carlito(MetricGrid grid)
        => new(1950, 550, 0, LineMetricSource.HorizontalHeader, 2048, grid);

    private static LineMetrics Caladea(MetricGrid grid)
        => new(900, 250, 0, LineMetricSource.TypographicMetrics, 1000, grid);

    private static LineMetrics DejaVuSans(MetricGrid grid)
        => new(1901, 483, 0, LineMetricSource.HorizontalHeader, 2048, grid);

    private static LineMetrics Face(string name, MetricGrid grid) => name switch
    {
        "serif" => Serif(grid),
        "sans" => Sans(grid),
        "carlito" => Carlito(grid),
        "caladea" => Caladea(grid),
        "dejavu" => DejaVuSans(grid),
        _ => throw new ArgumentOutOfRangeException(nameof(name)),
    };

    [Fact]
    public void ImpressFormatsAgainstSixHundredDpiInHundredthsOfAMillimetre()
    {
        // sd/source/ui/app/sdmod.cxx:83-85 — one VirtualDevice for the whole application, its map
        // mode set to Map100thMM and then RefDevMode::Dpi600. A device pixel is 4.233 units, which
        // is 2.4 twips: eighteen times the coarsest thing Writer ever sees.
        MetricGrid.Presentation.Dpi.ShouldBe(600);
        MetricGrid.Presentation.Unit.ShouldBe(MetricUnit.Mm100);
        MetricGrid.Presentation.QuantisesAdvances.ShouldBeFalse();

        // 18 pt is 635 units exactly, and 635 units at 600 dpi is 150 whole pixels.
        MetricGrid.Presentation.ToPixels(2048, 2048, Length.FromPoints(18)).ShouldBe(150);
        MetricGrid.Presentation.ToLength(150).Mm100.ShouldBe(635);
    }

    [Fact]
    public void CalcFormatsAgainstSevenHundredAndTwentyDpiInHundredthsOfAMillimetre()
    {
        // Not ScDocument's own device, and that is the whole point. `GetVirtualDevice_100th_mm`
        // really is RefDevMode::MSO1 at 8640 dpi (sc/source/core/data/documen8.cxx:182-193) and it
        // is not what draws a printed cell: ScOutputData formats against the *output* device, which
        // on a PDF export is the writer's own reference device, RefDevMode::PDF1 = 720 dpi.
        // 8640 dpi in 1/100 mm scores 92 of 273 measured pairs and 720 dpi scores 273 of 273.
        MetricGrid.Spreadsheet.Dpi.ShouldBe(720);
        MetricGrid.Spreadsheet.Unit.ShouldBe(MetricUnit.Mm100);
        MetricGrid.Spreadsheet.QuantisesAdvances.ShouldBeFalse();

        // 18 pt is 635 units, and 635 units at 720 dpi is 180 whole pixels.
        MetricGrid.Spreadsheet.ToPixels(2048, 2048, Length.FromPoints(18)).ShouldBe(180);
        MetricGrid.Spreadsheet.ToLength(180).Mm100.ShouldBe(635);
    }

    [Theory]
    [InlineData("serif", 5.0, 157, 195)]
    [InlineData("serif", 8.5, 267, 331)]
    [InlineData("serif", 10.0, 313, 389)]
    [InlineData("serif", 12.0, 377, 470)]
    [InlineData("serif", 15.5, 487, 606)]
    [InlineData("serif", 19.0, 597, 741)]
    [InlineData("serif", 22.5, 711, 885)]
    [InlineData("sans", 6.5, 207, 254)]
    [InlineData("sans", 10.0, 318, 394)]
    [InlineData("sans", 13.5, 428, 530)]
    [InlineData("sans", 17.0, 546, 673)]
    [InlineData("sans", 18.0, 576, 711)]
    [InlineData("sans", 20.5, 656, 809)]
    [InlineData("sans", 24.0, 766, 944)]
    [InlineData("carlito", 8.0, 271, 347)]
    [InlineData("carlito", 11.5, 385, 495)]
    [InlineData("carlito", 15.0, 504, 648)]
    [InlineData("carlito", 18.5, 622, 796)]
    [InlineData("carlito", 22.0, 737, 944)]
    [InlineData("caladea", 6.0, 191, 246)]
    [InlineData("caladea", 9.5, 301, 386)]
    [InlineData("caladea", 13.0, 411, 525)]
    [InlineData("caladea", 16.5, 521, 665)]
    [InlineData("caladea", 20.0, 635, 813)]
    [InlineData("caladea", 23.5, 745, 953)]
    [InlineData("dejavu", 7.5, 246, 310)]
    [InlineData("dejavu", 11.0, 360, 453)]
    [InlineData("dejavu", 14.5, 474, 597)]
    [InlineData("dejavu", 18.0, 588, 737)]
    [InlineData("dejavu", 21.5, 703, 881)]
    public void AnImpressLineIsAsTallAsLibreOfficeDrawsIt(
        string face, double points, long ascent, long height)
    {
        // Read off a six-line text box on its own slide, `probes/refdev-01/probe-impress.py`. The
        // box asks for style:font-independent-line-spacing="false", which is ODF's default and the
        // only branch that consults a face's metrics at all — a PPTX body sets the flag and gets
        // ImplCalculateFontIndependentLineSpacing, where no metric is read.
        LineMetrics metrics = Face(face, MetricGrid.Presentation);
        Length em = Length.FromPoints(points);

        metrics.ScaledAscent(em).Mm100.ShouldBe(ascent);
        metrics.ScaledLineHeight(em).Mm100.ShouldBe(height);
        (metrics.ScaledAscent(em) + metrics.ScaledDescent(em))
            .ShouldBe(metrics.ScaledLineHeight(em));
    }

    [Theory]
    [InlineData("serif", 5.0, 159, 198)]
    [InlineData("serif", 8.5, 268, 332)]
    [InlineData("serif", 12.0, 377, 469)]
    [InlineData("serif", 15.5, 487, 607)]
    [InlineData("serif", 19.0, 596, 741)]
    [InlineData("serif", 22.5, 709, 882)]
    [InlineData("sans", 6.5, 208, 258)]
    [InlineData("sans", 10.0, 321, 395)]
    [InlineData("sans", 13.5, 430, 533)]
    [InlineData("sans", 17.0, 543, 670)]
    [InlineData("sans", 20.5, 656, 808)]
    [InlineData("sans", 24.0, 766, 946)]
    [InlineData("carlito", 8.0, 268, 342)]
    [InlineData("carlito", 11.5, 385, 494)]
    [InlineData("carlito", 15.0, 504, 646)]
    [InlineData("carlito", 18.5, 621, 797)]
    [InlineData("carlito", 22.0, 737, 945)]
    [InlineData("caladea", 6.0, 191, 244)]
    [InlineData("caladea", 9.5, 303, 388)]
    [InlineData("caladea", 13.0, 413, 529)]
    [InlineData("caladea", 16.5, 526, 671)]
    [InlineData("caladea", 20.0, 635, 811)]
    [InlineData("caladea", 23.5, 748, 956)]
    [InlineData("dejavu", 7.5, 247, 311)]
    [InlineData("dejavu", 11.0, 360, 452)]
    [InlineData("dejavu", 14.5, 476, 596)]
    [InlineData("dejavu", 18.0, 589, 737)]
    [InlineData("dejavu", 21.5, 706, 886)]
    public void ACalcLineIsAsTallAsLibreOfficeDrawsIt(
        string face, double points, long ascent, long height)
    {
        // Read off a two-paragraph cell in a top-aligned row of its own printed page,
        // `probes/refdev-01/probe-calc.py`. A cell holding two paragraphs is an EditCell and takes
        // the same EditEngine path a slide's text box does — the device underneath it is what
        // differs.
        LineMetrics metrics = Face(face, MetricGrid.Spreadsheet);
        Length em = Length.FromPoints(points);

        metrics.ScaledAscent(em).Mm100.ShouldBe(ascent);
        metrics.ScaledLineHeight(em).Mm100.ShouldBe(height);
        (metrics.ScaledAscent(em) + metrics.ScaledDescent(em))
            .ShouldBe(metrics.ScaledLineHeight(em));
    }

    [Theory]
    [InlineData(5.0, 157, 195, 159, 198)]
    [InlineData(6.0, 191, 238, 187, 233)]
    [InlineData(7.0, 220, 275, 219, 272)]
    [InlineData(7.5, 237, 296, 236, 293)]
    public void TheTwoApplicationsAreTwoDevicesAndNotOne(
        double points, long impressAscent, long impressHeight, long calcAscent, long calcHeight)
    {
        // The same face at the same size, and LibreOffice draws two different lines depending on
        // which application is drawing it. Kept so that collapsing Impress's and Calc's devices
        // into one — which is exactly what reading `ScDocument::GetVirtualDevice_100th_mm` invites,
        // since it names the same RefDevMode Writer's does — fails here.
        Length em = Length.FromPoints(points);

        Serif(MetricGrid.Presentation).ScaledAscent(em).Mm100.ShouldBe(impressAscent);
        Serif(MetricGrid.Presentation).ScaledLineHeight(em).Mm100.ShouldBe(impressHeight);
        Serif(MetricGrid.Spreadsheet).ScaledAscent(em).Mm100.ShouldBe(calcAscent);
        Serif(MetricGrid.Spreadsheet).ScaledLineHeight(em).Mm100.ShouldBe(calcHeight);
    }

    [Theory]
    // Impress, 600 dpi: the pair converted separately is the taller at 6.0 and 8.5 pt…
    [InlineData(600, 6.0, 238, 237, 238)]
    [InlineData(600, 8.5, 331, 330, 331)]
    // …and the shorter at 10.5 and 18.0, so neither rounding is the rule and their maximum is.
    [InlineData(600, 10.5, 410, 411, 411)]
    [InlineData(600, 18.0, 702, 703, 703)]
    // Calc, 720 dpi: the same disagreement, in both directions, on a different device.
    [InlineData(720, 7.5, 292, 293, 293)]
    [InlineData(720, 11.0, 431, 430, 431)]
    [InlineData(720, 17.0, 664, 663, 664)]
    [InlineData(720, 22.0, 860, 861, 861)]
    public void EditEngineKeepsTheTallerOfTheTwoRoundings(
        int dpi, double points, long split, long sum, long drawn)
    {
        // EditEngine measures a line twice and keeps the larger. The text portion's own height is
        // OutputDevice::GetTextHeight, which converts the summed device-pixel ascent and descent in
        // one step; the formatter metric is FormatterFontMetric::GetHeight, which is
        // GetFontMetric().GetAscent() + GetDescent() and converts each on its own
        // (vcl/source/outdev/font.cxx:351-352). ImpEditEngine::CreateLines then does
        //   if (nLineHeight > pLine->GetHeight()) pLine->SetHeight(nLineHeight)
        // — editeng/source/editeng/impedit3.cxx:1516-1518.
        //
        // Over the 312-pair Impress table, converting each separately is right on 274 and
        // converting the sum once on 270; they disagree in *opposite* directions, so their maximum
        // is right on 312. Both single rules are stated here so that proposing either fails.
        MetricGrid grid = new(dpi, QuantisesAdvances: false, Unit: MetricUnit.Mm100);
        Length em = Length.FromPoints(points);

        long ascentPx = grid.ToPixels(1825, 2048, em);
        long descentPx = grid.ToPixels(443, 2048, em);

        (grid.ToLength(ascentPx) + grid.ToLength(descentPx)).Mm100.ShouldBe(split);
        grid.ToLength(ascentPx + descentPx).Mm100.ShouldBe(sum);

        Serif(grid).ScaledLineHeight(em).Mm100.ShouldBe(drawn);
        drawn.ShouldBe(Math.Max(split, sum));
    }

    [Fact]
    public void TheEngineDecidesTheGroupingAndNotOnlyWhereTheLeadingSits()
    {
        // `leadingAboveText` is not a cosmetic flag. It is which of LibreOffice's two text engines
        // the metrics belong to, and the two engines differ in three ways at once: Writer converts
        // ascent-plus-descent as one and adds a separately converted line gap; EditEngine converts
        // each on its own, takes the taller of that and the summed conversion, and has no gap at
        // all. On Liberation Serif at 10 pt on Writer's own device that is 231 twips against 222.
        Length em = Length.FromPoints(10);

        LineMetrics writer = Serif(MetricGrid.Reference) with { LeadingAboveText = true };
        LineMetrics editEngine = Serif(MetricGrid.Reference);

        writer.ScaledLineHeight(em).Twips.ShouldBe(231);
        editEngine.ScaledLineHeight(em).Twips.ShouldBe(222);

        // Writer's ascent carries the leading; EditEngine's does not.
        writer.ScaledAscent(em).Twips.ShouldBe(187);
        editEngine.ScaledAscent(em).Twips.ShouldBe(178);
    }

    [Fact]
    public void TheLogicalUnitIsPartOfTheDeviceAndNotAnAfterthought()
    {
        // The same resolution in two map units is two different quantisations, and getting this
        // wrong is invisible in a resolution sweep: 8640 dpi in twips is six pixels to the unit and
        // rounds essentially nothing, where 8640 dpi in 1/100 mm is 3.4 and rounds a great deal.
        // Two prior rounds swept 72 to 6000 dpi in twips and could not find any of the three
        // devices this project actually uses.
        Length em = Length.FromPoints(10);

        MetricGrid twips = new(8640, QuantisesAdvances: false, Unit: MetricUnit.Twip);
        MetricGrid mm100 = new(8640, QuantisesAdvances: false, Unit: MetricUnit.Mm100);

        // 10 pt is 200 twips and 353 hundredths of a millimetre, and those are not the same number
        // of pixels even at the same resolution — 1200 against 1201, because the em is snapped to a
        // whole logical unit before the device sees it.
        twips.ToPixels(2048, 2048, em).ShouldBe(1200);
        mm100.ToPixels(2048, 2048, em).ShouldBe(1201);

        // …and yet they answer differently, because converting back is a sixth of a twip in one
        // and 0.294 of a hundredth of a millimetre in the other. 222 twips is 391.7 units, and the
        // millimetre device draws 391.
        Serif(twips).ScaledLineHeight(em).Twips.ShouldBe(222);
        Serif(mm100).ScaledLineHeight(em).Mm100.ShouldBe(391);
    }

    [Fact]
    public void ADeviceInMillimetresRoundsTheEmSizeFirst()
    {
        // The order is the thing that was wrong before, not the unit. The device sets the font at a
        // whole number of pixels and only then scales the metrics through it, so 5 pt — 176 units,
        // 42 pixels at 600 dpi — gives Liberation Sans an ascent of 161 units where rounding an
        // exactly scaled ascent gives 160. Over the 507-pair table that order is worth a unit on
        // 425 of them.
        Length em = Length.FromPoints(5);

        MetricGrid.Presentation.ToEmSize(em).Mm100.ShouldBe(178);
        MetricGrid.Presentation.ToPixels(2048, 2048, em).ShouldBe(42);

        Sans(MetricGrid.Presentation).ScaledAscent(em).Mm100.ShouldBe(161);
        Length.FromMm100((long)Math.Round(1854 * 176.0 / 2048)).Mm100.ShouldBe(159);
    }
}

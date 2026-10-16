using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// The grid Writer measures every ordinary document's line heights on.
/// </summary>
/// <remarks>
/// <para>
/// <b>Every expectation here is a distance LibreOffice 26.2.4.2 itself drew</b>, read out of the
/// <c>Td</c>/<c>Tm</c> operators of its own PDF by <c>dotnet/probes/lineheight-01/probe-grid.py</c>:
/// one page per (face, size) with two lines on it, so the first baseline's distance below the top
/// margin is the ascent and the gap between the two is the line height. The probe covers five faces
/// at every half point from 5 to 24 — 195 pairs — and the rule these tests state is exact on all 195,
/// ascent and line height both. A second run over eight more faces, including bold, italic,
/// monospace and symbol cuts, is exact on a further 234.
/// </para>
/// <para>
/// The rule is a device, not a formula. Writer formats against a <c>VirtualDevice</c> in
/// <c>VirtualDevice::RefDevMode::MSO1</c> with <c>MapUnit::MapTwip</c>
/// (<c>sw/source/core/doc/DocumentDeviceManager.cxx</c>:259), and <c>MSO1</c> is <c>6*1440</c> = 8640
/// dpi (<c>vcl/source/gdi/virdev.cxx</c>:407) — six device pixels to the twip. Ascent, descent and
/// line gap are each rounded to a whole pixel there
/// (<c>FontMetricData::ImplCalcLineSpacing</c>, <c>vcl/source/font/fontmetric.cxx</c>:538-540);
/// <c>OutputDevice::GetTextHeight</c> then converts ascent-plus-descent back to twips as one value
/// and <c>GetFontMetric</c> converts the line gap on its own, so it is <b>two</b> roundings of a
/// three-term sum, grouped 2 + 1.
/// </para>
/// <para>
/// The design-unit metrics below are stated rather than read from the installed files, so the
/// arithmetic is tested without the tests depending on a font being present.
/// </para>
/// </remarks>
public class ReferenceGridTests
{
    // hhea ascender, −descender, lineGap; units per em. Liberation Serif and Liberation Sans are the
    // pair that matters most: their totals are the *same* 2355 units.
    // Every expectation in this file is a distance Writer drew, so every face here is built the way
    // Writer asks for one: `leadingAbove: true`. That flag is not decoration — it is which of
    // LibreOffice's two text engines the metrics belong to, and it decides the *grouping* of the
    // conversion as well as where the leading sits. Writer converts ascent-plus-descent once and adds
    // the gap (`GetTextHeight() + GetFontLeading()`); EditEngine takes the taller of two roundings and
    // has no gap at all. `refdev-01` measured the second on 780 pairs across Impress and Calc.
    private static LineMetrics Serif(MetricGrid? grid = null, bool leadingAbove = true)
        => new(1825, 443, 87, LineMetricSource.HorizontalHeader, 2048, grid, leadingAbove);

    private static LineMetrics Sans(MetricGrid? grid = null, bool leadingAbove = true)
        => new(1854, 434, 67, LineMetricSource.HorizontalHeader, 2048, grid, leadingAbove);

    private static LineMetrics Carlito(MetricGrid? grid = null)
        => new(1950, 550, 0, LineMetricSource.HorizontalHeader, 2048, grid, LeadingAboveText: true);

    private static LineMetrics Caladea(MetricGrid? grid = null)
        => new(900, 250, 0, LineMetricSource.TypographicMetrics, 1000, grid, LeadingAboveText: true);

    private static LineMetrics DejaVuSans(MetricGrid? grid = null)
        => new(1901, 483, 0, LineMetricSource.HorizontalHeader, 2048, grid, LeadingAboveText: true);

    [Fact]
    public void TheReferenceDeviceIsSixDevicePixelsToTheTwip()
    {
        // 8640 dpi is what makes the *horizontal* half of the grid a no-op and the vertical half worth
        // exactly one twip: the em is set in whole pixels, and at six pixels to the twip every font
        // size a document can express is already a whole number of them.
        MetricGrid.Reference.Dpi.ShouldBe(8640);
        MetricGrid.Reference.ToPixels(2048, 2048, Length.FromPoints(10)).ShouldBe(1200);
        MetricGrid.Reference.ToLength(1200).Twips.ShouldBe(200);
    }

    [Theory]
    // Liberation Serif — 1825 + 443 + 87 = 2355 units
    [InlineData("serif", 5.0, 115)]
    [InlineData("serif", 8.0, 184)]
    [InlineData("serif", 9.0, 207)]
    [InlineData("serif", 10.0, 231)]
    [InlineData("serif", 10.5, 242)]
    [InlineData("serif", 11.0, 253)]
    [InlineData("serif", 12.0, 276)]
    [InlineData("serif", 13.0, 299)]
    [InlineData("serif", 16.0, 368)]
    [InlineData("serif", 18.0, 414)]
    [InlineData("serif", 24.0, 552)]
    // Liberation Sans — 1854 + 434 + 67 = 2355 units, the identical total
    [InlineData("sans", 5.0, 115)]
    [InlineData("sans", 8.0, 184)]
    [InlineData("sans", 9.0, 207)]
    [InlineData("sans", 10.0, 230)]
    [InlineData("sans", 11.0, 253)]
    [InlineData("sans", 12.0, 276)]
    [InlineData("sans", 13.0, 300)]
    [InlineData("sans", 16.0, 369)]
    [InlineData("sans", 24.0, 552)]
    // Carlito, whose line gap is zero
    [InlineData("carlito", 10.0, 244)]
    [InlineData("carlito", 12.0, 293)]
    [InlineData("carlito", 18.0, 440)]
    [InlineData("carlito", 24.0, 586)]
    // Caladea, which asks for its typographic metrics and has a 1000-unit em
    [InlineData("caladea", 10.0, 230)]
    [InlineData("caladea", 13.0, 299)]
    [InlineData("caladea", 24.0, 552)]
    // DejaVu Sans
    [InlineData("dejavu", 10.0, 233)]
    [InlineData("dejavu", 12.0, 280)]
    [InlineData("dejavu", 24.0, 559)]
    public void ALineIsAsTallAsLibreOfficeDrawsIt(string face, double points, long twips)
        => Face(face, MetricGrid.Reference)
            .ScaledLineHeight(Length.FromPoints(points)).Twips.ShouldBe(twips);

    [Theory]
    [InlineData(10.0, 230, 231)]
    [InlineData(13.0, 300, 299)]
    [InlineData(16.0, 369, 368)]
    public void TheSplitBetweenAscentAndDescentDecidesRatherThanTheirSum(
        double points, long sans, long serif)
    {
        // Liberation Sans and Liberation Serif state the *same* 2355 units over the same 2048-unit em,
        // so any rule that is a function of the total predicts one number for both — and LibreOffice
        // draws two, differing in *both* directions across these three sizes. That is the refutation
        // `words-pages-01` §4 recorded and could not explain: the two faces part company because their
        // ascents and descents round to different whole pixels before anything is added up.
        Length em = Length.FromPoints(points);

        (Sans().LineHeight == Serif().LineHeight).ShouldBeTrue("the design totals are identical");
        Sans(MetricGrid.Reference).ScaledLineHeight(em).Twips.ShouldBe(sans);
        Serif(MetricGrid.Reference).ScaledLineHeight(em).Twips.ShouldBe(serif);
    }

    [Fact]
    public void OneCaseRefutesEveryRuleThatWasTriedBeforeThisOne()
    {
        // Liberation Serif at 10 pt. LibreOffice draws 231 twips; the em is 1200 device pixels, and
        // the three metrics land on 1069.336, 259.570 and 50.977 pixels.
        //
        // Every alternative below was proposed and measured by an earlier round, and each is stated
        // here so that re-proposing it fails rather than being re-derived. The device value 231 is the
        // only one of the five that is what LibreOffice draws.
        Length em = Length.FromPoints(10);
        const int Upem = 2048;

        MetricGrid reference = MetricGrid.Reference;
        long ascentPx = reference.ToPixels(1825, Upem, em);
        long descentPx = reference.ToPixels(443, Upem, em);
        long gapPx = reference.ToPixels(87, Upem, em);

        (ascentPx, descentPx, gapPx).ShouldBe((1069L, 260L, 51L));

        // What the tree does now: (ascent + descent) converted as one, the gap converted alone.
        Serif(reference).ScaledLineHeight(em).Twips.ShouldBe(231);

        // 1. Scale the design-unit total once. This is what the tree did before, and 22 of 195 pairs
        //    disagree with it.
        Length.FromEmu((long)Math.Round(2355L * em.Emu / (double)Upem)).Twips.ShouldBe(230);

        // 2. Round all three separately, straight to twips, with no device in it.
        long separately = (long)Math.Round(1825 * 200.0 / Upem)
            + (long)Math.Round(443 * 200.0 / Upem)
            + (long)Math.Round(87 * 200.0 / Upem);
        separately.ShouldBe(229);

        // 3. Round all three to whole device pixels and convert the sum once — the 3 + 1 grouping
        //    rather than the 2 + 1 one. This is the near miss, and it is a miss.
        reference.ToLength(ascentPx + descentPx + gapPx).Twips.ShouldBe(230);

        // 4. Convert the gap with .NET's default midpoint rule. 51 pixels is 8.5 twips, and banker's
        //    rounding takes a half to even where C++ `llround` takes it away from zero
        //    (`CoordinateMapper::ViewToLogicDistanceY`, vcl/source/outdev/CoordinateMapper.cxx:279).
        Math.Round(gapPx / 6.0).ShouldBe(8);
        reference.ToLength(gapPx).Twips.ShouldBe(9);
    }

    [Theory]
    [InlineData("serif", 5.0, 93)]
    [InlineData("serif", 10.0, 187)]
    [InlineData("serif", 12.0, 224)]
    [InlineData("serif", 24.0, 448)]
    [InlineData("sans", 5.0, 94)]
    [InlineData("sans", 10.0, 188)]
    [InlineData("sans", 11.0, 206)]
    [InlineData("sans", 24.0, 451)]
    public void TheFirstBaselineSitsWhereLibreOfficePutsIt(string face, double points, long twips)
    {
        // Writer charges the external leading to the *ascent* — `SwFntObj::GetFontAscent` adds
        // `GetFontLeading` after the CJK adjustment (sw/source/core/txtnode/fntcache.cxx:324-329) — so
        // the ascent is the gridded ascent plus the separately gridded gap, and this is what decides
        // where the first line of every page lands.
        Length em = Length.FromPoints(points);
        LineMetrics writer = Face(face, MetricGrid.Reference) with { LeadingAboveText = true };

        writer.ScaledAscent(em).Twips.ShouldBe(twips);
        (writer.ScaledAscent(em) + writer.ScaledDescent(em)).ShouldBe(writer.ScaledLineHeight(em));
    }

    [Fact]
    public void TheReferenceDeviceDoesNotQuantiseAdvances()
    {
        // The vertical metrics go through the grid and the horizontal ones do not, and that asymmetry
        // is measured rather than assumed: `probes/printer-metric-advance.py`'s control half — the
        // same body with `fUsePrinterMetrics` clear — has unquantised scaling exact on 96 of 96 rows.
        // A printer's grid rounds the em to whole pixels and every advance with it; this one cannot,
        // because at six pixels to the twip the em is already whole.
        MetricGrid.Reference.QuantisesAdvances.ShouldBeFalse();
        MetricGrid.Printer.QuantisesAdvances.ShouldBeTrue();
    }

    [Fact]
    public void APrintersGridIsStillTheOtherAnswer()
    {
        // The two grids are not interchangeable and the difference is not a rounding. An 11 pt
        // Liberation Sans line is 253 twips on the reference device and 252 on the 600 dpi printer,
        // and a 9.5 pt Liberation Serif line is 219 against 216 — which is the pair that shows the
        // printer is a coarser grid rather than a smaller number, because it moves in both
        // directions. Every figure is LibreOffice's own; the printer ones come from its renderings
        // of `A_320.doc` and `150_5300_13_chg10.doc`, whose Dops set fUsePrinterMetrics, and agree
        // with `probes/printer-metric-advance.py`'s authored pair. Kept so that collapsing the two
        // grids into one fails here.
        //
        // 260 stood on the second line until 2026-08-15 and was 300 dpi's answer; see
        // MetricGridTests' class remark for the three sources that replaced it.
        Length em = Length.FromPoints(11);

        Sans(MetricGrid.Reference).ScaledLineHeight(em).Twips.ShouldBe(253);
        Sans(MetricGrid.Printer).ScaledLineHeight(em).Twips.ShouldBe(252);

        Length small = Length.FromPoints(9.5);
        Serif(MetricGrid.Reference).ScaledLineHeight(small).Twips.ShouldBe(219);
        Serif(MetricGrid.Printer).ScaledLineHeight(small).Twips.ShouldBe(216);
    }

    private static LineMetrics Face(string name, MetricGrid grid) => name switch
    {
        "serif" => Serif(grid),
        "sans" => Sans(grid),
        "carlito" => Carlito(grid),
        "caladea" => Caladea(grid),
        "dejavu" => DejaVuSans(grid),
        _ => throw new ArgumentOutOfRangeException(nameof(name)),
    };
}

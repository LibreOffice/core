using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// Word's 127% line scale for a face that declares an East Asian code page.
/// </summary>
/// <remarks>
/// <para>
/// <b>Every expectation here is a distance LibreOffice 26.2.4.2 itself drew</b>, read out of its own
/// PDF by <c>dotnet/probes/words-metrics-01/probe-cjk127.py</c> — the same two-lines-per-page
/// arrangement <see cref="ReferenceGridTests"/> uses, so the first baseline's distance below the top
/// margin is the ascent and the gap between the two baselines is the line height. Three faces at
/// every half point from 5 to 24 pt, 117 pairs, and the rule below is exact on all 117 in both
/// columns.
/// </para>
/// <para>
/// The rule is <c>lcl_ApplyCjkHeightAdjustment</c> (<c>sw/source/core/txtnode/fntcache.cxx</c>:270-292,
/// tdf#129808): with the document's <c>MS_WORD_COMP_GRID_METRICS</c> flag set and the face declaring
/// CP932, CP936, CP949 or CP950, <c>(nBase * 127) / 100</c> in integer arithmetic.
/// </para>
/// <para>
/// <b>What it multiplies is the point of these tests.</b> <c>probes/lineheight-01</c> §7(a) recorded
/// the rule as scaling the finished ascent and line height, and was exact on all 39 of its IPAGothic
/// pairs — but IPAGothic's <c>hhea</c> line gap is <b>zero</b>, so on that face the leading term
/// vanishes and the question cannot be asked. <c>GetFontHeight</c> reads
/// <c>lcl_ApplyCjkHeightAdjustment(m_nPrtHeight, …) + GetFontLeading(…)</c>: the scale reaches the
/// device's ascent-plus-descent and the leading is added afterwards, unscaled. WenQuanYi Zen Hei's
/// gap of 92/1024 separates the two readings — at 12 pt they give 406 twips and 412, and LibreOffice
/// draws 406.
/// </para>
/// <para>
/// The design-unit metrics are stated rather than read from the installed files, so the arithmetic is
/// tested without the tests depending on a font being present.
/// </para>
/// </remarks>
public class EastAsianLineScaleTests
{
    // hhea ascender, −descender, lineGap; units per em. WenQuanYi Zen Hei is the face that decides
    // this rule, because it is the one with a line gap.
    private static LineMetrics ZenHei(MetricGrid grid)
        => new(986, 304, 92, LineMetricSource.HorizontalHeader, 1024, grid, LeadingAboveText: true,
               DeclaresEastAsianCodePage: true);

    private static LineMetrics IpaGothic(MetricGrid grid)
        => new(1802, 246, 0, LineMetricSource.HorizontalHeader, 2048, grid, LeadingAboveText: true,
               DeclaresEastAsianCodePage: true);

    // The control: a line gap of its own and no East Asian code page, so the scale must not touch it.
    private static LineMetrics Serif(MetricGrid grid)
        => new(1825, 443, 87, LineMetricSource.HorizontalHeader, 2048, grid, LeadingAboveText: true);

    private static Length Pt(double points) => Length.FromPoints(points);

    [Theory]
    // face, points, ascent in twips, line height in twips — all measured.
    [InlineData(5.0, 130, 169)]
    [InlineData(6.0, 158, 202)]
    [InlineData(8.0, 209, 270)]
    [InlineData(10.0, 263, 338)]
    [InlineData(11.0, 289, 371)]
    [InlineData(12.0, 315, 406)]
    [InlineData(14.0, 367, 473)]
    [InlineData(16.0, 420, 540)]
    [InlineData(18.0, 472, 608)]
    [InlineData(20.0, 524, 676)]
    [InlineData(24.0, 629, 811)]
    public void WenQuanYiZenHeiScalesTheWayLibreOfficeDrewIt(double points, int ascent, int height)
    {
        LineMetrics metrics = ZenHei(MetricGrid.Reference.AsWordDocument());

        metrics.ScaledAscent(Pt(points)).Twips.ShouldBe(ascent);
        metrics.ScaledLineHeight(Pt(points)).Twips.ShouldBe(height);
    }

    [Theory]
    [InlineData(5.0, 111, 127)]
    [InlineData(6.0, 134, 152)]
    [InlineData(8.0, 179, 203)]
    [InlineData(10.0, 223, 254)]
    [InlineData(11.0, 246, 279)]
    [InlineData(12.0, 267, 304)]
    [InlineData(14.0, 312, 355)]
    [InlineData(16.0, 358, 406)]
    [InlineData(18.0, 402, 457)]
    [InlineData(20.0, 447, 508)]
    [InlineData(24.0, 535, 609)]
    public void IpaGothicScalesTheWayLibreOfficeDrewIt(double points, int ascent, int height)
    {
        LineMetrics metrics = IpaGothic(MetricGrid.Reference.AsWordDocument());

        metrics.ScaledAscent(Pt(points)).Twips.ShouldBe(ascent);
        metrics.ScaledLineHeight(Pt(points)).Twips.ShouldBe(height);
    }

    [Theory]
    [InlineData(5.0, 93, 115)]
    [InlineData(10.0, 187, 231)]
    [InlineData(12.0, 224, 276)]
    [InlineData(18.0, 336, 414)]
    [InlineData(24.0, 448, 552)]
    public void AFaceDeclaringNoEastAsianCodePageIsUntouched(double points, int ascent, int height)
    {
        // The same document, the same grid, the same flag: only the face differs, and the scale must
        // not reach it. Measured in the same run as the two above.
        LineMetrics metrics = Serif(MetricGrid.Reference.AsWordDocument());

        metrics.ScaledAscent(Pt(points)).Twips.ShouldBe(ascent);
        metrics.ScaledLineHeight(Pt(points)).Twips.ShouldBe(height);
    }

    [Fact]
    public void TheScaleReachesTheAscentAndDescentAndNotTheLeading()
    {
        // The whole of what `lineheight-01` §7(a) could not decide, in one assertion. WenQuanYi Zen
        // Hei at 12 pt on the reference grid: a = 1387, d = 428, g = 129 device pixels.
        //
        //   ascent+descent -> round(1815/6) = 303 twips, scaled: 303*127/100 = 384
        //   leading        -> round( 129/6) =  22 twips, unscaled
        //   line height    -> 384 + 22 = 406, which is what LibreOffice drew.
        //
        // Scaling the finished height instead gives (303 + 22) * 127 / 100 = 412, six twips too many
        // — and on a face with no line gap the two are the same number, which is why 39 of 39 IPAGothic
        // pairs agreed with the wrong rule.
        LineMetrics scaled = ZenHei(MetricGrid.Reference.AsWordDocument());
        LineMetrics plain = ZenHei(MetricGrid.Reference);

        plain.ScaledLineHeight(Pt(12)).Twips.ShouldBe(325);
        scaled.ScaledLineHeight(Pt(12)).Twips.ShouldBe(406);
        scaled.ScaledLineHeight(Pt(12)).Twips.ShouldNotBe(412);
    }

    [Fact]
    public void OnlyADocumentThatCameFromWordAsksForTheScale()
    {
        // Measured, not reasoned: the same two lines of WenQuanYi Zen Hei at 12 pt are 406 twips apart
        // when LibreOffice reads them from a .docx and 325 apart when it reads them from a .fodt.
        // `MS_WORD_COMP_GRID_METRICS` is a document compatibility setting that defaults to false —
        // `DocumentSettingManager` initialises `mbMsWordCompGridMetrics(false)` — and ODF carries its
        // own value for it.
        MetricGrid.Reference.ScalesEastAsianFaces.ShouldBeFalse();
        MetricGrid.Reference.AsWordDocument().ScalesEastAsianFaces.ShouldBeTrue();

        ZenHei(MetricGrid.Reference).ScaledLineHeight(Pt(12)).Twips.ShouldBe(325);
    }

    [Fact]
    public void TheScaleTravelsWithWhicheverDeviceTheDocumentAskedFor()
    {
        // The flag and the device are independent in the C++ — every caller of
        // `lcl_ApplyCjkHeightAdjustment` passes the reference device it happens to have and asks the
        // document for the flag separately — so a Word document laid out against a printer keeps both.
        MetricGrid printer = MetricGrid.Printer.AsWordDocument();

        printer.Dpi.ShouldBe(600);
        printer.QuantisesAdvances.ShouldBeTrue();
        printer.ScalesEastAsianFaces.ShouldBeTrue();
    }

    // ------------------------------------------------ the other application's rule

    /// <summary>
    /// The 127% scale does not exist on the EditEngine path, and the two rules cannot both apply.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>lcl_ApplyCjkHeightAdjustment</c> has four callers and all four are in
    /// <c>sw/source/core/txtnode/fntcache.cxx</c> — <c>SwFntObj::GetFontAscent</c> and
    /// <c>GetFontHeight</c>. EditEngine has no equivalent, and <c>MS_WORD_COMP_GRID_METRICS</c>
    /// occurs nowhere outside <c>sw/</c>. So the scale is Writer's alone, and the taller-of-two-
    /// roundings in <see cref="LineMetrics.ScaledLineHeight"/> is EditEngine's alone.
    /// </para>
    /// <para>
    /// Measured independently by <c>probes/refdev-01</c>, which put IPAGothic — a face that declares
    /// CP932 — through both devices and found it exact on <b>39 of 39</b> pairs on each with no
    /// scale at all. The two rounds arrived at the same boundary from opposite sides.
    /// </para>
    /// <para>
    /// The expectations are LibreOffice's own, in hundredths of a millimetre, from
    /// <c>probes/refdev-01/impress-extra.txt</c> and <c>calc-extra.txt</c>. Writer draws the same
    /// face at the same size 304 twips tall — 536 hundredths — because it *does* scale it.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheScaleDoesNotExistOnTheEditEnginePath()
    {
        // IPAGothic at 12 pt, EditEngine's grouping and no leading, on each device.
        LineMetrics impress = new(
            1802, 246, 0, LineMetricSource.HorizontalHeader, 2048, MetricGrid.Presentation,
            LeadingAboveText: false, DeclaresEastAsianCodePage: true);
        LineMetrics calc = new(
            1802, 246, 0, LineMetricSource.HorizontalHeader, 2048, MetricGrid.Spreadsheet,
            LeadingAboveText: false, DeclaresEastAsianCodePage: true);

        impress.ScaledAscent(Pt(12)).Mm100.ShouldBe(373);
        impress.ScaledLineHeight(Pt(12)).Mm100.ShouldBe(424);

        calc.ScaledAscent(Pt(12)).Mm100.ShouldBe(374);
        calc.ScaledLineHeight(Pt(12)).Mm100.ShouldBe(423);
    }

    [Fact]
    public void AskingAnEditEngineGridForWordsScaleChangesNothing()
    {
        // Both application rules hang off `LeadingAboveText`, and they are written on the same
        // branch so that they are mutually exclusive by construction. This is that, asserted: even
        // with the flag forced on and the face declaring the code page, nothing moves.
        LineMetrics plain = new(
            1802, 246, 0, LineMetricSource.HorizontalHeader, 2048, MetricGrid.Presentation,
            LeadingAboveText: false, DeclaresEastAsianCodePage: true);
        LineMetrics forced = plain with { Grid = MetricGrid.Presentation.AsWordDocument() };

        forced.ScaledAscent(Pt(12)).ShouldBe(plain.ScaledAscent(Pt(12)));
        forced.ScaledLineHeight(Pt(12)).ShouldBe(plain.ScaledLineHeight(Pt(12)));

        // And the descent, which is `height − ascent`: the hazard the shared branch removes is a
        // grid that scaled one of them and not the other.
        forced.ScaledDescent(Pt(12)).ShouldBe(plain.ScaledDescent(Pt(12)));
        forced.ScaledDescent(Pt(12)).ShouldBeGreaterThan(Length.Zero);
    }

    [Fact]
    public void OnWritersOwnPathTheScaleStillApplies()
        // The positive control for the two above: the same flag, the same face, Writer's engine.
        => new LineMetrics(
                1802, 246, 0, LineMetricSource.HorizontalHeader, 2048,
                MetricGrid.Reference.AsWordDocument(),
                LeadingAboveText: true, DeclaresEastAsianCodePage: true)
            .ScaledLineHeight(Pt(12)).Twips.ShouldBe(304);

    [Theory]
    // The four code pages `lcl_ApplyCjkHeightAdjustment` tests, as bits 17 to 20 of ulCodePageRange1
    // (`include/vcl/fontcapabilities.hxx`:169-172).
    [InlineData(1u << 17, true)]   // CP932, Japanese
    [InlineData(1u << 18, true)]   // CP936, Simplified Chinese
    [InlineData(1u << 19, true)]   // CP949, Korean
    [InlineData(1u << 20, true)]   // CP950, Traditional Chinese
    [InlineData(1u << 16, false)]  // CP874, Thai — adjacent and not one of them
    [InlineData(1u << 21, false)]  // CP1361, Johab — adjacent and not one of them
    [InlineData(1u, false)]        // CP1252, Latin 1
    [InlineData(0u, false)]
    public void TheCodePageBitsAreTheFourWordSinglesOut(uint range, bool declares)
        => new Os2Table(
                Version: 1, TypoAscender: 0, TypoDescender: 0, TypoLineGap: 0,
                WindowsAscent: 0, WindowsDescent: 0, FsSelection: 0, StrikeoutSize: 0,
                StrikeoutPosition: 0, Weight: 400, WidthClass: 5, CapHeight: 0, XHeight: 0,
                CodePageRange1: range)
            .DeclaresEastAsianCodePage.ShouldBe(declares);
}

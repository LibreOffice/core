using XRay.Office.Core.Charts;
using Shouldly;
using static XRay.Office.Spreadsheets.Tests.BiffChartFixture;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What size and weight a BIFF chart's title, axis titles and axis labels are set at.
/// </summary>
/// <remarks>
/// <para>
/// The companion to <see cref="XlsChartFontTests"/>, which takes the <em>family</em> out of the
/// same <c>CHFONT</c> record. The record holds a bare index into the workbook's <c>FONT</c>
/// buffer, so all three answers come from the same two bytes and differ only in which text they
/// dress — which is decided by where the record sits.
/// </para>
/// <para>
/// <strong>Nothing read these, and every chart in the corpus states them.</strong>
/// <see cref="ChartPlot"/> kept chart2's own defaults — a 13 pt regular title, a 9 pt regular
/// axis title — and a census of the corpus's fifteen BIFF chart substreams finds
/// <em>every one</em> stating a title <c>CHFONT</c> that disagrees with that: 14 pt bold, 18 pt
/// bold, 12 pt bold and 10.8 pt regular across the six documents that hold them. Measured on
/// <c>Template Pilot Logbook JAR-FCL V3.0.xls</c> page 17 with <c>pdftohtml -xml</c>, the
/// reference marks the chart title and both axis titles <c>&lt;b&gt;</c> and we marked none of
/// them, and its axis title is 10 pt against our 9.
/// </para>
/// <para>
/// <strong>The fallback is <c>XclImpChText::UpdateText</c></strong>
/// (<c>sc/source/filter/excel/xichart.cxx:1042-1057</c>): a text object keeps its own
/// <c>CHFONT</c> and takes the default text's when it has none.
/// <c>XclImpChChart::GetDefaultText</c> (<c>:3956-3970</c>) gives the title the <em>global</em>
/// default and gives an axis title and an axis label the <em>axes-set</em> default in BIFF8.
/// </para>
/// </remarks>
public sealed class XlsChartTextSizeTests
{
    [Fact]
    public void TheTitlesOwnFontDecidesItsSizeAndWeight()
    {
        ChartPlot plot = Chart(Substream([.. Title(FourteenBold)]));

        plot.TitleSize.Points.ShouldBe(14.0);
        plot.IsTitleBold.ShouldBeTrue();
    }

    /// <summary>
    /// A weight of 400 is a statement of regular, not an absence of one.
    /// </summary>
    /// <remarks>
    /// The size has to move for the case to prove anything — the eight-point font here is
    /// regular <em>and</em> smaller than the 13 pt default, so a reader that ignored the record
    /// entirely would fail on the size while agreeing on the weight.
    /// </remarks>
    [Fact]
    public void ARegularFontLeavesTheTitleUnbolded()
    {
        ChartPlot plot = Chart(Substream([.. Title(EightRegular)]));

        plot.TitleSize.Points.ShouldBe(8.0);
        plot.IsTitleBold.ShouldBeFalse();
    }

    /// <summary>A chart stating no font at all keeps chart2's defaults, as before this was read.</summary>
    /// <remarks>
    /// 13 pt and 9 pt are LibreOffice's own — <c>chart2/source/model/main/Title.cxx</c> — and are
    /// what <see cref="ChartPlot"/> already carried. This case is what makes the change an
    /// override rather than a replacement.
    /// </remarks>
    [Fact]
    public void AChartStatingNoFontKeepsTheModelsDefaults()
    {
        ChartPlot plot = Chart(Substream([]));

        plot.TitleSize.Points.ShouldBe(13.0);
        plot.AxisTitleSize.Points.ShouldBe(9.0);
        plot.LabelSize.Points.ShouldBe(10.0);
        plot.IsTitleBold.ShouldBeFalse();
        plot.IsAxisTitleBold.ShouldBeFalse();
    }

    /// <summary>An axis title takes its own <c>CHFONT</c>, which is a different record.</summary>
    /// <remarks>
    /// The two are set from different fonts here on purpose: the corpus's charts state one
    /// weight for the title and another for the axis titles —
    /// <c>Template Pilot Logbook JAR-FCL V3.0.xls</c> is 12 pt bold against 10 pt bold — and a
    /// reader that filed both under one index would draw the axis titles at the title's size.
    /// </remarks>
    [Fact]
    public void AnAxisTitleTakesItsOwnFontAndNotTheTitles()
    {
        ChartPlot plot = Chart(Substream([.. Title(FourteenBold), .. AxisTitle(EightRegular)]));

        plot.TitleSize.Points.ShouldBe(14.0);
        plot.AxisTitleSize.Points.ShouldBe(8.0);
        plot.IsTitleBold.ShouldBeTrue();
        plot.IsAxisTitleBold.ShouldBeFalse();
    }

    /// <summary>
    /// A <c>CHTEXT</c> whose <c>CHOBJECTLINK</c> names nothing dresses nothing.
    /// </summary>
    /// <remarks>
    /// Excel writes an unlinked <c>CHTEXT</c> constantly — it is the placeholder for every object
    /// that could carry a title and does not. A reader taking the first <c>CHFONT</c> under any
    /// <c>CHTEXT</c> would give the title this font, which on the corpus is the legend's.
    /// </remarks>
    [Fact]
    public void AnUnlinkedTextBlockSetsNoTitleFont()
    {
        Chart(Substream([.. Group(ChText, new byte[32], Record(ChFont, Word(FourteenBold)))]))
            .TitleSize.Points.ShouldBe(13.0);
    }

    /// <summary>
    /// The title falls back to the global default text's font, not to the axes-set one.
    /// </summary>
    /// <remarks>
    /// The two disagree here, which no corpus document does. It pins <c>GetDefaultText</c>'s
    /// table rather than an outcome.
    /// </remarks>
    [Fact]
    public void ATitleWithNoFontFallsBackToTheGlobalDefaultText()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. DefaultText(GlobalDefaultText, FourteenBold),
            .. DefaultText(AxesSetDefaultText, EightRegular),
            .. Title(null),
        ]));

        plot.TitleSize.Points.ShouldBe(14.0);
        plot.IsTitleBold.ShouldBeTrue();
    }

    /// <summary>And an axis title falls back to the axes-set one, which BIFF8 writes.</summary>
    [Fact]
    public void AnAxisTitleWithNoFontFallsBackToTheAxesSetDefaultText()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. DefaultText(GlobalDefaultText, FourteenBold),
            .. DefaultText(AxesSetDefaultText, EightRegular),
            .. AxisTitle(null),
        ]));

        plot.AxisTitleSize.Points.ShouldBe(8.0);
        plot.IsAxisTitleBold.ShouldBeFalse();
    }

    /// <summary>The axis labels take the <c>CHFONT</c> that sits inside the <c>CHAXIS</c>.</summary>
    [Fact]
    public void TheAxisLabelsTakeTheFontOnTheAxis()
    {
        ChartPlot plot = Chart(Substream([.. Axis(CategoryAxisType, FourteenBold)]));

        plot.LabelSize.Points.ShouldBe(14.0);
        plot.IsLabelBold.ShouldBeTrue();
    }

    /// <summary>
    /// When the two axes disagree the category axis' font is the one kept.
    /// </summary>
    /// <remarks>
    /// <see cref="ChartPlot"/> holds one label size for both axes and BIFF gives each its own, so
    /// one has to be chosen. The category axis is it because its labels are what
    /// <see cref="ChartAxisLabels"/> tests for collision — the size they are measured at decides
    /// whether the axis rotates or thins and therefore how many labels a page shows, while the
    /// value axis' size only widens a band. Fourteen of the corpus's fifteen substreams state the
    /// same size on both and the choice is moot;
    /// <c>2012-GA-Survey-Chapter-6-Tables-16Dec2013-V2.xls</c> states 8 pt and 10 pt and is the
    /// one this case stands for. Recorded rather than resolved — resolving it means a second
    /// property on the model and a reason from more than one file.
    /// </remarks>
    [Fact]
    public void TheCategoryAxisFontWinsWhenTheTwoAxesDisagree()
    {
        Chart(Substream([.. Axis(ValueAxisType, EightRegular), .. Axis(CategoryAxisType, FourteenBold)]))
            .LabelSize.Points.ShouldBe(14.0);
    }

    /// <summary>And it wins whichever order the two axes come in.</summary>
    /// <remarks>
    /// BIFF writes the category axis first, so a reader keeping whichever came first is right on
    /// every corpus file and wrong in principle. This is the case that separates the two rules.
    /// </remarks>
    [Fact]
    public void TheCategoryAxisFontWinsWhicheverOrderTheAxesCome()
    {
        Chart(Substream([.. Axis(CategoryAxisType, FourteenBold), .. Axis(ValueAxisType, EightRegular)]))
            .LabelSize.Points.ShouldBe(14.0);
    }

    /// <summary>An axis stating no font of its own uses the value axis', when only that states one.</summary>
    [Fact]
    public void TheValueAxisFontIsUsedWhenTheCategoryAxisStatesNone()
    {
        Chart(Substream([.. Axis(ValueAxisType, EightRegular)])).LabelSize.Points.ShouldBe(8.0);
    }

    /// <summary>A <c>CHTEXT</c> linked as the chart's title, optionally carrying a font.</summary>
    private static byte[] Title(ushort? font) => LinkedText(TitleLink, font);

    /// <summary>A <c>CHTEXT</c> linked as the value axis' title.</summary>
    private static byte[] AxisTitle(ushort? font) => LinkedText(ValueAxisTitleLink, font);

    /// <summary>
    /// A <c>CHTEXT</c> with a string and a <c>CHOBJECTLINK</c>, in the order Excel writes them.
    /// </summary>
    /// <remarks>
    /// The font comes first and the link last, which is the whole reason the reader has to hold
    /// the index until the group closes: when the <c>CHFONT</c> is read it is not yet known what
    /// the text is. A title with no string is dropped by
    /// <c>lclFinalizeTitle</c> (<c>xichart.cxx:1261-1274</c>) and so is its font, which is why
    /// every one of these carries one.
    /// </remarks>
    private static byte[] LinkedText(ushort link, ushort? font)
    {
        List<byte[]> children = [];
        if (font is { } index) children.Add(Record(ChFont, Word(index)));

        children.Add(Record(ChString, [0, 0, 5, 0, .. "Title"u8]));
        children.Add(Record(ChObjectLink, [.. Word(link), 0, 0, 0, 0]));

        return Group(ChText, new byte[32], [.. children]);
    }

    /// <summary>A <c>CHDEFAULTTEXT</c> and the <c>CHTEXT</c> it heads, carrying one font.</summary>
    private static byte[] DefaultText(ushort id, ushort font) =>
    [
        .. Record(ChDefaultText, Word(id)),
        .. Group(ChText, new byte[32], Record(ChFont, Word(font))),
    ];

    /// <summary>A <c>CHAXIS</c> of one dimension carrying its own <c>CHFONT</c>.</summary>
    private static byte[] Axis(ushort type, ushort font)
        => Group(ChAxis, [.. Word(type), .. new byte[16]], Record(ChFont, Word(font)));

    // The two FONT indices SizedFonts adds, with the buffer's phantom fourth entry counted.
    private const ushort FourteenBold = 6;
    private const ushort EightRegular = 7;

    private const ushort TitleLink = 1;
    private const ushort ValueAxisTitleLink = 2;

    private const ushort CategoryAxisType = 0;
    private const ushort ValueAxisType = 1;
}

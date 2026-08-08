using Paperless.Core.Charts;
using Paperless.Core.Graphics;
using Shouldly;
using static Paperless.Spreadsheets.Tests.BiffChartFixture;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// What colour a BIFF chart's frame, its plot wall and its series are filled in.
/// </summary>
/// <remarks>
/// <para>
/// The BIFF chart path drew no fills whatever before this: measured on
/// <c>EHEST-Pre-departure-checklist…xls</c> page 8, <strong>zero fill operators against the
/// reference's seven</strong>, while the OOXML chart path on the same corpus emits 20 against 21
/// on a comparable page. <see cref="ChartPlot"/> already carried
/// <see cref="ChartPlot.Background"/>, <see cref="ChartPlot.PlotBackground"/> and each series'
/// fill; only the reader was missing, so a bar was stroked and never filled.
/// </para>
/// <para>
/// Which object a format record dresses is decided entirely by where it sits, exactly as a
/// <c>CHFONT</c>'s meaning is — a <c>CHFRAME</c> under <c>CHCHART</c> is the chart's own
/// background and the same record under <c>CHAXESSET</c> is the plot area's wall. That is what
/// most of these cases pin.
/// </para>
/// </remarks>
public sealed class XlsChartFormatTests
{
    [Fact]
    public void TheChartsOwnFrameIsItsBackground()
    {
        Chart(Substream([.. Frame(Red)])).Background.ShouldBe(Colour.FromRgb(0xFF0000));
    }

    [Fact]
    public void TheAxesSetsFrameIsThePlotAreasWall()
    {
        ChartPlot plot = Chart(Substream([.. Frame(Red), .. AxesSet(Frame(Green))]));

        plot.Background.ShouldBe(Colour.FromRgb(0xFF0000));
        plot.PlotBackground.ShouldBe(Colour.FromRgb(0x00FF00));
    }

    /// <summary>
    /// A legend's frame is neither, though it sits in the same tree and states the same record.
    /// </summary>
    /// <remarks>
    /// The legend's frame here is the only <c>CHAREAFORMAT</c> in the file, so a reader that took
    /// any frame it met would paint the whole chart green.
    /// </remarks>
    [Fact]
    public void ALegendsFrameIsNeitherTheBackgroundNorTheWall()
    {
        ChartPlot plot = Chart(Substream([.. Group(ChLegend, new byte[20], Frame(Green))]));

        plot.Background.ShouldBeNull();
        plot.PlotBackground.ShouldBeNull();
    }

    /// <summary>A pattern of <c>EXC_PATT_NONE</c> is a stated absence, not an absent statement.</summary>
    [Fact]
    public void AFrameStatingNoPatternIsNotFilled()
    {
        Chart(Substream([.. Frame(Red, pattern: 0)])).Background.ShouldBeNull();
    }

    /// <summary>
    /// An automatic area names no colour, so the layout's own default stands.
    /// </summary>
    /// <remarks>
    /// Excel's automatic colour comes from a chart palette this reader does not have. The corpus
    /// makes that safe rather than right: a record-level census of the six OLE2 workbooks on the
    /// sheets track holding a chart substream finds 114 <c>CHAREAFORMAT</c> records and
    /// <em>none</em> of them automatic. This case pins the behaviour so the limit is visible
    /// rather than latent.
    /// </remarks>
    [Fact]
    public void AnAutomaticAreaNamesNothing()
    {
        Chart(Substream([.. Frame(Red, automatic: true)])).Background.ShouldBeNull();
    }

    [Fact]
    public void ASeriesTakesTheFillAndLineItsDataFormatStates()
    {
        ChartPlot plot = Chart(
            Substream([.. Series(AreaFormat(Green), LineFormat(Red))]), withData: true);

        plot.Series.Count.ShouldBe(1);
        plot.Series[0].Fill.ShouldBe(Colour.FromRgb(0x00FF00));
        plot.Series[0].Line.ShouldBe(Colour.FromRgb(0xFF0000));
    }

    /// <summary>Each series keeps its own colours rather than the last one winning.</summary>
    [Fact]
    public void TwoSeriesKeepTheirOwnFills()
    {
        ChartPlot plot = Chart(
            Substream([.. Series(AreaFormat(Green)), .. Series(AreaFormat(Red))]), withData: true);

        plot.Series.Count.ShouldBe(2);
        plot.Series[0].Fill.ShouldBe(Colour.FromRgb(0x00FF00));
        plot.Series[1].Fill.ShouldBe(Colour.FromRgb(0xFF0000));
    }

    /// <summary>
    /// A <c>CHESCHERFORMAT</c> supersedes the <c>CHAREAFORMAT</c> beside it.
    /// </summary>
    /// <remarks>
    /// <c>XclImpChFrameBase::ConvertAreaBase</c> carries the rule as a comment —
    /// <em>"CHESCHERFORMAT overrides CHAREAFORMAT (even if it is auto)"</em> — and it is what
    /// separates a right colour from a plausible one on the corpus. All nine chart substreams of
    /// <c>EHEST…xls</c> state their filled series at palette indices 24, 10 and 13, and the
    /// reference draws <c>#6699FF</c>, <c>#FF0000</c>, <c>#FFFF00</c>: the first is the Escher
    /// override and the other two are the palette, so reading only the palette looks right on two
    /// thirds of the marks.
    /// </remarks>
    [Fact]
    public void AnEscherFormatOverridesTheAreaFormatBesideIt()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. Group(ChFrame, [], AreaFormat(Green), EscherFormat(EscherBlue)),
        ]));

        plot.Background.ShouldBe(Colour.FromRgb(0x6699FF));
    }

    /// <summary>
    /// And it overrides it in the other order too, which the records do not guarantee.
    /// </summary>
    /// <remarks>
    /// Excel writes the <c>CHAREAFORMAT</c> first, so a reader keeping whichever came first is
    /// right on every corpus file and wrong in principle. This case is the one that separates
    /// "the Escher format wins" from "the first record wins".
    /// </remarks>
    [Fact]
    public void TheEscherFormatWinsWhicheverOrderTheRecordsCome()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. Group(ChFrame, [], EscherFormat(EscherBlue), AreaFormat(Green)),
        ]));

        plot.Background.ShouldBe(Colour.FromRgb(0x6699FF));
    }

    /// <summary>An Escher format asserting no fill leaves the area format's answer standing.</summary>
    [Fact]
    public void AnEscherFormatThatIsNotFilledDoesNotOverride()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. Group(ChFrame, [], AreaFormat(Green), EscherFormat(EscherBlue, filled: false)),
        ]));

        plot.Background.ShouldBe(Colour.FromRgb(0x00FF00));
    }

    /// <summary>
    /// An indexed or scheme colour word names a table the substream does not carry.
    /// </summary>
    /// <remarks>
    /// The top byte of an MSO colour states its kind. Bit 1 — which is what these files set — is a
    /// literal Excel merely says came from its palette, and is taken; a scheme index is not, and
    /// the <c>CHAREAFORMAT</c> beside it is then the best answer available.
    /// </remarks>
    [Fact]
    public void ASchemeColourWordIsNotTakenAsALiteral()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. Group(ChFrame, [], AreaFormat(Green), EscherFormat(0x08000002)),
        ]));

        plot.Background.ShouldBe(Colour.FromRgb(0x00FF00));
    }

    /// <summary>A chart stating no format at all keeps every colour null, as before this was read.</summary>
    [Fact]
    public void AChartStatingNoFormatsFillsNothing()
    {
        ChartPlot plot = Chart(Substream([.. Series()]), withData: true);

        plot.Background.ShouldBeNull();
        plot.PlotBackground.ShouldBeNull();
        plot.Series.ShouldAllBe(series => series.Fill == null && series.Line == null);
    }

    /// <summary>A <c>CHFRAME</c> holding the frame's records, with one area format inside.</summary>
    private static byte[] Frame(ushort colourIndex, ushort pattern = 1, bool automatic = false)
        => Group(ChFrame, [], AreaFormat(colourIndex, pattern, automatic));

    /// <summary>A <c>CHAXESSET</c> wrapping whatever belongs to the plot area.</summary>
    private static byte[] AxesSet(params byte[][] children) => Group(ChAxesSet, new byte[18], children);

    /// <summary>
    /// One series with a resolvable value, and its <c>CHDATAFORMAT</c>'s format records.
    /// </summary>
    /// <remarks>
    /// The series has to carry a value link that resolves or <c>BuildSeries</c> drops it — a
    /// series with no numbers is not drawn, so it could carry no fill either. Hence
    /// <c>withData</c> on the fixture, which puts a worksheet holding one number ahead of the
    /// chart sheet and the <c>EXTERNSHEET</c> that lets a <c>tRef3d</c> reach it.
    /// </remarks>
    private static byte[] Series(params byte[][] formats)
        => Group(ChSeries, new byte[8], SeriesLink(),
            Group(ChDataFormat, [.. Word(0xFFFF), .. Word(0), .. Word(0), .. Word(0)], formats));

    // Palette indices of the default BIFF8 table, which the fixture writes no PALETTE over.
    private const ushort Red = 2;
    private const ushort Green = 3;

    /// <summary>
    /// The MSO colour word <c>EHEST…xls</c> states for its first series — <c>#6699FF</c>.
    /// </summary>
    /// <remarks>
    /// Written blue-green-red with a top byte of 2, which is the form every one of the corpus's
    /// twenty <c>CHESCHERFORMAT</c> fill words takes.
    /// </remarks>
    private const uint EscherBlue = 0x02FF9966;
}

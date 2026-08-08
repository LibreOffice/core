using Paperless.Core.Charts;
using Paperless.Core.Diagnostics;
using Paperless.Spreadsheets.MsBinary;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// The three pieces that turn a BIFF chart's <c>CHSOURCELINK</c> into plotted numbers: the token
/// decoder, the <c>ixti</c> table, and the series the two of them together produce.
/// </summary>
/// <remarks>
/// <para>
/// Asserted from bytes because no round trip produces the case. Calc exports a chart to
/// <c>.xls</c> through its own writer, whose links it also reads, so a fixture generated that way
/// proves only that we agree with ourselves; and the two corpus workbooks that exercise this are
/// binary files a unit test cannot carry.
/// </para>
/// <para>
/// The numbers in <see cref="ASeriesPlotsTheCellsItsLinkNames"/> are the shape
/// <c>EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls</c> writes: a value link into one
/// column and a category link into another, both on a sheet the chart is not embedded in.
/// </para>
/// </remarks>
public sealed class XlsChartSourceTests
{
    /// <summary>Builds a record stream the way the chart tests next door do.</summary>
    private sealed class Stream
    {
        private readonly List<byte> _bytes = [];

        public Stream Record(ushort id, params byte[] body)
        {
            _bytes.AddRange([(byte)(id & 0xFF), (byte)(id >> 8),
                             (byte)(body.Length & 0xFF), (byte)(body.Length >> 8)]);
            _bytes.AddRange(body);
            return this;
        }

        public Stream Begin() => Record(0x1033);

        public Stream End() => Record(0x1034);

        /// <summary>A <c>CHSERIES</c> stating numeric categories and values.</summary>
        public Stream Series(int count)
            => Record(0x1003, 1, 0, 1, 0,
                (byte)(count & 0xFF), (byte)(count >> 8),
                (byte)(count & 0xFF), (byte)(count >> 8),
                1, 0, 0, 0);

        /// <summary>A <c>CHSOURCELINK</c> naming a rectangle with a BIFF8 <c>tArea3d</c>.</summary>
        public Stream Link(int destination, int ixti, int firstRow, int lastRow, int column)
            => Record(0x1051,
                (byte)destination, 2, 0, 0, 0, 0,
                11, 0,
                0x3B,
                (byte)(ixti & 0xFF), (byte)(ixti >> 8),
                (byte)(firstRow & 0xFF), (byte)(firstRow >> 8),
                (byte)(lastRow & 0xFF), (byte)(lastRow >> 8),
                (byte)(column & 0xFF), (byte)(column >> 8),
                (byte)(column & 0xFF), (byte)(column >> 8));

        /// <summary>A <c>CHSOURCELINK</c> of a type that carries no formula at all.</summary>
        public Stream EmptyLink(int destination)
            => Record(0x1051, (byte)destination, 1, 0, 0, 0, 0);

        /// <summary>A <c>CHSTRING</c>: two unused bytes, an eight-bit count, a flags byte, text.</summary>
        public Stream Text(string text)
        {
            List<byte> body = [0, 0, (byte)text.Length, 1];
            foreach (char c in text) body.AddRange([(byte)(c & 0xFF), (byte)(c >> 8)]);
            return Record(0x100D, [.. body]);
        }

        public XlsChartBuilder Build()
        {
            XlsChartBuilder builder = new();
            List<Diagnostic> diagnostics = [];
            BiffRecordReader stream = new([.. _bytes], diagnostics);

            while (stream.MoveNext()) builder.Read(stream.RecordId, stream);

            return builder;
        }
    }

    private static Stream Chart() => new Stream().Record(0x1002, new byte[16]).Begin();

    /// <summary>A table in which <c>ixti</c> 0 names sheet 3 of this workbook.</summary>
    private static XlsExternSheets SelfSheets(params (int Supbook, int First, int Last)[] entries)
    {
        List<byte> supbook = [8, 0, 0x01, 0x04];
        List<byte> table = [(byte)entries.Length, 0];
        foreach ((int supbookIndex, int first, int last) in entries)
        {
            table.AddRange([(byte)supbookIndex, (byte)(supbookIndex >> 8),
                            (byte)(first & 0xFF), (byte)(first >> 8),
                            (byte)(last & 0xFF), (byte)(last >> 8)]);
        }

        XlsExternSheets sheets = new();
        sheets.ReadSupBook(Reader(0x01AE, [.. supbook]));
        sheets.ReadExternSheet(Reader(0x0017, [.. table]));
        return sheets;
    }

    private static BiffRecordReader Reader(ushort id, byte[] body)
    {
        List<byte> bytes = [(byte)(id & 0xFF), (byte)(id >> 8),
                            (byte)(body.Length & 0xFF), (byte)(body.Length >> 8)];
        bytes.AddRange(body);

        List<Diagnostic> diagnostics = [];
        BiffRecordReader reader = new([.. bytes], diagnostics);
        reader.MoveNext();
        return reader;
    }

    /// <summary>
    /// A series plots the cells its value link names, labelled by the cells its category link does.
    /// </summary>
    [Fact]
    public void ASeriesPlotsTheCellsItsLinkNames()
    {
        XlsChartBuilder builder = Chart()
            .Series(3)
            .Begin()
                .EmptyLink(0)
                .Link(destination: 1, ixti: 0, firstRow: 99, lastRow: 101, column: 23)
                .Link(destination: 2, ixti: 0, firstRow: 99, lastRow: 101, column: 17)
                .EmptyLink(3)
            .End()
            .End()
            .Build();

        XlsChartData data = new();
        foreach (XlsChartRange range in builder.Ranges()) data.Want(3, range);

        data.Offer(3, 99, 23, 10.0, "10");
        data.Offer(3, 100, 23, 20.0, "20");
        data.Offer(3, 101, 23, 30.0, "30");
        data.Offer(3, 99, 17, 0.0, "Jan");
        data.Offer(3, 100, 17, 0.0, "Feb");
        data.Offer(3, 101, 17, 0.0, "Mar");

        ChartPlot plot = builder.Build(data, SelfSheets((0, 3, 3)), ownSheet: 0).ShouldNotBeNull();

        plot.Series.Count.ShouldBe(1);
        plot.Series[0].Values.ShouldBe([10.0, 20.0, 30.0]);
        plot.Categories.ShouldBe(["Jan", "Feb", "Mar"]);
    }

    /// <summary>
    /// A chart built with nothing gathered draws no series rather than an empty one.
    /// </summary>
    /// <remarks>
    /// The distinction matters to the axis and not only to the marks: a series of three nulls
    /// pins the value axis to the 0–12 default that a plot with no numbers gets, which is exactly
    /// the defect the source links were read to remove.
    /// </remarks>
    [Fact]
    public void ASeriesWhoseCellsWereNotGatheredIsNotDrawn()
    {
        XlsChartBuilder builder = Chart()
            .Series(3)
            .Begin()
                .Link(destination: 1, ixti: 0, firstRow: 99, lastRow: 101, column: 23)
            .End()
            .End()
            .Build();

        builder.Build(new XlsChartData(), SelfSheets((0, 3, 3)), ownSheet: 0)
            .ShouldNotBeNull().Series.ShouldBeEmpty();

        builder.Build(null, null, ownSheet: 0).ShouldNotBeNull().Series.ShouldBeEmpty();
    }

    /// <summary>
    /// The rectangles a chart reports are what the workbook goes and reads, sheet included.
    /// </summary>
    [Fact]
    public void TheRangesReportedCarryTheSheetTheyName()
    {
        XlsChartBuilder builder = Chart()
            .Series(2)
            .Begin()
                .Link(destination: 1, ixti: 1, firstRow: 1, lastRow: 615, column: 2)
                .Link(destination: 2, ixti: 1, firstRow: 1, lastRow: 615, column: 0)
            .End()
            .End()
            .Build();

        List<XlsChartRange> ranges = [.. builder.Ranges()];
        ranges.Count.ShouldBe(2);
        ranges.ShouldContain(new XlsChartRange(1, 1, 615, 2, 2));
        ranges.ShouldContain(new XlsChartRange(1, 1, 615, 0, 0));

        // ixti 1, not 0: an EXTERNSHEET is a table and a chart routinely uses a later entry.
        XlsExternSheets sheets = SelfSheets((0, 0, 0), (0, 7, 7));
        sheets.SheetOf(1).ShouldBe(7);
        sheets.SheetOf(0).ShouldBe(0);
    }

    /// <summary>A series names itself with the <c>CHSTRING</c> that follows its title link.</summary>
    /// <remarks>
    /// The same record inside a <c>CHTEXT</c> is a chart or axis title, so the two are told apart
    /// by the innermost open container and by whether a title link asked for one.
    /// </remarks>
    [Fact]
    public void ASeriesTakesItsNameFromTheStringAfterItsTitleLink()
    {
        XlsChartBuilder builder = Chart()
            .Series(2)
            .Begin()
                .EmptyLink(0)
                .Text("Total simulator")
                .Link(destination: 1, ixti: 0, firstRow: 1, lastRow: 2, column: 2)
            .End()
            .End()
            .Build();

        XlsChartData data = new();
        data.Want(0, new XlsChartRange(0, 1, 2, 2, 2));
        data.Offer(0, 1, 2, 1.0, "1");
        data.Offer(0, 2, 2, 2.0, "2");

        ChartPlot plot = builder.Build(data, SelfSheets((0, 0, 0)), ownSheet: 0).ShouldNotBeNull();
        plot.Series.Count.ShouldBe(1);
        plot.Series[0].Name.ShouldBe("Total simulator");
    }

    /// <summary>
    /// A reference into another workbook resolves to nothing rather than to this workbook's
    /// sheet of the same number.
    /// </summary>
    /// <remarks>
    /// The failure this prevents is silent and plausible: an external link would plot whatever
    /// happens to be in the same rows and columns of a local sheet, and nothing downstream could
    /// tell that the numbers came from the wrong file.
    /// </remarks>
    [Fact]
    public void AReferenceIntoAnotherWorkbookResolvesToNothing()
    {
        XlsExternSheets sheets = new();
        sheets.ReadSupBook(Reader(0x01AE, [2, 0, 0x01, 0x04]));               // this workbook
        sheets.ReadSupBook(Reader(0x01AE, [2, 0, 8, 0, 0, 0, 0, 0, 0, 0]));   // another one
        sheets.ReadExternSheet(Reader(0x0017, [2, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0]));

        sheets.SheetOf(0).ShouldBe(1);
        sheets.SheetOf(1).ShouldBeNull();
    }

    /// <summary>A sheet the workbook has deleted is not a sheet a chart can plot.</summary>
    [Fact]
    public void ADeletedSheetResolvesToNothing()
        => SelfSheets((0, 0xFFFF, 0xFFFF)).SheetOf(0).ShouldBeNull();

    /// <summary>
    /// Several <c>EXTERNSHEET</c> records combine with the later one's entries first.
    /// </summary>
    /// <remarks>
    /// The format allows one record and a third-party writer emits several; Excel inserts each
    /// new record's entries before the ones already read, and LibreOffice reproduces that
    /// (i#104057, <c>XclImpLinkManagerImpl::ReadExternsheet</c>). Appending instead shifts every
    /// <c>ixti</c> in the file by the size of the first table.
    /// </remarks>
    [Fact]
    public void ALaterExternSheetRecordGoesInFront()
    {
        XlsExternSheets sheets = new();
        sheets.ReadSupBook(Reader(0x01AE, [8, 0, 0x01, 0x04]));
        sheets.ReadExternSheet(Reader(0x0017, [1, 0, 0, 0, 5, 0, 5, 0]));
        sheets.ReadExternSheet(Reader(0x0017, [1, 0, 0, 0, 2, 0, 2, 0]));

        sheets.SheetOf(0).ShouldBe(2);
        sheets.SheetOf(1).ShouldBe(5);
    }

    /// <summary>
    /// The token decoder reads the one reference a chart link holds, in both generations.
    /// </summary>
    /// <remarks>
    /// BIFF8 gives a column two bytes and keeps the relative-reference flags in its top two bits;
    /// BIFF5 gives it one and keeps them in the row instead. Reading a BIFF5 area with the BIFF8
    /// layout produces a rectangle that is plausible, wrong, and usually enormous.
    /// </remarks>
    [Fact]
    public void TheTokenDecoderReadsBothGenerations()
    {
        // tArea3d, BIFF8: ixti 0, rows 1..615, column 2.
        byte[] biff8 = [0x3B, 0, 0, 1, 0, 0x67, 0x02, 2, 0, 2, 0];
        XlsChartFormula.Read(Reader(0x1051, biff8), biff8.Length, BiffVersion.Biff8)
            .ShouldBe(new XlsChartRange(0, 1, 615, 2, 2));

        // tRef3d, BIFF8: ixti 4, row 9, column 3.
        byte[] reference = [0x3A, 4, 0, 9, 0, 3, 0];
        XlsChartFormula.Read(Reader(0x1051, reference), reference.Length, BiffVersion.Biff8)
            .ShouldBe(new XlsChartRange(4, 9, 9, 3, 3));

        // tArea3d, BIFF5: an external index and eight reserved bytes, then one-byte columns.
        byte[] biff5 = [0x3B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0x67, 0x02, 2, 2];
        XlsChartFormula.Read(Reader(0x1051, biff5), biff5.Length, BiffVersion.Biff5)
            .ShouldBe(new XlsChartRange(0, 1, 615, 2, 2));
    }

    /// <summary>
    /// A link whose formula is anything but a single reference names no rectangle.
    /// </summary>
    /// <remarks>
    /// A guess here is worse than nothing: the cache would be read out of a rectangle the file
    /// never mentioned and the chart would plot it without complaint.
    /// </remarks>
    [Fact]
    public void AnExpressionNamesNoRectangle()
    {
        byte[] sum = [0x1E, 1, 0, 0x1E, 2, 0, 0x03];
        XlsChartFormula.Read(Reader(0x1051, sum), sum.Length, BiffVersion.Biff8).ShouldBeNull();

        byte[] truncated = [0x3B, 0, 0, 1, 0];
        XlsChartFormula.Read(Reader(0x1051, truncated), truncated.Length, BiffVersion.Biff8)
            .ShouldBeNull();
    }

    /// <summary>
    /// A category axis labels every category and lets the labels overlap.
    /// </summary>
    /// <remarks>
    /// <c>XclImpChLabelRange::Convert</c> sets <c>TextOverlap</c> and <c>TextBreak</c> from
    /// <c>mnLabelFreq == 1</c>, and <c>XclChLabelRange</c>'s constructor defaults that field to 1
    /// — so an axis that states nothing behaves the same as an axis that states the default, and
    /// only a stated frequency above one hands the labels to the collision rules.
    /// </remarks>
    [Theory]
    [InlineData(1, true)]
    [InlineData(2, false)]
    public void TheCategoryAxisOverlapsItsLabelsUnlessTheFrequencySaysOtherwise(
        int frequency, bool expected)
    {
        ChartPlot plot = Chart()
            .Record(0x101D, 0, 0)
            .Begin()
                .Record(0x1020, 1, 0, (byte)frequency, 0, 1, 0, 0, 0)
            .End()
            .End()
            .Build()
            .Build(null, null, ownSheet: 0)
            .ShouldNotBeNull();

        plot.CategoryAxisText.OverlapAllowed.ShouldBe(expected);
        plot.CategoryAxisText.LineBreakAllowed.ShouldBe(expected);
        plot.CategoryAxisText.Stagger.ShouldBe(ChartLabelStagger.SideBySide);
    }

    /// <summary>An axis with no <c>CHLABELRANGE</c> at all behaves as one stating the default.</summary>
    [Fact]
    public void AnAxisStatingNothingStillOverlapsItsLabels()
        => Chart().End().Build().Build(null, null, ownSheet: 0)
            .ShouldNotBeNull().CategoryAxisText.OverlapAllowed.ShouldBeTrue();

    /// <summary>
    /// A <c>CHLABELRANGE</c> under the value axis says nothing about the category labels.
    /// </summary>
    /// <remarks>
    /// BIFF numbers its axes by dimension and writes the record under whichever one carries it;
    /// taking it from the value axis would let a chart's own Y settings decide how its X labels
    /// are set.
    /// </remarks>
    [Fact]
    public void TheValueAxisDoesNotDecideHowCategoryLabelsAreSet()
        => Chart()
            .Record(0x101D, 1, 0)
            .Begin().Record(0x1020, 1, 0, 5, 0, 1, 0, 0, 0).End()
            .End()
            .Build()
            .Build(null, null, ownSheet: 0)
            .ShouldNotBeNull().CategoryAxisText.OverlapAllowed.ShouldBeTrue();
}

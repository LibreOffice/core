using Paperless.Core.Charts;
using Paperless.Core.Diagnostics;
using Paperless.Spreadsheets.MsBinary;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A BIFF chart substream, read into the same <see cref="ChartPlot"/> the other two families use.
/// </summary>
/// <remarks>
/// <para>
/// A chart substream is a tree written flat: every container record is followed by
/// <c>CHBEGIN</c>, its children and <c>CHEND</c>, so what a record means depends on which
/// container it sits in. <c>CHSTRING</c> inside a <c>CHTEXT</c> is a title; the same record under
/// a series' source link is a series name. Getting the nesting wrong reads a chart that is
/// plausible and wrong, which is why it is asserted here from bytes rather than only through a
/// document.
/// </para>
/// <para>
/// The records are hand-built because the corpus reaches this through chart <em>sheets</em>, which
/// no round trip can produce: Calc has no chart-sheet concept and imports one as an ordinary sheet
/// carrying an OLE object, so exporting back to <c>.xls</c> never writes a <c>BOUNDSHEET</c> of
/// type 2. The two corpus files that do — the 2012 GA Survey chapter tables — are measured instead.
/// </para>
/// </remarks>
public sealed class XlsChartRecordTests
{
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

        /// <summary>A <c>CHSTRING</c>: two unused bytes, an eight-bit count, a flags byte, the text.</summary>
        public Stream Text(string text)
        {
            List<byte> body = [0, 0, (byte)text.Length, 1];
            foreach (char c in text) body.AddRange([(byte)(c & 0xFF), (byte)(c >> 8)]);
            return Record(0x100D, [.. body]);
        }

        public Stream Link(ushort target) => Record(0x1027, (byte)(target & 0xFF), (byte)(target >> 8));

        /// <summary>A whole <c>CHTEXT</c> group carrying one string linked to one target.</summary>
        public Stream Titled(string text, ushort target)
            => Record(0x1025, new byte[26]).Begin().Text(text).Link(target).End();

        public ChartPlot? Read()
        {
            XlsChartBuilder builder = new();
            List<Diagnostic> diagnostics = [];
            BiffRecordReader stream = new([.. _bytes], diagnostics);

            while (stream.MoveNext()) builder.Read(stream.RecordId, stream);

            return builder.Build();
        }
    }

    private static Stream Chart()
        => new Stream()
            .Record(0x1002, new byte[16])
            .Begin();

    [Fact]
    public void TheTitlesAreReadAndToldApartByTheirLink()
    {
        ChartPlot plot = Chart()
            .Titled("Chart 8", 1)
            .Titled("Gallons Per Hour", 2)
            .Titled("Aircraft Type", 3)
            .End()
            .Read()
            .ShouldNotBeNull();

        plot.Title.ShouldBe("Chart 8");
        plot.ValueAxisTitle.ShouldBe("Gallons Per Hour");
        plot.CategoryAxisTitle.ShouldBe("Aircraft Type");
    }

    /// <summary>
    /// <c>CHBAR</c>'s low flag turns the chart on its side, and the axes go with it.
    /// </summary>
    /// <remarks><c>EXC_CHBAR_HORIZONTAL</c>, <c>sc/source/filter/inc/xlchart.hxx:350</c>.</remarks>
    [Theory]
    [InlineData(0x0000, ChartBarDirection.Column)]
    [InlineData(0x0001, ChartBarDirection.Bar)]
    public void TheBarDirectionComesFromTheTypeGroup(int flags, ChartBarDirection expected)
    {
        ChartPlot plot = Chart()
            .Record(0x1014, new byte[20])
            .Begin()
            .Record(0x1017, 0, 0, 50, 0, (byte)(flags & 0xFF), (byte)(flags >> 8))
            .End()
            .End()
            .Read()
            .ShouldNotBeNull();

        plot.Kind.ShouldBe(ChartPlotKind.Bar);
        plot.Direction.ShouldBe(expected);
    }

    /// <summary>
    /// A stated axis range is kept and an automatic one is left for the engine to resolve.
    /// </summary>
    /// <remarks>
    /// <c>CHVALUERANGE</c> writes five doubles and then a flags word whose low bits say which of
    /// them Excel computed rather than the author (<c>EXC_CHVALUERANGE_AUTO*</c>). Reading the
    /// numbers without the flags pins every automatic axis to whatever Excel last cached.
    /// </remarks>
    [Fact]
    public void AnAutomaticAxisRangeIsNotTakenLiterally()
    {
        byte[] stated = new byte[42];
        BitConverter.GetBytes(2.0).CopyTo(stated, 0);
        BitConverter.GetBytes(18.0).CopyTo(stated, 8);
        BitConverter.GetBytes(4.0).CopyTo(stated, 16);

        ChartPlot fixed_ = Chart()
            .Record(0x101D, 1, 0).Begin().Record(0x101F, stated).End()
            .End().Read().ShouldNotBeNull();

        fixed_.ValueScale.Minimum.ShouldBe(2.0);
        fixed_.ValueScale.Maximum.ShouldBe(18.0);
        fixed_.ValueScale.MajorUnit.ShouldBe(4.0);

        byte[] automatic = (byte[])stated.Clone();
        automatic[40] = 0x07;   // AUTOMIN | AUTOMAX | AUTOMAJOR

        ChartPlot resolved = Chart()
            .Record(0x101D, 1, 0).Begin().Record(0x101F, automatic).End()
            .End().Read().ShouldNotBeNull();

        resolved.ValueScale.Minimum.ShouldBeNull();
        resolved.ValueScale.Maximum.ShouldBeNull();
        resolved.ValueScale.MajorUnit.ShouldBeNull();
    }

    /// <summary>
    /// A major gridline is a <c>CHAXISLINE</c> of identity 1 inside the axis it belongs to.
    /// </summary>
    /// <remarks>
    /// Identity 0 is the axis line itself, which every axis has and which says nothing about
    /// gridlines (<c>EXC_CHAXISLINE_AXISLINE</c>, <c>xlchart.hxx:447</c>).
    /// </remarks>
    [Fact]
    public void OnlyAMajorGridLineTurnsTheGridOn()
    {
        Chart().Record(0x101D, 1, 0).Begin().Record(0x1021, 0, 0).End().End()
            .Read().ShouldNotBeNull().ValueGrid.ShouldBeNull();

        Chart().Record(0x101D, 1, 0).Begin().Record(0x1021, 1, 0).End().End()
            .Read().ShouldNotBeNull().ValueGrid.ShouldNotBeNull();
    }

    /// <summary>A substream with no <c>CHCHART</c> in it is not a chart.</summary>
    [Fact]
    public void ASubstreamWithoutAChartRecordYieldsNothing()
        => new Stream().Titled("Orphan", 1).Read().ShouldBeNull();
}

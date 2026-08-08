using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.MsBinary;

/// <summary>
/// The BIFF chart record identifiers, named as <c>sc/source/filter/inc/xlchart.hxx</c> names them.
/// </summary>
internal static class BiffChartRecords
{
    public const ushort Chart = 0x1002;
    public const ushort Series = 0x1003;
    public const ushort String = 0x100D;
    public const ushort TypeGroup = 0x1014;
    public const ushort Legend = 0x1015;
    public const ushort Bar = 0x1017;
    public const ushort Line = 0x1018;
    public const ushort Pie = 0x1019;
    public const ushort Area = 0x101A;
    public const ushort Scatter = 0x101B;
    public const ushort Axis = 0x101D;
    public const ushort ValueRange = 0x101F;
    public const ushort AxisLine = 0x1021;
    public const ushort Text = 0x1025;
    public const ushort ObjectLink = 0x1027;
    public const ushort Begin = 0x1033;
    public const ushort End = 0x1034;
    public const ushort RadarLine = 0x103E;
    public const ushort RadarArea = 0x1040;
    public const ushort AxesSet = 0x1041;
    public const ushort SourceLink = 0x1051;

    /// <summary>True for any record this reader acts on or has to track the nesting of.</summary>
    /// <remarks>
    /// The whole 0x08xx and 0x10xx range plus the few sheet records a chart substream reuses.
    /// Deciding by range rather than by a list is deliberate: an unrecognised chart record must
    /// still be counted so that a <c>CHBEGIN</c> attaches to the right header.
    /// </remarks>
    public static bool IsChartRecord(ushort id) => id is >= 0x1000 and <= 0x10FF;
}

/// <summary>
/// Reads a BIFF chart substream into a <see cref="ChartPlot"/>.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A chart substream is a tree written flat.</strong> Every container record is followed
/// by <c>CHBEGIN</c>, its children, and <c>CHEND</c> — so what a record means depends entirely on
/// which container it is inside. <c>CHSTRING</c> under a <c>CHTEXT</c> is a title; the same record
/// under a <c>CHSERIES</c>' source link is a series name. This tracks the open containers on a
/// stack, which is what <c>XclImpChGroupBase::ReadRecordGroup</c>
/// (<c>sc/source/filter/excel/xichart.cxx:397-420</c>) achieves by recursion.
/// </para>
/// <para>
/// <strong>What it reads is what a page shows.</strong> The chart's titles, its axis titles, its
/// type and direction, and the value axis' scale — everything <see cref="ChartPlot"/> needs to be
/// laid out and painted by the same engine the SpreadsheetML and ODF charts already go through.
/// </para>
/// <para>
/// <strong>The series data is resolved through the workbook, not through the substream.</strong>
/// A BIFF series names its values through a <c>CHSOURCELINK</c> whose payload is a formula token
/// array, and only a link of type <c>EXC_CHSRCLINK_WORKSHEET</c> carries one
/// (<c>XclImpChSourceLink::ReadChSourceLink</c>, <c>xichart.cxx</c>); a link stating
/// <c>DIRECTLY</c> or <c>DEFAULT</c> names nothing, and LibreOffice produces a series with an
/// empty range for it and draws no marks. The rectangle that link names is decoded here and the
/// cells behind it are handed in by <see cref="XlsChartData"/>, which the workbook reader fills
/// as it reads its sheets — because the cells a chart plots are routinely on a sheet the chart
/// itself is not embedded in, and may not have been read when the chart is met. Reading the
/// <c>LABEL</c>/<c>NUMBER</c> records that trail the chart substream instead would be reading
/// the wrong thing: those are the sheet's own cells, not the series'.
/// </para>
/// </remarks>
internal sealed class XlsChartBuilder
{
    private readonly Stack<ushort> _open = new();
    private ushort _header;

    private string? _title;
    private string? _categoryTitle;
    private string? _valueTitle;

    private string? _pendingText;
    private int _pendingLink = -1;

    private int _axis = -1;
    private bool _valueGrid;
    private bool _categoryGrid;

    private ChartPlotKind _kind = ChartPlotKind.Bar;
    private ChartBarDirection _direction = ChartBarDirection.Column;
    private bool _stacked;
    private ChartScaleRequest _valueScale;
    private bool _hasType;
    private bool _hasLegend;

    private readonly List<SeriesLinks> _series = [];
    private bool _expectSeriesName;

    /// <summary>The chart's own size, as <c>CHCHART</c> states it.</summary>
    /// <remarks>
    /// In 1/65536 of a point, and the rectangle Excel drew the chart at rather than the one it
    /// prints at. Kept because it is the only statement of the chart's aspect the file makes;
    /// a chart sheet's printed rectangle is computed from the paper instead.
    /// </remarks>
    public DocSize? StatedSize { get; private set; }

    /// <summary>True once a <c>CHCHART</c> record has been seen.</summary>
    public bool HasChart { get; private set; }

    /// <summary>Feeds one record of the chart substream.</summary>
    /// <param name="id">The record identifier.</param>
    /// <param name="stream">Positioned at the record's first byte.</param>
    public void Read(ushort id, BiffRecordReader stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        switch (id)
        {
            case BiffChartRecords.Begin:
                _open.Push(_header);
                return;

            case BiffChartRecords.End:
                Close(_open.Count > 0 ? _open.Pop() : (ushort)0);
                return;

            default:
                break;
        }

        // Only a chart record can head a group; the page-setup and drawing records a chart
        // substream also carries sit outside the tree entirely.
        if (BiffChartRecords.IsChartRecord(id)) _header = id;

        // "The next record" is one record only: the flag is spent whether or not a CHSTRING
        // is what turned up.
        bool expectName = _expectSeriesName;
        _expectSeriesName = false;

        switch (id)
        {
            case BiffChartRecords.Chart:
                HasChart = true;
                stream.Skip(8);
                StatedSize = new DocSize(FixedPoints(stream), FixedPoints(stream));
                break;

            case BiffChartRecords.Text:
                _pendingText = null;
                _pendingLink = -1;
                break;

            case BiffChartRecords.Series:
                _series.Add(new SeriesLinks());
                break;

            case BiffChartRecords.SourceLink when InnermostIs(BiffChartRecords.Series):
                ReadSourceLink(stream);
                break;

            case BiffChartRecords.String when expectName && InnermostIs(BiffChartRecords.Series):
                // A series whose name is typed rather than linked writes it as a CHSTRING
                // immediately after the title link, which is what ReadChSourceLink reaches
                // forward for (xichart.cxx:763-769). Read flat, that is "the next record".
                _expectSeriesName = false;
                stream.Skip(2);
                if (_series.Count > 0) _series[^1].Name = stream.ReadString(eightBitLength: true);
                break;

            case BiffChartRecords.String when Inside(BiffChartRecords.Text):
                // Two unused bytes, an eight-bit character count, then the characters.
                stream.Skip(2);
                _pendingText = stream.ReadString(eightBitLength: true);
                break;

            case BiffChartRecords.ObjectLink when Inside(BiffChartRecords.Text):
                _pendingLink = stream.ReadUInt16();
                break;

            case BiffChartRecords.Axis:
                _axis = stream.ReadUInt16();
                break;

            case BiffChartRecords.AxisLine when Inside(BiffChartRecords.Axis):
                if (stream.ReadUInt16() == MajorGridLine) MarkGrid();
                break;

            case BiffChartRecords.ValueRange:
                ReadValueRange(stream);
                break;

            case BiffChartRecords.Legend:
                _hasLegend = true;
                break;

            case BiffChartRecords.Bar:
                stream.Skip(4);
                ushort bar = stream.ReadUInt16();
                SetKind(ChartPlotKind.Bar);
                _direction = (bar & BarHorizontal) != 0
                    ? ChartBarDirection.Bar
                    : ChartBarDirection.Column;
                _stacked = (bar & (BarStacked | BarPercent)) != 0;
                break;

            case BiffChartRecords.Line:
                _stacked |= (stream.ReadUInt16() & (LineStacked | LinePercent)) != 0;
                SetKind(ChartPlotKind.Line);
                break;

            case BiffChartRecords.Area:
                _stacked |= (stream.ReadUInt16() & (LineStacked | LinePercent)) != 0;
                SetKind(ChartPlotKind.Area);
                break;

            case BiffChartRecords.Pie:
                SetKind(ChartPlotKind.Pie);
                break;

            case BiffChartRecords.Scatter:
                SetKind(ChartPlotKind.Scatter);
                break;

            case BiffChartRecords.RadarLine or BiffChartRecords.RadarArea:
                SetKind(ChartPlotKind.Radar);
                break;

            default:
                break;
        }
    }

    /// <summary>
    /// The chart, or null when the substream held none.
    /// </summary>
    /// <remarks>
    /// A chart with no series is still a chart: LibreOffice draws its frame, its axes, its
    /// gridlines and every title it carries, and so does this. Returning null for one would
    /// lose the page it prints on.
    /// </remarks>
    /// <summary>Every rectangle this chart's series name, for the workbook to gather.</summary>
    /// <remarks>
    /// Reported before the values are wanted rather than asked for when they are, because the
    /// sheet holding them may be read after the sheet the chart is embedded in — see
    /// <see cref="XlsChartData"/>.
    /// </remarks>
    public IEnumerable<XlsChartRange> Ranges()
    {
        foreach (SeriesLinks series in _series)
        {
            if (series.Values is { } values) yield return values;
            if (series.Categories is { } categories) yield return categories;
            if (series.Title is { } title) yield return title;
        }
    }

    /// <summary>
    /// The chart, or null when the substream held none.
    /// </summary>
    /// <param name="data">
    /// The cells the workbook gathered for this chart's links, or null when nothing gathered
    /// them — in which case the chart is built with no series, exactly as before this existed.
    /// </param>
    /// <param name="sheets">Resolves a token's <c>ixti</c> to a sheet index.</param>
    /// <param name="ownSheet">
    /// Which sheet the chart itself sits on, which is what a reference with no sheet part means.
    /// </param>
    public ChartPlot? Build(XlsChartData? data, XlsExternSheets? sheets, int ownSheet)
    {
        if (!HasChart) return null;

        (IReadOnlyList<string?> categories, IReadOnlyList<ChartSeries> series) =
            BuildSeries(data, sheets, ownSheet);

        return new ChartPlot
        {
            Title = _title,
            CategoryAxisTitle = _categoryTitle,
            ValueAxisTitle = _valueTitle,
            Kind = _kind,
            Direction = _direction,
            IsStacked = _stacked,
            Categories = categories,
            Series = series,
            ValueScale = _valueScale,
            ValueGrid = _valueGrid ? GridColour : null,
            CategoryGrid = _categoryGrid ? GridColour : null,
            Legend = _hasLegend ? ChartLegendPosition.Right : ChartLegendPosition.None,
        };
    }

    /// <summary>
    /// Reads one <c>CHSOURCELINK</c>: which part of a series it feeds, and from where.
    /// </summary>
    /// <remarks>
    /// <c>XclImpChSourceLink::ReadChSourceLink</c> (<c>xichart.cxx:744-770</c>). Only a link of
    /// type <c>EXC_CHSRCLINK_WORKSHEET</c> carries a formula at all; <c>DEFAULT</c> and
    /// <c>DIRECTLY</c> name nothing and are what a series writes for the parts it has no source
    /// for, which is why almost every chart in the corpus holds four of these and two are empty.
    /// </remarks>
    private void ReadSourceLink(BiffRecordReader stream)
    {
        if (_series.Count == 0) return;

        int destination = stream.ReadByte();
        int link = stream.ReadByte();
        stream.Skip(4);

        // A title link is followed by the literal string when there is one, whatever its link
        // type says — an unlinked series name is exactly the DIRECTLY case.
        if (destination == SourceTitle) _expectSeriesName = true;

        if (link != SourceLinkWorksheet) return;

        int length = stream.ReadUInt16();
        if (XlsChartFormula.Read(stream, length, stream.Version) is not { } range) return;

        SeriesLinks series = _series[^1];
        switch (destination)
        {
            case SourceValues: series.Values = range; break;
            case SourceCategories: series.Categories = range; break;
            case SourceTitle: series.Title = range; break;
            default: break;
        }
    }

    /// <summary>Turns the series' links into series, with whatever cells were gathered.</summary>
    /// <remarks>
    /// <para>
    /// A series with no resolvable value link is dropped rather than drawn empty. That is what
    /// LibreOffice shows for one — a legend entry and no marks — and an empty series here would
    /// additionally drag the value axis to the 0…12 default scale that a chart with no numbers
    /// at all gets, which is the whole defect this reads the links to remove.
    /// </para>
    /// <para>
    /// The categories come from the first series that names any. BIFF writes the same category
    /// link on every series of a chart, and Calc likewise takes the first
    /// (<c>XclImpChTypeGroup::CreateDataSeries</c> hands the group's categories to the
    /// diagram once).
    /// </para>
    /// </remarks>
    private (IReadOnlyList<string?> Categories, IReadOnlyList<ChartSeries> Series) BuildSeries(
        XlsChartData? data, XlsExternSheets? sheets, int ownSheet)
    {
        if (data is null || _series.Count == 0) return ([], []);

        List<string?> categories = [];
        List<ChartSeries> built = [];

        foreach (SeriesLinks series in _series)
        {
            if (series.Values is not { } values || Resolve(values, sheets, ownSheet) is not { } valueSheet)
            {
                continue;
            }

            List<double?> numbers = data.Numbers(valueSheet, values);
            if (numbers.TrueForAll(number => number is null)) continue;

            if (categories.Count == 0
                && series.Categories is { } labels
                && Resolve(labels, sheets, ownSheet) is { } labelSheet)
            {
                categories.AddRange(data.Texts(labelSheet, labels));
            }

            string? name = series.Name;
            if (name is null
                && series.Title is { } title
                && Resolve(title, sheets, ownSheet) is { } titleSheet)
            {
                name = data.TextOf(titleSheet, title.FirstRow, title.FirstColumn);
            }

            built.Add(new ChartSeries(name is { Length: > 0 } ? name : null, numbers));
        }

        // Categories are indexed by point, so a shorter list than the longest series leaves the
        // tail of that series unlabelled rather than mislabelled.
        return (categories, built);
    }

    private static int? Resolve(XlsChartRange range, XlsExternSheets? sheets, int ownSheet)
        => range.Ixti < 0 ? ownSheet : sheets?.SheetOf(range.Ixti);

    private void Close(ushort container)
    {
        if (container != BiffChartRecords.Text) return;

        // A title with no text is the placeholder Excel writes for every object that could carry
        // one; only a linked, non-empty block names anything.
        if (_pendingText is { Length: > 0 } text)
        {
            switch (_pendingLink)
            {
                case LinkTitle: _title ??= text; break;
                case LinkValueAxis: _valueTitle ??= text; break;
                case LinkCategoryAxis: _categoryTitle ??= text; break;
                default: break;
            }
        }

        _pendingText = null;
        _pendingLink = -1;
    }

    private void ReadValueRange(BiffRecordReader stream)
    {
        double minimum = stream.ReadDouble();
        double maximum = stream.ReadDouble();
        double major = stream.ReadDouble();
        stream.Skip(16);
        ushort flags = stream.ReadUInt16();

        _valueScale = new ChartScaleRequest(
            (flags & AutoMinimum) != 0 ? null : minimum,
            (flags & AutoMaximum) != 0 ? null : maximum,
            (flags & AutoMajor) != 0 || major <= 0.0 ? null : major,
            (flags & Reversed) != 0);
    }

    /// <summary>
    /// Which axis a major gridline belongs to.
    /// </summary>
    /// <remarks>
    /// BIFF numbers its axes by dimension — X is the categories and Y the values — and a bar
    /// chart turned on its side keeps that numbering while swapping which way each is drawn. The
    /// plot model names them by role instead, so the two are the same either way and no
    /// transposition is needed here.
    /// </remarks>
    private void MarkGrid()
    {
        if (_axis == AxisY) _valueGrid = true;
        else if (_axis == AxisX) _categoryGrid = true;
    }

    /// <summary>The first type group decides the chart; a second is a combination chart.</summary>
    private void SetKind(ChartPlotKind kind)
    {
        if (_hasType) return;
        _hasType = true;
        _kind = kind;
    }

    private bool Inside(ushort container) => _open.Contains(container);

    /// <summary>
    /// Whether the innermost open container is this one.
    /// </summary>
    /// <remarks>
    /// <c>CHSOURCELINK</c> appears under <c>CHSERIES</c> and under <c>CHTEXT</c>, meaning
    /// entirely different things, and a <c>CHTEXT</c> sits <em>inside</em> the series it labels.
    /// So membership is not enough here where it is for a title: only the innermost container
    /// separates a series' value link from a data label's text link.
    /// </remarks>
    private bool InnermostIs(ushort container) => _open.Count > 0 && _open.Peek() == container;

    /// <summary>The rectangles one series names, before any of them is resolved.</summary>
    private sealed class SeriesLinks
    {
        public XlsChartRange? Values { get; set; }

        public XlsChartRange? Categories { get; set; }

        public XlsChartRange? Title { get; set; }

        /// <summary>The name written literally, when the series states one that way.</summary>
        public string? Name { get; set; }
    }

    /// <summary>A length stated in 1/65536 of a point, which is how a chart states its frame.</summary>
    private static Length FixedPoints(BiffRecordReader stream)
        => Length.FromPoints(stream.ReadInt32() / 65536.0);

    /// <summary>Which part of a series a <c>CHSOURCELINK</c> feeds — <c>EXC_CHSRCLINK_*</c>.</summary>
    private const int SourceTitle = 0;

    private const int SourceValues = 1;

    private const int SourceCategories = 2;

    /// <summary>The only link type that carries a formula.</summary>
    private const int SourceLinkWorksheet = 2;

    private const int LinkTitle = 1;
    private const int LinkValueAxis = 2;
    private const int LinkCategoryAxis = 3;

    private const int AxisX = 0;
    private const int AxisY = 1;

    private const ushort MajorGridLine = 1;

    private const ushort BarHorizontal = 0x0001;
    private const ushort BarStacked = 0x0002;
    private const ushort BarPercent = 0x0004;
    private const ushort LineStacked = 0x0001;
    private const ushort LinePercent = 0x0002;

    private const ushort AutoMinimum = 0x0001;
    private const ushort AutoMaximum = 0x0002;
    private const ushort AutoMajor = 0x0004;
    private const ushort Reversed = 0x0040;

    /// <summary>
    /// The colour a gridline is drawn in when the file does not say.
    /// </summary>
    /// <remarks>
    /// Black, which is what Excel's own default chart gridline is and what LibreOffice draws for
    /// these charts. The <c>CHLINEFORMAT</c> beside the <c>CHAXISLINE</c> states the real colour
    /// as a palette index; reading it is recorded in the module's TODO.
    /// </remarks>
    private static readonly Colour GridColour = Colour.Black;
}

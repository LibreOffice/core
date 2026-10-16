using System.Buffers.Binary;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Diagnostics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.MsBinary;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Writes a one-chart-sheet BIFF8 workbook, for tests about what a chart substream states.
/// </summary>
/// <remarks>
/// <para>
/// Shared by <see cref="XlsChartFontTests"/> and <see cref="XlsChartFormatTests"/> because both
/// need the same envelope — a globals stream with the <c>FONT</c> buffer and a <c>BOUNDSHEET</c>
/// naming a chart sheet — and differ only in the records they put inside the chart. Two copies of
/// this is how the two would come apart.
/// </para>
/// <para>
/// <strong>Synthetic, and it has to be.</strong> The properties under test are decided by where a
/// record sits in a chart substream's tree, and a fixture that can put the same record in two
/// places is the only way to show that the position is what decides. The corpus's own chart
/// workbooks each state one answer throughout.
/// </para>
/// </remarks>
internal static class BiffChartFixture
{
    /// <summary>
    /// The <c>FONT</c> records every fixture writes, in order. Index 4 does not exist.
    /// </summary>
    /// <remarks>
    /// Record zero is the workbook's app font and is Liberation Sans, so a chart naming nothing
    /// and a chart whose family failed to be read look the same. That is why the discriminating
    /// cases name Caladea, Carlito or DejaVu Sans instead — and never Arial, which resolves
    /// through fontconfig to Liberation Sans and so cannot tell the two apart.
    /// </remarks>
    public static readonly string[] Fonts =
        ["Liberation Sans", "Caladea", "Carlito", "Liberation Serif", "DejaVu Sans"];

    /// <summary>
    /// Two further <c>FONT</c> records that differ in size and weight rather than in family.
    /// </summary>
    /// <remarks>
    /// Written after <see cref="Fonts"/> so that the indices those tests use do not move. With the
    /// phantom entry at four they land at 6 and 7 — see <see cref="XlsChartFontTests"/>' constants.
    /// The families repeat deliberately: a case about a size must not be able to pass because the
    /// face changed.
    /// </remarks>
    public static readonly (string Name, ushort Twips, ushort Weight)[] SizedFonts =
        [("Liberation Sans", 280, 700), ("Liberation Sans", 160, 400)];

    /// <summary>
    /// Reads a workbook holding one chart sheet whose substream is the bytes given.
    /// </summary>
    /// <param name="substream">A chart substream, <c>BOF</c> to <c>EOF</c>.</param>
    /// <param name="withData">
    /// Puts a worksheet holding one number ahead of the chart sheet, and the <c>SUPBOOK</c> and
    /// <c>EXTERNSHEET</c> that let <see cref="SeriesLink"/> reach it. A series whose values do not
    /// resolve is dropped rather than drawn empty, so anything asserting about
    /// <see cref="ChartPlot.Series"/> needs this and nothing else does.
    /// </param>
    /// <param name="cellFormat">
    /// The <c>ifmt</c> the worksheet's one cell is formatted with, or null to leave it General.
    /// An axis carrying no <c>CHFORMAT</c> links its number format to its source, and the source
    /// is that cell — so this is the only way to state the format such a chart resolves to. The
    /// index is defined by a <c>FORMAT</c> record when it is above the built-in range.
    /// </param>
    /// <param name="formatCode">
    /// The code that <c>ifmt</c> names, when it is above the built-in range. A date axis is
    /// resolved only when the categories' own format is a date one, so a test about one has to
    /// state a date code here.
    /// </param>
    /// <param name="hiddenRow">
    /// Hides the worksheet's second row. A chart whose <c>CHPROPERTIES</c> says it plots only the
    /// cells its sheet shows drops such a cell from the series entirely, so this and
    /// <see cref="SeriesAreaLink"/> together are what a case about that rule needs.
    /// </param>
    public static ChartPlot Chart(
        byte[] substream, bool withData = false, ushort? cellFormat = null,
        string formatCode = "0.0", bool hiddenRow = false)
    {
        List<byte> globals = [.. Record(Bof, [0x00, 0x06, 0x05, 0x00, 0, 0, 0, 0])];
        foreach (string name in Fonts) globals.AddRange(FontRecord(name));
        foreach ((string name, ushort twips, ushort weight) in SizedFonts)
            globals.AddRange(FontRecord(name, twips: twips, weight: weight));

        if (cellFormat is { } ifmt)
        {
            if (ifmt >= FirstUserFormat) globals.AddRange(FormatRecord(ifmt, formatCode));
            globals.AddRange(XfRecord(ifmt));
        }

        if (withData)
        {
            // A self SUPBOOK: the sheet count, then the marker that says "this workbook".
            globals.AddRange(Record(SupBook, [.. Word(1), 0x01, 0x04]));

            // One EXTERNSHEET entry, so ixti 0 names sheet 0 of this workbook.
            globals.AddRange(Record(ExternSheet, [.. Word(1), .. Word(0), .. Word(0), .. Word(0)]));
        }

        byte[] data = withData ? Worksheet(hiddenRow) : [];

        // BOUNDSHEET states where each sheet's BOF is, so the records have to be built once
        // everything before them is sized — including themselves.
        int directory = (withData ? BoundSheetRecord(0, worksheet: true).Length : 0)
            + BoundSheetRecord(0).Length
            + 4;
        int firstSheet = globals.Count + directory;

        if (withData) globals.AddRange(BoundSheetRecord(firstSheet, worksheet: true));
        globals.AddRange(BoundSheetRecord(firstSheet + data.Length));
        globals.AddRange(Record(Eof, []));
        globals.AddRange(data);
        globals.AddRange(substream);

        List<Diagnostic> diagnostics = [];
        XlsWorkbookReader reader = new([.. globals], diagnostics);
        reader.Read();

        return reader.Layouts
            .SelectMany(layout => layout.Drawings.Items)
            .Single(item => item.Chart is not null).Chart!;
    }

    /// <summary>
    /// A worksheet substream holding two numbers, at A1 and A2, for a series to plot.
    /// </summary>
    /// <remarks>
    /// The second cell and its <c>ROW</c> records were added for the visible-cells rule and are
    /// invisible to every case that came before it: those name A1 alone, through
    /// <see cref="SeriesLink"/>'s single-cell <c>tRef3d</c>.
    /// </remarks>
    /// <param name="hiddenRow">Whether row 2 states the hidden flag.</param>
    private static byte[] Worksheet(bool hiddenRow = false) =>
    [
        .. Record(Bof, [0x00, 0x06, 0x10, 0x00, 0, 0, 0, 0]),
        .. Record(Dimensions, [0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0]),
        .. RowRecord(0, hidden: false),
        .. RowRecord(1, hidden: hiddenRow),
        .. Record(Number, [.. Word(0), .. Word(0), .. Word(0), .. BitConverter.GetBytes(42.0)]),
        .. Record(Number, [.. Word(1), .. Word(0), .. Word(0), .. BitConverter.GetBytes(7.0)]),
        .. Record(Eof, []),
    ];

    /// <summary>
    /// A <c>ROW</c> record: the row, its height, and the flags whose bit 5 hides it.
    /// </summary>
    /// <remarks>
    /// Sixteen bytes, which is the shortest the reader will act on. The height is 255 twips so
    /// that a hidden row and a visible one differ in the flag alone.
    /// </remarks>
    private static byte[] RowRecord(ushort row, bool hidden) => Record(Row,
    [
        .. Word(row), .. Word(0), .. Word(2),
        .. Word(255),
        .. Word(0), .. Word(0),
        .. Word(hidden ? (ushort)0x0020 : (ushort)0),
        .. Word(0),
    ]);

    /// <summary>
    /// A <c>CHSOURCELINK</c> naming cell A1 of the workbook's first sheet as a series' values.
    /// </summary>
    /// <remarks>
    /// A destination of 1 is <c>EXC_CHSRCLINK_VALUES</c> and a link type of 2 is
    /// <c>EXC_CHSRCLINK_WORKSHEET</c>, which is the only type that carries a formula at all. The
    /// formula is one <c>tRef3d</c> token: the opcode, the <c>ixti</c>, then the row and column.
    /// </remarks>
    /// <param name="destination">
    /// Which part of the series the cell feeds — 1 for <c>EXC_CHSRCLINK_VALUES</c>, 3 for
    /// <c>EXC_CHSRCLINK_BUBBLES</c>, which is a bubble chart's third dimension.
    /// </param>
    /// <param name="row">
    /// Which row of the worksheet's one column the token names. The fixture's sheet holds 42 in
    /// row 0 and 7 in row 1, so two links naming different rows give a series two distinct
    /// sequences — which is what a case about the sizes needs, or it cannot tell them from the
    /// values.
    /// </param>
    public static byte[] SeriesLink(byte destination = 1, ushort row = 0) => Record(ChSourceLink,
    [
        destination, 2, 0, 0, 0, 0,
        .. Word(7),
        0x3A, .. Word(0), .. Word(row), .. Word(0),
    ]);

    /// <summary>
    /// A <c>CHSOURCELINK</c> naming <em>A1:A2</em> as a series' values or categories.
    /// </summary>
    /// <remarks>
    /// One <c>tArea3d</c> token rather than <see cref="SeriesLink"/>'s <c>tRef3d</c>: the
    /// visible-cells rule can only be seen over a rectangle holding both a shown cell and a
    /// hidden one.
    /// </remarks>
    /// <param name="destination">
    /// 1 for <c>EXC_CHSRCLINK_VALUES</c>, 2 for <c>EXC_CHSRCLINK_CATEGORY</c>.
    /// </param>
    public static byte[] SeriesAreaLink(byte destination = 1) => Record(ChSourceLink,
    [
        destination, 2, 0, 0, 0, 0,
        .. Word(11),
        0x3B, .. Word(0), .. Word(0), .. Word(1), .. Word(0), .. Word(0),
    ]);

    /// <summary>
    /// A <c>CHSOURCELINK</c> naming cell A1 as a series' <em>categories</em>.
    /// </summary>
    /// <remarks>
    /// The same shape as <see cref="SeriesLink"/> with destination 2,
    /// <c>EXC_CHSRCLINK_CATEGORY</c>. A date axis is resolved from the cells this names — both
    /// their numbers and the number format the first of them carries — so a chart with no
    /// category link has no date axis whatever its <c>CHDATERANGE</c> says.
    /// </remarks>
    public static byte[] CategoryLink() => Record(ChSourceLink,
    [
        2, 2, 0, 0, 0, 0,
        .. Word(7),
        0x3A, .. Word(0), .. Word(0), .. Word(0),
    ]);

    /// <summary>Wraps a chart's records in the substream envelope, <c>CHCHART</c> included.</summary>
    /// <param name="body">Everything between the <c>CHCHART</c> group's begin and end.</param>
    public static byte[] Substream(IEnumerable<byte> body) =>
    [
        // A chart substream's BOF states type 0x0020.
        .. Record(Bof, [0x00, 0x06, 0x20, 0x00, 0, 0, 0, 0]),

        // CHCHART: the frame rectangle, in 1/65536 pt. Only its presence matters here.
        .. Record(ChChart, new byte[16]),
        .. Record(ChBegin, []),
        .. body,
        .. Record(ChEnd, []),
        .. Record(Eof, []),
    ];

    /// <summary>A group: its header record, its children, and the begin and end around them.</summary>
    public static byte[] Group(ushort id, byte[] header, params byte[][] children) =>
    [
        .. Record(id, header),
        .. Record(ChBegin, []),
        .. children.SelectMany(child => child),
        .. Record(ChEnd, []),
    ];

    /// <summary>
    /// A BIFF8 <c>CHAREAFORMAT</c>: a foreground and background <c>RGB</c>, a pattern, flags,
    /// then the two palette indices that supersede the <c>RGB</c>s.
    /// </summary>
    /// <param name="foregroundIndex">The palette index the area fills with.</param>
    /// <param name="pattern">1 for solid; <c>EXC_PATT_NONE</c> is 0 and fills nothing.</param>
    /// <param name="automatic">Sets <c>EXC_CHAREAFORMAT_AUTO</c>, which names no colour at all.</param>
    public static byte[] AreaFormat(ushort foregroundIndex, ushort pattern = 1, bool automatic = false)
        => Record(ChAreaFormat,
        [
            .. Dword(0x00FFFFFF), .. Dword(0),             // the BIFF5 RGBs, superseded below
            .. Word(pattern),
            .. Word(automatic ? (ushort)1 : (ushort)0),
            .. Word(foregroundIndex), .. Word(9),
        ]);

    /// <summary>A BIFF8 <c>CHLINEFORMAT</c>: an <c>RGB</c>, a pattern, a weight, flags, an index.</summary>
    /// <param name="colourIndex">The palette index the line is drawn in.</param>
    /// <param name="pattern">0 for solid; <c>EXC_CHLINEFORMAT_NONE</c> is 5 and draws nothing.</param>
    /// <param name="automatic">Sets <c>EXC_CHLINEFORMAT_AUTO</c>.</param>
    /// <param name="weight">
    /// <c>EXC_CHLINEFORMAT_HAIR</c> is −1, <c>SINGLE</c> 0, <c>DOUBLE</c> 1 and <c>TRIPLE</c> 2 —
    /// signed, and the default here stays at 1 so that every case written before the weight was
    /// read keeps the bytes it was written against.
    /// </param>
    public static byte[] LineFormat(
        ushort colourIndex, ushort pattern = 0, bool automatic = false, short weight = 1)
        => Record(ChLineFormat,
        [
            .. Dword(0x00000000),
            .. Word(pattern),
            .. Word(unchecked((ushort)weight)),
            .. Word(automatic ? (ushort)1 : (ushort)0),
            .. Word(colourIndex),
        ]);

    /// <summary>
    /// A <c>CHESCHERFORMAT</c> stating one solid fill colour, as Excel writes it.
    /// </summary>
    /// <remarks>
    /// The payload is a bare DFF <c>msofbtOPT</c>: eight bytes of header whose instance is the
    /// property count, then six bytes per property. Two are written — the fill colour and the
    /// boolean group that says the shape is filled at all — because a colour with nothing
    /// asserting <c>fFilled</c> is what a chart writes when it wants no fill.
    /// </remarks>
    /// <param name="colour">An MSO colour word, top byte included.</param>
    /// <param name="filled">Whether <c>fFilled</c> is set in the boolean group.</param>
    public static byte[] EscherFormat(uint colour, bool filled = true)
    {
        byte[] properties =
        [
            .. Word(FillColourProperty), .. Dword(colour),
            .. Word(FillBooleanGroup), .. Dword(filled ? 0x001F0010u : 0x001F0000u),
        ];

        byte[] payload =
        [
            // instance = property count, in the top twelve bits of the first word; version 3.
            .. Word((ushort)((2 << 4) | 3)),
            .. Word(0xF00B),
            .. Dword((uint)properties.Length),
            .. properties,
        ];

        return Record(ChEscherFormat, payload);
    }

    /// <summary>
    /// A BIFF8 <c>FORMAT</c> record: an <c>ifmt</c> and the code it names.
    /// </summary>
    /// <param name="ifmt">The index, which a cell's <c>XF</c> or an axis' <c>CHFORMAT</c> points at.</param>
    /// <param name="code">The format code.</param>
    public static byte[] FormatRecord(ushort ifmt, string code)
    {
        ArgumentNullException.ThrowIfNull(code);

        return Record(Format,
        [
            .. Word(ifmt),
            .. Word((ushort)code.Length),
            0,                   // eight-bit characters
            .. code.Select(character => (byte)character),
        ]);
    }

    /// <summary>
    /// A BIFF8 cell <c>XF</c> stating one number format and nothing else.
    /// </summary>
    /// <remarks>
    /// The one field that is not obvious is <c>used</c>. In a cell <c>XF</c> a set bit means
    /// "this <c>XF</c> states the attribute", and bit 10 is the number format — without it the
    /// reader defers to the parent style and the cell comes out General
    /// (<c>XclImpXF::SetUsedFlags</c>, <c>sc/source/filter/excel/xistyle.cxx:1466</c>).
    /// </remarks>
    /// <param name="ifmt">The number format index.</param>
    public static byte[] XfRecord(ushort ifmt) => Record(Xf,
    [
        .. Word(0),               // font index
        .. Word(ifmt),
        .. Word(0),               // type and protection: a cell XF whose parent is 0
        .. Word(0),               // alignment
        .. Word(StatesFormat),
        .. new byte[10],          // borders and area
    ]);

    /// <summary>A BIFF8 <c>FONT</c> record naming one family, ten point and regular by default.</summary>
    /// <param name="name">The family.</param>
    /// <param name="family">
    /// The Windows <c>FF_*</c> family code the record declares — 0 dontcare, 1 roman, 2 swiss.
    /// Zero unless a test is about the family byte, which sits between the underline and the
    /// character set and is the one field whose offset nothing else in this fixture pins.
    /// </param>
    /// <param name="twips">The em size in twentieths of a point, which is how BIFF states it.</param>
    /// <param name="weight">400 or 700; BIFF's <c>bls</c> field is a full weight.</param>
    /// <remarks>
    /// Three optional parameters because two rounds extended this helper independently — one for
    /// the declared family, one for size and weight. Pass them by name: the family arrived first
    /// and sits second, so a positional size would be read as a family code.
    /// </remarks>
    public static byte[] FontRecord(
        string name, byte family = 0, ushort twips = 200, ushort weight = 400)
    {
        ArgumentNullException.ThrowIfNull(name);

        byte[] body =
        [
            .. Word(twips),      // height, in twentieths of a point
            .. Word(0),          // flags
            .. Word(0x7FFF),     // colour: automatic
            .. Word(weight),
            .. Word(0),          // escapement
            0,                   // underline
            family, 0, 0,        // family, character set, reserved
            (byte)name.Length,
            0,                   // eight-bit characters
            .. name.Select(character => (byte)character),
        ];

        return Record(Font, body);
    }

    /// <summary>A <c>BOUNDSHEET</c> naming a sheet at one offset.</summary>
    /// <param name="offset">Where that sheet's <c>BOF</c> is, from the start of the stream.</param>
    /// <param name="worksheet">A worksheet rather than a chart sheet; the kind is the flags' high byte.</param>
    public static byte[] BoundSheetRecord(int offset, bool worksheet = false)
    {
        byte[] body = new byte[4 + 2 + 2 + 5];
        BinaryPrimitives.WriteInt32LittleEndian(body, offset);
        body[5] = worksheet ? (byte)0x00 : (byte)0x02;
        body[6] = 5;      // cch
        body[7] = 0;      // compressed
        (worksheet ? "Data "u8 : "Chart"u8).CopyTo(body.AsSpan(8));
        return Record(BoundSheet, body);
    }

    public static byte[] Word(ushort value)
    {
        byte[] written = new byte[2];
        BinaryPrimitives.WriteUInt16LittleEndian(written, value);
        return written;
    }

    public static byte[] Dword(uint value)
    {
        byte[] written = new byte[4];
        BinaryPrimitives.WriteUInt32LittleEndian(written, value);
        return written;
    }

    public static byte[] Record(ushort id, byte[] payload)
    {
        ArgumentNullException.ThrowIfNull(payload);

        byte[] record = new byte[4 + payload.Length];
        BinaryPrimitives.WriteUInt16LittleEndian(record, id);
        BinaryPrimitives.WriteUInt16LittleEndian(record.AsSpan(2), (ushort)payload.Length);
        payload.CopyTo(record.AsSpan(4));
        return record;
    }

    public const ushort Bof = 0x0809;
    public const ushort Eof = 0x000A;
    public const ushort BoundSheet = 0x0085;
    public const ushort Font = 0x0031;
    public const ushort Dimensions = 0x0200;
    public const ushort Number = 0x0203;
    public const ushort Row = 0x0208;
    public const ushort ExternSheet = 0x0017;
    public const ushort SupBook = 0x01AE;
    public const ushort Format = 0x041E;
    public const ushort Xf = 0x00E0;

    /// <summary>The first <c>ifmt</c> a <c>FORMAT</c> record has to define; below it is built in.</summary>
    public const ushort FirstUserFormat = 164;

    /// <summary>Bit 10 of a cell <c>XF</c>'s used-attribute field: it states its own format.</summary>
    private const ushort StatesFormat = 0x0400;

    public const ushort ChChart = 0x1002;
    public const ushort ChSeries = 0x1003;
    public const ushort ChDataFormat = 0x1006;
    public const ushort ChString = 0x100D;
    public const ushort ChObjectLink = 0x1027;
    public const ushort ChLineFormat = 0x1007;
    public const ushort ChAreaFormat = 0x100A;
    public const ushort ChLegend = 0x1015;
    public const ushort ChAxis = 0x101D;

    /// <summary><c>CHTICK</c>, <c>EXC_ID_CHTICK</c>.</summary>
    public const ushort ChTick = 0x101E;

    /// <summary><c>CHAXISLINE</c>, which names which of an axis' lines the next format is for.</summary>
    public const ushort ChAxisLine = 0x1021;
    public const ushort ChDefaultText = 0x1024;
    public const ushort ChText = 0x1025;
    public const ushort ChFont = 0x1026;
    public const ushort ChFrame = 0x1032;
    public const ushort ChBegin = 0x1033;
    public const ushort ChEnd = 0x1034;
    public const ushort ChAxesSet = 0x1041;

    /// <summary><c>CHFRAMEPOS</c>, <c>xlchart.hxx</c>:641.</summary>
    public const ushort ChFramePos = 0x104F;
    public const ushort ChTypeGroup = 0x1014;
    public const ushort ChSeriesGroup = 0x1045;
    public const ushort ChValueRange = 0x101F;
    public const ushort ChProperties = 0x1044;
    public const ushort ChBar = 0x1017;
    public const ushort ChLine = 0x1018;
    public const ushort ChPie = 0x1019;
    public const ushort ChScatter = 0x101B;

    /// <summary>An axis' own number format index — <c>EXC_ID_CHFORMAT</c>.</summary>
    public const ushort ChFormat = 0x104E;

    public const ushort ChSourceLink = 0x1051;
    public const ushort ChEscherFormat = 0x1066;

    public const ushort GlobalDefaultText = 2;
    public const ushort AxesSetDefaultText = 3;

    /// <summary>The DFF property that holds a shape's foreground fill colour.</summary>
    private const ushort FillColourProperty = 385;

    /// <summary>The boolean group <c>fFilled</c> lives in, which is property 447.</summary>
    private const ushort FillBooleanGroup = 447;
}

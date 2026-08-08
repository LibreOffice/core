using System.Buffers.Binary;
using Paperless.Core.Charts;
using Paperless.Core.Diagnostics;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.MsBinary;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

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
    /// Reads a workbook holding one chart sheet whose substream is the bytes given.
    /// </summary>
    /// <param name="substream">A chart substream, <c>BOF</c> to <c>EOF</c>.</param>
    /// <param name="withData">
    /// Puts a worksheet holding one number ahead of the chart sheet, and the <c>SUPBOOK</c> and
    /// <c>EXTERNSHEET</c> that let <see cref="SeriesLink"/> reach it. A series whose values do not
    /// resolve is dropped rather than drawn empty, so anything asserting about
    /// <see cref="ChartPlot.Series"/> needs this and nothing else does.
    /// </param>
    public static ChartPlot Chart(byte[] substream, bool withData = false)
    {
        List<byte> globals = [.. Record(Bof, [0x00, 0x06, 0x05, 0x00, 0, 0, 0, 0])];
        foreach (string name in Fonts) globals.AddRange(FontRecord(name));

        if (withData)
        {
            // A self SUPBOOK: the sheet count, then the marker that says "this workbook".
            globals.AddRange(Record(SupBook, [.. Word(1), 0x01, 0x04]));

            // One EXTERNSHEET entry, so ixti 0 names sheet 0 of this workbook.
            globals.AddRange(Record(ExternSheet, [.. Word(1), .. Word(0), .. Word(0), .. Word(0)]));
        }

        byte[] data = withData ? Worksheet() : [];

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

    /// <summary>A worksheet substream holding one number, at A1, for a series to plot.</summary>
    private static byte[] Worksheet() =>
    [
        .. Record(Bof, [0x00, 0x06, 0x10, 0x00, 0, 0, 0, 0]),
        .. Record(Dimensions, [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0]),
        .. Record(Number, [.. Word(0), .. Word(0), .. Word(0), .. BitConverter.GetBytes(42.0)]),
        .. Record(Eof, []),
    ];

    /// <summary>
    /// A <c>CHSOURCELINK</c> naming cell A1 of the workbook's first sheet as a series' values.
    /// </summary>
    /// <remarks>
    /// A destination of 1 is <c>EXC_CHSRCLINK_VALUES</c> and a link type of 2 is
    /// <c>EXC_CHSRCLINK_WORKSHEET</c>, which is the only type that carries a formula at all. The
    /// formula is one <c>tRef3d</c> token: the opcode, the <c>ixti</c>, then the row and column.
    /// </remarks>
    public static byte[] SeriesLink() => Record(ChSourceLink,
    [
        1, 2, 0, 0, 0, 0,
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
    public static byte[] LineFormat(ushort colourIndex, ushort pattern = 0, bool automatic = false)
        => Record(ChLineFormat,
        [
            .. Dword(0x00000000),
            .. Word(pattern),
            .. Word(1),                                    // weight
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

    /// <summary>A BIFF8 <c>FONT</c> record naming one family at ten point.</summary>
    public static byte[] FontRecord(string name)
    {
        ArgumentNullException.ThrowIfNull(name);

        byte[] body =
        [
            .. Word(200),        // height, in twentieths of a point
            .. Word(0),          // flags
            .. Word(0x7FFF),     // colour: automatic
            .. Word(400),        // weight
            .. Word(0),          // escapement
            0,                   // underline
            0, 0, 0,             // family, character set, reserved
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
    public const ushort ExternSheet = 0x0017;
    public const ushort SupBook = 0x01AE;

    public const ushort ChChart = 0x1002;
    public const ushort ChSeries = 0x1003;
    public const ushort ChDataFormat = 0x1006;
    public const ushort ChLineFormat = 0x1007;
    public const ushort ChAreaFormat = 0x100A;
    public const ushort ChLegend = 0x1015;
    public const ushort ChAxis = 0x101D;
    public const ushort ChDefaultText = 0x1024;
    public const ushort ChText = 0x1025;
    public const ushort ChFont = 0x1026;
    public const ushort ChFrame = 0x1032;
    public const ushort ChBegin = 0x1033;
    public const ushort ChEnd = 0x1034;
    public const ushort ChAxesSet = 0x1041;
    public const ushort ChSourceLink = 0x1051;
    public const ushort ChEscherFormat = 0x1066;

    public const ushort GlobalDefaultText = 2;
    public const ushort AxesSetDefaultText = 3;

    /// <summary>The DFF property that holds a shape's foreground fill colour.</summary>
    private const ushort FillColourProperty = 385;

    /// <summary>The boolean group <c>fFilled</c> lives in, which is property 447.</summary>
    private const ushort FillBooleanGroup = 447;
}

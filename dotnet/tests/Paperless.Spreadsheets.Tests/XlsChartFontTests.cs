using System.Buffers.Binary;
using Paperless.Core.Charts;
using Paperless.Core.Diagnostics;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.MsBinary;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Which face a BIFF chart's text is measured and drawn in.
/// </summary>
/// <remarks>
/// <para>
/// A chart substream states its face as a <c>CHFONT</c> record holding nothing but an index into
/// the workbook's <c>FONT</c> buffer (<c>XclImpChFont::ReadChFont</c>,
/// <c>sc/source/filter/excel/xichart.cxx:941</c>), and which text that index dresses is decided
/// by where the record sits. <c>XclImpChChart::GetDefaultText</c> (<c>xichart.cxx:3956-3969</c>)
/// hands the chart's <em>global</em> default text to the title and the legend in every generation,
/// and to the axis labels too in BIFF5; only BIFF8 splits those onto the axes-set default.
/// </para>
/// <para>
/// <strong>Synthetic, and it has to be.</strong> A record-level census of all 61 OLE2 workbooks on
/// the sheets corpus track finds six holding a chart substream, fifteen substreams between them,
/// and <em>every one of the fifteen states a single family across every <c>CHFONT</c> it
/// carries</em> — global default, axes-set default and first-stated all agree. So no corpus
/// document separates the three answers, and only a file written for the purpose can. The
/// corpus's part is elsewhere: two of those six state Calibri, and both of them changed when this
/// was wired.
/// </para>
/// <para>
/// <strong>No fixture here states Arial.</strong> Arial resolves through fontconfig to Liberation
/// Sans, which is exactly the default an unwired consumer already used, so an Arial fixture passes
/// whether the family is read or not. Caladea and Carlito resolve to themselves.
/// </para>
/// </remarks>
public sealed class XlsChartFontTests
{
    [Fact]
    public void TheGlobalDefaultTextsFontIsTheChartsFamily()
    {
        Chart(Substream(globalFont: Caladea)).TextFamily.ShouldBe("Caladea");
    }

    /// <summary>
    /// The global default wins over the axes-set default when the two disagree.
    /// </summary>
    /// <remarks>
    /// This is the case the corpus cannot produce: all fifteen of its chart substreams state one
    /// family throughout. It pins the order rather than the outcome.
    /// </remarks>
    [Fact]
    public void TheGlobalDefaultOutranksTheAxesSetDefault()
    {
        Chart(Substream(globalFont: Caladea, axesSetFont: Carlito)).TextFamily.ShouldBe("Caladea");
    }

    [Fact]
    public void TheAxesSetDefaultIsUsedWhenThereIsNoGlobalOne()
    {
        Chart(Substream(axesSetFont: Carlito)).TextFamily.ShouldBe("Carlito");
    }

    /// <summary>
    /// A chart naming no default text at all still takes the first face it states.
    /// </summary>
    /// <remarks>
    /// The fallback matters because a <c>CHFONT</c> under an axis or a legend is that object's own
    /// font and heads no default; without this a chart writing only those would be measured in the
    /// sheet's face while its labels were drawn in its own.
    /// </remarks>
    [Fact]
    public void AFontStatedOnlyOnTheAxisIsStillTheChartsFamily()
    {
        Chart(Substream(axisFont: Carlito)).TextFamily.ShouldBe("Carlito");
    }

    /// <summary>
    /// A font stated on the axis does not outrank the chart's default text, whichever came first.
    /// </summary>
    /// <remarks>
    /// The axis' <c>CHFONT</c> is written before the default texts here, so a rule that simply
    /// took the first record would answer Carlito. It is not a default for anything —
    /// <c>ReadChDefaultText</c> only ever reaches the <c>CHTEXT</c> immediately after a
    /// <c>CHDEFAULTTEXT</c> — so the global default still decides.
    /// </remarks>
    [Fact]
    public void AnAxisFontDoesNotOutrankTheDefaultTextEvenWhenItComesFirst()
    {
        Chart(Substream(axisFont: Carlito, globalFont: Caladea)).TextFamily.ShouldBe("Caladea");
    }

    /// <summary>
    /// A chart stating no font at all leaves the family unstated, as before this was read.
    /// </summary>
    /// <remarks>
    /// Null rather than a name on purpose: which face "nothing stated" means is the consumer's
    /// question, and <c>SheetChart</c>'s measurer answers it with the sheet's own default. See
    /// <see cref="ChartPlot.TextFamily"/>.
    /// </remarks>
    [Fact]
    public void AChartStatingNoFontNamesNoFamily()
    {
        Chart(Substream()).TextFamily.ShouldBeNull();
    }

    /// <summary>
    /// The index goes through the buffer's phantom fourth entry, not through the record ordinal.
    /// </summary>
    /// <remarks>
    /// A workbook's fifth <c>FONT</c> record is font index 5 (<c>XclImpFontBuffer::GetFont</c>),
    /// and a chart indexes the same buffer a cell does. Index 5 here is the fifth record written,
    /// <c>DejaVu Sans</c>; a reader indexing the records directly would answer with the fourth.
    /// </remarks>
    [Fact]
    public void TheFontIndexSkipsTheBuffersPhantomFourthEntry()
    {
        Chart(Substream(globalFont: DejaVu)).TextFamily.ShouldBe("DejaVu Sans");
    }

    /// <summary>
    /// A <c>CHDEFAULTTEXT</c> whose <c>CHTEXT</c> is not the very next record names nothing.
    /// </summary>
    /// <remarks>
    /// <c>ReadChDefaultText</c> reads the identifier, then takes the following record only if it
    /// is a <c>CHTEXT</c> and drops the default otherwise. Here a <c>CHLEGEND</c> comes between,
    /// so the Caladea that follows dresses the legend and heads no default — and the axes-set
    /// default, which is intact, decides instead.
    /// </remarks>
    [Fact]
    public void ADefaultTextIdIsSpentOnTheRecordAfterItAndNoLater()
    {
        Chart(Substream(strandedGlobalFont: Caladea, axesSetFont: Carlito))
            .TextFamily.ShouldBe("Carlito");
    }

    /// <summary>
    /// A font on the axis is not the default text's, even while that default text is the last one
    /// opened.
    /// </summary>
    /// <remarks>
    /// The global default here states no font of its own — the placeholder Excel writes constantly
    /// — and the axis after it does. Only the innermost open container separates the two: without
    /// that test the axis' Carlito is filed as the chart's global default and outranks the
    /// axes-set default that really does state one.
    /// </remarks>
    [Fact]
    public void AFontOnTheAxisIsNotFiledAsTheDefaultTextThatPrecededIt()
    {
        Chart(Substream(emptyGlobalDefault: true, axisFont: Carlito, axesSetFont: Caladea))
            .TextFamily.ShouldBe("Caladea");
    }

    // Font indices into the buffer this workbook writes. Index 4 does not exist.
    private const ushort AppFont = 0;
    private const ushort Caladea = 1;
    private const ushort Carlito = 2;
    private const ushort DejaVu = 5;

    /// <summary>The five <c>FONT</c> records every fixture here writes, in order.</summary>
    /// <remarks>
    /// Record zero is the workbook's app font and is Liberation Sans, so a chart that names
    /// nothing and a chart whose family failed to be read look the same from the page — which is
    /// why every discriminating case above names one of the other two instead.
    /// </remarks>
    private static readonly string[] Fonts =
        ["Liberation Sans", "Caladea", "Carlito", "Liberation Serif", "DejaVu Sans"];

    private static ChartPlot Chart(byte[] substream)
    {
        List<byte> globals = [.. Record(Bof, [0x00, 0x06, 0x05, 0x00, 0, 0, 0, 0])];
        foreach (string name in Fonts) globals.AddRange(FontRecord(name));

        // BOUNDSHEET states where the sheet's BOF is, so the record has to be built once
        // everything before it is sized — including itself.
        byte[] boundSheet = BoundSheetRecord(0);
        globals.AddRange(BoundSheetRecord(globals.Count + boundSheet.Length + 4));
        globals.AddRange(Record(Eof, []));
        globals.AddRange(substream);

        List<Diagnostic> diagnostics = [];
        XlsWorkbookReader reader = new([.. globals], diagnostics);
        reader.Read();

        reader.Layouts.Count.ShouldBe(1);
        SheetDrawing drawing = reader.Layouts[0].Drawings.Items.Single(item => item.Chart is not null);
        return drawing.Chart!;
    }

    /// <summary>
    /// Writes a chart substream stating the fonts asked for, and nothing else that matters.
    /// </summary>
    /// <param name="globalFont">The font of the <c>EXC_CHDEFTEXT_GLOBAL</c> default text.</param>
    /// <param name="axesSetFont">The font of the <c>EXC_CHDEFTEXT_AXESSET</c> default text.</param>
    /// <param name="axisFont">A font on the axis itself, which heads no default text.</param>
    /// <param name="strandedGlobalFont">
    /// A global <c>CHDEFAULTTEXT</c> with a <c>CHLEGEND</c> between it and its <c>CHTEXT</c>, so
    /// the identifier is spent before the text arrives.
    /// </param>
    /// <param name="emptyGlobalDefault">
    /// Writes the global default text with no <c>CHFONT</c> in it, which is the placeholder Excel
    /// writes for an object that states nothing.
    /// </param>
    private static byte[] Substream(
        ushort? globalFont = null,
        ushort? axesSetFont = null,
        ushort? axisFont = null,
        ushort? strandedGlobalFont = null,
        bool emptyGlobalDefault = false)
    {
        List<byte> body = [];

        if (emptyGlobalDefault)
        {
            body.AddRange(Record(ChDefaultText, Word(GlobalDefaultText)));
            body.AddRange(Record(ChText, new byte[32]));
            body.AddRange(Record(ChBegin, []));
            body.AddRange(Record(ChEnd, []));
        }

        if (axisFont is { } onAxis)
        {
            body.AddRange(Record(ChAxis, new byte[18]));
            body.AddRange(Record(ChBegin, []));
            body.AddRange(Record(ChFont, Word(onAxis)));
            body.AddRange(Record(ChEnd, []));
        }

        if (strandedGlobalFont is { } stranded)
        {
            body.AddRange(Record(ChDefaultText, Word(GlobalDefaultText)));
            body.AddRange(Record(ChLegend, new byte[20]));
            body.AddRange(DefaultText(stranded));
        }

        if (globalFont is { } global) body.AddRange(DefaultText(global, GlobalDefaultText));
        if (axesSetFont is { } axesSet) body.AddRange(DefaultText(axesSet, AxesSetDefaultText));

        List<byte> chart =
        [
            // CHCHART: the frame rectangle, in 1/65536 pt. Only its presence matters here.
            .. Record(ChChart, new byte[16]),
            .. Record(ChBegin, []),
            .. body,
            .. Record(ChEnd, []),
        ];

        return
        [
            // A chart substream's BOF states type 0x0020.
            .. Record(Bof, [0x00, 0x06, 0x20, 0x00, 0, 0, 0, 0]),
            .. chart,
            .. Record(Eof, []),
        ];
    }

    /// <summary>A <c>CHTEXT</c> group carrying one <c>CHFONT</c>, optionally headed by an id.</summary>
    private static byte[] DefaultText(ushort font, ushort? id = null)
    {
        List<byte> written = [];
        if (id is { } which) written.AddRange(Record(ChDefaultText, Word(which)));

        written.AddRange(Record(ChText, new byte[32]));
        written.AddRange(Record(ChBegin, []));
        written.AddRange(Record(ChFont, Word(font)));
        written.AddRange(Record(ChEnd, []));
        return [.. written];
    }

    /// <summary>A BIFF8 <c>FONT</c> record naming one family at ten point.</summary>
    private static byte[] FontRecord(string name)
    {
        List<byte> body =
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

        return Record(Font, [.. body]);
    }

    /// <summary>A <c>BOUNDSHEET</c> naming a chart sheet at one offset.</summary>
    private static byte[] BoundSheetRecord(int offset)
    {
        byte[] body = new byte[4 + 2 + 2 + 5];
        BinaryPrimitives.WriteInt32LittleEndian(body, offset);
        body[5] = 0x02;   // the high byte of the flags: a chart sheet
        body[6] = 5;      // cch
        body[7] = 0;      // compressed
        "Chart"u8.CopyTo(body.AsSpan(8));
        return Record(BoundSheet, body);
    }

    private static byte[] Word(ushort value)
    {
        byte[] written = new byte[2];
        BinaryPrimitives.WriteUInt16LittleEndian(written, value);
        return written;
    }

    private static byte[] Record(ushort id, byte[] payload)
    {
        byte[] record = new byte[4 + payload.Length];
        BinaryPrimitives.WriteUInt16LittleEndian(record, id);
        BinaryPrimitives.WriteUInt16LittleEndian(record.AsSpan(2), (ushort)payload.Length);
        payload.CopyTo(record.AsSpan(4));
        return record;
    }

    private const ushort Bof = 0x0809;
    private const ushort Eof = 0x000A;
    private const ushort BoundSheet = 0x0085;
    private const ushort Font = 0x0031;

    private const ushort ChChart = 0x1002;
    private const ushort ChLegend = 0x1015;
    private const ushort ChAxis = 0x101D;
    private const ushort ChDefaultText = 0x1024;
    private const ushort ChText = 0x1025;
    private const ushort ChFont = 0x1026;
    private const ushort ChBegin = 0x1033;
    private const ushort ChEnd = 0x1034;

    private const ushort GlobalDefaultText = 2;
    private const ushort AxesSetDefaultText = 3;
}

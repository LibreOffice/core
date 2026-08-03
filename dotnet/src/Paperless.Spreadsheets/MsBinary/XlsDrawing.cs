using Paperless.Core.Diagnostics;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.MsBinary.Escher;
using Paperless.MsBinary.Records;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.MsBinary;

/// <summary>
/// Collects a sheet's or a chart's drawing records and turns them into anchored shapes.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The drawing is spread over three record kinds and only makes sense assembled.</strong>
/// The Escher (MS-ODRAW) byte stream arrives split across every <c>MSODRAWING</c> record in the
/// substream — the split is arbitrary and a container routinely straddles it — so the payloads are
/// concatenated into one buffer and walked once at the end. <c>OBJ</c> records carry what Excel
/// knows about a shape that Escher does not (its type and identifier), and <c>TXO</c> plus its
/// <c>CONTINUE</c> records carry a text box's string. This is
/// <c>XclImpDrawing::ReadMsoDrawing</c> (<c>sc/source/filter/excel/xiescher.cxx:4021</c>)
/// assembling <c>maDffStrm</c>, <c>maRawObjs</c> and <c>maTextMap</c>, and it has to be done that
/// way: reading the <c>MSODRAWING</c> records one at a time yields truncated containers.
/// </para>
/// <para>
/// <strong>A shape is matched to its <c>OBJ</c> by position in the assembled stream.</strong> Each
/// <c>OBJ</c> follows the <c>MSODRAWING</c> records that carry its shape, so noting how many
/// drawing bytes had arrived when it was read gives an offset that falls inside that shape's
/// container and after every earlier one. LibreOffice keys <c>maObjMap</c> on exactly that
/// (<c>xiescher.cxx:4058</c>, <c>FindDrawObj</c>) and finds the shape by upper bound; the same
/// walk is done here from the shape side, which needs no map.
/// </para>
/// <para>
/// <strong>Only text is drawn from a shape, not its fill or its outline.</strong> That is the
/// SpreadsheetML path's limit too (<see cref="SheetShapePainter"/>), so the two formats produce
/// the same page from the same document, and it is the difference that matters most: a text box is
/// the one thing on a sheet no walk of the cells can find. Fills and outlines are recorded in the
/// module's TODO.
/// </para>
/// </remarks>
internal sealed class XlsDrawingCollector(List<Diagnostic> diagnostics)
{
    /// <summary>
    /// How many bytes of Escher stream are accepted before the rest is dropped.
    /// </summary>
    /// <remarks>
    /// A guard against a damaged file whose record chain loops, not a real limit: the largest
    /// drawing in the corpus is under 300 kB, and a sheet reaching this has already produced more
    /// shapes than a page can show.
    /// </remarks>
    private const int MaxDrawingBytes = 8 * 1024 * 1024;

    /// <summary>The offsets a chart's client anchor is stated in, out of the chart area.</summary>
    /// <remarks><c>EXC_CHART_TOTALUNITS</c>, <c>sc/source/filter/inc/xlchart.hxx:163</c>.</remarks>
    private const double ChartTotalUnits = 4000.0;

    private readonly List<byte> _dff = [];
    private readonly List<ObjectEntry> _objects = [];

    /// <summary>True when nothing has been collected, which is nearly every sheet.</summary>
    public bool IsEmpty => _objects.Count == 0;

    /// <summary>Appends one <c>MSODRAWING</c> record's payload to the Escher stream.</summary>
    /// <param name="bytes">The record's bytes, its continuations already joined.</param>
    public void AddDrawing(byte[] bytes)
    {
        ArgumentNullException.ThrowIfNull(bytes);
        if (_dff.Count + bytes.Length > MaxDrawingBytes) return;
        _dff.AddRange(bytes);
    }

    /// <summary>
    /// Reads one <c>OBJ</c> record, keeping the object's type and its place in the stream.
    /// </summary>
    /// <remarks>
    /// Only the <c>ftCmo</c> subrecord is read. The rest describe a control's behaviour — a
    /// list box's source range, a button's macro — none of which reaches the page.
    /// </remarks>
    /// <param name="stream">Positioned at the record's first byte.</param>
    public void ReadObject(BiffRecordReader stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        ushort type = ushort.MaxValue;
        while (stream.RecordLeft >= 4)
        {
            ushort id = stream.ReadUInt16();
            int size = stream.ReadUInt16();
            if (id == 0 && size == 0) break;

            int left = Math.Min(size, stream.RecordLeft);
            if (id == ObjectCommon && left >= 6)
            {
                type = stream.ReadUInt16();
                stream.Skip(2);
                stream.Skip(left - 4);
            }
            else
            {
                stream.Skip(left);
            }
        }

        _objects.Add(new ObjectEntry(type, _dff.Count));
    }

    /// <summary>
    /// Reads one <c>TXO</c> record and attaches its string to the object just read.
    /// </summary>
    /// <remarks>
    /// The string is not in the <c>TXO</c> at all: the record states its length and the
    /// characters arrive in the <c>CONTINUE</c> that follows, with the formatting runs in a
    /// second one (<c>XclImpDrawing::ReadTxo</c>, <c>xiescher.cxx:4242</c>). This reader joins
    /// continuations into the record it is reading, so the header, the flags byte that opens the
    /// character data and the characters are read straight through — which is right, because the
    /// flags byte at a continuation boundary is exactly what
    /// <see cref="BiffRecordReader.ReadUnicodeString(int)"/> already expects to find there.
    /// </remarks>
    /// <param name="stream">Positioned at the record's first byte.</param>
    public void ReadText(BiffRecordReader stream)
    {
        ArgumentNullException.ThrowIfNull(stream);
        if (_objects.Count == 0) return;

        ushort flags = stream.ReadUInt16();
        stream.Skip(8);
        int length = stream.ReadUInt16();
        stream.Skip(2);
        stream.Skip(4);

        string text = length > 0 && stream.RecordLeft > 0
            ? stream.ReadUnicodeString(length)
            : string.Empty;

        if (text.Length == 0) return;

        _objects[^1] = _objects[^1] with
        {
            Text = text,

            // Bits 1-3 and 4-6 of the flags word: XclObjTextData::GetHorAlign and GetVerAlign
            // (sc/source/filter/inc/xlescher.hxx:401-402).
            Horizontal = (flags >> 1) & 0x07,
            Vertical = (flags >> 4) & 0x07,
        };
    }

    /// <summary>
    /// The shapes, anchored against a sheet's grid.
    /// </summary>
    /// <param name="grid">The sheet's columns and rows, which the anchor's offsets are fractions of.</param>
    public List<SheetDrawing> BuildForSheet(SheetGrid grid)
    {
        ArgumentNullException.ThrowIfNull(grid);
        return Build(anchor => SheetAnchor(anchor, grid));
    }

    /// <summary>
    /// The shapes of a chart substream, anchored against the chart's own rectangle.
    /// </summary>
    /// <remarks>
    /// A chart's drawing objects are positioned in quarter-thousandths of the chart area, and
    /// the fractions are stored in the <em>cell</em> fields of the client anchor rather than in
    /// its offsets — <c>XclImpChartDrawing::CalcAnchorRect</c>
    /// (<c>sc/source/filter/excel/xichart.cxx:4274</c>). So the same eighteen bytes mean
    /// something else here, and a shape on a chart sheet lands at an absolute place rather than
    /// against a column.
    /// </remarks>
    /// <param name="origin">Where the chart sits on the sheet.</param>
    /// <param name="size">How big the chart is.</param>
    public List<SheetDrawing> BuildForChart(DocPoint origin, DocSize size)
        => Build(anchor => ChartAnchor(anchor, origin, size));

    private List<SheetDrawing> Build(Func<Anchor, SheetDrawing?> place)
    {
        List<SheetDrawing> drawings = [];
        if (_dff.Count == 0 || _objects.Count == 0) return drawings;

        DffRecordBuffer buffer = new([.. _dff]);
        EscherDrawingReader reader = new(buffer, diagnostics);

        List<EscherShape> shapes = [];
        foreach (DffRecordHeader record in buffer.Range(0, buffer.Length))
        {
            if (record.Type == EscherRecordTypes.DrawingContainer)
                Flatten(reader.ReadDrawing(record), shapes);
        }

        // The n-th shape carrying client data is the n-th OBJ: both sequences are the drawing's
        // own order, and a shape without client data — the patriarch, a solver entry — has no OBJ
        // record of its own to consume one.
        int at = 0;
        foreach (EscherShape shape in shapes)
        {
            if (shape.ClientData is null) continue;
            if (at >= _objects.Count) break;

            ObjectEntry entry = _objects[at++];
            if (entry.Text is not { Length: > 0 }) continue;

            // A cell comment is not a shape on the page. Its `ftCmo` type is 25
            // (`EXC_OBJTYPE_NOTE`, `sc/source/filter/inc/xlescher.hxx:69`) and Calc's importer
            // takes the object apart rather than inserting it: `XclImpNoteObj` calls
            // `SetInsertSdrObj(false)` in its constructor — "caption object will be created
            // manually" — and turns the text into a `ScPostIt` on the cell instead
            // (`sc/source/filter/excel/xiescher.cxx:1852-1883`). The caption exists only when the
            // NOTE record marks the comment visible, which is the case this drops with it.
            if (entry.Type == NoteObject) continue;

            if (ClientAnchor(buffer, shape) is not { } anchor) continue;
            if (place(anchor) is not { } placed) continue;

            drawings.Add(placed with { Text = TextOf(entry), Name = NameOf(shape) });
        }

        return drawings;
    }

    /// <summary>Walks a group before its children, which is the order the objects arrive in.</summary>
    private static void Flatten(IReadOnlyList<EscherShape> shapes, List<EscherShape> into)
    {
        foreach (EscherShape shape in shapes)
        {
            into.Add(shape);
            if (shape.Children.Count > 0) Flatten(shape.Children, into);
        }
    }

    private static string? NameOf(EscherShape shape)
    {
        string? name = shape.Properties.Text(EscherPropertyIds.ShapeName);
        return name is { Length: > 0 } ? name : null;
    }

    private static SheetShapeText TextOf(ObjectEntry entry)
    {
        SheetShapeText body = new()
        {
            Paragraphs =
            [
                .. (entry.Text ?? string.Empty)
                    .Replace("\r\n", "\n", StringComparison.Ordinal)
                    .Split(['\n', '\r'])
                    .Select(line => new SheetShapeParagraph
                    {
                        Runs = [new SheetShapeRun(line, DefaultTextSize)],
                        Alignment = entry.Horizontal switch
                        {
                            HorizontalCentre => SheetShapeAlignment.Centre,
                            HorizontalRight => SheetShapeAlignment.Right,
                            _ => SheetShapeAlignment.Left,
                        },
                    }),
            ],
            Anchor = entry.Vertical switch
            {
                VerticalCentre => SheetShapeAnchor.Middle,
                VerticalBottom => SheetShapeAnchor.Bottom,
                _ => SheetShapeAnchor.Top,
            },
        };

        // The margins Excel gives a text box, which are not DrawingML's and are stated nowhere in
        // the file: XclImpTextObj::DoPreProcessSdrObj sets a tenth of a millimetre either way and
        // lets the text run to the shape's edge (sc/source/filter/excel/xiescher.cxx).
        return body with
        {
            LeftInset = TextInset,
            RightInset = TextInset,
            TopInset = Length.Zero,
            BottomInset = Length.Zero,
        };
    }

    private static SheetDrawing SheetAnchor(Anchor anchor, SheetGrid grid)
        => new()
        {
            Anchor = SheetAnchorKind.TwoCell,
            From = new SheetCellPoint(
                anchor.FirstColumn, Across(grid.Columns, anchor.FirstColumn, anchor.LeftOffset, ColumnUnits),
                anchor.FirstRow, Across(grid.Rows, anchor.FirstRow, anchor.TopOffset, RowUnits)),
            To = new SheetCellPoint(
                anchor.LastColumn, Across(grid.Columns, anchor.LastColumn, anchor.RightOffset, ColumnUnits),
                anchor.LastRow, Across(grid.Rows, anchor.LastRow, anchor.BottomOffset, RowUnits)),
        };

    private static SheetDrawing ChartAnchor(Anchor anchor, DocPoint origin, DocSize size)
    {
        Length left = origin.X + (size.Width * (anchor.FirstColumn / ChartTotalUnits));
        Length top = origin.Y + (size.Height * (anchor.FirstRow / ChartTotalUnits));
        Length right = origin.X + (size.Width * (anchor.LastColumn / ChartTotalUnits));
        Length bottom = origin.Y + (size.Height * (anchor.LastRow / ChartTotalUnits));

        if (right < left) (left, right) = (right, left);
        if (bottom < top) (top, bottom) = (bottom, top);

        return new SheetDrawing
        {
            Anchor = SheetAnchorKind.Absolute,
            Position = new DocPoint(left, top),
            Extent = new DocSize(right - left, bottom - top),
        };
    }

    /// <summary>How far into a column or row an anchor's fractional offset reaches.</summary>
    private static Length Across(SheetAxis axis, int index, int offset, double units)
        => axis.SizeAt(index) * Math.Min(offset / units, 1.0);

    /// <summary>
    /// The eighteen bytes of a BIFF8 client anchor: a flags word and four cell-and-offset pairs.
    /// </summary>
    /// <remarks>
    /// The column offsets are in 1024ths of the column's width and the row offsets in 256ths of
    /// the row's height — <c>lclGetXFromCol</c> and <c>lclGetYFromRow</c>
    /// (<c>sc/source/filter/excel/xlescher.cxx:54-67</c>). The asymmetry is the format's.
    /// </remarks>
    private static Anchor? ClientAnchor(DffRecordBuffer buffer, EscherShape shape)
    {
        if (shape.ClientAnchor is not { } header) return null;

        ReadOnlySpan<byte> content = buffer.Content(header);
        if (content.Length < 18) return null;

        return new Anchor(
            DffRecordBuffer.ReadUInt16(content[2..]),
            DffRecordBuffer.ReadUInt16(content[4..]),
            DffRecordBuffer.ReadUInt16(content[6..]),
            DffRecordBuffer.ReadUInt16(content[8..]),
            DffRecordBuffer.ReadUInt16(content[10..]),
            DffRecordBuffer.ReadUInt16(content[12..]),
            DffRecordBuffer.ReadUInt16(content[14..]),
            DffRecordBuffer.ReadUInt16(content[16..]));
    }

    private readonly record struct Anchor(
        int FirstColumn, int LeftOffset, int FirstRow, int TopOffset,
        int LastColumn, int RightOffset, int LastRow, int BottomOffset);

    private readonly record struct ObjectEntry(
        ushort Type, int DrawingOffset, string? Text = null, int Horizontal = 0, int Vertical = 0);

    /// <summary>The <c>ftCmo</c> subrecord identifier, <c>EXC_ID_OBJCMO</c>.</summary>
    private const ushort ObjectCommon = 0x0015;

    /// <summary>
    /// The <c>ftCmo</c> object type a cell comment has.
    /// </summary>
    /// <remarks><c>EXC_OBJTYPE_NOTE</c>, <c>sc/source/filter/inc/xlescher.hxx:69</c>.</remarks>
    private const ushort NoteObject = 25;

    private const int HorizontalCentre = 2;
    private const int HorizontalRight = 3;
    private const int VerticalCentre = 2;
    private const int VerticalBottom = 3;

    private const double ColumnUnits = 1024.0;
    private const double RowUnits = 256.0;

    /// <summary>
    /// The size a run of shape text is set at when the file states none.
    /// </summary>
    /// <remarks>
    /// Ten point, which is Excel's default text-box font and what the <c>TXO</c>'s formatting
    /// runs would name were they read. It is not <see cref="SheetShapeText.DefaultSize"/>: that
    /// is DrawingML's eighteen point, which is right for a SpreadsheetML body and nearly twice
    /// what a BIFF text box shows.
    /// </remarks>
    private static readonly Length DefaultTextSize = Length.FromPoints(10);

    /// <summary>The inset Excel leaves either side of a text box's text.</summary>
    private static readonly Length TextInset = Length.FromMm100(10);
}

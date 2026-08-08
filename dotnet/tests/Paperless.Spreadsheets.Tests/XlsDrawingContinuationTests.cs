using System.Buffers.Binary;
using Paperless.Core.Diagnostics;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.MsBinary;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// What a bare <c>CONTINUE</c> record means inside a sheet's drawing block.
/// </summary>
/// <remarks>
/// <para>
/// Excel writes a sheet's Escher stream as one <c>MSODRAWING</c> record per shape with that
/// shape's <c>OBJ</c> after it, and switches to <c>CONTINUE</c> records — in the same
/// interleaving — once the stream passes the 8224-byte record ceiling. So a <c>CONTINUE</c>
/// following an <c>OBJ</c> carries the drawing's next shape and not the object's payload, and a
/// reader that joins continuations to whatever preceded them loses every shape past the ceiling.
/// </para>
/// <para>
/// Synthetic, and it has to be: the case only arises in a file with more shapes on one sheet than
/// fit in 8224 bytes of Escher, which is about 25 pictures. The corpus has such a document
/// (<c>INDEX_Digital_Transformation_Toolkits.xls</c>, 25 <c>MSODRAWING</c> records, 70
/// <c>CONTINUE</c>s and 95 <c>OBJ</c>s) and it is a binary file a unit test cannot carry.
/// </para>
/// </remarks>
public sealed class XlsDrawingContinuationTests
{
    private const ushort Bof = 0x0809;
    private const ushort Eof = 0x000A;
    private const ushort BoundSheet = 0x0085;
    private const ushort Dimensions = 0x0200;
    private const ushort MsoDrawing = 0x00EC;
    private const ushort Obj = 0x005D;
    private const ushort Continue = 0x003C;

    [Fact]
    public void AShapeArrivingInAContinueRecordIsStillDrawn()
    {
        IReadOnlyList<SheetDrawing> drawings = Read(splitAcrossContinue: true);

        // Two shapes carry client data and two OBJ records name them. Before the fix the second
        // shape's container was absorbed into the first OBJ and only one drawing came out.
        drawings.Count.ShouldBe(2);
        drawings.ShouldAllBe(drawing => drawing.IsChart);
    }

    [Fact]
    public void TheSameShapesInTwoMsoDrawingRecordsAreDrawnTheSameWay()
    {
        // The control: identical Escher bytes, split across two MSODRAWING records instead of an
        // MSODRAWING and a CONTINUE. This path always worked, so it separates "the continuation
        // is read" from "the Escher stream is assembled at all".
        Read(splitAcrossContinue: false).Count.ShouldBe(2);
    }

    /// <summary>Reads a one-sheet BIFF8 workbook holding two shapes, and returns its drawings.</summary>
    private static IReadOnlyList<SheetDrawing> Read(bool splitAcrossContinue)
    {
        byte[] escher = Escher(out int split);

        List<byte> sheet =
        [
            .. Record(Bof, [0x00, 0x06, 0x10, 0x00, 0, 0, 0, 0]),
            .. Record(Dimensions, [0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0]),
            .. Record(MsoDrawing, escher[..split]),
            .. ChartObject(1),
            .. Record(splitAcrossContinue ? Continue : MsoDrawing, escher[split..]),
            .. ChartObject(2),
            .. Record(Eof, []),
        ];

        List<byte> globals =
        [
            .. Record(Bof, [0x00, 0x06, 0x05, 0x00, 0, 0, 0, 0]),
        ];

        // BOUNDSHEET names where the sheet's BOF is, so the record has to be built once the size
        // of everything before it is known. Its own length is part of that, hence the two steps.
        byte[] boundSheet = BoundSheetRecord(0);
        int offset = globals.Count + boundSheet.Length + 4;
        globals.AddRange(BoundSheetRecord(offset));
        globals.AddRange(Record(Eof, []));
        globals.AddRange(sheet);

        List<Diagnostic> diagnostics = [];
        XlsWorkbookReader reader = new([.. globals], diagnostics);
        reader.Read();

        reader.Layouts.Count.ShouldBe(1);
        diagnostics.ShouldBeEmpty();
        return reader.Layouts[0].Drawings.Items;
    }

    private static byte[] BoundSheetRecord(int offset)
    {
        byte[] body = new byte[4 + 2 + 2 + 5];
        BinaryPrimitives.WriteInt32LittleEndian(body, offset);
        body[6] = 5;      // cch
        body[7] = 0;      // compressed
        "Sheet"u8.CopyTo(body.AsSpan(8));
        return Record(BoundSheet, body);
    }

    /// <summary>An <c>OBJ</c> record whose <c>ftCmo</c> declares an embedded chart.</summary>
    /// <remarks>
    /// A chart rather than a picture because a picture needs a blip in the workbook's
    /// <c>MSODRAWINGGROUP</c> to survive, and the shape's identity is what is under test rather
    /// than what it draws. Type 5 is <c>ftCmo</c>'s chart, and a chart object reaches the drawing
    /// list with no substream behind it — which is what keeps a sheet holding only a chart from
    /// losing its printed range.
    /// </remarks>
    private static byte[] ChartObject(ushort id)
    {
        byte[] body = new byte[4 + 18];
        BinaryPrimitives.WriteUInt16LittleEndian(body, 0x0015);       // ft = ftCmo
        BinaryPrimitives.WriteUInt16LittleEndian(body.AsSpan(2), 18); // cb
        BinaryPrimitives.WriteUInt16LittleEndian(body.AsSpan(4), 5);  // ot = chart
        BinaryPrimitives.WriteUInt16LittleEndian(body.AsSpan(6), id);
        BinaryPrimitives.WriteUInt16LittleEndian(body.AsSpan(8), 0x0010); // fPrint
        return Record(Obj, body);
    }

    /// <summary>
    /// A drawing container holding the patriarch and two anchored shapes, and where to cut it.
    /// </summary>
    /// <param name="split">
    /// The offset the second shape's container starts at, which is where Excel would have run out
    /// of record and started a <c>CONTINUE</c>.
    /// </param>
    private static byte[] Escher(out int split)
    {
        byte[] patriarch = Container(0xF004,
        [
            .. Dff(0xF009, 1, new byte[16]),
            .. Dff(0xF00A, 2, Sp(1024, 0x0005)),   // group | patriarch
        ]);

        byte[] first = Shape(1025);
        byte[] second = Shape(1026);

        byte[] group = Container(0xF003, [.. patriarch, .. first, .. second]);
        byte[] drawing = Container(0xF002, [.. Dff(0xF008, 0, [3, 0, 0, 0, 2, 4, 0, 0]), .. group]);

        // Everything up to the second shape goes in the first record; the second shape goes in the
        // continuation. The container headers are therefore incomplete in both halves, which is
        // the point — they only make sense once the payloads are concatenated.
        split = drawing.Length - second.Length;
        return drawing;
    }

    private static byte[] Shape(uint id) => Container(0xF004,
    [
        .. Dff(0xF00A, 75, Sp(id, 0x0A00)),  // instance 75 = picture frame; flags: has anchor
        .. Dff(0xF010, 0, new byte[18]),     // client anchor, all zeroes: cell A1
        .. Dff(0xF011, 0, []),               // client data: what makes it an OBJ's shape
    ]);

    private static byte[] Sp(uint id, uint flags)
    {
        byte[] body = new byte[8];
        BinaryPrimitives.WriteUInt32LittleEndian(body, id);
        BinaryPrimitives.WriteUInt32LittleEndian(body.AsSpan(4), flags);
        return body;
    }

    /// <summary>A DFF container record: version 0xF, its children as payload.</summary>
    private static byte[] Container(ushort type, byte[] children)
        => Dff(type, 0, children, version: 0xF);

    /// <summary>One DFF record: instance, version, type, length, payload.</summary>
    private static byte[] Dff(ushort type, int instance, byte[] payload, int version = 0)
    {
        byte[] record = new byte[8 + payload.Length];
        BinaryPrimitives.WriteUInt16LittleEndian(record, (ushort)((instance << 4) | version));
        BinaryPrimitives.WriteUInt16LittleEndian(record.AsSpan(2), type);
        BinaryPrimitives.WriteUInt32LittleEndian(record.AsSpan(4), (uint)payload.Length);
        payload.CopyTo(record.AsSpan(8));
        return record;
    }

    private static byte[] Record(ushort id, byte[] payload)
    {
        byte[] record = new byte[4 + payload.Length];
        BinaryPrimitives.WriteUInt16LittleEndian(record, id);
        BinaryPrimitives.WriteUInt16LittleEndian(record.AsSpan(2), (ushort)payload.Length);
        payload.CopyTo(record.AsSpan(4));
        return record;
    }
}

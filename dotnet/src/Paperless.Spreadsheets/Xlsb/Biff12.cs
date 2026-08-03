using System.Buffers.Binary;
using System.Text;

namespace Paperless.Spreadsheets.Xlsb;

/// <summary>
/// The BIFF12 record identifiers this reader acts on.
/// </summary>
/// <remarks>
/// <para>
/// Taken from <c>sc/source/filter/inc/biffhelper.hxx</c>, which is where LibreOffice keeps the
/// whole table. Only the records the reader does something with are named: an unknown record is
/// skipped by its stated length, which is what makes the format forward-compatible and what lets
/// this cover a real workbook without covering all two hundred ids.
/// </para>
/// <para>
/// <strong>A container's end id is its start id plus one</strong> throughout the format —
/// <c>SHEETS</c> is 0x008F and its end 0x0090, <c>CELLXFS</c> 0x0269 and its end 0x026A — which
/// is the convention LibreOffice's own <c>RecordInfo</c> tables state one pair at a time
/// (<c>{ BIFF12_ID_CELLXFS, BIFF12_ID_CELLXFS + 1 }</c>, <c>stylesfragment.cxx:325</c>). It is
/// worth knowing because it is what makes a context stack cheap: nothing has to be looked up to
/// know when a container closed.
/// </para>
/// </remarks>
internal static class Biff12
{
    // Workbook part.
    public const int Workbook = 0x0083;
    public const int WorkbookPr = 0x0099;
    public const int Sheets = 0x008F;
    public const int Sheet = 0x009C;

    // Shared strings part.
    public const int Sst = 0x009F;
    public const int Si = 0x0013;

    // Styles part.
    public const int NumFmts = 0x0267;
    public const int NumFmt = 0x002C;
    public const int Fonts = 0x0263;
    public const int Font = 0x002B;
    public const int CellXfs = 0x0269;
    public const int CellStyleXfs = 0x0272;
    public const int Xf = 0x002F;

    // Worksheet part.
    public const int Worksheet = 0x0081;
    public const int SheetPr = 0x0093;
    public const int Dimension = 0x0094;
    public const int SheetFormatPr = 0x01E5;
    public const int Cols = 0x0186;
    public const int Col = 0x003C;
    public const int SheetData = 0x0091;
    public const int Row = 0x0000;
    public const int MergeCells = 0x00B1;
    public const int MergeCell = 0x00B0;

    /// <summary>A hyperlink over a range of cells; <c>BIFF12_ID_HYPERLINK</c>.</summary>
    public const int HLink = 0x01EE;
    public const int PageMargins = 0x01DC;
    public const int PrintOptions = 0x01DD;
    public const int PageSetup = 0x01DE;
    public const int HeaderFooter = 0x01DF;
    public const int RowBreaks = 0x0188;
    public const int ColBreaks = 0x018A;
    public const int Brk = 0x018C;

    // Cell records. The three families differ only in how the column is stated: a CELL_* names
    // it, a MULTCELL_* continues from the previous cell, and a FORMULA_* names it and carries a
    // token array after the cached result.
    public const int CellBlank = 0x0001;
    public const int CellRk = 0x0002;
    public const int CellError = 0x0003;
    public const int CellBool = 0x0004;
    public const int CellDouble = 0x0005;
    public const int CellString = 0x0006;
    public const int CellSi = 0x0007;
    public const int CellRString = 0x003E;

    public const int FormulaString = 0x0008;
    public const int FormulaDouble = 0x0009;
    public const int FormulaBool = 0x000A;
    public const int FormulaError = 0x000B;

    public const int MultCellBlank = 0x000C;
    public const int MultCellRk = 0x000D;
    public const int MultCellError = 0x000E;
    public const int MultCellBool = 0x000F;
    public const int MultCellDouble = 0x0010;
    public const int MultCellString = 0x0011;
    public const int MultCellSi = 0x0012;
    public const int MultCellRString = 0x003D;
}

/// <summary>One record: an identifier and the bytes between its header and the next.</summary>
/// <param name="Id">The record identifier.</param>
/// <param name="Data">Its payload.</param>
internal readonly record struct Biff12Record(int Id, ReadOnlyMemory<byte> Data);

/// <summary>
/// Splits a BIFF12 part into records.
/// </summary>
/// <remarks>
/// <para>
/// The framing is two variable-length integers and then the payload: seven bits of value per
/// byte, low group first, the top bit meaning "another byte follows", up to four bytes each.
/// <c>lclReadCompressedInt</c> and <c>lclReadRecordHeader</c>
/// (<c>oox/source/core/recordparser.cxx:255-290</c>) are the whole of it, and the identifier and
/// the size are read by the same routine — which matters, because the identifier is one byte
/// below 0x80 and two above it, so a reader that assumes two bytes desynchronises on the first
/// record of every part.
/// </para>
/// <para>
/// Whole-part rather than streamed. A worksheet part is the same order of size as the XML it
/// replaces and the reader walks it twice — once for cells and once for the print setup — so
/// holding it costs one copy and saves reopening the package part.
/// </para>
/// </remarks>
internal static class Biff12Stream
{
    /// <summary>The records of one part, in order, stopping at the first malformed header.</summary>
    /// <remarks>
    /// A truncated part yields what it had rather than throwing: a workbook whose last sheet was
    /// cut off still has readable earlier ones, which is the same leniency the BIFF8 reader
    /// applies to a damaged record stream.
    /// </remarks>
    public static IEnumerable<Biff12Record> Records(ReadOnlyMemory<byte> part)
    {
        int at = 0;
        while (at < part.Length)
        {
            if (!TryReadCompressed(part.Span, ref at, out int id)) yield break;
            if (!TryReadCompressed(part.Span, ref at, out int size)) yield break;
            if (size < 0 || at + size > part.Length) yield break;

            yield return new Biff12Record(id, part.Slice(at, size));
            at += size;
        }
    }

    private static bool TryReadCompressed(ReadOnlySpan<byte> data, ref int at, out int value)
    {
        value = 0;
        for (int part = 0; part < 4; part++)
        {
            if (at >= data.Length) return false;

            byte b = data[at++];
            value |= (b & 0x7F) << (7 * part);
            if ((b & 0x80) == 0) return value >= 0;
        }
        return value >= 0;
    }
}

/// <summary>
/// A cursor over one record's payload.
/// </summary>
/// <remarks>
/// <para>
/// Every read is bounds-checked and a read past the end yields zero rather than throwing, which
/// is exactly what <c>SequenceInputStream</c> does once its <c>isEof</c> flag is set — and the
/// importers depend on it: several of them read a trailing field that older writers omit.
/// </para>
/// <para>
/// A struct over a span, so a record costs no allocation. It is passed by reference throughout
/// because the position advances.
/// </para>
/// </remarks>
internal ref struct Biff12Cursor(ReadOnlySpan<byte> data)
{
    private readonly ReadOnlySpan<byte> _data = data;
    private int _at;

    /// <summary>How many bytes are left.</summary>
    public readonly int Remaining => Math.Max(0, _data.Length - _at);

    /// <summary>True once a read has run past the end of the record.</summary>
    public bool IsExhausted { get; private set; }

    /// <summary>Skips forward, clamped to the end of the record.</summary>
    public void Skip(int count) => _at = Math.Min(_data.Length, _at + Math.Max(0, count));

    public byte ReadByte()
    {
        if (_at + 1 > _data.Length) { IsExhausted = true; return 0; }
        return _data[_at++];
    }

    public ushort ReadUInt16()
    {
        if (_at + 2 > _data.Length) { IsExhausted = true; _at = _data.Length; return 0; }
        ushort value = BinaryPrimitives.ReadUInt16LittleEndian(_data[_at..]);
        _at += 2;
        return value;
    }

    public int ReadInt32()
    {
        if (_at + 4 > _data.Length) { IsExhausted = true; _at = _data.Length; return 0; }
        int value = BinaryPrimitives.ReadInt32LittleEndian(_data[_at..]);
        _at += 4;
        return value;
    }

    public uint ReadUInt32() => unchecked((uint)ReadInt32());

    public double ReadDouble()
    {
        if (_at + 8 > _data.Length) { IsExhausted = true; _at = _data.Length; return 0; }
        double value = BinaryPrimitives.ReadDoubleLittleEndian(_data[_at..]);
        _at += 8;
        return value;
    }

    /// <summary>
    /// A BIFF12 string: a character count and that many UTF-16 code units.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A count of −1 means "no string", not "a string of length −1"</strong> —
    /// <c>BiffHelper::readString</c> says so in as many words
    /// (<c>sc/source/filter/oox/biffhelper.cxx:86-89</c>), and it is written by real files for an
    /// optional field such as a sheet's code name. Reading the count as unsigned turns it into
    /// four billion characters and takes the rest of the part with it.
    /// </para>
    /// <para>
    /// The count is clamped to what the record actually holds, which is the same guard
    /// LibreOffice applies (<c>min(nCharCount, getRemaining() / 2)</c>), and a NUL is replaced by
    /// a question mark rather than terminating the string, because BIFF12 strings are counted
    /// rather than terminated and a NUL inside one is a producer defect rather than an end.
    /// </para>
    /// </remarks>
    public string ReadString()
    {
        int count = ReadInt32();
        if (count <= 0) return string.Empty;

        count = Math.Min(count, Remaining / 2);
        if (count <= 0) return string.Empty;

        StringBuilder text = new(count);
        for (int at = 0; at < count; at++)
        {
            char character = (char)ReadUInt16();
            text.Append(character == '\0' ? '?' : character);
        }
        return text.ToString();
    }

    /// <summary>A cell range: first row, last row, first column, last column, each a signed int.</summary>
    /// <remarks>
    /// The row pair comes before the column pair, which is the reverse of how a range is written
    /// down (<c>BinRange::read</c>, <c>sc/source/filter/oox/addressconverter.cxx:59</c>). Reading
    /// them in the written order gives a plausible range that is wrong on every non-square block.
    /// </remarks>
    public (int FirstRow, int LastRow, int FirstColumn, int LastColumn) ReadRange()
    {
        int firstRow = ReadInt32();
        int lastRow = ReadInt32();
        int firstColumn = ReadInt32();
        int lastColumn = ReadInt32();
        return (firstRow, lastRow, firstColumn, lastColumn);
    }
}

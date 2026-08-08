namespace Paperless.Spreadsheets.MsBinary;

/// <summary>
/// A rectangle a chart's <c>CHSOURCELINK</c> names, before its sheet has been resolved.
/// </summary>
/// <remarks>
/// The sheet is carried as the <c>ixti</c> the token wrote rather than as an index, because
/// resolving one needs the <c>EXTERNSHEET</c> table and that table is read from the workbook
/// globals — which a token decoder has no business reaching into. <see cref="XlsExternSheets"/>
/// turns it into a sheet index, or into nothing when the reference is to another workbook.
/// </remarks>
/// <param name="Ixti">The index into the <c>EXTERNSHEET</c> table, or −1 for a reference with
/// no sheet part at all, which means the sheet the chart itself sits on.</param>
/// <param name="FirstRow">The first row, zero-based.</param>
/// <param name="LastRow">The last row, inclusive.</param>
/// <param name="FirstColumn">The first column, zero-based.</param>
/// <param name="LastColumn">The last column, inclusive.</param>
internal readonly record struct XlsChartRange(
    int Ixti,
    int FirstRow,
    int LastRow,
    int FirstColumn,
    int LastColumn)
{
    /// <summary>How many cells the rectangle covers.</summary>
    public int Count => (LastRow - FirstRow + 1) * (LastColumn - FirstColumn + 1);

    /// <summary>
    /// The cells, in the order a series reads them.
    /// </summary>
    /// <remarks>
    /// Row-major, which for the single column or single row a real chart link names is simply
    /// "down" or "across". A block reference is not something Excel writes for a series, and
    /// row-major is what Calc's own <c>ScRange</c> iteration would give if it did.
    /// </remarks>
    public IEnumerable<(int Row, int Column)> Cells()
    {
        for (int row = FirstRow; row <= LastRow; row++)
        {
            for (int column = FirstColumn; column <= LastColumn; column++) yield return (row, column);
        }
    }
}

/// <summary>
/// Decodes the formula a <c>CHSOURCELINK</c> carries into the one rectangle it names.
/// </summary>
/// <remarks>
/// <para>
/// A chart's series does not state a range; it states an RPN token array that happens to consist
/// of a single reference token, which is what <c>XclImpChSourceLink::ReadChSourceLink</c> hands
/// to the formula compiler with <c>EXC_FMLATYPE_CHART</c>
/// (<c>sc/source/filter/excel/xichart.cxx:744-761</c>). Only that shape is decoded here: a link
/// whose formula is an expression names no rectangle a cache can be read out of, and a decoder
/// that guessed would plot the wrong column with no way to tell.
/// </para>
/// <para>
/// This is deliberately not <see cref="XlsNameRanges"/> reused. That decoder answers "which
/// rectangles does this name cover", folds a union of several into a list and discards the
/// sheet part on purpose; a chart link needs exactly the opposite — one rectangle, and the sheet
/// part is the whole difficulty.
/// </para>
/// </remarks>
internal static class XlsChartFormula
{
    /// <summary>Reads the rectangle out of the next <paramref name="length"/> bytes.</summary>
    /// <param name="stream">The record stream, positioned at the token array.</param>
    /// <param name="length">How many bytes the token array occupies.</param>
    /// <param name="version">Which BIFF generation the token layouts follow.</param>
    /// <returns>The rectangle, or null when the formula is not a single reference.</returns>
    public static XlsChartRange? Read(BiffRecordReader stream, int length, BiffVersion version)
    {
        ArgumentNullException.ThrowIfNull(stream);

        if (length <= 0 || length > stream.RecordLeft) return null;

        byte[] tokens = stream.ReadBytes(length);
        if (tokens.Length == 0) return null;

        // The three "classes" of a reference token — reference, value and array — differ only in
        // the top two bits of the opcode and describe the same rectangle.
        byte opcode = tokens[0];
        byte kind = (byte)(opcode >= 0x20 ? opcode & 0x3F : opcode);
        bool biff8 = version == BiffVersion.Biff8;

        return kind switch
        {
            // tArea3d: ixti, then the same four fields tArea holds.
            0x3B when biff8 && tokens.Length >= 11 => Area(tokens, 3, ixti: Word(tokens, 1), biff8: true),
            0x3B when !biff8 && tokens.Length >= 17 => Area(tokens, 11, ixti: Word(tokens, 1), biff8: false),

            // tRef3d: ixti, then one cell.
            0x3A when biff8 && tokens.Length >= 7 => Cell(tokens, 3, ixti: Word(tokens, 1), biff8: true),
            0x3A when !biff8 && tokens.Length >= 14 => Cell(tokens, 11, ixti: Word(tokens, 1), biff8: false),

            // tArea and tRef, which name no sheet: the chart's own.
            0x25 when biff8 && tokens.Length >= 9 => Area(tokens, 1, ixti: -1, biff8: true),
            0x25 when !biff8 && tokens.Length >= 7 => Area(tokens, 1, ixti: -1, biff8: false),
            0x24 when biff8 && tokens.Length >= 5 => Cell(tokens, 1, ixti: -1, biff8: true),
            0x24 when !biff8 && tokens.Length >= 4 => Cell(tokens, 1, ixti: -1, biff8: false),

            _ => null,
        };
    }

    private static ushort Word(byte[] tokens, int at) => (ushort)(tokens[at] | (tokens[at + 1] << 8));

    /// <summary>
    /// An area token's rectangle: two rows then two columns.
    /// </summary>
    /// <remarks>
    /// BIFF8 gives each column two bytes, whose top two bits are the relative-reference flags
    /// and are masked off; BIFF5 gives each one byte and keeps the flags in the row instead.
    /// </remarks>
    private static XlsChartRange Area(byte[] tokens, int at, int ixti, bool biff8)
    {
        int firstRow = Word(tokens, at);
        int lastRow = Word(tokens, at + 2);
        int firstColumn;
        int lastColumn;

        if (biff8)
        {
            firstColumn = Word(tokens, at + 4) & 0x3FFF;
            lastColumn = Word(tokens, at + 6) & 0x3FFF;
        }
        else
        {
            firstRow &= 0x3FFF;
            lastRow &= 0x3FFF;
            firstColumn = tokens[at + 4];
            lastColumn = tokens[at + 5];
        }

        return Normalise(ixti, firstRow, lastRow, firstColumn, lastColumn);
    }

    private static XlsChartRange Cell(byte[] tokens, int at, int ixti, bool biff8)
    {
        int row = Word(tokens, at);
        int column;

        if (biff8)
        {
            column = Word(tokens, at + 2) & 0x3FFF;
        }
        else
        {
            row &= 0x3FFF;
            column = tokens[at + 2];
        }

        return Normalise(ixti, row, row, column, column);
    }

    /// <summary>Puts the corners in order and refuses a rectangle no sheet could hold.</summary>
    private static XlsChartRange Normalise(int ixti, int firstRow, int lastRow, int firstColumn, int lastColumn)
        => new(
            ixti,
            Math.Min(firstRow, lastRow),
            Math.Max(firstRow, lastRow),
            Math.Min(firstColumn, lastColumn),
            Math.Max(firstColumn, lastColumn));
}

/// <summary>
/// The workbook's <c>SUPBOOK</c> and <c>EXTERNSHEET</c> tables, which are what turn a token's
/// <c>ixti</c> into a sheet.
/// </summary>
/// <remarks>
/// <para>
/// A BIFF8 3D reference names its sheet twice indirectly: the token holds an index into the
/// <c>EXTERNSHEET</c> table, whose entries hold a <c>SUPBOOK</c> index and a range of sheet
/// positions <em>within that supbook</em>. A supbook is a workbook — usually this one, written
/// as the two-byte marker <c>0x0401</c> — so only for that case does a sheet position mean an
/// index into this file's own <c>BOUNDSHEET</c> order.
/// </para>
/// <para>
/// A reference into another workbook resolves to nothing here rather than to a guess. Excel
/// caches the values of an external link in the <c>SUPBOOK</c> itself, and reading that cache
/// is a separate feature; what must not happen is a chart plotting this workbook's sheet 2
/// because another workbook's sheet 2 was named.
/// </para>
/// <para>
/// <strong>Several <c>EXTERNSHEET</c> records, and the order they combine in.</strong> The
/// format allows only one, and LibreOffice carries a note that a third-party writer emits
/// several — inserting each new record's entries <em>before</em> the ones already read, because
/// that is what Excel does (<c>XclImpLinkManagerImpl::ReadExternsheet</c>, i#104057,
/// <c>sc/source/filter/excel/xilink.cxx:818-838</c>). Reproduced rather than tidied.
/// </para>
/// </remarks>
internal sealed class XlsExternSheets
{
    private readonly List<bool> _supbookIsSelf = [];
    private readonly List<(int Supbook, int First, int Last)> _entries = [];

    /// <summary>True when nothing has been read, so no reference can resolve.</summary>
    public bool IsEmpty => _entries.Count == 0;

    /// <summary>Reads one <c>SUPBOOK</c> record, which is one workbook the file refers to.</summary>
    /// <remarks>
    /// Only whether it is this workbook is kept. The record's remaining shape — an encoded URL
    /// and a list of sheet names — is what an external-link reader would need and is on the
    /// module's TODO; a self supbook has neither, its whole payload being the sheet count and
    /// the marker.
    /// </remarks>
    public void ReadSupBook(BiffRecordReader stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        stream.Skip(2);
        bool self = stream.RecordLeft == 2 && stream.ReadUInt16() == SelfMarker;
        _supbookIsSelf.Add(self);
    }

    /// <summary>Reads one <c>EXTERNSHEET</c> record: the table of <c>ixti</c> entries.</summary>
    public void ReadExternSheet(BiffRecordReader stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        int count = stream.ReadUInt16();
        count = Math.Min(count, stream.RecordLeft / 6);
        if (count <= 0) return;

        List<(int, int, int)> read = new(count);
        for (int at = 0; at < count; at++)
        {
            int supbook = stream.ReadUInt16();
            int first = stream.ReadUInt16();
            int last = stream.ReadUInt16();
            read.Add((supbook, first, last));
        }

        _entries.InsertRange(0, read);
    }

    /// <summary>
    /// Which sheet of this workbook an <c>ixti</c> names, or null when it names none.
    /// </summary>
    /// <param name="ixti">The token's index into the table.</param>
    public int? SheetOf(int ixti)
    {
        if (ixti < 0 || ixti >= _entries.Count) return null;

        (int supbook, int first, int last) = _entries[ixti];
        if (supbook < 0 || supbook >= _supbookIsSelf.Count || !_supbookIsSelf[supbook]) return null;

        // A deleted sheet is written as 0xFFFF, and a span covering several sheets is a 3D sum
        // rather than anything a chart series names. The first sheet is what a single-sheet
        // reference resolves to and is also what Calc takes as the range's own sheet.
        if (first is DeletedSheet or > MaxSheets || last is DeletedSheet) return null;

        return first;
    }

    /// <summary>The two-byte marker a <c>SUPBOOK</c> uses to mean "this workbook".</summary>
    private const ushort SelfMarker = 0x0401;

    private const int DeletedSheet = 0xFFFF;

    /// <summary>More sheets than any BIFF workbook holds; a larger index is corruption.</summary>
    private const int MaxSheets = 0x0FFF;
}

/// <summary>
/// The cell values a workbook's charts need, gathered while its sheets are read.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why this exists at all, rather than a chart simply reading the cells it names.</strong>
/// A chart substream is embedded in the sheet its picture sits on, and the cells it plots are
/// routinely on a different sheet — one that may not have been read yet. The link is therefore
/// resolved in two passes: a scan finds every rectangle any chart in the workbook names before
/// the first sheet is read, and the sheet reader then offers exactly those cells as it meets
/// them. Nothing else is retained, which is what keeps a workbook of a million cells from
/// paying for a chart of fifty.
/// </para>
/// <para>
/// Both halves of a cell are kept. A series plots the number; a category axis prints the text
/// the authoring application displayed, which for the date column a flight log plots against is
/// not the serial and not its shortest round-trip form either.
/// </para>
/// </remarks>
internal sealed class XlsChartData
{
    private readonly Dictionary<int, List<XlsChartRange>> _wanted = [];
    private readonly Dictionary<(int Sheet, int Row, int Column), (double? Number, string Text)> _cells = [];

    /// <summary>True when no chart in the workbook names a cell, so no sheet need offer any.</summary>
    public bool IsEmpty => _wanted.Count == 0;

    /// <summary>Which sheets hold cells some chart plots, in ascending order.</summary>
    public IEnumerable<int> Sheets()
    {
        List<int> sheets = [.. _wanted.Keys];
        sheets.Sort();
        return sheets;
    }

    /// <summary>Notes that some chart plots a rectangle of a sheet.</summary>
    public void Want(int sheet, XlsChartRange range)
    {
        if (sheet < 0 || range.Count <= 0 || range.Count > MaxCellsPerRange) return;

        if (!_wanted.TryGetValue(sheet, out List<XlsChartRange>? ranges))
        {
            ranges = [];
            _wanted[sheet] = ranges;
        }

        if (!ranges.Contains(range)) ranges.Add(range);
    }

    /// <summary>Whether any chart names this cell.</summary>
    /// <remarks>
    /// Asked once per cell of every sheet of a workbook that holds a chart, so the miss has to
    /// be cheap: a sheet no chart reads at all costs one dictionary probe, and a sheet one does
    /// costs a walk of its handful of rectangles.
    /// </remarks>
    public bool Wants(int sheet, int row, int column)
    {
        if (_wanted.Count == 0 || !_wanted.TryGetValue(sheet, out List<XlsChartRange>? ranges)) return false;

        foreach (XlsChartRange range in ranges)
        {
            if (row >= range.FirstRow && row <= range.LastRow
                && column >= range.FirstColumn && column <= range.LastColumn)
            {
                return true;
            }
        }

        return false;
    }

    /// <summary>Records a wanted cell's value and the text its sheet shows for it.</summary>
    public void Offer(int sheet, int row, int column, double? number, string text)
        => _cells[(sheet, row, column)] = (number, text ?? string.Empty);

    /// <summary>The numbers a rectangle holds, one per cell, null where a cell has none.</summary>
    public List<double?> Numbers(int sheet, XlsChartRange range)
    {
        List<double?> values = [];
        foreach ((int row, int column) in range.Cells())
        {
            values.Add(_cells.TryGetValue((sheet, row, column), out (double? Number, string Text) cell)
                ? cell.Number
                : null);
        }

        return values;
    }

    /// <summary>The displayed text a rectangle holds, one per cell.</summary>
    public List<string?> Texts(int sheet, XlsChartRange range)
    {
        List<string?> values = [];
        foreach ((int row, int column) in range.Cells())
        {
            values.Add(_cells.TryGetValue((sheet, row, column), out (double? Number, string Text) cell)
                ? cell.Text
                : null);
        }

        return values;
    }

    /// <summary>The displayed text of one cell, or null when it holds none.</summary>
    public string? TextOf(int sheet, int row, int column)
        => _cells.TryGetValue((sheet, row, column), out (double? Number, string Text) cell)
            ? cell.Text
            : null;

    /// <summary>
    /// The largest rectangle a chart link is believed rather than ignored.
    /// </summary>
    /// <remarks>
    /// A whole-column reference in BIFF8 covers 65536 cells and is what a corrupt or
    /// over-general link looks like; a real series names the rows it has. The cap is generous
    /// enough for the longest series in the corpus, which is 615 points, and it bounds what one
    /// bad token can make a sheet read remember.
    /// </remarks>
    private const int MaxCellsPerRange = 65536;
}

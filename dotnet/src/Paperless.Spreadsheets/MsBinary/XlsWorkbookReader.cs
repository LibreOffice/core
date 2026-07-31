using Paperless.Core.Diagnostics;
using Paperless.Core.Extraction;
using Paperless.Spreadsheets.Numbers;
using Paperless.Text.Encodings;

namespace Paperless.Spreadsheets.MsBinary;

/// <summary>
/// Reads the <c>Workbook</c> stream of a legacy Excel file into the content tree.
/// </summary>
/// <remarks>
/// <para>
/// A BIFF workbook is a flat record stream cut into substreams by <c>BOF</c>/<c>EOF</c>
/// pairs: one for the workbook globals, then one per sheet, plus substreams for charts,
/// macro sheets and VB modules that a content reader has no use for. The globals hold
/// everything shared — the shared string table, the number formats, the cell formats, and
/// the <c>BOUNDSHEET</c> records that name each sheet and say where its substream starts.
/// </para>
/// <para>
/// Nothing here decodes formula tokens. A cell's <c>FORMULA</c> record carries the result
/// its authoring application last computed, and that cached result is what a reference
/// renderer shows, so extraction reports it and reports the formula as absent rather than
/// as a wrong expression. The consequence is visible and deliberate:
/// <see cref="ContentTableCell.Formula"/> is null for a BIFF workbook where it is populated
/// for an ODF one.
/// </para>
/// </remarks>
internal sealed class XlsWorkbookReader
{
    /// <summary>The most cells one sheet may materialise.</summary>
    /// <remarks>
    /// A guard against a corrupt row or column index, not against a large workbook: BIFF8
    /// caps a sheet at 256 columns by 65536 rows, so a well-formed sheet cannot reach this.
    /// </remarks>
    public const int MaxCellsPerSheet = 4_000_000;

    /// <summary>How many consecutive empty rows are materialised inside a sheet's used range.</summary>
    /// <remarks>
    /// Matches the ODS reader's own limit, so the two produce the same tree for the same
    /// workbook. Blank rows between content are worth keeping — they are blank lines in the
    /// extracted text — but a run longer than this is a gap, not layout.
    /// </remarks>
    public const int MaxConsecutiveEmptyRows = 4096;

    private readonly BiffStream _stream;
    private readonly List<Diagnostic> _diagnostics;
    private readonly List<string> _sharedStrings = [];
    private readonly List<SheetEntry> _sheets = [];
    private readonly List<XfRecord> _formats = [];
    private readonly Dictionary<int, string> _formatCodes = [];
    private readonly Dictionary<int, NumberFormatCode> _parsedFormats = [];
    private bool _reportedFormat;
    private bool _reportedSstIndex;

    public XlsWorkbookReader(byte[] workbook, List<Diagnostic> diagnostics)
    {
        _diagnostics = diagnostics;
        _stream = new BiffStream(workbook, diagnostics);
    }

    /// <summary>The BIFF generation the file turned out to be.</summary>
    public BiffVersion Version => _stream.Version;

    /// <summary>How many sheets the workbook declares, hidden ones included.</summary>
    public int SheetCount => _sheets.Count;

    /// <summary>True when the workbook counts days from 1904 rather than from 1900.</summary>
    public bool Uses1904Epoch { get; private set; }

    /// <summary>True when a <c>FILEPASS</c> record says the workbook is encrypted.</summary>
    public bool IsEncrypted { get; private set; }

    /// <summary>
    /// Reads the workbook, producing one section per worksheet in sheet order.
    /// </summary>
    public List<ContentSection> Read()
    {
        List<ContentSection> sections = [];

        ReadGlobals();
        if (IsEncrypted) return sections;

        int index = 0;
        foreach (SheetEntry sheet in _sheets)
        {
            // Chart, macro and Visual Basic substreams carry no cells. They keep their place
            // in the sheet order — a workbook's third sheet is still its third — but produce
            // no section, exactly as LibreOffice's SkipSubStream does.
            if (sheet.Kind != SheetKind.Worksheet)
            {
                index++;
                continue;
            }

            sections.Add(ReadSheet(sheet, index));
            index++;
        }

        return sections;
    }

    /// <summary>
    /// Reads the workbook globals: the strings, formats and sheet directory everything else
    /// depends on.
    /// </summary>
    private void ReadGlobals()
    {
        if (!_stream.MoveNext()) return;

        // The first record is the workbook's BOF and it states the BIFF generation. Every
        // record layout after it depends on the answer, so it is read before anything else.
        if (BiffRecords.IsBof(_stream.RecordId)) ReadBof();

        while (_stream.MoveNext())
        {
            switch (_stream.RecordId)
            {
                case BiffRecords.Eof:
                    return;

                case BiffRecords.FilePass:
                    IsEncrypted = true;
                    return;

                case BiffRecords.CodePage:
                    ReadCodePage();
                    break;

                case BiffRecords.DateMode:
                    Uses1904Epoch = _stream.ReadUInt16() != 0;
                    break;

                case BiffRecords.BoundSheet:
                    ReadBoundSheet();
                    break;

                case BiffRecords.Sst:
                    ReadSharedStrings();
                    break;

                case BiffRecords.Format or BiffRecords.Format2:
                    ReadFormat();
                    break;

                case BiffRecords.Xf or BiffRecords.Xf2 or BiffRecords.Xf3 or BiffRecords.Xf4:
                    ReadXf();
                    break;

                default:
                    break;
            }
        }
    }

    private void ReadBof()
    {
        ushort version = _stream.ReadUInt16();
        _stream.Version = version >= BiffRecords.VersionBiff8 ? BiffVersion.Biff8 : BiffVersion.Biff5;

        // BIFF2 to BIFF4 record ids overlap with BIFF5's but their payloads differ. Reading
        // them on the BIFF5 path recovers the cells that happen to agree and skips the rest,
        // which is better than refusing a file outright.
        if (version < BiffRecords.VersionBiff5 && _stream.RecordId != BiffRecords.Bof)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2321",
                $"The workbook is BIFF version 0x{version:X4} (Excel 4.0 or older). It is read "
                + "on the BIFF5 path, so records whose layout changed later may be skipped."));
        }
    }

    private void ReadCodePage()
    {
        int codePage = _stream.ReadUInt16();

        // 1200 means "the strings are UTF-16 already". Treating it as a code page would
        // reinterpret text that is already correct, so the marker is honoured by leaving the
        // 8-bit encoding at its default — which BIFF8 barely uses anyway.
        if (codePage == LegacyCodePages.Utf16Marker) return;

        _stream.Encoding = LegacyCodePages.Get(codePage, out bool resolved);
        if (!resolved)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2322",
                $"The workbook names code page {codePage}, which this platform cannot resolve; "
                + $"8-bit text is read as {_stream.Encoding.WebName} instead."));
        }
    }

    /// <summary>
    /// Reads one <c>BOUNDSHEET</c>: a sheet's name, its visibility, and where its substream
    /// begins.
    /// </summary>
    /// <remarks>
    /// The stream offset is absolute within the <c>Workbook</c> stream and is written after
    /// the fact by the exporter, which is why it is the field most likely to be wrong — see
    /// <see cref="ReadSheet"/>.
    /// </remarks>
    private void ReadBoundSheet()
    {
        int offset = _stream.ReadInt32();
        ushort flags = _stream.ReadUInt16();

        // The name's length is one byte in both generations; only what follows it differs.
        int length = _stream.ReadByte();
        string name = _stream.Version == BiffVersion.Biff8
            ? _stream.ReadUnicodeString(length)
            : _stream.ReadByteString(length);

        _sheets.Add(new SheetEntry
        {
            Name = name,
            Offset = offset,

            // Bit 0 is hidden and bit 1 is "very hidden", the state Excel's user interface
            // cannot undo. Both are hidden as far as a reader is concerned.
            IsHidden = (flags & 0x0003) != 0,
            Kind = (flags >> 8) switch
            {
                0x00 => SheetKind.Worksheet,
                0x02 => SheetKind.Chart,
                _ => SheetKind.Other,
            },
        });
    }

    /// <summary>Reads the shared string table.</summary>
    /// <remarks>
    /// Every string in a BIFF8 workbook that is not a formula result lives here, and cells
    /// reference it by index. It is also the record most likely to be split across
    /// <c>CONTINUE</c>s — one 8224-byte ceiling against a table that routinely runs to
    /// megabytes — so it is the record that exercises the continuation handling hardest.
    /// </remarks>
    private void ReadSharedStrings()
    {
        _stream.Skip(4); // Total reference count, which says nothing about the table's size.
        long declared = (uint)_stream.ReadInt32();

        // A count larger than the bytes left cannot be honest. Clamping to the byte count
        // rather than rejecting the record keeps the strings that are really there.
        long count = Math.Min(declared, _stream.RecordLeft);
        _sharedStrings.Capacity = (int)Math.Min(count, 65536);

        for (long i = 0; i < count && _stream.IsValid; i++)
            _sharedStrings.Add(_stream.ReadString(eightBitLength: false));

        if (declared > count)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2323",
                $"The shared string table declares {declared} strings but holds at most {count}; "
                + "the rest have been ignored."));
        }
    }

    private void ReadFormat()
    {
        int index;
        string code;

        if (_stream.Version == BiffVersion.Biff8)
        {
            index = _stream.ReadUInt16();
            code = _stream.ReadUnicodeString(_stream.ReadUInt16());
        }
        else if (_stream.RecordId == BiffRecords.Format)
        {
            index = _stream.ReadUInt16();
            code = _stream.ReadByteString(_stream.ReadByte());
        }
        else
        {
            // BIFF2/3 numbered formats by their order of appearance rather than stating an
            // index, so the next free slot above the built-ins is where this one lands.
            index = BuiltInNumberFormats.FirstUserIndex + _formatCodes.Count;
            code = _stream.ReadByteString(_stream.ReadByte());
        }

        _formatCodes[index] = code;
    }

    private void ReadXf()
    {
        XfRecord xf;

        if (_stream.Version == BiffVersion.Biff8 || _stream.RecordId == BiffRecords.Xf)
        {
            _stream.ReadUInt16(); // Font index; the content tree records no fonts.
            ushort numberFormat = _stream.ReadUInt16();
            ushort typeProtection = _stream.ReadUInt16();
            ushort alignment = _stream.ReadUInt16();

            // Where the "attribute used" bits live moved between BIFF5 and BIFF8: BIFF5 keeps
            // them in the alignment field, BIFF8 in a misc field that BIFF5 does not have.
            ushort used = _stream.Version == BiffVersion.Biff8 ? _stream.ReadUInt16() : alignment;

            xf = MakeXf(numberFormat, typeProtection, used);
        }
        else
        {
            // BIFF3/BIFF4 pack font and format into single bytes, and disagree with each
            // other about which field holds the parent index. Only the number format matters
            // here, and that is in the same place in both.
            _stream.ReadByte();
            ushort numberFormat = _stream.ReadByte();
            ushort typeProtection = _stream.ReadUInt16();
            ushort alignment = _stream.ReadUInt16();
            xf = MakeXf(numberFormat, typeProtection, alignment);
        }

        _formats.Add(xf);

        static XfRecord MakeXf(ushort numberFormat, ushort typeProtection, ushort used)
        {
            const ushort styleFlag = 0x0004;
            const byte differentNumberFormat = 0x01;

            bool isCellXf = (typeProtection & styleFlag) == 0;
            bool ownFormat = ((used >> 10) & differentNumberFormat) != 0;

            return new XfRecord
            {
                NumberFormatIndex = numberFormat,
                IsCellXf = isCellXf,

                // In a cell XF a set bit means "this XF states the attribute"; in a style XF
                // the sense is inverted. LibreOffice writes the same comparison
                // (XclImpXF::SetUsedFlags), and getting it backwards silently gives every
                // cell the default format.
                StatesNumberFormat = isCellXf == ownFormat,
                ParentIndex = (ushort)((typeProtection >> 4) & 0x0FFF),
            };
        }
    }

    /// <summary>
    /// The number format an XF index resolves to, following the parent style where the cell
    /// format does not state one of its own.
    /// </summary>
    private NumberFormatCode FormatOf(int xfIndex)
    {
        int index = NumberFormatIndexOf(xfIndex);
        if (_parsedFormats.TryGetValue(index, out NumberFormatCode? cached)) return cached;

        string? code = _formatCodes.TryGetValue(index, out string? stated)
            ? stated
            : BuiltInNumberFormats.Code(index);

        NumberFormatCode parsed = code is null ? NumberFormatCode.General : NumberFormatCode.Parse(code);

        if (!parsed.IsUnderstood && !_reportedFormat)
        {
            _reportedFormat = true;
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2324",
                $"The number format \"{parsed.Code}\" uses a construct Paperless does not "
                + "reproduce, so the text of the cells using it may differ from what the "
                + "authoring application showed."));
        }

        _parsedFormats[index] = parsed;
        return parsed;
    }

    private int NumberFormatIndexOf(int xfIndex)
    {
        int index = xfIndex;

        // Two hops at most: a cell XF may defer to its style XF, and a style XF has no
        // parent. The loop bound is what keeps a file whose parent chain is cyclic from
        // hanging the reader.
        for (int hop = 0; hop < 4; hop++)
        {
            if (index < 0 || index >= _formats.Count) return 0;
            XfRecord xf = _formats[index];
            if (xf.StatesNumberFormat || !xf.IsCellXf || xf.ParentIndex == index) return xf.NumberFormatIndex;
            index = xf.ParentIndex;
        }

        return 0;
    }

    /// <summary>Reads one sheet substream into a section.</summary>
    private ContentSection ReadSheet(SheetEntry sheet, int index)
    {
        SheetBuilder builder = new(this, sheet.Name);

        if (StartSubstream(sheet))
        {
            ReadSheetRecords(builder);
        }
        else
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2325",
                $"No sheet substream could be found for \"{sheet.Name}\"; the sheet is empty.",
                new DiagnosticLocation(PartName: "Workbook", Context: sheet.Name)));
        }

        ContentSection section = new()
        {
            Kind = SectionKind.Sheet,
            Index = index,
            Name = sheet.Name,

            // Hidden sheets are extracted and flagged rather than skipped, which is what the
            // ODS reader does: a caller indexing content wants them and a caller reproducing
            // what a reader sees can check the flag.
            IsHidden = sheet.IsHidden,
        };

        section.Children.Add(builder.Build());
        return section;
    }

    /// <summary>
    /// Positions the stream at a sheet's substream, falling back to a forward scan.
    /// </summary>
    /// <remarks>
    /// The <c>BOUNDSHEET</c> offset is not reliable. LibreOffice carries the same fallback
    /// with a note that third-party writers produce files whose offsets point at nothing
    /// (<c>sc/source/filter/excel/read.cxx:52-66</c>, i#115255): if the offset does not land
    /// on a <c>BOF</c>, scan forward from it until one turns up.
    /// </remarks>
    private bool StartSubstream(SheetEntry sheet)
    {
        if (BiffRecords.IsBof(_stream.PeekRecordId(sheet.Offset)) && _stream.MoveNext(sheet.Offset))
            return true;

        int from = sheet.Offset > 0 && sheet.Offset < _stream.Length ? sheet.Offset : 0;
        if (!_stream.MoveNext(from)) return false;

        _diagnostics.Add(new Diagnostic(
            DiagnosticSeverity.Warning, "PL2326",
            $"The directory entry for \"{sheet.Name}\" points at offset {sheet.Offset}, which is "
            + "not a BOF record; the substream was found by scanning forward instead.",
            new DiagnosticLocation(PartName: "Workbook", ByteOffset: sheet.Offset, Context: sheet.Name)));

        while (!BiffRecords.IsBof(_stream.RecordId))
        {
            if (!_stream.MoveNext()) return false;
        }

        return true;
    }

    private void ReadSheetRecords(SheetBuilder builder)
    {
        // A chart or a drawing embedded in a sheet opens its own BOF/EOF pair inside the
        // sheet's. Counting depth is what keeps the inner EOF from ending the sheet early.
        int depth = 0;

        while (_stream.MoveNext())
        {
            ushort id = _stream.RecordId;

            if (BiffRecords.IsBof(id))
            {
                depth++;
                continue;
            }

            if (id == BiffRecords.Eof)
            {
                if (depth == 0) return;
                depth--;
                continue;
            }

            if (depth > 0) continue;

            switch (id)
            {
                case BiffRecords.Blank or BiffRecords.Blank2:
                    ReadBlank(builder);
                    break;

                case BiffRecords.MulBlank:
                    ReadMulBlank(builder);
                    break;

                case BiffRecords.Number or BiffRecords.Number2:
                    ReadNumber(builder);
                    break;

                case BiffRecords.Integer2:
                    ReadInteger(builder);
                    break;

                case BiffRecords.Rk:
                    ReadRk(builder);
                    break;

                case BiffRecords.MulRk:
                    ReadMulRk(builder);
                    break;

                case BiffRecords.Label or BiffRecords.Label2 or BiffRecords.RString:
                    ReadLabel(builder, id);
                    break;

                case BiffRecords.LabelSst:
                    ReadLabelSst(builder);
                    break;

                case BiffRecords.BoolErr or BiffRecords.BoolErr2:
                    ReadBoolErr(builder);
                    break;

                case BiffRecords.Formula2 or BiffRecords.Formula3 or BiffRecords.Formula4:
                    ReadFormula(builder);
                    break;

                case BiffRecords.String or BiffRecords.String2:
                    ReadFormulaString(builder);
                    break;

                case BiffRecords.MergedCells:
                    ReadMergedCells(builder);
                    break;

                case BiffRecords.Dimensions or BiffRecords.Dimensions2:
                    ReadDimensions(builder);
                    break;

                default:
                    break;
            }
        }
    }

    private void ReadDimensions(SheetBuilder builder)
    {
        // BIFF8 widened the row fields to 32 bits; the older layout is four 16-bit fields.
        // The last row and column are the first *unused* ones, so both are exclusive bounds.
        int firstRow;
        int lastRow;
        int firstColumn;
        int lastColumn;

        if (_stream.Version == BiffVersion.Biff8 && _stream.RecordId == BiffRecords.Dimensions)
        {
            firstRow = _stream.ReadInt32();
            lastRow = _stream.ReadInt32();
            firstColumn = _stream.ReadUInt16();
            lastColumn = _stream.ReadUInt16();
        }
        else
        {
            firstRow = _stream.ReadUInt16();
            lastRow = _stream.ReadUInt16();
            firstColumn = _stream.ReadUInt16();
            lastColumn = _stream.ReadUInt16();
        }

        builder.SetDeclaredExtent(firstRow, lastRow, firstColumn, lastColumn);
    }

    private (int Row, int Column, int Xf) ReadCellHeader()
        => (_stream.ReadUInt16(), _stream.ReadUInt16(), _stream.ReadUInt16());

    private void ReadBlank(SheetBuilder builder)
    {
        (int row, int column, int xf) = ReadCellHeader();
        builder.SetBlank(row, column, xf);
    }

    /// <summary>Reads a run of blank cells sharing one row.</summary>
    /// <remarks>
    /// The run's last column is a trailing field after the array, so the loop reads pairs
    /// until only that field is left rather than trusting a count — which is what LibreOffice
    /// does too (<c>ImportExcel::Mulblank</c>), and what keeps a truncated record from
    /// running past its end.
    /// </remarks>
    private void ReadMulBlank(SheetBuilder builder)
    {
        int row = _stream.ReadUInt16();
        int column = _stream.ReadUInt16();

        while (_stream.RecordLeft > 2 && _stream.IsValid)
        {
            builder.SetBlank(row, column, _stream.ReadUInt16());
            column++;
        }
    }

    private void ReadNumber(SheetBuilder builder)
    {
        (int row, int column, int xf) = ReadCellHeader();
        builder.SetNumber(row, column, xf, _stream.ReadDouble());
    }

    private void ReadInteger(SheetBuilder builder)
    {
        (int row, int column, int xf) = ReadCellHeader();
        builder.SetNumber(row, column, xf, _stream.ReadUInt16());
    }

    private void ReadRk(SheetBuilder builder)
    {
        (int row, int column, int xf) = ReadCellHeader();
        builder.SetNumber(row, column, xf, RkValue(_stream.ReadInt32()));
    }

    private void ReadMulRk(SheetBuilder builder)
    {
        int row = _stream.ReadUInt16();
        int column = _stream.ReadUInt16();

        while (_stream.RecordLeft > 2 && _stream.IsValid)
        {
            int xf = _stream.ReadUInt16();
            builder.SetNumber(row, column, xf, RkValue(_stream.ReadInt32()));
            column++;
        }
    }

    /// <summary>
    /// Decodes an RK number: Excel's packed 32-bit encoding for the numbers that do not need
    /// a full double.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The low two bits are flags and the top thirty are the value. Bit 1 chooses between the
    /// two encodings: clear, and the thirty bits are the <em>high</em> half of an IEEE double
    /// whose low half is zero; set, and they are a signed integer. Bit 0 says the decoded
    /// value is a hundred times too large, which is how two decimal places are stored
    /// compactly.
    /// </para>
    /// <para>
    /// This is the routine to get wrong quietly. Every combination of the two flags produces
    /// a plausible number from the same bits, so a reader that treats the integer form as a
    /// double, or forgets the hundredths flag, yields a workbook full of numbers that are
    /// merely wrong rather than obviously broken. Ported from
    /// <c>XclTools::GetDoubleFromRK</c>.
    /// </para>
    /// </remarks>
    public static double RkValue(int encoded)
    {
        const int hundredthsFlag = 0x00000001;
        const int integerFlag = 0x00000002;

        double value = (encoded & integerFlag) != 0
            ? encoded >> 2
            : BitConverter.UInt64BitsToDouble((ulong)(uint)(encoded & ~0x00000003) << 32);

        return (encoded & hundredthsFlag) != 0 ? value / 100.0 : value;
    }

    private void ReadLabel(SheetBuilder builder, ushort id)
    {
        (int row, int column, int xf) = ReadCellHeader();

        // The BIFF2 spelling of LABEL carries a three-byte cell header, so the XF index read
        // above consumed one byte too many. It is the only record where that is true.
        bool eightBitLength = id == BiffRecords.Label2 && _stream.Version == BiffVersion.Biff5;
        string text = _stream.ReadString(eightBitLength);
        builder.SetText(row, column, xf, text);
    }

    private void ReadLabelSst(SheetBuilder builder)
    {
        (int row, int column, int xf) = ReadCellHeader();
        int index = _stream.ReadInt32();

        if (index >= 0 && index < _sharedStrings.Count)
        {
            builder.SetText(row, column, xf, _sharedStrings[index]);
            return;
        }

        // A cell pointing outside the table is a damaged file, not an empty cell, so it
        // becomes a blank rather than being dropped: the row still has the right shape.
        builder.SetBlank(row, column, xf);
        if (_reportedSstIndex) return;

        _reportedSstIndex = true;
        _diagnostics.Add(new Diagnostic(
            DiagnosticSeverity.Error, "PL2327",
            $"A cell references shared string {index}, but the table holds "
            + $"{_sharedStrings.Count}; the cell has been left empty.",
            new DiagnosticLocation(PartName: "Workbook", Context: builder.SheetName)));
    }

    private void ReadBoolErr(SheetBuilder builder)
    {
        (int row, int column, int xf) = ReadCellHeader();
        byte value = _stream.ReadByte();
        byte kind = _stream.ReadByte();

        if (kind == 0) builder.SetBoolean(row, column, xf, value != 0);
        else builder.SetError(row, column, xf, BiffErrors.ToCellError(value));
    }

    /// <summary>Reads a formula cell, keeping its cached result and discarding its tokens.</summary>
    /// <remarks>
    /// The eight bytes of result are a double unless their last two bytes are 0xFFFF, which
    /// no finite double has: then the first byte says which of string, boolean, error or
    /// empty the result is. A string result is not in this record at all — it arrives in the
    /// <c>STRING</c> record that follows.
    /// </remarks>
    private void ReadFormula(SheetBuilder builder)
    {
        (int row, int column, int xf) = ReadCellHeader();
        byte[] result = _stream.ReadBytes(8);
        _stream.SkipRecord();

        if (result.Length < 8) return;

        bool special = result[6] == 0xFF && result[7] == 0xFF;
        if (!special)
        {
            builder.SetNumber(row, column, xf, BitConverter.ToDouble(result, 0));
            return;
        }

        switch (result[0])
        {
            case 0: // A string, whose text follows in the next record.
                builder.ExpectString(row, column, xf);
                break;

            case 1:
                builder.SetBoolean(row, column, xf, result[2] != 0);
                break;

            case 2:
                builder.SetError(row, column, xf, BiffErrors.ToCellError(result[2]));
                break;

            default: // 3 is an empty result, and anything else is treated as one.
                builder.SetBlank(row, column, xf);
                break;
        }
    }

    private void ReadFormulaString(SheetBuilder builder)
    {
        // BIFF8 writes a 16-bit length here; the older generations an 8-bit one.
        bool eightBitLength = _stream.Version != BiffVersion.Biff8
                              && _stream.RecordId == BiffRecords.String2;
        builder.SetExpectedString(_stream.ReadString(eightBitLength));
    }

    private void ReadMergedCells(SheetBuilder builder)
    {
        int count = _stream.ReadUInt16();
        for (int i = 0; i < count && _stream.RecordLeft >= 8; i++)
        {
            int firstRow = _stream.ReadUInt16();
            int lastRow = _stream.ReadUInt16();
            int firstColumn = _stream.ReadUInt16();
            int lastColumn = _stream.ReadUInt16();
            builder.AddMergedRange(firstRow, lastRow, firstColumn, lastColumn);
        }
    }

    /// <summary>
    /// The workbook's day zero for a given serial number.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The <c>1904</c> record chooses between two epochs, and the 1900 one has a defect in
    /// it: Lotus 1-2-3 treated 1900 as a leap year, Excel copied the bug for compatibility,
    /// and so serials 1 to 60 count from a day later than serials 61 and up. LibreOffice
    /// corrects for it by adding a day to anything landing before 1900-03-01
    /// (<c>XclRoot::GetDateTimeFromDouble</c>); shifting the epoch instead is the same
    /// correction expressed so that displayed text and reported value cannot disagree.
    /// </para>
    /// <para>
    /// Serial 60 is Excel's 29 February 1900, a date that never happened and that .NET
    /// cannot represent. It comes out as 1 March, sharing that day with serial 61.
    /// </para>
    /// </remarks>
    public DateTime EpochFor(double serial)
        => Uses1904Epoch ? new DateTime(1904, 1, 1)
            : serial < 61 ? new DateTime(1899, 12, 31)
            : new DateTime(1899, 12, 30);

    /// <summary>Turns a serial number into the date a spreadsheet shows for it.</summary>
    public DateTime SerialToDateTime(double serial)
    {
        DateTime epoch = EpochFor(serial);
        try
        {
            return epoch.AddDays(serial);
        }
        catch (ArgumentOutOfRangeException)
        {
            // A serial outside the calendar at all. The number is still the cell's value;
            // only reading it as a date is impossible.
            return epoch;
        }
    }

    private sealed class SheetEntry
    {
        public required string Name { get; init; }

        public required int Offset { get; init; }

        public required bool IsHidden { get; init; }

        public required SheetKind Kind { get; init; }
    }

    private enum SheetKind
    {
        Worksheet,
        Chart,
        Other,
    }

    private readonly struct XfRecord
    {
        public ushort NumberFormatIndex { get; init; }

        public bool IsCellXf { get; init; }

        public bool StatesNumberFormat { get; init; }

        public ushort ParentIndex { get; init; }
    }

    /// <summary>
    /// Accumulates one sheet's cells and turns them into a table.
    /// </summary>
    private sealed class SheetBuilder(XlsWorkbookReader owner, string sheetName)
    {
        private readonly SortedDictionary<int, SortedDictionary<int, Cell>> _rows = [];
        private readonly List<(int FirstRow, int LastRow, int FirstColumn, int LastColumn)> _merged = [];
        private int _cellCount;
        private bool _reportedLimit;
        private (int Row, int Column, int Xf)? _pendingString;

        public string SheetName => sheetName;

        public void SetDeclaredExtent(int firstRow, int lastRow, int firstColumn, int lastColumn)
        {
            // Recorded but not enforced. The extent is advisory — plenty of files understate
            // it — so it is used only to notice a disagreement, never to drop a cell that is
            // really there.
            if (lastRow <= firstRow && lastColumn <= firstColumn) return;
            DeclaredRowCount = Math.Max(0, lastRow - firstRow);
            DeclaredColumnCount = Math.Max(0, lastColumn - firstColumn);
        }

        public int DeclaredRowCount { get; private set; }

        public int DeclaredColumnCount { get; private set; }

        public void SetBlank(int row, int column, int xf) => Put(row, column, new Cell(xf));

        public void SetNumber(int row, int column, int xf, double value)
            => Put(row, column, new Cell(xf) { Number = value });

        public void SetText(int row, int column, int xf, string text)
            => Put(row, column, new Cell(xf) { Text = text });

        public void SetBoolean(int row, int column, int xf, bool value)
            => Put(row, column, new Cell(xf) { Boolean = value });

        public void SetError(int row, int column, int xf, CellError error)
            => Put(row, column, new Cell(xf) { Error = error });

        /// <summary>Notes that the next <c>STRING</c> record belongs to this cell.</summary>
        public void ExpectString(int row, int column, int xf)
        {
            Put(row, column, new Cell(xf) { Text = string.Empty });
            _pendingString = (row, column, xf);
        }

        /// <summary>Attaches a <c>STRING</c> record to the formula cell that asked for one.</summary>
        public void SetExpectedString(string text)
        {
            if (_pendingString is not { } at) return;
            Put(at.Row, at.Column, new Cell(at.Xf) { Text = text });
            _pendingString = null;
        }

        public void AddMergedRange(int firstRow, int lastRow, int firstColumn, int lastColumn)
        {
            if (lastRow < firstRow || lastColumn < firstColumn) return;
            _merged.Add((firstRow, lastRow, firstColumn, lastColumn));
        }

        private void Put(int row, int column, Cell cell)
        {
            if (row < 0 || column < 0 || column > 16383 || row > 1048575) return;

            if (_cellCount >= MaxCellsPerSheet)
            {
                if (_reportedLimit) return;
                _reportedLimit = true;
                owner._diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Warning, "PL2328",
                    $"Sheet \"{sheetName}\" holds more than {MaxCellsPerSheet} cells; the rest "
                    + "have been dropped.",
                    new DiagnosticLocation(PartName: "Workbook", Context: sheetName)));
                return;
            }

            if (!_rows.TryGetValue(row, out SortedDictionary<int, Cell>? cells))
            {
                cells = [];
                _rows[row] = cells;
            }

            if (!cells.ContainsKey(column)) _cellCount++;
            cells[column] = cell;
        }

        public ContentTable Build()
        {
            ApplyMerges(out Dictionary<(int, int), (int Rows, int Columns)> spans,
                        out HashSet<(int, int)> covered);

            ContentTable table = new()
            {
                ColumnCount = _rows.Count == 0 ? 0 : _rows.Values.Max(LastUsedColumn) + 1,
            };

            int previousRow = -1;
            foreach ((int rowIndex, SortedDictionary<int, Cell> cells) in _rows)
            {
                int gap = rowIndex - previousRow - 1;
                int from = gap > MaxConsecutiveEmptyRows ? rowIndex : previousRow + 1;

                // Empty rows inside the used range are content: they are blank lines in the
                // extracted text and they keep the row numbering honest. A gap longer than
                // the cap is a hole rather than layout, so only its end is materialised.
                for (int empty = from; empty < rowIndex; empty++)
                    table.Children.Add(new ContentTableRow { Index = empty });

                table.Children.Add(BuildRow(rowIndex, cells, spans, covered));
                previousRow = rowIndex;
            }

            return table;
        }

        private static int LastUsedColumn(SortedDictionary<int, Cell> cells)
        {
            int last = -1;
            foreach ((int column, Cell cell) in cells)
            {
                if (!cell.IsEmpty) last = column;
            }

            return last;
        }

        private ContentTableRow BuildRow(
            int rowIndex,
            SortedDictionary<int, Cell> cells,
            Dictionary<(int, int), (int Rows, int Columns)> spans,
            HashSet<(int, int)> covered)
        {
            ContentTableRow row = new() { Index = rowIndex };

            // Trailing blanks are padding: BIFF writes a MULBLANK across a whole formatted
            // row whether or not anything is in it.
            int last = LastUsedColumn(cells);

            for (int column = 0; column <= last; column++)
            {
                if (covered.Contains((rowIndex, column))) continue;

                (int rowSpan, int columnSpan) = spans.TryGetValue((rowIndex, column), out var span)
                    ? span
                    : (1, 1);

                // A column with no record at all still occupies its place in the row. BIFF
                // writes nothing for an untouched cell, so without this a row holding A and D
                // would extract as two adjacent columns and every value after the gap would
                // appear one column too far left.
                Cell cell = cells.TryGetValue(column, out Cell stated) ? stated : default;
                row.Children.Add(cell.ToContentCell(owner, rowIndex, column, rowSpan, columnSpan));
            }

            return row;
        }

        /// <summary>
        /// Turns the merged ranges into spans on their top-left cells, and a set of the cells
        /// they hide.
        /// </summary>
        /// <remarks>
        /// The hidden cells are dropped rather than emitted empty, which is what the ODS
        /// reader does with <c>covered-table-cell</c>. BIFF really does write them — a merged
        /// range across three columns arrives as a label and a two-cell MULBLANK — so without
        /// this a merged row gains phantom columns.
        /// </remarks>
        private void ApplyMerges(
            out Dictionary<(int, int), (int Rows, int Columns)> spans,
            out HashSet<(int, int)> covered)
        {
            spans = [];
            covered = [];

            foreach ((int firstRow, int lastRow, int firstColumn, int lastColumn) in _merged)
            {
                spans[(firstRow, firstColumn)] = (lastRow - firstRow + 1, lastColumn - firstColumn + 1);

                for (int row = firstRow; row <= lastRow; row++)
                {
                    for (int column = firstColumn; column <= lastColumn; column++)
                    {
                        if (row != firstRow || column != firstColumn) covered.Add((row, column));
                    }
                }
            }
        }

        /// <summary>One cell as the record stream stated it, before formatting is applied.</summary>
        private readonly struct Cell(int xf)
        {
            public int Xf { get; } = xf;

            public double? Number { get; init; }

            public string? Text { get; init; }

            public bool? Boolean { get; init; }

            public CellError? Error { get; init; }

            public bool IsEmpty => Number is null && Text is null && Boolean is null && Error is null;

            public ContentTableCell ToContentCell(
                XlsWorkbookReader owner, int row, int column, int rowSpan, int columnSpan)
            {
                ContentTableCell cell = new()
                {
                    Row = row,
                    Column = column,
                    RowSpan = rowSpan,
                    ColumnSpan = columnSpan,
                    Value = ValueOf(owner),
                };

                string text = DisplayedText(owner);
                if (text.Length > 0)
                {
                    ContentParagraph paragraph = new();
                    paragraph.Children.Add(new ContentRun { Text = text });
                    cell.Children.Add(paragraph);
                }

                return cell;
            }

            /// <summary>
            /// The cell's underlying value, typed by what the number format says it means.
            /// </summary>
            /// <remarks>
            /// A date is a number and a time is a fraction of a day; only the format says
            /// which. A time is reported as a <see cref="TimeSpan"/> rather than a
            /// time-of-day <see cref="DateTime"/> for the reason the ODS reader gives: the
            /// file does not distinguish half past two from an elapsed two and a half hours,
            /// and inventing a date would invent data.
            /// </remarks>
            private object? ValueOf(XlsWorkbookReader owner)
            {
                if (Error is { } error) return error;
                if (Boolean is { } boolean) return boolean;
                if (Text is { } text) return text;
                if (Number is not { } number) return null;

                return owner.FormatOf(Xf).ValueKind switch
                {
                    NumberFormatCode.CellValueKind.DateTime => owner.SerialToDateTime(number),
                    NumberFormatCode.CellValueKind.Duration => TimeSpan.FromDays(number),
                    _ => number,
                };
            }

            /// <summary>The text the authoring application displayed for this cell.</summary>
            private string DisplayedText(XlsWorkbookReader owner)
            {
                if (Error is { } error) return BiffErrors.Text(error);

                // Calc shows TRUE and FALSE whatever the number format says, and the format
                // Excel gives a boolean cell spells the same two words out as literals, so
                // the two agree and this needs no format lookup.
                if (Boolean is { } boolean) return boolean ? "TRUE" : "FALSE";

                if (Text is { } text) return owner.FormatOf(Xf).FormatText(text);
                if (Number is not { } number) return string.Empty;

                return owner.FormatOf(Xf).Format(number, owner.EpochFor(number));
            }
        }
    }
}

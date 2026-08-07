using Paperless.Core.Charts;
using Paperless.Core.Diagnostics;
using Paperless.Core.Extraction;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.MsBinary.Escher;
using Paperless.Spreadsheets.Layout;
using Paperless.Core.Numbers;
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

    private readonly BiffRecordReader _stream;
    private readonly List<Diagnostic> _diagnostics;
    private readonly List<string> _sharedStrings = [];
    private readonly List<SheetEntry> _sheets = [];
    private readonly List<XfRecord> _formats = [];
    private readonly XlsCellFormats _cellFormats = new();

    // The formatting runs of the shared strings that have any, by their index in the table. A
    // dictionary rather than a parallel list because almost no string is rich, and because the
    // table routinely runs to tens of thousands of entries.
    private readonly Dictionary<int, List<BiffFormattingRun>> _sharedStringRuns = [];

    // The rich cells of the sheet being read, held until its formats have been pooled: a cell's
    // portions are a delta over what its XF resolved to, and that is only known at the end.
    private readonly List<PendingRichCell> _richCells = [];
    private readonly Dictionary<int, SheetCellFormat> _resolvedFormats = [];
    private readonly Dictionary<int, int> _rowFormats = [];
    private readonly Dictionary<int, int> _columnFormats = [];
    private readonly Dictionary<int, string> _formatCodes = [];
    private readonly Dictionary<int, NumberFormatCode> _parsedFormats = [];
    private readonly List<SheetLayout> _layouts = [];
    private readonly Dictionary<int, List<SheetRange>> _printAreas = [];
    private readonly Dictionary<int, SheetRange> _repeatColumns = [];
    private readonly Dictionary<int, SheetRange> _repeatRows = [];
    private readonly XlsDecorationTable _decoration = new();
    private XlsSheetDecoration _sheetDecoration = new();
    private XlsSheetPrintState _page = new();
    private XlsDrawingCollector _drawings = new([]);

    /// <summary>The sheet's <c>NOTE</c> records, joined to their objects once it is read.</summary>
    private readonly List<(int Column, int Row, ushort Object)> _notes = [];
    private readonly List<byte> _drawingGroup = [];
    private Dictionary<int, EscherBlip>? _blips;
    private bool _reportedFormat;
    private bool _reportedSstIndex;

    public XlsWorkbookReader(byte[] workbook, List<Diagnostic> diagnostics)
    {
        _diagnostics = diagnostics;
        _stream = new BiffRecordReader(workbook, diagnostics);
    }

    /// <summary>How many sheets the workbook declares, hidden ones included.</summary>
    public int SheetCount => _sheets.Count;

    /// <summary>The document's own file name, for the <c>&amp;F</c> header field.</summary>
    public string FileName { get; set; } = string.Empty;

    /// <summary>True when the workbook counts days from 1904 rather than from 1900.</summary>
    public bool Uses1904Epoch { get; private set; }

    /// <summary>Which epoch the workbook's date serials count from.</summary>
    /// <remarks>
    /// Stated as the shared <see cref="SpreadsheetDateSystem"/> rather than as a bare
    /// <see cref="DateTime"/> so that the XLS and XLSX paths hand the formatter the same
    /// thing, and so the phantom 29 February 1900 is corrected in one place for both.
    /// </remarks>
    public SpreadsheetDateSystem DateSystem
        => Uses1904Epoch ? SpreadsheetDateSystem.Date1904 : SpreadsheetDateSystem.Date1900;

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
            // A chart sheet is a sheet: it has its own page setup, it prints, and losing it
            // costs a page of the workbook. Macro and Visual Basic substreams carry nothing a
            // reader wants and keep their place in the numbering without producing a section,
            // exactly as LibreOffice's SkipSubStream does.
            if (sheet.Kind == SheetKind.Chart) sections.AddRange(ReadChartSheet(sheet, index));
            else if (sheet.Kind == SheetKind.Worksheet) sections.Add(ReadSheet(sheet, index));

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

                case BiffRecords.Font or BiffRecords.Font34:
                    ReadFont();
                    break;

                case BiffRecords.Palette:
                    ReadPalette();
                    break;

                // The workbook's picture store. It is stated once, in the globals, and every
                // sheet's shapes index into it — so it has to be collected before any sheet is
                // read, which is what makes it a globals record rather than a sheet one.
                case BiffRecords.MsoDrawingGroup:
                    _drawingGroup.AddRange(_stream.ReadBytes(_stream.RecordLeft));
                    break;

                case BiffPageRecords.Name:
                    ReadName();
                    break;

                default:
                    break;
            }
        }
    }

    /// <summary>
    /// The workbook's picture store, read once from the globals and shared by every sheet.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Excel keeps the <c>OfficeArtDggContainer</c> inline, in <c>MSODRAWINGGROUP</c> records at the
    /// head of the workbook, where Word puts it in the table stream and PowerPoint in a
    /// <c>PPDrawingGroup</c>. The bytes are the same structure in all three, so the store is read by
    /// <see cref="EscherBlips"/> rather than here — with no delay stream, because a workbook's
    /// blips are inline in the <c>msofbtBSE</c> and there is no second stream for a <c>foDelay</c>
    /// to point into (<c>XclImpObjectManager::ReadMsoDrawingGroup</c>,
    /// <c>sc/source/filter/excel/xiescher.cxx</c>, hands <c>maDffStrm</c> alone to
    /// <c>SvxMSDffManager</c>).
    /// </para>
    /// <para>
    /// Read lazily and kept, because most workbooks have no drawing group at all: eight of the
    /// corpus's sixty-one <c>.xls</c> carry one, and the other fifty-three would pay a walk of an
    /// empty buffer per sheet.
    /// </para>
    /// </remarks>
    private Dictionary<int, EscherBlip> Blips =>
        _blips ??= _drawingGroup.Count > 0 ? EscherBlips.Read([.. _drawingGroup], [], []) : [];

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
        {
            string text = _stream.ReadString(eightBitLength: false, out List<BiffFormattingRun>? runs);
            if (runs is not null) _sharedStringRuns[_sharedStrings.Count] = runs;
            _sharedStrings.Add(text);
        }

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
            ushort fontIndex = _stream.ReadUInt16();
            ushort numberFormat = _stream.ReadUInt16();
            ushort typeProtection = _stream.ReadUInt16();
            ushort alignment = _stream.ReadUInt16();

            // Where the "attribute used" bits live moved between BIFF5 and BIFF8: BIFF5 keeps
            // them in the alignment field, BIFF8 in a misc field that BIFF5 does not have.
            bool biff8 = _stream.Version == BiffVersion.Biff8;
            ushort used = biff8 ? _stream.ReadUInt16() : alignment;

            xf = MakeXf(numberFormat, typeProtection, used) with
            {
                FontIndex = fontIndex,
                Alignment = biff8
                    ? XlsCellFormats.Align8(alignment, used)
                    : XlsCellFormats.Align5(alignment),
            };

            // The borders and the fill come after the alignment in the same record, so they are
            // read here rather than in a second pass: the stream is already positioned on them.
            ReadXfDecoration(xf.IsCellXf, used);
        }
        else
        {
            // BIFF3/BIFF4 pack font and format into single bytes, and disagree with each
            // other about which field holds the parent index. Only the number format matters
            // here, and that is in the same place in both.
            byte fontIndex = _stream.ReadByte();
            ushort numberFormat = _stream.ReadByte();
            ushort typeProtection = _stream.ReadUInt16();
            ushort alignment = _stream.ReadUInt16();
            xf = MakeXf(numberFormat, typeProtection, alignment) with
            {
                FontIndex = fontIndex,
                Alignment = XlsCellFormats.Align5(alignment),
            };
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
    /// Reads the borders and the fill that follow an <c>XF</c>'s number format.
    /// </summary>
    /// <remarks>
    /// <para>
    /// BIFF5 and BIFF8 lay the same information out differently and neither is guessable: BIFF8
    /// writes two border dwords then a two-byte area word, and hides the <em>fill pattern</em>
    /// in the second border dword rather than in the area word; BIFF5 writes the area dword
    /// first and puts one of the four edges — the bottom — inside it. Both are transcribed from
    /// <c>XclImpXF::ReadXF5</c> and <c>ReadXF8</c>
    /// (<c>sc/source/filter/excel/xistyle.cxx:1201</c> and <c>:1225</c>).
    /// </para>
    /// <para>
    /// The "states it" flags follow the same inversion the number format does: in a cell XF a
    /// set bit means the XF states the attribute, and in a style XF a cleared bit does
    /// (<c>XclImpXF::SetUsedFlags</c>, <c>xistyle.cxx:1466</c>). Border is bit 3 of the
    /// six-bit field and area bit 4.
    /// </para>
    /// </remarks>
    private void ReadXfDecoration(bool isCellXf, ushort used)
    {
        const int borderBit = 0x08;
        const int areaBit = 0x10;

        int flags = (used >> 10) & 0x3F;
        bool border = isCellXf == ((flags & borderBit) != 0);
        bool area = isCellXf == ((flags & areaBit) != 0);

        if (_stream.Version == BiffVersion.Biff8)
        {
            if (_stream.RecordLeft < 10) return;

            uint border1 = _stream.ReadUInt32();
            uint border2 = _stream.ReadUInt32();
            ushort fill = _stream.ReadUInt16();
            _decoration.Add(XlsXfDecoration.FromBiff8(border1, border2, fill, border, area));
        }
        else
        {
            if (_stream.RecordLeft < 8) return;

            uint fill = _stream.ReadUInt32();
            uint lines = _stream.ReadUInt32();
            _decoration.Add(XlsXfDecoration.FromBiff5(fill, lines, border, area));
        }
    }

    /// <summary>
    /// Reads a <c>FONT</c> record.
    /// </summary>
    /// <remarks>
    /// The layout is the same from BIFF5 on bar the name's encoding: height, flags, colour index,
    /// weight, escapement, underline, family, character set, one reserved byte, then a
    /// length-prefixed name (<c>XclImpFont::ReadFontData5</c>,
    /// <c>sc/source/filter/excel/xistyle.cxx:439</c>). BIFF2–4 have no colour or weight field and
    /// carry bold as a flag, which is why the short form is read separately.
    /// </remarks>
    private void ReadFont()
    {
        if (_stream.RecordLeft < 4) return;

        int height = _stream.ReadUInt16();
        ushort flags = _stream.ReadUInt16();
        bool italic = (flags & 0x0002) != 0;

        // fStrikeOut is in the flags beside fItalic and is there in every BIFF; the underline
        // is a byte of its own and only from BIFF5 on (XclFontData::FillFromMsoFont,
        // sc/source/filter/excel/xlstyle.cxx). BIFF2's fUnderline lives in the flags instead,
        // which is the one case the short form below still misses.
        bool strike = (flags & 0x0008) != 0;

        int weight = (flags & 0x0001) != 0 ? BoldWeight : NormalWeight;
        int colour = AutomaticColourIndex;
        SheetUnderline underline = SheetUnderline.None;

        if (_stream.RecordLeft >= 10)
        {
            colour = _stream.ReadUInt16();
            weight = _stream.ReadUInt16();
            _stream.Skip(2);                    // escapement
            underline = Underline(_stream.ReadByte());
            _stream.Skip(3);                    // family, character set, reserved
        }

        string name = _stream.RecordLeft > 0
            ? _stream.Version == BiffVersion.Biff8
                ? _stream.ReadString(eightBitLength: true)
                : _stream.ReadByteString(_stream.ReadByte())
            : string.Empty;

        _cellFormats.AddFont(new BiffFont(
            name,
            Length.FromTwips(height),
            weight is >= 100 and <= 1000 ? weight : NormalWeight,
            italic,
            colour,
            underline,
            strike));
    }

    /// <summary>
    /// The <c>FONT</c> record's underline byte.
    /// </summary>
    /// <remarks>
    /// <c>EXC_FONTUNDERL_*</c> (<c>sc/source/filter/inc/xlstyle.hxx</c>): 0 none, 1 single,
    /// 2 double, 0x21 single accounting, 0x22 double accounting. The two accounting forms differ
    /// only in how wide Calc draws the line, which is not reproduced — see
    /// <see cref="SheetUnderline"/> — so each folds onto its plain counterpart.
    /// </remarks>
    private static SheetUnderline Underline(int stated) => stated switch
    {
        0x01 or 0x21 => SheetUnderline.SingleLine,
        0x02 or 0x22 => SheetUnderline.DoubleLine,
        _ => SheetUnderline.None,
    };

    /// <summary>BIFF's own weights, which are the only two any writer emits.</summary>
    private const int NormalWeight = 400;

    /// <inheritdoc cref="NormalWeight"/>
    private const int BoldWeight = 700;

    /// <summary>The colour index meaning "the window's text colour", which prints black.</summary>
    private const int AutomaticColourIndex = 0x7FFF;

    /// <summary>
    /// Reads a <c>PALETTE</c> record: the colours the workbook redefines, from index eight up.
    /// </summary>
    /// <remarks>
    /// Each entry is four bytes of red, green, blue and a padding byte — not the BGR order the
    /// rest of the format uses for colours packed into bit fields.
    /// </remarks>
    private void ReadPalette()
    {
        if (_stream.RecordLeft < 2) return;

        int count = _stream.ReadUInt16();
        List<Colour> colours = new(count);

        for (int at = 0; at < count && _stream.RecordLeft >= 4; at++)
        {
            byte red = _stream.ReadByte();
            byte green = _stream.ReadByte();
            byte blue = _stream.ReadByte();
            _stream.ReadByte();
            colours.Add(new Colour(red, green, blue));
        }

        // One record, two consumers: a font's colour index and a fill's resolve against the
        // same table, so both are handed it rather than either re-reading the record.
        _decoration.SetPalette(colours.Count, colours);
        _cellFormats.SetPalette(colours);
    }

    /// <summary>
    /// One sheet's per-cell text formats, from the XF index every cell record carries.
    /// </summary>
    /// <remarks>
    /// BIFF is the one format where this costs nothing extra: every cell record already states
    /// its <c>ixfe</c>, and <c>ROW</c> and <c>COLINFO</c> already state the defaults, so the map
    /// falls out of the same pass that read the cells rather than needing a second walk.
    /// </remarks>
    /// <summary>A cell whose <c>SST</c> string carried formatting runs, kept until the sheet ends.</summary>
    private readonly record struct PendingRichCell(
        int Row, int Column, int Xf, string Text, List<BiffFormattingRun> Runs);

    /// <summary>
    /// Turns the sheet's formatting runs into portions of its cells' text.
    /// </summary>
    /// <remarks>
    /// A run states a start and nothing else, so a portion reaches to the next run's start and the
    /// last to the end of the string — and the characters before the first run keep the cell's own
    /// font, which is why the runs are not simply zipped into a list. LibreOffice pairs them the
    /// same way (<c>XclImpString::GetFormats</c>, <c>sc/source/filter/excel/xlstring.cxx</c>).
    /// </remarks>
    private SheetRichText BuildRichText()
    {
        if (_richCells.Count == 0) return SheetRichText.Empty;

        SheetRichText.Builder rich = new();

        foreach (PendingRichCell cell in _richCells)
        {
            SheetCellFormat format = CellFormatOf(cell.Xf);
            List<SheetTextPortion> portions = [];

            for (int at = 0; at < cell.Runs.Count; at++)
            {
                int start = cell.Runs[at].Start;
                int end = at + 1 < cell.Runs.Count ? cell.Runs[at + 1].Start : cell.Text.Length;
                if (end <= start) continue;

                portions.Add(new SheetTextPortion(
                    start, end - start, _cellFormats.ApplyFont(format, cell.Runs[at].FontIndex)));
            }

            rich.Set(cell.Row, cell.Column, cell.Text, format, portions);
        }

        return rich.Build();
    }

    private SheetCellFormats BuildFormats(SheetBuilder builder)
    {
        SheetCellFormats.Builder formats = new();
        Dictionary<int, int> pooled = [];

        foreach ((int row, int column, int xf) in builder.CellFormats())
        {
            formats.SetCell(row, column, Pool(xf));
        }

        foreach ((int row, int xf) in _rowFormats) formats.SetRow(row, Pool(xf));
        foreach ((int column, int xf) in _columnFormats) formats.SetColumn(column, Pool(xf));

        return formats.Build();

        int Pool(int xf)
        {
            if (pooled.TryGetValue(xf, out int index)) return index;

            index = formats.Intern(CellFormatOf(xf));
            pooled[xf] = index;
            return index;
        }
    }

    /// <summary>The text format an XF index resolves to, resolved once per index.</summary>
    private SheetCellFormat CellFormatOf(int xfIndex)
    {
        if (_resolvedFormats.TryGetValue(xfIndex, out SheetCellFormat? cached)) return cached;

        XfRecord xf = xfIndex >= 0 && xfIndex < _formats.Count ? _formats[xfIndex] : default;
        SheetCellFormat format = _cellFormats.Resolve(xf.FontIndex, xf.Alignment, FormatOf(xfIndex));

        _resolvedFormats[xfIndex] = format;
        return format;
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

        if (!parsed.IsFullyReproduced && !_reportedFormat)
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

    /// <summary>
    /// Reads a chart sheet: a sheet whose whole content is one chart.
    /// </summary>
    /// <remarks>
    /// <para>
    /// It is a substream of its own, headed by a <c>BOF</c> of type <c>0x0020</c>, and it holds
    /// the page setup a worksheet's does — margins, <c>SETUP</c>, header and footer — plus the
    /// chart records and any drawing objects laid over the chart.
    /// </para>
    /// <para>
    /// <strong>The chart's printed rectangle is computed from the paper, not read.</strong>
    /// <c>XclImpChartObj::FinalizeTabChart</c> (<c>sc/source/filter/excel/xiescher.cxx</c>)
    /// derives it: the paper less the margins, less two centimetres of width and one of height
    /// "to give some more extra space", less another two and one when the sheet prints its row
    /// and column headings, offset a centimetre from the left of the sheet and half a centimetre
    /// from the top. The <c>CHCHART</c> record does state a rectangle, and it is the one Excel
    /// showed the chart at on screen rather than the one it prints at — using it puts the chart
    /// off the paper.
    /// </para>
    /// <para>
    /// The chart lands as an absolutely anchored drawing rather than as cells, which is what
    /// makes the rest of the pipeline work unchanged: <c>SheetDrawingArea</c> widens the printed
    /// range to cover it, <c>SheetEmptyPages</c> keeps the page it overlaps, and
    /// <c>SheetPageGraphics</c> paints it through the same <c>SheetChart</c> a worksheet's chart
    /// goes through.
    /// </para>
    /// </remarks>
    private List<ContentSection> ReadChartSheet(SheetEntry sheet, int index)
    {
        _page = new XlsSheetPrintState
        {
            DefaultFont = _cellFormats.DefaultFont,

            // BIFF8 keeps every height its file states. `ImportExcel::Read` calls
            // `AdjustRowHeight()` (sc/source/filter/excel/read.cxx:780) and
            // `ImportExcel8::Read` has the same call `#if 0`-ed out, with the reason beside
            // it: "Excel documents look much better without this call; better in the sense
            // that the row heights are identical to the original heights in Excel"
            // (read.cxx:1282-1288). So a `.xls` written this century is never re-measured.
            RowHeightsAreManual = _stream.Version == BiffVersion.Biff8,
        };
        _sheetDecoration = new XlsSheetDecoration();
        _drawings = new XlsDrawingCollector(_diagnostics, Blips);
        XlsChartBuilder chart = new();

        if (!StartSubstream(sheet))
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2327",
                $"No chart substream could be found for \"{sheet.Name}\"; the sheet is empty.",
                new DiagnosticLocation(PartName: "Workbook", Context: sheet.Name)));
            return [];
        }

        ReadChartRecords(chart);

        ChartPlot? plot = chart.Build();
        SheetPrintSetup setup = _page.ToSetup();
        DocRect frame = ChartSheetFrame(setup);

        List<SheetDrawing> drawings =
        [
            new SheetDrawing
            {
                Anchor = SheetAnchorKind.Absolute,
                Position = new DocPoint(frame.X, frame.Y),
                Extent = new DocSize(frame.Width, frame.Height),
                Name = sheet.Name,
                IsChart = true,
                Chart = plot,
            },
            .. _drawings.BuildForChart(
                new DocPoint(frame.X, frame.Y), new DocSize(frame.Width, frame.Height)),
        ];

        _layouts.Add(new SheetLayout
        {
            Name = sheet.Name,
            Index = index,
            IsHidden = sheet.IsHidden,
            Setup = setup,
            Grid = _page.ToGrid(),
            Cells = new ContentTable(),
            Drawings = new SheetDrawings(drawings),
            FileName = FileName,
        });

        ContentSection section = new()
        {
            Kind = SectionKind.Sheet,
            Index = index,
            Name = sheet.Name,
            IsHidden = sheet.IsHidden,
        };
        section.Children.Add(new ContentTable());

        List<ContentSection> sections = [section];

        // The chart follows its sheet as a sibling rather than sitting inside it, which is where
        // `XlsxCharts` and `OdfChart` already put one — see the module's TODO.
        if (ChartSection(plot, index) is { } frameSection) sections.Add(frameSection);

        return sections;
    }

    /// <summary>The chart's titles as a frame section, or null when it names none.</summary>
    private static ContentSection? ChartSection(ChartPlot? plot, int index)
    {
        if (plot is null) return null;

        string?[] lines = [plot.Title, plot.CategoryAxisTitle, plot.ValueAxisTitle];
        if (Array.TrueForAll(lines, line => line is not { Length: > 0 })) return null;

        ContentSection section = new() { Kind = SectionKind.Frame, Index = index, Name = plot.Title };
        foreach (string? line in lines)
        {
            if (line is not { Length: > 0 }) continue;

            ContentParagraph paragraph = new();
            paragraph.Children.Add(new ContentRun { Text = line });
            section.Children.Add(paragraph);
        }

        return section;
    }

    /// <summary>
    /// Where a chart sheet's chart is drawn on the sheet, in the sheet's own coordinates.
    /// </summary>
    /// <remarks>
    /// <c>XclImpChartObj::FinalizeTabChart</c>, in hundredths of a millimetre throughout. The
    /// two extra subtractions and the offsets are the C++'s own constants and are reproduced as
    /// written; three of them are unexplained there too.
    /// </remarks>
    private static DocRect ChartSheetFrame(SheetPrintSetup setup)
    {
        Length left = Length.FromMm100(ChartSheetOffsetX);
        Length top = Length.FromMm100(ChartSheetOffsetY);

        Length right = setup.PageSize.Width - setup.LeftMargin - setup.RightMargin
                       - Length.FromMm100(ChartSheetSlackX);
        Length bottom = setup.PageSize.Height - setup.TopMargin - setup.BottomMargin
                        - Length.FromMm100(ChartSheetSlackY);

        if (setup.PrintsHeadings)
        {
            right -= Length.FromMm100(ChartSheetSlackX);
            bottom -= Length.FromMm100(ChartSheetSlackY);
        }

        return new DocRect(
            left, top,
            Length.Max(right - left, Length.FromMm100(1000)),
            Length.Max(bottom - top, Length.FromMm100(1000)));
    }

    private const int ChartSheetOffsetX = 1000;
    private const int ChartSheetOffsetY = 500;
    private const int ChartSheetSlackX = 2000;
    private const int ChartSheetSlackY = 1000;

    /// <summary>
    /// Walks a chart substream, routing its records to the page setup, the drawing layer or the
    /// chart.
    /// </summary>
    private void ReadChartRecords(XlsChartBuilder chart)
    {
        int depth = 0;

        while (_stream.MoveNext())
        {
            ushort id = _stream.RecordId;

            if (BiffRecords.IsBof(id)) { depth++; continue; }
            if (id == BiffRecords.Eof)
            {
                if (depth == 0) return;
                depth--;
                continue;
            }

            if (depth > 0) continue;

            switch (id)
            {
                case BiffRecords.MsoDrawing or BiffRecords.MsoDrawingSelection:
                    _drawings.AddDrawing(_stream.ReadBytes(_stream.RecordLeft));
                    break;

                case BiffRecords.Obj:
                    _drawings.ReadObject(_stream);
                    break;

                case BiffRecords.Txo:
                    _drawings.ReadText(_stream);
                    break;

                default:
                    if (BiffChartRecords.IsChartRecord(id)) chart.Read(id, _stream);
                    else ReadPageRecord(id);
                    break;
            }
        }
    }

    /// <summary>Reads one sheet substream into a section.</summary>
    private ContentSection ReadSheet(SheetEntry sheet, int index)
    {
        SheetBuilder builder = new(this, sheet.Name);
        _page = new XlsSheetPrintState
        {
            DefaultFont = _cellFormats.DefaultFont,

            // BIFF8 keeps every height its file states. `ImportExcel::Read` calls
            // `AdjustRowHeight()` (sc/source/filter/excel/read.cxx:780) and
            // `ImportExcel8::Read` has the same call `#if 0`-ed out, with the reason beside
            // it: "Excel documents look much better without this call; better in the sense
            // that the row heights are identical to the original heights in Excel"
            // (read.cxx:1282-1288). So a `.xls` written this century is never re-measured.
            RowHeightsAreManual = _stream.Version == BiffVersion.Biff8,
        };
        _sheetDecoration = new XlsSheetDecoration();
        _drawings = new XlsDrawingCollector(_diagnostics, Blips);
        _rowFormats.Clear();
        _columnFormats.Clear();
        _richCells.Clear();
        _notes.Clear();

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

        ContentTable table = builder.Build();
        section.Children.Add(table);

        // The print names are workbook-level and scoped by the sheet's position in the
        // directory, so they are attached here rather than while the sheet's records go by.
        if (_printAreas.TryGetValue(index, out List<SheetRange>? areas))
            _page.PrintAreas.AddRange(areas);
        if (_repeatColumns.TryGetValue(index, out SheetRange columns))
            _page.RepeatColumns = columns;
        if (_repeatRows.TryGetValue(index, out SheetRange rows))
            _page.RepeatRows = rows;

        // After ToGrid, and it has to be: a client anchor states its offsets as fractions of the
        // column and row it names, so a drawing cannot be placed until their sizes are known.
        SheetGrid grid = _page.ToGrid();

        _layouts.Add(new SheetLayout
        {
            Name = sheet.Name,
            Index = index,
            IsHidden = sheet.IsHidden,
            Setup = _page.ToSetup(),
            Grid = grid,
            Cells = table,
            StatedMerges = builder.StatedMerges,
            HyperlinkRanges = builder.HyperlinkRanges,
            Formatting = _sheetDecoration.Resolve(_decoration),
            Formats = BuildFormats(builder),
            RichText = BuildRichText(),
            Drawings = _drawings.IsEmpty
                ? SheetDrawings.Empty
                : new SheetDrawings(_drawings.BuildForSheet(grid)),
            Notes = BuildNotes(),
            FileName = FileName,
        });

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

                case BiffRecords.FormulaString or BiffRecords.FormulaString2:
                    ReadFormulaString(builder);
                    break;

                case BiffRecords.HLink:
                    ReadHyperlink(builder);
                    break;

                case BiffRecords.MergedCells:
                    ReadMergedCells(builder);
                    break;

                // The drawing layer, which is three record kinds and one assembly step; see
                // XlsDrawingCollector. Kept in the sheet loop rather than skipped, because a
                // text box is the only content on a sheet that no walk of the cells can find.
                // The drawing layer, which is three record kinds and one assembly step; see
                // XlsDrawingCollector. Kept in the sheet loop rather than skipped, because a
                // text box is the only content on a sheet that no walk of the cells can find.
                case BiffRecords.MsoDrawing or BiffRecords.MsoDrawingSelection:
                    _drawings.AddDrawing(_stream.ReadBytes(_stream.RecordLeft));
                    break;

                case BiffRecords.Obj:
                    _drawings.ReadObject(_stream);
                    break;

                case BiffRecords.Txo:
                    _drawings.ReadText(_stream);
                    break;

                case BiffRecords.Note:
                    ReadNote();
                    break;

                case BiffRecords.Dimensions or BiffRecords.Dimensions2:
                    ReadDimensions(builder);
                    break;

                default:
                    ReadPageRecord(id);
                    break;
            }
        }
    }

    /// <summary>Joins the sheet's <c>NOTE</c> records to the comment objects they name.</summary>
    private SheetNotes BuildNotes()
    {
        if (_notes.Count == 0) return SheetNotes.Empty;

        Dictionary<ushort, string> texts = _drawings.NoteTexts();
        if (texts.Count == 0) return SheetNotes.Empty;

        List<SheetNote> notes = [];
        foreach ((int column, int row, ushort identifier) in _notes)
        {
            if (texts.TryGetValue(identifier, out string? text))
                notes.Add(new SheetNote(column, row, text));
        }

        return notes.Count == 0 ? SheetNotes.Empty : new SheetNotes { Items = notes };
    }

    /// <summary>
    /// Reads one <c>NOTE</c> record: which cell a comment hangs off and which object holds it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// BIFF8 only. The pre-BIFF8 layout put the comment's characters in the record itself and
    /// continued them in further NOTEs with a row of <c>0xFFFF</c>; from BIFF8 the record names an
    /// object instead and the text is that object's <c>TXO</c>
    /// (<c>XclImpNote</c>, <c>sc/source/filter/excel/xicontent.cxx</c>). Only the newer form is
    /// read, because it is the only one that appears in the corpus and because the older one is a
    /// separate continuation scheme rather than a shorter record.
    /// </para>
    /// <para>
    /// The join is deferred: a NOTE may arrive before its OBJ, so the identifier is kept and
    /// resolved against <see cref="XlsDrawingCollector.NoteTexts"/> once the sheet has been read.
    /// </para>
    /// </remarks>
    private void ReadNote()
    {
        if (_stream.Version != BiffVersion.Biff8) return;
        if (_stream.RecordLeft < 8) return;

        int row = _stream.ReadUInt16();
        int column = _stream.ReadUInt16();
        _stream.Skip(2);
        ushort identifier = _stream.ReadUInt16();

        _notes.Add((column, row, identifier));
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
        builder.SetNumber(row, column, xf, BiffRecordReader.RkValue(_stream.ReadInt32()));
    }

    private void ReadMulRk(SheetBuilder builder)
    {
        int row = _stream.ReadUInt16();
        int column = _stream.ReadUInt16();

        while (_stream.RecordLeft > 2 && _stream.IsValid)
        {
            int xf = _stream.ReadUInt16();
            builder.SetNumber(row, column, xf, BiffRecordReader.RkValue(_stream.ReadInt32()));
            column++;
        }
    }

    private void ReadLabel(SheetBuilder builder, ushort id)
    {
        (int row, int column, int xf) = ReadCellHeader();

        // 0x0004 is the Excel 2.1 spelling, whose string length is one byte where every
        // later generation uses two. Its cell header is three bytes of attributes rather
        // than a two-byte XF index as well, which this does not correct for: BIFF2 is read
        // best-effort, and the string is what matters.
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
            string text = _sharedStrings[index];
            builder.SetText(row, column, xf, text);

            if (_sharedStringRuns.TryGetValue(index, out List<BiffFormattingRun>? runs))
                _richCells.Add(new PendingRichCell(row, column, xf, text, runs));

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
                              && _stream.RecordId == BiffRecords.FormulaString2;
        builder.SetExpectedString(_stream.ReadString(eightBitLength));
    }

    /// <summary>
    /// Records which cells an <c>HLINK</c> covers, without decoding the link itself.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The record opens with a <c>XclRange</c> — first and last row, then first and last column —
    /// followed by the embedded <c>StdLink</c> structure (<c>XclImpHyperlink::ReadHlink</c>,
    /// <c>sc/source/filter/excel/xicontent.cxx:221-231</c>). Excel writes rubbish in the high
    /// byte of the column indices and the importer masks it off, which is reproduced here.
    /// </para>
    /// <para>
    /// Only whether a URL results matters to layout, not what it is: a link that resolves to an
    /// empty string is dropped before it reaches a cell (<c>ReadHlink</c>'s
    /// <c>if (!aString.isEmpty())</c>), and the string is non-empty exactly when the flags name a
    /// UNC path, a file or URL moniker, or a text mark
    /// (<c>ReadEmbeddedData</c>, <c>xicontent.cxx:233-330</c>). Testing the flags rather than
    /// walking the monikers keeps this to a dozen lines and cannot disagree with itself about a
    /// path it never has to resolve.
    /// </para>
    /// </remarks>
    private void ReadHyperlink(SheetBuilder builder)
    {
        // BIFF8 only; earlier generations have no HLINK record at all.
        if (_stream.RecordLeft < 8 + 16 + 8) return;

        int firstRow = _stream.ReadUInt16();
        int lastRow = _stream.ReadUInt16();
        int firstColumn = _stream.ReadUInt16() & 0xFF;
        int lastColumn = _stream.ReadUInt16() & 0xFF;

        _stream.Skip(16);                     // the StdLink GUID
        _stream.Skip(4);                      // the stream version
        uint flags = _stream.ReadUInt32();

        const uint Body = 0x00000001;         // EXC_HLINK_BODY
        const uint Mark = 0x00000008;         // EXC_HLINK_MARK
        const uint Unc = 0x00000100;          // EXC_HLINK_UNC

        if ((flags & (Body | Mark | Unc)) == 0) return;

        builder.AddHyperlinkRange(firstRow, lastRow, firstColumn, lastColumn);
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


    /// <summary>The sheets' print setups and geometry, in directory order.</summary>
    /// <remarks>
    /// Chart and macro substreams contribute nothing here, exactly as they contribute no
    /// section: they keep their place in the sheet numbering and have no page setup of their
    /// own.
    /// </remarks>
    public IReadOnlyList<SheetLayout> Layouts => _layouts;

    /// <summary>
    /// Reads one of the records that carry a sheet's page geometry.
    /// </summary>
    /// <remarks>
    /// Reached from the sheet loop's default branch, so a workbook with none of them costs one
    /// switch per record. Every one of these is optional and several are written only when they
    /// differ from Excel's default, which is why <see cref="XlsSheetPrintState"/> starts from
    /// the defaults rather than from zero.
    /// </remarks>
    private void ReadPageRecord(ushort id)
    {
        switch (id)
        {
            case BiffPageRecords.LeftMargin or BiffPageRecords.RightMargin
                or BiffPageRecords.TopMargin or BiffPageRecords.BottomMargin:
                if (_stream.RecordLeft >= 8) _page.SetMargin(id, _stream.ReadDouble());
                break;

            case BiffPageRecords.Setup:
                ReadSetup();
                break;

            case BiffPageRecords.WsBool:
                if (_stream.RecordLeft >= 2)
                {
                    _page.SetFitsToPages(
                        (_stream.ReadUInt16() & BiffPageRecords.WsBoolFitToPage) != 0);
                }
                break;

            case BiffPageRecords.Header or BiffPageRecords.Footer:
                // An empty record is a header that was switched off, and it is the emptiness
                // rather than the record's absence that Calc reads as "no header". The length
                // field is one byte in BIFF5 and two in BIFF8, which is not a property of the
                // format but of this record — LibreOffice picks between ReadByteString(false)
                // and ReadUniString() on the generation (xipage.cxx:114).
                _page.SetFurniture(
                    id,
                    _stream.RecordLeft > 0
                        ? _stream.ReadString(_stream.Version == BiffVersion.Biff5)
                        : string.Empty);
                break;

            case BiffPageRecords.HorizontalCentre or BiffPageRecords.VerticalCentre
                or BiffPageRecords.PrintHeaders or BiffPageRecords.PrintGridLines:
                if (_stream.RecordLeft >= 2) _page.SetFlag(id, _stream.ReadUInt16() != 0);
                break;

            case BiffPageRecords.HorizontalPageBreaks or BiffPageRecords.VerticalPageBreaks:
                ReadPageBreaks(id);
                break;

            case BiffPageRecords.DefColWidth:
                if (_stream.RecordLeft >= 2) _page.SetDefaultColumnWidth(_stream.ReadUInt16());
                break;

            case BiffPageRecords.DefaultRowHeight:
                if (_stream.RecordLeft >= 4)
                {
                    // Bit 0 is the sheet's own fUnsynced — EXC_DEFROW_UNSYNCED,
                    // sc/source/filter/inc/xltable.hxx:114 — and it is not about the default
                    // height alone. `XclImpColRowSettings::Convert` answers it by marking every
                    // row of the sheet manual before it looks at a single ROW record
                    // (sc/source/filter/excel/colrowst.cxx:212-215), so a sheet that sets it has
                    // no row Calc will re-measure, whatever the ROW records say.
                    bool manual = (_stream.ReadUInt16() & 0x0001) != 0;
                    _page.SetDefaultRowHeight(_stream.ReadUInt16(), manual);
                }
                break;

            case BiffPageRecords.ColInfo:
                ReadColInfo();
                break;

            case BiffPageRecords.Row:
                ReadRow();
                break;

            default:
                break;
        }
    }

    private void ReadSetup()
    {
        if (_stream.RecordLeft < 12) return;

        int paper = _stream.ReadUInt16();
        int scale = _stream.ReadUInt16();
        int startPage = _stream.ReadUInt16();
        int fitWidth = _stream.ReadUInt16();
        int fitHeight = _stream.ReadUInt16();
        ushort flags = _stream.ReadUInt16();

        // BIFF5 added the resolutions and the header and footer margins; a BIFF4 SETUP stops
        // after the flags, so the rest is read only when the record is long enough to hold it.
        double? headerMargin = null;
        double? footerMargin = null;
        if (_stream.RecordLeft >= 20)
        {
            _stream.ReadUInt16();
            _stream.ReadUInt16();
            headerMargin = _stream.ReadDouble();
            footerMargin = _stream.ReadDouble();
        }

        _page.SetSetup(paper, scale, startPage, fitWidth, fitHeight, flags, headerMargin, footerMargin);
    }

    /// <summary>
    /// Reads a <c>HORIZONTALPAGEBREAKS</c> or <c>VERTICALPAGEBREAKS</c> record.
    /// </summary>
    /// <remarks>
    /// BIFF8 widened each entry from one position to a position plus the first and last
    /// column or row the break applies to, and LibreOffice ignores the extra pair
    /// (<c>XclImpPageSettings::ReadPageBreaks</c>, <c>sc/source/filter/excel/xipage.cxx:151</c>)
    /// — a break in Calc runs across the whole sheet, so a partial one cannot be represented.
    /// </remarks>
    private void ReadPageBreaks(ushort id)
    {
        if (_stream.RecordLeft < 2) return;

        int count = _stream.ReadUInt16();
        bool wide = _stream.Version == BiffVersion.Biff8;
        List<int> positions = [];

        for (int at = 0; at < count && _stream.RecordLeft >= 2; at++)
        {
            positions.Add(_stream.ReadUInt16());
            if (wide && _stream.RecordLeft >= 4) _stream.Skip(4);
        }

        _page.AddBreaks(id, positions);
    }

    private void ReadColInfo()
    {
        if (_stream.RecordLeft < 10) return;

        int first = _stream.ReadUInt16();
        int last = _stream.ReadUInt16();
        int width = _stream.ReadUInt16();

        // The field after the width is the run's XF index, which is what formats the cells no
        // record was written for — a column filled yellow, or set in a font, in a sheet with
        // nothing in it.
        int xf = _stream.ReadUInt16();
        ushort options = _stream.ReadUInt16();

        _page.AddColumns(first, last, width, (options & 0x0001) != 0);
        _sheetDecoration.SetColumns(first, last, xf);

        for (int column = first; column <= last && column <= SheetAddress.MaxColumn; column++)
        {
            _columnFormats[column] = xf;
        }
    }

    /// <summary>
    /// Reads a <c>ROW</c> record's height and hidden flag.
    /// </summary>
    /// <remarks>
    /// Bit 15 of the height means "not set by hand" rather than being part of the number, so it
    /// is masked off and the value used regardless — which is what Calc does
    /// (<c>ImportExcel::Row34</c>, <c>sc/source/filter/excel/impop.cxx:1026</c>). A height of
    /// zero after masking falls back to the sheet default rather than collapsing the row.
    /// </remarks>
    private void ReadRow()
    {
        if (_stream.RecordLeft < 16) return;

        int row = _stream.ReadUInt16();
        _stream.Skip(4);
        int height = _stream.ReadUInt16() & 0x7FFF;
        _stream.Skip(4);
        ushort flags = _stream.ReadUInt16();

        // fUnsynced, bit 6: the height does not match the font, meaning a user set it. Without it
        // the height is Excel's own measurement and Calc recomputes it on load.
        _page.AddRow(row, height, (flags & 0x0020) != 0, (flags & 0x0040) != 0);

        // The trailing ixfe is the row's default cell format, and it only applies when the
        // record says so: fGhostDirty, bit 7 of grbit, is what makes the field mean anything.
        // Without it the field is present but inert, and a reader that honoured it anyway would
        // paint whole rows Excel leaves plain. Only the low twelve bits are the XF index; the
        // top four are flags.
        if ((flags & 0x0080) != 0 && _stream.RecordLeft >= 2)
        {
            int xf = _stream.ReadUInt16() & 0x0FFF;
            _sheetDecoration.SetRow(row, xf);
            _rowFormats[row] = xf;
        }
    }

    /// <summary>
    /// Reads a <c>NAME</c> record, keeping only the two built-in names that are print setup.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A built-in name's text is one byte holding a code rather than a name — 6 for the print
    /// area and 7 for the repeated rows and columns — and its value is a formula token array,
    /// not text. Only the reference tokens are decoded: a print area is one or more
    /// <c>tArea3d</c> tokens, optionally wrapped in a <c>tMemFunc</c> and joined by the union
    /// operator, and anything else in the array means the name is not a plain range and is
    /// skipped rather than guessed at.
    /// </para>
    /// <para>
    /// <c>itab</c> is one-based and zero means a workbook-global name, which neither of these
    /// ever is.
    /// </para>
    /// </remarks>
    private void ReadName()
    {
        if (_stream.RecordLeft < 14) return;

        ushort flags = _stream.ReadUInt16();
        _stream.ReadByte();
        int nameLength = _stream.ReadByte();
        int formulaLength = _stream.ReadUInt16();
        _stream.Skip(2);
        int sheet = _stream.ReadUInt16();
        int customMenu = _stream.ReadByte();
        int description = _stream.ReadByte();
        int help = _stream.ReadByte();
        int status = _stream.ReadByte();

        if ((flags & BiffPageRecords.NameBuiltIn) == 0 || nameLength < 1 || sheet < 1) return;

        byte code;
        if (_stream.Version == BiffVersion.Biff8)
        {
            // The name is a BIFF8 string, so its own flags byte comes first and a built-in
            // name's single character may be stored wide.
            if (_stream.RecordLeft < 2) return;
            byte stringFlags = _stream.ReadByte();
            code = _stream.ReadByte();
            if ((stringFlags & 0x01) != 0 && _stream.RecordLeft >= 1) _stream.ReadByte();
            if (nameLength > 1) _stream.Skip(Math.Min(_stream.RecordLeft, (nameLength - 1) * ((stringFlags & 0x01) != 0 ? 2 : 1)));
        }
        else
        {
            code = _stream.ReadByte();
            if (nameLength > 1) _stream.Skip(Math.Min(_stream.RecordLeft, nameLength - 1));
        }

        if (code is not (BiffPageRecords.BuiltInPrintArea or BiffPageRecords.BuiltInPrintTitles))
            return;

        List<SheetRange> ranges = XlsNameRanges.Read(
            _stream, Math.Min(formulaLength, _stream.RecordLeft), _stream.Version);

        _ = customMenu;
        _ = description;
        _ = help;
        _ = status;

        int index = sheet - 1;
        if (code == BiffPageRecords.BuiltInPrintArea)
        {
            if (!_printAreas.TryGetValue(index, out List<SheetRange>? areas))
                _printAreas[index] = areas = [];
            areas.AddRange(ranges);
            return;
        }

        // Print_Titles holds both bands in one name and tells them apart by shape, exactly as
        // the OOXML path does: a band spanning every row is the repeated columns, and one
        // spanning every column is the repeated rows.
        foreach (SheetRange range in ranges)
        {
            if (range.LastRow >= SheetAddress.MaxRow) _repeatColumns[index] = range;
            else if (range.LastColumn >= SheetAddress.MaxColumn) _repeatRows[index] = range;
        }
    }

    /// <summary>Turns a serial number into the date a spreadsheet shows for it.</summary>
    /// <remarks>
    /// <para>
    /// The conversion itself is <see cref="SpreadsheetDate"/>, shared with the XLSX path, so
    /// that the two readers cannot drift on the two things a serial number is easy to get
    /// wrong. The <c>1904</c> record chooses between two epochs, and the 1900 one has a defect
    /// in it: Lotus 1-2-3 treated 1900 as a leap year, Excel copied the bug for compatibility,
    /// and so serials 1 to 60 count from a day later than serials 61 and up. Serial 60 is
    /// Excel's 29 February 1900, a date that never happened; it comes out as 1 March, sharing
    /// that day with serial 61, which is what LibreOffice's own correction produces
    /// (<c>XclRoot::GetDateTimeFromDouble</c>).
    /// </para>
    /// <para>
    /// The fraction is rounded to the millisecond. A serial is a binary double and half past
    /// two is 0.604166666…, so taking it whole leaves a few hundred nanoseconds of residue —
    /// enough to make a timestamp compare unequal to the one it plainly is, and enough to make
    /// 14:30 render as 14:29:59 under a seconds format.
    /// </para>
    /// </remarks>
    public DateTime SerialToDateTime(double serial)
        => SpreadsheetDate.FromSerial(serial, DateSystem)
           ?? (Uses1904Epoch ? new DateTime(1904, 1, 1) : new DateTime(1899, 12, 30));

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

    private readonly record struct XfRecord
    {
        public ushort NumberFormatIndex { get; init; }

        /// <summary>Which <c>FONT</c> record the XF names.</summary>
        public ushort FontIndex { get; init; }

        /// <summary>Its alignment fields, already decoded for the version in hand.</summary>
        public BiffAlignment Alignment { get; init; }

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
        private int _declaredLastRow = -1;
        private int _declaredLastColumn = -1;
        private bool _reportedLimit;
        private bool _reportedExtent;
        private (int Row, int Column, int Xf)? _pendingString;

        public string SheetName => sheetName;

        /// <summary>Every cell's position and XF index, for the format map.</summary>
        public IEnumerable<(int Row, int Column, int Xf)> CellFormats()
        {
            foreach ((int row, SortedDictionary<int, Cell> cells) in _rows)
            {
                foreach ((int column, Cell cell) in cells) yield return (row, column, cell.Xf);
            }
        }

        /// <summary>Records what DIMENSIONS says the sheet's used range is.</summary>
        /// <remarks>
        /// Advisory, never enforced. The record states the first <em>unused</em> row and
        /// column, so both bounds are exclusive, and plenty of files understate them — a cell
        /// outside the declared range is still a cell. It is kept only so that a disagreement
        /// can be reported, because a sheet whose content runs past what it claims is usually
        /// a file that was written by something that lost track of its own extent.
        /// </remarks>
        public void SetDeclaredExtent(int firstRow, int lastRow, int firstColumn, int lastColumn)
        {
            if (lastRow <= firstRow && lastColumn <= firstColumn) return;
            _declaredLastRow = lastRow - 1;
            _declaredLastColumn = lastColumn - 1;
        }

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

        public void AddHyperlinkRange(int firstRow, int lastRow, int firstColumn, int lastColumn)
        {
            if (lastRow < firstRow || lastColumn < firstColumn) return;
            _links.Add(new SheetRange(firstColumn, firstRow, lastColumn, lastRow));
        }

        /// <summary>The <c>HLINK</c> ranges, for <see cref="SheetLayout.HyperlinkRanges"/>.</summary>
        public IReadOnlyList<SheetRange> HyperlinkRanges => _links;

        private readonly List<SheetRange> _links = [];

        /// <summary>The <c>MERGEDCELLS</c> ranges, for <see cref="SheetLayout.StatedMerges"/>.</summary>
        /// <remarks>
        /// The ranges rather than the spans the cells end up carrying: a merge whose whole block is
        /// blank has an anchor that <see cref="BuildRow"/> drops as trailing padding, and it is
        /// precisely that block a neighbour's long string must not run through.
        /// </remarks>
        public IReadOnlyList<SheetRange> StatedMerges
        {
            get
            {
                List<SheetRange> ranges = new(_merged.Count);
                foreach ((int firstRow, int lastRow, int firstColumn, int lastColumn) in _merged)
                    ranges.Add(new SheetRange(firstColumn, firstRow, lastColumn, lastRow));
                return ranges;
            }
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

            // A blank cell's XF is the whole point of the record: BIFF writes BLANK and
            // MULBLANK for cells that hold nothing but a fill or a border, so recording the
            // format only for cells with content would lose most of a formatted sheet.
            owner._sheetDecoration.SetCell(row, column, cell.Xf);

            if (!cell.IsEmpty) CheckExtent(row, column);
        }

        /// <summary>Notes a cell that falls outside what DIMENSIONS declared.</summary>
        private void CheckExtent(int row, int column)
        {
            if (_reportedExtent || _declaredLastRow < 0) return;
            if (row <= _declaredLastRow && column <= _declaredLastColumn) return;

            _reportedExtent = true;
            owner._diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Information, "PL2329",
                $"Sheet \"{sheetName}\" has content at row {row + 1}, column {column + 1}, "
                + $"outside the used range it declares (to row {_declaredLastRow + 1}, column "
                + $"{_declaredLastColumn + 1}). The content is kept; the declaration is not "
                + "trusted.",
                new DiagnosticLocation(PartName: "Workbook", Context: sheetName)));
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

                NumberFormatCode format = owner.FormatOf(Xf);
                if (!format.IsDateTime) return number;

                if (format.IsTimeOnly)
                {
                    // An elapsed format keeps the whole days: [h]:mm over 1.5 is 36 hours,
                    // not 12. A clock format keeps only the remainder.
                    return SpreadsheetDate.ToTimeOfDay(
                        number, keepWholeDays: format.Sections[0].HasElapsed);
                }

                return owner.SerialToDateTime(number);
            }

            /// <summary>The text the authoring application displayed for this cell.</summary>
            private string DisplayedText(XlsWorkbookReader owner)
            {
                if (Error is { } error) return BiffErrors.Text(error);

                // Calc shows TRUE and FALSE whatever the number format says, and the format
                // Excel gives a boolean cell spells the same two words out as literals, so
                // the two agree and this needs no format lookup.
                if (Boolean is { } boolean) return boolean ? "TRUE" : "FALSE";

                if (Text is { } text) return NumberFormatter.Format(owner.FormatOf(Xf), text);
                if (Number is not { } number) return string.Empty;

                return NumberFormatter.Format(owner.FormatOf(Xf), number, owner.DateSystem);
            }
        }
    }
}

using System.Text;
using XRay.Office.Core.Diagnostics;

namespace XRay.Office.Spreadsheets.MsBinary;

/// <summary>The record identifiers of BIFF's pivot cache sub-grammar.</summary>
/// <remarks>
/// Numbered as <c>sc/source/filter/inc/xlpivot.hxx</c> numbers them. The first four arrive in
/// the workbook globals and describe <em>where</em> a cache's source data is; the rest live in
/// the <c>_SX_DB_CUR</c> storage and are the data itself.
/// </remarks>
internal static class BiffPivotRecords
{
    // Workbook globals.
    public const ushort DconRef = 0x0051;
    public const ushort DconName = 0x0052;
    public const ushort SxIdStm = 0x00D5;
    public const ushort Sxvs = 0x00E3;

    // The pivot cache stream.
    public const ushort SxDb = 0x00C6;
    public const ushort SxField = 0x00C7;
    public const ushort SxIndexList = 0x00C8;
    public const ushort SxDouble = 0x00C9;
    public const ushort SxBoolean = 0x00CA;
    public const ushort SxError = 0x00CB;
    public const ushort SxInteger = 0x00CC;
    public const ushort SxString = 0x00CD;
    public const ushort SxDateTime = 0x00CE;
    public const ushort SxEmpty = 0x00CF;
    public const ushort SxNumGroup = 0x00D8;
    public const ushort SxGroupInfo = 0x00D9;

    /// <summary>True for any of the seven records that carry one cached value.</summary>
    public static bool IsItem(ushort id)
        => id is SxDouble or SxBoolean or SxError or SxInteger or SxString or SxDateTime or SxEmpty;

    // SXVS source types (EXC_SXVS_*).
    public const ushort SourceUnknown = 0x0000;
    public const ushort SourceSheet = 0x0001;
    public const ushort SourceExternal = 0x0002;

    // SXFIELD flags (EXC_SXFIELD_*).
    public const ushort FieldHasItems = 0x0001;
    public const ushort FieldPostponed = 0x0002;
    public const ushort FieldCalculated = 0x0004;
    public const ushort FieldHasChild = 0x0008;
    public const ushort FieldNumGroup = 0x0010;
    public const ushort Field16BitIndexes = 0x0200;
    public const ushort FieldDataMask = 0x0DE0;

    // The item data types a SXFIELD's flags can declare (EXC_SXFIELD_DATA_*).
    public const ushort DataNone = 0x0000;
    public const ushort DataString = 0x0480;
    public const ushort DataInteger = 0x0520;
    public const ushort DataDouble = 0x0560;
    public const ushort DataStringInteger = 0x05A0;
    public const ushort DataStringDouble = 0x05E0;
    public const ushort DataDate = 0x0900;
    public const ushort DataDateEmpty = 0x0980;
    public const ushort DataDateNumber = 0x0D00;
    public const ushort DataDateString = 0x0D80;

    /// <summary>True for a data type the importer recognises, which decides a field's kind.</summary>
    public static bool IsKnownDataType(ushort type)
        => type is DataString or DataInteger or DataDouble or DataStringInteger
            or DataStringDouble or DataDate or DataDateEmpty or DataDateNumber or DataDateString;
}

/// <summary>Which of BIFF's seven pivot-cache item records produced a cached value.</summary>
internal enum XlsPivotItemKind
{
    /// <summary>An <c>SXEMPTY</c>: the source cell was blank, and nothing is written for it.</summary>
    Empty,

    /// <summary>An <c>SXSTRING</c>.</summary>
    Text,

    /// <summary>An <c>SXDOUBLE</c>.</summary>
    Double,

    /// <summary>An <c>SXDATETIME</c>.</summary>
    DateTime,

    /// <summary>An <c>SXINTEGER</c>.</summary>
    Integer,

    /// <summary>An <c>SXBOOLEAN</c>.</summary>
    Boolean,

    /// <summary>An <c>SXERROR</c>.</summary>
    Error,
}

/// <summary>One value held in a pivot cache.</summary>
/// <remarks>
/// The union <c>XclImpPCItem</c> is (<c>sc/source/filter/inc/xlpivot.hxx:363</c>), flattened:
/// which member is meaningful is what <see cref="Kind"/> says.
/// </remarks>
internal readonly record struct XlsPivotItem
{
    /// <summary>Which record produced this value.</summary>
    public XlsPivotItemKind Kind { get; init; }

    /// <summary>The text of an <see cref="XlsPivotItemKind.Text"/> item.</summary>
    public string? Text { get; init; }

    /// <summary>The value of a numeric, integer or Boolean item.</summary>
    public double Number { get; init; }

    /// <summary>The value of a date/time item, as a broken-down calendar date.</summary>
    public (int Year, int Month, int Day, int Hour, int Minute, int Second) When { get; init; }

    /// <summary>The BIFF error code of an <see cref="XlsPivotItemKind.Error"/> item.</summary>
    public byte ErrorCode { get; init; }

    /// <summary>An item that stands for a blank source cell.</summary>
    public static XlsPivotItem Blank => new() { Kind = XlsPivotItemKind.Empty };
}

/// <summary>
/// What the workbook globals say about one pivot cache: which stream holds it, and where its
/// source data came from.
/// </summary>
/// <remarks>
/// A cache is opened by an <c>SXIDSTM</c> and described by the <c>SXVS</c>, <c>DCONREF</c> and
/// <c>DCONNAME</c> records that follow it — the same grouping
/// <c>XclImpPivotTableManager::ReadSxidstm</c> makes
/// (<c>sc/source/filter/excel/xipivot.cxx:1657</c>), where each of the later three is applied to
/// the most recently opened cache.
/// </remarks>
internal sealed class XlsPivotCacheSource
{
    /// <summary>The <c>SXIDSTM</c> identifier, which names the stream in <c>_SX_DB_CUR</c>.</summary>
    public ushort StreamId { get; private set; }

    /// <summary>The <c>SXVS</c> source type.</summary>
    public ushort SourceType { get; private set; } = BiffPivotRecords.SourceUnknown;

    /// <summary>The sheet the source range is on, as the file names it.</summary>
    public string TabName { get; private set; } = string.Empty;

    /// <summary>True when the source data is in this workbook rather than another one.</summary>
    public bool IsSelfReference { get; private set; }

    /// <summary>The defined name the source range is, when <c>DCONNAME</c> states one.</summary>
    public string SourceRangeName { get; private set; } = string.Empty;

    /// <summary>Reads <c>SXIDSTM</c>: which stream in the cache storage holds this cache.</summary>
    public void ReadStreamId(BiffRecordReader stream) => StreamId = stream.ReadUInt16();

    /// <summary>Reads <c>SXVS</c>: whether the source is a sheet, an external file or a query.</summary>
    public void ReadSourceType(BiffRecordReader stream) => SourceType = stream.ReadUInt16();

    /// <summary>Reads <c>DCONREF</c>: the source range and the encoded URL naming its sheet.</summary>
    /// <remarks>
    /// Only the first one counts, and only when an <c>SXVS</c> has said the source is a sheet:
    /// <c>DCONREF</c> is used in other contexts too, and a second one belongs to something else
    /// (<c>XclImpPivotCache::ReadDconref</c>, <c>sc/source/filter/excel/xipivot.cxx:642</c>).
    /// </remarks>
    public void ReadSourceReference(BiffRecordReader stream)
    {
        if (TabName.Length > 0 || SourceType != BiffPivotRecords.SourceSheet) return;

        // The range, which is not needed here: the generated sheet's extent is whatever the
        // cache turns out to hold, and a range into a sheet that exists is never generated.
        stream.ReadUInt16();
        stream.ReadUInt16();
        stream.ReadUInt16();
        stream.ReadUInt16();

        string encoded = stream.ReadString(eightBitLength: false);
        DecodeUrl(encoded, out string tabName, out bool selfReference);
        TabName = tabName;
        IsSelfReference = selfReference;
    }

    /// <summary>Reads <c>DCONNAME</c>: the defined name the source range is.</summary>
    /// <remarks>
    /// The word after the name is zero for a workbook-scoped name and the length of a sheet
    /// name otherwise; only the workbook-scoped form is a reference into this file, which is
    /// what makes it the test for self-reference here as it is in
    /// <c>XclImpPivotCache::ReadDConName</c> (<c>xipivot.cxx:664</c>).
    /// </remarks>
    public void ReadSourceName(BiffRecordReader stream)
    {
        string name = stream.ReadString(eightBitLength: false);
        IsSelfReference = stream.ReadUInt16() == 0;
        SourceRangeName = IsSelfReference ? name : string.Empty;
    }

    /// <summary>
    /// Pulls the sheet name and the self-reference flag out of an encoded Excel URL.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A transcription of the state machine in <c>XclImpUrlHelper::DecodeUrl</c>
    /// (<c>sc/source/filter/excel/xihelper.cxx:613</c>), less the part that rebuilds the file
    /// path: the pivot cache reader uses only the sheet name and the flag, and the path it also
    /// decodes is stored and never read (<c>XclImpPivotCache::maUrl</c>).
    /// </para>
    /// <para>
    /// The distinction the flag makes is the whole point of this method. A cache whose source is
    /// a sheet of <em>this</em> workbook is read straight from that sheet and generates nothing;
    /// one naming another file has no sheet to read, and its cached copy of the data becomes a
    /// sheet of its own.
    /// </para>
    /// </remarks>
    internal static void DecodeUrl(string encoded, out string tabName, out bool selfReference)
    {
        const char startEncoded = '\x01';
        const char startSelf = '\x02';
        const char startSelfEncoded = '\x03';
        const char urlDosDrive = '\x01';
        const char urlRaw = '\x05';

        selfReference = false;
        StringBuilder sheet = new();

        // Initial, path, file name, sheet name. The raw mode the C++ has is only reachable
        // through the path building this omits, and it never reaches a sheet name.
        int state = 0;
        for (int at = 0; at < encoded.Length; at++)
        {
            char c = encoded[at];
            switch (state)
            {
                case 0:
                    if (c == startEncoded) state = 1;
                    else if (c is startSelf or startSelfEncoded) { selfReference = true; state = 3; }
                    else if (c == '[') state = 2;
                    else state = 1;
                    break;

                case 1:
                    // Both of these consume characters that must not be mistaken for the '['
                    // that opens a file name.
                    if (c == urlDosDrive) at++;
                    else if (c == urlRaw && at + 1 < encoded.Length) at += 1 + encoded[at + 1];
                    else if (c == '[') state = 2;
                    break;

                case 2:
                    if (c == ']') state = 3;
                    break;

                default:
                    sheet.Append(c);
                    break;
            }
        }

        tabName = sheet.ToString();
    }
}

/// <summary>A sheet synthesised from a pivot cache's own copy of its source data.</summary>
internal sealed class XlsPivotCacheSheet
{
    /// <summary>The sheet's name.</summary>
    public required string Name { get; init; }

    /// <summary>How many columns the cache's fields fill, the header row included.</summary>
    public required int ColumnCount { get; init; }

    /// <summary>How many rows the cache's records fill, the header row included.</summary>
    public required int RowCount { get; init; }

    /// <summary>Every cell that holds something, keyed by position.</summary>
    public required IReadOnlyDictionary<(int Row, int Column), XlsPivotItem> Cells { get; init; }
}

/// <summary>
/// Reads a BIFF8 pivot cache and rebuilds the source data it holds a copy of.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why a workbook can gain a sheet no record names.</strong> A pivot table is drawn from
/// a <em>cache</em> — a flattened snapshot of the source range, stored in the <c>_SX_DB_CUR</c>
/// storage rather than in the workbook stream. When that source range is in another file, or on
/// a sheet that has since been deleted, the cache is the only copy of the data left. Calc has no
/// pivot cache of its own, so its importer materialises the snapshot as a real sheet named
/// <c>DPCache</c> and points the pivot table at that
/// (<c>XclImpPivotCache::ReadPivotCacheStream</c>, <c>sc/source/filter/excel/xipivot.cxx:680</c>).
/// The sheet prints, and on a large cache it can be most of the workbook's pages.
/// </para>
/// <para>
/// <strong>The storage layout.</strong> One <c>SXDB</c> header, then one <c>SXFIELD</c> per
/// column, then one <c>SXINDEXLIST</c> per source record. A field's values reach the reader by
/// one of two routes and the flags in its <c>SXFIELD</c> say which. An <em>inline</em> field is
/// followed immediately by its distinct values, and each <c>SXINDEXLIST</c> then holds one index
/// into that list per inline field. A <em>postponed</em> field has no value list at all; its
/// values arrive after each <c>SXINDEXLIST</c>, one per postponed field, in field order, round
/// and round. A cache commonly mixes the two, and this one does: two inline fields indexed by
/// the list, six postponed fields whose values follow it.
/// </para>
/// </remarks>
internal static class XlsPivotCacheReader
{
    /// <summary>Calc's last row, which bounds the generated sheet as it bounds the C++ one.</summary>
    private const int MaxRow = 1_048_575;

    /// <summary>Calc's last column.</summary>
    private const int MaxColumn = 16_383;

    /// <summary>The name the generated sheet takes, before any suffix.</summary>
    public const string SheetName = "DPCache";

    /// <summary>
    /// Decides whether a cache needs a generated sheet, and what that sheet would be called.
    /// </summary>
    /// <param name="source">What the workbook globals said about the cache.</param>
    /// <param name="sheetNames">The workbook's own sheet names, to resolve the source against.</param>
    /// <returns>The sheet's name, or null when the cache's source data is already in the file.</returns>
    /// <remarks>
    /// Follows <c>ReadPivotCacheStream</c>'s opening: a cache whose source is a sheet of this
    /// workbook is read from that sheet, and only a cache naming another file, or a sheet that
    /// is no longer there, generates one.
    /// </remarks>
    public static string? GeneratedSheetName(XlsPivotCacheSource source, IReadOnlyList<string> sheetNames)
    {
        ArgumentNullException.ThrowIfNull(source);
        ArgumentNullException.ThrowIfNull(sheetNames);

        if (source.SourceType is not (BiffPivotRecords.SourceSheet or BiffPivotRecords.SourceExternal))
            return null;

        if (source.IsSelfReference)
        {
            if (source.SourceRangeName.Length > 0) return null;

            foreach (string name in sheetNames)
            {
                if (string.Equals(name, source.TabName, StringComparison.Ordinal)) return null;
            }
        }

        return source.TabName.Length > 0 ? $"{SheetName}_{source.TabName}" : SheetName;
    }

    /// <summary>
    /// Reads a cache stream and lays its records out as a grid: field names across the first
    /// row, one source record per row after it.
    /// </summary>
    /// <param name="cache">The bytes of the stream in the <c>_SX_DB_CUR</c> storage.</param>
    /// <param name="name">The sheet name, already made unique against the workbook's own.</param>
    /// <param name="version">The workbook's BIFF generation, which decides how strings read.</param>
    /// <param name="encoding">The workbook's code page, for a BIFF5 byte string.</param>
    /// <param name="diagnostics">Where damage found while reading is recorded.</param>
    /// <returns>The grid, or null when the cache holds no field with data.</returns>
    public static XlsPivotCacheSheet? Read(
        byte[] cache,
        string name,
        BiffVersion version,
        Encoding encoding,
        List<Diagnostic> diagnostics)
    {
        ArgumentNullException.ThrowIfNull(cache);
        ArgumentNullException.ThrowIfNull(diagnostics);

        BiffRecordReader stream = new(cache, diagnostics) { Version = version, Encoding = encoding };

        List<Field> fields = [];
        List<Field> inlineFields = [];   // fields the SXINDEXLIST indexes into, in field order
        List<Field> postponedFields = [];
        Dictionary<(int Row, int Column), XlsPivotItem> cells = [];

        Field? current = null;
        int postponed = 0;
        int column = 0;
        int row = 0;

        while (stream.MoveNext())
        {
            ushort id = stream.RecordId;
            if (id == BiffRecords.Eof) break;

            switch (id)
            {
                case BiffPivotRecords.SxField:
                {
                    current = ReadField(stream);
                    fields.Add(current);

                    // A field takes a column when it has values of its own to put in it. A
                    // grouping field, which only re-labels another field's values, does not.
                    //
                    // The list membership is *not* conditional on there being a column left for
                    // it: an SXINDEXLIST holds one index per field on the inline list, so a field
                    // dropped from the list would leave every field after it reading the wrong
                    // byte. The C++ makes the same split — the list always takes the field and
                    // only WriteFieldNameToSource is guarded by the column limit.
                    if (current.HasOriginalItems || current.IsStandard)
                    {
                        (current.HasPostponedItems ? postponedFields : inlineFields).Add(current);

                        if (column <= MaxColumn)
                        {
                            current.Column = column++;
                            cells[(0, current.Column)] = new XlsPivotItem
                            {
                                Kind = XlsPivotItemKind.Text,
                                Text = current.Name,
                            };
                        }
                    }

                    // Items that follow belong to this field only if it declared any inline.
                    if (!current.HasInlineItems) current = null;
                    break;
                }

                case BiffPivotRecords.SxIndexList:
                {
                    if (row <= MaxRow && ++row <= MaxRow)
                    {
                        foreach (Field field in inlineFields)
                        {
                            int index = field.Uses16BitIndexes ? stream.ReadUInt16() : stream.ReadByte();
                            if (index < field.OriginalItems.Count && field.Column >= 0)
                                Place(cells, row, field.Column, field.OriginalItems[index]);
                        }
                    }

                    current = null;
                    break;
                }

                case BiffPivotRecords.SxNumGroup:
                    // The three items after it are the grouping's limits and step rather than
                    // source values, which is what the flag is for.
                    if (current is not null)
                    {
                        stream.ReadUInt16();
                        current.NumberGroupInfoRead = current.IsNumberGroup || current.IsDateGroup;
                    }

                    break;

                case BiffPivotRecords.SxGroupInfo:
                    break;

                default:
                {
                    if (!BiffPivotRecords.IsItem(id)) break;

                    XlsPivotItem item = ReadItem(stream, id);

                    if (current is not null)
                    {
                        current.Add(item);
                    }
                    else if (postponedFields.Count > 0)
                    {
                        Field field = postponedFields[postponed];
                        field.Add(item);

                        // A cache with no inline field has no SXINDEXLIST to start a row, so
                        // the first postponed field of each pass starts one instead.
                        if (inlineFields.Count == 0 && postponed == 0) row++;
                        if (row <= MaxRow && field.Column >= 0 && field.OriginalItems.Count > 0)
                            Place(cells, row, field.Column, field.OriginalItems[^1]);

                        postponed = (postponed + 1) % postponedFields.Count;
                    }

                    break;
                }
            }
        }

        if (column == 0) return null;

        return new XlsPivotCacheSheet
        {
            Name = name,
            ColumnCount = column,
            RowCount = row + 1,
            Cells = cells,
        };
    }

    /// <summary>Writes one value, skipping the blanks that stand for an empty source cell.</summary>
    private static void Place(
        Dictionary<(int Row, int Column), XlsPivotItem> cells, int row, int column, XlsPivotItem item)
    {
        // XclImpPCItem::WriteToSource asks each accessor in turn and an empty item answers none
        // of them, so it writes nothing at all rather than a blank cell.
        if (item.Kind == XlsPivotItemKind.Empty) return;
        cells[(row, column)] = item;
    }

    /// <summary>Reads one of the seven item records.</summary>
    private static XlsPivotItem ReadItem(BiffRecordReader stream, ushort id) => id switch
    {
        BiffPivotRecords.SxDouble => new XlsPivotItem
        {
            Kind = XlsPivotItemKind.Double,
            Number = stream.ReadDouble(),
        },
        BiffPivotRecords.SxBoolean => new XlsPivotItem
        {
            Kind = XlsPivotItemKind.Boolean,
            Number = stream.ReadUInt16() != 0 ? 1 : 0,
        },
        BiffPivotRecords.SxError => new XlsPivotItem
        {
            Kind = XlsPivotItemKind.Error,
            ErrorCode = (byte)stream.ReadUInt16(),
        },
        BiffPivotRecords.SxInteger => new XlsPivotItem
        {
            Kind = XlsPivotItemKind.Integer,
            Number = stream.ReadInt16(),
        },
        BiffPivotRecords.SxString => new XlsPivotItem
        {
            Kind = XlsPivotItemKind.Text,
            Text = stream.ReadString(eightBitLength: false),
        },
        BiffPivotRecords.SxDateTime => ReadDateTime(stream),
        _ => XlsPivotItem.Blank,
    };

    private static XlsPivotItem ReadDateTime(BiffRecordReader stream)
    {
        int year = stream.ReadUInt16();
        int month = stream.ReadUInt16();
        int day = stream.ReadByte();
        int hour = stream.ReadByte();
        int minute = stream.ReadByte();
        int second = stream.ReadByte();

        return new XlsPivotItem
        {
            Kind = XlsPivotItemKind.DateTime,
            When = (year, month, day, hour, minute, second),
        };
    }

    /// <summary>
    /// Reads an <c>SXFIELD</c> and classifies the field, which is what decides where its values
    /// come from.
    /// </summary>
    /// <remarks>
    /// The classification is deliberately restrictive, exactly as
    /// <c>XclImpPCField::ReadSxfield</c> is (<c>xipivot.cxx:249</c>): a field is standard,
    /// grouped, calculated or nothing, decided by agreement between its flags and its four item
    /// counts. Anything that does not agree is left unknown and contributes no column, because
    /// guessing at a field whose shape is not recognised would put values in the wrong ones.
    /// </remarks>
    private static Field ReadField(BiffRecordReader stream)
    {
        Field field = new()
        {
            Flags = stream.ReadUInt16(),
        };

        stream.ReadUInt16();                      // group child field index
        stream.ReadUInt16();                      // group base field index
        int visible = stream.ReadUInt16();
        int grouped = stream.ReadUInt16();
        int based = stream.ReadUInt16();
        int original = stream.ReadUInt16();
        field.GroupItemCount = grouped;
        field.OriginalItemCount = original;
        field.Name = stream.RecordLeft >= 3 ? stream.ReadString(eightBitLength: false) : string.Empty;

        ushort flags = field.Flags;
        bool items = (flags & BiffPivotRecords.FieldHasItems) != 0;
        bool postponed = (flags & BiffPivotRecords.FieldPostponed) != 0;
        bool calculated = (flags & BiffPivotRecords.FieldCalculated) != 0;
        bool child = (flags & BiffPivotRecords.FieldHasChild) != 0;
        bool numeric = (flags & BiffPivotRecords.FieldNumGroup) != 0;

        ushort type = (ushort)(flags & BiffPivotRecords.FieldDataMask);
        bool known = BiffPivotRecords.IsKnownDataType(type);
        bool typeless = type == BiffPivotRecords.DataNone;

        if (visible == 0 && !postponed) return field;

        if (items && !postponed)
        {
            if (!calculated)
            {
                if (!numeric)
                {
                    if (known && grouped == 0 && based == 0 && original == visible)
                        field.Kind = FieldKind.Standard;
                    else if (typeless && grouped == visible && based > 0 && original == 0)
                        field.Kind = FieldKind.StandardGroup;
                }
                else if (grouped == visible && based == 0)
                {
                    if (!child && known && original > 0)
                    {
                        field.Kind = type switch
                        {
                            BiffPivotRecords.DataInteger or BiffPivotRecords.DataDouble
                                => FieldKind.NumberGroup,
                            BiffPivotRecords.DataDate => FieldKind.DateGroup,
                            _ => FieldKind.Unknown,
                        };
                    }
                    else if (child && type == BiffPivotRecords.DataDate && original > 0)
                    {
                        field.Kind = FieldKind.DateGroup;
                    }
                    else if (typeless && original == 0)
                    {
                        field.Kind = FieldKind.DateChild;
                    }
                }
            }
            else if (!child && !numeric && grouped == 0 && based == 0 && original == 0)
            {
                field.Kind = FieldKind.Calculated;
            }
        }
        else if (!items && postponed)
        {
            if (!calculated && !child && !numeric && known && grouped == 0 && based == 0 && original == 0)
                field.Kind = FieldKind.Standard;
        }
        else if (!calculated && !child && !numeric && grouped == 0 && based == 0 && original == 0)
        {
            field.Kind = FieldKind.Standard;
        }

        return field;
    }

    /// <summary>What a <c>SXFIELD</c>'s flags and item counts turned out to describe.</summary>
    private enum FieldKind
    {
        Unknown,
        Standard,
        StandardGroup,
        NumberGroup,
        DateGroup,
        DateChild,
        Calculated,
    }

    /// <summary>One column of a pivot cache, and the values it holds.</summary>
    private sealed class Field
    {
        public ushort Flags { get; init; }

        public string Name { get; set; } = string.Empty;

        public FieldKind Kind { get; set; } = FieldKind.Unknown;

        public int GroupItemCount { get; set; }

        public int OriginalItemCount { get; set; }

        /// <summary>Which column of the generated sheet this field fills, or -1 for none.</summary>
        public int Column { get; set; } = -1;

        /// <summary>The distinct values an <c>SXINDEXLIST</c> indexes into.</summary>
        public List<XlsPivotItem> OriginalItems { get; } = [];

        /// <summary>True once <c>SXNUMGROUP</c> has been read, which redirects the next three items.</summary>
        public bool NumberGroupInfoRead { get; set; }

        public bool IsStandard => Kind == FieldKind.Standard;

        public bool IsNumberGroup => Kind == FieldKind.NumberGroup;

        public bool IsDateGroup => Kind is FieldKind.DateGroup or FieldKind.DateChild;

        private bool IsGroup => Kind is FieldKind.StandardGroup or FieldKind.NumberGroup
            or FieldKind.DateGroup or FieldKind.DateChild;

        private bool IsSupported => Kind is not (FieldKind.Calculated or FieldKind.Unknown);

        public bool HasPostponedItems
            => IsStandard && (Flags & BiffPivotRecords.FieldPostponed) != 0;

        public bool HasOriginalItems
            => IsSupported && (OriginalItemCount > 0 || HasPostponedItems);

        public bool HasInlineItems
            => (IsStandard || IsGroup) && (GroupItemCount > 0 || OriginalItemCount > 0);

        public bool Uses16BitIndexes
            => IsStandard && (Flags & BiffPivotRecords.Field16BitIndexes) != 0;

        /// <summary>Files one item away, which list depending on what has been read so far.</summary>
        public void Add(XlsPivotItem item)
        {
            // The three items after SXNUMGROUP are the grouping's own limits; only what follows
            // them is source data.
            if (NumberGroupInfoRead)
            {
                if (_groupLimits < 3) { _groupLimits++; return; }
                OriginalItems.Add(item);
                return;
            }

            // For a standard field the visible items and the original items are the same list.
            if (IsStandard) OriginalItems.Add(item);
        }

        private int _groupLimits;
    }
}

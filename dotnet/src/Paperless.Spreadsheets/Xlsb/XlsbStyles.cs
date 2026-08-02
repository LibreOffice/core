using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.Ooxml;

namespace Paperless.Spreadsheets.Xlsb;

/// <summary>
/// Reads <c>styles.bin</c> as far as extraction needs it: the number format each cell format names.
/// </summary>
/// <remarks>
/// <para>
/// The result is an <see cref="XlsxStyles"/>, because it is the same table: an XLSB
/// <c>NUMFMT</c> record carries the id and the code an XLSX <c>&lt;numFmt&gt;</c> carries, and an
/// <c>XF</c> inside <c>CELLXFS</c> carries the <c>numFmtId</c> an <c>&lt;xf&gt;</c> carries. Only
/// the encoding differs, so only the reading is written twice.
/// </para>
/// <para>
/// <strong>The container is what distinguishes a cell format from a style format, and nothing
/// else does.</strong> <c>XF</c> is one record identifier used inside both <c>CELLSTYLEXFS</c>
/// and <c>CELLXFS</c>, and LibreOffice tells them apart purely by which container it is inside
/// when it sees one (<c>createCellXf</c> versus <c>createStyleXf</c>,
/// <c>sc/source/filter/oox/stylesfragment.cxx:302-306</c>). A flat walk that collects every
/// <c>XF</c> therefore builds a table whose indices are all shifted by however many named styles
/// the workbook has — which reads as a number-format bug and is a parsing one. Hence the
/// container stack below, small as it is.
/// </para>
/// </remarks>
internal static class XlsbStyles
{
    /// <summary>
    /// Reads the styles part, or an empty table when it is absent.
    /// </summary>
    /// <remarks>
    /// The default font travels back with the number formats because it is in the same part and
    /// this is the only walk of it. It is not extraction's business — a column width is what
    /// wants it, and only layout has columns — but reading it a second time later would mean
    /// opening and decoding <c>styles.bin</c> twice. See <see cref="SheetColumnDigits"/>.
    /// </remarks>
    /// <param name="part">The part's bytes, or null when it is absent.</param>
    public static (XlsxStyles Styles, SheetDefaultFont? DefaultFont) Read(byte[]? part)
    {
        if (part is null) return (XlsxStyles.Empty, null);

        Dictionary<int, string> customCodes = [];
        List<int> cellFormatIds = [];
        SheetDefaultFont? defaultFont = null;
        int container = 0;

        foreach (Biff12Record record in Biff12Stream.Records(part))
        {
            // Every container's end identifier is its start plus one, throughout the format.
            if (record.Id == container + 1) { container = 0; continue; }

            switch (record.Id)
            {
                case Biff12.NumFmts or Biff12.CellXfs or Biff12.CellStyleXfs or Biff12.Fonts:
                    container = record.Id;
                    break;

                case Biff12.NumFmt when container == Biff12.NumFmts:
                {
                    Biff12Cursor cursor = new(record.Data.Span);
                    int id = cursor.ReadUInt16();
                    string code = cursor.ReadString();
                    if (code.Length > 0) _ = customCodes.TryAdd(id, code);
                    break;
                }

                case Biff12.Xf when container == Biff12.CellXfs:
                {
                    // styleXfId, numFmtId, fontId, fillId, borderId, then the packed alignment
                    // and the used-flags word. Only the second is extraction's business.
                    Biff12Cursor cursor = new(record.Data.Span);
                    _ = cursor.ReadUInt16();
                    cellFormatIds.Add(cursor.ReadUInt16());
                    break;
                }

                case Biff12.Font when container == Biff12.Fonts && defaultFont is null:
                    defaultFont = ReadFont(record);
                    break;
            }
        }

        return (XlsxStyles.FromRecords(customCodes, cellFormatIds), defaultFont);
    }

    /// <summary>
    /// Reads a <c>BrtFont</c> as far as measuring a digit needs it.
    /// </summary>
    /// <remarks>
    /// <c>Font::importFont</c> (<c>sc/source/filter/oox/stylesbuffer.cxx:661</c>): the height in
    /// twips, the flags, the weight and the escapement as words; the underline, family, character
    /// set and one unused byte; an eight-byte colour; the scheme byte; then the name. Italic is
    /// bit one of the flags (<c>BIFF_FONTFLAG_ITALIC</c>, <c>:199</c>), and the weight is the same
    /// 100–1000 scale every BIFF uses.
    /// </remarks>
    private static SheetDefaultFont? ReadFont(Biff12Record record)
    {
        // Height, flags, weight, escapement, four bytes of underline/family/charset/unused, the
        // eight-byte colour and the scheme byte: 21 before the name.
        const int FixedBytes = 21;
        const ushort ItalicFlag = 0x0002;

        if (record.Data.Length < FixedBytes) return null;

        Biff12Cursor cursor = new(record.Data.Span);
        int height = cursor.ReadUInt16();
        ushort flags = cursor.ReadUInt16();
        int weight = cursor.ReadUInt16();
        cursor.Skip(FixedBytes - 6);
        string name = cursor.ReadString();

        return new SheetDefaultFont(
            name.Length == 0 ? null : name,
            height > 0 ? Length.FromTwips(height) : Length.FromPoints(10),
            weight is >= 100 and <= 1000 ? weight : 400,
            (flags & ItalicFlag) != 0);
    }
}

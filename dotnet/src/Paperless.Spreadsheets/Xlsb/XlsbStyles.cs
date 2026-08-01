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
    /// <summary>Reads the styles part, or an empty table when it is absent.</summary>
    public static XlsxStyles Read(byte[]? part)
    {
        if (part is null) return XlsxStyles.Empty;

        Dictionary<int, string> customCodes = [];
        List<int> cellFormatIds = [];
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
            }
        }

        return XlsxStyles.FromRecords(customCodes, cellFormatIds);
    }
}

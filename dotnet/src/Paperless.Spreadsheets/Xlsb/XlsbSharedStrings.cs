namespace Paperless.Spreadsheets.Xlsb;

/// <summary>
/// The shared string table of an XLSB workbook: <c>SST</c> and the <c>SI</c> records inside it.
/// </summary>
/// <remarks>
/// <para>
/// One <c>SI</c> per entry, in order, and the index a cell states is the position — the count in
/// the <c>SST</c> header is a hint the reader does not need and does not trust, because a
/// truncated part still has usable entries before the truncation.
/// </para>
/// <para>
/// <strong>Every <c>SI</c> is a rich string whether or not it has any runs.</strong> The leading
/// flag byte says which of the two optional tails follow (<c>RichString::importString</c>,
/// <c>sc/source/filter/oox/richstring.cxx:329</c>), and reading the string without it takes the
/// flag as the low byte of the character count — which produces one plausible entry and then
/// desynchronises the whole table.
/// </para>
/// </remarks>
internal static class XlsbSharedStrings
{
    private const byte HasFonts = 0x01;
    private const byte HasPhonetics = 0x02;

    /// <summary>Reads the shared strings, or an empty table when the part is absent.</summary>
    public static IReadOnlyList<string> Read(byte[]? part)
    {
        if (part is null) return [];

        List<string> strings = [];
        foreach (Biff12Record record in Biff12Stream.Records(part))
        {
            if (record.Id != Biff12.Si) continue;

            Biff12Cursor cursor = new(record.Data.Span);
            strings.Add(ReadRichString(ref cursor, rich: true));
        }
        return strings;
    }

    /// <summary>
    /// The plain text of a BIFF12 string, run structure read past.
    /// </summary>
    /// <remarks>
    /// The runs have to be <em>consumed</em> rather than ignored where the caller reads on — a
    /// <c>FORMULA_STRING</c> cell has its token array after the string — so the portion list and
    /// the phonetic tail are walked even though extraction records neither. A run is four bytes,
    /// a character position and a font id (<c>FontPortionModel::read</c>, <c>:151</c>).
    /// </remarks>
    /// <param name="cursor">Positioned at the string.</param>
    /// <param name="rich">
    /// True where the string is preceded by its flag byte, which is the case for every
    /// <c>SI</c> and every <c>CELL_RSTRING</c> and not for a <c>CELL_STRING</c>.
    /// </param>
    public static string ReadRichString(ref Biff12Cursor cursor, bool rich)
    {
        byte flags = rich ? cursor.ReadByte() : (byte)0;
        string text = cursor.ReadString();

        if ((flags & HasFonts) != 0)
        {
            int runs = cursor.ReadInt32();
            cursor.Skip(Math.Clamp(runs, 0, cursor.Remaining / 4) * 4);
        }

        if ((flags & HasPhonetics) != 0)
        {
            _ = cursor.ReadString();
            int portions = cursor.ReadInt32();
            cursor.Skip(Math.Clamp(portions, 0, cursor.Remaining / 6) * 6);

            // PhoneticSettings::importStringData is a font id and two packed fields.
            cursor.Skip(4);
        }

        return text;
    }
}

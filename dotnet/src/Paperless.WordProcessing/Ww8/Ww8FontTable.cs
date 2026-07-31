using System.Buffers.Binary;
using System.Text;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// A DOC's font table: the families its <c>sprmCRgFtc0</c> indexes name.
/// </summary>
/// <remarks>
/// <para>
/// A run of <c>FFN</c> structures, each preceded by its own length. They are variable-sized and there is
/// no index, so a reader can only walk them forwards — which means one malformed length loses every font
/// after it, and is why the walk stops rather than trying to resynchronise.
/// </para>
/// <para>
/// The name is at a fixed offset within each entry, after four bytes of flags and weight and character
/// set, one index to the alternate name, ten bytes of PANOSE and twenty-four of font signature. Those
/// last two are the ones easy to forget: skipping only PANOSE puts the name twenty-four bytes early and
/// reads the signature as UTF-16, which produces a plausible-looking string of CJK.
/// </para>
/// </remarks>
public sealed class Ww8FontTable
{
    /// <summary>Where a Word 8 entry's name starts, measured from the byte after its length.</summary>
    /// <remarks>
    /// One byte of flags, two of weight, one of character set, one of alternate-name index, ten of
    /// PANOSE and twenty-four of font signature. LibreOffice walks the same offsets a field at a time in
    /// <c>WW8Fonts::WW8Fonts</c>.
    /// </remarks>
    private const int NameOffset = 1 + 2 + 1 + 1 + 10 + 24;

    /// <summary>The shortest payload a Word 8 entry can have and still hold a name.</summary>
    /// <remarks>
    /// Forty-one bytes, which is <see cref="NameOffset"/> plus the two of a terminator. An entry shorter
    /// than this is malformed, and LibreOffice stops the walk at one for the same reason.
    /// </remarks>
    private const int MinimumPayload = NameOffset + 2;

    /// <summary>How many fonts are read before the rest are ignored.</summary>
    private const int MaxFonts = 4096;

    private readonly string[] _names;

    private Ww8FontTable(string[] names) => _names = names;

    /// <summary>An empty table, for a document that declares none.</summary>
    public static Ww8FontTable Empty { get; } = new([]);

    /// <summary>How many fonts the table holds.</summary>
    public int Count => _names.Length;

    /// <summary>
    /// The family name at an index, or null when the table has no such entry.
    /// </summary>
    /// <remarks>
    /// Only the primary name. An entry may carry an alternate to fall back to, which is the document's
    /// own substitution suggestion — Paperless resolves substitutions through LibreOffice's table
    /// instead, so honouring this one would introduce a second and different answer.
    /// </remarks>
    public string? Name(int index)
        => index >= 0 && index < _names.Length ? _names[index] : null;

    /// <summary>
    /// How much of the header comes before the first entry.
    /// </summary>
    /// <remarks>
    /// Four bytes, not two. The first two are a count that duplicates what the FIB already states, and
    /// the two after them are the string table's extra-data length — which for this table is always zero
    /// and is easy to miss, since LibreOffice writes it as a bare <c>rSt.SeekRel(2)</c> after reading the
    /// count. Starting two bytes early reads the count's high half as an entry length and finds no fonts
    /// at all, which then shows up as every paragraph being laid out in a substituted face.
    /// </remarks>
    private const int HeaderLength = 4;

    /// <summary>
    /// Parses the table from the <c>SttbfFfn</c>'s bytes.
    /// </summary>
    /// <remarks>
    /// The declared count is skipped rather than trusted — a producer that disagrees with itself is
    /// common, and the walk is bounded by the bytes either way.
    /// </remarks>
    public static Ww8FontTable Parse(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length <= HeaderLength) return Empty;

        List<string> names = [];
        int at = HeaderLength;

        while (at < bytes.Length && names.Count < MaxFonts)
        {
            int payload = bytes[at];
            at++;

            if (payload < MinimumPayload || at + payload > bytes.Length) break;

            names.Add(NameIn(bytes.Slice(at, payload)));
            at += payload;
        }

        return names.Count > 0 ? new Ww8FontTable([.. names]) : Empty;
    }

    /// <summary>
    /// The name inside one entry's payload.
    /// </summary>
    /// <remarks>
    /// UTF-16, terminated by a null unit rather than by the payload's end — the payload is padded, so
    /// decoding all of it appends whatever the padding happens to be.
    /// </remarks>
    private static string NameIn(ReadOnlySpan<byte> payload)
    {
        ReadOnlySpan<byte> name = payload[NameOffset..];

        int units = 0;
        while ((units * 2) + 1 < name.Length
               && BinaryPrimitives.ReadUInt16LittleEndian(name[(units * 2)..]) != 0)
        {
            units++;
        }

        return units == 0 ? string.Empty : Encoding.Unicode.GetString(name[..(units * 2)]);
    }
}

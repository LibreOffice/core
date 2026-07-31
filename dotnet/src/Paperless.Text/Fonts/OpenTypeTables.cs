using System.Buffers.Binary;
using System.Text;

namespace Paperless.Text.Fonts;

/// <summary>
/// The sfnt table directory: which tables a font file holds and where each one is.
/// </summary>
/// <remarks>
/// <para>
/// Hand-rolled rather than taken from a library, deliberately. Deriving a line height needs raw
/// access to <c>hhea</c>, <c>OS/2</c> and a specific bit of <c>fsSelection</c>, and needs to apply
/// Paperless's own precedence between them rather than whatever a library decided — and that
/// precedence is the single most visible way two renderers diverge on identical input
/// (<c>research/06-rendering.md</c> section B.4).
/// </para>
/// <para>
/// A font file may be a collection: one file holding several faces that share their outline data. So
/// a face is addressed by file <em>and</em> index, and a reader that assumes one face per file reads
/// the first face of every CJK font on the system, which is rarely the one asked for.
/// </para>
/// </remarks>
public sealed class SfntFile
{
    /// <summary>The tag of a TrueType collection, which holds several faces.</summary>
    private const uint CollectionTag = 0x74746366;   // 'ttcf'

    /// <summary>The version tag of an OpenType font with CFF outlines.</summary>
    private const uint OpenTypeTag = 0x4F54544F;     // 'OTTO'

    /// <summary>The version tag of a TrueType font with glyf outlines.</summary>
    private const uint TrueTypeTag = 0x00010000;

    /// <summary>Apple's alternative TrueType tag, which some older fonts still use.</summary>
    private const uint AppleTrueTypeTag = 0x74727565;   // 'true'

    private readonly byte[] _data;
    private readonly Dictionary<uint, (int Offset, int Length)> _tables = [];

    private SfntFile(byte[] data) => _data = data;

    /// <summary>How many faces the file holds.</summary>
    public int FaceCount { get; private set; } = 1;

    /// <summary>Which face of the file this instance reads.</summary>
    public int FaceIndex { get; private set; }

    /// <summary>
    /// The whole font file, not just this face's tables.
    /// </summary>
    /// <remarks>
    /// Exposed because a shaper needs the file and a face index rather than a table directory: HarfBuzz
    /// parses the sfnt itself and reads tables this reader deliberately does not, <c>GSUB</c> and
    /// <c>GPOS</c> among them. Handing over the bytes is what keeps the two agreeing about which face
    /// they are looking at.
    /// </remarks>
    public ReadOnlyMemory<byte> Bytes => _data;

    /// <summary>The tags of the tables the face holds.</summary>
    public IReadOnlyCollection<uint> TableTags => _tables.Keys;

    /// <summary>
    /// Reads a face's table directory.
    /// </summary>
    /// <param name="data">The whole font file.</param>
    /// <param name="faceIndex">Which face of a collection to read; ignored for a single font.</param>
    /// <returns>The directory, or null when the bytes are not a font this reader understands.</returns>
    public static SfntFile? Open(byte[] data, int faceIndex = 0)
    {
        ArgumentNullException.ThrowIfNull(data);
        if (data.Length < 12) return null;

        SfntFile file = new(data) { FaceIndex = faceIndex };
        int directoryAt = 0;

        uint tag = BinaryPrimitives.ReadUInt32BigEndian(data);
        if (tag == CollectionTag)
        {
            if (data.Length < 16) return null;

            int count = (int)BinaryPrimitives.ReadUInt32BigEndian(data.AsSpan(8));
            if (count <= 0 || 12 + (4 * count) > data.Length) return null;

            file.FaceCount = count;
            if (faceIndex < 0 || faceIndex >= count) return null;

            directoryAt = (int)BinaryPrimitives.ReadUInt32BigEndian(
                data.AsSpan(12 + (4 * faceIndex)));
            if (directoryAt < 0 || directoryAt + 12 > data.Length) return null;

            tag = BinaryPrimitives.ReadUInt32BigEndian(data.AsSpan(directoryAt));
        }

        if (tag is not (OpenTypeTag or TrueTypeTag or AppleTrueTypeTag)) return null;

        int numTables = BinaryPrimitives.ReadUInt16BigEndian(data.AsSpan(directoryAt + 4));
        int entriesAt = directoryAt + 12;
        if (numTables <= 0 || entriesAt + (16 * numTables) > data.Length) return null;

        for (int i = 0; i < numTables; i++)
        {
            int entry = entriesAt + (16 * i);
            uint tableTag = BinaryPrimitives.ReadUInt32BigEndian(data.AsSpan(entry));
            long offset = BinaryPrimitives.ReadUInt32BigEndian(data.AsSpan(entry + 8));
            long length = BinaryPrimitives.ReadUInt32BigEndian(data.AsSpan(entry + 12));

            // A table that runs past the end of the file is skipped rather than fatal: a font with a
            // damaged optional table is still worth reading for the tables that are intact.
            if (offset < 0 || length < 0 || offset + length > data.Length) continue;

            file._tables[tableTag] = ((int)offset, (int)length);
        }

        return file._tables.Count > 0 ? file : null;
    }

    /// <summary>A table's bytes, or empty when the face has no such table.</summary>
    public ReadOnlySpan<byte> Table(string tag)
    {
        ArgumentNullException.ThrowIfNull(tag);
        return Table(TagOf(tag));
    }

    /// <summary>A table's bytes by numeric tag, or empty when the face has no such table.</summary>
    public ReadOnlySpan<byte> Table(uint tag)
        => _tables.TryGetValue(tag, out (int Offset, int Length) at)
            ? _data.AsSpan(at.Offset, at.Length)
            : default;

    /// <summary>True when the face holds a table.</summary>
    public bool Has(string tag) => _tables.ContainsKey(TagOf(tag));

    /// <summary>
    /// The four-byte tag a name spells, big-endian, as the format stores it.
    /// </summary>
    /// <remarks>
    /// A tag shorter than four characters is padded with spaces, because that is how the format
    /// writes them — <c>OS/2</c> is four characters but <c>cvt</c> is three and is stored as
    /// <c>"cvt "</c>.
    /// </remarks>
    public static uint TagOf(string tag)
    {
        ArgumentNullException.ThrowIfNull(tag);

        uint value = 0;
        for (int i = 0; i < 4; i++)
        {
            value = (value << 8) | (uint)(i < tag.Length ? tag[i] : ' ');
        }
        return value;
    }
}

/// <summary>
/// The <c>head</c> table: the font's design grid and a few global flags.
/// </summary>
/// <param name="UnitsPerEm">
/// The design grid every other metric is expressed in. Not a fixed number: 1000 for most CFF fonts
/// and 2048 for most TrueType ones, so a reader that assumes either scales half the fonts wrongly.
/// </param>
/// <param name="MacStyle">The bold and italic bits, which are the fallback when OS/2 is absent.</param>
/// <param name="IndexToLocFormat">Whether the glyph location table holds short or long offsets.</param>
public readonly record struct HeadTable(int UnitsPerEm, int MacStyle, int IndexToLocFormat)
{
    /// <summary>The units-per-em used when the table is missing or malformed.</summary>
    public const int DefaultUnitsPerEm = 1000;

    /// <summary>True when the font's own header says it is bold.</summary>
    public bool IsBold => (MacStyle & 0x0001) != 0;

    /// <summary>True when the font's own header says it is italic.</summary>
    public bool IsItalic => (MacStyle & 0x0002) != 0;

    /// <summary>Reads the table, or the defaults when it is absent.</summary>
    public static HeadTable Read(SfntFile file)
    {
        ArgumentNullException.ThrowIfNull(file);

        ReadOnlySpan<byte> head = file.Table("head");
        if (head.Length < 54) return new HeadTable(DefaultUnitsPerEm, 0, 0);

        int unitsPerEm = BinaryPrimitives.ReadUInt16BigEndian(head[18..]);

        // The format requires a power of two between 16 and 16384; anything else is a broken font,
        // and taking it at its word would scale every metric by a wrong factor.
        if (unitsPerEm is < 16 or > 16384) unitsPerEm = DefaultUnitsPerEm;

        return new HeadTable(
            unitsPerEm,
            BinaryPrimitives.ReadUInt16BigEndian(head[44..]),
            BinaryPrimitives.ReadInt16BigEndian(head[50..]));
    }
}

/// <summary>
/// The <c>hhea</c> table: horizontal metrics that apply to the whole face.
/// </summary>
/// <param name="Ascender">Distance above the baseline.</param>
/// <param name="Descender">Distance below the baseline, negative as the font stores it.</param>
/// <param name="LineGap">Recommended extra leading between lines.</param>
/// <param name="NumberOfHorizontalMetrics">
/// How many glyphs have their own advance width in <c>hmtx</c>; the rest repeat the last one.
/// </param>
public readonly record struct HorizontalHeaderTable(
    int Ascender,
    int Descender,
    int LineGap,
    int NumberOfHorizontalMetrics)
{
    /// <summary>
    /// True when the metrics are usable.
    /// </summary>
    /// <remarks>
    /// A font whose ascent is negative or whose descent is positive has them the wrong way round.
    /// Real fonts do this, and believing them puts the baseline outside the line — so LibreOffice
    /// checks the signs before trusting the table, and so does this.
    /// </remarks>
    public bool IsPlausible => Ascender >= 0 && Descender <= 0;

    /// <summary>Reads the table, or zeroes when it is absent.</summary>
    public static HorizontalHeaderTable Read(SfntFile file)
    {
        ArgumentNullException.ThrowIfNull(file);

        ReadOnlySpan<byte> hhea = file.Table("hhea");
        if (hhea.Length < 36) return default;

        return new HorizontalHeaderTable(
            BinaryPrimitives.ReadInt16BigEndian(hhea[4..]),
            BinaryPrimitives.ReadInt16BigEndian(hhea[6..]),
            BinaryPrimitives.ReadInt16BigEndian(hhea[8..]),
            BinaryPrimitives.ReadUInt16BigEndian(hhea[34..]));
    }
}

/// <summary>
/// The <c>OS/2</c> table: the metrics Windows historically used, plus the font's own opinion of
/// which metrics to believe.
/// </summary>
/// <param name="Version">The table's version, which decides how many fields it has.</param>
/// <param name="TypoAscender">The typographic ascent the font recommends.</param>
/// <param name="TypoDescender">The typographic descent, negative as the font stores it.</param>
/// <param name="TypoLineGap">The typographic line gap.</param>
/// <param name="WindowsAscent">The clipping ascent, which GDI used as the line's top.</param>
/// <param name="WindowsDescent">The clipping descent, positive as the font stores it.</param>
/// <param name="FsSelection">
/// The style and metric flags. Bit 7 is <c>USE_TYPO_METRICS</c>, which is the font saying "believe my
/// typographic metrics, not the historically bloated Windows ones".
/// </param>
/// <param name="StrikeoutSize">The strikethrough stroke width.</param>
/// <param name="StrikeoutPosition">The strikethrough offset above the baseline.</param>
/// <param name="Weight">The weight on the 1-1000 scale.</param>
/// <param name="WidthClass">The width class, 1 to 9.</param>
/// <param name="CapHeight">The capital height, or zero when the version is too old to hold it.</param>
/// <param name="XHeight">The x-height, or zero when the version is too old to hold it.</param>
public readonly record struct Os2Table(
    int Version,
    int TypoAscender,
    int TypoDescender,
    int TypoLineGap,
    int WindowsAscent,
    int WindowsDescent,
    int FsSelection,
    int StrikeoutSize,
    int StrikeoutPosition,
    int Weight,
    int WidthClass,
    int CapHeight,
    int XHeight)
{
    /// <summary>The <c>fsSelection</c> bit meaning "use my typographic metrics".</summary>
    public const int UseTypoMetricsFlag = 1 << 7;

    /// <summary>True when the font asks for its typographic metrics to be used.</summary>
    public bool UseTypoMetrics => (FsSelection & UseTypoMetricsFlag) != 0;

    /// <summary>True when the table's own style bits say the font is italic.</summary>
    public bool IsItalic => (FsSelection & 0x0001) != 0;

    /// <summary>True when the table's own style bits say the font is bold.</summary>
    public bool IsBold => (FsSelection & 0x0020) != 0;

    /// <summary>Reads the table, or null when the face has none.</summary>
    /// <remarks>
    /// Absent only on old Mac-only fonts, since the table is mandatory on Windows — but "old
    /// Mac-only font" describes plenty of what is installed on a Linux machine, so the absence has
    /// to be handled rather than assumed away.
    /// </remarks>
    public static Os2Table? Read(SfntFile file)
    {
        ArgumentNullException.ThrowIfNull(file);

        ReadOnlySpan<byte> os2 = file.Table("OS/2");
        if (os2.Length < 78) return null;

        int version = BinaryPrimitives.ReadUInt16BigEndian(os2);

        return new Os2Table(
            version,
            TypoAscender: BinaryPrimitives.ReadInt16BigEndian(os2[68..]),
            TypoDescender: BinaryPrimitives.ReadInt16BigEndian(os2[70..]),
            TypoLineGap: BinaryPrimitives.ReadInt16BigEndian(os2[72..]),
            WindowsAscent: BinaryPrimitives.ReadUInt16BigEndian(os2[74..]),
            WindowsDescent: BinaryPrimitives.ReadUInt16BigEndian(os2[76..]),
            FsSelection: BinaryPrimitives.ReadUInt16BigEndian(os2[62..]),
            StrikeoutSize: BinaryPrimitives.ReadInt16BigEndian(os2[26..]),
            StrikeoutPosition: BinaryPrimitives.ReadInt16BigEndian(os2[28..]),
            Weight: BinaryPrimitives.ReadUInt16BigEndian(os2[4..]),
            WidthClass: BinaryPrimitives.ReadUInt16BigEndian(os2[6..]),
            // Version 2 added these. Reading them from an earlier table would take whatever bytes
            // happen to follow it.
            CapHeight: version >= 2 && os2.Length >= 90 ? BinaryPrimitives.ReadInt16BigEndian(os2[88..]) : 0,
            XHeight: version >= 2 && os2.Length >= 88 ? BinaryPrimitives.ReadInt16BigEndian(os2[86..]) : 0);
    }
}

/// <summary>
/// The <c>post</c> table: the underline metrics and whether the font is monospaced.
/// </summary>
/// <param name="UnderlinePosition">
/// The underline's offset from the baseline, negative below it as the font stores it.
/// </param>
/// <param name="UnderlineThickness">The underline stroke width.</param>
/// <param name="ItalicAngle">The italic angle in degrees, negative for a forward slant.</param>
/// <param name="IsFixedPitch">True when every glyph has the same advance width.</param>
public readonly record struct PostTable(
    int UnderlinePosition,
    int UnderlineThickness,
    double ItalicAngle,
    bool IsFixedPitch)
{
    /// <summary>Reads the table, or zeroes when it is absent.</summary>
    public static PostTable Read(SfntFile file)
    {
        ArgumentNullException.ThrowIfNull(file);

        ReadOnlySpan<byte> post = file.Table("post");
        if (post.Length < 32) return default;

        // italicAngle is a 16.16 fixed-point number, not an integer.
        int fixedAngle = BinaryPrimitives.ReadInt32BigEndian(post[4..]);

        return new PostTable(
            BinaryPrimitives.ReadInt16BigEndian(post[8..]),
            BinaryPrimitives.ReadInt16BigEndian(post[10..]),
            fixedAngle / 65536.0,
            BinaryPrimitives.ReadUInt32BigEndian(post[12..]) != 0);
    }
}

/// <summary>
/// The <c>name</c> table: the strings a font calls itself by.
/// </summary>
/// <remarks>
/// A font carries several names for several audiences, in several encodings, sometimes in several
/// languages. What matters for matching a document's font request is the family name a Windows
/// application would see, because that is the name a document was authored against — so the Windows
/// Unicode records are preferred and the Macintosh ones are the fallback.
/// </remarks>
public static class NameTable
{
    /// <summary>The name identifier of the font family.</summary>
    public const int FamilyId = 1;

    /// <summary>The name identifier of the subfamily, such as "Bold Italic".</summary>
    public const int SubfamilyId = 2;

    /// <summary>The name identifier of the full name.</summary>
    public const int FullNameId = 4;

    /// <summary>The name identifier of the PostScript name.</summary>
    public const int PostScriptNameId = 6;

    /// <summary>
    /// The typographic family, which is the family a font with more than four styles belongs to.
    /// </summary>
    /// <remarks>
    /// A family with light, semibold and black weights cannot express them through the four Windows
    /// style slots, so it splits itself into several families for old applications and names the real
    /// one here. Ignoring this makes "Source Sans 3 Light" a family of its own rather than a weight.
    /// </remarks>
    public const int TypographicFamilyId = 16;

    /// <summary>The typographic subfamily, paired with <see cref="TypographicFamilyId"/>.</summary>
    public const int TypographicSubfamilyId = 17;

    /// <summary>The name with an identifier, or null when the face carries none.</summary>
    public static string? Read(SfntFile file, int nameId)
    {
        ArgumentNullException.ThrowIfNull(file);

        ReadOnlySpan<byte> name = file.Table("name");
        if (name.Length < 6) return null;

        int count = BinaryPrimitives.ReadUInt16BigEndian(name[2..]);
        int stringsAt = BinaryPrimitives.ReadUInt16BigEndian(name[4..]);
        if (count <= 0 || 6 + (12 * count) > name.Length) return null;

        string? best = null;
        int bestScore = -1;

        for (int i = 0; i < count; i++)
        {
            ReadOnlySpan<byte> record = name[(6 + (12 * i))..];
            int platform = BinaryPrimitives.ReadUInt16BigEndian(record);
            int encoding = BinaryPrimitives.ReadUInt16BigEndian(record[2..]);
            int language = BinaryPrimitives.ReadUInt16BigEndian(record[4..]);
            int id = BinaryPrimitives.ReadUInt16BigEndian(record[6..]);
            int length = BinaryPrimitives.ReadUInt16BigEndian(record[8..]);
            int offset = BinaryPrimitives.ReadUInt16BigEndian(record[10..]);

            if (id != nameId) continue;

            int at = stringsAt + offset;
            if (at < 0 || length < 0 || at + length > name.Length) continue;

            // Windows Unicode English first, then any Windows Unicode, then Macintosh Roman. A
            // localised name would match a document authored in that language and not one authored
            // elsewhere, so the English name is the portable choice.
            int score = (platform, encoding, language) switch
            {
                (3, 1 or 10, 0x0409) => 4,
                (3, 1 or 10, _) => 3,
                (0, _, _) => 2,
                (1, 0, _) => 1,
                _ => 0,
            };
            if (score <= bestScore) continue;

            best = platform switch
            {
                // Windows and the Unicode platform store UTF-16 big-endian.
                3 or 0 => Encoding.BigEndianUnicode.GetString(name.Slice(at, length)),
                _ => Encoding.Latin1.GetString(name.Slice(at, length)),
            };
            bestScore = score;
        }

        return string.IsNullOrWhiteSpace(best) ? null : best.Trim();
    }
}

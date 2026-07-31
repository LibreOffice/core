using System.Buffers.Binary;

namespace Paperless.Text.Fonts;

/// <summary>
/// The <c>cmap</c> table: which glyph draws which character.
/// </summary>
/// <remarks>
/// <para>
/// Several subtables, for several platforms, and a face may hold any combination. What matters is
/// finding one that maps Unicode: format 4 covers the basic plane and format 12 covers everything, so
/// a font with emoji or rare CJK needs the latter and a font without it is fine with the former.
/// Preferring format 4 when both exist would silently lose every character above U+FFFF.
/// </para>
/// <para>
/// Both formats are read fully into a dictionary rather than searched in place. A face's coverage is
/// asked about constantly — every fallback decision is a coverage question — and the table is a few
/// thousand entries, so the memory is cheaper than repeatedly bisecting a big-endian structure.
/// </para>
/// </remarks>
public sealed class CharacterMap
{
    private readonly Dictionary<int, ushort> _glyphs;

    private CharacterMap(Dictionary<int, ushort> glyphs) => _glyphs = glyphs;

    /// <summary>How many characters the face maps.</summary>
    public int Count => _glyphs.Count;

    /// <summary>An empty map, for a face whose <c>cmap</c> is absent or unreadable.</summary>
    public static CharacterMap Empty { get; } = new([]);

    /// <summary>The glyph a character maps to, or zero when the face has none for it.</summary>
    /// <remarks>
    /// Glyph zero is the "missing glyph" by definition, so zero doubles as "not covered" without
    /// needing a nullable — which is what the format itself does.
    /// </remarks>
    public ushort GlyphFor(int codePoint)
        => _glyphs.TryGetValue(codePoint, out ushort glyph) ? glyph : (ushort)0;

    /// <summary>True when the face has a glyph for a character.</summary>
    public bool Covers(int codePoint) => GlyphFor(codePoint) != 0;

    /// <summary>Reads the best Unicode subtable the face offers.</summary>
    public static CharacterMap Read(SfntFile file)
    {
        ArgumentNullException.ThrowIfNull(file);

        ReadOnlySpan<byte> cmap = file.Table("cmap");
        if (cmap.Length < 4) return Empty;

        int count = BinaryPrimitives.ReadUInt16BigEndian(cmap[2..]);
        if (count <= 0 || 4 + (8 * count) > cmap.Length) return Empty;

        int bestOffset = -1;
        int bestScore = -1;

        for (int i = 0; i < count; i++)
        {
            ReadOnlySpan<byte> record = cmap[(4 + (8 * i))..];
            int platform = BinaryPrimitives.ReadUInt16BigEndian(record);
            int encoding = BinaryPrimitives.ReadUInt16BigEndian(record[2..]);
            long offset = BinaryPrimitives.ReadUInt32BigEndian(record[4..]);

            if (offset < 0 || offset + 4 > cmap.Length) continue;

            int format = BinaryPrimitives.ReadUInt16BigEndian(cmap[(int)offset..]);
            if (format is not (4 or 12 or 6 or 0)) continue;

            // A full-repertoire subtable beats a basic-plane one, whatever platform declares it: the
            // difference is whether characters above U+FFFF exist at all.
            int score = (platform, encoding, format) switch
            {
                (3, 10, 12) => 6,
                (0, _, 12) => 5,
                (3, 1, 4) => 4,
                (0, _, 4) => 3,
                (3, 0, _) => 2,
                _ => 1,
            };
            if (score <= bestScore) continue;

            bestScore = score;
            bestOffset = (int)offset;
        }

        if (bestOffset < 0) return Empty;

        Dictionary<int, ushort> glyphs = [];
        ReadOnlySpan<byte> subtable = cmap[bestOffset..];

        switch (BinaryPrimitives.ReadUInt16BigEndian(subtable))
        {
            case 0: ReadFormat0(subtable, glyphs); break;
            case 4: ReadFormat4(subtable, glyphs); break;
            case 6: ReadFormat6(subtable, glyphs); break;
            case 12: ReadFormat12(subtable, glyphs); break;
        }

        return glyphs.Count > 0 ? new CharacterMap(glyphs) : Empty;
    }

    /// <summary>Format 0: a byte-indexed table of 256 glyphs, from the earliest Macintosh fonts.</summary>
    private static void ReadFormat0(ReadOnlySpan<byte> table, Dictionary<int, ushort> glyphs)
    {
        if (table.Length < 262) return;
        for (int character = 0; character < 256; character++)
        {
            byte glyph = table[6 + character];
            if (glyph != 0) glyphs[character] = glyph;
        }
    }

    /// <summary>
    /// Format 4: segments of the basic plane, each mapping a range through a delta or an array.
    /// </summary>
    /// <remarks>
    /// The delta arithmetic is modulo 65536 and the format relies on it — a segment maps its
    /// characters by adding a value that is expected to wrap. Doing the arithmetic in 32 bits without
    /// masking produces glyph indexes above the font's glyph count for every segment that wraps.
    /// </remarks>
    private static void ReadFormat4(ReadOnlySpan<byte> table, Dictionary<int, ushort> glyphs)
    {
        if (table.Length < 14) return;

        int segmentsDoubled = BinaryPrimitives.ReadUInt16BigEndian(table[6..]);
        int segments = segmentsDoubled / 2;
        if (segments <= 0) return;

        int endsAt = 14;
        int startsAt = endsAt + segmentsDoubled + 2;
        int deltasAt = startsAt + segmentsDoubled;
        int rangesAt = deltasAt + segmentsDoubled;
        if (rangesAt + segmentsDoubled > table.Length) return;

        for (int segment = 0; segment < segments; segment++)
        {
            int end = BinaryPrimitives.ReadUInt16BigEndian(table[(endsAt + (2 * segment))..]);
            int start = BinaryPrimitives.ReadUInt16BigEndian(table[(startsAt + (2 * segment))..]);
            int delta = BinaryPrimitives.ReadInt16BigEndian(table[(deltasAt + (2 * segment))..]);
            int rangeOffset = BinaryPrimitives.ReadUInt16BigEndian(table[(rangesAt + (2 * segment))..]);

            if (start > end) continue;

            for (int character = start; character <= end && character <= 0xFFFF; character++)
            {
                ushort glyph;
                if (rangeOffset == 0)
                {
                    glyph = (ushort)((character + delta) & 0xFFFF);
                }
                else
                {
                    int at = rangesAt + (2 * segment) + rangeOffset + (2 * (character - start));
                    if (at + 2 > table.Length) continue;

                    glyph = BinaryPrimitives.ReadUInt16BigEndian(table[at..]);
                    if (glyph != 0) glyph = (ushort)((glyph + delta) & 0xFFFF);
                }

                if (glyph != 0) glyphs[character] = glyph;
            }
        }
    }

    /// <summary>Format 6: one contiguous range of characters.</summary>
    private static void ReadFormat6(ReadOnlySpan<byte> table, Dictionary<int, ushort> glyphs)
    {
        if (table.Length < 10) return;

        int first = BinaryPrimitives.ReadUInt16BigEndian(table[6..]);
        int count = BinaryPrimitives.ReadUInt16BigEndian(table[8..]);
        if (10 + (2 * count) > table.Length) return;

        for (int i = 0; i < count; i++)
        {
            ushort glyph = BinaryPrimitives.ReadUInt16BigEndian(table[(10 + (2 * i))..]);
            if (glyph != 0) glyphs[first + i] = glyph;
        }
    }

    /// <summary>Format 12: groups covering the whole of Unicode, which is what emoji need.</summary>
    private static void ReadFormat12(ReadOnlySpan<byte> table, Dictionary<int, ushort> glyphs)
    {
        if (table.Length < 16) return;

        long groups = BinaryPrimitives.ReadUInt32BigEndian(table[12..]);
        if (groups <= 0 || 16 + (12 * groups) > table.Length) return;

        for (int group = 0; group < groups; group++)
        {
            ReadOnlySpan<byte> entry = table[(16 + (12 * group))..];
            long start = BinaryPrimitives.ReadUInt32BigEndian(entry);
            long end = BinaryPrimitives.ReadUInt32BigEndian(entry[4..]);
            long firstGlyph = BinaryPrimitives.ReadUInt32BigEndian(entry[8..]);

            if (start > end || end > 0x10FFFF) continue;

            // A group may claim an enormous range; the cap keeps a malformed font from filling memory
            // while still covering anything a real one declares.
            if (end - start > 0x20000) end = start + 0x20000;

            for (long character = start; character <= end; character++)
            {
                long glyph = firstGlyph + (character - start);
                if (glyph is > 0 and <= 0xFFFF) glyphs[(int)character] = (ushort)glyph;
            }
        }
    }
}

/// <summary>
/// A font face read from an sfnt file: its metrics, its coverage and its advance widths.
/// </summary>
/// <remarks>
/// <para>
/// The advance widths are the reason this has to be exact. A line break is decided by where the text
/// runs past the margin, so an advance that is wrong by one design unit eventually moves a break, and
/// a moved break shifts everything after it — the whole page, and then every page after that. That is
/// why the metric-compatible substitutions matter and why this reads the font rather than guessing.
/// </para>
/// </remarks>
public sealed class OpenTypeFace
{
    private readonly ushort[] _advances;
    private readonly ushort _lastAdvance;

    private OpenTypeFace(
        SfntFile file,
        HeadTable head,
        HorizontalHeaderTable horizontal,
        Os2Table? os2,
        PostTable post,
        CharacterMap characters,
        ushort[] advances)
    {
        File = file;
        Head = head;
        Horizontal = horizontal;
        Os2 = os2;
        Post = post;
        Characters = characters;
        _advances = advances;
        _lastAdvance = advances.Length > 0 ? advances[^1] : (ushort)0;
    }

    /// <summary>The table directory this face was read from.</summary>
    public SfntFile File { get; }

    /// <summary>The <c>head</c> table.</summary>
    public HeadTable Head { get; }

    /// <summary>The <c>hhea</c> table.</summary>
    public HorizontalHeaderTable Horizontal { get; }

    /// <summary>The <c>OS/2</c> table, or null when the face has none.</summary>
    public Os2Table? Os2 { get; }

    /// <summary>The <c>post</c> table.</summary>
    public PostTable Post { get; }

    /// <summary>The face's character coverage.</summary>
    public CharacterMap Characters { get; }

    /// <summary>Design units per em; every metric here is in these units.</summary>
    public int UnitsPerEm => Head.UnitsPerEm;

    /// <summary>The family name the face calls itself.</summary>
    public string? FamilyName =>
        NameTable.Read(File, NameTable.TypographicFamilyId)
        ?? NameTable.Read(File, NameTable.FamilyId);

    /// <summary>The subfamily, such as "Bold Italic".</summary>
    public string? SubfamilyName =>
        NameTable.Read(File, NameTable.TypographicSubfamilyId)
        ?? NameTable.Read(File, NameTable.SubfamilyId);

    /// <summary>
    /// The weight on the OpenType 1-1000 scale, from <c>OS/2</c> or from the header's bold bit.
    /// </summary>
    public int Weight => Os2?.Weight is > 0 and <= 1000 ? Os2.Value.Weight : Head.IsBold ? 700 : 400;

    /// <summary>True when the face is italic or oblique.</summary>
    public bool IsItalic => Os2?.IsItalic ?? Head.IsItalic || Post.ItalicAngle != 0;

    /// <summary>True when every glyph has the same advance width.</summary>
    public bool IsFixedPitch => Post.IsFixedPitch;

    /// <summary>Reads a face, or null when the bytes are not a font this reader understands.</summary>
    /// <param name="data">The whole font file.</param>
    /// <param name="faceIndex">Which face of a collection to read.</param>
    public static OpenTypeFace? Read(byte[] data, int faceIndex = 0)
    {
        if (SfntFile.Open(data, faceIndex) is not { } file) return null;

        HeadTable head = HeadTable.Read(file);
        HorizontalHeaderTable horizontal = HorizontalHeaderTable.Read(file);

        return new OpenTypeFace(
            file,
            head,
            horizontal,
            Os2Table.Read(file),
            PostTable.Read(file),
            CharacterMap.Read(file),
            ReadAdvances(file, horizontal));
    }

    /// <summary>Reads a face from a file on disk, or null when it is not a font.</summary>
    public static OpenTypeFace? ReadFile(string path, int faceIndex = 0)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        return Read(System.IO.File.ReadAllBytes(path), faceIndex);
    }

    /// <summary>
    /// A glyph's advance width in design units.
    /// </summary>
    /// <remarks>
    /// The table stores an advance only for the glyphs whose widths differ; every glyph after the last
    /// one repeats it. That is not a corner case — a CJK font is mostly one width, so most of its
    /// glyphs are covered by the repeat, and a reader that returns zero past the end of the table
    /// measures a whole Japanese paragraph as empty.
    /// </remarks>
    public int AdvanceOf(ushort glyph)
        => glyph < _advances.Length ? _advances[glyph] : _lastAdvance;

    /// <summary>A character's advance width in design units, or zero when it is not covered.</summary>
    public int AdvanceForCharacter(int codePoint)
    {
        ushort glyph = Characters.GlyphFor(codePoint);
        return glyph == 0 ? 0 : AdvanceOf(glyph);
    }

    /// <summary>True when the face has a glyph for a character.</summary>
    public bool HasGlyphFor(int codePoint) => Characters.Covers(codePoint);

    private static ushort[] ReadAdvances(SfntFile file, HorizontalHeaderTable horizontal)
    {
        ReadOnlySpan<byte> hmtx = file.Table("hmtx");
        int count = horizontal.NumberOfHorizontalMetrics;
        if (hmtx.Length < 4 || count <= 0) return [];

        count = Math.Min(count, hmtx.Length / 4);
        ushort[] advances = new ushort[count];
        for (int i = 0; i < count; i++)
        {
            advances[i] = BinaryPrimitives.ReadUInt16BigEndian(hmtx[(4 * i)..]);
        }
        return advances;
    }
}

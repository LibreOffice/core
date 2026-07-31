using System.Buffers.Binary;

namespace Paperless.Rendering.Pdf;

/// <summary>
/// A font file taken apart into its tables, and put back together again.
/// </summary>
/// <remarks>
/// <para>
/// Small and deliberately separate from <c>Paperless.Text</c>'s OpenType reader, which
/// reads a font in order to measure it. This one exists to <em>rewrite</em> one: the PDF
/// writer takes what <c>hb-subset</c> produces and replaces its character map, which needs
/// a table directory that can be edited and re-serialised rather than one that can only be
/// queried.
/// </para>
/// <para>
/// Why the cmap has to be replaced at all is the whole of the reason this file exists.
/// A simple TrueType font in a PDF addresses its glyphs with one-byte codes, and the
/// mapping from code to glyph is the font's own cmap, not the PDF's. LibreOffice writes a
/// platform 1, encoding 0, format 0 table that is the identity — measured on its export of
/// <c>prose-odt.odt</c>, where codes 1 to 28 map to glyphs 1 to 28 — so a code <em>is</em>
/// a glyph index. hb-subset builds a Unicode cmap instead, which addresses nothing a PDF
/// asks for, so it is thrown away and the identity written in its place.
/// </para>
/// </remarks>
internal sealed class SfntTables
{
    private const uint CollectionTag = 0x74746366;   // 'ttcf'

    private SfntTables(uint version, List<(uint Tag, byte[] Data)> tables)
    {
        Version = version;
        Tables = tables;
    }

    /// <summary>The sfnt version tag: <c>0x00010000</c> for glyf outlines, <c>OTTO</c> for CFF.</summary>
    public uint Version { get; }

    /// <summary>The tables, in tag order.</summary>
    public List<(uint Tag, byte[] Data)> Tables { get; }

    /// <summary>Makes a tag from its four characters.</summary>
    public static uint Tag(string name)
    {
        ArgumentNullException.ThrowIfNull(name);
        return ((uint)name[0] << 24) | ((uint)name[1] << 16) | ((uint)name[2] << 8) | name[3];
    }

    /// <summary>Reads a face's tables, or null when the bytes are not a font.</summary>
    public static SfntTables? Read(ReadOnlySpan<byte> data, int faceIndex = 0)
    {
        if (data.Length < 12) return null;

        int directory = 0;
        if (BinaryPrimitives.ReadUInt32BigEndian(data) == CollectionTag)
        {
            if (data.Length < 16) return null;

            int count = (int)BinaryPrimitives.ReadUInt32BigEndian(data[8..]);
            if (faceIndex < 0 || faceIndex >= count || 12 + (4 * count) > data.Length) return null;

            directory = (int)BinaryPrimitives.ReadUInt32BigEndian(data[(12 + (4 * faceIndex))..]);
            if (directory + 12 > data.Length) return null;
        }

        uint version = BinaryPrimitives.ReadUInt32BigEndian(data[directory..]);
        int tableCount = BinaryPrimitives.ReadUInt16BigEndian(data[(directory + 4)..]);
        if (directory + 12 + (16 * tableCount) > data.Length) return null;

        List<(uint, byte[])> tables = new(tableCount);
        for (int i = 0; i < tableCount; i++)
        {
            int record = directory + 12 + (16 * i);
            uint tag = BinaryPrimitives.ReadUInt32BigEndian(data[record..]);
            int offset = (int)BinaryPrimitives.ReadUInt32BigEndian(data[(record + 8)..]);
            int length = (int)BinaryPrimitives.ReadUInt32BigEndian(data[(record + 12)..]);

            if (offset < 0 || length < 0 || (long)offset + length > data.Length) return null;

            tables.Add((tag, data.Slice(offset, length).ToArray()));
        }

        tables.Sort((a, b) => a.Item1.CompareTo(b.Item1));
        return new SfntTables(version, tables);
    }

    /// <summary>The bytes of one table, or null when the face does not have it.</summary>
    public byte[]? Table(string name)
    {
        uint tag = Tag(name);
        foreach ((uint candidate, byte[] data) in Tables)
        {
            if (candidate == tag) return data;
        }

        return null;
    }

    /// <summary>Replaces a table, adding it when it was absent.</summary>
    public void Replace(string name, byte[] data)
    {
        ArgumentNullException.ThrowIfNull(data);

        uint tag = Tag(name);
        for (int i = 0; i < Tables.Count; i++)
        {
            if (Tables[i].Tag != tag) continue;

            Tables[i] = (tag, data);
            return;
        }

        Tables.Add((tag, data));
        Tables.Sort((a, b) => a.Tag.CompareTo(b.Tag));
    }

    /// <summary>
    /// Serialises the tables back into a font file.
    /// </summary>
    /// <remarks>
    /// Each table is padded to a four-byte boundary and checksummed, and
    /// <c>head.checkSumAdjustment</c> is recomputed last from the whole file, because it is
    /// defined in terms of a file that already contains it. A font whose adjustment is
    /// stale still renders in every reader tested, but it fails validation, and a PDF that
    /// fails validation is a support question waiting to happen.
    /// </remarks>
    public byte[] Serialise()
    {
        int count = Tables.Count;
        int headerSize = 12 + (16 * count);
        int total = headerSize;
        foreach ((_, byte[] data) in Tables) total += (data.Length + 3) & ~3;

        byte[] file = new byte[total];
        BinaryPrimitives.WriteUInt32BigEndian(file, Version);
        BinaryPrimitives.WriteUInt16BigEndian(file.AsSpan(4), (ushort)count);

        // searchRange, entrySelector and rangeShift: a binary-search hint no modern reader uses and
        // every validator checks. Derived rather than guessed.
        int entrySelector = 0;
        while (1 << (entrySelector + 1) <= count) entrySelector++;
        int searchRange = 16 << entrySelector;
        BinaryPrimitives.WriteUInt16BigEndian(file.AsSpan(6), (ushort)searchRange);
        BinaryPrimitives.WriteUInt16BigEndian(file.AsSpan(8), (ushort)entrySelector);
        BinaryPrimitives.WriteUInt16BigEndian(file.AsSpan(10), (ushort)((count * 16) - searchRange));

        int at = headerSize;
        int headRecord = -1;
        for (int i = 0; i < count; i++)
        {
            (uint tag, byte[] data) = Tables[i];
            int record = 12 + (16 * i);

            BinaryPrimitives.WriteUInt32BigEndian(file.AsSpan(record), tag);
            BinaryPrimitives.WriteUInt32BigEndian(file.AsSpan(record + 4), Checksum(data));
            BinaryPrimitives.WriteUInt32BigEndian(file.AsSpan(record + 8), (uint)at);
            BinaryPrimitives.WriteUInt32BigEndian(file.AsSpan(record + 12), (uint)data.Length);

            data.CopyTo(file.AsSpan(at));
            at += (data.Length + 3) & ~3;

            if (tag == Tag("head")) headRecord = i;
        }

        if (headRecord >= 0)
        {
            int headOffset = (int)BinaryPrimitives.ReadUInt32BigEndian(
                file.AsSpan(12 + (16 * headRecord) + 8));

            if (headOffset + 12 <= file.Length)
            {
                BinaryPrimitives.WriteUInt32BigEndian(file.AsSpan(headOffset + 8), 0);
                uint whole = Checksum(file);
                BinaryPrimitives.WriteUInt32BigEndian(
                    file.AsSpan(headOffset + 8), unchecked(0xB1B0AFBAu - whole));
            }
        }

        return file;
    }

    /// <summary>
    /// A platform 1, encoding 0, format 0 character map that is the identity.
    /// </summary>
    /// <remarks>
    /// Exactly what LibreOffice writes into the fonts it embeds, and the reason a
    /// one-byte code in a content stream selects the glyph with that index.
    /// </remarks>
    public static byte[] IdentityCharacterMap(int highestCode)
    {
        byte[] table = new byte[4 + 8 + 262];

        BinaryPrimitives.WriteUInt16BigEndian(table.AsSpan(0), 0);      // version
        BinaryPrimitives.WriteUInt16BigEndian(table.AsSpan(2), 1);      // one subtable
        BinaryPrimitives.WriteUInt16BigEndian(table.AsSpan(4), 1);      // platform: Macintosh
        BinaryPrimitives.WriteUInt16BigEndian(table.AsSpan(6), 0);      // encoding: Roman
        BinaryPrimitives.WriteUInt32BigEndian(table.AsSpan(8), 12);     // offset to the subtable

        BinaryPrimitives.WriteUInt16BigEndian(table.AsSpan(12), 0);     // format 0
        BinaryPrimitives.WriteUInt16BigEndian(table.AsSpan(14), 262);   // its length
        BinaryPrimitives.WriteUInt16BigEndian(table.AsSpan(16), 0);     // language

        for (int code = 0; code <= Math.Min(highestCode, 255); code++) table[18 + code] = (byte)code;

        return table;
    }

    private static uint Checksum(ReadOnlySpan<byte> data)
    {
        uint sum = 0;
        int whole = data.Length & ~3;

        for (int i = 0; i < whole; i += 4)
        {
            sum = unchecked(sum + BinaryPrimitives.ReadUInt32BigEndian(data[i..]));
        }

        if (whole < data.Length)
        {
            uint tail = 0;
            for (int i = whole; i < data.Length; i++) tail |= (uint)data[i] << (24 - (8 * (i - whole)));
            sum = unchecked(sum + tail);
        }

        return sum;
    }
}

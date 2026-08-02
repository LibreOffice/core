using System.Text;
using Paperless.MsBinary.Records;

namespace Paperless.MsBinary.Escher;

/// <summary>
/// A shape's <c>msofbtOPT</c> property table: the sparse bag holding nearly everything about
/// how a shape looks.
/// </summary>
/// <remarks>
/// <para>
/// The table has no fixed fields. It is a run of six-byte entries — a sixteen-bit identifier
/// with two flag bits, then a thirty-two-bit value — and the record's *instance* rather than
/// its length states how many there are (<c>DffPropSet::ReadPropSet</c>,
/// <c>filter/source/msfilter/dffpropset.cxx</c>). A property whose complex bit is set stores
/// its value's *length* in those four bytes, and the value itself in a block that begins
/// where the fixed entries end, in property order.
/// </para>
/// <para>
/// The trap here is that the complex block's position is only derivable from the entry count:
/// it starts exactly <c>6 × count</c> bytes into the payload, and the entries do not record
/// where their own data is. Trust the length field over the count and every complex value
/// afterwards is offset by six bytes per miscount, which yields plausible-looking strings
/// rather than an error.
/// </para>
/// </remarks>
public sealed class EscherPropertyTable
{
    private readonly Dictionary<ushort, Entry> _entries;

    private EscherPropertyTable(Dictionary<ushort, Entry> entries) => _entries = entries;

    /// <summary>A table with nothing in it, for a shape carrying no property record.</summary>
    public static EscherPropertyTable Empty { get; } = new([]);

    /// <summary>How many properties the table holds.</summary>
    public int Count => _entries.Count;

    /// <summary>
    /// Reads a property table from an <c>msofbtOPT</c> record.
    /// </summary>
    /// <param name="content">The record's payload.</param>
    /// <param name="propertyCount">The record header's instance, which states the entry count.</param>
    public static EscherPropertyTable Read(ReadOnlySpan<byte> content, int propertyCount)
    {
        // Each entry is six bytes, so a count that could not fit is a corrupt header rather
        // than a very large table; clamping keeps the complex block's start honest.
        int count = Math.Clamp(propertyCount, 0, content.Length / 6);
        if (count == 0) return Empty;

        Dictionary<ushort, Entry> entries = new(count);
        int complex = count * 6;

        for (int i = 0; i < count; i++)
        {
            ReadOnlySpan<byte> entry = content[(i * 6)..];
            ushort raw = DffRecordBuffer.ReadUInt16(entry);
            uint value = DffRecordBuffer.ReadUInt32(entry[2..]);

            ushort id = (ushort)(raw & 0x3FFF);
            bool isBlip = (raw & 0x4000) != 0;
            bool isComplex = (raw & 0x8000) != 0;

            int dataStart = 0, dataLength = 0;
            if (isComplex && value > 0)
            {
                dataStart = complex;
                dataLength = (int)Math.Min(
                    ArrayLength(content, complex, id, value),
                    (uint)Math.Max(content.Length - complex, 0));
                complex += dataLength;
            }

            // A later entry for the same id is a writer's mistake; the first one wins, which
            // is what the C++ set does by writing into a fixed-size array in order.
            entries.TryAdd(id, new Entry(value, isBlip, isComplex, dataStart, dataLength));
        }

        // The complex block is only reachable while the payload span is alive, so it is copied
        // out. Complex values are names and vertex arrays — small next to the table itself.
        byte[] copy = content.ToArray();
        foreach (KeyValuePair<ushort, Entry> pair in entries)
        {
            if (pair.Value.DataLength > 0) entries[pair.Key] = pair.Value with { Data = copy };
        }

        return new EscherPropertyTable(entries);
    }

    /// <summary>
    /// The nine properties whose complex value is an <c>IMsoArray</c> rather than a blob.
    /// </summary>
    /// <remarks>
    /// They matter because their stated length lies. See <see cref="ArrayLength"/>.
    /// </remarks>
    private static bool IsArray(ushort id) => id is 325   // pVertices
        or 326   // pSegmentInfo
        or 337   // connectorPoints
        or 341   // Handles
        or 342   // pFormulas
        or 343   // textRectangles
        or 407   // fillShadeColors
        or 463   // lineDashStyle
        or 899;  // pWrapPolygonVertices

    /// <summary>
    /// How many bytes a complex property really occupies.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The stated value, except for the nine array properties, where <strong>a writer may leave
    /// the array's own six-byte header out of the count</strong>. The header is three words —
    /// element count, allocated count, element size — and when the stated length happens to equal
    /// <c>count × size</c> exactly, the six bytes are there in the data and missing from the
    /// number (<c>DffPropSet::ReadPropSet</c>, <c>filter/source/msfilter/dffpropset.cxx:1166-1204</c>,
    /// whose comment on the subject is "I love special treatments").
    /// </para>
    /// <para>
    /// <strong>The failure is silent and it is not local.</strong> Complex values sit in one block
    /// in property order, so a shape whose vertex array is under-counted by six shifts every
    /// complex property after it — and a shape name read six bytes early comes back as plausible
    /// text rather than as an error. LibreOffice's own PPT export writes a vertex array on every
    /// shape it emits, so this is the common case in that format rather than a corner of it.
    /// </para>
    /// </remarks>
    private static uint ArrayLength(ReadOnlySpan<byte> content, int at, ushort id, uint stated)
    {
        if (!IsArray(id) || at < 0 || at + 6 > content.Length) return stated;

        short count = unchecked((short)DffRecordBuffer.ReadUInt16(content[at..]));
        short allocated = unchecked((short)DffRecordBuffer.ReadUInt16(content[(at + 2)..]));
        short size = unchecked((short)DffRecordBuffer.ReadUInt16(content[(at + 4)..]));

        if (allocated < count || count <= 0) return stated;

        // A negative element size is a quarter of its magnitude; -16 means four bytes, which is
        // how a writer says "eight-byte elements, low four bytes only".
        if (size < 0) size = (short)(-size >> 2);

        return (uint)(size * count) == stated ? stated + 6 : stated;
    }

    /// <summary>Whether the shape states this property itself.</summary>
    public bool Has(ushort id) => _entries.ContainsKey(id);

    /// <summary>
    /// This table with another laid underneath it, as a shape naming an <c>hspMaster</c> takes
    /// the properties of the shape it names.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>DffPropertyReader::ReadPropSet</c> (<c>filter/source/msfilter/msdffimp.cxx:278-289</c>)
    /// reads a shape's own table and then, when it states <c>hspMaster</c>, seeks to that shape
    /// and merges its table in with <c>bSetUninitializedOnly</c> — the <c>|=</c> overload at
    /// <c>dffpropset.cxx:1245</c>. Without it a slide's placeholders inherit nothing: PowerPoint
    /// writes the fill, the line and the colours once on the master's placeholder and leaves the
    /// per-slide copy stating little beyond its anchor.
    /// </para>
    /// <para>
    /// <strong>An ordinary property is taken only where this table is silent</strong>, which is
    /// what the C++ <c>!IsProperty(nRecType) || !IsHardAttribute(nRecType)</c> reduces to for a
    /// table holding exactly what its shape stated.
    /// </para>
    /// <para>
    /// <strong>A boolean group merges bit by bit, not as a value.</strong> Each group entry packs
    /// thirty-two booleans as sixteen values and sixteen "was this stated" bits, so a shape
    /// stating one boolean of a group states the group — and taking the master's word for the
    /// whole entry would silently overwrite the fifteen the child did set. Per bit: the child's
    /// value where the child states it, the master's where only the master does, and otherwise
    /// the host's default, which is left to the caller because it is not in either table.
    /// </para>
    /// </remarks>
    /// <param name="master">The table to inherit from; typically the master shape's.</param>
    public EscherPropertyTable InheritFrom(EscherPropertyTable master)
    {
        ArgumentNullException.ThrowIfNull(master);
        if (master._entries.Count == 0) return this;

        Dictionary<ushort, Entry> merged = new(_entries);

        foreach (KeyValuePair<ushort, Entry> pair in master._entries)
        {
            if (IsBooleanGroup(pair.Key))
            {
                merged[pair.Key] = merged.TryGetValue(pair.Key, out Entry child)
                    ? child with { Value = MergeFlags(child.Value, pair.Value.Value) }
                    : pair.Value;
            }
            else
            {
                merged.TryAdd(pair.Key, pair.Value);
            }
        }

        return new EscherPropertyTable(merged);
    }

    /// <summary>
    /// Whether an identifier names a group of booleans rather than one value.
    /// </summary>
    /// <remarks>
    /// A group is written under its highest member's identifier, and those are the identifiers
    /// congruent to 63 modulo 64 — <c>( nRecType &amp; 0x3f ) == 0x3f</c> in
    /// <c>DffPropSet::ReadPropSet</c>. 447 is the fill group, 511 the line group, 191 the text
    /// group.
    /// </remarks>
    private static bool IsBooleanGroup(ushort id) => (id & 0x3F) == 0x3F;

    /// <summary>
    /// One boolean group's entry, with the master's stated bits filled in under the child's.
    /// </summary>
    /// <remarks>
    /// The C++ does this in four masked steps; they reduce to taking, for every bit the master
    /// states and the child does not, the master's value, and accumulating the stated bits of
    /// both. The high half is that stated set — <c>nComplexIndexOrFlagsHAttr |= nContent >> 16</c>
    /// — and is what <see cref="StatesBoolean"/> reads.
    /// </remarks>
    private static uint MergeFlags(uint child, uint master)
    {
        uint childStated = child >> 16;
        uint masterStated = master >> 16;
        uint taken = masterStated & ~childStated;

        uint values = (child & 0xFFFF & ~taken) | (master & 0xFFFF & taken);
        return ((childStated | masterStated) << 16) | values;
    }

    /// <summary>The property's value, or the fallback when the shape does not state it.</summary>
    public uint Value(ushort id, uint fallback = 0)
        => _entries.TryGetValue(id, out Entry entry) ? entry.Value : fallback;

    /// <summary>The property's value read as a signed integer.</summary>
    public int SignedValue(ushort id, int fallback = 0)
        => _entries.TryGetValue(id, out Entry entry) ? unchecked((int)entry.Value) : fallback;

    /// <summary>The property's value read as a boolean, as the format's boolean properties are.</summary>
    public bool Flag(ushort id, bool fallback = false)
        => _entries.TryGetValue(id, out Entry entry) ? entry.Value != 0 : fallback;

    /// <summary>
    /// A boolean property, which the format does not store under its own identifier.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Each group of boolean properties is written as <em>one</em> thirty-two-bit entry under the
    /// group's highest identifier — <c>id | 31</c> — with each boolean occupying the bit
    /// <c>1 &lt;&lt; (base - id)</c> of its low half and a "was this stated" bit sixteen places
    /// higher. So <c>fLine</c> (508) is bit 3 of property 511, and asking for property 508 directly
    /// finds nothing at all whatever the shape said. <c>DffPropSet::GetPropertyBool</c>,
    /// <c>filter/source/msfilter/dffpropset.cxx:1308</c>.
    /// </para>
    /// <para>
    /// The trap is that a table stating the group at all looks, to <see cref="Has"/>, like a table
    /// stating every boolean in it: property 511 is present as soon as one of its thirty-two
    /// members is. <see cref="StatesBoolean"/> is what tells them apart, and it matters because
    /// "stated false" and "said nothing" take different defaults in every host.
    /// </para>
    /// </remarks>
    /// <param name="id">The boolean property's own identifier, not its group's.</param>
    /// <param name="fallback">What to answer when the shape does not state it.</param>
    public bool Boolean(ushort id, bool fallback = false)
    {
        ushort group = (ushort)(id | 31);
        if (!_entries.TryGetValue(group, out Entry entry)) return fallback;
        return (entry.Value & (1u << (group - id))) != 0;
    }

    /// <summary>Whether the shape states this boolean property, as opposed to leaving it default.</summary>
    /// <inheritdoc cref="Boolean"/>
    public bool StatesBoolean(ushort id)
    {
        ushort group = (ushort)(id | 31);
        return _entries.TryGetValue(group, out Entry entry)
            && (entry.Value & (1u << (group - id + 16))) != 0;
    }

    /// <summary>
    /// Whether the property's value is an index into the blip store rather than a plain number.
    /// </summary>
    /// <remarks>
    /// The distinction is a bit on the entry rather than a property of the identifier, because
    /// the same property can hold either depending on how the writer stored the picture.
    /// </remarks>
    public bool IsBlip(ushort id) => _entries.TryGetValue(id, out Entry entry) && entry.IsBlip;

    /// <summary>
    /// The elements of an <c>IMsoArray</c> property, past its three-word header.
    /// </summary>
    /// <remarks>
    /// An element size of <c>0xFFF0</c> means eight-byte elements of which only the low four
    /// bytes were written — a compression the format applies to coordinate arrays that fit in
    /// sixteen bits, and the reason a vertex reader cannot assume the size it expects
    /// (<c>filter/source/msfilter/msdffimp.cxx:2216-2220</c>).
    /// </remarks>
    /// <param name="id">The property.</param>
    /// <param name="count">How many elements the header claims, clamped to what is present.</param>
    /// <param name="elementSize">How many bytes each occupies.</param>
    public ReadOnlySpan<byte> Array(ushort id, out int count, out int elementSize)
    {
        count = 0;
        elementSize = 0;

        ReadOnlySpan<byte> data = Data(id);
        if (data.Length < 6) return default;

        int stated = unchecked((short)DffRecordBuffer.ReadUInt16(data));
        int size = unchecked((short)DffRecordBuffer.ReadUInt16(data[4..]));

        if (size == unchecked((short)0xFFF0)) size = 4;
        else if (size < 0) size = -size >> 2;
        if (size <= 0 || stated <= 0) return default;

        ReadOnlySpan<byte> elements = data[6..];
        count = Math.Min(stated, elements.Length / size);
        elementSize = size;
        return elements;
    }

    /// <summary>A complex property's raw bytes, empty when it is absent or not complex.</summary>
    public ReadOnlySpan<byte> Data(ushort id)
        => _entries.TryGetValue(id, out Entry entry) && entry.Data is { } data
            ? data.AsSpan(entry.DataStart, entry.DataLength)
            : default;

    /// <summary>
    /// A complex property read as text, which the format stores as UTF-16 with a trailing NUL.
    /// </summary>
    /// <remarks>
    /// The terminator is part of the stored length, so it has to be trimmed; leaving it in
    /// produces strings that compare unequal to the same text read from anywhere else.
    /// </remarks>
    public string? Text(ushort id)
    {
        ReadOnlySpan<byte> data = Data(id);
        if (data.Length < 2) return null;

        string text = Encoding.Unicode.GetString(data[..(data.Length & ~1)]);
        int nul = text.IndexOf('\0', StringComparison.Ordinal);
        if (nul >= 0) text = text[..nul];
        return text.Length > 0 ? text : null;
    }

    private readonly record struct Entry(
        uint Value, bool IsBlip, bool IsComplex, int DataStart, int DataLength)
    {
        public byte[]? Data { get; init; }
    }
}

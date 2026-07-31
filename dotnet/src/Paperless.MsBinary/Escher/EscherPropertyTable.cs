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
                dataLength = (int)Math.Min(value, (uint)Math.Max(content.Length - complex, 0));
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

    /// <summary>Whether the shape states this property itself.</summary>
    public bool Has(ushort id) => _entries.ContainsKey(id);

    /// <summary>The property's value, or the fallback when the shape does not state it.</summary>
    public uint Value(ushort id, uint fallback = 0)
        => _entries.TryGetValue(id, out Entry entry) ? entry.Value : fallback;

    /// <summary>The property's value read as a signed integer.</summary>
    public int SignedValue(ushort id, int fallback = 0)
        => _entries.TryGetValue(id, out Entry entry) ? unchecked((int)entry.Value) : fallback;

    /// <summary>The property's value read as a boolean, as the format's boolean properties are.</summary>
    public bool Flag(ushort id, bool fallback = false)
        => _entries.TryGetValue(id, out Entry entry) ? entry.Value != 0 : fallback;

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

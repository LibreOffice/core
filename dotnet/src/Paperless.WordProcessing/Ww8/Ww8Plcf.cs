using System.Buffers.Binary;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// A PLCF: WW8's universal sparse array keyed by document position.
/// </summary>
/// <remarks>
/// <para>
/// Every "what applies at this position" table in a WW8 file is a PLCF — section descriptors,
/// formatting exceptions, fields, bookmarks, footnote and comment references. The layout is
/// always the same: <c>n + 1</c> ascending positions, then <c>n</c> fixed-size data records. The
/// extra position is the end of the last record's range, which is what makes the array a
/// partition of the document rather than a list of points.
/// </para>
/// <para>
/// Because the record size is fixed but not stored, <c>n</c> is derived from the total length —
/// so passing the wrong record size silently yields a different, plausible-looking number of
/// entries. That is the one way to misuse this type, and it is why the size is a required
/// argument rather than a default.
/// </para>
/// </remarks>
public sealed class Ww8Plcf
{
    private readonly int[] _positions;
    private readonly byte[] _data;
    private readonly int _recordSize;

    private Ww8Plcf(int[] positions, byte[] data, int recordSize)
    {
        _positions = positions;
        _data = data;
        _recordSize = recordSize;
    }

    /// <summary>How many records the table holds.</summary>
    public int Count => Math.Max(0, _positions.Length - 1);

    /// <summary>The positions, ascending, with one more than <see cref="Count"/>.</summary>
    public IReadOnlyList<int> Positions => _positions;

    /// <summary>
    /// Parses a PLCF.
    /// </summary>
    /// <param name="bytes">The table's bytes.</param>
    /// <param name="recordSize">
    /// The size of each data record, which is fixed by which table this is. Zero is legal and
    /// common: a PLCF of positions alone partitions the document without describing the parts.
    /// </param>
    public static Ww8Plcf Parse(ReadOnlySpan<byte> bytes, int recordSize)
    {
        ArgumentOutOfRangeException.ThrowIfNegative(recordSize);

        int count = recordSize == 0
            ? Math.Max(0, (bytes.Length / 4) - 1)
            : Math.Max(0, (bytes.Length - 4) / (4 + recordSize));

        int[] positions = new int[count + 1];
        for (int i = 0; i <= count; i++)
        {
            int at = 4 * i;
            positions[i] = at + 4 <= bytes.Length
                ? BinaryPrimitives.ReadInt32LittleEndian(bytes[at..])
                : positions[Math.Max(0, i - 1)];
        }

        byte[] data = new byte[count * recordSize];
        if (recordSize > 0)
        {
            int dataStart = 4 * (count + 1);
            int available = Math.Max(0, Math.Min(data.Length, bytes.Length - dataStart));
            bytes.Slice(dataStart, available).CopyTo(data);
        }

        return new Ww8Plcf(positions, data, recordSize);
    }

    /// <summary>An empty table, for a document that declares none.</summary>
    public static Ww8Plcf Empty { get; } = new([0], [], 0);

    /// <summary>The record at an index.</summary>
    public ReadOnlySpan<byte> Record(int index)
        => _recordSize == 0 || index < 0 || index >= Count
            ? default
            : _data.AsSpan(index * _recordSize, _recordSize);

    /// <summary>
    /// The index of the record whose range contains a position, or -1 when none does.
    /// </summary>
    /// <remarks>
    /// Binary search, because this is asked once per character position in the document and a
    /// linear scan would make the walk quadratic.
    /// </remarks>
    public int IndexOf(int position)
    {
        int low = 0;
        int high = Count - 1;
        while (low <= high)
        {
            int middle = low + ((high - low) / 2);
            if (position < _positions[middle]) high = middle - 1;
            else if (position >= _positions[middle + 1]) low = middle + 1;
            else return middle;
        }
        return -1;
    }
}

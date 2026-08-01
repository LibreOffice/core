using System.Buffers.Binary;

namespace Paperless.Vector.EmfPlus;

/// <summary>
/// A bounds-checked little-endian cursor over one EMF+ record's bytes.
/// </summary>
/// <remarks>
/// <para>
/// EMF+ needs its own cursor rather than the EMF reader's because the two disagree about what a
/// stream is. An EMF record is a slice of the file at a known offset and its sub-structures point
/// at the file; an EMF+ record's fields are read strictly in order, and an object may be
/// <em>assembled</em> from several records into a buffer that was never a slice of anything. So a
/// cursor over a span, carried by value into each parser, is what both cases have in common.
/// </para>
/// <para>
/// Every read past the end sets <see cref="Failed"/> and answers zero rather than throwing, for
/// the reason the whole library gives: a picture that cannot be read is a document to draw
/// without that picture. A parser therefore has to check <see cref="Failed"/> before it acts on
/// what it read, not before it reads.
/// </para>
/// </remarks>
internal sealed class EmfPlusStream
{
    private readonly byte[] _bytes;
    private readonly int _start;
    private readonly int _end;
    private int _position;

    /// <summary>Creates a cursor over a range of a buffer.</summary>
    /// <param name="bytes">The buffer.</param>
    /// <param name="start">Where this record's data begins.</param>
    /// <param name="length">How many bytes it has.</param>
    public EmfPlusStream(byte[] bytes, int start, int length)
    {
        _bytes = bytes ?? throw new ArgumentNullException(nameof(bytes));
        _start = Math.Clamp(start, 0, bytes.Length);
        _end = (int)Math.Clamp((long)_start + Math.Max(length, 0), _start, bytes.Length);
        _position = _start;
    }

    /// <summary>True once a read ran past the end.</summary>
    public bool Failed { get; private set; }

    /// <summary>How many bytes are left.</summary>
    public int Remaining => Math.Max(_end - _position, 0);

    /// <summary>How far into the record the cursor is.</summary>
    public int Offset => _position - _start;

    /// <summary>Reads one byte.</summary>
    public byte U8()
    {
        if (_position + 1 > _end)
        {
            Failed = true;
            return 0;
        }

        return _bytes[_position++];
    }

    /// <summary>Reads a 16-bit unsigned integer.</summary>
    public ushort U16()
    {
        if (_position + 2 > _end)
        {
            Failed = true;
            return 0;
        }

        ushort value = BinaryPrimitives.ReadUInt16LittleEndian(_bytes.AsSpan(_position));
        _position += 2;
        return value;
    }

    /// <summary>Reads a 16-bit signed integer.</summary>
    public short I16() => unchecked((short)U16());

    /// <summary>Reads a 32-bit unsigned integer.</summary>
    public uint U32()
    {
        if (_position + 4 > _end)
        {
            Failed = true;
            return 0;
        }

        uint value = BinaryPrimitives.ReadUInt32LittleEndian(_bytes.AsSpan(_position));
        _position += 4;
        return value;
    }

    /// <summary>Reads a 32-bit signed integer.</summary>
    public int I32() => unchecked((int)U32());

    /// <summary>Reads a 32-bit float, answering zero for a NaN or an infinity.</summary>
    /// <remarks>
    /// Every coordinate, size and matrix entry in EMF+ is a float, and a non-finite one poisons
    /// every arithmetic it touches: one NaN in a world transform makes the whole rest of the
    /// picture disappear rather than one shape. Filtering at the read is the only place that
    /// catches all of them.
    /// </remarks>
    public float F32()
    {
        float value = BitConverter.UInt32BitsToSingle(U32());
        return float.IsFinite(value) ? value : 0f;
    }

    /// <summary>Reads a UTF-16LE string of a stated length in characters.</summary>
    /// <param name="characters">How many code units to read.</param>
    public string Utf16(int characters)
    {
        if (characters <= 0) return string.Empty;
        if ((long)characters * 2 > Remaining)
        {
            Failed = true;
            return string.Empty;
        }

        Span<char> buffer = characters <= 512 ? stackalloc char[characters] : new char[characters];
        for (int i = 0; i < characters; i++) buffer[i] = (char)U16();

        int length = characters;
        while (length > 0 && buffer[length - 1] == '\0') length--;

        return new string(buffer[..length]);
    }

    /// <summary>The rest of the record as a span, without moving the cursor.</summary>
    public ReadOnlySpan<byte> Rest() => _bytes.AsSpan(Math.Min(_position, _end), Remaining);

    /// <summary>A span of the next <paramref name="length"/> bytes, and moves past them.</summary>
    public ReadOnlySpan<byte> Take(int length)
    {
        if (length <= 0) return default;
        if (length > Remaining)
        {
            Failed = true;
            return default;
        }

        ReadOnlySpan<byte> span = _bytes.AsSpan(_position, length);
        _position += length;
        return span;
    }

    /// <summary>Moves the cursor forward, clamped to the end.</summary>
    public void Skip(int count) => _position = (int)Math.Clamp((long)_position + count, _start, _end);

    /// <summary>Moves the cursor to an offset from the record's start, clamped to the end.</summary>
    public void SeekTo(long offset) => _position = (int)Math.Clamp(_start + Math.Max(offset, 0), _start, _end);
}

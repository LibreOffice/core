namespace Paperless.MsBinary.Records;

/// <summary>
/// The eight-byte record header the Microsoft drawing layer and PowerPoint's own record
/// stream share.
/// </summary>
/// <remarks>
/// <para>
/// Two bytes of version and instance, two of record type, four of payload length —
/// <c>DFF_COMMON_RECORD_HEADER_SIZE</c> in <c>include/svx/msdffdef.hxx:24</c>. Escher
/// (MS-ODRAW) records and PowerPoint's <c>PPT_PST_*</c> records use the identical header
/// and are freely nested inside one another in the same stream, which is why one reader
/// serves both.
/// </para>
/// <para>
/// The low nibble of the first word is the version, and the single value
/// <see cref="ContainerVersion"/> means "this record's payload is more records". Everything
/// else is a leaf atom. The remaining twelve bits are the instance, which is not a version
/// at all but a per-type discriminator: it carries the property count of an
/// <c>msofbtOPT</c>, the shape type of an <c>msofbtSp</c>, and which of a document's three
/// slide lists a <c>SlideListWithText</c> is.
/// </para>
/// </remarks>
/// <param name="Version">The low nibble of the first word; <see cref="ContainerVersion"/> for a container.</param>
/// <param name="Instance">The high twelve bits of the first word, meaning whatever the record type says.</param>
/// <param name="Type">The record type.</param>
/// <param name="Length">The declared payload length, which may overrun the stream.</param>
/// <param name="Position">The offset of the header itself within the stream.</param>
public readonly record struct DffRecordHeader(
    byte Version, ushort Instance, ushort Type, uint Length, int Position)
{
    /// <summary>The size of the header itself, which the declared length excludes.</summary>
    public const int HeaderSize = 8;

    /// <summary>The version value that marks a container rather than a leaf atom.</summary>
    public const byte ContainerVersion = 0x0F;

    /// <summary>Whether the payload is a sequence of further records.</summary>
    public bool IsContainer => Version == ContainerVersion;

    /// <summary>The offset of the first payload byte.</summary>
    public int ContentStart => Position + HeaderSize;

    /// <summary>
    /// The offset one past the record, as declared. May exceed the stream's length, which is
    /// why callers clamp rather than trust it.
    /// </summary>
    public long DeclaredEnd => (long)Position + HeaderSize + Length;
}

/// <summary>
/// Walks a buffer of <see cref="DffRecordHeader"/>-prefixed records.
/// </summary>
/// <remarks>
/// <para>
/// Reading is over a whole in-memory buffer rather than a forward-only stream because
/// nothing in these formats can be read in one pass: PowerPoint's persist directory turns an
/// object id into a byte offset, a shape's text may live in a container the document's slide
/// list points at, and Escher's property table stores its complex values in a block after the
/// fixed entries. All of that needs random access.
/// </para>
/// <para>
/// Every method clamps to the buffer. A declared length that overruns is extremely common in
/// files that were truncated or written by a third-party tool, and a reader that throws on one
/// loses the whole document rather than one record.
/// </para>
/// </remarks>
public sealed class DffRecordBuffer
{
    private readonly byte[] _data;

    /// <summary>Wraps a buffer holding a record stream.</summary>
    /// <param name="data">The whole stream. Not copied; the caller must not mutate it.</param>
    public DffRecordBuffer(byte[] data)
    {
        ArgumentNullException.ThrowIfNull(data);
        _data = data;
    }

    /// <summary>The stream's length in bytes.</summary>
    public int Length => _data.Length;

    /// <summary>The whole stream, for callers reading fixed structures at a known offset.</summary>
    public ReadOnlySpan<byte> Bytes => _data;

    /// <summary>Reads the header at an offset, or fails when eight bytes do not fit.</summary>
    public bool TryReadHeader(int offset, out DffRecordHeader header)
    {
        header = default;
        if (offset < 0 || offset > _data.Length - DffRecordHeader.HeaderSize) return false;

        ReadOnlySpan<byte> span = _data.AsSpan(offset);
        ushort versionAndInstance = ReadUInt16(span);
        header = new DffRecordHeader(
            (byte)(versionAndInstance & 0x000F),
            (ushort)(versionAndInstance >> 4),
            ReadUInt16(span[2..]),
            ReadUInt32(span[4..]),
            offset);
        return true;
    }

    /// <summary>The record's payload, clamped to what the buffer actually holds.</summary>
    public ReadOnlySpan<byte> Content(DffRecordHeader header)
    {
        int start = header.ContentStart;
        if (start < 0 || start >= _data.Length) return default;
        long end = Math.Min(header.DeclaredEnd, _data.Length);
        return _data.AsSpan(start, (int)(end - start));
    }

    /// <summary>The offset one past the record, clamped to the buffer.</summary>
    public int EndOf(DffRecordHeader header)
        => (int)Math.Clamp(header.DeclaredEnd, header.ContentStart, _data.Length);

    /// <summary>The records directly inside a container.</summary>
    /// <remarks>
    /// A container whose declared length overruns still yields the children that do fit, which
    /// is what makes a truncated slide give up only its tail.
    /// </remarks>
    public IEnumerable<DffRecordHeader> Children(DffRecordHeader container)
        => Range(container.ContentStart, EndOf(container));

    /// <summary>The records in a byte range, walked as siblings.</summary>
    /// <param name="start">The first record's header offset.</param>
    /// <param name="end">One past the last byte to consider; clamped to the buffer.</param>
    public IEnumerable<DffRecordHeader> Range(int start, int end)
    {
        int limit = Math.Min(end, _data.Length);
        int position = Math.Max(start, 0);

        while (position <= limit - DffRecordHeader.HeaderSize)
        {
            if (!TryReadHeader(position, out DffRecordHeader header)) yield break;
            yield return header;

            // Advancing by the header alone when the length is zero is what stops a run of
            // zero bytes — the usual tail of a truncated stream — from looping forever.
            long next = header.DeclaredEnd;
            if (next <= position) next = position + DffRecordHeader.HeaderSize;
            if (next > limit) yield break;
            position = (int)next;
        }
    }

    /// <summary>The first child of a container with the given type, if any.</summary>
    public DffRecordHeader? FirstChild(DffRecordHeader container, ushort type)
    {
        foreach (DffRecordHeader child in Children(container))
        {
            if (child.Type == type) return child;
        }

        return null;
    }

    /// <summary>Reads a little-endian 16-bit value, or zero when the span is short.</summary>
    public static ushort ReadUInt16(ReadOnlySpan<byte> span)
        => span.Length >= 2 ? (ushort)(span[0] | (span[1] << 8)) : (ushort)0;

    /// <summary>Reads a little-endian 32-bit value, or zero when the span is short.</summary>
    public static uint ReadUInt32(ReadOnlySpan<byte> span)
        => span.Length >= 4
            ? (uint)(span[0] | (span[1] << 8) | (span[2] << 16) | (span[3] << 24))
            : 0u;
}

using System.Buffers.Binary;
using System.Text;
using Paperless.Core.Diagnostics;
using Paperless.Text.Encodings;

namespace Paperless.Spreadsheets.MsBinary;

/// <summary>
/// Reads the record stream a BIFF workbook is made of, following <c>CONTINUE</c> records
/// as if they were not there.
/// </summary>
/// <remarks>
/// <para>
/// A BIFF record is a 16-bit identifier, a 16-bit length and that many bytes of payload. A
/// payload longer than the 8224-byte record ceiling is split, and the remainder arrives in
/// one or more <c>CONTINUE</c> records (0x003C) that carry no header of their own. The
/// modelled equivalent of LibreOffice's <c>XclImpStream</c>
/// (<c>sc/source/filter/inc/xistream.hxx:230-276</c>): callers read values and the
/// boundaries are invisible.
/// </para>
/// <para>
/// The boundaries are invisible <em>except</em> to a string, which is the trap in this
/// format. When a Unicode string spans into a <c>CONTINUE</c>, the continuation begins with
/// a fresh flags byte, and its 16-bit bit may differ from the one the string started with —
/// so a wide string can continue as a compressed one and the reader must re-read that byte
/// at every boundary (<c>XclImpStream::JumpToNextStringContinue</c>). Reading straight
/// through instead produces text that is correct up to the boundary and mojibake after it.
/// </para>
/// <para>
/// Every read is clamped. A record whose declared length overruns the stream, a chain of
/// continuations that runs off the end, a length field that describes more strings than
/// there are bytes: all of them yield short data and a diagnostic rather than an exception,
/// because a workbook that is damaged in its last sheet still has readable earlier ones.
/// </para>
/// </remarks>
internal sealed class BiffStream
{
    /// <summary>
    /// How many consecutive zero records (id and length both zero) are stepped over before
    /// the stream is given up on.
    /// </summary>
    /// <remarks>
    /// Not a theoretical case: LibreOffice carries the same counter with the comment that
    /// "Crystal Report" writes zero records between real ones
    /// (<c>XclImpStream::StartNextRecord</c>, i#4266).
    /// </remarks>
    private const int MaxZeroRecords = 5;

    private readonly byte[] _data;
    private readonly List<Diagnostic> _diagnostics;
    private readonly List<Segment> _segments = [];

    private int _segment;
    private int _position;
    private int _nextRecordPos;
    private int _reportedOverruns;

    public BiffStream(byte[] data, List<Diagnostic> diagnostics)
    {
        _data = data;
        _diagnostics = diagnostics;
    }

    /// <summary>The BIFF generation, set once the workbook's first BOF has been read.</summary>
    public BiffVersion Version { get; set; } = BiffVersion.Biff8;

    /// <summary>
    /// The encoding for 8-bit strings, from the <c>CODEPAGE</c> record.
    /// </summary>
    /// <remarks>
    /// This governs BIFF5 text and the handful of byte strings BIFF8 still uses. It does
    /// <em>not</em> govern a BIFF8 "compressed" Unicode string: those bytes are Latin-1 code
    /// points rather than code-page bytes, which is why compressed text in a Cyrillic
    /// workbook is still read correctly without consulting the code page at all.
    /// </remarks>
    public Encoding Encoding { get; set; } = LegacyCodePages.Fallback;

    /// <summary>The whole stream's length.</summary>
    public int Length => _data.Length;

    /// <summary>The current record's identifier.</summary>
    public ushort RecordId { get; private set; }

    /// <summary>Where the current record's header started.</summary>
    public int RecordPosition { get; private set; }

    /// <summary>
    /// False once a read has run past what the stream holds. Reads keep returning zeroes so
    /// a caller need not check after every one.
    /// </summary>
    public bool IsValid { get; private set; }

    /// <summary>Bytes left in the current record, counting its continuations.</summary>
    public int RecordLeft
    {
        get
        {
            if (!IsValid) return 0;
            int left = 0;
            for (int i = _segment; i < _segments.Count; i++)
                left += _segments[i].End - Math.Max(_position, _segments[i].Start);
            return left;
        }
    }

    /// <summary>Bytes left before the next continuation boundary.</summary>
    private int SegmentLeft
        => _segment < _segments.Count ? _segments[_segment].End - _position : 0;

    /// <summary>Advances to the next record, from wherever the last one ended.</summary>
    /// <returns>False at the end of the stream.</returns>
    public bool MoveNext() => MoveNext(_nextRecordPos);

    /// <summary>Advances to the record starting at an absolute offset.</summary>
    public bool MoveNext(int position)
    {
        int zeroRecords = 0;

        while (true)
        {
            if (position < 0 || position + 4 > _data.Length)
            {
                IsValid = false;
                return false;
            }

            ushort id = BinaryPrimitives.ReadUInt16LittleEndian(_data.AsSpan(position));
            int size = BinaryPrimitives.ReadUInt16LittleEndian(_data.AsSpan(position + 2));

            if (id == 0 && size == 0)
            {
                // Padding written between records by some producers. Stepping over a few is
                // recovery; stepping over many means the position is not a record boundary at
                // all, and continuing would resynchronise onto noise.
                if (++zeroRecords > MaxZeroRecords)
                {
                    IsValid = false;
                    return false;
                }

                position += 4;
                continue;
            }

            RecordPosition = position;
            RecordId = id;
            IsValid = true;
            _segments.Clear();
            AddSegment(position + 4, size);

            // Absorb the continuations now, so that reading is a walk over one logical
            // payload and the caller never has to know a boundary exists.
            int next = _segments[^1].End;
            while (next + 4 <= _data.Length
                   && BinaryPrimitives.ReadUInt16LittleEndian(_data.AsSpan(next)) == BiffRecords.Continue)
            {
                int continued = BinaryPrimitives.ReadUInt16LittleEndian(_data.AsSpan(next + 2));
                AddSegment(next + 4, continued);
                next = _segments[^1].End;
                if (_segments[^1].End <= _segments[^1].Start) break;
            }

            _segment = 0;
            _position = _segments[0].Start;
            _nextRecordPos = next;
            return true;
        }
    }

    /// <summary>The identifier of the record at an offset, without moving.</summary>
    /// <remarks>
    /// Used to check that a <c>BOUNDSHEET</c> offset really points at a <c>BOF</c> before
    /// jumping there. It often does not — see <see cref="XlsWorkbookReader"/>.
    /// </remarks>
    public ushort PeekRecordId(int position)
        => position >= 0 && position + 4 <= _data.Length
            ? BinaryPrimitives.ReadUInt16LittleEndian(_data.AsSpan(position))
            : (ushort)0;

    private void AddSegment(int start, int declared)
    {
        int end = start + declared;
        if (end > _data.Length)
        {
            end = _data.Length;
            Report("PL2320", $"A record at offset {start - 4} declares {declared} bytes but the "
                             + "stream ends first; the record has been truncated.");
        }

        _segments.Add(new Segment(start, Math.Max(start, end)));
    }

    private void Report(string code, string message)
    {
        // One diagnostic per kind of damage. A file whose record chain is broken produces the
        // same complaint at every subsequent record, and thousands of copies help nobody.
        if (_reportedOverruns++ > 0) return;
        _diagnostics.Add(new Diagnostic(
            DiagnosticSeverity.Warning, code, message,
            new DiagnosticLocation(PartName: "Workbook")));
    }

    /// <summary>Moves to the next continuation, or invalidates the stream when there is none.</summary>
    private bool NextSegment()
    {
        if (_segment + 1 >= _segments.Count)
        {
            IsValid = false;
            return false;
        }

        _segment++;
        _position = _segments[_segment].Start;
        return true;
    }

    /// <summary>Makes <paramref name="count"/> bytes readable without crossing a boundary.</summary>
    private bool Ensure(int count)
    {
        while (IsValid && SegmentLeft <= 0)
        {
            if (!NextSegment()) return false;
        }

        return IsValid && SegmentLeft >= count;
    }

    public byte ReadByte()
    {
        if (!Ensure(1)) return 0;
        return _data[_position++];
    }

    public ushort ReadUInt16()
    {
        if (Ensure(2))
        {
            ushort value = BinaryPrimitives.ReadUInt16LittleEndian(_data.AsSpan(_position));
            _position += 2;
            return value;
        }

        // A scalar straddling a continuation boundary is not supposed to happen, but a
        // damaged file does it; assembling it byte-wise is closer to right than giving up.
        return (ushort)(ReadByte() | (ReadByte() << 8));
    }

    public short ReadInt16() => unchecked((short)ReadUInt16());

    public uint ReadUInt32()
    {
        if (Ensure(4))
        {
            uint value = BinaryPrimitives.ReadUInt32LittleEndian(_data.AsSpan(_position));
            _position += 4;
            return value;
        }

        return (uint)(ReadUInt16() | (ReadUInt16() << 16));
    }

    public int ReadInt32() => unchecked((int)ReadUInt32());

    public double ReadDouble()
    {
        if (Ensure(8))
        {
            double value = BinaryPrimitives.ReadDoubleLittleEndian(_data.AsSpan(_position));
            _position += 8;
            return value;
        }

        ulong bits = ReadUInt32();
        bits |= (ulong)ReadUInt32() << 32;
        return BitConverter.UInt64BitsToDouble(bits);
    }

    /// <summary>Reads raw bytes, spanning continuations as needed.</summary>
    public byte[] ReadBytes(int count)
    {
        if (count <= 0) return [];

        byte[] buffer = new byte[Math.Min(count, RecordLeft)];
        int written = 0;
        while (written < buffer.Length && IsValid)
        {
            if (!Ensure(1)) break;
            int take = Math.Min(SegmentLeft, buffer.Length - written);
            _data.AsSpan(_position, take).CopyTo(buffer.AsSpan(written));
            _position += take;
            written += take;
        }

        return written == buffer.Length ? buffer : buffer[..written];
    }

    /// <summary>Skips bytes, spanning continuations.</summary>
    public void Skip(int count)
    {
        while (count > 0 && IsValid)
        {
            if (!Ensure(1)) return;
            int take = Math.Min(SegmentLeft, count);
            _position += take;
            count -= take;
        }
    }

    /// <summary>Abandons the rest of the current record.</summary>
    public void SkipRecord() => Skip(RecordLeft);

    /// <summary>
    /// Reads a string in whichever form this BIFF generation uses, given its length field.
    /// </summary>
    /// <param name="eightBitLength">
    /// True when the length is one byte rather than two. Which it is depends on the record,
    /// not on the format, so the caller has to say.
    /// </param>
    public string ReadString(bool eightBitLength)
    {
        int chars = eightBitLength ? ReadByte() : ReadUInt16();
        return Version == BiffVersion.Biff8
            ? ReadUnicodeString(chars)
            : ReadByteString(chars);
    }

    /// <summary>
    /// Reads a BIFF8 Unicode string whose character count has already been read.
    /// </summary>
    /// <remarks>
    /// The flags byte that follows the count says whether the characters are 16-bit, whether
    /// rich-text formatting runs follow the characters, and whether an Asian phonetic block
    /// does. Both trailers sit <em>after</em> the character data and their sizes are declared
    /// before it, which is why they are read here and skipped at the end rather than being
    /// left to the caller.
    /// </remarks>
    public string ReadUnicodeString(int chars)
    {
        byte flags = ReadByte();
        return ReadUnicodeString(chars, flags);
    }

    /// <inheritdoc cref="ReadUnicodeString(int)"/>
    public string ReadUnicodeString(int chars, byte flags)
    {
        bool wide = (flags & 0x01) != 0;
        bool rich = (flags & 0x08) != 0;
        bool farEast = (flags & 0x04) != 0;

        int runs = rich ? ReadUInt16() : 0;
        int extended = farEast ? ReadInt32() : 0;

        string text = ReadRawUnicodeString(chars, wide);

        Skip(runs * 4);
        Skip(extended);
        return text;
    }

    /// <summary>
    /// Reads the character data of a BIFF8 string, re-reading the flags byte at every
    /// continuation boundary.
    /// </summary>
    public string ReadRawUnicodeString(int chars, bool wide)
    {
        if (chars <= 0) return string.Empty;

        StringBuilder builder = new(Math.Min(chars, 4096));
        int left = chars;

        while (left > 0 && IsValid)
        {
            int available = wide ? SegmentLeft / 2 : SegmentLeft;
            int take = Math.Min(left, Math.Max(available, 0));

            for (int i = 0; i < take; i++)
            {
                int code = wide ? ReadUInt16() : ReadByte();

                // A NUL inside a string is not a terminator here; LibreOffice substitutes a
                // question mark for it so the character positions of the rich-text runs that
                // may follow still line up.
                builder.Append(code == 0 ? '?' : (char)code);
            }

            left -= take;
            if (left <= 0) break;

            // The continuation restates whether it is wide. This byte is the whole reason a
            // string cannot simply be read as a block.
            if (!NextSegment()) break;
            wide = (ReadByte() & 0x01) != 0;
        }

        return builder.ToString();
    }

    /// <summary>Reads a code-page-encoded string whose length has already been read.</summary>
    public string ReadByteString(int length)
    {
        byte[] bytes = ReadBytes(length);
        return bytes.Length == 0 ? string.Empty : Encoding.GetString(bytes);
    }

    private readonly record struct Segment(int Start, int End);
}

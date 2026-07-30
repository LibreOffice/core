using System.Buffers.Binary;
using System.Text;
using Paperless.Core.Diagnostics;

namespace Paperless.WordProcessing.Ww8;

/// <summary>One piece of a WW8 document's text.</summary>
/// <param name="StartCharacter">The first character position this piece covers.</param>
/// <param name="EndCharacter">One past the last character position it covers.</param>
/// <param name="FileOffset">Where its bytes begin in the <c>WordDocument</c> stream.</param>
/// <param name="IsCompressed">
/// True when the bytes are one per character in the document's code page, false when they are
/// UTF-16. A single document mixes both, piece by piece.
/// </param>
/// <param name="PropertyModifier">
/// The piece's <c>prm</c>: a formatting modifier applied to the whole piece. Kept because a
/// complex file can carry character formatting here rather than in the usual exception tables.
/// </param>
public readonly record struct Ww8Piece(
    int StartCharacter,
    int EndCharacter,
    uint FileOffset,
    bool IsCompressed,
    ushort PropertyModifier)
{
    /// <summary>How many characters this piece covers.</summary>
    public int Length => EndCharacter - StartCharacter;

    /// <summary>How many bytes one character takes in this piece.</summary>
    public int BytesPerCharacter => IsCompressed ? 1 : 2;
}

/// <summary>
/// The piece table: the map from a document's logical character positions to the bytes that hold
/// them.
/// </summary>
/// <remarks>
/// <para>
/// This is the structure a WW8 reader must get right before anything else. Word's fast-save
/// appends edited text to the end of the file rather than rewriting it in place, so a document's
/// text is a sequence of pieces pointing at <em>disjoint, out-of-order</em> byte ranges. Reading
/// the stream from <c>fcMin</c> to <c>fcMac</c> and hoping produces text that is scrambled rather
/// than obviously broken — sentences from different edits interleaved — which is why
/// <c>research/02-writer.md</c> section C.2 names it the first thing to get right.
/// </para>
/// <para>
/// Each piece also states its own encoding: compressed pieces are one byte per character in the
/// document's code page, uncompressed ones are UTF-16. One document mixes both freely, so the
/// decision is per piece and never per document.
/// </para>
/// </remarks>
public sealed class Ww8PieceTable
{
    /// <summary>
    /// The bit in a piece's <c>fc</c> that marks it as one byte per character.
    /// </summary>
    /// <remarks>
    /// When set, the remaining bits are twice the real offset — the field stores a byte offset for
    /// UTF-16 text and reuses the same width for half as many bytes. Forgetting the halving reads
    /// each piece from twice its true position.
    /// </remarks>
    private const uint CompressedFlag = 0x4000_0000;

    private const uint OffsetMask = 0x3FFF_FFFF;

    private readonly List<Ww8Piece> _pieces = [];
    private readonly byte[] _wordDocument;
    private readonly Encoding _codePage;

    private Ww8PieceTable(byte[] wordDocument, Encoding codePage)
    {
        _wordDocument = wordDocument;
        _codePage = codePage;
    }

    /// <summary>The pieces, in logical order.</summary>
    public IReadOnlyList<Ww8Piece> Pieces => _pieces;

    /// <summary>How many characters the whole text — body and subdocuments — occupies.</summary>
    public int CharacterCount => _pieces.Count == 0 ? 0 : _pieces[^1].EndCharacter;

    /// <summary>
    /// Parses the piece table from a document's <c>Clx</c>.
    /// </summary>
    /// <param name="clx">The <c>Clx</c> bytes from the table stream.</param>
    /// <param name="wordDocument">The whole <c>WordDocument</c> stream.</param>
    /// <param name="codePage">The encoding for compressed pieces.</param>
    /// <param name="diagnostics">Receives malformed structures.</param>
    /// <remarks>
    /// A <c>Clx</c> is a run of optional <c>Prc</c> blocks — formatting prefixes this reader does
    /// not need — followed by exactly one <c>Pcdt</c> holding the pieces. The prefixes must still
    /// be walked to find the <c>Pcdt</c>, because their lengths are what says where it starts.
    /// </remarks>
    public static Ww8PieceTable Parse(
        ReadOnlySpan<byte> clx, byte[] wordDocument, Encoding codePage, IList<Diagnostic> diagnostics)
    {
        ArgumentNullException.ThrowIfNull(wordDocument);
        ArgumentNullException.ThrowIfNull(codePage);

        Ww8PieceTable table = new(wordDocument, codePage);

        int position = 0;
        while (position < clx.Length)
        {
            byte kind = clx[position];
            if (kind == 0x01)
            {
                // A Prc: two bytes of length, then that many bytes of grpprl.
                if (position + 3 > clx.Length) break;
                short length = BinaryPrimitives.ReadInt16LittleEndian(clx[(position + 1)..]);
                position += 3 + Math.Max(0, (int)length);
                continue;
            }

            if (kind == 0x02)
            {
                if (position + 5 > clx.Length) break;
                int length = BinaryPrimitives.ReadInt32LittleEndian(clx[(position + 1)..]);
                int start = position + 5;
                if (length < 0 || start + length > clx.Length) length = clx.Length - start;
                table.ParsePieces(clx.Slice(start, length), diagnostics);
                return table;
            }

            // Neither marker: the Clx is malformed. Stop rather than resynchronising, because
            // guessing at where the pieces start would produce plausible-looking rubbish.
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2300",
                $"The piece table contains an unrecognised block type 0x{kind:X2}; the text may be "
                + "incomplete.",
                new DiagnosticLocation("Clx", position)));
            break;
        }

        if (table._pieces.Count == 0)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2301",
                "The document declares no piece table, so its text cannot be located."));
        }
        return table;
    }

    private void ParsePieces(ReadOnlySpan<byte> plcPcd, IList<Diagnostic> diagnostics)
    {
        // A PlcPcd is n+1 character positions followed by n eight-byte descriptors, so its length
        // determines n: 4(n+1) + 8n = length.
        int count = (plcPcd.Length - 4) / 12;
        if (count <= 0)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2302",
                $"The piece table is {plcPcd.Length} bytes, too short to describe any text."));
            return;
        }

        int descriptorsAt = 4 * (count + 1);
        for (int i = 0; i < count; i++)
        {
            int startCharacter = BinaryPrimitives.ReadInt32LittleEndian(plcPcd[(4 * i)..]);
            int endCharacter = BinaryPrimitives.ReadInt32LittleEndian(plcPcd[(4 * (i + 1))..]);

            int descriptor = descriptorsAt + (8 * i);
            uint rawOffset = BinaryPrimitives.ReadUInt32LittleEndian(plcPcd[(descriptor + 2)..]);
            ushort modifier = BinaryPrimitives.ReadUInt16LittleEndian(plcPcd[(descriptor + 6)..]);

            bool compressed = (rawOffset & CompressedFlag) != 0;
            uint offset = rawOffset & OffsetMask;
            if (compressed) offset /= 2;

            if (endCharacter <= startCharacter) continue;

            _pieces.Add(new Ww8Piece(startCharacter, endCharacter, offset, compressed, modifier));
        }
    }

    /// <summary>
    /// Where a character position's bytes are in the <c>WordDocument</c> stream, or -1 when no piece
    /// covers it.
    /// </summary>
    /// <remarks>
    /// This is the lookup every formatting question goes through. WW8's exception tables are keyed by
    /// <em>byte</em> offset while everything the reader walks is a character position, so a fast-saved
    /// document — whose positions are not in file order — resolves its formatting correctly only if
    /// the translation goes through the pieces rather than through arithmetic on the position.
    /// </remarks>
    public int FileOffsetOf(int position)
    {
        int low = 0;
        int high = _pieces.Count - 1;
        while (low <= high)
        {
            int middle = low + ((high - low) / 2);
            Ww8Piece piece = _pieces[middle];
            if (position < piece.StartCharacter) high = middle - 1;
            else if (position >= piece.EndCharacter) low = middle + 1;
            else
            {
                long offset = piece.FileOffset
                    + ((long)(position - piece.StartCharacter) * piece.BytesPerCharacter);
                return offset is >= 0 and <= int.MaxValue ? (int)offset : -1;
            }
        }
        return -1;
    }

    /// <summary>
    /// Reads the characters in a range of character positions.
    /// </summary>
    /// <remarks>
    /// The result has exactly one <see cref="char"/> per character position, which matters because
    /// every other WW8 table is keyed by position: a formatting exception, a footnote reference and
    /// a field marker are all located by CP, so the text and those tables have to agree on what a
    /// position counts. A piece that runs past the end of the stream is truncated and reported
    /// rather than abandoning the range.
    /// </remarks>
    public string ReadText(int startCharacter, int endCharacter, IList<Diagnostic>? diagnostics = null)
    {
        if (endCharacter <= startCharacter) return string.Empty;

        StringBuilder text = new(endCharacter - startCharacter);
        foreach (Ww8Piece piece in _pieces)
        {
            if (piece.EndCharacter <= startCharacter) continue;
            if (piece.StartCharacter >= endCharacter) break;

            int from = Math.Max(startCharacter, piece.StartCharacter);
            int to = Math.Min(endCharacter, piece.EndCharacter);

            long byteStart = piece.FileOffset + ((long)(from - piece.StartCharacter) * piece.BytesPerCharacter);
            long byteCount = (long)(to - from) * piece.BytesPerCharacter;

            if (byteStart < 0 || byteStart >= _wordDocument.Length)
            {
                Report(diagnostics, piece);
                continue;
            }
            if (byteStart + byteCount > _wordDocument.Length)
            {
                byteCount = _wordDocument.Length - byteStart;
                Report(diagnostics, piece);
            }

            ReadOnlySpan<byte> bytes = _wordDocument.AsSpan((int)byteStart, (int)byteCount);
            text.Append(piece.IsCompressed
                ? DecodeCompressed(bytes)
                : Encoding.Unicode.GetString(bytes));
        }

        return text.ToString();
    }

    /// <summary>
    /// Decodes a compressed piece, honouring the eight characters WW8 maps outside its code page.
    /// </summary>
    /// <remarks>
    /// A compressed piece is nearly the document's code page, with one exception that matters:
    /// bytes 0x82, 0x83, 0x88, 0x8A, 0x8C, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x9A,
    /// 0x9B and 0x9C map to fixed Unicode characters regardless of the code page. Windows-1252
    /// already maps them that way, so a Western document needs no special case — but a Cyrillic or
    /// Greek document does, because its code page assigns those bytes to letters instead.
    /// </remarks>
    private string DecodeCompressed(ReadOnlySpan<byte> bytes)
    {
        // Fast path: no byte needs the override, so the code page can decode the run in one go.
        bool needsOverride = false;
        foreach (byte value in bytes)
        {
            if (value is >= 0x82 and <= 0x9C && CompressedOverride(value) != '\0')
            {
                needsOverride = true;
                break;
            }
        }
        if (!needsOverride) return _codePage.GetString(bytes);

        StringBuilder text = new(bytes.Length);
        Span<byte> single = stackalloc byte[1];
        foreach (byte value in bytes)
        {
            char overridden = CompressedOverride(value);
            if (overridden != '\0')
            {
                text.Append(overridden);
                continue;
            }
            single[0] = value;
            text.Append(_codePage.GetString(single));
        }
        return text.ToString();
    }

    /// <summary>
    /// The fixed Unicode character a compressed byte maps to, or NUL when the code page decides.
    /// </summary>
    private static char CompressedOverride(byte value) => value switch
    {
        0x82 => '‚',
        0x83 => 'ƒ',
        0x88 => 'ˆ',
        0x8A => 'Š',
        0x8C => 'Œ',
        0x91 => '‘',
        0x92 => '’',
        0x93 => '“',
        0x94 => '”',
        0x95 => '•',
        0x96 => '–',
        0x97 => '—',
        0x98 => '˜',
        0x9A => 'š',
        0x9B => '›',
        0x9C => 'œ',
        _ => '\0',
    };

    private static void Report(IList<Diagnostic>? diagnostics, Ww8Piece piece)
        => diagnostics?.Add(new Diagnostic(
            DiagnosticSeverity.Error, "PL2303",
            $"A text piece points at offset {piece.FileOffset}, past the end of the WordDocument "
            + "stream; that part of the text has been skipped.",
            new DiagnosticLocation("WordDocument", piece.FileOffset)));
}

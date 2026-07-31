using System.Buffers.Binary;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// The paragraph or character formatting exceptions for a document, indexed by byte position.
/// </summary>
/// <remarks>
/// <para>
/// WW8 stores formatting as <em>exceptions</em> packed into 512-byte pages called FKPs, and finds
/// the right page through a PLCF of page numbers. So resolving "what formatting applies here"
/// takes three steps: the character position becomes a byte position through the piece table, the
/// byte position selects an FKP, and the FKP's own index selects the grpprl within it.
/// </para>
/// <para>
/// The two flavours differ in one respect that is easy to miss. A paragraph FKP's index entries
/// are thirteen bytes each — one byte of offset plus twelve of cached height — while a character
/// FKP's are a single byte. Reading one with the other's stride finds grpprls at the wrong offsets,
/// which decode as plausible but wrong formatting rather than failing.
/// </para>
/// </remarks>
public sealed class Ww8FormattingTable
{
    /// <summary>The size of an FKP page, fixed by the format.</summary>
    public const int PageSize = 512;

    private readonly List<Entry> _entries = [];

    private Ww8FormattingTable(bool paragraphs) => IsParagraphs = paragraphs;

    /// <summary>True for paragraph exceptions, false for character exceptions.</summary>
    public bool IsParagraphs { get; }

    /// <summary>
    /// Reads a formatting table from its index PLCF and the pages it points at.
    /// </summary>
    /// <param name="index">
    /// The <c>PlcfbtePapx</c> or <c>PlcfbteChpx</c>: a PLCF whose four-byte records are page
    /// numbers.
    /// </param>
    /// <param name="wordDocument">The whole <c>WordDocument</c> stream, which holds the pages.</param>
    /// <param name="paragraphs">True to read paragraph exceptions, false for character ones.</param>
    public static Ww8FormattingTable Parse(
        ReadOnlySpan<byte> index, byte[] wordDocument, bool paragraphs)
    {
        ArgumentNullException.ThrowIfNull(wordDocument);

        Ww8FormattingTable table = new(paragraphs);
        Ww8Plcf pages = Ww8Plcf.Parse(index, recordSize: 4);

        for (int i = 0; i < pages.Count; i++)
        {
            uint pageNumber = BinaryPrimitives.ReadUInt32LittleEndian(pages.Record(i));
            long pageStart = (long)pageNumber * PageSize;
            if (pageStart < 0 || pageStart + PageSize > wordDocument.Length) continue;

            table.ReadPage(wordDocument.AsMemory((int)pageStart, PageSize));
        }

        table._entries.Sort((left, right) => left.Start.CompareTo(right.Start));
        return table;
    }

    /// <summary>An empty table, for a document that declares none.</summary>
    public static Ww8FormattingTable Empty { get; } = new(paragraphs: false);

    /// <summary>
    /// The grpprl whose range covers a byte position, or empty when none does.
    /// </summary>
    public ReadOnlyMemory<byte> Find(int byteOffset) => FindWithRange(byteOffset).Properties;

    /// <summary>
    /// The grpprl covering a byte position, together with the range it covers.
    /// </summary>
    /// <remarks>
    /// The range is what makes resolving formatting affordable. The walk asks for every character
    /// position in the document, and a formatting exception typically spans hundreds of them, so a
    /// caller that knows where the answer stops being valid can resolve once per run instead of once
    /// per character. An empty result reports an empty range, so a caller cannot cache "nothing" over
    /// a span that has something.
    /// </remarks>
    public (ReadOnlyMemory<byte> Properties, int Start, int End) FindWithRange(int byteOffset)
    {
        int low = 0;
        int high = _entries.Count - 1;
        while (low <= high)
        {
            int middle = low + ((high - low) / 2);
            Entry entry = _entries[middle];
            if (byteOffset < entry.Start) high = middle - 1;
            else if (byteOffset >= entry.End) low = middle + 1;
            else return (entry.Properties, entry.Start, entry.End);
        }
        return (default, 0, 0);
    }

    private void ReadPage(ReadOnlyMemory<byte> page)
    {
        ReadOnlySpan<byte> span = page.Span;

        // The run count is the page's last byte, so a page is read backwards before forwards.
        int runs = span[PageSize - 1];
        if (runs == 0) return;

        int indexStride = IsParagraphs ? 13 : 1;
        int positionsBytes = 4 * (runs + 1);
        if (positionsBytes + (runs * indexStride) > PageSize - 1) return;

        for (int i = 0; i < runs; i++)
        {
            int start = BinaryPrimitives.ReadInt32LittleEndian(span[(4 * i)..]);
            int end = BinaryPrimitives.ReadInt32LittleEndian(span[(4 * (i + 1))..]);
            if (end <= start) continue;

            int wordOffset = span[positionsBytes + (i * indexStride)];
            if (wordOffset == 0) continue;

            ReadOnlyMemory<byte> properties = IsParagraphs
                ? ReadParagraphProperties(page, wordOffset * 2)
                : ReadCharacterProperties(page, wordOffset * 2);

            if (!properties.IsEmpty) _entries.Add(new Entry(start, end, properties));
        }
    }

    /// <summary>
    /// Reads a PAPX, whose length is counted in words and whose first two operand bytes are the
    /// style index rather than a sprm.
    /// </summary>
    /// <remarks>
    /// The length byte is words, not bytes, and a zero means the real length is in the byte after
    /// it — a compression trick for PAPXs longer than 254 words. Both cases put the style index
    /// first, which is why the grpprl starts two bytes later than the length prefix suggests.
    /// </remarks>
    private static ReadOnlyMemory<byte> ReadParagraphProperties(ReadOnlyMemory<byte> page, int at)
    {
        ReadOnlySpan<byte> span = page.Span;
        if (at < 0 || at >= PageSize) return default;

        int words = span[at];
        int start;
        int length;

        if (words == 0)
        {
            // The two forms do not measure the same way, and the difference is one byte. A non-zero cb
            // means 2*cb - 1 bytes, which is always odd; a grpprl of even length therefore has to use this
            // form, where a second count follows and means 2*cb' bytes exactly. Subtracting one here as
            // well loses the last byte of every even-length PAPX — which is half of them — and a walk that
            // is one byte short at the end drops whatever sprm was last. That is not a visible corruption:
            // the sprms before it decode perfectly, so the document merely lacks one property. It cost a
            // cell's padding in a table before it was found.
            if (at + 1 >= PageSize) return default;
            words = span[at + 1];
            start = at + 2;
            length = words * 2;
        }
        else
        {
            start = at + 1;
            length = (words * 2) - 1;
        }

        if (length <= 0 || start + length > PageSize) return default;

        // The first two bytes are the istd, which the caller reads separately: it is not a sprm and
        // walking it as one would desynchronise the grpprl.
        return page.Slice(start, length);
    }

    /// <summary>Reads a CHPX, whose length prefix is a plain byte count.</summary>
    private static ReadOnlyMemory<byte> ReadCharacterProperties(ReadOnlyMemory<byte> page, int at)
    {
        ReadOnlySpan<byte> span = page.Span;
        if (at < 0 || at >= PageSize) return default;

        int length = span[at];
        return length <= 0 || at + 1 + length > PageSize ? default : page.Slice(at + 1, length);
    }

    /// <summary>
    /// The style index a PAPX names, and the grpprl after it.
    /// </summary>
    /// <remarks>
    /// Static rather than a member because a PAPX's shape is the same wherever it came from — a
    /// page here, or a style definition in the stylesheet.
    /// </remarks>
    public static (ushort StyleIndex, ReadOnlyMemory<byte> Properties) SplitParagraphProperties(
        ReadOnlyMemory<byte> papx)
        => papx.Length < 2
            ? ((ushort)0, default)
            : (BinaryPrimitives.ReadUInt16LittleEndian(papx.Span), papx[2..]);

    private readonly record struct Entry(int Start, int End, ReadOnlyMemory<byte> Properties);
}

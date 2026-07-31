using System.Buffers.Binary;
using Paperless.Core;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// The File Information Block: a WW8 document's table of contents.
/// </summary>
/// <remarks>
/// <para>
/// The FIB sits at the start of the <c>WordDocument</c> stream and is, in effect, a manually
/// versioned index: a handful of flags, the extents of the text, and then dozens of
/// offset-and-length pairs pointing at every auxiliary table — the stylesheet, the piece table,
/// the formatting exceptions, the footnote and header position tables. Nothing else in the file
/// can be found without it (<c>research/02-writer.md</c> section C.2).
/// </para>
/// <para>
/// Its layout is self-describing in exactly one respect: three counts (<c>csw</c>,
/// <c>cslw</c>, <c>cbRgFcLcb</c>) give the length of each following array, and they differ by
/// Word version. Reading them rather than assuming Word 97's values is what lets one parser
/// handle files from Word 97 through Word 2007.
/// </para>
/// </remarks>
public sealed class Ww8Fib
{
    /// <summary>The magic number every WW8 document starts with.</summary>
    public const ushort Signature = 0xA5EC;

    /// <summary>
    /// The <c>nFib</c> of Word 97, the oldest version this reader handles.
    /// </summary>
    /// <remarks>
    /// Word 6 and Word 95 use a different sprm numbering and a different FIB layout, so they are
    /// rejected rather than misread. LibreOffice supports them; Paperless reports them as
    /// unsupported until it does too.
    /// </remarks>
    public const ushort MinimumVersion = 193;

    private readonly uint[] _fileOffsets;
    private readonly uint[] _lengths;

    private Ww8Fib(ReadOnlySpan<byte> stream)
    {
        Identifier = BinaryPrimitives.ReadUInt16LittleEndian(stream);
        Version = BinaryPrimitives.ReadUInt16LittleEndian(stream[2..]);
        LanguageId = BinaryPrimitives.ReadUInt16LittleEndian(stream[6..]);

        ushort flags = BinaryPrimitives.ReadUInt16LittleEndian(stream[0x0A..]);
        IsComplex = (flags & 0x0004) != 0;
        IsEncrypted = (flags & 0x0100) != 0;
        UsesTable1Stream = (flags & 0x0200) != 0;
        HasExtendedCharacters = (flags & 0x1000) != 0;

        // The three self-describing counts. Each array's length is stated rather than fixed,
        // which is the only reason one parser can read six Word versions.
        int csw = BinaryPrimitives.ReadUInt16LittleEndian(stream[0x20..]);
        int cslwOffset = 0x22 + (2 * csw);
        int cslw = BinaryPrimitives.ReadUInt16LittleEndian(stream[cslwOffset..]);
        int longsOffset = cslwOffset + 2;

        int[] longs = new int[cslw];
        for (int i = 0; i < cslw; i++)
            longs[i] = BinaryPrimitives.ReadInt32LittleEndian(stream[(longsOffset + (4 * i))..]);

        // FibRgLw97's meaningful fields. The reserved slots between them are not skipped by
        // guesswork: their indices are fixed by the structure.
        CbMac = At(longs, 0);
        TextLength = At(longs, 3);
        FootnoteTextLength = At(longs, 4);
        HeaderTextLength = At(longs, 5);
        MacroTextLength = At(longs, 6);
        AnnotationTextLength = At(longs, 7);
        EndnoteTextLength = At(longs, 8);
        TextBoxTextLength = At(longs, 9);
        HeaderTextBoxTextLength = At(longs, 10);

        int pairsCountOffset = longsOffset + (4 * cslw);
        int pairsCount = BinaryPrimitives.ReadUInt16LittleEndian(stream[pairsCountOffset..]);
        int pairsOffset = pairsCountOffset + 2;

        _fileOffsets = new uint[pairsCount];
        _lengths = new uint[pairsCount];
        for (int i = 0; i < pairsCount; i++)
        {
            int at = pairsOffset + (8 * i);
            if (at + 8 > stream.Length) break;
            _fileOffsets[i] = BinaryPrimitives.ReadUInt32LittleEndian(stream[at..]);
            _lengths[i] = BinaryPrimitives.ReadUInt32LittleEndian(stream[(at + 4)..]);
        }

        static int At(int[] values, int index) => index < values.Length ? values[index] : 0;
    }

    /// <summary>Parses the FIB from the start of the <c>WordDocument</c> stream.</summary>
    /// <exception cref="MalformedDocumentException">
    /// The stream is too short, does not carry the WW8 signature, or is from a Word version this
    /// reader does not handle.
    /// </exception>
    public static Ww8Fib Parse(ReadOnlySpan<byte> stream)
    {
        // 0x9A is where the offset table starts in the smallest legal FIB; anything shorter
        // cannot be one.
        if (stream.Length < 0x9A)
        {
            throw new MalformedDocumentException(
                $"The WordDocument stream is {stream.Length} bytes, too short to hold a File "
                + "Information Block.");
        }

        Ww8Fib fib = new(stream);
        if (fib.Identifier != Signature)
        {
            throw new MalformedDocumentException(
                $"The WordDocument stream starts with 0x{fib.Identifier:X4} rather than the WW8 "
                + $"signature 0x{Signature:X4}.");
        }

        if (fib.Version < MinimumVersion)
        {
            throw new UnsupportedFormatException(
                Core.Formats.DocumentFormat.Doc,
                $"The document was written by Word 95 or earlier (nFib {fib.Version}); only Word "
                + "97 and later are supported. Earlier versions use a different sprm numbering "
                + "and would be misread rather than merely incomplete.");
        }

        return fib;
    }

    /// <summary>The magic number, which should be <see cref="Signature"/>.</summary>
    public ushort Identifier { get; }

    /// <summary>
    /// The <c>nFib</c> format version: 193 for Word 97, 217 for Word 2000, 257 for Word 2002,
    /// 268 for Word 2003, 274 for Word 2007.
    /// </summary>
    public ushort Version { get; }

    /// <summary>
    /// The document's language id, which is how a WW8 file names its code page — indirectly.
    /// </summary>
    /// <remarks>
    /// Unlike BIFF, WW8 records no code page at all: the encoding of its 8-bit text has to be
    /// inferred from this language (<c>research/05-infrastructure.md</c> section F.2).
    /// </remarks>
    public ushort LanguageId { get; }

    /// <summary>
    /// True when the document was fast-saved, so its text is in several out-of-order pieces.
    /// </summary>
    /// <remarks>
    /// The piece table is read either way — Word 97 always writes one — so this is informational
    /// rather than a switch. It is worth surfacing because a complex file is where a reader that
    /// ignores the piece table produces scrambled text rather than an obvious failure.
    /// </remarks>
    public bool IsComplex { get; }

    /// <summary>True when the document is encrypted and cannot be read without a password.</summary>
    public bool IsEncrypted { get; }

    /// <summary>
    /// Which stream holds the auxiliary tables: <c>1Table</c> when true, <c>0Table</c> when false.
    /// </summary>
    /// <remarks>
    /// Both names exist in the wild and a document carries only the one it uses. Choosing the
    /// wrong one finds either nothing or another save's stale tables.
    /// </remarks>
    public bool UsesTable1Stream { get; }

    /// <summary>True when the document may contain characters outside the ANSI range.</summary>
    public bool HasExtendedCharacters { get; }

    /// <summary>The count of bytes in the <c>WordDocument</c> stream that are in use.</summary>
    public int CbMac { get; }

    /// <summary>How many characters the main body occupies.</summary>
    public int TextLength { get; }

    /// <summary>How many characters the footnote texts occupy, after the body.</summary>
    public int FootnoteTextLength { get; }

    /// <summary>How many characters the header and footer texts occupy.</summary>
    public int HeaderTextLength { get; }

    /// <summary>How many characters the macro texts occupy.</summary>
    public int MacroTextLength { get; }

    /// <summary>How many characters the comment texts occupy.</summary>
    public int AnnotationTextLength { get; }

    /// <summary>How many characters the endnote texts occupy.</summary>
    public int EndnoteTextLength { get; }

    /// <summary>How many characters the text-box texts occupy.</summary>
    public int TextBoxTextLength { get; }

    /// <summary>How many characters the header text-box texts occupy.</summary>
    public int HeaderTextBoxTextLength { get; }

    /// <summary>The offset of one of the auxiliary tables, by its index in the FIB.</summary>
    public uint FileOffset(Ww8FibTable table)
        => (int)table < _fileOffsets.Length ? _fileOffsets[(int)table] : 0;

    /// <summary>The length of one of the auxiliary tables, by its index in the FIB.</summary>
    public uint Length(Ww8FibTable table)
        => (int)table < _lengths.Length ? _lengths[(int)table] : 0;

    /// <summary>True when the document declares a table at this index.</summary>
    public bool Has(Ww8FibTable table) => Length(table) > 0;
}

/// <summary>
/// The auxiliary tables the FIB points at, by their index in its offset-and-length array.
/// </summary>
/// <remarks>
/// Named rather than numbered at the call sites, because the array is positional and an
/// off-by-one silently reads a different table — which then parses as garbage rather than
/// failing. Only the tables Paperless reads are listed; the array has well over a hundred slots.
/// </remarks>
public enum Ww8FibTable
{
    /// <summary>The stylesheet.</summary>
    StyleSheet = 1,

    /// <summary>Footnote reference positions.</summary>
    FootnoteReferences = 2,

    /// <summary>Footnote text ranges.</summary>
    FootnoteTexts = 3,

    /// <summary>Comment reference positions.</summary>
    AnnotationReferences = 4,

    /// <summary>Comment text ranges.</summary>
    AnnotationTexts = 5,

    /// <summary>Section descriptors.</summary>
    SectionDescriptors = 6,

    /// <summary>Header and footer text ranges.</summary>
    HeaderTexts = 11,

    /// <summary>The index of the character-formatting exception pages.</summary>
    CharacterFormattingIndex = 12,

    /// <summary>The index of the paragraph-formatting exception pages.</summary>
    ParagraphFormattingIndex = 13,

    /// <summary>The font table.</summary>
    FontNames = 15,

    /// <summary>Field positions in the body.</summary>
    BodyFields = 16,

    /// <summary>Field positions in headers and footers.</summary>
    HeaderFields = 17,

    /// <summary>Field positions in footnotes.</summary>
    FootnoteFields = 18,

    /// <summary>Field positions in comments.</summary>
    AnnotationFields = 19,

    /// <summary>The bookmark names, as a string table.</summary>
    BookmarkNames = 21,

    /// <summary>
    /// The bookmark start positions, whose four-byte records index <see cref="BookmarkEnds"/>.
    /// </summary>
    /// <remarks>
    /// The index here is the FIB's <em>read order</em>, as everywhere in this enumeration, and not
    /// the byte offsets the C++ header's comments carry: those describe the Word 6 layout, where
    /// several of the fields are absent — <c>fcSttbfRMark</c>'s documented 0x1fa works out at index
    /// 44, where the Word 97 order puts it at 51. <c>ww8scan.cxx</c>'s <c>WW8Fib::WW8Fib</c> reads
    /// them in sequence and is the authority.
    /// </remarks>
    BookmarkStarts = 22,

    /// <summary>The bookmark end positions, reached through the start records rather than by order.</summary>
    BookmarkEnds = 23,

    /// <summary>
    /// The document properties: the <c>Dop</c>.
    /// </summary>
    /// <remarks>
    /// A single fixed-layout record rather than a table, and the only place a few document-wide layout
    /// decisions live — the default tab interval among them, which no paragraph states. Thirty-one, which
    /// is <c>fcDop</c>'s place in the FIB's array of offsets, two before the piece table's thirty-three.
    /// </remarks>
    DocumentProperties = 31,

    /// <summary>The piece table and its formatting prefixes.</summary>
    PieceTable = 33,

    /// <summary>Comment authors.</summary>
    AnnotationOwners = 36,

    /// <summary>Endnote reference positions.</summary>
    EndnoteReferences = 46,

    /// <summary>Endnote text ranges.</summary>
    EndnoteTexts = 47,

    /// <summary>Field positions in endnotes.</summary>
    EndnoteFields = 48,

    /// <summary>Drawing information: the Escher container shared with XLS and PPT.</summary>
    DrawingInformation = 50,

    /// <summary>The revision authors, as a string table: <c>SttbfRMark</c>.</summary>
    RevisionAuthors = 51,

    /// <summary>Text-box text ranges in the body.</summary>
    TextBoxTexts = 56,

    /// <summary>Text-box text ranges in headers and footers.</summary>
    HeaderTextBoxTexts = 58,

    /// <summary>
    /// The list definitions. Only the definition array is inside the declared length; the levels
    /// that follow it are not, which is why reading them needs the rest of the stream.
    /// </summary>
    ListDefinitions = 73,

    /// <summary>The list format overrides.</summary>
    ListFormatOverrides = 74,
}

using System.Buffers.Binary;
using System.Text;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Globalization;
using Paperless.Core.Numbering;
using Paperless.Text.Encodings;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// Reads a WW8 document's text and structure into the format-independent content tree.
/// </summary>
/// <remarks>
/// <para>
/// The walk is over <em>character positions</em>, not bytes. Everything in a WW8 file is keyed by
/// position — a formatting exception, a footnote reference, a field marker, a paragraph mark — so
/// the reader steps through positions and asks each table what applies there. The piece table turns
/// a position into the bytes that hold it, which is the indirection that makes a fast-saved
/// document readable at all.
/// </para>
/// <para>
/// Structure comes from characters rather than markup. A paragraph ends at a carriage return, a
/// table cell at U+0007, a field at U+0015 — so the same loop that collects text also decides where
/// paragraphs, cells and fields begin and end. Two of those characters are ambiguous on their own
/// and need the paragraph's properties to resolve: U+0007 is a cell end or a row end depending on a
/// sprm, and a paragraph is in a table only if one says so.
/// </para>
/// </remarks>
public sealed partial class Ww8DocumentReader
{
    /// <summary>The character that ends a paragraph.</summary>
    private const char ParagraphMark = '\r';

    /// <summary>The character that ends a table cell or, with the right sprm, a table row.</summary>
    private const char CellMark = '\u0007';

    /// <summary>
    /// The size of a text-box story's descriptor in the PLCF that partitions the text-box
    /// subdocuments.
    /// </summary>
    private const int TextBoxRecordSize = 22;

    private readonly byte[] _wordDocument;
    private readonly byte[] _table;
    private readonly Ww8Fib _fib;
    private readonly List<Diagnostic> _diagnostics;
    private readonly Encoding _codePage;
    private readonly Ww8PieceTable _pieces;
    private readonly Ww8FormattingTable _paragraphProperties;
    private readonly Ww8FormattingTable _characterProperties;
    private readonly Ww8StyleSheet _styles;
    private readonly Ww8Numbering _numbering;
    private readonly List<ContentNode> _hoisted = [];

    // Two counters, not one: the reference marks in the body number themselves as the walk meets
    // them, while the note sections number themselves as the notes are read. Sharing a counter makes
    // each note's label one higher than the mark that cites it.
    private int _footnoteNumber;
    private int _noteNumber;

    // Section indexes, counted the way the ODF and OOXML readers count them: the notes, comments and
    // frames share one sequence, and the page furniture has its own.
    private int _hoistedIndex;
    private int _furnitureIndex;

    /// <summary>Creates a reader over a document's streams.</summary>
    /// <param name="wordDocument">The <c>WordDocument</c> stream.</param>
    /// <param name="table">The <c>0Table</c> or <c>1Table</c> stream, whichever the FIB names.</param>
    /// <param name="fib">The parsed FIB.</param>
    /// <param name="diagnostics">Receives problems found while reading.</param>
    public Ww8DocumentReader(
        byte[] wordDocument, byte[] table, Ww8Fib fib, List<Diagnostic> diagnostics)
    {
        ArgumentNullException.ThrowIfNull(wordDocument);
        ArgumentNullException.ThrowIfNull(table);
        ArgumentNullException.ThrowIfNull(fib);
        ArgumentNullException.ThrowIfNull(diagnostics);

        _wordDocument = wordDocument;
        _table = table;
        _fib = fib;
        _diagnostics = diagnostics;

        // WW8 records no code page: the encoding of its 8-bit text is inferred from the document's
        // language id (research/05-infrastructure.md section F.2).
        _codePage = LegacyCodePages.Get(
            LegacyCodePages.FromLanguage(WindowsLanguages.TagOf(fib.LanguageId)));

        _pieces = Ww8PieceTable.Parse(
            Slice(Ww8FibTable.PieceTable), wordDocument, _codePage, diagnostics);

        _paragraphProperties = fib.Has(Ww8FibTable.ParagraphFormattingIndex)
            ? Ww8FormattingTable.Parse(
                Slice(Ww8FibTable.ParagraphFormattingIndex), wordDocument, paragraphs: true)
            : Ww8FormattingTable.Empty;

        _characterProperties = fib.Has(Ww8FibTable.CharacterFormattingIndex)
            ? Ww8FormattingTable.Parse(
                Slice(Ww8FibTable.CharacterFormattingIndex), wordDocument, paragraphs: false)
            : Ww8FormattingTable.Empty;

        _styles = fib.Has(Ww8FibTable.StyleSheet)
            ? Ww8StyleSheet.Parse(Slice(Ww8FibTable.StyleSheet))
            : Ww8StyleSheet.Empty;

        _numbering = fib.Has(Ww8FibTable.ListDefinitions)
            ? Ww8Numbering.Parse(
                Slice(Ww8FibTable.ListDefinitions, toStreamEnd: true),
                Slice(Ww8FibTable.ListFormatOverrides))
            : Ww8Numbering.Empty;
    }

    /// <summary>
    /// The document's sections, in document order.
    /// </summary>
    /// <remarks>
    /// A document with no <c>PlcfSed</c> still has one section carrying the defaults, which is what
    /// Word means by its absence rather than an absence of pages.
    /// </remarks>
    public IReadOnlyList<Model.WritingSection> Sections
    {
        get
        {
            List<Model.WritingSection> sections = Ww8SectionTable.Read(
                SectionDescriptors, _wordDocument);

            return sections.Count > 0 ? sections : [new Model.WritingSection()];
        }
    }

    /// <summary>
    /// The <c>PlcfSed</c>, whose positions delimit the sections in character space.
    /// </summary>
    /// <remarks>
    /// Read on demand and cached, because the layout walk asks it once per paragraph: DOC delimits sections
    /// by position rather than by a marker in the text, so which section a paragraph is in is a lookup
    /// against this rather than something the walk can count.
    /// </remarks>
    private Ww8Plcf SectionDescriptors =>
        _sectionDescriptors ??= PlcfOf(
            Ww8FibTable.SectionDescriptors, Ww8SectionTable.DescriptorSize);

    private Ww8Plcf? _sectionDescriptors;

    /// <summary>
    /// Which section a character position falls in.
    /// </summary>
    /// <remarks>
    /// A <c>PlcfSed</c>'s positions are the section <em>ends</em> in character space, so the lookup is the
    /// same one a formatting table does — and a position past the last one belongs to the last section,
    /// which is what a document whose descriptors do not cover its whole text means.
    /// </remarks>
    internal int SectionAt(int position)
    {
        int count = SectionDescriptors.Count;
        if (count <= 1) return 0;

        int index = SectionDescriptors.IndexOf(position);
        return index < 0 ? count - 1 : Math.Min(index, count - 1);
    }

    /// <summary>Reads the document.</summary>
    /// <param name="metadata">
    /// The document's properties, which live in the OLE property sets rather than anywhere this
    /// reader can see — so they are supplied by the caller that opened the compound file.
    /// </param>
    public ContentDocument Read(DocumentMetadata metadata)
    {
        ArgumentNullException.ThrowIfNull(metadata);

        ContentDocument document = new() { Metadata = metadata };

        Ww8Range body = Ranges.Body;
        ContentSection bodySection = new() { Kind = SectionKind.Body, Index = 0 };
        ReadRange(body, bodySection);
        document.Children.Add(bodySection);

        // Notes and comments belong immediately after the body they reference, then the furniture.
        ReadNotes(document);
        ReadComments(document);
        ReadTextBoxes(document);
        ReadHeadersAndFooters(document);

        foreach (ContentNode node in _hoisted) document.Children.Add(node);
        // Every range has been walked, so every bookmark position that exists has been located.
        BuildBookmarks();

        return document;
    }

    /// <summary>
    /// The character-position extents of each subdocument.
    /// </summary>
    /// <remarks>
    /// WW8 concatenates the subdocuments into one position space in a fixed order — body,
    /// footnotes, headers, macros, comments, endnotes, text boxes, header text boxes — each as long
    /// as the matching count in the FIB. Nothing marks the boundaries in the text, so the counts are
    /// the only way to know where the body stops and the footnotes begin; getting the order wrong
    /// silently reads a footnote as body text.
    /// </remarks>
    private Ww8Ranges Ranges
    {
        get
        {
            int at = 0;
            Ww8Range Next(int length)
            {
                Ww8Range range = new(at, at + Math.Max(0, length));
                at = range.End;
                return range;
            }

            return new Ww8Ranges(
                Next(_fib.TextLength),
                Next(_fib.FootnoteTextLength),
                Next(_fib.HeaderTextLength),
                Next(_fib.MacroTextLength),
                Next(_fib.AnnotationTextLength),
                Next(_fib.EndnoteTextLength),
                Next(_fib.TextBoxTextLength),
                Next(_fib.HeaderTextBoxTextLength));
        }
    }

    /// <summary>
    /// Reads the footnotes and endnotes, each as its own section.
    /// </summary>
    /// <remarks>
    /// The subdocument holds every note's text end to end; a PLCF of positions is what says where
    /// one note stops and the next starts. Without it every footnote in a document would arrive as
    /// one section.
    /// </remarks>
    private void ReadNotes(ContentDocument document)
    {
        ReadSubdocumentParts(
            document, Ranges.Footnotes, Ww8FibTable.FootnoteTexts, SectionKind.Note, numbered: true);
        ReadSubdocumentParts(
            document, Ranges.Endnotes, Ww8FibTable.EndnoteTexts, SectionKind.Note, numbered: true);
    }

    private void ReadComments(ContentDocument document)
    {
        List<string> authors = ReadAnnotationOwners();
        Ww8Plcf references = PlcfOf(Ww8FibTable.AnnotationReferences, recordSize: 30);

        int index = 0;
        foreach (Ww8Range part in SplitSubdocument(
                     Ranges.Comments, Ww8FibTable.AnnotationTexts))
        {
            ContentSection section = new()
            {
                Kind = SectionKind.Comment,
                Index = _hoistedIndex,
                Name = AuthorOf(references, authors, index),
            };
            ReadRange(part, section);
            if (HasText(section))
            {
                document.Children.Add(section);
                _hoistedIndex++;
            }
            index++;
        }
    }

    /// <summary>
    /// Reads the header and footer stories.
    /// </summary>
    /// <remarks>
    /// The header subdocument's PLCF begins with six stories that are not headers at all — the
    /// footnote and endnote separators, continuation separators and continuation notices — followed
    /// by six per section: even and odd headers, even and odd footers, and first-page header and
    /// footer. Reading the first six as headers puts a separator line's text into the document.
    /// </remarks>
    private void ReadHeadersAndFooters(ContentDocument document)
    {
        List<Ww8Range> stories = [.. SplitSubdocument(Ranges.Headers, Ww8FibTable.HeaderTexts)];

        string[] names = ["even header", "odd header", "even footer", "odd footer",
                          "first header", "first footer"];

        for (int story = SeparatorStories; story < stories.Count; story++)
        {
            int slot = (story - SeparatorStories) % 6;
            SectionKind kind = slot is 2 or 3 or 5 ? SectionKind.Footer : SectionKind.Header;

            ContentSection section = new()
            {
                Kind = kind,
                Index = _furnitureIndex,
                Name = names[slot],
            };
            ReadRange(stories[story], section);

            // Most of the twelve slots hold nothing but a paragraph mark: Word writes every slot
            // whether the section uses it or not, so emptiness is what distinguishes an unused
            // header from one the document actually has.
            if (!HasText(section)) continue;

            document.Children.Add(section);
            _furnitureIndex++;
        }
    }

    /// <summary>
    /// Reads the text-box stories, each as its own frame section.
    /// </summary>
    /// <remarks>
    /// Text boxes in the body and text boxes inside headers are two separate subdocuments with two
    /// separate PLCFs, so both have to be walked; a document with a text box in its header has an
    /// empty body text-box subdocument and all its shape text in the second.
    /// </remarks>
    private void ReadTextBoxes(ContentDocument document)
    {
        foreach ((Ww8Range subdocument, Ww8FibTable boundaries) in new[]
                 {
                     (Ranges.TextBoxes, Ww8FibTable.TextBoxTexts),
                     (Ranges.HeaderTextBoxes, Ww8FibTable.HeaderTextBoxTexts),
                 })
        {
            foreach (Ww8Range part in SplitSubdocument(
                         subdocument, boundaries, recordSize: TextBoxRecordSize))
            {
                ContentSection section = new() { Kind = SectionKind.Frame, Index = _hoistedIndex };
                ReadRange(part, section);
                if (!HasText(section)) continue;

                document.Children.Add(section);
                _hoistedIndex++;
            }
        }
    }

    private void ReadSubdocumentParts(
        ContentDocument document,
        Ww8Range subdocument,
        Ww8FibTable boundaries,
        SectionKind kind,
        bool numbered)
    {
        foreach (Ww8Range part in SplitSubdocument(subdocument, boundaries))
        {
            ContentSection section = new()
            {
                Kind = kind,
                Index = _hoistedIndex,
                Name = numbered ? OutlineNumbers.Digits(++_noteNumber) : null,
            };
            ReadRange(part, section);
            if (!HasText(section)) continue;

            document.Children.Add(section);
            _hoistedIndex++;
        }
    }

    /// <summary>
    /// True when a story holds something a reader would see.
    /// </summary>
    /// <remarks>
    /// A story that contains only paragraph marks is a placeholder rather than content — Word writes
    /// all twelve header slots and a terminating paragraph for every subdocument whether they are used
    /// or not, so counting children would report a dozen empty headers on a document that has one.
    /// </remarks>
    private static bool HasText(ContentNode node)
    {
        foreach (ContentNode child in node.Children)
        {
            if (child is ContentRun run && !string.IsNullOrWhiteSpace(run.Text)) return true;
            if (child is ContentImage or ContentTable) return true;
            if (HasText(child)) return true;
        }
        return false;
    }

    /// <summary>
    /// Splits a subdocument into its parts, using the PLCF of boundaries the FIB names.
    /// </summary>
    /// <param name="subdocument">The subdocument's extent in the document's position space.</param>
    /// <param name="boundaries">The PLCF of positions that partitions it.</param>
    /// <param name="recordSize">
    /// The size of the PLCF's data records. Zero for the note, comment and header tables, which are
    /// positions alone; the text-box tables carry a 22-byte descriptor per story, and reading one of
    /// those as though it had none takes its record bytes for positions — which point anywhere and
    /// yield stories full of unrelated text.
    /// </param>
    /// <remarks>
    /// <para>
    /// The PLCF's positions are relative to the subdocument's start, so they have to be rebased. Two
    /// adjustments then make the parts into stories, and both are needed for the same reason — a WW8
    /// story is <em>terminated</em> by a paragraph mark rather than merely containing one. The
    /// PLCF's last part is the subdocument's own terminating mark and is not a story at all, and each
    /// story's final character is its terminator rather than the start of another paragraph. So a
    /// story is read one character short, exactly as LibreOffice reads it
    /// (<c>SwWW8ImplReader::Read_HdFtText</c> passes <c>nLen - 1</c>). Without these a document with
    /// one header arrives with an empty paragraph inside it and an extra empty header after it.
    /// </para>
    /// <para>
    /// Empty stories are yielded rather than skipped: a caller that identifies a story by its position
    /// in the sequence — as the header stories are, six per section in a fixed order — would otherwise
    /// see every later story shift up by however many unused slots preceded it. When the table is
    /// absent the whole subdocument is one story, which keeps the text rather than losing it to a
    /// missing index.
    /// </para>
    /// </remarks>
    private IEnumerable<Ww8Range> SplitSubdocument(
        Ww8Range subdocument,
        Ww8FibTable boundaries,
        int recordSize = 0)
    {
        if (subdocument.Length <= 0) yield break;

        Ww8Plcf plcf = PlcfOf(boundaries, recordSize);
        if (plcf.Count <= 0)
        {
            yield return subdocument;
            yield break;
        }

        for (int i = 0; i < plcf.Count - 1; i++)
        {
            int start = subdocument.Start + plcf.Positions[i];
            int end = subdocument.Start + plcf.Positions[i + 1] - 1;
            if (end > subdocument.End) end = subdocument.End;
            yield return new Ww8Range(start, Math.Max(start, end));
        }
    }

    /// <summary>
    /// The comment authors, from the table the FIB points at.
    /// </summary>
    /// <remarks>
    /// A <c>GrpXstAtnOwners</c> is not a string table: it is a bare run of length-prefixed UTF-16
    /// strings with no count and no header, filling the table's declared length. It looks enough like
    /// the string tables elsewhere in the file to be read as one, and doing so takes the first name's
    /// length for a marker and its first two characters for a count — so the names come back as
    /// nonsense rather than not at all.
    /// </remarks>
    private List<string> ReadAnnotationOwners()
    {
        List<string> authors = [];
        ReadOnlySpan<byte> owners = Slice(Ww8FibTable.AnnotationOwners);

        int position = 0;
        while (position + 2 <= owners.Length)
        {
            int characters = BinaryPrimitives.ReadUInt16LittleEndian(owners[position..]);
            position += 2;

            int bytes = characters * 2;
            if (bytes < 0 || position + bytes > owners.Length) break;

            authors.Add(Encoding.Unicode.GetString(owners.Slice(position, bytes)));
            position += bytes;
        }

        return authors;
    }

    /// <summary>
    /// The author of a comment, looked up through its reference record.
    /// </summary>
    /// <remarks>
    /// The index into the author table is not the record's first field: it follows the commenter's
    /// initials, which occupy a fixed 22 bytes however short they are — so reading the record from its
    /// start finds the initials' length rather than the author.
    /// </remarks>
    private static string? AuthorOf(Ww8Plcf references, List<string> authors, int index)
    {
        if (index >= references.Count) return null;

        // xstUsrInitl: a two-byte count, up to nine characters, and a terminator.
        const int InitialsSize = 22;

        ReadOnlySpan<byte> record = references.Record(index);
        if (record.Length < InitialsSize + 2) return null;

        int author = BinaryPrimitives.ReadUInt16LittleEndian(record[InitialsSize..]);
        return author >= 0 && author < authors.Count && authors[author].Length > 0
            ? authors[author]
            : null;
    }

    /// <summary>
    /// The style index of the paragraph a character position belongs to.
    /// </summary>
    /// <remarks>
    /// Answered from the paragraph exception table rather than from the walk's own state, because a
    /// paragraph's properties live at its <em>end</em> mark: while the reader is producing a
    /// paragraph's runs it has not yet seen the mark that says which style they belong to. The
    /// exception table has no such ordering problem — its ranges cover the whole paragraph, so any
    /// position inside one finds the same PAPX.
    /// </remarks>
    private ushort ParagraphStyleIndexAt(int position)
    {
        int byteOffset = _pieces.FileOffsetOf(position);
        return byteOffset < 0
            ? (ushort)0
            : Ww8FormattingTable.SplitParagraphProperties(
                _paragraphProperties.Find(byteOffset)).StyleIndex;
    }

    private Ww8Plcf PlcfOf(Ww8FibTable table, int recordSize)
        => _fib.Has(table) ? Ww8Plcf.Parse(Slice(table), recordSize) : Ww8Plcf.Empty;

    /// <summary>The bytes of one auxiliary table, or empty when the document has none.</summary>
    /// <param name="table">Which table to slice.</param>
    /// <param name="toStreamEnd">
    /// Extend the slice to the end of the table stream rather than stopping at the declared length.
    /// The list definitions need this: their declared length covers only the definition array, and the
    /// variably sized level definitions follow it outside the length — so honouring the length exactly
    /// finds every list's header and none of its levels.
    /// </param>
    private ReadOnlySpan<byte> Slice(Ww8FibTable table, bool toStreamEnd = false)
    {
        long offset = _fib.FileOffset(table);
        long length = _fib.Length(table);

        if (length <= 0 || offset < 0 || offset >= _table.Length) return default;
        if (toStreamEnd) return _table.AsSpan((int)offset);
        if (offset + length > _table.Length)
        {
            length = _table.Length - offset;
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2304",
                $"The {table} table runs past the end of the table stream and has been truncated.",
                new DiagnosticLocation(_fib.UsesTable1Stream ? "1Table" : "0Table", offset)));
        }

        return _table.AsSpan((int)offset, (int)length);
    }

}

/// <summary>A range of character positions.</summary>
/// <param name="Start">The first position in the range.</param>
/// <param name="End">One past the last position in the range.</param>
public readonly record struct Ww8Range(int Start, int End)
{
    /// <summary>How many positions the range covers.</summary>
    public int Length => Math.Max(0, End - Start);
}

/// <summary>The character-position extents of a document's subdocuments.</summary>
/// <param name="Body">The main text.</param>
/// <param name="Footnotes">Every footnote's text, end to end.</param>
/// <param name="Headers">The header, footer and note-separator stories.</param>
/// <param name="Macros">Macro text, which Paperless never reads as content.</param>
/// <param name="Comments">Every comment's text, end to end.</param>
/// <param name="Endnotes">Every endnote's text, end to end.</param>
/// <param name="TextBoxes">Text-box text.</param>
/// <param name="HeaderTextBoxes">Text-box text belonging to headers and footers.</param>
public readonly record struct Ww8Ranges(
    Ww8Range Body,
    Ww8Range Footnotes,
    Ww8Range Headers,
    Ww8Range Macros,
    Ww8Range Comments,
    Ww8Range Endnotes,
    Ww8Range TextBoxes,
    Ww8Range HeaderTextBoxes);

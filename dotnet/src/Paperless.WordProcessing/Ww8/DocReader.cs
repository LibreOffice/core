using Paperless.Containers;
using Paperless.Containers.Ole2;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Core.Graphics;
using Paperless.MsBinary.PropertySets;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// Reads legacy binary Word documents: <c>doc</c> and the <c>dot</c> template, as written by Word 97
/// through Word 2003.
/// </summary>
/// <remarks>
/// <para>
/// A DOC file is an OLE2 compound file whose <c>WordDocument</c> stream begins with the FIB — a
/// versioned table of contents locating everything else. Almost nothing lives in that stream except
/// the text: styles, formatting indexes, the piece table and the note tables all live in a second
/// stream, and <em>which</em> second stream is a bit in the FIB. Reading <c>0Table</c> when the
/// document says <c>1Table</c> finds a stale copy left behind by an earlier save, which parses
/// cleanly and describes a different document.
/// </para>
/// <para>
/// Metadata comes from the OLE property sets rather than from anything Word-specific, so it is read
/// by the shared reader in <c>Paperless.MsBinary</c>
/// (<c>research/05-infrastructure.md</c> section G.3).
/// </para>
/// </remarks>
public static class DocReader
{
    /// <summary>The name of the stream holding the FIB and the document's text.</summary>
    public const string WordDocumentStreamName = "WordDocument";

    /// <summary>Reads a document, leaving the source's stream for the caller to dispose.</summary>
    /// <param name="source">The document to read.</param>
    /// <param name="format">The identified format, recorded on the result.</param>
    /// <exception cref="MalformedDocumentException">
    /// The file is not a Word 97-2003 document: it has no <c>WordDocument</c> stream, or that stream
    /// does not begin with a FIB this reader understands.
    /// </exception>
    /// <exception cref="PasswordRequiredException">The document is encrypted.</exception>
    public static Ww8Document Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        CompoundFile file = CompoundFile.Open(source.Stream, leaveOpen: true);
        try
        {
            List<Diagnostic> diagnostics = [.. file.Diagnostics];

            byte[] wordDocument = ReadStream(file, WordDocumentStreamName)
                ?? throw new MalformedDocumentException(
                    "The compound file has no WordDocument stream, so it is not a Word document.");

            // Parsing the FIB is also the format check: it rejects a stream that does not begin with
            // the WW8 signature, and one written by Word 95 or earlier — which shares the container
            // and the signature but numbers its sprms differently, so it would be misread rather
            // than merely read incompletely.
            Ww8Fib fib = Ww8Fib.Parse(wordDocument);

            if (fib.IsEncrypted)
            {
                throw new PasswordRequiredException(
                    "The document is encrypted.", passwordWasSupplied: false);
            }

            // The FIB names which of the two table streams is current. A document that has been
            // saved repeatedly usually contains both, and the stale one parses without complaint.
            string tableName = fib.UsesTable1Stream ? "1Table" : "0Table";
            byte[] table = ReadStream(file, tableName) ?? [];
            if (table.Length == 0)
            {
                diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Error, "PL2310",
                    $"The document names {tableName} as its table stream, but that stream is "
                    + "missing or empty; formatting and structure cannot be resolved."));
            }

            Ww8DocumentReader reader = new(wordDocument, table, fib, diagnostics);
            ContentDocument content = reader.Read(OlePropertySetReader.Read(file));

            return new Ww8Document(
                format, file, content, diagnostics, reader.Sections, reader);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }

    /// <summary>
    /// The whole of one stream, or null when the compound file has no such stream.
    /// </summary>
    /// <remarks>
    /// Read whole rather than streamed because every WW8 structure is reached by an offset another
    /// structure states: the piece table points into the text stream, the FIB points into the table
    /// stream, and an FKP is found by a page number. None of that can be done over a forward-only
    /// reader.
    /// </remarks>
    private static byte[]? ReadStream(CompoundFile package, string name)
    {
        IPackagePart? part = package.GetPart(name);
        if (part is null) return null;

        using Stream stream = part.Open();
        using MemoryStream buffer = new(part.Length > 0 ? (int)Math.Min(part.Length, int.MaxValue) : 0);
        stream.CopyTo(buffer);
        return buffer.ToArray();
    }
}

/// <summary>A legacy binary Word document that has been read.</summary>
public sealed class Ww8Document : IWordProcessingDocument, IPaginatedDocument
{
    private readonly CompoundFile _file;
    private readonly Ww8DocumentReader _reader;

    internal Ww8Document(
        DocumentFormat format,
        CompoundFile file,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics,
        IReadOnlyList<Model.WritingSection> sections,
        Ww8DocumentReader reader)
    {
        Format = format;
        _file = file;
        _reader = reader;
        Content = content;
        Diagnostics = diagnostics;
        Sections = sections.Count > 0 ? sections : [new Model.WritingSection()];
    }

    /// <inheritdoc/>
    public DocumentFormat Format { get; }

    /// <inheritdoc/>
    public DocumentFamily Family => DocumentFamily.WordProcessing;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => Content.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Model.WritingSection> Sections { get; }

    /// <summary>
    /// The underlying compound file, for callers that need a stream the content tree does not
    /// expose — embedded objects and pictures, above all. Valid until this document is disposed.
    /// </summary>
    public CompoundFile File => _file;

    /// <summary>
    /// Lays the document out into pages.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A paragraph keeps its space-before at the top of a page, which LibreOffice's WW8 importer sets
    /// unconditionally (<c>PARA_SPACE_MAX_AT_PAGES</c> in <c>ww8par.cxx</c>). The two spacings
    /// <em>add</em> rather than the larger winning, which is what a comparison against LibreOffice's own
    /// rendering of a DOC shows — the opposite of the DOCX path, whose flag is absent by default.
    /// </para>
    /// <para>
    /// The document's own answer, read from <c>Dop.fDontUseHTMLAutoSpacing</c> — the flag whose name states
    /// what is switched <em>off</em>, so a document that does not use HTML auto-spacing adds the two
    /// spacings rather than collapsing them. A document with no <c>Dop</c> at all adds them, which is what
    /// every document LibreOffice itself wrote does.
    /// </para>
    /// </remarks>
    public IPageSequence Layout(LayoutOptions? options = null)
    {
        LayoutFonts fonts = new();
        List<PageBlock> blocks = BlocksOf(fonts, _reader.ReadLayoutBlocks());

        PaginationOptions pagination = PaginationOptions.Word with
        {
            CollapsesSpacing = _reader.DocumentProperties.CollapsesSpacing,
            MaxPages = options?.MaxPages is > 0 ? options.MaxPages : PaginationOptions.Word.MaxPages,
        };

        return new WordProcessingPages(
            new Paginator(pagination).Paginate(
                blocks,
                [.. Sections.Select((section, index) =>
                    new PaginatedSection(section, Furniture(fonts, index)))]),
            blocks);
    }

    /// <summary>
    /// The document's headers and footers, ready for the page frames.
    /// </summary>
    /// <remarks>
    /// The same conversion the body goes through, over the six stories the header subdocument holds for this
    /// section — see <c>Ww8DocumentReader.ReadLayoutFurniture</c> for why the order of those stories is the
    /// whole mapping. One font cache is shared with the body, so a header in the body's face resolves to the
    /// identical face object rather than an equal one.
    /// </remarks>
    private PageFurnitureSet? Furniture(LayoutFonts fonts, int section)
    {
        Ww8LayoutFurniture stated = _reader.ReadLayoutFurniture(section);

        Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> headers = [];
        Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> footers = [];

        Fill(headers, stated.Headers);
        Fill(footers, stated.Footers);

        PageFurnitureSet set = new(headers, footers);
        return set.IsEmpty ? null : set;

        void Fill(
            Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> into,
            IReadOnlyDictionary<Model.PageFurnitureSlot,
                List<Ww8DocumentReader.Ww8LayoutParagraph>> from)
        {
            foreach ((Model.PageFurnitureSlot slot,
                      List<Ww8DocumentReader.Ww8LayoutParagraph> stories) in from)
            {
                List<PageParagraph> converted = Convert(fonts, stories);
                if (converted.Count > 0) into[slot] = converted;
            }
        }
    }

    /// <summary>
    /// Turns the recorded blocks into the layout engine's own.
    /// </summary>
    /// <remarks>
    /// A shallow conversion: the column grid, the spans and the vertical merges were resolved during the
    /// read, from the same <c>sprmTDefTable</c> edges the extraction tree used, so all that happens here is
    /// that each paragraph gets its face resolved — the one thing the reader cannot do, having no fonts.
    /// Recursive through a cell's own blocks, which is what makes a nested table convert by the same code as
    /// a table in the body: by this point the assembler has already put each in the right list.
    /// </remarks>
    private static List<PageBlock> BlocksOf(LayoutFonts fonts, IReadOnlyList<Ww8LayoutBlock> stated)
    {
        List<PageBlock> blocks = new(stated.Count);

        foreach (Ww8LayoutBlock block in stated)
        {
            if (block.Paragraph is { } paragraph)
            {
                blocks.AddRange(Convert(fonts, [paragraph])
                    .Select(converted => (PageBlock)(converted with
                    {
                        SectionIndex = paragraph.SectionIndex,
                    })));
                continue;
            }

            if (block.Table is { } table && Grid(fonts, table) is { } grid) blocks.Add(grid);
        }

        return blocks;
    }

    /// <summary>A recorded table as the layout engine's own, or null when it has no usable grid.</summary>
    private static PageTable? Grid(LayoutFonts fonts, Ww8LayoutTable table)
    {
        if (table.ColumnWidths.Count == 0 || table.Rows.Count == 0) return null;

        List<PageTableRow> rows = new(table.Rows.Count);

        foreach (Ww8LayoutRow row in table.Rows)
        {
            List<PageTableCell> cells = new(row.Cells.Count);
            foreach (Ww8LayoutCell cell in row.Cells)
            {
                cells.Add(new PageTableCell
                {
                    Blocks = BlocksOf(fonts, [.. cell.Blocks]),
                    Column = cell.Column,
                    ColumnSpan = cell.ColumnSpan,
                    RowSpan = cell.RowSpan,
                    Padding = cell.Padding,
                });
            }

            rows.Add(new PageTableRow { Cells = cells, IsHeader = row.IsHeader });
        }

        return new PageTable
        {
            SectionIndex = table.SectionIndex,
            ColumnWidths = table.ColumnWidths,
            Rows = rows,
            HeaderRowCount = table.HeaderRowCount,
            LeftIndent = table.LeftIndent,
        };
    }

    /// <summary>Turns recorded paragraphs into the layout engine's own, resolving each one's face.</summary>
    /// <remarks>
    /// A paragraph whose family resolves to nothing at all is dropped: there is nothing to measure it with,
    /// and a machine missing its fonts should fail the comparison tests rather than quietly produce a page
    /// of guesses. Table paragraphs are <em>not</em> filtered here — the reader keeps them inside the table
    /// block they belong to, so the body's list holds none and a cell's list holds nothing else.
    /// </remarks>
    private static List<PageParagraph> Convert(
        LayoutFonts fonts, List<Ww8DocumentReader.Ww8LayoutParagraph> stated)
    {
        List<PageParagraph> paragraphs = new(stated.Count);

        foreach (Ww8DocumentReader.Ww8LayoutParagraph paragraph in stated)
        {
            OpenTypeFace? face = fonts.Face(
                paragraph.FamilyName, paragraph.Weight, paragraph.IsItalic);
            if (face is null) continue;

            paragraphs.Add(new PageParagraph
            {
                Text = paragraph.Text,
                Face = face,
                Font = fonts.Reference(paragraph.FamilyName, paragraph.Weight, paragraph.IsItalic),
                Colour = paragraph.Colour ?? Colour.Black,
                Format = paragraph.Format,
                EmSize = paragraph.Size,
                Language = paragraph.Language,
                Shaping = new Text.Shaping.ShapingOptions(Language: paragraph.Language),
                Runs = RunsOf(fonts, paragraph, face),
                Notes = NotesOf(fonts, paragraph.Notes),
            });
        }

        return paragraphs;
    }

    /// <summary>The notes anchored in a paragraph, with their bodies converted.</summary>
    /// <remarks>
    /// A note whose body converts to nothing is dropped rather than kept empty, because an empty note would
    /// still reserve the separator's room at the foot of the page and so change where the body's text breaks.
    /// </remarks>
    private static List<PageNote> NotesOf(
        LayoutFonts fonts, IReadOnlyList<Ww8LayoutNote>? stated)
    {
        if (stated is null || stated.Count == 0) return [];

        List<PageNote> notes = new(stated.Count);

        foreach (Ww8LayoutNote note in stated)
        {
            List<PageBlock> blocks = BlocksOf(fonts, note.Blocks);
            if (blocks.Count == 0) continue;

            notes.Add(new PageNote
            {
                Blocks = blocks,
                Offset = note.Offset,
                IsEndnote = note.IsEndnote,
            });
        }

        return notes;
    }

    /// <summary>
    /// The paragraph's runs, or nothing when every one of them is the paragraph's own formatting.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Returning an empty list for a uniform paragraph puts plain prose back on the single-face path,
    /// which shapes the whole paragraph in one call. That is not only cheaper: a run boundary breaks the
    /// shaping context, so a paragraph split into runs it does not need loses a kern pair at every
    /// boundary and measures very slightly wide.
    /// </para>
    /// <para>
    /// This is the earliest point at which the question can be answered, because two CHPXs asking for
    /// different families can resolve to the same face — one naming <c>Calibri</c> and the next
    /// <c>Carlito</c> is one face on a Linux machine, and splitting there would be splitting on nothing.
    /// </para>
    /// </remarks>
    private static List<PageRun> RunsOf(
        LayoutFonts fonts,
        Ww8DocumentReader.Ww8LayoutParagraph paragraph,
        OpenTypeFace paragraphFace)
    {
        IReadOnlyList<Ww8DocumentReader.Ww8LayoutRun> stated = paragraph.Runs ?? [];
        List<PageRun> runs = new(stated.Count);
        bool varies = false;

        foreach (Ww8DocumentReader.Ww8LayoutRun run in stated)
        {
            OpenTypeFace face =
                fonts.Face(run.FamilyName, run.Weight, run.IsItalic) ?? paragraphFace;

            // The escapement is resolved here rather than where it was read, because its rise is a
            // fraction of the face's height and the face is only known now.
            Core.Units.Length size = run.Escapement.SizeOf(run.Size);
            Core.Units.Length rise = run.Escapement.RiseOf(face, run.Size);

            if (face != paragraphFace
                || size != paragraph.Size
                || run.Colour != paragraph.Colour
                || run.Language != paragraph.Language
                || rise != Core.Units.Length.Zero)
            {
                varies = true;
            }

            runs.Add(new PageRun(
                run.Start,
                run.Length,
                face,
                size,
                fonts.Reference(run.FamilyName, run.Weight, run.IsItalic),
                run.Colour ?? paragraph.Colour ?? Colour.Black,
                new Text.Shaping.ShapingOptions(Language: run.Language),
                rise));
        }

        return varies ? runs : [];
    }

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}

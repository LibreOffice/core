using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Core.Graphics;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Rtf;

/// <summary>Reads Rich Text Format documents.</summary>
/// <remarks>
/// RTF is the one word-processing format with no container: the whole document is one byte stream
/// of nested groups. So there is no package to open and no parts to resolve — the reader is the
/// tokeniser plus the state machine in <see cref="RtfDocumentReader"/>.
/// </remarks>
public static class RtfReader
{
    /// <summary>
    /// The largest RTF document Paperless will buffer.
    /// </summary>
    /// <remarks>
    /// RTF is read whole rather than streamed, because its state machine can be sent back to an
    /// earlier destination at any point and because the format is verbose enough that a large
    /// document is still modest in memory. The cap bounds what a hostile input can allocate.
    /// </remarks>
    public const long MaxDocumentBytes = 512L * 1024 * 1024;

    /// <summary>Reads a document, leaving the source's stream for the caller to dispose.</summary>
    public static RtfDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        byte[] data = ReadAll(source.Stream);
        if (!LooksLikeRtf(data))
        {
            throw new MalformedDocumentException(
                "The document does not begin with '{\\rtf', so it is not Rich Text Format.");
        }

        List<Diagnostic> diagnostics = [];
        RtfDocumentReader reader = new(data, diagnostics);
        ContentDocument content = reader.Read();

        return new RtfDocument(
            format, content, diagnostics, reader.Sections, reader.LayoutBlocks,
            reader.HeaderLayout, reader.FooterLayout);
    }

    /// <summary>
    /// True when the bytes start with an RTF signature, allowing for leading whitespace and a
    /// byte-order mark.
    /// </summary>
    /// <remarks>
    /// Some producers write a UTF-8 BOM before the signature even though RTF is not UTF-8, and
    /// mail systems add leading blank lines. Neither makes the document unreadable, so neither is
    /// treated as fatal.
    /// </remarks>
    private static bool LooksLikeRtf(ReadOnlySpan<byte> data)
    {
        if (data.Length >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF) data = data[3..];
        while (data.Length > 0 && data[0] is (byte)' ' or (byte)'\t' or (byte)'\r' or (byte)'\n')
            data = data[1..];

        return data.StartsWith("{\\rtf"u8);
    }

    private static byte[] ReadAll(Stream stream)
    {
        if (stream.CanSeek)
        {
            if (stream.Length - stream.Position > MaxDocumentBytes)
            {
                throw new MalformedDocumentException(
                    $"The document is larger than the {MaxDocumentBytes / (1024 * 1024)} MB "
                    + "Paperless will buffer for RTF.");
            }

            byte[] exact = new byte[stream.Length - stream.Position];
            stream.ReadExactly(exact);
            return exact;
        }

        using MemoryStream buffer = new();
        stream.CopyTo(buffer);
        return buffer.ToArray();
    }
}

/// <summary>An RTF document that has been read.</summary>
public sealed class RtfDocument : IWordProcessingDocument, IPaginatedDocument
{
    private readonly IReadOnlyList<RtfLayoutBlock> _layoutBlocks;
    private readonly
        IReadOnlyDictionary<(int Section, Model.PageFurnitureSlot Slot), IReadOnlyList<RtfLayoutBlock>>
        _headerLayout;

    private readonly
        IReadOnlyDictionary<(int Section, Model.PageFurnitureSlot Slot), IReadOnlyList<RtfLayoutBlock>>
        _footerLayout;

    internal RtfDocument(
        DocumentFormat format,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics,
        IReadOnlyList<Model.WritingSection> sections,
        IReadOnlyList<RtfLayoutBlock> layoutBlocks,
        IReadOnlyDictionary<(int Section, Model.PageFurnitureSlot Slot), IReadOnlyList<RtfLayoutBlock>>
            headerLayout,
        IReadOnlyDictionary<(int Section, Model.PageFurnitureSlot Slot), IReadOnlyList<RtfLayoutBlock>>
            footerLayout)
    {
        Format = format;
        Content = content;
        Diagnostics = diagnostics;
        Sections = sections.Count > 0 ? sections : [new Model.WritingSection()];
        _layoutBlocks = layoutBlocks;
        _headerLayout = headerLayout;
        _footerLayout = footerLayout;
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
    /// Lays the document out into pages.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The paragraphs were collected during the read rather than by a second pass, because RTF is a token
    /// stream with nothing to revisit — see <see cref="RtfLayoutParagraph"/>.
    /// </para>
    /// <para>
    /// The two spacings add rather than the larger winning. LibreOffice's RTF importer turns HTML
    /// auto-spacing on by default — "opt-in for RTF, opt-out for OOXML", as its own comment in
    /// <c>SettingsTable.cxx</c> puts it — which is the opposite of the DOCX path and the same as ODF's.
    /// </para>
    /// </remarks>
    public IPageSequence Layout(LayoutOptions? options = null)
    {
        LayoutFonts fonts = new();
        List<PageBlock> blocks = [];

        foreach (RtfLayoutBlock block in _layoutBlocks)
        {
            blocks.AddRange(Convert(fonts, block));
        }

        PaginationOptions pagination = PaginationOptions.Word with
        {
            CollapsesSpacing = false,
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
    /// Turns a recorded table into the layout engine's own, or null when it has no usable grid.
    /// </summary>
    /// <remarks>
    /// A shallow conversion: the grid and the spans were resolved during the read, from the same
    /// <c>\cellx</c> edges the extraction tree used, so all that happens here is that each cell's
    /// paragraphs get their faces resolved — the one thing the reader cannot do, since it has no fonts.
    /// </remarks>
    private static PageTable? Grid(LayoutFonts fonts, RtfLayoutTable table)
    {
        if (table.ColumnWidths.Count == 0 || table.Rows.Count == 0) return null;

        List<PageTableRow> rows = new(table.Rows.Count);
        foreach (RtfLayoutRow row in table.Rows)
        {
            List<PageTableCell> cells = new(row.Cells.Count);
            foreach (RtfLayoutCell cell in row.Cells)
            {
                cells.Add(new PageTableCell
                {
                    Blocks = Convert(fonts, cell.Blocks),
                    Column = cell.Column,
                    ColumnSpan = cell.ColumnSpan,
                    RowSpan = cell.RowSpan,
                    Padding = cell.Padding,
                    VerticalAlignment = cell.VerticalAlignment,
                });
            }

            rows.Add(new PageTableRow
            {
                Cells = cells,
                MinHeight = row.MinHeight,
                IsHeader = row.IsHeader,
            });
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

    /// <summary>
    /// The document's headers and footers, ready for the page frames.
    /// </summary>
    /// <remarks>
    /// The same conversion the body goes through, over the paragraphs the read collected per slot. One
    /// font cache is shared with the body's conversion, so a header in the body's face costs no second
    /// lookup — and, more to the point, resolves to the identical face object, which is what lets the
    /// measurement caches downstream see them as one font.
    /// </remarks>
    private PageFurnitureSet? Furniture(LayoutFonts fonts, int section)
    {
        Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> headers = [];
        Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> footers = [];

        Fill(headers, _headerLayout);
        Fill(footers, _footerLayout);

        PageFurnitureSet set = new(headers, footers);
        return set.IsEmpty ? null : set;

        // A section with no header of its own inherits the previous section's, which is what RTF means by
        // saying nothing: \sectd resets the geometry and leaves the running heads alone, so a document that
        // writes one header and three \sect marks has that header on all four sections.
        void Fill(
            Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> into,
            IReadOnlyDictionary<(int Section, Model.PageFurnitureSlot Slot), IReadOnlyList<RtfLayoutBlock>>
                from)
        {
            foreach (Model.PageFurnitureSlot slot in Enum.GetValues<Model.PageFurnitureSlot>())
            {
                for (int at = section; at >= 0; at--)
                {
                    if (!from.TryGetValue((at, slot), out IReadOnlyList<RtfLayoutBlock>? stated)) continue;

                    List<PageBlock> converted = Convert(fonts, stated);
                    if (converted.Count > 0) into[slot] = converted;
                    break;
                }
            }
        }
    }

    /// <summary>Turns recorded paragraphs into the layout engine's own, resolving each one's face.</summary>
    /// <remarks>
    /// A paragraph whose family resolves to nothing at all is dropped rather than drawn in a fallback,
    /// because there is nothing to measure it with — and a machine with no fonts installed should fail the
    /// comparison tests rather than quietly produce a page of guesses.
    /// </remarks>
    /// <summary>
    /// Turns recorded blocks into the layout engine's own, recursing into nested tables.
    /// </summary>
    /// <remarks>
    /// One conversion for the body and for a cell's contents, which is what lets a table inside a cell be
    /// converted by the same code as a table in the body — RTF distinguishes them only by <c>\itap</c>
    /// depth, and by this point that has already put each table in the right list.
    /// </remarks>
    private static List<PageBlock> Convert(LayoutFonts fonts, IReadOnlyList<RtfLayoutBlock> stated)
    {
        List<PageBlock> blocks = new(stated.Count);
        foreach (RtfLayoutBlock block in stated) blocks.AddRange(Convert(fonts, block));
        return blocks;
    }

    /// <summary>One block, which yields nothing when its family resolves to no face at all.</summary>
    private static IEnumerable<PageBlock> Convert(LayoutFonts fonts, RtfLayoutBlock block)
    {
        if (block.Paragraph is { } paragraph)
        {
            return Convert(fonts, [paragraph])
                .Select(converted => (PageBlock)(converted with
                {
                    SectionIndex = paragraph.SectionIndex,
                }));
        }

        return block.Table is { } table && Grid(fonts, table) is { } grid
            ? [grid]
            : [];
    }

    private static List<PageParagraph> Convert(
        LayoutFonts fonts, List<RtfLayoutParagraph> stated)
    {
        List<PageParagraph> paragraphs = new(stated.Count);

        foreach (RtfLayoutParagraph paragraph in stated)
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
    private static List<PageNote> NotesOf(LayoutFonts fonts, IReadOnlyList<RtfLayoutNote>? stated)
    {
        if (stated is null || stated.Count == 0) return [];

        List<PageNote> notes = new(stated.Count);

        foreach (RtfLayoutNote note in stated)
        {
            List<PageBlock> blocks = Convert(fonts, note.Blocks);
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
    /// This is also the earliest point at which the question can be answered, because two runs asking for
    /// different families can resolve to the same face — an RTF naming <c>Calibri</c> in one run and
    /// <c>Carlito</c> in the next is one face on a Linux machine, and splitting there would be splitting
    /// on nothing.
    /// </para>
    /// </remarks>
    private static List<PageRun> RunsOf(
        LayoutFonts fonts, RtfLayoutParagraph paragraph, OpenTypeFace paragraphFace)
    {
        IReadOnlyList<RtfLayoutRun> stated = paragraph.Runs ?? [];
        List<PageRun> runs = new(stated.Count);
        bool varies = false;

        foreach (RtfLayoutRun run in stated)
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
    /// <remarks>
    /// Nothing to release: RTF has no container, and the document was read from a buffer the
    /// caller owns.
    /// </remarks>
    public void Dispose() { }
}

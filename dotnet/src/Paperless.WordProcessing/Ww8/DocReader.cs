using Paperless.Containers;
using Paperless.Containers.Ole2;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
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

    /// <summary>The optional stream a document's pictures live in when it has one.</summary>
    public const string PictureStreamName = "Data";

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

            // "Little joke from Microsoft: sometimes a stream named DATA exists", says
            // SwWW8ImplReader::ImportGraf — and when it does, every picture's PICF and bytes are in
            // it rather than in WordDocument, at the same offset sprmCPicLocation states. A reader
            // that looks in the main stream regardless finds whatever happens to sit at that offset.
            byte[]? data = ReadStream(file, PictureStreamName);

            Ww8DocumentReader reader =
                new(wordDocument, table, fib, diagnostics, data, source.FileName);
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
        Marks = reader.Marks;
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

    /// <inheritdoc/>
    public Model.WritingMarks Marks { get; }

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
        // A document asking for printer metrics is measured on the printer's pixel grid, which rounds every
        // font metric and is worth up to 2.8% of a line's height. See Ww8DocumentProperties.UsesPrinterMetrics.
        LayoutFonts fonts = new()
        {
            Metrics = _reader.DocumentProperties.UsesPrinterMetrics ? MetricGrid.Printer : null,
        };

        List<PageBlock> blocks = BlocksOf(fonts, _reader.ReadLayoutBlocks(), TextWidths());

        PaginationOptions pagination = PaginationOptions.Word with
        {
            CollapsesSpacing = _reader.DocumentProperties.CollapsesSpacing,
            MaxPages = options?.MaxPages is > 0 ? options.MaxPages : PaginationOptions.Word.MaxPages,
        };

        Paginator paginator = new(pagination);

        List<PaginatedSection> sections = new(Sections.Count);
        Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> headers = [];
        Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> footers = [];

        for (int i = 0; i < Sections.Count; i++)
        {
            sections.Add(new PaginatedSection(Sections[i], Furniture(fonts, i, headers, footers)));
        }

        return new WordProcessingPages(
            paginator.Paginate(blocks, sections),
            paginator.Blocks ?? blocks);
    }

    /// <summary>
    /// The document's headers and footers, ready for the page frames.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same conversion the body goes through, over the six stories the header subdocument holds for this
    /// section — see <c>Ww8DocumentReader.ReadLayoutFurniture</c> for why the order of those stories is the
    /// whole mapping. One font cache is shared with the body, so a header in the body's face resolves to the
    /// identical face object rather than an equal one.
    /// </para>
    /// <para>
    /// A slot whose story is empty is <em>inherited</em> from the section before it, not dropped. That is
    /// what "link to previous" is on the wire: Word writes a section's header story only when the section
    /// unlinks it, so in a document where one running head covers twenty sections nineteen of the stories
    /// are a bare paragraph mark. LibreOffice reaches the same place through
    /// <c>SwWW8ImplReader::CopyPageDescHdFt</c>, reached from <c>Read_HdFt</c> whenever the story is
    /// shorter than two characters (<c>ww8par.cxx</c>). Dropping the slot instead leaves every section
    /// after the first with no running head at all, and a page with no header is a page that holds more
    /// lines — so the document silently paginates short.
    /// </para>
    /// </remarks>
    /// <param name="fonts">The font cache shared with the body.</param>
    /// <param name="section">Which section's furniture to read.</param>
    /// <param name="headers">
    /// The headers in force, by slot, carried across the sections and updated in place: what this section
    /// states replaces a slot, and what it leaves empty keeps whatever the section before it had.
    /// </param>
    /// <param name="footers">The footers in force, by the same rule.</param>
    private PageFurnitureSet? Furniture(
        LayoutFonts fonts,
        int section,
        Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> headers,
        Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> footers)
    {
        Ww8LayoutFurniture stated = _reader.ReadLayoutFurniture(section);
        IReadOnlyList<Length> widths = TextWidths();

        Fill(headers, stated.Headers);
        Fill(footers, stated.Footers);

        PageFurnitureSet set = new(headers, footers);
        return set.IsEmpty ? null : set;

        void Fill(
            Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> into,
            IReadOnlyDictionary<Model.PageFurnitureSlot, List<Ww8LayoutBlock>> from)
        {
            foreach ((Model.PageFurnitureSlot slot, List<Ww8LayoutBlock> stories) in from)
            {
                List<PageBlock> converted = BlocksOf(fonts, stories, widths);
                if (converted.Count > 0) into[slot] = converted;
            }
        }
    }

    /// <summary>
    /// Each section's text width, which is the only thing an auto-width text frame has to go on.
    /// </summary>
    /// <remarks>
    /// A frame whose <c>sprmPDxaWidth</c> is ten or less is as wide as the text it sits beside, and
    /// <c>WW8SwFlyPara</c> takes that from <c>m_aSectionManager.GetTextAreaWidth()</c>
    /// (<c>sw/source/filter/ww8/ww8par6.cxx:1953</c>). Indexed by section because a document can change
    /// its margins part-way and the frames after the change follow the new ones.
    /// </remarks>
    private List<Length> TextWidths()
    {
        List<Length> widths = new(Sections.Count);
        foreach (Model.WritingSection section in Sections) widths.Add(section.Page.ColumnWidth);
        return widths;
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
    /// <param name="fonts">The shared font cache.</param>
    /// <param name="stated">The blocks as the reader recorded them.</param>
    /// <param name="textWidths">
    /// Each section's text width, for the one thing that needs it — an auto-width text frame. Empty
    /// inside a cell, where a frame is not read in the first place.
    /// </param>
    private static List<PageBlock> BlocksOf(
        LayoutFonts fonts,
        IReadOnlyList<Ww8LayoutBlock> stated,
        IReadOnlyList<Length>? textWidths = null)
    {
        List<PageBlock> blocks = new(stated.Count);

        foreach (Ww8LayoutBlock block in stated)
        {
            if (block.Paragraph is { } paragraph)
            {
                blocks.AddRange(Convert(fonts, [paragraph], textWidths)
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
                    Shading = cell.Shading,
                    Borders = cell.Borders,
                });
            }

            rows.Add(new PageTableRow
            {
                Cells = cells,
                IsHeader = row.IsHeader,
                MinHeight = row.MinHeight,
                HasExactHeight = row.HasExactHeight,
                CanSplit = row.CanSplit,
            });
        }

        return new PageTable
        {
            SectionIndex = table.SectionIndex,
            ColumnWidths = table.ColumnWidths,
            Rows = rows,
            HeaderRowCount = table.HeaderRowCount,
            LeftIndent = table.LeftIndent,

            // Every DOC is a Word document by definition, so the flag is not read from anything: it is
            // what LibreOffice's own filter sets on import, and it changes where the inner grid lines
            // stop. Measured on the corpus table — the reference's inner horizontals run 56.95 to 538.35
            // where the same table in ODF runs 56.45 to 538.85.
            JoinsBordersLikeWord = true,
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
        LayoutFonts fonts,
        List<Ww8DocumentReader.Ww8LayoutParagraph> stated,
        IReadOnlyList<Length>? textWidths = null)
    {
        List<PageParagraph> paragraphs = new(stated.Count);

        foreach (Ww8DocumentReader.Ww8LayoutParagraph paragraph in stated)
        {
            OpenTypeFace? face = fonts.Face(
                paragraph.FamilyName, paragraph.Weight, paragraph.IsItalic);
            if (face is null) continue;

            FontReference? font = fonts.Reference(
                paragraph.FamilyName, paragraph.Weight, paragraph.IsItalic);

            // The runs first, then the text they map: `Apply` rewrites both together, and the offsets it
            // preserves are the ones the notes and frames below were recorded against.
            List<PageRun> runs = RunsOf(fonts, paragraph, face);

            paragraphs.Add(new PageParagraph
            {
                Text = CaseMapping.Apply(paragraph.Text, runs),
                Face = face,
                Font = font,
                Colour = paragraph.Colour ?? Colour.Black,
                Format = paragraph.Format,
                Label = Label(paragraph, face, font),
                EmSize = paragraph.Size,
                Language = paragraph.Language,
                Shaping = new Text.Shaping.ShapingOptions(
                    Language: paragraph.Language, DisableKerning: !paragraph.AutoKerning),
                Metrics = fonts.Metrics,

                // #i3952#, which the WW8 importer turns on for every DOC without asking the file
                // (`ww8par.cxx`:2041). See PageParagraph.BlanksAreTransparentToHeight.
                BlanksAreTransparentToHeight = true,
                Runs = runs,
                Notes = NotesOf(fonts, paragraph.Notes),
                Frames = FramesOf(fonts, paragraph.Frames, paragraph.TextFrames, WidthFor(paragraph)),
            });
        }

        return paragraphs;

        Length WidthFor(Ww8DocumentReader.Ww8LayoutParagraph paragraph)
            => textWidths is { Count: > 0 }
                ? textWidths[Math.Clamp(paragraph.SectionIndex, 0, textWidths.Count - 1)]
                : Length.Zero;
    }

    /// <summary>
    /// The label a list item draws, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// In the item's own face, at the level's own size. WW8 states a label's character formatting in the
    /// level's <c>grpprlChpx</c>, and the two halves of it are treated differently on purpose: the
    /// <em>font</em> is not read, because the only thing it usually carries is a symbol face for a bullet
    /// whose code point <see cref="Ww8Numbering"/> has already normalised to U+2022 — keeping it would
    /// draw a real bullet through a face with no glyph for it — while the <em>size</em> is, because it
    /// survives that normalisation and Word writes it constantly. A level a size larger than its items
    /// makes their first lines taller; see <see cref="PageParagraph.LabelRaisesFirstLine"/>.
    /// </para>
    /// <para>
    /// The follower and its stop come from the level's <c>ixchFollow</c> and its <c>grpprlPapx</c>, which
    /// is where Word states them. Assuming <see cref="LabelFollow.Nothing"/> instead worked only for the
    /// documents that also repeat the level's geometry on every list paragraph — Word usually does, and
    /// the ones that do not drew the label touching the item's first word.
    /// </para>
    /// <para>
    /// The paragraph's own reference travels with it, because the label is set in the paragraph's
    /// own face and a label that names only a family embeds no font program: the reference's
    /// <c>FaceKey</c> is the font file's path, and it is the only thing that can be turned back
    /// into bytes. Without it <c>word-features.doc</c> rendered with its list labels'
    /// <c>LiberationSerif</c> reported <c>emb no</c> by <c>pdffonts</c> while every body face in
    /// the same PDF reported <c>emb yes</c>.
    /// </para>
    /// </remarks>
    private static PageLabel? Label(
        Ww8DocumentReader.Ww8LayoutParagraph paragraph, OpenTypeFace face, FontReference? font)
        => paragraph.ListMarker is { Length: > 0 } marker
            ? PageLabel.Measured(
                marker, face,
                paragraph.ListLabelSize > Core.Units.Length.Zero
                    ? paragraph.ListLabelSize
                    : paragraph.Size,
                new Text.Shaping.ShapingOptions(
                    Language: paragraph.Language, DisableKerning: !paragraph.AutoKerning)) with
            {
                Font = font,
                Colour = paragraph.Colour ?? Colour.Black,
                Follow = paragraph.ListFollow switch
                {
                    1 => LabelFollow.Space,
                    2 => LabelFollow.Nothing,
                    _ => LabelFollow.ListTab,
                },
                TabStop = Core.Units.Length.FromTwips(paragraph.ListTabStop),
            }
            : null;

    /// <summary>
    /// The floating shapes anchored in a paragraph, as frames the layout engine can place.
    /// </summary>
    /// <remarks>
    /// The conversion itself is <see cref="Ww8Frames.Build"/>'s; all that happens here is that the
    /// shape's own text gets its faces resolved, which is the one thing the reader cannot do. A shape
    /// that <see cref="Ww8Frames.Build"/> declines — one with no area, one deleted, one hidden — is
    /// dropped rather than placed empty, since an empty frame would still make a hole in the text.
    /// </remarks>
    /// <param name="fonts">The shared font cache.</param>
    /// <param name="stated">The drawings anchored in the paragraph, or null when it anchors none.</param>
    /// <param name="textFrames">
    /// The Word text frames it anchors, or null when it anchors none. A different mechanism entirely —
    /// see <see cref="Ww8TextFramePosition"/> — and the two produce the same kind of frame, so they are
    /// converted together and the layout engine sees one list.
    /// </param>
    /// <param name="textWidth">The section's text width, for an auto-width text frame.</param>
    private static List<PageFrame> FramesOf(
        LayoutFonts fonts,
        IReadOnlyList<Ww8LayoutFrame>? stated,
        IReadOnlyList<Ww8LayoutTextFrame>? textFrames = null,
        Length textWidth = default)
    {
        if ((stated is null || stated.Count == 0)
            && (textFrames is null || textFrames.Count == 0))
        {
            return [];
        }

        List<PageFrame> frames = new((stated?.Count ?? 0) + (textFrames?.Count ?? 0));

        foreach (Ww8LayoutTextFrame frame in textFrames ?? [])
        {
            List<PageBlock> blocks = BlocksOf(fonts, frame.Blocks);
            if (blocks.Count == 0) continue;
            if (Ww8TextFrames.Build(frame, blocks, textWidth) is { } placed) frames.Add(placed);
        }

        foreach (Ww8LayoutFrame frame in stated ?? [])
        {
            // An inline picture has no origin to be placed against and no wrap to obey: it hangs on
            // the line where its anchor character sits. So it skips Ww8Frames.Build entirely, whose
            // whole subject is the FSPA an inline picture does not have.
            if (frame.IsInline)
            {
                frames.Add(new PageFrame
                {
                    Size = new Core.Geometry.DocSize(
                        Core.Units.Length.FromTwips(frame.Anchor.Width),
                        Core.Units.Length.FromTwips(frame.Anchor.Height)),
                    Anchor = FrameAnchor.AsCharacter,
                    AnchorOffset = frame.Offset,
                    Wrap = TextWrap.Through,
                    IsImage = true,
                    Image = frame.Picture.Raster,
                    Vector = frame.Picture.Vector,
                });

                continue;
            }

            PageFrame? built = Ww8Frames.Build(
                frame.Anchor, frame.Shape, frame.Offset, BlocksOf(fonts, frame.Blocks),
                frame.IsSetInLine);
            if (built is not null)
            {
                frames.Add(built with { Image = frame.Picture.Raster, Vector = frame.Picture.Vector });
            }
        }

        return frames;
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
                Placement = note.Placement,
                Restart = note.Restart,
                Numbering = note.Numbering,

                // WW8's copy of the number is the U+0002 at the head of the note's own range, so its offset
                // is nought — the same place ODF's and RTF's end up, by a third route.
                Citation = note.Citation,
                BodyOffset = 0,
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
                || rise != Core.Units.Length.Zero
                // A case map has to survive the uniform-paragraph shortcut: it is the one property here
                // that changes the *characters*, so dropping the runs would draw the text as stored.
                || run.CaseMap != PageCaseMap.None
                // So does a highlight, and for the same reason read the other way: the paragraph carries
                // no highlight of its own, so a paragraph highlighted end to end is uniform by every other
                // test and would lose its band entirely.
                || run.Highlight is not null
                // And so do the two rules, for the same reason: neither changes a width, so a paragraph
                // underlined end to end is uniform by every measurement test and would be drawn plain.
                || run.IsUnderlined
                || run.IsStruckThrough
                // Kerning, unlike the two rules, does change a measurement — so a run that kerns
                // inside a paragraph that does not has to survive the shortcut or its width is the
                // paragraph's answer rather than its own.
                || run.AutoKerning != paragraph.AutoKerning)
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
                new Text.Shaping.ShapingOptions(
                    Language: run.Language, DisableKerning: !run.AutoKerning),
                rise,
                run.CaseMap,
                Highlight: run.Highlight ?? default,
                IsUnderlined: run.IsUnderlined,
                IsStruckThrough: run.IsStruckThrough));
        }

        return varies ? runs : [];
    }

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}

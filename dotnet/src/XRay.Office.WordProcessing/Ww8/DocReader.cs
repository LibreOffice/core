using System.Text;
using XRay.Office.Containers;
using XRay.Office.Containers.Ole2;
using XRay.Office.Core;
using XRay.Office.Core.Diagnostics;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Formats;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.MsBinary.PropertySets;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;

namespace XRay.Office.WordProcessing.Ww8;

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
/// by the shared reader in <c>XRay.Office.MsBinary</c>
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
        // Every document is measured on a device's pixel grid; the flag only chooses which. A document
        // asking for printer metrics gets the printer's, which rounds every font metric and is worth up to
        // 1.5% of a line's height; everything else gets Writer's 8640 dpi virtual reference device, worth
        // one twip. See Ww8DocumentProperties.UsesPrinterMetrics and MetricGrid.
        LayoutFonts fonts = new()
        {
            // `AsWordDocument` is the MS_WORD_COMP_GRID_METRICS compatibility flag, which a Word
            // binary always carries: a face declaring an East Asian code page has its ascent and its
            // ascent-plus-descent scaled by 127%. See MetricGrid.AsWordDocument.
            Metrics = (_reader.DocumentProperties.UsesPrinterMetrics
                ? MetricGrid.Printer
                : MetricGrid.Reference).AsWordDocument(),

            // The FFN's own pitch and family bits, which decide the fallback for a family that is
            // neither installed nor substitutable. See Ww8FontTable.ShapeOf.
            DeclaredShapes = _reader.Fonts.ShapeOf,
        };

        List<PageBlock> blocks = BlocksOf(fonts, _reader.ReadLayoutBlocks(), TextWidths());

        PaginationOptions pagination = PaginationOptions.Word with
        {
            CollapsesSpacing = _reader.DocumentProperties.CollapsesSpacing,

            // `SwWW8ImplReader::ReadDocInfo` sets `CONTINUOUS_ENDNOTES` on every DOC it opens
            // (`ww8par.cxx`:2050), as `WriterFilter` does for every DOCX — and as neither the RTF
            // filter nor either ODF one does. See PaginationOptions.UsesWordNoteSeparator.
            UsesWordNoteSeparator = true,

            // The other half of that same setting, and the half it is named for: endnotes collecting at
            // the end of the document follow the last body content instead of taking pages of their own.
            // It changes nothing for a DOC whose `epc` is zero, whose endnotes collect at the end of the
            // section instead. See PaginationOptions.EndnotesFollowTheBody.
            EndnotesFollowTheBody = true,

            NoteSeparatorHeight = NoteReservation(fonts) is { } reservation
                ? reservation
                : PaginationOptions.Word.NoteSeparatorHeight,

            MaxPages = options?.MaxPages is > 0 ? options.MaxPages : PaginationOptions.Word.MaxPages,
        };

        Paginator paginator = new(pagination);

        List<PaginatedSection> sections = new(Sections.Count);
        FurnitureCarry carry = new();

        for (int i = 0; i < Sections.Count; i++)
        {
            PageFurnitureSet? furniture = Furniture(fonts, i, carry);
            sections.Add(new PaginatedSection(
                Sections[i], furniture, carry.StatesOwnFurniture, SideMarginsAreASectionIndent: true));
        }

        return new WordProcessingPages(
            paginator.Paginate(blocks, sections),
            paginator.Blocks ?? blocks);
    }

    /// <summary>
    /// The room Word leaves above a page's notes, or null when the default style's face cannot be read.
    /// </summary>
    /// <remarks>
    /// The default paragraph style's line height, measured on the same faces and the same device grid
    /// every other run in the document is measured on — see
    /// <see cref="PaginationOptions.NoteSeparatorHeight"/> for why it is that style and not the note's.
    /// Null rather than zero for an unreadable face, so that the caller falls back to Writer's fixed
    /// reservation instead of reserving nothing at all.
    /// </remarks>
    private Length? NoteReservation(LayoutFonts fonts)
    {
        (string? family, Length size, int weight, bool italic) = _reader.DefaultStyleFont;

        return fonts.Face(family, weight, italic) is { } face
            ? LineSpacing.Resolve(face, fonts.Metrics, WriterLineBox.LeadingAboveText)
                .ScaledLineHeight(size)
            : null;
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
    /// <para>
    /// <em>Inherited from the section before it</em> is the whole of the rule and not a paraphrase of it.
    /// <c>CopyPageDescHdFt(pPrev, …)</c> copies out of <c>pPrevious-&gt;mpPage</c>, the immediately
    /// preceding <em>segment's page descriptor</em> — and a continuous section never gets one, because
    /// <c>InsertSegments</c> turns it into a Writer text section instead (<c>ww8par.cxx</c>:4422). So when
    /// the section before this one is continuous the copy source is null, the <c>else if (pPrev)</c> arm
    /// never runs, and the slot keeps the empty format <c>SwFormatHeader(true)</c> left there: the running
    /// head <em>stops</em>, rather than reaching back past the continuous section to the last one that had
    /// a page of its own. Carrying it further is how the FAA advisory circulars grew a running head on
    /// every page that should have had none.
    /// </para>
    /// <para>
    /// A slot also stops being inherited at all once <c>grpfIhdt</c> loses its bit, which
    /// <c>InsertSegments</c> clears for an empty story whose predecessor's bit was already clear
    /// (<c>ww8par6.cxx</c>:1237-1258). That is why this works off the six story lengths rather than off
    /// what the dictionaries happen to hold: a slot missing from them may have had no story, or a story
    /// whose blocks all came out empty, and only the first of those two ends the inheritance.
    /// </para>
    /// </remarks>
    /// <param name="fonts">The font cache shared with the body.</param>
    /// <param name="section">Which section's furniture to read.</param>
    /// <param name="carry">
    /// What the section before this one left in force, updated in place: the furniture of the last section
    /// that had a page descriptor, its <c>grpfIhdt</c>, and whether it was the immediately preceding
    /// section — which is what decides whether this one may copy from it at all.
    /// </param>
    private PageFurnitureSet? Furniture(LayoutFonts fonts, int section, FurnitureCarry carry)
    {
        Ww8LayoutFurniture stated = _reader.ReadLayoutFurniture(section);
        IReadOnlyList<Length> widths = TextWidths();

        // `grpfIhdt` as `wwSectionManager::InsertSegments` synthesises it for Word 8 and later: every
        // section states an odd and a first pair, and an even pair as well under `fFacingPages`; a bit
        // whose story is empty survives only where the section before it had that bit on.
        int on = HeaderOddBit | FooterOddBit | HeaderFirstBit | FooterFirstBit;
        if (_reader.DocumentProperties.HasFacingPages) on |= HeaderEvenBit | FooterEvenBit;

        for (int slot = 0; slot < FurnitureStoryCount; slot++)
        {
            int bit = 1 << slot;
            if ((on & bit) == 0) continue;
            if (StoryLength(stated, slot) > 0) continue;
            if (section > 0 && (carry.Enabled & bit) != 0) continue;
            on &= ~bit;
        }

        // A continuous section becomes a Writer *text* section rather than a page descriptor, so the one
        // in force stays in force and its own stories are never read — Word writes a full set into every
        // section descriptor whether or not the section can use them. The exception `InsertSegments`
        // makes for itself is `HasOwnHeaderFooter`: a continuous section that states a running head of
        // its own does get a descriptor built, on the not-first slots alone (`ww8par.cxx`:4528).
        bool continuous = section > 0 && Sections[section].Break == Model.SectionBreak.Continuous;
        bool ownFurniture =
            StoryLength(stated, 0) >= 2 || StoryLength(stated, 1) >= 2
            || StoryLength(stated, 2) >= 2 || StoryLength(stated, 3) >= 2;

        carry.StatesOwnFurniture = ownFurniture;

        if (continuous && !ownFurniture)
        {
            carry.Enabled = on;
            carry.IsImmediate = false;
            return carry.Set;
        }

        Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> headers = [];
        Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> footers = [];

        bool titlePage = Sections[section].HasDifferentFirstPage;

        for (int slot = 0; slot < FurnitureStoryCount; slot++)
        {
            if ((on & (1 << slot)) == 0) continue;

            (bool isHeader, Model.PageFurnitureSlot which) = FurnitureSlotOf(slot);
            Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> into = isHeader ? headers : footers;

            IReadOnlyDictionary<Model.PageFurnitureSlot, List<Ww8LayoutBlock>> from =
                isHeader ? stated.Headers : stated.Footers;

            if (StoryLength(stated, slot) > 0 && from.TryGetValue(which, out List<Ww8LayoutBlock>? own))
            {
                List<PageBlock> converted = BlocksOf(fonts, own, widths);
                if (converted.Count > 0)
                {
                    into[which] = converted;
                    continue;
                }
            }

            // `else if (pPrev) CopyPageDescHdFt(pPrev, pPD, nI)` — one slot, out of the immediately
            // preceding section's page descriptor, and nothing at all when it has none.
            if (!carry.IsImmediate) continue;

            IReadOnlyDictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> source =
                isHeader ? carry.Headers : carry.Footers;

            if (source.TryGetValue(which, out IReadOnlyList<PageBlock>? inherited)) into[which] = inherited;
        }

        Complete(headers, isHeader: true);
        Complete(footers, isHeader: false);

        PageFurnitureSet set = new(headers, footers);
        PageFurnitureSet? result = set.IsEmpty ? null : set;

        carry.Headers = headers;
        carry.Footers = footers;
        carry.Set = result;
        carry.Enabled = on;

        // A continuous section's descriptor is built and then thrown away again unless the section holds
        // a hard page break to hang it on (`#i40766#`, `ww8par.cxx`:4540-4560) — so the *next* section
        // still finds `mpPage` null and inherits nothing. Its own pages keep what it built, which is why
        // it is computed at all.
        carry.IsImmediate = !continuous;

        return result;

        // The two rules `Read_HdFt` applies to every slot it turns on (ww8par.cxx:2519-2545), which
        // between them decide what the master page and the title page carry when only one page kind has
        // a story of its own.
        void Complete(Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> into, bool isHeader)
        {
            // Which of this kind's slots the section has turned on, read off the synthesised `grpfIhdt`
            // rather than off what landed in the dictionary: a slot whose bit is on and whose story is
            // both empty and uninheritable turns the master page's on while putting nothing in it.
            int odd = isHeader ? HeaderOddBit : FooterOddBit;
            int even = isHeader ? HeaderEvenBit : FooterEvenBit;
            int first = isHeader ? HeaderFirstBit : FooterFirstBit;

            bool enabled = (on & odd) != 0
                || (on & even) != 0
                || (titlePage && (on & first) != 0);

            if (!enabled) return;

            // "Cannot have left without right", as `Read_HdFt` labels it (#i17196#): every slot the
            // section turns on gives the *master* page one too, and the master's is what the Default
            // slot holds. So a section stating an even-page header alone still has one on its odd pages
            // — an empty one, which draws nothing and occupies the band.
            //
            // Only where Default is otherwise unfilled: a slot inherited from an earlier section is what
            // LibreOffice's own CopyPageDescHdFt puts there, and an inherited head beats a blank.
            if (!into.ContainsKey(Model.PageFurnitureSlot.Default))
            {
                into[Model.PageFurnitureSlot.Default] =
                    BlocksOf(fonts, _reader.BlankFurniture(section), widths);
            }

            // The left page by the same rule and a narrower condition: `if (bUseLeft)` sets the frame on
            // `pPD->GetLeft()` on the even iteration alone, so an even slot whose bit is on but whose
            // story is empty and uninheritable is a blank rather than a fall-back to the master's. Where
            // the bit is *off* the left page has no header at all, which this dictionary still cannot
            // say — an absent slot means "fall back to Default". That case reaches five sections of
            // three .doc in this corpus, measured over the header PLC of all 66; it is not this one.
            if ((on & even) != 0 && !into.ContainsKey(Model.PageFurnitureSlot.Even))
            {
                into[Model.PageFurnitureSlot.Even] =
                    BlocksOf(fonts, _reader.BlankFurniture(section), widths);
            }

            // And a title page never borrows the master's. `Read_HdFt` fills the first-page format from
            // the first-page story or not at all — only the `bUseFirst` iteration puts text in one — so
            // a section with a title page and no first-page story of this kind gets a blank one.
            //
            // Measured against LibreOffice's own flat-ODF export of the `title-page-no-head.doc`
            // fixture: `<style:header-first><text:p/></style:header-first>` beside a populated
            // `<style:header>`. The corpus case is `150_5300_13_chg12.doc`, whose first section states a
            // first-page footer and no first-page header — LibreOffice's page one carries no running
            // head, and falling back to the master's put "9/29/06 AC 150/5300-13 CHG 10" across the top
            // of its title page.
            if (titlePage && !into.ContainsKey(Model.PageFurnitureSlot.First))
            {
                into[Model.PageFurnitureSlot.First] =
                    BlocksOf(fonts, _reader.BlankFurniture(section), widths);
            }
        }

    }

    /// <summary>How many header stories a DOC section descriptor writes, whatever the section uses.</summary>
    private const int FurnitureStoryCount = 6;

    // The `grpfIhdt` bits, in the order the stories are written — `nsHdFtFlags` in `ww8scan.hxx`.
    private const int HeaderEvenBit = 0x01;
    private const int HeaderOddBit = 0x02;
    private const int FooterEvenBit = 0x04;
    private const int FooterOddBit = 0x08;
    private const int HeaderFirstBit = 0x10;
    private const int FooterFirstBit = 0x20;

    /// <summary>What the story at <paramref name="slot"/> is, as a kind and a slot.</summary>
    private static (bool IsHeader, Model.PageFurnitureSlot Slot) FurnitureSlotOf(int slot) => slot switch
    {
        0 => (true, Model.PageFurnitureSlot.Even),
        1 => (true, Model.PageFurnitureSlot.Default),
        2 => (false, Model.PageFurnitureSlot.Even),
        3 => (false, Model.PageFurnitureSlot.Default),
        4 => (true, Model.PageFurnitureSlot.First),
        _ => (false, Model.PageFurnitureSlot.First),
    };

    /// <summary>How long a section's story is, nought where the subdocument ends before it.</summary>
    private static int StoryLength(Ww8LayoutFurniture furniture, int slot)
        => slot < furniture.StoryLengths.Count ? furniture.StoryLengths[slot] : 0;

    /// <summary>
    /// What the section before this one left in force, threaded through the whole document.
    /// </summary>
    /// <remarks>
    /// Three things rather than one, because <c>Read_HdFt</c> asks three separate questions of the
    /// predecessor: what its page descriptor holds, which of the six slots its <c>grpfIhdt</c> had on, and
    /// whether the <em>immediately</em> preceding section had a page descriptor at all — a continuous one
    /// has none, and the copy is then never made.
    /// </remarks>
    private sealed class FurnitureCarry
    {
        /// <summary>The last page descriptor's headers, by slot.</summary>
        public IReadOnlyDictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> Headers { get; set; } =
            new Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>>();

        /// <summary>The last page descriptor's footers, by slot.</summary>
        public IReadOnlyDictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>> Footers { get; set; } =
            new Dictionary<Model.PageFurnitureSlot, IReadOnlyList<PageBlock>>();

        /// <summary>The whole set, which a continuous section hands straight on.</summary>
        public PageFurnitureSet? Set { get; set; }

        /// <summary>The previous section's <c>grpfIhdt</c>, which decides what a bit may inherit.</summary>
        public int Enabled { get; set; }

        /// <summary>True when the section immediately before this one had a page descriptor.</summary>
        public bool IsImmediate { get; set; }

        /// <summary>
        /// True when the section just described is the continuous-with-its-own-running-head case, which is
        /// the only one where <c>InsertSegments</c> builds a page descriptor for a continuous section — and
        /// therefore the only one where such a section has anywhere to put a page-number restart.
        /// </summary>
        public bool StatesOwnFurniture { get; set; }
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

        // Every flow a DOC has passes through here — the body, each header and footer, each note, each
        // text frame, and each cell by way of `Grid` — so one call covers all of them, and a cell's run
        // of bordered paragraphs is joined without seeing the paragraphs on either side of its table.
        ParagraphBorderJoin.Apply(blocks);

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
                CoveredTopRule = row.CoveredTopRule,
                CoveredBottomRule = row.CoveredBottomRule,
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
            HorizontalPosition = table.HorizontalPosition,

            // Every DOC is a Word document by definition, so the flag is not read from anything: it is
            // what LibreOffice's own filter sets on import, and it changes where the inner grid lines
            // stop. Measured on the corpus table — the reference's inner horizontals run 56.95 to 538.35
            // where the same table in ODF runs 56.45 to 538.85.
            JoinsBordersLikeWord = true,
            MinHeightIncludesInsets = true,
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
                Text = CaseMapping.Apply(WithSymbols(fonts, paragraph), runs),
                Face = face,
                Font = font,
                Colour = paragraph.Colour ?? Colour.Transparent,
                Format = paragraph.Format,
                Label = Label(fonts, paragraph, face, font),
                Borders = paragraph.Borders,
                EmSize = paragraph.Size,
                Language = paragraph.Language,
                Shaping = new Text.Shaping.ShapingOptions(
                    Language: paragraph.Language, DisableKerning: !paragraph.AutoKerning),
                Tracking = paragraph.Tracking,
                WidthPerCent = paragraph.WidthPerCent,
                Metrics = fonts.Metrics,
                Fallback = fonts.Fallback,
                AddsScriptSpace = true,

                // #i3952#, which the WW8 importer turns on for every DOC without asking the file
                // (`ww8par.cxx`:2041). See PageParagraph.BlanksAreTransparentToHeight.
                BlanksAreTransparentToHeight = true,
                Runs = runs,
                Fields = paragraph.Fields ?? [],
                Notes = NotesOf(fonts, paragraph.Notes),
                Frames = WithCheckBoxes(
                    FramesOf(
                        fonts, paragraph.Frames, paragraph.TextFrames, WidthFor(paragraph),
                        paragraph.IsInTable),
                    fonts, paragraph, face),
            });
        }

        return paragraphs;

        Length WidthFor(Ww8DocumentReader.Ww8LayoutParagraph paragraph)
            => textWidths is { Count: > 0 }
                ? textWidths[Math.Clamp(paragraph.SectionIndex, 0, textWidths.Count - 1)]
                : Length.Zero;
    }

    /// <summary>
    /// Adds the square each legacy <c>FORMCHECKBOX</c> in the paragraph puts on its line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SwFieldFormCheckboxPortion::Format</c> (<c>sw/source/core/text/portxt.cxx</c>:1492) sets the
    /// portion's width and height to <c>rInf.GetTextHeight()</c> and its ascent to
    /// <c>rInf.GetAscent()</c> — "the width of the checkbox portion is the same as its height since
    /// it's a square and that size depends on the font size" — and
    /// <c>SwTextPaintInfo::DrawCheckBox</c> (<c>inftxt.cxx</c>:1247) strokes that rectangle deflated by
    /// 25 twips a side, plus both diagonals when it is ticked. The same geometry the DOCX side already
    /// draws, and the measurement that pinned it is recorded on
    /// <c>DocxLayoutSource.CheckBoxFrame</c>: the square follows the run's text height and nothing the
    /// field declares.
    /// </para>
    /// <para>
    /// The run under the anchor decides the face, not the paragraph: a form's boxes are regularly set
    /// in a face of their own, and the square is as tall as the line it sits on.
    /// </para>
    /// </remarks>
    private static List<PageFrame> WithCheckBoxes(
        List<PageFrame> frames,
        LayoutFonts fonts,
        Ww8DocumentReader.Ww8LayoutParagraph paragraph,
        OpenTypeFace face)
    {
        if (paragraph.CheckBoxes is not { Count: > 0 } boxes) return frames;

        foreach (Ww8DocumentReader.Ww8LayoutCheckBox box in boxes)
        {
            OpenTypeFace boxFace = face;
            Length size = paragraph.Size;

            foreach (Ww8DocumentReader.Ww8LayoutRun run in paragraph.Runs ?? [])
            {
                if (box.Offset < run.Start || box.Offset >= run.End) continue;

                if (fonts.Face(run.FamilyName ?? paragraph.FamilyName, run.Weight, run.IsItalic)
                    is { } resolved)
                {
                    boxFace = resolved;
                }

                if (run.Size > Length.Zero) size = run.Size;
                break;
            }

            LineMetrics metrics =
                LineSpacing.Resolve(boxFace, fonts.Metrics, WriterLineBox.LeadingAboveText);
            Length side = metrics.ScaledLineHeight(size);
            if (side <= CheckBoxInset + CheckBoxInset) continue;

            frames.Add(new PageFrame
            {
                Size = new Core.Geometry.DocSize(side, side),
                Anchor = FrameAnchor.AsCharacter,
                AnchorOffset = box.Offset,
                InlineAscent = metrics.ScaledAscent(size),
                BorderColour = Colour.Black,
                BorderWidth = CheckBoxStroke,
                BorderInset = CheckBoxInset,
                IsCrossed = box.IsChecked,
            });
        }

        return frames;
    }

    /// <summary>How far inside the portion the box is stroked — <c>DrawCheckBox</c>'s own 25 twips.</summary>
    private static readonly Length CheckBoxInset = Length.FromTwips(25);

    /// <summary>The hairline the reference strokes it with — the 0.1 pt its PDF writer sets for a nought-width pen.</summary>
    private static readonly Length CheckBoxStroke = Length.FromPoints(0.1);

    /// <summary>
    /// The label a list item draws, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// At the level's own size, and — for a bullet whose level names a symbol face — in the face
    /// LibreOffice would substitute for it. Both halves come out of the level's <c>grpprlChpx</c>:
    /// <c>sprmCHps</c> for the size, which Word writes constantly and which makes an item's first line
    /// taller when it exceeds the item's own (see <see cref="PageParagraph.LabelRaisesFirstLine"/>),
    /// and <c>sprmCRgFtc0</c> for the face, which is what <see cref="SymbolLabel"/> needs in order to
    /// tell a symbol slot from the character it collides with.
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
        LayoutFonts fonts,
        Ww8DocumentReader.Ww8LayoutParagraph paragraph,
        OpenTypeFace face,
        FontReference? font)
    {
        if (paragraph.ListMarker is not { Length: > 0 } marker) return null;

        // A bullet's base font is the paragraph's with its posture reset — `#i53199` in
        // `SwTextFormatter::NewNumberPortion` — so only the level can lean it; a number's base keeps
        // the paragraph's, and the level overrides it only where it states one. Measured over the
        // `.doc` column of `probes/words-r59/label-slant.py`, where the level's `sprmCFItalic`
        // survives a round trip through 26.2.4.2 and the paragraph mark's own formatting does not.
        bool isBullet = paragraph.ListLabelSlot != '\0';
        bool italic = paragraph.ListLabelItalic ?? (!isBullet && paragraph.IsItalic);

        (string text, OpenTypeFace labelFace, FontReference? labelFont) =
            SymbolLabel(fonts, paragraph.ListLabelFamily, paragraph.ListLabelSlot, italic)
            ?? LevelLabel(fonts, paragraph.ListLabelFamily, marker, italic)
            ?? Paragraphs(fonts, paragraph, marker, face, font, italic);

        return PageLabel.Measured(
            text, labelFace,
            paragraph.ListLabelSize > Core.Units.Length.Zero
                ? paragraph.ListLabelSize
                : paragraph.Size,
            new Text.Shaping.ShapingOptions(
                Language: paragraph.Language, DisableKerning: !paragraph.AutoKerning)) with
        {
            Font = labelFont,
            Colour = paragraph.Colour ?? Colour.Transparent,
            Follow = paragraph.ListFollow switch
            {
                1 => LabelFollow.Space,
                2 => LabelFollow.Nothing,
                _ => LabelFollow.ListTab,
            },
            TabStop = Core.Units.Length.FromTwips(paragraph.ListTabStop),
        };
    }

    /// <summary>
    /// The paragraph's text with every <c>sprmCSymbol</c> run's placeholder replaced by its slot.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A WW8 symbol is not stored where it is drawn. The piece table holds a placeholder — U+0028 on
    /// every corpus document carrying one — and the slot lives in the CHPX beside it, so a reader that
    /// takes the stream at its word draws <c>(</c> where the document asks for <c>×</c>. Measured on
    /// <c>RMI_…GettingOffOil.doc</c>, whose reference reads <c>3.6×-more-efficient</c> against our
    /// <c>3.6(-more-efficient</c>.
    /// </para>
    /// <para>
    /// The replacement is one character per character the run covers, which is what
    /// <c>SwWW8ImplReader::ReadChars</c> does — it inserts <c>m_cSymbol</c> once for every character
    /// position the sprm is in force over (<c>ww8par.cxx</c>:3410-3413). Preserving the length is not
    /// incidental: every note, frame and field offset in the paragraph was recorded against this
    /// string, and a substitution that changed its length would move all of them.
    /// </para>
    /// <para>
    /// Unconditional, unlike the face beside it. LibreOffice replaces the placeholder whether or not
    /// the face it names can be loaded, so a slot with no recode table still draws its own code point
    /// rather than the placeholder.
    /// </para>
    /// </remarks>
    private static string WithSymbols(
        LayoutFonts fonts, Ww8DocumentReader.Ww8LayoutParagraph paragraph)
    {
        IReadOnlyList<Ww8DocumentReader.Ww8LayoutRun> runs = paragraph.Runs ?? [];

        StringBuilder? text = null;

        foreach (Ww8DocumentReader.Ww8LayoutRun run in runs)
        {
            if (run.SymbolSlot is not { } slot || slot == '\0') continue;

            char drawn = SymbolLabel(fonts, run.FamilyName, slot) is { Text: [char recoded] }
                ? recoded
                : slot;

            text ??= new StringBuilder(paragraph.Text);

            for (int index = run.Start; index < run.End && index < text.Length; index++)
            {
                text[index] = drawn;
            }
        }

        return text?.ToString() ?? paragraph.Text;
    }

    /// <summary>
    /// A symbol level's slot, drawn from the face that can actually show it, or null when none can.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The WW8 half of <c>DocxLayoutSource.Symbol</c>, and the same three cases: the face is installed
    /// and its slots are drawn as they stand; the face is absent and <see cref="SymbolFontRecode"/>
    /// holds a table for it, so the slot is recoded into OpenSymbol exactly as LibreOffice's
    /// <c>ConvertChar::RecodeChar</c> does; or neither, and the caller's own text and face stand.
    /// </para>
    /// <para>
    /// <strong>WW8 needs the face more than DOCX does.</strong> A DOCX bullet arrives already aliased
    /// into the private use area, so a fallback can at least tell it is a symbol. WW8 states the slot
    /// raw — <c>0xB7</c> in <c>Symbol</c> — so without <c>sprmCRgFtc0</c> the character is
    /// indistinguishable from MIDDLE DOT and was drawn as one.
    /// </para>
    /// </remarks>
    /// <summary>
    /// The label set in the face the level names, when that face is an ordinary one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="SymbolLabel"/> handles a level naming a <em>symbol</em> face, where the slot has to be
    /// recoded before it means anything. This is the other half, and it was missing: a level naming
    /// Courier New draws its label in Courier New, and nothing about that needs recoding.
    /// </para>
    /// <para>
    /// LibreOffice makes no distinction between the two — <c>WW8ListManager::ReadLVL</c>
    /// (<c>sw/source/filter/ww8/ww8par3.cxx</c>:1077-1095) reads the <c>SvxFontItem</c> out of the
    /// level's character format and calls <c>SetBulletFont</c> whatever the character is. Measured on
    /// <c>A320SimNotes.doc</c>, whose second-level bullet is Word's default <c>o</c> in Courier New:
    /// LibreOffice's PDF embeds four cuts of Liberation Mono that the document names nowhere else, and
    /// ours embedded none, drawing the bullet in the item's own Liberation Serif Bold instead.
    /// </para>
    /// <para>
    /// Refused when the resolved face has no glyph for the marker, which is the guard the symbol path
    /// gets from its recode table: a level naming a face that cannot draw its own label would put a
    /// missing-glyph box where a bullet belongs, and the paragraph's face is the better answer.
    /// </para>
    /// </remarks>
    private static (string Text, OpenTypeFace Face, FontReference? Font)? LevelLabel(
        LayoutFonts fonts, string? family, string marker, bool italic)
    {
        if (family is not { Length: > 0 } || marker.Length == 0) return null;
        if (SymbolFontRecode.IsRecodeable(family)) return null;
        if (fonts.Face(family, 400, italic) is not { } levelFace) return null;

        foreach (char character in marker)
        {
            if (!levelFace.HasGlyphFor(character)) return null;
        }

        return (marker, levelFace, fonts.Reference(family, 400, italic));
    }

    /// <summary>
    /// The label in the paragraph's own face, at the label's own posture.
    /// </summary>
    /// <remarks>
    /// What is left when the level names no face of its own, and it is the paragraph's face unless
    /// the label leans where the paragraph does not or the other way about — a level's
    /// <c>sprmCFItalic</c> is stated in either direction and beats the paragraph in both. Resolved
    /// only in that case, so a list paragraph whose label agrees with its text puts no second
    /// request through the font cache.
    /// </remarks>
    private static (string Text, OpenTypeFace Face, FontReference? Font) Paragraphs(
        LayoutFonts fonts,
        Ww8DocumentReader.Ww8LayoutParagraph paragraph,
        string marker,
        OpenTypeFace face,
        FontReference? font,
        bool italic)
    {
        if (italic == paragraph.IsItalic) return (marker, face, font);

        return fonts.Face(paragraph.FamilyName, paragraph.Weight, italic) is { } leaned
            ? (marker, leaned, fonts.Reference(paragraph.FamilyName, paragraph.Weight, italic))
            : (marker, face, font);
    }

    /// <param name="fonts">The shared font cache.</param>
    /// <param name="family">The family the level names, or null when it names none.</param>
    /// <param name="slot">The raw code point the level states, or <c>\0</c> for a counter.</param>
    /// <param name="italic">
    /// Whether the slot is drawn leaning. Defaulted <c>false</c> for the two callers that resolve a
    /// <c>sprmCSymbol</c> inside ordinary text rather than a list level, because those are given a
    /// family and a slot and not the run's own CHPX. Whether 26.2.4.2 leans a recoded symbol in an
    /// italic run is <strong>not measured</strong> — round 58's residual is a column of list bullets
    /// at one x position and holds no run symbol — so the default preserves what was drawn before
    /// rather than guessing.
    /// </param>
    private static (string Text, OpenTypeFace Face, FontReference? Font)? SymbolLabel(
        LayoutFonts fonts, string? family, char slot, bool italic = false)
    {
        if (family is not { Length: > 0 }) return null;
        if (slot == '\0') return null;
        if (!SymbolFontRecode.IsRecodeable(family)) return null;

        if (fonts.Face(family, 400, italic) is not { } statedFace) return null;
        FontReference? reference = fonts.Reference(family, 400, italic);

        // The face's own file is present, so its slots are drawable as they stand.
        if (reference is not null
            && !reference.IsSubstituted
            && !SymbolFontRecode.IsSubstituteFamily(reference.FamilyName))
        {
            return (slot.ToString(), statedFace, reference);
        }

        if (!SymbolFontRecode.TryRecode(family, slot, out char recoded)) return null;

        // The recode and the face go together: the code point means nothing anywhere but OpenSymbol,
        // so a resolution that landed elsewhere leaves the caller's fallback in place.
        if (fonts.Face(SymbolFontRecode.SubstituteFamily, 400, italic) is not { } symbolFace)
        {
            return null;
        }

        FontReference? symbolReference =
            fonts.Reference(SymbolFontRecode.SubstituteFamily, 400, italic);

        return symbolReference is not null
               && SymbolFontRecode.IsSubstituteFamily(symbolReference.FamilyName)
            ? (recoded.ToString(), symbolFace, symbolReference)
            : null;
    }

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
    /// <param name="inTableCell">
    /// True when the anchoring paragraph is inside a table cell, which is the one thing about the
    /// paragraph a shape's own position depends on — see <see cref="Ww8Frames.LaysOutInTableCell"/>.
    /// </param>
    private static List<PageFrame> FramesOf(
        LayoutFonts fonts,
        IReadOnlyList<Ww8LayoutFrame>? stated,
        IReadOnlyList<Ww8LayoutTextFrame>? textFrames = null,
        Length textWidth = default,
        bool inTableCell = false)
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
                    Crop = frame.Picture.Crop,
                });

                continue;
            }

            PageFrame? built = Ww8Frames.Build(
                frame.Anchor, frame.Shape, frame.Offset, BlocksOf(fonts, frame.Blocks),
                frame.IsSetInLine, inTableCell, frame.WrittenByWord97);
            if (built is null) continue;

            PageFrame envelope = built with
            {
                Image = frame.Picture.Raster,
                Vector = frame.Picture.Vector,
                Crop = frame.Picture.Crop,
            };
            frames.Add(envelope);

            // A group is flattened into its envelope and one sibling frame per leaf shape. The
            // envelope keeps the wrap and is what the text avoids; the members are drawn inside it.
            foreach (Ww8LayoutFrameMember member in frame.Members)
            {
                PageFrame? placed = Ww8Frames.Member(
                    envelope, member.Shape,
                    (member.Left, member.Top, member.Width, member.Height),
                    BlocksOf(fonts, member.Blocks), member.Picture);

                if (placed is not null) frames.Add(placed);
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
        FontReference? paragraphFont =
            fonts.Reference(paragraph.FamilyName, paragraph.Weight, paragraph.IsItalic);

        foreach (Ww8DocumentReader.Ww8LayoutRun run in stated)
        {
            // A `sprmCSymbol` run's face is decided together with the character it draws — see
            // WithSymbols — so the same resolution answers both, and a slot recoded into OpenSymbol
            // has to be drawn from OpenSymbol or it is .notdef.
            (OpenTypeFace Face, FontReference? Font)? symbol = run.SymbolSlot is { } slot
                ? SymbolLabel(fonts, run.FamilyName, slot) is { } resolved
                    ? (resolved.Face, resolved.Font)
                    : null
                : null;

            OpenTypeFace face = symbol?.Face
                ?? fonts.Face(run.FamilyName, run.Weight, run.IsItalic)
                ?? paragraphFace;
            FontReference? font = symbol is { } named
                ? named.Font
                : fonts.Reference(run.FamilyName, run.Weight, run.IsItalic);

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
                || run.Underline != TextUnderline.None
                || run.IsStruckThrough
                // Kerning, unlike the two rules, does change a measurement — so a run that kerns
                // inside a paragraph that does not has to survive the shortcut or its width is the
                // paragraph's answer rather than its own.
                || run.AutoKerning != paragraph.AutoKerning
                // And tracking for the same reason again, read the other way: a run condensed inside a
                // paragraph that is not would otherwise be measured at the paragraph's own spacing.
                || run.Tracking != paragraph.Tracking
                // And a character width, which multiplies the advance rather than adding to it, so a
                // scaled run folded into an unscaled paragraph is measured and broken at the
                // paragraph's own width. See `Ww8LayoutRun.WidthPerCent` for the merge that has to
                // survive first.
                || run.WidthPerCent != paragraph.WidthPerCent
                // A symbol's face is its own even when it happens to equal the paragraph's: losing the
                // runs here would draw its slot out of whatever the paragraph is set in.
                || run.SymbolSlot is not null
                // And a synthetic oblique, which is drawing-only in the same way and was the one
                // missing from this list: an italic run whose family has no italic installed resolves to
                // the *same* face as its upright neighbour, so nothing above can see it and the fold
                // would draw it upright. See PageRun.LeansDifferently.
                || PageRun.LeansDifferently(font, paragraphFont))
            {
                varies = true;
            }

            runs.Add(new PageRun(
                run.Start,
                run.Length,
                face,
                size,
                font,
                run.Colour ?? paragraph.Colour ?? Colour.Transparent,
                new Text.Shaping.ShapingOptions(
                    Language: run.Language, DisableKerning: !run.AutoKerning),
                rise,
                run.CaseMap,
                Highlight: run.Highlight ?? default,
                Underline: run.Underline,
                IsStruckThrough: run.IsStruckThrough,
                Tracking: run.Tracking,
                WidthPerCent: run.WidthPerCent));
        }

        return varies ? runs : [];
    }

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}

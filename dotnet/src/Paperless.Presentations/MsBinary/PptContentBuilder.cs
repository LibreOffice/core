using Paperless.Core.Diagnostics;
using Paperless.Core.Extraction;
using Paperless.MsBinary.Escher;
using Paperless.MsBinary.Records;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// Turns a binary PowerPoint record tree into the shared content tree.
/// </summary>
/// <remarks>
/// The shape of the result deliberately matches what the ODF path produces for the same deck:
/// each slide is a <see cref="SectionKind.Slide"/> section, its speaker notes a
/// <see cref="SectionKind.SlideNotes"/> section beside it, shapes are read in document order
/// including the contents of groups, and a hidden slide is extracted with
/// <see cref="ContentSection.IsHidden"/> set rather than skipped. A caller indexing a mixed
/// corpus should not be able to tell which of the two files it was given.
/// </remarks>
internal sealed class PptContentBuilder
{
    /// <summary>
    /// The header instance of each of the document's three slide lists.
    /// </summary>
    /// <remarks>
    /// The instance is the reliable discriminator — 0 for slides, 1 for masters, 2 for notes.
    /// LibreOffice instead uses the lists' order in the container and then has to special-case
    /// files that write notes before slides
    /// (<c>filter/source/msfilter/svdfppt.cxx:1502</c>, the <c>notePresentationSwap</c> flag);
    /// reading the instance the format states avoids the guess entirely.
    /// </remarks>
    private const ushort SlideListInstance = 0;
    private const ushort NotesListInstance = 2;

    private readonly DffRecordBuffer _stream;
    private readonly PptPersistDirectory _persist;
    private readonly List<Diagnostic> _diagnostics;
    private readonly EscherDrawingReader _escher;

    public PptContentBuilder(
        DffRecordBuffer stream, PptPersistDirectory persist, List<Diagnostic> diagnostics)
    {
        _stream = stream;
        _persist = persist;
        _diagnostics = diagnostics;
        _escher = new EscherDrawingReader(stream, diagnostics);
    }

    /// <summary>Reads the document container and fills the content tree.</summary>
    public void Build(ContentDocument content)
    {
        if (_persist.DocumentOffset is not { } offset
            || !_stream.TryReadHeader(offset, out DffRecordHeader document)
            || document.Type != PptRecordTypes.Document)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2403",
                "The persist directory does not resolve to a Document record, so no slides "
                + "could be read."));
            return;
        }

        List<List<SlideEntry>> lists = [];
        List<ushort> instances = [];

        foreach (DffRecordHeader child in _stream.Children(document))
        {
            if (child.Type != PptRecordTypes.SlideListWithText) continue;
            lists.Add(ReadSlideList(child));
            instances.Add(child.Instance);
        }

        List<SlideEntry> slides = [];
        List<SlideEntry> notes = [];

        // Every writer distinguishes the three lists by instance. A file that leaves them all
        // equal — which a repair tool can produce — falls back to their order in the container,
        // masters first, which is what LibreOffice relies on unconditionally.
        bool byInstance = instances.Contains(SlideListInstance)
                          && instances.Distinct().Count() == instances.Count;

        for (int i = 0; i < lists.Count; i++)
        {
            bool isSlides = byInstance ? instances[i] == SlideListInstance : i == 1;
            bool isNotes = byInstance ? instances[i] == NotesListInstance : i == 2;

            if (isSlides) slides.AddRange(lists[i]);
            else if (isNotes) notes.AddRange(lists[i]);
        }

        Dictionary<uint, SlideEntry> notesBySlide = [];
        foreach (SlideEntry entry in notes) notesBySlide.TryAdd(entry.SlideId, entry);

        for (int index = 0; index < slides.Count; index++)
        {
            SlideEntry entry = slides[index];
            ContentSection slide = ReadSlide(entry, index);
            content.Children.Add(slide);

            if (notesBySlide.TryGetValue(entry.SlideId, out SlideEntry notesEntry)
                && ReadNotes(notesEntry, index) is { } notesSection)
            {
                content.Children.Add(notesSection);
            }
        }
    }

    /// <summary>
    /// Reads one <c>SlideListWithText</c>: an entry per slide, each followed by that slide's
    /// outline text.
    /// </summary>
    /// <remarks>
    /// The text records belonging to an entry are the ones between its
    /// <c>SlidePersistAtom</c> and the next, which is why the range is recorded here rather
    /// than looked for later: nothing inside the text records says which slide they belong to.
    /// </remarks>
    private List<SlideEntry> ReadSlideList(DffRecordHeader list)
    {
        List<SlideEntry> entries = [];
        int listEnd = _stream.EndOf(list);
        int pendingIndex = -1;

        foreach (DffRecordHeader record in _stream.Children(list))
        {
            if (record.Type != PptRecordTypes.SlidePersistAtom) continue;

            if (pendingIndex >= 0) entries[pendingIndex] = entries[pendingIndex] with { TextEnd = record.Position };

            ReadOnlySpan<byte> content = _stream.Content(record);
            if (content.Length < 16) continue;

            entries.Add(new SlideEntry(
                PersistId: DffRecordBuffer.ReadUInt32(content),
                SlideId: DffRecordBuffer.ReadUInt32(content[12..]),
                TextStart: _stream.EndOf(record),
                TextEnd: listEnd));
            pendingIndex = entries.Count - 1;
        }

        return entries;
    }

    /// <summary>Reads one slide into a section.</summary>
    private ContentSection ReadSlide(SlideEntry entry, int index)
    {
        if (Resolve(entry, PptRecordTypes.Slide) is not { } container)
        {
            return new ContentSection { Kind = SectionKind.Slide, Index = index };
        }

        bool hidden = false;
        foreach (DffRecordHeader record in _stream.Children(container))
        {
            if (record.Type == PptRecordTypes.SlideShowSlideInfoAtom)
            {
                hidden |= IsHidden(_stream.Content(record));
            }
        }

        ContentSection slide = new()
        {
            Kind = SectionKind.Slide,
            Index = index,
            IsHidden = hidden,
        };

        ReadDrawing(container, entry, slide);
        return slide;
    }

    /// <summary>Reads one notes page, or null when it holds no text.</summary>
    /// <remarks>
    /// LibreOffice writes a notes page for every slide whether or not it has notes, carrying a
    /// slide thumbnail and an empty placeholder. Reporting those as content would invent a
    /// notes section for every slide in every deck, so an empty one is dropped — the same rule
    /// the ODF path applies.
    /// </remarks>
    private ContentSection? ReadNotes(SlideEntry entry, int index)
    {
        if (Resolve(entry, PptRecordTypes.Notes) is not { } container) return null;

        ContentSection notes = new() { Kind = SectionKind.SlideNotes, Index = index };
        ReadDrawing(container, entry, notes);

        return notes.GetText().Trim().Length > 0 ? notes : null;
    }

    /// <summary>
    /// The container a slide-list entry names, when its persist id resolves to one of the
    /// expected type.
    /// </summary>
    private DffRecordHeader? Resolve(SlideEntry entry, ushort expected)
    {
        if (_persist.Resolve(entry.PersistId) is not { } offset)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2404",
                $"Slide list entry {entry.SlideId} names persist id {entry.PersistId}, which the "
                + "persist directory does not resolve; the page was skipped."));
            return null;
        }

        if (!_stream.TryReadHeader(offset, out DffRecordHeader header)) return null;

        if (header.Type != expected)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2405",
                $"Persist id {entry.PersistId} resolves to offset {offset}, which holds record "
                + $"type {header.Type} rather than the expected {expected}."));
            return null;
        }

        return header;
    }

    /// <summary>
    /// Whether a slide's show information marks it as skipped.
    /// </summary>
    /// <remarks>
    /// The flag lives in the middle of a transition record: four bytes of advance time, four of
    /// a sound reference, then single bytes of direction and transition type, and only then the
    /// flags word whose bit 2 means "do not show"
    /// (<c>sd/source/filter/ppt/pptin.cxx:1825</c>). Nothing names it; counting the preceding
    /// fields is the whole of finding it.
    /// </remarks>
    private static bool IsHidden(ReadOnlySpan<byte> content)
        => content.Length >= 12 && (DffRecordBuffer.ReadUInt16(content[10..]) & 0x0004) != 0;

    /// <summary>Reads a page's Escher drawing, adding each shape's text in document order.</summary>
    private void ReadDrawing(DffRecordHeader page, SlideEntry entry, ContentSection target)
    {
        DffRecordHeader? drawing = _stream.FirstChild(page, PptRecordTypes.Drawing);
        if (drawing is not { } ppDrawing) return;

        DffRecordHeader? dg = _stream.FirstChild(ppDrawing, EscherRecordTypes.DrawingContainer);
        if (dg is not { } drawingContainer) return;

        foreach (EscherShape shape in _escher.ReadDrawing(drawingContainer))
        {
            AddShape(shape, entry, target);
        }
    }

    /// <summary>Adds one shape's text, then its children's, keeping document order.</summary>
    private void AddShape(EscherShape shape, SlideEntry entry, ContentSection target)
    {
        // The background shape is a fill, not content; the deleted flag marks a record left
        // behind by an undo. Neither belongs in extracted text.
        if (!shape.IsBackground && !shape.IsDeleted && ReadShapeText(shape, entry) is { } run)
        {
            foreach (ContentParagraph paragraph in PptTextReader.ToParagraphs(run))
            {
                target.Children.Add(paragraph);
            }
        }

        foreach (EscherShape child in shape.Children) AddShape(child, entry, target);
    }

    /// <summary>
    /// A shape's text, whether it holds the characters itself or refers to the slide list.
    /// </summary>
    private PptTextRun? ReadShapeText(EscherShape shape, SlideEntry entry)
    {
        if (shape.ClientTextbox is not { } textbox) return null;

        int start = textbox.ContentStart;
        int end = _stream.EndOf(textbox);

        // An outline placeholder stores only a reference; the characters are in the slide's own
        // entry in the document's slide list.
        foreach (DffRecordHeader record in _stream.Range(start, end))
        {
            if (record.Type != PptRecordTypes.OutlineTextRefAtom) continue;

            uint reference = DffRecordBuffer.ReadUInt32(_stream.Content(record));
            return ReadOutlineText(entry, reference);
        }

        return PptTextReader.Read(_stream, start, end);
    }

    /// <summary>
    /// The <paramref name="reference"/>th text run of a slide's entry in the document's slide
    /// list.
    /// </summary>
    /// <remarks>
    /// The runs are not delimited by a container: a run begins at a <c>TextHeaderAtom</c> and
    /// ends at the next one, or at the next slide's <c>SlidePersistAtom</c>. LibreOffice
    /// synthesises a client-textbox header over exactly that span
    /// (<c>filter/source/msfilter/svdfppt.cxx:6660</c>); the effect is the same either way.
    /// </remarks>
    private PptTextRun? ReadOutlineText(SlideEntry entry, uint reference)
    {
        int matches = 0;
        int start = -1;

        foreach (DffRecordHeader record in _stream.Range(entry.TextStart, entry.TextEnd))
        {
            if (record.Type == PptRecordTypes.SlidePersistAtom) break;
            if (record.Type != PptRecordTypes.TextHeaderAtom) continue;

            if (start >= 0) return PptTextReader.Read(_stream, start, record.Position);
            if (matches++ == reference) start = record.Position;
        }

        return start >= 0 ? PptTextReader.Read(_stream, start, entry.TextEnd) : null;
    }

    /// <summary>One entry of a slide list, with the byte range holding its outline text.</summary>
    private readonly record struct SlideEntry(
        uint PersistId, uint SlideId, int TextStart, int TextEnd);
}

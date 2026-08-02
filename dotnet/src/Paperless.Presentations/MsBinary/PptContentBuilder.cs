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
    private readonly DffRecordBuffer _stream;
    private readonly PptPersistDirectory _persist;
    private readonly List<Diagnostic> _diagnostics;
    private readonly EscherDrawingReader _escher;

    /// <summary>The style sheet of each master, by the slide id its persist atom gives it.</summary>
    private readonly Dictionary<uint, PptStyleSheet> _stylesByMaster = [];

    /// <summary>
    /// The sheet a page whose master cannot be found falls back to.
    /// </summary>
    /// <remarks>
    /// LibreOffice's <c>m_pDefaultSheet</c> — the last main master read
    /// (<c>filter/source/msfilter/svdfppt.cxx:1619</c>). Notes pages reach it by the same route:
    /// a notes master carries no <c>TxMasterStyleAtom</c> of its own, so their defaults come
    /// from the main master's notes instance.
    /// </remarks>
    private PptStyleSheet? _defaultStyles;
    private PptFontTable _fonts = PptFontTable.Empty;

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
        if (PptPages.Read(_stream, _persist, _diagnostics) is not { } pages) return;

        ReadMasterStyles(pages);

        // Only a bullet needs the font collection here, and only to tell a symbol face from a
        // text one — but a bullet stated in one is a glyph slot rather than a letter, so without
        // this an extracted list is marked with the letter the slot happens to share a code with.
        _fonts = PptFontTable.Read(_stream, pages.Environment);

        Dictionary<uint, PptPageEntry> notesBySlide = [];
        foreach (PptPageEntry entry in pages.Notes) notesBySlide.TryAdd(entry.SlideId, entry);

        for (int index = 0; index < pages.Slides.Count; index++)
        {
            PptPageEntry entry = pages.Slides[index];
            ContentSection slide = ReadSlide(entry, index);
            content.Children.Add(slide);

            if (notesBySlide.TryGetValue(entry.SlideId, out PptPageEntry notesEntry)
                && ReadNotes(notesEntry, index) is { } notesSection)
            {
                content.Children.Add(notesSection);
            }
        }
    }

    /// <summary>Reads one slide into a section.</summary>
    private ContentSection ReadSlide(PptPageEntry entry, int index)
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

        ReadDrawing(container, entry, slide, StylesFor(container));
        return slide;
    }

    /// <summary>
    /// Builds one style sheet per main master, and notes which master each is for.
    /// </summary>
    /// <remarks>
    /// A title master states another master's slide id and carries no styles of its own, so it
    /// is resolved in a second pass — the first cannot, because the list may write a title
    /// master before the main master it points at.
    /// </remarks>
    private void ReadMasterStyles(PptPages pages)
    {
        DffRecordHeader? environment = pages.Environment;
        Dictionary<uint, uint> derived = [];

        foreach (PptPageEntry entry in pages.Masters)
        {
            if (_persist.Resolve(entry.PersistId) is not { } offset) continue;
            if (!_stream.TryReadHeader(offset, out DffRecordHeader header)) continue;

            // A notes master is in this list too on some files, as a Notes container. It has no
            // TxMasterStyleAtom, so there is nothing here to read from it.
            if (header.Type != PptRecordTypes.MainMaster) continue;

            uint parent = PptPages.MasterIdOf(_stream, header) ?? 0;
            if (parent != 0)
            {
                derived[entry.SlideId] = parent;
                continue;
            }

            PptStyleSheet sheet = PptStyleSheet.Read(_stream, header, environment);
            _stylesByMaster[entry.SlideId] = sheet;
            _defaultStyles = sheet;
        }

        foreach ((uint child, uint parent) in derived)
        {
            if (_stylesByMaster.TryGetValue(parent, out PptStyleSheet? sheet))
            {
                _stylesByMaster[child] = sheet;
            }
            else if (_defaultStyles is { } fallback)
            {
                _stylesByMaster[child] = fallback;
            }
        }
    }

    /// <summary>The style sheet a page resolves its unstated formatting against.</summary>
    private PptStyleSheet? StylesFor(DffRecordHeader page)
        => PptPages.MasterIdOf(_stream, page) is { } master
           && _stylesByMaster.TryGetValue(master, out PptStyleSheet? sheet)
            ? sheet
            : _defaultStyles;

    /// <summary>Reads one notes page, or null when it holds no text.</summary>
    /// <remarks>
    /// LibreOffice writes a notes page for every slide whether or not it has notes, carrying a
    /// slide thumbnail and an empty placeholder. Reporting those as content would invent a
    /// notes section for every slide in every deck, so an empty one is dropped — the same rule
    /// the ODF path applies.
    /// </remarks>
    private ContentSection? ReadNotes(PptPageEntry entry, int index)
    {
        if (Resolve(entry, PptRecordTypes.Notes) is not { } container) return null;

        ContentSection notes = new() { Kind = SectionKind.SlideNotes, Index = index };
        ReadDrawing(container, entry, notes, _defaultStyles);

        return notes.GetText().Trim().Length > 0 ? notes : null;
    }

    /// <summary>
    /// The container a slide-list entry names, when its persist id resolves to one of the
    /// expected type.
    /// </summary>
    private DffRecordHeader? Resolve(PptPageEntry entry, ushort expected)
        => PptPages.Resolve(_stream, _persist, entry, expected, _diagnostics);

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
    private void ReadDrawing(
        DffRecordHeader page, PptPageEntry entry, ContentSection target, PptStyleSheet? styles)
    {
        DffRecordHeader? drawing = _stream.FirstChild(page, PptRecordTypes.Drawing);
        if (drawing is not { } ppDrawing) return;

        DffRecordHeader? dg = _stream.FirstChild(ppDrawing, EscherRecordTypes.DrawingContainer);
        if (dg is not { } drawingContainer) return;

        foreach (EscherShape shape in _escher.ReadDrawing(drawingContainer))
        {
            AddShape(shape, entry, target, styles);
        }
    }

    /// <summary>Adds one shape's text, then its children's, keeping document order.</summary>
    private void AddShape(
        EscherShape shape, PptPageEntry entry, ContentSection target, PptStyleSheet? styles)
    {
        // The background shape is a fill, not content; the deleted flag marks a record left
        // behind by an undo. Neither belongs in extracted text.
        if (!shape.IsBackground && !shape.IsDeleted && ReadShapeText(shape, entry) is { } run)
        {
            foreach (ContentParagraph paragraph in PptTextReader.ToParagraphs(run, styles, _fonts))
            {
                target.Children.Add(paragraph);
            }
        }

        foreach (EscherShape child in shape.Children) AddShape(child, entry, target, styles);
    }

    /// <summary>
    /// A shape's text, whether it holds the characters itself or refers to the slide list.
    /// </summary>
    private PptTextRun? ReadShapeText(EscherShape shape, PptPageEntry entry)
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
    private PptTextRun? ReadOutlineText(PptPageEntry entry, uint reference)
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
}

using Paperless.Core.Diagnostics;
using Paperless.MsBinary.Records;

namespace Paperless.Presentations.MsBinary;

/// <summary>One entry of a slide list, with the byte range holding its outline text.</summary>
/// <remarks>
/// The range is recorded when the list is walked rather than looked for later, because nothing
/// inside a text record says which slide it belongs to: the records belonging to an entry are
/// simply the ones between its <c>SlidePersistAtom</c> and the next.
/// </remarks>
/// <param name="PersistId">The persist id naming the page's container.</param>
/// <param name="SlideId">The page's own id, which is what a <c>SlideAtom</c> refers to.</param>
/// <param name="TextStart">The first offset of the entry's outline text records.</param>
/// <param name="TextEnd">One past the last.</param>
internal readonly record struct PptPageEntry(
    uint PersistId, uint SlideId, int TextStart, int TextEnd);

/// <summary>
/// The document container's three slide lists, told apart and resolved.
/// </summary>
/// <remarks>
/// Shared by extraction and layout because both need exactly the same walk and neither should
/// own it: a divergence between the two would mean the same deck extracted one set of slides and
/// drew another.
/// </remarks>
internal sealed class PptPages
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
    private const ushort MasterListInstance = 1;
    private const ushort NotesListInstance = 2;

    private PptPages(
        DffRecordBuffer stream,
        DffRecordHeader document,
        List<PptPageEntry> slides,
        List<PptPageEntry> masters,
        List<PptPageEntry> notes)
    {
        Document = document;
        Slides = slides;
        Masters = masters;
        Notes = notes;
        Environment = stream.FirstChild(document, PptRecordTypes.Environment);
    }

    /// <summary>The <c>Document</c> container itself.</summary>
    public DffRecordHeader Document { get; }

    /// <summary>The document's <c>Environment</c> container, when it has one.</summary>
    public DffRecordHeader? Environment { get; }

    /// <summary>The slides, in presentation order.</summary>
    public IReadOnlyList<PptPageEntry> Slides { get; }

    /// <summary>The masters, in the order the file writes them.</summary>
    public IReadOnlyList<PptPageEntry> Masters { get; }

    /// <summary>The notes pages, each naming the slide it belongs to by slide id.</summary>
    public IReadOnlyList<PptPageEntry> Notes { get; }

    /// <summary>
    /// Walks the document container, or reports why it could not be found.
    /// </summary>
    /// <param name="stream">The document stream.</param>
    /// <param name="persist">The persist directory, which is the only way to the container.</param>
    /// <param name="diagnostics">Where to record what could not be read.</param>
    public static PptPages? Read(
        DffRecordBuffer stream, PptPersistDirectory persist, List<Diagnostic> diagnostics)
    {
        if (persist.DocumentOffset is not { } offset
            || !stream.TryReadHeader(offset, out DffRecordHeader document)
            || document.Type != PptRecordTypes.Document)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2403",
                "The persist directory does not resolve to a Document record, so no slides "
                + "could be read."));
            return null;
        }

        List<List<PptPageEntry>> lists = [];
        List<ushort> instances = [];

        foreach (DffRecordHeader child in stream.Children(document))
        {
            if (child.Type != PptRecordTypes.SlideListWithText) continue;
            lists.Add(ReadList(stream, child));
            instances.Add(child.Instance);
        }

        List<PptPageEntry> slides = [];
        List<PptPageEntry> masters = [];
        List<PptPageEntry> notes = [];

        // Every writer distinguishes the three lists by instance. A file that leaves them all
        // equal — which a repair tool can produce — falls back to their order in the container,
        // masters first, which is what LibreOffice relies on unconditionally.
        bool byInstance = instances.Contains(SlideListInstance)
                          && instances.Distinct().Count() == instances.Count;

        for (int i = 0; i < lists.Count; i++)
        {
            bool isSlides = byInstance ? instances[i] == SlideListInstance : i == 1;
            bool isMasters = byInstance ? instances[i] == MasterListInstance : i == 0;
            bool isNotes = byInstance ? instances[i] == NotesListInstance : i == 2;

            if (isSlides) slides.AddRange(lists[i]);
            else if (isMasters) masters.AddRange(lists[i]);
            else if (isNotes) notes.AddRange(lists[i]);
        }

        return new PptPages(stream, document, slides, masters, notes);
    }

    /// <summary>
    /// The container an entry names, when its persist id resolves to one of the expected type.
    /// </summary>
    public static DffRecordHeader? Resolve(
        DffRecordBuffer stream,
        PptPersistDirectory persist,
        PptPageEntry entry,
        ushort expected,
        List<Diagnostic> diagnostics)
    {
        if (persist.Resolve(entry.PersistId) is not { } offset)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2404",
                $"Slide list entry {entry.SlideId} names persist id {entry.PersistId}, which the "
                + "persist directory does not resolve; the page was skipped."));
            return null;
        }

        if (!stream.TryReadHeader(offset, out DffRecordHeader header)) return null;

        if (header.Type != expected)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2405",
                $"Persist id {entry.PersistId} resolves to offset {offset}, which holds record "
                + $"type {header.Type} rather than the expected {expected}."));
            return null;
        }

        return header;
    }

    /// <summary>
    /// The slide id of the master a page names, or null when the page has no <c>SlideAtom</c>.
    /// </summary>
    /// <remarks>
    /// The field is a <em>slide</em> id, not a persist id — masters number themselves from
    /// <c>0x80000000</c> — so it is matched against the master list's persist atoms rather than
    /// resolved through the persist directory (<c>svdfppt.cxx:2520</c>). It sits behind a
    /// four-byte layout geometry and the eight placeholder ids of that layout.
    /// </remarks>
    public static uint? MasterIdOf(DffRecordBuffer stream, DffRecordHeader page)
    {
        if (stream.FirstChild(page, PptRecordTypes.SlideAtom) is not { } atom) return null;

        ReadOnlySpan<byte> content = stream.Content(atom);
        return content.Length >= 16 ? DffRecordBuffer.ReadUInt32(content[12..]) : null;
    }

    private static List<PptPageEntry> ReadList(DffRecordBuffer stream, DffRecordHeader list)
    {
        List<PptPageEntry> entries = [];
        int listEnd = stream.EndOf(list);
        int pendingIndex = -1;

        foreach (DffRecordHeader record in stream.Children(list))
        {
            if (record.Type != PptRecordTypes.SlidePersistAtom) continue;

            if (pendingIndex >= 0)
                entries[pendingIndex] = entries[pendingIndex] with { TextEnd = record.Position };

            ReadOnlySpan<byte> content = stream.Content(record);
            if (content.Length < 16) continue;

            entries.Add(new PptPageEntry(
                PersistId: DffRecordBuffer.ReadUInt32(content),
                SlideId: DffRecordBuffer.ReadUInt32(content[12..]),
                TextStart: stream.EndOf(record),
                TextEnd: listEnd));
            pendingIndex = entries.Count - 1;
        }

        return entries;
    }
}

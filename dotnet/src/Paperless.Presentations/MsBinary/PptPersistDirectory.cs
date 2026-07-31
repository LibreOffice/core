using Paperless.Core.Diagnostics;
using Paperless.MsBinary.Records;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// The map from a PowerPoint persist id to the offset in the <c>PowerPoint Document</c> stream
/// where that object's current version begins.
/// </summary>
/// <remarks>
/// <para>
/// A binary PowerPoint file is written incrementally: saving appends the changed objects and
/// leaves the previous copies in place, orphaned but perfectly parseable. Nothing in the record
/// tree distinguishes a current slide from a superseded one, so a reader that scans the stream
/// top to bottom silently reports an old version of the deck. The only correct route is this
/// directory.
/// </para>
/// <para>
/// It is assembled by walking the <c>UserEditAtom</c> chain backwards from the newest edit —
/// found via the <c>Current User</c> stream, or by taking the last such atom in the stream when
/// that fails — and reading each edit's <c>PersistPtrIncrementalBlock</c>. Each block holds runs
/// of offsets, prefixed by a word packing a start id in its low twenty bits and a count in its
/// top twelve. Because the walk runs newest first, <strong>the first offset written for an id
/// wins</strong> and later (older) blocks must not overwrite it — that single rule is what makes
/// the directory current rather than original
/// (<c>SdrPowerPointImport::SdrPowerPointImport</c>, <c>filter/source/msfilter/svdfppt.cxx:1358</c>).
/// </para>
/// </remarks>
public sealed class PptPersistDirectory
{
    private readonly Dictionary<uint, uint> _offsets;

    private PptPersistDirectory(Dictionary<uint, uint> offsets, uint documentPersistId)
    {
        _offsets = offsets;
        DocumentPersistId = documentPersistId;
    }

    /// <summary>The persist id of the document container, from the newest edit atom.</summary>
    public uint DocumentPersistId { get; }

    /// <summary>How many ids the directory resolves.</summary>
    public int Count => _offsets.Count;

    /// <summary>The stream offset the document container begins at, when it resolves.</summary>
    public int? DocumentOffset => Resolve(DocumentPersistId);

    /// <summary>The stream offset for a persist id, or null when it does not resolve.</summary>
    public int? Resolve(uint persistId)
        => _offsets.TryGetValue(persistId, out uint offset) ? (int)offset : null;

    /// <summary>
    /// Builds the directory for a document stream.
    /// </summary>
    /// <param name="stream">The <c>PowerPoint Document</c> stream.</param>
    /// <param name="currentUserEdit">
    /// The offset of the newest <c>UserEditAtom</c>, as the <c>Current User</c> stream states it.
    /// Zero when that stream is missing or unreadable, in which case the stream is searched.
    /// </param>
    /// <param name="diagnostics">Where to record what could not be read.</param>
    public static PptPersistDirectory Read(
        DffRecordBuffer stream, uint currentUserEdit, List<Diagnostic> diagnostics)
    {
        ArgumentNullException.ThrowIfNull(stream);
        ArgumentNullException.ThrowIfNull(diagnostics);

        UserEdit? newest = ReadUserEdit(stream, (int)currentUserEdit);
        if (newest is null)
        {
            newest = FindLastUserEdit(stream);
            if (newest is not null && currentUserEdit != 0)
            {
                diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Warning, "PL2400",
                    $"The 'Current User' stream points at offset {currentUserEdit}, which is not a "
                    + "UserEditAtom; the last such atom in the stream was used instead."));
            }
        }

        if (newest is null)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2401",
                "The document stream contains no UserEditAtom, so the persist directory that "
                + "locates the current version of each slide cannot be built."));
            return new PptPersistDirectory([], 0);
        }

        Dictionary<uint, uint> offsets = [];
        UserEdit edit = newest.Value;

        // The chain is walked by strictly decreasing offset. That is both how the format is
        // written and the cycle guard: an edit atom pointing forwards, or at itself, ends the
        // walk rather than looping. LibreOffice does the same at svdfppt.cxx:1401.
        int position = edit.End;
        int guard = 0;

        while (position > 0 && guard++ < MaxEditChain)
        {
            ReadBlock(stream, edit, offsets, diagnostics);

            if (edit.PreviousEdit == 0 || edit.PreviousEdit >= (uint)position) break;
            UserEdit? previous = ReadUserEdit(stream, (int)edit.PreviousEdit);
            if (previous is null) break;

            position = (int)edit.PreviousEdit;
            edit = previous.Value;
        }

        return new PptPersistDirectory(offsets, newest.Value.DocumentReference);
    }

    /// <summary>
    /// How many edit sessions the chain may hold before the walk gives up.
    /// </summary>
    /// <remarks>
    /// A document saved once a day for a decade has a few thousand. The cap is a backstop
    /// against a file whose offsets happen to decrease forever in a very large stream.
    /// </remarks>
    public const int MaxEditChain = 100_000;

    /// <summary>Reads one edit's persist block into the map, newest entry winning.</summary>
    private static void ReadBlock(
        DffRecordBuffer stream, UserEdit edit, Dictionary<uint, uint> offsets,
        List<Diagnostic> diagnostics)
    {
        if (edit.PersistDirectory == 0
            || !stream.TryReadHeader((int)edit.PersistDirectory, out DffRecordHeader header)
            || header.Type != PptRecordTypes.PersistPtrIncrementalBlock)
        {
            return;
        }

        ReadOnlySpan<byte> content = stream.Content(header);
        int position = 0;

        while (position + 4 <= content.Length)
        {
            uint packed = DffRecordBuffer.ReadUInt32(content[position..]);
            position += 4;

            uint first = packed & 0x000FFFFF;
            uint count = packed >> 20;

            for (uint i = 0; i < count && position + 4 <= content.Length; i++, position += 4)
            {
                uint offset = DffRecordBuffer.ReadUInt32(content[position..]);

                // An offset past the end of the stream is a corrupt entry; recording it would
                // turn a bad directory into a bad slide rather than a missing one.
                if (offset >= (uint)stream.Length)
                {
                    diagnostics.Add(new Diagnostic(
                        DiagnosticSeverity.Warning, "PL2402",
                        $"Persist entry {first + i} points at offset {offset}, past the end of the "
                        + $"{stream.Length}-byte document stream; it was ignored."));
                    continue;
                }

                // Newest wins: the walk is newest first, so an id already present came from a
                // later edit and this one is the superseded copy.
                offsets.TryAdd(first + i, offset);
            }
        }
    }

    /// <summary>Reads a <c>UserEditAtom</c> at an offset, or fails when there is not one there.</summary>
    private static UserEdit? ReadUserEdit(DffRecordBuffer stream, int offset)
    {
        if (offset <= 0 || !stream.TryReadHeader(offset, out DffRecordHeader header)) return null;
        if (header.Type != PptRecordTypes.UserEditAtom) return null;

        ReadOnlySpan<byte> content = stream.Content(header);
        if (content.Length < 24) return null;

        return new UserEdit(
            PreviousEdit: DffRecordBuffer.ReadUInt32(content[8..]),
            PersistDirectory: DffRecordBuffer.ReadUInt32(content[12..]),
            DocumentReference: DffRecordBuffer.ReadUInt32(content[16..]),
            MaxPersistWritten: DffRecordBuffer.ReadUInt32(content[20..]),
            End: stream.EndOf(header));
    }

    /// <summary>
    /// The last <c>UserEditAtom</c> among the stream's top-level records.
    /// </summary>
    /// <remarks>
    /// The fallback for a file whose <c>Current User</c> stream is missing or stale — common in
    /// files that have been through a repair tool. Only top-level records are considered,
    /// because an edit atom is always written at the stream's top level and scanning inside
    /// containers would find the bytes of unrelated payloads that happen to look like one.
    /// </remarks>
    private static UserEdit? FindLastUserEdit(DffRecordBuffer stream)
    {
        UserEdit? found = null;
        foreach (DffRecordHeader header in stream.Range(0, stream.Length))
        {
            if (header.Type != PptRecordTypes.UserEditAtom) continue;
            if (ReadUserEdit(stream, header.Position) is { } edit) found = edit;
        }

        return found;
    }

    /// <summary>One edit session, as far as finding the current objects needs it.</summary>
    private readonly record struct UserEdit(
        uint PreviousEdit,
        uint PersistDirectory,
        uint DocumentReference,
        uint MaxPersistWritten,
        int End);
}

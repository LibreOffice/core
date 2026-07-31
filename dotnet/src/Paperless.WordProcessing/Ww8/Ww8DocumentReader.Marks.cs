using System.Buffers.Binary;
using System.Text;
using Paperless.Core.Extraction;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.Ww8;

/// <content>
/// What the walk records rather than resolves: bookmarks, tracked changes and a field's definition.
/// </content>
public sealed partial class Ww8DocumentReader
{
    private readonly WritingMarkBuilder _marks = new();

    private List<BookmarkEvent>? _bookmarkEvents;
    private List<string>? _revisionAuthors;

    /// <summary>The marks the walk recorded: tracked changes, bookmarks and fields.</summary>
    public WritingMarks Marks => _marks.Build();

    /// <summary>A bookmark's start or end, in character-position space.</summary>
    private readonly record struct BookmarkEvent(int Position, int Index, string Name, bool IsStart);

    /// <summary>
    /// The bookmarks, from the three tables that between them state one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The two position tables are not parallel arrays.</strong> <c>PlcfBkf</c> holds the
    /// starts, each with a four-byte record whose first sixteen bits are an <em>index into</em>
    /// <c>PlcfBkl</c> — so bookmark <c>i</c> ends at <c>PlcfBkl[record(i)]</c> and not at
    /// <c>PlcfBkl[i]</c>. LibreOffice does exactly this in <c>WW8PLCFx_Book::GetLen</c>
    /// (<c>ww8scan.cxx</c>): it reads the start's record, takes it as <c>nEndIdx</c>, and looks the
    /// end position up by it. Pairing by ordinal happens to agree whenever a document's bookmarks
    /// neither nest nor overlap, which is most of them — so the mistake survives every simple test
    /// and mangles exactly the documents that use bookmarks seriously.
    /// </para>
    /// <para>
    /// The count is the smallest of the three tables', because a file whose name table and position
    /// tables disagree is malformed and reading past the shorter one names bookmarks after whatever
    /// followed it in the stream. LibreOffice clamps the same three the same way.
    /// </para>
    /// </remarks>
    private List<BookmarkEvent> BookmarkEvents()
    {
        if (_bookmarkEvents is not null) return _bookmarkEvents;

        _bookmarkEvents = [];

        List<string> names = ReadStringTable(Slice(Ww8FibTable.BookmarkNames));
        Ww8Plcf starts = PlcfOf(Ww8FibTable.BookmarkStarts, recordSize: 4);
        Ww8Plcf ends = PlcfOf(Ww8FibTable.BookmarkEnds, recordSize: 0);

        int count = Math.Min(names.Count, Math.Min(starts.Count, ends.Count));
        for (int i = 0; i < count; i++)
        {
            ReadOnlySpan<byte> record = starts.Record(i);
            if (record.Length < 2) continue;

            int endIndex = BinaryPrimitives.ReadUInt16LittleEndian(record);
            if (endIndex < 0 || endIndex >= ends.Positions.Count) continue;

            string name = names[i];
            if (name.Length == 0) continue;

            _bookmarkEvents.Add(new BookmarkEvent(starts.Positions[i], i, name, IsStart: true));
            _bookmarkEvents.Add(new BookmarkEvent(ends.Positions[endIndex], i, name, IsStart: false));
        }

        // Sorted so the walk can fire them with one advancing index rather than a search per
        // character. Ends before starts at the same position, so that two adjacent bookmarks do not
        // nest — the same order LibreOffice's own iterator takes pains to produce.
        _bookmarkEvents.Sort(static (left, right) =>
        {
            int byPosition = left.Position.CompareTo(right.Position);
            if (byPosition != 0) return byPosition;
            return left.IsStart.CompareTo(right.IsStart);
        });

        return _bookmarkEvents;
    }

    /// <summary>The revision authors, from <c>SttbfRMark</c>, indexed by the revision sprms.</summary>
    private string? RevisionAuthor(ushort index)
    {
        _revisionAuthors ??= ReadStringTable(Slice(Ww8FibTable.RevisionAuthors));
        return index < _revisionAuthors.Count && _revisionAuthors[index].Length > 0
            ? _revisionAuthors[index]
            : null;
    }

    /// <summary>
    /// Reads an <c>STTBF</c>: a counted table of length-prefixed strings.
    /// </summary>
    /// <remarks>
    /// Not the same shape as the <c>GrpXstAtnOwners</c> the comment authors come from, which has no
    /// header at all. Here the first sixteen bits are either the string count or the marker 0xFFFF
    /// that says the strings are UTF-16 and the real count follows; then a sixteen-bit
    /// <c>cbExtra</c>, which is per-entry trailing data that has to be skipped or every name after
    /// the first is read from the middle of the previous entry. <c>WW8ReadSTTBF</c> in
    /// <c>ww8scan.cxx</c> is the same three steps.
    /// </remarks>
    private static List<string> ReadStringTable(ReadOnlySpan<byte> table)
    {
        List<string> strings = [];
        if (table.Length < 6) return strings;

        int position = 0;
        int marker = BinaryPrimitives.ReadUInt16LittleEndian(table);
        position += 2;

        bool unicode = marker == 0xFFFF;
        int count = marker;
        if (unicode)
        {
            count = BinaryPrimitives.ReadUInt16LittleEndian(table[position..]);
            position += 2;
        }

        int extra = BinaryPrimitives.ReadUInt16LittleEndian(table[position..]);
        position += 2;

        for (int i = 0; i < count; i++)
        {
            int characters;
            if (unicode)
            {
                if (position + 2 > table.Length) break;
                characters = BinaryPrimitives.ReadUInt16LittleEndian(table[position..]);
                position += 2;
            }
            else
            {
                if (position + 1 > table.Length) break;
                characters = table[position];
                position += 1;
            }

            int bytes = unicode ? characters * 2 : characters;
            if (bytes < 0 || position + bytes > table.Length) break;

            strings.Add(unicode
                ? Encoding.Unicode.GetString(table.Slice(position, bytes))
                : Encoding.Latin1.GetString(table.Slice(position, bytes)));

            position += bytes + extra;
        }

        return strings;
    }

    // ------------------------------------------------------------------- the walk's hooks

    /// <summary>
    /// Fires every bookmark start and end that falls at or before a position.
    /// </summary>
    /// <remarks>
    /// At or before, rather than at: a bookmark can be anchored to a paragraph mark or to a field
    /// character, neither of which contributes text, and testing for equality would leave such a
    /// bookmark unfired for the rest of the document.
    /// </remarks>
    private void FireBookmarks(WalkState state, int position)
    {
        List<BookmarkEvent> events = BookmarkEvents();
        while (state.BookmarkIndex < events.Count && events[state.BookmarkIndex].Position <= position)
        {
            BookmarkEvent mark = events[state.BookmarkIndex++];
            string key = mark.Index.ToString(System.Globalization.CultureInfo.InvariantCulture);

            if (mark.IsStart) _marks.OpenBookmark(key, mark.Name, _marks.At(OffsetIn(state)));
            else _marks.CloseBookmark(key, _marks.At(OffsetIn(state)));
        }
    }

    /// <summary>How far into the paragraph being built the walk has got.</summary>
    private static int OffsetIn(WalkState state)
    {
        int offset = state.Text.Length;
        foreach (ContentRun run in state.Runs) offset += run.Text.Length;
        return offset;
    }

    /// <summary>The paragraph's text so far, the half-built run included.</summary>
    private static string ParagraphTextIn(WalkState state)
    {
        StringBuilder text = new();
        foreach (ContentRun run in state.Runs) text.Append(run.Text);
        return text.Append(state.Text).ToString();
    }

    /// <summary>That text between two offsets, clamped to what exists.</summary>
    private static string SliceIn(WalkState state, int start, int end)
    {
        string text = ParagraphTextIn(state);
        int from = Math.Clamp(start, 0, text.Length);
        int to = Math.Clamp(end, from, text.Length);
        return text[from..to];
    }

    /// <summary>
    /// Opens, extends and closes the tracked changes the character formatting states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// WW8 states a revision as a run property rather than as a wrapper, so a change begins where the
    /// flag turns on and ends where it turns off or where its author changes — which is why this is
    /// driven from the resolved format rather than from any marker in the text.
    /// </para>
    /// <para>
    /// Called before the caller drops deleted and hidden text, because a deletion's whole record is
    /// the text about to be dropped: the range is empty, since nothing between its two positions
    /// reaches the document, and the words are carried on the change instead.
    /// </para>
    /// </remarks>
    private void RecordRevisions(WalkState state, Ww8CharacterFormat format, string text)
    {
        Revision insertion = format.IsInserted
            ? new Revision(RevisionAuthor(format.InsertionAuthor), Ww8DateTime.Decode(format.InsertionDate))
            : default;
        Revision deletion = format.IsDeleted
            ? new Revision(RevisionAuthor(format.DeletionAuthor), Ww8DateTime.Decode(format.DeletionDate))
            : default;

        Track(state, InsertionKey, WritingChangeKind.Insertion, format.IsInserted, insertion, null);
        Track(state, DeletionKey, WritingChangeKind.Deletion, format.IsDeleted, deletion, text);
    }

    private void Track(
        WalkState state,
        string key,
        WritingChangeKind kind,
        bool active,
        Revision revision,
        string? deletedText)
    {
        bool open = _marks.HasOpenChange(key);
        Revision current = key == InsertionKey ? state.OpenInsertion : state.OpenDeletion;

        if (open && (!active || current != revision))
        {
            _marks.CloseChange(key, _marks.At(OffsetIn(state)));
            open = false;
        }

        if (active && !open)
        {
            _marks.OpenChange(key, kind, revision.Author, revision.When, _marks.At(OffsetIn(state)));
            if (key == InsertionKey) state.OpenInsertion = revision;
            else state.OpenDeletion = revision;
        }

        if (active && deletedText is { Length: > 0 }) _marks.AppendChangeText(key, deletedText);
    }

    /// <summary>Closes whatever revisions are open, at a paragraph's end or a range's.</summary>
    /// <remarks>
    /// An insertion's text is taken from the paragraph rather than accumulated, because it is in the
    /// paragraph: only a deletion's has to be kept separately, and only because it is nowhere else.
    /// </remarks>
    private void CloseRevisions(WalkState state)
    {
        if (_marks.HasOpenChange(InsertionKey))
        {
            _marks.CloseChange(InsertionKey, _marks.At(OffsetIn(state)));
        }
        if (_marks.HasOpenChange(DeletionKey))
        {
            _marks.CloseChange(DeletionKey, _marks.At(OffsetIn(state)));
        }
    }

    private const string InsertionKey = "ins";
    private const string DeletionKey = "del";

    /// <summary>One revision's identity, as the sprms state it.</summary>
    private readonly record struct Revision(string? Author, DateTime? When);

    // ---------------------------------------------------------------------------- fields

    /// <summary>Records a field at its <c>U+0015</c> end, instruction and cached result both.</summary>
    private void RecordField(WalkState state)
    {
        if (!state.FieldResultStarted) return;

        int end = OffsetIn(state);
        _marks.AddField(
            state.Instruction.ToString(),
            SliceIn(state, state.FieldResultOffset, end),
            state.FieldResultStart,
            _marks.At(end));

        state.FieldResultStarted = false;
        state.FieldResultStart = null;
    }
}

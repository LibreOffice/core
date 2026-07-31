namespace Paperless.WordProcessing.Model;

/// <summary>
/// Collects a document's tracked changes, bookmarks and fields as the reader walks it.
/// </summary>
/// <remarks>
/// <para>
/// Shared by all four readers, because the four formats disagree about how a mark is spelled and
/// agree about what one is. Each reader announces its paragraphs and hands over an offset; the
/// pairing of a start with an end, the materialising of the paragraphs positions point into, and the
/// document order that makes two positions comparable all happen here.
/// </para>
/// <para>
/// <strong>Only the paragraphs a mark touches are materialised.</strong> An unmarked document
/// allocates nothing beyond an integer per paragraph, which is what lets this run inside the
/// extraction pass rather than beside it — extraction is the common case and must not pay for the
/// layout model.
/// </para>
/// <para>
/// The offsets a reader hands over count the characters of the paragraph's own runs. They are not
/// the format's own offsets and are not meant to be: DOCX counts XML text nodes, WW8 counts
/// character positions in a piece table, and both include text — a field's instruction, a deletion —
/// that the extracted paragraph does not. Counting what the model holds is what makes the four
/// formats' marks comparable with each other.
/// </para>
/// </remarks>
internal sealed class WritingMarkBuilder
{
    /// <summary>Holds the paragraphs the positions point into, so each has an owner.</summary>
    private readonly WritingBody _paragraphs = new(WritingBodyKind.Body);

    // The open paragraphs, innermost last. A stack because a note's body is read in the middle of
    // the paragraph that anchors it, in three of the four readers.
    private readonly List<int> _openOrder = [];
    private readonly List<WritingParagraph?> _openNode = [];

    private readonly List<WritingChange> _changes = [];
    private readonly List<WritingBookmark> _bookmarks = [];
    private readonly List<WritingField> _fields = [];

    private readonly Dictionary<string, PendingBookmark> _openBookmarks = new(StringComparer.Ordinal);
    private readonly Dictionary<string, PendingChange> _openChanges = new(StringComparer.Ordinal);

    private int _nextOrder;
    private WritingParagraph? _lastClosed;

    /// <summary>Whether anything has been recorded, so a caller can skip building an empty set.</summary>
    public bool IsEmpty => _changes.Count == 0 && _bookmarks.Count == 0 && _fields.Count == 0;

    /// <summary>Announces the start of a paragraph, taking the next document-order index.</summary>
    /// <remarks>
    /// At the start rather than at the end, so that the order reflects where paragraphs begin. Taken
    /// at the end instead, a footnote's paragraphs — which finish before the paragraph that anchors
    /// them — would sort in front of it.
    /// </remarks>
    public void OpenParagraph()
    {
        _openOrder.Add(_nextOrder++);
        _openNode.Add(null);
    }

    /// <summary>Announces the end of a paragraph, giving the model paragraph its text.</summary>
    /// <param name="text">
    /// The paragraph's text as extraction produced it, without any generated list label: the label is
    /// computed rather than stored and no format's offsets count it.
    /// </param>
    public void CloseParagraph(string text)
    {
        if (_openOrder.Count == 0) return;

        WritingParagraph? node = _openNode[^1];
        _openOrder.RemoveAt(_openOrder.Count - 1);
        _openNode.RemoveAt(_openNode.Count - 1);

        if (node is null) return;

        node.Append(text);
        _lastClosed = node;
        FillTextFrom(node);
    }

    /// <summary>
    /// Fills in the text of the marks that cover part of a paragraph, now that it has one.
    /// </summary>
    /// <remarks>
    /// A change's words and a field's cached result are both simply the paragraph's text between the
    /// mark's two positions — but a reader that meets the mark's end before its paragraph's has no
    /// paragraph to take them from, which is ODF's case, where a change is delimited by two empty
    /// elements and a field's result is its element's content. Filling them here means the readers
    /// that <em>can</em> slice the text as they go say so by having set it already, and the rest get
    /// the same answer without a second walk. Only the tail of each list is scanned, since marks are
    /// appended in the order they close.
    /// </remarks>
    private void FillTextFrom(WritingParagraph node)
    {
        string text = node.Text;

        for (int i = _changes.Count - 1; i >= 0; i--)
        {
            WritingChange change = _changes[i];
            if (!ReferenceEquals(change.Range.Start.Paragraph, node)) break;
            if (change.Text is not null || change.Range.IsEmpty) continue;
            if (!ReferenceEquals(change.Range.End.Paragraph, node)) continue;

            _changes[i] = change with { Text = Between(text, change.Range) };
        }

        for (int i = _fields.Count - 1; i >= 0; i--)
        {
            WritingField field = _fields[i];
            if (!ReferenceEquals(field.Range.Start.Paragraph, node)) break;
            if (field.Result is not null) continue;
            if (!ReferenceEquals(field.Range.End.Paragraph, node)) continue;

            _fields[i] = field with { Result = Between(text, field.Range) };
        }
    }

    private static string Between(string text, WritingRange range)
    {
        int from = Math.Clamp(range.Start.Offset, 0, text.Length);
        int to = Math.Clamp(range.End.Offset, from, text.Length);
        return text[from..to];
    }

    /// <summary>
    /// A position at an offset into the paragraph being read.
    /// </summary>
    /// <remarks>
    /// Null when no paragraph is open and none has closed — a mark before the document's first
    /// paragraph, which has nothing to be a position in. A mark <em>between</em> two paragraphs takes
    /// the end of the one before it, which is where the formats put it: a bookmark declared at block
    /// level covers from there to wherever its end lands.
    /// </remarks>
    public WritingPosition? At(int offset)
    {
        if (_openOrder.Count > 0)
        {
            WritingParagraph node = _openNode[^1] ?? Materialise(_openOrder.Count - 1);
            return new WritingPosition(node, Math.Max(0, offset));
        }

        return _lastClosed is { } previous
            ? new WritingPosition(previous, previous.Length)
            : null;
    }

    /// <summary>Materialises the model paragraph for an open frame, keeping its document order.</summary>
    private WritingParagraph Materialise(int frame)
    {
        WritingParagraph node = new();
        int order = _openOrder[frame];

        // Assigned rather than left to WritingDocument.AssignDocumentOrder, which numbers a whole
        // tree: these paragraphs are the sparse few a mark touched, and their indexes have to be the
        // ones the walk gave them or two positions in different paragraphs would compare by offset.
        node.DocumentOrder = order;
        node.DocumentOrderEnd = order + 1;

        _paragraphs.Add(node);
        _openNode[frame] = node;
        return node;
    }

    // ------------------------------------------------------------------------- bookmarks

    /// <summary>Records the start of a bookmark, to be closed by the same key.</summary>
    /// <param name="key">
    /// What the format pairs the two halves by: a <c>w:id</c> in DOCX, the name itself in RTF and
    /// ODF, an index in WW8. Not the name, because two formats let one name be reused and none of
    /// them pairs by it.
    /// </param>
    /// <param name="name">The bookmark's name.</param>
    /// <param name="start">Where it begins.</param>
    public void OpenBookmark(string key, string name, WritingPosition? start)
    {
        if (start is not { } from) return;
        _openBookmarks[key] = new PendingBookmark(name, from);
    }

    /// <summary>Closes a bookmark opened under a key, discarding an end that pairs with nothing.</summary>
    public void CloseBookmark(string key, WritingPosition? end)
    {
        if (!_openBookmarks.Remove(key, out PendingBookmark pending)) return;
        if (end is not { } to) return;

        _bookmarks.Add(new WritingBookmark(pending.Name, new WritingRange(pending.Start, to)));
    }

    /// <summary>Records a bookmark whose two ends are already known.</summary>
    /// <remarks>
    /// For the format that states a bookmark as a pair of tables rather than as a pair of markers in
    /// the text: WW8 knows both positions before the walk starts, so pairing them through
    /// <see cref="OpenBookmark"/> would be a state machine standing in for a lookup.
    /// </remarks>
    public void AddBookmark(string name, WritingPosition? start, WritingPosition? end)
    {
        if (start is not { } from || end is not { } to) return;
        _bookmarks.Add(new WritingBookmark(name, new WritingRange(from, to)));
    }

    /// <summary>Records a bookmark that marks a position rather than a span.</summary>
    public void AddPointBookmark(string name, WritingPosition? at)
    {
        if (at is not { } position) return;
        _bookmarks.Add(new WritingBookmark(name, new WritingRange(position, position)));
    }

    // --------------------------------------------------------------------- tracked changes

    /// <summary>Records the start of a tracked change, to be closed by the same key.</summary>
    /// <param name="key">
    /// What pairs the halves: a change region's id in ODF, and in the two formats that state a change
    /// as a run property — WW8's <c>sprmCFRMark*</c> and RTF's <c>\revised</c>/<c>\deleted</c> — the
    /// kind alone, since only one insertion and one deletion can be in force at a time.
    /// </param>
    /// <param name="kind">What the change did.</param>
    /// <param name="author">Who made it, or null when the file names nobody.</param>
    /// <param name="timestamp">When, or null when the file records no date.</param>
    /// <param name="start">Where the change begins.</param>
    public void OpenChange(
        string key,
        WritingChangeKind kind,
        string? author,
        DateTime? timestamp,
        WritingPosition? start)
    {
        if (start is not { } from) return;
        _openChanges[key] = new PendingChange(kind, author, timestamp, from, new System.Text.StringBuilder());
    }

    /// <summary>True while a change is open under a key.</summary>
    public bool HasOpenChange(string key) => _openChanges.ContainsKey(key);

    /// <summary>
    /// Adds to the text an open change concerns, which for a deletion is the only record of it.
    /// </summary>
    public void AppendChangeText(string key, string text)
    {
        if (_openChanges.TryGetValue(key, out PendingChange? pending)) pending.Text.Append(text);
    }

    /// <summary>Closes a tracked change opened under a key.</summary>
    public void CloseChange(string key, WritingPosition? end)
    {
        if (!_openChanges.Remove(key, out PendingChange? pending)) return;
        if (end is not { } to) return;

        _changes.Add(new WritingChange(pending.Kind, new WritingRange(pending.Start, to))
        {
            Author = pending.Author,
            Timestamp = pending.Timestamp,
            Text = pending.Text.Length == 0 ? null : pending.Text.ToString(),
        });
    }

    /// <summary>Records a tracked change whose extent is already known.</summary>
    public void AddChange(
        WritingChangeKind kind,
        string? author,
        DateTime? timestamp,
        string? text,
        WritingPosition? start,
        WritingPosition? end)
    {
        if (start is not { } from || end is not { } to) return;

        _changes.Add(new WritingChange(kind, new WritingRange(from, to))
        {
            Author = author,
            Timestamp = timestamp,
            Text = string.IsNullOrEmpty(text) ? null : text,
        });
    }

    // ---------------------------------------------------------------------------- fields

    /// <summary>Records a field, its instruction and the result the file cached.</summary>
    /// <param name="instruction">The instruction, for the three formats that have one.</param>
    /// <param name="result">The cached result, which is what a reader saw.</param>
    /// <param name="start">Where the result begins.</param>
    /// <param name="end">Where it ends.</param>
    /// <param name="kind">
    /// What the field computes, when the caller knows it from something other than the instruction —
    /// which is ODF's case, where a field is a typed element and there is no instruction to read.
    /// </param>
    public void AddField(
        string? instruction,
        string? result,
        WritingPosition? start,
        WritingPosition? end,
        WritingFieldKind? kind = null)
    {
        if (start is not { } from || end is not { } to) return;

        if (kind is null && string.IsNullOrWhiteSpace(instruction) && string.IsNullOrEmpty(result))
        {
            // No instruction, no result and no kind is not a field: it is a fragment of markup that
            // looked like one. A caller that names a kind is asserting otherwise and is believed —
            // which is what ODF needs, where the element *is* the definition and there is nothing
            // else to judge by.
            return;
        }

        _fields.Add(new WritingField(kind ?? FieldInstructions.KindOf(instruction), new WritingRange(from, to))
        {
            Instruction = string.IsNullOrWhiteSpace(instruction) ? null : instruction.Trim(),
            Result = result,
        });
    }

    /// <summary>The marks collected, in the order they were recorded.</summary>
    public WritingMarks Build()
        => IsEmpty ? WritingMarks.Empty : new WritingMarks(_changes, _bookmarks, _fields);

    private readonly record struct PendingBookmark(string Name, WritingPosition Start);

    private sealed record PendingChange(
        WritingChangeKind Kind,
        string? Author,
        DateTime? Timestamp,
        WritingPosition Start,
        System.Text.StringBuilder Text);
}

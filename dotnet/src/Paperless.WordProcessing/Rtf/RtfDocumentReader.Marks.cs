using System.Text;
using Paperless.Core.Extraction;
using Paperless.WordProcessing.Model;
using Paperless.WordProcessing.Ww8;

namespace Paperless.WordProcessing.Rtf;

/// <content>
/// What the token stream records rather than resolves: tracked changes, bookmarks and fields.
/// </content>
/// <remarks>
/// Recorded as the content walk runs, for the same reason the layout properties are: RTF is a token
/// stream with nothing to revisit, and a second pass would mean running the whole state machine
/// again — encoding, destinations and all — with two runs free to disagree.
/// </remarks>
public sealed partial class RtfDocumentReader
{
    private readonly WritingMarkBuilder _marks = new();

    /// <summary>
    /// The revision authors from <c>{\*\revtbl}</c>, which <c>\revauth</c> indexes.
    /// </summary>
    /// <remarks>
    /// Zero-based, and its conventional first entry is <c>Unknown</c> rather than a real person —
    /// LibreOffice's importer fills the same map with <c>m_aAuthors[m_aAuthors.size()] = aName</c>
    /// as each entry closes (<c>rtfdocumentimpl.cxx</c>, <c>Destination::REVISIONENTRY</c>) and then
    /// looks <c>\revauthN</c> up by <c>N</c> directly. Treating the table as one-based names the
    /// wrong person for every revision in the document, and names nobody for the last.
    /// </remarks>
    private readonly List<string> _revisionAuthors = [];

    private int _fieldResultDepth = -1;
    private int _fieldResultOffset;
    private WritingPosition? _fieldResultStart;
    private string? _fieldInstruction;

    /// <summary>The marks the walk recorded.</summary>
    public WritingMarks Marks => _marks.Build();

    /// <summary>How far into the flow's half-built paragraph the walk has got.</summary>
    private static int OffsetIn(Flow flow)
    {
        int offset = flow.PendingText.Length;
        foreach (ContentRun run in flow.PendingRuns) offset += run.Text.Length;
        return offset;
    }

    /// <summary>That paragraph's text so far.</summary>
    private static string ParagraphTextIn(Flow flow)
    {
        StringBuilder text = new();
        foreach (ContentRun run in flow.PendingRuns) text.Append(run.Text);
        return text.Append(flow.PendingText).ToString();
    }

    private static string SliceIn(Flow flow, int start, int end)
    {
        string text = ParagraphTextIn(flow);
        int from = Math.Clamp(start, 0, text.Length);
        int to = Math.Clamp(end, from, text.Length);
        return text[from..to];
    }

    /// <summary>A position at the current point of the current flow's paragraph.</summary>
    private WritingPosition? Here() => _marks.At(OffsetIn(CurrentFlow));

    // ------------------------------------------------------------------------- bookmarks

    /// <summary>
    /// Records a bookmark half, whose name is the destination's own text.
    /// </summary>
    /// <remarks>
    /// RTF pairs the two halves <em>by name</em>, not by an id — <c>{\*\bkmkstart foo}</c> and
    /// <c>{\*\bkmkend foo}</c> — which is the one place among the four formats where the name is
    /// also the key. So a document that reuses a name has bookmarks that cannot be told apart, and
    /// this pairs the end with the most recent unclosed start of that name.
    /// </remarks>
    private void RecordBookmark(GroupState state, bool start)
    {
        string name = state.Collected.ToString().Trim();
        if (name.Length == 0) return;

        if (start) _marks.OpenBookmark(name, name, Here());
        else _marks.CloseBookmark(name, Here());
    }

    // --------------------------------------------------------------------- tracked changes

    /// <summary>Records a revision-table entry, whose text ends at a semicolon.</summary>
    private void RecordRevisionAuthor(GroupState state)
    {
        string name = state.Collected.ToString().TrimEnd(';').Trim();
        if (name.Length > 0 || _revisionAuthors.Count > 0) _revisionAuthors.Add(name);
    }

    private string? RevisionAuthor(int index)
        => index >= 0 && index < _revisionAuthors.Count && _revisionAuthors[index].Length > 0
            ? _revisionAuthors[index]
            : null;

    /// <summary>
    /// Opens and closes the insertion the character state describes, as text is appended.
    /// </summary>
    /// <remarks>
    /// <c>\revised</c> is a toggle rather than a wrapper, so an insertion begins at the first
    /// character it covers and ends where the toggle goes off — which in practice is where the
    /// group holding it closes, since that is how every producer writes one.
    /// </remarks>
    private void TrackInsertion(GroupState state)
    {
        bool open = _marks.HasOpenChange(InsertionKey);
        string? author = RevisionAuthor(state.RevisionAuthor);
        DateTime? when = Ww8DateTime.Decode(state.RevisionDate);

        if (open && (!state.Revised || _openInsertionAuthor != author || _openInsertionDate != when))
        {
            _marks.CloseChange(InsertionKey, Here());
            open = false;
        }

        if (state.Revised && !open)
        {
            _marks.OpenChange(InsertionKey, WritingChangeKind.Insertion, author, when, Here());
            _openInsertionAuthor = author;
            _openInsertionDate = when;
        }
    }

    /// <summary>Closes an insertion left open at a paragraph's end.</summary>
    private void CloseInsertion(Flow flow)
    {
        if (_marks.HasOpenChange(InsertionKey))
            _marks.CloseChange(InsertionKey, _marks.At(OffsetIn(flow)));
    }

    /// <summary>
    /// Records a deletion at the position its text was removed from.
    /// </summary>
    /// <remarks>
    /// The text arrives because <c>\deleted</c> routes the group to a destination that collects it
    /// rather than one that drops it — the extracted document still has none of it, and the record
    /// is the only place the words survive. The range is empty for the same reason.
    /// </remarks>
    private void RecordDeletion(GroupState state)
    {
        string removed = state.Collected.ToString();
        WritingPosition? at = Here();

        _marks.AddChange(
            WritingChangeKind.Deletion,
            RevisionAuthor(state.DeletionAuthor),
            Ww8DateTime.Decode(state.DeletionDate),
            removed,
            at,
            at);
    }

    private const string InsertionKey = "ins";

    private string? _openInsertionAuthor;
    private DateTime? _openInsertionDate;

    // ---------------------------------------------------------------------------- fields

    /// <summary>Notes where a field's cached result begins.</summary>
    private void BeginFieldResult()
    {
        _fieldResultDepth = _groupDepth;
        _fieldResultOffset = OffsetIn(CurrentFlow);
        _fieldResultStart = Here();
    }

    /// <summary>Records the field once its <c>\fldrslt</c> group has closed.</summary>
    private void EndFieldResult()
    {
        _marks.AddField(
            _fieldInstruction,
            SliceIn(CurrentFlow, _fieldResultOffset, OffsetIn(CurrentFlow)),
            _fieldResultStart,
            Here());

        _fieldResultDepth = -1;
        _fieldResultStart = null;
        _fieldInstruction = null;
    }
}

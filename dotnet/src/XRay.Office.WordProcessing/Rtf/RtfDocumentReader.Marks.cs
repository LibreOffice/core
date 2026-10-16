using System.Globalization;
using System.Text;
using XRay.Office.Core.Extraction;
using XRay.Office.WordProcessing.Model;
using XRay.Office.WordProcessing.Ww8;

namespace XRay.Office.WordProcessing.Rtf;

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
    private int _fieldResultLayoutOffset;
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
    /// The naming the importer gives this document's bookmarks, which is not the file's own.
    /// </summary>
    /// <remarks>See <see cref="RtfBookmarkRotation"/>: RTF states a bookmark half's name before its
    /// id and <c>DomainMapper_Impl::SetBookmarkName</c> expects the opposite order, so every name
    /// after the first lands on the bookmark before it.</remarks>
    private readonly RtfBookmarkRotation _bookmarkNames = new();

    /// <summary>The layout places of bookmarks that have opened and not yet closed, by id.</summary>
    private readonly Dictionary<int, RtfLayoutAnchor> _openLayoutAnchors = [];

    /// <summary>The depth of the group a <c>\bkmkstart</c> or <c>\bkmkend</c> opened.</summary>
    /// <remarks>
    /// <para>
    /// A group nested inside a bookmark's destination inherits that destination, so without this it
    /// closes as a second half of its own — and a spurious half is not free under the rotation: it
    /// takes an id, and an <em>end</em> that names nothing takes <c>m_aBookmarks[""]</c>, which
    /// <c>std::map</c> value-initialises to <b>0</b> and which therefore closes the document's first
    /// bookmark under the wrong name. <c>RTFDocumentImpl::popState</c> guards it with
    /// <c>if (&amp;getDestinationText() != getCurrentDestinationText()) break; // not for nested
    /// group</c> (<c>rtfdocumentimpl.cxx</c>:2736-2740, :2751-2755).
    /// </para>
    /// <para>
    /// The nested group's text is <em>not</em> dropped with it, because that same test is what says
    /// the two share one buffer: 26.2.4.2 reads <c>{\*\bkmkend {x}A}</c> as a half named
    /// <c>xA</c>, which is why <see cref="_bookmarkName"/> is the reader's and not the group's.
    /// </para>
    /// </remarks>
    private int _bookmarkDepth = -1;

    /// <summary>The name the current bookmark destination has collected, nested groups included.</summary>
    private readonly StringBuilder _bookmarkName = new();

    /// <summary>Opens a bookmark half's name, which the group that carries it will close.</summary>
    private void BeginBookmarkName()
    {
        _bookmarkDepth = _groupDepth;
        _bookmarkName.Clear();
    }

    /// <summary>Adds a stretch of a bookmark's name, from that group or from one inside it.</summary>
    private void AppendBookmarkName(string text) => _bookmarkName.Append(text);

    /// <summary>
    /// Records a bookmark half, whose name is the destination's own text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// RTF pairs the two halves <em>by name</em> — <c>{\*\bkmkstart foo}</c> and
    /// <c>{\*\bkmkend foo}</c> — but the importer does not: it turns each name into an id as the
    /// destination closes and pairs the halves by <em>that</em>, so the key here is the id and the
    /// name a bookmark ends up with is settled only when it closes.
    /// </para>
    /// <para>
    /// Which matters because the name it settles on is usually another bookmark's. The rotation is
    /// the reference's own and is reproduced rather than corrected, because a <c>REF</c> field
    /// expands from whichever bookmark holds its name — see <see cref="RtfReferenceFields"/>.
    /// </para>
    /// </remarks>
    private void RecordBookmark(bool start)
    {
        if (_groupDepth != _bookmarkDepth) return;
        _bookmarkDepth = -1;

        string name = _bookmarkName.ToString().Trim();
        _bookmarkName.Clear();

        RtfBookmarkRotation.Half half = _bookmarkNames.Take(name, start);
        string key = half.Id.ToString(CultureInfo.InvariantCulture);

        // A named place in the paragraph being laid out, for a PDF's destinations. The name is the one the
        // reference settles on, which is the one a link to it names, and which is known only when the
        // closing half arrives — so the place is recorded now and named then.
        if (half.Opens)
        {
            Flow flow = CurrentFlow;
            RtfLayoutAnchor anchor = new() { Offset = flow.LayoutLength };
            flow.LayoutAnchors.Add(anchor);
            _openLayoutAnchors[half.Id] = anchor;
        }
        else if (_openLayoutAnchors.Remove(half.Id, out RtfLayoutAnchor? opened))
        {
            opened.Name = half.Name;
        }

        if (half.Opens) _marks.OpenBookmark(key, half.Name, Here());
        else _marks.CloseBookmark(key, Here(), half.Name);
    }

    // --------------------------------------------------------------------- tracked changes

    /// <summary>
    /// Records a revision-table entry, whose text ends at a semicolon.
    /// </summary>
    /// <remarks>
    /// The entries are the table's <em>inner</em> groups, so the outer <c>{\*\revtbl}</c> closes
    /// last with nothing collected and must not add an entry of its own — the indexes are positional
    /// and a spurious one at the end is the sort of thing that only shows up in the last author's
    /// name.
    /// </remarks>
    private void RecordRevisionAuthor(GroupState state)
    {
        if (state.Collected.Length == 0) return;
        _revisionAuthors.Add(state.Collected.ToString().TrimEnd(';').Trim());
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

    /// <summary>
    /// What this field's <c>\fldrslt</c> is to draw instead of the result the producer cached, or
    /// null when it is to draw the cache.
    /// </summary>
    /// <remarks>Only ever set on the second read of a document that states a <c>REF</c> — see
    /// <see cref="ReferenceExpansions"/>.</remarks>
    private string? _referenceExpansion;

    /// <summary>Whether the expansion has already replaced a stretch of the cached result.</summary>
    private bool _referenceDrawn;

    /// <summary>
    /// The text each <c>REF</c> field is to draw, by the bookmark it names, or null to draw every
    /// field's cached result.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer recomputes a <c>REF</c> from its bookmark when the document is loaded, so the string
    /// the producing application cached is not what the reference draws — and the RTF import's own
    /// bookmark naming (<see cref="RtfBookmarkRotation"/>) routinely makes it a different string.
    /// </para>
    /// <para>
    /// Supplied by the caller rather than computed here because a <c>REF</c> may name a bookmark the
    /// walk has not reached: <see cref="RtfReader"/> reads such a document twice, computes the
    /// expansions from the first read's marks, and hands them to the second. A document stating no
    /// <c>REF</c> is read once, which is all but a handful of them.
    /// </para>
    /// </remarks>
    public IReadOnlyDictionary<string, string>? ReferenceExpansions { get; init; }

    /// <summary>Notes where a field's cached result begins, and what is to be drawn in its place.</summary>
    private void BeginFieldResult()
    {
        _fieldResultDepth = _groupDepth;
        _fieldResultOffset = OffsetIn(CurrentFlow);
        _fieldResultLayoutOffset = CurrentFlow.LayoutLength;
        _fieldResultStart = Here();

        _referenceDrawn = false;
        _referenceExpansion = null;
        if (ReferenceExpansions is { } expansions
            && FieldInstructions.ReferenceBookmark(_fieldInstruction) is { } bookmark
            && expansions.TryGetValue(bookmark, out string? expansion))
        {
            _referenceExpansion = expansion;
        }
    }

    /// <summary>
    /// The text to append for a stretch of a field result, which is the expansion for the first
    /// stretch of a substituted one and nothing for the rest of it.
    /// </summary>
    /// <remarks>
    /// The whole cached result is one field portion in Writer, however many runs the file breaks it
    /// into, so the expansion replaces the lot and takes the formatting of the first of them.
    /// </remarks>
    private string SubstitutedFieldText(string text)
    {
        if (_referenceExpansion is not { } expansion) return text;
        if (_referenceDrawn) return string.Empty;

        _referenceDrawn = true;
        return expansion;
    }

    /// <summary>Records the field once its <c>\fldrslt</c> group has closed.</summary>
    /// <remarks>
    /// A <c>PAGE</c> or <c>NUMPAGES</c> result is additionally recorded as a span over the paragraph's
    /// layout text, because those two are the fields whose cached result is wrong on every page but one
    /// — see <see cref="Layout.PageFields"/>. Everything else keeps the cache, which is what a reference
    /// renderer draws.
    /// </remarks>
    private void EndFieldResult(GroupState state)
    {
        // A field whose cached result is empty still draws its expansion, so the substitution cannot
        // wait for text that never comes.
        if (_referenceExpansion is { Length: > 0 } expansion && !_referenceDrawn && !state.Hidden)
        {
            _referenceDrawn = true;
            NoteBodyContent();
            AppendToParagraph(state, expansion);
        }

        if (FieldInstructions.PageFieldOf(_fieldInstruction) is { } page)
        {
            // A negative length is the one case to drop: a `\par` inside the result group flushed the
            // paragraph and reset `LayoutLength` to nought, so the offset taken at `\fldrslt` now
            // indexes a string that no longer exists. `ResetParagraphState` is static and cannot clear
            // the offset itself, so the comparison stands in for it.
            int length = CurrentFlow.LayoutLength - _fieldResultLayoutOffset;
            if (length >= 0)
            {
                CurrentFlow.PendingPageFields.Add(new Layout.PageFieldSpan(
                    _fieldResultLayoutOffset, length, page.Kind, page.Format));
            }
        }

        _marks.AddField(
            _fieldInstruction,
            SliceIn(CurrentFlow, _fieldResultOffset, OffsetIn(CurrentFlow)),
            _fieldResultStart,
            Here());

        _fieldResultDepth = -1;
        _fieldResultStart = null;
        _fieldInstruction = null;
        _referenceExpansion = null;
        _referenceDrawn = false;
    }
}

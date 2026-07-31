namespace Paperless.WordProcessing.Model;

/// <summary>What a tracked change did to the range it covers.</summary>
/// <remarks>
/// The four kinds Writer's own <c>RedlineType</c> distinguishes for text
/// (<c>sw/inc/redline.hxx</c>); the table-row and paragraph-move kinds beside them are about
/// editing rather than about what the text says, and nothing here would read them.
/// </remarks>
public enum WritingChangeKind
{
    /// <summary>Text a tracked change added, which is part of the document.</summary>
    Insertion,

    /// <summary>Text a tracked change removed, which is not.</summary>
    Deletion,

    /// <summary>Character formatting a tracked change altered.</summary>
    Formatting,

    /// <summary>Paragraph formatting a tracked change altered.</summary>
    ParagraphFormatting,
}

/// <summary>
/// One tracked change: what it did, over what range, by whom and when.
/// </summary>
/// <remarks>
/// <para>
/// A <em>record</em> of the change, not an instruction to apply it. Extraction still says what the
/// changes leave — an insertion is content and a deletion is not — and every one of the four readers
/// resolves them that way before this is filled in. What this adds is the part resolution throws
/// away: who, when, and which words.
/// </para>
/// <para>
/// <see cref="Text"/> is the load-bearing member for a deletion. Deleted text is still in the file —
/// that is what makes the change reversible — but it is deliberately absent from the extracted
/// paragraph, so a deletion's <see cref="Range"/> is empty and collapsed onto the position the text
/// was removed from. Without the text beside it the record would say that somebody deleted
/// something, somewhere, and nothing more.
/// </para>
/// </remarks>
/// <param name="Kind">What the change did.</param>
/// <param name="Range">
/// Where it applies. Non-empty for an insertion, whose text is in the document; empty for a
/// deletion, whose text is not.
/// </param>
public sealed record WritingChange(WritingChangeKind Kind, WritingRange Range)
{
    /// <summary>Who made the change, as the file records the name.</summary>
    /// <remarks>
    /// Null when the file names nobody. That is not rare: WW8 and RTF both index an author table, and
    /// the conventional first entry of an RTF <c>{\*\revtbl}</c> is <c>Unknown</c>.
    /// </remarks>
    public string? Author { get; init; }

    /// <summary>
    /// When the change was made, in whatever precision the file states.
    /// </summary>
    /// <remarks>
    /// <see cref="DateTime"/> rather than <see cref="DateTimeOffset"/> because three of the four
    /// formats state no zone at all: a WW8 <c>DTTM</c> and an RTF <c>\revdttm</c> are packed local
    /// fields, and an ODF <c>dc:date</c> is usually a bare date. Attaching an offset would invent one.
    /// Null when the file records no date, which includes the zero <c>DTTM</c> both legacy formats
    /// write when the producer suppressed it.
    /// </remarks>
    public DateTime? Timestamp { get; init; }

    /// <summary>
    /// The text the change concerns: what a deletion removed, or what an insertion added.
    /// </summary>
    /// <remarks>
    /// For a deletion this is the only place the words survive, since extraction drops them. For an
    /// insertion it duplicates text that is also in the paragraph, and is kept anyway so the two
    /// kinds report the same thing.
    /// </remarks>
    public string? Text { get; init; }
}

/// <summary>
/// A bookmark: a name and the range it covers.
/// </summary>
/// <remarks>
/// <para>
/// A range, not a point. All four formats say so — ODF pairs <c>text:bookmark-start</c> with
/// <c>text:bookmark-end</c>, DOCX pairs <c>w:bookmarkStart</c> with <c>w:bookmarkEnd</c> by
/// <c>w:id</c>, RTF pairs <c>{\*\bkmkstart}</c> with <c>{\*\bkmkend}</c> by name, and WW8 pairs its
/// two bookmark PLCFs by an index one holds into the other. Writer models it the same way:
/// <c>IDocumentMarkAccess</c>'s marks are "(start, end-or-same, name)" triples
/// (<c>sw/inc/IDocumentMarkAccess.hxx</c>).
/// </para>
/// <para>
/// A collapsed range is a legitimate bookmark rather than a malformed one — ODF spells it
/// <c>text:bookmark</c>, a single element — and is what a cross-reference target usually is.
/// </para>
/// </remarks>
/// <param name="Name">The name content refers to the bookmark by.</param>
/// <param name="Range">What it covers; empty for a point bookmark.</param>
public sealed record WritingBookmark(string Name, WritingRange Range)
{
    /// <summary>True for a bookmark that marks a position rather than a span of text.</summary>
    public bool IsPoint => Range.IsEmpty;
}

/// <summary>
/// What a field computes, as far as it is worth distinguishing.
/// </summary>
/// <remarks>
/// A common vocabulary over two incompatible ways of saying it: the Word family names a field in an
/// instruction string — <c>PAGE \* ARABIC</c> — while ODF has one element per kind and no instruction
/// at all. Writer has the same problem and solves it the same way, with a <c>SwFieldIds</c> enum
/// (<c>sw/inc/fldbas.hxx</c>) that both its importers map onto.
/// </remarks>
public enum WritingFieldKind
{
    /// <summary>A field this vocabulary does not name. The instruction still says what it is.</summary>
    Unknown,

    /// <summary>The current page's number.</summary>
    PageNumber,

    /// <summary>How many pages the document has.</summary>
    PageCount,

    /// <summary>A date, usually today's.</summary>
    Date,

    /// <summary>A time, usually now.</summary>
    Time,

    /// <summary>When the document was created.</summary>
    CreationDate,

    /// <summary>When the document was last saved.</summary>
    ModificationDate,

    /// <summary>An author's name.</summary>
    Author,

    /// <summary>The document's file name.</summary>
    FileName,

    /// <summary>The document's title.</summary>
    Title,

    /// <summary>The document's subject.</summary>
    Subject,

    /// <summary>The document's keywords.</summary>
    Keywords,

    /// <summary>The document's description or comments.</summary>
    Description,

    /// <summary>The enclosing chapter's heading or number.</summary>
    Chapter,

    /// <summary>A cross-reference to a bookmark, a heading or a numbered item.</summary>
    Reference,

    /// <summary>The page a cross-reference's target is on.</summary>
    PageReference,

    /// <summary>A hyperlink, whose target is in the instruction and nowhere else.</summary>
    Hyperlink,

    /// <summary>A numbered sequence — figure and table captions.</summary>
    Sequence,

    /// <summary>A user variable, set or read.</summary>
    Variable,

    /// <summary>A table of contents or other generated index.</summary>
    TableOfContents,

    /// <summary>How many words the document has.</summary>
    WordCount,
}

/// <summary>
/// A field: what it says to compute, and what the writing application last computed.
/// </summary>
/// <remarks>
/// <para>
/// Both halves, because they answer different questions. The <see cref="Result"/> is what a reader
/// saw and what a reference renderer draws, so it stays preferred by default — a headless renderer
/// cannot recompute <c>PAGE</c> before it has paginated, and must not recompute <c>DATE</c> at all
/// if it is to reproduce the file. The <see cref="Instruction"/> is what the result cannot tell you:
/// a hyperlink's target lives there and nowhere else, and a page number that reads "4" says nothing
/// about whether it was <c>PAGE</c> or a typed digit.
/// </para>
/// <para>
/// <see cref="Instruction"/> is null for ODF, which has no instruction string: a field is a typed
/// element and <see cref="Kind"/> carries the whole of its meaning.
/// </para>
/// </remarks>
/// <param name="Kind">What the field computes.</param>
/// <param name="Range">The result's extent in the document's text.</param>
public sealed record WritingField(WritingFieldKind Kind, WritingRange Range)
{
    /// <summary>The instruction, verbatim, for the three formats that have one.</summary>
    public string? Instruction { get; init; }

    /// <summary>The result the writing application cached, which is what a reader saw.</summary>
    public string? Result { get; init; }
}

/// <summary>
/// What a document records over its text rather than in it: tracked changes, bookmarks and fields.
/// </summary>
/// <remarks>
/// <para>
/// Separate from both the extraction tree and the paragraph hints, because all three are ranges over
/// the document rather than properties of a run — Writer keeps redlines in a <c>SwRedlineTable</c>
/// and marks in an <c>IDocumentMarkAccess</c> for the same reason, "independent of the hints system"
/// (<c>research/02-writer.md</c> section A.5).
/// </para>
/// <para>
/// Every position here is a <see cref="WritingPosition"/> into a paragraph the recording pass
/// materialised. Only the paragraphs a mark actually touches are materialised, so a document with no
/// marks builds none and a document with three builds three — which is what keeps extraction from
/// paying for a model it did not ask for.
/// </para>
/// </remarks>
public sealed class WritingMarks
{
    /// <summary>A document that records nothing.</summary>
    public static readonly WritingMarks Empty = new([], [], []);

    /// <summary>Creates a set of marks.</summary>
    /// <param name="changes">The tracked changes, in document order.</param>
    /// <param name="bookmarks">The bookmarks, in the order their starts were seen.</param>
    /// <param name="fields">The fields, in document order.</param>
    public WritingMarks(
        IReadOnlyList<WritingChange> changes,
        IReadOnlyList<WritingBookmark> bookmarks,
        IReadOnlyList<WritingField> fields)
    {
        Changes = changes;
        Bookmarks = bookmarks;
        Fields = fields;
    }

    /// <summary>The tracked changes the document records.</summary>
    public IReadOnlyList<WritingChange> Changes { get; }

    /// <summary>The bookmarks the document records.</summary>
    public IReadOnlyList<WritingBookmark> Bookmarks { get; }

    /// <summary>The fields the document records, definition and cached result both.</summary>
    public IReadOnlyList<WritingField> Fields { get; }

    /// <summary>True when the document records none of the three.</summary>
    public bool IsEmpty => Changes.Count == 0 && Bookmarks.Count == 0 && Fields.Count == 0;
}

namespace Paperless.WordProcessing.Model;

/// <summary>
/// What a text attribute sets, and how it competes with the others at the same position.
/// </summary>
/// <remarks>
/// The order of these members <em>is</em> the resolution order: a value later in the enumeration
/// wins over an earlier one covering the same character. That is why they are declared here rather
/// than left to the caller — Writer's own comment on the equivalent constants notes that direct
/// formatting must sort after a character style, and a link's formatting after both, or a hyperlink
/// inside a styled run loses its colour.
/// </remarks>
public enum TextHintKind
{
    /// <summary>A named character style applied to a run.</summary>
    CharacterStyle,

    /// <summary>A hyperlink, whose own formatting overrides the character style's.</summary>
    Hyperlink,

    /// <summary>
    /// Direct formatting — the ad-hoc bold or colour a user applied, belonging to no style.
    /// </summary>
    DirectFormatting,

    /// <summary>A field, which occupies one placeholder character.</summary>
    Field,

    /// <summary>A footnote or endnote anchor, which occupies one placeholder character.</summary>
    NoteAnchor,

    /// <summary>A comment anchor, which occupies one placeholder character.</summary>
    CommentAnchor,

    /// <summary>An as-character anchored frame — an inline image — at one placeholder character.</summary>
    InlineFrame,

    /// <summary>An explicit line or page break inside a run, at one placeholder character.</summary>
    Break,

    /// <summary>An insertion recorded by a tracked change.</summary>
    Insertion,

    /// <summary>A deletion recorded by a tracked change.</summary>
    Deletion,
}

/// <summary>
/// One attribute over a range of a paragraph's text.
/// </summary>
/// <param name="Kind">What the attribute sets.</param>
/// <param name="Start">The first character it covers.</param>
/// <param name="End">One past the last character it covers.</param>
/// <param name="Value">
/// The value: a style name, a hyperlink target, a formatting set, an anchor's identity. Untyped
/// because the model stores what the formats say and layout decides what to do with it.
/// </param>
public readonly record struct TextHint(
    TextHintKind Kind,
    int Start,
    int End,
    object? Value)
{
    /// <summary>How many characters the attribute covers.</summary>
    public int Length => End - Start;

    /// <summary>
    /// True when the attribute marks a single position rather than a range.
    /// </summary>
    /// <remarks>
    /// A field, an anchor and an inline frame are all one character wide, and that character is a
    /// placeholder in the paragraph's text rather than anything a reader sees. Keeping it in the text
    /// is what makes every offset in the document — a bookmark, a tracked change, a formatting run —
    /// agree on what a position counts.
    /// </remarks>
    public bool IsAnchor => Kind is TextHintKind.Field
                                 or TextHintKind.NoteAnchor
                                 or TextHintKind.CommentAnchor
                                 or TextHintKind.InlineFrame
                                 or TextHintKind.Break;

    /// <summary>True when the attribute covers a position.</summary>
    public bool Covers(int position) => position >= Start && position < End;

    /// <summary>True when the attribute overlaps a half-open range.</summary>
    public bool Overlaps(int start, int end) => Start < end && start < End;
}

/// <summary>
/// A paragraph's character formatting, stored as intervals over its text rather than as runs of it.
/// </summary>
/// <remarks>
/// Named for LibreOffice's <c>SwpHints</c>, whose design this follows closely enough that the C++ is
/// worth reading beside it (<c>sw/inc/ndhints.hxx</c>).
/// </remarks>
/// <remarks>
/// <para>
/// This is the one part of Writer's model worth copying exactly. Character formatting is
/// <em>overlapping</em> in practice — a bold range and a hyphenated-language range and a hyperlink
/// each start and end wherever the user put them — and storing it as a list of runs forces every
/// overlap to split every run that crosses it. A document where three attributes overlap pairwise
/// becomes seven runs, none of which corresponds to anything the user did, and each edit re-splits
/// them. Intervals store what was actually applied, and runs are computed when something needs them.
/// </para>
/// <para>
/// Attributes are kept sorted by start, then by <see cref="TextHintKind"/>, so that resolving a
/// position is a scan in priority order rather than a sort. That ordering is load-bearing: see
/// <see cref="TextHintKind"/>.
/// </para>
/// </remarks>
public sealed class TextHints
{
    private readonly List<TextHint> _attributes = [];

    /// <summary>The attributes, sorted by start and then by kind.</summary>
    public IReadOnlyList<TextHint> All => _attributes;

    /// <summary>How many attributes the paragraph carries.</summary>
    public int Count => _attributes.Count;

    /// <summary>Adds an attribute, keeping the collection sorted.</summary>
    /// <remarks>
    /// An empty range is rejected rather than stored: it can cover no character, so it could only
    /// ever confuse a later scan. An anchor is one character wide by definition, and is normalised to
    /// that rather than trusted, since a caller computing it from a byte offset can be off by one.
    /// </remarks>
    public void Add(TextHint attribute)
    {
        TextHint normalised = attribute.IsAnchor
            ? attribute with { End = attribute.Start + 1 }
            : attribute;

        if (normalised.Length <= 0 || normalised.Start < 0) return;

        int index = _attributes.BinarySearch(normalised, Ordering.Instance);
        _attributes.Insert(index < 0 ? ~index : index, normalised);
    }

    /// <summary>Adds a ranged attribute.</summary>
    /// <param name="kind">What the attribute sets.</param>
    /// <param name="start">The first character it covers.</param>
    /// <param name="end">One past the last character it covers.</param>
    /// <param name="value">The value the attribute carries.</param>
    public void Add(TextHintKind kind, int start, int end, object? value = null)
        => Add(new TextHint(kind, start, end, value));

    /// <summary>Adds an attribute at a single placeholder position.</summary>
    /// <param name="kind">What the attribute sets.</param>
    /// <param name="position">The placeholder character's position.</param>
    /// <param name="value">The value the attribute carries.</param>
    public void AddAnchor(TextHintKind kind, int position, object? value = null)
        => Add(new TextHint(kind, position, position + 1, value));

    /// <summary>
    /// The attributes covering a position, in resolution order — lowest priority first.
    /// </summary>
    /// <remarks>
    /// In order so a caller can apply them one after another and let the last win, which is the same
    /// shape every one of Paperless's format readers already uses for style resolution.
    /// </remarks>
    public List<TextHint> At(int position)
    {
        List<TextHint> covering = [];
        foreach (TextHint attribute in _attributes)
        {
            // Sorted by start, so nothing beyond the position can begin before it.
            if (attribute.Start > position) break;
            if (attribute.Covers(position)) covering.Add(attribute);
        }

        covering.Sort(static (left, right) => left.Kind.CompareTo(right.Kind));
        return covering;
    }

    /// <summary>The highest-priority attribute of a kind at a position, if any.</summary>
    public TextHint? At(TextHintKind kind, int position)
    {
        TextHint? found = null;
        foreach (TextHint attribute in _attributes)
        {
            if (attribute.Start > position) break;
            if (attribute.Kind == kind && attribute.Covers(position)) found = attribute;
        }
        return found;
    }

    /// <summary>Every attribute overlapping a half-open range.</summary>
    public List<TextHint> Overlapping(int start, int end)
    {
        List<TextHint> found = [];
        foreach (TextHint attribute in _attributes)
        {
            if (attribute.Start >= end) break;
            if (attribute.Overlaps(start, end)) found.Add(attribute);
        }
        return found;
    }

    /// <summary>
    /// The positions where the set of attributes in force changes, ascending.
    /// </summary>
    /// <remarks>
    /// This is how intervals become runs: between two consecutive boundaries the formatting is
    /// uniform by construction, so a consumer that wants runs — a renderer, or the extraction tree —
    /// asks for the boundaries and reads the attributes once per span rather than once per character.
    /// The paragraph's own ends are always boundaries, so a paragraph with no attributes still yields
    /// one span.
    /// </remarks>
    public List<int> Boundaries(int textLength)
    {
        SortedSet<int> boundaries = [0];
        if (textLength > 0) boundaries.Add(textLength);

        foreach (TextHint attribute in _attributes)
        {
            if (attribute.Start > 0 && attribute.Start < textLength) boundaries.Add(attribute.Start);
            if (attribute.End > 0 && attribute.End < textLength) boundaries.Add(attribute.End);
        }

        return [.. boundaries];
    }

    /// <summary>
    /// Shifts and splits the attributes for an insertion of text at a position.
    /// </summary>
    /// <remarks>
    /// An attribute that spans the insertion point grows to include the new text, which is what a
    /// user typing inside a bold run expects. An attribute starting at exactly that point does not:
    /// text typed immediately before a bold run is not bold, and an anchor must not stretch at all
    /// because its width is the placeholder character it stands for.
    /// </remarks>
    public void Insert(int position, int length)
    {
        if (length <= 0) return;

        for (int i = 0; i < _attributes.Count; i++)
        {
            TextHint attribute = _attributes[i];

            if (attribute.Start >= position)
            {
                _attributes[i] = attribute with
                {
                    Start = attribute.Start + length,
                    End = attribute.End + length,
                };
            }
            else if (attribute.End > position && !attribute.IsAnchor)
            {
                _attributes[i] = attribute with { End = attribute.End + length };
            }
        }
    }

    /// <summary>
    /// Shifts and trims the attributes for a deletion of a range, dropping those left empty.
    /// </summary>
    public void Delete(int start, int length)
    {
        if (length <= 0) return;

        int end = start + length;
        for (int i = _attributes.Count - 1; i >= 0; i--)
        {
            TextHint attribute = _attributes[i];

            int newStart = attribute.Start >= end ? attribute.Start - length
                : attribute.Start > start ? start
                : attribute.Start;

            int newEnd = attribute.End >= end ? attribute.End - length
                : attribute.End > start ? start
                : attribute.End;

            if (newEnd <= newStart) _attributes.RemoveAt(i);
            else _attributes[i] = attribute with { Start = newStart, End = newEnd };
        }
    }

    /// <summary>Removes every attribute.</summary>
    public void Clear() => _attributes.Clear();

    /// <summary>Sorts by start and then by kind, which is the resolution order.</summary>
    private sealed class Ordering : IComparer<TextHint>
    {
        public static Ordering Instance { get; } = new();

        public int Compare(TextHint left, TextHint right)
        {
            int byStart = left.Start.CompareTo(right.Start);
            if (byStart != 0) return byStart;

            int byKind = left.Kind.CompareTo(right.Kind);
            return byKind != 0 ? byKind : left.End.CompareTo(right.End);
        }
    }
}

namespace Paperless.WordProcessing.Model;

/// <summary>
/// A position in the document: a paragraph and an offset into its text.
/// </summary>
/// <remarks>
/// A node reference plus an offset, never an offset into a whole-document string. Paragraphs get
/// split, merged and inserted, and every format's own offsets are relative to something — so an
/// absolute character offset would be invalidated by the next edit and could not be compared across
/// two documents at all. Writer models it the same way, for the same reason
/// (<c>research/02-writer.md</c> section A.2).
/// </remarks>
/// <param name="Paragraph">The paragraph the position is in.</param>
/// <param name="Offset">How many characters into that paragraph's text, placeholders included.</param>
public readonly record struct WritingPosition(WritingParagraph Paragraph, int Offset)
    : IComparable<WritingPosition>
{
    /// <summary>
    /// Orders two positions by document order, then by offset.
    /// </summary>
    /// <remarks>
    /// Two positions in different paragraphs compare by their paragraphs' document order, which is
    /// only meaningful once <see cref="WritingDocument.AssignDocumentOrder"/> has run — before that
    /// every paragraph's order is -1 and positions in different paragraphs compare equal.
    /// </remarks>
    public int CompareTo(WritingPosition other)
    {
        int byNode = Paragraph.DocumentOrder.CompareTo(other.Paragraph.DocumentOrder);
        return byNode != 0 ? byNode : Offset.CompareTo(other.Offset);
    }

    /// <summary>True when this position precedes another.</summary>
    public static bool operator <(WritingPosition left, WritingPosition right)
        => left.CompareTo(right) < 0;

    /// <summary>True when this position follows another.</summary>
    public static bool operator >(WritingPosition left, WritingPosition right)
        => left.CompareTo(right) > 0;

    /// <summary>True when this position precedes another or equals it.</summary>
    public static bool operator <=(WritingPosition left, WritingPosition right)
        => left.CompareTo(right) <= 0;

    /// <summary>True when this position follows another or equals it.</summary>
    public static bool operator >=(WritingPosition left, WritingPosition right)
        => left.CompareTo(right) >= 0;
}

/// <summary>
/// A range between two positions, which is what nearly every operation on a document takes.
/// </summary>
/// <remarks>
/// The equivalent of Writer's <c>SwPaM</c>. Normalised on construction so that
/// <see cref="Start"/> never follows <see cref="End"/>: a selection made backwards is the same range
/// as one made forwards, and letting the two differ means every consumer has to normalise it again.
/// </remarks>
public readonly record struct WritingRange
{
    /// <summary>Creates a range between two positions, in either order.</summary>
    public WritingRange(WritingPosition first, WritingPosition second)
    {
        bool forwards = first <= second;
        Start = forwards ? first : second;
        End = forwards ? second : first;
    }

    /// <summary>The earlier position.</summary>
    public WritingPosition Start { get; }

    /// <summary>The later position.</summary>
    public WritingPosition End { get; }

    /// <summary>True when the range covers nothing.</summary>
    public bool IsEmpty => Start == End;

    /// <summary>True when a position lies in the range, its end excluded.</summary>
    public bool Contains(WritingPosition position) => position >= Start && position < End;

    /// <summary>True when this range and another share any position.</summary>
    public bool Overlaps(WritingRange other) => Start < other.End && other.Start < End;
}

/// <summary>
/// A style: a name, what it inherits from, and the formatting it sets.
/// </summary>
/// <param name="Name">The style's name, which is what content refers to it by.</param>
/// <param name="Family">What the style applies to.</param>
/// <param name="ParentName">The style it inherits from, or null for a root style.</param>
/// <param name="Properties">
/// The formatting it sets. Untyped because each format's property model differs and the model stores
/// what the file said; resolving it into measurements is layout's job.
/// </param>
public sealed record WritingStyle(
    string Name,
    WritingStyleFamily Family,
    string? ParentName,
    object? Properties)
{
    /// <summary>The style this one links to for its other half, where a format pairs them.</summary>
    /// <remarks>
    /// DOCX pairs a paragraph style with a character style through <c>w:link</c>, and ODF pairs a
    /// list style with a paragraph style. The link is not inheritance and must not be followed as
    /// though it were.
    /// </remarks>
    public string? LinkedStyleName { get; init; }
}

/// <summary>What a style applies to.</summary>
public enum WritingStyleFamily
{
    /// <summary>A paragraph style.</summary>
    Paragraph,

    /// <summary>A character style.</summary>
    Character,

    /// <summary>A frame style.</summary>
    Frame,

    /// <summary>A page style.</summary>
    Page,

    /// <summary>A list style.</summary>
    List,

    /// <summary>A table style.</summary>
    Table,
}

/// <summary>
/// The document's styles, resolvable through their parent chains.
/// </summary>
/// <remarks>
/// One pool per family, because a paragraph style and a character style may share a name — ODF
/// documents routinely have both called <c>Standard</c> — and resolving across families would take
/// one style's parent from the other family's chain.
/// </remarks>
public sealed class WritingStyles
{
    /// <summary>How deep a parent chain is followed before it is treated as circular.</summary>
    public const int MaxChainDepth = 32;

    private readonly Dictionary<(WritingStyleFamily Family, string Name), WritingStyle> _styles =
        new(FamilyAndNameComparer.Instance);

    private readonly Dictionary<WritingStyleFamily, string> _defaults = [];

    /// <summary>Every style, in no particular order.</summary>
    public IEnumerable<WritingStyle> All => _styles.Values;

    /// <summary>Adds or replaces a style.</summary>
    public void Add(WritingStyle style)
    {
        ArgumentNullException.ThrowIfNull(style);
        _styles[(style.Family, style.Name)] = style;
    }

    /// <summary>Names the style a family falls back to when content names none.</summary>
    public void SetDefault(WritingStyleFamily family, string name)
    {
        ArgumentException.ThrowIfNullOrEmpty(name);
        _defaults[family] = name;
    }

    /// <summary>The style with a name in a family, or null when there is none.</summary>
    public WritingStyle? Find(WritingStyleFamily family, string? name)
        => name is not null && _styles.TryGetValue((family, name), out WritingStyle? style)
            ? style
            : null;

    /// <summary>The family's default style, or null when none was named.</summary>
    public WritingStyle? Default(WritingStyleFamily family)
        => _defaults.TryGetValue(family, out string? name) ? Find(family, name) : null;

    /// <summary>
    /// A style's ancestors and itself, outermost first.
    /// </summary>
    /// <remarks>
    /// Outermost first so a caller can apply them in order and let the nearest win — the same shape
    /// every one of Paperless's readers already uses, over four different encodings of the same idea.
    /// The family's default style comes first of all when the chain does not reach it, since that is
    /// what a style with no parent still inherits from. Cycle-guarded: a parent loop is malformed but
    /// does occur, and this walks a chain built from untrusted input.
    /// </remarks>
    public List<WritingStyle> Chain(WritingStyleFamily family, string? name)
    {
        List<WritingStyle> chain = [];
        HashSet<string> visited = new(StringComparer.Ordinal);

        string? current = name;
        for (int depth = 0; depth < MaxChainDepth; depth++)
        {
            if (current is null || !visited.Add(current)) break;
            if (Find(family, current) is not { } style) break;

            chain.Add(style);
            current = style.ParentName;
        }

        chain.Reverse();

        if (Default(family) is { } fallback
            && !chain.Any(s => string.Equals(s.Name, fallback.Name, StringComparison.Ordinal)))
        {
            chain.Insert(0, fallback);
        }

        return chain;
    }

    private sealed class FamilyAndNameComparer
        : IEqualityComparer<(WritingStyleFamily Family, string Name)>
    {
        public static FamilyAndNameComparer Instance { get; } = new();

        public bool Equals(
            (WritingStyleFamily Family, string Name) left,
            (WritingStyleFamily Family, string Name) right)
            => left.Family == right.Family
               && string.Equals(left.Name, right.Name, StringComparison.Ordinal);

        public int GetHashCode((WritingStyleFamily Family, string Name) value)
            => HashCode.Combine(value.Family, StringComparer.Ordinal.GetHashCode(value.Name));
    }
}

/// <summary>
/// The layout-facing document: the body, the furniture around it, and the styles they refer to.
/// </summary>
/// <remarks>
/// <para>
/// Distinct from the extraction tree in <c>Paperless.Core</c>, deliberately. Extraction is the common
/// case and must not pay for what layout needs — interval-tagged formatting, document-order indexes,
/// resolvable style chains — so the two are separate models built by separate passes, and a caller
/// that only wants text never constructs this one.
/// </para>
/// <para>
/// All four importers converge here, which is what keeps layout written once. That is the same
/// arrangement LibreOffice uses, where the DOCX, DOC and RTF importers all build one <c>SwDoc</c>.
/// </para>
/// </remarks>
public sealed class WritingDocument
{
    private readonly List<WritingBody> _flows = [];

    /// <summary>The document's main text.</summary>
    public WritingBody Body { get; } = new(WritingBodyKind.Body);

    /// <summary>
    /// The flows beside the body: headers, footers, notes, comments and frames.
    /// </summary>
    /// <remarks>
    /// Kept beside the body rather than inside it because that is where they are in every format: a
    /// footnote's text is not at the point that cites it, and a header belongs to a page rather than
    /// to a position in the text. The anchor in the body is an attribute over a placeholder character
    /// that names the flow.
    /// </remarks>
    public IReadOnlyList<WritingBody> Flows => _flows;

    /// <summary>The document's styles.</summary>
    public WritingStyles Styles { get; } = new();

    /// <summary>The document's properties.</summary>
    public Core.Documents.DocumentMetadata Metadata { get; set; } =
        Core.Documents.DocumentMetadata.Empty;

    /// <summary>Adds a flow beside the body.</summary>
    public WritingBody AddFlow(WritingBody flow)
    {
        ArgumentNullException.ThrowIfNull(flow);
        _flows.Add(flow);
        return flow;
    }

    /// <summary>
    /// Numbers every node in document order, so positions and ranges can be compared.
    /// </summary>
    /// <remarks>
    /// Once, after the tree is built, rather than as nodes are appended: an importer adds thousands
    /// and would pay for a renumbering at each one, while nothing reads the order until the document
    /// is complete. Each node's own index and its end index bracket its whole subtree, which is what
    /// makes containment two integer comparisons.
    /// </remarks>
    public int AssignDocumentOrder()
    {
        int next = 0;
        next = Number(Body, next);
        foreach (WritingBody flow in _flows) next = Number(flow, next);
        return next;

        static int Number(WritingNode node, int next)
        {
            node.DocumentOrder = next;
            next++;

            switch (node)
            {
                case WritingBody body:
                    foreach (WritingNode child in body.Children) next = Number(child, next);
                    break;

                case WritingTable table:
                    foreach (WritingTableRow row in table.Rows) next = Number(row, next);
                    break;

                case WritingTableRow row:
                    foreach (WritingTableCell cell in row.Cells) next = Number(cell, next);
                    break;

                case WritingTableCell cell:
                    next = Number(cell.Content, next);
                    break;
            }

            node.DocumentOrderEnd = next;
            return next;
        }
    }

    /// <summary>Every node in the document, in document order.</summary>
    public IEnumerable<WritingNode> InDocumentOrder()
    {
        foreach (WritingNode node in Walk(Body)) yield return node;
        foreach (WritingBody flow in _flows)
        {
            foreach (WritingNode node in Walk(flow)) yield return node;
        }

        static IEnumerable<WritingNode> Walk(WritingNode node)
        {
            yield return node;

            IEnumerable<WritingNode> children = node switch
            {
                WritingBody body => body.Children,
                WritingTable table => table.Rows,
                WritingTableRow row => row.Cells,
                WritingTableCell cell => [cell.Content],
                _ => [],
            };

            foreach (WritingNode child in children)
            {
                foreach (WritingNode inner in Walk(child)) yield return inner;
            }
        }
    }
}

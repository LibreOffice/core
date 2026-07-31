using Paperless.MsBinary.Records;

namespace Paperless.MsBinary.Escher;

/// <summary>
/// A rectangle as the drawing layer states it: four bounds in whatever unit the containing
/// record uses, which differs between hosts.
/// </summary>
/// <remarks>
/// Deliberately not converted to a <c>Length</c>. A child anchor is in the parent group's own
/// coordinate space, a PowerPoint client anchor is in master units of 1/576 inch, and Word's
/// is in twips; only the host knows which, so the shared reader reports the numbers the file
/// states and leaves the conversion to the caller that knows the unit.
/// </remarks>
/// <param name="Left">The left edge.</param>
/// <param name="Top">The top edge.</param>
/// <param name="Right">The right edge.</param>
/// <param name="Bottom">The bottom edge.</param>
public readonly record struct EscherRectangle(int Left, int Top, int Right, int Bottom)
{
    /// <summary>The width, which a flipped shape may state as negative.</summary>
    public int Width => Right - Left;

    /// <summary>The height, which a flipped shape may state as negative.</summary>
    public int Height => Bottom - Top;
}

/// <summary>
/// One shape from an Escher drawing: a leaf shape, or a group holding others.
/// </summary>
/// <remarks>
/// <para>
/// The host-specific records — <c>ClientAnchor</c>, <c>ClientData</c> and
/// <c>ClientTextbox</c> — are reported as record headers rather than parsed. Their payloads
/// are, by the format's own definition, whatever the host application chose: PowerPoint puts
/// its text records in the client textbox, Word an <c>FSPA</c> index in the client anchor, and
/// Excel a cell reference. Decoding them here would make the shared reader one host's.
/// </para>
/// </remarks>
public sealed class EscherShape
{
    /// <summary>The shape's document-wide identifier, from the <c>msofbtSp</c> record.</summary>
    public uint ShapeId { get; init; }

    /// <summary>
    /// The shape type, which the <c>msofbtSp</c> record carries in its header's instance
    /// rather than in its payload.
    /// </summary>
    public ushort ShapeType { get; init; }

    /// <summary>The shape's flags word.</summary>
    public EscherShapeAttributes Flags { get; init; }

    /// <summary>The shape's property table; never null, empty when it states none.</summary>
    public EscherPropertyTable Properties { get; init; } = EscherPropertyTable.Empty;

    /// <summary>
    /// The properties a shape's master supplies, from the secondary and tertiary tables.
    /// </summary>
    public EscherPropertyTable MasterProperties { get; init; } = EscherPropertyTable.Empty;

    /// <summary>The shape's bounds inside its parent group, when it is a group member.</summary>
    public EscherRectangle? ChildAnchor { get; init; }

    /// <summary>
    /// The coordinate space a group's children are expressed in, from <c>msofbtSpgr</c>.
    /// Present only on a group shape.
    /// </summary>
    public EscherRectangle? GroupBounds { get; init; }

    /// <summary>The host-defined anchor record, unparsed.</summary>
    public DffRecordHeader? ClientAnchor { get; init; }

    /// <summary>The host-defined data record, unparsed.</summary>
    public DffRecordHeader? ClientData { get; init; }

    /// <summary>The host-defined text record, unparsed.</summary>
    public DffRecordHeader? ClientTextbox { get; init; }

    /// <summary>The shapes inside this group, in document order. Empty for a leaf shape.</summary>
    public IReadOnlyList<EscherShape> Children { get; init; } = [];

    /// <summary>Whether this shape is a group holding <see cref="Children"/>.</summary>
    public bool IsGroup => (Flags & EscherShapeAttributes.Group) != 0;

    /// <summary>
    /// Whether this shape is the page's background fill rather than content on the page.
    /// </summary>
    public bool IsBackground => (Flags & EscherShapeAttributes.Background) != 0;

    /// <summary>Whether the shape was deleted but its record left in place.</summary>
    public bool IsDeleted => (Flags & EscherShapeAttributes.Deleted) != 0;

    /// <summary>The shape's name, when it was explicitly set.</summary>
    public string? Name => Properties.Text(EscherPropertyIds.ShapeName);

    /// <summary>This shape and every shape beneath it, in document order.</summary>
    public IEnumerable<EscherShape> SelfAndDescendants()
    {
        yield return this;
        foreach (EscherShape child in Children)
        {
            foreach (EscherShape descendant in child.SelfAndDescendants()) yield return descendant;
        }
    }
}

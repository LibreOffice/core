namespace Paperless.MsBinary.Escher;

/// <summary>
/// Escher (MS-ODRAW) is the shared drawing-object format embedded inside DOC, XLS and
/// PPT. Every shape, picture, text box and grouping in a legacy binary file is an
/// Escher record.
/// </summary>
/// <remarks>
/// Because all three legacy formats delegate their drawings to it, implementing Escher
/// once buys shape support across all of them — the same leverage LibreOffice gets from
/// <c>filter/source/msfilter/msdffimp.cxx</c>. Shape properties arrive as a sparse
/// property table (the <c>DFF_Prop_*</c> identifiers) rather than as fixed fields, so
/// the decoder is driven by a property-id lookup. See
/// <c>dotnet/research/04-impress.md</c> section C.
/// </remarks>
public static class EscherRecordTypes
{
    /// <summary>Container for a drawing group; the root of a file's drawing data.</summary>
    public const ushort DrawingGroupContainer = 0xF000;

    /// <summary>The blip (picture) store container.</summary>
    public const ushort BlipStoreContainer = 0xF001;

    /// <summary>Container for one page or sheet's drawing.</summary>
    public const ushort DrawingContainer = 0xF002;

    /// <summary>Container for a shape group.</summary>
    public const ushort ShapeGroupContainer = 0xF003;

    /// <summary>Container for a single shape.</summary>
    public const ushort ShapeContainer = 0xF004;

    /// <summary>Container for the connector, alignment and callout rules of one drawing.</summary>
    public const ushort SolverContainer = 0xF005;

    /// <summary>Document-wide drawing state: the shape-id clusters and the blip count.</summary>
    public const ushort DrawingGroup = 0xF006;

    /// <summary>One entry in the blip store.</summary>
    public const ushort BlipStoreEntry = 0xF007;

    /// <summary>Per-drawing state: the shape count and the id of the last shape.</summary>
    public const ushort Drawing = 0xF008;

    /// <summary>The group shape's own bounds, present only on a group's first shape.</summary>
    public const ushort ShapeGroup = 0xF009;

    /// <summary>The shape's type and identifier.</summary>
    public const ushort Shape = 0xF00A;

    /// <summary>The sparse property table carrying most of a shape's formatting.</summary>
    public const ushort ShapeProperties = 0xF00B;

    /// <summary>Host-defined text for the shape, in the host application's own format.</summary>
    public const ushort ClientTextbox = 0xF00D;

    /// <summary>The shape's bounding box, for child shapes of a group.</summary>
    public const ushort ChildAnchor = 0xF00F;

    /// <summary>The shape's bounding box, for top-level shapes.</summary>
    public const ushort ClientAnchor = 0xF010;

    /// <summary>Host-application data attached to a shape.</summary>
    public const ushort ClientData = 0xF011;

    /// <summary>
    /// A second property table, holding the properties a shape's master supplies rather than
    /// the shape itself.
    /// </summary>
    public const ushort SecondaryShapeProperties = 0xF121;

    /// <summary>A third property table, used by newer writers for the same purpose.</summary>
    public const ushort TertiaryShapeProperties = 0xF122;
}

/// <summary>
/// The attribute bits an <c>msofbtSp</c> record carries in its flags word.
/// </summary>
/// <remarks>
/// Mirrors <c>ShapeFlag</c> in <c>include/filter/msfilter/escherex.hxx:83</c>. The two that
/// matter most to a reader are <see cref="Group"/>, which marks the shape a
/// <c>SpgrContainer</c>'s bounds belong to rather than a real shape, and
/// <see cref="Background"/>, which marks a page's background fill — a shape in the record
/// tree that is not a shape on the page.
/// </remarks>
[Flags]
public enum EscherShapeAttributes : uint
{
    /// <summary>No flags.</summary>
    None = 0x000,

    /// <summary>The shape is a group.</summary>
    Group = 0x001,

    /// <summary>The shape is a member of a group.</summary>
    Child = 0x002,

    /// <summary>The drawing's single topmost group shape.</summary>
    Patriarch = 0x004,

    /// <summary>The shape has been deleted but its record is still present.</summary>
    Deleted = 0x008,

    /// <summary>The shape is an OLE object.</summary>
    OleShape = 0x010,

    /// <summary>The shape inherits from the master shape named by <c>hspMaster</c>.</summary>
    HaveMaster = 0x020,

    /// <summary>The shape is flipped horizontally.</summary>
    FlipHorizontal = 0x040,

    /// <summary>The shape is flipped vertically.</summary>
    FlipVertical = 0x080,

    /// <summary>The shape is a connector.</summary>
    Connector = 0x100,

    /// <summary>The shape carries an anchor record.</summary>
    HaveAnchor = 0x200,

    /// <summary>The shape is the page background rather than page content.</summary>
    Background = 0x400,

    /// <summary>The shape carries a shape-type property.</summary>
    HaveShapeProperty = 0x800,
}

/// <summary>
/// The <c>DFF_Prop_*</c> identifiers of the shape properties a reader actually consults.
/// </summary>
/// <remarks>
/// The full table in <c>include/svx/msdffdef.hxx:151</c> runs to some 150 entries; this holds
/// the ones extraction and the drawing IR need, and the numbering is the file format's, so
/// adding one is a matter of copying its number across.
/// </remarks>
public static class EscherPropertyIds
{
    /// <summary>Rotation, as a 16.16 fixed-point number of degrees.</summary>
    public const ushort Rotation = 4;

    /// <summary>The host's identifier for the shape's text.</summary>
    public const ushort TextId = 128;

    /// <summary>The blip to display, as an index into the blip store.</summary>
    public const ushort Picture = 260;

    /// <summary>The picture's original file name.</summary>
    public const ushort PictureName = 261;

    /// <summary>The shape's foreground fill colour.</summary>
    public const ushort FillColour = 385;

    /// <summary>Whether the shape is filled at all.</summary>
    public const ushort Filled = 447;

    /// <summary>The line colour.</summary>
    public const ushort LineColour = 448;

    /// <summary>Whether the shape has an outline at all.</summary>
    public const ushort Lined = 511;

    /// <summary>The master shape this one inherits from.</summary>
    public const ushort MasterShape = 769;

    /// <summary>The shape's name, when one was set explicitly.</summary>
    public const ushort ShapeName = 896;

    /// <summary>The shape's alternative text.</summary>
    public const ushort AlternativeText = 897;

    /// <summary>Whether the shape is hidden.</summary>
    public const ushort Hidden = 958;
}

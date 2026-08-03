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

    /// <summary>The inset between the shape's left edge and its own text, in EMUs.</summary>
    public const ushort TextInsetLeft = 129;

    /// <summary>The inset between the shape's top edge and its own text, in EMUs.</summary>
    public const ushort TextInsetTop = 130;

    /// <summary>The inset between the shape's right edge and its own text, in EMUs.</summary>
    public const ushort TextInsetRight = 131;

    /// <summary>The inset between the shape's bottom edge and its own text, in EMUs.</summary>
    public const ushort TextInsetBottom = 132;

    /// <summary>
    /// Whether and how the shape's own text wraps at its margins, an <c>MSO_WRAPMODE</c>.
    /// </summary>
    /// <remarks>
    /// Zero — wrap at the shape's own rectangle — is the default, and 2 means "do not wrap", in
    /// which case a line runs on past the shape rather than breaking. Not to be confused with the
    /// wrap <em>distances</em> at 900-903, which are about text <em>outside</em> the shape.
    /// </remarks>
    public const ushort WrapText = 133;

    /// <summary>Where the shape's text sits inside it, an <c>MSO_ANCHOR</c>.</summary>
    /// <remarks>
    /// Ten values (<c>include/svx/msdffdef.hxx:522</c>), of which the "centered" and "baseline"
    /// halves differ from the plain three only in how the block is justified across the shape.
    /// </remarks>
    public const ushort TextAnchor = 135;

    /// <summary>The blip to display, as an index into the blip store.</summary>
    public const ushort Picture = 260;

    /// <summary>The picture's original file name.</summary>
    public const ushort PictureName = 261;

    /// <summary>The shape's foreground fill colour.</summary>
    public const ushort FillColour = 385;

    /// <summary>
    /// Whether the shape is filled at all — a <em>boolean</em> property, so read it with
    /// <see cref="EscherPropertyTable.Boolean"/> rather than <see cref="EscherPropertyTable.Value"/>.
    /// </summary>
    /// <remarks>
    /// 443 rather than the group identifier 447 the entry is actually written under. Naming the
    /// group here instead would make <c>Value(Filled)</c> compile and return the whole group's
    /// thirty-two bits, which is non-zero for a shape that states any fill property at all.
    /// </remarks>
    public const ushort Filled = 443;

    /// <summary>The line colour.</summary>
    public const ushort LineColour = 448;

    /// <summary>The line's thickness in EMUs; 9525, a point in EMUs, when it states none.</summary>
    public const ushort LineWidth = 459;

    /// <inheritdoc cref="Filled"/>
    /// <summary>Whether the shape has an outline at all.</summary>
    public const ushort Lined = 508;

    /// <summary>The master shape this one inherits from.</summary>
    public const ushort MasterShape = 769;

    /// <summary>The shape's name, when one was set explicitly.</summary>
    public const ushort ShapeName = 896;

    /// <summary>The shape's alternative text.</summary>
    public const ushort AlternativeText = 897;

    /// <summary>How far text must stay clear of the shape on its left, in EMUs.</summary>
    /// <remarks>
    /// Word's, and the four are the only place a floating shape's wrap spacing is recorded — the
    /// <c>FSPA</c> carries the rectangle and the wrap mode but no distances. A shape stating none
    /// does not get zero; see the host's reader for the default it takes.
    /// </remarks>
    public const ushort WrapDistanceLeft = 900;

    /// <inheritdoc cref="WrapDistanceLeft"/>
    public const ushort WrapDistanceTop = 901;

    /// <inheritdoc cref="WrapDistanceLeft"/>
    public const ushort WrapDistanceRight = 902;

    /// <inheritdoc cref="WrapDistanceLeft"/>
    public const ushort WrapDistanceBottom = 903;

    /// <summary>How the shape sits inside its horizontal origin: an offset, or an edge to align to.</summary>
    /// <remarks>
    /// One of the four properties Word writes into the <em>tertiary</em> table
    /// (<see cref="EscherRecordTypes.TertiaryShapeProperties"/>) rather than the shape's own, which
    /// is why that table is reported separately. <c>msdffimp.cxx:5216</c> reads all four as raw
    /// six-byte entries under the identifiers 0x038F to 0x0392.
    /// </remarks>
    public const ushort HorizontalPosition = 911;

    /// <summary>What the shape's horizontal position is measured from.</summary>
    /// <inheritdoc cref="HorizontalPosition"/>
    public const ushort HorizontalRelation = 912;

    /// <summary>How the shape sits inside its vertical origin.</summary>
    /// <inheritdoc cref="HorizontalPosition"/>
    public const ushort VerticalPosition = 913;

    /// <summary>What the shape's vertical position is measured from.</summary>
    /// <inheritdoc cref="HorizontalPosition"/>
    public const ushort VerticalRelation = 914;

    /// <summary>The kind of shadow, an <c>MSO_SHADOWTYPE</c>; 0 is a plain offset.</summary>
    public const ushort ShadowType = 512;

    /// <summary>The shadow's colour.</summary>
    /// <remarks>
    /// A shape stating no shadow colour does not get black. The default is <c>0x00808080</c>,
    /// applied unconditionally by <c>msdffimp.cxx:2646-2649</c> whether or not the shape casts a
    /// shadow at all — which is why a file's own flat-ODF export carries a shadow colour on
    /// every shape and a visible shadow on very few.
    /// </remarks>
    public const ushort ShadowColour = 513;

    /// <summary>How opaque the shadow is, as 16.16 fixed point; <c>0x10000</c> is fully opaque.</summary>
    public const ushort ShadowOpacity = 516;

    /// <summary>How far right the shadow sits, in EMUs.</summary>
    public const ushort ShadowOffsetX = 517;

    /// <summary>How far down the shadow sits, in EMUs.</summary>
    public const ushort ShadowOffsetY = 518;

    /// <summary>
    /// Whether the shape casts a shadow at all. A boolean property; see <see cref="Filled"/>.
    /// </summary>
    /// <remarks>
    /// 574 rather than the group identifier 575 the entry is written under, and the distinction
    /// decides the whole feature: <c>fShadow</c> is bit 1 of the group and <c>fshadowObscured</c>
    /// is bit 0. LibreOffice spells the same test as
    /// <c>IsProperty(DFF_Prop_fshadowObscured) &amp;&amp; (GetPropertyValue(...) &amp; 2)</c>
    /// (<c>msdffimp.cxx:2665-2668</c>), which is the group being present and its second bit set.
    /// </remarks>
    public const ushort Shadowed = 574;

    /// <summary>Whether the shape is hidden. A boolean property; see <see cref="Filled"/>.</summary>
    public const ushort Hidden = 958;
}

/// <summary>
/// The built-in shape types an <c>msofbtSp</c> record's instance names, for the handful a reader
/// has to tell apart before it can draw anything.
/// </summary>
/// <remarks>
/// <c>MSO_SPT</c> in <c>include/svx/msdffdef.hxx:276</c> runs to some two hundred entries, nearly
/// all of them preset geometries that belong with the preset-geometry evaluator rather than here.
/// These three are the ones that change what a shape <em>is</em>: a picture frame holds a raster, a
/// text box holds a story, and everything else is drawn.
/// </remarks>
public static class EscherShapeTypes
{
    /// <summary>A plain rectangle, which is also what a shape with no type at all is drawn as.</summary>
    public const ushort Rectangle = 1;

    /// <summary>
    /// A straight line across the shape's own rectangle, from one corner to the opposite one.
    /// </summary>
    /// <remarks>
    /// The one preset whose outline is not its rectangle, which is why it is here rather than with the
    /// preset geometries: <c>SvxMSDffManager::ImportShape</c>
    /// (<c>filter/source/msfilter/msdffimp.cxx</c>:4403) gives it a two-point <c>SdrPathObj</c> from the
    /// bounding rectangle's top-left to its bottom-right instead of a shape with an area. A reader that
    /// draws it as its rectangle paints an opaque box over whatever the line was drawn across.
    /// </remarks>
    public const ushort Line = 20;

    /// <summary>
    /// A straight connector, which is the same diagonal as <see cref="Line"/> by another name.
    /// </summary>
    /// <remarks>
    /// Word writes this type for a line drawn with the connector tool, and the two are indistinguishable
    /// on the page: <c>ImportShape</c> builds an <c>SdrEdgeObj</c> whose track runs from the bounding
    /// rectangle's top-left to its bottom-right (<c>filter/source/msfilter/msdffimp.cxx</c>:4820), which
    /// is the path the line branch builds. The <em>bent</em> and <em>curved</em> connectors that follow it
    /// in <c>MSO_SPT</c> are not straight and are not this.
    /// </remarks>
    public const ushort StraightConnector = 32;

    /// <summary>A frame whose content is the blip its <c>pib</c> property names.</summary>
    public const ushort PictureFrame = 75;

    /// <summary>A box whose content is text the host stores elsewhere.</summary>
    public const ushort TextBox = 202;
}

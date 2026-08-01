namespace Paperless.Vector.Metafiles;

/// <summary>
/// How logical coordinates in a metafile become physical ones.
/// </summary>
/// <remarks>
/// <para>
/// The values are the GDI <c>MM_*</c> constants and are shared by WMF and EMF
/// (<c>emfio/inc/mtftools.hxx:101-111</c>). Six of the eight are fixed physical scales; only
/// <see cref="Isotropic"/> and <see cref="Anisotropic"/> take their scale from the window and
/// viewport extents, which is why those two are the ones that go wrong.
/// </para>
/// <para>
/// <b>This enum is the single commonest source of a wrongly-scaled or mirrored picture.</b>
/// Five of the modes flip the y axis — GDI's metric modes measure upwards — so a decoder that
/// ignores the mode does not merely draw at the wrong size, it draws upside down.
/// </para>
/// </remarks>
public enum MappingMode
{
    /// <summary>One logical unit is one device pixel, y downwards.</summary>
    Text = 0x01,

    /// <summary>One logical unit is 0.1 mm, y upwards.</summary>
    LoMetric = 0x02,

    /// <summary>One logical unit is 0.01 mm, y upwards.</summary>
    HiMetric = 0x03,

    /// <summary>One logical unit is 0.01 in, y upwards.</summary>
    LoEnglish = 0x04,

    /// <summary>One logical unit is 0.001 in, y upwards.</summary>
    HiEnglish = 0x05,

    /// <summary>One logical unit is a twip, y upwards.</summary>
    Twips = 0x06,

    /// <summary>Arbitrary scale from the window and viewport extents, axes equally scaled.</summary>
    Isotropic = 0x07,

    /// <summary>Arbitrary scale from the window and viewport extents, axes independent.</summary>
    Anisotropic = 0x08,
}

/// <summary>What happens to the gaps in hatched fills, dashed lines and text cells.</summary>
public enum BackgroundMode
{
    /// <summary>Not stated by the file.</summary>
    None = 0,

    /// <summary>Gaps are left alone.</summary>
    Transparent = 1,

    /// <summary>Gaps are filled with the background colour.</summary>
    Opaque = 2,
}

/// <summary>Which parts of a self-intersecting polygon are inside it.</summary>
public enum PolygonFillMode
{
    /// <summary>The even-odd rule. GDI's default.</summary>
    Alternate = 1,

    /// <summary>The non-zero winding rule.</summary>
    Winding = 2,
}

/// <summary>
/// How a new clipping region combines with the current one.
/// </summary>
/// <remarks>
/// Only <see cref="And"/> and <see cref="Copy"/> can be honoured: <c>IDrawingSink.ClipPath</c>
/// intersects and nothing else, so <see cref="Diff"/>, <see cref="Or"/> and <see cref="Xor"/>
/// need real path arithmetic. See <c>src/Paperless.Vector/TODO.md</c>.
/// </remarks>
public enum RegionCombineMode
{
    /// <summary>Intersect.</summary>
    And = 0x01,

    /// <summary>Union.</summary>
    Or = 0x02,

    /// <summary>Symmetric difference.</summary>
    Xor = 0x03,

    /// <summary>Subtract.</summary>
    Diff = 0x04,

    /// <summary>Replace.</summary>
    Copy = 0x05,
}

/// <summary>
/// Where a text record's stated point sits relative to the text it draws.
/// </summary>
/// <remarks>
/// The horizontal bits are a two-bit field rather than independent flags —
/// <see cref="Centre"/> is <c>Right | 0x0004</c> — so they must be masked with
/// <see cref="TextAlignmentMask.Horizontal"/> and compared, not tested individually.
/// [MS-WMF] 2.1.2.3 misstates <see cref="Baseline"/> as 0x0018; it is 0x0010
/// (<c>emfio/inc/mtftools.hxx:184-186</c>).
/// </remarks>
[Flags]
public enum TextAlignment
{
    /// <summary>Left, top, and leave the current position alone — GDI's initial state.</summary>
    Default = 0x0000,

    /// <summary>Advance the current position past the drawn text.</summary>
    UpdateCurrentPosition = 0x0001,

    /// <summary>The point is the right edge.</summary>
    Right = 0x0002,

    /// <summary>The point is the horizontal centre.</summary>
    Centre = 0x0006,

    /// <summary>The point is the bottom of the text cell.</summary>
    Bottom = 0x0008,

    /// <summary>The point is on the baseline.</summary>
    Baseline = 0x0010,

    /// <summary>The run reads right to left.</summary>
    RightToLeftReading = 0x0100,
}

/// <summary>The masks that pick the two multi-bit fields out of a <see cref="TextAlignment"/>.</summary>
/// <remarks>
/// Constants rather than enum members because they duplicate the values of the members they
/// select, which a flags enum cannot express and an analyser rightly objects to.
/// </remarks>
public static class TextAlignmentMask
{
    /// <summary>The bits that carry the horizontal alignment.</summary>
    public const TextAlignment Horizontal = (TextAlignment)0x0006;

    /// <summary>The bits that carry the vertical alignment.</summary>
    public const TextAlignment Vertical = (TextAlignment)0x0018;
}

/// <summary>The kinds of brush a metafile can select. [MS-WMF] 2.1.1.4.</summary>
public enum BrushStyle
{
    /// <summary>A uniform colour.</summary>
    Solid = 0,

    /// <summary>Nothing is filled.</summary>
    Null = 1,

    /// <summary>One of the six GDI hatch patterns.</summary>
    Hatched = 2,

    /// <summary>A tiled monochrome bitmap.</summary>
    Pattern = 3,

    /// <summary>An index into the logical palette.</summary>
    Indexed = 4,

    /// <summary>A tiled DIB.</summary>
    DibPattern = 5,

    /// <summary>A tiled DIB addressed by pointer.</summary>
    DibPatternPt = 6,

    /// <summary>A tiled 8x8 pattern.</summary>
    Pattern8x8 = 7,

    /// <summary>A tiled 8x8 DIB.</summary>
    DibPattern8x8 = 8,

    /// <summary>A tiled monochrome pattern.</summary>
    MonoPattern = 9,
}

/// <summary>The six GDI hatch patterns. [MS-WMF] 2.1.1.12.</summary>
public enum HatchStyle
{
    /// <summary>Horizontal lines.</summary>
    Horizontal = 0,

    /// <summary>Vertical lines.</summary>
    Vertical = 1,

    /// <summary>Lines at 45 degrees, descending to the right.</summary>
    ForwardDiagonal = 2,

    /// <summary>Lines at 45 degrees, ascending to the right.</summary>
    BackwardDiagonal = 3,

    /// <summary>Horizontal and vertical lines.</summary>
    Cross = 4,

    /// <summary>Both diagonals.</summary>
    DiagonalCross = 5,
}

/// <summary>
/// The pen-style bit fields. [MS-EMF] 2.1.23, and <c>emfio/inc/mtftools.hxx:207-232</c>.
/// </summary>
/// <remarks>
/// A WMF pen carries only the low nibble; the cap, join and geometric bits are EMF's, and are
/// declared here because the pen object is shared between the two formats.
/// </remarks>
public static class PenStyle
{
    /// <summary>Solid.</summary>
    public const uint Solid = 0x00000000;

    /// <summary>Dashed.</summary>
    public const uint Dash = 0x00000001;

    /// <summary>Dotted.</summary>
    public const uint Dot = 0x00000002;

    /// <summary>Alternating dash and dot.</summary>
    public const uint DashDot = 0x00000003;

    /// <summary>Alternating dash and two dots.</summary>
    public const uint DashDotDot = 0x00000004;

    /// <summary>Nothing is stroked.</summary>
    public const uint Null = 0x00000005;

    /// <summary>Solid, drawn inside the shape's frame.</summary>
    public const uint InsideFrame = 0x00000006;

    /// <summary>A user-supplied dash array.</summary>
    public const uint UserStyle = 0x00000007;

    /// <summary>Every other pixel.</summary>
    public const uint Alternate = 0x00000008;

    /// <summary>The bits carrying the dash style.</summary>
    public const uint StyleMask = 0x0000000F;

    /// <summary>Round line ends.</summary>
    public const uint EndCapRound = 0x00000000;

    /// <summary>Square line ends, extended past the endpoint.</summary>
    public const uint EndCapSquare = 0x00000100;

    /// <summary>Flat line ends.</summary>
    public const uint EndCapFlat = 0x00000200;

    /// <summary>The bits carrying the end cap.</summary>
    public const uint EndCapMask = 0x00000F00;

    /// <summary>Round joins.</summary>
    public const uint JoinRound = 0x00000000;

    /// <summary>Bevelled joins.</summary>
    public const uint JoinBevel = 0x00001000;

    /// <summary>Mitred joins.</summary>
    public const uint JoinMiter = 0x00002000;

    /// <summary>The bits carrying the join.</summary>
    public const uint JoinMask = 0x0000F000;
}

/// <summary>
/// The predefined objects a metafile may select without creating them.
/// </summary>
/// <remarks>
/// In EMF a stock object is named by an index with the top bit set
/// (<c>ENHMETA_STOCK_OBJECT</c>, <c>emfio/inc/mtftools.hxx:66</c>); in WMF the same values
/// appear as ordinary handles, which is one reason the object table must tolerate a select of
/// a handle it never saw created.
/// </remarks>
public enum StockObject
{
    /// <summary>An opaque white brush.</summary>
    WhiteBrush = 0,

    /// <summary>An opaque light grey brush.</summary>
    LightGrayBrush = 1,

    /// <summary>An opaque grey brush.</summary>
    GrayBrush = 2,

    /// <summary>An opaque dark grey brush.</summary>
    DarkGrayBrush = 3,

    /// <summary>An opaque black brush.</summary>
    BlackBrush = 4,

    /// <summary>A brush that fills nothing.</summary>
    NullBrush = 5,

    /// <summary>A white hairline pen.</summary>
    WhitePen = 6,

    /// <summary>A black hairline pen.</summary>
    BlackPen = 7,

    /// <summary>A pen that strokes nothing.</summary>
    NullPen = 8,

    /// <summary>The fixed-pitch ANSI font.</summary>
    AnsiFixedFont = 11,

    /// <summary>The variable-pitch ANSI font.</summary>
    AnsiVariableFont = 12,

    /// <summary>The fixed-pitch system font.</summary>
    SystemFixedFont = 16,
}

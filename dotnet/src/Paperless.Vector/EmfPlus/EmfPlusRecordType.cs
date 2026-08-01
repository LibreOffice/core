namespace Paperless.Vector.EmfPlus;

/// <summary>
/// The EMF+ record types, as [MS-EMFPLUS] 2.1.1.1 numbers them.
/// </summary>
/// <remarks>
/// The numbering is its own space: an EMF+ record type shares nothing with an
/// <see cref="Emf.EmfRecordType"/> and the two streams are interleaved in one file, so keeping
/// them in separate enums is what stops a dispatch reading the wrong table. Every value in
/// <c>emfio</c>'s list is here whether or not it is drawn, because a record that is named can be
/// skipped deliberately and one that is not shows up as "unknown" on every file that uses it.
/// </remarks>
internal enum EmfPlusRecordType : ushort
{
    /// <summary>The first record, which states the version and whether the file is dual.</summary>
    Header = 0x4001,

    /// <summary>The last record.</summary>
    EndOfFile = 0x4002,

    /// <summary>Producer-private data, ignored.</summary>
    Comment = 0x4003,

    /// <summary>Hands the device context back to GDI for the records that follow.</summary>
    GetDc = 0x4004,

    /// <summary>Begins a multi-format section; only the EMF+ part is read.</summary>
    MultiFormatStart = 0x4005,

    /// <summary>One alternative rendering of a multi-format section.</summary>
    MultiFormatSection = 0x4006,

    /// <summary>Ends a multi-format section.</summary>
    MultiFormatEnd = 0x4007,

    /// <summary>Creates a pen, brush, path, region, image, font, string format or image attributes.</summary>
    Object = 0x4008,

    /// <summary>Fills the whole clip with one colour.</summary>
    Clear = 0x4009,

    /// <summary>Fills a list of rectangles.</summary>
    FillRects = 0x400A,

    /// <summary>Strokes a list of rectangles.</summary>
    DrawRects = 0x400B,

    /// <summary>Fills a polygon stated inline.</summary>
    FillPolygon = 0x400C,

    /// <summary>Strokes a run of connected lines.</summary>
    DrawLines = 0x400D,

    /// <summary>Fills an ellipse.</summary>
    FillEllipse = 0x400E,

    /// <summary>Strokes an ellipse.</summary>
    DrawEllipse = 0x400F,

    /// <summary>Fills a pie slice.</summary>
    FillPie = 0x4010,

    /// <summary>Strokes a pie slice.</summary>
    DrawPie = 0x4011,

    /// <summary>Strokes an arc.</summary>
    DrawArc = 0x4012,

    /// <summary>Fills a region object.</summary>
    FillRegion = 0x4013,

    /// <summary>Fills a path object.</summary>
    FillPath = 0x4014,

    /// <summary>Strokes a path object.</summary>
    DrawPath = 0x4015,

    /// <summary>Fills a closed cardinal spline.</summary>
    FillClosedCurve = 0x4016,

    /// <summary>Strokes a closed cardinal spline.</summary>
    DrawClosedCurve = 0x4017,

    /// <summary>Strokes an open cardinal spline.</summary>
    DrawCurve = 0x4018,

    /// <summary>Strokes a run of cubic Béziers.</summary>
    DrawBeziers = 0x4019,

    /// <summary>Draws an image into a rectangle.</summary>
    DrawImage = 0x401A,

    /// <summary>Draws an image into a parallelogram named by three corners.</summary>
    DrawImagePoints = 0x401B,

    /// <summary>Draws a string with a font and a string format.</summary>
    DrawString = 0x401C,

    /// <summary>Moves the origin brushes and pens are aligned to.</summary>
    SetRenderingOrigin = 0x401D,

    /// <summary>Sets the anti-aliasing mode.</summary>
    SetAntiAliasMode = 0x401E,

    /// <summary>Sets the text rendering hint.</summary>
    SetTextRenderingHint = 0x401F,

    /// <summary>Sets the gamma text is drawn at.</summary>
    SetTextContrast = 0x4020,

    /// <summary>Sets how images are resampled.</summary>
    SetInterpolationMode = 0x4021,

    /// <summary>Sets the half-pixel offset convention.</summary>
    SetPixelOffsetMode = 0x4022,

    /// <summary>Sets whether drawing is blended or copied.</summary>
    SetCompositingMode = 0x4023,

    /// <summary>Sets the compositing quality.</summary>
    SetCompositingQuality = 0x4024,

    /// <summary>Saves the graphics state under a numbered index.</summary>
    Save = 0x4025,

    /// <summary>Restores a numbered graphics state.</summary>
    Restore = 0x4026,

    /// <summary>Opens a container with its own coordinate space.</summary>
    BeginContainer = 0x4027,

    /// <summary>Opens a container that keeps the current coordinate space.</summary>
    BeginContainerNoParams = 0x4028,

    /// <summary>Closes a container.</summary>
    EndContainer = 0x4029,

    /// <summary>Replaces the world transform.</summary>
    SetWorldTransform = 0x402A,

    /// <summary>Resets the world transform to the identity.</summary>
    ResetWorldTransform = 0x402B,

    /// <summary>Combines a transform with the world transform.</summary>
    MultiplyWorldTransform = 0x402C,

    /// <summary>Combines a translation with the world transform.</summary>
    TranslateWorldTransform = 0x402D,

    /// <summary>Combines a scale with the world transform.</summary>
    ScaleWorldTransform = 0x402E,

    /// <summary>Combines a rotation with the world transform.</summary>
    RotateWorldTransform = 0x402F,

    /// <summary>Sets the page unit and scale, which sit between world and device space.</summary>
    SetPageTransform = 0x4030,

    /// <summary>Clips nothing.</summary>
    ResetClip = 0x4031,

    /// <summary>Combines a rectangle into the clip.</summary>
    SetClipRect = 0x4032,

    /// <summary>Combines a path object into the clip.</summary>
    SetClipPath = 0x4033,

    /// <summary>Combines a region object into the clip.</summary>
    SetClipRegion = 0x4034,

    /// <summary>Moves the clip.</summary>
    OffsetClip = 0x4035,

    /// <summary>Draws glyphs the producer has already positioned.</summary>
    DrawDriverString = 0x4036,

    /// <summary>Fills and strokes a path in one record.</summary>
    StrokeFillPath = 0x4037,

    /// <summary>A serialised custom object, which only the producer understands.</summary>
    SerializableObject = 0x4038,

    /// <summary>Terminal-server graphics state, which no drawing depends on.</summary>
    SetTsGraphics = 0x4039,

    /// <summary>Terminal-server clip state.</summary>
    SetTsClip = 0x403A,
}

/// <summary>The kinds of object an <see cref="EmfPlusRecordType.Object"/> record creates.</summary>
/// <remarks>
/// Stated in the record's flags word as the middle byte, which is why the values are shifted:
/// <c>flags &amp; 0x7F00</c> is the type and <c>flags &amp; 0xFF</c> the slot.
/// </remarks>
internal enum EmfPlusObjectType
{
    /// <summary>A brush.</summary>
    Brush = 0x100,

    /// <summary>A pen.</summary>
    Pen = 0x200,

    /// <summary>A path.</summary>
    Path = 0x300,

    /// <summary>A region.</summary>
    Region = 0x400,

    /// <summary>An image.</summary>
    Image = 0x500,

    /// <summary>A font.</summary>
    Font = 0x600,

    /// <summary>A string format.</summary>
    StringFormat = 0x700,

    /// <summary>Colour and gamma adjustments applied to an image.</summary>
    ImageAttributes = 0x800,

    /// <summary>A custom line cap.</summary>
    CustomLineCap = 0x900,
}

/// <summary>The unit a size or a coordinate is stated in ([MS-EMFPLUS] 2.1.1.33).</summary>
internal enum EmfPlusUnit
{
    /// <summary>The world space the world transform maps from.</summary>
    World = 0,

    /// <summary>The display device's own unit.</summary>
    Display = 1,

    /// <summary>A device pixel.</summary>
    Pixel = 2,

    /// <summary>A point, 1/72 inch.</summary>
    Point = 3,

    /// <summary>An inch.</summary>
    Inch = 4,

    /// <summary>1/300 inch.</summary>
    Document = 5,

    /// <summary>A millimetre.</summary>
    Millimetre = 6,
}

/// <summary>How a clip record combines its shape with the clip already in force.</summary>
internal enum EmfPlusCombineMode
{
    /// <summary>The new shape becomes the clip.</summary>
    Replace = 0,

    /// <summary>The clip narrows to the overlap.</summary>
    Intersect = 1,

    /// <summary>The clip widens to cover both.</summary>
    Union = 2,

    /// <summary>The clip becomes the parts covered by exactly one.</summary>
    Xor = 3,

    /// <summary>The new shape is taken out of the clip.</summary>
    Exclude = 4,

    /// <summary>The clip becomes the part of the new shape outside the old clip.</summary>
    Complement = 5,
}

/// <summary>The five brush types ([MS-EMFPLUS] 2.1.1.3).</summary>
internal enum EmfPlusBrushType
{
    /// <summary>One colour.</summary>
    SolidColour = 0,

    /// <summary>A two-colour hatch pattern.</summary>
    Hatch = 1,

    /// <summary>A tiled image.</summary>
    Texture = 2,

    /// <summary>A gradient from a centre point out to a boundary path.</summary>
    PathGradient = 3,

    /// <summary>A gradient along a line.</summary>
    LinearGradient = 4,
}

/// <summary>The bits of a pen's data flags that say which optional fields follow.</summary>
/// <remarks>
/// <b>The fields are positional and every one of them is optional</b>, so a decoder that reads
/// them in the wrong order or skips a set bit misaligns everything after it — including the
/// brush at the end, which then reads a pen's dash array as a colour.
/// </remarks>
internal static class EmfPlusPenData
{
    /// <summary>A transform applied to the pen.</summary>
    public const uint Transform = 0x0001;

    /// <summary>A start cap.</summary>
    public const uint StartCap = 0x0002;

    /// <summary>An end cap.</summary>
    public const uint EndCap = 0x0004;

    /// <summary>A line join.</summary>
    public const uint Join = 0x0008;

    /// <summary>A miter limit.</summary>
    public const uint MiterLimit = 0x0010;

    /// <summary>One of the five predefined line styles.</summary>
    public const uint LineStyle = 0x0020;

    /// <summary>The cap the ends of each dash take.</summary>
    public const uint DashedLineCap = 0x0040;

    /// <summary>How far into the dash pattern the line starts.</summary>
    public const uint DashedLineOffset = 0x0080;

    /// <summary>A custom dash array.</summary>
    public const uint DashedLine = 0x0100;

    /// <summary>Which side of the path the stroke sits on.</summary>
    public const uint Alignment = 0x0200;

    /// <summary>A compound-line array, which splits the stroke into parallel strips.</summary>
    public const uint CompoundLine = 0x0400;

    /// <summary>A custom start cap, stated as a path.</summary>
    public const uint CustomStartCap = 0x0800;

    /// <summary>A custom end cap.</summary>
    public const uint CustomEndCap = 0x1000;
}

/// <summary>The bits a brush's <c>BrushData</c> word can carry.</summary>
internal static class EmfPlusBrushData
{
    /// <summary>A path gradient states its boundary as a path rather than as bare points.</summary>
    public const uint Path = 0x01;

    /// <summary>A transform applied to the brush.</summary>
    public const uint Transform = 0x02;

    /// <summary>An explicit list of colours and positions.</summary>
    public const uint PresetColours = 0x04;

    /// <summary>A horizontal blend-factor curve.</summary>
    public const uint BlendFactorsHorizontal = 0x08;

    /// <summary>A vertical blend-factor curve, which only a linear gradient has.</summary>
    public const uint BlendFactorsVertical = 0x10;

    /// <summary>The brush is a focus-scaled path gradient.</summary>
    public const uint FocusScales = 0x40;
}

namespace Paperless.Vector.Emf;

/// <summary>
/// The record types an enhanced metafile is made of. [MS-EMF] 2.1.1.
/// </summary>
/// <remarks>
/// Numbered rather than bit-encoded, unlike WMF's function words, and every record carries its
/// own size — so an unknown type costs nothing but a seek, which is why the switch that
/// dispatches these can afford to be exhaustive rather than defensive.
/// </remarks>
internal enum EmfRecordType : uint
{
    /// <summary>The header, read before the replay begins.</summary>
    Header = 1,

    /// <summary>Cubic Béziers through 32-bit points.</summary>
    PolyBezier = 2,

    /// <summary>A closed polygon of 32-bit points.</summary>
    Polygon = 3,

    /// <summary>An open polyline of 32-bit points.</summary>
    Polyline = 4,

    /// <summary>Béziers continuing from the current position.</summary>
    PolyBezierTo = 5,

    /// <summary>A polyline continuing from the current position.</summary>
    PolylineTo = 6,

    /// <summary>Several polylines in one record.</summary>
    PolyPolyline = 7,

    /// <summary>Several polygons in one record.</summary>
    PolyPolygon = 8,

    /// <summary>Sets the window extent.</summary>
    SetWindowExtEx = 9,

    /// <summary>Sets the window origin.</summary>
    SetWindowOrgEx = 10,

    /// <summary>Sets the viewport extent.</summary>
    SetViewportExtEx = 11,

    /// <summary>Sets the viewport origin.</summary>
    SetViewportOrgEx = 12,

    /// <summary>Sets the brush origin, which a tiled fill would start from.</summary>
    SetBrushOrgEx = 13,

    /// <summary>The end of the record stream.</summary>
    Eof = 14,

    /// <summary>Sets one pixel to a colour.</summary>
    SetPixelV = 15,

    /// <summary>Font-mapper flags, which change no geometry.</summary>
    SetMapperFlags = 16,

    /// <summary>Sets the mapping mode.</summary>
    SetMapMode = 17,

    /// <summary>Sets the background mode.</summary>
    SetBkMode = 18,

    /// <summary>Sets the polygon fill rule.</summary>
    SetPolyFillMode = 19,

    /// <summary>Sets the binary raster operation.</summary>
    SetRop2 = 20,

    /// <summary>Sets how a stretched bitmap is resampled.</summary>
    SetStretchBltMode = 21,

    /// <summary>Sets the text alignment word.</summary>
    SetTextAlign = 22,

    /// <summary>Colour adjustment for stretched bitmaps.</summary>
    SetColorAdjustment = 23,

    /// <summary>Sets the text colour.</summary>
    SetTextColor = 24,

    /// <summary>Sets the background colour.</summary>
    SetBkColor = 25,

    /// <summary>Moves the clip region.</summary>
    OffsetClipRgn = 26,

    /// <summary>Moves the current position.</summary>
    MoveToEx = 27,

    /// <summary>Intersects the clip with the metaregion.</summary>
    SetMetaRgn = 28,

    /// <summary>Subtracts a rectangle from the clip.</summary>
    ExcludeClipRect = 29,

    /// <summary>Intersects the clip with a rectangle.</summary>
    IntersectClipRect = 30,

    /// <summary>Scales the viewport extent by a ratio.</summary>
    ScaleViewportExtEx = 31,

    /// <summary>Scales the window extent by a ratio.</summary>
    ScaleWindowExtEx = 32,

    /// <summary>Pushes the device context.</summary>
    SaveDc = 33,

    /// <summary>Pops the device context.</summary>
    RestoreDc = 34,

    /// <summary>Replaces the world transform.</summary>
    SetWorldTransform = 35,

    /// <summary>Combines a transform with the world transform.</summary>
    ModifyWorldTransform = 36,

    /// <summary>Selects an object by handle.</summary>
    SelectObject = 37,

    /// <summary>Creates a pen.</summary>
    CreatePen = 38,

    /// <summary>Creates a brush.</summary>
    CreateBrushIndirect = 39,

    /// <summary>Frees a handle.</summary>
    DeleteObject = 40,

    /// <summary>An arc named by a centre, a radius and two angles.</summary>
    AngleArc = 41,

    /// <summary>An ellipse inscribed in a rectangle.</summary>
    Ellipse = 42,

    /// <summary>A rectangle.</summary>
    Rectangle = 43,

    /// <summary>A rounded rectangle.</summary>
    RoundRect = 44,

    /// <summary>An open elliptical arc.</summary>
    Arc = 45,

    /// <summary>An arc closed by its chord.</summary>
    Chord = 46,

    /// <summary>An arc closed through the centre.</summary>
    Pie = 47,

    /// <summary>Selects a palette.</summary>
    SelectPalette = 48,

    /// <summary>Creates a palette.</summary>
    CreatePalette = 49,

    /// <summary>Replaces entries in a palette.</summary>
    SetPaletteEntries = 50,

    /// <summary>Resizes a palette.</summary>
    ResizePalette = 51,

    /// <summary>Maps the logical palette onto the device's.</summary>
    RealizePalette = 52,

    /// <summary>A flood fill, which needs to read the page back.</summary>
    ExtFloodFill = 53,

    /// <summary>A line from the current position.</summary>
    LineTo = 54,

    /// <summary>An arc continuing from the current position.</summary>
    ArcTo = 55,

    /// <summary>Points with per-point verbs, of 32-bit coordinates.</summary>
    PolyDraw = 56,

    /// <summary>Sets which way arcs sweep.</summary>
    SetArcDirection = 57,

    /// <summary>Sets the miter limit.</summary>
    SetMiterLimit = 58,

    /// <summary>Starts recording a path rather than drawing.</summary>
    BeginPath = 59,

    /// <summary>Stops recording a path.</summary>
    EndPath = 60,

    /// <summary>Closes the open subpath.</summary>
    CloseFigure = 61,

    /// <summary>Fills the recorded path.</summary>
    FillPath = 62,

    /// <summary>Strokes and fills the recorded path.</summary>
    StrokeAndFillPath = 63,

    /// <summary>Strokes the recorded path.</summary>
    StrokePath = 64,

    /// <summary>Replaces curves in the path with lines.</summary>
    FlattenPath = 65,

    /// <summary>Replaces the path with its stroked outline.</summary>
    WidenPath = 66,

    /// <summary>Uses the recorded path as a clip.</summary>
    SelectClipPath = 67,

    /// <summary>Discards the recorded path.</summary>
    AbortPath = 68,

    /// <summary>Private data, including EMF+ records and embedded documents.</summary>
    Comment = 70,

    /// <summary>Fills a region with a brush.</summary>
    FillRgn = 71,

    /// <summary>Strokes a region's border.</summary>
    FrameRgn = 72,

    /// <summary>Inverts a region's colours.</summary>
    InvertRgn = 73,

    /// <summary>Fills a region with the selected brush.</summary>
    PaintRgn = 74,

    /// <summary>Combines a region into the clip.</summary>
    ExtSelectClipRgn = 75,

    /// <summary>Blits a bitmap unscaled.</summary>
    BitBlt = 76,

    /// <summary>Blits a bitmap scaled.</summary>
    StretchBlt = 77,

    /// <summary>Blits a bitmap through a mask.</summary>
    MaskBlt = 78,

    /// <summary>Blits a bitmap into a parallelogram.</summary>
    PlgBlt = 79,

    /// <summary>Places a DIB one to one.</summary>
    SetDIBitsToDevice = 80,

    /// <summary>Places a DIB scaled.</summary>
    StretchDIBits = 81,

    /// <summary>Creates a font.</summary>
    ExtCreateFontIndirectW = 82,

    /// <summary>Draws eight-bit text.</summary>
    ExtTextOutA = 83,

    /// <summary>Draws Unicode text.</summary>
    ExtTextOutW = 84,

    /// <summary>Béziers through 16-bit points.</summary>
    PolyBezier16 = 85,

    /// <summary>A closed polygon of 16-bit points.</summary>
    Polygon16 = 86,

    /// <summary>An open polyline of 16-bit points.</summary>
    Polyline16 = 87,

    /// <summary>16-bit Béziers continuing from the current position.</summary>
    PolyBezierTo16 = 88,

    /// <summary>A 16-bit polyline continuing from the current position.</summary>
    PolylineTo16 = 89,

    /// <summary>Several 16-bit polylines.</summary>
    PolyPolyline16 = 90,

    /// <summary>Several 16-bit polygons.</summary>
    PolyPolygon16 = 91,

    /// <summary>Points with per-point verbs, of 16-bit coordinates.</summary>
    PolyDraw16 = 92,

    /// <summary>Creates a brush from a monochrome bitmap.</summary>
    CreateMonoBrush = 93,

    /// <summary>Creates a brush from a DIB.</summary>
    CreateDibPatternBrushPt = 94,

    /// <summary>Creates a pen with caps, joins and a dash array.</summary>
    ExtCreatePen = 95,

    /// <summary>Several eight-bit text runs in one record.</summary>
    PolyTextOutA = 96,

    /// <summary>Several Unicode text runs in one record.</summary>
    PolyTextOutW = 97,

    /// <summary>Sets the colour-management mode.</summary>
    SetIcmMode = 98,

    /// <summary>Creates a colour space.</summary>
    CreateColorSpace = 99,

    /// <summary>Selects a colour space.</summary>
    SetColorSpace = 100,

    /// <summary>Frees a colour space.</summary>
    DeleteColorSpace = 101,

    /// <summary>OpenGL data.</summary>
    GlsRecord = 102,

    /// <summary>OpenGL data with bounds.</summary>
    GlsBoundedRecord = 103,

    /// <summary>The pixel format the recording device used.</summary>
    PixelFormat = 104,

    /// <summary>A printer escape that draws.</summary>
    DrawEscape = 105,

    /// <summary>A printer escape.</summary>
    ExtEscape = 106,

    /// <summary>The start of a print job.</summary>
    StartDoc = 107,

    /// <summary>A compact text record.</summary>
    SmallTextOut = 108,

    /// <summary>Forces a font mapping.</summary>
    ForceUfiMapping = 109,

    /// <summary>A named printer escape.</summary>
    NamedEscape = 110,

    /// <summary>Colour-corrects a palette.</summary>
    ColorCorrectPalette = 111,

    /// <summary>Names an eight-bit ICM profile.</summary>
    SetIcmProfileA = 112,

    /// <summary>Names a Unicode ICM profile.</summary>
    SetIcmProfileW = 113,

    /// <summary>Blits a bitmap with an alpha channel or a constant alpha.</summary>
    AlphaBlend = 114,

    /// <summary>Sets the drawing layout, left-to-right or right-to-left.</summary>
    SetLayout = 115,

    /// <summary>Blits a bitmap with one colour knocked out.</summary>
    TransparentBlt = 116,

    /// <summary>The same, under an older name.</summary>
    TransparentDib = 117,

    /// <summary>Fills rectangles or triangles with interpolated colours.</summary>
    GradientFill = 118,

    /// <summary>Links font identifiers.</summary>
    SetLinkedUfis = 119,

    /// <summary>Sets the extra spacing justification adds.</summary>
    SetTextJustification = 120,
}

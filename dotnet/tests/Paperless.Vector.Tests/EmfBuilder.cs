using System.Buffers.Binary;
using System.Text;

namespace Paperless.Vector.Tests;

/// <summary>
/// Builds enhanced metafiles a record at a time, for tests that need one exact record.
/// </summary>
/// <remarks>
/// <para>
/// Hand-built for the same reason <see cref="WmfBuilder"/> is: a test that asserts on the
/// mapping of a <c>SetWorldTransform</c> has to be able to write exactly that record and nothing
/// else. A producer's output is a fixture for fidelity, not for a unit test.
/// </para>
/// <para>
/// The reference device is stated as 8000 pixels to 80 mm, which makes one logical unit exactly
/// 1/100 mm and lets an assertion name the millimetre it expects. That is the same trick the WMF
/// fixture plays with 2540 units to the inch, and it is worth playing because a metafile test
/// that has to reason about a scale factor is testing the test.
/// </para>
/// <para>
/// Every record is a whole number of 32-bit words including its own eight-byte header, and the
/// header's byte count and record count are both checked by real consumers — so both are filled
/// in when the file is closed rather than guessed at.
/// </para>
/// </remarks>
internal sealed class EmfBuilder
{
    private readonly List<byte> _records = [];
    private int _count;

    /// <summary>The logical bounding rectangle, in device units.</summary>
    public (int Left, int Top, int Right, int Bottom) Bounds { get; set; } = (0, 0, 7999, 5999);

    /// <summary>The physical frame, in 1/100 mm.</summary>
    public (int Left, int Top, int Right, int Bottom) Frame { get; set; } = (0, 0, 7999, 5999);

    /// <summary>The reference device's size in pixels.</summary>
    public (int Width, int Height) DevicePixels { get; set; } = (8000, 6000);

    /// <summary>The reference device's size in millimetres.</summary>
    public (int Width, int Height) DeviceMillimetres { get; set; } = (80, 60);

    /// <summary>Appends a record with 32-bit parameters.</summary>
    public EmfBuilder Record(EmfFunction function, params int[] parameters)
    {
        ArgumentNullException.ThrowIfNull(parameters);

        Span<byte> payload = parameters.Length == 0 ? [] : new byte[parameters.Length * 4];
        for (int i = 0; i < parameters.Length; i++)
        {
            BinaryPrimitives.WriteInt32LittleEndian(payload[(i * 4)..], parameters[i]);
        }

        return Raw(function, payload);
    }

    /// <summary>Appends a record with an arbitrary payload, padded to a whole 32-bit word.</summary>
    public EmfBuilder Raw(EmfFunction function, ReadOnlySpan<byte> payload)
    {
        int padding = (4 - (payload.Length & 3)) & 3;
        byte[] header = new byte[8];

        BinaryPrimitives.WriteUInt32LittleEndian(header, (uint)function);
        BinaryPrimitives.WriteUInt32LittleEndian(header.AsSpan(4), (uint)(8 + payload.Length + padding));

        _records.AddRange(header);
        _records.AddRange(payload.ToArray());
        for (int i = 0; i < padding; i++) _records.Add(0);
        _count++;

        return this;
    }

    /// <summary>Appends a record whose payload is a bounds rectangle then more parameters.</summary>
    public EmfBuilder Bounded(EmfFunction function, params int[] parameters)
        => Record(function, [0, 0, 0, 0, .. parameters]);

    /// <summary>Appends a <c>CreatePen</c> at a stated handle.</summary>
    public EmfBuilder Pen(int handle, int style, int width, byte r, byte g, byte b)
        => Record(EmfFunction.CreatePen, handle, style, width, 0, r | (g << 8) | (b << 16));

    /// <summary>Appends an <c>ExtCreatePen</c>, optionally with a user dash array.</summary>
    public EmfBuilder ExtendedPen(int handle, int style, int width, byte r, byte g, byte b, params int[] dashes)
    {
        ArgumentNullException.ThrowIfNull(dashes);

        return Record(
            EmfFunction.ExtCreatePen,
            [handle, 0, 0, 0, 0, style, width, 0, r | (g << 8) | (b << 16), 0, dashes.Length, .. dashes]);
    }

    /// <summary>Appends a <c>CreateBrushIndirect</c> with a solid colour.</summary>
    public EmfBuilder SolidBrush(int handle, byte r, byte g, byte b)
        => Record(EmfFunction.CreateBrushIndirect, handle, 0, r | (g << 8) | (b << 16), 0);

    /// <summary>Appends a <c>CreateBrushIndirect</c> that fills nothing.</summary>
    public EmfBuilder NullBrush(int handle) => Record(EmfFunction.CreateBrushIndirect, handle, 1, 0, 0);

    /// <summary>Appends a <c>CreateBrushIndirect</c> with one of the six hatches.</summary>
    public EmfBuilder HatchBrush(int handle, byte r, byte g, byte b, int hatch)
        => Record(EmfFunction.CreateBrushIndirect, handle, 2, r | (g << 8) | (b << 16), hatch);

    /// <summary>Appends a <c>SelectObject</c>.</summary>
    public EmfBuilder Select(int handle) => Record(EmfFunction.SelectObject, handle);

    /// <summary>Appends a <c>DeleteObject</c>.</summary>
    public EmfBuilder Delete(int handle) => Record(EmfFunction.DeleteObject, handle);

    /// <summary>Appends a <c>Rectangle</c>, whose corners are stated as they are.</summary>
    public EmfBuilder Rectangle(int left, int top, int right, int bottom)
        => Record(EmfFunction.Rectangle, left, top, right, bottom);

    /// <summary>Appends an <c>Ellipse</c>.</summary>
    public EmfBuilder Ellipse(int left, int top, int right, int bottom)
        => Record(EmfFunction.Ellipse, left, top, right, bottom);

    /// <summary>Appends a <c>Polygon16</c> or <c>Polyline16</c>.</summary>
    public EmfBuilder Poly16(EmfFunction function, params (short X, short Y)[] points)
    {
        ArgumentNullException.ThrowIfNull(points);

        byte[] payload = new byte[20 + (points.Length * 4)];
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(16), points.Length);

        for (int i = 0; i < points.Length; i++)
        {
            BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(20 + (i * 4)), points[i].X);
            BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(22 + (i * 4)), points[i].Y);
        }

        return Raw(function, payload);
    }

    /// <summary>Appends a <c>Polygon</c> or <c>Polyline</c> with 32-bit points.</summary>
    public EmfBuilder Poly32(EmfFunction function, params (int X, int Y)[] points)
    {
        ArgumentNullException.ThrowIfNull(points);

        byte[] payload = new byte[20 + (points.Length * 8)];
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(16), points.Length);

        for (int i = 0; i < points.Length; i++)
        {
            BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(20 + (i * 8)), points[i].X);
            BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(24 + (i * 8)), points[i].Y);
        }

        return Raw(function, payload);
    }

    /// <summary>Appends an <c>ExtCreateFontIndirectW</c>.</summary>
    public EmfBuilder Font(int handle, string family, int height, int escapement = 0, int weight = 400)
    {
        ArgumentNullException.ThrowIfNull(family);

        byte[] payload = new byte[4 + 92];
        BinaryPrimitives.WriteInt32LittleEndian(payload, handle);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(4), height);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(8), 0);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(12), escapement);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(16), escapement);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(20), weight);

        for (int i = 0; i < family.Length && i < 31; i++)
        {
            BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(32 + (i * 2)), family[i]);
        }

        return Raw(EmfFunction.ExtCreateFontIndirectW, payload);
    }

    /// <summary>
    /// Appends an <c>ExtTextOutW</c>, with the string and any advances placed after the header.
    /// </summary>
    /// <remarks>
    /// The string and the DX array are named by offsets from the start of the record, not by
    /// position, which is the whole reason this record is worth a builder: getting the offsets
    /// wrong reads the wrong bytes and the text comes out as noise rather than as an error.
    /// </remarks>
    public EmfBuilder Text(
        int x,
        int y,
        string text,
        uint options = 0,
        int[]? advances = null,
        (int Left, int Top, int Right, int Bottom)? rectangle = null)
    {
        ArgumentNullException.ThrowIfNull(text);

        // 8 record header + 16 bounds + 4 mode + 8 scales + 8 reference + 4 chars + 4 offString
        // + 4 options + 16 rectangle + 4 offDx.
        const int HeaderBytes = 76;

        int stringBytes = text.Length * 2;
        int stringPadding = (4 - (stringBytes & 3)) & 3;
        int dxAt = HeaderBytes + stringBytes + stringPadding;

        byte[] payload = new byte[dxAt - 8 + ((advances?.Length ?? 0) * 4)];

        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(16), 1);         // GM_COMPATIBLE
        BinaryPrimitives.WriteSingleLittleEndian(payload.AsSpan(20), 1f);
        BinaryPrimitives.WriteSingleLittleEndian(payload.AsSpan(24), 1f);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(28), x);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(32), y);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(36), text.Length);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(40), HeaderBytes);
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(44), options);

        if (rectangle is { } rect)
        {
            BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(48), rect.Left);
            BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(52), rect.Top);
            BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(56), rect.Right);
            BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(60), rect.Bottom);
        }

        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(64), advances is null ? 0 : dxAt);

        for (int i = 0; i < text.Length; i++)
        {
            BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(HeaderBytes - 8 + (i * 2)), text[i]);
        }

        if (advances is not null)
        {
            for (int i = 0; i < advances.Length; i++)
            {
                BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(dxAt - 8 + (i * 4)), advances[i]);
            }
        }

        return Raw(EmfFunction.ExtTextOutW, payload);
    }

    /// <summary>Appends an <c>ExtSelectClipRgn</c> holding a scan list of rectangles.</summary>
    public EmfBuilder ClipRegion(int mode, params (int Left, int Top, int Right, int Bottom)[] rectangles)
    {
        ArgumentNullException.ThrowIfNull(rectangles);

        byte[] region = new byte[32 + (rectangles.Length * 16)];
        BinaryPrimitives.WriteUInt32LittleEndian(region, 32);
        BinaryPrimitives.WriteUInt32LittleEndian(region.AsSpan(4), 1);          // RDH_RECTANGLES
        BinaryPrimitives.WriteUInt32LittleEndian(region.AsSpan(8), (uint)rectangles.Length);
        BinaryPrimitives.WriteUInt32LittleEndian(region.AsSpan(12), (uint)(rectangles.Length * 16));

        for (int i = 0; i < rectangles.Length; i++)
        {
            Span<byte> at = region.AsSpan(32 + (i * 16));
            BinaryPrimitives.WriteInt32LittleEndian(at, rectangles[i].Left);
            BinaryPrimitives.WriteInt32LittleEndian(at[4..], rectangles[i].Top);
            BinaryPrimitives.WriteInt32LittleEndian(at[8..], rectangles[i].Right);
            BinaryPrimitives.WriteInt32LittleEndian(at[12..], rectangles[i].Bottom);
        }

        byte[] payload = new byte[8 + region.Length];
        BinaryPrimitives.WriteUInt32LittleEndian(payload, (uint)region.Length);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(4), mode);
        region.CopyTo(payload.AsSpan(8));

        return Raw(EmfFunction.ExtSelectClipRgn, payload);
    }

    /// <summary>Appends a <c>SetWorldTransform</c>.</summary>
    public EmfBuilder World(float m11, float m12, float m21, float m22, float dx, float dy)
    {
        byte[] payload = new byte[24];
        BinaryPrimitives.WriteSingleLittleEndian(payload, m11);
        BinaryPrimitives.WriteSingleLittleEndian(payload.AsSpan(4), m12);
        BinaryPrimitives.WriteSingleLittleEndian(payload.AsSpan(8), m21);
        BinaryPrimitives.WriteSingleLittleEndian(payload.AsSpan(12), m22);
        BinaryPrimitives.WriteSingleLittleEndian(payload.AsSpan(16), dx);
        BinaryPrimitives.WriteSingleLittleEndian(payload.AsSpan(20), dy);
        return Raw(EmfFunction.SetWorldTransform, payload);
    }

    /// <summary>Appends a <c>GradientFill</c> between two colours across a rectangle.</summary>
    /// <param name="left">The rectangle's left edge in logical units.</param>
    /// <param name="top">The top edge.</param>
    /// <param name="right">The right edge.</param>
    /// <param name="bottom">The bottom edge.</param>
    /// <param name="from">The colour at the top-left vertex.</param>
    /// <param name="to">The colour at the bottom-right vertex.</param>
    /// <param name="vertical">True for top-to-bottom, false for left-to-right.</param>
    public EmfBuilder Gradient(
        int left,
        int top,
        int right,
        int bottom,
        (byte R, byte G, byte B) from,
        (byte R, byte G, byte B) to,
        bool vertical)
    {
        byte[] payload = new byte[16 + 12 + 32 + 8];

        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(16), 2);        // two vertices
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(20), 1);        // one rectangle
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(24), vertical ? 1u : 0u);

        Vertex(payload.AsSpan(28), left, top, from);
        Vertex(payload.AsSpan(44), right, bottom, to);

        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(60), 0);
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(64), 1);

        return Raw(EmfFunction.GradientFill, payload);

        static void Vertex(Span<byte> at, int x, int y, (byte R, byte G, byte B) colour)
        {
            BinaryPrimitives.WriteInt32LittleEndian(at, x);
            BinaryPrimitives.WriteInt32LittleEndian(at[4..], y);

            // Sixteen bits a channel, and only the high byte carries anything.
            BinaryPrimitives.WriteUInt16LittleEndian(at[8..], (ushort)(colour.R << 8));
            BinaryPrimitives.WriteUInt16LittleEndian(at[10..], (ushort)(colour.G << 8));
            BinaryPrimitives.WriteUInt16LittleEndian(at[12..], (ushort)(colour.B << 8));
            BinaryPrimitives.WriteUInt16LittleEndian(at[14..], 0);
        }
    }

    /// <summary>
    /// Appends a <c>StretchDIBits</c> carrying an uncompressed 24-bit DIB.
    /// </summary>
    /// <param name="destination">Where it goes, in logical units.</param>
    /// <param name="width">The bitmap's width in pixels.</param>
    /// <param name="height">The bitmap's height in pixels.</param>
    /// <param name="pixels">Blue, green and red for each pixel, bottom row first.</param>
    /// <param name="rop">The ternary raster operation.</param>
    public EmfBuilder Bitmap(
        (int X, int Y, int Width, int Height) destination,
        int width,
        int height,
        byte[] pixels,
        uint rop = 0x00CC0020)
    {
        ArgumentNullException.ThrowIfNull(pixels);

        const int HeaderBytes = 40;
        int stride = ((width * 24) + 31) / 32 * 4;

        byte[] dib = new byte[HeaderBytes + (stride * height)];
        BinaryPrimitives.WriteUInt32LittleEndian(dib, HeaderBytes);
        BinaryPrimitives.WriteInt32LittleEndian(dib.AsSpan(4), width);
        BinaryPrimitives.WriteInt32LittleEndian(dib.AsSpan(8), height);
        BinaryPrimitives.WriteUInt16LittleEndian(dib.AsSpan(12), 1);
        BinaryPrimitives.WriteUInt16LittleEndian(dib.AsSpan(14), 24);

        for (int y = 0; y < height; y++)
        {
            pixels.AsSpan(y * width * 3, width * 3).CopyTo(dib.AsSpan(HeaderBytes + (y * stride)));
        }

        // 8 record header + 16 bounds + 24 source and destination points + 16 offsets and sizes
        // + 4 usage + 4 rop + 8 destination extent.
        const int RecordHeader = 80;

        byte[] payload = new byte[RecordHeader - 8 + dib.Length];

        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(16), destination.X);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(20), destination.Y);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(24), 0);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(28), 0);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(32), width);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(36), height);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(40), RecordHeader);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(44), HeaderBytes);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(48), RecordHeader + HeaderBytes);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(52), dib.Length - HeaderBytes);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(56), 0);
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(60), rop);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(64), destination.Width);
        BinaryPrimitives.WriteInt32LittleEndian(payload.AsSpan(68), destination.Height);

        dib.CopyTo(payload.AsSpan(RecordHeader - 8));

        return Raw(EmfFunction.StretchDIBits, payload);
    }

    /// <summary>The finished file, with the header's counts filled in.</summary>
    public byte[] Build()
    {
        Record(EmfFunction.Eof, 0, 0, 20);

        const int HeaderSize = 88;
        byte[] header = new byte[HeaderSize];

        BinaryPrimitives.WriteUInt32LittleEndian(header, 1);
        BinaryPrimitives.WriteUInt32LittleEndian(header.AsSpan(4), HeaderSize);
        Rect(header.AsSpan(8), Bounds);
        Rect(header.AsSpan(24), Frame);
        BinaryPrimitives.WriteUInt32LittleEndian(header.AsSpan(40), 0x464D4520);
        BinaryPrimitives.WriteUInt32LittleEndian(header.AsSpan(44), 0x00010000);
        BinaryPrimitives.WriteUInt32LittleEndian(header.AsSpan(48), (uint)(HeaderSize + _records.Count));
        BinaryPrimitives.WriteUInt32LittleEndian(header.AsSpan(52), (uint)(_count + 1));
        BinaryPrimitives.WriteUInt16LittleEndian(header.AsSpan(56), 64);
        BinaryPrimitives.WriteInt32LittleEndian(header.AsSpan(72), DevicePixels.Width);
        BinaryPrimitives.WriteInt32LittleEndian(header.AsSpan(76), DevicePixels.Height);
        BinaryPrimitives.WriteInt32LittleEndian(header.AsSpan(80), DeviceMillimetres.Width);
        BinaryPrimitives.WriteInt32LittleEndian(header.AsSpan(84), DeviceMillimetres.Height);

        return [.. header, .. _records];

        static void Rect(Span<byte> at, (int Left, int Top, int Right, int Bottom) rect)
        {
            BinaryPrimitives.WriteInt32LittleEndian(at, rect.Left);
            BinaryPrimitives.WriteInt32LittleEndian(at[4..], rect.Top);
            BinaryPrimitives.WriteInt32LittleEndian(at[8..], rect.Right);
            BinaryPrimitives.WriteInt32LittleEndian(at[12..], rect.Bottom);
        }
    }

    /// <summary>The finished file, decoded.</summary>
    public VectorImage Decode() => VectorImages.Decode(Build());

    /// <summary>A description of the file, for a failure message.</summary>
    public override string ToString() => new StringBuilder()
        .Append(_count)
        .Append(" records, ")
        .Append(_records.Count)
        .Append(" bytes")
        .ToString();
}

/// <summary>The EMF record types the builder writes. [MS-EMF] 2.1.1.</summary>
internal enum EmfFunction : uint
{
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

    /// <summary>The end of the record stream.</summary>
    Eof = 14,

    /// <summary>Sets the mapping mode.</summary>
    SetMapMode = 17,

    /// <summary>Sets the background mode.</summary>
    SetBkMode = 18,

    /// <summary>Sets the polygon fill rule.</summary>
    SetPolyFillMode = 19,

    /// <summary>Sets the text alignment word.</summary>
    SetTextAlign = 22,

    /// <summary>Sets the text colour.</summary>
    SetTextColor = 24,

    /// <summary>Sets the background colour.</summary>
    SetBkColor = 25,

    /// <summary>Moves the current position.</summary>
    MoveToEx = 27,

    /// <summary>Subtracts a rectangle from the clip.</summary>
    ExcludeClipRect = 29,

    /// <summary>Intersects the clip with a rectangle.</summary>
    IntersectClipRect = 30,

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

    /// <summary>A line from the current position.</summary>
    LineTo = 54,

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

    /// <summary>Uses the recorded path as a clip.</summary>
    SelectClipPath = 67,

    /// <summary>Discards the recorded path.</summary>
    AbortPath = 68,

    /// <summary>Combines a region into the clip.</summary>
    ExtSelectClipRgn = 75,

    /// <summary>Places a DIB scaled.</summary>
    StretchDIBits = 81,

    /// <summary>Creates a font.</summary>
    ExtCreateFontIndirectW = 82,

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

    /// <summary>Creates a pen with caps, joins and a dash array.</summary>
    ExtCreatePen = 95,

    /// <summary>Blits a bitmap with an alpha channel or a constant alpha.</summary>
    AlphaBlend = 114,

    /// <summary>Blits a bitmap with one colour knocked out.</summary>
    TransparentBlt = 116,

    /// <summary>Fills rectangles or triangles with interpolated colours.</summary>
    GradientFill = 118,
}

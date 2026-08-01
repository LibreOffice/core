using System.Buffers.Binary;

namespace Paperless.Vector.Tests;

/// <summary>
/// Builds Windows Metafiles a byte at a time, for tests that need one exact record.
/// </summary>
/// <remarks>
/// <para>
/// Hand-built rather than converted from a source document, for the same reason the SVG tests
/// hold their SVG as a string: a test that asserts on the mapping of a
/// <c>SetWindowExt(1000, 500)</c> has to be able to write exactly that record and nothing else.
/// A producer's output is a fixture for fidelity, not for a unit test — it contains a hundred
/// records and none of them is under the test's control.
/// </para>
/// <para>
/// The record size is in 16-bit words and includes the six-byte header, which is the one thing
/// about the format that is easy to get wrong and impossible to notice: a record one word too
/// long silently swallows the next record.
/// </para>
/// </remarks>
internal sealed class WmfBuilder
{
    private readonly List<byte> _records = [];

    /// <summary>The bounding rectangle a placeable header states, in logical units.</summary>
    public (short Left, short Top, short Right, short Bottom) Bounds { get; set; } = (0, 0, 1000, 1000);

    /// <summary>The placeable header's logical units per inch.</summary>
    public ushort UnitsPerInch { get; set; } = 1440;

    /// <summary>True to write a placeable header before the metafile header.</summary>
    public bool Placeable { get; set; } = true;

    /// <summary>Appends a record with 16-bit parameters.</summary>
    public WmfBuilder Record(WmfFunction function, params short[] parameters)
    {
        ArgumentNullException.ThrowIfNull(parameters);

        Span<byte> words = stackalloc byte[parameters.Length * 2];
        for (int i = 0; i < parameters.Length; i++)
        {
            BinaryPrimitives.WriteInt16LittleEndian(words[(i * 2)..], parameters[i]);
        }

        return Raw(function, words);
    }

    /// <summary>Appends a record with an arbitrary payload, padded to a whole word.</summary>
    public WmfBuilder Raw(WmfFunction function, ReadOnlySpan<byte> payload)
    {
        int length = payload.Length + (payload.Length & 1);
        uint size = (uint)(3 + (length / 2));

        byte[] header = new byte[6];
        BinaryPrimitives.WriteUInt32LittleEndian(header, size);
        BinaryPrimitives.WriteUInt16LittleEndian(header.AsSpan(4), (ushort)function);

        _records.AddRange(header);
        _records.AddRange(payload.ToArray());
        if (length != payload.Length) _records.Add(0);

        return this;
    }

    /// <summary>Appends a <c>SetWindowOrg</c>, whose parameters are y then x.</summary>
    public WmfBuilder WindowOrigin(short x, short y) => Record(WmfFunction.SetWindowOrg, y, x);

    /// <summary>Appends a <c>SetWindowExt</c>, whose parameters are height then width.</summary>
    public WmfBuilder WindowExtent(short width, short height) => Record(WmfFunction.SetWindowExt, height, width);

    /// <summary>Appends a <c>SetMapMode</c>.</summary>
    public WmfBuilder MapMode(short mode) => Record(WmfFunction.SetMapMode, mode);

    /// <summary>Appends a <c>Rectangle</c>, whose parameters are bottom, right, top, left.</summary>
    public WmfBuilder Rectangle(short left, short top, short right, short bottom)
        => Record(WmfFunction.Rectangle, bottom, right, top, left);

    /// <summary>Appends an <c>Ellipse</c>.</summary>
    public WmfBuilder Ellipse(short left, short top, short right, short bottom)
        => Record(WmfFunction.Ellipse, bottom, right, top, left);

    /// <summary>Appends a <c>CreateBrushIndirect</c> with a solid colour.</summary>
    public WmfBuilder SolidBrush(byte r, byte g, byte b)
        => Record(WmfFunction.CreateBrushIndirect, 0, (short)(r | (g << 8)), b, 0);

    /// <summary>Appends a <c>CreateBrushIndirect</c> that fills nothing.</summary>
    public WmfBuilder NullBrush() => Record(WmfFunction.CreateBrushIndirect, 1, 0, 0, 0);

    /// <summary>Appends a <c>CreatePenIndirect</c>.</summary>
    public WmfBuilder Pen(short style, short width, byte r, byte g, byte b)
        => Record(WmfFunction.CreatePenIndirect, style, width, 0, (short)(r | (g << 8)), b);

    /// <summary>Appends a <c>SelectObject</c>.</summary>
    public WmfBuilder Select(short handle) => Record(WmfFunction.SelectObject, handle);

    /// <summary>Appends a <c>DeleteObject</c>.</summary>
    public WmfBuilder Delete(short handle) => Record(WmfFunction.DeleteObject, handle);

    /// <summary>Appends a <c>Polygon</c>.</summary>
    public WmfBuilder Polygon(params (short X, short Y)[] points)
    {
        ArgumentNullException.ThrowIfNull(points);

        List<short> parameters = [(short)points.Length];
        foreach ((short x, short y) in points)
        {
            parameters.Add(x);
            parameters.Add(y);
        }

        return Record(WmfFunction.Polygon, [.. parameters]);
    }

    /// <summary>Appends a <c>Polyline</c>.</summary>
    public WmfBuilder Polyline(params (short X, short Y)[] points)
    {
        ArgumentNullException.ThrowIfNull(points);

        List<short> parameters = [(short)points.Length];
        foreach ((short x, short y) in points)
        {
            parameters.Add(x);
            parameters.Add(y);
        }

        return Record(WmfFunction.Polyline, [.. parameters]);
    }

    /// <summary>Appends a <c>CreateFontIndirect</c> naming a family and a height.</summary>
    public WmfBuilder Font(string family, short height, short escapement = 0, short weight = 400)
    {
        ArgumentNullException.ThrowIfNull(family);

        byte[] payload = new byte[18 + 32];
        BinaryPrimitives.WriteInt16LittleEndian(payload, height);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(2), 0);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(4), escapement);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(6), escapement);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(8), weight);

        for (int i = 0; i < family.Length && i < 31; i++) payload[18 + i] = (byte)family[i];

        return Raw(WmfFunction.CreateFontIndirect, payload);
    }

    /// <summary>Appends a <c>TextOut</c>.</summary>
    public WmfBuilder TextOut(string text, short x, short y)
    {
        ArgumentNullException.ThrowIfNull(text);

        int stored = (text.Length + 1) & ~1;
        byte[] payload = new byte[2 + stored + 4];
        BinaryPrimitives.WriteInt16LittleEndian(payload, (short)text.Length);
        for (int i = 0; i < text.Length; i++) payload[2 + i] = (byte)text[i];
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(2 + stored), y);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(4 + stored), x);

        return Raw(WmfFunction.TextOut, payload);
    }

    /// <summary>The finished file, with its end-of-file record.</summary>
    public byte[] Build()
    {
        Record(WmfFunction.Eof);

        List<byte> file = [];

        if (Placeable)
        {
            byte[] header = new byte[22];
            BinaryPrimitives.WriteUInt32LittleEndian(header, 0x9AC6CDD7);
            BinaryPrimitives.WriteInt16LittleEndian(header.AsSpan(6), Bounds.Left);
            BinaryPrimitives.WriteInt16LittleEndian(header.AsSpan(8), Bounds.Top);
            BinaryPrimitives.WriteInt16LittleEndian(header.AsSpan(10), Bounds.Right);
            BinaryPrimitives.WriteInt16LittleEndian(header.AsSpan(12), Bounds.Bottom);
            BinaryPrimitives.WriteUInt16LittleEndian(header.AsSpan(14), UnitsPerInch);
            file.AddRange(header);
        }

        byte[] metaHeader = new byte[18];
        BinaryPrimitives.WriteUInt16LittleEndian(metaHeader, 1);          // memory metafile
        BinaryPrimitives.WriteUInt16LittleEndian(metaHeader.AsSpan(2), 9);
        BinaryPrimitives.WriteUInt16LittleEndian(metaHeader.AsSpan(4), 0x0300);
        BinaryPrimitives.WriteUInt32LittleEndian(metaHeader.AsSpan(6), (uint)((18 + _records.Count) / 2));
        BinaryPrimitives.WriteUInt16LittleEndian(metaHeader.AsSpan(10), 16);
        BinaryPrimitives.WriteUInt32LittleEndian(metaHeader.AsSpan(12), 64);

        file.AddRange(metaHeader);
        file.AddRange(_records);

        return [.. file];
    }
}

/// <summary>The record functions the builder can write.</summary>
internal enum WmfFunction : ushort
{
    Eof = 0x0000,
    SetBkMode = 0x0102,
    SetMapMode = 0x0103,
    SetTextColour = 0x0209,
    SetWindowOrg = 0x020B,
    SetWindowExt = 0x020C,
    LineTo = 0x0213,
    MoveTo = 0x0214,
    ExcludeClipRect = 0x0415,
    IntersectClipRect = 0x0416,
    Arc = 0x0817,
    Ellipse = 0x0418,
    Pie = 0x081A,
    Rectangle = 0x041B,
    RoundRect = 0x061C,
    SaveDc = 0x001E,
    TextOut = 0x0521,
    Polygon = 0x0324,
    Polyline = 0x0325,
    Escape = 0x0626,
    RestoreDc = 0x0127,
    SelectObject = 0x012D,
    SetTextAlign = 0x012E,
    Chord = 0x0830,
    ExtTextOut = 0x0A32,
    PolyPolygon = 0x0538,
    StretchDib = 0x0F43,
    DeleteObject = 0x01F0,
    CreatePenIndirect = 0x02FA,
    CreateFontIndirect = 0x02FB,
    CreateBrushIndirect = 0x02FC,
    CreateRegion = 0x06FF,
}

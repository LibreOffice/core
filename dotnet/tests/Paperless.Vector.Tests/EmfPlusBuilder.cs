using System.Buffers.Binary;

namespace Paperless.Vector.Tests;

/// <summary>
/// Builds the EMF+ record stream that rides inside an EMF's comment records.
/// </summary>
/// <remarks>
/// <para>
/// The counterpart of <see cref="EmfBuilder"/>, and needed for the same reason: a test that
/// asserts on the compressed point encoding has to be able to write exactly that record. It is a
/// separate class because an EMF+ record's framing shares nothing with an EMF record's — twelve
/// bytes against eight, a flags word the record's meaning is carried in, and a data size as well
/// as a size.
/// </para>
/// <para>
/// The finished stream goes into an <see cref="EmfBuilder"/> through
/// <see cref="EmfBuilderExtensions.Plus"/>, so a fixture is an ordinary EMF that happens to carry
/// one — which is what an EMF+ file is.
/// </para>
/// </remarks>
internal sealed class EmfPlusBuilder
{
    private readonly List<byte> _records = [];

    /// <summary>Appends a record with a raw payload.</summary>
    /// <param name="type">The record type.</param>
    /// <param name="flags">The flags word, which carries the object slot and the record's options.</param>
    /// <param name="data">The payload after the twelve-byte header.</param>
    public EmfPlusBuilder Record(ushort type, ushort flags, ReadOnlySpan<byte> data)
    {
        int padding = (4 - (data.Length & 3)) & 3;

        byte[] header = new byte[12];
        BinaryPrimitives.WriteUInt16LittleEndian(header, type);
        BinaryPrimitives.WriteUInt16LittleEndian(header.AsSpan(2), flags);
        BinaryPrimitives.WriteUInt32LittleEndian(header.AsSpan(4), (uint)(12 + data.Length + padding));
        BinaryPrimitives.WriteUInt32LittleEndian(header.AsSpan(8), (uint)data.Length);

        _records.AddRange(header);
        _records.AddRange(data.ToArray());
        for (int i = 0; i < padding; i++) _records.Add(0);

        return this;
    }

    /// <summary>Appends bytes verbatim, for a test that needs a malformed record.</summary>
    /// <param name="bytes">The bytes.</param>
    public EmfPlusBuilder Raw(ReadOnlySpan<byte> bytes)
    {
        _records.AddRange(bytes.ToArray());
        return this;
    }

    /// <summary>Appends an <c>EmfPlusHeader</c>.</summary>
    /// <param name="dual">True to say the file also carries a GDI description of the same drawing.</param>
    /// <param name="dpi">The resolution the picture was recorded at.</param>
    public EmfPlusBuilder Header(bool dual = false, int dpi = 96)
    {
        byte[] data = new byte[16];
        BinaryPrimitives.WriteUInt32LittleEndian(data, 0xDBC01002);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(4), 1);
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(8), dpi);
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(12), dpi);

        return Record(0x4001, (ushort)(dual ? 1 : 0), data);
    }

    /// <summary>Appends an <c>EmfPlusGetDC</c>, which hands the device context back to GDI.</summary>
    public EmfPlusBuilder GetDc() => Record(0x4004, 0, []);

    /// <summary>Appends an <c>EmfPlusEndOfFile</c>.</summary>
    public EmfPlusBuilder End() => Record(0x4002, 0, []);

    /// <summary>Appends a solid brush object.</summary>
    /// <param name="slot">Which of the 256 object slots.</param>
    /// <param name="argb">The colour, alpha in the top byte.</param>
    public EmfPlusBuilder SolidBrush(int slot, uint argb)
    {
        byte[] data = new byte[12];
        BinaryPrimitives.WriteUInt32LittleEndian(data, 0xDBC01002);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(4), 0);        // solid
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(8), argb);

        return Object(slot, 0x100, data);
    }

    /// <summary>Appends a hatch brush object.</summary>
    /// <param name="slot">Which object slot.</param>
    /// <param name="style">Which of the fifty-three hatch styles.</param>
    /// <param name="foreground">The lines' colour.</param>
    /// <param name="background">The colour behind them.</param>
    public EmfPlusBuilder HatchBrush(int slot, int style, uint foreground, uint background)
    {
        byte[] data = new byte[20];
        BinaryPrimitives.WriteUInt32LittleEndian(data, 0xDBC01002);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(4), 1);        // hatch
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(8), style);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(12), foreground);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(16), background);

        return Object(slot, 0x100, data);
    }

    /// <summary>Appends a linear gradient brush object.</summary>
    /// <param name="slot">Which object slot.</param>
    /// <param name="rect">The gradient's extent, in world units.</param>
    /// <param name="from">The colour at the start.</param>
    /// <param name="to">The colour at the end.</param>
    public EmfPlusBuilder LinearBrush(
        int slot, (float X, float Y, float Width, float Height) rect, uint from, uint to)
    {
        byte[] data = new byte[48];
        BinaryPrimitives.WriteUInt32LittleEndian(data, 0xDBC01002);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(4), 4);        // linear gradient
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(8), 0);        // no optional data
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(12), 0);        // wrap mode
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(16), rect.X);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(20), rect.Y);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(24), rect.Width);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(28), rect.Height);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(32), from);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(36), to);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(40), from);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(44), to);

        return Object(slot, 0x100, data);
    }

    /// <summary>
    /// Appends a pen object, with whichever optional fields the flags ask for.
    /// </summary>
    /// <param name="slot">Which object slot.</param>
    /// <param name="argb">The pen's colour.</param>
    /// <param name="width">The pen's width, in the stated unit.</param>
    /// <param name="unit">The unit the width is in; 0 is world and 2 is pixels.</param>
    /// <param name="join">A line join, or null to leave the field out.</param>
    /// <param name="dashes">A custom dash array, or null to leave the field out.</param>
    /// <param name="startCap">A start cap, or null to leave the field out.</param>
    public EmfPlusBuilder Pen(
        int slot,
        uint argb,
        float width,
        int unit = 0,
        int? join = null,
        float[]? dashes = null,
        int? startCap = null)
    {
        List<byte> data = [];

        uint penData = 0;
        if (startCap is not null) penData |= 0x0002;
        if (join is not null) penData |= 0x0008;
        if (dashes is not null) penData |= 0x0100;

        Add32(data, 0xDBC01002);
        Add32(data, 0);                                                     // pen type
        Add32(data, penData);
        Add32(data, (uint)unit);
        AddSingle(data, width);

        if (startCap is { } cap) Add32(data, (uint)cap);
        if (join is { } line) Add32(data, (uint)line);

        if (dashes is not null)
        {
            Add32(data, (uint)dashes.Length);
            foreach (float dash in dashes) AddSingle(data, dash);
        }

        // Every pen ends with a brush, and it is the field a misread optional field moves.
        Add32(data, 0xDBC01002);
        Add32(data, 0);
        Add32(data, argb);

        return Object(slot, 0x200, [.. data]);
    }

    /// <summary>Appends a path object.</summary>
    /// <param name="slot">Which object slot.</param>
    /// <param name="points">The points, in world units.</param>
    /// <param name="types">One point type a point: 0 starts a figure, 1 a line, 3 a Bézier, 0x80 closes.</param>
    public EmfPlusBuilder Path(int slot, (float X, float Y)[] points, byte[] types)
    {
        ArgumentNullException.ThrowIfNull(points);
        ArgumentNullException.ThrowIfNull(types);

        List<byte> data = [];
        Add32(data, 0xDBC01002);
        Add32(data, (uint)points.Length);
        Add32(data, 0);                                                     // uncompressed points

        foreach ((float x, float y) in points)
        {
            AddSingle(data, x);
            AddSingle(data, y);
        }

        data.AddRange(types);

        return Object(slot, 0x300, [.. data]);
    }

    /// <summary>Appends a font object.</summary>
    /// <param name="slot">Which object slot.</param>
    /// <param name="family">The family name.</param>
    /// <param name="size">The em size, in the stated unit.</param>
    /// <param name="unit">The unit; 0 is world and 2 is pixels.</param>
    /// <param name="style">The style bits: 1 bold, 2 italic, 4 underline, 8 strikeout.</param>
    public EmfPlusBuilder Font(int slot, string family, float size, int unit = 0, int style = 0)
    {
        ArgumentNullException.ThrowIfNull(family);

        List<byte> data = [];
        Add32(data, 0xDBC01002);
        AddSingle(data, size);
        Add32(data, (uint)unit);
        Add32(data, (uint)style);
        Add32(data, 0);                                                     // reserved
        Add32(data, (uint)family.Length);

        foreach (char character in family)
        {
            data.Add((byte)character);
            data.Add((byte)(character >> 8));
        }

        return Object(slot, 0x600, [.. data]);
    }

    /// <summary>Appends a string format object.</summary>
    /// <param name="slot">Which object slot.</param>
    /// <param name="alignment">0 near, 1 centre, 2 far.</param>
    /// <param name="lineAlignment">The same, vertically.</param>
    /// <param name="tracking">A multiplier on every character's advance.</param>
    public EmfPlusBuilder StringFormat(int slot, int alignment, int lineAlignment = 0, float tracking = 1f)
    {
        List<byte> data = [];
        Add32(data, 0xDBC01002);
        Add32(data, 0);                                                     // flags
        Add32(data, 0);                                                     // language
        Add32(data, (uint)alignment);
        Add32(data, (uint)lineAlignment);
        Add32(data, 0);                                                     // digit substitution
        Add32(data, 0);                                                     // digit language
        AddSingle(data, 0);                                                 // first tab offset
        Add32(data, 0);                                                     // hotkey prefix
        AddSingle(data, 0);                                                 // leading margin
        AddSingle(data, 0);                                                 // trailing margin
        AddSingle(data, tracking);
        Add32(data, 0);                                                     // trimming
        Add32(data, 0);                                                     // tab stops
        Add32(data, 0);                                                     // ranges

        return Object(slot, 0x700, [.. data]);
    }

    /// <summary>Appends an object record, or several when it is too large for one.</summary>
    /// <param name="slot">Which object slot.</param>
    /// <param name="type">The object type, shifted into the flags word's middle byte.</param>
    /// <param name="data">The object's bytes.</param>
    /// <param name="split">
    /// How many bytes to put in each part, or zero for one record. Non-zero exercises the
    /// continuation form, which is how a texture brush carrying a photograph is written.
    /// </param>
    public EmfPlusBuilder Object(int slot, int type, ReadOnlySpan<byte> data, int split = 0)
    {
        ushort flags = (ushort)((slot & 0xFF) | type);

        if (split <= 0) return Record(0x4008, flags, data);

        int at = 0;

        while (at < data.Length)
        {
            int take = Math.Min(split, data.Length - at);

            byte[] part = new byte[4 + take];
            BinaryPrimitives.WriteUInt32LittleEndian(part, (uint)data.Length);
            data.Slice(at, take).CopyTo(part.AsSpan(4));

            Record(0x4008, (ushort)(flags | 0x8000), part);
            at += take;
        }

        return this;
    }

    /// <summary>Appends a <c>FillRects</c> naming a colour outright.</summary>
    /// <param name="argb">The colour.</param>
    /// <param name="rectangles">The rectangles, in world units.</param>
    public EmfPlusBuilder FillRects(uint argb, params (float X, float Y, float Width, float Height)[] rectangles)
        => Rects(0x400A, 0x8000, argb, rectangles);

    /// <summary>Appends a <c>FillRects</c> naming a brush by slot.</summary>
    /// <param name="slot">Which brush.</param>
    /// <param name="rectangles">The rectangles.</param>
    public EmfPlusBuilder FillRectsWithBrush(
        int slot, params (float X, float Y, float Width, float Height)[] rectangles)
        => Rects(0x400A, 0, (uint)slot, rectangles);

    /// <summary>Appends a <c>DrawRects</c>.</summary>
    /// <param name="pen">Which pen.</param>
    /// <param name="rectangles">The rectangles.</param>
    public EmfPlusBuilder DrawRects(int pen, params (float X, float Y, float Width, float Height)[] rectangles)
    {
        ArgumentNullException.ThrowIfNull(rectangles);

        List<byte> data = [];
        Add32(data, (uint)rectangles.Length);
        foreach ((float x, float y, float w, float h) in rectangles)
        {
            AddSingle(data, x);
            AddSingle(data, y);
            AddSingle(data, w);
            AddSingle(data, h);
        }

        return Record(0x400B, (ushort)(pen & 0xFF), [.. data]);
    }

    private EmfPlusBuilder Rects(
        ushort type, ushort flags, uint brushOrColour, (float X, float Y, float Width, float Height)[] rectangles)
    {
        ArgumentNullException.ThrowIfNull(rectangles);

        List<byte> data = [];
        Add32(data, brushOrColour);
        Add32(data, (uint)rectangles.Length);

        foreach ((float x, float y, float w, float h) in rectangles)
        {
            AddSingle(data, x);
            AddSingle(data, y);
            AddSingle(data, w);
            AddSingle(data, h);
        }

        return Record(type, flags, [.. data]);
    }

    /// <summary>Appends a <c>FillEllipse</c> naming a colour.</summary>
    /// <param name="argb">The colour.</param>
    /// <param name="rect">The rectangle the ellipse is inscribed in.</param>
    public EmfPlusBuilder FillEllipse(uint argb, (float X, float Y, float Width, float Height) rect)
    {
        List<byte> data = [];
        Add32(data, argb);
        AddSingle(data, rect.X);
        AddSingle(data, rect.Y);
        AddSingle(data, rect.Width);
        AddSingle(data, rect.Height);

        return Record(0x400E, 0x8000, [.. data]);
    }

    /// <summary>Appends a <c>FillPie</c> naming a colour.</summary>
    /// <param name="argb">The colour.</param>
    /// <param name="rect">The rectangle the ellipse is inscribed in.</param>
    /// <param name="start">The start angle in degrees, clockwise from the x axis.</param>
    /// <param name="sweep">How far it sweeps, in degrees.</param>
    public EmfPlusBuilder FillPie(
        uint argb, (float X, float Y, float Width, float Height) rect, float start, float sweep)
    {
        List<byte> data = [];
        Add32(data, argb);
        AddSingle(data, start);
        AddSingle(data, sweep);
        AddSingle(data, rect.X);
        AddSingle(data, rect.Y);
        AddSingle(data, rect.Width);
        AddSingle(data, rect.Height);

        return Record(0x4010, 0x8000, [.. data]);
    }

    /// <summary>Appends a <c>FillPath</c>.</summary>
    /// <param name="slot">Which path.</param>
    /// <param name="argb">The colour.</param>
    /// <param name="winding">True for the non-zero rule.</param>
    public EmfPlusBuilder FillPath(int slot, uint argb, bool winding = false)
    {
        byte[] data = new byte[4];
        BinaryPrimitives.WriteUInt32LittleEndian(data, argb);

        return Record(0x4014, (ushort)((slot & 0xFF) | 0x8000 | (winding ? 0x2000 : 0)), data);
    }

    /// <summary>Appends a <c>DrawPath</c>.</summary>
    /// <param name="slot">Which path.</param>
    /// <param name="pen">Which pen.</param>
    public EmfPlusBuilder DrawPath(int slot, int pen)
    {
        byte[] data = new byte[4];
        BinaryPrimitives.WriteUInt32LittleEndian(data, (uint)pen);

        return Record(0x4015, (ushort)(slot & 0xFF), data);
    }

    /// <summary>
    /// Appends a <c>DrawLines</c>, in whichever of the three point encodings is asked for.
    /// </summary>
    /// <param name="pen">Which pen.</param>
    /// <param name="points">The points, in world units.</param>
    /// <param name="encoding">
    /// 0 for floats, 0x4000 for compressed 16-bit points, 0x800 for relative points.
    /// </param>
    /// <param name="close">True to draw a line from the last point back to the first.</param>
    public EmfPlusBuilder DrawLines(
        int pen, (float X, float Y)[] points, ushort encoding = 0, bool close = false)
    {
        ArgumentNullException.ThrowIfNull(points);

        List<byte> data = [];
        Add32(data, (uint)points.Length);
        AddPoints(data, points, encoding);

        return Record(0x400D, (ushort)((pen & 0xFF) | encoding | (close ? 0x2000 : 0)), [.. data]);
    }

    /// <summary>Appends a <c>FillPolygon</c>.</summary>
    /// <param name="argb">The colour.</param>
    /// <param name="points">The polygon's points.</param>
    public EmfPlusBuilder FillPolygon(uint argb, params (float X, float Y)[] points)
    {
        ArgumentNullException.ThrowIfNull(points);

        List<byte> data = [];
        Add32(data, argb);
        Add32(data, (uint)points.Length);
        AddPoints(data, points, 0);

        return Record(0x400C, 0x8000, [.. data]);
    }

    /// <summary>Appends a <c>DrawString</c>.</summary>
    /// <param name="font">Which font.</param>
    /// <param name="argb">The colour.</param>
    /// <param name="format">Which string format, or 0xFF for none.</param>
    /// <param name="text">The text.</param>
    /// <param name="rect">The layout rectangle, in world units.</param>
    public EmfPlusBuilder DrawString(
        int font, uint argb, int format, string text, (float X, float Y, float Width, float Height) rect)
    {
        ArgumentNullException.ThrowIfNull(text);

        List<byte> data = [];
        Add32(data, argb);
        Add32(data, (uint)format);
        Add32(data, (uint)text.Length);
        AddSingle(data, rect.X);
        AddSingle(data, rect.Y);
        AddSingle(data, rect.Width);
        AddSingle(data, rect.Height);

        foreach (char character in text)
        {
            data.Add((byte)character);
            data.Add((byte)(character >> 8));
        }

        return Record(0x401C, (ushort)((font & 0xFF) | 0x8000), [.. data]);
    }

    /// <summary>Appends a <c>SetWorldTransform</c>.</summary>
    /// <param name="m11">The x scale.</param>
    /// <param name="m12">The y shear.</param>
    /// <param name="m21">The x shear.</param>
    /// <param name="m22">The y scale.</param>
    /// <param name="dx">The x translation.</param>
    /// <param name="dy">The y translation.</param>
    public EmfPlusBuilder World(float m11, float m12, float m21, float m22, float dx, float dy)
    {
        List<byte> data = [];
        AddSingle(data, m11);
        AddSingle(data, m12);
        AddSingle(data, m21);
        AddSingle(data, m22);
        AddSingle(data, dx);
        AddSingle(data, dy);

        return Record(0x402A, 0, [.. data]);
    }

    /// <summary>Appends a <c>ScaleWorldTransform</c>.</summary>
    /// <param name="x">The x factor.</param>
    /// <param name="y">The y factor.</param>
    /// <param name="post">True to apply the scale after the world transform rather than before.</param>
    public EmfPlusBuilder Scale(float x, float y, bool post = false)
    {
        List<byte> data = [];
        AddSingle(data, x);
        AddSingle(data, y);

        return Record(0x402E, (ushort)(post ? 0x2000 : 0), [.. data]);
    }

    /// <summary>Appends a <c>TranslateWorldTransform</c>.</summary>
    /// <param name="x">The x offset.</param>
    /// <param name="y">The y offset.</param>
    /// <param name="post">True to apply the translation after the world transform.</param>
    public EmfPlusBuilder Translate(float x, float y, bool post = false)
    {
        List<byte> data = [];
        AddSingle(data, x);
        AddSingle(data, y);

        return Record(0x402D, (ushort)(post ? 0x2000 : 0), [.. data]);
    }

    /// <summary>Appends a <c>ResetWorldTransform</c>.</summary>
    public EmfPlusBuilder ResetWorld() => Record(0x402B, 0, []);

    /// <summary>Appends a <c>SetPageTransform</c>.</summary>
    /// <param name="scale">The page scale.</param>
    /// <param name="unit">The page unit.</param>
    public EmfPlusBuilder PageTransform(float scale, int unit)
    {
        List<byte> data = [];
        AddSingle(data, scale);

        return Record(0x4030, (ushort)unit, [.. data]);
    }

    /// <summary>Appends a <c>Save</c>.</summary>
    /// <param name="index">The index the state is filed under.</param>
    public EmfPlusBuilder Save(int index) => Indexed(0x4025, index);

    /// <summary>Appends a <c>Restore</c>.</summary>
    /// <param name="index">Which saved state.</param>
    public EmfPlusBuilder Restore(int index) => Indexed(0x4026, index);

    /// <summary>Appends a <c>BeginContainerNoParams</c>.</summary>
    /// <param name="index">The index the state is filed under.</param>
    public EmfPlusBuilder BeginContainer(int index) => Indexed(0x4028, index);

    /// <summary>Appends an <c>EndContainer</c>.</summary>
    /// <param name="index">Which container.</param>
    public EmfPlusBuilder EndContainer(int index) => Indexed(0x4029, index);

    private EmfPlusBuilder Indexed(ushort type, int index)
    {
        byte[] data = new byte[4];
        BinaryPrimitives.WriteInt32LittleEndian(data, index);
        return Record(type, 0, data);
    }

    /// <summary>Appends a <c>SetClipRect</c>.</summary>
    /// <param name="rect">The rectangle, in world units.</param>
    /// <param name="mode">The combine mode: 0 replace, 1 intersect, 4 exclude.</param>
    public EmfPlusBuilder ClipRect((float X, float Y, float Width, float Height) rect, int mode = 0)
    {
        List<byte> data = [];
        AddSingle(data, rect.X);
        AddSingle(data, rect.Y);
        AddSingle(data, rect.Width);
        AddSingle(data, rect.Height);

        return Record(0x4032, (ushort)((mode & 0x0F) << 8), [.. data]);
    }

    /// <summary>Appends a <c>ResetClip</c>.</summary>
    public EmfPlusBuilder ResetClip() => Record(0x4031, 0, []);

    /// <summary>The stream as an <c>EMR_COMMENT</c> payload.</summary>
    public byte[] Comment()
    {
        byte[] payload = new byte[8 + _records.Count];

        BinaryPrimitives.WriteUInt32LittleEndian(payload, (uint)(4 + _records.Count));
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(4), 0x2B464D45);
        _records.CopyTo(payload.AsSpan(8));

        return payload;
    }

    /// <summary>The record bytes, without the comment wrapper.</summary>
    public byte[] Bytes() => [.. _records];

    private static void AddPoints(List<byte> data, (float X, float Y)[] points, ushort encoding)
    {
        if ((encoding & 0x800) != 0)
        {
            // Relative points: each is a delta from the one before, in one or two bytes.
            float px = 0;
            float py = 0;

            foreach ((float x, float y) in points)
            {
                AddInteger(data, (int)Math.Round(x - px));
                AddInteger(data, (int)Math.Round(y - py));
                px = x;
                py = y;
            }

            return;
        }

        foreach ((float x, float y) in points)
        {
            if ((encoding & 0x4000) != 0)
            {
                data.AddRange(BitConverter.GetBytes((short)x));
                data.AddRange(BitConverter.GetBytes((short)y));
                continue;
            }

            AddSingle(data, x);
            AddSingle(data, y);
        }
    }

    /// <summary>Writes an <c>EmfPlusInteger7</c> or <c>EmfPlusInteger15</c>.</summary>
    private static void AddInteger(List<byte> data, int value)
    {
        if (value is >= -64 and <= 63)
        {
            data.Add((byte)(value & 0x7F));
            return;
        }

        int clamped = Math.Clamp(value, -16384, 16383);
        data.Add((byte)(0x80 | ((clamped >> 8) & 0x7F)));
        data.Add((byte)(clamped & 0xFF));
    }

    private static void Add32(List<byte> data, uint value)
    {
        Span<byte> at = stackalloc byte[4];
        BinaryPrimitives.WriteUInt32LittleEndian(at, value);
        data.AddRange(at);
    }

    private static void AddSingle(List<byte> data, float value)
    {
        Span<byte> at = stackalloc byte[4];
        BinaryPrimitives.WriteSingleLittleEndian(at, value);
        data.AddRange(at);
    }
}

/// <summary>Puts an EMF+ stream inside an EMF.</summary>
internal static class EmfBuilderExtensions
{
    /// <summary>Appends an <c>EMR_COMMENT</c> carrying an EMF+ record stream.</summary>
    /// <param name="builder">The enclosing metafile.</param>
    /// <param name="plus">The EMF+ records.</param>
    public static EmfBuilder Plus(this EmfBuilder builder, EmfPlusBuilder plus)
    {
        ArgumentNullException.ThrowIfNull(builder);
        ArgumentNullException.ThrowIfNull(plus);

        return builder.Raw(EmfFunction.Comment, plus.Comment());
    }
}

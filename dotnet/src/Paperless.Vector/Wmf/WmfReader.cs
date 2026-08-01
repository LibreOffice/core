using System.Buffers.Binary;
using System.Text;
using Paperless.Core.Diagnostics;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector.Metafiles;

namespace Paperless.Vector.Wmf;

/// <summary>
/// Replays a Windows Metafile's records into a display list.
/// </summary>
/// <remarks>
/// <para>
/// Ported record by record from <c>emfio/source/reader/wmfreader.cxx</c>, which is the working
/// reference rather than [MS-WMF]: the specification describes the format and not the twenty
/// years of producer bugs the format is actually full of. Where a decision here looks odd, the
/// citation beside it says which file it came from.
/// </para>
/// <para>
/// Everything stateful — the mapping, the clip, the selected objects — lives in
/// <see cref="MetafileDeviceContext"/> and is shared with the EMF reader still to come. What is
/// specific to WMF is the record layout, the two headers, and the size rule.
/// </para>
/// </remarks>
internal sealed class WmfReader
{
    /// <summary>The signature of a placeable header, <c>D7 CD C6 9A</c> little-endian.</summary>
    public const uint PlaceableKey = 0x9AC6CDD7;

    /// <summary>
    /// The width, in logical units, past which a header-less WMF is scaled down.
    /// </summary>
    /// <remarks>
    /// A bare metafile states no physical size at all, so one has to be invented from the
    /// drawing's own extent. LibreOffice invents 96 units to the inch and then shrinks anything
    /// wider than this (<c>wmfreader.cxx:1530</c>, <c>1613-1620</c>), which keeps a
    /// 30 000-unit-wide drawing from claiming to be a 26-foot picture.
    /// </remarks>
    private const int MaxImpliedWidth = 1024;

    /// <summary>The comment escape that carries a chunk of an embedded EMF: "WMFC".</summary>
    private const uint EmfCommentMagic = 0x43464D57;

    /// <summary>The comment escape that carries Unicode text: the first half of its magic.</summary>
    private const uint UnicodeEscapeMagic = 0x2C2A4F4F;

    private const ushort MetafileComment = 15;
    private const ushort PrivateEscapeUnicode = 2;

    private readonly byte[] _bytes;
    private readonly List<Diagnostic> _diagnostics;
    private readonly HashSet<string> _reported = [];
    private readonly MetafileDeviceContext _context = new();
    private readonly MetafileBudget _budget;
    private readonly MetafileTextEngine _text;
    private readonly DisplayList _list = new();
    private readonly MetafilePainter _painter;
    private readonly List<byte> _embeddedEmf = [];

    private int _position;
    private bool _failed;
    private int _unitsPerInch = 96;
    private int _skipRecords;
    private int _emfChunks;
    private uint _emfChunkTotal;

    /// <summary>Creates a reader over a metafile's bytes.</summary>
    /// <param name="bytes">The whole file.</param>
    /// <param name="limits">The work caps.</param>
    /// <param name="diagnostics">Where problems are recorded.</param>
    /// <param name="text">The text engine, shared so its font cache is.</param>
    public WmfReader(byte[] bytes, VectorLimits limits, List<Diagnostic> diagnostics, MetafileTextEngine text)
    {
        _bytes = bytes;
        _diagnostics = diagnostics;
        _text = text;
        _budget = new MetafileBudget(limits);
        _painter = new MetafilePainter(_list, _context, _budget);
    }

    /// <summary>The recorded picture.</summary>
    public DisplayList Content => _list;

    /// <summary>True when a limit stopped the replay early.</summary>
    public bool IsTruncated => _budget.IsExhausted;

    /// <summary>True when the file was placeable, so its physical size was stated rather than guessed.</summary>
    public bool IsPlaceable { get; private set; }

    /// <summary>
    /// The EMF an <c>ESCAPE</c> record carried, when the file hid one.
    /// </summary>
    /// <remarks>
    /// Reported rather than replayed. A dual-format WMF holds a complete EMF spread across
    /// comment escapes so that a consumer which understands EMF can use the better rendering;
    /// replaying both would draw everything twice. Until the EMF reader exists, the WMF records
    /// are what get drawn and this says what was left on the table.
    /// </remarks>
    public IReadOnlyList<byte> EmbeddedEmf => _embeddedEmf;

    /// <summary>The picture's physical extent, which is also its coordinate space.</summary>
    /// <remarks>
    /// <para>
    /// <b>The view box and the intrinsic size coincide for WMF, and that is a fact about WMF
    /// rather than a simplification.</b> Every point is mapped into 1/100 mm as it is read — the
    /// display list holds no logical coordinates at all — so the coordinate space the commands
    /// are in <em>is</em> the physical one. EMF will differ, because its <c>rclBounds</c> is in
    /// device units and its <c>rclFrame</c> in 1/100 mm, and the two are stated independently.
    /// </para>
    /// <para>
    /// The size is the viewport extent, which is what LibreOffice uses for a WMF's
    /// <c>PrefSize</c> when there is no frame rectangle (<c>~MtfTools</c>,
    /// <c>mtftools.cxx:1244-1252</c>).
    /// </para>
    /// </remarks>
    public DocSize Extent { get; private set; }

    /// <summary>True when the bytes begin like a WMF.</summary>
    /// <remarks>
    /// Two shapes to recognise: a placeable header's magic, or a bare metafile header whose
    /// first two words are the type and a header size of 9 words. The second is only three bytes
    /// of signal, so it is checked together with the record-count and version fields that follow
    /// — a bare WMF is otherwise indistinguishable from a great many other things.
    /// </remarks>
    public static bool Looks(ReadOnlySpan<byte> data)
    {
        if (data.Length < 18) return false;

        if (BinaryPrimitives.ReadUInt32LittleEndian(data) == PlaceableKey) return true;

        ushort type = BinaryPrimitives.ReadUInt16LittleEndian(data);
        ushort headerWords = BinaryPrimitives.ReadUInt16LittleEndian(data[2..]);
        ushort version = BinaryPrimitives.ReadUInt16LittleEndian(data[4..]);
        uint sizeWords = BinaryPrimitives.ReadUInt32LittleEndian(data[6..]);

        // Type 1 is a memory metafile and type 2 a disk one; only 2 is ever written to a file,
        // but both turn up embedded. The version is 0x0100 or 0x0300. The size is in words and
        // must at least reach the header.
        return type is 1 or 2 && headerWords == 9 && version is 0x0100 or 0x0300 && sizeWords >= 9;
    }

    /// <summary>Reads the whole file. False when nothing could be drawn at all.</summary>
    public bool Read()
    {
        _context.Mapping.SetMode(MappingMode.Anisotropic);
        _context.Mapping.WindowOriginX = 0;
        _context.Mapping.WindowOriginY = 0;
        _context.Mapping.SetWindowExtent(1, 1);
        _context.Mapping.SetViewportExtent(10000, 10000);

        if (!ReadHeader())
        {
            Warn("PL6035", "A WMF's header could not be read; the picture was not drawn.");
            return false;
        }

        ReplayRecords();
        _painter.Finish();

        Extent = new DocSize(
            MetafileMapping.Emu(_context.Mapping.ViewportExtentX),
            MetafileMapping.Emu(_context.Mapping.ViewportExtentY));

        if (_budget.Truncation("WMF") is { } truncation) _diagnostics.Add(truncation);

        if (_context.Clip.HasUnsupportedOperation)
        {
            Warn(
                "PL6034",
                "A WMF excluded an area from its clip, which cannot be expressed; more of the "
                    + "picture may be visible than the file intended.");
        }

        if (_embeddedEmf.Count > 0)
        {
            Warn(
                "PL6030",
                $"A WMF carries an embedded EMF of {_embeddedEmf.Count} bytes in its escape records. "
                    + "The WMF records were drawn; the EMF rendering, which the producer expected a "
                    + "capable consumer to prefer, was not.");
        }

        return _list.Count > 0;
    }

    // ---------------------------------------------------------------- headers

    /// <summary>
    /// Reads the placeable header if there is one, then the metafile header, and settles the
    /// window and viewport the whole picture is mapped through.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>The placeable header's bounding rectangle is not used.</b> It is read, and then
    /// discarded in favour of what <see cref="Measure"/> finds by scanning the records — which is
    /// what <c>emfio</c> does too (<c>wmfreader.cxx:1580</c> overwrites it unconditionally). Real
    /// placeable headers disagree with their own contents often enough that the records are the
    /// more reliable source, and the header's <em>units per inch</em> field, which the records
    /// cannot supply, is the part that is kept.
    /// </para>
    /// <para>
    /// This is the trap the TODO warns about, in its WMF form: the logical coordinate space
    /// comes from the window records and the physical extent from the units-per-inch field, and
    /// they are independent. Deriving one from the other gives a picture that is the right shape
    /// and the wrong size.
    /// </para>
    /// </remarks>
    private bool ReadHeader()
    {
        uint key = U32();
        if (_failed) return false;

        IsPlaceable = key == PlaceableKey;
        Bounds bounds;

        if (IsPlaceable)
        {
            Skip(2);                        // HWmf, unused
            Skip(8);                        // the bounding rectangle, discarded — see the remarks
            _unitsPerInch = U16();
            Skip(4);                        // reserved
            Skip(2);                        // checksum, deliberately not verified

            // The records start after both headers: 22 bytes of placeable plus 18 of metafile.
            bounds = Measure(40);
        }
        else
        {
            _unitsPerInch = 96;
            bounds = Measure(18);

            if (bounds.Width > MaxImpliedWidth)
            {
                // Scaling by changing the assumed resolution rather than the extent, so that the
                // logical coordinates the records carry stay untouched.
                _unitsPerInch = (int)Math.Round(_unitsPerInch * (double)bounds.Width / MaxImpliedWidth);
            }
        }

        if (_unitsPerInch <= 0) _unitsPerInch = 96;

        // Undocumented, and load-bearing: in an anisotropic placeable metafile the units-per-inch
        // field is widely ignored by other office suites, and honouring it when it exceeds the
        // window makes the picture come out smaller than an inch square
        // (wmfreader.cxx:2142-2156).
        if (IsPlaceable
            && bounds.MapMode == MappingMode.Anisotropic
            && _unitsPerInch > bounds.Width
            && _unitsPerInch > bounds.Height)
        {
            _unitsPerInch = Math.Max(Math.Max(bounds.Width, bounds.Height), 1);
        }

        _context.Mapping.WindowOriginX = bounds.Left;
        _context.Mapping.WindowOriginY = bounds.Top;
        _context.Mapping.SetWindowExtent(Math.Abs(bounds.Width), Math.Abs(bounds.Height));

        int viewportX = 10000;
        int viewportY = 10000;

        if (Math.Abs(bounds.Width) > 1 && Math.Abs(bounds.Height) > 1)
        {
            viewportX = (int)Math.Round(Math.Abs(bounds.Width) * 2540.0 / _unitsPerInch);
            viewportY = (int)Math.Round(Math.Abs(bounds.Height) * 2540.0 / _unitsPerInch);
        }

        _context.Mapping.SetViewportExtent(Math.Max(viewportX, 1), Math.Max(viewportY, 1));

        Seek(IsPlaceable ? 22 : 0);

        uint metaKey = U32();
        if (_failed) return false;

        if (metaKey != 0x00090001)
        {
            ushort next = U16();

            // The other spelling seen in the wild: a type-1 memory metafile whose first DWORD
            // reads as 0x00010000 with the header size in the following word.
            if (metaKey != 0x00010000 || next != 0x0009) return false;
        }

        Skip(2);        // version
        Skip(4);        // size in words
        Skip(2);        // maximum simultaneous objects
        Skip(4);        // largest record, in words
        Skip(2);        // unused

        return !_failed;
    }

    /// <summary>The window a scan of the records implies.</summary>
    private readonly record struct Bounds(int Left, int Top, int Width, int Height, MappingMode MapMode);

    /// <summary>
    /// Scans the records for the window the picture is drawn in, without drawing anything.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Three sources in order of trust, from <c>WmfReader::GetPlaceableBound</c>
    /// (<c>wmfreader.cxx:2139-2184</c>): the window records if there are any, the viewport
    /// records otherwise, and failing both the extent of everything the file draws. The last is
    /// not a fallback for broken files — plenty of real metafiles state no window at all and are
    /// meant to be measured.
    /// </para>
    /// <para>
    /// The scan is charged against the same budget as the replay, because it reads the same
    /// records and a file that is expensive to replay is expensive to measure.
    /// </para>
    /// </remarks>
    private Bounds Measure(int start)
    {
        int savedPosition = _position;
        bool savedFailed = _failed;

        Seek(start);

        int left = int.MaxValue;
        int top = int.MaxValue;
        int right = int.MinValue;
        int bottom = int.MinValue;
        bool any = false;

        int windowX = 0;
        int windowY = 0;
        (int W, int H)? windowExtent = null;
        int viewportX = 0;
        int viewportY = 0;
        (int W, int H)? viewportExtent = null;
        MappingMode mode = MappingMode.Anisotropic;

        void Include(int x, int y)
        {
            // A y-up mapping mode is measured in the space the records are in, so the sign has to
            // be undone before the extent means anything (GetWinExtMax, wmfreader.cxx:132-145).
            if (mode == MappingMode.HiMetric) y = -y;

            left = Math.Min(left, x);
            top = Math.Min(top, y);
            right = Math.Max(right, x);
            bottom = Math.Max(bottom, y);
            any = true;
        }

        while (!_failed)
        {
            int recordStart = _position;
            uint size = U32();
            ushort function = U16();

            if (_failed || size < 3 || function == (ushort)WmfRecordType.Eof) break;
            if (!_budget.ChargeRecord()) break;

            long next = recordStart + ((long)size * 2);
            if (next > _bytes.Length) break;

            switch ((WmfRecordType)function)
            {
                case WmfRecordType.SetWindowOrg:
                    (windowX, windowY) = ReadYx();
                    break;

                case WmfRecordType.SetWindowExt:
                {
                    int h = I16();
                    int w = I16();
                    windowExtent = (w, h);
                    break;
                }

                case WmfRecordType.SetViewportOrg:
                    (viewportX, viewportY) = ReadYx();
                    break;

                case WmfRecordType.SetViewportExt:
                {
                    int h = I16();
                    int w = I16();
                    viewportExtent = (w, h);
                    break;
                }

                case WmfRecordType.SetMapMode:
                    mode = (MappingMode)U16();
                    break;

                case WmfRecordType.MoveTo:
                case WmfRecordType.LineTo:
                {
                    (int x, int y) = ReadYx();
                    Include(x, y);
                    break;
                }

                case WmfRecordType.Rectangle:
                case WmfRecordType.IntersectClipRect:
                case WmfRecordType.ExcludeClipRect:
                case WmfRecordType.Ellipse:
                {
                    (int l, int t, int r, int b) = ReadRect();
                    Include(l, t);
                    Include(r, b);
                    break;
                }

                case WmfRecordType.RoundRect:
                {
                    Skip(4);
                    (int l, int t, int r, int b) = ReadRect();
                    Include(l, t);
                    Include(r, b);
                    break;
                }

                case WmfRecordType.Arc:
                case WmfRecordType.Pie:
                case WmfRecordType.Chord:
                {
                    Skip(8);
                    (int l, int t, int r, int b) = ReadRect();
                    Include(l, t);
                    Include(r, b);
                    break;
                }

                case WmfRecordType.Polygon:
                case WmfRecordType.Polyline:
                {
                    int count = U16();
                    if (!MetafileBudget.Plausible(count, 4, next - _position)) break;
                    for (int i = 0; i < count && !_failed; i++)
                    {
                        int x = I16();
                        int y = I16();
                        Include(x, y);
                    }

                    break;
                }

                case WmfRecordType.PolyPolygon:
                {
                    int polygons = U16();
                    if (!MetafileBudget.Plausible(polygons, 2, next - _position)) break;

                    long points = 0;
                    for (int i = 0; i < polygons && !_failed; i++) points += U16();
                    if (!MetafileBudget.Plausible((int)Math.Min(points, int.MaxValue), 4, next - _position)) break;

                    for (long i = 0; i < points && !_failed; i++)
                    {
                        int x = I16();
                        int y = I16();
                        Include(x, y);
                    }

                    break;
                }

                case WmfRecordType.SetPixel:
                {
                    Skip(4);
                    (int x, int y) = ReadYx();
                    Include(x, y);
                    break;
                }

                case WmfRecordType.TextOut:
                {
                    int length = U16();
                    if (length > 0)
                    {
                        Skip((length + 1) & ~1);
                        (int x, int y) = ReadYx();
                        Include(x, y);
                    }

                    break;
                }

                case WmfRecordType.ExtTextOut:
                {
                    (int x, int y) = ReadYx();
                    int length = U16();
                    Skip(2);
                    if (length > 0) Include(x, y);
                    break;
                }

                case WmfRecordType.BitBlt:
                case WmfRecordType.StretchBlt:
                case WmfRecordType.DibBitBlt:
                case WmfRecordType.DibStretchBlt:
                case WmfRecordType.StretchDib:
                {
                    Skip(4);
                    if ((WmfRecordType)function == WmfRecordType.StretchDib) Skip(2);
                    if ((WmfRecordType)function is WmfRecordType.DibStretchBlt
                        or WmfRecordType.StretchBlt
                        or WmfRecordType.StretchDib)
                    {
                        Skip(4);
                    }

                    Skip(4);
                    if (NoSourceBitmap(function, size)) Skip(2);

                    int height = I16();
                    int width = I16();
                    if (width != 0 && height != 0)
                    {
                        (int x, int y) = ReadYx();
                        Include(x, y);
                        Include(x + width, y + height);
                    }

                    break;
                }

                case WmfRecordType.PatBlt:
                {
                    Skip(4);
                    int height = I16();
                    int width = I16();
                    (int x, int y) = ReadYx();
                    Include(x, y);
                    Include(x + width, y + height);
                    break;
                }

                default:
                    break;
            }

            Seek((int)next);
        }

        _position = savedPosition;
        _failed = savedFailed;

        if (windowExtent is { } window)
        {
            return new Bounds(windowX, windowY, window.W, window.H, mode);
        }

        if (viewportExtent is { } viewport)
        {
            return new Bounds(viewportX, viewportY, viewport.W, viewport.H, mode);
        }

        if (any)
        {
            // Inclusive, as GDI's rectangles are: a drawing from x=0 to x=100 is 101 units wide.
            return new Bounds(left, top, right - left + 1, bottom - top + 1, mode);
        }

        return new Bounds(0, 0, MaxImpliedWidth + 1, MaxImpliedWidth + 1, mode);
    }

    // ---------------------------------------------------------------- replay

    private void ReplayRecords()
    {
        while (!_failed && !_budget.IsExhausted)
        {
            int recordStart = _position;
            uint size = U32();
            ushort function = U16();

            if (_failed || size < 3 || function == (ushort)WmfRecordType.Eof) break;
            if (!_budget.ChargeRecord()) break;

            long next = recordStart + ((long)size * 2);
            if (next > _bytes.Length) break;

            if (_skipRecords > 0)
            {
                _skipRecords--;
            }
            else
            {
                Record((WmfRecordType)function, size, (int)next);
            }

            Seek((int)next);
        }
    }

    private void Record(WmfRecordType function, uint size, int end)
    {
        switch (function)
        {
            case WmfRecordType.SetBkColour:
                _context.BackgroundColour = ReadColour();
                break;

            case WmfRecordType.SetBkMode:
                _context.BackgroundMode = (BackgroundMode)U16();
                break;

            case WmfRecordType.SetMapMode:
                _context.Mapping.SetMode((MappingMode)U16());
                break;

            case WmfRecordType.SetRop2:
                // Only R2_NOP is modelled; see MetafileDeviceContext.IsNoOperation.
                _context.IsNoOperation = U16() == 11;
                break;

            case WmfRecordType.SetTextColour:
                _context.TextColour = ReadColour();
                break;

            case WmfRecordType.SetTextAlign:
                _context.TextAlignment = (TextAlignment)U16();
                break;

            case WmfRecordType.SetPolyFillMode:
                _context.PolygonFillMode = U16() == 2 ? PolygonFillMode.Winding : PolygonFillMode.Alternate;
                break;

            case WmfRecordType.SetWindowOrg:
            {
                (int x, int y) = ReadYx();
                _context.Mapping.WindowOriginX = x;
                _context.Mapping.WindowOriginY = y;
                break;
            }

            case WmfRecordType.SetWindowExt:
            {
                int h = I16();
                int w = I16();
                _context.Mapping.SetWindowExtent(w, h);
                break;
            }

            case WmfRecordType.OffsetWindowOrg:
            {
                (int x, int y) = ReadYx();
                _context.Mapping.WindowOriginX += x;
                _context.Mapping.WindowOriginY += y;
                break;
            }

            case WmfRecordType.ScaleWindowExt:
            {
                (double x, double y) = ReadRatio();
                _context.Mapping.ScaleWindowExtent(x, y);
                break;
            }

            case WmfRecordType.OffsetViewportOrg:
            {
                (int x, int y) = ReadYx();
                _context.Mapping.ViewportOriginX += x;
                _context.Mapping.ViewportOriginY += y;
                break;
            }

            case WmfRecordType.ScaleViewportExt:
            {
                (double x, double y) = ReadRatio();
                _context.Mapping.ScaleViewportExtent(x, y);
                break;
            }

            case WmfRecordType.SetViewportOrg:
            case WmfRecordType.SetViewportExt:
                // Deliberately ignored, as emfio ignores them (wmfreader.cxx:449-455): the
                // viewport a WMF states describes the device it was recorded on, and honouring it
                // rescales the picture to a screen it is not being drawn to.
                break;

            case WmfRecordType.SaveDc:
                _context.Save();
                break;

            case WmfRecordType.RestoreDc:
                _context.Restore(I16());
                break;

            case WmfRecordType.MoveTo:
            {
                (int x, int y) = ReadYx();
                _context.CurrentPosition = (x, y);
                break;
            }

            case WmfRecordType.LineTo:
            {
                (int x, int y) = ReadYx();
                (double px, double py) = _context.CurrentPosition;
                _painter.Stroke(new GraphicsPath().MoveTo(Map(px, py)).LineTo(Map(x, y)));
                _context.CurrentPosition = (x, y);
                break;
            }

            case WmfRecordType.SetPixel:
            {
                Colour colour = ReadColour();
                (int x, int y) = ReadYx();
                DocPoint point = Map(x, y);

                // A pixel has no length, so it is drawn as the thinnest line the device can
                // manage rather than as a rectangle of an invented size. The pen is swapped
                // rather than the sink written to directly, so the clip and the budget still
                // apply.
                MetafilePen pen = _context.Pen;
                _context.Pen = new MetafilePen(colour, Length.Zero);
                _painter.Stroke(new GraphicsPath()
                    .MoveTo(point)
                    .LineTo(new DocPoint(point.X + Length.FromEmu(1), point.Y)));
                _context.Pen = pen;
                break;
            }

            case WmfRecordType.Rectangle:
                Shape(MetafileGeometry.Rectangle(MapRect(ReadRect())));
                break;

            case WmfRecordType.RoundRect:
            {
                int h = I16();
                int w = I16();
                DocRect rect = MapRect(ReadRect());

                // GDI states the corner ellipse's full width and height; the radii are half.
                Shape(MetafileGeometry.RoundRectangle(
                    rect,
                    _context.Mapping.MapWidth(w) / 2.0,
                    _context.Mapping.MapWidth(h) / 2.0));
                break;
            }

            case WmfRecordType.Ellipse:
                Shape(MetafileGeometry.Ellipse(MapRect(ReadRect())));
                break;

            case WmfRecordType.Arc:
            case WmfRecordType.Pie:
            case WmfRecordType.Chord:
            {
                (int ex, int ey) = ReadYx();
                (int sx, int sy) = ReadYx();
                DocRect rect = MapRect(ReadRect());
                DocPoint start = Map(sx, sy);
                DocPoint finish = Map(ex, ey);

                if (function == WmfRecordType.Pie && sx == ex && sy == ey)
                {
                    // i73608: GDI draws a full ellipse when a pie's two radials coincide, where
                    // an arc-drawing routine would draw nothing.
                    Shape(MetafileGeometry.Ellipse(rect));
                    break;
                }

                MetafileGeometry.ArcClosure closure = function switch
                {
                    WmfRecordType.Pie => MetafileGeometry.ArcClosure.Pie,
                    WmfRecordType.Chord => MetafileGeometry.ArcClosure.Chord,
                    _ => MetafileGeometry.ArcClosure.Open,
                };

                GraphicsPath path = MetafileGeometry.Arc(
                    rect, start, finish, closure, _context.IsArcDirectionClockwise);

                // An open arc is a stroke only: GDI never fills one, whatever brush is selected.
                if (closure == MetafileGeometry.ArcClosure.Open) _painter.Stroke(path);
                else Shape(path);

                if (function == WmfRecordType.Arc) _context.CurrentPosition = (ex, ey);
                break;
            }

            case WmfRecordType.Polygon:
            {
                if (ReadPoints(U16(), end) is { } points) Shape(MetafileGeometry.Polygon(points));
                break;
            }

            case WmfRecordType.Polyline:
            {
                if (ReadPoints(U16(), end) is { } points) _painter.Stroke(MetafileGeometry.Polyline(points));
                break;
            }

            case WmfRecordType.PolyPolygon:
                PolyPolygon(end);
                break;

            case WmfRecordType.IntersectClipRect:
                _context.Clip.Intersect(MapRect(ReadRect()));
                break;

            case WmfRecordType.ExcludeClipRect:
                Skip(8);
                _context.Clip.MarkUnsupported();
                break;

            case WmfRecordType.OffsetClipRgn:
            {
                int h = I16();
                int w = I16();
                (double dx, double dy) = _context.Mapping.MapSizeMm100(w, h);
                _context.Clip = _context.Clip.Translate(MetafileMapping.Emu(dx), MetafileMapping.Emu(dy));
                break;
            }

            case WmfRecordType.SelectClipRegion:
            {
                int handle = U16();
                if (handle == 0) _context.Clip.Reset();
                else if (_context.Objects[handle] is MetafileRegion region) _context.Clip.Replace(region.Path);
                break;
            }

            case WmfRecordType.SelectObject:
            case WmfRecordType.SelectPalette:
                if (!_context.Select(U16()))
                {
                    Warn("PL6031", "A WMF selected an object it never created; the previous one was kept.");
                }

                break;

            case WmfRecordType.DeleteObject:
                _context.Objects.Delete(U16());
                break;

            case WmfRecordType.CreatePenIndirect:
                CreatePen();
                break;

            case WmfRecordType.CreateBrushIndirect:
                CreateBrush();
                break;

            case WmfRecordType.CreateFontIndirect:
                CreateFont();
                break;

            case WmfRecordType.CreatePalette:
                CreatePalette();
                break;

            case WmfRecordType.CreateRegion:
                CreateRegion(end);
                break;

            case WmfRecordType.CreatePatternBrush:
                CreatePatternBrush(end);
                break;

            case WmfRecordType.DibCreatePatternBrush:
                CreateDibPatternBrush(end);
                break;

            case WmfRecordType.CreateBrush:
            case WmfRecordType.CreateBitmap:
            case WmfRecordType.CreateBitmapIndirect:
                // Not modelled, but the handle must still be consumed or every later select is
                // off by one.
                _context.Objects.Add(UnsupportedGraphicsObject.Instance);
                Warn("PL6031", "A WMF created a bitmap or brush object Paperless does not model.");
                break;

            case WmfRecordType.TextOut:
                TextOut(end);
                break;

            case WmfRecordType.ExtTextOut:
                ExtTextOut(end);
                break;

            case WmfRecordType.PatBlt:
            {
                Skip(4);
                int h = I16();
                int w = I16();
                (int x, int y) = ReadYx();
                _painter.Fill(MetafileGeometry.Rectangle(MapRect((x, y, x + w, y + h))));
                break;
            }

            case WmfRecordType.FillRegion:
            case WmfRecordType.PaintRegion:
            {
                int handle = U16();
                if (function == WmfRecordType.FillRegion)
                {
                    // FillRegion names the region first and the brush second.
                    if (_context.Objects[handle] is MetafileRegion regionToFill
                        && _context.Objects[U16()] is MetafileBrush brush)
                    {
                        MetafileBrush previous = _context.Brush;
                        bool selected = _context.IsBrushSelected;
                        _context.Brush = brush;
                        _context.IsBrushSelected = true;
                        _painter.Fill(regionToFill.Path);
                        _context.Brush = previous;
                        _context.IsBrushSelected = selected;
                    }
                }
                else if (_context.Objects[handle] is MetafileRegion region)
                {
                    _painter.Fill(region.Path);
                }

                break;
            }

            case WmfRecordType.FrameRegion:
            case WmfRecordType.InvertRegion:
                Warn("PL6031", $"A WMF used the {function} record, which Paperless does not draw.");
                break;

            case WmfRecordType.DibBitBlt:
            case WmfRecordType.DibStretchBlt:
            case WmfRecordType.StretchDib:
                DibBlt(function, size, end);
                break;

            case WmfRecordType.BitBlt:
            case WmfRecordType.StretchBlt:
                DeviceBlt(function, size, end);
                break;

            case WmfRecordType.SetDibToDev:
                SetDibToDevice(end);
                break;

            case WmfRecordType.Escape:
                Escape(size, end);
                break;

            case WmfRecordType.RealizePalette:
            case WmfRecordType.SetRelAbs:
            case WmfRecordType.SetStretchBltMode:
            case WmfRecordType.SetTextCharExtra:
            case WmfRecordType.SetTextJustification:
            case WmfRecordType.SetMapperFlags:
            case WmfRecordType.ResizePalette:
            case WmfRecordType.AnimatePalette:
            case WmfRecordType.SetPalEntries:
            case WmfRecordType.ResetDc:
            case WmfRecordType.StartDoc:
            case WmfRecordType.StartPage:
            case WmfRecordType.EndPage:
            case WmfRecordType.AbortDoc:
            case WmfRecordType.EndDoc:
                // Records with no visible effect on a device-independent replay.
                break;

            case WmfRecordType.FloodFill:
            case WmfRecordType.ExtFloodFill:
            case WmfRecordType.DrawText:
                Warn("PL6031", $"A WMF used the {function} record, which Paperless does not draw.");
                break;

            default:
                Warn("PL6031", $"A WMF used an unknown record function 0x{(ushort)function:X4}.");
                break;
        }
    }

    // ---------------------------------------------------------------- objects

    private void CreatePen()
    {
        uint style = U16();
        int width = I16();
        Skip(2);        // the pen's height, which GDI ignores
        Colour colour = ReadColour();

        Length mapped = _context.Mapping.MapWidth(width);

        // The dot length is one logical unit wider than the pen, which is what keeps a
        // one-unit pen's dashes from collapsing to nothing (mtftools.cxx:1069-1074).
        Length dot = _context.Mapping.MapWidth(width + 1);

        _context.Objects.Add(new MetafilePen(
            colour,
            mapped,
            style,
            MetafilePen.Dashes(style, dot),
            (style & PenStyle.EndCapMask) switch
            {
                PenStyle.EndCapRound when width != 0 => LineCap.Round,
                PenStyle.EndCapSquare when width != 0 => LineCap.Square,
                _ => LineCap.Butt,
            },
            (style & PenStyle.JoinMask) switch
            {
                PenStyle.JoinRound => LineJoin.Round,
                PenStyle.JoinBevel => LineJoin.Bevel,
                _ => LineJoin.Miter,
            }));
    }

    private void CreateBrush()
    {
        BrushStyle style = (BrushStyle)U16();
        Colour colour = ReadColour();
        HatchStyle hatch = (HatchStyle)U16();

        _context.Objects.Add(style switch
        {
            BrushStyle.Hatched => new MetafileBrush(BrushStyle.Hatched, colour, hatch),
            BrushStyle.Null => MetafileBrush.None,
            _ => new MetafileBrush(BrushStyle.Solid, colour),
        });
    }

    private void CreateFont()
    {
        int height = I16();
        int width = I16();
        int escapement = I16();
        Skip(2);            // orientation, which GDI ties to the escapement in this mapping mode
        int weight = I16();
        byte italic = U8();
        byte underline = U8();
        byte strikeOut = U8();
        byte charSet = U8();
        Skip(4);            // output precision, clipping precision, quality, pitch and family

        // The face name is a fixed 32-byte field, NUL-padded rather than NUL-terminated when it
        // fills the field exactly.
        Span<byte> name = stackalloc byte[32];
        Read(name);
        int length = name.IndexOf((byte)0);
        if (length < 0) length = name.Length;

        // The face name is in the font's own character set, which is stated in the same record —
        // so a Cyrillic font's name has to be decoded with the Cyrillic code page.
        string family = Encoding.Latin1.GetString(name[..length]);
        try
        {
            family = Text.Encodings.LegacyCodePages
                .Get(MetafileTextEngine.CodePage(charSet))
                .GetString(name[..length]);
        }
        catch (ArgumentException)
        {
            // Keep the Latin-1 reading; a face name that will not decode is still better than none.
        }

        // A stated height of zero means "the device decides", which for a metafile means nothing
        // at all — so it takes the same 12 pt LibreOffice gives it (mtftools.cxx:1108-1110).
        Length size = height == 0
            ? MetafileFont.Default.Size
            : _context.Mapping.MapSize(0, Math.Abs(height), rotate: false).Height;

        if (size <= Length.Zero) size = MetafileFont.Default.Size;

        _context.Objects.Add(new MetafileFont(
            family,
            size,
            weight is > 0 and <= 1000 ? weight : 400,
            italic != 0,
            underline != 0,
            strikeOut != 0,
            escapement,
            charSet,
            width != 0 && height != 0 ? Math.Abs((double)width / height) : 0));
    }

    private void CreatePalette()
    {
        Skip(2);            // the first entry's index, which only matters for animation
        int count = U16();

        if (!MetafileBudget.Plausible(count, 4, _bytes.Length - _position))
        {
            _context.Objects.Add(UnsupportedGraphicsObject.Instance);
            return;
        }

        List<Colour> colours = new(count);
        for (int i = 0; i < count && !_failed; i++)
        {
            uint entry = U32();
            colours.Add(new Colour((byte)entry, (byte)(entry >> 8), (byte)(entry >> 16)));
        }

        _context.Objects.Add(new MetafilePalette(colours));
    }

    /// <summary>
    /// Reads a region object's scan list into a path.
    /// </summary>
    /// <remarks>
    /// <c>emfio</c> does not implement this record at all — it creates a placeholder and warns
    /// (<c>wmfreader.cxx:1246-1251</c>) — so a WMF that clips through a region draws unclipped
    /// there. The scan list is a set of disjoint rectangles, which one path of several subpaths
    /// expresses exactly, so decoding it costs little and is strictly better.
    /// </remarks>
    private void CreateRegion(int end)
    {
        Skip(2);            // next in chain
        Skip(2);            // object type
        Skip(4);            // object count
        Skip(2);            // region size in bytes
        int scans = U16();
        Skip(2);            // the largest scan's point count
        (int left, int top, int right, int bottom) = ReadRect();

        if (!MetafileBudget.Plausible(scans, 8, end - _position))
        {
            _context.Objects.Add(UnsupportedGraphicsObject.Instance);
            return;
        }

        GraphicsPath path = new();
        int rectangles = 0;

        for (int i = 0; i < scans && !_failed && _position < end; i++)
        {
            int count = U16();
            int scanTop = I16();
            int scanBottom = I16();

            if (count < 0 || count % 2 != 0 || !MetafileBudget.Plausible(count, 2, end - _position)) break;

            for (int j = 0; j < count / 2 && !_failed; j++)
            {
                int scanLeft = I16();
                int scanRight = I16();
                MetafileGeometry.AddRectangle(path, MapRect((scanLeft, scanTop, scanRight, scanBottom)));
                rectangles++;
            }

            Skip(2);        // the count, repeated
        }

        _context.Objects.Add(new MetafileRegion(path, MapRect((left, top, right, bottom)), rectangles));
    }

    private void CreatePatternBrush(int end)
    {
        // [MS-WMF] 2.3.4.4 wraps the bitmap in a 32-byte legacy BITMAP structure.
        DeviceIndependentBitmap.Result? bitmap = DeviceIndependentBitmap.ReadDeviceDependent(
            Span(end), 32, _context.TextColour, _context.BackgroundColour);

        AddPatternBrush(bitmap);
    }

    private void CreateDibPatternBrush(int end)
    {
        BrushStyle style = (BrushStyle)U16();
        Skip(2);            // colour usage

        ReadOnlySpan<byte> data = Span(end);

        // [MS-WMF] 2.3.4.8 says BS_PATTERN means a Bitmap16, but many producers write a DIB
        // anyway, so the leading DWORD decides: only a handful of values are valid DIB header
        // sizes (wmfreader.cxx:1058-1084).
        bool looksLikeDib = true;
        if (style == BrushStyle.Pattern && data.Length >= 4)
        {
            uint probe = BinaryPrimitives.ReadUInt32LittleEndian(data);
            looksLikeDib = probe is 12 or 40 or 52 or 56 or 64 or 108 or 124;
        }

        AddPatternBrush(looksLikeDib
            ? DeviceIndependentBitmap.Read(data)
            : DeviceIndependentBitmap.ReadDeviceDependent(data, 10, _context.TextColour, _context.BackgroundColour));
    }

    private void AddPatternBrush(DeviceIndependentBitmap.Result? bitmap)
    {
        if (bitmap is not { } tile)
        {
            _context.Objects.Add(new MetafileBrush(BrushStyle.Solid, Colour.White));
            Warn("PL6031", "A WMF's pattern brush could not be read; it was filled white instead.");
            return;
        }

        // A pattern brush tiles in device pixels, and a display list has none. The tile is sized
        // at 96 pixels to the inch — the same assumption the rest of Paperless makes about a
        // nominal pixel — so the pattern's scale is at least consistent with everything else.
        DocSize size = new(
            Length.FromPixels(tile.Width, 96),
            Length.FromPixels(tile.Height, 96));

        _context.Objects.Add(new MetafileBrush(
            BrushStyle.DibPattern, _context.TextColour, HatchStyle.Horizontal, tile.Image, size));
    }

    // ---------------------------------------------------------------- drawing

    /// <summary>Fills and strokes a closed shape, which is what GDI's shape records do.</summary>
    private void Shape(GraphicsPath path) => _painter.FillAndStroke(path);

    private void PolyPolygon(int end)
    {
        int polygons = U16();
        if (!MetafileBudget.Plausible(polygons, 2, end - _position)) return;

        int[] counts = new int[polygons];
        long total = 0;

        for (int i = 0; i < polygons && !_failed; i++)
        {
            counts[i] = U16();
            total += counts[i];
        }

        if (_failed || !MetafileBudget.Plausible((int)Math.Min(total, int.MaxValue), 4, end - _position)) return;
        if (!_budget.ChargeSegments((int)Math.Min(total, int.MaxValue))) return;

        GraphicsPath path = new();

        for (int i = 0; i < polygons && !_failed; i++)
        {
            List<DocPoint> points = new(counts[i]);
            for (int j = 0; j < counts[i] && !_failed; j++) points.Add(Map(I16(), I16()));
            MetafileGeometry.AddPolygon(path, points);
        }

        Shape(path);
    }

    private List<DocPoint>? ReadPoints(int count, int end)
    {
        if (!MetafileBudget.Plausible(count, 4, end - _position)) return null;
        if (!_budget.ChargeSegments(count)) return null;

        List<DocPoint> points = new(count);
        for (int i = 0; i < count && !_failed; i++) points.Add(Map(I16(), I16()));

        return points.Count > 0 ? points : null;
    }

    private void TextOut(int end)
    {
        int length = U16();
        int stored = (length + 1) & ~1;

        if (length <= 0 || _position + stored > end) return;

        string text = MetafileTextEngine.Decode(Span(_position + length), _context.Font);
        Skip(stored);
        (int x, int y) = ReadYx();

        DrawText(text, x, y, null);
    }

    private void ExtTextOut(int end)
    {
        (int x, int y) = ReadYx();
        int length = U16();
        ushort options = U16();

        const ushort Opaque = 0x0002;
        const ushort Clipped = 0x0004;

        DocRect rect = default;
        if ((options & (Opaque | Clipped)) != 0)
        {
            int left = I16();
            int top = I16();
            int right = I16();
            int bottom = I16();
            rect = MapRect((left, top, right, bottom));

            if ((options & Opaque) != 0) _painter.FillBackground(rect);
        }

        if (length <= 0) return;

        int stored = (length + 1) & ~1;
        if (_position + stored > end) stored = Math.Max(end - _position, 0);
        if (stored <= 0) return;

        string text = MetafileTextEngine.Decode(Span(_position + Math.Min(length, stored)), _context.Font);
        Skip(stored);

        // Whatever is left in the record is the DX array: one 16-bit advance per character of
        // the *encoded* string, which is not the same count as the decoded string when the code
        // page is multi-byte.
        List<Length>? advances = null;
        int remaining = (end - _position) / 2;

        if (remaining >= text.Length && text.Length > 0)
        {
            advances = ReadAdvances(text.Length);
        }

        if ((options & Clipped) != 0 && !rect.IsEmpty)
        {
            MetafileClip saved = _context.Clip;
            _context.Clip = _context.Clip.Clone();
            _context.Clip.Intersect(rect);
            DrawText(text, x, y, advances);
            _context.Clip = saved;
        }
        else
        {
            DrawText(text, x, y, advances);
        }
    }

    /// <summary>
    /// Reads a DX array and maps it, keeping the running sum in logical units.
    /// </summary>
    /// <remarks>
    /// The sum is accumulated before mapping and the per-character advance taken as the
    /// difference of two mapped sums, rather than mapping each advance on its own. With an
    /// integer mapping the two differ: mapping fifty one-unit advances separately rounds fifty
    /// times and can lose a whole unit of width off a string, which shows up as text that drifts
    /// left of where the file put it (<c>MtfTools::DrawText</c>, <c>mtftools.cxx:1988-2008</c>).
    /// </remarks>
    private List<Length> ReadAdvances(int count)
    {
        List<Length> advances = new(count);
        double sum = 0;
        Length previous = Length.Zero;

        for (int i = 0; i < count && !_failed; i++)
        {
            sum += I16();
            (double dx, double dy) = _context.Mapping.MapSizeMm100(sum, 0);
            Length mapped = MetafileMapping.Emu(Math.Sqrt((dx * dx) + (dy * dy)) * Math.Sign(sum == 0 ? 1 : sum));
            advances.Add(mapped - previous);
            previous = mapped;
        }

        return advances;
    }

    private void DrawText(string text, int x, int y, IReadOnlyList<Length>? advances)
    {
        if (string.IsNullOrEmpty(text)) return;

        DocPoint reference = (_context.TextAlignment & TextAlignment.UpdateCurrentPosition) != 0
            ? Map(_context.CurrentPosition.X, _context.CurrentPosition.Y)
            : Map(x, y);

        if (_text.Layout(text, _context.Font, reference, _context.TextAlignment, advances) is not { } laid) return;

        // GDI states the escapement anticlockwise in tenths of a degree, and document space is
        // y-down, so a positive escapement is a positive rotation about the origin here too.
        double radians = _context.Font.Escapement * Math.PI / 1800.0;
        _painter.DrawGlyphRun(laid.Run, radians);

        if ((_context.TextAlignment & TextAlignment.UpdateCurrentPosition) != 0)
        {
            // The current position advances in logical units, so the run's width has to come back
            // through the mapping. Only the common unrotated case is handled exactly.
            double scale = _context.Mapping.MapSizeMm100(1, 0).Width * MetafileMapping.EmuPerMm100;
            double advance = scale == 0 ? 0 : laid.Width.Emu / scale;
            _context.CurrentPosition = (_context.CurrentPosition.X + advance, _context.CurrentPosition.Y);
        }
    }

    // ---------------------------------------------------------------- bitmaps

    private void DibBlt(WmfRecordType function, uint size, int end)
    {
        uint rop = U32();
        if (function == WmfRecordType.StretchDib) Skip(2);      // colour usage

        int sourceHeight = 0;
        int sourceWidth = 0;

        if (function is WmfRecordType.DibStretchBlt or WmfRecordType.StretchDib)
        {
            sourceHeight = I16();
            sourceWidth = I16();
        }

        int sourceY = I16();
        int sourceX = I16();

        bool noSource = function != WmfRecordType.StretchDib && NoSourceBitmap((ushort)function, size);
        if (noSource) Skip(2);

        int destHeight = I16();
        int destWidth = I16();
        (int destX, int destY) = ReadYx();

        if (destWidth == 0 || destHeight == 0) return;

        DocRect destination = MapRect((destX, destY, destX + destWidth, destY + destHeight));

        if (noSource)
        {
            // No bitmap at all: the raster operation is meant to combine the destination with the
            // brush, which a display list cannot express.
            Warn("PL6033", "A WMF blitted with no source bitmap, which Paperless cannot reproduce.");
            return;
        }

        if (DeviceIndependentBitmap.Read(Span(end)) is not { } bitmap)
        {
            Warn("PL6031", "A WMF carried a bitmap in a form Paperless cannot read.");
            return;
        }

        Blit(bitmap, destination, sourceX, sourceY, sourceWidth, sourceHeight, rop);
    }

    private void DeviceBlt(WmfRecordType function, uint size, int end)
    {
        Skip(4);            // raster operation

        if (function == WmfRecordType.StretchBlt) Skip(4);

        int sourceY = I16();
        int sourceX = I16();

        bool noSource = NoSourceBitmap((ushort)function, size);
        if (noSource) Skip(2);

        int destHeight = I16();
        int destWidth = I16();
        (int destX, int destY) = ReadYx();

        if (noSource || destWidth == 0 || destHeight == 0)
        {
            Warn("PL6033", "A WMF blitted with no source bitmap, which Paperless cannot reproduce.");
            return;
        }

        DocRect destination = MapRect((destX, destY, destX + destWidth, destY + destHeight));

        // The inline bitmap is device-dependent: no colour table, no compression, and only the
        // monochrome form is worth reading — which is the same limit emfio's CreateBitmap16 has.
        if (DeviceIndependentBitmap.ReadDeviceDependent(
                Span(end), 10, _context.TextColour, _context.BackgroundColour) is not { } bitmap)
        {
            Warn("PL6031", "A WMF carried a device-dependent bitmap Paperless cannot read.");
            return;
        }

        Blit(bitmap, destination, sourceX, sourceY, destWidth, destHeight, 0x00CC0020);
    }

    /// <summary>
    /// Reads <c>META_SETDIBITSTODEVICE</c>, which <c>emfio</c> does not implement at all.
    /// </summary>
    /// <remarks>
    /// It is a straight one-to-one blit of a DIB, with no scaling: the destination extent is the
    /// source extent. Producers that write it are usually placing a scanned image, so leaving it
    /// out means leaving out the picture.
    /// </remarks>
    private void SetDibToDevice(int end)
    {
        Skip(2);            // colour usage
        Skip(2);            // scan count
        Skip(2);            // start scan
        int sourceY = I16();
        int sourceX = I16();
        int height = I16();
        int width = I16();
        (int destX, int destY) = ReadYx();

        if (width == 0 || height == 0) return;

        if (DeviceIndependentBitmap.Read(Span(end)) is not { } bitmap)
        {
            Warn("PL6031", "A WMF carried a bitmap in a form Paperless cannot read.");
            return;
        }

        Blit(bitmap, MapRect((destX, destY, destX + width, destY + height)), sourceX, sourceY, width, height, 0x00CC0020);
    }

    /// <summary>
    /// Places a bitmap, expressing a source rectangle as a scale plus a clip.
    /// </summary>
    /// <remarks>
    /// <b>Cropping without a codec.</b> The image is still encoded — nothing in this library has
    /// looked at a pixel — so a source rectangle cannot be cut out of it. Placing the whole image
    /// scaled so that the wanted part lands exactly on the destination, and clipping to the
    /// destination, is the same picture and needs no decode. The alternative would be to pull a
    /// rasteriser into <c>Paperless.Vector</c>, which is the dependency
    /// <c>RasterImage.Encoded</c> exists to avoid.
    /// </remarks>
    private void Blit(
        DeviceIndependentBitmap.Result bitmap,
        DocRect destination,
        int sourceX,
        int sourceY,
        int sourceWidth,
        int sourceHeight,
        uint rop)
    {
        const uint SourceCopy = 0x00CC0020;
        const uint Blackness = 0x00000042;
        const uint Whiteness = 0x00FF0062;

        if (rop is Blackness or Whiteness)
        {
            _painter.FillBackground(destination);
            return;
        }

        if (rop != SourceCopy)
        {
            // Every other ternary operation combines the source with what is already on the page,
            // which a display list has no way to read back. Drawing the source alone is what
            // LibreOffice falls back to as well, and it is right for the commonest of them.
            Warn(
                "PL6033",
                $"A WMF blitted with raster operation 0x{rop:X8}; the source was drawn without it.");
        }

        bool crops = sourceWidth > 0
            && sourceHeight > 0
            && (sourceX != 0 || sourceY != 0 || sourceWidth != bitmap.Width || sourceHeight != bitmap.Height)
            && sourceX + sourceWidth <= bitmap.Width
            && sourceY + sourceHeight <= bitmap.Height;

        if (!crops)
        {
            _painter.DrawImage(bitmap.Image, destination);
            return;
        }

        double scaleX = (double)destination.Width.Emu / sourceWidth;
        double scaleY = (double)destination.Height.Emu / sourceHeight;

        DocRect whole = new(
            destination.X - Length.FromEmu((long)Math.Round(sourceX * scaleX)),
            destination.Y - Length.FromEmu((long)Math.Round(sourceY * scaleY)),
            Length.FromEmu((long)Math.Round(bitmap.Width * scaleX)),
            Length.FromEmu((long)Math.Round(bitmap.Height * scaleY)));

        _painter.DrawImage(bitmap.Image, whole, destination);
    }

    /// <summary>
    /// True when a blit record carries no bitmap.
    /// </summary>
    /// <remarks>
    /// There is no flag for it: the two forms are told apart by size alone, because the record's
    /// high byte is its original parameter count and a record exactly three words longer than
    /// that has nothing after its parameters (<c>wmfreader.cxx:919</c>). Reading the bitmap that
    /// is not there is how a decoder walks off the end of a legitimate file.
    /// </remarks>
    private static bool NoSourceBitmap(ushort function, uint size) => size == ((uint)function >> 8) + 3;

    // ---------------------------------------------------------------- escapes

    /// <summary>
    /// Reads an <c>ESCAPE</c> record, which is where a producer hides things the format has no
    /// record for.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two payloads matter. A <c>WMFC</c> comment carries one chunk of a complete EMF, which the
    /// producer expects a capable consumer to prefer over the WMF records around it — those are
    /// accumulated and <em>reported</em>, not replayed, because replaying both would draw the
    /// picture twice. A private Unicode escape carries text the eight-bit records cannot express,
    /// together with a count of the following records that draw the same text in a legacy
    /// approximation.
    /// </para>
    /// <para>
    /// <b>Nothing an escape contains is resolved outside the document.</b> Escapes are the WMF
    /// analogue of the SVG <c>href</c> that turned out to read <c>/etc/passwd</c>: they are an
    /// open extension point in an attacker-supplied file. The two payloads above are the only
    /// ones read at all, and neither names anything outside the byte range it was found in.
    /// </para>
    /// </remarks>
    private void Escape(uint size, int end)
    {
        ushort mode = U16();
        int length = U16();

        if (mode != MetafileComment || length < 4) return;
        if (size != (((uint)length + 1) >> 1) + 5) return;

        uint magic = U32();

        if (magic == UnicodeEscapeMagic && length >= 14)
        {
            UnicodeEscape(end);
            return;
        }

        if (magic != EmfCommentMagic || length < 34) return;
        if (length + 10 > size * 2) return;

        uint commentType = U32();
        uint version = U32();
        Skip(2);            // checksum
        Skip(4);            // flags
        uint chunkCount = U32();
        uint chunkSize = U32();
        Skip(4);            // remaining size
        uint totalSize = U32();

        if (commentType != 0x01 || version != 0x00010000 || chunkCount == 0) return;

        if (_emfChunks == 0)
        {
            _emfChunkTotal = chunkCount;
            if (totalSize > (uint)(_bytes.Length - _position)) return;
        }
        else if (_emfChunkTotal != chunkCount)
        {
            // The chunks disagree about how many there are, so the reassembly cannot be trusted.
            _embeddedEmf.Clear();
            _emfChunkTotal = 0;
            return;
        }

        _emfChunks++;

        if (chunkSize + 34 > (uint)length) return;
        if (chunkSize > (uint)(end - _position)) return;

        _embeddedEmf.AddRange(Span(_position + (int)chunkSize));
    }

    /// <summary>
    /// Reads the private Unicode text escape and draws it, skipping the legacy records that
    /// follow.
    /// </summary>
    /// <remarks>
    /// The escape states how many following records it replaces, and those records draw the same
    /// string transliterated into an eight-bit code page. Drawing both would double-strike every
    /// glyph, so the count is honoured — which also means that if this parse is wrong the text is
    /// lost rather than drawn twice, so every field is bounds-checked before it is believed.
    /// The record's CRC is not checked: it guards against corruption, not against an attacker,
    /// and the bounds checks do not depend on it.
    /// </remarks>
    private void UnicodeEscape(int end)
    {
        ushort second = U16();
        if (second != 0x000A) return;

        Skip(4);            // the payload's CRC — see the remarks
        uint escape = U32();
        if (escape != PrivateEscapeUnicode) return;

        int x = I32();
        int y = I32();
        uint characters = U32();

        if (_failed || characters == 0) return;
        if (!MetafileBudget.Plausible((int)Math.Min(characters, int.MaxValue), 2, end - _position)) return;

        char[] buffer = new char[characters];
        for (int i = 0; i < characters && !_failed; i++) buffer[i] = (char)U16();
        if (_failed) return;

        string text = new(buffer);

        uint dxCount = U32();
        List<Length>? advances = null;

        if (dxCount >= (uint)text.Length && MetafileBudget.Plausible((int)Math.Min(dxCount, int.MaxValue), 4, end - _position))
        {
            advances = new List<Length>(text.Length);
            double sum = 0;
            Length previous = Length.Zero;

            for (int i = 0; i < dxCount && !_failed; i++)
            {
                int advance = I32();
                if (i >= text.Length) continue;

                sum += advance;
                Length mapped = MetafileMapping.Emu(_context.Mapping.MapSizeMm100(sum, 0).Width);
                advances.Add(mapped - previous);
                previous = mapped;
            }
        }

        uint skip = U32();
        if (_failed) return;

        DrawText(text, x, y, advances);
        _skipRecords = (int)Math.Min(skip, 4096);
    }

    // ---------------------------------------------------------------- helpers

    private DocPoint Map(double x, double y) => _context.Mapping.MapPoint(x, y);

    private DocRect MapRect((int Left, int Top, int Right, int Bottom) rect)
        => _context.Mapping.MapRect(rect.Left, rect.Top, rect.Right, rect.Bottom);

    private Colour ReadColour() => _context.Palette.Resolve(U32());

    private (int X, int Y) ReadYx()
    {
        int y = I16();
        int x = I16();
        return (x, y);
    }

    /// <summary>
    /// Reads a rectangle, which WMF states bottom-right first and in y/x order.
    /// </summary>
    /// <remarks>
    /// The coordinates are kept as the file states them rather than made exclusive, because
    /// GDI's rectangles are inclusive and the difference is a whole logical unit on every edge —
    /// visible as a one-pixel gap between shapes that the producer meant to abut.
    /// </remarks>
    private (int Left, int Top, int Right, int Bottom) ReadRect()
    {
        int bottom = I16();
        int right = I16();
        int top = I16();
        int left = I16();
        return (left, top, right, bottom);
    }

    private (double X, double Y) ReadRatio()
    {
        int yDenominator = I16();
        int yNumerator = I16();
        int xDenominator = I16();
        int xNumerator = I16();

        return (
            xDenominator == 0 ? 1 : (double)xNumerator / xDenominator,
            yDenominator == 0 ? 1 : (double)yNumerator / yDenominator);
    }

    private void Warn(string code, string message)
    {
        if (!_reported.Add(code + message)) return;
        _diagnostics.Add(new Diagnostic(DiagnosticSeverity.Warning, code, message));
    }

    private ReadOnlySpan<byte> Span(int end)
    {
        int from = Math.Clamp(_position, 0, _bytes.Length);
        int to = Math.Clamp(end, from, _bytes.Length);
        return _bytes.AsSpan(from, to - from);
    }

    private void Read(Span<byte> destination)
    {
        if (_position + destination.Length > _bytes.Length)
        {
            _failed = true;
            destination.Clear();
            return;
        }

        _bytes.AsSpan(_position, destination.Length).CopyTo(destination);
        _position += destination.Length;
    }

    private byte U8()
    {
        if (_position + 1 > _bytes.Length)
        {
            _failed = true;
            return 0;
        }

        return _bytes[_position++];
    }

    private short I16()
    {
        if (_position + 2 > _bytes.Length)
        {
            _failed = true;
            return 0;
        }

        short value = BinaryPrimitives.ReadInt16LittleEndian(_bytes.AsSpan(_position));
        _position += 2;
        return value;
    }

    private ushort U16()
    {
        if (_position + 2 > _bytes.Length)
        {
            _failed = true;
            return 0;
        }

        ushort value = BinaryPrimitives.ReadUInt16LittleEndian(_bytes.AsSpan(_position));
        _position += 2;
        return value;
    }

    private int I32()
    {
        if (_position + 4 > _bytes.Length)
        {
            _failed = true;
            return 0;
        }

        int value = BinaryPrimitives.ReadInt32LittleEndian(_bytes.AsSpan(_position));
        _position += 4;
        return value;
    }

    private uint U32()
    {
        if (_position + 4 > _bytes.Length)
        {
            _failed = true;
            return 0;
        }

        uint value = BinaryPrimitives.ReadUInt32LittleEndian(_bytes.AsSpan(_position));
        _position += 4;
        return value;
    }

    private void Skip(int count) => _position = Math.Min(_position + count, _bytes.Length);

    private void Seek(int position) => _position = Math.Clamp(position, 0, _bytes.Length);
}

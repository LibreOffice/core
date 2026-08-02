using System.Buffers.Binary;
using System.Text;
using Paperless.Core.Diagnostics;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector.EmfPlus;
using Paperless.Vector.Metafiles;

namespace Paperless.Vector.Emf;

/// <summary>
/// Replays an enhanced metafile's records into a display list.
/// </summary>
/// <remarks>
/// <para>
/// Ported record by record from <c>emfio/source/reader/emfreader.cxx</c>, on the device context,
/// mapping, clip, object table, text engine and DIB reader that WMF already built in
/// <c>Paperless.Vector.Metafiles</c>. Everything stateful is shared with WMF; what is EMF's own
/// is the header, the 32-bit record layout, the current path, the world transform, and the fact
/// that a bitmap's header and its bits are stated as two offsets rather than as one blob.
/// </para>
/// <para>
/// <b>The header is where an EMF differs from a WMF in kind rather than in degree.</b> A WMF
/// maps every coordinate into 1/100 mm as it is read because that is all it can do; an EMF
/// states its logical coordinate space (<c>rclBounds</c>, device units), its physical extent
/// (<c>rclFrame</c>, 1/100 mm) and the reference device that relates them (<c>szlDevice</c> and
/// <c>szlMillimeters</c>) as four independent quantities. Deriving any of them from another is
/// the classic wrongly-scaled metafile.
/// </para>
/// </remarks>
internal sealed class EmfReader
{
    /// <summary>The signature every EMF header carries: the ASCII " EMF", little-endian.</summary>
    public const uint Signature = 0x464D4520;

    private const uint StockObjectFlag = 0x80000000;

    private const ushort EtoOpaque = 0x0002;
    private const ushort EtoClipped = 0x0004;
    private const ushort EtoGlyphIndex = 0x0010;
    private const ushort EtoRightToLeft = 0x0080;
    private const ushort EtoNoRect = 0x0100;
    private const ushort EtoSmallChars = 0x0200;
    private const ushort EtoPerGlyphY = 0x2000;

    private const uint CommentEmfPlus = 0x2B464D45;
    private const uint CommentPublic = 0x43494447;

    private readonly byte[] _bytes;
    private readonly List<Diagnostic> _diagnostics;
    private readonly HashSet<string> _reported = [];
    private readonly MetafileDeviceContext _context = new();
    private readonly MetafileBudget _budget;
    private readonly MetafileTextEngine _text;
    private readonly DisplayList _list = new();
    private readonly MetafilePainter _painter;

    private int _position;
    private bool _failed;
    private int _end;
    private long _recordsLeft = int.MaxValue;
    private PendingBlit? _pending;
    private DocRect _frame;
    private (int Left, int Top, int Right, int Bottom) _bounds;
    private (int Left, int Top, int Right, int Bottom) _frameUnits;
    private (int X, int Y) _devicePixels = (100, 100);
    private (int X, int Y) _deviceMillimetres = (1, 1);
    private EmfPlusReader? _plus;

    /// <summary>Creates a reader over an enhanced metafile's bytes.</summary>
    /// <param name="bytes">The whole file.</param>
    /// <param name="limits">The work caps.</param>
    /// <param name="diagnostics">Where problems are recorded.</param>
    /// <param name="text">The text engine, shared so its font cache is.</param>
    public EmfReader(byte[] bytes, VectorLimits limits, List<Diagnostic> diagnostics, MetafileTextEngine text)
    {
        _bytes = bytes;
        _diagnostics = diagnostics;
        _text = text;
        _end = bytes.Length;
        _budget = new MetafileBudget(limits);
        _painter = new MetafilePainter(_list, _context, _budget);
    }

    /// <summary>The recorded picture.</summary>
    public DisplayList Content => _list;

    /// <summary>True when a limit stopped the replay early.</summary>
    public bool IsTruncated => _budget.IsExhausted;

    /// <summary>
    /// The physical extent the header states, which is the picture's intrinsic size.
    /// </summary>
    /// <remarks>
    /// <c>rclFrame</c>, in 1/100 mm, or the viewport extent when the frame is empty — which is
    /// what <c>~MtfTools</c> falls back to (<c>mtftools.cxx:1248-1252</c>). Real EMFs written by
    /// non-Microsoft producers do state an empty frame, so the fallback is load-bearing rather
    /// than defensive.
    /// </remarks>
    public DocSize Extent { get; private set; }

    /// <summary>
    /// The rectangle of the recorded commands' coordinates that fills a destination.
    /// </summary>
    /// <remarks>
    /// Every point is mapped as the record is read, and the mapping subtracts the frame's own
    /// origin, so the commands land in a rectangle at the origin the size of the frame. The view
    /// box and the intrinsic size therefore agree <em>numerically</em> — but they are computed
    /// from different header fields and neither is derived from the other, which is the whole
    /// point of keeping them apart: a file whose <c>rclBounds</c> disagrees with its
    /// <c>rclFrame</c> about the scale gets the right physical size and the right coordinates,
    /// where a decoder that derived one from the other would get one of them wrong.
    /// </remarks>
    public DocRect ViewBox { get; private set; }

    /// <summary>True when the bytes begin like an EMF.</summary>
    /// <remarks>
    /// Three fields have to agree: a record type of 1, a header at least 88 bytes long, and the
    /// " EMF" signature at offset 40. Any one of them alone is weak — a type of 1 is a single
    /// byte of signal — and together they are as strong as a magic number.
    /// </remarks>
    public static bool Looks(ReadOnlySpan<byte> data)
    {
        if (data.Length < 88) return false;
        if (BinaryPrimitives.ReadUInt32LittleEndian(data) != 1) return false;

        uint headerSize = BinaryPrimitives.ReadUInt32LittleEndian(data[4..]);
        if (headerSize < 88 || headerSize > data.Length) return false;

        return BinaryPrimitives.ReadUInt32LittleEndian(data[40..]) == Signature;
    }

    /// <summary>Reads the whole file. False only when the header could not be read.</summary>
    public bool Read()
    {
        if (!ReadHeader())
        {
            Warn("PL6035", "An EMF's header could not be read; the picture was not drawn.");
            return false;
        }

        ReplayRecords();
        FlushBlit();
        _painter.Finish();

        Extent = Measure();
        ViewBox = new DocRect(DocPoint.Origin, Extent);

        if (_budget.Truncation("EMF") is { } truncation) _diagnostics.Add(truncation);

        if (_context.Clip.HasUnsupportedOperation)
        {
            Warn(
                "PL6034",
                "An EMF combined clipping regions in a way that cannot be expressed; more of the "
                    + "picture may be visible than the file intended.");
        }

        return true;
    }

    /// <summary>
    /// The picture's physical extent, from the three header fields that can state it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>The frame and the bounds are stated independently and this is the one place that
    /// matters.</b> <c>rclFrame</c> is the physical extent in 1/100 mm and wins outright when it
    /// is there. When it is not — and plenty of non-Microsoft producers write zeros — the extent
    /// is derived from <c>rclBounds</c>, which is in device units, through the reference device
    /// that the header also states. Deriving it the other way round, or assuming as WMF may that
    /// the two coincide, gives a picture that is the right shape and the wrong size.
    /// </para>
    /// <para>
    /// The last resort is the viewport extent, which is what <c>~MtfTools</c> uses
    /// (<c>mtftools.cxx:1248-1252</c>) and which is a device-unit quantity read as 1/100 mm —
    /// a guess, but the same guess LibreOffice makes, so the two agree on the files that need it.
    /// </para>
    /// </remarks>
    private DocSize Measure()
    {
        if (!_frame.IsEmpty) return _frame.Size;

        int width = _bounds.Right - _bounds.Left + 1;
        int height = _bounds.Bottom - _bounds.Top + 1;

        if (width > 1 && height > 1)
        {
            double x = (double)width * _context.Mapping.ReferenceMillimetresX * 100.0
                / _context.Mapping.ReferencePixelsX;
            double y = (double)height * _context.Mapping.ReferenceMillimetresY * 100.0
                / _context.Mapping.ReferencePixelsY;

            if (x >= 1 && y >= 1) return new DocSize(MetafileMapping.Emu(x), MetafileMapping.Emu(y));
        }

        return new DocSize(
            MetafileMapping.Emu(_context.Mapping.ViewportExtentX),
            MetafileMapping.Emu(_context.Mapping.ViewportExtentY));
    }

    // ---------------------------------------------------------------- header

    /// <summary>
    /// Reads <c>EMR_HEADER</c> and settles the coordinate space, the physical extent and the
    /// reference device that relates them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>The reference device is the scale.</b> In the default <c>MM_TEXT</c> mapping a logical
    /// unit is one device pixel, and the only thing that says how big a pixel is is
    /// <c>szlMillimeters</c> against <c>szlDevice</c> — so those two fields, not the bounds and
    /// not the frame, are what turns the records into millimetres
    /// (<c>MtfTools::ImplMap</c>, <c>mtftools.cxx:603-604</c>).
    /// </para>
    /// <para>
    /// <b>The frame's origin is subtracted from every mapped point.</b> An EMF whose frame
    /// starts at (500, 500) is not a picture that belongs 5 mm down and right of its own
    /// top-left corner; it is a picture whose coordinates happen to start there. That
    /// subtraction is <c>mtftools.cxx:619-620</c> and it is why <see cref="ViewBox"/> starts at
    /// the origin.
    /// </para>
    /// <para>
    /// The stated byte count is believed only as far as the file actually goes: a truncated EMF
    /// is common and the records that are there are still worth drawing.
    /// </para>
    /// </remarks>
    private bool ReadHeader()
    {
        uint type = U32();
        uint headerSize = U32();

        if (_failed || type != (uint)EmfRecordType.Header || headerSize < 88) return false;

        (int boundsLeft, int boundsTop, int boundsRight, int boundsBottom) = ReadRect();
        (int frameLeft, int frameTop, int frameRight, int frameBottom) = ReadRect();

        if (U32() != Signature) return false;

        Skip(4);                        // version, which not even Windows checks
        uint bytes = U32();
        uint records = U32();
        Skip(2);                        // handle count; the object table grows as it needs to
        Skip(2);                        // reserved
        Skip(4);                        // description length
        Skip(4);                        // description offset
        Skip(4);                        // palette entries

        int pixelsX = I32();
        int pixelsY = I32();
        int millimetresX = I32();
        int millimetresY = I32();

        if (_failed) return false;

        if (bytes >= 88 && bytes <= (uint)_bytes.Length) _end = (int)bytes;
        if (records > 0) _recordsLeft = records;

        _context.Mapping.ReferencePixelsX = pixelsX > 0 ? pixelsX : 100;
        _context.Mapping.ReferencePixelsY = pixelsY > 0 ? pixelsY : 100;
        _context.Mapping.ReferenceMillimetresX = millimetresX > 0 ? millimetresX : 1;
        _context.Mapping.ReferenceMillimetresY = millimetresY > 0 ? millimetresY : 1;

        // Inclusive, as every GDI rectangle is: a frame from 0 to 7999 is 8000 units wide, and
        // dropping the final unit is a systematic 1/100 mm short on every picture.
        _frame = frameRight > frameLeft && frameBottom > frameTop
            ? new DocRect(
                Length.FromMm100(frameLeft),
                Length.FromMm100(frameTop),
                Length.FromMm100(frameRight - frameLeft + 1),
                Length.FromMm100(frameBottom - frameTop + 1))
            : DocRect.Empty;

        _bounds = (boundsLeft, boundsTop, boundsRight, boundsBottom);
        _frameUnits = (frameLeft, frameTop, frameRight, frameBottom);
        _devicePixels = (_context.Mapping.ReferencePixelsX, _context.Mapping.ReferencePixelsY);
        _deviceMillimetres = (_context.Mapping.ReferenceMillimetresX, _context.Mapping.ReferenceMillimetresY);

        _context.Mapping.FrameOffsetX = frameLeft;
        _context.Mapping.FrameOffsetY = frameTop;

        if (!_frame.IsEmpty) _context.Clip.Bounds = new DocRect(DocPoint.Origin, _frame.Size);

        Seek((int)Math.Min(headerSize, (uint)_bytes.Length));
        return !_failed;
    }

    // ---------------------------------------------------------------- replay

    private void ReplayRecords()
    {
        while (!_failed && !_budget.IsExhausted && _recordsLeft-- > 0)
        {
            int start = _position;
            uint type = U32();
            uint size = U32();

            // Every EMF record is a whole number of 32-bit words including its own eight-byte
            // header, which is the cheapest check there is that the stream is still in step.
            if (_failed || size < 8 || (size & 3) != 0) break;

            long next = start + size;
            if (next > _end) break;
            if (!_budget.ChargeRecord()) break;

            EmfRecordType record = (EmfRecordType)type;

            // The blit deferral only ever has to see the next record, and only a blit can be its
            // partner — so anything else settles it before it is read.
            if (_pending is not null && record is not (EmfRecordType.BitBlt
                or EmfRecordType.StretchBlt
                or EmfRecordType.StretchDIBits))
            {
                FlushBlit();
            }

            if (record == EmfRecordType.Comment) Comment((int)next);
            else if (Replays(record)) Record(record, start, (int)next);

            Seek((int)next);

            if (record == EmfRecordType.Eof) break;
        }
    }

    /// <summary>
    /// Whether a GDI record is drawn, given what the EMF+ stream has said so far.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>This is the dual-mode decision, and it is made per record because the format makes it
    /// per record.</b> A file may carry a GDI description and a GDI+ description of the same
    /// drawing, and replaying both draws everything twice. LibreOffice's rule
    /// (<c>emfreader.cxx:955-963</c>) is the one ported here, and it has three parts:
    /// </para>
    /// <list type="number">
    /// <item><description>
    /// Before any EMF+ record has been seen, GDI records are drawn. A file that carries no EMF+ at
    /// all is therefore untouched by any of this, and one whose EMF+ starts late still draws what
    /// came before.
    /// </description></item>
    /// <item><description>
    /// Once an EMF+ record has been seen, GDI records are <em>not</em> drawn — whether or not the
    /// header called the file dual. An EMF+ Only file's GDI half is a "this needs GDI+" notice,
    /// and a dual file's is the same picture again.
    /// </description></item>
    /// <item><description>
    /// Except immediately after an <c>EmfPlusGetDC</c>, which exists precisely to hand the device
    /// context back for records GDI+ has no form for; and except <c>EMR_EOF</c>, which is
    /// structural rather than drawing.
    /// </description></item>
    /// </list>
    /// <para>
    /// The choice is therefore made once, in front of the record loop, and never mid-stream — the
    /// requirement the WMF note in <c>TODO.md</c> states — with the hand-back as the one bracket
    /// the format itself defines.
    /// </para>
    /// </remarks>
    private bool Replays(EmfRecordType record)
    {
        if (_plus is null) return true;
        if (_plus.WantsDeviceContext) return true;

        return record == EmfRecordType.Eof;
    }

    /// <summary>
    /// Reads an <c>EMR_COMMENT</c>, which is where an EMF+ stream lives.
    /// </summary>
    /// <remarks>
    /// A comment is opaque to a consumer that does not understand it, which makes it the most
    /// attacker-controllable part of a metafile: every field here is checked against the record's
    /// own extent rather than believed. The data size counts the four-byte identifier that follows
    /// it, which is the off-by-four every reader of this record has to get right
    /// (<c>emfreader.cxx:935-938</c>).
    /// </remarks>
    private void Comment(int end)
    {
        uint dataSize = U32();
        if (_failed || dataSize < 4) return;

        long available = Math.Min(dataSize, (uint)Math.Max(end - _position, 0));
        if (available < 4) return;

        uint identifier = U32();
        if (identifier != CommentEmfPlus) return;

        int length = (int)(available - 4);
        if (length <= 0) return;

        _plus ??= new EmfPlusReader(
            _context,
            _painter,
            _budget,
            _text,
            _diagnostics,
            _frameUnits,
            _devicePixels,
            _deviceMillimetres);

        _plus.Process(_bytes, _position, length);
    }

    private void Record(EmfRecordType type, int start, int end)
    {
        switch (type)
        {
            // ------------------------------------------------------ mapping
            case EmfRecordType.SetWindowExtEx:
            {
                int w = I32();
                int h = I32();
                _context.Mapping.SetWindowExtent(w, h, deriveViewport: true);
                break;
            }

            case EmfRecordType.SetWindowOrgEx:
            {
                int x = I32();
                int y = I32();
                _context.Mapping.SetWindowOrigin(x, y, deriveViewport: true);
                break;
            }

            case EmfRecordType.SetViewportExtEx:
            {
                int w = I32();
                int h = I32();
                _context.Mapping.SetViewportExtent(w, h);
                break;
            }

            case EmfRecordType.SetViewportOrgEx:
                _context.Mapping.ViewportOriginX = I32();
                _context.Mapping.ViewportOriginY = I32();
                break;

            case EmfRecordType.ScaleWindowExtEx:
            {
                (double x, double y) = ReadRatio();
                _context.Mapping.ScaleWindowExtent(x, y);
                break;
            }

            case EmfRecordType.ScaleViewportExtEx:
            {
                (double x, double y) = ReadRatio();
                _context.Mapping.ScaleViewportExtent(x, y);
                break;
            }

            case EmfRecordType.SetMapMode:
                _context.Mapping.SetMode((MappingMode)U32());
                break;

            case EmfRecordType.SetWorldTransform:
                _context.Mapping.World = ReadTransform();
                break;

            case EmfRecordType.ModifyWorldTransform:
            {
                AffineTransform transform = ReadTransform();
                ModifyWorldTransform(transform, U32());
                break;
            }

            // ------------------------------------------------------ modes
            case EmfRecordType.SetBkMode:
                _context.BackgroundMode = (BackgroundMode)U32();
                break;

            case EmfRecordType.SetBkColor:
                _context.BackgroundColour = ReadColour();
                break;

            case EmfRecordType.SetTextColor:
                _context.TextColour = ReadColour();
                break;

            case EmfRecordType.SetTextAlign:
                _context.TextAlignment = (TextAlignment)U32();
                break;

            case EmfRecordType.SetPolyFillMode:
                _context.PolygonFillMode = U32() == 2 ? PolygonFillMode.Winding : PolygonFillMode.Alternate;
                break;

            case EmfRecordType.SetRop2:
                _context.IsNoOperation = U32() == 11;
                break;

            case EmfRecordType.SetArcDirection:
                // 0x02 is AD_CLOCKWISE; anything else is anticlockwise, GDI's default.
                _context.IsArcDirectionClockwise = U32() == 2;
                break;

            case EmfRecordType.SetMiterLimit:
            {
                // Stated as a 32-bit float in the specification and as an integer by half the
                // producers; both spellings of "10" have to survive.
                uint raw = U32();
                double limit = raw is > 0 and < 0x00010000 ? raw : BitConverter.UInt32BitsToSingle(raw);
                if (limit >= 1 && limit < 1e6) _context.MiterLimit = limit;
                break;
            }

            case EmfRecordType.SaveDc:
                _context.Save();
                break;

            case EmfRecordType.RestoreDc:
                // EMF states this relative and negative; a positive value is not the WMF
                // "specific saved state" spelling here, so it is taken as the top of the stack
                // (emfreader.cxx:1280-1284).
                _context.Restore(I32() is int saved and < 0 ? saved : -1);
                break;

            // ------------------------------------------------------ objects
            case EmfRecordType.SelectObject:
                if (!_context.Select((int)U32()))
                {
                    Warn("PL6031", "An EMF selected an object it never created; the previous one was kept.");
                }

                break;

            case EmfRecordType.DeleteObject:
            {
                uint handle = U32();
                if ((handle & StockObjectFlag) == 0) _context.Objects.Delete((int)(handle & 0xFFFF));
                break;
            }

            case EmfRecordType.CreatePen:
                CreatePen();
                break;

            case EmfRecordType.ExtCreatePen:
                ExtCreatePen(end);
                break;

            case EmfRecordType.CreateBrushIndirect:
                CreateBrush();
                break;

            case EmfRecordType.ExtCreateFontIndirectW:
                CreateFont();
                break;

            case EmfRecordType.CreatePalette:
                CreatePalette(end);
                break;

            case EmfRecordType.CreateDibPatternBrushPt:
            case EmfRecordType.CreateMonoBrush:
                CreateDibBrush(start, end);
                break;

            // ------------------------------------------------------ paths
            case EmfRecordType.BeginPath:
                _context.Path.Clear();
                _context.Path.IsRecording = true;
                break;

            case EmfRecordType.AbortPath:
                _context.Path.Clear();
                _context.Path.IsRecording = false;
                break;

            case EmfRecordType.EndPath:
                _context.Path.IsRecording = false;
                break;

            case EmfRecordType.CloseFigure:
                _context.Path.Close();
                break;

            case EmfRecordType.FillPath:
                StrokeAndFillPath(stroke: false, fill: true);
                break;

            case EmfRecordType.StrokeAndFillPath:
                StrokeAndFillPath(stroke: true, fill: true);
                break;

            case EmfRecordType.StrokePath:
                StrokeAndFillPath(stroke: true, fill: false);
                break;

            case EmfRecordType.FlattenPath:
            case EmfRecordType.WidenPath:
                // Both rewrite the path into an equivalent one; flattening replaces curves with
                // lines and widening replaces the path with its own stroked outline. Neither
                // changes what a subsequent fill or stroke covers to within the flattening
                // tolerance, and both are left alone rather than approximated.
                break;

            case EmfRecordType.SelectClipPath:
                SelectClipPath((RegionCombineMode)U32());
                break;

            // ------------------------------------------------------ clipping
            case EmfRecordType.IntersectClipRect:
                _context.Clip = _context.Clip.Clone();
                _context.Clip.Intersect(MapRect(ReadRectExclusive()));
                break;

            case EmfRecordType.ExcludeClipRect:
            {
                DocRect excluded = MapRect(ReadRectExclusive());
                _context.Clip = _context.Clip.Clone();
                _context.Clip.Exclude(excluded);
                break;
            }

            case EmfRecordType.OffsetClipRgn:
            {
                int x = I32();
                int y = I32();
                (double dx, double dy) = _context.Mapping.MapSizeMm100(x, y);
                _context.Clip = _context.Clip.Translate(MetafileMapping.Emu(dx), MetafileMapping.Emu(dy));
                break;
            }

            case EmfRecordType.ExtSelectClipRgn:
                ExtSelectClipRegion(end);
                break;

            case EmfRecordType.SetMetaRgn:
                // Folds the clip into the metaregion and resets the clip to everything; since
                // the two are only ever used intersected, keeping the clip is the same picture.
                break;

            // ------------------------------------------------------ drawing
            case EmfRecordType.MoveToEx:
            {
                int x = I32();
                int y = I32();
                _context.CurrentPosition = (x, y);
                if (_context.Path.IsRecording) _context.Path.MoveTo(Map(x, y));
                break;
            }

            case EmfRecordType.LineTo:
            {
                int x = I32();
                int y = I32();

                if (_context.Path.IsRecording)
                {
                    _context.Path.LineTo(Map(x, y));
                }
                else
                {
                    (double px, double py) = _context.CurrentPosition;
                    _painter.Stroke(new GraphicsPath().MoveTo(Map(px, py)).LineTo(Map(x, y)));
                }

                _context.CurrentPosition = (x, y);
                break;
            }

            case EmfRecordType.SetPixelV:
            {
                int x = I32();
                int y = I32();
                DrawPixel(x, y, ReadColour());
                break;
            }

            case EmfRecordType.Rectangle:
            {
                // Not made inclusive: emfio builds this polygon from the stated corners
                // (emfreader.cxx:1439-1443) where the arc and clip records subtract one, and the
                // difference is a whole logical unit on two edges.
                (int l, int t, int r, int b) = ReadRect();
                Shape(MetafileGeometry.Rectangle(_context.Mapping.MapRect(l, t, r, b)));
                break;
            }

            case EmfRecordType.RoundRect:
            {
                (int l, int t, int r, int b) = ReadRectExclusive();
                uint width = U32();
                uint height = U32();
                Shape(MetafileGeometry.RoundRectangle(
                    MapRect((l, t, r, b)),
                    _context.Mapping.MapWidth(width >> 1),
                    _context.Mapping.MapWidth(height >> 1)));
                break;
            }

            case EmfRecordType.Ellipse:
            {
                (int l, int t, int r, int b) = ReadRect();
                Shape(MetafileGeometry.Ellipse(_context.Mapping.MapRect(l, t, r, b)));
                break;
            }

            case EmfRecordType.Arc:
            case EmfRecordType.ArcTo:
            case EmfRecordType.Chord:
            case EmfRecordType.Pie:
                Arc(type);
                break;

            case EmfRecordType.AngleArc:
                AngleArc();
                break;

            case EmfRecordType.Polygon:
                Polygon(ReadPolygon(end, sixteen: false, continueFrom: false));
                break;

            case EmfRecordType.Polygon16:
                Polygon(ReadPolygon(end, sixteen: true, continueFrom: false));
                break;

            case EmfRecordType.Polyline:
                Polyline(ReadPolygon(end, sixteen: false, continueFrom: false), continueFrom: false);
                break;

            case EmfRecordType.Polyline16:
                Polyline(ReadPolygon(end, sixteen: true, continueFrom: false), continueFrom: false);
                break;

            case EmfRecordType.PolylineTo:
                Polyline(ReadPolygon(end, sixteen: false, continueFrom: true), continueFrom: true);
                break;

            case EmfRecordType.PolylineTo16:
                Polyline(ReadPolygon(end, sixteen: true, continueFrom: true), continueFrom: true);
                break;

            case EmfRecordType.PolyBezier:
                Bezier(ReadPolygon(end, sixteen: false, continueFrom: false), continueFrom: false);
                break;

            case EmfRecordType.PolyBezier16:
                Bezier(ReadPolygon(end, sixteen: true, continueFrom: false), continueFrom: false);
                break;

            case EmfRecordType.PolyBezierTo:
                Bezier(ReadPolygon(end, sixteen: false, continueFrom: true), continueFrom: true);
                break;

            case EmfRecordType.PolyBezierTo16:
                Bezier(ReadPolygon(end, sixteen: true, continueFrom: true), continueFrom: true);
                break;

            case EmfRecordType.PolyPolygon:
                PolyPolygon(end, sixteen: false, closed: true);
                break;

            case EmfRecordType.PolyPolygon16:
                PolyPolygon(end, sixteen: true, closed: true);
                break;

            case EmfRecordType.PolyPolyline:
                PolyPolygon(end, sixteen: false, closed: false);
                break;

            case EmfRecordType.PolyPolyline16:
                PolyPolygon(end, sixteen: true, closed: false);
                break;

            case EmfRecordType.PolyDraw:
                PolyDraw(end, sixteen: false);
                break;

            case EmfRecordType.PolyDraw16:
                PolyDraw(end, sixteen: true);
                break;

            case EmfRecordType.GradientFill:
                GradientFill(end);
                break;

            // ------------------------------------------------------ regions
            case EmfRecordType.FillRgn:
            case EmfRecordType.PaintRgn:
                FillRegion(type == EmfRecordType.FillRgn, end);
                break;

            // ------------------------------------------------------ bitmaps
            case EmfRecordType.BitBlt:
            case EmfRecordType.StretchBlt:
                Blt(type, start, end);
                break;

            case EmfRecordType.StretchDIBits:
                StretchDIBits(start, end);
                break;

            case EmfRecordType.SetDIBitsToDevice:
                SetDIBitsToDevice(start, end);
                break;

            case EmfRecordType.AlphaBlend:
                AlphaBlend(start, end);
                break;

            case EmfRecordType.TransparentBlt:
            case EmfRecordType.TransparentDib:
                TransparentBlt(start, end);
                break;

            // ------------------------------------------------------ text
            case EmfRecordType.ExtTextOutA:
            case EmfRecordType.ExtTextOutW:
            case EmfRecordType.PolyTextOutA:
            case EmfRecordType.PolyTextOutW:
                ExtTextOut(type, start, end);
                break;

            case EmfRecordType.SmallTextOut:
                SmallTextOut(end);
                break;

            // ------------------------------------------------------ ignored
            case EmfRecordType.Header:
            case EmfRecordType.Eof:
            case EmfRecordType.Comment:
            case EmfRecordType.SetBrushOrgEx:
            case EmfRecordType.SetMapperFlags:
            case EmfRecordType.SetStretchBltMode:
            case EmfRecordType.SetColorAdjustment:
            case EmfRecordType.SelectPalette:
            case EmfRecordType.SetPaletteEntries:
            case EmfRecordType.ResizePalette:
            case EmfRecordType.RealizePalette:
            case EmfRecordType.SetIcmMode:
            case EmfRecordType.CreateColorSpace:
            case EmfRecordType.SetColorSpace:
            case EmfRecordType.DeleteColorSpace:
            case EmfRecordType.PixelFormat:
            case EmfRecordType.StartDoc:
            case EmfRecordType.ForceUfiMapping:
            case EmfRecordType.SetLinkedUfis:
            case EmfRecordType.SetTextJustification:
            case EmfRecordType.SetLayout:
            case EmfRecordType.GlsRecord:
            case EmfRecordType.GlsBoundedRecord:
            case EmfRecordType.DrawEscape:
            case EmfRecordType.ExtEscape:
            case EmfRecordType.NamedEscape:
            case EmfRecordType.ColorCorrectPalette:
            case EmfRecordType.SetIcmProfileA:
            case EmfRecordType.SetIcmProfileW:
                break;

            case EmfRecordType.MaskBlt:
            case EmfRecordType.PlgBlt:
            case EmfRecordType.FrameRgn:
            case EmfRecordType.InvertRgn:
            case EmfRecordType.ExtFloodFill:
                Warn("PL6031", $"An EMF used the {type} record, which Paperless does not draw.");
                break;

            default:
                Warn("PL6031", $"An EMF used an unknown record type {(uint)type}.");
                break;
        }
    }

    // ---------------------------------------------------------------- objects

    private void CreatePen()
    {
        uint handle = U32();
        if ((handle & StockObjectFlag) != 0) return;

        uint style = U32();
        int width = I32();
        Skip(4);                        // the pen's height, which GDI ignores
        Colour colour = ReadColour();

        Store(handle, Pen(style, width, colour, null));
    }

    /// <summary>
    /// Reads <c>EMR_EXTCREATEPEN</c>, the record that carries caps, joins and a dash array.
    /// </summary>
    /// <remarks>
    /// <b>Geometric and cosmetic pens are different quantities in the same field.</b> A
    /// geometric pen's width is in logical units and scales with the picture; a cosmetic pen's
    /// is in <em>device</em> units and [MS-EMF] 2.2.20 requires it to be 1 — a hairline, whatever
    /// the picture's scale. Mapping the stated width for both makes a cosmetic pen come out at
    /// one logical unit, which on a typical reference device is about a fortieth of a
    /// millimetre: invisible at print resolution rather than the thinnest visible line GDI draws.
    /// </remarks>
    private void ExtCreatePen(int end)
    {
        uint handle = U32();
        if ((handle & StockObjectFlag) != 0) return;

        Skip(4);                        // offBmi
        Skip(4);                        // cbBmi
        Skip(4);                        // offBits
        Skip(4);                        // cbBits
        uint style = U32();
        int width = I32();
        uint brushStyle = U32();
        Colour colour = ReadColour();
        int hatch = I32();
        uint entries = U32();

        if (_failed) return;

        // A hatched pen names its colour by alias rather than by value for four of the twelve
        // hatch styles ([MS-EMF] 2.1.17, and emfreader.cxx:1351-1360).
        colour = hatch switch
        {
            8 or 9 => _context.TextColour,
            10 or 11 => _context.BackgroundColour,
            _ => colour,
        };

        List<Length>? dashes = null;

        if ((style & PenStyle.StyleMask) == PenStyle.UserStyle
            && MetafileBudget.Plausible((int)Math.Min(entries, int.MaxValue), 4, end - _position)
            && entries > 0)
        {
            dashes = new List<Length>((int)entries);
            for (int i = 0; i < entries && !_failed; i++) dashes.Add(_context.Mapping.MapWidth(U32()));

            // An odd-length dash array repeats to an even one, because a dash pattern alternates
            // and a renderer given an odd count would swap ink and gap on every other repeat.
            if (dashes.Count % 2 != 0) dashes.AddRange(dashes);
        }

        Store(handle, Pen(style, width, colour, dashes));
    }

    private MetafilePen Pen(uint style, int width, Colour colour, IReadOnlyList<Length>? dashes)
    {
        const uint Geometric = 0x00010000;

        // A style word beyond PS_INSIDEFRAME is meaningless in the low nibble and emfio folds it
        // to cosmetic-solid rather than trusting it (emfreader.cxx:1323).
        if ((style & PenStyle.StyleMask) > PenStyle.InsideFrame && (style & PenStyle.StyleMask) != PenStyle.UserStyle)
        {
            style = PenStyle.Solid;
        }

        bool cosmetic = (style & Geometric) == 0;
        Length mapped = cosmetic && width <= 1 ? Length.Zero : _context.Mapping.MapWidth(width);

        return new MetafilePen(
            colour,
            mapped,
            style,
            dashes ?? MetafilePen.Dashes(style, _context.Mapping.MapWidth(Math.Abs(width) + 1)),
            (style & PenStyle.EndCapMask) switch
            {
                PenStyle.EndCapRound when width > 1 => LineCap.Round,
                PenStyle.EndCapSquare when width > 1 => LineCap.Square,
                _ => LineCap.Butt,
            },
            (style & PenStyle.JoinMask) switch
            {
                PenStyle.JoinRound => LineJoin.Round,
                PenStyle.JoinBevel => LineJoin.Bevel,
                _ => LineJoin.Miter,
            });
    }

    private void CreateBrush()
    {
        uint handle = U32();
        if ((handle & StockObjectFlag) != 0) return;

        BrushStyle style = (BrushStyle)U32();
        Colour colour = ReadColour();
        int hatch = I32();

        MetafileBrush brush = style switch
        {
            BrushStyle.Null => MetafileBrush.None,
            BrushStyle.Hatched when hatch is >= 0 and <= 5 => new MetafileBrush(
                BrushStyle.Hatched, colour, (HatchStyle)hatch),

            // Six of the twelve hatch styles are not hatches at all but named solid colours
            // ([MS-EMF] 2.1.17, and emfreader.cxx:1387-1401).
            BrushStyle.Hatched when hatch is 8 or 9 => new MetafileBrush(BrushStyle.Solid, _context.TextColour),
            BrushStyle.Hatched when hatch is 10 or 11 => new MetafileBrush(
                BrushStyle.Solid, _context.BackgroundColour),
            _ => new MetafileBrush(BrushStyle.Solid, colour),
        };

        Store(handle, brush);
    }

    private void CreateFont()
    {
        uint handle = U32();
        if ((handle & StockObjectFlag) != 0) return;

        int height = I32();
        int width = I32();
        int escapement = I32();
        Skip(4);                        // orientation, which GDI ties to the escapement here
        int weight = I32();
        byte italic = U8();
        byte underline = U8();
        byte strikeOut = U8();
        byte charSet = U8();
        Skip(4);                        // output and clipping precision, quality, pitch and family

        // The face name is 32 UTF-16 code units, NUL-padded rather than NUL-terminated when it
        // fills the field exactly.
        Span<char> name = stackalloc char[32];
        int length = 0;
        for (int i = 0; i < 32; i++)
        {
            char c = (char)U16();
            if (c == 0) continue;
            name[length++] = c;
        }

        if (_failed) return;

        Length size = height == 0
            ? MetafileFont.Default.Size
            : _context.Mapping.MapSize(0, Math.Abs(height), rotate: false).Height;

        if (size <= Length.Zero) size = MetafileFont.Default.Size;

        Store(handle, new MetafileFont(
            new string(name[..length]),
            size,
            weight is > 0 and <= 1000 ? weight : 400,
            italic != 0,
            underline != 0,
            strikeOut != 0,
            escapement,
            charSet,
            width != 0 && height != 0 ? Math.Abs((double)width / height) : 0));
    }

    private void CreatePalette(int end)
    {
        uint handle = U32();
        if ((handle & StockObjectFlag) != 0) return;

        Skip(2);                        // version
        int count = U16();

        if (!MetafileBudget.Plausible(count, 4, end - _position))
        {
            Store(handle, UnsupportedGraphicsObject.Instance);
            return;
        }

        List<Colour> colours = new(count);
        for (int i = 0; i < count && !_failed; i++)
        {
            uint entry = U32();
            colours.Add(new Colour((byte)entry, (byte)(entry >> 8), (byte)(entry >> 16)));
        }

        Store(handle, new MetafilePalette(colours));
    }

    private void CreateDibBrush(int start, int end)
    {
        uint handle = U32();
        if ((handle & StockObjectFlag) != 0) return;

        Skip(4);                        // colour usage
        uint headerOffset = U32();
        uint headerBytes = U32();
        uint bitsOffset = U32();
        uint bitsBytes = U32();

        if (Dib(start, end, headerOffset, headerBytes, bitsOffset, bitsBytes) is not { } tile)
        {
            Store(handle, new MetafileBrush(BrushStyle.Solid, _context.TextColour));
            Warn("PL6031", "An EMF's pattern brush could not be read; it was filled flat instead.");
            return;
        }

        // A pattern brush tiles in device pixels and a display list has none; 96 to the inch is
        // the nominal pixel the rest of Paperless assumes.
        Store(handle, new MetafileBrush(
            BrushStyle.DibPattern,
            _context.TextColour,
            HatchStyle.Horizontal,
            tile.Image,
            new DocSize(Length.FromPixels(tile.Width, 96), Length.FromPixels(tile.Height, 96))));
    }

    private void Store(uint handle, GraphicsObject value) => _context.Objects.Set((int)(handle & 0xFFFF), value);

    // ---------------------------------------------------------------- drawing

    private void Shape(GraphicsPath path)
    {
        if (_context.Path.IsRecording) _context.Path.Add(path);
        else _painter.FillAndStroke(path);
    }

    /// <summary>
    /// Draws the recorded path, which is what <c>FillPath</c>, <c>StrokePath</c> and
    /// <c>StrokeAndFillPath</c> all end at.
    /// </summary>
    /// <remarks>
    /// The path is cleared afterwards, as <c>MtfTools::StrokeAndFillPath</c> does
    /// (<c>mtftools.cxx:1487</c>): GDI discards the path when it is drawn, so a file that draws
    /// the same path twice states it twice, and keeping it would double every such figure.
    /// </remarks>
    private void StrokeAndFillPath(bool stroke, bool fill)
    {
        GraphicsPath path = _context.Path.Path;

        if (!_context.Path.IsEmpty)
        {
            if (fill) _painter.Fill(path);
            if (stroke) _painter.Stroke(path);
        }

        _context.Path.Clear();
    }

    private void DrawPixel(int x, int y, Colour colour)
    {
        DocPoint point = Map(x, y);

        MetafilePen pen = _context.Pen;
        _context.Pen = new MetafilePen(colour, Length.Zero);
        _painter.Stroke(new GraphicsPath()
            .MoveTo(point)
            .LineTo(new DocPoint(point.X + Length.FromEmu(1), point.Y)));
        _context.Pen = pen;
    }

    private void Polygon(List<DocPoint>? points)
    {
        if (points is not { Count: > 0 }) return;

        if (_context.Path.IsRecording) _context.Path.AddPolygon(points);
        else _painter.FillAndStroke(MetafileGeometry.Polygon(points));
    }

    private void Polyline(List<DocPoint>? points, bool continueFrom)
    {
        if (points is not { Count: > 0 }) return;

        if (_context.Path.IsRecording)
        {
            if (continueFrom) foreach (DocPoint point in points) _context.Path.LineTo(point);
            else
            {
                _context.Path.MoveTo(points[0]);
                for (int i = 1; i < points.Count; i++) _context.Path.LineTo(points[i]);
            }
        }
        else
        {
            List<DocPoint> whole = points;

            if (continueFrom)
            {
                whole = [Map(_context.CurrentPosition.X, _context.CurrentPosition.Y), .. points];
            }

            _painter.Stroke(MetafileGeometry.Polyline(whole));
        }

        Advance(points);
    }

    /// <summary>
    /// Draws or records a run of cubic Béziers.
    /// </summary>
    /// <remarks>
    /// The point count is one start plus a multiple of three, and a record that breaks that is
    /// malformed rather than merely short — <c>MtfTools::DrawPolyBezier</c> refuses it outright
    /// (<c>mtftools.cxx:1948-1954</c>) because there is no way to tell which of the trailing
    /// points is the odd one out.
    /// </remarks>
    private void Bezier(List<DocPoint>? points, bool continueFrom)
    {
        if (points is not { Count: > 0 }) return;

        List<DocPoint> whole = continueFrom
            ? [Map(_context.CurrentPosition.X, _context.CurrentPosition.Y), .. points]
            : points;

        if (whole.Count < 4 || (whole.Count - 4) % 3 != 0)
        {
            Warn("PL6031", "An EMF's Bézier record did not hold a whole number of curves.");
            return;
        }

        if (_context.Path.IsRecording)
        {
            _context.Path.MoveTo(whole[0]);
            for (int i = 1; i + 2 < whole.Count; i += 3)
            {
                _context.Path.CubicTo(whole[i - 1], whole[i], whole[i + 1], whole[i + 2]);
            }
        }
        else
        {
            GraphicsPath path = new();
            path.MoveTo(whole[0]);
            for (int i = 1; i + 2 < whole.Count; i += 3) path.CubicTo(whole[i], whole[i + 1], whole[i + 2]);
            _painter.Stroke(path);
        }

        Advance(points);
    }

    private void Advance(List<DocPoint> points)
    {
        // The current position follows the drawing, and it is kept in logical units — so it has
        // to come back through the mapping rather than be taken from the mapped point.
        if (points.Count == 0) return;

        DocPoint last = points[^1];
        _context.CurrentPosition = Unmap(last);
    }

    private void PolyPolygon(int end, bool sixteen, bool closed)
    {
        Skip(16);                       // bounds, which the records themselves supply
        int polygons = (int)U32();
        int total = (int)U32();

        int each = sixteen ? 4 : 8;

        if (_failed
            || !MetafileBudget.Plausible(polygons, 4, end - _position)
            || polygons <= 0)
        {
            return;
        }

        int[] counts = new int[polygons];
        long sum = 0;
        for (int i = 0; i < polygons && !_failed; i++)
        {
            counts[i] = (int)U32();
            sum += counts[i];
        }

        if (_failed || sum <= 0 || sum > total + polygons) sum = Math.Min(sum, int.MaxValue);
        if (!MetafileBudget.Plausible((int)Math.Min(sum, int.MaxValue), each, end - _position)) return;
        if (!_budget.ChargeSegments((int)Math.Min(sum, int.MaxValue))) return;

        GraphicsPath path = new();

        for (int i = 0; i < polygons && !_failed; i++)
        {
            List<DocPoint> points = new(Math.Max(counts[i], 0));
            for (int j = 0; j < counts[i] && !_failed; j++) points.Add(ReadPoint(sixteen));

            if (points.Count == 0) continue;

            if (closed)
            {
                MetafileGeometry.AddPolygon(path, points);
            }
            else if (_context.Path.IsRecording)
            {
                _context.Path.MoveTo(points[0]);
                for (int j = 1; j < points.Count; j++) _context.Path.LineTo(points[j]);
            }
            else
            {
                _painter.Stroke(MetafileGeometry.Polyline(points));
            }
        }

        if (!closed) return;

        if (_context.Path.IsRecording) _context.Path.Add(path);
        else _painter.FillAndStroke(path);
    }

    /// <summary>
    /// Reads <c>EMR_POLYDRAW</c>, whose points carry their own verbs.
    /// </summary>
    /// <remarks>
    /// The only record in either format where the geometry is stated as an instruction stream
    /// rather than as a shape, and the only one that can close a figure in the middle of itself.
    /// It always draws through the path, because a mixture of moves, lines and curves is what a
    /// path is (<c>emfreader.cxx:1037-1064</c>).
    /// </remarks>
    private void PolyDraw(int end, bool sixteen)
    {
        const byte CloseFigure = 0x01;
        const byte LineTo = 0x02;
        const byte BezierTo = 0x04;
        const byte MoveTo = 0x06;

        Skip(16);                       // bounds
        int count = (int)U32();
        int each = sixteen ? 4 : 8;

        if (_failed || !MetafileBudget.Plausible(count, each + 1, end - _position) || count <= 0) return;
        if (!_budget.ChargeSegments(count)) return;

        DocPoint[] points = new DocPoint[count];
        for (int i = 0; i < count && !_failed; i++) points[i] = ReadPoint(sixteen);

        byte[] verbs = new byte[count];
        for (int i = 0; i < count && !_failed; i++) verbs[i] = U8();
        if (_failed) return;

        bool recording = _context.Path.IsRecording;
        MetafilePath path = recording ? _context.Path : new MetafilePath();

        DocPoint current = Map(_context.CurrentPosition.X, _context.CurrentPosition.Y);

        for (int i = 0; i < count; i++)
        {
            if (verbs[i] == MoveTo)
            {
                path.MoveTo(points[i]);
                current = points[i];
            }
            else if ((verbs[i] & BezierTo) != 0)
            {
                if (i + 2 >= count) break;

                path.CubicTo(current, points[i], points[i + 1], points[i + 2]);
                current = points[i + 2];
                i += 2;
                if ((verbs[i] & CloseFigure) != 0) path.Close();
            }
            else if ((verbs[i] & LineTo) != 0)
            {
                path.LineTo(points[i]);
                current = points[i];
                if ((verbs[i] & CloseFigure) != 0) path.Close();
            }
        }

        _context.CurrentPosition = Unmap(current);

        if (recording) return;

        _painter.Stroke(path.Path);
    }

    private void Arc(EmfRecordType type)
    {
        (int l, int t, int r, int b) = ReadRectExclusive();
        int startX = I32();
        int startY = I32();
        int endX = I32();
        int endY = I32();

        if (_failed) return;

        DocRect rect = MapRect((l, t, r, b));
        DocPoint start = Map(startX, startY);
        DocPoint finish = Map(endX, endY);

        MetafileGeometry.ArcClosure closure = type switch
        {
            EmfRecordType.Pie => MetafileGeometry.ArcClosure.Pie,
            EmfRecordType.Chord => MetafileGeometry.ArcClosure.Chord,
            _ => MetafileGeometry.ArcClosure.Open,
        };

        if (type == EmfRecordType.Pie && startX == endX && startY == endY)
        {
            // i73608: GDI draws a full ellipse when a pie's two radials coincide.
            Shape(MetafileGeometry.Ellipse(rect));
            return;
        }

        GraphicsPath path = MetafileGeometry.Arc(
            rect, start, finish, closure, _context.IsArcDirectionClockwise);

        if (closure == MetafileGeometry.ArcClosure.Open)
        {
            if (_context.Path.IsRecording) _context.Path.Add(path);
            else _painter.Stroke(path);
        }
        else
        {
            Shape(path);
        }

        if (type == EmfRecordType.ArcTo) _context.CurrentPosition = (endX, endY);
    }

    /// <summary>
    /// Reads <c>EMR_ANGLEARC</c>, which names an arc by a centre, a radius and two angles.
    /// </summary>
    /// <remarks>
    /// The only arc record that states angles directly, so it needs none of the ray-to-parameter
    /// conversion the others do. It is built in logical coordinates and every point — control
    /// points included — is mapped afterwards, which is exact: a cubic Bézier's image under an
    /// affine map is the cubic through its mapped control points.
    /// </remarks>
    private void AngleArc()
    {
        int centreX = I32();
        int centreY = I32();
        long radius = U32();
        float startDegrees = Single();
        float sweepDegrees = Single();

        if (_failed || radius <= 0 || !float.IsFinite(startDegrees) || !float.IsFinite(sweepDegrees)) return;

        double from = startDegrees * Math.PI / 180.0;
        double sweep = Math.Clamp(sweepDegrees, -360.0, 360.0) * Math.PI / 180.0;
        if (_context.IsArcDirectionClockwise) sweep = -Math.Abs(sweep);

        int segments = Math.Max(1, (int)Math.Ceiling(Math.Abs(sweep) / (Math.PI / 2)));
        double step = sweep / segments;
        double alpha = 4.0 / 3.0 * Math.Tan(step / 4.0);

        GraphicsPath path = new();

        // AngleArc draws a line from the current position to the start of the arc before the arc
        // itself ([MS-EMF] 2.3.5.2, and emfreader.cxx:1486-1488).
        path.MoveTo(Map(_context.CurrentPosition.X, _context.CurrentPosition.Y));
        path.LineTo(Map(centreX + (radius * Math.Cos(from)), centreY - (radius * Math.Sin(from))));

        double angle = from;
        for (int i = 0; i < segments; i++)
        {
            double next = angle + step;

            path.CubicTo(
                Map(
                    centreX + (radius * Math.Cos(angle)) - (alpha * radius * Math.Sin(angle)),
                    centreY - (radius * Math.Sin(angle)) - (alpha * radius * Math.Cos(angle))),
                Map(
                    centreX + (radius * Math.Cos(next)) + (alpha * radius * Math.Sin(next)),
                    centreY - (radius * Math.Sin(next)) + (alpha * radius * Math.Cos(next))),
                Map(centreX + (radius * Math.Cos(next)), centreY - (radius * Math.Sin(next))));

            angle = next;
        }

        if (_context.Path.IsRecording) _context.Path.Add(path);
        else _painter.Stroke(path);

        _context.CurrentPosition = (
            centreX + (radius * Math.Cos(angle)),
            centreY - (radius * Math.Sin(angle)));
    }

    /// <summary>
    /// Reads <c>EMR_GRADIENTFILL</c>, the one drawing record that names its own colours.
    /// </summary>
    /// <remarks>
    /// The rectangle modes map straight onto a two-stop linear <c>GradientPaint</c>. The triangle
    /// mode does not: a Gouraud triangle interpolates three colours barycentrically and no paint
    /// in the drawing IR can express that. It is filled with the mean of its three vertices —
    /// which is the right average colour and the wrong distribution — rather than left as a hole,
    /// because a missing facet in a shaded drawing reads as a defect while a flat one reads as
    /// coarse shading. <c>emfio</c> draws nothing at all here
    /// (<c>emfreader.cxx:2548-2551</c>).
    /// </remarks>
    private void GradientFill(int end)
    {
        Skip(16);                       // bounds
        int vertices = (int)U32();
        int primitives = (int)U32();
        uint mode = U32();

        if (_failed
            || vertices is <= 0 or > (256 * 1024)
            || primitives is <= 0 or > (256 * 1024)
            || ((long)vertices * 16) + ((long)primitives * 8) > end - _position)
        {
            return;
        }

        (int X, int Y, Colour Colour)[] points = new (int, int, Colour)[vertices];

        for (int i = 0; i < vertices && !_failed; i++)
        {
            int x = I32();
            int y = I32();

            // The channels are sixteen bits each and only the high byte carries anything: GDI
            // stores 0xFF00 for full intensity, so a decoder that takes the low byte draws black.
            byte r = (byte)(U16() >> 8);
            byte g = (byte)(U16() >> 8);
            byte b = (byte)(U16() >> 8);
            Skip(2);                    // alpha, which the rectangle modes ignore

            points[i] = (x, y, new Colour(r, g, b));
        }

        if (_failed) return;

        bool triangles = mode == 0x02;
        int indices = triangles ? 3 : 2;

        for (int i = 0; i < primitives && !_failed; i++)
        {
            int a = (int)U32();
            int b = (int)U32();
            int c = triangles ? (int)U32() : 0;

            if (a < 0 || a >= vertices || b < 0 || b >= vertices) continue;
            if (triangles && (c < 0 || c >= vertices)) continue;

            if (triangles)
            {
                Colour mean = new(
                    (byte)((points[a].Colour.R + points[b].Colour.R + points[c].Colour.R) / 3),
                    (byte)((points[a].Colour.G + points[b].Colour.G + points[c].Colour.G) / 3),
                    (byte)((points[a].Colour.B + points[b].Colour.B + points[c].Colour.B) / 3));

                _painter.FillWith(
                    MetafileGeometry.Polygon(
                    [
                        Map(points[a].X, points[a].Y),
                        Map(points[b].X, points[b].Y),
                        Map(points[c].X, points[c].Y),
                    ]),
                    Paint.Solid(mean));

                Warn(
                    "PL6036",
                    "An EMF shaded a triangle between three colours, which the drawing model "
                        + "cannot express; it was filled with their average.");
                continue;
            }

            DocRect rect = MapRect((points[a].X, points[a].Y, points[b].X, points[b].Y));
            if (rect.IsEmpty) continue;

            // Mode 0 runs left to right and mode 1 top to bottom ([MS-EMF] 2.1.12).
            DocPoint from = new(rect.Left, rect.Top);
            DocPoint to = mode == 0
                ? new DocPoint(rect.Right, rect.Top)
                : new DocPoint(rect.Left, rect.Bottom);

            _painter.FillWith(
                MetafileGeometry.Rectangle(rect),
                new GradientPaint(
                    GradientKind.Linear,
                    [new GradientStop(0, points[a].Colour), new GradientStop(1, points[b].Colour)],
                    from,
                    to,
                    AffineTransform.Identity));
        }

        _ = indices;
    }

    // ---------------------------------------------------------------- regions

    /// <summary>
    /// Reads a <c>RegionData</c> structure into one path of disjoint rectangles.
    /// </summary>
    /// <remarks>
    /// <b>A GDI region is a scan list, and disjoint subpaths are their own union under either
    /// fill rule</b> — which is why a region can be expressed exactly by a sink that only
    /// intersects. The window origin is added to every rectangle before it is mapped, which
    /// cancels the subtraction the mapping is about to do: region coordinates are already
    /// window-relative (<c>emfreader.cxx:388-391</c>).
    /// </remarks>
    private (GraphicsPath Path, List<DocRect> Rectangles)? ReadRegion(int end)
    {
        if (end - _position < 32) return null;

        Skip(4);                        // header size
        uint kind = U32();
        int count = (int)U32();
        Skip(4);                        // region size in bytes
        Skip(16);                       // the region's own bounds

        const uint Rectangles = 1;
        if (kind != Rectangles || count <= 0) return null;
        if (!MetafileBudget.Plausible(count, 16, end - _position)) return null;
        if (!_budget.ChargeSegments(count * 4)) return null;

        GraphicsPath path = new();
        List<DocRect> rectangles = new(count);

        bool scales = _context.Mapping.ScalesRatherThanMaps;

        for (int i = 0; i < count && !_failed; i++)
        {
            int left = I32() + _context.Mapping.WindowOriginX;
            int top = I32() + _context.Mapping.WindowOriginY;
            int right = I32() + _context.Mapping.WindowOriginX;
            int bottom = I32() + _context.Mapping.WindowOriginY;

            DocRect rect = scales
                ? DocRect.FromCorners(
                    _context.Mapping.ScalePoint(left, top),
                    _context.Mapping.ScalePoint(right, bottom))
                : MapRect((left, top, right, bottom));

            rectangles.Add(rect);
            MetafileGeometry.AddRectangle(path, rect);
        }

        return _failed ? null : (path, rectangles);
    }

    private void ExtSelectClipRegion(int end)
    {
        uint regionBytes = U32();
        RegionCombineMode mode = (RegionCombineMode)U32();

        if (_failed) return;

        _context.Clip = _context.Clip.Clone();

        if (regionBytes == 0)
        {
            // A null region with RGN_COPY resets the clip, which is what the Win32 API does even
            // though [MS-EMF] 2.3.2.2 says the region data must be ignored
            // (emfreader.cxx:1586-1600).
            if (mode == RegionCombineMode.Copy) _context.Clip.Reset();
            return;
        }

        if (ReadRegion(end) is not { } region)
        {
            _context.Clip.MarkUnsupported();
            return;
        }

        Combine(region.Path, region.Rectangles, mode);
    }

    private void SelectClipPath(RegionCombineMode mode)
    {
        if (_context.Path.IsEmpty) return;

        _context.Clip = _context.Clip.Clone();
        Combine(_context.Path.Path, null, mode);
    }

    private void Combine(GraphicsPath path, List<DocRect>? rectangles, RegionCombineMode mode)
    {
        switch (mode)
        {
            case RegionCombineMode.Copy:
                if (rectangles is not null) _context.Clip.Replace(rectangles);
                else _context.Clip.Replace(path, _context.FillRule);
                break;

            // A multi-rectangle region intersects as rectangles rather than as one path of
            // disjoint subpaths, though both are exact, because only the rectangle form leaves
            // the clip rectangular — and a later RGN_OR or RGN_XOR is exact only if it is.
            case RegionCombineMode.And:
                if (rectangles is not null) _context.Clip.Intersect(rectangles);
                else _context.Clip.Intersect(path, _context.FillRule);
                break;

            case RegionCombineMode.Diff:
                if (rectangles is not null) _context.Clip.Exclude(rectangles);
                else _context.Clip.MarkUnsupported();
                break;

            case RegionCombineMode.Or:
                if (rectangles is not null) _context.Clip.Union(rectangles);
                else _context.Clip.MarkUnsupported();
                break;

            case RegionCombineMode.Xor:
                if (rectangles is not null) _context.Clip.SymmetricDifference(rectangles);
                else _context.Clip.MarkUnsupported();
                break;

            default:
                // An arbitrary path as the operand of a union, a symmetric difference or a
                // complement is the one case left, and it needs a general polygon boolean.
                // Leaving the clip alone draws too much rather than too little.
                _context.Clip.MarkUnsupported();
                break;
        }
    }

    private void FillRegion(bool withBrush, int end)
    {
        Skip(16);                       // bounds
        Skip(4);                        // region data size
        uint handle = withBrush ? U32() : 0;

        if (ReadRegion(end) is not { } region) return;

        if (!withBrush)
        {
            _painter.Fill(region.Path);
            return;
        }

        if (_context.Objects[(int)(handle & 0xFFFF)] is not MetafileBrush brush) return;

        MetafileBrush previous = _context.Brush;
        bool selected = _context.IsBrushSelected;
        _context.Brush = brush;
        _context.IsBrushSelected = true;
        _painter.Fill(region.Path);
        _context.Brush = previous;
        _context.IsBrushSelected = selected;
    }

    // ---------------------------------------------------------------- bitmaps

    private void Blt(EmfRecordType type, int start, int end)
    {
        Skip(16);                       // bounds
        int destX = I32();
        int destY = I32();
        int destWidth = I32();
        int destHeight = I32();
        uint rop = U32();
        int sourceX = I32();
        int sourceY = I32();
        Skip(24);                       // the source's own transform, which GDI applies to the bits
        Skip(4);                        // background colour
        Skip(4);                        // colour usage
        uint headerOffset = U32();
        uint headerBytes = U32();
        uint bitsOffset = U32();
        uint bitsBytes = U32();

        int sourceWidth = 0;
        int sourceHeight = 0;

        if (type == EmfRecordType.StretchBlt)
        {
            sourceWidth = I32();
            sourceHeight = I32();
        }

        if (_failed || destWidth == 0 || destHeight == 0) return;

        DocRect destination = MapRect((destX, destY, destX + destWidth, destY + destHeight));

        if (headerBytes == 0 || bitsBytes == 0)
        {
            // No bitmap at all: the operation combines the destination with the brush, which a
            // display list cannot read back. BLACKNESS and WHITENESS are the two that need no
            // source and are honoured exactly.
            PatternBlt(destination, rop);
            return;
        }

        if (Dib(start, end, headerOffset, headerBytes, bitsOffset, bitsBytes) is not { } bitmap)
        {
            Warn("PL6031", "An EMF carried a bitmap in a form Paperless cannot read.");
            return;
        }

        Queue(new PendingBlit(
            Payload(start, headerOffset, headerBytes, bitsOffset, bitsBytes),
            bitmap,
            destination,
            rop,
            (sourceX, sourceY, sourceWidth, sourceHeight)));
    }

    private void StretchDIBits(int start, int end)
    {
        Skip(16);                       // bounds
        int destX = I32();
        int destY = I32();
        int sourceX = I32();
        int sourceY = I32();
        int sourceWidth = I32();
        int sourceHeight = I32();
        uint headerOffset = U32();
        uint headerBytes = U32();
        uint bitsOffset = U32();
        uint bitsBytes = U32();
        Skip(4);                        // colour usage
        uint rop = U32();
        int destWidth = I32();
        int destHeight = I32();

        if (_failed || destWidth == 0 || destHeight == 0) return;

        DocRect destination = MapRect((destX, destY, destX + destWidth, destY + destHeight));

        if (headerBytes == 0 || bitsBytes == 0)
        {
            PatternBlt(destination, rop);
            return;
        }

        if (Dib(start, end, headerOffset, headerBytes, bitsOffset, bitsBytes) is not { } bitmap)
        {
            Warn("PL6031", "An EMF carried a bitmap in a form Paperless cannot read.");
            return;
        }

        Queue(new PendingBlit(
            Payload(start, headerOffset, headerBytes, bitsOffset, bitsBytes),
            bitmap,
            destination,
            rop,
            (sourceX, sourceY, sourceWidth, sourceHeight)));
    }

    /// <summary>
    /// Reads <c>EMR_SETDIBITSTODEVICE</c>, which <c>emfio</c> does not implement.
    /// </summary>
    /// <remarks>
    /// A one-to-one placement with no scaling: the destination extent is the source extent, and
    /// the record additionally names a band of scan lines. Producers that write it are usually
    /// placing a scanned image, so leaving it out means leaving out the picture.
    /// </remarks>
    private void SetDIBitsToDevice(int start, int end)
    {
        Skip(16);                       // bounds
        int destX = I32();
        int destY = I32();
        int sourceX = I32();
        int sourceY = I32();
        int sourceWidth = I32();
        int sourceHeight = I32();
        uint headerOffset = U32();
        uint headerBytes = U32();
        uint bitsOffset = U32();
        uint bitsBytes = U32();
        Skip(4);                        // colour usage
        Skip(4);                        // first scan line
        Skip(4);                        // scan line count

        if (_failed || sourceWidth <= 0 || sourceHeight <= 0) return;
        if (headerBytes == 0 || bitsBytes == 0) return;

        if (Dib(start, end, headerOffset, headerBytes, bitsOffset, bitsBytes) is not { } bitmap)
        {
            Warn("PL6031", "An EMF carried a bitmap in a form Paperless cannot read.");
            return;
        }

        Queue(new PendingBlit(
            Payload(start, headerOffset, headerBytes, bitsOffset, bitsBytes),
            bitmap,
            MapRect((destX, destY, destX + sourceWidth, destY + sourceHeight)),
            RasterOperations.SourceCopy,
            (sourceX, sourceY, sourceWidth, sourceHeight)));
    }

    /// <summary>
    /// Reads <c>EMR_ALPHABLEND</c>, the one record that states transparency outright.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two independent kinds of transparency, and they are handled differently on purpose. A
    /// constant source alpha is a uniform opacity, which the sink already takes — so it costs no
    /// decode at all. <c>AC_SRC_ALPHA</c> means the DIB's fourth byte is a real alpha channel,
    /// and that does need pixels: the byte is normally unused and taken as opaque, so nothing
    /// downstream would read it.
    /// </para>
    /// <para>
    /// <c>emfio</c> rebuilds the DIB as a V5 header and hands it to a bitmap decoder
    /// (<c>emfreader.cxx:1645-1714</c>); here the pixels are read straight, which needs no codec
    /// because an uncompressed DIB is not encoded.
    /// </para>
    /// </remarks>
    private void AlphaBlend(int start, int end)
    {
        Skip(16);                       // bounds
        int destX = I32();
        int destY = I32();
        int destWidth = I32();
        int destHeight = I32();
        Skip(2);                        // blend operation and flags, of which only one is defined
        byte constantAlpha = U8();
        byte alphaFormat = U8();
        int sourceX = I32();
        int sourceY = I32();
        Skip(24);                       // the source's own transform
        Skip(4);                        // background colour
        Skip(4);                        // colour usage
        uint headerOffset = U32();
        uint headerBytes = U32();
        uint bitsOffset = U32();
        uint bitsBytes = U32();
        int sourceWidth = I32();
        int sourceHeight = I32();

        if (_failed || destWidth == 0 || destHeight == 0 || headerBytes == 0 || bitsBytes == 0) return;

        FlushBlit();

        // A destination extent of n covers n+1 units here, unlike every other blit record
        // (emfreader.cxx:1639).
        DocRect destination = MapRect((destX, destY, destX + destWidth + 1, destY + destHeight + 1));
        ReadOnlyMemory<byte> payload = Payload(start, headerOffset, headerBytes, bitsOffset, bitsBytes);

        const byte SourceAlpha = 0x01;
        double opacity = constantAlpha / 255.0;

        if (alphaFormat != SourceAlpha)
        {
            if (Dib(start, end, headerOffset, headerBytes, bitsOffset, bitsBytes) is not { } plain) return;
            Place(plain.Image, plain.Width, plain.Height, destination, (sourceX, sourceY, sourceWidth, sourceHeight), opacity);
            return;
        }

        if (DeviceIndependentBitmap.ReadPixels(payload.Span, alphaFromUnusedByte: true) is not { } pixels)
        {
            Warn(
                "PL6033",
                "An EMF blended a compressed bitmap with an alpha channel; it was drawn opaque.");

            if (Dib(start, end, headerOffset, headerBytes, bitsOffset, bitsBytes) is not { } fallback) return;
            Place(fallback.Image, fallback.Width, fallback.Height, destination, (sourceX, sourceY, sourceWidth, sourceHeight), opacity);
            return;
        }

        Place(pixels.Image, pixels.Width, pixels.Height, destination, (sourceX, sourceY, sourceWidth, sourceHeight), opacity);
    }

    private void TransparentBlt(int start, int end)
    {
        Skip(16);                       // bounds
        int destX = I32();
        int destY = I32();
        int destWidth = I32();
        int destHeight = I32();
        Colour transparent = ReadColour();
        int sourceX = I32();
        int sourceY = I32();
        Skip(24);                       // the source's own transform
        Skip(4);                        // background colour
        Skip(4);                        // colour usage
        uint headerOffset = U32();
        uint headerBytes = U32();
        uint bitsOffset = U32();
        uint bitsBytes = U32();
        int sourceWidth = I32();
        int sourceHeight = I32();

        if (_failed || destWidth == 0 || destHeight == 0 || headerBytes == 0 || bitsBytes == 0) return;

        FlushBlit();

        DocRect destination = MapRect((destX, destY, destX + destWidth, destY + destHeight));
        ReadOnlyMemory<byte> payload = Payload(start, headerOffset, headerBytes, bitsOffset, bitsBytes);

        if (DeviceIndependentBitmap.ReadPixels(payload.Span) is not { } pixels)
        {
            Warn("PL6033", "An EMF knocked a colour out of a compressed bitmap; it was drawn opaque.");
            if (Dib(start, end, headerOffset, headerBytes, bitsOffset, bitsBytes) is not { } fallback) return;
            Place(fallback.Image, fallback.Width, fallback.Height, destination, (sourceX, sourceY, sourceWidth, sourceHeight));
            return;
        }

        Place(
            RasterOperations.KnockOut(pixels, transparent),
            pixels.Width,
            pixels.Height,
            destination,
            (sourceX, sourceY, sourceWidth, sourceHeight));
    }

    /// <summary>Holds one blit back, so a mask and its image can be seen as a pair.</summary>
    private void Queue(PendingBlit blit)
    {
        if (_pending is { } waiting)
        {
            if (waiting.PairsWith(blit, out bool invert) && Merge(waiting, blit, invert)) return;

            FlushBlit();
        }

        _pending = blit;
    }

    private bool Merge(PendingBlit mask, PendingBlit image, bool invertMask)
    {
        if (DeviceIndependentBitmap.ReadPixels(mask.Data.Span) is not { } maskPixels) return false;
        if (DeviceIndependentBitmap.ReadPixels(image.Data.Span) is not { } imagePixels) return false;

        _pending = null;

        Place(
            RasterOperations.Merge(imagePixels, maskPixels, invertMask),
            imagePixels.Width,
            imagePixels.Height,
            image.Destination,
            image.Source);

        return true;
    }

    private void FlushBlit()
    {
        if (_pending is not { } blit) return;

        _pending = null;

        if (blit.Operation is RasterOperations.Blackness or RasterOperations.Whiteness)
        {
            PatternBlt(blit.Destination, blit.Operation);
            return;
        }

        if (blit.Operation is RasterOperations.DestinationCopy) return;

        if (blit.Operation != RasterOperations.SourceCopy)
        {
            Warn(
                "PL6033",
                $"An EMF blitted with raster operation 0x{blit.Operation:X8}; the source was drawn "
                    + "without it.");
        }

        Place(blit.Bitmap.Image, blit.Bitmap.Width, blit.Bitmap.Height, blit.Destination, blit.Source);
    }

    private void Place(
        RasterImage image,
        int width,
        int height,
        DocRect destination,
        (int X, int Y, int Width, int Height) source,
        double opacity = 1.0)
    {
        if (!SourceRectangle.Crops(source, width, height))
        {
            _painter.DrawImage(image, destination, null, opacity);
            return;
        }

        _painter.DrawImage(
            image, SourceRectangle.Whole(destination, source, width, height), destination, opacity);
    }

    /// <summary>Fills a rectangle for the two operations that need no source bitmap.</summary>
    private void PatternBlt(DocRect destination, uint rop)
    {
        if (rop == RasterOperations.Blackness)
        {
            _painter.FillWith(MetafileGeometry.Rectangle(destination), Paint.Solid(Colour.Black));
            return;
        }

        if (rop == RasterOperations.Whiteness)
        {
            _painter.FillWith(MetafileGeometry.Rectangle(destination), Paint.Solid(Colour.White));
            return;
        }

        if (rop == RasterOperations.DestinationCopy) return;

        // PATCOPY and its relatives fill with the selected brush, which is expressible; the rest
        // combine with the destination and are not.
        const uint PatternCopy = 0x00F00021;
        if (rop == PatternCopy)
        {
            _painter.Fill(MetafileGeometry.Rectangle(destination));
            return;
        }

        Warn(
            "PL6033",
            $"An EMF blitted with no source bitmap under raster operation 0x{rop:X8}, which "
                + "Paperless cannot reproduce.");
    }

    /// <summary>The two pieces of a DIB the record points at, spliced back together.</summary>
    private ReadOnlyMemory<byte> Payload(
        int start,
        uint headerOffset,
        uint headerBytes,
        uint bitsOffset,
        uint bitsBytes)
    {
        ReadOnlySpan<byte> header = Piece(start, headerOffset, headerBytes);
        ReadOnlySpan<byte> bits = Piece(start, bitsOffset, bitsBytes);

        if (header.IsEmpty) return ReadOnlyMemory<byte>.Empty;

        byte[] joined = new byte[header.Length + bits.Length];
        header.CopyTo(joined);
        bits.CopyTo(joined.AsSpan(header.Length));
        return joined;
    }

    private DeviceIndependentBitmap.Result? Dib(
        int start,
        int end,
        uint headerOffset,
        uint headerBytes,
        uint bitsOffset,
        uint bitsBytes)
    {
        _ = end;
        return DeviceIndependentBitmap.ReadSplit(
            Piece(start, headerOffset, headerBytes), Piece(start, bitsOffset, bitsBytes));
    }

    private ReadOnlySpan<byte> Piece(int start, uint offset, uint length)
    {
        if (offset == 0 || length == 0) return default;
        if (offset > int.MaxValue || length > int.MaxValue) return default;

        long from = (long)start + offset;
        long to = from + length;

        if (from < 0 || from >= _bytes.Length || to > _bytes.Length) return default;

        return _bytes.AsSpan((int)from, (int)length);
    }

    // ---------------------------------------------------------------- text

    private void ExtTextOut(EmfRecordType type, int start, int end)
    {
        bool eightBit = type is EmfRecordType.ExtTextOutA or EmfRecordType.PolyTextOutA;
        bool poly = type is EmfRecordType.PolyTextOutA or EmfRecordType.PolyTextOutW;

        Skip(16);                       // bounds
        uint graphicsMode = U32();
        float scaleX = Single();
        float scaleY = Single();

        int runs = poly ? (int)U32() : 1;
        if (_failed || runs <= 0) return;

        // Only the first run of a poly-text record is drawn: the following runs each need their
        // own EmrText header, and the offsets are relative to the record rather than to the run,
        // so a producer that writes more than one is vanishingly rare. Reported rather than
        // guessed at.
        if (runs > 1)
        {
            Warn(
                "PL6031",
                $"An EMF drew {runs} text runs in one record; only the first was drawn.");
        }

        int x = I32();
        int y = I32();
        int characters = (int)U32();
        uint stringOffset = U32();
        uint options = U32();
        (int left, int top, int right, int bottom) = ReadRect();
        uint dxOffset = U32();

        if (_failed) return;

        BackgroundMode background = _context.BackgroundMode;
        DocRect rect = MapRect((left, top, right, bottom));

        if ((options & EtoNoRect) != 0) _context.BackgroundMode = BackgroundMode.Transparent;
        else if ((options & EtoOpaque) != 0) _painter.FillBackground(rect);

        TextAlignment alignment = (options & EtoRightToLeft) != 0
            ? _context.TextAlignment | TextAlignment.RightToLeftReading
            : _context.TextAlignment;

        if (characters > 0 && stringOffset > 0)
        {
            bool glyphs = (options & EtoGlyphIndex) != 0 && !eightBit;
            int each = glyphs || !eightBit ? 2 : 1;

            ReadOnlySpan<byte> data = Piece(start, stringOffset, (uint)(characters * each));
            List<Length>? advances = ReadAdvances(start, dxOffset, characters, options, end);

            bool clipped = (options & EtoClipped) != 0 && !rect.IsEmpty;
            MetafileClip saved = _context.Clip;

            if (clipped)
            {
                _context.Clip = _context.Clip.Clone();
                _context.Clip.Intersect(rect);
            }

            if (!data.IsEmpty)
            {
                if (glyphs) DrawGlyphs(Glyphs(data), x, y, alignment, advances, scaleX, scaleY, graphicsMode);
                else DrawText(Text(data, eightBit), x, y, alignment, advances, scaleX, scaleY, graphicsMode);
            }

            if (clipped) _context.Clip = saved;
        }

        _context.BackgroundMode = background;
    }

    private void SmallTextOut(int end)
    {
        int x = I32();
        int y = I32();
        int characters = (int)U32();
        uint options = U32();
        uint graphicsMode = U32();
        float scaleX = Single();
        float scaleY = Single();

        DocRect rect = default;
        if ((options & EtoNoRect) == 0)
        {
            (int left, int top, int right, int bottom) = ReadRect();
            rect = MapRect((left, top, right, bottom));
        }

        if (_failed || characters <= 0) return;

        bool eightBit = (options & EtoSmallChars) != 0;
        int bytes = characters * (eightBit ? 1 : 2);
        if (bytes > end - _position) return;

        BackgroundMode background = _context.BackgroundMode;
        if ((options & EtoNoRect) != 0) _context.BackgroundMode = BackgroundMode.Transparent;
        else if ((options & EtoOpaque) != 0) _painter.FillBackground(rect);

        TextAlignment alignment = (options & EtoRightToLeft) != 0
            ? _context.TextAlignment | TextAlignment.RightToLeftReading
            : _context.TextAlignment;

        ReadOnlySpan<byte> data = Span(_position + bytes);

        bool clipped = (options & EtoClipped) != 0 && !rect.IsEmpty;
        MetafileClip saved = _context.Clip;

        if (clipped)
        {
            _context.Clip = _context.Clip.Clone();
            _context.Clip.Intersect(rect);
        }

        DrawText(Text(data, eightBit), x, y, alignment, null, scaleX, scaleY, graphicsMode);

        if (clipped) _context.Clip = saved;
        _context.BackgroundMode = background;
    }

    private string Text(ReadOnlySpan<byte> data, bool eightBit)
    {
        if (eightBit) return MetafileTextEngine.Decode(data, _context.Font);

        Span<char> characters = data.Length / 2 <= 512
            ? stackalloc char[data.Length / 2]
            : new char[data.Length / 2];

        for (int i = 0; i < characters.Length; i++)
        {
            characters[i] = (char)BinaryPrimitives.ReadUInt16LittleEndian(data[(i * 2)..]);
        }

        int length = characters.Length;
        while (length > 0 && characters[length - 1] == '\0') length--;

        return new string(characters[..length]);
    }

    private static ushort[] Glyphs(ReadOnlySpan<byte> data)
    {
        ushort[] glyphs = new ushort[data.Length / 2];
        for (int i = 0; i < glyphs.Length; i++)
        {
            glyphs[i] = BinaryPrimitives.ReadUInt16LittleEndian(data[(i * 2)..]);
        }

        return glyphs;
    }

    /// <summary>
    /// Reads the DX array and maps it, keeping the running sum in logical units.
    /// </summary>
    /// <remarks>
    /// <b>Honour the array rather than re-measuring.</b> It is how the producer recorded the
    /// result of its own text layout, and a decoder that measures the string again substitutes
    /// its own — which drifts from what the file intended, glyph by glyph, and by the end of a
    /// long label is visibly wrong. The sum is accumulated before mapping and each advance taken
    /// as the difference of two mapped sums, because mapping fifty advances separately rounds
    /// fifty times (<c>MtfTools::DrawText</c>, <c>mtftools.cxx:1988-2008</c>).
    /// </remarks>
    private List<Length>? ReadAdvances(int start, uint offset, int count, uint options, int end)
    {
        if (offset == 0 || count <= 0) return null;

        int each = (options & EtoPerGlyphY) != 0 ? 8 : 4;
        ReadOnlySpan<byte> data = Piece(start, offset, (uint)Math.Min((long)count * each, end));

        if (data.Length < count * each) return null;

        if ((options & EtoPerGlyphY) != 0)
        {
            Warn(
                "PL6031",
                "An EMF stated per-glyph vertical offsets; the horizontal advances were honoured "
                    + "and the vertical ones were not.");
        }

        List<Length> advances = new(count);
        double sum = 0;
        Length previous = Length.Zero;

        for (int i = 0; i < count; i++)
        {
            sum += BinaryPrimitives.ReadInt32LittleEndian(data[(i * each)..]);

            (double dx, double dy) = _context.Mapping.MapSizeMm100(sum, 0);
            Length mapped = MetafileMapping.Emu(
                Math.Sqrt((dx * dx) + (dy * dy)) * (sum < 0 ? -1 : 1));

            advances.Add(mapped - previous);
            previous = mapped;
        }

        return advances;
    }

    private void DrawText(
        string text,
        int x,
        int y,
        TextAlignment alignment,
        IReadOnlyList<Length>? advances,
        float scaleX,
        float scaleY,
        uint graphicsMode)
    {
        if (string.IsNullOrEmpty(text)) return;

        DocPoint reference = (alignment & TextAlignment.UpdateCurrentPosition) != 0
            ? Map(_context.CurrentPosition.X, _context.CurrentPosition.Y)
            : Map(x, y);

        MetafileFont font = TextScale(scaleX, scaleY, graphicsMode, out double extra);

        if (_text.Layout(text, font, reference, alignment, advances) is not { } laid) return;

        foreach (GlyphRun run in laid.Runs)
        {
            _painter.DrawGlyphRun(run, (font.Escapement * Math.PI / 1800.0) + extra);
        }
        AdvanceCurrentPosition(alignment, laid.Width);
    }

    private void DrawGlyphs(
        ushort[] glyphs,
        int x,
        int y,
        TextAlignment alignment,
        IReadOnlyList<Length>? advances,
        float scaleX,
        float scaleY,
        uint graphicsMode)
    {
        if (glyphs.Length == 0) return;

        DocPoint reference = (alignment & TextAlignment.UpdateCurrentPosition) != 0
            ? Map(_context.CurrentPosition.X, _context.CurrentPosition.Y)
            : Map(x, y);

        MetafileFont font = TextScale(scaleX, scaleY, graphicsMode, out double extra);

        if (_text.LayoutGlyphs(glyphs, font, reference, alignment, advances) is not { } laid) return;

        foreach (GlyphRun run in laid.Runs)
        {
            _painter.DrawGlyphRun(run, (font.Escapement * Math.PI / 1800.0) + extra);
        }
        AdvanceCurrentPosition(alignment, laid.Width);
    }

    /// <summary>The record's <c>iGraphicsMode</c> value meaning <c>GM_ADVANCED</c>.</summary>
    private const uint GraphicsModeAdvanced = 2;

    /// <summary>
    /// Works out how the selected font has to be turned or condensed for one text record.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>Which of the two rules applies is decided by the record's own graphics mode, not by the
    /// device context's</b> — <c>MtfTools::DrawText</c> takes <c>iGraphicsMode</c> straight off the
    /// record and branches on it (<c>emfio/source/reader/mtftools.cxx:2083-2138</c>, called from
    /// <c>emfreader.cxx:2055</c> and <c>:2273</c>).
    /// </para>
    /// <para>
    /// <b><c>GM_ADVANCED</c>: the world transform rotates the text.</b> In advanced mode a
    /// producer turns text by turning the coordinate system rather than by setting a font
    /// escapement, so the escapement is zero and the rotation only exists in the transform. It is
    /// recovered by mapping the local <em>Y</em> axis and reading the angle off the result — which
    /// is what <c>mtftools.cxx:2085-2100</c> does, and why the +90° is there: the vector measured
    /// is the up direction, and the font's orientation describes the reading direction, a quarter
    /// turn from it. <c>2014BSA_Sunday_Killion.pptx</c> is the corpus case; its chart EMFs draw
    /// each category label under its own <c>EMR_MODIFYWORLDTRANSFORM</c> of
    /// <c>[0 −1; 1 0]</c> with <c>lfEscapement</c> zero, so without this the fourteen labels are
    /// drawn flat and overlapping instead of turned on their sides.
    /// </para>
    /// <para>
    /// <b><c>GM_COMPATIBLE</c>: the record's own scale factors do it.</b> The two floats are the
    /// recording device's page-to-device scale; differing magnitudes condense or expand the font
    /// by their ratio, and a negative one mirrors an axis, which reverses the direction the text
    /// reads in without mirroring the glyphs.
    /// </para>
    /// <para>
    /// The two are exclusive in the reference, and deliberately so: an advanced-mode record's
    /// scale factors have already been folded into its transform, so applying them again would
    /// condense the font twice.
    /// </para>
    /// </remarks>
    private MetafileFont TextScale(
        float scaleX, float scaleY, uint graphicsMode, out double extraRotation)
    {
        extraRotation = 0;

        MetafileFont font = _context.Font;

        if (graphicsMode == GraphicsModeAdvanced)
        {
            extraRotation = TransformRotation();
            return font;
        }

        if (!float.IsFinite(scaleX) || !float.IsFinite(scaleY) || scaleX == 0 || scaleY == 0) return font;

        double x = Math.Abs(scaleX);
        double y = Math.Abs(scaleY);

        if (Math.Abs(x - y) > 1e-6)
        {
            double ratio = y / x;
            font = font with { WidthScale = (font.WidthScale > 0 ? font.WidthScale : 1) * ratio };
        }

        if (scaleX < 0 && scaleY < 0) extraRotation = Math.PI;
        else if (scaleX < 0 || scaleY < 0) extraRotation = -2 * (font.Escapement * Math.PI / 1800.0);

        return font;
    }

    /// <summary>
    /// The angle the current mapping turns text through, anticlockwise in radians.
    /// </summary>
    /// <remarks>
    /// Measured rather than decomposed: the local <em>Y</em> axis is mapped and the angle read off
    /// the mapped vector, so a mapping built from any number of chained
    /// <c>EMR_MODIFYWORLDTRANSFORM</c> records, window/viewport settings and mirrors is handled by
    /// the same three lines. Zero when the mapped vector has no horizontal component, which is the
    /// unrotated case and the overwhelming majority of records — <c>if (fX)</c> at
    /// <c>mtftools.cxx:2093</c>, and the reason an ordinary metafile is untouched by any of this.
    /// <para>
    /// <b>The two points are rounded to whole 1/100 mm before they are subtracted</b>, because
    /// <c>ImplMap</c> returns an integer <c>Point</c> and the reference's "is there a horizontal
    /// component" test is therefore made on integers. Measuring in EMU instead makes an unrotated
    /// mapping produce a horizontal component of one or two EMU out of a million from ordinary
    /// rounding, and <c>acos</c> of that is 90° — which lands as a 180° turn on text that should
    /// not move at all.
    /// </para>
    /// </remarks>
    private double TransformRotation()
    {
        (double ox, double oy) = _context.Mapping.MapPointMm100(0, 0);
        (double ux, double uy) = _context.Mapping.MapPointMm100(0, 100);

        if (!double.IsFinite(ox) || !double.IsFinite(oy)
            || !double.IsFinite(ux) || !double.IsFinite(uy))
        {
            return 0;
        }

        double dx = Math.Round(ux) - Math.Round(ox);
        double dy = Math.Round(uy) - Math.Round(oy);

        double length = Math.Sqrt((dx * dx) + (dy * dy));
        if (dx == 0 || length == 0) return 0;

        double degrees = Math.Acos(Math.Clamp(dx / length, -1.0, 1.0)) * 180.0 / Math.PI;
        if (dy > 0) degrees = 360 - degrees;

        return (degrees + 90) * Math.PI / 180.0;
    }

    private void AdvanceCurrentPosition(TextAlignment alignment, Length width)
    {
        if ((alignment & TextAlignment.UpdateCurrentPosition) == 0) return;

        double scale = _context.Mapping.MapSizeMm100(1, 0).Width * MetafileMapping.EmuPerMm100;
        double advance = scale == 0 ? 0 : width.Emu / scale;
        _context.CurrentPosition = (_context.CurrentPosition.X + advance, _context.CurrentPosition.Y);
    }

    // ---------------------------------------------------------------- helpers

    /// <summary>
    /// Combines a transform with the world transform, in the order the record names.
    /// </summary>
    /// <remarks>
    /// The two multiplication orders are not interchangeable and the record states which it
    /// means, so getting them the wrong way round leaves a rotated-then-translated drawing
    /// translated-then-rotated (<c>MtfTools::ModifyWorldTransform</c>,
    /// <c>mtftools.cxx:2965-3040</c>).
    /// </remarks>
    private void ModifyWorldTransform(AffineTransform transform, uint mode)
    {
        const uint Identity = 0x01;
        const uint LeftMultiply = 0x02;
        const uint RightMultiply = 0x03;
        const uint Set = 0x04;

        _context.Mapping.World = mode switch
        {
            Identity => AffineTransform.Identity,
            Set => transform,
            LeftMultiply => Concatenate(transform, _context.Mapping.World),
            RightMultiply => Concatenate(_context.Mapping.World, transform),
            _ => _context.Mapping.World,
        };
    }

    private static AffineTransform Concatenate(AffineTransform left, AffineTransform right) => new(
        (left.A * right.A) + (left.B * right.C),
        (left.A * right.B) + (left.B * right.D),
        (left.C * right.A) + (left.D * right.C),
        (left.C * right.B) + (left.D * right.D),
        (left.E * right.A) + (left.F * right.C) + right.E,
        (left.E * right.B) + (left.F * right.D) + right.F);

    private AffineTransform ReadTransform()
    {
        float m11 = Single();
        float m12 = Single();
        float m21 = Single();
        float m22 = Single();
        float dx = Single();
        float dy = Single();

        if (_failed
            || !float.IsFinite(m11) || !float.IsFinite(m12) || !float.IsFinite(m21)
            || !float.IsFinite(m22) || !float.IsFinite(dx) || !float.IsFinite(dy))
        {
            return AffineTransform.Identity;
        }

        return new AffineTransform(m11, m12, m21, m22, dx, dy);
    }

    private List<DocPoint>? ReadPolygon(int end, bool sixteen, bool continueFrom)
    {
        _ = continueFrom;

        Skip(16);                       // bounds
        int count = (int)U32();
        int each = sixteen ? 4 : 8;

        if (_failed || !MetafileBudget.Plausible(count, each, end - _position) || count <= 0) return null;
        if (!_budget.ChargeSegments(count)) return null;

        List<DocPoint> points = new(count);
        for (int i = 0; i < count && !_failed; i++) points.Add(ReadPoint(sixteen));

        return points.Count > 0 ? points : null;
    }

    private DocPoint ReadPoint(bool sixteen) => sixteen ? Map(I16(), I16()) : Map(I32(), I32());

    private DocPoint Map(double x, double y) => _context.Mapping.MapPoint(x, y);

    /// <summary>
    /// A mapped point back in logical units, for the current position.
    /// </summary>
    /// <remarks>
    /// The current position is kept logical so that a mapping record between a move and a line
    /// is honoured, but several records set it from a point they have already mapped. Inverting
    /// the mapping numerically rather than algebraically keeps this to one place and works for
    /// every one of the eight modes; a mapping that collapses an axis is not invertible and the
    /// position is left where it was.
    /// </remarks>
    private (double X, double Y) Unmap(DocPoint point)
    {
        DocPoint origin = Map(0, 0);
        DocPoint unitX = Map(1, 0);
        DocPoint unitY = Map(0, 1);

        double a = unitX.X.Emu - origin.X.Emu;
        double b = unitX.Y.Emu - origin.Y.Emu;
        double c = unitY.X.Emu - origin.X.Emu;
        double d = unitY.Y.Emu - origin.Y.Emu;

        double determinant = (a * d) - (b * c);
        if (Math.Abs(determinant) < 1e-9) return _context.CurrentPosition;

        double dx = point.X.Emu - origin.X.Emu;
        double dy = point.Y.Emu - origin.Y.Emu;

        return (((dx * d) - (dy * c)) / determinant, ((dy * a) - (dx * b)) / determinant);
    }

    private DocRect MapRect((int Left, int Top, int Right, int Bottom) rect)
        => _context.Mapping.MapRect(rect.Left, rect.Top, rect.Right, rect.Bottom);

    private (int Left, int Top, int Right, int Bottom) ReadRect()
    {
        int left = I32();
        int top = I32();
        int right = I32();
        int bottom = I32();
        return (left, top, right, bottom);
    }

    /// <summary>
    /// Reads a rectangle whose right and bottom edges are exclusive, and makes them inclusive.
    /// </summary>
    /// <remarks>
    /// EMF states rectangles both ways and the record decides which: the clip and arc records
    /// are exclusive and <c>EMR_RECTANGLE</c> is not, which is what <c>EmfReader::ReadRectangle</c>
    /// distinguishes with its two overloads (<c>emfreader.cxx:2755-2778</c>). One logical unit on
    /// two edges is a hairline gap between shapes a producer meant to abut.
    /// </remarks>
    private (int Left, int Top, int Right, int Bottom) ReadRectExclusive()
    {
        int left = I32();
        int top = I32();
        int right = I32();
        int bottom = I32();
        return (left, top, right - 1, bottom - 1);
    }

    private (double X, double Y) ReadRatio()
    {
        int xNumerator = I32();
        int xDenominator = I32();
        int yNumerator = I32();
        int yDenominator = I32();

        return (
            xDenominator == 0 ? 1 : (double)xNumerator / xDenominator,
            yDenominator == 0 ? 1 : (double)yNumerator / yDenominator);
    }

    private Colour ReadColour() => _context.Palette.Resolve(U32());

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

    private byte U8()
    {
        if (_position + 1 > _bytes.Length)
        {
            _failed = true;
            return 0;
        }

        return _bytes[_position++];
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

    private float Single() => BitConverter.UInt32BitsToSingle(U32());

    private void Skip(int count) => _position = Math.Min(_position + count, _bytes.Length);

    private void Seek(int position) => _position = Math.Clamp(position, 0, _bytes.Length);
}

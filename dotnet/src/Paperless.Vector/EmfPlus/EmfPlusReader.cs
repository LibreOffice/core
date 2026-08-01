using Paperless.Core.Diagnostics;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Vector.Metafiles;

namespace Paperless.Vector.EmfPlus;

/// <summary>
/// Replays the EMF+ record stream that rides inside an enhanced metafile's comment records.
/// </summary>
/// <remarks>
/// <para>
/// <b>EMF+ is not a file format.</b> It has no header of its own on disk, no signature and no
/// extension: it is a second description of the same drawing, carried in
/// <c>EMR_COMMENT_EMFPLUS</c> records inside an ordinary EMF. So there is no
/// <c>IVectorImageDecoder</c> for it and nothing new registers in <c>VectorImages</c> — the EMF
/// decoder finds it, and this class replays it onto the device context, clip, painter, budget and
/// text engine the EMF reader already has.
/// </para>
/// <para>
/// Ported from <c>drawinglayer/source/tools/emfphelperdata.cxx</c> and its seven companion files,
/// with the record dispatch in <c>emfio/source/reader/emfreader.cxx</c> deciding which of the two
/// descriptions is replayed; see <see cref="Emf.EmfReader"/> for that decision.
/// </para>
/// <para>
/// <b>The coordinate chain is four stages and none of them is GDI's.</b> World transform, then
/// the page transform's unit and scale, then the reference device's pixels-to-millimetres, then
/// the frame origin — composed into one matrix whenever any of them changes, exactly as
/// <c>EmfPlusHelperData::mappingChanged</c> does. None of <c>MetafileMapping</c>'s eight mapping
/// modes appear anywhere in it, which is why EMF+ carries its own mapping rather than reusing the
/// one WMF and EMF share.
/// </para>
/// </remarks>
internal sealed class EmfPlusReader
{
    private readonly EmfPlusObject?[] _objects = new EmfPlusObject?[256];
    private readonly Dictionary<int, SavedState> _saved = [];
    private readonly Dictionary<int, SavedState> _containers = [];
    private readonly List<byte> _multipart = [];
    private readonly HashSet<string> _reported = [];

    private readonly MetafileDeviceContext _context;
    private readonly MetafilePainter _painter;
    private readonly MetafileBudget _budget;
    private readonly MetafileTextEngine _text;
    private readonly List<Diagnostic> _diagnostics;

    private readonly double _frameLeft;
    private readonly double _frameTop;
    private readonly double _deviceX;
    private readonly double _deviceY;
    private readonly DocRect _bounds;

    private AffineTransform _world = AffineTransform.Identity;
    private AffineTransform _map = AffineTransform.Identity;
    private double _pageScaleX = 1.0;
    private double _pageScaleY = 1.0;
    private double _scaleX = 1.0;
    private double _scaleY = 1.0;
    private double _dpiX;
    private double _dpiY;
    private bool _multipartOpen;
    private ushort _multipartFlags;

    /// <summary>
    /// Creates a reader seeded with the enclosing EMF's header fields.
    /// </summary>
    /// <remarks>
    /// The frame and the reference device come from <c>EMR_HEADER</c> and nowhere else — an EMF+
    /// stream states neither. That is what <c>MtfTools::PassEMFPlusHeaderInfo</c>
    /// (<c>emfio/source/reader/mtftools.cxx:3165-3196</c>) exists to hand across, and it is why an
    /// EMF+ replay cannot be started without having read the EMF header first.
    /// </remarks>
    /// <param name="context">The device context, shared with the EMF reader so a GetDC bracket sees the same clip.</param>
    /// <param name="painter">The painter, shared for the same reason.</param>
    /// <param name="budget">The work caps.</param>
    /// <param name="text">The text engine.</param>
    /// <param name="diagnostics">Where problems are recorded.</param>
    /// <param name="frame">The EMF's <c>rclFrame</c>, in 1/100 mm.</param>
    /// <param name="devicePixels">The reference device's size in pixels.</param>
    /// <param name="deviceMillimetres">The reference device's size in millimetres.</param>
    public EmfPlusReader(
        MetafileDeviceContext context,
        MetafilePainter painter,
        MetafileBudget budget,
        MetafileTextEngine text,
        List<Diagnostic> diagnostics,
        (int Left, int Top, int Right, int Bottom) frame,
        (int X, int Y) devicePixels,
        (int X, int Y) deviceMillimetres)
    {
        _context = context ?? throw new ArgumentNullException(nameof(context));
        _painter = painter ?? throw new ArgumentNullException(nameof(painter));
        _budget = budget ?? throw new ArgumentNullException(nameof(budget));
        _text = text ?? throw new ArgumentNullException(nameof(text));
        _diagnostics = diagnostics ?? throw new ArgumentNullException(nameof(diagnostics));

        _frameLeft = frame.Left;
        _frameTop = frame.Top;

        int pixelsX = devicePixels.X > 0 ? devicePixels.X : 100;
        int pixelsY = devicePixels.Y > 0 ? devicePixels.Y : 100;
        int millimetresX = deviceMillimetres.X > 0 ? deviceMillimetres.X : 1;
        int millimetresY = deviceMillimetres.Y > 0 ? deviceMillimetres.Y : 1;

        _deviceX = 100.0 * millimetresX / pixelsX;
        _deviceY = 100.0 * millimetresY / pixelsY;

        // The header record states the recording resolution, but plenty of files have no header
        // at all — the reference device's own resolution is the only other thing that can say what
        // a point or a millimetre is worth.
        _dpiX = pixelsX * 25.4 / millimetresX;
        _dpiY = pixelsY * 25.4 / millimetresY;

        _bounds = new DocRect(
            DocPoint.Origin,
            new DocSize(
                MetafileMapping.Emu(Math.Max(frame.Right - frame.Left + 1, 1)),
                MetafileMapping.Emu(Math.Max(frame.Bottom - frame.Top + 1, 1))));

        Remap();
    }

    /// <summary>
    /// True when the file's header said it carries a GDI description of the same drawing as well.
    /// </summary>
    public bool IsDual { get; private set; }

    /// <summary>
    /// True when the stream has just handed the device context back, so the GDI records that
    /// follow are meant to be drawn.
    /// </summary>
    public bool WantsDeviceContext { get; private set; }

    /// <summary>True once an EMF+ record has actually been seen.</summary>
    public bool HasRecords { get; private set; }

    /// <summary>
    /// Replays one comment record's worth of EMF+ records.
    /// </summary>
    /// <remarks>
    /// The framing is twelve bytes — type, flags, size, data size — and the size counts the header
    /// itself, so a size below twelve is the end of anything trustworthy. Every field is checked
    /// against what is left rather than believed, because a comment's payload is the most
    /// attacker-controllable part of a metafile: it is opaque to every consumer that does not
    /// understand it.
    /// </remarks>
    /// <param name="bytes">The buffer the payload sits in.</param>
    /// <param name="start">Where the payload begins.</param>
    /// <param name="length">How long it is.</param>
    public void Process(byte[] bytes, int start, int length)
    {
        ArgumentNullException.ThrowIfNull(bytes);

        EmfPlusStream stream = new(bytes, start, length);

        // The hand-back lasts until the next comment, which is what makes it bracket the GDI
        // records between the two (emfreader.cxx:597 clears it on entry).
        WantsDeviceContext = false;

        while (stream.Remaining >= 12 && !_budget.IsExhausted)
        {
            int at = stream.Offset;

            ushort type = stream.U16();
            ushort flags = stream.U16();
            uint size = stream.U32();
            uint dataSize = stream.U32();

            if (stream.Failed || size < 12) break;
            if (size - 12 > (uint)stream.Remaining) break;
            if (dataSize > size - 12) break;
            if (!_budget.ChargeRecord()) break;

            HasRecords = true;

            long next = (long)at + size;
            EmfPlusRecordType record = (EmfPlusRecordType)type;

            if (Continued(record, flags, dataSize, stream))
            {
                stream.SeekTo(next);
                continue;
            }

            if (record != EmfPlusRecordType.Object || (flags & 0x8000) == 0)
            {
                Record(record, flags, stream);
            }

            stream.SeekTo(next);
        }
    }

    /// <summary>
    /// Handles the continuation form of an object record, which splits one object over several.
    /// </summary>
    /// <remarks>
    /// <b>A single object may be larger than a record can hold</b> — a texture brush carrying a
    /// photograph routinely is — and EMF+ says so by setting the top flag bit and repeating the
    /// record with the same slot and type. Each part after its four-byte total-size field is
    /// appended, and the assembled buffer is parsed when a record arrives that is not part of the
    /// same object. A decoder that treats each part as a whole object parses the first fragment as
    /// an image header and then draws nothing at all.
    /// </remarks>
    /// <returns>True when the record was a fragment and nothing else should be done with it.</returns>
    private bool Continued(EmfPlusRecordType record, ushort flags, uint dataSize, EmfPlusStream stream)
    {
        bool fragment = record == EmfPlusRecordType.Object
            && ((_multipartOpen && (flags & 0x7FFF) == (_multipartFlags & 0x7FFF)) || (flags & 0x8000) != 0);

        if (fragment && dataSize >= 4)
        {
            if (!_multipartOpen)
            {
                _multipartOpen = true;
                _multipartFlags = flags;
                _multipart.Clear();
            }

            stream.Skip(4);             // the object's total size, which the parts repeat

            int take = (int)Math.Min(dataSize - 4, (uint)stream.Remaining);
            if (_multipart.Count + take <= MaxAssembledObject) _multipart.AddRange(stream.Take(take));

            return true;
        }

        if (_multipartOpen)
        {
            _multipartOpen = false;

            byte[] assembled = [.. _multipart];
            _multipart.Clear();
            Object(_multipartFlags, new EmfPlusStream(assembled, 0, assembled.Length));
        }

        return false;
    }

    /// <summary>How large an assembled multi-part object may grow before the rest is dropped.</summary>
    /// <remarks>
    /// A cap is needed because the parts are attacker-controlled and nothing in the format bounds
    /// how many there may be. Sixty-four megabytes is far beyond any real embedded image and far
    /// below what would exhaust a process.
    /// </remarks>
    private const int MaxAssembledObject = 64 * 1024 * 1024;

    // ---------------------------------------------------------------- dispatch

    private void Record(EmfPlusRecordType type, ushort flags, EmfPlusStream stream)
    {
        switch (type)
        {
            case EmfPlusRecordType.Header:
            {
                IsDual = (flags & 0x01) != 0;
                stream.Skip(4);         // version
                stream.Skip(4);         // the recording device's kind
                uint dpiX = stream.U32();
                uint dpiY = stream.U32();

                if (dpiX is > 0 and < 100000) _dpiX = dpiX;
                if (dpiY is > 0 and < 100000) _dpiY = dpiY;
                break;
            }

            case EmfPlusRecordType.GetDc:
                WantsDeviceContext = true;
                break;

            case EmfPlusRecordType.Object:
                Object(flags, stream);
                break;

            case EmfPlusRecordType.Clear:
                _painter.FillWith(MetafileGeometry.Rectangle(_bounds), Paint.Solid(EmfPlusBrush.Argb(stream.U32())));
                break;

            // ------------------------------------------------------ shapes
            case EmfPlusRecordType.FillRects:
            case EmfPlusRecordType.DrawRects:
                Rectangles(type, flags, stream);
                break;

            case EmfPlusRecordType.FillEllipse:
            case EmfPlusRecordType.DrawEllipse:
            {
                bool fill = type == EmfPlusRecordType.FillEllipse;
                uint brush = fill ? stream.U32() : 0;
                (double x, double y, double w, double h) = Rect(stream, (flags & 0x4000) != 0);

                GraphicsPath path = Ellipse(x, y, w, h);
                if (fill) Fill(path, flags, brush);
                else Stroke(path, flags);
                break;
            }

            case EmfPlusRecordType.FillPie:
            case EmfPlusRecordType.DrawPie:
            case EmfPlusRecordType.DrawArc:
                Pie(type, flags, stream);
                break;

            case EmfPlusRecordType.FillPolygon:
            {
                uint brush = stream.U32();
                int points = (int)stream.U32();

                if (EmfPlusPath.Read(stream, points, flags, withTypes: false) is { } polygon)
                {
                    Fill(polygon.ToPath(Map, close: true), flags, brush);
                }

                break;
            }

            case EmfPlusRecordType.DrawLines:
            {
                int points = (int)stream.U32();

                if (EmfPlusPath.Read(stream, points, flags, withTypes: false) is { } line)
                {
                    // 0x2000 asks for one more line from the last point back to the first.
                    Stroke(line.ToPath(Map, close: (flags & 0x2000) != 0), flags);
                }

                break;
            }

            case EmfPlusRecordType.FillPath:
            {
                uint brush = stream.U32();
                if (_objects[flags & 0xFF] is EmfPlusPath path) Fill(path.ToPath(Map), flags, brush);
                break;
            }

            case EmfPlusRecordType.DrawPath:
            {
                uint pen = stream.U32();
                if (_objects[flags & 0xFF] is EmfPlusPath path) StrokeWithPen(path.ToPath(Map), pen);
                break;
            }

            case EmfPlusRecordType.StrokeFillPath:
                // The record names a path and nothing else: the pen and brush are the ones a
                // preceding record left current, which EMF+ has no state for and this reader
                // therefore does not track. Drawing it with an arbitrary slot would be worse than
                // not drawing it.
                Warn("PL6037", "An EMF+ filled and stroked a path in one record, which was not drawn.");
                break;

            case EmfPlusRecordType.FillRegion:
            {
                uint brush = stream.U32();
                if (_objects[flags & 0xFF] is EmfPlusRegion region) FillRegion(region, flags, brush);
                break;
            }

            case EmfPlusRecordType.DrawBeziers:
                Beziers(flags, stream);
                break;

            case EmfPlusRecordType.DrawCurve:
            {
                double tension = stream.F32();
                int offset = (int)stream.U32();
                int segments = (int)stream.U32();
                int points = (int)stream.U32();

                if (EmfPlusPath.Read(stream, points, flags, withTypes: false) is { } curve
                    && curve.Spline(Map, tension, offset, segments, closed: false) is { } path)
                {
                    Stroke(path, flags);
                }

                break;
            }

            case EmfPlusRecordType.DrawClosedCurve:
            case EmfPlusRecordType.FillClosedCurve:
            {
                bool fill = type == EmfPlusRecordType.FillClosedCurve;
                uint brush = fill ? stream.U32() : 0;
                double tension = stream.F32();
                int points = (int)stream.U32();

                if (EmfPlusPath.Read(stream, points, flags, withTypes: false) is { } curve
                    && curve.Spline(Map, tension, 0, 0, closed: true) is { } path)
                {
                    if (fill) Fill(path, flags, brush);
                    else Stroke(path, flags);
                }

                break;
            }

            // ------------------------------------------------------ images
            case EmfPlusRecordType.DrawImage:
            case EmfPlusRecordType.DrawImagePoints:
                Image(type, flags, stream);
                break;

            // ------------------------------------------------------ text
            case EmfPlusRecordType.DrawString:
                DrawString(flags, stream);
                break;

            case EmfPlusRecordType.DrawDriverString:
                DrawDriverString(flags, stream);
                break;

            // ------------------------------------------------------ transforms
            case EmfPlusRecordType.SetWorldTransform:
                _world = EmfPlusBrush.ReadTransform(stream);
                Remap();
                break;

            case EmfPlusRecordType.ResetWorldTransform:
                _world = AffineTransform.Identity;
                Remap();
                break;

            case EmfPlusRecordType.MultiplyWorldTransform:
                Multiply(EmfPlusBrush.ReadTransform(stream), flags);
                break;

            case EmfPlusRecordType.TranslateWorldTransform:
            {
                double dx = stream.F32();
                double dy = stream.F32();
                Multiply(new AffineTransform(1, 0, 0, 1, dx, dy), flags);
                break;
            }

            case EmfPlusRecordType.ScaleWorldTransform:
            {
                double sx = stream.F32();
                double sy = stream.F32();
                Multiply(new AffineTransform(sx, 0, 0, sy, 0, 0), flags);
                break;
            }

            case EmfPlusRecordType.RotateWorldTransform:
            {
                double radians = stream.F32() * Math.PI / 180.0;
                double cos = Math.Cos(radians);
                double sin = Math.Sin(radians);
                Multiply(new AffineTransform(cos, sin, -sin, cos, 0, 0), flags);
                break;
            }

            case EmfPlusRecordType.SetPageTransform:
            {
                double scale = stream.F32();
                _pageScaleX = ToPixels(scale, (EmfPlusUnit)(flags & 0xFF), horizontal: true);
                _pageScaleY = ToPixels(scale, (EmfPlusUnit)(flags & 0xFF), horizontal: false);
                Remap();
                break;
            }

            // ------------------------------------------------------ state
            case EmfPlusRecordType.Save:
                Push(_saved, (int)stream.U32());
                break;

            case EmfPlusRecordType.Restore:
                Pop(_saved, (int)stream.U32());
                break;

            case EmfPlusRecordType.BeginContainerNoParams:
                Push(_containers, (int)stream.U32());
                break;

            case EmfPlusRecordType.BeginContainer:
                BeginContainer(flags, stream);
                break;

            case EmfPlusRecordType.EndContainer:
                Pop(_containers, (int)stream.U32());
                break;

            // ------------------------------------------------------ clipping
            case EmfPlusRecordType.ResetClip:
                _context.Clip = _context.Clip.Clone();
                _context.Clip.Reset();
                break;

            case EmfPlusRecordType.SetClipRect:
            {
                (double x, double y, double w, double h) = Rect(stream, compressed: false);
                ClipRectangle(DocRect.FromCorners(Map(x, y), Map(x + w, y + h)), Combine(flags));
                break;
            }

            case EmfPlusRecordType.SetClipPath:
                if (_objects[flags & 0xFF] is EmfPlusPath clip) ClipShape(clip.ToPath(Map), Combine(flags));
                break;

            case EmfPlusRecordType.SetClipRegion:
                if (_objects[flags & 0xFF] is EmfPlusRegion area) ClipRegion(area, Combine(flags));
                break;

            case EmfPlusRecordType.OffsetClip:
            {
                double dx = stream.F32();
                double dy = stream.F32();

                // The offset is a distance, so it goes through the mapping's linear part only —
                // running it through Map would add the frame origin a second time.
                _context.Clip = _context.Clip.Translate(
                    MetafileMapping.Emu((dx * _map.A) + (dy * _map.C)),
                    MetafileMapping.Emu((dx * _map.B) + (dy * _map.D)));
                break;
            }

            // ------------------------------------------------------ quality
            case EmfPlusRecordType.EndOfFile:
            case EmfPlusRecordType.Comment:
            case EmfPlusRecordType.SetRenderingOrigin:
            case EmfPlusRecordType.SetAntiAliasMode:
            case EmfPlusRecordType.SetTextRenderingHint:
            case EmfPlusRecordType.SetTextContrast:
            case EmfPlusRecordType.SetInterpolationMode:
            case EmfPlusRecordType.SetPixelOffsetMode:
            case EmfPlusRecordType.SetCompositingMode:
            case EmfPlusRecordType.SetCompositingQuality:
            case EmfPlusRecordType.MultiFormatStart:
            case EmfPlusRecordType.MultiFormatSection:
            case EmfPlusRecordType.MultiFormatEnd:
            case EmfPlusRecordType.SetTsGraphics:
            case EmfPlusRecordType.SetTsClip:
                // All of these change how a device renders rather than what is drawn: anti-alias
                // and pixel-offset modes are about sampling, the rendering origin only shifts a
                // brush's phase, and the terminal-server records restate state a real record has
                // already given. A display list has no equivalent for any of them.
                break;

            case EmfPlusRecordType.SerializableObject:
                Warn("PL6031", "An EMF+ carried a producer-private object, which was not drawn.");
                break;

            default:
                Warn("PL6031", $"An EMF+ used record type 0x{(ushort)type:X4}, which Paperless does not draw.");
                break;
        }
    }

    // ---------------------------------------------------------------- objects

    private void Object(ushort flags, EmfPlusStream stream)
    {
        int slot = flags & 0xFF;
        EmfPlusObjectType kind = (EmfPlusObjectType)(flags & 0x7F00);

        switch (kind)
        {
            case EmfPlusObjectType.Brush:
            {
                EmfPlusBrush brush = new();
                brush.Read(stream);
                _objects[slot] = brush;
                break;
            }

            case EmfPlusObjectType.Pen:
            {
                EmfPlusPen pen = new();
                pen.ReadPen(stream);
                _objects[slot] = pen;

                if (pen.HasCustomCap)
                {
                    Warn(
                        "PL6038",
                        "An EMF+ pen asked for an arrow or a custom line cap, which the drawing "
                            + "model cannot express; the line was drawn without it.");
                }

                break;
            }

            case EmfPlusObjectType.Path:
            {
                stream.Skip(4);         // version
                int points = (int)stream.U32();
                uint pathFlags = stream.U32();

                if (!MetafileBudget.Plausible(points, 3, stream.Remaining)) break;
                if (!_budget.ChargeSegments(points)) break;

                _objects[slot] = EmfPlusPath.Read(stream, points, pathFlags, withTypes: true);
                break;
            }

            case EmfPlusObjectType.Region:
            {
                EmfPlusRegion region = new();
                region.Read(stream, Map);
                _objects[slot] = region;
                break;
            }

            case EmfPlusObjectType.Image:
            {
                EmfPlusImage image = new();
                image.Read(stream);
                _objects[slot] = image;

                if (image.IsMetafile)
                {
                    Warn(
                        "PL6039",
                        "An EMF+ carried a metafile as an image, which Paperless does not replay "
                            + "inside another metafile.");
                }

                break;
            }

            case EmfPlusObjectType.Font:
            {
                EmfPlusFont font = new();
                font.Read(stream);
                _objects[slot] = font;
                break;
            }

            case EmfPlusObjectType.StringFormat:
            {
                EmfPlusStringFormat format = new();
                format.Read(stream);
                _objects[slot] = format;
                break;
            }

            case EmfPlusObjectType.ImageAttributes:
            case EmfPlusObjectType.CustomLineCap:
                // Both are read for their slot and nothing else: image attributes are colour and
                // gamma adjustments that would need the pixels, and a custom line cap is a line
                // decoration the drawing model has no place for.
                _objects[slot] = null;
                break;

            default:
                _objects[slot] = null;
                break;
        }
    }

    // ---------------------------------------------------------------- shapes

    /// <summary>The wrap mode that does not repeat, which is the only one a paint can state.</summary>
    private const int WrapClamp = 4;

    private void Rectangles(EmfPlusRecordType type, ushort flags, EmfPlusStream stream)
    {
        bool fill = type == EmfPlusRecordType.FillRects;
        uint brush = fill ? stream.U32() : 0;
        int count = (int)stream.U32();
        bool compressed = (flags & 0x4000) != 0;

        if (!MetafileBudget.Plausible(count, compressed ? 8 : 16, stream.Remaining)) return;

        GraphicsPath outlines = new();

        for (int i = 0; i < count && !stream.Failed; i++)
        {
            (double x, double y, double w, double h) = Rect(stream, compressed);

            GraphicsPath rect = new GraphicsPath()
                .MoveTo(Map(x, y))
                .LineTo(Map(x + w, y))
                .LineTo(Map(x + w, y + h))
                .LineTo(Map(x, y + h))
                .Close();

            if (!fill)
            {
                foreach (PathCommand command in rect.Commands)
                {
                    switch (command.Verb)
                    {
                        case PathVerb.MoveTo: outlines.MoveTo(command.Point); break;
                        case PathVerb.LineTo: outlines.LineTo(command.Point); break;
                        default: outlines.Close(); break;
                    }
                }

                continue;
            }

            // Each rectangle fills its own interior, so they go through one at a time: emitting
            // them as a single path would cancel pairwise overlaps under the even-odd rule and
            // leave a hole where two of them meet.
            Fill(rect, flags, brush);
        }

        if (!fill) Stroke(outlines, flags);
    }

    private void Pie(EmfPlusRecordType type, ushort flags, EmfPlusStream stream)
    {
        bool fill = type == EmfPlusRecordType.FillPie;
        uint brush = fill ? stream.U32() : 0;

        double start = stream.F32() * Math.PI / 180.0;
        double sweep = Math.Clamp(stream.F32(), -360, 360) * Math.PI / 180.0;
        (double x, double y, double w, double h) = Rect(stream, (flags & 0x4000) != 0);

        GraphicsPath path = Arc(x, y, w, h, start, sweep, close: type != EmfPlusRecordType.DrawArc);

        if (fill) Fill(path, flags, brush);
        else Stroke(path, flags);
    }

    private void Beziers(ushort flags, EmfPlusStream stream)
    {
        int count = (int)stream.U32();
        if (count < 4) return;

        if (EmfPlusPath.Read(stream, count, flags, withTypes: false) is not { Count: >= 4 } points) return;

        GraphicsPath path = new();
        (double x, double y) = points.Raw(0);
        path.MoveTo(Map(x, y));

        for (int i = 1; i + 2 < points.Count; i += 3)
        {
            (double c1x, double c1y) = points.Raw(i);
            (double c2x, double c2y) = points.Raw(i + 1);
            (double ex, double ey) = points.Raw(i + 2);

            path.CubicTo(Map(c1x, c1y), Map(c2x, c2y), Map(ex, ey));
        }

        Stroke(path, flags);
    }

    /// <summary>
    /// An ellipse arc built in the file's own coordinates and mapped point by point.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Built unmapped and mapped afterwards because the world transform may rotate or shear, and a
    /// shape built from a mapped rectangle would be an axis-aligned ellipse in the wrong space. A
    /// cubic Bézier's image under an affine map is the cubic through its mapped control points, so
    /// mapping the control points is exact rather than an approximation.
    /// </para>
    /// <para>
    /// <b>GDI+ states an arc's ends as angles and GDI states them as points on a ray</b>, so none
    /// of the ray-to-parameter conversion the WMF and EMF arcs need applies here: the angle
    /// <em>is</em> the ellipse's parameter, measured clockwise because y runs down.
    /// </para>
    /// </remarks>
    private GraphicsPath Arc(double x, double y, double w, double h, double start, double sweep, bool close)
    {
        double rx = w / 2.0;
        double ry = h / 2.0;
        double cx = x + rx;
        double cy = y + ry;

        int segments = Math.Max(1, (int)Math.Ceiling(Math.Abs(sweep) / (Math.PI / 2)));
        double step = sweep / segments;
        double alpha = 4.0 / 3.0 * Math.Tan(step / 4.0);

        GraphicsPath path = new();
        path.MoveTo(Map(cx + (rx * Math.Cos(start)), cy + (ry * Math.Sin(start))));

        double angle = start;

        for (int i = 0; i < segments; i++)
        {
            double next = angle + step;

            path.CubicTo(
                Map(
                    cx + (rx * Math.Cos(angle)) - (alpha * rx * Math.Sin(angle)),
                    cy + (ry * Math.Sin(angle)) + (alpha * ry * Math.Cos(angle))),
                Map(
                    cx + (rx * Math.Cos(next)) + (alpha * rx * Math.Sin(next)),
                    cy + (ry * Math.Sin(next)) - (alpha * ry * Math.Cos(next))),
                Map(cx + (rx * Math.Cos(next)), cy + (ry * Math.Sin(next))));

            angle = next;
        }

        if (close)
        {
            path.LineTo(Map(cx, cy));
            path.Close();
        }

        return path;
    }

    private GraphicsPath Ellipse(double x, double y, double w, double h)
    {
        GraphicsPath path = Arc(x, y, w, h, 0, 2 * Math.PI, close: false);
        path.Close();
        return path;
    }

    // ---------------------------------------------------------------- painting

    private void Fill(GraphicsPath path, ushort flags, uint brushOrColour)
    {
        FillRule rule = (flags & 0x2000) != 0 ? FillRule.NonZero : FillRule.EvenOdd;

        if ((flags & 0x8000) != 0)
        {
            _painter.FillWith(path, Paint.Solid(EmfPlusBrush.Argb(brushOrColour)), rule);
            return;
        }

        if (_objects[(int)(brushOrColour & 0xFF)] is not EmfPlusBrush brush) return;

        FillWithBrush(path, brush, rule);
    }

    private void FillWithBrush(GraphicsPath path, EmfPlusBrush brush, FillRule rule)
    {
        switch (brush.Type)
        {
            case EmfPlusBrushType.Hatch:
            {
                if (HatchLines(brush.HatchStyle) is { } style)
                {
                    // A GDI+ hatch is opaque: it states both colours, and the background is always
                    // painted. A GDI hatch takes its background from the device context instead.
                    _painter.FillWith(path, Paint.Solid(brush.SecondColour), rule);
                    _painter.FillHatch(path, brush.Colour, style, rule);
                    return;
                }

                // The percentage and decorative hatches are dot screens rather than lines, and a
                // blend of the two colours is what they average to — which is what LibreOffice
                // draws for them as well.
                _painter.FillWith(path, Paint.Solid(Blend(brush.Colour, brush.SecondColour, Shade(brush.HatchStyle))), rule);
                return;
            }

            case EmfPlusBrushType.Texture:
                if (Texture(brush) is { } tile) _painter.FillWith(path, tile, rule);
                else _painter.FillWith(path, Paint.Solid(brush.Colour), rule);
                return;

            case EmfPlusBrushType.LinearGradient:
            {
                GradientPaint gradient = Linear(brush);

                // A GDI+ gradient repeats or mirrors outside its own rectangle and
                // GradientPaint has no spread method at all, so a brush whose rectangle is
                // short against the shape comes out as one ramp and then flat colour where the
                // file asked for stripes. It is only reported when the shape actually reaches
                // past the ramp, because a gradient that covers what it fills looks the same
                // under every wrap mode. The same gap the SVG side records as PL6021.
                if (brush.WrapMode != WrapClamp && Repeats(path, gradient))
                {
                    Warn(
                        "PL6041",
                        "An EMF+ gradient brush repeated outside its own rectangle, which the "
                            + "drawing model cannot express; one ramp was drawn and then held "
                            + "at its end colour.");
                }

                _painter.FillWith(path, gradient, rule);
                return;
            }

            case EmfPlusBrushType.PathGradient:
                _painter.FillWith(path, Radial(brush), rule);
                return;

            default:
                _painter.FillWith(path, Paint.Solid(brush.Colour), rule);
                return;
        }
    }

    private void Stroke(GraphicsPath path, ushort flags) => StrokeWithPen(path, flags & 0xFFu);

    private void StrokeWithPen(GraphicsPath path, uint index)
    {
        if (_objects[(int)(index & 0xFF)] is not EmfPlusPen pen) return;

        double pixels = ToPixels(pen.Width, pen.Unit, horizontal: true);
        Length width = MetafileMapping.Emu(pixels * _scaleY);

        IReadOnlyList<Length>? dashes = null;

        if (pen.Dashes() is { Length: > 0 } pattern)
        {
            List<Length> lengths = new(pattern.Length);
            foreach (double each in pattern) lengths.Add(width * Math.Max(each, 0.01));

            // A dash pattern alternates, so an odd-length array would swap ink and gap on every
            // other repeat; repeating it makes the period even without changing the picture.
            if (lengths.Count % 2 != 0) lengths.AddRange(lengths);
            dashes = lengths;
        }

        _painter.StrokeWith(path, new Stroke(
            Paint.Solid(pen.Colour),
            width,
            EmfPlusPen.Cap(pen.StartCap),
            pen.Join,
            pen.MiterLimit,
            dashes,
            dashes is null ? Length.Zero : width * pen.DashOffset));
    }

    private void FillRegion(EmfPlusRegion region, ushort flags, uint brushOrColour)
    {
        if (region.IsEmpty) return;

        GraphicsPath? path = region.RectanglePath();

        if (region.Shapes.Count == 0)
        {
            if (path is null) path = MetafileGeometry.Rectangle(_bounds);
            Fill(path, (ushort)(flags | 0x2000), brushOrColour);
            return;
        }

        // The rectangles and the shapes are a union and an intersection, so the fill goes inside a
        // clip rather than into one path — which keeps the pair exact.
        MetafileClip saved = _context.Clip;

        if (path is not null && region.Rectangles is { } rectangles)
        {
            _context.Clip = _context.Clip.Clone();
            _context.Clip.Intersect(rectangles);
        }

        foreach (GraphicsPath shape in region.Shapes) Fill(shape, flags, brushOrColour);

        _context.Clip = saved;
    }

    // ---------------------------------------------------------------- brushes

    /// <summary>
    /// The six EMF+ hatch styles that are lines, as the six GDI hatches the painter draws.
    /// </summary>
    /// <remarks>
    /// GDI+ names fifty-three hatch styles where GDI names six. Twenty of them are one of those
    /// six at a different weight or spacing — a "dark upward diagonal" is a backward diagonal —
    /// and mapping those keeps them lines. The percentage screens and the decorative ones
    /// (confetti, weave, brick) have no line form at all, and <see cref="Shade"/> handles them.
    /// </remarks>
    private static HatchStyle? HatchLines(int style) => style switch
    {
        0x00 or 0x19 or 0x1B or 0x1D or 0x20 => HatchStyle.Horizontal,
        0x01 or 0x18 or 0x1A or 0x1C or 0x21 => HatchStyle.Vertical,
        0x02 or 0x12 or 0x14 or 0x16 or 0x1E => HatchStyle.ForwardDiagonal,
        0x03 or 0x13 or 0x15 or 0x17 or 0x1F => HatchStyle.BackwardDiagonal,
        0x04 or 0x2B or 0x2E or 0x30 => HatchStyle.Cross,
        0x05 => HatchStyle.DiagonalCross,
        _ => null,
    };

    /// <summary>How much of the foreground colour a dot-screen hatch style averages to.</summary>
    /// <remarks>
    /// The twelve percentage styles say so in their own names; every other style that is not a
    /// line averages to something near half, which is what LibreOffice uses for all of them.
    /// </remarks>
    private static double Shade(int style) => style switch
    {
        0x06 => 0.05,
        0x07 => 0.10,
        0x08 => 0.20,
        0x09 => 0.25,
        0x0A => 0.30,
        0x0B => 0.40,
        0x0C => 0.50,
        0x0D => 0.60,
        0x0E => 0.70,
        0x0F => 0.75,
        0x10 => 0.80,
        0x11 => 0.90,
        _ => 0.50,
    };

    /// <summary>True when a shape reaches past the end of the gradient that fills it.</summary>
    private static bool Repeats(GraphicsPath path, GradientPaint gradient)
    {
        double dx = gradient.End.X.Emu - gradient.Start.X.Emu;
        double dy = gradient.End.Y.Emu - gradient.Start.Y.Emu;
        double span = Math.Sqrt((dx * dx) + (dy * dy));

        if (span <= 0) return true;

        double left = double.MaxValue;
        double top = double.MaxValue;
        double right = double.MinValue;
        double bottom = double.MinValue;

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            left = Math.Min(left, command.Point.X.Emu);
            right = Math.Max(right, command.Point.X.Emu);
            top = Math.Min(top, command.Point.Y.Emu);
            bottom = Math.Max(bottom, command.Point.Y.Emu);
        }

        if (right < left) return false;

        return Math.Max(right - left, bottom - top) > span * 1.01;
    }

    private static Colour Blend(Colour foreground, Colour background, double factor)
    {
        double f = Math.Clamp(factor, 0, 1);

        return new Colour(
            (byte)Math.Round((foreground.R * f) + (background.R * (1 - f))),
            (byte)Math.Round((foreground.G * f) + (background.G * (1 - f))),
            (byte)Math.Round((foreground.B * f) + (background.B * (1 - f))),
            (byte)Math.Round((foreground.A * f) + (background.A * (1 - f))));
    }

    private BitmapPaint? Texture(EmfPlusBrush brush)
    {
        if (brush.Texture?.Image is not { } image) return null;

        int width = brush.Texture.Width > 0 ? brush.Texture.Width : 1;
        int height = brush.Texture.Height > 0 ? brush.Texture.Height : 1;

        // A texture brush maps one bitmap pixel onto one world unit, so the tile's size in the
        // document is the pixel count times what a world unit is worth.
        DocSize tile = new(
            MetafileMapping.Emu(width * _scaleX),
            MetafileMapping.Emu(height * _scaleY));

        if (tile.Width <= Length.Zero || tile.Height <= Length.Zero) return null;

        return new BitmapPaint(image, tile, DocPoint.Origin, Stretch: brush.WrapMode == WrapClamp);
    }

    /// <summary>
    /// A linear gradient brush, with the brush transform applied to its two ends.
    /// </summary>
    /// <remarks>
    /// A linear gradient is settled by two points, so applying the brush transform to the points
    /// and then mapping them is exact for any transform that is a similarity — which every rotated
    /// gradient in real clip art is. A sheared or anisotropically scaled brush would additionally
    /// tilt the ramp's iso-lines, which two points cannot say; LibreOffice expresses that by
    /// conjugating the whole map transform, which <c>GradientPaint.Transform</c> could carry too.
    /// </remarks>
    private GradientPaint Linear(EmfPlusBrush brush)
    {
        (double x, double y) = brush.FirstPoint;
        (double w, double h) = brush.Extent;

        _ = h;

        (double sx, double sy) = Apply(brush.Transform, x, y);
        (double ex, double ey) = Apply(brush.Transform, x + w, y);

        return new GradientPaint(
            GradientKind.Linear,
            Stops(brush),
            Map(sx, sy),
            Map(ex, ey),
            AffineTransform.Identity);
    }

    /// <summary>
    /// A path gradient brush, as the nearest radial gradient.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>This is the one brush the drawing model genuinely cannot state.</b> A GDI+ path gradient
    /// runs from one centre colour out to a colour <em>per boundary vertex</em>, Gouraud-shaded
    /// between them — a star with three surround colours has three coloured points and no radial
    /// gradient anywhere in it. <c>GradientPaint</c> has one ramp and one centre, so what is drawn
    /// is the ramp from the centre colour to the first surround colour over the boundary's own
    /// bounding ellipse.
    /// </para>
    /// <para>
    /// That is exact whenever the surround colours are all the same, which is the common case and
    /// the one that reads as a radial gradient in the first place. When they are not, the shape is
    /// right and the colours around its edge are not, and <c>PL6040</c> says so. LibreOffice
    /// renders the general case by triangulating the boundary and Gouraud-shading each triangle
    /// into a 256-pixel bitmap used as a texture; doing the same here would mean rasterising in a
    /// library arranged not to.
    /// </para>
    /// </remarks>
    private GradientPaint Radial(EmfPlusBrush brush)
    {
        (double cx, double cy) = Apply(brush.Transform, brush.FirstPoint.X, brush.FirstPoint.Y);
        DocPoint centre = Map(cx, cy);

        Length rx = Length.FromEmu(1);
        Length ry = Length.FromEmu(1);

        if (brush.Boundary is { Count: > 0 } boundary)
        {
            (double left, double top, double right, double bottom) = boundary.RawBounds();

            (double x0, double y0) = Apply(brush.Transform, left, top);
            (double x1, double y1) = Apply(brush.Transform, right, bottom);

            DocRect bounds = DocRect.FromCorners(Map(x0, y0), Map(x1, y1));
            rx = Length.Max(bounds.Width / 2.0, Length.FromEmu(1));
            ry = Length.Max(bounds.Height / 2.0, Length.FromEmu(1));
        }

        if (brush.SurroundColours is { Length: > 1 } surround)
        {
            foreach (Colour colour in surround)
            {
                if (colour == surround[0]) continue;

                Warn(
                    "PL6040",
                    "An EMF+ path gradient named a different colour at each boundary point, which "
                        + "the drawing model cannot express; one ramp was drawn instead.");
                break;
            }
        }

        double squash = ry.Emu / (double)rx.Emu;

        return new GradientPaint(
            GradientKind.Elliptical,
            Stops(brush),
            centre,
            new DocPoint(centre.X + rx, centre.Y),
            new AffineTransform(1, 0, 0, squash, 0, centre.Y.Emu * (1 - squash)));
    }

    /// <summary>
    /// A gradient brush's colour stops, from its two colours and whichever blend curve it carried.
    /// </summary>
    /// <remarks>
    /// Three spellings and they are mutually exclusive: no curve at all, a factor curve that says
    /// how far towards the second colour each position sits, or a preset list of colours. Ported
    /// from <c>EmfPlusHelperData::EMFPPlusFillPolygon</c>'s gradient branch.
    /// </remarks>
    private static List<GradientStop> Stops(EmfPlusBrush brush)
    {
        List<GradientStop> stops = [];

        if (brush.PresetPositions is { Length: > 0 } positions && brush.PresetColours is { } colours)
        {
            for (int i = 0; i < positions.Length && i < colours.Length; i++)
            {
                stops.Add(new GradientStop(Math.Clamp(positions[i], 0, 1), colours[i]));
            }
        }
        else if (brush.BlendPositions is { Length: > 0 } blend && brush.BlendFactors is { } factors)
        {
            for (int i = 0; i < blend.Length && i < factors.Length; i++)
            {
                stops.Add(new GradientStop(
                    Math.Clamp(blend[i], 0, 1),
                    Blend(brush.SecondColour, brush.Colour, factors[i])));
            }
        }

        if (stops.Count >= 2) return stops;

        stops.Clear();
        stops.Add(new GradientStop(0, brush.Colour));
        stops.Add(new GradientStop(1, brush.SecondColour));
        return stops;
    }

    // ---------------------------------------------------------------- images

    private void Image(EmfPlusRecordType type, ushort flags, EmfPlusStream stream)
    {
        stream.Skip(4);                 // the image-attributes slot, which needs the pixels
        int sourceUnit = stream.I32();

        if (_objects[flags & 0xFF] is not EmfPlusImage image || image.Image is not { } raster) return;

        // [MS-EMFPLUS] 2.3.4.8 allows only a pixel source rectangle, and every producer writes one.
        if (sourceUnit != (int)EmfPlusUnit.Pixel) return;

        double sx = stream.F32();
        double sy = stream.F32();
        double sw = stream.F32();
        double sh = stream.F32();

        double dx;
        double dy;
        double dw;
        double dh;
        double shearX = 0;
        double shearY = 0;

        if (type == EmfPlusRecordType.DrawImagePoints)
        {
            if (stream.U32() != 3) return;

            EmfPlusPath? corners = EmfPlusPath.Read(stream, 3, flags, withTypes: false);
            if (corners is not { Count: 3 }) return;

            (double x1, double y1) = corners.Raw(0);
            (double x2, double y2) = corners.Raw(1);
            (double x3, double y3) = corners.Raw(2);

            dx = x1;
            dy = y1;
            dw = x2 - x1;
            dh = y3 - y1;
            shearX = x3 - x1;
            shearY = y2 - y1;
        }
        else
        {
            (dx, dy, dw, dh) = Rect(stream, (flags & 0x4000) != 0);
        }

        if (dw == 0 || dh == 0) return;

        // The destination is a parallelogram in the file's own space and the map may rotate it, so
        // it is expressed as a transform and the image drawn into a square under it. That is
        // exactly what a DrawImagePoints record means, and it costs nothing for the ordinary
        // rectangle case, where the shear terms are zero.
        const double Unit = MetafilePainter.PlacementUnit;

        AffineTransform placement = Concat(
            new AffineTransform(dw / Unit, shearY / Unit, shearX / Unit, dh / Unit, dx, dy),
            Emu(_map));

        bool cropped = sw > 0 && sh > 0 && image.Width > 0 && image.Height > 0
            && (sx != 0 || sy != 0 || Math.Abs(sw - image.Width) > 0.5 || Math.Abs(sh - image.Height) > 0.5);

        if (!cropped)
        {
            _painter.DrawTransformedImage(raster, placement);
            return;
        }

        // The whole image is placed so that the wanted part lands on the square, and the square
        // clips the rest — the same answer the WMF and EMF source rectangles got, and the reason
        // nothing here needs a codec to crop with.
        DocRect whole = new(
            Length.FromEmu((long)Math.Round(-sx / sw * Unit)),
            Length.FromEmu((long)Math.Round(-sy / sh * Unit)),
            Length.FromEmu((long)Math.Round(image.Width / sw * Unit)),
            Length.FromEmu((long)Math.Round(image.Height / sh * Unit)));

        _painter.DrawTransformedImage(raster, placement, whole);
    }

    // ---------------------------------------------------------------- text

    private void DrawString(ushort flags, EmfPlusStream stream)
    {
        uint brush = stream.U32();
        uint format = stream.U32();
        int length = (int)Math.Min(stream.U32(), 0x10000u);

        double lx = stream.F32();
        double ly = stream.F32();
        double lw = stream.F32();
        double lh = stream.F32();

        string text = stream.Utf16(length);
        if (text.Length == 0) return;

        if (_objects[flags & 0xFF] is not EmfPlusFont font) return;

        EmfPlusStringFormat? layout = _objects[(int)(format & 0xFF)] as EmfPlusStringFormat;
        MetafileFont resolved = Font(font);

        double em = ToWorld(resolved.Size);
        double width = ToWorld(_text.Measure(text, resolved)) * (layout?.Tracking ?? 1.0);

        double x = lx;
        double y = ly;

        if (layout is not null)
        {
            x += layout.Alignment switch
            {
                1 => (0.5 * lw) + ((layout.LeadingMargin - layout.TrailingMargin) * em) - (0.5 * width),
                2 => lw - (layout.TrailingMargin * em) - width,
                _ => layout.LeadingMargin * em,
            };

            y += layout.LineAlignment switch
            {
                1 => (lh - em) / 2.0,
                2 => lh - em,
                _ => 0,
            };
        }

        // The baseline is placed in the file's own space and mapped once, so a rotated world
        // transform rotates the whole placement rather than only the run's direction.
        double ascent = ToWorld(Ascent(resolved));
        DocPoint origin = Map(x, y + ascent);

        IReadOnlyList<Length>? advances = Tracking(text, resolved, layout);

        if (_text.Layout(text, resolved, origin, TextAlignment.Baseline, advances) is not { } laid) return;

        _painter.DrawGlyphRun(laid.Run, Rotation(), Paint.Solid(TextColour(flags, brush)));
    }

    /// <summary>
    /// Draws glyphs the producer has already positioned, one run per shared baseline.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>The option flags decide both what the array holds and how many positions follow it.</b>
    /// Bit 0 says the values are characters to look up in the font's character map rather than
    /// glyph indices; bit 2 says the producer supplied one origin and the font's own advances
    /// rather than a position per glyph. LibreOffice reads a position per glyph unconditionally
    /// and only handles the character form, so a record that sets bit 2 desynchronises its parse.
    /// </para>
    /// <para>
    /// Glyphs sharing a y are one run, which keeps a PDF backend emitting real text; a change of y
    /// starts a new one, because a glyph run has a single baseline.
    /// </para>
    /// </remarks>
    private void DrawDriverString(ushort flags, EmfPlusStream stream)
    {
        uint brush = stream.U32();
        uint options = stream.U32();
        uint hasMatrix = stream.U32();
        int count = (int)Math.Min(stream.U32(), 0x10000u);

        if (count <= 0) return;
        if (_objects[flags & 0xFF] is not EmfPlusFont font) return;

        bool characters = (options & 0x01) != 0;
        bool realised = (options & 0x04) != 0;

        ushort[] codes = new ushort[count];
        for (int i = 0; i < count; i++) codes[i] = stream.U16();

        int positions = realised ? 1 : count;
        if ((long)positions * 8 > stream.Remaining) return;

        double[] xs = new double[count];
        double[] ys = new double[count];

        for (int i = 0; i < positions; i++)
        {
            xs[i] = stream.F32();
            ys[i] = stream.F32();
        }

        AffineTransform? matrix = hasMatrix != 0 ? EmfPlusBrush.ReadTransform(stream) : null;

        MetafileFont resolved = Font(font);
        Paint paint = Paint.Solid(TextColour(flags, brush));
        double rotation = Rotation();

        int at = 0;

        while (at < count)
        {
            int run = 1;
            if (!realised) while (at + run < count && Math.Abs(ys[at + run] - ys[at]) < 1e-6) run++;
            else run = count;

            List<Length>? advances = null;

            if (!realised && run > 1)
            {
                advances = new List<Length>(run);
                for (int i = 0; i < run; i++)
                {
                    double advance = i + 1 < run ? xs[at + i + 1] - xs[at + i] : 0;
                    advances.Add(MetafileMapping.Emu(advance * _scaleX));
                }
            }

            (double px, double py) = Apply(matrix, xs[at], ys[at]);
            DocPoint origin = Map(px, py);

            (GlyphRun Run, Length Width)? laid = characters
                ? _text.Layout(new string([.. codes.Skip(at).Take(run).Select(code => (char)code)]),
                    resolved, origin, TextAlignment.Baseline, advances)
                : _text.LayoutGlyphs([.. codes.Skip(at).Take(run)], resolved, origin, TextAlignment.Baseline, advances);

            if (laid is { } placed) _painter.DrawGlyphRun(placed.Run, rotation, paint);

            at += run;
        }
    }

    /// <summary>
    /// Per-character advances when the string format asks for tracking, or null for the font's own.
    /// </summary>
    /// <remarks>
    /// Tracking is a multiplier on every character's natural advance, so it is the one thing that
    /// makes an EMF+ string need a DX array where the file supplies none. Measuring per character
    /// rather than scaling the total keeps the run's internal spacing right.
    /// </remarks>
    private List<Length>? Tracking(string text, MetafileFont font, EmfPlusStringFormat? format)
    {
        if (format is null || Math.Abs(format.Tracking - 1.0) < 1e-6) return null;

        List<Length> advances = new(text.Length);
        foreach (char character in text) advances.Add(_text.Measure(character.ToString(), font) * format.Tracking);
        return advances;
    }

    private MetafileFont Font(EmfPlusFont font)
    {
        double pixels = ToPixels(font.Size, font.Unit, horizontal: false);
        Length size = MetafileMapping.Emu(pixels * _scaleY);

        if (size <= Length.Zero) size = MetafileFont.Default.Size;

        return new MetafileFont(
            font.Family is { Length: > 0 } family ? family : MetafileFont.Default.Family,
            size,
            font.IsBold ? 700 : 400,
            font.IsItalic,
            font.IsUnderlined,
            font.IsStruckOut);
    }

    private Length Ascent(MetafileFont font)
        => _text.Face(font) is { } face ? LineSpacing.Resolve(face.Face).ScaledAscent(font.Size) : font.Size;

    private Colour TextColour(ushort flags, uint brushOrColour)
    {
        if ((flags & 0x8000) != 0) return EmfPlusBrush.Argb(brushOrColour);

        return _objects[(int)(brushOrColour & 0xFF)] is EmfPlusBrush brush ? brush.Colour : Colour.Black;
    }

    /// <summary>The angle the mapping rotates a baseline through, positive anticlockwise.</summary>
    private double Rotation()
    {
        double angle = Math.Atan2(_map.B, _map.A);
        return Math.Abs(angle) < 1e-9 ? 0 : -angle;
    }

    // ---------------------------------------------------------------- clipping

    private static EmfPlusCombineMode Combine(ushort flags) => (EmfPlusCombineMode)((flags >> 8) & 0x0F);

    private void ClipRectangle(DocRect rect, EmfPlusCombineMode mode)
    {
        _context.Clip = _context.Clip.Clone();

        switch (mode)
        {
            case EmfPlusCombineMode.Replace:
                _context.Clip.Replace([rect]);
                break;

            case EmfPlusCombineMode.Intersect:
                _context.Clip.Intersect(rect);
                break;

            case EmfPlusCombineMode.Exclude:
                _context.Clip.Exclude(rect);
                break;

            default:
                _context.Clip.MarkUnsupported();
                break;
        }
    }

    private void ClipShape(GraphicsPath path, EmfPlusCombineMode mode)
    {
        _context.Clip = _context.Clip.Clone();

        switch (mode)
        {
            case EmfPlusCombineMode.Replace:
                _context.Clip.Replace(path, FillRule.EvenOdd);
                break;

            case EmfPlusCombineMode.Intersect:
                _context.Clip.Intersect(path, FillRule.EvenOdd);
                break;

            default:
                _context.Clip.MarkUnsupported();
                break;
        }
    }

    private void ClipRegion(EmfPlusRegion region, EmfPlusCombineMode mode)
    {
        if (region.IsApproximate) _context.Clip.MarkUnsupported();

        if (region.IsInfinite)
        {
            if (mode == EmfPlusCombineMode.Replace)
            {
                _context.Clip = _context.Clip.Clone();
                _context.Clip.Reset();
            }

            return;
        }

        _context.Clip = _context.Clip.Clone();

        if (mode is not (EmfPlusCombineMode.Replace or EmfPlusCombineMode.Intersect))
        {
            _context.Clip.MarkUnsupported();
            return;
        }

        if (mode == EmfPlusCombineMode.Replace) _context.Clip.Reset();

        if (region.Rectangles is { } rectangles) _context.Clip.Intersect(rectangles);
        foreach (GraphicsPath shape in region.Shapes) _context.Clip.Intersect(shape, FillRule.EvenOdd);
    }

    // ---------------------------------------------------------------- state

    private sealed record SavedState(AffineTransform World, MetafileClip Clip, double PageScaleX, double PageScaleY);

    /// <summary>
    /// Saves the graphics state under a numbered index, as <c>Save</c> and <c>BeginContainer</c> do.
    /// </summary>
    /// <remarks>
    /// <b>EMF+ save stacks are keyed, not stacked.</b> A record states the index it means, and a
    /// second save to the same index replaces the first — so a decoder that pushes onto a stack
    /// and pops from it restores the wrong state as soon as a file reuses an index, which the ones
    /// PowerPoint writes do. Ported from <c>EmfPlusHelperData::GraphicStatePush</c>.
    /// </remarks>
    private void Push(Dictionary<int, SavedState> map, int index)
        => map[index] = new SavedState(_world, _context.Clip.Clone(), _pageScaleX, _pageScaleY);

    private void Pop(Dictionary<int, SavedState> map, int index)
    {
        if (!map.TryGetValue(index, out SavedState? state)) return;

        _world = state.World;
        _context.Clip = state.Clip;
        _pageScaleX = state.PageScaleX;
        _pageScaleY = state.PageScaleY;
        Remap();
    }

    private void BeginContainer(ushort flags, EmfPlusStream stream)
    {
        (double dx, double dy, double dw, double dh) = Rect(stream, compressed: false);
        (double sx, double sy, double sw, double sh) = Rect(stream, compressed: false);
        int index = (int)stream.U32();

        Push(_containers, index);

        if (sw == 0 || sh == 0) return;

        EmfPlusUnit unit = (EmfPlusUnit)(flags & 0xFF);

        AffineTransform transform = new(
            ToPixels(dw / sw, unit, horizontal: true),
            0,
            0,
            ToPixels(dh / sh, unit, horizontal: false),
            ToPixels(dx - sx, unit, horizontal: true),
            ToPixels(dy - sy, unit, horizontal: false));

        _world = Concat(_world, transform);
        Remap();
    }

    // ---------------------------------------------------------------- mapping

    private void Multiply(AffineTransform transform, ushort flags)
    {
        // 0x2000 is the post-multiply flag: the stated transform applies after the world one.
        _world = (flags & 0x2000) != 0 ? Concat(_world, transform) : Concat(transform, _world);
        Remap();
    }

    /// <summary>Composes the four stages into the one matrix every point goes through.</summary>
    private void Remap()
    {
        double sx = _pageScaleX * _deviceX;
        double sy = _pageScaleY * _deviceY;

        _map = new AffineTransform(
            _world.A * sx,
            _world.B * sy,
            _world.C * sx,
            _world.D * sy,
            (_world.E * sx) - _frameLeft,
            (_world.F * sy) - _frameTop);

        _scaleX = Math.Sqrt((_map.A * _map.A) + (_map.B * _map.B));
        _scaleY = Math.Sqrt((_map.C * _map.C) + (_map.D * _map.D));

        if (_scaleX <= 0) _scaleX = 1;
        if (_scaleY <= 0) _scaleY = 1;
    }

    private DocPoint Map(double x, double y) => new(
        MetafileMapping.Emu((x * _map.A) + (y * _map.C) + _map.E),
        MetafileMapping.Emu((x * _map.B) + (y * _map.D) + _map.F));

    /// <summary>A length in EMUs back in the file's own units, for an offset stated there.</summary>
    private double ToWorld(Length length) => length.Emu / (_scaleX * MetafileMapping.EmuPerMm100);

    /// <summary>
    /// A size in one of the seven EMF+ units, as the pixels the rest of the mapping expects.
    /// </summary>
    /// <remarks>
    /// A port of <c>EmfPlusHelperData::unitToPixel</c>. World and display units are already what
    /// the world transform maps from, so they pass through; the physical units need the recording
    /// resolution, which is why the header's DPI fields are load-bearing rather than informational.
    /// </remarks>
    private double ToPixels(double value, EmfPlusUnit unit, bool horizontal)
    {
        double dpi = horizontal ? _dpiX : _dpiY;

        return unit switch
        {
            EmfPlusUnit.Point => value / 72.0 * dpi,
            EmfPlusUnit.Inch => value * dpi,
            EmfPlusUnit.Millimetre => value / 25.4 * dpi,
            EmfPlusUnit.Document => value * dpi / 300.0,
            _ => value,
        };
    }

    private static (double X, double Y) Apply(AffineTransform? transform, double x, double y)
    {
        if (transform is not { } t) return (x, y);

        return ((x * t.A) + (y * t.C) + t.E, (x * t.B) + (y * t.D) + t.F);
    }

    /// <summary>Applies <paramref name="first"/> and then <paramref name="second"/>.</summary>
    private static AffineTransform Concat(AffineTransform first, AffineTransform second) => new(
        (first.A * second.A) + (first.B * second.C),
        (first.A * second.B) + (first.B * second.D),
        (first.C * second.A) + (first.D * second.C),
        (first.C * second.B) + (first.D * second.D),
        (first.E * second.A) + (first.F * second.C) + second.E,
        (first.E * second.B) + (first.F * second.D) + second.F);

    /// <summary>The mapping as a transform into EMUs rather than into 1/100 mm.</summary>
    private static AffineTransform Emu(AffineTransform map) => new(
        map.A * MetafileMapping.EmuPerMm100,
        map.B * MetafileMapping.EmuPerMm100,
        map.C * MetafileMapping.EmuPerMm100,
        map.D * MetafileMapping.EmuPerMm100,
        map.E * MetafileMapping.EmuPerMm100,
        map.F * MetafileMapping.EmuPerMm100);

    private static (double X, double Y, double Width, double Height) Rect(EmfPlusStream stream, bool compressed)
    {
        if (!compressed) return (stream.F32(), stream.F32(), stream.F32(), stream.F32());

        return (stream.I16(), stream.I16(), stream.I16(), stream.I16());
    }

    private void Warn(string code, string message)
    {
        if (!_reported.Add(code + message)) return;
        _diagnostics.Add(new Diagnostic(DiagnosticSeverity.Warning, code, message));
    }
}

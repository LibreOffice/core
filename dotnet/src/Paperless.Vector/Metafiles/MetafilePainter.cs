using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// Turns a device context plus a shape into <see cref="IDrawingSink"/> calls.
/// </summary>
/// <remarks>
/// <para>
/// This is where the stateful half meets the stateless one. A metafile record says "polygon,
/// here are the points"; the sink wants a path and a paint. Everything in between — which
/// brush, which fill rule, whether the pen strokes at all, what the clip is — lives in
/// <see cref="MetafileDeviceContext"/> and is resolved here, once, for all three metafile
/// formats.
/// </para>
/// <para>
/// <b>The clip is emitted lazily and re-emitted whole.</b> A metafile's clip is device state
/// that changes when a record says so; the sink's clip is a scope that only ever narrows. The
/// only way to widen a sink's clip is to restore, so the painter keeps one save level open for
/// the clip and, whenever the clip has changed since the last drawing call, restores and saves
/// again. That is exactly <c>MtfTools::UpdateClipRegion</c>
/// (<c>emfio/source/reader/mtftools.cxx:1254-1289</c>), and doing it lazily rather than on
/// every clip record matters because files set the clip far more often than they draw.
/// </para>
/// </remarks>
public sealed class MetafilePainter
{
    /// <summary>
    /// The spacing between hatch lines, in 1/100 mm.
    /// </summary>
    /// <remarks>
    /// LibreOffice's constant for a hatch translated out of a metafile
    /// (<c>mtftools.cxx:106</c>, <c>Hatch(eStyle, rColor, 50, nAngle)</c>). GDI's own spacing is
    /// in device pixels and therefore resolution-dependent, which is no use to a
    /// resolution-independent display list.
    /// </remarks>
    public const int HatchSpacingMm100 = 50;

    private readonly IDrawingSink _sink;
    private readonly MetafileDeviceContext _context;
    private readonly MetafileBudget _budget;
    private MetafileClip? _appliedClip;
    private bool _open;

    /// <summary>Creates a painter over a sink and the context whose state it reads.</summary>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="context">The device context the records mutate.</param>
    /// <param name="budget">The work limit every emitted command is charged against.</param>
    public MetafilePainter(IDrawingSink sink, MetafileDeviceContext context, MetafileBudget budget)
    {
        _sink = sink ?? throw new ArgumentNullException(nameof(sink));
        _context = context ?? throw new ArgumentNullException(nameof(context));
        _budget = budget ?? throw new ArgumentNullException(nameof(budget));
    }

    /// <summary>Closes the clip's save level, if one is open.</summary>
    /// <remarks>
    /// Must be called once when the record stream ends, or the recorded display list is
    /// unbalanced and every sink that tracks depth will report a leak.
    /// </remarks>
    public void Finish()
    {
        if (!_open) return;

        _sink.Restore();
        _open = false;
        _appliedClip = null;
    }

    /// <summary>Fills and strokes a shape with the current brush and pen.</summary>
    public void FillAndStroke(GraphicsPath path)
    {
        Fill(path);
        Stroke(path);
    }

    /// <summary>Fills a shape with the current brush, if it fills anything.</summary>
    public void Fill(GraphicsPath path)
    {
        ArgumentNullException.ThrowIfNull(path);

        if (_context.IsNoOperation) return;

        MetafileBrush brush = _context.EffectiveBrush;
        if (brush.IsNull) return;
        if (!ApplyClip(path)) return;

        switch (brush.Style)
        {
            case BrushStyle.Hatched:
                // An opaque background under a hatch is GDI's rule and is what makes hatched
                // shapes in old clip art hide what they overlap rather than veil it.
                if (_context.BackgroundMode == BackgroundMode.Opaque)
                {
                    _sink.FillPath(path, Paint.Solid(_context.BackgroundColour), _context.FillRule);
                }

                Hatch(path, brush);
                break;

            case BrushStyle.Pattern:
            case BrushStyle.DibPattern:
            case BrushStyle.DibPatternPt:
            case BrushStyle.Pattern8x8:
            case BrushStyle.DibPattern8x8:
            case BrushStyle.MonoPattern:
                if (brush.Pattern is { } tile && !brush.PatternSize.IsEmpty)
                {
                    _sink.FillPath(
                        path,
                        new BitmapPaint(tile, brush.PatternSize, DocPoint.Origin, Stretch: false),
                        _context.FillRule);
                }
                else
                {
                    _sink.FillPath(path, Paint.Solid(brush.Colour), _context.FillRule);
                }

                break;

            default:
                _sink.FillPath(path, Paint.Solid(brush.Colour), _context.FillRule);
                break;
        }
    }

    /// <summary>Strokes a shape with the current pen, if it strokes anything.</summary>
    public void Stroke(GraphicsPath path)
    {
        ArgumentNullException.ThrowIfNull(path);

        if (_context.IsNoOperation) return;
        if (_context.Pen.ToStroke(_context.MiterLimit) is not { } stroke) return;
        if (!ApplyClip(path)) return;

        _sink.StrokePath(path, stroke);
    }

    /// <summary>
    /// Fills a shape with a paint the record supplies rather than with the selected brush.
    /// </summary>
    /// <remarks>
    /// <c>EMR_GRADIENTFILL</c> is the reason this exists: it names its own colours and ignores
    /// whatever brush is selected, so it needs the clip, the budget and the no-op flag without
    /// the brush resolution the rest of <see cref="Fill"/> does.
    /// </remarks>
    /// <param name="path">The shape.</param>
    /// <param name="paint">What to fill it with.</param>
    public void FillWith(GraphicsPath path, Paint paint)
    {
        ArgumentNullException.ThrowIfNull(path);

        if (_context.IsNoOperation) return;
        if (!ApplyClip(path)) return;

        _sink.FillPath(path, paint, _context.FillRule);
    }

    /// <summary>Fills a rectangle with the background colour, as an opaque text record does.</summary>
    public void FillBackground(DocRect rect)
    {
        if (_context.IsNoOperation || rect.IsEmpty) return;

        GraphicsPath path = GraphicsPath.Rectangle(rect);
        if (!ApplyClip(path)) return;

        _sink.FillPath(path, Paint.Solid(_context.BackgroundColour));
    }

    /// <summary>Draws a raster image into a destination rectangle.</summary>
    /// <param name="image">The image, decoded or still encoded.</param>
    /// <param name="destination">Where it goes.</param>
    /// <param name="clipTo">
    /// A rectangle to clip to, for a record that draws part of a bitmap: the whole image is
    /// placed so that the wanted part lands on the destination, and this hides the rest. It is
    /// how a source rectangle survives without a codec to crop with.
    /// </param>
    /// <param name="opacity">
    /// A uniform opacity, which is how a constant source alpha survives without decoding a
    /// pixel: <c>AlphaBlend</c>'s <c>SrcConstantAlpha</c> is exactly this, and the sink already
    /// takes it.
    /// </param>
    public void DrawImage(RasterImage image, DocRect destination, DocRect? clipTo = null, double opacity = 1.0)
    {
        ArgumentNullException.ThrowIfNull(image);

        if (_context.IsNoOperation || destination.IsEmpty) return;
        if (!_budget.ChargeCommand()) return;

        EnsureClip();

        if (clipTo is { } window)
        {
            _sink.Save();
            _sink.ClipPath(GraphicsPath.Rectangle(window));
            _sink.DrawImage(image, destination, opacity);
            _sink.Restore();
        }
        else
        {
            _sink.DrawImage(image, destination, opacity);
        }
    }

    /// <summary>Draws a glyph run, rotating it when the font asks for an escapement.</summary>
    /// <param name="run">The positioned glyphs.</param>
    /// <param name="radians">
    /// The baseline's rotation about the run's origin, positive anticlockwise. Non-zero
    /// escapements are common in charts, where every axis label is rotated.
    /// </param>
    public void DrawGlyphRun(GlyphRun run, double radians)
    {
        ArgumentNullException.ThrowIfNull(run);

        if (_context.IsNoOperation) return;
        if (!_budget.ChargeCommand()) return;

        EnsureClip();

        if (radians == 0)
        {
            _sink.DrawGlyphRun(run, Paint.Solid(_context.TextColour));
            return;
        }

        // Rotation is expressed as a transform about the run's origin rather than by rotating
        // the glyph offsets, so that the backend still sees one run of one font and can emit
        // real text rather than a glyph-per-command.
        double cos = Math.Cos(radians);
        double sin = Math.Sin(radians);
        double ox = run.Origin.X.Emu;
        double oy = run.Origin.Y.Emu;

        _sink.Save();
        _sink.Transform(new AffineTransform(
            cos,
            -sin,
            sin,
            cos,
            ox - (ox * cos) - (oy * sin),
            oy + (ox * sin) - (oy * cos)));
        _sink.DrawGlyphRun(run, Paint.Solid(_context.TextColour));
        _sink.Restore();
    }

    /// <summary>Makes the sink's clip agree with the context's, charging the path first.</summary>
    private bool ApplyClip(GraphicsPath path)
    {
        if (!_budget.ChargeCommand()) return false;
        if (!_budget.ChargeSegments(MetafileGeometry.SegmentCount(path))) return false;

        EnsureClip();
        return true;
    }

    private void EnsureClip()
    {
        if (_appliedClip is not null && _appliedClip.SameAs(_context.Clip)) return;

        if (_open) _sink.Restore();

        _sink.Save();
        _open = true;
        _context.Clip.Apply(_sink);
        _appliedClip = _context.Clip.Clone();
    }

    /// <summary>
    /// Draws a hatch as stroked lines clipped to the shape.
    /// </summary>
    /// <remarks>
    /// <c>Paint</c> has no hatch kind — <c>Paperless.Core.Graphics.Paint</c> says so explicitly,
    /// and resolves hatches "into a tiled BitmapPaint or an explicit set of stroked lines at
    /// read time". This is the second of those, and it is the better one here: a tiled bitmap
    /// would need a rasteriser, whereas lines stay resolution-independent and print sharp.
    /// </remarks>
    private void Hatch(GraphicsPath path, MetafileBrush brush)
    {
        DocRect bounds = Bounds(path);
        if (bounds.IsEmpty) return;

        Stroke stroke = new(Paint.Solid(brush.Colour), Length.Zero);
        Length spacing = Length.FromMm100(HatchSpacingMm100);

        _sink.Save();
        _sink.ClipPath(path, _context.FillRule);

        switch (brush.Hatch)
        {
            case HatchStyle.Vertical:
                Vertical(bounds, spacing, stroke);
                break;

            case HatchStyle.ForwardDiagonal:
                Diagonal(bounds, spacing, stroke, descending: true);
                break;

            case HatchStyle.BackwardDiagonal:
                Diagonal(bounds, spacing, stroke, descending: false);
                break;

            case HatchStyle.Cross:
                Horizontal(bounds, spacing, stroke);
                Vertical(bounds, spacing, stroke);
                break;

            case HatchStyle.DiagonalCross:
                Diagonal(bounds, spacing, stroke, descending: true);
                Diagonal(bounds, spacing, stroke, descending: false);
                break;

            default:
                Horizontal(bounds, spacing, stroke);
                break;
        }

        _sink.Restore();
    }

    private void Horizontal(DocRect bounds, Length spacing, Stroke stroke)
    {
        for (Length y = bounds.Top; y <= bounds.Bottom; y += spacing)
        {
            if (!_budget.ChargeCommand()) return;
            _sink.StrokePath(
                new GraphicsPath().MoveTo(new DocPoint(bounds.Left, y)).LineTo(new DocPoint(bounds.Right, y)),
                stroke);
        }
    }

    private void Vertical(DocRect bounds, Length spacing, Stroke stroke)
    {
        for (Length x = bounds.Left; x <= bounds.Right; x += spacing)
        {
            if (!_budget.ChargeCommand()) return;
            _sink.StrokePath(
                new GraphicsPath().MoveTo(new DocPoint(x, bounds.Top)).LineTo(new DocPoint(x, bounds.Bottom)),
                stroke);
        }
    }

    private void Diagonal(DocRect bounds, Length spacing, Stroke stroke, bool descending)
    {
        // Diagonals at 45 degrees are spaced along the axis they cross, so the perpendicular
        // spacing is the stated one times root two. Stepping by the stated spacing instead
        // draws a visibly denser hatch than either GDI or LibreOffice.
        Length step = spacing * Math.Sqrt(2.0);
        Length span = bounds.Width + bounds.Height;

        for (Length offset = Length.Zero; offset <= span; offset += step)
        {
            if (!_budget.ChargeCommand()) return;

            DocPoint a;
            DocPoint b;

            if (descending)
            {
                a = new DocPoint(bounds.Left + offset - bounds.Height, bounds.Top);
                b = new DocPoint(bounds.Left + offset, bounds.Bottom);
            }
            else
            {
                a = new DocPoint(bounds.Left + offset, bounds.Top);
                b = new DocPoint(bounds.Left + offset - bounds.Height, bounds.Bottom);
            }

            _sink.StrokePath(new GraphicsPath().MoveTo(a).LineTo(b), stroke);
        }
    }

    private static DocRect Bounds(GraphicsPath path)
    {
        bool any = false;
        Length left = default, top = default, right = default, bottom = default;

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            DocPoint point = command.Point;
            if (!any)
            {
                left = right = point.X;
                top = bottom = point.Y;
                any = true;
                continue;
            }

            left = Length.Min(left, point.X);
            top = Length.Min(top, point.Y);
            right = Length.Max(right, point.X);
            bottom = Length.Max(bottom, point.Y);
        }

        return any ? new DocRect(left, top, right - left, bottom - top) : DocRect.Empty;
    }
}

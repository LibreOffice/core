using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using SkiaSharp;

namespace Paperless.Rendering.Raster;

/// <summary>
/// Draws a page's display list onto a Skia canvas.
/// </summary>
/// <remarks>
/// <para>
/// One page at a time: the canvas is supplied and <see cref="BeginPage"/> only records the
/// sheet size, because a raster backend has to allocate its surface before it can be
/// handed a canvas, and it can only do that once it knows how big the page is. So
/// <see cref="RasterRenderer"/> asks the page for its size, makes the surface, and then
/// draws into it.
/// </para>
/// <para>
/// Coordinates are converted once, by a scale on the canvas: EMUs to device pixels is
/// <c>dpi / 914400</c>, and Skia's y axis already grows downwards as a document's does, so
/// no flip is needed — the opposite of the PDF backend, where every coordinate has to be
/// subtracted from the page height.
/// </para>
/// <para>
/// Glyphs are drawn from their ids at explicit positions and never re-shaped, which is the
/// whole point of the display list carrying ids: layout already committed to these advances
/// when it chose where the line broke, and asking Skia to lay the text out again would
/// produce a page whose glyphs disagree with its own line breaks.
/// </para>
/// </remarks>
internal sealed class SkiaDrawingSink : IDrawingSink, IDisposable
{
    private readonly SKCanvas _canvas;
    private readonly RasterRenderOptions _options;
    private readonly Dictionary<string, SKTypeface?> _typefaces = new(StringComparer.Ordinal);
    private readonly List<int> _groups = [];
    private readonly float _scale;

    /// <summary>Creates a sink drawing onto a canvas at a resolution.</summary>
    /// <param name="canvas">Where to draw.</param>
    /// <param name="options">The resolution, antialiasing and background.</param>
    public SkiaDrawingSink(SKCanvas canvas, RasterRenderOptions options)
    {
        ArgumentNullException.ThrowIfNull(canvas);
        ArgumentNullException.ThrowIfNull(options);

        _canvas = canvas;
        _options = options;
        _scale = (float)(options.Dpi / Length.EmuPerInch);
    }

    /// <inheritdoc/>
    public void BeginPage(DocSize size) => _canvas.Save();

    /// <inheritdoc/>
    public void EndPage() => _canvas.Restore();

    /// <inheritdoc/>
    public void Save() => _canvas.Save();

    /// <inheritdoc/>
    public void Restore() => _canvas.Restore();

    /// <inheritdoc/>
    public void Transform(AffineTransform transform)
    {
        // Skia's matrix is stated in device space, so the translation converts to pixels while
        // the linear part does not: scaling a scale factor would square it.
        _canvas.Concat(new SKMatrix(
            (float)transform.A, (float)transform.C, (float)(transform.E * _scale),
            (float)transform.B, (float)transform.D, (float)(transform.F * _scale),
            0, 0, 1));
    }

    /// <inheritdoc/>
    public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero)
    {
        ArgumentNullException.ThrowIfNull(path);

        using SKPath skia = Convert(path, rule);
        _canvas.ClipPath(skia, SKClipOperation.Intersect, _options.Antialias);
    }

    /// <inheritdoc/>
    public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero)
    {
        ArgumentNullException.ThrowIfNull(path);
        ArgumentNullException.ThrowIfNull(paint);

        // Conical and rectangular gradients have no Skia shader and no PDF shading either, so
        // both backends expand them into flat bands through the same shared arithmetic. Doing it
        // here rather than inside Brush keeps the two backends drawing the same picture, which is
        // the whole reason the decomposition is shared.
        if (paint is GradientPaint { Stops.Count: > 0 } gradient
            && !Fills.Gradients.HasNativeForm(gradient.Kind))
        {
            Fills.Gradients.DrawBands(this, path, gradient, rule);
            return;
        }

        if (paint is MeshPaint mesh)
        {
            FillMesh(path, mesh, rule);
            return;
        }

        using SKPath skia = Convert(path, rule);
        using SKPaint brush = Brush(paint, SKPaintStyle.Fill, Fills.Gradients.Bounds(path));
        _canvas.DrawPath(skia, brush);
    }

    /// <summary>
    /// Fills a path with a triangle mesh, as <c>SkVertices</c> inside the path as a clip.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The clip is what makes the mesh a fill: a path-gradient brush's boundary and the shape it
    /// fills need not be the same polygon, so the triangles are drawn where they lie and the path
    /// decides how much shows. The PDF backend paints its <c>/ShadingType 4</c> inside the same
    /// clip, so the two agree by construction.
    /// </para>
    /// <para>
    /// <b><c>SKBlendMode.Dst</c>, and the paint carries no shader.</b> Skia's
    /// <c>drawVertices</c> blends the per-vertex colours <em>with the paint's shader</em>, so a
    /// mode that reads the source — <c>Modulate</c>, the usual choice — multiplies the vertex
    /// colours by whatever the shaderless paint supplies and can black the mesh out entirely.
    /// <c>Dst</c> names the vertex colours alone, which is what a mesh means.
    /// </para>
    /// <para>
    /// <b>Antialiasing is off for the triangles.</b> Adjacent fan triangles share an edge
    /// exactly, and two antialiased edges composited over each other leave a visible seam of the
    /// background along every one of them — a hundred-vertex boundary would be a hundred pale
    /// spokes. The mesh's own outline is antialiased by the clip instead.
    /// </para>
    /// </remarks>
    private void FillMesh(GraphicsPath path, MeshPaint mesh, FillRule rule)
    {
        List<SKPoint> positions = [];
        List<SKColor> colours = [];

        foreach (MeshTriangle triangle in Fills.Meshes.Valid(mesh))
        {
            foreach (int index in (ReadOnlySpan<int>)[triangle.A, triangle.B, triangle.C])
            {
                MeshVertex vertex = mesh.Vertices[index];
                positions.Add(Point(vertex.Position));
                colours.Add(new SKColor(
                    vertex.Colour.R, vertex.Colour.G, vertex.Colour.B, vertex.Colour.A));
            }
        }

        if (positions.Count == 0) return;

        using SKPath skia = Convert(path, rule);
        using SKVertices vertices = SKVertices.CreateCopy(
            SKVertexMode.Triangles, [.. positions], [.. colours]);
        using SKPaint brush = new() { IsAntialias = false };

        _canvas.Save();
        _canvas.ClipPath(skia, SKClipOperation.Intersect, _options.Antialias);
        _canvas.DrawVertices(vertices, SKBlendMode.Dst, brush);
        _canvas.Restore();
    }

    /// <inheritdoc/>
    public void StrokePath(GraphicsPath path, Stroke stroke)
    {
        ArgumentNullException.ThrowIfNull(path);
        ArgumentNullException.ThrowIfNull(stroke);

        using SKPath skia = Convert(path, FillRule.NonZero);
        using SKPaint pen = Brush(stroke.Paint, SKPaintStyle.Stroke);

        // A zero width is a hairline — the thinnest the device can draw — which is a real
        // concept in the office formats and not the same as invisible. Skia spells it the same
        // way, so it passes straight through; anything else scales like a length.
        pen.StrokeWidth = (float)(stroke.Width.Emu * _scale);
        pen.StrokeCap = stroke.Cap switch
        {
            LineCap.Round => SKStrokeCap.Round,
            LineCap.Square => SKStrokeCap.Square,
            _ => SKStrokeCap.Butt,
        };
        pen.StrokeJoin = stroke.Join switch
        {
            LineJoin.Round => SKStrokeJoin.Round,
            LineJoin.Bevel => SKStrokeJoin.Bevel,
            _ => SKStrokeJoin.Miter,
        };
        pen.StrokeMiter = (float)stroke.MiterLimit;

        if (stroke.DashPattern is { Count: > 0 } dashes)
        {
            float[] intervals = new float[dashes.Count % 2 == 0 ? dashes.Count : dashes.Count * 2];
            for (int i = 0; i < intervals.Length; i++)
            {
                intervals[i] = (float)(dashes[i % dashes.Count].Emu * _scale);
            }

            pen.PathEffect = SKPathEffect.CreateDash(intervals, (float)(stroke.DashOffset.Emu * _scale));
        }

        _canvas.DrawPath(skia, pen);
        pen.PathEffect?.Dispose();
    }

    /// <inheritdoc/>
    public void DrawGlyphRun(GlyphRun run, Paint paint)
    {
        ArgumentNullException.ThrowIfNull(run);
        ArgumentNullException.ThrowIfNull(paint);
        if (run.Glyphs.Count == 0) return;

        if (TypefaceFor(run.Font) is not { } typeface) return;

        using SKFont font = new(typeface, (float)(run.FontSize.Emu * _scale));
        font.Subpixel = _options.Antialias;
        font.Edging = _options.Antialias ? SKFontEdging.Antialias : SKFontEdging.Alias;
        font.Hinting = SKFontHinting.None;

        ushort[] ids = new ushort[run.Glyphs.Count];
        SKPoint[] positions = new SKPoint[run.Glyphs.Count];

        for (int i = 0; i < run.Glyphs.Count; i++)
        {
            PositionedGlyph glyph = run.Glyphs[i];
            ids[i] = glyph.GlyphId;
            positions[i] = new SKPoint(
                (float)((run.Origin.X + glyph.Offset.X).Emu * _scale),
                (float)((run.Origin.Y + glyph.Offset.Y).Emu * _scale));
        }

        using SKPaint brush = Brush(paint, SKPaintStyle.Fill);

        if (_options.GlyphOutlines)
        {
            DrawOutlines(font, ids, positions, brush);
            return;
        }

        using SKTextBlobBuilder builder = new();
        builder.AddPositionedRun(ids, font, positions);
        using SKTextBlob? blob = builder.Build();
        if (blob is null) return;

        _canvas.DrawText(blob, 0, 0, brush);
    }

    /// <summary>
    /// Draws each glyph from its own outline, at its exact position.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Slower than a text blob and the reason it is the default anyway is measured: Skia's
    /// glyph cache rasterises a mask per glyph and places it at a <em>whole pixel</em>
    /// vertically — <c>SKFont.Subpixel</c> quantises the horizontal position and nothing
    /// quantises the vertical. On <c>prose-odt.odt</c> at 150 dpi that moved the page's ink
    /// centroid down by 0.56 px, a quarter of a point, and made the comparison script report
    /// a reflow cascade on a page whose layout was exact. A half-pixel error that only ever
    /// appears in one direction is worse than a slow path: it is indistinguishable from a
    /// layout bug, which is the one thing an image diff is worst at telling apart.
    /// </para>
    /// <para>
    /// This is not "text as outlines" in the sense the PDF backend forbids. Nothing is lost —
    /// the display list still carries the glyph ids and the text — it is only how the pixels
    /// are produced.
    /// </para>
    /// </remarks>
    private void DrawOutlines(SKFont font, ushort[] ids, SKPoint[] positions, SKPaint brush)
    {
        for (int i = 0; i < ids.Length; i++)
        {
            using SKPath? outline = font.GetGlyphPath(ids[i]);
            if (outline is null || outline.IsEmpty) continue;

            _canvas.Save();
            _canvas.Translate(positions[i].X, positions[i].Y);
            _canvas.DrawPath(outline, brush);
            _canvas.Restore();
        }
    }

    /// <inheritdoc/>
    public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0)
    {
        ArgumentNullException.ThrowIfNull(image);
        // Not `image.Width <= 0`: a reader may hand over an image it has not decoded, and an
        // undecoded one reports no size until a codec has looked at it. Testing the size here
        // dropped every `RasterImage.Encoded` silently — laying out correctly and drawing nothing.
        if (Empty(image) || destination.IsEmpty) return;
        if (Image(image) is not { } drawable) return;

        using (drawable)
        using (SKPaint brush = new()
        {
            IsAntialias = _options.Antialias,
            Color = SKColors.White.WithAlpha((byte)Math.Clamp(Math.Round(opacity * 255), 0, 255)),
        })
        {
            _canvas.DrawImage(
                drawable,
                Rect(destination),
                new SKSamplingOptions(SKFilterMode.Linear, SKMipmapMode.None),
                brush);
        }
    }

    /// <inheritdoc/>
    public void BeginTransparencyGroup(double opacity)
    {
        using SKPaint brush = new()
        {
            Color = SKColors.White.WithAlpha((byte)Math.Clamp(Math.Round(opacity * 255), 0, 255)),
        };

        // An offscreen layer rather than an alpha on every member, because a group at half
        // opacity is not the same picture as each of its members at half opacity: the members'
        // overlaps stay opaque against each other and only the composite fades.
        _groups.Add(_canvas.SaveLayer(brush));
    }

    /// <inheritdoc/>
    public void EndTransparencyGroup()
    {
        if (_groups.Count == 0) return;

        _canvas.RestoreToCount(_groups[^1]);
        _groups.RemoveAt(_groups.Count - 1);
    }

    /// <inheritdoc/>
    public void Dispose()
    {
        foreach (SKTypeface? typeface in _typefaces.Values) typeface?.Dispose();
        _typefaces.Clear();
    }

    // ------------------------------------------------------------------------------ helpers

    /// <summary>
    /// The Skia face behind a resolved reference.
    /// </summary>
    /// <remarks>
    /// Loaded from the file the reference names rather than looked up by family, because
    /// the reference is the outcome of resolution and asking Skia to resolve it again would
    /// let a second, differently-tuned font matcher overrule the one layout measured with.
    /// The family lookup is only the fallback for a reference that names no file.
    /// </remarks>
    private SKTypeface? TypefaceFor(FontReference font)
    {
        string key = font.FaceKey.Length > 0 ? font.FaceKey : font.FamilyName;
        if (_typefaces.TryGetValue(key, out SKTypeface? cached)) return cached;

        SKTypeface? typeface = null;
        (string path, int index) = SplitKey(font.FaceKey);

        if (path.Length > 0 && File.Exists(path)) typeface = SKTypeface.FromFile(path, index);
        typeface ??= SKTypeface.FromFamilyName(
            font.FamilyName,
            font.Weight,
            (int)SKFontStyleWidth.Normal,
            font.IsItalic ? SKFontStyleSlant.Italic : SKFontStyleSlant.Upright);

        // A face Skia cannot supply is not reported here on purpose: substitution happened during
        // resolution, and `SystemFontResolver.Substitutions` already records what was swapped for
        // what. Reporting it a second time from a backend would name the family rather than the
        // request, which is the less useful half of the pair.

        _typefaces[key] = typeface;
        return typeface;
    }

    private static (string Path, int Index) SplitKey(string key)
    {
        int hash = key.LastIndexOf('#');
        if (hash <= 0 || !int.TryParse(key[(hash + 1)..], out int index)) return (key, 0);

        return (key[..hash], index);
    }

    private SKPaint Brush(Paint paint, SKPaintStyle style, DocRect? region = null)
    {
        SKPaint brush = new() { IsAntialias = _options.Antialias, Style = style };

        switch (paint)
        {
            case SolidPaint solid:
                brush.Color = new SKColor(solid.Colour.R, solid.Colour.G, solid.Colour.B, solid.Colour.A);
                break;

            case GradientPaint gradient when gradient.Stops.Count > 0:
                brush.Shader = Shader(gradient);
                break;

            case BitmapPaint bitmap when region is { } bounds:
                brush.Shader = Shader(bitmap, bounds);

                // With a shader set, the paint's own alpha modulates the shader's output —
                // which is what a:alphaModFix and DFF_Prop_fillOpacity ask for, a transparent
                // fill over an opaque picture.
                brush.Color = brush.Shader is null
                    ? SKColors.Transparent
                    : SKColors.White.WithAlpha(
                        (byte)Math.Clamp(Math.Round(bitmap.Opacity * 255), 0, 255));
                break;

            default:
                brush.Color = SKColors.Transparent;
                break;
        }

        return brush;
    }

    /// <summary>
    /// A gradient as Skia states it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Linear is a linear shader and radial a radial one. Elliptical is the <em>same</em>
    /// radial shader with the gradient's own transform squashing one axis, which is what that
    /// transform exists for: it distorts the ramp without distorting the geometry underneath
    /// it. The two kinds that have no shader — conical and rectangular — never reach here;
    /// <see cref="FillPath"/> sends them to the shared band decomposition instead.
    /// </para>
    /// <para>
    /// The stops are normalised first, so a list that started at 0.2 or repeated an offset
    /// reaches Skia in the strictly-increasing, end-to-end form both it and PDF need. Skia
    /// tolerates more than PDF does, and normalising in one place is what stops the two
    /// backends drawing different pictures from the same list.
    /// </para>
    /// </remarks>
    private SKShader Shader(GradientPaint gradient)
    {
        IReadOnlyList<GradientStop> stops = Fills.Gradients.Normalise(gradient.Stops);

        SKColor[] colours = new SKColor[stops.Count];
        float[] offsets = new float[stops.Count];

        for (int i = 0; i < stops.Count; i++)
        {
            colours[i] = new SKColor(stops[i].Colour.R, stops[i].Colour.G, stops[i].Colour.B, stops[i].Colour.A);
            offsets[i] = (float)stops[i].Offset;
        }

        SKPoint start = Point(gradient.Start);
        SKPoint end = Point(gradient.End);
        SKMatrix local = Matrix(gradient.Transform);

        // Skia states a spread as a tile mode and repeats for nothing. The PDF backend has to
        // lengthen the shading's axis to say the same thing, which is why the period arithmetic
        // lives in Fills.Gradients and only that backend calls it.
        SKShaderTileMode mode = gradient.Spread switch
        {
            SpreadMethod.Reflect => SKShaderTileMode.Mirror,
            SpreadMethod.Repeat => SKShaderTileMode.Repeat,
            _ => SKShaderTileMode.Clamp,
        };

        if (gradient.Kind == GradientKind.Linear)
        {
            return SKShader.CreateLinearGradient(start, end, colours, offsets, mode, local);
        }

        float radius = (float)Math.Sqrt(
            ((end.X - start.X) * (end.X - start.X)) + ((end.Y - start.Y) * (end.Y - start.Y)));
        if (radius <= 0) radius = 1;

        // A focal radial is Skia's two-point conical with the inner circle collapsed to a point
        // at the focus, which is the same two-circle form PDF's /ShadingType 3 takes. Both are
        // exact, so a focus needs no approximation in either backend.
        if (gradient.Focus is { } focus)
        {
            return SKShader.CreateTwoPointConicalGradient(
                Point(focus), 0, start, radius, colours, offsets, mode, local);
        }

        return SKShader.CreateRadialGradient(start, radius, colours, offsets, mode, local);
    }

    /// <summary>
    /// A tiled or stretched bitmap fill as Skia states it.
    /// </summary>
    /// <remarks>
    /// A repeating image shader rather than the explicit grid of draws the PDF backend emits.
    /// The two are the same picture because both take their origin and step from
    /// <see cref="Fills.Tiles"/>: the shader's local matrix places one tile and Skia repeats it
    /// on exactly the lattice the PDF backend walks. Stretching is the degenerate case where
    /// the lattice has one cell the size of the region, and it clamps rather than repeats so
    /// that a rounding pixel at the edge does not wrap the far side of the image into view.
    /// </remarks>
    private SKShader? Shader(BitmapPaint bitmap, DocRect region)
    {
        if (Empty(bitmap.Image)) return null;
        if (Image(bitmap.Image) is not { } image) return null;

        try
        {
            DocRect cell = bitmap.Stretch
                ? region
                : Fills.Tiles.Cover(bitmap, region).FirstOrDefault();

            if (cell.Width.Emu <= 0 || cell.Height.Emu <= 0) return null;

            SKRect placed = Rect(cell);
            SKMatrix local = SKMatrix.CreateScaleTranslation(
                placed.Width / image.Width,
                placed.Height / image.Height,
                placed.Left,
                placed.Top);

            SKShaderTileMode mode = bitmap.Stretch ? SKShaderTileMode.Clamp : SKShaderTileMode.Repeat;
            return SKShader.CreateImage(image, mode, mode, local);
        }
        finally
        {
            image.Dispose();
        }
    }

    /// <summary>
    /// True when an image has nothing to draw: neither pixels nor bytes to decode into some.
    /// </summary>
    /// <remarks>
    /// Not <c>Width &lt;= 0</c>, which asks the same question only of an image that has already
    /// been decoded. A reader emits <see cref="RasterImage.Encoded"/> and leaves the dimensions
    /// at zero until a codec has seen the bytes, so testing the width here discards every
    /// picture every reader emits — silently, and only in the backends.
    /// </remarks>
    private static bool Empty(RasterImage image)
        => image.Pixels.IsEmpty && image.EncodedBytes.IsEmpty;

    /// <summary>
    /// A display-list image as an immutable Skia one.
    /// </summary>
    /// <remarks>
    /// The pixels are copied rather than pinned and installed, because an <c>SKImage</c>
    /// outlives the call that made it — a shader holds one for as long as the paint does —
    /// and a pinned array freed at the end of the call would leave it reading released memory.
    /// </remarks>
    private static SKImage? Image(RasterImage image)
    {
        // A reader hands over the bytes the file stored; the codec lives here.
        if (Images.RasterImageDecoder.Ensure(image) is not { } decoded) return null;
        image = decoded;

        if (image.Pixels.Length < image.Width * image.Height * 4) return null;

        SKImageInfo info = new(image.Width, image.Height, SKColorType.Rgba8888, SKAlphaType.Unpremul);
        byte[] pixels = image.Pixels.ToArray();
        System.Runtime.InteropServices.GCHandle pin =
            System.Runtime.InteropServices.GCHandle.Alloc(pixels, System.Runtime.InteropServices.GCHandleType.Pinned);

        try
        {
            using SKData data = SKData.CreateCopy(pin.AddrOfPinnedObject(), (ulong)pixels.Length);
            return SKImage.FromPixels(info, data, info.RowBytes);
        }
        finally
        {
            pin.Free();
        }
    }

    /// <summary>The document-space transform Skia states in device pixels.</summary>
    private SKMatrix Matrix(AffineTransform transform) => new(
        (float)transform.A, (float)transform.C, (float)(transform.E * _scale),
        (float)transform.B, (float)transform.D, (float)(transform.F * _scale),
        0, 0, 1);

    private SKPath Convert(GraphicsPath path, FillRule rule)
    {
        using SKPathBuilder builder = new()
        {
            FillType = rule == FillRule.EvenOdd ? SKPathFillType.EvenOdd : SKPathFillType.Winding,
        };

        foreach (PathCommand command in path.Commands)
        {
            switch (command.Verb)
            {
                case PathVerb.MoveTo:
                    builder.MoveTo(Point(command.Point));
                    break;
                case PathVerb.LineTo:
                    builder.LineTo(Point(command.Point));
                    break;
                case PathVerb.CubicTo:
                    builder.CubicTo(Point(command.Control1), Point(command.Control2), Point(command.Point));
                    break;
                case PathVerb.Close:
                default:
                    builder.Close();
                    break;
            }
        }

        return builder.Detach();
    }

    private SKPoint Point(DocPoint point)
        => new((float)(point.X.Emu * _scale), (float)(point.Y.Emu * _scale));

    private SKRect Rect(DocRect rect) => new(
        (float)(rect.Left.Emu * _scale),
        (float)(rect.Top.Emu * _scale),
        (float)(rect.Right.Emu * _scale),
        (float)(rect.Bottom.Emu * _scale));
}

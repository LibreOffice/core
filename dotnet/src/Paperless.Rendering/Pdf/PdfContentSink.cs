using System.Globalization;
using System.Text;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Rendering.Pdf;

/// <summary>One page's content stream, and the sheet it goes on.</summary>
/// <param name="Size">The sheet size the layout reported.</param>
/// <param name="Content">The operators, ready to be deflated into a stream object.</param>
internal readonly record struct PdfPageContent(DocSize Size, string Content);

/// <summary>
/// Turns drawing commands into PDF content-stream operators.
/// </summary>
/// <remarks>
/// <para>
/// Two conventions run through the whole of this, and both are chosen to match what
/// LibreOffice's own export writes, because the fidelity harness reads that shape and is
/// pointed at our output as well as at its.
/// </para>
/// <list type="bullet">
///   <item>
///     <b>No flipping transform.</b> PDF's y grows upwards and a document's grows down, and
///     the tempting fix is one <c>cm</c> at the top of the page. It is the wrong fix: the
///     text matrix inherits the transform, so every glyph would be drawn upside down and
///     have to be flipped back. Each coordinate is converted instead — <c>pageHeight - y</c>
///     — which is what LibreOffice does and what makes a <c>Td</c> in our stream directly
///     comparable with a <c>Td</c> in its.
///   </item>
///   <item>
///     <b>Every operation brackets itself in <c>q … Q</c>.</b> There is no ambient graphics
///     state to keep in step, which matters because the display list has none either: every
///     draw call carries its own paint.
///   </item>
/// </list>
/// </remarks>
internal sealed class PdfContentSink(
    PdfDocumentWriter writer,
    PdfFontCatalogue fonts,
    PdfRenderOptions options) : IDrawingSink
{
    /// <summary>
    /// How far a glyph may sit from where the font's own advances would put it before the
    /// content stream states a correction, in points.
    /// </summary>
    /// <remarks>
    /// A twentieth of a twip. Below it a correction would be rounded away by the number
    /// formatter anyway; above it, the pen has drifted from where layout put it, and layout
    /// is the thing being reproduced. Justification is the common cause — a blank on a
    /// justified line is wider than the font says — and it is why an unjustified line
    /// produces one <c>Tj</c> and a justified one a <c>TJ</c> array.
    /// </remarks>
    private const double PenTolerancePoints = 0.0025;

    private readonly List<PdfPageContent> _pages = [];
    private readonly List<(string Name, int Id)> _xObjects = [];
    private readonly List<(string Name, int Id)> _states = [];
    private readonly List<(string Name, int Id)> _shadings = [];
    private readonly Dictionary<string, string> _stateNames = new(StringComparer.Ordinal);
    private readonly Dictionary<RasterImage, string> _imageNames = new(ReferenceEqualityComparer.Instance);
    private readonly Stack<StringBuilder> _groups = new();
    private readonly Stack<double> _groupOpacities = new();
    private readonly Stack<(AffineTransform Ctm, DocRect? Clip)> _saved = new();

    private StringBuilder _content = new();
    private DocSize _size;
    private double _pageHeight;
    private int _depth;
    private bool _open;

    /// <summary>Where the space a draw call states its coordinates in sits on the page.</summary>
    private AffineTransform _ctm = AffineTransform.Identity;

    /// <summary>
    /// The smallest rectangle in page coordinates that still contains everything the active
    /// clip can show, or null when nothing has been clipped.
    /// </summary>
    /// <remarks>
    /// A bounding box rather than the region itself, and deliberately so: it is only ever
    /// asked whether something falls <em>entirely outside</em> it, and a box that is too large
    /// answers that conservatively — it keeps a glyph the real clip would have hidden, and can
    /// never drop one the real clip would have shown.
    /// </remarks>
    private DocRect? _clip;

    /// <summary>The pages drawn so far, in order.</summary>
    public IReadOnlyList<PdfPageContent> Pages => _pages;

    /// <summary>True when a page was begun and never ended.</summary>
    public bool HasUnclosedPage => _open;

    /// <inheritdoc/>
    public void BeginPage(DocSize size)
    {
        _content = new StringBuilder();
        _size = size;
        _pageHeight = size.Height.Points;
        _depth = 0;
        _open = true;
        _saved.Clear();
        _ctm = AffineTransform.Identity;
        _clip = null;
    }

    /// <inheritdoc/>
    public void EndPage()
    {
        if (!_open) return;

        // A page that unbalanced its own state stack would otherwise nest the next one inside it.
        while (_depth > 0) { _content.Append("Q\n"); _depth--; }

        _saved.Clear();
        _ctm = AffineTransform.Identity;
        _clip = null;

        _pages.Add(new PdfPageContent(_size, _content.ToString()));
        _open = false;
    }

    /// <inheritdoc/>
    public void Save()
    {
        _saved.Push((_ctm, _clip));
        _content.Append("q\n");
        _depth++;
    }

    /// <inheritdoc/>
    public void Restore()
    {
        if (_depth == 0) return;

        _content.Append("Q\n");
        _depth--;
        if (_saved.Count > 0) (_ctm, _clip) = _saved.Pop();
    }

    /// <inheritdoc/>
    public void Transform(AffineTransform transform)
    {
        AppendTransform(transform);
        _ctm = AffineTransform.Concat(transform, _ctm);
    }

    /// <inheritdoc/>
    public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero)
    {
        ArgumentNullException.ThrowIfNull(path);
        if (path.Commands.Count == 0) return;

        AppendClip(path, rule);

        if (Fills.Gradients.Bounds(path) is not { } bounds) return;

        DocRect box = Mapped(bounds, _ctm);
        _clip = _clip is { } existing ? Intersect(existing, box) : box;
    }

    /// <summary>Writes the <c>cm</c> a transform is, without recording it.</summary>
    /// <remarks>
    /// Split out from <see cref="Transform"/> because the fills below state a transform inside
    /// a <c>q … Q</c> of their own making, which the tracked state knows nothing about. Those
    /// brackets are balanced and hold nothing but a shading, so the transform they state is
    /// gone by the time anything the tracked state governs is drawn again — recording it would
    /// leave the current matrix wrong for the rest of the page.
    /// </remarks>
    private void AppendTransform(AffineTransform transform)
    {
        // A transform stated in a y-down space has to be conjugated by the flip that takes that
        // space to PDF's, or a rotation turns the wrong way and a translation moves up the page.
        // With F(x, y) = (x, H - y), the matrix below is F * T * F, which is F * T * F-inverse
        // because F is its own inverse.
        double e = (transform.C * _pageHeight) + (transform.E / Length.EmuPerPoint);
        double f = (_pageHeight * (1 - transform.D)) - (transform.F / Length.EmuPerPoint);

        _content.Append(CultureInfo.InvariantCulture,
            $"{N(transform.A)} {N(-transform.B)} {N(-transform.C)} {N(transform.D)} {N(e)} {N(f)} cm\n");
    }

    /// <summary>Writes the path and the <c>W n</c> a clip is, without recording it.</summary>
    /// <remarks>The counterpart of <see cref="AppendTransform"/>, and for the same reason.</remarks>
    private void AppendClip(GraphicsPath path, FillRule rule)
    {
        AppendPath(path);
        _content.Append(rule == FillRule.EvenOdd ? "W* n\n" : "W n\n");
    }

    /// <inheritdoc/>
    public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero)
    {
        ArgumentNullException.ThrowIfNull(path);
        ArgumentNullException.ThrowIfNull(paint);
        if (path.Commands.Count == 0) return;

        switch (paint)
        {
            case GradientPaint gradient when gradient.Stops.Count > 0:
                FillGradient(path, gradient, rule);
                return;

            case BitmapPaint bitmap:
                FillBitmap(path, bitmap, rule);
                return;

            case MeshPaint mesh:
                FillMesh(path, mesh, rule);
                return;

            default:
                break;
        }

        Colour colour = Flatten(paint);
        if (colour.IsTransparent) return;

        _content.Append("q\n");
        AppendAlpha(colour.A, stroking: false);
        _content.Append(CultureInfo.InvariantCulture,
            $"{PdfSyntax.Component(colour.R)} {PdfSyntax.Component(colour.G)} {PdfSyntax.Component(colour.B)} rg\n");

        AppendPath(path);
        _content.Append(rule == FillRule.EvenOdd ? "f*\n" : "f\n").Append("Q\n");
    }

    /// <inheritdoc/>
    public void StrokePath(GraphicsPath path, Stroke stroke)
    {
        ArgumentNullException.ThrowIfNull(path);
        ArgumentNullException.ThrowIfNull(stroke);
        if (path.Commands.Count == 0) return;

        Colour colour = Flatten(stroke.Paint);
        if (colour.IsTransparent) return;

        _content.Append("q\n");
        AppendAlpha(colour.A, stroking: true);

        // Width first, then the caps and joins, then the colour, then the path: the order
        // LibreOffice writes and the order the harness's stroke reader pairs a pen with a line in.
        // A zero width is a hairline in both — the thinnest the device can draw — and PDF spells
        // it the same way, so it needs no special case.
        _content.Append(CultureInfo.InvariantCulture,
            $"{N(stroke.Width.Points)} w\n{(int)stroke.Cap} J\n{(int)stroke.Join} j\n");

        if (stroke.Join == LineJoin.Miter && Math.Abs(stroke.MiterLimit - 10.0) > 0.0001)
        {
            _content.Append(CultureInfo.InvariantCulture, $"{N(stroke.MiterLimit)} M\n");
        }

        AppendDash(stroke);

        _content.Append(CultureInfo.InvariantCulture,
            $"{PdfSyntax.Component(colour.R)} {PdfSyntax.Component(colour.G)} {PdfSyntax.Component(colour.B)} RG\n");

        AppendPath(path);
        _content.Append("S\nQ\n");
    }

    /// <inheritdoc/>
    public void DrawGlyphRun(GlyphRun run, Paint paint)
    {
        ArgumentNullException.ThrowIfNull(run);
        ArgumentNullException.ThrowIfNull(paint);
        if (run.Glyphs.Count == 0) return;


        // An em of nothing paints nothing, so the glyphs would be a text layer over blank paper.
        // Autofit is what produces one: a shape whose paragraphs carry an absolute top margin can
        // be too tall for its box at *every* font scale, and the shrink search then runs to the
        // bottom of its grid and rounds the smaller runs to zero. Measured on
        // `NWD-GLA-Community-Outreach-Day-Oct-2025.pptx`, whose subtitle holds sixteen paragraphs
        // at a 12 pt top margin in a 90.7 pt box — 180 pt of margin alone. The reference reaches
        // the same conclusion and draws no glyph at all there; across the 98 decks of slides
        // batches 001–010 it writes `0 Tf` exactly nowhere.
        if (run.FontSize <= Length.Zero) return;

        Colour colour = Flatten(paint);
        if (colour.IsTransparent) return;

        // Before `fonts.Observe`, so a run nothing can see contributes no glyph to a subset either.
        if (Hidden(run)) return;

        // Before anything is written, because the pen corrections below are computed against the
        // widths the file will state and this is what supplies them when the face did not load.
        fonts.Observe(run);

        string?[] texts = ClusterTexts(run);
        double size = run.FontSize.Points;

        _content.Append("q\n");
        AppendAlpha(colour.A, stroking: false);
        _content.Append(CultureInfo.InvariantCulture,
            $"{PdfSyntax.Component(colour.R)} {PdfSyntax.Component(colour.G)} {PdfSyntax.Component(colour.B)} rg\n");
        _content.Append("BT\n");

        double lineX = 0, lineY = 0;
        bool positioned = false;
        string resource = string.Empty;

        int at = 0;
        while (at < run.Glyphs.Count)
        {
            (string segmentResource, int end) = Segment(run, texts, at);

            double x = (run.Origin.X + run.Glyphs[at].Offset.X).Points;
            double y = _pageHeight - (run.Origin.Y + run.Glyphs[at].Offset.Y).Points;

            // Td is relative to the previous line's origin rather than to the page, so the first
            // one in a block is absolute — which is exactly what makes a pen position readable
            // out of a content stream without tracking a matrix, on our side and on LibreOffice's.
            _content.Append(CultureInfo.InvariantCulture,
                $"{N(positioned ? x - lineX : x)} {N(positioned ? y - lineY : y)} Td\n");
            lineX = x;
            lineY = y;
            positioned = true;

            if (!string.Equals(resource, segmentResource, StringComparison.Ordinal))
            {
                _content.Append(CultureInfo.InvariantCulture, $"/{segmentResource} {N(size)} Tf\n");
                resource = segmentResource;
            }

            AppendShow(run, texts, at, end, x, size);
            at = end;
        }

        _content.Append("ET\nQ\n");
    }

    // ----------------------------------------------------------------------------- clipping

    /// <summary>
    /// True when the active clip excludes the whole of a run, so that drawing it would put
    /// text in the file that no reader of the page can see and every reader of the text can.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A clip in a content stream hides ink; it does not remove the glyphs from the stream. So
    /// a <c>W n</c> followed by a <c>Tj</c> outside it produces a page that looks right and
    /// extracts wrongly — <c>pdftotext</c> reads the whole of it. That is the inverse of the
    /// defect a word-count comparison usually catches, and it is invisible to every pixel
    /// metric.
    /// </para>
    /// <para>
    /// It is also a disagreement between our own two backends rather than a difference of
    /// opinion with anyone else's: the raster sink hands the same clip to Skia, which drops
    /// the glyphs outright, so one display list produced a picture with the text and a PDF
    /// whose text layer had it. What forces the case in practice is a metafile whose records
    /// draw well beyond the bounds its own header declares, played into a picture frame that
    /// is scaled from those bounds — a whole journal page arriving inside a 95 × 33 mm
    /// banner.
    /// </para>
    /// <para>
    /// Every approximation here is in the direction of drawing too much: the clip is
    /// remembered as a bounding box, the run's box is inflated by an em on each side and by
    /// two ems above the baseline, and only a run that misses that enlarged box entirely is
    /// dropped.
    /// </para>
    /// </remarks>
    private bool Hidden(GlyphRun run)
    {
        if (_clip is not { } clip) return false;

        Length minX = run.Origin.X, maxX = run.Origin.X;
        Length minY = run.Origin.Y, maxY = run.Origin.Y;

        foreach (PositionedGlyph glyph in run.Glyphs)
        {
            Length x = run.Origin.X + glyph.Offset.X;
            Length y = run.Origin.Y + glyph.Offset.Y;

            if (x < minX) minX = x;
            if (x + glyph.Advance > maxX) maxX = x + glyph.Advance;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }

        // The glyph positions describe baselines, so the box has to be grown to the face's
        // extent. Two ems up and one down covers any face's ascent and descent with room to
        // spare, which is the point: an over-large box keeps text, and a tight one loses it.
        Length pad = run.FontSize;
        DocRect box = new(
            minX - pad,
            minY - (pad * 2),
            (maxX - minX) + (pad * 2),
            (maxY - minY) + (pad * 3));

        return !Mapped(box, _ctm).IntersectsWith(clip);
    }

    /// <summary>The bounding box of a rectangle carried through a transform.</summary>
    /// <remarks>
    /// The four corners rather than the two opposite ones, because a rotation or a shear moves
    /// each corner independently and taking two of them would describe a rectangle the shape
    /// does not fit inside.
    /// </remarks>
    private static DocRect Mapped(DocRect rect, AffineTransform transform)
    {
        if (transform.IsIdentity) return rect;

        double left = double.PositiveInfinity, top = double.PositiveInfinity;
        double right = double.NegativeInfinity, bottom = double.NegativeInfinity;

        foreach ((Length cx, Length cy) in (ReadOnlySpan<(Length, Length)>)
                 [(rect.Left, rect.Top), (rect.Right, rect.Top),
                  (rect.Right, rect.Bottom), (rect.Left, rect.Bottom)])
        {
            double x = (transform.A * cx.Emu) + (transform.C * cy.Emu) + transform.E;
            double y = (transform.B * cx.Emu) + (transform.D * cy.Emu) + transform.F;

            left = Math.Min(left, x);
            right = Math.Max(right, x);
            top = Math.Min(top, y);
            bottom = Math.Max(bottom, y);
        }

        return new DocRect(
            Length.FromEmu(Clamp(left)), Length.FromEmu(Clamp(top)),
            Length.FromEmu(Clamp(right) - Clamp(left)), Length.FromEmu(Clamp(bottom) - Clamp(top)));

        // A degenerate matrix — a scale of zero, an infinity arriving from a metafile — would
        // otherwise overflow the cast and produce a box on the wrong side of the page.
        static long Clamp(double value)
            => double.IsFinite(value) ? (long)Math.Clamp(value, long.MinValue / 4d, long.MaxValue / 4d) : 0;
    }

    /// <summary>The overlap of two rectangles, or an empty one where they do not meet.</summary>
    private static DocRect Intersect(DocRect a, DocRect b)
    {
        Length left = a.Left > b.Left ? a.Left : b.Left;
        Length top = a.Top > b.Top ? a.Top : b.Top;
        Length right = a.Right < b.Right ? a.Right : b.Right;
        Length bottom = a.Bottom < b.Bottom ? a.Bottom : b.Bottom;

        return right <= left || bottom <= top
            ? DocRect.Empty
            : new DocRect(left, top, right - left, bottom - top);
    }

    // -------------------------------------------------------------------------------- fills

    /// <summary>
    /// Fills a path with a gradient.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Clip, then <c>sh</c>, which is the form LibreOffice writes
    /// (<c>PDFWriterImpl::drawGradient</c>, <c>vcl/source/pdf/pdfwriter_impl.cxx:9194</c>) and
    /// not the shading <em>pattern</em> the same picture could be spelled as. The reason is
    /// the transform: a pattern's <c>/Matrix</c> maps pattern space to the page's default
    /// space and so ignores any <c>cm</c> in force, which would leave a rotated shape's
    /// gradient pointing the wrong way, whereas <c>sh</c> paints in the current user space
    /// and inherits it for nothing.
    /// </para>
    /// <para>
    /// A gradient's own <see cref="GradientPaint.Transform"/> — what expresses a squashed or
    /// rotated ramp without distorting the shape under it — is then simply one more <c>cm</c>
    /// inside the clip.
    /// </para>
    /// </remarks>
    private void FillGradient(GraphicsPath path, GradientPaint gradient, FillRule rule)
    {
        if (!Fills.Gradients.HasNativeForm(gradient.Kind))
        {
            Fills.Gradients.DrawBands(this, path, gradient, rule);
            return;
        }

        // What the shading has to cover, carried back through the gradient's own transform,
        // because that transform is applied inside the clip below. Only a repeating spread
        // reads it; a padded one covers the plane by /Extend alone.
        DocRect? extent = gradient.Spread == SpreadMethod.Pad || Fills.Gradients.Bounds(path) is not { } box
            ? null
            : Fills.Gradients.Untransformed(box, gradient.Transform);

        string name = Shading(gradient, alphaOnly: false, extent);

        _content.Append("q\n");
        AppendTransparency(path, gradient, rule, extent);
        AppendClip(path, rule);

        if (gradient.Transform != AffineTransform.Identity) AppendTransform(gradient.Transform);

        _content.Append(CultureInfo.InvariantCulture, $"/{name} sh\nQ\n");
    }

    /// <summary>
    /// Fills a path with a triangle mesh: a <c>/ShadingType 4</c> painted inside the path as a
    /// clip, exactly as a gradient is.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The clip is what makes the mesh a <em>fill</em> rather than a picture in its own right.
    /// A path-gradient brush's boundary and the shape it fills need not be the same polygon —
    /// GDI+ lets a brush built from a star fill a rectangle — so the triangles are painted
    /// wherever they lie and the path decides how much of that is seen.
    /// </para>
    /// <para>
    /// A mesh with a translucent vertex takes the same luminosity soft mask a fading gradient
    /// does, and for the same reason: a shading's colour space is <c>DeviceRGB</c> and has no
    /// alpha, so the alpha has to be a second shading. That the two share
    /// <see cref="SoftMask"/> is what keeps a faded mesh and a faded gradient from disagreeing.
    /// </para>
    /// </remarks>
    private void FillMesh(GraphicsPath path, MeshPaint mesh, FillRule rule)
    {
        int id = PdfShadings.WriteMesh(writer, mesh, _pageHeight, alphaOnly: false);
        if (id == 0) return;

        string name = string.Create(CultureInfo.InvariantCulture, $"Sh{_shadings.Count + 1}");
        _shadings.Add((name, id));

        _content.Append("q\n");

        if (Fills.Meshes.Fades(mesh)
            && PdfShadings.WriteMesh(writer, mesh, _pageHeight, alphaOnly: true) is > 0 and int alpha)
        {
            string mask = string.Create(CultureInfo.InvariantCulture, $"Sh{_shadings.Count + 1}");
            _shadings.Add((mask, alpha));

            StringBuilder inner = new();
            StringBuilder outer = _content;
            _content = inner;

            AppendClip(path, rule);
            _content.Append(CultureInfo.InvariantCulture, $"/{mask} sh\n");

            _content = outer;
            SoftMask(inner);
        }

        AppendClip(path, rule);
        _content.Append(CultureInfo.InvariantCulture, $"/{name} sh\nQ\n");
    }

    /// <summary>Names a shading, adding it to the page's resources.</summary>
    private string Shading(GradientPaint gradient, bool alphaOnly, DocRect? extent)
    {
        int id = PdfShadings.Write(writer, gradient, _pageHeight, alphaOnly, extent);
        string name = string.Create(CultureInfo.InvariantCulture, $"Sh{_shadings.Count + 1}");
        _shadings.Add((name, id));
        return name;
    }

    /// <summary>
    /// States a gradient's transparency, if it has any.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A PDF shading has no alpha channel — its colour space is <c>DeviceRGB</c> and that is
    /// all — so a gradient that fades has to say so somewhere else. Where depends on what it
    /// does: one alpha shared by every stop is a constant <c>/ca</c> in an <c>ExtGState</c>,
    /// which costs one small object; an alpha that <em>varies</em> along the ramp needs a
    /// luminosity soft mask, which is a second shading in <c>DeviceGray</c> painted into a
    /// transparency group whose brightness the mask reads as the alpha.
    /// </para>
    /// <para>
    /// This is not a refinement. The raster backend honours a stop's alpha for nothing, because
    /// a Skia shader's colours carry one, so without this the same <see cref="GradientPaint"/>
    /// would fade on a PNG and be opaque in a PDF — two pictures from one display list, which is
    /// the failure the shared band decomposition exists to prevent elsewhere.
    /// </para>
    /// <para>
    /// <c>/BC [0]</c> makes everything outside the group's own bounding box fully masked, so
    /// only what the mask actually paints shows through. The group states
    /// <c>/CS /DeviceGray</c> to match, since a luminosity mask reads brightness.
    /// </para>
    /// </remarks>
    private void AppendTransparency(
        GraphicsPath path, GradientPaint gradient, FillRule rule, DocRect? extent)
    {
        IReadOnlyList<GradientStop> stops = Fills.Gradients.Normalise(gradient.Stops);

        byte first = stops[0].Colour.A;
        bool uniform = true;
        foreach (GradientStop stop in stops)
        {
            if (stop.Colour.A != first) { uniform = false; break; }
        }

        if (uniform)
        {
            AppendAlpha(first, stroking: false);
            return;
        }

        string mask = Shading(gradient, alphaOnly: true, extent);

        StringBuilder inner = new();
        StringBuilder outer = _content;
        _content = inner;

        AppendClip(path, rule);
        if (gradient.Transform != AffineTransform.Identity) AppendTransform(gradient.Transform);
        _content.Append(CultureInfo.InvariantCulture, $"/{mask} sh\n");

        _content = outer;

        SoftMask(inner);
    }

    /// <summary>
    /// Turns a written mask stream into a luminosity soft mask and puts it in force.
    /// </summary>
    /// <remarks>
    /// Shared by the two paints that carry their own alpha and cannot say so in
    /// <c>DeviceRGB</c> — a fading gradient and a mesh with a translucent vertex. <c>/BC [0]</c>
    /// makes everything outside the group's bounding box fully masked, so only what the mask
    /// actually paints shows through, and the group states <c>/CS /DeviceGray</c> because a
    /// luminosity mask reads brightness.
    /// </remarks>
    private void SoftMask(StringBuilder inner)
    {
        int form = writer.Reserve();
        string name = string.Create(CultureInfo.InvariantCulture, $"Fm{_xObjects.Count + 1}");
        _xObjects.Add((name, form));

        writer.SetStream(
            form,
            "/Type/XObject/Subtype/Form/FormType 1"
            + $"/BBox[0 0 {N(_size.Width.Points)} {N(_pageHeight)}]"
            + "/Group<</Type/Group/S/Transparency/CS/DeviceGray>>"
            + $"/Resources {ResourcesPlaceholder} 0 R",
            Encoding.Latin1.GetBytes(inner.ToString()),
            compress: true);

        string state = string.Create(CultureInfo.InvariantCulture, $"GS{_states.Count + 1}");
        _states.Add((state, writer.Add(
            $"<</Type/ExtGState/SMask<</S/Luminosity/G {form} 0 R/BC[0]>>>>")));

        _content.Append(CultureInfo.InvariantCulture, $"/{state} gs\n");
    }

    /// <summary>
    /// Fills a path with a bitmap, tiled or stretched.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One <c>Do</c> per tile inside a clip, not a <c>/PatternType 1</c> tiling pattern.
    /// That is what LibreOffice writes — measured on its own PDF of
    /// <c>tests/corpus/features/paint-fills.fodp</c>, whose one checkerboard rectangle comes
    /// out as a <c>re W* n</c> clip and 47 <c>q … cm /Im10 Do Q</c> groups sharing a single
    /// 8×8 image XObject — and it has the same advantage the <c>sh</c> form has: an explicit
    /// tile inherits the current transform, where a pattern matrix does not.
    /// </para>
    /// <para>
    /// The image itself is written once however many tiles name it, which is what makes the
    /// difference between 47 copies of a bitmap and one.
    /// </para>
    /// </remarks>
    private void FillBitmap(GraphicsPath path, BitmapPaint bitmap, FillRule rule)
    {
        if (Empty(bitmap.Image)) return;
        if (Fills.Gradients.Bounds(path) is not { } bounds || bounds.IsEmpty) return;

        string name = ImageName(bitmap.Image);
        if (name.Length == 0) return;

        _content.Append("q\n");
        AppendClip(path, rule);
        AppendAlpha(
            (byte)Math.Clamp(Math.Round(bitmap.Opacity * 255), 0, 255), stroking: false);

        foreach (DocRect tile in Fills.Tiles.Cover(bitmap, bounds))
        {
            _content.Append(CultureInfo.InvariantCulture,
                $"q {N(tile.Width.Points)} 0 0 {N(tile.Height.Points)} "
                + $"{N(tile.Left.Points)} {N(_pageHeight - tile.Bottom.Points)} cm\n/{name} Do Q\n");
        }

        _content.Append("Q\n");
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

    /// <summary>The resource name of an image, written once however many times it is drawn.</summary>
    private string ImageName(RasterImage image)
    {
        if (_imageNames.TryGetValue(image, out string? existing)) return existing;

        string name = PdfImages.Write(writer, image, options, _xObjects);
        _imageNames[image] = name;
        return name;
    }

    /// <inheritdoc/>
    public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0)
    {
        ArgumentNullException.ThrowIfNull(image);
        // Not `image.Width <= 0`: a reader may hand over an image it has not decoded, and an
        // undecoded one reports no size until a codec has looked at it. Testing the size here
        // dropped every `RasterImage.Encoded` silently — laying out correctly and drawing nothing.
        if (Empty(image) || destination.IsEmpty) return;

        string name = ImageName(image);
        if (name.Length == 0) return;

        _content.Append("q\n");
        if (opacity < 1.0) AppendAlpha((byte)Math.Clamp(Math.Round(opacity * 255), 0, 255), stroking: false);

        // An image XObject draws into the unit square, so the transform is the destination
        // rectangle: width and height on the diagonal, and the bottom-left corner as the offset.
        _content.Append(CultureInfo.InvariantCulture,
            $"{N(destination.Width.Points)} 0 0 {N(destination.Height.Points)} "
            + $"{N(destination.Left.Points)} {N(_pageHeight - destination.Bottom.Points)} cm\n");

        _content.Append(CultureInfo.InvariantCulture, $"/{name} Do\nQ\n");
    }

    /// <inheritdoc/>
    public void BeginTransparencyGroup(double opacity)
    {
        _groups.Push(_content);
        _content = new StringBuilder();
        _groupOpacities.Push(Math.Clamp(opacity, 0, 1));
    }

    /// <inheritdoc/>
    public void EndTransparencyGroup()
    {
        if (_groups.Count == 0) return;

        string inner = _content.ToString();
        _content = _groups.Pop();
        double opacity = _groupOpacities.Pop();

        // A real transparency group rather than a constant alpha on each member, because the two
        // differ: a group at half opacity shows its own overlaps at full strength and only the
        // whole composite is faded, which is what a shape group's transparency means.
        int form = writer.Reserve();
        string name = string.Create(CultureInfo.InvariantCulture, $"Fm{_xObjects.Count + 1}");
        _xObjects.Add((name, form));

        writer.SetStream(
            form,
            "/Type/XObject/Subtype/Form/FormType 1"
            + $"/BBox[0 0 {N(_size.Width.Points)} {N(_pageHeight)}]"
            + "/Group<</Type/Group/S/Transparency/CS/DeviceRGB>>"
            + $"/Resources {ResourcesPlaceholder} 0 R",
            Encoding.Latin1.GetBytes(inner),
            compress: true);

        _content.Append(CultureInfo.InvariantCulture, $"q\n/{StateFor(opacity, both: true)} gs\n/{name} Do\nQ\n");
    }

    /// <summary>
    /// The object number the shared resource dictionary will take.
    /// </summary>
    /// <remarks>
    /// Reserved by <see cref="PdfRenderer"/> before any page is drawn, precisely so that a
    /// form XObject written mid-page can name it. One dictionary serves every page and every
    /// form, which is what LibreOffice's export does too — its page tree and its pages both
    /// point at the same <c>/Resources</c>.
    /// </remarks>
    public int ResourcesPlaceholder { get; set; }

    /// <summary>Appends the <c>/XObject</c> and <c>/ExtGState</c> entries to the shared resources.</summary>
    public void WriteResources(StringBuilder resources)
    {
        ArgumentNullException.ThrowIfNull(resources);

        Append(resources, "/XObject", _xObjects);
        Append(resources, "/ExtGState", _states);
        Append(resources, "/Shading", _shadings);

        static void Append(StringBuilder into, string key, List<(string Name, int Id)> entries)
        {
            if (entries.Count == 0) return;

            into.Append(key).Append("<<");
            foreach ((string name, int id) in entries)
            {
                into.Append(CultureInfo.InvariantCulture, $"/{name} {id} 0 R");
            }

            into.Append(">>");
        }
    }

    // -------------------------------------------------------------------------------- text

    /// <summary>
    /// How far the next stretch of glyphs runs that can share one pen placement.
    /// </summary>
    /// <remarks>
    /// A vertical offset breaks the stretch and a horizontal one does not, which is not an
    /// asymmetry in the format so much as one in what PDF can express cheaply: a horizontal
    /// difference is a number in the middle of a <c>TJ</c> array, and a vertical one needs a
    /// new text position. Marks placed above or below a base glyph are the case that needs it.
    /// A font change breaks it too, since a face's glyphs beyond the 255th live in a second
    /// PDF font.
    /// </remarks>
    private (string Resource, int End) Segment(GlyphRun run, string?[] texts, int start)
    {
        (string resource, _) = fonts.Map(run.Font, run.Glyphs[start].GlyphId, texts[start]);
        Length y = run.Glyphs[start].Offset.Y;

        int end = start + 1;
        while (end < run.Glyphs.Count)
        {
            if (run.Glyphs[end].Offset.Y != y) break;

            (string next, _) = fonts.Map(run.Font, run.Glyphs[end].GlyphId, texts[end]);
            if (!string.Equals(next, resource, StringComparison.Ordinal)) break;

            end++;
        }

        return (resource, end);
    }

    /// <summary>Writes one stretch of glyphs, correcting the pen wherever it has drifted.</summary>
    private void AppendShow(GlyphRun run, string?[] texts, int start, int end, double originX, double size)
    {
        StringBuilder items = new();
        StringBuilder hex = new();
        bool adjusted = false;
        double pen = originX;

        for (int i = start; i < end; i++)
        {
            PositionedGlyph glyph = run.Glyphs[i];
            double wanted = (run.Origin.X + glyph.Offset.X).Points;
            double drift = wanted - pen;

            if (Math.Abs(drift) > PenTolerancePoints && size > 0)
            {
                if (hex.Length > 0) { items.Append('<').Append(hex).Append('>'); hex.Clear(); }

                // A number in a TJ array moves the pen backwards by its own thousandth of an em,
                // so a glyph that should sit further right takes a negative one.
                items.Append(N(-drift * 1000.0 / size)).Append(' ');
                pen = wanted;
                adjusted = true;
            }

            (_, byte code) = fonts.Map(run.Font, glyph.GlyphId, texts[i]);
            hex.Append(code.ToString("X2", CultureInfo.InvariantCulture));
            pen += size * fonts.Width(run.Font, glyph.GlyphId) / 1000.0;
        }

        if (hex.Length > 0) items.Append('<').Append(hex).Append('>');

        _content.Append(adjusted ? $"[{items}]TJ\n" : $"{items}Tj\n");
    }

    /// <summary>
    /// What each glyph stands for, or null when an earlier glyph already said it.
    /// </summary>
    /// <remarks>
    /// A cluster is a stretch of text and a stretch of glyphs, and the two need not be the
    /// same length: a ligature is one glyph for several characters, and a decomposition is
    /// several glyphs for one. So the text belonging to a cluster is found from the cluster
    /// boundaries rather than assumed to be one character, and it is attributed to the first
    /// glyph of the cluster — giving it to all of them would make the word appear twice in
    /// anything that extracts the text back out.
    /// </remarks>
    private static string?[] ClusterTexts(GlyphRun run)
    {
        string?[] texts = new string?[run.Glyphs.Count];
        if (run.ClusterMap.Count != run.Glyphs.Count) return texts;

        List<int> starts = [.. new SortedSet<int>(run.ClusterMap)];
        HashSet<int> seen = [];

        for (int i = 0; i < run.Glyphs.Count; i++)
        {
            int cluster = run.ClusterMap[i];
            if (cluster < 0 || cluster >= run.Text.Length) continue;
            if (!seen.Add(cluster)) continue;

            int index = starts.BinarySearch(cluster);
            int end = index >= 0 && index + 1 < starts.Count ? starts[index + 1] : run.Text.Length;
            if (end <= cluster || end > run.Text.Length) end = Math.Min(cluster + 1, run.Text.Length);

            texts[i] = run.Text[cluster..end];
        }

        return texts;
    }

    // -------------------------------------------------------------------------------- paths

    private void AppendPath(GraphicsPath path)
    {
        if (Rectangle(path) is { } rect)
        {
            _content.Append(CultureInfo.InvariantCulture,
                $"{N(rect.Left.Points)} {N(_pageHeight - rect.Bottom.Points)} "
                + $"{N(rect.Width.Points)} {N(rect.Height.Points)} re\n");
            return;
        }

        foreach (PathCommand command in path.Commands)
        {
            switch (command.Verb)
            {
                case PathVerb.MoveTo:
                    _content.Append(CultureInfo.InvariantCulture, $"{X(command.Point)} m\n");
                    break;
                case PathVerb.LineTo:
                    _content.Append(CultureInfo.InvariantCulture, $"{X(command.Point)} l\n");
                    break;
                case PathVerb.CubicTo:
                    _content.Append(CultureInfo.InvariantCulture,
                        $"{X(command.Control1)} {X(command.Control2)} {X(command.Point)} c\n");
                    break;
                case PathVerb.Close:
                default:
                    _content.Append("h\n");
                    break;
            }
        }
    }

    /// <summary>
    /// The axis-aligned rectangle a path is, or null when it is anything else.
    /// </summary>
    /// <remarks>
    /// Worth detecting rather than emitting four line segments, because <c>re</c> is how a
    /// rectangle is spelled in every PDF a reader has seen, including LibreOffice's — and
    /// the fills the harness reads are rectangles. A shade, a footnote separator and a
    /// frame's background are all one of these.
    /// </remarks>
    private static DocRect? Rectangle(GraphicsPath path)
    {
        List<DocPoint> points = [];
        bool closed = false;

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.MoveTo && points.Count == 0) points.Add(command.Point);
            else if (command.Verb == PathVerb.LineTo && points.Count > 0 && !closed) points.Add(command.Point);
            else if (command.Verb == PathVerb.Close && points.Count >= 4 && !closed) closed = true;
            else return null;
        }

        if (!closed) return null;

        // Five points is the same rectangle with its first corner repeated, which is how a
        // metafile states one; both spellings reach here.
        if (points.Count == 5 && points[4] == points[0]) points.RemoveAt(4);
        if (points.Count != 4) return null;

        bool horizontalFirst = points[0].Y == points[1].Y && points[1].X == points[2].X
                               && points[2].Y == points[3].Y && points[3].X == points[0].X;
        bool verticalFirst = points[0].X == points[1].X && points[1].Y == points[2].Y
                             && points[2].X == points[3].X && points[3].Y == points[0].Y;

        if (!horizontalFirst && !verticalFirst) return null;

        Length left = points[0].X, right = points[0].X, top = points[0].Y, bottom = points[0].Y;
        foreach (DocPoint point in points)
        {
            if (point.X < left) left = point.X;
            if (point.X > right) right = point.X;
            if (point.Y < top) top = point.Y;
            if (point.Y > bottom) bottom = point.Y;
        }

        return new DocRect(left, top, right - left, bottom - top);
    }

    private void AppendDash(Stroke stroke)
    {
        if (stroke.DashPattern is not { Count: > 0 } pattern) return;

        StringBuilder dashes = new("[");
        for (int i = 0; i < pattern.Count; i++)
        {
            if (i > 0) dashes.Append(' ');
            dashes.Append(N(pattern[i].Points));
        }

        _content.Append(CultureInfo.InvariantCulture,
            $"{dashes}] {N(stroke.DashOffset.Points)} d\n");
    }

    // ------------------------------------------------------------------------------- paints

    /// <summary>
    /// The one colour a paint is drawn as where only one is available.
    /// </summary>
    /// <remarks>
    /// Fills no longer come here — a gradient is a shading and a bitmap is a grid of image
    /// draws — but a <em>pen</em> and a glyph run still do, and both are one colour in PDF:
    /// there is no gradient stroke operator, and text is shown in the current fill colour.
    /// LibreOffice's own writer has neither either. A gradient pen is drawn as its middle
    /// stop, which is the closest single colour to the ramp, and a bitmap pen as nothing.
    /// </remarks>
    private static Colour Flatten(Paint paint) => paint switch
    {
        SolidPaint solid => solid.Colour,
        GradientPaint { Stops.Count: > 0 } gradient
            => Fills.Gradients.Sample(Fills.Gradients.Normalise(gradient.Stops), 0.5),
        MeshPaint mesh => Fills.Meshes.Average(mesh),
        _ => Colour.Transparent,
    };

    /// <summary>Names an <c>ExtGState</c> holding a constant alpha, reusing one when it exists.</summary>
    /// <remarks>
    /// Reused rather than written per use because a document with a half-transparent table
    /// shade in every row would otherwise carry one object per row, all identical.
    /// </remarks>
    private string StateFor(double alpha, bool both)
    {
        string entries = both ? $"/ca {N(alpha)}/CA {N(alpha)}" : $"/ca {N(alpha)}";
        if (_stateNames.TryGetValue(entries, out string? existing)) return existing;

        string name = string.Create(CultureInfo.InvariantCulture, $"GS{_states.Count + 1}");
        _states.Add((name, writer.Add($"<</Type/ExtGState{entries}>>")));
        _stateNames[entries] = name;
        return name;
    }

    private void AppendAlpha(byte alpha, bool stroking)
    {
        if (alpha == 255) return;

        string name = stroking
            ? StateFor(alpha / 255.0, both: true)
            : StateFor(alpha / 255.0, both: false);

        _content.Append(CultureInfo.InvariantCulture, $"/{name} gs\n");
    }

    // ------------------------------------------------------------------------------ helpers

    private static string N(double value) => PdfSyntax.Number(value);

    private string X(DocPoint point)
        => string.Create(
            CultureInfo.InvariantCulture,
            $"{N(point.X.Points)} {N(_pageHeight - point.Y.Points)}");
}

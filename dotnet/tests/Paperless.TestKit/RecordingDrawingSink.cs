using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.TestKit;

/// <summary>
/// An <see cref="IDrawingSink"/> that records what it was asked to draw.
/// </summary>
/// <remarks>
/// <para>
/// The point of a display list is that it can be inspected instead of rasterised, and this is what makes
/// that useful in a test: what a page emits can be compared with what a reference renderer put on the
/// page without either side producing pixels. A comparison of images answers "does it look the same",
/// which is a weaker question and a much harder one to attribute — a shifted baseline and a wrong glyph
/// look equally different.
/// </para>
/// <para>
/// Deliberately not a full implementation of the geometry: transforms and clips are recorded as calls
/// rather than applied, because layout emits nothing that needs them and a test asserting on a
/// transformed coordinate would be asserting on this class rather than on the code under test.
/// </para>
/// </remarks>
public sealed class RecordingDrawingSink : IDrawingSink
{
    private readonly List<DrawnPage> _pages = [];
    private DrawnPage? _current;

    /// <summary>The pages drawn, in order.</summary>
    public IReadOnlyList<DrawnPage> Pages => _pages;

    /// <summary>How many times a page was left open when the next began.</summary>
    /// <remarks>
    /// A test's canary for a drawing path that forgets <see cref="EndPage"/>: one missed call nests every
    /// following page inside the last, which is invisible in the recorded content and fatal in a real
    /// backend.
    /// </remarks>
    public int UnclosedPages { get; private set; }

    /// <inheritdoc/>
    public void BeginPage(DocSize size)
    {
        if (_current is not null) UnclosedPages++;
        _current = new DrawnPage(size);
    }

    /// <inheritdoc/>
    public void EndPage()
    {
        if (_current is null) return;

        _pages.Add(_current);
        _current = null;
    }

    /// <inheritdoc/>
    public void DrawGlyphRun(GlyphRun run, Paint paint)
    {
        ArgumentNullException.ThrowIfNull(run);
        _current?.Runs.Add(new DrawnGlyphRun(run, paint));
    }

    /// <inheritdoc/>
    public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0)
    {
        ArgumentNullException.ThrowIfNull(image);
        _current?.Images.Add(destination);
    }

    /// <inheritdoc/>
    public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero)
    {
        ArgumentNullException.ThrowIfNull(path);

        _current?.Fills.Add(paint);
        _current?.FilledPaths.Add(new DrawnFill(Bounds(path), paint));
    }

    /// <summary>
    /// A path's bounding box, which is all a comparison needs of the shapes drawn so far.
    /// </summary>
    /// <remarks>
    /// Everything Paperless fills today is a rectangle — a separator rule, a cell's shading — so the bounds
    /// are the shape rather than an approximation of it. A curve would be flattened to its control points'
    /// extent, which is wrong for a curve and honest about being a test harness rather than a rasteriser.
    /// </remarks>
    private static DocRect Bounds(GraphicsPath path)
    {
        Length left = default;
        Length top = default;
        Length right = default;
        Length bottom = default;
        bool any = false;

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            foreach (DocPoint point in Points(command))
            {
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
        }

        return any
            ? new DocRect(left, top, right - left, bottom - top)
            : default;
    }

    private static IEnumerable<DocPoint> Points(PathCommand command)
    {
        yield return command.Point;

        if (command.Verb != PathVerb.CubicTo) yield break;

        yield return command.Control1;
        yield return command.Control2;
    }

    /// <inheritdoc/>
    public void StrokePath(GraphicsPath path, Stroke stroke)
    {
        ArgumentNullException.ThrowIfNull(path);

        _current?.Strokes.Add(stroke);
        _current?.StrokedPaths.Add(new DrawnStroke(Bounds(path), stroke));
    }

    /// <inheritdoc/>
    public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero) => Clips++;

    /// <inheritdoc/>
    public void Transform(AffineTransform transform) => Transforms++;

    /// <inheritdoc/>
    public void Save() => Saves++;

    /// <inheritdoc/>
    public void Restore() => Restores++;

    /// <inheritdoc/>
    public void BeginTransparencyGroup(double opacity) => TransparencyGroups++;

    /// <inheritdoc/>
    public void EndTransparencyGroup() { }

    /// <summary>How many clips were pushed.</summary>
    public int Clips { get; private set; }

    /// <summary>How many transforms were applied.</summary>
    public int Transforms { get; private set; }

    /// <summary>How many states were saved.</summary>
    public int Saves { get; private set; }

    /// <summary>How many were restored.</summary>
    public int Restores { get; private set; }

    /// <summary>How many transparency groups were opened.</summary>
    public int TransparencyGroups { get; private set; }
}

/// <summary>One page's worth of recorded drawing.</summary>
/// <param name="Size">The page size the drawing path declared.</param>
public sealed record DrawnPage(DocSize Size)
{
    /// <summary>The glyph runs, in the order they were drawn.</summary>
    public List<DrawnGlyphRun> Runs { get; } = [];

    /// <summary>Where images were placed.</summary>
    public List<DocRect> Images { get; } = [];

    /// <summary>The paints paths were filled with.</summary>
    public List<Paint> Fills { get; } = [];

    /// <summary>The filled paths, in order, paired with the paint each was filled with.</summary>
    /// <remarks>
    /// The geometry as well as the paint, because a fill's *position* is the interesting half for everything
    /// drawn that is not text — a footnote separator, a cell border, a shaded row. <see cref="Fills"/> keeps
    /// only the paints and predates anything caring where they went.
    /// </remarks>
    public List<DrawnFill> FilledPaths { get; } = [];

    /// <summary>The stroked paths, in order, paired with the pen each was stroked with.</summary>
    /// <remarks>
    /// As with <see cref="FilledPaths"/>, the geometry as well as the pen: what a cell border needs checking on
    /// is where the line ran and how far, and a table's borders are drawn consolidated — one stroke per grid
    /// line — so the extents are the evidence that the consolidation matched.
    /// </remarks>
    public List<DrawnStroke> StrokedPaths { get; } = [];

    /// <summary>The strokes paths were drawn with.</summary>
    public List<Stroke> Strokes { get; } = [];
}

/// <summary>One recorded filled path, as its bounding box and the paint it was filled with.</summary>
/// <param name="Bounds">The path's extent, which for everything drawn so far <em>is</em> the shape.</param>
/// <param name="Paint">What it was filled with.</param>
public readonly record struct DrawnFill(DocRect Bounds, Paint Paint);

/// <summary>One recorded stroked path, as its bounding box and the pen it was stroked with.</summary>
/// <param name="Bounds">The path's extent; for a straight line, the line.</param>
/// <param name="Stroke">The pen it was stroked with.</param>
public readonly record struct DrawnStroke(DocRect Bounds, Stroke Stroke);

/// <summary>One recorded glyph run and the paint it was drawn with.</summary>
/// <param name="Run">The run.</param>
/// <param name="Paint">The paint.</param>
public sealed record DrawnGlyphRun(GlyphRun Run, Paint Paint)
{
    /// <summary>The run's text, for a test that wants to say which line it is asserting on.</summary>
    public string Text => Run.Text;

    /// <summary>Where the run's baseline starts.</summary>
    public DocPoint Origin => Run.Origin;

    /// <summary>The sum of the run's advances: how wide the drawn text is.</summary>
    public Core.Units.Length Width
    {
        get
        {
            Core.Units.Length total = Core.Units.Length.Zero;
            foreach (PositionedGlyph glyph in Run.Glyphs) total += glyph.Advance;
            return total;
        }
    }
}

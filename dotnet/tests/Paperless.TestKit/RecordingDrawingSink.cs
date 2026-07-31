using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

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
        => _current?.Fills.Add(paint);

    /// <inheritdoc/>
    public void StrokePath(GraphicsPath path, Stroke stroke) => _current?.Strokes.Add(stroke);

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

    /// <summary>The strokes paths were drawn with.</summary>
    public List<Stroke> Strokes { get; } = [];
}

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

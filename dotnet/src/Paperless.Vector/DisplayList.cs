using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.Vector;

/// <summary>
/// A recorded sequence of <see cref="IDrawingSink"/> calls, replayable into any sink.
/// </summary>
/// <remarks>
/// <para>
/// This is what "decoded" means for a vector image. A raster image decodes to pixels; a
/// vector image decodes to <em>drawing commands</em>, because that is the only form that
/// survives being scaled to a print-resolution PDF. Rasterising at decode time would throw
/// away the thing the format was chosen for.
/// </para>
/// <para>
/// Recording rather than replaying straight through matters for two reasons. The same
/// picture is commonly drawn many times — a logo on every slide master, a bullet glyph on
/// every list item — and decoding once and replaying costs nothing per use. And the
/// commands can be counted, which is how <see cref="VectorLimits"/> bounds work that
/// arrives from an untrusted document.
/// </para>
/// <para>
/// It is deliberately an <see cref="IDrawingSink"/> itself, so a decoder writes to the same
/// interface a backend implements and needs no second output path.
/// </para>
/// </remarks>
public sealed class DisplayList : IDrawingSink
{
    private readonly List<DrawingCommand> _commands = [];

    /// <summary>The recorded commands, in order.</summary>
    public IReadOnlyList<DrawingCommand> Commands => _commands;

    /// <summary>How many commands have been recorded.</summary>
    public int Count => _commands.Count;

    /// <summary>Replays every recorded command into another sink.</summary>
    /// <param name="sink">Where to replay.</param>
    public void Replay(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);

        foreach (DrawingCommand command in _commands) command.ApplyTo(sink);
    }

    /// <inheritdoc/>
    public void BeginPage(DocSize size) => _commands.Add(new BeginPageCommand(size));

    /// <inheritdoc/>
    public void EndPage() => _commands.Add(EndPageCommand.Instance);

    /// <inheritdoc/>
    public void Save() => _commands.Add(SaveCommand.Instance);

    /// <inheritdoc/>
    public void Restore() => _commands.Add(RestoreCommand.Instance);

    /// <inheritdoc/>
    public void Transform(AffineTransform transform) => _commands.Add(new TransformCommand(transform));

    /// <inheritdoc/>
    public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero)
    {
        ArgumentNullException.ThrowIfNull(path);
        _commands.Add(new ClipCommand(path, rule));
    }

    /// <inheritdoc/>
    public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero)
    {
        ArgumentNullException.ThrowIfNull(path);
        ArgumentNullException.ThrowIfNull(paint);
        _commands.Add(new FillCommand(path, paint, rule));
    }

    /// <inheritdoc/>
    public void StrokePath(GraphicsPath path, Stroke stroke)
    {
        ArgumentNullException.ThrowIfNull(path);
        ArgumentNullException.ThrowIfNull(stroke);
        _commands.Add(new StrokeCommand(path, stroke));
    }

    /// <inheritdoc/>
    public void DrawGlyphRun(GlyphRun run, Paint paint)
    {
        ArgumentNullException.ThrowIfNull(run);
        ArgumentNullException.ThrowIfNull(paint);
        _commands.Add(new GlyphRunCommand(run, paint));
    }

    /// <inheritdoc/>
    public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0)
    {
        ArgumentNullException.ThrowIfNull(image);
        _commands.Add(new ImageCommand(image, destination, opacity));
    }

    /// <inheritdoc/>
    public void BeginTransparencyGroup(double opacity) => _commands.Add(new BeginGroupCommand(opacity));

    /// <inheritdoc/>
    public void EndTransparencyGroup() => _commands.Add(EndGroupCommand.Instance);
}

/// <summary>One recorded <see cref="IDrawingSink"/> call.</summary>
/// <remarks>
/// A closed hierarchy — every case is declared in this file — so a consumer can switch over
/// it exhaustively. It mirrors <see cref="IDrawingSink"/> one case per method rather than
/// inventing a second vocabulary, because two vocabularies would drift.
/// </remarks>
public abstract record DrawingCommand
{
    /// <summary>Issues this command to a sink.</summary>
    public abstract void ApplyTo(IDrawingSink sink);
}

/// <summary>A recorded <see cref="IDrawingSink.BeginPage"/>.</summary>
/// <param name="Size">The page size.</param>
public sealed record BeginPageCommand(DocSize Size) : DrawingCommand
{
    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.BeginPage(Size);
    }
}

/// <summary>A recorded <see cref="IDrawingSink.EndPage"/>.</summary>
public sealed record EndPageCommand : DrawingCommand
{
    /// <summary>The single instance; the command carries no state.</summary>
    public static EndPageCommand Instance { get; } = new();

    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.EndPage();
    }
}

/// <summary>A recorded <see cref="IDrawingSink.Save"/>.</summary>
public sealed record SaveCommand : DrawingCommand
{
    /// <summary>The single instance; the command carries no state.</summary>
    public static SaveCommand Instance { get; } = new();

    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.Save();
    }
}

/// <summary>A recorded <see cref="IDrawingSink.Restore"/>.</summary>
public sealed record RestoreCommand : DrawingCommand
{
    /// <summary>The single instance; the command carries no state.</summary>
    public static RestoreCommand Instance { get; } = new();

    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.Restore();
    }
}

/// <summary>A recorded <see cref="IDrawingSink.Transform"/>.</summary>
/// <param name="Transform">The transform to concatenate.</param>
public sealed record TransformCommand(AffineTransform Transform) : DrawingCommand
{
    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.Transform(Transform);
    }
}

/// <summary>A recorded <see cref="IDrawingSink.ClipPath"/>.</summary>
/// <param name="Path">The clip geometry.</param>
/// <param name="Rule">Which fill rule decides what is inside it.</param>
public sealed record ClipCommand(GraphicsPath Path, FillRule Rule) : DrawingCommand
{
    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.ClipPath(Path, Rule);
    }
}

/// <summary>A recorded <see cref="IDrawingSink.FillPath"/>.</summary>
/// <param name="Path">The geometry.</param>
/// <param name="Paint">What to fill it with.</param>
/// <param name="Rule">Which fill rule decides what is inside it.</param>
public sealed record FillCommand(GraphicsPath Path, Paint Paint, FillRule Rule) : DrawingCommand
{
    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.FillPath(Path, Paint, Rule);
    }
}

/// <summary>A recorded <see cref="IDrawingSink.StrokePath"/>.</summary>
/// <param name="Path">The geometry.</param>
/// <param name="Stroke">The pen.</param>
public sealed record StrokeCommand(GraphicsPath Path, Stroke Stroke) : DrawingCommand
{
    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.StrokePath(Path, Stroke);
    }
}

/// <summary>A recorded <see cref="IDrawingSink.DrawGlyphRun"/>.</summary>
/// <param name="Run">The positioned glyphs.</param>
/// <param name="Paint">What to draw them with.</param>
public sealed record GlyphRunCommand(GlyphRun Run, Paint Paint) : DrawingCommand
{
    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.DrawGlyphRun(Run, Paint);
    }
}

/// <summary>A recorded <see cref="IDrawingSink.DrawImage"/>.</summary>
/// <param name="Image">The raster, decoded or still encoded.</param>
/// <param name="Destination">Where it goes.</param>
/// <param name="Opacity">A uniform opacity multiplier.</param>
public sealed record ImageCommand(RasterImage Image, DocRect Destination, double Opacity) : DrawingCommand
{
    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.DrawImage(Image, Destination, Opacity);
    }
}

/// <summary>A recorded <see cref="IDrawingSink.BeginTransparencyGroup"/>.</summary>
/// <param name="Opacity">The opacity the group composites at.</param>
public sealed record BeginGroupCommand(double Opacity) : DrawingCommand
{
    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.BeginTransparencyGroup(Opacity);
    }
}

/// <summary>A recorded <see cref="IDrawingSink.EndTransparencyGroup"/>.</summary>
public sealed record EndGroupCommand : DrawingCommand
{
    /// <summary>The single instance; the command carries no state.</summary>
    public static EndGroupCommand Instance { get; } = new();

    /// <inheritdoc/>
    public override void ApplyTo(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        sink.EndTransparencyGroup();
    }
}

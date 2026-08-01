using System.Diagnostics;
using System.Globalization;
using Paperless.Core.Diagnostics;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using ShimSkiaSharp;
using CorePathCommand = Paperless.Core.Graphics.PathCommand;

namespace Paperless.Vector.Svg;

/// <summary>
/// Turns <c>ShimSkiaSharp</c>'s canvas-command list into <see cref="IDrawingSink"/> calls.
/// </summary>
/// <remarks>
/// <para>
/// The two command sets line up almost one for one, which is the whole reason this library
/// was chosen over a rasterising one: both are display lists of the same shape, descended
/// from the same Skia canvas model that <c>IDrawingSink</c> was already modelled on. The
/// translation is therefore mostly a rename, and the interesting work is in the three places
/// they do <em>not</em> line up — clip composition, paints, and text.
/// </para>
/// <para>
/// <b>Text is re-shaped, not copied.</b> The shim's text commands carry a string, a family
/// name and a size; they never carry glyph ids. So every run is shaped here through
/// <see cref="SvgTextEngine"/>, which is what keeps an SVG's lettering consistent with the
/// document's own text rather than with a second font stack.
/// </para>
/// <para>
/// Every recorded command is charged against <see cref="VectorLimits"/>. Truncating in the
/// middle of a picture leaves a prefix, which is why <see cref="IsTruncated"/> is reported
/// back rather than swallowed.
/// </para>
/// </remarks>
internal sealed class SvgPictureTranslator
{
    private readonly DisplayList _list;
    private readonly SvgTextEngine _text;
    private readonly VectorLimits _limits;
    private readonly List<Diagnostic> _diagnostics;
    private readonly HashSet<string> _reported = new(StringComparer.Ordinal);
    private readonly Stack<bool> _layers = new();
    private readonly Stopwatch _clock = Stopwatch.StartNew();
    private int _commands;
    private int _segments;

    /// <summary>Creates a translator writing into a display list.</summary>
    /// <param name="list">Receives the translated commands.</param>
    /// <param name="text">Resolves and shapes text.</param>
    /// <param name="limits">Caps the work.</param>
    /// <param name="diagnostics">Collects what could not be translated faithfully.</param>
    public SvgPictureTranslator(
        DisplayList list,
        SvgTextEngine text,
        VectorLimits limits,
        List<Diagnostic> diagnostics)
    {
        _list = list;
        _text = text;
        _limits = limits;
        _diagnostics = diagnostics;
    }

    /// <summary>True when a limit stopped the translation before the picture ended.</summary>
    public bool IsTruncated { get; private set; }

    /// <summary>Translates a picture's commands into the display list.</summary>
    public void Translate(SKPicture? picture) => Translate(picture, 0);

    private void Translate(SKPicture? picture, int depth)
    {
        if (picture?.Commands is not { } commands) return;

        if (depth > _limits.MaxNestingDepth)
        {
            Report("PL6010", "An SVG nested pictures more deeply than Paperless will follow.");
            IsTruncated = true;
            return;
        }

        foreach (CanvasCommand command in commands)
        {
            if (!Charge()) return;

            switch (command)
            {
                case SaveCanvasCommand:
                    _layers.Push(false);
                    _list.Save();
                    break;

                case SaveLayerCanvasCommand layer:
                    _layers.Push(true);
                    _list.Save();

                    // A layer is how the library spells group opacity, and it has to stay a
                    // group: a group drawn at 40% is not the same picture as each of its
                    // members drawn at 40% wherever they overlap.
                    _list.BeginTransparencyGroup(Alpha(layer.Paint));
                    break;

                case RestoreCanvasCommand:
                    if (_layers.Count > 0 && _layers.Pop()) _list.EndTransparencyGroup();
                    _list.Restore();
                    break;

                case SetMatrixCanvasCommand matrix:
                    _list.Transform(ShimGeometry.Transform(matrix.DeltaMatrix));
                    break;

                case ClipRectCanvasCommand rect:
                    Clip(rect.Rect, rect.Operation);
                    break;

                case ClipPathCanvasCommand clip:
                    Clip(clip.ClipPath, clip.Operation);
                    break;

                case DrawPathCanvasCommand path:
                    Draw(path.Path, path.Paint);
                    break;

                case DrawImageCanvasCommand image:
                    Draw(image);
                    break;

                case DrawPictureCanvasCommand nested:
                    Translate(nested.Picture, depth + 1);
                    break;

                case DrawTextCanvasCommand text:
                    DrawText(text.Text, text.Paint, text.X, text.Y, text.TextAlign);
                    break;

                case DrawTextBlobCanvasCommand blob:
                    DrawBlob(blob);
                    break;

                case DrawPositionedTextRunCanvasCommand run:
                    DrawPositioned(run);
                    break;

                case DrawTextOnPathCanvasCommand onPath:
                    DrawOnPath(onPath);
                    break;

                default:
                    Report("PL6011", $"An SVG produced a drawing command Paperless does not translate ({command.GetType().Name}).");
                    break;
            }
        }
    }

    // ------------------------------------------------------------------------------ geometry

    private void Clip(SKRect rect, SKClipOperation operation)
    {
        if (operation == SKClipOperation.Difference)
        {
            Report("PL6012", "An SVG subtracted a region from its clip; Paperless intersects clips only.");
            return;
        }

        _list.ClipPath(GraphicsPath.Rectangle(ShimGeometry.Rect(rect)));
    }

    private void Clip(ClipPath? clip, SKClipOperation operation)
    {
        if (clip is null || operation == SKClipOperation.Difference)
        {
            if (operation == SKClipOperation.Difference)
            {
                Report("PL6012", "An SVG subtracted a region from its clip; Paperless intersects clips only.");
            }
            return;
        }

        // A ClipPath is a union of PathClips, and each may nest a further ClipPath that
        // intersects it. The sink can only intersect, so the union is expressed as one path
        // with several subpaths under the non-zero rule — which is the union exactly when the
        // subpaths wind the same way, and near enough otherwise, whereas emitting them as
        // separate clips would intersect them and leave only their overlap.
        if (clip.Clips is { Count: > 0 } clips)
        {
            GraphicsPath combined = new();
            FillRule rule = FillRule.NonZero;

            foreach (PathClip element in clips)
            {
                if (element.Path is not { } path) continue;

                SKMatrix? local = Combine(element.Transform, clip.Transform);
                GraphicsPath converted = ShimGeometry.ToPath(path, local, ref _segments);
                rule = ShimGeometry.Rule(path.FillType);

                foreach (CorePathCommand step in converted.Commands) Append(combined, step);
            }

            if (combined.Commands.Count > 0) _list.ClipPath(combined, rule);

            // A clip on one member of a union clips only that member, and the sink has no way
            // to say so — intersecting it globally would clip the other members away as well.
            // With a single member there is nothing to get wrong, so that case is honoured and
            // the rest is reported rather than silently over-clipped.
            if (clips.Count == 1)
            {
                Clip(clips[0].Clip, SKClipOperation.Intersect);
            }
            else if (clips.Any(element => element.Clip is not null))
            {
                Report("PL6023", "An SVG clipped one member of a clip-path union; Paperless clipped none of them.");
            }
        }

        Clip(clip.Clip, SKClipOperation.Intersect);
    }

    private static SKMatrix? Combine(SKMatrix? inner, SKMatrix? outer) => (inner, outer) switch
    {
        (null, null) => null,
        ({ } only, null) => only,
        (null, { } only) => only,
        ({ } a, { } b) => a.PostConcat(b),
    };

    private static void Append(GraphicsPath target, CorePathCommand step)
    {
        switch (step.Verb)
        {
            case PathVerb.MoveTo: target.MoveTo(step.Point); break;
            case PathVerb.LineTo: target.LineTo(step.Point); break;
            case PathVerb.CubicTo: target.CubicTo(step.Control1, step.Control2, step.Point); break;
            case PathVerb.Close:
            default: target.Close(); break;
        }
    }

    private void Draw(SKPath? path, SKPaint? paint)
    {
        if (path is null || paint is null) return;

        GraphicsPath geometry = ShimGeometry.ToPath(path, null, ref _segments);
        if (geometry.Commands.Count == 0 || !ChargeSegments()) return;

        FillRule rule = ShimGeometry.Rule(path.FillType);

        if (paint.Style is SKPaintStyle.Fill or SKPaintStyle.StrokeAndFill
            && Fill(paint) is { } fill)
        {
            _list.FillPath(geometry, fill, rule);
        }

        if (paint.Style is SKPaintStyle.Stroke or SKPaintStyle.StrokeAndFill
            && Pen(paint) is { } pen)
        {
            _list.StrokePath(geometry, pen);
        }
    }

    private void Draw(DrawImageCanvasCommand command)
    {
        if (command.Image?.Data is not { Length: > 0 } bytes) return;

        DocRect destination = ShimGeometry.Rect(command.Dest);
        if (destination.IsEmpty) return;

        // The source rectangle is the whole image in every case the SVG library produces, so
        // a partial source would be a crop this cannot express; it is worth saying so rather
        // than silently drawing the whole picture in the crop's place.
        if (command.Source.Width > 0
            && command.Image.Width > 0
            && (Math.Abs(command.Source.Width - command.Image.Width) > 0.5f
                || Math.Abs(command.Source.Height - command.Image.Height) > 0.5f))
        {
            Report("PL6013", "An SVG drew part of an image; Paperless drew the whole of it.");
        }

        _list.DrawImage(RasterImage.Encoded(bytes), destination, Alpha(command.Paint));
    }

    // ------------------------------------------------------------------------------ text

    private void DrawText(string? text, SKPaint? paint, float x, float y, SKTextAlign? align)
    {
        if (string.IsNullOrEmpty(text) || paint is null) return;

        // The library resolves text-anchor into the origin itself, using the width this
        // engine measured, so an alignment surviving to here is the exception rather than the
        // rule — but when one does, it has to be honoured or the run lands a width away.
        float origin = align switch
        {
            SKTextAlign.Center => x - (_text.Measure(text, paint) / 2),
            SKTextAlign.Right => x - _text.Measure(text, paint),
            _ => x,
        };

        if (_text.Run(text, paint, ShimGeometry.Point(origin, y)) is not { } run) return;

        _list.DrawGlyphRun(run, TextPaint(paint));
    }

    private void DrawBlob(DrawTextBlobCanvasCommand command)
    {
        if (command.TextBlob is not { } blob || blob.Text is not { Length: > 0 } text) return;
        if (blob.Points is not { Length: > 0 } points) return;

        SKPaint paint = Blend(command.Paint, blob.Font);

        // One point per text element, which is how the library spells letter-spacing and
        // per-character x/y lists. Each element is shaped on its own; the alternative — one
        // shaped run whose glyphs are then scattered — would break every ligature and mark
        // attachment the shaper had just made.
        int index = 0;
        for (int i = 0; i < points.Length && index < text.Length; i++)
        {
            int length = char.IsHighSurrogate(text[index]) && index + 1 < text.Length ? 2 : 1;
            string element = text.Substring(index, length);
            index += length;

            if (!Charge()) return;

            DocPoint at = ShimGeometry.Point(points[i].X + command.X, points[i].Y + command.Y);
            if (_text.Run(element, paint, at) is { } run) _list.DrawGlyphRun(run, TextPaint(paint));
        }
    }

    private void DrawPositioned(DrawPositionedTextRunCanvasCommand command)
    {
        if (command.Fragments is not { Count: > 0 } fragments) return;

        SKPaint paint = Blend(command.Paint, command.Font);

        foreach (PositionedTextRunFragment fragment in fragments)
        {
            if (!Charge()) return;
            if (string.IsNullOrEmpty(fragment.Text)) continue;

            DocPoint at = ShimGeometry.Point(fragment.Point);
            bool rotated = Math.Abs(fragment.RotationDegrees) > 1e-4;

            if (rotated)
            {
                // About the glyph's own origin, which is what SVG's rotate list means; a
                // rotation about the picture origin would fling the letter off the page.
                _list.Save();
                _list.Transform(AffineTransform.Concat(
                    AffineTransform.Rotation(fragment.RotationDegrees * Math.PI / 180.0),
                    AffineTransform.Translation(at.X.Emu, at.Y.Emu)));
            }

            DocPoint origin = rotated ? DocPoint.Origin : at;
            if (_text.Run(fragment.Text, paint, origin) is { } run)
            {
                _list.DrawGlyphRun(run, TextPaint(paint));
            }

            if (rotated) _list.Restore();
        }
    }

    private void DrawOnPath(DrawTextOnPathCanvasCommand command)
    {
        if (string.IsNullOrEmpty(command.Text)) return;

        // The library normally expands <textPath> into one positioned draw per glyph and this
        // command never appears; when it does, the text is placed at the path's start rather
        // than dropped, because a missing wordmark is worse than an unbent one.
        Report("PL6014", "An SVG placed text on a path; Paperless drew it straight from the path's start.");

        DocPoint start = command.Path?.Commands?.OfType<MoveToPathCommand>().FirstOrDefault() is { } move
            ? ShimGeometry.Point(move.X + command.HOffset, move.Y + command.VOffset)
            : ShimGeometry.Point(command.HOffset, command.VOffset);

        SKPaint paint = Blend(command.Paint, command.Font);
        if (_text.Run(command.Text, paint, start) is { } run) _list.DrawGlyphRun(run, TextPaint(paint));
    }

    /// <summary>
    /// A paint carrying the size and face a separate <see cref="SKFont"/> states.
    /// </summary>
    /// <remarks>
    /// The shim's newer text commands split what the older ones kept together: the colour
    /// arrives on the paint and the face and size on a font. Merging them here means the rest
    /// of the text path sees one shape of request.
    /// </remarks>
    private static SKPaint Blend(SKPaint? paint, SKFont? font)
    {
        SKPaint result = paint?.Clone() ?? new SKPaint();
        if (font is null) return result;

        if (font.Typeface is { } typeface) result.Typeface = typeface;
        if (font.Size > 0) result.TextSize = font.Size;

        return result;
    }

    /// <summary>
    /// What to draw glyphs with.
    /// </summary>
    /// <remarks>
    /// The sink draws a run with one paint, so stroked text — <c>fill="none"
    /// stroke="red"</c> — is drawn filled in the stroke's colour. That is closer than not
    /// drawing it: outlined lettering at document sizes reads as solid lettering anyway, and
    /// the alternative loses the word.
    /// </remarks>
    private Paint TextPaint(SKPaint paint)
    {
        if (paint.Style == SKPaintStyle.Stroke)
        {
            Report("PL6015", "An SVG stroked its text; Paperless filled it with the stroke's colour instead.");
        }

        return Fill(paint) ?? Paint.Solid(Colour.Black);
    }

    // ------------------------------------------------------------------------------ paint

    private Paint? Fill(SKPaint paint)
    {
        if (paint.ColorFilter is not null || paint.ImageFilter is not null)
        {
            Report("PL6016", "An SVG applied a filter effect; Paperless drew the unfiltered shape.");
        }

        if (paint.BlendMode is not SKBlendMode.SrcOver and not SKBlendMode.Src)
        {
            Report("PL6017", $"An SVG used the '{paint.BlendMode}' blend mode; Paperless composited it normally.");
        }

        switch (paint.Shader)
        {
            case ColorShader colour:
                return Paint.Solid(ShimGeometry.Colour(colour.Color));

            case LinearGradientShader linear:
                return Gradient(
                    GradientKind.Linear,
                    linear.Colors,
                    linear.ColorPos,
                    linear.Start,
                    linear.End,
                    linear.Mode,
                    linear.LocalMatrix);

            case RadialGradientShader radial:
                return Gradient(
                    GradientKind.Radial,
                    radial.Colors,
                    radial.ColorPos,
                    radial.Center,
                    new SKPoint(radial.Center.X + radial.Radius, radial.Center.Y),
                    radial.Mode,
                    radial.LocalMatrix);

            case TwoPointConicalGradientShader conical:
                // SVG's fx/fy radial gradient. The two circles are stated as an outer circle
                // and a focus, which is the form PDF's /ShadingType 3 and Skia's two-point
                // conical shader both take — so nothing is approximated and PL6018 is gone.
                return Gradient(
                    GradientKind.Radial,
                    conical.Colors,
                    conical.ColorPos,
                    conical.End,
                    new SKPoint(conical.End.X + conical.EndRadius, conical.End.Y),
                    conical.Mode,
                    conical.LocalMatrix,
                    Same(conical.Start, conical.End) ? null : conical.Start);

            case PictureShader:
                Report("PL6019", "An SVG filled a shape with a pattern; Paperless left it unfilled.");
                return null;

            case { } other:
                Report("PL6020", $"An SVG used a '{other.GetType().Name}' fill Paperless does not express.");
                return null;

            case null:
                return paint.Color is { } solid ? Paint.Solid(ShimGeometry.Colour(solid)) : null;
        }
    }

    private static bool Same(SKPoint a, SKPoint b) => a.X == b.X && a.Y == b.Y;

    private static GradientPaint Gradient(
        GradientKind kind,
        SKColorF[]? colours,
        float[]? offsets,
        SKPoint start,
        SKPoint end,
        SKShaderTileMode mode,
        SKMatrix? local,
        SKPoint? focus = null)
    {
        List<GradientStop> stops = [];
        for (int i = 0; i < (colours?.Length ?? 0); i++)
        {
            double offset = offsets is { Length: > 0 } && i < offsets.Length
                ? offsets[i]
                : colours!.Length == 1 ? 0 : (double)i / (colours!.Length - 1);

            stops.Add(new GradientStop(offset, ShimGeometry.Colour(colours![i])));
        }

        if (stops.Count == 0) stops.Add(new GradientStop(0, Colour.Transparent));

        // SVG's spreadMethod, which the shim carries as a Skia tile mode and both backends
        // state natively — Skia as the same tile mode, PDF by lengthening the shading's axis
        // over as many periods as the shape spans.
        SpreadMethod spread = mode switch
        {
            SKShaderTileMode.Mirror => SpreadMethod.Reflect,
            SKShaderTileMode.Repeat => SpreadMethod.Repeat,
            _ => SpreadMethod.Pad,
        };

        return new GradientPaint(
            kind,
            stops,
            ShimGeometry.Point(start),
            ShimGeometry.Point(end),
            local is { } matrix ? ShimGeometry.Transform(matrix) : AffineTransform.Identity,
            spread,
            focus is { } point ? ShimGeometry.Point(point) : null);
    }

    private Stroke? Pen(SKPaint paint)
    {
        if (Fill(paint) is not { } colour) return null;

        if (paint.IsStrokeNonScaling)
        {
            Report("PL6022", "An SVG asked for a non-scaling stroke; Paperless scaled it with the shape.");
        }

        IReadOnlyList<Length>? dashes = null;
        Length phase = Length.Zero;

        if (paint.PathEffect is DashPathEffect dash && dash.Intervals is { Length: > 0 } intervals)
        {
            dashes = [.. intervals.Select(interval => ShimGeometry.Emu(interval))];
            phase = ShimGeometry.Emu(dash.Phase);
        }

        return new Stroke(
            colour,
            ShimGeometry.Emu(paint.StrokeWidth),
            paint.StrokeCap switch
            {
                SKStrokeCap.Round => LineCap.Round,
                SKStrokeCap.Square => LineCap.Square,
                _ => LineCap.Butt,
            },
            paint.StrokeJoin switch
            {
                SKStrokeJoin.Round => LineJoin.Round,
                SKStrokeJoin.Bevel => LineJoin.Bevel,
                _ => LineJoin.Miter,
            },
            paint.StrokeMiter > 0 ? paint.StrokeMiter : 4.0,
            dashes,
            phase);
    }

    /// <summary>The opacity a layer or image paint asks for.</summary>
    private static double Alpha(SKPaint? paint)
        => paint?.Color is { } colour ? colour.Alpha / 255.0 : 1.0;

    // ------------------------------------------------------------------------------ budget

    private bool Charge()
    {
        if (IsTruncated) return false;

        if (++_commands > _limits.MaxCommands)
        {
            Report("PL6001", $"An SVG produced more than {_limits.MaxCommands} drawing commands and was truncated.");
            IsTruncated = true;
            return false;
        }

        // Checked per command rather than per picture: the point is to stop a document that
        // is slow rather than large, and a check that only runs at the end never stops
        // anything.
        if (_clock.Elapsed > _limits.TimeLimit)
        {
            Report("PL6001", $"An SVG took longer than {_limits.TimeLimit.TotalSeconds:0.#} s to translate and was truncated.");
            IsTruncated = true;
            return false;
        }

        return true;
    }

    private bool ChargeSegments()
    {
        if (_segments <= _limits.MaxPathSegments) return true;

        Report("PL6001", $"An SVG produced more than {_limits.MaxPathSegments} path segments and was truncated.");
        IsTruncated = true;
        return false;
    }

    /// <summary>
    /// Records a diagnostic once per kind.
    /// </summary>
    /// <remarks>
    /// A picture with two thousand filtered shapes would otherwise produce two thousand
    /// identical warnings, which buries the one that was worth reading.
    /// </remarks>
    private void Report(string code, string message)
    {
        if (!_reported.Add(string.Create(CultureInfo.InvariantCulture, $"{code}:{message}"))) return;

        _diagnostics.Add(new Diagnostic(DiagnosticSeverity.Warning, code, message));
    }
}

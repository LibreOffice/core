using System.Collections.Concurrent;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;
using ShimSkiaSharp;

namespace Paperless.Vector.Svg;

/// <summary>
/// Resolves and shapes SVG text through Paperless's own font stack.
/// </summary>
/// <remarks>
/// <para>
/// This is the reason to prefer a scene-building SVG library over a rasterising one. The
/// library hands out font <em>requests</em> — a family name, a weight, a slant and a size —
/// and asks its host to resolve, measure and shape them. Answering those with
/// <c>Paperless.Text</c> means SVG text is resolved by the same substitution table and shaped
/// by the same HarfBuzz as body text, so a logo's wordmark and the paragraph beside it agree
/// about what "Calibri" is. A second, divergent text path is exactly what this avoids.
/// </para>
/// <para>
/// <b>Measurement is load-bearing, not cosmetic.</b> <c>text-anchor</c> is resolved by the
/// library <em>before</em> it emits a draw command: it measures the run and shifts the origin
/// itself. Measured against <c>Svg.SceneGraph</c> 5.1.1, a <c>text-anchor="middle"</c> run at
/// <c>x="150"</c> came out at <c>x=120</c> for a 60-unit-wide measurement. So a wrong
/// measurement does not produce slightly-wrong glyph spacing; it puts centred and
/// right-anchored text in the wrong place entirely.
/// </para>
/// </remarks>
internal sealed class SvgTextEngine
{
    /// <summary>
    /// The family used when the SVG names none.
    /// </summary>
    /// <remarks>
    /// SVG leaves the initial <c>font-family</c> to the user agent. LibreOffice resolves it
    /// through its default-font machinery, which lands on Liberation Sans on Linux — the same
    /// answer <c>Paperless.Spreadsheets.Layout.SheetFonts.DefaultFamily</c> settled on for the
    /// same reason.
    /// </remarks>
    public const string DefaultFamily = "Liberation Sans";

    private static readonly Lazy<SystemFontResolver> SharedResolver =
        new(SystemFontResolver.Build, LazyThreadSafetyMode.ExecutionAndPublication);

    private readonly SystemFontResolver _resolver;
    private readonly ITextShaper _shaper;
    private readonly ConcurrentDictionary<(string Family, int Weight, bool Italic), ResolvedFace?> _faces = new();

    /// <summary>Creates an engine over a font resolver and a shaper.</summary>
    /// <param name="resolver">Resolves family names; null uses the shared system resolver.</param>
    /// <param name="shaper">Shapes runs; null uses <see cref="TextShaper.Default"/>.</param>
    public SvgTextEngine(SystemFontResolver? resolver = null, ITextShaper? shaper = null)
    {
        _resolver = resolver ?? SharedResolver.Value;
        _shaper = shaper ?? TextShaper.Default;
    }

    /// <summary>A face resolved from an SVG font request.</summary>
    /// <param name="Face">The OpenType tables, for measuring and shaping.</param>
    /// <param name="Reference">What the display list names the face by.</param>
    public sealed record ResolvedFace(OpenTypeFace Face, FontReference Reference);

    /// <summary>
    /// The face a shim paint asks for.
    /// </summary>
    /// <remarks>
    /// A paint whose <see cref="SKPaint.Typeface"/> is null is not an error: the library
    /// leaves it null when the SVG names no family, and there is then no weight or slant to
    /// recover either, because the shim carries both on the typeface rather than on the paint.
    /// The default face is used in that case, which is what a user agent does.
    /// </remarks>
    public ResolvedFace? Face(SKPaint? paint)
    {
        SKTypeface? typeface = paint?.Typeface;

        string family = typeface?.FamilyName is { Length: > 0 } named ? named : DefaultFamily;
        int weight = typeface is null ? 400 : (int)typeface.FontWeight;
        bool italic = typeface?.FontSlant is SKFontStyleSlant.Italic or SKFontStyleSlant.Oblique;

        return _faces.GetOrAdd((family, weight, italic), Load);
    }

    /// <summary>The em size a shim paint asks for, in EMUs.</summary>
    /// <remarks>
    /// The shim states text size in SVG user units, like every other length it carries, so it
    /// goes through the same one conversion as the geometry.
    /// </remarks>
    public static Length Size(SKPaint? paint) => ShimGeometry.Emu(paint?.TextSize ?? 0);

    /// <summary>The advance width of a run, in SVG user units.</summary>
    /// <remarks>
    /// Answered in the shim's units rather than in EMUs because the caller is the library,
    /// which is laying out in its own space.
    /// </remarks>
    public float Measure(string? text, SKPaint? paint)
    {
        if (string.IsNullOrEmpty(text) || Face(paint) is not { } face) return 0;

        ShapedText shaped = _shaper.Shape(face.Face, text);
        float size = paint?.TextSize ?? 0;

        return (float)((double)shaped.AdvanceInDesignUnits / shaped.UnitsPerEm * size);
    }

    /// <summary>
    /// Shapes a run into a display-list glyph run whose baseline starts at an origin.
    /// </summary>
    /// <param name="text">The characters.</param>
    /// <param name="paint">The shim paint naming the face and size.</param>
    /// <param name="origin">Where the baseline starts, in document coordinates.</param>
    public GlyphRun? Run(string? text, SKPaint? paint, DocPoint origin)
    {
        if (string.IsNullOrEmpty(text) || Face(paint) is not { } face) return null;

        Length emSize = Size(paint);
        if (emSize <= Length.Zero) return null;

        ShapedText shaped = _shaper.Shape(face.Face, text);
        if (shaped.Glyphs.Count == 0) return null;

        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);
        Length pen = Length.Zero;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, emSize);

            // The shaper's vertical offset is up-positive and document space is down-positive,
            // so a mark placed above its base has to change sign on the way through.
            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(pen + shaped.Scale(glyph.OffsetX, emSize), -shaped.Scale(glyph.OffsetY, emSize)),
                advance));

            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new GlyphRun
        {
            Font = face.Reference,
            FontSize = emSize,
            Origin = origin,
            Glyphs = glyphs,
            Text = text,
            ClusterMap = clusters,
        };
    }

    /// <summary>The face's vertical metrics at a paint's size, in SVG user units.</summary>
    /// <remarks>
    /// Skia's sign convention, which is what the library expects back: ascent is negative
    /// because it is measured upwards from the baseline in a y-down space.
    /// </remarks>
    public SKFontMetrics Metrics(SKPaint? paint)
    {
        if (Face(paint) is not { } face) return default;

        double size = paint?.TextSize ?? 0;
        double scale = size / face.Face.UnitsPerEm;
        FontVerticalMetrics metrics = LineSpacing.ResolveDecorations(face.Face, LineSpacing.Resolve(face.Face));

        return new SKFontMetrics
        {
            Ascent = (float)(-metrics.Ascent * scale),
            Descent = (float)(metrics.Descent * scale),
            Top = (float)(-metrics.Ascent * scale),
            Bottom = (float)(metrics.Descent * scale),
            Leading = (float)(metrics.LineGap * scale),
            UnderlinePosition = (float)(-metrics.UnderlinePosition * scale),
            UnderlineThickness = (float)(metrics.UnderlineThickness * scale),
            StrikeoutPosition = (float)(-metrics.StrikeoutPosition * scale),
            StrikeoutThickness = (float)(metrics.StrikeoutThickness * scale),
        };
    }

    private ResolvedFace? Load((string Family, int Weight, bool Italic) key)
    {
        try
        {
            FontReference reference = _resolver.Resolve(new FontRequest(key.Family, key.Weight, key.Italic));
            return new ResolvedFace(_resolver.LoadOpenType(reference), reference);
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // A face that will not load is a picture drawn without its lettering, not a
            // document that fails to render.
            return null;
        }
    }
}

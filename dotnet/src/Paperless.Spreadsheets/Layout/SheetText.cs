using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Shaping;

namespace Paperless.Spreadsheets.Layout;

/// <summary>A shaped piece of cell text, positioned once it is placed.</summary>
/// <remarks>
/// Shaped without an origin and given one later, because a cell's horizontal position depends on
/// the run's own width — a number is drawn against the right edge of its column, so where it
/// starts cannot be known until it has been measured.
/// </remarks>
internal sealed class SheetTextRun
{
    private readonly List<PositionedGlyph> _glyphs;
    private readonly List<int> _clusters;
    private readonly FontReference _font;
    private readonly Length _size;
    private readonly string _text;

    internal SheetTextRun(
        List<PositionedGlyph> glyphs,
        List<int> clusters,
        FontReference font,
        Length size,
        string text,
        Length width)
    {
        _glyphs = glyphs;
        _clusters = clusters;
        _font = font;
        _size = size;
        _text = text;
        Width = width;
    }

    /// <summary>How far the run's pen travels.</summary>
    public Length Width { get; }

    /// <summary>The em size the run is set at.</summary>
    public Length Size => _size;

    /// <summary>The run placed at a baseline origin.</summary>
    public GlyphRun At(DocPoint origin) => new()
    {
        Font = _font,
        FontSize = _size,
        Origin = origin,
        Glyphs = _glyphs,
        Text = _text,
        ClusterMap = _clusters,
    };
}

/// <summary>
/// Shapes cell text in a resolved face.
/// </summary>
/// <remarks>
/// Shaping only — where the run goes is <see cref="SheetTextLayout"/>'s business. The two are
/// kept apart because measuring is what pagination needs (<see cref="SheetTextOverflow"/> widens
/// a sheet's print area by measuring strings) and placing is what drawing needs, and the first
/// must not drag in the second.
/// </remarks>
internal static class SheetText
{
    /// <summary>Shapes a string, or null when there is no face to shape it with.</summary>
    /// <param name="text">The text to shape.</param>
    /// <param name="face">The face to shape it in.</param>
    /// <param name="size">The em size to scale the advances to.</param>
    public static SheetTextRun? Shape(string text, SheetFace? face, Length size)
    {
        if (text.Length == 0 || face is not { } resolved) return null;

        ShapedText shaped = TextShaper.Default.Shape(resolved.Face, text);

        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);
        Length pen = Length.Zero;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, size);
            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(
                    pen + shaped.Scale(glyph.OffsetX, size),
                    -shaped.Scale(glyph.OffsetY, size)),
                advance));
            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new SheetTextRun(glyphs, clusters, resolved.Reference, size, text, pen);
    }

    /// <summary>How wide a string is in a face, without keeping the run.</summary>
    /// <param name="text">The text to measure.</param>
    /// <param name="face">The face; null measures as nothing.</param>
    /// <param name="size">The em size.</param>
    public static Length Measure(string text, SheetFace? face, Length size)
        => Shape(text, face, size)?.Width ?? Length.Zero;

    /// <summary>The face a sheet's cells fall back to, resolved once.</summary>
    public static SheetFace? DefaultFace => SheetFonts.For(SheetCellFormat.Default);
}

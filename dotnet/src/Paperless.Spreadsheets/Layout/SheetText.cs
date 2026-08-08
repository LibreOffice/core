using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Shaping;

namespace Paperless.Spreadsheets.Layout;

/// <summary>One stretch of a shaped cell run set in a single face, size and colour.</summary>
/// <remarks>
/// A plain cell has exactly one of these and a rich one has several. The offset is measured from
/// the run's own start, so placing the run is one addition per segment rather than a re-shape.
/// </remarks>
/// <param name="Glyphs">Its glyphs, positioned from the segment's own origin.</param>
/// <param name="Clusters">The cluster map, as indices into <paramref name="Text"/>.</param>
/// <param name="Face">The face it is set in, for the metrics its line height needs.</param>
/// <param name="Size">The em size.</param>
/// <param name="Colour">Its own colour, or null to take the cell's.</param>
/// <param name="Text">The characters it covers.</param>
/// <param name="Offset">How far along the run it starts.</param>
/// <param name="Width">How far its pen travels.</param>
internal sealed record SheetTextSegment(
    List<PositionedGlyph> Glyphs,
    List<int> Clusters,
    SheetFace Face,
    Length Size,
    Colour? Colour,
    string Text,
    Length Offset,
    Length Width);

/// <summary>A shaped piece of cell text, positioned once it is placed.</summary>
/// <remarks>
/// <para>
/// Shaped without an origin and given one later, because a cell's horizontal position depends on
/// the run's own width — a number is drawn against the right edge of its column, so where it
/// starts cannot be known until it has been measured.
/// </para>
/// <para>
/// A list of segments rather than one, because a cell's text may change format part-way through.
/// Concatenating separately shaped segments is exactly right here and would not be in a word
/// processor: cell text is not kerned (<see cref="SheetText.NoKerning"/>), so there is no pair
/// adjustment to lose across a format boundary, and Calc splits a cell's text into EditEngine
/// portions on the same boundaries anyway. Measured on Liberation Sans: neither its <c>GPOS</c>
/// nor its legacy <c>kern</c> table has a pair for any boundary in the corpus document, so
/// shaping the portions together would give the same widths.
/// </para>
/// </remarks>
internal sealed class SheetTextRun
{
    private readonly List<SheetTextSegment> _segments;

    internal SheetTextRun(List<SheetTextSegment> segments, Length width)
    {
        _segments = segments;
        Width = width;
    }

    /// <summary>How far the run's pen travels.</summary>
    public Length Width { get; }

    /// <summary>The pieces the run is made of, in reading order.</summary>
    public IReadOnlyList<SheetTextSegment> Segments => _segments;

    /// <summary>The em size the run is set at, which is its first segment's.</summary>
    public Length Size => _segments.Count > 0 ? _segments[0].Size : Length.Zero;

    /// <summary>
    /// The distance from the baseline to the top of the tallest segment.
    /// </summary>
    /// <remarks>
    /// The maximum rather than the cell's own, because a line is as tall as what is on it: a
    /// sixteen-point word inside a ten-point cell pushes the whole line's baseline down. That is
    /// EditEngine's rule as much as Writer's, and it is why the metrics travel with the segment.
    /// </remarks>
    public Length Ascent => Extreme(segment => segment.Face.AscentAt(segment.Size));

    /// <summary>The distance from the baseline to the bottom of the deepest segment.</summary>
    public Length Descent => Extreme(segment => segment.Face.DescentAt(segment.Size));

    /// <summary>How tall Calc considers this run's line: ascent plus descent, with no line gap.</summary>
    public Length LineHeight => Ascent + Descent;

    /// <summary>The run placed at a baseline origin, one glyph run per segment.</summary>
    /// <param name="origin">Where the run's pen starts.</param>
    public IEnumerable<(GlyphRun Run, Colour? Colour)> At(DocPoint origin)
    {
        foreach (SheetTextSegment segment in _segments)
        {
            yield return (
                new GlyphRun
                {
                    Font = segment.Face.Reference,
                    FontSize = segment.Size,
                    Origin = new DocPoint(origin.X + segment.Offset, origin.Y),
                    Glyphs = segment.Glyphs,
                    Text = segment.Text,
                    ClusterMap = segment.Clusters,
                },
                segment.Colour);
        }
    }

    private Length Extreme(Func<SheetTextSegment, Length> of)
    {
        Length largest = Length.Zero;
        foreach (SheetTextSegment segment in _segments)
        {
            Length value = of(segment);
            if (value > largest) largest = value;
        }
        return largest;
    }
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
    /// <summary>
    /// How a cell's text is shaped: kerned pairs off.
    /// </summary>
    /// <remarks>
    /// Calc says so in as many words — "There is no cell attribute for kerning, default is kerning
    /// OFF, all kerning is stored at an EditText object that is drawn using EditEngine"
    /// (<c>ScDrawStringsVars::SetPattern</c>, <c>sc/source/ui/view/output2.cxx:405-409</c>) — and
    /// it is measurable rather than theoretical: HarfBuzz kerns <c>1.2E+11</c> by 152 design
    /// units, which puts a right-aligned cell 0.74 pt away from where LibreOffice puts it. That
    /// is a difference no font metric or margin would explain, so it is worth having the reason
    /// written down beside the flag.
    /// </remarks>
    public static readonly ShapingOptions NoKerning = new(DisableKerning: true);

    /// <summary>Shapes a string, or null when there is no face to shape it with.</summary>
    /// <param name="text">The text to shape.</param>
    /// <param name="face">The face to shape it in.</param>
    /// <param name="size">The em size to scale the advances to.</param>
    /// <param name="colour">Its own colour, or null to take the cell's.</param>
    public static SheetTextRun? Shape(string text, SheetFace? face, Length size, Colour? colour = null)
    {
        if (text.Length == 0 || face is not { } resolved) return null;

        return new SheetTextRun(
            [Segment(text, resolved, size, colour, Length.Zero, out Length width)], width);
    }

    /// <summary>
    /// Shapes a range of a rich cell's text, one segment per portion it crosses.
    /// </summary>
    /// <remarks>
    /// The range is a range of the <em>cell's</em> text rather than a substring handed in, because
    /// the portions index into that text: shortening a clipped rich cell or taking one wrapped line
    /// of it has to keep the formatting lined up with the characters, and a substring loses the
    /// offset that says how.
    /// </remarks>
    /// <param name="text">The cell's whole text.</param>
    /// <param name="portions">Its portions, normalised to cover it exactly once.</param>
    /// <param name="scale">The print zoom, which multiplies every em size.</param>
    /// <param name="start">The first character to shape.</param>
    /// <param name="end">One past the last.</param>
    /// <param name="percent">A further shrink, as an integer percentage; 100 for none.</param>
    public static SheetTextRun? ShapeRich(
        string text,
        IReadOnlyList<SheetTextPortion> portions,
        double scale,
        int start,
        int end,
        long percent = 100)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(portions);

        int first = Math.Max(start, 0);
        int last = Math.Min(end, text.Length);
        if (last <= first) return null;

        List<SheetTextSegment> segments = [];
        Length offset = Length.Zero;

        foreach (SheetTextPortion portion in portions)
        {
            int from = Math.Max(portion.Start, first);
            int to = Math.Min(portion.End, last);
            if (to <= from) continue;

            if (SheetFonts.For(portion.Format) is not { } face) continue;

            Length size = SizeOf(portion.Format.FontSize, scale, percent);
            if (size <= Length.Zero) continue;

            segments.Add(Segment(
                text[from..to], face, size, portion.Format.Colour, offset, out Length width));
            offset += width;
        }

        return segments.Count == 0 ? null : new SheetTextRun(segments, offset);
    }

    /// <summary>
    /// A portion's em size, snapped to the drawing device's unit before anything scales it.
    /// </summary>
    /// <remarks>
    /// The same two roundings a plain cell's size goes through, in the same order: the shrink
    /// percentage is applied in whole twips because Calc's own shrink loop works there, and the
    /// result is snapped to a hundredth of a millimetre because that is the unit the device draws
    /// in. See <see cref="SheetDeviceUnits"/>.
    /// </remarks>
    public static Length SizeOf(Length stated, double scale, long percent)
    {
        Length scaled = SheetDeviceUnits.SnapFontSize(stated, scale);

        return percent == 100
            ? scaled
            : SheetDeviceUnits.SnapFontSize(Length.FromTwips(scaled.Twips * percent / 100));
    }

    private static SheetTextSegment Segment(
        string text, SheetFace face, Length size, Colour? colour, Length offset, out Length width)
    {
        ShapedText shaped = TextShaper.Default.Shape(face.Face, text, NoKerning);

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

        width = pen;
        return new SheetTextSegment(glyphs, clusters, face, size, colour, text, offset, pen);
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

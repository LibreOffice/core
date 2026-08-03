using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// One shaped line of furniture text — a header, a footer, or a row or column heading — with the
/// metrics needed to place it vertically.
/// </summary>
/// <remarks>
/// <para>
/// A separate helper from <c>SheetText</c>, which shapes cell text, because the two are placed by
/// different rules and need different things. A cell's text sits on a baseline derived from the
/// row; a header's is <em>centred</em> in its band, which needs the line height and the ascent
/// rather than just the advance width — <c>ScPrintFunc::PrintHF</c> moves the draw point down by
/// half the difference between the band and the text
/// (<c>sc/source/ui/view/printfun.cxx:1879</c>).
/// </para>
/// <para>
/// If the cell-text work wants these metrics too, this is the file to fold into: the face and the
/// resolver are the same, and only the placement rules differ.
/// </para>
/// </remarks>
internal static class SheetBandText
{
    /// <summary>
    /// The face Calc's furniture is drawn in.
    /// </summary>
    /// <remarks>
    /// The default cell font, not a separate one: <c>ScPrintFunc::MakeEditEngine</c> fills the
    /// header's defaults from <c>getDefaultCellAttribute</c> and only overrides the height unit
    /// (<c>printfun.cxx:1769-1774</c>), and <c>PrintPage</c> builds the heading font from a bare
    /// <c>ScPatternAttr</c> the same way (<c>printfun.cxx:2354</c>). So a header and a column
    /// heading are drawn in whatever a plain cell would be.
    /// </remarks>
    private const string DefaultFamily = "Liberation Sans";

    /// <summary>Ten point, which is Calc's default cell font height.</summary>
    public static Length DefaultSize { get; } = Length.FromPoints(10);

    /// <summary>
    /// The face the furniture is drawn in, together with the reference it was resolved through.
    /// </summary>
    /// <remarks>
    /// Both, and resolved in one place, because the reference cannot be rebuilt from the face
    /// afterwards: an <see cref="OpenTypeFace"/> is a parsed table directory and does not know
    /// which file it was read out of. The resolver's own <c>FaceKey</c> is that file's path, and
    /// it is what lets a PDF embed the face — see the remark on <see cref="Description"/>.
    /// </remarks>
    private static readonly Lazy<(OpenTypeFace? Face, FontReference? Reference)> Resolved =
        new(Load);

    private static readonly Lazy<LineMetrics?> Metrics = new(
        () => Resolved.Value.Face is { } face ? LineSpacing.Resolve(face) : null);

    /// <summary>The distance from a line's top to its baseline, at a size.</summary>
    /// <param name="size">The em size.</param>
    public static Length AscentAt(Length size)
        => Metrics.Value is { } metrics ? metrics.ScaledAscent(size) : size * 0.9;

    /// <summary>How tall one line is, at a size.</summary>
    /// <remarks>
    /// Ascent plus descent without the line gap, which is what a single line occupies and what
    /// Calc's <c>GetTextHeight</c> answers for a one-line header. Measured on Liberation Sans at
    /// ten point that is 11.1 pt, and it is what puts LibreOffice's header baseline 10.55 pt
    /// below the top of a band 14.099 pt tall.
    /// </remarks>
    /// <param name="size">The em size.</param>
    public static Length LineHeightAt(Length size)
        => Metrics.Value is { } metrics
            ? metrics.ScaledAscent(size) + metrics.ScaledDescent(size)
            : size * 1.15;

    /// <summary>
    /// How tall one line of a <em>chart's</em> text is, at a size.
    /// </summary>
    /// <remarks>
    /// Ascent plus descent plus the line gap, which is the face's own line height and not
    /// <see cref="LineHeightAt"/>'s. The two differ because a chart is not laid out by Calc: its
    /// labels are made by <c>chart2</c>'s view as plain text shapes, which take the face's metrics
    /// whole, where a cell's height comes from <c>ScDrawStringsVars</c> and drops the gap
    /// (<c>sc/source/ui/view/output2.cxx:734</c>). Liberation Sans is 1.1499 em here against
    /// 1.1494 there, and the difference compounds through the insets that place the plot area
    /// rather than showing up in any one label.
    /// </remarks>
    /// <param name="size">The em size.</param>
    public static Length ChartLineHeightAt(Length size)
        => Metrics.Value is { } metrics ? metrics.ScaledLineHeight(size) : size * 1.15;

    /// <summary>
    /// The metrics of a named face, or the furniture's own where it names none.
    /// </summary>
    /// <remarks>
    /// The furniture itself never names one — a header and a column heading are drawn in whatever
    /// a plain cell would be — but a shape's text does, and it is laid out by the same three calls.
    /// Returning null for a family that cannot be resolved would silently lose the text; falling
    /// back to the default face loses only the face, which is what a substitution is.
    /// </remarks>
    /// <param name="family">The family name, or null for the furniture's own face.</param>
    private static (OpenTypeFace? Face, FontReference Reference, LineMetrics? Metrics) FaceFor(
        string? family)
    {
        if (string.IsNullOrWhiteSpace(family)) return (Resolved.Value.Face, Description, Metrics.Value);

        return SheetFonts.ForFamily(family) is { } named
            ? (named.Face, named.Reference, named.Metrics)
            : (Resolved.Value.Face, Description, Metrics.Value);
    }

    /// <summary>The distance from a line's top to its baseline, at a size, in a named face.</summary>
    /// <param name="size">The em size.</param>
    /// <param name="family">The family name, or null for the furniture's own face.</param>
    public static Length AscentAt(Length size, string? family)
        => FaceFor(family).Metrics is { } metrics ? metrics.ScaledAscent(size) : size * 0.9;

    /// <inheritdoc cref="ChartLineHeightAt(Length)"/>
    /// <param name="size">The em size.</param>
    /// <param name="family">The family name, or null for the furniture's own face.</param>
    public static Length ChartLineHeightAt(Length size, string? family)
        => FaceFor(family).Metrics is { } metrics ? metrics.ScaledLineHeight(size) : size * 1.15;

    /// <summary>Shapes one line, or null when there is no face to shape it with.</summary>
    /// <param name="text">The text.</param>
    /// <param name="size">The em size.</param>
    public static BandRun? Shape(string text, Length size) => Shape(text, size, null);

    /// <inheritdoc cref="Shape(string, Length)"/>
    /// <param name="text">The text.</param>
    /// <param name="size">The em size.</param>
    /// <param name="family">The family name, or null for the furniture's own face.</param>
    public static BandRun? Shape(string text, Length size, string? family)
    {
        if (text.Length == 0) return null;

        (OpenTypeFace? resolved, FontReference reference, _) = FaceFor(family);
        if (resolved is not { } face) return null;

        ShapedText shaped = TextShaper.Default.Shape(face, text);

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

        return new BandRun(glyphs, clusters, reference, size, text, pen);
    }

    /// <summary>
    /// How a backend names the furniture's face: the resolver's own key, which is a file path.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The reference is kept from the resolution rather than rebuilt from the loaded face. Naming
    /// the family instead — <c>FaceKey = face.FamilyName</c>, which is what this did — gives
    /// <c>FileFontProvider</c> a key it cannot open, so the PDF writer referenced the face and
    /// embedded no <c>FontFile2</c> for it. A reader then substitutes or draws tofu, and neither
    /// the page count nor the extracted words change, which is why the sweep never saw it.
    /// </para>
    /// <para>
    /// Measured with <c>pdffonts</c> on <c>sheet-features.ods</c>: the two cell faces reported
    /// <c>emb yes</c> and the header's third face <c>emb no</c>, in a file whose text extracted
    /// correctly throughout.
    /// </para>
    /// </remarks>
    private static FontReference Description =>
        Resolved.Value.Reference
        ?? new FontReference { FamilyName = DefaultFamily, FaceKey = string.Empty };

    private static (OpenTypeFace? Face, FontReference? Reference) Load()
    {
        try
        {
            SystemFontResolver resolver = SystemFontResolver.Build();
            FontReference reference = resolver.Resolve(new FontRequest(DefaultFamily));
            return (resolver.LoadOpenType(reference), reference);
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // No readable face is not a reason to fail a layout: the page, its geometry and
            // everything drawn as a path are already decided, and only the ink is missing.
            return (null, null);
        }
    }
}

/// <summary>A shaped line of furniture text, positioned once it is placed.</summary>
/// <remarks>
/// Shaped without an origin and given one later, because where it starts depends on its own
/// width: a header's right part ends at the band's right edge, so its start is only known once it
/// has been measured.
/// </remarks>
internal sealed class BandRun
{
    private readonly List<PositionedGlyph> _glyphs;
    private readonly List<int> _clusters;
    private readonly FontReference _font;
    private readonly Length _size;
    private readonly string _text;

    internal BandRun(
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

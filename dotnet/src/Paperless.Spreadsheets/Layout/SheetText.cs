using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
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
/// Shapes cell text in the sheet's default face.
/// </summary>
/// <remarks>
/// <para>
/// One face for the whole workbook, deliberately for now. Cell fonts are a formatting attribute
/// the content tree does not carry and the readers do not yet resolve — <c>FONT</c> records and
/// <c>styles.xml</c>'s <c>fonts</c> element are both on the module's TODO — so drawing every
/// cell in the default face is the honest shape of what has been read. It costs nothing in
/// pagination: a row's height and a column's width come from the file, never from the text.
/// </para>
/// <para>
/// Liberation Sans is the face asked for because it is what LibreOffice puts in a new
/// spreadsheet on Linux (<c>DefaultFontType::LATIN_SPREADSHEET</c>) and therefore what every
/// reference rendering of a document that names no font is measured in. The resolver
/// substitutes if it is absent, and a face that cannot be read at all yields no run rather than
/// an exception, so a machine with no fonts draws an empty page instead of failing.
/// </para>
/// </remarks>
internal static class SheetText
{
    private const string DefaultFamily = "Liberation Sans";

    private static readonly Lazy<OpenTypeFace?> Face = new(Load);
    private static readonly Lazy<FontReference> Reference = new(Describe);

    /// <summary>Shapes a string, or null when there is no face to shape it with.</summary>
    /// <param name="text">The text to shape.</param>
    /// <param name="size">The em size to scale the advances to.</param>
    public static SheetTextRun? Shape(string text, Length size)
    {
        if (text.Length == 0 || Face.Value is not { } face) return null;

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

        return new SheetTextRun(glyphs, clusters, Reference.Value, size, text, pen);
    }

    private static OpenTypeFace? Load()
    {
        try
        {
            SystemFontResolver resolver = SystemFontResolver.Build();
            return resolver.LoadOpenType(resolver.Resolve(new FontRequest(DefaultFamily)));
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // No readable face is not a reason to fail a layout — the pages, their count and
            // their geometry are all already decided, and only the ink is missing.
            return null;
        }
    }

    private static FontReference Describe() => Face.Value is { } face
        ? new FontReference
        {
            FamilyName = face.FamilyName ?? DefaultFamily,
            RequestedFamily = DefaultFamily,
            Weight = face.Weight,
            IsItalic = face.IsItalic,
            FaceKey = face.FamilyName ?? DefaultFamily,
        }
        : new FontReference { FamilyName = DefaultFamily, FaceKey = DefaultFamily };
}

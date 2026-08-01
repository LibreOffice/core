using System.Collections.Concurrent;
using System.Text;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Encodings;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// Resolves, decodes, measures and shapes the text a metafile draws.
/// </summary>
/// <remarks>
/// <para>
/// A metafile's text records are the one place where the format hands over a problem the rest
/// of Paperless has already solved: a family name, a weight, a slant and eight-bit bytes in
/// some code page. Answering them through <c>Paperless.Text</c> — the same substitution table
/// and the same HarfBuzz as body text — is what makes a label inside an embedded picture agree
/// with the paragraph beside it about what "Arial" is, and it is the same argument
/// <c>Svg/SvgTextEngine.cs</c> makes for SVG.
/// </para>
/// <para>
/// <b>Alignment is resolved here, not by the sink.</b> <c>GlyphRun.Origin</c> is the start of
/// the baseline and nothing else; GDI's point may be any of nine positions depending on the
/// text-alignment word, and turning one into the other needs the run's measured width and the
/// face's ascent. A decoder that ignores the word draws every centred label half a string too
/// far right — visible immediately, and easy to mistake for a mapping bug.
/// </para>
/// </remarks>
public sealed class MetafileTextEngine
{
    private static readonly Lazy<SystemFontResolver> SharedResolver =
        new(SystemFontResolver.Build, LazyThreadSafetyMode.ExecutionAndPublication);

    private readonly SystemFontResolver _resolver;
    private readonly ITextShaper _shaper;
    private readonly ConcurrentDictionary<(string Family, int Weight, bool Italic), ResolvedFace?> _faces = new();

    /// <summary>Creates an engine over a font resolver and a shaper.</summary>
    /// <param name="resolver">Resolves family names; null uses the shared system resolver.</param>
    /// <param name="shaper">Shapes runs; null uses <see cref="TextShaper.Default"/>.</param>
    public MetafileTextEngine(SystemFontResolver? resolver = null, ITextShaper? shaper = null)
    {
        _resolver = resolver ?? SharedResolver.Value;
        _shaper = shaper ?? TextShaper.Default;
    }

    /// <summary>A face resolved from a metafile's font object.</summary>
    /// <param name="Face">The OpenType tables, for measuring and shaping.</param>
    /// <param name="Reference">What the display list names the face by.</param>
    public sealed record ResolvedFace(OpenTypeFace Face, FontReference Reference);

    /// <summary>
    /// The code page a GDI character-set byte names.
    /// </summary>
    /// <remarks>
    /// <para>
    /// WMF text is eight-bit and the only thing that says how to read it is this byte on the
    /// font object — so the font a run is drawn with decides what the run <em>says</em>, not
    /// merely what it looks like. Getting it wrong produces the failure the
    /// <c>extraction-comparison</c> skill describes: perfect ASCII with every accented character
    /// mangled.
    /// </para>
    /// <para>
    /// <c>SYMBOL_CHARSET</c> maps to Windows-1252 rather than to anything symbolic, which is
    /// what <c>emfio</c> does too (<c>wmfreader.cxx:1219-1220</c>): a symbol font's bytes index
    /// its own glyphs directly, and there is no code page that describes that. The text will be
    /// wrong; there is no encoding that would make it right.
    /// </para>
    /// </remarks>
    public static int CodePage(byte characterSet) => characterSet switch
    {
        0x00 => 1252,   // ANSI
        0x02 => 1252,   // SYMBOL
        0x4D => 10000,  // MAC
        0x80 => 932,    // SHIFTJIS
        0x81 => 949,    // HANGUL
        0x82 => 1361,   // JOHAB
        0x86 => 936,    // GB2312
        0x88 => 950,    // CHINESEBIG5
        0xA1 => 1253,   // GREEK
        0xA2 => 1254,   // TURKISH
        0xA3 => 1258,   // VIETNAMESE
        0xB1 => 1255,   // HEBREW
        0xB2 => 1256,   // ARABIC
        0xBA => 1257,   // BALTIC
        0xCC => 1251,   // RUSSIAN
        0xDE => 874,    // THAI
        0xEE => 1250,   // EASTEUROPE
        0xFF => 437,    // OEM
        _ => 1252,      // DEFAULT, and anything unrecognised
    };

    /// <summary>Decodes a metafile's eight-bit text with the selected font's code page.</summary>
    public static string Decode(ReadOnlySpan<byte> bytes, MetafileFont font)
    {
        ArgumentNullException.ThrowIfNull(font);

        if (bytes.IsEmpty) return string.Empty;

        Encoding encoding = LegacyCodePages.Get(CodePage(font.CharacterSet));

        // Trailing NULs are routine: the record pads its string to an even length and some
        // producers pad with NUL rather than shortening the count.
        int length = bytes.Length;
        while (length > 0 && bytes[length - 1] == 0) length--;

        return encoding.GetString(bytes[..length]);
    }

    /// <summary>The face a metafile font object asks for, or null when none will load.</summary>
    public ResolvedFace? Face(MetafileFont font)
    {
        ArgumentNullException.ThrowIfNull(font);

        string family = font.Family is { Length: > 0 } named ? named : MetafileFont.Default.Family;
        return _faces.GetOrAdd((family, font.Weight, font.IsItalic), Load);
    }

    /// <summary>The advance width of a string in the given font.</summary>
    public Length Measure(string? text, MetafileFont font)
    {
        ArgumentNullException.ThrowIfNull(font);

        if (string.IsNullOrEmpty(text) || Face(font) is not { } face) return Length.Zero;

        return _shaper.Shape(face.Face, text).Width(font.Size);
    }

    /// <summary>
    /// Shapes a run and places it where a metafile's alignment word says it goes.
    /// </summary>
    /// <param name="text">The characters.</param>
    /// <param name="font">The selected font.</param>
    /// <param name="reference">The point the record states, already mapped.</param>
    /// <param name="alignment">The text-alignment word in force.</param>
    /// <param name="advances">
    /// Per-character advances in EMUs, already mapped, or null to use the font's own. Honouring
    /// them rather than re-measuring is what keeps a producer's intended spacing: the DX array
    /// is how a metafile records the result of <em>its</em> text layout, and a decoder that
    /// re-measures substitutes its own.
    /// </param>
    /// <returns>The run and the width it occupied, or null when there is nothing to draw.</returns>
    public (GlyphRun Run, Length Width)? Layout(
        string? text,
        MetafileFont font,
        DocPoint reference,
        TextAlignment alignment,
        IReadOnlyList<Length>? advances = null)
    {
        ArgumentNullException.ThrowIfNull(font);

        if (string.IsNullOrEmpty(text) || font.Size <= Length.Zero) return null;
        if (Face(font) is not { } face) return null;

        ShapedText shaped = _shaper.Shape(face.Face, text);
        if (shaped.Glyphs.Count == 0) return null;

        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);

        Length width = advances is { Count: > 0 }
            ? Place(shaped, font, advances, glyphs, clusters)
            : Place(shaped, font, glyphs, clusters);

        Length x = (alignment & TextAlignmentMask.Horizontal) switch
        {
            TextAlignment.Centre => reference.X - (width / 2.0),
            TextAlignment.Right => reference.X - width,
            _ => reference.X,
        };

        LineMetrics metrics = LineSpacing.Resolve(face.Face);

        // Tested a bit at a time in priority order rather than as a masked field, because
        // [MS-WMF] 2.1.2.3 gives TA_BASELINE as 0x0018 where it is really 0x0010 — and producers
        // that believed the specification write 0x0018, which a field comparison matches neither
        // as baseline nor as bottom (emfio/inc/mtftools.hxx:184-186, and MtfTools::DrawText's
        // own test at mtftools.cxx:2016-2021).
        Length y;
        if ((alignment & TextAlignment.Baseline) != 0) y = reference.Y;
        else if ((alignment & TextAlignment.Bottom) != 0) y = reference.Y - metrics.ScaledDescent(font.Size);
        else y = reference.Y + metrics.ScaledAscent(font.Size);

        GlyphRun run = new()
        {
            Font = face.Reference,
            FontSize = font.Size,
            Origin = new DocPoint(x, y),
            Glyphs = glyphs,
            Text = text,
            ClusterMap = clusters,
            IsRightToLeft = (alignment & TextAlignment.RightToLeftReading) != 0,
        };

        return (run, width);
    }

    /// <summary>Places glyphs at the advances the font gives them.</summary>
    private static Length Place(
        ShapedText shaped,
        MetafileFont font,
        List<PositionedGlyph> glyphs,
        List<int> clusters)
    {
        Length pen = Length.Zero;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, font.Size);

            // A condensed or expanded logical font states a character width as well as a height;
            // the shaper knows nothing about it, so it scales the advances after the fact.
            if (font.WidthScale > 0) advance *= font.WidthScale;

            // The shaper's vertical offset is up-positive and document space is down-positive.
            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(pen + shaped.Scale(glyph.OffsetX, font.Size), -shaped.Scale(glyph.OffsetY, font.Size)),
                advance));

            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return pen;
    }

    /// <summary>
    /// Places glyphs at the advances the file states.
    /// </summary>
    /// <remarks>
    /// The array is per <em>character</em> and the glyphs are per cluster, so the two are lined
    /// up through the cluster map rather than by position: one ligature covering two characters
    /// takes the sum of their two advances, and a character that decomposed into two glyphs
    /// shares one. Assuming a one-to-one correspondence is right for the Latin text most
    /// metafiles carry and silently wrong for everything else.
    /// </remarks>
    private static Length Place(
        ShapedText shaped,
        MetafileFont font,
        IReadOnlyList<Length> advances,
        List<PositionedGlyph> glyphs,
        List<int> clusters)
    {
        Length pen = Length.Zero;
        int count = shaped.Glyphs.Count;

        for (int i = 0; i < count; i++)
        {
            ShapedGlyph glyph = shaped.Glyphs[i];

            // The advance of this glyph is the sum of the stated advances of every character it
            // covers, which is the characters from its cluster up to the next glyph's.
            int from = Math.Clamp(glyph.Cluster, 0, advances.Count);
            int to = i + 1 < count ? Math.Clamp(shaped.Glyphs[i + 1].Cluster, from, advances.Count) : advances.Count;

            Length advance = Length.Zero;
            for (int c = from; c < to; c++) advance += advances[c];

            // A glyph inside a cluster gets nothing of its own: the whole cluster's width was
            // charged to the first glyph in it.
            if (to == from && i + 1 < count) advance = Length.Zero;

            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(pen + shaped.Scale(glyph.OffsetX, font.Size), -shaped.Scale(glyph.OffsetY, font.Size)),
                advance));

            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return pen;
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
            // A face that will not load is a picture drawn without its lettering, not a document
            // that fails to render.
            return null;
        }
    }
}

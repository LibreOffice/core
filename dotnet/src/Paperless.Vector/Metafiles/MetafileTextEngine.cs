using System.Collections.Concurrent;
using System.Text;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Encodings;
using Paperless.Text.Fonts;
using Paperless.Text.Itemisation;
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
    private readonly ConcurrentDictionary<string, Dictionary<ushort, char>> _reverse = new();

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
    /// <remarks>
    /// Segmented exactly as <see cref="Layout"/> segments it, because the two are compared: an
    /// EMF+ <c>DrawString</c> aligns inside its own layout rectangle by subtracting this width
    /// from it, and a width measured in one face against a run drawn in three would offset the
    /// whole string. Left to right, because nothing that asks for a width states a direction and
    /// a sum of advances does not depend on one.
    /// </remarks>
    public Length Measure(string? text, MetafileFont font)
    {
        ArgumentNullException.ThrowIfNull(font);

        if (string.IsNullOrEmpty(text) || Face(font) is not { } face) return Length.Zero;

        Length width = Length.Zero;
        foreach (Segment segment in Segments(text, font, face, rightToLeft: false))
        {
            width += segment.Shaped.Width(font.Size);
        }

        return width;
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
    /// <returns>The runs and the width they occupied, or null when there is nothing to draw.</returns>
    /// <remarks>
    /// <para>
    /// <b>Several runs rather than one, because a metafile names one font for text that may need
    /// several faces.</b> A GDI text record states a family, a weight and a slant and then a
    /// string; nothing says what script the string is in, and nothing says which face can draw
    /// it. Shaping the whole string in the one resolved face is what a decoder does first and it
    /// is wrong as soon as the requested family is missing: measured on
    /// <c>TestAlignRtlReading.emf</c>, which asks for Noto Sans Arabic and gets Liberation Sans
    /// on a machine without it, <b>35 of the 45 glyphs came back <c>.notdef</c></b> and the
    /// picture measured <c>ink_ratio 0.166</c> against LibreOffice's.
    /// </para>
    /// <para>
    /// So the string goes through the same two itemisers body text goes through and for the same
    /// reasons: <c>TextItemiser</c> for the bidi levels and the script runs, because HarfBuzz
    /// gives a different answer for a run tagged with the wrong direction; then
    /// <c>FontItemiser</c> for coverage, because the resolved face may have none. The base
    /// direction is the record's own <c>TA_RTLREADING</c> bit, which is the only thing a metafile
    /// says about direction at all.
    /// </para>
    /// <para>
    /// <b>The DX array is sliced with the segments rather than abandoned.</b> It is per character
    /// in the record's logical order, so a segment takes the slice its own characters index —
    /// which keeps a producer's spacing exactly where the string needed only one face, the case
    /// every Latin metafile is.
    /// </para>
    /// </remarks>
    public (IReadOnlyList<GlyphRun> Runs, Length Width)? Layout(
        string? text,
        MetafileFont font,
        DocPoint reference,
        TextAlignment alignment,
        IReadOnlyList<Length>? advances = null)
    {
        ArgumentNullException.ThrowIfNull(font);

        if (string.IsNullOrEmpty(text) || font.Size <= Length.Zero) return null;
        if (Face(font) is not { } face) return null;

        bool rightToLeft = (alignment & TextAlignment.RightToLeftReading) != 0;

        List<Segment> segments = Segments(text, font, face, rightToLeft);
        if (segments.Count == 0) return null;

        List<(GlyphRun Run, Length Width)> placed = new(segments.Count);
        Length total = Length.Zero;

        foreach (Segment segment in segments)
        {
            List<PositionedGlyph> glyphs = new(segment.Shaped.Glyphs.Count);
            List<int> clusters = new(segment.Shaped.Glyphs.Count);

            // The slice covers the control characters cut out after the stretch as well, so their
            // stated advances are charged to its last glyph rather than lost. `Place` already
            // charges every array entry from the last glyph's cluster to the end of the slice.
            IReadOnlyList<Length>? slice =
                Slice(advances, segment.Start, segment.Length + segment.Trailing);

            Length width = slice is not null
                ? Place(segment.Shaped, font, slice, glyphs, clusters)
                : Place(segment.Shaped, font, glyphs, clusters);

            if (glyphs.Count == 0) continue;

            GlyphRun run = new()
            {
                Font = segment.Reference,
                FontSize = font.Size,
                Origin = DocPoint.Origin,           // filled in once the total width is known
                Glyphs = glyphs,
                Text = text[segment.Start..(segment.Start + segment.Length)],
                ClusterMap = clusters,
                IsRightToLeft = segment.IsRightToLeft,
            };

            placed.Add((run, width));
            total += width;
        }

        if (placed.Count == 0) return null;

        Length x = (alignment & TextAlignmentMask.Horizontal) switch
        {
            TextAlignment.Centre => reference.X - (total / 2.0),
            TextAlignment.Right => reference.X - total,
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

        List<GlyphRun> runs = new(placed.Count);
        Length pen = x;

        foreach ((GlyphRun run, Length width) in placed)
        {
            runs.Add(run with { Origin = new DocPoint(pen, y) });
            pen += width;
        }

        return (runs, total);
    }

    /// <summary>One stretch of a record's text with a single direction, script and face.</summary>
    /// <param name="Start">Its first character, as an index into the record's text.</param>
    /// <param name="Length">How many characters it covers.</param>
    /// <param name="Trailing">
    /// How many control characters were cut out immediately after this stretch. Their DX entries
    /// belong to it, so that what follows still starts where the producer put it.
    /// </param>
    /// <param name="IsRightToLeft">True when the stretch is drawn right to left.</param>
    /// <param name="Shaped">Its glyphs.</param>
    /// <param name="Reference">The face they were shaped with.</param>
    private readonly record struct Segment(
        int Start, int Length, int Trailing, bool IsRightToLeft, ShapedText Shaped, FontReference Reference);

    /// <summary>
    /// Cuts a record's string into the stretches a shaper can take, in the order they are drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The segments come back in <em>visual</em> order, so a caller can lay them left to right
    /// with a running pen. Inside a right-to-left item the coverage split has to be reversed as
    /// well: <c>FontItemiser</c> answers in logical order, and an Arabic phrase whose digits fall
    /// to one face and whose letters fall to another would otherwise be drawn with the two halves
    /// swapped — which looks like a shaping bug and is a sequencing one.
    /// </para>
    /// <para>
    /// <b>Every C0 control character is cut out first, the tab included.</b> That is
    /// <c>ImplLayoutArgs::AddRun</c> splitting on <c>IsControlChar</c>, which is what a GDI text
    /// record meets in LibreOffice because <c>MtfTools::DrawText</c> plays it through
    /// <c>OutputDevice::DrawTextArray</c>. The tab survives
    /// <see cref="ShapingControls.IsRemovedBeforeShaping"/> so that a paragraph can resolve it against
    /// its own stops; a picture has no stops, so here it can only be a glyph, and no text face has
    /// one. See <see cref="ShapingControls.IsControlCharacter"/> for what that cost.
    /// </para>
    /// <para>
    /// <b>A cut character still spends its entry in the DX array.</b> The array is per character in
    /// the record's logical order and a producer that wrote an advance for its tab meant the next
    /// character to start after it, so the stretch before a run of control characters is widened by
    /// their advances rather than losing them — <see cref="Segment.Trailing"/>. Control characters
    /// before the string's first glyph are the one case that cannot be expressed, because a run is
    /// placed at its first glyph and has nowhere to state a leading blank; no corpus document has one.
    /// </para>
    /// </remarks>
    private List<Segment> Segments(string text, MetafileFont font, ResolvedFace face, bool rightToLeft)
    {
        List<Segment> segments = [];

        List<TextItem> items = TextItemiser.InVisualOrder(TextItemiser.Itemise(
            text, rightToLeft ? BidiDirection.RightToLeft : BidiDirection.LeftToRight));

        foreach (TextItem item in items)
        {
            List<FaceRun> faces = [];
            Dictionary<int, int> trailing = [];

            foreach ((int start, int length, int cut) in Uncontrolled(text, item.Start, item.Length))
            {
                List<FaceRun> piece = FontItemiser.Split(text, start, length, face.Face, _resolver);
                if (piece.Count > 0 && cut > 0) trailing[piece[^1].Start] = cut;
                faces.AddRange(piece);
            }

            if (item.IsRightToLeft) faces.Reverse();

            foreach (FaceRun run in faces)
            {
                // A fallback face is re-resolved by family through the same cache the primary
                // came from, so that the glyph ids and the FontReference a backend embeds are
                // guaranteed to come from one file. Taking the fallback resolver's face object
                // and the resolver's reference separately is how a run ends up drawn with one
                // font's indices and another font's outlines.
                ResolvedFace drawn = face;

                if (run.IsFallback && run.Face.FamilyName is { Length: > 0 } family)
                {
                    drawn = _faces.GetOrAdd((family, font.Weight, font.IsItalic), Load) ?? face;
                }

                ShapedText shaped = _shaper.Shape(
                    drawn.Face,
                    text.AsSpan(run.Start, run.Length),
                    new ShapingOptions(Script: item.Script, RightToLeft: item.IsRightToLeft));

                if (shaped.Glyphs.Count == 0) continue;

                segments.Add(new Segment(
                    run.Start,
                    run.Length,
                    trailing.GetValueOrDefault(run.Start),
                    item.IsRightToLeft,
                    shaped,
                    drawn.Reference));
            }
        }

        return segments;
    }

    /// <summary>
    /// The stretches of a range that hold no control character, in logical order, each with the
    /// number of control characters cut out immediately after it.
    /// </summary>
    /// <param name="text">The record's text.</param>
    /// <param name="start">The range's first character.</param>
    /// <param name="length">How many characters it covers.</param>
    private static List<(int Start, int Length, int Cut)> Uncontrolled(
        ReadOnlySpan<char> text, int start, int length)
    {
        List<(int, int, int)> pieces = [];
        int end = start + length;
        int at = start;

        while (at < end)
        {
            int content = at;
            while (content < end && !ShapingControls.IsControlCharacter(text[content])) content++;

            int cut = content;
            while (cut < end && ShapingControls.IsControlCharacter(text[cut])) cut++;

            if (content > at) pieces.Add((at, content - at, cut - content));
            at = cut;
        }

        return pieces;
    }

    /// <summary>The slice of a DX array a segment's own characters index, or null when there is none.</summary>
    private static IReadOnlyList<Length>? Slice(IReadOnlyList<Length>? advances, int start, int length)
    {
        if (advances is not { Count: > 0 }) return null;
        if (start == 0 && length >= advances.Count) return advances;
        if (start >= advances.Count) return null;

        int take = Math.Min(length, advances.Count - start);
        Length[] slice = new Length[take];
        for (int i = 0; i < take; i++) slice[i] = advances[start + i];
        return slice;
    }

    /// <summary>
    /// Places glyphs a record states by index rather than by character.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>An <c>ETO_GLYPH_INDEX</c> run has already been shaped by the producer</b>, and
    /// re-shaping it would be wrong twice over: the indices are the producer's own choice of
    /// glyph, and the characters they came from are not in the record at all. So they go into
    /// the run as they stand — which is exactly what <c>GlyphRun</c> carries, so nothing is lost.
    /// LibreOffice instead converts them to outlines (<c>emfreader.cxx:2119-2123</c>, tdf#168107)
    /// because its metafile actions cannot express a glyph index; ours can.
    /// </para>
    /// <para>
    /// What is lost is the text, and with it extraction and a PDF's <c>ToUnicode</c> map. It is
    /// recovered by inverting the face's own character map, which is the same table the producer
    /// used to build the indices in the first place. A glyph reached through a substitution or a
    /// ligature will not invert, and those characters are simply absent from
    /// <see cref="GlyphRun.Text"/> rather than guessed at.
    /// </para>
    /// </remarks>
    /// <param name="glyphs">The glyph indices, in visual order.</param>
    /// <param name="font">The selected font.</param>
    /// <param name="reference">The point the record states, already mapped.</param>
    /// <param name="alignment">The text-alignment word in force.</param>
    /// <param name="advances">Per-glyph advances in EMUs, already mapped, or null for the font's own.</param>
    /// <returns>The run and the width it occupied, or null when there is nothing to draw.</returns>
    public (IReadOnlyList<GlyphRun> Runs, Length Width)? LayoutGlyphs(
        IReadOnlyList<ushort> glyphs,
        MetafileFont font,
        DocPoint reference,
        TextAlignment alignment,
        IReadOnlyList<Length>? advances = null)
    {
        ArgumentNullException.ThrowIfNull(glyphs);
        ArgumentNullException.ThrowIfNull(font);

        if (glyphs.Count == 0 || font.Size <= Length.Zero) return null;
        if (Face(font) is not { } face) return null;

        Dictionary<ushort, char> characters = Reverse(face);
        List<PositionedGlyph> placed = new(glyphs.Count);
        List<int> clusters = new(glyphs.Count);
        StringBuilder text = new(glyphs.Count);

        double scale = (double)font.Size.Emu / face.Face.UnitsPerEm;
        Length pen = Length.Zero;

        for (int i = 0; i < glyphs.Count; i++)
        {
            Length advance = advances is { Count: > 0 } && i < advances.Count
                ? advances[i]
                : Length.FromEmu((long)Math.Round(face.Face.AdvanceOf(glyphs[i]) * scale));

            if (advances is null && font.WidthScale > 0) advance *= font.WidthScale;

            clusters.Add(text.Length);
            if (characters.TryGetValue(glyphs[i], out char character)) text.Append(character);

            placed.Add(new PositionedGlyph(glyphs[i], new DocPoint(pen, Length.Zero), advance));
            pen += advance;
        }

        Length x = (alignment & TextAlignmentMask.Horizontal) switch
        {
            TextAlignment.Centre => reference.X - (pen / 2.0),
            TextAlignment.Right => reference.X - pen,
            _ => reference.X,
        };

        LineMetrics metrics = LineSpacing.Resolve(face.Face);

        Length y;
        if ((alignment & TextAlignment.Baseline) != 0) y = reference.Y;
        else if ((alignment & TextAlignment.Bottom) != 0) y = reference.Y - metrics.ScaledDescent(font.Size);
        else y = reference.Y + metrics.ScaledAscent(font.Size);

        GlyphRun run = new()
        {
            Font = face.Reference,
            FontSize = font.Size,
            Origin = new DocPoint(x, y),
            Glyphs = placed,
            Text = text.ToString(),
            ClusterMap = clusters,
            IsRightToLeft = (alignment & TextAlignment.RightToLeftReading) != 0,
        };

        // One run, always: a glyph-index record has already been shaped by the producer, so
        // there is no itemisation to do — no script to infer, no direction to apply, and no
        // fallback possible, because the indices name glyphs in the face the record selected and
        // in no other.
        return ((GlyphRun[])[run], pen);
    }

    /// <summary>
    /// The face's character map, inverted, so a glyph index can be named in the extracted text.
    /// </summary>
    /// <remarks>
    /// <c>CharacterMap</c> answers only in the forward direction, so the inverse is built by
    /// asking it about every code point in the Basic Multilingual Plane's assigned range once
    /// per face and caching the answer. That is about sixty thousand dictionary probes, which is
    /// under a millisecond and happens only for the files that actually use glyph indices.
    /// </remarks>
    private Dictionary<ushort, char> Reverse(ResolvedFace face)
        => _reverse.GetOrAdd(face.Reference.FaceKey, _ =>
        {
            Dictionary<ushort, char> map = [];

            for (int codePoint = 0x20; codePoint < 0xFFFE; codePoint++)
            {
                if (codePoint is >= 0xD800 and <= 0xDFFF) continue;

                ushort glyph = face.Face.Characters.GlyphFor(codePoint);

                // The first code point that reaches a glyph wins, so that ASCII beats the
                // compatibility duplicates that map to the same glyph further up.
                if (glyph != 0) map.TryAdd(glyph, (char)codePoint);
            }

            return map;
        });

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

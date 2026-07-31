using System.Runtime.InteropServices;
using HarfBuzzSharp;
using Paperless.Text.Fonts;
using Buffer = HarfBuzzSharp.Buffer;

namespace Paperless.Text.Shaping;

/// <summary>
/// Shapes with HarfBuzz, configured the way LibreOffice configures it.
/// </summary>
/// <remarks>
/// <para>
/// HarfBuzz is what LibreOffice shapes with, so the point of using it is that advance widths agree by
/// construction rather than by approximation. That only holds if it is set up the same way, and four
/// details of LibreOffice's setup in <c>vcl/</c> are what make the answers match:
/// </para>
/// <list type="bullet">
///   <item>
///     The font is scaled to the face's own units per em —
///     <c>hb_font_set_scale(font, upem, upem)</c> in <c>LogicalFontInstance::InitHbFont</c> — so
///     advances come back on the design grid, unrounded and independent of any output resolution.
///     Shaping at a device size instead would round every glyph and accumulate the error along the
///     line.
///   </item>
///   <item>
///     Glyph metrics come from the OpenType tables (<c>hb_ot_font_set_funcs</c>), not from a
///     rasteriser, so they are the unhinted design values Writer lays out with.
///   </item>
///   <item>
///     No features are passed unless something is being switched off, which leaves HarfBuzz's defaults
///     — <c>kern</c>, <c>liga</c> and <c>clig</c> among them — in force.
///   </item>
///   <item>
///     Clusters are counted per character
///     (<c>HB_BUFFER_CLUSTER_LEVEL_CHARACTERS</c> in <c>CommonSalLayout.cxx</c>), so a cluster index is
///     an index into the text rather than into a grapheme numbering that would have to be mapped back.
///   </item>
/// </list>
/// <para>
/// The face's bytes are handed over whole and HarfBuzz parses the sfnt itself. That is deliberate: it
/// reads <c>GSUB</c> and <c>GPOS</c>, which Paperless's own reader does not, and letting each side
/// parse the file it was given avoids the two disagreeing about which face of a collection they are
/// looking at.
/// </para>
/// </remarks>
public sealed class HarfBuzzShaper : ITextShaper, IDisposable
{
    /// <summary>
    /// A run longer than this is shaped in pieces.
    /// </summary>
    /// <remarks>
    /// Shaping cost is superlinear in some scripts, and a document can contain a paragraph of
    /// pathological length. Splitting caps the damage; the split points are chosen at spaces so the
    /// only shaping lost across a boundary is a kern next to a space, which no font defines.
    /// </remarks>
    private const int MaxCharactersPerShapingCall = 4096;

    private readonly Dictionary<OpenTypeFace, FaceResources> _faces = [];
    private readonly Lock _gate = new();
    private bool _disposed;

    /// <summary>
    /// True when the native harfbuzz library is present and loadable.
    /// </summary>
    /// <remarks>
    /// Probed once by asking harfbuzz for something trivial. Paperless has to keep working where the
    /// native library is missing — an unshaped document is a document with slightly wrong line breaks,
    /// where a <c>DllNotFoundException</c> is no document at all.
    /// </remarks>
    public static bool IsAvailable => Availability.Value;

    /// <inheritdoc/>
    public ShapedText Shape(OpenTypeFace face, ReadOnlySpan<char> text, ShapingOptions options = default)
    {
        ArgumentNullException.ThrowIfNull(face);
        ObjectDisposedException.ThrowIf(_disposed, this);

        if (text.Length == 0) return new ShapedText([], [0], face.UnitsPerEm);

        Font? font = FontFor(face);
        if (font is null) return MetricsShaper.Instance.Shape(face, text, options);

        List<ShapedGlyph> glyphs = new(text.Length);
        int at = 0;
        while (at < text.Length)
        {
            int length = SegmentLength(text, at);
            ShapeSegment(font, text, at, length, options, glyphs);
            at += length;
        }

        ShapedGlyph[] shaped = [.. glyphs];
        return new ShapedText(shaped, ShapedText.PrefixWidths(shaped, text.Length), face.UnitsPerEm);
    }

    /// <inheritdoc/>
    public void Dispose()
    {
        lock (_gate)
        {
            if (_disposed) return;
            _disposed = true;

            foreach (FaceResources resources in _faces.Values) resources.Dispose();
            _faces.Clear();
        }
    }

    /// <summary>
    /// How many characters to shape in one call, starting at a position.
    /// </summary>
    /// <remarks>
    /// The whole remainder when it fits. Otherwise back off to the last space before the cap, so a
    /// segment boundary never falls inside a word — a word split across two shaping calls would lose
    /// its ligatures and its internal kerning.
    /// </remarks>
    private static int SegmentLength(ReadOnlySpan<char> text, int start)
    {
        int remaining = text.Length - start;
        if (remaining <= MaxCharactersPerShapingCall) return remaining;

        int limit = start + MaxCharactersPerShapingCall;
        for (int i = limit - 1; i > start; i--)
        {
            if (text[i] == ' ') return i - start + 1;
        }

        // A single unbroken run longer than the cap: split it anyway rather than shaping something
        // pathological, and take a possibly-wrong kern at the one boundary.
        return MaxCharactersPerShapingCall;
    }

    private static void ShapeSegment(
        Font font,
        ReadOnlySpan<char> text,
        int start,
        int length,
        ShapingOptions options,
        List<ShapedGlyph> into)
    {
        using Buffer buffer = new()
        {
            // Per character, so a cluster is an index into the text. Anything coarser would number
            // graphemes and leave the mapping back to characters to be reconstructed.
            ClusterLevel = ClusterLevel.Characters,
            Direction = options.RightToLeft ? Direction.RightToLeft : Direction.LeftToRight,
        };

        BufferFlags flags = BufferFlags.Default;
        if (start == 0) flags |= BufferFlags.BeginningOfText;
        if (start + length == text.Length) flags |= BufferFlags.EndOfText;
        buffer.Flags = flags;

        buffer.AddUtf16(text[start..(start + length)]);

        // Guessing fills in whatever was not set: the script from the text, and the direction if the
        // caller had no opinion. Then the caller's explicit choices are put back on top.
        buffer.GuessSegmentProperties();

        if (options.Script is { Length: 4 } script)
        {
            buffer.Script = Script.Parse(script);
        }
        if (options.RightToLeft)
        {
            buffer.Direction = Direction.RightToLeft;
        }
        if (options.Language is { Length: > 0 } language)
        {
            buffer.Language = LanguageFor(language);
        }

        font.Shape(buffer, FeaturesFor(options));

        ReadOnlySpan<GlyphInfo> infos = buffer.GetGlyphInfoSpan();
        ReadOnlySpan<GlyphPosition> positions = buffer.GetGlyphPositionSpan();
        int count = Math.Min(infos.Length, positions.Length);

        for (int i = 0; i < count; i++)
        {
            into.Add(new ShapedGlyph(
                (ushort)infos[i].Codepoint,
                start + (int)infos[i].Cluster,
                positions[i].XAdvance,
                positions[i].XOffset,
                positions[i].YOffset));
        }
    }

    /// <summary>
    /// The feature list, which is empty unless something is being switched off.
    /// </summary>
    /// <remarks>
    /// An empty list means HarfBuzz's defaults, and HarfBuzz's defaults are what LibreOffice uses. A
    /// list that named <c>kern</c> and <c>liga</c> explicitly, even with value one, would be a
    /// different thing: it would pin them on for scripts where HarfBuzz would have chosen otherwise.
    /// </remarks>
    private static Feature[] FeaturesFor(ShapingOptions options)
    {
        if (!options.DisableKerning && !options.DisableLigatures) return [];

        List<Feature> features = [];
        if (options.DisableKerning)
        {
            features.Add(Off('k', 'e', 'r', 'n'));
        }
        if (options.DisableLigatures)
        {
            // Both, and only these two: they are the optional ligatures. The required ones live under
            // 'rlig' and stay on, because a script that needs them is unreadable without them.
            features.Add(Off('l', 'i', 'g', 'a'));
            features.Add(Off('c', 'l', 'i', 'g'));
        }
        return [.. features];

        static Feature Off(char a, char b, char c, char d)
            => new(new Tag(a, b, c, d), 0, 0, uint.MaxValue);
    }

    private static Language LanguageFor(string tag)
    {
        try
        {
            return new Language(tag);
        }
        catch (ArgumentException)
        {
            // A tag a document made up is not worth failing a layout over; shaping without it differs
            // only for the handful of language-specific features.
            return Language.Default;
        }
    }

    private Font? FontFor(OpenTypeFace face)
    {
        lock (_gate)
        {
            if (_faces.TryGetValue(face, out FaceResources? existing)) return existing.Font;

            FaceResources? resources = FaceResources.Create(face);
            if (resources is null) return null;

            _faces[face] = resources;
            return resources.Font;
        }
    }

    /// <summary>
    /// The harfbuzz objects for one face, kept for as long as the shaper is.
    /// </summary>
    /// <remarks>
    /// Building them means parsing the whole font, so they are cached per face rather than per call —
    /// laying out a page shapes hundreds of runs with the same handful of faces.
    /// </remarks>
    private sealed class FaceResources : IDisposable
    {
        private readonly GCHandle _pin;
        private readonly Blob _blob;
        private readonly Face _face;

        private FaceResources(GCHandle pin, Blob blob, Face face, Font font)
        {
            _pin = pin;
            _blob = blob;
            _face = face;
            Font = font;
        }

        internal Font Font { get; }

        internal static FaceResources? Create(OpenTypeFace face)
        {
            byte[] bytes = face.File.Bytes.ToArray();
            GCHandle pin = GCHandle.Alloc(bytes, GCHandleType.Pinned);

            try
            {
                // ReadOnly rather than Duplicate: the bytes are pinned for as long as the blob lives,
                // so there is nothing to gain from harfbuzz keeping a second copy of a font file.
                Blob blob = new(pin.AddrOfPinnedObject(), bytes.Length, MemoryMode.ReadOnly);
                blob.MakeImmutable();

                Face harfBuzzFace = new(blob, face.File.FaceIndex);
                harfBuzzFace.MakeImmutable();

                Font font = new(harfBuzzFace);

                // The face's own grid, which is what makes the advances design units. LibreOffice does
                // exactly this, and it is the reason a measurement here can be compared with one there.
                int unitsPerEm = face.UnitsPerEm > 0 ? face.UnitsPerEm : harfBuzzFace.UnitsPerEm;
                font.SetScale(unitsPerEm, unitsPerEm);
                font.SetFunctionsOpenType();

                return new FaceResources(pin, blob, harfBuzzFace, font);
            }
            catch (Exception exception) when (exception is DllNotFoundException
                                                 or EntryPointNotFoundException
                                                 or TypeInitializationException)
            {
                // No native library: the caller falls back to metrics-only shaping.
                pin.Free();
                return null;
            }
            catch
            {
                pin.Free();
                throw;
            }
        }

        public void Dispose()
        {
            Font.Dispose();
            _face.Dispose();
            _blob.Dispose();
            if (_pin.IsAllocated) _pin.Free();
        }
    }

    private static class Availability
    {
        internal static bool Value { get; } = Probe();

        private static bool Probe()
        {
            try
            {
                using Buffer buffer = new();
                buffer.AddUtf16("a");
                return buffer.Length == 1;
            }
            catch (Exception exception) when (exception is DllNotFoundException
                                                 or EntryPointNotFoundException
                                                 or TypeInitializationException
                                                 or BadImageFormatException)
            {
                return false;
            }
        }
    }
}

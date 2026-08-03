using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;

namespace Paperless.Rendering.Tests;

/// <summary>
/// A page built by hand, so a backend can be asked to draw one exact thing.
/// </summary>
/// <remarks>
/// Layout is verified against LibreOffice elsewhere and is not what these tests are about.
/// What they need is a page whose content is known to the decimal — one rectangle at a
/// stated place, one glyph run at a stated baseline — so that what comes out of the backend
/// can be compared with what went in rather than with another renderer's opinion.
/// </remarks>
internal sealed class DrawnPage(DocSize size, Action<IDrawingSink> draw) : IPage
{
    /// <summary>A4, which every corpus document uses.</summary>
    public static DocSize A4 => new(Length.FromMillimetres(210), Length.FromMillimetres(297));

    /// <inheritdoc/>
    public int Index { get; init; }

    /// <inheritdoc/>
    public DocSize Size => size;

    /// <inheritdoc/>
    public string? Label => null;

    /// <inheritdoc/>
    public void Draw(IDrawingSink sink)
    {
        sink.BeginPage(size);
        draw(sink);
        sink.EndPage();
    }
}

/// <summary>A sequence of hand-built pages.</summary>
internal sealed class DrawnPages(params IPage[] pages) : IPageSequence
{
    /// <inheritdoc/>
    public int Count => pages.Length;

    /// <inheritdoc/>
    public IPage this[int index] => pages[index];
}

/// <summary>
/// A real installed face, and glyph runs made from it.
/// </summary>
/// <remarks>
/// A real face rather than a fabricated one because the thing under test is font embedding:
/// a subsetter needs a font with outlines to cut down, and a width needs an <c>hmtx</c> to
/// come from. The tests skip when no face can be resolved rather than asserting against a
/// substitute, since a substituted face would make every width assertion a statement about
/// the wrong font.
/// </remarks>
internal static class TestFace
{
    private static readonly Lazy<(FontReference Reference, OpenTypeFace Face)?> Resolved = new(Resolve);

    /// <summary>True when a usable face was found on this machine.</summary>
    public static bool IsAvailable => Resolved.Value is not null;

    /// <summary>The resolved reference; only valid when <see cref="IsAvailable"/>.</summary>
    public static FontReference Reference => Resolved.Value!.Value.Reference;

    /// <summary>The face behind it.</summary>
    public static OpenTypeFace Face => Resolved.Value!.Value.Face;

    /// <summary>
    /// A glyph run for a string, shaped and positioned like layout's own.
    /// </summary>
    /// <param name="text">What to draw.</param>
    /// <param name="origin">Where the baseline starts.</param>
    /// <param name="size">The em size.</param>
    /// <param name="extraPerSpace">
    /// How much to add to the advance of each blank, which is what justification does and the
    /// reason a run's advances can differ from the font's.
    /// </param>
    public static GlyphRun Run(string text, DocPoint origin, Length size, Length extraPerSpace = default)
    {
        ShapedText shaped = TextShaper.Default.Shape(Face, text);

        List<PositionedGlyph> glyphs = [];
        List<int> clusters = [];
        Length pen = Length.Zero;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, size);
            if (extraPerSpace != Length.Zero
                && glyph.Cluster >= 0 && glyph.Cluster < text.Length && text[glyph.Cluster] == ' ')
            {
                advance += extraPerSpace;
            }

            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(pen + shaped.Scale(glyph.OffsetX, size), -shaped.Scale(glyph.OffsetY, size)),
                advance));

            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new GlyphRun
        {
            Font = Reference,
            FontSize = size,
            Origin = origin,
            Glyphs = glyphs,
            Text = text,
            ClusterMap = clusters,
        };
    }

    private static (FontReference, OpenTypeFace)? Resolve()
    {
        try
        {
            SystemFontResolver resolver = SystemFontResolver.Build();

            foreach (string family in (string[])["Liberation Serif", "DejaVu Serif", "Carlito", "serif"])
            {
                FontReference reference = resolver.Resolve(new FontRequest(family));
                if (reference.FaceKey.Length == 0) continue;

                OpenTypeFace face = resolver.LoadOpenType(reference);
                return (reference, face);
            }
        }
        catch (InvalidOperationException)
        {
            // No readable face: the tests that need one skip rather than assert about a substitute.
        }

        return null;
    }
}

/// <summary>
/// A real installed face with <c>CFF</c> outlines rather than <c>glyf</c> ones.
/// </summary>
/// <remarks>
/// <para>
/// Found by its bytes rather than by its name, because what matters is the flavour and no
/// family name states it: an sfnt beginning <c>OTTO</c> carries a <c>CFF </c> table, which PDF
/// admits only as a <c>/FontFile3</c>. Any <c>.otf</c> on the machine will do, so scanning is
/// both more portable than naming a family and closer to the thing under test.
/// </para>
/// <para>
/// The face is loaded straight from its path, bypassing <c>SystemFontResolver</c>: the
/// resolver answers a family request through fontconfig and would hand back whatever the
/// machine prefers for that name, which on most Linux machines is a TrueType face.
/// </para>
/// </remarks>
internal static class TestCffFace
{
    /// <summary>The sfnt version tag <c>OTTO</c>.</summary>
    private static readonly byte[] Otto = "OTTO"u8.ToArray();

    private static readonly Lazy<(FontReference Reference, OpenTypeFace Face)?> Resolved = new(Resolve);

    /// <summary>True when a CFF-flavoured face was found on this machine.</summary>
    public static bool IsAvailable => Resolved.Value is not null;

    /// <summary>The resolved reference; only valid when <see cref="IsAvailable"/>.</summary>
    public static FontReference Reference => Resolved.Value!.Value.Reference;

    /// <summary>The face behind it.</summary>
    public static OpenTypeFace Face => Resolved.Value!.Value.Face;

    /// <summary>A glyph run for a string, shaped and positioned like layout's own.</summary>
    /// <param name="text">What to draw.</param>
    /// <param name="origin">Where the baseline starts.</param>
    /// <param name="size">The em size.</param>
    public static GlyphRun Run(string text, DocPoint origin, Length size)
    {
        ShapedText shaped = TextShaper.Default.Shape(Face, text);

        List<PositionedGlyph> glyphs = [];
        List<int> clusters = [];
        Length pen = Length.Zero;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, size);
            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(pen + shaped.Scale(glyph.OffsetX, size), -shaped.Scale(glyph.OffsetY, size)),
                advance));
            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new GlyphRun
        {
            Font = Reference,
            FontSize = size,
            Origin = origin,
            Glyphs = glyphs,
            Text = text,
            ClusterMap = clusters,
        };
    }

    private static (FontReference, OpenTypeFace)? Resolve()
    {
        foreach (string directory in (string[])
                 ["/usr/share/fonts", "/usr/local/share/fonts", "/Library/Fonts"])
        {
            if (!Directory.Exists(directory)) continue;

            foreach (string path in Directory.EnumerateFiles(directory, "*.otf", SearchOption.AllDirectories))
            {
                try
                {
                    byte[] head = new byte[4];
                    using (FileStream stream = File.OpenRead(path))
                    {
                        if (stream.Read(head) != 4 || !head.AsSpan().SequenceEqual(Otto)) continue;
                    }

                    if (OpenTypeFace.ReadFile(path) is not { } face) continue;

                    return (
                        new FontReference
                        {
                            FamilyName = face.FamilyName ?? Path.GetFileNameWithoutExtension(path),
                            FaceKey = path,
                        },
                        face);
                }
                catch (IOException)
                {
                    // Unreadable font file: try the next one.
                }
                catch (UnauthorizedAccessException)
                {
                    // Likewise.
                }
            }
        }

        return null;
    }
}

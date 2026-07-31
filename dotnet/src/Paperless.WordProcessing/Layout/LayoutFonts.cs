using Paperless.Core.Graphics;
using Paperless.Text.Fonts;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// Resolves the faces a layout needs, once per distinct request.
/// </summary>
/// <remarks>
/// <para>
/// A document has a handful of fonts and thousands of runs, and resolving one means walking a
/// substitution chain and reading a font file off disk — so the cache is not an optimisation but the
/// difference between laying out a book and appearing to hang. The key is what actually decides which
/// file is opened: the requested family, the weight and the slant.
/// </para>
/// <para>
/// A face that cannot be read caches as a miss rather than throwing. There is nothing to measure the text
/// with, and a caller that dropped the paragraph would produce a silently shorter document — so the
/// decision is left to the caller, which usually falls back to the paragraph's own face.
/// </para>
/// </remarks>
public sealed class LayoutFonts
{
    private readonly SystemFontResolver _fonts;
    private readonly Dictionary<(string? Family, int Weight, bool Italic), Resolved> _resolved = [];

    /// <summary>Creates a cache over a resolver, or over the installed fonts.</summary>
    /// <param name="fonts">The resolver to use, or null to build one over the installed fonts.</param>
    public LayoutFonts(SystemFontResolver? fonts = null)
        => _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());

    /// <summary>The substitutions made so far.</summary>
    /// <remarks>
    /// Worth surfacing rather than swallowing: a substitution that is not metric-compatible changes every
    /// line break after the first paragraph that uses it, so it is the first thing to check when a
    /// comparison against a reference renderer disagrees.
    /// </remarks>
    public IReadOnlyList<FontSubstitution> Substitutions => _fonts.Substitutions;

    /// <summary>The face a request resolves to, or null when nothing could be read.</summary>
    public OpenTypeFace? Face(string? family, int weight, bool isItalic)
        => Lookup(family, weight, isItalic).Face;

    /// <summary>The reference a request resolves to, or null when nothing could be read.</summary>
    /// <remarks>
    /// Kept beside the face rather than derived from it, because the two answer different questions: the
    /// face has the metrics that decided the layout, and the reference records <em>which</em> face that was
    /// and what was asked for before substitution.
    /// </remarks>
    public FontReference? Reference(string? family, int weight, bool isItalic)
        => Lookup(family, weight, isItalic).Reference;

    private Resolved Lookup(string? family, int weight, bool isItalic)
    {
        (string? Family, int Weight, bool Italic) key = (family, weight, isItalic);
        if (_resolved.TryGetValue(key, out Resolved cached)) return cached;

        Resolved resolved = default;
        try
        {
            FontReference reference = _fonts.Resolve(
                new FontRequest(family ?? string.Empty, weight, isItalic));

            resolved = new Resolved(_fonts.LoadOpenType(reference), reference);
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // A font that cannot be read is not a reason to fail a layout; the caller decides what to
            // measure the text with instead.
        }

        _resolved[key] = resolved;
        return resolved;
    }

    private readonly record struct Resolved(OpenTypeFace? Face, FontReference? Reference);
}

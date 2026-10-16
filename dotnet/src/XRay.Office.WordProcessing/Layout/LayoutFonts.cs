using XRay.Office.Core.Graphics;
using XRay.Office.Text.Fonts;

namespace XRay.Office.WordProcessing.Layout;

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

    /// <summary>
    /// The device grid this document's font metrics are rounded through, or null to scale them exactly.
    /// </summary>
    /// <remarks>
    /// The document's answer rather than a face's, which is why it lives beside the resolver: a document
    /// that asks to be laid out against a printer asks it of every font it uses. Null for all but the few
    /// that do — see <see cref="MetricGrid"/>.
    /// </remarks>
    public MetricGrid? Metrics { get; init; }

    /// <summary>
    /// What the document's own font table says the shape of a family is, or null when it has none.
    /// </summary>
    /// <remarks>
    /// The document's answer rather than the resolver's, which is why it lives here beside
    /// <see cref="Metrics"/>: the font table belongs to one file and the resolver is shared. It only
    /// matters for a family that is neither installed nor substitutable — but that is the common case
    /// on a Linux box, and it is the difference between a serif document rendering in DejaVu Serif as
    /// LibreOffice renders it and in DejaVu Sans, which reflows every line of it.
    /// </remarks>
    public Func<string, DeclaredFontShape>? DeclaredShapes { get; init; }

    /// <summary>
    /// Where a paragraph looks for a face when its own has no glyph for a character.
    /// </summary>
    /// <remarks>
    /// The resolver that chose the face in the first place, which is what
    /// <see cref="IGlyphFallbackResolver"/> asks for: a fallback decided by a second index could name
    /// a face this one would not have chosen, and the face decides the advance.
    /// </remarks>
    public IGlyphFallbackResolver Fallback => _fonts;

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
            // The declared family, not the declared pitch. Both are in the table and only the first
            // was measured to move LibreOffice — see DocxLayoutSource.Face for the probe.
            //
            // The two arms that reach here take *different* answers, and which one applies is
            // decided by whether the document supplied a font table at all:
            //
            //   * DOC supplies one, and its `FFN` states a class per font. That class is the whole
            //     answer, `Unknown` included — the WW8 reader sets `FAMILY_DONTKNOW` on the item for
            //     `ff` 0, 6 and 7, which appends no generic to the fontconfig pre-match, so the
            //     answer is fontconfig's own and not the roman default. Measured on nine flat-ODF
            //     fixtures exported to Word 97 and back: only `roman` draws DejaVu Serif.
            //   * RTF supplies none, and never states a class the filter acts on — `\fnil`,
            //     `\froman`, `\fswiss` and `\fmodern` are all inert, measured — so the family is
            //     never set and Writer's roman pool default stands.
            //
            // See `WordFallbackClass`, `probes/words-r55/doc-family-code.py` and
            // `probes/words-r54/cross-format-fallback.py`.
            FontFamilyClass declaredClass = DeclaredShapes is { } declared
                ? WordFallbackClass.ForWw8Font(
                    family, family is { Length: > 0 } named ? declared(named).Class : FontFamilyClass.Unknown)
                : WordFallbackClass.ForDeclared(family, FontFamilyClass.Unknown);

            FontReference reference = _fonts.Resolve(
                new FontRequest(
                    family ?? string.Empty, weight, isItalic, DeclaredClass: declaredClass));

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

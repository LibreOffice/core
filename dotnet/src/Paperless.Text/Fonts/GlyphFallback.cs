namespace Paperless.Text.Fonts;

/// <summary>
/// Chooses a face for a character the face in force cannot draw.
/// </summary>
/// <remarks>
/// <para>
/// Separate from <see cref="IFontResolver"/> because it answers a different question. A resolver
/// answers "what did the author mean by <em>Calibri</em>"; this answers "the face I have has no
/// glyph for this character, what does". The first is asked once per run, the second only for the
/// characters a run cannot show, and a document that needs neither should pay for neither.
/// </para>
/// <para>
/// LibreOffice asks the platform first — fontconfig's <c>FcFontMatch</c> with the missing characters
/// as a charset, in <c>vcl/unx/generic/fontmanager/fontconfig.cxx</c> — and falls back to a
/// hard-coded list of families when that fails. Paperless has only the second half, deliberately:
/// going through fontconfig would add a second source of truth for substitution rather than the
/// missing one, and the same document would then render differently on two machines with the same
/// fonts but different fontconfig configurations.
/// </para>
/// </remarks>
public interface IGlyphFallbackResolver
{
    /// <summary>
    /// A face that can draw a character, or null when nothing installed can.
    /// </summary>
    /// <param name="codePoint">The character the primary face has no glyph for.</param>
    /// <param name="weight">The weight to match, on the OpenType 1-1000 scale.</param>
    /// <param name="isItalic">Whether an italic face is wanted.</param>
    OpenTypeFace? FallbackFor(int codePoint, int weight = 400, bool isItalic = false);
}

/// <summary>One mid-run fallback: a stretch the run's own face could not show.</summary>
/// <remarks>
/// Reported rather than applied silently, for the same reason a family substitution is. A fallback
/// face is almost never metric-compatible with the one it replaces, so the run it lands in measures
/// differently and every line after it can break somewhere else — and a caller comparing against a
/// reference renderer otherwise has no way to tell that from a layout bug. One entry per contiguous
/// stretch rather than per character, so a paragraph in a script the face does not cover leaves one
/// line in the list and not a thousand.
/// </remarks>
/// <param name="CodePoint">The first character of the stretch that was missing.</param>
/// <param name="FromFamily">The family the run was set in.</param>
/// <param name="ToFamily">The family that drew it, or null when nothing could.</param>
public readonly record struct GlyphFallback(int CodePoint, string? FromFamily, string? ToFamily)
{
    /// <summary>True when a face was found; false means the character draws as a missing-glyph box.</summary>
    public bool IsResolved => ToFamily is not null;

    /// <inheritdoc/>
    public override string ToString()
        => IsResolved
            ? $"U+{CodePoint:X4} not in {FromFamily}, drawn from {ToFamily}"
            : $"U+{CodePoint:X4} not in {FromFamily}, and in nothing installed";
}

/// <summary>
/// The generic glyph-fallback list LibreOffice carries, in its own order.
/// </summary>
/// <remarks>
/// Ported verbatim from <c>ImplInitGenericGlyphFallback</c> in
/// <c>vcl/source/font/PhysicalFontCollection.cxx</c>. It is grouped: each group holds families that
/// cover roughly the same characters, so the first installed member of a group is as good as any
/// other and the groups are tried in turn. Porting it rather than inventing an order matters because
/// which face draws a missing character decides its advance width, and therefore where the line
/// holding it breaks.
/// </remarks>
public static class GlyphFallbackFamilies
{
    /// <summary>The families to try, in order, as normalised names.</summary>
    public static IReadOnlyList<string> InOrder { get; } =
    [
        "eudc",
        "arialunicodems", "cyberbit", "code2000",
        "andalesansui",
        "starsymbol", "opensymbol",
        "msmincho", "fzmingti", "fzheiti", "ipamincho", "sazanamimincho", "kochimincho",
        "sunbatang", "sundotum", "baekmukdotum", "gulim", "batang", "dotum",
        "hgmincholightj", "msunglightsc", "msunglighttc", "hymyeongjolightk",
        "tahoma", "dejavusans", "timesnewroman", "liberationsans",
        "shree", "mangal",
        "raavi", "shruti", "tunga",
        "latha", "gautami", "kartika", "vrinda",
        "shayyalmt", "naskmt", "scheherazade",
        "david", "nachlieli", "lucidagrande",
        "norasi", "angsanaupc",
        "khmerossystem",
        "muktinarrow",
        "phetsarathot",
        "padauk", "pinlonmyanmar",
        "iskoolapota", "lklug",
    ];
}

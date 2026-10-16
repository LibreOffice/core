namespace XRay.Office.Text.Fonts;

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
/// LibreOffice asks the platform first — <c>FcFontSetMatch</c> with the missing characters as a
/// charset, in <c>vcl/unx/generic/font/fontconfig.cxx</c> — and falls back to a hard-coded list of
/// families only when that answers nothing
/// (<c>PhysicalFontCollection::GetGlyphFallbackFont</c>, <c>:231-291</c>). X-Ray.Office asks them in
/// the same order, and the platform half is modelled from the machine's own fontconfig
/// configuration rather than from the font files: see <see cref="FontconfigPreferences"/>, which
/// records the measurement showing that this answer cannot be derived from the fonts.
/// <para>
/// <strong>The list came first here until round 64 and it was a measured defect.</strong> The list
/// heads with <c>starsymbol, opensymbol</c>, so every character OpenSymbol covers was drawn from
/// OpenSymbol — while OpenSymbol is on no fontconfig preference list at all, so the reference never
/// answers a glyph fallback with it. On a machine with no fontconfig the list still comes first,
/// because there is then no configuration for the other half to read.
/// </para>
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

    /// <summary>
    /// The same question, told which face the run was set in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Which face fontconfig answers with depends on the request, not only on the
    /// character.</strong> <c>FontConfigManager::Substitute</c> builds the pattern from the
    /// requested family and appends <c>serif</c> or <c>sans</c> for the item's family class
    /// (<c>vcl/unx/generic/font/fontconfig.cxx</c>:1075-1088), and it is <em>that</em> generic's
    /// preference list that ranks the faces covering the character. Measured on 26.2.4.2: a run
    /// declared <c>swiss</c> draws <c>U+2713</c> in DejaVu Sans and the same run declared
    /// <c>roman</c> draws it in FreeSerif.
    /// </para>
    /// <para>
    /// The face is passed rather than the request because it is what every caller already holds —
    /// <see cref="Itemisation.FontItemiser"/> is given the primary face and nothing else — and
    /// because a resolver that chose the face can look the request back up from it. A resolver that
    /// cannot answers as it did before, which is what the default does.
    /// </para>
    /// </remarks>
    /// <param name="codePoint">The character the primary face has no glyph for.</param>
    /// <param name="weight">The weight to match, on the OpenType 1-1000 scale.</param>
    /// <param name="isItalic">Whether an italic face is wanted.</param>
    /// <param name="primary">The face the run was set in, or null when the caller has none.</param>
    OpenTypeFace? FallbackFor(int codePoint, int weight, bool isItalic, OpenTypeFace? primary)
        => FallbackFor(codePoint, weight, isItalic);

    /// <summary>
    /// The same question asked for <em>all</em> of a run's missing characters at once.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>LibreOffice asks for one face covering the whole set, and that is not the same
    /// question as asking about each character on its own.</strong>
    /// <c>OutputDevice::ImplGlyphFallbackLayout</c> collects every code unit a layout could not map
    /// into one string and hands it down as <c>rMissingCodes</c>
    /// (<c>vcl/source/outdev/font.cxx</c>); <c>FontConfigManager::Substitute</c> puts every code
    /// point of it into a single <c>FC_CHARSET</c>
    /// (<c>vcl/unx/generic/font/fontconfig.cxx</c>:1092-1116); and <c>FcCompareCharSet</c> scores a
    /// candidate by how many of the set it is <em>missing</em>, at <c>PRI_CHARSET</c> — fontconfig's
    /// highest priority, above the family list and above the language. So a face that covers more of
    /// the run wins over one that is better placed and covers less.
    /// </para>
    /// <para>
    /// The chosen face is then subtracted from the set and the next fallback level asks again with
    /// the remainder, which is why the answer for one character can depend on what else the run was
    /// missing. Measured: <c>AAC-AD-No-2021-01…doc</c> draws <c>U+2011</c> in FreeSerif where a
    /// one-character probe of the same request draws it in DejaVu Serif.
    /// </para>
    /// <para>
    /// Defaulted to the single-character question over the first of the set, so an implementation
    /// that does not model it stays valid and answers as it did.
    /// </para>
    /// </remarks>
    /// <param name="codePoints">The characters the primary face has no glyph for, in text order.</param>
    /// <param name="weight">The weight to match, on the OpenType 1-1000 scale.</param>
    /// <param name="isItalic">Whether an italic face is wanted.</param>
    /// <param name="primary">The face the run was set in, or null when the caller has none.</param>
    OpenTypeFace? FallbackFor(
        IReadOnlyList<int> codePoints, int weight, bool isItalic, OpenTypeFace? primary)
        => codePoints is { Count: > 0 }
            ? FallbackFor(codePoints[0], weight, isItalic, primary)
            : null;

    /// <summary>
    /// The same question, told which of the run's font items the characters came from.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The item is the pattern.</strong> Its family is the pattern's first
    /// <c>FC_FAMILY</c>, its class decides the generic appended as the second, and its language is
    /// the <c>FC_LANG</c> that outranks both — see <see cref="FontItem"/> for why it has to travel
    /// with the run and cannot be recovered from the face.
    /// </para>
    /// <para>
    /// Defaulted to the item-less question, so a caller with no script items to distinguish — a
    /// slide, a sheet, a metafile — behaves exactly as it did.
    /// </para>
    /// </remarks>
    /// <param name="codePoints">The characters the primary face has no glyph for, in text order.</param>
    /// <param name="weight">The weight to match, on the OpenType 1-1000 scale.</param>
    /// <param name="isItalic">Whether an italic face is wanted.</param>
    /// <param name="primary">The face the run was set in, or null when the caller has none.</param>
    /// <param name="item">The font item the run is set from, or default when the caller has none.</param>
    OpenTypeFace? FallbackFor(
        IReadOnlyList<int> codePoints, int weight, bool isItalic, OpenTypeFace? primary, FontItem item)
        => FallbackFor(codePoints, weight, isItalic, primary);

    /// <summary>
    /// The same question for a run set in a pi face, which fontconfig is never asked about.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>LibreOffice declines to ask fontconfig when the face in force is OpenSymbol, and
    /// that is the whole of the difference.</strong>
    /// <c>FcGlyphFallbackSubstitution::FindFontSubstitute</c> returns false outright for a
    /// Microsoft-symbol-encoded pattern and again for OpenSymbol — "a unicode font, but it still
    /// deserves to be treated as a symbol font"
    /// (<c>vcl/unx/generic/font/fontsubst.cxx</c>:100-107). With the hook declining,
    /// <c>PhysicalFontCollection::GetGlyphFallbackFont</c> falls to
    /// <c>ImplInitGenericGlyphFallback</c>'s fixed list and nothing else
    /// (<c>vcl/source/font/PhysicalFontCollection.cxx</c>:283-291), so a character no family on
    /// that list covers is drawn as the pi face's own missing-glyph box rather than being sought
    /// across everything installed.
    /// </para>
    /// <para>
    /// It is a real difference and not a nicety, because the list is short and the machine is not.
    /// Measured on 26.2.4.2: <c>23-session-2-pptx.pptx</c> recodes a Webdings <c>a</c> bullet to
    /// <c>U+E340</c> (<c>aWebDingsTab</c>) and <c>FAAAIandtheArtandScienceofV&amp;Vfinal.pptx</c>
    /// recodes one to <c>U+E63F</c>; OpenSymbol holds neither, nothing on the generic list holds
    /// either, and the only installed faces that do are <c>Unifont CSUR</c> and
    /// <c>Unifont Sample</c> — neither of which is on the list. The reference draws OpenSymbol's
    /// box; we drew a Unifont glyph, 25 times on the first deck.
    /// </para>
    /// <para>
    /// The list is still searched, which is what keeps the other half right: the same deck class
    /// includes <c>FAA_Form_337.ppt</c>, whose five Monotype Sorts slots recode to
    /// <c>U+2776</c>-<c>U+277A</c>, and <c>dejavusans</c> is on the generic list and holds all
    /// five — so the reference draws them in DejaVu Sans and so do we.
    /// </para>
    /// <para>
    /// Defaulted to the ordinary answer so an implementation that does not model the distinction
    /// stays valid and behaves as it did. A wrapper that forwards <see cref="FallbackFor(int, int, bool)"/> must
    /// forward this too, or the run it guards silently loses the rule.
    /// </para>
    /// </remarks>
    /// <param name="codePoint">The character the pi face has no glyph for.</param>
    /// <param name="weight">The weight to match, on the OpenType 1-1000 scale.</param>
    /// <param name="isItalic">Whether an italic face is wanted.</param>
    OpenTypeFace? SymbolFallbackFor(int codePoint, int weight = 400, bool isItalic = false)
        => FallbackFor(codePoint, weight, isItalic);

    /// <summary>
    /// The reference naming a face this resolver returned, or null when it did not return it.
    /// </summary>
    /// <remarks>
    /// A face on its own is enough to <em>measure</em> and to <em>shape</em>, and not enough to
    /// <em>embed</em>: a PDF writer loads the font program through the reference's face key, so a
    /// fallback face named only by its family reaches the writer with no program behind it and is
    /// announced in the file without being embedded. Measured on <c>手机免提系统TSB.doc</c>, whose
    /// three fallback faces all came out <c>emb no</c> — which the corpus gate scores as a failure,
    /// correctly, because a reader without those fonts installed sees nothing.
    /// <para>
    /// Defaulted to null so an implementation that only answers coverage questions stays valid; the
    /// caller then falls back to naming the face, which is what it did before this existed.
    /// </para>
    /// </remarks>
    Core.Graphics.FontReference? ReferenceFor(OpenTypeFace face) => null;

    /// <summary>
    /// The same reference, carrying the lean a fallback face has no italic of its own to give.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="ReferenceFor(OpenTypeFace)"/> is a reverse lookup from a face and says so: it has
    /// <em>no request to compare against</em>, so it cannot answer
    /// <c>LogicalFontInstance::NeedsArtificialItalic()</c> — <em>italic was asked for and the face
    /// that answered has none</em> — and it never has. Every face reached through
    /// <see cref="Itemisation.FontItemiser"/> therefore arrived at the page upright however italic
    /// the run around it was, which is synthetic oblique lost a second time and at a different seat
    /// from the one round 56 closed in the four word-processing readers.
    /// </para>
    /// <para>
    /// The request is the caller's to supply, so it is a parameter rather than something inferred:
    /// a fallback face is by definition not the face the run asked for, and its own
    /// <c>IsItalic</c> is a fact about the substitute rather than about the document. The rule
    /// itself lives here, once, because all three tracks reach it — <c>PageDrawing.ByFace</c>,
    /// <c>SlideTextLayout.Block.FontFor</c> and <c>SheetFonts.ForFallback</c>.
    /// </para>
    /// <para>
    /// Measured against LibreOffice <b>26.2.4.2</b> on 41 authored two-run packages over
    /// <em>six</em> filters — <c>.docx</c>, <c>.fodt</c>, <c>.fodp</c>, <c>.fods</c>, <c>.pptx</c>
    /// and <c>.xlsx</c> — in <c>probes/words-r58/fallback-oblique.py</c> and
    /// <c>fallback-oblique-ooxml.py</c>. The format is varied deliberately, because the one earlier
    /// round that got a fallback question wrong here got it wrong by holding the format fixed. Every
    /// italic case shears on the reference and none did here: CJK to WenQuanYi Zen Hei 6 of 6,
    /// symbols and Hebrew to DejaVu Sans 4 of 4, the same under a bold request, and the same when
    /// the primary face is itself only synthetically oblique. Four negative controls — italic Latin
    /// in a family whose italic <em>is</em> installed, and the identical fallback text with no
    /// italic asked for — are nought on both sides in all six formats.
    /// </para>
    /// <para>
    /// <strong>The reference does not go looking for an italic face, and that is what makes this the
    /// whole of the fix.</strong> Hebrew from an italic <c>Carlito</c> run is covered by DejaVu Sans,
    /// which has no italic here, and by Liberation Sans, which does. 26.2.4.2 draws it in
    /// <b>DejaVu Sans, sheared</b> — its fallback order wins over the slant, exactly as
    /// <see cref="SystemFontResolver.FallbackFor(int, int, bool)"/> already ranks family above slant. So the face
    /// this interface picks was already right and only the lean was missing.
    /// </para>
    /// </remarks>
    /// <param name="face">A face <see cref="FallbackFor(int, int, bool)"/> returned.</param>
    /// <param name="isItalicRequested">
    /// Whether the run this face is standing in for asked for italic. That is <em>not</em> the
    /// primary face's <c>IsItalic</c> alone: a primary that is itself being sheared asked for italic
    /// too and has no italic face to prove it with, and the reference shears the fallback in that
    /// case as well.
    /// </param>
    Core.Graphics.FontReference? ReferenceFor(OpenTypeFace face, bool isItalicRequested)
        => ReferenceFor(face) is { } reference
            ? reference with { SyntheticOblique = isItalicRequested && !face.IsItalic }
            : null;
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

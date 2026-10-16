using XRay.Office.Core.Diagnostics;
using XRay.Office.Core.Graphics;

namespace XRay.Office.Text.Fonts;

/// <summary>One face found on the machine: where it is, and what it says about itself.</summary>
/// <param name="Path">The file the face lives in.</param>
/// <param name="FaceIndex">Which face of that file, for a collection.</param>
/// <param name="FamilyName">The family name the face reports.</param>
/// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">Whether the face is italic or oblique.</param>
/// <param name="IsFixedPitch">Whether every glyph has the same advance.</param>
public readonly record struct InstalledFace(
    string Path,
    int FaceIndex,
    string FamilyName,
    int Weight,
    bool IsItalic,
    bool IsFixedPitch)
{
    /// <summary>A stable key for the face, for caching and for deduplicating embedded fonts.</summary>
    public string FaceKey => FaceIndex == 0 ? Path : $"{Path}#{FaceIndex}";

    /// <summary>The family name in the form the substitution table is keyed on.</summary>
    public string NormalisedFamily => FontSubstitutions.Normalise(FamilyName);
}

/// <summary>
/// Finds the faces installed on this machine, by reading their <c>name</c> tables.
/// </summary>
/// <remarks>
/// <para>
/// The font files are read rather than fontconfig being asked. That is a deliberate trade: it costs a
/// scan of the font directories at start-up and gives up any substitution rules an administrator
/// configured, in exchange for the same answer on every machine and no native dependency. Since the
/// substitution chain comes from LibreOffice's own table rather than from the platform, going through
/// fontconfig would add a second source of truth rather than the missing one.
/// </para>
/// <para>
/// One decision is the exception, and it is marked out rather than quietly folded in: which of
/// several installed faces draws a character <em>nothing on LibreOffice's fallback list</em> covers.
/// No table answers that, and the answer is measurably a property of the machine's fontconfig
/// configuration rather than of the fonts — see <see cref="FontconfigPreferences"/>. That is read,
/// and only for that. Everything above it still comes from the table.
/// </para>
/// <para>
/// Only the family name, weight and slant are read at index time, which is a few kilobytes per file
/// rather than the whole face. A machine with several hundred fonts is indexed in well under a
/// second, and nothing is parsed twice: the face itself is loaded only when something asks to measure
/// with it.
/// </para>
/// </remarks>
public sealed class SystemFontIndex
{
    /// <summary>The directories searched, in order, when none is given.</summary>
    /// <remarks>
    /// The user's own fonts come first, because a font a user installed for themselves is the one
    /// they expect a document to render in.
    /// </remarks>
    public static IReadOnlyList<string> DefaultDirectories { get; } = Defaults();

    /// <summary>Builds the search order: the faces we ship, then the platform's own.</summary>
    /// <remarks>
    /// <para>
    /// The platform list is a local rather than a second static field, and that is not a style
    /// choice. Static field and property initialisers run in <em>declaration order</em>, so a
    /// <c>DefaultDirectories</c> declared above a <c>PlatformDirectories</c> it reads gets null
    /// and the type initialiser throws — which surfaces as
    /// <c>TypeInitializationException</c> from every call site at once, not as anything that
    /// points at the ordering. Keeping the list here means there is no order to get wrong.
    /// </para>
    /// <para>
    /// The faces we ship go <em>last</em>, so an installed one always wins. They are a floor
    /// under the font set, not an override of it: their job is that a machine missing Carlito or
    /// DejaVu renders correctly instead of silently substituting, which is the failure
    /// <c>dotnet/CLAUDE.md</c> records as moving 53 of 534 page counts with LibreOffice held
    /// constant. See <see cref="BundledFonts"/> for the measurement that settled the direction,
    /// and for the opt-in that reverses it.
    /// </para>
    /// </remarks>
    private static List<string> Defaults()
    {
        List<string> directories = [];

        if (BundledFonts.Preferred && BundledFonts.Directory is { } first) directories.Add(first);

        string home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
        directories.AddRange([
            Path.Combine(home, ".local", "share", "fonts"),
            Path.Combine(home, ".fonts"),
            "/usr/local/share/fonts",
            "/usr/share/fonts",
            "/Library/Fonts",
            "/System/Library/Fonts",
            Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows), "Fonts"),
        ]);

        // Last, so an installed face always wins. That is the safe direction and it was measured
        // rather than assumed -- see BundledFonts.Preferred.
        if (!BundledFonts.Preferred && BundledFonts.Directory is { } last) directories.Add(last);

        return directories;
    }

    /// <summary>The extensions a font file may have.</summary>
    private static readonly string[] Extensions = [".ttf", ".otf", ".ttc", ".otc"];

    private readonly Dictionary<string, List<InstalledFace>> _families =
        new(StringComparer.Ordinal);

    private readonly List<Diagnostic> _diagnostics = [];

    /// <summary>Every face found, in no particular order.</summary>
    public IEnumerable<InstalledFace> Faces => _families.Values.SelectMany(f => f);

    /// <summary>How many families were found.</summary>
    public int FamilyCount => _families.Count;

    /// <summary>Files that looked like fonts and could not be read.</summary>
    public IReadOnlyList<Diagnostic> Diagnostics => _diagnostics;

    /// <summary>Scans the given directories, or the platform's defaults.</summary>
    public static SystemFontIndex Build(IEnumerable<string>? directories = null)
    {
        SystemFontIndex index = new();

        foreach (string directory in directories ?? DefaultDirectories)
        {
            if (!Directory.Exists(directory)) continue;

            IEnumerable<string> files;
            try
            {
                files = Directory.EnumerateFiles(directory, "*", SearchOption.AllDirectories);
            }
            catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
            {
                // A directory the process cannot read is not a reason to have no fonts at all.
                continue;
            }

            foreach (string file in files)
            {
                if (!Extensions.Contains(Path.GetExtension(file), StringComparer.OrdinalIgnoreCase))
                    continue;

                index.Add(file);
            }
        }

        return index;
    }

    /// <summary>The faces of a family, or empty when none is installed under that name.</summary>
    public IReadOnlyList<InstalledFace> Family(string? familyName)
        => _families.TryGetValue(FontSubstitutions.Normalise(familyName), out List<InstalledFace>? faces)
            ? faces
            : [];

    /// <summary>True when a family is installed.</summary>
    public bool Has(string? familyName) => Family(familyName).Count > 0;

    /// <summary>
    /// The face of a family closest to a requested weight and slant.
    /// </summary>
    /// <remarks>
    /// Slant first, then weight distance. Getting an upright face where an italic was asked for is
    /// visibly wrong in a way that a hundred points of weight is not, so slant is never traded for a
    /// closer weight — which is what sorting by a combined score would do.
    /// </remarks>
    public InstalledFace? Best(string? familyName, int weight, bool italic)
    {
        IReadOnlyList<InstalledFace> faces = Family(familyName);
        if (faces.Count == 0) return null;

        InstalledFace? best = null;
        int bestScore = int.MaxValue;

        foreach (InstalledFace face in faces)
        {
            int score = (face.IsItalic == italic ? 0 : 10_000) + Math.Abs(face.Weight - weight);
            if (score >= bestScore) continue;

            bestScore = score;
            best = face;
        }

        return best;
    }

    private void Add(string path)
    {
        OpenTypeFace? first;
        try
        {
            first = OpenTypeFace.ReadFile(path);
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
        {
            return;
        }

        if (first is null)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Information, "PL5000",
                "A file with a font extension is not a font this reader understands, so it has been "
                + "ignored.",
                new DiagnosticLocation(path)));
            return;
        }

        int faces = first.File.FaceCount;
        for (int index = 0; index < faces; index++)
        {
            OpenTypeFace? face = index == 0 ? first : SafeRead(path, index);
            if (face?.FamilyName is not { Length: > 0 } family) continue;

            InstalledFace installed = new(
                path, index, family, face.Weight, face.IsItalic, face.IsFixedPitch);

            if (!_families.TryGetValue(installed.NormalisedFamily, out List<InstalledFace>? list))
            {
                list = [];
                _families[installed.NormalisedFamily] = list;
            }

            // The same family may be installed twice — a system copy and a user copy — and the first
            // directory searched wins, since that is the one a user expects.
            if (!list.Any(f => f.Weight == installed.Weight && f.IsItalic == installed.IsItalic))
                list.Add(installed);
        }
    }

    private static OpenTypeFace? SafeRead(string path, int faceIndex)
    {
        try
        {
            return OpenTypeFace.ReadFile(path, faceIndex);
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
        {
            return null;
        }
    }
}

/// <summary>
/// Resolves a document's font request against the installed faces, the way LibreOffice would.
/// </summary>
/// <remarks>
/// <para>
/// The order is LibreOffice's: the requested family if it is installed, then its substitution chain
/// from LibreOffice's own table, then a face of the right general shape, then a last resort. Never
/// null — a document that names a font nobody has still has to render, and refusing to choose would
/// turn a cosmetic difference into a failure.
/// </para>
/// <para>
/// Every substitution is reported, and whether it was metric-compatible is reported with it. That
/// distinction is the difference between "this page looks slightly different" and "every page after
/// this one is wrong", because a substitute with different advance widths reflows the text and moves
/// every break after the first one.
/// </para>
/// </remarks>
public sealed class SystemFontResolver : IFontResolver, IGlyphFallbackResolver
{
    private readonly SystemFontIndex _index;
    private readonly Dictionary<string, OpenTypeFace> _loaded = new(StringComparer.Ordinal);
    private readonly Dictionary<OpenTypeFace, string> _keys = new(ReferenceEqualityComparer.Instance);
    private readonly List<FontSubstitution> _substitutions = [];
    private readonly Dictionary<(string CodePoints, int Weight, bool Italic, string Generic, string Language), OpenTypeFace?>
        _fallbacks = [];
    private readonly Dictionary<string, string> _genericByFaceKey = new(StringComparer.Ordinal);
    private readonly Dictionary<string, string> _languageByFaceKey = new(StringComparer.Ordinal);
    private readonly List<GlyphFallback> _glyphFallbacks = [];
    private readonly FontconfigPreferences _preferences;

    /// <summary>Creates a resolver over an index of installed faces.</summary>
    /// <param name="index">The faces found on the machine.</param>
    /// <param name="preferences">
    /// The machine's fontconfig preferences, used only to order the last-resort glyph fallback;
    /// null takes this machine's own, which is read once per process.
    /// </param>
    public SystemFontResolver(SystemFontIndex index, FontconfigPreferences? preferences = null)
    {
        ArgumentNullException.ThrowIfNull(index);
        _index = index;
        _preferences = preferences ?? FontconfigPreferences.Machine;
    }

    /// <summary>Creates a resolver over the platform's font directories.</summary>
    public static SystemFontResolver Build() => new(SystemFontIndex.Build());

    /// <summary>The index this resolver searches.</summary>
    public SystemFontIndex Index => _index;

    /// <summary>
    /// Every substitution made so far, in the order they were made.
    /// </summary>
    /// <remarks>
    /// Recorded rather than logged: a silent substitution explains most otherwise-baffling reflow
    /// differences, so a caller comparing output against a reference wants the list, not a message
    /// that went to a log nobody read.
    /// </remarks>
    public IReadOnlyList<FontSubstitution> Substitutions => _substitutions;

    /// <summary>
    /// Every mid-run glyph fallback made so far, one per contiguous stretch, in the order they were made.
    /// </summary>
    /// <remarks>
    /// The same argument as <see cref="Substitutions"/>, and a sharper one: a fallback face is
    /// chosen for its coverage rather than for its metrics, so it is almost never metric-compatible
    /// with the face it stands in for. A run that quietly used two faces measures differently from
    /// one that used one, and without this list there is nothing to distinguish that from a layout
    /// bug. Characters that nothing installed could draw are recorded too, with a null family — a
    /// missing-glyph box is worth knowing about before it is seen on a page.
    /// </remarks>
    public IReadOnlyList<GlyphFallback> GlyphFallbacks => _glyphFallbacks;

    /// <inheritdoc/>
    public OpenTypeFace? FallbackFor(int codePoint, int weight = 400, bool isItalic = false)
        => FallbackFor(codePoint, weight, isItalic, primary: null);

    /// <inheritdoc/>
    /// <remarks>
    /// <para>
    /// <strong>fontconfig first, and LibreOffice's own generic list only when it answers
    /// nothing.</strong> <c>PhysicalFontCollection::GetGlyphFallbackFont</c> calls the fallback
    /// hook and reaches <c>ImplInitGenericGlyphFallback</c>'s list only <c>if (!pFallbackData)</c>
    /// (<c>vcl/source/font/PhysicalFontCollection.cxx</c>:231-291). This used to ask them the other
    /// way about, and the list heads with <c>starsymbol, opensymbol</c> — so every character
    /// OpenSymbol covers was drawn from OpenSymbol, where the reference draws it from whatever
    /// fontconfig ranks best. On this machine OpenSymbol is on no fontconfig preference list at
    /// all, so the reference never answers a glyph fallback with it.
    /// </para>
    /// <para>
    /// <strong>What fontconfig ranks best is decided by <em>one</em> generic's preference list, and
    /// which generic that is comes from the request rather than from the character.</strong>
    /// <c>FontConfigManager::Substitute</c> puts the requested family in the pattern and appends
    /// <c>serif</c> for <c>FAMILY_ROMAN</c> and <c>sans</c> for <c>FAMILY_SWISS</c>
    /// (<c>vcl/unx/generic/font/fontconfig.cxx</c>:1075-1088) — the same function and the same
    /// switch the pre-match substitution uses, because the glyph-fallback hook goes through it too.
    /// <c>FcConfigSubstitute</c> then expands that generic's <c>&lt;prefer&gt;</c> list into the
    /// pattern's family list and <c>FC_CHARSET</c> outranks it, so the answer is *the first face on
    /// that one list that covers the character*. See
    /// <see cref="FontconfigPreferences.RankOf(string?, string?)"/> for the measurement.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 over six declared classes and twelve characters
    /// (<c>probes/fonts-r64/gen-generic.py</c>): <c>U+2713</c> is drawn in <b>FreeSerif</b> when the
    /// class is roman, modern, script, decorative or undeclared — Writer's own default is roman —
    /// and in <b>DejaVu Sans</b> when it is swiss; <c>U+2011</c> is DejaVu <b>Serif</b> against
    /// DejaVu <b>Sans</b> on the same split; <c>U+4E00</c> is WenQuanYi Zen Hei either way, because
    /// it is the first face on both lists that covers it.
    /// </para>
    /// <para>
    /// <strong>The emoji list is asked first, and that is a language rule rather than a family
    /// one.</strong> <c>getExemplarLangTagForCodePoint</c> answers <c>und-zsye</c> for a character
    /// with the Unicode <c>Emoji</c> property (<c>fontconfig.cxx</c>:1026-1029), and fontconfig
    /// scores <c>PRI_LANG</c> above <c>PRI_FAMILY_WEAK</c> — so an emoji code point goes to the
    /// emoji face whatever generic the pattern named. Asked as a coverage question over the
    /// <c>emoji</c> preference list rather than as a Unicode property, because the two are the same
    /// set by construction: the property is what an emoji font is built to cover. Measured:
    /// <c>U+2714</c>, <c>U+2611</c> and <c>U+263A</c> answer <b>Noto Color Emoji</b> under all six
    /// declared classes, while <c>U+2713</c> — which the property excludes and Noto Color Emoji
    /// does not hold — answers the generic's list.
    /// </para>
    /// <para>
    /// The generic list keeps the machine with no fontconfig behaving exactly as it did: there it
    /// is asked first, because the second stage's ordering is fontconfig's and there is none.
    /// </para>
    /// </remarks>
    public OpenTypeFace? FallbackFor(int codePoint, int weight, bool isItalic, OpenTypeFace? primary)
        => FallbackFor([codePoint], weight, isItalic, primary);

    /// <inheritdoc/>
    /// <remarks>
    /// <para>
    /// <strong>The pattern carries the whole set at once, and that is a different question from the
    /// same one asked character by character.</strong>
    /// <c>OutputDevice::ImplGlyphFallbackLayout</c> gathers every code unit of the layout that came
    /// back unmapped into one <c>OUString</c> and hands it to
    /// <c>PhysicalFontCollection::GetGlyphFallbackFont</c>
    /// (<c>vcl/source/outdev/font.cxx</c>), which passes it through to
    /// <c>FontConfigManager::Substitute</c> as a single <c>FC_CHARSET</c>
    /// (<c>vcl/unx/generic/font/fontconfig.cxx</c>:1092-1116). <c>FcCompareCharSet</c> scores by
    /// <em>how many of the set the face is missing</em> and <c>PRI_CHARSET</c> is fontconfig's
    /// highest priority, so a face covering more of the run's missing characters beats one that is
    /// better placed on the family list and covers fewer.
    /// </para>
    /// <para>
    /// The answer is then subtracted from the set — <c>Substitute</c> ends by rebuilding
    /// <c>rMissingCodes</c> from the code points the chosen face's charset does <em>not</em> hold —
    /// and the next fallback level asks again with what is left. So one run can reach several
    /// fallback faces, and which characters each one draws depends on the others.
    /// </para>
    /// <para>
    /// Ordering the levels is the caller's, because it is the caller that knows which characters a
    /// run was missing; see <see cref="Itemisation.FontItemiser"/>.
    /// </para>
    /// </remarks>
    public OpenTypeFace? FallbackFor(
        IReadOnlyList<int> codePoints, int weight, bool isItalic, OpenTypeFace? primary)
        => FallbackFor(codePoints, weight, isItalic, primary, default);

    /// <inheritdoc/>
    public OpenTypeFace? FallbackFor(
        IReadOnlyList<int> codePoints, int weight, bool isItalic, OpenTypeFace? primary, FontItem item)
    {
        ArgumentNullException.ThrowIfNull(codePoints);
        if (codePoints.Count == 0) return null;

        // The run's own item where the caller has one, and the reverse lookup from the face where it
        // has not: a slide or a sheet has no script items to choose between.
        string generic = item.IsStated
            ? GenericFor(new FontRequest(
                item.FamilyName ?? string.Empty, DeclaredClass: item.DeclaredClass))
            : GenericForFallback(primary);
        string language = (item.IsStated ? item.Language : LanguageForFallback(primary))
                          ?? string.Empty;
        string key = string.Join('\u0000', codePoints);

        // Cached because a run of unsupported text asks the same question for every character, and
        // answering it means opening font files until one covers the character.
        if (_fallbacks.TryGetValue((key, weight, isItalic, generic, language), out OpenTypeFace? cached))
        {
            return cached;
        }

        OpenTypeFace? found;

        if (_preferences.IsConfigured)
        {
            found = OnPreferenceList(codePoints, weight, isItalic, EmojiGeneric)
                    ?? Preferred(codePoints, weight, isItalic, generic, language)
                    ?? OnGenericList(codePoints, weight, isItalic);
        }
        else
        {
            found = OnGenericList(codePoints, weight, isItalic)
                    ?? Preferred(codePoints, weight, isItalic, generic, language);
        }

        _fallbacks[(key, weight, isItalic, generic, language)] = found;
        return found;
    }

    /// <summary>The language a pattern whose face is this one would carry as <c>FC_LANG</c>.</summary>
    /// <remarks>
    /// The same reverse lookup <see cref="GenericForFallback"/> makes and with the same first-writer
    /// rule, because the two travel together: a run's font item supplies both, and a face this
    /// resolver never chose carries neither.
    /// </remarks>
    private string? LanguageForFallback(OpenTypeFace? primary)
        => primary is not null
           && _keys.TryGetValue(primary, out string? faceKey)
           && _languageByFaceKey.TryGetValue(faceKey, out string? language)
            ? language
            : null;

    /// <summary>fontconfig's generic family for a pattern whose face is this one.</summary>
    /// <remarks>
    /// The generic the <em>request</em> carried, where this resolver answered the request and so
    /// knows it — a <c>.doc</c> whose <c>FFN</c> declares <c>Calibri</c> swiss and
    /// <c>Times New Roman</c> roman puts its two runs on two different lists, and only the request
    /// says so. A face this resolver did not choose — an embedded one, or one loaded by key — falls
    /// back to the generic fontconfig files the face's own family under, which is what the pattern
    /// would have carried had the document declared nothing.
    /// </remarks>
    private string GenericForFallback(OpenTypeFace? primary)
    {
        if (primary is not null
            && _keys.TryGetValue(primary, out string? faceKey)
            && _genericByFaceKey.TryGetValue(faceKey, out string? declared))
        {
            return declared;
        }

        return (_preferences.IsConfigured ? _preferences.GenericNameOf(primary?.FamilyName) : null)
               ?? DefaultGeneric;
    }

    /// <summary>fontconfig's own answer for a pattern that named no generic: <c>49-sansserif.conf</c>.</summary>
    private const string DefaultGeneric = "sansserif";

    /// <summary>The generic whose preference list a code point with the Emoji property lands on.</summary>
    private const string EmojiGeneric = "emoji";

    /// <summary>The first face on one generic's preference list that covers every missing character.</summary>
    /// <remarks>
    /// <strong>Every one of them, not the first.</strong> <c>PRI_CHARSET</c> outranks every family
    /// key, so a list member that covers only part of the set does not beat a face further down
    /// that covers all of it — and this stage exists to short-circuit the full ranking, which it may
    /// only do where it cannot be wrong. Where nothing on the list covers the whole set the search
    /// falls through to <see cref="Preferred"/>, which relaxes the coverage requirement one
    /// character at a time.
    /// </remarks>
    private OpenTypeFace? OnPreferenceList(
        IReadOnlyList<int> codePoints, int weight, bool isItalic, string generic)
    {
        foreach (string family in _preferences.InOrderFor(generic))
        {
            if (_index.Best(family, weight, isItalic) is not { } candidate) continue;
            if (CoversAll(candidate, codePoints) is { } face) return face;
        }

        return null;
    }

    /// <summary>The best covering face by fontconfig's scoring, as far as this models it.</summary>
    /// <remarks>
    /// Coverage is the filter and the generic's preference order is the rank, because
    /// <c>FC_CHARSET</c> outranks <c>FC_FAMILY</c> and the family list outranks slant and weight
    /// (<c>PRI_CHARSET</c>, then <c>PRI_FAMILY_WEAK</c>, then <c>PRI_SLANT</c> and
    /// <c>PRI_WEIGHT</c> in <c>fcmatch.c</c>). The merged order across every generic is the
    /// tie-break: a face on no list at all scores the same as any other on
    /// <c>PRI_FAMILY_WEAK</c>, and this keeps such a face ordered as it was before the lists were
    /// read apart. Ordinal family name is the last resort, which is the order LibreOffice's own
    /// font set is in — <c>FontCfgWrapper::getFontSet</c> stable-sorts it by family name
    /// (<c>vcl/unx/generic/font/fontconfig.cxx</c>:362), and <c>FcFontSetMatch</c> keeps the first
    /// of equal scores.
    /// </remarks>
    /// <param name="codePoints">The characters the run's own face could not draw, in text order.</param>
    /// <param name="weight">The weight to match, on the OpenType 1-1000 scale.</param>
    /// <param name="isItalic">Whether an italic face is wanted.</param>
    /// <param name="generic">The generic whose preference list ranks the candidates.</param>
    /// <param name="language">
    /// The font item's language, or empty for none. Ranked <em>above</em> the generic's preference
    /// list, because <c>fcmatch.c</c> scores <c>PRI_LANG</c> above <c>PRI_FAMILY_WEAK</c> — which is
    /// what makes a Hebrew character in a complex-script run answer FreeSans where the sans-serif
    /// list on its own answers DejaVu Sans. See <see cref="FontLanguages"/>.
    /// </param>
    private OpenTypeFace? Preferred(
        IReadOnlyList<int> codePoints, int weight, bool isItalic, string generic, string language)
    {
        int exemplar = FontLanguages.ExemplarOf(language);

        List<InstalledFace> ordered = _index.Faces
            .OrderBy(face => _preferences.RankOf(face.FamilyName, generic))
            .ThenBy(face => _preferences.RankOf(face.FamilyName))
            .ThenBy(face => face.IsItalic == isItalic ? 0 : 1)
            .ThenBy(face => Math.Abs(face.Weight - weight))
            .ThenBy(face => face.FamilyName, StringComparer.Ordinal)
            .ToList();

        // `PRI_CHARSET` first, `PRI_LANG` second, the family keys last. The coverage requirement is
        // relaxed one character at a time rather than every face being loaded and counted, so a
        // single missing character -- which is nearly every call -- costs exactly what it did
        // before the set existed.
        for (int allowed = 0; allowed < codePoints.Count; allowed++)
        {
            for (int pass = exemplar == FontLanguages.Neutral ? 1 : 0; pass < 2; pass++)
            {
                foreach (InstalledFace candidate in ordered)
                {
                    if (pass == 0 && Covers(candidate, exemplar) is null) continue;

                    if (CoversAllBut(candidate, codePoints, allowed) is { } face) return face;
                }
            }
        }

        return null;
    }

    /// <summary>The first face on LibreOffice's own generic glyph-fallback list that covers a character.</summary>
    private OpenTypeFace? OnGenericList(IReadOnlyList<int> codePoints, int weight, bool isItalic)
    {
        for (int allowed = 0; allowed < codePoints.Count; allowed++)
        {
            foreach (string family in GlyphFallbackFamilies.InOrder)
            {
                if (_index.Best(family, weight, isItalic) is not { } candidate) continue;
                if (CoversAllBut(candidate, codePoints, allowed) is { } face) return face;
            }
        }

        return null;
    }

    /// <inheritdoc/>
    /// <remarks>
    /// LibreOffice's own list and nothing else. The interface's own remarks hold the mechanism and
    /// the measurements; the implementation is the ordinary search with its second stage — the
    /// stand-in for the fontconfig hook — left off, because that hook is exactly what
    /// <c>FcGlyphFallbackSubstitution::FindFontSubstitute</c> declines to run for a pi face
    /// (<c>vcl/unx/generic/font/fontsubst.cxx</c>:100-107).
    /// </remarks>
    public OpenTypeFace? SymbolFallbackFor(int codePoint, int weight = 400, bool isItalic = false)
        => OnGenericList([codePoint], weight, isItalic);

    /// <summary>Records a fallback, resolved or not, for the caller comparing against a reference.</summary>
    public void RecordGlyphFallback(int codePoint, string? fromFamily, string? toFamily)
        => _glyphFallbacks.Add(new GlyphFallback(codePoint, fromFamily, toFamily));

    /// <summary>The face behind an installed entry when it covers a character, else null.</summary>
    /// <remarks>
    /// <para>
    /// <strong>Covering the character is not enough; the face has to be able to draw it.</strong>
    /// Every one of the three fallback stages — the emoji list, the request's own generic list and
    /// LibreOffice's fixed list — asks its candidates through this one method, so putting the test
    /// here makes an unpaintable candidate fall through to the next one everywhere at once, rather
    /// than ending the search with a face that produces an empty run.
    /// </para>
    /// <para>
    /// It is a floor and it deliberately changes no preference: the candidates are offered in
    /// exactly the order they were, and one is only ever <em>skipped</em>, never promoted. So the
    /// advance follows whichever face actually draws, because the face this returns is the face that
    /// is measured, shaped and painted. See <see cref="GlyphPainting.CanPaint"/> for what counts as
    /// paintable and what is deliberately left out of it.
    /// </para>
    /// <para>
    /// Nothing installed here fires it: the one colour face on the machine is Noto Color Emoji, and
    /// its <c>CBDT</c> strikes are read. It is the guard the round that made that face reachable did
    /// not have.
    /// </para>
    /// </remarks>
    private OpenTypeFace? Covers(InstalledFace candidate, int codePoint)
    {
        OpenTypeFace? face = LoadCached(candidate.FaceKey);
        return face is not null && GlyphPainting.CanPaintCharacter(face, codePoint) ? face : null;
    }

    /// <summary>The face behind an installed entry when it covers every character, else null.</summary>
    private OpenTypeFace? CoversAll(InstalledFace candidate, IReadOnlyList<int> codePoints)
        => CoversAllBut(candidate, codePoints, 0);

    /// <summary>
    /// The face behind an installed entry when it is missing at most <paramref name="allowed"/> of
    /// the characters, else null.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>FcCompareCharSet</c> scores a font by <c>FcCharSetSubtractCount</c> — the size of the
    /// pattern's charset minus the font's — so this is that score, tested against a budget rather
    /// than compared, which is what lets the search stay lazy.
    /// </para>
    /// <para>
    /// <strong>A character the face cannot <em>paint</em> counts as missing, not as covered.</strong>
    /// This is <see cref="Covers"/>'s guard applied to the set, and the two must agree: the round
    /// that made the whole set one question and the round that added the guard landed against the
    /// same base, and merging them textually left this path asking <c>HasGlyphFor</c> alone — which
    /// is coverage, so a face that maps a character and keeps its shape in a table this renderer
    /// does not read would end the search and draw a blank at the right advance. That is the defect
    /// the guard exists to prevent, reintroduced on the path that had become the common one.
    /// </para>
    /// </remarks>
    private OpenTypeFace? CoversAllBut(InstalledFace candidate, IReadOnlyList<int> codePoints, int allowed)
    {
        OpenTypeFace? face = LoadCached(candidate.FaceKey);
        if (face is null) return null;

        int missing = 0;
        foreach (int codePoint in codePoints)
        {
            if (GlyphPainting.CanPaintCharacter(face, codePoint)) continue;
            if (++missing > allowed) return null;
        }

        // A face that covers none of them is not an answer, however generous the budget.
        return missing < codePoints.Count ? face : null;
    }

    /// <summary>Loads a face by key through the resolver's own cache, or null when it cannot be read.</summary>
    private OpenTypeFace? LoadCached(string faceKey)
    {
        if (_loaded.TryGetValue(faceKey, out OpenTypeFace? existing))
        {
            // Also on the hit, because the face may have been loaded through `LoadOpenType` — which
            // fills `_loaded` and not `_keys` — and a fallback that finds it there would otherwise
            // have no key to be embedded by.
            _keys[existing] = faceKey;
            return existing;
        }

        (string path, int index) = SplitKey(faceKey);

        OpenTypeFace? face;
        try
        {
            face = path.Length > 0 ? OpenTypeFace.ReadFile(path, index) : null;
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
        {
            // A font that cannot be read is not a reason to abandon the search for one that can.
            face = null;
        }

        if (face is not null)
        {
            _loaded[faceKey] = face;
            _keys[face] = faceKey;
        }

        return face;
    }

    /// <inheritdoc/>
    /// <remarks>
    /// A reverse lookup over the faces this resolver has loaded rather than a fresh resolution: the
    /// face is the answer already given, and re-resolving by family name could land on a different
    /// file — a family with several faces installed, or a collection whose members share one name.
    /// Reference equality is the right key for exactly that reason.
    /// </remarks>
    public FontReference? ReferenceFor(OpenTypeFace face)
        => face is not null && _keys.TryGetValue(face, out string? faceKey)
            ? new FontReference
            {
                FamilyName = face.FamilyName ?? string.Empty,
                RequestedFamily = face.FamilyName ?? string.Empty,
                Weight = face.Weight,
                IsItalic = face.IsItalic,
                FaceKey = faceKey,
            }
            : null;

    /// <summary>The families a resolver falls back to when nothing in the chain is installed.</summary>
    /// <remarks>
    /// <para>
    /// DejaVu first, and that ordering is measured rather than chosen. A chain that names none of
    /// the faces on the machine is where LibreOffice stops consulting its own table and asks
    /// fontconfig, and fontconfig's generic families resolve to DejaVu on a stock Linux
    /// configuration — <c>60-latin.conf</c> heads every one of its three preference lists with it.
    /// [24.2.7-audit: VERIFIED 2026-08-21, round words-r53 — unrecognised families still all
    /// land on DejaVu on 26.2.4.2. Which DejaVu is wrong; see GenericFallbacks.]
    /// Verified against LibreOffice 24.2.7.2 on this machine over fifty-five families: every single
    /// one that reached the generic path landed on DejaVu Sans, DejaVu Serif or DejaVu Sans Mono,
    /// and none landed on Liberation.
    /// <strong>Re-checked against 26.2.4.2 on 2026-08-21 (`TODO.24-2-7-audit.md`,
    /// `probes/words-r53/font-fallback-recheck.py`) and still correct in this respect:</strong> ten
    /// unrecognised families, four installed controls, and every unrecognised one landed on DejaVu.
    /// <b>But which DejaVu is wrong — see <see cref="GenericFallbacks"/>.</b>
    /// </para>
    /// <para>
    /// Preferring Liberation here looked right and is not: Liberation is the metric-compatible
    /// stand-in for Arial, Times New Roman and Courier New specifically, and those three reach it
    /// through their <em>chains</em>, which name it outright. By the time control arrives here the
    /// request is for something Liberation was never built to imitate, so its metrics carry no
    /// authority — and choosing it puts a face on the page that LibreOffice would not have used.
    /// </para>
    /// </remarks>
    private static readonly string[] SerifFallbacks =
        ["dejavuserif", "liberationserif", "timesnewroman", "freeserif", "notoserif"];

    private static readonly string[] SansFallbacks =
        ["dejavusans", "liberationsans", "arial", "freesans", "notosans"];

    private static readonly string[] MonoFallbacks =
        ["dejavusansmono", "liberationmono", "couriernew", "freemono", "notosansmono"];

    /// <summary>The faces a family fontconfig files under <c>math</c> resolves to.</summary>
    /// <remarks>
    /// <para>
    /// <strong>The maths generic is not a shape and cannot be served by one.</strong>
    /// <c>45-generic.conf</c> files <c>Cambria Math</c> and six siblings under <c>math</c>, and
    /// then — in the same file — prepends <c>lang=und-zmth</c> to any pattern whose family is
    /// <c>math</c>. So the match is decided by which installed face declares the *mathematical
    /// orthography*, which no shape fallback can express.
    /// </para>
    /// <para>
    /// The head of the list is <c>60-generic.conf</c>'s own <c>math</c> preference order, verbatim
    /// and in its order. Behind it sits the orthography carrier, which on a stock Debian or Ubuntu
    /// is GNU FreeFont's <c>FreeSerif</c>: <c>fc-list :lang=und-zmth</c> answers it and nothing
    /// else here. Behind that the sans-serif fallbacks, because a machine with neither a maths
    /// face nor FreeFont has nothing to satisfy the lang test and the pattern falls all the way
    /// through to the configuration's overall default — which is the case the earlier
    /// "Cambria Math draws in DejaVu Sans" measurement was taken in.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 over the four corpus documents that name <c>Cambria Math</c> —
    /// <c>DRX-Ascend System Course Description.docx</c>,
    /// <c>075_Storyboard_Template_Fillable_Format…docx</c>,
    /// <c>084_Printable_Graph_Paper_Template…docx</c> and
    /// <c>AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc</c>. All four use it for a handful of
    /// characters (a <c>U+2010</c> hyphen, in DRX-Ascend's case, seven times); the reference draws
    /// every one of them in <c>FreeSerif</c> and we drew them in <c>DejaVuSerif</c>.
    /// </para>
    /// </remarks>
    private static readonly string[] MathFallbacks =
        ["xitsmath", "stixtwomath", "cambriamath", "latinmodernmath",
         "minionmath", "lucidamath", "asanamath"];

    /// <summary>The face that declares the mathematical orthography where no maths font does.</summary>
    /// <remarks>
    /// <para>
    /// GNU FreeFont's roman: <c>fc-list :lang=und-zmth</c> answers <c>FreeSerif.ttf</c> and nothing
    /// else on a stock Debian or Ubuntu, and <em>only its regular file</em> — <c>FreeSerifBold</c>
    /// does not declare the orthography.
    /// </para>
    /// <para>
    /// <strong>Which is why it is taken at regular weight and upright whatever was asked
    /// for.</strong> fontconfig ranks a language requirement above weight and slant, so a pattern
    /// carrying <c>und-zmth</c> can only be satisfied by the one file that declares it. Measured:
    /// <c>fc-match "Cambria Math:bold"</c> answers FreeSerif <em>Regular</em> where
    /// <c>fc-match "FreeSerif:bold"</c> answers FreeSerif Bold, and 26.2.4.2 draws
    /// <c>075_Storyboard_Template_Fillable_Format…docx</c>'s title — a 48 pt <c>w:b</c> run in
    /// Cambria Math — in FreeSerif regular.
    /// </para>
    /// </remarks>
    private const string MathOrthographyFace = "freeserif";

    /// <summary>The faces to try for a request that names no family at all.</summary>
    /// <remarks>
    /// A blank family name is not a family nobody has installed — it is a document expressing no
    /// preference, and the answer to that is the application's default rather than the answer given
    /// to an unrecognised name. So it comes from the configuration's <c>DefaultFonts</c> node rather
    /// than its <c>FontSubstitutions</c> node, and it is read from there rather than transcribed:
    /// the list is data in the tree, and a hand-copied prefix of it would silently diverge on a
    /// machine whose installed faces differ from this one's. Measured: a fixture declaring no font
    /// [24.2.7-audit: VERIFIED 2026-08-21, round words-r54 — Liberation Serif on 26.2.4.2, by two
    /// fixtures that reach DefaultFonts where round 53's reached Word's default instead.]
    /// renders in Liberation Serif, and that now holds on <b>26.2.4.2</b> as well as on 24.2.7.2.
    /// <para>
    /// Round 53 could not decide this and said why: a DOCX carrying no <c>styles.xml</c> at all is
    /// given <em>Word's</em> default rather than LibreOffice's, and its no-family case duly came
    /// back Carlito. Round 54 built the two fixtures that do reach <c>DefaultFonts</c>
    /// (<c>probes/words-r54/font-fallback-rule.py</c>, cases <c>odf:no-font-at-all</c> and
    /// <c>nofamily:docx-empty-docdefaults</c>): a flat ODF file declaring no font anywhere, and a
    /// DOCX whose <c>docDefaults</c> state an empty <c>w:rFonts</c>. <b>Both render in Liberation
    /// Serif</b>, which is what this list heads with, and the DOCX case is the stronger of the two
    /// because that filter's own default for a *named* family is roman — so this is not the roman
    /// default arriving by another route.
    /// </para>
    /// <para>
    /// The confounder round 53 hit has a third face worth recording: <c>w:ascii=""</c> — an empty
    /// attribute rather than an absent element — comes back <b>DejaVu Serif</b>, because the filter
    /// reads it as a named family that happens to be empty and applies its roman default to it. So
    /// "the document says nothing" and "the document says nothing at all, in writing" are two
    /// states in LibreOffice and one state here.
    /// </para>
    /// Routing the blank case through the generic unknown-family rule instead sets every such
    /// document in DejaVu Sans and reflows all of them — and routing it through the word-processing
    /// roman default instead moved <b>29 corpus <c>.doc</c> documents</b> off Liberation Serif and
    /// lost 17 verdicts before <c>WordFallbackClass</c> was guarded on the name being non-empty.
    /// </remarks>
    private static IReadOnlyList<string> DefaultFallbacks => FontSubstitutions.DefaultLatinTextChain;

    /// <inheritdoc/>
    public FontReference Resolve(FontRequest request)
    {
        // The document's own embedded font wins over anything installed: it is what the author saw,
        // and it is the only face guaranteed to have the metrics the document was laid out against.
        if (request.EmbeddedFaceKey is { Length: > 0 } embedded)
        {
            return new FontReference
            {
                FamilyName = request.FamilyName,
                RequestedFamily = request.FamilyName,
                Weight = request.Weight,
                IsItalic = request.IsItalic,
                FaceKey = embedded,
            };
        }

        // The requested family, if it is here.
        if (_index.Best(request.FamilyName, request.Weight, request.IsItalic) is { } exact)
            return Reference(request, exact, requested: request.FamilyName);

        // A family fontconfig files under `math` is answered by the orthography, not by a shape,
        // and that beats the document's own declaration. See MathFace.
        if (MathFace(request) is { } maths)
        {
            Record(request, maths);
            return Reference(request, maths, requested: request.FamilyName);
        }

        // What the *document* declared about the family, when it declared something acted on.
        //
        // This runs **before** the substitution chain, and that ordering is the whole point rather than
        // a detail. `FontConfigManager::Substitute` (`vcl/unx/generic/font/fontconfig.cxx`) is the
        // *pre-match* substitution: it runs before LibreOffice consults `VCL.xcu` at all, and it asks
        // fontconfig for the requested name plus a generic family. A name fontconfig aliases only
        // weakly therefore loses to the generic and never reaches the chain. Measured against 26.2.4.2
        // with authored one-paragraph documents on the four names that can tell the two orderings
        // apart, because each has a chain entry that *is* installed: `Times`, `Helvetica`, `Albany`
        // and `Thorndale` all answer DejaVu once a class is declared, where the chain answers
        // Liberation.
        string[]? generic = DeclaredGenericFor(request);

        if (generic is not null)
        {
            // Except a *strong* metric alias, which fontconfig's own `30-metric-aliases.conf` binds hard
            // enough to beat a generic family. The test is whether an installed face declares itself the
            // equivalent of the very name asked for — Liberation Sans of Arial, Carlito of Calibri — and
            // not whether the two are transitively compatible: Liberation Sans is metrically Helvetica's
            // equal through Arial, and `Helvetica` declared swiss still renders in DejaVu Sans.
            foreach (string candidate in FontSubstitutions.ChainFor(request.FamilyName))
            {
                if (!ClaimsEquivalenceWith(candidate, request.FamilyName)) continue;
                if (_index.Best(candidate, request.Weight, request.IsItalic) is not { } aliased) continue;

                Record(request, aliased);
                return Reference(request, aliased, requested: request.FamilyName);
            }

            foreach (string candidate in generic)
            {
                if (_index.Best(candidate, request.Weight, request.IsItalic) is not { } preferred)
                    continue;

                Record(request, preferred);
                return Reference(request, preferred, requested: request.FamilyName);
            }
        }

        // LibreOffice's substitution chain, in its own order — when the running binary reaches it
        // at all. See `ConsultsTheChain`.
        if (ConsultsTheChain(request))
        {
            foreach (string candidate in FontSubstitutions.ChainFor(request.FamilyName))
            {
                if (_index.Best(candidate, request.Weight, request.IsItalic) is not { } substitute)
                    continue;

                Record(request, substitute);
                return Reference(request, substitute, requested: request.FamilyName);
            }
        }

        // Nothing named matched, so fall back by shape. A monospaced request must not land on a
        // proportional face: the document is relying on the columns lining up.
        IReadOnlyList<string> fallbacks = GenericFallbacks(request);

        foreach (string candidate in fallbacks)
        {
            if (_index.Best(candidate, request.Weight, request.IsItalic) is not { } fallback) continue;

            Record(request, fallback);
            return Reference(request, fallback, requested: request.FamilyName);
        }

        // Last resort: whatever is installed. A document still has to render, and a caller comparing
        // against a reference has the substitution list to explain why it does not match.
        InstalledFace? any = _index.Faces
            .OrderBy(f => f.IsItalic == request.IsItalic ? 0 : 1)
            .ThenBy(f => Math.Abs(f.Weight - request.Weight))
            .ThenBy(f => f.FamilyName, StringComparer.Ordinal)
            .Cast<InstalledFace?>()
            .FirstOrDefault();

        if (any is not { } last)
        {
            // No fonts at all. Report the request unchanged rather than inventing a name: a caller
            // that cannot load the face will find out, and a made-up family would hide why.
            return new FontReference
            {
                FamilyName = request.FamilyName,
                RequestedFamily = request.FamilyName,
                Weight = request.Weight,
                IsItalic = request.IsItalic,
                FaceKey = string.Empty,
            };
        }

        Record(request, last);
        return Reference(request, last, requested: request.FamilyName);
    }

    /// <inheritdoc/>
    public IFontFace LoadFace(FontReference reference)
    {
        ArgumentNullException.ThrowIfNull(reference);

        if (!_loaded.TryGetValue(reference.FaceKey, out OpenTypeFace? face))
        {
            (string path, int index) = SplitKey(reference.FaceKey);
            face = path.Length > 0 ? OpenTypeFace.ReadFile(path, index) : null;

            if (face is null)
            {
                throw new Core.MalformedDocumentException(
                    $"The face '{reference.FaceKey}' could not be read as a font.");
            }

            _loaded[reference.FaceKey] = face;
        }

        // Also on the hit, and for the same reason `LoadCached` does it: a face reached only through
        // this path would otherwise have no key, and `GenericForFallback` and `ReferenceFor` both
        // ask the face for one.
        _keys[face] = reference.FaceKey;

        return new ResolvedFontFace(reference, face);
    }

    /// <summary>
    /// Loads a reference as the concrete OpenType face, which is what layout needs.
    /// </summary>
    /// <remarks>
    /// <see cref="LoadFace"/> answers the <see cref="IFontFace"/> a caller supplying its own embedded
    /// faces can implement, and that interface deliberately exposes only coverage and vertical metrics.
    /// Measuring and shaping need the tables themselves, so layout asks for this instead — and gets the
    /// same cached instance, since the bytes belong to the resolver either way.
    /// </remarks>
    public OpenTypeFace LoadOpenType(FontReference reference)
    {
        ArgumentNullException.ThrowIfNull(reference);
        return ((ResolvedFontFace)LoadFace(reference)).OpenType;
    }

    private static (string Path, int Index) SplitKey(string faceKey)
    {
        int hash = faceKey.LastIndexOf('#');
        return hash < 0 || !int.TryParse(faceKey[(hash + 1)..], out int index)
            ? (faceKey, 0)
            : (faceKey[..hash], index);
    }

    /// <remarks>
    /// <c>SyntheticOblique</c> is the whole of <c>LogicalFontInstance::NeedsArtificialItalic()</c>
    /// — <em>italic was asked for and the face that answered has none</em> — and it belongs here
    /// rather than at a call site because this is the one place that holds both halves at once.
    /// Every other constructor of a <see cref="FontReference"/> in this file either has no request
    /// to compare against (<see cref="ReferenceFor"/>, a reverse lookup from a face) or is
    /// asserting the request onto the answer (the embedded-face arm, where the document supplied
    /// the face and its own declaration is all there is), and neither can decide this.
    /// </remarks>
    private FontReference Reference(FontRequest request, InstalledFace face, string requested)
    {
        // Which of fontconfig's generic preference lists a *glyph fallback* out of this face will be
        // ranked by. It is a property of the request rather than of the face — the same DejaVu Serif
        // answers a roman request and an undeclared one — so it is recorded here, where both halves
        // are in hand, and read back through the face in `GenericForFallback`.
        //
        // **The first request to reach a face keeps it, and that is a stability requirement rather
        // than a preference.** Measurement and drawing itemise the same paragraph separately and
        // must choose the same fallback face for it, or a line is measured at one width and painted
        // at another; an entry that could change between the two passes would do exactly that. So a
        // face two differently-declared families both resolve to answers for the first of them,
        // which in a document is its body face.
        _genericByFaceKey.TryAdd(face.FaceKey, GenericFor(request));

        // And the language, for the same reason and under the same first-writer rule: the two are
        // the two halves of one pattern, and fontconfig ranks the language above the generic.
        if (request.Language is { Length: > 0 } language) _languageByFaceKey.TryAdd(face.FaceKey, language);

        return new FontReference
        {
            FamilyName = face.FamilyName,
            RequestedFamily = requested,
            Weight = face.Weight,
            IsItalic = face.IsItalic,
            SyntheticOblique = request.IsItalic && !face.IsItalic,
            FaceKey = face.FaceKey,
        };
    }

    /// <summary>
    /// The generic family <c>FontConfigManager::Substitute</c> would put in this request's pattern.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>serif</c> for <c>FAMILY_ROMAN</c> and <c>sans</c> for <c>FAMILY_SWISS</c>
    /// (<c>vcl/unx/generic/font/fontconfig.cxx</c>:1075-1088) and <strong>nothing for the
    /// rest</strong> — and where nothing is appended, the family's own filing or
    /// <c>49-sansserif.conf</c> supplies one, which is what
    /// <see cref="FontconfigPreferences.GenericNameOf"/> answers.
    /// </para>
    /// <para>
    /// <strong>Pitch is deliberately not read here, unlike in <see cref="DeclaredGenericFor"/>.</strong>
    /// <c>addtopattern</c> does put the pitch in the pattern as <c>FC_SPACING</c>, but
    /// <c>PRI_SPACING</c> sits <em>below</em> <c>PRI_FAMILY_WEAK</c> in <c>fcmatch.c</c> — so it
    /// breaks ties between faces the family list has already ranked equal and never chooses which
    /// list ranks them. Substitution is the other way round, because a request marked fixed is
    /// relying on its columns; that is a different question and it has its own helper.
    /// </para>
    /// </remarks>
    private string GenericFor(FontRequest request)
        => request.DeclaredClass switch
        {
            FontFamilyClass.Serif => "serif",
            FontFamilyClass.SansSerif => DefaultGeneric,
            _ => (_preferences.IsConfigured ? _preferences.GenericNameOf(request.FamilyName) : null)
                 ?? DefaultGeneric,
        };

    private void Record(FontRequest request, InstalledFace chosen)
        => _substitutions.Add(new FontSubstitution(
            request.FamilyName,
            chosen.FamilyName,
            FontSubstitutions.AreMetricCompatible(request.FamilyName, chosen.FamilyName)));

    /// <summary>
    /// The shape to fall back to once nothing the chain named turned out to be installed.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice's own <c>FontType</c> decides it, which is the point: the configuration that says
    /// what to substitute also says what shape the family is, and reading the second half means the
    /// answer is data rather than a guess. A document's declared pitch still wins, because a request
    /// marked fixed is relying on its columns whatever the family is called.
    /// </para>
    /// <para>
    /// Where the table has never heard of the family the answer is sans-serif, and that is not a
    /// coin toss — but it is only the answer for the callers that arrive here <em>without</em> a
    /// declared class, which is ODF text, XLSX, PPTX and flat ODS. Measured against
    /// [24.2.7-audit: VERIFIED 2026-08-21, round words-r54 — correct for every format that reaches
    /// it undeclared. Round 53's WRONG verdict was right about the DOCX answer and wrong about the
    /// seat: that is the word-processing filter's roman default, and it lives in
    /// XRay.Office.WordProcessing.WordFallbackClass. The stated fontconfig *reason* below is still
    /// falsified and is corrected in the paragraph after it.]
    /// [24.2.7-audit: VERIFIED 2026-08-21, round words-r55 — re-confirmed from a *fifth* caller,
    /// which is the useful part: the DOC filter reaches here undeclared after all. Its FFN sets
    /// FAMILY_DONTKNOW on the item for ff 0, 6 and 7, and nine flat-ODF fixtures exported to Word 97
    /// and back (probes/words-r55/doc-family-code.py) answer DejaVu Sans, and DejaVu Sans *Mono* for
    /// Consolas — this switch's own column. The paragraph below, which says the word-processing
    /// filters never arrive here undeclared, was true of what round 54 could measure and is not true
    /// of DOC.]
    /// LibreOffice 26.2.4.2 here, an unrecognised family reaching this path undeclared resolves to
    /// DejaVu Sans through every one of those filters — <c>Aptos</c>, <c>Candara</c> and
    /// <c>Consolas</c> in authored PPTX, XLSX and flat ODS files answer DejaVu Sans, DejaVu Sans and
    /// DejaVu Sans <em>Mono</em>, tracking fontconfig's own filing exactly
    /// (<c>probes/words-r54/cross-format-fallback.py</c>). Over all 302 slides and 307 sheets
    /// renderings compared against the reference's own embedded font lists, <b>zero</b> documents
    /// show ours DejaVu Sans against the reference's DejaVu Serif.
    /// </para>
    /// <para>
    /// <strong>The DOCX and RTF filters do not reach this undeclared, and round 53 caught that
    /// as a defect here.</strong> They default an unrecognised family's class to roman before the
    /// request is ever built — DOCX because the class is inherited and its floor is Writer's roman
    /// pool default, RTF because its filter never sets a family and that same pool default stands —
    /// so they arrive with <c>DeclaredClass = Serif</c> and are
    /// answered by <see cref="DeclaredGenericFor"/> above rather than by this switch.
    /// <b>DOC is the exception and round 55 measured it</b>: the WW8 reader writes
    /// <c>FAMILY_DONTKNOW</c> onto the item for an <c>ff</c> of 0, 6 or 7, so those runs do arrive
    /// here undeclared and are answered by this switch, correctly. That was
    /// worth <b>32 of 337 words renderings</b> drawn in DejaVu Sans where the reference has DejaVu
    /// Serif. It is fixed in the reader, in
    /// <c>XRay.Office.WordProcessing.Layout.WordFallbackClass</c>, because the difference is a
    /// property of the *filter*: putting it here would have set every slide and every sheet in a
    /// serif face that 26.2.4.2 sets in a grotesque.
    /// </para>
    /// <para>
    /// <b>The reason originally given for this branch is falsified independently of the answer.</b>
    /// <c>fc-match Aptos</c> on this machine returns <c>DejaVuSans.ttf</c>, as does
    /// <c>fc-match ""</c> — but so it does for names fontconfig files under <c>monospace</c>, and
    /// those answer DejaVu Sans Mono here. The branch is right because <c>49-sansserif.conf</c>
    /// appends <c>sans-serif</c> to any pattern that has not already named a generic, which is a
    /// statement about the *configuration* rather than about a "default family"; see
    /// <see cref="FontconfigPreferences.GenericClassOf"/>, which is what actually answers.
    /// </para>
    /// </remarks>
    private IReadOnlyList<string> GenericFallbacks(FontRequest request)
    {
        if (request.Pitch == FontPitch.Fixed) return MonoFallbacks;

        // "No font named" and "a font nobody has" are different questions with different answers,
        // and only the second one is fontconfig's to answer.
        if (string.IsNullOrWhiteSpace(request.FamilyName)) return DefaultFallbacks;

        // What the *document* says the family is, which beats what anything says the *name* is.
        // The same helper the pre-match step above uses, so the two cannot drift apart — reaching
        // here at all means the declared generic named nothing installed either.
        if (DeclaredGenericFor(request) is { } declared) return declared;

        return ShapeOf(request.FamilyName) switch
        {
            FontFamilyClass.Fixed => MonoFallbacks,
            FontFamilyClass.Serif => SerifFallbacks,

            // A symbol face has no shape-compatible stand-in among the text faces, so there is
            // nothing better to do than treat it as text and let glyph fallback place what it can.
            _ => SansFallbacks,
        };
    }

    /// <summary>
    /// The generic shape a family name implies, read from fontconfig rather than from
    /// <c>VCL.xcu</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This is the substitution the running binary makes, and the table's <c>FontType</c>
    /// is not.</strong> By the time control reaches here the family is not installed and no chain
    /// entry has answered, which is the case <c>FcPreMatchSubstitution::FindFontSubstitute</c>
    /// (<c>vcl/unx/generic/font/fontsubst.cxx</c>:98) handles by handing the name to fontconfig —
    /// so the generic that decides the face is the one *fontconfig* files the name under, not the
    /// one LibreOffice's own configuration does. The two disagree, and not rarely: over the 296
    /// families the sample corpus names, ten differ. <c>Century Schoolbook</c>, <c>Century</c>,
    /// <c>NewCenturySchlbk</c>, <c>Book Antiqua</c>, <c>Bookman Old Style</c>, <c>CG Times</c> and
    /// <c>Times-Roman</c> are romans to <c>VCL.xcu</c> and are filed under nothing at all by
    /// fontconfig, so they take its sans-serif default; <c>Lucida Console</c> is fixed to
    /// <c>VCL.xcu</c> and likewise unfiled; <c>Palatino Linotype</c>, <c>SimSun</c> and
    /// <c>ＭＳ 明朝</c> go the other way, unknown to the table and filed <c>serif</c> by
    /// fontconfig. Measured on the installed 26.2.4.2 with the 296-row face probe: the binary
    /// answers fontconfig's way on every one.
    /// </para>
    /// <para>
    /// <strong>Through the filters that reach here undeclared, which is not all of them.</strong>
    /// Round 54 measured the same question through six filters and found two answers: ODF text,
    /// XLSX, PPTX and flat ODS track fontconfig exactly as this paragraph says, while DOCX, DOC and
    /// RTF default the class to roman first and so never consult this at all — <c>Consolas</c>,
    /// which fontconfig files under <c>monospace</c>, comes out DejaVu Sans Mono through the ODF
    /// filter and DejaVu <em>Serif</em> through the DOCX one, from the same binary and the same
    /// fontconfig. So "the binary answers fontconfig's way" is a claim about the *caller* as much as
    /// about the name. See <c>XRay.Office.WordProcessing.Layout.WordFallbackClass</c> and
    /// <c>probes/words-r54/font-fallback-rule.py</c>.
    /// </para>
    /// <para>
    /// The table is still the answer where there is no fontconfig to ask — Windows, most macOS —
    /// because there the pre-match hook does not exist either and <c>ImplFontSubstitute</c> really
    /// is what runs.
    /// </para>
    /// </remarks>
    private FontFamilyClass ShapeOf(string? familyName)
        => _preferences.IsConfigured
            ? _preferences.GenericClassOf(familyName)
            : FontSubstitutions.ClassOf(familyName);

    /// <summary>
    /// Whether LibreOffice's own <c>SubstFonts</c> chain is reached for this request at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>PhysicalFontCollection::FindFontFamily</c> calls the pre-match hook
    /// (<c>vcl/source/font/PhysicalFontCollection.cxx</c>:1142) and returns whatever it names if
    /// that family is installed (<c>:1151</c>); <c>ImplFontSubstitute</c>, which is the chain, is
    /// only reached in the *second* loop at <c>:1180</c>. On Linux the hook asks fontconfig, and
    /// fontconfig always answers. So the chain is unreachable for a family fontconfig speaks for —
    /// which, once the generic default is counted, is every family.
    /// </para>
    /// <para>
    /// <strong>Only it demonstrably is not, for the eight names the chain currently gets right.</strong>
    /// Over the 296 corpus families, exactly 18 are answered by the chain rather than by a shape
    /// fallback, and the split is clean: eight are families fontconfig itself names — <c>Arial</c>,
    /// <c>Calibri</c>, <c>Cambria</c>, <c>Courier</c>, <c>Courier New</c>, <c>Helvetica</c>,
    /// <c>Times</c>, <c>Times New Roman</c> — where our chain and fontconfig's own aliases reach the
    /// same installed face and the chain is a faithful stand-in for the alias expansion this
    /// resolver does not implement. Six are pi faces, where the hook bails before fontconfig is
    /// asked at all (<c>fontsubst.cxx</c>:101, on a symbol-encoded request). The remaining four are
    /// the defect: <c>CG Times</c>, <c>Times-Roman</c>, <c>MS Gothic</c> and <c>MS PGothic</c>, all
    /// four unnamed by fontconfig, all four sent by the chain to a face 26.2.4.2 does not use.
    /// </para>
    /// <para>
    /// So the rule is <em>a family fontconfig names nowhere does not reach the chain</em>, which is
    /// the mechanism restated in terms of what is checkable. It replaces a hardcoded pair —
    /// <c>Helv</c> and <c>SansSerif</c>, found by the same probe a round earlier — and derives both
    /// of them, neither being named by any <c>&lt;alias&gt;</c> in a stock configuration.
    /// </para>
    /// <para>
    /// <c>MS Gothic</c> is worth naming because the obvious objection is that fontconfig answers by
    /// character and this request carries none, so an East Asian family might deserve its chain
    /// entry. It does not: 26.2.4.2 draws Latin text declared <c>MS Gothic</c> in DejaVu Sans and
    /// <em>Japanese</em> text declared <c>MS Gothic</c> in WenQuanYi Zen Hei, and the chain's answer
    /// — IPAGothic — is neither. The CJK coverage comes back through glyph fallback, which reads
    /// the same configuration and ranks WenQuanYi Zen Hei first.
    /// </para>
    /// </remarks>
    private bool ConsultsTheChain(FontRequest request)
    {
        // A machine with no fontconfig has no pre-match hook either, so nothing overrides the table.
        if (!_preferences.IsConfigured) return true;

        // A pi face is symbol-encoded, and the hook bails on those before fontconfig is consulted.
        if (FontSubstitutions.ClassOf(request.FamilyName) == FontFamilyClass.Symbol) return true;

        return _preferences.Names(request.FamilyName);
    }

    /// <summary>
    /// The faces the generic family a document's own declaration implies, or null when it implies none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two classes and only two, because that is what LibreOffice sends: <c>FontConfigManager::Substitute</c>
    /// (<c>vcl/unx/generic/font/fontconfig.cxx</c>) appends <c>"serif"</c> as a second <c>FC_FAMILY</c>
    /// for <c>FAMILY_ROMAN</c> and <c>"sans"</c> for <c>FAMILY_SWISS</c>, and nothing at all for any
    /// other family type. A monospaced class looks as though it ought to add <c>"monospace"</c> and
    /// does not — measured, <c>Times</c> declared <c>modern</c> still comes out Liberation Serif, which
    /// is the plain <c>fc-match Times</c> answer. The readers collapse those codes to
    /// <see cref="FontFamilyClass.Unknown"/> for the same reason.
    /// </para>
    /// <para>
    /// <strong>A declared fixed pitch wins over a declared family</strong>, because a document relying
    /// on its columns lining up is making the stronger statement: measured, <c>Garamond</c> declared
    /// roman <em>and</em> fixed answers DejaVu Sans Mono rather than DejaVu Serif.
    /// </para>
    /// <para>
    /// <strong>A pi face is exempt</strong>, whatever class the document put beside it. Every Word
    /// document that uses <c>Symbol</c> declares it roman and there is no roman equivalent of a font of
    /// arrows and Greek letters; fontconfig knows it too and binds the name hard enough to survive a
    /// generic — <c>fc-match "Symbol,serif"</c> and <c>fc-match Symbol</c> both answer OpenSymbol, and
    /// so does 26.2.4.2 on a document declaring Symbol as a roman. Without this the declaration sent
    /// every Symbol run to DejaVu Serif, which draws the characters rather than the symbols they stand
    /// for — measured on <c>ABCD-FE-01-00 Flight Envelope.docx</c> and its sibling.
    /// </para>
    /// <para>
    /// The lists themselves are the existing shape fallbacks, already documented as the answer
    /// fontconfig's generic families give on this configuration and measured face by face.
    /// </para>
    /// </remarks>
    private static string[]? DeclaredGenericFor(FontRequest request)
    {
        if (request.Pitch == FontPitch.Fixed) return MonoFallbacks;

        if (FontSubstitutions.ClassOf(request.FamilyName) == FontFamilyClass.Symbol) return null;

        return request.DeclaredClass switch
        {
            FontFamilyClass.Serif => SerifFallbacks,
            FontFamilyClass.SansSerif => SansFallbacks,
            _ => null,
        };
    }

    /// <summary>
    /// The face a family fontconfig files under <c>math</c> resolves to, or null when it files it
    /// under something else.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The maths generic is answered by a language, not by a shape, and that is why it
    /// cannot go through <see cref="DeclaredGenericFor"/>.</strong> <c>45-generic.conf</c> files
    /// <c>Cambria Math</c> and six siblings under <c>math</c> and then prepends
    /// <c>lang=und-zmth</c> to any pattern whose family is <c>math</c>. fontconfig ranks a
    /// language requirement above weight and slant and above the generic a caller appended, so the
    /// answer is the same whatever the document declared and whatever weight it asked for:
    /// <c>fc-match "Cambria Math"</c>, <c>"Cambria Math,serif"</c>, <c>"Cambria Math,sans"</c> and
    /// <c>"Cambria Math:bold"</c> all answer FreeSerif <em>Regular</em> here.
    /// </para>
    /// <para>
    /// Three tiers, in fontconfig's own order. <see cref="MathFallbacks"/> is
    /// <c>60-generic.conf</c>'s <c>math</c> preference list verbatim, taken at the requested weight
    /// because a real maths family has faces to choose between; then
    /// <see cref="MathOrthographyFace"/>, pinned to regular because only its regular file declares
    /// the orthography; then nothing, so a machine with neither falls through to the ordinary path
    /// and lands on the configuration's overall default, which is what
    /// <c>Cambria Math draws in DejaVu Sans</c> was measured on.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2, faces read out of the PDFs: the four corpus documents naming
    /// <c>Cambria Math</c> — <c>DRX-Ascend System Course Description.docx</c>,
    /// <c>075_Storyboard_Template_Fillable_Format…docx</c>,
    /// <c>084_Printable_Graph_Paper_Template…docx</c> and
    /// <c>AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc</c> — are drawn in FreeSerif by the
    /// reference and were drawn in DejaVu Serif here.
    /// </para>
    /// </remarks>
    private InstalledFace? MathFace(FontRequest request)
    {
        if (_preferences.GenericNameOf(request.FamilyName) != "math") return null;

        foreach (string candidate in MathFallbacks)
        {
            if (_index.Best(candidate, request.Weight, request.IsItalic) is { } maths) return maths;
        }

        return _index.Best(MathOrthographyFace, weight: 400, italic: false);
    }

    /// <summary>
    /// True when an installed family declares itself the metric equivalent of the name asked for.
    /// </summary>
    /// <remarks>
    /// Deliberately one-directional and deliberately not <see cref="FontSubstitutions.AreMetricCompatible"/>,
    /// which is transitive: Liberation Sans and Helvetica are compatible through Arial, and a document
    /// naming Helvetica as a grotesque still renders in DejaVu Sans. What survives a generic family in
    /// fontconfig is the alias bound to the requested name itself, so that is what is tested.
    /// </remarks>
    private static bool ClaimsEquivalenceWith(string candidate, string requested)
        => FontSubstitutions.MicrosoftEquivalentOf(candidate) is { } equivalent
           && FontSubstitutions.Normalise(equivalent) == FontSubstitutions.Normalise(requested);

    /// <summary>A face loaded through a resolver.</summary>
    private sealed class ResolvedFontFace(FontReference reference, OpenTypeFace face) : IFontFace
    {
        /// <inheritdoc/>
        public FontReference Reference { get; } = reference;

        /// <inheritdoc/>
        public int UnitsPerEm => face.UnitsPerEm;

        /// <inheritdoc/>
        public FontVerticalMetrics VerticalMetrics =>
            LineSpacing.ResolveDecorations(face, LineSpacing.Resolve(face));

        /// <summary>The underlying face, for callers needing advances or the line metrics.</summary>
        public OpenTypeFace OpenType => face;

        /// <inheritdoc/>
        public bool HasGlyphFor(int codePoint) => face.HasGlyphFor(codePoint);

        /// <inheritdoc/>
        /// <remarks>
        /// Nothing to release: the face's bytes are cached by the resolver that loaded it, so
        /// disposing one view of it must not invalidate the others.
        /// </remarks>
        public void Dispose() { }
    }
}

/// <summary>One substitution a resolver made.</summary>
/// <param name="Requested">The family the document asked for.</param>
/// <param name="Chosen">The family that was used instead.</param>
/// <param name="IsMetricCompatible">
/// True when the substitute has the original's advance widths, so every line breaks where it did.
/// This is the difference between a page that looks slightly different and a document whose every
/// later page is wrong.
/// </param>
public readonly record struct FontSubstitution(
    string Requested,
    string Chosen,
    bool IsMetricCompatible);

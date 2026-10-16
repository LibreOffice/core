using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// The order glyph fallback asks its two sources in, and which of fontconfig's preference lists
/// ranks the answer.
/// </summary>
/// <remarks>
/// <para>
/// <c>PhysicalFontCollection::GetGlyphFallbackFont</c> calls the fontconfig hook first and reaches
/// <c>ImplInitGenericGlyphFallback</c>'s fixed list only <c>if (!pFallbackData)</c>
/// (<c>vcl/source/font/PhysicalFontCollection.cxx</c>:231-291). This used to ask them the other way
/// about, and because the fixed list heads with <c>starsymbol, opensymbol</c> every character
/// OpenSymbol covers was drawn from OpenSymbol — a face on no fontconfig preference list, so one the
/// reference never answers a glyph fallback with.
/// </para>
/// <para>
/// The faces are the machine's own, because the question is which of several <em>installed</em>
/// faces wins and a fabricated font set cannot pose it. The configuration is not: each test builds
/// the fontconfig tree it needs, so it says the same thing on a machine whose <c>/etc/fonts</c>
/// differs.
/// </para>
/// </remarks>
public class GlyphFallbackOrderTests
{
    /// <summary>U+2022 BULLET: OpenSymbol has it, and so does every text face on the machine.</summary>
    private const int Bullet = 0x2022;

    /// <summary>
    /// U+2011 NON-BREAKING HYPHEN: DejaVu Sans and DejaVu Serif have it, Carlito and Liberation do
    /// not — so it separates the two generics without OpenSymbol having a say.
    /// </summary>
    private const int NonBreakingHyphen = 0x2011;

    /// <summary>U+2714 HEAVY CHECK MARK: <c>Emoji=Yes</c>, and FreeSerif holds it too.</summary>
    private const int HeavyCheckMark = 0x2714;

    private static SystemFontIndex Installed() => SystemFontIndex.Build(["/usr/share/fonts"]);

    private static SystemFontResolver Resolver(SystemFontIndex index, params string[] files)
        => new(index, FontconfigPreferences.Read(files));

    /// <summary>The face a request resolves to, which is what a run would be set in.</summary>
    private static OpenTypeFace Primary(
        SystemFontResolver resolver, string family, FontFamilyClass declared)
        => resolver.LoadOpenType(
            resolver.Resolve(new FontRequest { FamilyName = family, DeclaredClass = declared }));

    [Fact]
    public void FontconfigIsAskedBeforeLibreOfficesOwnList()
    {
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("OpenSymbol") || !index.Has("DejaVu Sans"),
            "the faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-latin.conf", Alias("sans-serif", "DejaVu Sans"));

        // Both faces hold U+2022 and OpenSymbol heads the fixed list, so the source that is asked
        // first is the whole of the difference.
        Resolver(index, tree.Root).FallbackFor(Bullet)?.FamilyName.ShouldBe("DejaVu Sans");
    }

    [Fact]
    public void WithNoFontconfigTheListIsAskedFirst()
    {
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("OpenSymbol") || !index.Has("DejaVu Sans"),
            "the faces this compares are not installed; see check-env.sh");

        // Windows and most macOS installations. There is no preference order to ask, so the fixed
        // list is the only source of truth and must still come first.
        new SystemFontResolver(index, FontconfigPreferences.None)
            .FallbackFor(Bullet)?.FamilyName.ShouldBe("OpenSymbol");
    }

    [Fact]
    public void APiFaceIsStillAnsweredByTheListAlone()
    {
        // The rule the previous round established: `FcGlyphFallbackSubstitution::FindFontSubstitute`
        // returns false for OpenSymbol and for a Microsoft-symbol-encoded pattern, so the hook never
        // runs and the fixed list is the whole answer. Reversing the two stages must not reach it.
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("OpenSymbol") || !index.Has("DejaVu Sans"),
            "the faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-latin.conf", Alias("sans-serif", "DejaVu Sans"));

        Resolver(index, tree.Root).SymbolFallbackFor(Bullet)?.FamilyName.ShouldBe("OpenSymbol");
    }

    [Fact]
    public void TheRequestsOwnGenericDecidesWhichPreferenceListRanksTheAnswer()
    {
        // 26.2.4.2, measured over six declared classes in `probes/fonts-r64/gen-generic.py`: the
        // same run declared roman draws U+2011 in DejaVu Serif and declared swiss draws it in
        // DejaVu Sans. `FontConfigManager::Substitute` appends `serif` for FAMILY_ROMAN and `sans`
        // for FAMILY_SWISS, and only that generic's <prefer> list enters the pattern.
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("DejaVu Serif") || !index.Has("DejaVu Sans") || !index.Has("Carlito"),
            "the faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-latin.conf", Alias("serif", "DejaVu Serif"));
        tree.Write("conf.d/61-latin.conf", Alias("sans-serif", "DejaVu Sans"));

        // One resolver each, because one resolver is one document: a face records the generic of
        // the *first* request that reached it, so that measurement and drawing cannot disagree
        // about the same paragraph. See `SystemFontResolver.Reference`.
        SystemFontResolver asRoman = Resolver(index, tree.Root);
        SystemFontResolver asSwiss = Resolver(index, tree.Root);

        asRoman.FallbackFor(
                NonBreakingHyphen, 400, false, Primary(asRoman, "Carlito", FontFamilyClass.Serif))
            ?.FamilyName.ShouldBe("DejaVu Serif");
        asSwiss.FallbackFor(
                NonBreakingHyphen, 400, false, Primary(asSwiss, "Carlito", FontFamilyClass.SansSerif))
            ?.FamilyName.ShouldBe("DejaVu Sans");
    }

    [Fact]
    public void AFaceKeepsTheGenericOfTheFirstRequestThatReachedIt()
    {
        // Not a preference: measurement and drawing itemise a paragraph separately, so an entry
        // that changed when a later family resolved to the same face would have them choose two
        // different fallback faces for one line.
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("DejaVu Serif") || !index.Has("DejaVu Sans") || !index.Has("Carlito"),
            "the faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-latin.conf", Alias("serif", "DejaVu Serif"));
        tree.Write("conf.d/61-latin.conf", Alias("sans-serif", "DejaVu Sans"));

        SystemFontResolver resolver = Resolver(index, tree.Root);
        OpenTypeFace roman = Primary(resolver, "Carlito", FontFamilyClass.Serif);
        Primary(resolver, "Carlito", FontFamilyClass.SansSerif);

        resolver.FallbackFor(NonBreakingHyphen, 400, false, roman)
            ?.FamilyName.ShouldBe("DejaVu Serif");
    }

    [Fact]
    public void AFaceThisResolverNeverChoseTakesTheGenericItsOwnFamilyIsFiledUnder()
    {
        // An embedded face, or one loaded by key: there is no request to read the generic off, and
        // the answer is then the one the pattern would have carried had the document declared
        // nothing — which is the family's own filing.
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("DejaVu Serif") || !index.Has("DejaVu Sans") || !index.Has("Carlito"),
            "the faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/45-latin.conf", Conf(Default("Carlito", "serif")));
        tree.Write("conf.d/60-latin.conf", Alias("serif", "DejaVu Serif"));
        tree.Write("conf.d/61-latin.conf", Alias("sans-serif", "DejaVu Sans"));

        SystemFontResolver resolver = Resolver(index, tree.Root);
        if (index.Best("Carlito", 400, false) is not { } installed) throw new InvalidOperationException("Carlito");
        OpenTypeFace? carlito = OpenTypeFace.ReadFile(installed.Path);

        resolver.FallbackFor(NonBreakingHyphen, 400, false, carlito)
            ?.FamilyName.ShouldBe("DejaVu Serif");
    }

    [Fact]
    public void AnEmojiCodePointGoesToTheEmojiListWhateverGenericTheRequestNamed()
    {
        // `getExemplarLangTagForCodePoint` answers und-zsye for a character with the Emoji property,
        // and fontconfig scores PRI_LANG above PRI_FAMILY_WEAK — so the emoji face wins over the
        // generic's own list. Measured on 26.2.4.2: U+2714 answers Noto Color Emoji under all six
        // declared classes, though FreeSerif holds it and is on the serif list.
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("Noto Color Emoji") || !index.Has("FreeSerif"),
            "the faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-generic.conf", Alias("emoji", "Noto Color Emoji"));
        tree.Write("conf.d/69-unifont.conf", Alias("serif", "FreeSerif"));

        Resolver(index, tree.Root)
            .FallbackFor(HeavyCheckMark)?.FamilyName.ShouldBe("Noto Color Emoji");
    }

    [Fact]
    public void WithNoEmojiListTheGenericsOwnListAnswersTheSameCharacter()
    {
        // The control on the test above: it is the emoji list that moves the answer, not coverage.
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("Noto Color Emoji") || !index.Has("FreeSerif"),
            "the faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/69-unifont.conf", Alias("serif", "FreeSerif"));

        SystemFontResolver resolver = Resolver(index, tree.Root);
        OpenTypeFace roman = Primary(resolver, "Carlito", FontFamilyClass.Serif);

        resolver.FallbackFor(HeavyCheckMark, 400, false, roman)
            ?.FamilyName.ShouldBe("FreeSerif");
    }

    [Fact]
    public void TheItemsLanguageOutranksTheGenericsPreferenceList()
    {
        // `FontConfigManager::Substitute` adds the item's language to the pattern as FC_LANG
        // (`vcl/unx/generic/font/fontconfig.cxx`:1092, 1118-1119) and `fcmatch.c` scores PRI_LANG
        // above PRI_FAMILY_WEAK, so among the faces covering the character the ones supporting the
        // language come first. Measured on 26.2.4.2: a complex-script run draws U+05D0 in FreeSans,
        // because Writer's default CTL language is Hindi and DejaVu Sans has no Devanagari.
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("DejaVu Sans") || !index.Has("FreeSans"),
            "the faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-latin.conf", Alias("sans-serif", "DejaVu Sans", "FreeSans"));

        SystemFontResolver resolver = Resolver(index, tree.Root);

        // Both cover U+2610 and DejaVu Sans is first on the only list in force, so the list alone
        // answers DejaVu Sans.
        resolver
            .FallbackFor(
                [Ballot], 400, false, primary: null,
                new FontItem("Calibri", FontFamilyClass.Unknown, "en-US"))
            ?.FamilyName.ShouldBe("DejaVu Sans");

        // The same request under the complex-script item's own language answers the face that has
        // Devanagari, although it is second on the list.
        resolver
            .FallbackFor(
                [Ballot], 400, false, primary: null,
                new FontItem("Calibri", FontFamilyClass.Unknown, "hi-IN"))
            ?.FamilyName.ShouldBe("FreeSans");
    }

    [Fact]
    public void TheItemTravelsWithTheRunRatherThanWithTheFaceItChose()
    {
        // The generic used to be recorded against the face the request resolved to, first writer
        // winning. In a word-processing document the first request to reach a face is the paragraph
        // mark's, so a run on a different font item silently took the paragraph's — which is why
        // every cell of `probes/fonts-r65/gen-scriptitem.py` answered as though the item did not
        // exist until the item was passed in.
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("Carlito") || !index.Has("DejaVu Sans") || !index.Has("FreeSerif"),
            "the faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-latin.conf", Conf(
            Alias2("sans-serif", "DejaVu Sans"), Alias2("serif", "FreeSerif")));

        SystemFontResolver resolver = Resolver(index, tree.Root);

        // Resolve the face as a roman request first, which is what records `serif` against it.
        OpenTypeFace carlito = Primary(resolver, "Calibri", FontFamilyClass.Serif);
        resolver.FallbackFor(Tick, 400, false, carlito)?.FamilyName.ShouldBe("FreeSerif");

        // The same face, asked under a swiss item, takes the swiss list.
        resolver
            .FallbackFor(
                [Tick], 400, false, carlito,
                new FontItem("Calibri", FontFamilyClass.SansSerif, "en-US"))
            ?.FamilyName.ShouldBe("DejaVu Sans");
    }

    [Fact]
    public void OneFaceIsSoughtForAllOfTheRunsMissingCharactersAtOnce()
    {
        // `ImplGlyphFallbackLayout` gathers the layout's unmapped code units into one string and
        // `Substitute` puts every code point of it into a single FC_CHARSET, whose score —
        // how many of the set the candidate is missing — is fontconfig's highest priority. So a
        // face further down the family list wins if it covers more of the run.
        SystemFontIndex index = Installed();
        Assert.SkipWhen(
            !index.Has("OpenSymbol") || !index.Has("DejaVu Sans"),
            "the faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-latin.conf", Alias("sans-serif", "OpenSymbol", "DejaVu Sans"));

        SystemFontResolver resolver = Resolver(index, tree.Root);
        FontItem item = new("Calibri", FontFamilyClass.Unknown, "en-US");

        // OpenSymbol holds U+2713 and is first on the list, so on its own it answers.
        resolver.FallbackFor([Tick], 400, false, primary: null, item)
            ?.FamilyName.ShouldBe("OpenSymbol");

        // It does not hold U+2011. Asked for both at once the answer is the face that covers both,
        // although it is second on the same list.
        resolver.FallbackFor([Tick, NonBreakingHyphen], 400, false, primary: null, item)
            ?.FamilyName.ShouldBe("DejaVu Sans");
    }

    /// <summary>U+2610 BALLOT BOX: DejaVu Sans and the Free faces have it; Carlito does not.</summary>
    private const int Ballot = 0x2610;

    /// <summary>U+2713 CHECK MARK: OpenSymbol, DejaVu Sans and FreeSerif have it, Carlito does not.</summary>
    private const int Tick = 0x2713;

    private static string Alias2(string subject, params string[] preferred)
        => "<alias><family>" + subject + "</family><prefer>"
           + string.Concat(preferred.Select(f => $"<family>{f}</family>"))
           + "</prefer></alias>";

    private static string Alias(string subject, params string[] preferred)
        => "<?xml version=\"1.0\"?><fontconfig><alias><family>" + subject + "</family><prefer>"
           + string.Concat(preferred.Select(f => $"<family>{f}</family>"))
           + "</prefer></alias></fontconfig>";

    private static string Conf(params string[] aliases)
        => "<?xml version=\"1.0\"?><fontconfig>" + string.Concat(aliases) + "</fontconfig>";

    private static string Default(string subject, string target)
        => "<alias><family>" + subject + "</family><default><family>" + target
           + "</family></default></alias>";

    /// <summary>A throwaway fontconfig tree: a root file including a <c>conf.d</c> beside it.</summary>
    private sealed class Tree : IDisposable
    {
        private readonly string _directory;

        private Tree(string directory) => _directory = directory;

        public string Root => Path.Combine(_directory, "fonts.conf");

        public static Tree Create()
        {
            string directory = Path.Combine(Path.GetTempPath(), "fb-" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(Path.Combine(directory, "conf.d"));
            File.WriteAllText(
                Path.Combine(directory, "fonts.conf"),
                "<?xml version=\"1.0\"?><fontconfig><include ignore_missing=\"yes\">conf.d</include>"
                + "</fontconfig>");
            return new Tree(directory);
        }

        public void Write(string relative, string contents)
            => File.WriteAllText(Path.Combine(_directory, relative), contents);

        public void Dispose()
        {
            try
            {
                Directory.Delete(_directory, recursive: true);
            }
            catch (IOException)
            {
                // A temporary directory that will not delete is not a test failure.
            }
        }
    }
}

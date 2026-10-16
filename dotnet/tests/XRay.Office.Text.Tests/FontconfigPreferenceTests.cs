using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// The last step of glyph fallback: which of several installed faces draws a character nothing on
/// LibreOffice's own fallback list covers.
/// </summary>
/// <remarks>
/// <para>
/// Two of these tests pin a rule that was <em>rejected</em>, which is the point of writing them.
/// The resolver used to break that tie alphabetically by family name — a rule its own comment
/// admitted had no basis — and on this machine that puts <c>IPAGothic</c> and <c>Unifont</c> ahead
/// of <c>WenQuanYi Zen Hei</c> on every Han character, which is how a Chinese document came out as
/// a page of boxes. Reinstating the alphabet fails <see cref="ThePreferredFamilyBeatsTheAlphabet"/>;
/// dropping the configuration reader altogether fails
/// <see cref="TheAlphabetIsStillTheRuleWithNoFontconfig"/>, which is the behaviour a machine with no
/// fontconfig has to keep.
/// </para>
/// <para>
/// The parsing tests do not touch the machine's own configuration: they build a small tree in a
/// temporary directory, so they say the same thing on a machine with no <c>/etc/fonts</c> at all.
/// </para>
/// </remarks>
public class FontconfigPreferenceTests
{
    /// <summary>U+624B 手, the first character of the corpus document this was found on.</summary>
    private const int Han = 0x624B;

    // ------------------------------------------------------------------------------- the parser

    [Fact]
    public void APreferListForAGenericFamilyIsRead()
    {
        using Tree tree = Tree.Create();
        tree.Write("conf.d/50-one.conf", Alias("sans-serif", "Preferred Sans", "Second Sans"));

        FontconfigPreferences preferences = FontconfigPreferences.Read([tree.Root]);

        preferences.InOrder.ShouldBe(["preferredsans", "secondsans"]);
        preferences.RankOf("Preferred Sans").ShouldBeLessThan(preferences.RankOf("Second Sans"));
    }

    [Fact]
    public void ConfigurationFilesRankInAscendingNameOrder()
    {
        // fontconfig turns `<prefer>` into an `<edit name="family" mode="prepend">` applied at the
        // position of the matched family, so each file's entries land behind those of the files
        // already read. Checkable on the machine this was written on: `fc-match sans-serif` answers
        // DejaVu Sans (57-dejavu-sans.conf) and then WenQuanYi Zen Hei (64-wqy-zenhei.conf).
        using Tree tree = Tree.Create();
        tree.Write("conf.d/64-late.conf", Alias("sans-serif", "Late Sans"));
        tree.Write("conf.d/57-early.conf", Alias("sans-serif", "Early Sans"));

        FontconfigPreferences preferences = FontconfigPreferences.Read([tree.Root]);

        preferences.InOrder.ShouldBe(["earlysans", "latesans"]);
    }

    [Fact]
    public void AnAliasOfAConcreteFamilyIsNotAPreference()
    {
        // 30-metric-aliases.conf says Helvetica prefers Nimbus Sans. That is a statement about two
        // named families being interchangeable, not about what should draw a character nobody's
        // font covers, and reading it would rank half the machine's fonts for no reason.
        using Tree tree = Tree.Create();
        tree.Write("conf.d/30-metric.conf", Alias("Helvetica", "Nimbus Sans"));

        FontconfigPreferences.Read([tree.Root]).InOrder.ShouldBeEmpty();
    }

    [Fact]
    public void AFamilyNamedTwiceKeepsItsBetterRank()
    {
        using Tree tree = Tree.Create();
        tree.Write("conf.d/25-first.conf", Alias("sans-serif", "Shared"));
        tree.Write("conf.d/26-second.conf", Alias("serif", "Other", "Shared"));

        FontconfigPreferences preferences = FontconfigPreferences.Read([tree.Root]);

        preferences.InOrder.ShouldBe(["shared", "other"]);
    }

    [Fact]
    public void EachGenericKeepsItsOwnOrderAsWellAsTheMergedOne()
    {
        // The merged order answers "what does this machine prefer overall"; a *pattern* carries one
        // generic, and `FcConfigSubstitute` expands only that generic's <prefer> list into it. So a
        // face on the sans-serif list scores no better than an unnamed one against a serif pattern,
        // which is what separates DejaVu Sans from FreeSerif on a glyph fallback.
        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-latin.conf", Alias("serif", "Some Serif"));
        tree.Write("conf.d/61-latin.conf", Alias("sans-serif", "Some Sans"));

        FontconfigPreferences preferences = FontconfigPreferences.Read([tree.Root]);

        preferences.InOrderFor("serif").ShouldBe(["someserif"]);
        preferences.InOrderFor("sans-serif").ShouldBe(["somesans"]);
        preferences.RankOf("Some Sans", "serif").ShouldBe(int.MaxValue);
        preferences.RankOf("Some Serif", "sans-serif").ShouldBe(int.MaxValue);
        preferences.RankOf("Some Sans", "sans-serif").ShouldBe(0);

        // And the merged order still ranks both, which is what breaks a tie between two faces
        // neither of the pattern's own list names.
        preferences.RankOf("Some Serif").ShouldBe(0);
        preferences.RankOf("Some Sans").ShouldBe(1);
    }

    [Fact]
    public void AGenericsShortSpellingSharesItsList()
    {
        // LibreOffice appends the short spelling — `"sans"` for FAMILY_SWISS,
        // `vcl/unx/generic/font/fontconfig.cxx`:1082 — and the configuration files its lists under
        // the long one. Reading them apart would leave the swiss case with an empty list.
        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-latin.conf", Alias("sans-serif", "Some Sans"));
        tree.Write("conf.d/61-latin.conf", Alias("monospace", "Some Mono"));

        FontconfigPreferences preferences = FontconfigPreferences.Read([tree.Root]);

        preferences.RankOf("Some Sans", "sans").ShouldBe(0);
        preferences.RankOf("Some Mono", "mono").ShouldBe(0);
    }

    [Fact]
    public void AGenericNothingPrefersHasAnEmptyList()
        => FontconfigPreferences.None.InOrderFor("serif").ShouldBeEmpty();

    [Fact]
    public void AnUnnamedFamilyHasNoRank()
        => FontconfigPreferences.None.RankOf("Anything").ShouldBe(int.MaxValue);

    [Fact]
    public void AMalformedConfigurationFileIsSkippedRatherThanThrowing()
    {
        using Tree tree = Tree.Create();
        tree.Write("conf.d/50-broken.conf", "<fontconfig><alias>");
        tree.Write("conf.d/51-good.conf", Alias("serif", "Good Serif"));

        FontconfigPreferences.Read([tree.Root]).InOrder.ShouldBe(["goodserif"]);
    }

    // --------------------------------------------------------------------------- the resolver

    [Fact]
    public void ThePreferredFamilyBeatsTheAlphabet()
    {
        SystemFontIndex index = SystemFontIndex.Build(["/usr/share/fonts"]);
        Assert.SkipWhen(
            !index.Has("WenQuanYi Zen Hei") || !index.Has("IPAGothic"),
            "the CJK faces this compares are not installed; see check-env.sh");

        using Tree tree = Tree.Create();
        tree.Write("conf.d/64-wqy.conf", Alias("sans-serif", "WenQuanYi Zen Hei"));

        SystemFontResolver resolver = new(index, FontconfigPreferences.Read([tree.Root]));

        resolver.FallbackFor(Han)?.FamilyName.ShouldBe("WenQuanYi Zen Hei");
    }

    [Fact]
    public void TheAlphabetIsStillTheRuleWithNoFontconfig()
    {
        // A machine with no fontconfig — every Windows one — must behave exactly as this did before
        // the configuration was read at all: deterministic, and by name.
        SystemFontIndex index = SystemFontIndex.Build(["/usr/share/fonts"]);
        Assert.SkipWhen(
            !index.Has("WenQuanYi Zen Hei") || !index.Has("IPAGothic"),
            "the CJK faces this compares are not installed; see check-env.sh");

        SystemFontResolver resolver = new(index, FontconfigPreferences.None);

        string? chosen = resolver.FallbackFor(Han)?.FamilyName;

        chosen.ShouldNotBeNull();
        chosen.ShouldNotBe("WenQuanYi Zen Hei");
        string.CompareOrdinal(chosen, "WenQuanYi Zen Hei").ShouldBeLessThan(0);
    }

    [Fact]
    public void APreferenceForAFamilyThatCannotDrawTheCharacterIsSkipped()
    {
        SystemFontIndex index = SystemFontIndex.Build(["/usr/share/fonts"]);
        Assert.SkipWhen(
            !index.Has("WenQuanYi Zen Hei") || !index.Has("Liberation Sans"),
            "the faces this compares are not installed; see check-env.sh");

        // Liberation Sans is ranked first and has no Han glyphs, so coverage still decides.
        using Tree tree = Tree.Create();
        tree.Write("conf.d/10-first.conf", Alias("sans-serif", "Liberation Sans", "WenQuanYi Zen Hei"));

        SystemFontResolver resolver = new(index, FontconfigPreferences.Read([tree.Root]));

        resolver.FallbackFor(Han)?.FamilyName.ShouldBe("WenQuanYi Zen Hei");
    }

    // ------------------------------------------------------- the classification, and why it exists

    [Fact]
    public void AFamilyIsFiledUnderTheGenericItDefaultsTo()
    {
        // `45-latin.conf`'s whole content, in miniature: an <alias> whose body is a <default> naming
        // a generic is what says a family is a grotesque or a roman.
        using Tree tree = Tree.Create();
        tree.Write(
            "conf.d/45-latin.conf",
            Conf(
                Default("Calibri", "sans-serif"),
                Default("Cambria", "serif"),
                Default("Courier New", "monospace")));

        FontconfigPreferences fontconfig = FontconfigPreferences.Read([tree.Root]);

        fontconfig.GenericClassOf("Calibri").ShouldBe(FontFamilyClass.SansSerif);
        fontconfig.GenericClassOf("Cambria").ShouldBe(FontFamilyClass.Serif);
        fontconfig.GenericClassOf("Courier New").ShouldBe(FontFamilyClass.Fixed);
    }

    [Fact]
    public void AFamilyFiledUnderNothingIsAGrotesque()
    {
        // `49-sansserif.conf`: sans-serif is appended to any pattern that has not already named a
        // generic, so fontconfig has an answer for every name and never reports "unknown". This is
        // the rule the whole round turns on — `Century Schoolbook` reaches it, and `VCL.xcu` calls
        // the same family a roman.
        using Tree tree = Tree.Create();
        tree.Write("conf.d/45-latin.conf", Conf(Default("Cambria", "serif")));

        FontconfigPreferences.Read([tree.Root])
            .GenericClassOf("Century Schoolbook").ShouldBe(FontFamilyClass.SansSerif);
    }

    [Fact]
    public void ADefaultChainIsWalkedToWhicheverGenericItReaches()
    {
        // 30-metric-aliases.conf's <default>s name *concrete* families, so one hop is normal and the
        // answer depends on where the hop lands. Both of these are on the machine this was written
        // on and both are measured: `fc-match "Century Schoolbook"` answers DejaVu Sans and
        // `fc-match "Palatino Linotype"` answers DejaVu Serif.
        using Tree tree = Tree.Create();
        tree.Write(
            "conf.d/30-metric.conf",
            Conf(
                Default("Century Schoolbook", "New Century Schoolbook"),
                Default("Palatino Linotype", "Palatino")));
        tree.Write("conf.d/45-latin.conf", Conf(Default("Palatino", "serif")));

        FontconfigPreferences fontconfig = FontconfigPreferences.Read([tree.Root]);

        fontconfig.GenericClassOf("Century Schoolbook").ShouldBe(FontFamilyClass.SansSerif);
        fontconfig.GenericClassOf("Palatino Linotype").ShouldBe(FontFamilyClass.Serif);
    }

    [Fact]
    public void AFamilyTakesTheGenericOfAFamilyItAccepts()
    {
        // The case that says <default> alone is not enough. `Palatino` is filed under no generic at
        // all; what makes `fc-match Palatino` answer DejaVu Serif is that it *accepts* Palatino
        // Linotype, which 45-latin.conf files under serif — the accepted family joins the pattern
        // and brings its own generic with it. Reading only <default> answers DejaVu Sans and is
        // measurably wrong.
        using Tree tree = Tree.Create();
        tree.Write("conf.d/30-metric.conf", Conf(Accept("Palatino", "Palatino Linotype")));
        tree.Write("conf.d/45-latin.conf", Conf(Default("Palatino Linotype", "serif")));

        FontconfigPreferences.Read([tree.Root])
            .GenericClassOf("Palatino").ShouldBe(FontFamilyClass.Serif);
    }

    [Fact]
    public void AGenericThisResolverHasNoFaceListForBehavesAsTheDefault()
    {
        // `Cambria Math` is filed `math` by 45-generic.conf. A maths face is a roman, and filing it
        // as one is wrong: no maths font is installed on a stock configuration, so the pattern falls
        // through to the overall default and 26.2.4.2 draws it in DejaVu Sans.
        using Tree tree = Tree.Create();
        tree.Write("conf.d/45-generic.conf", Conf(Default("Cambria Math", "math")));

        FontconfigPreferences.Read([tree.Root])
            .GenericClassOf("Cambria Math").ShouldBe(FontFamilyClass.SansSerif);
    }

    [Fact]
    public void AGenericPreferListDoesNotClassifyTheFamiliesOnIt()
    {
        // 60-latin.conf lists a dozen families under <alias><family>serif</family><prefer>. That is
        // a preference order, not a classification, and following it would file every family on the
        // machine under every generic that mentions it.
        using Tree tree = Tree.Create();
        tree.Write("conf.d/60-latin.conf", Alias("serif", "Some Roman"));

        FontconfigPreferences fontconfig = FontconfigPreferences.Read([tree.Root]);

        fontconfig.Names("Some Roman").ShouldBeFalse();
        fontconfig.RankOf("Some Roman").ShouldBe(0);
    }

    [Fact]
    public void ACycleInTheChainTerminates()
    {
        // 30-metric-aliases.conf really does contain `Arial → Helvetica` and `Helvetica → Arial`.
        using Tree tree = Tree.Create();
        tree.Write(
            "conf.d/30-metric.conf",
            Conf(Default("Arial", "Helvetica"), Default("Helvetica", "Arial")));

        FontconfigPreferences.Read([tree.Root])
            .GenericClassOf("Arial").ShouldBe(FontFamilyClass.SansSerif);
    }

    [Fact]
    public void WhetherFontconfigNamesAFamilyAtAllIsAskable()
    {
        // The question the substitution order turns on: a family fontconfig names nowhere gets
        // nothing but the default generic, so LibreOffice's own chain is never reached for it.
        using Tree tree = Tree.Create();
        tree.Write("conf.d/45-latin.conf", Conf(Default("Calibri", "sans-serif")));

        FontconfigPreferences fontconfig = FontconfigPreferences.Read([tree.Root]);

        fontconfig.IsConfigured.ShouldBeTrue();
        fontconfig.Names("Calibri").ShouldBeTrue();
        fontconfig.Names("CG Times").ShouldBeFalse();
    }

    [Fact]
    public void WithNoFontconfigThereIsNoClassificationToRead()
    {
        // Not "sans-serif" — *unknown*, so the resolver can tell "fontconfig says grotesque" from
        // "there is no fontconfig here" and fall back to LibreOffice's own table for the second.
        FontconfigPreferences.None.IsConfigured.ShouldBeFalse();
        FontconfigPreferences.None.GenericClassOf("Calibri").ShouldBe(FontFamilyClass.Unknown);
        FontconfigPreferences.None.Names("Calibri").ShouldBeFalse();
    }

    [Fact]
    public void AGenericNamedByADocumentClassifiesAsItself()
    {
        using Tree tree = Tree.Create();
        tree.Write("conf.d/45-latin.conf", Conf(Default("Calibri", "sans-serif")));

        FontconfigPreferences fontconfig = FontconfigPreferences.Read([tree.Root]);

        fontconfig.GenericClassOf("serif").ShouldBe(FontFamilyClass.Serif);
        fontconfig.GenericClassOf("monospace").ShouldBe(FontFamilyClass.Fixed);
    }

    private static string Alias(string subject, params string[] preferred)
        => "<?xml version=\"1.0\"?><fontconfig><alias><family>" + subject + "</family><prefer>"
           + string.Concat(preferred.Select(f => $"<family>{f}</family>"))
           + "</prefer></alias></fontconfig>";

    /// <summary>One or more alias fragments wrapped as a configuration file.</summary>
    private static string Conf(params string[] aliases)
        => "<?xml version=\"1.0\"?><fontconfig>" + string.Concat(aliases) + "</fontconfig>";

    private static string Default(string subject, string target)
        => "<alias><family>" + subject + "</family><default><family>" + target
           + "</family></default></alias>";

    private static string Accept(string subject, params string[] accepted)
        => "<alias><family>" + subject + "</family><accept>"
           + string.Concat(accepted.Select(f => $"<family>{f}</family>"))
           + "</accept></alias>";

    /// <summary>A throwaway fontconfig tree: a root file including a <c>conf.d</c> beside it.</summary>
    private sealed class Tree : IDisposable
    {
        private readonly string _directory;

        private Tree(string directory) => _directory = directory;

        public string Root => Path.Combine(_directory, "fonts.conf");

        public static Tree Create()
        {
            string directory = Path.Combine(Path.GetTempPath(), "fc-" + Guid.NewGuid().ToString("N"));
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

using Paperless.Text.Fonts;
using Shouldly;

namespace Paperless.Text.Tests;

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

    private static string Alias(string subject, params string[] preferred)
        => "<?xml version=\"1.0\"?><fontconfig><alias><family>" + subject + "</family><prefer>"
           + string.Concat(preferred.Select(f => $"<family>{f}</family>"))
           + "</prefer></alias></fontconfig>";

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

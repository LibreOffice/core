using System.Text;
using XRay.Office.Core.Globalization;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// Liang's algorithm over a Hunspell pattern file, and where those files come from.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The English cases are unconditional and the others are not, and that asymmetry is the
/// design.</strong> <c>hyph_en_US.dic</c> is in the tree, so a machine that cannot run these has a
/// broken build rather than a missing option; <c>hyph_fr.dic</c> and <c>hyph_es.dic</c> are not,
/// because their licences are LGPL and a GPL/LGPL/MPL tri-licence, so a test for them skips where
/// they are absent. A test that fails on a machine without a dictionary would be asserting that
/// LibreOffice is installed.
/// </para>
/// <para>
/// The expected breaks are not invented. Four of them were read out of LibreOffice 26.2.4.2's own
/// PDF text layer in <c>probes/chart-hyph-r105</c> §1.2, on three corpus documents whose charts it
/// hyphenates visibly: <c>Gov-ernance</c>, <c>Con-sumption</c>, <c>Pes-simistic</c> and
/// <c>Pas-senger</c>. They are the reference's answers rather than this implementation's.
/// </para>
/// </remarks>
public class HyphenationTests
{
    /// <summary>Where LibreOffice 26.2.4.2 keeps the three files this tree does not ship.</summary>
    private const string Extensions = "/opt/libreoffice26.2/share/extensions";

    private static IHyphenator English => Hyphenators.For("en-US")
        ?? throw new InvalidOperationException(
            "hyph_en_US.dic ships with XRay.Office.Core and must be beside the assembly");

    private static string[] Break(IHyphenator hyphenator, string word, string language = "en-US")
        => [.. hyphenator.FindHyphenationPoints(word, language)
            .Select(at => word[..at] + "-" + word[at..])];

    /// <summary>The shipped file is found, and its own minima are what it states.</summary>
    /// <remarks>
    /// <c>LEFTHYPHENMIN 2</c> and <c>RIGHTHYPHENMIN 3</c> are lines 2 and 3 of the file and are
    /// read rather than written down here — <c>README_hyph_en_US.txt</c> records the same pair as
    /// a change made in its 2010-03-16 revision, so the values are the data's and not a
    /// convention.
    /// </remarks>
    [Fact]
    public void TheShippedEnglishPatternsAreFoundAndCarryTheirOwnMinima()
    {
        Hyphenators.Enabled.ShouldBeTrue();
        Hyphenators.SearchPath.ShouldNotBeEmpty();

        HyphenationPatterns patterns = English.ShouldBeOfType<HyphenationPatterns>();

        patterns.LeftMinimum.ShouldBe(2);
        patterns.RightMinimum.ShouldBe(3);
        patterns.Count.ShouldBeGreaterThan(10_000);
    }

    /// <summary>The four breaks LibreOffice 26.2.4.2 was measured drawing.</summary>
    /// <remarks>
    /// Each is a hyphen the reference put on a corpus chart's own page, so this is a comparison
    /// against the reference's output rather than against a second implementation of Liang's
    /// algorithm. The point asserted is the one the reference took, which is the last that fits;
    /// the others are listed so a change in the pattern set shows up as a whole word rather than
    /// as an index.
    /// </remarks>
    [Theory]
    [InlineData("Governance", "Gov-ernance", "Gover-nance")]
    [InlineData("Consumption", "Con-sumption", "Consump-tion")]
    [InlineData("Pessimistic", "Pes-simistic")]
    [InlineData("Passenger", "Pas-senger", "Passen-ger")]
    public void TheReferencesOwnHyphensAreReproduced(string word, params string[] expected)
        => Break(English, word).ShouldBe(expected);

    /// <summary>The witness pair, which is the whole of the chart seat.</summary>
    /// <remarks>
    /// <c>038_Competitive_Advantage_Card</c>'s axis turns 45 degrees for <c>Cost Efficiency</c>
    /// and wraps upright for <c>Cost Stretched</c> — measured on the reference with the words
    /// swapped and again with the dictionary switched off (<c>probes/chart-axisrot-r91</c> §3,
    /// <c>probes/chart-hyph-r105</c> §1.1). The difference between the two is exactly this: the
    /// first word has hyphenation points and the second has none. Every one of r91's four
    /// controls — all of which the reference wraps — has none either.
    /// </remarks>
    [Fact]
    public void TheWitnessWordHyphenatesAndAllFiveOfItsControlsDoNot()
    {
        Break(English, "Efficiency").ShouldBe(["Ef-ficiency", "Effi-ciency"]);

        foreach (string control in new[]
                 { "Stretched", "Thoughts", "Strengths", "Scratched", "Cost" })
        {
            English.FindHyphenationPoints(control, "en-US").ShouldBeEmpty(control);
        }
    }

    /// <summary>The minima are enforced, at both ends and on the whole word.</summary>
    /// <remarks>
    /// <c>hyphenation</c> has a pattern point after <c>hy</c> and this file's left minimum is 2,
    /// so it survives; a right minimum of 3 is what keeps a point two characters from the end out.
    /// A word shorter than the two minima together can have no point at all and is answered
    /// without consulting the patterns.
    /// </remarks>
    [Fact]
    public void NoPointFallsInsideEitherMinimum()
    {
        foreach (int at in English.FindHyphenationPoints("hyphenation", "en-US"))
        {
            at.ShouldBeGreaterThanOrEqualTo(2);
            ("hyphenation".Length - at).ShouldBeGreaterThanOrEqualTo(3);
        }

        English.FindHyphenationPoints("area", "en-US").ShouldBeEmpty();
        English.FindHyphenationPoints("", "en-US").ShouldBeEmpty();
    }

    /// <summary>Case and a curly apostrophe are folded, and a trailing stop is not a letter.</summary>
    /// <remarks>
    /// All three are LibreOffice's own normalisations, <c>hyphenimp.cxx</c>:315-333. The folding
    /// is character by character so an offset means the same thing in the caller's word as in the
    /// folded one; a smart apostrophe reaching the patterns unfolded would miss every
    /// <c>'</c> pattern the 2010 revision added.
    /// </remarks>
    [Fact]
    public void CaseAnApostropheAndATrailingStopAreNormalised()
    {
        English.FindHyphenationPoints("EFFICIENCY", "en-US")
            .ShouldBe(English.FindHyphenationPoints("efficiency", "en-US"));

        English.FindHyphenationPoints("Governance.", "en-US")
            .ShouldBe(English.FindHyphenationPoints("Governance", "en-US"));

        English.FindHyphenationPoints("defiance’s", "en-US")
            .ShouldBe(English.FindHyphenationPoints("defiance's", "en-US"));
    }

    /// <summary>A pattern file supplied as a stream is used for the language it is named for.</summary>
    /// <remarks>
    /// This is the supported route for the three files that are not shipped, and it needs nothing
    /// on disk — which is also what makes it the route a caller with the patterns embedded in
    /// their own application takes.
    /// </remarks>
    [Fact]
    public void APatternFileSuppliedAsAStreamAnswersForItsLanguage()
    {
        // The two minima are floored at 2 whatever the file says, because LibreOffice floors
        // them the same way: hyphenimp.cxx:435 and the linguistic property's own default of 2.
        const string invented = "UTF-8\nLEFTHYPHENMIN 1\nRIGHTHYPHENMIN 1\nzq1zq\n";

        try
        {
            Hyphenators.For("zz-ZZ").ShouldBeNull();

            Hyphenators.Register("zz-ZZ", new MemoryStream(Encoding.UTF8.GetBytes(invented)));

            Break(Hyphenators.For("zz-ZZ")!, "zqzq", "zz-ZZ").ShouldBe(["zq-zq"]);
        }
        finally
        {
            Hyphenators.Forget();
        }
    }

    /// <summary>
    /// A LibreOffice installation's own dictionaries are reachable by pointing at the folder
    /// above them.
    /// </summary>
    /// <remarks>
    /// <strong>Skips where they are absent, and that is not a weakened assertion.</strong> CI has
    /// no LibreOffice, and a test that fails there would be asserting that one is installed rather
    /// than that this code can use one. What it does assert where they are present is the whole
    /// contract for a supplied dictionary: one directory named, the per-language subdirectories
    /// found inside it, and the language actually answered from the right file — <c>fr</c> states
    /// <c>RIGHTHYPHENMIN 2</c> where English states 3, so the two cannot be confused.
    /// </remarks>
    [Fact]
    public void ALibreOfficeInstallationsFrenchAndSpanishAreReachableThroughTheSearchPath()
    {
        Assert.SkipUnless(
            Directory.Exists(Path.Combine(Extensions, "dict-fr")),
            $"no Hunspell dictionaries at {Extensions}; they are not shipped with XRay.Office");

        string? before = Environment.GetEnvironmentVariable(Hyphenators.Variable);

        try
        {
            Environment.SetEnvironmentVariable(Hyphenators.Variable, Extensions);
            Hyphenators.Forget();

            HyphenationPatterns french =
                Hyphenators.For("fr-FR").ShouldBeOfType<HyphenationPatterns>();

            french.RightMinimum.ShouldBe(2);

            // hyph_fr.dic states NEXTLEVEL on its fourth line, so every pattern in it belongs to
            // the ordinary level and dropping the (empty) compound level must lose nothing.
            french.Count.ShouldBeGreaterThan(1_000);
            Break(french, "hyphenation", "fr-FR").ShouldNotBeEmpty();

            HyphenationPatterns spanish =
                Hyphenators.For("es-ES").ShouldBeOfType<HyphenationPatterns>();

            // hyph_es.dic opens with 61 lines of licence as `%` comments, which must not become
            // patterns.
            spanish.Count.ShouldBeGreaterThan(100);
            Break(spanish, "hyphenation", "es-ES").ShouldNotBeEmpty();
        }
        finally
        {
            Environment.SetEnvironmentVariable(Hyphenators.Variable, before);
            Hyphenators.Forget();
        }
    }

    /// <summary>Switching the variable off is the no-dictionary state, reproducibly.</summary>
    /// <remarks>
    /// A deployment that does not carry the data is what the default path has to keep working for,
    /// and this is how a machine that does carry it can be put in that state to check.
    /// </remarks>
    [Fact]
    public void TheVariableTurnsEveryPatternFileOff()
    {
        string? before = Environment.GetEnvironmentVariable(Hyphenators.Variable);

        try
        {
            Environment.SetEnvironmentVariable(Hyphenators.Variable, "0");
            Hyphenators.Forget();

            Hyphenators.Enabled.ShouldBeFalse();
            Hyphenators.SearchPath.ShouldBeEmpty();
            Hyphenators.For("en-US").ShouldBeNull();
        }
        finally
        {
            Environment.SetEnvironmentVariable(Hyphenators.Variable, before);
            Hyphenators.Forget();
        }
    }

    /// <summary>Non-standard hyphenation is skipped rather than half-applied.</summary>
    /// <remarks>
    /// A <c>pattern/replacement,cut,pos</c> line changes the characters drawn, not only where the
    /// line ends, so applying the point without the replacement would misspell the word. None of
    /// the four files LibreOffice 26.2.4.2 ships uses the form.
    /// </remarks>
    [Fact]
    public void ANonStandardPatternIsIgnored()
    {
        HyphenationPatterns patterns = HyphenationPatterns.Read(
            new MemoryStream(Encoding.UTF8.GetBytes(
                "UTF-8\nLEFTHYPHENMIN 2\nRIGHTHYPHENMIN 2\nc1k/k=k,1,2\nzz1zz\n")));

        patterns.Count.ShouldBe(1);
        patterns.FindHyphenationPoints("backen", "de-DE").ShouldBeEmpty();
        Break(patterns, "zzzz", "de-DE").ShouldBe(["zz-zz"]);
    }
}

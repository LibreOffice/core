using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Globalization;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The category axis' rotate-or-wrap decision, and the dictionary that decides it.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Every label here fits its slot on two lines, so no width rule can separate these
/// cases.</strong> That is the point. <c>lcl_hasWordBreak</c> restarts the axis with line breaking
/// off when a laid-out line <em>begins in the middle of a word</em>
/// (<c>VCartesianAxis.cxx</c>:369-404, :888-905), and with a hyphenation dictionary loaded the
/// outliner creates such a line by putting a hyphenated piece of the next word on the previous
/// one. The one-line labels then collide and the axis turns 45 degrees.
/// </para>
/// <para>
/// The experiment is <c>probes/chart-hyph-r105</c> §1.1's, reproduced in miniature: hold every
/// character fixed and vary only whether a dictionary answers. LibreOffice 26.2.4.2 turns
/// <c>038_Competitive_Advantage_Card</c>'s axis under the five locales it has patterns for and
/// wraps it upright under the three it does not.
/// </para>
/// </remarks>
public class ChartAxisHyphenationTests
{
    /// <summary>Half an em per character, 1.15 em a line — the other axis tests' ruler.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly Length Size = Length.FromPoints(10);

    /// <summary>
    /// Five categories 60 pt apart, which is the arithmetic the whole class turns on.
    /// </summary>
    /// <remarks>
    /// At half an em a character the wrap limit is 0.95 × 60 = 57 pt, and against it
    /// <c>Efficiency</c> is 50 (fits), <c>Cost Efficiency</c> is 75 (does not), and
    /// <c>Cost Ef-</c> is 40 (fits, which is what makes the hyphen possible). Wrapped onto two
    /// lines the label is 50 pt wide against a 60 pt pitch, so wrapped labels do not collide and
    /// the axis has no other reason to turn.
    /// </remarks>
    private static ChartAxisLabelLayout Resolve(string label, IHyphenator? hyphenator)
    {
        string?[] texts = new string?[5];
        Length[] centres = new Length[5];

        for (int at = 0; at < 5; at++)
        {
            texts[at] = label;
            centres[at] = Length.FromPoints(20.0 + (at * 60.0));
        }

        return ChartAxisLabels.Resolve(
            texts, centres, new ChartAxisText(LineBreakAllowed: true), Size,
            new ChartText(new Ruler(), null), hyphenator: hyphenator);
    }

    /// <summary>Without a dictionary the labels wrap onto two lines and stay upright.</summary>
    /// <remarks>
    /// This is the tree's behaviour before this seat and the behaviour of any deployment that does
    /// not carry the pattern file, so it is asserted explicitly rather than left implied. It is
    /// also what 26.2.4.2 itself draws when its chart runs are tagged <c>de-DE</c>, a locale it
    /// ships no patterns for.
    /// </remarks>
    [Fact]
    public void WithNoDictionaryTheLabelsWrapUprightOntoTwoLines()
    {
        ChartAxisLabelLayout layout = Resolve("Cost Efficiency", Hyphenators.None);

        layout.Rotation.ShouldBe(0.0);
        layout.Rhythm.ShouldBe(1);
        layout.Texts.ShouldNotBeNull();
        layout.Texts![0].ShouldBe("Cost\nEfficiency");
    }

    /// <summary>With the shipped English patterns the same labels turn 45 degrees.</summary>
    /// <remarks>
    /// Not one character differs from the case above. <c>Efficiency</c> breaks after
    /// <c>Ef</c> and after <c>Effi</c>, so the first line can be <c>Cost Ef-</c> and the second
    /// begins inside the word — which turns line breaking off, and the unwrapped labels then
    /// overlap at this pitch.
    /// </remarks>
    [Fact]
    public void WithTheShippedEnglishPatternsTheSameLabelsAreTurned()
    {
        ChartAxisLabelLayout layout = Resolve("Cost Efficiency", null);

        layout.Rotation.ShouldBe(Math.PI / 4.0, 1e-9);
        layout.Texts.ShouldBeNull();
    }

    /// <summary>
    /// A label whose second word has no hyphenation point is unaffected by the dictionary.
    /// </summary>
    /// <remarks>
    /// The control, and it is r91's own: <c>Cost Stretched</c> is <em>wider</em> than
    /// <c>Cost Efficiency</c> was and the reference wraps it upright under all eight locales,
    /// because <c>Stretched</c> holds no point in any of the four installed dictionaries. A width
    /// rule would order these two the other way round.
    /// </remarks>
    [Fact]
    public void ALabelWithNoHyphenationPointWrapsWithTheDictionaryAsWithout()
    {
        ChartAxisLabelLayout with = Resolve("Cost Stretched", null);
        ChartAxisLabelLayout without = Resolve("Cost Stretched", Hyphenators.None);

        // Field by field rather than as records: Texts is a list, so the generated equality is
        // reference equality on it and two identical arrangements would compare unequal.
        with.Rotation.ShouldBe(without.Rotation);
        with.Rhythm.ShouldBe(without.Rhythm);
        with.Reserved.ShouldBe(without.Reserved);
        with.Texts.ShouldBe(without.Texts);

        with.Rotation.ShouldBe(0.0);
        with.Texts![0].ShouldBe("Cost\nStretched");
    }

    /// <summary>
    /// A supplied French dictionary reaches the axis by the same route the shipped one does.
    /// </summary>
    /// <remarks>
    /// <strong>Skips where LibreOffice's dictionaries are absent.</strong> What it proves is that
    /// the runtime-supplied path is not a second-class one: a caller who supplies patterns gets
    /// the same arrangement decision a shipped file gives, which is the whole point of not
    /// bundling French. The experiment is the class' own, run on French words:
    /// <c>Efficacité</c> breaks after <c>Ef</c> and <c>Marque</c> does not break at all, so one
    /// label turns and the other wraps with every character of the first two held fixed.
    /// <para>
    /// <strong>An English control would be worthless here.</strong> French patterns break
    /// <c>Stretched</c> after <c>Stret</c> — a pattern file answers for the language it is
    /// written for, and applying it to another language's words is not a control but a different
    /// experiment.
    /// </para>
    /// </remarks>
    [Fact]
    public void ASuppliedFrenchDictionaryTurnsTheAxisTheSameWay()
    {
        const string French = "/opt/libreoffice26.2/share/extensions/dict-fr/hyph_fr.dic";

        Assert.SkipUnless(
            File.Exists(French),
            $"no French Hunspell dictionary at {French}; it is not shipped with X-Ray.Office");

        HyphenationPatterns patterns =
            HyphenationPatterns.ReadFile(French)
            ?? throw new InvalidOperationException(French);

        patterns.FindHyphenationPoints("Marque", "fr-FR").ShouldBeEmpty();

        // Same two words, same widths, and only the dictionary varies.
        Resolve("Coût Efficacité", Hyphenators.None).Rotation.ShouldBe(0.0);
        Resolve("Coût Efficacité", patterns).Rotation.ShouldBe(Math.PI / 4.0, 1e-9);

        // And the arrangement follows the patterns rather than the mere presence of a
        // hyphenator: a French word French cannot break leaves the axis upright.
        Resolve("Coût Marque", patterns).Rotation.ShouldBe(0.0);
    }
}

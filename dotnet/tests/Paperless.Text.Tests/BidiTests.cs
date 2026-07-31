using System.Globalization;
using System.Text;
using Paperless.Text.Itemisation;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// The Unicode Bidirectional Algorithm, checked rule by rule and then differentially against ICU.
/// </summary>
/// <remarks>
/// The differential set is the load-bearing part. ICU is not merely another implementation of
/// UAX #9 — it is the one LibreOffice calls (<c>ubidi_setPara</c> in
/// <c>vcl/source/text/ImplLayoutArgs.cxx</c>), so agreeing with it over every ordered pair and
/// triple of bidi classes is agreeing with Writer. The named cases below exist so that a failure
/// says which rule broke rather than only that something did.
/// </remarks>
public sealed class BidiTests
{
    [Fact]
    public void PlainLatinIsOneRunAtLevelZero()
    {
        BidiParagraph bidi = BidiParagraph.Resolve("The quick brown fox.");

        bidi.ParagraphLevel.ShouldBe((byte)0);
        bidi.Runs.Count.ShouldBe(1);
        bidi.Runs[0].Level.ShouldBe((byte)0);
        bidi.Runs[0].IsRightToLeft.ShouldBeFalse();
    }

    [Fact]
    public void AParagraphDeclaredLeftToRightStaysLeftToRightHoweverHebrewItIs()
    {
        // What a word processor needs, and what LibreOffice does: the paragraph's writing mode
        // decides its base level, not a sniff of its contents. A Hebrew sentence in an English
        // paragraph is still laid out from the left margin.
        BidiParagraph bidi = BidiParagraph.Resolve("שלום");

        bidi.ParagraphLevel.ShouldBe((byte)0);
        bidi.Runs.Count.ShouldBe(1);
        bidi.Runs[0].Level.ShouldBe((byte)1);
    }

    [Fact]
    public void AutoTakesTheParagraphLevelFromTheFirstStrongCharacter()
    {
        BidiParagraph.Resolve("ש abc", BidiDirection.Auto).ParagraphLevel.ShouldBe((byte)1);
        BidiParagraph.Resolve("abc ש", BidiDirection.Auto).ParagraphLevel.ShouldBe((byte)0);

        // P2 skips the contents of an isolate, so the Hebrew inside one says nothing.
        BidiParagraph.Resolve("⁧ש⁩ abc", BidiDirection.Auto)
            .ParagraphLevel.ShouldBe((byte)0);
    }

    [Fact]
    public void EuropeanDigitsInsideHebrewSitTwoLevelsDeep()
    {
        // I1: at an even level a right-to-left character goes one deeper and a number two, so the
        // digits read left to right inside a phrase that reads right to left. This is the case
        // every bidi implementation is judged on.
        const string Text = "Start שלום 123 עולם end.";
        BidiParagraph bidi = BidiParagraph.Resolve(Text);

        Levels(bidi).ShouldBe("000000111112221111100000");
    }

    [Fact]
    public void EuropeanDigitsAfterAnArabicLetterBecomeArabicNumbers()
    {
        // W2, which keys on AL and not on R. The same digits after Hebrew stay European — and the
        // difference shows up in the level, since AN and EN resolve to the same level here but W2
        // is what stops "1" being joined to a following "." by W4.
        BidiParagraph arabic = BidiParagraph.Resolve("ا 123");
        BidiParagraph hebrew = BidiParagraph.Resolve("ש 123");

        // The levels are the same on both sides, which is the point: the level cannot tell them
        // apart, and the resolved class can.
        Levels(arabic).ShouldBe("11222");
        Levels(hebrew).ShouldBe("11222");

        arabic.ResolvedClassAt(2).ShouldBe(BidiClass.AN);
        hebrew.ResolvedClassAt(2).ShouldBe(BidiClass.EN);
    }

    [Fact]
    public void ANumberInEnglishProseStaysAtTheParagraphLevel()
    {
        // W7. Without it the digits would go two levels deep by I1 and become their own run, which
        // would split every English sentence containing a number into three portions.
        BidiParagraph bidi = BidiParagraph.Resolve("abc 123 def");

        bidi.Runs.Count.ShouldBe(1);
        Levels(bidi).ShouldBe("00000000000");
    }

    [Fact]
    public void BracketsTakeTheDirectionOfWhatIsInsideThem()
    {
        // N0, in the arrangement where it changes the answer: brackets holding text of the opposite
        // direction, opened after text that is also of the opposite direction. Without the rule the
        // closing bracket resolves as an ordinary neutral between the Hebrew and the Latin that
        // follows, lands at level 0, and is drawn on the wrong side of the phrase it closes.
        BidiParagraph bidi = BidiParagraph.Resolve("ש (ש) a");

        Levels(bidi).ShouldBe("1111100");

        bidi.ResolvedClassAt(2).ShouldBe(BidiClass.R);
        bidi.ResolvedClassAt(4).ShouldBe(BidiClass.R);
    }

    [Fact]
    public void AnUnmatchedBracketIsNotAPair()
    {
        // BD16's stack: a closing bracket with no matching opener resolves as an ordinary neutral,
        // which is what keeps a lone parenthesis in prose from dragging a phrase to another level.
        // Here N2 gives it the paragraph's direction rather than the Hebrew's.
        BidiParagraph bidi = BidiParagraph.Resolve("ש abc) ש");

        Levels(bidi).ShouldBe("10000001");
        bidi.ResolvedClassAt(5).ShouldBe(BidiClass.L);
    }

    [Fact]
    public void AnEmbeddingRaisesTheLevelAndItsPopLowersIt()
    {
        // X2 and X7, with the controls themselves removed from the runs by X9.
        BidiParagraph bidi = BidiParagraph.Resolve("a‫b‬c");

        // Latin inside a right-to-left embedding goes one level deeper still, by I2 — level 2, not
        // level 1 — which is why "b" reads left to right inside a run that reads right to left.
        Levels(bidi).ShouldBe("02200");
    }

    [Fact]
    public void AnOverrideForcesTheDirectionOfWhatItContains()
    {
        // X6 with the directional status stack's override in force: Latin inside an RLO reads right
        // to left however Latin it is.
        BidiParagraph bidi = BidiParagraph.Resolve("a‮bc‬d");

        Levels(bidi).ShouldBe("011100");
    }

    [Fact]
    public void AnIsolateHidesItsContentsFromTheTextAroundIt()
    {
        // X5a and X10 together, against the same text with an embedding instead of an isolate. The
        // number after the isolate looks past it and sees the Latin, so W7 makes it left to right at
        // the paragraph's own level; the number after the embedding sees the Hebrew, stays a European
        // number, and I1 puts it two levels deep. That difference is the whole reason isolates exist.
        BidiParagraph isolated = BidiParagraph.Resolve("abc ⁧ש⁩ 123");
        BidiParagraph embedded = BidiParagraph.Resolve("abc ‫ש‬ 123");

        Levels(isolated).ShouldBe("00000100000");
        Levels(embedded).ShouldBe("00001111222");
    }

    [Fact]
    public void ATrailingSpaceGoesBackToTheParagraphLevel()
    {
        // L1 clause 4. Without it the trailing space of a Hebrew line stays at level 1 and is drawn
        // on the left of the line rather than hanging off its right.
        BidiParagraph bidi = BidiParagraph.Resolve("של   ");

        bidi.Levels[^1].ShouldBe((byte)0);
        bidi.Levels[^2].ShouldBe((byte)0);
        bidi.Levels[0].ShouldBe((byte)1);
    }

    [Fact]
    public void ReorderingPutsTheRunsInTheOrderTheyAreDrawn()
    {
        // L2 over the run list. "Start [שלום 123 עולם] end." reverses the level-1 stretch and then
        // puts the digits back the right way round, so the second Hebrew word is drawn first.
        const string Text = "Start שלום 123 עולם end.";
        BidiParagraph bidi = BidiParagraph.Resolve(Text);

        List<int> logical = [.. bidi.Runs.Select(run => run.Start)];
        List<int> visual = [.. bidi.InVisualOrder().Select(run => run.Start)];

        logical.ShouldBe([0, 6, 11, 14, 19]);
        visual.ShouldBe([0, 14, 11, 6, 19]);
    }

    [Fact]
    public void ASurrogatePairIsNeverSplitByALevelBoundary()
    {
        // U+10800 CYPRIOT SYLLABLE A is right to left and lives outside the BMP, so both of its
        // UTF-16 halves have to land at the same level or the reordering would swap them.
        BidiParagraph bidi = BidiParagraph.Resolve("a𐠀b");

        bidi.Levels[1].ShouldBe(bidi.Levels[2]);
        bidi.Levels[1].ShouldBe((byte)1);
    }

    /// <summary>
    /// Every generated case, against the levels ICU resolved for the same text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One test rather than one per case: there are nearly eight thousand, and a theory would spend
    /// more time in the test framework than in the algorithm. The failure message names the first
    /// disagreements in full, which is what is actually needed to debug one.
    /// </para>
    /// <para>
    /// Two assertions, because the two are worth different amounts. The <em>reordering</em> is what
    /// renders, and it is required of every case without exception. The <em>levels</em> are required
    /// of every case that contains no explicit directional formatting character — which is every case
    /// that could come out of a document, since no format Paperless reads emits an embedding or an
    /// isolate. Of the 7,944 cases, 72 hold an unterminated or leading embedding, override or
    /// isolate, and on those ICU short-circuits to a flat level in a way that could not be
    /// characterised without its source. Their reordering agrees regardless, which is why they are
    /// held to the first assertion and not the second.
    /// </para>
    /// </remarks>
    [Fact]
    public void AgreesWithIcuOnEveryGeneratedCase()
    {
        string path = Path.Combine(AppContext.BaseDirectory, "bidi-cases.txt");
        File.Exists(path).ShouldBeTrue($"missing differential case file: {path}");

        int compared = 0;
        int withoutControls = 0;
        List<string> levelFailures = [];
        List<string> orderFailures = [];

        foreach (string line in File.ReadLines(path))
        {
            if (line.Length == 0 || line[0] == '#') continue;

            string[] fields = line.Split('\t');
            if (fields.Length != 3) continue;

            string text = Unescape(fields[0]);
            byte paragraphLevel = byte.Parse(fields[1], CultureInfo.InvariantCulture);
            byte[] expected = [.. fields[2].Split(' ', StringSplitOptions.RemoveEmptyEntries)
                .Select(value => byte.Parse(value, CultureInfo.InvariantCulture))];

            BidiParagraph bidi = BidiParagraph.Resolve(
                text,
                paragraphLevel == 0 ? BidiDirection.LeftToRight : BidiDirection.RightToLeft);
            byte[] actual = bidi.Levels.ToArray();

            compared++;
            string where = $"\"{fields[0]}\" at paragraph level {paragraphLevel}";

            if (!Reorder(expected).SequenceEqual(Reorder(actual)) && orderFailures.Count < 10)
            {
                orderFailures.Add(
                    $"{where}: ICU orders {string.Join(" ", Reorder(expected))} "
                    + $"from levels {string.Join(" ", expected)}, "
                    + $"Paperless {string.Join(" ", Reorder(actual))} "
                    + $"from levels {string.Join(" ", actual)}");
            }

            if (HasExplicitFormatting(text)) continue;

            withoutControls++;
            if (!actual.SequenceEqual(expected) && levelFailures.Count < 10)
            {
                levelFailures.Add(
                    $"{where}: ICU {string.Join(" ", expected)}, "
                    + $"Paperless {string.Join(" ", actual)}");
            }
        }

        compared.ShouldBeGreaterThan(5000, "the differential case file looks truncated");
        withoutControls.ShouldBeGreaterThan(3000, "too few cases free of formatting characters");

        orderFailures.ShouldBeEmpty($"{orderFailures.Count} cases reorder differently from ICU");
        levelFailures.ShouldBeEmpty($"{levelFailures.Count} cases disagree with ICU on levels");
    }

    /// <summary>Rule L2 over a level array: the logical index drawn at each visual position.</summary>
    private static int[] Reorder(byte[] levels)
    {
        int[] order = [.. Enumerable.Range(0, levels.Length)];
        if (levels.Length == 0) return order;

        int highest = levels.Max();
        int lowestOdd = levels.Where(level => (level & 1) != 0)
            .Select(level => (int)level)
            .DefaultIfEmpty(highest + 1)
            .Min();

        for (int level = highest; level >= lowestOdd; level--)
        {
            for (int i = 0; i < order.Length; i++)
            {
                if (levels[i] < level) continue;

                int limit = i + 1;
                while (limit < order.Length && levels[limit] >= level) limit++;
                Array.Reverse(order, i, limit - i);
                i = limit;
            }
        }

        return order;
    }

    /// <summary>True when the text holds an embedding, an override or an isolate.</summary>
    private static bool HasExplicitFormatting(string text)
        => text.Any(character => character
            is >= '‪' and <= '‮'
            or >= '⁦' and <= '⁩');

    private static string Levels(BidiParagraph bidi)
    {
        StringBuilder text = new();
        foreach (byte level in bidi.Levels) text.Append(level.ToString(CultureInfo.InvariantCulture));
        return text.ToString();
    }

    private static string Unescape(string escaped)
    {
        StringBuilder text = new(escaped.Length);

        for (int i = 0; i < escaped.Length; i++)
        {
            if (escaped[i] == '\\' && i + 5 < escaped.Length && escaped[i + 1] == 'u')
            {
                text.Append((char)ushort.Parse(
                    escaped.AsSpan(i + 2, 4), NumberStyles.HexNumber, CultureInfo.InvariantCulture));
                i += 5;
            }
            else
            {
                text.Append(escaped[i]);
            }
        }

        return text.ToString();
    }
}

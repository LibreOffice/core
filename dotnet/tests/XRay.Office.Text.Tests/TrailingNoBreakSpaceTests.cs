using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// A paragraph ending in a no-break space and ordinary blanks costs Writer two more line pitches.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Measured rather than ported.</strong> The trailing characters of one paragraph in
/// <c>150-5370-10H.docx</c> are <c>U+00A0</c> then <c>U+0020</c>, and LibreOffice 26.2.4.2 spends two
/// more line pitches on it than we did: the word bound to the no-break space moves onto a line of its
/// own, and an empty line follows. Nine tails, five adjustments, a one-line paragraph and a synthetic
/// body swept from one to four lines are in <c>probes/trailing-nbsp-wrap/</c>.
/// </para>
/// <para>
/// Two rounds recorded this as "a paragraph ending in a trailing space", having read the two
/// characters off a terminal where a no-break space and a blank are indistinguishable, and warned that
/// a fix would move text on most of any real corpus. It does not: trailing ordinary blanks cost
/// nothing however many there are, and only 7 of the corpus's 140 OOXML word documents hold a
/// paragraph that ends the way this one does.
/// </para>
/// <para>
/// The cases below state widths in ems of the test string rather than in points, so they say what the
/// rule is rather than restating Carlito's metrics.
/// </para>
/// </remarks>
public sealed class TrailingNoBreakSpaceTests
{
    private static readonly Length Size = Length.FromPoints(10);

    private const char Hard = '\u00A0';

    /// <summary>The flag is off by default, so Impress and Calc are untouched by any of this.</summary>
    private static readonly ParagraphFormat Writer = new() { SpillsTrailingNoBreakSpace = true };

    /// <summary>
    /// The corpus case: the last word is bound to the no-break space and travels onto its own line,
    /// and a blank line follows.
    /// </summary>
    [Fact]
    public void TheBoundWordMovesToItsOwnLineAndABlankLineFollows()
    {
        string text = $"alpha bravo charlie.{Hard} ";
        List<TextLine> filled = Fill(text, Writer);

        Text(text, filled, 0).ShouldBe("alpha bravo");
        Text(text, filled, 1).ShouldBe($"charlie.{Hard}");
        filled[^1].Length.ShouldBe(0, "the paragraph gains an empty line after the spilled one");
        filled.Count.ShouldBe(3);
    }

    /// <summary>
    /// A break opportunity immediately before the no-break space means nothing visible moves — and
    /// the paragraph still costs two pitches, both of them invisible.
    /// </summary>
    /// <remarks>
    /// This is the row that identifies the no-break space as the binding agent rather than the blank:
    /// the same tail with a blank in front of it drags nothing, because there is now somewhere to
    /// break. It is also why the rule is expressed as "break at the last opportunity before the hard
    /// space" and not as "move the last word".
    /// </remarks>
    [Fact]
    public void ABreakOpportunityBeforeTheHardSpaceLeavesEveryVisibleWordWhereItWas()
    {
        string text = $"alpha bravo charlie. {Hard} ";
        List<TextLine> filled = Fill(text, Writer);

        Text(text, filled, 0).ShouldBe("alpha bravo charlie.");
        filled.Count.ShouldBe(3, "two more lines, and neither of them holds a visible word");

        // The spilled line carries the no-break space itself, which is content and so is not trimmed
        // the way the blank after it is — it simply draws as a blank. The ordinary space that follows
        // it hangs, exactly as a trailing blank does on any other line.
        Text(text, filled, 1).ShouldBe($"{Hard}");
        filled[2].Length.ShouldBe(0);
    }

    /// <summary>Nothing else about a paragraph's tail costs anything.</summary>
    [Theory]
    [InlineData("alpha bravo charlie.")]
    [InlineData("alpha bravo charlie. ")]
    [InlineData("alpha bravo charlie.   ")]
    [InlineData("alpha bravo charlie.\u00A0")]
    [InlineData("alpha bravo charlie.\u00A0X")]
    [InlineData("alpha bravo charlie.\u00A0 X")]
    public void EveryOtherTailIsOneLine(string text)
    {
        Fill(text, Writer).Count.ShouldBe(1);
    }

    /// <summary>However many blanks follow the no-break space, the cost is the same two lines.</summary>
    [Theory]
    [InlineData(1)]
    [InlineData(2)]
    [InlineData(5)]
    public void TheBlanksAfterItAreCountedOnceHoweverManyThereAre(int blanks)
    {
        Fill($"alpha bravo charlie.{Hard}{new string(' ', blanks)}", Writer).Count.ShouldBe(3);
    }

    /// <summary>
    /// Block adjustment takes the trailing blanks into its justification and gains only the first of
    /// the two lines.
    /// </summary>
    [Theory]
    [InlineData(TextAlignment.Justify)]
    [InlineData(TextAlignment.Distribute)]
    public void ABlockAdjustedParagraphGainsTheSpilledLineAndNotTheBlankOne(TextAlignment alignment)
    {
        string text = $"alpha bravo charlie.{Hard} ";
        List<TextLine> filled = Fill(
            text,
            new ParagraphFormat { SpillsTrailingNoBreakSpace = true, Alignment = alignment });

        Text(text, filled, 0).ShouldBe("alpha bravo");
        filled.Count.ShouldBe(2);
    }

    /// <summary>
    /// With the flag off — every presentation and spreadsheet layout — the tail costs nothing, so
    /// this cannot reach Impress or Calc.
    /// </summary>
    [Fact]
    public void AParagraphThatDoesNotDeclareTheRuleIsUnaffected()
    {
        Fill($"alpha bravo charlie.{Hard} ", ParagraphFormat.Default).Count.ShouldBe(1);
        Fill($"alpha bravo charlie.{Hard} ", tabs: null).Count.ShouldBe(1);
    }

    /// <summary>
    /// A paragraph that is nothing but the tail keeps its single line: the split would have to land on
    /// the line's own start, which would move everything and make no progress.
    /// </summary>
    [Fact]
    public void AParagraphWithNothingBeforeTheHardSpaceIsNotSplit()
    {
        List<TextLine> filled = Fill($"{Hard} ", Writer);

        filled[0].Start.ShouldBe(0);
        filled.Count.ShouldBe(2, "no word to spill, but the blank line still follows");
    }

    /// <summary>One line's visible text, which is what the cases assert against.</summary>
    private static string Text(string paragraph, List<TextLine> lines, int at)
        => new(lines[at].VisibleTextIn(paragraph.AsSpan()));

    private static List<TextLine> Fill(string text, ParagraphFormat? tabs)
    {
        // Room for the whole paragraph and then some, so nothing here is a fitting decision: every
        // extra line these cases see is the rule's, not the width's.
        return Filler().Fill(text, Size, Length.FromPoints(1000), tabs: tabs);
    }

    private static LineFiller Filler()
        => new(Measurer(), breaker: null, breaksOverflowingBlanks: false);

    private static TextMeasurer Measurer()
    {
        string? path = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(path is null, "Carlito is not installed; see check-env.sh");
        return new TextMeasurer(OpenTypeFace.ReadFile(path!).ShouldNotBeNull());
    }

    private static string? FindFont(string fileName)
    {
        foreach (string root in new[]
                 {
                     "/usr/share/fonts", "/usr/local/share/fonts", "/Library/Fonts",
                     "C:\\Windows\\Fonts",
                 })
        {
            if (!Directory.Exists(root)) continue;

            string[] found = Directory.GetFiles(root, fileName, SearchOption.AllDirectories);
            if (found.Length > 0) return found[0];
        }

        return null;
    }
}

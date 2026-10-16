using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// A run of blanks wider than the line breaks inside it, for EditEngine and not for Writer.
/// </summary>
/// <remarks>
/// <para>
/// A word processor lets a line's trailing blanks hang past the margin, which is why
/// <see cref="LineFiller"/> measures a candidate to its visible end. EditEngine — Calc's text
/// layout — does not: <c>ImpEditEngine::ImpBreakLine</c> walks a character-position array that
/// counts every advance, blanks included, and when the character it stops on is a blank it breaks
/// one past it and compresses that blank away
/// (<c>editeng/source/editeng/impedit3.cxx:2016-2035</c>).
/// </para>
/// <para>
/// The consequence is a row height, not an alignment: a spreadsheet cell holding a hundred spaces
/// is four lines tall in Calc and one line tall without this. Measured on
/// <c>dotnet/probes/sheets-rest-01/mkspaceprobe.py</c> under the installed LibreOffice 26.2.4.2 —
/// eighteen wrapped cells in a 71.25 pt column differing only in their trailing whitespace — the
/// answers ladder 1, 2, 2, 3, 4, 4, 5 and 7 lines for 30, 40, 60, 80, 100, 120, 160 and 200
/// blanks, and the rule below reproduces every one of them.
/// </para>
/// <para>
/// The cases here are written against a width stated in blanks rather than in points, so they say
/// what the rule is rather than restating one font's metrics: whatever a blank measures, a line
/// twenty of them wide holds twenty.
/// </para>
/// </remarks>
public sealed class OverflowingBlankBreakTests
{
    private static readonly Length Size = Length.FromPoints(10);

    [Fact]
    public void WriterLetsAWholeParagraphOfBlanksHangOnOneLine()
    {
        // The control, and the behaviour every other caller still gets. It is not a bug: a line's
        // trailing blanks are invisible, and pushing a word to the next line because the space
        // after it did not fit is what this rule exists to prevent.
        Filler(breaksOverflowingBlanks: false)
            .Fill(new string(' ', 100), Size, RoomFor(10))
            .Count.ShouldBe(1, "a word processor lets them hang");
    }

    [Theory]
    [InlineData(10, 1)]
    [InlineData(20, 1)]
    [InlineData(30, 2)]  // thirty blanks do not fit on a line that holds twenty
    [InlineData(40, 2)]
    [InlineData(60, 3)]
    [InlineData(100, 5)]
    public void EditEngineCutsARunOfBlanksIntoLineFuls(int blanks, int lines)
    {
        // Twenty blanks are an exact fit, so a line consumes twenty of them.
        Filler(breaksOverflowingBlanks: true)
            .Fill(new string(' ', blanks), Size, RoomFor(20))
            .Count.ShouldBe(lines);
    }

    [Fact]
    public void TheBlankThatOverflowsStaysOnTheLineItEnds()
    {
        // Twenty and a half blanks wide, deliberately: on a line of exactly twenty the twenty-first
        // blank is the first that does not fit and the break lands *at* twenty, which cannot show
        // where the overflowing blank went. Half a blank of slack separates the two.
        List<TextLine> lines =
            Filler(breaksOverflowingBlanks: true).Fill(new string(' ', 100), Size, RoomFor(20.5));

        // `nBreakPos = nMaxBreakPos + 1` — "Break behind the blank, blank will be compressed".
        // The blank is inside the line's range and outside its visible text, so it takes no room
        // and the line is not over-full.
        lines[0].End.ShouldBe(21, "the line ends one past the blank that overflowed");
        lines[0].VisibleEnd.ShouldBe(0, "and none of it is visible text");
        lines[1].Start.ShouldBe(21, "the next line starts after it");
    }

    [Fact]
    public void AWordFollowedByBlanksKeepsTheWordOnTheFirstLine()
    {
        // The shape `SIL_TDB648` and the FAA accessory list actually hold: a short label and a
        // long tail of padding. The label must not be pushed off its own line by its padding.
        List<TextLine> lines =
            Filler(breaksOverflowingBlanks: true).Fill("ab" + new string(' ', 100), Size, RoomFor(20));

        lines[0].Start.ShouldBe(0);
        lines[0].VisibleEnd.ShouldBe(2, "the word is on the first line");
        lines.Count.ShouldBeGreaterThan(1, "and its padding is not");
    }

    [Fact]
    public void ASingleTrailingBlankStraddlingTheEdgeChangesNothing()
    {
        // The bound at the other end, and the reason this is a rule about blanks that *overflow*
        // rather than about counting blanks at all. Both engines put "aa " and then "bb" here:
        // EditEngine stops on the blank and breaks one past it, which is where the visible-end
        // measurement had already broken. A rule that simply counted the blanks would break
        // between the two words differently and move every justified line in the corpus.
        List<TextLine> hanging = Filler(breaksOverflowingBlanks: false).Fill("aa bb", Size, RoomFor(3));
        List<TextLine> cutting = Filler(breaksOverflowingBlanks: true).Fill("aa bb", Size, RoomFor(3));

        cutting.Count.ShouldBe(hanging.Count);
        for (int at = 0; at < cutting.Count; at++)
        {
            cutting[at].End.ShouldBe(hanging[at].End);
            cutting[at].VisibleEnd.ShouldBe(hanging[at].VisibleEnd);
        }
    }

    /// <summary>
    /// A width of so many blanks, so the cases state the rule rather than one font's metrics.
    /// </summary>
    /// <remarks>
    /// A whole number of them is an exact fit and the break lands on the boundary — twenty blanks
    /// of room take twenty blanks, not twenty-one — which is the same equality EditEngine's
    /// <c>&lt; nRemainingWidth</c> resolves. Pass a fraction when the case is about the blank that
    /// overflows rather than about how many fit.
    /// </remarks>
    private static Length RoomFor(double blanks)
    {
        TextMeasurer measurer = Measurer();
        Length one = measurer.Measure(" ", Size);
        return Length.FromEmu((long)(one.Emu * blanks));
    }

    private static LineFiller Filler(bool breaksOverflowingBlanks)
        => new(Measurer(), breaker: null, breaksOverflowingBlanks);

    private static TextMeasurer Measurer()
    {
        string? path = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(path is null, "Carlito is not installed; see check-env.sh");
        return new TextMeasurer(OpenTypeFace.ReadFile(path!).ShouldNotBeNull());
    }

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype",
                     "/usr/share/fonts",
                 })
        {
            if (!Directory.Exists(directory)) continue;

            string? found = Directory
                .EnumerateFiles(directory, fileName, SearchOption.AllDirectories)
                .FirstOrDefault();
            if (found is not null) return found;
        }

        return null;
    }
}

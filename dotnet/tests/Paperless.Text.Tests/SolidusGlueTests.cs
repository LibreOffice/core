using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// A break directly after a solidus is pulled back to the blank standing behind it.
/// </summary>
/// <remarks>
/// <para>
/// UAX #14 classes U+002F SOLIDUS as <c>SY</c> — a symbol allowing a break after — and LibreOffice
/// takes that back for paths and URIs in <c>BreakIterator_Unicode::getLineBreak</c>
/// (<c>i18npool/source/breakiterator/breakiterator_unicode.cxx:541-560</c>, i#17155). Having chosen
/// a break, it asks whether the character before it is a solidus, and if so scans backwards up to
/// 66 characters for whitespace and moves the break to just after it. When that lands on the line's
/// own start, EditEngine abandons the break iterator and cuts at the fitting limit instead —
/// "No separator in line =&gt; Chop!", <c>editeng/source/editeng/impedit3.cxx:2236-2247</c>.
/// </para>
/// <para>
/// So one rule moves a line's content in both directions, which is what made the residue it was
/// found in look like a rounding. The cases below are LibreOffice's own answers, read out of the
/// PDF it renders for a twelve-cell probe workbook in one wrapping column
/// (<c>dotnet/probes/sheets-r35/tokens.txt</c>): <c>CAT.IDE.A.170/CAT.IDE.H.170</c> alone in a cell
/// breaks after its solidus, and the same string after <c>AMC1 </c> does not break there at all and
/// is cut mid-number at the fitting limit.
/// </para>
/// <para>
/// Every width here is <em>measured from the text itself</em> rather than stated in points, so the
/// tests assert where the lines break rather than re-deriving one font's metrics.
/// </para>
/// </remarks>
public class SolidusGlueTests
{
    private static readonly Length Size = Length.FromPoints(10);

    /// <summary>The blank behind the solidus takes the whole path onto the next line.</summary>
    [Fact]
    public void ABreakAfterASolidusIsPulledBackToTheBlankBehindIt()
    {
        const string text = "AMC1 path/name";
        (LineFiller filler, TextMeasurer measurer) = Filler();

        // Room for one character past the solidus, so the solidus break is the last that fits.
        List<TextLine> lines = filler.Fill(text, Size, Width(measurer, text[..11]));

        lines.Count.ShouldBe(2);
        lines[0].End.ShouldBe(5);
        lines[1].Start.ShouldBe(5);
    }

    /// <summary>With nothing behind it, the solidus break stands.</summary>
    /// <remarks>
    /// The control for the rule above, and the half that keeps a long path breakable at all:
    /// LibreOffice's scan gives up at the start of the paragraph exactly as it does at 66
    /// characters.
    /// </remarks>
    [Fact]
    public void WithNoBlankBehindItTheSolidusBreakStands()
    {
        const string text = "path/nameXlongtail";
        (LineFiller filler, TextMeasurer measurer) = Filler();

        List<TextLine> lines = filler.Fill(text, Size, Width(measurer, text[..14]));

        lines.Count.ShouldBe(2);
        lines[0].End.ShouldBe(5);
    }

    /// <summary>
    /// The scan reaches 66 characters back and no further.
    /// </summary>
    /// <remarks>
    /// <c>nOverlyLong</c>, whose own comment calls the figure arbitrary. Both legs are the same
    /// text with a different run length, so the only thing separating them is the distance.
    /// </remarks>
    [Theory]
    [InlineData(64, 2)]
    [InlineData(65, 68)]
    public void TheScanGivesUpBeyondSixtySixCharacters(int run, int expectedEnd)
    {
        string text = "x " + new string('a', run) + "/tail";
        (LineFiller filler, TextMeasurer measurer) = Filler();

        List<TextLine> lines = filler.Fill(text, Size, Width(measurer, text[..(run + 5)]));

        lines[0].End.ShouldBe(expectedEnd);
    }

    /// <summary>
    /// When the blank is the line's own start, the path is cut across the line instead.
    /// </summary>
    /// <remarks>
    /// LibreOffice draws exactly this, and it is the leg that makes a row <em>shorter</em> than the
    /// plain rule would: three lines rather than four, with the second holding twenty-five
    /// characters ending mid-number.
    /// </remarks>
    [Fact]
    public void AGlueOntoTheLineStartCutsThePathAcrossTheLine()
    {
        const string text = "AMC1 CAT.IDE.A.170/CAT.IDE.H.170; CA";
        (LineFiller filler, TextMeasurer measurer) = Filler();

        List<TextLine> lines = filler.Fill(text, Size, Width(measurer, text[5..30]));

        lines.Count.ShouldBe(3);
        lines[0].End.ShouldBe(5);
        lines[1].End.ShouldBe(30);
        text[lines[1].Start..lines[1].End].ShouldBe("CAT.IDE.A.170/CAT.IDE.H.1");
    }

    /// <summary>The same string with no word in front of it breaks at the solidus.</summary>
    /// <remarks>
    /// The pair this makes with the test above is the whole finding in two lines of fixture: the
    /// text after the first blank is identical and the line breaks are not.
    /// </remarks>
    [Fact]
    public void TheSameStringAloneInTheCellBreaksAtTheSolidus()
    {
        const string text = "CAT.IDE.A.170/CAT.IDE.H.170; CA";
        (LineFiller filler, TextMeasurer measurer) = Filler();

        List<TextLine> lines = filler.Fill(text, Size, Width(measurer, text[..25]));

        lines.Count.ShouldBe(2);
        lines[0].End.ShouldBe(14);
    }

    /// <summary>
    /// A full stop between letters is no opportunity at all, so its word is cut.
    /// </summary>
    /// <remarks>
    /// The other control: the probe's two synthetic rows differ only in the separator, and
    /// LibreOffice breaks the solidus one after its second separator and the full-stop one at
    /// twenty-five characters. Without it, a rule that merely suppressed the solidus would look
    /// indistinguishable from one that never had the opportunity.
    /// </remarks>
    [Fact]
    public void AFullStopBetweenLettersIsNotAnOpportunity()
    {
        const string solidus = "abcd/efghijklmnop/qrstuvwxyz/ABCDEFGH";
        const string stop = "abcd.efghijklmnop.qrstuvwxyz.ABCDEFGH";
        (LineFiller filler, TextMeasurer measurer) = Filler();

        filler.Fill(solidus, Size, Width(measurer, solidus[..25]))[0].End.ShouldBe(18);
        filler.Fill(stop, Size, Width(measurer, stop[..25]))[0].End.ShouldBe(25);
    }

    private static Length Width(TextMeasurer measurer, string text)
        => measurer.Measure(text, Size);

    private static (LineFiller Filler, TextMeasurer Measurer) Filler()
    {
        string? path = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(path is null, "Carlito is not installed; see check-env.sh");
        TextMeasurer measurer = new(OpenTypeFace.ReadFile(path!).ShouldNotBeNull());
        return (new LineFiller(measurer), measurer);
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

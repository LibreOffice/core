using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests the case a run is <em>drawn</em> in, which every word-processing format states separately
/// from the case it is stored in: <c>w:caps</c>, <c>w:smallCaps</c>, <c>sprmCFCaps</c>,
/// <c>sprmCFSmallCaps</c>, <c>\caps</c>, <c>\scaps</c>, <c>fo:text-transform</c> and
/// <c>fo:font-variant</c>.
/// </summary>
/// <remarks>
/// <para>
/// Forty-three of the corpus's hundred and thirty-six DOCX files declare <c>w:caps</c> and
/// twenty-eight declare <c>w:smallCaps</c>, so this is ordinary formatting rather than a corner.
/// A reader that parses the toggles and never applies them draws a heading in the case the file
/// happens to store it in, which is usually not the case Word displays.
/// </para>
/// <para>
/// The invariant the tests exist for is the length. The mapped text is indexed by the paragraph's
/// runs, note anchors, frame anchors and bookmarks, all of which were recorded against the stored
/// text — so a mapping that lengthened it would silently move every one of them.
/// </para>
/// </remarks>
public class CaseMappingTests
{
    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }

    private static PageRun Run(int start, int length, PageCaseMap map, double points = 10)
        => new(start, length, Face, Length.FromPoints(points), CaseMap: map);

    [Fact]
    public void AParagraphWithNoCaseMapIsReturnedUntouched()
    {
        List<PageRun> runs = [Run(0, 5, PageCaseMap.None)];

        CaseMapping.Apply("Hello", runs).ShouldBe("Hello");
        runs.Count.ShouldBe(1);
    }

    [Fact]
    public void FullCapitalsUppercaseTheRunAndLeaveItOneRun()
    {
        List<PageRun> runs = [Run(0, 5, PageCaseMap.Uppercase), Run(5, 6, PageCaseMap.None)];

        CaseMapping.Apply("Hello world", runs).ShouldBe("HELLO world");

        runs.Count.ShouldBe(2);
        runs[0].CaseMap.ShouldBe(PageCaseMap.None);
        runs[0].EmSize.ShouldBe(Length.FromPoints(10));
    }

    /// <summary>
    /// Small capitals split the run at every change between what was lowercase and what was not,
    /// because only the former is drawn smaller.
    /// </summary>
    [Fact]
    public void SmallCapitalsSplitAtEveryCaseBoundaryAndShrinkOnlyWhatWasLowercase()
    {
        List<PageRun> runs = [Run(0, 5, PageCaseMap.SmallCaps)];

        CaseMapping.Apply("Hello", runs).ShouldBe("HELLO");

        // "H" at full size, "ello" at four fifths of it.
        runs.Count.ShouldBe(2);
        runs[0].Start.ShouldBe(0);
        runs[0].Length.ShouldBe(1);
        runs[0].EmSize.ShouldBe(Length.FromPoints(10));

        runs[1].Start.ShouldBe(1);
        runs[1].Length.ShouldBe(4);
        runs[1].EmSize.ShouldBe(Length.FromEmu((long)Math.Round(Length.FromPoints(10).Emu * 0.8)));
    }

    /// <summary>
    /// A shrunken small capital keeps the run's own size for its line metrics.
    /// </summary>
    /// <remarks>
    /// Writer builds the smaller font inside <c>SwSubFont::DoOnCapitals</c> and leaves
    /// <c>SwFont::GetHeight</c> — which is what <c>SwLineLayout::CalcLine</c> asks — reporting the
    /// unshrunken one. Without that separation a heading set entirely in lowercase small capitals
    /// would draw its line four fifths as tall as the reference and repaginate everything under it.
    /// </remarks>
    [Fact]
    public void ASmallCapitalKeepsTheRunsOwnSizeForItsLineMetrics()
    {
        List<PageRun> runs = [Run(0, 3, PageCaseMap.SmallCaps)];

        CaseMapping.Apply("abc", runs).ShouldBe("ABC");

        runs.Count.ShouldBe(1);
        runs[0].MetricEmSize.ShouldBe(Length.FromPoints(10));
        runs[0].ToFormattedRun().LineEmSize.ShouldBe(Length.FromPoints(10));
        runs[0].ToFormattedRun().EmSize.ShouldBeLessThan(Length.FromPoints(10));
    }

    /// <summary>
    /// The mapping never changes a length, however the Unicode uppercase of a character behaves.
    /// </summary>
    /// <remarks>
    /// The German sharp s is the case that matters: its uppercase as a <em>string</em> is two
    /// characters, and applying that would shift every offset recorded after it. LibreOffice guards
    /// the same thing with <c>bCaseMapLengthDiffers</c>.
    /// </remarks>
    [Fact]
    public void TheMappingNeverChangesTheTextsLength()
    {
        const string text = "Straße 1 – naïve ﬁn";
        List<PageRun> runs = [Run(0, text.Length, PageCaseMap.Uppercase)];

        string mapped = CaseMapping.Apply(text, runs);

        mapped.Length.ShouldBe(text.Length);
        mapped.ShouldContain('ß');
        mapped.ShouldContain("NAÏVE");
    }

    /// <summary>A run stating a case map but covering nothing leaves the text alone.</summary>
    [Fact]
    public void AnEmptyRunIsHarmless()
    {
        List<PageRun> runs = [Run(0, 0, PageCaseMap.SmallCaps), Run(0, 3, PageCaseMap.None)];

        CaseMapping.Apply("abc", runs).ShouldBe("abc");
        runs.Count.ShouldBe(2);
    }

    /// <summary>
    /// Runs reaching past the end of the text are clamped rather than throwing.
    /// </summary>
    /// <remarks>
    /// The runs come from a document. One that overruns its own paragraph is a repair, not a reason
    /// to fail the render — the same leniency <see cref="Text.Layout.MeasuredParagraph"/> applies to
    /// the ranges it measures.
    /// </remarks>
    [Fact]
    public void ARunPastTheEndOfTheTextIsClamped()
    {
        List<PageRun> runs = [Run(0, 99, PageCaseMap.Uppercase)];

        CaseMapping.Apply("abc", runs).ShouldBe("ABC");
    }
}

using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;
using Xunit;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What sizes the line an empty ODF paragraph occupies.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice's ODF export writes an empty line as <c>&lt;text:p&gt;&lt;text:span
/// text:style-name="T15"/&gt;&lt;/text:p&gt;</c> — a span carrying the character formatting and
/// no characters — and EditEngine measures that line from the character attributes at the
/// paragraph's own position, which is the span's
/// (<c>editeng/source/editeng/impedit3.cxx</c>:1896-1902). Reading the paragraph's own cascade
/// instead gives the shape's default size, which on a presentation placeholder is far larger.
/// </para>
/// <para>
/// This is the whole of what a blind reading of three corpus slides reported as
/// "inter-paragraph spacing too large by a constant": the constant is
/// <c>1.2 × (default size − span size)</c>, once per empty paragraph. On
/// <c>0335fab9-79f0-4944-b92c-f223837ca2d8.odp</c> page 6 that is 1.2 × (32 − 16) = 19.2 pt on
/// every gap, and the reading measured 19.2. Nothing was <em>added</em>, no margins were being
/// summed where the reference collapses them, and no shrink-to-fit was involved — all three of
/// which were on the table before the documents were read.
/// </para>
/// <para>
/// The fixture's four slides and the 26.2.4.2 figures they are checked against are in its own
/// header. The assertions here are baseline pitches rather than heights because that is what a
/// PDF states and what the fixture was verified through.
/// </para>
/// </remarks>
public class OdpEmptyParagraphTests
{
    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("odp-empty-paragraph.fodp")));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    /// <summary>The distance from the first drawn baseline to the second, in points.</summary>
    private static double PitchOn(LaidOutSlide slide)
    {
        List<double> baselines =
        [
            .. slide.Shapes.Where(shape => shape.Text is not null)
                .SelectMany(shape => shape.Text!.Runs)
                .Select(run => run.Run.Origin.Y.Points)
                .Distinct()
                .Order(),
        ];

        baselines.Count.ShouldBe(2);
        return baselines[1] - baselines[0];
    }

    [Fact]
    public void AnEmptyParagraphIsAsTallAsItsOwnEmptySpan()
    {
        SlidePages pages = Layout();
        pages.Count.ShouldBe(4);

        // 8 pt of "Alpha" plus 40 pt of the empty span, each at 1.2 em.
        PitchOn(pages.Slides[0]).ShouldBe(57.572, 0.01);

        // The same two paragraphs with an 8 pt empty span between them.
        PitchOn(pages.Slides[1]).ShouldBe(19.162, 0.01);
    }

    /// <remarks>
    /// The control. A paragraph with no span at all <em>does</em> take the shape's default —
    /// 24 pt here — and both renderers agreed on that case before this rule was found, which is
    /// why the defect could not be seen by comparing an empty paragraph against nothing.
    /// </remarks>
    [Fact]
    public void AParagraphWithNoSpanTakesTheShapeDefault()
        => PitchOn(Layout().Slides[2]).ShouldBe(38.381, 0.01);

    /// <remarks>
    /// Where an empty paragraph holds several empty spans the last one entered decides — not the
    /// largest, and not the outermost. Measured over four one-attribute variants of a corpus
    /// slide: 26.2.4.2 answers 8 pt for a 40 pt span followed by an 8 pt one, 40 pt for the
    /// reverse order, and 8 pt for an 8 pt span nested inside a 40 pt one.
    /// </remarks>
    [Fact]
    public void TheLastEmptySpanDecidesRatherThanTheLargest()
    {
        SlidePages pages = Layout();

        PitchOn(pages.Slides[3]).ShouldBe(PitchOn(pages.Slides[1]), 0.01);
        Math.Abs(PitchOn(pages.Slides[3]) - PitchOn(pages.Slides[0])).ShouldBeGreaterThan(1.0);
    }

    /// <remarks>
    /// The box is 18 cm wide and the words are short, so nothing wraps and no autofit fires;
    /// stated here because a pitch assertion would otherwise be measuring the fit.
    /// </remarks>
    [Fact]
    public void EverySlideDrawsBothWordsAtTheStatedSize()
    {
        foreach (LaidOutSlide slide in Layout().Slides)
        {
            IReadOnlyList<PlacedGlyphRun> runs =
                [.. slide.Shapes.Where(shape => shape.Text is not null)
                                .SelectMany(shape => shape.Text!.Runs)];

            string.Concat(runs.Select(run => run.Run.Text)).ShouldBe("AlphaBeta");

            // 7.994 rather than 8: a Length is an exact whole number of EMUs and 8 pt is not
            // one, so the comparison is a tolerance rather than an equality.
            runs.Select(run => run.Run.FontSize.Points).ShouldAllBe(size => Math.Abs(size - 8) < 0.01);
        }
    }
}

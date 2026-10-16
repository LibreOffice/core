using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;
using Xunit;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What a <c>text:a</c> does to a slide's line breaking and to how its lines are stacked.
/// </summary>
/// <remarks>
/// <para>
/// A hyperlink in a draw shape's text is not a character property: it is an EditEngine
/// <em>field</em>. <c>xmloff/source/text/txtparai.cxx</c>:1352-1370 asks the cursor for a
/// <c>HyperLinkURL</c> property and builds an <c>XMLImpHyperlinkContext_Impl</c> when it has one
/// and an <c>XMLUrlFieldImportContext</c> when it does not; Writer's text cursor has it and
/// Draw's, Impress's and Calc's do not.
/// </para>
/// <para>
/// Two rules follow, and neither is visible in the formatting. A field is one portion, so a field
/// wider than the room left on its line is filled to the <em>cell</em> — every character, on Latin
/// text — rather than moved down or broken at a separator
/// (<c>editeng/source/editeng/impedit3.cxx</c>:1101-1200). And the lines it spills onto belong to
/// the line that holds it, so each is drawn one <em>ascent</em> below the last rather than one line
/// height (<c>:3778-3795</c>, whose comment says so), and the formatter never sees them at all —
/// the block's measured height, which is what anchors it and what the shrink-to-fit search
/// measures, counts the field's line once.
/// </para>
/// <para>
/// So <em>“the reference breaks a long URL at any character where we break only at <c>-</c> and
/// <c>/</c>”</em> is not a URL rule and not a hyphenation rule, and the 1.0 em pitch a blind
/// reading of <c>0335fab9-79f0-4944-b92c-f223837ca2d8.odp</c> page 6 reported is not a
/// line-spacing rule. Both are what a field does.
/// </para>
/// <para>
/// The fixture's four slides and the 26.2.4.2 figures they are checked against are in its own
/// header. Reach across the converted corpus: <strong>665 <c>text:a</c> in 156 of the 302
/// <c>.odp</c></strong>.
/// </para>
/// </remarks>
public class OdpHyperlinkFieldTests
{
    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("odp-hyperlink-field.fodp")));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    /// <summary>The drawn baselines of a slide, in points from the page top, in order.</summary>
    private static List<double> Baselines(LaidOutSlide slide) =>
    [
        .. slide.Shapes.Where(shape => shape.Text is not null)
            .SelectMany(shape => shape.Text!.Runs)
            .Select(run => run.Run.Origin.Y.Points)
            .Distinct()
            .Order(),
    ];

    /// <summary>The distances between consecutive baselines.</summary>
    private static List<double> Pitches(LaidOutSlide slide)
    {
        List<double> baselines = Baselines(slide);
        return [.. baselines.Zip(baselines.Skip(1), (a, b) => b - a)];
    }

    /// <summary>The text drawn on each line, in order.</summary>
    private static List<string> Lines(LaidOutSlide slide)
    {
        Dictionary<double, string> byBaseline = [];

        foreach (PlacedGlyphRun run in
                 slide.Shapes.Where(shape => shape.Text is not null)
                             .SelectMany(shape => shape.Text!.Runs))
        {
            double y = Math.Round(run.Run.Origin.Y.Points, 3);
            byBaseline[y] = byBaseline.TryGetValue(y, out string? already)
                ? already + run.Run.Text
                : run.Run.Text;
        }

        return [.. byBaseline.OrderBy(pair => pair.Key).Select(pair => pair.Value)];
    }

    /// <remarks>
    /// The spill lines are 14.400 pt apart where the ordinary pitch is 17.773 — the line's
    /// <em>ascent</em> against its height. 26.2.4.2 draws this slide's five baselines at
    /// 42.747, 60.521, 74.921, 89.321 and 107.094.
    /// </remarks>
    [Fact]
    public void AFieldsSpillLinesAreOneAscentApartRatherThanOneLineHeight()
    {
        List<double> pitches = Pitches(Layout().Slides[0]);

        pitches.Count.ShouldBe(4);
        pitches[0].ShouldBe(17.773, 0.02);
        pitches[1].ShouldBe(14.400, 0.02);
        pitches[2].ShouldBe(14.400, 0.02);
        pitches[3].ShouldBe(17.773, 0.02);
    }

    /// <remarks>
    /// The control, and the one that says the pitch belongs to the field rather than to the URL:
    /// the same characters written as ordinary text draw every pitch at 17.773 and break at the
    /// solidi, which is what UAX #14 and LibreOffice's own path glue give.
    /// </remarks>
    [Fact]
    public void TheSameCharactersAsPlainTextTakeTheOrdinaryPitchAndBreakAtASolidus()
    {
        List<double> pitches = Pitches(Layout().Slides[1]);

        pitches.Count.ShouldBe(4);
        pitches.ShouldAllBe(pitch => Math.Abs(pitch - 17.773) < 0.02);

        Lines(Layout().Slides[1])[1].ShouldEndWith("/");
    }

    /// <remarks>
    /// The break positions themselves, which are what a character-level fill produces and a
    /// word-level one cannot: 26.2.4.2 ends the field's first line inside <c>organisational</c>
    /// and its second inside <c>planning</c>.
    /// </remarks>
    [Fact]
    public void AFieldIsBrokenAtAnyCharacterRatherThanAtASeparator()
    {
        List<string> lines = Lines(Layout().Slides[0]);

        lines.Count.ShouldBe(5);
        lines[1].ShouldBe("https://www.example.org/working-with-us/hr-connect/or");
        lines[2].ShouldBe("ganisational-development/career-and-development-plan");
        lines[3].ShouldBe("ning-framework/leadership/");
    }

    /// <remarks>
    /// <para>
    /// The spill is invisible to the formatter, so a middle-anchored box whose second paragraph is
    /// an over-long field is centred as though that paragraph were one line and the spill hangs
    /// below the box. The box runs 28.35 to 170.08 pt and is centred on 99.2; two lines of 17.773
    /// put the block's top at 81.43 and its first baseline at 95.83, which is what 26.2.4.2 draws.
    /// </para>
    /// <para>
    /// Counting the spill would put the first baseline 14.4 pt higher, so this assertion is what
    /// separates <em>“the painter advances by the ascent”</em> from <em>“the spill lines are
    /// lines”</em>.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheBlockHeightThatAnchorsAMiddleAnchoredBoxCountsTheFieldsLineOnce()
        => Baselines(Layout().Slides[2])[0].ShouldBe(95.840, 0.05);

    /// <remarks>
    /// The control for the whole rule: a field that fits its line is placed exactly as ordinary
    /// text would be, so a document whose links are short must not move at all.
    /// </remarks>
    [Fact]
    public void AFieldThatFitsChangesNothing()
    {
        List<double> pitches = Pitches(Layout().Slides[3]);

        pitches.Count.ShouldBe(2);
        pitches.ShouldAllBe(pitch => Math.Abs(pitch - 17.773) < 0.02);
    }
}

using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.Presentations.Ooxml;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Where a tab on a slide advances to.
/// </summary>
/// <remarks>
/// <para>
/// Two defects, and the second hid behind the first. Slide text was shaped with its tabs left in,
/// so a tab advanced the pen by whatever the face happens to give U+0009 rather than to a stop;
/// and the interval those stops sit at was a word processor's half inch rather than the inch both
/// slide formats use.
/// </para>
/// <para>
/// Measured on <c>slides/batch-001/ppt/policy-pesentation.ppt</c>, whose tenth page positions its
/// conclusion with three tabs. LibreOffice's PDF puts "a*" at 281.03 pt and ours put it at 173.20
/// — 107.83 pt out, which is three times the half inch the stops were spaced at. That page's ink
/// imbalance went from a major difference to none.
/// </para>
/// </remarks>
public class SlideTabStopTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static readonly Length Inch = Length.FromEmu(Length.EmuPerInch);

    private static SlideTextBody Body(string tabSize, string text) => PptxTextBody.Read(
        XElement.Parse(
            $"""
             <a:txBody xmlns:a="{A}">
               <a:bodyPr lIns="0" tIns="0" rIns="0" bIns="0" wrap="none"/>
               <a:p><a:pPr {tabSize}/><a:r><a:rPr lang="en-US" sz="1800"/>
                 <a:t>{text}</a:t></a:r></a:p>
             </a:txBody>
             """));

    private static List<PlacedGlyphRun> Place(SlideTextBody body)
        => SlideTextLayout.Place(
            body,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(720), Length.FromPoints(200)),
            new SlideFonts());

    [Fact]
    public void ATabAdvancesToTheNextStopRatherThanByItsOwnGlyph()
    {
        // Three tabs, so the text starts at the third stop. The stretches either side of a tab are
        // emitted as separate runs, which is also how the tab character stops being drawn.
        List<PlacedGlyphRun> placed = Place(Body(string.Empty, "\t\t\tend"));

        placed.Count.ShouldBe(1);
        placed[0].Run.Text.ShouldBe("end");
        placed[0].Run.Origin.X.Points.ShouldBe((Inch * 3).Points, 0.01);
    }

    [Fact]
    public void TheDefaultIntervalIsAnInchAndNotAWordProcessorsHalf()
    {
        Place(Body(string.Empty, "a\tb"))[1].Run.Origin.X.Points.ShouldBe(Inch.Points, 0.01);

        // The same paragraph under Word's half inch would have put it here, which is the state
        // this replaced.
        Place(Body(string.Empty, "a\tb"))[1].Run.Origin.X.Points.ShouldBeGreaterThan(70.0);
    }

    [Fact]
    public void AStatedDefTabSzIsUsedInsteadOfTheDefault()
    {
        // Half an inch, stated in EMUs.
        List<PlacedGlyphRun> placed = Place(Body("defTabSz=\"457200\"", "a\tb"));

        placed[1].Run.Origin.X.Points.ShouldBe(36.0, 0.01);
    }

    [Fact]
    public void ALineWithNoTabIsPlacedExactlyWhereItWasBefore()
    {
        // The untabbed line — nearly every line — goes through the same code path as one stretch
        // at offset zero, so a regression here would be a regression everywhere.
        List<PlacedGlyphRun> placed = Place(Body(string.Empty, "plain"));

        placed.Count.ShouldBe(1);
        placed[0].Run.Origin.X.ShouldBe(Length.Zero);
    }

    [Fact]
    public void ATabIsNotDrawnAsACharacter()
    {
        foreach (PlacedGlyphRun run in Place(Body(string.Empty, "a\tb")))
        {
            run.Run.Text.ShouldNotContain("\t");
        }
    }
}

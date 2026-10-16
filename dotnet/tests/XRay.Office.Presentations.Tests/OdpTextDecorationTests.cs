using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;
using Xunit;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The rule under and through an ODF slide's text.
/// </summary>
/// <remarks>
/// <para>
/// <c>OdfTextFormat</c> has resolved <c>style:text-underline-style</c> and
/// <c>style:text-line-through-style</c> since it was written, and <c>OdfTextBody.Run</c> passed
/// on neither — so no ODF slide had ever had a rule drawn on it. Two blind readings of unrelated
/// corpus pages reported the same thing: the hyperlink colour right and the underline missing.
/// Counted in the PDF's own marks rather than in a raster, <c>0335fab9</c> page 6 had 0 against
/// the reference's 9 and <c>medicines_bulletin</c> page 1 had 0 against 8.
/// </para>
/// <para>
/// No gate column can see it — a rule carries no characters and adds no page — which is why it
/// survived three rounds of the <c>.odp</c> column.
/// </para>
/// <para>
/// Drawn rather than read off the run, because the defect was in neither the style resolver nor
/// the decoration painter but in the one step between them.
/// </para>
/// </remarks>
public class OdpTextDecorationTests
{
    /// <summary>Thin, wide fills: what a rule is, whoever draws it.</summary>
    private static int RulesOn(int slide)
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("odp-text-decoration.fodp")));

        RecordingDrawingSink sink = new();
        IPageSequence pages = ((IPaginatedDocument)document).Layout();
        pages.Count.ShouldBe(4);
        pages[slide].Draw(sink);

        return sink.Pages[0].FilledPaths.Count(
            fill => fill.Bounds.Height.Points <= 2.5 && fill.Bounds.Width.Points >= 20);
    }

    [Fact]
    public void AnUnderlinedRunGetsARuleUnderIt() => RulesOn(1).ShouldBe(1);

    [Fact]
    public void AStruckThroughRunGetsARuleThroughIt() => RulesOn(2).ShouldBe(1);

    /// <remarks>
    /// Both controls, and they are not the same control: one states nothing and one states
    /// <c>none</c>. ODF spells "off" several ways and <c>OdfTextFormat.IsLineOn</c> is what has
    /// to know them, so a suite that only ever asserted the positive case would pass against a
    /// reader that drew a rule for every run.
    /// </remarks>
    [Fact]
    public void ARunThatStatesNoDecorationGetsNoRule()
    {
        RulesOn(0).ShouldBe(0);
        RulesOn(3).ShouldBe(0);
    }
}

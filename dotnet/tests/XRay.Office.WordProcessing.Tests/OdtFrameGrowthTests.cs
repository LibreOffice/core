using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>draw:frame</c> whose stated height is a <em>floor</em> is made as tall as its own text,
/// so it pushes the text below it down and can begin a page.
/// </summary>
/// <remarks>
/// <para>
/// ODF states Writer's three fly height kinds by which attribute carries the length:
/// <c>fo:min-height</c> on the <c>draw:text-box</c> is <c>SwFrameSize::Minimum</c> and
/// <c>svg:height</c> on the <c>draw:frame</c> is <c>SwFrameSize::Fixed</c>
/// (<c>xmloff/source/text/XMLTextFrameContext.cxx</c>:997-1010 and :655-661). The growth itself is
/// <c>SwFlyFrame::Format</c> (<c>sw/source/core/layout/fly.cxx</c>:1549-1570): the content height,
/// raised to the stated minimum less the frame's insets, raised again to <c>MINFLY</c>, and the
/// insets added back.
/// </para>
/// <para>
/// <strong>What this replaced was a frame that was placed but never grown.</strong> The frame took
/// its floor as its height, its text was drawn past its bottom, and nothing below it moved — so on
/// <c>odt-frame-grows-page.fodt</c> the whole document came out on one page with the frame's ten
/// lines and the body's five drawn over one another.
/// </para>
/// <para>
/// Ground truth is 26.2.4.2's own PDF of each fixture, read with <c>pdftotext -bbox</c>. The
/// figures are quoted per assertion.
/// </para>
/// </remarks>
public sealed class OdtFrameGrowthTests
{
    /// <summary>How close a baseline has to be to the reference's, in points.</summary>
    /// <remarks>
    /// Half a point, which is the tolerance the rest of this suite compares a baseline at. The
    /// measured differences here are 0.05 pt — the two renderers' page heights differ by that much,
    /// 841.8898 against 841.90 — so the margin is ample and the assertion is still tight enough to
    /// fail on one line's worth of growth in either direction.
    /// </remarks>
    private const double Tolerance = 0.5;

    /// <summary>
    /// A frame with no <c>svg:height</c> is as tall as its own three lines, so the paragraph that
    /// anchors it starts below them.
    /// </summary>
    /// <remarks>
    /// 26.2.4.2 draws GROWSONE at <c>yMin</c> 56.7288, GROWSTWO at 70.1788, GROWSTHREE at 83.6288
    /// and ANCHORA at 97.0788 — three 13.45 pt lines between the frame's first line and the body's,
    /// which is the frame's whole height and nothing else, its floor being nought.
    /// </remarks>
    [Fact]
    public void AFrameWithNoStatedHeightIsAsTallAsItsText()
    {
        Dictionary<string, DrawnWord> words = Words("odt-frame-grows.fodt", 1)[0];

        (words["ANCHORA"].Baseline - words["GROWSONE"].Baseline)
            .ShouldBe(97.0788 - 56.7288, Tolerance);

        // And the frame's own lines are one line apart, so the height is three of them rather than
        // one line and a gap the size of two.
        (words["GROWSTWO"].Baseline - words["GROWSONE"].Baseline)
            .ShouldBe(70.1788 - 56.7288, Tolerance);
    }

    /// <summary>
    /// <c>fo:min-height</c> is the whole frame's floor, so a frame holding one line and stating 3 cm
    /// is 3 cm tall.
    /// </summary>
    /// <remarks>
    /// 26.2.4.2 draws FLOORED at <c>yMin</c> 123.9788 and ANCHORB at 209.0288, which is 85.05 pt
    /// apart — 3 cm to a hundredth of a point. The frame holds one 13.45 pt line, so a reading that
    /// took the floor as the <em>content</em> height rather than the frame's would put the two
    /// 85.05 pt apart as well only by accident; what settles it is the companion fixture, where the
    /// same style states no padding and the content wins.
    /// </remarks>
    [Fact]
    public void AStatedMinimumHeightIsAFloorForTheWholeFrame()
    {
        Dictionary<string, DrawnWord> words = Words("odt-frame-grows.fodt", 1)[0];

        (words["ANCHORB"].Baseline - words["FLOORED"].Baseline)
            .ShouldBe(209.0288 - 123.9788, Tolerance);
    }

    /// <summary>
    /// A grown frame pushes the body text past the page bottom, which begins a page.
    /// </summary>
    /// <remarks>
    /// <c>odt-frame-grows-page.fodt</c> is a 10 cm page holding a ten-line frame and five body
    /// paragraphs. 26.2.4.2 puts GROWS01 to GROWS10, ANCHOR and AFTER1 on page one and AFTER2 to
    /// AFTER4 on page two. Before the frame was grown the whole document was one page.
    /// </remarks>
    [Fact]
    public void AGrownFrameBeginsAPage()
    {
        List<Dictionary<string, DrawnWord>> pages = Words("odt-frame-grows-page.fodt", 2);

        pages[0].Keys.ShouldContain("AFTER1");
        pages[0].Keys.ShouldNotContain("AFTER2");
        pages[1].Keys.Order(StringComparer.Ordinal).ShouldBe(["AFTER2", "AFTER3", "AFTER4"]);

        // ANCHOR sits under the frame's tenth line, not beside its first: 191.2282 against 56.7282
        // in the reference, ten 13.45 pt lines apart.
        (pages[0]["ANCHOR"].Baseline - pages[0]["GROWS01"].Baseline)
            .ShouldBe(191.2282 - 56.7282, Tolerance);
    }

    /// <summary>Every page's words, by their text.</summary>
    private static List<Dictionary<string, DrawnWord>> Words(string fixture, int expectedPages)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(expectedPages);
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages.Select(
            page => DrawnWords.On(page).ToDictionary(word => word.Text, word => word))];
    }
}

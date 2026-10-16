using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A shape anchored inside another frame's own text is placed and drawn.
/// </summary>
/// <remarks>
/// <para>
/// An anchored object belongs to the <em>page</em> however deeply its anchor is nested — which is
/// why <c>FrameResolution</c> already walked the body's blocks, the running heads, the footnotes
/// and every table cell. What it did not walk was a placed frame's own content, on the reasoning
/// that the outer frame's rectangle has to exist first. It does — but it exists by the time the
/// outer frame has been placed, so the walk simply belongs after that loop rather than beside it.
/// </para>
/// <para>
/// The failure was silent in the way a missing anchored object always is: the outer frame is drawn,
/// filled and stroked, and only the inner shape's text is absent. Measured on the converted corpus,
/// <b>six documents lose between 42 and 264 alphanumeric characters</b> to it, and on three of them
/// that loss is the document's whole shortfall against the reference to the character —
/// <c>020_Project_Timeline_Template_Modern_Theme</c> went from 109 of the reference's 316 to 316.
/// </para>
/// <para>
/// Ground truth is 26.2.4.2's own PDF of the fixture, read with <c>pdftotext -bbox</c>: OUTER at
/// 56.80, 56.7288, ANCHOR at 56.80, 70.1788 and INNER at 170.20, 113.4288.
/// </para>
/// </remarks>
public sealed class OdtNestedFrameTests
{
    /// <summary>Half a point, the tolerance the rest of this suite compares a position at.</summary>
    private const double Tolerance = 0.5;

    /// <summary>
    /// The shape inside the frame is drawn at all.
    /// </summary>
    [Fact]
    public void AShapeAnchoredInsideAFrameIsDrawn()
    {
        Words().Keys.ShouldContain("INNER");
    }

    /// <summary>
    /// And it is placed against the frame it is anchored in, not against the page or the body.
    /// </summary>
    /// <remarks>
    /// The shape states <c>svg:x="4cm" svg:y="2cm"</c> relative to its anchor paragraph, which is the
    /// frame's own first paragraph — so it lands 113.4 pt right of the frame's text and 56.7 pt below
    /// it, and 26.2.4.2 draws it exactly there. Placing it against the body area instead would put it
    /// at the same x by coincidence (the frame starts at the body's left edge) and 56.7 pt higher,
    /// which is why the vertical assertion is the load-bearing one.
    /// </remarks>
    [Fact]
    public void ItIsPlacedAgainstTheFrameItIsAnchoredIn()
    {
        Dictionary<string, DrawnWord> words = Words();

        (words["INNER"].Left - words["OUTER"].Left).ShouldBe(170.20 - 56.80, Tolerance);
        (words["INNER"].Baseline - words["OUTER"].Baseline).ShouldBe(113.4288 - 56.7288, Tolerance);
    }

    /// <summary>The first page's words, by their text.</summary>
    private static Dictionary<string, DrawnWord> Words()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require("odt-nested-frame.fodt")))
        {
            using IDocument document = new WordProcessingReader().Read(source);
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1);
            pages[0].Draw(sink);
        }

        return DrawnWords.On(sink.Pages[0]).ToDictionary(word => word.Text, word => word);
    }
}

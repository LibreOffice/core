using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Where <c>wp:positionV relativeFrom="paragraph"</c> measures its offset from.
/// </summary>
/// <remarks>
/// <para>
/// Not from the anchor paragraph's first line, but from a point above it: Writer's
/// <c>SwAnchoredObjectPosition::GetTopForObjPos</c>
/// (<c>sw/source/core/objectpositioning/anchoredobjectposition.cxx:225</c>) takes the anchor frame's own
/// top and adds back <c>GetUpperSpaceAmountConsideredForPrevFrame</c> — the <em>previous</em> paragraph's
/// lower space and line spacing (<c>sw/source/core/layout/flowfrm.cxx:1835</c>) — so the paragraph's own
/// space-before is not in the origin. <c>relativeFrom="paragraph"</c> is
/// <c>RelOrientation::FRAME</c>, whose vertical offset is nought
/// (<c>anchoredobjectposition.cxx:282</c>), which is what makes that origin the whole answer.
/// </para>
/// <para>
/// The fixture is authored for this test: three copies of the same 20 pt square, all at
/// <c>posOffset</c> nought, differing only in the spacing around the paragraph they anchor to. Measured
/// against LibreOffice 24.2.7.2, in points from the top of an A4 page with 2 cm margins:
/// </para>
/// <list type="table">
/// <item><term>no spacing at all</term><description>56.651 — the top margin</description></item>
/// <item><term>20 pt space-<em>before</em></term><description>70.451 — above the gap, not below it;
/// this engine drew it at 90.490 before the fix</description></item>
/// <item><term>20 pt space-<em>after</em> on the paragraph above</term><description>138.051 — below that
/// gap, which belongs to the paragraph above</description></item>
/// </list>
/// </remarks>
public sealed class AnchorParagraphSpacingTests
{
    /// <summary>
    /// How far a placed frame may sit from LibreOffice's, in points.
    /// </summary>
    /// <remarks>
    /// A point. The difference under test is 20 pt, and the residual disagreement on this fixture is
    /// 0.04 — the same page-origin rounding every comparison in this suite carries.
    /// </remarks>
    private const double TolerancePoints = 1.0;

    [Theory]
    [InlineData(0, 56.651)]
    [InlineData(1, 70.451)]
    [InlineData(2, 138.051)]
    public void AParagraphAnchorIgnoresTheParagraphsOwnSpaceBefore(int index, double libreOffice)
    {
        List<DocRect> images = Images();

        images.Count.ShouldBe(3);
        images[index].Y.Points.ShouldBe(libreOffice, TolerancePoints);
    }

    /// <summary>Every image the document draws, top to bottom.</summary>
    private static List<DocRect> Images()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source =
               DocumentSource.FromFile(Corpus.Require("anchor-paragraph-spacing.docx")))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages.SelectMany(page => page.Images).OrderBy(image => image.Y.Emu)];
    }
}

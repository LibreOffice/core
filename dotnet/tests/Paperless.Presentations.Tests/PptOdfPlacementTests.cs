using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks that the same deck lands in the same places through the binary and ODF paths.
/// </summary>
/// <remarks>
/// <para>
/// <c>ppt-features.ppt</c> is <c>slides-features.odp</c> converted by LibreOffice, so the two
/// files describe the same slides in two vocabularies that share nothing: EMUs against master
/// units of a 576th of an inch, a <c>svg:x</c> attribute against an eight-byte client anchor whose
/// fields are in the order top, left, right, bottom. A disagreement here is one of the two readers
/// being wrong about a deck whose correct answer is not in doubt, which is a far sharper signal
/// than either reader measured alone.
/// </para>
/// <para>
/// <strong>What is compared is the geometry, not the text.</strong> The two paths still lay text
/// out slightly differently — an ODP states its line spacing per paragraph style and a PPT states
/// it per outline level, and the ODF side does not yet read the list style its outline paragraphs
/// take their bullets and indents from — so a baseline comparison would pin those gaps as though
/// they were agreement. The shape rectangles have no such excuse: they are the same rectangles.
/// </para>
/// </remarks>
public class PptOdfPlacementTests
{
    /// <summary>
    /// How far the two may differ, in points.
    /// </summary>
    /// <remarks>
    /// A fifth of a point, which is what the conversion itself cost. A master unit is a 576th of
    /// an inch — an eighth of a point — and 576ths of an inch cannot express a whole number of
    /// centimetres, so every length in the deck was quantised on the way into the binary file and
    /// an extent, being a difference of two quantised edges, can lose two of them. Measured: the
    /// 2 cm margin is 720000 EMUs exactly in ODF and 454 units in PPT, 56.693 pt against 56.750;
    /// the 8 cm shape is 226.772 against 226.680; the ellipse's 2.012 cm height is 57.033 against
    /// 56.875, the worst of the deck at 0.158. Nothing here is a tolerance for a layout
    /// difference; a real one is tens of points.
    /// </remarks>
    private const double TolerancePoints = 0.2;

    private static IReadOnlyList<LaidOutSlide> Layout(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides;
    }

    [Fact]
    public void TheSameDeckPlacesItsShapesIdenticallyThroughTheBinaryAndOdfPaths()
    {
        IReadOnlyList<LaidOutSlide> binary = Layout("ppt-features.ppt");
        IReadOnlyList<LaidOutSlide> odf = Layout("slides-features.odp");

        binary.Count.ShouldBe(odf.Count);

        for (int slide = 0; slide < odf.Count; slide++)
        {
            binary[slide].Size.Width.Points.ShouldBe(
                odf[slide].Size.Width.Points, TolerancePoints, $"slide {slide + 1} width");
            binary[slide].Size.Height.Points.ShouldBe(
                odf[slide].Size.Height.Points, TolerancePoints, $"slide {slide + 1} height");
            binary[slide].IsHidden.ShouldBe(odf[slide].IsHidden, $"slide {slide + 1} hidden");

            binary[slide].Shapes.Count.ShouldBe(
                odf[slide].Shapes.Count, $"slide {slide + 1} shape count");

            for (int shape = 0; shape < odf[slide].Shapes.Count; shape++)
            {
                DocRect mine = binary[slide].Shapes[shape].Bounds;
                DocRect theirs = odf[slide].Shapes[shape].Bounds;
                string where = $"slide {slide + 1} shape {shape + 1}";

                mine.X.Points.ShouldBe(theirs.X.Points, TolerancePoints, $"{where}: left");
                mine.Y.Points.ShouldBe(theirs.Y.Points, TolerancePoints, $"{where}: top");
                mine.Width.Points.ShouldBe(theirs.Width.Points, TolerancePoints, $"{where}: width");
                mine.Height.Points.ShouldBe(theirs.Height.Points, TolerancePoints, $"{where}: height");
            }
        }
    }

    [Fact]
    public void TheSameDeckPutsItsTextAtTheSamePenThroughBothPaths()
    {
        IReadOnlyList<LaidOutSlide> binary = Layout("ppt-features.ppt");
        IReadOnlyList<LaidOutSlide> odf = Layout("slides-features.odp");

        // Across the shape only. The vertical is where the two paths genuinely still differ, and
        // the horizontal is where a difference would mean something: it is the shape's rectangle,
        // its text insets, and — for the ellipse — the box inscribed in it at 45 degrees, all
        // resolved through two unrelated readers.
        //
        // Only the shapes whose run counts agree. The deck's outline placeholder is deliberately
        // not among them: the binary path draws a marker run before each of its paragraphs and
        // indents the text past it, and the ODF path reads neither, because an ODF outline takes
        // its bullet and its indents from the list style its paragraphs are nested in and nothing
        // consults it yet. Comparing them would assert that the missing indent is correct.
        int compared = 0;

        for (int slide = 0; slide < odf.Count; slide++)
        {
            for (int shape = 0; shape < odf[slide].Shapes.Count; shape++)
            {
                if (binary[slide].Shapes[shape].Text is not { } mine) continue;
                if (odf[slide].Shapes[shape].Text is not { } theirs) continue;
                if (mine.Runs.Count != theirs.Runs.Count) continue;

                for (int run = 0; run < mine.Runs.Count; run++)
                {
                    mine.Runs[run].Run.Origin.X.Points.ShouldBe(
                        theirs.Runs[run].Run.Origin.X.Points,
                        TolerancePoints,
                        $"slide {slide + 1} shape {shape + 1} run {run + 1}");
                    compared++;
                }
            }
        }

        // A guard against the comparison quietly comparing nothing, which is what a reader that
        // stopped producing text would look like.
        compared.ShouldBeGreaterThanOrEqualTo(8);
    }
}

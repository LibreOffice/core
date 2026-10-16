using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What a binary PowerPoint shape's <c>txflTextFlow</c> does to its text.
/// </summary>
/// <remarks>
/// <para>
/// <c>ppt-text-flow.ppt</c> holds three identical 2.4 × 1.6 in boxes side by side, each reading
/// "Ag" at 16 pt, each anchored top, each with the same insets — and three different values of
/// Escher property 136: <c>mso_txflHorzN</c> (0), <c>mso_txflTtoBA</c> (1) and
/// <c>mso_txflBtoT</c> (2), left to right. Only that one property differs between them, and it
/// differs in four bytes: the file is a LibreOffice export whose three shapes' <c>WrapText</c>
/// entries were rewritten in place by <c>probes/slides-r56/repid-textflow.py</c>, so nothing else
/// about the boxes can have moved.
/// </para>
/// <para>
/// <b>Measured on LibreOffice 26.2.4.2, which is what makes these the right assertions.</b> On
/// the six-arm probe (<c>probes/slides-r56/patch-textflow.py</c>, which patches the same property
/// in a corpus document that already carries it) values 1, 3 and 5 all draw the text matrix
/// <c>0 -1 1 0</c> at identical pens and value 2 draws <c>0 1 -1 0</c>; 0 and 4 leave the text
/// upright. Our own pens land on the reference's to <b>0.05 pt on 18 of 18</b> — three anchors ×
/// two directions × three boxes.
/// </para>
/// <para>
/// The assertions below are stated as <em>relations</em> between the three boxes rather than as
/// absolute points, so nothing here depends on the face's ascent — the one quantity in the
/// arithmetic that belongs to the machine rather than to the file.
/// </para>
/// </remarks>
public class PptTextFlowTests
{
    private static IReadOnlyList<PlacedText> TextOf(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        SlidePages pages = (SlidePages)((IPaginatedDocument)document).Layout();

        return [.. pages.Slides[0].Shapes
            .Where(shape => shape.Text is not null)
            .Select(shape => shape.Text!)];
    }

    /// <summary>The three boxes, ordered left to right by where their first glyph lands.</summary>
    private static List<(PlacedText Text, DocPoint Pen)> Boxes()
    {
        List<(PlacedText, DocPoint)> boxes = [];

        foreach (PlacedText text in TextOf("ppt-text-flow.ppt"))
        {
            if (text.Runs.Count == 0) continue;

            boxes.Add((text, ShapeTransform.Apply(text.Transform, text.Runs[0].Run.Origin)));
        }

        return [.. boxes.OrderBy(box => box.Item2.X.Emu)];
    }

    [Fact]
    public void AHorizontalFlowLeavesTheTextUpright()
    {
        (PlacedText text, _) = Boxes()[0];

        // The control, and the reason the two below mean anything: the same box with the same
        // insets and the same anchor is NOT turned when the property says HorzN.
        text.IsUpright.ShouldBeTrue();
    }

    [Fact]
    public void ATopToBottomFlowTurnsTheTextAQuarterOneWayAndBottomToTopTheOther()
    {
        List<(PlacedText Text, DocPoint Pen)> boxes = Boxes();
        boxes.Count.ShouldBe(3);

        AffineTransform down = boxes[1].Text.Transform;
        AffineTransform up = boxes[2].Text.Transform;

        // A quarter turn: the diagonal vanishes and the off-diagonal is unit.
        Math.Abs(down.A).ShouldBeLessThan(1e-9);
        Math.Abs(down.D).ShouldBeLessThan(1e-9);
        Math.Abs(up.A).ShouldBeLessThan(1e-9);
        Math.Abs(up.D).ShouldBeLessThan(1e-9);

        // And the two are opposite turns rather than the same one.
        Math.Sign(down.B).ShouldBe(-Math.Sign(up.B));
        Math.Sign(down.C).ShouldBe(-Math.Sign(up.C));
    }

    [Fact]
    public void AVerticalFlowStacksItsLinesDownThePageAndBottomToTopStacksThemUp()
    {
        List<(PlacedText Text, DocPoint Pen)> boxes = Boxes();

        // Two glyphs, one run each on this deck; the advance direction is what the turn decides.
        // Down-the-page reading means the run's advance maps to +y on the slide, and the other
        // way round for BtoT. Reading the sign off the matrix keeps this independent of how many
        // runs the layouter chose to split the line into.
        boxes[1].Text.Transform.B.ShouldBeGreaterThan(0);
        boxes[2].Text.Transform.B.ShouldBeLessThan(0);
    }

    [Fact]
    public void TheTurnedBoxesKeepTheirOwnPlacesOnTheSlide()
    {
        List<(PlacedText Text, DocPoint Pen)> boxes = Boxes();

        // The boxes are 3 in apart and 2.4 in wide, so a turn that moved a box onto its
        // neighbour — which is what a rotation about the wrong centre does — would collapse
        // this ordering. Stated as an ordering rather than as three coordinates.
        boxes[0].Pen.X.ShouldBeLessThan(boxes[1].Pen.X);
        boxes[1].Pen.X.ShouldBeLessThan(boxes[2].Pen.X);
    }
}

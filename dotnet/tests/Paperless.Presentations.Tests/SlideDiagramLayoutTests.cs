using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// A SmartArt diagram drawn by evaluating its layout definition, because it has no baked drawing.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why this path exists.</strong> A modern Office writes the diagram already laid out
/// into a <c>diagramDrawing</c> part and Paperless reads that — exactly and cheaply. Office 2007
/// wrote none, because the drawing vocabulary's namespace is dated 2008 and the files predate
/// it, and LibreOffice's own import fixtures have theirs removed by hand so that the evaluator
/// is what gets tested. Measured over <c>sd/qa/unit/data/pptx</c>: 66 decks carry a diagram and
/// 37 of them have no usable baked drawing. Every one of those 37 drew as nothing before this.
/// </para>
/// <para>
/// <strong>What the fixture exercises.</strong> <c>slide-diagram-evaluated.pptx</c> is
/// hand-written on the same skeleton as <c>slide-diagram-baked.pptx</c> with the drawing part
/// removed and a layout definition that uses the four commonest algorithms together —
/// <c>lin</c> divides the frame, <c>composite</c> places two children inside each division from
/// constraints, <c>sp</c> draws the box and <c>tx</c> sets the label. Every number is round, so
/// a disagreement here is an arithmetic bug rather than a rounding: three 180 × 108 pt boxes at
/// 72, 252 and 432 pt across a 540 × 216 pt frame.
/// </para>
/// <para>
/// The comparison against LibreOffice's own rendering lives in
/// <c>Paperless.Fidelity.Tests/SlideDiagramLayoutComparisonTests</c>; this file pins the numbers
/// that comparison depends on, so a regression names which algorithm broke.
/// </para>
/// </remarks>
public class SlideDiagramLayoutTests
{
    /// <summary>A twentieth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.05;

    private const string Deck = "slide-diagram-evaluated.pptx";

    /// <summary>The frame's top-left corner, in points.</summary>
    private const double FrameLeft = 72;
    private const double FrameTop = 108;

    /// <summary>
    /// The evaluator produces one shape per node and nothing for the scaffolding.
    /// </summary>
    /// <remarks>
    /// Six presentation points become three drawn shapes and three text-only ones: the
    /// <c>root</c> point is the frame, each <c>nodeComp</c> is a grouping shape with no geometry
    /// of its own, each <c>box</c> is a rounded rectangle and each <c>label</c> is a
    /// <c>hideGeom</c> shape that draws only its text. A reader that treated <c>hideGeom</c> as
    /// "not a shape" would lose every label; one that drew grouping shapes would put three
    /// unfilled rectangles behind the diagram.
    /// </remarks>
    [Fact]
    public void EveryNodeBecomesAShapeAndNothingElseDoes()
    {
        LaidOutSlide slide = Slide();

        slide.Shapes.Count.ShouldBe(6);
        slide.Shapes.Count(shape => shape.Fill is not null).ShouldBe(3);
        slide.Shapes.Count(shape => shape.Text is not null).ShouldBe(3);
    }

    /// <summary>
    /// The linear algorithm divides the frame equally and the composite places inside each part.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The three boxes are a third of the frame wide each — 180 pt of 540 — and half its height,
    /// which is the <c>h</c> constraint's factor of 0.5. Their vertical position is the part
    /// worth pinning: no constraint states one, so both children start at the top and the
    /// composite then centres the whole group in the space it did not use
    /// (<c>diagramlayoutatoms.cxx:714-727</c>). 216 pt of frame less 108 pt of content is 108 pt
    /// of slack, half of which is 54 — so the boxes sit at 162 pt rather than at 108.
    /// </para>
    /// <para>
    /// A reader that skipped the centring would place every box against the top of its frame,
    /// which looks plausible on a diagram whose constraints happen to fill the height and is
    /// wrong by half the slack on every one that does not.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(0, 0)]
    [InlineData(1, 180)]
    [InlineData(2, 360)]
    public void ALinearRowDividesTheFrameEqually(int index, double left)
    {
        DocRect bounds = Filled()[index].Bounds;

        bounds.Left.Points.ShouldBe(FrameLeft + left, TolerancePoints);
        bounds.Top.Points.ShouldBe(FrameTop + 54, TolerancePoints);
        bounds.Width.Points.ShouldBe(180, TolerancePoints);
        bounds.Height.Points.ShouldBe(108, TolerancePoints);
    }

    /// <summary>
    /// A <c>ctrX</c> constraint centres a child, rather than placing its left edge.
    /// </summary>
    /// <remarks>
    /// The label is 0.6 of its composite's width — 108 pt of 180 — and centred, so its left edge
    /// is at 36 pt within the composite and not at 0. Centring is applied <em>after</em> the
    /// width is known (<c>aPos.X = ctrX − aSize.Width / 2</c>), which is why the two constraints
    /// cannot be applied in either order.
    /// </remarks>
    [Fact]
    public void ACentredChildIsPlacedFromItsCentre()
    {
        PlacedText text = Slide().Shapes.First(shape => shape.Text is not null).Text!;

        // The label box spans 108 to 216 pt horizontally; a 14 pt "Alpha" centred in it starts
        // near 144, whereas an uncentred label would start at 108.
        text.Runs[0].Run.Origin.X.Points.ShouldBeGreaterThan(135);
        text.Runs[0].Run.Origin.X.Points.ShouldBeLessThan(155);
    }

    /// <summary>
    /// The colour transform's list cycles across the nodes rather than colouring them all alike.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is the whole reason both diagram parts have to be read. The quick style says
    /// <c>a:fillRef idx="1"</c>, which names the theme's first fill style — <c>solidFill</c> of
    /// <c>phClr</c> in every theme measured — and the colour transform says what <c>phClr</c>
    /// stands for, cycling a three-colour list by the presentation point's own
    /// <c>presStyleIdx</c>. Read the quick style alone and all three boxes are the theme's first
    /// accent.
    /// </para>
    /// <para>
    /// The three colours are the LibreOffice theme's accents 1 to 3, which are far enough apart
    /// that a wrong index is unmistakable.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(0, 0x18A303u)]
    [InlineData(1, 0x0369A3u)]
    [InlineData(2, 0xA33E03u)]
    public void TheColourListCyclesAcrossTheNodes(int index, uint colour)
    {
        Filled()[index].Fill.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Colour.FromRgb(colour));
    }

    /// <summary>
    /// A node's text comes from the data model and is drawn once per presenting shape.
    /// </summary>
    /// <remarks>
    /// Both the box and the label present the same data node — <c>srcOrd</c> 0 and 1 of the same
    /// point — and only the label draws it, because the <c>sp</c> algorithm throws the text away
    /// again ("text should appear only in tx node", <c>diagramlayoutatoms.cxx:1675</c>). Without
    /// that, every diagram draws each label twice, once behind the other.
    /// </remarks>
    [Fact]
    public void EachLabelIsDrawnOnce()
    {
        string[] drawn = Slide().Shapes
            .Where(shape => shape.Text is not null)
            .SelectMany(shape => shape.Text!.Runs)
            .Select(run => run.Run.Text)
            .ToArray();

        drawn.ShouldBe(["Alpha", "Beta", "Gamma"]);
    }

    /// <summary>
    /// Extraction reads the data model, not the tree the evaluator built.
    /// </summary>
    /// <remarks>
    /// The same separation the baked path keeps, and for the same reason: the drawn tree repeats
    /// a node's text wherever the layout put it, and an index wants what the author typed.
    /// </remarks>
    [Fact]
    public void ExtractionStillReadsTheDataModelOnceEach()
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(Deck)));

        string[] words = document.Content.GetText()
            .Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);

        words.ShouldBe(["Alpha", "Beta", "Gamma"]);
    }

    private static List<PlacedShape> Filled()
        => Slide().Shapes.Where(shape => shape.Fill is not null).ToList();

    private static LaidOutSlide Slide()
    {
        using IDocument read =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(Deck)));

        read.ShouldBeAssignableTo<IPaginatedDocument>();
        IReadOnlyList<LaidOutSlide> slides =
            ((SlidePages)((IPaginatedDocument)read).Layout()).Slides;

        slides.Count.ShouldBe(1);
        return slides[0];
    }
}

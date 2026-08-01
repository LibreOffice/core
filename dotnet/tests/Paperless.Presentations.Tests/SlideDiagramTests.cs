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
/// A SmartArt diagram drawn from the shape tree the authoring application baked into the package.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why the baked tree rather than the layout algorithms.</strong> Measured over every
/// OOXML document in the LibreOffice source tree carrying a <c>dgm:relIds</c> — 86 of them — 46
/// have a <c>diagramDrawing</c> part with at least one <c>dsp:sp</c> in it, 15 have the part
/// with the shapes taken out, and 25 have no part. Every one of the 40 without a usable one is
/// a LibreOffice import fixture, and the split is by authoring application: of the 62 written by
/// Office 2010 or later, 46 carry a usable baked drawing and the other 16 all show the part
/// having been removed by hand. Office 2007 wrote none at all — 0 of 24 — which is consistent
/// with the drawing vocabulary's own namespace being dated 2008.
/// </para>
/// <para>
/// So the baked tree is a DrawingML shape tree in a different namespace, and the whole slide
/// layouter already draws those. That is what is checked here: <c>slide-diagram-baked.pptx</c>
/// is hand-written so that every coordinate is exact, and its five shapes cover what a baked
/// tree actually contains — preset geometry, a solid fill, a gradient fill, a filled connector
/// with neither text nor a text rectangle, and a stroked one with no fill at all.
/// </para>
/// <para>
/// The comparison against LibreOffice's own rendering lives in
/// <c>Paperless.Fidelity.Tests/SlideDiagramComparisonTests</c>; this file pins the numbers that
/// comparison depends on, so a regression names which mapping broke rather than only that the
/// page changed.
/// </para>
/// </remarks>
public class SlideDiagramTests
{
    /// <summary>A twentieth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.05;

    private const string Deck = "slide-diagram-baked.pptx";

    /// <summary>
    /// The frame's top-left corner, in points — the origin of the diagram's own coordinates.
    /// </summary>
    private const double FrameLeft = 72;
    private const double FrameTop = 108;

    /// <summary>
    /// Every baked shape reaches the slide, connectors included.
    /// </summary>
    /// <remarks>
    /// The count is the assertion that a mis-set namespace would fail first: the tree is written
    /// in Microsoft's <c>dsp</c> vocabulary rather than PresentationML, so a reader looking for
    /// <c>p:sp</c> finds nothing at all and draws an empty region — which is precisely what a
    /// diagram did before this, and what the slide's text made look like a text-only problem.
    /// </remarks>
    [Fact]
    public void EveryShapeOfTheBakedTreeIsPlaced()
    {
        Slide().Shapes.Count.ShouldBe(5);
    }

    /// <summary>
    /// A baked shape's offset is measured from the frame, not from the slide.
    /// </summary>
    /// <remarks>
    /// The frame maps a child coordinate space exactly as a <c>p:grpSp</c> does, with no
    /// <c>a:chOff</c> or <c>a:chExt</c> — so the mapping is a translation by the frame's offset
    /// and nothing else. LibreOffice states the same thing from the other end at
    /// <c>oox/source/drawingml/diagram/diagram.cxx:131</c>,
    /// <c>pParentShape-&gt;setChildSize(pParentShape-&gt;getSize())</c>: a child space the same
    /// size as the frame. Getting this wrong puts the whole diagram at the slide's origin, which
    /// looks like a missing frame rather than a missing translation.
    /// </remarks>
    [Theory]
    [InlineData(0, 0, 36, 144, 144)]        // ellipse
    [InlineData(1, 180, 54, 180, 108)]      // roundRect
    [InlineData(2, 396, 54, 144, 108)]      // rect
    [InlineData(3, 147.6, 93.6, 28.8, 28.8)] // rightArrow, the filled connector
    [InlineData(4, 36, 180, 468, 0)]        // line, the stroked connector
    public void AShapeSitsWhereTheDiagramSpacePutsIt(
        int index, double left, double top, double width, double height)
    {
        DocRect bounds = Shape(index).Bounds;

        bounds.Left.Points.ShouldBe(FrameLeft + left, TolerancePoints);
        bounds.Top.Points.ShouldBe(FrameTop + top, TolerancePoints);
        bounds.Width.Points.ShouldBe(width, TolerancePoints);
        bounds.Height.Points.ShouldBe(height, TolerancePoints);
    }

    /// <summary>
    /// A baked shape's fill and pen are ordinary DrawingML and resolve as such.
    /// </summary>
    /// <remarks>
    /// Worth asserting separately from placement because the diagram vocabulary renames only the
    /// <em>outer</em> elements: <c>dsp:spPr</c> holds an <c>a:solidFill</c> and an <c>a:ln</c>
    /// that are byte-for-byte what a <c>p:sp</c> would hold. A reader that renamed too deeply
    /// would place all four shapes correctly and fill none of them.
    /// </remarks>
    [Theory]
    [InlineData(0, 0xC5E0B4u, 0x375623u, 1.0)]
    [InlineData(2, 0xBDD7EEu, 0x2E75B6u, 1.0)]
    [InlineData(3, 0xA6A6A6u, 0x595959u, 0.5)]
    public void AShapeKeepsItsOwnFillAndPen(
        int index, uint fill, uint line, double penPoints)
    {
        PlacedShape shape = Shape(index);

        shape.Fill.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Colour.FromRgb(fill));
        Stroke pen = shape.Line.ShouldNotBeNull();
        pen.Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Colour.FromRgb(line));
        pen.Width.Points.ShouldBe(penPoints, TolerancePoints);
    }

    /// <summary>A gradient inside a baked shape is the same gradient a slide shape would get.</summary>
    [Fact]
    public void AGradientFillSurvivesTheRename()
    {
        GradientPaint gradient = Shape(1).Fill.ShouldBeOfType<GradientPaint>();

        gradient.Kind.ShouldBe(GradientKind.Linear);
        gradient.Stops[0].Colour.ShouldBe(Colour.FromRgb(0xFFD966));
        gradient.Stops[^1].Colour.ShouldBe(Colour.FromRgb(0xBF8F00));

        // a:lin ang="5400000" is a quarter turn clockwise from the x axis: straight down.
        gradient.Start.X.ShouldBe(gradient.End.X);
        gradient.Start.Y.Emu.ShouldBeLessThan(gradient.End.Y.Emu);
    }

    /// <summary>
    /// A shape's text goes in the rectangle the file states, not in the geometry's own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>dsp:txXfrm</c> is a third way of stating a text area, beside a preset's built-in one
    /// and a <c>a:custGeom</c>'s <c>a:rect</c>, and it overrides both. It is stated in the
    /// <em>diagram's</em> coordinates rather than the shape's, which is the trap: taken as a
    /// shape-local rectangle it puts the text off the shape entirely, and for the many diagrams
    /// whose text rectangle happens to be centred it does so invisibly.
    /// </para>
    /// <para>
    /// The third shape's rectangle is deliberately off-centre — the right three quarters of the
    /// top half — so that honouring it and ignoring it give visibly different answers. 286 of
    /// the 469 baked shapes in LibreOffice's corpus carry one.
    /// </para>
    /// </remarks>
    [Fact]
    public void TextGoesInTheStatedTextRectangle()
    {
        // The rect is 144 by 108 pt at (468, 162); its txXfrm is 108 by 54 pt at (504, 162).
        // Centred in the shape the run would start near 468 + (144 - width) / 2; centred in the
        // stated rectangle it starts near 504 + (108 - width) / 2, some 33 pt further right.
        GlyphRun run = OnlyRun(Shape(2));

        run.Origin.X.Points.ShouldBeGreaterThan(525);
        run.Origin.X.Points.ShouldBeLessThan(545);

        // And vertically: anchor="ctr" centres in a 54 pt band starting at 162, so the baseline
        // is near 162 + 27, not near 162 + 54.
        run.Origin.Y.Points.ShouldBeGreaterThan(180);
        run.Origin.Y.Points.ShouldBeLessThan(200);
    }

    /// <summary>A baked shape with no text body produces no text.</summary>
    /// <remarks>
    /// The connectors between a diagram's nodes are shapes like any other and carry neither a
    /// <c>dsp:txBody</c> nor a <c>dsp:txXfrm</c>. A reader that synthesised an empty text body
    /// for them would lay out nothing and cost nothing, which is why this is easy to leave
    /// broken; it matters because the fallback for a missing text rectangle is the preset's, and
    /// a preset lookup on a shape that has no text is work that can only produce a wrong answer.
    /// </remarks>
    [Fact]
    public void AConnectorHasGeometryAndNoText()
    {
        PlacedShape connector = Shape(3);

        connector.Text.ShouldBeNull();
        connector.Outline.Commands.Count.ShouldBeGreaterThan(0);
    }

    /// <summary>
    /// Extraction still reads the data model, and reads each node's text exactly once.
    /// </summary>
    /// <remarks>
    /// The two paths disagree on purpose. The baked tree is what the author sees, so it repeats a
    /// node's text wherever the layout drew it and adds text the layout generated; the data model
    /// is what the author typed, once each. An index wants the second, so drawing from the baked
    /// tree must not change what extraction returns.
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

    private static GlyphRun OnlyRun(PlacedShape shape)
    {
        PlacedText text = shape.Text.ShouldNotBeNull();
        text.Runs.Count.ShouldBe(1);
        return text.Runs[0].Run;
    }

    private static PlacedShape Shape(int index)
    {
        LaidOutSlide slide = Slide();
        slide.Shapes.Count.ShouldBeGreaterThan(index);
        return slide.Shapes[index];
    }

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

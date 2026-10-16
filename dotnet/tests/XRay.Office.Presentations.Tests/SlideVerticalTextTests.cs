using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// <c>a:bodyPr/@vert</c>: a body that reads down the shape rather than across it.
/// </summary>
/// <remarks>
/// <para>
/// Measured against 26.2.4.2 on an authored deck rather than read out of the importer, because
/// the importer sends the three turning values down three different property paths and two of
/// them turn out to draw the same thing. <c>vert</c> becomes <c>WritingMode2::TB_RL90</c> and
/// <c>eaVert</c> becomes <c>TB_RL</c> with a swapped pair of adjusts
/// (<c>oox/source/drawingml/textbodypropertiescontext.cxx:126-200</c>) — and on Latin text the
/// reference draws <b>165 identical glyph matrices at identical positions</b> for the two, over
/// all three anchors. <c>vert270</c> is the same turn the other way.
/// </para>
/// <para>
/// The other two values of <c>ST_TextVerticalType</c> are deliberately not turns and that was
/// measured on the same deck: <c>mongolianVert</c> draws <b>horizontally</b> — the importer's own
/// comment says the rendering is not implemented for shape text — and <c>wordArtVert</c> stacks
/// one upright glyph per line. Neither appears on any slide part of the 302-document corpus, and
/// this reader draws both horizontally, which is right for the first and wrong for the second.
/// </para>
/// <para>
/// <c>slide-text-vertical.pptx</c> is three identical 230 × 160 pt boxes carrying one 14 pt word
/// each, at <c>horz</c>, <c>vert</c> and <c>vert270</c>, with <b>asymmetric insets</b> of
/// 10 / 20 / 30 / 40 pt. The asymmetry is the whole design: with the DrawingML defaults, or with
/// any symmetric quadruple, a reader that forgets to rotate the insets is indistinguishable from
/// one that remembers. `TextBodyProperties::pushTextDistances` rotates them, and it rotates them
/// in the same direction as the turn.
/// </para>
/// </remarks>
public class SlideVerticalTextTests
{
    /// <summary>A twentieth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.05;

    private const string Deck = "slide-text-vertical.pptx";

    /// <summary>The box, in points: the same for all three shapes but for the step across.</summary>
    private const double X0 = 200000.0 / 12700, Y0 = 300000.0 / 12700;
    private const double Width = 230, Height = 160, Step = 3000000.0 / 12700;

    private const double LeftInset = 10, TopInset = 20, RightInset = 30, BottomInset = 40;

    /// <summary>An upright body is not turned, which is the control for the two below.</summary>
    [Fact]
    public void AHorizontalBodyIsNotTurned()
    {
        Matrix(0).ShouldBe((1.0, 0.0, 0.0, 1.0));

        DocPoint origin = Origin(0);
        origin.X.Points.ShouldBe(X0 + LeftInset, TolerancePoints);
    }

    /// <summary><c>vert</c> turns the writing a quarter clockwise.</summary>
    [Fact]
    public void AVerticalBodyTurnsClockwise()
    {
        (double a, double b, double c, double d) = Matrix(1);

        // A quarter turn clockwise on a y-down page: (1,0) becomes (0,1).
        a.ShouldBe(0, 1e-9);
        b.ShouldBe(1, 1e-9);
        c.ShouldBe(-1, 1e-9);
        d.ShouldBe(0, 1e-9);
    }

    /// <summary><c>vert270</c> turns it the other way.</summary>
    [Fact]
    public void AVertical270BodyTurnsAnticlockwise()
    {
        (double a, double b, double c, double d) = Matrix(2);

        a.ShouldBe(0, 1e-9);
        b.ShouldBe(-1, 1e-9);
        c.ShouldBe(1, 1e-9);
        d.ShouldBe(0, 1e-9);
    }

    /// <summary>
    /// The turned writing starts where the reference starts it, insets and all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Stated as the three origins against each other rather than as three absolute points,
    /// so nothing here depends on the face's ascent — the one quantity in the arithmetic that
    /// belongs to the machine rather than to the file. The reference's own numbers, for the
    /// record: the three run origins are (25.739, 57.600), (437.953, 43.597) and
    /// (512.164, 143.575) in slide points, and ours land on them to 0.045.
    /// </para>
    /// <para>
    /// What each assertion would catch: the first, a turn that keeps the untransposed box or
    /// takes its inset from the wrong edge; the second, a turn about the wrong point; the
    /// third and fourth, the inset rotation running the wrong way — which is the failure a
    /// symmetric fixture cannot see.
    /// </para>
    /// </remarks>
    [Fact]
    public void ATurnedBodyStartsWhereTheReferenceStartsIt()
    {
        double ascent = Origin(0).Y.Points - (Y0 + TopInset);
        ascent.ShouldBeGreaterThan(0);

        DocPoint clockwise = Origin(1);
        DocPoint anticlockwise = Origin(2);

        // vert: down the right-hand side, in by rIns; along the flow, in by tIns.
        clockwise.X.Points.ShouldBe(
            X0 + Step + Width - RightInset - ascent, TolerancePoints);
        clockwise.Y.Points.ShouldBe(Y0 + TopInset, TolerancePoints);

        // vert270: up the left-hand side, in by lIns; along the flow, in by bIns.
        anticlockwise.X.Points.ShouldBe(
            X0 + (2 * Step) + LeftInset + ascent, TolerancePoints);
        anticlockwise.Y.Points.ShouldBe(Y0 + Height - BottomInset, TolerancePoints);
    }

    /// <summary>The turn moves the writing and leaves the shape where it is.</summary>
    [Fact]
    public void TheThreeShapesKeepTheSameFootprint()
    {
        (double left, double top, double right, double bottom) upright = Drawn(0);

        foreach (int index in (int[])[1, 2])
        {
            (double left, double top, double right, double bottom) turned = Drawn(index);

            (turned.right - turned.left).ShouldBe(upright.right - upright.left, TolerancePoints);
            (turned.bottom - turned.top).ShouldBe(upright.bottom - upright.top, TolerancePoints);
            turned.top.ShouldBe(upright.top, TolerancePoints);
            (turned.left - upright.left).ShouldBe(index * Step, TolerancePoints);
        }
    }

    private static (double A, double B, double C, double D) Matrix(int index)
    {
        AffineTransform m = Text(index).Transform;
        return (m.A, m.B, m.C, m.D);
    }

    /// <summary>Where the shape's only run starts, in slide coordinates.</summary>
    private static DocPoint Origin(int index)
    {
        PlacedText text = Text(index);
        text.Runs.Count.ShouldBe(1);
        return ShapeTransform.Apply(text.Transform, text.Runs[0].Run.Origin);
    }

    private static (double Left, double Top, double Right, double Bottom) Drawn(int index)
    {
        IReadOnlyList<PathCommand> commands = Shape(index).Outline.Commands;
        commands.Count.ShouldBeGreaterThan(0);

        double left = double.MaxValue, top = double.MaxValue;
        double right = double.MinValue, bottom = double.MinValue;

        foreach (PathCommand command in commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            left = Math.Min(left, command.Point.X.Points);
            right = Math.Max(right, command.Point.X.Points);
            top = Math.Min(top, command.Point.Y.Points);
            bottom = Math.Max(bottom, command.Point.Y.Points);
        }

        return (left, top, right, bottom);
    }

    private static PlacedText Text(int index) => Shape(index).Text.ShouldNotBeNull();

    private static PlacedShape Shape(int index)
    {
        LaidOutSlide slide = Slide();
        slide.Shapes.Count.ShouldBe(3);
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

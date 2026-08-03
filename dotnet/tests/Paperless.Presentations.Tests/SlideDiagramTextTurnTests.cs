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
/// A SmartArt shape whose <c>dsp:txXfrm</c> turns its text area against its own rotation.
/// </summary>
/// <remarks>
/// <para>
/// A diagram lays shapes on their sides constantly — a chevron pointing down a column, an arrow
/// between two stacked boxes — and it does that with an ordinary <c>&lt;a:xfrm rot="5400000"/&gt;</c>
/// on the shape. The writing is not meant to turn with it, so the same shape's
/// <c>dsp:txXfrm</c> states <c>rot="-5400000"</c>: an angle stated <em>against</em> the shape's,
/// not a second copy of it. The two add, and the sum is what the text is drawn at —
/// <c>oox/source/drawingml/transform2dcontext.cxx:53-58</c> puts the attribute into the same
/// field <c>a:bodyPr/@rot</c> feeds, adding rather than replacing, and
/// <c>transform2dcontext.cxx:341-344</c> reads the sum back as "the rotation beyond compensation
/// of the shape rotation".
/// </para>
/// <para>
/// Ignoring the attribute leaves the shape's own quarter turn on the text, which is one of the
/// most visible defects this corpus has produced and one no page-count or shape check can see:
/// on <c>slides/batch-015/pptx/schematicplay.pptx</c> the eight diagram shapes drew in exactly
/// the right places while every word inside them stacked into a vertical band down the middle of
/// the slide, where the overlapping glyphs collapsed under extraction — 2040 extractable words
/// against LibreOffice's 2129.
/// </para>
/// <para>
/// <c>slide-diagram-text-turned.pptx</c> is the two cases side by side and nothing else: two
/// 2 × 1 inch <c>rect</c> shapes with the same 14 pt "Turn" in them, the second reaching its
/// footprint by laying a 1 × 2 inch box on its side. Both rotations are about the same centre,
/// so a reader that honours the text area's turn draws the second exactly 180 pt to the right of
/// the first — and one that does not draws it on its side, 36 pt off in each direction because a
/// transposed box centres its text somewhere else.
/// </para>
/// </remarks>
public class SlideDiagramTextTurnTests
{
    /// <summary>A twentieth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.05;

    private const string Deck = "slide-diagram-text-turned.pptx";

    /// <summary>How far apart the two text rectangles are stated to be, in points.</summary>
    private const double Apart = 180;

    /// <summary>
    /// The turned shape's text is drawn upright, because the two quarter turns cancel.
    /// </summary>
    /// <remarks>
    /// The matrix rather than the position, because this is the half of the defect a reader can
    /// get wrong on its own: text laid out in the right rectangle and then turned with the shape
    /// is still unreadable, and still collapses under extraction.
    /// </remarks>
    [Fact]
    public void TheTurnedShapeDrawsItsTextUpright()
    {
        AffineTransform matrix = Text(1).Transform;

        matrix.A.ShouldBe(1, 1e-9);
        matrix.D.ShouldBe(1, 1e-9);
        matrix.B.ShouldBe(0, 1e-9);
        matrix.C.ShouldBe(0, 1e-9);
    }

    /// <summary>
    /// It lands in the rectangle the file states, 180 pt right of the identical upright one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both shapes state a 2 × 1 inch text area with the same text, the same face and the same
    /// centring, so every quantity that decides where a glyph goes is shared and the two origins
    /// can differ only by the 180 pt between the rectangles.
    /// </para>
    /// <para>
    /// <strong>The box turns; it is not transposed.</strong> LibreOffice scales the text box to
    /// the stated width and height first and rotates the result about its centre
    /// (<c>svx/source/sdr/contact/viewcontactofsdrobjcustomshape.cxx:168-191</c>), so the lines
    /// break at the stated width; only <c>TextPreRotateAngle</c> — what a diagram's <c>upr</c>
    /// and <c>grav</c> produce — is applied before the scale and so reshapes the box. A reader
    /// that transposed here would centre "Turn" in a 1 × 2 inch box and miss by 36 pt each way,
    /// which is what these tolerances are tight enough to catch.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheTurnedShapeTextLandsInTheStatedRectangle()
    {
        DocPoint upright = Origin(0);
        DocPoint turned = Origin(1);

        (turned.X.Points - upright.X.Points).ShouldBe(Apart, TolerancePoints);
        turned.Y.Points.ShouldBe(upright.Y.Points, TolerancePoints);
    }

    /// <summary>The turn moves the writing and not the shape.</summary>
    /// <remarks>
    /// The two boxes are the same size and 180 pt apart on the page whatever the text does, so
    /// this is what says a failure above is about the text area rather than about the placement
    /// the text area is measured against.
    /// </remarks>
    [Fact]
    public void BothShapesOccupyTheSameFootprintApartFromTheOffset()
    {
        (double left, double top, double right, double bottom) upright = Drawn(0);
        (double left, double top, double right, double bottom) turned = Drawn(1);

        (turned.right - turned.left).ShouldBe(upright.right - upright.left, TolerancePoints);
        (turned.bottom - turned.top).ShouldBe(upright.bottom - upright.top, TolerancePoints);
        turned.top.ShouldBe(upright.top, TolerancePoints);
        (turned.left - upright.left).ShouldBe(Apart, TolerancePoints);
    }

    /// <summary>The box a shape's outline actually covers on the slide, in points.</summary>
    private static (double Left, double Top, double Right, double Bottom) Drawn(int index)
    {
        IReadOnlyList<PathCommand> commands = Shape(index).Outline.Commands;
        commands.Count.ShouldBeGreaterThan(0);

        double left = double.MaxValue, top = double.MaxValue;
        double right = double.MinValue, bottom = double.MinValue;

        foreach (PathCommand command in commands)
        {
            // Close carries no point, and counting its default would put every path's corner at
            // the slide's origin.
            if (command.Verb == PathVerb.Close) continue;

            left = Math.Min(left, command.Point.X.Points);
            right = Math.Max(right, command.Point.X.Points);
            top = Math.Min(top, command.Point.Y.Points);
            bottom = Math.Max(bottom, command.Point.Y.Points);
        }

        return (left, top, right, bottom);
    }

    /// <summary>Where the shape's only run starts, in slide coordinates.</summary>
    private static DocPoint Origin(int index)
    {
        PlacedText text = Text(index);
        text.Runs.Count.ShouldBe(1);
        return ShapeTransform.Apply(text.Transform, text.Runs[0].Run.Origin);
    }

    private static PlacedText Text(int index) => Shape(index).Text.ShouldNotBeNull();

    private static PlacedShape Shape(int index)
    {
        LaidOutSlide slide = Slide();
        slide.Shapes.Count.ShouldBe(2);
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

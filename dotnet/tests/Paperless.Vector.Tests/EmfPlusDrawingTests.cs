using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// What an EMF+ stream draws, one record at a time.
/// </summary>
/// <remarks>
/// <para>
/// Every fixture is built by <see cref="EmfPlusBuilder"/> inside an <see cref="EmfBuilder"/> whose
/// reference device is 8000 pixels to 80 mm — so one EMF+ world unit is exactly 1/100 mm and an
/// assertion can name the millimetre it expects. A test that has to reason about a scale factor
/// is testing the test.
/// </para>
/// <para>
/// The picture is drawn at its intrinsic size, so the recorded coordinates reach
/// <see cref="Recorder"/> unscaled.
/// </para>
/// </remarks>
public sealed class EmfPlusDrawingTests
{
    private const uint Red = 0xFFFF0000;
    private const uint Blue = 0xFF0000FF;
    private const uint Green = 0xFF00FF00;

    [Fact]
    public void EachRectangleOfAFillRectsRecordIsFilledInItsOwnRight()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .FillRects(Red, (1000, 1000, 2000, 1000), (2000, 1500, 2000, 1000))
            .End());

        // Emitted one at a time rather than as one path: under the even-odd rule a single path
        // would cancel the overlap and leave a hole where the two squares meet.
        sink.Fills.Count.ShouldBe(2);

        sink.Fills[0].Bounds.X.Millimetres.ShouldBe(10, 0.01);
        sink.Fills[0].Bounds.Width.Millimetres.ShouldBe(20, 0.01);
        sink.Fills[0].Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(255, 0, 0));
        sink.Fills[1].Bounds.X.Millimetres.ShouldBe(20, 0.01);
    }

    [Fact]
    public void AColourStatedOnTheRecordCarriesItsAlphaChannel()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .FillRects(0x80FF0000, (0, 0, 1000, 1000))
            .End());

        // GDI has no alpha at all, so a reader carried over from the EMF side that masks the top
        // byte away draws every translucent highlight in a chart as an opaque block.
        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<SolidPaint>().Colour.A.ShouldBe((byte)0x80);
    }

    [Fact]
    public void ABrushIsNamedByItsSlotRatherThanByValue()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .SolidBrush(3, Blue)
            .FillRectsWithBrush(3, (0, 0, 1000, 1000))
            .End());

        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0, 0, 255));
    }

    [Fact]
    public void AnObjectSlotIsOverwrittenInPlaceWithNoDeleteAndNoHandleArithmetic()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .SolidBrush(7, Blue)
            .SolidBrush(7, Green)
            .FillRectsWithBrush(7, (0, 0, 1000, 1000))
            .End());

        // Unlike GDI's table, which assigns the lowest free handle and has a delete, an EMF+ slot
        // is simply written over — so the second brush is the one that draws.
        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0, 255, 0));
    }

    [Fact]
    public void ARecordThatNamesAnEmptySlotDrawsNothingRatherThanFailing()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .FillRectsWithBrush(12, (0, 0, 1000, 1000))
            .End());

        sink.Fills.ShouldBeEmpty();
    }

    // ---------------------------------------------------------------- paths

    [Fact]
    public void APathsPointTypesDecideWhereItsFiguresBeginAndEnd()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Path(1, [(0, 0), (1000, 0), (1000, 1000), (0, 0), (2000, 2000), (3000, 2000)], [0, 1, 1, 0x81, 0, 1])
            .FillPath(1, Red)
            .End());

        // Two figures: the closed triangle and the open line, in one path.
        sink.Fills.ShouldHaveSingleItem().Bounds.Width.Millimetres.ShouldBe(30, 0.01);
    }

    [Fact]
    public void ABezierPointIsPlacedByItsDistancePastTheLastOnCurvePoint()
    {
        VectorImage image = Build(new EmfPlusBuilder()
            .Header()
            .Path(1, [(0, 0), (0, 2000), (2000, 2000), (2000, 0)], [0, 3, 3, 3])
            .FillPath(1, Red)
            .End());

        CountingSink sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));

        // Four points, one of them the start: exactly one cubic segment, not three lines through
        // the control points — which is what a decoder that reads the type rather than the
        // position produces.
        sink.Cubics.ShouldBe(1);
        sink.Lines.ShouldBe(0);
    }

    [Fact]
    public void RelativePointsAreAccumulatedRatherThanTakenAsCoordinates()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Pen(1, Red, 10)
            .DrawLines(1, [(1000, 1000), (1050, 1000), (1100, 1050), (1150, 1050)], encoding: 0x800)
            .End());

        // Each point is a delta from the one before. Taken as coordinates the run would be 50
        // units across and start at the origin; accumulated it is 150 units across and starts at
        // 1000. LibreOffice takes them as coordinates (emfppath.cxx's 0x800 branch).
        DocRect bounds = sink.Strokes.ShouldHaveSingleItem().Bounds;

        bounds.X.Millimetres.ShouldBe(10, 0.02);
        bounds.Width.Millimetres.ShouldBe(1.5, 0.02);
    }

    [Fact]
    public void CompressedPointsAreSixteenBitIntegersRatherThanFloats()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Pen(1, Red, 10)
            .DrawLines(1, [(1000, 1000), (3000, 2000)], encoding: 0x4000)
            .End());

        DocRect bounds = sink.Strokes.ShouldHaveSingleItem().Bounds;

        bounds.X.Millimetres.ShouldBe(10, 0.02);
        bounds.Width.Millimetres.ShouldBe(20, 0.02);
    }

    [Fact]
    public void TheClosingFlagOnADrawLinesRecordAddsTheLineBackToTheStart()
    {
        Recorder open = Draw(new EmfPlusBuilder()
            .Header().Pen(1, Red, 10)
            .DrawLines(1, [(0, 0), (1000, 0), (1000, 1000)])
            .End());

        Recorder closed = Draw(new EmfPlusBuilder()
            .Header().Pen(1, Red, 10)
            .DrawLines(1, [(0, 0), (1000, 0), (1000, 1000)], close: true)
            .End());

        open.Strokes.ShouldHaveSingleItem();
        closed.Strokes.ShouldHaveSingleItem();
        closed.Strokes[0].Bounds.ShouldBe(open.Strokes[0].Bounds);
    }

    // ---------------------------------------------------------------- brushes

    [Fact]
    public void AHatchBrushPaintsItsOwnBackgroundBeforeItsLines()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .HatchBrush(1, 0x03, Red, Blue)               // backward diagonal
            .FillRectsWithBrush(1, (0, 0, 4000, 4000))
            .End());

        // A GDI+ hatch states both colours and is opaque, unlike GDI's, which takes its
        // background from the device context.
        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0, 0, 255));
        sink.Strokes.Count.ShouldBeGreaterThan(4);
        sink.Strokes[0].Stroke.Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(255, 0, 0));
    }

    [Fact]
    public void APercentageHatchIsBlendedRatherThanStroked()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .HatchBrush(1, 0x0C, 0xFF000000, 0xFFFFFFFF)  // fifty per cent
            .FillRectsWithBrush(1, (0, 0, 4000, 4000))
            .End());

        // The twelve percentage styles are dot screens with no line form at all, so the average
        // of the two colours is what they come to — which is what LibreOffice draws for them too.
        sink.Strokes.ShouldBeEmpty();

        Colour filled = sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<SolidPaint>().Colour;
        filled.R.ShouldBeInRange((byte)126, (byte)129);
    }

    [Fact]
    public void ALinearGradientRunsBetweenTheEndsOfItsOwnRectangle()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .LinearBrush(1, (1000, 1000, 2000, 1000), Red, Blue)
            .FillRectsWithBrush(1, (1000, 1000, 2000, 1000))
            .End());

        GradientPaint gradient = sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<GradientPaint>();

        gradient.Kind.ShouldBe(GradientKind.Linear);
        gradient.Stops.Count.ShouldBe(2);
        gradient.Stops[0].Colour.ShouldBe(new Colour(255, 0, 0));
        gradient.Start.X.Millimetres.ShouldBe(10, 0.01);
        gradient.End.X.Millimetres.ShouldBe(30, 0.01);
    }

    [Fact]
    public void AGradientThatRepeatsPastItsOwnRectangleSaysSo()
    {
        VectorImage covering = Build(new EmfPlusBuilder()
            .Header()
            .LinearBrush(1, (0, 0, 4000, 4000), Red, Blue)
            .FillRectsWithBrush(1, (0, 0, 4000, 4000))
            .End());

        VectorImage repeating = Build(new EmfPlusBuilder()
            .Header()
            .LinearBrush(1, (0, 0, 400, 400), Red, Blue)
            .FillRectsWithBrush(1, (0, 0, 4000, 4000))
            .End());

        // A gradient that covers what it fills looks the same under every wrap mode, so the
        // diagnostic is scoped to the shape that actually reaches past the ramp — otherwise it
        // would fire on nearly every gradient in every file.
        covering.Diagnostics.ShouldNotContain(diagnostic => diagnostic.Code == "PL6041");
        repeating.Diagnostics.ShouldContain(diagnostic => diagnostic.Code == "PL6041");
    }

    // ---------------------------------------------------------------- pens

    [Fact]
    public void APensOptionalFieldsArePositionalAndTheBrushSitsAfterAllOfThem()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Pen(1, Green, 50, join: 2, dashes: [3f, 1f], startCap: 2)
            .DrawLines(1, [(0, 0), (4000, 0)])
            .End());

        Stroke stroke = sink.Strokes.ShouldHaveSingleItem().Stroke;

        // If any one of the three optional fields were read in the wrong order the brush at the
        // end would land on the dash array and the line would come out an arbitrary colour.
        stroke.Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0, 255, 0));
        stroke.Join.ShouldBe(LineJoin.Round);
        stroke.Cap.ShouldBe(LineCap.Round);
        stroke.DashPattern.ShouldNotBeNull().Count.ShouldBe(2);
    }

    [Fact]
    public void APenWithNoOptionalFieldsIsSolidAndMitred()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Pen(1, Red, 100)
            .DrawLines(1, [(0, 0), (4000, 0)])
            .End());

        Stroke stroke = sink.Strokes.ShouldHaveSingleItem().Stroke;

        stroke.DashPattern.ShouldBeNull();
        stroke.Join.ShouldBe(LineJoin.Miter);
        stroke.Width.Millimetres.ShouldBe(1, 0.01);
    }

    [Fact]
    public void APenWidthIsMeasuredInTheSpaceTheWorldTransformMapsFrom()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Pen(1, Red, 100)
            .Scale(3, 3)
            .DrawLines(1, [(0, 0), (1000, 0)])
            .End());

        // A pen's width scales with the picture, unlike a GDI cosmetic pen's.
        sink.Strokes.ShouldHaveSingleItem().Stroke.Width.Millimetres.ShouldBe(3, 0.01);
    }

    [Fact]
    public void ACustomLineCapIsReportedRatherThanApproximated()
    {
        VectorImage image = Build(new EmfPlusBuilder()
            .Header()
            .Pen(1, Red, 100, startCap: 0x14)             // an arrow anchor
            .DrawLines(1, [(0, 0), (1000, 0)])
            .End());

        image.Diagnostics.ShouldContain(diagnostic => diagnostic.Code == "PL6038");
    }

    // ---------------------------------------------------------------- transforms

    [Fact]
    public void TheWorldTransformMovesWhatIsDrawnAfterIt()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Translate(1000, 500)
            .FillRects(Red, (0, 0, 1000, 1000))
            .End());

        sink.Fills.ShouldHaveSingleItem().Bounds.X.Millimetres.ShouldBe(10, 0.01);
        sink.Fills[0].Bounds.Y.Millimetres.ShouldBe(5, 0.01);
    }

    [Fact]
    public void PreAndPostMultiplyingATransformAreDifferentPictures()
    {
        Recorder pre = Draw(new EmfPlusBuilder()
            .Header().Translate(1000, 0).Scale(2, 2)
            .FillRects(Red, (0, 0, 500, 500))
            .End());

        Recorder post = Draw(new EmfPlusBuilder()
            .Header().Translate(1000, 0).Scale(2, 2, post: true)
            .FillRects(Red, (0, 0, 500, 500))
            .End());

        // Pre-multiplying applies the scale first, so the translation is unscaled; post-multiplying
        // applies it last and scales the translation with everything else.
        pre.Fills.ShouldHaveSingleItem().Bounds.X.Millimetres.ShouldBe(10, 0.01);
        post.Fills.ShouldHaveSingleItem().Bounds.X.Millimetres.ShouldBe(20, 0.01);
    }

    [Fact]
    public void ResetWorldTransformPutsTheIdentityBack()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Translate(3000, 3000)
            .ResetWorld()
            .FillRects(Red, (0, 0, 1000, 1000))
            .End());

        sink.Fills.ShouldHaveSingleItem().Bounds.X.Millimetres.ShouldBe(0, 0.01);
    }

    [Fact]
    public void APageTransformScalesEverythingBetweenWorldSpaceAndTheDevice()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .PageTransform(2f, 2)                         // twice, in pixels
            .FillRects(Red, (0, 0, 1000, 1000))
            .End());

        sink.Fills.ShouldHaveSingleItem().Bounds.Width.Millimetres.ShouldBe(20, 0.01);
    }

    [Fact]
    public void SaveAndRestoreAreKeyedByIndexRatherThanStacked()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Translate(1000, 0)
            .Save(1)
            .Translate(1000, 0)
            .Save(2)
            .Translate(1000, 0)
            .Restore(1)
            .FillRects(Red, (0, 0, 500, 500))
            .End());

        // A stack would pop the state saved under 2; the file said 1, and PowerPoint's output
        // reuses indexes freely.
        sink.Fills.ShouldHaveSingleItem().Bounds.X.Millimetres.ShouldBe(10, 0.01);
    }

    [Fact]
    public void AContainerRestoresTheTransformItWasOpenedWith()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .BeginContainer(1)
            .Translate(2000, 0)
            .EndContainer(1)
            .FillRects(Red, (0, 0, 500, 500))
            .End());

        sink.Fills.ShouldHaveSingleItem().Bounds.X.Millimetres.ShouldBe(0, 0.01);
    }

    // ---------------------------------------------------------------- clipping

    [Fact]
    public void SetClipRectNarrowsWhatIsDrawnAfterIt()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .ClipRect((1000, 1000, 2000, 2000), mode: 0)
            .FillRects(Red, (0, 0, 8000, 6000))
            .End());

        // The frame's own clip plus the record's, which is the painter's lazy re-emission.
        sink.Clips.Count.ShouldBe(2);
        sink.Clips[1].X.Millimetres.ShouldBe(10, 0.01);
        sink.Clips[1].Width.Millimetres.ShouldBe(20, 0.01);
    }

    [Fact]
    public void ResetClipWidensItAgain()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .ClipRect((1000, 1000, 2000, 2000), mode: 0)
            .ResetClip()
            .FillRects(Red, (0, 0, 8000, 6000))
            .End());

        sink.Clips.Count.ShouldBe(1);
        sink.Fills.ShouldHaveSingleItem().Bounds.Width.Millimetres.ShouldBe(80, 0.01);
    }

    [Fact]
    public void AnExcludedRectangleLeavesTheRestOfTheClipExact()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .ClipRect((0, 0, 4000, 4000), mode: 1)
            .ClipRect((1000, 1000, 1000, 1000), mode: 4)
            .FillRects(Red, (0, 0, 8000, 6000))
            .End());

        // A rectangle minus a rectangle is at most four rectangles, so subtraction stays exact
        // without any path arithmetic.
        VectorImage image = Build(new EmfPlusBuilder()
            .Header()
            .ClipRect((0, 0, 4000, 4000), mode: 1)
            .ClipRect((1000, 1000, 1000, 1000), mode: 4)
            .FillRects(Red, (0, 0, 8000, 6000))
            .End());

        image.Diagnostics.ShouldNotContain(diagnostic => diagnostic.Code == "PL6034");
        sink.Fills.ShouldHaveSingleItem();
    }

    // ---------------------------------------------------------------- shapes

    [Fact]
    public void AnEllipseIsBuiltInTheFilesOwnSpaceAndMappedAfterwards()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .FillEllipse(Red, (1000, 1000, 2000, 1000))
            .End());

        DocRect bounds = sink.Fills.ShouldHaveSingleItem().Bounds;

        bounds.X.Millimetres.ShouldBe(10, 0.05);
        bounds.Width.Millimetres.ShouldBe(20, 0.05);
        bounds.Height.Millimetres.ShouldBe(10, 0.05);
    }

    [Fact]
    public void APieSweepsFromItsStatedAngleAndClosesThroughTheCentre()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .FillPie(Red, (0, 0, 4000, 4000), 0, 90)
            .End());

        DocRect bounds = sink.Fills.ShouldHaveSingleItem().Bounds;

        // Zero degrees is the positive x axis and the sweep is clockwise because y runs down, so
        // the quadrant covered is the lower-right one — from the centre to the far corner.
        bounds.X.Millimetres.ShouldBe(20, 0.1);
        bounds.Y.Millimetres.ShouldBe(20, 0.1);
        bounds.Width.Millimetres.ShouldBe(20, 0.1);
    }

    [Fact]
    public void AFillPolygonIsClosedWhetherOrNotItSaysSo()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .FillPolygon(Red, (0, 0), (2000, 0), (2000, 2000))
            .End());

        sink.Fills.ShouldHaveSingleItem().Bounds.Width.Millimetres.ShouldBe(20, 0.01);
    }

    [Fact]
    public void DrawRectsStrokesEveryRectangleInOnePath()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Pen(1, Red, 10)
            .DrawRects(1, (0, 0, 1000, 1000), (3000, 0, 1000, 1000))
            .End());

        sink.Strokes.ShouldHaveSingleItem().Bounds.Width.Millimetres.ShouldBe(40, 0.05);
    }

    // ---------------------------------------------------------------- text

    [Fact]
    public void ADrawStringPlacesItsBaselineBelowTheLayoutRectanglesTop()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Font(1, "Arial", 400)
            .DrawString(1, 0xFF102030, 0xFF, "Paperless", (1000, 1000, 6000, 1000))
            .End());

        (string text, DocPoint origin, Length size, _, Paint paint) = sink.Runs.ShouldHaveSingleItem();

        text.ShouldBe("Paperless");
        sink.GlyphRuns.ShouldHaveSingleItem().Font.RequestedFamily.ShouldBe("Arial");
        size.Millimetres.ShouldBe(4, 0.01);
        origin.X.Millimetres.ShouldBe(10, 0.01);

        // The baseline sits one ascent below the rectangle's top, not on it.
        origin.Y.Millimetres.ShouldBeGreaterThan(12.5);
        origin.Y.Millimetres.ShouldBeLessThan(14.5);
        paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0x10, 0x20, 0x30));
    }

    [Fact]
    public void AStringFormatCentresTheTextInItsLayoutRectangle()
    {
        Recorder near = Draw(new EmfPlusBuilder()
            .Header().Font(1, "Arial", 400).StringFormat(2, 0)
            .DrawString(1, 0xFF000000, 2, "Paperless", (0, 0, 6000, 1000))
            .End());

        Recorder centred = Draw(new EmfPlusBuilder()
            .Header().Font(1, "Arial", 400).StringFormat(2, 1)
            .DrawString(1, 0xFF000000, 2, "Paperless", (0, 0, 6000, 1000))
            .End());

        Recorder far = Draw(new EmfPlusBuilder()
            .Header().Font(1, "Arial", 400).StringFormat(2, 2)
            .DrawString(1, 0xFF000000, 2, "Paperless", (0, 0, 6000, 1000))
            .End());

        near.Runs.ShouldHaveSingleItem().Origin.X.Millimetres.ShouldBe(0, 0.01);
        centred.Runs.ShouldHaveSingleItem().Origin.X.Millimetres.ShouldBeGreaterThan(5);
        far.Runs.ShouldHaveSingleItem().Origin.X.Millimetres.ShouldBeGreaterThan(
            centred.Runs[0].Origin.X.Millimetres + 4);
    }

    [Fact]
    public void TrackingWidensEveryAdvanceRatherThanTheWholeRun()
    {
        Recorder plain = Draw(new EmfPlusBuilder()
            .Header().Font(1, "Arial", 400).StringFormat(2, 0)
            .DrawString(1, 0xFF000000, 2, "Paperless", (0, 0, 6000, 1000))
            .End());

        Recorder tracked = Draw(new EmfPlusBuilder()
            .Header().Font(1, "Arial", 400).StringFormat(2, 0, tracking: 2f)
            .DrawString(1, 0xFF000000, 2, "Paperless", (0, 0, 6000, 1000))
            .End());

        Length narrow = Width(plain.GlyphRuns.ShouldHaveSingleItem());
        Length wide = Width(tracked.GlyphRuns.ShouldHaveSingleItem());

        wide.Emu.ShouldBeGreaterThan((long)(narrow.Emu * 1.8));
    }

    [Fact]
    public void AFontsStyleBitsReachTheResolvedFace()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .Font(1, "Arial", 400, style: 1)
            .DrawString(1, 0xFF000000, 0xFF, "Bold", (0, 0, 6000, 1000))
            .End());

        sink.GlyphRuns.ShouldHaveSingleItem().Font.Weight.ShouldBe(700);
    }

    // ---------------------------------------------------------------- objects

    [Fact]
    public void AnObjectSplitOverSeveralRecordsIsAssembledBeforeItIsParsed()
    {
        (float X, float Y)[] points = new (float, float)[64];
        byte[] types = new byte[64];

        for (int i = 0; i < points.Length; i++)
        {
            points[i] = (i * 60, (i % 2) * 1000);
            types[i] = (byte)(i == 0 ? 0 : 1);
        }

        EmfPlusBuilder whole = new EmfPlusBuilder().Header();
        EmfPlusBuilder parts = new EmfPlusBuilder().Header();

        whole.Path(1, points, types);

        // The same object, written in 64-byte fragments — which is how a texture brush carrying a
        // photograph is written, and what a reader that parses each fragment alone gets wrong.
        byte[] bytes = Fragmentable(points, types);
        parts.Object(1, 0x300, bytes, split: 64);

        Recorder one = Draw(whole.FillPath(1, Red).End());
        Recorder many = Draw(parts.FillPath(1, Red).End());

        many.Fills.ShouldHaveSingleItem().Bounds.ShouldBe(one.Fills.ShouldHaveSingleItem().Bounds);
    }

    // ---------------------------------------------------------------- dual mode

    [Fact]
    public void TheGdiRecordsAreLeftAloneOnceAnEmfPlusRecordHasBeenSeen()
    {
        Recorder sink = Draw(new EmfBuilder()
            .Plus(new EmfPlusBuilder().Header(dual: true).FillRects(Red, (0, 0, 1000, 1000)).End())
            .SolidBrush(1, 0, 0, 255)
            .Select(1)
            .Rectangle(2000, 2000, 4000, 4000));

        // Replaying both descriptions would draw the whole picture twice; the GDI half of a dual
        // file is the same drawing again (emfreader.cxx:955-963).
        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(255, 0, 0));
    }

    [Fact]
    public void AnEmfPlusOnlyFileSkipsItsGdiRecordsJustTheSame()
    {
        Recorder sink = Draw(new EmfBuilder()
            .Plus(new EmfPlusBuilder().Header(dual: false).FillRects(Red, (0, 0, 1000, 1000)).End())
            .SolidBrush(1, 0, 0, 255)
            .Select(1)
            .Rectangle(2000, 2000, 4000, 4000));

        // The GDI half of an EMF+ Only file is a "this needs GDI+" notice rather than a drawing,
        // so the dual flag changes nothing about which records are replayed.
        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(255, 0, 0));
    }

    [Fact]
    public void GdiRecordsBeforeTheFirstEmfPlusRecordAreStillDrawn()
    {
        Recorder sink = Draw(new EmfBuilder()
            .SolidBrush(1, 0, 0, 255)
            .Select(1)
            .Rectangle(2000, 2000, 4000, 4000)
            .Plus(new EmfPlusBuilder().Header(dual: true).FillRects(Red, (0, 0, 1000, 1000)).End()));

        // A file that carries no EMF+ at all is untouched by any of this, and one whose EMF+
        // starts late still draws what came before it.
        sink.Fills.Count.ShouldBe(2);
        sink.Fills[0].Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0, 0, 255));
    }

    [Fact]
    public void GetDcHandsTheDeviceContextBackForTheGdiRecordsThatFollow()
    {
        Recorder sink = Draw(new EmfBuilder()
            .Plus(new EmfPlusBuilder().Header(dual: true).FillRects(Red, (0, 0, 1000, 1000)).GetDc())
            .SolidBrush(1, 0, 0, 255)
            .Select(1)
            .Rectangle(2000, 2000, 4000, 4000)
            .Plus(new EmfPlusBuilder().FillRects(Green, (5000, 5000, 500, 500)).End()));

        // The one bracket the format itself defines: GetDC exists for the records GDI+ has no
        // form for, and it lasts until the next comment.
        sink.Fills.Count.ShouldBe(3);
        sink.Fills[1].Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0, 0, 255));
    }

    [Fact]
    public void TheHandBackEndsAtTheNextComment()
    {
        Recorder sink = Draw(new EmfBuilder()
            .Plus(new EmfPlusBuilder().Header(dual: true).GetDc())
            .Plus(new EmfPlusBuilder().FillRects(Red, (0, 0, 1000, 1000)).End())
            .SolidBrush(1, 0, 0, 255)
            .Select(1)
            .Rectangle(2000, 2000, 4000, 4000));

        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(255, 0, 0));
    }

    // ---------------------------------------------------------------- helpers

    private static byte[] Fragmentable((float X, float Y)[] points, byte[] types)
    {
        List<byte> data = [];

        void Add32(uint value)
        {
            data.Add((byte)value);
            data.Add((byte)(value >> 8));
            data.Add((byte)(value >> 16));
            data.Add((byte)(value >> 24));
        }

        Add32(0xDBC01002);
        Add32((uint)points.Length);
        Add32(0);

        foreach ((float x, float y) in points)
        {
            Add32(BitConverter.SingleToUInt32Bits(x));
            Add32(BitConverter.SingleToUInt32Bits(y));
        }

        data.AddRange(types);
        return [.. data];
    }

    private static Length Width(GlyphRun run)
    {
        Length width = Length.Zero;
        foreach (PositionedGlyph glyph in run.Glyphs) width += glyph.Advance;
        return width;
    }

    private static VectorImage Build(EmfPlusBuilder plus) => Build(new EmfBuilder().Plus(plus));

    private static VectorImage Build(EmfBuilder emf) => emf.Decode();

    private static Recorder Draw(EmfPlusBuilder plus) => Draw(new EmfBuilder().Plus(plus));

    private static Recorder Draw(EmfBuilder emf)
    {
        VectorImage image = emf.Decode();
        Recorder sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return sink;
    }

    /// <summary>A sink that counts path verbs, for a test that has to see the curve itself.</summary>
    private sealed class CountingSink : IDrawingSink
    {
        public int Cubics { get; private set; }

        public int Lines { get; private set; }

        public void BeginPage(DocSize size) { }

        public void EndPage() { }

        public void Save() { }

        public void Restore() { }

        public void Transform(AffineTransform transform) { }

        public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero) { }

        public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero) => Count(path);

        public void StrokePath(GraphicsPath path, Stroke stroke) => Count(path);

        public void DrawGlyphRun(GlyphRun run, Paint paint) { }

        public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0) { }

        public void BeginTransparencyGroup(double opacity) { }

        public void EndTransparencyGroup() { }

        private void Count(GraphicsPath path)
        {
            foreach (PathCommand command in path.Commands)
            {
                if (command.Verb == PathVerb.CubicTo) Cubics++;
                if (command.Verb == PathVerb.LineTo) Lines++;
            }
        }
    }
}

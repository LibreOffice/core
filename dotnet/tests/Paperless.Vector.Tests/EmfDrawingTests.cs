using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector.Emf;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// What an EMF's records draw.
/// </summary>
/// <remarks>
/// Every fixture states a reference device of 8000 pixels to 80 mm, so one logical unit is
/// exactly 1/100 mm and an assertion can name the millimetre it expects. That separates a
/// mapping bug from a drawing bug: under this mapping any geometry error shows as a wrong
/// number rather than as a wrong scale.
/// </remarks>
public class EmfDrawingTests
{
    private const int Mm = 100;     // logical units per millimetre, under the fixtures' mapping

    [Fact]
    public void TheHeaderStatesTheCoordinateSpaceAndThePhysicalExtentIndependently()
    {
        // A frame of 80 x 60 mm over a bounds of 4000 x 3000 device units: the two disagree about
        // the scale on purpose, which is what tells a decoder that derives one from the other
        // apart from one that reads both.
        EmfBuilder builder = new()
        {
            Bounds = (0, 0, 3999, 2999),
            Frame = (0, 0, 7999, 5999),
            DevicePixels = (4000, 3000),
            DeviceMillimetres = (80, 60),
        };

        VectorImage image = builder
            .SolidBrush(1, 0x30, 0x60, 0x90)
            .Select(1)
            .Rectangle(0, 0, 2000, 1500)
            .Decode();

        // The physical extent is the frame, in 1/100 mm.
        image.IntrinsicSize.Width.Millimetres.ShouldBe(80, 0.01);
        image.IntrinsicSize.Height.Millimetres.ShouldBe(60, 0.01);

        // The coordinate space is the bounds, scaled by the reference device: 4000 device units
        // over 80 mm is 2/100 mm each, so half the bounds is half the frame.
        Recorder recorder = Draw(image);
        recorder.Fills.ShouldHaveSingleItem().Bounds.Width.Millimetres.ShouldBe(40, 0.05);
    }

    [Fact]
    public void RectangleIsFilledAndStroked()
    {
        Recorder recorder = Draw(Page()
            .SolidBrush(1, 0x20, 0x40, 0x80)
            .Select(1)
            .Pen(2, 0, 20, 0xC0, 0x00, 0x00)
            .Select(2)
            .Rectangle(1 * Mm, 2 * Mm, 6 * Mm, 5 * Mm));

        recorder.Fills.Count.ShouldBe(1);
        recorder.Strokes.Count.ShouldBe(1);

        (DocRect bounds, Paint paint, _) = recorder.Fills[0];
        bounds.X.Millimetres.ShouldBe(1.0, 0.01);
        bounds.Y.Millimetres.ShouldBe(2.0, 0.01);
        bounds.Width.Millimetres.ShouldBe(5.0, 0.01);
        bounds.Height.Millimetres.ShouldBe(3.0, 0.01);
        paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0x20, 0x40, 0x80));

        recorder.Strokes[0].Stroke.Width.Millimetres.ShouldBe(0.2, 0.01);
    }

    [Fact]
    public void AHandleStatedByTheRecordIsWhereTheObjectGoes()
    {
        // WMF assigns a handle by position; EMF states it. A file that creates object 40 without
        // ever creating 0 to 39 is routine, and appending instead would select the wrong pen.
        Recorder recorder = Draw(Page()
            .SolidBrush(40, 0x11, 0x22, 0x33)
            .Select(40)
            .Rectangle(0, 0, 1000, 1000));

        recorder.Fills.ShouldHaveSingleItem()
            .Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0x11, 0x22, 0x33));
    }

    [Fact]
    public void AStockObjectIsSelectedWithoutEverBeingCreated()
    {
        const int NullPen = unchecked((int)0x80000008);
        const int GreyBrush = unchecked((int)0x80000002);

        Recorder recorder = Draw(Page()
            .Select(NullPen)
            .Select(GreyBrush)
            .Rectangle(0, 0, 1000, 1000));

        recorder.Strokes.ShouldBeEmpty();
        recorder.Fills.ShouldHaveSingleItem()
            .Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0x80, 0x80, 0x80));
    }

    [Fact]
    public void APathIsBuiltByDrawingRecordsAndDrawnOnlyWhenTheFileSaysSo()
    {
        Recorder recorder = Draw(Page()
            .SolidBrush(1, 0x00, 0x80, 0x00)
            .Select(1)
            .Record(EmfFunction.BeginPath)
            .Record(EmfFunction.MoveToEx, 1 * Mm, 1 * Mm)
            .Record(EmfFunction.LineTo, 5 * Mm, 1 * Mm)
            .Record(EmfFunction.LineTo, 5 * Mm, 4 * Mm)
            .Record(EmfFunction.CloseFigure)
            .Record(EmfFunction.EndPath)
            .Record(EmfFunction.FillPath));

        // Nothing was drawn while the path was open, and one fill came out of it.
        recorder.Fills.ShouldHaveSingleItem();
        recorder.Fills[0].Bounds.Width.Millimetres.ShouldBe(4.0, 0.01);
        recorder.Fills[0].Bounds.Height.Millimetres.ShouldBe(3.0, 0.01);
        recorder.Strokes.ShouldBeEmpty();
    }

    [Fact]
    public void APathIsDiscardedOnceItIsDrawn()
    {
        // GDI clears the path when it draws it, so a second FillPath with nothing in between
        // draws nothing. Keeping it would double every figure in a file that draws in a loop.
        Recorder recorder = Draw(Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.BeginPath)
            .Record(EmfFunction.MoveToEx, 0, 0)
            .Record(EmfFunction.LineTo, 1000, 0)
            .Record(EmfFunction.LineTo, 1000, 1000)
            .Record(EmfFunction.EndPath)
            .Record(EmfFunction.FillPath)
            .Record(EmfFunction.FillPath));

        recorder.Fills.ShouldHaveSingleItem();
    }

    [Fact]
    public void AnAbortedPathDrawsNothing()
    {
        Recorder recorder = Draw(Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.BeginPath)
            .Record(EmfFunction.MoveToEx, 0, 0)
            .Record(EmfFunction.LineTo, 1000, 1000)
            .Record(EmfFunction.AbortPath)
            .Record(EmfFunction.StrokeAndFillPath));

        recorder.Fills.ShouldBeEmpty();
        recorder.Strokes.ShouldBeEmpty();
    }

    [Fact]
    public void AWorldTransformMovesAndScalesWhatFollows()
    {
        Recorder recorder = Draw(Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .World(2f, 0f, 0f, 2f, 10 * Mm, 5 * Mm)
            .Rectangle(0, 0, 1000, 1000));

        DocRect bounds = recorder.Fills.ShouldHaveSingleItem().Bounds;
        bounds.X.Millimetres.ShouldBe(10.0, 0.01);
        bounds.Y.Millimetres.ShouldBe(5.0, 0.01);
        bounds.Width.Millimetres.ShouldBe(20.0, 0.01);
    }

    [Fact]
    public void ModifyWorldTransformComposesInTheOrderTheRecordNames()
    {
        // Scale by two, then translate by 10 mm on the right — which under right-multiplication
        // means the translation is applied in the *scaled* space, so it lands at 20 mm.
        const int RightMultiply = 3;

        byte[] scaleThenTranslate = Transform(2f, 0f, 0f, 2f, 0f, 0f);
        byte[] translate = Transform(1f, 0f, 0f, 1f, 10 * Mm, 0f);

        Recorder recorder = Draw(Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Raw(EmfFunction.SetWorldTransform, translate)
            .Raw(EmfFunction.ModifyWorldTransform, [.. scaleThenTranslate, .. Word(RightMultiply)])
            .Rectangle(0, 0, 100, 100));

        // (1,0,0,1,1000,0) then right-multiplied by (2,0,0,2,0,0) is (2,0,0,2,2000,0).
        recorder.Fills.ShouldHaveSingleItem().Bounds.X.Millimetres.ShouldBe(20.0, 0.01);
    }

    [Fact]
    public void ASaveAndRestoreOfTheDeviceContextPutsTheWorldTransformBack()
    {
        Recorder recorder = Draw(Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.SaveDc)
            .World(1f, 0f, 0f, 1f, 30 * Mm, 0f)
            .Rectangle(0, 0, 100, 100)
            .Record(EmfFunction.RestoreDc, -1)
            .Rectangle(0, 0, 100, 100));

        recorder.Fills.Count.ShouldBe(2);
        recorder.Fills[0].Bounds.X.Millimetres.ShouldBe(30.0, 0.01);
        recorder.Fills[1].Bounds.X.Millimetres.ShouldBe(0.0, 0.01);
    }

    [Fact]
    public void AnExcludedRectangleIsSubtractedExactlyRatherThanIgnored()
    {
        // The gap WMF left open. A rectangle taken out of a rectangle is at most four
        // rectangles, so no path arithmetic is needed and nothing has to be approximated.
        VectorImage image = Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.IntersectClipRect, 0, 0, 40 * Mm, 30 * Mm)
            .Record(EmfFunction.ExcludeClipRect, 0, 0, 40 * Mm, 10 * Mm)
            .Rectangle(0, 0, 1000, 1000)
            .Decode();

        image.Diagnostics.ShouldNotContain(d => d.Code == "PL6034");

        Recorder recorder = Draw(image);

        // One clip path holding the band that is left: the top 10 mm are gone. The frame clip
        // VectorImage.Draw emits comes first, so the file's own is the last one.
        recorder.Clips.Count.ShouldBe(2);
        recorder.Clips[^1].Y.Millimetres.ShouldBe(10.0, 0.01);
        recorder.Clips[^1].Height.Millimetres.ShouldBe(20.0, 0.01);
    }

    [Fact]
    public void AClipRegionKeepsEveryRectangleOfItsScanList()
    {
        const int Copy = 5;

        Recorder recorder = Draw(Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .ClipRegion(Copy, (0, 0, 10 * Mm, 10 * Mm), (20 * Mm, 0, 30 * Mm, 10 * Mm))
            .Rectangle(0, 0, 4000, 4000));

        // Both rectangles go in as disjoint subpaths of one path, which is their union under
        // either fill rule.
        recorder.Clips.Count.ShouldBe(2);
        recorder.Clips[^1].X.Millimetres.ShouldBe(0.0, 0.01);
        recorder.Clips[^1].Width.Millimetres.ShouldBe(30.0, 0.01);
    }

    [Fact]
    public void ARegionSubtractedFromTheClipIsExact()
    {
        const int Difference = 4;

        VectorImage image = Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.IntersectClipRect, 0, 0, 40 * Mm, 30 * Mm)
            .ClipRegion(Difference, (10 * Mm, 0, 40 * Mm, 30 * Mm))
            .Rectangle(0, 0, 4000, 4000)
            .Decode();

        image.Diagnostics.ShouldNotContain(d => d.Code == "PL6034");

        Recorder recorder = Draw(image);
        recorder.Clips.Count.ShouldBe(2);
        recorder.Clips[^1].Width.Millimetres.ShouldBe(10.0, 0.01);
    }

    [Fact]
    public void ARegionUnionedWithTheClipIsExactBecauseARegionIsRectangles()
    {
        const int Or = 2;

        VectorImage image = Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.IntersectClipRect, 0, 0, 10 * Mm, 10 * Mm)
            .ClipRegion(Or, (20 * Mm, 0, 30 * Mm, 10 * Mm))
            .Rectangle(0, 0, 4000, 4000)
            .Decode();

        // This was reported as unexpressible and is not: a union reads the clip's own area, and
        // while that area is a rectangle set the answer is another rectangle set.
        image.Diagnostics.ShouldNotContain(d => d.Code == "PL6034");

        Recorder recorder = Draw(image);
        recorder.Clips[^1].X.Millimetres.ShouldBe(0.0, 0.01);
        recorder.Clips[^1].Width.Millimetres.ShouldBe(30.0, 0.01);
    }

    [Fact]
    public void ARegionXoredWithTheClipIsExactAndDropsTheOverlap()
    {
        const int Xor = 3;

        VectorImage image = Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.IntersectClipRect, 0, 0, 20 * Mm, 10 * Mm)
            .ClipRegion(Xor, (10 * Mm, 0, 30 * Mm, 10 * Mm))
            .Rectangle(0, 0, 4000, 4000)
            .Decode();

        image.Diagnostics.ShouldNotContain(d => d.Code == "PL6034");

        // 0–20 xor 10–30 is 0–10 and 20–30, so the middle 10 mm is clipped away and the outer
        // extent stays 30 mm. A test asserting only the extent would pass for a plain union.
        Recorder recorder = Draw(image);
        recorder.Clips[^1].X.Millimetres.ShouldBe(0.0, 0.01);
        recorder.Clips[^1].Width.Millimetres.ShouldBe(30.0, 0.01);
    }

    [Fact]
    public void APathXoredWithTheClipIsStillReportedBecauseItsAreaIsNotRectangles()
    {
        const int Xor = 3;

        VectorImage image = Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.BeginPath)
            .Record(EmfFunction.MoveToEx, 0, 0)
            .Record(EmfFunction.LineTo, 10 * Mm, 0)
            .Record(EmfFunction.LineTo, 10 * Mm, 10 * Mm)
            .Record(EmfFunction.CloseFigure)
            .Record(EmfFunction.EndPath)
            .Record(EmfFunction.SelectClipPath, Xor)
            .Rectangle(0, 0, 4000, 4000)
            .Decode();

        // The one case the rectangle algebra cannot reach, and the reason PL6034 still exists.
        image.Diagnostics.ShouldContain(d => d.Code == "PL6034");
    }

    [Fact]
    public void ARecordedPathBecomesTheClip()
    {
        const int Copy = 5;

        Recorder recorder = Draw(Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.BeginPath)
            .Record(EmfFunction.MoveToEx, 2 * Mm, 2 * Mm)
            .Record(EmfFunction.LineTo, 12 * Mm, 2 * Mm)
            .Record(EmfFunction.LineTo, 12 * Mm, 9 * Mm)
            .Record(EmfFunction.CloseFigure)
            .Record(EmfFunction.EndPath)
            .Record(EmfFunction.SelectClipPath, Copy)
            .Rectangle(0, 0, 4000, 4000));

        recorder.Clips.Count.ShouldBe(2);
        recorder.Clips[^1].X.Millimetres.ShouldBe(2.0, 0.01);
        recorder.Clips[^1].Width.Millimetres.ShouldBe(10.0, 0.01);
    }

    [Fact]
    public void AGradientFillBecomesALinearGradientPaint()
    {
        Recorder recorder = Draw(Page()
            .Gradient(1 * Mm, 2 * Mm, 21 * Mm, 12 * Mm, (0xFF, 0x00, 0x00), (0x00, 0x00, 0xFF), vertical: false));

        (DocRect bounds, Paint paint, _) = recorder.Fills.ShouldHaveSingleItem();
        bounds.Width.Millimetres.ShouldBe(20.0, 0.01);

        GradientPaint gradient = paint.ShouldBeOfType<GradientPaint>();
        gradient.Kind.ShouldBe(GradientKind.Linear);
        gradient.Stops[0].Colour.ShouldBe(new Colour(0xFF, 0, 0));
        gradient.Stops[1].Colour.ShouldBe(new Colour(0, 0, 0xFF));

        // Horizontal mode runs left to right, so the gradient's axis has no vertical component.
        gradient.Start.Y.ShouldBe(gradient.End.Y);
        gradient.End.X.ShouldBeGreaterThan(gradient.Start.X);
    }

    [Fact]
    public void AGradientsChannelsAreInTheHighByteOfSixteenBits()
    {
        // GDI stores 0xFF00 for full intensity. A decoder that takes the low byte draws black,
        // which looks like a colour-resolution bug rather than a field-width one.
        Recorder recorder = Draw(Page()
            .Gradient(0, 0, 1000, 1000, (0xFF, 0xFF, 0xFF), (0x40, 0x80, 0xC0), vertical: true));

        GradientPaint gradient = recorder.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<GradientPaint>();
        gradient.Stops[0].Colour.ShouldBe(Colour.White);
        gradient.Stops[1].Colour.ShouldBe(new Colour(0x40, 0x80, 0xC0));
    }

    [Fact]
    public void AGeometricPenScalesWithThePictureAndACosmeticOneDoesNot()
    {
        const int Geometric = 0x00010000;

        Recorder recorder = Draw(Page()
            .ExtendedPen(1, Geometric, 50, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.MoveToEx, 0, 0)
            .Record(EmfFunction.LineTo, 1000, 0)
            .ExtendedPen(2, 0, 1, 0, 0, 0)
            .Select(2)
            .Record(EmfFunction.MoveToEx, 0, 500)
            .Record(EmfFunction.LineTo, 1000, 500));

        recorder.Strokes.Count.ShouldBe(2);
        recorder.Strokes[0].Stroke.Width.Millimetres.ShouldBe(0.5, 0.01);

        // A cosmetic pen is one *device* unit wide whatever the picture's scale, which is a
        // hairline rather than a fortieth of a millimetre.
        recorder.Strokes[1].Stroke.Width.ShouldBe(Length.Zero);
    }

    [Fact]
    public void AUserDashArrayIsHonouredRatherThanReplacedByAStockPattern()
    {
        const int Geometric = 0x00010000;
        const int UserStyle = 0x00000007;

        Recorder recorder = Draw(Page()
            .ExtendedPen(1, Geometric | UserStyle, 20, 0, 0, 0, 300, 100, 50, 100)
            .Select(1)
            .Record(EmfFunction.MoveToEx, 0, 0)
            .Record(EmfFunction.LineTo, 4000, 0));

        IReadOnlyList<Length> dashes = recorder.Strokes.ShouldHaveSingleItem().Stroke.DashPattern.ShouldNotBeNull();
        dashes.Count.ShouldBe(4);
        dashes[0].Millimetres.ShouldBe(3.0, 0.01);
        dashes[1].Millimetres.ShouldBe(1.0, 0.01);
    }

    [Fact]
    public void AnOddDashArrayIsRepeatedSoInkAndGapDoNotSwap()
    {
        const int Geometric = 0x00010000;
        const int UserStyle = 0x00000007;

        Recorder recorder = Draw(Page()
            .ExtendedPen(1, Geometric | UserStyle, 20, 0, 0, 0, 300, 100, 50)
            .Select(1)
            .Record(EmfFunction.MoveToEx, 0, 0)
            .Record(EmfFunction.LineTo, 4000, 0));

        recorder.Strokes.ShouldHaveSingleItem().Stroke.DashPattern.ShouldNotBeNull().Count.ShouldBe(6);
    }

    [Fact]
    public void CapsAndJoinsComeFromThePenStyleWord()
    {
        const int Geometric = 0x00010000;
        const int EndCapRound = 0x00000000;
        const int JoinBevel = 0x00001000;

        Recorder recorder = Draw(Page()
            .ExtendedPen(1, Geometric | EndCapRound | JoinBevel, 40, 0, 0, 0)
            .Select(1)
            .Poly16(EmfFunction.Polyline16, (0, 0), (1000, 0), (1000, 1000)));

        Stroke stroke = recorder.Strokes.ShouldHaveSingleItem().Stroke;
        stroke.Cap.ShouldBe(LineCap.Round);
        stroke.Join.ShouldBe(LineJoin.Bevel);
    }

    [Fact]
    public void AMiterLimitIsDeviceStateRatherThanPenState()
    {
        const int Geometric = 0x00010000;

        Recorder recorder = Draw(Page()
            .ExtendedPen(1, Geometric, 40, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.SetMiterLimit, 4)
            .Poly16(EmfFunction.Polyline16, (0, 0), (1000, 0), (1000, 1000)));

        recorder.Strokes.ShouldHaveSingleItem().Stroke.MiterLimit.ShouldBe(4.0, 0.001);
    }

    [Fact]
    public void SixteenAndThirtyTwoBitPolygonsDrawTheSameShape()
    {
        Recorder small = Draw(Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Poly16(EmfFunction.Polygon16, (0, 0), (1000, 0), (1000, 1000)));

        Recorder large = Draw(Page()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Poly32(EmfFunction.Polygon, (0, 0), (1000, 0), (1000, 1000)));

        small.Fills.ShouldHaveSingleItem().Bounds.ShouldBe(large.Fills.ShouldHaveSingleItem().Bounds);
    }

    [Fact]
    public void APolylineToContinuesFromTheCurrentPosition()
    {
        Recorder recorder = Draw(Page()
            .Pen(1, 0, 20, 0, 0, 0)
            .Select(1)
            .Record(EmfFunction.MoveToEx, 5 * Mm, 5 * Mm)
            .Poly16(EmfFunction.PolylineTo16, (2000, 500)));

        DocRect bounds = recorder.Strokes.ShouldHaveSingleItem().Bounds;
        bounds.X.Millimetres.ShouldBe(5.0, 0.01);
        bounds.Width.Millimetres.ShouldBe(15.0, 0.01);
    }

    [Fact]
    public void ABezierRecordWithoutAWholeNumberOfCurvesIsRefused()
    {
        // There is no way to tell which of the trailing points is the odd one out, so emfio
        // refuses the record rather than drawing three quarters of a curve.
        VectorImage image = Page()
            .Pen(1, 0, 20, 0, 0, 0)
            .Select(1)
            .Poly16(EmfFunction.PolyBezier16, (0, 0), (100, 0), (200, 0))
            .Decode();

        image.Diagnostics.ShouldContain(d => d.Code == "PL6031");
    }

    [Fact]
    public void TextIsDrawnAtTheStatedPointWithTheSelectedFont()
    {
        Recorder recorder = Draw(Page()
            .Font(1, "Liberation Sans", -400)
            .Select(1)
            .Record(EmfFunction.SetTextColor, 0x0000FF)
            .Text(3 * Mm, 10 * Mm, "Paperless EMF"));

        (string text, DocPoint origin, Length size, _, Paint paint) = recorder.Runs.ShouldHaveSingleItem();
        text.ShouldBe("Paperless EMF");
        origin.X.Millimetres.ShouldBe(3.0, 0.01);
        size.Millimetres.ShouldBe(4.0, 0.01);
        paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0xFF, 0, 0));
    }

    [Fact]
    public void ADxArrayIsHonouredRatherThanTheStringBeingReMeasured()
    {
        // The array is how the producer recorded the result of its own layout. Re-measuring
        // substitutes ours, which drifts glyph by glyph and is visibly wrong by the end of a
        // long label.
        int[] wide = [400, 400, 400, 400];

        Recorder recorder = Draw(Page()
            .Font(1, "Liberation Sans", -300)
            .Select(1)
            .Text(0, 10 * Mm, "IIII", advances: wide));

        GlyphRunFor(recorder).Glyphs.Count.ShouldBe(4);

        // Four millimetres each, whatever the face thinks an "I" is worth.
        foreach (PositionedGlyph glyph in GlyphRunFor(recorder).Glyphs)
        {
            glyph.Advance.Millimetres.ShouldBe(4.0, 0.05);
        }
    }

    [Fact]
    public void ARecordWhoseWholeStringIsATabDrawsNothing()
    {
        // A picture has no paragraph and no tab stops, so a tab in a text record can only ever be a
        // glyph — and no text face has one, so it falls through glyph fallback to whatever last-resort
        // face covers the C0 range and draws its hex box. LibreOffice never gets there: a GDI record
        // reaches ImplLayoutArgs::AddRun, which splits every run at each control character.
        //
        // Measured on `16 - UTM - (NASA).pptx`, whose slide 29 is one EMF holding 62 of exactly this
        // record and whose PDF carried a twelfth, unembedded face because of them.
        Recorder recorder = Draw(Page()
            .Font(1, "Liberation Sans", -400)
            .Select(1)
            .Text(3 * Mm, 10 * Mm, "\t"));

        recorder.GlyphRuns.ShouldBeEmpty();
        recorder.Runs.ShouldBeEmpty();
    }

    [Fact]
    public void ATabInsideAStringIsNotDrawnAndStillSpendsItsAdvance()
    {
        // Cut, not deleted. The DX array is per character in the record's own order, so the producer's
        // advance for the tab still separates what is either side of it — dropping the entry as well as
        // the glyph would pull the rest of the string left by one stated advance.
        int[] wide = [400, 400, 400];

        Recorder recorder = Draw(Page()
            .Font(1, "Liberation Sans", -300)
            .Select(1)
            .Text(0, 10 * Mm, "A\tB", advances: wide));

        recorder.GlyphRuns.Sum(r => r.Glyphs.Count).ShouldBe(2);
        recorder.Runs.Select(r => r.Text).ShouldBe(["A", "B"]);

        // "A" at the reference point; "B" two stated advances further on, not one.
        recorder.Runs[0].Origin.X.Millimetres.ShouldBe(0.0, 0.05);
        recorder.Runs[1].Origin.X.Millimetres.ShouldBe(8.0, 0.05);
    }

    [Fact]
    public void GlyphIndicesBypassCharacterMappingAndStillExtract()
    {
        // ETO_GLYPH_INDEX text has already been shaped by the producer, so the indices go into
        // the run as they stand. The text is recovered by inverting the face's own character map.
        const uint GlyphIndex = 0x0010;

        Recorder plain = Draw(Page()
            .Font(1, "Liberation Sans", -400)
            .Select(1)
            .Text(0, 10 * Mm, "AB"));

        ushort[] indices = [.. GlyphRunFor(plain).Glyphs.Select(g => g.GlyphId)];

        Recorder recorder = Draw(Page()
            .Font(1, "Liberation Sans", -400)
            .Select(1)
            .Text(0, 10 * Mm, new string([(char)indices[0], (char)indices[1]]), GlyphIndex));

        GlyphRun run = GlyphRunFor(recorder);
        run.Glyphs.Select(g => g.GlyphId).ShouldBe(indices);
        run.Text.ShouldBe("AB");
    }

    [Fact]
    public void AnOpaqueTextRectangleIsPaintedBeforeTheText()
    {
        const uint Opaque = 0x0002;

        // The rectangle is the record's own, not the run's bounds, and it is filled with the
        // background colour rather than with the selected brush.
        Recorder recorder = Draw(Page()
            .Font(1, "Liberation Sans", -300)
            .Select(1)
            .Record(EmfFunction.SetBkColor, 0x00FFFF)
            .Text(0, 10 * Mm, "x", Opaque, rectangle: (0, 5 * Mm, 20 * Mm, 12 * Mm)));

        recorder.Fills.ShouldNotBeEmpty();
        recorder.Fills[0].Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0xFF, 0xFF, 0x00));
    }

    [Fact]
    public void ABitmapIsPlacedWhereTheRecordSaysAndStaysEncoded()
    {
        byte[] pixels = new byte[2 * 2 * 3];
        for (int i = 0; i < pixels.Length; i++) pixels[i] = (byte)(i * 10);

        Recorder recorder = Draw(Page().Bitmap((10 * Mm, 5 * Mm, 20 * Mm, 10 * Mm), 2, 2, pixels));

        (RasterImage image, DocRect destination, _) = recorder.Images.ShouldHaveSingleItem();
        destination.X.Millimetres.ShouldBe(10.0, 0.01);
        destination.Width.Millimetres.ShouldBe(20.0, 0.01);

        // Still encoded: nothing in the library has looked at a pixel for an ordinary blit.
        image.IsDecoded.ShouldBeFalse();
        image.EncodedMediaType.ShouldBe("image/bmp");
    }

    [Fact]
    public void AMaskAndAnImageBlittedInSuccessionBecomeOneBitmapWithAnAlphaChannel()
    {
        // The transparent bitmap idiom: no record says "transparent", so a producer says it by
        // blitting a mask with SRCAND and then the image with SRCPAINT to the same rectangle.
        const uint SourceAnd = 0x008800C6;
        const uint SourcePaint = 0x00EE0086;

        // A 2x1 mask: white on the left (transparent), black on the right (opaque).
        byte[] mask = [0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00];
        byte[] image = [0x10, 0x20, 0x30, 0x40, 0x50, 0x60];

        Recorder recorder = Draw(Page()
            .Bitmap((0, 0, 10 * Mm, 10 * Mm), 2, 1, mask, SourceAnd)
            .Bitmap((0, 0, 10 * Mm, 10 * Mm), 2, 1, image, SourcePaint));

        RasterImage merged = recorder.Images.ShouldHaveSingleItem().Image;

        merged.IsDecoded.ShouldBeTrue();
        merged.Width.ShouldBe(2);

        // Straight RGBA: the left pixel is transparent, the right opaque, and both keep the
        // colour image's own channels.
        merged.Pixels.Span[3].ShouldBe((byte)0);
        merged.Pixels.Span[7].ShouldBe((byte)255);
        merged.Pixels.Span[4].ShouldBe((byte)0x60);
    }

    [Fact]
    public void AnUnmergedRasterOperationDrawsTheSourceAndSaysSo()
    {
        const uint SourceInvert = 0x00660046;

        VectorImage image = Page()
            .Bitmap((0, 0, 10 * Mm, 10 * Mm), 1, 1, [0x10, 0x20, 0x30], SourceInvert)
            .Decode();

        image.Diagnostics.ShouldContain(d => d.Code == "PL6033");
        Draw(image).Images.ShouldHaveSingleItem();
    }

    [Fact]
    public void BlacknessAndWhitenessFillWithoutASource()
    {
        const uint Blackness = 0x00000042;

        Recorder recorder = Draw(Page()
            .Bitmap((0, 0, 10 * Mm, 10 * Mm), 1, 1, [0x10, 0x20, 0x30], Blackness));

        recorder.Images.ShouldBeEmpty();
        recorder.Fills.ShouldHaveSingleItem()
            .Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Colour.Black);
    }

    [Fact]
    public void AnEmfIsRecognisedByItsSignatureRatherThanByItsFirstWord()
    {
        byte[] file = Page().Rectangle(0, 0, 100, 100).Build();

        VectorImages.For(file).ShouldBeOfType<EmfImageDecoder>();

        // The record type alone is one byte of signal; the " EMF" signature at offset 40 is what
        // makes the sniff as strong as a magic number.
        byte[] broken = [.. file];
        broken[40] = 0;
        VectorImages.For(broken).ShouldBeNull();
    }

    private static EmfBuilder Page() => new();

    private static byte[] Word(int value)
    {
        byte[] bytes = new byte[4];
        System.Buffers.Binary.BinaryPrimitives.WriteInt32LittleEndian(bytes, value);
        return bytes;
    }

    private static byte[] Transform(float m11, float m12, float m21, float m22, float dx, float dy)
    {
        byte[] bytes = new byte[24];
        System.Buffers.Binary.BinaryPrimitives.WriteSingleLittleEndian(bytes, m11);
        System.Buffers.Binary.BinaryPrimitives.WriteSingleLittleEndian(bytes.AsSpan(4), m12);
        System.Buffers.Binary.BinaryPrimitives.WriteSingleLittleEndian(bytes.AsSpan(8), m21);
        System.Buffers.Binary.BinaryPrimitives.WriteSingleLittleEndian(bytes.AsSpan(12), m22);
        System.Buffers.Binary.BinaryPrimitives.WriteSingleLittleEndian(bytes.AsSpan(16), dx);
        System.Buffers.Binary.BinaryPrimitives.WriteSingleLittleEndian(bytes.AsSpan(20), dy);
        return bytes;
    }

    private static GlyphRun GlyphRunFor(Recorder recorder) => recorder.GlyphRuns.ShouldHaveSingleItem();

    private static Recorder Draw(EmfBuilder builder) => Draw(builder.Decode());

    private static Recorder Draw(VectorImage image)
    {
        Recorder recorder = new();
        image.Draw(recorder, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return recorder;
    }
}

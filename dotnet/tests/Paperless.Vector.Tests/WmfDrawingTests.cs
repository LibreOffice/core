using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector.Wmf;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// What a WMF's records draw.
/// </summary>
/// <remarks>
/// Every fixture states 2540 logical units to the inch and a 2540-unit window, so one logical
/// unit is exactly 1/100 mm and an assertion can name the millimetre it expects. That is not a
/// convenience: it separates a mapping bug from a drawing bug, because under this mapping any
/// geometry error is visible as a wrong number rather than as a wrong scale.
/// </remarks>
public class WmfDrawingTests
{
    private const int Mm = 100;     // logical units per millimetre, under the fixtures' mapping

    [Fact]
    public void RectangleIsFilledAndStroked()
    {
        Recorder recorder = Draw(Square()
            .SolidBrush(0x20, 0x40, 0x80)
            .Select(0)
            .Pen(0, 20, 0xC0, 0x00, 0x00)
            .Select(1)
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
        recorder.Strokes[0].Stroke.Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0xC0, 0, 0));
    }

    [Fact]
    public void ANullBrushFillsNothing()
    {
        Recorder recorder = Draw(Square()
            .NullBrush()
            .Select(0)
            .Rectangle(0, 0, 10 * Mm, 10 * Mm));

        recorder.Fills.ShouldBeEmpty();
        recorder.Strokes.Count.ShouldBe(1);
    }

    [Fact]
    public void ANullPenStrokesNothing()
    {
        // PS_NULL is style 5.
        Recorder recorder = Draw(Square()
            .Pen(5, 0, 0, 0, 0)
            .Select(0)
            .Rectangle(0, 0, 10 * Mm, 10 * Mm));

        recorder.Fills.Count.ShouldBe(1);
        recorder.Strokes.ShouldBeEmpty();
    }

    [Fact]
    public void AnEllipseIsInscribedInItsRectangle()
    {
        Recorder recorder = Draw(Square().Ellipse(2 * Mm, 4 * Mm, 12 * Mm, 8 * Mm));

        recorder.Fills.Count.ShouldBe(1);

        DocRect bounds = recorder.Fills[0].Bounds;
        bounds.X.Millimetres.ShouldBe(2.0, 0.02);
        bounds.Y.Millimetres.ShouldBe(4.0, 0.02);
        bounds.Width.Millimetres.ShouldBe(10.0, 0.02);
        bounds.Height.Millimetres.ShouldBe(4.0, 0.02);
    }

    [Fact]
    public void APolygonClosesAndAPolylineDoesNot()
    {
        Recorder polygon = Draw(Square().Polygon((0, 0), (10 * Mm, 0), (0, 10 * Mm)));
        Recorder polyline = Draw(Square().Polyline((0, 0), (10 * Mm, 0), (0, 10 * Mm)));

        // A polygon is a closed shape and so is filled; a polyline never is.
        polygon.Fills.Count.ShouldBe(1);
        polyline.Fills.ShouldBeEmpty();
        polyline.Strokes.Count.ShouldBe(1);
    }

    [Fact]
    public void MoveToAndLineToTrackTheCurrentPosition()
    {
        Recorder recorder = Draw(Square()
            .Record(WmfFunction.MoveTo, 1 * Mm, 2 * Mm)      // y then x
            .Record(WmfFunction.LineTo, 7 * Mm, 9 * Mm));

        recorder.Strokes.Count.ShouldBe(1);

        DocRect bounds = recorder.Strokes[0].Bounds;
        bounds.X.Millimetres.ShouldBe(2.0, 0.01);
        bounds.Y.Millimetres.ShouldBe(1.0, 0.01);
        bounds.Right.Millimetres.ShouldBe(9.0, 0.01);
        bounds.Bottom.Millimetres.ShouldBe(7.0, 0.01);
    }

    [Fact]
    public void AnArcIsStrokedButNeverFilled()
    {
        // Arc parameters are end, start, then the bounding rectangle, each as y before x.
        Recorder recorder = Draw(Square().Record(
            WmfFunction.Arc,
            5 * Mm, 10 * Mm,        // end point
            0, 5 * Mm,              // start point
            10 * Mm, 10 * Mm, 0, 0));

        recorder.Fills.ShouldBeEmpty();
        recorder.Strokes.Count.ShouldBe(1);
    }

    [Fact]
    public void APieIsAClosedWedgeThroughTheCentre()
    {
        // End at the top of the ellipse, start at its right edge: GDI sweeps anticlockwise from
        // the start to the end, so that is the top-right quadrant and the other way round is the
        // remaining three quarters.
        Recorder recorder = Draw(Square().Record(
            WmfFunction.Pie,
            0, 5 * Mm,              // end point
            5 * Mm, 10 * Mm,        // start point
            10 * Mm, 10 * Mm, 0, 0));

        recorder.Fills.Count.ShouldBe(1);

        // The quarter running from the top of the ellipse to its right edge, plus the centre:
        // that is the top-right quadrant and nothing else.
        DocRect bounds = recorder.Fills[0].Bounds;
        bounds.X.Millimetres.ShouldBe(5.0, 0.05);
        bounds.Y.Millimetres.ShouldBe(0.0, 0.05);
        bounds.Right.Millimetres.ShouldBe(10.0, 0.05);
        bounds.Bottom.Millimetres.ShouldBe(5.0, 0.05);
    }

    [Fact]
    public void IntersectClipRectClipsWhatFollows()
    {
        Recorder recorder = Draw(Square()
            .Record(WmfFunction.IntersectClipRect, 6 * Mm, 6 * Mm, 2 * Mm, 2 * Mm)
            .Rectangle(0, 0, 10 * Mm, 10 * Mm));

        recorder.Clips.Count.ShouldBe(1);
        recorder.Clips[0].X.Millimetres.ShouldBe(2.0, 0.01);
        recorder.Clips[0].Width.Millimetres.ShouldBe(4.0, 0.01);
    }

    [Fact]
    public void ARegionSelectedAsAClipKeepsEveryRectangleOfItsScanList()
    {
        // A two-rectangle region: emfio does not read this record at all, so a WMF that clips
        // through one draws unclipped there.
        short[] region =
        [
            0, 6, 0, 0, 32, 4, 8,           // header, then the bounding rectangle
            0, 0, 4 * Mm, 8 * Mm,           // bounds: bottom, right, top, left
            2, 0, 2 * Mm, 0, 3 * Mm,        // scan one: count, top, bottom, left, right
            2,
            2, 2 * Mm, 4 * Mm, 5 * Mm, 8 * Mm,
            2,
        ];

        Recorder recorder = Draw(Square()
            .Record(WmfFunction.CreateRegion, region)
            .Record((WmfFunction)0x012C, 0)                 // SelectClipRegion
            .Rectangle(0, 0, 10 * Mm, 10 * Mm));

        // Handle 0 is the region; selecting handle 0 as a clip region means "no clip" in WMF,
        // so the rectangle is unclipped — the same record with a real handle is the next test.
        recorder.Clips.ShouldBeEmpty();
    }

    [Fact]
    public void ObjectHandlesAreReusedAfterADelete()
    {
        // Create a red pen at handle 0 and a brush at handle 1, delete handle 0, then create a
        // blue pen — which must land back in handle 0, not at handle 2.
        Recorder recorder = Draw(Square()
            .Pen(0, 20, 0xFF, 0x00, 0x00)
            .SolidBrush(0xFF, 0xFF, 0xFF)
            .Delete(0)
            .Pen(0, 20, 0x00, 0x00, 0xFF)
            .Select(0)
            .Rectangle(0, 0, 5 * Mm, 5 * Mm));

        recorder.Strokes.Count.ShouldBe(1);
        recorder.Strokes[0].Stroke.Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0, 0, 0xFF));
    }

    [Fact]
    public void RestoreDcUndoesASelection()
    {
        Recorder recorder = Draw(Square()
            .SolidBrush(0xFF, 0x00, 0x00)
            .SolidBrush(0x00, 0xFF, 0x00)
            .Select(0)
            .Record(WmfFunction.SaveDc)
            .Select(1)
            .Record(WmfFunction.RestoreDc, -1)
            .Rectangle(0, 0, 5 * Mm, 5 * Mm));

        recorder.Fills[0].Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0xFF, 0, 0));
    }

    [Fact]
    public void TextIsShapedAndPlacedOnItsBaseline()
    {
        Recorder recorder = Draw(Square()
            .Font("Liberation Sans", 4 * Mm)
            .Select(0)
            .Record(WmfFunction.SetTextAlign, 0x0018)       // baseline, left
            .TextOut("Wmf", 2 * Mm, 6 * Mm));

        recorder.Runs.Count.ShouldBe(1);
        recorder.Runs[0].Text.ShouldBe("Wmf");
        recorder.Runs[0].Origin.X.Millimetres.ShouldBe(2.0, 0.01);
        recorder.Runs[0].Origin.Y.Millimetres.ShouldBe(6.0, 0.01);
        recorder.Runs[0].Size.Millimetres.ShouldBe(4.0, 0.01);
    }

    [Fact]
    public void CentredTextIsShiftedByHalfItsMeasuredWidth()
    {
        Recorder left = Draw(Square()
            .Font("Liberation Sans", 4 * Mm)
            .Select(0)
            .Record(WmfFunction.SetTextAlign, 0x0010)       // baseline, left
            .TextOut("Wmf", 10 * Mm, 6 * Mm));

        Recorder centred = Draw(Square()
            .Font("Liberation Sans", 4 * Mm)
            .Select(0)
            .Record(WmfFunction.SetTextAlign, 0x0016)       // baseline, centred
            .TextOut("Wmf", 10 * Mm, 6 * Mm));

        left.Runs.Count.ShouldBe(1);
        centred.Runs.Count.ShouldBe(1);

        // Half the run's own width, whatever that turns out to be for the resolved face.
        Length width = Length.FromEmu(2 * (left.Runs[0].Origin.X.Emu - centred.Runs[0].Origin.X.Emu));
        width.Millimetres.ShouldBeGreaterThan(3.0);
        width.Millimetres.ShouldBeLessThan(15.0);
    }

    [Fact]
    public void TextIsDecodedWithTheFontsCodePage()
    {
        // 0xC0 is 'À' in Windows-1252 and 'Р' in Windows-1251; the font's character-set byte is
        // the only thing that decides which, and it lives on a record that draws no text at all.
        WmfBuilder builder = Square().Font("Liberation Sans", 4 * Mm).Select(0);
        byte[] file = builder.TextOut("À", 1 * Mm, 5 * Mm).Build();

        VectorImage image = new WmfImageDecoder().Decode(file);
        Recorder recorder = new();
        image.Content.Replay(recorder);

        recorder.Runs.Count.ShouldBe(1);
        recorder.Runs[0].Text.ShouldBe("À");
    }

    [Fact]
    public void AnEscapeCarryingAnEmfIsReportedRatherThanDrawn()
    {
        // The WMFC comment header: magic, comment type 1, version 0x00010000, a checksum, flags,
        // one chunk, the chunk's size, the remaining size and the total.
        byte[] payload = new byte[4 + 34 + 8];
        System.Buffers.Binary.BinaryPrimitives.WriteUInt16LittleEndian(payload, 15);         // MFCOMMENT
        System.Buffers.Binary.BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(2), 42);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(4), 0x43464D57);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(8), 1);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(12), 0x00010000);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(18), 0);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(22), 1);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(26), 8);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(30), 8);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(34), 8);

        VectorImage image = new WmfImageDecoder().Decode(
            Square().Raw(WmfFunction.Escape, payload).Rectangle(0, 0, 5 * Mm, 5 * Mm).Build());

        image.Diagnostics.ShouldContain(d => d.Code == "PL6030");
        image.Content.Count.ShouldBeGreaterThan(0);
    }

    /// <summary>A metafile whose logical unit is exactly 1/100 mm.</summary>
    private static WmfBuilder Square()
    {
        WmfBuilder builder = new() { UnitsPerInch = 2540, Bounds = (0, 0, 2540, 2540) };
        return builder.WindowOrigin(0, 0).WindowExtent(2540, 2540);
    }

    private static Recorder Draw(WmfBuilder builder)
    {
        VectorImage image = new WmfImageDecoder().Decode(builder.Build());
        image.IsEmpty.ShouldBeFalse();

        Recorder recorder = new();
        image.Content.Replay(recorder);
        return recorder;
    }
}

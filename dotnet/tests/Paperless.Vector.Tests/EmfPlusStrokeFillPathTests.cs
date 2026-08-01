using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// <c>EmfPlusStrokeFillPath</c>, the one record [MS-EMFPLUS] enumerates and never specifies.
/// </summary>
/// <remarks>
/// <para>
/// The record appears in the <c>RecordType</c> enumeration (2.1.1.1) with a one-sentence
/// description and is <em>absent</em> from the drawing-record table in 2.3.4 that gives all
/// twenty-one records with a defined layout — no <c>Flags</c> bit assignment, no
/// <c>ObjectID</c>, no <c>RecordData</c>. So these tests pin a <em>reading</em> rather than a
/// specification, and the reading is the conservative one: the flags' low byte is an object slot
/// because it is one in every record that names an object, and "current pen" and "current brush"
/// mean the ones the previous drawing record used because a format with no pen or brush state
/// gives those words no other referent.
/// </para>
/// <para>
/// What matters most here is the second test: a file the reading does not fit must be no worse
/// off than when the record drew nothing at all.
/// </para>
/// </remarks>
public class EmfPlusStrokeFillPathTests
{
    private const ushort StrokeFillPath = 0x4037;

    private static readonly (float X, float Y)[] Triangle = [(0, 0), (40, 0), (40, 40)];
    private static readonly byte[] TriangleTypes = [0, 1, 0x81];

    [Fact]
    public void ItUsesThePenAndBrushTheLastDrawingRecordUsed()
    {
        Recorder recorder = Draw(new EmfPlusBuilder()
            .Header()
            .Path(1, Triangle, TriangleTypes)
            .Pen(2, 0xFF0000FF, 3)
            .SolidBrush(3, 0xFFFF0000)
            .FillPath(1, 0xFFFF0000)                    // establishes the current brush
            .DrawPath(1, 2)                             // establishes the current pen
            .Path(4, [(60, 0), (100, 0), (100, 40)], TriangleTypes)
            .Record(StrokeFillPath, 4, [])
            .End());

        // One fill and one stroke from the two establishing records, and one of each from the
        // record under test — which is what "strokes and fills in one record" has to mean.
        recorder.Fills.Count.ShouldBe(2);
        recorder.Strokes.Count.ShouldBe(2);

        recorder.Fills[^1].Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(255, 0, 0));
        recorder.Strokes[^1].Stroke.Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0, 0, 255));

        // And it is the second triangle that was drawn, not the first again.
        recorder.Fills[^1].Bounds.X.Millimetres.ShouldBeGreaterThan(recorder.Fills[0].Bounds.X.Millimetres);
    }

    [Fact]
    public void ASlotThatHoldsNoPathDrawsNothingAndSaysSo()
    {
        VectorImage image = new EmfBuilder().Plus(new EmfPlusBuilder()
            .Header()
            .SolidBrush(1, 0xFFFF0000)
            .FillRects(0xFFFF0000, (0, 0, 10, 10))
            .Record(StrokeFillPath, 7, [])              // slot 7 was never given a path
            .End()).Decode();

        image.Diagnostics.ShouldContain(d => d.Code == "PL6037");
    }

    [Fact]
    public void BeforeAnythingHasBeenDrawnThereIsNoCurrentPenOrBrushAndItSaysSo()
    {
        VectorImage image = new EmfBuilder().Plus(new EmfPlusBuilder()
            .Header()
            .Path(1, Triangle, TriangleTypes)
            .Record(StrokeFillPath, 1, [])
            .FillRects(0xFFFF0000, (0, 0, 10, 10))
            .End()).Decode();

        // A record naming a real path but arriving before any pen or brush has been used is the
        // case the reading cannot serve, and reporting it is better than picking slot zero.
        image.Diagnostics.ShouldContain(d => d.Code == "PL6037");
    }

    private static Recorder Draw(EmfPlusBuilder plus)
    {
        VectorImage image = new EmfBuilder().Plus(plus).Decode();
        Recorder sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return sink;
    }
}

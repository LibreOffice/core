using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The recording half of the seam: every sink call survives a round trip.
/// </summary>
/// <remarks>
/// A decoded vector image is a display list, so a command the recorder drops is a picture
/// that silently loses part of itself. The metafile decoders will write through this same
/// class, which is why it is tested on its own rather than only through SVG.
/// </remarks>
public sealed class DisplayListTests
{
    [Fact]
    public void EverySinkCallIsRecordedAndReplayedInOrder()
    {
        DisplayList list = new();

        list.BeginPage(new DocSize(Length.FromPoints(10), Length.FromPoints(20)));
        list.Save();
        list.Transform(AffineTransform.Scale(2, 2));
        list.ClipPath(Rect(0, 0, 10, 10));
        list.FillPath(Rect(1, 1, 2, 2), Paint.Solid(Colour.Black));
        list.StrokePath(Rect(3, 3, 2, 2), new Stroke(Paint.Solid(Colour.White), Length.FromPoints(1)));
        list.DrawImage(RasterImage.Encoded(new byte[] { 1, 2, 3 }), new DocRect(default, default));
        list.BeginTransparencyGroup(0.5);
        list.EndTransparencyGroup();
        list.Restore();
        list.EndPage();

        list.Count.ShouldBe(11);

        Counter sink = new();
        list.Replay(sink);

        sink.Calls.ShouldBe([
            "BeginPage", "Save", "Transform", "ClipPath", "FillPath",
            "StrokePath", "DrawImage", "BeginGroup", "EndGroup", "Restore", "EndPage",
        ]);
    }

    [Fact]
    public void ReplayingTwiceProducesTheSameCallsBothTimes()
    {
        // The reason to record at all: the same picture is drawn on many pages, and a
        // display list that consumed itself would only work the first time.
        DisplayList list = new();
        list.FillPath(Rect(0, 0, 5, 5), Paint.Solid(Colour.Black));

        Counter first = new();
        Counter second = new();

        list.Replay(first);
        list.Replay(second);

        second.Calls.ShouldBe(first.Calls);
    }

    private static GraphicsPath Rect(double x, double y, double width, double height)
        => GraphicsPath.Rectangle(new DocRect(
            Length.FromPoints(x), Length.FromPoints(y), Length.FromPoints(width), Length.FromPoints(height)));

    private sealed class Counter : IDrawingSink
    {
        public List<string> Calls { get; } = [];

        public void BeginPage(DocSize size) => Calls.Add("BeginPage");

        public void EndPage() => Calls.Add("EndPage");

        public void Save() => Calls.Add("Save");

        public void Restore() => Calls.Add("Restore");

        public void Transform(AffineTransform transform) => Calls.Add("Transform");

        public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero) => Calls.Add("ClipPath");

        public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero)
            => Calls.Add("FillPath");

        public void StrokePath(GraphicsPath path, Stroke stroke) => Calls.Add("StrokePath");

        public void DrawGlyphRun(GlyphRun run, Paint paint) => Calls.Add("DrawGlyphRun");

        public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0)
            => Calls.Add("DrawImage");

        public void BeginTransparencyGroup(double opacity) => Calls.Add("BeginGroup");

        public void EndTransparencyGroup() => Calls.Add("EndGroup");
    }
}

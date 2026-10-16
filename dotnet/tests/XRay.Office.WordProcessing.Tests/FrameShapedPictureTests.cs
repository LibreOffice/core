using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A picture whose own shape properties name a preset: the outline bounds the bitmap, not the
/// bounding box.
/// </summary>
/// <remarks>
/// <para>
/// A <c>pic:pic</c> carries an <c>a:prstGeom</c> in its <c>pic:spPr</c> exactly as a
/// <c>wps:wsp</c> does, and this side read it for the fill and the outline while drawing the
/// bitmap over the whole rectangle — so a photograph Word puts in a circle came out as a square
/// photograph with a circle drawn round it.
/// </para>
/// <para>
/// <b>The rule, in <c>oox</c>'s own words.</b> <c>Shape::createAndInsert</c> (read in
/// <c>/home/user/libreoffice-core</c>, which declares <c>27.2.0.0.alpha0+</c> and is not the
/// reference binary's source) sets
/// <c>bIsCroppedGraphic = (aServiceName == "com.sun.star.drawing.GraphicObjectShape" &amp;&amp;
/// !mpCustomShapePropertiesPtr-&gt;representsDefaultShape())</c> under the comment *"Use custom
/// shape instead of GraphicObjectShape if the image is cropped to shape. Except rectangle, which
/// does not require further cropping"*, and <c>representsDefaultShape</c> is false exactly when
/// the shape names a preset other than <c>rect</c> or carries an <c>a:custGeom</c> path list
/// (<c>customshapeproperties.cxx</c>:123-128). So the bitmap becomes a custom shape's fill and the
/// geometry is what bounds it.
/// </para>
/// <para>
/// <b>Confirmed a second time against 26.2.4.2's own output</b>, which is what the first leg
/// cannot stand on: <c>picture-shaped.docx</c> converted to PDF emits the ellipse as a clip path
/// — <c>72 612 m … c … h W* n</c> — immediately before <c>288 0 0 216 72 504.05 cm /Im7 Do</c>,
/// and <c>picture-shaped-rect.docx</c> emits <c>72 504 288 216 re W* n</c> instead.
/// </para>
/// <para>
/// <b>Reach.</b> <c>probes/wordsgroup-r128/pic-preset-census.py</c> over the 947-document corpus:
/// <b>8 shaped pictures in 2 <c>docx</c></b> through <c>pic:pic</c>, and
/// <c>shaped-picture-census.py</c> adds 5 more in 2 further <c>docx</c> through a <c>wps:wsp</c>
/// with an <c>a:blipFill</c> — 4 documents in all, against 272 <c>docx</c> scanned. The same
/// census finds 70 shaped pictures in 7 <c>pptx</c>, where <c>SlideDrawing.DrawPicture</c> has
/// clipped to the outline all along; that asymmetry is what this closes.
/// </para>
/// </remarks>
public sealed class FrameShapedPictureTests
{
    /// <summary>
    /// A picture in a non-rectangular preset is clipped, and to the preset rather than to its box.
    /// </summary>
    /// <remarks>
    /// The curve count is the assertion that matters. A rectangular clip is four straight
    /// segments; LibreOffice's own ellipse is twelve cubics, and any clip carrying a cubic at all
    /// cannot be the bounding box.
    /// </remarks>
    [Fact]
    public void AShapedPictureIsClippedToItsOutline()
    {
        RecordingDrawingSink sink = Draw("picture-shaped.docx");

        GraphicsPath clip = sink.ClipPaths.ShouldHaveSingleItem();
        clip.Commands.Count(command => command.Verb == PathVerb.CubicTo)
            .ShouldBeGreaterThan(0);
    }

    /// <summary>
    /// And the clip is the frame's own rectangle in extent: an ellipse inscribed in the picture's
    /// 288 × 216 pt box, at the one-inch margin.
    /// </summary>
    [Fact]
    public void TheClipSpansThePicturesOwnRectangle()
    {
        RecordingDrawingSink sink = Draw("picture-shaped.docx");
        DocRect bounds = Bounds(sink.ClipPaths.ShouldHaveSingleItem());

        bounds.X.Points.ShouldBe(72, 1.5);
        bounds.Width.Points.ShouldBe(288, 1.5);
        bounds.Height.Points.ShouldBe(216, 1.5);
    }

    /// <summary>
    /// The bitmap still fills the frame, so the outline cropped the picture rather than resizing
    /// it — the distinction the reference draws by giving a custom shape a bitmap fill.
    /// </summary>
    [Fact]
    public void TheBitmapStillFillsTheFrame()
    {
        DocRect destination = Draw("picture-shaped.docx")
            .Pages.SelectMany(page => page.Images).ShouldHaveSingleItem();

        destination.Width.Points.ShouldBe(288, 1.0);
        destination.Height.Points.ShouldBe(216, 1.0);
    }

    /// <summary>
    /// A crop and an outline compose: the picture is still drawn into the larger rectangle a crop
    /// needs, and the ellipse is still what bounds it.
    /// </summary>
    /// <remarks>
    /// The two halves used to be exclusive here — the crop was the only thing that took a clip at
    /// all — so this is the case a fix that simply swapped one clip for the other would fail.
    /// </remarks>
    [Fact]
    public void ACroppedShapedPictureKeepsBoth()
    {
        RecordingDrawingSink sink = Draw("picture-shaped-crop.docx");

        DocRect destination = sink.Pages.SelectMany(page => page.Images).ShouldHaveSingleItem();

        // 288 / (1 - 0.1 - 0.3) = 480 pt across and 216 / (1 - 0.2 - 0.4) = 540 pt down.
        destination.Width.Points.ShouldBe(480, 1.0);
        destination.Height.Points.ShouldBe(540, 1.0);

        GraphicsPath clip = sink.ClipPaths.ShouldHaveSingleItem();
        clip.Commands.Count(command => command.Verb == PathVerb.CubicTo).ShouldBeGreaterThan(0);
        Bounds(clip).Width.Points.ShouldBe(288, 1.5);
    }

    /// <summary>
    /// And a rectangular one takes no clip at all. <c>a:prstGeom prst="rect"</c> is the commonest
    /// value in the corpus, and clipping it would change the bytes of nearly every rendering that
    /// carries a picture to arrive exactly where not clipping arrives.
    /// </summary>
    [Fact]
    public void ARectangularPictureIsNotClipped()
        => Draw("picture-shaped-rect.docx").Clips.ShouldBe(0);

    // ------------------------------------------------------------------ helpers

    /// <summary>A path's bounding box, control points included.</summary>
    private static DocRect Bounds(GraphicsPath path)
    {
        double left = double.MaxValue;
        double top = double.MaxValue;
        double right = double.MinValue;
        double bottom = double.MinValue;

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            foreach (DocPoint point in Points(command))
            {
                left = Math.Min(left, point.X.Points);
                top = Math.Min(top, point.Y.Points);
                right = Math.Max(right, point.X.Points);
                bottom = Math.Max(bottom, point.Y.Points);
            }
        }

        return new DocRect(
            Length.FromPoints(left), Length.FromPoints(top),
            Length.FromPoints(right - left), Length.FromPoints(bottom - top));

        static IEnumerable<DocPoint> Points(PathCommand command)
        {
            yield return command.Point;

            if (command.Verb != PathVerb.CubicTo) yield break;

            yield return command.Control1;
            yield return command.Control2;
        }
    }

    private static RecordingDrawingSink Draw(string name)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(name)))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return sink;
    }
}

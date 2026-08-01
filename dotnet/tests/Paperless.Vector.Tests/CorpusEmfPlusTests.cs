using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The corpus fixture that carries an EMF+ stream, read the way a reader would read it.
/// </summary>
/// <remarks>
/// <para>
/// One fixture, and it is hand-built rather than exported for a reason the WMF and EMF fixtures
/// do not share: <b>LibreOffice cannot write EMF+ at all</b>, and neither can any other tool on a
/// Linux machine. It reads EMF+ — <c>emfio</c>'s whole EMF+ path exists for that — but the
/// producers are GDI+ itself, PowerPoint and Visio. So a fixture either comes from a Windows
/// machine, or is built a record at a time, and only the second is reproducible here.
/// </para>
/// <para>
/// It is an EMF+ <em>Dual</em> file: the picture is stated twice, once in EMF+ records and once
/// in GDI records that must not be replayed. That is the property worth having in a committed
/// fixture, because it is the one a regression would silently double.
/// </para>
/// </remarks>
public sealed class CorpusEmfPlusTests
{
    private static readonly string Corpus = Locate();

    [Fact]
    public void TheDualFixtureDrawsItsEmfPlusHalfAndNotItsGdiHalf()
    {
        VectorImage image = Decode();

        image.IsEmpty.ShouldBeFalse();
        image.IsTruncated.ShouldBeFalse();

        // A frame of 8000 by 6000 hundredths of a millimetre against a reference device of 8000
        // by 6000 pixels to 80 by 60 mm: one EMF+ world unit is exactly 1/100 mm.
        image.IntrinsicSize.Width.Millimetres.ShouldBe(80, 0.01);
        image.IntrinsicSize.Height.Millimetres.ShouldBe(60, 0.01);

        Recorder sink = Draw(image);

        // The GDI half is a rectangle at 4 mm and an ellipse beside it, both in #C06030. If
        // either were replayed the fill count would be two higher and one of them would be that
        // colour.
        foreach ((DocRect _, Paint paint, FillRule _) in sink.Fills)
        {
            if (paint is SolidPaint solid) solid.Colour.ShouldNotBe(new Colour(0xC0, 0x60, 0x30));
        }
    }

    [Fact]
    public void EveryKindOfBrushInTheFixtureReachesTheSink()
    {
        Recorder sink = Draw(Decode());

        sink.Fills.Any(fill => fill.Paint is GradientPaint).ShouldBeTrue();
        sink.Fills.Any(fill => fill.Paint is SolidPaint { Colour.A: < 255 }).ShouldBeTrue();

        // The hatch is stroked lines clipped to the shape, over a background the brush states
        // itself — which is what makes a GDI+ hatch opaque where a GDI one is not.
        sink.Strokes.Count(stroke => stroke.Stroke.Width == Length.Zero).ShouldBeGreaterThan(4);
    }

    [Fact]
    public void TheFixturesTextIsOneRunOfRealCharacters()
    {
        Recorder sink = Draw(Decode());

        (string text, DocPoint origin, Length size, _, _) = sink.Runs.ShouldHaveSingleItem();

        text.ShouldBe("Paperless EMF+");
        size.Millimetres.ShouldBe(4, 0.05);

        // Centred in a layout rectangle from 4 mm to 76 mm, so the run starts well right of it.
        origin.X.Millimetres.ShouldBeGreaterThan(20);
        origin.X.Millimetres.ShouldBeLessThan(40);
    }

    [Fact]
    public void TheFixturesClipIsNarrowedAndThenWidenedAgain()
    {
        Recorder sink = Draw(Decode());

        // The frame's clip, then the record's, then the frame's again after the reset.
        sink.Clips.Count.ShouldBeGreaterThanOrEqualTo(3);
        sink.Clips.Any(clip => Math.Abs(clip.Width.Millimetres - 30) < 0.1).ShouldBeTrue();
    }

    [Fact]
    public void TheFixtureIsSniffedByContentLikeAnyOtherMetafile()
    {
        byte[] bytes = File.ReadAllBytes(Path.Combine(Corpus, "emfplus-shapes.emf"));

        // An EMF+ file is an EMF: it has no signature of its own, so nothing in the seam changes
        // and no new decoder registers. The EMF decoder is what claims it.
        VectorImages.For(bytes).ShouldBeOfType<Emf.EmfImageDecoder>();
    }

    private static VectorImage Decode()
        => VectorImages.Decode(File.ReadAllBytes(Path.Combine(Corpus, "emfplus-shapes.emf")));

    private static Recorder Draw(VectorImage image)
    {
        Recorder sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return sink;
    }

    private static string Locate()
    {
        DirectoryInfo? at = new(AppContext.BaseDirectory);

        while (at is not null)
        {
            string candidate = Path.Combine(at.FullName, "tests", "corpus", "features");
            if (Directory.Exists(candidate)) return candidate;

            at = at.Parent;
        }

        throw new DirectoryNotFoundException("The corpus directory could not be found.");
    }
}

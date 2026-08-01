using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Paperless.Vector;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// The vector pictures on <c>vector-picture-deck</c>, in both formats.
/// </summary>
/// <remarks>
/// <para>
/// One deck holding a WMF, an EMF and an SVG, read as ODP and as LibreOffice's own PPTX export of
/// the same source. The pair is the point: the ODP names its pictures by package entry and declares
/// a media type per picture, while the PPTX names relationships and declares nothing useful —
/// LibreOffice writes the EMF into <c>ppt/media/image2.wmf</c> — so both readers have to reach the
/// same three pictures from the bytes alone.
/// </para>
/// <para>
/// The measurement against LibreOffice's own rendering is the render sweep; this file pins what that
/// comparison rests on, so a regression says which of the three stopped decoding rather than only
/// that the slide changed.
/// </para>
/// </remarks>
public class SlideVectorPictureTests
{
    public static TheoryData<string> BothFormats => ["vector-picture-deck.odp", "vector-picture-deck.pptx"];

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void AllThreeVectorPicturesDecode(string document)
    {
        IReadOnlyList<PlacedPicture> pictures = Pictures(document);

        pictures.Count.ShouldBe(3);

        foreach (PlacedPicture picture in pictures)
        {
            Lazy<VectorImage> vector = picture.Vector.ShouldNotBeNull();
            vector.Value.IsEmpty.ShouldBeFalse();
            vector.Value.IsTruncated.ShouldBeFalse();
        }
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void TheWordmarkInsideTheSvgSurvivesAsARealGlyphRun(string document)
    {
        // The one picture whose content is checkable without a rasteriser, and the reason the SVG
        // is in the deck at all: it holds a "PAPERLESS" wordmark, so a decode that produced an
        // outline or nothing at all is visible in a text extraction of the rendered page.
        VectorImage svg = Pictures(document)[2].Vector.ShouldNotBeNull().Value;

        PlacedDrawingSink sink = new();
        svg.Draw(sink, Destination);

        sink.Runs.ShouldHaveSingleItem().Run.Text.ShouldBe("PAPERLESS");
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void APictureIsStretchedByItsFrameRatherThanByItsInk(string document)
    {
        // The trap the seam's author named. `VectorImage.Draw` maps the picture's whole view box
        // onto the destination, so the ink lands wherever the picture's own margins put it. Taking
        // the extent of the ink instead makes a logo with margins several times too large and
        // clipped — which reads as a mapping bug in the decoder and is not one.
        //
        // Measured on the SVG: its view box is 200 × 100 user units and its rounded rectangle sits
        // at x = 2 with width 196, so drawn into an 80 × 40 mm destination the rectangle lands
        // 0.8 mm in and is 78.4 mm wide. Scaling the *ink* onto the destination instead would put
        // it at zero and 80 wide, and every picture with a margin would creep outwards.
        VectorImage svg = Pictures(document)[2].Vector.ShouldNotBeNull().Value;

        svg.IntrinsicSize.Width.Millimetres.ShouldBe(40, 0.01);
        svg.IntrinsicSize.Height.Millimetres.ShouldBe(20, 0.01);

        PlacedDrawingSink sink = new();
        svg.Draw(sink, Destination);

        DocRect plate = sink.Fills[0].Bounds;
        plate.X.Millimetres.ShouldBe(0.8, 0.05);
        plate.Width.Millimetres.ShouldBe(78.4, 0.1);
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void ThePicturesGoWhereTheirFramesAre(string document)
    {
        IReadOnlyList<PlacedPicture> pictures = Pictures(document);

        pictures[0].Destination.Width.Millimetres.ShouldBe(80, 0.1);
        pictures[0].Destination.Height.Millimetres.ShouldBe(60, 0.1);
        pictures[1].Destination.X.Millimetres.ShouldBe(100, 0.1);
        pictures[2].Destination.Height.Millimetres.ShouldBe(40, 0.1);
    }

    [Fact]
    public void ThePptxKeepsTheSvgBesideItsRasterFallback()
    {
        // The `asvg:svgBlip` case, which only the OOXML export has: `r:embed` names a PNG and the
        // extension names the SVG. Both are kept — the vector is drawn and the raster is what a
        // decode coming back empty would fall through to.
        PlacedPicture svg = Pictures("vector-picture-deck.pptx")[2];

        svg.Vector.ShouldNotBeNull();
        svg.Image.ShouldNotBeNull().EncodedBytes.Length.ShouldBeGreaterThan(
            svg.Vector.Value.Content.Count, "the raster fallback is the larger of the two");
    }

    [Fact]
    public void TheOdpNamesTheVectorAloneBecauseItHasNoAlternativeToChooseFrom()
    {
        // ODF's `draw:frame` may list several `draw:image` children as alternatives, and the first
        // that can be drawn wins; there is no extension to look inside, so a picture is one thing.
        foreach (PlacedPicture picture in Pictures("vector-picture-deck.odp"))
        {
            picture.Image.ShouldBeNull();
        }
    }

    /// <summary>An 80 × 40 mm rectangle at the origin: twice the SVG's own size, aspect preserved.</summary>
    private static DocRect Destination => new(
        Core.Units.Length.Zero,
        Core.Units.Length.Zero,
        Core.Units.Length.FromMillimetres(80),
        Core.Units.Length.FromMillimetres(40));

    private static IReadOnlyList<PlacedPicture> Pictures(string document)
    {
        using IDocument read =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(document)));

        SlidePages pages = (SlidePages)((IPaginatedDocument)read).Layout();

        return
        [
            .. pages.Slides[0].Shapes
                .Select(shape => shape.Picture)
                .Where(picture => picture is not null)
                .Select(picture => picture!),
        ];
    }
}

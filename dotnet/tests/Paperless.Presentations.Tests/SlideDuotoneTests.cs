using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// <c>a:duotone</c>, and a picture fill's <c>a:alphaModFix</c>.
/// </summary>
/// <remarks>
/// <para>
/// A duotone maps a picture onto the ramp between two colours by its own brightness. It is how
/// an Office theme paints one grey texture in a deck's own colours, and neither the element nor
/// the string appeared anywhere in <c>dotnet/src</c>:
/// <c>slides/batch-012/pptx/order-of-worship-ppt-revised-2018.pptx</c> takes its whole
/// background that way and drew as a dark grey vignette against a pale reference — 766.96 of
/// unaccounted ink over 28 pages, 27% of the slides track's whole figure, and 8.03 after.
/// </para>
/// <para>
/// <c>a:alphaModFix</c> was a different shape of the same invisibility: parsed into
/// <c>DrawingBlipFill.Opacity</c>, carried to <c>PlacedPicture</c> on the <c>p:pic</c> path, and
/// dropped on the fill path because <c>BitmapPaint</c> had nowhere to put it. <c>3492.pptx</c>
/// lays black text over a runway photograph its layout states at <c>amt="16000"</c>; we drew the
/// photograph at full strength and the text was unreadable on it.
/// </para>
/// <para>
/// <c>slide-duotone.pptx</c> is built for both. Its image is four rows — black, mid grey,
/// saturated blue, white — which is the smallest thing that separates a duotone from a
/// brightness ramp: the blue row has the same value in one channel as the white row and a
/// completely different luminance. Page 1 draws it duotoned and plain side by side; page 2
/// draws it as a shape fill at 50% and at full over a red slide.
/// </para>
/// </remarks>
public class SlideDuotoneTests
{
    private const string Deck = "slide-duotone.pptx";

    [Fact]
    public void ADuotonedPictureCarriesBothItsColours()
    {
        RasterImage image = Pictures(0)
            .First(picture => picture.Image?.Duotone is not null)
            .Image.ShouldNotBeNull();

        image.Duotone.ShouldBe(
            new DuotoneRecolour(Colour.FromRgb(0x112255), Colour.FromRgb(0xEEDDAA)));
    }

    [Fact]
    public void APictureStatingNoDuotoneCarriesNone()
    {
        Pictures(0).Count(picture => picture.Image?.Duotone is null).ShouldBe(1);
    }

    [Fact]
    public void APictureFillCarriesItsAlphaModFix()
    {
        List<double> opacities = Slides()[1].Shapes
            .Select(shape => shape.Fill)
            .OfType<BitmapPaint>()
            .Select(paint => paint.Opacity)
            .Order()
            .ToList();

        opacities.ShouldBe([0.5, 1.0]);
    }

    private static IReadOnlyList<PlacedPicture> Pictures(int slide)
        => [.. Slides()[slide].Shapes.Select(shape => shape.Picture).OfType<PlacedPicture>()];

    private static IReadOnlyList<LaidOutSlide> Slides()
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(Deck)));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides;
    }
}

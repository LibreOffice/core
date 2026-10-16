using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.MsBinary.Escher;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Escher picture cropping, from the four properties to the rectangle a slide draws into.
/// </summary>
/// <remarks>
/// <para>
/// <c>cropFromTop</c> and its three siblings (256–259) state 16.16 fixed-point <em>fractions of
/// the picture</em>, not lengths — <c>include/svx/msdffdef.hxx:131</c> says so in its own comment
/// and <c>lcl_ApplyCropping</c> (<c>filter/source/msfilter/msdffimp.cxx:3781-3833</c>) divides
/// each by 65536. A crop becomes a larger destination rectangle plus the clip
/// <see cref="SlideDrawing"/> already applies to every picture, so the whole feature on the
/// binary path is four property reads.
/// </para>
/// <para>
/// <b>The fixture is round-tripped through LibreOffice on purpose.</b>
/// <c>picture-crop.pptx</c> states <c>a:srcRect l="10000" t="20000" r="30000" b="40000"</c> on a
/// picture 288 × 216 pt at (72, 72); <c>picture-crop.ppt</c> is that file converted by
/// <c>soffice --convert-to ppt</c>, which is what turns the <c>a:srcRect</c> into the four
/// Escher properties. Both are read here, so the two paths are held to the same answer and the
/// arithmetic they share cannot drift apart.
/// </para>
/// <para>
/// The numbers are LibreOffice's own. Rendering <c>picture-crop.ppt</c> with the reference at
/// 26.2.4.2 places the picture at <c>(24.38, 36.03)–(505.13, 574.75)</c> in PDF space; we place
/// it at <c>(24.39, 36.01)–(505.13, 574.72)</c>, which is agreement to 0.03 pt on all four
/// edges.
/// </para>
/// </remarks>
public class EscherPictureCropTests
{
    /// <summary>The shape the fixture anchors its picture on: 288 × 216 pt at (72, 72).</summary>
    private static DocRect Anchor => new(
        Length.FromEmu(914400), Length.FromEmu(914400),
        Length.FromEmu(3657600), Length.FromEmu(2743200));

    /// <summary>A crop fraction as the file states it, in 16.16 fixed point.</summary>
    private static uint Fixed(double fraction) => (uint)Math.Round(fraction * 65536);

    // ------------------------------------------------------------------ the property reads

    [Fact]
    public void AShapeStatingNoCropIsPlacedWhereItIs()
        => EscherPicture.Cropped(Table((EscherPropertyIds.Picture, 1)), Anchor).ShouldBe(Anchor);

    [Fact]
    public void EachOfTheFourPropertiesIsRead()
    {
        DocRect placed = EscherPicture.Cropped(
            Table(
                (EscherPropertyIds.CropFromLeft, Fixed(0.10)),
                (EscherPropertyIds.CropFromTop, Fixed(0.20)),
                (EscherPropertyIds.CropFromRight, Fixed(0.30)),
                (EscherPropertyIds.CropFromBottom, Fixed(0.40))),
            Anchor);

        // 1 - 0.1 - 0.3 = 0.6 across, 1 - 0.2 - 0.4 = 0.4 down.
        ((double)placed.Width.Emu).ShouldBe(Anchor.Width.Emu / 0.6, 200);
        ((double)placed.Height.Emu).ShouldBe(Anchor.Height.Emu / 0.4, 200);
        ((double)placed.Left.Emu).ShouldBe(Anchor.Left.Emu - (0.10 * placed.Width.Emu), 200);
        ((double)placed.Top.Emu).ShouldBe(Anchor.Top.Emu - (0.20 * placed.Height.Emu), 200);
    }

    /// <summary>
    /// The property is signed. Read unsigned, a −1% crop is a crop of 65 535 picture-widths and
    /// the shape's picture leaves the slide entirely.
    /// </summary>
    [Fact]
    public void ANegativeCropIsReadAsNegative()
    {
        DocRect placed = EscherPicture.Cropped(
            Table((EscherPropertyIds.CropFromLeft, unchecked((uint)-(int)Fixed(0.25)))), Anchor);

        placed.Width.ShouldBeLessThan(Anchor.Width);
        placed.Left.ShouldBeGreaterThan(Anchor.Left);
    }

    /// <summary>
    /// A crop that keeps nothing is a file's error and not a reason to lose the picture; the
    /// uncropped rectangle is the right picture in the right place.
    /// </summary>
    [Fact]
    public void ACropThatKeepsNothingFallsBackToTheAnchor()
        => EscherPicture.Cropped(
                Table(
                    (EscherPropertyIds.CropFromLeft, Fixed(0.6)),
                    (EscherPropertyIds.CropFromRight, Fixed(0.6))),
                Anchor)
            .ShouldBe(Anchor);

    // ------------------------------------------------------------------ the fractions alone

    /// <summary>
    /// <c>Crop</c> reads the same four properties <c>Cropped</c> does, for the two hosts that
    /// cannot apply them where they read them: a sheet's drawing is anchored to cells and a Word
    /// frame is placed by the layout engine, so neither has a rectangle at read time.
    /// </summary>
    [Fact]
    public void TheFractionsAreTheFourPropertiesInOrder()
    {
        PictureCropFractions crop = EscherPicture.Crop(
            Table(
                (EscherPropertyIds.CropFromLeft, Fixed(0.10)),
                (EscherPropertyIds.CropFromTop, Fixed(0.20)),
                (EscherPropertyIds.CropFromRight, Fixed(0.30)),
                (EscherPropertyIds.CropFromBottom, Fixed(0.40))));

        crop.Left.ShouldBe(0.10, 0.0001);
        crop.Top.ShouldBe(0.20, 0.0001);
        crop.Right.ShouldBe(0.30, 0.0001);
        crop.Bottom.ShouldBe(0.40, 0.0001);
    }

    /// <summary>
    /// And applying them gives what <c>Cropped</c> gives, which is the invariant that stops the
    /// three hosts drifting into two arithmetics.
    /// </summary>
    [Fact]
    public void ApplyingTheFractionsIsCropped()
    {
        EscherPropertyTable table = Table(
            (EscherPropertyIds.CropFromLeft, Fixed(0.10)),
            (EscherPropertyIds.CropFromTop, Fixed(0.20)),
            (EscherPropertyIds.CropFromRight, Fixed(0.30)),
            (EscherPropertyIds.CropFromBottom, Fixed(0.40)));

        EscherPicture.Crop(table).Apply(Anchor).ShouldBe(EscherPicture.Cropped(table, Anchor));
    }

    /// <summary>The fractions are signed too, and for the same reason the rectangle is.</summary>
    [Fact]
    public void ANegativeFractionIsReadAsNegative()
        => EscherPicture.Crop(
                Table((EscherPropertyIds.CropFromLeft, unchecked((uint)-(int)Fixed(0.25)))))
            .Left.ShouldBe(-0.25, 0.0001);

    /// <summary>A table stating no crop reads as no crop, rather than as four zeroes to divide by.</summary>
    [Fact]
    public void AShapeStatingNoCropHasNoFractions()
        => EscherPicture.Crop(Table((EscherPropertyIds.Picture, 1))).IsNone.ShouldBeTrue();

    // ------------------------------------------------------------------ end to end, both paths

    [Fact]
    public void ACroppedPictureInAPptIsDrawnLargerThanItsFrame()
    {
        DocRect destination = OnlyPictureOf("picture-crop.ppt");

        // 288 / 0.6 = 480 pt wide and 216 / 0.4 = 540 pt tall, at (24, −36). The tolerance is
        // the file's own: a .ppt round trip stores the anchor in master units and the crop in
        // 16.16, so the fixture's own fractions come back as 0.0999 rather than 0.1.
        destination.Width.Points.ShouldBe(480, 1.0);
        destination.Height.Points.ShouldBe(540, 1.5);
        destination.Left.Points.ShouldBe(24, 0.5);
        destination.Top.Points.ShouldBe(-36, 1.5);
    }

    /// <summary>
    /// The same document before its conversion, so the shared arithmetic is proved on the path
    /// it came from as well as the one it was ported to.
    /// </summary>
    [Fact]
    public void TheSameCropOnThePptxPathGivesTheSameRectangle()
    {
        DocRect destination = OnlyPictureOf("picture-crop.pptx");

        destination.Width.Points.ShouldBe(480, 0.1);
        destination.Height.Points.ShouldBe(540, 0.1);
        destination.Left.Points.ShouldBe(24, 0.1);
        destination.Top.Points.ShouldBe(-36, 0.1);
    }

    /// <summary>The single picture on the fixture's single slide, and where it is drawn.</summary>
    private static DocRect OnlyPictureOf(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        LaidOutSlide slide = ((SlidePages)((IPaginatedDocument)document).Layout()).Slides[0];

        return slide.Shapes
            .Select(shape => shape.Picture)
            .OfType<PlacedPicture>()
            .ShouldHaveSingleItem()
            .Destination;
    }

    /// <summary>An Escher property table holding exactly the given entries.</summary>
    private static EscherPropertyTable Table(params (ushort Id, uint Value)[] entries)
    {
        byte[] content = new byte[entries.Length * 6];
        for (int i = 0; i < entries.Length; i++)
        {
            content[i * 6] = (byte)entries[i].Id;
            content[(i * 6) + 1] = (byte)(entries[i].Id >> 8);
            content[(i * 6) + 2] = (byte)entries[i].Value;
            content[(i * 6) + 3] = (byte)(entries[i].Value >> 8);
            content[(i * 6) + 4] = (byte)(entries[i].Value >> 16);
            content[(i * 6) + 5] = (byte)(entries[i].Value >> 24);
        }

        return EscherPropertyTable.Read(content, entries.Length);
    }
}

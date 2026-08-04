using Paperless.Core.Graphics;
using Paperless.Rendering.Images;
using Shouldly;

namespace Paperless.Rendering.Tests;

/// <summary>
/// The duotone transform, against LibreOffice 24.2.7.2's own rendering of the same four pixels.
/// </summary>
/// <remarks>
/// <para>
/// The expectations are not derived from the formula. They are the colours <c>soffice</c> put on
/// the page for <c>slide-duotone.pptx</c>, read out of the rendered PDF at 30 dpi: a picture of
/// four rows — black, mid grey, saturated blue, white — under
/// <c>&lt;a:duotone&gt;&lt;a:srgbClr val="112255"/&gt;&lt;a:srgbClr val="EEDDAA"/&gt;</c>.
/// </para>
/// <para>
/// Two details of <c>BitmapDuoToneFilter</c> are load-bearing and both show here. The luminance
/// weights are <c>(B×29 + G×151 + R×76) >> 8</c>, integers summing to 256 rather than the
/// Rec. 601 coefficients they approximate — the blue row is what separates them, at luminance
/// 28 against Rec. 601's 29. And both divisions truncate, which is what puts the grey row at
/// 127/126/127 rather than at 128/127/128.
/// </para>
/// </remarks>
public class DuotoneFilterTests
{
    private static readonly Colour Dark = Colour.FromRgb(0x112255);
    private static readonly Colour Light = Colour.FromRgb(0xEEDDAA);

    [Theory]
    // source                     what LibreOffice draws
    [InlineData(0, 0, 0, 0x11, 0x22, 0x55)]           // black becomes the first colour exactly
    [InlineData(255, 255, 255, 0xEE, 0xDD, 0xAA)]     // white becomes the second
    [InlineData(128, 128, 128, 127, 126, 127)]        // mid grey, both divisions truncating
    [InlineData(0, 0, 255, 41, 54, 93)]               // saturated blue: luminance 28, not 29
    public void APixelLandsWhereTheReferencePutsIt(
        byte red, byte green, byte blue, int expectedRed, int expectedGreen, int expectedBlue)
    {
        RasterImage recoloured = RasterImageDecoder.Ensure(Pixel(red, green, blue)).ShouldNotBeNull();

        recoloured.Pixels.Span[0].ShouldBe((byte)expectedRed);
        recoloured.Pixels.Span[1].ShouldBe((byte)expectedGreen);
        recoloured.Pixels.Span[2].ShouldBe((byte)expectedBlue);
    }

    [Fact]
    public void AlphaSurvivesTheTransform()
    {
        // BitmapDuoToneFilter copies GetAlpha() into the result, which is what keeps a duotoned
        // PNG's cut-out.
        RasterImage recoloured =
            RasterImageDecoder.Ensure(Pixel(10, 20, 30, alpha: 64)).ShouldNotBeNull();

        recoloured.Pixels.Span[3].ShouldBe((byte)64);
    }

    [Fact]
    public void TheTransformIsConsumedOnceAndTheEncodedBytesGoWithIt()
    {
        // The encoded bytes are kept so a JPEG can reach a PDF as DCTDecode without being
        // re-encoded. A recoloured picture must not take that path — it would emit the original
        // and lose the transform on one backend only. Clearing the pending transform is what
        // stops a second Ensure applying it twice.
        RasterImage once = RasterImageDecoder.Ensure(Pixel(128, 128, 128)).ShouldNotBeNull();

        once.Duotone.ShouldBeNull();
        once.EncodedBytes.IsEmpty.ShouldBeTrue();

        RasterImage twice = RasterImageDecoder.Ensure(once).ShouldNotBeNull();
        twice.Pixels.Span[0].ShouldBe((byte)127);
    }

    [Fact]
    public void APictureWithNoDuotoneKeepsItsBytes()
    {
        RasterImage plain = new()
        {
            Width = 1,
            Height = 1,
            Pixels = new byte[] { 10, 20, 30, 255 },
            EncodedBytes = new byte[] { 1, 2, 3 },
        };

        RasterImage same = RasterImageDecoder.Ensure(plain).ShouldNotBeNull();

        same.Pixels.Span[0].ShouldBe((byte)10);
        same.EncodedBytes.Length.ShouldBe(3);
    }

    /// <summary>One straight-RGBA pixel with the transform still pending on it.</summary>
    private static RasterImage Pixel(byte red, byte green, byte blue, byte alpha = 255)
        => new()
        {
            Width = 1,
            Height = 1,
            Pixels = new[] { red, green, blue, alpha },
            EncodedBytes = new byte[] { 0xFF, 0xD8, 0xFF },
            EncodedMediaType = "image/jpeg",
            Duotone = new DuotoneRecolour(Dark, Light),
        };
}

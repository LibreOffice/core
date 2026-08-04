using System.Runtime.InteropServices;
using Paperless.Core.Graphics;
using SkiaSharp;

namespace Paperless.Rendering.Images;

/// <summary>
/// Decodes an embedded picture into the display list's <see cref="RasterImage"/>.
/// </summary>
/// <remarks>
/// <para>
/// <b>Why it lives here.</b> <c>Paperless.Core</c> holds the drawing IR and has no external
/// dependencies, so it can describe a decoded image but cannot decode one. The codecs
/// LibreOffice reaches for are libpng, libjpeg-turbo, giflib and libwebp
/// (<c>vcl/source/filter/</c>, twenty-two format directories, most of them thin wrappers —
/// see <c>research/06-rendering.md</c> §D.1); Skia already carries the same set, and Skia is
/// already a dependency of this library and of nothing above it. So the decoder sits beside
/// the backends, and what crosses back into Core is a buffer of bytes with no Skia in its
/// signature.
/// </para>
/// <para>
/// <b>Straight alpha, not premultiplied.</b> <see cref="RasterImage"/> says so and PDF needs
/// it: an <c>/SMask</c> is a separate greyscale image whose samples are the alpha, and
/// dividing premultiplied colour back out loses precision exactly where the alpha is low.
/// Skia converts on readback, which is why the pixels are read into an explicitly
/// unpremultiplied image rather than copied out of the decoded bitmap.
/// </para>
/// <para>
/// <b>The original bytes are kept.</b> They cost nothing — <c>ReadOnlyMemory</c> is a view
/// over the caller's buffer — and they are what lets the PDF writer copy a JPEG through as
/// <c>DCTDecode</c> instead of decoding and re-encoding it, which LibreOffice also does
/// (<c>vcl/source/pdf/pdfextoutdevdata.cxx:439</c>) and which is both faster and lossless.
/// </para>
/// </remarks>
public static class RasterImageDecoder
{
    /// <summary>
    /// Decodes a picture, or returns null when the bytes are not one this can read.
    /// </summary>
    /// <param name="encoded">The picture as it was stored in the document.</param>
    /// <param name="declaredMediaType">
    /// What the document said it was, if anything. Used only when the bytes themselves do not
    /// say — a mislabelled part is common enough that the content wins, which is the same rule
    /// format identification follows.
    /// </param>
    /// <remarks>
    /// Null rather than an exception, because a picture that will not decode is a document
    /// defect and not a reason to fail a rendering: the page still has its text, its tables and
    /// every other shape on it. A caller that wants to say so records a <c>Diagnostic</c>.
    /// </remarks>
    public static RasterImage? Decode(ReadOnlyMemory<byte> encoded, string? declaredMediaType = null)
    {
        if (encoded.IsEmpty) return null;

        string? mediaType = Sniff(encoded.Span) ?? declaredMediaType;

        try
        {
            using SKImage? image = FromEncoded(encoded.Span);
            if (image is null || image.Width <= 0 || image.Height <= 0) return null;

            SKImageInfo info = new(image.Width, image.Height, SKColorType.Rgba8888, SKAlphaType.Unpremul);
            byte[] pixels = new byte[checked(info.RowBytes * info.Height)];

            GCHandle pin = GCHandle.Alloc(pixels, GCHandleType.Pinned);
            try
            {
                if (!image.ReadPixels(info, pin.AddrOfPinnedObject(), info.RowBytes, 0, 0)) return null;
            }
            finally
            {
                pin.Free();
            }

            return new RasterImage
            {
                Width = image.Width,
                Height = image.Height,
                Pixels = pixels,
                EncodedBytes = encoded,
                EncodedMediaType = mediaType,
            };
        }
        catch (Exception exception) when (exception is OutOfMemoryException or OverflowException)
        {
            // A header claiming a gigapixel image is a defence problem, not a decode failure. The
            // allocation is what catches it, since nothing else knows how large the pixels will be
            // until the header has been believed.
            return null;
        }
    }

    /// <summary>
    /// The media type the bytes themselves say they are, or null.
    /// </summary>
    /// <remarks>
    /// <para>
    /// By content and never by the part name, which is the same rule format identification
    /// follows and for the same reason: a <c>.png</c> holding a JPEG is common in real
    /// documents, and believing the name would cost the pass-through — or worse, write a JPEG
    /// into a PDF claiming to be deflated RGB.
    /// </para>
    /// <para>
    /// Only the formats Skia decodes are recognised. A TIFF or an EPS returns null here and
    /// fails to decode, which is correct: LibreOffice reads both through codecs Skia does not
    /// carry, and reporting a media type for a picture that will not draw would be worse than
    /// saying nothing.
    /// </para>
    /// </remarks>
    /// <summary>
    /// The image with its pixels available, decoding it first if a reader handed it over
    /// still encoded.
    /// </summary>
    /// <remarks>
    /// Readers emit <see cref="RasterImage.Encoded"/> rather than decoding, so that nothing
    /// on the extraction path depends on a codec. Every backend that wants pixels therefore
    /// has to come through here first. Returns null when the bytes are not an image this
    /// decoder knows, which is a document to draw without the picture rather than a document
    /// that fails to draw.
    /// </remarks>
    public static RasterImage? Ensure(RasterImage image)
    {
        ArgumentNullException.ThrowIfNull(image);

        if (image.IsDecoded) return Recoloured(image);
        if (image.EncodedBytes.IsEmpty) return null;

        RasterImage? decoded = Decode(image.EncodedBytes, image.EncodedMediaType);

        // The original bytes are kept, so a JPEG can still pass through to DCTDecode
        // untouched even though it had to be decoded here to learn its size.
        return decoded is null
            ? null
            : Recoloured(decoded with
              {
                  EncodedBytes = image.EncodedBytes,
                  EncodedMediaType = image.EncodedMediaType,
                  Duotone = image.Duotone,
              });
    }

    /// <summary>
    /// A decoded image with its pending <see cref="RasterImage.Duotone"/> carried out.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>BitmapDuoToneFilter</c> (<c>vcl/source/bitmap/BitmapDuoToneFilter.cxx</c>) replaces
    /// each channel with <c>light × l / 255 + dark × (255 − l) / 255</c>, where <c>l</c> is
    /// the pixel's luminance and both divisions truncate. Alpha is carried through untouched,
    /// which is what makes a duotoned PNG keep its cut-out.
    /// </para>
    /// <para>
    /// <strong>The encoded bytes have to go.</strong> They are kept so a JPEG can reach a PDF
    /// as <c>DCTDecode</c> without being re-encoded, and that pass-through would emit the
    /// original picture and lose the recolouring entirely — the failure would look like the
    /// transform not being implemented, on the one backend where it is hardest to see.
    /// </para>
    /// </remarks>
    private static RasterImage Recoloured(RasterImage image)
    {
        if (image.Duotone is not { } duotone || !image.IsDecoded) return image;

        ReadOnlySpan<byte> source = image.Pixels.Span;
        byte[] pixels = new byte[source.Length];
        source.CopyTo(pixels);

        for (int i = 0; i + 3 < pixels.Length; i += 4)
        {
            int luminance = Luminance(pixels[i], pixels[i + 1], pixels[i + 2]);
            pixels[i] = Mix(luminance, duotone.Dark.R, duotone.Light.R);
            pixels[i + 1] = Mix(luminance, duotone.Dark.G, duotone.Light.G);
            pixels[i + 2] = Mix(luminance, duotone.Dark.B, duotone.Light.B);
        }

        return image with { Pixels = pixels, EncodedBytes = default, Duotone = null };
    }

    /// <summary>
    /// A pixel's brightness on VCL's scale.
    /// </summary>
    /// <remarks>
    /// <c>BitmapColor::GetLuminance</c> is <c>(B × 29 + G × 151 + R × 76) >> 8</c>
    /// (<c>include/vcl/BitmapColor.hxx</c>) — integer weights summing to 256, not the
    /// Rec. 601 coefficients they approximate. Using 0.299/0.587/0.114 instead is within one
    /// step per channel and drifts the two renderers apart on a large flat area, which is
    /// exactly what a duotoned background is.
    /// </remarks>
    private static int Luminance(byte red, byte green, byte blue)
        => ((blue * 29) + (green * 151) + (red * 76)) >> 8;

    /// <summary>One channel of the ramp, with LibreOffice's two truncating divisions.</summary>
    private static byte Mix(int luminance, byte dark, byte light)
        => (byte)((light * luminance / 0xFF) + (dark * (0xFF - luminance) / 0xFF));

    public static string? Sniff(ReadOnlySpan<byte> encoded)
    {
        // Spelled as bytes rather than as a u8 literal: PNG's first byte is 0x89, and a u8 literal
        // would encode that as the two bytes UTF-8 uses for U+0089 and never match anything.
        ReadOnlySpan<byte> png = [0x89, (byte)'P', (byte)'N', (byte)'G', 0x0D, 0x0A, 0x1A, 0x0A];
        if (encoded.Length >= 8 && encoded[..8].SequenceEqual(png)) return "image/png";
        if (encoded.Length >= 3 && encoded[0] == 0xFF && encoded[1] == 0xD8 && encoded[2] == 0xFF) return "image/jpeg";
        if (encoded.Length >= 6 && (encoded[..6].SequenceEqual("GIF87a"u8) || encoded[..6].SequenceEqual("GIF89a"u8)))
        {
            return "image/gif";
        }

        if (encoded.Length >= 2 && encoded[0] == 'B' && encoded[1] == 'M') return "image/bmp";
        if (encoded.Length >= 12 && encoded[..4].SequenceEqual("RIFF"u8) && encoded[8..12].SequenceEqual("WEBP"u8))
        {
            return "image/webp";
        }

        // An icon's header is four bytes of which two are a constant zero, so it is checked last:
        // it is the weakest signature here and would claim other formats' bytes if checked first.
        if (encoded.Length >= 4 && encoded[0] == 0 && encoded[1] == 0 && encoded[2] is 1 or 2 && encoded[3] == 0)
        {
            return "image/vnd.microsoft.icon";
        }

        return null;
    }

    private static SKImage? FromEncoded(ReadOnlySpan<byte> encoded)
    {
        // Copied into SKData rather than wrapped, because an SKImage decoded lazily would read the
        // span after this call has returned.
        using SKData data = SKData.CreateCopy(encoded.ToArray());
        return SKImage.FromEncodedData(data);
    }
}

using System.Globalization;
using Paperless.Core.Graphics;

namespace Paperless.Rendering.Pdf;

/// <summary>
/// Writes a decoded raster as a PDF image XObject.
/// </summary>
/// <remarks>
/// <para>
/// Two shapes, and which one is used is the difference between a fast lossless copy and a
/// slow lossy one. A JPEG that arrived compressed is written straight through as
/// <c>DCTDecode</c>: PDF's image filter and JPEG's are the same thing, so re-encoding the
/// pixels would spend time to produce a larger, worse image. Anything else is deflated
/// eight-bit RGB.
/// </para>
/// <para>
/// Transparency is a separate <c>/SMask</c> image rather than a fourth channel, because
/// PDF has no RGBA colour space. The mask is written only when some pixel is not opaque,
/// which keeps the common case — every office picture without an alpha channel — to one
/// stream.
/// </para>
/// </remarks>
internal static class PdfImages
{
    /// <summary>Writes an image and returns the resource name that draws it.</summary>
    public static string Write(
        PdfDocumentWriter writer,
        RasterImage image,
        PdfRenderOptions options,
        List<(string Name, int Id)> xObjects)
    {
        ArgumentNullException.ThrowIfNull(writer);
        ArgumentNullException.ThrowIfNull(image);
        ArgumentNullException.ThrowIfNull(options);
        ArgumentNullException.ThrowIfNull(xObjects);

        // A reader may hand over an undecoded image; the geometry below needs its size, and
        // the JPEG pass-through still uses the original bytes that `Ensure` carries forward.
        if (Images.RasterImageDecoder.Ensure(image) is not { } decoded) return string.Empty;
        image = decoded;

        string name = string.Create(CultureInfo.InvariantCulture, $"Im{xObjects.Count + 1}");
        int id = writer.Reserve();
        xObjects.Add((name, id));

        string geometry =
            $"/Type/XObject/Subtype/Image/Width {image.Width}/Height {image.Height}"
            + "/ColorSpace/DeviceRGB/BitsPerComponent 8";

        if (options.PassThroughJpeg
            && image.EncodedMediaType is "image/jpeg"
            && !image.EncodedBytes.IsEmpty)
        {
            writer.SetStream(id, geometry + "/Filter/DCTDecode", image.EncodedBytes.Span, compress: false);
            return name;
        }

        int pixels = image.Width * image.Height;
        ReadOnlySpan<byte> source = image.Pixels.Span;
        if (source.Length < pixels * 4)
        {
            writer.Set(id, "null");
            return string.Empty;
        }

        byte[] rgb = new byte[pixels * 3];
        byte[] alpha = new byte[pixels];
        bool transparent = false;

        for (int i = 0; i < pixels; i++)
        {
            rgb[(i * 3) + 0] = source[(i * 4) + 0];
            rgb[(i * 3) + 1] = source[(i * 4) + 1];
            rgb[(i * 3) + 2] = source[(i * 4) + 2];
            alpha[i] = source[(i * 4) + 3];
            if (alpha[i] != 255) transparent = true;
        }

        string mask = string.Empty;
        if (transparent)
        {
            int maskId = writer.Reserve();
            writer.SetStream(
                maskId,
                $"/Type/XObject/Subtype/Image/Width {image.Width}/Height {image.Height}"
                + "/ColorSpace/DeviceGray/BitsPerComponent 8",
                alpha,
                compress: true);

            mask = $"/SMask {maskId} 0 R";
        }

        writer.SetStream(id, geometry + mask, rgb, compress: true);
        return name;
    }
}

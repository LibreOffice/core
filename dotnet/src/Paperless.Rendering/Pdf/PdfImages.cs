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

        // The pass-through hands the file's own bytes to DCTDecode, so the colour space it declares
        // has to be the one the JPEG really carries rather than the one the decoded path produces.
        // Only greyscale and three-channel colour are passed through; anything else falls through to
        // be decoded, which is the safe answer for a CMYK JPEG whose inversion depends on an Adobe
        // marker this writer has no reason to interpret.
        int components = options.PassThroughJpeg
            && image.EncodedMediaType is "image/jpeg"
            && !image.EncodedBytes.IsEmpty
            ? JpegComponents(image.EncodedBytes.Span)
            : -1;

        if (components is 1 or 3)
        {
            string colour = components == 1
                ? $"/Type/XObject/Subtype/Image/Width {image.Width}/Height {image.Height}"
                  + "/ColorSpace/DeviceGray/BitsPerComponent 8"
                : geometry;

            writer.SetStream(id, colour + "/Filter/DCTDecode", image.EncodedBytes.Span, compress: false);
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

    /// <summary>
    /// How many colour components a JPEG's frame header states, or -1 when none can be read.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A JPEG passed through to <c>DCTDecode</c> takes its colour space from the PDF, not
    /// from itself</strong>, so a greyscale JPEG announced as <c>/DeviceRGB</c> is read three samples
    /// at a time out of a stream that has one — the image comes out repeated across the row and
    /// squashed down the page, which looks like a decoder fault rather than a metadata one. Measured
    /// on <c>omrIMInterpretiveGuideLine.doc</c>, whose departmental seal is a 635×638 one-component
    /// JPEG and drew as three squashed seals.
    /// </para>
    /// <para>
    /// Only the <c>SOF</c> marker is looked for, and only its component count is taken. Every marker
    /// except the standalone ones carries a two-byte length, so the scan is a walk over segment
    /// headers; it stops at <c>SOS</c>, since entropy-coded data is not segmented and reading it as
    /// though it were finds markers that are not there.
    /// </para>
    /// </remarks>
    private static int JpegComponents(ReadOnlySpan<byte> jpeg)
    {
        if (jpeg.Length < 4 || jpeg[0] != 0xFF || jpeg[1] != 0xD8) return -1;

        int at = 2;
        while (at + 3 < jpeg.Length)
        {
            if (jpeg[at] != 0xFF) return -1;

            byte marker = jpeg[at + 1];

            // Fill bytes: a marker may be preceded by any number of 0xFF.
            if (marker == 0xFF) { at++; continue; }

            // The standalone markers, which carry no length: TEM, RSTn, SOI, EOI.
            if (marker == 0x01 || marker is >= 0xD0 and <= 0xD9) { at += 2; continue; }

            // Start of scan, and past it the entropy-coded data. Nothing after it is a segment.
            if (marker == 0xDA) return -1;

            int length = (jpeg[at + 2] << 8) | jpeg[at + 3];
            if (length < 2) return -1;

            // Every SOF except DHT (0xC4), DNL (0xC8) and DAC (0xCC) states the frame's geometry,
            // and the component count is the ninth byte of the segment.
            bool isFrame = marker is >= 0xC0 and <= 0xCF and not (0xC4 or 0xC8 or 0xCC);
            if (isFrame)
            {
                return at + 9 < jpeg.Length && length >= 8 ? jpeg[at + 9] : -1;
            }

            at += 2 + length;
        }

        return -1;
    }
}

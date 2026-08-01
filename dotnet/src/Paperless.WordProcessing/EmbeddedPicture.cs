using Paperless.Core.Diagnostics;
using Paperless.Core.Graphics;

namespace Paperless.WordProcessing;

/// <summary>
/// What a document's picture bytes are, and which of them a raster backend can be handed.
/// </summary>
/// <remarks>
/// <para>
/// One place for a question all four front ends ask, because all four answer it from the bytes rather
/// than from what the file called them. ODF names a picture by a package entry whose extension is
/// decorative, DOCX by a relationship to a part whose name is a producer's choice, RTF by a control word
/// that its own exporters get wrong, and DOC by an Escher blip record whose type is the only honest
/// label of the four. Sniffing is what the format catalogue does for whole documents and for the same
/// reason: a <c>.png</c> holding a JPEG is common, and believing the name costs the PDF writer its
/// pass-through — or worse, writes a JPEG into a PDF declared as deflated RGB.
/// </para>
/// <para>
/// <strong>Nothing here decodes.</strong> The bytes go into <see cref="RasterImage.Encoded"/> and stop;
/// whichever backend wants pixels asks <c>RasterImageDecoder.Ensure</c> for them. A reader that decoded
/// would put a codec on the extraction path, which the layering forbids — <c>Paperless.Rendering</c> is
/// below nothing that reads a document.
/// </para>
/// <para>
/// The signature table is deliberately a near-duplicate of <c>RasterImageDecoder.Sniff</c>'s rather than
/// a call to it. That method lives in <c>Paperless.Rendering</c> beside the Skia decoder it guards, and
/// referencing it from here would create exactly the dependency the previous paragraph exists to avoid.
/// The two lists are allowed to differ, and one difference is intended: this one also recognises the
/// vector formats, which the decoder has no reason to name because it cannot decode any of them.
/// </para>
/// </remarks>
internal static class EmbeddedPicture
{
    /// <summary>
    /// The picture some bytes hold, or null when they are not one a raster backend can draw.
    /// </summary>
    /// <remarks>
    /// A metafile, an unrecognised blob and an empty part all give null, and the first two also leave a
    /// <see cref="Diagnostic"/> behind. Null rather than an exception in every case: a picture that
    /// cannot be drawn is a page that still has its text, its tables and every other shape on it, which
    /// is the leniency rule the whole library reads by.
    /// </remarks>
    /// <param name="bytes">The picture exactly as the document stored it.</param>
    /// <param name="declaredMediaType">What the document said it was, where it said anything.</param>
    /// <param name="where">What to call the picture in a diagnostic: a part name, a frame name.</param>
    /// <param name="diagnostics">Where to record a picture that will not draw, or null to say nothing.</param>
    public static RasterImage? Read(
        ReadOnlyMemory<byte> bytes,
        string? declaredMediaType,
        string? where,
        List<Diagnostic>? diagnostics)
    {
        if (bytes.IsEmpty) return null;

        if (Raster(bytes.Span) is { } mediaType)
        {
            (int width, int height) = Dimensions(bytes.Span);

            return RasterImage.Encoded(bytes, mediaType) with { Width = width, Height = height };
        }

        if (Vector(bytes.Span) is { } vector)
        {
            diagnostics?.Add(new Diagnostic(
                DiagnosticSeverity.Information, "PL2370",
                $"A {vector} picture was found and has not been drawn: vector import is not "
                + "implemented, so the frame reserves its room and stays empty.",
                where is null ? null : new DiagnosticLocation(where)));

            return null;
        }

        diagnostics?.Add(new Diagnostic(
            DiagnosticSeverity.Warning, "PL2371",
            "A picture's bytes are in no format this library recognises, so the frame reserves its "
            + "room and stays empty."
            + (declaredMediaType is null ? "" : $" The document declared them as '{declaredMediaType}'."),
            where is null ? null : new DiagnosticLocation(where)));

        return null;
    }

    /// <summary>
    /// A picture's size in pixels, read out of its header without decoding it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice does exactly this and keeps it apart from decoding for exactly this reason:
    /// <c>GraphicDescriptor</c> (<c>vcl/source/filter/graphicfilter2.cxx:63</c>) sniffs GIF, JPEG, BMP
    /// and PNG in turn and reports the format and the pixel size from the header alone, so that a
    /// caller can say how large a picture is without paying a codec for it. A reader wants the same
    /// thing — "a 4000 × 3000 photograph scaled into two centimetres" is answerable from twenty-four
    /// bytes — and it is the one property of a picture that costs nothing.
    /// </para>
    /// <para>
    /// It is <em>not</em> what makes a picture draw, and that distinction was worth a wrong diagnosis
    /// once. Both <c>SkiaDrawingSink.DrawImage</c> and <c>PdfContentSink.DrawImage</c> used to open with
    /// <c>if (image.Width &lt;= 0 || image.Height &lt;= 0) return;</c> and then decode, which dropped every
    /// <c>RasterImage.Encoded</c> before the decoder beneath it was reached — laying out correctly and
    /// drawing nothing. Both now test emptiness rather than size, so a picture whose header this does not
    /// read still draws; the zeroes it returns for one cost only the answer to "how large is it".
    /// </para>
    /// </remarks>
    public static (int Width, int Height) Dimensions(ReadOnlySpan<byte> bytes)
    {
        // PNG: the IHDR chunk is fixed at offset 8 by the specification — signature, length, type,
        // then the two big-endian dimensions. It is always the first chunk, so no walk is needed.
        if (bytes.Length >= 24 && bytes[12] == 'I' && bytes[13] == 'H' && bytes[14] == 'D' && bytes[15] == 'R')
        {
            return (BigEndian(bytes[16..]), BigEndian(bytes[20..]));
        }

        if (bytes.Length >= 10 && (bytes[..3].SequenceEqual("GIF"u8)))
        {
            return (LittleEndian16(bytes[6..]), LittleEndian16(bytes[8..]));
        }

        if (bytes.Length >= 26 && bytes[0] == 'B' && bytes[1] == 'M')
        {
            // The DIB header at 14. Only BITMAPINFOHEADER and its successors, whose width and height
            // are signed 32-bit at 4 and 8; a negative height means the rows are stored top down,
            // which changes nothing about how large the picture is.
            int header = LittleEndian32(bytes[14..]);
            if (header >= 40)
            {
                return (Math.Abs(LittleEndian32(bytes[18..])), Math.Abs(LittleEndian32(bytes[22..])));
            }
        }

        if (bytes.Length >= 4 && bytes[0] == 0xFF && bytes[1] == 0xD8) return Jpeg(bytes);

        return (0, 0);
    }

    /// <summary>
    /// A JPEG's size, from the first start-of-frame marker.
    /// </summary>
    /// <remarks>
    /// A walk rather than a fixed offset, because a JPEG's frame header sits after however many
    /// application, comment and quantisation segments the producer wrote — a file from a camera can
    /// carry sixty kilobytes of EXIF before it. The marker set is the one
    /// <c>ImpDetectJPG_GetNextMarker</c> uses (<c>graphicfilter2.cxx:112</c>): every
    /// <c>0xC0</c>–<c>0xCF</c> is a start of frame <em>except</em> <c>0xC4</c>, <c>0xC8</c> and
    /// <c>0xCC</c>, which are the Huffman, extension and arithmetic-coding tables and carry no size.
    /// </remarks>
    private static (int Width, int Height) Jpeg(ReadOnlySpan<byte> bytes)
    {
        int at = 2;

        while (at + 4 <= bytes.Length)
        {
            if (bytes[at] != 0xFF) return (0, 0);

            // Fill bytes: a marker may be preceded by any number of extra 0xFF octets.
            int marker = at + 1;
            while (marker < bytes.Length && bytes[marker] == 0xFF) marker++;
            if (marker >= bytes.Length) return (0, 0);

            byte code = bytes[marker];

            // The standalone markers, which carry no length word: restart, start of image, end of
            // image, and the private 0x01. Anything else is followed by a two-byte segment length.
            if (code is 0xD8 or 0x01 || (code >= 0xD0 && code <= 0xD9))
            {
                at = marker + 1;
                continue;
            }

            if (marker + 3 > bytes.Length) return (0, 0);
            int length = BigEndian16(bytes[(marker + 1)..]);
            if (length < 2) return (0, 0);

            if (code >= 0xC0 && code <= 0xCF && code is not (0xC4 or 0xC8 or 0xCC))
            {
                // Precision, then height, then width — in that order, which is the one thing about
                // this structure that is easy to write backwards.
                int frame = marker + 3;
                if (frame + 5 > bytes.Length) return (0, 0);

                return (BigEndian16(bytes[(frame + 3)..]), BigEndian16(bytes[(frame + 1)..]));
            }

            at = marker + 1 + length;
        }

        return (0, 0);
    }

    private static int BigEndian(ReadOnlySpan<byte> bytes)
        => (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];

    private static int BigEndian16(ReadOnlySpan<byte> bytes) => (bytes[0] << 8) | bytes[1];

    private static int LittleEndian16(ReadOnlySpan<byte> bytes) => bytes[0] | (bytes[1] << 8);

    private static int LittleEndian32(ReadOnlySpan<byte> bytes)
        => bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);

    /// <summary>
    /// The media type of a raster the backends can decode, or null for anything else.
    /// </summary>
    /// <remarks>
    /// Only what Skia carries, since that is what will be asked to decode it. A TIFF or an EPS is a real
    /// picture that LibreOffice reads through a codec Skia does not have, and claiming a media type for
    /// one would produce a PDF image object that no reader can render — worse than the empty frame it
    /// gets instead.
    /// </remarks>
    public static string? Raster(ReadOnlySpan<byte> bytes)
    {
        // Spelled as bytes rather than as a u8 literal: PNG's first byte is 0x89, and a u8 literal would
        // encode that as the two bytes UTF-8 uses for U+0089 and never match anything.
        ReadOnlySpan<byte> png = [0x89, (byte)'P', (byte)'N', (byte)'G', 0x0D, 0x0A, 0x1A, 0x0A];
        if (bytes.Length >= 8 && bytes[..8].SequenceEqual(png)) return "image/png";

        if (bytes.Length >= 3 && bytes[0] == 0xFF && bytes[1] == 0xD8 && bytes[2] == 0xFF)
        {
            return "image/jpeg";
        }

        if (bytes.Length >= 6
            && (bytes[..6].SequenceEqual("GIF87a"u8) || bytes[..6].SequenceEqual("GIF89a"u8)))
        {
            return "image/gif";
        }

        if (bytes.Length >= 12 && bytes[..4].SequenceEqual("RIFF"u8) && bytes[8..12].SequenceEqual("WEBP"u8))
        {
            return "image/webp";
        }

        // Last of the raster signatures because it is the weakest — two ASCII letters — and would claim
        // the first two bytes of something else if it were checked first.
        if (bytes.Length >= 2 && bytes[0] == 'B' && bytes[1] == 'M') return "image/bmp";

        return null;
    }

    /// <summary>
    /// The name of the vector format some bytes are, or null when they are not one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Recognised so that a metafile can be declined by name rather than as an unreadable blob, which is
    /// the difference between "not implemented yet" and "this document is broken". Real <c>.docx</c> and
    /// <c>.doc</c> files embed these constantly — every chart, every pasted Visio drawing, every clip-art
    /// arrow — so the message a caller gets for one is worth getting right.
    /// </para>
    /// <para>
    /// EMF is identified by its type <em>and</em> its signature, at offsets 0 and 40, because the leading
    /// <c>0x00000001</c> alone is four bytes that any file could start with; the <c>" EMF"</c> at 40 is
    /// what <c>EMR_HEADER</c> guarantees. WMF has two forms and both are checked: the placeable header
    /// Aldus prepended, and the bare <c>METAHEADER</c> whose first four bytes are a memory/disk type and
    /// a fixed header size of nine words.
    /// </para>
    /// </remarks>
    public static string? Vector(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length >= 44
            && bytes[0] == 0x01 && bytes[1] == 0 && bytes[2] == 0 && bytes[3] == 0
            && bytes[40..44].SequenceEqual(" EMF"u8))
        {
            return "EMF";
        }

        // The placeable header: 0x9AC6CDD7 little-endian, which is what every WMF written for exchange
        // carries and what RTF's \wmetafile bytes begin with when a producer includes it.
        if (bytes.Length >= 4
            && bytes[0] == 0xD7 && bytes[1] == 0xCD && bytes[2] == 0xC6 && bytes[3] == 0x9A)
        {
            return "WMF";
        }

        // The bare METAHEADER: mtType is 1 (memory) or 2 (disk) and mtHeaderSize is always 9 words.
        if (bytes.Length >= 6
            && bytes[1] == 0 && bytes[0] is 1 or 2 && bytes[2] == 9 && bytes[3] == 0)
        {
            return "WMF";
        }

        // StarView's own metafile, which LibreOffice writes into ODF packages for a shape it has
        // rendered itself. Nothing else produces one, and nothing but LibreOffice reads one.
        if (bytes.Length >= 6 && bytes[..6].SequenceEqual("VCLMTF"u8)) return "SVM";

        if (IsSvg(bytes)) return "SVG";

        return null;
    }

    /// <summary>
    /// Whether some bytes are an SVG document.
    /// </summary>
    /// <remarks>
    /// By looking for the root element within the first few hundred bytes rather than at offset zero,
    /// because an SVG legally begins with an XML declaration, a byte-order mark, a doctype, a comment or
    /// any amount of white space between them. The window is bounded so that a large non-XML file is not
    /// scanned end to end to answer no.
    /// </remarks>
    private static bool IsSvg(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length < 4) return false;

        ReadOnlySpan<byte> head = bytes[..Math.Min(bytes.Length, 512)];
        ReadOnlySpan<byte> bom = [0xEF, 0xBB, 0xBF];

        if (head[0] is not ((byte)'<' or (byte)' ' or (byte)'\n' or (byte)'\r' or (byte)'\t')
            && !head.StartsWith(bom))
        {
            return false;
        }

        return head.IndexOf("<svg"u8) >= 0;
    }
}

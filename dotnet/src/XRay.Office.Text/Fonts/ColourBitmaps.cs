using System.Buffers.Binary;

namespace XRay.Office.Text.Fonts;

/// <summary>
/// One glyph's colour bitmap, taken from the strike the face stores it in.
/// </summary>
/// <remarks>
/// <para>
/// A colour bitmap face has no outline for the glyph at all — <c>NotoColorEmoji.ttf</c> carries
/// <c>CBDT</c> and <c>CBLC</c> and neither <c>glyf</c> nor <c>CFF </c> — so this is the only thing
/// that can put ink on the page for it. The payload is a whole image file, almost always a PNG,
/// which is why nothing here decodes: the bytes go to whichever backend has a codec, exactly as
/// <see cref="Core.Graphics.RasterImage.Encoded"/> already carries a picture a reader has not
/// decoded.
/// </para>
/// <para>
/// The metrics are in <em>pixels of the strike</em>, not in design units, and
/// <see cref="PlacementIn"/> is what converts them. Keeping the two apart matters because a face
/// may hold several strikes at different resolutions and the same glyph then has different pixel
/// metrics in each while occupying the same place on the em.
/// </para>
/// </remarks>
public sealed record ColourBitmap
{
    /// <summary>The image file exactly as the face stores it.</summary>
    public required ReadOnlyMemory<byte> Image { get; init; }

    /// <summary>The media type of <see cref="Image"/>; <c>image/png</c> for every colour strike.</summary>
    public required string MediaType { get; init; }

    /// <summary>The strike's horizontal resolution, in pixels per em.</summary>
    public required int PixelsPerEmX { get; init; }

    /// <summary>The strike's vertical resolution, in pixels per em.</summary>
    public required int PixelsPerEmY { get; init; }

    /// <summary>The bitmap's width in pixels.</summary>
    public required int PixelWidth { get; init; }

    /// <summary>The bitmap's height in pixels.</summary>
    public required int PixelHeight { get; init; }

    /// <summary>How far right of the pen the bitmap's left edge sits, in pixels.</summary>
    public required int BearingX { get; init; }

    /// <summary>How far above the baseline the bitmap's <em>top</em> edge sits, in pixels.</summary>
    public required int BearingY { get; init; }

    /// <summary>
    /// Where the bitmap goes on the em, in the face's own design units.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Each of the four pixel measurements is scaled by <c>unitsPerEm / ppem</c> and rounded to a
    /// whole design unit, and the box is then <em>closed</em> from the rounded top and the rounded
    /// height rather than by rounding the bottom separately — so a box is exactly as tall as its
    /// own stated height whatever the rounding did.
    /// </para>
    /// <para>
    /// Verified against LibreOffice 26.2.4.2's own PDF of a <c>U+2714</c> probe, which draws the
    /// glyph in a Type 3 char proc as
    /// <c>q 1247.55859375 0 0 1174.31640625 0 -247.55859375 cm /Im12 Do Q</c> under a
    /// <c>/FontMatrix[0.001 …]</c>. Noto Color Emoji is 2048 units per em with one 109 ppem strike
    /// whose glyphs are 136 × 128 pixels at <c>bearingX 0, bearingY 101</c>, so those three numbers
    /// are 2555, 2405 and −507 design units — and <c>2555 × 1000/2048</c>, <c>2405 × 1000/2048</c>
    /// and <c>−507 × 1000/2048</c> are the reference's three constants to the last digit. Rounding
    /// rather than truncating is what the pair settles: the width is 2555.30 units and the height
    /// 2404.99, so one of them rounds down and the other up, and truncation would have written
    /// 2404.
    /// </para>
    /// </remarks>
    /// <param name="unitsPerEm">The face's design units per em.</param>
    /// <returns>The box in design units, y growing upward from the baseline.</returns>
    public (int Left, int Bottom, int Width, int Height) PlacementIn(int unitsPerEm)
    {
        int left = Scaled(BearingX, PixelsPerEmX);
        int top = Scaled(BearingY, PixelsPerEmY);
        int width = Scaled(PixelWidth, PixelsPerEmX);
        int height = Scaled(PixelHeight, PixelsPerEmY);

        return (left, top - height, width, height);

        int Scaled(int pixels, int perEm) => perEm <= 0
            ? 0
            : (int)Math.Round(pixels * (double)unitsPerEm / perEm, MidpointRounding.AwayFromZero);
    }
}

/// <summary>
/// The <c>CBLC</c>/<c>CBDT</c> tables: a whole image file per glyph, per strike.
/// </summary>
/// <remarks>
/// <para>
/// <strong>This is the half of colour fonts that the corpus actually reaches.</strong> Of every
/// face installed here exactly one is a colour face — <c>NotoColorEmoji.ttf</c>, which is
/// <c>CBDT</c>/<c>CBLC</c> — and no installed face carries <c>COLR</c>/<c>CPAL</c> at all, so
/// layered colour glyphs are deferred rather than implemented; see
/// <see cref="GlyphPainting.CanPaint"/> for what happens instead when one is met.
/// </para>
/// <para>
/// <c>CBLC</c> is <c>EBLC</c> with colour strikes, so the structures are the embedded-bitmap ones:
/// a <c>BitmapSize</c> per strike, an index subtable array per strike, and a <c>CBDT</c> offset per
/// glyph. Index subtable formats 1 to 5 are all read; image formats 17, 18 and 19 — the three that
/// wrap a whole PNG — are the ones that can carry colour, and the bit-aligned monochrome formats
/// are deliberately not read because a face using them is not a colour face and its glyphs come
/// from its outlines instead.
/// </para>
/// </remarks>
public static class ColourBitmaps
{
    /// <summary>Whether the face carries colour bitmap strikes at all.</summary>
    public static bool Has(OpenTypeFace? face)
        => face is not null && face.File.Has("CBLC") && face.File.Has("CBDT");

    /// <summary>
    /// The colour bitmap for one glyph, or null when the face has none for it.
    /// </summary>
    /// <param name="face">The face to read.</param>
    /// <param name="glyphId">The glyph index within that face.</param>
    /// <param name="pixelsPerEm">
    /// The size the glyph will be drawn at, to choose between several strikes; zero or less asks
    /// for the largest strike, which is what a size-independent consumer such as a PDF Type 3 char
    /// proc wants.
    /// </param>
    public static ColourBitmap? Of(OpenTypeFace? face, ushort glyphId, int pixelsPerEm = 0)
    {
        if (!Has(face) || face is null) return null;

        ReadOnlySpan<byte> locations = face.File.Table("CBLC");
        ReadOnlySpan<byte> data = face.File.Table("CBDT");
        if (locations.Length < 8 || data.Length < 4) return null;

        long strikeCount = BinaryPrimitives.ReadUInt32BigEndian(locations[4..]);
        if (strikeCount <= 0 || 8 + (StrikeRecordSize * strikeCount) > locations.Length) return null;

        ColourBitmap? best = null;
        int bestDistance = int.MaxValue;
        int bestSize = -1;

        for (int i = 0; i < strikeCount; i++)
        {
            ReadOnlySpan<byte> record = locations[(8 + (StrikeRecordSize * i))..];

            int first = BinaryPrimitives.ReadUInt16BigEndian(record[40..]);
            int last = BinaryPrimitives.ReadUInt16BigEndian(record[42..]);
            if (glyphId < first || glyphId > last) continue;

            int perEmX = record[44];
            int perEmY = record[45];
            if (perEmX <= 0 || perEmY <= 0) continue;

            // Nearest strike to the size asked for, and the larger of two equally near ones, so a
            // glyph is never scaled up from a smaller strike when a bigger one is equally close.
            int distance = pixelsPerEm > 0 ? Math.Abs(perEmY - pixelsPerEm) : 0;
            if (distance > bestDistance || (distance == bestDistance && perEmY <= bestSize)) continue;

            if (InStrike(record, locations, data, glyphId, perEmX, perEmY) is not { } found) continue;

            best = found;
            bestDistance = distance;
            bestSize = perEmY;
        }

        return best;
    }

    /// <summary>A <c>BitmapSize</c> record: two offsets, two counts, two line metrics and the ppem.</summary>
    private const int StrikeRecordSize = 48;

    /// <summary>An <c>IndexSubTableArray</c> entry: a glyph range and the offset of its subtable.</summary>
    private const int IndexRecordSize = 8;

    /// <summary>The glyph's bitmap within one strike, or null when the strike does not hold it.</summary>
    private static ColourBitmap? InStrike(
        ReadOnlySpan<byte> strike,
        ReadOnlySpan<byte> locations,
        ReadOnlySpan<byte> data,
        ushort glyphId,
        int perEmX,
        int perEmY)
    {
        long arrayAt = BinaryPrimitives.ReadUInt32BigEndian(strike);
        long subtables = BinaryPrimitives.ReadUInt32BigEndian(strike[8..]);
        if (arrayAt < 0 || subtables <= 0) return null;
        if (arrayAt + (IndexRecordSize * subtables) > locations.Length) return null;

        for (int i = 0; i < subtables; i++)
        {
            ReadOnlySpan<byte> entry = locations[(int)(arrayAt + (IndexRecordSize * i))..];
            int first = BinaryPrimitives.ReadUInt16BigEndian(entry);
            int last = BinaryPrimitives.ReadUInt16BigEndian(entry[2..]);
            if (glyphId < first || glyphId > last) continue;

            long at = arrayAt + BinaryPrimitives.ReadUInt32BigEndian(entry[4..]);
            if (at < 0 || at + 8 > locations.Length) continue;

            ReadOnlySpan<byte> subtable = locations[(int)at..];
            int indexFormat = BinaryPrimitives.ReadUInt16BigEndian(subtable);
            int imageFormat = BinaryPrimitives.ReadUInt16BigEndian(subtable[2..]);
            long imagesAt = BinaryPrimitives.ReadUInt32BigEndian(subtable[4..]);

            if (Locate(subtable, indexFormat, glyphId, first, last) is not { } located) continue;

            ReadOnlySpan<byte> image = Span(data, imagesAt + located.Offset, located.Length);
            if (image.IsEmpty) continue;

            return Decode(image, imageFormat, located.Metrics, perEmX, perEmY);
        }

        return null;
    }

    /// <summary>
    /// Where a glyph's record sits inside the image data, and the metrics the index states for it.
    /// </summary>
    /// <remarks>
    /// Formats 2 and 5 store <em>constant</em> metrics for every glyph of the range and leave the
    /// image data holding nothing but the payload; formats 1, 3 and 4 store an offset per glyph and
    /// the metrics travel with the image. Both shapes come back through the same tuple so that the
    /// decoder below has one case for image format 19, which is the format that says "the metrics
    /// were in the index".
    /// </remarks>
    private static (long Offset, long Length, BitmapMetrics? Metrics)? Locate(
        ReadOnlySpan<byte> subtable, int indexFormat, ushort glyphId, int first, int last)
    {
        int index = glyphId - first;
        int count = last - first + 1;

        switch (indexFormat)
        {
            case 1:
            {
                if (8 + (4 * (count + 1)) > subtable.Length) return null;
                long start = BinaryPrimitives.ReadUInt32BigEndian(subtable[(8 + (4 * index))..]);
                long end = BinaryPrimitives.ReadUInt32BigEndian(subtable[(8 + (4 * (index + 1)))..]);
                return end > start ? (start, end - start, null) : null;
            }

            case 2:
            {
                if (subtable.Length < 20) return null;
                long size = BinaryPrimitives.ReadUInt32BigEndian(subtable[8..]);
                if (size <= 0) return null;
                return (size * index, size, BitmapMetrics.Big(subtable[12..]));
            }

            case 3:
            {
                if (8 + (2 * (count + 1)) > subtable.Length) return null;
                long start = BinaryPrimitives.ReadUInt16BigEndian(subtable[(8 + (2 * index))..]);
                long end = BinaryPrimitives.ReadUInt16BigEndian(subtable[(8 + (2 * (index + 1)))..]);
                return end > start ? (start, end - start, null) : null;
            }

            case 4:
            {
                if (subtable.Length < 12) return null;
                long pairs = BinaryPrimitives.ReadUInt32BigEndian(subtable[8..]);
                if (pairs <= 0 || 12 + (4 * (pairs + 1)) > subtable.Length) return null;

                for (int i = 0; i < pairs; i++)
                {
                    ReadOnlySpan<byte> pair = subtable[(12 + (4 * i))..];
                    if (BinaryPrimitives.ReadUInt16BigEndian(pair) != glyphId) continue;

                    long start = BinaryPrimitives.ReadUInt16BigEndian(pair[2..]);
                    long end = BinaryPrimitives.ReadUInt16BigEndian(subtable[(12 + (4 * (i + 1)) + 2)..]);
                    return end > start ? (start, end - start, null) : null;
                }

                return null;
            }

            case 5:
            {
                if (subtable.Length < 24) return null;
                long size = BinaryPrimitives.ReadUInt32BigEndian(subtable[8..]);
                long glyphs = BinaryPrimitives.ReadUInt32BigEndian(subtable[20..]);
                if (size <= 0 || glyphs <= 0 || 24 + (2 * glyphs) > subtable.Length) return null;

                for (int i = 0; i < glyphs; i++)
                {
                    if (BinaryPrimitives.ReadUInt16BigEndian(subtable[(24 + (2 * i))..]) != glyphId) continue;
                    return (size * i, size, BitmapMetrics.Big(subtable[12..]));
                }

                return null;
            }

            default:
                return null;
        }
    }

    /// <summary>Turns one <c>CBDT</c> record into a bitmap, for the three formats that hold a PNG.</summary>
    /// <remarks>
    /// 17 carries small metrics and 18 big ones, both followed by a length and the file; 19 carries
    /// the length and the file alone and takes its metrics from the index subtable, so it is only
    /// readable under index format 2 or 5.
    /// </remarks>
    private static ColourBitmap? Decode(
        ReadOnlySpan<byte> record, int imageFormat, BitmapMetrics? inherited, int perEmX, int perEmY)
    {
        BitmapMetrics metrics;
        int at;

        switch (imageFormat)
        {
            case 17 when record.Length >= 9:
                metrics = BitmapMetrics.Small(record);
                at = 5;
                break;

            case 18 when record.Length >= 12:
                metrics = BitmapMetrics.Big(record);
                at = 8;
                break;

            case 19 when record.Length >= 4 && inherited is { } stated:
                metrics = stated;
                at = 0;
                break;

            default:
                return null;
        }

        long length = BinaryPrimitives.ReadUInt32BigEndian(record[at..]);
        int payload = at + 4;
        if (length <= 0 || payload + length > record.Length) return null;
        if (metrics.Width <= 0 || metrics.Height <= 0) return null;

        return new ColourBitmap
        {
            Image = record.Slice(payload, (int)length).ToArray(),
            MediaType = "image/png",
            PixelsPerEmX = perEmX,
            PixelsPerEmY = perEmY,
            PixelWidth = metrics.Width,
            PixelHeight = metrics.Height,
            BearingX = metrics.BearingX,
            BearingY = metrics.BearingY,
        };
    }

    /// <summary>A slice that stays inside the table, or empty when the extent is not there.</summary>
    private static ReadOnlySpan<byte> Span(ReadOnlySpan<byte> table, long offset, long length)
        => offset < 0 || length <= 0 || offset + length > table.Length
            ? default
            : table.Slice((int)offset, (int)length);

    /// <summary>A glyph's size and bearings in pixels, from either metrics record.</summary>
    private readonly record struct BitmapMetrics(int Height, int Width, int BearingX, int BearingY)
    {
        /// <summary><c>SmallGlyphMetrics</c>: height, width, bearings and one advance.</summary>
        public static BitmapMetrics Small(ReadOnlySpan<byte> record)
            => new(record[0], record[1], (sbyte)record[2], (sbyte)record[3]);

        /// <summary><c>BigGlyphMetrics</c>: the same, with vertical bearings and advance after it.</summary>
        public static BitmapMetrics Big(ReadOnlySpan<byte> record)
            => new(record[0], record[1], (sbyte)record[2], (sbyte)record[3]);
    }
}

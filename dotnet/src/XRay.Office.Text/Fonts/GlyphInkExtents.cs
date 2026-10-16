using System.Buffers.Binary;

namespace XRay.Office.Text.Fonts;

/// <summary>
/// How far a glyph's ink reaches above and below the baseline, in design units.
/// </summary>
/// <remarks>
/// <para>
/// <strong>This is a metric, not an outline, and the distinction is the reason it is not part of
/// <see cref="GlyphOutlines"/>.</strong> Every <c>glyf</c> record opens with a ten-byte header
/// carrying the glyph's own bounding box, so an extent costs two <c>loca</c> reads and four
/// big-endian shorts — no contour decoding, no composite recursion, and nothing that would tempt a
/// caller to draw text as paths. <see cref="GlyphOutlines"/> exists for Fontwork alone and says so
/// at its head; widening it to serve a measurement would blur that.
/// </para>
/// <para>
/// <strong>What needs it is a clip.</strong> LibreOffice decides whether a shape's text survives a
/// <c>vertOverflow="clip"</c> body by the ink of each drawn portion rather than by the line box it
/// sits in: <c>TextHierarchyBreakupBlockText::processDrawPortionInfo</c> keeps a portion only when
/// its start position and both corners of <c>TextLayouterDevice::getTextBoundRect</c> fall inside
/// the clip range (<c>svx/source/svdraw/svdoutl.cxx</c>:120-160). So a descender decides it: at 16 pt
/// in a 33 pt button, DejaVu Sans draws <c>Icon sets</c> and drops <c>Inventory list</c>, and the
/// only difference between them is the <c>y</c>.
/// </para>
/// <para>
/// A composite glyph's own header records the assembled box, so accented Latin answers correctly
/// without following its components. A face carrying no <c>glyf</c> — every CFF face — answers null,
/// and the caller falls back to the font's declared descent, which over-clips rather than
/// under-clips and is what the line box gave before.
/// </para>
/// </remarks>
public static class GlyphInkExtents
{
    /// <summary>Whether this face records glyph bounding boxes this reader can produce.</summary>
    public static bool CanMeasure(OpenTypeFace? face)
        => face is not null && face.File.Has("glyf") && face.File.Has("loca");

    /// <summary>
    /// The ink of one glyph, in the face's design units, or null when it cannot be read.
    /// </summary>
    /// <param name="face">The face to read.</param>
    /// <param name="glyphId">The glyph index within that face.</param>
    /// <returns>
    /// How far the ink reaches above the baseline and how far below it, both non-negative and both
    /// zero for a glyph with no contours at all — a space is ink-free rather than unmeasurable.
    /// Null when the face carries no <c>glyf</c> outlines or the record is malformed.
    /// </returns>
    public static (int Above, int Below)? Of(OpenTypeFace? face, ushort glyphId)
    {
        if (!CanMeasure(face) || face is null) return null;

        if (!Locate(face, glyphId, out int offset, out int length))
        {
            // An empty entry in `loca` is a glyph with no contours, which every face uses for the
            // space. That is a real answer of "no ink" rather than a failure to read one.
            return EmptyEntry(face, glyphId) ? (0, 0) : null;
        }

        ReadOnlySpan<byte> glyf = face.File.Table("glyf");
        if (length < 10 || offset + length > glyf.Length) return null;

        ReadOnlySpan<byte> glyph = glyf.Slice(offset, length);

        // Bytes 2-9 of the record are xMin, yMin, xMax, yMax, signed and in design units. The
        // vertical pair is all this needs, and it is stated for composites as well as simple
        // glyphs.
        int yMin = BinaryPrimitives.ReadInt16BigEndian(glyph[4..]);
        int yMax = BinaryPrimitives.ReadInt16BigEndian(glyph[8..]);

        return (Math.Max(0, yMax), Math.Max(0, -yMin));
    }

    /// <summary>Whether <c>loca</c> gives this glyph a zero-length record.</summary>
    private static bool EmptyEntry(OpenTypeFace face, ushort glyphId)
    {
        ReadOnlySpan<byte> loca = face.File.Table("loca");
        bool isLong = face.Head.IndexToLocFormat != 0;
        int entry = isLong ? 4 : 2;

        int start = glyphId * entry;
        if (start + (2 * entry) > loca.Length) return false;

        long from = isLong
            ? BinaryPrimitives.ReadUInt32BigEndian(loca[start..])
            : BinaryPrimitives.ReadUInt16BigEndian(loca[start..]) * 2L;
        long to = isLong
            ? BinaryPrimitives.ReadUInt32BigEndian(loca[(start + entry)..])
            : BinaryPrimitives.ReadUInt16BigEndian(loca[(start + entry)..]) * 2L;

        return to == from;
    }

    /// <summary>Where a glyph's record sits in <c>glyf</c>, from <c>loca</c>.</summary>
    private static bool Locate(OpenTypeFace face, ushort glyphId, out int offset, out int length)
    {
        offset = 0;
        length = 0;

        ReadOnlySpan<byte> loca = face.File.Table("loca");
        bool isLong = face.Head.IndexToLocFormat != 0;
        int entry = isLong ? 4 : 2;

        int start = glyphId * entry;
        if (start + (2 * entry) > loca.Length) return false;

        long from = isLong
            ? BinaryPrimitives.ReadUInt32BigEndian(loca[start..])
            : BinaryPrimitives.ReadUInt16BigEndian(loca[start..]) * 2L;
        long to = isLong
            ? BinaryPrimitives.ReadUInt32BigEndian(loca[(start + entry)..])
            : BinaryPrimitives.ReadUInt16BigEndian(loca[(start + entry)..]) * 2L;

        if (to <= from || from < 0 || to > int.MaxValue) return false;

        offset = (int)from;
        length = (int)(to - from);
        return true;
    }
}

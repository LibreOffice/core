namespace XRay.Office.Presentations.MsBinary;

/// <summary>
/// How many hyperlinks a deck's <c>_PID_HLINKS</c> blob yields <em>as the reference reads it</em>,
/// which is not always how many it declares.
/// </summary>
/// <remarks>
/// <para>
/// <c>ImplSdPPTImport::Import</c> builds one <c>SdHyperlinkEntry</c> per link of the
/// <c>_PID_HLINKS</c> blob in the user-defined property section — six OLE properties each, and
/// the loop <c>break</c>s at the first one whose type is not <c>VT_I4</c> or whose string will
/// not read (<c>sd/source/filter/ppt/pptin.cxx</c>:353-518). Only the entries it built are then
/// given an index from the <c>ExObjList</c>'s <c>ExHyperlink</c> records, in stream order
/// (<c>:530-549</c>), so <strong>the number of entries the blob yields caps how many of a deck's
/// hyperlink identifiers can ever match a text range</strong>.
/// </para>
/// <para>
/// <strong>And the blob is handed to that loop truncated, by an arithmetic defect in the property
/// reader.</strong> <c>Section::Read</c> sizes each property's buffer and then clamps it:
/// </para>
/// <code>
/// if( nPropSize &gt; nSecSize - nSecOfs )
///     nPropSize = nSecSize - nSecOfs;
/// </code>
/// <para>
/// (<c>sd/source/filter/ppt/propread.cxx</c>:443-447.) <c>nSecSize</c> is the section's own
/// declared <em>length</em> and <c>nSecOfs</c> is its <em>absolute offset</em> in the stream —
/// a length less a position, which is not a quantity. A user-defined section that begins late in
/// a <c>\005DocumentSummaryInformation</c> stream therefore has every large property cut short,
/// and <c>_PID_HLINKS</c> is the largest property such a stream carries.
/// </para>
/// <para>
/// <strong>Measured at 26.2.4.2 rather than inferred from the source, which declares a different
/// version.</strong> On <c>RESPA_-_Section_8_Webinar.ppt</c> the section is at offset 1540 and
/// declares 2244, so a 2180-byte blob is handed over as <strong>704</strong> bytes; the second
/// entry ends at byte 592 and the third would end at 820, so exactly <strong>two</strong> entries
/// are built where the blob declares ten. Patching one text range's <c>exHyperlinkId</c> to each
/// of the file's ten <c>ExHyperlinkAtom</c> values in turn and rendering all ten through
/// 26.2.4.2, <strong>the first two draw the range underlined in the scheme's hyperlink colour and
/// the other eight draw it in the body's own colour with no rule under it</strong>
/// (<c>probes/slides-r103/hyperid.py</c>). Two, from a rule with no free parameter.
/// </para>
/// <para>
/// Reading it matters well beyond the colour of a URL: a hyperlink is an EditEngine
/// <em>field</em>, a field's spill lines are not counted in the height the shrink-to-fit search
/// measures, and a block one line short of the reference's fits a larger row of
/// <c>constScaleLevels</c> — so a single unresolvable identifier moves every character on the
/// slide by a point of drawn size. See <c>probes/slides-r103/results.md</c>.
/// </para>
/// </remarks>
internal static class PptHyperlinkBlob
{
    /// <summary>The user-defined property section's format identifier, as bytes.</summary>
    /// <remarks>
    /// <c>{D5CDD505-2E9C-101B-9397-08002B2CF9AE}</c>, the section <c>_PID_HLINKS</c> lives in
    /// (<c>pptin.cxx</c>:353-358, which spells the same sixteen bytes out).
    /// </remarks>
    private static ReadOnlySpan<byte> UserDefinedSection =>
    [
        0x05, 0xD5, 0xCD, 0xD5, 0x9C, 0x2E, 0x1B, 0x10,
        0x93, 0x97, 0x08, 0x00, 0x2B, 0x2C, 0xF9, 0xAE,
    ];

    private const uint VtI4 = 3;
    private const uint VtLpstr = 30;
    private const uint VtLpwstr = 31;
    private const uint VtBlob = 65;

    /// <summary>
    /// The number of hyperlink entries the deck's <c>_PID_HLINKS</c> yields, or null when the
    /// document declares no such blob and the reference falls back to one entry per
    /// <c>ExHyperlink</c> record (<c>pptin.cxx</c>:551-575).
    /// </summary>
    /// <param name="summary">
    /// The whole <c>\005DocumentSummaryInformation</c> stream, or an empty span when the compound
    /// file has none.
    /// </param>
    public static int? Count(ReadOnlySpan<byte> summary)
    {
        if (!Section(summary, out int sectionOffset, out int sectionSize)) return null;
        if (!Blob(summary, sectionOffset, out int blobOffset)) return null;

        // Section::Read, propread.cxx:443-447. The clamp is the defect this class exists for; a
        // section whose offset is past its declared size wraps in unsigned arithmetic and clamps
        // nothing, which is the ordinary case and is why most documents are unaffected.
        long clamp = (long)(uint)sectionSize - (long)(uint)sectionOffset;
        int available = summary.Length - blobOffset;
        if (clamp >= 0 && clamp < available) available = (int)clamp;

        return Entries(summary.Slice(blobOffset, Math.Max(available, 0)));
    }

    /// <summary>Finds the user-defined section, and its declared size.</summary>
    private static bool Section(ReadOnlySpan<byte> summary, out int offset, out int size)
    {
        offset = 0;
        size = 0;
        if (summary.Length < 28) return false;

        uint sections = ReadUInt32(summary, 24);
        if (sections > 64) return false;

        for (int i = 0; i < sections; i++)
        {
            int entry = 28 + (i * 20);
            if (entry + 20 > summary.Length) return false;
            if (!summary.Slice(entry, 16).SequenceEqual(UserDefinedSection)) continue;

            offset = (int)ReadUInt32(summary, entry + 16);
            if (offset < 0 || offset + 8 > summary.Length) return false;

            size = (int)ReadUInt32(summary, offset);
            return true;
        }

        return false;
    }

    /// <summary>Finds <c>_PID_HLINKS</c> through the section's own dictionary, property zero.</summary>
    private static bool Blob(ReadOnlySpan<byte> summary, int section, out int offset)
    {
        offset = 0;
        uint count = ReadUInt32(summary, section + 4);
        if (count > 4096) return false;

        int dictionary = 0;
        int candidate = 0;
        Span<int> ids = count <= 256 ? stackalloc int[(int)count] : new int[count];
        Span<int> offsets = count <= 256 ? stackalloc int[(int)count] : new int[count];

        for (int i = 0; i < count; i++)
        {
            int entry = section + 8 + (i * 8);
            if (entry + 8 > summary.Length) return false;

            ids[i] = (int)ReadUInt32(summary, entry);
            offsets[i] = section + (int)ReadUInt32(summary, entry + 4);
            if (ids[i] == 0) dictionary = offsets[i];
        }

        if (dictionary == 0 || dictionary + 4 > summary.Length) return false;

        uint names = ReadUInt32(summary, dictionary);
        if (names > 4096) return false;

        int position = dictionary + 4;
        int wanted = 0;
        for (int i = 0; i < names && position + 8 <= summary.Length; i++)
        {
            int id = (int)ReadUInt32(summary, position);
            int length = (int)ReadUInt32(summary, position + 4);
            if (length < 0 || position + 8 + length > summary.Length) return false;

            ReadOnlySpan<byte> name = summary.Slice(position + 8, length);
            int end = name.IndexOf((byte)0);
            if (end >= 0) name = name[..end];
            if (name.SequenceEqual("_PID_HLINKS"u8)) wanted = id;

            position += 8 + length;
        }

        if (wanted == 0) return false;

        for (int i = 0; i < count; i++)
        {
            if (ids[i] != wanted) continue;
            candidate = offsets[i];
            break;
        }

        if (candidate <= 0 || candidate + 12 > summary.Length) return false;
        if (ReadUInt32(summary, candidate) != VtBlob) return false;

        offset = candidate;
        return true;
    }

    /// <summary>
    /// Walks the blob exactly as <c>pptin.cxx</c>:392-518 does, and answers how many entries it
    /// built before it stopped.
    /// </summary>
    private static int Entries(ReadOnlySpan<byte> blob)
    {
        if (blob.Length < 12) return 0;

        uint properties = ReadUInt32(blob, 8);
        if (properties % 6 != 0) return 0;

        int declared = (int)(properties / 6);
        int position = 12;
        int built = 0;

        for (int i = 0; i < declared; i++)
        {
            bool typed = true;
            for (int field = 0; field < 4; field++)
            {
                if (position + 8 > blob.Length || ReadUInt32(blob, position) != VtI4)
                {
                    typed = false;
                    break;
                }

                position += 8;
            }

            if (!typed) break;
            if (!ReadString(blob, ref position)) break;   // the target
            if (!ReadString(blob, ref position)) break;   // the sub-address

            built++;
        }

        return built;
    }

    /// <summary>
    /// <c>PropItem::Read</c> (<c>propread.cxx</c>:73-186): a type, a count, the characters, and a
    /// dword alignment — and it answers <em>false</em>, which stops the caller, whenever the last
    /// character it could read is not a terminator. That is the failure this whole class is about:
    /// a truncated buffer does not throw and does not read a short string, it ends the list.
    /// </summary>
    private static bool ReadString(ReadOnlySpan<byte> blob, ref int position)
    {
        if (position + 8 > blob.Length) return false;

        uint type = ReadUInt32(blob, position);
        int count = (int)ReadUInt32(blob, position + 4);
        if (count < 0) return false;

        int start = position + 8;
        if (type == VtLpwstr)
        {
            // The count is in characters; the terminator has to be inside the buffer.
            if (count == 0 || start + (count * 2) > blob.Length) return false;
            if (ReadUInt16(blob, start + ((count - 1) * 2)) != 0) return false;

            position = start + (count * 2) + ((count & 1) != 0 ? 2 : 0);
            return true;
        }

        if (type != VtLpstr) return false;
        if (count == 0 || start + count > blob.Length) return false;
        if (blob[start + count - 1] != 0) return false;

        position = start + count + ((4 - (count & 3)) & 3);
        return true;
    }

    private static uint ReadUInt32(ReadOnlySpan<byte> data, int offset)
        => (uint)(data[offset] | (data[offset + 1] << 8) | (data[offset + 2] << 16)
                  | (data[offset + 3] << 24));

    private static ushort ReadUInt16(ReadOnlySpan<byte> data, int offset)
        => (ushort)(data[offset] | (data[offset + 1] << 8));
}

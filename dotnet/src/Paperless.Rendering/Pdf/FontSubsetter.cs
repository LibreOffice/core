using System.Buffers;
using System.Runtime.InteropServices;

namespace Paperless.Rendering.Pdf;

/// <summary>
/// Cuts a font down to the glyphs a document actually uses, with <c>hb-subset</c>.
/// </summary>
/// <remarks>
/// <para>
/// The same tool LibreOffice subsets with, and the one part of PDF font embedding that has
/// a ready answer. Hand-rolling it would mean rewriting <c>glyf</c>, <c>loca</c> and
/// <c>hmtx</c> — feasible — and then <c>CFF</c>, which is a second outline format with its
/// own charset, encoding and local subroutine indexes. Every <c>.otf</c> on the machine is
/// CFF, so a hand-rolled subsetter would embed a broken font for a whole class of faces.
/// </para>
/// <para>
/// HarfBuzzSharp's managed binding does not expose <c>hb_subset_*</c> — verified against
/// 14.2.1.1, whose only matching export is the unrelated <c>hb_set_is_subset</c> — but the
/// native library it ships <em>does</em>, all thirty-two entry points. So the binding here
/// is direct rather than through the wrapper, and it deliberately uses only the blob and
/// face calls it needs so that it does not have to agree with the wrapper about object
/// lifetimes.
/// </para>
/// <para>
/// The glyph renumbering is stated rather than inferred. hb-subset renumbers retained
/// glyphs compactly, and the order it would choose is an implementation detail, but the
/// PDF has already committed to a code for each glyph by the time the font is built — the
/// content stream was written first. <c>hb_subset_input_old_to_new_glyph_mapping</c> takes
/// the mapping as an input, so the font is built to the codes rather than the codes read
/// off the font.
/// </para>
/// </remarks>
internal static partial class FontSubsetter
{
    private const string Library = "libHarfBuzzSharp";

    /// <summary><c>HB_MEMORY_MODE_READONLY</c>.</summary>
    private const int MemoryModeReadOnly = 0;

    /// <summary><c>HB_SUBSET_SETS_DROP_TABLE_TAG</c>.</summary>
    private const int DropTableTagSet = 3;

    private static readonly Lazy<bool> Availability = new(Probe);

    /// <summary>True when the native harfbuzz library is present and exposes the subsetter.</summary>
    /// <remarks>
    /// Probed once. Paperless has to keep producing a PDF where the native library is
    /// missing: a PDF that names a font it did not embed still holds the right text at the
    /// right pen positions, where a <see cref="DllNotFoundException"/> is no PDF at all.
    /// </remarks>
    public static bool IsAvailable => Availability.Value;

    /// <summary>
    /// Produces a font holding only the given glyphs, renumbered to their PDF codes.
    /// </summary>
    /// <param name="font">The whole font file.</param>
    /// <param name="faceIndex">Which face of a collection to subset.</param>
    /// <param name="glyphsByCode">
    /// The original glyph id for each PDF character code, index 0 being <c>.notdef</c>.
    /// The subset holds exactly these, numbered by their index here.
    /// </param>
    /// <returns>The subsetted font, or null when subsetting is unavailable or fails.</returns>
    public static byte[]? Subset(ReadOnlyMemory<byte> font, int faceIndex, IReadOnlyList<ushort> glyphsByCode)
    {
        ArgumentNullException.ThrowIfNull(glyphsByCode);
        if (font.IsEmpty || glyphsByCode.Count == 0 || !IsAvailable) return null;

        nint blob = 0, face = 0, input = 0, subset = 0, produced = 0;
        MemoryHandle pinned = font.Pin();

        try
        {
            unsafe
            {
                blob = hb_blob_create((nint)pinned.Pointer, (uint)font.Length, MemoryModeReadOnly, 0, 0);
            }

            if (blob == 0) return null;

            face = hb_face_create(blob, (uint)Math.Max(faceIndex, 0));
            if (face == 0) return null;

            input = hb_subset_input_create_or_fail();
            if (input == 0) return null;

            nint glyphs = hb_subset_input_glyph_set(input);
            nint mapping = hb_subset_input_old_to_new_glyph_mapping(input);
            if (glyphs == 0 || mapping == 0) return null;

            for (int code = 0; code < glyphsByCode.Count; code++)
            {
                hb_set_add(glyphs, glyphsByCode[code]);
                hb_map_set(mapping, glyphsByCode[code], (uint)code);
            }

            // The layout tables describe substitutions and positioning that shaping has already
            // applied; carrying them into the PDF would embed rules nothing will run, and on a
            // large face they are most of the file. Measured on Carlito with five glyphs
            // retained: 4964 bytes with them and 2664 without.
            nint drop = hb_subset_input_set(input, DropTableTagSet);
            if (drop != 0)
            {
                foreach (string table in (string[])["GSUB", "GPOS", "GDEF"]) hb_set_add(drop, SfntTables.Tag(table));
            }

            subset = hb_subset_or_fail(face, input);
            if (subset == 0) return null;

            produced = hb_face_reference_blob(subset);
            if (produced == 0) return null;

            uint length = 0;
            nint data = hb_blob_get_data(produced, ref length);
            if (data == 0 || length == 0) return null;

            byte[] bytes = new byte[length];
            Marshal.Copy(data, bytes, 0, (int)length);
            return WithIdentityCharacterMap(bytes, glyphsByCode.Count - 1);
        }
        catch (DllNotFoundException)
        {
            return null;
        }
        catch (EntryPointNotFoundException)
        {
            return null;
        }
        finally
        {
            if (produced != 0) hb_blob_destroy(produced);
            if (subset != 0) hb_face_destroy(subset);
            if (input != 0) hb_subset_input_destroy(input);
            if (face != 0) hb_face_destroy(face);
            if (blob != 0) hb_blob_destroy(blob);
            pinned.Dispose();
        }
    }

    /// <summary>
    /// Swaps the subset's character map for one whose codes are its glyph indices.
    /// </summary>
    /// <remarks>
    /// hb-subset writes a Unicode cmap, which is right for a font used as a font and wrong
    /// for one addressed by a PDF's one-byte codes. Returning the font unchanged when it
    /// cannot be rewritten is the safer failure: a Unicode cmap makes a viewer that trusts
    /// it draw the wrong glyphs, but the alternative — no font at all — loses the face.
    /// </remarks>
    private static byte[] WithIdentityCharacterMap(byte[] font, int highestCode)
    {
        if (SfntTables.Read(font) is not { } tables) return font;

        tables.Replace("cmap", SfntTables.IdentityCharacterMap(highestCode));
        return tables.Serialise();
    }

    private static bool Probe()
    {
        try
        {
            nint input = hb_subset_input_create_or_fail();
            if (input == 0) return false;

            hb_subset_input_destroy(input);
            return true;
        }
        catch (DllNotFoundException)
        {
            return false;
        }
        catch (EntryPointNotFoundException)
        {
            return false;
        }
    }

    [LibraryImport(Library)]
    private static partial nint hb_blob_create(nint data, uint length, int mode, nint userData, nint destroy);

    [LibraryImport(Library)]
    private static partial void hb_blob_destroy(nint blob);

    [LibraryImport(Library)]
    private static partial nint hb_blob_get_data(nint blob, ref uint length);

    [LibraryImport(Library)]
    private static partial nint hb_face_create(nint blob, uint index);

    [LibraryImport(Library)]
    private static partial void hb_face_destroy(nint face);

    [LibraryImport(Library)]
    private static partial nint hb_face_reference_blob(nint face);

    [LibraryImport(Library)]
    private static partial void hb_set_add(nint set, uint value);

    [LibraryImport(Library)]
    private static partial void hb_map_set(nint map, uint key, uint value);

    [LibraryImport(Library)]
    private static partial nint hb_subset_input_create_or_fail();

    [LibraryImport(Library)]
    private static partial void hb_subset_input_destroy(nint input);

    [LibraryImport(Library)]
    private static partial nint hb_subset_input_glyph_set(nint input);

    [LibraryImport(Library)]
    private static partial nint hb_subset_input_old_to_new_glyph_mapping(nint input);

    [LibraryImport(Library)]
    private static partial nint hb_subset_input_set(nint input, int which);

    [LibraryImport(Library)]
    private static partial nint hb_subset_or_fail(nint face, nint input);
}

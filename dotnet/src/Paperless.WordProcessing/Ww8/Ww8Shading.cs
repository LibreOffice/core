using System.Buffers.Binary;
using Paperless.Core.Graphics;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// A DOC's cell shading: two colours and a pattern index, flattened to the one colour it looks like.
/// </summary>
/// <remarks>
/// <para>
/// Word states a shade as a <em>pattern</em> — a foreground colour, a background colour, and an index
/// naming how much of the cell the foreground covers — rather than as the resulting colour. Twenty per cent
/// black on white is not grey in the file; it is black over white at index 4. So the read has to blend, and
/// the blend has to use Word's own percentages, which are neither linear in the index nor evenly spaced:
/// LibreOffice keeps them in <c>SwWW8Shade::SetShade</c>'s <c>eMSGrayScale</c> table
/// (<c>sw/source/filter/ww8/ww8par6.cxx</c>), and this is that table.
/// </para>
/// <para>
/// Two spellings again, and LibreOffice's own export writes both: <c>sprmTDefTableShd80</c> packs a
/// palette-indexed pattern into sixteen bits per cell, and <c>sprmTDefTableShd</c> gives ten bytes per cell
/// with full colours. The newer one wins per cell where it says anything, which is what
/// <c>WW8TabDesc::SetTabShades</c> does.
/// </para>
/// </remarks>
internal static class Ww8Shading
{
    /// <summary>
    /// How much of the cell each pattern index covers, in thousandths.
    /// </summary>
    /// <remarks>
    /// Index 0 is the clear brush — no shading at all, whatever the two colours say — and index 1 is solid.
    /// From there the percentages are Word's own set, and the run from 26 to 34 is undocumented in the
    /// specification and treated as half coverage. The hatches, 14 to 25, are all a third: LibreOffice
    /// cannot draw a hatch behind a table cell and blends them instead.
    /// </remarks>
    private static readonly int[] Coverage =
    [
        0, 1000,
        50, 100, 200, 250, 300, 400, 500, 600, 700, 750, 800, 900,
        333, 333, 333, 333, 333, 333, 333, 333, 333, 333, 333, 333,
        500, 500, 500, 500, 500, 500, 500, 500, 500,
        25, 75, 125, 150, 175, 225, 275, 325, 350, 375, 425, 450, 475, 525, 550, 575,
        625, 650, 675, 725, 775, 825, 850, 875, 925, 950, 975,
    ];

    /// <summary>How many bytes one cell of the newer shading operand takes.</summary>
    public const int LongEntryLength = 10;

    /// <summary>
    /// Reads the newer per-cell shading operand: ten bytes each, two colours and an index.
    /// </summary>
    /// <param name="operand">The sprm's operand.</param>
    /// <param name="firstCell">
    /// Which cell the operand starts at. Word splits a wide row's shading across three sprms and the second
    /// and third start at cells 22 and 44 — a reader taking every one of them from zero shades the first
    /// columns three times and the rest never.
    /// </param>
    /// <returns>One entry per cell from the first, null where the cell has no shading.</returns>
    public static List<Colour?> ReadLong(ReadOnlyMemory<byte> operand, int firstCell)
    {
        ReadOnlySpan<byte> bytes = operand.Span;
        List<Colour?> shades = [];

        for (int i = 0; i < firstCell; i++) shades.Add(null);

        for (int at = 0; at + LongEntryLength <= bytes.Length; at += LongEntryLength)
        {
            Colour? fore = Ww8Colours.FromColorRef(BinaryPrimitives.ReadUInt32LittleEndian(bytes[at..]));
            Colour? back = Ww8Colours.FromColorRef(
                BinaryPrimitives.ReadUInt32LittleEndian(bytes[(at + 4)..]));
            int index = BinaryPrimitives.ReadUInt16LittleEndian(bytes[(at + 8)..]);

            shades.Add(Blend(fore, back, index));
        }

        return shades;
    }

    /// <summary>How many bytes one cell of the older shading operand takes.</summary>
    public const int ShortEntryLength = 2;

    /// <summary>
    /// Reads the older per-cell shading operand: a sixteen-bit <c>SHD</c> each.
    /// </summary>
    /// <param name="operand">The sprm's operand.</param>
    /// <remarks>
    /// Three bit fields in sixteen bits: the foreground's palette index in the low five, the background's
    /// in the next five, and the pattern in the top six. Word 6 and 7 used only five bits for the pattern,
    /// which is why LibreOffice masks it by version — the extra bit is what the later patterns need, and
    /// masking it off turns a ninety-five per cent shade into a thirty-one per cent one.
    /// </remarks>
    /// <returns>One entry per cell, null where the cell has no shading.</returns>
    public static List<Colour?> ReadShort(ReadOnlyMemory<byte> operand)
    {
        ReadOnlySpan<byte> bytes = operand.Span;
        List<Colour?> shades = [];

        for (int at = 0; at + ShortEntryLength <= bytes.Length; at += ShortEntryLength)
        {
            ushort packed = BinaryPrimitives.ReadUInt16LittleEndian(bytes[at..]);

            // An all-zero SHD is the automatic shade — no fill — rather than black on black at index zero.
            if (packed == 0)
            {
                shades.Add(null);
                continue;
            }

            shades.Add(Blend(
                Ww8Colours.At(packed & 0x1F),
                Ww8Colours.At((packed >> 5) & 0x1F),
                (packed >> 10) & 0x3F));
        }

        return shades;
    }

    /// <summary>
    /// The one colour a pattern looks like, or null when it is not a fill at all.
    /// </summary>
    /// <param name="fore">The pattern's foreground, or null for the automatic colour.</param>
    /// <param name="back">Its background, or null for the automatic colour.</param>
    /// <param name="index">The pattern index.</param>
    /// <remarks>
    /// The automatic colours are resolved the way <c>SetShade</c> resolves them, and the two directions are
    /// not symmetrical: an automatic foreground is black, because there is no such thing as an automatic
    /// pattern colour, while an automatic <em>background</em> counts as white for the blend but stays
    /// automatic when the pattern is clear. So a clear brush over an automatic background is no shading,
    /// and a clear brush over a stated one is that colour.
    /// </remarks>
    private static Colour? Blend(Colour? fore, Colour? back, int index)
    {
        int coverage = index >= 0 && index < Coverage.Length ? Coverage[index] : 0;

        if (coverage == 0) return back;

        Colour foreground = fore ?? Colour.Black;
        Colour background = back ?? Colour.White;

        return new Colour(
            Mix(foreground.R, background.R, coverage),
            Mix(foreground.G, background.G, coverage),
            Mix(foreground.B, background.B, coverage));
    }

    /// <summary>One channel of the blend, truncated as Word's integer arithmetic truncates it.</summary>
    private static byte Mix(byte fore, byte back, int coverage)
        => (byte)(((fore * coverage) + (back * (1000 - coverage))) / 1000);
}

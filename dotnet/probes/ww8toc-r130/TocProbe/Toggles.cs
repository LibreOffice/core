// How often a run states a toggle sprm relative to its style AND names a character style in the
// same CHPX — the population on which "which style is the toggle relative to" can be decided
// wrongly — with the same count for runs that name no character style as the base rate.
using XRay.Office.Core.Diagnostics;
using XRay.Office.WordProcessing.Ww8;

internal static class Toggles
{
    /// <summary>The character toggle sprms, whose operand 0x80/0x81 means "as/against the style".</summary>
    /// <remarks>
    /// <c>sprmCFBold</c> through <c>sprmCFVanish</c> and the five later ones, all
    /// <c>SPRA::operand_toggle_1b_0</c> in <c>sw/source/filter/ww8/sprmids.hxx</c>:315-345.
    /// </remarks>
    private static readonly ushort[] ToggleSprms =
    [
        0x0835, 0x0836, 0x0837, 0x0838, 0x0839, 0x083A, 0x083B, 0x083C,
        0x0854, 0x0858, 0x085A, 0x085C, 0x085D,
    ];

    internal static void Row(string path, (byte[] Word, byte[] Table, Ww8Fib Fib, Ww8StyleSheet Styles) doc)
    {
        List<Diagnostic> diags = [];
        Ww8PieceTable pieces = Ww8PieceTable.Parse(
            doc.Table.AsSpan((int)doc.Fib.FileOffset(Ww8FibTable.PieceTable),
                (int)doc.Fib.Length(Ww8FibTable.PieceTable)),
            doc.Word, System.Text.Encoding.Latin1, diags);
        Ww8FormattingTable chars = Ww8FormattingTable.Parse(
            doc.Table.AsSpan((int)doc.Fib.FileOffset(Ww8FibTable.CharacterFormattingIndex),
                (int)doc.Fib.Length(Ww8FibTable.CharacterFormattingIndex)), doc.Word, paragraphs: false);

        string text = pieces.ReadText(0, doc.Fib.TextLength, diags);
        int withStyle = 0, withoutStyle = 0, characters = 0;
        int lastFrom = -1;

        for (int i = 0; i < text.Length; i++)
        {
            (ReadOnlyMemory<byte> props, int from, int to) = chars.FindWithRange(pieces.FileOffsetOf(i));
            if (to <= from || from == lastFrom) continue;
            lastFrom = from;

            bool toggle = false;
            ushort istd = 0;
            foreach (Ww8Sprm sprm in Ww8SprmReader.Read(props))
            {
                if (sprm.Identifier == Ww8SprmReader.Ids.CharacterStyle) istd = sprm.Word;
                else if (Array.IndexOf(ToggleSprms, sprm.Identifier) >= 0 && sprm.Byte >= 0x80) toggle = true;
            }

            if (!toggle) continue;
            if (istd != 0)
            {
                withStyle++;
                characters += to - from;
            }
            else
            {
                withoutStyle++;
            }
        }

        Console.WriteLine($"{System.IO.Path.GetFileName(path)}\t{withStyle}\t{withoutStyle}\t{characters}");
    }
}

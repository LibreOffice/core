// Prints, for one stretch of a story's text, each run's CHPX and the paragraph style in force —
// the raw answer to "where does this word's boldness come from".
using XRay.Office.Core.Diagnostics;
using XRay.Office.WordProcessing.Ww8;

internal static class Runs
{
    internal static void Dump(byte[] wordDocument, byte[] table, Ww8Fib fib, Ww8StyleSheet styles, string needle)
    {
        List<Diagnostic> diags = [];
        Ww8PieceTable pieces = Ww8PieceTable.Parse(
            table.AsSpan((int)fib.FileOffset(Ww8FibTable.PieceTable), (int)fib.Length(Ww8FibTable.PieceTable)),
            wordDocument, System.Text.Encoding.Latin1, diags);
        Ww8FormattingTable chars = Ww8FormattingTable.Parse(
            table.AsSpan((int)fib.FileOffset(Ww8FibTable.CharacterFormattingIndex),
                (int)fib.Length(Ww8FibTable.CharacterFormattingIndex)), wordDocument, paragraphs: false);
        Ww8FormattingTable paras = Ww8FormattingTable.Parse(
            table.AsSpan((int)fib.FileOffset(Ww8FibTable.ParagraphFormattingIndex),
                (int)fib.Length(Ww8FibTable.ParagraphFormattingIndex)), wordDocument, paragraphs: true);

        string text = pieces.ReadText(0, fib.TextLength, diags);
        int at = text.IndexOf(needle, StringComparison.Ordinal);
        if (at < 0) { Console.WriteLine($"'{needle}' not found"); return; }

        Console.WriteLine($"'{needle}' at cp {at}");
        string last = "";
        for (int i = Math.Max(0, at - 60); i < Math.Min(text.Length, at + needle.Length + 8); i++)
        {
            (ReadOnlyMemory<byte> props, int from, int to) = chars.FindWithRange(pieces.FileOffsetOf(i));
            string hex = Convert.ToHexString(props.Span) + $" [{from}..{to})";
            if (hex != last)
            {
                Console.WriteLine($"  cp {i} {Show(text[i])} fc={pieces.FileOffsetOf(i)} chpx={hex}");
                last = hex;
            }
        }

        // The paragraph this stretch is in: the PAPX at the next paragraph mark.
        int mark = text.IndexOf('\r', at);
        if (mark < 0) return;
        (ushort istd, ReadOnlyMemory<byte> papx) =
            Ww8FormattingTable.SplitParagraphProperties(paras.Find(pieces.FileOffsetOf(mark)));
        Console.WriteLine($"  paragraph mark cp {mark}: istd {istd} ({styles.NameOf(istd) ?? "-"}) "
            + $"papx={Convert.ToHexString(papx.Span)}");
        Console.WriteLine($"  its style chpx={Convert.ToHexString((styles.At(istd)?.CharacterProperties ?? default).Span)}"
            + $" base={styles.At(istd)?.BaseIndex}");
    }

    private static string Show(char c) => c < ' ' ? $"U+{(int)c:X4}" : $"'{c}'";
}

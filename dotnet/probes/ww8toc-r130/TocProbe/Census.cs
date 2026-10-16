// One line per document: how many TOC/INDEX fields it holds, how many drawn characters sit in
// their cached results, and how many of those name the built-in Hyperlink character style — which
// is the population the round's change can reach, and its own base rate.
using XRay.Office.Core.Diagnostics;
using XRay.Office.WordProcessing.Ww8;

internal static class Census
{
    internal static void Row(string path, (byte[] Word, byte[] Table, Ww8Fib Fib, Ww8StyleSheet Styles) doc)
    {
        byte[] wordDocument = doc.Word;
        byte[] table = doc.Table;
        Ww8Fib fib = doc.Fib;
        Ww8StyleSheet styles = doc.Styles;
        List<Diagnostic> diags = [];
        Ww8PieceTable pieces = Ww8PieceTable.Parse(
            table.AsSpan((int)fib.FileOffset(Ww8FibTable.PieceTable), (int)fib.Length(Ww8FibTable.PieceTable)),
            wordDocument, System.Text.Encoding.Latin1, diags);
        Ww8FormattingTable chars = Ww8FormattingTable.Parse(
            table.AsSpan((int)fib.FileOffset(Ww8FibTable.CharacterFormattingIndex),
                (int)fib.Length(Ww8FibTable.CharacterFormattingIndex)), wordDocument, paragraphs: false);
        Ww8FieldTypes fields = Ww8FieldTypes.Parse(
            table.AsSpan((int)fib.FileOffset(Ww8FibTable.BodyFields),
                (int)fib.Length(Ww8FibTable.BodyFields)), diags);

        string text = pieces.ReadText(0, fib.TextLength, diags);
        Stack<int> open = new();
        Stack<bool> code = new();
        int depth = 0, tocFields = 0, inside = 0, hyperlink = 0, otherStyle = 0, decoratedBefore = 0;

        for (int i = 0; i < text.Length; i++)
        {
            char c = text[i];
            if (c == '')
            {
                int type = fields.At(i) ?? 0;
                if (type is 13 or 8)
                {
                    tocFields++;

                    // The run that ends where the field begins: what a decoration open at that point
                    // would be. Counted only when it states one and the field's own CHPX does not.
                    if (i > 0 && StatesDecoration(chars.Find(pieces.FileOffsetOf(i - 1)))) decoratedBefore++;
                }

                open.Push(type);
                code.Push(true);
                continue;
            }

            if (c == '')
            {
                if (code.Count > 0) { code.Pop(); code.Push(false); }
                if (open.Count > 0 && open.Peek() is 13 or 8) depth++;
                continue;
            }

            if (c == '')
            {
                if (code.Count > 0) code.Pop();
                int closed = open.Count > 0 ? open.Pop() : 0;
                if (closed is 13 or 8 && depth > 0) depth--;
                continue;
            }

            if (code.Count > 0 && code.Peek()) continue;
            if (depth == 0) continue;

            inside++;
            ushort istd = StyleIn(chars.Find(pieces.FileOffsetOf(i)));
            if (istd == 0) continue;
            if (styles.At(istd) is { IsCharacterStyle: true, Sti: Ww8Style.HyperlinkStyle }) hyperlink++;
            else otherStyle++;
        }

        Console.WriteLine($"{System.IO.Path.GetFileName(path)}\t{tocFields}\t{inside}\t{hyperlink}"
            + $"\t{otherStyle}\t{decoratedBefore}");
    }

    private static ushort StyleIn(ReadOnlyMemory<byte> grpprl)
    {
        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(grpprl))
        {
            if (sprm.Identifier == Ww8SprmReader.Ids.CharacterStyle) return sprm.Word;
        }

        return 0;
    }

    private static bool StatesDecoration(ReadOnlyMemory<byte> grpprl)
    {
        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(grpprl))
        {
            // sprmCKul, the underline, which is what the one witness states.
            if (sprm.Identifier == 0x2A3E && sprm.Byte != 0) return true;
        }

        return false;
    }
}

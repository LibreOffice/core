// The second half of the probe: the body's TOC/INDEX fields, and the character styles the runs
// inside their cached results name.
using XRay.Office.Core.Diagnostics;
using XRay.Office.WordProcessing.Ww8;

internal static class Fields
{
    internal static void Dump(byte[] wordDocument, byte[] table, Ww8Fib fib, Ww8StyleSheet styles)
    {
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
        Console.WriteLine($"body chars: {text.Length}, field beginnings: {fields.Count}");

        Stack<int> open = new();
        Stack<bool> inCode = new();
        int tocDepth = 0;
        Dictionary<int, int> inToc = [];
        Dictionary<int, int> outside = [];
        List<(int Start, int End)> results = [];
        int resultStart = -1;
        Dictionary<int, int> fieldTypeCounts = [];

        for (int i = 0; i < text.Length; i++)
        {
            char c = text[i];
            if (c == FieldBegin)
            {
                int type = fields.At(i) ?? 0;
                fieldTypeCounts[type] = fieldTypeCounts.GetValueOrDefault(type) + 1;
                open.Push(type);
                inCode.Push(true);
                continue;
            }

            if (c == FieldSeparator)
            {
                if (inCode.Count > 0) { inCode.Pop(); inCode.Push(false); }
                if (open.Count > 0 && open.Peek() is 13 or 8)
                {
                    if (tocDepth == 0) resultStart = i + 1;
                    tocDepth++;
                }

                continue;
            }

            if (c == FieldEnd)
            {
                if (inCode.Count > 0) inCode.Pop();
                int closed = open.Count > 0 ? open.Pop() : 0;
                if (closed is 13 or 8 && tocDepth > 0)
                {
                    tocDepth--;
                    if (tocDepth == 0 && resultStart >= 0)
                    {
                        results.Add((resultStart, i));
                        resultStart = -1;
                    }
                }

                continue;
            }

            if (inCode.Count > 0 && inCode.Peek()) continue;

            ReadOnlyMemory<byte> chpx = chars.Find(pieces.FileOffsetOf(i));
            int istd = 0;
            foreach (Ww8Sprm sprm in Ww8SprmReader.Read(chpx))
            {
                if (sprm.Identifier == Ww8SprmReader.Ids.CharacterStyle) istd = sprm.Word;
            }

            Dictionary<int, int> bucket = tocDepth > 0 ? inToc : outside;
            bucket[istd] = bucket.GetValueOrDefault(istd) + 1;
        }

        Console.WriteLine("field types: " + string.Join(", ",
            fieldTypeCounts.OrderByDescending(p => p.Value).Select(p => $"{p.Key}x{p.Value}")));
        Console.WriteLine($"TOC/INDEX results: {results.Count}");
        foreach ((int s, int e) in results) Console.WriteLine($"  cp {s}..{e}");
        Console.WriteLine("character styles INSIDE a TOC result:");
        foreach ((int k, int v) in inToc.OrderByDescending(p => p.Value))
        {
            Console.WriteLine($"  istd {k} ({styles.NameOf(k) ?? "-"}): {v} chars");
        }

        Console.WriteLine("character styles OUTSIDE:");
        foreach ((int k, int v) in outside.OrderByDescending(p => p.Value).Take(8))
        {
            Console.WriteLine($"  istd {k} ({styles.NameOf(k) ?? "-"}): {v} chars");
        }
    }

    private const char FieldBegin = '';
    private const char FieldSeparator = '';
    private const char FieldEnd = '';
}

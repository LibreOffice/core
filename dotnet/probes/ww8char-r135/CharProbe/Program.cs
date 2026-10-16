// Two censuses of a WW8 document's character layer, read from the file rather than from a rendering.
//
//   CharProbe --toggles <doc>...   one row per document: where a toggle sprm's base style is decided
//                                  by the CHPX's own `sprmCIstd`, and where the two readings DISAGREE
//   CharProbe --fields  <doc>...   one row per document: CHPX runs that end exactly where a field
//                                  begins, split by whether that field reaches `Read_F_Tox`
using XRay.Office.Containers;
using XRay.Office.Containers.Ole2;
using XRay.Office.Core.Diagnostics;
using XRay.Office.WordProcessing.Ww8;

string mode = args.Length > 0 ? args[0] : "--toggles";
Census.Why = Environment.GetEnvironmentVariable("CHARPROBE_WHY") == "1";
Census.BiDi = Environment.GetEnvironmentVariable("CHARPROBE_BIDI") == "1";
if (mode == "--toggles")
{
    Console.WriteLine("document\tchpx_toggle_istd\tchars_istd\tchpx_disagree\tchars_disagree"
        + "\tchpx_toggle_no_istd\tchars_no_istd\tchpx_styletoggle\tchpx_styletoggle_disagree");
}
else
{
    Console.WriteLine("document\tindex_fields\tother_fields\tindex_run_ends\tindex_run_drawable"
        + "\tchars_index_run\tother_run_ends\tother_run_drawable");
}

foreach (string one in args[1..])
{
    try
    {
        Doc doc = Doc.Read(one);
        if (mode == "--toggles") Census.Toggles(one, doc); else Census.Fields(one, doc);
    }
    catch (Exception e)
    {
        Console.Error.WriteLine($"{Path.GetFileName(one)}: {e.GetType().Name} {e.Message}");
    }
}

internal static class Census
{
    /// <summary>Whether a disagreeing CHPX is described on stderr as well as counted.</summary>
    internal static bool Why { get; set; }

    /// <summary>
    /// The character toggle sprms `Read_BoldUsw` handles: `sprmCFBold` through `sprmCFVanish`, plus
    /// the out-of-sequence `sprmCFDStrike`. Every one takes the four-state operand.
    /// </summary>
    internal static readonly ushort[] ToggleSprms =
        [0x0835, 0x0836, 0x0837, 0x0838, 0x0839, 0x083A, 0x083B, 0x083C, 0x2A53];

    /// <summary>
    /// The BiDi pair `Read_BoldBiDiUsw` handles, against `m_n81BiDiFlags` rather than `m_n81Flags`.
    /// </summary>
    /// <remarks>
    /// `sprmCFBoldBi` and `sprmCFItalicBi` (`ww8par6.cxx`:3254-3320) follow the identical rule and
    /// neither is read by this tree at all, so the census asks how many CHPX state one.
    /// </remarks>
    internal static readonly ushort[] BiDiToggleSprms = [0x0858, 0x085A];

    /// <summary>Whether the census is counting the BiDi pair instead of the western nine.</summary>
    internal static bool BiDi { get; set; }

    /// <summary>Which bit of `m_n81Flags` a toggle sprm is, so a style's answer fits in one word.</summary>
    internal static int Bit(ushort id) => id switch { 0x2A53 => 8, 0x0858 => 0, 0x085A => 1, _ => id - 0x0835 };

    internal static void Toggles(string path, Doc doc)
    {
        int withStyle = 0, charsWith = 0, disagree = 0, charsDisagree = 0, noStyle = 0, charsNo = 0;
        int styleToggle = 0, styleToggleDisagree = 0;
        int lastFrom = -1;

        for (int i = 0; i < doc.Text.Length; i++)
        {
            (ReadOnlyMemory<byte> props, int from, int to) =
                doc.Chars.FindWithRange(doc.Pieces.FileOffsetOf(i));
            if (to <= from || from == lastFrom) continue;
            lastFrom = from;

            List<ushort> toggles = [];
            ushort? istd = null;
            foreach (Ww8Sprm sprm in Ww8SprmReader.Read(props))
            {
                if (sprm.Identifier == Ww8SprmReader.Ids.CharacterStyle) istd = sprm.Word;
                else if (Array.IndexOf(BiDi ? BiDiToggleSprms : ToggleSprms, sprm.Identifier) >= 0 && sprm.Byte >= 0x80)
                {
                    toggles.Add(sprm.Identifier);
                }
            }

            // The second half of the same rule, and the one the run's own sprms do not show: a
            // CHARACTER STYLE's own toggles are settled against its base chain when the style is read,
            // so layering the chain over the paragraph's resolved value answers differently wherever the
            // two disagree. Counted over every CHPX that names a style whose chain states such a toggle.
            if (istd is { } layered && layered != 0 && doc.StyleStatesRelativeToggle(layered))
            {
                styleToggle++;
                if (doc.StyleChainDisagrees(layered, doc.StyleFlags(doc.ParagraphStyleAt(i))))
                {
                    styleToggleDisagree++;
                }
            }

            if (toggles.Count == 0) continue;

            if (istd is not { } named)
            {
                noStyle++;
                charsNo += to - from;
                continue;
            }

            withStyle++;
            charsWith += to - from;

            // The two readings, and the old one is NOT "the paragraph style's own flags": this tree
            // layered the named character style's chain over the paragraph's resolved value first, so
            // the operand's base was the chain applied to the paragraph's answer. 26.2.4.2's is the
            // same chain applied to nothing, which is `m_n81Flags`. Comparing the paragraph's own flags
            // against the named style's instead counts a CHPX as disagreeing wherever the style states
            // the toggle at all, which over-counts: see this round's write-up.
            int paragraph = doc.StyleFlags(doc.ParagraphStyleAt(i));
            int fromParagraph = doc.StyleChainOver(named, paragraph);
            int fromNamed = doc.StyleChainOver(named, 0);
            foreach (ushort toggle in toggles)
            {
                if (((fromParagraph >> Bit(toggle)) & 1) == ((fromNamed >> Bit(toggle)) & 1)) continue;

                disagree++;
                charsDisagree += to - from;
                if (Why)
                {
                    Console.Error.WriteLine($"  {Path.GetFileName(path)} cp {i} sprm 0x{toggle:X4} "
                        + $"istd {named} chars {to - from} "
                        + $"text '{doc.Text.Substring(i, Math.Min(40, doc.Text.Length - i)).Replace('\r', ' ')}'");
                }

                break;
            }
        }

        Console.WriteLine($"{Path.GetFileName(path)}\t{withStyle}\t{charsWith}\t{disagree}"
            + $"\t{charsDisagree}\t{noStyle}\t{charsNo}\t{styleToggle}\t{styleToggleDisagree}");
    }

    /// <summary>
    /// A sprm that puts a drawable character attribute on the control stack.
    /// </summary>
    /// <remarks>
    /// `sprmCRsidText` and the other revision-save ids are not one, which is why a run carrying
    /// nothing else cannot show the defect even where it is stripped.
    /// </remarks>
    private static bool IsDrawable(ushort id) => id
        is 0x0835 or 0x0836 or 0x0837 or 0x0838 or 0x0839 or 0x083A or 0x083B or 0x083C or 0x2A53
        or 0x2A3E or 0x2A42 or 0x4A43 or 0x4A4F or 0x4A50 or 0x4A51 or 0x6870 or 0x4A30 or 0x2A44
        or 0x4A61;

    internal static void Fields(string path, Doc doc)
    {
        int index = 0, other = 0, indexEnds = 0, indexDrawable = 0, indexChars = 0, otherEnds = 0,
            otherDrawable = 0;

        for (int i = 0; i < doc.Text.Length; i++)
        {
            if (doc.Text[i] != Doc.FieldBegin) continue;

            int type = doc.Fields.At(i) ?? 0;
            bool isIndex = type is 13 or 8;
            if (isIndex) index++; else other++;
            if (i == 0) continue;

            int beginOffset = doc.Pieces.FileOffsetOf(i);
            (ReadOnlyMemory<byte> props, int from, int to) =
                doc.Chars.FindWithRange(doc.Pieces.FileOffsetOf(i - 1));
            if (to <= from || to != beginOffset) continue;

            bool drawable = false;
            foreach (Ww8Sprm sprm in Ww8SprmReader.Read(props))
            {
                if (IsDrawable(sprm.Identifier)) drawable = true;
            }

            if (isIndex)
            {
                indexEnds++;
                if (drawable)
                {
                    indexDrawable++;
                    indexChars += to - from;
                }
            }
            else
            {
                otherEnds++;
                if (drawable) otherDrawable++;
            }
        }

        Console.WriteLine($"{Path.GetFileName(path)}\t{index}\t{other}\t{indexEnds}\t{indexDrawable}"
            + $"\t{indexChars}\t{otherEnds}\t{otherDrawable}");
    }
}

internal sealed class Doc
{
    internal const char FieldBegin = (char)0x13;

    public required Ww8PieceTable Pieces { get; init; }

    public required Ww8FormattingTable Chars { get; init; }

    public required Ww8FormattingTable Paragraphs { get; init; }

    public required Ww8FieldTypes Fields { get; init; }

    public required Ww8StyleSheet Styles { get; init; }

    public required string Text { get; init; }

    /// <summary>How much of <see cref="Text"/> is the body story.</summary>
    public required int BodyLength { get; init; }

    /// <summary>The length of every story the piece table addresses.</summary>
    private static int TotalLength(Ww8Fib fib) => fib.TextLength + fib.FootnoteTextLength
        + fib.HeaderTextLength + fib.MacroTextLength + fib.AnnotationTextLength
        + fib.EndnoteTextLength + fib.TextBoxTextLength + fib.HeaderTextBoxTextLength;

    private readonly Dictionary<ushort, int> _flags = [];

    public ushort ParagraphStyleAt(int position)
    {
        int mark = Text.IndexOf('\r', position);
        if (mark < 0) mark = Text.Length - 1;
        (ushort istd, _) = Ww8FormattingTable.SplitParagraphProperties(
            Paragraphs.Find(Pieces.FileOffsetOf(mark)));
        return istd;
    }

    /// <summary>A style's resolved toggle bits, which is exactly LibreOffice's `m_n81Flags`.</summary>
    public int StyleFlags(ushort styleIndex)
    {
        if (_flags.TryGetValue(styleIndex, out int cached)) return cached;

        int flags = 0;
        foreach (ReadOnlyMemory<byte> half in Styles.ResolveCharacterChain(styleIndex))
        {
            foreach (Ww8Sprm sprm in Ww8SprmReader.Read(half))
            {
                if (Array.IndexOf(Census.ToggleSprms, sprm.Identifier) < 0) continue;

                int bit = Census.Bit(sprm.Identifier);
                bool on = sprm.Byte switch
                {
                    0 => false,
                    1 => true,
                    128 => ((flags >> bit) & 1) != 0,
                    129 => ((flags >> bit) & 1) == 0,
                    _ => sprm.Byte != 0,
                };
                flags = on ? flags | (1 << bit) : flags & ~(1 << bit);
            }
        }

        _flags[styleIndex] = flags;
        return flags;
    }

    /// <summary>
    /// A style chain's toggle bits, resolved over a given seed.
    /// </summary>
    /// <remarks>
    /// Seeded with nothing this is `m_n81Flags`; seeded with the paragraph's own answer it is what
    /// layering the chain over the paragraph's resolved format produced, which is what this tree did.
    /// </remarks>
    public int StyleChainOver(ushort styleIndex, int seed)
    {
        int flags = seed;
        foreach (ReadOnlyMemory<byte> half in Styles.ResolveCharacterChain(styleIndex))
        {
            foreach (Ww8Sprm sprm in Ww8SprmReader.Read(half))
            {
                if (Array.IndexOf(Census.ToggleSprms, sprm.Identifier) < 0) continue;

                int bit = Census.Bit(sprm.Identifier);
                bool on = sprm.Byte switch
                {
                    0 => false,
                    1 => true,
                    128 => ((flags >> bit) & 1) != 0,
                    129 => ((flags >> bit) & 1) == 0,
                    _ => sprm.Byte != 0,
                };
                flags = on ? flags | (1 << bit) : flags & ~(1 << bit);
            }
        }

        return flags;
    }

    /// <summary>Whether a style's own chain states a toggle relative to what it inherits.</summary>
    public bool StyleStatesRelativeToggle(ushort styleIndex)
    {
        foreach (ReadOnlyMemory<byte> half in Styles.ResolveCharacterChain(styleIndex))
        {
            foreach (Ww8Sprm sprm in Ww8SprmReader.Read(half))
            {
                if (Array.IndexOf(Census.ToggleSprms, sprm.Identifier) >= 0 && sprm.Byte >= 0x80)
                {
                    return true;
                }
            }
        }

        return false;
    }

    /// <summary>
    /// Whether resolving a character style's chain from nothing answers differently from resolving it
    /// over the paragraph style's value — the second half of the same defect.
    /// </summary>
    public bool StyleChainDisagrees(ushort styleIndex, int paragraphFlags)
    {
        int fromNone = 0, overParagraph = paragraphFlags;
        foreach (ReadOnlyMemory<byte> half in Styles.ResolveCharacterChain(styleIndex))
        {
            foreach (Ww8Sprm sprm in Ww8SprmReader.Read(half))
            {
                if (Array.IndexOf(Census.ToggleSprms, sprm.Identifier) < 0) continue;

                int bit = Census.Bit(sprm.Identifier);
                fromNone = Apply(fromNone, bit, sprm.Byte);
                overParagraph = Apply(overParagraph, bit, sprm.Byte);
            }
        }

        // Only the bits the chain actually decides can differ; a bit it never touches keeps its seed.
        for (int bit = 0; bit <= 8; bit++)
        {
            if (!Touches(styleIndex, bit)) continue;
            if (((fromNone >> bit) & 1) != ((overParagraph >> bit) & 1)) return true;
        }

        return false;

        static int Apply(int flags, int bit, byte operand)
        {
            bool on = operand switch
            {
                0 => false,
                1 => true,
                128 => ((flags >> bit) & 1) != 0,
                129 => ((flags >> bit) & 1) == 0,
                _ => operand != 0,
            };
            return on ? flags | (1 << bit) : flags & ~(1 << bit);
        }
    }

    private bool Touches(ushort styleIndex, int bit)
    {
        foreach (ReadOnlyMemory<byte> half in Styles.ResolveCharacterChain(styleIndex))
        {
            foreach (Ww8Sprm sprm in Ww8SprmReader.Read(half))
            {
                if (Array.IndexOf(Census.ToggleSprms, sprm.Identifier) >= 0
                    && Census.Bit(sprm.Identifier) == bit)
                {
                    return true;
                }
            }
        }

        return false;
    }

    public static Doc Read(string path)
    {
        using FileStream stream = File.OpenRead(path);
        CompoundFile file = CompoundFile.Open(stream, leaveOpen: true);
        byte[] word = Part(file, "WordDocument")!;
        Ww8Fib fib = Ww8Fib.Parse(word);
        byte[] table = Part(file, fib.UsesTable1Stream ? "1Table" : "0Table") ?? [];
        List<Diagnostic> diags = [];

        Ww8PieceTable pieces = Ww8PieceTable.Parse(
            table.AsSpan((int)fib.FileOffset(Ww8FibTable.PieceTable),
                (int)fib.Length(Ww8FibTable.PieceTable)), word, System.Text.Encoding.Latin1, diags);

        return new Doc
        {
            Pieces = pieces,
            Chars = Ww8FormattingTable.Parse(
                table.AsSpan((int)fib.FileOffset(Ww8FibTable.CharacterFormattingIndex),
                    (int)fib.Length(Ww8FibTable.CharacterFormattingIndex)), word, paragraphs: false),
            Paragraphs = Ww8FormattingTable.Parse(
                table.AsSpan((int)fib.FileOffset(Ww8FibTable.ParagraphFormattingIndex),
                    (int)fib.Length(Ww8FibTable.ParagraphFormattingIndex)), word, paragraphs: true),
            Fields = Ww8FieldTypes.Parse(
                table.AsSpan((int)fib.FileOffset(Ww8FibTable.BodyFields),
                    (int)fib.Length(Ww8FibTable.BodyFields)), diags),
            Styles = Ww8StyleSheet.Parse(
                table.AsSpan((int)fib.FileOffset(Ww8FibTable.StyleSheet),
                    (int)fib.Length(Ww8FibTable.StyleSheet))),
            // Every story, not just the body: a run in a header, a footnote or a text box is drawn
            // and resolves its formatting the same way, and a census of the body alone under-counts.
            Text = pieces.ReadText(0, TotalLength(fib), diags),
            BodyLength = fib.TextLength,
        };
    }

    private static byte[]? Part(CompoundFile package, string name)
    {
        IPackagePart? part = package.GetPart(name);
        if (part is null) return null;

        using Stream stream = part.Open();
        using MemoryStream buffer = new();
        stream.CopyTo(buffer);
        return buffer.ToArray();
    }
}

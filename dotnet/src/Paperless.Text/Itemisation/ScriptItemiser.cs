namespace Paperless.Text.Itemisation;

/// <summary>
/// A stretch of text in one script.
/// </summary>
/// <param name="Start">Its first character, as an index into the text.</param>
/// <param name="Length">How many UTF-16 code units it covers.</param>
/// <param name="Script">Its ISO 15924 code, ready to hand to a shaper.</param>
public readonly record struct ScriptRun(int Start, int Length, string Script)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;
}

/// <summary>
/// Splits text into runs of one script, UAX #24.
/// </summary>
/// <remarks>
/// <para>
/// A port of the algorithm LibreOffice uses, which is ICU's <c>ScriptRun</c> sample vendored into
/// <c>vcl/source/gdi/scrptrun.cxx</c> and driven from <c>vcl::text::TextLayoutCache</c>. Ported rather
/// than reinvented because HarfBuzz gives different answers for a run tagged with the wrong script,
/// and LibreOffice's answers are the ones that have to be reproduced.
/// </para>
/// <para>
/// The interesting half is the resolution rule: characters of script <c>Common</c> or
/// <c>Inherited</c> — the space, the full stop, the digits, a combining mark — have no script of
/// their own and take their neighbours'. Splitting a run at every one of them would be both wrong and
/// expensive, because a shaper loses its context at a run boundary and the measured width changes
/// with it.
/// </para>
/// <para>
/// The bracket stack is the part that looks like an accident and is not. A parenthesis is
/// <c>Common</c>, so its script comes from what is inside it; but the *closing* parenthesis must
/// match the *opening* one rather than what precedes it, or "(Ελληνικά) English" would put the two
/// halves of the pair in different runs. The stack carries the script each open bracket was pushed
/// with and back-fills it once the run's script is finally known.
/// </para>
/// </remarks>
public static class ScriptItemiser
{
    /// <summary>
    /// The paired punctuation the run splitter tracks, as flat pairs.
    /// </summary>
    /// <remarks>
    /// Exactly LibreOffice's list — ASCII paired punctuation and the angle brackets, the guillemets,
    /// the curly quotes and the single guillemets, and the CJK brackets
    /// (<c>PairIndices</c> in <c>vcl/source/gdi/scrptrun.cxx</c>). Not the Unicode bracket property,
    /// which is a different and larger set: matching LibreOffice's run boundaries means matching the
    /// list it actually consults.
    /// </remarks>
    private static readonly int[] Pairs =
    [
        0x0028, 0x0029, 0x003C, 0x003E, 0x005B, 0x005D, 0x007B, 0x007D,
        0x00AB, 0x00BB,
        0x2018, 0x2019, 0x201C, 0x201D, 0x2039, 0x203A,
        0x3008, 0x3009, 0x300A, 0x300B, 0x300C, 0x300D, 0x300E, 0x300F,
        0x3010, 0x3011, 0x3014, 0x3015, 0x3016, 0x3017, 0x3018, 0x3019,
        0x301A, 0x301B,
    ];

    /// <summary>The script runs of a text, in order, partitioning it.</summary>
    public static List<ScriptRun> Itemise(ReadOnlySpan<char> text)
    {
        List<ScriptRun> runs = [];
        if (text.Length == 0) return runs;

        List<PairEntry> stack = [];
        int stackTop = -1;
        int at = 0;

        while (at < text.Length)
        {
            int startTop = stackTop;
            int start = at;
            string script = ScriptProperties.Common;

            while (at < text.Length)
            {
                int width = 1;
                int codePoint = text[at];
                if (char.IsHighSurrogate(text[at]) && at + 1 < text.Length
                    && char.IsLowSurrogate(text[at + 1]))
                {
                    codePoint = char.ConvertToUtf32(text[at], text[at + 1]);
                    width = 2;
                }

                string characterScript = ScriptProperties.ScriptOf(codePoint);
                int pairIndex = PairIndexOf(codePoint);

                if (pairIndex >= 0)
                {
                    if ((pairIndex & 1) == 0)
                    {
                        // An opening bracket, pushed with whatever the run's script is so far — which
                        // may still be Common, and is back-filled below once a strong script arrives.
                        stackTop++;
                        if (stackTop == stack.Count) stack.Add(default);
                        stack[stackTop] = new PairEntry(pairIndex, script);
                    }
                    else if (stackTop >= 0)
                    {
                        int wanted = pairIndex & ~1;
                        while (stackTop >= 0 && stack[stackTop].PairIndex != wanted) stackTop--;
                        if (stackTop < startTop) startTop = stackTop;
                        if (stackTop >= 0) characterScript = stack[stackTop].Script;
                    }
                }

                if (!SameScript(script, characterScript))
                {
                    break;
                }

                if (ScriptProperties.IsWeak(script) && !ScriptProperties.IsWeak(characterScript))
                {
                    script = characterScript;

                    // Now that the run's script is known, the brackets opened before it was tell
                    // their closers the right answer.
                    while (startTop < stackTop)
                    {
                        startTop++;
                        stack[startTop] = stack[startTop] with { Script = script };
                    }
                }

                if (pairIndex >= 0 && (pairIndex & 1) != 0 && stackTop >= 0)
                {
                    stackTop--;
                    if (startTop >= 0) startTop--;
                }

                at += width;
            }

            runs.Add(new ScriptRun(start, at - start, script));
        }

        return runs;
    }

    /// <summary>
    /// Whether two scripts can share a run.
    /// </summary>
    /// <remarks>
    /// Either being weak is enough, which is what lets a space sit inside a Greek run and a full stop
    /// inside an Arabic one. <c>Zzzz</c> is deliberately <em>not</em> weak: an unassigned character
    /// really does end a run, because whatever face ends up drawing its box is not the face drawing
    /// the text around it.
    /// </remarks>
    private static bool SameScript(string a, string b)
        => ScriptProperties.IsWeak(a)
           || ScriptProperties.IsWeak(b)
           || string.Equals(a, b, StringComparison.Ordinal);

    /// <summary>The index of a code point in the paired-punctuation list, or -1.</summary>
    private static int PairIndexOf(int codePoint)
    {
        // Nearly every character is outside the list's range, and this is asked once per character of
        // every paragraph, so the cheap rejection is worth stating.
        if (codePoint is < 0x0028 or > 0x301B) return -1;

        for (int i = 0; i < Pairs.Length; i++)
        {
            if (Pairs[i] == codePoint) return i;
        }

        return -1;
    }

    /// <summary>One open bracket, and the script the run had when it was seen.</summary>
    private readonly record struct PairEntry(int PairIndex, string Script);
}

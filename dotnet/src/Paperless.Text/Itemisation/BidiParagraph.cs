namespace Paperless.Text.Itemisation;

/// <summary>Which way a paragraph reads, before its own text is looked at.</summary>
public enum BidiDirection
{
    /// <summary>Take the direction from the paragraph's first strong character (UAX #9 P2/P3).</summary>
    Auto = 0,

    /// <summary>Left to right, whatever the text contains.</summary>
    LeftToRight,

    /// <summary>Right to left, whatever the text contains.</summary>
    RightToLeft,
}

/// <summary>
/// A stretch of text at one embedding level.
/// </summary>
/// <param name="Start">Its first character, as an index into the paragraph's text.</param>
/// <param name="Length">How many UTF-16 code units it covers.</param>
/// <param name="Level">Its resolved embedding level; odd means right to left.</param>
public readonly record struct BidiRun(int Start, int Length, byte Level)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;

    /// <summary>True when the run reads right to left, which is what an odd level means.</summary>
    public bool IsRightToLeft => (Level & 1) != 0;
}

/// <summary>
/// A paragraph with the Unicode Bidirectional Algorithm resolved over it.
/// </summary>
/// <remarks>
/// <para>
/// UAX #9 in full: the paragraph level (P2, P3), the explicit levels and isolates (X1–X8), the
/// isolating run sequences (X10), the weak, neutral and implicit resolutions (W1–W7, N0–N2, I1, I2)
/// and the reordering (L1, L2). Written in the standard's own order, rule by rule, for the same
/// reason the line breaker is: when a level comes out wrong the only useful question is which rule
/// decided it, and a table-driven implementation cannot answer that.
/// </para>
/// <para>
/// It is here rather than delegated because LibreOffice delegates to ICU
/// (<c>ubidi_setPara</c>, <c>vcl/source/text/ImplLayoutArgs.cxx</c>) and Paperless has no ICU to
/// delegate to. The property tables are read from ICU's own data, so the input to the algorithm is
/// the same even though the code is not.
/// </para>
/// <para>
/// The algorithm runs over UTF-16 code units rather than code points, which is what ICU does too. A
/// surrogate pair takes its whole code point's class on both halves, so the two always land at the
/// same level and no run boundary can ever fall between them.
/// </para>
/// </remarks>
public sealed class BidiParagraph
{
    /// <summary>The deepest embedding level UAX #9 allows (BD2).</summary>
    private const byte MaxDepth = 125;

    /// <summary>A level meaning "not yet assigned", used while filling in the X9-removed characters.</summary>
    private const int Unassigned = -1;

    private readonly BidiClass[] _initialClasses;
    private readonly BidiClass[] _classes;
    private readonly byte[] _levels;
    private readonly BidiRun[] _runs;

    private BidiParagraph(
        byte paragraphLevel,
        BidiClass[] initialClasses,
        BidiClass[] classes,
        byte[] levels,
        BidiRun[] runs)
    {
        ParagraphLevel = paragraphLevel;
        _initialClasses = initialClasses;
        _classes = classes;
        _levels = levels;
        _runs = runs;
    }

    /// <summary>The paragraph's own embedding level: even for left to right, odd for right to left.</summary>
    public byte ParagraphLevel { get; }

    /// <summary>True when the paragraph as a whole reads right to left.</summary>
    public bool IsRightToLeft => (ParagraphLevel & 1) != 0;

    /// <summary>The resolved embedding level of every character, one per UTF-16 code unit.</summary>
    public ReadOnlySpan<byte> Levels => _levels;

    /// <summary>
    /// The level runs, in the order the characters are stored.
    /// </summary>
    /// <remarks>
    /// Logical order, not visual: this is what a portion of text is, and Writer's own line portions
    /// are in the same order — its PDF export emits them logically and positions them visually, so a
    /// comparison against its output matches these one for one.
    /// </remarks>
    public IReadOnlyList<BidiRun> Runs => _runs;

    /// <summary>The Bidi_Class of a character as the tables gave it, before any rule ran.</summary>
    public BidiClass InitialClassAt(int index) => _initialClasses[index];

    /// <summary>
    /// The Bidi_Class of a character once the weak and neutral rules have resolved it.
    /// </summary>
    /// <remarks>
    /// Worth exposing because the level does not say everything: European and Arabic digits resolve
    /// to the same level beside Hebrew and to different classes, and which one a character ended up
    /// as is what W4's number joining and a shaper's digit substitution both turn on.
    /// </remarks>
    public BidiClass ResolvedClassAt(int index) => _classes[index];

    /// <summary>
    /// Resolves the algorithm over a paragraph.
    /// </summary>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="baseDirection">
    /// The direction the paragraph is declared to have. <see cref="BidiDirection.Auto"/> derives it
    /// from the text; anything else is honoured whatever the text says, which is what a word processor
    /// needs — an English paragraph containing nothing but Hebrew is still left-aligned, and
    /// LibreOffice takes the paragraph's writing mode rather than sniffing its content.
    /// </param>
    public static BidiParagraph Resolve(
        ReadOnlySpan<char> text, BidiDirection baseDirection = BidiDirection.LeftToRight)
    {
        BidiClass[] initial = ClassifyEach(text);

        byte paragraphLevel = baseDirection switch
        {
            BidiDirection.LeftToRight => 0,
            BidiDirection.RightToLeft => 1,
            _ => AutoLevel(initial, MatchingPdi(initial), 0, initial.Length),
        };

        return Resolve(text, initial, paragraphLevel);
    }

    /// <summary>
    /// The runs in the order they are drawn left to right, which is rule L2.
    /// </summary>
    /// <remarks>
    /// Runs are reversed rather than characters. That is the same answer L2 gives per character —
    /// every character in a run shares its level, so reversing the run and reversing its characters
    /// pick out the same ordering — and it is the answer a shaper needs, because the glyphs inside a
    /// run are put in visual order by the shaper itself rather than by this.
    /// </remarks>
    public IReadOnlyList<BidiRun> InVisualOrder()
    {
        List<BidiRun> order = [.. _runs];
        if (order.Count < 2) return order;

        byte highest = 0;
        byte lowestOdd = MaxDepth + 1;
        foreach (BidiRun run in order)
        {
            if (run.Level > highest) highest = run.Level;
            if ((run.Level & 1) != 0 && run.Level < lowestOdd) lowestOdd = run.Level;
        }

        for (int level = highest; level >= lowestOdd; level--)
        {
            for (int i = 0; i < order.Count; i++)
            {
                if (order[i].Level < level) continue;

                int limit = i + 1;
                while (limit < order.Count && order[limit].Level >= level) limit++;
                order.Reverse(i, limit - i);
                i = limit;
            }
        }

        return order;
    }

    /// <summary>
    /// The Bidi_Class of every UTF-16 code unit.
    /// </summary>
    /// <remarks>
    /// Both halves of a surrogate pair take the class of the code point they spell, so the pair can
    /// never be split by a level boundary. Assigning the low surrogate something else — <c>BN</c>, say
    /// — would be defensible for the levels and disastrous for the reordering, which would then be
    /// free to reverse the two halves independently.
    /// </remarks>
    private static BidiClass[] ClassifyEach(ReadOnlySpan<char> text)
    {
        BidiClass[] classes = new BidiClass[text.Length];

        for (int i = 0; i < text.Length; i++)
        {
            int codePoint = text[i];
            int width = 1;

            if (char.IsHighSurrogate(text[i]) && i + 1 < text.Length && char.IsLowSurrogate(text[i + 1]))
            {
                codePoint = char.ConvertToUtf32(text[i], text[i + 1]);
                width = 2;
            }

            BidiClass resolved = BidiProperties.ClassOf(codePoint);
            for (int j = 0; j < width; j++) classes[i + j] = resolved;
            i += width - 1;
        }

        return classes;
    }

    /// <summary>The whole algorithm, once the paragraph's level is known.</summary>
    private static BidiParagraph Resolve(
        ReadOnlySpan<char> text, BidiClass[] initial, byte paragraphLevel)
    {
        if (FlatLevel(initial, paragraphLevel) is { } single)
        {
            byte[] flat = new byte[initial.Length];
            Array.Fill(flat, single);
            return new BidiParagraph(paragraphLevel, initial, [.. initial], flat, RunsOf(flat));
        }

        int length = initial.Length;
        BidiClass[] classes = [.. initial];
        int[] levels = new int[length];
        int[] matchingPdi = MatchingPdi(initial);

        ExplicitLevels(initial, classes, levels, matchingPdi, paragraphLevel);

        // Every sequence's own level and its two boundary types are worked out before any of them is
        // resolved, because they are read off the *explicit* levels. Computing them lazily inside the
        // loop would let a sequence already resolved by I1 or I2 answer for the one after it, and a
        // sequence next to text the implicit rules had raised would see the wrong direction beyond
        // its edge. That is a one-character bug with a whole-paragraph effect: a space at a boundary
        // taking the paragraph's direction instead of the embedding's puts every later run's pen in
        // the wrong place.
        List<int[]> sequences = IsolatingRunSequences(initial, levels, matchingPdi);
        List<(byte Level, BidiClass Sos, BidiClass Eos)> boundaries = [];

        foreach (int[] sequence in sequences)
        {
            byte level = (byte)levels[sequence[0]];
            (BidiClass sos, BidiClass eos) =
                Boundaries(sequence, initial, levels, level, paragraphLevel);
            boundaries.Add((level, sos, eos));
        }

        for (int i = 0; i < sequences.Count; i++)
        {
            ResolveSequence(text, sequences[i], initial, classes, levels, boundaries[i]);
        }

        byte[] final = FinaliseLevels(initial, levels, paragraphLevel);

        return new BidiParagraph(paragraphLevel, initial, classes, final, RunsOf(final));
    }

    /// <summary>
    /// The one level the whole paragraph sits at, when it has one — ICU's short circuit.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A departure from the letter of UAX #9, taken deliberately because it is what LibreOffice
    /// sees. ICU's <c>directionFromFlags</c> looks only at which classes are present: with nothing
    /// right-to-left, and no Arabic number that a neighbouring neutral could drag right-to-left, it
    /// declares the paragraph left to right and reports every character at the paragraph's level
    /// without running the rules at all. Strict UAX #9 would put an Arabic number in an English
    /// paragraph two levels deep by I1; ICU leaves it where it is, and that is the difference between
    /// two portions and one.
    /// </para>
    /// <para>
    /// The Arabic-number clause is the subtle half and was found by measurement: ICU resolves "٠٠"
    /// flat and "٠ ٠" as 2 1 2, because the space between two Arabic numbers can itself become
    /// right-to-left under N1 and a paragraph containing one is therefore not uniformly left to
    /// right after all.
    /// </para>
    /// <para>
    /// It is safe as well as faithful. Every level the rules would have produced under this condition
    /// shares the paragraph's parity, and L2 reverses nothing when all levels share a parity — so the
    /// visual order is identical either way and only the run count differs.
    /// </para>
    /// </remarks>
    private static byte? FlatLevel(BidiClass[] initial, byte paragraphLevel)
    {
        bool anyRight = false;
        bool anyArabicNumber = false;
        bool anyPossibleNeutral = false;

        foreach (BidiClass type in initial)
        {
            switch (type)
            {
                case BidiClass.AN:
                    anyArabicNumber = true;
                    break;
                case BidiClass.LRE:
                case BidiClass.LRO:
                    anyPossibleNeutral = true;
                    break;
                case BidiClass.R:
                case BidiClass.AL:
                case BidiClass.RLI:
                    anyRight = true;
                    break;
                case BidiClass.RLE:
                case BidiClass.RLO:
                    anyRight = true;
                    anyPossibleNeutral = true;
                    break;
                case BidiClass.ON:
                case BidiClass.CS:
                case BidiClass.ES:
                case BidiClass.ET:
                case BidiClass.B:
                case BidiClass.S:
                case BidiClass.WS:
                case BidiClass.BN:
                case BidiClass.PDF:
                    anyPossibleNeutral = true;
                    break;
            }
        }

        return (paragraphLevel & 1) == 0
               && !anyRight
               && !(anyArabicNumber && anyPossibleNeutral)
            ? paragraphLevel
            : null;
    }

    /// <summary>
    /// P2 and P3: the level implied by the first strong character, skipping isolated runs.
    /// </summary>
    /// <remarks>
    /// Also serves X5c, where an FSI's direction is the level its own contents imply — which is why
    /// it takes a range rather than reading the whole paragraph.
    /// </remarks>
    private static byte AutoLevel(BidiClass[] classes, int[] matchingPdi, int start, int end)
    {
        for (int i = start; i < end; i++)
        {
            switch (classes[i])
            {
                case BidiClass.L:
                    return 0;
                case BidiClass.R:
                case BidiClass.AL:
                    return 1;
                case BidiClass.LRI:
                case BidiClass.RLI:
                case BidiClass.FSI:
                    // The contents of an isolate say nothing about the paragraph around it, so skip
                    // to its matching PDI rather than reading the first strong character inside it.
                    i = matchingPdi[i];
                    break;
            }
        }

        return 0;
    }

    /// <summary>BD9: the PDI that matches each isolate initiator, or the end of the text.</summary>
    private static int[] MatchingPdi(BidiClass[] classes)
    {
        int[] matching = new int[classes.Length];

        for (int i = 0; i < classes.Length; i++)
        {
            matching[i] = -1;
            if (classes[i] is not (BidiClass.LRI or BidiClass.RLI or BidiClass.FSI)) continue;

            int depth = 1;
            int j = i + 1;
            for (; j < classes.Length; j++)
            {
                if (classes[j] is BidiClass.LRI or BidiClass.RLI or BidiClass.FSI) depth++;
                else if (classes[j] == BidiClass.PDI && --depth == 0) break;
            }

            matching[i] = j;
        }

        return matching;
    }

    /// <summary>The isolate initiator each PDI matches, or -1 for an unmatched one.</summary>
    private static int[] MatchingInitiator(BidiClass[] classes, int[] matchingPdi)
    {
        int[] matching = new int[classes.Length];
        Array.Fill(matching, -1);

        for (int i = 0; i < classes.Length; i++)
        {
            if (matchingPdi[i] > 0 && matchingPdi[i] < classes.Length) matching[matchingPdi[i]] = i;
        }

        return matching;
    }

    /// <summary>
    /// X1 to X8: the levels the embedding and isolate controls set.
    /// </summary>
    /// <remarks>
    /// The two overflow counters and the valid-isolate count are the whole of what makes this
    /// well-defined for malformed input, which every real document eventually contains: an
    /// unterminated embedding must not raise the level of the rest of the paragraph, and a PDI with
    /// nothing to close must not pop the stack out from under a legitimate one.
    /// </remarks>
    private static void ExplicitLevels(
        BidiClass[] initial,
        BidiClass[] classes,
        int[] levels,
        int[] matchingPdi,
        byte paragraphLevel)
    {
        Stack<StatusEntry> stack = new();
        stack.Push(new StatusEntry(paragraphLevel, BidiClass.ON, IsIsolate: false));

        int overflowIsolates = 0;
        int overflowEmbeddings = 0;
        int validIsolates = 0;

        for (int i = 0; i < initial.Length; i++)
        {
            switch (initial[i])
            {
                case BidiClass.RLE:
                case BidiClass.LRE:
                case BidiClass.RLO:
                case BidiClass.LRO:
                case BidiClass.RLI:
                case BidiClass.LRI:
                case BidiClass.FSI:
                {
                    BidiClass control = initial[i];
                    bool isolate = control is BidiClass.RLI or BidiClass.LRI or BidiClass.FSI;

                    // X5c: an FSI takes the direction its own contents imply.
                    bool rightToLeft = control switch
                    {
                        BidiClass.FSI => AutoLevel(initial, matchingPdi, i + 1, matchingPdi[i]) == 1,
                        BidiClass.RLE or BidiClass.RLO or BidiClass.RLI => true,
                        _ => false,
                    };

                    StatusEntry top = stack.Peek();

                    if (isolate)
                    {
                        // An isolate initiator is part of the text outside it, so it keeps the level
                        // it was found at and takes any override in force there.
                        levels[i] = top.Level;
                        if (top.Override != BidiClass.ON) classes[i] = top.Override;
                    }

                    int newLevel = rightToLeft ? (top.Level + 1) | 1 : (top.Level + 2) & ~1;

                    if (newLevel <= MaxDepth && overflowIsolates == 0 && overflowEmbeddings == 0)
                    {
                        if (isolate) validIsolates++;

                        stack.Push(new StatusEntry(
                            newLevel,
                            control switch
                            {
                                BidiClass.LRO => BidiClass.L,
                                BidiClass.RLO => BidiClass.R,
                                _ => BidiClass.ON,
                            },
                            isolate));

                        if (!isolate) levels[i] = newLevel;
                    }
                    else if (isolate)
                    {
                        overflowIsolates++;
                    }
                    else if (overflowIsolates == 0)
                    {
                        overflowEmbeddings++;
                    }

                    break;
                }

                case BidiClass.PDI:
                {
                    // X6a. An overflowing isolate is closed first; a PDI with no isolate open at all
                    // does nothing, which is what keeps stray ones harmless.
                    if (overflowIsolates > 0)
                    {
                        overflowIsolates--;
                    }
                    else if (validIsolates > 0)
                    {
                        overflowEmbeddings = 0;
                        while (!stack.Peek().IsIsolate) stack.Pop();
                        stack.Pop();
                        validIsolates--;
                    }

                    StatusEntry top = stack.Peek();
                    levels[i] = top.Level;
                    if (top.Override != BidiClass.ON) classes[i] = top.Override;
                    break;
                }

                case BidiClass.PDF:
                {
                    // X7. The PDF itself stays at the level it closes into, not the one it closes.
                    levels[i] = stack.Peek().Level;

                    if (overflowIsolates > 0)
                    {
                        // An embedding inside an overflowing isolate was never opened.
                    }
                    else if (overflowEmbeddings > 0)
                    {
                        overflowEmbeddings--;
                    }
                    else if (!stack.Peek().IsIsolate && stack.Count >= 2)
                    {
                        stack.Pop();
                    }

                    break;
                }

                case BidiClass.B:
                {
                    // X8. A paragraph separator ends everything: it can only be the last character of
                    // the paragraph, and it takes the paragraph's own level.
                    stack.Clear();
                    stack.Push(new StatusEntry(paragraphLevel, BidiClass.ON, IsIsolate: false));
                    overflowIsolates = 0;
                    overflowEmbeddings = 0;
                    validIsolates = 0;
                    levels[i] = paragraphLevel;
                    break;
                }

                default:
                {
                    StatusEntry top = stack.Peek();
                    levels[i] = top.Level;
                    if (top.Override != BidiClass.ON) classes[i] = top.Override;
                    break;
                }
            }
        }
    }

    /// <summary>True for the characters X9 removes from further consideration.</summary>
    /// <remarks>
    /// They are not deleted — a level has to come out for every character, because the caller indexes
    /// by character — but they take no part in the weak, neutral or implicit rules, and they are
    /// invisible to the "adjacent" in every one of those rules. Leaving them in would let an
    /// invisible character break a bracket pair or a number sequence.
    /// </remarks>
    private static bool RemovedByX9(BidiClass type) => type
        is BidiClass.LRE or BidiClass.RLE or BidiClass.LRO or BidiClass.RLO
        or BidiClass.PDF or BidiClass.BN;

    /// <summary>
    /// X10: the isolating run sequences, each as the indices of the characters it covers.
    /// </summary>
    /// <remarks>
    /// A level run is a maximal stretch at one level. A sequence chains level runs together across
    /// an isolate: the text before an LRI and the text after its matching PDI are one context, so a
    /// number after the isolate still sees the strong character before it. Getting this wrong is not
    /// visible until a document uses isolates, which is exactly when it matters.
    /// </remarks>
    private static List<int[]> IsolatingRunSequences(
        BidiClass[] initial, int[] levels, int[] matchingPdi)
    {
        List<int[]> levelRuns = [];
        List<int> current = [];
        int currentLevel = Unassigned;

        for (int i = 0; i < initial.Length; i++)
        {
            if (RemovedByX9(initial[i])) continue;

            if (levels[i] != currentLevel)
            {
                if (current.Count > 0) levelRuns.Add([.. current]);
                current.Clear();
                currentLevel = levels[i];
            }

            current.Add(i);
        }

        if (current.Count > 0) levelRuns.Add([.. current]);

        int[] runOfCharacter = new int[initial.Length];
        for (int run = 0; run < levelRuns.Count; run++)
        {
            foreach (int at in levelRuns[run]) runOfCharacter[at] = run;
        }

        int[] matchingInitiator = MatchingInitiator(initial, matchingPdi);

        List<int[]> sequences = [];
        for (int run = 0; run < levelRuns.Count; run++)
        {
            int first = levelRuns[run][0];

            // A run beginning with a matched PDI is the continuation of an earlier sequence, so it is
            // not the start of one.
            if (initial[first] == BidiClass.PDI && matchingInitiator[first] != -1) continue;

            List<int> sequence = [];
            int at = run;
            while (true)
            {
                sequence.AddRange(levelRuns[at]);

                int last = sequence[^1];
                if (initial[last] is BidiClass.LRI or BidiClass.RLI or BidiClass.FSI
                    && matchingPdi[last] < initial.Length)
                {
                    at = runOfCharacter[matchingPdi[last]];
                }
                else
                {
                    break;
                }
            }

            sequences.Add([.. sequence]);
        }

        return sequences;
    }

    /// <summary>Applies W1–W7, N0–N2 and I1–I2 to one isolating run sequence.</summary>
    private static void ResolveSequence(
        ReadOnlySpan<char> text,
        int[] sequence,
        BidiClass[] initial,
        BidiClass[] classes,
        int[] levels,
        (byte Level, BidiClass Sos, BidiClass Eos) boundary)
    {
        int length = sequence.Length;
        BidiClass[] types = new BidiClass[length];
        for (int i = 0; i < length; i++) types[i] = classes[sequence[i]];

        (byte level, BidiClass sos, BidiClass eos) = boundary;

        ResolveWeak(types, sos);
        ResolveBrackets(text, sequence, initial, types, level, sos);
        ResolveNeutral(types, level, sos, eos);

        byte[] resolved = ResolveImplicit(types, level);

        for (int i = 0; i < length; i++)
        {
            classes[sequence[i]] = types[i];
            levels[sequence[i]] = resolved[i];
        }
    }

    /// <summary>
    /// The sos and eos types of a sequence: what it sees beyond each of its ends.
    /// </summary>
    /// <remarks>
    /// The higher of the sequence's level and its neighbour's, turned into a direction. That is what
    /// makes an isolated number see the paragraph rather than the text next to it — and the reason
    /// W2 and N1 give sensible answers at a paragraph's edges rather than falling off them.
    /// </remarks>
    private static (BidiClass Sos, BidiClass Eos) Boundaries(
        int[] sequence, BidiClass[] initial, int[] levels, byte level, byte paragraphLevel)
    {
        int before = sequence[0] - 1;
        while (before >= 0 && RemovedByX9(initial[before])) before--;
        int levelBefore = before >= 0 ? levels[before] : paragraphLevel;

        int last = sequence[^1];
        int levelAfter;
        if (initial[last] is BidiClass.LRI or BidiClass.RLI or BidiClass.FSI)
        {
            // An unmatched isolate initiator ends the sequence, and what follows it belongs to the
            // paragraph rather than to this sequence.
            levelAfter = paragraphLevel;
        }
        else
        {
            int after = last + 1;
            while (after < initial.Length && RemovedByX9(initial[after])) after++;
            levelAfter = after < initial.Length ? levels[after] : paragraphLevel;
        }

        return (DirectionOf(Math.Max(levelBefore, level)), DirectionOf(Math.Max(levelAfter, level)));
    }

    private static BidiClass DirectionOf(int level) => (level & 1) != 0 ? BidiClass.R : BidiClass.L;

    /// <summary>W1 to W7, in order.</summary>
    private static void ResolveWeak(BidiClass[] types, BidiClass sos)
    {
        int length = types.Length;

        // W1: a non-spacing mark takes the type of what it is attached to. After an isolate
        // initiator or a PDI it becomes ON instead, because a mark cannot attach across an isolate.
        BidiClass previous = sos;
        for (int i = 0; i < length; i++)
        {
            if (types[i] == BidiClass.NSM)
            {
                types[i] = previous is BidiClass.LRI or BidiClass.RLI or BidiClass.FSI or BidiClass.PDI
                    ? BidiClass.ON
                    : previous;
            }

            previous = types[i];
        }

        // W2: a European number after an Arabic letter is an Arabic number. This is the rule that
        // makes "1234" inside Arabic prose read right to left with the text and left to right inside
        // itself — and it keys on AL specifically, so the same digits inside Hebrew stay European.
        BidiClass strong = sos;
        for (int i = 0; i < length; i++)
        {
            switch (types[i])
            {
                case BidiClass.L:
                case BidiClass.R:
                case BidiClass.AL:
                    strong = types[i];
                    break;
                case BidiClass.EN when strong == BidiClass.AL:
                    types[i] = BidiClass.AN;
                    break;
            }
        }

        // W3: an Arabic letter is just a right-to-left character from here on.
        for (int i = 0; i < length; i++)
        {
            if (types[i] == BidiClass.AL) types[i] = BidiClass.R;
        }

        // W4: a single separator between two numbers of the same kind joins them. One separator, not
        // several: "1,,2" is not a number, and the rule looks at exactly one character.
        for (int i = 1; i < length - 1; i++)
        {
            if (types[i] is not (BidiClass.ES or BidiClass.CS)) continue;

            if (types[i - 1] == BidiClass.EN && types[i + 1] == BidiClass.EN)
            {
                types[i] = BidiClass.EN;
            }
            else if (types[i] == BidiClass.CS
                     && types[i - 1] == BidiClass.AN && types[i + 1] == BidiClass.AN)
            {
                types[i] = BidiClass.AN;
            }
        }

        // W5: a run of terminators adjacent to a European number joins it, on either side — this is
        // the currency sign in "$1" and the percent sign in "1%".
        for (int i = 0; i < length; i++)
        {
            if (types[i] != BidiClass.ET) continue;

            int end = i;
            while (end < length && types[end] == BidiClass.ET) end++;

            BidiClass adjacent = i > 0 ? types[i - 1] : BidiClass.ON;
            if (adjacent != BidiClass.EN) adjacent = end < length ? types[end] : BidiClass.ON;

            if (adjacent == BidiClass.EN)
            {
                for (int j = i; j < end; j++) types[j] = BidiClass.EN;
            }

            i = end - 1;
        }

        // W6: whatever separators and terminators are left are ordinary neutrals.
        for (int i = 0; i < length; i++)
        {
            if (types[i] is BidiClass.ET or BidiClass.ES or BidiClass.CS) types[i] = BidiClass.ON;
        }

        // W7: a European number after a left-to-right character is itself left to right, so a number
        // in English prose does not become an island at a deeper level.
        strong = sos;
        for (int i = 0; i < length; i++)
        {
            switch (types[i])
            {
                case BidiClass.L:
                case BidiClass.R:
                    strong = types[i];
                    break;
                case BidiClass.EN when strong == BidiClass.L:
                    types[i] = BidiClass.L;
                    break;
            }
        }
    }

    /// <summary>
    /// BD16 and N0: the paired brackets, and what they resolve to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Not an optional refinement. Without it, "‏هذا (abc) هذا" puts the parentheses the wrong way
    /// round — the opening one at the right of the Latin text and the closing one at its left — which
    /// is the single most noticeable bidi bug a reader will find in a real document, because
    /// parenthesised Latin inside right-to-left prose is everywhere in technical writing.
    /// </para>
    /// <para>
    /// The pair stack is capped at 63 entries as BD16 requires, and overflowing it abandons the rule
    /// for the rest of the sequence rather than resolving the pairs found so far differently.
    /// </para>
    /// </remarks>
    private static void ResolveBrackets(
        ReadOnlySpan<char> text,
        int[] sequence,
        BidiClass[] initial,
        BidiClass[] types,
        byte level,
        BidiClass sos)
    {
        const int MaxPairingDepth = 63;

        List<(int Bracket, int At)> stack = [];
        List<(int Open, int Close)> pairs = [];

        for (int i = 0; i < sequence.Length; i++)
        {
            // Only a character that is still ON can be a bracket: one an override already forced to a
            // direction is no longer neutral, so there is nothing for N0 to decide.
            if (types[i] != BidiClass.ON) continue;

            int codePoint = text[sequence[i]];
            switch (BidiProperties.BracketTypeOf(codePoint))
            {
                case BracketType.Open:
                    if (stack.Count == MaxPairingDepth) return;
                    stack.Add((BidiProperties.Canonical(BidiProperties.PairedBracket(codePoint)), i));
                    break;

                case BracketType.Close:
                    int wanted = BidiProperties.Canonical(codePoint);
                    for (int s = stack.Count - 1; s >= 0; s--)
                    {
                        if (stack[s].Bracket != wanted) continue;

                        pairs.Add((stack[s].At, i));
                        stack.RemoveRange(s, stack.Count - s);
                        break;
                    }
                    break;
            }
        }

        pairs.Sort((a, b) => a.Open.CompareTo(b.Open));
        BidiClass embedding = DirectionOf(level);
        BidiClass opposite = embedding == BidiClass.L ? BidiClass.R : BidiClass.L;

        foreach ((int open, int close) in pairs)
        {
            bool hasEmbedding = false;
            bool hasOpposite = false;

            for (int i = open + 1; i < close; i++)
            {
                BidiClass strong = StrongDirection(types[i]);
                if (strong == embedding) hasEmbedding = true;
                else if (strong == opposite) hasOpposite = true;
            }

            if (!hasEmbedding && !hasOpposite) continue;

            BidiClass chosen;
            if (hasEmbedding)
            {
                // N0 b: a strong type matching the embedding direction inside the brackets settles it.
                chosen = embedding;
            }
            else
            {
                // N0 c: only the opposite direction inside. Then the text *before* the opening bracket
                // decides — brackets follow the direction they were opened in when their contents
                // disagree with the paragraph.
                BidiClass context = sos;
                for (int i = open - 1; i >= 0; i--)
                {
                    BidiClass strong = StrongDirection(types[i]);
                    if (strong == BidiClass.ON) continue;
                    context = strong;
                    break;
                }

                chosen = context == opposite ? opposite : embedding;
            }

            types[open] = chosen;
            types[close] = chosen;

            // A combining mark on a bracket follows it. The mark's type was replaced by W1 long ago,
            // so its original class is what has to be consulted.
            FollowBracket(sequence, initial, types, open, chosen);
            FollowBracket(sequence, initial, types, close, chosen);
        }
    }

    /// <summary>The marks that trailed a bracket take the direction the bracket resolved to.</summary>
    private static void FollowBracket(
        int[] sequence, BidiClass[] initial, BidiClass[] types, int at, BidiClass chosen)
    {
        for (int i = at + 1; i < sequence.Length && initial[sequence[i]] == BidiClass.NSM; i++)
        {
            types[i] = chosen;
        }
    }

    /// <summary>
    /// Which direction a resolved type counts as for the neutral rules.
    /// </summary>
    /// <remarks>
    /// Numbers count as right to left. That is what makes "abc 123 ‏עברית" put its space between the
    /// number and the Hebrew at the Hebrew's level rather than the paragraph's — and it is easy to
    /// forget, because a number is not a right-to-left character in any other sense.
    /// </remarks>
    private static BidiClass StrongDirection(BidiClass type) => type switch
    {
        BidiClass.L => BidiClass.L,
        BidiClass.R or BidiClass.EN or BidiClass.AN => BidiClass.R,
        _ => BidiClass.ON,
    };

    /// <summary>N1 and N2: what the neutrals between two directions become.</summary>
    private static void ResolveNeutral(BidiClass[] types, byte level, BidiClass sos, BidiClass eos)
    {
        BidiClass embedding = DirectionOf(level);

        for (int i = 0; i < types.Length; i++)
        {
            if (!IsNeutralOrIsolate(types[i])) continue;

            int end = i;
            while (end < types.Length && IsNeutralOrIsolate(types[end])) end++;

            BidiClass before = i > 0 ? StrongDirection(types[i - 1]) : sos;
            BidiClass after = end < types.Length ? StrongDirection(types[end]) : eos;

            // N1 when the two sides agree, N2 — the embedding direction — when they do not.
            BidiClass resolved = before == after && before != BidiClass.ON ? before : embedding;
            for (int j = i; j < end; j++) types[j] = resolved;

            i = end - 1;
        }
    }

    /// <summary>The NI set of BD13: the neutrals plus the isolate formatting characters.</summary>
    private static bool IsNeutralOrIsolate(BidiClass type) => type
        is BidiClass.B or BidiClass.S or BidiClass.WS or BidiClass.ON
        or BidiClass.FSI or BidiClass.LRI or BidiClass.RLI or BidiClass.PDI;

    /// <summary>I1 and I2: the level each character ends up at.</summary>
    private static byte[] ResolveImplicit(BidiClass[] types, byte level)
    {
        byte[] levels = new byte[types.Length];
        Array.Fill(levels, level);

        for (int i = 0; i < types.Length; i++)
        {
            if ((level & 1) == 0)
            {
                // I1: inside left-to-right text, right-to-left goes one deeper and a number two, so a
                // number inside a right-to-left phrase reads left to right within itself.
                if (types[i] == BidiClass.R) levels[i] = (byte)(level + 1);
                else if (types[i] is BidiClass.AN or BidiClass.EN) levels[i] = (byte)(level + 2);
            }
            else if (types[i] is BidiClass.L or BidiClass.AN or BidiClass.EN)
            {
                // I2: inside right-to-left text, both left-to-right text and numbers go one deeper.
                levels[i] = (byte)(level + 1);
            }
        }

        return levels;
    }

    /// <summary>
    /// Gives the X9-removed characters a level, then applies L1.
    /// </summary>
    /// <remarks>
    /// A removed character takes the level of the character that <em>follows</em> it. UAX #9 leaves
    /// this open — the characters were removed, so their levels mean nothing to the algorithm — and
    /// the reference implementation propagates forward instead. Backwards is what ICU does, and ICU
    /// is what LibreOffice asks: for "a RLE b PDF c" it reports levels 0 2 2 0 0, giving the RLE the
    /// level of the text it opens and the PDF the level of the text it returns to. Propagating
    /// forward would report 0 0 2 2 0 and put the run boundaries one character out from Writer's.
    /// L1 then resets tabs, paragraph separators and the whitespace before them — and at the end of
    /// the paragraph — to the paragraph's own level, which is what puts the trailing space of a
    /// right-to-left line on the side the reader expects.
    /// </remarks>
    private static byte[] FinaliseLevels(BidiClass[] initial, int[] levels, byte paragraphLevel)
    {
        int[] filled = [.. levels];

        for (int i = 0; i < initial.Length; i++)
        {
            if (RemovedByX9(initial[i])) filled[i] = Unassigned;
        }

        if (filled.Length > 0 && filled[^1] == Unassigned) filled[^1] = paragraphLevel;
        for (int i = filled.Length - 2; i >= 0; i--)
        {
            if (filled[i] == Unassigned) filled[i] = filled[i + 1];
        }

        byte[] result = new byte[filled.Length];
        for (int i = 0; i < filled.Length; i++) result[i] = (byte)filled[i];

        for (int i = 0; i < initial.Length; i++)
        {
            if (initial[i] is not (BidiClass.B or BidiClass.S)) continue;

            result[i] = paragraphLevel;
            for (int j = i - 1; j >= 0 && IsResettable(initial[j]); j--) result[j] = paragraphLevel;
        }

        for (int j = initial.Length - 1; j >= 0 && IsResettable(initial[j]); j--)
        {
            result[j] = paragraphLevel;
        }

        return result;
    }

    /// <summary>The characters L1 clause 4 resets: whitespace, and the invisible formatting.</summary>
    private static bool IsResettable(BidiClass type) => type
        is BidiClass.WS or BidiClass.FSI or BidiClass.LRI or BidiClass.RLI or BidiClass.PDI
        or BidiClass.LRE or BidiClass.RLE or BidiClass.LRO or BidiClass.RLO
        or BidiClass.PDF or BidiClass.BN;

    /// <summary>The maximal stretches at one level, in logical order.</summary>
    private static BidiRun[] RunsOf(byte[] levels)
    {
        if (levels.Length == 0) return [];

        List<BidiRun> runs = [];
        int start = 0;

        for (int i = 1; i <= levels.Length; i++)
        {
            if (i < levels.Length && levels[i] == levels[start]) continue;

            runs.Add(new BidiRun(start, i - start, levels[start]));
            start = i;
        }

        return [.. runs];
    }

    /// <summary>One frame of X1's directional status stack.</summary>
    private readonly record struct StatusEntry(int Level, BidiClass Override, bool IsIsolate);
}

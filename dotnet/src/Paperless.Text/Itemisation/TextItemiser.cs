namespace Paperless.Text.Itemisation;

/// <summary>
/// One sub-run of a paragraph: a stretch with a single direction and a single script.
/// </summary>
/// <remarks>
/// This is the unit a shaper is handed. HarfBuzz needs a direction and a script per call and gives
/// different results for a run tagged with the wrong one, so a paragraph has to be cut into these
/// before anything is measured.
/// </remarks>
/// <param name="Start">Its first character, as an index into the paragraph's text.</param>
/// <param name="Length">How many UTF-16 code units it covers.</param>
/// <param name="Level">Its bidi embedding level; odd means right to left.</param>
/// <param name="Script">Its ISO 15924 code.</param>
public readonly record struct TextItem(int Start, int Length, byte Level, string Script)
{
    /// <summary>One past the item's last character.</summary>
    public int End => Start + Length;

    /// <summary>True when the item is shaped right to left.</summary>
    public bool IsRightToLeft => (Level & 1) != 0;
}

/// <summary>
/// Cuts a paragraph into the sub-runs a shaper can take: one direction, one script each.
/// </summary>
/// <remarks>
/// <para>
/// The bidi runs first, then the script runs inside them, which is the order
/// <c>GenericSalLayout::LayoutText</c> works in: it walks the runs
/// <c>ImplLayoutArgs</c> resolved with ICU, and subdivides each against the script runs from
/// <c>vcl::text::TextLayoutCache</c> (<c>vcl/source/gdi/CommonSalLayout.cxx</c>). Doing it the other
/// way round would give the same partition — both are partitions of the same text — but it would put
/// the boundaries in a different order and make a comparison against LibreOffice's own portions
/// harder to read.
/// </para>
/// <para>
/// Format control characters are cut out rather than shaped, which is
/// <c>ImplLayoutArgs::AddRun</c>'s splitting on <c>IsControlChar</c>. A left-to-right mark handed to a
/// shaper comes back as a missing-glyph box with a real advance, which is both visible and wide; cut
/// out, it takes no room at all, which is what it means.
/// </para>
/// <para>
/// <strong>This list is deliberately narrower than <see cref="Shaping.ShapingControls"/>'s</strong>,
/// which is what a shaper is actually handed. Cutting here removes a character from every sub-run, so a
/// paragraph whose whole text is one control character produces no run at all — and a paragraph with no
/// run has no line for a frame anchored in it to be measured against. The commonest such paragraph in
/// the corpus is a logo: a footer paragraph holding nothing but a <c>w:drawing</c>, whose text is the
/// single U+0001 that stands for it. Measured on
/// <c>1603642410-MoM-CASCOM-06-2020-draft04.docx</c>, whose nine footers each lost the six words in
/// that shape. So the C0 range stays in the items and is dropped one layer down, where removing it
/// costs the glyph and not the structure.
/// </para>
/// </remarks>
public static class TextItemiser
{
    /// <summary>Cuts a paragraph into sub-runs, resolving the bidi algorithm over it first.</summary>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="baseDirection">The direction the paragraph is declared to have.</param>
    public static List<TextItem> Itemise(
        ReadOnlySpan<char> text, BidiDirection baseDirection = BidiDirection.LeftToRight)
        => Itemise(text, BidiParagraph.Resolve(text, baseDirection));

    /// <summary>Cuts a paragraph into sub-runs against a bidi resolution already done.</summary>
    /// <remarks>
    /// Separate because the resolution is per paragraph and the itemisation is per paragraph too, but
    /// a caller that already has the levels — to align a line, say, or to place a caret — should not
    /// pay for them twice.
    /// </remarks>
    public static List<TextItem> Itemise(ReadOnlySpan<char> text, BidiParagraph bidi)
    {
        ArgumentNullException.ThrowIfNull(bidi);

        List<TextItem> items = [];
        if (text.Length == 0) return items;

        List<ScriptRun> scripts = ScriptItemiser.Itemise(text);
        int scriptAt = 0;

        foreach (BidiRun run in bidi.Runs)
        {
            while (scriptAt < scripts.Count && scripts[scriptAt].End <= run.Start) scriptAt++;

            for (int at = scriptAt; at < scripts.Count && scripts[at].Start < run.End; at++)
            {
                int start = Math.Max(scripts[at].Start, run.Start);
                int end = Math.Min(scripts[at].End, run.End);
                if (end <= start) continue;

                AddWithoutControls(text, items, start, end, run.Level, scripts[at].Script);
            }
        }

        return items;
    }

    /// <summary>
    /// The items in the order they are drawn left to right, which is rule L2.
    /// </summary>
    /// <remarks>
    /// The reordering runs over the bidi levels, so a script boundary inside one level run does not
    /// move anything: two Latin and Greek items at level 1 stay in the order the level run puts them.
    /// That is why this reverses stretches of items at or above each level rather than sorting them.
    /// </remarks>
    public static List<TextItem> InVisualOrder(IReadOnlyList<TextItem> items)
    {
        ArgumentNullException.ThrowIfNull(items);

        List<TextItem> order = [.. items];
        ReorderVisually(order, item => item.Level);
        return order;
    }

    /// <summary>
    /// Puts anything carrying bidi levels into the order it is drawn left to right, in place.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Rule L2 itself: from the highest level down to the lowest odd one, reverse every contiguous
    /// stretch at or above that level. Generic in what it is reordering because the same rule has
    /// to run over two different things — the itemiser's sub-runs, and the glyph runs a line
    /// actually draws, which are those sub-runs cut again at every change of font, size or colour.
    /// Two implementations of L2 would be two chances to disagree about a line that reorders.
    /// </para>
    /// <para>
    /// The parts must be in logical order and must cover a contiguous stretch of one line, which
    /// is what makes reversing a range the right operation: L2 is defined per line, over the
    /// levels of the characters on it.
    /// </para>
    /// </remarks>
    /// <param name="parts">The parts, in logical order; reordered in place.</param>
    /// <param name="levelOf">Each part's embedding level.</param>
    public static void ReorderVisually<T>(IList<T> parts, Func<T, byte> levelOf)
    {
        ArgumentNullException.ThrowIfNull(parts);
        ArgumentNullException.ThrowIfNull(levelOf);

        if (parts.Count < 2) return;

        byte highest = 0;
        byte lowestOdd = byte.MaxValue;
        foreach (T part in parts)
        {
            byte level = levelOf(part);
            if (level > highest) highest = level;
            if ((level & 1) != 0 && level < lowestOdd) lowestOdd = level;
        }

        for (int level = highest; level >= lowestOdd; level--)
        {
            for (int i = 0; i < parts.Count; i++)
            {
                if (levelOf(parts[i]) < level) continue;

                int limit = i + 1;
                while (limit < parts.Count && levelOf(parts[limit]) >= level) limit++;
                Reverse(parts, i, limit - 1);
                i = limit;
            }
        }
    }

    /// <summary>Reverses a range of a list in place, which <see cref="List{T}"/> alone can do.</summary>
    private static void Reverse<T>(IList<T> parts, int from, int to)
    {
        while (from < to)
        {
            (parts[from], parts[to]) = (parts[to], parts[from]);
            from++;
            to--;
        }
    }

    /// <summary>
    /// True when a paragraph could hold more than one direction, decided without resolving it.
    /// </summary>
    /// <remarks>
    /// The guard that keeps the common case free. A left-to-right paragraph with nothing
    /// right-to-left in it resolves flat, itemises into one sub-run per formatting run and draws
    /// exactly as it did before any of this existed — so the whole apparatus is worth skipping
    /// rather than worth running and finding it had no effect. Conservative in the direction that
    /// matters: it says yes for anything that <em>might</em> reorder, including the embedding and
    /// isolate controls, and only says no when every character is left to right or neutral.
    /// </remarks>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="baseDirection">The direction the paragraph is declared to have.</param>
    public static bool MayReorder(
        ReadOnlySpan<char> text, BidiDirection baseDirection = BidiDirection.LeftToRight)
    {
        if (baseDirection != BidiDirection.LeftToRight) return true;

        for (int i = 0; i < text.Length; i++)
        {
            int codePoint = char.IsHighSurrogate(text[i]) && i + 1 < text.Length
                            && char.IsLowSurrogate(text[i + 1])
                ? char.ConvertToUtf32(text[i], text[i + 1])
                : text[i];

            if (BidiProperties.ClassOf(codePoint)
                is BidiClass.R or BidiClass.AL or BidiClass.AN
                or BidiClass.RLE or BidiClass.RLO or BidiClass.RLI
                or BidiClass.LRE or BidiClass.LRO or BidiClass.LRI or BidiClass.FSI)
            {
                return true;
            }
        }

        return false;
    }

    /// <summary>Adds a range as items, leaving the format control characters out of them.</summary>
    private static void AddWithoutControls(
        ReadOnlySpan<char> text, List<TextItem> items, int start, int end, byte level, string script)
    {
        int at = start;

        for (int i = start; i < end; i++)
        {
            if (!IsFormatControl(text[i])) continue;

            if (i > at) items.Add(new TextItem(at, i - at, level, script));
            at = i + 1;
        }

        if (end > at) items.Add(new TextItem(at, end - at, level, script));
    }

    /// <summary>
    /// True for a character that must not appear in a sub-run's range at all.
    /// </summary>
    /// <remarks>
    /// <see cref="Shaping.ShapingControls"/> minus the C0 range, for the reason on the class: a cut here
    /// removes the character from every run, and a paragraph that is nothing but control characters then
    /// has no run and no line. The C0 range is dropped inside the shaper instead, which costs it its
    /// glyph and its advance and leaves the run it sits in intact.
    /// </remarks>
    private static bool IsFormatControl(char character) => character
        is '\u0000'
        or >= '\u200E' and <= '\u200F'
        or >= '\u2028' and <= '\u202E'
        or '\u2060'
        or >= '\u206A' and <= '\u206F'
        or '\uFEFF' or '\uFFFE' or '\uFFFF';
}

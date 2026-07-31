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
/// out, it takes no room at all, which is what it means. The one departure from LibreOffice's list is
/// the C0 range: it removes U+0001 to U+001F, and Paperless keeps them, because the tab is in that
/// range and the tab's width is resolved by the line filler rather than by the shaper.
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
        if (order.Count < 2) return order;

        byte highest = 0;
        byte lowestOdd = byte.MaxValue;
        foreach (TextItem item in order)
        {
            if (item.Level > highest) highest = item.Level;
            if ((item.Level & 1) != 0 && item.Level < lowestOdd) lowestOdd = item.Level;
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
    /// True for a character that must not reach a shaper.
    /// </summary>
    /// <remarks>
    /// LibreOffice's <c>IsControlChar</c> (<c>vcl/source/text/ImplLayoutArgs.cxx</c>) minus its C0
    /// range: the directional marks and the embedding and override controls, the invisible operators
    /// and the deprecated format characters, the byte-order mark, the two permanent non-characters,
    /// and NUL. Every one of them has no width and no glyph, and a shaper handed one draws a box.
    /// </remarks>
    private static bool IsFormatControl(char character) => character
        is '\u0000'
        or >= '\u200E' and <= '\u200F'
        or >= '\u2028' and <= '\u202E'
        or '\u2060'
        or >= '\u206A' and <= '\u206F'
        or '\uFEFF' or '\uFFFE' or '\uFFFF';
}

using Paperless.Core.Units;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// The case a run's text is <em>drawn</em> in, which every word-processing format states separately from
/// the case it is stored in.
/// </summary>
/// <remarks>
/// <c>SvxCaseMap</c> (<c>include/editeng/svxfont.hxx</c>), minus the two members no word-processing format
/// writes for a run — lowercase and title case exist in the item but neither OOXML nor WW8 nor RTF has a
/// character property that produces them.
/// </remarks>
public enum PageCaseMap : byte
{
    /// <summary>Drawn as stored.</summary>
    None = 0,

    /// <summary>Drawn in capitals: <c>w:caps</c>, <c>sprmCFCaps</c>, <c>\caps</c>.</summary>
    Uppercase = 1,

    /// <summary>
    /// Drawn in capitals, with what was lowercase drawn smaller: <c>w:smallCaps</c>, <c>sprmCFSmallCaps</c>,
    /// <c>\scaps</c>.
    /// </summary>
    SmallCaps = 2,
}

/// <summary>
/// Resolves <see cref="PageCaseMap"/> away: rewrites a paragraph's text into the case it is drawn in, and
/// splits the runs a small-capitals map needs split.
/// </summary>
/// <remarks>
/// <para>
/// Done in the readers rather than in the layout engine because the engine measures and draws from
/// <see cref="PageParagraph.Text"/>, and a case map changes what the glyphs are — so a run left mapped
/// would be measured in one case and drawn in another. Doing it once here also keeps the four readers from
/// each growing their own copy.
/// </para>
/// <para>
/// <strong>The mapping is per character and never changes a length.</strong> Uppercasing is a string
/// operation in every other context, and here it must not be: a paragraph's text is indexed by its
/// bookmarks, its note anchors, its frame anchors and its runs, so the German <c>ß</c> becoming
/// <c>SS</c> would shift every offset recorded after it. <see cref="char.ToUpperInvariant(char)"/> is the
/// per-character map, which leaves <c>ß</c> alone — as LibreOffice's own <c>SvxFont::CalcCaseMap</c> notes
/// it must when the transliteration would change the length
/// (<c>editeng/source/items/svxfont.cxx</c>, <c>bCaseMapLengthDiffers</c>).
/// </para>
/// <para>
/// Extraction is deliberately untouched. <c>IDocument</c> reports what the document says, and LibreOffice's
/// own text filters do the same; only the page shows the mapped case.
/// </para>
/// </remarks>
public static class CaseMapping
{
    /// <summary>
    /// The proportion a small-capital is drawn at, as LibreOffice 24.2 has it.
    /// </summary>
    /// <remarks>
    /// <c>SMALL_CAPS_PERCENTAGE</c>, 80 (<c>include/editeng/svxfont.hxx:35</c>). Older versions used 66 and
    /// the value survives as a per-document compatibility flag, but only ODF carries it — a DOCX, a DOC and
    /// an RTF all get 80.
    /// </remarks>
    public const double SmallCapitalProportion = 0.8;

    /// <summary>
    /// Applies the case maps the runs carry, returning the text as it is drawn.
    /// </summary>
    /// <remarks>
    /// The run list is rewritten in place, because a small-capitals run becomes several: one per stretch
    /// of the text that was lowercase and one per stretch that was not, since only the first is drawn
    /// smaller. A run whose map is <see cref="PageCaseMap.None"/> is left exactly as it was, so a paragraph
    /// with no case map at all — nearly all of them — costs one scan and no allocation.
    /// </remarks>
    /// <param name="text">The paragraph's text as the document stores it.</param>
    /// <param name="runs">
    /// Its runs, partitioning that text. Rewritten in place when any of them maps case; left untouched
    /// otherwise.
    /// </param>
    /// <returns>The text as it is drawn, always the same length as <paramref name="text"/>.</returns>
    public static string Apply(string text, List<PageRun> runs)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(runs);

        bool maps = false;
        foreach (PageRun run in runs)
        {
            if (run.CaseMap != PageCaseMap.None) { maps = true; break; }
        }

        if (!maps || text.Length == 0) return text;

        char[] mapped = text.ToCharArray();
        List<PageRun> rewritten = new(runs.Count);

        foreach (PageRun run in runs)
        {
            int start = Math.Clamp(run.Start, 0, text.Length);
            int end = Math.Clamp(run.End, start, text.Length);

            if (run.CaseMap == PageCaseMap.None || end == start)
            {
                rewritten.Add(run);
                continue;
            }

            for (int i = start; i < end; i++) mapped[i] = char.ToUpperInvariant(mapped[i]);

            if (run.CaseMap == PageCaseMap.Uppercase)
            {
                rewritten.Add(run with { CaseMap = PageCaseMap.None });
                continue;
            }

            // Small capitals: the characters the mapping actually changed are the ones that were
            // lowercase, and they are the ones drawn smaller. Every other character — a capital already, a
            // digit, a space, a full stop — stays at the run's own size, which is why one run becomes an
            // alternating sequence rather than a uniformly smaller one.
            Length small = Length.FromEmu((long)Math.Round(run.EmSize.Emu * SmallCapitalProportion));

            int segment = start;
            bool shrinks = mapped[start] != text[start];

            for (int i = start + 1; i <= end; i++)
            {
                bool here = i < end && mapped[i] != text[i];
                if (i < end && here == shrinks) continue;

                rewritten.Add(run with
                {
                    Start = segment,
                    Length = i - segment,
                    EmSize = shrinks ? small : run.EmSize,
                    MetricEmSize = shrinks ? run.EmSize : default,
                    CaseMap = PageCaseMap.None,
                });

                segment = i;
                shrinks = here;
            }
        }

        runs.Clear();
        runs.AddRange(rewritten);
        return new string(mapped);
    }
}

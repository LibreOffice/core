using Paperless.Core.Graphics;
using Paperless.Text.Fonts;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// Resolves a legacy symbol face away: rewrites the characters a run set in one is <em>drawn</em> as,
/// and moves the run onto the face that can show them.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A symbol face addresses glyphs by byte position, not by character.</strong> A document states
/// a Wingdings tick as <c>0xFC</c> and means "the glyph in slot 0xFC", not <c>ü</c>. On a machine that
/// has the face the slot is drawn from it and nothing needs doing; on one that does not — and Wingdings,
/// Webdings, Symbol and Monotype Sorts are not fonts Linux has — LibreOffice substitutes OpenSymbol and
/// recodes every character through <see cref="SymbolFontRecode"/>'s tables.
/// </para>
/// <para>
/// <strong>This is not a bullet rule.</strong> LibreOffice hangs the conversion on the *font instance*
/// (<c>ImplFontCache::GetFontInstance</c>, <c>vcl/source/font/fontcache.cxx:167-171</c>) and applies it
/// to every string that instance ever draws, in <c>OutputDevice::ImplLayout</c>
/// (<c>vcl/source/outdev/text.cxx:1157-1161</c>) — which is also the path every *measurement* takes, so
/// the recode decides line breaking as well as ink. A list label is one case of it and, measured on this
/// corpus, the smaller one: of 96 words documents where the reference embedded OpenSymbol and we
/// embedded none, 35 were still gapped after the label case was wired, and 20 of those 25 DOCX carry no
/// <c>w:sym</c> at all — they are ordinary runs whose <c>w:rFonts</c> names a symbol face.
/// </para>
/// <para>
/// Done in the readers rather than in the layout engine for exactly the reason
/// <see cref="CaseMapping"/> is: the engine measures and draws from
/// <see cref="PageParagraph.Text"/>, so a run left unrecoded would be measured as one thing and drawn as
/// another. The mapping is per character and never changes a length, which it must not be — a
/// paragraph's text is indexed by its bookmarks, its note anchors, its frame anchors and its runs.
/// </para>
/// <para>
/// <strong>Extraction is deliberately untouched.</strong> LibreOffice draws the OpenSymbol glyph and
/// writes the *original* code point into the PDF's <c>ToUnicode</c>, so its text layer still reads the
/// slot the document stated. Both that and ours mean nothing outside one font, so this changes what is
/// drawn and leaves what a caller extracts alone.
/// </para>
/// </remarks>
public static class SymbolRecoding
{
    /// <summary>
    /// Resolves a face by family name for the recode's own request.
    /// </summary>
    /// <remarks>
    /// Returns null when the family cannot be loaded at all, which leaves the run exactly as it was.
    /// </remarks>
    public delegate (OpenTypeFace Face, FontReference? Font)? FaceResolver(string family);

    /// <summary>
    /// Recodes every run set in a substituted symbol face, returning the text as it is drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The gate is LibreOffice's own, and it is a gate on the <em>substitution</em> rather than on the
    /// name: <c>ImplFontCache</c> attaches a conversion only when the face it found is not the face that
    /// was asked for. A machine that really has Wingdings draws the slots from Wingdings, and recoding
    /// them there would pick the wrong glyph.
    /// </para>
    /// <para>
    /// A run whose recode cannot be completed is left alone rather than half-done: the code points
    /// mean nothing in any face but OpenSymbol, so a run carrying them in something else would draw
    /// <c>.notdef</c> — worse than the wrong-but-present glyph it replaced.
    /// </para>
    /// </remarks>
    /// <param name="text">The paragraph's text, after any case mapping.</param>
    /// <param name="runs">
    /// Its runs, partitioning that text. Rewritten in place when any of them recodes; left untouched
    /// otherwise, so a paragraph with no symbol run — nearly all of them — costs one scan and no
    /// allocation.
    /// </param>
    /// <param name="resolve">How to load a face by family name.</param>
    /// <returns>The text as it is drawn, always the same length as <paramref name="text"/>.</returns>
    public static string Apply(string text, List<PageRun> runs, FaceResolver resolve)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(runs);
        ArgumentNullException.ThrowIfNull(resolve);

        if (text.Length == 0 || runs.Count == 0) return text;

        bool any = false;
        foreach (PageRun run in runs)
        {
            if (Recodes(run.Font)) { any = true; break; }
        }

        if (!any) return text;

        if (resolve(SymbolFontRecode.SubstituteFamily) is not (OpenTypeFace symbol, { } reference)
            || !SymbolFontRecode.IsSubstituteFamily(reference.FamilyName))
        {
            return text;
        }

        char[] recoded = text.ToCharArray();

        for (int index = 0; index < runs.Count; index++)
        {
            PageRun run = runs[index];
            if (!Recodes(run.Font)) continue;

            string requested = run.Font!.RequestedFamily!;
            int start = Math.Clamp(run.Start, 0, text.Length);
            int end = Math.Clamp(run.End, start, text.Length);

            for (int at = start; at < end; at++)
            {
                if (SymbolFontRecode.TryRecode(requested, recoded[at], out char mapped))
                {
                    recoded[at] = mapped;
                }
            }

            runs[index] = run with { Face = symbol, Font = reference };
        }

        return new string(recoded);
    }

    /// <summary>
    /// Whether a resolved reference names a symbol face that was asked for and not found.
    /// </summary>
    /// <remarks>
    /// Both halves matter. <see cref="FontReference.IsSubstituted"/> alone fires on every unavailable
    /// text face, and <see cref="SymbolFontRecode.IsRecodeable"/> alone fires on a machine that has the
    /// face installed — where the slots are drawable as they stand and recoding them would be wrong.
    /// </remarks>
    private static bool Recodes(FontReference? font)
        => font is { IsSubstituted: true }
           && SymbolFontRecode.IsRecodeable(font.RequestedFamily)
           && !SymbolFontRecode.IsSubstituteFamily(font.RequestedFamily);
}

using XRay.Office.Core.Graphics;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Presentations.Layout;

/// <summary>
/// Resolves the symbol face a run asks for — DrawingML's <c>a:rPr/a:sym</c> — into the face and
/// the code points that are actually drawn.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A symbol run is a face switch that lasts for the private-use characters and no
/// longer.</strong> <c>oox/source/drawingml/textrun.cxx:96-135</c> walks a run's text splitting
/// it into maximal stretches by the predicate <c>(ch &amp; 0xff00) == 0xf000</c>, sets
/// <c>CharFontName</c>/<c>CharFontCharSet</c> from <c>a:sym</c> over each private-use stretch,
/// and resets the four properties to the run's own values after every one. So a run reading
/// <c>"Contact us &#xF0E0; today"</c> is three runs to the layout, of which only the middle one
/// is set in the symbol face. Measured over the slides track: <b>45 of the affected
/// <c>a:t</c> values hold both kinds of character</b>, so the split is the common case rather
/// than the corner.
/// </para>
/// <para>
/// <strong>This is a normalisation over the paragraph, not a decision the reader could have
/// made.</strong> The reader cannot know whether Wingdings is installed, and that is exactly
/// what decides whether the slot is drawn as it stands or recoded — the distinction
/// <see cref="SymbolFontRecode"/>'s remarks record having been got wrong once already. It also
/// cannot be done in the painter: <c>SlideTextLayout.EmitStretch</c> takes a run's characters
/// from the paragraph's shared string, so a per-run substitution has to reach the string. Doing
/// it here, where <see cref="SlideFonts"/> is in hand and before anything is measured, keeps the
/// recode in front of line breaking — an OpenSymbol arrow and a <c>.notdef</c> box are not the
/// same width.
/// </para>
/// <para>
/// <strong>Every offset survives, which is what makes it a rewrite rather than a rebuild.</strong>
/// A recode is one code point for one code point (<c>ConvertChar::RecodeChar</c>), so the
/// paragraph's text keeps its length and a run only ever splits at a boundary inside its own
/// range. Nothing downstream — the marker's reach, the measured runs, the colour and decoration
/// lookups, the tab ruler — sees an index it did not see before.
/// </para>
/// </remarks>
internal static class SlideSymbolRuns
{
    /// <summary>
    /// Whether a symbol face's slots have to be recoded rather than drawn where the file put them.
    /// </summary>
    /// <remarks>
    /// The three-part rule shared by the bullet path (<c>SlideTextLayout.Recoded</c>) and the run
    /// path, so the two cannot drift: the face must be one of the fourteen LibreOffice holds a
    /// table for, and its own file must be absent. When the file is present the slot is drawn
    /// from it unchanged, which is <c>ConvertChar::GetRecodeData</c> only supplying a recode once
    /// the substitution has landed on StarSymbol or OpenSymbol
    /// (<c>unotools/source/misc/fontcvt.cxx:1345-1356</c>).
    /// </remarks>
    /// <param name="typeface">The face the file named for the symbol.</param>
    /// <param name="reference">What resolving that face gave, or null when nothing was resolved.</param>
    internal static bool Recodes(string? typeface, FontReference? reference)
        => SymbolFontRecode.IsRecodeable(typeface)
           && (reference is null
               || reference.IsSubstituted
               || SymbolFontRecode.IsSubstituteFamily(reference.FamilyName));

    /// <summary>
    /// The same decision for a run's <c>a:sym</c>, which reaches it down one of two paths.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A request that is not symbol-encoded has to actually land on OpenSymbol.</strong>
    /// <see cref="Recodes(string?, FontReference?)"/> accepts any substitution, which is right
    /// when fontconfig has been bypassed — <c>FcPreMatchSubstitution::FindFontSubstitute</c>
    /// refuses a symbol-encoded request outright (<c>fontsubst.cxx:100-104</c>), leaving
    /// <c>VCL.xcu</c>'s chain, which ends at OpenSymbol for every face there is a table for. It is
    /// wrong when fontconfig <em>has</em> answered, because fontconfig does not know the name
    /// meant a symbol font: it answers <c>Wingdings</c> with DejaVu Sans, and LibreOffice then
    /// draws the slot from DejaVu Sans.
    /// </para>
    /// <para>
    /// Measured, and it moved three glyphs on two documents — see <see cref="SlideSymbolFont"/>
    /// for the coordinates. <c>Symbol</c> passes this arm anyway, because fontconfig binds that
    /// one family to OpenSymbol by name; <c>Wingdings</c> without a <c>charset="2"</c> does not.
    /// See <see cref="SymbolFontRecode.IsAliasedToSubstitute"/> for where the alias comes from.
    /// </para>
    /// <para>
    /// It reads the requested family rather than the resolved one, which is the only reading that
    /// works here: <em>our</em> resolver applies <c>VCL.xcu</c>'s chain unconditionally, so it
    /// answers <c>Wingdings</c> with OpenSymbol whichever path the request took, and asking it
    /// cannot tell the two apart. Teaching the resolver the distinction would move every family
    /// on every track and is a font-layer change; this is the presentation-layer statement of the
    /// same fact, kept where its two callers are.
    /// </para>
    /// </remarks>
    internal static bool Recodes(SlideSymbolFont font, FontReference? reference)
        => font.IsMicrosoftEncoded
            ? Recodes(font.Typeface, reference)
            : SymbolFontRecode.IsRecodeable(font.Typeface)
              && SymbolFontRecode.IsAliasedToSubstitute(font.Typeface);

    /// <summary>
    /// A body whose symbol runs have been resolved, or the body itself when it has none.
    /// </summary>
    /// <remarks>
    /// Returns the same instance when nothing changes, which is the overwhelming majority of
    /// bodies — 13 documents in 163 hold a symbol run at all — so the cost on everything else is
    /// one flag test per run.
    /// </remarks>
    internal static SlideTextBody Normalise(SlideTextBody body, SlideFonts fonts)
    {
        List<SlideParagraph>? replaced = null;

        for (int index = 0; index < body.Paragraphs.Count; index++)
        {
            SlideParagraph paragraph = body.Paragraphs[index];
            if (Normalise(paragraph, fonts) is not { } normalised) continue;

            replaced ??= [.. body.Paragraphs];
            replaced[index] = normalised;
        }

        return replaced is null ? body : body with { Paragraphs = replaced };
    }

    /// <summary>One paragraph's symbol runs resolved, or null when it has none to resolve.</summary>
    private static SlideParagraph? Normalise(SlideParagraph paragraph, SlideFonts fonts)
    {
        bool any = false;
        foreach (SlideTextRun run in paragraph.Runs)
        {
            if (run.SymbolFont is not null && run.Length > 0) { any = true; break; }
        }

        if (!any) return null;

        string text = paragraph.Text;
        char[]? recoded = null;
        List<SlideTextRun> runs = new(paragraph.Runs.Count);
        bool split = false;

        foreach (SlideTextRun run in paragraph.Runs)
        {
            if (run.SymbolFont is not { } symbol
                || run.Length <= 0
                || run.Start < 0
                || run.End > text.Length)
            {
                runs.Add(run.SymbolFont is null ? run : run with { SymbolFont = null });
                continue;
            }

            // Resolved once for the whole run: the answer depends on the face and the weight,
            // neither of which changes within it.
            (_, FontReference? reference) =
                fonts.Resolve(symbol.Typeface, run.Weight, run.IsItalic);
            bool recodes = Recodes(symbol, reference);

            int position = run.Start;
            while (position < run.End)
            {
                bool isPrivateUse = IsPrivateUse(text[position]);
                int end = position + 1;
                while (end < run.End && IsPrivateUse(text[end]) == isPrivateUse) end++;

                if (position != run.Start || end != run.End) split = true;

                if (!isPrivateUse)
                {
                    runs.Add(run with
                    {
                        Start = position, Length = end - position, SymbolFont = null,
                    });
                    position = end;
                    continue;
                }

                // The face switch itself, and it is narrower than LibreOffice's on purpose.
                //
                // `textrun.cxx` switches unconditionally and lets VCL substitute; on the
                // non-symbol-encoded path VCL asks fontconfig, which answers `Wingdings` with
                // DejaVu Sans. Our resolver has no such path — it applies `VCL.xcu`'s chain
                // whatever the request, so asking it for `Wingdings` returns *OpenSymbol*, and
                // switching to it would draw the private-use slot as .notdef out of a face that
                // does not hold it. Leaving the run in its own face instead sends the slot to
                // glyph fallback, which is where it went before this rule existed and is the
                // nearer of the two to a reference that draws it from DejaVu Sans.
                //
                // So: switch when the recode fires, or when the named face is genuinely installed
                // and its own slots are therefore drawable. Otherwise leave the run alone.
                string typeface = run.Typeface ?? symbol.Typeface;

                if (!recodes && reference is { IsSubstituted: false } resolved)
                {
                    typeface = resolved.FamilyName;
                }

                if (recodes)
                {
                    bool touched = false;
                    for (int at = position; at < end; at++)
                    {
                        if (!SymbolFontRecode.TryRecode(symbol.Typeface, text[at], out char slot))
                        {
                            continue;
                        }

                        recoded ??= text.ToCharArray();
                        recoded[at] = slot;
                        touched = true;
                    }

                    // The recode and the face go together, exactly as the bullet path has it: a
                    // stretch none of whose slots the table covers keeps both, because asking
                    // OpenSymbol for a private-use code point it does not hold is .notdef.
                    if (touched) typeface = SymbolFontRecode.SubstituteFamily;
                }

                runs.Add(run with
                {
                    Start = position,
                    Length = end - position,
                    Typeface = typeface,
                    SymbolFont = null,
                });
                position = end;
            }
        }

        if (recoded is null && !split) return paragraph with { Runs = runs };

        return paragraph with
        {
            Text = recoded is null ? text : new string(recoded),
            Runs = runs,
        };
    }

    /// <summary>
    /// LibreOffice's own test for a symbol slot: the whole <c>U+F000</c> plane page.
    /// </summary>
    /// <remarks>
    /// <c>(getText()[nIndex] &amp; 0xff00) == 0xf000</c>, <c>textrun.cxx:100</c>. Wider than
    /// <see cref="SymbolFontRecode"/>'s own <c>F020</c>–<c>F0FF</c> guard, and deliberately kept
    /// wider: the face switch applies to the whole page and only the recode is restricted, which
    /// is the same asymmetry LibreOffice has between <c>textrun.cxx</c> and <c>RecodeString</c>.
    /// </remarks>
    private static bool IsPrivateUse(char character) => (character & 0xFF00) == 0xF000;
}

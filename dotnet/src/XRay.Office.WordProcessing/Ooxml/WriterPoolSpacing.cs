namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// The vertical margins Writer's own built-in paragraph styles carry before a DOCX has said
/// anything about them.
/// </summary>
/// <remarks>
/// <para>
/// A DOCX style whose <c>w:name</c> is one of Word's built-in names is not created fresh by
/// LibreOffice's importer: it is *found*, because Writer already has a style of that name in its
/// pool with its own spacing, font and outline level. The imported properties are applied on top,
/// and anything the file does not state is whatever the pool style holds.
/// </para>
/// <para>
/// That is normally invisible, because the importer clears the pool style's direct values first.
/// It becomes visible through <see cref="WordStyles.CompleteOneSidedSpacing"/>, where a
/// half-stated <c>w:spacing</c> freezes the other half at whatever the style resolves to at that
/// point in the import — the style's own pool row when Writer has one under its <c>w:name</c>,
/// and otherwise its parent's, a parent whose own definition has not been reached yet still
/// holding exactly these numbers. Which of the two it is was measured separately; see the
/// remarks on <see cref="WordStyles.CompleteOneSidedSpacing"/>.
/// </para>
/// <para>
/// The table is measured rather than read off <c>DocumentStylePoolManager.cxx</c>, because the
/// source's <c>bNoDefault</c> guard says these should not apply at all and they demonstrably do.
/// Each row is one rendered probe against LibreOffice 24.2.7.2: a child style based on a parent of
/// that name, declared before it, stating one of the two margins and reading back the other. The
/// three non-zero groups line up with the pool declarations at
/// <c>sw/source/core/doc/DocumentStylePoolManager.cxx:810</c> (the <c>Heading</c> base, 12 pt and
/// 6 pt), <c>:699</c> (<c>Text body</c>, nought and 7 pt) and <c>:974</c> (<c>Caption</c>, 6 pt
/// and 6 pt), which is the check that the measurement is describing a real rule and not a
/// coincidence.
/// </para>
/// <para>
/// Anything not named here measured as nought above and nought below, which is also the honest
/// default: a name Writer does not recognise becomes a brand-new style with no spacing at all.
/// </para>
/// <para>
/// [24.2.7-audit: VERIFIED 2026-08-21, round words-r56 — the whole table re-measured on 26.2.4.2,
/// both halves of every row, by `probes/words-r56/audit_poolspacing.py`. 27 of the 28 names it
/// tests answer exactly what this class claims, including the three that claim nothing (`Quote`,
/// `Normal`, `List Paragraph`). The one exception is the row the paragraph on `ChildKeeps` had
/// already put in doubt — lower-case `body text` answers nought on both sides, not 0/140 — and it
/// is now removed rather than left standing, which is what that paragraph asked the round that
/// re-measured the table to do. Zero corpus documents name a parent that way and 80 name
/// `Body Text`, so the correction has no reach and is made because it is true.
/// **The probe's own first run reported nine rows wrong and every one was an artefact**: it named
/// the two case variants of a heading `heading-5` and `Heading-5`, which are one file on this
/// mount, and a missing conversion reads as nought which reads as a finding. It now numbers its
/// packages and refuses to print anything unless every conversion produced output.]
/// </para>
/// </remarks>
internal static class WriterPoolSpacing
{
    private const int Pt6 = 120;
    private const int Pt7 = 140;
    private const int Pt12 = 240;

    /// <summary>
    /// Built-in <c>w:name</c> to the pool style's space above and below, in twips.
    /// </summary>
    /// <remarks>
    /// Keyed on the exact <c>w:name</c> string, and both case variants are spelled out, because
    /// that is how LibreOffice matches too — <c>StyleSheetTable::ConvertStyleName</c>
    /// (<c>sw/source/writerfilter/dmapper/StyleSheetTable.cxx:1640</c>) is an ordinal map that
    /// lists <c>heading 1</c> and <c>Heading 1</c> as separate entries rather than folding case.
    /// </remarks>
    private static readonly Dictionary<string, (int Above, int Below)> Pool =
        new(StringComparer.Ordinal)
        {
            // Heading 1-9 all inherit Writer's "Heading" base, which is where the 12/6 lives.
            ["heading 1"] = (Pt12, Pt6), ["Heading 1"] = (Pt12, Pt6),
            ["heading 2"] = (Pt12, Pt6), ["Heading 2"] = (Pt12, Pt6),
            ["heading 3"] = (Pt12, Pt6), ["Heading 3"] = (Pt12, Pt6),
            ["heading 4"] = (Pt12, Pt6), ["Heading 4"] = (Pt12, Pt6),
            ["heading 5"] = (Pt12, Pt6), ["Heading 5"] = (Pt12, Pt6),
            ["heading 6"] = (Pt12, Pt6), ["Heading 6"] = (Pt12, Pt6),
            ["heading 7"] = (Pt12, Pt6), ["Heading 7"] = (Pt12, Pt6),
            ["heading 8"] = (Pt12, Pt6), ["Heading 8"] = (Pt12, Pt6),
            ["heading 9"] = (Pt12, Pt6), ["Heading 9"] = (Pt12, Pt6),

            // Title and Subtitle measure the same, and Writer's pool puts them under "Heading" too.
            ["Title"] = (Pt12, Pt6),
            ["Subtitle"] = (Pt12, Pt6),

            ["caption"] = (Pt6, Pt6), ["Caption"] = (Pt6, Pt6),

            // "Text body", and "List", which Writer bases on it.
            //
            // Only the capitalised spelling. Lower-case `body text` measures nought on both sides on
            // 26.2.4.2 where `Body Text` measures 140 below, and the two are separate entries in
            // `ConvertStyleName`'s ordinal map rather than one folded case — so this is not an
            // inconsistency to be smoothed over, it is what that map says. See the audit marker on
            // this class; the corpus names `Body Text` in 80 documents and `body text` in none.
            ["Body Text"] = (0, Pt7),
            ["List"] = (0, Pt7),
        };

    /// <summary>
    /// The names whose pool spacing a style keeps when it is the one being half-stated, rather
    /// than only when it is the parent being read through.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's Heading 1-9, Title and Subtitle all descend from its <c>Heading</c> base, and it
    /// is the base's 12 pt / 6 pt that survives — uniformly, not the per-level rows
    /// <c>DocumentStylePoolManager.cxx</c>:843-906 declares (10/6 for Heading 2, 7/6 for
    /// Heading 3, 6/3 for Heading 5, 3/3 from Heading 6 down). Those per-level items sit behind
    /// the <c>bNoDefault</c> guard that the note above already records as inoperative, so what a
    /// heading actually holds is its base's.
    /// </para>
    /// <para>
    /// Measured on the installed 26.2.4.2 by
    /// <c>dotnet/probes/words-pagination-01/one-sided-spacing-source.py</c>, one document naming
    /// fifteen children after built-in styles over a single custom parent declared last, each
    /// stating only <c>w:before="480"</c> — a control value that never appears in the answers, so
    /// "mirror the stated value" is refuted rather than assumed away. Heading 1-9, Title and
    /// Subtitle all read 240 above and 120 below; <c>Caption</c>, <c>List</c>, <c>Quote</c> and
    /// <c>Body Text</c> all read nought on both sides, which is why they are absent here even
    /// though <see cref="Pool"/> lists three of them.
    /// </para>
    /// <para>
    /// That asymmetry is the whole content of this set: <c>Body Text</c> as a <em>parent</em>
    /// still measures 140 below and <c>Caption</c> still measures 120, so <see cref="Pool"/> is
    /// not wrong — the two positions genuinely answer differently, and only the heading family
    /// answers from both.
    /// </para>
    /// <para>
    /// The same sweep also puts one entry of <see cref="Pool"/> in doubt. Lower-case
    /// <c>body text</c> measures nought below as a parent on 26.2.4.2 where <c>Body Text</c>
    /// measures 140, so the two spellings are not interchangeable the way that table has them.
    /// Left alone deliberately: it is a 24.2.7.2 measurement, no corpus document names a parent
    /// that way, and correcting it belongs to whichever round re-measures the whole table.
    /// </para>
    /// </remarks>
    private static readonly HashSet<string> ChildKeeps =
        new(StringComparer.Ordinal)
        {
            "heading 1", "Heading 1", "heading 2", "Heading 2", "heading 3", "Heading 3",
            "heading 4", "Heading 4", "heading 5", "Heading 5", "heading 6", "Heading 6",
            "heading 7", "Heading 7", "heading 8", "Heading 8", "heading 9", "Heading 9",
            "Title", "Subtitle",
        };

    /// <summary>
    /// The space above and below Writer's built-in style of this name, in twips, or a pair of
    /// noughts when the name is not one of Writer's.
    /// </summary>
    /// <param name="styleName">A style's <c>w:name</c>, or null.</param>
    public static (int Above, int Below) For(string? styleName)
        => TryFor(styleName, out (int Above, int Below) spacing) ? spacing : (0, 0);

    /// <summary>
    /// The space above and below Writer's built-in style of this name, and whether Writer has a
    /// style of that name at all.
    /// </summary>
    /// <remarks>
    /// Callers that must tell "not one of Writer's" from "one of Writer's, and nought" want this
    /// rather than <see cref="For(string?)"/>. <see cref="WordStyles.CompleteOneSidedSpacing"/>
    /// is the one that does: an unrecognised parent lets the style's own hierarchy answer, while
    /// a recognised parent holding nought is a real nought and suppresses it.
    /// </remarks>
    /// <param name="styleName">A style's <c>w:name</c>, or null.</param>
    /// <param name="spacing">The pool style's space above and below, in twips.</param>
    public static bool TryFor(string? styleName, out (int Above, int Below) spacing)
    {
        if (styleName is not null && Pool.TryGetValue(styleName, out (int, int) found))
        {
            spacing = found;
            return true;
        }

        spacing = (0, 0);
        return false;
    }

    /// <summary>
    /// The space Writer's built-in style of this name keeps when it is itself the style whose
    /// <c>w:spacing</c> is half-stated, and whether it keeps any.
    /// </summary>
    /// <remarks>
    /// False for every name outside <see cref="ChildKeeps"/>, including names
    /// <see cref="For(string?)"/> answers for — see that set's remarks for why the two positions
    /// differ.
    /// </remarks>
    /// <param name="styleName">A style's <c>w:name</c>, or null.</param>
    /// <param name="spacing">The pool style's space above and below, in twips.</param>
    public static bool TryForOwnName(string? styleName, out (int Above, int Below) spacing)
    {
        if (styleName is not null && ChildKeeps.Contains(styleName))
        {
            spacing = (Pt12, Pt6);
            return true;
        }

        spacing = (0, 0);
        return false;
    }
}

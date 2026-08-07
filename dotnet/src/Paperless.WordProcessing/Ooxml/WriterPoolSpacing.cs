namespace Paperless.WordProcessing.Ooxml;

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
/// half-stated <c>w:spacing</c> freezes the other half at whatever the *parent* style holds at
/// that point in the import — and a parent whose own definition has not been reached yet still
/// holds exactly these numbers.
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
            ["Body Text"] = (0, Pt7), ["body text"] = (0, Pt7),
            ["List"] = (0, Pt7),
        };

    /// <summary>
    /// The space above and below Writer's built-in style of this name, in twips, or a pair of
    /// noughts when the name is not one of Writer's.
    /// </summary>
    /// <param name="styleName">A style's <c>w:name</c>, or null.</param>
    public static (int Above, int Below) For(string? styleName)
        => styleName is not null && Pool.TryGetValue(styleName, out (int, int) spacing) ? spacing : (0, 0);
}

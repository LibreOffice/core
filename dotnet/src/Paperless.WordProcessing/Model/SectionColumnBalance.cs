namespace Paperless.WordProcessing.Model;

/// <summary>
/// Decides which of a Word document's multi-column sections balance their columns.
/// </summary>
/// <remarks>
/// <para>
/// A pass over the finished section list rather than a field either reader can fill in on its own,
/// because the answer is a property of the section <em>after</em> the one being decided. Both of
/// LibreOffice's Word importers say so in the same shape: the WW8 reader marks a section unbalanced when
/// <c>aNext == aEnd || !aNext-&gt;IsContinuous()</c> (<c>sw/source/filter/ww8/ww8par.cxx</c>:4576), and
/// the DOCX one reaches <c>pPrevSection-&gt;DontBalanceTextColumns()</c> from the branch that handles
/// every break type that is "effectively a page break"
/// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:1919), with the last-section case handled
/// separately in <c>ApplyColumnProperties</c>.
/// </para>
/// <para>
/// So the shape people write is: a stretch of two-column text in the middle of an otherwise single-column
/// page — section break continuous, columns, section break continuous — and it is *that* trailing
/// continuous break that makes Word share the text evenly between the columns. Put a page break after the
/// same stretch and Word fills column one to the bottom of the page instead.
/// </para>
/// </remarks>
internal static class SectionColumnBalance
{
    /// <summary>
    /// Marks the sections whose columns balance, leaving the list otherwise untouched.
    /// </summary>
    /// <param name="sections">The sections in document order; modified in place.</param>
    /// <param name="noColumnBalance">
    /// The document's own switch — <c>w:noColumnBalance</c> in a DOCX's settings, <c>fNoColumnBalance</c>
    /// in a DOC's <c>Dop</c>. Set, it turns balancing off for every section at once, which is what both
    /// importers do with it before they look at anything else.
    /// </param>
    internal static void Apply(List<WritingSection> sections, bool noColumnBalance)
    {
        if (noColumnBalance) return;

        for (int i = 0; i + 1 < sections.Count; i++)
        {
            WritingSection section = sections[i];
            if (section.Page.Columns <= 1) continue;

            // Only a continuous break leaves the section above it free to balance. A new-column break is
            // the one case the two importers differ on — DOCX leaves the previous section alone, WW8 does
            // not — and no document in the sample corpus has one following a multi-column section, so the
            // stricter reading is taken rather than a guess about which is right.
            if (sections[i + 1].Break != SectionBreak.Continuous) continue;

            // "Word will interpret a continuous section break between them as if it was a section break
            // next page" when the two disagree about the sheet, and a promoted break is a page break —
            // the same compatibility test `ResolveContinuousBreaks` applies to the geometry.
            if (!SameSheet(section.Page, sections[i + 1].Page)) continue;

            sections[i] = section with { BalancesColumns = true };
        }
    }

    /// <summary>Whether two sections are cut from the same sheet, which is what makes a break continuous.</summary>
    private static bool SameSheet(PageGeometry a, PageGeometry b)
        => a.Size.Width == b.Size.Width
            && a.Size.Height == b.Size.Height
            && a.IsLandscape == b.IsLandscape;
}

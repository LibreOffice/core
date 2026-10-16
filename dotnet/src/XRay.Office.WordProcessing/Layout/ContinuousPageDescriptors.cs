using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// What a continuous section break does with the page properties its own section states.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A continuous section has no page of its own, so it has no page style of its own, so its
/// <c>w:pgMar</c> and its running head are both inert unless something gives them a page to land
/// on.</strong> That is one rule and not two, and it decides paper size, margins, header, footer,
/// numbering format and number restart together, because in Writer all six live on a page descriptor.
/// </para>
/// <para>
/// <c>SectionPropertyMap::CloseSectionGroup</c>'s continuous branch
/// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:1722-1852) inserts a Writer <em>text</em>
/// section and then calls <c>InheritOrFinalizePageStyles</c>, which hands the section the previous
/// one's page style outright whenever it has not created one of its own (<c>:1309-1323</c>). It creates
/// one only because <c>DomainMapper_Impl::PushPageHeaderFooter</c> called <c>GetPageStyle</c> while
/// importing a header or footer the section named (<c>DomainMapper_Impl.cxx</c>:4169) — so a section
/// that names none never has a style to apply, and one that names one has a style that no page yet
/// uses. Attaching it is the <c>else if</c> at <c>:1746</c>, guarded on some slot <em>not</em> being
/// linked to the previous section: it walks the section's own paragraphs for the first carrying
/// <c>BreakType_PAGE_BEFORE</c> and sets <c>PageDescName</c> — and the page number offset — on that one
/// (<c>:1794-1801</c>), failing that on the previous section's last paragraph if that carries one.
/// With no such paragraph anywhere the style stays orphaned and every page keeps the section above's.
/// </para>
/// <para>
/// The WW8 reader states the same rule in its own words and its own code, under the comment <em>"In
/// this nightmare scenario the continuous section has its own headers and footers so we will try and
/// find a hard page break between here and the end of the section and put the headers and footers
/// there"</em> — and caches the descriptor so that it can put the old one back when the search fails:
/// <c>if (bFailed) aIter-&gt;mpPage = pOrig</c> (<c>sw/source/filter/ww8/ww8par.cxx</c>:4515-4560).
/// RTF reaches the dmapper rule, since writerfilter serves both.
/// </para>
/// <para>
/// <strong>Measured on <c>hdss-bulletin-issue-285-25-june-2025.docx</c> against 26.2.4.2</strong>
/// (<c>dotnet/probes/words-continuous-r72/</c>), whose second section is continuous, names
/// <c>header2.xml</c> and states <c>w:top="1418"</c> where the first section states
/// <c>w:top="454" w:header="340"</c>. Four one-attribute mutations settle both halves at once:
/// putting section 2's <c>w:top</c> to 3000 twips moves the reference's page 2 not at all, and neither
/// does the same change to section 3, which contains a hard page break but names no furniture;
/// giving section 2's first paragraph a <c>w:pageBreakBefore</c> makes the reference apply the header
/// <em>and</em> the 1418-twip margin from that page on, page for page and point for point with ours.
/// So the geometry travels with the header and dies with it.
/// </para>
/// <para>
/// <strong>And that is where the reference's page-2 body comes from.</strong> Every page keeps section
/// 1's descriptor, whose header is a single empty paragraph in the <c>Header</c> style — 9 pt Arial,
/// resolving to Liberation Sans. Writer's top margin is that section's <c>w:header</c>, 340 twips =
/// 17.00 pt; <c>PrepareHeaderFooterProperties</c> makes the header's declared height
/// <c>w:top − w:header</c> = 5.70 pt and its body distance that less 1 mm = 2.865 pt
/// (<c>PropertyMap.cxx</c>:1148-1195); the paragraph is 10.35 pt tall, which is more than the declared
/// height, and a DOCX header is imported dynamic-height with dynamic spacing, so it eats the whole of
/// the distance. The body therefore starts at 17.00 + 10.35 = <strong>27.35 pt</strong>, and the first
/// line — <c>Body</c>, <c>w:line="280" w:lineRule="atLeast"</c>, natural height 12.07 pt — puts its
/// baseline 14 pt less its descent below that, at <strong>39.12 pt</strong> against the reference's
/// measured 39.10. Nothing in either section's <c>w:top</c> is 39.1 pt because the number is not a
/// margin: it is the first section's margin plus its empty header.
/// </para>
/// <para>
/// <strong>Reach: 10 of the corpus's 272 DOCX</strong> carry a continuous section that is not the
/// document's first — 23 such sections whose descriptor lands nowhere and 4 that land at a hard break
/// (<c>census.py</c>). The figure recorded by the previous round, 16, counted seven documents whose
/// only continuous <c>w:sectPr</c> is the first section's, where the break type says nothing at all.
/// </para>
/// </remarks>
public static class ContinuousPageDescriptors
{
    /// <summary>
    /// Replaces each continuous section whose page descriptor reaches no page with the descriptor
    /// still in force, and leaves every other section exactly as it was.
    /// </summary>
    /// <param name="sections">The document's sections, in order.</param>
    /// <param name="blocks">
    /// The document's blocks, read for one thing only: whether a section contains a paragraph or table
    /// that starts a page of its own, which is the hard break a descriptor can be hung on.
    /// </param>
    /// <returns>The sections with the rule applied; the same list when nothing changed.</returns>
    public static IReadOnlyList<PaginatedSection> Resolve(
        IReadOnlyList<PaginatedSection> sections,
        IReadOnlyList<PageBlock> blocks)
    {
        ArgumentNullException.ThrowIfNull(sections);
        ArgumentNullException.ThrowIfNull(blocks);

        if (sections.Count < 2) return sections;

        bool[]? breaks = null;
        List<PaginatedSection>? resolved = null;

        for (int i = 1; i < sections.Count; i++)
        {
            PaginatedSection section = resolved is null ? sections[i] : resolved[i];
            if (section.Section.Break != SectionBreak.Continuous) continue;

            // A Writer text section is exempt, and not as a convenience: the whole rule is about a
            // continuous section whose *page style* reaches no page, and a text section never asked for
            // one. ODF states such a section directly (`text:section`) and the master page still decides
            // the paper, the margins and the running head, so there is no descriptor to lose and nothing
            // to inherit — see WritingSection.IsTextSection.
            if (section.Section.IsTextSection) continue;

            // The guard the else-if states: a section every one of whose slots is still linked to the
            // previous one is never even offered a page style, so there is nothing to attach and no
            // point searching for a break.
            if (section.StatesOwnFurniture)
            {
                breaks ??= HardBreaks(blocks, sections.Count);
                if (breaks[i]) continue;
            }

            resolved ??= [.. sections];
            resolved[i] = Inherit(resolved[i - 1], section);
        }

        return resolved ?? sections;
    }

    /// <summary>
    /// One section wearing the descriptor still in force: the previous section's paper, margins,
    /// furniture and numbering, with its own columns, which are the one thing a Writer text section
    /// really does carry.
    /// </summary>
    private static PaginatedSection Inherit(PaginatedSection previous, PaginatedSection section)
    {
        WritingSection from = previous.Section;
        WritingSection own = section.Section;

        PageMargins margins = from.Page.Margins;

        // The WW8 reader has already made these an indent on the text section, which is not part of
        // the descriptor and so does not fall with it. See PaginatedSection.SideMarginsAreASectionIndent
        // for the two file:line citations and the measurement that separates the readers.
        if (section.SideMarginsAreASectionIndent)
        {
            margins = margins with { Left = own.Page.Margins.Left, Right = own.Page.Margins.Right };
        }

        // The furniture goes with it, and it is taken from the *resolved* previous section rather than
        // from what that section stated. That is what makes the carry follow the descriptor chain
        // instead of the section chain, and the two genuinely differ: §17.10.1 has a DOCX section
        // inherit any slot it does not name, so hdss-bulletin's third section would otherwise inherit
        // the second's running head — the very head whose descriptor has just been found to reach no
        // page. `InheritOrFinalizePageStyles` hands on the page *style*, and an orphaned style is still
        // orphaned when the next section inherits its name.
        return section with
        {
            Section = own with
            {
                Page = from.Page with
                {
                    Margins = margins,
                    Gutter = own.Page.Gutter,
                    Columns = own.Page.Columns,
                    ColumnGap = own.Page.ColumnGap,
                    ColumnRuler = own.Page.ColumnRuler,
                },
                Headers = from.Headers,
                Footers = from.Footers,
                HasDifferentFirstPage = from.HasDifferentFirstPage,
                HasDifferentEvenPages = from.HasDifferentEvenPages,
                PageNumberFormat = from.PageNumberFormat,

                // The offset is set inside the same two branches that set `PageDescName`, so a
                // descriptor that reaches no paragraph carries its restart into nothing.
                RestartPageNumberAt = null,
            },
            Furniture = previous.Furniture,
        };
    }

    /// <summary>Which sections hold a block that starts a page of its own.</summary>
    private static bool[] HardBreaks(IReadOnlyList<PageBlock> blocks, int sections)
    {
        bool[] found = new bool[sections];

        foreach (PageBlock block in blocks)
        {
            bool hard = block is PageParagraph { Format.StartsNewPage: true }
                        or PageTable { StartsNewPage: true };

            if (hard) found[Math.Clamp(block.SectionIndex, 0, sections - 1)] = true;
        }

        return found;
    }
}

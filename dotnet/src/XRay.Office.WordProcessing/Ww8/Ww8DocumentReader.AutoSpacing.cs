using XRay.Office.Core.Units;

namespace XRay.Office.WordProcessing.Ww8;

/// <content>
/// Where HTML auto-spacing is <em>not</em> drawn.
/// </content>
/// <remarks>
/// <para>
/// <c>sprmPFDyaBeforeAuto</c> and <c>sprmPFDyaAfterAuto</c> ask for fourteen points, and Word draws
/// nothing at four places the flag alone does not distinguish: the top of a flow, the two edges of a
/// table cell, and between two items of one list. LibreOffice reproduces all four as post-conditions on
/// the node it has just finished rather than as part of resolving the sprm, in
/// <c>SwWW8ImplReader::FinalizeTextNode</c> (<c>sw/source/filter/ww8/ww8par.cxx</c>:2627-2681) and
/// <c>WW8TabDesc::SetPamInCell</c> (<c>ww8par2.cxx</c>:2896-2935), and so does this.
/// </para>
/// <para>
/// All four are conditional on the document <em>using</em> HTML auto-spacing — every one of
/// LibreOffice's tests is written <c>&amp;&amp; !m_xWDop-&gt;fDontUseHTMLAutoSpacing</c>. A document that
/// switched it off gets the five-point margin everywhere, edges included.
/// </para>
/// </remarks>
public sealed partial class Ww8DocumentReader
{
    /// <summary>
    /// Drops the auto margins a flow's edges, its cells' edges and its lists do not draw.
    /// </summary>
    /// <remarks>
    /// Applied per flow, because LibreOffice's <c>m_bFirstPara</c> is reset for every story it reads —
    /// <c>WW8ReaderSave</c> sets it on entry to a header, a footnote or a text box and restores it on the
    /// way out (<c>ww8par.cxx</c>:2195, 2243) — so a running head's first paragraph is as exempt as the
    /// body's.
    /// </remarks>
    /// <param name="blocks">The flow's blocks, rewritten in place.</param>
    private void SuppressAutoSpacing(List<Ww8LayoutBlock> blocks)
        => SuppressAutoSpacing(blocks, DocumentProperties.CollapsesSpacing);

    /// <inheritdoc cref="SuppressAutoSpacing(List{Ww8LayoutBlock})"/>
    /// <param name="blocks">The flow's blocks, rewritten in place.</param>
    /// <param name="collapsesSpacing">
    /// Whether the document uses HTML auto-spacing at all — the negation of
    /// <c>fDontUseHTMLAutoSpacing</c>, which every one of LibreOffice's four tests is conditional on.
    /// </param>
    /// <remarks>
    /// Split from the reader's own state so the rules can be exercised on hand-built blocks: the sprms
    /// they act on are ones LibreOffice reads and never writes, so no fixture for them can be produced
    /// by converting a document with <c>soffice</c>.
    /// </remarks>
    internal static void SuppressAutoSpacing(List<Ww8LayoutBlock> blocks, bool collapsesSpacing)
    {
        ArgumentNullException.ThrowIfNull(blocks);
        if (!collapsesSpacing) return;

        // One walk in document order, cells included, because LibreOffice's own memory of "the last
        // numbered paragraph" is reader-global and survives a cell boundary. Walking each cell as its
        // own little flow was the defect: a list that runs to the end of a cell never met the
        // unnumbered paragraph that hands its margin back, so every such list closed up.
        new ListRun().Walk(blocks);

        // The flow's own first paragraph, and then every cell's edges, which the walk reaches through
        // the tables. A flow beginning with a table has no first paragraph of its own: the paragraph
        // that would be it is the first of a cell, and the cell rule is the one that applies.
        if (blocks.Count > 0 && blocks[0].Paragraph is { HasAutoSpaceBefore: true } first)
        {
            blocks[0] = new Ww8LayoutBlock(WithoutSpaceBefore(first));
        }
    }

    /// <summary>
    /// The run of numbered paragraphs the walk is inside, and where its margin has to be given back.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice zeroes both auto margins of every numbered paragraph and then gives the lower one
    /// back to whichever item turned out to be the last, by remembering the previous numbered
    /// paragraph and restoring its lower margin when the rule changes or the numbering stops
    /// (<c>SwWW8ImplReader::FinalizeTextNode</c>, <c>ww8par.cxx</c>:2627-2673). The memory is a member
    /// of the reader, not of a cell, which is why this is one walk rather than one per flow.
    /// </para>
    /// <para>
    /// <strong>A cell's last paragraph is zeroed but never remembered.</strong>
    /// <c>WW8TabDesc::SetPamInCell</c> (<c>ww8par2.cxx</c>:2908-2934) forces that paragraph's lower
    /// spacing to nought on the way into the next cell, so the item the restore lands on is the one
    /// before it. Measured against LibreOffice's own flat-ODF export of <c>FlightLaws.doc</c>, whose
    /// four-bullet <em>Ground Mode</em> cell puts <c>fo:margin-bottom="0.1945in"</c> on the
    /// <em>third</em> bullet and nought on the fourth, and likewise on the ninth of ten in
    /// <em>Flight Mode</em>, the second of three in <em>Flare Mode</em> and the first of the two that
    /// close the <em>Protections</em> cell. Six groups, six agreements. A list run that ends inside a
    /// cell — because a heading follows it there — keeps the ordinary rule and gives the margin to its
    /// own last item.
    /// </para>
    /// <para>
    /// The claim this replaces was that <c>FlightLaws.doc</c> holds empty paragraphs in its cells that
    /// we drop. It holds none: between <c>Is active until shortly after liftoff.</c> and
    /// <c>After touchdown, …</c> the piece table has a single <c>U+000D</c>, and the gap the reference
    /// draws there is this margin.
    /// </para>
    /// </remarks>
    private sealed class ListRun
    {
        /// <summary>
        /// Always the HTML value: every caller is already behind the <c>CollapsesSpacing</c> test,
        /// which is the same condition <c>GetParagraphAutoSpace</c> branches on to choose between the
        /// two.
        /// </summary>
        private static readonly Length AutoSpacing =
            Length.FromTwips(Ww8LayoutFormat.HtmlAutoSpacingTwips);

        /// <summary>The list the remembered paragraph belongs to, or nought for none.</summary>
        private int _rule;

        /// <summary>The list holding the remembered paragraph, so its margin can be given back.</summary>
        private List<Ww8LayoutBlock>? _remembered;

        /// <summary>Where in that list it sits.</summary>
        private int _at = -1;

        /// <summary>Walks a run of blocks in document order, recursing into any table among them.</summary>
        internal void Walk(List<Ww8LayoutBlock> blocks)
        {
            for (int index = 0; index < blocks.Count; index++)
            {
                if (blocks[index].Table is { } table)
                {
                    blocks[index] = new Ww8LayoutBlock(Table(table));
                    continue;
                }

                if (blocks[index].Paragraph is not { } paragraph) continue;

                // Nothing at this level is a cell's last paragraph, so every one of them is eligible
                // to be remembered.
                Item(blocks, index, paragraph, remember: true);
            }
        }

        /// <summary>One table's cells, in document order, with the cell edges applied to each.</summary>
        private Ww8LayoutTable Table(Ww8LayoutTable table)
        {
            List<Ww8LayoutRow> rows = new(table.Rows.Count);

            foreach (Ww8LayoutRow row in table.Rows)
            {
                List<Ww8LayoutCell> cells = new(row.Cells.Count);

                foreach (Ww8LayoutCell cell in row.Cells)
                {
                    List<Ww8LayoutBlock> blocks = [.. cell.Blocks];

                    for (int index = 0; index < blocks.Count; index++)
                    {
                        if (blocks[index].Table is { } nested)
                        {
                            blocks[index] = new Ww8LayoutBlock(Table(nested));
                            continue;
                        }

                        if (blocks[index].Paragraph is not { } paragraph) continue;

                        // A paragraph a cell mark ended is zeroed like any other and then left
                        // alone: `SetPamInCell` has already forced its lower spacing to nought, so it
                        // neither consults nor becomes the memory and the restore lands on the item
                        // before it.
                        Item(blocks, index, paragraph, remember: index != blocks.Count - 1);
                    }

                    // "The first paragraph in a cell with upper autospacing has upper spacing set to
                    // 0", and the last with lower autospacing likewise — LibreOffice's own comments,
                    // and its own ordering: both run after the list rules and so overrule them.
                    if (blocks.Count > 0 && blocks[0].Paragraph is { HasAutoSpaceBefore: true } head)
                    {
                        blocks[0] = new Ww8LayoutBlock(WithoutSpaceBefore(head));
                    }

                    if (blocks.Count > 0 && blocks[^1].Paragraph is { HasAutoSpaceAfter: true } tail)
                    {
                        blocks[^1] = new Ww8LayoutBlock(WithoutSpaceAfter(tail));
                    }

                    cells.Add(cell with { Blocks = blocks });
                }

                rows.Add(row with { Cells = cells });
            }

            return table with { Rows = rows };
        }

        /// <summary>
        /// <c>FinalizeTextNode</c>'s three branches over one paragraph.
        /// </summary>
        /// <param name="blocks">The list the paragraph sits in, rewritten in place.</param>
        /// <param name="index">Where in it.</param>
        /// <param name="paragraph">The paragraph itself.</param>
        /// <param name="remember">
        /// False for a cell's last paragraph, which LibreOffice's cell rule has already settled.
        /// </param>
        private void Item(
            List<Ww8LayoutBlock> blocks, int index, Ww8LayoutParagraph paragraph, bool remember)
        {
            int rule = paragraph.ListRule;

            if (rule != 0 && (paragraph.HasAutoSpaceBefore || paragraph.HasAutoSpaceAfter))
            {
                if (paragraph.HasAutoSpaceAfter) paragraph = WithoutSpaceAfter(paragraph);
                if (_rule != 0 && paragraph.HasAutoSpaceBefore)
                {
                    paragraph = WithoutSpaceBefore(paragraph);
                }

                blocks[index] = new Ww8LayoutBlock(paragraph);

                if (!remember) return;

                if (rule != _rule) Restore();

                _remembered = blocks;
                _at = index;
                _rule = rule;
                return;
            }

            if (!remember) return;

            // The numbering stopped, so whichever item was last regains the margin that separates the
            // list from what follows it. The restoration is unconditional on the remembered
            // paragraph's own flags, as it is there: a paragraph only becomes the remembered one by
            // having had an auto margin in the first place.
            if (rule == 0) Restore();

            _remembered = null;
            _at = -1;
            _rule = rule;
        }

        /// <summary>Hands the margin back to the remembered item, and forgets it.</summary>
        private void Restore()
        {
            if (_remembered is { } list && _at >= 0 && _at < list.Count
                && list[_at].Paragraph is { } item)
            {
                list[_at] = new Ww8LayoutBlock(WithSpaceAfter(item, AutoSpacing));
            }

            _remembered = null;
            _at = -1;
        }
    }

    private static Ww8LayoutParagraph WithoutSpaceBefore(Ww8LayoutParagraph paragraph)
        => paragraph with { Format = paragraph.Format with { SpaceBefore = Length.Zero } };

    private static Ww8LayoutParagraph WithoutSpaceAfter(Ww8LayoutParagraph paragraph)
        => paragraph with { Format = paragraph.Format with { SpaceAfter = Length.Zero } };

    private static Ww8LayoutParagraph WithSpaceAfter(Ww8LayoutParagraph paragraph, Length value)
        => paragraph with { Format = paragraph.Format with { SpaceAfter = value } };
}

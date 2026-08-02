using Paperless.Core.Units;

namespace Paperless.WordProcessing.Ww8;

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

        SuppressWithinList(blocks);

        // The flow's own first paragraph, and then every cell's edges, which the walk reaches through
        // the tables. A flow beginning with a table has no first paragraph of its own: the paragraph
        // that would be it is the first of a cell, and the cell rule is the one that applies.
        if (blocks.Count > 0 && blocks[0].Paragraph is { HasAutoSpaceBefore: true } first)
        {
            blocks[0] = new Ww8LayoutBlock(WithoutSpaceBefore(first));
        }

        for (int i = 0; i < blocks.Count; i++)
        {
            if (blocks[i].Table is { } table) blocks[i] = new Ww8LayoutBlock(WithCellEdges(table));
        }
    }

    /// <summary>The same rules over one table's cells, and over any table nested in them.</summary>
    private static Ww8LayoutTable WithCellEdges(Ww8LayoutTable table)
    {
        List<Ww8LayoutRow> rows = new(table.Rows.Count);

        foreach (Ww8LayoutRow row in table.Rows)
        {
            List<Ww8LayoutCell> cells = new(row.Cells.Count);

            foreach (Ww8LayoutCell cell in row.Cells)
            {
                List<Ww8LayoutBlock> blocks = [.. cell.Blocks];

                SuppressWithinList(blocks);

                // "The first paragraph in a cell with upper autospacing has upper spacing set to 0", and
                // the last with lower autospacing likewise — LibreOffice's own comments, and its own
                // ordering: both run after the list rules and so overrule them.
                if (blocks.Count > 0 && blocks[0].Paragraph is { HasAutoSpaceBefore: true } head)
                {
                    blocks[0] = new Ww8LayoutBlock(WithoutSpaceBefore(head));
                }

                if (blocks.Count > 0 && blocks[^1].Paragraph is { HasAutoSpaceAfter: true } tail)
                {
                    blocks[^1] = new Ww8LayoutBlock(WithoutSpaceAfter(tail));
                }

                for (int i = 0; i < blocks.Count; i++)
                {
                    if (blocks[i].Table is { } nested)
                    {
                        blocks[i] = new Ww8LayoutBlock(WithCellEdges(nested));
                    }
                }

                cells.Add(cell with { Blocks = blocks });
            }

            rows.Add(row with { Cells = cells });
        }

        return table with { Rows = rows };
    }

    /// <summary>
    /// Closes up the auto margins between two items of one list.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word draws no auto space inside a list and one after it, which LibreOffice reaches by zeroing
    /// both margins of every numbered paragraph and then giving the margin back to whichever item turned
    /// out to be the last — it remembers the previous numbered paragraph and restores its lower margin
    /// when the rule changes or the numbering stops (<c>ww8par.cxx</c>:2627-2673).
    /// </para>
    /// <para>
    /// The restoration is unconditional on the remembered paragraph's own flags, as it is there: a
    /// paragraph only becomes the remembered one by having had an auto margin in the first place.
    /// </para>
    /// </remarks>
    private static void SuppressWithinList(List<Ww8LayoutBlock> blocks)
    {
        // Always the HTML value: every caller is already behind the `CollapsesSpacing` test, which is
        // the same condition `GetParagraphAutoSpace` branches on to choose between the two.
        Length autoSpacing = Length.FromTwips(Ww8LayoutFormat.HtmlAutoSpacingTwips);

        int previousRule = 0;
        int previous = -1;

        for (int i = 0; i < blocks.Count; i++)
        {
            if (blocks[i].Paragraph is not { } paragraph)
            {
                // A table breaks the run of paragraphs without ending the list the way a plain
                // paragraph does, so the remembered item is simply forgotten rather than restored.
                previousRule = 0;
                previous = -1;
                continue;
            }

            int rule = paragraph.ListRule;

            if (rule != 0 && (paragraph.HasAutoSpaceBefore || paragraph.HasAutoSpaceAfter))
            {
                if (paragraph.HasAutoSpaceAfter) paragraph = WithoutSpaceAfter(paragraph);
                if (previousRule != 0 && paragraph.HasAutoSpaceBefore)
                {
                    paragraph = WithoutSpaceBefore(paragraph);
                }

                blocks[i] = new Ww8LayoutBlock(paragraph);

                if (rule != previousRule && previous >= 0)
                {
                    blocks[previous] = new Ww8LayoutBlock(
                        WithSpaceAfter(blocks[previous].Paragraph!.Value, autoSpacing));
                }

                previous = i;
                previousRule = rule;
                continue;
            }

            if (rule == 0 && previous >= 0)
            {
                blocks[previous] = new Ww8LayoutBlock(
                    WithSpaceAfter(blocks[previous].Paragraph!.Value, autoSpacing));
            }

            previous = -1;
            previousRule = rule;
        }
    }

    private static Ww8LayoutParagraph WithoutSpaceBefore(Ww8LayoutParagraph paragraph)
        => paragraph with { Format = paragraph.Format with { SpaceBefore = Length.Zero } };

    private static Ww8LayoutParagraph WithoutSpaceAfter(Ww8LayoutParagraph paragraph)
        => paragraph with { Format = paragraph.Format with { SpaceAfter = Length.Zero } };

    private static Ww8LayoutParagraph WithSpaceAfter(Ww8LayoutParagraph paragraph, Length value)
        => paragraph with { Format = paragraph.Format with { SpaceAfter = value } };
}

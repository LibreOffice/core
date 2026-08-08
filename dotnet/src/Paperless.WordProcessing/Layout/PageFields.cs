namespace Paperless.WordProcessing.Layout;

/// <summary>
/// A field whose value only pagination can supply.
/// </summary>
/// <remarks>
/// <para>
/// Deliberately two members rather than the document model's whole <c>WritingFieldKind</c> vocabulary.
/// Every other field a document can carry is either constant for the run — an author, a title, a save
/// date — or is a cross-reference the reader already resolved, and for all of those the cached result
/// the producer wrote is both correct and the thing a reference renderer draws. These two are the ones
/// whose cached result is *wrong on every page but one*, because the value changes as the document is
/// laid out and cannot be known before it is.
/// </para>
/// </remarks>
public enum PageFieldKind
{
    /// <summary>The page's own number: Word's <c>PAGE</c>, ODF's <c>text:page-number</c>.</summary>
    PageNumber,

    /// <summary>
    /// How many pages the document has: <c>NUMPAGES</c>, <c>text:page-count</c>.
    /// </summary>
    /// <remarks>
    /// The total is not known while the running head is being laid out, so it takes a second pass: the
    /// document is filled once to learn how many pages it has, the furniture is told, and it is filled
    /// again. That is one extra pass over documents that carry the field and none over those that do not.
    /// </remarks>
    PageCount,
}

/// <summary>
/// Where a field's result sits in a paragraph's text, and what it computes.
/// </summary>
/// <remarks>
/// <para>
/// A span over <see cref="PageParagraph.Text"/> rather than a run of its own, because that is what a
/// field's result is in all four formats: ordinary text, carrying ordinary run formatting, bracketed by
/// markers that occupy no width. Modelling it as a run would force every reader to split its runs at the
/// field's edges and would make a field that spans two differently-formatted runs unrepresentable.
/// </para>
/// <para>
/// The span covers the *result*, never the instruction: the instruction is not drawn and the readers
/// already drop it.
/// </para>
/// </remarks>
/// <param name="Start">The first character of the result, as an offset into the paragraph's text.</param>
/// <param name="Length">How many characters the result occupies; zero for a field with an empty result.</param>
/// <param name="Kind">What the field computes.</param>
/// <param name="Format">
/// The sequence the field's own <c>\*</c> switch asks for, or null to take the section's — which is
/// where all four formats state it and what almost every field does.
/// </param>
public readonly record struct PageFieldSpan(
    int Start,
    int Length,
    PageFieldKind Kind,
    NoteNumberFormat? Format = null)
{
    /// <summary>One past the result's last character.</summary>
    public int End => Start + Length;
}

/// <summary>
/// Substitutes the fields whose value pagination decides into the blocks that carry them.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why this exists at all.</strong> All four readers parse a <c>PAGE</c> field, record its kind,
/// and lay out the result string the producing application cached — so a nine-page document whose footer
/// says <c>Page 9</c> prints <c>Page 9</c> on all nine of its pages. Measured on
/// <c>batch-010/195584360.docx</c>: ours printed <c>Page 10</c> on every one of its twenty pages where
/// LibreOffice printed 1 to 20. The parse looked like support, which is what let it survive; the
/// vocabulary was produced by every reader and consumed by nothing.
/// </para>
/// <para>
/// <strong>Why it rewrites the text rather than the drawing.</strong> A page number is text of a
/// different width from the one that was cached — <c>1</c> against <c>10</c> is half the advance — so
/// substituting it after the line has been broken would put the rest of the line in the wrong place, and
/// a right-aligned or centred footer in the wrong place entirely. The substitution therefore happens
/// before the flow is laid out, which is also where Writer does it: a <c>SwFieldPortion</c> is built from
/// the field's *expansion* and takes part in line breaking like any other portion
/// (<c>sw/source/core/text/porfld.cxx</c>).
/// </para>
/// <para>
/// <strong>What it does not do.</strong> Body text is left alone. A <c>PAGE</c> in the body is legal and
/// rare, and resolving it is circular in a way the running head's is not: the page a paragraph lands on
/// is decided by how tall the paragraphs before it are, and rewriting one changes that. Writer solves it
/// with a formatting loop that re-runs until the numbers stop moving; that is a larger change than this
/// and buys, on this corpus, nothing — a scan of the 134 zip-container word documents finds every
/// <c>PAGE</c> field in a header or a footer.
/// </para>
/// </remarks>
public static class PageFields
{
    /// <summary>
    /// The blocks with every page-number field replaced by the number the page actually carries.
    /// </summary>
    /// <remarks>
    /// Returns the input unchanged — the same list, not a copy — when nothing in it carries a field this
    /// resolves. That is the overwhelmingly common case, and it is what keeps a document with no fields
    /// from paying for this at all.
    /// </remarks>
    /// <param name="blocks">The blocks to resolve, usually one running head's.</param>
    /// <param name="pageNumber">The number this page prints.</param>
    /// <param name="format">The sequence the section writes its page numbers in.</param>
    /// <param name="totalPages">
    /// How many pages the document has, or nought when that is not yet known — the first pass of a
    /// two-pass layout. A <c>NUMPAGES</c> field is left at its cached result while it is nought, which is
    /// what the pass that is only measuring the document wants.
    /// </param>
    public static IReadOnlyList<PageBlock> Resolve(
        IReadOnlyList<PageBlock> blocks, int pageNumber, NoteNumberFormat format, int totalPages = 0)
    {
        if (!CarriesPageNumber(blocks) && !(totalPages > 0 && CarriesPageCount(blocks))) return blocks;

        List<PageBlock> resolved = new(blocks.Count);

        foreach (PageBlock block in blocks)
        {
            resolved.Add(ResolveBlock(block, pageNumber, format, totalPages));
        }

        return resolved;
    }

    /// <summary>
    /// True when anything in these blocks would be changed by <see cref="Resolve"/>.
    /// </summary>
    /// <remarks>
    /// Asked by the caller as well, because it decides whether a laid-out running head can be cached
    /// across pages: one without a page number is the same on every page and must be laid out once, and
    /// one with a page number is different on every page and must not be.
    /// </remarks>
    /// <param name="blocks">The blocks to examine.</param>
    public static bool CarriesPageNumber(IReadOnlyList<PageBlock>? blocks)
        => Carries(blocks, PageFieldKind.PageNumber);

    /// <summary>
    /// True when these blocks carry a <c>NUMPAGES</c> field.
    /// </summary>
    /// <remarks>
    /// Kept apart from <see cref="CarriesPageNumber"/> because the two answer different questions, and
    /// conflating them costs a re-shaping of the running head on every page of every document: a head
    /// holding only a page <em>count</em> is the same on every page, so it is still cached against its
    /// slot alone. What a page count needs is a second pass, not a per-page one.
    /// </remarks>
    /// <param name="blocks">The blocks to examine.</param>
    public static bool CarriesPageCount(IReadOnlyList<PageBlock>? blocks)
        => Carries(blocks, PageFieldKind.PageCount);

    private static bool Carries(IReadOnlyList<PageBlock>? blocks, PageFieldKind kind)
    {
        if (blocks is null) return false;

        foreach (PageBlock block in blocks)
        {
            switch (block)
            {
                case PageParagraph paragraph:
                    foreach (PageFieldSpan field in paragraph.Fields)
                    {
                        if (field.Kind == kind) return true;
                    }

                    break;

                case PageTable table:
                    foreach (PageTableRow row in table.Rows)
                    {
                        foreach (PageTableCell cell in row.Cells)
                        {
                            if (Carries(cell.Blocks, kind)) return true;
                        }
                    }

                    break;
            }
        }

        return false;
    }

    private static PageBlock ResolveBlock(
        PageBlock block, int pageNumber, NoteNumberFormat format, int totalPages)
        => block switch
        {
            PageParagraph paragraph => ResolveParagraph(paragraph, pageNumber, format, totalPages),
            PageTable table => ResolveTable(table, pageNumber, format, totalPages),
            _ => block,
        };

    private static PageTable ResolveTable(
        PageTable table, int pageNumber, NoteNumberFormat format, int totalPages)
    {
        List<PageTableRow> rows = new(table.Rows.Count);
        foreach (PageTableRow row in table.Rows)
        {
            List<PageTableCell> cells = new(row.Cells.Count);
            foreach (PageTableCell cell in row.Cells)
            {
                cells.Add(cell with { Blocks = Resolve(cell.Blocks, pageNumber, format, totalPages) });
            }

            rows.Add(row with { Cells = cells });
        }

        return table with { Rows = rows };
    }

    /// <summary>
    /// One paragraph, with each page-number span replaced and every offset after it moved.
    /// </summary>
    /// <remarks>
    /// Spans are applied from the *end* backwards, so that an earlier span's offsets are still valid when
    /// it is reached. Applying them forwards would need every remaining span shifted as it went, which is
    /// the same arithmetic done once per span rather than not at all.
    /// </remarks>
    private static PageParagraph ResolveParagraph(
        PageParagraph paragraph, int pageNumber, NoteNumberFormat format, int totalPages)
    {
        List<PageFieldSpan> spans = [];
        foreach (PageFieldSpan field in paragraph.Fields)
        {
            if (field.Kind == PageFieldKind.PageCount && totalPages <= 0) continue;
            if (field.Start < 0 || field.End > paragraph.Text.Length || field.Length < 0) continue;
            spans.Add(field);
        }

        if (spans.Count == 0) return paragraph;
        spans.Sort((left, right) => left.Start.CompareTo(right.Start));

        PageParagraph result = paragraph;
        for (int at = spans.Count - 1; at >= 0; at--)
        {
            PageFieldSpan span = spans[at];
            int value = span.Kind == PageFieldKind.PageCount ? totalPages : pageNumber;

            result = Splice(
                result, span.Start, span.Length,
                NoteNumbering.Render(span.Format ?? format, value));
        }

        // Every span is dropped, including a page count this pass could not resolve. That is safe because
        // resolution never consumes its input: the furniture set keeps the section's own blocks and
        // resolves a fresh copy for each page, so the pass that learns the total starts from the spans
        // again rather than from what the measuring pass left.
        return result with { Fields = [] };
    }

    /// <summary>
    /// Replaces <paramref name="length"/> characters at <paramref name="start"/> with
    /// <paramref name="replacement"/>, carrying every offset in the paragraph with it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The replacement takes the formatting of the run the span *started* in, which is what Word does with
    /// a field result: the whole result is one run, and a producer that split it across two only did so
    /// because it wrote several digits. Runs wholly inside the span are dropped and runs after it move.
    /// </para>
    /// <para>
    /// A paragraph with no runs at all is uniform, and its whole text is drawn in the paragraph's own face
    /// — so there is nothing to remap and the text alone changes.
    /// </para>
    /// </remarks>
    private static PageParagraph Splice(
        PageParagraph paragraph, int start, int length, string replacement)
    {
        int end = start + length;
        int delta = replacement.Length - length;

        string text = string.Concat(
            paragraph.Text.AsSpan(0, start), replacement, paragraph.Text.AsSpan(end));

        PageParagraph result = paragraph with { Text = text };

        if (paragraph.Runs.Count > 0)
        {
            List<PageRun> runs = new(paragraph.Runs.Count);
            bool replaced = false;

            foreach (PageRun run in paragraph.Runs)
            {
                if (run.End <= start)
                {
                    runs.Add(run);
                    continue;
                }

                if (run.Start >= end)
                {
                    runs.Add(run with { Start = run.Start + delta });
                    continue;
                }

                // The run overlaps the span. Its part before the span survives; the replacement takes the
                // formatting of the first such run; its part after the span survives, moved.
                if (run.Start < start) runs.Add(run with { Length = start - run.Start });

                if (!replaced && replacement.Length > 0)
                {
                    runs.Add(run with { Start = start, Length = replacement.Length });
                    replaced = true;
                }

                if (run.End > end)
                {
                    runs.Add(run with
                    {
                        Start = end + delta,
                        Length = run.End - end,
                    });
                }
            }

            result = result with { Runs = runs };
        }

        if (paragraph.Notes.Count > 0)
        {
            List<PageNote> notes = new(paragraph.Notes.Count);
            foreach (PageNote note in paragraph.Notes)
            {
                notes.Add(note.Offset >= end ? note with { Offset = note.Offset + delta } : note);
            }

            result = result with { Notes = notes };
        }

        if (paragraph.Frames.Count > 0)
        {
            List<PageFrame> frames = new(paragraph.Frames.Count);
            foreach (PageFrame frame in paragraph.Frames)
            {
                frames.Add(frame.AnchorOffset >= end
                    ? frame with { AnchorOffset = frame.AnchorOffset + delta }
                    : frame);
            }

            result = result with { Frames = frames };
        }

        return result;
    }
}

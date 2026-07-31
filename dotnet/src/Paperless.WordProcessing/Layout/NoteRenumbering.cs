namespace Paperless.WordProcessing.Layout;

/// <summary>
/// Numbers a document's notes again once its pages exist, for a class that restarts on every page.
/// </summary>
/// <remarks>
/// <para>
/// <b>The circularity, and Writer's answer to it.</b> A note's number under a per-page restart is its
/// position within its page; the width of the citation depends on that number; and where the citing line
/// breaks depends on that width. There is no fixed point to find, and Writer does not look for one. It
/// numbers per page as a pass over pages that already exist — <c>SwRootFrame::UpdateFootnoteNums</c>
/// (<c>sw/source/core/layout/ftnfrm.cxx</c>:971) runs only for <c>FTNNUM_PAGE</c> and walks the finished
/// page frames, and <c>SwPageFrame::UpdateFootnoteNum</c> (<c>ftnfrm.cxx</c>:2564) counts one page's notes
/// and assigns 1..n — and then it <em>damps</em>. <c>SwTextFootnote::SetNumber</c>
/// (<c>sw/source/core/txtnode/atrftn.cxx</c>:362) invalidates the citing text node and every text node of
/// the note's own body, so a renumber really can rebreak the line that cites it; <c>flowfrm.cxx</c>:2268
/// renumbers both pages the moment a note moves between them; and <c>txtftn.cxx</c>:560 then validates the
/// frame under the comment <em>"We break the oscillation"</em>, accepting a layout one iteration stale
/// rather than chasing a fixed point that need not exist.
/// </para>
/// <para>
/// So the shape here is Writer's: paginate with the document-order numbering, renumber per page over the
/// finished pages, lay out once more, and stop. <see cref="Paginator"/> does the stopping; this does the
/// numbering.
/// </para>
/// <para>
/// <b>Why a rewrite of the blocks rather than a number on the note.</b> Because the number is not a number
/// anywhere else in this engine: LibreOffice draws it twice, as a superscript at the anchor in the sentence
/// and again at the head of the note, and all four readers emit it into the text at both places. So a
/// renumbering is a text edit — which is also what makes the width feedback real, and what makes doing it
/// once and stopping the only terminating answer.
/// </para>
/// <para>
/// Every document whose notes do not restart per page takes none of this: <see cref="Applies"/> is the
/// guard and it is false for all but one file in the corpus.
/// </para>
/// </remarks>
internal static class NoteRenumbering
{
    /// <summary>True when any note in the document asks for a per-page restart.</summary>
    /// <remarks>
    /// Asked before anything else so that the common document pays one walk over its blocks and nothing
    /// more — no second pagination, no rewritten paragraphs, no allocation.
    /// </remarks>
    public static bool Applies(IReadOnlyList<PageBlock> blocks)
    {
        foreach (PageBlock block in blocks)
        {
            if (block is PageParagraph paragraph
                && paragraph.Notes.Any(note => note.Restart == NoteRestart.EachPage))
            {
                return true;
            }
        }

        return false;
    }

    /// <summary>
    /// The blocks with each restarting note cited by its position on its page, or null when nothing moved.
    /// </summary>
    /// <remarks>
    /// Null rather than an equal list, because that is the paginator's signal to stop: if no citation
    /// changed then the pages already say what they should and laying them out again would produce the same
    /// answer at the cost of a second shaping pass.
    /// </remarks>
    /// <param name="blocks">The document's blocks, as they were paginated.</param>
    /// <param name="pages">The pages that pagination produced from them.</param>
    public static IReadOnlyList<PageBlock>? Apply(
        IReadOnlyList<PageBlock> blocks, IReadOnlyList<LaidOutPage> pages)
    {
        Dictionary<PageNote, string> renumbered = new(ByReference.Instance);

        foreach (LaidOutPage page in pages)
        {
            // Two counters per page, because the two classes are two sequences: a page citing a footnote
            // and an endnote cites 1 and i, not 1 and ii.
            int footnotes = 0;
            int endnotes = 0;

            foreach (PageNote note in NotesOn(blocks, page))
            {
                if (note.Restart != NoteRestart.EachPage) continue;

                int index = note.IsEndnote ? endnotes++ : footnotes++;

                // A note that recorded no citation still counts towards its page's total — it is a note —
                // but there is nothing in the text to find and replace, so it is left as it was rather than
                // having a number spliced in at whatever offset it happens to carry.
                if (note.Citation.Length == 0) continue;

                string citation = note.Numbering.Citation(index);

                if (!string.Equals(citation, note.Citation, StringComparison.Ordinal))
                {
                    renumbered[note] = citation;
                }
            }
        }

        if (renumbered.Count == 0) return null;

        List<PageBlock> rewritten = new(blocks.Count);
        foreach (PageBlock block in blocks)
        {
            rewritten.Add(
                block is PageParagraph paragraph ? Rewrite(paragraph, renumbered) : block);
        }

        return rewritten;
    }

    /// <summary>
    /// The notes a page holds, in the order they are cited.
    /// </summary>
    /// <remarks>
    /// Recomputed from the page's lines rather than recorded during pagination, which keeps this a pure
    /// function of a page and its blocks: a note belongs to the page holding the line that contains its
    /// anchor, which is the same rule the paginator uses to decide whose room it takes. Endnotes collecting
    /// at the end of the document are skipped, because the page that cites one is not the page it lands on.
    /// </remarks>
    private static IEnumerable<PageNote> NotesOn(IReadOnlyList<PageBlock> blocks, LaidOutPage page)
    {
        IReadOnlyList<PageBlock> own = page.Blocks ?? blocks;

        foreach (PlacedLine line in page.Lines)
        {
            if (line.ParagraphIndex < 0 || line.ParagraphIndex >= own.Count) continue;
            if (own[line.ParagraphIndex] is not PageParagraph paragraph) continue;
            if (paragraph.Notes.Count == 0) continue;

            foreach (PageNote note in paragraph.Notes)
            {
                if (note.Placement == NotePlacement.DocumentEnd) continue;
                if (note.Offset >= line.Box.Line.Start && note.Offset < line.Box.Line.End)
                {
                    yield return note;
                }
            }
        }
    }

    /// <summary>One paragraph with the citations of its renumbered notes replaced.</summary>
    /// <remarks>
    /// In ascending order of offset, so that each replacement's shift applies to everything after it and to
    /// nothing before. Nearly every renumbering leaves the lengths alone — a page's fourth note is one digit
    /// whether it is cited 4 or 12 — and then the shift is zero and the paragraph is rebuilt identically but
    /// for its text.
    /// </remarks>
    private static PageParagraph Rewrite(
        PageParagraph paragraph, Dictionary<PageNote, string> renumbered)
    {
        if (!paragraph.Notes.Any(renumbered.ContainsKey)) return paragraph;

        string text = paragraph.Text;
        List<PageRun> runs = [.. paragraph.Runs];
        List<PageFrame> frames = [.. paragraph.Frames];
        List<PageNote> notes = [.. paragraph.Notes];

        // By position rather than by identity, and in ascending order of offset so that each replacement's
        // shift lands on everything after it and on nothing before. The list is rebuilt as it goes — a
        // shifted note is a new record — so an identity lookup would find the first note and lose the rest.
        int[] order =
            [.. Enumerable.Range(0, notes.Count).OrderBy(at => paragraph.Notes[at].Offset)];

        foreach (int at in order)
        {
            PageNote note = paragraph.Notes[at];
            if (!renumbered.TryGetValue(note, out string? citation)) continue;

            // The note itself is rewritten whether or not its width changed, since its body carries the
            // same number at its head and a body left alone would cite the note it used to be.
            int offset = notes[at].Offset;
            int was = note.Citation.Length;

            if (offset < 0 || offset + was > text.Length) continue;

            text = string.Concat(text.AsSpan(0, offset), citation, text.AsSpan(offset + was));

            int shift = citation.Length - was;
            if (shift != 0)
            {
                Shift(runs, frames, notes, offset, was, shift);
            }

            notes[at] = notes[at] with
            {
                Citation = citation,
                Blocks = RewriteBody(notes[at], citation),
            };
        }

        return paragraph with { Text = text, Runs = runs, Frames = frames, Notes = notes };
    }

    /// <summary>The note's body with the number at its head replaced.</summary>
    /// <remarks>
    /// The second of the two places a note's number is drawn, and the one a reader never guesses at: the
    /// offset came from whichever reader built the note, because three of the four prepend the number and
    /// DOCX puts it where a <c>w:footnoteRef</c> marks.
    /// </remarks>
    private static IReadOnlyList<PageBlock> RewriteBody(PageNote note, string citation)
    {
        if (note.Blocks.Count == 0 || note.Blocks[0] is not PageParagraph first) return note.Blocks;

        int offset = note.BodyOffset;
        int was = note.Citation.Length;
        if (offset < 0 || offset + was > first.Text.Length) return note.Blocks;

        if (!first.Text.AsSpan(offset, was).SequenceEqual(citation))
        {
            string text = string.Concat(
                first.Text.AsSpan(0, offset), citation, first.Text.AsSpan(offset + was));

            List<PageRun> runs = [.. first.Runs];
            List<PageFrame> frames = [.. first.Frames];
            List<PageNote> nested = [.. first.Notes];

            int shift = citation.Length - was;
            if (shift != 0) Shift(runs, frames, nested, offset, was, shift);

            List<PageBlock> blocks = [.. note.Blocks];
            blocks[0] = first with { Text = text, Runs = runs, Frames = frames, Notes = nested };
            return blocks;
        }

        return note.Blocks;
    }

    /// <summary>
    /// Moves everything a paragraph indexes by character offset across a replacement.
    /// </summary>
    /// <remarks>
    /// Three lists, because three things index into a paragraph's text and every one of them would put its
    /// own object in the wrong place if it were left behind: the runs decide what is drawn in which face,
    /// the frames decide where an anchored object hangs, and the notes decide which line takes a note's
    /// room. The run covering the citation grows or shrinks; the runs after it move; the runs before it are
    /// untouched.
    /// </remarks>
    /// <param name="runs">The paragraph's runs, edited in place.</param>
    /// <param name="frames">Its frames, edited in place.</param>
    /// <param name="notes">Its notes, edited in place.</param>
    /// <param name="offset">Where the replaced text began.</param>
    /// <param name="length">How long the replaced text was.</param>
    /// <param name="shift">How much longer the replacement is; negative when it is shorter.</param>
    private static void Shift(
        List<PageRun> runs, List<PageFrame> frames, List<PageNote> notes,
        int offset, int length, int shift)
    {
        for (int i = 0; i < runs.Count; i++)
        {
            if (runs[i].Start >= offset + length)
            {
                runs[i] = runs[i] with { Start = runs[i].Start + shift };
            }
            else if (runs[i].Start <= offset && runs[i].End >= offset + length)
            {
                runs[i] = runs[i] with { Length = Math.Max(0, runs[i].Length + shift) };
            }
        }

        for (int i = 0; i < frames.Count; i++)
        {
            if (frames[i].AnchorOffset >= offset + length)
            {
                frames[i] = frames[i] with { AnchorOffset = frames[i].AnchorOffset + shift };
            }
        }

        for (int i = 0; i < notes.Count; i++)
        {
            if (notes[i].Offset >= offset + length)
            {
                notes[i] = notes[i] with { Offset = notes[i].Offset + shift };
            }
        }
    }

    /// <summary>
    /// Identity rather than value equality, for keying the renumbering on the notes themselves.
    /// </summary>
    /// <remarks>
    /// <see cref="PageNote"/> is a record, so two notes of the same class citing the same number in
    /// paragraphs of the same shape compare equal — which is exactly what a document restarting its
    /// numbering is full of. What is being looked up here is "this note object", so the comparison has to
    /// be the reference.
    /// </remarks>
    private sealed class ByReference : IEqualityComparer<PageNote>
    {
        public static ByReference Instance { get; } = new();

        public bool Equals(PageNote? x, PageNote? y) => ReferenceEquals(x, y);

        public int GetHashCode(PageNote note)
            => System.Runtime.CompilerServices.RuntimeHelpers.GetHashCode(note);
    }
}

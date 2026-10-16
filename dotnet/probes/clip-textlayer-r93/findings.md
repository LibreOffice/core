# Does the drawing-layer clip take the text with it? No — and my first test said yes

Two rounds implemented the Calc drawing-layer clip independently, both landed, and they disagreed
on one point worth 25 gate verdicts:

- `ink-pass-r92` took the clip **only where a drawing leaves the block**, and as
  `ClipPathKeepingText`, on the ground that the reference keeps the covered glyphs. It measured
  **no gate movement** across its 74 movers.
- `sheet-shapefill-r92` took it **unconditionally** and let it remove text, reporting the sheets
  gate at **262 → 287** because *"the reference's counts already have the clipped text removed"*.

## The measurement

**Of 834 reference pages carrying a genuine drawing-layer clip, 733 still extract text lying
wholly outside it.** The reference's counts do not have that text removed.

**And the sheets gate at `96882554d`, which carries the conditional rule, reads `278 of 307`** —
24 `words`, 3 `pages`, 2 `pages,words`. That is exactly the figure `ods-notes-r92` measured for its
own change alone, so the clip moved no verdict in either direction, which is what `ink-pass-r92`
reported.

The `262 → 287` is not reproduced. Two things about how it was taken: it was measured at **load
15**, where `dotnet/CLAUDE.md` records that a gate run undercounts *on the reference side*; and its
**262 base disagrees by 12 rows** with `ods-notes-r92`'s independent sweep of a neighbouring commit.
An undercounted base inflates every improvement measured against it.

## The instrument warning, which is the more useful half

**My first attempt at this measurement was vacuous and said the opposite.** Searching each
reference page for the first `<x> <y> <w> <h> re W* n` and asking whether any extracted word fell
outside it gave a crisp *"197 pages examined, 0 with text outside the clip"* — which reads as
decisive support for removing the text.

It was decisive support for nothing. **All 197 of those rectangles were the page-level clip**:
median area as a fraction of the page, `1.000`; minimum, `1.000`. LibreOffice emits a full-page
clip first, so "no text outside it" is a tautology. Filtering to rectangles under 90 % of the page
found **834** real drawing-layer clips and reversed the answer.

The check that caught it took one command — print the clip area as a fraction of the page — and it
is the same check `render-comparison` already prescribes under *"check your own comparator on a
known answer first"*. **A crisp number from an unvalidated instrument is the most expensive kind.**

## Left open, and it is narrower than what was claimed

Whether a clipped **shape's own** text should leave the text layer. The 733 pages above are mostly
*cell* text, which the drawing layer's clip never governed, so they refute the general claim
without settling the specific one. A round wanting to reopen this should measure a page where the
overhanging object is a shape carrying text, and compare the reference's extracted characters for
that shape alone.

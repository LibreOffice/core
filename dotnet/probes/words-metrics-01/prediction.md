# words/metrics-01 — predictions, written before diagnosis

Written 2026-08-15 after the *baseline re-measure only* (`baseline.tsv`), before any
source reading or probe. Scored honestly at the end of `results.md`.

## What the baseline already established (not predictions — measurements)

Re-running `words/metrics-001` on `ea37e4214b6` (line-height law merged) gives
**6 of 8 failing**, the same *count* the brief carries but **not the same membership**:

| document | pages ours/ref | verdict | brief said |
|---|---|---|---|
| `A320SimNotes.doc` | 41/42 | pages | *not mentioned at all* |
| `手机免提系统TSB.doc` | 2/3 | pages,words | CJK, no wrap |
| `02_mcar_part-2_and_IS_v2.10.docx` | 314/312 | pages | mcar trio |
| `AWR OPS-AOC 044 …docx` | 12/15 | pages,words | breaks one word later |
| `FRE-03_mcar_part-3_and_IS_v2.9.docx` | 76/76 | **match** | mcar trio (failing) |
| `OM template … NCC operators…docx` | 164/165 | pages | TOC 13.80 vs 14.10 |
| `SPA-02_mcar_part-2_and_IS_v2.9.docx` | 267/266 | pages | mcar trio |
| `review-welsh-government-…docx` | 14/14 | **match** | CONFIDENTIA L, 5-line cell |

So two of the brief's named failures are already closed by the line-height merge, and one
document the brief never names is failing. **The brief's "three mcar documents, one fix
would move all three" is already refuted at the baseline**: one of the three passes.

## Predictions

**P1 — the mcar pair is not the 0.1% advance seat.** The remaining two mcar documents are
+2 of 312 and +1 of 266 pages, i.e. we set **more** pages than the reference, by 0.6% and
0.4%. A ~0.1% *advance excess* (we run wider) would push text down and could do this — but
FRE-03, byte-similar and from the same family, sits exactly on 76/76. I predict the two
残 failures are **not** closed by any change to kerning/advance, and that the surviving
gap is a small number of *local* break decisions rather than an accumulating one.
Confidence 0.6.

**P2 — the mcar pair does share one seat with each other.** Same authoring pipeline, same
verdict direction, near-identical documents. One fix moves both or neither. Confidence 0.7.

**P3 — the CJK 127% scale is NOT what closes `手机免提系统TSB.doc`.** The brief's own
description of the document is a *wrapping* failure — lines run off the right margin and
179 characters are clipped. A line-height scale changes how tall a line is, not where it
breaks. Fixing the height alone leaves the page count wrong. I predict implementing
`lcl_ApplyCjkHeightAdjustment` alone does not change this document's verdict.
Confidence 0.8.

**P4 — the CJK 127% scale costs at least one of the three currently-passing CJK
documents.** The brief frames it as "one to gain, three to risk". Since P3 says the gain
is not there, the expected value is negative and I predict I will measure it and **not
ship it**. Confidence 0.55 on the loss; ~0.75 that I decline to ship.

**P5 — `A320SimNotes.doc` is a font-resolution failure, not a metrics one.** Its font
column reads **6/10**: the reference embeds ten faces and we embed six. A document that
resolves four fewer faces than the reference is measuring substituted metrics on some of
its runs, which is the cascade rule in miniature. Confidence 0.65.

**P6 — `OM template` (164/165) is the cheapest of the six and closes with a sub-point
change.** One page in 165 after the line-height merge moved it from the briefed
13.80-vs-14.10 pt TOC gap. Confidence 0.5.

**P7 — `AWR OPS-AOC 044` (12/15) is the most expensive and I will not close it.** Three
pages short of fifteen is 20%, an order of magnitude past anything an advance-width
divergence produces over twelve pages; it is a different mechanism (the brief says every
justified line breaks one word later). Confidence 0.7 that I leave it open.

**P8 — headline.** I close **two** of the six and leave four, with `done-*` intact and
reach across the 200 at worst neutral. Confidence 0.4 on exactly two; 0.75 on "between
one and three".

## What would falsify the round

- Any fix that closes a `metrics-001` document while moving a `words/done-*` document to
  failing is not a fix; it is a trade, and must be reported as one.
- A prediction scored by grep rather than by what resolves at run time. Reach is counted
  from re-rendered output against the banked refs, never from a source search.

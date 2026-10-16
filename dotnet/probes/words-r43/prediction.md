# Words round 43 — predictions, committed before anything post-change is measured

Baseline reproduced at `cce1cc314`: **154/200**, absolute page error **78**, **164** exactly-correct
page counts, absolute word error **6512** (`baseline.tsv`). Every figure the brief carries, to the
digit.

## The scoreboard

**Nothing moves.** No file under `dotnet/src` is touched this round: the header work ends in a
refutation and a recorded deviation, and the pairing work is in `pdf-ops.py`, a diagnostic that no
rendering path reads.

| | baseline | predicted after |
|---|---:|---:|
| documents matching | 154 | 154 |
| absolute page error | 78 | 78 |
| exactly-correct page counts | 164 | 164 |
| absolute word error | 6512 | 6512 |
| renderings changed | — | **0** |

Reach is zero by construction rather than by measurement, and the check is that the corpus is not
re-rendered at all: the same `base/ours` PDFs are classified twice, by two versions of the matcher.

## The instrument, measured against round 42's oracle

`box-note-anatomy.py` is the test oracle, run over the same jobs with `$PDF_OPS` pointing first at
the main checkout's `pdf-ops.py` and then at this worktree's.

1. **Cross-orientation notes go to zero.** The new `pairable()` refuses a linear record a partner
   of the other orientation, so the note that reports one rule's width against another's height
   cannot be minted. Round 42 counted 142 of them over 44 documents.
2. **Hairline notes keep their count and change their name.** 146 were measured; they are now
   spelled `hairline` rather than `size`, and `box-note-anatomy.py` reads both spellings, so its
   own verdict column should be unchanged for them.
3. **Rule notes: 151 ± 10.** Round 42 counted 151 same-direction rule-scale survivors. This should
   move a little in both directions and I cannot predict the sign: a record whose true partner was
   stolen by a cross pair may now find it and agree (one note fewer), or find it and differ (one
   note the same), or find nothing and become one-sided (one note fewer, one one-sided record
   more).
4. **`one-sided` rises by 0–3 documents.** Every refused cross pair leaves two records unmatched
   unless a same-orientation partner is inside the window. I expect the second to be the common
   case — two renderers that both draw a table draw both of its edges — so the rise should be
   small. A large rise would say the refusal is too strict and is the result to look for.

## The dominant-class table

The brief states the target: `box` about **5 of 46 failing against 3 of 154 matching**, which is
what round 42 computed by post-filtering its own anatomy. This round gets there by fixing the
matcher instead, so the two should agree.

| dominant kind | published (r42, re-measured on my own PDFs) | predicted cleaned |
|---|---|---|
| `box` | to be measured | **5 failing / 3 matching**, ±1 either way |
| `hairline` | did not exist | a new class, mostly on matching documents |
| `glyphs` | to be measured | unchanged ±2 |
| `one-sided` | to be measured | +0 to +3 |

## What a stored measurement from before this change means afterwards

Any `box` count recorded by rounds 34, 39 or 42 is the sum of three things: same-orientation rule
differences, hairline segments of flattened curves, and cross-orientation pairs that the matcher
made and no longer makes. Only the first is still called `box`.

## Task one

No prediction to make about the scoreboard, because no rule is implemented. The prediction that
can be checked is the census: `table-only-header-census.py` says **3 of the 134 DOCX** have a
section inheriting a table-only header, and it can see nothing of the **66 `.doc`** in the track.
If a later round decides to reproduce LibreOffice's behaviour, the verdict prediction is **+1**
(`UG.CAO.00133 … Language.docx`, 3944 words against 3700, whose 244-word surplus is exactly the
head it draws on thirteen pages the reference leaves bare), and no other document in the census
can flip, because both of the others also differ in page count.

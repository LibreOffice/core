# Sheets round thirty-seven — results

Base `545550952`, verified with `git log --oneline -1` before anything was measured.

## Baseline

Swept with `SOURCE_DATE_EPOCH=1700000000` against a checksummed snapshot of the base CLI.
171 rows, no duplicate path, no `ref-failed`, no `ours-failed`.

| | brief | measured |
|---|---:|---:|
| match | 155/171 | **155/171** |
| absolute page error | 73 | **73** |
| exact page counts | 161 | **161** |
| absolute word error | 27161 | **27163** |

Three of four to the digit. The word error is two off over 171 documents, which is inside the
reference's own instability rather than a change in either program.

## Method

Each leg renders into its own directory from its own checksummed CLI snapshot, and the gate is
scored from the PDFs afterwards: `score.sh` reproduces `batch-check.sh`'s three checks against
**the same reference bytes** on both sides. Re-scoring the baseline's own renderings with it
returns 155 / 73 / 161 / 27163 exactly — the control that says the scorer and the sweep agree —
and holding the references fixed removes the reference's own instability from the comparison,
which on this track has been worth a page before.

## Result

**Nothing moved on the scoreboard, and that is the honest headline.** The final tree scores
155/171, absolute page error 73, 161 exact page counts and absolute word error 27163 — every
figure identical to the baseline, every earlier batch intact.

That was predicted, in `PREDICTION.md`, and it is a property of the gate rather than of the
change: `batch-check.sh` decides on page count, extractable words and unembedded fonts, and a
suppressed border, a recoloured fill and a grid that stops short move none of the three.

### Reach, measured by rendering

| | documents changed |
|---|---:|
| the decoration change alone | **57 of 171** — 22 `.xls`, 35 `.xlsx` |
| the whole round | **52 of 171** — 17 `.xls`, 35 `.xlsx` |
| the BIFF border transfer, against the decoration change alone | 16 `.xls` |

The BIFF step lowers the total, and coherently: of the 16 renderings it moves, **five come back
byte-identical to the baseline**. Without the transfer the origin's *interior* right and bottom
borders were being drawn on the range's outer edge, which is a change from the base and a wrong
one; with it, the outer edge is the line the covered cells used to draw, which is what the base
happened to put there.

### Whether the ink moved the right way

The gate cannot see this change, so the fourth instrument decides it. Of the 57 documents the
decoration change moves, **50 already pass the gate** and are therefore comparable page by page;
the other seven are refused by `pdf-image-diff.py` because their page counts differ.

Over those 50, total `|ink|%` against the reference goes **583.90 → 512.14**, a 12.3% reduction.

Per document it is **22 better, 16 worse, 12 unchanged**, and the asymmetry is in the sizes
rather than the counts: the largest gain is −21.80 on a base of 94.90 (`atspp_pay_tables.xlsx`)
and the largest loss is +2.07 on a base of 25.55
(`flightstandards-doc-Cross-reference-table_version02.xlsx`). Both halves are worth reporting —
sixteen documents measurably further from the reference is not nothing, and the residue on them
is a lead this round did not follow.

### Cross-track

Every line of the diff is inside `src/Paperless.Spreadsheets`, so this should reach neither
other family. Measured rather than argued, rendering each document with both snapshots and
byte-comparing: **words 0 of 200, slides 0 of 163.**

The slides sweep first reported **2 of 163 changed**, and both were measurement artefacts:
re-rendered individually, `item_3-environmental_process_briefing.pptx` and
`30-04-2021 merged NDoH and NICD_Presentation HBV BD meeting 05May2021_1.pptx` are byte-identical
between the two snapshots, and the base CLI run twice is byte-identical to itself. The sweep ran
under load average 20–35 beside three other agents; a render truncated under load leaves a file
that exists and differs, which a byte comparison reports exactly as a real change. **Re-run a
byte-reach outlier individually before believing it.**

## The prediction, and how it did

| | predicted | measured |
|---|---|---:|
| sheets renderings byte-changed | 40–80 of 171 | **52** (57 for the first change alone) |
| verdicts moved | 0, band 0–1 | **0** |
| words renderings byte-changed | 0 of 200 | **0** |
| slides renderings byte-changed | 0 of 163 | **0** |

Inside every band. The reach band was set high on the reasoning that this census's condition —
a merge with a covered cell whose style paints a border — nearly *implies* a changed rendering,
because the commonest way a file states a merge is to write the same box on every cell of it;
52 against a ceiling of 47 over the two thirds of the track a zip-level census can read is that
reasoning coming out right, with the `.xls` half riding on top of it.

## What this did not close

`7-memento-2015-transports-aeriens-b.xls` is the document the lead came from, and **the round
did not fix it.** On its page 2 the reference draws 115.28 pt verticals at x = 119.99 and
512.39 in `#0066CC`; we draw a **single 13.1 pt segment** at x = 120.02 in `#0066CC` and nothing
at all below it — identically before and after.

That is a sharper statement of the problem than round thirty-six's, and it points somewhere
else. A merged block known to the decoration path emits its left edge **once per covered row**,
each carrying the origin's style, because only an overlap in the column direction suppresses a
left edge. Eight segments is what a known merge would produce here. One segment, with the
covered rows drawing nothing, says **the block is not in our model as a merge at all** — so the
question is no longer which cell's colour wins but why that `MERGEDCELLS` range does not reach
`StatedMerges` for this sheet. Not investigated further, and labelled unmeasured.

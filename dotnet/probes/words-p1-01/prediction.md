# words-p1-01 — prediction

Written after re-measuring the baseline (which the brief requires as step 1) and **before any
diagnostic probe of my own**, before opening any of the seven documents, and before any change
to the tree. Committed before the first measurement of a fixed binary.

## The baseline I measured, not the one I was given

Rendered all 200 `words` documents with a binary built from this worktree's HEAD
(`886bcde7091`), `SOURCE_DATE_EPOCH=1700000000`, verdicts taken against the banked references at
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/` with `probes/lineheight-01/verdict.py`.

| | measured |
|---|---|
| `words/pagination-001` | **3 match of 10** |
| `words/done-*` | **158 match of 159** |
| whole `words` track | **173 match of 200** |

The seven failing, exactly as briefed — nothing in this group moved under the line-height or
reference-device merges:

```
24-25_FAA_Holdover_Tables__docx        154/155    -1
A_320__doc                             141/118   +23
ESPN-R - MCF - RA - Ed1__docx           59/58     +1
FAA 2025-26 Holdover Tables__docx      185/167   +18
absrc-pac-01-info-note-en__doc            6/7     -1
report-template__docx                    19/20    -1
template---tpr-…-with-guidance__docx      8/7     +1
```

The one standing `done-*` mismatch is `airbus-pdf-information-package_v1-4.docx`, 1272 words
against 1299, exactly as the brief says.

## Predictions

| | claim | conf |
|---|---|---:|
| P1 | The Holdover `TABLE ADJ-28` +2.88 pt step is a **block** measured once per table, and the block is the repeated header — not a per-row rule over-applying | 45% |
| P2 | The two Holdover documents are one defect with one fix: whatever moves `FAA 2025-26` toward 167 also moves `24-25_FAA` off 154 | 55% |
| P3 | `FAA 2025-26 Holdover Tables.docx` lands within ±3 pages of 167 | 35% |
| P4 | `A_320.doc`'s near-empty pages are a **table-fits-the-page** test made against the room left below the header rather than against the body height, so a one-page table is pushed whole | 50% |
| P5 | `A_320.doc` lands within ±5 pages of 118 | 30% |
| P6 | At least two of the seven close outright | 45% |
| P7 | No `done-*` document loses its verdict | 70% |
| P8 | Fidelity is no worse than 30 of 550 | 60% |
| P9 | Reach across the 200 is more than 20 renderings changed | 60% |
| P10 | At least one of the four documents with only first-divergence notes (`absrc-pac`, `report-template`, `template---tpr`, the rest of `ESPN-R RA`) is **not** attempted this round | 80% |
| P11 | The `±1` documents (`24-25_FAA`, `ESPN-R RA`, `absrc-pac`, `report-template`, `template---tpr`) are five different causes, not one | 65% |

## What would falsify the headline

If the Holdover step turns out to be a per-row drift after all — a residue that grows with row
count rather than arriving at one table — then P1 is wrong and the previous round's
characterisation of it as "one block" was wrong too, and the whole target is much worse
conditioned than briefed.

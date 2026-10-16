# Proportional line spacing: what the percentage is taken *of*, and who keeps the last one

Measured 2026-08-15 against the installed LibreOffice **26.2.4.2** with Carlito, Caladea,
Liberation and DejaVu present (`fc-match "DejaVu Sans"` → `DejaVuSans.ttf`). Every figure below
is read out of the two PDFs' own text matrices with
`render-comparison/scripts/pdf-ops.py dump … --only text`, never from a raster.

Both probes exist because the obvious reading of `w:line="288" w:lineRule="auto"` — *multiply the
line height by 1.2* — agrees with LibreOffice on every line where the base height and the line
height are the same number, which is nearly every line, and disagrees on the two shapes below.

## 1. `mk.py` — a blank run is in the base and out of the line height

Six paragraphs per group in a 10 pt Arial style at `w:line="288" w:lineRule="auto"` with
`w:contextualSpacing`, so the baseline-to-baseline pitch is the line height plus the proportional
gap and nothing else. `docDefaults` names Calibri 11 pt. Only the paragraph mark's `rPr` and one
interior run vary.

| group | what varies | line height | reference pitch | implied base |
|---|---|---:|---:|---:|
| A | nothing | 11.50 | 13.80 | 11.50 |
| B | paragraph mark `rPr` = Calibri 11 pt | 11.50 | 13.80 | 11.50 |
| C | paragraph mark `rPr` = Arial 11 pt | 11.50 | 13.80 | 11.50 |
| D | paragraph mark `rPr` = Arial 20 pt | 11.50 | 13.80 | 11.50 |
| E | a Calibri 11 pt run holding **text** | 13.43 | 16.10 | 13.43 |
| F | a Calibri 11 pt run holding **one tab** | 11.50 | **14.15** | 13.43 |
| G | a Calibri 11 pt run holding **one space** | 11.50 | **14.15** | 13.43 |
| H | a Calibri 22 pt run holding one tab | 11.50 | **16.85** | 26.86 |
| I | an Arial 20 pt run holding one tab | 11.50 | **16.10** | 23.00 |

Three conclusions, and the first two are the ones that cost a round each:

- **The paragraph mark's `rPr` does not enter the line height or the base** (B, C, D are A).
  This is the third refutation of the paragraph-mark hypothesis on this document and it is
  now measured rather than argued.
- **A blank or tab run is transparent to the line height and opaque to the base** (F, G, H, I).
  The two exclusions are separate rules with different membership: `#i3952#` /
  `IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION` decides `SwLineLayout::Height`, and
  `m_nLineSpacingBaseHeight` is a second maximum with its own test.
- **The gap is `(prop − 100)% × base` added to the line height, not `prop × line height`.**
  Scaling gives 13.80 for all nine rows. Every implied base above is the blank run's own line
  height to the twip: Carlito 11 pt is 268 twips and 268/5 = 53 twips = 2.65 pt.

This is what a contents entry costs. `OM template for non-complex NCC operators_August 2016.docx`
sets each `TOC4` line with a `minorHAnsi` 11 pt run holding the tab between the number and the
title, so its contents pitch is 14.15 and drawing it at 13.80 fitted 83 entries per page where the
reference fits 79.

## 2. `mkhdr.py` — a running head keeps its last paragraph's gap

A two-paragraph header whose second paragraph is empty, `w:top` 720 and `w:header` 709. The body's
first baseline reports the header band's height directly.

| paragraph mark | `w:line` | reference | this engine, before |
|---|---:|---:|---:|
| 10 pt | 240 (100%) | 774.04 | 774.05 |
| 10 pt | 480 (200%) | **762.54** | 774.05 |
| 12 pt | 240 (100%) | 771.74 | 771.75 |
| 12 pt | 360 (150%) | **764.89** | 771.75 |
| 12 pt | 480 (200%) | **757.99** | 771.75 |
| 20 pt | 480 (200%) | **739.54** | 762.55 |

Every reference row is the header plus that last paragraph's own proportional gap. Every "before"
row is the header without it — it moves with the size and not with the spacing.

`mkhdr.py` also authors a `t*` pair with a further paragraph *after* the empty one, and those
matched before the fix: inside a flow the gap is unambiguous, because whether it is charged to the
line above or to the line below puts the same distance in the same place. Only at the flow's
**end** do the two answers differ, and there they differ by the whole gap. `mkbody.py` is the
control: the same empty paragraph in the body rather than in a header matched at 100%, 200% and
two sizes before the fix and after it, because a body paragraph always has the page break below it
rather than a frame edge.

It is 13.75 pt per page on `OM template`, whose running head ends with an empty 12 pt paragraph at
`w:line="480"`.

## Reach

`words/done-*`, 161 documents, before and after: **159 match / 2 mismatch, and not one row of the
TSV changed** — same pages, same words, same verdicts. The two failures
(`airbus-pdf-information-package_v1-4`, words 1269/1299) are identical before and after and
predate this round.

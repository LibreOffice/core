# words/regress-01 — prediction, written before the fixes and before any sweep

Round `words-regress-01`, 2026-08-15, worktree `wt-w-regress`. Reference LibreOffice **26.2.4.2**,
`fc-match "DejaVu Sans"` → DejaVu, Carlito/Caladea/Liberation all present, `libreoffice-math` still
absent. Fidelity baseline re-established before anything was touched: **Failed 30, Passed 520,
Skipped 0, Total 550** — the briefed number exactly.

Two `words/done-015` documents regressed when two correct fixes each removed an error that had been
cancelling another. The brief is that what they exposed is latent and older than today. Both seats
below were found by measurement before this file was written; what is predicted is what fixing them
does, which is the part not yet known.

## Seat 1 — `Sample_SQMS_Program.docx`, 60 pages against 61

Not a uniform shortfall. Pages 1–58 are line-for-line identical; the whole divergence is **1.30 pt
of row height accumulated on page 59**, which lets a four-line follow part fit where the reference
takes three. Read out of the two PDFs' own row rules:

| page-59 row | ours | reference | Δ |
|---|---:|---:|---:|
| repeated header row | 64.00 | 64.30 | **−0.30** |
| follow part of the row split from page 58 | 15.30 | 16.30 | **−1.00** |
| every other row on the page (30.10 / 43.90 / 18.30) | equal | equal | 0 |

Two independent causes, both measured on authored probes through the installed 26.2.4.2:

**(A) A split row's follow part does not carry the paragraph's `w:spacing w:before`.**
`probe-rowsplit-spacing.py` sweeps the two spacings independently over a row cut across a page.
The reference's follow part is `before + remaining lines + after + border` in all eight
combinations; ours is that less `before`, exactly, every time — 0.00, 1.00, 0.00, 1.00, 2.00,
0.00, 2.00, 5.00 pt short for `before` = 0, 1, 0, 1, 2, 0, 2, 5 pt. `probe-rowsplit.py`'s `solo`
variant — one cell, no siblings — refutes the competing reading that an empty sibling cell is
being re-laid-out on the follow part. `probe-rowsplit-paras.py` shows the rule is not special to a
mid-paragraph cut: a follow part that *begins* a fresh paragraph carries that paragraph's
space-before too. So the rule is uniform — **the top of a follow part is the containing block's
top, not its first line's top** — and this is Writer's `AddParaSpacingToTableCells` compatibility
behaviour, which the DOCX importer switches on and which this tree already honours at the *bottom*
of a cell (`PlacedFlow.Advance`) and not at the top of a follow.

**(B) Our line height is one twip short at some sizes.** `probe-lineheight.py`, 195 (font, size)
pairs across Liberation Serif/Sans, Carlito, Caladea and DejaVu Sans at every half-point from 5 to
24 pt: we agree with 26.2.4.2 on 174 and are **0.05 pt short on 21**, never long. Liberation Serif
at 10 pt — the size the SQMS header row is set in — is 11.55 in the reference and 11.50 here, and
four line gaps plus a descent is the 0.30 pt above.

### What is predicted

- **P1 (75%).** Fixing (A) alone flips `Sample_SQMS_Program.docx` back to 61 pages. Our four-line
  follow part ends at 81.70 pt; the reference refuses one ending at 80.15 and places one at 81.65,
  so +1.00 should carry it over the same edge. *Refuted if the page count stays 60.*
- **P2 (60%).** It is still a knife edge and I will say so: the margin is under half a point, so
  P1 could be right for the wrong reason. Falsification test — the *reference's own* page-59 last
  row must be reproduced row for row after the fix, not merely the page count.
- **P3 (85%).** (B) is **not** fixed this round. It is a font-metric rounding that moves every
  document in the corpus, it is the single highest-risk area named in `dotnet/CLAUDE.md`, and I
  could not reconstruct 26.2.4.2's rounding chain from the 195 measurements — no plausible device
  resolution reproduces all of them. It is recorded with its probe and left open.
- **P4 (65%).** Fix (A) changes fewer than 20 of the 200 words renderings. Only a table row that
  actually splits across a page and whose paragraph states a space-before can move.

## Seat 2 — `airbus-pdf-information-package_v1-4.docx`, 1272 words against 1299

The 59-word page-9 shortfall is not a page-9 defect. It is the repeated header row of the invoice
mapping table being **one line shorter in our rendering on every page it repeats**, which lets one
extra body row fit per page from page 6 onward and leaves page 9 holding only the contact table.

Confirmed in the operators, not in a raster: on page 6 the reference draws the header row in
`LiberationSans-Bold` and wraps `Mapping` / `ID` onto two lines and column 4 onto four; we draw it
in `LiberationSans` regular, fit `Mapping ID` on one line and column 4 on three.

**The seat: `w:tblStylePr` is read by nothing.** `grep -rn "tblStylePr\|cnfStyle\|tblLook" dotnet/src`
returns nothing at all — conditional table-style formatting is unimplemented, not mis-mapped. The
table names `PlainTable1`, whose `<w:tblStylePr w:type="firstRow"><w:rPr><w:b/>` is what makes the
header bold, and `w:tblLook w:firstRow="1"` switches it on. The document supplies its own proof:
the run holding `(do not change!)` carries an explicit `<w:b w:val="0"/>`, which is only meaningful
if something above it is turning bold on.

Census over the whole track, by parsing every part of every file rather than grepping: **14 of the
134 DOCX files declare `w:tblStylePr`; 7 of them name such a style from a table**, and every one of
those 7 has a `firstRow` layer carrying `w:rPr` with `w:tblLook w:firstRow="1"`. That is the reach
ceiling, fixed before anything is measured. Four are `done-*` (regression risk), two are
`pagination-002` failures (possible gain), one is `done-003`.

Scope: **run properties only** — the `w:rPr` of the style itself and of its `firstRow`, `lastRow`,
`firstCol`, `lastCol` and four corner-cell layers, in ECMA-376 §17.7.6 precedence, gated by
`w:tblLook`. Deliberately not done: the `w:tcPr` half (shading and borders) and the `w:pPr` half,
and the band layers — no used style in this track carries `w:rPr` on a band, so implementing them
would be reach I cannot measure.

### What is predicted

- **P5 (70%).** `airbus-pdf-information-package_v1-4.docx` flips to `match`. Its band is 25.98 and
  it is at −27; making the header row four lines tall on pages 6–9 should restore the reference's
  row distribution and most of the 59 words on page 9. *Refuted if the deficit stays worse than −26.*
- **P6 (40%).** Its page count stays at 9. The header row growing by one line could push the table
  onto a tenth page; the reference has 9, so this is the risk.
- **P7 (50%).** At least one of the two `pagination-002` documents (`150-5370-10H.docx`,
  `AC-150-5370-10G-updated-201604.docx`) moves its page count. Neither is predicted to *pass* —
  both are large pagination failures with other causes.
- **P8 (55%).** No `done-*` document regresses. The exposure is the four `done-*` documents that
  resolve a conditional layer; `mde087077~283.docx` is the one I am least sure of, because its
  `firstRow` layer also carries a `w:pPr` that this round does not apply.

## Across both

- **P9 (80%).** Fidelity stays exactly **30 failed of 550**, same tests, none gained or lost.
- **P10 (70%).** Every other test project stays at zero failures, and `XRay.Office.WordProcessing`
  gains only the new tests.
- **P11 (60%).** The two fixes together move **10–35 of the 200** words renderings and the track's
  `match` count does not fall.
- **P12 (75%).** The new tests fail against the unfixed tree — the split-row ones because the
  follow part is short by the space-before, the table-style ones because nothing reads
  `w:tblStylePr` at all.

## The falsification test I am writing for myself

If more than two `done-*` documents change verdict in either direction, the conditional-table-style
layer is reaching further than the census says it can, and the census is wrong rather than the
implementation being lucky. If `Sample_SQMS_Program.docx` stays at 60 pages, seat 1 is not closed
by (A) alone and (B) is load-bearing after all — in which case I will say the document could not be
closed rather than tune anything to reach 61.

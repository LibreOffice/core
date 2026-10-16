# sheets-sil-01 — `SIL_TDB648.xlsx`, 89 pages against 90

Measured 2026-08-15 against LibreOffice **26.2.4.2** with `fonts-dejavu-core` present, corpus at
`/c/sandbox/workdir/sample-files`, reference bank `refpdfs-26.2.4.2-fonts/`.

`SilProbe` prints, per sheet, the used range, the printed range, every page placement pagination
produces and which of them survive the empty-page pass. `LASTROWS='Sheet=N;…'` forces a printed
last row so the effect of a wider print area can be measured without changing the reader;
`ROWSTARTS=1` dumps the row and column offsets; `DRAWROWS=3,53` compares individual heights.

## What it found

The document is four errors that cancel to one, and they are two defects:

| region | before | reference | after |
|---|---:|---:|---:|
| blank last row band of `TerrDB Verification`, both column bands | 0 | 2 | 2 |
| 4th row band of `RAAS`, column bands 0 and 1 | 0 | 2 | 2 |
| column band 1 of `RUNWAYS`, row bands 1-3 | 3 | 0 | 0 |
| total | 89 | 90 | 90 |

**1. A drawing's bounding rectangle is not its anchor.** Each of the ten sheets carries a group of
seven watermark pictures turned 27°. `ScDrawLayer::GetPrintArea` and `ScDocument::HasAnyDraw` both
ask `GetCurrentBoundRect`, which for such a group is the union of the *turned* boxes: 4.2% lower
than the group's frame and 1.1% short of its right edge. The turn has to be applied after the frame
has scaled the parts, not before — the other order makes each watermark 197 pt tall where the
reference draws it 255 (and the right order 252).

Checked against LibreOffice's own answer by exporting the workbook to flat ODF and reading the
`table:end-cell-address` it wrote for each of the ten groups: **10 of 10 columns and 9 of 10 rows
exact**, the tenth one row out.

**2. The optimal-height pass covers the drawing area, not the cells.**
`ScDocRowHeightUpdater::updateAll(bOnlyUsedRows=true)` takes its last row from
`ScDocument::GetPrintArea` (`dociter.cxx:1731-1734`), which is maxed against the drawing layer. So
`RAAS`, whose cells end at row 33 and whose watermark reaches row 148, has every empty row between
measured: LibreOffice's flat-ODF export gives rows 53 to 148 a height of **0.1756in = 253 twips**
where we kept the file's stated 12.6 pt default, snapped by the MSO 0.75 pt rule to **240**.
Thirteen twips a row over a hundred rows is a whole band of pages.

## The separate picture defect, and its cause

The cover photograph on `General Info` rendered **4.7% shorter** than the reference's, top-anchored
on a `twoCellAnchor` spanning rows 0→17. The cause is `editAs="oneCell"`, which the reader did not
read and whose doc comment asserted "would change nothing that is drawn". Calc takes the shape's own
`a:ext` for such an anchor and ignores the second corner (`drawingfragment.cxx:284-295`). Measured
off the PDFs' image transforms: reference 300.73 pt, ours 286.55 before and **300.73 after**;
the stated `a:ext` is 3819475 EMU = 300.75 pt.

It is *not* the same root cause as the pagination gap, and the arithmetic says so: the anchor gives
286.6 pt and the stated extent 300.75, a 4.9% gap, while the group anchors on the other sheets are
resized to the live grid — confirmed by measuring the spacing of the watermark repeats in the
reference PDF, six gaps agreeing on 1.0249 to three decimals against a live-grid prediction.

## Measurements worth keeping

- The reference's `RAAS` row bands, read off the watermark repeats across pages 23-26 of the
  reference PDF: 12739, 12650, 12651 twips against our 12690, 12660, 12720 — the band structure
  agrees, which is what ruled out a page-height error.
- The reference's drawn row pitch on `RAAS` page 1 is 12.6425 pt in both renderings, so the *live*
  row heights were never in question; only the drawing's rectangle and the empty rows below the
  cells were.

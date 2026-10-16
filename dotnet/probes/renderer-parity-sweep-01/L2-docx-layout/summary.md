# L2-docx-layout — summary

Reference for this sweep is **LibreOffice 24.2.7.2** (`/data/bench/lo/*/out.pdf`, PDF producer
"LibreOffice 24.2"; `soffice --version` on this container agrees). Two of the four biggest faults
below are rules the tree implements correctly for **26.2.4.2** and that 24.2.7.2 does not have.
Applying those two patches is therefore a *reference-version decision*, not a plain bug fix —
read `findings.md` §A and §B before sequencing them.

## A · A `w:trHeight` floor is raised by the row's border and cell margins; 24.2 raises it by neither
- `dotnet/src/XRay.Office.WordProcessing/Layout/TableLayouter.cs:199-206`; patch `patches/row-height-floor.diff`.
- **23 of 28** documents contain `w:trHeight` rows; it changes every one. Dominant on 009, 026,
  057, 058, 069, 141, 061, 093, 117, 151, 160, 041, 044, 154 and drives the page counts of 015, 018.
- Re-run of this project's own probe against 24.2.7.2: LO reads 24.00 pt at `w:sz` 0/4/8/16/24
  against a 24 pt floor where we read 24.00/24.50/25.00/26.00/27.00. Four graph-paper grids lay
  out at their declared row sums to 0.05 pt while ours run one border per row over.
- **Confidence: high** on the measurement, **medium** on whether the project wants to track 24.2.

## B · A `nextPage` section break keeps its space-before at every compatibility mode; 24.2 collapses it at mode ≥ 15
- `dotnet/src/XRay.Office.WordProcessing/Layout/Paginator.cs:1395-1402`; patch `patches/section-break-space-before.diff`.
- Costs 20 pt at the top of every section-opening page: 36 sections each in 015 and 018, 83 in 185,
  10 in 093, 6 in 063, 5 in 117, 4 in 037.
- Nine synthetics on 24.2.7.2 (plain / landscape / `w:titlePg` × mode 15 / 12 / none): LO 72.03 at
  mode 15 and 92.03 otherwise; we read 92.44 in all nine.
- **Confidence: high** on the measurement, **medium** on the version question, as A.

## C · `w:hideMark` on every cell of a blank row makes `w:trHeight` exact
- Reader-side: `Ooxml/DocxLayoutSource.Tables.cs` must set `PageTableRow.HasExactHeight`. **Cross-lane.**
- 009 and 026 only — but they are the two worst pages in the lane (SSIM 0.438 / 0.512). Ablation on
  026 through `soffice`: baseline 265.50 pt (= the declared sum exactly), `w:hideMark` deleted 417.30,
  `w:hideMark` on half the cells 417.30, the non-breaking space replaced by `X` 417.30.
- **Confidence: high** for these two documents; the emptiness test has an unexplained residual.

## D · Table position: `w:tblpX` is never read, and an absent `w:tblInd` at mode ≤ 14 skips the cell-padding subtraction
- Reader-side: `Ooxml/DocxLayoutSource.Tables.cs` `LeftEdge`/`Table`. **Cross-lane.**
- 191 (35.05 pt too far right = 29.7 of `w:tblpX` + 5.4 of the padding rule), 154, 160.
- Probe on 24.2.7.2: at modes 12 and 14 a table with no `w:tblInd` sits one cell padding left of the
  margin (66.60 pt) where we put it at 71.75; an **absent** `compatibilityMode` behaves like mode 15,
  not like 12.
- **Confidence: high.**

## E · A dxa `w:tblW` is ignored whenever every `w:gridCol` states a width
- Reader-side: `Ooxml/DocxLayoutSource.Tables.cs` `Fit`. **Cross-lane.**
- 026 only, and exact: LO draws 704.10 pt = `w:tblW` 14081 twips; we draw 708.70 = the grid sum 14174.
- **Confidence: high.**

## F · Anchored shapes are painted in the reverse of LibreOffice's order
- 024, whose page is 52 anchored shapes and no table. All eight page-1 shapes are drawn and placed
  correctly; the paint orders are exact reverses, so our last shape — a full-page dark rectangle —
  buries the page. `wp:anchor/@relativeHeight` is not read anywhere in the tree.
- Seat is split: ordering lives in `Layout/FrameLayout.cs` + `Layout/PageDrawing.cs` (mine), the key
  in `Ooxml/DocxFrames.cs` (**cross-lane**). No patch — a Layout-only half would be inert.
- **Confidence: high** on the observation, **low** on the mechanism.

## Not this lane
- **097, 140, 121** — page geometry identical to 0.1 pt on every compared page; the whole difference
  is one word crossing a break. **L1 (advance divergence).**
- **094** — confirmed `lo-broken`: LibreOffice drops the timeline's first column, its week bands and
  its first data row; ours is correct. **154** is also `lo-broken` for its title.
- **020** (style colour resolved to the template's blue), **063** (masthead title missing) — content
  and colour, not geometry.
- **011, 037** — real vertical/indent defects I did not seat; see `findings.md` §G.

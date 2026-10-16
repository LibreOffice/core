# sheets/pagination-001 — prediction

Written and committed **before** any post-change measurement. Diagnosis measurements (probe
workbooks rendered through the installed 26.2.4.2, and the pre-change corpus sweep) were taken
first and are stated here as established fact; everything under "Predictions" is a claim about
what the change will do and is scored honestly in `results.md`.

Baseline, measured on this tree before any edit, reusing the banked reference PDFs at
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/` with `SOURCE_DATE_EPOCH=1700000000`:

```
sheets track   TOTAL 171  MATCH 156  MISMATCH 15
sheets/done-*  TOTAL 156  MATCH 156  MISMATCH   0
```

The five in the group, before:

| document | ours/ref pages | ours/ref words | verdict |
|---|---|---|---|
| ODs-February-2022-Airbus-Commercial-Aircraft.xlsx | 154/175 | 15511/15715 | pages |
| CIS_Debian_Linux_8_Benchmark_v1.0.0.xls | 109/88 | 7541/8981 | pages,words |
| sectors-defense-and-aerospace.xlsx | 227/449 | 23037/22997 | pages |
| grants-2005.xls | 219/201 | 34032/34036 | pages |
| SIL_TDB648.xlsx | 89/90 | 7499/7500 | pages |

Those reproduce the briefed figures exactly, which is what validates the harness.

## What was established by measurement

### A. Paper size — an unknown `paperSize` index drops the orientation too

17 one-cell probe workbooks varying only `<pageSetup>`, rendered through the installed
26.2.4.2, plus a 136-point sweep of `paperSize="0".."135"` all at `orientation="landscape"`:

* Indices LibreOffice knows resolve normally and the landscape flag swaps them.
  `paperSize="9"` → 841.89 x 595.30; `paperSize="8"` → 1190.55 x 841.89.
* Indices it does **not** know — `0`, `48`, `49`, `71`–`74`, `77`, `84`–`87`, `91`–`135` —
  fall back to the locale default paper **and the `orientation` attribute does not rotate it**:
  every one renders 595.304 x 841.89, A4 *portrait*, despite asking for landscape.
* `usePrinterDefaults="1"` does the same thing to a *known* index: `paperSize="8"` or `"9"`
  with `orientation="landscape"` both render A4 portrait.

Airbus states `paperSize="121"` on nine of its thirteen sheets. 121 is outside the DMPAPER
enumeration (which ends at 118). We resolve it to the A4 fallback and then **swap it**, so we
emit A4 landscape where the reference emits A4 portrait. Measured on our current output: 154
pages = 6 A4-portrait + 5 A3-landscape + **143 A4-landscape**; the reference is 170 A4-portrait
+ 5 A3-landscape. The five A3 pages agree on both sides, which is what rules the metrics out.

Across the sheets corpus the only `paperSize` indices used at all are 1, 5, 8, 9, 17 and 121,
so extending the table beyond its current 0–18 buys nothing measurable and is not being done.

### B. Column width — the digit-width carry constant is one notch too high

`sectors-defense-and-aerospace.xlsx` has no `<pageSetup>`, no `<pageMargins>` and one
`<col min="1" max="300" width="40"/>`; its default font is **Calibri 12**. Column width is
`width x digitWidth`, with no padding term (confirmed: `width="10"` and `width="20"` give
exactly 61.0015 pt and 122.003 pt).

A 205-point sweep — 5 faces x sizes 6.0..26.0 in 0.5 pt steps, each read out of the reference
PDF as a filled cell's rectangle — establishes that LibreOffice's digit width is always a whole
number of twips, and that no simple rule reproduces every point. But over the **17 default-font
configurations the sheets corpus actually uses**, a single constant does exist. Writing the rule
as "truncate unless the fraction exceeds `c`":

| configuration | exact twips | LibreOffice | constraint |
|---|---:|---:|---|
| Carlito 11 (65 docs) | 111.5039 | 111 | `c >= 0.5039` |
| Liberation Sans 12 | 133.4766 | 133 | `c >= 0.4766` |
| Liberation Sans 11 | 122.3535 | 122 | `c >= 0.3535` |
| DejaVu Sans 10 | 127.2461 | 127 | `c >= 0.2461` |
| **Carlito 12 (7 docs)** | **121.6406** | **122** | **`c < 0.6406`** |
| DejaVu Sans 12 | 152.6953 | 153 | `c < 0.6953` |
| DejaVu Sans 11 | 139.9707 | 140 | `c < 0.9707` |

Window: **`0.5039 <= c < 0.6406`**. The current constant is **0.67**, just outside it, and the
only corpus configuration it gets wrong is Carlito 12 — where we compute 121 twips and
LibreOffice computes 122. Forty columns wide that is 2 pt, and it is the whole 227-vs-449 gap:
A4 usable width is 487.73 pt, two reference columns are 488.07 pt (one column per page) and two
of ours are 484.0 pt (two columns per page).

**Plain round-half-up was tried and rejected.** It scores better on the uniform sweep
(194/205 against 172) but it gives Carlito 11 → 112, and Carlito 11 is the default font of 65
corpus documents. It would have broken 51 passing documents to fix 6.

Chosen value **0.57**, the midpoint of the window (0.5722), which also scores 190/205 on the
independent uniform sweep against the current constant's 172.

## Predictions

1. **Airbus goes to 175/175 pages and matches.** Page sizes become 170 A4-portrait + 5
   A3-landscape. Confidence **high** — the mechanism is measured on both sides and the A3 pages
   already agree. Residual risk: the row-per-page count follows from the portrait height and
   the brief says the row pitch already agrees to a tenth of a point, so 48 rows per page should
   follow; if the page count lands near but not on 175 the pitch claim is what to re-check.

2. **sectors-defense-and-aerospace goes to 449/449 pages and matches.** Confidence
   **medium-high**. The column arithmetic is knife-edge — 488.07 against 487.73 pt, a margin of
   0.34 pt — so this prediction is genuinely sensitive to our margin computation being right to
   better than a third of a point. If it lands on 227 still, the margin is the thing to measure,
   not the digit width.

3. **Exactly one other configuration class moves, and it is the seven Calibri-12 documents.**
   Every other corpus configuration computes the identical digit width at 0.57 and at 0.67, so
   no other document can move for this reason. Of the seven, five currently pass. Confidence
   **high** that no non-Calibri-12 document moves; confidence only **medium** that all five
   passing Calibri-12 documents keep passing, since their column widths change by 1 twip and
   they are currently passing with a width we now believe is wrong.

4. **`sheets/done-*` stays at 156/156.** Confidence **medium**. This is the prediction most
   likely to be wrong, and prediction 3 says exactly where it would break.

5. **Documents 2 and 4 (CIS_Debian, grants-2005) are NOT fixed by either change.** Neither
   states an out-of-range paper size and neither is Calibri 12. Confidence **high**.

6. **On whether 2, 3 and 4 share a seat: they do not.** Document 3 is a column-width defect
   (the printed block is the wrong width because one column is 1 twip too narrow); documents 2
   and 4 are blank-page-count defects at the far end of an over-extended print area. The brief
   grouped them as "how wide the printed area is computed and how many empty pages that buys",
   and for document 3 the answer is that the width is wrong for a reason that has nothing to do
   with the print area's extent. Confidence **high** for 3 being separate; whether 2 and 4 share
   a seat *with each other* is still open at the time of writing.

7. **Neither change moves any word count materially**, because neither alters what is drawn,
   only where the page boundaries fall. Small movements are expected where a cell's text is
   clipped differently at a column edge. Confidence **medium**.

## What is not being attempted

* **SIL_TDB648.xlsx** (89 against 90, one missing blank page at page 17) — not attempted.
* **The opaque EGPWS photo painted over SIL_TDB648's page-1 text** — a z-order defect; logged,
  not fixed.
* **`Courier` resolving to a narrower face than the reference's** — found while enumerating the
  corpus's default fonts and unrelated to this round: for `Courier` at 10 pt LibreOffice's digit
  width is 127 twips (the same answer it gives for `MS Sans Serif`, `Helv` and `Roboto Regular`,
  i.e. its generic fallback) where ours is 120 (a real monospace face). One corpus document.
  Recorded, not fixed.
* **Extending `ExcelPaperSizes` past index 18.** The full 0–135 map is measured and recorded in
  `results.md`, but no corpus document reaches it.

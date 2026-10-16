# Sheets rebase-01 — results

Reference binary in this container: **LibreOffice 26.2.4.2 620(Build:2)**.
Every stored sheets figure was taken against **24.2.7.2**.

Scratch, scripts and raw output:
`/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/sheets/`

> ## Read this first — two confounds, one fatal
>
> The container differed from the environment the stored figures were taken in **in two ways,
> not one**: the LibreOffice version *and* a missing `fonts-dejavu-core`. DejaVu sits ahead of
> WenQuanYi Zen Hei in the fallback chain and 267 of the 534 reference PDFs fell back to
> WenQuanYiZenHei, so a large part of the corpus rendered with the wrong metrics.
>
> **Every ours-vs-reference number in this document is therefore WITHDRAWN**, including the
> `135/171` scoreboard and the reference movement table. They are recorded below with their
> method, because the method is sound and worth re-running, but they are not results.
>
> **What survives the confound and is a result:**
>
> * §4 — the `7-memento` diagnosis. It rests on the `.xls` bytes and on reading two source
>   trees. Border colour is not a font property.
> * §5 — the column-fit re-check. Authored minimal sheets, `Liberation Sans` named explicitly,
>   reference-only.
> * §2 — the corpus fact that two of the five page-split cluster names are not files.

---

## 1. The prediction, committed before measuring

`prediction.md` in this directory, written before `ref-baseline.sh` ran, before the `.xls`
bytes were opened and before our reader was read. Scored:

| # | Prediction | Outcome |
|---|---|---|
| P1.1 | ≥168 of 171 render without failure | **right** — 171/171, zero ref-failures |
| P1.2 | prior "16 changed / 305 total \|Δ\|" reproduces *approximately*, not to the digit | **wrong** — reproduced **exactly**: 16 and 305 |
| P1.3 | 227→449 is real, not a load artifact — betting against the brief | **right** (deterministic); *cause* now unestablished |
| P1.3a | 449 ≈ 2×227 less a small remainder, a column-band split | **right** — 449 = 2×227 − 5, and width-driven (§3) |
| P1.4 | 109→88 and 220→201 reproduce to the digit | **right** — both exact |
| P1.5 | no complete stored per-document reference table exists | **wrong** — `probes/sheets-r40/base-whole-track.tsv` has all 171 |
| P2.1 | the `MERGEDCELLS` range is present in the stream | **wrong** — the sheet has **zero** `MERGEDCELLS` records |
| P2.2 | the defect is in our BIFF reader (~65/35) | **wrong** — the reader is correct; the defect is in the decoration path |
| P2.3 | flagged that my own P2.2 and the 1-segment arithmetic disagreed, and that P2.3 was better evidenced | the flag was right; the mechanism I guessed was not |
| P4.1 | the column-fit refutation holds (~70%) | **right** |
| P4.2 | row height responds to font size; column width does not respond to content | **right**, both directions measured |

The brief's dominant pattern held again, twice over: P1.2 said the stored figure would only
roughly reproduce and it reproduced **to the digit**; and the sentence attached to the Task 2
figure was wrong at its premise.

---

## 2. The reference baseline — method sound, numbers WITHDRAWN

`ref-baseline.sh /c/sandbox/workdir/sample-files 'sheets/batch-0*' … 6` completed in a single
foreground pass: **171 documents, 171 rendered, 0 `ref-failed`**. Output
`refbase/ref-baseline.tsv`, header naming the binary.

Against `probes/sheets-r40/base-whole-track.tsv` (the stored 24.2.7.2 reference column, 171
rows, complete — every document had a stored value to compare against, contra P1.5):

* **16 of 171 reference page counts changed, total |Δ| = 305.** The prior session's coarse
  figure reproduced *exactly*, both numbers.
* 121 of 171 reference **word** counts changed and 32 of 171 reference **font** counts changed
  (31 of the 32 downward by exactly one).

That last pair is what should have raised the alarm and did not: a font-count change of −1 on
32 documents is a *font-set* signature, not a version signature. It is now explained by the
missing DejaVu, and it means the 16/305 page figure is **a version effect and a font effect
mixed**, not a version effect. Withdrawn.

The largest movers, for the re-run to check against:

| document | stored ref (24.2.7.2) | measured here | Δ |
|---|---:|---:|---:|
| `sectors-defense-and-aerospace.xlsx` | 227 | 449 | +222 |
| `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` | 109 | 88 | −21 |
| `grants-2005.xls` | 220 | 201 | −19 |
| `afn-afn-20250801-fy25-jan25-mar25.xlsx` | 270 | 282 | +12 |
| `SIL_TDB648.xlsx` | 88 | 78 | −10 |

### Stored reference page counts I could and could not find

**Could:** all 171, in `dotnet/probes/sheets-r40/base-whole-track.tsv`, in `ours/ref` form.
No document lacked a stored value; no document was measured that had none.

**Could not:** `RMP 2011-2014 Rev1 sd.xls` and `CSJU List of Beneficiaries 2015.xlsx` from the
page-split cluster are **not files in this corpus** and never were. A full-corpus `find` returns
only `sheets/batch-011/xls/RMP 2011-2014 and Inventory.xls` and
`sheets/batch-013/xlsx/CSJU List of Recipients of funds 2013-2020.xlsx`. The CSJU workbook has
sheets named `2013`…`2020`, so "List of Beneficiaries 2015" is a **title on a sheet**, not a
filename — the user named the page they were looking at. The RMP workbook contains a sheet
`RMP 2011-2014` (and `'RMP 2011-2014'!Print_Titles`), but the string `Rev1` does **not** occur
anywhere in its bytes, so that identification is *probable, not established*. Anyone chasing
this cluster should chase the two workbooks, not the two names.

---

## 3. The 227 → 449 figure — established twice, but as the wrong claim

The brief asked for this to be established twice before believing it, on the grounds that a
doubling is the shape of a measurement artifact. It is **not an artifact**, by two independent
means — and then a third consideration removes the *interpretation* anyway.

**Means 1 — solo, unloaded re-render.** `sectors-defense-and-aerospace.xlsx` rendered alone,
nothing else running, `SOURCE_DATE_EPOCH` and `TZ` pinned: **449 pages**, file size
**1,084,225 bytes — byte-for-byte the same size as the sweep's copy**, differing only in
`/CreationDate`. Page size A4 595.304 × 841.89 in both. Pages 447/448/449 carry 11/13/1 words,
so there is no truncated tail. Word count *rose* 23066 → 23964; a truncated render loses words.
The same solo re-render reproduced 88, 201 and 190 exactly for the other three named documents.

**Means 2 — mechanism, not repetition.** The pagination is **width-driven**. Re-rendering the
same workbook on **A3 landscape — 1190.55 × 841.89, exactly twice the width and the identical
height**, so rows-per-page cannot change — gives **143 pages**, a 3.1× collapse. Consecutive
pages confirm the structure directly: page 3 carries a *different column* of the *same rows* as
page 1. The document declares no `pageSetup` and no print titles on any of its 13 sheets, so
every page-geometry decision is a LibreOffice default.

**But the cause is now open.** "Width-driven" is exactly what a *font metric* change moves, and
the DejaVu confound means I cannot say whether 227 → 449 is the version, the font, or both. The
established facts are: the 449 is deterministic and reproducible, and the mechanism is a
horizontal column-band split. The attribution is not established and must not be reported as a
24.2 → 26.2 effect.

---

## 4. `7-memento-2015-transports-aeriens-b.xls` — the brief's premise is refuted, and the real defect is located

The brief: *"a merged block that the decoration path knows about emits its left edge once per
covered row, and one segment says the block is not in our model as a merge"* — find why that
`MERGEDCELLS` range never reaches `StatedMerges`.

**There is no such range. The premise is false at the source.**

### 4.1 The BIFF read (measured, from the bytes)

A linear walk of the whole `Workbook` stream — 421,499 bytes, 16,815 records, ending exactly on
a record boundary; 33 `BOF`, 33 `EOF`, 32 `BOUNDSHEET`, so no nested substreams and no
attribution guesswork:

* The workbook contains **12** `MERGEDCELLS` (`0x00E5`) records, **178** ranges in total.
* They live in sheets `7.1.3.`, `7.1.4.`, `7.1.5.`, `7.2.1.`, `7.2.2.`, `7.3.1.`, `7.3.2.`,
  `7.4.1.`, `7.5.2.2 `, `7.5.3.`, `7.5.9.`, `7.6.1.`.
* **Reference page 2 is sheet `7.1.1.`** (confirmed by its extracted text), and sheet `7.1.1.`
  has **zero** `MERGEDCELLS` records — 665 records in its substream, opcode census taken, no
  `0x00E5` among them.

Our reader is not dropping anything. For completeness it was read anyway and is correct:
`ReadMergedCells` at `dotnet/src/XRay.Office.Spreadsheets/MsBinary/XlsWorkbookReader.cs:1737-1748`
reads the count and every 8-byte range and calls `builder.AddMergedRange`, dispatched from the
sheet record loop at `:1355-1356`, and reaches `StatedMerges` at `:1224`. No filter, no cap, no
single-record assumption.

### 4.2 What actually draws those verticals (measured)

The blue verticals are the **left and right borders of column 1**, which is a single very wide
column (`COLINFO` width 18102/256 chars ≈ 392 pt, matching x = 119.99 → 512.39 exactly). No
merge is needed for them to exist. In sheet `7.1.1.`, from the XF records and the cell records:

| edge | stated by | border style | palette index | colour | cells |
|---|---|---|---|---|---|
| x ≈ 512.4 | column 1's **right** border | 1 (thin) | **30** | `#0066CC` | 67 |
| x ≈ 512.4 | column 2's **left** border | 1 (thin) | **56** | `#003366` | 63 |

Both neighbours state a **same-width** border on the **same physical edge** with **different
colours**. The file carries a `PALETTE` record (56 entries, 7 differing from the default), and
index 30 is `#0066CC` and index 56 is `#003366` in **both** the file's palette and the default —
so this is not a palette bug.

### 4.3 The divergence, measured two ways

*Vector*, from the page-2 content streams:

| | x ≈ 120.0 | x ≈ 512.4 |
|---|---|---|
| reference | `#0066CC`, 5 runs (13.46, 102.56×3, 115.28) | `#0066CC`, 10 runs |
| ours | `#0066CC`, 2 runs (13.1, 13.48) | **`#003366`, 32 runs** + `#0066CC`, 2 |

*Raster*, 150 dpi page 2, independent of my stroke interpreter:

| | `#0066CC` px | `#003366` px |
|---|---:|---:|
| reference | **63,765** | **1** |
| ours | 18,061 | **847** |

The reference paints essentially **no** `#003366` on that page. We paint 847 px of it, and 3.5×
too little `#0066CC`. Both renders place the **same columns and the same rows** on page 2 —
their `pdftotext -layout` output for page 2 is character-identical — so this is not a banding
difference.

### 4.4 Diagnosis — file and line

Our tie-break is a **faithful** port and is not the bug. `SheetCellBorders.Resolve`
(`dotnet/src/XRay.Office.Spreadsheets/Layout/SheetDecoration.cs:155-156`) returns `own` unless the
neighbour `IsHeavierThan` it, and `IsHeavierThan` (`:114-124`) compares width, then doubleness,
then distance, then dot pattern — never colour. That mirrors `svx::frame::Style::operator<`
(`svx/source/dialog/framelink.cxx:306-335`) exactly, and `std::max` returning its first argument
on a tie is correctly reproduced.

**The bug is that we apply the interior rule at the edges of the printed column band.**
LibreOffice's `Array::GetCellStyleLeft` (`svx/source/dialog/framelinkarray.cxx:782-799`) takes
the max **only inside the clip range**, and has three special cases before it:

```
// left clipping border: always own left style
if( nCol == mxImpl->mnFirstClipCol ) …GetStyleLeft();
// right clipping border: always right style of left neighbor cell
if( nCol == mxImpl->mnLastClipCol + 1 ) …(nCol - 1…)->GetStyleRight();
// outside clipping columns: invisible
if( !mxImpl->IsColInClipRange( nCol ) ) return OBJ_STYLE_NONE;
// inside clipping range: maximum of own left style and right style of left neighbor cell
return std::max(…GetStyleLeft(), …(nCol - 1…)->GetStyleRight());
```

`Edges.Build` in `dotnet/src/XRay.Office.Spreadsheets/Layout/SheetPageDecoration.cs` has no
equivalent. It calls `Resolve` at all four edges — **lines 710, 716, 731 and 738** — and each
call reaches one cell *outside* the placed band:

* **`:738`** (trailing vertical, the one this defect is on):
  `Resolve(own.Right, decoration(row.Row, column.Column + 1).Borders.Left)`
* `:716` (leading vertical): `Resolve(own.Left, decoration(row.Row, column.Column - 1).Borders.Right)`
* `:710` / `:731` are the row-direction twins.

On page 2 the band ends at column 1. On the 49 rows where column 1 states **no** right border
while column 2 — which is **not on this page** — states a `#003366` left border, `Resolve(None,
#003366)` returns the neighbour, and we paint a border belonging to a column the reader is not
printing. LibreOffice's `mnLastClipCol + 1` rule takes the in-band column's right style
unconditionally and never consults column 2, so it paints `#0066CC` where column 1 states one
and nothing where it does not.

**Status:** the record evidence (4.1), the XF evidence (4.2) and the two divergence
measurements (4.3) are **measured**. The attribution in 4.4 is **inferred from control flow in
both trees** — I did not execute our reader with the band edge suppressed. The decisive
confirmation is a one-line change (clamp the ±1 lookups to the placed band at its outer edges)
and a re-render; it needs a build and its own worktree.

Note also that x ≈ 120.0 is still short by more than the colour rule explains (2 runs against 5;
18,061 px against 63,765). The clip rule accounts for the *wrong-colour* segments at the
trailing edge; a second effect at the leading edge is **open and unexplained**.

---

## 5. The column-fit predicate, re-checked on 26.2.4.2 — **HOLDS**

Seven authored flat-ODS sheets, one variable at a time, three points on the font axis and two on
each content axis, `Liberation Sans` named explicitly so the font confound cannot reach it.
3 columns × 4 rows, every cell boxed; the drawn rules give widths and heights directly.
Generator and raw output: `scratchpad/sheets/colfit.py`, `scratchpad/sheets/colfit/`.

| axis | varied | column widths (pt) | row heights (pt) |
|---|---|---|---|
| A | font 8 → 12 → 20 pt, width fixed 1in | 72.0 / 72.0 / 72.0 — **unchanged** | 12.79 → 14.99 → **24.43** |
| B | content 2 → 40 chars, width fixed 1in | 72.0 → 71.97 — **unchanged** | 12.79 → 12.82 — unchanged |
| C | content 2 → 40 chars, width **optimal** | 63.98 → 63.95 — **unchanged** | 12.79 → 12.82 — unchanged |

**Verdict: holds.** Column width does not respond to content length on 26.2.4.2 — not even when
the column asks for `style:use-optimal-column-width="true"`, which is the strongest form of the
predicate and the one most likely to have changed. It does not respond to font size either
(axis A's control). Row height responds to font size strongly and monotonically. The axis is
still **row heights**, exactly as the refutation on 24.2.7.2 said.

The two controls are the point: axis A's column row and axis B's row-height row both come back
unchanged, so the instrument is not simply reporting "changed" for everything.

---

## 6. Withdrawn: the scoreboard and the page-split cluster

Recorded for the re-run, **not** as results. Method: `scratchpad/sheets/ours-vs-stored.sh`,
which renders only ours and scores against banked reference PDFs using `batch-check.sh`'s three
predicates verbatim (page count; words within 2% *and* >3 absolute; unembedded fonts). It was
validated on `sheets/batch-001` first — 10/10 — before the full sweep.

Measured **135/171** against `refpdfs-26.2.4.2` (the *pre*-DejaVu reference renderings), versus
the stored **155/171** at 24.2.7.2. Both halves of that comparison are contaminated: the
reference PDFs were rendered without DejaVu, and our renders were made after it was installed.
**The pair is mismatched and the number is void.**

One structural observation from it is worth carrying forward because it is about *direction*
rather than magnitude, and it is what a re-run should test: of 21 apparent regressions,
**zero were "ours-only"** — every one involved the reference column moving, our column moving,
or both. Our own column moved on 16 documents despite the code being at `HEAD`, which is itself
a font-metric signature and is consistent with the confound the coordinator found.

Page-split cluster, reference side, all likewise withdrawn:

| document | stored ours/ref (24.2.7.2) | measured here | note |
|---|---|---|---|
| `FAA-2019-0995-0002_attachment_2.xlsx` | 32/33 | 32/33 | unchanged both binaries |
| `aircraft_analysis_2016-04-27.xls` | 44/46 | 44/46 | unchanged; cell fallback still never fires |
| `FY2018_Q4_UAS_Sightings.xlsx` | 304/302 | 304/302 | unchanged both binaries |
| `CSJU List of Recipients of funds 2013-2020.xlsx` | 97/96 | 97/97 | reference moved to meet us |
| `RMP 2011-2014 and Inventory.xls` | 38/38 | 38/38 | matches; **not the named file** (§2) |

---

## 7. What I could not establish

* **Any ours-vs-reference verdict.** The reference half available to me was rendered without
  DejaVu. Everything in §6 needs re-running against `refpdfs-26.2.4.2-fonts/`.
* **Whether 227 → 449 is a version effect or a font effect.** §3 establishes it is real and
  width-driven; the attribution needs the corrected baseline.
* **Whether the clip-range rule is the whole of the `7-memento` defect.** §4.4 is inferred from
  control flow, and the leading edge at x ≈ 120.0 is short by more than that rule explains.
  Both need a build in an isolated worktree.
* **Corpus reach of the clip-range defect.** It should touch every sheet whose printed band ends
  inside a bordered region — that is a large class and includes documents that currently *match*,
  so the reach census must be run over the matching documents too, not only the failing ones.
* **`RMP 2011-2014 Rev1 sd.xls`.** Probably the `RMP 2011-2014` sheet of
  `RMP 2011-2014 and Inventory.xls`, but `Rev1` appears nowhere in that file's bytes.

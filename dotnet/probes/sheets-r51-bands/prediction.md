# Round 51 — sheets — prediction

Written and committed **before any post-change rendering**. Environment: LibreOffice
**26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r51`,
base `bd0f5ac1cf2`.

## Baseline, reproduced before anything was believed

`batch-check.sh sample-files 'sheets/*' … 6` → `TOTAL 325 MATCH 277 MISMATCH 48`. That total is
inflated by the case-insensitive mount: five documents are counted twice under two spellings of
one inode. Case-folded to unique paths: **307 documents, 267 match, 40 mismatch**, and the 40
mismatching paths are **exactly** the 40 rows `MANIFEST.tsv` marks `open`. Baseline reproduced to
the document.

## What is being changed

Not the briefed item. The chart-legend lead was investigated first and is reported as a
refutation in `results.md`; the fix shipped here came from underneath it, out of the blind page
readings plus a footer census.

**The defect.** `XlsxPrintSetup` and `XlsbPrintSetup` never set `HeaderGap`/`FooterGap`, so both
inherit `SheetPrintSetup`'s ODF default of **142 twips (7.1 pt)**. `SheetPageDecoration.DrawBand`
computes its text rectangle as `bandHeight − gap` and returns early when that is negative. So
**every XLSX/XLSB header or footer whose stated band is under 7.1 pt is dropped outright** — no
ink, no words. `XlsPrintSetup` already has the right rule and does not share it: its `Gap()`
returns zero when the band is pinned, which is `PageSettingsConverter::convertHeaderFooterData`'s
`mnBodyDist = max(0, statedBand − nominal)` (`sc/source/filter/oox/pagesettings.cxx:1029-1041`).

**Measured, not reasoned about.** Six authored margin variants of
`020_Free_Blood_Pressure_Chart…xlsx`, varying only `bottom` and `footer`, rendered both ways:

| bottom | footer | stated band | ours | reference |
|---:|---:|---:|---:|---:|
| 0.30 in | 0.30 in | 0.0 pt | not drawn | **not drawn** |
| 0.30 | 0.25 | 3.6 pt | **not drawn** | drawn, text top at 770.37 |
| 0.35 | 0.25 | 7.2 pt | drawn at 762.85 | drawn at **766.77** |
| 0.50 | 0.25 | 18.0 pt | drawn at 762.85 | drawn at 762.32 |
| 0.30 | 0.10 | 14.4 pt | drawn at 773.65 | drawn at 773.12 |
| 0.75 | 0.30 | 32.4 pt | drawn at 759.25 | drawn at 758.72 |

Three separate rules fall out, and each is a separate change:

1. A band under 7.1 pt is suppressed by us and drawn by the reference. → the gap.
2. When the band is shorter than its text the reference **top-aligns** it at the band's own top
   edge (770.37 ≈ 770.4, 766.77 ≈ 766.8) and we bottom-align it on the footer margin, 3.9 pt out.
   That is `nDif = max(0, paperHeight − textHeight)` (`printfun.cxx:1876-1912`). → a clamp.
3. A band of stated height **zero** draws nothing, on this binary. `SheetPageDecoration`'s own
   comment says the opposite and cites a 24.2.7.2 measurement on
   `2012-GA-Survey-Chapter-6-Tables-16Dec2013-V2.xls`. On 26.2.4.2 that document's reference PDF
   contains **no footer at all** and ours contains four `Page 6 - N` lines. The stored claim has
   decayed with the binary; the guard is being made explicit.

**And a second, independent defect on the same document.** `&K` is a colour code and Calc eats
**six characters** after it unconditionally (`pagesettings.cxx:639-647`). We eat *up to six hex
digits* and stop at the first non-hex one, so Excel's theme form `&K01+049` leaves `+049` in the
drawn text. Censused over all 307 sheets documents: **16 `&K` occurrences in 5 documents take the
theme form**, and our banked PDFs draw `+049`/`+034` **30 times** where the reference draws it
**0** times.

## Documents predicted to change, and to what

**Verdict movement predicted: +1.** sheets **267 → 268 of 307**.

| document | status | now | predicted | verdict |
|---|---|---|---|---|
| `020_Free_Blood_Pressure_Chart…` | open | 117/133 words | **133/133** | `words` → **`match`** |
| `2012-GA-Survey-Chapter-5-Tables…xls` | done | 501/495 | 495/495 | match → match |
| `2012-GA-Survey-Chapter-6-Tables…xls` | done | 636/624 | 624/624 | match → match |
| `021_Control_Chart_Template…` | done | 925/921 | 921/921 | match → match |
| `018_Weight_Loss_Chart…` | done | 968/958 | 960/958 | match → match |
| `022_Pareto_Chart_Template…` | done | 191/192 | 190/192 | match → match |
| `023_Waterfall_Chart_Template…` | done | 881/868 | 881/868 (no change) | match → match |
| `fm-provider-service-measures.xlsx` | done | 21245/21348 | up, by at most ~90 | match → match |
| `FAA-2019-0995-0002_attachment_2.xlsx` | done | 9995/9995 | **unknown — at risk** | match → ? |

`020`'s landing value is derived, not hoped for: the reference draws the footer on **4** of its 6
pages (the other two come from the `©` sheet, which declares none), and the footer contributes
4 word-tokens a page — `https://…` on the left, and `2010-2017 Vertex42 LLC` on the right, the
`©` itself carrying no letter or digit. 4 × 4 = **16**, and 133 − 117 = 16.

**Placement-only, no word change predicted:** the 11 further documents the census finds with a
*pinned* band ≥ 7.1 pt. Their band text moves up to the band's top edge. Among them is
`fm-provider-service-measures.xlsx`, whose header `SheetPrintSetup.HeaderIsDynamic` records as
LibreOffice 21.609 pt against our 30.212 pt — that discrepancy should close.

**Page counts: 0 of 307 predicted to change.** `SheetPagination` reads `HeaderHeight` and
`FooterHeight` and nothing else about a band; the diff changes the gaps, a draw-time clamp and a
draw-time guard, none of which those two properties depend on.

## What this census cannot see

1. **`.xls` and `.xlsb` bands are not censused.** The census parsed `xl/worksheets/sheetN.xml`
   out of OPC packages only. BIFF `HEADER`/`FOOTER` records and BIFF12 `BrtBeginHeaderFooter`
   were never decoded, so the reach of the zero-band guard and the clamp on those two readers is
   **unmeasured**. The two `2012-GA-Survey` documents were found by searching banked PDFs for
   footer literals, not by the census — which is itself the evidence that the census misses XLS.
2. **ODS and FODS are not censused at all.** `OdsPrintSetup` sets its own gaps, but the clamp and
   the zero-band guard live in the shared `SheetPageDecoration` and reach ODF workbooks too.
3. **The census re-implements `SheetBandHeight.Measure` rather than calling it**, and assumes an
   11 pt default font where the workbook's real default may differ. Bands near the
   pinned/dynamic boundary (`nominal ≈ statedBand`) will therefore be misclassified in both
   directions. Four of the eleven "misplaced" rows sit within 0.2 pt of that boundary.
4. **It counts sheets that declare a band, not sheets that print pages.** A band on a sheet
   outside the printed range moves nothing, so the census over-reaches. `FAA-2019-0995-0002`'s
   nine-line header on `sheet10` is the case where this decides whether a passing document
   survives, and the census cannot answer it.
5. **`firstHeader`/`evenHeader` variants are not separated from the odd pair.** The band is sized
   from the maximum of all three, so a document whose first-page band alone is undersized is
   invisible to the split above.
6. **The `&K` census over `.xls` is a raw byte scan** and its nine hits are false positives from
   compressed streams; no BIFF header string was decoded. If a `.xls` in this corpus carries a
   theme-form `&K`, this round will not have predicted it.

## Shared layer

**No.** All four files changed are in `XRay.Office.Spreadsheets`:
`Layout/SheetPageDecoration.cs`, `Layout/SheetBandHeight.cs`, `Layout/SheetHeaderFooter.cs`,
`Ooxml/XlsxPrintSetup.cs`, `Xlsb/XlsbPrintSetup.cs`, `MsBinary/XlsPrintSetup.cs`. Nothing in
`Core`, `Containers`, `Text`, `Vector`, `Rendering`, `Markup` or `XRay.Office.Ooxml` is touched, so
words and slides cannot be reached. That is a claim about the diff and it is checkable from the
diff; the parent owes no cross-track sweep for it.

---

# Addendum, committed before the second post-change sweep

The first sweep met the prediction (+1 verdict, `020` landing on 133/133 exactly) and turned up
one movement the prediction had flagged as **at risk and unknown**:
`FAA-2019-0995-0002_attachment_2.xlsx` went 9995/9995 → **10015/9995**, gaining five `PAGE`, five
`OF` and ten page numbers the reference does not draw. It still matches — the band is 199.9 — but
it is a real step away from the reference and it is being fixed rather than left.

Three authored single-sheet probes, one variable at a time, all at the same pinned band of 3.6 pt
and the same text, rendered both ways:

| probe | band | text | reference | ours |
|---|---|---|---|---|
| `hA` | header, 3.6 pt | one line | **draws it** | draws it |
| `hB` | header, 3.6 pt | **8 blank lines**, then the same line | **draws nothing** | draws it |
| `hC` | footer, 3.6 pt | 8 blank lines, then the line | draws nothing | draws nothing |

So it is not "an oversized band is suppressed" — `hA` and `020` both overflow their bands and are
both drawn, `020`'s single line running 8.4 pt past a 3.6 pt band. The rule that fits all four
measurements is per **line**: *a line whose origin falls inside the band is drawn in full; a line
whose origin falls past the band's bottom edge is not drawn at all.* `hB`'s text line starts about
100 pt below a band 3.6 pt tall, so it is dropped, and its eight blank lines carry no ink.

## Predicted

| document | now | predicted |
|---|---|---|
| `FAA-2019-0995-0002_attachment_2.xlsx` | 10015/9995 | **9995/9995** |
| everything else | — | **no field changes** |

**Verdict movement predicted: 0.** sheets stays at **268 of 307**. Page counts: 0 of 307.

## What this cannot see

- **Multi-line bands that fit.** For a dynamic band `FooterHeight` was grown to hold the text, so
  the last line is inside by construction — *except* that `DrawBand`'s own `bandText` and
  `SheetBandHeight.Measure`'s `measured` are computed by different code with different default
  sizes (`SheetBandText.DefaultSize` against the workbook's own default font). Where they
  disagree, a legitimate last line could fall marginally past the band bottom and be dropped.
  `fm-provider-service-measures.xlsx`, whose footer is two lines, is the named case to watch.
- `.xls`, `.xlsb` and ODF multi-line bands are not censused at all, for the same reason as before.
- The probes vary the *header* case and infer the footer; `hC` is the only footer point with
  leading blanks, and both sides draw nothing there for possibly different reasons — ours because
  the ninth line lands off the page, not because it was clipped.

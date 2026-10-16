# sheets-c-01 — four fixes, measured

The three defects `sheets-b-01` diagnosed, shipped, plus one it mis-seated. The prediction is in
`prediction.md` beside this file and was committed as `71e0b925ce1` **before any post-change
measurement**; the code is `655b8a6f5e6`. Every "ours" figure is a render of the CLI in
`/c/sandbox/workdir/wt-sheets-c`, `XRAY_OFFICE_CLI` set explicitly, `SOURCE_DATE_EPOCH=1700000000`,
`TZ=UTC`. Every "reference" figure is read out of
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/`.

---

## 0. The prediction, scored

| # | predicted | outcome |
|---|---|---|
| 1 | the brief's seat, `DrawingChart.cs:363-371`, is the **extraction** reader and moves no ink | **right** — the rendering seat is `DrawingChartPlot.ReadSequence`, a second cache reader |
| 2 | the Pilot Logbook's missing 45° is **not** the same root as the `c:f` defect | **right** — that file is a `.xls` and BIFF charts already resolve against the live sheet |
| 3 | its real seat is the BIFF **date-axis** branch of `XclImpChLabelRange::Convert` | **right**, and it is the fourth fix |
| 4 | `c:f` reaches **1** document of 171 | **right** — 1 |
| 5 | the date axis reaches **1** document | **right** — 1 |
| 6 | the area gap reaches **1** document | **wrong — 2.** The census had a real bug; see §7 |
| 7 | the link colour reaches **20–33** documents | **right** — 31 |
| 8 | 22–35 sheets renderings change | **right** — 34 of 171 |
| 9 | zero renderings changed in words and slides | **right** — 0 of 200, 0 of 163 |
| 10 | zero verdict movement | **refuted** — one document gains a verdict; see §7 |

---

## 1. What the four fixes are

| # | fix | seat | in the brief? |
|---|---|---|---|
| 1a | a chart's `c:f` is resolved against the live sheet, XLSX only | `DrawingChartPlot.ReadSequence` + new `XlsxChartRanges` | yes (task 1), **mis-seated** |
| 1b | a BIFF **date** axis keeps chart2's label defaults instead of `CHLABELRANGE`'s frequency rule | `XlsChartReader.ReadDateRange` | **no — the fourth fix** |
| 2 | an area series skips a missing point instead of plotting `0.0` | `ChartLayout.AddAreas` | yes (task 2) |
| 3 | a hyperlink cell's text is painted `#000080` | `SheetTextLayout.Ink` | yes (task 3) |

**The fourth fix was in scope, not opportunistic.** The brief's task 1 asks, in terms, whether
resolving `c:f` also fixes `Template Pilot Logbook…`'s angled axis "for free" and tells me to
verify that it does. It does not (§4). Answering the question the brief actually asked meant
finding what does, and that turned out to be a one-record importer gap two files away.

---

## 2. Two claims in the brief, refuted

**The seat named for the range defect is the wrong file.**
`DrawingChart.cs:363-371` is the **extraction** reader: it builds a `ContentSection` and is reached
from `XlsxCharts.Read`. Nothing it produces is drawn. The rendering reader is a second,
near-identical cache reader at `DrawingChartPlot.cs:1735-1771`. A fix confined to the seat the
brief names moves no ink at all. Both are now threaded, so a chart's extracted table and its drawn
picture cannot disagree about how many points it has.

**"`Template Pilot Logbook…`'s angled axis has the same root" is false.** That file is a `.xls`.
`XlsChartReader.BuildSeries` (`XlsChartReader.cs:407-450`) already calls
`data.Numbers(valueSheet, values)` and `data.Texts(labelSheet, labels)` — the BIFF path has
*always* resolved against the live sheet, and its 615 category labels have always reached
`ChartAxisLabels`. See §4 for what actually stops the rotation.

---

## 3. Fix 1a — `c:f` against the live sheet

### The rule, and why it is conditional on the host format

LibreOffice keeps two chart data providers and they answer this oppositely.
`ChartConverter::createDataSequence` (`oox/source/drawingml/chart/chartconverter.cxx:117-152`)
reads the cache and ignores the formula — right for Impress and Writer, whose numbers live in a
second document the reader must not open. Calc overrides it:
`ExcelChartConverter::createDataSequence` (`sc/source/filter/oox/excelchartconverter.cxx:76-94`)
parses the formula and falls back to the cache **only** when there is none.

The seam is a `ChartRangeResolver` the caller supplies. Null — the default, and what the PPTX and
DOCX readers pass — leaves the cache-only path byte-identical. §7 measures that rather than
arguing it.

### Acceptance test: 11 of 11 axes

The predecessor's proof was that feeding `ChartScale.Resolve` the live maximum reproduces the
reference on 11 of 11. Reproduced, through the shipped code rather than through a hand-fed
maximum. `axisscan.py` (beside this file) reads the tick sequence out of `pdftotext -bbox`;
**run first on the reference, where the answer was already known — it returns `sheets-b-01`'s
whole column.**

| page | ours before | ours after | reference |
|---|---|---|---|
| 19 | 0..25 step 5 | **0..90 step 10** | 0..90 step 10 |
| 21 | 0..8 step 1 | **0..40 step 5** | 0..40 step 5 |
| 22 | 0..8 step 1 | **0..40 step 5** | 0..40 step 5 |
| 23 | 0..16 step 2 | 0..16 step 2 | 0..16 step 2 |
| 25 | 0..16 step 2 | **0..35 step 5** | 0..35 step 5 |
| 27 | 0..18 | **0..44** | 0..44 |
| 29 | 0..14 | **0..48** | 0..48 |
| 31 | 0..6 | **0..14** | 0..14 |
| 33 | 0..4 | **0..15** | 0..15 |
| 35 | 0..4 | **0..19** | 0..19 (top label 18) |
| 37 | 0..3 | **0..4** | 0..4 |
| 39 | 0..4 | **0..19** | 0..19 (top label 18) |

The briefed case, pages 21 and 22, goes from **0..8 against the reference's 0..40** to 0..40.

**The interval agrees too, and that is a separate measurement.** Counting `#D9D9D9` strokes —
the gridlines — on the same twelve pages:

| page | 19 | 21 | 22 | 23 | 25 | 27 | 29 | 31 | 33 | 35 | 37 | 39 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| before | 7 | 10 | 10 | 10 | 10 | 11 | 9 | 8 | 6 | 6 | 5 | 6 |
| after | 11 | 10 | 10 | 10 | 9 | 24 | 26 | 16 | 17 | 21 | 6 | 21 |
| reference | 12 | 11 | 11 | 11 | 10 | 25 | 27 | 17 | 18 | 22 | 7 | 22 |

After the fix the count is **exactly one below the reference's on all twelve pages**. That constant
is the same on every chart and is not something this change introduced or fixed — before it, the
offset ranged from 0 to −16. So range *and* interval reproduce on 11 of 11 charts.

### Independently: every chart page moved closer

`pdf-image-diff.py`, whole document, ours against the reference:

| page | 19 | 21 | 22 | 23 | 25 | 27 | 29 | 31 | 33 | 35 | 37 | 39 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| before `diff%` | 3.81 | 3.12 | 4.00 | 3.19 | 2.82 | 3.39 | 3.83 | 2.64 | 3.67 | 3.59 | 4.05 | 3.84 |
| after `diff%` | 3.16 | 2.61 | 1.90 | 3.11 | 2.36 | 3.28 | 3.60 | 2.36 | 2.94 | 3.24 | 3.05 | 3.61 |

**Twelve of twelve improved**, mean 3.50% → 2.94%, and page 22's verdict crosses from `shifted` to
`ok`. The document's "10 pages with major differences" is unchanged, because the remaining majors
are the pivot-table frame the reference generates and we do not (`sheets-b-01` §1b, deliberately
not worked) and the label rhythm below.

### What is still wrong, and it is not this seat

On 6 of the 12 pages the reference labels **every second tick** where we label every one — page 27,
reference 12 labels at step 4 against our 23 at step 2, over the *same* 24 gridlines. The scale
interval agrees; only the thinning differs. That is a value-axis label-thinning defect, a separate
seat, and it is what is left of these pages after this round.

---

## 4. Fix 1b — the BIFF date axis, and the answer to "did task 1 fix the Pilot Logbook?"

### **No. Explicitly: task 1 did not fix it, and could not have.**

Rendering the Pilot Logbook with fix 1a alone leaves the axis exactly as it was. The document is a
`.xls`; fix 1a is XLSX-only by construction, and the BIFF path was already resolving live cells.

### An instrument correction worth recording

The brief's test — "the reference sets 848 glyphs at 45° and our PDF contains zero `Tm` operators"
— **cannot be run as stated on our own output.** LibreOffice turns text with the *text* matrix
(`Tm`); XRay.Office turns it with the *CTM* (`cm`, `SheetChart.cs:132-152`). A `Tm`-only count scores
a correctly rotated XRay.Office page as zero. My first pass did exactly that and read a working fix
as a no-op. `tm-rotation-scan.py` counts both, and was checked against the reference first, where
it returns 848/33 to the digit.

| | 45° | 90° |
|---|---:|---:|
| ours, before this round | **0** | 3 |
| ours, after | **6** | 3 |
| reference | 848 `Tm` (≈ one per glyph) | 33 |

### The seat

`XclImpChLabelRange::Convert` (`sc/source/filter/excel/xichart.cxx:3013-3047`) is an `if` and an
`else` over `CHDATERANGE`'s `DATEAXIS` flag, and it sets `TEXTOVERLAP`, `TEXTBREAK` and
`ARRANGEORDER` **in the `else` alone**. A date axis takes the other branch and leaves chart2's own
defaults standing — `TextBreak` false, `TextOverlap` false, `ArrangeOrder` automatic
(`chart2/source/model/main/Axis.cxx:239-242`). We read `CHLABELRANGE` and applied its frequency
rule to every axis, and never read `CHDATERANGE` (`0x1062`) at all.

Overlap is the *first* thing `ChartAxisLabels.Resolve` tests, so an axis that allows it returns
before the auto-rotate ladder is reached. The file states `CHDATERANGE` flags `0x00ff`, `DATEAXIS`
(`0x0010`) included, on both its charts — read straight out of the `Workbook` stream. With the
record read, `Resolve` returns rotation **0.7854 rad** where it used to return 0.

### Honest accounting of what this buys

It corrects the **angle** and exposes an over-aggressive **rhythm**. On page 16 we now draw 2
rotated category labels where we drew 17 upright ones, against the reference's ~17 rotated: after
rotating, our collision test keeps incrementing the rhythm to 11 (and 13 on the second chart) where
the reference settles at 1. Net effect on the page, by pixels against the reference:

| | page 16 `diff%` | `ink%` |
|---|---|---|
| before | 8.32 | 2.69 |
| after | **8.01** | 2.81 |

So slightly closer overall and slightly further in ink; the page stays `MAJOR` either way. The fix
is right at its seat, is a faithful port of a branch we had wrong, and is backed by an authored
test — but it does not on its own make that chart look like the reference. The residual is the same
label-thinning defect as §3, plus a plot-geometry difference, and both are separate seats.

---

## 5. Fix 2 — the area series' missing points

`AreaChart::createShapes` (`chart2/source/view/charttypes/AreaChart.cxx:691-706`) `continue`s past
a NaN and, under the default `LEAVE_GAP` treatment, advances the polygon index first — one polygon
per run of consecutive real points. We turned a missing point into `0.0`.

Page 16, the first series' fill rectangle:

| | fill | width |
|---|---|---|
| before | `(153.00, 155.89)-(603.95, 410.84)` | **451 pt**, spanning the plot at the baseline |
| after | `(611.92, 186.84)-(623.41, 415.97)` | 11.5 pt, at the right-hand end |
| reference | `(599.73, 167.67)-(639.21, 415.13)` | 39.5 pt, at the right-hand end |

The shape class is now right — a sliver where the data is, not a rectangle across the plot. The
remaining width difference is plot geometry, not point handling.

**Only the default treatment is implemented, because only the default is read.** Neither reader
carries `CHPROPERTIES`' empty mode nor `c:dispBlanksAs`, so a chart asking for `zero` or `span`
gets the gap. Recorded in the code rather than left to be discovered.

---

## 6. Fix 3 — the hyperlink colour

`ans_mappings_of_eccairs_terms.xlsx`, every text fill in the document:

| | `#0000FF` | `#000080` |
|---|---|---|
| ours before | **342** | 0 |
| ours after | 0 | **342** |
| reference | 0 | 131 |

Colour matches to the digit. The count differs because the reference coalesces adjacent runs and we
emit one per run — `sheets-b-01` measured the same 131 and the same 342.

`SheetLayout.HoldsField()` is reused rather than duplicated, as briefed. It was already consumed
for wrap (`SheetTextLayout`), clip and optimal row height; colour is its third consequence and they
are now stated once between them.

---

## 7. Measured reach, and verdict movement

All 534 documents of all three tracks rendered twice: once at `71e0b925ce1` (the prediction
commit, pre-fix) and once at the code commit, `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` on both,
then compared byte for byte with `/CreationDate`, `/ModDate` and `/ID` blanked. 534 of 534
rendered on each leg; nothing failed or timed out.

| track | changed | of | predicted |
|---|---:|---:|---|
| sheets | **34** | 171 | 22–35 |
| words | **0** | 200 | 0 |
| slides | **0** | 163 | 0 |

**The before leg was validated before it was believed.** The base build is only useful if it is
really the base: `ans_mappings_of_eccairs_terms.xlsx` has 342 `#0000FF` text fills and no
`#000080` on the before leg and the exact reverse on the after leg, which no other binary
produces.

### Which fix each of the 34 is

| fix | documents | which |
|---|---:|---|
| 1a — `c:f` | **1** | `Keywords_Mapping_Graphs_and_Charts.xlsx` |
| 1b — date axis | **1** | `Template Pilot Logbook JAR-FCL V3.0.xls` |
| 2 — area gap | **2** | the Pilot Logbook, and `EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls` |
| 3 — link colour | **31** | of the 33 documents holding hyperlink records |

31 + 3 distinct chart documents = 34. The two hyperlink documents that did **not** move are
`CSA_CCM_v1.2.xls` and `Application_Compliance_Checklist_5_Apr_2021.xlsx`;
`SheetLayout.HoldsField` requires the linked cell to hold text, and a link on a numeric cell stays
a plain attribute.

### The one thing the census got wrong, and how

**Fix 2 reaches two documents, not the one I predicted**, and the miss was an instrument bug worth
naming. My area-chart census tested `zipfile.is_zipfile(path)` before `olefile.isOleFile(path)` —
and `is_zipfile` returns **true** for `EHEST-Pre-departure-checklist…xls`, an OLE2 workbook, because
it scans for an end-of-central-directory signature anywhere in the file. Every such `.xls` was
routed down the OOXML branch and its nine `CHAREA` records never counted. Testing OLE2 first finds
them. This is `CLAUDE.md`'s "detect formats by content, never by extension" rule biting a probe
script rather than the reader.

It is also exactly the failure the prediction named — *"an under-reaching census conceals itself,
because a low prediction that comes true reads as well-calibrated"* — and it is only visible
because the sweep was run over the whole track rather than over the census's answer.
`area-census.py` beside this file is the corrected version.

**EHEST is a clean win, to the digit.** The old code drew the empty-point polygon as a rectangle
across the plot and stroked its outline; on each of its four chart pages we drew **4** `#339966`
strokes where the reference draws **2**, and now draw **2**. `pdf-image-diff.py` against the
reference: page 19 4.49% → 4.39%, page 20 5.52% → 5.43%, page 22 5.36% → 5.27%, page 24
5.51% → 5.42%.

### One false positive, and why it is worth recording

A first pass of this diff — run while the before leg was still rendering — reported **35**
documents, the extra being `STC_WebList.xlsx`, a 4372-page, 13.6 MB PDF that was **still being
written** when it was hashed. Re-rendering it with the same binary twice gives byte-identical
output, and its before, after and control renders all hash the same. A sweep comparison is only
valid once the sweep has exited; a file count reaching its target is not the same thing.

### Verdict movement: **one, and my prediction of zero is refuted**

Page counts are identical on all 34. Unembedded fonts are 0 on both legs. Extractable words are
identical on 32 of the 34; the two that move are:

| document | reference | before | after | gate band (2% of reference) |
|---|---:|---:|---:|---|
| `Keywords_Mapping_Graphs_and_Charts.xlsx` | 4814 | 4641 (Δ 173) | **4776 (Δ 38)** | 96.3 → **fail becomes pass** |
| `Template Pilot Logbook JAR-FCL V3.0.xls` | 1610 | 1340 (Δ 270) | 1334 (Δ 276) | 32.2 → fails either way |

`batch-check.sh:125` fails a document when `d > b*0.02 && d > 3`. **`Keywords_Mapping` moves from
`words` to `ok`**: 46 of 46 pages, 2 fonts, 0 unembedded, and its word count now inside the band.
The extra 135 words are the pivot grand-total row's categories and values, which the chart could
not draw before because the cache did not carry them.

I predicted zero verdict movement and there is one, in the right direction. It was foreseeable and
I did not foresee it: I reasoned that an axis range is invisible to the gate, which is true, and
forgot that the *category labels and data labels* that come with the missing points are extractable
text.

The Pilot Logbook moves 6 words the other way and stays failing by a factor of eight either side;
its gap is the reference's ~17 rotated labels against our 2, which is the thinning defect in §4.

---

## 8. Tests

Five new files, 23 cases. **All five are detected by reintroduction** — `verify-test.sh` exit 0 on
each, with the mutation named:

| file | project | mutation reintroduced | detected by |
|---|---|---|---|
| `XlsChartDateAxisTests.cs` | Spreadsheets | `CategoryTextOf(_everyLabel, _isDateAxis)` → `(…, false)` | `ADateAxisKeepsTheDefaults…`, `TheOrderOfTheTwoRecords…` |
| `SheetHyperlinkColourTests.cs` | Spreadsheets | `Ink` returns `portion ?? fallback` | both cases |
| `XlsxChartRangeTests.cs` | Spreadsheets | stop undoubling `''` in a quoted sheet name | `AnApostropheInAQuotedSheetNameIsUndoubled` |
| `ChartAreaGapTests.cs` | Core | restore the `: 0.0` fallback for a missing point | 3 of 4 cases |
| `DrawingChartFormulaTests.cs` | Presentations | disable the `c:f` branch in `ReadSequence` | `WithAResolverTheFormulaWins` |

**None is only a drift guard.** Within those files the cases that did *not* fail under their file's
mutation are deliberate controls — `AnUnbrokenSeriesIsOnePolygon`,
`WithoutAResolverTheCacheIsRead`, `ANonDateAxisTakesItsOverlapRuleFromTheLabelFrequency` — each
pinning the behaviour that must **not** move. A control that failed would mean the fix was a
blanket change rather than a conditional one, which is the risk the brief named.

---

## 9. Build and test counts

`dotnet build -v q -nologo` on the whole solution: **0 warnings, 0 errors.**

Ten non-Fidelity projects, each run on its own project file, counts compared against the
briefed baseline:

| project | briefed | now | Δ |
|---|---:|---:|---:|
| Core | 284 | **288** | +4 |
| Containers | 109 | 109 | |
| Text | 287 | 287 | |
| Vector | 295 | 295 | |
| Rendering | 121 | 121 | |
| Markup | 259 | 259 | |
| OpenDocument | 125 | 125 | |
| WordProcessing | 761 | 761 | |
| Spreadsheets | 621 | **636** | +15 |
| Presentations | 592 | **596** | +4 |
| **total** | 3454 | **3477** | **+23** |

**0 failed**, and the 23 added are exactly this round's 23 new cases.

Three notes, because a count that is not what was briefed is worth explaining rather than
rounding:

- The brief gives the total as **3458** where the sum of its own per-project figures is 3454.
  I have used the per-project figures, since those are what I can compare against.
- **Text reports 0 skipped here where the brief says 14.** That is an environment difference,
  not something this round changed — nothing in it touches `XRay.Office.Text`.
- **`XRay.Office.Vector` failed 1 of 295 on its first run and passed 295 of 295 on the next**,
  unchanged in between, while the corpus sweep was saturating the machine. That is the
  truncation/flake failure mode `CLAUDE.md` warns about, seen from the other side. Recorded
  rather than quietly re-run: the clean figure is the second one, and nothing in this round is
  within reach of that project.

---

## 10. What I did not do

- **Value-axis label thinning** (§3) and **category-label rhythm on a sparse axis** (§4). Both are
  in `ChartAxisLabels`/`ChartLayout`, both are what is left of these two documents, and both are
  bigger than the four fixes here.
- **The pivot-table frame**, **border coalescing**, and anything in `SheetPageDecoration.cs` — all
  excluded by the brief.
- **`.xlsb`.** `XlsbReader` reaches the same `XlsxDrawings.Read` and would need its own resolver.
  There is no `.xlsb` in the sheets track, so I could not measure whether wiring it moves anything
  and have not claimed it is done.
- **A second area-chart treatment.** `zero` and `span` are unread; see §5.

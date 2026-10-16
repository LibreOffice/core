# sheets/pagination-001 — results

Three defects found, three fixed. **Three of the five documents now match the gate outright, a
fourth is page-exact but still short on words, and the fifth was not attempted.**

Everything below is measured against the installed **LibreOffice 26.2.4.2**, reusing the banked
reference PDFs at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/`, with
`SOURCE_DATE_EPOCH=1700000000`. Nothing here is read off the 27.2-alpha C++ tree, which is not
checked out in this worktree at all.

## Scoreboard

| | before | after |
|---|---:|---:|
| `sheets/*` (all 171) | 156 match / 15 mismatch | **159 match / 12 mismatch** |
| `sheets/done-*` (156) | 156 / 156 | **156 / 156 — no regression** |

| document | before | after | |
|---|---|---|---|
| `ODs-February-2022-Airbus-Commercial-Aircraft.xlsx` | 154/175 pages | **175/175, match** | fixed |
| `sectors-defense-and-aerospace.xlsx` | 227/449 pages | **449/449, match** | fixed |
| `grants-2005.xls` | 219/201 pages | **201/201, match** | fixed |
| `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` | 109/88 pages, words 7541/8981 | **88/88 pages**, words 7541/8981 | pages fixed, words not |
| `SIL_TDB648.xlsx` | 89/90 pages | 89/90 pages | **not attempted** |

The pre-change sweep reproduced every briefed figure exactly (154/175, 109/88, 227/449, 219/201,
89/90 and words 23037/22997), which is what validates the harness before any of this is trusted.

**Run-to-run stability: the whole 171-document sweep was run twice in the identical
configuration and 0 of 171 documents differed** in pages, words or verdict. That includes all
four documents the brief lists as unstable — `ans_mappings_of_eccairs_terms`,
`PBN Matrix NAAs (V01)`, `fse_identification_form` and `SIL_TDB648`. Their instability does not
reach the gate's columns under `SOURCE_DATE_EPOCH`, so no per-document movement below is
attributable to it.

## Defect 1 — an unknown `paperSize` index discards the orientation as well as the size

**Seat:** `ExcelPaperSizes.Page`, used by all three readers.

`ODs-February-2022-Airbus-Commercial-Aircraft.xlsx` states `paperSize="121"` on eight of its
thirteen sheets. 121 is past the end of the DMPAPER enumeration, which stops at 118 — a printer
driver's own size. We resolved it to the A4 fallback and then applied the file's
`orientation="landscape"` to that fallback, emitting 143 pages of A4 landscape where the
reference emits A4 portrait.

Measured by rendering a one-cell probe workbook at `paperSize="0"` through `"135"`, all at
`orientation="landscape"`, through the installed binary and reading the page box out of each PDF:

```
UNRESOLVED — default paper, orientation discarded:  0, 48-49, 71-74, 77, 84-87, 91-135
RESOLVED   — table entry, orientation applied:      1-47, 50-70, 75-76, 78-83, 88-90
```

Every unresolved index renders 595.304 x 841.89 — A4 **portrait** — having asked for landscape.
Every resolved one swaps normally (`9` → 841.89 x 595.30, `8` → 1190.55 x 841.89).

`usePrinterDefaults="1"` does the same thing to an index the reference resolves perfectly well:
with `paperSize="8"` or `"9"` and `orientation="landscape"` it still renders A4 portrait.

Our table covers indices 0–18 and every one of those entries was confirmed correct against the
sweep. It is **not** being extended, because across the whole corpus the only indices used are
1, 5, 8, 9, 17 and 121 — the full 0–135 map is recorded here instead, so a future round that
needs an entry can take it from measurement rather than from a specification.

**Reach: one document.** Measured, not grepped — across all 534 corpus documents exactly one
(Airbus, 8 sheets) states a `paperSize` outside the table, zero state `usePrinterDefaults`, and
of the 336 BIFF `SETUP` records in the corpus zero combine a valid setup with landscape and an
out-of-table index. So the rule is right and its corpus reach is a single document.

### What was deliberately *not* changed, and why

The XLSB reader's "paper not stated" arm still turns the fallback, where the OOXML reader no
longer does. That inconsistency is an admission rather than a distinction: the OOXML behaviour is
directly measured, and **no XLSB measurement is obtainable** — the corpus holds no `.xlsb` at
all and LibreOffice cannot write one to make a probe from. `XlsbReaderTests.TheStatedPaperCounts‐
OnlyWhenTheSettingsAreMarkedInvalid` pins the existing behaviour and it is left passing rather
than flipped on an argument by analogy. It is flagged in the source at the point of the
inconsistency.

The BIFF `.xls` "settings invalid" arm looks changed and is not: `landscape` there is already
`_setupIsValid && !_portrait`, so an invalid setup could never reach the swap. Corpus evidence
agrees with the rule anyway — of the documents whose sheets set `EXC_SETUP_INVALID`, none
produces a landscape page in the reference, and both `NorwegianXPensionXFundXanalysisXexports‐
XandXLAWS-1.xls` (which asks for Tabloid landscape on two invalid sheets and gets A4 portrait)
and `Template Pilot Logbook JAR-FCL V3.0.xls` match us page-size for page-size.

## Defect 2 — the digit-width carry constant was calibrated against a superseded binary

**Seat:** `SheetFonts.DigitWidthCarry`, **0.67 → 0.57**.

A SpreadsheetML column width is a count of digits of the workbook's default font, so the
printed width of a column is `width x digitWidth` and the digit width is the one thing about a
sheet's geometry that no file states. LibreOffice reports it as a whole number of twips off its
reference device, and no simple rule reproduces every case.

Swept over **205 points** — 5 faces at every half point from 6 to 26, each read out of a filled
cell's rectangle in the reference's own PDF:

| rule | agrees with 26.2.4.2 |
|---|---:|
| truncate | 119 / 205 |
| **carry at 0.57 (chosen)** | **190 / 205** |
| round half up | 194 / 205 |
| carry at 0.67 (previous) | 172 / 205 |

The fractional part alone cannot decide it — the reference truncates a fraction as large as
0.521 and carries one as small as 0.440 — so the constant is fitted, as it always was. What
pins it is not the uniform sweep but **the seventeen default-font configurations the corpus
actually uses**, enumerated from all 171 sheets documents and each measured with its own probe
workbook:

| configuration | exact twips | 26.2.4.2 | constraint |
|---|---:|---:|---|
| Carlito 11 (65 docs) | 111.5039 | 111 | `c >= 0.5039` |
| Liberation Sans 12 | 133.4766 | 133 | `c >= 0.4766` |
| DejaVu Sans 10 | 127.2461 | 127 | `c >= 0.2461` |
| **Carlito 12 (7 docs)** | **121.6406** | **122** | **`c < 0.6406`** |
| DejaVu Sans 12 | 152.6953 | 153 | `c < 0.6953` |
| DejaVu Sans 11 | 139.9707 | 140 | `c < 0.9707` |

Window `0.5039 <= c < 0.6406`; 0.57 is its midpoint. **The previous 0.67 sits just outside it**,
and the single corpus configuration it got wrong was Carlito 12.

**Round half up scores better on the sweep and would have been a disaster.** It gives Carlito 11
→ 112, and Carlito 11 is the default font of 65 corpus documents against Carlito 12's 7. It
would have broken 51 passing documents to fix 6. This was caught by pre-screening which
documents each candidate rule moves, before running anything.

**Ground truth moved.** The 0.67 constant was correct for 24.2.7.2, whose recorded figure for
Carlito 12 was **121**. The installed 26.2.4.2 answers **122**, and two independent paths agree:
the filled-cell rectangle in its PDF export, and its flat-ODF export of
`aircraft_analysis_2016-04-27.xls`, whose BIFF default column width moved from `0.916in` (1319
twips) to **`0.9236in` (1330 twips)** — a different code path reaching the same digit width and
moving by the same one twip. This is the container note's warning landing in practice: a figure
calibrated against 24.2.7.2 is a claim about a superseded binary.

One twip is normally invisible. It stops being invisible when the fit is close:
`sectors-defense-and-aerospace.xlsx` is 40 columns wide in Calibri 12, where one twip per digit
is 2 pt per column — two reference columns need 488.07 pt of a 487.73 pt page and two of ours
needed 484.0 pt. One column per page against two; **449 pages against 227**.

**Reach: seven documents, six of which moved.** All six improved, none regressed:

| document | words before | words after | reference |
|---|---:|---:|---:|
| `2015-19 top 25 Arms producing companies…xlsx` | 2924 | **2913** | 2913 (exact) |
| `DOE-C2M2-V1.1-to-DOE-C2M2-V2.1 (1.1.0).xlsx` | 15617 | **15616** | 15616 (exact) |
| `TDA_Smoke-Detectors.xlsx` | 3765 | 3766 | 3767 |
| `aircraft_analysis_2016-04-27.xls` | 11658 | 11676 | 11671 |
| `Laser Report 2024 FOIA __Oct (1).xlsx` | 116219 | 116956 | 116735 |
| `sectors-defense-and-aerospace.xlsx` | 227 pages | **449 pages** | 449 |

`dragon-175066A.xlsx`, the document the 0.67 was originally fitted for, is untouched: its
default font 宋体 resolves to DejaVu Sans, whose 12-point width of 152.6953 carries at both
constants.

## Defect 3 — the print-area extension must wrap at sixteen bits, because the reference's does

**Seat:** `SheetTextOverflow.CachedTextWidth`.

**This is the answer to the brief's question about documents 2, 3 and 4 sharing a seat: 2 and 4
share one, and 3 does not share it with them.** Documents 2 and 4 are one defect; document 3
turned out to be the column-width defect above, which has nothing to do with how far the print
area extends.

`ScTable::MaybeAddExtraColumn` does not measure the cell — it reads the width out of
`ScColumn`'s text-width cache, which holds it as a `sal_uInt16` in pixels of a 600 dpi reference
device. 65536 of those pixels are **7864.32 pt**, so a cell wider than that is cached as the
remainder and the print area is extended by the remainder rather than by the real width.

Measured on an authored one-cell probe — *n* repetitions of `M` at ten point in a 2 cm column,
rendered through the installed binary:

| characters | 900 | 930 | 945 | **950** | 1000 | 1500 | 1890 | **2000** |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| reference pages | 17 | 18 | 18 | **1** | 1 | 11 | 18 | **2** |
| our pages (before) | 17 | 18 | 18 | 18 | 19 | 28 | 35 | 37 |

**The reference's page count falls and then rises again.** No clamp, budget or saturating
measurement can produce that shape; it is a modulus, and the period is the 16-bit boundary. Ours
was perfectly linear.

What it costs on the two documents:

* `grants-2005.xls` holds a 2831-character string in `H1292` measuring 12834 pt — 1.63 times the
  limit. Wrapped it is 4970 pt, which stops the print area at column CA where we reached IB:
  18 column bands of empty paper, and exactly the 219-against-201 gap.
* `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` is the same shape twice, at `Level 1!F95` and
  `Level 2!F23` (both the same 2498-character string). Our last printed column was GM against
  the reference's CU and EA: 51+37 blank pages against 37+30, all 21 extra pages.

The row bands were never wrong on either document, and neither over-extension came from a stale
`DIMENSIONS` record, a defined print area, an autofilter or a drawing anchor — all four were
checked and none is present.

**Blank-page *emission* was never the defect.** The reference emits blank pages too — 67 of its
88 CIS pages and 45 of its 201 grants pages carry no drawing operator at all. Both stacks run
the same rule; they disagreed only about how wide the area is. This is consistent with
`sheets-overflow-01/results.md` §3 and locates the fix on the other side of that split: in the
one measurement that decides the width, not in whatever decides to emit a page.

**Reach: two documents.** No other corpus document holds a cell over the 7864.32 pt limit in a
non-wrapping column, and the sweep confirms none moved. Neither document lost any text — word
counts are byte-identical before and after (grants 34032, CIS 7541).

## Visual verification

Six labelled pairs were handed to a **fresh subagent**, blind, with no page counts, no
expectations and no access to the repository. Its reading, unprompted:

* `Airbus` p12 — "both portrait, same proportions… I can find no difference."
* `sectors` p3 — "identically… same line-break points."
* `CIS` p5 (content) — "to the pixel as far as I can see."
* `CIS` p60 (trailing) — "both read as completely, genuinely empty."
* `grants` p20 (content) — "same number of rows, same values in the same order."
* `grants` p158 (trailing) — **"this is the one that disagrees."**

The compositor warned each half was shown at 90–93% of its rendered size; the one disagreement
is far too large for that to matter, and the five clean matches were re-checked against the
operators rather than the raster where it counted.

### A real defect the gate cannot see, found this way — logged, not fixed

On `grants-2005.xls` page 158 — a page both stacks now agree exists and both leave textless —
**we paint 486 filled rectangles and the reference paints none.** Confirmed in the inflated
content streams rather than in the image:

```
OURS page 158:  486 `re`, 490 fills, 0 text-show ops, colours (1,1,0.8) and (1,1,1)
REF  page 158:    1 `re`,   7 fills, 0 text-show ops, no `rg` at all
```

So we emit the sheet's alternating row banding across trailing pages that hold no content. The
reference paints that same banding on the *content* pages (both sides show it on p20) and stops.
Page count and word count both match, so **no gate column can see this**; only the blind read
found it. It is a paint-side defect, distinct from all three fixed here, and it is left for a
round that can take it on its own terms rather than opened as a fourth front late in this one.

## What was not done

* **`SIL_TDB648.xlsx` (89 against 90) — not attempted.** Its per-page character blocks are
  identical but shifted from page 17, where the reference emits a blank page and we do not.
  Untouched by all three fixes, as predicted.
* **The opaque EGPWS photo painted over `SIL_TDB648`'s page-1 text** — a z-order defect, not
  investigated. Logged here so it is not lost.
* **`CIS_Debian`'s ~1440-word deficit on the wide "remediation procedure" band.** Now the only
  thing keeping that document off a match: it is page-exact at 88/88 and fails on words alone,
  7541 against 8981. The brief's advice not to assume one fix would get both was correct.
* **Extending `ExcelPaperSizes` past index 18** — the measured 0–135 map is above; no corpus
  document reaches it.
* **`Courier` resolving to a narrower face than the reference's**, found while enumerating the
  corpus's default fonts and unrelated to this round. At 10 pt LibreOffice's digit width for
  `Courier` is 127 twips — the same answer it gives for `MS Sans Serif`, `Helv` and
  `Roboto Regular`, i.e. its generic fallback — where ours is 120, a real monospace face. One
  corpus document.
* **Multi-paragraph cells are measured wrong**, independently of everything above: LibreOffice
  measures an EditEngine cell's *longest line* and we measure the concatenation. A probe of
  one 300-character line / two 150-character paragraphs / one 150-character line gives the
  reference 5/3/3 and us 5/5/3. It does not affect either `.xls` here — `grants-2005.xls` has
  zero newlines in its 3164 SST strings — but it will affect ODS and OOXML multi-paragraph cells.

## Prediction scorecard

Scored against `prediction.md`, committed before any post-change measurement.

| # | prediction | outcome |
|---|---|---|
| 1 | Airbus → 175/175, 170 portrait + 5 A3 landscape | **hit exactly** |
| 2 | sectors → 449/449 | **hit exactly**, despite the 0.34 pt margin flagged as the risk |
| 3 | only the seven Calibri-12 documents move | **hit** — six moved, all improved, nothing else touched |
| 4 | `done-*` stays 156/156 | **hit** (called medium confidence; it held) |
| 5 | documents 2 and 4 not fixed by either change | **hit** — and then fixed by a third change found later |
| 6 | 2/3/4 do **not** all share a seat | **hit, and sharpened**: 2 and 4 share one seat exactly; 3 is separate |
| 7 | no material word movement | **hit** — five small movements, all toward the reference |

Seven of seven. One factual error in the prediction, corrected in the source and here: Airbus
states `paperSize="121"` on **eight** sheets, not nine.

## Test counts

Every project run individually on the final tree.

| project | result |
|---|---|
| `XRay.Office.Containers.Tests` | 109 passed, 0 failed, 0 skipped |
| `XRay.Office.Core.Tests` | 332 passed, 0 failed, 0 skipped |
| `XRay.Office.Markup.Tests` | 259 passed, 0 failed, 0 skipped |
| `XRay.Office.OpenDocument.Tests` | 125 passed, 0 failed, 0 skipped |
| `XRay.Office.Presentations.Tests` | 679 passed, 0 failed, 0 skipped |
| `XRay.Office.Rendering.Tests` | 150 passed, 0 failed, 1 skipped |
| `XRay.Office.Spreadsheets.Tests` | **793** passed, 0 failed, 0 skipped (was 792) |
| `XRay.Office.Text.Tests` | 349 passed, 0 failed, 0 skipped |
| `XRay.Office.Vector.Tests` | 295 passed, 0 failed, 0 skipped |
| `XRay.Office.WordProcessing.Tests` | 827 passed, 0 failed, 0 skipped |
| `XRay.Office.Fidelity.Tests` | 520 passed, **30 failed**, 0 skipped, 550 total |

**Fidelity is 30 of 550, exactly the briefed baseline, and 0 skipped.** The 30 are the same
word-processing and slide failures as before — none of them is a document this round moved.
Build is 0 warnings, 0 errors.

### The tests were verified to fail against the unfixed tree

Not assumed. The source files were copied aside (never `git stash` — the stash stack is
repository-global and this clone has sixteen worktrees), reverted to `HEAD`, rebuilt, and the
behaviour assertions run against the unfixed library:

```
Failed  AirbusLandscapeOnAnUnknownPaperStaysPortrait
Failed  APrinterDefaultKeepsTheApplicationsOwnPortraitPaper  (all three cases)
Failed  ADigitWidthIsNeitherTruncatedNorRounded(Carlito, 12, 122)
Failed  ABiffDefaultColumnCarriesExcelsFontDependentPadding
Failed  APrintAreaExtensionWrapsAtSixteenBitsOfSixHundredDpi
Passed  ControlTheA3SheetIsStillTurned          <- control, passes on both trees
Passed  ControlCarlito11IsUnchanged             <- control, passes on both trees
```

The two controls are the point of the exercise: they confirm the new tests are targeted at the
three defects rather than blanket assertions that any change would trip. The API-level tests
(`ExcelPaperSizes.Page`, `.Default`, `.TryPortrait`) do not compile against the unfixed tree,
which proves the API is new but proves nothing about behaviour — hence the separate
behaviour-only run above.

## Contradicting the brief

Two things, both small.

1. **`SIL_TDB648.xlsx` words are 7499/7500, not 7499/7499.** The brief says "words exactly
   7499/7499"; the measured reference is 7500. It does not change the verdict — the document
   fails on pages either way, and one word is inside the band — but the figure is off by one and
   the document is on the unstable list, so the discrepancy is worth not attributing to a change.

2. **The four "unstable" documents were stable here.** Rendered twice in the identical
   configuration with `SOURCE_DATE_EPOCH` set, 0 of 171 documents differed in pages, words or
   verdict — including all four. Their instability is real but does not reach the gate's columns
   under a pinned epoch, so the warning is about ink rather than about the numbers this round
   turned on.

## Reproducing

```sh
export XRAY_OFFICE_CLI=<tree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
export SOURCE_DATE_EPOCH=1700000000
sweep.sh /c/sandbox/workdir/sample-files 'sheets/*' /abs/out 10
```

`sweep.sh` is `batch-check.sh`'s three checks in the same order with the identical `words_of`
definition, differing only in that it takes the reference half from the banked PDFs instead of
re-rendering it. It was validated before use by reproducing every briefed figure for all five
documents in the group, on both halves of the gate.

# The 87 gate mismatches, classified — and 49 of them are not defects

**Measured 2026-09-06 at `2f4709c08`.** Environment, stated once because a stored figure is
evidence about an environment and not about the code:

| | |
|---|---|
| ours | `XRay.Office.Cli` at `2f4709c08`, `XRAY_OFFICE_BUNDLED_FONTS` unset (installed faces win) |
| ref24 | `/usr/bin/soffice` — **LibreOffice 24.2.7.2 420(Build:2)**, which is what `batch-check.sh` measures against |
| ref26 | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2 0229ac93fcf0d7cb**, with the eight Latin duplicate faces moved aside |
| fonts | system fontconfig: Carlito, Caladea, Liberation, DejaVu, WenQuanYi, IPAGothic |
| rule | `batch-check.sh` of 2026-09-05: page count, then max(2 %, 15) **alphanumeric characters** |
| corpus | `/home/user/sample-files`, 947 documents; gate at the base commit banked at `/home/user/gate-2f47/` |

The data is `classification.tsv`, one row per mismatch, with its own header carrying the
same four lines. `screen26.sh` renders a queue through 26.2.4.2; `join26.py` applies the
gate's own verdict rule to each of the three pairings; `causes.py` adds the cause column.

---

## The headline: screen against 26.2 before working anything

**49 of the 87 match 26.2.4.2 under the gate's own rule.** They are the version gap, not
defects, and no amount of work on the tree will close them — closing them would mean moving
*away* from the binary this tree is calibrated to.

| | count |
|---|---:|
| **version gap** — matches 26.2.4.2 | **49** |
| the two references disagree with each other — read, do not score | 7 |
| raster/outline ceiling — the reference draws a picture, we draw text | 7 |
| volatile recalculation — the reference re-evaluates `TODAY()`, we print the cached value | 6 |
| **chart sheet not fitted to one page — closed this round** | **2** |
| pagination, page fill | 2 |
| pagination, a break one side takes and the other does not | 2 |
| missing text | 2 |
| number format | 2 |
| DOCX SmartArt drawn as an empty frame | 1 |
| a chart drawn wrong outright | 1 |
| screened but not read | 6 |
| | **87** |

`screen.py`'s own headline — "eleven of the worst thirty words documents were the version
gap" — understates it at whole-corpus scale. **Over half of everything the gate calls a
mismatch is the gate's reference being a version behind the tree's target.**

Some of the individual figures are large enough that scoring them against 24.2 is actively
misleading:

| document | ours | ref24 | ref26 |
|---|---:|---:|---:|
| `sectors-defense-and-aerospace.xlsx` | 449 pages | 227 | **449** |
| `A_320.doc` | 118 pages | 150 | **118** |
| `grants-2005.xls` | 201 pages | 220 | **201** |
| `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` | 88 pages, 49 864 glyphs | 109, 90 659 | **88, 49 734** |
| `24-25_FAA_Holdover_Tables.docx` | 155 pages | 141 | **155** |
| `FAA 2025-26 Holdover Tables.docx` | 167 pages | 154 | **167** |
| `150-5370-10H.docx` | 727 pages | 721 | **727** |

The two Holdover Tables are worth calling out by name: `dotnet/CLAUDE.md` records them as
"two Holdover Tables share one bug and one 13-page gap". **Against 26.2.4.2 there is no gap
and no bug** — 155/155 and 167/167, glyphs within 0.4 %. Whatever that pair once shared, it
is now the version rule.

### One whole sub-family of sheets failures is the version gap, and it looks exactly like a defect

**Twenty-one** of the 49 are spreadsheets where 24.2.7.2 draws materially more text than
both we and 26.2.4.2 do — in whole readable words, with our own output carrying truncated
fragments where the surplus should be. That is the classic signature of clipping text we
should be spilling, and for nineteen of the twenty-one it is a 24.2 behaviour that 26.2
dropped. (The other two, `051_Manufacturer_defect_analysis` and
`054_Problem_analysis_with_Pareto_chart`, are a different 24.2-only behaviour with the same
signature: 24.2 draws Excel's own *"This chart isn't available in your version of Excel"*
fallback notice, worth −130 glyphs each, and 26.2 does not.)

`essd-16-3433-2024-t02.xlsx` is the clean witness. Our pages 2–4 are **empty**; 24.2.7.2's
pages 2–4 carry the continuation of a `Description` column whose text overflows to the right
of the page, re-drawn at the correct offset on each following page column. Glyphs: ours
2 346, **24.2 5 403**, **26.2 2 349**. We agree with 26.2 to three characters. Same shape,
same verdict, on `orbus_togaf_tool_csq.xls` (−6 702 against 24.2), `grants-2005.xls`
(−5 463), `7-memento-2015…xls` (−4 796), `Aircraft_Database.xlsx` (−3 853),
`RCO_VOR_Master_List_082824.xlsx` (−2 555) and eight more.

Anyone taking the sheets track's "we clip cell text the reference spills" as a defect will
spend a round implementing 24.2.7.2's behaviour and regress against the target.

---

## What was closed: a chart sheet is always printed to fit one page

**The corpus holds exactly two workbooks with an `xl/chartsheets/` part, and both were
failing the gate.** `062_Run_chart_cb7476ea.xlsx` and
`057_Simple_balance_sheet_Use_this_template_e2d4cbb2.xlsx`, each printing exactly one page
more than the reference, with a sliver of a chart's rotated category labels alone on it.

**Mechanism.** `PageSettingsConverter::writePageSettingsProperties`
(`sc/source/filter/oox/pagesettings.cxx:905-972`) branches on `eSheetType ==
WorksheetType::Chart` three times, and the branch is unconditional in all three:

- `ScaleToPages = 1` — *"always fit chart sheet to 1 page"* (`:910-914`). A chart sheet's
  own `scale` and `fitToPage` never reach the printout.
- landscape unless the file explicitly names an orientation — *"chart sheets default to
  landscape"* (`:931-932`).
- `PrintGrid` and `PrintHeaders` forced off — *"no gridlines in chart sheets"* (`:971-972`).

**Why the first one has teeth, measured on `057`.** Its chart arrives as an
`xdr:absoluteAnchor` with an explicit extent of 8 656 320 × 6 278 880 EMU. We drew it at
exactly that: the chart background rectangle in our PDF is `(50.40, 63.61)–(731.99, 558.00)`,
which is 681.6 × 494.4 pt — the declared extent to the hundredth. The reference's is
`(98.76, 78.36)–(713.77, 553.22)`, 615.0 × 474.9, because it is fitted. At 100 % the chart's
rotated category labels hang past the right edge of the printable area, so the sheet
paginates into a second page column carrying the tail of two labels and nothing else.

**Seat.** `XlsxPrintSetup.Read`, keyed on the part's root element name — a `chartsheet` part
has a `chartsheet` root by schema, and the reader is handed the loaded part and nothing else,
so the root name and the relationship type are the same fact.

**Reach, whole corpus:**

| document | before | after | reference |
|---|---|---|---|
| `062_Run_chart_cb7476ea.xlsx` | `pages,words` — 3 pages, 680 glyphs | **`match`** — 2 pages, 643 glyphs | 2 pages, 645 |
| `057_Simple_balance_sheet…xlsx` | `pages,words` — 4 pages, 2 207 | `words` — 3 pages, 2 185 | 3 pages, 1 877 |

`057`'s residual is **not** this defect and cannot be closed by drawing less. The reference
draws that chart's rotated category labels as vector outlines with no text layer behind
them: its chart page yields **112** alphanumeric characters to `pdftotext` against our
**398**. Ours is the searchable output and the word gate scores it as the failure — the
outlining ceiling `TODO.raster-ceiling.md` describes, arriving on the sheets track.

Tests: `SheetChartSheetPrintTests`, seven of them. Verified by reverting the source and
re-running: **5 fail, 2 pass** — the two that pass are the controls (a worksheet is
untouched; a chart sheet naming portrait keeps portrait).

**The sheets track re-swept, whole track, scored against the banked gate**
(`sheets-after.tsv`, `sheets-movers.txt`): `TOTAL 307  MATCH 258  MISMATCH 49`, against the
banked `MATCH 257`. **Exactly two verdicts moved and both are the two above.** Nothing else
changed verdict in either direction.

One further row's numbers moved without its verdict doing so, and it is worth recording
because it is the reference side rather than ours: `alle einzeln.xlsx` reads
`278872/278866` in the banked gate and `278872/278868` now — **our count is identical and
the reference's moved by two characters**, which is the date-bearing-sheet
non-reproducibility `dotnet/CLAUDE.md` describes. Splitting a sweep diff by which side moved
is what keeps that out of a regression list.

---

## What was left, and why

### Unwinnable as the gate is scored (13)

**Raster/outline ceiling, 7.** All seven are already in `dotnet/raster-ceiling-pages.tsv`
with verdict `ceiling`, and all seven have us drawing *more* text than the reference.
`Thailand17.ppt` and `W3_Case_Study…ppt` are worth noting together: their surplus is +500
glyphs each and the two surplus token multisets are **identical**, because the two decks
carry the same slide. The list under-counts by construction, and `057`'s residual (above) is
an eighth instance on a track the file says is "nearly untouched by this".

**Volatile recalculation, 6.** Both references re-evaluate `TODAY()` and lookups on load; we
print the cached value the file was saved with. `040_Blood_pressure_tracker` prints
`11/6/2022` where both references print `9/6/2026`; `sistem-rekod-markah` prints 1 395
cached `5`s where both references print 1 395 `#N/A`. Closing these means a formula
evaluator, and `XRay.Office.Spreadsheets` has none. **This is the same class as the caution
about date-bearing sheets in `dotnet/CLAUDE.md`, and it is worth separating from it:** that
caution is about `&D` in a *header*, which `SOURCE_DATE_EPOCH` pins on our side and not on
the reference's. This is about `TODAY()` in a *cell*, which no environment variable touches
— our side reads the cached value from the file whatever the clock says. Two of these six
also carry a second, smaller defect of ours: `065_Weight_loss_tracker` prints an unformatted
date serial (`44790`) and prints the day-name format code `aaaa` literally where the
reference prints `Sunday`.

### Needs reading rather than scoring (7)

The two references disagree with each other, so there is no verdict to move toward. The
largest are `02_mcar_part-2_and_IS_v2.10.docx` — **200 pages on 26.2, 312 on 24.2, 314
ours** — and its sibling `SPA-02_…v2.9.docx`, 205/266/268. A 112-page reference split is not
a tolerance question, and whichever way it resolves it is a large finding.

### Genuine, single, and not a group (12)

Each was read or measured; none shares a mechanism with another.

- **`024_Unit_Circle_Chart…docx` — DOCX SmartArt drawn as an empty frame.** Read: we draw an
  empty rectangle where the reference draws five coloured circles reading `YOUR TEXT`, and
  the −40 glyph deficit is exactly those five. Diagram support exists and is thorough — ten
  files under `XRay.Office.Presentations/Ooxml/PptxDiagram*.cs` — and is reachable only from
  the PPTX path. Three corpus DOCX carry a `word/diagrams/` part; the other two
  (`SPA-06_mcar_part-6…docx`, `t_TEMPforInvProgs.docx`) are missing their diagrams too and
  pass the gate anyway, at −197 and −212 glyphs. **The gate is blind to two of the three.**
  Same page also shows a heading colliding with a logo image.
- **`014_Contextures_chart_sample_991ecfc5.xls` — a chart drawn wrong outright.** Read: we
  plot about twenty `#N/A` categories the reference omits, we draw no secondary axis at all
  (the reference's `Cumulative %` line and its 0–100 % right-hand axis are absent), our bars
  run off the top of the plot area, and our value axis reads `$1`/`$0` where the reference
  reads `$800`, `$720`, `$640`. The word gate scores it at −24 glyphs, which is a rounding
  error against how wrong the page is — a clean example of the gate being blind.
- **`053_Personal_asset_inventory…xlsx`.** Read: our heading text and its line art are
  **blue** where the reference's are **teal** — the same wrong colour on both — and our
  chart's value axis steps 100 000 where the reference steps 50 000. Neither is visible to
  the gate; what the gate sees is 4 pages against 2 and two extra `Page n of 4` footers.
- `Template Pilot Logbook JAR-FCL V3.0.xls` — 124 cells read `00:00` where the reference
  reads `0:00`; the date cells differ the same way. A leading zero in our `h:mm` formatting.
- `042_Business_monthly_budget…xlsx` — we print `500`, `1000`, `-100` where the reference
  prints `500.00`, `1,000.00`, `(100.00)`.
- `068_Blue_inventory_list…xlsx`, `omrIMInterpretiveGuideLine.doc` — text the reference draws
  and we omit entirely; in both cases the omitted words account for the whole deficit exactly.
- **Four `.doc`/`.docx` pagination failures that split two and two.** Two are *fill*
  differences — a wrap or a line height drifting until a page overflows. `1447.doc` fits
  less on every page (1 570 against 1 928 on page 1) and spills 103 glyphs onto a fourth;
  `OM template…docx` is one page over on 165 with glyphs agreeing to 0.03 % and nineteen
  pages differing by a line or two each. The other two are a *break* one side takes and the
  other does not, which is a different and more tractable question: `AAC-AD-No-2021-01…doc`
  puts **more** on its page 3 than the reference does (3 266 against 3 117) and then emits a
  245-glyph page 4 with no counterpart, every later page running exactly one behind;
  `absrc-pac-01-info-note-en.doc` is the mirror — the reference splits its page 1 into
  411 + 146 glyphs where we keep 555 together, and its every later page is one behind ours.

### Screened but not read (6)

`UG.CAO.00006…docx`, `055`, `029`, `048`, `076`, and `053`'s page count. Each is recorded in
`classification.tsv` with what the token diff shows and an explicit note that it was screened
and not read. They are not given a cause here because the round did not establish one.

---

## Two things this contradicts in the record

1. **`dotnet/CLAUDE.md`: "the two Holdover Tables share one bug and one 13-page gap."**
   Against 26.2.4.2 both are page-exact and within 0.4 % on glyphs. Whatever the shared bug
   was, it is closed or it was the version rule; the sentence should not send another round
   after it.

2. **`dotnet/CLAUDE.md`, on `SOURCE_DATE_EPOCH`: "the two halves of the gate do not have the
   same reproducibility properties."** True, and it is not the only mechanism that makes a
   date-bearing sheet unscoreable. `batch-check.sh` sets `SOURCE_DATE_EPOCH` on neither side,
   so a `&D` header prints today on *both*. What actually moves is `TODAY()` **in a cell**:
   the reference recalculates it and we print the value cached in the file, so those six
   documents diverge by the whole distance between the file's save date and today, and they
   diverge further every day. No environment variable closes that; a formula evaluator does.

## Reproducing

```sh
# 1. render the queue through 26.2.4.2 (the eight Latin duplicate faces must be aside)
./screen26.sh /home/user/sample-files the87.txt /abs/out 5

# 2. score ours against both references with the gate's own rule
python3 join26.py --summary > screen26.tsv

# 3. add the cause column
python3 causes.py > classification.tsv
```

`tdiff.py` prints the tokens present in one rendering and not the other, which is what turned
"a spreadsheet is 300 characters short" into "the reference continues an overflowing cell onto
the next page column". `pageglyphs.py` gives per-page alphanumeric counts side by side, which
is what found `essd`'s three empty pages and `AAC-AD`'s inserted one. `inkbox.py` measures the
ink bounding box of one page both ways. `pairpdf.sh` composes one page of the banked pair for
a reviewer, without re-rendering either side.

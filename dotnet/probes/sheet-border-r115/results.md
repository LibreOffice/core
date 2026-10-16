# Round 115 — a cell border takes the sheet's print scale, and O56's flat 0.75 was never a style table

Seat: **O56**, *a spreadsheet's cell borders are stroked at a flat 0.75 pt where the reference
resolves the stated width*. Reference throughout is **26.2.4.2**
(`/opt/libreoffice26.2/program/soffice`). Base commit `f9fe56de5`, branch `agent/sheetborder`.

---

## 0. Summary, measured separated from inferred

**Measured.**

- The style→width table this tree already had is **right**, and the reference agrees with it at
  model level: `--convert-to fods` on the worst witness prints `fo:border="0.74pt solid #bfbfbf"`
  for a `thin` border — 15 twips round-tripped through 1/100 mm — where we state 0.75 pt.
- What was missing is the **print scale**. Drawn stroke width at 26.2.4.2 is
  `max(0.1 pt, stated width × print scale)`, measured over one workbook of thirteen cells at nine
  stated scales, as `.xlsx` and again as `.xls`.
- Three widths in the reader were also wrong and are now measured rather than derived: the
  SpreadsheetML `double` split, the BIFF `double` split, and two pattern names.
- Over the sheets track, like-for-like against the banked 26.2.4.2 leg: documents **≥1.5× heavier
  go 31 → 0**, within-10 % **111 → 172**, mean |ratio−1| **0.3047 → 0.0571**, 79 better, 4 worse,
  108 level of 191 comparable.
- Reach: **120 of 307 sheets renderings change**, `SOURCE_DATE_EPOCH` pinned; **0 page counts and
  0 alphanumeric counts move**, as a stroke weight must not.

**Inferred, and said so.** The words and slides tracks are argued unmoved from confinement — the
diff is three files, all under `src/XRay.Office.Spreadsheets`, and `git grep` finds no caller of any
of them outside `XRay.Office.Spreadsheets/Layout` and `…/Ooxml` — rather than swept. Nothing here
rests on a visual reading: **no blind reader is available in this container** (established three
times; a sibling session cannot see `/home/user/...` and cannot report back), and I did not open a
page myself.

---

## 1. The brief's hypothesis is refuted before anything else

The brief expected *"which width each style maps to"* — a table of five or six numbers. That table
was already correct in both readers, and the reference confirms it without rendering:

```sh
/opt/libreoffice26.2/program/soffice --headless --norestore \
  -env:UserInstallation=file:///tmp/lo1 --convert-to fods --outdir . \
  079_Org_charts_visual_248d1dcf.xlsx
grep -o 'fo:border[a-z-]*="[^"]*"' 079_Org_charts_visual_248d1dcf.fods | sort | uniq -c
```

```
      7 fo:border-right="0.74pt solid #bfbfbf"        <- `thin`, i.e. 15 twips
      4 fo:border-bottom="0.74pt solid #bfbfbf"
      2 fo:border="1.5pt solid #000000"
      2 fo:border-right="0.74pt dotted #000000"
      …
```

15 twips is 0.75 pt is 26.46 hundredths of a millimetre; ODF states 26 and reads back 0.74 pt. So
the reference's *model* width and ours agree, and the 5× gap is entirely in what is **drawn**.

The banked legs say the same at width level. On the worst witness, `079_Org_charts_visual.xlsx`,
the two sides draw the **same 127 strokes in the same places in the same colour**, and only the
width differs:

| page | sheet (`fitToPage="1"`) | ours (base) | 26.2.4.2 |
|---|---|---|---|
| 1 | Contents, no stated scale | 0.75 × 2 | 0.29481 / 0.295 × 4 |
| 2 | Layered 5 level, `scale="96"` | 0.75 × 28 | 0.17252 × 28 |
| 3 | Sub-layered 5 level, `scale="42"` | 0.75 × 127 | 0.15 × 127 |

Three pages of one document, three different reference widths for the same `thin` border. That is
not a style table; it is a per-sheet factor.

## 2. The rule, measured at 26.2.4.2 over nine scales and two filters

`make.py` builds one workbook per stated scale: thirteen cells, one border style each, all four
edges, no text, no fill. `scale-sweep.txt` is the raw `w`-operator histogram of the first page of
each, read straight out of the content stream (`pymupdf`'s `read_contents`), so nothing is
inferred from a rasteriser.

```
out/borders-100.pdf   0.1x6  0.50002x10  0.737x20  0.75004x5  1.75008x5  1.75745x21  2.49983x5
out/borders-75.pdf    0.1x6  0.37503x10  0.55276x20  0.56254x5  1.31259x5  1.31812x21  1.87492x5
out/borders-50.pdf    0.1x6  0.25001x10  0.3685x20   0.37502x5  0.87504x5  0.87873x21  1.24992x5
out/borders-42.pdf    0.1x6  0.21002x10  0.30956x20  0.31503x5  0.73508x5  0.73817x21  1.04999x5
out/borders-25.pdf    0.1x1  0.10001x5  0.12501x10  0.18426x20  0.18752x5  0.43755x5  0.43939x21  0.625x5
out/borders-10.pdf    0.09923x20 0.1x1  0.10001x20  0.17503x5  0.17577x21  0.25002x5
out/borders-200.pdf   0.1x6  1.00006x10  1.47402x20  1.5001x5  3.50023x5  3.51497x5  4.99976x5
out/borders-400.pdf   0.1x1  0.19956x5  2.00013x12  3.00019x5  7.00045x5  9.99951x5
xls/borders-100.pdf   0.1x6  0.50002x10  0.737x20  0.75004x5  1.75008x5  1.75745x21  2.49983x5
xls/borders-42.pdf    0.1x6  0.21002x10  0.30956x20  0.31503x5  0.73508x5  0.73817x21  1.04999x5
```

Rows identified by grouping the strokes by their own y, in the fixture's cell order:

| SpreadsheetML style | stated | drawn at 100 % | drawn at 42 % | drawn at 400 % |
|---|---|---|---|---|
| `hair` | 1 twip = 0.05 pt | **0.1** (floor) | **0.1** (floor) | 0.19956 |
| `thin` | 15 twips = 0.75 pt | 0.75004 | 0.31503 | 3.00019 |
| `medium` | 35 twips = 1.75 pt | 1.75008 | 0.73508 | 7.00045 |
| `thick` | 50 twips = 2.5 pt | 2.49983 | 1.04999 | 9.99951 |
| `double` | 10 / 15 / 10 twips | 0.50002 ×2, centres 1.248 apart | 0.21002 ×2 | 2.00013 ×2 |
| `dotted`, `dashed`, `dashDot`, `dashDotDot` | 15 twips | **0.737** | 0.30956 | — |
| `mediumDashed`, `mediumDashDot`, `mediumDashDotDot`, `slantDashDot` | 35 twips | **1.75745** | 0.73817 | — |

**The rule is `max(0.1 pt, stated × scale)`**, with the patterned styles' stated width first
rounded to a whole 1/100 mm. Three arms, each with its own evidence:

1. **The scale.** Every non-floored number above is the 100 % number times the stated scale, to
   five figures, at nine scales and in both file formats. In source (27.2 tree, so *probably* also
   26.2's): `ScDocument::FillInfo` hands `svx::frame::Style` the width in twips times `fColScale`
   (`sc/source/core/data/fillinfo.cxx`:1144-1147), and for a print or PDF export that factor is the
   constant twips→1/100 mm conversion (`ScPrintFunc::InitModes`, `printfun.cxx`:2627-2628) — the
   zoom is the fractional scale of the `Map100thMM` map mode the page is drawn through (`:2639`),
   which multiplies widths and coordinates alike.
2. **The floor is 0.1 pt**, one pixel of the PDF export's own 720 dpi device.
   `PDFPage::appendLineInfo` writes `72/DPIX` for a width that does not survive as a whole logic
   unit (`vcl/source/pdf/PDFPage.cxx`:497-505). Measured twice over: `hair` is 0.1 at 100 % and
   0.19956 at 400 %, so it is a floor and not a constant; and `thin` at 10 % is 0.10001 rather than
   0.075.
3. **A patterned width goes through a whole 1/100 mm and a solid one does not.**
   `VclMetafileProcessor2D::processPolygonStrokePrimitive2D` builds a `LineInfo` whose width is
   `std::round(getTransformedLineWidth(...))` in logic units
   (`drawinglayer/source/processor2d/vclmetafileprocessor2d.cxx`:1818-1819). 15 twips is 26.46
   hundredths of a millimetre → 26 → 0.73701 pt, and 35 twips is 61.74 → 62 → 1.75748. Measured
   0.737 and 1.75745. The rounding happens **before** the zoom: at 42 % the same lines are 0.30956
   (= 0.737 × 0.42) and 0.73817, not `round(26.46 × 0.42)`.

### 2.1 `double` is two different numbers in the two filters

- **SpreadsheetML.** `Border::convertBorderLine` calls
  `lclSetBorderLineWidth(rBorderLine, 10, 15, 10)` (`sc/source/filter/oox/stylesbuffer.cxx`:1723),
  so the whole rule is 35 twips = 1.75 pt. Measured: two strokes of 0.50002 pt whose centres are
  **1.248 pt** apart, i.e. a 0.748 pt gap.
- **BIFF.** `ppnLineParam` gives style 6 `EXC_BORDER_THICK` and `DOUBLE_THIN`
  (`sc/source/filter/excel/xistyle.cxx`:966-982), and `DOUBLE_THIN`'s `BorderWidthImpl` is
  `CHANGE_DIST` with the two lines pinned at 10 (`editeng/source/items/borderline.cxx`:358-360),
  so 10 / 30 / 10 and a whole rule of 50 twips = 2.5 pt. Measured on the *same* workbook saved as
  `.xls`: 0.50002 pt lines **1.984 pt** apart.

This tree gave both `width / 3` each — 16.67 / 16.67 / 16.67 twips out of a 50-twip total, so the
`.xlsx` whole rule was 2.5 pt where it is 1.75, and both splits were wrong.

### 2.2 Two pattern names were swapped

`mediumDashed` is `DASHED` and `slantDashDot` is `FINE_DASHED` in the same switch
(`stylesbuffer.cxx`:1735-1745); this tree had them as `FineDashed` and `DashDot`. Confirmed in the
reference's own dash arrays on the fixture: `mediumDashed` alone draws `[8.0 2.5]`, and
`slantDashDot` shares `[3.0 1.0]` with `dashed`. (The BIFF table was already right.)

## 3. What changed

`src/XRay.Office.Spreadsheets/Layout/SheetPageDecoration.cs`

- `Stroke`/`Line` are instance methods; a new `Drawn(border, width)` applies the rounding, the
  scale and the floor, and the **extension** at each end (half the crossing border's width) is
  scaled too, because in the reference the extension is in the same logic space as the width.
- The resolve — `SheetBorder.IsHeavierThan`, which decides a shared edge — is deliberately left on
  the **unscaled** widths, because in the reference the `Style` comparison happens before the map
  mode's zoom and its 1-twip special case would not survive scaling.

`src/XRay.Office.Spreadsheets/Ooxml/XlsxCellDecoration.cs` — `double` → 10/15/10; `mediumDashed` →
`Dashed`; `slantDashDot` → `FineDashed`.

`src/XRay.Office.Spreadsheets/MsBinary/XlsCellDecoration.cs` — style 6 → 10/30/10.

ODS is untouched: ODF states the widths outright and `ApplyLineWidths` already reads
`style:border-line-width`, which LibreOffice writes for every double.

## 4. What is measured and deliberately NOT changed — new seat O59

**A border's dash lengths are absolute, not proportional to its width.** `Dashes()` derives them
from the line width; 26.2.4.2's own dash arrays on the fixture are the same five whatever the
width, and they take the print scale like everything else:

| pattern | 26.2.4.2 at 100 % | at 42 % |
|---|---|---|
| dotted | `[0.49999 0.99998]` | `[0.21001 0.42002]` |
| fine-dashed (`dashed`, `slantDashDot`) | `[2.99995 0.99998]` | `[1.26005 0.42002]` |
| dashed (`mediumDashed`) | `[7.99987 2.49996]` | `[3.36014 1.05004]` |
| dash-dot | `[7.99987 2.49996 2.49996 2.49996]` | `[3.36014 1.05004 1.05004 1.05004]` |
| dash-dot-dot | `[7.99987 2.49996 2.49996 2.49996 2.49996 2.49996]` | six terms, ×0.42 |

A thin dotted border is `[0.5 1.0]` where this tree draws `[0.75 0.75]`, and a *medium* dotted one
is the same `[0.5 1.0]` where this tree draws `[1.75 1.75]`. It moves no width, so it is not part
of this seat and its reach is unmeasured. **Seated as O59.**

## 5. Re-measuring the class, the same way the census was made

`census.py` is round 113's instrument re-implemented: eight pages sampled per document, both sides
must emit **≥10 stroked items with counts within 25 %**, and the statistic is the ratio of mean
stroke widths. The reference leg is the banked `/home/user/gate-r114/ref`; our two legs were
rendered fresh at the round's base and with the fix, both with `SOURCE_DATE_EPOCH=1700000000`, on
the same day (2026-09-13), which is what C13 requires.

```sh
python3 census.py /home/user/sb-base  /home/user/gate-r114/ref like-for-like-base.tsv
python3 census.py /home/user/sb-after /home/user/gate-r114/ref like-for-like-after.tsv
```

**Sheets track, 307 documents, 191 comparable on both legs:**

| | base | after |
|---|---:|---:|
| median ratio | 1.002 | **1.000** |
| ours lighter by >10 % | 20 | 15 |
| within 10 % | 111 | **172** |
| ours heavier 10–50 % | 29 | 4 |
| **ours heavier ≥1.5×** | **31** | **0** |
| mean \|ratio − 1\| | 0.3047 | **0.0571** |
| better / worse / level | — | 79 / 4 / 108 |

`class-before-after.txt` is the per-document table for all 31. Every one lands within 8 % of
1.000 and twelve land on it exactly; the worst witness goes **4.759 → 0.989** (ours 0.750 → 0.156
against the reference's 0.158).

**Why 31 and not round 113's 27.** That census covered all 947 documents and reported 27
spreadsheets plus 2 `.ppt`; this one covers the 307-document sheets track only, so the two `.ppt`
are out of scope by construction, and it finds **five spreadsheets round 113's did not** —
`edb-emissions-databank v27`, `Special-Procedures_2025-07-10.xls`, `TICAPCapability_Final.xls`,
`Background_Declaration_Template.xls`, `CSJU List of Recipients of funds 2013-2020.xlsx`. The
instrument differs in two ways I can name (this one counts `fs` — stroked-and-filled — items as
stroked, and our base leg is `f9fe56de5` rather than round 113's commit) and I did not isolate
which. **The before/after comparison is sound because both legs go through the same instrument;
the 31 is not directly comparable with the banked 29.**

### 5.1 The four rows whose ratio worsens are the instrument, and both witnesses improve at width level

- `029_Annual_budget` 0.734 → 0.548. At width level after the fix we draw `0.8050 ×1` and
  `1.1500 ×1` against the reference's `0.8050 ×1` and `1.1499 ×1` — **exact** — and `0.3391 ×2`
  against its `0.3390 ×21`. The mean falls because the cell borders correctly shrank while 19
  chart strokes at 0.3450 and 17 zero-width strokes stayed put.
- `064_Small_business_cash_flow` 0.294 → 0.123. After the fix we draw `0.1875 ×48` where the
  reference draws `0.1875 ×50` — again exact, 0.75 × 0.25 — and the mean falls because 240
  zero-width strokes dominate it.

This is the *"mean width per document is coarse"* limit round 113 stated, arriving. A per-width
histogram is the right instrument for a residual; the mean is only good for a class.

### 5.2 Residuals in the after leg, none of them this seat

The 15 documents still >10 % **lighter** than the reference are chart-bearing workbooks
(`014_Contextures_chart_sample.xls` at 0.000 against 0.250, six more `Contextures` samples,
`microsoft_learn_multi_chart_examples`): we draw a chart's rules at width 0 where the reference
strokes them. The three still >10 % heavier are `022_Pareto_Chart_Template` (1.286),
`019_Free_Blood_Sugar_Chart` (1.277) and `018_Weight_Loss_Chart` (1.147), all chart-bearing too.
**Not cell borders**, and unseated.

## 6. Reach, honestly

```sh
XRAY_OFFICE_CLI=…/Release/net10.0/linux-x64/XRay.Office.Cli SOURCE_DATE_EPOCH=1700000000 \
  python3 sweep.py /home/user/sample-files 'sheets/*/*/*' /home/user/sb-{base,after} 3
python3 reach.py /home/user/sb-base /home/user/sb-after moved.tsv
```

Both legs rendered 307 of 307 with no failures.

```
documents 307  missing-in-after 0  bytes-moved 120
  page count moved   0
  glyph count moved  0
```

**120 of 307 sheets renderings change and not one gate column moves**, which is what a stroke
weight must do: it adds no page and no alphanumeric character. **This round cannot move the
gate, and claiming otherwise would be claiming reach the gate cannot see.** Ink is the
instrument, and §5 is the ink measurement.

The words and slides tracks were **not** swept. The argument that they cannot move is the diff's
confinement — three files under `src/XRay.Office.Spreadsheets`, and

```sh
git grep -ln 'SheetPageDecoration\|XlsxCellDecoration\|XlsCellDecoration' -- 'src/**/*.cs'
```

names only `src/XRay.Office.Spreadsheets/Layout` and `src/XRay.Office.Spreadsheets/Ooxml`. That is an
inference, not a measurement, and it is labelled as one.

## 7. Tests

`tests/XRay.Office.Spreadsheets.Tests/SheetBorderWidthTests.cs`, eight cases over three new
fixtures — `sheet-border-widths.xlsx`, `sheet-border-widths-scaled.xlsx` (the same thirteen cells
at `pageSetup scale="42"`) and `sheet-border-widths.xls` (26.2.4.2's own `.xls` of the first).
Every expectation is a number read out of 26.2.4.2's PDF of that fixture.

**The mutation control was run**: at the round's base, with the three source files reverted and
the tests unchanged, **7 of the 8 fail** and the eighth —
`TheFourSolidStylesAreTheirStatedTwips`, the control that must hold in both states — passes.

### 7.1 Full suite, real totals

Run project by project rather than as a solution, and the counts are the ones the runner printed:

| project | passed | failed | skipped |
|---|---:|---:|---:|
| XRay.Office.Core | 588 | 0 | 0 |
| XRay.Office.Containers | 109 | 0 | 0 |
| XRay.Office.Text | 728 | 0 | 0 |
| XRay.Office.Vector | 309 | 0 | 0 |
| XRay.Office.Rendering | 164 | 0 | 0 |
| XRay.Office.Markup | 259 | 0 | 0 |
| XRay.Office.OpenDocument | 160 | 0 | 0 |
| XRay.Office.WordProcessing | 1938 | 0 | 0 |
| XRay.Office.Spreadsheets | 1360 | 0 | 0 |
| XRay.Office.Presentations | 1193 | 0 | 0 |
| XRay.Office.Fidelity | 542 | **10** | 0 |
| **total** | **7350** | **10** | **0** |

`XRay.Office.Fidelity` ran 552 of 552 with **0 skipped**, so LibreOffice was present and the
project covered what it is meant to. Its ten failures are the two families this repository leaves
failing on purpose plus one sheet row, and **all ten fail at the round's base too**:

- nine are the reconstructed-position family — `PageDrawingComparisonTests` on `paginated.docx`,
  `.doc`, `.rtf` and `.fodt`, `TabStopComparisonTests` on `list-label-overrun` in four formats,
  and `JustificationShrinkComparisonTests` on `justify-shrink-2013.docx`;
- one is `SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt("sheet-rich-text.xlsx")`,
  the anchor-offset clamp its own remark records. **Re-checked explicitly**: with the three source
  files reverted and everything else identical, that filter is 3 passed / 1 failed, the same row.
  A picture's position is not reachable from a border width, and the measurement says so.

## 8. Files

| file | what |
|---|---|
| `make.py` | builds the thirteen-style fixture at nine stated print scales |
| `scale-sweep.txt` | 26.2.4.2's `w`-operator histogram for each, plus the two `.xls` |
| `widths.py` | per-PDF stroke-width histogram (8 pages sampled) |
| `census.py` | the like-for-like census, round 113's guard re-implemented |
| `sweep.py` | our half of one track, one directory per document, `SOURCE_DATE_EPOCH` pinned |
| `reach.py` | base-vs-after confinement: bytes, page counts, alphanumeric counts |
| `like-for-like-base.tsv`, `like-for-like-after.tsv` | the two censuses |
| `class-before-after.txt` | the 31-document class, per document |
| `moved.tsv` | the 120 renderings that changed |

Measured against **26.2.4.2**; corpus `/home/user/sample-files`; reference bank
`/home/user/gate-r114/ref`; 2026-09-13.

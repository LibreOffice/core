# Slides-F round 01 — a marker states its own paint, and `a:noFill` is a suppression

Reference **LibreOffice 26.2.4.2 620(Build:2)** with the corrected font set — `check-env.sh` green
on all five sections (soffice 26.2.4.2 620(Build:2); writer module converts; Calibri→Carlito,
Cambria→Caladea, Arial→Liberation Sans, Times→Liberation Serif, Courier→Liberation Mono, DejaVu
Sans→DejaVu Sans; pdftoppm 26.01.0; pdftotext 26.01.0). Base `4f0fd5fde66`, branch `wt-slides-f`.
Every rendering with `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC`.

`prediction.md` beside this file was committed as **`ef565e65bac`** before the census was written,
before anything was rendered and before any test existed. It is unedited and scored in §7. **Four
of its eight numbers are right, one is wrong in both directions at once, and the one it was most
afraid of — that the pixel metric could not see the change — was right twice over.**

---

## 1. Headline

| | |
|---|---:|
| slides renderings changed | **2 of 163** |
| words renderings changed | **0 of 200**, byte-compared, whole track on both legs |
| sheets renderings changed | **0 of 171**, byte-compared, whole track on both legs |
| verdicts moved | **0**, every gate column identical to the digit on both changed decks |
| `FAAAI…` page 7, the 8 scatter markers | ours **`#850F89` ×8** — the reference's own value, at the reference's own coordinates |
| `8_P-Pavese_AIRBUS…` page 15, 10 scatter markers | ours **`#70AD47` ×10** — likewise |
| records changed in the two documents | **8 and 10**, and *nothing else*: 1143 and 2049 records, counts equal on both legs |
| the pixel metric's reading | FAAAI **identical to two decimals on both legs**; AIRBUS **+0.02 further**, and §5.4 measures why |
| tests | 16 added — **15 verified by reintroduction, 0 drift guards**, 1 undetected mutation proved an equivalent formulation |
| build / suite | 0 warnings, 0 errors; ten non-Fidelity projects **3622 total, 0 failed, 1 skipped** |

---

## 2. The briefed claims, checked before being built on — and one of them is wrong

| briefed claim | verdict |
|---|---|
| `LineOf` (`DrawingChartPlot.cs:1423`) turns a stated `a:noFill` into "states nothing" | **true, verbatim.** `:1425` returns null for *no* `a:ln`; `:1427` returns null for `a:ln/a:noFill`. Two different statements, one value |
| `:797`'s `?? autoLine` "then draws the line the file explicitly suppressed" | **false for every line and scatter series in the corpus** — see below |
| this is why e-01's census undercounted, in the direction it predicted | **true**, and reproduced: my census counts the element directly and finds the case e-01's inference could not see |
| a marker's `c:marker/c:spPr` is never read (`ChartLayout.cs:2089`) | **true, verbatim.** `MarkerOf` (`:935-964`) reads `c:symbol` and nothing else; `ChartSeries` had no marker-paint member; both painters derived the colour from the series |
| `FAAAI…` is right *by accident* because of it | **true**, and now measured from the other side: with the marker read, the accident is no longer load-bearing |
| fixing the first without the second makes things worse | **true, and worse than "worse"** — the markers would go **black**, not merely off-colour. `ColourOf`'s linear-series *fill* table is `[]` (`DrawingChartAutoFormat.cs:191`), so a line series has no automatic fill and `series.Fill ?? stroke` ends at `Colour.Black` |
| `c:minorGridlines` is "small, and its reach is known" | **the reach is right, the size is wrong** — §8 |

### The correction: it is a leaked colour, not a drawn line

`DrawingChartPlot.cs:813-814` already read the same `a:noFill` for a different purpose —
`HasLine = scatterLine && …noFill is null` — and `ChartLayout.cs:2082` draws the polyline only
`if (series.HasLine …)`. **So the suppressed line was never drawn.** What the file's `a:noFill`
failed to suppress was the *value* in `ChartSeries.Line`, which then leaked into every consumer
that does not consult `HasLine`:

- `ChartLayout.cs:2089`, the marker's fill — the FAAAI and AIRBUS case, and the whole of this
  round's measured reach;
- `ChartLayout.Plots.cs:123-131`, a radar's closed stroke and its markers, where `HasLine` is not
  consulted at all;
- the frame-series border at `Plots.cs:341, 637, 652, 683, 729` and `ChartLayout.cs:2375, 2461`,
  reachable only at chart style 9–16 or 33–40 — **census reach 0**;
- the legend key at `ChartLayout.cs:3064` and the trendline fallback at `:992`.

Predicted in advance (`prediction.md`, C1) and confirmed. The brief's sentence would have sent a
round looking for a missing-line defect that does not exist.

### A third site of the same collapse, located and measured at zero

`DrawingChartPlot.cs:511` is `LineOf(Child(table, "spPr"), theme) ?? DefaultGrid` — a `c:dTable`
stating `a:ln/a:noFill` would be given the default grid colour, exactly the same confusion.
Censused over all 534 documents: **0 instances.** Deliberately not implemented, on the precedent
e-01 set for the fill half. `:387` (`GridOf`) is *not* an instance: it tests `noFill` separately
at `:385` and is correct.

---

## 3. The census — counting the element, not inferring it

`census.py` walks each package's parts transitively from `_rels/.rels`, so a chart part nothing
references is not counted, and parses every part with `ElementTree`. **No regex touches markup.**
It counts `a:noFill` occurrences **directly** rather than inferring them from what draws, which is
precisely how e-01's census came out one low.

Over all **534** documents (163 slides, 200 words, 171 sheets):

| | slides | words | sheets |
|---|---:|---:|---:|
| reachable chart parts | 61 in 15 decks | 1 in 1 doc | 11 in 1 doc |
| `c:ser/c:spPr/a:ln/a:noFill` **declared** | **22 in 7 decks** | 0 | 11 in 1 doc |
| …of which the series' `Line` is non-null today (**reach**) | **3 in 2 decks** | 0 | **0** |
| `c:marker/c:spPr` carrying a fill or a line | **11 in 4 decks** | 0 | 0 |
| `c:dPt/c:marker/c:spPr` | 0 | 0 | 0 |
| `c:dPt/c:spPr/a:ln/a:noFill` | 16 in 3 decks | 0 | 0 |
| ODF chart parts (`chart:chart` in an embedded `content.xml`) | **0** | 0 | 0 |

**The declaration-to-reach gap is 22 → 3 and it is the point of the exercise.** Nineteen of the
twenty-two are on *filled* series at chart style 2, where `spFilledSeriesLines` is `Invisible` and
`autoLine` is already null — the file suppresses a line the automatic table never offered. Blind
spot 1 of the prediction named exactly this and gave the reason.

Ceiling for the round: the union of the two reach rows — 2 decks (`FAAAI…`, `8_P-Pavese_AIRBUS…`)
from the `a:noFill` half and 4 (those two, `171128IPAP`, `1_Country-Updates_DRC_English`) from the
marker half, so **at most 4 of 163**. Measured: **2**, and the other two are byte-identical for a
reason the census could see and I did not read out of it in advance: their stated marker colour is
the same accent their series' line already states, so the correct value and the accidental value
coincide. Re-rendered individually at the end of the round and byte-compared: unchanged.

---

## 4. The fix

**`dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs`**

- **`:798`** → `SuppressesLine(properties) ? null : LineOf(properties, theme) ?? autoLine`.
- **`:813-814`** → `MarkerFill = MarkerFillOf(element, theme)`, `MarkerLine = LineOf(MarkerProperties(element), theme)`.
- **`:815`** → `HasLine = scatterLine && !SuppressesLine(properties)`, the inline `noFill` test
  replaced by the named predicate so the two readings of one element cannot drift apart.
- **new, `:1452`** → `SuppressesLine`, with the distinction written down: `a:noFill` resolves to
  `LineStyle_NONE` in `LineFormatter::convertFormatting`, absence is what the automatic table is
  *for*, and collapsing them is the defect.
- **new, `:1456`** → `MarkerProperties`.
- **new, `:1480`** → `MarkerFillOf`: the marker's `a:solidFill`, and failing that its `a:ln`
  colour — `TypeGroupConverter::convertMarker`, `typegroupconverter.cxx:657-678`, whose
  `if (aSymbol.FillColor < 0)` branch is tdf#124817.

**`dotnet/src/XRay.Office.Core/Charts/ChartPlot.cs`**

- **`:188`, `:192`** → `MarkerFill` and `MarkerLine` on `ChartSeries`, null meaning "the file
  states nothing".

**`dotnet/src/XRay.Office.Core/Charts/ChartLayout.cs`**

- **`:2091-2092`** → `series.MarkerFill ?? series.Fill ?? stroke`, `series.MarkerLine ?? stroke`.

**`dotnet/src/XRay.Office.Core/Charts/ChartLayout.Plots.cs`**

- **`:130-131`** → the same for the radar painter.

**Why a gradient marker takes its line colour rather than its gradient.** `FillOf` deliberately
reads a gradient's middle stop for a *series*, and `MarkerFillOf` deliberately does not.
`convertMarker` reads `getFillProperties().maFillColor`, which only `a:solidFill` sets, so the
reference genuinely never sees the stops. This is not a detail: **both of `8_P-Pavese_AIRBUS…`'s
markers state a three-stop `a:gradFill`**, and the fallback is the only thing that finds a colour
for them at all. Measured, not argued — the reference draws those ten markers in the accent the
marker's `a:ln` names.

---

## 5. Measured reach and **direction**

Two full sweeps of all 163, each re-counted from disk (163 and 163, not from a loop counter) and
byte-compared with nothing masked, `SOURCE_DATE_EPOCH` making that legitimate: **2 differ.**

### 5.1 Instruments, on known answers first

| control | result |
|---|---|
| `pdf-image-diff.py`, base against **itself** (`Thailand17.ppt`, 54 pages) | 0 major, every row 0 |
| `pdf-image-diff.py`, an untouched deck base against branch | 54 pages, 0 major, identical |
| `\|signed\| ≤ \|ink\|` invariant | asserted per document per leg; holds on all four |
| the two legs are two legs | FAAAI p7 base holds `#850B88` ×8, branch `#850F89` ×8 |
| e-01's own figure reproduced | FAAAI p7 reads `4.73 / 0.40 / 0.41 / 25 shifted` — the digits e-01 recorded |
| the final binary is the committed branch | the four census decks re-rendered at the end and byte-compared against `sweep-branch`: 4 of 4 identical |

### 5.2 `FAAAIandtheArtandScienceofV&Vfinal.pptx` — the debit closes

| page 7 | base | **branch** | reference |
|---|---:|---:|---:|
| marker discs at the 8 scatter points | `#850B88` ×8 | **`#850F89` ×8** | `#850F89` ×8 fills + 8 strokes |
| `#850F89` elsewhere on the page | 1 | 1 | 1 |
| `pdf-image-diff` p7 | 4.73 / 0.40 / 0.41 | **4.73 / 0.40 / 0.41** | — |
| document `\|ink\|%` | 4.40 | 4.40 | — |

The eight are paired against the reference's eight **by coordinate** — ours at (436.78, 294.29),
(323.62, 289.94), … against the reference's (434.98, 292.31), (321.02, 287.94), … — and the whole
document's operator dump differs in **exactly those 8 records out of 1143**, with the record count
equal on both legs. So this is not "the markers and probably nothing else"; it is measured to be
nothing else.

**The pixel metric is identical on both legs to two decimals, and that is reported as blindness,
not as agreement.** Eight 6.3 pt discs at a 512 px raster, 4/255 in green and 1/255 in blue.
Predicted (P7) before the instrument was run.

One free known-answer check fell out of it: the reference's own page 7 strokes `#850B88` once —
the *slide's* own shape, not the chart — which is the shaded accent e-01 taught us to compute.
Our two legs both draw it. The e-01 arithmetic is confirmed by a third party.

### 5.3 `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` — 10 markers, same story

| page 15 | base | **branch** | reference |
|---|---:|---:|---:|
| left scatter chart's 10 markers | `#5B9BD5` ×10 | **`#70AD47` ×10** | `#70AD47` ×10 fills + 10 strokes |
| right scatter chart's 10 markers | `#5B9BD5` | `#5B9BD5` unchanged | `#5B9BD5` |
| records differing between legs | — | **10 of 2049** | — |

The right-hand chart is the internal control and it is the FAAAI situation before this round: its
marker states the same accent its series resolves to, so the correct reading and the accidental
one coincide and nothing moves. The left-hand chart's marker states `accent6` and its series
resolved to a blue; ten for ten, at the reference's own coordinates.

### 5.4 The one page the pixel metric calls **further**, and what actually causes it

`pdf-image-diff` on AIRBUS page 15: `|ink|%` **0.36 → 0.38**. Reported as **1 page further** on
that instrument, because reporting it any other way would be choosing the flattering measurement.

It is not the colour. A 2×2 was run — both colours × both marker sizes, four builds, four
renderings of the same deck:

| page 15 `\|ink\|%` | our marker size (6.30 pt) | the reference's size (4.99 pt) |
|---|---:|---:|
| **series colour** (base) | 0.36 | **0.33** |
| **the marker's own colour** (branch) | 0.38 | **0.33** |

At the reference's own marker size the two colours are **indistinguishable to this instrument**,
and both are better than either at ours. The +0.02 is our marker being **26% too wide**: the disc
is `plot.LabelSize * 0.7` and the file states `<c:size val="5"/>`, which `convertMarker` turns
into 5 pt through `convertPointToMm100`. Painting the *correct*, darker colour over the surplus
annulus costs more mean-luma gap than it recovers on the overlap.

So: **the drawing moved onto the reference and the metric moved away from it**, and the cause is a
different, now-measured defect. This is §8's first item and it is better specified than anything
that was on the list before.

### 5.5 Cross-track: measured, not inherited

The brief said not to inherit e-01's zero, because the change is in a different function. It was
not inherited. **Both whole tracks were rendered twice** — base build and branch build, 200 and
171 files each, re-counted from disk on both legs — and byte-compared:

| track | documents | differing |
|---|---:|---:|
| words | 200 | **0** |
| sheets | 171 | **0** |

Two independent reasons back it, both measured rather than argued: the census finds the corpus's
entire OOXML chart surface outside slides to be one `.docx` with no `a:noFill` and no marker
`spPr`, and one `.xlsx` whose 11 `a:noFill` series are all filled series at style 2 where `Line`
is already null; and the corpus holds **zero** ODF chart parts, so the Core painter's shared path
is exercised by nothing outside OOXML. The painter's fallback is byte-identical when both new
members are null, and two Core tests pin exactly that.

---

## 6. Verdicts: **0 of 163**, predicted plainly and confirmed

`xray-office analyze`, in process:

| deck | leg | pages | alnum words | raw words | faces | unembedded |
|---|---|---:|---:|---:|---:|---:|
| `FAAAI…` | base | 30 | 1115 | 1201 | 6 | 0 |
| | **branch** | **30** | **1115** | **1201** | **6** | **0** |
| | reference | 30 | 1101 | 1187 | 6 | 0 |
| `8_P-Pavese_AIRBUS…` | base | 26 | 2055 | 2243 | 8 | 0 |
| | **branch** | **26** | **2055** | **2243** | **8** | **0** |
| | reference | 26 | 2011 | 2202 | 11 | 0 |

Identical to the digit on every column. The other 161 renderings are byte-identical, so their
verdicts are identical by construction. **Slides stays 144 of 163 and 163 of 163 page-exact.**

This was predicted plainly rather than hedged, and it is the right outcome: the gate asks how many
pages, how many extractable words and whether the fonts are embedded, and a marker's fill is none
of the three.

---

## 7. The prediction, scored

| # | predicted | measured | |
|---|---|---|---|
| P1 | `a:noFill` census: **4–12 series across 2–5 decks** | **22 across 7** | ❌ wrong in both directions |
| P2 | marker `spPr`: **1–4 decks** | **4** | ✅ |
| P3 | changed renderings: **1–4** | **2** | ✅ |
| P4 | FAAAI's 8 markers return to `#850F89`; 1 page closer, 0 further | 8 of 8, exact, coordinate-paired | ✅ |
| P5 | verdicts **0 of 163** | 0, every column | ✅ |
| P6 | words 0/200, sheets 0/171, **re-measured not inherited** | 0 and 0, both tracks swept twice | ✅ |
| P7 | the pixel metric reads the **same on both legs** on FAAAI | 4.73 / 0.40 / 0.41 on both | ✅ |
| C1' | `Sector_Skills…` must not move | byte-identical | ✅ |
| C1 (prose) | the brief's "draws the line" is wrong; it is a leaked colour | confirmed at `:813` and `ChartLayout.cs:2082` | ✅ |

**P1 is the instructive failure and it failed on both sides at once**, which is worth more than a
near miss: the *declaration* count was three times my ceiling and the *reach* was below my floor.
A prediction phrased in the wrong unit cannot be right by luck. The blind spot that saved it was
written down — blind spot 1 said the census would count declarations the renderer resolves away,
and named filled series at the default style as the reason. That is exactly the 19.

The other cost worth naming: **P4's "0 further" is true at the operator level and false on the
pixel metric**, and only the 2×2 in §5.4 separates them. Had I reported the metric alone this
round would read as a regression; had I reported the operators alone it would read as a clean win.
Neither is the measurement.

---

## 8. Still open, in order, with numbers

1. **A marker ignores `c:size`.** New this round and measured, not guessed: we draw
   `plot.LabelSize * 0.7` — 6.30 pt on both affected decks — where the file states
   `<c:size val="5"/>` and the reference draws 4.99 pt. The 2×2 in §5.4 puts it at **0.03
   `|ink|%` on one page**, and it is what makes a correct colour read as a regression. Both
   corpus decks state `c:size`; `convertMarker`'s `convertPointToMm100(nOoxSize)` is the whole
   rule. This is the cheapest well-specified item on the list.
2. **A marker is a fill with no outline.** The reference draws each marker as a fill **and** a
   stroke (16 marker records against our 8 on FAAAI page 7, 20 against 10 on AIRBUS page 15).
   `ChartLayout.cs:2223` returns `new ChartShape(path, fill, null)` for a circle. `MarkerLine` is
   now read and reaches the painter, so this is a one-line change *and* it needs a direction
   measurement, because doubling our marker ink at the wrong marker size will move `|ink|` the
   wrong way — take it **after** item 1, not before.
3. **`c:minorGridlines`, `DrawingChartPlot.cs:374`.** Reach confirmed at e-01's figure — 3 decks,
   12 instances. **The brief's "it is small" is wrong and that is the finding here.** There is no
   minor-tick concept anywhere in the tree: `grep` for `MinorTick|MinorUnit|minorUnit|MinorStep`
   across `Core/Charts` and the OOXML reader returns **nothing**. It needs a minor interval on
   `ChartScaleResult` (LibreOffice's own auto-subdivision rule, not a guess), a model member, a
   reader, a painter — and a colour, because e-01 measured the reference stroking `#8B8B8B` and
   `#666666` where `GridProperties.cxx:64-66`'s `0xB3B3B3` says otherwise. Implementing it in the
   documented colour would move ink the wrong way on all three decks. It is a round, not a tail.
4. **`c:dPt/c:spPr/a:ln/a:noFill`** — 16 instances in 3 decks, the same defect one level down, in
   `PointFills`. Not implemented; counted so the next round starts from a number.
5. **`c:dTable` at `DrawingChartPlot.cs:511`** — same collapse, **reach 0 of 534**. Located and
   deliberately left, on the precedent of the fill half.

---

## 9. Tests — 16 added, **15 verified by reintroduction, 0 drift guards**

`tests/XRay.Office.Presentations.Tests/DrawingChartMarkerPaintTests.cs` (8, the reader) and
`tests/XRay.Office.Core.Tests/ChartMarkerPaintTests.cs` (8, the painter). Every mutation below was
applied by `verify-test.sh` to a clean tree, rebuilt on both legs, and named its failing tests.

| mutation | detected by |
|---|---|
| M1 `MarkerFill = MarkerFillOf(…)` → `null` (the original defect) | 4 tests, incl. `AMarkerIsPaintedInTheColourItStates` |
| M2 the `a:noFill` suppression reverted to `LineOf(…) ?? autoLine` (the other original defect) | `AStatedNoFillOnASeriesLineIsNotTheAutomaticColour`, `AMarkerKeepsItsOwnColourWhenTheSeriesLineIsSuppressed` |
| M3 a gradient marker read through `FillOf` (the middle stop) | `AMarkerFilledWithAGradientTakesItsLineColourAndNotTheGradient` |
| M4 the tdf#124817 line fallback dropped | the same |
| M5 the cartesian painter reverted to `series.Fill ?? stroke` | 3 tests, incl. `AMarkerWithNoSeriesColourLeftIsNotDrawnBlack` |
| M6 the radar painter reverted | `ARadarMarkerTakesItsOwnColour` |
| M7 precedence inverted, `series.Fill` before `series.MarkerFill` | `AMarkerColourBeatsTheSeriesFillToo` |
| M8 `HasLine` stops honouring the `a:noFill` | `AStatedNoFillOnASeriesLineIsNotTheAutomaticColour` |
| M9 `MarkerLine` unread at the reader | `AMarkerIsPaintedInTheColourItStates` |
| M10 the painter ignores `MarkerLine` | **undetected on the first run — a real gap**, closed by `AStrokedMarkerIsStrokedInTheColourItStates` and detected on the re-run |

**M10 is the one worth reading.** It went undetected because `Cross` and `Star` are the only two
marker shapes drawn as a stroke rather than a fill (`ChartLayout.cs:2241-2253`), and every test I
had written used a circle — so the whole `MarkerLine` path was unmeasured at the painter. A test
was added and the mutation re-run: detected. Recorded as a gap that the mutation found and closed,
not as a score.

**M11 was undetected and is *not* a gap.** `Colour marker = series.MarkerFill ?? series.Fill ?? stroke`
was rewritten as `series.MarkerFill ?? series.Fill ?? series.Line ?? Colour.Black`. With
`stroke = series.Line ?? series.Fill ?? Colour.Black` (`:2080`), the two are the same function:
the inner `?? series.Fill` can only be reached when `series.Fill` is already null. An **equivalent
formulation**, which is a different finding from an undetected defect and is reported as one.

**No fixture was added.** Every case is built from markup in the test file; every colour in it is
read out of `FAAAI…`, `8_P-Pavese_AIRBUS…` or their reference PDFs. No third-party content enters
the repository.

---

## 10. Final state

```
dotnet build XRay.Office.slnx -v q -nologo     0 Warning(s)   0 Error(s)
```

| project | before | after |
|---|---:|---:|
| Core | 305 | **313** |
| Containers | 109 | 109 |
| Text | 289 | 289 |
| Vector | 295 | 295 |
| Rendering | 149 (1 skipped) | 149 (1 skipped) |
| Markup | 259 | 259 |
| OpenDocument | 125 | 125 |
| WordProcessing | 789 | 789 |
| Spreadsheets | 663 | 663 |
| Presentations | 623 | **631** |
| **total** | **3606** | **3622**, 0 failed, 1 skipped |

Each project run individually and its count read, not the colour. `XRay.Office.Fidelity.Tests` was
**not run** — another agent owns it.

---

## 11. Measured versus inferred

**Measured or read directly:**

- Both full slides sweeps (163 / 163, re-counted from disk) and both full cross-track sweeps
  (200 / 200 and 171 / 171), all byte-compared with nothing masked.
- Every colour count in §5.2 and §5.3, out of the PDF content streams of both legs and the
  canonical reference, counting `rg` and `RG` alike; and the **whole-document** operator dumps,
  which is how "8 records of 1143" and "10 of 2049" are exhaustive rather than sampled.
- The 2×2 in §5.4: four builds, four renderings, four `|ink|%` readings.
- Every gate row in §6, from `xray-office analyze` in process.
- The census over all 534 documents, from parsed OPC parts, and the separate `c:dTable` census.
- All ten reintroduction rows in §9, from `verify-test.sh` exit 0 with named tests, plus M11's
  non-detection and the algebra that makes it an equivalence.
- `typegroupconverter.cxx:626-682`; `DrawingChartPlot.cs:374-390, 505-515, 780-830, 1420-1500`;
  `ChartLayout.cs:2040-2095, 2185-2265, 3033-3070, 3270-3290`; `ChartLayout.Plots.cs:95-140`.
- `check-env.sh`, quoted at the head of this file.

**Inferred, and flagged:**

- **That slides stays 144 of 163.** Not re-run as a gate sweep. It follows from 161 renderings
  byte-identical and the 2 changed having identical gate columns, but the 144 itself is the
  brief's figure.
- **That the AIRBUS `|ink|` regression is caused by the marker size and nothing else.** The 2×2
  isolates the size as sufficient to remove it, which is strong; it does not prove no third factor
  contributes.
- **That the reference's 17-and-20 marker records against our 8-and-10 are the missing outline**
  rather than a second mark. Read from the reference's fill/stroke pairs sharing one bounding box
  per marker, not traced through `ShapeFactory`.
- **That the corpus holds no ODF chart at all.** Measured as "no embedded `content.xml` contains a
  `chart:chart` element", which is the right test for the readers we have; a chart stored some
  other way would not be seen.

## 12. Files

- `prediction.md` — committed as `ef565e65bac` before any measurement, unedited.
- `census.py` — the part-walking census; importable, and the `c:dTable` census reuses its
  resolvers.
- `census-slides.tsv`, `census-words.tsv`, `census-sheets.tsv` — its output, one row per document
  that has anything to report.
- `show-markers.py` — prints the `c:ser` markup this round is about for one deck.
- `sweep.sh` — the render sweep both legs of every measurement here used.

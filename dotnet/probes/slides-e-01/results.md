# Slides-E round 01 — an automatic chart stroke is the accent *put through* the theme's line style

Reference **LibreOffice 26.2.4.2 620(Build:2)** with the corrected font set (`check-env.sh`
green on all five sections: soffice 26.2.4.2, writer module converts, Calibri→Carlito,
Cambria→Caladea, Arial→Liberation Sans, Times→Liberation Serif, Courier→Liberation Mono,
DejaVu Sans→DejaVu Sans; pdftoppm 26.01.0, pdftotext 26.01.0). Base `ceea7e754e4`.
Renders with `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC`.

`prediction.md` beside this file was committed as `58ce91f56d5` before any sweep, any gate run
and any pixel comparison, and is unedited. It is scored in §7. **Four of its six numbers are
right, one is wrong by exactly the blind spot it named, and one is right in a way that turned out
to be invisible to the instrument that was supposed to confirm it.**

---

## 1. Headline

| | |
|---|---:|
| slides renderings changed | **2 of 163** |
| words renderings changed | **0** |
| sheets renderings changed | **0** |
| verdicts moved | **0**, every gate column identical to the digit on both legs |
| `Demick_JetBlue` page 4 series colours | ours **`#B45D03` `#761D26` `#12415C`** — the reference's own three values, exactly |
| `Demick_JetBlue` changed pages | **5**, all 5 closer to the reference, **0 further** |
| the second changed deck | `FAAAI…` — **8 marker fills move off the reference**, and the pixel diff cannot see it |
| tests | 10 added, **10 verified by reintroduction, 0 drift guards** |
| build / suite | 0 warnings, 0 errors; ten non-Fidelity projects **3587 total, 0 failed, 1 skipped** |

---

## 2. The briefed claims, each checked before being built on

The brief said the fix hypothesis was a hypothesis. It survived; the surrounding claims did too,
with one correction.

| briefed claim | verdict |
|---|---|
| `DrawingChartAutoFormat.ColourOf` (`DrawingChartAutoFormat.cs:175`) takes no `DrawingStyleMatrix` | **true**, verbatim — six parameters, the sixth `DrawingTheme? theme` |
| `AutoLineWidth` (`DrawingChartPlot.cs:1357-1360`) fetches the same style, reads only `@w`, discards the theme | **true** — `styles.LineStyle(SubtleStyleIndex)` at `:1357`, `Drawing.Number(line, "w")` at `:1358`, `_ = theme;` at `:1360` |
| the auto-colour rule and the matrix route both exist | **true** — `PptxSlideLayoutChart.cs:57` passes `theme.Styles` into `DrawingChartPlot.Read` |
| **`automatic.Styles` is in scope at all three `ColourOf` call sites** | **true, and checked rather than assumed** — `DrawingChartPlot.cs:787`, `:790` sit in the series loop where `automatic` is a local; `:1289` is inside `PointFills`, where `ChartAutoContext automatic` is a parameter. **Unlike the crop round, the rectangle really is in scope.** |
| the fix "threads one parameter and calls `DrawingStyleMatrix.Substitute` at `:263`" | **true**, and `Substitute` is `public static` and already keeps the placeholder's own child transforms (`Replace`, `:266-284`) — which is the whole mechanism |
| the reference's arithmetic: `shade 50000` + `satMod 103000` on `F07F09`/`9F2936`/`1B587C` | **confirmed by rendering**, not re-derived: our branch now emits those three hex values and the reference PDF holds the same three |
| `c:minorGridlines` unread at `DrawingChartPlot.cs:374` | **true** — `GridOf` tests `majorGridlines` only. Quantified in §8; **not** implemented here |

### The one correction: this is a `stroke:` question, and there is a `fill:` half that is dead

`ColourOf`'s fill table (`FilledSeriesFills`) carries a remark saying the themed index is not read
because a fill entry reaches only `Theme::getFillStyle`'s gradient. That reasoning is thin —
`FillFormatter::convertFormatting` pushes `getPhColor` into the themed `FillProperties` exactly the
way `LineFormatter` does (`objectformatter.cxx:876-889`) — but **the conclusion is right for this
corpus, and now measured rather than argued**:

> Of the 163 decks, **5** have a filled series taking an automatic fill, and **0 of 5** resolve
> through a `fillStyleLst` entry that is anything but a bare
> `<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>`. Measured reach of the fill half: **zero.**

So the fill half is deliberately not implemented, and a test pins that a fill is *not* put through
the line style.

---

## 3. The census — by walking parts, and its ceiling was one low

`census.py` opens each deck as an OPC package, resolves `slide → chart` through the relationship
parts, resolves `slide → layout → master → theme` the same way (and a chart's own `themeOverride`
where it has one), and parses every part with `ElementTree`. **No regex touches markup**, which is
the rule `slides-c-01` paid for.

| | |
|---|---:|
| documents | 163 |
| zip containers (pptx) | 114 |
| decks with any chart part | **16** |
| decks with a series taking an automatic **stroke** | **2** |
| decks where that stroke resolves through a `phClr` carrying a transform | **1** |

The one is `Demick_JetBlue.pptx`: 5 chart parts, **16 automatic-stroke series**. That figure is
the instrument's known-answer check — the human review recorded in `TODO.batches.md:9587` calls
them "`Demick_JetBlue`'s sixteen line series", and the census reproduces 16 from the parts alone.

The other is `Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx`, whose
subtle line style is
`<a:ln w="9525" …><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>…</a:ln>` — a bare
placeholder. **It is the control**, and it did not move.

### The census missed one, in the direction it said it might

Blind spot 2 of the prediction was that "auto stroke" is decided by the absence of a fill under
`c:ser/c:spPr/a:ln`, and that this would *inflate* the ceiling. It deflated it instead.
`FAAAIandtheArtandScienceofV&Vfinal.pptx`'s single scatter series states

```xml
<c:spPr><a:ln w="25400" cap="rnd"><a:noFill/><a:round/></a:ln><a:effectLst/></c:spPr>
```

The census read `a:noFill` as "the series states its line", which is what the element says. **The
renderer does not.** `DrawingChartPlot.LineOf` (`DrawingChartPlot.cs:1420-1426`) returns `null`
for a stated `a:noFill`, and its caller is `LineOf(properties, theme) ?? autoLine` — so a series
that says *no line* is handed the automatic colour instead. That is a pre-existing defect my
change made visible; §5 has its consequence.

**The census as a predictor: ceiling 1, measured 2, and the miss is a defect rather than a
counting error.** Reported as the miss it is, not as a ±1 tolerance.

---

## 4. The fix

`dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingChartAutoFormat.cs`

- **`:175`** → `ColourOf` gains `DrawingStyleMatrix? styles = null` as a seventh parameter, and
  for `stroke: true` returns `ThroughSubtleLineStyle(placeholder, styles, theme)`.
- **new, `:198-256`** → `ThroughSubtleLineStyle`: take `styles.LineStyle(SubtleStyleIndex)`,
  `DrawingStyleMatrix.Substitute` the accent for its `phClr`, read the `a:solidFill` back through
  `DrawingColour`. Null matrix, no `a:solidFill`, or no readable colour → the accent unchanged.

`dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs`

- **`:787`, `:790`** → `automatic.Styles` threaded into both `ColourOf` calls.
- **`:1289`** → the same, inside `PointFills`.

`dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingStyleMatrix.cs`

- **`:143-155`** → the `LineStyle` doc comment. The brief called its old sentence — *"the caller
  … wants the width and supplies its own colour from the accent cycle"* — "the bug, written
  down", and it was. The replacement says the accent **is** the placeholder, and says what the
  old sentence claimed, so the correction survives rather than being quietly deleted.

`AutoLineWidth`'s `_ = theme;` at `:1360` is left alone: a width genuinely does not need the
colour, so the discard is correct there and only reads as a wart.

**Why the placeholder, not the colour.** `LineFormatter::convertFormatting`
(`oox/source/drawingml/chart/objectformatter.cxx:857-864`) is three lines:
`aLineProps.assignUsed(*mxAutoLine)` copies the themed line whole, the shape's own line goes over
it, and `pushToPropMap(rPropMap, …, getPhColor(nSeriesIdx))` resolves the result with the accent
as `nPhClr`. So the theme's `a:shade`/`a:satMod` act on the accent, and a theme naming a
*literal* colour there beats the accent entirely — implemented that way, and pinned by a test,
though no corpus theme does it.

---

## 5. Measured reach and **direction**

Two full sweeps of all 163, both re-counted from disk (163 files each, not from the loop
counter), byte-compared raw and with `/CreationDate` normalised: **2 differ, both columns agree.**

### Instruments, on known answers first

| control | result |
|---|---|
| `pdf-image-diff.py`, base against **itself** | 10 pages, **0** rows, 0 major |
| `pdf-image-diff.py`, an untouched deck (`Thailand17.ppt`) base against branch | 54 pages, **0** rows, 0 major |
| the two legs are two legs | `Demick_JetBlue` p4 base holds `#F07F09`×31, branch holds `#B45D03`×23 |
| `\|signed\| ≤ \|ink\|` invariant | asserted per leg: base 29.11 ≤ 29.24, branch 28.03 ≤ 28.17 |
| operator census counts **both** `rg` and `RG` | it does — LibreOffice draws these markers as `f*` fills and we as `f`, and a stroke-only count would have scored §5.2 as zero |

### 5.1 `Demick_JetBlue.pptx` — 5 pages, all five closer

| page | base `ink%` | branch `ink%` | base `\|ink\|%` | branch `\|ink\|%` | direction |
|---:|---:|---:|---:|---:|---|
| 4 | 6.57 | **6.29** | 6.57 | **6.29** | closer |
| 5 | 3.28 | **3.14** | 3.28 | **3.14** | closer |
| 6 | 4.98 | **4.74** | 4.99 | **4.75** | closer |
| 7 | 6.72 | **6.48** | 6.72 | **6.49** | closer |
| 8 | 5.38 | **5.20** | 5.38 | **5.20** | closer |
| document (10 pages) | 29.11 | **28.03** | 29.24 | **28.17** | |

**5 closer, 0 further.** Pages 1, 2, 3, 9 and 10 are identical to two decimals on both legs, as
they must be — they carry no chart. Major-page count is 6 on both legs: the residual on these
pages is dominated by the missing minor grid, not by the colour.

The colour census on page 4 is the primary evidence, and it is exact:

| colour | base | **branch** | reference |
|---|---:|---:|---:|
| `#F07F09` accent 1 | 31 | **8** | 9 |
| `#B45D03` accent 1 through the subtle line style | 0 | **23** | 46 |
| `#9F2936` accent 2 | 23 | **0** | 0 |
| `#761D26` | 0 | **23** | 46 |
| `#1B587C` accent 3 | 23 | **0** | 0 |
| `#12415C` | 0 | **23** | 46 |
| `#B3B3B3` our grid | 21 | 21 | 0 |
| `#8B8B8B` / `#666666` reference grid | 0 | 0 | 49 / 35 |

**All three land on the reference's own hex values.** And the internal control is in the first
row: base's 31 `#F07F09` are 23 automatic strokes plus 8 records that state accent 1 directly;
the branch keeps exactly those 8. The change moved the automatic strokes and nothing else.

The reference's 46-against-23 is a polyline-splitting difference that predates this round; the
`#B3B3B3` against `#8B8B8B`/`#666666` row is §8's open item.

### 5.2 `FAAAI…pptx` — 8 marker fills move **away** from the reference, and the pixel diff is blind to it

| | base | branch |
|---|---|---|
| page 7, 8 scatter markers (≈6.3 pt fills) | `#850F89` | **`#850B88`** |
| the reference draws | `#850F89` ×17 | `#850F89` ×17 |
| `pdf-image-diff` page 7 | 4.73 / 0.40 / 0.41 / 25 shifted | **4.73 / 0.40 / 0.41 / 25 shifted** |
| whole document, major pages | 0 of 30 | 0 of 30 |

**The pixel metric is identical on both legs to two decimal places.** The change is 4/255 in green
and 1/255 in blue over eight 6-point discs at a 512 px raster — precisely blind spot 3 of the
prediction. Reporting this as "no change" would be reporting an instrument's resolution as a
measurement, so it is reported as **1 page further at the operator level, 0 pages measurable at
the pixel level**.

Two pre-existing defects meet here, and neither is this round's:

1. **`LineOf` turns a stated `a:noFill` into "states nothing".** `DrawingChartPlot.cs:1423`
   returns `null` for `a:noFill`, and `:797`'s `LineOf(properties, theme) ?? autoLine` then
   supplies the automatic colour. A series whose file says it has no line is given one — 2 pt
   wide, from the stated `w="25400"`.
2. **A marker's own `c:marker/c:spPr` is never read.** The deck states
   `<c:marker><c:symbol val="circle"/><c:spPr><a:solidFill><a:schemeClr val="accent1"/>…`, and
   `ChartLayout.cs:2089` draws the marker from `series.Fill ?? stroke` instead. Base was right by
   accident, because the series' automatic colour happened to be the same accent 1 the marker
   states.

Fixing (1) alone would make it worse — the markers would then have no series colour to inherit.
The correct fix is (2), which needs a new `ChartSeries` member in `XRay.Office.Core` and therefore
the cross-track sweep a Core change owes. It is a round of its own, and the brief names this deck
as one not to work; it is §8's item 2 rather than this round's.

### 5.3 Cross-track: zero, over the **whole** OOXML chart surface

Not a sample. The corpus holds exactly **one** `.docx` and **one** `.xlsx` carrying a
`charts/chart*.xml` part — `words/batch-020/docx/ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx`
and `sheets/batch-010/xlsx/Keywords_Mapping_Graphs_and_Charts.xlsx`. Rendered on both legs:
**both byte-identical.**

That is the measurement; the proof is stronger. `DrawingChartPlot.Read` has exactly three callers
in the tree, and only the presentations one supplies a matrix:

- `XRay.Office.WordProcessing/Ooxml/DocxPictures.cs:208` — no `styles` argument at all;
- `XRay.Office.Spreadsheets/Ooxml/XlsxDrawings.cs:272-273` — `styles: null`, written literally;
- `XRay.Office.Presentations/Ooxml/PptxSlideLayoutChart.cs:57` — `theme.Styles`.

With a null matrix `ThroughSubtleLineStyle` returns its input. `WithNoFormatMatrixTheAccentIsRaw`
pins that so the two tracks stay zero when someone later wires a matrix into either.

---

## 6. Verdicts: **0 of 163**, predicted and confirmed

The gate columns of both changed renderings, `xray-office analyze` (in-process, no poppler):

| deck | leg | pages | alnum words | raw words | faces | unembedded |
|---|---|---:|---:|---:|---:|---:|
| `Demick_JetBlue` | base | 10 | 643 | 673 | 4 | 0 |
| | **branch** | **10** | **643** | **673** | **4** | **0** |
| | reference | 10 | 608 | 632 | 4 | 0 |
| `FAAAI…` | base | 30 | 1115 | 1201 | 6 | 0 |
| | **branch** | **30** | **1115** | **1201** | **6** | **0** |
| | reference | 30 | 1101 | 1187 | 6 | 0 |

Identical to the digit on every column. The other 161 renderings are byte-identical, so their
verdicts are identical by construction. **Slides stays 144 of 163 and 163 of 163 page-exact.**

This was predicted plainly and it is the correct outcome, not a weak one. The gate asks how many
pages, how many extractable words, and are the fonts embedded. A colour is none of the three, and
`Demick_JetBlue` remains a `words` failure at 643 against 608 for reasons this round did not
touch.

---

## 7. The prediction, scored

| # | predicted | measured | |
|---|---|---|---|
| P1 | slides renderings changed: **1**, `Demick_JetBlue` | **2** — `Demick_JetBlue` **and** `FAAAI…` | ❌, by the blind spot it named |
| P2 | words **0 of 200**, sheets **0 of 171**, zero by construction | 0 and 0, over the whole OOXML chart surface, plus the call-site proof | ✅ |
| P3 | verdicts moved **0 of 163** | **0**, every column identical to the digit | ✅ |
| P4 | 5 chart pages, `\|ink\|%` down on every one, none up; accents land on `B45D03`/`761D26`/`12415C` | **5 of 5 down, 0 up**; all three hex values exact | ✅ |
| P5 | fill half not implemented, census reach 0 | 5 decks have automatic fills, **0** resolve through anything but a bare `phClr` | ✅ |
| C1 | `Sector_Skills` must not change | it did not | ✅ |

**P1 is the instructive one and it failed in the honest direction.** The prediction said the
census could be one *low* as well as one high, and named the reason — that "auto stroke" was
decided from `c:ser/c:spPr/a:ln` having a fill. It was one low, and the miss turned out to be a
defect (`a:noFill` read as a statement) rather than a counting slip. A census that had been
right by luck would have taught nothing; this one located a second bug.

The prediction's other cost is smaller and worth naming: **P4 was confirmed by the operator
census and could not have been confirmed by the pixel diff on the second deck**, where the change
is real and the metric reads 4.73 / 0.40 / 0.41 on both legs. The blind spot was written down in
advance, which is the only reason the FAAAI result was not read as "unchanged".

---

## 8. Still open, in order, with numbers

1. **`c:minorGridlines`, unread at `DrawingChartPlot.cs:374`.** Censused this round by walking
   the chart parts: **3 decks, 12 instances** — `Demick_JetBlue` 10, `N2_E_Maestroni_Swarm_COP`
   1, `171128IPAP` 1. It needs a reader *and* a colour: `Demick_JetBlue` page 4 has the
   reference stroking 49 `#8B8B8B` and 35 `#666666` where we stroke 21 `#B3B3B3`, so
   `GridProperties.cxx:64-66`'s `0xB3B3B3` default is no longer what 26.2.4.2 draws either. This
   is the remaining ~90% of that deck's chart residual.
2. **A marker's own `c:marker/c:spPr`.** `ChartLayout.cs:2089` and `ChartLayout.Plots.cs:134`
   draw markers from the series colour. Needs a `ChartSeries` member in `XRay.Office.Core`, so it
   owes a cross-track sweep. It is what makes `FAAAI…` right rather than accidentally right.
3. **`a:noFill` on a series line.** `DrawingChartPlot.cs:1423` returns `null`, and `:797`'s
   `?? autoLine` then draws the line the file suppressed. Must land *after* item 2 or it makes
   `FAAAI…` worse.
4. **The fill half of the automatic tables**, `THEMED_STYLE_SUBTLE`/`INTENSE` into
   `getFillStyle`. Measured reach **zero** on this corpus. Do not spend a round on it; the
   remark in `FilledSeriesFills` is now backed by a measurement instead of an argument.

---

## 9. Tests — 10 added, **10 verified by reintroduction, 0 drift guards**

`dotnet/tests/XRay.Office.Presentations.Tests/DrawingChartThemedLineTests.cs`. Every mutation was
applied to a clean tree, built, and named a failing test by `verify-test.sh` (exit 0).

| mutation | detected by |
|---|---|
| M1 `return stroke ? ThroughSubtleLineStyle(…) : placeholder` → `return placeholder` (the original defect) | `AnAutomaticStrokeIsTheAccentPutThroughTheThemesSubtleLineStyle`, `TheSameThreeComeOutOfColourOfDirectly`, `AThemeLineStyleStatingALiteralColourOverridesTheAccent`, `TheThemeActsOnTheCycleShadedAccentAndNotOnTheBareOne` |
| M2 apply it to fills as well as strokes | `AFillIsNotPutThroughTheLineStyle` |
| M3 `SubtleStyleIndex = 1` → `2` | `ItIsTheFirstLineStyleAndNotTheSecond` |
| M4 stop threading `automatic.Styles` at the call sites | the same three as M1 |
| M5 null matrix returns black instead of the accent | `WithNoFormatMatrixTheAccentIsRaw`, `TheThemeActsOnTheCycleShadedAccentAndNotOnTheBareOne` |
| M6 a theme entry with no `a:solidFill` returns a literal instead of the accent | `AThemeLineStyleWithNoSolidFillLeavesTheAccentAlone` |
| M7b hardcode a −5% darkening instead of reading the theme's own transform | `AThemeStatingNoTransformLeavesTheAccentAlone` + 4 others |
| M8 `LineOf(...) ?? autoLine` → `autoLine ?? LineOf(...)` (merge order inverted) | `AStatedColourStillWinsOverTheThemedAccent` |

**One mutation was not detected and it was not a defect.** M7 inserted
`ThroughSubtleLineStyle(resolved, null, theme)` inside `Resolve` to swap the order of the cycle
shade and the theme transform — but with a `null` matrix that call is the identity, so the
mutation was an equivalent formulation rather than a defect. Recorded as such rather than as an
undetected mutation, because those are different findings.

**No fixture was added.** Every case is built from markup in the test file, and every value in it
is read out of `Demick_JetBlue.pptx` or its reference PDF — no third-party content enters the
repository.

---

## 10. Final state

```
dotnet build -v q -nologo     0 Warning(s)   0 Error(s)
```

| project | before | after |
|---|---:|---:|
| Core | 305 | 305 |
| Containers | 109 | 109 |
| Text | 289 | 289 |
| Vector | 295 | 295 |
| Rendering | 149 (1 skipped) | 149 (1 skipped) |
| Markup | 259 | 259 |
| OpenDocument | 125 | 125 |
| WordProcessing | 783 | 783 |
| Spreadsheets | 650 | 650 |
| Presentations | 613 | **623** |
| **total** | **3577** | **3587**, 0 failed, 1 skipped |

`XRay.Office.Fidelity.Tests` was **not run** — another agent owns it.

---

## 11. Measured versus inferred

**Measured or read directly:**

- Both full sweeps of the 163, re-counted from disk (163 / 163), byte-compared raw and
  date-normalised, and the two differing names.
- Every colour count in §5.1 and §5.2, out of the PDF content streams of both legs and the
  canonical reference, counting `rg` and `RG` alike.
- Every `ink%` / `|ink|%` figure, with the invariant asserted per leg and both known-answer
  controls run before any of them was believed.
- All six gate rows in §6, from `xray-office analyze` in process.
- The census, and the fill-half census, from parsed OPC parts.
- Both cross-track renderings on both legs, and the three `DrawingChartPlot.Read` call sites.
- `objectformatter.cxx:740-889`, `:246-305`; `DrawingStyleMatrix.cs:143-296`;
  `DrawingChartPlot.cs:374`, `:787-800`, `:1289`, `:1348-1362`, `:1392-1426`.
- Every reintroduction row in §9, from `verify-test.sh` exit 0 with named tests.
- `check-env.sh`, quoted at the head of this file.

**Inferred, and flagged:**

- **That slides stays 144 of 163.** Not re-run as a full gate sweep. It follows from two measured
  facts — 161 renderings byte-identical, and the 2 that changed having identical gate columns —
  but the 144 itself is the brief's figure, taken on trust.
- **That the reference's 46-record polylines against our 23 are a splitting difference rather
  than missing ink.** Read from the counts and the unchanged `|ink|%` structure, not traced
  through the path construction.
- **That the residual on `Demick_JetBlue`'s chart pages is dominated by the minor grid.** The
  colour census makes it very likely (84 grid records in the reference against 21 in ours) but no
  experiment isolated the grid's contribution.
- **That `FAAAI…`'s 8 marker fills are what changed and nothing else on that page.** Established
  by pairing the records by coordinate, which is a strong match but not a proof of exhaustiveness.

## 12. Files

- `prediction.md` — committed as `58ce91f56d5` before measurement, unedited.
- `census.py` / `census.tsv` — the part-walking census; `census.py` is importable and the
  fill-half census reuses its resolvers.

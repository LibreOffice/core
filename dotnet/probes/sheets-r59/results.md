# Round 59 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r59`, base
`dc9ca5900c2`. Read `prediction.md` (`aca409a6129`) beside this file first — it was committed
before a line of the change was written and before anything was rendered post-change.

## 1. Baseline: **277 of 307**, one above the briefed 276, and only the reference side moved

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 363 MATCH 301 MISMATCH 62`; scored against
`MANIFEST.tsv`'s 307 sheets paths by round 58's `score.py`, which refuses to print unless every
manifest path found a row: **277 match / 30 mismatch**.

Differenced document by document against round 58's final sweep, **exactly one row moved and it is
the reference half**: `unstable-001/xlsx/fse_identification_form.xlsx`, `REF words 427 → 440`, with
our side pinned at 440 in both sweeps. That is the calendar volatility round 57 separated, on the
document round 58 named. **The comparable-to-round-58 figure is 276 and the live one is 277.**

`TOTAL` was **363** here where round 58's first two sweeps said 325 — the alias count is not
static, as `CLAUDE.md` now records. Distinct lower-cased paths: 307 in both sweeps.

## 2. Result: **277 → 278**, and the volatile document takes one back

| document | before | after | ours/ref |
|---|---|---|---|
| `chartset-002/xlsx/003_advanced_excel_pie.xlsx` | `words` | **match** | 138 → **143** / 143 |
| `chartset-004/xlsx/019_advanced_excel_pie.xlsx` | `words` | **match** | 135 → **140** / 140 |
| `chartset-002/xlsx/011_advanced_excel_pie.xlsx` | `words` | `words` | 135 → 136 / 140 |
| `chartset-002/xlsx/027_advanced_excel_pie.xlsx` | `words` | `words` | 135 → 136 / 140 |
| `chartset-004/xlsx/microsoft_learn_multi_chart_examples.xlsx` | `words` | `words` | 203 → **200** / 225 |
| `unstable-001/xlsx/fse_identification_form.xlsx` | `match` | `words` | **the reference's clock**, 440 → 427 |

**Two verdicts gained on our side; one lost on the reference's.** No `done` document moved on our
side at all — the three `done`/`pagination`/`table` rows that appear in the diff moved only their
reference word counts (`PBN Matrix` 5549 → 5544, `SIL_TDB648` 7497 → 7498,
`FAA-2019-0995-0002` 9990 → 9995), all date-bearing.

**Against the prediction of +4, this is +2, and the two that did not move are named and
explained in § 6.** `microsoft_learn_multi_chart_examples` moved 3 words *away* from the reference
and is a regression in distance, not in verdict; it is also the document that refuted my own reach
census (§ 5).

## 3. Round 58's "our pie is 18% larger" is refuted, and the cause is the instrument

`pdf-ops.py`'s bounding box for a path includes a bezier's **control points**. We emit one cubic per
arc segment and LibreOffice emits a polygonised arc, so our wedge boxes bulge outside the circle by
up to **8.11 pt** and the reference's by **0.11**. Taking the union of the five wedge boxes — which
is what produced 235.8 — measures our control polygon, not our pie.

Read instead off the wedge corner: the first wedge runs from twelve o'clock through 62.6°, so it
lies wholly in the upper-right quadrant and its box's lower-left corner **is** the pie centre, with
its top edge at centre + radius. That is exact for both renderers.

| `003_advanced_excel_pie` p1 | ours (base) | reference |
|---|---:|---:|
| centre | (402.48, 461.56) | (408.84, 464.74) |
| radius | 110.62 | **99.78** |
| diameter | 221.24 | 199.56 |

**10.9% larger, not 18%.** The centre offset in the brief — 6.4 pt left, 3.2 pt low — reproduces
exactly at 6.36 and 3.18.

## 4. The law, read off the reference before the C++ was opened

`probe-pieradius.py`: sixteen one-variable rewrites of the witness's own chart part, rendered
through the installed binary, with the geometry read back from the first wedge.

| variant | radius | |
|---|---:|---|
| `01-nolabels` | 110.44 | the control — no rule at all |
| `07-pos-ctr`, `08-pos-inEnd` | 110.44 | **our own geometry, to 0.16%** |
| `04-cat-only`, `05-val-only`, `15-cat-only-16pt` | 110.44 | short label text — the shrink is not a constant |
| `00-asis` (`bestFit`) | **99.78** | |
| `09-pos-outEnd` | **99.78** | *identical to `bestFit`, to the digit* |
| `13-label-8pt` | 101.28 | |
| `14-label-16pt` | 72.08 | |
| `03-notitle` | 121.61 | |
| `12-bare` | 131.44 | |

So **our geometry was never wrong; it was right for the wrong placement**, and the shrink is driven
by what the labels consume. Word counts of the same rewrites, which bracket the answer:
`inEnd` **138 — our exact figure**, `ctr` 139, `bestFit` **143**, `outEnd` **146**. The gate's band
is `d > ref×0.02 && d > 3`, both conditions, so 143 ± 3 passes: **`outEnd` for every label
overshoots as surely as `ctr` undershoots.**

Four further laws, each measured on the reference's own page before `PieChart.cxx` was read:

* the label's legend key is `int(fontHeight × 0.6)` = 211 mm100 = **5.98 pt** — measured 5.98;
* the key-to-text gap is `key + max(100, fontHeight × 0.22)` = 311 mm100 = **8.818 pt** — measured
  8.81;
* the `OUTSIDE` anchor is the rim point on the bisector plus a flat **150 mm100 = 4.252 pt** in the
  radius direction, and the eight-way alignment table decides which corner of the label block sits
  on it. Predicted x 462.87 against a measured 462.90, and across all five slices of the `outEnd`
  rewrite the four alignment families land the block's left or right edge on the anchor to
  **0.12 pt**;
* the block's height is **n × (ascent + descent)** with the first baseline at `blockTop − ascent`,
  which lands all five `outEnd` baselines to **0.09 pt**. **The 0.30-em vertical text inset that
  `ShapeFactory::createText` sets is not in it** — the model that includes it misses by 4.5 pt, and
  the horizontal 0.18-em inset is likewise absent (it would put the text 1.8 pt right of where it
  is drawn).

### The port of `performLabelBestFitInnerPlacement` is verified against the reference's own answers

`check-bestfit.py` runs the predicate on five cases whose outcome the reference states from the
outside: the four labels drawn on two lines near the pie fitted, and the one drawn on one line
beyond the rim did not. Given the reference's own block width — **74.0 pt**, which is what its four
inner placements imply — the port returns

| slice | reference | port | agreement |
|---|---|---|---|
| M1, 1 line, 88.16 wide | outside | **fails** | ✓ |
| M2 | inside at (59.05, −3.93) | **(59.09, −3.96)** | **0.04 pt** |
| M3 | inside at (24.84, −63.80) | **(24.87, −63.85)** | **0.05 pt** |
| M4 | inside at (−55.86, −18.15) | **(−55.74, −18.15)** | **0.12 pt** |
| M5 | inside at (−50.70, 31.10) | **(−50.59, 31.12)** | **0.11 pt** |

**Four independent inner placements to 0.12 pt and the one failure reproduced as a failure.** The
port is not the residual.

### The reference draws six label keys for five labels, and that identifies the mechanism

`xShapes->remove(aPieLabelInfo.xTextShape)` in the outside fallback takes the *text* away and leaves
its sibling key behind, so the discarded inner attempt's key stays on the page. The reference draws
`#4F81BD` squares at both **(390.67, 504.20)** and **(462.90, 556.07)**. It is reproduced, and
`PieLabelKeepsTheDiscardedInnerKey` pins it.

## 5. What changed, measured

| | base | after |
|---|---:|---:|
| `003_advanced_excel_pie` radius | 110.62 | **104.13** (reference 99.78) |
| its centre | (402.48, 461.56) | **(412.64, 468.05)** (reference (408.84, 464.74)) |
| label keys drawn on its page 1 | **0** | **6** (reference 6) |
| labels wrapped onto two lines | 0 | **4** (reference 4) |
| labels rebuilt outside the rim | 0 | **1** (reference 1) |
| its words | 138 | **143** — the reference's figure exactly |
| sheets verdicts | 277 | **278** |

The label *structure* is now the reference's exactly: one nineteen-glyph label outside the rim with
its key, four twenty-glyph labels wrapped after the value and placed inside their slices, and the
sixth key of the discarded attempt.

### The residual, and the experiment that was run and rejected

Our radius is **4.4% too large** where it was 10.9%, and the centre 3.8 pt right and 3.3 pt low
where it was 6.4 left and 3.2 low.

`probe-oursvsref.py` compares our radius with the reference's over eight rewrites. The four
controls — `01-nolabels`, `07-pos-ctr`, `04-cat-only`, `12-bare` — agree to **0.16–0.24%**, so the
base geometry is exact and the residual is entirely in the second pass.

**A one-bit alternative was tested and rejected on the measurement.** If the label's group box
carried `ShapeFactory::createText`'s 0.18/0.30 em insets, the fit test would be stricter and the
consumed rectangle larger. Adding them:

| | no insets (shipped) | with insets |
|---|---|---|
| `00-asis` radius error | 4.36% | **1.47%** |
| `14-label-16pt` | 7.38% | **0.72%** |
| the four controls | 0.16–0.24% | 0.16–0.24% |
| `003` label structure | **1 outside, 4 wrapped inside — the reference's** | 3 outside, 2 inside |
| `003` centre error | 3.8 pt | **13.6 pt** |
| `003` words | **143 (reference 143)** | 146 (the `outEnd` figure) |

**The insets buy the radius and lose the page.** They are also contradicted directly: the
reference's text is drawn at `blockLeft + 8.818` with no left inset and its baseline at
`blockTop − ascent` with no upper inset, both measured to 0.03 pt. Rejected, and recorded here
rather than silently dropped.

### The residual's real cause is a text metric, and it is now measured

**Our chart label line height for Carlito is 1.2207 em and the reference's is 1.1219 em at 10 pt**
— 12.21 pt against 11.23, an 8.8% difference, read off the two-line labels of the same document on
both sides. Our ascent is 9.51 pt against the reference's 9.00.

**Our single-line labels nevertheless agree with the reference to 0.01 pt, because the two errors
cancel**: a label is drawn at `blockCentre − blockHeight/2 + ascent`, and we are 0.50 too tall and
0.51 too high in the ascent. Round 58's agreement on the `ctr` placement was that cancellation, not
a correct metric. The moment a label has two lines, or its box is measured for a fit test, the
error is live.

**And it is not a constant factor.** At 15.89 pt the reference's two-line spacing is **19.45 pt =
1.2241 em** — where ours is 1.2207, agreeing to 0.4%. So the reference's Carlito line height is
**sub-linear below about 16 pt** and `ascent + descent + lineGap` scaled linearly cannot be right at
both sizes. Carlito's hhea, OS/2 typo and OS/2 win metrics all give 1.2207, so the reference is not
reading a different table. Two points, no law: named for the next round rather than fitted here.

That metric is what decides the two pies that did not flip. Their pass-1 label boxes are 8.8% taller
than the reference's, which changes which labels leave their slices, which changes the consumed
rectangle, which decides both the shrink and the position:

| | `003`/`019` | `011`/`027` |
|---|---|---|
| pass-1 consumed rect | `(232.12, 269.72, 320.39, 234.23)` | `(291.86, 269.68, 260.17, 233.84)` |
| a label overflows to the **left** | yes | **no** |
| `nDiffLeft` | 57.22 | **116.96** |
| resulting centre x | 412.64 (reference 408.84) | **383.01** (reference 411.11) |

`VDiagram::adjustInnerSize` does `if (nDiffLeft >= 0) aNewPos.X -= nDiffLeft`, so when nothing
overflows on the left the diagram is slammed against the available rectangle's left edge. That is
the reference's own arithmetic, faithfully ported; it is the *input* that differs.

## 6. Prediction against measurement

| | predicted | measured |
|---|---|---|
| sheets verdicts | **277 → 281** | **277 → 278** — WRONG, +2 not +4 |
| `003_advanced_excel_pie` | 140–145 words | **143**, the reference's own figure |
| `019_advanced_excel_pie` | ≥ 137 | **140**, exact |
| `011`, `027` | ≥ 137 | **136 — WRONG by one word each** |
| page counts, anywhere | 0 | **0** |
| its pie radius | 99.78 ± 0.5 | **104.13 — WRONG**, 4.4% over |
| its pie centre | (408.84, 464.74) ± 0.5 | **(412.64, 468.05) — WRONG**, 3.8 and 3.3 out |
| label legend keys on `003` p1 | 0 → 6 | **0 → 6** |
| sheets documents whose ink changes | 5 | **7** (§ 7) — under-reached |
| `Keywords_Mapping…` | keys appear, words unchanged | **words unchanged; no keys appear** — its charts are all `barChart` and the key is drawn only on the pie path |
| tests | +8 to +18, `XRay.Office.Core` | **+8, `XRay.Office.Core` only** |
| `MANIFEST.tsv` | 4 rows change status | **2 do** |

**Six of twelve.** Every miss is on the same axis — the shrink's magnitude and therefore its
position — and chasing it is what found the line-height divergence, which is a defect nothing on
this project had measured.

## 7. Shared layer — **yes**, and my own reach census was refuted by the sweep

The diff is `XRay.Office.Core/Charts/*` and `XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs`. **Both
reach all three tracks and the parent owes the cross-track sweep.**

The prediction's census matched `<c:pieChart>` and answered **5 sheets, 3 slides, 2 words**. It was
wrong in two independent ways:

1. **A chart part may bind the chart namespace as the default, with no `c:` prefix.**
   `microsoft_learn_multi_chart_examples.xlsx` does, and the census reported it as holding no chart
   of any kind. **Its word count moved in the sweep, from a document the census said could not be
   touched.** A prefix-anchored regex census is not a census.
2. **`c:dLblPos` is optional and a pie's default placement is `bestFit`**
   (`typegroupconverter.cxx:95-107`). The prediction file named this as blind spot 3 and said the
   figure was "a floor, not a ceiling"; it was.

`census-piereach.py` corrects both, excludes doughnuts (`bMovementAllowed && !m_bUseRings` gates
the mechanism off for a ring chart) and reads all 946 manifest rows:

| family | documents | |
|---|---:|---|
| **sheets** | **7** | 4 pies + `microsoft_learn_multi_chart_examples` + `005_Contextures_chart_sample_6e279b08` (done) + `003_Contextures_chart_sample_9bda2719.xlsm` (done) |
| **slides** | **5** | `chartset-005/…076_Hexadonut…`, `chartset-006/…031_Alarm_Clock_Pie-Chart…`, `chartset-012/…061_Four-Stage_Serpentine_Chart…`, `done-007/bitesize-writing-a-report.pptx`, `done-011/3495.pptx` |
| **words** | **4** | `chartset-001/pie-chart-result.docx`, `chartset-001/pie-chart-template.docx`, `chartset-009/027_Unit_Circle_Chart_Graphical_Chart…docx`, `chartset-012/021_Unit_Circle_Chart_3D_Pie_Chart…docx` |

The two `outEnd`-only slides decks and the two `outEnd`-only words documents are listed because they
are pie parts with data labels; **the `outEnd` placement itself was deliberately not changed this
round** — the old 1.1-radius rule is still there — so they should not move. That is a prediction the
parent's sweep can falsify.

**Both `done` sheets documents in the list were measured, not reasoned about**: neither
`005_Contextures_chart_sample_6e279b08` nor `003_Contextures_chart_sample_9bda2719.xlsm` moved a
page, a word or a font in the whole-track sweep.

`showLegendKey` was censused separately: **62 `val="1"` elements in 5 sheets documents, 0 slides,
0 words.** Four are this round's pies; the fifth is `done-010/Keywords_Mapping_Graphs_and_Charts`,
whose eleven chart parts are **all `barChart`** — so its 38 keys are still not drawn, the key is
implemented on the pie path only, and that document did not move.

## 8. The two censuses that had never been run — both negative for `colorScale`

`census-blindspots.py`, all **946** manifest rows, refuses to summarise unless every one produced
output.

* **`.xls` `CF12` (0x087A), read for the first time on this project**: **42 records in 2 documents,
  and all 42 are `ct=1` — `cellIs`.** Plus 18 `CONDFMT12` in one document. **There is no `.xls`
  colour scale, data bar or icon set anywhere in this corpus.** Round 58 guessed that by
  construction; it is now measured, and the record that no census here had read is closed.
* **The `x14` extension arm**: **17 blocks, 27 rules in 11 sheets documents — 9 `dataBar`, 18
  `iconSet`, zero `colorScale`.** So round 58's colour-scale reach of 38 documents and 43 rules is
  the whole-corpus figure and not an OOXML-arm subset. What the arm *does* add is eleven documents'
  worth of `dataBar`/`iconSet` that no census here had seen, listed in the probe's output.

## 9. `cellIs` was censused and not implemented, and the census is the deliverable

The round's budget went to the pie. What is now known, over the 243 xlsx-family sheets rows with
every row producing output:

* **123 `cellIs` rules in 18 documents**, by operator: `greaterThan` 65 (3 documents), `lessThan` 29
  (10), `equal` 25 (5), `lessThanOrEqual` 2, `between` 1, `notBetween` 1.
* **Every one of the 123 carries a `dxfId`** — there is no arm that needs a style name.
* **118 of the 125 operands are literals**: 95 bare numbers and 23 quoted strings. The seven that are
  not are `TODAY()` ×2, `TODAY()+30` ×2, `$B$4` ×2 and `$C$5` ×1 — so a comparison against a literal
  covers 94% of the corpus and the remainder needs three specific things, not a formula evaluator.
* **And the finding that changes the plan: 87 of the 123 `dxf`s state a font and no fill at all.**
  27 state a fill only, 9 state both. So a fill-only implementation reaches **36 of 123 rules**, and
  the majority of `cellIs`'s ink is a font colour. That was not known when the brief called it
  "cheap"; it is still cheap, but it is two arms rather than one.

## 10. Tests

**+8, all in `XRay.Office.Core`** (337 → 345). The ten non-Fidelity projects, re-derived:
Containers 109, Core 345, Markup 259, OpenDocument 125, Presentations 831, Rendering 153
(+1 skipped), Spreadsheets 974, Text 617, Vector 298, WordProcessing 1180 — **4891 passed, 0
failed, 1 skipped**. `dotnet build -v q -nologo` → **0 warnings, 0 errors**.

**Six mutations through `verify-test.sh`; four detected, two not, and the two are reported as what
they are.**

| mutation | outcome |
|---|---|
| the key is `0.5 × fontHeight` instead of `0.6` | **detected** — `ALabelLegendKeyIsSixTenthsOfTheFontHeightSquare` |
| the outside anchor uses `4 × 150` mm100 | **detected** — `OnlyTheLabelOnTheNarrowestSliceIsRebuiltOutside`, `PieLabelKeepsTheDiscardedInnerKey` |
| the wrap allowance is `2.0 × radius` instead of `0.8` | **detected** — `ALabelWiderThanFourFifthsOfTheRadiusWraps` |
| the diagram is never shrunk | **detected** — `ALabelRebuiltOutsideTheRimShrinksTheDiagram` |
| a ring chart takes the pie label path | **detected** — `ADoughnutIsNotBestFitted` |
| the `d(P,F) > r` early-out is relaxed to `2r` | **NOT detected** — and it is an equivalent formulation on these inputs, not a drift guard: every case it would have rejected is rejected again by the `CP`/`CM` angle test a few lines later. A fixture where the diagonal exceeds the radius while the vertex rays stay inside the slice would separate them; the corpus holds none |
| `HasBestFitLabels`'s `Rings` clause is dropped | **NOT detected** — `AddRing`'s own `hole > 0` guard already keeps a doughnut off the pie label path, so the clause is redundant for the labels and matters only to the shrink, which is a no-op when nothing leaves the diagram. Two independent gates; the mutation removes one of them |

## 11. The vision round

Three fresh subagents, one composed pair each, `Read` on one image path only, no project
documents, no source, no shell. Each was asked to describe the halves separately, to give a
direction, and to say what looked identical. **No page was chosen by `--worst`.**

### `003_advanced_excel_pie` p1 — chosen because it is the page the change targets

The reviewer, who had never seen the document:

> *"The fifth label sits **outside** the pie, up and to the right of the blue slice … It is preceded
> by a small filled blue square marker (roughly 10×10 px) and reads in full: 'M1; Actual; 93; 17%'."*

and, on the other four:

> *"All four interior data label strings are identical, including the same line-break position after
> the semicolon ('M2; Actual; 100;' / '19%' etc.), the same font, the same size, and the same
> placement relative to their own slice."*

**That is the whole of this round's change seen from outside by a reader who has never seen the
code** — the outside rebuild, its key, and the wrap — and `pdf-ops.py` confirms it independently:
our key at (468.93, 563.84) and text at 477.75 against the reference's (462.90, 556.07) and 471.71,
and 16+3 glyphs against 17+3 on each of the four wrapped labels.

The same reviewer, on the reference's truncated label:

> *"The reference renders only 'M1; Actual;' … The reference's string stops dead at x≈1868, the same
> x as its chart frame's right border … the classic signature of clipping rather than omission."*

**Confirmed, and with the correction that matters.** The reference's *text layer* carries the whole
19-glyph run on **both** pages — `(471.71, 555.82)` on page 1 and `(8.94, 555.82)` on page 2 — and
`pdftotext` decodes all four tokens on each. Only the raster is clipped. That distinction is
precisely why the page-2 extraction can gain words at all, and it is what the parent's reading of
"page 1 carries `M1; Actual;` and page 2 carries `93; 17%`" describes at the pixel level rather than
at the text level.

### `011_advanced_excel_pie` p1 — chosen because it is the one that did *not* flip

An independent reviewer on an unrelated page reported, unprompted:

> *"The reference's chart content is shifted right relative to the frame's left edge, by roughly
> 50 px … both shifts agree at about +50 px, so it is the whole chart interior moving, not just one
> element."*
> *"The reference's pie is slightly smaller than ours … smaller by roughly 16 px, i.e. about 4–5% of
> its diameter."*

**Both confirmed by a second instrument, on the same objects.** `pdf-ops.py` gives our centre
(383.01, 467.86) against the reference's (411.11, 464.57) — 28.1 pt, which is 47 px at this
composition's 1.68 px/pt — and radii 104.32 against 99.78, **4.5%**. Two independent readers, two
unrelated pages, and a measurement that names the same objects: that is the discriminator
`HANDOVER.md` § 7 asks for, and it holds here.

### Refuted, for the fourth and fifth reader on the same page and the same split

**Both** reviewers reported that the reference's chart frame is narrower on the right, or has no
right border at all — reviewer A at *"about 115 px further left than ours"*, reviewer B *"I cannot
see a right-hand vertical border for this frame anywhere"*. Reviewer A also read the reference's
title as *"6–8% wider"* and *"about 95 px outside its frame's right border"*.

`pdf-ops.py` on both halves: the frame is `(164.98, 341.02)-(674.99, 624.37)` in ours and
`(165.37, 341.26)-(674.82, 623.93)` in the reference — **the same rectangle to 0.4 pt** — and the
title is 18.00 pt Carlito-Bold at x = 333.29 in ours against 18.01 pt at 333.19 in the reference.
Both pages are A4, **595.3 pt wide, and the frame's right edge is at 675 pt**, so on page 1 the
frame runs off the right of the MediaBox in *both* renderings and neither draws a right border there.

`HANDOVER.md` § 7 records this page producing a page-split misreading for three rounds under the
sentence "we draw no legend". **This is the fourth and fifth reader and a new sentence for the same
cause.** The lesson is not that the readers are unreliable — the same two readers produced the two
confirmed findings above — it is that *this page's horizontal split is a reliable generator of
false negatives about anything near its right edge*, and any claim about that region needs
`pdf-ops.py` before it is believed.

### `microsoft_learn_multi_chart_examples` p4 — chosen because its words moved *away*

The reviewer describes two entirely different pictures: ours draws *"a large solid olive/yellow-green
filled shape … clipped at the top page edge"* with a tapering red band, the reference *"a solid
medium-blue rectangle … about 426 × 45 px"* inside a faint grey plot frame. Both show `Q2 Q3 Q4` and
neither shows `Q1`.

**Not chased.** It is a pagination and chart-type divergence on a document that fails by 25 words
and has nothing to do with pies; it is recorded because the page was rendered for a stated reason
and the reading should not be lost. The document's *pie* is `chart4.xml` and the change did reach it
— that is how the census refutation in § 7 was found.

## 12. The 24.2.7.2 audit

Counters re-derived at this tree with the file's own commands: **39 open hits in 27 files**;
**22 marker lines — 19 `VERIFIED`, 2 `FIXED`, 1 `WRONG`, 0 `UNDECIDED`**. Round 58 reported 39 open
and 21 markers (18/2/1) at its own branch tip; the extra `VERIFIED` is round 56's `SlideDrawing.cs`
arriving through the merge, so the two readings are consistent.

**No site was re-checked this round**, and that is a deliberate choice rather than an omission:
`XRay.Office.Spreadsheets` is ten of ten and nine correct, and the round's whole budget went into the
pie subsystem. The next one is named rather than guessed — **`XRay.Office.Core/Graphics/GlyphRun.cs`,
2 open sites**, because § 5 has just measured a live vertical-metric divergence in exactly that
area: the reference's Carlito line height is 1.1219 em at 10 pt and 1.2241 at 15.89, and ours is
1.2207 at both.

## 13. `MANIFEST.tsv`

Lives in the corpus repository and was **not touched**. **Two rows change status and one is
volatile**, and the change is proposed rather than made:

| path | proposed |
|---|---|
| `sheets/chartset-002/xlsx/003_advanced_excel_pie.xlsx` | `open` → `done` |
| `sheets/chartset-004/xlsx/019_advanced_excel_pie.xlsx` | `open` → `done` |
| `sheets/unstable-001/xlsx/fse_identification_form.xlsx` | **leave as it is** — its reference half moved 440 → 427 with our side pinned, for the second round running, and it is already in the `unstable` batch for exactly this |

## 14. What the next round should do first

1. **The chart label's vertical metrics.** Two measurements, no law: the reference stacks Carlito's
   chart-label lines at **1.1219 em at 10.01 pt** and **1.2241 em at 15.89 pt**, and its ascent at
   10 pt is 9.00 against our 9.51. `SheetBandText.ChartLineHeightAt` answers a face's own
   `ascent + descent + lineGap`, which is 1.2207 for Carlito at every size. **Our single-line labels
   agree only because the ascent error cancels the height error**, so this is invisible until a
   label wraps or its box is measured — which is now, everywhere. Closing it is what takes
   `011_advanced_excel_pie` and `027_advanced_excel_pie` the one word each they need, and it moves
   the pie's radius and centre on all seven affected sheets documents. The probe to write is a
   size series on one face, then the same series on a second face, with the two-line spacing read
   from a `c:separator` of `\n` so nothing depends on where the wrap falls.
2. **`cellIs`, in two arms rather than one** (§ 9): the fill arm is 36 of 123 rules and the font arm
   is 87. 118 of 125 operands are literals; every rule carries a `dxfId`. Where a `dxf` fill and a
   colour scale meet, the scale must keep winning.
3. **`c:dPt` per-point fills on bar charts** — 11 stated colours drawn as one on
   `003_Contextures_chart_sample`, 35 fills in 7 sheets documents, and the slides track has 144 on
   pies and 31 on bars. The cross-track census is in round 58 § 3.
4. **`showLegendKey` on a bar chart.** 38 keys in `done-010/Keywords_Mapping_Graphs_and_Charts`
   alone, still undrawn: this round implemented the key on the pie path only.
5. **`c:dLblPos="outEnd"` still takes the old 1.1-radius rule**, which this round measured to be
   wrong: the reference's OUTSIDE anchor is `radius + 150 mm100` with the block's *edge* on it, not
   its centre. Four slides and words documents state it. Left alone deliberately because this track
   cannot measure them.
6. **`ChartLayout.IntervalsThatFit`** — untouched for a fifth round; round 56 § 9 has the census.

# Round 60 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r60`, base
`c17996f89cb`. Read `prediction.md` (`3511eb47f00`) beside this file first — it was committed
before a line of the change was written and before anything was rendered post-change.

## 1. Baseline: **278 of 307**, the briefed figure exactly

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 363  MATCH 303  MISMATCH 60`; scored by
round 58's `score.py` against `MANIFEST.tsv`'s 307 sheets paths, which refuses to print unless
every manifest path found a row: **278 match, 24 `words`, 5 `pages,words`**.
`fse_identification_form.xlsx` is *inside* the mismatches at 440/427, so the live 278 already
carries the volatile document on its losing side and the brief's warning did not have to be used.

The pie geometry reproduced round 59's residual to the hundredth before anything was changed —
`003` centre (412.64, 468.05) radius 104.13 against the reference's (408.84, 464.74) and 99.78,
where round 59 recorded 412.64/468.05 and 104.13. The instrument is the wedge corner, not a
bounding box.

## 2. The law: a chart's text is quantised onto a **96 dpi** device, and the line gap is not in it

Round 59 left two points and no law. `probe-chartvmetrics2.py` renders **117 one-variable
rewrites** of `003_advanced_excel_pie.xlsx`'s own chart part through the installed binary —
thirteen sizes from 6 to 40 pt on **Carlito, Liberation Sans and Liberation Serif**, each as a
one-line and a three-line chart title, plus a single-line `dLblPos="ctr"` data-label series —
and `law-chartvmetrics.py` reads them back.

**Every one of the 39 measured baseline pitches is an integer multiple of 0.75 pt**, which is one
pixel at 96 dpi, and the integer is

    hpx   = round(size_pt × 96 / 72)
    pitch = ( round(ascender/upem × hpx) + round(−descender/upem × hpx) ) × 72/96

with the `hhea` **line gap excluded**.

| law | max error | mean | exact |
|---|---:|---:|---:|
| **pixel, no gap** | **0.089 pt** | **0.036** | 7/39 |
| pixel, with the gap | 1.498 | 0.473 | 6/39 |
| continuous, no gap | 0.988 | 0.309 | 0/39 |
| continuous, with the gap — **what we shipped** | 2.476 | 0.613 | 0/39 |

Carlito's `hhea` gap is zero and both Liberation faces' is not, so the three faces separate the
gap term outright; the sizes where `size × 4/3` is not a whole number separate the pixel rounding
from exact scaling. The 0.089 residual is the instrument's own — the em is read off a PDF text
matrix to two decimals and each rendering is divided by its own `drawn em / stated em`.

The **ascent** is the same rounding, measured independently: a CENTER label's block centre `C` is
size-independent because `ctr` does not shrink the diagram, so `C = y1 − (H/2 − A)` must come out
constant over the size series. It does, to **0.042–0.069 pt** under the pixel law, against
**0.58–0.70 pt** under the model we shipped. At 10 pt in Carlito the pixel law gives H = 11.25 and
A = 9.00 where the reference draws 11.23 and 9.00.

**This is not a new mechanism in the tree.** `MetricGrid` already carries exactly this arithmetic
for Impress (600 dpi) and Calc (720 dpi), including EditEngine's max-of-two-roundings height and
its no-external-leading ascent. The defect was that `SheetBandText.Ungridded` dropped the grid
*entirely*: **chart2 does not use no device, it uses a different one**, and 96 dpi is the platform
default a `VirtualDevice` created with no `RefDevMode` keeps.

### The first cut of the probe could not measure it, and that is recorded rather than dropped

`probe-chartvmetrics.py` tried to read the pitch off the pie's own data labels. Above about twelve
point the five `ctr` labels overlap and the runs cannot be grouped into blocks; it refuses to
summarise and says which cases failed. The title is the clean witness for the pitch and the
value-only label for the ascent, which is why there are two instruments and not one.

## 3. What shipped

* `XRay.Office.Text/Fonts/LineSpacing.cs` — **additive only**: `MetricGrid.Chart` = 96 dpi in
  1/100 mm. No existing caller's arithmetic changes.
* `XRay.Office.Spreadsheets/Layout/SheetBandText.cs` — `ChartLineHeightAt` takes that grid;
  `ChartAscentAt` is new; `SheetShapePainter`'s two calls move to `ShapeLineHeightAt`, which keeps
  the old ungridded arithmetic byte for byte and says at its site that it is **unmeasured**.
* `XRay.Office.Spreadsheets/Layout/SheetChart.cs` — the two drawing paths take `ChartAscentAt`.
  **The height and the ascent had to move together**: the old height was 0.50 pt too tall and the
  old ascent 0.51 too high, so single-line labels landed right by cancellation.

## 4. Result: **278 → 277**, and the three moves are three different things

| document | before | after | which side moved |
|---|---|---|---|
| `chartset-002/xlsx/011_advanced_excel_pie.xlsx` | `words` 136/140 | **`match` 137/140** | **ours, a gain** |
| `chartset-002/xlsx/003_advanced_excel_pie.xlsx` | `match` 143/143 | **`words` 139/143** | **ours, a regression** |
| `metrics-001/xlsx/ans_mappings_of_eccairs_terms.xlsx` | `match` 191/191 | `pages` 191/**190** | **the reference's, and it is non-deterministic — § 6** |

**Our own side is +1 and −1: net nought.** The printed −1 is the reference half of a document our
binary cannot touch, and § 6 proves that inside a single sweep and then again outside it. Three
`done` documents moved only their reference word counts (`PBN Matrix` 5544→5547, `SIL_TDB648`
7493→7497, `033_Event_planning_tracker` 503→505) and `047_Date_tracker_Gantt` moved its reference
by 25 words; all are date-bearing. `027_advanced_excel_pie` did **not** move at all and is still
136/140.

**Zero regressions among the other 76 `done` chart-bearing documents**, which is what the census
said the risk was. One of the 78 regressed and it is named above.

### What the change actually corrected, which the gate cannot see

| `003_advanced_excel_pie` page 1 | before | after | reference |
|---|---:|---:|---:|
| legend entry pitch | 15.04 | **14.09** | 14.08 |
| wrapped data-label line pitch | 12.21 | **11.25** | 11.23 |
| pie radius | 104.13 | 104.70 | 99.78 |
| pie centre x | 412.64 | 382.80 | 408.84 |
| pie centre y | 468.05 | **467.68** | 464.74 |

Two vertical quantities that were 7–8% wrong on **97 sheets documents** are now right to 0.02 pt.
The radius and the centre x went the wrong way, and § 5 says exactly why and hands the next round
the arithmetic.

## 5. The regression is understood, and it is the pass-1 consumed rectangle

`003`'s four lost words are the tokens `M1;` and `19%`, which the reference draws **twice** — once
on page 1 and once on the sliver of page 2 that the A4 MediaBox cuts the chart across. Our base
rendering duplicated them by accident of position; the corrected label boxes slid the diagram
left and the duplication went with it. `pdftotext` token histograms of both sides confirm it: the
reference has `Actual;` seven times, `M1;` twice and `19%` twice, our base had the same, and our
after has six/one/one.

A temporary trace at the call site (added, read, and removed — `ChartLayout.cs` is unchanged in the
commit) gives the real rectangles, in points:

```
outer  (174.90, 269.52)-(630.06, 490.96)     available, 455.16 x 221.44
area   (291.76, 269.52)-(513.20, 490.96)     the unshrunk diagram, squared and centred
003 consumed  (291.76, …)-(552.56, 502.99)   nDiffLeft = 116.86  -> slammed to outer.Left
019 consumed  (232.25, …)-(552.78, 503.17)   nDiffLeft =  57.35  -> centre 412.45
```

`VDiagram::adjustInnerSize` is transcribed correctly — it was checked line by line against
`chart2/source/view/diagram/VDiagram.cxx` — so the divergence is entirely in **what pass 1
consumes**. Solving the reference's own arithmetic backwards from its answer (centre x 408.84,
side 199.56) pins it:

* `consumed.Right = 617.34 − d` and `consumed.Left = 174.90 + d`, where `d = nDiffLeft`;
* `consumed ⊇ area` forces `consumed.Right ≥ 513.20`, hence **`d ≤ 104.14`** and therefore
  **`consumed.Left ≤ 279.04 < 291.76`** — the reference's pass 1 *does* put a label out to the
  left of the pie, and ours (after the fix) does not;
* its consumed height is **243.32** against our 233.5.

Our base rendering had `consumed.Left = 232.12` — too far left by 8–47 pt — which is why it landed
3.80 pt from the reference by cancellation. **The right answer is between our before and our
after**, and the quantity to fix is which labels pass 1 rebuilds outside.

### The one-bit alternative was tested again, with the corrected metric, and refuted again

`ShapeFactory::createText` really does set `TextLeftDistance/TextRightDistance = 0.18 × fontHeight`
and `TextUpperDistance/TextLowerDistance = 0.30 ×` (`ShapeFactory.cxx`:2279-2296), so a label
shape's bounding box — which is what `performLabelBestFitInnerPlacement` and the diagram group's
box are taken from — is 0.36 em wider and 0.60 em taller than its text. Round 59 rejected putting
those in the label box; **this round put them in the fit test only, which is a different change,
and it is refuted on its own measurement**:

| | shipped | insets in the fit test |
|---|---:|---:|
| `003` words (ref 143) | 141 | **146** |
| `011` words (ref 140) | 138 | **143** |
| `019` words (ref 140) | **140** | **143 — a regression on a passing document** |
| `027` words (ref 140) | 138 | 143 |
| `003` centre x (ref 408.84) | 382.80 | 395.23 |

It buys 13 pt of centre on `003` and pushes every document to the `outEnd` word count, exactly the
failure mode round 59 recorded — and this time it breaks a document that passes. Both cuts are
here rather than one silently dropped; the tree is back at the shipped state and was re-rendered to
prove it (141 words, centre 382.80).

## 6. `ans_mappings_of_eccairs_terms.xlsx`'s **reference** rendering is non-deterministic

This is the round's second measured result and it matters beyond this round.

The document has two directory entries on this mount — `.XLSX` and `.xlsx`, one inode — so a sweep
renders it **twice**, and the usual trap becomes an instrument: two independent renderings of the
same bytes by the same binary in the same sweep.

```
base  sweep   .XLSX  ref 191 pages / 27896 words        .xlsx  ref 191 / 27895
after sweep   .XLSX  ref 191 pages / 27897 words        .xlsx  ref 191 / 190 pages, 27897
```

Nine further renderings outside the sweep, three serial and six in parallel to reproduce the
sweep's load: pages **191 191 191 191 190 191 191 191 191**, words **28082 28081 28083 28081
28084 28082 28081 28081 28081**. **One in nine loses a page.** Our side is pinned at 191 pages and
27894 words in both sweeps.

The document holds no `TODAY`, `NOW`, `RAND` or `RANDBETWEEN`, so this is not calendar volatility
like `fse_identification_form`: it is instability in the reference's own layout of a 191-page
workbook. **Any sweep can lose this verdict at random**, and a round that changed something
unrelated would read it as its own regression.

## 7. Prediction against measurement

| | predicted | measured |
|---|---|---|
| sheets verdicts | **278 → 280** | **278 → 277 — WRONG**, and our own side is net nought |
| `011_advanced_excel_pie` | `words` → `match`, 136 → 140 | **`match`, 137** — right verdict, wrong number |
| `027_advanced_excel_pie` | `words` → `match`, 136 → 140 | **136, no movement at all — WRONG** |
| `003` stays `match` at 143 | | **139, `words` — WRONG**, and § 5 says why |
| `019` stays `match` at 140 | | **140 — right** |
| `003` radius within 1.5 pt of 99.78 | | **104.70 — WRONG**, it grew |
| `003` centre within 2 pt of (408.84, 464.74) | | **(382.80, 467.68) — WRONG** in x, closer in y |
| page counts, anywhere | 0 change | **0 on our side**; the reference moved one, § 6 |
| regressions among the 78 `done` chart documents | **0**, "most likely to be wrong" | **1**, and it was the one the round targeted |
| tests | +6 to +14 | **+40**, `XRay.Office.Spreadsheets` |
| other tracks | 0 documents, structurally | **0** — the `XRay.Office.Text` edit is a new constant |
| `MANIFEST.tsv` | 2 rows | **3**, and one of them is a regression |

**Four of twelve, and the two headline numbers are both wrong.** What the round got right is the
thing it set out to measure — the law — and what it got wrong is the consequence it predicted from
it. The prediction file named exactly this risk ("regressions among the 78 `done` chart documents:
0, and this is the number most likely to be wrong") and it was right to.

## 8. Shared layer

The diff touches `XRay.Office.Text/Fonts/LineSpacing.cs` — **one new static property, no existing
caller's arithmetic changed** — and three files in `XRay.Office.Spreadsheets`. Words and slides
cannot move **by construction** rather than by census: their chart text goes through `FrameChart`
and `SlideChart`, which have their own measurers and were not touched. The parent's cross-track
sweep should read **zero** movement on both; that is a falsifiable prediction and the point of
saying it this way.

`census-charttext.py`, both readers (an OOXML `charts/chartN.xml` part; a BIFF substream whose BOF
document type is `0x0020`), all **946** manifest rows, case-folded where it accumulates:

| family | documents holding a chart | parts |
|---|---:|---:|
| **sheets** | **97** — 78 `done`, 19 `open` | 154 |
| slides | 67 | 159 |
| words | 10 | 10 |

**All 97 sheets documents were re-laid-out and one of them regressed.** The census over-reaches by
construction (a chart on a hidden sheet is counted), which is the safe direction for a risk figure.

Slides and words probably carry the same defect — the law is a property of `chart2`, not of Calc —
and this round must not be read as having fixed it there.

## 9. Tests

**+40, all in `XRay.Office.Spreadsheets`** (980 → 1020). Re-derived by running each project:
Containers 109, Core 345, Markup 259, OpenDocument 125, Presentations 836, Rendering 153
(+1 skipped), Spreadsheets 1020, Text 624, Vector 298, WordProcessing 1188 — **4957 passed,
0 failed, 1 skipped**. `dotnet build -v q -nologo` → **0 warnings, 0 errors**.

Thirty-five of the forty are the reference's own measured pitches, one per (face, size).

**Four mutations through `verify-test.sh`, all four detected:**

| mutation | outcome |
|---|---|
| `MetricGrid.Chart` is 720 dpi (Calc's) instead of 96 | **detected** — 31 of 40 fail |
| the chart metrics drop the grid entirely (the pre-round arithmetic) | **detected** — 35 of 40 |
| the chart pitch keeps the external leading | **detected** — 17 of 40, and the 17 are exactly the two Liberation faces, which is the discriminator the test was built around |
| the drawing-shape path takes the chart device too | **detected** — by the three-answers test alone |

None is a drift guard.

## 10. The vision round

Three fresh subagents, one composed pair each at 200 dpi, `Read` on one image path only, no project
documents, no source, no shell, each asked to describe the halves separately, give a direction, and
say what looked identical. **No page was chosen by `--worst`.**

### `003_advanced_excel_pie` p1 — chosen because it is the document the round regressed

> *"The right pie's centre is about 45 px further right than the left's … which is what pushes the
> M1 label into the clip."*
> *"The left pie is larger: radius ≈180 px vs ≈165 px … roughly 9% bigger."*

**Both confirmed by a second instrument on the same objects.** `pdf-ops.py` reads the wedge corner
at (382.80, 467.68) r 104.70 against (408.84, 464.74) r 99.78 — 26.04 pt of centre, which is 45 px
at this composite's 1.73 px/pt, and 4.9% of radius against the reviewer's 9% (a reviewer's
diameter estimate off a 200 dpi composite is worth about a factor of two here, and the *direction*
is what was asked for and is right).

The reviewer also reported, unprompted, the whole of what round 59 shipped and this round kept:
*"the M1 outside label … preceded by a small blue square marker"*, the four interior labels
*"wrapped onto the same two lines at the same break point"*, and the reference's M1
*"cut off at the frame's right border"*.

### `011_advanced_excel_pie` p1 — chosen because it is the document the round gained

An independent reviewer, on the page that has generated a false negative for five readers:

> *"The whole pie sits further right on the right half … about 47 px further right."*
> *"Chart title is larger and lower on the right … about 17 px (≈8%) wider … baseline also about
> 16 px lower."*

The pie shift is confirmed: (383.04, 467.46) against (411.11, 464.57), 28.07 pt = 48 px. **The
"larger title" half is refuted for the third round running** — both titles are 22 glyphs of
Carlito-Bold, ours 18.00 pt at x = 356.81 and the reference's 18.01 at 356.91, agreeing to 0.10 pt.
The same reviewer also said our chart frame *"is at least ~130 px wider"* and marked it moderate
confidence *"since I cannot see where the left frame actually terminates"* — which is exactly
right: the frames agree to 0.4 pt and the composite crops ours at the divider. **Sixth and seventh
readers, same page, same illusion, and this one flagged its own confidence.**

### But the *vertical* half of that reading is real, and no metric had found it

Two blind reviewers on two unrelated documents both said the reference's chart title sits about
16 px lower. `pdf-ops.py`:

| | ours | reference | |
|---|---:|---:|---|
| `003` title baseline | 601.44 | 591.87 | **9.57 pt high** |
| `011` title baseline | 601.44 | 591.87 | **9.57 pt high** |
| title x | 333.29 / 356.81 | 333.19 / 356.91 | agree to 0.10 |

**Our chart title is 9.57 pt too high, identically on both documents**, and it was 601.61 before
this round's change — so this is **pre-existing and not caused by it** (the change moved it 0.17 pt).
It reaches every chart title in the corpus. That is a lead no gate column can see, produced by two
readers who had never seen the document, and it is the best-localised new item on the track.

### `005_Contextures_chart_sample_6e279b08` p1 — chosen because its words moved (296 → 300, still `match`)

Reported, **not confirmed, and recorded as leads rather than findings**:

> *"The right half draws a thin light-grey rectangle around all three charts; the left half draws
> none."* — our page draws 63 strokes to the reference's 66, so "none" is certainly wrong as
> stated; whether a chart *frame* is among them is unmeasured.
> *"The right's horizontal scale is roughly 2.3× wider … its bars and nearly all of its title are
> pushed past the page edge."* — a category-axis extent claim on a document that passes the gate.
> *"The left's four data labels are black and bold; the right's are white."* — `pdf-ops.py` says
> both sides draw them in 14 pt Carlito-**Bold**, so the weight half is refuted; it does not expose
> a text fill colour, so the colour half is **not yet checked** and needs an instrument that can.

## 11. The 24.2.7.2 audit

Counters re-derived at this tree with the file's own commands: **38 open hits in 26 files**;
**23 marker lines — 19 `VERIFIED`, 3 `FIXED`, 1 `WRONG`, 0 `UNDECIDED`**. Round 59 read 39 open in
27 files and 22 markers (19/2/1) at its own tip; the difference is the words track's `FIXED`
arriving through the merge at `c17996f89cb`, and the two readings are consistent.

**The brief's named next site is refuted.** It said *"`XRay.Office.Core/Graphics/GlyphRun.cs` is the
named next audit site precisely because this divergence is live there — a 24.2.7.2-era claim about
vertical metrics may be the seat."* It is not. `GlyphRun.cs`'s two open hits are at lines 347 and
369, and both belong to `LuminanceRecolour` — `a:blip/a:lum` brightness and contrast, PowerPoint's
washout, and `Bitmap::Adjust`'s `msoBrightness` branch. **There is no vertical-metric claim in that
file at all**, and the seat of this round's divergence was `SheetBandText.Ungridded` in
`XRay.Office.Spreadsheets`, which carries no 24.2.7.2 marker because it was never a 24.2.7.2 claim.
Round 59 named it from the text of its own § 12 rather than from the file; one `git grep` separates
them.

`XRay.Office.Text`'s four open hits are likewise already-marked sites whose *prose* mentions the
version — `MeasuredParagraph.cs` (VERIFIED, round words-r53) and `SystemFontResolver.cs` (VERIFIED,
rounds words-r53 and words-r54). That is the self-corruption this file warns about, still live in
the open count.

**No site was re-checked this round.** The budget went into the law and the sweep, and the round
would rather say so than mark a site it did not probe. The next one on this track should be
`XRay.Office.Ooxml/DrawingML/DrawingFill.cs` (1 open hit) or
`XRay.Office.Rendering/Images/RasterImageDecoder.cs` (1) — both untouched, both single-site, and
neither has anything to do with the last two rounds' subject, which is the point.

## 12. `MANIFEST.tsv`

Lives in the corpus repository and was **not touched**. Three rows change, and one of them is a
regression:

| path | proposed |
|---|---|
| `sheets/chartset-002/xlsx/011_advanced_excel_pie.xlsx` | `open` → **`done`** |
| `sheets/chartset-002/xlsx/003_advanced_excel_pie.xlsx` | **`done` → `open`** — 139/143, and § 5 has the arithmetic |
| `sheets/metrics-001/xlsx/ans_mappings_of_eccairs_terms.xlsx` | move to the **`unstable`** treatment — its *reference* rendering loses a page one time in nine (§ 6). Leaving it `done` means every future sweep has an 11% chance of reporting a phantom regression on it |
| `sheets/unstable-001/xlsx/fse_identification_form.xlsx` | **leave** — mismatched in both sweeps this round, calendar volatility, already in `unstable` |

## 13. What the next round should do first

1. **The pass-1 consumed rectangle** (§ 5). The arithmetic is solved: the reference's
   `consumed.Left ≤ 279.04` where ours is 291.76, its consumed height is 243.32 where ours is
   233.5, and `d ≤ 104.14`. What differs is which labels pass 1 rebuilds outside at radius 110.44.
   Closing it takes `003` back and should take `027` with it — and the shape-factory insets are
   **not** the answer, measured twice now, once per metric.
2. **The chart title is 9.57 pt too high** (§ 10) — pre-existing, identical on two documents,
   reaching every chart title in the corpus, found by two blind readers and confirmed by
   `pdf-ops.py`. Better localised than anything else open on the track.
3. **`cellIs` conditional formatting**, still not implemented and still two arms: 123 rules in 18
   documents, every one carrying a `dxfId`, 118 of 125 operands literal, **87 of 123 `dxf`s state
   a font and no fill**. Where a `dxf` fill and a colour scale meet the scale must keep winning
   (`fillinfo.cxx` order). Round 59 § 9 has the census.
4. **`c:dPt` per-point fills** — 35 in 7 sheets documents, 144 + 31 on slides.
5. **The same 96 dpi law for `SlideChart` and `FrameChart`** — 67 slides and 10 words documents
   hold a chart (§ 8) and neither measurer was touched. The probe is written and face-agnostic.
6. **`showLegendKey` on a bar chart** — 38 undrawn keys in `Keywords_Mapping_Graphs_and_Charts`.

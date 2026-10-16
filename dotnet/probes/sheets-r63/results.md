# Round 63 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r63`, base
`43142b73ccf`. Read `prediction.md` beside this file first — it was committed at `a7827a41421`,
before a line of behavioural code was written and before anything was rendered post-change.

Sweeps ran with `TMPDIR` on `/c/sandbox/workdir`. `verify-test.sh` was run **only after every
sweep had finished**, and two documents re-rendered afterwards are byte-identical to the final
sweep's copies once `/CreationDate` is masked — which `batch-check.sh` does not pin, and which is
the only field that differs between a sweep copy and a hand render.

## 1. Baseline: **280 of 307**, reproduced exactly

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 363 MATCH 306 MISMATCH 57
REF-CANNOT-RENDER 0`; scored by `score.py` over `MANIFEST.tsv`'s 307 sheets paths, which refuses
to print unless every one found a row: **280 match, 22 `words`, 5 `pages,words`**. The brief's
figure exactly.

## 2. Result: **803 of 946 unchanged — sheets 280, slides 200, words 323, zero verdict movement
in either direction on any track**

All three tracks were swept in this worktree because the diff touches `XRay.Office.Core` and
`XRay.Office.Ooxml`. **10 sheets renderings changed and 0 slides and 0 words renderings changed**,
and the 10 are the chart-area border, not the wrap limit.

| | predicted | measured |
|---|---|---|
| sheets verdicts | 278 to 281, ≤2 losses, ≤1 gain | **280 — no movement, 0 losses, 0 gains** |
| sheets renderings changed by the wrap rule | 8 to 25 | **0 of 307, byte for byte** — wrong, and wrong in the direction that costs nothing |
| `023_Waterfall` | unchanged at 872/868 | **unchanged — right** |
| `058_Social_media_engagement_data` | unchanged at 200/194 | **unchanged — right, and § 6 says why the brief expected otherwise** |
| the four pies | unchanged, word-exact | **unchanged — right** |
| page counts on our side | 0 change | **0 — right** |
| slides verdicts | 0 to 3 move | **0 — right** |
| words verdicts | 0 to 2 move | **0 — right** |
| "more axes turn 45°" | strictly more wrapping | **true of the rule and true of the control decks; 0 corpus documents are in the band it moved** |

**The prediction's own largest blind spot did not fire, because nothing fired.** Stated per
direction as `COMMON.md` now requires: **0 improved, 0 worsened** on the wrap rule; **10 improved,
0 worsened** on the border.

## 3. The wrap limit is **0.95 of the tick spacing** — the C++'s own constant, and round 30's
fitted 1.000 is refuted

Round 30 bracketed the limit at [0.990, 1.056] of the tick spacing, shipped 1.000 as "the only
round number in the intersection", and explicitly **rejected** `createTextShapes`'s own
`0.95 × spacing` as "0.88 of it on those decks". Round 62 read the same bracket as a measurement
of `true ÷ 0.975` and shipped `AdvanceScale` as a stated stop-gap. **Both readings are wrong, and
the reason is one sentence: round 30's decks all carried a one-word label, and a one-word label
cannot see this limit at all.**

`lcl_hasWordBreak` does not turn the axis. It sets `m_bLineBreakAllowed = false` and restarts
(`VCartesianAxis.cxx`:888-903); the 45° follows only if the labels then *collide* as single lines.
So a one-word label wider than 0.95 of a tick but narrower than a whole one breaks, unbreaks, and
comes out **upright** — and every one-word deck therefore turns at the collision boundary, 1.000,
whatever the wrap limit is.

### The decks, 328 of them, and the arm that would have come out differently

Round 30's generator re-run at six sizes (`make-rot-probe.py`, `rot-sizes.txt`), then the tick
spacing swept **continuously by the chart frame's own width** rather than by an integer category
count (`make-fine-probe.py`, `rot-fine.txt`), then the same with a **two-word** label
(`make-cats-probe.py`, `cats.txt`). LibreOffice's decision is read two ways on every deck — the
depth of the bottom band in its own `chart:coordinate-region`, and whether the labels are in the
exported PDF's text layer at all, since 26.2.4.2 draws a 45° chart label as outlines. **The two
readings agree on all 328 decks.** Nothing of ours runs inside any of it.

| decks | what varies | boundary, quantised ruler | boundary, unquantised |
|---|---|---|---|
| one word, six sizes, category count | 7/8/10/11/13/14 pt | intersect at **[0.9805, 1.0102)** | **EMPTY** — [1.0168, 1.0036) |
| one word, frame width, 10 and 11 pt | tick spacing, continuously | intersect at **[0.9974, 0.9988)** | **EMPTY** — [1.0230, 0.9766) |
| one word, 12 categories instead of 13 | the count | [0.9973, 0.9988) — the same | |
| **two words**, 10 pt (`A`, `D`) | frame width | **[0.9470, 0.9505)** | [0.9713, 0.9748) |
| **two words**, 11 pt (`H`) | frame width | **[0.9486, 0.9524)** | [0.9276, 0.9312) |

- **0.95 is in both two-word brackets and 1.000 is in neither.** Nothing is fitted: 0.95 is
  `nReduce = (nLimitedSpaceForText*5)/100`, taken in integer 1/100 mm, which at these spacings is
  at most 0.08% above a flat 0.95 — a difference the decks cannot see and the shipped code does
  not model.
- **The unquantised readings are disjoint at every arm.** That is round 62's pixel-em law
  confirmed from a completely different observable — LibreOffice's rotation decision rather than
  its `TJ` adjustments — and it is the arm that would have come out differently under the rival
  hypothesis. Six sizes of one-word decks alone already refute the unquantised ruler.
- **A four-arm control separates which word decides.** `Mi Column` (short first word) turns where
  `Middle Column` does; `MiddleMiddleMi Column` turns everywhere, as a 67.73 pt first word must;
  `Middle Colum` and `Middle Columnn` move the boundary to exactly where `0.95 × spacing` puts
  their second word. So the trigger is *any* word over the limit, and the limit is 0.95.
- **The first label is not tested.** Deck `C`'s widest word is `START` at 31.96 in label zero,
  against a limit of 30.95 at the narrowest spacing swept, and LibreOffice leaves that axis
  upright. That is `nTick > 0` (`VCartesianAxis.cxx`:892), measured rather than cited.
- **A trailing blank hangs.** The same deck requires it: `Middle ` at 31.43 would break where
  `Middle` at 28.72 does not. A hyphen is the other way and is kept.

### `AdvanceScale` is deleted

`IChartTextMeasurer.AdvanceScale`, `ChartText.AdvanceScale` and the `SheetChart` override are
gone. The limit is now stated in one ruler's units and needs no per-consumer correction. **Round
62's account of why the method existed is superseded**: it was not a units patch on a fitted
constant, it was a fitted constant that had located the wrong boundary.

### And the change moves nothing

**0 of 307 sheets renderings differ byte for byte**, and 0 of the **34** slides and words documents
that hold a labelled category axis — rendered at the reverted rule and at the shipped one, in this
tree, with the file restored by `cp` + `touch` and the tree confirmed clean afterwards. So the
rule is now right and no corpus document sits in the 2.5–7% band it moved.

**The live control that the constant is in the rendering path and not only in the tests**: our own
CLI over deck series `A` puts our rotation boundary at a tick spacing of **36.30**, which is
`33.597 / 0.95` for the *slides* measurer's unquantised width — where the old rule put it at 34.46
and the reference is at 35.41. Our error on that deck goes from −0.95 pt to +0.89 pt, and the
residue is exactly the pixel em that `SlideChart` still does not apply. **So the prediction's claim
that 0.95 is "closer to the truth than 1.000 at every size" is wrong at 10 pt** — there it is the
same distance on the other side — and right at 8, 9, 11, 13 and 14.

## 4. The automatic `#D9D9D9` chart-area border, implemented — and the reading was right for two
rounds while the code's own remarks said why it could not be

`ChartPlot.Border`'s remarks cited `objectformatter.cxx:837-847` and tdf#150176 for "a chart with
no `a:ln` has no frame". **That passage is an exception and the remarks read it as the rule**:

```cpp
if ( eObjType == OBJECTTYPE_CHARTSPACE ) {
    rData.mrFilter.getMediaDescriptor()[PROP_FILTERNAME] >>= aFilterName;
    if (!aFilterName.startsWithIgnoreAsciiCase("Impress")) {
        mxAutoLine->maLineFill.moFillType = getDefaultChartAreaLineStyle();
        mxAutoLine->moLineWidth = getDefaultChartAreaLineWidth();      // 9525 EMU = 0.75 pt
        mxAutoLine->maLineFill.maFillColor.setSrgbClr(0xD9D9D9);
```

Every host **but** Impress gives a chart space with no line of its own a solid `D9D9D9` line
0.75 pt wide. Four blind readers across rounds 61 and 62 reported the missing border on three
unrelated **spreadsheets**, which is exactly the family the exception does not cover.

| | before | after | reference |
|---|---:|---:|---:|
| `microsoft_learn_multi_chart_examples` | 0 | **12** | 12 |
| `005_Contextures_chart_sample_6e279b08` | 0 | **8** | 8 |
| `002_Contextures` / `013_Contextures` | 0 | **3** | 3 |
| `008_/010_/012_Contextures`, `023_Waterfall`, `dynamicbubblechart` | 0 | **2** | 2 |
| `001_Contextures_chart_sample_b089bc34` | 0 | **1** | 1 |

**Exact on all ten, and the rectangle is right too**: `023_Waterfall`'s page-1 frame comes out at
(67.78, 425.91)-(530.84, 756.17) against the reference's (68.17, 425.79)-(530.67, 755.77) — within
0.4 pt on every edge. **10 renderings changed, 0 verdicts moved, and no word count moved anywhere**,
which is what a stroke must do.

### The census over-reaches and the ink says by how much

`census-chartborder.py` over all 946 manifest paths: **90 sheets documents / 138 parts and 10
words documents / 10 parts** have a non-Impress chart space that states no line; **0 slides**,
because the exception is theirs. The ink control that census cannot be — `count-greystrokes.py`
over the whole baseline sweep — says the reference draws more `D9D9D9` strokes than we did on
**21** documents and 378 strokes, of which **12** were a clean 0-against-N. **Ten of those twelve
changed.** So the shape census over-reaches by 9× and the reason is exactly `COMMON.md` § 6: a
chart part on a sheet that is never printed states everything and draws nothing.

### The differing-pixel count is the wrong instrument for a hairline, and says so

`diffpixels.py` at 100 dpi over the ten: 988 146 → 990 402 differing pixels, **improving on four
documents and worsening on three**. A 0.75 pt stroke placed 0.4 pt from the reference's misses its
pixel row about half the time and then counts *twice*. The stroke count and the rectangle's
coordinates are the measurement; the raster is not. Recorded because a round that only reported
the favourable instrument would have looked better and said less.

## 5. The vision round — three blind readers, one confirmation of this round's own change, one
lead no metric produced, and one refutation of the brief

Three subagents, one composed pair each, `Read` on one image path only, no project documents, no
source, no shell, each asked to describe the halves separately, give a direction and a confidence,
and say what looked identical. **No page was chosen by `--worst`.** All three pairs are ours at
this round's tree against the reference.

### `058_Social_media_engagement_data` p1 — chosen because it is the document the brief says decides the round

The reader ranked first, at high confidence:

> *"the upper half draws 2/20/2023–3/15/2023 where the lower half draws 8/21/2026–9/13/2026 …
> twenty-four rows deep, while the adjacent impressions column is unchanged"* — and, separately,
> *"the lower half is internally inconsistent: its table dates (2026) disagree with its own chart
> category axis, which still reads 2/20/2023 … 3/15/2023"*.

**Confirmed by a second instrument immediately.** `grep` over the workbook: **24 `TODAY` formulas
in `sheet11.xml` and one in `table11.xml`**. The reference recomputes them at render time — 21
August 2026 is the day this round ran — and we draw the cached 2023 values. A token diff confirms
it: the reference's `8/21/2026 … 9/13/2026` against our `2/20/2023 … 3/15/2023`.

**So `058` is a volatile-date document and belongs with `fse_identification_form.xlsx` under the
`unstable` treatment, not with the axis-fitting cluster.** § 6 proposes it.

And the reader **refutes the brief's other claim about it** at high confidence: *"both halves draw
24 category labels, one per bar, with no thinning or skipping; both are rotated at approximately
the same 45-degree slant; in neither half is any label truncated"*. The brief has *"the reference
draws about 24 consecutive date labels where we draw ten"*. At this tree we draw 24, as it does.

Her third finding is a **new lead no metric on this project has produced**: *"the upper half draws
the headline in a dark gold/olive where the lower half draws it in dark teal"*, high confidence,
*"the hues are far apart and saturated enough that neither compression nor rescaling could account
for it"*. Not measured this round.

### `046_Cost_analysis_with_Pareto_chart` p1 — chosen because the parent corrected this round's brief about it

The parent's correction: `046` is `text`, not `ceiling`, because against the **reference** it is
754 characters to 753 and the difference is `ment` on our side against three full stops on theirs.

**Confirmed, exactly.** The charstream test at this tree: ours 754, reference 753, `only ours:
{'t':1,'n':1,'e':1,'m':1}`, `only ref: {'.':3}`. The reference truncates a rotated category label
with an ellipsis where we draw the word in full.

The reader, who was asked about shortened text and did not know the claim, ranked second at
medium-high confidence: *"the left half draws them overlapping and colliding, with the first few
names running together into an unreadable clot around 'Manufacturing equipment', where the right
half draws them cleanly separated"* — and then, at **low** confidence and unprompted, *"I could not
resolve whether either half is actually abbreviating a label with a trailing ellipsis … the left
half's smudge could be hiding one"*. **She named the limit of her own instrument and was right
about it**; the character stream is what settles it. That is the calibration `COMMON.md` asks for:
vision supplied the object and the direction, an instrument supplied the confirmation.

She also ranked first, at high confidence, *"the left half draws 'Cost Analysis' in near-black
where the right half draws the same words in a medium steel blue"* — the same **title colour**
defect the `058` reader found independently on a different document. Two readers, two documents,
same object, same shape of difference, no instrument yet.

### `023_Waterfall_Chart_Template_for_Excel` **page 2** — chosen because it is the largest unworked chart defect and page 2 has never been read

- **This round's own change, confirmed blind.** Among what looks identical: *"the chart's outer
  frame: both halves draw a thin light rectangular outline around the whole chart at what appears
  to be the same position, size and line weight. Neither omits it."* Round 62's reader on page 1
  reported that outline as present on the reference and absent on ours. It is now present on both,
  and a reader who was not told anything says so unprompted.
- **The chart itself is worse than page 1's.** *"The left half draws the series as horizontal bars
  extending rightwards from x = 0, where the right half draws it as one narrow vertical stack
  pinned at x = 0 with essentially no horizontal extent"* (high confidence); *"the left half draws
  a numeric value axis covering 7000 down to −2000 … the right half draws no numeric vertical axis
  at all — in its place is a single text label, 'Delta 5'"* (high); *"the right half draws a thin
  blue horizontal line running across the plot … the left half draws no connecting line of any
  kind"* (high); *"the left half draws every bar in a single medium blue … the right half draws its
  segments in alternating green and red"* (high). **Ours draws three bars where the reference draws
  a stacked column, and the two are not the same chart.**
- And, corroborating round 62's reader on page 1 across a different page: *"the left half draws the
  `#N/A` entries in solid black; the right half draws them in pale grey"* (high) and *"the left
  half draws the footer … in black; the right half draws both in grey"* (medium-high).

## 6. `MANIFEST.tsv`

Lives in the corpus repository and was **not touched**. One row is proposed, and it is a
reclassification rather than a verdict change:

| path | proposed | why |
|---|---|---|
| `sheets/chartset-007/xlsx/058_Social_media_engagement_data_cfa1cb18.xlsx` | `kind` `text` → **`unstable`** | 24 `TODAY()` formulas; the reference draws the day it is run and we draw the cached value, so the two sides can never agree on that column and the document's content is a function of the date |

`046_Cost_analysis_with_Pareto_chart` stays `open`/`text` and the parent's correction is confirmed
by the character stream (§ 5). `023_Waterfall` stays `done` at 872/868 and should still be read as
a `done` document whose page-2 chart draws the wrong chart.

## 7. Tests: **5133 passed, 0 failed, 1 skipped**, and six mutations all detected

Re-derived by running each project rather than quoted: Containers 109, Core **412**, Markup 259,
OpenDocument 125, Presentations **882**, Rendering 153 (+1 skipped), Spreadsheets 1035, Text 625,
Vector 302, WordProcessing 1231. `dotnet build -v q -nologo` → **0 warnings, 0 errors**.

Core 407 → 412 and Presentations 878 → 882. **Nothing else moved, and that is itself a finding: no
existing assertion anywhere was pinning the wrap constant**, which is how a fitted 1.000 survived
thirty-three rounds.

| mutation | outcome |
|---|---|
| `WrapFraction` 0.95 → 1.00 | **detected** by 3 of the 5 new tests |
| the first label is tested again (`at = 1` → `at = 0`) | **detected** by `TheFirstLabelIsNotTestedForABreak` |
| a trailing blank is counted again | **detected** by `ATrailingBlankIsNotCountedInAWordsWidthButAHyphenIs` |
| the automatic chart-area line is not applied by Calc | **detected** by `AChartSpaceStatingNoLineGetsTheGreyDefaultOutsideImpress` |
| the automatic line ignores `a:ln/a:noFill` | **detected** by `AnExplicitNoFillLeavesTheChartAreaWithNoBorder` |
| the Impress exception is dropped (`automaticChartAreaLine` → `true`) | **detected** by `TheSameChartSpaceGetsNoBorderUnderImpress` |

All six are detectors by reintroduction; **none of the eight new tests is only a drift guard**.
`AWordBreaksAtNineteenTwentiethsOfTheTickSpacing`'s two cases fail under the two rival readings in
*opposite* directions — a limit of the whole spacing leaves both thinned, a limit of 0.90 turns
both — and `AOneWordLabelThatBreaksButDoesNotCollideStaysUpright` asserts the case every deck
before this round was blind to.

## 8. Shared layer

`XRay.Office.Core/Charts/ChartAxisLabels.cs` and `ChartLayout.cs` (the interface),
`XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs`, plus one call-site line each in
`XRay.Office.Spreadsheets/Ooxml/XlsxDrawings.cs` and `XRay.Office.WordProcessing/Ooxml/DocxPictures.cs`.

Census, all 946 manifest paths:

- **wrap limit** (`census-catlabels.py`): a labelled category axis in **62 sheets documents / 94
  parts, 32 slides / 68, 2 words / 2**, plus BIFF chart substreams in 7 sheets, 2 slides, 0 words.
  Narrower than round 62's text-bearing-chart reach because `Wraps` is only ever asked about a
  category axis. **Measured: 0 of those 34 slides and words documents change byte for byte, and
  both full sweeps confirm 0 verdicts move.**
- **chart-area border** (`census-chartborder.py`): **90 sheets / 138 parts, 10 words / 10 parts,
  0 slides**. Measured on the sweep: **10 sheets renderings change**. The 10 words documents are
  named in `census-chartborder.py`'s output and **did not change any words verdict** (323 before
  and after); their renderings were not byte-diffed for want of a same-tree baseline on that
  track, and that is the one gap in this round's cross-track evidence.

Falsifiable statement for the parent: **0 verdicts move on words and slides at HEAD**, and the
only words renderings that can differ are the 10 with a border-less OOXML chart part.

## 9. The 24.2.7.2 audit — counters re-derived, **no re-check run**, and the site round 62 named
had already been re-checked by the round that named it

Re-derived at this base with the file's own commands: **37 open hits in 26 files** — unchanged
from round 62 — and **34 marker lines, 29 `VERIFIED`, 4 `FIXED`, 1 `WRONG`, 0 `UNDECIDED`**, where
round 62 recorded 31/26/4/1. The three extra `VERIFIED` came in with the slides and words branches
merged beside it. **Sixth time a quoted number in that file has failed to reproduce**;
`TODO.24-2-7-audit.md` is corrected again.

Round 62 (sheets) named `XRay.Office.Core/Graphics/GlyphRun.cs` :347/:369 as the next site. **It
already carries a `VERIFIED` marker dated the same day, from slides-r62** — the round that named
it and the round that did it are the same round, and the sheets write-up did not know because the
merge had not happened yet. Its claim one is verified; **claims two and three are not, and the
marker says why: no corpus document states a lone `a:lum` brightness or a non-washout pair.** They
can only be settled by authored fixtures, and `probes/sheets-r61/audit_lumpercent.py` is the
harness — three arms (brightness alone, contrast alone, both) separate
`BColorModifier_RGBLuminanceContrast`'s "whole brightness after the contrast"
(`basegfx/source/color/bcolormodifier.cxx`:387-405) from `Bitmap::Adjust`'s msoBrightness branch's
"half before and half after" (`vcl/source/bitmap/bitmap.cxx`:1694-1698) outright, since the two
formulas differ on any non-zero pair.

**This round pointed no probe at the list**, and says so rather than filing a hurried one: its
budget went to 328 decks, four whole-track sweeps and a cross-track control. `RasterImageDecoder.cs`
:239 is the same branch from the decoder's side and belongs in the same run.

## 10. What the next round should do first

1. **`023_Waterfall`'s page-2 chart, which is not the same chart as the reference's.** A blind
   reader at high confidence: we draw three horizontal blue bars against a value axis running
   7000 to −2000; the reference draws one narrow **stacked column** in green and red, with a
   connector line and no numeric value axis at all. Page 1's nine-of-twelve-bars defect is the
   same document and probably the same cause. It passes the gate at 872/868 and no column can see
   any of it.
2. **A rotated category label's ellipsis.** `046` is settled to the character: the reference draws
   `…` where we draw `ment`, and a blind reader independently reports our labels colliding into an
   unreadable clot where the reference's are separated. That is a *clip and ellipsise to the room
   available* rule we do not implement at all, and it is the axis-label machinery this round has
   just put on a measured footing.
3. **A chart title's stated colour.** Two blind readers, two unrelated documents, same direction:
   `058`'s headline is olive on our side and teal on the reference's; `046`'s is near-black on ours
   and steel blue on the reference's. No instrument has been pointed at it. It is the same shape
   the `D9D9D9` border had after two readings, and that one turned out to be real and exact.
4. **A data label's stated text colour** — `ChartDataLabel` still carries no colour field. 22
   sheets documents / 40 parts, 49 slides, 7 words. Untouched by this round and it owes a corpus
   gate.
5. **The 96 dpi width law for `SlideChart` and `FrameChart`.** This round's control measures what
   is left: on deck `A` our slides boundary is 36.30 where the reference's is 35.41, and
   36.30 × 0.975 = 35.39. It is one line in `ChartFace.Shape` for words and a rescale of the placed
   runs for slides, and it is the last reason the three consumers' chart rulers differ.
6. **`cellIs` conditional formatting** — 123 rules in 18 documents, two arms, unchanged since round
   59. Still the only item on this list that is not a chart.
7. **The page's own right-hand clip** — the reference stops page-1 drawing content at 516.24 pt on
   `003` and we run to 550.80. Unmoved since round 62.

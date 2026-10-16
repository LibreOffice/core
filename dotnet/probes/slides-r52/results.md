# slides-r52 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`fc-match Calibri` → `Carlito-Regular.ttf`, base `a21e64f6d7e`, `SOURCE_DATE_EPOCH=1700000000`,
`TZ=UTC`. Read `prediction.md` beside this file first; it was committed before anything was
rendered post-change.

## Baseline, and it reproduced exactly

Whole-track sweep, all 35 slides batches, reconciled document by document against `MANIFEST.tsv`
on a **case-folded** identity: **302 of 302 agree, 0 disagreements — 199 of 302 passing.**
The sweep's own `TOTAL` reads 311; that is the case-insensitive mount's 9 alias spellings.

A second, independent confirmation of the baseline arrived late and is worth recording, because it
is the alias trap firing exactly as the brief said it would. The **before** leg of this round's
ink A/B — a rendering of the whole track from a binary rebuilt at the base commit — totals
`ABS-INK 1413.17 / SIGNED 1038.16 / MAJOR 507` **over the script's own 311 rows**, and
**1394.04 / 1026.62 / 494 over `MANIFEST.tsv`'s 302 paths.** Round 51 reported
**1394.03 / 1026.61 / 494**. The manifest-scored figure reproduces the predecessor to the digit;
the sweep's own `TOTAL` does not, and the whole 19-unit discrepancy is nine documents counted
twice. *Score against the manifest, never a sweep's `TOTAL`.*

## Measured against the prediction

| | predicted | measured |
|---|---|---|
| **verdict movement** | −2 to +4, **most likely 0** | **0** — 199 → 199, and `MANIFEST` agrees on all 302 |
| page counts | 0 of 302 | **0 of 302** |
| word counts | "both directions, net gain" | **8 documents moved, 3 nearer the reference and 5 further** |
| renderings moved | "well under 241 candidates" | **101 of 302** — 73 improved, 28 worsened |
| `abs_ink` | not predicted | **1394.04 → 1238.64**, −155.40 (−11.1%) |
| signed ink | not predicted | 1026.62 → **913.80** |
| major pages | not predicted | 494 → **434** |
| tests | re-measure four theories | four theories re-measured, **14 expectations moved** |

For scale: round 50 moved `abs_ink` by −10.34 and round 51 by −15.33. This one moves it by
**−155.40**, and it does it with zero verdict movement — which is the gate's blind spot stated as
a number rather than as a complaint.

## Target 1 is empty, and the instrument that says so passes a known-answer check

Round 51's first item was the wrap approximation in `PptSlideLayout.Autofits`, on a census
finding **36 of 51 `.ppt` documents** with a `wrapNone`-and-not-`fFitShapeToText` OPT table. It
said 36 was an upper bound until each OPT table was tied to its `TextHeaderAtom` kind.

Tied (`ppt-autofit-census.py`):

| | |
|---|---:|
| `.ppt` documents | 51 |
| text-bearing shapes | 6989 |
| of Body/HalfBody/QuarterBody kind | 1402 |
| … `fFitShapeToText` (both suppress autofit) | 143 |
| … wrapping (both autofit) | 1234 |
| … **`wrapNone`, so we suppress autofit** | **25, all in `Fundamentals_Module_1_basics.ppt`** |
| … of those, carrying an `OEPlaceholderAtom` | **0** |

**Known-answer control**: the same walker restricted to r51's measurement — OPT tables alone,
whole stream — returns **36 documents**, r51's figure exactly. The instrument agrees with the
independent one on the common quantity, so the collapse is the Body-kind restriction and not a
different parser.

The placeholder column is the other half, and it is what makes the answer zero rather than one.
`svdfppt.cxx` reaches `bAutoGrowWidth = !bWordWrap` only through
`dynamic_cast<SdrObjCustomShape*>(pRet) && eTextKind == Rectangle`, and **line 846 sets
`pRet = nullptr` for any shape carrying an `OEPlaceholderAtom`**. So the wrap term can fire only
on a shape with no placeholder atom — which all 25 are. On every one of them LibreOffice
suppresses autofit exactly as we do.

**The comment standing beside `Autofits` was right and round 51's refutation of it was measuring
the wrong population.** It reaches zero documents and no code changed there.

## What the round shipped instead, and it is target 2 arriving underneath target 1

`SlideAutofit` was an explicit port of **24.2.7.2**'s bisection. Its own remarks said so, said that
25.2 had replaced the search with a walk down `constScaleLevels`, and ended
*"check which version wrote the reference before porting anything out of this tree."* The
installed reference has been **26.2.4.2** for many rounds. Nobody checked.

### The measurement: 36 one-slide decks, one variable

A single 40 pt paragraph in a 360 pt-wide `a:normAutofit` box, box height 60…480 pt, **each height
in its own file**, `/Tf` and baseline pitch read out of both content streams
(`make-fit-probe.py`, `research/probes/slides-r15/read-autofit.py`).

The reference's answers, as the box shrinks:

| box pt | reference | `constScaleLevels` row |
|---|---|---|
| 480…336 | 40 pt / spacing 1.00 | *unscaled — no overflow* |
| 324…312 | 40 / **0.90** | `{1.000, 0.900}` |
| 300…288 | 37 / 0.90 | `{0.925, 0.900}` |
| 276…228 | 34 / **0.90** | `{0.850, 0.900}` |
| 216…204 | 34 / **0.80** | `{0.850, 0.800}` |
| 192…180 | 31 / 0.80 | `{0.775, 0.800}` |
| 168…144 | 28 / 0.80 | `{0.700, 0.800}` |
| 132…120 | 25 / 0.80 | `{0.625, 0.800}` |
| 108…96 | 22 / 0.80 | `{0.550, 0.800}` |
| 84 | 19 / 0.80 | `{0.475, 0.800}` |
| 72…60 | 16 / 0.80 | `{0.400, 0.800}` |

**Every row of the table, in order, with both 0.850 rows present and nothing off the table.** Our
bisection answered 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37,
38, 39, 40 — essentially every whole point — and agreed with the reference on **13 of 36** boxes.

**The two documents this round was briefed to chase are the two ends of that one table.** Height
168 is the case a blind reviewer named unprompted as *"font scale applied without the matching
spacing reduction"*: both sides draw **28 pt** and the reference's baseline pitch is **26.90**
against our **33.62**. It is not a missing multiplier — `ScaledSpace` has scaled paragraph space
since round 20 — it is that our search chose the pair `(0.700, 1.000)` where the table has no such
row.

Three further conditions, each measured on authored decks rather than read out of the tree:

1. **No slack.** 24.2's `aCurrentTextBoxSize.extendBy(0, -50)` is gone. Stepping 330…340 pt at
   1 pt, the reference stops scaling at **exactly 336** and scales at 335; 1.417 pt of slack would
   have put the boundary at 334.
2. **The stated scale is still thrown away.** `fontScale="62500"`, `lnSpcReduction="20000"`, and
   both together, in a box far too tall to shrink: **40 pt at full spacing in all three.**
   `SlideAutofit`'s existing note survives the version move. (The mechanism is visible in
   `textbodypropertiescontext.cxx`:241-243 — `lnSpcReduction` defaults to `100000` and the
   property is set to `1.0 - 1.0 = 0.0`, which fails `setupAutoFitText`'s `fSpacingScale > 0.0`
   guard — but the `fontScale`+`lnSpcReduction` case should have passed that guard and did not, so
   the measurement is what is being relied on here, not the reading.)
3. **The rounding is `std::round` in the hundredth-of-a-millimetre domain and the order of the
   arithmetic decides two cases.** At a stated 30 pt the reference draws **25** at level 0.850
   (25.5 rounding *down*) and **17** at level 0.550 (16.5 rounding *up*). Both fall out of
   `roundToNearestPt(roundToNearestPt(deviceRealised) × level)` in doubles — 899.5833…→25.49999996
   and 582.0833…→16.50000000 — and neither falls out of multiplying in points, which gives 26
   and 17.

### Known-answer checks after the change

| probe | before | after |
|---|---:|---:|
| 36 solo decks at a stated 40 pt, size **and** pitch | 13 of 36 | **36 of 36** |
| 21 solo decks at a stated 30 pt (the half-point levels) | not run | **21 of 21** |
| 9 + 3 decks reproducing the unit tests' fixture | — | **12 of 12** |
| `tests/corpus/features/slide-autofit-grid.pptx`, all 23 slides | — | **23 of 23**, ≤0.0007 pt |

## Refutations, including two of my own claims

### 1. Round 51's target 1 — above. Reaches zero documents.

### 2. My own prediction item 6 was wrong, and my own measurement is what killed it

I wrote that *"the reference's shared-outliner state leak is real and I am not reproducing it"*,
on the evidence that the same 480 pt box answered spacing 1.00 as a solo slide and 0.90 as page 71
of a 71-slide deck.

**There is no leak.** The 0.90 reading was an `awk` filter of mine that collapsed rows on the
`/Tf` column alone, so the transition from `{1.000, 0.900}` to unscaled at page 47 was invisible
and page 71 inherited page 42's line in the output. Re-read collapsing on *both* columns, the
71-slide deck's transition sits at height **336** — the same boundary the 36 solo decks give, to
the point. A two-slide deck rendered in both orders gives the tall box 40/1.00 either way.

So the multi-slide fixture is sound, the solo decks are sound, and they agree. **An instrument
that aggregates on one column cannot see a change in another, and I built the claim on it.**

### 3. "The reference draws fractional em sizes there" does not explain the regressions

Eight documents lost `/Tf` agreement (§ below). All eight are `.ppt`, and on several the reference
draws sizes that are *not* whole points — `Lepore.ppt` p2 draws **20.0 ×11 and 20.4 ×6 on one
page**, and 20.4 is 24 × 0.850 exactly, unrounded. That is a fit path with
`setRoundFontSizeToPt` off, so not `setupAutoFitText`. A tempting explanation.

**Run over the documents that improved, it dies** (§ 7 of HANDOVER, and this is its fifth
instrument): 45 of the 77 improved documents also carry >5% fractional reference sizes, and
`2014BSA_Sunday_Killion.pptx` is 81% fractional and **improved**. Mean share 0.2736 among the
worsened against 0.1015 among the improved — a tendency, not a classifier. The regressions stay
unexplained, and § "left open" says what to measure instead.

## Verdict movement: none, in either direction

199 → 199. `MANIFEST.tsv` agrees on all 302 documents in both sweeps, so no document changed
verdict and none needs re-filing. Page counts moved on **0 of 302**.

Eight word counts moved, all inside the band, and they are **not** all improvements:

| Δ | document | ours/ref before → after | |
|---:|---|---|---|
| +8 | `Framing Europe.ppt` | 2237/2237 → 2245/2237 | **was exact, now +8** |
| +4 | `Structural Testing.pptx` | 4362/4340 → 4366/4340 | further |
| +2 | `RRM-training-syllabus-Chapter-3…ppt` | 2475/2576 → 2477/2576 | nearer |
| −2 | `pods05.ppt` | 1882/1884 → 1880/1884 | further |
| −2 | `1200-Assigning-Club-officers.pptx` | 165/165 → 163/165 | **was exact, now −2** |
| +1 | `8.16_AOD_FINAL_Provider_Training…ppt` | 4310/4311 → 4311/4311 | **now exact** |
| +1 | `Intersil_Italy_CAN_Bus…pptx` | 3454/3435 → 3455/3435 | further |
| −1 | `Architecture.ppt` | 846/845 → 845/845 | **now exact** |

Three nearer, five further, none near its band edge. Our own extracted character multiset moved on
**19 of 302** renderings, 15 of them by a single character — re-wrap moving a hyphen or a space.
The change is geometric and the text layer barely notices, which is why the gate cannot see it.

## The previously-passing batches were re-swept, and none moved

The sweep glob was `slides/*`, so `slides/done-001` … `done-015` are inside both legs rather than
checked afterwards: **144 rows, 144 `match`, identical before and after.** The whole-track
reconciliation against `MANIFEST.tsv` covers the same ground document by document — 302 of 302
agree in both sweeps — which is the statement that no batch was traded for another.

## Ink, per document, before → after

Scored over `MANIFEST.tsv`'s 302 paths: `abs_ink` **1394.04 → 1238.64**, signed
**1026.62 → 913.80**, major pages **494 → 434**. `|signed| ≤ unsigned` holds on both sides.
**101 documents moved, 73 improved and 28 worsened.**

| Δ abs | document | abs before → after |
|---:|---|---|
| **−14.58** | `dhs-293364.pptx` | 17.10 → 2.52 |
| **−14.09** | `ev122_jj-liew-tort-of-bribery-webinar.pptx` | 15.23 → 1.14 |
| **−13.47** | `71393_pp7.ppt` | 17.37 → 3.90 |
| **−11.95** | `chapter_4_0.pptx` | 25.43 → 13.48 |
| **−9.43** | `berlin.ppt` | 16.67 → 7.24 |
| **−9.41** | `ITE106-Chapter 4.ppt` | 28.63 → 19.22 |
| **−9.21** | `Employment-Based_I-485.ppt` | 18.99 → 9.78 |
| **−8.32** | `2015-Civil-Rights-Website-training.ppt` | 44.42 → 36.10 |
| **−5.97** | `Copy of Full deck with references Promotion of Outside Interests…pptx` | 8.54 → 2.57 |
| **−5.18** | `16 - UTM - (NASA).pptx` | 20.28 → 15.10 |
| **−4.82** | `BHCA Part II webinar #2 - 10.2020.pptx` | 6.25 → 1.43 |
| **−4.25** | `BUS-Chapter 05.ppt` | 14.26 → 10.01 |

**The regressions, stated rather than netted:**

| Δ abs | document | abs before → after |
|---:|---|---|
| **+4.69** | `Lepore.ppt` | 4.90 → 9.59 |
| **+3.35** | `gfopportunitiesforlinkagespres_2010_en.ppt` | 11.84 → 15.19 |
| **+3.16** | `FAA_Form_337.ppt` | 18.68 → 21.84 |
| **+2.46** | `ws_prod-g-doc-Events-r-6.-ESM.ppt` | 8.43 → 10.89 |
| **+1.26** | `010605Vul.ppt` | 4.33 → 5.59 |
| **+1.21** | `joint_user_outcomes_michael_fullerton_29.06.12.ppt` | 8.12 → 9.33 |
| **+1.20** | `hofman.ppt` | 1.33 → 2.53 |
| +0.65 | `066_Free_PowerPoint_Layered_Funnel_Process_c0b380ec.pptx` | 3.30 → 3.95 |
| +0.48 | `067_Free_PowerPoint_Layered_Funnel_Process_4_Stages_4deb9647.pptx` | 2.68 → 3.16 |
| +0.44 | `009_3-Circle_Venn_PowerPoint_Diagram_2e59baa9.pptx` | 0.10 → 0.54 |

The remaining 18 are ≤ +0.40.

## The instrument that measures what the change actually controls

Round 51 recorded that a small ink rise can be the *opposite* of a regression, so ink alone cannot
adjudicate the twenty-eight above. `tf-agreement.py` scores, per page, the multiset of `/Tf` sizes
we draw against the reference's, weighted by how many show operators carry each:

| | before | after |
|---|---:|---:|
| mean per-document agreement | 0.72571 | **0.75160** |
| pages whose size multiset is **exact** | 1388 of 4515 | **1552 of 4515** |
| documents whose agreement moved | — | 85: **77 improved, 8 worsened** |

The eight that worsened are `Lepore`, `WC_Update-Aug03`, `gfopportunitiesforlinkagespres_2010_en`,
`FAA_Form_337`, `ws_prod-g-doc-Events-r-6.-ESM`, `joint_user_outcomes…`, `EG1_dsrc tech` and
`010605Vul` — **all `.ppt`, and the same documents as the ink regressions.** So on those the ink
rise agrees with the sizes and they are real regressions, not the round-51 unmasking effect. They
are named here for that reason.

## The blind readings, before and after, four fresh reviewers

Four reviewers, none of which had read this brief, could read source, documentation or `results.md`,
or run a command. Each was given one composed page and nothing else, and asked to describe each
half separately, give **direction**, say what looked identical, and name the causes the image
cannot decide between. The before/after split was not disclosed to any of them.

### `2015-Civil-Rights-Website-training__ppt` p42 — closed

- **Before**: *"the top breaks lines earlier … the top's body runs two lines longer overall — 14 vs
  12 … the top's last line crowding the page number."*
- **After**: *"all 12 lines break at exactly the same words in both halves … no line wraps earlier
  or later in either"*, and it listed left margin, right wrap boundary, intra-paragraph pitch,
  title, rule, logo and page-number position as identical.

Measured on the page rather than read: the body em was **24.009 pt against the reference's 21.997**
and is now **21.997**; baselines went **21 → 19** against the reference's 19, and all nineteen sit
within **0.5 pt** of the reference's.

**And the after-reviewer's own top-ranked observation was an instrument defect, which it caught
itself.** It reported the body block ~15–20% taller and the page number drifting with it, and
named *"anisotropic canvas scale — the two halves rasterized at the same width but different
heights"* as its first candidate, with rule-to-page-number span as the discriminator. `pdfinfo`
says both pages are 720 × 540 pt and the baselines agree within 0.5 pt, so the candidate the
reviewer put first is the correct one and the difference is in the composed image. `page-vision`
already says this — *check that "it is absent" is not a fact about the pipeline that built the
image* — and a reviewer applying it unprompted is the method working.

### `ITE106-Chapter 4__ppt` p7 — better, not closed, and it names the next target

- **Before**: ours 9 body lines to the reference's 10, ours 10–13% smaller, inter-bullet gaps
  **3.0 line-heights against 1.8**. Its proposed mechanism, unprompted: *"an autofit implementation
  that shrinks glyphs but not paragraph spacing."*
- **After**: **10 lines to 10, 2/2/3/3 both sides.** But the reference still sets the body larger,
  and the reviewer again reported our inter-bullet gaps as far wider — *"roughly 2.3 extra
  line-heights of white space, versus roughly 0.85"*.

Measured on the page:

| | ours before | ours after | reference |
|---|---:|---:|---:|
| body `/Tf` | 21.005 | **21.997** | **24.009** |
| baselines | 18 | **19** | 19 |
| intra-paragraph gap | 25.20 | 23.75 | **28.80** |
| inter-paragraph gap | 72.87 | 64.96 | **50.88** |
| paragraph space above the line | 47.67 | 41.21 | **22.08** |

Our paragraph space is **1.9 times** the reference's once each side's own spacing scale is taken
off (41.21/0.8 = 51.5 against 22.08/0.9 = 24.5). That extra height is why the walk stops one row
deeper than the reference's — 0.775 where the reference takes 0.850. **The residual on this page is
inter-paragraph space in the `.ppt` reader, and the autofit table is now carrying its error rather
than causing it.**

## The property of the fix that the next round has to know

A bisection over a fine grid **absorbs a small height-measurement error**; a twelve-row table
**quantises it into a whole row**. That is the mechanism behind all eight `/Tf` regressions, and it
is not an argument against the table — the table is what the reference does, measured 36 + 21 + 12
+ 23 ways. It means the value of every remaining `.ppt` height defect has gone up: an error that
used to cost a tenth of a point now costs 7.5% of the font size.

## Instruments, including one that returned zero and is reported as such

`offpage-charstream.py` was written to reproduce r50's "30 of our renderings drew text outside the
page against the reference's 9". It returns **0 for both legs and 0 for the reference**, which is a
failed known-answer check, so **nothing is claimed from it**. The reason is structural: `pdftotext
-bbox` reports the text poppler extracts, and text outside the media box is clipped before it gets
there. Off-page text has to be counted from the content stream's pen positions, which is what r50
must have done. The script's charstream half is sound and is quoted above.

## Tests

`SlideAutofitTests` is now **43 theories and facts, 773 tests in the project** against 772.
**Fourteen expectations moved**, and every one was re-measured against 26.2.4.2 rather than
adjusted to fit:

| theory | how re-measured | moved |
|---|---|---:|
| `OneLineShrinksToTheSizeTheReferenceDraws` | 9 one-slide decks, "A" at 40 pt in a 60 pt box | 6 of 9 |
| `TwoLinesShrinkToTheSizeTheReferenceDraws` | 3 of the same | 2 of 3 |
| `AWrappingBodyLandsOnTheReferencesSizeAndSpacing` | the existing `slide-autofit-grid.pptx`, all 23 slides re-read | 6 rows replaced by 7 |
| `TheFitsSpacingScaleReachesAParagraphsOwnSpace` | heights re-chosen, expectations unchanged | 2 of 3 heights |

The wrapping theory gained a case on purpose. **120 pt and 135 pt both draw 17 pt and differ only
in the spacing beside it, 0.80 and 0.90** — the pair that exists because `constScaleLevels` holds
0.850 twice. No search over a font scale can produce two spacings at one size, so that pair fails
against every reading of the bisection and against a font-scale table without the second column.
It is the discriminating case and it is the reason the heights moved as well as the values.

`AnOverflowingBodyStopsAtAQuarterAndOverflows` and `NoBodyIsEverScaledToNothing` **did not move**:
their expectations were already read off a banked 26.2.4.2 rendering of `NWD-GLA…`, and 0.250/0.800
is the table's last row, so the walk subsumes the old clamp rather than contradicting it. That is a
drift guard that was pointing at the right binary all along.

**Verification by reintroduction was not applicable to this change and is stated rather than
implied.** The mutation that matters — putting the bisection back — is the diff itself, and it was
measured directly on 36 + 21 solo decks and on the 23-slide fixture: 13 of 36 against 36 of 36.
The `TheFitsSpacingScaleReachesAParagraphsOwnSpace` heights are chosen so one lands on each of the
three spacing scales, which is what makes a spacing-scale regression detectable there; a box at
full spacing alone would pass under either reading.

Ten non-Fidelity projects: Core 337, Containers 109, Text 596, Vector 295,
Rendering 150 (+1 skipped, `PdfFontTests.ACffFlavouredFaceIsNotClaimedToBeTrueType`, skipped at
baseline too), Markup 259, OpenDocument 125, WordProcessing 1061, Spreadsheets 886,
**Presentations 773** — **4591 passed, 0 failed, 1 skipped**, against the briefed
4590/0/1: **+1**, which is the extra theory case.

`cd dotnet && dotnet build -v q -nologo` → **0 warnings, 0 errors**.

## Shared layers: none, and it is a two-file diff

`dotnet/src/XRay.Office.Presentations/Layout/SlideAutofit.cs` and its test file, plus four probe
scripts and these two documents. Nothing in `Core`, `Containers`, `Text`, `Vector`, `Rendering`,
`Markup` or `XRay.Office.Ooxml`; `DocxPictures.cs`, `XlsxDrawings.cs` and the DrawingML fill path are
untouched. `Scaling` is a private nested type of `SlideTextLayout` and `Solve` is called from
nowhere outside `XRay.Office.Presentations`. **No cross-track sweep is owed.** The WordProcessing and
Spreadsheets suites were run anyway and are green.

`MANIFEST.tsv` needs **no** change: zero verdicts moved, no document changed kind, and it lives in
the corpus repo, which was not touched.

## Left open, in the order the next round should take them

1. **Inter-paragraph space in the `.ppt` reader.** Measured on `ITE106-Chapter 4.ppt` p7: our
   paragraph space is **1.9×** the reference's with each side's spacing scale removed (41.21/0.8
   against 22.08/0.9), and that is what pushes the fit one row deeper than the reference's. Two
   blind reviewers on that page, one before the change and one after, both put the inter-bullet
   gap at the top of their list, and the earlier one named the mechanism. It is now *also* the
   most likely driver of the eight `/Tf` regressions, all of which are `.ppt`. Start there and
   re-run `tf-agreement.py` — it is the instrument that can see it and the gate cannot.
2. **The reference has a second fit mode on `.ppt` that does not round to whole points.**
   `Lepore.ppt` p2: the reference draws **20.0 ×11 and 20.4 ×6 on one page**, and 20.4 is
   24 × 0.850 exactly. `setRoundFontSizeToPt` is on only inside `setupAutoFitText`, so those
   shapes are not going through it, yet they are on a `constScaleLevels` font scale. Find which
   `SdrFitToSizeType` they take. Seven of the eight regressed documents carry such sizes — but the
   control above says that is not a classifier, so measure it, do not assume it.
3. **`8_P-Pavese_AIRBUS…pptx`'s missing orange table backgrounds** — untouched this round and still
   the third item. Its `a:tblPr` does name a style id and the reference draws 30 `#FBECE7` +
   25 `#F8D7CD` fills; find out whether that id resolves against the 74 built-in table styles
   ported from `predefined-table-styles.cxx`. Its `abs_ink` is **47.76**, the second largest on the
   track. An earlier brief dismissed this and was wrong; the user was right.
4. **`pitchFamily`'s family nibble** — still a decision for the user, not a patch. Nothing has
   changed since r50 measured it.
5. `038_Competitive_Advantage_Card` and `035_Chemistry_Column_PowerPoint_Chart` — still the only
   two genuine content differences in the old `text` pool, both in chart labels.

## The audit, run rather than recommended — and it is a corpus-wide finding, not a slides one

The line in `SlideAutofit`'s remarks that said *"this is a port of 24.2 and the reference is now
26.2.4.2, check which version wrote the reference before porting anything out of this tree"* had
been sitting in the file, correct and unread, for the whole time the container has been on
26.2.4.2. It cost the largest single ink movement this track has recorded.

So the one-line audit was run:

```sh
grep -rn "24\.2\.7" dotnet/src --include=*.cs
```

**47 hits in 29 files, and only one of them is the one this round fixed:**

| project | files carrying a claim measured against 24.2.7.2 |
|---|---:|
| `XRay.Office.WordProcessing` | 8 |
| `XRay.Office.Spreadsheets` | 8 |
| **`XRay.Office.Presentations`** | **6** |
| `XRay.Office.Text` | 4 |
| `XRay.Office.Core`, `XRay.Office.Ooxml`, `XRay.Office.Rendering` | 1 each |

`CLAUDE.md` § "This container" already says that *"individual claims calibrated to 24.2.7.2
behaviour … are now claims about a superseded binary and each needs one re-check before it is
relied on"*, and names three. There are forty-seven. This round re-checked one of them and it was
wrong in a way worth 11% of the track's ink and 164 pages of exact font sizes.

The remaining five in `XRay.Office.Presentations` are `SlideTextLayout.cs`, `PptxTextStyles.cs`,
`SlideDrawing.cs`, `OdpSlideLayout.cs` and `PptxSlideLayout.cs`. **The other twenty-three files are
words' and sheets' and are named here because no slides round will ever open them.** Each is a
cheap, self-contained re-check of exactly the shape this round ran: read the claim, author the
variant it was measured on, render it through the installed `soffice`, compare.

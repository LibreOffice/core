# slides-r54 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
base `4b50291b09d`, branch `wt-slides-r54`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`. Read
`prediction.md` beside this file first; it was committed as `e7a62caeeb8`, before anything was
built or rendered post-change.

## Baseline, and it reproduced to the digit

| | briefed | measured |
|---|---|---|
| passing over `MANIFEST.tsv` | 199 of 302 | **199 of 302, 0 disagreements** |
| `abs_ink` / signed / major | 1233.54 / — / 432 | **1233.54 / 913.68 / 432** |
| `tf-agreement` mean | 0.75210 | **0.75210** |
| exact `/Tf` pages | 1558 of 4515 | **1558 of 4515** |

## The whole round

| | base | final |
|---|---:|---:|
| passing | **199 of 302** | **199 of 302** |
| `abs_ink` | 1233.54 | **1147.17** |
| signed ink | 913.68 | 831.85 |
| major pages | 432 | **403** |
| `tf-agreement` mean | 0.75210 | **0.77053** |
| exact `/Tf` pages | 1558 of 4515 | **1709 of 4515** |
| page counts changed | | **0 of 302** |

**42 documents moved: 38 improved, 4 worsened.** The four regressions, named rather than netted:

| Δ | document | before → after |
|---:|---|---|
| **+0.27** | `undp_presentation_revised_17_may.ppt` | 20.30 → 20.57 |
| **+0.14** | `ws_prod…European-Safety-Strategy-Initiative.ppt` | 7.09 → 7.23 |
| **+0.14** | `pods05.ppt` | 9.02 → 9.16 |
| **+0.13** | `Thailand17.ppt` | 17.74 → 17.87 |

Two of the four are the round-53 shape rather than real: `baseline-agreement.py` puts
`ws_prod…European-Safety` at **1.0764 → 0.0282** mean |dy| and `pods05` at **1.0251 → 0.7319**
while their unsigned ink rose. `undp` (1.2307 → 1.2346) and `Thailand17` (806 → 781 paired
baselines, mean flat) are the two genuine ones, and both are under a third of a point of ink.

The largest improvements: `ITE106-Chapter 4.ppt` **18.85 → 5.86**,
`gfopportunitiesforlinkagespres` **16.11 → 5.75**, `Lepore.ppt` **9.76 → 0.53**,
`FAA_Form_337.ppt` 21.84 → 14.26, `BUS-Chapter 05.ppt` 10.01 → 2.66, `ws_prod…ESM.ppt`
10.86 → 4.94, `2015-Civil-Rights-Website-training.ppt` 36.23 → 30.32,
`joint_user_outcomes` 9.33 → 4.47, `Employment-Based_I-485.ppt` 9.74 → 6.21,
`berlin.ppt` 7.24 → 3.83, `010605Vul.ppt` 5.59 → 1.90, `RESPA_-_Section_8_Webinar.ppt` 6.95 → 3.49.

## 1. The `.ppt` autofit's spacing reduction — the brief's item 1, and the brief's condition is wrong

### The mechanism is measured, on an authored known-answer `.ppt`

Round 53 read the chain out of the C++ and did not measure it. Authoring the deck needed one fact
round 53's plan did not have: **`soffice --convert-to ppt` cannot preserve `a:normAutofit`, because
autofit is not spelled anywhere in the binary format.** `svdfppt.cxx:1030-1039` infers it from the
TextHeaderAtom's *instance* — Body, HalfBody or QuarterBody are fitted and nothing else is — and a
round-tripped text box comes out instance 4, TextInShape. Measured: the first cut of the probe drew
all fifteen slides at the stated 40 pt over 21 overflowing lines. `ppt-patch-kind.py` flips the
chosen TextHeaderAtoms to instance 1; the record body is a single `uint32`, so the edit is
length-preserving and needs no container-length arithmetic.

`make-ppt-fit-probe.py` → `soffice --convert-to ppt` → `ppt-patch-kind.py` → render both halves.
Fifteen slides, one 360 × H pt box each, H from 60 to 200 pt, three paragraphs of 40 pt text.

| box | `.pptx` `/Tf` | pitch | pitch/em | `.ppt` `/Tf` | pitch | pitch/em |
|---:|---:|---:|---:|---:|---:|---:|
| 60–110 pt | 10.006 | 9.609 | **0.960** | 10.006 | 12.087 | **1.208** |
| 150 pt | 15.987 | 15.364 | **0.961** | 13.011 | 15.661 | **1.204** |
| 190 pt | 15.987 | 15.364 | **0.961** | 15.987 | 19.233 | **1.203** |

**The `.pptx` half draws `1.2 × 0.8 × em`; the `.ppt` half draws `1.2 × em`.** Same box, same text,
same fit table. And because the binary side's lines are taller the fit *search* lands on a different
row — at a 150 pt box the reference draws 13.011 pt on the binary side against 15.987 on the OOXML
side. Ours reproduced the `.pptx` half on the `.ppt` at the base commit: **6 of 15 slides right, and
the six only by sitting on `FitLevels`' floor.** After: **15 of 15**, `/Tf` to 0.0004 pt and pitch to
0.0002 pt.

A second, independent confirmation in the source that round 53 did not have: **the UNO route and the
binary route disagree at exactly 100.** `SvxLineSpacingItem::PutValue`
(`editeng/source/items/paraitem.cxx:194-202`) reads `style::LineSpacingMode::PROP` and writes
`eInterLineSpaceRule = Off` *when the height is exactly 100* and `Prop` otherwise — that is the path
every OOXML and ODF line spacing takes. The `.ppt` importer calls `SetPropLineSpace(100)` directly,
and `lspcitem.hxx:86-91` shows that setter writes `Prop` unconditionally. The defect is `.ppt`-only
by construction, and no change to `XRay.Office.Text` was needed.

### The condition is refuted, by an A/B over the whole track

The brief's rule — a paragraph is exempt when it states a line feed **or** its first portion states
a typeface index (`svdfppt.cxx:6266-6271`) — was implemented in full, with the third term
`GetAttrib` adds for depth > 0 in a `TextInShape`/`Subtitle` object, and swept. Then "every `.ppt`
paragraph" was implemented and swept. Both against the same base and the same reference PDFs:

| | base | **A** — the record's own disjunction | **B** — every `.ppt` paragraph |
|---|---:|---:|---:|
| `abs_ink` | 1233.54 | 1220.48 (**−13.06**) | **1147.58 (−85.96)** |
| major pages | 432 | 432 | **403** |
| `tf-agreement` | 0.75210 | 0.75398 | **0.75866** |
| exact `/Tf` pages | 1558 | 1562 | **1565** |
| documents moved | | 13 (9 better, 4 worse) | 34 (**30 better**, 4 worse) |

**B by a factor of six and a half.** `Lepore.ppt` is the case that identifies it and it is decisive
rather than statistical: its body paragraph's mask is **0** and its character run states only a font
**height** — soft by both of the record's terms, and by the third — and the reference nevertheless
draws it at a pitch of `1.2 × em` under a 0.850 font scale. Under A it does not move at all; under B
it goes **9.76 → 0.53**.

`GetAttrib` has two further hardness terms that neither this reader nor a census of the record can
evaluate — a destination instance of `TSS_Type::Unknown`, and a comparison of the source instance's
master level against the destination instance's when a Body-kind object carries no
`OEPlaceholderAtom`. The honest statement is that one of them always fires, and that the measured
outcome on 26.2.4.2 is simply: **no `.ppt` paragraph takes the `::Off` arm.** The record-level
census stands as a fact about the corpus and not about the rule: over the 42 of 51 `.ppt` documents
its parser reads, **3736 paragraphs, 2156 hard (1947 by font index, 436 by line feed), 1580 soft**.

**The authored deck could not have chosen between A and B and this is worth saying plainly.**
LibreOffice's own PPT export writes `lf=100` *and* `font=1` on every paragraph it emits, so every
paragraph of the probe is hard and both rules pass it 15 of 15. A known-answer deck built by round
trip inherits the exporter's habits; the corpus A/B is what discriminated.

## 2. `Lepore.ppt` — the brief's item 2, closed

Its page 2 draws 20.0 × 11 and 20.4 × 6 on one page, and the brief was right that 20.4 = 24 × 0.850
exactly. The pair is the answer: **the 20.4 figures are the six bullets and the 20.0 figures are the
eleven text lines.**

`Outliner::setRoundFontSizeToPt` — which the fit turns on and nothing else does — rounds a run's
scaled height to a whole point, twice. `Outliner::ImpCalcBulletFont`
(`editeng/source/outliner/outliner.cxx:851-855`) never reaches it:

```
double fFontScaleY = pFmt->GetBulletRelSize() / 100.0 * getScalingParameters().fFontY;
double fScaledLineHeight = aStdFont.GetFontSize().Height() * fFontScaleY;
aBulletFont.SetFontSize(Size(0, basegfx::fround(fScaledLineHeight)));
```

One multiplication and one `fround`, taken on the item's own height in hundredths of a millimetre.
24 pt is 847 units; `fround(847 × 0.85) = 720` units = **20.409 pt**, which is what the reference
draws, against `round(24 × 0.85) = 20` → **20.013 pt** for the text. We drew both at 20.013.

After both fixes, `Lepore.ppt` page 2 draws **20.4094 and 20.0126** against the reference's 20.409
and 20.013, with eleven text lines to its eleven and every text baseline within 0.2 pt. The document
goes **9.76 → 0.53** `abs_ink`.

**The marker fix is worth far more in `/Tf` agreement than in ink, and that is the point of measuring
the quantity the change controls.** On top of B it is worth **−0.41 `abs_ink`** — a bullet is a
small amount of ink — and **`tf-agreement` 0.75866 → 0.77053 with exact-`/Tf` pages 1565 → 1709**.
144 pages went from one wrong size in the multiset to exact.

`baseline-agreement.py` on `Lepore.ppt` reads **269 paired at 0.0264 → 333 paired at 0.1380**: 64
baselines that could not be paired at all before — page 2's whole body, whose sizes did not match —
now can, and the mean over the larger set is 0.138 pt. Reported as a widening of coverage, not as a
regression, and both figures are given so it can be read either way.

What remains on that page is the marker's **vertical** placement: we draw the bullet 0.92 pt above
the text baseline and the reference draws it 0.99 pt below, a 1.9 pt gap. That is
`ALIGN_BOTTOM`/`aBulletArea.Bottom()` (`outliner.cxx:909-919`) and it is left open.

## 3. `NAS-Infrastructure-Roadmaps-v16.0.pptx` — the brief's item 3 is the wrong diagnosis, and the right one is bigger

### The vision round, and what it cost to check it

Three fresh blind subagents, none of which read this brief, the project docs or any source, each
given one composed page and nothing else. Page 8 was chosen because it is the document's **maximum**
`|ink|%` (5.53 of 137 pages, 55 major) and given to **two** reviewers independently; page 99 is the
second (4.95) and went to a third.

**Both page-8 reviewers ranked the same finding first, described it in the same terms, and were
both wrong.** They reported that the shaded region right of the 2026 column — the largest coloured
area on the page — is a pale lavender on our side and a saturated blue on the reference's, and both
marked it *confident*. A colour histogram of the same region says the two sides draw
`(204,229,255)` and `(230,230,255)` **in the same two colours and in the same proportions**, in the
150 dpi rasters *and* in the composed image the reviewers actually saw. There is no difference.

This is the third instance of `HANDOVER.md` § 7's rule and it strengthens it: the two reports were
about the same *object* and the page was chosen for a stated reason — the two discriminators § 7
offers — and the reading was still an artefact. **The one discriminator that held is the third:
a different instrument.** Run it before dispatching anything.

The same reviewer B, asked to count, reported that **"neither half draws more things than the other
— the object inventory matches item for item"**. Our page 8 writes 102 `BT` blocks to the
reference's 99. **So the brief's "66 text blocks to the reference's 61 — a visibility question" is
not what the page shows**, and NAS's ink is not surplus shapes.

### What it is: we draw rotated text through a different operator, and on 197 pages we draw none

Reviewer C, on page 99, reported that three band titles the reference draws as rotated left-margin
labels we draw as horizontal text inside the plot, and that two more strings are drawn **upside
down** on our side. Confirmed by a second instrument: on page 99 the reference's content stream
carries **101 rotated `Tm`** and ours carries **one `cm`, not rotated, and no rotated `Tm` at all**.

**The obvious census of this is wrong and its first cut was.** LibreOffice rotates text by writing a
rotated `Tm` inside `BT … ET`; X-Ray.Office rotates it by pushing a rotated `cm` and writing an upright
`Tm` or none. A `Tm`-only census reports that we draw *no* rotated text anywhere — **18832
operators to nought** — which is false: on page 8 we write 5 rotated `cm` against the reference's 77
rotated `Tm`, and both reviewers independently described that page's rotated spine captions as
identical. `rotated-text-census.py` counts text blocks rotated by **either** route, tracking `q`/`Q`:

| | |
|---|---|
| documents drawing rotated text on either side | **73 of 302** |
| rotated text blocks, ours → reference | **1097 → 1905** |
| **pages where the reference rotates text and we rotate none** | **197** |

`NAS-Infrastructure-Roadmaps-v16.0.pptx` alone accounts for 307 of the shortfall over 33 such pages
— which is what its 159.88 `abs_ink`, twice the next document on the track, actually is. The rest
are spread: `section_1_our_rights_presentation` 81 over 11 pages,
`ws_prod…M.017-(French)-France.ppt` 73 over 12, `2015-Civil-Rights-Website-training.ppt` 39 over 10,
`Structural Testing.pptx` 39 over 13, `attendance-updates-for-governors.pptx` 36 over 8.

And it runs both ways: `Demick_JetBlue.pptx` rotates **76 blocks where the reference rotates 8**,
`Sylva introduction session.pptx` 34 to 8, `8_P-Pavese_AIRBUS` 21 to 1. Over-rotation and
under-rotation are both present and they are probably not one defect.

**On the "we render better" list.** The brief said to treat that as a claim to re-measure. Measured:
NAS's page 8 is 3.18% differing pixels and page 99 is 4.43%, its 55 major pages are real, and the
reference draws rotated labels we draw flat across 33 of its pages. It is not a false positive.

## Measured against the prediction

| # | predicted | measured |
|---|---|---|
| 1 | **verdict movement 0**, surprise outside −1…+2 | **0** — 199 → 199, `MANIFEST` agrees on all 302 |
| 2 | page counts: 0 of 302 | **0 of 302** |
| 3 | the known-answer deck lands 15 of 15 | **15 of 15**, `/Tf` to 0.0004 pt, pitch to 0.0002 pt — but see the caveat above: **the deck could not discriminate the two candidate rules** |
| 4 | 18–30 documents move, **all `.ppt`** | **wrong twice over.** The predicted rule moved **13**; the shipped rule moves **34**; the marker fix takes it to **42**, of which **8 are `.pptx`** |
| 5 | `abs_ink` −5 to −25 | **−86.37.** Outside the range by more than a factor of three |
| 6 | `tf-agreement` 0.753–0.762, exact pages rise | **0.77053** and **1709**, above the range |
| 7 | `gfopportunities` p6 draws 26 / 22 / 17 at a pitch of `1.2 × em` | **held, and better than stated** — every baseline on that page now matches the reference's, sizes 36 / 25.994 / 21.997 / 17.008 exactly and y within 0.06 pt |
| 8 | the 219 "both shrunk" blocks do not move — no document outside the change list moves `tf-agreement` by > 0.001 | **refuted.** Under the predicted rule alone, `Thailand17` moved −0.01796, `undp` −0.01882 and `architecture6` +0.01336, none of them on the change list, and two of the three the wrong way |
| 9 | no cross-track reach | **held.** Four files, all in `XRay.Office.Presentations`; WordProcessing 1096 and Spreadsheets 925 green |

**Prediction 5 is the round's worst call and in the opposite direction to round 53's.** Round 53
extrapolated a census of *candidates* into an `abs_ink` estimate and over-shot by a factor of three;
this round censused *symptoms already visible in the rendering* — 100 disagreeing constant-pitch
blocks in 24 documents — and under-shot by more than three, because a rule that changes the fit
*search* changes pages the symptom census cannot see at all. Both failures have the same root: an
`abs_ink` point estimate needs a model of how much of a page moves, and neither round had one. The
next round should predict a **sign and a rank**, and say so, rather than a range.

Blind spot #1 of the prediction — "whether a shape is autofitted at all, and which
`constScaleLevels` row it lands on, are invisible in the record" — is exactly the term that made
prediction 5 wrong, and it was written down before the sweep.

## Refutations, collected

1. **The brief's condition for the `.ppt` spacing rule.** Right mechanism, wrong condition; A/B above.
2. **`NAS` page 8's "five surplus text blocks".** A blind counter and a `BT` count both say the
   inventories match; the document's ink is rotated text.
3. **Two blind reviewers agreeing, confidently, on a difference that a histogram says is not there.**
4. **My own `Tm`-only rotation census**, which reported 18832 to nought and was an artefact of the
   two stacks using different operators. Caught by the fact that the same reviewers had called page
   8's rotated captions identical.
5. **Prediction 8**, my own control.

## Tests

Two new files, ten new tests, **8 net** against the briefed base (two of the ten replace nothing;
the count is 4703 − 4695).

| test | mutation | outcome |
|---|---|---|
| `PptStatedLineSpacingTests` (4 theory cases + 2 facts) | `LineSpacingStated = true` → `false` | **DETECTED**, 6 of 6 |
| `SlideMarkerScaleTests.AFittedBodyDrawsItsBulletUnroundedAndItsTextRounded` | `ScaledMarker(...)` → the old `scaling.Scaled(...)` | **DETECTED** |
| `SlideMarkerScaleTests.AnUnfittedBodyDrawsItsBulletAtItsTextSize` | same | **survives — a drift guard by design**, and the control that says the two sizes below come from the fit |

`SlideMarkerScaleTests`' first cut asserted that the bullet is the *larger* of the two sizes and
failed: the run's size is `round(stated × scale)`, so it lands **above** the bullet's unrounded
`stated × scale` whenever the fraction is a half or more. On the fixture the fit answers 0.400 and
24 × 0.400 = 9.6 rounds up to 10, so the bullet is the smaller. The test now asserts the invariant —
exactly one of the two is a whole number of points — and says why in the code.

Ten non-Fidelity projects, run **one at a time** with nothing else running: Core 337, Containers 109,
Text 611, Vector 295, Rendering 150 (+1 skipped, the same `PdfFontTests` case as at baseline),
Markup 259, OpenDocument 125, WordProcessing 1096, Spreadsheets 925, **Presentations 796** —
**4703 passed, 0 failed, 1 skipped**, against the briefed 4695/0/1: **+8**.

`cd dotnet && dotnet build -v q -nologo` → **0 warnings, 0 errors**.

## Shared layers: none

Four source files: `Layout/SlideText.cs`, `Layout/SlideTextLayout.cs`, `Layout/SlideAutofit.cs` and
`MsBinary/PptTextBody.cs`, all in `XRay.Office.Presentations`. `XRay.Office.Text`'s `LineSpacingRule` was
deliberately **not** touched — the distinction "states exactly 100%" versus "states nothing" lives on
`SlideParagraph`, which no other family constructs — so words and sheets are unreachable by type.
Their suites were run anyway and are green. **No cross-track sweep is owed.**

`MANIFEST.tsv` needs no change: zero verdicts moved and no document changed kind.

## The previously-passing batches

The sweep glob is `slides/*`, so `slides/done-001` … `done-015` are inside both legs rather than
checked afterwards, and the whole-track reconciliation against `MANIFEST.tsv` covers the same ground
document by document — **302 of 302 agree in both legs**.

## The 24.2.7.2 audit

One site re-checked, by probe against the installed binary.

| site | claim | outcome |
|---|---|---|
| `PptxSlideLayout.cs:763` `CellBody` | a PPTX table cell takes font-independent line spacing — one em of ascent, a 1.2 em box — where 24.2.7.2 drew the face's own 0.907 em | **VERIFIED 26.2.4.2.** `make-cell-baseline-probe.py`: one table, one cell, zero margins, top-anchored, six stated sizes 10–40 pt. The reference's first baseline sits **1.0007, 1.0005, 1.0003, 1.0003, 1.0002 and 1.0002 ems** below the cell's top edge — one em on all six — and our six land on the reference's to **0.000 pt** |

`PptxSlideLayout.cs`'s other two (`:940` themed drop shadow, `:1579` gradient focus truncation) and
`SlideDrawing.cs`'s two (the metafile fill) are untouched and each needs an authored variant read
back through `soffice`. Corpus-wide the list falls from 44 open hits to 43; 12 lines are now marked,
9 verified, 2 wrong, 1 undecided.

## Left open, in the order the next round should take them

1. **Rotated text.** 197 pages where the reference rotates text and we rotate none, 73 documents
   drawing rotated text on either side, and it runs both ways — `Demick_JetBlue.pptx` rotates 76
   blocks where the reference rotates 8. This is the track's largest unworked front and it is what
   `NAS-Infrastructure-Roadmaps-v16.0.pptx`'s 159.88 `abs_ink` actually is: 307 missing rotated
   blocks over 33 of its pages. Start from `a:bodyPr/@vert` and `@rot` on the OOXML side and the
   Escher `txflTextFlow` property on the binary side, and **use `rotated-text-census.py`, whose own
   remarks say why a `Tm`-only count is wrong.**
2. **The fitted bullet's vertical placement.** `Lepore.ppt` page 2 now draws both sizes exactly and
   puts the bullet 1.9 pt too high: `Outliner::StripBullet` sets `ALIGN_BOTTOM` for a symbol and
   draws it from `aBulletArea.Bottom()`, where a number is drawn at `nFirstLineMaxAscent`
   (`outliner.cxx:909-919`). One rule, two arms, and `SlideMarker.IsSymbol` already carries the
   discriminator.
3. **`2015-Civil-Rights-Website-training.ppt`** is now the track's second-largest at 30.32 and its
   `baseline-agreement` mean is 1.4915 over 1228 pairs — the largest remaining text-layout residue
   on a single document.
4. **The audit**: `PptxSlideLayout.cs` 2 remaining, `SlideDrawing.cs` 2, `PptxTextStyles.cs` 1,
   `OdpSlideLayout.cs` 1. `SlideDrawing.cs`'s pair is the one whose own remarks say the correlation
   it rests on (inline metafile keeps its fill, package entry loses it) may not be the cause.
5. The `pitchFamily` family nibble — still a decision for the user, not a patch. Unchanged since r50.

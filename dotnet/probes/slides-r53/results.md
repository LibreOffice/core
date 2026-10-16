# slides-r53 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
base `41445736a8c`, branch `wt-slides-r53`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`. Read
`prediction.md` beside this file first; it was committed as `6df1fdb86cd`, before anything was
built or rendered post-change.

## Baseline, and it reproduced exactly

Whole-track sweep at the base commit — our side re-rendered, the reference PDFs reused from
`scratch-r52-slides/ink-after/ref`, which is legitimate because nothing has touched `soffice`.

| | |
|---|---|
| sweep's own `TOTAL` | 311 rows, MATCH 201 |
| scored over `MANIFEST.tsv`'s 302 slides paths | **199 of 302, 0 disagreements** with the `status` column |
| `abs_ink` / signed / major | **1238.56 / 913.58 / 434** |

Round 52 recorded 1238.64 / 913.80 / 434. The 0.08 between them is the words track's
`DrawingChartPlot.cs`, merged after r52's slides leg, which its merge note says moved exactly one
slides row. A second, independent confirmation: `tf-agreement.py`'s mean over the same 302
documents reads **0.75160**, which is round 52's closing figure to the digit.

## Two fixes shipped, measured separately, and both confined to `XRay.Office.Presentations`

### 1. A `.ppt` blank line takes the height of the run it sits in — `PptTextBody.Runs`

`Runs` walks the character-property runs accumulating `position`, records `atStart` when
`start >= position && start < runEnd`, and broke when `position >= end`. For an **empty**
paragraph `start == end`, and the runs are contiguous from zero, so the run that *ends* at
`start` was the last one it saw — one short of the run that *contains* `start`. `atStart` was
therefore **never** found for any empty paragraph except one at text position 0, and every blank
line in a `.ppt` fell back to the master level's character height.

Three independent routes agree on the answer, on `ITE106-Chapter 4.ppt` p7:

1. **The record.** `ppt-style-dump.py` — a Python parser that shares nothing with the C# reader —
   reads the body text as 537 characters with `\r` at 116, 117, 237, 238, 392, 393 and 536:
   **paired** returns, so each bullet is followed by a genuinely empty paragraph. The character
   runs are `117@24, 1@12, 120@24, 1@12, 154@24, 1@12, 143@24`, and the blank paragraphs start at
   117, 238 and 393 — exactly the three one-character runs stating **12 pt**.
2. **The reference's own model.** LibreOffice's flat-ODF export of the deck gives those blank
   paragraphs `fo:margin-top="0.106cm"` = 3.004 pt = `12 × 20/80`, against the text paragraphs'
   `"0.212cm"` = `24 × 20/80`.
3. **The rendered page.** The reference's inter-bullet baseline gap decomposes as
   `28.800 + 3.004 + 1.2×12 + 6.008 = 52.212` against the **52.214** it draws.

Ours needed `h(blank) + marginTop(blank) = 39.615` to explain its 68.769 gap, which is
`1.2 × round(32 × 0.925) × 0.9 + 32 × 0.25 × 0.9 = 39.60` — **a blank line of 32 pt, the level
default.** The two derivations, one from the record and one from the geometry, agree.

### 2. A stated line **height** moves the ascent with it — `SlideTextLayout.Stated`

EditEngine tests four rules in order — `SvxLineSpaceRule::Min`, `::Fix`,
`SvxInterLineSpaceRule::Prop`, `::Off` (`impedit3.cxx:1530-1602`). The first two were not
transcribed at all: a stated height went through `LineSpacingRule.Apply` — Writer's whole-twip
arithmetic — and the ascent was left at one em. The reference does
`MaxAscent += fround(scaleY(stated)) − naturalHeight`.

Measured on `make-linespace-probe.py`, twelve `a:lnSpc/a:spcPts` boxes (10, 24 and 50 pt of
stated height in 11, 12, 24 and 40 pt text). Before, our ascent was the em on all twelve — for
12 pt text in a stated 24 pt line that is **9.58 pt of vertical displacement of the whole block**.
After, all twelve land on the reference's pitch to a thousandth of a point and on its first
baseline to the uniform 0.028 pt every case on that probe carries.

This one was found by the audit rather than by the brief: it is a *fifth* branch that
`SlideTextLayout.cs`'s six `24.2.7` sites collectively described and none of them implemented.

## Measured against the prediction

| | predicted | measured |
|---|---|---|
| **verdict movement** | **0** (surprise outside −2…+2) | **0** — 199 → 199, `MANIFEST` agrees on all 302 |
| page counts | 0 of 302 | **0 of 302** |
| **`ITE106` p7 known answer** | `/Tf` 21.9969 → **24.009**, pitch 23.754 → **28.800**, gap 68.769 → **52.212 ± 0.05** | **24.0094 / 28.800 / 52.200** — and all ten baselines within **0.043 pt** of the reference's |
| documents whose rendering moves (fix 1) | 20–26, **all `.ppt`** | **23, all `.ppt`** |
| `abs_ink` (fix 1) | −15 to −45 | **−6.77** — the point estimate was **far too high** |
| `tf-agreement` rises | yes | 0.75160 → **0.75210**, exact pages 1552 → **1558** |
| at least 3 of r52's 8 regressed documents improve on `tf-agreement` | yes | **wrong: 2 moved and both got worse.** See below |
| the three r52 regressions absent from the census stay put | yes | **held** — `ws_prod…ESM`, `010605Vul`, `EG1_dsrc tech` unmoved on `tf-agreement` |
| word counts move both ways | yes | **5 moved, 3 nearer and 2 further** |

**The `abs_ink` estimate was the round's worst call and is stated as such.** −15 to −45 was
extrapolated from `ITE106`'s 19.22 and the census's 26 documents; the truth is that most of the
673 blank paragraphs sit on runs whose stated height is close to the level default, so the census
was, exactly as `prediction.md` warned it might be, an upper bound on *changes* rather than an
estimate of them.

### Prediction 6, refuted by my own measurement

I predicted that at least three of round 52's eight `/Tf` regressions would improve, because five
of them appear in the blank-paragraph census. Two moved — `Lepore.ppt` **−0.00445** and
`gfopportunitiesforlinkagespres` **−0.03408** — and both moved the **wrong way**.
`FAA_Form_337`, `joint_user_outcomes` and `WC_Update-Aug03` did not move at all despite carrying
3, 2 and 10 affected blank paragraphs each. Appearing in the census is not the same as the change
reaching the shape whose fit row is wrong, and I wrote the prediction as though it were.

The **control** in the same prediction held: the three regressed documents *absent* from the
census were predicted unmoved and are unmoved. So the instrument is discriminating; the
hypothesis was wrong.

## The whole round, base → both fixes, scored over `MANIFEST.tsv`

| | base | final |
|---|---:|---:|
| passing | **199 of 302** | **199 of 302** |
| `abs_ink` | 1238.56 | **1233.54** |
| signed ink | 913.58 | 913.68 |
| major pages | 434 | **432** |
| `tf-agreement` mean | 0.72571 → 0.75160 (r52) → **0.75160** | **0.75210** |
| pages with an exact `/Tf` multiset | 1552 of 4515 | **1558 of 4515** |

**35 documents moved: 23 improved and 12 worsened.** The regressions, named rather than netted:

| Δ abs | document | abs before → after |
|---:|---|---|
| **+3.20** | `NAS-Infrastructure-Roadmaps-v16.0.pptx` | 156.68 → 159.88 |
| **+1.91** | `undp_presentation_revised_17_may.ppt` | 18.39 → 20.30 |
| **+0.92** | `gfopportunitiesforlinkagespres_2010_en.ppt` | 15.19 → 16.11 |
| +0.19 | `concepts-surrounding-cloud-computing…ppt` | 9.14 → 9.33 |
| +0.17 | `Lepore.ppt` | 9.59 → 9.76 |
| +0.17 | `0335fab9-79f0-4944-b92c-f223837ca2d8.ppt` | 1.60 → 1.77 |
| +0.13 | `ws_prod-g-doc-Events-Part-M-presentation.ppt` | 2.78 → 2.91 |
| +0.13 | `2015-Civil-Rights-Website-training.ppt` | 36.10 → 36.23 |
| +0.07 | `pres_ioc_phuket.ppt`, `ws_prod…MDM.032-(ENGLISH)-CZ.ppt` | |
| +0.05 | `Airport Planning 09112013.ppt` | |
| +0.01 | `redac-nasops-201503-RIRP-portfolio-update.pptx` | |

The largest improvements: `Thailand17.ppt` **22.19 → 17.74**, `8.16_AOD_FINAL_Provider_Training`
**25.57 → 23.23**, `ws_prod…M.017-(French)-France.ppt` **21.01 → 18.80**,
`Session-1-Presentation-Reporting-Forms` 2.04 → 1.51, `ITE106-Chapter 4.ppt` 19.22 → 18.85.

## The instrument that says the ink rises are not regressions

`baseline-agreement.py` scores where our text baselines sit against the reference's: per page,
bucket both sides by rounded `/Tf`, and pair **in order** within a bucket, only when the two
buckets hold the same number of baselines. In order rather than by nearest neighbour, because a
nearest-neighbour pairing rewards a whole-block shift by matching each line to its neighbour —
the bug that manufactured 142 phantom box notes for round 50.

**Known-answer control:** run on two documents this round does not touch, `Demick_JetBlue.pptx`
and `Wildlife for REDAC September 11.pptx`, it reports **identical figures to the digit** on both
legs (6.0620 / 27 / 44 and 0.1230 / 429 / 442).

| document | Δ abs_ink | mean \|dy\| base → final | within 0.1 pt |
|---|---:|---|---|
| `undp_presentation_revised_17_may.ppt` | **+1.91** | **8.3728 → 1.2307** | 182 → 212 of 313 |
| `NAS-Infrastructure-Roadmaps-v16.0.pptx` | **+3.20** | 2.0808 → 2.0607 | 2381 → 2418 of 2909 |
| `Lepore.ppt` | +0.17 | **1.1579 → 0.0264** | 245 → 261 of 269 |
| `concepts-surrounding-cloud-computing` | +0.19 | 4.0823 → 3.5775 | 161 → 171 of 191 |
| `2015-Civil-Rights-Website-training.ppt` | +0.13 | 2.3911 → 2.3133 | 771 → 776 of 1182 |
| `gfopportunitiesforlinkagespres` | +0.92 | 2.3878 → 2.3100 | 147 → 146 of 205 → 196 |
| `Thailand17.ppt` | −4.45 | 3.4366 → 0.5804 | 580 → 661 of 806 |
| `8.16_AOD_FINAL_Provider_Training` | −2.34 | 3.6705 → 1.7085 | 618 → 685 |
| `iep-amount-frequency-for-webinar.ppt` | −0.27 | **1.7024 → 0.0360** | 259 → 429 of 459 |
| `AATF-Fact-Sheet-2025.pptx` | −0.35 | 0.8874 → 0.3540 | 40 → **68 of 69** |
| `Session-1-Presentation-Reporting-Forms` | −0.53 | 1.2303 → 0.4945 | 196 → 227 of 249 |
| `COMSTAC_11_5_2021am_2.pptx` | −0.10 | 0.6895 → 0.4898 | 473 → 528 of 610 |

**Every document examined improved, including all five ink regressions.** `NAS` page 8 is the
worked case: its ten stated-height baselines were 1.58 pt out and are now within **0.014 pt** of
the reference's, one for one, and the page's own `diff%` fell 7.89 → 7.71 while its unsigned ink
rose 2.34 → 5.53. That page draws **66** text blocks to the reference's **61**, so moving the
text it shares onto the correct baseline changes how the surplus overlaps. This is round 51's
unmasking effect with a per-baseline receipt.

## Refutations

### 1. `8_P-Pavese_AIRBUS…pptx`'s table fills — the brief's item 3 is stale

**The fills are already drawn at `41445736a8c`.** Rendering page 14 with this tree's own CLI and
counting connected regions gives **30 `#FBECE7` and 25 `#F8D7CD`** — the reference's figures
exactly. The route: `PptxSlideLayout.cs:737-739` reads `a:tblPr/a:tableStyleId`,
`DrawingTableStyle.Read` searches the package's `a:tblStyleLst` first and falls back at
`DrawingTableStyle.cs:185` to `DrawingPredefinedTableStyles.Create`, whose `Map` holds the
document's `{21E4AEA4-8DFA-4A89-87EB-49C32662AFE0}` at line 105 as *Medium-Style-2 / accent2*.
The colours check out independently: `accent2` is `ED7D31`, and DrawingML's gamma-corrected tint
at 20000 and 40000 gives `#FBECE7` and `#F8D7CD`, which is what `wholeTbl` and `band1H` state.
The fix landed in `c2fa7537f6b`. The document's `abs_ink` of 47.76 is spread over **26 pages with
one major page**, which was never the shape of 55 missing fills, and the brief's ranking of it as
"the second largest on the track, untouched" was reading a number that had already moved on.

Corpus reach of the built-in table: **36 documents name a `tableStyleId`, 10 of them name one
their own package does not define, and all 8 such ids fall in the five implemented groups.** Six
groups still return `null` at `DrawingPredefinedTableStyles.cs:158` and no corpus document needs
them.

### 2. `SlideChart` does **not** have `FrameChart`'s run-fusing defect

The round-52 merge note left this as a known cross-track item: "`SheetChart`/`SlideChart` still
run multi-line labels together". For slides it is **false**. `FrameChart.Text` shapes the label
with one `face.Shape(label.Text, …)` call, which draws `\n` as a zero-width nothing;
`SlideChart.Text` routes the label through `SlideTextLayout.Place`, which breaks on it.

Measured on the 12 corpus documents whose charts carry a two-line data label
(`showPercent` without `showVal`, or an explicit `\n` separator): the extracted percentages come
out as separate tokens on all four inspected, and `005_advanced_powerpoint_doughnut.pptx` and
`012_advanced_powerpoint_pie.pptx` sit at `abs_ink` **0.16** and **0.07** with **zero** major
pages — which a fused label could not produce. Nothing was changed. **`SheetChart` is the sheets
track's and this says nothing about it.**

### 3. My own prediction 6 — above.

## The 24.2.7.2 audit: `SlideTextLayout.cs`'s six sites, all six re-checked

The brief's instruction was to start item 1 here, because a stale site would *be* item 1's answer.
It was not: item 1's answer is in `PptTextBody`, and it was found by measuring the page. But the
audit paid for itself anyway — **it produced fix 2**, which none of the six sites described.

Each re-check is a probe against the installed 26.2.4.2, not a reading. The datelines are gone and
each site now names 26.2.4.2 and **2026-08-21**.

| site | claim | outcome |
|---|---|---|
| `:145` `OnGrid` | a shape's rectangle is an integer number of hundredths of a millimetre | **still correct.** Twelve boxes whose top steps by 40 EMU across 1944.000…1945.222 units draw at exactly **two** baselines, 444.9260 and 444.8980 pt — 0.0280 pt apart, one unit — with the transition on the half. Quantised, and by `round` |
| `:297` `HeightToLastNonEmpty` | only a run of empty paragraphs *at the end* is dropped | **still correct.** One body, one 240 pt autofit box: four empty paragraphs at the end fits at **18.992 pt** over 12 lines, as does none at all; three of them moved into the middle fits at **15.987** over 9, as does all four |
| `:792` | the `Off` branch has no four-fifths on the ascent | **still correct** |
| `:1007` | `fround` below 100%, truncation above | **still correct** |
| `:1095` `Proportioned` | the same, on the product | **still correct.** 40 authored `a:lnSpc/a:spcPct` boxes — ten percentages × four em sizes — pitch equal to the reference's on **40 of 40** |
| `:1133` `ProportionedAscent` | capped at `fround(txt × factor × prop × 0.8)` below 100%, moves with the whole change above | **still correct.** 35.676 pt at 93% of 40 pt against an arithmetic 35.712; 63.937 at 150% against 63.981 |

**And one recorded divergence is confirmed rather than merely surviving.** The comment beside
`Spacing` says `LineSpacingRule.Apply`'s clamp of a proportion below 50% up to 50 is Writer's rule
and not EditEngine's, and that dropping it is unmeasurable because nothing in the corpus states
under 60%. At 40% the reference draws a **19.191 pt** pitch on a 47.991 pt natural line, which is
`fround(0.40 × natural)` and not the 24.009 a clamp to 50% would give. **26.2.4.2 has no such
clamp either**, so the divergence is right and now measured rather than argued.

Corpus-wide the audit list falls from **48 sites in 30 files to 42**. `XRay.Office.Presentations`
still holds **9**: `SlideAutofit.cs` 4 (re-checked and fixed by r52; the datelines are historical
narrative about the superseded port and were left, which is arguably wrong and is flagged here),
`PptxSlideLayout.cs` 3, `SlideDrawing.cs` 2, `PptxTextStyles.cs` 1, `OdpSlideLayout.cs` 1.

## What the round found and did **not** ship

### The `.ppt` autofit's spacing reduction is disabled by a *hard font index*

`gfopportunitiesforlinkagespres_2010_en.ppt` p6 is the one document whose `tf-agreement` fell and
whose cause is understood. The reference draws its three blocks at **26 / 22 / 17 pt** where the
record states **28 / 24 / 18** — a font scale of 0.925, which is `constScaleLevels` row 2 — but at
a baseline pitch of **31.181 pt on a 25.994 pt em**, which is `1.2 × em` at spacing **1.000**. The
table has no `(0.925, 1.000)` row, and round 52 established on 36 one-slide decks that it has no
such row in 26.2.4.2 either.

The mechanism, read out of the tree and then checked against the record:

- `svdfppt.cxx:6267-6271` — `bIsHardAttribute` is set when the paragraph states a line feed **or
  when its first portion states a hard `PPT_CharAttr_Font`**, i.e. a typeface index.
- `:6285-6288` — it then puts `SvxLineSpacingItem` with `SetPropLineSpace(nPropLineSpace)`, and
  `lspcitem.hxx:86-91` shows that setter also sets `eInterLineSpaceRule = Prop`.
- `impedit3.cxx:1553-1602` — the four rules are an `else if` chain. The `Prop` arm does **nothing
  at all** when the proportion is exactly 100, and the `::Off` arm — the only place a
  non-proportional paragraph picks up the fit's `fSpacingY` — is then unreachable.

So **a `.ppt` paragraph whose first character run states a typeface index gets the autofit's font
scale and no spacing reduction.** Every character run in this shape states `font=0`;
`ITE106`'s state none, and its reference is unscaled, so it does not discriminate.

Not implemented. It would need `LineSpacingRule` to distinguish "states a proportion of exactly
100%" from "states nothing", which today are the same value, and it flips the spacing on a large
share of `.ppt` shapes. `pitch-ratio.py` over the reference's own renderings shows both formats do
draw 1.08 and 0.96 pitches — `.ppt` 106 and 53 blocks of 1375, `.pptx` 135 and 35 of 2274 — so the
effect is conditional and not universal, and it needs its own known-answer deck before anything
changes.

### A residual, bounded and attributed

Every case on `make-linespace-probe.py` — stated-height or proportional or plain — puts our first
baseline **0.028 pt** below the reference's, one hundredth of a millimetre, with the pitch exact.
That is the constant `SlideTextPlacementTests` already records as the shift LibreOffice's PDF
export puts on every pen; the `OnGrid` staircase above independently measures the same 0.0280 pt
as one map unit. It is not a stated-height effect and it does not appear on the corpus documents
measured here — `ITE106` p7's first two baselines agree with the reference's to **0.000 pt**.

## Tests

`PptBlankParagraphTests` **already claimed** that a blank line takes its run's height, and it
**passed before the fix**: its fixture states one character run over the whole text, so the blank
paragraph sits strictly inside it and the walk finds it on the first iteration. The theory added
here splits the runs at the blank paragraph — the shape every real deck writes — which is the only
place the walk went wrong. That is worth stating plainly: a test can assert the right property and
still not reach the defect, and this one did that for several rounds.

**Both new tests verified by reintroduction** with `verify-test.sh`:

| test | mutation | outcome |
|---|---|---|
| `PptBlankParagraphTests.TheRunThatBeginsAtABlankParagraphIsStillItsRun` | `if (position >= end && found) break;` → `if (position >= end) break;` | **DETECTED**, both theory cases |
| `SlideStatedLineHeightTests` (3 theory cases + 1 fact) | `if (Stated(…) is { } stated)` → `if (false && Stated(…) is { } stated)` | **DETECTED**, 4 of 5 |

The fifth, `APlainParagraphIsUnchanged`, correctly survives the mutation: it is the control that
says the new arm is not swallowing the ordinary case, and it is a drift guard by design.

Ten non-Fidelity projects, run **one project at a time**: Core 337, Containers 109, Text 596,
Vector 295, Rendering 150 (+1 skipped, the same `PdfFontTests` case as at baseline), Markup 259,
OpenDocument 125, WordProcessing 1083, Spreadsheets 895, **Presentations 787** —
**4636 passed, 0 failed, 1 skipped**, against the briefed 4629/0/1: **+7**.

Two whole-solution `dotnet test` runs died with `Fatal error. Internal CLR error. (0x80131506)`
while a corpus sweep was running. That is the "under load a test run reports failures that are not
there" trap arriving as a crash rather than as a failure; the per-project run above was made with
nothing else running.

`cd dotnet && dotnet build -v q -nologo` → **0 warnings, 0 errors**.

## Shared layers: none

Two source files, both in `XRay.Office.Presentations`:
`MsBinary/PptTextBody.cs` and `Layout/SlideTextLayout.cs`. Nothing in `Core`, `Containers`,
`Text`, `Vector`, `Rendering`, `Markup` or `XRay.Office.Ooxml`. `SlideTextLayout` is called only
from `XRay.Office.Presentations` (`SlideShapes`, `SlideTable`, `SlideChart`, `SlideDrawing`).
**No cross-track sweep is owed**, and the WordProcessing and Spreadsheets suites were run anyway
and are green at their briefed counts.

`MANIFEST.tsv` needs **no** change: zero verdicts moved and no document changed kind.

## The previously-passing batches

The sweep glob is `slides/*`, so `slides/done-001` … `done-015` are inside both legs rather than
checked afterwards. The whole-track reconciliation against `MANIFEST.tsv` covers the same ground
document by document — **302 of 302 agree in both legs** — which is the statement that no batch
was traded for another.

## Left open, in the order the next round should take them

1. **The hard-font-index spacing rule above.** It is the one understood cause of a `tf-agreement`
   fall this round, the mechanism is nailed to three specific lines of LibreOffice, and it needs a
   `.ppt` known-answer deck — which can be authored by round-tripping a `.pptx` probe through
   `soffice --convert-to ppt` and then reading the record back with `ppt-style-dump.py` to confirm
   what the round trip actually wrote.
2. **`Lepore.ppt` — the brief's item 2, now half answered.** Its baselines are **0.0264 pt** mean
   error after this round, from 1.1579. What remains is purely the drawn size, and 20.4 = 24 ×
   0.850 exactly, unrounded. With the geometry this close it is now a clean single-variable
   question.
3. **`NAS-Infrastructure-Roadmaps-v16.0.pptx`** is the track's largest `abs_ink` by a factor of
   two and its page 8 draws **66 text blocks to the reference's 61**. Five surplus blocks, not a
   layout error — that is a shape-visibility question, and it is worth more than anything left in
   text layout.
4. `PptxSlideLayout.cs`'s three audit sites and `SlideDrawing.cs`'s two, by the method above.
5. The `pitchFamily` family nibble — still a decision for the user, not a patch. Unchanged since
   r50.

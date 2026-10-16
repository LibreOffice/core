# slides-r51 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`fc-match Calibri` → `Carlito-Regular.ttf`, base `bd0f5ac1cf2`, `SOURCE_DATE_EPOCH=1700000000`,
`TZ=UTC`. Read `prediction.md` beside this file first; it was committed before anything was
rendered post-change.

## Baseline, and it reproduced exactly

Whole-track sweep, all 35 slides batches, reconciled document by document against `MANIFEST.tsv`
on a **case-folded** identity: **302 of 302 agree, 0 disagreements — 199 of 302 passing.**

The sweep's `TOTAL` line reads **311**. That is the case-insensitive mount, not the corpus: 9
documents are enumerated under two spellings each. `311 − 9 = 302`. The nine are listed in §6.

## Measured against the prediction

| | predicted | measured |
|---|---|---|
| **verdict movement** | **0** | **0** — 199 → 199 |
| page-count movement | not predicted | **0 of 302** |
| word-count movement | implied 0 | **0 of 302** |
| renderings moved (slides) | 11 named | **16** — the 11 named less 1, plus 6 unnamed |
| direction | `abs_ink` should fall | **1409.36 → 1394.03**, signed 1040.62 → 1026.61 |
| major pages | not predicted | 498 → **494** |

The prediction was **right, including its own stated limits**, and the way it was wrong is the
part worth keeping.

## The change: `a:clrChange` was a route, not a rule — the seventh instance

`grep -rn clrChange dotnet/src` returned **0 hits**. Everything else was already there:

| piece | state before this round |
|---|---|
| `ColourKnockout`, per-channel box match, binary alpha | `Core/Graphics/GlyphRun.cs:248` |
| the decoder that applies it, before duotone and luminance | `Rendering/Images/RasterImageDecoder.cs:185` |
| binary `.ppt` populating it from Escher property 263 | `MsBinary/PptSlideLayout.cs:1061` |
| **OOXML populating it from `a:clrChange`** | **absent** |

So the feature worked on `.ppt` and did nothing on `.pptx`, `.docx` and `.xlsx`.

### The corpus instance, and how it was found

`social-media-app-bulletin-january.pptx` p3, a document that **passes every gate column**. Its
wordmark is a 450 × 95 PNG, colour type 2, **91.6% pure `#000000`**, no alpha channel and no
`tRNS`, under a `clrChange` knocking black out. Drawn as stored it is a 337.5 × 71.25 pt opaque
black slab covering the words *Social Media* in the title. The title stays in the text layer, so
the word count never moves and no gate column can see it.

Measured in the picture's own box at 150 dpi: **91.6% near-black → 0.1%**, replaced by the page
background, wordmark intact, title legible.

### The blind reading and the measurement landed on the same mechanism

A reviewer that had not read this brief, could not read source or documentation and could not run
a command was given the paired page and nothing else. It reported the black rectangle as the most
prominent difference, and listed as its candidate causes: alpha dropped/composited on black,
premultiplied-vs-straight alpha, **an unsupported DrawingML transparency effect naming
`a:clrChange` explicitly**, a shape fill behind the picture, a CMYK decode failure, and an
unapplied mask. Its proposed discriminator — extract the embedded file and check whether it
already has a black matte — is the measurement that had already been run.

The measurements separate them: colour type 2 with no `tRNS` kills the three alpha candidates and
the mask; `<a:noFill/>` on the pic's `spPr` kills the shape fill; an 8-bit RGB PNG kills the
colour-space candidate. What remains is what the markup states.

## Two conditions that were measured rather than assumed

### 1. Equal colours are not a no-op

`fillproperties.cxx`:240 applies the transform when the colours differ **or** the destination
carries transparency. **All 93 corpus occurrences are `from == to`** with `<a:alpha val="0"/>`.
A reader that short-circuits on equal colours implements exactly nothing while looking correct.
Pinned by test, and the mutation that reintroduces it is detected.

### 2. The tolerance is format-dependent, and it is not the number the `.ppt` path uses

`fillproperties.cxx`:245-264 (tdf#149670) overrides the starting 9 from the stored format: **PNG
and TIFF 1, JPEG 15, BMP 0**, otherwise 9. That is a different number from the 9 the Escher call
site passes, and both are correct for their own call site.

**This is what made the census under-reach, exactly as the prediction said it would.** The
prediction wrote down that a tolerance-0 census is a *lower bound*. Eight documents it had
classified as **inert** moved, and all eight improved — every one a JPEG where `DF1F06` matches
**0.0% at tolerance 0 and ~49% at tolerance 15**:

`REDAC HF briefing Sep 2016_LARD_ATC_TO_FE_v2`, `ANG C-1 ATC-TO Core Program…`,
`redac-sas-201403-ppt-portfolio-rev-sim`, `BasicMed_AME_Presentation`,
`02_REDAC_New ATM briefing _March 2017v2`, `NAS-Infrastructure-Roadmaps-v16.0`,
`NAS-Infrastructure-Roadmaps-Weather`, `NAS-Infrastructure-Roadmaps-HSI`.

## The refutation the sweep produced: an alpha-bearing picture is not knocked out

The first post-change sweep showed **`vv_summit_SAIC-PRESENTATION_FAA-V&V-Summit_508c.pptx` and
its duplicate `vvsummit2022` going from an EXACT page 13 (0.00) to 0.28** — the only real
regression, and caused by the fix.

Its picture is an **RGBA** PNG, 66.1% `F4F4F4`, fully opaque, under a textbook knockout identical
in shape to the one that had just been fixed. Sampling page 13 at 100 dpi settled which side was
right: the reference keeps the grey at 25.2% of the page, our baseline kept it at 31.3%, and our
post-change render had none.

**Two authored one-shape decks, varying exactly one thing** — same pixels, same `clrChange`, same
geometry, saved once as RGB PNG and once as RGBA (`probe-alpha/make.py`), rendered through the
installed 26.2.4.2:

| stored format | reference renders |
|---|---|
| **RGB PNG** | colour **knocked out** |
| **RGBA PNG** | colour **untouched** |

The mechanism is `Graphic::colorChange`, `vcl/source/graphic/UnoGraphic.cxx`:188-208: it branches
on `aBitmap.HasAlpha()`, an alpha-bearing bitmap takes `ChangeColorAlpha`, and **only a bitmap
without alpha reaches the `CreateAlphaMask(aColorFrom, nTolerance)` branch that is the knockout.**

Implemented. Both documents returned to **0.71 / 0.46**, their exact baseline, while
`social-media-app` held 6.21 → 0.93 and `Technical_Report_Elements[1]` held 8.74 → 3.13.

This is prediction item 7 firing: I had written that the reference's behaviour was *inferred from
one document's code path, not measured individually*. It was, and it was wrong on a subset.

### `useA="0"` — implemented, reaches nothing, and said so

`ColorChangeContext::~ColorChangeContext` calls `maColorChangeTo.clearTransparence()` when `useA`
is false (`misccontexts.cxx`:266-270), which turns a knockout into nothing. Censused: **93 of 93
occurrences omit the attribute** and so default to true. It reaches zero documents today and is
pinned because the attribute is the whole difference between knocking a colour out and not.

## Slides: per-document movement, before → after

`abs_ink` 1409.36 → **1394.03**; signed 1040.62 → 1026.61; major pages 498 → 494.
**16 documents moved, 13 improved, 3 worsened.**

| Δ abs | document | abs before → after |
|---:|---|---|
| **−5.61** | `Technical_Report_Elements[1]` | 8.74 → 3.13 |
| **−5.28** | `social-media-app-bulletin-january` | 6.21 → 0.93 |
| −1.78 | `redac-nasops-201503-RIRP-portfolio-update` | 7.71 → 5.93 |
| −0.55 | `REDAC HF briefing Sep 2016_LARD_ATC_TO_FE_v2` | 1.71 → 1.16 |
| −0.49 | `bitesize-writing-a-report` | 4.15 → 3.66 |
| −0.42 | `ANG C-1 ATC-TO Core Program…` | 0.97 → 0.55 |
| −0.33 | `redac-sas-201403-ppt-portfolio-rev-sim` | 3.44 → 3.11 |
| −0.33 | `County ACHS Presentaion Webinar 8-16-16 Peg` | 11.80 → 11.47 |
| −0.32 | `BasicMed_AME_Presentation` | 1.04 → 0.72 |
| −0.16 | `02_REDAC_New ATM briefing _March 2017v2` | 1.98 → 1.82 |
| −0.07 | `NAS-Infrastructure-Roadmaps-v16.0` | 157.46 → 157.39 |
| −0.04 | `NAS-Infrastructure-Roadmaps-Weather` | 11.97 → 11.93 |
| −0.04 | `NAS-Infrastructure-Roadmaps-HSI` | 6.60 → 6.56 |
| +0.02 | `171128IPAP` | 13.87 → 13.89 |
| +0.03 | `16 - UTM - (NASA)` | 20.25 → 20.28 |
| +0.04 | `REDAC briefing March12-13-2014jemvbFINAL.ppt` | 3.82 → 3.86 |

The three residual regressions are all ≤ +0.04 and are anti-aliased fringe at knockout edges.
Recorded rather than buried.

**One document named in the prediction did not move at all:**
`FAAAIandtheArtandScienceofV&Vfinal` — which is on the *"we render better than the reference, do
not work it"* list, and which prediction item 4 covered (a knockout can be correct and invisible).
Its `clrChange` knocks white out of a picture drawn on white. No pixels, no risk.

## Shared layer: this diff touches `XRay.Office.Ooxml`

Named in the prediction before the change, and then **measured, not reasoned about**. Before and
after were produced from the same tree with the route switched off and back on, restoring with
`cp` + `touch` and rebuilding both ways.

| document | track | `diff%` b→a | `abs_ink` b→a |
|---|---|---|---|
| `system_design__technical_architecture_template` | words | 90.87 → 92.02 | 3.52 → **3.85** |
| `098_Business_Case_Template_Fillable_Layout` | words | 7.80 → **7.77** | 0.29 → 0.32 |
| `090_Business_Case_Template_Blue_Theme` | words | 13.43 → **13.42** | 3.74 → 3.80 |
| `096_Business_Case_Template_Editable_Layout` | words | 24.47 → **24.45** | 0.37 → 0.41 |
| `091_Business_Case_Template_Complete_Guide` | words | 30.17 → **30.15** | 2.24 → 2.26 |
| `100_Business_Case_Template_Modern_Format` | words | 17.00 → 17.00 | **unchanged** |
| `095_Business_Case_Template_Easy_Format` | words | 6.84 → 6.84 | **unchanged** |
| `094_Volunteer_Sign_Up_Sheet_Template_Editable` | sheets | 1.71 → **1.68** | 0.34 → **0.33** |
| `100_Volunteer_Sign_Up_Sheet_Template_Tabular` | sheets | 2.57 → **2.39** | 1.05 → **0.98** |

**The two unchanged documents are exactly the two RGBA ones** — the alpha condition above,
correctly declining to knock them out, independently confirmed on a different track.

### The ink metric moves the wrong way on a document the change demonstrably fixes

Worth recording as an instrument caveat, because the aggregate says the opposite of the pixels.

`system_design`'s `clrChange` lives in `header1.xml`, so it is on **every page**, and every one of
its 17 pages moved by about +0.02 `|ink|`. Sampling the colour the change actually controls,
`FDFDFD`, on page 1 at 100 dpi:

| | FDFDFD-ish, non-white |
|---|---:|
| ours, before | **3.374%** |
| ours, after | **0.048%** |
| **reference** | **0.040%** |

We were drawing a grey the reference does not draw, and now we do not. **The reference applies
`clrChange` in Writer too**, and after the change we match it. `pdf-image-diff`'s own `diff%` for
that page agrees — 5.63 → **5.59** — while its `ink%` goes 0.24 → 0.28 over an *identical* region
list.

The reason is region-local cancellation: our stray grey was numerically **offsetting a
pre-existing deficit of the dark blue** we already drew too little of in the same regions
(reference 1.98% against our 1.75%). Removing a wrong mark unmasked a different wrong mark, and
the ink figure rose while the page got closer.

**So a small ink rise is not by itself evidence of a regression, and on this change it is
repeatedly the opposite.** Rank by `|ink|`, decide by signed — and when they disagree with the
raw pixels, go and sample the colour the change controls.

The full cross-track sweep over all 337 words and 307 sheets documents is still owed and is the
parent's to run.

## Instrument defects, both fixed

### 1. `track-ink-sweep.sh` reported two measurements under one name

Its `INK` summary summed the **signed** column while `ink-ranking.py`, in the same skill,
headlines the **unsigned** one. `research/probes/slides-r19/ink-columns.py` recorded the problem
in round 19 and the live script was never fixed.

`ink.tsv` now carries `abs_ink` and `signed_ink` as separate named columns behind a header row,
and the summary prints both labelled plus checks `|signed| ≤ unsigned`. **Known-answer check
before use** (COMMON.md §5): on `slides/ceiling-002` it returns signed 100.58 and unsigned 110.97,
identical to the independent r19 script read off the same `cmp/`.

### 2. `look.py` materialised the spelling it globbed for

`resolve()` globbed both `{ext}` and `{ext.upper()}`. On this case-insensitive mount the
upper-case glob **creates** the second name in the directory cache permanently, which is how a
sweep total went 305 → 311 with the corpus unchanged. Replaced by filtering one case-sensitive
walk, which cannot create a name. Verified still resolving all four `.PPTX`-spelled documents.

This stops new ones. It cannot un-create the nine already materialised, so every total in this
file is reconciled case-folded:

```
Sylva%20introduction%20session, FAAAIandtheArtandScienceofV&Vfinal, Ramp Up Campaign - French,
012_3-Step_Hexagons_Puzzle_Diagram, 100_Lime_and_Lemon_Data-Driven_Chart, 030_Abstract_Petal_Diagram,
041_Crosswise_Quadrant_Diagram, 003_2-Circle_Venn_Diagram, 049_Five-Block_Hub_Spoke
```

### 3. A trap this round hit, worth writing down

**`sed -i` on this mount drops the executable bit.** The script was patched, `bash -n` passed, the
change was committed as mode `100644`, and the next run died with `Permission denied`. Every check
passed and the file was unrunnable. Restore the mode and confirm with `git ls-files -s`.

## Target 2 — `.ppt` autofit: two blind readings, and they are two different defects

Not implemented this round. Characterised so the next round starts from measurements.

Two more fresh reviewers, briefed with no numbers and forbidden source, docs and commands:

- **`2015-Civil-Rights-Website-training__ppt` p42** — ours ~9% **larger** on the identical string,
  **14 body lines to the reference's 12**, ours **overruns the frame** while the reference fits
  comfortably. Its top candidate, unprompted: *"autofit / shrink-on-overflow not applied… the fact
  that TOP visibly overruns the frame bottom while BOTTOM fits neatly is the single strongest
  hint."*
- **`ITE106-Chapter 4__ppt` p7** — ours ~10–20% **smaller**, **9 body lines to 10**, with
  conspicuously larger gaps between bullets.

Both reviewers independently, and without being asked to compare notes, separated **glyph size**
from **inter-paragraph spacing**: within a paragraph the line pitch tracks the font size, but the
side with the *smaller* type has the *larger* paragraph gaps. Both named it as a second effect
needing its own explanation.

### And a census that refutes the comment standing beside the code

`PptSlideLayout.Autofits` suppresses autofit when the shape grows to its text or when the text
does not wrap. The comment beside it records that the wrap half is an approximation which errs by
**not** shrinking a non-wrapping outline placeholder where the reference would, and asserts:

> *No deck in the slides corpus holds that combination … but it is the first place to look if one
> turns up.*

Censused over all 51 `.ppt` documents by parsing the Escher `msofbtOPT` property tables directly
(`ppt-wrap-census.py`, `WrapText`=133/`WrapNone`=2, `FitTextToShape`=191/`fFitShapeToText`=2):

| | |
|---|---:|
| `.ppt` documents | 51 |
| **with ≥1 non-wrapping, non-fit-to-shape text shape** | **36** |
| `2015-Civil-Rights-Website-training` | **14 such shapes** |
| `ITE106-Chapter 4` | **0** |

So the "no deck holds that combination" claim is wrong at the shape level, and the document a
blind reviewer independently found overflowing is one of the heaviest holders of it.

**Stated limit, because this census can also over-reach:** it counts every OPT table, and
`Autofits` additionally requires the text kind to be Body/HalfBody/QuarterBody. Tying each OPT
table to its `TextHeaderAtom` kind was not done, so **36 is an upper bound** on the affected
population. What it does establish is that the population is not zero.

`ITE106` holds none of it and moves the **opposite** way, so the two documents are **two defects,
not one**, and a single change aimed at "autofit" will not close both.

## Tests

`BlipColourChangeTests`, **25 tests**. Verified by **reintroduction** twice, both DETECTED:

| mutation | detected by |
|---|---|
| short-circuit the knockout when `from == to` (the plausible wrong reading) | 3 of 25 — `EqualColoursStillKnockOutWhenTheDestinationIsTransparent`, `AKnockoutResolvesToItsFromColour`, `APngKnockoutMatchesOneStepOffButNotTwo` |
| flatten the format-dependent tolerance back to the Escher 9 | `TheToleranceIsChosenByTheStoredFormat`, `APngKnockoutMatchesOneStepOffButNotTwo` |

The rest are drift guards pinning the cases that were already correct — the identity change, the
opaque recolour, the absent element, `useA` defaulting true, and the PNG colour-type/`tRNS` table.

Ten non-Fidelity projects, all green: Containers 109, Core 337, Markup 259, OpenDocument 125,
**Presentations 772**, Rendering 150 (+1 skipped, `PdfFontTests.ACffFlavouredFaceIsNotClaimedToBeTrueType`,
skipped at baseline too), Spreadsheets 886, Text 596, Vector 295, WordProcessing 1052 —
**4581 passed, 0 failed, 1 skipped**, against the briefed 4556: **+25, exactly the new tests.**

## Left open, in the order the next round should take them

1. **`.ppt` autofit, as two defects.** Start with `2015-Civil-Rights-Website-training` and the
   wrap approximation in `PptSlideLayout.Autofits`, whose own comment predicted this symptom and
   whose "no deck holds that combination" is refuted above. **Tie the OPT tables to their
   `TextHeaderAtom` kinds first** — that turns the upper bound of 36 into the real reach and is
   the measurement the fix should be sized from. `ITE106` is separate and moves the other way.
2. **Inter-paragraph spacing versus font scale**, which two blind reviewers separated from the
   size question on two unrelated documents. Unexplored, and it is the other half of HANDOVER §8's
   largest named front.
3. **Decide `pitchFamily`'s family nibble** — still a decision, not a patch. r50 measured it;
   nothing has changed.
4. **The general `a:clrChange` recolour** is deliberately not implemented: `ColourKnockout` carries
   only the fully-transparent corner, and the corpus states zero of the general case. If a
   document ever states one it is drawn as stored, exactly as before this round.
5. `038_Competitive_Advantage_Card` and `035_Chemistry_Column_PowerPoint_Chart` — still the only
   two genuine content differences in the old `text` pool, both in chart labels.

`MANIFEST.tsv` needs **no** change from this round: zero verdicts moved and no document changed
kind. It lives in the corpus repo and was not touched.

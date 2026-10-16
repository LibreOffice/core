# slides-r52 — prediction

Committed **before** anything was rendered post-change.

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`fc-match Calibri` → `Carlito-Regular.ttf`, base `a21e64f6d7e`, `SOURCE_DATE_EPOCH=1700000000`,
`TZ=UTC`, 32 cores, `/c/sandbox/workdir` 203 G free.

## Baseline, reproduced before anything was believed

Whole-track sweep, all 35 slides batches, reconciled against `MANIFEST.tsv` on a **case-folded**
identity: **302 of 302 agree, 0 disagreements — 199 of 302 passing.** The briefed figure.

The sweep's own `TOTAL` line says **311**; that is the case-insensitive mount (9 documents under
two spellings, one inode each), not a corpus change. `311 − 9 = 302`.

## What this round refutes before it changes anything — target 1 is empty

Round 51's first item was the wrap approximation in `PptSlideLayout.Autofits`, and its census
found **36 of 51 `.ppt` documents** holding a `wrapNone`-and-not-`fFitShapeToText` OPT table. It
said explicitly that 36 was an **upper bound** until each OPT table was tied to its
`TextHeaderAtom` kind. That has now been done (`ppt-autofit-census.py`), and the bound collapses:

| | |
|---|---:|
| `.ppt` documents | 51 |
| text-bearing shapes | 6989 |
| of Body/HalfBody/QuarterBody kind | 1402 |
| … `fFitShapeToText` (both suppress autofit) | 143 |
| … wrapping (both autofit) | 1234 |
| … **`wrapNone`, so WE suppress autofit** | **25, in 1 document** |
| … of those, carrying an `OEPlaceholderAtom` | **0** |

**Known-answer control** (COMMON § 5): the same walker, restricted to the r51 measurement — OPT
tables alone, whole stream — returns **36 documents**, r51's figure to the digit. So the walker
agrees with the independent instrument on the common quantity and the collapse is the Body-kind
restriction, not a different parser.

And the placeholder column is the second half. `svdfppt.cxx` reaches the wrap term only through
`dynamic_cast<SdrObjCustomShape*>(pRet) && eTextKind == Rectangle`, and line 846 sets
`pRet = nullptr` for **any** shape carrying an `OEPlaceholderAtom`. So `bAutoGrowWidth = !bWordWrap`
can only fire on a shape with no placeholder atom at all — which is all 25 of them. **On every one
of the 25, LibreOffice suppresses autofit exactly as we do.**

Target 1 as briefed reaches **zero documents**. The comment beside `Autofits` was right and r51's
refutation of it was measuring the wrong population.

## The change I intend to make, and it is target 2 arriving from underneath target 1

`SlideAutofit`'s own remarks say it is a port of **24.2.7.2**'s bisection and warn that 25.2
replaced the search with a walk down `constScaleLevels` — twelve discrete `(font, spacing)` rows,
format unscaled, take the **first** row that fits. The installed reference is **26.2.4.2**. The
comment has been standing there for rounds saying *"check which version wrote the reference before
porting anything out of this tree"*, and nobody has.

Measured, on **36 one-slide decks, one variable (box height), each its own file** so the
reference's shared-outliner state leak cannot contaminate it — a single 40 pt paragraph in a
360 pt-wide box, heights 60…480 pt, `/Tf` and baseline pitch read out of both content streams:

| box pt | reference | ours |
|---:|---|---|
| 480…336 | 40 / spacing 1.00 | 40 / 1.00 |
| 324, 312 | 40 / **0.90** | 38, 37 / 1.00 |
| 300, 288 | 37 / 0.90 | 35 / 0.90–1.00 |
| 276…228 | 34 / 0.90 | 35, 33, 31 / 0.90–1.00 |
| 216, 204 | 34 / **0.80** | 30 / 0.90–1.00 |
| 192, 180 | 31 / 0.80 | 29 / 1.00 |
| 168…144 | 28 / 0.80 | 28, 26, 24 / 1.00 |
| 132, 120 | 25 / 0.80 | 24, 25 / 0.80–0.90 |
| 108, 96 | 22 / 0.80 | 22, 20 / 1.00 |
| 84 | 19 / 0.80 | 19 / 0.90 |
| 72, 60 | 16 / 0.80 | 19, 17 / 0.80–1.00 |

The reference's nine distinct sizes are **exactly** `40 × {1.000, 0.925, 0.850, 0.775, 0.700,
0.625, 0.550, 0.475, 0.400}` and nothing else; the spacing that comes with them is 0.90 above
0.85 and 0.80 at and below it — **`constScaleLevels` row for row, in order, with both 0.85 rows
present**. Ours is a continuum: 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34,
35, 36, 37, 38, 39, 40 — essentially every whole point.

**The two documents this round was briefed to chase are the two ends of that one table.**
`2015-Civil-Rights` p42 (ours ~9% larger, overruns) is a box where our bisection settled higher
than the level the reference takes; `ITE106` p7 (ours ~10–20% smaller, conspicuously larger gaps
between bullets) is a box where the reference takes a row whose **spacing** is 0.80 and we land on
1.00. Height 168 in the grid above is that case exactly: **same 28 pt font, reference pitch 26.90,
ours 33.62.** A blind reviewer named it unprompted as *"font scale applied without the matching
spacing reduction"*; it is the level table's second column.

Three further conditions measured rather than assumed, all on authored decks:

1. **No slack.** 24.2's `aCurrentTextBoxSize.extendBy(0, -50)` is gone. Stepping the box 330…340 pt
   at 1 pt, the reference stops scaling at **exactly 336** and scales at 335. The 50 unit (1.417 pt)
   allowance would have put the boundary at 334.
2. **The stated scale is still thrown away.** `fontScale="62500"`, `lnSpcReduction="20000"`, and
   both together, in a box far too tall to shrink: **40 pt at full spacing in all three.**
   `SlideAutofit`'s existing note survives the version move.
3. **The rounding is `std::round` in the hundredth-of-a-millimetre domain, and the order of the
   arithmetic decides two cases.** At a stated 30 pt the reference draws 25 pt at level 0.850
   (25.5 rounds **down**) and 17 pt at level 0.550 (16.5 rounds **up**). Both fall out of
   `roundToNearestPt(roundToNearestPt(h_mm100) × level)` computed in doubles — 899.583…→25.49999996
   and 582.083…→16.50000000 — and neither falls out of multiplying in points.

## Documents I expect to change

Candidate census (`autofit-reach-census.py`), and it is a **candidate** count:

| | |
|---|---:|
| OOXML slides documents | 251 |
| … with ≥1 `a:normAutofit` on a **slide** part | 70 |
| … with `a:normAutofit` only in a layout/master | 120 |
| binary `.ppt` (every Body-kind text shape autofits, no markup asks) | 51 |
| **candidates** | **241 of 302** |

## Verdict movement I expect: **−2 to +4, most likely 0**

Stated as a range because this is not a colour change and I will not pretend it is.

The gate's three columns are page count, extractable words within max(2%, 3), and font embedding.
A slide deck's page count is its slide count, so **page counts cannot move** (0 of 302 predicted).
Font embedding cannot move. Words can, in both directions and only through one mechanism: text
that currently overflows its shape far enough to leave the **media box** is absent from the text
layer, and every body whose scale changes changes how much of it does that. We currently land
*larger* than the reference on more boxes than we land smaller, so the net should be words gained.

I am predicting **199 → 199** as the single most likely outcome, with real probability mass on
±1–2 either side. **If a passing document regresses I will name it rather than net it away.**

## What this census CANNOT see

Written down before the sweep, because an under-reaching census conceals itself.

1. **A candidate is not a mover.** Only a body that *overflows* is scaled at all, and only a body
   whose overflow lands on a different row than our bisection's answer changes a pixel. Nothing in
   the markup says which. The candidate count of 241 is an upper bound and I expect the measured
   mover count to be well under it.
2. **The 120 "layout/master only" documents.** A `normAutofit` in a layout reaches a slide only
   through a placeholder the slide actually instantiates and does not override. I resolved the
   part, not the instantiation.
3. **The `.ppt` side has no markup at all**, so its 51 documents are counted by *kind*, and a
   Body-kind shape whose text is short enough never reaches the search. The reach there is
   unknowable from the file.
4. **Line-count feedback.** Every level changes the wrap, so the search is not monotonic in the
   box height and a document can move to a *larger* font under the new rule. Two rows in the grid
   above do exactly that.
5. **The tests are calibrated to 24.2.7.2.** `SlideAutofitTests` says in its own remarks that
   every expectation is a measurement — of the superseded binary. I expect to have to re-measure
   `OneLineShrinksToTheSizeTheReferenceDraws`, `TwoLinesShrinkToTheSizeTheReferenceDraws`,
   `AWrappingBodyLandsOnTheReferencesSizeAndSpacing` and
   `TheFitsSpacingScaleReachesAParagraphsOwnSpace` against 26.2.4.2, and any of those I cannot
   re-measure I will say so about rather than adjust to fit.
6. **The reference's shared-outliner state leak is real and I am not reproducing it.** The same
   480 pt box answers spacing 1.00 as a solo slide and 0.90 as page 71 of a 71-slide deck. Every
   number above is from solo decks. Corpus decks are multi-slide, so the reference's own answer
   there is contaminated by the previous shape — which puts a floor on how well any renderer can
   agree with it, and it is a floor I cannot measure from here.
7. **`FitFloor` is unchanged and its measurement stands.** `AnOverflowingBodyStopsAtAQuarterAndOverflows`
   was already read off a banked **26.2.4.2** rendering of `NWD-GLA…`, and 0.250/0.800 is the
   table's last row, so the table walk subsumes it rather than contradicting it.

## Shared layers

The diff is expected to land in **`XRay.Office.Presentations` only** —
`Layout/SlideAutofit.cs`, possibly `Layout/SlideTextLayout.cs`, and the tests. Nothing in `Core`,
`Containers`, `Text`, `Vector`, `Rendering` or `Markup`, and nothing in `XRay.Office.Ooxml`. If that
changes I will say so and name the affected documents from a census, and the parent runs the
cross-track sweep.

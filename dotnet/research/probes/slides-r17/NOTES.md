# Slides, round seventeen — working notes

## The base

The worktree opened **437 commits behind** `claude/paperless-odf-phase-1-rnyzcu` — the seventh
agent in a row to open on a stale base. Fast-forwarded to `7e1b7c79e` before measuring anything.

## The autofit search is not the defect the brief names, and the reason is checkable

The round-sixteen brief hands this on as *"the autofit search prefers shrinking the spacing where
LibreOffice shrinks the font … `Solve` keeps the tightest fit, so a near-tie tips the wrong way"*,
and points at the search's **preference rule** as the thing to fix.

The preference rule is not ours to fix: it is already the reference's, transcribed statement for
statement. Fetched `svx/source/svdraw/svdotext.cxx` at tag `libreoffice-24.2.7.2` — the release
that made every reference PDF, and *not* the 27.2-alpha in this checkout, which has replaced the
function outright — and compared `autoFitTextForCompatibility` (lines 1277–1383) against
`SlideAutofit.Solve`:

| reference, 24.2.7.2 | `SlideAutofit.cs` | |
|---|---|---|
| `fScaleY = fMinY + (fMaxY - fMinY) * 0.5` | `candidate = minimum + ((maximum - minimum) / 2)` | same |
| `floor(fFontHeightPt * fScaleY/100 * 10)/10` | `Math.Floor(fontHeightPoints * candidate * 10.0) / 10.0` | same |
| `for (fCurrentSpacing : {100.0, 90.0, 80.0})` | `FitSpacings = [1.0, 0.9, 0.8]` | same order |
| `if (fCurrentFitFactor >= fFitFactorTarget) continue;` | `if (fit >= 1.0) continue;` | same early exit |
| `if (spacing==100) { fit > target ? fMinY : fMaxY = scale }` | identical | same |
| `(fBestFitFactor < target && fCurrent > fBest) \|\| (fCurrent >= target && fCurrent < fBest)` | identical | same tie-break, ties keep the earlier candidate |
| 10 iterations | `FitIterations = 10` | same |

So **both sides walk the same grid in the same order and keep the same candidate.** A
disagreement about which candidate wins therefore cannot come from the ordering — it can only
come from the *heights the two sides measure*, since the height is the search's only input.

### What that implies about `2015-Civil-Rights-Website-training.ppt` page 21

Traced by hand from the recorded figures (body states 20 pt at 80% spacing; the grid is the run's
own height, 706 mm100 = 20.0126 pt):

- The reference's answer, **19 pt at 100% spacing**, is 18 lines x 643 = 11574 mm100.
- Ours, **20 pt at 90% spacing**, is 19 lines x 610 = 11590 mm100.
- The search subtracts 50 mm100 of slack from each: 11524 and 11540.

Ours is the *taller* block, so ours is the **tighter** fit — and the tie-break keeps the tightest
fit at or above one. For the reference to have kept 19 pt at 100%, the 20 pt/90% candidate must
have **failed to fit for it**, i.e. its measured height there was greater than the box. The box
therefore lies in `[11524, 11540)`: a window of 16 mm100, 0.45 pt, **0.14% of the block**.

So the open question is a **measurement** difference of under half a point across nineteen lines,
not a preference between candidates. That is the inverse of the brief's framing in the same way
round sixteen's finding was the inverse of round fifteen's.

### And the measurement difference has a name

`size-census.py` compares the `/Tf` sizes the two renderings actually use, page by page. On this
deck 55 of 94 pages differ, and the commonest difference is not an autofit disagreement at all:

```
page    1  ours  20.000 x11   ref  20.010 x11   (-0.05%)
page    3  ours  24.000 x9    ref  24.010 x9    (-0.04%)
page    6  ours  28.000 x9    ref  28.010 x9    (-0.04%)
```

The reference draws a 20 pt run at **20.0126 pt**, because the character height lives in the
model's map unit and for a draw object that unit is a hundredth of a millimetre — 706 of them.
Every advance width it measures, every line break it takes and every height the autofit search
compares is taken at that size. We drew exactly 20 pt: **0.06% narrower on every run of every
deck.** That is the class of error that decides a 0.14% near-tie, and it is fixed in
`SlideAutofit.Quantised`.

## Batches 008, 010 and 012: the word gate is at its ceiling on all three

Every remaining word-gate failure in the three batches this round was pointed at is a document
where **we draw more text than the reference**, and in four of the five the excess is confined to
pages already on `TODO.raster-ceiling.md`. Measured per page with `page-words.py`:

| Document | pages carrying the excess | residue once they are removed | ref words | inside 2%? |
|---|---|---|---|---|
| `batch-010/…/W3_Case_Study…ppt` | 10 (+93) | 817 | 817 | exactly |
| `batch-010/…/Fundamentals_Module_1_basics.ppt` | 6 (+50) | 1096 | 1099 | yes |
| `batch-012/…/OnTrac_StarCertification…pptx` | 9 (+46), 10 (+251) | 1047 | 1045 | yes |
| `batch-012/…/NAS-Infrastructure-Roadmaps-v16.0.pptx` | the 24 `Requires="v"` pages | — | — | the named ceiling |
| `batch-008/…/8_P-Pavese…pptx` | 5 (+44), 6 (+44) | 2152 | 2108 | **no — see below** |

So 008, 010 and 012 cannot be advanced on the gate. Two corrections to the record fall out:

**`8_P-Pavese` page 6 is the raster ceiling and is not on the list.** `pdfimages -list` shows the
reference drawing the *same* 692x240 JPEG-plus-soft-mask on pages 5 and 6 and us drawing neither.
Page 6 is +44 on a base of 180 — **+24.4%**, just under the list's 25% threshold. So is page 16 of
the same deck at +23%. The threshold, not the class, is what kept them off.

**But `8_P-Pavese` page 16 is not the ceiling, and it is what keeps the document failing.**
`pdfimages` finds no raster there on either side, and the page's ink is 0.30 signed over 26
regions — a modest reflow, not missing content. What differs is that the reference draws the
category-axis labels of `chart5.xml` on one line — *lundi mardi mercredi jeudi vendredi samedi
dimanche* — and we break each of them into a narrow column. Our page 16 content stream has
**`Tm=0`**, so nothing on it is rotated; the labels are horizontal on both sides and only ours
wrap.

The axis states `<a:bodyPr rot="-60000000"/>`, a thousand degrees, and
`ObjectFormatter::convertTextRotation` discards anything outside ±5400000 —
`oox/source/drawingml/chart/objectformatter.cxx:1087-1091`, **byte-identical in 24.2.7.2** — so
the rotation reads as zero and line breaking is allowed on both sides. `DrawingChartPlot.AxisTextOf`
already implements that clamp correctly. The disagreement is therefore in **how much width the
axis gives each category slot** in `Paperless.Core/Charts`, which is a shared layer and was left
alone this round. Removing pages 5, 6 and 16 leaves 2106 against 2108.


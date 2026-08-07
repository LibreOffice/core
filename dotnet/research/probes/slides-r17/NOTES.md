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

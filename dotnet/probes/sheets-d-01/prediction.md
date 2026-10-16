# sheets-d-01 — the prediction, committed before measuring

Written after (a) reading `sheets-b-01`, `sheets-rebase-02`, `sheets-c-01` and the last four merge
notes, (b) reading `svx/source/dialog/framelinkarray.cxx`,
`svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx` and
`drawinglayer/source/primitive2d/borderlineprimitive2d.cxx` **in the 27.2.0.0.alpha0+ tree, which
is not the reference binary**, and (c) reproducing the brief's two headline counts on
`T0A0D0000090006XLSE.xls` page 3 with an authored stroke census.

**No authored probe has been rendered by `soffice` yet. No source has been modified.**

---

## 0. What is already reproduced, and is therefore not predicted

| brief's claim | measured here |
|---|---|
| reference 19 strokes, ours 103, page 3 of `T0A0D0000090006XLSE.xls` | **19 and 103**, exactly |
| our segments overlap at the joins by ~0.75 pt | **0.75 pt** — `53.455→108.942` then `108.192→159.540` |
| reference merges a whole grid line | its five verticals each run `142.865→771.448`, the full table height; its twelve horizontals each run `53.433→380.693`, the full width |

**Instrument check first, as the brief demands.** Both renderers write a cell border as
`m … l S` with the width in `w` — the `ml` idiom on both sides, same units, same colour operator.
Neither writes a border as `re`. A stroke census is therefore symmetric between the two and the
`Tm`-versus-`cm` trap of `sheets-c-01` has no analogue here. Fills are `re f` on both sides and are
counted separately.

One thing the brief does not mention and the census shows: **the reference emits two of its
nineteen strokes twice** — `V 380.324` and `H 143.234` each appear twice, identical. That is
`Array::CreateB2DPrimitiveRange` expanding its loop one cell beyond the range on every side
(`framelinkarray.cxx:1440-1446`), so the extra column's left edge duplicates the last column's
right edge. **A stroke count compared against the reference's must expect a duplicate at the far
edges**, and "19" is 17 distinct lines.

---

## 1. The merging rule, predicted

**P1.1 — what makes two edges identically styled.** Two collinear segments merge iff they share an
endpoint exactly *and* agree on: **line width**, **colour**, **dash pattern**, and the **number and
arrangement of sub-lines** (single against double, and each sub-line's own width and colour). Any
one of those differing splits the run in two. Predicted from
`tryMergeBorderLinePrimitive2D` — `getStrokeAttribute() ==`, then per-`BorderLine`
`getLineAttribute() ==` (which is width + colour + join + cap) and `isGap() ==`.

**P1.2 — a style change mid-run gives exactly two strokes**, one per side, not three and not a
blend.

**P1.3 — a gap mid-run gives two strokes and they do not bridge the gap.** The merge test is
endpoint coincidence; a missing cell edge leaves no segment to coincide with.

**P1.4 — merging is over the whole emitted set, not per row or per column.** The merge loop scans
*every* primitive already accumulated and tries both orders (A.end==B.start and B.end==A.start),
so two touching collinear segments merge no matter which cells produced them. Consequence I expect
to be able to show: **two visually separate tables that happen to share a grid line and touch will
merge into one stroke.**

**P1.5 — the run is maximal along a grid line, and rectangles do not merge.** A cross-product test
means a horizontal never merges with a vertical, so a bordered block yields one stroke per grid
line and never a closed rectangle. This is the half of the standing comment in
`SheetPageDecoration.DrawBorders` that is right.

## 2. The extension rule, predicted — and this is the part I most expect to be wrong

The current code extends **every** end of **every** cell edge by half the width of the perpendicular
border it crosses, which is what produces the 0.75 pt overlap. I predict the real rule is:

**P2.1 — an end that has a *used collinear continuation* gets zero extension; only an end with no
collinear continuation is extended by half the crossing width.**

`HelperCreateHorizontalEntry` adds the collinear left/right neighbour's own top border to the
connect-style vector (`rStartLFromL = GetCellStyleTop(col-1, row)`, perpendicular `-rX`), and
`getExtends` takes the **minimum** cut set over every connected style. `findCut` on two parallel
lines fails and leaves the `CutSet` at its all-zero default, so a used collinear neighbour pins the
extension at zero — by the strict-minimum branch or by the "equal centre point, use medium cut"
branch, both of which return zero against `{-h,+h,-h,+h}`.

**P2.2 — therefore LibreOffice does not double-ink at a style break either.** Where a run *breaks*
because red meets blue, both segments still have a used collinear neighbour, so both extensions are
zero and the two abut exactly: **no overlap and no gap at an interior joint, merged or not.**

**P2.3 — the merged stroke's outer ends carry the crossing extension**, half the minimum crossing
width, which is what the current code already computes and what the page-3 numbers show
(`53.802 - 0.375 = 53.427 ≈ 53.433`).

**P2.4 — the overlap, not the segment count, is the visible defect**, so P2.1 is worth more ink
than P1.1. A run of *n* cells merged into one stroke puts down exactly the same ink as *n* abutting
butt-capped segments; it is the extension at every interior joint that doubles the ink. If P2.1
holds and P1.1 were skipped entirely, the raster would already be right and only the stroke count
would differ.

**Why I expect to be wrong somewhere here.** The C++ read is of 27.2.0.0.alpha0+ and two
predictions died to exactly that gap this session. `getExtends` has a second branch for
multi-line (double) borders that I have not traced, and `IsUsed()` on a collinear neighbour of a
*different* style may not behave as I read it.

## 3. Reach across the 171

**P3.1 — 120 to 165 renderings change bytes, point estimate 150.** A shared, identically-styled
cell edge is the commonest thing in a spreadsheet; almost any workbook with a bordered table has
one. Documents with no cell borders at all are the only ones that cannot move.

**P3.2 — 0 of 200 words and 0 of 163 slides change.** The seat is
`XRay.Office.Spreadsheets.Layout.SheetPageDecoration`, which neither track reaches. If either moves,
the change was not confined to the seat and the round is wrong.

**P3.3 — verdict movement is zero, and I say so plainly.** Page count, a 2%-and-3-absolute word
band and unembedded fonts cannot see a stroke count, a stroke length, or 0.75 pt of doubled ink on
a hairline. Sheets stays **144 of 171**. The one way this could be wrong is the way
`sheets-c-01`'s zero was wrong — if merging changed something that is *also* text — and there is no
such coupling here, because no glyph is emitted by `DrawBorders`.

## 4. Direction

**P4.1 — every page whose vector changes moves toward the reference in stroke count**, trivially,
because merging can only reduce a count that is currently too high.

**P4.2 — pages carrying hairline grids get measurably closer in pixels; pages carrying thick
borders barely move.** The doubled ink is at most one border-width square per joint; on a 0.75 pt
hairline rasterised at 512 px on A4 (1 px ≈ 1.6 pt) that is a *sub-pixel* effect showing up as
antialiasing, which is exactly what `sheets-rebase-02` §2 measured as the residual 1.35% on
`7-memento…` page 2.

**P4.3 — I predict 0 to 5 pages get *further* from the reference**, and that any that do are
documents where an extension we currently draw was accidentally covering a defect elsewhere —
the `EHEST-SMS` shape from `words-e-01`. **Anything further is a finding and gets its own section.**

**P4.4 — the pixel instrument will under-report.** `pdf-image-diff.py`'s 512 px raster puts one
pixel at ~1.6 pt on A4, and the whole effect is 0.75 pt wide. The primary instrument is therefore
**vector**: strokes per page and total inked line length as a union of intervals, so double-inked
overlap is counted once. Pixels are the secondary check. A round that quoted only `diff%` here
would report a working fix as noise — the `words-d-01` media-box shape.

## 5. What my census cannot see, named in advance

* **Fills.** Our fill rectangles are per-cell where the reference's are per-run
  (`sheets-b-01` §8, 159 against 52 on `grants-2005` page 79). Adjacent fills **abut**, they do not
  overlap, so unlike borders they put down identical ink — total area already matches to 0.3%.
  I am deliberately not merging fills, and if a page gets further in pixels this is the first place
  to look for why.
* **Dash phase.** A merged dashed run restarts its pattern once; four per-cell segments restart it
  four times. If LibreOffice merges dashed borders at all, the phase is a second difference my
  count-based census will not show, and I may not be able to reproduce it exactly.
* **Band boundaries.** A run cannot cross a printed page's column or row band, because Calc draws
  each band through its own `ScOutputData`. I cannot see from a PDF whether a run that *stops* at a
  band edge stopped for that reason or for a style reason.
* **The reference's duplicate strokes** (§0). If I reproduce the reference's *count* exactly on
  some page it will be by not emitting those duplicates and being wrong by two in the other
  direction on the same page. I will report distinct lines, not raw counts, wherever the two differ.
* **Merged cells.** `SheetMerges` already suppresses interior edges; whether a block's left edge
  emitted per-row then merged gives the same result as one edge is something the census can show
  but the gate cannot.
* **Anything the reference draws that we do not draw at all.** Coalescing cannot create a stroke.
  A page where the reference has more *distinct lines* than we do after this change has a second,
  unrelated defect.

## 6. The claim in our own source that I expect to refute

`SheetPageDecoration.DrawBorders`' remarks say, of the reference:

> Calc does not [merge runs], and measuring LibreOffice's own PDF of `sheet-decor-ods.ods`
> confirms it — B4's box arrives as four separate `m … l S` pairs rather than one closed path.

**The measurement is real and the conclusion does not follow.** A cell's four edges run in four
directions; `tryMergeBorderLinePrimitive2D`'s cross-product test refuses them by construction. That
observation cannot distinguish "Calc does not merge" from "Calc merges collinear runs only", and
the page-3 census says it is the second. The same paragraph then states that three abutting
segments "put down the same ink, so they are left as segments here" — which is true of *abutting*
segments and false of the *extended* ones the code actually emits.

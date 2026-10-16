# Sheets round thirty-seven — a merged range's decoration

Base `545550952`, verified with `git log --oneline -1` before anything was measured.

## What the probes settle

`merge-decor.fods` and `merge-grid.fods` are the same five-by-four sheet: a 2 × 3 merged
block at B2:C4 whose **origin** states a red fill and a 2 pt blue box, whose six
**covered** cells state a green fill and a 1 pt magenta box, and whose surrounding cells
state a yellow fill and a 0.5 pt black box. `merge-grid.fods` adds `style:print="grid"`.

Rendered by the installed `soffice` (24.2.7.2) and read with `pdf-ops.py dump`, three
things follow, and none of them is in `SheetPageDecoration`:

1. **The fill is the origin's and it covers the whole range.** One `#FF0000` rectangle
   from (113.27, 717.36) to (226.69, 768.33) — two columns by three rows. `#00FF00`
   appears **nowhere**: a covered cell's own fill is never painted.
2. **The border is the origin's and the interior edges are suppressed.** Four `#0000FF`
   lines, one per side of the whole block. `#FF00FF` appears nowhere. The interior
   vertical at x = 170.05 is drawn only above and below the block; the interior
   horizontals at y = 751.26 and 734.29 only to its left and right.
3. **The printed grid is suppressed inside the range too.** With `style:print="grid"` the
   hairline at x = 169.97 runs 768.33–785.20 and 700.41–717.28 and stops at the block;
   the hairlines at y = 751.35 and 734.37 run 56.69–113.27 and 226.77–283.35 only.

The mechanism, in the source: `Array::GetCellStyleTop`/`Left`/`Right`/`Bottom` take their
style from `ArrayImpl::GetMergedStyleSourceCell` — the merge's top-left cell — and return
`OBJ_STYLE_NONE` when `IsMergedOverlapped*` (`svx/source/dialog/framelinkarray.cxx:460-490,
782-856`); `ScOutputData::DrawBackground` extends a run across `ATTR_MERGE`'s column count
(`sc/source/ui/view/output.cxx:1155-1170`); `ScOutputData::DrawGrid` skips a line where the
neighbouring cell `bHOverlapped`/`bVOverlapped` (`output.cxx`, the `bHOver` branch).

The reference draws each side of the block as **one** primitive rather than one per covered
cell, which is `tryMergeBorderLinePrimitive2D` merging collinear lines of equal
`LineAttribute`. Segments of equal colour and width abutting with butt caps put the same ink
down, so this round does not reproduce the merging.

## The census, and what it counted over

`census.py` and `census2.py` read `xl/worksheets/*.xml` and `xl/styles.xml` directly.
**They can read 110 of the track's 171 documents**; the other 61 are `.xls`, which states a
merge in a `MERGEDCELLS` record no zip-level census can see. Over the 110:

| | documents |
|---|---:|
| state a `mergeCell` | 56 |
| print the grid (`printOptions/@gridLines`) | 8 |
| both | 5 |
| a merge whose covered cell states a *different* style index | 43 |
| a merge whose covered cell's style paints a border or a fill | **47** |

7818 merges in total, 2592 of them with a covered cell stating a different style. Read the
47 as a **ceiling over two thirds of the track**, not as reach: a covered cell whose border
happens to equal the origin's puts down the same ink either way. Reach is measured by
rendering.

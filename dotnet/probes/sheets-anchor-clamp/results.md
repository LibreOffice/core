# An XLSX two-cell anchor: whose 0.14 pt is it?

Round `agent/draw-shapes`, base `6bf527227`. Environment: this container, `/usr/bin/soffice` =
**24.2.7.2**, `/opt/libreoffice26.2/program/soffice` = **26.2.4.2** with the Latin Noto and the
metric-compatible duplicates already moved aside. Every figure below was taken here, against both
binaries, and every reference number is read out of the reference's own PDF content stream
(`readimg.py` walks `q`/`Q`/`cm`/`Do` and reports the image XObject's placed rectangle).

## The question

`SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt` fails on
`sheet-rich-text.xlsx`. The rectangles, in points:

| | width | height |
|---|---:|---:|
| `sheet-rich-text.fods`, 24.2.7.2 **and** 26.2.4.2 | 95.046 | 46.800 |
| `sheet-rich-text.xlsx`, 24.2.7.2 | 95.017 | 46.772 |
| `sheet-rich-text.xlsx`, 26.2.4.2 | **94.904** | **46.658** |
| ours, both spellings | 95.074 | 46.800 |

Top-left identical on all five (62.447, 761.470). Only the far edges move, and they move on
**both** axes by the same 0.113 pt.

The previous round's note on the test blamed the derived grid — "the span is a sum of grid extents
and the grid's own units are character widths and font-derived row heights". **That is refuted
here.** The picture's height is `rowOff` 640080 EMU less `rowOff` 45720 EMU *within one row*, which
is 46.800 pt of pure EMU arithmetic with no grid and no font in it at all, and it moves by the same
amount as the width. And both binaries compute the *same* grid: exported to flat ODF by each, the
sheet's columns are `0.7in` and its rows `0.2799in`/`0.7in` in both.

## What each binary actually does

Ten probes on the corpus document with only `xdr:to/@colOff` and `@rowOff` changed
(`make-offset-probes.py`), then eleven more at 500-EMU steps through the interesting region and six
on a wider column (`make-fine-probes.py`), then six more varying the cell size
(`make-cellsize-probes.py`). Cell B is 1778 (1/100 mm) wide in the corpus document. Offsets are the
drawn in-cell offset, in 1/100 mm, recovered from the PDF rectangle:

| `xdr:to/@colOff` (EMU) | raw ÷ 360 | 24.2.7.2 | 26.2.4.2 |
|---:|---:|---:|---:|
| 100 000 | 277.78 | 277 | 277 |
| 320 040 | 889.00 | 888 | 888 |
| 630 000 | 1750.00 | 1749 | 1749 |
| 635 000 | 1763.89 | 1763 | 1763 |
| 638 500 | 1773.61 | 1773 | 1773 |
| 639 000 | 1775.00 | 1774 | **1773** |
| 639 445 | 1776.24 | 1775 | **1773** |
| 640 080 | 1778.00 | 1777 | **1773** |
| 700 000 | 1944.44 | 1943 | **1773** |
| 900 000 | 2500.00 | 2499 | **1773** |

**24.2.7.2 does not clamp at all.** Its drawn offset is `round(EMU / 360) − 1` for every one of the
seventeen probes, including offsets 1.4× the cell's own width. The unconditional `− 1` is a
`tools::Rectangle` inclusivity artefact and is 0.028 pt, inside every tolerance in this project.

**26.2.4.2 clamps the offset to `cellSize − 5` (1/100 mm)** — 0.14 pt inside the cell's far edge —
and does it on both axes. Verified against eight different cell sizes, measured from each binary's
own flat-ODF round trip rather than inferred:

| cell (1/100 mm) | drawn cap | cell − cap |
|---:|---:|---:|
| 979 | 974 | 5 |
| 1371 | 1365 | 6 |
| 1778 | 1773 | 5 |
| 2154 | 2148 | 6 |
| 2545 | 2541 | 4 |
| 2937 | 2932 | 5 |
| 3328 | 3324 | 4 |
| 3916 | 3911 | 5 |

The 4/5/6 spread is not noise in the cap; it is the difference between a column's *own* width and
the difference of two cumulative positions, each of which is `(twips × 127 + 36) / 72` truncated.
Taking `cellSize = position(col+1) − position(col)` in 1/100 mm makes the cap exactly
`cellSize − 5` on all eight, and the model then reproduces every one of the 34 measured rectangles.

## Whose defect it is

**LibreOffice's, and 26.2.4.2's rather than the format's.** Four independent reasons:

1. **It fires on a valid anchor.** `xdr:to/@colOff` equal to the cell's own extent is how any
   picture snapped to a column edge is written; the corpus document's own
   `a:ext cx="1207080" cy="594360"` states 95.046 × 46.800 pt, which is exactly what the unclamped
   arithmetic gives and exactly what the clamp destroys.
2. **The same binary disagrees with itself.** 26.2.4.2 draws the *same picture* at
   95.046 × 46.800 from the ODF spelling and 94.904 × 46.658 from the XLSX one.
3. **It disagrees with 24.2.7.2**, which applies no clamp anywhere in the probed range.
4. **It disagrees with the source in this tree.** `ShapeAnchor::calcCellAnchorEmu`
   (`sc/source/filter/oox/drawingbase.cxx`) clamps to `getCellPosition(col+1) − 1 twip`, i.e.
   635 EMU, with the comment *"reduce cell's right edge by a full twip"*. That constant produces a
   drawn cap of `cellSize − 3` and a width of 3351 (1/100 mm) on the corpus document — matching
   neither binary against the 3348 measured from 26.2.4.2 and the 3352 from 24.2.7.2. So the
   magnitude is not a documented rule; it is an emergent one, and it has already moved twice.

So the tree is **not** contorted to reproduce it. We draw the anchor arithmetic the file states,
which is what 24.2.7.2 draws to within 0.057 pt, what both binaries draw from the ODF spelling to
within 0.028 pt, and what the picture's own `a:ext` says.

## Reach of not reproducing it

0.113 pt on a far edge, on any XLSX drawing whose `to` offset reaches the last ~5/100 mm of its end
cell. It moves no gate column — the gate is page count, characters and font embedding — and it is
below the 0.1 pt tolerance of every fidelity comparison except this one, which asserts at exactly
0.1 pt and misses by 0.070 pt on width and 0.042 pt on height.

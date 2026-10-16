# A category label that breaks at a single blank was thrown away

Round `agent/chart`, 2026-09-05. Both reference binaries, on this container:
`soffice` 24.2.7.2 and `/opt/libreoffice26.2/program/soffice` 26.2.4.2.

## The question

`ChartAxisLabels.Wrap` breaks a category label at its blanks so it fits one tick's worth of
axis — 0.95 of the tick spacing, `VCartesianAxis.cxx:753-759`. It then asked whether anything
had changed by comparing the finished string's **length** against the original's:

```csharp
if (built.Length == text.Length) continue;   // discard the wrap
```

A break replaces exactly one blank with exactly one newline, so `ACCOUNT MANAGER` set on two
lines is the same fifteen characters it was on one. **Every label that breaks at a single
space — which is what a two-word category name is — was discarded**, the collision test then
measured the unwrapped run, found an overlap that the reference does not have, and thinned the
axis to every second label.

The second half is that a wrapped arrangement is invisible to `IsPlain`: it is upright, one
row, every label drawn, and nonetheless two lines deep. With the wrap restored and the
reservation left at one line, the second line was drawn through the chart's own bottom edge.

## The probe

`make-wrap-probe.py` builds 21 decks from `tests/corpus/features/chart-face-theme-minor.pptx`,
eight categories each, in three arms at seven word lengths:

* **A** two words of *n* characters, so the label is over the limit and each word is under it;
* **B** one word of 2*n* characters — the same total width with no break opportunity at all;
* **C** one word of *n* characters, which fits and must not move.

`run-wrap.sh` renders each through both references and through our CLI; `score-wrap.py` reads
the PDF text layer and reports how many of the eight distinct `Kat<i>` labels were drawn and on
how many baselines. `0/8 r1` is the rotated case: LibreOffice emits one `Tj` per glyph for
rotated text, so no whole token survives — the picture confirms all eight are drawn at 45°.

## Result

```
deck                  ref24        ref26       before        after
wrapA-n04            8/8 r2       8/8 r2       4/8 r1       8/8 r2
wrapA-n06            8/8 r2       8/8 r2       4/8 r1       8/8 r2
wrapA-n08            8/8 r2       8/8 r2       4/8 r1       8/8 r2
wrapA-n10            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapA-n12            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapA-n14            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapA-n16            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapB-n04            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapB-n06            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapB-n08            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapB-n10            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapB-n12            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapB-n14            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapB-n16            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapC-n04            8/8 r1       8/8 r1       8/8 r1       8/8 r1
wrapC-n06            8/8 r1       8/8 r1       8/8 r1       8/8 r1
wrapC-n08            8/8 r1       8/8 r1       8/8 r1       8/8 r1
wrapC-n10            8/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapC-n12            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapC-n14            0/8 r1       0/8 r1       0/8 r1       0/8 r1
wrapC-n16            0/8 r1       0/8 r1       0/8 r1       0/8 r1
```

* **The three decks that wrap are the three that moved**: `4/8` on one line before, `8/8` on
  two lines after, which is what both references draw. Nothing else in the table changed.
* **Arm B is the control that the rotation branch was not touched.** A single word wider than
  0.95 of a tick turns line breaking off and the axis 45°, at every length, before and after.
* **Arm C is the control that an axis whose labels fit does not move**, and it also carries a
  version gap: at `n=10` 24.2.7.2 leaves the axis upright and 26.2.4.2 turns it. We match
  26.2.4.2, which is the build the tree is calibrated to.

## Corpus effect

`033_Event_planning_tracker_Use_this_template_f29a848e.xlsx`, whose two charts share six
two-word category names, is the witness the code comment already named. Page-1 mean absolute
grey difference at 30 dpi:

| | against 24.2.7.2 | against 26.2.4.2 |
|---|---:|---:|
| before | 11.31 | 12.52 |
| after wrap restored, one line reserved | 11.46 | 12.67 |
| after the reservation as well | **11.04** | **12.40** |

The middle row is worth keeping: restoring the wrap **on its own** made the page worse, because
the second line was then drawn outside the chart. The ink gain is small either way — six labels
that were not drawn are six labels' worth of ink — and the visible defect is closed: all six
category names are drawn on two lines each, inside the frame, exactly as the reference draws
them.

## Method notes

The "before" leg was built by copying the two changed sources aside, `git checkout`-ing them,
`touch`-ing, and deleting `src/XRay.Office.Core/{obj,bin}`; restored the same way and verified by
re-rendering `wrapA-n06` and byte-comparing it against the "after" leg's own copy. It is equal.

# r185 — the words track after the VML preset-geometry fix

```
TOTAL 337  MATCH 329  REF-CANNOT-RENDER 0
ABS-INK 431.83 (unsigned |ink|%, a track total)  SIGNED-INK 312.11  MAJOR PAGES 170  over 329
WORST PAGE 6.42 on words/done-014/doc/PK_FlugzeugeStricken.doc
```

Against r184's `434.55 / 314.83 / 170`, gate unmoved at 329 `match`. **One row of 337 differs**:
`090_Business_Case_Template_Blue_Theme` 4.25 → **1.53** on its only page.

## Why only one, when the census said 27 documents

The census counted **202 inked `#_x0000_t202` uses across 27 documents**, and none of them moved.
They are almost all `mc:Fallback` VML beside a `mc:Choice` holding the same shape as DrawingML —
which is the half this reader takes — so they are never read at all. The reach of a VML geometry
rule is the shapes in a bare `w:pict`, not every `v:shape` in the package, and a census that does
not separate the two overstates it by an order of magnitude.

## What is left on 090

1.53, and it is not the banner. Two things a blind reading of the composed page caught and the
region list agrees with: everything from the first heading down sits about 8 px (at 120 dpi) lower
in ours than in the reference, and both tables sit 7–10 px further right and a few px taller. The
banner itself — position, size, fill and the white heading inside it — is right.

## The head, ranked on the worst page

| worst | mean | sum | pages | major | document (`drift` 0) |
|---:|---:|---:|---:|---:|---|
| 6.42 | 1.02 | 7.13 | 7 | 2 | `PK_FlugzeugeStricken.doc` — spent, `TODO.word-parity.md` |
| 5.18 | 5.18 | 5.18 | 1 | 1 | `021_Unit_Circle_Chart_3D_Pie_Chart.docx` |
| 4.64 | 4.64 | 4.64 | 1 | 1 | `016_Project_Timeline_Template_Complete_Guide.docx` |
| 3.68 | 0.51 | 4.04 | 8 | 1 | `1_tpr_template__from_fy14_.docx` |
| 3.58 | 3.58 | 3.58 | 1 | 1 | `028_Unit_Circle_Chart_Optimized_Graph.docx` |
| 2.77 | 0.23 | 10.22 | 44 | 2 | `docs-quality-MA.IMS.00001-…-manual.docx` — spent, `probes/footerlink-r183` |
| 2.43 | 0.06 | 7.00 | 118 | 3 | `A_320.doc` |
| 2.07 | 0.69 | 3.45 | 5 | 1 | `644730BRI0mna000BOX361539B00public0.doc` |
| 2.04 | 0.86 | 2.57 | 3 | 2 | `Case-Study-Heathrow-Airport.docx` |
| 1.53 | 1.53 | 1.53 | 1 | 1 | `090_Business_Case_Template_Blue_Theme.docx` |

## What the four blind readings of the chart cluster established

All four were read blind, given no numbers, one page each.

* **`021`, 5.18** — the reference draws the `c:pie3DChart` as a **1370 × 732 JPEG**: a true extruded
  pie, an elliptical top face 790 px wide filling the plot area with shaded side walls. We draw a
  flat 2-D circle 455 px across, height-constrained rather than width-filling, in four flat fills.
  **Not a chart-type gap** — `pie3DChart` is recognised, and the corpus's two other `pie3DChart`
  documents score 0.30 and 0.04. It is the 3-D projection: the reference's own
  `pie-chart-result.docx` is rasterised the same way and matches, because its chart is small.
* **`028`, 3.58** — `c:ofPieChart`. Our pie is ~1.3× the reference's diameter, our bar-of-pie is
  1.7× wider, the two bar segments are **stacked in the opposite order**, our legend shows 16
  entries against the reference's 12, and the reference fills the plot area with a light hatch we
  leave white. The corpus's other `ofPieChart` scores 0.55, so again not the kind.
* **`016`, 4.64** — the *reference* draws less: its title banner collapses to a 20 px strip at the
  page edge with no `Project Timeline` text, and its `Starting Date:` label is missing, while ours
  draws both. Needs the Word arm measured before it is called ours.
* **`090`, 4.25 → 1.53** — fixed this round.

So of the four, one is fixed, one is a 3-D projection we do not implement, one is a chart-layout
cluster inside `ofPie`, and one may be the reference's.

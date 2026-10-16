# A chart's bold text was drawn light on the words track — r188

Measured 2026-09-28 against **LibreOffice 26.2.4.2** (`/opt/libreoffice26.2/program/soffice`,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`), `SOURCE_DATE_EPOCH=0`.

## 0. How it was found, and the reading that was wrong first

Closing the 3-D pie left `021_Unit_Circle_Chart_3D_Pie_Chart`'s labels visibly lighter than the
reference's. The first reading was that the weight is stated only on each **per-point**
`c:dLbl`, which `DrawingChartPlot` does not read — taken from the first `c:dLbls` in document
order, which is the per-point one nested inside the series-level one.

**`census.py` refuted it.** Over 699 corpus chart parts, a per-point `c:dLbl` states a size,
weight or colour *beyond what its enclosing `c:dLbls` already states* in **4 documents**, and
`021` is not one of them: its series-level `c:dLbls` carries the same
`<a:defRPr sz="1400" b="1">` after its four `c:dLbl` children, so the per-point statements are
repeats. **Count the override against what it inherits, not on its own** — an override that
repeats its parent cannot be seen on the page, and a census that misses that subtraction sends
a round after a reader gap that is not there.

## 1. Where the weight was actually lost

Not in the reader. `DrawingChartPlot.Read` on that part answers
`IsDataLabelBold=True size=14pt`, and the drawn size is right: 14.00 against the reference's
14.01. What `pdffonts` says is the whole of it:

| | face set of the page |
|---|---|
| ours, before | `Carlito-Regular` `DejaVuSans` `DejaVuSerif` |
| 26.2.4.2 | **`Carlito-Bold`** `DejaVuSans` `DejaVuSerif` |

**`FrameChart.ChartFace` resolved one face and shaped every label through it.** Its own remarks
named the gap and deferred it:

> *"`bold` is the slides track's `ChartPlot.IsTitleBold` reaching a consumer that cannot yet act
> on it. A `ChartFace` resolves one face and shapes every label through it, so drawing a title
> bold means resolving a second face here and threading it through `Shape` — a change that moves
> every DOCX whose chart has a title, on a words sweep this round did not run."*

So it is the words track alone: `SlideChart` passes `label.IsBold == true` to its measurer and
`SheetChart` reads `label.IsBold ?? false`, and both have since they were written. A chart's
bold **title, axis labels, axis titles, legend and data labels** were all light on a DOCX, a DOC
and an RTF.

## 2. What was implemented

`ChartFace` resolves the family at weight 700 beside weight 400 and keeps the pair; `Shape`,
`Measure`, `LineHeightAt` and `AscentAt` all take the weight; `FrameChart.Text` and
`FrameChart.Lines` pass `label.IsBold == true`.

**A family with one weight collapses back to one face.** The resolver answers the same file for
both requests, and its `FaceKey` is that file's path — which is what the PDF writer embeds by —
so pairing such a face with itself would put a second identical subset in every document that
uses one. `ChartFace.Same` is that test.

The vertical metrics take the weight too, because a label is drawn at
`blockCentre − blockHeight/2 + ascent` and the two have to move together or a bold label sits
off its own baseline. Carlito's two weights share their vertical metrics, so on this corpus the
change is horizontal only; the test asserts the pair is consistent rather than that it differs.

## 3. Reach and cost

**5 of the corpus's 10 chart-bearing words documents state a bold anywhere in a chart part, and
exactly those 5 move.** Our half of all 10, plus the 10 converted `.odt` that hold a
`chart:chart`, rendered at the round's base and again with the change:

| document | `b="1"` in a chart part | moved | worst `diff%` before → after |
|---|---:|---|---|
| `021_Unit_Circle_Chart_3D_Pie_Chart` | 5 | yes | 5.64 → 5.68 |
| `028_Unit_Circle_Chart_Optimized_Graph` | 1 | yes | 16.47 → 16.48 |
| `pie-chart-result` | 1 | yes | **1.66 → 1.55** |
| `pie-chart-template` | 1 | yes | **0.21 → 0.11** |
| `ABCD-FE-01-00 Flight Envelope` | 17 | yes | not scoreable — 15 pages against 16 |
| the other 5 words charts | 0 | **no** | — |
| all 10 converted `.odt` charts | 0 | **no** | — |

**The `diff%` column is the wrong one to read here and the face set is the right one.** On
`021` it goes 5.64 → 5.68 and on `028` 16.47 → 16.48, because bold labels are wider and the
extra width moves where a label wraps — trading one error for a smaller one of a different
kind. What is unambiguous is that `021`'s page now embeds `Carlito-Bold` where 26.2.4.2 embeds
`Carlito-Bold`, and embedded `Carlito-Regular` before; `028` and `pie-chart-result` already
carried both faces for their body text, so their face sets do not move and only their pixels do.

Nothing on the slides or sheets tracks can move: `ChartFace` has one consumer and it is
`FrameChart`.

## Files

| file | what it is |
|---|---|
| `census.py` | per-point `c:dLbl` overrides *beyond what the series level states* |
| `score.sh` | scores a before/after pair of ours-only renders against 26.2.4.2 |

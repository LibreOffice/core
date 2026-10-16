# Round 53 — sheets — third prediction: `SheetChart` fuses a two-line data label

Committed before the sweep that measures it. Same process note as the addendum: two documents
(`003_advanced_excel_pie`, `005_Contextures_chart_sample`) were spot-rendered first, so their
figures below are **measurements**; the prediction proper is the rest of the track and the
cross-track reach.

## The defect

The words track fixed this in `FrameChart` in round 52 and left `SheetChart` carrying it
deliberately. A data label that shows a percentage beside a category or a series name is written on
two lines — Office's own separator, which `ChartDataLabel.Separator` already defaults to `"\n"` —
and shaping the whole string as one run draws the newline as a zero-width nothing, running the two
halves together. On `005_Contextures_chart_sample` that is `East26%` and `West17%` where the
reference draws `East` `26%` and `West` `17%`: **four fused labels, four tokens lost.**

## Census

Charts whose `c:dLbls` states `showPercent="1"` together with at least one other `show*`:

| family | documents |
|---|---:|
| sheets | **5** — `003`, `011`, `019`, `027` `_advanced_excel_pie`, `005_Contextures_chart_sample` (all `open`) |
| slides | 8 (all `done`) |
| words | 6 (5 `done`, 1 `open`) |

The slides and words rows are **out of reach by construction**: the change is in
`XRay.Office.Spreadsheets/Layout/SheetChart.cs`, which only a workbook's chart is drawn through.
`SlideChart` still carries the identical defect and is the slides track's to fix.

## Predicted verdict movement: **0**

| document | before | after | verdict |
|---|---|---|---|
| `005_Contextures_chart_sample…xlsx` | 289/300, band 6.00 | **293** *(measured)* | `words`, **still failing** |
| `003_advanced_excel_pie.xlsx` | 138/143 | **138** *(measured)* — no label of its is fused | `words`, unchanged |
| `011`, `019`, `027` `_advanced_excel_pie` | 135/140 | unchanged | `words`, unchanged |
| every other sheets document | — | unchanged | unchanged |

**Zero.** This is a correctness fix on ink and on tokenisation that moves no verdict, and it is
being shipped anyway because the fused token is plainly wrong and the risk outside the five named
documents is nil.

## What the census cannot see

1. A label fused by a separator that is **not** `showPercent` — a `c:separator` element stating
   `"\n"` explicitly on a label combining two other parts would not be found by this census's
   `showPercent` key. The change handles any label carrying a break; the census under-reports it.
2. `.xls` charts. `XlsChartSource` builds labels through the same `ChartLabel` model and would be
   fixed by the same code path — the drawing is shared — but no `.xls` document was censused for a
   break-carrying label.
3. Rotated labels are deliberately left on the single-run path (stacking under a rotation needs the
   lines offset along the rotated normal), so a rotated fused label would stay fused. None of the
   five carries one.

## What `005`'s remaining 7 words are, and it is a different defect

After the fix its only divergence is **`Sales` seven times** — and the mechanism is now located.
Five of its six charts state `<c:title><c:overlay val="0"/></c:title>`, a title element with **no
text at all**, and every one of them has exactly one series named `Sales`.
`ChartSpaceConverter::convertFromModel` (`oox/source/drawingml/chart/chartspaceconverter.cxx:181-204`)
fills such a title from `PlotAreaConverter::getAutomaticTitle()`, which
`TypeGroupConverter::getSingleSeriesTitle` (`typegroupconverter.cxx:272-281`) answers with the
single series' cached name. **We draw no title at all.** That is a `XRay.Office.Ooxml` change reaching
all three tracks and it is *not* being made this round — see `results.md` § "left open", including
the branch that substitutes the literal `Chart Title` when there is no single series, which is the
part that makes it a cross-track risk rather than a one-line addition.

# The chartset cluster: a volatile-date confound and an axis-density defect

> **RETRACTED IN PART, 2026-09-08.** The confound below stands and was confirmed. **The defect
> below does not.** Round `chart-axis-r87` refuted it on two counts, both mine:
>
> 1. **`MaximumAutoIntervalCount = 10` is not a constant standing in for a rule.** It is the
>    *first* pass, which is exactly what `VCartesianAxis.cxx:1563-1564` returns while nothing has
>    been measured, and `ChartLayout.IntervalsThatFit` has been the port of
>    `estimateMaximumAutoMainIncrementCount` — available length over label height, clamped into
>    `[2,10]` — since before this probe was written. I read the constant and did not read the
>    caller.
> 2. **The step formula quoted below is the logarithmic routine.** A linear axis takes
>    `calculateExplicitIncrementAndScaleForLinear`, which normalises the distance to
>    1/2/5 × 10ᵏ and climbs that ladder (`ScaleAutomatism.cxx:882-910`). I read
>    `calculateExplicitIncrementAndScaleForLogarithmic`, whose tail converts back to a logarithmic
>    scale, and did not notice which function I was in.
>
> The witness's real seat is the worksheet's **print zoom** reaching a device-quantised label
> height: every length in a chart's composition scales with the zoom and cancels, and the interval
> cap is a ratio of a length to a height that does not. See `probes/chart-axis-r87/results.md`,
> merged at `47259cf60`. The +18 decomposition below is right; the mechanism under it is not.


Scope: the 14 `chartset` rows among the 33 original-corpus failures at
`probes/orig-gate-r83`. Measured from that run's own renders; nothing rebuilt.

## A sixth confound: `TODAY()` is recalculated on load

**10 of the 14** carry `TODAY()`/`NOW()` in `xl/worksheets/*.xml`:

    047_Date_tracker_Gantt 27 · 033_Event_planning_tracker 20 · 065_Weight_loss_tracker 20
    055_Project_timeline_with_milestones 13 · 070_Equipment_inventory_list 8
    040_Blood_pressure_tracker 8 · 075_Idea_planner_tasks 5 · 071_Four-week_project_timeline 3
    030_Basic_balance_sheet 2 · 045_Check_register_with_chart 1

The four without: `053_Personal_asset_inventory`, `057_Simple_balance_sheet`,
`029_Annual_budget`, `038_Competitive_Advantage_Card`.

The reference **recalculates** them and prints today's date; we print the cached value the
file was saved with. On `075_Idea_planner_tasks` the reference prints `9/8/2026` where we
print `4/19/2017`; on `065_Weight_loss_tracker`, `04/08/26 Tuesday` against `08/21/22 Sunday`
— the *weekday* moves too, because it is derived.

This is a confound in the same family as the five font ones: the glyph count moves for a
reason that is not a rendering defect, and it moves **differently on every day the gate is
run**. Any glyph delta on these ten is unusable until the date contribution is subtracted.
It also means these rows are not reproducible: a re-measure tomorrow gives a different number.

## The defect underneath: we always draw the densest legal axis

`040_Blood_pressure_tracker` decomposes **exactly**, and its chart states no `c:min`,
`c:max` or `c:majorUnit` on either `c:valAx` — both axes are fully automatic.

| | reference | ours |
|---|---|---|
| primary value axis | 0 20 40 60 80 100 120 140 | same |
| secondary value axis | 60 65 70 75 (step 5) | 62 64 66 68 70 72 74 76 78 (step 2) |
| category axis | `9/8/2026` ×8 (8 chars) | `11/6/2022` ×8 (9 chars) |

Date length accounts for **+8** glyphs, the extra secondary-axis labels for **+10**. The row's
measured delta is **+18** against a band of 15.

The mechanism, both sides read:

- `ScaleAutomatism.cxx:43-49` — `lcl_getMaximumAutoIncrementCount` returns 10 (500 for a date
  axis). This is the value our `ChartScale.cs:131-136` cites, and it is **not the answer**: it
  is the *upper clamp*.
- `ScaleAutomatism.cxx:143-151` — `setMaximumAutoMainIncrementCount` clamps its argument into
  `[2, 10]`. Ten is the ceiling.
- `VCoordinateSystem.cxx:417` — the argument comes from
  `pVAxis->estimateMaximumAutoMainIncrementCount()`, for every non-date X axis.
- `VCartesianAxis.cxx:1559-1576` — that estimate is the axis's own drawn length divided by the
  measured label height: `nTotalAvailable = nMaxHeight`, `nSingleNeeded =
  m_nMaximumTextHeightSoFar`. It returns the fallback 10 **only** when no label has been
  measured yet (`m_nMaximumTextWidthSoFar==0 && m_nMaximumTextHeightSoFar==0`).
- `ScaleAutomatism.cxx:~427` — the step is then
  `approxCeil((approxCeil(max) - approxFloor(min)) / nMaxMainIncrementCount)`, increased by
  1.0 per iteration until the tick count fits.

So the cap is a **layout** quantity, not a constant. For this chart the pulse range is ~60..78;
a cap of 10 gives `ceil(18/10) = 2` — our step — and the reference's step of 5 implies its
estimate came back at 3 or 4, because the secondary axis is short and its labels are tall.

Our `ChartScale.cs` hardcodes `MaximumAutoIntervalCount = 10` and its doc-comment states the
clamp ceiling as though it were the rule, so we produce the densest axis every automatic axis
allows. On a tall axis that is right by coincidence; on a short one it is wrong, and it is
wrong in the direction of drawing *more* labels — which is what the positive deltas in this
cluster look like.

## What to dispatch

1. Feed the axis's drawn length and measured label height into the interval cap, mirroring
   `estimateMaximumAutoMainIncrementCount`, and keep 10 as the clamp ceiling it actually is.
   Correct the doc-comment at `ChartScale.cs:131-136` at the same time — it is the misreading
   that produced the constant.
2. Decide the volatile question separately. It is not axis work and it changes ten rows'
   numbers on its own.

The remaining cluster rows to re-read after 1 lands: `057_Simple_balance_sheet` (+308 against
a band of 38) and `038_Competitive_Advantage_Card` (+136 / 29) carry no volatile formula, so
their deltas are clean and neither is explained yet.

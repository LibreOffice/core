# L7-charts — summary

14 documents. **Seven chart root causes**, six of them patched (RC-2 has two patches), plus four
documents that hold no chart part, one where the reference is the worse renderer, one bar-of-pie
the 24.2 reference cannot draw, and one (#186) deliberately left unpatched as a cross-lane
question.

## Apply order

The six independent diffs apply standalone, in any order:

```
patches/grid-under-series.diff
patches/cellrange-data-labels.diff
patches/legend-marker-key.diff
patches/multilevel-categories.diff
patches/empty-axis-title.diff
patches/wrapped-category-labels.diff
```

Then the dependent pair, in this order:

```
patches/series-name-from-range.diff      (a)
patches/unnamed-series-legend.diff       (b) — requires (a); fails `patch -p1 --dry-run` without it
```

`(b)` extends the `SeriesName(...)` helper `(a)` introduces, so on a pristine tree its context does
not exist. The prerequisite is stated in a `#` header inside `unnamed-series-legend.diff` itself.

`grid-under-series.diff`, `legend-marker-key.diff` and `wrapped-category-labels.diff` all touch
`ChartLayout.cs`, but at disjoint hunks (~172/~940/~3130, ~3400, ~1960), so offsets shift and
nothing conflicts.

## Root causes

| # | Root cause | File | Docs | Confidence |
|---|---|---|---|---|
| 1 | Bars and candles are emitted into `Boxes`, painted before `Lines`, so the gridline runs **over** every bar — the "horizontal stripes". Every other series mark already goes to `Shapes`. | `Core/Charts/ChartLayout.cs:3069`, `ChartLayout.Plots.cs:466` | #158 #161 #188 | high |
| 2a | The series *name* is read from the cache only; the `ChartRangeResolver` in scope three lines above is never asked. No name → no legend entries → no legend. | `Ooxml/DrawingML/DrawingChartPlot.cs:1165` | #166 | high |
| 2b | A series with **no `c:tx` at all** gets no name, where Calc generates `Column C`. | `Core/Charts/ChartLayout.cs:3516` | #188 | med-high |
| 3 | `c15:datalabelsRange` is never read, so a `CELLRANGE` field draws its own placeholder `[CELLRANGE]`. | `DrawingChartPlot.cs:1569`, `ChartDataLabel.cs:78-82` | #002 | high |
| 4 | A line series' legend key draws the rule and drops the symbol the reference draws on it. | `ChartLayout.cs:3404-3419`, `LegendEntry` `:3540` | #055 | high |
| 5 | A `c:multiLvlStrRef` is handed to the resolver, which flattens its 2×8 rectangle into 16 categories; `ReadMultiLevel` is dead on the Calc host. | `DrawingChartPlot.cs:2472-2482` | #161 | high |
| 6 | An axis `c:title` with no text draws nothing; the reference substitutes `Axis Title`. | `DrawingChartPlot.cs:235-236,265` | #161 | high |
| 7 | `LineBreakAllowed` is read only to switch itself off; labels are measured as one line, collide, and every other one is dropped. | `ChartAxisLabels.cs:157-165,456` | #158 | med-high |
| — | 4 of the 14 documents hold **no chart part**; 1 is `lo-broken` in our favour; 1 is a bar-of-pie 24.2 cannot draw; #186 is a cross-lane pivot question. | — | #014 #022 #095 #114 #173 #175 #186 | high |

## Two corrections to the case notes

- **#002** — "the plot area is filled black where the reference leaves it white" is wrong. The
  file states the black (`c:chartSpace/c:spPr` → `schemeClr tx1`, theme `dk1` = `000000`) and the
  reference draws the same black chart on **its page 8**. Our 5-vs-8 pagination put a different
  page under the comparison.
- **#055** — "the markers are not drawn, in the plot or in the legend" is half wrong. We *do*
  draw them in the plot; only the legend key is missing. The tree already records this exact
  misreading of this exact page (`TODO.batches.md:16815,16966`).

## Cross-lane dependencies

1. `XRay.Office.Spreadsheets/Ooxml/XlsxDrawings.cs:328-330` passes `styles: null` → hairline
   automatic series lines (#166's "pale" series). Same gap in
   `XRay.Office.WordProcessing/Ooxml/DocxPictures.cs:266-269`.
2. The range resolver must skip a trailing totals row (#186), needing the sheet's pivot/table
   model — and needing the 26.2 pivot-regeneration question settled first.
3. `XRay.Office.Presentations/Layout/SlideChart.cs` runs a `\n`-bearing label into one glyph run;
   RC-7 can newly expose it on slides.
4. Six test assertions read bars/candles out of `drawing.Boxes` (listed in `findings.md` §1);
   `dotnet/tests/` is unowned by any lane.

# A reversed category axis, and an accounting format's zero

Measured 2026-09-06 in `/home/user/wt-slidechart`, branch `agent/slidechart`, base `de8423d2b`.
Reference `/opt/libreoffice26.2/program/soffice` **26.2.4.2**, its Latin metric duplicates and its
Latin `NotoSans`/`NotoSerif` moved aside; `/usr/bin/soffice` is **24.2.7.2** and is what the gate's
own reference half is rendered with. Corpus `/home/user/sample-files`, 947 documents.
Ink is the mean absolute grey difference at 30 dpi, page for page, over the shared pages — the
same measure as `probes/words-apo-table/inkcheck.py`, which is where both of these defects were
found.

Both defects were found by spot-checking, not by the gate, and **neither moves a gate verdict**:
the slides track is 302 of 302 unchanged after the fix, `match` 291 / `words` 10 / `unembedded` 1
in both directions.

---

## 1. `N2_E_Maestroni_Swarm_COP.pptx` page 7 — a Gantt chart

### Not three faults. At least six, and the brief names three of them.

The brief's hypothesis was that a reversed axis failing to reserve label room would produce all
three symptoms from one cause. It does not. **The reversal is one cause with two consequences**,
and the other symptoms are independent of it — they are present in identical form with the
reversal patched out of the file.

| symptom | cause | fixed here |
|---|---|:-:|
| last category at the top | `c:catAx/c:scaling/c:orientation val="maxMin"` never read | yes |
| the value axis along the wrong edge | the same statement — the axis stands at the *start* of the axis it crosses | yes |
| date labels along the wrong edge once the axis moves | `c:valAx/c:tickLblPos val="high"` never read | yes |
| the value axis at the far end on four other charts | `c:valAx/c:crosses val="max"` never read | yes |
| category labels unrotated | **not a fault** — the reference draws them unrotated too | n/a |
| category labels clipped off the left edge, all 55 drawn | the plot rectangle is taken from `c:manualLayout` verbatim, and no arrangement runs on a *vertical* category axis | no |
| date labels unrotated and overlapping | `c:valAx/c:txPr/a:bodyPr rot` is never read — there is no `ValueAxisText` | no |
| every string in the chart too large | 26.2.4.2 scales the whole chart's primitives 0.950 vertically and 0.835 horizontally; we do not | no |

### The reversal, measured

The decisive instrument is one attribute. `probes/chart-cat-reverse` builds a copy of the deck
with `<c:orientation val="maxMin"/>` on the category axis changed to `minMax` and renders both
through 26.2.4.2; the two renderings differ in nothing else.

| | `minMax` (patched) | `maxMin` (as authored) |
|---|---|---|
| `LEOP [0000]`, the first category | y = **507.14** (bottom) | y = **106.97** (top) |
| value axis line | H at y = **514.97** (bottom) | H at y = **108.00** (top) |
| value axis labels | y 94–108 (**top**) | y 500–537 (**bottom**) |
| category labels drawn | 19 of 55 | 19 of 55 |
| plot rectangle | 218.78–698.00 × 140.12–514.97 | 225.58–719.66 × 108.00–491.41 |

The same experiment on a column chart, `002_advanced_powerpoint_column.pptx`:

| | as authored | category axis reversed |
|---|---|---|
| `M1`, the first category | x = **109.87** | x = **521.29** |
| value axis line | V at x = **85.58** (left) | V at x = **559.11** (right) |
| value axis labels | x 62–73 (left) | x = **566.19** (right) |
| category labels | y = 431.61 (bottom) | y = 431.61 (bottom) |
| leftmost clustered pair | red 98.87–116.62, blue 116.62–134.36 | **blue** 75.49–93.23, **red** 93.23–110.98 |

So the reversal mirrors the whole bar, not its slot: **the series inside one category turn round
with the categories.** That is the third of the four `wt-slides-chart` assertions, measured rather
than inherited.

### The mechanism, in the C++

- `AxisProperties::initAxisPositioning`, `chart2/source/view/axes/VAxisProperties.cxx`:232-234 —
  the value axis stands at `ChartAxisPosition_START` of the axis it crosses, and at `_END` exactly
  when `m_bIsMainAxis == m_bCrossingAxisHasReverseDirection`.
- `VCartesianCoordinateSystem.cxx`:145 sets that flag from the crossing scale's
  `Orientation == AxisOrientation_REVERSE`.
- `VCartesianAxis::getAxisIntersectionValue`, `VCartesianAxis.cxx`:1092-1101 — `c:crosses`
  (`CrossoverPosition`) decides which end; `autoZero` asks for value zero on the crossing axis,
  which for a category axis running from a half to n-and-a-half clamps to its minimum.
- `VCartesianAxis::getLabelLineIntersectionValue`, `:1103-1113` — the *labels* have a line of
  their own: `c:tickLblPos val="low"` puts them at the crossing axis' logical minimum, `high` at
  its maximum, `nextTo` wherever the axis line is.
- `oox/source/drawingml/chart/axisconverter.cxx`:92-101 and :443-451 map `c:tickLblPos` and
  `c:crosses` onto those two properties.

Each of the four is logical — an end of the crossing axis in that axis' own direction — and the
reversal is what turns a logical end into a screen edge. Modelling them any other way needs a
special case per combination; modelling them this way needs none.

### `c:crosses` was necessary, and the sweep is what said so

Reading the reversal alone regressed four sheets documents, all horizontal bar charts that reverse
their categories. Every one of them states `c:valAx/c:crosses val="max"` and its own `c:axPos`
agrees; 26.2.4.2 draws their value axes along the *bottom* of a reversed chart where
`045_Check_register_with_chart`, which says `autoZero`, has its along the top.

Censused over the corpus's chart parts, 281 value axes:

| `c:crosses` | category axis | | parts | documents |
|---|---|---|---:|---:|
| `autoZero` | forward | primary | 214 | 120 |
| `autoZero` | forward | secondary | 33 | 28 |
| `autoZero` | reversed | primary | 13 | 11 |
| `autoZero` | reversed | secondary | 1 | 1 |
| `crossesAt` | forward | primary | 1 | 1 |
| `max` | forward | secondary | 14 | 10 |
| **`max`** | **reversed** | **primary** | **4** | **4** |
| `max` | reversed | secondary | 1 | 1 |

**No primary value axis crossing a forward category axis says anything but `autoZero`**, so
reading `c:crosses` cannot move any chart except the four it was added for. `c:crossesAt` is not
read: on a category crossing axis it names a category index, one chart part states one, and it is
on a value axis whose category axis is forward.

### Reach

- **Reversed category axis: 15 documents** — sheets 14, slides 1, words 0 — of the 167
  chart-bearing documents in a corpus of 947. Six of the fourteen sheets ones are radar charts,
  where the reversal turns the categories the other way round the web.
- **`c:tickLblPos` other than `nextTo`/`none` on a value axis: 15 documents** — 11 slides, 4
  sheets. Only **four** say `high`: the Maestroni deck and three sheets. The other eleven say
  `low`, which on a forward category axis is the side we already drew them on, so they cannot
  move.
- **`c:crosses val="max"` on a primary axis: 4 documents**, all listed above.

### Before and after, against 26.2.4.2

`ink.tsv` — every corpus document the change can reach, rendered with the base binary and with the
fixed one, both against the same 26.2.4.2 reference. **Ten improve, nine are unchanged, one moves
by 0.01**; the sum of the twenty means goes **101.98 → 98.30**.

| document | mean before | mean after | worst before | worst after |
|---|---:|---:|---:|---:|
| `N2_E_Maestroni_Swarm_COP.pptx` | 3.98 | **3.95** | 39.53 | **38.53** |
| `034_Personal_net_worth_calculator` | 1.01 | **0.51** | 6.21 | **2.66** |
| `045_Check_register_with_chart` | 1.48 | **0.96** | 1.48 | **0.96** |
| `032_Business_expenses_budget` | 3.13 | **2.15** | 7.20 | **3.36** |
| `030_Basic_balance_sheet` | 3.50 | **2.72** | 11.12 | **9.01** |
| `056_Quarterly_sale_report` | 1.63 | **0.99** | 3.61 | **1.96** |
| `003_Contextures_chart_sample` | 0.69 | **0.65** | 2.30 | **2.06** |
| `010_Contextures_chart_sample` | 3.58 | **3.50** | 8.27 | **7.94** |
| `031_Business_expense_budget` | 1.09 | **1.01** | 3.16 | **2.78** |
| `065_Weight_loss_tracker` | 46.93 | 46.91 | 72.23 | 72.23 |
| `023_Waterfall_Chart_Template` | 2.45 | 2.46 | 5.68 | 5.68 |
| nine others | — | unchanged | — | unchanged |

**Page 7 moves by one point of ink for a fault that is the whole shape of the page**, and the
reason is worth writing down rather than explaining away: 44% of that page's ink is the strip left
of x = 230, which holds 55 clipped category labels against the reference's 19, and 90% of it is
inside the plot's own y band, whose rectangle is 145.06–737.41 against the reference's
225.58–719.66. The bars are now in the right order and in the wrong rectangle. The three faults
below are what own that rectangle.

### What page 7 still gets wrong

**1. The plot rectangle is `c:manualLayout` taken at its word.** The deck states
`layoutTarget="inner"`, `x=0.20148`, `y=0.03647`, `w=0.82271`, `h=0.90339`, and
`PlotAreaOf` (`ChartLayout.cs`) returns exactly `frame × those fractions` — 145.06–737.41 ×
108.89–513.09 on a 720-wide frame, so **the plot runs 17 pt off the right edge of the chart**.
26.2.4.2 gives 225.58–719.66 × 108.00–491.41. It does not honour the stated rectangle either:
`0.20148 + 0.82271 = 1.0242`, so the rectangle the file asks for does not fit, and
`VDiagram::adjustInnerSize` (`chart2/source/view/diagram/VDiagram.cxx`:652-700) shrinks the inner
rectangle by how far the drawn labels overflow the available one and clamps it to the outer. That
is an iterative fit — `ChartView::impl_createDiagramAndContent` drives it — and it is a round of
its own, but it is the seat of most of this page's remaining error.

**2. No arrangement runs on a vertical category axis.** `ChartAxisLabels.Resolve` reproduces
`VCartesianAxis::createTextShapes`' restart loop, and `ChartLayout.Place` calls it only under
`plot.HasAxes && columns`. So a horizontal bar chart never thins, rotates or staggers its category
labels: we draw all 55 at a 7.35 pt pitch with a 10.5 pt line height, and 26.2.4.2 draws every
third one — 19, at a 20.91 pt pitch. Making `Resolve` axis-agnostic is the fix; it needs the
collision test and the depth to be taken along the axis rather than along x.

**3. The chart's own text is drawn at the frame's scale and 26.2.4.2 scales it.** Every string
26.2.4.2 draws inside this chart carries `Tm` with a horizontal scale of `0.879407236`, and its
sizes are 7.609 for the 8 pt axis labels and 11.4 for the 12 pt title — **0.9511 and 0.950
vertically, 0.8364 and 0.8354 horizontally**. Slide text on the same page is unscaled and agrees
with ours exactly, so it is the chart's primitives and not the page. This is the same phenomenon
`dotnet/CLAUDE.md` already records for `tdf106217.pptx` — *"the chart is scaled unequally"* — and
it is why the reference's rotated labels are drawn as **glyph outlines**: a 45° rotation on top of
an anisotropic scale is a shear the PDF text state cannot carry, so `pdftotext` and PyMuPDF both
report the date labels as absent. **They are not absent.** They are 173 filled paths between
x 193.63–712.80 and y 500.20–537.70. An earlier reading of this page from `pdftotext` concluded
that 26.2.4.2 draws no date labels at all, and that conclusion was wrong in exactly the way
`CLAUDE.md`'s standing warning about that channel says it will be.

**4. The value axis' own `a:bodyPr rot` is never read.** The file states `rot="-2700000"` on
`c:valAx/c:txPr`; `AxisTextOf` is called for the category axis alone and there is no
`ValueAxisText`, so our date labels are upright and overlap — `11/22/2013` is 51.9 pt wide at a
32.4 pt pitch.

**5. One label size serves both axes.** `ChartPlot.LabelSize` is a single property, so this
chart's category labels are drawn at the value axis' `sz="900"` where the axis states nothing and
should take the chart space's `sz="800"`.

---

## 2. `Demick_JetBlue.pptx` page 5 — an accounting format's zero

### The zero placeholder, and the two binaries disagree about it

The value axis states
`_("$"* #,##0.00_);_("$"* \(#,##0.00\);_("$"* "-"??_);_(@_)`. The zero subformat's two `?` hold
the dash clear of the column's right edge so that it lines up with the decimal point of the rows
above it.

`NumberFormatter` wrote an ordinary blank for an unfilled `?`. Two things then went wrong at once,
and the second is why a dash-only fix would not have been enough: an ordinary blank is narrower
than a digit, **and** it is trailing whitespace, which a right-aligned line drops from both its
width and its glyphs (`TextMeasurer.TrimTrailingSpaces`, `VisibleEnd`). The whole tail vanished.

`probes/chart-cat-reverse/make-numfmt.py` builds a sixteen-cell workbook over the seven codes that
use `?` or `*` and renders it through both installed binaries, reading the glyphs out of the PDFs
rather than out of `pdftotext`, which cannot show the difference:

| code | value | 26.2.4.2 | 24.2.7.2 |
|---|---|---|---|
| `_("$"* "-"??_)` | 0 | `$` … `-` U+2007 U+2007 blank | `$` … `-` blank blank blank |
| `??0` | 5 | U+2007 U+2007 `5` | blank blank `5` |
| `# ??/??` | 2.7 | `2` blank U+2007 `7/10` | `2` blank blank `7/10` |
| `# ??/??` | 1.25 | `1` blank U+2007 `1/4` U+2007 | `1` blank blank `1/4` blank |
| `# ?/?` | 3 | `3` blank U+2007 blank U+2007 | `3` blank blank blank blank |
| `0.??` | 1.0 | `1.` U+2007 U+2007 | `1.` blank blank |

**26.2.4.2 writes U+2007 FIGURE SPACE for every unfilled `?`; 24.2.7.2 writes U+0020 for every
one of them.** The seat is `cBlankDigit = 0x2007`, `svl/source/numbers/zformat.cxx`:71 —
*"tdf#158890 use figure space for '?'"* — reached from the integer run (`:4791`), the trailing
decimals (`ImpNumberFill`, `:4623`) and a fraction's denominator (`:4935`). The tree is screened
against 26.2.4.2, so it follows 26.2.4.2; every figure in `NumberFormatterTests` was 24.2.7.2's
answer and is now 26.2.4.2's, and the constant's own remarks say so, so that nobody re-derives it
in the other direction.

The fix is in `XRay.Office.Core/Numbers` and not in the axis, which is the whole reason
`Core/Numbers` came down a layer.

### What it moved

| | before | after | 26.2.4.2 |
|---|---|---|---|
| the zero label as drawn | ` $-` | ` $-` U+2007 U+2007 | ` $-` U+2007 U+2007 blank |
| its span | 146.54–159.23 | 134.13–159.30 | 134.89–162.92 |
| the dash's own x | **155.85** | **143.43** | **144.14** |

**11.71 pt of error to 0.71 pt**, and the residual is the chart's own scale (§1.3 above), not the
format.

Page 5's ink is 23.49 before and 23.50 after, and the document's mean is 11.26 either way: the
label is six glyphs on a page whose plot rectangle is 161.99–662.93 against the reference's
165.71–661.95 and whose 26 rotated category labels are drawn as outlines. **The ink is the wrong
instrument for this defect and the glyph position is the right one**, which is the same lesson as
§1's page 7 in the other direction.

### `_x` is left alone, on measurement

`SvNumberformat::InsertBlanks` (`zformat.cxx`:89-104) inserts one, two or three ordinary spaces
from a 96-entry table of coarse character widths, where we insert one. Over the chart parts and
workbook styles of all 947 corpus documents there are **2572** `_x` directives and the `x` is one
of `(`, `)`, `-` and a blank in every one — 1161, 1132, 140 and 139 — and all four are `1` in that
table. So the table has nothing to add here and a wrong count would be invisible; the citation is
in `NumberFormatSection` for whoever meets a format that needs it.

### Reach

A `?` digit placeholder appears in **6 chart parts** — `044_Cash_flow_forecast`,
`056_Quarterly_sale_report`, `057_Simple_balance_sheet`, `032_Business_expenses_budget`,
`Demick_JetBlue.pptx` and `southern-classic-kennesaw-state-university-final.pptx` — and in the
`xl/styles.xml` of **45 workbooks**, **47 documents** in all. Extraction changes for all of them:
a `?` now extracts as U+2007. That is what 26.2.4.2 puts on the page and in its own clipboard, and
U+2007 is Unicode whitespace, so a word count that splits on Unicode whitespace is unaffected; a
`wc -w`, which splits on ASCII whitespace only, is not. The gate counts alphanumeric characters
and tokens carrying one, so neither column can see it — and the slides sweep confirms that at 302
of 302.

---

## 3. The category-label collision on `Demick_JetBlue.pptx` page 5

**Ours is the more legible page and the reference is not wrong.** The difference should be closed
towards 26.2.4.2, and it is not closed here.

The chart's category axis states `<c:tickLblPos val="nextTo"/>` and `<c:crosses val="autoZero"/>`,
and its value axis runs from −200,000 to 1,200,000 — so the category axis crosses *inside* the
plot area, at the zero gridline. 26.2.4.2 therefore hangs the 26 rotated quarter labels from the
zero line at y = 374.83, and they run down and to the right across the plot's whole negative band
and through the `$(200,000.00)` label. We draw them below the plot rectangle instead, clear of
everything.

The evidence that this is the format rather than a bug:

- `ST_TickLblPos`' own definition: `nextTo` means beside the axis; `low` and `high` mean the ends
  of the plot area. A file that wants its labels out of the data says `low`, and thousands do.
- `oox/source/drawingml/chart/axisconverter.cxx`:97-99 maps `nextTo` to
  `ChartAxisLabelPosition_NEAR_AXIS`, and `VCartesianAxis::getLabelLineIntersectionValue`
  (`:1103-1113`) then puts the label line at `getAxisIntersectionValue()` — the crossing position,
  which `autoZero` makes the value zero. So 26.2.4.2 is doing what the file says, by the shortest
  path there is.
- Our own behaviour has no rule behind it: `AddCategoryAxis` draws category labels at
  `area.Bottom` unconditionally, ignoring where the axis crosses. It is legible by accident.

So this is **our** defect, not LibreOffice's, and the standing instruction to close the gap
applies. It is unclosed because the fix is a feature — the category axis' crossing position, which
also decides where the axis *line* is drawn — with a reach of its own: **4 documents** hold a chart
whose category axis crosses inside the plot (`nextTo`, `autoZero`, and at least one negative value
in a series): `Demick_JetBlue.pptx`,
`southern-classic-kennesaw-state-university-final.pptx`, `055_Project_timeline_with_milestones`
and `031_Business_expense_budget`.

---

## Files

| file | what it is |
|---|---|
| `census.py`, `census.tsv` | reversed category axes, tick-label positions and accounting formats over all 947 documents |
| `make-numfmt.py`, `numfmt.xlsx` | the sixteen-cell number-format probe, and the reader that prints its glyphs from both binaries |
| `slides-sweep.py`, `slides-after.tsv` | the slides track re-scored against the gate's own banked reference PDFs; the script validates its verdict rule against the gate's two halves before scoring and refuses to run if it does not reproduce all 302 |
| `affected.txt`, `ink.tsv` | the twenty corpus documents the change can reach, and their ink before and after against 26.2.4.2 |

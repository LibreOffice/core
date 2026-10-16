# words-r52 — prediction, committed before any change

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`fc-match Calibri` → `Carlito-Regular.ttf`, corpus `/c/sandbox/workdir/sample-files`,
worktree `wt-words-r50` on branch `wt-words-r52`, base `166a019c6b0`,
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## Baseline, reproduced before anything was touched

`batch-check.sh … 'words/*' … 8` → `TOTAL 355  MATCH 333  MISMATCH 22`. Scored against
`MANIFEST.tsv`'s 337 path list instead of that total (the extra 18 rows are the case-insensitive
mount's alias entries): **316 match, 21 open, 0 disagreements with the manifest's status column,
document for document.** The briefed baseline reproduces.

---

## Fix A — a VML shape's fill, its outline and its straight connectors

### The observable, measured before the change

Stroke and fill operator records, ours against the reference (`pdf-ops.py dump --only stroke|fill`):

| document | ref strokes | ref fills | ours strokes | ours fills |
|---|---:|---:|---:|---:|
| `065_Work_Breakdown_Structure_Template_Blue_Theme` | 43 | 21 | **0** | **0** |
| `068_Work_Breakdown_Structure_Template_Green_Theme` | 71 | 41 | **0** | **0** |
| `069_Work_Breakdown_Structure_Template_Professional_Format` | 59 | 0 | **0** | **0** |

Read out of the reference's own content stream rather than a raster: `068`'s reference emits
**41 `f*` fills at `#E2EFD9`** and **36 strokes at `#70AD47`**, which are exactly the literal RGB
values in `fillcolor="#e2efd9 [665]"` and `strokecolor="#70ad47 [3209]"` — the theme index in the
brackets is *not* consulted. That is what `ConversionHelper::decodeColor`
(`oox/source/vml/vmlformatting.cxx:252-257`) does: a leading `#RRGGBB` returns before the palette
branch at line 282 is ever reached. `069` confirms it a second time — 22 fills, all `#F2F2F2`,
from `fillcolor="#f2f2f2 [3052]"`.

Stroke widths, measured off the 300 dpi reference raster rather than off the `w` operator (the
whole PDF carries a single `0.1 w`, which is *not* the drawn width): a `v:rect` border that states
no `strokeweight` comes out **1 device pixel** — a hairline — and a `v:shape type="#_x0000_t32"`
connector stating `strokeweight="1pt"` comes out **4 px at 300 dpi ≈ 0.96 pt**. So `strokeweight`
is honoured and its absence means a hairline.

### The change

`DocxVmlFrames` reads `fillcolor`, `strokecolor`, `filled`, `stroked`, `strokeweight` and the
`v:fill`/`v:stroke` child equivalents, and sets the `Fill`, `BorderColour`, `BorderWidth`,
`IsLine` and `IsLineMirrored` that `PageFrame` already carries and `PageDrawing` already paints.

Deliberately conservative, and the conservatism is the same rule `DocxFrames.Appearance` already
documents — *read what the shape states and never invent ink*:

- a fill and a rectangular border are applied only to `v:rect` and `v:roundrect`, whose geometry
  **is** the rectangle we would draw;
- a stroke as a corner-to-corner diagonal (`IsLine`) is applied only to a straight connector —
  `o:connectortype`, or a `type` resolving to `o:spt="32"`;
- **no default is invented.** LibreOffice defaults an unstated VML fill to white and an unstated
  stroke to black (`FillModel::pushToPropMap`, `StrokeModel::pushToPropMap`); doing that here
  would put a white fill and a black box around all 37 `#_x0000_t75` picture shapes in the words
  corpus, none of which states either. Absent colour therefore means no paint;
- the zero-extent rejection in `Flatten` and `Floating` is relaxed **only** for a straight
  connector, which is how VML writes a vertical rule (`width:0;height:12.7pt`). The inline arm
  `One()` keeps its extent check unchanged, because a zero-extent inline shape reserving line room
  would move a page count.

### Documents predicted to change

Census over the 271 distinct words documents, counting only VML outside `mc:Fallback` that states
a colour and is a `rect`/`roundrect` or a straight connector — **10 documents**:

| shapes | document |
|---:|---|
| 56 | `chartset-010/docx/067_Work_Breakdown_Structure_Template_Gray_Theme` |
| 53 | `chartset-011/docx/068_Work_Breakdown_Structure_Template_Green_Theme` |
| 47 | `chartset-013/docx/069_Work_Breakdown_Structure_Template_Professional_Format` |
| 39 | `chartset-010/docx/066_Work_Breakdown_Structure_Template_Colored_Background` |
| 31 | `chartset-011/docx/065_Work_Breakdown_Structure_Template_Blue_Theme` |
| 4 | `chartset-010/docx/090_Business_Case_Template_Blue_Theme` |
| 4 | `missing-001/docx/May 25 bulletin focus on carers in the workplace` |
| 1 | `chartset-006/docx/093_Business_Case_Template_Customizable_Layout` |
| 1 | `chartset-008/docx/098_Business_Case_Template_Fillable_Layout` |
| 1 | `chartset-012/docx/095_Business_Case_Template_Easy_Format` |

### Verdict movement predicted: **0**

A fill and an outline carry no text and no height. Neither can move a page count, an extractable
word count or a font list, and no column of `batch-check.sh` looks at either. **Zero is the
honest answer here and the measurement is the rendering**, not the gate. Six of the ten documents
above already `match` and must still `match`; `065` and `069` stay open on words and must not get
worse.

### What this census cannot see

- **VML inside `mc:Fallback` — 4166 shapes.** We read the `mc:Choice` DrawingML instead, and
  round 51 established by deletion that the fallback contributes nothing. Invisible either way.
- **`v:line` — 6 shapes in `33004.docx` and `JEMIT_Template.docx`.** `IsShape` does not accept
  the element at all, so it stays unread and unpainted. Not in scope, and named so that a later
  round does not read a zero here as evidence there is nothing there.
- **`v:shapetype` inheritance.** A shape can inherit its fill and stroke from the `v:shapetype`
  it names. Measured rather than assumed: all 64 colour-bearing `v:shapetype` elements in the
  words corpus carry only `filled="f"` / `stroked="f"` and **not one carries a colour**, so under
  a stated-colour-only rule the inheritance cannot change anything. This is the census's weakest
  point and it was closed by measurement, not by argument.
- **`#_x0000_t136` WordArt (15 shapes, 4 documents) and `#_x0000_t15` (3 shapes).** Both state a
  `fillcolor` that fills glyph outlines or a non-rectangular silhouette, not a rectangle. They are
  excluded, so they will keep drawing nothing; that is under-drawing, not a regression.
- **Ordering.** A fill is painted before the frame's own content. If any of the ten documents
  has a filled shape stacked over text that is currently visible, the fill will hide it. Nothing
  in the census can see paint order; only the after-rendering can.

---

## Fix B — chart categories, and a bar-of-pie's percentages

### The observable

Two blind readings, each by a fresh subagent forbidden to read any project file or run any
command, each asked to transcribe the halves separately, independently transcribed the same two
things on `028` and `027`:

- the reference's labels and legend read `Branch 1 Stem 2 Leaf 5`, ours read `Branch 1`;
- on `028` the reference puts a percentage on every label and **ours carries none at all**, while
  on `027` ours *does* draw the percentage (its labels fuse into `Branch 315%`).

That split is the whole diagnosis and neither reader was told anything.

### The two causes, both found in the source after the readings

1. **`DrawingChartPlot.ReadSequence` flattens a `c:multiLvlStrCache` by walking
   `cache.Descendants(pt)`** — every level's points, all keyed on `@idx`, so each level
   **overwrites** the one before it and the last level written wins. The brief called this
   "taken at one level"; it is more specific than that, and the level that survives is the
   outermost. `DrawingChart.ReadSequence` — the *extraction* reader — already joins the levels
   correctly, so the two readers of the same element disagree with each other today.
   LibreOffice joins them with a space, outermost first:
   `lcl_getExplicitSimpleCategories`, `chart2/source/tools/ExplicitCategoriesProvider.cxx:376-395`.
2. **`c:ofPieChart` is not treated as a pie for `c:showPercent`.** `LabelOf` gates the percentage
   on `kind == ChartPlotKind.Pie`, and `ofPieChart` maps to `ChartPlotKind.OfPie`. LibreOffice's
   table puts `TYPEID_OFPIE` in `TYPECATEGORY_PIE` beside `TYPEID_PIE` and `TYPEID_DOUGHNUT`
   (`oox/source/drawingml/chart/typegroupconverter.cxx:103-105`), and it is that category the
   percentage is ANDed with (`seriesconverter.cxx:140`).

### Documents predicted to change, and by how much

The joined labels are worth **+56 words per occurrence-set** on both `027` and `028`, computed
from their caches; each document draws the set twice, once as data labels and once as the legend,
so **+112**.

| document | ours now | predicted | reference | band | predicted verdict |
|---|---:|---:|---:|---:|---|
| `027_Unit_Circle_Chart_Graphical_Chart` | 261 | ~373 | 378 | 7.56 | **match** |
| `029_Unit_Circle_Chart_Pie_Theme` | 107 | ~111 | 114 | 3 | **match** (at the edge) |
| `028_Unit_Circle_Chart_Optimized_Graph` | 191 | ~319 | 327 | 6.54 | still **open** |
| `024_Unit_Circle_Chart_Colorful_Circles` | 95 | 95 | 105 | 3 | unchanged |

`029` gains only the four percentages its four `1st Qtr`…`4th Qtr` categories are worth; at a
predicted gap of 3 against a band of exactly 3 it passes only because the test is `d > 3`, so it
is the prediction most likely to be wrong by one.

**`024` is refuted as a chart document.** It carries no chart part at all — its graphic is a
`word/diagrams/` SmartArt (`data1.xml`, `layout1.xml`, `drawing1.xml`). The brief grouped it with
the three chart documents; it is a different defect and nothing here touches it.

### Verdict movement predicted: **+2**, words 316 → 318

`027` and `029`. `028` improves by about 128 words and stays open.

### This diff touches a shared layer

`XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs` is below all three families. Census over the whole
corpus: **23 charts in 23 documents hold a `c:multiLvlStrRef`** — 2 words, **18 slides**,
**3 sheets** — and 2 documents hold a `c:ofPieChart`, both words.

Slides: `chartset-002/{001,006,014,017,030,033}`, `chartset-003/{002,003,009,010,011,019,027}`,
`chartset-004/{018,022,025,026}`, `done-011/171128IPAP.pptx`.
Sheets: `chartset-005/004_Contextures_chart_sample`, `chartset-007/040_Blood_pressure_tracker`,
`chartset-008/034_Personal_net_worth_calculator`.

**20 of those 23 charts are bar, line or area charts**, where the categories feed a category axis
rather than a legend. Today we draw the outermost level on that axis; after the change we draw the
join. LibreOffice draws a *complex* category axis as several stacked rows of labels, which is
neither of those — so on the axis charts this is a change towards the reference in content and
away from it in shape, and it can change label wrapping, plot-area size and therefore ink.
**This must be swept, not reasoned about**, and I will sweep those slides and sheets batches
myself rather than hand the parent an argument.

### What this census cannot see

- **The ODF chart reader** (`XRay.Office.OpenDocument/OdfChartPlot.cs`) and the two binary readers
  (`XlsChartReader`, the `.ppt` path) have their own category code and are not touched. A grep for
  `multiLvlStrRef` cannot find an ODF or binary equivalent, so a zero there is not evidence.
- **`ChartRangeResolver`.** In the spreadsheet family the `c:f` wins over the cache when the
  workbook can be resolved, and a resolved range never goes through the multi-level branch at all.
  So the three sheets documents may not move even though the census names them — an
  **over**-reach in the safe direction, which is the direction that does not conceal itself.
- **Doughnut.** `TYPEID_DOUGHNUT` is also `TYPECATEGORY_PIE` in LibreOffice. Whether
  `c:doughnutChart` maps to `ChartPlotKind.Pie` here is not something this census establishes, and
  if it maps elsewhere then a doughnut's `c:showPercent` is a second instance of the same defect
  that this change will not fix.
- **Label collision.** Both blind readers reported the reference's longer labels overlapping each
  other. Longer labels on our side will collide too, and a collision that suppresses a label would
  cost words the arithmetic above has counted. The arithmetic is an upper bound.

---

## What this round does *not* do, and why

**The brief's item 2 — "DrawingML connectors dropped by a one-line predicate" — does not hold as
stated, and the brief asked for exactly this check.** `056`'s 39 connectors were counted and
read: 34 are `prstGeom prst="line"` and 5 are `straightConnector1`, and **34 of the 39 state no
`a:ln` at all.** Their line comes from `wps:style/a:lnRef idx="1"` with an `a:schemeClr` — the
theme's `a:fmtScheme/a:lnStyleLst`, which `DocxFrames.Appearance` deliberately does not read.
Relaxing the `Width <= 0 || Height <= 0` predicate on its own would admit 34 frames carrying
`Fill = null, BorderColour = null` and **paint nothing**. The predicate is necessary and is not
sufficient, and shipping it alone would have produced a confident zero.

The real seat is that `DrawingStyleMatrix` — which exists, is correct, and already resolves
`a:fillRef`/`a:lnRef` for the slides path — never reaches `DocxFrames`; `DocxFrameContext` carries
only the colour scheme. That is the "route, not a rule" shape this project has now hit seven
times. Its reach is **458 shapes across 40 words documents**, most of which currently pass, so it
wants its own round and its own census rather than a corner of this one.

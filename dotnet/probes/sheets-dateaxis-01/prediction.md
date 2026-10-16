# Prediction — a BIFF date axis, measured against 26.2.4.2 rather than read off the 27.2 tree

Written before any code was changed. Scored in `results.md`.

## What the round set out to do

`sheets/batch-010/xls/Template Pilot Logbook JAR-FCL V3.0.xls` is page-exact at 38/38 and 204
words short. `probes/sheets-chart-01/results.md` diagnosed it as a missing axis *type*: the
reference draws a date axis on pages 16–18 and we draw a category axis, so its ~38 tick labels
become our 2.

## 0. Is the deficit real, or a `pdftotext` artefact? — checked first, per the brief

**Real.** Whitespace-stripped character streams, per page, `pdftotext -layout` both sides:

| | ours | reference |
|---|---:|---:|
| whole document | 6121 | 6529 |
| page 16 | 128 | 352 |
| page 17 | 91 | 383 |
| page 18 | 19 | 55 |

Five other pages differ by a few characters in each direction (identical word counts — nbsp and
column padding). Pages 16, 17 and 18 are the deficit and they are the three chart pages. The
reference draws 30, 38 and 5 date labels there; we draw 2, 2 and 0.

So this is *not* one of `TODO.raster-ceiling.md`'s four shapes. Content is genuinely missing.

## 1. LibreOffice's tick rule, established by measuring the installed 26.2.4.2

Reference page 17's labels are `30/12/99`, then `02/01/03`, `02/01/06` … `02/01/11` — 38 of them.
Page 16's are `30/12/99`, then `02/11/03`, `02/09/07` … `02/03/11` — 30, at an apparent
**46-month** step. Neither is a whole number of years from the first label, and both keep the
day `02` forever. That is the shape the whole rule falls out of:

1. **The axis minimum is serial 0** (`30/12/1899`), not the data minimum 37935.
2. **The tick grid is 2 MONTHS**, not 3 years. `nMaxMainIncrementCount` for a date axis is
   `MAXIMUM_MANUAL_INCREMENT_COUNT` = 500 and is **never** narrowed by the axis' own label
   measurement — `VCoordinateSystem::prepareAutomaticAxisScaling` returns early for a date X axis,
   before `setMaximumAutoMainIncrementCount`. So the interval is
   `nDayCount / 499 = 41292 / 499 = 82` days → MONTH (82 > 31), `floor(82/31) = 2`.
3. **Ticks are calendar additions with roll-over normalisation.** `1899-12-30 + 2 months` is
   `1900-02-30`, and `comphelper::date::normalize` rolls rather than clamps: 30 − 28 = **1900-03-02**.
   Every later tick keeps day 02. This is the whole of the "+3 days" that made the labels look
   unexplainable.
4. **The labels are then thinned by the ordinary collision ladder.** 679 ticks at rhythm 18 give
   38 labels (page 17) and at rhythm 23 give 30 (page 16). Both counts reproduce exactly.

Reproduced from scratch: a hand-built `.xlsx` carrying the same 799 categories (one at 37935,
24 at 41258–41292, the rest blank) draws `30/12/1899, 02/11/1902, 02/09/1905 …` — the same
minimum, the same 2-month grid, the same day-02 roll-over.

**The axis minimum of 0 is `AreaChart::addSeries`.** `chart2/…/AreaChart.cxx:136-143` silently
promotes a series' `LEAVE_GAP` to `USE_ZERO` for any *area* chart, so the 774 blank category cells
count as serial 0. Measured, not read: the same probe as a **line** chart, or as a **bar** chart,
or with `dispBlanksAs="span"`, takes the data minimum instead; only the area chart goes to 0. The
workbook's `CHPROPERTIES` states `mnEmptyMode = 0` = skip = `LEAVE_GAP`, and LibreOffice's own
round-trip of it writes `<c:dispBlanksAs val="gap"/>` — so the model says gap and the plotter
overrides it. Flipping that one element from `areaChart` to `lineChart` in the round-tripped
workbook moves the axis from `30/12/99` to `10/11/03`, which is the single-variable proof.

## 2. Reach, from what charts resolve to rather than from a grep

Censused by decoding every OLE2 workbook's chart substreams and reading `CHDATERANGE`'s flags:

| | |
|---|---|
| corpus documents with a BIFF date axis (`EXC_CHDATERANGE_DATEAXIS`) | **1** |
| chart substreams on it | 2 |
| `.pptx`/`.xlsx` documents with a `c:dateAx` | 3, all on the slides track |
| ODF `chart:axis-type="date"` | 0 |

So the BIFF half of this reaches exactly one document. **I predict 1 of 171 sheets renderings
change and 0 elsewhere**, because I am wiring the BIFF reader only and leaving OOXML's `c:dateAx`
where it is — three decks is a follow-up with its own regression sweep, not a rider on this one.

## Predictions, scored in `results.md`

| # | prediction |
|---|---|
| 1 | The deficit is real content, not tokenisation — **already measured above**, recorded so it is scored rather than assumed |
| 2 | Exactly **1 of 171** sheets renderings change; 0 on words and slides |
| 3 | The page count stays **38/38** — a chart is drawn inside its anchor and cannot move a break |
| 4 | Our page 17 draws **between 30 and 45** tick labels where the reference draws 38 |
| 5 | Our axis minimum lands on `30/12/99` and our first two labels read `30/12/99` and something of the form `02/mm/yy` |
| 6 | The word gate **does not close**, or closes only marginally. The reference's 38 labels extract as ~152 `pdftotext` words because rotated text fragments; ours will too, but the fragment count per label is ours to get wrong. Best estimate **1520–1590 against 1531**, band ±33.6 — so this is a coin toss and I am calling it as *may still fail* |
| 7 | The series mark moves from ~75% of plot width to **91.8–100%**, which is what `sheets-chart-01` predicted for the reference and a blind reviewer confirmed |
| 8 | `XRay.Office.Fidelity.Tests` stays at **30 failed of 550**, the same 30 by name |
| 9 | Batches 001–006 stay at **57/60** and batches 007–009 unchanged |
| 10 | The legend on page 17 stays wrong. `sheets-chart-01` left `VLegend`'s CUSTOM path open and I am not fixing it; but the plot geometry moves under this change, so I predict our legend **moves with it** and the 932-ink-pixels-against-0 finding still stands afterwards |

## What I expect to be wrong about

Prediction 4 is the fragile one and 6 depends on it. The rhythm the collision ladder settles on is
decided by a label box measured in our font stack against a tick spacing of about 0.9 pt, and one
step of rhythm is two labels. The reference's own shown-label spacing is 16.81 pt, and our
`Collides` test says two 45°-rotated boxes of that height clear each other at 17.25 pt — i.e. our
ladder should stop one step *later* than LibreOffice's. If that is right, prediction 4 lands near
36 rather than 38 and prediction 6 fails low.

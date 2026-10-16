# The `.xlsx` chart failures, classified — and there are thirteen of them, not fourteen

Round `agent/xlsxchart`, 2026-09-08, from `6d19fd594`.

## Environment

- Reference: **`/opt/libreoffice26.2/program/soffice`, LibreOffice 26.2.4.2**
  (`0229ac93fcf0d7cbc6376066c6f35021cef002dc`). Not `/usr/bin/soffice`, which is 24.2.7.2.
- The tarball's bundled Latin duplicates are aside — `share/fonts/truetype/.duplicates-aside`
  holds 38 faces including `LiberationSansNarrow`, `.noto-aside` 8. `fc-match "DejaVu Sans"`
  answers `DejaVuSans.ttf` and `fc-match Calibri` answers `Carlito-Regular.ttf`.
- Corpora: `/home/user/sample-files` (947) and `/home/user/corpus-odf` (the converted column).
- Every rendering on both sides taken with `SOURCE_DATE_EPOCH=1700000000`, one output directory
  per *document* keyed on an MD5 of its path, LibreOffice profiles keyed on the same digest,
  `timeout -k 30 240` on both sides.
- Banked reference halves reused rather than re-rendered, which is sound because the diff is
  confined to `dotnet/src` and cannot reach `soffice`: `/home/user/gate-orig-r83/ref` for the
  original corpus and `/home/user/gate-odf-r80/ref` for the `.ods` column.
- **The box was at 100% disk for most of this round** (73 MB free at the worst). Every sweep here
  scores its rendering and deletes it, which is why `confine.py` exists rather than a second bank.

## What the brief got wrong

1. **It is thirteen rows, not fourteen, and only ten of them hold a chart.** The two non-`chartset`
   `.xlsx` failures — `sheets/done-009/…sistem-rekod-markah…` and
   `sheets/unstable-001/fse_identification_form` — contain **no chart part at all**; neither zip
   holds `xl/charts/`. And three of the thirteen `chartset-*` rows hold none either:
   `070_Equipment_inventory_list`, `071_Four-week_project_timeline` and `075_Idea_planner_tasks`.
   `chartset-*` is a batch name, not a census.
2. **The brief's own table quotes column 4 and calls it column 9.** Its "Δ glyphs / band" figures
   are the `words` deltas: `047` is given as −162, which is `657/819`; its glyph delta is **−300**
   against a band of 67.2. `053` +12 is `57/45`; its glyph delta is +31. Every one of the seven
   figures in that table is the word delta. The rule *is* column 9 — `batch-check.sh`:279-295
   compares `$og`/`$rg` under `d > b*0.02 && d > 15` — so the correction is to the numbers, not to
   the rule.
3. **The volatile-date class is not the answer, and it is the thing this round most expected to
   find.** Ten of the thirteen carry `TODAY()`; freezing every one of them at the serial its own
   cache was built with moves **one** verdict (`040_Blood_pressure_tracker`, which becomes a
   match) and leaves the other nine's glyph deltas within 5 of where they were. A changed date
   changes *which* characters are drawn, not *how many* — `4/14/2017` and `8/24/2026` are both
   eight alphanumerics — so a volatile workbook drifts in ink and barely at all in the column the
   gate scores. `frozen-rows.tsv`.

## The instrument that settles "is it the chart?"

`strip-charts.py` removes every `xdr:` anchor holding a `graphicFrame` from a workbook's drawings
and leaves everything else alone, so the same sheet renders with no chart on it. The chart's own
contribution is then the difference between the two renderings, on each side separately, and it
needs no guess about where on the page the chart sits.

| | pages ours/ref | glyphs ours/ref | with every chart removed |
|---|---|---|---|
| `029_Annual_budget` | 2/2 | 943/914 | **594/594, match** |
| `030_Basic_balance_sheet` | 8/8 | 1727/1624 | **6/6, 1077/1077, match** |
| `033_Event_planning_tracker` | 3/3 | 2720/2650 | 2397/2327, still +70 |
| `040_Blood_pressure_tracker` | 1/1 | 737/727 | **567/567, match** |
| `045_Check_register_with_chart` | 1/1 | 736/764 | 676/704, still −28 |
| `047_Date_tracker_Gantt_chart` | 5/8 | 3058/3353 | 2940/3235, still 5/8 and −295 |
| `053_Personal_asset_inventory` | 4/2 | 257/226 | 160/129, still 4/2 and +31 |
| `055_Project_timeline` | 2/2 | 960/934 | **680/680, match** |
| `057_Simple_balance_sheet` | 3/3 | 2185/1877 | **2/2, 1765/1765, match** |
| `065_Weight_loss_tracker` | 2/2 | 340/308 | **286/286, match** |
| `070_Equipment_inventory_list` | 1/1 | 1004/1029 | **646/646, match** |
| `071_Four-week_project_timeline` | 1/2 | 400/587 | unchanged (no frame to remove) |
| `075_Idea_planner_tasks` | 1/1 | 503/484 | unchanged (no frame to remove) |

(Every figure in that table is of the **frozen** workbook — `TODAY()` replaced by the serial its
own cache was computed at — so the dates are out of it.)

**Six rows are the chart and nothing else; one is a graphic frame that is not a chart; six are the
sheet and the chart has no part in them.** And in every one of the six chart rows the sign is the
same: **we draw more chart text than 26.2.4.2 does.**

## The thirteen, classified

| class | rows | |
|---|---:|---|
| **the sheet, not the chart** | **6** | `033` `045` `047` `053` `071` `075` — two of them hold no chart part |
| **the chart's tick or category labels** | **4** | `029` `030` `040` `055` |
| **the reference outlines what we draw as text (a measured ceiling)** | **2** | `057` `065` |
| **a slicer's fallback shape** | **1** | `070` |

`classify.tsv` is the row-by-row form with what decides each.

### The ceiling class, and why no fix can win those two rows

`057_Simple_balance_sheet` is the clean case. Its chartsheet's twenty category labels are long
account names on a horizontal axis; **we draw them as real text turned 45°** — 20 spans with a
`dir` of `(0.707, −0.707)` — and 26.2.4.2 draws **no turned text at all** on that page while
carrying **331 glyph-sized filled paths** against our 114. That is `dotnet/CLAUDE.md`'s shear rule
arriving on the sheets track: `VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D`
(`drawinglayer/source/processor2d/vclprocessor2d.cxx`:126-141) accepts a text primitive only while
`abs(fontScaling.getY() * fShearX) < 1` and decomposes everything else to filled polygons. Our
arrangement agrees; only the representation differs, and ours is the better output. The row's whole
+308 is that. `065_Weight_loss_tracker` is the same shape and less cleanly: 120 glyph-sized fills
on the reference's chart page against our 12, over a wider rectangle than any ink of ours.

**These two belong in the `ceiling-*` class, not in a chart class.** Closing them would mean
outlining glyphs to make a text-extraction gate greener.

### The four label rows are four different questions, not one

- `040`: the value axis' **automatic increment**. Ours 2 — 60 62 … 80 — against the reference's 5,
  60 65 70 75 80. Same minimum and maximum, eleven ticks against five.
- `030`: the same question with the sign the other way. Its two charts' value axes run 1000 … 8000
  in ours and 500 … 2500 in the reference, and we additionally draw category labels
  (`Total current assets`, `Liabilities and owner's equity`) the reference does not. Its charts are
  also worth **two pages** on both sides: 8/8 becomes 6/6 with them gone.
- `055`: a **label format**, not a scale. The chart writes `4/24/2023` where the reference writes
  `24 Apr`, the same thirteen dates either way.
- `029`: our charts draw a **legend** — `Income`, `Expenses` at x 330.9 and 402.9 on page 1 — that
  the reference does not draw at all, and a second chart's category labels likewise.

`ChartScale.Resolve` and `ChartLayout.IntervalsThatFit` are already a step-for-step port of
`ScaleAutomatism::calculateExplicitIncrementAndScaleForLinear` and
`VCartesianAxis::estimateMaximumAutoMainIncrementCount`, so `040` and `030` are a question about
the axis length and the label height those two are fed rather than about the arithmetic. Left with
their seat.

## What moved: a wrapping cell whose text begins outside its own column

This is `075_Idea_planner_tasks`, and it is a *sheet* rule that a chart batch was hiding.

Its `B6` holds `" Task Status Indicator"`, wraps, carries one indent level, and sits in a column
one character wide with `C6` occupied beside it. 26.2.4.2 draws **none** of it; this tree drew all
twenty-two characters on one line straight across `C` and `D`. That is the row's whole failure:
503 alphanumerics against 485.

### The rule, and the two halves of it

**Only a wrapping cell has a paper at all.** `DrawEditParam::calcPaperSize`
(`sc/source/ui/view/output2.cxx`:2684-2700) sets the EditEngine's paper to
`rAlignRect.GetWidth() − nLeftM − nRightM` and is called only under `if (rParam.mbBreak)`; a cell
that does not wrap keeps the initial `Size(1000000, 1000000)` and writes across whatever is beside
it. `calcMargins` (`:2665-2682`) adds `ATTR_INDENT` to whichever side the cell is aligned to, so an
indented cell in a narrow column reaches a **negative** paper.

**A negative paper does not stop the breaking.** `ImpEditEngine::calculateMaxLineWidth`
(`editeng/source/editeng/impedit3.cxx`:530-545) ends with `if (nMaxLineWidth <= 0) nMaxLineWidth = 1;`
— one unit, which no glyph fits — so every character takes a line of its own.

**What suppresses the cell is where the block starts, not the paper's sign.** The block is laid out
at the cell's left edge plus the left margin plus the indent; when that is at or past the column's
right edge, none of it meets the cell's own rectangle and
`ImpEditEngine::DrawText_ToRectangle` strips the portions and returns without emitting one —
`if (!aContentRange.overlaps(aClipRange)) return;` (`impedit3.cxx`:3408-3440).

### How the predicate was pinned, and the wrong one that fitted the first eight measurements

Eight column widths of the corpus workbook put the step between **1.5 and 1.75** characters, which
is 5.25 pt and 6.11 pt of column. Two predicates fit that alone — *the paper is not positive*
(cliff at margin + indent + margin = 6.69 pt) and *the block starts outside* (cliff at
margin + indent = 5.70 pt) — and the first is the one a reading of `calcPaperSize` suggests. It is
wrong, and `features/sheet-narrow-wrap.fods` is what says so: a 1.42 pt column with **no** indent
has a negative paper and 26.2.4.2 draws `N A R R O W` on six lines there. Four further variants of
the corpus workbook with `indent="1"` removed are drawn in full at every width. Twelve renderings
in all; the surviving predicate is `margin + indent ≥ column width`.

Three one-attribute variants say what does *not* decide it, and each was a live hypothesis:

| variant | 26.2.4.2 |
|---|---|
| `wrapText` removed | draws the whole string, at **our** x to 0.1 pt |
| the occupied neighbour `C6` emptied | unchanged — nothing to do with the spill rule |
| the row twenty times as tall | unchanged — nothing to do with `SkipOutsideFormat` |

### What it is worth

`SheetTextLayout.StartsOutsideItsCell` is the predicate and the clamp is in `Wrap`. After it:

- **21 of 21** one-attribute variants of the corpus workbook agree with 26.2.4.2 on pages and
  alphanumerics, including both sides of the 1.5/1.75 step.
- `features/sheet-narrow-wrap.fods` agrees glyph for glyph and position for position: six
  single-character lines at x = 57.685 against the reference's 57.685, pitch 11.196 pt.

## Confinement

**The original corpus, all 947, our half re-rendered against the banked 26.2.4.2 reference**
(`confine.py`, `after-rows.tsv` against `probes/orig-gate-r83/rows.tsv`):

| | |
|---|---|
| rows scored | 947 |
| documents whose page **or** alphanumeric count changed | **1** |
| that one | `075_Idea_planner_tasks`, 503 → 484 against the reference's 485, `words` → `match` |

so the words, slides, `.doc`, `.ppt`, `.xls` and `.pptx` tracks are all untouched, measured rather
than argued. The scoreboard goes **914 → 915 of 947**, `.xlsx` **226 → 227 of 241**.

*`confine.py` scores pages and alphanumerics and not `unembedded`, so its own totals read 916; the
second "movement" it reports, `vvsummit2022-Research-Roadmap…pptx`, is that missing column and not
a change in the tree. Compare its rows on columns 2 and 3, as the table above does.*

**The converted `.ods` column, before and after, both halves rendered at this base** — the r80 bank
is three rounds old, so its `ours` column is not a baseline and a binary was built at this round's
base to produce one (`before-ods.tsv` against `after-ods.tsv`, 296 rows scoreable on both sides):

| | before | after |
|---|---:|---:|
| match | 258 | **259** |
| documents whose counts changed | | **1** |

and that one is `075_Idea_planner_tasks.ods`, the same workbook's ODF twin, 512 → 493 against 485 —
inside the band, so it gains its verdict too.

**What this does not measure is ink.** Two full banks do not fit on this box, so every rendering
was scored and deleted. A cell that now takes a character per line where it took one long line
before changes positions without changing counts, and no column here would see it; the corpus
carries no such document that also changes a page or a character, which is as far as this goes.

## Files

| | |
|---|---|
| `devolatile.py` | freeze a workbook's `TODAY()`/`NOW()` at the serial its own cache was built with |
| `strip-charts.py` | remove every `xdr:graphicFrame` anchor, leaving the sheet alone |
| `sweep.py` | render a directory of workbooks both ways and score by the gate's rule |
| `confine.py` | re-score our half of a corpus against a banked reference, deleting as it goes |
| `worddiff.py`, `textdiff.py` | the token and span differences of two renderings |
| `outlines.py` | each side's glyph-sized filled paths, per page — the outlining census |
| `charttext.py` | what the charts themselves draw, as whole-minus-stripped |
| `idea-variants.py` | the one-attribute variants of `075`'s blocked header cell |
| `classify.tsv` | the thirteen rows, their class and what decides each |
| `frozen-rows.tsv`, `nochart-rows.tsv`, `frozen-vs-nochart.tsv` | the two scored sweeps |
| `after-rows.tsv`, `after-a.tsv`, `after-b.tsv` | the whole-corpus confinement run |
| `before-ods.tsv`, `after-ods.tsv` | the `.ods` column at this base, before and after |
| `variants.tsv` | the twenty-one one-attribute variants, before and after, against 26.2.4.2 |
| `frozen-worddiff.txt`, `outline-census.tsv`, `frozen-serials.tsv` | the censuses |

## Reproducing

```sh
export PATH=/opt/libreoffice26.2/program:$PATH        # 26.2.4.2, never /usr/bin/soffice
export SOURCE_DATE_EPOCH=1700000000
CLI=<worktree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli

python3 devolatile.py <workbook> frozen/<workbook>          # 045 needs its serial passed: 44984
python3 strip-charts.py frozen/<w> nochart/<w>
python3 sweep.py "$CLI" frozen out-frozen 1
python3 sweep.py "$CLI" nochart out-nochart 1
python3 charttext.py

python3 idea-variants.py && python3 sweep.py "$CLI" idea out-idea 1

cut -f1 ../orig-gate-r83/rows.tsv > corpus-list.txt
python3 confine.py "$CLI" corpus-list.txt out-confine 3 > after-rows.tsv
CORPUS=/home/user/corpus-odf BANK=/home/user/gate-odf-r80/ref \
  python3 confine.py "$CLI" ods-list.txt out-confine-ods 3 > after-ods.tsv
```

## Provenance

Measured 2026-09-08 in the `/home/user` container against `/opt/libreoffice26.2` 26.2.4.2 with the
tarball's Latin duplicates and `LiberationSansNarrow` moved aside, `fonts-dejavu-core` present.
`PROVENANCE.tsv` rows to add by hand — do not regenerate the index in this container.

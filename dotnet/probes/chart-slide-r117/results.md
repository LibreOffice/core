# O43 has no witness, O58 is half a rotated label's height, and O60's 3.3 pt is a trailing space

Round 117, seat `agent/chartslide`, based on `88209aa4b`. Reference **LibreOffice 26.2.4.2**
(`/opt/libreoffice26.2/program/soffice`). C++ read out of `/home/user/libreoffice-core`, which
declares `27.2.0.0.alpha0+` and is **not** the reference binary's source; every arm below says
whether it is a reading of that tree or a measurement of 26.2.4.2.

Every reference render in this round was taken **twice** and compared (C11). Every rendering of
ours in §1.4 and §2.4 was taken with `SOURCE_DATE_EPOCH=1700000000` pinned, and both legs on
2026-09-13 UTC (C13).

## 0. What this round settles

| Question | Answer |
|---|---|
| Does any corpus document witness O43's two-word wrap restart? | **No.** Of the 176 chart-bearing documents, 15 axes are even in the configuration where it could fire; 7 of them wrap; 6 of the 7 already restart in this tree and come out turned 45°, which is what the reference draws; the one that stays wrapped upright sits **41 % of a pitch** clear of the reference's own restart threshold, and the reference wraps it too. §1 |
| Then what closes O43? | Nothing to implement. It is a **nil-reach** entry and is closed as one. §1.5 |
| O58 — what does a turned value label reserve at the axis' far end? | **`h·sin(θ)/2`** — half the label's line height, with **no width term at all**. Measured on `027` with the value axis' scale pinned: widening every label from 19.18 pt of ink to 40.69 moves the drawn plot width by **0.02 pt** in 8 of 8 variants. §2 |
| Does it close `027`? | Yes. Its savings chart's axis goes **108.17 → 115.664** against the reference's **115.666**, from a left edge that already agreed at 645.05. §2.5 |
| O58's reach | **1 of 176** chart-bearing renderings moves and the other 175 are byte-identical; **0** page counts and **0** alphanumeric counts move, so no gate column can see it. §2.6 |
| O60 — is the `.ppt` table cell's measure ~3.3 pt narrower here? | **No, and the seat's own instrument is what said so.** `measure3.py` adds the last glyph's design advance, and on the reference the last glyph of a justified line is a **space**. Excluding it, `architecture6` page 10 breaks at **exactly the same words on all 12 lines** as the de-confounded reference. §3 |
| What is left of O60 | One break on **page 14**, and the measure gap that flips it is bounded in **[0.021, 13.19) pt** — with the reference's *own* declared column arithmetic (400.88 pt) landing on **our** side of it. §3.4 |
| Tests | 3 added, **2 of them failing at the base**; one existing fixture re-tuned with its reason. 6821 unit tests in ten projects, 0 failed, 0 skipped; `XRay.Office.Fidelity.Tests` **Failed 10, Passed 542, Skipped 0, Total 552** — the banked ten. §4 |

---

## 1. O43 — the census, and it finds nothing

### 1.1 What had to be counted

Round 111 characterised the two-word restart over 20 sweeps and declined to implement it; round 112
then read the three documents the row was opened for and found **none of them a two-word wrap
case**. So the row claimed a mechanism with no known witness. The question this round asks is
whether the *corpus* holds one.

The mechanism can only change a rendering where all of the following hold at once, and each is a
condition of `createTextShapes`'s own restart (`chart2/source/view/axes/VCartesianAxis.cxx`:888-903,
read in the 27.2 tree):

```
if (nLimitedSpaceForText > 0
        && !rAxisLabelProperties.m_bOverlapAllowed
        && rAxisLabelProperties.m_fRotationAngleDegree == 0.0
        && nTick > 0
        && lcl_hasWordBreak(pTickInfo->xTextShape))
```

* the axis is **horizontal** (a vertical one takes the whole band beside it and no reduction);
* **`TextBreak` is on** and **`TextOverlap` is off** and the stated rotation is **zero**;
* the label is at **tick index ≥ 1** — `nTick > 0`;
* the label **actually wraps**, because a label that fits its slot never breaks at all;
* and *we* do **not** restart, i.e. neither the width arm (a word wider than `0.95 × pitch`) nor
  the hyphenation arm of `ChartAxisLabels.Wraps` fires.

Under those, the reference restarts when round 111's best-supported form
`1.05·L + 0.10·R + c > 0.95·spacing` holds, with `L` the widest word, `R` the sum of the others and
`c ∈ (−0.183, −0.070]`; this tree restarts only when `L > 0.95·spacing`. The reference's condition
is strictly the weaker of the two, so the divergence is one-directional: **the reference turns the
axis where we wrap it.**

### 1.2 The instrument

`axis-census.py` runs the CLI over the 176 chart-bearing corpus documents
(`probes/chart-fit/census.tsv`) with a temporary trace patched into
`ChartAxisLabels.Resolve` — banked beside this file as `axis-trace.diff` and **reverted**, not
committed to `src`. The trace emits one `AXTRACE` row per multi-word label per resolve, with the
axis' direction, tick spacing, em size, the three stated flags, the label's whole width, its widest
word and the sum of the others; and one `AXRESULT` row per axis that returns, with the arrangement
it settled on and how many of its labels were drawn wrapped. Rows are deduplicated — `Resolve` runs
once per attempt and once per chart copy.

```sh
tail -n +2 probes/chart-fit/census.tsv | cut -f1 > /tmp/chartdocs.txt
python3 probes/chart-slide-r117/axis-census.py \
    --cli tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli \
    --out axis-rows.tsv --results axis-results.tsv --jobs 2 < /tmp/chartdocs.txt
```

**176 of 176 rendered, none failed.** `axis-rows.tsv` holds 415 label rows and
`axis-results.tsv` 193 axis rows in 96 documents.

### 1.3 The census

| | |
|---|---:|
| chart-bearing corpus documents | 176 |
| axes reaching `ChartAxisLabels.Resolve` | 193 in 96 documents |
| … carrying a label with a blank at tick index ≥ 1 | 103 rows in 14 documents |
| **… horizontal, `TextBreak` on, `TextOverlap` off, stated rotation 0** | **15 axes** |
| … of which the label actually wraps (`whole > 0.95 × pitch`) | **7** |
| … whose *final* arrangement still draws a wrapped label | **1** |

The four axes in the whole corpus whose final arrangement keeps a wrapped label are:

| document | pitch | overlap | worst slack against the reference's restart form |
|---|---:|---|---:|
| `TOGAF9-Tool-ConfReqts-CSQ.xls` | 56.05 | **allowed** | −13.14 pt |
| `orbus_togaf_tool_csq.xls` | 60.20 | **allowed** | −8.57 pt |
| `014_Contextures_chart_sample_991ecfc5.xls` | 59.55 | **allowed** | +31.18 pt |
| `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 27.48 | off | **+11.36 pt (41.3 % of the pitch)** |

Three of the four allow overlap, and the restart is gated on `!m_bOverlapAllowed` in the reference's
own source — so the mechanism cannot reach them however negative the arithmetic looks. The fourth is
the only corpus axis the seat could have moved, and it is 41 % of a pitch clear.

The other six wrapping axes already restart here and are drawn turned 45°, which is the arrangement
the reference draws: `038_Competitive_Advantage_Card` (hyphenation),
`033_Event_planning_tracker` at both of its pitches, `046_Cost_analysis_with_Pareto_chart`,
`023_Waterfall_Chart_Template` (width arm) and `057_Simple_balance_sheet`.

### 1.4 How far the pitch would have to be wrong

Our tick pitch is our own layout's, so the census is only as good as it. For each eligible axis the
band of pitches in which *the reference restarts and we do not* is
`[max_L/0.95, min_over_labels((1.05·L + 0.10·R − 0.13)/0.95)]`, capped at `whole/0.95` below which no
break happens at all:

| axis | our pitch | band | in band? |
|---|---:|---|---|
| `038_Competitive_Advantage_Card` | 54.20 | 53.00 … 58.36 | **yes**, and it restarts anyway through hyphenation |
| `023_Waterfall_Chart_Template` | 34.00 | 35.36 … 40.02 | no — **+4.0 %** away |
| `033_Event_planning_tracker` | 67.49 / 72.46 | 57.04 … 63.17 | no, −6.4 % and −12.8 % |
| `039_Baby_growth_tracker` | 57.37 | 38.66 … 41.76 | no, −27 % |
| the other nine | | | no, −36 % to −79 % |

So one axis is inside the band on the width arm alone and restarts for a different, already-modelled
reason; the next nearest would need its pitch to be **4 % larger** before the question could arise.

### 1.5 Confirmed at the binary

Three single-document readings, each rendered **twice** by 26.2.4.2 and pixel-identical between the
two runs.

* **`038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx`** — the census'
  only in-band axis. The reference draws all five category names **turned 45° on one line each, as
  outlines** (they are absent from `get_text` entirely, which is the shear rule); so does this tree.
  I read both croppings myself, at 3× on the chart region; **this is my own reading and no
  independent reader was available** (see §5).
* **`8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` page 16** — the one axis in the corpus whose final
  arrangement keeps a wrapped label with overlap off. The reference draws
  `[08 h ; ` / `10 h[` on **two upright lines per label**, at a tick pitch of **27.5 pt** against our
  27.4769, with nine labels at 445.8, 473.3, 500.7, … — exactly our arrangement. It does not restart.
* **`014_Contextures_chart_sample_991ecfc5.xls`** — an overlap-allowed axis. Its two multi-word
  category labels are drawn on **one upright line each** at 221.5 and 343.6; ours are at 221.2 and
  343.7. No restart on either side.

### 1.6 The finding

**O43's two-word restart has no witness in the 947-document corpus.** It is a real threshold of
26.2.4.2 — round 111's 20 sweeps stand and are not re-derived here — but nothing this project
renders can see it, and the only axis that could have has 41 % of a pitch of margin. The row is
closed as a nil-reach entry.

Two limits of the census, stated rather than buried. It is a claim about **the seven extensions the
corpus holds**; the converted-ODF column was not swept, and its `.odp` charts go through the same
`ChartAxisLabels.Resolve`, so a document there could in principle differ. And it uses **our** pitch,
which §1.4 bounds rather than removes.

---

## 2. O58 — a turned value label reserves half its height, and no width at all

### 2.1 The site, read in the 27.2 tree

`createSingleLabel` (`VCartesianAxis.cxx`:122-151) builds the label at the tick with
`ShapeFactory::makeTransformation(anchor, −angle)` and then calls
`LabelPositionHelper::correctPositionForRotation`. For a bottom axis that is
`lcl_correctRotation_Bottom` (`chart2/source/view/main/LabelPositionHelper.cxx`:241-256), whose
`0 < θ ≤ 90` branch is

```cpp
rfXCorrection = -aSize.Height*std::sin( fAnglePi )/2.0;
if( !bRotateAroundCenter )
    rfXCorrection -= aSize.Width *std::cos( fAnglePi )/2.0;
```

and whose `fAnglePositiveDegree==0.0` branch is **empty**.

The piece that makes the arithmetic come out is `makeTransformation`'s own comment — *"As autogrow
is active the rectangle is automatically expanded to that side to which the text is not adjusted"*.
A bottom-axis label is centre-adjusted, so **before** the correction the box's top *centre* is on the
tick, and rotating it about that point puts its right edge at `w·cos/2 + h·sin`. The correction then
takes `(w·cos + h·sin)/2` off it:

```
right edge − tick  =  w·cos/2 + h·sin − (w·cos + h·sin)/2  =  h·sin/2
```

**with no `w` in it.** Upright, no correction runs and the centred box's plain `w/2` stands.

### 2.2 Measured at the binary, and the width term is refuted by a sweep

`turn027.py` rewrites **one thing at a time** in `027_Simple_personal_cash_flow_statement`'s
savings chart (`xl/charts/chart44.xml`), a bar chart whose money axis runs along the bottom and
whose labels 26.2.4.2 turns 45° because they collide. Every variant pins the scale
(`c:min` 0, `c:max` 14000, `c:majorUnit` 2000) so the tick set cannot move and the
automatic-increment cap of O51 cannot enter.

The instrument is **the chart's own longest bar**: its value is 12,000 of a pinned 14,000, so the
plot's width is `bar × 14/12` read off a filled path, with no label in it.

```sh
python3 probes/chart-slide-r117/turn027.py write <dir> "base|-2700000" "w6|-2700000" \
    "base|0" "w6|0" "base|-5400000" "w6|-5400000" "none|-2700000" "none|0"
/opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir <dir>/ref <dir>/src/*.xlsx
python3 probes/chart-slide-r117/read58.py <dir>/ref
```

| variant | plot left | plot width | plot right | last label | its ink | ink right | past the tick |
|---|---:|---:|---:|---|---:|---:|---:|
| **no tick labels** | 645.05 | **117.759** | 762.81 | — | — | — | — |
| 45°, `"$"#,##0` | 645.05 | **115.666** | 760.72 | `$14,000` | 19.18 | 762.72 | **2.00** |
| 45°, `…"WWWWWW"` | 645.05 | **115.666** | 760.72 | `$14,000WWWWWW` | **40.69** | 762.71 | **1.99** |
| 90°, `"$"#,##0` | 645.05 | 114.780 | 759.84 | `$14,000` | 5.94 | 762.73 | 2.89 |
| 90°, `…"WWWWWW"` | 645.05 | 114.780 | 759.84 | `$14,000WWWWWW` | 5.94 | 762.73 | 2.89 |

**The label's ink more than doubles and the plot width moves by 0.000.** Over the fuller sweep of
seven number formats the widest movement is 0.02 pt (`n4`, 115.647), where half the width would have
moved it 10.8. And every one of the eight labels' right edges sits **2.00 pt past its own tick** —
at `$0` as at `$14,000`, and at the wide format as at the plain one, so the whole widening goes to
the left.

### 2.3 The constant is `h·sin(θ)/2`, over three sizes and four angles

The **no-tick-labels** variant is the control that turns the reserve into a directly readable
number: with `tickLblPos="none"` nothing is reserved, so `availableRight = 762.81` and
`reserve = 762.81 − plotRight`.

| stated size | 22.5° | 45° | 67.5° | 90° |
|---|---:|---:|---:|---:|
| 6 pt | | 1.35 | | 1.92 |
| 9 pt (as authored) | 1.15 | 2.09 | 2.82 | 2.97 |
| 14 pt | | 3.33 | | 4.69 |

Dividing each by `sin(θ)` gives an `h/2` of **3.005, 2.956, 3.052 and 2.970** across the four angles
at 9 pt — consistent to **1.6 %** — and the three sizes are linear in the em: 0.225, 0.232 and
0.238 page-points of reserve per stated point at 45°, against 0.320, 0.330 and 0.335 at 90°, whose
ratio is 0.703–0.711 against `sin 45° = 0.7071`.

The page is drawn at a uniform 0.567222 (the workbook's print scale — the drawn label size is 5.105
for a stated 9), so in the chart's own units `h` comes out at **1.12–1.19 em depending on size**,
which is the 96 dpi device's own line height and exactly the shape `MetricGrid.Chart` already
carries.

### 2.4 The upright control

The same chart with `c:majorUnit` pinned to 14,000 draws **two** labels, which do not collide and are
therefore not turned:

| format | last label's ink | its centre | plot right | reserve |
|---|---:|---:|---:|---:|
| `"$"#,##0` | 21.18 | 752.18 | 752.18 | 10.63 |
| `…"WW"` | 31.31 | 747.11 | 747.11 | 15.70 |
| `…"WWWW"` | 41.46 | 742.03 | 742.04 | 20.77 |
| `…"WWWWWW"` | 51.60 | 737.33 | 736.97 | 25.84 |

**The label is centred on its tick to 0.04 pt at four widths and the reserve is half its width**, so
the corrected rule is not "the overhang is gone" — it is two rules, exactly as the source's own
`if (angle == 0.0) {}` is.

### 2.5 Implemented

`ChartLayout.PlotAreaOf`'s bar branch:

```csharp
right -= valueTurn == 0.0
    ? valueLabel / 2.0
    : (valueHeight * valueSin) / 2.0;
```

in place of `((valueLabel * valueCos) + (valueHeight * valueSin)) / 2`. No constant is added.

On `027` page 6, measured the same way on both sides — the plot width read off the longest bar:

| | plot left | plot width | plot right |
|---|---:|---:|---:|
| 26.2.4.2 (twice, pixel-identical) | 645.05 | **115.666** | 760.72 |
| this tree, base | 645.05 | 108.17 | 753.22 |
| this tree, after | 645.05 | **115.664** | 760.72 |

The 7.51 pt O58 was opened for is **0.002 pt** after it.

### 2.6 Reach

`probes/chart-collide-r110/confine.py` over the 176 chart-bearing documents, twice — once with the
CLI built at `88209aa4b` and once from this diff — with `SOURCE_DATE_EPOCH=1700000000` pinned, both
legs on 2026-09-13 UTC:

```sh
export SOURCE_DATE_EPOCH=1700000000
for leg in base after; do
  python3 probes/chart-collide-r110/confine.py --out /tmp/conf-$leg \
      --cli /tmp/cli-$leg/XRay.Office.Cli --rows rows-$leg.tsv --jobs 2 < /tmp/chartdocs.txt
done
```

**1 of 176 renderings moves and it is `027`; the other 175 are byte-identical**, none failed on
either leg, and **no page count and no alphanumeric count moves anywhere** — so no gate column can
see this and the gate's silence is the expected result rather than a null one. `reach-base.tsv`,
`reach-after.tsv`.

The unpinned trap is worth restating because it was checked: with `SOURCE_DATE_EPOCH` unset the same
sweep reports every rendering as moved and means nothing.

### 2.7 What the same reading predicts and this round did not implement

The near end. The same algebra puts the *first* label's left edge at `tick − (w·cos + h·sin/2)`,
which is far more than the upright `w/2`, and `PlotAreaOf`'s bar branch reserves nothing at all for
the value labels on the left. It cannot be witnessed on `027`, whose left edge is set by the much
wider category names and did not move by a hundredth of a point across any of the twenty variants
above. **Not implemented; measure the reach before spending a round.**

---

## 3. O60 — the 3.3 pt is a justified trailing space

### 3.1 What the seat says and what the instrument does

O60 records `architecture6.ppt` page 10's body cell as justifying to a pen end of 681.63–682.33 in
the reference against 678.54–678.58 here, from a common left edge of 199.05 — a measure of ~482.85
against 479.51. Both figures reproduce exactly:

```sh
python3 measure3.py dca/dc.pdf 10 199.05 ref-declass   # 681.63 682.24 681.97 682.33 681.72 …
python3 measure3.py ours/d.pdf 10 199.05 ours          # 678.57 678.56 678.56 678.58 678.58 …
```

`measure3.py` takes each line's last character's origin and adds **that character's own design
advance**. On the reference the last character of a justified line is a **space** — the reference
draws it, this tree does not — and the space's design advance at 14 pt Liberation Sans is
`0.27783 × 14 = 3.890 pt`. That is the whole of the 3.3.

### 3.2 The same page, with trailing spaces excluded

De-confounded exactly as round 115 did it — `declass.py`, which clears the family nibble of all
seven `FontEntityAtom`s so the seventh confound cannot contribute — and rendered **twice**, the two
runs identical in every page's text:

| | 26.2.4.2, class cleared | this tree |
|---|---|---|
| lines in the cell | 12 | 12 |
| baselines | 197.9, 214.8, 231.6, 248.4, 265.2, 282.0, 320.2, 355.5, 372.4, 407.9, 424.7, 460.3 | **identical** |
| line 1 | `structured into three logical components that interact with each other. The ` | `…other. The` |
| last **non-space** glyph's end, justified lines | 677.74, 678.32, 678.07, 678.41, 677.80, 677.83, 677.85 | 678.57, 678.56, 678.56, 678.58, 678.57, 678.54, 678.54 |
| … unjustified lines | 425.73, 407.83, 259.04, 462.35, 343.13 | 425.30, 407.64, 258.98, 462.13, 343.03 |

**Every one of the twelve lines breaks at the same word.** The unjustified lines — which carry no
trailing space and no justification stretch — agree to **0.06–0.44 pt**, which is round 115's own
"0.21–0.45 pt" for the short lines, and there is nothing else. The justified lines differ by 0.5 pt
in the other direction because the reference distributes its stretch over the trailing space too.

**Page 10's measure was never evidence of anything either way.** Bracketing it from the break points
— the widest line that fits and the narrowest that is refused, at Liberation Sans 14.003 pt —
gives `[474.82, 489.61)`, a **14.8 pt** window, and both candidate arithmetics (490.32 − 2 × 5.414 =
479.49, and the same less the two 0.23 pt borders = 479.03) sit comfortably inside it.

### 3.3 And the as-authored render is not comparable at all

Rendered as authored, the reference's page 10 breaks at **quite different words** — 59 to 66
characters a line against our 65 to 76 — because O57's confound has it measuring in DejaVu Sans and
drawing Liberation Sans. Bracketing that render's breaks with **Liberation** metrics gives a measure
of `[413.2, 416.3)`, sixty-five points narrower than anything real. **A break-point bracket is only
valid in the face the layout actually measured in**, which on this deck is not the face the PDF
names. That is the de-confounding check the brief asked for, and it passes: round 115's §6 table was
taken on the de-classed variant and reproduces.

### 3.4 What is actually left, and where

Diffing all 31 pages of the de-confounded reference against ours line by line
(`pagelines.py`), the residual is:

* **page 14 — two break decisions**, and they are the "one wrapped word" the row names. The
  reference breaks `…provides services to the` / `layer above it…`; we fit `layer` on the first
  line. Same for the second cell (`…several teams with` / `each team…`).
* **pages 21 and 24** — no break moves. What differs is where the *words inside one justified line*
  fall, by up to 1.6 pt, and a 0.1 pt baseline drift.
* **page 27** — no text difference at all.
* every page — the footer sits at y 508.1 in the reference and 508.3 here, and `Chapter 6
  Architectural design` at x 285.9 against 286.3.

Page 14's measure, bracketed from each side's own break points at Liberation Sans 14.003 (valid
here, the class being cleared) over all five paragraphs of the cell:

| | widest line that fits | narrowest refused | measure in |
|---|---:|---:|---|
| 26.2.4.2, class cleared | 394.648 | 400.829 | **[394.648, 400.829)** |
| this tree | 400.850 | 407.837 | **[400.850, 407.837)** |

The two are **disjoint and adjacent**: we are wider by at least **0.021 pt** and at most 13.19.

And the reference's own resolved view of the file settles which of us is following its declared
geometry. `soffice --convert-to fodp` writes page 14's table as a frame at `svg:x="2.844cm"` with
columns `5.451cm` and `14.525cm` and every cell at `fo:padding-left="0.191cm"
fo:padding-right="0.191cm"`:

```
column 2 left  = 80.62 + 154.49            = 235.11 pt
text left      = 235.11 + 5.414            = 240.53 pt   (drawn at 240.5 on both sides)
inner measure  = 411.72 − 2 × 5.414        = 400.88 pt
```

**400.88 is inside our interval and outside the reference's**, by 0.05 pt. So the reference's
effective measure is a little under its own declared column arithmetic — `Cell::TakeTextAnchorRect`
(`svx/source/table/cell.cxx`:642-649) subtracts the four text distances and nothing else, so the
27.2 tree does not explain where the 0.05 goes, and the two 0.23 pt borders would account for it but
are not in that function. **Nothing was implemented for it.** Subtracting a border to make one break
flip is a constant fitted to one line, and the gap is smaller than the instrument's own agreement
with the unjustified lines on page 10.

### 3.5 The finding

O60 is **restated, not closed**: its 3.3 pt is withdrawn, its page-10 evidence is withdrawn, and
what remains is a **sub-0.06 pt** measure difference that flips one break on page 14 and nothing
else on the deck's other 30 pages. It is no longer "0.69 % of the measure"; it is 0.012 %.

---

## 4. Tests

Three added, in `tests/XRay.Office.Core.Tests/ChartTurnedValueLabelOverhangTests.cs`:

| test | at `88209aa4b` | after |
|---|---|---|
| `WideningATurnedValueAxisLabelDoesNotMoveThePlotsFarEdge` | **fails** | passes |
| `ATurnedValueAxisGivesUpHalfItsLabelHeightTimesTheSine` | **fails** | passes |
| `AnUprightValueAxisStillGivesUpHalfItsLabelWidth` | passes | passes |

The third is the mutation control: without it the corrected rule is indistinguishable from deleting
the overhang, and it is the one that would fail if `valueTurn == 0.0`'s branch were dropped.

**One existing fixture was re-tuned and it is not a green-washing.**
`ChartValueAxisArrangementTests.AHorizontalValueAxisTurnsItsLabelsWhenTheyCollide` builds a
synthetic axis on a 420 pt frame and asserts that a colliding axis turns. The corrected reserve
widens the plot rectangle the collision is then decided in, and on that ruler 420 sat within a few
points of the boundary: swept over 300…460 pt the axis turns at 360 and 380 under **either** rule
and at 420 under the old one only. The assertion and the rule under test are unchanged; the frame is
380 with the reason written beside it.

Totals, projects run individually (`dotnet test XRay.Office.slnx` is the one most likely to truncate):

| project | passed | failed | skipped |
|---|---:|---:|---:|
| Containers | 109 | 0 | 0 |
| Core | 591 | 0 | 0 |
| Markup | 259 | 0 | 0 |
| OpenDocument | 160 | 0 | 0 |
| Presentations | 1203 | 0 | 0 |
| Rendering | 164 | 0 | 0 |
| Spreadsheets | 1360 | 0 | 0 |
| Text | 728 | 0 | 0 |
| Vector | 309 | 0 | 0 |
| WordProcessing | 1938 | 0 | 0 |
| **total** | **6821** | **0** | **0** |
| `XRay.Office.Fidelity.Tests` | 542 | **10** | 0 (of 552) |

The ten are the banked ten — the same count round 113 and round 115 report, `SheetDrawingComparisonTests` among them.
Build is warning-free.

## 5. Which conclusions rest on my own reading

**No blind reader was available** — a subagent here has no subagent tool, which four rounds have now
established, and `create_session` spawns a sibling in another container that cannot open
`/home/user/...`. Two page readings in this round are therefore mine and are contaminated by knowing
what I expected:

* `038`'s chart region at 3×, reference and ours side by side (§1.5). Everything it is used for is
  corroborated by arithmetic that does not depend on it: the reference's category names are absent
  from `get_text` while ours are present and turned, and our own `AXRESULT` row says rotation 0.7854.
* nothing else. §2 and §3 are read entirely out of PDF content streams and drawing operators.

`architecture6` page 14 de-confounded and `pres_ioc_phuket.ppt` are both still named as deserving an
independent reader, and this round adds no third.

## 6. Files

| file | what |
|---|---|
| `axis-census.py` | the O43 census driver |
| `axis-trace.diff` | the temporary `ChartAxisLabels.Resolve` trace it needs, reverted after use |
| `axis-rows.tsv` | 415 multi-word label rows over 176 documents |
| `axis-results.tsv` | 193 resolved arrangements |
| `turn027.py` | the O58 variant writer and reader — number format, angle, size, major unit |
| `read58.py` | the plot-width reader for a variant directory |
| `reach-base.tsv`, `reach-after.tsv` | the 176-document confinement, both legs |
| `cellbreaks.py` | one line per drawn line, with the last non-space glyph's end |
| `pagelines.py` | a line-by-line diff of one page of two PDFs |

# Round 117 — a border's dashes are absolute, and a chart's rules were stroked at zero because the theme's line styles never reached two of the three readers

Seat: **O59** (*a cell border's dash lengths are absolute and this tree derives them from the
line width*) and **the unseated chart-rule half of round 115's residual**. Reference throughout is
**26.2.4.2** (`/opt/libreoffice26.2/program/soffice`). Base commit `88209aa4b`, branch
`agent/strokeresid`. Every measurement below is dated 2026-09-13 and both legs of every
before/after pair were rendered on that same UTC day, which is what C13 requires.

**Seat numbers taken:** the chart work is recorded as **O63** in the register and the two residuals
it leaves as **O61** and **O62**. A parallel round is live; if it took any of those, the parent
resolves the collision on merge.

---

## 0. Summary, measured separated from inferred

**Measured.**

- **O59 is real, and its reach is small.** A border's dash array is `svtools`' fixed table times
  the border's *pattern scale*, which for a Calc cell is the twips-to-1/100 mm factor and has
  nothing to do with the line's width — one table unit is ten twips, half a point. Confirmed in
  source, on a purpose-built fixture and on a corpus document at a fitted scale of 0.40002.
  **12 of 307 sheets documents (3.9 %) state a patterned border at all**, and the two independent
  censuses that say so agree document for document.
- **Round 115's diagnosis of the chart half is wrong, and the real mechanism is bigger.** It is not
  that "we stroke a chart's rules at width 0 where the reference strokes them" in the sense of a
  weight disagreement: the spreadsheets and words chart readers were handed a **null theme format
  matrix**, so every axis line, tick and gridline that states no width of its own was resolved to
  **zero**. The slides reader was already given one; the other two were not.
- **And a DrawingML line width is a whole hundredth of a millimetre, not the EMU the file
  states.** `convertEmuToHmm` rounds half up, so 9525 EMU (0.75 pt) is 26 and is drawn at
  0.73701 pt. Six independent widths out of the reference's own `--convert-to ods` of three corpus
  workbooks, and four more out of its PDF of two purpose-built fixtures, agree.
- **A chart marker is filled *and* stroked, in its own fill colour.** The reference emits two
  operations per marker; this tree emitted one.
- **And a series' `a:ln` with no `w` is neither the stated nor the automatic width but a flat
  35/100 mm — while a *data point*'s is a hairline.** Seven one-variable measurements, §4.4.
- **The ink census itself was measuring the wrong thing on this class.** "Mean stroke width" scores
  a `0 w` hairline as nothing, and dropping hairlines brings ten of round 115's seventeen residual
  documents inside 6 % of 1.000. Seated as confound **C14**.
- Reach, `SOURCE_DATE_EPOCH` pinned, both legs rendered on the same UTC day: see §6.2.
- **No gate column can move and none did.** A stroke width, a dash pattern and a marker outline
  add no page and no alphanumeric character. The coordinator's whole-corpus gate on round 115's
  border change — 120 of 307 renderings changed, **0 verdicts moved** — is the measurement that
  settles the class, and this round is the same class.

**Inferred, and said so.**

- **Where 35 hundredths of a millimetre comes from.** It is measured seven times and located
  nowhere in the 27.2 tree; the rule is empirical and is labelled so in the code.
- **The per-point line width.** §4.4's data-point rule is applied to the whole series, which is
  exact only where every point states an `a:ln`. That is the shape both witnesses have, and no
  corpus document was found in the mixed case — but "not found" here is "not censused".
- **Nothing rests on a visual reading.** No blind reader is available in this container — a
  subagent has no subagent tool, established four times — and I did not open a page myself. The
  page I would name for an independent reader is §7.4's: page 1 of
  `084_Service_invoice_Use_this_template`, where the reference underlines `jordan@example.com` and
  we draw nothing. Every other claim here is a number out of a content stream or out of the
  reference's own resolved model.

---

## 1. Census before building — half (a), the dashes

**Question.** How many corpus spreadsheets state a dashed or dotted cell border at all?

Two instruments, deliberately independent.

`dash-census.py` reads the *statement*: `xl/styles.xml`'s `<borders>` for the zip formats, and the
`XF` records of the `Workbook` stream for the OLE ones, with a BIFF walker written for this
(`XF` 0x00E0, plus the cell records that reference an `XF` index) rather than borrowed. It reports
"stated in the style table" and "referenced by a cell, row or column" separately.

`dash-census-ods.py` reads the *reference's own resolution*: `/home/user/corpus-odf/sheets` is
26.2.4.2's `--convert-to ods` of the whole sheets track, so its `fo:border` values are what the
reference itself resolved, through one code path for `.xlsx`, `.xlsm` and `.xls` alike.

| | documents | rate |
|---|---:|---:|
| sheets track | 307 | — |
| …that state a patterned border (statement census) | **12** | **3.9 %** |
| …that reference one from a cell, row or column | 9 | 2.9 % |
| …that the reference resolves to a patterned `fo:border` | **12** | **3.9 %** |
| whole corpus | 947 | — |
| …sheets documents with one | 12 | **1.3 %** |

**The two censuses name the same twelve documents.** The three where the statement census finds a
pattern only in a `<dxf>` — a conditional format — are among them, which is the reference resolving
a conditional border that this tree does not read at all (`XlsxConditionalFormats` reads fills and
`XlsxConditionalStyles` fonts; neither reads a border). That is a separate absence and is not this
seat.

**Which patterns, by document, out of the reference's own ODS:** `dotted` 9 documents,
`fine-dashed` 4. **`dashed` — the `[8.0 2.5]` array — has zero corpus reach**, as do `dash-dot`
and `dash-dot-dot`. So four of the five arrays this round corrects are exercised only by the
fixture, and saying otherwise would be claiming reach that is not there.

`dash-census.tsv`, `dash-census-ods.tsv`.

### 1.1 What the corpus is, and is not

The corpus is seven extensions and holds no ODF and no `.ods` original, so "12 of 307" is a claim
about `.xlsx`, `.xlsm` and `.xls`. The ODS column above is 26.2.4.2's *conversion* of those same
307 documents, not an independent population — it is a second reading of one corpus, which is what
makes it a good cross-check of the instrument and a poor second sample.

Nine of the 307 files carry an `.xls` extension over zip bytes; the census reports container
rather than extension, and the split is 252 zip and 55 OLE.

## 2. The dash mechanism, confirmed twice

**In source (27.2 tree, so *probably* also 26.2's).** `ScDocument::FillInfo` builds every cell
border as `svx::frame::Style(pBox->GetLeft(), fColScale)`
(`sc/source/core/data/fillinfo.cxx`:1144-1147), whose second argument lands in `mfPatternScale`.
`CreateBorderPrimitives` then asks for
`svtools::GetLineDashing(rBorder.Type(), rBorder.PatternScale() * fPatScFact)` with
`fPatScFact = 10.0` (`svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx`:599-601), and
`GetDashing` is a table of small integers (`svtools/source/control/ctrlbox.cxx`:248-283):

| style | table |
|---|---|
| `DOTTED` | 1, 2 |
| `FINE_DASHED` | 6, 2 |
| `DASHED` | 16, 5 |
| `DASH_DOT` | 16, 5, 5, 5 |
| `DASH_DOT_DOT` | 16, 5, 5, 5, 5, 5 |

For a print or a PDF export `fColScale` is the constant twips-to-1/100 mm factor 2540/1440 =
1.76389 (round 115's arm 1, `ScPrintFunc::InitModes`, `printfun.cxx`:2627-2628), so one table unit
is `10 × 1.76389` hundredths of a millimetre — **ten twips, half a point** — and the map mode's
zoom multiplies it afterwards exactly as it multiplies the width.

**Against 26.2.4.2's own output.** Round 115's thirteen-style fixture gives `[0.49999 0.99998]`,
`[2.99995 0.99998]`, `[7.99987 2.49996]` and the two dash-dot extensions at 100 %, and the same
five arrays times 0.42 at `pageSetup scale="42"`. That is 0.5 pt per unit at both scales.

**And on a corpus document rather than a fixture**, which is the leg round 115 did not have. Page 1
of `079_Org_charts_visual_248d1dcf.xlsx` is a `fitToPage` sheet; the reference draws its dotted
borders as

```
0.29481 w   [ .20001 .40001 ] 0
```

The fitted scale is `0.20001 / 0.5 = 0.40002`. The width is then `26/100 mm × 0.40002 = 0.29482`
against the measured 0.29481 — the patterned-width rounding round 115 established — and the dash is
`[0.5 1.0] pt × 0.40002 = [0.20001 0.40002]` against the measured `[0.20001 0.40001]`. **Two
quantities, five figures each, one free parameter shared between them and read off the first.**
This tree drew `0.75 w [ .75 .75 ] 0` there before round 115 and `0.29481 w [ .29481 .29481 ] 0`
after it; it now draws `0.2948 w [ .2 .4 ] 0`.

**One detail worth keeping.** The dash lengths are *not* rounded to a whole logic unit and the
width is: `VclMetafileProcessor2D::processPolygonStrokePrimitive2D` writes
`std::round(getTransformedLineWidth(...))` for the width and a bare `getTransformedLineWidth(...)`
for `SetDashLen` and `SetDistance`
(`drawinglayer/source/processor2d/vclmetafileprocessor2d.cxx`:1818-1874). The measured pair above
shows both halves of that in one line.

## 3. Census before building — half (b), the chart rules

**Question.** How many corpus documents hold a chart whose axis line or gridlines state no width
of their own, and therefore take the *automatic* format?

`chart-census.py` walks every `charts/chart*.xml` part in every zip document of all three tracks
and classifies each `c:catAx`/`c:valAx`/`c:dateAx`/`c:serAx` and each `c:majorGridlines` /
`c:minorGridlines` by whether its `c:spPr/a:ln` carries a `w`. It also reports the theme's first
`a:lnStyleLst` width, which is where an automatic width comes from.

| track | documents | zip | binary | chart-bearing | ≥1 automatic width | ≥1 stated width |
|---|---:|---:|---:|---:|---:|---:|
| sheets | 307 | 252 | 55 | 92 | **26** | 68 |
| slides | 302 | 253 | 49 | 67 | 17 | 41 |
| words | 338 | 298 | 40 | 10 | **1** | 2 |
| **total** | **947** | 803 | 144 | **169** | **44** | 111 |

So **169 of 947 (17.8 %) carry an OOXML chart**, and **44 of 947 (4.6 %), or 26 % of the
chart-bearing ones, hold at least one rule with no stated width**. Every one of the 44 has a theme
with an `a:lnStyleLst`, so there is always an answer to be had.

The defect is per route, and the base rates that matter are per track: **26 of 307 sheets (8.5 %)
and 1 of 338 words (0.3 %)** were affected; the 17 slides documents are the **control**, because
`PptxSlideLayoutChart` already passed the matrix and their rules were already drawn at the theme's
width.

**What the instrument cannot see.** The 144 binary documents — 55 `.xls`, 49 `.ppt`, 40 `.doc` —
hold BIFF and Escher charts read by `XlsChartReader`, not by `DrawingChartPlot`. Nothing here is a
claim about them, and §7 records one of them as a new seat.

### 3.1 Where round 115's residual list lands in that census

**Eleven of the seventeen** documents round 115 left more than 10 % from the reference are among
the 26: `001`, `005`, `008` and `010` Contextures, `microsoft_learn_multi_chart_examples`,
`DynamicBubbleChart`, `022_Pareto_Chart_Template`, `029_Annual_budget`,
`032_Business_expenses_budget`, `046_Cost_analysis` and `064_Small_business_cash_flow`. That is the
seat.

The other six are `002` and `014` Contextures, `018_Weight_Loss_Chart`,
`019_Free_Blood_Sugar_Chart`, `084_Service_invoice` and `TK-Syllabus-Comparison-Document-v2`, and
§4.3, §6.1 and §7 account for every one of them: two are the marker outline, one is a BIFF chart
and three are underlines.

## 4. The chart mechanism, and why round 115's sentence is wrong

Round 115 wrote *"on several of them we stroke a chart's rules at width 0 where the reference
strokes them"*. Half of that is right and the half that is wrong sent the next round the wrong way:
**the reference also writes `0 w`** — 108 times on the first witness — so a zero width is not by
itself the disagreement. Read item by item against the banked reference bank
(`/home/user/gate-r114/ref`), the three witnesses split into three different things:

`008_Contextures_chart_sample_7ea7a1d7.xlsx`, per page:

| | ours (r114) | 26.2.4.2 |
|---|---|---|
| gridlines and axis lines | `0.0000 w` × 27, `#8B8B8B` | `0.7354 w` × 27, `#878787` |

`022_Pareto_Chart_Template`, page 1:

| | ours (r114) | 26.2.4.2 |
|---|---|---|
| gridlines | 0.2500 × 33 | 0.2551 × 33 |
| series marks | 1.0000 × 2, 2.0000 × 2 | 0.9921 × 2, 2.0124 × 2 |
| data-point markers | *(filled only)* | fill **and** `0 w` stroke × 16 |

`018_Weight_Loss_Chart`, page 1: the same three, with 1.2500 → 1.2472, 2.5000 → 2.4943 and
3.0000 → 3.0045, and 17 marker outlines missing.

Three mechanisms, and each is confirmed twice.

### 4.1 The automatic width was never resolved, because the theme's format matrix was not passed

**Source.** `spAxisLines` and `spMajorGridLines` in `oox/source/drawingml/chart/objectformatter.cxx`
(:216-228) give every chart style `mnRelLineWidth = 100` and `THEMED_STYLE_SUBTLE`, so an automatic
axis line or gridline is the theme's first `a:lnStyleLst` entry at full width.
`DrawingChartAutoFormat.AutomaticLineWidth` already modelled that and already carried a remark
saying it had been *measured* by patching a theme — and it returns `Length.Zero` when there is no
format matrix. `XlsxDrawings.Plot` passed `styles: null`, `XlsxDrawings.ExtendedPlot` passed
`styles: null`, and `DocxPictures.LoadChart` passed none at all. Only `PptxSlideLayoutChart` joined
the two halves, and its own comment says so.

**Binary.** 26.2.4.2's `--convert-to ods` of `008_Contextures_chart_sample` resolves that chart's
axes and gridlines to `svg:stroke-width="0.026cm" svg:stroke-color="#878787"`, and its PDF strokes
them at 0.7354. Ours strokes them at zero and in `#8B8B8B` — and **the colour was the same
defect**: with the matrix present we resolve the theme's own `phClr` line style, `shade 95000` and
`satMod 105000` included, and land on `#878787` exactly.

### 4.2 A DrawingML line width is a whole hundredth of a millimetre

**Source.** `LineProperties::getLineWidth()` is `convertEmuToHmm(moLineWidth.value_or(0))`
(`oox/source/drawingml/lineproperties.cxx`:560-563); `convertEmuToHmm` is
`o3tl::convertNarrowing<sal_Int32, emu, mm100>` (`include/oox/drawingml/drawingmltypes.hxx`:186-190)
whose `MulDiv` is `(n × 1 + 180) / 360` for a positive value
(`include/o3tl/unit_conversion.hxx`:72-78) — an integer, rounded half up.

**Binary, ten values over five documents.** 26.2.4.2's `--convert-to ods` writes every chart's
`svg:stroke-width` as a whole hundredth of a millimetre and never as the stated EMU:

| document | resolved `svg:stroke-width` |
|---|---|
| `008_Contextures_chart_sample` | `0.026cm` |
| `018_Weight_Loss_Chart` | `0.009 0.035 0.044 0.071 0.079 0.088 0.106 cm` |
| `022_Pareto_Chart_Template` | `0.009 0.018 0.035 0.071 cm` |

and its PDFs stroke them at 0.2551, 0.9921, 1.2472, 2.0124, 2.4943 and 3.0045 pt, which are
9, 35, 44, 71, 88 and 106 hundredths of a millimetre to four decimals. The corresponding stated
widths are 0.25, 1, 1.25, 2, 2.5 and 3 pt and **the reference draws none of them.** On the
purpose-built fixture the reference draws 0.7357 for a stated 9525 EMU and 2.2354 for a stated
28575, which are 26 and 79 hundredths of a millimetre times that chart's own fit scale of 0.998.

### 4.3 A chart marker is filled *and* stroked, in its own fill colour

**Binary first, because the fixture separates two things a corpus document cannot.**
`022_Pareto_Chart_Template` states a marker with a `6B0C00` fill and a `6B0C00` `a:ln`, and
26.2.4.2 emits sixteen filled quads and sixteen `0 w` stroked quads at the identical rectangles —
so the marker is two operations, but the file cannot say which colour the stroke came from. The
fixture states an `ED7D31` fill and a `203864` outline: **26.2.4.2 strokes all four markers in
`ED7D31`, and `203864` appears nowhere on the page.**

**Source.** `TypeGroupConverter::convertMarker` sets `Symbol::FillColor` from the marker's fill and
only reaches for its line colour in the `tdf#124817` branch, when there is no fill at all
(`oox/source/drawingml/chart/typegroupconverter.cxx`:656-679); `VLegendSymbolFactory` states the
same in a comment — *"border of symbols always same as fill color"*.

This tree drew the four closed marker shapes as a fill and nothing else, and passed the marker's
own `a:ln` colour to the two stroke-only shapes.

### 4.4 A series that states an `a:ln` with no width takes neither rule, and a data point takes a third

This one was found by the census rather than before it, in two steps, and both steps are why §6's
figures are the third sweep and not the first.

**Step one.** Handing the sheets route its style matrix made `064_Small_business_cash_flow`'s alert
line **3.74 pt** where the reference draws **0.99** — a 3.8× overshoot on a document that had been
8× too light. One fixture varied one attribute at a time, read back through 26.2.4.2's own
`--convert-to ods`:

| the series states | resolved `svg:stroke-width` |
|---|---|
| no `c:spPr` at all, theme 9525, `c:style val="1"` | `0.079cm` — 9525 × **300 %** |
| no `c:spPr` at all, theme 9525, `c:style val="18"` | `0.132cm` — 9525 × **500 %** |
| the same, with the style wrapped in the `mc:AlternateContent` pairing `c14:style val="118"` with a `c:style val="18"` fallback | `0.132cm`, unchanged — the fallback wins |
| `<c:spPr><a:ln><a:solidFill/></a:ln></c:spPr>`, no `w` | **`0.035cm`** |
| the same, with the theme's subtle line raised to 38100 | **`0.035cm`**, while every gridline moves 0.026 → 0.106 |

The last two rows are the measurement: the gridlines follow the theme and the series does not, so
35 hundredths of a millimetre is a flat default and not a scaled width. And it is **not** a
property of being drawn as a line — a `c:barChart` series and a `c:pieChart` series stating the
same `a:ln` at the same style are both `0.035cm`, even though `spFilledSeriesLines` is
`AUTOFORMAT_INVISIBLE` over styles 17-32 and gives them no automatic width to replace.

**Step two, and it is the slides census that found it.** With that rule in, one deck went 0.596 →
**1.873**: `bitesize-writing-a-report.pptx`, whose first chart is a `c:pieChart` with ten `c:dPt`
children, each stating its own `<a:ln><a:solidFill/>` with no `w`. 26.2.4.2 strokes all twenty of
its wedge outlines at **`0 w`**.

The one-variable probe settles it. Adding a single `c:dPt` to the pie fixture, whose `a:ln` states
a colour and no `w`, 26.2.4.2 draws:

```
0        (0.0, 0.667, 0.0)  ×1     <- the point that states its own a:ln
0.99036  (0.125, 0.22, 0.392) ×3   <- the three that take the series'
```

So **a stated `a:ln` with no width is 35/100 mm on a series and a hairline on a point.**

This model has no per-point line width — `ChartSeries.PointFills` has no companion — so
`DrawingChartPlot.PointsStateTheirOwnLine` gives the whole series the point's answer whenever any
point states an `a:ln`. That is exact for a series where *every* point states one, which is the
shape that occurs, and wrong for a mixed series; the proper fix is a per-point width in the model
and it is recorded in §7.5.

**Where 35 comes from is not established.** It is what chart2 answers once the OOXML importer has
set no width, and it could not be located in the 27.2 tree; the seven measurements above are what
the rule rests on.

## 5. What changed

`src/XRay.Office.Spreadsheets/Layout/SheetPageDecoration.cs`

- `Dashes` is an instance method taking only the border, and returns
  `DashUnit × table × _scale` where `DashUnit` is ten twips. It no longer sees the drawn width.

`src/XRay.Office.Ooxml/DrawingML/DrawingChartAutoFormat.cs`

- New `LineWidth(long emu)` — `Length.FromMm100(Length.FromEmu(emu).Mm100)` — with the two legs of
  §4.2 in its remark. `AutomaticLineWidth` goes through it.

`src/XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs`

- `StatedLineWidth`, `LineWidthOf`, `AutoLineWidth` and `AutomaticChartAreaLineWidth` go through it
  too. Those are every width a chart carries in the OOXML reader.
- New `SeriesLineWidth` and `PointsStateTheirOwnLine`, the rule of §4.4: a stated `w`, then — when
  the series states an `a:ln` without one — a hairline if its points state their own `a:ln` and a
  flat `SeriesLineDefault` of 35 hundredths of a millimetre otherwise, and the automatic entry only
  when the series states no `a:ln` at all. The old code was `StatedLineWidth ?? AutoLineWidth`,
  which has the middle case taking the chart style's 300–700 % multiplier.

`src/XRay.Office.Spreadsheets/Ooxml/XlsxDrawings.cs`

- `Plot` and `ExtendedPlot` take the theme's `DrawingStyleMatrix` and hand it on. The workbook
  reader already read it and already passed it to `XlsxDrawings.Read` for a worksheet shape's
  `xdr:style`; only the two chart call sites dropped it.

`src/XRay.Office.WordProcessing/Ooxml/DocxPictures.cs`

- `LoadChart` passes `_file.ShapeStyles`, which `DocxFile` has read since shapes needed it.

`src/XRay.Office.Core/Charts/ChartLayout.cs`

- The four closed marker shapes — circle, diamond, triangle and the square default — are returned
  with their fill as **both** fill and outline. The cross and the star, which have no fill, keep
  the marker's stated line colour.

Five tests in `XRay.Office.Presentations.Tests` asserted a chart width as the EMU the file states and
now assert the hundredth of a millimetre the reference keeps; each carries the reason inline. They
are listed in §8 because a changed assertion is not a passing one.

Nothing in the ODF or BIFF chart readers is touched: ODF states `svg:stroke-width` in centimetres
to three decimals, which is already this unit, and BIFF is §7's seat.

## 6. The census, before and after — and what the census is actually measuring

`census2.py` is round 113's instrument (eight pages sampled, both sides must emit ≥10 stroked
items with counts within 25 %, the statistic is the ratio of mean stroke widths) with **two
columns added**, and those two columns are the point of this section.

**A `0 w` stroke is a hairline, and the mean counts it as nothing.** LibreOffice's own PDF export
draws a hairline at `72/DPIX` = a tenth of a point (`PDFPage::appendLineInfo`,
`vcl/source/pdf/PDFPage.cxx`:497-505), so a zero-width item is real ink; the census scores it as
zero. `hair_ours` / `hair_ref` count them, and `ratio_ex_hair` is the same ratio with them dropped.

At this round's **base**, over the sheets track against the banked 26.2.4.2 leg
(`/home/user/gate-r114/ref`), the seventeen documents outside ±10 % look like this:

| document | ratio | hairlines ours | hairlines ref | ratio excluding hairlines |
|---|---:|---:|---:|---:|
| `014_Contextures…xls` | 0.000 | 108 | 108 | 0.000 |
| `008_Contextures` | 0.035 | 54 | 0 | **0.979** |
| `microsoft_learn_multi_chart` | 0.066 | 166 | 6 | **0.939** |
| `064_Small_business_cash_flow` | 0.123 | 240 | 78 | 0.479 |
| `001_Contextures` | 0.149 | 54 | 0 | 1.754 |
| `010_Contextures` | 0.300 | 48 | 0 | **1.148** |
| `005_Contextures` | 0.397 | 108 | 0 | **1.000** |
| `029_Annual_budget` | 0.548 | 17 | 0 | **0.954** |
| `032_Business_expenses_budget` | 0.708 | 30 | 4 | 0.872 |
| `DynamicBubbleChart` | 0.725 | 28 | 1 | **1.052** |
| `TK-Syllabus-Comparison` | 0.800 | 0 | 0 | 0.800 |
| `002_Contextures` | 0.825 | 0 | 0 | 0.825 |
| `084_Service_invoice` | 0.849 | 0 | 0 | 0.849 |
| `046_Cost_analysis` | 0.857 | 2 | 0 | **0.989** |
| `018_Weight_Loss_Chart` | 1.147 | 0 | 54 | **0.998** |
| `019_Free_Blood_Sugar_Chart` | 1.277 | 0 | 120 | **1.040** |
| `022_Pareto_Chart_Template` | 1.286 | 0 | 16 | **1.008** |

**Read the last column.** Ten of the seventeen are within 6 % of 1.000 once hairlines are dropped,
and every one of the three "heavier" rows is within 4 %. Round 113's statistic was, on this class,
**measuring which items are hairlines rather than how heavy the strokes are** — the lighter rows are
ones where we make hairlines of rules the reference strokes (§4.1), and the heavier rows are ones
where the reference makes hairlines we do not draw at all (§4.3). Three rows — `TK-Syllabus`,
`002_Contextures`, `084_Service_invoice` — have no hairlines on either side and are a genuine
weight difference; they are not this seat and §7 says what is known about them.

`014_Contextures…xls` is 108 hairlines on both sides and no other stroke of ours at all: it is a
BIFF chart and §7 seats it.


### 6.1 The class, before and after

Both legs of ours rendered fresh on **2026-09-13**, `SOURCE_DATE_EPOCH=1700000000`, against the
banked 26.2.4.2 leg — same UTC day, which is what C13 requires. The base leg reproduces round 115's
"after" figures to the fourth decimal (mean |ratio−1| 0.0570 against its 0.0571), which is the
instrument agreeing with itself across two rounds.

**Sheets track, 307 documents, 191 comparable on both legs:**

| | base | after |
|---|---:|---:|
| median ratio | 1.000 | 1.000 |
| ours lighter by >10 % | 14 | **4** |
| within 10 % | 174 | **186** |
| ours heavier 10–50 % | 3 | **1** |
| ours heavier ≥1.5× | 0 | 0 |
| mean \|ratio − 1\| | 0.0571 | **0.0218** |
| better / worse / level | — | **32 / 3 / 156** |

Three more documents become comparable after the change — `021_Control_Chart_Template`,
`062_Run_chart` and `063_Sales_pipeline` — because the two sides' item counts now agree inside the
census's 25 % guard, which they did not when we drew no marker outlines. The table above is the
**common 191** so that the two columns are the same population; on the after leg's own 194 the
figures are 4 / 188 / 2 / 0 and a mean of 0.0215.

**The five rows still outside ±10 %, and none of them is this seat:**

| document | ratio | why |
|---|---:|---|
| `014_Contextures…xls` | 0.000 | a BIFF chart — **O61** |
| `TK-Syllabus-Comparison` | 0.800 | underlines the reference strokes — **O62** |
| `002_Contextures` | 0.824 | the same |
| `084_Service_invoice` | 0.849 | the same |
| `019_Free_Blood_Sugar_Chart` | 1.152 | we draw 60 marker outlines where the reference draws 120; ratio excluding hairlines **1.045** |

**The three that get worse are worth naming rather than hiding**, and all three are small:
`002_Contextures` 0.825 → 0.824, `035_Project_plan_for_law_firms` 1.000 → 1.003, and
`053_Personal_asset_inventory` 0.997 → 1.041 — whose ratio excluding hairlines *improves*,
1.077 → 1.041, so the mean moved because the marker outlines it gained are scored as zero.

`like-for-like-base.tsv`, `like-for-like-after.tsv`.

### 6.2 Reach, honestly

```sh
XRAY_OFFICE_CLI=…/Release/net10.0/linux-x64/XRay.Office.Cli SOURCE_DATE_EPOCH=1700000000 \
  python3 sweep.py /home/user/sample-files '<track>/*/*/*' /home/user/r117-{base,after}/<track> 4
python3 reach2.py /home/user/r117-base/<track> /home/user/r117-after/<track> moved-<track>.tsv
```

All three tracks rendered **947 of 947 with no failures on both legs**.

| track | documents | renderings changed | page counts moved | alphanumeric counts moved |
|---|---:|---:|---:|---:|
| sheets | 307 | **60** | **0** | **0** |
| words | 338 | **7** | **0** | **0** |
| slides | 302 | **46** | **0** | **0** |

**No gate column can see any of this and none moved.** A dash pattern, a stroke width and a marker
outline add no page and no alphanumeric character, and the coordinator's whole-corpus gate on round
115's border change — 120 of 307 renderings changed, 947 documents, 915 match, 32 mismatch, **0
verdicts changed** against a same-UTC-day baseline — is the measurement that settles the class
rather than an argument. **Claiming gate reach here would be claiming reach the gate cannot see.**
Ink and the two width censuses are the instruments.

### 6.3 The two confounds that bite this seat, and where they land

**C13** — `TODAY()`/`NOW()` recalculated on load, and a gate compared across a UTC date boundary.
Both of our legs were rendered on **2026-09-13** and so was every census run; the reference leg is
a bank, so it contributes no date at all.

**C11** — the reference is not reproducible run to run on a small non-fixed set, and the two
sharpened members are two-state oscillators: `047_Date_tracker_Gantt_chart` (3441 ↔ 3367
characters) and `SIL_TDB648` (30896 ↔ 30899). Where they land here:

- `047_Date_tracker_Gantt_chart` **is** one of the 60 sheets renderings that moved — it is one of
  the 26 documents with an automatic chart width — but it is **not in the census** at all, excluded
  by the ≥10-items-within-25 % guard, so no per-document claim in this round rests on it. The reach
  measurement that does include it compares **our** base against **our** after and never touches the
  reference, so C11 cannot reach it.
- `SIL_TDB648` **is** in the census, at a ratio of **1.029 on both legs, byte-identical rows** —
  the change does not touch it and it is inside ±10 % either way.

So neither needed a second reference render, and I did not do one. Had either landed in the
witness set, the brief's rule applies and it would have.

### 6.4 The other two tracks, censused as well as confined

The change reaches the words route (the style matrix) and the slides route (the 1/100 mm
quantiser and the marker outline), so both were censused against the same banked reference rather
than argued about.

**Words, 338 documents, 173 comparable on both legs:** mean |ratio−1| **0.0506 → 0.0481**, lighter
by >10 % **11 → 10**, better/worse/level **1 / 2 / 170**. Only three rows move at all:

| document | base | after |
|---|---:|---:|
| `ABCD-FE-01-00 Flight Envelope` — the track's one automatic-width document | 0.458 | **0.933** |
| `clustered-column-result` | 0.999 | 1.026 |
| `clustered-column-template` | 1.012 | 1.037 |

The two `clustered-column` rows read *worse* and are not. Each had exactly **one** zero-width item
which is now a real width, and their ratio **excluding hairlines improves** — 1.042 → 1.026 and
1.060 → 1.037. That is C14 again, in the direction that makes a fix look like a regression.

**Slides, 302 documents, 113 comparable on both legs:** mean |ratio−1| **0.0582 → 0.0555**, lighter
by >10 % **11 → 10**, better/worse/level **20 / 3 / 90**, and no row ≥1.5× on either leg. The
slides route had the style matrix already, so what moves here is the 1/100 mm quantiser and the
marker outline — and it moves cleanly: **thirteen `advanced_powerpoint_*` decks go 1.008 → 1.000
exactly**, `bar_chart`, `stacked_area_chart`, `stacked_bar_chart` go 1.016 → 1.000 and `3492`
1.012 → 1.000. Seven more become comparable — the four `advanced_powerpoint_line` decks at 1.000,
`Demick_JetBlue` at 1.005, `line_chart` at 1.003 and `scatter_chart` at 1.086 — because their item
counts now agree once the markers are stroked. The three that get worse move by 0.001, 0.002 and
0.005.

`Demick_JetBlue` is the control worth naming: at the base we drew 397 strokes at 0.75 where the
reference draws 393 at 0.7346, and 255 of its marker outlines were missing; after, the widths are
26/100 mm on both sides and the ratio is 1.005.


## 7. What is left, and two new seats

### 7.1 O59 is closed

The five arrays are absolute, they take the print scale, and the tests read every number out of
26.2.4.2's own PDF. Two of the five — `dashed` and the two dash-dot forms — have **zero corpus
reach** and are exercised only by the fixture; that is stated rather than hidden, and it is why
this round claims 12 documents and not 307.

### 7.2 The chart half is closed for the OOXML readers, and one residual is not this seat

After the change, `sheet-chart-auto-line.xlsx` draws its automatic lines at **0.737 pt** against
26.2.4.2's **0.7357**. The 0.18 % is the chart's own anisotropic fit scale — the drawn-extent
squeeze `ChartLayout.DrawnExtent` models — and not the width rule; the same 0.18 % appears on the
series polyline (2.2394 against 2.2354) and on `008_Contextures` (0.7370 against 0.7354). It is
already recorded against `ChartLayout` and is not opened again here.

The reference draws **55** automatic lines on that fixture where we draw **23**. That is a count,
not a width: it is ticks and gridlines we do not emit, and no measurement here says which. It is
recorded below.

### 7.3 New seat — **O61**: a BIFF chart's own rules are never given a width

`XlsChartReader` resolves no line width at all, so a `.xls` chart's rules are all hairlines.
Measured over the sheets track: **7 of 64 `.xls` documents carry a chart** (2.3 % of the track),
counted from 26.2.4.2's own `--convert-to ods` — a `chart:chart` inside an `Object N/content.xml` —
which agrees with the XML census exactly on the zip half (92 of 254 against 92).

Of those seven, **one shows the pathology**: `014_Contextures_chart_sample_991ecfc5.xls` draws 108
strokes, every one a hairline, against the reference's 136 of which 108 are hairlines and 28 are
strokes at 0.794, 0.9884, 1.106 and 1.9768 pt. Mean stroke width 0.000 against 0.250. The other six
are within 6 % of the reference's mean and one of them (`TOGAF9-Tool-ConfReqts-CSQ`) is *heavier*
than the reference, so this is not a uniform BIFF-wide zero.

**Reach: 1 of 307 sheets documents, and the class is at most 7.** Worth one round only alongside
something else on that reader.

### 7.4 New seat — **O62**: a Calc cell's hyperlink underline is stroked by the reference and drawn by nothing here

The three documents in §6 with no hairline on either side are all this. On
`084_Service_invoice_Use_this_template`, page 1 carries `jordan@example.com` at (84, 234) and
26.2.4.2 strokes a navy `#000080` line 0.51 pt thick from x 83.8 to 193.3 at y 243.8, just under
that baseline; **our page has nothing at all in that band.** The same shows as 0.595 pt on the same
document's second link, 1.389 pt on `002_Contextures_chart_sample` — the thickness tracking the
face's size, as an underline's does — and as several hundred red 0.794 pt rules on
`TK-Syllabus-Comparison-Document-v2`, which is the row the census scores 0.800.

This is *the* instrument trap the brief names, arriving from the other side: the underline is a
**stroke** in the reference and is missing here rather than being a fill, so the guard that
excluded 584 documents from the original census does not catch it.

**Not censused beyond these three**; the right instrument is a count of `hyperlink`-formatted and
underlined cells over the sheets track, which this round did not build.

### 7.5 Not fixed, deliberately

- **A per-point line width is not modelled.** `ChartSeries` carries `PointFills` and no
  companion for the outline, so §4.4's data-point rule is applied to the whole series. Exact where
  every point states an `a:ln`, wrong where only some do; no corpus document in the residual set
  is the mixed case, so its reach is unmeasured.
- **A marker whose `c:marker/c:spPr` states `a:noFill` is still drawn filled.** Our model has one
  marker colour and no way to say "no fill"; the reference's `tdf#124817` branch takes the line
  colour in that case and draws a hollow symbol. No corpus document in the residual set exercises
  it, so nothing here measures its reach.
- **A conditional format's border is not read at all**, in any of the three readers. Three of the
  twelve documents in §1 state a patterned border only in a `<dxf>`, and the reference resolves it.
  That is a reader gap rather than a stroke gap and belongs to whoever next opens
  `XlsxConditionalFormats`.
- **Non-chart DrawingML outlines do not go through the 1/100 mm quantiser.** `LineProperties`
  applies to every shape, not only to a chart's, so a worksheet or slide shape's `a:ln w` is very
  probably subject to the same rounding. This round changed only the chart readers, because those
  are the ones it measured; extending it is a separate, larger change with its own reach.

## 8. Tests

**New — thirteen cases over three new fixtures and three of round 115's.**

`tests/XRay.Office.Spreadsheets.Tests/SheetBorderDashTests.cs` — four cases on
`sheet-border-widths.xlsx`, `sheet-border-widths-scaled.xlsx` and `sheet-border-widths.xls`: the
five arrays, a thin and a medium dotted border drawing the *same* array, every dash taking the
print scale, and the BIFF filter giving the same answers as the OOXML one.

`tests/XRay.Office.Spreadsheets.Tests/SheetChartLineWidthTests.cs` — seven cases. Four on the new
`sheet-chart-auto-line.xlsx`: an automatic axis line takes the theme's subtle-line width, none of
the chart's furniture is stroked at zero, a stated width is kept as a whole hundredth of a
millimetre, and a filled marker is stroked in its own fill colour. Three on the new
`sheet-chart-series-width.xlsx`, which holds §4.4's cases in one chart: three line series get
2.2394, 0.9921 and 3.7417 pt, the middle one does **not** take the chart style's multiplier, and a
fourth series — a *bar* stating the same widthless `a:ln` — gets the same 0.9921 the reference
draws it at.

`tests/XRay.Office.WordProcessing.Tests/FrameChartLineWidthTests.cs` — two cases on the new
`words-chart-auto-line.docx`, the same chart part in a Writer document, so the two routes are
pinned separately.

The three fixtures are purpose-built by `make-chart-fixtures.py` and carry a theme whose
`a:lnStyleLst` first entry is `w="9525"`, a line series stating `w="28575"`, and a diamond marker
whose fill and outline colours **differ** — which is what lets §4.3 be measured at all.

**Mutation control, run on the final set.** At the round's base, with the six source files
reverted and the tests and fixtures unchanged, **all thirteen fail**: 4 of 4
`SheetBorderDashTests`, 7 of 7 `SheetChartLineWidthTests`, 2 of 2 `FrameChartLineWidthTests`. The
control that must hold in both states is round 115's `SheetBorderWidthTests`, which is **8 passed
of 8 at the base** and 8 of 8 after — the widths are unchanged and only the dashes moved.

**Changed — six existing cases in five files.** Six `XRay.Office.Presentations.Tests` cases asserted
a chart's width as the EMU the file states. They now assert the hundredth of a millimetre the reference
keeps, and each carries the reason inline:

| test | was | now |
|---|---|---|
| `DrawingChartAreaBorderTests.AChartSpaceStatingNoLineGetsTheGreyDefaultOutsideImpress` | 9525 EMU | 26/100 mm |
| `DrawingChartAreaBorderTests.AStatedLineWinsOverTheAutomaticOne` | 19050 EMU | 53/100 mm |
| `DrawingChartAutoFormatTests.AChartSpacesStatedOutlineIsRead` | 25400 EMU | 71/100 mm |
| `DrawingChartAutoFormatTests.ALineSeriesStatingNoWidthTakesTheThemesSubtleLineTrebled` | 28575 EMU | 79/100 mm |
| `DrawingChartFurnitureTests.AnUnstatedGridTakesTheThemesSubtleLineWidth` | 9525 / 38100 EMU | 26 / 106 /100 mm |
| `DrawingChartFurnitureTests.AStatedColourWinsAndAStatedWidthAloneDoesNot` | 19050 EMU | 53/100 mm |
| `DrawingChartMinorGridTests.TheMinorGridCarriesTheWidthAndDashItStates` | 6350 EMU | 18/100 mm |
| `DrawingChartDateAxisTests.APresetDashOnASeriesReachesTheModel` | dash total 2×38100 EMU | 2×38160 EMU |

Two of those numbers are directly measured — 9525 and 28575 are the fixture's own, and 26.2.4.2
draws them at 0.7357 and 2.2354 — and the rest follow the one arithmetic rule that fits all ten
values in §4.2.

### 8.1 Full suite, real totals

Run project by project rather than as a solution, and the counts are the ones the runner printed:

| project | passed | failed | skipped |
|---|---:|---:|---:|
| XRay.Office.Core | 588 | 0 | 0 |
| XRay.Office.Containers | 109 | 0 | 0 |
| XRay.Office.Text | 728 | 0 | 0 |
| XRay.Office.Vector | 309 | 0 | 0 |
| XRay.Office.Rendering | 164 | 0 | 0 |
| XRay.Office.Markup | 259 | 0 | 0 |
| XRay.Office.OpenDocument | 160 | 0 | 0 |
| XRay.Office.WordProcessing | 1940 | 0 | 0 |
| XRay.Office.Spreadsheets | 1371 | 0 | 0 |
| XRay.Office.Presentations | 1203 | 0 | 0 |
| XRay.Office.Fidelity | 542 | **10** | 0 |
| **total** | **7373** | **10** | **0** |

Against round 115's totals the additions are exactly the new cases: Spreadsheets 1360 → 1371
(+4 dash, +7 chart) and WordProcessing 1938 → 1940 (+2). Presentations is unchanged at 1203
because its six changed cases are edits and not additions.

`XRay.Office.Fidelity` ran 552 of 552 with **0 skipped**, so LibreOffice was present and the
project covered what it is meant to. Its ten failures are the same rows round 115 recorded, and
they fail at this round's base too — the run in §8's mutation control had the identical list:

- nine are the reconstructed-position family this repository leaves failing on purpose —
  `PageDrawingComparisonTests` on `paginated.docx`, `.doc`, `.rtf` and `.fodt`,
  `TabStopComparisonTests` on `list-label-overrun` in four formats, and
  `JustificationShrinkComparisonTests` on `justify-shrink-2013.docx`;
- one is `SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt("sheet-rich-text.xlsx")`,
  the anchor-offset clamp its own remark records. A picture's position is not reachable from a dash
  array, a chart line width or a marker outline.

## 9. Files

See `README.md` for the table. Everything measured here is dated **2026-09-13** against
**26.2.4.2**; corpus `/home/user/sample-files`, reference bank `/home/user/gate-r114/ref`,
converted ODF corpus `/home/user/corpus-odf`.

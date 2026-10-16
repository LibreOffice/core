# Round 118 — a Calc hyperlink's underline, a BIFF chart's weights, and two of three witnesses refuted

Three seats: **O62** (a Calc hyperlink cell's missing underline), **O61** (a BIFF chart's rules
never given a width) and **O60** (a `.ppt` table cell's inner measure). O62 and O61 are
implemented and measured; O60 closes on its measurement. **Two** new seats are opened, **O64**
(the reference's underline thickness is quantised to a whole hundredth of a millimetre and the
value it quantises is not derived here) and **O65** (a *rich* hyperlink cell keeps the file's own
colour at the reference and this tree substitutes the application's navy).

Everything below separates **measured** from **inferred**, and every mechanism is confirmed
twice — once in the `27.2.0.0.alpha0+` source tree at `/home/user/libreoffice-core`, once
against `/opt/libreoffice26.2/program/soffice` (**26.2.4.2**), which is the only reference.

---

## 0. The headline, before the detail

| | measured |
|---|---|
| O62 mechanism | `ScEditUtil::GetCellFieldValue` sets the **colour and the underline in the same two lines**; this tree had the colour and not the line |
| O62 base rate | **74 of 307** sheets documents hold a hyperlink-field cell; **73 of 307** hold an underlined cell |
| O62 reach | **27 of 243** zip spreadsheets hold a hyperlink cell whose own font states no underline — **212 cells** |
| O62 reach, rendered | **29 of 307** renderings change. **0** page counts, **0** alphanumeric characters, **0** gate verdicts |
| O62 witnesses | of round 117's three, **one is this seat and two are refuted** — one we already draw, one is a *strikethrough* |
| O61 mechanism | the `CHLINEFORMAT` weight is four numbers: hair 0, single **35**, double **70**, triple **105** hundredths of a millimetre |
| O61 reach | **2 of 307** renderings change; mean stroke width on the seat's own witness **0.000 → 0.2205** against 0.2500 |
| O60 | closed on its measurement; the discriminating quantity is **0.077 pt**, below the instrument's own agreement of 0.06–0.44 pt |
| tests | **7399 total, 7389 passed, 10 failed** — the same ten, by name, at the base |
| new seats | **O64** — the reference's underline thickness is a whole hundredth of a millimetre and this round did not derive the value it quantises. **O65** — a rich hyperlink cell keeps the file's blue at the reference and takes the navy here; 1 document, and it predates this round |
| did it get closer | link-coloured rule length within 2 % of the reference on **27 of 29** documents after, **0 of 29** before |

---

## 1. The census, with the base rate beside it

Two instruments, as the brief asks, and **they disagree — which is the finding**.

### 1.1 Instrument B — 26.2.4.2's own resolved view

`fods-census.py`: `--convert-to fods` on every one of the sheets track's 307 spreadsheets
through 26.2.4.2, then count, per document, cells holding a `<text:a>` with non-empty text and
cells whose *resolved* cell style states `style:text-underline-style` other than `none`.

```sh
python3 fods-census.py /home/user/sample-files/sheets fods-census.tsv 3
```

**307 of 307 converted, 0 failures.**

| | documents | base rate | cells |
|---|---:|---:|---:|
| a cell whose text is inside a `text:a` (a hyperlink field) | **74** | 24.1 % | 4357 |
| a cell whose resolved style states an underline | **73** | 23.8 % | 4471 |
| a cell that is both | 56 | 18.2 % | 3744 |
| an underlined span inside a drawing shape | 3 | 1.0 % | 23 |
| **either** | **90** | **29.3 %** | — |

### 1.2 Instrument A — the files' own markup

`xml-census.py`, over the 243 zip spreadsheets of the 307 (the 64 `.xls` need a BIFF parser and
are covered by B alone). Per cell: is it covered by a worksheet's `<hyperlinks>`, and does its
*resolved* font — `applyFont` over `cellXfs` and `cellStyleXfs`, as
`Xf::importXf`/`Xf::finalizeImport` resolve it (`sc/source/filter/oox/stylesbuffer.cxx`:2165-2196,
2350-2385) — carry `<u/>`?

| | documents of 243 | cells |
|---|---:|---:|
| a hyperlink-covered non-empty cell | 63 | 1983 |
| a cell whose resolved font states `<u/>` | 53 | 2244 |
| both on one cell | 46 | 1771 |
| **a hyperlink cell whose own font states NO underline** | **27** | **212** |

That last row is the seat: **1771 of 1983 hyperlink cells are already underlined by the file**,
and this tree already drew those. The 212 in 27 documents are the ones only the field rule
reaches.

### 1.3 Where the two instruments disagree, and why each disagreement matters

**They agree exactly on the hyperlink count** — 243 of 243 documents, cell for cell. They
disagree on the underline count on **17 of 243**, and in two different ways.

1. **B undercounts, and it is a repeat-run artefact.** A flat ODF writes a run of identically
   formatted cells once with `table:number-columns-repeated`, so B's `ul_cells` is a floor:
   `hdss-bulletin-index-2019-2022.xlsx` reads 350 by A and 25 by B, `essd-16-3433-2024-t02.xlsx`
   112 and 0. Nothing is wrong with either count; they answer different questions.

2. **B says the opposite of the truth on exactly the attribute this seat is about, and that is
   not an artefact — it is the export doing what its source says.** `ScXMLExport` resolves a
   field with `ScEditUtil::GetCellFieldValue(*pField, &rDoc, nullptr, nullptr)`
   (`sc/source/filter/xml/xmlexprt.cxx`:3062) — **both out-parameters null** — so neither the
   `LINKS` colour nor the line style reaches the flat file. Measured on
   `084_Service_invoice_Use_this_template`: the fods gives the `jordan@example.com` cell
   `underline=none` and `colour=#000000`, while 26.2.4.2's own PDF of the same file paints that
   text `#000080` and strokes a `#000080` rule 0.51 pt thick under it.

   **So `--convert-to fods` is the wrong instrument for a hyperlink cell's underline and colour,
   and it fails silently and plausibly.** The right instruments are the file's own hyperlink
   markup (A) and the reference's ink.

---

## 2. The mechanism, twice

### 2.1 In source

`ScEditUtil::GetCellFieldValue` (`sc/source/core/tool/editutil.cxx`:209-244), for a
`text::textfield::Type::URL`:

```cpp
svtools::ColorConfigEntry eEntry =
    INetURLHistory::GetOrCreate()->QueryUrl(aURL) ? svtools::LINKSVISITED : svtools::LINKS;

if (ppTextColor)
    *ppTextColor = ScModule::get()->GetColorConfig().GetColorValue(eEntry).nColor;
…
if (ppFldLineStyle)
    *ppFldLineStyle = FontLineStyle::LINESTYLE_SINGLE;
```

**The colour and the underline come out of one function call, thirteen lines apart.** The blind
reader's own observation — *"a hyperlink character style whose underline component is not
applied while its colour is — the navy is clearly right in both, so colour resolution is
working"* — reads as two paths. It is one path, of which this tree had modelled one half.

The rest of the chain:

* `ScFieldEditEngine::CalcFieldValue` (`editutil.cxx`:895-906) passes both out.
* `ImpEditEngine::UpdateFields` (`editeng/source/editeng/impedit2.cxx`:3229-3231) stores them on
  the field's own `EditCharAttribField`.
* `EditCharAttribField::SetFont` (`editeng/source/editeng/editattr.cxx`:349-350) applies them:
  `rFont.SetColor(*mxTxtColor)` and `rFont.SetUnderline(*mxFldLineStyle)`.

`SetUnderline` **replaces**. A hyperlink cell whose font states a double underline is drawn with
one line, not two, and not three.

### 2.2 Against the binary

A purpose-built fixture, `tests/corpus/features/sheet-hyperlink-underline.xlsx`
(`make-fixture.py`), holds five cells in Arial 12 with no stated colour — one per arm.
Rendered through 26.2.4.2 **twice** (C11), byte-identical output both times:

| cell | states | 26.2.4.2 draws |
|---|---|---|
| A1 `linkplain` | linked; font has no `<u/>` | text `#000080`, **one** `#000080` stroke, 0.595 pt, x 51.392–94.705 |
| A2 `bareplain` | not linked, same font | black text, **no rule** |
| A3 `linkdouble` | linked; font has `<u val="double"/>` | text `#000080`, **one** `#000080` stroke, 0.595 pt |
| A4 `baredouble` | not linked, same double font | black text, **two** black strokes, 0.397 pt |
| A5 `1205` | linked, but the cell holds a **number** | black text, **no rule** |

Every arm of the rule is measured on the reference, including the two that a fix applied to the
cell format instead of to the field would get wrong (A3 would draw two lines; A5 would draw one).
A5 is `WorksheetGlobals::insertHyperlink` converting only a `CELLTYPE_STRING` or `CELLTYPE_EDIT`
cell, which `SheetLayout.HoldsField` already models.

---

## 3. Round 117's three witnesses: one is this seat, two are refuted

### 3.1 `084_Service_invoice_Use_this_template` — the seat, confirmed

Rendered both ways, the reference twice. Three `#000080` strokes on the reference's pages that
have nothing at all on ours:

| page | reference | this tree, base |
|---|---|---|
| 1 | stroke 0.51 pt, x 83.79–193.29, y 243.78 | nothing |
| 3 | stroke 0.595 pt, x 51.39–320.80, y 112.73 | nothing |
| 3 | stroke 0.51 pt, x 51.39–419.19, y 126.54 | nothing |

The cell is B13, style 29, whose `cellXf` carries `xfId="34"` (Excel's `Hyperlink` cell style,
whose font 11 *is* underlined blue) but `applyFont="1"` with its own **font 34** — Aptos 10 pt,
`#FF0000000`, no `<u/>`. So the file's own resolution says "not underlined, black" and the
reference draws underlined navy. That is the whole seat.

### 3.2 `002_Contextures_chart_sample` — **refuted; we already draw all ten**

The row records *"1.389 pt on `002_Contextures_chart_sample` — the thickness tracking the face's
size"*. Both instruments say its ten hyperlink cells are **all** underlined by the file, and the
renderings agree:

| | reference | this tree, base |
|---|---|---|
| items | 10 navy **strokes**, 1.389 pt | 10 navy **fills**, 1.326 pt high |
| first, page 1 | x 108.1–272.4, stroke centre y 137.62 (band 136.93–138.32) | x 108.1–272.5, band y 136.88–138.21 |
| colour | `#000080` | `#000080` |

Same count, same places, same colour. What differs is **shape** — the reference strokes and this
tree fills — and 0.063 pt of thickness. It was never O62.

### 3.3 `TK-Syllabus-Comparison-Document-v2` — **refuted twice: they are strikethroughs**

The row records *"several hundred red 0.794 pt rules"*. Neither instrument finds a single
hyperlink cell in this workbook (`link_cells` 0 by the fods, `hl_cells` 0 by the XML), and the
file says why: `xl/sharedStrings.xml` holds **4221 `<strike/>` runs and 12 `<u/>` runs**. It is a
syllabus *comparison* document; the red text is deleted text, struck through.

Measured over all 1235 pages of both renderings:

| | reference | this tree, base |
|---|---|---|
| strokes at 0.794 pt | **7625** | 0 |
| red thin fills, 0.72 pt high | 0 | **7449** |
| hairlines (`0 w`) | 34163 | 32340 |

And on page 5, the run `Be familiar with` at Carlito 10.998 has its baseline at 357.79: the
reference's rule sits at **355.49**, 2.30 pt *above* it. That is a strikethrough, not an
underline. Ours fills 355.07–355.79 where the reference's stroke covers 355.09–355.89 — the same
top edge to **0.02 pt**, 0.074 pt thinner.

**So this document's 0.800 census score is the fill-versus-stroke trap, not missing ink**, and it
is C14's sibling: C14 says a mean stroke width scores a hairline as nothing; this says it scores
a *filled* rule as nothing too. A stroke census cannot see any rule this tree draws, because
`SheetTextLayout.Rule` emits `FillPath` where `PDFWriterImpl::drawStraightTextLine`
(`vcl/source/pdf/pdfwriter_impl.cxx`:6658-6760) emits a stroke.

---

## 4. What changed

`src/XRay.Office.Spreadsheets/Layout/SheetTextLayout.cs` gains `Line(stated, field)` beside the
existing `Ink(portion, fallback, field)`, and `Decorate`/`DecorateSegment` take the field flag:

```csharp
private static SheetUnderline Line(SheetUnderline stated, bool field)
    => field ? SheetUnderline.SingleLine : stated;
```

`SheetCellFormat.Underline`'s remark, which said *"The underline survives that style and the
colour does not"*, is corrected: neither survives, and the field decides both.

**Not changed, and stated rather than hidden:** `DrawRotated` and `DrawStacked` draw no rules at
all, so a *turned* or *stacked* hyperlink cell still gets none. That gap predates this round and
is not this seat; no corpus document in the 29 movers is a turned hyperlink cell.

---

## 5. Reach, measured honestly

Three sweeps of the whole sheets track (`sweep.py`), all on **2026-09-13 UTC**, all with
`SOURCE_DATE_EPOCH=1700000000`, one temporary directory per *document*:

| leg | engine | rows | failures |
|---|---|---:|---:|
| `ref-sweep.tsv` | 26.2.4.2 | 307 | 0 |
| `ours-base.tsv` | this tree at `79ee2f7e3` | 307 | 0 |
| `ours-after.tsv` | + O62 | 307 | 0 |
| `ours-final.tsv` | + O62 + O61 | 307 | 0 |

**C13 is not a hazard here and the reason is structural, not lucky.** This tree evaluates no
volatile function — `TODAY()` is taken from the file's cache — and `SOURCE_DATE_EPOCH` pins the
only clock it does read, so our renders are identical whatever day they run on. The reference leg
was rendered **once** and its numbers reused for both comparisons, so the reference column is
literally the same bytes on both legs and cannot move a verdict between them.

### 5.1 O62

**29 of 307 renderings change.** `moved.txt` lists them.

* **0** change page count.
* **0** change alphanumeric characters (column 9).
* Gate verdict **293 of 307 both legs**; **0 rows moved**, in either direction.

**An underline is ink, not characters, and the gate cannot see it. That is expected and is
stated rather than dressed up as reach the gate could confirm.** The gate's three checks are page
count, alphanumeric characters and font embedding; a rule under a word changes none of them.

The reconciliation with §1.2 is exact: the 27 **zip** movers are precisely instrument A's 27
candidates — no document in one set and not the other — and the remaining two are
`Praktikastellen_-_chinesischsprachiger_Kulturraum.xls` and `RMP 2011-2014 and Inventory.xls`,
which A cannot see because it does not parse BIFF.

**Did it get closer? Yes, and the statistic has to be ink rather than objects.** `rule-check.py`
renders each of the 29 through both engines and measures the `#000080` rules — counting *both*
shapes, because the reference strokes them and this tree fills them. Its first column is a
**count** and is misleading: on `fm-provider-service-measures.xlsx` page 33 the reference strokes
x 113.3–373.9 once, while this tree keeps the cell's rich segments and fills 113.3–138.9 and
141.9–374.3. Two objects, one rule, the same ink. The honest column is the **covered length**:

| | summed length of link-coloured rules, pt |
|---|---:|
| this tree, base | 30 819.0 |
| this tree, after | 78 109.6 |
| 26.2.4.2 | 71 761.7 |

**27 of the 29 land within 2 % of the reference after the change; 0 of 29 did before.** The whole
of the 6 348 pt excess is one document — `Praktikastellen…xls`, §5.3 — and without it the two
sides are 71 807.0 against 71 761.7, **0.06 %**.

The other exception is `6880ac7361ca1b99a9230811_ST Capability List Rev.16 - Web.xlsx` at 163.0
against 149.6, 9 % over on three rules; not chased.

Three documents the *fods* census nominated did not move, and all three are the same story:
`Performance.xlsx`, `hdss-bulletin-index-2019-2022.xlsx` and `atspp_pay_tables.xlsx` state
`<u/>` on **every** one of their hyperlink cells (13/13, 350/350, 121/121 by instrument A), so
this tree already drew every rule and the field rule adds none. They were nominated only by the
repeat-run undercount of §1.3.

### 5.2 A second new seat, found by the reach measurement — **O65**

`Praktikastellen_-_chinesischsprachiger_Kulturraum.xls` is the one mover whose rules the
reference draws in a different colour, and the difference is **not** this round's:

| | this tree, base | this tree, after | 26.2.4.2 |
|---|---|---|---|
| rules under its hyperlink cells | 52, `#000080` | 53, `#000080` | 54, **`#0000FF`** |
| the text of those cells, page 11 | 10 spans `#000080` | 10 spans `#000080` | 10 spans **`#0000FF`** |
| first rule, page 11 | x 52.4–176.2, y 159.69 | same | x 52.4–176.2, y 160.24 |

Same cells, same places, **the reference keeps the file's own blue and this tree substitutes the
application's navy** — and it did so before this round as well, so it is the *colour* half of
`SheetLayout.HoldsField` and not the underline half. What separates it from the workbooks
`SheetHyperlinkColourTests` measured navy on: its hyperlink cells are **rich** cells.
26.2.4.2's own fods writes them as `<text:p><text:span text:style-name="T3"><text:a …>`, a span
with its own character style, which is `lclInsertUrl`'s `CELLTYPE_EDIT` branch
(`sc/source/filter/excel/xicontent.cxx`:155-195) rather than its `CELLTYPE_STRING` one. **That the
two branches differ on the page is measured; that the edit branch is the reason is a
hypothesis** — the branch still builds an `SvxURLField`, so why the LINKS colour does not reach it
is not established here.

Seated as **O65**. Reach on this census: 1 of the 29, 53 rules and 10 text spans a page.

### 5.3 O61

**2 of 307 renderings change**: `014_Contextures_chart_sample_991ecfc5.xls` — the seat's own
witness — and `EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls`. Again 0 pages, 0 characters,
0 gate verdicts.

Base to final, over the whole track: **31 of 307 renderings change, 0 page counts, 0 characters,
0 verdicts.**

---

## 6. The thickness, which is a residual and a new seat

The change puts the rule where the reference puts it. It does not make it the same thickness,
and the gap is **not** a property of this round's rule — the same gap is on every underline and
every strikethrough this tree already drew.

Six pairs measured on four documents:

| document, face, size | 26.2.4.2 | in 1/100 mm | this tree |
|---|---:|---:|---:|
| invoice p1, 10 pt | 0.51 | **18** | 0.4398 |
| invoice p3, 12 pt | 0.595 | **21** | 0.5269 |
| invoice p3, 10 pt | 0.51 | **18** | 0.4833 |
| fixture, Liberation Sans 12 | 0.595 | **21** | 0.638 |
| `002_Contextures` | 1.389 | **49** | 1.326 |
| `TK-Syllabus`, Carlito 11 (strike) | 0.794 | **28** | 0.72 |
| fixture, double underline | 0.397 | **14** | 0.638 |

**Measured:** every one of the reference's seven values is a whole hundredth of a millimetre to
four decimal places, and none of ours is. The sign is not constant — ours is 14 % thin on the
invoice's first rule and 7 % heavy on the fixture's — so it is not a scale.

**Not established:** *which* number the reference is quantising. `FontMetricData::ImplInitTextLineSize`
(`vcl/source/font/fontmetric.cxx`:200-330) has two paths — HarfBuzz's `post`/`OS/2` metrics with
`std::ceil`, and a descent-based fallback for the faces in
`Office::Common::Misc::FontsDontUseUnderlineMetrics`, which in **both** the 27.2 tree and the
installed 26.2.4.2 registry is exactly *Liberation Serif, Liberation Sans, Liberation Mono*.
Neither path reproduces the measured values: Liberation Sans' `post.underlineThickness` of 150/2048
is 0.7324 pt at 10 pt against a measured 0.51, and its fallback `((descent*25)+50)/100` at the
1/100 mm device gives 0.5386. `PDFWriterImpl` sets its reference device to
`RefDevMode::PDF1` = **720 dpi** (`vcl/source/gdi/virdev.cxx`:394-413), which cannot produce a
whole hundredth of a millimetre at all — so the value is quantised somewhere this round did not
find.

**Seated as O64.** Its reach is every underline and strikethrough in the corpus, and its size is
under a tenth of a point per rule.

---

## 7. O61 — a BIFF chart's line weights

### 7.1 The rule, in source

`XclImpChLineFormat::ReadChLineFormat` (`sc/source/filter/excel/xichart.cxx`:452-463) reads the
weight as a **signed** `Int16`; `XclChPropSetHelper::WriteLineProperties`
(`sc/source/filter/excel/xlchart.cxx`:906-925) turns it into an API width in **hundredths of a
millimetre**:

```cpp
sal_Int32 nApiWidth = 0;    // 0 is the width of a hair line
switch( rLineFmt.mnWeight )
{
    case EXC_CHLINEFORMAT_SINGLE:   nApiWidth = 35;     break;
    case EXC_CHLINEFORMAT_DOUBLE:   nApiWidth = 70;     break;
    case EXC_CHLINEFORMAT_TRIPLE:   nApiWidth = 105;    break;
}
```

`HAIR` is **−1** and `SINGLE` is **0** (`sc/source/filter/inc/xlchart.hxx`:260-263), which is the
trap: a reader taking the field as unsigned, or treating 0 as "unstated", makes every
single-weight line a hairline — which is the state this replaces.

An *automatic* line takes `rFmtInfo.mnAutoLineWeight` from `spFmtInfos`
(`xlchart.cxx`:420-440). Twelve of its sixteen object types are `HAIR`; the exceptions are
`LINEARSERIES`, `FILLEDSERIES` and `ERRORBAR` at `SINGLE` and `TRENDLINE` at `DOUBLE`. **So an
automatic width is a hairline everywhere this reader models except a series.**

*And this is where O63's open question is answered.* O63 recorded *"where 35/100 mm comes from is
not established"* for the OOXML side. It is `EXC_CHLINEFORMAT_SINGLE`'s API width, and it is
written down in the BIFF filter in plain sight. Whether the OOXML importer reaches the same
constant by the same route is **not** shown here — only that the number is not arbitrary.

### 7.2 The rule, against the binary

`o61-census.py` exports every `.xls` of the sheets track through 26.2.4.2's own
`--convert-to ods` and reads the `svg:stroke-width` off each chart part's style. Over the **7**
documents that carry a BIFF chart:

| part | no stated width | 0.035 cm (35) | 0.07 cm (70) |
|---|---:|---:|---:|
| series | 35 | **7** | **19** |
| axis | 36 | 0 | 0 |
| grid | 11 | 0 | 0 |
| chart frame | 13 | 2 | 1 |
| wall | 11 | 5 | 0 |
| plot / legend / title / floor | all | 0 | 0 |

**Every stated width in the whole class is 35 or 70 hundredths of a millimetre exactly** — the
two the table predicts — and nothing else appears.

> **An instrument note, because the first cut of this census said the opposite.** It anchored on
> `<style:style …>\s*<style:graphic-properties`, and an axis and a series put
> `<style:chart-properties>` first, so it reported *"no axis or series in the class states a
> width"* — a clean, plausible, entirely wrong answer. The corrected script reads the whole style
> block; `o61-census.py`'s docstring carries the warning.

### 7.3 What changed and what it moved

`XlsChartReader.ReadLineFormat` stops skipping the weight and maps it through `WeightWidth`;
`_axisLineWidths`, `_gridWidths` and `SeriesLinks.LineWidth` carry it to `ChartGrid.Width` and
`ChartSeries.LineWidth`, which the model already had.

On the seat's own witness, `014_Contextures_chart_sample_991ecfc5.xls`, over the whole document:

| | stroked items | hairlines | mean width | mean excluding hairlines |
|---|---:|---:|---:|---:|
| 26.2.4.2 | 136 | 108 | **0.2500** | 1.2143 |
| this tree, base | 108 | 108 | **0.0000** | 0.0000 |
| this tree, after | 108 | 88 | **0.2205** | 1.1905 |

The 16 new strokes at 0.9921 pt are the reference's 16 at 0.9884 — count for count, and the
0.37 % is the chart's own anisotropic fit scale that O63 §7.2 already records, not a width error.

On `EHEST-Pre-departure-checklist`: mean stroke width **0.5917 → 0.6196** against the reference's
0.6235, excluding hairlines 0.6445 → 0.6593 against 0.6659, and the hairline count 166 → 122
against 133.

### 7.4 What is left on that witness, and it is not this seat

The reference draws **136** stroked items on `014` and this tree draws **108**. The 28 it does
not draw are the chart-area frame and the plot wall — the `chart` and `wall` rows of §7.2 —
which are a *reader gap and a model gap*, not a width gap:

* the chart-area frame's line is not read from BIFF at all, though `ChartPlot.Border` and
  `.BorderWidth` exist for it;
* there is **no wall border in the model** — `ChartPlot.PlotBackground` is a fill and nothing else.

Two counts in the same family: on `EHEST` this tree now draws 44 strokes at ≈1.286 where the
reference draws 22 at 1.2897 — the width is right and the count is doubled. Neither is measured
further here.

**O61 closes as implemented with the reach stated: 2 of 307 documents, out of the 7 that hold a
BIFF chart and the 5 that state a series width.** The axis-line and gridline halves of the rule
are exercised **only by the fixture**: no axis or grid in the corpus states a weight at all.

---

## 8. O60 — closed on its measurement

Round 117 withdrew the ~3.3 pt figure as an instrument artefact (a justified trailing space worth
3.890 pt) and left one break on page 14 of `architecture6.ppt`, with the gap bounded in
**[0.021, 13.19) pt**. This round re-took the load-bearing number independently rather than
citing it.

`soffice --convert-to fodp` on `architecture6.ppt` through 26.2.4.2, read back with an XML
parser rather than a regex:

* the page-14 table is a `draw:frame` at `svg:x="2.844cm"`, `svg:width="19.975cm"`;
* two `table:table-column`s of **5.451 cm** and **14.525 cm**;
* every cell style carries `fo:padding-left="0.191cm" fo:padding-right="0.191cm"` and
  `fo:border-left="0.23pt solid …"`.

So the **declared** inner measure of the wide column is `14.525 − 2 × 0.191 = 14.143 cm =
400.90 pt` (round 117 wrote 400.88; the 0.03 pt is its arithmetic, and nothing turns on it).
Against round 117's break brackets:

| | bracket | 400.90 | 400.90 − 0.46 (the two borders) |
|---|---|---|---|
| 26.2.4.2 | [394.648, 400.829) | **outside, by 0.077 pt** | inside |
| this tree | [400.850, 407.837) | inside | outside |

**The direction is now named: the reference behaves as though the two 0.23 pt cell borders come
off the text area and this tree behaves as though they do not.** `Cell::TakeTextAnchorRect`
(`svx/source/table/cell.cxx`:642-649) in the 27.2 tree subtracts the four text distances and
nothing else, so the source does not support the reference's own behaviour, and no reading here
can settle which is right for 26.2.4.2.

**And the discriminator is below the instrument's floor.** The only thing separating the two
arithmetics is 0.077 pt of bracket exclusion, against an instrument whose own agreement elsewhere
on the same deck — page 10's unjustified lines, where nothing is in dispute — is **0.06 to
0.44 pt**. A change fitted to a 0.077 pt exclusion is a constant fitted to one line.

The class is bounded as well. `o60-census.py` over the 251 OOXML presentations of the slides
track: **484 tables, 30 documents with a cell side border, 8 with justified table text, 5 with
both** — and being in that class is necessary, not sufficient, since the gap only shows when it
flips a break. The `.ppt` half is not scanned, which is why 5 is a floor and why the seat's own
witness is not in it.

**O60 closes as established with the measurement.** Nothing is implemented; the mechanism, the
bracket and the direction are recorded so the next round does not re-derive them.

---

## 9. Tests

### 9.1 New — thirteen cases over two classes and one new fixture

`tests/XRay.Office.Spreadsheets.Tests/SheetHyperlinkUnderlineTests.cs` — four cases on the new
`tests/corpus/features/sheet-hyperlink-underline.xlsx`: a linked cell with no stated underline is
ruled and its unlinked twin is not; the rule spans the run and sits under its baseline; a linked
*double* underline draws one line where its unlinked twin draws two; a hyperlink on a *numeric*
cell draws neither the rule nor the link colour. Every one of the four is measured on 26.2.4.2
first — §2.2.

`tests/XRay.Office.Spreadsheets.Tests/XlsChartLineWidthTests.cs` — nine cases: the four weights and
one unrecognised value, an axis line and its grid taking different weights, an automatic series
outline being single-weight while an automatic axis line stays a hairline, and a line whose
pattern is `NONE` taking no width at all.

`BiffChartFixture.LineFormat` gains a `weight` parameter, defaulted to the 1 it always wrote so
that every case written before the weight was read keeps its bytes.

### 9.2 Mutation control

With the three source files reverted to `79ee2f7e3` and the tests, the fixture and the fixture
helper left in place, rebuilt clean:

**8 of the 13 fail; the other 5 pass at both states, and that is the design.** Every case whose
expected value is non-zero fails at the base:

| fails at the base | passes at both |
|---|---|
| `ALinkedCellIsUnderlinedWhereItsOwnFontStatesNoLine` | `AHyperlinkOnANumericCellDrawsNoRuleAndNoLinkColour` |
| `TheRuleSpansTheRunAndSitsUnderItsBaseline` | `ASeriesTakesTheWidthItsWeightNames(-1, 0)` |
| `ALinkedDoubleUnderlineIsDrawnAsOneLineAndAnUnlinkedOneAsTwo` | `ASeriesTakesTheWidthItsWeightNames(3, 0)` |
| `ASeriesTakesTheWidthItsWeightNames(0, 35)` | `AnAutomaticAxisLineStaysAHairline` |
| `ASeriesTakesTheWidthItsWeightNames(1, 70)` | `ALineStatingNoPatternTakesNoWidth` |
| `ASeriesTakesTheWidthItsWeightNames(2, 105)` | |
| `AnAutomaticSeriesOutlineIsSingleWeightAndNamesNoColour` | |
| `AnAxisLineAndItsGridTakeTheirOwnWidths` | |

The five that pass at both are the controls — the cases that assert a *zero* width or *no* rule,
which a broken reader and a fixed one agree on. A mutation control that reported 13 of 13 failing
would mean the controls were not controls.

### 9.3 The full run, to completion

```
Vector          309 passed
Markup          259 passed
Rendering       164 passed
OpenDocument    160 passed
Containers      109 passed
Core            591 passed
Spreadsheets   1384 passed
Text            728 passed
WordProcessing 1940 passed
Presentations  1203 passed
Fidelity        542 passed, 10 FAILED   (552)
--------------------------------------------
                7389 passed, 10 failed, 7399 total
```

The ten are `TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop` ×4,
`PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt` ×4,
`SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt(sheet-rich-text.xlsx)` and
`JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt(justify-shrink-2013.docx)`.
**The same ten fail by name at the base**, run separately with the source reverted — they are the
truncated-declared-width family `dotnet/CLAUDE.md` records as left failing on purpose, plus the
two its own remarks classify. Nothing here touched them.

---

## 10. What this round did not do

* **The turned and stacked cell paths draw no rules at all**, hyperlink or otherwise. Pre-existing;
  no corpus witness among the 29 movers.
* **The thickness quantiser** — §6, seated as **O64**.
* **A rich hyperlink cell's colour** — §5.2, seated as **O65**. It predates this round; the change
  neither caused nor worsened it beyond the one rule it added on that document.
* **A BIFF chart's frame line and its wall border** — §7.4. The frame's fields exist on the model
  and are unread; the wall's do not exist.
* **The fill-versus-stroke shape difference.** This tree fills every rule and 26.2.4.2 strokes
  every one. The ink is the same to within §6's thickness, but any census that counts strokes
  scores all of ours as absent — which is how `TK-Syllabus` came to be filed as missing ink. Worth
  a line in whatever the next census instrument is; not worth changing the drawing for.
* **The `.xls` half of instrument A.** The XML census does not parse BIFF, so its 27-document
  figure is over 243 of the 307 and the two `.xls` movers were found by the render diff alone.

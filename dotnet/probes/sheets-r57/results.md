# Round 57 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r57`, base
`a45eb8e5391`. Read `prediction.md` (`eb114f4edbc`) beside this file first: it was committed
before a line of the change was written and before anything was rendered post-change.

## 1. Baseline, reproduced — and the one document that looked like movement was the calendar

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 291 MISMATCH 34`. Scored against
`MANIFEST.tsv`'s 307 sheets paths (the raw total double-counts 18 case-alias entries):
**277 match / 30 mismatch**, one *above* the briefed 276.

The one is `sheets/unstable-001/xlsx/fse_identification_form.xlsx`, and the sweep refuted it
against itself. The corpus mount gives that file two names, so `batch-check.sh` rendered it twice
minutes apart:

```
fse_identification_form.XLSX   3/3   440/427   3/3   words
fse_identification_form.xlsx   3/3   440/440   3/3   match
```

Our half is pinned by `SOURCE_DATE_EPOCH` at 440 both times; **the reference's moved 427 → 440
with the wall clock.** The reproducible baseline is therefore **276 of 307**, exactly as briefed.
No manifest-`done` document mismatched and 30 of the 31 manifest-`open` documents mismatched on
both spellings. *The case alias — normally only a counting nuisance — is here the cheapest
available control on the reference's own reproducibility.*

## 2. Result

**276 → 276 of 307. Zero verdict movement, which is what the prediction file said.** Corpus
795 of 946.

| | base | after |
|---|---|---|
| `fm-provider-service-measures` p36, median body-token displacement | **18.460 pt** | **0.006 pt** |
| `FY2023-AIP-grants` p1, the same | **18.489 pt** | **0.035 pt** |
| `FY2023-AIP-grants` words | 51039 / ref 51045 | **51045 / ref 51045** |
| `fm-provider` p15 tokens | 599 / ref 610 | **610 / ref 610** |
| our-side word counts that moved | — | **6 documents, every one upwards** |
| tests, ten non-Fidelity projects | 4825 / 0 / 1 | **4830 / 0 / 1** |

## 3. The 18.46 pt body offset: not a header height counted twice — a band that is not scaled

Round 56 measured a uniform downward translation of the body on two documents that agree to a
twentieth of a point, and its brief said to test whether the header height was being counted
twice. It is not. **Both witnesses are *scaled* worksheets** — `fitToHeight="17"` on
`fm-provider` sheet 7 and `scale="43"` on `FY2023` sheet 1 — and the mechanism is one line of the
reference's own source:

```
ScPrintFunc::GetDocPageSize   (sc/source/ui/view/printfun.cxx:3002-3003)
    aPageRect.SetTop( ( aPageRect.Top() + nTopMargin ) * 100 / nZoom + aHdr.nHeight );
```

The page rectangle is built in **document twips**: each margin is divided by the zoom and each
band is added whole. A document twip reaches the paper at `zoom/100` of a physical twip, because
the map mode the page is drawn through carries the zoom as its scale fraction
(`ScPrintFunc::InitModes`, `printfun.cxx:2645`). So the margin comes back out at full size and
**the band arrives at `nHeight × zoom/100`**.

Arithmetic on both witnesses, done before the probe was written:

| | stated band | nominal | printed band `H` | zoom | `H × (1 − zoom)` | round 56 measured |
|---|---:|---:|---:|---:|---:|---:|
| `fm-provider` sheet 7 | 32.4 pt | 14 (one `&14` line) | ≈ 35.45 | ≈ 0.479 | **18.5** | 18.46 |
| `FY2023-AIP-grants` sheet 1 | 32.4 pt | 33 (three 11 pt lines → **pinned**) | 32.4 | 0.43 | **18.47** | 18.49 |

`probe-bandscale.py` — five print scales, one 14 pt header line over a 32.4 pt stated band, the
100 % control first:

| scale | band text size ref / ours | body token y ref / ours, **before** | ours − ref | **after** |
|---:|---|---|---:|---:|
| 100 | 14.0 / 14.0 | 56.18 / 56.21 | 0.03 | **0.03** |
| 80 | 11.2 / 11.2 | 49.18 / 56.04 | 6.86 | **0.08** |
| 60 | 8.4 / 8.4 | 42.33 / 55.98 | 13.65 | **0.02** |
| 40 | 5.6 / 5.6 | 35.46 / 55.90 | 20.44 | **0.00** |
| 25 | 3.5 / 3.5 | 30.26 / 55.83 | 25.57 | **0.03** |

Three things that reading gets right and "counted twice" does not:

1. **The residual moves with the scale** — it is `HeaderHeight × (1 − zoom)` to within 1.5 %,
   not a constant 18 pt. A double count would have given the same number at every scale.
2. **The band's own *text* was already right on our side and agrees at all five scales.**
   `SheetPageDecoration.DrawBand` has taken the zoom since it was written. That is why a scaled
   sheet's header was the correct size over a body that was in the wrong place, and it is what
   made the defect look like a mystery rather than a scale.
3. **`SheetPagination.DocPageSize` had already ported the same arithmetic**, which is why no page
   count was ever wrong — but its comment said the bands "are printed at full size whatever the
   sheet's scale: they are page furniture rather than content", and
   `SheetPrintSetup.PrintableArea`, which *places* what a page holds, implemented that sentence
   instead. **Two ports of one formula, one of them written from the prose of the other.**

The witnesses, re-measured token by token against the stored reference bank
(`witness-bodyoffset.py`, median over the page's paired tokens rather than over its first one, so
a translation and a scale error cannot look alike):

| | paired tokens | base median | after median |
|---|---:|---:|---:|
| `fm-provider-service-measures` p36 | 550 | **18.460** | **0.006** |
| `FY2023-AIP-grants` p1 | 2242 | **18.489** | **0.035** |

## 4. Prediction against measurement

| | predicted | measured |
|---|---|---|
| sheets verdicts | 276 → **276**, zero movement either way | **276, zero movement** |
| documents whose body ink moves | 53 xlsx-family plus unmeasured `.xls` | consistent; 6 documents moved a *word count*, 3 of them `.xls` |
| direction of the movement | **upwards, always** | **6 of 6 upwards, 0 downwards** |
| page counts anywhere | 0 change | **0 change** |
| word counts | 0 on all but **at most two** documents | **WRONG — six moved** (§ 5). The direction held; the number did not |
| band ink | 0 change | **0 change**; the probe's band-text column is identical before and after at all five scales |
| the 100 % control | byte-identical | **unmoved: 0.03 pt before and after** |
| words / slides tracks | 0 | **0 — no shared layer touched** (§ 9) |
| tests | +6 to +12, `XRay.Office.Spreadsheets` only | **+5, `XRay.Office.Spreadsheets` only** — one below the range |
| `MANIFEST.tsv` | no row changes status | **no row changes status** |

**Eight of ten.** Both misses are counts rather than directions, and one of them is under: the
tests came in at +5 against a floor of +6 because the three drawing tests and the one unit test
turned out to cover the four mutations between them (§ 8) and a fifth would have been a drift
guard.

## 5. The six word counts that moved, all upwards, and the sweep split by side

**Split every sweep diff by which side moved** — the trap the brief names, and it decided the
headline here. Thirteen rows changed a metric; six of them are ours and seven are the reference's
clock.

**Ours (the change):**

| document | before → after | reference |
|---|---:|---:|
| `FY2023-AIP-grants.xlsx` | 51039 → **51045** | 51045 — **exact** |
| `fm-provider-service-measures.xlsx` | 21347 → 21358 | 21348 |
| `orbus_togaf_tool_csq.xls` (both aliases) | 46836 → 46838 | 46780 |
| `TOGAF9-Tool-ConfReqts-CSQ.xls` | 23618 → 23620 | 23513 |
| `Application_Compliance_Checklist_5_Apr_2021.xlsx` | 17620 → 17622 | 17654 |
| `environment-edb-docs-edb-emissions-databank.xls` | 63953 → 63954 | 63955 |

**Every one is an increase**, which is what the prediction said the mechanism could only produce:
the body moves *up*, so a row that used to run off the bottom of the paper comes back onto it.
**Three of the six are `.xls`** — the arm neither census could see, and the third round running
that blind spot 1 has fired where it was pointed.

`fm-provider` is the one to read closely, because its total moved *away* from the reference
(21347 was one short, 21358 is ten over) and the per-page reading says the change is right anyway:
**the whole movement is on page 15, where we gained exactly the eleven tokens the reference has
and the page went from 599/610 to 610/610 with an empty `ours − ref` difference.** The ten extra
tokens elsewhere in that document are pre-existing and untouched. A document-level count would
have scored this as a regression.

**The reference's (the calendar):** `047_Date_tracker_Gantt_chart` 848 → 822, `PBN Matrix NAAs`,
`ans_mappings_of_eccairs_terms`, `SIL_TDB648` 7495 → 7492, `FAA…attachment_2.XLSX` 9995 → 9990,
`fse_identification_form` 440 → 427. Our side is byte-stable on all six.

## 6. The grey cell fills are conditional formatting, and the census says do not start it here

Round 56 handed on "three `#C0C0C0` fills the reference draws on `FAA-2019-0995-0002` p28 that we
do not draw at all — census before writing anything". Censused, and the answer is that the fills
are not a fill rule at all.

The three grey rows are the ones whose column C holds `MISSING`, and `sheet10` carries

```xml
<conditionalFormatting sqref="C5:F99">
  <cfRule type="expression" dxfId="1" priority="1" stopIfTrue="1">
    <formula>MID($C5,1,7)="MISSING"</formula></cfRule></conditionalFormatting>
<conditionalFormatting sqref="G5:R99">
  <cfRule type="expression" dxfId="0" priority="2" stopIfTrue="1">
    <formula>AND(MID($C5,1,7)="MISSING",$G5="")</formula></cfRule></conditionalFormatting>
```

with `dxf` 0 and 1 both `<fill><patternFill><bgColor indexed="22"/>`, and indexed 22 is `#C0C0C0`.
**Drawing them needs a formula evaluator with `MID` and `AND`, and a relative-reference rewrite
per row.** Two things were checked before that was believed: the `cellXf`s on those cells state
`fillId="0"` and their `xfId="6"` cell style states `fillId="0"` too, so it is not style
inheritance; and the workbook has no other grey fill that could account for it.

The census, over the 243 xlsx-family sheets documents:

| `cfRule type` | documents |
|---|---:|
| `colorScale` | **38** |
| `expression` | **34** |
| `cellIs` | 18 |
| `dataBar` | 6 |
| `containsText` | 5 |
| `duplicateValues` | 5 |
| `iconSet` | 2 |
| `notContainsBlanks` | 2 |
| `containsBlanks`, `endsWith` | 1 each |
| **any rule at all** | **89 of 243** |

We implement **none** of it: the only file that reads a `cfRule` is `Ooxml/XlsxHiddenValues.cs`,
and it reads one to find out whether an icon set or data bar *hides* its cell's value. So the item
round 56 handed on as "three grey fills" is the hardest arm — `expression`, 34 documents — of a
subsystem reaching **89 of 243**, of which `colorScale` (38 documents) needs no formula at all and
`cellIs` (18) needs only a comparison. That is the order a round should take them in, and it is
not a fill this round could have written.

## 7. The four `_advanced_excel_pie` documents: not clipping, and two measured causes

The brief records these as "established as clipping at the horizontal page split (`17%` → `7%`,
`trend` → `rend`), not a fused label". **They are not clipping.** `pdf-ops.py` on
`003_advanced_excel_pie` page 2 shows both renderers emitting *every* glyph of every label —
19-glyph and 20-glyph runs on both sides. `17%`/`7%` and `trend`/`rend` are **`pdftotext`'s
decoding of a run whose origin is off the left edge of the MediaBox**, not glyphs anyone dropped;
poppler recovers the part that lands on the paper. The tokens the gate is missing are missing
because the labels are *in different places*, and there are two measurable reasons:

1. **The reference wraps each pie data label onto two lines and we set it on one.** Its M2 label
   is `M2; Actual; 100;` (17 glyphs) at x 439.67 followed by `19%` (3 glyphs) at x 463.90, 11.2 pt
   below; ours is one 20-glyph run at x 412.57. Centred text makes a wrapped label start 27 pt
   further right, which is what decides whether its tokens fall on page 1 or page 2.
2. **Every chart run is in the wrong face and the title in the wrong size.** The reference's title
   is **18.01 pt Carlito Bold** and ours is **13.00 pt Liberation Sans**; its labels are 10.01 pt
   Carlito and ours 10.00 pt Liberation Sans. The workbook's theme minor font is Calibri.

Both were reported independently by a blind reviewer who had only the composed page (§ 8) —
"the reference title is bolder and larger", "the reference wraps most labels onto two lines,
ours keeps each on one line" — on a page chosen for a stated reason rather than by `--worst`,
and confirmed by a different instrument. That is the three-part discriminator `HANDOVER.md` § 7
asks for, and it holds here.

`ChartLayout` and the chart text stack are `XRay.Office.Core`, so this is the same shape of job as
`IntervalsThatFit` and owes a corpus gate.

## 8. The vision round: one confirmation, one extension, and two refutations

Three fresh subagents, one composed page each, no project documents, no source, no shell, each
asked to describe the halves separately and to give a direction. **Every page was chosen for a
stated reason** and none by `--worst`.

* **`fm-provider-service-measures` p36** (chosen because it is the page the change targets).
  "Both tables appear to start and end at essentially the same vertical position … row-by-row
  content lines up at the same heights on both sides." That is the fix, seen from outside, and the
  reviewer volunteered it as the closest candidate for a difference and then declined to call one.
* **`FAA-2019-0995-0002_attachment_2` p28** (chosen for the grey fills). "Distinct grey shading is
  visible in the reference … the top panel shows them plain white." **Confirmed**, and the
  reviewer's *extent* is wrong in an instructive way: it reported the grey as filling the seven
  narrow check columns on every row, and `pdf-ops.py` says it is **three rectangles spanning
  x 214–754**, i.e. three whole *rows*. A band across a row read as a stripe down a column.
* **`003_advanced_excel_pie` p1** (chosen for the chart title and label wrap). Both findings in
  § 7, independently.

**Refuted, both by an instrument that answers the exact claim:**

* *"Ours reads `Page 24 of 18` where the reference reads `Page 18 of 18`."* Neither string is in
  either document. `pdftotext` on pages 34-37 gives `Page 34 of 38` … `Page 37 of 38` on **both
  sides, identically**. A 3.79 pt footer rasterised at 150 dpi is below what a reader can resolve,
  and this is the first time a reviewer on this track has invented a *token* rather than
  misjudged a position.
* *"Ours draws a black outer border around the table and the reference does not."* Counted:
  the reference draws **90 horizontal and 5 vertical** strokes on that page and we draw **44 and
  4**. The claim points the wrong way. **This is the second consecutive round in which a reviewer
  has reported a rule or border the reference supposedly omits, and the second in which the
  reference draws more of them than we do** — round 56's was `NPIAS_App_A` p12 at 22 against 24.
  Worth naming as a class: *reviewers systematically over-report borders on our side.*

And a number counted while refuting that one, which is **not** a defect and is written down so it
is not chased: the reference emits one `#F2F2F2` rectangle for the whole block where we emit
**171 per-cell fills of the same colour**, and it emits two coincident rules per row edge where we
emit one. Same ink, different op counts, same pitch (16.56 pt) on both.

## 9. Shared layer

**No.** Every file this round changes is in `XRay.Office.Spreadsheets`:
`Layout/SheetPrintSetup.cs`, `Layout/SpreadsheetPages.cs`, `Layout/SheetNotes.cs`,
`Ooxml/XlsxNoteCaptions.cs` (comment only). `git diff a45eb8e5391..HEAD --stat -- dotnet/src`
names no other project. The words and slides tracks cannot see it and **no cross-track sweep is
owed**. `OdsPrintSetup` is untouched and there is no `.ods` in the sheets corpus, so that arm is
unmeasured either way.

## 10. Tests

**+5, all in `XRay.Office.Spreadsheets`** (956 → 961). The ten non-Fidelity projects, re-derived
rather than quoted: Containers 109, Core 337, Markup 259, OpenDocument 125, Presentations 819,
Rendering 153 (+1 skipped), Spreadsheets 961, Text 617, Vector 295, WordProcessing 1155 —
**4830 passed, 0 failed, 1 skipped**, against a base of 4825 by the same count.
`dotnet build -v q -nologo` → **0 warnings, 0 errors**.

**Three mutations through `verify-test.sh`, all three detected:**

| mutation | detected by |
|---|---|
| the band taken at full size again (`TopMargin + HeaderHeight`) | `AScaledSheetReservesOnlyTheScaledPartOfItsBand`, `APinnedBandScalesLikeADynamicOne`, `ThePrintableAreaScalesTheBandsAndNotTheMargins` |
| **the wiring** — `PrintableAreaAt(_scale)` → `PrintableAreaAt(1.0)` at the call site | the first two of those, from the whole 961-test run with no filter |
| the margins scaled too (`(TopMargin + HeaderHeight) * scale`) | all three |

The wiring mutation is the one worth having, and it is the lesson round 56 wrote down: the unit
test alone passes under it, because the unit is still correct and simply never asked for the
page's scale. The `Unscaled` sheet is the control and no mutation moves it.

## 11. The 24.2.7.2 audit — `Ooxml/XlsxNoteCaptions.cs`, **VERIFIED**

The claim: *a VML note anchor's offsets are 96-dpi screen pixels*
(`ShapeAnchor::importVmlAnchor` → `CellAnchorType::Pixel`, `calcCellAnchorEmu` through
`Unit::ScreenX`). `audit_vmlanchor.py`, eight 60 pt rows and one shown comment, the only variable
being the anchor's row offset, read out of `soffice --convert-to fods` rather than off a
rendering — a shown comment has a border and a shadow, so a PDF measures its decoration too.

| row offset | annotation `svg:y` | step | implied dpi |
|---:|---:|---:|---:|
| 0 | **119.988** (the control: 2 × 60.0 pt of row grid) | — | — |
| 20 | 134.986 | 14.998 | **96.0** |
| 40 | 149.983 | 14.998 | **96.0** |
| 60 | 164.974 | 14.990 | **96.1** |

Seventy-two dpi would have stepped it by 20 pt and EMUs by nothing. **VERIFIED on 26.2.4.2.**

**The probe's first cut said "neither" at every step, and it was measuring a clamp rather than a
law.** Its rows were 20 pt — 26.7 px — and its offsets 48, 96 and 144, so all three saturated at
exactly one row (39.996, 59.897, 59.897, 59.897). Chasing that produced the round's second audit
finding: **26.2.4.2 clamps a VML anchor's offset to the anchored cell's own extent** — 200 px and
400 px into a 60 pt row both land on 179.885 against a row-3 top of 180.0 — and
`XlsxVml.ParseAnchor` does not clamp. **Recorded at the site and not implemented**, because the
clamp needs the sheet's grid at anchor-resolution time and because the corpus barely exercises it.

**And that census had to be written twice, which is the same lesson again.** Its first cut
compared every anchor against the tallest row and widest column stated *anywhere in the workbook*
and answered **zero** — a bound generous enough that nothing can exceed it, since one 200 pt row
exempts the whole file. The corpus's largest row offset is 111 px (83.25 pt) against a 20 px
default row, so "zero" was a property of the bound. Resolving each VML part to its own worksheet
through the relationships and comparing against *that row's own height* gives
**5 anchors of 365, in one document of fifteen** (`023_Waterfall_Chart_Template_for_Excel`), the
worst overshooting by 20 %. *A search that finds nothing has to be shown capable of finding
something* — twice in one round, in a probe and in a census, both caught before they were
reported.

**Counters, re-derived with the file's own commands.** At this tree: **39 open sites, 19 marker
lines (16 `VERIFIED`, 2 `FIXED`, 1 `WRONG`, 0 `UNDECIDED`)**. At the base: **40 open, 18 lines
(15/2/1/0)**. The file's stored table said **42** open with `XRay.Office.Presentations` at 11; the
real figures at the base are 40 and 9, so **two of its three headline numbers were already wrong
when they were written** and only the sheets column moved this round. Corrected there, for the
fifth time in that file's history.

**And round 56's pattern did not repeat.** It recorded "the only sheets site found wrong is the
only *furniture* claim, which is a pattern rather than a coincidence" and sent this round at the
other furniture claim on the strength of it. `XlsxNoteCaptions.cs` is correct. Two observations of
one event; the re-check is what settled it either way. `XRay.Office.Spreadsheets` is now **nine of
ten** re-checked, eight correct, and **`Layout/SheetText.cs` is the last one**.

## 12. `MANIFEST.tsv`

Lives in the corpus repository and was **not touched**. **No row changes status** — the round
moves no verdict, and that was predicted. One row is worth a note rather than a change:
`sheets/unstable-001/xlsx/fse_identification_form.xlsx` is marked `open` and matched on one of its
two renderings in the *baseline* sweep and on neither in the post-change sweep, with our side
byte-stable at 440 throughout. Its batch is already named `unstable-001`; no change is proposed.

## 13. What the next round should do first

1. **Conditional formatting, and take `colorScale` first.** 89 of 243 xlsx-family documents carry
   a `cfRule` and we draw none of them. `colorScale` is the largest arm (38 documents) and needs
   no formula evaluator at all — a minimum, a maximum and an interpolation over a stated range.
   `cellIs` (18) is a comparison. `expression` (34), which is what the FAA grey fills are, needs
   `MID`, `AND` and a per-row relative-reference rewrite and should be last. § 6 has the census.
2. **The chart face, which is not a chart-layout problem.** On `003_advanced_excel_pie` the
   reference draws every chart run in **Carlito** and its title at **18.01 pt bold**; we draw
   Liberation Sans at 13.00 pt. The workbook's theme minor font is Calibri. Confirmed by a blind
   reviewer and by `pdf-ops.py`. This is a *font resolution* question before it is a layout one
   and it may be much cheaper than the label wrap beside it.
3. **The pie data-label wrap** — the other half of § 7, and the one that actually moves the four
   `_advanced_excel_pie` verdicts. `XRay.Office.Core`, so it owes a corpus gate.
4. **`Layout/SheetText.cs`** — the last unverified 24.2.7.2 site in `XRay.Office.Spreadsheets`.
5. **`ChartLayout.IntervalsThatFit`** — untouched again; round 56 § 9 has the census (256 axes,
   129 documents, all three tracks), the sign argument that rules out the shape insets, and the
   arithmetic bound on the mm100 truncation.
6. **The note page's own scale**, which this round deliberately left alone and wrote down at the
   call site: `SheetNotePages` uses `PrintableAreaAt(1.0)` where `ScPrintFunc::PrintNotes` shares
   `aPageRect` with `PrintPage`, and `SheetPage`'s note constructor leaves `Placement` at its
   default, so a note page's band is drawn at a **one per cent** zoom. Zero xlsx-family documents
   set `cellComments="atEnd"`; the `.xls` `SETUP` `fNotes` bit is not censused.
7. Still unworked, all ink: the chart area's light-grey border; a data label group's stated `bg1`
   fill; a band's `&K` colour, which is now the last thing on a band that does not print.
